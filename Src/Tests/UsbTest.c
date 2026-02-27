/** @file
  UsbTest.c - USB Test Module

  Enumerates USB controllers and connected devices via USB I/O Protocol,
  reports VID/PID, class, and device descriptor info.

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
#include <Protocol/UsbIo.h>

/**
  Run the USB test module.
**/
EFI_STATUS
EFIAPI
UsbTestRun (
  IN  HW_TEST_MODULE  *Self,
  OUT HW_TEST_RESULT  *Result
  )
{
  EFI_STATUS              Status;
  UINT64                  StartTime;
  UINT64                  EndTime;
  EFI_HANDLE              *HandleBuffer;
  UINTN                   HandleCount;
  UINTN                   i;
  UINTN                   DeviceCount;
  UINT16                  FirstVid;
  UINT16                  FirstPid;
  BOOLEAN                 HasFirstDevice;

  StringSafeCopy (Result->TestName, sizeof (Result->TestName), L"USB Test");
  StartTime = TimerGetMs ();

  DeviceCount = 0;
  HasFirstDevice = FALSE;
  FirstVid = 0;
  FirstPid = 0;

  // Locate all USB I/O protocol handles
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiUsbIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  if (EFI_ERROR (Status) || HandleCount == 0) {
    EndTime = TimerGetMs ();
    Result->DurationMs = EndTime - StartTime;
    Result->Status = TestStatusSkip;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"No USB devices detected"
      );
    return EFI_SUCCESS;
  }

  for (i = 0; i < HandleCount; i++) {
    EFI_USB_IO_PROTOCOL       *UsbIo;
    EFI_USB_DEVICE_DESCRIPTOR DevDesc;

    Status = gBS->HandleProtocol (
                    HandleBuffer[i],
                    &gEfiUsbIoProtocolGuid,
                    (VOID **)&UsbIo
                    );

    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = UsbIo->UsbGetDeviceDescriptor (UsbIo, &DevDesc);
    if (!EFI_ERROR (Status)) {
      DeviceCount++;

      if (!HasFirstDevice) {
        FirstVid = DevDesc.IdVendor;
        FirstPid = DevDesc.IdProduct;
        HasFirstDevice = TRUE;
      }
    }
  }

  FreePool (HandleBuffer);

  EndTime = TimerGetMs ();
  Result->DurationMs = EndTime - StartTime;

  if (DeviceCount > 0) {
    Result->Status = TestStatusPass;
    if (HasFirstDevice) {
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"%d USB devices | First: VID=%04X PID=%04X",
        DeviceCount, FirstVid, FirstPid
        );
    } else {
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"%d USB devices enumerated",
        DeviceCount
        );
    }
  } else {
    Result->Status = TestStatusSkip;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"USB I/O handles found but no valid descriptors"
      );
  }

  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gUsbTestModule = {
  L"USB Test",                                        // Name
  L"USB device enumeration, VID/PID, descriptors",    // Description
  TRUE,                                               // IsQuickTest
  UsbTestRun                                          // Run
};
