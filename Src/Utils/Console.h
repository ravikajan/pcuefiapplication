/** @file
  Console.h - Console I/O Helpers

  Provides colored text output, screen clearing, cursor positioning,
  and keyboard input utilities for the UEFI console.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_CONSOLE_H_
#define HW_CONSOLE_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleTextOut.h>

// Color constants (EFI text attributes)
#define CON_COLOR_DEFAULT     EFI_WHITE
#define CON_COLOR_HIGHLIGHT   EFI_YELLOW
#define CON_COLOR_SUCCESS     EFI_GREEN
#define CON_COLOR_ERROR       EFI_RED
#define CON_COLOR_WARNING     EFI_YELLOW
#define CON_COLOR_INFO        EFI_CYAN
#define CON_COLOR_HEADER      EFI_LIGHTBLUE
#define CON_BG_DEFAULT        EFI_BACKGROUND_BLACK
#define CON_BG_HEADER         EFI_BACKGROUND_BLUE

/**
  Clear the console screen.
**/
VOID
EFIAPI
ConsoleClear (
  VOID
  );

/**
  Set the console text color and background.

  @param[in] Foreground   EFI text foreground color attribute.
  @param[in] Background   EFI text background color attribute.
**/
VOID
EFIAPI
ConsoleSetColor (
  IN UINTN Foreground,
  IN UINTN Background
  );

/**
  Set the cursor position on the console.

  @param[in] Col  Column (0-based).
  @param[in] Row  Row (0-based).
**/
VOID
EFIAPI
ConsoleSetPosition (
  IN UINTN Col,
  IN UINTN Row
  );

/**
  Print a string at a specific position with color.

  @param[in] Col         Column position.
  @param[in] Row         Row position.
  @param[in] Foreground  Text color.
  @param[in] Background  Background color.
  @param[in] Fmt         Format string.
  @param[in] ...         Variable arguments.
**/
VOID
EFIAPI
ConsolePrintAt (
  IN UINTN    Col,
  IN UINTN    Row,
  IN UINTN    Foreground,
  IN UINTN    Background,
  IN CHAR16   *Fmt,
  ...
  );

/**
  Print a horizontal line across the screen.

  @param[in] Row     Row position.
  @param[in] Char    Character to use for the line.
  @param[in] Color   Color attribute.
**/
VOID
EFIAPI
ConsolePrintLine (
  IN UINTN    Row,
  IN CHAR16   Char,
  IN UINTN    Color
  );

/**
  Wait for a single key press and return it.

  @param[out] Key  The key data that was pressed.

  @retval EFI_SUCCESS  A key was read.
**/
EFI_STATUS
EFIAPI
ConsoleWaitForKey (
  OUT EFI_INPUT_KEY *Key
  );

/**
  Print the application header banner.
**/
VOID
EFIAPI
ConsolePrintHeader (
  VOID
  );

/**
  Print a status badge [PASS], [FAIL], [SKIP] with color.

  @param[in] Col     Column position.
  @param[in] Row     Row position.
  @param[in] Status  The test status to display.
**/
VOID
EFIAPI
ConsolePrintStatusBadge (
  IN UINTN           Col,
  IN UINTN           Row,
  IN UINT8           Status
  );

/**
  Get the console dimensions.

  @param[out] Cols  Number of columns.
  @param[out] Rows  Number of rows.
**/
VOID
EFIAPI
ConsoleGetSize (
  OUT UINTN *Cols,
  OUT UINTN *Rows
  );

#endif // HW_CONSOLE_H_
