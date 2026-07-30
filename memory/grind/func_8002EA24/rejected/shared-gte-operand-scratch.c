/* REJECTED (session 1) -- shared GTE-operand scratch variable.
 *
 * Hypothesis: target routes every GTE operand through $t4 with a preceding
 * "addu $t4, <src>, $zero", which looks like a single reused C scratch
 * variable feeding all four asm sites. Probe: declare one function-scope
 * scratch (tried both "s32 gp;" and "s32 *gp;") and assign it before each
 * lwc2/swc2/mtc2 block.
 *
 * MEASURED: sandbox --disable all = 24 (both spellings), vs 20 for the
 * per-site local form and 18 for HEAD. Build shrank to 98 insns: GCC still
 * emits NO copy (it computes the address straight into the operand register)
 * and the long cross-block live range additionally perturbed the $a0/$a1/$v0
 * assignment in the comparison chain. The $t4 copy is NOT produced by
 * variable reuse. KILLED.
 */
s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    s32 *gp;
    *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
    *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
    *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
    {
        gp = (s32 *)(obj + 0xF8);
        __asm__ volatile("lwc2 $0, 0(%0)\nlwc2 $1, 4(%0)" : : "r"(gp));
        __asm__ volatile("nop\nnop\n.word 0x4A486012");
        gp = (s32 *)(obj + 0x100);
        __asm__ volatile("swc2 $25, 0(%0)\nswc2 $26, 4(%0)\nswc2 $27, 8(%0)" : : "r"(gp));
    }

    {
        s32 x = *(s32 *)(obj + 0x100);
        s32 z;
        s32 a0_var;
        s32 sp_var;
        s32 min_y;
        s32 max_y;
        s32 y_low;
        s32 y;
        s32 neg_threshold = -threshold;

        if (x < neg_threshold || threshold < x) return 0;
        z = *(s32 *)(obj + 0x104);
        if (z < neg_threshold || threshold < z) return 0;

        a0_var = x * x + z * z;
        if (r_sq < a0_var) return 0;
        a0_var = r_sq - a0_var;

        if ((u32)a0_var < 0x400) {
            a0_var = (u32)*(((u8 *)&D_8008D118) + a0_var) >> 3;
        } else {
            s32 lzcr = 0;
            if (a0_var >= 0) {
                __asm__ volatile("mtc2 %0, $30" : : "r"(a0_var));
                gp = &sp_var;
                __asm__ volatile("nop\nnop\nswc2 $31, 0(%0)" : : "r"(gp));
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(((u8 *)&D_8008D118) + ((u32)a0_var >> shift));
                a0_var = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_y = 0;
        min_y = 0;
        y_low = *(s32 *)(obj + 0xB0);
        if (y_low < 0) {
            min_y = y_low;
        } else {
            max_y = y_low;
        }
        y = *(s32 *)(obj + 0x108);
        if (max_y < y - a0_var) return 0;
        if (y + a0_var < min_y) return 0;
        return 1;
    }
}