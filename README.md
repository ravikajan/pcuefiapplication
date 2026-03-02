# UEFI Hardware Test Suite

A **stand-alone UEFI application** (`BOOTX64.EFI`) that boots on x86-64 PCs from USB and runs automated hardware diagnostics — **no operating system required**.

Designed for PC resellers, refurbishers, repair technicians, and IT departments to validate hardware quickly and reliably at the firmware level.

## Features

| Feature                  | Description                                               |
| ------------------------ | --------------------------------------------------------- |
| **Pre-OS Boot**          | Runs directly on UEFI firmware, no OS needed              |
| **Interactive UI**       | Keyboard-navigable menus, progress bars, PASS/FAIL badges |
| **8 Test Modules**       | CPU, Memory, Storage, Display, Network, WiFi, USB, Bluetooth |
| **3 Test Profiles**      | Full Test, Quick Test, Custom Selection                   |
| **Report Export**        | JSON, CSV, TXT reports saved to FAT32 filesystem          |
| **Modular Architecture** | Easy to add/modify test modules                           |

## Hardware Tests

| Test          | What it does                                                       |
| ------------- | ------------------------------------------------------------------ |
| **CPU**       | CPUID vendor detection, integer arithmetic integrity, stress loops |
| **Memory**    | UEFI memory map enumeration, walking-bit pattern tests             |
| **Storage**   | Block I/O device enumeration, media info, read verification        |
| **Display**   | GOP mode listing, per-color manual validation, user PASS/FAIL note |
| **Network**   | Simple Network Protocol detection, MAC, link status                |
| **WiFi**      | WiFi-capable adapter scan/list and link/connectivity diagnostics    |
| **USB**       | Interactive USB port check (baseline, plug-in detect, PASS/FAIL)  |
| **Bluetooth** | Bluetooth HC Protocol detection                                    |

## Prerequisites

- **TianoCore EDK II** — cloned and buildable ([github.com/tianocore/edk2](https://github.com/tianocore/edk2))
- **Visual Studio 2019 or 2022** (for `VS2019` / `VS2022` toolchain) or GCC
- **NASM** assembler (required by EDK II)
- **Python 3** (required by EDK II build system)
- **QEMU** + **OVMF** firmware (for testing without real hardware)

## Quick Start

### 1. Clone EDK II

```batch
git clone https://github.com/tianocore/edk2.git C:\edk2
cd C:\edk2
git submodule update --init
```

### 2. Set Environment

```batch
set EDK2_PATH=C:\edk2
set TOOL_CHAIN_TAG=VS2019
```

### 3. Build

```batch
cd e:\Project\pcuefiapplication
build.bat
```

This creates `HwTestApp.efi` in the project directory.

### 4. Test in QEMU

```batch
run_qemu.bat
```

### 5. Deploy to USB

```batch
REM Format USB as FAT32 first, then:
make_usb.bat E
```

Where `E` is your USB drive letter.

Optional: download common UEFI network drivers first:

```batch
download_common_drivers.bat
```

`make_usb.bat` will copy `Drivers\*.efi` into `EFI\BOOT\DRIVERS` and `EFI\HWTEST\DRIVERS`.
At startup, the app auto-loads drivers from these folders before running tests.

Offline/local bundle flow (recommended):

```batch
build_local_bundle.bat
make_usb.bat E
```

This uses only local prebuilt driver binaries in `Drivers\`.
No driver download is performed at boot time.

### 6. Boot on Real Hardware

1. Insert USB into target PC
2. Enter BIOS/UEFI setup (F2, F12, or DEL)
3. Disable Secure Boot (if enabled)
4. Select USB as boot device
5. Hardware Test Suite starts automatically

## Project Structure

```
pcuefiapplication/
├── HwTestPkg.dec          # EDK II package declaration
├── HwTestPkg.dsc          # EDK II build configuration
├── HwTestApp.inf          # Module definition (sources, protocols)
├── build.bat              # Build script
├── run_qemu.bat           # QEMU test launcher
├── make_usb.bat           # USB deployment script
└── Src/
    ├── Main.c             # Entry point & menu loop
    ├── Utils/
    │   ├── Console.h/c    # Colored text I/O, header banner
    │   ├── Timer.h/c      # Timing & delays
    │   └── String.h/c     # String formatting helpers
    ├── Ui/
    │   ├── Graphics.h/c   # GOP framebuffer access
    │   ├── Menu.h/c       # Interactive menu system
    │   ├── Progress.h/c   # Progress screen
    │   └── Results.h/c    # Results summary screen
    ├── Tests/
    │   ├── TestModule.h   # Test module interface
    │   ├── CpuTest.c      # CPU diagnostics
    │   ├── MemoryTest.c   # Memory diagnostics
    │   ├── StorageTest.c  # Storage diagnostics
    │   ├── DisplayTest.c  # Display diagnostics
    │   ├── NetworkTest.c  # Network diagnostics
    │   ├── WifiTest.c     # WiFi diagnostics
    │   ├── UsbTest.c      # USB diagnostics
    │   └── BluetoothTest.c# Bluetooth diagnostics
    ├── Runner/
    │   └── TestRunner.h/c # Test orchestration
    └── Report/
        └── Report.h/c     # Report generation & export
```

## Adding a New Test Module

1. Create `Src/Tests/MyTest.c` implementing the `HW_TEST_MODULE` interface:

```c
#include "TestModule.h"

EFI_STATUS EFIAPI MyTestRun (HW_TEST_MODULE *Self, HW_TEST_RESULT *Result) {
  // Your test logic here
  Result->Status = TestStatusPass;
  return EFI_SUCCESS;
}

HW_TEST_MODULE gMyTestModule = {
  L"My Test", L"Description", TRUE, MyTestRun
};
```

2. Add the source to `HwTestApp.inf` under `[Sources]`
3. Register in `TestRunner.c`:
   ```c
   extern HW_TEST_MODULE gMyTestModule;
   // In TestRunnerInit():
   RegisterModule (&gMyTestModule);
   ```

## Architecture

```
┌──────────────┐
│   Main.c     │  Entry point & menu loop
├──────────────┤
│   UI Layer   │  Menu, Progress, Results (Console + GOP)
├──────────────┤
│  TestRunner  │  Orchestrates Full/Quick/Custom profiles
├──────────────┤
│ Test Modules │  CPU | Mem | Storage | Display | Net | WiFi | USB | BT
├──────────────┤
│   Report     │  JSON / CSV / TXT → FAT32 filesystem
├──────────────┤
│   Utilities  │  Console, Timer, String helpers
├──────────────┤
│  UEFI/EDK II │  Boot Services, Protocols, GOP
└──────────────┘
```

## License

BSD-2-Clause-Patent (same as EDK II)
