@echo off
REM ============================================================
REM  Download common UEFI network drivers (iPXE)
REM ============================================================

echo ============================================
echo   Download Common UEFI Drivers
echo ============================================
echo.

SET DRIVER_DIR=%~dp0Drivers
IF NOT EXIST "%DRIVER_DIR%" mkdir "%DRIVER_DIR%"
IF NOT EXIST "%DRIVER_DIR%\Network" mkdir "%DRIVER_DIR%\Network"
IF NOT EXIST "%DRIVER_DIR%\WiFi" mkdir "%DRIVER_DIR%\WiFi"
IF NOT EXIST "%DRIVER_DIR%\Bluetooth" mkdir "%DRIVER_DIR%\Bluetooth"

echo Downloading iPXE x64 EFI drivers...
powershell -NoProfile -ExecutionPolicy Bypass -Command "Invoke-WebRequest -Uri 'https://boot.ipxe.org/x86_64-efi/ipxe.efi' -OutFile '%DRIVER_DIR%\Network\ipxe_x64.efi'"
IF ERRORLEVEL 1 (
  echo WARNING: Failed to download ipxe_x64.efi
)

powershell -NoProfile -ExecutionPolicy Bypass -Command "Invoke-WebRequest -Uri 'https://boot.ipxe.org/x86_64-efi/snponly.efi' -OutFile '%DRIVER_DIR%\Network\snponly_x64.efi'"
IF ERRORLEVEL 1 (
  echo WARNING: Failed to download snponly_x64.efi
)

powershell -NoProfile -ExecutionPolicy Bypass -Command "Invoke-WebRequest -Uri 'https://boot.ipxe.org/x86_64-efi/ipxe-legacy.efi' -OutFile '%DRIVER_DIR%\Network\ipxe_legacy_x64.efi'"
IF ERRORLEVEL 1 (
  echo WARNING: Failed to download ipxe_legacy_x64.efi
)

REM Flatten into Drivers root so make_usb can copy a single set easily
copy /Y "%DRIVER_DIR%\Network\*.efi" "%DRIVER_DIR%\" >nul 2>nul

echo.
echo Done. Files in: %DRIVER_DIR%
dir /b "%DRIVER_DIR%\*.efi" 2>nul
echo.
echo NOTE:
echo   - Network drivers downloaded to Drivers\Network and Drivers\ root.
echo   - No universal public UEFI WiFi/Bluetooth drivers exist for all chipsets.
echo   - Put vendor-specific WiFi .efi in Drivers\WiFi and Bluetooth .efi in Drivers\Bluetooth.
