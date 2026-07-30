/* REJECTED (grind s1, 2026-07-30) — literal 3 / 1 / -1 instead of the three
 * constant-holder locals, inside the `goto`-loop form: score 23, 35 insns.
 *
 * WHY IT IS DEAD: the `goto`-loop form deliberately has no NOTE_INSN_LOOP_BEG
 * (that is what keeps loop strength reduction off — see
 * do-while-loop-form-triggers-strength-reduction.c). No loop note ALSO means no
 * loop-invariant code motion, so a literal constant used inside the loop cannot
 * be hoisted into a register held across the loop. GCC's cse/combine instead
 * folds one of them away entirely, and the build comes out at 35 instructions
 * against target's 36 — one instruction SHORT, with score 23.
 *
 * Target genuinely holds all three constants in loop-invariant registers before
 * the loop (`addiu $t4,$zero,-1` / `addiu $t3,$zero,3` / `addiu $t2,$zero,1` at
 * the top, then `beq $v0,$t4` / `sb $t3,0($a2)` / `sh $t2,-0x52($a1)` inside).
 * In the goto form the ONLY way to get that is to name them as locals assigned
 * before the loop, which is what candidate.c does.
 *
 * This is the crux of the loop-form dilemma the next session inherits:
 *   - real loop  => LICM hoists the literals for free, but strength reduction
 *                   destroys the two-cursor addressing (+2 insns).
 *   - goto loop  => addressing is exact, but the constants must be named locals.
 * Neither branch is free. Resolving it (a real-loop spelling that resists
 * strength reduction) is frontier item F1.
 *
 * Measured: sandbox --disable all => score 23, build_insns 35, target_insns 36.
 */
extern s16 D_80094B9E[];
void func_80040CB8(void *arg0) {
    s32 i = 0;
    s32 link = (s32)arg0 + 0x94;
    s16 *tbl = D_80094B9E;
    s32 ent = (s32)arg0 + 0x90C;
    s8 *slot = (s8 *)arg0 + 0x8B4;

loop:
    {
        s16 id = *tbl;
        if (id != -1) {
            *(s16 *)(ent - 0x56) = id;
            *slot = 3;
            *(s8 *)(ent - 0x57) = 0;
            *(s16 *)(ent - 0x50) = 0;
            *(s32 *)(ent - 0x4C) = link;
            *(s16 *)(ent - 0x52) = 1;
            *(s16 *)(ent - 0x4E) = 0;
            {
                u16 w = *(u16 *)((s32)arg0 + 0x16);
                slot += 0x68;
                *(s32 *)ent = 0;
                *(s16 *)(ent - 0x54) = w;
                ent += 0x68;
            }
        }
        link += 0x68;
        i++;
        tbl = (s16 *)((s32)tbl + 0xA);
        if (i < 0x12) goto loop;
    }
    *(s16 *)((s32)slot + 2) = -1;
}
