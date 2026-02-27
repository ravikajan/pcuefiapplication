/** @file
  StorageTest.c - Storage Test Module

  Enumerates block I/O devices, reports media info (size, block size),
  and performs sequential read verification.

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
#include <Protocol/BlockIo.h>

/**
  Run the Storage test module.
**/
EFI_STATUS
EFIAPI
StorageTestRun (
  IN  HW_TEST_MODULE  *Self,
  OUT HW_TEST_RESULT  *Result
  )
{
  EFI_STATUS                Status;
  UINT64                    StartTime;
  UINT64                    EndTime;
  EFI_HANDLE                *HandleBuffer;
  UINTN                     HandleCount;
  UINTN                     i;
  UINTN                     DeviceCount;
  UINT64                    TotalSize;
  BOOLEAN                   ReadOk;
  CHAR16                    SizeStr[32];

  StringSafeCopy (Result->TestName, sizeof (Result->TestName), L"Storage Test");
  StartTime = TimerGetMs ();

  DeviceCount = 0;
  TotalSize = 0;
  ReadOk = TRUE;

  // Locate all Block I/O protocol handles
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  if (EFI_ERROR (Status) || HandleCount == 0) {
    EndTime = TimerGetMs ();
    Result->DurationMs = EndTime - StartTime;
    Result->Status = TestStatusFail;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"No block I/O devices found"
      );
    return EFI_SUCCESS;
  }

  // Enumerate each block device
  for (i = 0; i < HandleCount; i++) {
    EFI_BLOCK_IO_PROTOCOL *BlockIo;

    Status = gBS->HandleProtocol (
                    HandleBuffer[i],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **)&BlockIo
                    );

    if (EFI_ERROR (Status)) {
      continue;
    }

    // Skip logical partitions, only test whole devices
    if (!BlockIo->Media->LogicalPartition) {
      DeviceCount++;

      // Calculate device size
      {
        UINT64 MediaSize = MultU64x32 (
                             BlockIo->Media->LastBlock + 1,
                             BlockIo->Media->BlockSize
                             );
        TotalSize += MediaSize;
      }

      // Read first block to verify I/O
      if (BlockIo->Media->MediaPresent) {
        UINT8 *ReadBuffer = AllocatePool (BlockIo->Media->BlockSize);
        if (ReadBuffer != NULL) {
          Status = BlockIo->ReadBlocks (
                              BlockIo,
                              BlockIo->Media->MediaId,
                              0,
                              BlockIo->Media->BlockSize,
                              ReadBuffer
                              );
          if (EFI_ERROR (Status)) {
            ReadOk = FALSE;
          }
          FreePool (ReadBuffer);
        }
      }
    }
  }

  FreePool (HandleBuffer);

  EndTime = TimerGetMs ();
  Result->DurationMs = EndTime - StartTime;

  StringFormatSize (TotalSize, SizeStr, sizeof (SizeStr));

  if (DeviceCount > 0 && ReadOk) {
    Result->Status = TestStatusPass;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"%d devices | Total: %s | Read OK",
      DeviceCount, SizeStr
      );
  } else if (DeviceCount > 0) {
    Result->Status = TestStatusFail;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"%d devices | Total: %s | Read FAILED",
      DeviceCount, SizeStr
      );
  } else {
    Result->Status = TestStatusFail;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"No physical devices detected"
      );
  }

  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gStorageTestModule = {
  L"Storage Test",                                    // Name
  L"Block I/O enumeration, media info, read test",    // Description
  TRUE,                                               // IsQuickTest
  StorageTestRun                                      // Run
};
