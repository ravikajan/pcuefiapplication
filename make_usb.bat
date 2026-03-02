@echo off
setlocal EnableDelayedExpansion
REM ============================================================
REM  UEFI Hardware Test Suite - USB Deployment Script
REM  Copies BOOTX64.EFI to a USB drive for UEFI boot
REM ============================================================

echo ============================================
echo   UEFI Hardware Test Suite - USB Deploy
echo ============================================
echo.

REM --- Get USB drive letter ---
IF "%1"=="" (
    echo Usage: make_usb.bat [DRIVE_LETTER]
    echo.
    echo Example:
    echo   make_usb.bat E
    echo.
    echo This will copy BOOTX64.EFI to E:\EFI\BOOT\BOOTX64.EFI
    echo.
    echo IMPORTANT: The USB drive must be formatted as FAT32!
    exit /b 1
)

SET USB_DRIVE=%1:
SET EFI_BOOT_DIR=%USB_DRIVE%\EFI\BOOT
SET EFI_DRV_DIR=%USB_DRIVE%\EFI\BOOT\DRIVERS
SET EFI_APP_DRV_DIR=%USB_DRIVE%\EFI\HWTEST\DRIVERS

REM --- Verify drive exists ---
IF NOT EXIST "%USB_DRIVE%\" (
    echo ERROR: Drive %USB_DRIVE% not found.
    exit /b 1
)

REM --- Locate the EFI binary ---
SET EFI_BIN=%~dp0HwTestApp.efi
IF NOT EXIST "%EFI_BIN%" (
    echo ERROR: HwTestApp.efi not found. Run build.bat first.
    exit /b 1
)

REM --- Create EFI boot directory ---
IF NOT EXIST "%EFI_BOOT_DIR%" (
    echo Creating directory: %EFI_BOOT_DIR%
    mkdir "%EFI_BOOT_DIR%"
)

IF NOT EXIST "%EFI_DRV_DIR%" (
    mkdir "%EFI_DRV_DIR%"
)

IF NOT EXIST "%EFI_APP_DRV_DIR%" (
    mkdir "%EFI_APP_DRV_DIR%"
)

REM --- Copy the binary ---
echo Copying HwTestApp.efi to %EFI_BOOT_DIR%\BOOTX64.EFI ...
copy /Y "%EFI_BIN%" "%EFI_BOOT_DIR%\BOOTX64.EFI"

IF ERRORLEVEL 1 (
    echo.
    echo FAILED to copy file!
    exit /b 1
)

echo.
echo ============================================
echo   USB DEPLOYMENT COMPLETE
echo ============================================

REM --- Copy optional driver binaries (recursive) ---
set DRIVER_FOUND=0
for /R "%~dp0Drivers" %%F in (*.efi) do (
    set DRIVER_FOUND=1
    copy /Y "%%F" "%EFI_DRV_DIR%\" >nul
    copy /Y "%%F" "%EFI_APP_DRV_DIR%\" >nul
)

if "!DRIVER_FOUND!"=="0" (
    echo No optional driver binaries found in %~dp0Drivers
) else (
    echo Optional drivers copied to %EFI_DRV_DIR% and %EFI_APP_DRV_DIR%
)

echo.
echo   Drive: %USB_DRIVE%
echo   Path:  %EFI_BOOT_DIR%\BOOTX64.EFI
echo.
echo To boot from this USB:
echo   1. Insert the USB drive into the target PC
echo   2. Enter BIOS/UEFI setup (usually F2, F12, or DEL at boot)
echo   3. Disable Secure Boot (if enabled)
echo   4. Select the USB drive as boot device
echo   5. The Hardware Test Suite will start automatically
echo.

endlocal
