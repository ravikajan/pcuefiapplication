# Optional Driver Folder

Place optional UEFI driver binaries (`*.efi`) here.

This project uses an offline/local driver model:

- Boot-time app does NOT download drivers from internet.
- App only loads local prebuilt binaries copied on USB.

Use `download_common_drivers.bat` only as a local preparation step to fetch common prebuilt network binaries.
You can also manually place vendor-compatible WiFi/Bluetooth/NIC `.efi` drivers in subfolders:

- `Drivers/Network`
- `Drivers/WiFi`
- `Drivers/Bluetooth`

Then run `build_local_bundle.bat` and `make_usb.bat <DRIVE>`.

During USB deployment, drivers are copied to:

- `EFI/BOOT/DRIVERS`
- `EFI/HWTEST/DRIVERS`

The app auto-loads drivers from those paths at startup before tests run.
