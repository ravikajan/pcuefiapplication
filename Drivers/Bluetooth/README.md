# Bluetooth UEFI Driver Notes (Offline Bundle)

Bluetooth pre-OS support in UEFI is highly platform-specific.

## Recommended approach

1. Get OEM preboot Bluetooth `.efi` driver for your exact hardware/BIOS.
2. Place the binary in `Drivers/Bluetooth`.
3. Build/package with:
   - `build_local_bundle.bat`
   - `make_usb.bat <DRIVE>`

## Important

- There is no widely available universal Bluetooth UEFI `.efi` package for all PCs.
- Many systems expose Bluetooth only after OS driver load.
