/** @file
  String.c - String Formatting Utilities Implementation

  Copyright (c) 2026. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "String.h"

VOID
EFIAPI
StringUint64ToDec (
  IN  UINT64  Value,
  OUT CHAR16  *Buffer,
  IN  UINTN   Size
  )
{
  UnicodeSPrint (Buffer, Size, L"%lu", Value);
}

VOID
EFIAPI
StringUint64ToHex (
  IN  UINT64  Value,
  OUT CHAR16  *Buffer,
  IN  UINTN   Size
  )
{
  UnicodeSPrint (Buffer, Size, L"0x%lX", Value);
}

VOID
EFIAPI
StringSafeCopy (
  OUT CHAR16       *Dest,
  IN  UINTN        DestSize,
  IN  CONST CHAR16 *Src
  )
{
  UINTN MaxChars;
  UINTN SrcLen;
  UINTN CopyLen;

  if (Dest == NULL || Src == NULL || DestSize < sizeof (CHAR16)) {
    return;
  }

  MaxChars = (DestSize / sizeof (CHAR16)) - 1;
  SrcLen = StrLen (Src);
  CopyLen = (SrcLen < MaxChars) ? SrcLen : MaxChars;

  CopyMem (Dest, Src, CopyLen * sizeof (CHAR16));
  Dest[CopyLen] = L'\0';
}

VOID
EFIAPI
StringSafeCat (
  IN OUT CHAR16       *Dest,
  IN     UINTN        DestSize,
  IN     CONST CHAR16 *Src
  )
{
  UINTN MaxChars;
  UINTN DestLen;
  UINTN SrcLen;
  UINTN CopyLen;
  UINTN Remaining;

  if (Dest == NULL || Src == NULL || DestSize < sizeof (CHAR16)) {
    return;
  }

  MaxChars = (DestSize / sizeof (CHAR16)) - 1;
  DestLen = StrLen (Dest);

  if (DestLen >= MaxChars) {
    return;
  }

  Remaining = MaxChars - DestLen;
  SrcLen = StrLen (Src);
  CopyLen = (SrcLen < Remaining) ? SrcLen : Remaining;

  CopyMem (Dest + DestLen, Src, CopyLen * sizeof (CHAR16));
  Dest[DestLen + CopyLen] = L'\0';
}

VOID
EFIAPI
StringFormatSize (
  IN  UINT64  Bytes,
  OUT CHAR16  *Buffer,
  IN  UINTN   Size
  )
{
  if (Bytes >= (1024ULL * 1024ULL * 1024ULL)) {
    UnicodeSPrint (Buffer, Size, L"%lu GB", Bytes / (1024ULL * 1024ULL * 1024ULL));
  } else if (Bytes >= (1024ULL * 1024ULL)) {
    UnicodeSPrint (Buffer, Size, L"%lu MB", Bytes / (1024ULL * 1024ULL));
  } else if (Bytes >= 1024ULL) {
    UnicodeSPrint (Buffer, Size, L"%lu KB", Bytes / 1024ULL);
  } else {
    UnicodeSPrint (Buffer, Size, L"%lu B", Bytes);
  }
}
