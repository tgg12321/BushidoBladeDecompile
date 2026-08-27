/* BEST BAN-COMPLIANT FORM (grind s30b, 2026-08-27, forensics modality).
 * MEASURED THIS SESSION on the current chassis: `sandbox func_80057CC8
 * --disable all` -> score 30, target_insns 111, build_insns 112, rules_dropped 0.
 * This is formB from s29, re-measured from scratch; it is the lowest-scoring form
 * in the bank that materialises the vertex-table base exactly ONCE at source level
 * and therefore does not touch the family the owner refused on 2026-07-20 / that
 * layer-1 FAILed four times (2026-08-20 x3, 2026-08-27 x1).
 *
 * WHY candidate.c NO LONGER HOLDS THE score-0 FORM: the score-0 body that occupied
 * this file (static inline vert_base_57CC8 / vert_angle_57CC8 helpers) was ruled a
 * re-spelling of the banned repeated-materialisation family by the layer-1
 * cheat-reviewer on 2026-08-27 and is banked at
 * rejected/layer1-fail-0827-0417.c. Leaving it here as "the candidate" invites a
 * future session to resubmit a construct the driver rejects before the Judge ever
 * sees it. The honest best submittable form is this one.
 *
 * WHAT s30b PROVED ABOUT THIS FORM'S +1 INSTRUCTION (see evidence.md s30b block):
 * the extra instruction is NOT closable by allocation persuasion. The target loads
 * the base twice (asm/funcs/func_80057CC8.s:17 `lw $a2,0x4($s2)`, :50
 * `lw $a0,0x4($s2)`) and the first copy lives in a CALL-CLOBBERED register that is
 * dead before the `jal`, so the two loads are two distinct RTL values, not one
 * pseudo. GCC 2.7.2 has exactly one mechanism that can turn a single RTL load into
 * per-use loads from the original address -- the REG_EQUIV / reg_equiv_mem path in
 * local-alloc.c's update_equiv_regs -- and it is doubly gated (RTX_UNCHANGING_P on
 * the MEM, and every reference to the pseudo confined to one basic block). Both
 * gates were exercised directly this session; see the rejected/s30b-* forms.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s32 pi;
    s32 ni;
    u16 cy;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ni = (s16) next_idx;
    ang_next = ratan2(table[ni * 2] - (s16) cx, table[ni * 2 + 1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
