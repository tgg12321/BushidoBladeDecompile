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
- CONFIRMED (cc1psx calibration + micro-tests): the five scratchpad halfword sites are a fold difference between our cc1 and Sony's cc1psx; the target agrees with cc1psx. Owner question filed (borderline.md 2026-09-26).
- KILLED for 0x5A (our cc1): every spelling folds to `lh` (micro_all.c: tA/tE/tG/tH/tB/tF, in-situ h1/h2). No ordinary barrier exists at those sites in the target.
- KILLED: FAKE self-assign `yaw = yaw;` as a split-spelling substitute for the yaw reuse (S2 56).
- KILLED: hi/lo split of `other` (Nb/Nb2 70); block-scoped hi/lo/i/d together (N2 62).
- NEUTRAL (adopted): i/j split (Nc 33), block-scoped d (Na 33).
- FRONTIER: (1) owner answer on the fold-fidelity question — with Sony-like folding N4 should reduce to the reloc artifact only (certify with the oracle); (2) Ruling 11 packages for `yaw` and `other` (dumps: tmp/func_8001A820/alloc.py = BB2_ALLOC_DEBUG/BB2_FINDREG_DEBUG on the instrumented cc1, mk.py = rtl_track dumps; measure chain-extender / pointer-alias / duplicated-into-arms variants per lesson 10); (3) permuter NOT run this session (whole-function 576-insn workspace not built) — the 0x5A mechanism analysis says it cannot help, but run it before any "no C form" claim goes to layer-2.
