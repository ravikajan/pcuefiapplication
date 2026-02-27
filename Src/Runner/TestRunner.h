/** @file
  TestRunner.h - Test Runner & Orchestration

  Manages test module registration and execution with progress callbacks.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_TEST_RUNNER_H_
#define HW_TEST_RUNNER_H_

#include <Uefi.h>
#include "../Tests/TestModule.h"

/**
  Initialize the test runner and register all test modules.
**/
VOID
EFIAPI
TestRunnerInit (
  VOID
  );

/**
  Get the total number of registered test modules.

  @return Number of registered modules.
**/
UINTN
EFIAPI
TestRunnerGetModuleCount (
  VOID
  );

/**
  Get a specific test module by index.

  @param[in] Index  Module index.

  @return  Pointer to the module, or NULL if index is out of range.
**/
HW_TEST_MODULE *
EFIAPI
TestRunnerGetModule (
  IN UINTN Index
  );

/**
  Run all registered test modules.

  @param[out] Results        Array to receive test results (must hold GetModuleCount elements).
  @param[out] ResultCount    Number of results written.
  @param[in]  ProgressCb     Optional progress callback.
**/
VOID
EFIAPI
TestRunnerRunAll (
  OUT HW_TEST_RESULT              *Results,
  OUT UINTN                       *ResultCount,
  IN  HW_TEST_PROGRESS_CALLBACK   ProgressCb    OPTIONAL
  );

/**
  Run only quick-test modules.

  @param[out] Results        Array to receive test results.
  @param[out] ResultCount    Number of results written.
  @param[in]  ProgressCb     Optional progress callback.
**/
VOID
EFIAPI
TestRunnerRunQuick (
  OUT HW_TEST_RESULT              *Results,
  OUT UINTN                       *ResultCount,
  IN  HW_TEST_PROGRESS_CALLBACK   ProgressCb    OPTIONAL
  );

/**
  Run selected test modules (by index array).

  @param[in]  Indices        Array of module indices to run.
  @param[in]  IndexCount     Number of indices.
  @param[out] Results        Array to receive test results.
  @param[out] ResultCount    Number of results written.
  @param[in]  ProgressCb     Optional progress callback.
**/
VOID
EFIAPI
TestRunnerRunSelected (
  IN  UINTN                       *Indices,
  IN  UINTN                       IndexCount,
  OUT HW_TEST_RESULT              *Results,
  OUT UINTN                       *ResultCount,
  IN  HW_TEST_PROGRESS_CALLBACK   ProgressCb    OPTIONAL
  );

#endif // HW_TEST_RUNNER_H_
