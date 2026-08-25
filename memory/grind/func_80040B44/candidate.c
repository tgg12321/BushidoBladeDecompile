/* candidate.c - func_80040B44 - s2 (2026-08-24, structural) - HONEST FLOOR 0.
 * `sandbox func_80040B44 --disable all` = score 0, build_insns 93 == target_insns 93,
 * measured with this exact body in src/text1a_pre.c. Zero regfix/asmfix reliance,
 * zero inline asm, zero FAKE constructs, zero coercion constructs of any family.
 * The 15 regfix rules the function still carries are now pure debt: retiring them is
 * the operator step (`engine retire func_80040B44`), not a source change.
 *
 * The four structural facts that closed it (all measured this session, s2):
 *  1. Addend order of a two-term add is preserved from source ONLY in the INTEGER
 *     domain. c-typeck.c's pointer_int_sum() rewrites every pointer+int PLUS_EXPR to
 *     pointer-first, so `a3*0x68 + (u8*)t7` is identical to `(u8*)t7 + a3*0x68`.
 *     Casting the base to s32 and adding in integers reproduces target's index-first
 *     `addu` operand order at the five sites that need it, while the one site target
 *     spells base-first stays pointer arithmetic.
 *  2. The two init loops share ONE counter (`i`, target holds it in $a0 across both)
 *     and use FRESH pointer locals (p1 -> $a1, p2 -> $v0).
 *  3. The walking pointer's advance belongs at the TOP of the loop body, not in a
 *     preheader statement. reorg.c then steals that addiu into the `bne` delay slot and
 *     replicates it in the preheader, and emits the exit-path compensation
 *     `addiu t3,t3,-2` itself. That single insn was the 92-vs-93 shortfall and the
 *     forbidden lost-codegen regfix @175 existed only to fake it - it is COMPILER
 *     OUTPUT, never a source statement. As a bonus the shape drops the pointer to 14
 *     reg_n_refs, below global.c's floor_log2 step at 16, which flips the $t2/$t3 seats
 *     onto target's assignment.
 *  4. There is no `t4` variable. t4 == t5 + 0x58 invariantly, so the else arm is
 *     written against t5 and loop.c's strength reduction creates t4 as a giv - and a
 *     giv's initializer is emitted AFTER move_movables' hoisted `li 0xffff`, which is
 *     the preheader order target has and no source ordering could otherwise produce.
 *
 * Also gone: the `s32 one = 1;` holder s1 flagged as a forbidden-catalog spelling.
 * Measured byte-NEUTRAL to delete once (4) landed - `*a1 = 1;` is the honest form.
 */
void func_80040B44(s32 *arg0) {
    s32 seen[18];
    s32 *t5;
    s32 *t7;
    s32 *v1;
    s32 *t3;
    u16 a0_val;
    s32 i;

    t5 = (s32 *)((u8 *)arg0 + 0x10D4);
    t7 = (s32 *)((u8 *)arg0 + 0x94);
    v1 = *(s32 **)((u8 *)arg0 + 0x1C);
    t3 = (s32 *)((u8 *)v1 + *(s32 *)((u8 *)v1 + 8));

    {
        s32 *p1;
        i = 0x11;
        p1 = &seen[17];
        do {
            *p1 = 0;
            i--;
            p1--;
        } while (i >= 0);
    }
    {
        s32 *p2;
        i = 0x13;
        p2 = (s32 *)((u8 *)arg0 + 0x4C);
        do {
            *(s32 *)((u8 *)p2 + 0x1A34) = 0;
            i--;
            p2 = (s32 *)((u8 *)p2 - 4);
        } while (i >= 0);
    }

    a0_val = *(u16 *)t3;
    if ((a0_val & 0xFFFF) == 0xFFFF) goto done;

    {
        do {
            s32 a3;
            s32 *a1;
            s32 t2;

            t3 = (s32 *)((u8 *)t3 + 2);
            a3 = *(u16 *)t3;
            t3 = (s32 *)((u8 *)t3 + 2);
            a1 = (s32 *)(a3 * 4 + (s32)&seen[0]);
            t2 = a0_val & 0xFFFF;

            if (*a1 == 0) {
                s32 *slot = (s32 *)(a3 * 0x68 + (s32)t7);
                *(s16 *)((u8 *)slot + 2) = (s16)t2;
                *a1 = 1;
                *(s32 *)((t2 << 2) + (s32)arg0 + 0x1A34) = (s32)slot;
            } else {
                s32 *t0 = t5;
                s32 *a2 = (s32 *)(a3 * 0x68 + (s32)t7);
                s32 *end = (s32 *)((u8 *)a2 + 0x60);

                do {
                    *(Copy16 *)t0 = *(Copy16 *)a2;
                    a2 = (s32 *)((u8 *)a2 + 0x10);
                    t0 = (s32 *)((u8 *)t0 + 0x10);
                } while (a2 != end);

                *(Copy8 *)t0 = *(Copy8 *)a2;

                *(s16 *)((u8 *)t5 + 2) = (s16)t2;
                *(s32 *)((u8 *)t5 + 0x58) = (s32)((u8 *)t7 + a3 * 0x68);
                *(s32 *)((t2 << 2) + (s32)arg0 + 0x1A34) = (s32)t5;
                t5 = (s32 *)((u8 *)t5 + 0x68);
            }

            a0_val = *(u16 *)t3;
        } while (a0_val != 0xFFFF);
    }
done:
    *(s32 *)((u8 *)t5 + 0x58) = 0;
}
