/* func_800324D0 - BEST FORM [s22 2026-09-07, rederive]. sandbox --disable all
 * prints 5 on today's HEAD reference; the HONEST FLOOR IS 3. THE FLOOR DROPPED
 * 15 -> 3 THIS SESSION, and it dropped because the CHASSIS CHANGED, not because
 * a new spelling was found: the 2026-09-07 -msoft-float adoption (a42d7ff7)
 * retired the FAKE-annotated duplicated-tail body that had matched at 0, and the
 * whole s15-s21 RA analysis was measured on the hard-float chassis.
 *
 * (0) THE +2 REFERENCE ARTIFACT IS STILL LIVE AND IS RE-CONFIRMED. HEAD now
 *     ships INCLUDE_RODATA("asm/rodata", jtbl_800105A0) + INCLUDE_ASM, so the
 *     reference object build/src/code6cac_b.o reaches the jump table through
 *     relocations against the NAMED GLOBAL jtbl_800105A0, which engine/score.py
 *     does not mask, while any C build reaches it through a .rodata
 *     SECTION-relative reloc that score.py does mask. That is exactly the s15
 *     artifact, and it is confirmed independently this session: the s21 ledger
 *     body, banked at 15, re-measures at 17 on this reference
 *     (s22/... ), i.e. every number printed this session is honest+2.
 *     STANDING PROCEDURE: `& tools/wteng.ps1 main build` from a pristine
 *     `git checkout -- src/code6cac_b.c` FIRST (SHA1 == oracle,
 *     s22/build_head_reference.log), then apply a body, then sandbox, then
 *     subtract 2.
 *
 * (1) THE RESIDUAL IS NO LONGER AN RA SEAT. IT IS ONE LOOP-INVARIANT HOIST.
 *     The duplicated-tail construct still does its RA job perfectly: the walker
 *     takes $v1 and every one of the 15 register diffs the s21 body carried is
 *     gone. The entire remaining 3 is that the target hoists `li t0,255` (the
 *     0xFF head-test constant) into the preheader and our build materialises it
 *     inside the loop as `li v0,255` with the matching `bne a2,v0`
 *     (s22/target.dis vs s22/dup.dis).
 *
 * (2) THE GATE IS loop.c:1631 AND ITS ARITHMETIC IS FULLY MEASURED, NOT GUESSED.
 *         hoist  <=>  threshold * savings * m->lifetime  >=  insn_count
 *     For pseudo 85 (the 255) the .loop dump prints `(life 1), savings 1`, and
 *     under -msoft-float threshold = 2 * (1 + n_non_fixed_regs) = 58 (it was
 *     122 under hard float, because the 32 FP registers were still allocatable).
 *     So the budget is a hard `loop insn_count <= 58`. Measured boundary,
 *     K = number of arms carrying the duplicated tail (s22/ksweep.log +
 *     the .loop dumps):
 *         K=0 47 insns  hoisted   score 29      K=6  59 insns  NOT hoisted  26
 *         K=5 57 insns  hoisted   score 23      K=7  61 insns  NOT hoisted   5
 *         K=12 72 insns NOT hoisted score 5
 *     Base loop insn_count is 47 and each duplicated arm costs exactly +2.
 *     The RA flip needs K >= 7 (K=6 measures 26, K=7 measures 5). So the
 *     function is short by EXACTLY 3 loop instructions / 2 duplicated arms:
 *     the hoist wants <= 58 and the register allocator wants 61.
 *
 * (3) WHAT WAS MEASURED AGAINST THAT 3-INSTRUCTION GAP AND FAILED.
 *     - `u32 c` (sheds the zero-extends): 67 insns, one short of the target's
 *       68, score 9 at every K >= 7 (s22 gen3 sweep).
 *     - staged-cmd tail (the s21 chassis) + K: 17/18 flat for K<=6 and 30 at
 *       K=7; the staged body's own loop is 49 insns, so it is 2 WORSE on the
 *       budget than the plain body. Dead.
 *     - val read hoisted to the loop head: moves the RA flip one arm cheaper
 *       (K=6 instead of K=7, s22 gen4 sweep) but the hoisted lbu costs one byte
 *       of its own, and 47+12 = 59 still misses the 58 budget by ONE. Score 6.
 *     - val read hoisted into the else-of-0xFF arm: flip still at K=7, +2 on
 *       the score. Dead (7).
 *     - merging the two `ptr++` sites (short-command and payload) into one:
 *       compiles to 66 insns, i.e. it merges two instructions the TARGET keeps,
 *       so the target provably has two separate ptr++ sites. Dead.
 *     The lifetime channel is structurally shut for this constant: m->lifetime
 *     is luid(last ref) - luid(set), and a single-use constant compare emits
 *     the `(set (reg 85) (const_int 255))` immediately before its compare, so
 *     lifetime is 1 unless the SAME constant is used at a second point in the
 *     loop. `savings` is n_times_set (loop.c:793 + 597), so it is 1 unless the
 *     constant is materialised twice, which costs bytes.
 *
 * (4) THE HONEST READING OF (2): the ORIGINAL source cannot have had the
 *     duplicated tail. A 47-insn loop hoists the 0xFF under BOTH thresholds,
 *     and the target hoists it - so the original body is a K=0-shaped loop
 *     whose walker wins $v1 for a reason that is not reg_n_refs inflation. The
 *     duplicated-tail construct was always the wrong mechanism; the hard-float
 *     chassis merely hid that. The next attack is the K=0 RA seat with an
 *     11-instruction slack budget, NOT another duplication count.
 *
 * BODY BELOW: plain chassis, tail duplicated into the FIRST SEVEN arms - the
 * minimum K that flips the allocator, and therefore the form that sits closest
 * to the loop.c budget (61 vs 58). Measured this session: score 5 (honest 3),
 * build_insns 68 == target_insns 68, rules_dropped 0
 * (s22/sandbox_K7_final.log).
 *
 * FAKE: the loop tail (`c = *ptr; ptr++;`) is duplicated into the first seven
 * command arms instead of being reached by falling out of the switch,
 * mechanism: flow.c's reg_n_refs census counts the duplicated walker
 * references before global.c's allocno_compare ranks the allocnos, so the
 * walker pseudo outranks the operand carrier and takes $v1 instead of $a2,
 * while jump2's cross-jump pass runs after reload and re-merges the identical
 * tails so not one duplicated reference materialises (68 == 68),
 * lever-exhaustion: memory/grind/func_800324D0/hypotheses.md s1-s22.
 * NOTE FOR THE NEXT SESSION: this FAKE is NOT submittable as it stands - it no
 * longer produces a match (3 short), and (4) argues it is the wrong mechanism
 * outright. It is banked as the lowest measured floor, not as a candidate.
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
            switch (c - 0x80) {
                case 0: pad[0xA1] = val; c = *ptr; ptr++; continue;
                case 1: pad[0xA3] = val; c = *ptr; ptr++; continue;
                case 2: pad[0xA7] = val; c = *ptr; ptr++; continue;
                case 3: pad[0xA8] = val; c = *ptr; ptr++; continue;
                case 4: pad[0xA9] = val; c = *ptr; ptr++; continue;
                case 5: pad[0xA5] = val; c = *ptr; ptr++; continue;
                case 6: pad[0xA6] = val; c = *ptr; ptr++; continue;
                case 7: pad[0xA2] = val; break;
                case 8: pad[0xA4] = val; break;
                case 9: pad[0xAA] = val; break;
                case 10: pad[0xAB] = val; break;
                case 11: pad[0xAC] = val; break;
            }
        }
        c = *ptr;
        ptr++;
    }
}
