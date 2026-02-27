/** @file
  BluetoothTest.c - Bluetooth Test Module

  Attempts to detect Bluetooth hardware via UEFI Bluetooth protocols.
  Reports presence/absence of Bluetooth Host Controller.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "TestModule.h"
#include "../Utils/Console.h"
#include "../Utils/Timer.h"
#include "../Utils/String.h"
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>

//
// Bluetooth HC Protocol GUID
// This may not be available on all UEFI firmware implementations.
// GUID: {A3EFC951-EF7C-493F-B5BA-B7E73E6EB2B5}
//
STATIC EFI_GUID mBluetoothHcProtocolGuid = {
  0xA3EFC951, 0xEF7C, 0x493F,
  { 0xB5, 0xBA, 0xB7, 0xE7, 0x3E, 0x6E, 0xB2, 0xB5 }
};

/**
  Run the Bluetooth test module.
**/
EFI_STATUS
EFIAPI
BluetoothTestRun (
  IN  HW_TEST_MODULE  *Self,
  OUT HW_TEST_RESULT  *Result
  )
{
  EFI_STATUS  Status;
  UINT64      StartTime;
  UINT64      EndTime;
  EFI_HANDLE  *HandleBuffer;
  UINTN       HandleCount;

  StringSafeCopy (Result->TestName, sizeof (Result->TestName), L"Bluetooth Test");
  StartTime = TimerGetMs ();

  // Try to locate Bluetooth HC protocol handles
  HandleCount = 0;
  HandleBuffer = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &mBluetoothHcProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  EndTime = TimerGetMs ();
  Result->DurationMs = EndTime - StartTime;

  if (!EFI_ERROR (Status) && HandleCount > 0) {
    Result->Status = TestStatusPass;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"%d Bluetooth controller(s) detected",
      HandleCount
      );

    if (HandleBuffer != NULL) {
      FreePool (HandleBuffer);
    }
  } else {
    // Bluetooth not detected — this is expected on many systems
    // where firmware doesn't expose BT via UEFI protocols
    Result->Status = TestStatusSkip;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"No Bluetooth UEFI protocol found (common)"
      );
  }

  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gBluetoothTestModule = {
  L"Bluetooth Test",                                  // Name
  L"Bluetooth HC protocol detection",                 // Description
  FALSE,                                              // IsQuickTest
  BluetoothTestRun                                    // Run
};
