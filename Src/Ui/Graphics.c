/** @file
  Graphics.c - Graphics Output Protocol UI Layer Implementation

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Graphics.h"
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>

STATIC EFI_GRAPHICS_OUTPUT_PROTOCOL *mGop = NULL;

EFI_STATUS
EFIAPI
GfxInit (
  OUT EFI_GRAPHICS_OUTPUT_PROTOCOL **Gop
  )
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  (VOID **)&mGop
                  );

  if (EFI_ERROR (Status)) {
    mGop = NULL;
    if (Gop != NULL) {
      *Gop = NULL;
    }
    return Status;
  }

  // Try to set best available mode (highest resolution)
  {
    UINT32 MaxMode = mGop->Mode->MaxMode;
    UINT32 BestMode = mGop->Mode->Mode;
    UINT32 BestPixels = 0;
    UINT32 i;

    for (i = 0; i < MaxMode; i++) {
      EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
      UINTN InfoSize;

      Status = mGop->QueryMode (mGop, i, &InfoSize, &Info);
      if (!EFI_ERROR (Status)) {
        UINT32 Pixels = Info->HorizontalResolution * Info->VerticalResolution;
        if (Pixels > BestPixels) {
          BestPixels = Pixels;
          BestMode = i;
        }
      }
    }

    if (BestMode != mGop->Mode->Mode) {
      mGop->SetMode (mGop, BestMode);
    }
  }

  if (Gop != NULL) {
    *Gop = mGop;
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
GfxGetResolution (
  OUT UINT32 *Width,
  OUT UINT32 *Height
  )
{
  if (mGop != NULL && mGop->Mode != NULL && mGop->Mode->Info != NULL) {
    *Width = mGop->Mode->Info->HorizontalResolution;
    *Height = mGop->Mode->Info->VerticalResolution;
  } else {
    *Width = 80;
    *Height = 25;
  }
}

VOID
EFIAPI
GfxFillRect (
  IN UINT32                         X,
  IN UINT32                         Y,
  IN UINT32                         Width,
  IN UINT32                         Height,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *Color
  )
{
  if (mGop == NULL) {
    return;
  }

  mGop->Blt (
          mGop,
          Color,
          EfiBltVideoFill,
          0, 0,
          X, Y,
          Width, Height,
          0
          );
}

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
  )
{
  UINT32 FillWidth;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL BorderColor;

  if (mGop == NULL) {
    return;
  }

  if (Percent > 100) {
    Percent = 100;
  }

  // Draw border (1px lighter outline)
  BorderColor.Blue = 0x88;
  BorderColor.Green = 0x88;
  BorderColor.Red = 0x88;
  BorderColor.Reserved = 0x00;

  // Top border
  GfxFillRect (X, Y, Width, 1, &BorderColor);
  // Bottom border
  GfxFillRect (X, Y + Height - 1, Width, 1, &BorderColor);
  // Left border
  GfxFillRect (X, Y, 1, Height, &BorderColor);
  // Right border
  GfxFillRect (X + Width - 1, Y, 1, Height, &BorderColor);

  // Background fill (inner area)
  GfxFillRect (X + 2, Y + 2, Width - 4, Height - 4, BgColor);

  // Foreground fill based on percentage
  FillWidth = ((Width - 4) * Percent) / 100;
  if (FillWidth > 0) {
    GfxFillRect (X + 2, Y + 2, FillWidth, Height - 4, FgColor);
  }
}

VOID
EFIAPI
GfxClearScreen (
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color
  )
{
  UINT32 Width;
  UINT32 Height;

  if (mGop == NULL) {
    return;
  }

  GfxGetResolution (&Width, &Height);
  GfxFillRect (0, 0, Width, Height, Color);
}

BOOLEAN
EFIAPI
GfxIsAvailable (
  VOID
  )
{
  return (mGop != NULL);
}
