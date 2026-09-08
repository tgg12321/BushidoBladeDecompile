/* s4 (permuter modality, 2026-09-08): BODY UNCHANGED - the floor is still 7/202,
 * re-measured this session with this exact body spliced into src/code6cac_b.c.  A
 * 55,531-iteration decomp-permuter campaign on this chassis (the first faithful
 * workspace this function has ever had - tmp/grind/func_8002D780/s4/nonmatchings/
 * func_8002D780, rebuild recipe in evidence.md [s4]) produced 12 finds and no
 * improvement; its best find re-measures at 8 on the engine.  Nine embedded-
 * assignment spellings of the residual-B dist copy also measured 7 = control.
 * See hypotheses.md [s4]. */
/* func_8002D780 - grind candidate (s3 structural, 2026-09-08). Honest sandbox floor
 * 7/202 (s2 left it at 10/202). Measured THIS session with these exact edits in
 * src/code6cac_b.c: `sandbox func_8002D780 --disable all` -> score 7, and
 * build_insns == target_insns == 202. Diff: tmp/grind/func_8002D780/s3/pairdiff_e2.txt.
 *
 * What moved the floor this session (all ordinary C; no FAKE construct present):
 *  1. 10 -> 7, and the whole tail residual (cluster C, 3 insns) CLOSED: the third
 *     func_8002D518 call is written as a test with an explicit `return 0` fall-out
 *
 *         if (func_8002D518(sqrt_val, dist, p10C, p124) != 0) return 1;
 *         return 0;
 *
 *     instead of `return func_8002D518(...) != 0;`.  All three post-call exits are now
 *     the same shape, jump.c's cross-jump merges them differently, and reorg.c
 *     (fill_simple_delay_slots) steals `li v0,1` into the SECOND post-call branch's
 *     delay slot exactly as the target does -- the standalone `.L303: li v0,1` block
 *     and the `j` over it are both gone.  s1 and s2 measured eleven tail SPELLINGS
 *     (nested `== 0`, goto-to-shared-label, truthy if, result local, ternary, `!!`,
 *     ...) at no change; this one is not a respelling of the tail but a change to the
 *     BLOCK STRUCTURE, which is what the s2 frontier said to attack.
 *  2. This alone lands at 201 insns (one short of the target's 202), which finally
 *     lets the s2-banked LZCR slot-address operand pay for itself: passing
 *     `"r"(&sp_var)` as a third operand of the LZCS island makes cc1 materialise the
 *     frame-slot address into a pseudo, reproducing the target's
 *     `addiu v0,sp,16; move t4,v0` instead of our `move t4,sp` -- and takes the count
 *     back to exactly 202.  s2 measured that operand at 203 insns and could not adopt
 *     it; residuals B and C were coupled by the insn count, and (1) is what unlocked it.
 *  3. Byte-neutral-but-closer: the third triangle side test is written as the cross
 *     product of two vertex-0-relative vectors, with all six differences as named
 *     locals (ax/az = centroid relative to vertex 0, bx/bz = the point, dx/dz = the
 *     0->2 edge).  Score is unchanged at 7, but it moves half of residual A: the
 *     target's `subu v0,t2,t1` (cx - x0) now precedes `subu a0,t0,a3` (z2 - z0) at the
 *     top of the test-3 block, matching insn-for-insn.  Declaration ORDER is
 *     load-bearing here -- see the header note below.
 *
 * DO NOT "clean up" the declaration order of ax/az/bx/bz/dz/dx.  Measured this session
 * (all at 202 insns): ax,az,bx,bz,dz,dx = 7 (this form); ax,az,dz,dx,bx,bz = 7;
 * ax,dz,dx,az,bx,bz = 14; ax,dx,dz,az,bx,bz = 14; ax,dz,dx,az,bz,bx = 14;
 * ax,dz,dx,bx,bz,az = 14; dx,ax,dz,az,bx,bz = 14; inlining az and/or bz = 14;
 * reassigning cx/cz in place = 28; fully inlining every difference = 26/27.
 *
 * Remaining 7 differing insns (tmp/grind/func_8002D780/s3/pairdiff_e2.txt):
 *  A. 2 insns -- the SECOND subu pair of the test-3 block: the target emits
 *     `subu v1,t5,t1` (dx = x2 - x0) before `subu v0,a2,a3` (az = cz - z0); we emit
 *     az first.  Declaring dx ahead of az DOES produce the target order (variants
 *     f1/f3, tmp/grind/func_8002D780/s3/pairdiff_f1.txt) but permutes dz and dx
 *     between $a0 and $v1, costing 7 -- so residual A is now an ALLOCATION question
 *     (dz must land in $a0 and dx in $v1 with dx emitted second), not a scheduling one.
 *  B. 5 insns -- the `dist` copy into $a0 (`move a0,s1` in the beqz delay slot),
 *     feeding the LZCS island input and the `srlv`, while $s1 keeps the compare, the
 *     small-path LUT index and the call arguments.  Fourteen natural copy spellings
 *     are now measured dead across s1/s2/s3; cse.c make_regs_eqv folds every one.
 *
 * The three cop2 blocks are the owner-authorized canonical LZCS/LZCR + mvmva idiom
 * (.claude/rules/cop2-addressing-preamble-cluster.md); the LZCS island's
 * "$12","$13","$14","$15" clobber list is the byte-matched sibling spelling that ships
 * on main for the SAME island in this TU (func_8002BC68 src/code6cac_b.c:762-767,
 * func_8002BEA0 :825-830, judge ruling 2026-07-28 recorded in-line at :751-758). */
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
                s32 t, dz, dx;
                t = cx - x0;
                dz = z2 - z0;
                dx = x2 - x0;
                kc = dz * t;
                t = cz - z0;
                kc -= dx * t;
                t = px - x0;
                kp = dz * t;
                t = pz - z0;
                kp -= dx * t;
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
            s32 m = dist;
            s32 lzcr = 0;
            if (dist >= 0) {
                m = dist;
                __asm__ volatile(
                    "addu $t4, %0, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop"
                    : : "r"(m) : "$12");
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(&sp_var) : "$12", "$13", "$14", "$15");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *((&D_8008D118) + ((u32)m >> shift));
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
