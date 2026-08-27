/* alt_D5_alltargetseats_score2_s27.c -- s27 best-structured form.
 *
 * sandbox func_80041188 --disable all = score 2 at 133 build / 132 target insns.
 * (Not a floor improvement: candidate.c still holds the floor at 1. Banked because
 *  this is the first form in the grind's history whose ENTIRE residual is one
 *  surplus instruction: every callee-saved seat is target's, target's block-2
 *  `addiu $s3,$s7,0x20` is present, and every other instruction matches.)
 *
 * Chassis: V15a (goto loop1, the two owner-ALLOWED split increments) plus ONE new
 * construct -- a same-value re-store of `out2` inside loop1 (`out2 = pa4 + 0x20;`
 * placed after the func_800523E0 call). Mechanism: the re-store makes out2
 * loop-carried, which lifts it from 3 refs / live 42 = 714 to 5 refs / live 42 =
 * 2380 (flow.c REG_N_REFS; priority formula E-s26-3). 2380 lands inside the
 * admissible band (pa4 1458, stptr2 2500), so global.c hands out2 $s6, pa4 $s7 and
 * a3 $fp -- target's disposition. Cost: the re-store is a real addiu that reorg.c
 * puts in the loop-back branch delay slot, displacing `addiu $s3,$s3,0x68`.
 *
 * SHIPPING NOTE: this body is NOT vetted. It carries three constructs that would
 * each need a /* FAKE * / annotation and a family citation before any submission:
 * the two split increments (owner ruling 2026-08-27, split/redundant-arithmetic
 * class) and this same-value re-store of a LOCAL (dead-store family,
 * .claude/rules/dead-store-fake-exception.md). It is banked as a MEASUREMENT, not
 * as a candidate.
 */
