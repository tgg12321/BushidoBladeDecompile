---
name: gnd-init-80041688-frame-coercion-evidence
description: gnd_init_80041688 follow-up evidence for the forbidden 0x38 frame-size gap and rejected phantom-struct loophole.
---

# gnd_init_80041688 — frame-coercion evidence

Date: 2026-06-05

## Summary

`gnd_init_80041688` is blocked by a target frame-size gap, not by the three
RGB-load-order regfix rules alone. HEAD gets the target 0x38 frame via:

```c
volatile s32 sp10[8];
...
(void)sp10;
```

That is forbidden frame coercion per `dead-vars-local-array.md`. The target asm
has no stack references except `ra`/`s0` save/restore, so there is no observed
semantic local traffic corresponding to the 0x20 local area.

## Measurements

All measurements are against `build/src/text1a.o` with `gnd_init_80041688`
rules disabled.

| Variant | Score | Insns | Frame | Cheat count | Outcome |
|---|---:|---:|---|---:|---|
| clean scalar, no `sp10` | 8 | 82/82 | 0x18 | 0 | Honest floor; frame is too small |
| false branch loads `b` first | 8 | 82/82 | 0x18 | 0 | Folds back; RGB order unchanged |
| RGB accumulator forms | 15-16 | 80/82 | 0x18 | 0 | Worse; expression shrinks |
| single `GndColor`/struct locals | 15-34 | 84-87/82 | 0x20-0x28 | 0 | Extra instructions / wrong frame |
| exact-frame real aggregate locals | 9 | 87/82 | 0x38 | 0 | Frame matches but emits stack stores |
| unused local array | 2 | 82/82 | 0x38 | 1 | Forbidden; old family of workaround |
| unused struct wrapping `s32 w[8]` | 2 | 82/82 | 0x38 | 0 | Detector loophole; still forbidden |

The exact-frame real aggregate cases (`struct_s32_exact_frame`,
`s32_color_array_exact_frame`, `cvector_array_exact_frame`) emit stack stores in
the false-color path, e.g. `sw ..., 16(sp)`, `sw ..., 20(sp)`, `sw ..., 24(sp)`.
Those stores are absent from target asm. They are not viable matches.

## Rejected loophole

An unused local struct:

```c
struct { s32 w[8]; } scratch;
```

currently reports cheat count 0 but produces the same phantom 0x38 frame as the
forbidden unused array. It has no semantic purpose and must be treated as the
same frame-coercion cheat-by-spelling, even if the current detector does not
catch it. Do not use this as a completion path.

## Current conclusion

No SOTN-clean C path was found. The best honest source remains the clean scalar
form at score 8, with the entire frame-size gap exposed. Completing this
function requires either a real semantic source construct that reserves 0x20
bytes without producing stack traffic, or a future policy/tooling decision.
