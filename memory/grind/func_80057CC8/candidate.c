/* BEST BAN-COMPLIANT FORM (grind s31, 2026-08-27, forensics modality).
 * MEASURED THIS SESSION: `sandbox func_80057CC8 --disable all` -> score 26,
 * target_insns 111, build_insns 110, rules_dropped 0.  Previous candidate
 * (s29/s30b formB, cached base, 112 insns) measured 30 on the same chassis this
 * session; it is banked at rejected/s30b-formB-cached-base-9th-callee-save-score30.c.
 *
 * WHAT CHANGED: the next-neighbour vertex ADDRESS is formed before the first
 * ratan2 call instead of after it.  That is not a second materialisation of the
 * vertex-table base (`table` is still read exactly once, and it is DEAD before the
 * first call) -- it is the ordinary C of computing both neighbour addresses from
 * the one base.  Its effect is a register-pressure one, measured and modelled in
 * evidence.md s31: it removes `next_idx` from the set of values live across the
 * first call and puts `next_vert` there instead, so the live-across-call set is 8
 * -- exactly the target's 8 ($s0-$s7) -- instead of formB's 9, which had to
 * commandeer $fp and pay an extra sw/lw pair.
 *
 * WHY IT IS STILL NOT 0 (s31 instruction-multiset theorem, evidence.md):
 * asm/funcs/func_80057CC8.s contains TWO loads of 0x4($s2) (:17 `lw $a2`, :50
 * `lw $a0`).  A form that materialises the base once emits exactly one.  This
 * form pays the whole difference in that single instruction: 110 = 111 - 1, with
 * the same callee-save count as the target.  The remaining distance is order
 * (our address arithmetic sits before the call, target's after it), which is the
 * direct consequence of not having the reload.  s30b proved GCC 2.7.2 has no
 * mechanism (REG_EQUIV rematerialisation / caller-save) that turns one RTL load
 * into two loads at the original address, so no ban-compliant form can supply
 * that instruction.  The residual is the policy question already refused
 * 2026-07-20 and standing-ruled 2026-07-27, not an unexplored spelling.
 *
 * SPELLING-INVARIANCE (measured s31): `table + ni * 2`, `&table[ni * 2]`, and
 * `(s16 *)((s32)table + ((s32)(s16)next_idx << 2))` emit byte-identical text;
 * `table + ni + ni` differs only in the PLUS operand order of one addu and
 * scores the same 26.  The u16-plus-(s16)-cast spelling of the centre reads and
 * a plain `s16` spelling ALSO emit byte-identical text (v3) -- the u16/s16
 * question is not a lever in this regime.
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
    s16 *next_vert;

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
    ni = (s16) next_idx;
    next_vert = &table[ni * 2];
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ang_next = ratan2(next_vert[0] - (s16) cx, next_vert[1] - (s16) cy) & 0xFFF;

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
