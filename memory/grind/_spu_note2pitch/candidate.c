/* CANDIDATE — _spu_note2pitch (src/main.c)  sandbox --disable all = 2  (s1, 2026-09-06)
 *
 * CHASSIS NOTES (both required to reproduce this score):
 *  1. The sibling `_spu_2pitch` (COMPLETED-C, defined immediately above this
 *     function in src/main.c) must carry the GNU89 `inline` keyword:
 *         inline u32 _spu_2pitch(u32 atten, u32 rem) { ... }
 *     GCC 2.7.2 inlines it here (integrate.c) AND still emits the out-of-line
 *     body; measured byte-neutral for the sibling (sandbox _spu_2pitch = 0 with
 *     the keyword). The target's tail (0x103B curve walk, upper spilled to
 *     0x8($sp), the 16-byte frame) is that inlined copy — longhand copies of the
 *     loop give an 8-byte frame (rejected/longhand-frame8.c).
 *  2. include/m2c_context.h prototype is `u16 _spu_note2pitch(u16,u16,u16,u16);`
 *     (already so at HEAD; the only caller func_8008B488 is INCLUDE_ASM).
 *
 * RESIDUAL (2 insns): at the join label the target orders
 *     andi a2,v0,0xFFFF ; addiu a0,zero,0x103B
 * and this form orders them the other way. Mechanism (sched.c adjust_priority /
 * birthing_insn_p, sched1): the andi's destination is integrate.c's parm-copy
 * pseudo (single-set) so it is priority-boosted and placed as late as possible;
 * the li's destination `ratio` is multi-set (loop updates) so it is not. The
 * target's andi must write a MULTI-SET user variable (see hypotheses.md H7).
 */
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    u16 base;
    u32 pitch;

    cen = (cen_note << 7) + cen_fine;
    tgt = (note << 7) + fine;
    diff = tgt - cen;
    absdiff = diff;
    if (diff < 0) {
        absdiff = -diff;
    }
    rem = absdiff / 1536;
    oct = rem;
    rem = absdiff - oct * 1536;
    if (diff >= 0) {
        base = 0x1000 << oct;
    } else {
        if (rem != 0) {
            oct++;
            rem = 0x600 - rem;
        }
        base = 0x1000 >> oct;
    }
    pitch = _spu_2pitch(base, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
