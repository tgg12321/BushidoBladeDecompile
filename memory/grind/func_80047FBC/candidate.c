/* s13 candidate (escalation modality, 2026-08-20) -- INTEGRATION HANDOFF form.
 *
 * BYTES ARE PROVEN: with this body compiled AS WRITTEN (pad present), the
 * engine sandbox scored 0 / 65 target insns / 65 build insns / 0 rules
 * dropped (tmp/grind/func_80047FBC/s13/sandbox_bodyB_final.log; the measured
 * body differed from this one only in the ANNOTATION COMMENT TEXT, which cpp
 * strips before cc1 -- see the detector note below for why the wording had to
 * change).
 *
 * Two match constructs, both owner-sanctioned families:
 *   (1) `volatile u32 pre_pad[8];` -- phantom-frame-slot volatile pad local,
 *       the GENERAL family established by the owner ruling 2026-08-18 in
 *       .claude/rules/no-new-park-categories.md (SOTN-master exhibits
 *       src/st/sel/2C048.c:564 `volatile u32 pad; // !FAKE:` and
 *       src/st/sel/stream.c:80 `volatile u32 pad[4]; // FAKE`). Array form,
 *       first-decl position, `// !FAKE` annotation -- all three form
 *       constraints satisfied. Frame forensics (s7): target reserves 32
 *       locals bytes at sp+0x18..sp+0x37 and NO instruction in
 *       asm/funcs/func_80047FBC.s touches them.
 *   (2) `arg0 = 0; /* FAKE ... *\/` -- dead-store-fake-exception; this exact
 *       lever on this exact function was Judge-PASSed 2026-07-20
 *       (docs/grind/decisions.md:981). It closes the LAST insn (target #18
 *       `addu $s0,$s4,$v0`) by defeating the cse2 canonical-register fold
 *       proven at s6.
 *
 * MEASURED THIS SESSION (chassis 2026-08-20, post asm-until-matched migration):
 *   fully clean body (no pad, no FAKE store)        sandbox --disable all = 15
 *   + arg0=0 FAKE only (pad stripped by detector)   sandbox --disable all = 14
 *   + pre_pad only (no FAKE store)                  sandbox --disable all =  1
 *   + both, pad honoured by the build               sandbox --disable all =  0
 * The ledger's historical floor of 1 is DEAD: it was measured on a chassis
 * where `s32 buf[8]; (void)buf;` survived cheat-stripping. The current
 * stripper removes that pair, so the honest floor with the old candidate is 14.
 *
 * WHY THIS IS NOT candidate-ready: the sandbox ALWAYS strips cheat-asm (the
 * `--disable` flag only controls regfix/asmfix rules), so an unwritten
 * volatile pad reads as score 14 until the function has its per-function row
 * in `engine/volatile_cheats.py::_SANCTIONED_UNWRITTEN_PADS` -- exactly the
 * mechanism the 2026-08-18 ruling mandates ("the sanctioned pads are
 * allowlisted in the engine's volatile-cheat detector so honest floors read
 * true"). engine/ is outside a grind session's allowed surface, so this is an
 * INTEGRATION HANDOFF, not an endgame lock. Operator steps are in
 * docs/grind/decisions.md (2026-08-20 entry for func_80047FBC).
 *
 * DETECTOR HYGIENE NOTE (do not "fix" by rewording): the first spelling used
 * the name `pad` AND the word "pad" inside its own annotation comment. That
 * made `find_unused_local_arrays`' reference check see a second occurrence of
 * the token and skip the declaration entirely -- an accidental
 * detector-evasion that produced a falsely-honest score 0. The name was
 * changed to `pre_pad` (matching the allowlisted convention for
 * func_8001E404 / func_8001E6E4 / func_8003CF84) and the annotation reworded
 * so the token does NOT recur; the detector now flags it correctly and the
 * honest floor reads 14 as it should. The under-detection is an engine
 * hygiene issue reported in the decisions.md entry; it was NOT exploited.
 */
void func_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    volatile u32 pre_pad[8]; // !FAKE: phantom-frame-slot volatile filler (owner ruling 2026-08-18, .claude/rules/no-new-park-categories.md): target reserves 32 locals bytes at sp+0x18..sp+0x37 that no instruction touches; mechanism: GCC 2.7.2 get_frame_size reserves declared locals
    u32 *p;
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = arg0;
    p = (u32 *)arg0;
    arg0 = 0; /* FAKE: defeats cse2 canonical-reg substitution
                 that folds {reg 72 arg0, reg 78 p, reg 79 base_addr}
                 equivalence class at insn 36 - RTL-proven s6 */
    p = (u32 *)((s32)p + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
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
            efc_buki_draw_zanzou(new_var,
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg2,
                          (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
}
