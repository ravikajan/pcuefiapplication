# WiFi UEFI Driver Notes (Offline Bundle)

There is no common universal WiFi UEFI `.efi` driver that works across all chipsets.

## Best practical vendor path

1. Use OEM firmware/vendor package for your exact platform:
   - Intel platform OEM (Dell/HP/Lenovo BIOS package with preboot modules)
   - Qualcomm/Realtek/Broadcom OEM firmware package
2. Extract the WiFi preboot `.efi` binary if provided by OEM.
3. Place it in this folder (`Drivers/WiFi`).
4. Run `build_local_bundle.bat` and then `make_usb.bat <DRIVE>`.

## What is commonly available online

- Common downloadable UEFI preboot drivers are usually Ethernet/SNP (already in `Drivers/Network`).
- WiFi preboot support is often firmware-integrated and not distributed as standalone `.efi`.

## Verification

At app boot, check the preload banner:
- loaded / started / failed counts

If WiFi test shows generic NIC mode + link down, firmware likely does not expose WiFi link in pre-OS.
