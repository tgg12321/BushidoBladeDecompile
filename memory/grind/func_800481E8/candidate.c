/* func_800481E8 - src/text1b.c - s5 SYNTHESIS COMPOSITE, BYTES PROVEN.
 *
 * STATUS 2026-08-20 (grind s5, synthesis modality): this body was applied to
 * src/text1b.c in place of `INCLUDE_ASM("asm/funcs", func_800481E8);` (line 176) and
 * the FULL DRIVER BUILD produced SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
 * oracle (tmp/grind/func_800481E8/s5/build_pad.log). The built object's frame is
 * `addiu sp,sp,-72`, identical to target, and the whole 58-line body disassembles
 * identical to asm/funcs/func_800481E8.s
 * (tmp/grind/func_800481E8/s5/built_func.txt). The function is byte-matched by this C.
 *
 * `sandbox func_800481E8 --disable all` still prints 10, NOT 0 - the sandbox strips
 * the unwritten pad unconditionally until func_800481E8 has its row in
 * engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS
 * ("func_800481E8": frozenset({("pre_pad", 8)})). engine/ is outside a grind session's
 * allowed surface AND outside the add-scope-allow classes
 * (.claude/rules/integration-handoff-self-serve.md denylist), so this is an
 * INTEGRATION HANDOFF requiring the same one-line owner grant the two in-file siblings
 * received on 2026-08-20. Operator steps: docs/grind/decisions.md, the 2026-08-20 s5
 * func_800481E8 entry.
 *
 * THE SYNTHESIS THAT CLOSED IT (what s1-s4 had wrong). s2's five-way frame taxonomy
 * filed this function's 32 untouched bytes under "(v) unwritten TAIL", and the owner's
 * 2026-08-20 grant entry accordingly said the sibling ruling does not extend to
 * trailing/tail-pad shapes. That taxonomy label was a mis-read of the frame map. The
 * untouched region sits at sp+0x18..0x37: ABOVE the outgoing-args area (0x00-0x17,
 * whose 5th-arg slot is written by `sw $v0,0x10($sp)` at 0x80048288) and BELOW the
 * saved-regs block (0x38-0x47: s0/s1/s2/ra). That is the VARS region - exactly where a
 * FIRST-DECLARED local lands - and it is byte-for-byte the same 32-byte
 * args24/vars32/regs16 = .frame 72 layout as the two granted siblings
 * func_80047EE8 and func_80047FBC. So the correct family was the LEADING-pad shape the
 * owner had already sanctioned generally (2026-08-18) and granted twice in this very
 * file, not the tail-pad shape that has no precedent. One first-decl
 * `volatile u32 pre_pad[8];` closed the entire 10-instruction residual on the first
 * try, with no other change to the s1 chassis.
 *
 * Two annotated constructs, both in sanctioned families:
 *  - `volatile u32 pre_pad[8];` - phantom-frame-slot volatile pad local family (owner
 *    ruling 2026-08-18, .claude/rules/no-new-park-categories.md:390), ARRAY form,
 *    first-decl position, no (void) shim.
 *  - `arg0 = 0;` - dead-store-fake-exception (dead store to a PARAM), the same lever
 *    Judge-PASSed on the in-file sibling func_80047EE8.
 *
 * Everything below the pad is the s1 chassis verbatim (walking pointer `p`, base copy,
 * early a0_for_call precompute, always-advance in the delay slot), re-measured
 * unchanged at floor 10 on the post-migration chassis by s4 and again by s5.
 * Prior-session notes retained below for provenance:
 *
 * s4 (permuter, 2026-08-20) CHASSIS RE-VERIFICATION -- READ THIS FIRST:
 *   - HEAD changed under this ledger: the 2026-08-19 asm-until-matched migration
 *     replaced the old 12-regfix-rule + $16-pin + INLINE_MOVE_ALIASING form in
 *     src/text1b.c with `INCLUDE_ASM("asm/funcs", func_800481E8);`. regfix.txt now
 *     holds only a COMMENT line for this function (regfix.txt:2442), zero rules.
 *   - The callee was renamed by a naming wave: the old `efc_buki_draw_zanzou` in
 *     this file is now `func_800482C8` (the jal target in asm/funcs/func_800481E8.s).
 *     This file has been updated accordingly. There is NO prototype for it above
 *     func_800481E8 in src/text1b.c (it is defined below), so the call is
 *     unprototyped/int-promoted -- the same ABI the banked floor was measured under.
 *   - MEASURED on the new chassis (s4): applying this file over the INCLUDE_ASM line
 *     gives sandbox --disable all = 10, 56/56 insns, rules_dropped 0,
 *     cheat_asm_stripped 278. Floor is UNCHANGED at 10 and the residual is still
 *     exactly the 10 frame-offset instructions. The migration is codegen-neutral here.
 *
 * s4-RERUN (permuter, 2026-08-20) -- RE-MEASURED AGAIN on HEAD after the driver
 * discarded the first s4 session and reverted its src/ edits: applying this file
 * over `INCLUDE_ASM("asm/funcs", func_800481E8);` at src/text1b.c:68 gives
 * sandbox --disable all = 10, 56/56 insns, rules_dropped 0, cheat_asm_stripped 278.
 * Unchanged. A third permuter seed (directed hybrid, PERM_GENERAL x PERM_LINESWAP
 * spanning both prior chassis) ran 46,124 iterations with ZERO finds -- banked in
 * rejected/directed-hybrid-seed-permuter-dry.c. Also measured: struct-walk (47
 * insns) and array-index (49 insns) chassis LOSE body instructions, so the
 * walking-pointer spelling below is load-bearing, not stylistic.
 */
