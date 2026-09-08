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
. build/envsetup.sh
lunch pb_infiniti-eng
mka pbrp
```

If there is no error, recovery.img will be found in `out/target/product/infiniti/recovery.img`

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

## To use it:

```shell
fastboot flash recovery recovery.img
```

or

```shell
fastboot flash recovery_a recovery.img
fastboot flash recovery_b recovery.img
```
