/* func_800480C0 - CANDIDATE (s24 synthesis, 2026-09-05, chassis 3baaedfe).
 *
 * REPRESENTATION NOTE (fixes the s23 consistency warning): main/HEAD carries
 * `INCLUDE_ASM("asm/funcs", func_800480C0);` at src/text1b.c:129 per the
 * asm-until-matched ruling (2026-08-19). This file is a grind candidate only;
 * it is NOT the state of main and asserts nothing about it.
 *
 * STATUS: BYTE-EXACT, NOT YET SUBMITTABLE - blocked on one allowlist row.
 *   - build-c text1b  -> build/src/text1b.o sha1 441ad473138db80847e60b0f8e2a8c2b07014ee8
 *   - word compare vs asm/funcs/func_800480C0.s -> 74/74 instructions, ONE differing
 *     word (index 58, 0c000000 vs 0c0120b2 = the un-relocated jal func_800482C8 of an
 *     unlinked object). tmp/grind/func_800480C0/s24/pad_bytes.txt
 *   - FULL BUILD this session: build/bb2.exe sha1
 *     62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
 *     tmp/grind/func_800480C0/s24/pad_full_build.txt
 *   - sandbox --disable all nevertheless prints score 20, because
 *     engine/volatile_cheats.py:249 find_unused_local_arrays strips the unreferenced
 *     pre_pad and the _SANCTIONED_UNWRITTEN_PADS allowlist (engine/volatile_cheats.py:746)
 *     has no row for func_800480C0. The 20 is an allowlist artefact, not a codegen gap.
 *
 * WHY THIS FORM AND NOT ANOTHER: func_80047FBC, the function IMMEDIATELY ABOVE this
 * one in src/text1b.c (line 83, COMPLETED-C on main), is line-for-line this same body -
 * same `volatile u32 pre_pad[8];` first local, same `arg0 = 0;` cse2 FAKE with the same
 * mechanism, same local set, same pointer walk, same loop, same 5-argument call to
 * func_800482C8 - differing only in taking 4 s16 parameters reused pairwise instead of
 * 6 used once. func_80047EE8 (line 19) and func_800481E8 (line 140) are the same family
 * and also ship ("pre_pad", 8). All three hold granted rows at
 * engine/volatile_cheats.py:757-758 and :767. func_800480C0 is the one hole in a run of
 * four consecutive sibling functions.
 *
 * The alternative producers are all measured dead: see hypotheses.md s22/s23/s24.
 * The best REFERENCED-carrier form (s22 k4) scores 1 - flow.c:1740-1741 can never delete
 * the last store to a frame object - and its carrier is itself a T1-failing dead array.
 */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    volatile u32 pre_pad[8]; /* FAKE: phantom-frame-slot volatile filler reproducing the target's allocated-but-untouched 32-byte vars region sp+0x18..0x37 (zero sw/lw there, s18 $sp census), mechanism: GCC 2.7.2 get_frame_size/expand_decl reserves declared locals (config/mips/mips.c:4443-4475), lever-exhaustion: hypotheses.md s1-s24 - 104 rejected forms, every referenced producer costs >=1 store by flow.c:1740-1741 */
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: breaks the $a0 == base_addr value association so cse2 cannot canonicalise the two base copies, mechanism: cse.c canonical-register substitution, lever-exhaustion: hypotheses.md s1-s3 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            func_800482C8(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}
