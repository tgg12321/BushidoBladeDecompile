/* CANDIDATE - func_8004473C (s4, 2026-09-06). MATCHED: sandbox --disable all = 0 at 49/49
 * instructions, and a full `verify-oracle` rebuild of the whole tree returns
 * build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle. Pure C, no FAKE
 * construct, no inline asm, no register pins (HEAD carried a register-pinned m2c body for
 * this function; this replaces it).
 *
 * HOW THE FLOOR MOVED 13 -> 0 THIS SESSION (three independent ordinary-C levers, each
 * measured with the engine metric against tmp/perm_4473C_s4a/target.o):
 *   1. LOOP-BODY STATEMENT ORDER. s1-s3 recorded the loop body as "byte-exact" and stopped
 *      permuting it. It is not: moving `dst->unk6 = 0;` to the second-to-last position and
 *      swapping `dst->unkC = 0;` ahead of `dst->unkA = 4;` takes form C from 13 to 9.
 *   2. FOR-INCREMENT ORDER. Spelling the comma-increment `dst++, src++, i++` (dst first)
 *      instead of `i++, dst++, src++` takes 9 -> 7, and later fixes the last two-instruction
 *      swap (the `addiu a1,a1,104` / `addiu a2,a2,1` pair) in the loop body schedule.
 *   3. THE CHAINED ASSIGNMENT. `D_800A9CF8.unk10 = (s32)(src = (Rec4473C *)game_GetCharData());`
 *      is the block-0 fix. s3 attributed the whole residual to sched1 boosting the standalone
 *      copy insn `src = <call temp>` (birthing_insn_p, sched.c:2505) to LAUNCH_PRIORITY, so it
 *      won the T-2 ready-list slot and pushed the count load next to `blez` (forcing a
 *      load-delay nop, 50 instructions). Writing the store and the pointer as ONE assignment
 *      expression means expand emits the store from the call-result pseudo directly and there
 *      is no separate user-copy insn in block 0 to be boosted: block 0 then emits
 *      `move a2,zero / lui a1; lw a1,12 / lui v1; lh v1,6 / lui a0; addiu a0,a0,16 / blez v1 /
 *      sw v0,0(a0) / addiu a3,a0,-16` - the target verbatim, store in the delay slot, no nop.
 *
 * CHASSIS: still requires the D_800A9CF8 aggregate merge (per-word splat symbols ->
 * Unk800A9CF8Header) established in s1 from base+offset evidence. The declaration is TU-LOCAL
 * here ONLY because a grind session's edit surface is src/text1a_c.c + memory/ + tmp/;
 * memory/grind/func_8004473C/candidate_merge.patch is the header-canonical variant
 * (include/game.h) that s3 measured byte-identical. See self_vet.md for prongs (c)/(d).
 */

typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;   /* stage id (stage_GetId) */
    s16 unk6;   /* entry count */
    s32 unk8;
    s32 unkC;   /* entry table (stride 0x68) */
    s32 unk10;  /* game_GetCharData() table (stride 0x68) */
    s32 unk14;
} Unk800A9CF8Header;

extern Unk800A9CF8Header D_800A9CF8;

extern void *game_GetCharData(void);

/* Per-entry record (stride 0x68) shared by the D_800A9CF8.unkC table and the
 * game_GetCharData() table; the sibling func_80044B30 walks both with the
 * same stride and field offsets. */
typedef struct {
    s8 unk0;
    s8 unk1;
    s16 unk2;
    u16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    s8 pad18[0x14];
    s32 unk2C;
    s32 unk30;
    s32 unk34;
    s8 pad38[0x14];
    s32 unk4C;
    s32 unk50;
    s32 unk54;
    s32 unk58;
    s32 unk5C;
    s8 pad60[8];
} Rec4473C;

void func_8004473C(void)
{
    Rec4473C *src;
    Rec4473C *dst;
    s32 i;

    D_800A9CF8.unk10 = (s32)(src = (Rec4473C *)game_GetCharData());
    dst = (Rec4473C *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; dst++, src++, i++) {
        dst->unk0 = 0;
        dst->unk1 = 0;
        dst->unk2 = 0;
        dst->unk4 = D_800A9CF8.unk0;
        dst->unk8 = 0;
        dst->unkC = 0;
        dst->unkA = 4;
        dst->unk10 = 0;
        dst->unk12 = 0;
        dst->unk14 = 0;
        dst->unk4C = src->unk2C;
        dst->unk50 = src->unk30;
        dst->unk54 = src->unk34;
        dst->unk6 = 0;
        dst->unk58 = -1;
    }
}
