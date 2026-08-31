/* func_80078654 — MATCHED FORM, grind session 11 (2026-08-31, modality: escalation/disposition).
 *
 * HONEST PURE-C SANDBOX FLOOR: 0.  target_insns 116 == build_insns 116,
 * rules_dropped 0, zero inline asm, zero pins.  Measured this session with this
 * body pasted over the INCLUDE_ASM line at src/text1b_b.c:1008:
 *   `sandbox func_80078654 --disable all` -> {"score": 0, ...}
 *
 * WHAT CLOSED IT (the ten-session residual, in one sentence): the 19-point
 * residual was a single two-way callee-save inversion (target: arg0 in $s1,
 * walk pointer var_s0 in $s0; ours the other way round), and it is closed by
 * giving var_s0's single loop-top reference a LOOP-NOTE WEIGHT of 9 with an
 * 8-deep `do { ... } while (0)` wrap.
 *
 * THE MECHANISM THE LEDGER WAS MISSING (flow.c:2081).  Every prior session
 * treated `reg_n_refs == emitted mentions` as an identity for this function
 * (s6/s7 measured it, s10 built the decision packet on it).  It is not an
 * identity — it is an artifact of this body having NO loop notes.  flow.c
 * counts each register mention as `loop_depth` references
 * (flow.c:2081/2329/2515/2725), where `loop_depth` is 1 + the number of
 * enclosing NOTE_INSN_LOOP_BEG notes (flow.c:434 initialises depth = 1,
 * flow.c:440-443 counts the notes).  Loop notes are NOT instructions, so a
 * wrap multiplies a pseudo's reg_n_refs at ZERO byte cost.  The s10 packet's
 * conclusion ("the original must have carried >= 8 walk references in insns
 * deleted between flow_analysis and global_alloc") named the wrong window:
 * no insn need be deleted at all — the references are counted more than once.
 *
 * THE ARITHMETIC.  Walk pseudo 73 has 5 mentions: 1 at the init (outside the
 * loop), 1 at `s.a = var_s0[0]`, 2 at `var_s0++`, 1 at the `var_s0[1]` test.
 * Parameter pseudo 72 has 13 mentions / 98 live, priority 3979.  The walk
 * pointer's live length is 91, so it needs nrefs >= 13 to outrank it
 * (13 -> pri 4285 > 3979; 12 -> 3956 < 3979).  Wrapping the loop-top read at
 * depth d gives nrefs = 4 + (1 + d), so d = 8 is the MINIMUM (d = 1 gives 6).
 * That is the prerequisite-3 "single level measured insufficient" justification
 * required by .claude/rules/do-while-zero-exception.md:70.
 *
 * WHY THE WRAP SITE IS EXACTLY THIS THREE-STATEMENT BLOCK.  A loop-note pair
 * is a hard scheduling boundary for sched2, so the wrapped region loses one
 * neighbour-supplied delay-slot fill and the build comes out at 117 insns
 * (a load-delay `#nop`).  Measured at every other site: wrapping `var_s0++`
 * = 117 (the increment can no longer fill the `lw v0,20(s1)` load-delay slot
 * at the loop tail); wrapping the loop-top read alone = 117 (the `lw v1,0(gp)`
 * from `s.h` can no longer fill the read's load-delay slot); wrapping the
 * init = 117; wrapping the test = 117.  The fix is to make the wrapped region
 * SELF-FILLING: extend it to `s.a = var_s0[0]; s.b = s.a + 0xC;
 * s.h = -D_800A3608;` so the `negu`/`lw` of `s.h` sits inside the region and
 * fills the read's load-delay slot from within.  That form is 116 == 116 with
 * ZERO differing instructions.
 *
 * INSTRUMENTS USED (both inherited): tmp/grind/func_80078654/s5/eval.sh
 * (instrumented-cc1 ALLOCDBG table + exact-pipeline objdump diff in one call,
 * run under WSL on a comment-free standalone chassis) and the s4 rig's
 * target.o.  Session-11 chassis variants are banked in
 * tmp/grind/func_80078654/s11/w*.c with their eval outputs in
 * tmp/grind/func_80078654/s5/s11_*/ .
 *
 * FAMILY / POLICY.  The construct is the SANCTIONED do-while(0) match device:
 * owner ruling 2026-07-06 (.claude/rules/do-while-zero-exception.md:29)
 * abolished the earlier reorg.c-only scoping and sanctions the wrap "for ANY
 * codegen effect, including register allocation", with an inline FAKE
 * annotation (prereq 1, :61) and, for nested wraps, a written
 * single-level-insufficient justification (prereq 3, :70).  Both are carried
 * inline at the construct site below.  Self-vet: memory/grind/func_80078654/self_vet.md.
 */

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 *var_s0;
    /* FAKE: constant-holder local, kept live across the SetDrawMode /
       func_8006E480 / AddPrim call sequence so the 0 argument comes out of a
       register instead of being re-materialized at each use.  Mechanism:
       local-alloc/global-alloc seat the constant in a call-saved quantity;
       replacing it with the literal 0 was measured this session at 113 insns
       vs the target's 116 (tmp/grind/func_80078654/s5/s11_w25/, chassis
       tmp/grind/func_80078654/s11/w25_nozero.c), so the holder is
       load-bearing.  Lever-exhaustion: memory/grind/func_80078654/hypotheses.md
       s1-s11. */
    s32 zero;

    zero = 0;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = D_800A3610[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 = D_800A3610 + 5;
    if (D_800A3608 >= 0xAAA) {
        if (D_800A3608 >= 0xB04) {
            s16 sv;
            s.cd_flag = 1;
            sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if (sv < 0) {
                sv = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) sv));
        }
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), arg0[5]);
        arg0[5] = arg0[5] + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    /* FAKE: 8-deep do-while(0) wrap around the walk-pointer read block.
       Effect: seats the table-walk pointer var_s0 in $s0 and the parameter
       arg0 in $s1 (loop-note reference weighting -> allocno priority).
       Mechanism: flow.c:2081 counts each register mention as loop_depth
       references, so the wrap multiplies var_s0's reg_n_refs without
       emitting an instruction; global.c's allocno priority then ranks
       var_s0 (13 refs / 91 live, pri 4285) above arg0 (13 / 98, pri 3979).
       Single level measured INSUFFICIENT (prerequisite 3 of
       .claude/rules/do-while-zero-exception.md): depth 1 yields 6 of the 13
       references the priority inversion requires; depth 8 is the minimum
       that reaches 13 at the only wrap site that costs no delay slot.
       Lever-exhaustion: memory/grind/func_80078654/hypotheses.md s1-s11
       (11 sessions, 8 modalities, 129k permuter iterations, 14 banked
       rejected forms). */
    do { do { do { do { do { do { do { do {
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), arg0[5]);
    var_s0++;
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
