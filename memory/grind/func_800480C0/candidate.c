/* func_800480C0 - CANDIDATE (s24 synthesis, 2026-09-06) - SOLVED THIS SESSION.
 *
 * REPRESENTATION NOTE: before this session main/HEAD carried
 * `INCLUDE_ASM("asm/funcs", func_800480C0);` at src/text1b.c:129 per the
 * asm-until-matched ruling (2026-08-19). This body is now installed there and is
 * submitted as candidate-ready; this file mirrors the installed body verbatim.
 *
 * MEASURED THIS SESSION (chassis 215489c4 + owner grant commit 661c01ef):
 *   - sandbox func_800480C0 --disable all -> "score": 0, 74/74 insns,
 *     rules_dropped 0  (tmp/grind/func_800480C0/s24/sandbox0.txt)
 *   - verify-oracle -> "ok": true, build/bb2.exe sha1
 *     62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
 *     (tmp/grind/func_800480C0/s24/verify_oracle.txt)
 *
 * WHAT CHANGED vs s23: nothing in the codegen. The body is the one banked since s23
 * (rejected/s23-volatile-pre-pad8-bytes-proven-sandbox-strips-to-20.c), which already
 * built the target's exact object but scored 20 because engine/volatile_cheats.py
 * stripped the unreferenced pad for want of an allowlist row. The owner added that row
 * on 2026-09-06 (engine/volatile_cheats.py:779, commit 661c01ef) and directed that the
 * annotations be respelled into the family's `// !FAKE:` form and that the `arg0 = 0;`
 * dead store be argued explicitly for layer-1. Both done; the argument is in
 * memory/grind/func_800480C0/self_vet.md.
 *
 * WHY THIS FORM AND NOT ANOTHER: func_80047FBC (src/text1b.c:83, COMPLETED-C on main)
 * is line-for-line this same body - same first-declared volatile u32 pre_pad[8], same
 * arg0 = 0 cse2 FAKE, same local set, same pointer walk, same loop, same 5-argument call
 * to func_800482C8 - differing only in taking 4 s16 parameters reused pairwise instead
 * of 6 used once. func_80047EE8 (:19) and func_800481E8 (:140) are the same family and
 * also ship ("pre_pad", 8) rows, at engine/volatile_cheats.py:757-758 and :767. Every
 * alternative producer is measured dead across s18-s23: the best REFERENCED-carrier form
 * (s22 k4) scores 1, because flow.c:1740-1741 never deletes the last store to a frame
 * object; the phantom family caps at 8 bytes (s22); integrate.c:2124 never fires;
 * donations do not stack; and widening the call puts the 7th argument word inside the
 * untouched window (s18).
 */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    volatile u32 pre_pad[8]; // !FAKE: phantom-frame-slot volatile filler (owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:422-434; grant row engine/volatile_cheats.py:779, commit 661c01ef): the target reserves 32 locals bytes at sp+0x18..sp+0x37 that no instruction in asm/funcs/func_800480C0.s reads or writes; mechanism: GCC 2.7.2 get_frame_size/expand_decl reserves declared locals (config/mips/mips.c:4443-4475); lever-exhaustion: memory/grind/func_800480C0/hypotheses.md s1-s23, 104 rejected forms, every referenced producer costs >=1 store (flow.c:1740-1741 never deletes the last store to a frame object)
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; // !FAKE: dead store to a PARAMETER (sanctioned dead-store family, .claude/rules/dead-store-fake-exception.md; identical construct in the matched sibling func_80047FBC at src/text1b.c:91). It defeats cse2's canonical-register substitution, which otherwise folds the {arg0, p, base_addr} equivalence class and emits one base copy instead of two; mechanism: GCC 2.7.2 cse.c canonical-reg substitution; lever-exhaustion: hypotheses.md s1-s3
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
