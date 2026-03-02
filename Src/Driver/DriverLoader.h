/** @file
  DriverLoader.h - Optional UEFI Driver Loader

  Loads optional driver images (*.efi) from known filesystem paths and
  reconnects controllers so newly loaded drivers can bind.
**/

#ifndef HW_DRIVER_LOADER_H_
#define HW_DRIVER_LOADER_H_

#include <Uefi.h>

EFI_STATUS
EFIAPI
DriverLoaderLoadAll (
  IN  EFI_HANDLE ImageHandle,
  OUT UINTN      *LoadedCount   OPTIONAL,
  OUT UINTN      *StartedCount  OPTIONAL,
  OUT UINTN      *FailedCount   OPTIONAL
  );

#endif // HW_DRIVER_LOADER_H_
