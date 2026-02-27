/** @file
  MemoryTest.c - Memory Test Module

  Queries the UEFI memory map, validates region sizes,
  and runs walking-bit pattern tests on allocated buffers.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "TestModule.h"
#include "../Utils/Console.h"
#include "../Utils/Timer.h"
#include "../Utils/String.h"
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>

// Size of buffer to allocate for walking-bit tests (1 MB)
#define MEM_TEST_BUFFER_SIZE    (1024 * 1024)

// Pattern test iterations
#define MEM_TEST_PATTERNS       4

/**
  Get total system memory from the UEFI memory map.

  @param[out] TotalPages      Total conventional memory pages.
  @param[out] AvailablePages  Available (free) memory pages.
  @param[out] RegionCount     Number of memory map entries.

  @retval EFI_SUCCESS  Memory map queried successfully.
**/
STATIC
EFI_STATUS
MemGetSystemMemory (
  OUT UINT64 *TotalPages,
  OUT UINT64 *AvailablePages,
  OUT UINTN  *RegionCount
  )
{
  EFI_STATUS             Status;
  EFI_MEMORY_DESCRIPTOR  *MemMap;
  UINTN                  MemMapSize;
  UINTN                  MapKey;
  UINTN                  DescSize;
  UINT32                 DescVer;
  UINT8                  *Ptr;
  UINT8                  *End;

  *TotalPages = 0;
  *AvailablePages = 0;
  *RegionCount = 0;

  MemMapSize = 0;
  MemMap = NULL;

  // First call to get required size
  Status = gBS->GetMemoryMap (&MemMapSize, MemMap, &MapKey, &DescSize, &DescVer);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return Status;
  }

  // Add extra space for the allocation itself
  MemMapSize += 2 * DescSize;
  MemMap = AllocatePool (MemMapSize);
  if (MemMap == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->GetMemoryMap (&MemMapSize, MemMap, &MapKey, &DescSize, &DescVer);
  if (EFI_ERROR (Status)) {
    FreePool (MemMap);
    return Status;
  }

  // Walk the memory map
  Ptr = (UINT8 *)MemMap;
  End = Ptr + MemMapSize;

  while (Ptr < End) {
    EFI_MEMORY_DESCRIPTOR *Desc = (EFI_MEMORY_DESCRIPTOR *)Ptr;

    (*RegionCount)++;

    switch (Desc->Type) {
      case EfiConventionalMemory:
        *AvailablePages += Desc->NumberOfPages;
        *TotalPages += Desc->NumberOfPages;
        break;
      case EfiLoaderCode:
      case EfiLoaderData:
      case EfiBootServicesCode:
      case EfiBootServicesData:
      case EfiRuntimeServicesCode:
      case EfiRuntimeServicesData:
      case EfiACPIReclaimMemory:
        *TotalPages += Desc->NumberOfPages;
        break;
      default:
        break;
    }

    Ptr += DescSize;
  }

  FreePool (MemMap);
  return EFI_SUCCESS;
}

/**
  Run walking-bit pattern test on a memory buffer.

  @param[in] Buffer  Buffer to test.
  @param[in] Size    Size of buffer in bytes.

  @retval TRUE   All patterns verified.
  @retval FALSE  Pattern mismatch detected.
**/
STATIC
BOOLEAN
MemWalkingBitTest (
  IN UINT8  *Buffer,
  IN UINTN  Size
  )
{
  UINTN i;
  UINT8 Patterns[MEM_TEST_PATTERNS] = { 0xAA, 0x55, 0xFF, 0x00 };
  UINTN p;

  for (p = 0; p < MEM_TEST_PATTERNS; p++) {
    // Write pattern
    SetMem (Buffer, Size, Patterns[p]);

    // Verify pattern
    for (i = 0; i < Size; i++) {
      if (Buffer[i] != Patterns[p]) {
        return FALSE;
      }
    }
  }

  // Walking 1s test (on first 4KB to keep it quick)
  {
    UINTN TestSize = (Size < 4096) ? Size : 4096;
    UINT8 Bit;

    for (Bit = 0; Bit < 8; Bit++) {
      UINT8 Val = (UINT8)(1 << Bit);
      SetMem (Buffer, TestSize, Val);

      for (i = 0; i < TestSize; i++) {
        if (Buffer[i] != Val) {
          return FALSE;
        }
      }
    }
  }

  return TRUE;
}

/**
  Run the Memory test module.
**/
EFI_STATUS
EFIAPI
MemoryTestRun (
  IN  HW_TEST_MODULE  *Self,
  OUT HW_TEST_RESULT  *Result
  )
{
  UINT64   StartTime;
  UINT64   EndTime;
  UINT64   TotalPages;
  UINT64   AvailablePages;
  UINTN    RegionCount;
  UINT8    *TestBuffer;
  BOOLEAN  PatternOk;
  CHAR16   TotalStr[32];
  CHAR16   AvailStr[32];

  StringSafeCopy (Result->TestName, sizeof (Result->TestName), L"Memory Test");
  StartTime = TimerGetMs ();

  // Query memory map
  TotalPages = 0;
  AvailablePages = 0;
  RegionCount = 0;
  MemGetSystemMemory (&TotalPages, &AvailablePages, &RegionCount);

  StringFormatSize (TotalPages * 4096ULL, TotalStr, sizeof (TotalStr));
  StringFormatSize (AvailablePages * 4096ULL, AvailStr, sizeof (AvailStr));

  // Allocate test buffer
  TestBuffer = AllocatePool (MEM_TEST_BUFFER_SIZE);
  if (TestBuffer == NULL) {
    Result->Status = TestStatusFail;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"Total: %s | Failed to allocate test buffer",
      TotalStr
      );
    EndTime = TimerGetMs ();
    Result->DurationMs = EndTime - StartTime;
    return EFI_SUCCESS;
  }

  // Run pattern test
  PatternOk = MemWalkingBitTest (TestBuffer, MEM_TEST_BUFFER_SIZE);

  FreePool (TestBuffer);

  EndTime = TimerGetMs ();
  Result->DurationMs = EndTime - StartTime;

  if (PatternOk) {
    Result->Status = TestStatusPass;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"Total: %s | Free: %s | %d regions | Pattern OK",
      TotalStr, AvailStr, RegionCount
      );
  } else {
    Result->Status = TestStatusFail;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"Total: %s | Pattern FAILED",
      TotalStr
      );
  }

  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gMemoryTestModule = {
  L"Memory Test",                                     // Name
  L"Memory map, walking-bit patterns, allocation",    // Description
  TRUE,                                               // IsQuickTest
  MemoryTestRun                                       // Run
};
