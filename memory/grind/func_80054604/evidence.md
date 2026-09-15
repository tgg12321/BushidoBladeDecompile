# Evidence bank — func_80054604

## s1 (2026-09-15, recon) — baseline 160 -> honest floor 26

- CHASSIS: canonical verdict C (asm_insns 0, hand_coded tier LOW). sandbox --disable all on the
  committed INCLUDE_ASM = 160 (whole function). Function = game_StageSetup per named_syms.txt:3671
  (7 params: a0 stage index (+0x131 = selection id), a1, a2..a5 stored to ctrl+0x44..0x4A as s16,
  a6 = optional data buffer pointer). Returns the offset from func_80044FA0 (0 when a6 == 0).
- OBJECT MODEL: D_800EFAE8 — MISMATCH-unmeasured against headers (no declaration anywhere); the
  target addresses it through a base register (s1 = lui/addiu D_800EFAE8, offsets 0x00,0x02,0x04,
  0x08,0x0C,0x10,0x14,0x1C,0x1E,0x20,0x2C,0x44,0x46,0x48,0x4A) = base-register evidence for a
  struct (aggregate-merge prong (a) satisfied by the binary itself). Measured with a TU-local
  `StageCtrl` typedef (0x4C bytes) + `StageCtrl *s = &D_800EFAE8`: score 26 (candidate.c). The
  same typedef used DIRECTLY (`D_800EFAE8.field`, no pointer local): score 82 — GCC materialises
  the address per extended block instead of once in the prologue (rejected/direct-global-no-pointer-
  local-82.c). D_800EFB14 = D_800EFAE8+0x2C — MATCHES as a SEPARATE plain extern for the two
  stores (target stores it via lui $at / sw %lo(D_800EFB14) in both branches, NOT via s1+0x2C) while
  the two reads go through s1+0x2C (struct field `data`); v2 reproduces both byte patterns exactly.
  This is the mixed-handle shape and is part of why a struct pointer local is needed. Other TU
  consumers (text1b.c:1588-1613, 1970-2765) declare D_800EFB0C/14/18..28/38 as per-word scalars —
  aggregate-merge prong (c) work for the closing session. D_800A3770 — MATCHES as `extern s32
  D_800A3770` used address-of only (lui/addiu a1 in the a6==0 cleanup arm). D_80015840 — MATCHES
  (const char[] string in text1a_b_pre_rodata.c:319, passed to printf).
- v1 (shared `tmp` local for the two func_8004153C results AND the cleanup index): 32. The shared
  pseudo crosses gpu_EnableDisplay in its third live range, so global.c gives it s0 and the two
  `if (v) func_8003FFC4(v)` sites get `move s0,v0` instead of using v0 directly. Separate locals
  (`v` for the two result tests, `n` for the cleanup index): 26. Target uses v0 directly.
- v2 residual (26) is ENTIRELY the callee-saved permutation: target s2=a6 s3=a1 s4=ret s7=a4 fp=a5;
  ours s2=a1 s3=ret s4=a6 s7=a5 fp=a4, plus the prologue lw order (target lw a6,a4,a5; ours a4,a6,a5)
  which is a consequence of the hard regs (sched2 orders lw sN behind sw sN). Body otherwise
  instruction-identical incl. the /360 magic-multiply (sign fix folded by combine: nonzero_bits of
  ((f>>8)&0x7F)<<14 has bit31 clear), the `*t++` triple (lh 0; addiu 2; lh 0; lh 2), the D_800EFB14
  re-read before `sw 4(s1)`, and both if-arms.
- PASS ATTRIBUTION (dumps in tmp/grind/func_80054604/dumps, lreg/greg read): global.c allocno_compare
  (tools/gcc-2.7.2/global.c:642) priorities from the .lreg "used N times across L insns" lines:
  id(80) 6153 > ctrl(79) 4285 > a1(73) 860 [n4,L93] > ret(81) 808 [n4,L99] > a6(78) 588 [n6,L204]
  > a2(74) 322 [n2,L62] > a3(75) 317 [n2,L63] > a5(77) 163 [n2,L122] > a4(76) 161 [n2,L124].
  Allocation order = this list (first free callee-saved reg each), which reproduces our s0..fp
  exactly. TARGET ORDER REQUIRES: a6 > a1 > ret > a2 > a3 > a4 >= a5 (tie -> lower allocno = a4).
- MECHANISM 1 (a6 too low): the three stack params carry REG_EQUIV notes (function.c:3824-3855,
  stack parm copied at fixed offset) and local-alloc.c:1064 DOUBLES reg_live_length of any
  single-set pseudo with a REG_EQUIV note (sched1 measured a6 L=102 via BB2_SLL_DEBUG; lreg shows
  204). With n=6 that is 588 < a1's 860. To beat a1 (n4,L93 — both fixed by the target's own
  uses: sll/addu for a1*6 + sh 8(s1)) a6 needs n>=8 (24/204 -> 1176) or must escape the doubling
  (gate local-alloc.c:1024 `reg_n_sets[regno] != 1` -> 12/102 -> 1176).
- MECHANISM 2 (a4 below a5): sched1 (reverse list scheduling, tie = higher LUID first,
  sched.c:2460) fills the load-delay slot before beqz with a5's lw (14), then a6's lw (16) at T-3,
  a4's (12) at T-4 -> post-sched1 order a4,a6,a5 -> L(a4)=62, L(a5)=61 -> doubled 124/122 -> a5 wins.
  Target needs L(a4) <= L(a5): a6's lw must not sit between them (or an insn must sit between the
  a4/a5 stores).
- PROBES (all v2-relative, instance results on this chassis, no FAKE present):
  * dead `a6 = 0;` after last use: 26, allocation unchanged (flow deletes the dead set before
    local-alloc counts reg_n_sets) -> not a route to undoubling.
  * a6 typed `u8 *`: 26, unchanged. * `a0 += 0x131` instead of `id` local: 26, unchanged.
  * a1 typed s16: 36 (worse). * a4,a5 typed s16: 32 — the prologue copies vanish (did_conversion
    path loads at the use site; a2/a3 move to s7/fp) -> wrong shape.

- [s1] OBJECT MODEL: D_800EFAE8 MISMATCH-unmeasured vs headers (no decl); base-register struct evidence in the binary; TU-local StageCtrl + pointer local measured 26, direct global 82. D_800EFB14 MATCHES as a separate plain extern for the stores (target uses lui $at/sw %lo) and as struct field +0x2C for the reads. D_800A3770 MATCHES as extern s32 (address-of only). D_80015840 MATCHES (existing const char[] string).

- [s1] canonical: verdict C, hand_coded tier LOW, distance 160 (size, not asm evidence).

- [s1] global.c allocno_compare priorities from .lreg (n refs, L insns): id 6153, ctrl 4285, a1 860 (4,93), ret 808 (4,99), a6 588 (6,204 = doubled from 102 by local-alloc.c:1064 REG_EQUIV rule), a2 322 (2,62), a3 317 (2,63), a5 163 (2,122), a4 161 (2,124). This order reproduces our s0..fp assignment exactly; target order requires a6 > a1 > ret > a2 > a3 > a4 >= a5.

- [s1] sched1 (reverse list scheduling, LUID tie-break sched.c:2460) places a6's lw between a4's and a5's in block 0 (load-delay slot before beqz takes a5), which is why L(a4) = L(a5) + 1 and a5 wins s7.

- [s1] body v2 is instruction-identical to the target apart from register names in s2/s3/s4/s7/fp and the prologue lw order; the /360 magic multiply, *t++ triple, and both if-arms match.
