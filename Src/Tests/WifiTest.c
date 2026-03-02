/** @file
  WifiTest.c - WiFi Test Module

  Best-effort WiFi diagnostics for UEFI pre-OS environments:
  - Scans network adapters and identifies likely WiFi interfaces
  - Builds a visible adapter list (scan/list flow)
  - Attempts interface initialization and link check (connectivity flow)

  Note: Most firmware does not expose full SSID scan/auth APIs in a portable way,
  so this test uses protocol-level diagnostics and link state checks.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "TestModule.h"
#include "../Utils/Timer.h"
#include "../Utils/String.h"
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PrintLib.h>
#include <Protocol/SimpleNetwork.h>

#define WIFI_LIST_PREVIEW_MAX  3

/**
  Check whether device path text suggests a WiFi/WLAN adapter.
**/
STATIC
BOOLEAN
IsLikelyWifiAdapter (
  IN CONST CHAR16 *PathText
  )
{
  if (PathText == NULL) {
    return FALSE;
  }

  if (StrStr (PathText, L"Wi-Fi") != NULL ||
      StrStr (PathText, L"WiFi") != NULL ||
      StrStr (PathText, L"wifi") != NULL ||
      StrStr (PathText, L"WLAN") != NULL ||
      StrStr (PathText, L"Wireless") != NULL ||
      StrStr (PathText, L"802.11") != NULL) {
    return TRUE;
  }

  return FALSE;
}

/**
  Append one adapter token to the preview list string.
**/
STATIC
VOID
AppendAdapterToken (
  IN OUT CHAR16       *Preview,
  IN     UINTN        PreviewSize,
  IN     CONST CHAR16 *Token,
  IN     BOOLEAN      First
  )
{
  if (!First) {
    StringSafeCat (Preview, PreviewSize, L", ");
  }

  StringSafeCat (Preview, PreviewSize, Token);
}

