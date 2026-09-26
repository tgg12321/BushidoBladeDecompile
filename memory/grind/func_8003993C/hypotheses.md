# Hypothesis ledger — func_8003993C

## s2 (manual slotA3 2026-09-26)
- KILLED: scratch args as `(u8 *)K + i*N`, `i*N + K`, `(s32 *)K + i*M`, `&((u8 (*)[N])K)[i]` — all identical (lifetime 2 remains); a `scr` pointer local (`scr + 0xA8 + i*264` reassociates, keeps 0x1F800000 in a reg, addiu 168); three base-pointer locals (whole sum becomes a reduced giv).
- KILLED as insn_count levers (no RTL change): `(u16)*(s16 *)` reads, `hw` u16 local, p/rob respellings (`(u8*)D + a + b`, `(PracticeMenuRec *)((u8 *)&D_80101EC8 + i*0x44C)`), chained `rot[0] = rot[2] = 0`, mask/shift order swaps (`(f & 0x10) >> 4` etc. change bytes).
- KILLED for sel register: `(b17 & 2) != 0`, `? 1 : 0`, if/else 0/1 — all local-allocated.
- OPEN frontier #1: find +2 RTL insns in the per-player loop from an ordinary spelling that combine/flow removes (candidates not yet tried: struct-typed record (fields + bitfield flags byte), explicit (s32)/(u16) conversions on call args, per-block rereads).
- OPEN frontier #2: ANY ordinary (single-write) construct that makes the weapon-arm `key` global (untied, a0) and puts `sel` in s2 — or a policy ruling (see docs/grind/borderline.md 2026-09-26 func_8003993C entry).
- [s2c] Permuter campaigns (4, ~6.9k iterations total): perm1 (x1, shared sel/win) found the insn_count lever (restore via table index -> 256 RTL); perm2/perm3 found only volatile/new_var/split-shift carriers; perm4 from the ORDINARY 82 body (L7, 487 iters) found only alias carriers (`e = (u8 *)rob;` reusing the event pointer, `new_var = (u8 *)rob`/`p + 0x18`) — i.e. the only improvements are more pseudo-sharing, consistent with the allocation-priority proof. No ordinary improvement found.

## s3 (manual slotH 2026-09-26)
- CONFIRMED: Ruling 11 applies to `temp` (selector + window) and `entry` (two table-entry addresses); necessity by local-alloc for the selector and the if-arm entry (ruling11.md). Landing candidate = candidate.c (sandbox 0).
- KILLED (per-value + sanctioned constructs, all on v/pv.c): dead store, self-assign, `V + 1 - 1`, `V - V` use, do-while(0) inside the arm (empty / wrapping the reader), duplicated write, pointer alias -> 89..105; annihilating live use `(V & 1) >> 1` in the join block -> V live over the whole loop ($s7 / spilled), 58 / 71; pre-loop read of the unwritten V (UB) -> $s4 / $s6, 43 / 47.
