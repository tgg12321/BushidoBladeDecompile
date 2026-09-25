typedef struct { s32 x, y, z; } Vec3i;

/* Sweeps a character's weapon segment against its limb spheres. The scratch
 * record at 0x1F8002B8 (`scr`, the one func_8002E838 / func_8002EA24 /
 * func_8002CA8C use) gets its two segment-end pointers (+0x60 -> scr+0x0,
 * +0x64 -> scr+0xC) and a copy of the tip at +0xC8; d = tip - base. The yaw
 * difference to the partner (partner+0x1D8) is folded into 0..0x800. The
 * horizontal length sqrt(dx^2 + dz^2 - dy^2) (a negative square prints
 * "ILLEGAL GUN MOTION" and returns) gives the pitch at +0xF8 and ratan2(dx,
 * dz) the yaw at +0xFA; unless `quiet`, effect 0xB is played at +0xC8. The
 * segment is then normalised to 1/4 length steps (q = (d << 12) / |d|) and
 * the tip pushed out by q*4 after the base takes the old tip. If the yaw
 * difference is under 0x400, func_8002E838 sets up the segment frame and each
 * of the 22 hit records of character `id` (D_800F5F68, 0x14 bytes each; 6..9
 * skipped unless obj+0x26C) is tested against its scratch point 0x1F8000A8 +
 * id*0x108 + i*0xC: a hit sets bit i in *hit, a second (inner) hit in *deep.
 * Finally the base is pulled back by q/4 into +0xA8, func_80053614 casts the
 * segment against the stage (hit point +0x100); on a stage hit that is not
 * material 7, the limb masks are cleared when the stage point is nearer the
 * base than obj+0xF4, and unless `quiet` effect 0xA plays at the hit point.
 * D_800A37E8.. receive -q.
 *
 * Both square roots are the D_8008D118 byte-LUT integer sqrt with the GTE
 * leading-zero count for inputs >= 0x400.
 *
 * GTE ISLANDS: census member of the 2026-08-17 owner cluster ruling
 * (.claude/rules/cop2-addressing-preamble-cluster.md:69). One gte_Lzc
 * (gtemac.h:230-236) per square root = gte_ldlzc (inline_o.h:645) + the two
 * gte_nop (:3068) in one statement, gte_stlzc (:2999) in the other; nothing
 * but macro text, every operand left to cc1 through %0 (the target's
 * `addu $t4,<reg>,$zero` is the macros' `move $12,%0`, and `addiu $v0,$sp,N`
 * is cc1 materializing &sp_tmp / &sp_tmp2). Same spelling and clobbers as the
 * func_8002CD58 gte_Lzc islands (this file). Everything else is ordinary C. */
