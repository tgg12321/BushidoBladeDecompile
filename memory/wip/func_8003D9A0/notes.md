# func_8003D9A0 — WIP (operator session 2026-08-04, post--mel)

Small loop function (60 insns): walks a `u32 *` packet list, per iteration
advances an s16 coordinate pair with wraparound at 0x200, calls
`initLoadImage(s1, s0, s4, s3)`, copies 24 bytes (`Copy24`) forward by 0x18,
steps s1 by 0x30.

## Where it stands
- Body bytes: ALL match target honestly (pins stripped) — including the
  lhu/lhu/addu/sh HImode add, sll/sra sign-extend, `slti 512`, the 4-arg
  call, and the 6-load/6-store Copy24 block.
- The 19-diff is 100% prologue/epilogue: frame 48 vs target 56 (phantom
  vars=8 — see meta root_cause; args=24 alternative refuted), save-offset
  shift from the frame delta, prologue interleave (target schedules
  `lh s4,0(s0)` / `lh s3,2(s0)` between register saves; ours packs saves
  contiguously), and `move s1,a2` placement (target: before `beqz`; ours:
  in its delay slot).
- Five pin-free spellings measured vars=0 (meta rejected_forms) via the
  fast `.frame` probe. The in-file witness tslLineG5Init proves vars=8 with
  zero cheats is reachable in this TU; its measured trigger (HImode bitwise
  pair) doesn't exist in this body, so find the slot's origin in RTL and
  work back to an equivalent C construct for THIS body.

## Cheats to retire on completion
pins ×2 + barrier (src), prologue_config.json entry (2 reorders — these
document the exact target order), frame_fix_funcs.txt `func_8003D9A0 56`.
Retiring the config entries needs the [infra-rule]-free path: delete lines,
full build SHA1 == oracle.

## Resume
Start at meta.next_avenues[0] (RTL slot-origin forensics). Instruments in
tmp/ (frame_probe.sh, fdiff.sh, v9a0.py) — regenerate from this note if
tmp/ was cleaned.
