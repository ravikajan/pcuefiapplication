/** @file
  Progress.c - Progress Screen Implementation

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Progress.h"
#include "../Utils/Console.h"
#include "Graphics.h"
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>

// Track test statuses for display
#define MAX_DISPLAY_TESTS 16

STATIC CHAR16          mTestNames[MAX_DISPLAY_TESTS][HW_TEST_NAME_MAX];
STATIC HW_TEST_STATUS  mTestStatuses[MAX_DISPLAY_TESTS];
STATIC UINTN           mTotalTests = 0;

VOID
EFIAPI
ProgressInit (
  IN UINTN TotalTests
  )
{
  UINTN i;

  mTotalTests = TotalTests;
  if (mTotalTests > MAX_DISPLAY_TESTS) {
    mTotalTests = MAX_DISPLAY_TESTS;
  }

  for (i = 0; i < MAX_DISPLAY_TESTS; i++) {
    mTestNames[i][0] = L'\0';
    mTestStatuses[i] = TestStatusSkip;
  }

  // Draw the progress screen
  ConsoleClear ();
  ConsolePrintHeader ();

  ConsolePrintAt (
    2, 4,
    CON_COLOR_HIGHLIGHT, CON_BG_DEFAULT,
    L"Running Hardware Tests..."
    );

  ConsolePrintLine (5, L'-', EFI_DARKGRAY);
}

VOID
EFIAPI
ProgressUpdate (
  IN UINTN            CurrentTest,
  IN UINTN            TotalTests,
  IN CHAR16           *TestName,
  IN HW_TEST_STATUS   Status
  )
{
  UINTN  Row;
  UINTN  Cols;
  UINTN  Rows;
  UINTN  Percent;
  UINTN  i;
  CHAR16 ProgressText[80];

  ConsoleGetSize (&Cols, &Rows);

  // Store the test name and status
  if (CurrentTest < MAX_DISPLAY_TESTS) {
    // Copy test name safely
    {
      UINTN Len = StrLen (TestName);
      UINTN Max = HW_TEST_NAME_MAX - 1;
      if (Len > Max) Len = Max;
      CopyMem (mTestNames[CurrentTest], TestName, Len * sizeof (CHAR16));
      mTestNames[CurrentTest][Len] = L'\0';
    }
    mTestStatuses[CurrentTest] = Status;
  }

  // Calculate progress percentage
  Percent = 0;
  if (TotalTests > 0) {
    Percent = ((CurrentTest + 1) * 100) / TotalTests;
  }

  // Draw progress bar text
  UnicodeSPrint (
    ProgressText, sizeof (ProgressText),
    L"  Progress: %d / %d tests  (%d%%)",
    CurrentTest + 1, TotalTests, Percent
    );
  ConsolePrintAt (2, 7, CON_COLOR_DEFAULT, CON_BG_DEFAULT, ProgressText);

  // Draw a text-based progress bar
  {
    UINTN BarWidth = Cols - 8;
    UINTN FillWidth = (BarWidth * Percent) / 100;
    UINTN Pad;

    ConsoleSetPosition (4, 8);
    ConsoleSetColor (EFI_GREEN, CON_BG_DEFAULT);
    gST->ConOut->OutputString (gST->ConOut, L"[");

    for (Pad = 0; Pad < BarWidth; Pad++) {
      if (Pad < FillWidth) {
        gST->ConOut->OutputString (gST->ConOut, L"#");
      } else {
        ConsoleSetColor (EFI_DARKGRAY, CON_BG_DEFAULT);
        gST->ConOut->OutputString (gST->ConOut, L"-");
        ConsoleSetColor (EFI_GREEN, CON_BG_DEFAULT);
      }
    }

    gST->ConOut->OutputString (gST->ConOut, L"]");
    ConsoleSetColor (CON_COLOR_DEFAULT, CON_BG_DEFAULT);
  }

  // Draw test list with statuses
  ConsolePrintLine (10, L'-', EFI_DARKGRAY);
  ConsolePrintAt (2, 11, CON_COLOR_INFO, CON_BG_DEFAULT, L"Test Results:");

  Row = 12;
  for (i = 0; i <= CurrentTest && i < MAX_DISPLAY_TESTS; i++) {
    // Badge
    ConsolePrintStatusBadge (4, Row, (UINT8)mTestStatuses[i]);

    // Test name
    ConsolePrintAt (
      12, Row,
      CON_COLOR_DEFAULT, CON_BG_DEFAULT,
      L"%s",
      mTestNames[i]
      );

    Row++;
  }

  // Currently running indicator
  if (Status == TestStatusSkip && CurrentTest < TotalTests) {
    // Test is still running — show a "running" indicator
    ConsolePrintAt (
      4, Row - 1,
      EFI_YELLOW, CON_BG_DEFAULT,
      L"[....]"
      );
  }
}

VOID
EFIAPI
ProgressComplete (
  IN UINTN TotalTests,
  IN UINTN Passed,
  IN UINTN Failed
  )
{
  UINTN Cols;
  UINTN Rows;
  CHAR16 Summary[128];

  ConsoleGetSize (&Cols, &Rows);

  ConsolePrintLine (Rows - 5, L'=', EFI_DARKGRAY);

  UnicodeSPrint (
    Summary, sizeof (Summary),
    L"  COMPLETE: %d/%d passed, %d failed",
    Passed, TotalTests, Failed
    );

  if (Failed == 0) {
    ConsolePrintAt (2, Rows - 4, EFI_GREEN, CON_BG_DEFAULT, Summary);
    ConsolePrintAt (
      2, Rows - 3, EFI_GREEN, CON_BG_DEFAULT,
      L"  ALL TESTS PASSED"
      );
  } else {
    ConsolePrintAt (2, Rows - 4, EFI_RED, CON_BG_DEFAULT, Summary);
    ConsolePrintAt (
      2, Rows - 3, EFI_RED, CON_BG_DEFAULT,
      L"  SOME TESTS FAILED - Review results above"
      );
  }

  ConsolePrintAt (
    2, Rows - 1,
    EFI_DARKGRAY, CON_BG_DEFAULT,
    L"  Press any key to continue..."
    );
}
