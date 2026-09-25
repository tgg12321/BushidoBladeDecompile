/*
 * REJECTED / KILLED (session 5b) — frontier F5, "move the guard-load and jalr
 * temp from $v0 to the target's $t0 (worth 4 points)". This file records a
 * STRUCTURAL KILL, not a candidate form: no pure-C body that also emits the
 * target's instruction sequence can put those temps in $t0.
 *
 * WHAT F5 WAS. Across four sessions, every measured form puts the D_800A2668
 * guard load in $v0 (lui $v0 / lw $v0) and the indirect call target in $v0
 * (lw $v0,0($s0) ; jalr $v0), while the target uses $t0 for all four
 * instructions (lui $t0 / lw $t0 ; lw $t0,0($s0) ; jalr $t0). At the floor-13
 * chassis that is 4 of the 13 remaining points and was the largest live bucket.
 *
 * THE DUMP (tmp/grind/motion_Close/s5b/f13.c.lreg + f13.c.greg, cc1 -da on the
 * floor-13 chassis). The two temps are BLOCK-LOCAL pseudos, so local-alloc.c
 * owns them, not global.c:
 *   Register 74 used 2 times across 4 insns in block 0; GR_REGS   (guard load)
 *   Register 75 used 4 times across 4 insns in block 2; GR_REGS   (call target)
 *   ;; Register dispositions: 72 in 17  73 in 16  74 in 2  75 in 2
 * Both land in hard reg 2 = $v0. (72/73 are count/p in $s1/$s0 as intended.)
 *
 * THE MECHANISM, AND WHY IT CANNOT BE STEERED. Both allocators pick the FIRST
 * FREE hard register in ASCENDING NUMERIC ORDER:
 *   tools/gcc-2.7.2/local-alloc.c:2249-2262  — `for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)`
 *       with `#ifdef REG_ALLOC_ORDER ... #else int regno = i; #endif`
 *   tools/gcc-2.7.2/global.c:1057-1062 and 1203-1209 — the same construct
 * and `REG_ALLOC_ORDER` is NOT defined anywhere in the MIPS backend
 * (`grep -n REG_ALLOC_ORDER tools/gcc-2.7.2/config/mips/*.h` returns nothing;
 * regclass.c:112 only defines reg_alloc_order under that #ifdef). So the scan
 * order for GR_REGS is $zero(0, fixed), $at(1, fixed), $v0(2), $v1(3),
 * $a0..$a3(4-7), $t0(8), ... — $v0 is the first allocatable register and it is
 * taken unless it is already excluded over the pseudo's live range.
 *
 * To reach $t0 = hard reg 8, ALL SIX of regs 2,3,4,5,6,7 must be excluded over
 * the guard-load range AND over the call-target range. Exclusion comes from
 * other values being live in those hard regs there. But motion_Close's target
 * byte stream contains NO value in $v0, $v1 or $a0-$a3 anywhere in the
 * function: the call takes no arguments (no $a0-$a3 setup insns exist in the
 * target), returns void (no $v0 consumer exists), and the only other live
 * values are p and count in $s0/$s1. A C body that manufactured six live
 * call-clobbered values to push the temps up to $t0 would have to emit the
 * instructions that produce them — instructions the target does not contain —
 * so it would ADD distance at least as fast as it removes it. The two
 * requirements are mutually exclusive by construction.
 *
 * CONSEQUENCE FOR THE FUNCTION. Combined with H1 (gcc-2.7.2's o32 backend
 * reserves REG_PARM_STACK_SPACE = 16 bytes of outgoing-arg area for every
 * C-level call, so any pure-C body containing a call has frame >= 28 -> 32
 * against the target's 16 — census of all 854 call-making functions in the
 * oracle build: none below 16), the residual at floor 13 decomposes as
 *   ~7  frame size + save offsets + restore offsets   — structurally dead (H1)
 *    4  $v0 vs $t0 on the guard load, body load, jalr  — structurally dead (F5, this file)
 *    2  beqz delay slot + prologue save order          — open
 * i.e. 11 of the 13 remaining points are now proven unreachable by any pure-C
 * form under the frozen toolchain, by two independent mechanisms. This is
 * evidence FOR an eventual escalation of motion_Close; it is NOT a disposition,
 * which is the driver's call, and the ladder still has untried modalities.
 * A future session must not spend further effort trying to steer these temps.
 */

/* No candidate body — this file is a kill record. The measured chassis it
   applies to is memory/grind/motion_Close/candidate.c (floor 13). */
