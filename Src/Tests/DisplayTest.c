/** @file
  DisplayTest.c - Display Test Module

  Queries available GOP modes, reports supported resolutions,
  and performs color fill tests for visual verification.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "TestModule.h"
#include "../Utils/Console.h"
#include "../Utils/Timer.h"
#include "../Utils/String.h"
#include "../Ui/Graphics.h"
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Protocol/GraphicsOutput.h>

#define DISPLAY_NOTE_MAX_CHARS  120

/**
  Wait for Enter key.
**/
STATIC
VOID
WaitForEnterKey (
  VOID
  )
{
  EFI_INPUT_KEY Key;

  while (TRUE) {
    ConsoleWaitForKey (&Key);
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      return;
    }
  }
}

/**
  Prompt user for pass/fail confirmation.

  @retval TRUE   User marked display test as PASS.
  @retval FALSE  User marked display test as FAIL.
**/
STATIC
BOOLEAN
PromptUserPassFail (
  VOID
  )
{
  EFI_INPUT_KEY Key;

  while (TRUE) {
    ConsoleWaitForKey (&Key);

    if (Key.UnicodeChar == L'Y' || Key.UnicodeChar == L'y') {
      return TRUE;
    }
    if (Key.UnicodeChar == L'N' || Key.UnicodeChar == L'n') {
      return FALSE;
    }
  }
}

/**
  Read an optional note line from keyboard input.
**/
STATIC
VOID
ReadUserNote (
  OUT CHAR16 *Buffer,
  IN  UINTN  BufferSize,
  IN  UINTN  Col,
  IN  UINTN  Row
  )
{
  EFI_INPUT_KEY Key;
  UINTN         MaxChars;
  UINTN         Length;
  UINTN         i;

  if (Buffer == NULL || BufferSize < sizeof (CHAR16)) {
    return;
  }

  MaxChars = (BufferSize / sizeof (CHAR16)) - 1;
  Buffer[0] = L'\0';
  Length = 0;

  while (TRUE) {
    // Clear input area
    ConsoleSetPosition (Col, Row);
    ConsoleSetColor (EFI_LIGHTGRAY, CON_BG_DEFAULT);
    for (i = 0; i < 70; i++) {
      gST->ConOut->OutputString (gST->ConOut, L" ");
    }

    // Redraw current input
    ConsolePrintAt (Col, Row, EFI_LIGHTGRAY, CON_BG_DEFAULT, L"%s", Buffer);

    ConsoleWaitForKey (&Key);

    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      break;
    }

    if (Key.UnicodeChar == CHAR_BACKSPACE) {
      if (Length > 0) {
        Length--;
        Buffer[Length] = L'\0';
      }
      continue;
    }

    if (Key.UnicodeChar >= L' ' && Length < MaxChars) {
      Buffer[Length] = Key.UnicodeChar;
      Length++;
      Buffer[Length] = L'\0';
    }
  }
}

