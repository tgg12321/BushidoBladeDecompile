/* CANDIDATE — _spu_note2pitch (src/main.c)  sandbox --disable all = 0  (s2b, 2026-09-06)
 * Supersedes the s2 body (per-arm `(u16)` casts) that layer-1 FAILed and whose
 * casts are now on this function's BANNED list. No cast, no per-arm narrowing.
 *
 * CHASSIS (both required, unchanged from s1/s2):
 *  1. The sibling `_spu_2pitch` (COMPLETED-C, defined immediately above in
 *     src/main.c) must carry the GNU89 `inline` keyword:
 *         inline u32 _spu_2pitch(u32 atten, u32 rem) { ... }
 *     GCC 2.7.2 integrates it here AND still emits the out-of-line body;
 *     sandbox _spu_2pitch --disable all = 0 with the keyword in place. The
 *     target's tail (0x103B curve walk, `upper` spilled to 0x8($sp), the
 *     16-byte frame) IS that inlined copy.  Measured 2026-09-06 (s2b): making
 *     the sibling's first parameter `u16` instead breaks it (sibling 1,
 *     _spu_note2pitch 22) — the parameter stays u32.
 *  2. include/m2c_context.h:1184 prototype `u16 _spu_note2pitch(u16,u16,u16,u16);`
 *     (already so at HEAD; the sole caller func_8008B488 is INCLUDE_ASM).
 *
 * WHAT CLOSES THE LAST 2 INSNS (residual: `andi $a2,$v0,0xFFFF` vs
 *  `addiu $a0,$zero,0x103B` at .L8008BBB0):
 *  the u16->u32 widening of the octave attenuation must land in a pseudo that
 *  is NOT single-set at sched1 time, otherwise sched.c adjust_priority ->
 *  birthing_insn_p (reg_n_sets[i]==1) boosts the andi to LAUNCH_PRIORITY, the
 *  backward list scheduler picks it first and therefore EMITS it last (after
 *  the li).  Staging the value through `diff` — a local the function already
 *  owns, dead from the `if (diff >= 0)` test onward — makes that pseudo
 *  two-set, the boost does not fire, the pair falls to the LUID tie-break and
 *  comes out in the target's order.  `diff` is also the variable the target
 *  keeps in $a2, so the seats match too (`andi $a2,$v0,0xFFFF`).
 *  Measured this session: staging through `diff` = 0; through `absdiff` = 5;
 *  `tgt` = 8; `cen` = 28; `oct` = 28; a FRESH u32 receiver = 2 (single-set,
 *  boost still fires); no staging at all (widen at the call) = 2.
 *  Family: staged-value-reused-variable (.claude/rules/staged-value-reused-variable.md).
 */
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    u16 atten;
    u32 pitch;
    cen = (cen_note << 7) + cen_fine;
    tgt = (note << 7) + fine;
    diff = tgt - cen;
    absdiff = (diff < 0) ? -diff : diff;
    oct = absdiff / 1536;
    rem = absdiff % 1536;
    if (diff >= 0) {
        atten = 0x1000 << oct;
    } else {
        if (rem != 0) {
            oct++;
            rem = 0x600 - rem;
        }
        atten = 0x1000 >> oct;
    }
    /* FAKE: the octave attenuation is staged through `diff` (dead from the
       `diff >= 0` test above onward; nothing reads it after this point) and
       consumed on the very next line, mechanism: GCC 2.7.2 sched.c
       adjust_priority -> birthing_insn_p (reg_n_sets[regno]==1) — a two-set
       pseudo is not boosted to LAUNCH_PRIORITY, so the widening `andi` is
       emitted before the inlinee's `li 0x103B` as in the target,
       lever-exhaustion: memory/grind/_spu_note2pitch/hypotheses.md s1 H1/H5/H6
       and s2b (24 measured spellings, all >= 2 without this staging) */
    diff = atten;
    pitch = _spu_2pitch(diff, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
