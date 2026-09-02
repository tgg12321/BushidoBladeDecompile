/* func_800480C0 — s1 (recon, 2026-09-02) handoff form. Bytes PROVEN: full build
 * SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa with this body in src/text1b.c
 * (tmp/grind/func_800480C0/s1/build.log). sandbox --disable all reads 20 ONLY
 * because engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS has no row for
 * func_800480C0 yet (the pad is stripped; all 20 residual insns are sp-offset
 * deltas of frame 0x38 vs 0x58). 6-arg twin of COMPLETED-C func_80047FBC
 * (same two constructs, same allowlist row shape). */
void func_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    /* FAKE: unwritten leading frame pad (phantom-frame-slot volatile pad local
     * family, owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:334).
     * Mechanism: GCC 2.7.2 function.c assign_stack_local reserves the array slot at
     * RTL-expand from the source DECL and never reclaims frame_offset, so a
     * declared-but-untouched local aggregate reproduces target's allocated-but-
     * unwritten 32-byte vars region (.frame $sp,88 - args 0x00-0x17, vars
     * 0x18-0x37, regs 0x38-0x57; ZERO sw/lw in 0x18-0x37 in
     * asm/funcs/func_800480C0.s). Lever-exhaustion: identical instruction stream
     * and frame layout to in-file siblings func_80047EE8 / func_80047FBC /
     * func_800481E8, whose honest producers were measured inert over 12+ sessions
     * (docs/grind/decisions.md:7401-7660, engine/volatile_cheats.py:753-767);
     * s1 ladder for THIS function in memory/grind/func_800480C0/evidence.md.
     * SOTN-master precedent: volatile u32 pad; // !FAKE: at src/st/sel/2C048.c:564
     * (docs/reference/sotn-construct-index.md:101); volatile u32 pad[4]; // FAKE at
     * src/st/sel/stream.c:80 (sotn-construct-index.md:103). */
    volatile u32 pre_pad[8]; // !FAKE: phantom-frame-slot volatile filler (owner ruling 2026-08-18)
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: dead store to a PARAM (dead-store-fake-exception family,
               * .claude/rules/dead-store-fake-exception.md). Mechanism: defeats
               * cse2's canonical-register substitution over the {arg0, p,
               * base_addr} equivalence class so the second pointer binds
               * addu $s0,$s2,$v0 (target insn #20) rather than $a0, and the
               * base copy binds addu $s2,$s0,$zero (target insn #5).
               * Lever-exhaustion: same lever Judge-PASSed on in-file siblings
               * func_80047FBC (decisions.md:981) and func_800481E8 (272e47c4);
               * without it this body measures 32 (s1 ladder). */
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
