/** @file
  DriverLoader.c - Optional UEFI Driver Loader
**/

#include "DriverLoader.h"

#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

STATIC CHAR16 *mDriverDirs[] = {
  L"\\EFI\\HWTEST\\DRIVERS",
  L"\\EFI\\BOOT\\DRIVERS"
};

STATIC
BOOLEAN
HasEfiExtension (
  IN CONST CHAR16 *Name
  )
{
  UINTN Len;

  if (Name == NULL) {
    return FALSE;
  }

  Len = StrLen (Name);
  if (Len < 4) {
    return FALSE;
  }

  return (Name[Len - 4] == L'.' &&
          (Name[Len - 3] == L'e' || Name[Len - 3] == L'E') &&
          (Name[Len - 2] == L'f' || Name[Len - 2] == L'F') &&
          (Name[Len - 1] == L'i' || Name[Len - 1] == L'I'));
}

STATIC
VOID
TryLoadFromDirectory (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_HANDLE         FsHandle,
  IN  EFI_FILE_PROTOCOL  *Root,
  IN  CONST CHAR16       *DirPath,
  OUT UINTN              *LoadedCount,
  OUT UINTN              *StartedCount,
  OUT UINTN              *FailedCount
  )
{
  EFI_STATUS         Status;
  EFI_FILE_PROTOCOL  *Dir;
  EFI_FILE_INFO      *Info;
  UINTN              InfoSize;

  Status = Root->Open (
                   Root,
                   &Dir,
                   (CHAR16 *)DirPath,
                   EFI_FILE_MODE_READ,
                   0
                   );
  if (EFI_ERROR (Status)) {
    return;
  }

  InfoSize = SIZE_OF_EFI_FILE_INFO + 512;
  Info = AllocateZeroPool (InfoSize);
  if (Info == NULL) {
    Dir->Close (Dir);
    return;
  }

  while (TRUE) {
    UINTN ReadSize;

    ReadSize = InfoSize;
    Status = Dir->Read (Dir, &ReadSize, Info);
    if (EFI_ERROR (Status) || ReadSize == 0) {
      break;
    }

    if ((Info->Attribute & EFI_FILE_DIRECTORY) != 0) {
      continue;
    }

    if (HasEfiExtension (Info->FileName)) {
      CHAR16      FullPath[260];
      EFI_DEVICE_PATH_PROTOCOL *FilePath;
      EFI_HANDLE  DriverImage;
      EFI_STATUS  StartStatus;

      UnicodeSPrint (FullPath, sizeof (FullPath), L"%s\\%s", DirPath, Info->FileName);

      FilePath = FileDevicePath (FsHandle, FullPath);
      if (FilePath == NULL) {
        (*FailedCount)++;
        continue;
      }

      Status = gBS->LoadImage (
                      FALSE,
                      ImageHandle,
                      FilePath,
                      NULL,
                      0,
                      &DriverImage
                      );
      FreePool (FilePath);

      if (EFI_ERROR (Status)) {
        (*FailedCount)++;
        continue;
      }

      (*LoadedCount)++;

      StartStatus = gBS->StartImage (DriverImage, NULL, NULL);
      if (EFI_ERROR (StartStatus)) {
        (*FailedCount)++;
      } else {
        (*StartedCount)++;
      }
    }
  }

  FreePool (Info);
  Dir->Close (Dir);
}

EFI_STATUS
EFIAPI
DriverLoaderLoadAll (
  IN  EFI_HANDLE ImageHandle,
  OUT UINTN      *LoadedCount   OPTIONAL,
  OUT UINTN      *StartedCount  OPTIONAL,
  OUT UINTN      *FailedCount   OPTIONAL
  )
{
  EFI_STATUS                       Status;
  EFI_HANDLE                       *FsHandles;
  UINTN                            FsCount;
  UINTN                            i;
  UINTN                            d;
  UINTN                            LocalLoaded;
  UINTN                            LocalStarted;
  UINTN                            LocalFailed;

  LocalLoaded = 0;
  LocalStarted = 0;
  LocalFailed = 0;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &FsCount,
                  &FsHandles
                  );
  if (EFI_ERROR (Status)) {
    if (LoadedCount != NULL) {
      *LoadedCount = 0;
    }
    if (StartedCount != NULL) {
      *StartedCount = 0;
    }
    if (FailedCount != NULL) {
      *FailedCount = 0;
    }
    return Status;
  }

  for (i = 0; i < FsCount; i++) {
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Fs;
    EFI_FILE_PROTOCOL               *Root;

    Status = gBS->HandleProtocol (
                    FsHandles[i],
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

    for (d = 0; d < (sizeof (mDriverDirs) / sizeof (mDriverDirs[0])); d++) {
      TryLoadFromDirectory (
        ImageHandle,
        FsHandles[i],
        Root,
        mDriverDirs[d],
        &LocalLoaded,
        &LocalStarted,
        &LocalFailed
        );
    }

    Root->Close (Root);
  }

  FreePool (FsHandles);

  {
    EFI_HANDLE *ControllerHandles;
    UINTN      HandleCount;
    UINTN      h;

    if (!EFI_ERROR (gBS->LocateHandleBuffer (AllHandles, NULL, NULL, &HandleCount, &ControllerHandles))) {
      for (h = 0; h < HandleCount; h++) {
        gBS->ConnectController (ControllerHandles[h], NULL, NULL, TRUE);
      }
      FreePool (ControllerHandles);
    }
  }

  if (LoadedCount != NULL) {
    *LoadedCount = LocalLoaded;
  }
  if (StartedCount != NULL) {
    *StartedCount = LocalStarted;
  }
  if (FailedCount != NULL) {
    *FailedCount = LocalFailed;
  }

  return EFI_SUCCESS;
}
