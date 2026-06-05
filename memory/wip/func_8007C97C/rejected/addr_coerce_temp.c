/* REJECTED — round-7 permuter find (2026-06-04, ~6749 iters)
 *
 * Permuter weighted score: 1010 (vs ~3100 baseline). Strong on the
 * permuter's metric, but the construct is a CHEAT-BY-SPELLING per
 * .claude/rules/no-new-park-categories.md ("cheats by any spelling")
 * and .claude/rules/dead-vars-local-array.md (scalar-address-coercion
 * form, detected by engine/volatile_cheats.find_addr_coerced_locals).
 *
 * The construct: `s32 *new_var = &temp_v1;` declared, address taken,
 * indirect access used purely to force GCC to materialize temp_v1
 * on the stack frame (producing the target's dead `sw` stores).
 * Zero semantic purpose — `(*new_var) << 5` is identical to
 * `temp_v1 << 5`. The address-of-local pattern exists only to bend
 * cc1's RA / dead-write analysis.
 *
 * Reviewer verdict: FAIL (same family as the round-0 sp[4] cheat,
 * just spelled with an indirect pointer).
 *
 * Preserved here so the next agent does NOT re-derive it. The full
 * permuter find variants 1010-1, 1095-1 (with `u32 *new_var2 = &temp_v0;`),
 * 1205-1 etc. ALL use the same address-coercion family. The permuter's
 * random PERM_GENERAL mutation will keep finding this; ignore them all.
 */

typedef unsigned int u32;
typedef signed int s32;
typedef signed short s16;
typedef unsigned char u8;

s32 func_8007C97C(u8 *arg0) {
    s32 *new_var;
    u32 temp_a1;
    s32 temp_a2;
    u32 temp_v0;
    s32 temp_v1;
    if (arg0 != 0) {
        temp_a2 = (u8) arg0[0];
        temp_a1 = temp_a2 >> 3;
        temp_a2 = ((s32) ((-(*((s16 *) (arg0 + 4)))) & 0xFF)) >> 3;
        temp_v0 = ((u8) arg0[2]) >> 3;
        temp_v0 = temp_v0 << 0xF;
        new_var = &temp_v1;                                         /* CHEAT */
        temp_v1 = ((s32) ((-(*((s16 *) (arg0 + 6)))) & 0xFF)) >> 3;
        {
            u32 r_e2 = (temp_a1 << 0xA) | 0xE2000000;
            return ((temp_v0 | r_e2) | ((*new_var) << 5)) | temp_a2; /* CHEAT */
        }
    }
    return 0;
}
