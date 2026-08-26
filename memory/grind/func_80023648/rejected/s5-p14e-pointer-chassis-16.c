/* REJECTED s5: pointer local `p14e = (s16*)(arg0+0x14E)` carrying all five 0x14E accesses
 * (multi-use, ordinary C, not a dead construct). Floor 16 at 159/159 - one WORSE than the
 * 15-floor candidate. Its permuter base score is 300 vs the candidate chassis's 95, i.e. it
 * is a genuinely distant basin; a 16.8k-iteration campaign from it (tmp/perm_23648_s5a)
 * never got below 265. The pointer chassis is a dead starting point, not a near miss.
 */
void func_80023648(u8 *arg0) {
    u16 kind = *(u16 *)(arg0 + 0x6A);
    s16 *new_var;
    s32 abs_val;
    s32 sub_result;
    s32 div16;
    s16 new_14e;
    s32 tbl_val;
    s32 mult_res;
    s32 limit;
    s32 speed_prod;
    s32 speed;
    s16 sin_val;
    s16 cos_val;
    s16 *p14e;

    if (kind == 0x13 || kind == 0x1B || kind == 0x30) {
        u32 bits = *(u32 *)(arg0 + 0x2C);
        if (bits & 0xF000) {
            s32 a1 = (bits >> 14) & 1;
            s32 a0;
            s32 a2;
            s16 *row;
            s16 *ent;

            if (!(bits & 0x1000)) {
                a1++;
            }
            a0 = (bits >> 15) & 1;
            if (!(bits & 0x2000)) {
                a0++;
            }

            new_var = &D_8008EB40;
            row = new_var + (a0 * 3);
            ent = &row[a1];
            a2 = *ent;

            if (D_800A38BA != 0 && *(s16 *)(arg0 + 6) == 0) {
                func_8001F860((s16 *)arg0, *(s16 *)(arg0 + 0x1CA) + a2 / 4);
            } else {
                func_8001F860((s16 *)arg0, *(s16 *)(arg0 + 0x1D8) + a2);
            }
        } else {
            if (D_800A38BA != 0 && *(s16 *)(arg0 + 6) == 0) {
                *(s16 *)(arg0 + 0x14C) = 0;
            }
        }

        p14e = (s16 *)(arg0 + 0x14E);
        abs_val = *(s16 *)(arg0 + 0x150);
        if (abs_val < 0) {
            abs_val = -abs_val;
        }
        if (abs_val >= 0x401) {
            abs_val = 0x400;
        }

        sub_result = *(u16 *)p14e - abs_val;
        div16 = *(s16 *)(arg0 + 0x1A);

        *p14e = sub_result;
        if (div16 < 0) {
            div16 += 15;
        }
        div16 >>= 4;
        new_14e = sub_result;
        new_14e = new_14e + div16;
        *p14e = new_14e;

        tbl_val = (&D_800A310C)[(&D_8008DA08)[*(s16 *)(arg0 + 0xA)]];
        sub_result = *(s16 *)(arg0 + 0x1A);
        mult_res = sub_result * tbl_val;
        limit = (mult_res << 4) >> 12;

        div16 = (s16)new_14e;
        if (limit < div16) {
            *p14e = limit;
        } else if ((s16)new_14e < 0) {
            *p14e = 0;
        }

        speed_prod = *p14e * *(s16 *)(arg0 + 0x44);
        speed = speed_prod >> 12;
        sin_val = (&Judge)[(*(u16 *)(arg0 + 0x1CA) & 0xFFF)];

        *(s32 *)(arg0 + 0xD8) += (sin_val * speed) >> 16;

        cos_val = (&Judge)[((*(s16 *)(arg0 + 0x1CA) + 0x400) & 0xFFF)];
        *(s32 *)(arg0 + 0xE0) += (cos_val * speed) >> 16;
    } else {
        if (*(s16 *)(arg0 + 0x14E) > 0) {
            if (kind != 0x22) {
                *(s16 *)(arg0 + 0x14C) = 0;
            }
            *(s16 *)(arg0 + 0x14E) = 0;
        }
    }
}
