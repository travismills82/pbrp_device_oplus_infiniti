# PitchBlack source patches

The OnePlus 15 ships Android 16 vendor services that are newer than the
Android 12.1 base used by PitchBlack. These patches keep the compatibility
work in the Infiniti device repository instead of requiring unpublished
changes in the surrounding source checkout.

Run the stack once after syncing PitchBlack and before building:

```shell
device/oplus/infiniti/scripts/apply-recovery-patches.sh
```

The script is idempotent: it reports patches that are already applied and
stops if a source revision is incompatible. The patch headers document why
each platform change is required.

Reference project revisions:

- `bootable/recovery`: `c3ee88af57d32a842b2a8ff861a72a7bf24da846`
- `hardware/interfaces`: `ae469cee0dce6d71489588126a15da8e67a50102`
- `system/vold`: `3a05a189e3a5e77fe21eb809d343816aa1177591`
- `vendor/pb`: `31bf0d4cd64bd125a8e5a2e3e54367b9344a515a`

Patch groups:

- `bootable-recovery`: decryption readiness, AIDL boot control, dual-channel
  camera torch support, and the ramdisk-local Magisk action.
- `hardware-interfaces`: recovery-local Gatekeeper AIDL v1 definitions.
- `system-vold`: Android 16 Gatekeeper, Weaver, KeyMint, and FBE compatibility.
- `vendor-pb`: export of the secondary torch path to Soong.