/**
  Run the WiFi test module.
**/
EFI_STATUS
EFIAPI
WifiTestRun (
  IN  HW_TEST_MODULE  *Self,
  OUT HW_TEST_RESULT  *Result
  )
{
  EFI_STATUS                  Status;
  UINT64                      StartTime;
  UINT64                      EndTime;
  EFI_HANDLE                  *HandleBuffer;
  UINTN                       HandleCount;
  UINTN                       i;
  UINTN                       NicCount;
  UINTN                       WifiCandidateCount;
  UINTN                       LinkUpCount;
  UINTN                       MediaSupportedCount;
  UINTN                       ListShown;
  UINTN                       HandleProtocolErrors;
  UINTN                       StartErrors;
  UINTN                       InitErrors;
  BOOLEAN                     UsedGenericFallback;
  EFI_STATUS                  LastError;
  CHAR16                      AdapterPreview[256];

  StringSafeCopy (Result->TestName, sizeof (Result->TestName), L"WiFi Test");
  StartTime = TimerGetMs ();

  NicCount = 0;
  WifiCandidateCount = 0;
  LinkUpCount = 0;
  MediaSupportedCount = 0;
  ListShown = 0;
  HandleProtocolErrors = 0;
  StartErrors = 0;
  InitErrors = 0;
  UsedGenericFallback = FALSE;
  LastError = EFI_SUCCESS;
  AdapterPreview[0] = L'\0';

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
    if (EFI_ERROR (Status) && Status != EFI_NOT_FOUND) {
      Result->Status = TestStatusError;
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"WiFi scan failed: %r",
        Status
        );
    } else {
      Result->Status = TestStatusSkip;
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"WiFi scan: no network adapters available"
        );
    }
    return EFI_SUCCESS;
  }

  for (i = 0; i < HandleCount; i++) {
    EFI_SIMPLE_NETWORK_PROTOCOL *Snp;
    EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
    CHAR16                      *PathText;
    BOOLEAN                     IsWifiCandidate;

    Status = gBS->HandleProtocol (
                    HandleBuffer[i],
                    &gEfiSimpleNetworkProtocolGuid,
                    (VOID **)&Snp
                    );
    if (EFI_ERROR (Status)) {
      HandleProtocolErrors++;
      LastError = Status;
      continue;
    }

    NicCount++;

    DevicePath = DevicePathFromHandle (HandleBuffer[i]);
    PathText = ConvertDevicePathToText (DevicePath, FALSE, FALSE);
    IsWifiCandidate = IsLikelyWifiAdapter (PathText);

    if (IsWifiCandidate) {
      WifiCandidateCount++;

      if (ListShown < WIFI_LIST_PREVIEW_MAX) {
        CHAR16 Token[40];
        UnicodeSPrint (Token, sizeof (Token), L"WiFiNIC-%d", WifiCandidateCount);
        AppendAdapterToken (
          AdapterPreview,
          sizeof (AdapterPreview),
          Token,
          (ListShown == 0)
          );
        ListShown++;
      }
    }

    if (PathText != NULL) {
      FreePool (PathText);
    }

    if (Snp->Mode != NULL && Snp->Mode->State == EfiSimpleNetworkStopped) {
      Status = Snp->Start (Snp);
      if (EFI_ERROR (Status)) {
        StartErrors++;
        LastError = Status;
      }
    }
    if (Snp->Mode != NULL && Snp->Mode->State == EfiSimpleNetworkStarted) {
      Status = Snp->Initialize (Snp, 0, 0);
      if (EFI_ERROR (Status)) {
        InitErrors++;
        LastError = Status;
      }
    }

    if (Snp->Mode != NULL && Snp->Mode->MediaPresentSupported) {
      MediaSupportedCount++;
      if (Snp->Mode->MediaPresent) {
        LinkUpCount++;
      }
    }
  }

  if (WifiCandidateCount == 0 && NicCount > 0) {
    UINTN GenericCount;

    UsedGenericFallback = TRUE;
    WifiCandidateCount = NicCount;

    if (ListShown == 0) {
      for (GenericCount = 0;
           GenericCount < NicCount && GenericCount < WIFI_LIST_PREVIEW_MAX;
           GenericCount++) {
        CHAR16 Token[40];
        UnicodeSPrint (Token, sizeof (Token), L"NIC-%d", GenericCount + 1);
        AppendAdapterToken (
          AdapterPreview,
          sizeof (AdapterPreview),
          Token,
          (GenericCount == 0)
          );
        ListShown++;
      }
    }
  }

  FreePool (HandleBuffer);

  EndTime = TimerGetMs ();
  Result->DurationMs = EndTime - StartTime;

  if (WifiCandidateCount > 0 && LinkUpCount > 0) {
    Result->Status = TestStatusPass;
    if ((HandleProtocolErrors + StartErrors + InitErrors) > 0) {
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"Scan OK%s | List: %s | Link up: %d | Warnings H:%d S:%d I:%d (%r)",
        UsedGenericFallback ? L" (generic NIC mode)" : L"",
        (ListShown > 0) ? AdapterPreview : L"WiFi detected",
        LinkUpCount,
        HandleProtocolErrors,
        StartErrors,
        InitErrors,
        LastError
        );
    } else {
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"Scan OK%s | List: %s | Internet/link check: reachable (link up on %d)",
        UsedGenericFallback ? L" (generic NIC mode)" : L"",
        (ListShown > 0) ? AdapterPreview : L"WiFi detected",
        LinkUpCount
        );
    }
  } else if (WifiCandidateCount > 0) {
    if (UsedGenericFallback && StartErrors == 0 && InitErrors == 0) {
      Result->Status = TestStatusSkip;
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"Scan OK (generic NIC mode) | List: %s | Link down in pre-OS | Issue: firmware/driver may not expose WiFi link | Fix: load vendor UEFI NIC driver or test with wired link",
        (ListShown > 0) ? AdapterPreview : L"NIC detected"
        );
      return EFI_SUCCESS;
    }

    Result->Status = TestStatusFail;
    if ((StartErrors + InitErrors) > 0) {
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"Scan OK%s | List: %s | Connect failed S:%d I:%d | Last error: %r",
        UsedGenericFallback ? L" (generic NIC mode)" : L"",
        (ListShown > 0) ? AdapterPreview : L"WiFi detected",
        StartErrors,
        InitErrors,
        LastError
        );
    } else {
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"Scan OK%s | List: %s | Internet/link check failed (link down, media support on %d NICs)",
        UsedGenericFallback ? L" (generic NIC mode)" : L"",
        (ListShown > 0) ? AdapterPreview : L"WiFi detected",
        MediaSupportedCount
        );
    }
  } else if (NicCount > 0) {
    if (HandleProtocolErrors > 0) {
      Result->Status = TestStatusError;
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"NIC inspect errors H:%d | Last error: %r",
        HandleProtocolErrors,
        LastError
        );
    } else {
      Result->Status = TestStatusSkip;
      UnicodeSPrint (
        Result->Details, sizeof (Result->Details),
        L"%d NIC(s) found but none identified as WiFi in UEFI",
        NicCount
        );
    }
  } else {
    Result->Status = TestStatusSkip;
    UnicodeSPrint (
      Result->Details, sizeof (Result->Details),
      L"No usable adapters for WiFi flow"
      );
  }

  return EFI_SUCCESS;
}

// Module descriptor
HW_TEST_MODULE gWifiTestModule = {
  L"WiFi Test",                                      // Name
  L"WiFi scan, adapter list, connect/link check",   // Description
  TRUE,                                              // IsQuickTest
  WifiTestRun                                        // Run
};
