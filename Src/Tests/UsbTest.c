/** @file
  UsbTest.c - USB Test Module

  Interactive USB port validation flow:
  - Lists currently visible USB endpoints
  - Asks user to plug a USB device into any port
  - Re-scans and validates plug-in detection

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
#include <Protocol/UsbIo.h>

#define USB_MAX_SIGNATURES    128
#define USB_LIST_MAX_LEN      220

typedef struct {
  UINTN   Count;
  UINTN   SignatureCount;
  UINT32  Signatures[USB_MAX_SIGNATURES];
  CHAR16  ListText[USB_LIST_MAX_LEN];
} USB_SNAPSHOT;

STATIC
VOID
WaitForEnter (
  VOID
  )
{
  EFI_INPUT_KEY Key;

  while (TRUE) {
    ConsoleWaitForKey (&Key);
    if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      return;
    }
  }
}

STATIC
BOOLEAN
SnapshotHasSignature (
  IN USB_SNAPSHOT *Snap,
  IN UINT32       Sig
  )
{
  UINTN i;

  for (i = 0; i < Snap->SignatureCount; i++) {
    if (Snap->Signatures[i] == Sig) {
      return TRUE;
    }
  }

  return FALSE;
}

STATIC
EFI_STATUS
CaptureUsbSnapshot (
  OUT USB_SNAPSHOT *Snap
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              *HandleBuffer;
  UINTN                   HandleCount;
  UINTN                   i;

  ZeroMem (Snap, sizeof (USB_SNAPSHOT));
  Snap->ListText[0] = L'\0';

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiUsbIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  if (EFI_ERROR (Status) || HandleCount == 0) {
    return EFI_NOT_FOUND;
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
      UINT32 Sig;

      Snap->Count++;
      Sig = ((UINT32)DevDesc.IdVendor << 16) | (UINT32)DevDesc.IdProduct;

      if (Snap->SignatureCount < USB_MAX_SIGNATURES) {
        Snap->Signatures[Snap->SignatureCount] = Sig;
        Snap->SignatureCount++;
      }

      if (Snap->Count <= 4) {
        CHAR16 Token[48];
        UnicodeSPrint (
          Token,
          sizeof (Token),
          (Snap->Count == 1) ? L"P%d(%04X:%04X)" : L", P%d(%04X:%04X)",
          Snap->Count,
          DevDesc.IdVendor,
          DevDesc.IdProduct
          );
        StringSafeCat (Snap->ListText, sizeof (Snap->ListText), Token);
      }
    }
  }

  FreePool (HandleBuffer);
  return (Snap->Count > 0) ? EFI_SUCCESS : EFI_NOT_FOUND;
}

STATIC
BOOLEAN
HasNewUsbDevice (
  IN USB_SNAPSHOT *Before,
  IN USB_SNAPSHOT *After
  )
{
  UINTN i;

  if (After->Count > Before->Count) {
    return TRUE;
  }

  for (i = 0; i < After->SignatureCount; i++) {
    if (!SnapshotHasSignature (Before, After->Signatures[i])) {
      return TRUE;
    }
  }

  return FALSE;
}

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
  USB_SNAPSHOT            Before;
  USB_SNAPSHOT            After;
  BOOLEAN                 Detected;

  StringSafeCopy (Result->TestName, sizeof (Result->TestName), L"USB Test");
  StartTime = TimerGetMs ();

  ConsoleClear ();
  ConsolePrintHeader ();
  ConsolePrintAt (2, 5, CON_COLOR_HIGHLIGHT, CON_BG_DEFAULT, L"USB Port Detection Test");
  ConsolePrintAt (2, 7, EFI_LIGHTGRAY, CON_BG_DEFAULT, L"Step 1: Keep current devices as baseline.");
  ConsolePrintAt (2, 8, EFI_LIGHTGRAY, CON_BG_DEFAULT, L"Press ENTER to scan current USB ports...");
  WaitForEnter ();

  Status = CaptureUsbSnapshot (&Before);

  if (EFI_ERROR (Status)) {
    EndTime = TimerGetMs ();
    Result->DurationMs = EndTime - StartTime;
    Result->Status = TestStatusSkip;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"Baseline scan failed: no USB devices visible in UEFI"
      );
    return EFI_SUCCESS;
  }

  ConsoleClear ();
  ConsolePrintHeader ();
  ConsolePrintAt (2, 5, CON_COLOR_INFO, CON_BG_DEFAULT, L"Baseline devices: %d", Before.Count);
  ConsolePrintAt (2, 6, EFI_DARKGRAY, CON_BG_DEFAULT, L"Ports: %s", Before.ListText[0] ? Before.ListText : L"(none)");
  ConsolePrintAt (2, 8, EFI_LIGHTGRAY, CON_BG_DEFAULT, L"Step 2: Plug a USB device into ANY port now.");
  ConsolePrintAt (2, 9, EFI_LIGHTGRAY, CON_BG_DEFAULT, L"Then press ENTER to rescan and validate port detection.");
  WaitForEnter ();

  Status = CaptureUsbSnapshot (&After);

  if (EFI_ERROR (Status)) {
    EndTime = TimerGetMs ();
    Result->DurationMs = EndTime - StartTime;
    Result->Status = TestStatusFail;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"Rescan failed after plug-in step | Baseline=%d",
      Before.Count
      );
    return EFI_SUCCESS;
  }

  Detected = HasNewUsbDevice (&Before, &After);

  EndTime = TimerGetMs ();
  Result->DurationMs = EndTime - StartTime;

  if (Detected) {
    Result->Status = TestStatusPass;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"USB port detect PASS | Before=%d After=%d | Before:%s | After:%s",
      Before.Count,
      After.Count,
      Before.ListText[0] ? Before.ListText : L"(none)",
      After.ListText[0] ? After.ListText : L"(none)"
      );
  } else {
    Result->Status = TestStatusFail;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"USB port detect FAIL | No new device after plug step | Before=%d After=%d | Issue: hub/port/driver/firmware filter",
      Before.Count,
      After.Count
      );
  }

  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gUsbTestModule = {
  L"USB Test",                                        // Name
  L"USB port list + plug-in detection validation",     // Description
  TRUE,                                               // IsQuickTest
  UsbTestRun                                          // Run
};
