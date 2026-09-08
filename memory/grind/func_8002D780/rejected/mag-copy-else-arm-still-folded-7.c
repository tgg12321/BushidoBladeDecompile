/* func_8002D780 - grind candidate (s2 structural, 2026-09-08). Honest sandbox floor
 * 10/202 (s1 left it at 47/202; measured this session with these exact edits in
 * src/code6cac_b.c: `sandbox func_8002D780 --disable all` -> score 10, build_insns ==
 * target_insns == 202).
 *
 * What moved the floor this session (all ordinary C, no FAKE construct present):
 *  1. 47 -> 23: the two cross products of each triangle side test are computed into
 *     NAMED LOCALS (kc = the centroid-side cross product, kp = the point-side cross
 *     product) instead of being written as one nested expression.  Mechanism: with the
 *     cross products as separate statements, cc1 emits the two multiply pairs as
 *     separate RTL groups, and sched1 (tools/gcc-2.7.2/sched.c) then produces the
 *     target's multiply order in block 5 instead of interleaving the centroid fixups.
 *  2. 23 -> 14: the sign test is written `(kc ^ kp) >= 0` (centroid term first), which
 *     is the operand order the target's three tests use; and kc/kp are REUSED for the
 *     second and third tests rather than being fresh locals per test (fresh locals per
 *     test measured 52 -- see rejected/).
 *  3. 14 -> 10: the LZCS island's clobber list is the sibling footprint
 *     "$12","$13","$14","$15" instead of "$12" alone.  This is the byte-matched
 *     sibling spelling that ships on main for the SAME island in this TU
 *     (func_8002BC68 src/code6cac_b.c:762-767 and func_8002BEA0 :825-830, whose
 *     in-line comment records the 2026-07-28 judge ruling: mentioning $13-$15 in the
 *     RTL is the only route by which reload1.c can pick $24 for the reload-emitted
 *     mflo).  Here it moves our `mflo $t7` (= $15) to the target's `mflo $t8` (= $24)
 *     and takes two further register-permutation insns with it.
 *  4. byte-neutral hygiene: the redundant `(u8 *)` casts on `&D_8008D118` are dropped
 *     (the symbol is already declared `extern u8` at src/code6cac_b.c:279), which
 *     removes the two declaration puns the s2 brief flagged.  Measured identical (10).
 *
 * Remaining 10 differing insns (tmp/grind/func_8002D780/s2/pairdiff_g1.txt):
 *  A. 2 insns -- the third (edge) test's delay-slot pick: target fills the test-2 bltz
 *     slot with `subu v0,t2,t1` (cx - x0), we fill it with `subu a0,t0,a3` (z2 - z0).
 *  B. 5 insns -- cluster 2: the target keeps a second pseudo holding `dist` in $a0
 *     (`move a0,s1` in the beqz delay slot) that feeds the LZCS input and the `srlv`,
 *     while $s1 keeps the compare / LUT index / call arguments; and its LZCR frame slot
 *     is addressed as `addiu v0,sp,16; move t4,v0` rather than our `move t4,sp`.
 *  C. 3 insns -- cluster 3: reorg.c will not steal `li v0,1` into the second
 *     post-call branch's slot (see evidence.md s1 for the instrumented trace).
 *
 * The three cop2 blocks are the owner-authorized canonical LZCS/LZCR + mvmva idiom
 * (.claude/rules/cop2-addressing-preamble-cluster.md). */
s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        s32 *vin;
        s32 *vout;
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        vin = (s32 *)(obj + 0xF8);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "lwc2 $0, 0($t4)\n"
            "lwc2 $1, 4($t4)\n"
            "nop\n"
            "nop\n"
            ".word 0x4A486012"
            : : "r"(vin) : "$12", "memory");
        vout = (s32 *)(obj + 0x100);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "swc2 $25, 0($t4)\n"
            "swc2 $26, 4($t4)\n"
            "swc2 $27, 8($t4)"
            : : "r"(vout) : "$12", "memory");
    }

    {
        s32 y = *(s32 *)(obj + 0x108);
        if (y < -threshold || threshold < y) return 0;
    }

    {
        s32 x0 = *(s32 *)(obj + 0xA8);
        s32 x2 = *(s32 *)(obj + 0xB8);
        s32 z0 = *(s32 *)(obj + 0xAC);
        s32 z2 = *(s32 *)(obj + 0xBC);
        s32 cx = (x0 + x2) / 3;
        s32 cz = (z0 + z2) / 3;
        s32 px = *(s32 *)(obj + 0x100);
        s32 pz = *(s32 *)(obj + 0x104);
        s32 kc = z0 * cx - x0 * cz;
        s32 kp = z0 * px - x0 * pz;

        if ((kc ^ kp) >= 0) {
            kc = z2 * cx - x2 * cz;
            kp = z2 * px - x2 * pz;
            if ((kc ^ kp) >= 0) {
                s32 ax = cx - x0;
                s32 az = cz - z0;
                s32 bx = px - x0;
                s32 bz = pz - z0;
                s32 dz = z2 - z0;
                s32 dx = x2 - x0;
                kc = dz * ax - dx * az;
                kp = dz * bx - dx * bz;
                if ((kc ^ kp) >= 0)
                    return 1;
            }
        }
    }

    {
        s32 y = *(s32 *)(obj + 0x108);
        s32 sp_var;
        s32 dist = r_sq - y * y;
        s32 sqrt_val;
        s32 *p118;
        s32 *p124;
        s32 *p10C;

        if ((u32)dist < 0x400) {
            sqrt_val = (u32)*((&D_8008D118) + dist) >> 3;
        } else {
            s32 mag = dist;
            s32 lzcr = 0;
            if (mag >= 0) {
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu $t4, %2, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(mag), "r"(&sp_var) : "$12", "$13", "$14", "$15");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *((&D_8008D118) + ((u32)mag >> shift));
                sqrt_val = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        p118 = (s32 *)(obj + 0x118);
        p124 = (s32 *)(obj + 0x124);
        p118[0] = *(s32 *)(obj + 0xA8) - *(s32 *)(obj + 0x100);
        p118[1] = *(s32 *)(obj + 0xAC) - *(s32 *)(obj + 0x104);
        p124[0] = *(s32 *)(obj + 0xB8) - *(s32 *)(obj + 0x100);
        p124[1] = *(s32 *)(obj + 0xBC) - *(s32 *)(obj + 0x104);
        if (func_8002D518(sqrt_val, dist, p118, p124) != 0) return 1;

        p10C = (s32 *)(obj + 0x10C);
        p10C[0] = -*(s32 *)(obj + 0x100);
        p10C[1] = -*(s32 *)(obj + 0x104);
        if (func_8002D518(sqrt_val, dist, p10C, p118) != 0) return 1;

        if (func_8002D518(sqrt_val, dist, p10C, p124) != 0) return 1;
        return 0;
    }
}
