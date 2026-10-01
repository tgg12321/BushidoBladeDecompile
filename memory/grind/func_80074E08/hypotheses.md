# Hypothesis ledger — func_80074E08

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: INCLUDE_ASM (revert 5f8e7c52b). Banked 0/281 body `rejected/retro-audit-2026-09-29.c` uses one `s8 *table` written 4x (`(s8 *)s.header + 0xC` for records[3], [2] in loop, [0], [1] after), each read once. Every plain per-site spelling 12/281 (`ff-c-2026-09-30/v1..v5`). Failed Ruling 5 ext. prong (C); the retro-audit's class B "re-prove under Ruling 11" package was never built.
- CONSTRAINTS: Ruling 5 ext. prong (C) `.claude/rules/reused-local-one-role.md:66-67`; no SOTN Q51 citation exists (3 scans); Ruling 9 needs `VAR = BASE + K` with NO cast (`.claude/rules/reused-local-meaning-source.md:36-37`) - the current `EnvA` typing forces the cast.
- BLOCKER: one multi-block `table` -> global alloc -> $v1; per-site locals -> local alloc -> $v0 (the whole 12). [I] Same shape sibling func_8007636C was admitted under Ruling 9 (71b14499d, `cells`, same +0xC sheet-cell meaning). Ruling 9 (c3e7a0b9e, 09-25) post-dates this function's landing, so it was never tried.
- PLAN:
  1. Respell the banked body with the TU's `S_80074488` (all-s32, the 8007636C spelling): `s32 *table = *(s32 **)(arg0[0] + 0x18);`, per site `s.sp18 = table[N]; cells = s.sp18 + 0xC; s.sp1C = cells;`, `s.sp1C += *(u8 *)(s.sp18 + 2) * 8;`. Delete the then-unused `EnvA` typedef (`src/text1b_tu2.c:75-89`).
  2. Same step: port work-area reads to SelWork (4fd37366e removed every raw `D_800A36A0 + off`): `+arg1*2+0xC` -> `SELWORK->f0C[arg1]`, `+8` -> `f08[arg1]`, `+0x24` -> `f24`. Target uses `lh` for the first f0C use and `lhu` later (a `(u16)` value cast is ordinary C). `f24` is really a DRAWENV* (`include/gpu.h:71-80`): retype in game.h (touches completed func_80077724) or cast per use; the target reloads `lw 0x24` before every read, so don't hoist it.
  3. `sandbox --diff`, expect 0 [I].
  4. Ruling 9 (b) evidence: run `memory/grind/func_800759D0/s0930/sheet_census.py` over root+0x18 entries [0..3] (D_SEL.BIN 0x330-0x344): one header per sheet, so +0xC is the first cell.
  5. Receipts exist (per-site 12/281, respellings, 35,979-iteration permuter, allocation receipt) but their dumps were in tmp/ - regenerate before citing. Fresh layer-2, `queue done`.
- DEPENDS: independent. If f24 is retyped, re-verify func_80077724.
- ODDS/LANE: ~1 session, ~65% [I]. Manual (header + Ruling 9 layer-2). First in the text1b_tu2 cluster.
