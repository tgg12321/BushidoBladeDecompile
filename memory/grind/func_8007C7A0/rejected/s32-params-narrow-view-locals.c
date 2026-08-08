/* REJECTED s2 (2026-08-08, structural) — sandbox 25, build_insns 44 (7 short
 * of target 51).
 *
 * Motivation: _SsVmVSetUp (src/main.c:1262, COMPLETED-C, 2-param Sony LIBSND
 * code) opens with `addu $a3,$a0,$zero` — a raw carrier copy of param0 into
 * $a3 — from the spelling `s32 a0` param + separate narrow local
 * `u16 a0h = a0;`. Hypothesis: the same spelling family (wide s32 params +
 * named s16 view locals, in-range clamp arm reading the RAW param) would
 * reproduce get_cs's carrier=$a3 allocation. Distinct from round-13 T4
 * (s32 sig + inline `(s16)` casts + param self-assign, score 24/44), which
 * dropped the carrier entirely.
 *
 * Measured result: the clamp-result local `cx` COALESCES into $a0 (the
 * `cx = arg0` copy makes cx's home the param's own hard reg once arg0 is
 * dead), so NO carrier copy is emitted at all; GCC also CSEs `D - 1` into a
 * single addiu per clamp (the s16-param spelling's double-decrement +
 * raw-limit-save `move $a2,$v0` disappear). Stream collapses to 44 insns.
 * The s32-param family cannot reproduce the 51-insn stream: the double
 * lim-1 decrement and the raw-limit save are artifacts of the s16-param
 * promotion pattern, not reachable from wide params + narrow views.
 *
 * Verdict: KILLED. _SsVmVSetUp's $a3 carrier is explained by the standard
 * model (its carrier lives nearly the whole 54-insn function and conflicts
 * with $v0/$v1/$a0/$a1 uses plus the $a2-resident table value at insns
 * 34-48; ascending scan lands $a3) — nothing transfers to a 51-insn leaf
 * whose conflict graph the s0/s1/s2 scans have exhaustively closed.
 */
s32 func_8007C7A0(s32 arg0, s32 arg1)
{
    s16 x;
    s16 y;
    s32 cx;
    s32 lo;
    s32 hi;

    x = arg0;
    if (x >= 0) {
        if ((D_8009BE78 - 1) < x) {
            cx = D_8009BE78 - 1;
        } else {
            cx = arg0;
        }
    } else {
        cx = 0;
    }
    y = arg1;
    if (y >= 0) {
        if ((D_8009BE7A - 1) < y) {
            arg1 = D_8009BE7A - 1;
        }
    } else {
        arg1 = 0;
    }
    hi = arg1 & 0xFFF;
    if ((u32)(D_8009BE74 - 1) >= 2U) {
        hi = arg1 & 0x3FF;
        hi = hi << 0xA;
        lo = cx & 0x3FF;
    } else {
        hi = hi << 0xC;
        lo = cx & 0xFFF;
    }
    {
        s32 pkt = lo | 0xE3000000;
        return hi | pkt;
    }
}
