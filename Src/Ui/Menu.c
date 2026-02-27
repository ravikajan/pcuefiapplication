/** @file
  Menu.c - Interactive Menu System Implementation

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Menu.h"
#include "../Utils/Console.h"
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>

/**
  Local safe string copy to avoid dependency on String.c symbol.
**/
STATIC
VOID
MenuStrCopy (
  OUT CHAR16       *Dest,
  IN  UINTN        DestSize,
  IN  CONST CHAR16 *Src
  )
{
  UINTN MaxChars;
  UINTN SrcLen;
  UINTN CopyLen;

  if (Dest == NULL || Src == NULL || DestSize < sizeof (CHAR16)) {
    return;
  }

  MaxChars = (DestSize / sizeof (CHAR16)) - 1;
  SrcLen = StrLen (Src);
  CopyLen = (SrcLen < MaxChars) ? SrcLen : MaxChars;

  CopyMem (Dest, Src, CopyLen * sizeof (CHAR16));
  Dest[CopyLen] = L'\0';
}

VOID
EFIAPI
MenuInit (
  OUT MENU_CONTEXT  *Menu,
  IN  CHAR16        *Title,
  IN  BOOLEAN       IsCheckbox,
  IN  UINTN         StartRow
  )
{
  ZeroMem (Menu, sizeof (MENU_CONTEXT));
  MenuStrCopy (Menu->Title, sizeof (Menu->Title), Title);
  Menu->IsCheckbox = IsCheckbox;
  Menu->StartRow = StartRow;
  Menu->ItemCount = 0;
  Menu->SelectedIndex = 0;
}

EFI_STATUS
EFIAPI
MenuAddItem (
  IN OUT MENU_CONTEXT  *Menu,
  IN     CHAR16        *Label,
  IN     CHAR16        *Description,
  IN     BOOLEAN       Selectable
  )
{
  MENU_ITEM *Item;

  if (Menu->ItemCount >= MENU_MAX_ITEMS) {
    return EFI_OUT_OF_RESOURCES;
  }

  Item = &Menu->Items[Menu->ItemCount];
  MenuStrCopy (Item->Label, sizeof (Item->Label), Label);
  MenuStrCopy (Item->Description, sizeof (Item->Description), Description);
  Item->Selectable = Selectable;
  Item->IsSelected = FALSE;

  Menu->ItemCount++;
  return EFI_SUCCESS;
}

VOID
EFIAPI
MenuDraw (
  IN MENU_CONTEXT *Menu
  )
{
  UINTN i;
  UINTN Row;
  UINTN Cols;
  UINTN Rows;

  ConsoleGetSize (&Cols, &Rows);

  // Draw title
  ConsolePrintAt (2, Menu->StartRow, CON_COLOR_HEADER, CON_BG_DEFAULT, Menu->Title);
  ConsolePrintLine (Menu->StartRow + 1, L'-', EFI_DARKGRAY);

  Row = Menu->StartRow + 2;

  for (i = 0; i < Menu->ItemCount; i++) {
    MENU_ITEM *Item = &Menu->Items[i];
    BOOLEAN IsHighlighted = (i == Menu->SelectedIndex);

    ConsoleSetPosition (2, Row);

    if (IsHighlighted) {
      ConsoleSetColor (EFI_WHITE, EFI_BACKGROUND_BLUE);
    } else if (!Item->Selectable) {
      ConsoleSetColor (EFI_DARKGRAY, CON_BG_DEFAULT);
    } else {
      ConsoleSetColor (EFI_LIGHTGRAY, CON_BG_DEFAULT);
    }

    // Prefix: arrow for highlighted, checkbox for multi-select
    if (Menu->IsCheckbox) {
      if (Item->IsSelected) {
        gST->ConOut->OutputString (gST->ConOut, L"  [X] ");
      } else {
        gST->ConOut->OutputString (gST->ConOut, L"  [ ] ");
      }
    } else {
      if (IsHighlighted) {
        gST->ConOut->OutputString (gST->ConOut, L"  >> ");
      } else {
        gST->ConOut->OutputString (gST->ConOut, L"     ");
      }
    }

    gST->ConOut->OutputString (gST->ConOut, Item->Label);

    // Pad to end of line for highlight bar
    {
      UINTN Printed;
      UINTN Pad;

      if (Menu->IsCheckbox) {
        Printed = 6 + StrLen (Item->Label) + 2;
      } else {
        Printed = 5 + StrLen (Item->Label) + 2;
      }

      for (Pad = Printed; Pad < Cols - 2; Pad++) {
        gST->ConOut->OutputString (gST->ConOut, L" ");
      }
    }

    ConsoleSetColor (CON_COLOR_DEFAULT, CON_BG_DEFAULT);
    Row++;
  }

  // Draw description of highlighted item
  Row = Menu->StartRow + Menu->ItemCount + 3;
  ConsolePrintLine (Row, L'-', EFI_DARKGRAY);
  Row++;

  // Clear old description
  ConsoleSetPosition (2, Row);
  {
    UINTN Pad;
    for (Pad = 0; Pad < Cols - 4; Pad++) {
      gST->ConOut->OutputString (gST->ConOut, L" ");
    }
  }

  if (Menu->SelectedIndex < Menu->ItemCount) {
    ConsolePrintAt (
      2, Row,
      EFI_CYAN, CON_BG_DEFAULT,
      Menu->Items[Menu->SelectedIndex].Description
      );
  }

  // Navigation help
  Row += 2;
  ConsolePrintAt (
    2, Row,
    EFI_DARKGRAY, CON_BG_DEFAULT,
    L"Use UP/DOWN arrows to navigate, ENTER to select"
    );

  if (Menu->IsCheckbox) {
    ConsolePrintAt (
      2, Row + 1,
      EFI_DARKGRAY, CON_BG_DEFAULT,
      L"SPACE to toggle selection, ENTER to confirm"
      );
  }
}

