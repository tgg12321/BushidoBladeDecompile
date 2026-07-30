/* REJECTED (grind s1, 2026-07-30) — do/while loop form: score 25, 38 insns.
 *
 * WHY IT IS DEAD: writing the 18-iteration loop as a real `do { } while (i <
 * 0x12);` makes GCC 2.7.2's loop.c recognise a loop (NOTE_INSN_LOOP_BEG), which
 * turns ON loop strength reduction. Strength reduction then invents a THIRD
 * induction pointer based at arg0+0x8B8 to serve the -0x57..-0x4C displacement
 * cluster, leaving arg0+0x90C used only for the single `sw zero, 0(a3)`. Result:
 * THREE `addiu rX,rX,0x68` increments per iteration where the target has TWO,
 * i.e. +2 instructions that no register choice can recover.
 *
 * Target's addressing shape (exactly two cursors: arg0+0x8B4 with displacements
 * {0, +2}, and arg0+0x90C with displacements {-0x57..-0x4C, 0}) is ONLY produced
 * by the `goto`-loop form, which loop.c does not recognise, so no strength
 * reduction runs and the displacements stay literally as the C wrote them.
 *
 * COROLLARY / do-not-retry: any loop spelling that GCC recognises as a loop
 * (`for`, `while`, `do/while`) re-bases the address cluster the same way. This
 * is not about the exit-test position; it is about the loop NOTE existing at
 * all. Do not re-propose "just write it as a normal for loop" — it costs +2
 * insns before register allocation is even reached.
 *
 * Measured: sandbox --disable all => score 25, build_insns 38, target_insns 36.
 * Disassembly banked at tmp/grind/func_80040CB8/s1/build2.txt.
 */
extern s16 D_80094B9E[];
void func_80040CB8(void *arg0) {
    s8 *slot = (s8 *)arg0 + 0x8B4;
    s32 i = 0;
    s32 link = (s32)arg0 + 0x94;
    s16 *tbl = D_80094B9E;
    s32 ent = (s32)arg0 + 0x90C;

    do {
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
    } while (i < 0x12);
    *(s16 *)((s32)slot + 2) = -1;
}
