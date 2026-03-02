@echo off
set WORKSPACE=E:\Project\edk2
set EDK_TOOLS_PATH=E:\Project\edk2\BaseTools
set BASE_TOOLS_PATH=E:\Project\edk2\BaseTools
set PYTHON_COMMAND=python
set NASM_PREFIX=C:\Users\ADMIN\AppData\Local\bin\NASM\
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86 >nul 2>&1
set PATH=E:\Project\edk2\BaseTools\Bin\Win32;%PATH%
cd /d E:\Project\edk2\BaseTools
nmake -f Makefile
