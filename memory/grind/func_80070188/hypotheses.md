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
