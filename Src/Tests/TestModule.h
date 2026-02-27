/** @file
  TestModule.h - Hardware Test Module Interface

  Defines the common interface for all hardware test modules.
  Each test module implements the Run() function and provides
  metadata (name, description, quick-test eligibility).

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_TEST_MODULE_H_
#define HW_TEST_MODULE_H_

#include <Uefi.h>
#include <Library/UefiLib.h>

// Maximum lengths for strings
#define HW_TEST_NAME_MAX        64
#define HW_TEST_DETAIL_MAX      512
#define HW_TEST_MAX_MODULES     16

// Test result status
typedef enum {
  TestStatusPass = 0,
  TestStatusFail,
  TestStatusSkip,
  TestStatusError
} HW_TEST_STATUS;

// Structured result from a single test
typedef struct {
  CHAR16          TestName[HW_TEST_NAME_MAX];
  HW_TEST_STATUS  Status;
  CHAR16          Details[HW_TEST_DETAIL_MAX];
  UINT64          DurationMs;
} HW_TEST_RESULT;

// Forward declaration
typedef struct _HW_TEST_MODULE HW_TEST_MODULE;

// Test run function signature
typedef EFI_STATUS (EFIAPI *HW_TEST_RUN_FUNC)(
  IN  HW_TEST_MODULE   *Self,
  OUT HW_TEST_RESULT    *Result
  );

// Hardware test module descriptor
struct _HW_TEST_MODULE {
  CHAR16              Name[HW_TEST_NAME_MAX];
  CHAR16              Description[HW_TEST_DETAIL_MAX];
  BOOLEAN             IsQuickTest;      // TRUE = included in Quick Test profile
  HW_TEST_RUN_FUNC    Run;
};

// Progress callback: called by test runner to update UI
typedef VOID (EFIAPI *HW_TEST_PROGRESS_CALLBACK)(
  IN UINTN            CurrentTest,
  IN UINTN            TotalTests,
  IN CHAR16           *TestName,
  IN HW_TEST_STATUS   Status
  );

#endif // HW_TEST_MODULE_H_
