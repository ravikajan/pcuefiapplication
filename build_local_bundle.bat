@echo off
REM ============================================================
REM  Build Local Bundle (Offline driver model)
REM  - Uses ONLY local prebuilt *.efi drivers in .\Drivers
REM  - Does NOT download any drivers at build time or boot time
REM ============================================================

echo ============================================
echo   Build Local Driver Bundle
echo ============================================
echo.

set DRIVER_COUNT=0
for /R "%~dp0Drivers" %%F in (*.efi) do (
  set /A DRIVER_COUNT+=1
)

if "%DRIVER_COUNT%"=="0" (
  echo WARNING: No local driver binaries found in .\Drivers
  echo         You can still build app, but optional NIC/WiFi/BT preload may be limited.
) else (
  echo Local prebuilt drivers found: %DRIVER_COUNT%
)

echo.
echo Building application...
call "%~dp0do_build.bat"
if errorlevel 1 (
  echo.
  echo BUILD FAILED.
  exit /b 1
)

echo.
echo BUILD COMPLETE (offline/local driver model)
echo Next: deploy with make_usb.bat [DRIVE_LETTER]
