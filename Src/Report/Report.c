/** @file
  Report.c - Report Generation & Export Implementation

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Report.h"
#include "../Utils/String.h"
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

// Max report buffer size (64KB per format)
#define REPORT_BUFFER_MAX   (64 * 1024)

/**
  Helper to get ASCII status string.
**/
STATIC
CONST CHAR8 *
StatusToAscii (
  IN HW_TEST_STATUS Status
  )
{
  switch (Status) {
    case TestStatusPass:  return "PASS";
    case TestStatusFail:  return "FAIL";
    case TestStatusSkip:  return "SKIP";
    case TestStatusError: return "ERROR";
    default:              return "UNKNOWN";
  }
}

/**
  Helper to convert CHAR16 to CHAR8 (ASCII only, truncates).
**/
STATIC
VOID
UnicodeToAscii (
  IN  CONST CHAR16  *Src,
  OUT CHAR8          *Dest,
  IN  UINTN          DestSize
  )
{
  UINTN i;
  UINTN Max = DestSize - 1;
  UINTN Len = StrLen (Src);

  if (Len > Max) {
    Len = Max;
  }

  for (i = 0; i < Len; i++) {
    Dest[i] = (CHAR8)(Src[i] & 0x7F);
  }
  Dest[Len] = '\0';
}

EFI_STATUS
EFIAPI
ReportGenerateTxt (
  IN  HW_TEST_RESULT  *Results,
  IN  UINTN           ResultCount,
  OUT CHAR8            **Buffer,
  OUT UINTN            *BufferSize
  )
{
  CHAR8  *Buf;
  UINTN  Offset;
  UINTN  i;
  CHAR8  NameA[64];
  CHAR8  DetailA[256];

  Buf = AllocateZeroPool (REPORT_BUFFER_MAX);
  if (Buf == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Offset = 0;

  Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
    "========================================\r\n"
    "   UEFI Hardware Test Suite - Report\r\n"
    "========================================\r\n\r\n"
    );

  Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
    "%-8s  %-20s  %-10s  %s\r\n",
    "Status", "Test Name", "Duration", "Details"
    );
  Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
    "------  --------------------  ----------  --------\r\n"
    );

  for (i = 0; i < ResultCount; i++) {
    UnicodeToAscii (Results[i].TestName, NameA, sizeof (NameA));
    UnicodeToAscii (Results[i].Details, DetailA, sizeof (DetailA));

    Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
      "[%-5a]  %-20a  %8lu ms  %a\r\n",
      StatusToAscii (Results[i].Status),
      NameA,
      Results[i].DurationMs,
      DetailA
      );
  }

  // Summary
  {
    UINTN Passed = 0, Failed = 0, Skipped = 0;
    for (i = 0; i < ResultCount; i++) {
      if (Results[i].Status == TestStatusPass)  Passed++;
      else if (Results[i].Status == TestStatusFail) Failed++;
      else Skipped++;
    }

    Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
      "\r\n========================================\r\n"
      "SUMMARY: Total=%d  Passed=%d  Failed=%d  Skipped=%d\r\n"
      "========================================\r\n",
      ResultCount, Passed, Failed, Skipped
      );
  }

  *Buffer = Buf;
  *BufferSize = Offset;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ReportGenerateCsv (
  IN  HW_TEST_RESULT  *Results,
  IN  UINTN           ResultCount,
  OUT CHAR8            **Buffer,
  OUT UINTN            *BufferSize
  )
{
  CHAR8  *Buf;
  UINTN  Offset;
  UINTN  i;
  CHAR8  NameA[64];
  CHAR8  DetailA[256];

  Buf = AllocateZeroPool (REPORT_BUFFER_MAX);
  if (Buf == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Offset = 0;

  // CSV header
  Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
    "TestName,Status,DurationMs,Details\r\n"
    );

  for (i = 0; i < ResultCount; i++) {
    UnicodeToAscii (Results[i].TestName, NameA, sizeof (NameA));
    UnicodeToAscii (Results[i].Details, DetailA, sizeof (DetailA));

    Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
      "\"%a\",\"%a\",%lu,\"%a\"\r\n",
      NameA,
      StatusToAscii (Results[i].Status),
      Results[i].DurationMs,
      DetailA
      );
  }

  *Buffer = Buf;
  *BufferSize = Offset;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ReportGenerateJson (
  IN  HW_TEST_RESULT  *Results,
  IN  UINTN           ResultCount,
  OUT CHAR8            **Buffer,
  OUT UINTN            *BufferSize
  )
{
  CHAR8  *Buf;
  UINTN  Offset;
  UINTN  i;
  CHAR8  NameA[64];
  CHAR8  DetailA[256];

  Buf = AllocateZeroPool (REPORT_BUFFER_MAX);
  if (Buf == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Offset = 0;

  Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
    "{\r\n"
    "  \"title\": \"UEFI Hardware Test Suite Report\",\r\n"
    "  \"version\": \"1.0\",\r\n"
    "  \"totalTests\": %d,\r\n"
    "  \"results\": [\r\n",
    ResultCount
    );

  for (i = 0; i < ResultCount; i++) {
    UnicodeToAscii (Results[i].TestName, NameA, sizeof (NameA));
    UnicodeToAscii (Results[i].Details, DetailA, sizeof (DetailA));

    Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
      "    {\r\n"
      "      \"testName\": \"%a\",\r\n"
      "      \"status\": \"%a\",\r\n"
      "      \"durationMs\": %lu,\r\n"
      "      \"details\": \"%a\"\r\n"
      "    }%a\r\n",
      NameA,
      StatusToAscii (Results[i].Status),
      Results[i].DurationMs,
      DetailA,
      (i < ResultCount - 1) ? "," : ""
      );
  }

  // Summary
  {
    UINTN Passed = 0, Failed = 0, Skipped = 0;
    for (i = 0; i < ResultCount; i++) {
      if (Results[i].Status == TestStatusPass)  Passed++;
      else if (Results[i].Status == TestStatusFail) Failed++;
      else Skipped++;
    }

    Offset += AsciiSPrint (Buf + Offset, REPORT_BUFFER_MAX - Offset,
      "  ],\r\n"
      "  \"summary\": {\r\n"
      "    \"passed\": %d,\r\n"
      "    \"failed\": %d,\r\n"
      "    \"skipped\": %d\r\n"
      "  }\r\n"
      "}\r\n",
      Passed, Failed, Skipped
      );
  }

  *Buffer = Buf;
  *BufferSize = Offset;
  return EFI_SUCCESS;
}

