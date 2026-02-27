@echo off
REM ============================================================
REM  UEFI Hardware Test Suite - QEMU Test Runner
REM  Boots the compiled EFI binary in QEMU with OVMF firmware
REM ============================================================

echo ============================================
echo   UEFI Hardware Test Suite - QEMU Runner
echo ============================================
echo.

REM --- Configuration ---
IF NOT DEFINED QEMU_PATH (
    SET QEMU_PATH=C:\Program Files\qemu
)

IF NOT DEFINED OVMF_PATH (
    REM Common OVMF locations
    IF EXIST "%EDK2_PATH%\Build\OvmfX64\RELEASE_VS2019\FV\OVMF.fd" (
        SET OVMF_PATH=%EDK2_PATH%\Build\OvmfX64\RELEASE_VS2019\FV\OVMF.fd
    ) ELSE IF EXIST "C:\OVMF\OVMF.fd" (
        SET OVMF_PATH=C:\OVMF\OVMF.fd
    ) ELSE (
        echo ERROR: OVMF firmware not found. Set OVMF_PATH environment variable.
        echo.
        echo You can download OVMF from:
        echo   https://retrage.github.io/edk2-nightly/
        echo.
        echo Example:
        echo   set OVMF_PATH=C:\OVMF\OVMF.fd
        exit /b 1
    )
)

REM --- Prepare virtual FAT32 disk directory ---
SET VDISK_DIR=%~dp0qemu_disk
SET EFI_BOOT_DIR=%VDISK_DIR%\EFI\BOOT

IF NOT EXIST "%EFI_BOOT_DIR%" (
    mkdir "%EFI_BOOT_DIR%"
)

REM --- Copy the EFI binary ---
SET EFI_BIN=%~dp0HwTestApp.efi
IF NOT EXIST "%EFI_BIN%" (
    REM Try build output location
    IF DEFINED EDK2_PATH (
        SET EFI_BIN=%EDK2_PATH%\Build\HwTestPkg\RELEASE_VS2019\X64\HwTestApp.efi
    )
)

IF NOT EXIST "%EFI_BIN%" (
    echo ERROR: HwTestApp.efi not found. Run build.bat first.
    exit /b 1
)

copy /Y "%EFI_BIN%" "%EFI_BOOT_DIR%\BOOTX64.EFI" >nul
echo Prepared: %EFI_BOOT_DIR%\BOOTX64.EFI

REM --- Launch QEMU ---
echo.
echo Launching QEMU...
echo   Firmware: %OVMF_PATH%
echo   Disk:     %VDISK_DIR%
echo.

"%QEMU_PATH%\qemu-system-x86_64.exe" ^
    -bios "%OVMF_PATH%" ^
    -drive format=raw,file=fat:rw:%VDISK_DIR% ^
    -m 512M ^
    -cpu qemu64 ^
    -net none ^
    -serial stdio ^
    -vga std

echo.
echo QEMU session ended.
