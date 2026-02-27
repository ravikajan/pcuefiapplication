/** @file
  String.h - String Formatting Utilities

  Provides integer-to-string conversion, wide-string helpers,
  and buffer formatting functions for report generation.

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HW_STRING_H_
#define HW_STRING_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>

/**
  Convert a UINT64 value to a decimal wide string.

  @param[in]  Value   The value to convert.
  @param[out] Buffer  Output buffer (must be at least 21 chars).
  @param[in]  Size    Size of buffer in bytes.
**/
VOID
EFIAPI
StringUint64ToDec (
  IN  UINT64  Value,
  OUT CHAR16  *Buffer,
  IN  UINTN   Size
  );

/**
  Convert a UINT64 value to a hexadecimal wide string.

  @param[in]  Value   The value to convert.
  @param[out] Buffer  Output buffer (must be at least 19 chars).
  @param[in]  Size    Size of buffer in bytes.
**/
VOID
EFIAPI
StringUint64ToHex (
  IN  UINT64  Value,
  OUT CHAR16  *Buffer,
  IN  UINTN   Size
  );

/**
  Safe wide string copy.

  @param[out] Dest      Destination buffer.
  @param[in]  DestSize  Size of destination in bytes.
  @param[in]  Src       Source string.
**/
VOID
EFIAPI
StringSafeCopy (
  OUT CHAR16       *Dest,
  IN  UINTN        DestSize,
  IN  CONST CHAR16 *Src
  );

/**
  Safe wide string concatenation.

  @param[in,out] Dest      Destination buffer.
  @param[in]     DestSize  Size of destination in bytes.
  @param[in]     Src       Source string to append.
**/
VOID
EFIAPI
StringSafeCat (
  IN OUT CHAR16       *Dest,
  IN     UINTN        DestSize,
  IN     CONST CHAR16 *Src
  );

/**
  Format bytes into a human-readable size string (KB, MB, GB).

  @param[in]  Bytes   Number of bytes.
  @param[out] Buffer  Output buffer.
  @param[in]  Size    Size of buffer in bytes.
**/
VOID
EFIAPI
StringFormatSize (
  IN  UINT64  Bytes,
  OUT CHAR16  *Buffer,
  IN  UINTN   Size
  );

#endif // HW_STRING_H_
