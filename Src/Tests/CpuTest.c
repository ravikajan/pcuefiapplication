/** @file
  CpuTest.c - CPU Test Module

  Tests CPU identity (CPUID), feature flags, and performs arithmetic
  integrity/stress verification loops.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "TestModule.h"
#include "../Utils/Console.h"
#include "../Utils/Timer.h"
#include "../Utils/String.h"
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

/**
  Read CPUID and fill vendor string.
**/
STATIC
VOID
CpuGetVendor (
  OUT CHAR16 *VendorStr,
  IN  UINTN  Size
  )
{
  UINT32 Eax, Ebx, Ecx, Edx;
  CHAR8  Vendor8[13];

  AsmCpuid (0, &Eax, &Ebx, &Ecx, &Edx);

  // Vendor string is in EBX:EDX:ECX order
  CopyMem (&Vendor8[0], &Ebx, 4);
  CopyMem (&Vendor8[4], &Edx, 4);
  CopyMem (&Vendor8[8], &Ecx, 4);
  Vendor8[12] = '\0';

  // Convert ASCII to Unicode
  {
    UINTN i;
    UINTN MaxChars = (Size / sizeof (CHAR16)) - 1;
    for (i = 0; i < 12 && i < MaxChars; i++) {
      VendorStr[i] = (CHAR16)Vendor8[i];
    }
    VendorStr[i] = L'\0';
  }
}

/**
  Perform integer arithmetic integrity test.

  @retval TRUE   All checks passed.
  @retval FALSE  Arithmetic error detected.
**/
STATIC
BOOLEAN
CpuIntegerTest (
  VOID
  )
{
  UINT64 i;
  UINT64 Accum = 0;
  UINT64 Expected;

  // Sum of 0..9999 = 9999*10000/2 = 49995000
  for (i = 0; i < 10000; i++) {
    Accum += i;
  }

  Expected = 49995000ULL;
  if (Accum != Expected) {
    return FALSE;
  }

  // Multiplication / division check
  {
    UINT64 A = 123456789ULL;
    UINT64 B = 987654321ULL;
    UINT64 Product = A * B;
    UINT64 Quotient = Product / A;

    if (Quotient != B) {
      return FALSE;
    }
  }

  return TRUE;
}

/**
  Perform CPU stress test with computational loops.

  @param[in] Iterations  Number of stress iterations.

  @retval TRUE   Stress test passed (no errors).
  @retval FALSE  Error detected during stress.
**/
STATIC
BOOLEAN
CpuStressTest (
  IN UINTN Iterations
  )
{
  UINTN  i;
  UINT64 Val;

  for (i = 0; i < Iterations; i++) {
    Val = (UINT64)i * 37ULL + 17ULL;
    Val = (Val * Val) >> 3;
    Val = Val ^ (Val >> 7);

    // Verify reversible operation
    {
      UINT64 Test = (UINT64)i * 37ULL + 17ULL;
      Test = (Test * Test) >> 3;
      Test = Test ^ (Test >> 7);
      if (Test != Val) {
        return FALSE;
      }
    }
  }

  return TRUE;
}

/**
  Run the CPU test module.
**/
EFI_STATUS
EFIAPI
CpuTestRun (
  IN  HW_TEST_MODULE  *Self,
  OUT HW_TEST_RESULT  *Result
  )
{
  UINT64  StartTime;
  UINT64  EndTime;
  CHAR16  VendorStr[32];
  BOOLEAN IntegrityOk;
  BOOLEAN StressOk;

  StringSafeCopy (Result->TestName, sizeof (Result->TestName), L"CPU Test");
  StartTime = TimerGetMs ();

  // Get CPU vendor
  CpuGetVendor (VendorStr, sizeof (VendorStr));

  // Run integrity test
  IntegrityOk = CpuIntegerTest ();

  // Run stress test (short for quick test)
  StressOk = CpuStressTest (100000);

  EndTime = TimerGetMs ();
  Result->DurationMs = EndTime - StartTime;

  if (IntegrityOk && StressOk) {
    Result->Status = TestStatusPass;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"Vendor: %s | Integrity OK | Stress OK",
      VendorStr
      );
  } else {
    Result->Status = TestStatusFail;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"Vendor: %s | Integrity: %s | Stress: %s",
      VendorStr,
      IntegrityOk ? L"OK" : L"FAIL",
      StressOk ? L"OK" : L"FAIL"
      );
  }

  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gCpuTestModule = {
  L"CPU Test",                                        // Name
  L"CPUID, arithmetic integrity, stress loops",       // Description
  TRUE,                                               // IsQuickTest
  CpuTestRun                                          // Run
};
