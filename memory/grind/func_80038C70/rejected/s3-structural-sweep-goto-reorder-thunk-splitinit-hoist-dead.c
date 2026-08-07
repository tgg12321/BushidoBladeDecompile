/* REJECTED — s3 structural sweep of the 13-pair cross-jump wall (2026-07-17).
 *
 * Five spellings NOT previously in the bank, all measured via label-normalized
 * cc1 asm diff vs m0 (committed baseline, 363 insns; harness
 * tmp/grind/motion_SetMotion/s3/s3_probe.py). Target requires TWO physical
 * [j sel_dispatch; delay li s0,0xD] blocks (==3 arm + case-9/11 jtbl block).
 *
 * r5-r8: ALL canonicalize to the IDENTICAL merged 361-insn shape (same 86
 * diff lines as honest-0xD): GCC folds/tensions each spelling back to the
 * plain set13 form before jump2 and find_cross_jump merges the pair.
 * r9: breaks the merge but diverges massively (619 diff lines, 362 insns).
 */

/* r5 — cross-case goto share: case 9/11 jumps INTO the ==3 arm's set13.
 * GCC does not duplicate: single block, case-9/11 jtbl entries retargeted
 * at the arm; identical merged layout. KILLED. */
    /* if (v0 != 10) { arm13: sel = 0xD; } else { sel = 0xF; } ...
       case 9: case 11: goto arm13; */

/* r6 — source-order reorder: honest case-9/11 body moved after case 12.
 * Physical placement is irrelevant: identical merged shape. KILLED. */

/* r7 — goto-thunk indirection: case 9/11 -> disp2: goto sel_dispatch;.
 * jump1 tensions the jump-to-jump, thunk deleted, merge fires. The
 * jump-chain sibling of m7's adjacent-label negative. KILLED. */

/* r8 — split-init accumulation (the SANCTIONED family, 2026-06-13):
 *   case 9: case 11: sel = 0xC; sel += 1;
 * cse constant-folds 12+1 -> plain set13 before jump2; identical merged
 * shape. The sanctioned family is measured DEAD on this wall. KILLED. */

/* r9 — hoisted-common-set: sel = 0xD; before the switch, case 9/11 = bare
 * goto sel_dispatch;. The ONLY spelling class that yields last1==0
 * (case-9/11 jtbl entries point straight at sel_dispatch — no block to
 * merge) — confirming find_cross_jump needs >=1 counted match — but the
 * hoisted li lands in the D_800A31FC beq's delay slot, the block vanishes
 * (target needs it at fc0), and the switch layout reshapes: 619 diff
 * lines. Byte-count kills it from the zero-insn side, the mirror of s2's
 * F1 slack-transplant kill. KILLED. */