/**
  Write a buffer to a file on the filesystem.
**/
STATIC
EFI_STATUS
WriteFile (
  IN EFI_FILE_PROTOCOL  *Root,
  IN CHAR16             *FileName,
  IN CHAR8              *Data,
  IN UINTN              DataSize
  )
{
  EFI_STATUS        Status;
  EFI_FILE_PROTOCOL *File;

  Status = Root->Open (
                   Root,
                   &File,
                   FileName,
                   EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ,
                   0
                   );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = File->Write (File, &DataSize, Data);
  File->Close (File);

  return Status;
}

EFI_STATUS
EFIAPI
ReportSave (
  IN HW_TEST_RESULT  *Results,
  IN UINTN           ResultCount,
  IN UINT32          FormatFlags
  )
{
  EFI_STATUS                       Status;
  EFI_HANDLE                       *HandleBuffer;
  UINTN                            HandleCount;
  UINTN                            i;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Fs;
  EFI_FILE_PROTOCOL                *Root;
  BOOLEAN                          Saved;
  CHAR16                           FileName[64];

  Saved = FALSE;

  // Locate file system handles
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  if (EFI_ERROR (Status) || HandleCount == 0) {
    return EFI_NOT_FOUND;
  }

  // Try each filesystem until we succeed
  for (i = 0; i < HandleCount && !Saved; i++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[i],
                    &gEfiSimpleFileSystemProtocolGuid,
                    (VOID **)&Fs
                    );

    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = Fs->OpenVolume (Fs, &Root);
    if (EFI_ERROR (Status)) {
      continue;
    }

    // Generate timestamp-based filename prefix
    {
      EFI_TIME  Time;
      gRT->GetTime (&Time, NULL);

      // Save TXT report
      if (FormatFlags & REPORT_FORMAT_TXT) {
        CHAR8 *TxtBuf;
        UINTN TxtSize;

        Status = ReportGenerateTxt (Results, ResultCount, &TxtBuf, &TxtSize);
        if (!EFI_ERROR (Status)) {
          UnicodeSPrint (FileName, sizeof (FileName),
            L"HwTest_%04d%02d%02d_%02d%02d%02d.txt",
            Time.Year, Time.Month, Time.Day,
            Time.Hour, Time.Minute, Time.Second
            );
          Status = WriteFile (Root, FileName, TxtBuf, TxtSize);
          if (!EFI_ERROR (Status)) {
            Saved = TRUE;
          }
          FreePool (TxtBuf);
        }
      }

      // Save CSV report
      if (FormatFlags & REPORT_FORMAT_CSV) {
        CHAR8 *CsvBuf;
        UINTN CsvSize;

        Status = ReportGenerateCsv (Results, ResultCount, &CsvBuf, &CsvSize);
        if (!EFI_ERROR (Status)) {
          UnicodeSPrint (FileName, sizeof (FileName),
            L"HwTest_%04d%02d%02d_%02d%02d%02d.csv",
            Time.Year, Time.Month, Time.Day,
            Time.Hour, Time.Minute, Time.Second
            );
          WriteFile (Root, FileName, CsvBuf, CsvSize);
          FreePool (CsvBuf);
        }
      }

      // Save JSON report
      if (FormatFlags & REPORT_FORMAT_JSON) {
        CHAR8 *JsonBuf;
        UINTN JsonSize;

        Status = ReportGenerateJson (Results, ResultCount, &JsonBuf, &JsonSize);
        if (!EFI_ERROR (Status)) {
          UnicodeSPrint (FileName, sizeof (FileName),
            L"HwTest_%04d%02d%02d_%02d%02d%02d.json",
            Time.Year, Time.Month, Time.Day,
            Time.Hour, Time.Minute, Time.Second
            );
          WriteFile (Root, FileName, JsonBuf, JsonSize);
          FreePool (JsonBuf);
        }
      }
    }

    Root->Close (Root);
  }

  FreePool (HandleBuffer);

  return Saved ? EFI_SUCCESS : EFI_NOT_FOUND;
}
