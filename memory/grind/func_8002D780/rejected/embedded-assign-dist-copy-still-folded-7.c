/* REJECTED [s4, permuter modality] - score 7 = control, 202 insns (no copy insn
 * materialised).  The embedded-assignment copy spelling `if ((u32)(m = dist) < 0x400)`
 * with `m` carrying the small-path LUT index, the srlv index and all three
 * func_8002D518 call arguments.  This is the spelling decomp-permuter itself
 * converged on (workspace tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780,
 * output-110-1) after 55,531 iterations, and it is the FIRST copy spelling in this
 * ledger that is not a plain `s32 c = dist;` declaration - so it was worth measuring
 * on the real chassis.  cse.c make_regs_eqv folds it exactly like the other twenty-two:
 * build_insns stays at 202, so no `move` survives to be allocated a second seat.
 *
 * Nine arrangements of this spelling were measured in s4, all 7 = control except the
 * one that is semantically wrong: m carrying (asm input + srlv), (asm input),
 * (srlv), (asm + srlv + small-path LUT), (asm + srlv, assigned in the `dist >= 0`
 * condition), (calls only), (small + srlv + calls), (small + calls) - all 7 at 202.
 * Assigning m inside the else arm's `dist >= 0` condition while using it on the
 * small path leaves m undefined there: 57 at 203 insns, not a codegen result.
 *
 * Residual B therefore remains open, and the copy-materialisation question is NOT a
 * source-spelling question: twenty-three spellings across s1-s4 all die in cse. */
s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    s32 m;
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

        if ((u32)(m = dist) < 0x400) {
            sqrt_val = (u32)*((&D_8008D118) + m) >> 3;
        } else {
            s32 lzcr = 0;
            if (dist >= 0) {
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu $t4, %2, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(dist), "r"(&sp_var) : "$12", "$13", "$14", "$15");
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
        if (func_8002D518(sqrt_val, m, p118, p124) != 0) return 1;

        p10C = (s32 *)(obj + 0x10C);
        p10C[0] = -*(s32 *)(obj + 0x100);
        p10C[1] = -*(s32 *)(obj + 0x104);
        if (func_8002D518(sqrt_val, m, p10C, p118) != 0) return 1;

        if (func_8002D518(sqrt_val, m, p10C, p124) != 0) return 1;
        return 0;
    }
}