UINTN
EFIAPI
MenuRun (
  IN OUT MENU_CONTEXT *Menu
  )
{
  EFI_INPUT_KEY Key;

  // Ensure selected index starts on a selectable item
  while (Menu->SelectedIndex < Menu->ItemCount &&
         !Menu->Items[Menu->SelectedIndex].Selectable) {
    Menu->SelectedIndex++;
  }

  while (TRUE) {
    MenuDraw (Menu);
    ConsoleWaitForKey (&Key);

    switch (Key.ScanCode) {
      case SCAN_UP:
        // Move up to previous selectable item
        if (Menu->SelectedIndex > 0) {
          Menu->SelectedIndex--;
          while (Menu->SelectedIndex > 0 &&
                 !Menu->Items[Menu->SelectedIndex].Selectable) {
            Menu->SelectedIndex--;
          }
          // If landed on non-selectable, move back down
          if (!Menu->Items[Menu->SelectedIndex].Selectable) {
            while (Menu->SelectedIndex < Menu->ItemCount &&
                   !Menu->Items[Menu->SelectedIndex].Selectable) {
              Menu->SelectedIndex++;
            }
          }
        }
        break;

      case SCAN_DOWN:
        // Move down to next selectable item
        if (Menu->SelectedIndex < Menu->ItemCount - 1) {
          Menu->SelectedIndex++;
          while (Menu->SelectedIndex < Menu->ItemCount - 1 &&
                 !Menu->Items[Menu->SelectedIndex].Selectable) {
            Menu->SelectedIndex++;
          }
          // If landed on non-selectable, move back up
          if (!Menu->Items[Menu->SelectedIndex].Selectable) {
            while (Menu->SelectedIndex > 0 &&
                   !Menu->Items[Menu->SelectedIndex].Selectable) {
              Menu->SelectedIndex--;
            }
          }
        }
        break;

      default:
        break;
    }

    // Handle character keys
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      // Enter pressed — confirm selection
      return Menu->SelectedIndex;
    }

    if (Key.UnicodeChar == L' ' && Menu->IsCheckbox) {
      // Space toggles checkbox
      if (Menu->Items[Menu->SelectedIndex].Selectable) {
        Menu->Items[Menu->SelectedIndex].IsSelected =
          !Menu->Items[Menu->SelectedIndex].IsSelected;
      }
    }

    if (Key.UnicodeChar == CHAR_BACKSPACE || Key.ScanCode == SCAN_ESC) {
      // Escape — return max to signal cancellation
      return (UINTN)-1;
    }
  }
}
