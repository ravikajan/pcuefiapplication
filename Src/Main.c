/** @file
  Main.c - UEFI Hardware Test Suite Entry Point

  UefiMain() initializes console, GOP, test runner, and enters the
  interactive main menu loop. Orchestrates the full application lifecycle:
  Boot Menu → Test Selection → Test Execution → Results → Report Export.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include "Utils/Console.h"
#include "Utils/Timer.h"
#include "Utils/String.h"
#include "Driver/DriverLoader.h"
#include "Ui/Graphics.h"
#include "Ui/Menu.h"
#include "Ui/Progress.h"
#include "Ui/Results.h"
#include "Tests/TestModule.h"
#include "Runner/TestRunner.h"
#include "Report/Report.h"

// Main menu item indices
#define MENU_FULL_TEST      0
#define MENU_QUICK_TEST     1
#define MENU_CUSTOM_TEST    2
#define MENU_VIEW_RESULTS   3
#define MENU_SAVE_REPORT    4
#define MENU_EXIT           5

// Global results storage
STATIC HW_TEST_RESULT mResults[HW_TEST_MAX_MODULES];
STATIC UINTN          mResultCount = 0;
STATIC BOOLEAN        mHasResults = FALSE;

/**
  Progress callback for the test runner — updates the progress screen.
**/
STATIC
VOID
EFIAPI
MainProgressCallback (
  IN UINTN            CurrentTest,
  IN UINTN            TotalTests,
  IN CHAR16           *TestName,
  IN HW_TEST_STATUS   Status
  )
{
  ProgressUpdate (CurrentTest, TotalTests, TestName, Status);

  // Small delay so user can see progress updating
  TimerDelayMs (100);
}

/**
  Run all tests (Full Test mode).
**/
STATIC
VOID
RunFullTest (
  VOID
  )
{
  UINTN Passed;
  UINTN Failed;
  UINTN i;

  ProgressInit (TestRunnerGetModuleCount ());
  TestRunnerRunAll (mResults, &mResultCount, MainProgressCallback);
  mHasResults = TRUE;

  // Count results
  Passed = 0;
  Failed = 0;
  for (i = 0; i < mResultCount; i++) {
    if (mResults[i].Status == TestStatusPass) {
      Passed++;
    } else if (mResults[i].Status == TestStatusFail) {
      Failed++;
    }
  }

  ProgressComplete (mResultCount, Passed, Failed);

  // Wait for key press
  {
    EFI_INPUT_KEY Key;
    ConsoleWaitForKey (&Key);
  }

  // Show results
  ResultsShow (mResults, mResultCount);
}

/**
  Run quick test subset.
**/
STATIC
VOID
RunQuickTest (
  VOID
  )
{
  UINTN Passed;
  UINTN Failed;
  UINTN QuickCount;
  UINTN i;

  // Count quick tests
  QuickCount = 0;
  for (i = 0; i < TestRunnerGetModuleCount (); i++) {
    HW_TEST_MODULE *Mod = TestRunnerGetModule (i);
    if (Mod != NULL && Mod->IsQuickTest) {
      QuickCount++;
    }
  }

  ProgressInit (QuickCount);
  TestRunnerRunQuick (mResults, &mResultCount, MainProgressCallback);
  mHasResults = TRUE;

  // Count results
  Passed = 0;
  Failed = 0;
  for (i = 0; i < mResultCount; i++) {
    if (mResults[i].Status == TestStatusPass) {
      Passed++;
    } else if (mResults[i].Status == TestStatusFail) {
      Failed++;
    }
  }

  ProgressComplete (mResultCount, Passed, Failed);

  {
    EFI_INPUT_KEY Key;
    ConsoleWaitForKey (&Key);
  }

  ResultsShow (mResults, mResultCount);
}

/**
  Run custom-selected tests.
**/
STATIC
VOID
RunCustomTest (
  VOID
  )
{
  MENU_CONTEXT SelectMenu;
  UINTN        Indices[HW_TEST_MAX_MODULES];
  UINTN        IndexCount;
  UINTN        i;
  UINTN        Choice;

  ConsoleClear ();
  ConsolePrintHeader ();

  // Build checkbox menu of all test modules
  MenuInit (&SelectMenu, L"Select Tests to Run", TRUE, 4);

  for (i = 0; i < TestRunnerGetModuleCount (); i++) {
    HW_TEST_MODULE *Mod = TestRunnerGetModule (i);
    if (Mod != NULL) {
      MenuAddItem (&SelectMenu, Mod->Name, Mod->Description, TRUE);
    }
  }

  Choice = MenuRun (&SelectMenu);

  // Check if user cancelled
  if (Choice == (UINTN)-1) {
    return;
  }

  // Collect selected indices
  IndexCount = 0;
  for (i = 0; i < SelectMenu.ItemCount; i++) {
    if (SelectMenu.Items[i].IsSelected) {
      Indices[IndexCount] = i;
      IndexCount++;
    }
  }

  if (IndexCount == 0) {
    ConsoleClear ();
    ConsolePrintHeader ();
    ConsolePrintAt (2, 5, EFI_YELLOW, CON_BG_DEFAULT, L"No tests selected. Returning to menu...");
    TimerDelayMs (1500);
    return;
  }

  ProgressInit (IndexCount);
  TestRunnerRunSelected (Indices, IndexCount, mResults, &mResultCount, MainProgressCallback);
  mHasResults = TRUE;

  // Count results
  {
    UINTN Passed = 0;
    UINTN Failed = 0;

    for (i = 0; i < mResultCount; i++) {
      if (mResults[i].Status == TestStatusPass) {
        Passed++;
      } else if (mResults[i].Status == TestStatusFail) {
        Failed++;
      }
    }

    ProgressComplete (mResultCount, Passed, Failed);
  }

  {
    EFI_INPUT_KEY Key;
    ConsoleWaitForKey (&Key);
  }

  ResultsShow (mResults, mResultCount);
}

