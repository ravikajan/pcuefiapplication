/** @file
  TestRunner.c - Test Runner & Orchestration Implementation

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "TestRunner.h"
#include "../Utils/Timer.h"
#include <Library/BaseMemoryLib.h>

// External module descriptors from test modules
extern HW_TEST_MODULE gCpuTestModule;
extern HW_TEST_MODULE gMemoryTestModule;
extern HW_TEST_MODULE gStorageTestModule;
extern HW_TEST_MODULE gDisplayTestModule;
extern HW_TEST_MODULE gNetworkTestModule;
extern HW_TEST_MODULE gUsbTestModule;
extern HW_TEST_MODULE gBluetoothTestModule;

// Registered modules
STATIC HW_TEST_MODULE *mModules[HW_TEST_MAX_MODULES];
STATIC UINTN           mModuleCount = 0;

/**
  Register a test module.
**/
STATIC
VOID
RegisterModule (
  IN HW_TEST_MODULE *Module
  )
{
  if (mModuleCount < HW_TEST_MAX_MODULES) {
    mModules[mModuleCount] = Module;
    mModuleCount++;
  }
}

VOID
EFIAPI
TestRunnerInit (
  VOID
  )
{
  mModuleCount = 0;

  // Register all test modules in execution order
  RegisterModule (&gCpuTestModule);
  RegisterModule (&gMemoryTestModule);
  RegisterModule (&gStorageTestModule);
  RegisterModule (&gDisplayTestModule);
  RegisterModule (&gNetworkTestModule);
  RegisterModule (&gUsbTestModule);
  RegisterModule (&gBluetoothTestModule);
}

UINTN
EFIAPI
TestRunnerGetModuleCount (
  VOID
  )
{
  return mModuleCount;
}

HW_TEST_MODULE *
EFIAPI
TestRunnerGetModule (
  IN UINTN Index
  )
{
  if (Index < mModuleCount) {
    return mModules[Index];
  }
  return NULL;
}

/**
  Internal: run a single module and fill result.
**/
STATIC
VOID
RunSingleModule (
  IN  HW_TEST_MODULE             *Module,
  OUT HW_TEST_RESULT             *Result,
  IN  UINTN                      CurrentIndex,
  IN  UINTN                      TotalCount,
  IN  HW_TEST_PROGRESS_CALLBACK  ProgressCb    OPTIONAL
  )
{
  ZeroMem (Result, sizeof (HW_TEST_RESULT));

  // Notify UI that test is starting
  if (ProgressCb != NULL) {
    ProgressCb (CurrentIndex, TotalCount, Module->Name, TestStatusSkip);
  }

  // Run the test
  Module->Run (Module, Result);

  // Notify UI with final status
  if (ProgressCb != NULL) {
    ProgressCb (CurrentIndex, TotalCount, Module->Name, Result->Status);
  }
}

VOID
EFIAPI
TestRunnerRunAll (
  OUT HW_TEST_RESULT              *Results,
  OUT UINTN                       *ResultCount,
  IN  HW_TEST_PROGRESS_CALLBACK   ProgressCb    OPTIONAL
  )
{
  UINTN i;

  *ResultCount = 0;

  for (i = 0; i < mModuleCount; i++) {
    RunSingleModule (mModules[i], &Results[i], i, mModuleCount, ProgressCb);
    (*ResultCount)++;
  }
}

VOID
EFIAPI
TestRunnerRunQuick (
  OUT HW_TEST_RESULT              *Results,
  OUT UINTN                       *ResultCount,
  IN  HW_TEST_PROGRESS_CALLBACK   ProgressCb    OPTIONAL
  )
{
  UINTN i;
  UINTN QuickCount;
  UINTN RunIndex;

  // Count quick test modules
  QuickCount = 0;
  for (i = 0; i < mModuleCount; i++) {
    if (mModules[i]->IsQuickTest) {
      QuickCount++;
    }
  }

  *ResultCount = 0;
  RunIndex = 0;

  for (i = 0; i < mModuleCount; i++) {
    if (mModules[i]->IsQuickTest) {
      RunSingleModule (mModules[i], &Results[RunIndex], RunIndex, QuickCount, ProgressCb);
      RunIndex++;
      (*ResultCount)++;
    }
  }
}

VOID
EFIAPI
TestRunnerRunSelected (
  IN  UINTN                       *Indices,
  IN  UINTN                       IndexCount,
  OUT HW_TEST_RESULT              *Results,
  OUT UINTN                       *ResultCount,
  IN  HW_TEST_PROGRESS_CALLBACK   ProgressCb    OPTIONAL
  )
{
  UINTN i;

  *ResultCount = 0;

  for (i = 0; i < IndexCount; i++) {
    UINTN Idx = Indices[i];
    if (Idx < mModuleCount) {
      RunSingleModule (mModules[Idx], &Results[i], i, IndexCount, ProgressCb);
      (*ResultCount)++;
    }
  }
}
