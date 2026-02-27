/** @file
  Progress.h - Progress Screen

  Displays real-time test execution progress with test names,
  progress bar, and per-component status badges.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_PROGRESS_H_
#define HW_PROGRESS_H_

#include <Uefi.h>
#include "../Tests/TestModule.h"

/**
  Initialize the progress screen.

  @param[in] TotalTests  Total number of tests to be run.
**/
VOID
EFIAPI
ProgressInit (
  IN UINTN TotalTests
  );

/**
  Update the progress screen with current test status.

  @param[in] CurrentTest  Current test index (0-based).
  @param[in] TotalTests   Total number of tests.
  @param[in] TestName     Name of the current test.
  @param[in] Status       Status of the test (running, pass, fail).
**/
VOID
EFIAPI
ProgressUpdate (
  IN UINTN            CurrentTest,
  IN UINTN            TotalTests,
  IN CHAR16           *TestName,
  IN HW_TEST_STATUS   Status
  );

/**
  Show completion message on progress screen.

  @param[in] TotalTests  Total tests that were run.
  @param[in] Passed      Number that passed.
  @param[in] Failed      Number that failed.
**/
VOID
EFIAPI
ProgressComplete (
  IN UINTN TotalTests,
  IN UINTN Passed,
  IN UINTN Failed
  );

#endif // HW_PROGRESS_H_