/**
  Run the Display test module.
**/
EFI_STATUS
EFIAPI
DisplayTestRun (
  IN  HW_TEST_MODULE  *Self,
  OUT HW_TEST_RESULT  *Result
  )
{
  EFI_STATUS                      Status;
  UINT64                          StartTime;
  UINT64                          EndTime;
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *Gop;
  UINT32                          MaxMode;
  UINT32                          CurrentWidth;
  UINT32                          CurrentHeight;
  UINT32                          BestWidth;
  UINT32                          BestHeight;
  UINT32                          i;
  BOOLEAN                         UserPass;
  CHAR16                          UserNote[DISPLAY_NOTE_MAX_CHARS];

  StringSafeCopy (Result->TestName, sizeof (Result->TestName), L"Display Test");
  StartTime = TimerGetMs ();

  // Try to locate GOP
  Status = gBS->LocateProtocol (
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  (VOID **)&Gop
                  );

  if (EFI_ERROR (Status)) {
    EndTime = TimerGetMs ();
    Result->DurationMs = EndTime - StartTime;
    Result->Status = TestStatusSkip;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"GOP not available (text-only mode)"
      );
    return EFI_SUCCESS;
  }

  // Get current mode info
  CurrentWidth = Gop->Mode->Info->HorizontalResolution;
  CurrentHeight = Gop->Mode->Info->VerticalResolution;

  // Enumerate all available modes
  MaxMode = Gop->Mode->MaxMode;
  BestWidth = 0;
  BestHeight = 0;

  for (i = 0; i < MaxMode; i++) {
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN InfoSize;

    Status = Gop->QueryMode (Gop, i, &InfoSize, &Info);
    if (!EFI_ERROR (Status)) {
      if (Info->HorizontalResolution > BestWidth) {
        BestWidth = Info->HorizontalResolution;
        BestHeight = Info->VerticalResolution;
      }
    }
  }

  // Interactive color validation flow (user confirms each color)
  {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL TestColors[4];
    CHAR16                        *ColorNames[4];
    UINTN c;

    // Red
    TestColors[0].Blue = 0x00; TestColors[0].Green = 0x00;
    TestColors[0].Red = 0xFF; TestColors[0].Reserved = 0x00;
    // Green
    TestColors[1].Blue = 0x00; TestColors[1].Green = 0xFF;
    TestColors[1].Red = 0x00; TestColors[1].Reserved = 0x00;
    // Blue
    TestColors[2].Blue = 0xFF; TestColors[2].Green = 0x00;
    TestColors[2].Red = 0x00; TestColors[2].Reserved = 0x00;
    // White
    TestColors[3].Blue = 0xFF; TestColors[3].Green = 0xFF;
    TestColors[3].Red = 0xFF; TestColors[3].Reserved = 0x00;

    ColorNames[0] = L"RED";
    ColorNames[1] = L"GREEN";
    ColorNames[2] = L"BLUE";
    ColorNames[3] = L"WHITE";

    ConsoleClear ();
    ConsolePrintHeader ();
    ConsolePrintAt (2, 5, CON_COLOR_HIGHLIGHT, CON_BG_DEFAULT, L"Display Color Validation");
    ConsolePrintAt (2, 7, EFI_LIGHTGRAY, CON_BG_DEFAULT, L"You will see 4 full-screen colors.");
    ConsolePrintAt (2, 8, EFI_LIGHTGRAY, CON_BG_DEFAULT, L"For each color, inspect panel quality and press ENTER.");
    ConsolePrintAt (2, 10, EFI_DARKGRAY, CON_BG_DEFAULT, L"Press ENTER to start...");
    WaitForEnterKey ();

    for (c = 0; c < 4; c++) {
      Status = Gop->Blt (
                      Gop,
                      &TestColors[c],
                      EfiBltVideoFill,
                      0, 0,
                      0, 0,
                      CurrentWidth, CurrentHeight,
                      0
                      );
      if (EFI_ERROR (Status)) {
        EndTime = TimerGetMs ();
        Result->DurationMs = EndTime - StartTime;
        Result->Status = TestStatusFail;
        UnicodeSPrint (
          Result->Details, sizeof (Result->Details),
          L"GOP Blt failed at color %d",
          c
          );
        return EFI_SUCCESS;
      }

      ConsolePrintAt (2, 2, EFI_BLACK, EFI_BACKGROUND_LIGHTGRAY,
        L" Color %d/4: %s - Press ENTER for next ",
        c + 1,
        ColorNames[c]
        );

      WaitForEnterKey ();
    }
  }

  // Restore console (clear screen)
  {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black;
    Black.Blue = 0; Black.Green = 0; Black.Red = 0; Black.Reserved = 0;
    Gop->Blt (Gop, &Black, EfiBltVideoFill, 0, 0, 0, 0,
              CurrentWidth, CurrentHeight, 0);
  }

  ConsoleClear ();
  ConsolePrintHeader ();
  ConsolePrintAt (2, 5, CON_COLOR_HIGHLIGHT, CON_BG_DEFAULT, L"Display Check Complete");
  ConsolePrintAt (2, 7, EFI_LIGHTGRAY, CON_BG_DEFAULT, L"Did all colors look correct?");
  ConsolePrintAt (2, 8, EFI_LIGHTGRAY, CON_BG_DEFAULT, L"Press Y for PASS or N for FAIL");

  UserPass = PromptUserPassFail ();
  UserNote[0] = L'\0';

  if (!UserPass) {
    ConsolePrintAt (2, 10, EFI_YELLOW, CON_BG_DEFAULT, L"Optional issue note (press ENTER to finish):");
    ReadUserNote (UserNote, sizeof (UserNote), 2, 11);
  }

  EndTime = TimerGetMs ();
  Result->DurationMs = EndTime - StartTime;

  if (UserPass) {
    Result->Status = TestStatusPass;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"Current: %dx%d | Max: %dx%d | %d modes | User visual check PASS",
      CurrentWidth, CurrentHeight,
      BestWidth, BestHeight,
      MaxMode
      );
  } else {
    Result->Status = TestStatusFail;
    if (StrLen (UserNote) > 0) {
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"Current: %dx%d | Max: %dx%d | %d modes | Visual FAIL | Note: %s",
        CurrentWidth, CurrentHeight,
        BestWidth, BestHeight,
        MaxMode,
        UserNote
        );
    } else {
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"Current: %dx%d | Max: %dx%d | %d modes | User visual check FAIL",
        CurrentWidth, CurrentHeight,
        BestWidth, BestHeight,
        MaxMode
        );
    }
  }

  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gDisplayTestModule = {
  L"Display Test",                                    // Name
  L"GOP modes, resolution info, color fill test",     // Description
  FALSE,                                              // IsQuickTest (visual)
  DisplayTestRun                                      // Run
};
