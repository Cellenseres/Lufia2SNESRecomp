# Decomp verification

The development-only `decomp-verify` target compares reconstructed game
semantics against the original ROM bytes with SNESRecomp's `interp816`
reference CPU.

It is not linked into `Lufia2Recomp`, is excluded from normal builds, and is
not part of release packaging.

## Run

After a normal configured Windows build tree exists:

```powershell
cmake --build build/windows --config Release --target decomp-verify
```

The target builds only the verifier and writes:

```text
build/windows/generated/DECOMP_VERIFY_REPORT.txt
```

The first registered comparison is `$83:BBF3`. It checks all 256 combinations
of the branch-relevant input bits plus 65,536 deterministic randomized cases.
For every case the native semantic routine is compared with the original ROM
routine executed by `interp816`.

The comparison currently covers:

- selected child action or normal return;
- accumulator low byte;
- N and Z flags;
- original data-read order and values.

The verifier stops at child-function entry points and treats them as opaque
boundaries. CPU-stack and child-return ABI verification remains a separate
consumer-bridge concern and is not claimed by this first semantic verifier.

## Reuse

`snes_function_verify.c/.h` contains no Lufia game logic. It provides a small
LoROM/WRAM reference bus and bounded `interp816` execution helper intended to
be reusable by additional SNESRecomp projects. Game-specific adapters live in
separate verifier translation units.
