# Hypothesis ledger — func_80070188 (s1 name: replay_camera_attack)

## Ruled out (s2, 2026-09-29) — see evidence.md for scores
- u8-array spellings of the record accesses (7 forms, all 118) and a loop-body `rec`
  named intermediate (139): cannot give the target's index-sharing addressing.
- `s32 port` (154), `port` computed in the condition via comma (142).
- s.x chains other than `a*116 + K + b*20` / `K + a*116 + b*20` (20-36).
- `D_800A3560[1].unk0` in the cancel arm (12, register address) ; timers reversed (12).
- `D_800A35C8[i] = 0xF` (116; not the target's semantics anyway).

## Open
- Landing: full oracle build with the record merge in include/game.h + consumers.

## s2 later (2026-09-29)
- KILLED by layer-2: record merge (func_8006E534 word pun, prong (d)); inline slot
  accessors (un-annotated named-intermediate device). Union declaration: owner question
  (borderline.md 2026-09-29).
- Per-site FAKE named intermediates: everything but the slot-1 cancel arm (nomerge_best.c
  14). Open: a cancel-arm spelling with a register index that is not a Q22 dummy local.
- KILLED (2026-09-29): port_ofs in the cancel arm (72-77); all slot*3 product spellings
  (14); permuter campaigns A (region) and B (plain) — only Q22-class finds.
- OPEN: cancel-arm restructure with a real (non-fixed) index variable; union (owner).

## s3 (2026-09-29)
- CONFIRMED: slot-1 constant-address bytes as their splat scalars (D_800A3563/D_800A3565) close the
  cancel arm (14 -> 6 full TU, all GPREL-name); `k` no longer needed.
- KILLED: idx initializer at do-body top (8); `sel` statement before the if (19).
- KILLED (E534 side): no pun-free single-sw spelling under records/2-D (see evidence [s3]).
- LANDED 2026-09-29 (9e69a87c7 / 1ed9c8db5); union question open in borderline.md.
