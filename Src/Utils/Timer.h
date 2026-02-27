/** @file
  Timer.h - Timer and Delay Utilities

  Provides timing functions using UEFI Boot Services for measuring
  test durations and implementing delays.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_TIMER_H_
#define HW_TIMER_H_

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>

/**
  Get the current timestamp in milliseconds (since boot).

  @return  Current time in milliseconds.
**/
UINT64
EFIAPI
TimerGetMs (
  VOID
  );

/**
  Delay execution for the specified number of milliseconds.

  @param[in] Milliseconds  Number of milliseconds to wait.
**/
VOID
EFIAPI
TimerDelayMs (
  IN UINTN Milliseconds
  );

/**
  Delay execution for the specified number of microseconds.

  @param[in] Microseconds  Number of microseconds to wait.
**/
VOID
EFIAPI
TimerDelayUs (
  IN UINTN Microseconds
  );

#endif // HW_TIMER_H_
