@echo off
set PYTHON_COMMAND=python
set PATH=E:\Project\edk2\BaseTools\Bin\Win32;%PATH%
set EDK_TOOLS_BIN=E:\Project\edk2\BaseTools\BinWrappers\WindowsLike
set NASM_PREFIX=C:\Users\ADMIN\AppData\Local\bin\NASM\
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64 >nul 2>&1
cd /d E:\Project\edk2
call edksetup.bat >nul 2>&1
build -p HwTestPkg/HwTestPkg.dsc -a X64 -t VS2022 -b RELEASE
