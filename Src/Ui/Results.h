/** @file
  Results.h - Results Summary Screen

  Displays tabular test results with PASS/FAIL/SKIP indicators.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_RESULTS_H_
#define HW_RESULTS_H_

#include <Uefi.h>
#include "../Tests/TestModule.h"

/**
  Display the results summary screen.

  @param[in] Results      Array of test results.
  @param[in] ResultCount  Number of results in the array.
**/
VOID
EFIAPI
ResultsShow (
  IN HW_TEST_RESULT  *Results,
  IN UINTN           ResultCount
  );

#endif // HW_RESULTS_H_
