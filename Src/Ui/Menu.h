/** @file
  Menu.h - Interactive Menu System

  Provides keyboard-navigable menus for the UEFI Hardware Test Suite.
  Used for Boot Menu, Custom Test selection, and other interactive screens.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_MENU_H_
#define HW_MENU_H_

#include <Uefi.h>
#include <Library/UefiLib.h>

#define MENU_MAX_ITEMS      16
#define MENU_ITEM_MAX_LEN   80

// Menu item descriptor
typedef struct {
  CHAR16    Label[MENU_ITEM_MAX_LEN];
  CHAR16    Description[MENU_ITEM_MAX_LEN];
  BOOLEAN   Selectable;
  BOOLEAN   IsSelected;    // For checkbox-style menus
} MENU_ITEM;

// Menu descriptor
typedef struct {
  CHAR16      Title[MENU_ITEM_MAX_LEN];
  MENU_ITEM   Items[MENU_MAX_ITEMS];
  UINTN       ItemCount;
  UINTN       SelectedIndex;
  BOOLEAN     IsCheckbox;  // TRUE = multi-select, FALSE = single-select
  UINTN       StartRow;    // Row on console where menu drawing starts
} MENU_CONTEXT;

/**
  Initialize a new menu context.

  @param[out] Menu       Menu context to initialize.
  @param[in]  Title      Menu title string.
  @param[in]  IsCheckbox TRUE for multi-select, FALSE for single-select.
  @param[in]  StartRow   Console row to begin drawing.
**/
VOID
EFIAPI
MenuInit (
  OUT MENU_CONTEXT  *Menu,
  IN  CHAR16        *Title,
  IN  BOOLEAN       IsCheckbox,
  IN  UINTN         StartRow
  );

/**
  Add an item to the menu.

  @param[in,out] Menu         Menu context.
  @param[in]     Label        Item label.
  @param[in]     Description  Item description (shown when highlighted).
  @param[in]     Selectable   Whether the item can be selected.

  @retval EFI_SUCCESS      Item added.
  @retval EFI_OUT_OF_RESOURCES  Menu is full.
**/
EFI_STATUS
EFIAPI
MenuAddItem (
  IN OUT MENU_CONTEXT  *Menu,
  IN     CHAR16        *Label,
  IN     CHAR16        *Description,
  IN     BOOLEAN       Selectable
  );

/**
  Draw the menu on the console.

  @param[in] Menu  Menu context to draw.
**/
VOID
EFIAPI
MenuDraw (
  IN MENU_CONTEXT *Menu
  );

/**
  Run the interactive menu loop. Returns when user presses Enter.

  @param[in,out] Menu  Menu context.

  @return  Index of the selected item (for single-select).
           For checkbox menus, check Items[i].IsSelected.
**/
UINTN
EFIAPI
MenuRun (
  IN OUT MENU_CONTEXT *Menu
  );

#endif // HW_MENU_H_
