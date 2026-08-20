/* func_800481E8 candidate — s1 (recon), re-verified s2 (structural). Floor 10.
 * APPLIED to src/text1b.c. Pure C, zero asm/pins. Residual 10 = frame delta only
 * (frame 40 vs 72, vars 0 vs 32): 2 sp adjusts + 8 reg save/restore offsets.
 * Loop + prologue staging byte-match. s2 MEASURED the phantom-slot axis DEAD for
 * this function (no combine-deletable extension can exist in its semantics; 52k
 * instrumented campaign: 0 clean vars>8 forms) — see evidence.md s2. Frame axis
 * has no sanctioned route left; next step is escalation modality.
 * Byte-identical alt spellings exist for the stream reads (s2 V6/V7):
 * `s32 a1w = *(u16*)p;` + arg `(s32)(s16)a1w`, and the u32/(u16) recast form.
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
