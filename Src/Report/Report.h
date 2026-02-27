/** @file
  Report.h - Report Generation & Export

  Generates test reports in JSON, CSV, and TXT formats.
  Writes reports to FAT32 filesystem via Simple File System Protocol.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_REPORT_H_
#define HW_REPORT_H_

#include <Uefi.h>
#include "../Tests/TestModule.h"

// Report format flags
#define REPORT_FORMAT_TXT    0x01
#define REPORT_FORMAT_CSV    0x02
#define REPORT_FORMAT_JSON   0x04
#define REPORT_FORMAT_ALL    (REPORT_FORMAT_TXT | REPORT_FORMAT_CSV | REPORT_FORMAT_JSON)

/**
  Generate and save test reports to the boot filesystem.

  @param[in] Results       Array of test results.
  @param[in] ResultCount   Number of results.
  @param[in] FormatFlags   Bitmask of REPORT_FORMAT_* flags.

  @retval EFI_SUCCESS      Reports saved successfully.
  @retval EFI_NOT_FOUND    No writable filesystem found.
**/
EFI_STATUS
EFIAPI
ReportSave (
  IN HW_TEST_RESULT  *Results,
  IN UINTN           ResultCount,
  IN UINT32          FormatFlags
  );

/**
  Generate a TXT report string.

  @param[in]  Results      Array of test results.
  @param[in]  ResultCount  Number of results.
  @param[out] Buffer       Output buffer (allocated by this function, caller must free).
  @param[out] BufferSize   Size of the output buffer.
**/
EFI_STATUS
EFIAPI
ReportGenerateTxt (
  IN  HW_TEST_RESULT  *Results,
  IN  UINTN           ResultCount,
  OUT CHAR8            **Buffer,
  OUT UINTN            *BufferSize
  );

/**
  Generate a CSV report string.
**/
EFI_STATUS
EFIAPI
ReportGenerateCsv (
  IN  HW_TEST_RESULT  *Results,
  IN  UINTN           ResultCount,
  OUT CHAR8            **Buffer,
  OUT UINTN            *BufferSize
  );

/**
  Generate a JSON report string.
**/
EFI_STATUS
EFIAPI
ReportGenerateJson (
  IN  HW_TEST_RESULT  *Results,
  IN  UINTN           ResultCount,
  OUT CHAR8            **Buffer,
  OUT UINTN            *BufferSize
  );

#endif // HW_REPORT_H_
