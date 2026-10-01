# Hypothesis ledger — func_80075F80

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: INCLUDE_ASM (803d0fea1). `rejected/joint-l2-r1.c` scored 0 but layer-2 FAILED (e1c8266265a54942) on (a) `(&D_800A35D0) + (arg3 * 2)` - now fixed on main (`D_800A35D0[2][2]`) - and (b) `*(s32 *)(cancel_base + 0x3C)`, a cast word view that needs a union. The body is full of raw `state[0x3C / 2]` accesses and the deleted `SELWORK_800768DC` macro.
- CONSTRAINTS: same pun ban and `[2][10]` need as func_800759D0. The `lw 0x3C` word test at 0x80076060 must be a Q46 union member named only at that site (`.claude/rules/aggregate-declaration-views.md:49-66`; Q57 size round-up OK, SelWork is already 0x94). One function-scope `work` pointer costs 78 (target reloads D_800A36A0 per block): use `SELWORK->` directly or one pointer per block (the no-pointer ablation also scored 0).
- BLOCKER: two shared declarations (D_8009BCF8[2][10], f3C union); after that it is a SelWork respell of an already-matching body.
- PLAN:
  1. game.h: `s16 f3C[2];` -> `union { s16 half[2]; s32 word; } f3C;`.
  2. Respell the completed f3C users to `.half[...]`: func_800747D8 (8 uses), func_8007526C (2), func_80075670 (1), func_8007636C (6), func_800768DC (9) in this file; func_80074488 (4) in text1b_tu1e.c. Each needs sandbox 0 and a re-recorded layer-2.
  3. Body via members: `f10.half[arg3]` (cancel path sets `f10.half[1/0] = 3`, `f18[1/0] = 1`), `f14.half[(arg3 != 0) ? 0 : 1]` (as func_80075670), `f1C[arg3]`, `f20[arg3]`, `f3C.half[arg3]`, `f38`, `f60`, `f65`, `f48[arg3][i]`, `f3C.word` only at 0x80076060, `func_800692C0(..., SELWORK->f40[arg3], D_800A35D0[arg3])`, `D_8009BCF8[arg1][row * 5 + col].unk0`.
  4. Measure; joint layer-2 with func_800759D0; `queue done` both.
- DEPENDS: header changes -> f3C respells -> both bodies, one commit. Q65 timing as for func_800759D0.
- ODDS/LANE: within the joint 1-2 sessions, ~55% [I]. Manual.
