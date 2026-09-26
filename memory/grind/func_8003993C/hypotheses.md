# Hypothesis ledger — func_8003993C

## s2 (manual slotA3 2026-09-26)
- KILLED: scratch args as `(u8 *)K + i*N`, `i*N + K`, `(s32 *)K + i*M`, `&((u8 (*)[N])K)[i]` — all identical (lifetime 2 remains); a `scr` pointer local (`scr + 0xA8 + i*264` reassociates, keeps 0x1F800000 in a reg, addiu 168); three base-pointer locals (whole sum becomes a reduced giv).
- KILLED as insn_count levers (no RTL change): `(u16)*(s16 *)` reads, `hw` u16 local, p/rob respellings (`(u8*)D + a + b`, `(PracticeMenuRec *)((u8 *)&D_80101EC8 + i*0x44C)`), chained `rot[0] = rot[2] = 0`, mask/shift order swaps (`(f & 0x10) >> 4` etc. change bytes).
- KILLED for sel register: `(b17 & 2) != 0`, `? 1 : 0`, if/else 0/1 — all local-allocated.
- OPEN frontier #1: find +2 RTL insns in the per-player loop from an ordinary spelling that combine/flow removes (candidates not yet tried: struct-typed record (fields + bitfield flags byte), explicit (s32)/(u16) conversions on call args, per-block rereads).
- OPEN frontier #2: ANY ordinary (single-write) construct that makes the weapon-arm `key` global (untied, a0) and puts `sel` in s2 — or a policy ruling (see docs/grind/borderline.md 2026-09-26 func_8003993C entry).
