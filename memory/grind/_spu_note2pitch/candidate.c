/* CANDIDATE — _spu_note2pitch (src/main.c)  sandbox --disable all = 0  (s2, 2026-09-06)
 * FULL BUILD verify-oracle ok:true with this body in place. COMPLETED-C form.
 *
 * CHASSIS (both required):
 *  1. The sibling `_spu_2pitch` (COMPLETED-C, defined immediately above in
 *     src/main.c) must carry the GNU89 `inline` keyword:
 *         inline u32 _spu_2pitch(u32 atten, u32 rem) { ... }
 *     GCC 2.7.2 integrates it here AND still emits the out-of-line body;
 *     sandbox _spu_2pitch --disable all = 0 with the keyword in place. The
 *     target's tail (0x103B curve walk, `upper` spilled to 0x8($sp), the
 *     16-byte frame) IS that inlined copy.
 *  2. include/m2c_context.h:1184 prototype `u16 _spu_note2pitch(u16,u16,u16,u16);`
 *     (already so at HEAD; the sole caller func_8008B488 is INCLUDE_ASM).
 *
 * WHAT CLOSED THE LAST 2 INSNS (s1 residual: andi/li order at .L8008BBB0):
 *  narrowing the octave base to u16 INSIDE EACH ARM (instead of once after the
 *  join) gives the widened receiver `atten` two static sets at sched1 time, so
 *  sched.c birthing_insn_p (reg_n_sets==1) no longer boosts the andi to
 *  LAUNCH_PRIORITY; jump2 cross-jumping then re-merges the two identical
 *  `andi` tails into the single one the target has after the join label.
 *  Measured: one-arm-only narrowing (vS) = 4, post-join narrowing (vF) = 2.
 */
u16 _spu_note2pitch(u16 cen_note, u16 cen_fine, u16 note, u16 fine) {
    s32 cen;
    s32 tgt;
    s32 diff;
    s32 absdiff;
    s32 oct;
    s32 rem;
    u32 atten;
    u32 pitch;
    cen = (cen_note << 7) + cen_fine;
    tgt = (note << 7) + fine;
    diff = tgt - cen;
    absdiff = (diff < 0) ? -diff : diff;
    oct = absdiff / 1536;
    rem = absdiff % 1536;
    if (diff >= 0) {
        atten = (u16)(0x1000 << oct);
    } else {
        if (rem != 0) {
            oct++;
            rem = 0x600 - rem;
        }
        atten = (u16)(0x1000 >> oct);
    }
    pitch = _spu_2pitch(atten, (rem < 0) ? -rem : rem);
    if (pitch >= 0x4000) {
        pitch = 0x3FFF;
    }
    return pitch;
}
