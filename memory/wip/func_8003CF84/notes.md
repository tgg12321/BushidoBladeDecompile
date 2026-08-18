# func_8003CF84 — WIP (current state 2026-08-18)

**Tree state: RESTORED TO HEAD.** No edit left in `src/code6cac_c2.c`.

## Measured

| form | frame | vars | sandbox `--disable all` |
|---|---|---|---|
| HEAD (`volatile s32 pad[4]` + `volatile s32 pad2[2]`) | 72 | 40 | 28 (both pads are cheat-stripped) |
| sanctioned pad only (`volatile u32 pre_pad[4]`) | 64 | 32 | **8** |
| no pad at all | 48 | 16 | 28 |

Instrument: `.frame` gradient from cc1 (`tmp/frameprobe.py`), plus the instrumented
cc1 slot census (`BB2_FRAME_DEBUG=1`, `tools/gcc-2.7.2/cc1`).

## The finding — the pad allowlist covers only HALF the frame defect

Target frame 0x48 = args 16 + **vars 40** + saves 16; `vec` sits at `sp+0x20`.
Slot census of the matching HEAD form: three BLKmode slots — 16 (`pad`), 16
(`vec[3]`, 12 rounded to 8-alignment), **8** (`pad2`).

- The sanctioned `("pre_pad", 4)` allowlist row (engine/volatile_cheats.py) supplies
  the **leading 16 bytes** (`sp+0x10..0x1F`, the fossilized outgoing-args partition —
  the 3-function carve-out of the 2026-08-17 owner ruling). With it alone the build is
  **208/208 instructions and byte-identical except the prologue/epilogue sp offsets**:
  `addiu sp,-64` vs `-72`, and the three saves/restores 8 lower. `vec` is already at
  the right offset. That is the whole residual 8.
- The remaining **8 bytes sit ABOVE `vec`** and are a SECOND, distinct object the
  ruling does not mention. No partition hypothesis removes it: args=16/24/32 all leave
  the same trailing 8.

## Killed levers for the trailing 8 (all measured `vars=32`, i.e. no effect)

Named s16/s32/u8 locals for the multiply-read globals (`D_8008EAC0[stage]`,
`D_800A38DC`, `D_800A3836`), pointer local for the 0x44C-stride record, `u8*` RMW for
the `D_800A37D2` counter, named `dx/dy/dz` delta locals, dropping the `vp` alias,
named pointer locals for the three `func_8001979C` args, `vec[4]`, narrow re-use of
`stage`, naming the `func_80054F68` result. 14 spellings, none moves `vars`.

Per [[phantom-slot-frame-lever]] the honest producer would be one unallocated pseudo
(`alter_reg` → `spill_new_pN`, 8 bytes, zero instructions). This function has no loop
(producer 1 unavailable), no algorithmically-required narrow second use (producer 2),
and every named-local form above (producer 3) register-allocates cleanly.

## Disposition

Needs an owner call, not more grinding: either extend the carve-out to the trailing
pad (HEAD's `pad2[2]`, i.e. a second allowlist row) or accept the endgame lock. Do NOT
ship the sanctioned pad alone — it changes live bytes and breaks the oracle.
