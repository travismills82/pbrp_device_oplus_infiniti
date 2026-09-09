# Bundled Magisk payload

The Infiniti recovery packages the verified Magisk APK as a recovery ZIP at:

```text
/system/etc/magisk/Magisk.zip
```

PitchBlack's Tools > Magisk action flashes this ramdisk-local package, so the
root installer does not depend on `/data` decryption or files under
`/sdcard/PBRP/tools`.

Pinned payload metadata:

- Package: `com.topjohnwu.magisk`
- Version: `30.7` (`30700`)
- SHA-256: `e0d32d2123532860f97123d927b1bb86c4e08e6fd8a48bfc6b5bee0afae9ebd5`
- Signer certificate SHA-256: `b4cb83b4dad99f997dbe872f013aa16c14eec41d167021f371f7e1330f273ee6`

Verify the APK, signer, recovery installer entries, and redistribution terms
before replacing this payload.
