# Hypothesis ledger — func_80044010

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: active; INCLUDE_ASM (`src/text1a_c.c:1041`), 34 insns. Banked `preauth_body.c` takes raw s32 params as a disclosed measurement shim (conflicting extern at `src/text1a_c.c:2109`). Authorized in 19d75899c (replacing `volatile s32 sp_pad;`), de-authorized by the 2026-10-01 inline-asm audit.
- CONSTRAINTS: same as save_vc_ctrl; no canonical asm admissible.
- BLOCKER [I, strong]: same phantom 8-byte frame. Target puts the frame alloc in the `bnez` delay slot (`asm/funcs/func_80044010.s:20`); banked body emits nops there. Banked loop is `if (n > 0) { i = 0; do ... while (i < n); }` - no folded guard, no phantom slot. Near-identical completed exhibits: func_800400B0 (`src/config.c`, `if (p) { for (i = 0; i < p[0]; i++) ... }`, same `beqz` + `addiu sp,-8` delay slot, `blez` guard, `sp+8 / jr / nop` epilogue), func_8004954C, func_8004001C.
- PLAN:
  1. Candidate with a natural `for (i = 0; i < n; i++) *p++ += (s32)base;` (`u16 n = hdr;` or `n = hdr & 0xFFFF`), keeping `*a0 = (u16)(hdr | 0x8000)` and the `D_80103608[slot]` / `D_80103658[slot]` writes.
  2. Honest prototype `void func_80044010(s32 *, s16)` (already used by `src/text1a_pre.c:23`, `src/sound.c:107`): fix the extern at `src/text1a_c.c:2109` and the casts at the :2212/:2215 call sites in completed func_80045B68 (bytes likely unchanged [I], but its body hash changes - include it in the layer-2).
  3. If the for-loop misses: producer 2 (the `s16 slot` sign-extend stranded at a label).
  4. Leave func_80044098 alone (reverse op, no frame, own approved FAKE).
- DEPENDS: after/with save_vc_ctrl.
- ODDS/LANE: 1-2 hours, high [I]. Manual.

## 2026-10-01 laneB — 0 in plain C (candidate.c)
- `s32 *base = p; hdr = *p; *p = (hdr | 0x8000) & 0xFFFF; p++; ...; u16 n = hdr; for (i = 0; i < n; i++)
  *p++ += (s32)base;` with the honest prototype `(s32 *p, s16 slot)`: sandbox 0 (34/34) on the spliced
  src (the TU's extern fixed to `(s32 *, s16)`, func_80045B68's two calls lose/gain their casts; it stays
  0 and the full build is byte-identical). m1001/ variants (raw-param shim, same codegen): a separate
  `p = hdr + 1` (q1-q6) 9-19, `n` as s32 / `(u16)` in the test / do-while (r2-r4, r6) 10-13,
  `(u16)(hdr | 0x8000)` store (r5) 2; preauth_body.c 3 (no frame).
- Frame: producer 1 (rotated guard). The loop's entry test `slt i, n` (pseudo 101, i = 0) is folded by
  combine into `blez n`; its REG_DEAD note is left as a bare `(use (reg:SI 101))` (.lreg insn 112),
  reload slots it: `.frame $sp,8 ... vars= 8` (save_vc_ctrl/m1001/frame.sh + combine.sh).
