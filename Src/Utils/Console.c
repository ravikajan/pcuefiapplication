/** @file
  Console.c - Console I/O Helpers Implementation

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Console.h"
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include "../Tests/TestModule.h"

VOID
EFIAPI
ConsoleClear (
  VOID
  )
{
  gST->ConOut->ClearScreen (gST->ConOut);
}

VOID
EFIAPI
ConsoleSetColor (
  IN UINTN Foreground,
  IN UINTN Background
  )
{
  gST->ConOut->SetAttribute (gST->ConOut, Foreground | Background);
}

VOID
EFIAPI
ConsoleSetPosition (
  IN UINTN Col,
  IN UINTN Row
  )
{
  gST->ConOut->SetCursorPosition (gST->ConOut, Col, Row);
}

VOID
EFIAPI
ConsolePrintAt (
  IN UINTN    Col,
  IN UINTN    Row,
  IN UINTN    Foreground,
  IN UINTN    Background,
  IN CHAR16   *Fmt,
  ...
  )
{
  VA_LIST Args;
  CHAR16  Buffer[256];

  ConsoleSetPosition (Col, Row);
  ConsoleSetColor (Foreground, Background);

  VA_START (Args, Fmt);
  UnicodeVSPrint (Buffer, sizeof (Buffer), Fmt, Args);
  VA_END (Args);

  gST->ConOut->OutputString (gST->ConOut, Buffer);
  ConsoleSetColor (CON_COLOR_DEFAULT, CON_BG_DEFAULT);
}

VOID
EFIAPI
ConsolePrintLine (
  IN UINTN    Row,
  IN CHAR16   Char,
  IN UINTN    Color
  )
{
  UINTN Cols;
  UINTN Rows;
  UINTN i;
  CHAR16 LineChar[2];

  ConsoleGetSize (&Cols, &Rows);
  ConsoleSetPosition (0, Row);
  ConsoleSetColor (Color, CON_BG_DEFAULT);

  LineChar[0] = Char;
  LineChar[1] = L'\0';

  for (i = 0; i < Cols; i++) {
    gST->ConOut->OutputString (gST->ConOut, LineChar);
  }

  ConsoleSetColor (CON_COLOR_DEFAULT, CON_BG_DEFAULT);
}

EFI_STATUS
EFIAPI
ConsoleWaitForKey (
  OUT EFI_INPUT_KEY *Key
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  Status = gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return gST->ConIn->ReadKeyStroke (gST->ConIn, Key);
}

VOID
EFIAPI
ConsolePrintHeader (
  VOID
  )
{
  UINTN Cols;
  UINTN Rows;

  ConsoleGetSize (&Cols, &Rows);

  // Top banner bar
  ConsoleSetColor (EFI_WHITE, CON_BG_HEADER);
  ConsoleSetPosition (0, 0);

  // Fill the top line with spaces for background
  {
    UINTN i;
    for (i = 0; i < Cols; i++) {
      gST->ConOut->OutputString (gST->ConOut, L" ");
    }
  }

  // Print the title centered
  {
    CHAR16 *Title = L"  UEFI Hardware Test Suite v1.0  ";
    UINTN  TitleLen = StrLen (Title);
    UINTN  StartCol = 0;
    if (Cols > TitleLen) {
      StartCol = (Cols - TitleLen) / 2;
    }
    ConsoleSetPosition (StartCol, 0);
    gST->ConOut->OutputString (gST->ConOut, Title);
  }

  // Subtitle line
  ConsoleSetColor (EFI_LIGHTGRAY, CON_BG_HEADER);
  ConsoleSetPosition (0, 1);
  {
    UINTN i;
    for (i = 0; i < Cols; i++) {
      gST->ConOut->OutputString (gST->ConOut, L" ");
    }
  }
  {
    CHAR16 *Subtitle = L"  Pre-OS Hardware Diagnostics  ";
    UINTN  SubLen = StrLen (Subtitle);
    UINTN  StartCol = 0;
    if (Cols > SubLen) {
      StartCol = (Cols - SubLen) / 2;
    }
    ConsoleSetPosition (StartCol, 1);
    gST->ConOut->OutputString (gST->ConOut, Subtitle);
  }

  // Separator line
  ConsoleSetColor (CON_COLOR_DEFAULT, CON_BG_DEFAULT);
  ConsolePrintLine (2, L'=', EFI_DARKGRAY);
}

VOID
EFIAPI
ConsolePrintStatusBadge (
  IN UINTN           Col,
  IN UINTN           Row,
  IN UINT8           Status
  )
{
  ConsoleSetPosition (Col, Row);

  switch (Status) {
    case TestStatusPass:
      ConsoleSetColor (EFI_GREEN, CON_BG_DEFAULT);
      gST->ConOut->OutputString (gST->ConOut, L"[PASS]");
      break;
    case TestStatusFail:
      ConsoleSetColor (EFI_RED, CON_BG_DEFAULT);
      gST->ConOut->OutputString (gST->ConOut, L"[FAIL]");
      break;
    case TestStatusSkip:
      ConsoleSetColor (EFI_YELLOW, CON_BG_DEFAULT);
      gST->ConOut->OutputString (gST->ConOut, L"[SKIP]");
      break;
    case TestStatusError:
      ConsoleSetColor (EFI_LIGHTRED, CON_BG_DEFAULT);
      gST->ConOut->OutputString (gST->ConOut, L"[ERR!]");
      break;
    default:
      ConsoleSetColor (EFI_DARKGRAY, CON_BG_DEFAULT);
      gST->ConOut->OutputString (gST->ConOut, L"[----]");
      break;
  }

  ConsoleSetColor (CON_COLOR_DEFAULT, CON_BG_DEFAULT);
}

VOID
EFIAPI
ConsoleGetSize (
  OUT UINTN *Cols,
  OUT UINTN *Rows
  )
{
  gST->ConOut->QueryMode (
                 gST->ConOut,
                 gST->ConOut->Mode->Mode,
                 Cols,
                 Rows
                 );
}
