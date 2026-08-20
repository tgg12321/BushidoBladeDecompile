/* func_80047EE8 (AddTbpOfst_80047EE8) - src/text1b.c - s12 COMPOSITE, BYTES PROVEN.
 *
 * STATUS 2026-08-20 (grind s12, escalation modality): this body was applied to
 * src/text1b.c in place of `INCLUDE_ASM("asm/funcs", func_80047EE8);` and the FULL
 * DRIVER BUILD produced SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
 * (tmp/grind/func_80047EE8/s12/build_oracle.log). The built object's frame is
 * `addiu sp,sp,-72`, identical to target (tmp/grind/func_80047EE8/s12/built_func.txt).
 * The function is byte-matched by this C.
 *
 * `sandbox func_80047EE8 --disable all` still prints 10, NOT 0 - the sandbox strips
 * cheat-asm unconditionally, and the sanctioned unwritten pad is stripped until
 * func_80047EE8 has its row in engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS
 * ("func_80047EE8": frozenset({("pre_pad", 8)})). engine/ is outside a grind session's
 * allowed surface, so this is an INTEGRATION HANDOFF, not an endgame lock. Operator
 * steps: docs/grind/decisions.md, 2026-08-20 func_80047EE8 entry.
 *
 * Two annotated constructs, both in sanctioned families:
 *  - `volatile u32 pre_pad[8];` - phantom-frame-slot volatile pad local family (owner
 *    ruling 2026-08-18, .claude/rules/no-new-park-categories.md:390). Supplies target's
 *    allocated-but-untouched 32-byte vars region (0x18-0x37, ZERO sw/lw). Closes the
 *    entire 10-instruction frame-offset residual the ledger carried since s1.
 *  - `arg0 = 0;` - dead-store-fake-exception (dead store to a PARAM), Judge-PASSed on
 *    the sibling's identical lever; 6 pure spellings measured dead on this body at s2.
 *
 * What this body DELETES from the pre-migration on-main form: 2 register-asm pins
 * (asm("$16"), asm("$18")), 1 INLINE_MOVE_ALIASING __asm__ block, the unqualified
 * `s32 unused_slack[8]` and its `(void)unused_slack;` detector shim. Net cheat count
 * strictly decreases; nothing here is a pin, an __asm__, an alias rename, or a
 * regfix/asmfix rule.
 *
 * Chassis levers that produced the floor-10 base (s1, retained verbatim): single-walker
 * pointer `p`; LIVE precompute of the call's first arg into `first` inside the loop
 * (consumed by the call - not a FAKE construct); the arg0=0 FAKE dead store.
 * Frame forensics naming the mechanism: evidence.md [s6]/[s7].
 */
void func_80047EE8(s32 arg0, s32 arg1)
{
    /* FAKE: unwritten leading frame pad (phantom-frame-slot volatile pad local
     * family, owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:390).
     * Mechanism: GCC 2.7.2 function.c assign_stack_local reserves the array slot at
     * RTL-expand from the source DECL and never reclaims frame_offset after DCE, so a
     * declared-but-untouched local aggregate reproduces target's allocated-but-unwritten
     * 32-byte vars region (.frame $sp,72 - args 0x00-0x17, vars 0x18-0x37, regs
     * 0x38-0x47; ZERO sw/lw in 0x18-0x37 - frame forensics in
     * memory/grind/func_80047EE8/evidence.md [s6]/[s7], cc1 size-pin puts the original
     * aggregate at 7-8 words). Lever-exhaustion: 9 structural .frame variants (s3),
     * ~26,500 permuter iters across two distinct basins (s4/s5), forensics (s6/s7),
     * rederive (s8/s9) - every honest producer measured inert; see hypotheses.md.
     * SOTN-master precedent: volatile u32 pad; // !FAKE: at src/st/sel/2C048.c:564
     * (docs/reference/sotn-construct-index.md:101); volatile u32 pad[4]; // FAKE at
     * src/st/sel/stream.c:80 (sotn-construct-index.md:103). */
    volatile u32 pre_pad[8];
    u32 *p;
    s32 saved;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    p = (u32 *) arg0;
    saved = (s32) p;
    arg0 = 0; /* FAKE: dead store to a PARAM (dead-store-fake-exception family,
               * .claude/rules/dead-store-fake-exception.md). Mechanism: defeats cse2's
               * canonical-register substitution over the {arg0, p, saved} equivalence
               * class so the second pointer binds addu $s0,$s2,$v0 rather than $a0.
               * Lever-exhaustion: 6 pure spellings of this init chain measured dead on
               * this body at s2 (rejected/pure-*.c). */
    p = (u32 *) ((s32) p + (((s32) (arg1 << 16)) >> 14));
    v_off = *p;
    p = (u32 *) (saved + ((v_off >> 2) << 2));
    count = *(p++);
    if (count != 0)
    {
        count--;
        do
        {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            s32 first;
            word = *p;
            p = (u32 *) (((s32) p) + 4);
            a1v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a2v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a3v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            new_var = a1v;
            new_var2 = word >> 2;
            first = saved + (new_var2 << 2);
            v0v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            func_800482C8(first, new_var, a2v, a3v, v0v);
        }
        while ((count--) != 0);
    }
}
