# Known issues

- H-only IRQ timing has not been validated.
- Save states are disabled until the hybrid CPU resume context is serialized.
- Soft reset is disabled while the current reset path assumes an HLE SPC player.