void func_800481E8(s32 arg0, s32 arg1)
{
    /* Pure C (s1 recon): the sibling InitHiraRmd_80047FBC prologue technique
     * transfers — base-copy staging + function-scope precompute makes GCC
     * stage arg0 through $s0 first ($s0=$a0; $s2=$s0), replacing the former
     * INLINE_MOVE_ALIASING __asm__ + $16 pin. The `arg0 = 0;` dead store is
     * load-bearing FAKE-family (dead-store-fake-exception, sibling
     * precedent in-file): without it GCC keeps arg0 live in $a0 and emits
     * `addu $s0,$a0,$v0` (measured, s1 probe); with it the second pointer
     * binds to base in $s2 — matching target.
     */
    /* FAKE: unwritten leading frame pad (phantom-frame-slot volatile pad local
     * family, owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md:390).
     * Mechanism: GCC 2.7.2 function.c assign_stack_local reserves the array slot at
     * RTL-expand from the source DECL and never reclaims frame_offset after DCE, so a
     * declared-but-untouched local aggregate reproduces target's allocated-but-unwritten
     * 32-byte vars region (.frame $sp,72 - args 0x00-0x17 incl the 5th-arg slot
     * sw $v0,0x10($sp); vars 0x18-0x37 with ZERO sw/lw; regs 0x38-0x47). Identical
     * shape and size to the two granted siblings in this file (func_80047EE8 /
     * func_80047FBC, owner ruling 2026-08-20). Lever-exhaustion: recon (s1),
     * structural entry-condition bisection + 11-variant .frame grid (s2), AND-gate
     * re-evaluation (s3), ~152k permuter iterations across three independent seeds
     * (s2/s4) - every honest producer measured inert; see hypotheses.md.
     * SOTN-master precedent: volatile u32 pad; // !FAKE: at src/st/sel/2C048.c:564
     * (docs/reference/sotn-construct-index.md:101); volatile u32 pad[4]; // FAKE at
     * src/st/sel/stream.c:80 (sotn-construct-index.md:103). */
    volatile u32 pre_pad[8];
    u32 *p;
    u32 *base;
    s32 count;
    s32 a0_for_call;
    p = (u32 *)arg0;
    base = p;
    arg0 = 0; /* FAKE: breaks $a0==base association, see block comment */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            u16 v0v;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = *((u16 *)p);
            new_var2 = word >> 2;
            a0_for_call = (s32)base + (new_var2 << 2);  /* compute early (target sched) */
            p = (u32 *)(((s32)p) + 2);  /* always advance (target delay slot) */
            if ((s32)a3v < 0x280) {
                v0v += 1;
            }
            func_800482C8(a0_for_call,
                          (s32)a1v,
                          (s32)a2v,
                          (s32)a3v,
                          (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}
