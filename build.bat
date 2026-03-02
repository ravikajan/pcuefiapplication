@echo off
REM ============================================================
REM  UEFI Hardware Test Suite - Build Script
REM  Requires: EDK II toolchain, Visual Studio 2019/2022
REM ============================================================

echo ============================================
echo   UEFI Hardware Test Suite - Build
echo ============================================
echo.

REM --- Configuration ---
REM Set EDK2_PATH to your EDK II checkout directory
IF NOT DEFINED EDK2_PATH (
    SET EDK2_PATH=E:\Project\edk2
)

IF NOT DEFINED TOOL_CHAIN_TAG (
    SET TOOL_CHAIN_TAG=VS2022
)

SET TARGET=RELEASE
SET ARCH=X64
SET PKG_NAME=HwTestPkg

echo EDK2 Path : %EDK2_PATH%
echo Toolchain : %TOOL_CHAIN_TAG%
echo Target    : %TARGET%
echo Arch      : %ARCH%
echo.

REM --- Verify EDK II exists ---
IF NOT EXIST "%EDK2_PATH%\edksetup.bat" (
    echo ERROR: EDK II not found at %EDK2_PATH%
    echo Please set EDK2_PATH environment variable to your EDK II directory.
    echo.
    echo Example:
    echo   set EDK2_PATH=C:\edk2
    echo   build.bat
    exit /b 1
)

REM --- Create symlink for our package in EDK II tree ---
IF NOT EXIST "%EDK2_PATH%\%PKG_NAME%" (
    echo Creating symlink: %EDK2_PATH%\%PKG_NAME% -^> %~dp0
    mklink /D "%EDK2_PATH%\%PKG_NAME%" "%~dp0"
    IF ERRORLEVEL 1 (
        echo WARNING: Symlink creation failed. You may need to run as Administrator.
        echo Alternatively, copy this folder to %EDK2_PATH%\%PKG_NAME%
    )
)

REM --- Setup EDK II environment ---
pushd "%EDK2_PATH%"
set PYTHON_COMMAND=python
set PATH=%EDK2_PATH%\BaseTools\Bin\Win32;%PATH%
set EDK_TOOLS_BIN=%EDK2_PATH%\BaseTools\BinWrappers\WindowsLike
set NASM_PREFIX=C:\Users\ADMIN\AppData\Local\bin\NASM\
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64 >nul 2>&1
call edksetup.bat

REM --- Build ---
echo.
echo Building %PKG_NAME%...
echo.

call build -p %PKG_NAME%/%PKG_NAME%.dsc -a %ARCH% -t %TOOL_CHAIN_TAG% -b %TARGET%

IF ERRORLEVEL 1 (
    echo.
    echo BUILD FAILED!
    popd
    exit /b 1
)

echo.
echo ============================================
echo   BUILD SUCCESSFUL!
echo ============================================
echo.

REM --- Locate the output binary ---
SET OUTPUT_DIR=%EDK2_PATH%\Build\%PKG_NAME%\%TARGET%_%TOOL_CHAIN_TAG%\%ARCH%
echo Output: %OUTPUT_DIR%\HwTestApp.efi
echo.

REM --- Copy to project directory ---
IF EXIST "%OUTPUT_DIR%\HwTestApp.efi" (
    copy /Y "%OUTPUT_DIR%\HwTestApp.efi" "%~dp0HwTestApp.efi" >nul
    echo Copied to: %~dp0HwTestApp.efi
)

popd
echo.
echo Done.
