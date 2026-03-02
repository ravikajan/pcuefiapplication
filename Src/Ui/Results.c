/** @file
  Results.c - Results Summary Screen Implementation

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Results.h"
#include "../Utils/Console.h"
#include "../Utils/String.h"
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>

VOID
EFIAPI
ResultsShow (
  IN HW_TEST_RESULT  *Results,
  IN UINTN           ResultCount
  )
{
  UINTN i;
  UINTN Row;
  UINTN Cols;
  UINTN Rows;
  UINTN Passed = 0;
  UINTN Failed = 0;
  UINTN Skipped = 0;
  BOOLEAN HasDebugIssue = FALSE;
  CHAR16 Summary[128];
  CHAR16 DurationStr[32];
  CHAR16 DebugName[HW_TEST_NAME_MAX];
  CHAR16 DebugDetails[HW_TEST_DETAIL_MAX];

  ConsoleGetSize (&Cols, &Rows);
  DebugName[0] = L'\0';
  DebugDetails[0] = L'\0';
  ConsoleClear ();
  ConsolePrintHeader ();

  ConsolePrintAt (
    2, 4,
    CON_COLOR_HIGHLIGHT, CON_BG_DEFAULT,
    L"Test Results Summary"
    );

  ConsolePrintLine (5, L'=', EFI_DARKGRAY);

  // Table header
  ConsolePrintAt (4, 7, CON_COLOR_INFO, CON_BG_DEFAULT, L"Status");
  ConsolePrintAt (14, 7, CON_COLOR_INFO, CON_BG_DEFAULT, L"Test Name");
  ConsolePrintAt (45, 7, CON_COLOR_INFO, CON_BG_DEFAULT, L"Duration");
  ConsolePrintAt (58, 7, CON_COLOR_INFO, CON_BG_DEFAULT, L"Details");
  ConsolePrintLine (8, L'-', EFI_DARKGRAY);

  Row = 9;

  for (i = 0; i < ResultCount && Row < Rows - 6; i++) {
    HW_TEST_RESULT *Res = &Results[i];

    // Status badge
    ConsolePrintStatusBadge (4, Row, (UINT8)Res->Status);

    // Count results
    switch (Res->Status) {
      case TestStatusPass:
        Passed++;
        break;
      case TestStatusFail:
        Failed++;
        break;
      case TestStatusSkip:
        Skipped++;
        break;
      default:
        Failed++;
        break;
    }

    // Test name
    ConsolePrintAt (14, Row, CON_COLOR_DEFAULT, CON_BG_DEFAULT, L"%s", Res->TestName);

    // Duration
    UnicodeSPrint (DurationStr, sizeof (DurationStr), L"%lu ms", Res->DurationMs);
    ConsolePrintAt (45, Row, EFI_LIGHTGRAY, CON_BG_DEFAULT, DurationStr);

    // Details (wider clip based on screen width)
    {
      CHAR16 DetailTrunc[HW_TEST_DETAIL_MAX];
      UINTN  DetailLen = StrLen (Res->Details);
      UINTN  MaxDetail = (Cols > 60) ? (Cols - 60) : 10;
      UINTN  CopyLen;

      if (DetailLen > MaxDetail) {
        CopyLen = MaxDetail;
      } else {
        CopyLen = DetailLen;
      }
      CopyMem (DetailTrunc, Res->Details, CopyLen * sizeof (CHAR16));
      DetailTrunc[CopyLen] = L'\0';

      if (Res->Status == TestStatusFail || Res->Status == TestStatusError) {
        ConsolePrintAt (58, Row, EFI_RED, CON_BG_DEFAULT, DetailTrunc);
      } else {
        ConsolePrintAt (58, Row, EFI_DARKGRAY, CON_BG_DEFAULT, DetailTrunc);
      }
    }

    if (!HasDebugIssue && (Res->Status == TestStatusFail || Res->Status == TestStatusError)) {
      StringSafeCopy (DebugName, sizeof (DebugName), Res->TestName);
      StringSafeCopy (DebugDetails, sizeof (DebugDetails), Res->Details);
      HasDebugIssue = TRUE;
    }

    Row++;
  }

  // Summary footer
  ConsolePrintLine (Rows - 5, L'=', EFI_DARKGRAY);

  UnicodeSPrint (
    Summary, sizeof (Summary),
    L"  Total: %d  |  Passed: %d  |  Failed: %d  |  Skipped: %d",
    ResultCount, Passed, Failed, Skipped
    );

  if (Failed == 0) {
    ConsolePrintAt (2, Rows - 4, EFI_GREEN, CON_BG_DEFAULT, Summary);
  } else {
    ConsolePrintAt (2, Rows - 4, EFI_RED, CON_BG_DEFAULT, Summary);
  }

  if (HasDebugIssue) {
    CHAR16 DebugLine[HW_TEST_DETAIL_MAX];
    UINTN  MaxDebugChars;

    ConsolePrintAt (2, Rows - 3, EFI_YELLOW, CON_BG_DEFAULT, L"  Debug [%s]", DebugName);

    MaxDebugChars = (Cols > 14) ? (Cols - 14) : 20;
    if (MaxDebugChars > (HW_TEST_DETAIL_MAX - 1)) {
      MaxDebugChars = HW_TEST_DETAIL_MAX - 1;
    }
    if (StrLen (DebugDetails) > MaxDebugChars) {
      CopyMem (DebugLine, DebugDetails, MaxDebugChars * sizeof (CHAR16));
      DebugLine[MaxDebugChars] = L'\0';
    } else {
      StringSafeCopy (DebugLine, sizeof (DebugLine), DebugDetails);
    }

    ConsolePrintAt (2, Rows - 2, EFI_LIGHTRED, CON_BG_DEFAULT, L"  %s", DebugLine);
  }

  ConsolePrintAt (
    2, Rows - 1,
    EFI_DARKGRAY, CON_BG_DEFAULT,
    L"  Press any key to return to main menu..."
    );

  // Wait for key
  {
    EFI_INPUT_KEY Key;
    ConsoleWaitForKey (&Key);
  }
}
