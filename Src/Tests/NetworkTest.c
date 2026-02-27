/** @file
  NetworkTest.c - Network Test Module

  Enumerates network interfaces via Simple Network Protocol,
  reports MAC address and link status.

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
#include <Protocol/SimpleNetwork.h>

/**
  Run the Network test module.
**/
EFI_STATUS
EFIAPI
NetworkTestRun (
  IN  HW_TEST_MODULE  *Self,
  OUT HW_TEST_RESULT  *Result
  )
{
  EFI_STATUS                    Status;
  UINT64                        StartTime;
  UINT64                        EndTime;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         HandleCount;
  UINTN                         i;
  UINTN                         NicCount;
  BOOLEAN                       AnyLinkUp;

  StringSafeCopy (Result->TestName, sizeof (Result->TestName), L"Network Test");
  StartTime = TimerGetMs ();

  NicCount = 0;
  AnyLinkUp = FALSE;

  // Locate all Simple Network Protocol handles
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleNetworkProtocolGuid,
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
      L"No network interfaces detected"
      );
    return EFI_SUCCESS;
  }

  for (i = 0; i < HandleCount; i++) {
    EFI_SIMPLE_NETWORK_PROTOCOL *Snp;

    Status = gBS->HandleProtocol (
                    HandleBuffer[i],
                    &gEfiSimpleNetworkProtocolGuid,
                    (VOID **)&Snp
                    );

    if (EFI_ERROR (Status)) {
      continue;
    }

    NicCount++;

    // Initialize the NIC if not already
    if (Snp->Mode->State == EfiSimpleNetworkStopped) {
      Snp->Start (Snp);
    }
    if (Snp->Mode->State == EfiSimpleNetworkStarted) {
      Snp->Initialize (Snp, 0, 0);
    }

    // Check media / link status
    if (Snp->Mode->MediaPresentSupported && Snp->Mode->MediaPresent) {
      AnyLinkUp = TRUE;
    }
  }

  // Report MAC address from first NIC
  {
    EFI_SIMPLE_NETWORK_PROTOCOL *Snp;

    Status = gBS->HandleProtocol (
                    HandleBuffer[0],
                    &gEfiSimpleNetworkProtocolGuid,
                    (VOID **)&Snp
                    );

    if (!EFI_ERROR (Status) && Snp->Mode->HwAddressSize >= 6) {
      UINT8 *Mac = Snp->Mode->CurrentAddress.Addr;

      EndTime = TimerGetMs ();
      Result->DurationMs = EndTime - StartTime;

      Result->Status = TestStatusPass;
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"%d NICs | MAC: %02X:%02X:%02X:%02X:%02X:%02X | Link: %s",
        NicCount,
        Mac[0], Mac[1], Mac[2], Mac[3], Mac[4], Mac[5],
        AnyLinkUp ? L"UP" : L"DOWN"
        );
    } else {
      EndTime = TimerGetMs ();
      Result->DurationMs = EndTime - StartTime;
      Result->Status = TestStatusPass;
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"%d NICs detected | Link: %s",
        NicCount,
        AnyLinkUp ? L"UP" : L"DOWN"
        );
    }
  }

  FreePool (HandleBuffer);
  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gNetworkTestModule = {
  L"Network Test",                                    // Name
  L"Network interface detection, MAC, link status",   // Description
  TRUE,                                               // IsQuickTest
  NetworkTestRun                                      // Run
};
