/* Steers two bone-angle sets (a, b) toward obj's partner (*(u8 **)obj):
 * while obj+0x6A is 0x15/0x25, a clamped heading (obj+0x1D8 - obj+0x1CA) and
 * a clamped elevation from ratan2(ground distance, height delta) are eased
 * 1/8 of the wrapped difference per call into obj+0x1E6/0x1E8 and applied
 * to both sets via func_8002F770 (otherwise both ease back toward 0; state
 * 0x1F holds elevation at 0x100). The ground distance is the D_8008D118
 * byte-LUT integer sqrt with the GTE leading-zero count for large inputs
 * (same idiom and island as func_8002E838 in code6cac_b.c). Also sets or
 * eases a twist in obj+0x1EA (states 0x1D/0xE, and obj+0xE in 6..7 with
 * obj+0x6A == 2), and adds random jitter to both sets when obj+0x26E is set. */
extern s32 rng_Next(void);
void func_8001F2E4(u8 *obj, u8 *a, u8 *b) {
    s32 sp_tmp;
    s32 sp_tmp2;
    s32 ang_z;
    s32 ang_y;
    s32 ang_x;
    s32 dx;
    s32 dz;
    s32 dist_sq;
    s32 dist;
    s32 d;
    s32 r;
    s32 tgt;
    s32 ang;
    s16 t;

    if (*(s16 *)(obj + 0x26C) == 0) {
        func_80027334((s32 *)a);
        func_80027334((s32 *)b);
    }
    if (*(u16 *)(obj + 0x6A) == 0x15 || *(u16 *)(obj + 0x6A) == 0x25) {
        if (*(s16 *)(obj + 0xC) == 0x1F) {
            ang_y = 0x100;
            ang_x = 0;
            ang_z = 0;
        } else {
            ang_z = (*(s16 *)(obj + 0x1D8) - *(s16 *)(obj + 0x1CA)) & 0xFFF;
            if (ang_z >= 0x800) {
                ang_z -= 0x1000;
            }
            if (ang_z < -0x1FF) {
                ang_z = -0x1FF;
            } else if (ang_z >= 0x200) {
                ang_z = 0x1FF;
            }
            dx = *(s32 *)(*(u8 **)obj + 0x180) - *(s32 *)(obj + 0x180);
            dz = *(s32 *)(*(u8 **)obj + 0x188) - *(s32 *)(obj + 0x188);
            dist_sq = dx * dx + dz * dz;
            if ((u32)dist_sq < 0x400) {
                dist = (u32)*(((u8 *)&D_8008D118) + dist_sq) >> 3;
            } else {
                s32 lzcr = 0;
                if (dist_sq >= 0) {
                    /* Hand-written GTE leading-zero-count block (LZCS in, LZCR out) —
                     * canonical inline asm, identical to the user-authorized block in
                     * the matched sibling func_8001A67C (src/code6cac.c). */
                    __asm__ volatile(
                        "addu   $t4, %1, $zero\n"
                        "mtc2   $t4, $30\n"
                        "nop\n"
                        "nop\n"
                        "addiu  $v0, $sp, 0x10\n"
                        "addu   $t4, $v0, $zero\n"
                        "swc2   $31, 0($t4)\n"
                        : "=m"(sp_tmp)
                        : "r"(dist_sq)
                        : "$2", "$12");
                    lzcr = sp_tmp;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tbl = *(((u8 *)&D_8008D118) + ((u32)dist_sq >> shift));
                    dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
                }
            }
            ang_y = 0x400 - ratan2(dist, *(s32 *)(*(u8 **)obj + 0x184) - *(s32 *)(obj + 0x184));
            if (ang_y < -0xFF) {
                ang_y = -0xFF;
            } else if (ang_y >= 0x100) {
                ang_y = 0xFF;
            }
            ang_x = 0;
        }
    } else {
        ang_x = 0;
        ang_y = 0;
        ang_z = 0;
    }

    d = (ang_z - *(s16 *)(obj + 0x1E6)) & 0xFFF;
    if (d >= 0x800) {
        d -= 0x1000;
    }
    *(s16 *)(obj + 0x1E6) = *(s16 *)(obj + 0x1E6) + d / 8;
    d = (ang_y - *(s16 *)(obj + 0x1E8)) & 0xFFF;
    if (d >= 0x800) {
        d -= 0x1000;
    }
    *(s16 *)(obj + 0x1E8) = *(s16 *)(obj + 0x1E8) + d / 8;
    func_8002F770((s16 *)(a + 0x36), *(s16 *)(obj + 0x1E6), *(s16 *)(obj + 0x1E8), ang_x);
    func_8002F770((s16 *)(b + 0x36), *(s16 *)(obj + 0x1E6), *(s16 *)(obj + 0x1E8), ang_x);

    t = *(s16 *)(obj + 0xC);
    if ((t == 0x1D || t == 0xE) && *(s16 *)(obj + 0x8C) != 0) {
        ang = (ratan2(D_800A387C, *(s32 *)(*(u8 **)obj + 0xF8) - *(s32 *)(obj + 0xF8)) - 0x400) & 0xFFF;
        if (ang >= 0x800) {
            ang -= 0x1000;
        }
        if (ang >= 0x200) {
            ang = 0x1FF;
        } else if (ang < -0x1FF) {
            ang = -0x1FF;
        }
        *(s16 *)(obj + 0x1EA) = ang;
        *(u16 *)(a + 0x7E) += ang;
        *(u16 *)(b + 0x7E) += ang;
    }

    if ((u32)(*(u16 *)(obj + 0xE) - 6) < 2U && *(u16 *)(obj + 0x6A) == 2) {
        if (*(s32 *)(obj + 0x268) == 0) {
            *(s32 *)(obj + 0x25C) = *(s32 *)(obj + 0xF4);
            *(s32 *)(obj + 0x260) = *(s32 *)(obj + 0xF8);
            *(s32 *)(obj + 0x264) = *(s32 *)(obj + 0xFC);
        }
        dx = *(s32 *)(*(u8 **)obj + 0xF4) - *(s32 *)(obj + 0x25C);
        dz = *(s32 *)(*(u8 **)obj + 0xFC) - *(s32 *)(obj + 0x264);
        dist_sq = dx * dx + dz * dz;
        if ((u32)dist_sq < 0x400) {
            dist = (u32)*(((u8 *)&D_8008D118) + dist_sq) >> 3;
        } else {
            s32 lzcr = 0;
            if (dist_sq >= 0) {
                /* Same LZC block; LZCR lands in the second slot, sp+0x14. */
                __asm__ volatile(
                    "addu   $t4, %1, $zero\n"
                    "mtc2   $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addiu  $v0, $sp, 0x14\n"
                    "addu   $t4, $v0, $zero\n"
                    "swc2   $31, 0($t4)\n"
                    : "=m"(sp_tmp2)
                    : "r"(dist_sq)
                    : "$2", "$12");
                lzcr = sp_tmp2;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(((u8 *)&D_8008D118) + ((u32)dist_sq >> shift));
                dist = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }
        tgt = (ratan2(dist, *(s32 *)(*(u8 **)obj + 0xF8) - *(s32 *)(obj + 0x260)) - 0x400) & 0xFFF;
        if (tgt >= 0x800) {
            tgt -= 0x1000;
        }
        if (tgt >= 0x200) {
            tgt = 0x1FF;
        } else if (tgt < -0x1FF) {
            tgt = -0x1FF;
        }
        d = (tgt - *(s16 *)(obj + 0x1EA)) & 0xFFF;
        if (d >= 0x800) {
            d -= 0x1000;
        }
        *(s16 *)(obj + 0x1EA) = *(s16 *)(obj + 0x1EA) + d / 8;
        *(u16 *)(a + 0x72) += *(s16 *)(obj + 0x1EA);
        *(u16 *)(b + 0x72) += *(s16 *)(obj + 0x1EA);
    }

    if (*(s16 *)(obj + 0x26E) != 0 && *(s16 *)(obj + 0x96) == 0) {
        r = (rng_Next() & 0x3F) - 0x20;
        *(u16 *)(a + 0xC) += r;
        *(u16 *)(b + 0xC) += r;
        *(u16 *)(a + 0x14) -= r;
        *(u16 *)(b + 0x14) -= r;
        r = (rng_Next() & 0x3F) - 0x20;
        *(u16 *)(a + 0x1E) += r;
        *(u16 *)(b + 0x1E) += r;
        *(u16 *)(a + 0x26) -= r;
        *(u16 *)(b + 0x26) -= r;
    }
}
