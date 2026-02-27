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

  // Perform color fill tests (brief — just verify Blt works)
  {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL TestColors[4];
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

      // Brief delay so the color is visible
      TimerDelayMs (200);
    }
  }

  // Restore console (clear screen)
  {
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black;
    Black.Blue = 0; Black.Green = 0; Black.Red = 0; Black.Reserved = 0;
    Gop->Blt (Gop, &Black, EfiBltVideoFill, 0, 0, 0, 0,
              CurrentWidth, CurrentHeight, 0);
  }

  EndTime = TimerGetMs ();
  Result->DurationMs = EndTime - StartTime;

  Result->Status = TestStatusPass;
  UnicodeSPrint (
    Result->Details, sizeof (Result->Details),
    L"Current: %dx%d | Max: %dx%d | %d modes | Blt OK",
    CurrentWidth, CurrentHeight,
    BestWidth, BestHeight,
    MaxMode
    );

  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gDisplayTestModule = {
  L"Display Test",                                    // Name
  L"GOP modes, resolution info, color fill test",     // Description
  FALSE,                                              // IsQuickTest (visual)
  DisplayTestRun                                      // Run
};