extern char D_80010478[];
extern void printf();
extern u8 D_800F5F68[];
extern void func_8002E838(u8 *obj);
extern s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq);
extern s32 func_80053614(s32 *, s32 *, s32 *, s32 *, s32 *);
extern s32 func_80054434(void);
void func_8002A458(u8 *obj, s32 *hit, s32 *deep, s32 quiet) {
    u8 *scr = (u8 *)0x1F8002B8;
    s32 id = *(s16 *)(obj + 4);
    u8 *partner = *(u8 **)obj;
    s32 work[64];
    s32 sp_tmp;
    s32 sp_tmp2;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 diff;
    s32 lzc_in;
    s32 len_sq;
    s32 hit_sq;
    s32 len;
    s32 qx;
    s32 qy;
    s32 qz;
    s32 *p;
    u8 *rec;
    s32 i;

    *(u8 **)(scr + 0x60) = scr;
    *(u8 **)(scr + 0x64) = scr + 0xC;
    *(Vec3i *)(scr + 0xC8) = *(Vec3i *)(scr + 0xC);
    dx = (*(s32 **)(scr + 0x64))[0] - (*(s32 **)(scr + 0x60))[0];
    dy = (*(s32 **)(scr + 0x64))[1] - (*(s32 **)(scr + 0x60))[1];
    dz = (*(s32 **)(scr + 0x64))[2] - (*(s32 **)(scr + 0x60))[2];
    diff = (*(s16 *)(partner + 0x1D8) - ratan2(dx, dz)) & 0xFFF;
    if (diff >= 0x800) {
        diff = 0x1000 - diff;
    }
    /* FAKE: `len` holds the squared horizontal length and is then replaced
     * by its square root (the in-place `len = sqrt(len)` shape); `lzc_in`
     * is the copy handed to gte_ldlzc, and the LZC arm stages the table byte
     * in it (staged-value-reused-variable: its LZC-input value is dead after
     * the island). Effect: the squared length stays in $a1 for printf / the
     * test / the shift while the copy lands in $a0, as in the target
     * (move $a0,$a1 in the beqz slot). A separate squared-length local, or a
     * separate table-byte local, leaves the copy in $v1
     * (memory/grind/func_8002A458/hypotheses.md rows B2/B3). */
    len = dx * dx + dz * dz - dy * dy;
    if (len < 0) {
        printf(D_80010478, len);
        return;
    }
    lzc_in = len;
    if ((u32)len < 0x400) {
        len = (u32)*(((u8 *)&D_8008D118) + len) >> 3;
    } else {
        /* gte_Lzc(lzc_in, &sp_tmp) -- gtemac.h:230-236: gte_ldlzc
         * (inline_o.h:645) + the two gte_nop (:3068), then gte_stlzc
         * (:2999) into the LZCR slot (sp+0x118 in the target). */
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $30\n"
            "nop\n"
            "nop\n"
            :: "r"(lzc_in) : "$12");
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $31, 0($12)\n"
            :: "r"(&sp_tmp) : "$12", "memory");
        {
            s32 lz = ~1;
            s32 shift;
            lz &= sp_tmp;
            shift = 0x16 - lz;
            lzc_in = *(((u8 *)&D_8008D118) + ((u32)len >> shift));
            len = (u32)(lzc_in << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    *(s16 *)(scr + 0xF8) = -ratan2(dy, len);
    *(s16 *)(scr + 0xFA) = ratan2(dx, dz);
    *(s16 *)(scr + 0xFC) = 0;
    if (quiet == 0) {
        func_80032854(id == 0, 0xB, scr + 0xC8, (s16 *)(scr + 0xF8));
    }
    len_sq = dx * dx + dy * dy + dz * dz;
    if ((u32)len_sq < 0x400) {
        len = (u32)*(((u8 *)&D_8008D118) + len_sq) >> 3;
    } else {
        /* gte_Lzc(len_sq, &sp_tmp2) -- gte_ldlzc (inline_o.h:645) +
         * 2x gte_nop (:3068), gte_stlzc (:2999); slot sp+0x11C. */
        __asm__ volatile(
            "move   $12, %0\n"
            "mtc2   $12, $30\n"
            "nop\n"
            "nop\n"
            :: "r"(len_sq) : "$12");
        __asm__ volatile(
            "move   $12, %0\n"
            "swc2   $31, 0($12)\n"
            :: "r"(&sp_tmp2) : "$12", "memory");
        {
            s32 lz = ~1;
            s32 shift;
            s32 tbl;
            lz &= sp_tmp2;
            shift = 0x16 - lz;
            tbl = *(((u8 *)&D_8008D118) + ((u32)len_sq >> shift));
            len = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
        }
    }
    qx = (dx << 12) / len;
    qy = (dy << 12) / len;
    qz = (dz << 12) / len;
    **(Vec3i **)(scr + 0x60) = **(Vec3i **)(scr + 0x64);
    (*(s32 **)(scr + 0x64))[0] += qx * 4;
    (*(s32 **)(scr + 0x64))[1] += qy * 4;
    (*(s32 **)(scr + 0x64))[2] += qz * 4;
    if (diff < 0x400) {
        func_8002E838(scr);
        /* FAKE: do-while(0) (do-while-zero-exception), observed effect: the
         * loop-note ref weighting on rec's set and the id * 0x1B8 multiply
         * seats rec in $s4 / scr in $s5 and id in $s7 / obj in $fp, as in
         * the target. Unwrapped: the four seats swap pairwise. */
        do {
            rec = &D_800F5F68[id * 0x1B8];
        } while (0);
        for (i = 0; i < 22; i++, rec += 0x14) {
            s32 *pos;
            if (*(s16 *)(obj + 0x26C) == 0 && i >= 6 && i <= 9) {
                continue;
            }
            pos = (s32 *)((u8 *)0x1F8000A8 + id * 0x108 + i * 0xC);
            if (func_8002EA24(scr, pos, *(u16 *)(rec + 0xC), *(u16 *)(rec + 0xE)) != 0) {
                s32 bit = 1 << i;
                *hit |= bit;
                if (*(s16 *)rec != 0
                    && func_8002EA24(scr, pos, *(u16 *)(rec + 0x10), *(u16 *)(rec + 0x12)) != 0) {
                    *deep |= bit;
                }
            }
        }
    }
    *(s32 *)(scr + 0xA8) = (*(s32 **)(scr + 0x60))[0] - qx / 4;
    *(s32 *)(scr + 0xAC) = (*(s32 **)(scr + 0x60))[1] - qy / 4;
    *(s32 *)(scr + 0xB0) = (*(s32 **)(scr + 0x60))[2] - qz / 4;
    if (func_80053614((s32 *)(scr + 0xA8), *(s32 **)(scr + 0x64), (s32 *)(scr + 0x100),
                      (s32 *)(scr + 0xF8), work) != 0
        && func_80054434() != 7) {
        if (*hit != 0) {
            p = *(s32 **)(scr + 0x60);
            /* FAKE: variable reuse -- the two squared distances (stage hit
             * point and obj+0xF4, each from the base) are formed in dx/dy/dz,
             * the segment delta, whose values are dead here. Effect: the
             * differences sit in $s2/$s3/$s1 as in the target; fresh locals,
             * or a fresh trio reused for both, land in $v0 and shift the
             * product registers and the spill reload register. */
            dx = *(s32 *)(scr + 0x100) - p[0];
            dy = *(s32 *)(scr + 0x104) - p[1];
            dz = *(s32 *)(scr + 0x108) - p[2];
            hit_sq = dx * dx + dy * dy + dz * dz;
            dx = *(s32 *)(obj + 0xF4) - p[0];
            dy = *(s32 *)(obj + 0xF8) - p[1];
            dz = *(s32 *)(obj + 0xFC) - p[2];
            if (hit_sq >= dx * dx + dy * dy + dz * dz) {
                goto done;
            }
            *deep = 0;
            *hit = 0;
        }
        if (quiet == 0) {
            func_80032854(id == 0, 0xA, scr + 0x100, 0);
        }
    }
done:
    D_800A37E8 = -qx;
    D_800A37EA = -qy;
    D_800A37EC = -qz;
}
