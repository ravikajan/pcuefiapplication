/** @file
  Timer.c - Timer and Delay Utilities Implementation

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Timer.h"
#include <Library/UefiRuntimeServicesTableLib.h>

// We use gBS->Stall() which takes microseconds.
// For millisecond timing we approximate using a loop around GetTime.
// UEFI doesn't provide a high-res monotonic clock universally,
// so we use Stall-based timing.

STATIC UINT64 mBootTimestampMs = 0;
STATIC BOOLEAN mTimerInitialized = FALSE;

/**
  Initialize the timer subsystem.
  Uses a simple counter incremented on each call, calibrated by Stall.
**/
STATIC
VOID
TimerInit (
  VOID
  )
{
  if (!mTimerInitialized) {
    mBootTimestampMs = 0;
    mTimerInitialized = TRUE;
  }
}

UINT64
EFIAPI
TimerGetMs (
  VOID
  )
{
  EFI_TIME  Time;
  EFI_STATUS Status;

  TimerInit ();

  // Use runtime services to get current time
  Status = gRT->GetTime (&Time, NULL);
  if (EFI_ERROR (Status)) {
    // Fallback: return accumulated stall-based estimate
    return mBootTimestampMs;
  }

  // Convert to a millisecond timestamp (not epoch, but good for duration measurement)
  {
    UINT64 Ms;
    Ms = (UINT64)Time.Hour * 3600000ULL +
         (UINT64)Time.Minute * 60000ULL +
         (UINT64)Time.Second * 1000ULL +
         (UINT64)(Time.Nanosecond / 1000000ULL);
    return Ms;
  }
}

VOID
EFIAPI
TimerDelayMs (
  IN UINTN Milliseconds
  )
{
  // gBS->Stall takes microseconds
  gBS->Stall (Milliseconds * 1000);
  mBootTimestampMs += Milliseconds;
}

VOID
EFIAPI
TimerDelayUs (
  IN UINTN Microseconds
  )
{
  gBS->Stall (Microseconds);
  mBootTimestampMs += Microseconds / 1000;
}
