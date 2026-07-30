/* REJECTED (session 1) -- local SCALAR spellings are frame-inert AND
 * codegen-inert for func_80086014. Do not re-propose any of them.
 *
 * WHY THIS MATTERS: the residual on func_80086014 is dominated by target's
 * 8-byte PHANTOM frame (addiu $sp,$sp,-8 with zero $sp traffic). The obvious
 * first idea is that the original source declared ordinary locals which GCC
 * 2.7.2 reserved frame bytes for while register-allocating them away -- the
 * documented [[phantom-frame-slots-gcc272]] artifact. That idea is DEAD for
 * this function's shape, measured, ten ways.
 *
 * METHOD: all ten spellings below were compiled in one translation unit with
 * the project's exact cc1 flags
 *   -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls
 *   -fno-builtin -w
 * and cc1's own `.frame` comment was read (`vars=` IS get_frame_size(), the
 * direct frame gradient -- strictly better than the sandbox score, which
 * cannot separate "wrong frame" from "wrong codegen").
 * Harness: tmp/grind/func_80086014/s1/{frame_probe.c,run_probe.sh}.
 *
 * RESULT: every one of the ten reported
 *     .frame $sp,0,$31   # vars= 0, regs= 0/0, args= 0, extra= 0
 * and every one emitted an IDENTICAL 16-instruction body (verified by insn
 * count, and vA/vB additionally verified byte-identical through the full
 * pipeline via `sandbox --disable all` + objdump: both score 10, both 25
 * built insns). So these are not merely "didn't help" -- they are the same
 * program to GCC. There is no gradient along this axis at all.
 *
 * The ten spellings, all vars=0:
 *   vA  the naive form (no locals)                  <- kept as candidate.c
 *   vB  s32 index local + u8 flags local
 *   vC  s16 (HImode) flags local, HImode bitwise or
 *   vD  TWO HImode locals feeding the HImode bitwise or -- this is the exact
 *       "minimal trigger" shape [[phantom-frame-slots-gcc272]] documents
 *       (two s16 locals feeding an HImode bitwise expression). It does NOT
 *       reproduce in this function's context. The documented trigger is
 *       context-dependent, not a shape you can transplant.
 *   vE  s16 index local + s16 flags local
 *   vF  u16 flags local
 *   vG  shared s32 return-value local + early-return guard inversion
 *   vH  byte-offset pointer spelling instead of a scaled index
 *   vI  both s16 params staged through s16 locals
 *   vJ  s32 params with (s16) casts at each use
 *
 * WHAT IS STILL ALIVE (do not confuse with this kill): >=4-byte local
 * AGGREGATES are NOT inert -- struct/union/array locals DO produce
 * `vars= 8` here, several with zero stack traffic. That is the live frontier;
 * see hypotheses.md H1 and rejected/union-word-view-pack-unpack-tax.c for how
 * far it got.
 *
 * Representative member of the dead family (vE), for the record:
 */
s32 func_80086014_REJECTED_vE(s16 idx, s16 x, s16 y)
{
    s16 i, flags;

    if ((u16)idx < 0x18) {
        i = idx;
        D_80102A7A[i * 8] = y;
        flags = D_800F65E0[i];
        D_80102A78[i * 8] = x;
        D_800F65E0[i] = flags | 3;
        return 0;
    }
    return -1;
}
