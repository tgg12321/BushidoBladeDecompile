/* REJECTED (grind s3, structural) — score 2, i.e. EXACTLY the base: inert.
 *
 * The idea: `p[4] = 0` style indexing sets MEM_IN_STRUCT_P (visible as `mem/s`
 * in the RTL dumps) while `*(s16 *)(p + 8) = 0` does not. sched.c's
 * true_dependence/anti_dependence (tools/gcc-2.7.2/sched.c:817-866) exempt an
 * in-struct MEM with a varying address from conflicting with a MEM that is
 * neither in-struct nor address-varying — which is exactly the `lw` of the
 * scalar global D_800A374C — but the exemption explicitly EXCLUDES QImode. So
 * the twelve RGB byte stores can never escape the barrier, while HImode
 * coordinate stores written as `sp[i]` (with `s16 *sp = (s16 *)p;`) CAN.
 *
 * Measured: identical score (2) and, with the target statement order, identical
 * scores to the plain-deref spellings (8 / 9 / 8 for the H2/H3/H4 variants).
 * Dissolving the dependence changes nothing because GCC 2.7.2's list scheduler
 * is movement-MINIMIZING: freeing an insn to move does not give it a reason to
 * move. Emitted order still tracks source order for every tied insn.
 *
 * Do not re-derive: aliasing freedom is not the binding constraint on this
 * function. Hard-register reuse of the 640 constant is (see
 * holder-const-set-sunk-by-sched1.c).
 */
void func_8003553C(void) {
    u8 *p;
    u8 *q;
    u32 *ot;
    s16 *sp;

    p = (u8 *)D_800A38B4;
    initPolyG4(p);
    sp = (s16 *)p;
    sp[8] = 640;
    sp[13] = 240;
    sp[17] = 240;
    p[4] = 0;
    p[5] = 0;
    p[6] = 0x80;
    p[0xC] = 0;
    p[0xD] = 0;
    p[0xE] = 0x80;
    p[0x14] = 0;
    p[0x15] = 0;
    p[0x16] = 0;
    p[0x1C] = 0;
    p[0x1D] = 0;
    p[0x1E] = 0;
    ot = (u32 *)(D_800A374C + 0x401C);
    sp[4] = 0;
    sp[5] = 0;
    sp[9] = 0;
    sp[12] = 0;
    sp[16] = 640;
    q = p;
    p += 0x24;
    ot_Link(ot, (u32 *)q);
    D_800A38B4 = p;
}
