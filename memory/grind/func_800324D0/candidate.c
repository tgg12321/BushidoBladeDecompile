/* [s22 2026-09-01] REDERIVE — MATCHED. `sandbox func_800324D0 --disable all`
 * = 0 with build_insns 68 == target_insns 68 and rules_dropped 0, and the full
 * build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa. Pure C: zero register
 * pins, zero inline asm. The floor went 15 -> 0. (This body was first reached in
 * the discarded s22 attempt; THIS session re-applied it to a pristine src,
 * re-measured it end to end, corrected the annotation's reg-count numbers to the
 * measured dump values, and rebuilt the self-vet.)
 *
 * WHAT CLOSED IT. Twenty-one sessions established that the entire residual was a
 * single 2-register swap: the target puts the stream walker in $v1 and the
 * command web in $a2; every previous body put them the other way round. s18
 * proved, over all 40320 allocation orders with an exact find_reg model, that the
 * target disposition is reached iff the walker's allocno outranks the operand
 * carrier's in global.c's allocno_compare, i.e. iff
 *     floor_log2(nrefs)*nrefs*size/reg_live_length
 * is larger for the walker. s20 then filed a closed-form foreclosure on that
 * channel with two legs: (a) DEMOTING the carrier needs livelen(val) >= 68,
 * unreachable; (b) PROMOTING the walker needs ~50 references, and "on a budget
 * that is exact at 68 instructions that costs >= +6 instructions".
 *
 * LEG (b) WAS WRONG, and that is the whole finding: references are only expensive
 * if they MATERIALIZE. Duplicating the loop tail (`c = *ptr; ptr++;` plus its
 * back-transfer) into each of the twelve switch arms adds references at ZERO
 * instruction cost, because jump2's cross-jump pass runs AFTER register
 * allocation and re-merges the thirteen identical tails back into the single
 * shared tail at .L800325C8 — while flow.c has already counted every duplicated
 * reference into reg_n_refs by the time global.c ranks allocnos.
 *
 * MEASURED ON THE .lreg RTL (the numbers in the FAKE annotation):
 *   base body (tmp/grind/func_800324D0/s22/lreg_seg.txt)
 *     walker  pseudo 73: used 24 times   |  carrier pseudo 76: used 26 times
 *     -> the carrier outranks the walker, carrier takes $v1. Floor 15.
 *   this body (tmp/grind/func_800324D0/s22/lreg_seg_p3.txt)
 *     walker  pseudo 73: used 96 times   |  carrier pseudo 75: used 26 times
 *     -> the walker outranks the carrier, walker takes $v1. Floor 0, 8/8.
 * The carrier is untouched at 26 in both — this is purely the promote-the-walker
 * leg that s20 believed was priced out, and it costs nothing.
 *
 * ALSO DELETED BY THIS BODY: the four `register ... asm("v1"/"v0"/"a2"/"a1")`
 * pins the function was still carrying on main, the s5 staged-tail FAKE
 * (`cmd = *ptr; c = cmd;`), and the `u32 cmd` local it borrowed. None is needed.
 * The staged read is in fact ACTIVELY HARMFUL on this chassis: the same 12-arm
 * duplication WITH the staged tail measures 27, not 0
 * (tmp/grind/func_800324D0/s22/sandbox_p1.log), because staging hands the
 * reference lift to the command web instead of to the walker. That is a real
 * discriminating measurement, not a preference.
 *
 * Measurement chain (all under tmp/grind/func_800324D0/s22/):
 *   build_head_reference.log   pristine HEAD build, SHA1 == oracle
 *   sandbox_base.log           s21 body re-measured:              15, 68/68
 *   sandbox_p1.log             12-arm dup + staged tail:          27, 68/68
 *   sandbox_p2.log             12-arm dup, no staging:             0, 68/68
 *   sandbox_p3.log             same, `cmd` local dropped:          0, 68/68
 *   sandbox_verify.log         re-applied to pristine src:         0, 68/68
 *   build_sha1_verify.log      full build SHA1 == oracle
 *   sandbox_final_s22b.log     after annotation correction:        0, 68/68
 *   build_sha1_final_s22b.log  full build SHA1 == oracle
 */
void func_800324D0(u8 *pad) {
    u8 *ptr;
    u8 c;
    u8 val;

    ptr = *(u8 **)(pad + 0x58);
    pad[0xA1] = 0xFF;
    pad[0xA3] = 0xFF;
    pad[0xA2] = 0xFF;
    pad[0xA4] = 0xFF;
    pad[0xAA] = 0;
    pad[0xA7] = 0;
    pad[0xA8] = 0;
    pad[0xA5] = 0;
    pad[0xA6] = 0xFF;
    pad[0xAB] = 0xFF;
    pad[0xAC] = 0xFF;

    c = ptr[4];
    ptr += 5;
    while (c != 0) {
        if (c == 0xFF) {
            ptr += 6;
        } else if (c < 0x80) {
            ptr++;
        } else {
            val = *ptr;
            ptr++;
            /* FAKE: the loop tail (`c = *ptr; ptr++;` + its back-transfer) is
             * duplicated into all twelve command arms instead of being reached
             * by falling out of the switch, mechanism: flow.c's reg_n_refs
             * census counts the duplicated walker references before global.c's
             * allocno_compare ranks the allocnos: the walker pseudo's
             * reg_n_refs goes 24 -> 96 while the operand carrier stays at 26
             * (measured, .lreg dumps), so the walker now outranks it and takes
             * $v1 instead of $a2, while jump2's
             * cross-jump pass runs after reload and re-merges the thirteen
             * identical tails, so not one duplicated reference materializes
             * (68 == 68, byte-identical, full SHA1 == oracle),
             * lever-exhaustion: memory/grind/func_800324D0/hypotheses.md s1-s21
             * (the whole demote-the-carrier channel, the RA-seat channel, four
             * permuter campaigns) and evidence.md [s22] */
            switch (c - 0x80) {
                case 0: pad[0xA1] = val; c = *ptr; ptr++; continue;
                case 1: pad[0xA3] = val; c = *ptr; ptr++; continue;
                case 2: pad[0xA7] = val; c = *ptr; ptr++; continue;
                case 3: pad[0xA8] = val; c = *ptr; ptr++; continue;
                case 4: pad[0xA9] = val; c = *ptr; ptr++; continue;
                case 5: pad[0xA5] = val; c = *ptr; ptr++; continue;
                case 6: pad[0xA6] = val; c = *ptr; ptr++; continue;
                case 7: pad[0xA2] = val; c = *ptr; ptr++; continue;
                case 8: pad[0xA4] = val; c = *ptr; ptr++; continue;
                case 9: pad[0xAA] = val; c = *ptr; ptr++; continue;
                case 10: pad[0xAB] = val; c = *ptr; ptr++; continue;
                case 11: pad[0xAC] = val; c = *ptr; ptr++; continue;
            }
        }
        c = *ptr;
        ptr++;
    }
}
