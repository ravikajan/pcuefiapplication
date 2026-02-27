/** @file
  Graphics.h - Graphics Output Protocol UI Layer

  Provides GOP framebuffer access, pixel drawing, rectangle fills,
  color palette, and bitmap font text rendering.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_GRAPHICS_H_
#define HW_GRAPHICS_H_

#include <Uefi.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/UefiBootServicesTableLib.h>

// Color palette (BGRR format for EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
#define GFX_COLOR_BLACK     {0x00, 0x00, 0x00, 0x00}
#define GFX_COLOR_WHITE     {0xFF, 0xFF, 0xFF, 0x00}
#define GFX_COLOR_RED       {0x00, 0x00, 0xDD, 0x00}
#define GFX_COLOR_GREEN     {0x00, 0xBB, 0x00, 0x00}
#define GFX_COLOR_BLUE      {0xDD, 0x44, 0x00, 0x00}
#define GFX_COLOR_YELLOW    {0x00, 0xDD, 0xDD, 0x00}
#define GFX_COLOR_CYAN      {0xDD, 0xDD, 0x00, 0x00}
#define GFX_COLOR_DARKGRAY  {0x44, 0x44, 0x44, 0x00}
#define GFX_COLOR_LIGHTGRAY {0xAA, 0xAA, 0xAA, 0x00}
#define GFX_COLOR_ORANGE    {0x00, 0x88, 0xFF, 0x00}
#define GFX_COLOR_DARKBLUE  {0x88, 0x22, 0x00, 0x00}

/**
  Initialize the Graphics Output Protocol.

  @param[out] Gop  Pointer to receive the GOP protocol.

  @retval EFI_SUCCESS      GOP initialized successfully.
  @retval EFI_NOT_FOUND    GOP not available.
**/
EFI_STATUS
EFIAPI
GfxInit (
  OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Gop
  );

/**
  Get the current framebuffer resolution.

  @param[out] Width   Horizontal resolution.
  @param[out] Height  Vertical resolution.
**/
VOID
EFIAPI
GfxGetResolution (
  OUT UINT32 *Width,
  OUT UINT32 *Height
  );

/**
  Fill a rectangle on the framebuffer.

  @param[in] X       Left position.
  @param[in] Y       Top position.
  @param[in] Width   Rectangle width.
  @param[in] Height  Rectangle height.
  @param[in] Color   Fill color.
**/
VOID
EFIAPI
GfxFillRect (
  IN UINT32                         X,
  IN UINT32                         Y,
  IN UINT32                         Width,
  IN UINT32                         Height,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *Color
  );

/**
  Draw a horizontal progress bar.

  @param[in] X         Left position.
  @param[in] Y         Top position.
  @param[in] Width     Total bar width.
  @param[in] Height    Bar height.
  @param[in] Percent   Fill percentage (0-100).
  @param[in] FgColor   Foreground (fill) color.
  @param[in] BgColor   Background color.
**/
VOID
EFIAPI
GfxDrawProgressBar (
  IN UINT32                         X,
  IN UINT32                         Y,
  IN UINT32                         Width,
  IN UINT32                         Height,
  IN UINT32                         Percent,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *FgColor,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *BgColor
  );

/**
  Clear the entire framebuffer with a color.

  @param[in] Color  Fill color.
**/
VOID
EFIAPI
GfxClearScreen (
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color
  );

/**
  Check if GOP is available.

  @retval TRUE   GOP is available.
  @retval FALSE  GOP is not available.
**/
BOOLEAN
EFIAPI
GfxIsAvailable (
  VOID
  );

#endif // HW_GRAPHICS_H_
