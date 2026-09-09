# PBRP device tree for OnePlus 15 (infiniti)

## Supported devices

- OnePlus 15

## Build it yourself?

```shell
mkdir pbrp && cd pbrp
repo init -u https://github.com/PitchBlackRecoveryProject/manifest_pb -b android-12.1
repo sync
git clone https://github.com/travismills82/pbrp_device_oplus_infiniti device/oplus/infiniti
```

```shell
. device/oplus/infiniti/scripts/apply-recovery-patches.sh
. build/envsetup.sh
lunch pb_infiniti-eng
WITH_BUNDLED_MAGISK=true make recoveryimage -j2
```

If there is no error, recovery.img will be found in `out/target/product/infiniti/recovery.img`

The patch script carries the required PitchBlack 12.1 framework, interface,
and vold compatibility changes in this device repository. It is safe to run
again and will stop if the synced source is incompatible.

## Features

Works:

- [X] ADB
- [X] Display
- [X] Decryption
- [X] Fastbootd
- [X] Flashing
- [X] OTA Flash
- [X] MTP
- [?] Sideload
- [X] Touch
- [X] USB OTG
- [X] Vibrator
- [X] Battery and charging status
- [X] Rear camera flashlight
- [X] Bundled Magisk installer

## To use it:

```shell
fastboot flash recovery recovery.img
```

or

```shell
fastboot flash recovery_a recovery.img
fastboot flash recovery_b recovery.img
```
