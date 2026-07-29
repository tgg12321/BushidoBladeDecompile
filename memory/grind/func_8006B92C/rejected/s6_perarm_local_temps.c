/* s6 P6b — REJECTED (score 6, build_insns 141: byte-identical to the h2a base).
 *
 * Change vs h2a: a SEPARATE temp per else arm (t1 in case 1, t2 in case 2), so
 * each is basic-block-local and therefore allocated by local-alloc.c rather
 * than global.c. Intent: have the shift value born BEFORE the mask constant
 * and live ACROSS it, so find_free_reg's `regs_live_at` scan marks $v0 busy
 * over the constant's range and pushes the constant to $v1 (target's register).
 *
 * Measured dead: the .lreg dump (tmp/grind/func_8006B92C/s6/dumps_p6b/
 * func_8006B92C.lreg) shows sched1 has already hoisted the whole mask chain
 * (insn 316 lui / 317 ori / 76 and) ABOVE the shift chain (insn 71 srl / 72
 * andi / 79 addiu) inside the else block — note the insn numbers are out of
 * order, i.e. the shift chain was emitted first by expand and MOVED DOWN.
 * t1 is pseudo 79, live [72..79], which does not overlap the constant pseudo
 * 95, live [316..76]. No conflict -> the constant still gets $v0 -> the
 * then-arm `lui $v0` is still deleted by reorg's redundant_insn.
 *
 * Conclusion: SOURCE STATEMENT ORDER inside the else arm is not a lever — the
 * list scheduler normalises it to mask-first regardless. (Consistent with s5
 * chassis B, whose PERM_LINESWAP over these two statements tied at base.)
 */
        } else {
            t1 = (a0 >> 13) & 7;            /* s32 t1, t2; at fn scope, one per arm */
            var_v1 = a0 & 0xFFFF1FFF;
            var_v0 = t1 + 1;                /* case 2: t2 - 1 */
            goto complete_store;
        }
