# Hypothesis ledger — func_8001A820

## [s2] 2026-09-26 manual slotL
- CONFIRMED: struct-typed scratch pointer (CamScratch *scr) over u8* casts (119 -> 81).
- CONFIRMED: duplicated head copy into both p arms (cross-jump) for the fighter select.
- CONFIRMED (diagnostic, construct needs policy review): halfword read into a u16 local before the intervening store (yaw: e6.c 75; roll: f4.c 36).
- CONFIRMED (construct needs policy review): final `yaw = math_SignExt12Div(yaw - base_yaw, 8); cam->h10 += yaw;` (f1.c 41) — makes `yaw` the cse head.
- CONFIRMED: `q = 0x2000000U / (dist + 0x4000) + 0x400;` one statement (L1.c 22).
- KILLED (lh stays): `(s16)(x & 0xFFFF)`, `((s32)(x<<16))>>16`, `s16 t = x` local, `u16 t = x` local without an intervening store (e1/e3/e4/e5 all 81); packed `u32 unk58` `(s16)(w >> 16)` and `s32 (w >> 16)` for 0x5A (h1/h2 36).
- KILLED: q/zoom split alone (v3a 81), q declared s32 (L3 36), `(q + dist)` order (L2 36), `dist += q` (L4 37); base_yaw declared s16 (v3b 87); decl-order swaps of yaw/i (f2/f3 75).
- OPEN: the three 0x5A sites — what separates the load from its extension in the original RTL (no store/call visible between them in target).
- OPEN: natural spellings for the yaw/roll read-before-store and the final yaw reuse (policy: Ruling 11 / multi-write) — see frontier.