/**
  Save report to USB.
**/
STATIC
VOID
SaveReport (
  VOID
  )
{
  EFI_STATUS Status;

  ConsoleClear ();
  ConsolePrintHeader ();

  if (!mHasResults || mResultCount == 0) {
    ConsolePrintAt (2, 5, EFI_YELLOW, CON_BG_DEFAULT,
      L"No test results to save. Run tests first.");
    TimerDelayMs (2000);
    return;
  }

  ConsolePrintAt (2, 5, CON_COLOR_INFO, CON_BG_DEFAULT,
    L"Saving reports (JSON, CSV, TXT) to filesystem...");

  Status = ReportSave (mResults, mResultCount, REPORT_FORMAT_ALL);

  if (!EFI_ERROR (Status)) {
    ConsolePrintAt (2, 7, EFI_GREEN, CON_BG_DEFAULT,
      L"Reports saved successfully!");
    ConsolePrintAt (2, 9, EFI_LIGHTGRAY, CON_BG_DEFAULT,
      L"Files: HwTest_<timestamp>.json/.csv/.txt");
  } else {
    ConsolePrintAt (2, 7, EFI_RED, CON_BG_DEFAULT,
      L"Failed to save reports.");
    ConsolePrintAt (2, 8, EFI_RED, CON_BG_DEFAULT,
      L"Ensure a writable FAT32 filesystem is available.");
  }

  ConsolePrintAt (2, 11, EFI_DARKGRAY, CON_BG_DEFAULT,
    L"Press any key to return to menu...");
  {
    EFI_INPUT_KEY Key;
    ConsoleWaitForKey (&Key);
  }
}

/**
  Application entry point.
**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  MENU_CONTEXT MainMenu;
  BOOLEAN      Running;
  UINTN        Choice;
  UINTN        DriverLoaded;
  UINTN        DriverStarted;
  UINTN        DriverFailed;

  // Disable watchdog timer (prevents auto-reset during long tests)
  gBS->SetWatchdogTimer (0, 0, 0, NULL);

  // Initialize GOP (optional — app works in text mode too)
  {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
    GfxInit (&Gop);
  }

  DriverLoaded = 0;
  DriverStarted = 0;
  DriverFailed = 0;
  DriverLoaderLoadAll (ImageHandle, &DriverLoaded, &DriverStarted, &DriverFailed);

  if (DriverLoaded > 0 || DriverFailed > 0) {
    ConsoleClear ();
    ConsolePrintHeader ();
    ConsolePrintAt (2, 5, CON_COLOR_INFO, CON_BG_DEFAULT,
      L"Local driver preload (offline): loaded=%d started=%d failed=%d",
      DriverLoaded,
      DriverStarted,
      DriverFailed
      );
    TimerDelayMs (1200);
  }

  // Initialize test runner
  TestRunnerInit ();

  // Clear results
  ZeroMem (mResults, sizeof (mResults));
  mResultCount = 0;
  mHasResults = FALSE;

  Running = TRUE;

  while (Running) {
    // Draw main screen
    ConsoleClear ();
    ConsolePrintHeader ();

    // Build main menu
    MenuInit (&MainMenu, L"Main Menu", FALSE, 4);
    MenuAddItem (&MainMenu, L"Full Test",     L"Run all hardware diagnostic tests", TRUE);
    MenuAddItem (&MainMenu, L"Quick Test",    L"Run essential tests only (CPU, Memory, Storage, Network, WiFi, USB)", TRUE);
    MenuAddItem (&MainMenu, L"Custom Test",   L"Select which tests to run", TRUE);
    MenuAddItem (&MainMenu, L"View Results",  L"View results from the last test run", TRUE);
    MenuAddItem (&MainMenu, L"Save Report",   L"Export test results to JSON, CSV, and TXT files", TRUE);
    MenuAddItem (&MainMenu, L"Exit",          L"Exit the Hardware Test Suite", TRUE);

    Choice = MenuRun (&MainMenu);

    switch (Choice) {
      case MENU_FULL_TEST:
        RunFullTest ();
        break;

      case MENU_QUICK_TEST:
        RunQuickTest ();
        break;

      case MENU_CUSTOM_TEST:
        RunCustomTest ();
        break;

      case MENU_VIEW_RESULTS:
        if (mHasResults && mResultCount > 0) {
          ResultsShow (mResults, mResultCount);
        } else {
          ConsoleClear ();
          ConsolePrintHeader ();
          ConsolePrintAt (2, 5, EFI_YELLOW, CON_BG_DEFAULT,
            L"No test results yet. Run a test first.");
          TimerDelayMs (2000);
        }
        break;

      case MENU_SAVE_REPORT:
        SaveReport ();
        break;

      case MENU_EXIT:
      default:
        Running = FALSE;
        break;
    }
  }

  // Exit message
  ConsoleClear ();
  ConsolePrintHeader ();
  ConsolePrintAt (2, 5, CON_COLOR_HIGHLIGHT, CON_BG_DEFAULT,
    L"UEFI Hardware Test Suite - Exiting...");
  ConsolePrintAt (2, 7, EFI_LIGHTGRAY, CON_BG_DEFAULT,
    L"System will return to UEFI firmware.");
  TimerDelayMs (2000);

  return EFI_SUCCESS;
}
