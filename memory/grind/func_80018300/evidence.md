# func_80018300 — evidence (manual lane, 2026-09-24)

Result: `candidate.c` scores `sandbox --disable all` = 0 (307/307). Honest bucket is
COMPLETED-INLINE-ASM-CANONICAL (GTE islands; census member row :61 of the 2026-08-17
cop2 addressing-preamble cluster ruling).

## Score trajectory (all sandbox --disable all, cheat-stripped)

| step | change | score | insns |
|---|---|---|---|
| a | first full draft (bisection loop, isqrt, `move $12` islands clobbering "$12") | 95 | 300/307 |
| b | hoisted `nthresh = -thresh` variable | 81 | 299/307 |
| c/d | island clobbers "$12".."$15"; constant asm operands as literals (no `scr` var) | 43 | 307/307 |
| e3 | LZC arm: `len` carries the shift, `sum` takes the table byte | 22 | 307/307 |
| f3 | `len = lz;` copy before `0x16 - (len & ~1)` | 16 | 307/307 |
| h1 | link index word staged through `thresh` | 8 | 307/307 (only frame offsets left) |
| j5/j6 | `s32 lz[5]` / `lz[6]` (oversized live LZC object) | 0 | 307/307 |
| final | inline_o.h spelling, "memory" on every island, annotations | 0 | 307/307 |

## Key mechanisms (measured with the instrumented cc1, tools/gcc-2.7.2/cc1)

- **$t4-$t7 footprint = inline_o.h clobbers.** PsyQ `inline_o.h` (the "DMPSX version 3" macro
  header, unexpanded $PSLibId$ so no release pinned; Xeeynamo/croc@f30ff1ee `include/psyq/inline_o.h`, sha256
  27a4abd6c311a4b65c394d5ecc5624a4f8c0182f990b997b0aee20aab581a9d6) spells its load/store GTE
  macros as `move $12,%0` + the cop2 transfer, its command macros (sqr0, gpf12) as nop, nop +
  the command word, and gte_nop as one bare `nop`; every statement clobbers
  "$12","$13","$14","$15","memory".
  gte_ldlvl :308, gte_lddp :443, gte_ldlzc :645, gte_sqr0 :1749, gte_gpf12 :1875,
  gte_stlvl :2403, gte_stlvnl :2422, gte_stlzc :2999, gte_nop :3068. `gtemac.h`
  gte_Square0 = ldlvl + sqr0 + stlvnl, the target's exact sequence. The target never uses
  $t5-$t7; with "$12","memory" only, count/out/data/radius land in $t5-$t8 and reload spills the
  constant asm operands elsewhere. With the full list, global.c gives data/radius $t8/$t9
  (pass 0, call-clobbered) and count/out $s0/$s1, and reload spills the constant operands
  to $s2 (greg dump: "Spilling reg 18") exactly as the target. Ablation on final body:
  "$12","memory" only = 44 at 301 insns.
- **pair staged through thresh.** Own pseudo: conflicts {2,3,4,7}+asm clobbers, find_reg
  takes $a1 (FINDREGDBG pseudo 77). Target seats it in $t1 = thresh's reg; merging into
  thresh's pseudo reproduces it. Ablation: 8.
- **len/sum reuse in the LZC arm.** Ablations: fresh `shift` 36; fresh byte expr 38; no
  `len = lz[0]` copy 6.
- **Frame.** Target frame 0x28 = saves 16 + locals 24, no calls. Only $sp traffic in the
  locals region is `swc2 $31,0($t4)` ($t4=$sp) and `lw $v1,0($sp)`. lz scalar / lz[1] /
  lz[4] / lz[7] all score 8; lz[5] and lz[6] score 0.
- **Real cop2 words.** 0x4AA00428 = SQR sf=0 lm=1 (target "sqr 0"); 0x4B98003D = GPF sf=1
  (target "gpf 1"). inline_o.h carries DMPSX placeholders 0x00000f3f / 0x000012bf; the
  in-tree `include/gte_macros.inc` table maps sqr0 -> 0x4AA00428 and gpf12 -> 0x4B98003D.

## Sandbox note
The 4 "not-scored" hunks (post-loop `j`, div-check branches off by 8 bytes) come from the
cheat-stripped sandbox object dropping the two single-`nop` gte_nop islands; the full
build (verify-oracle) is the real check.
