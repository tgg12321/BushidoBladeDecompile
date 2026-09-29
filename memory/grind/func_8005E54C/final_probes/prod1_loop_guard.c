s32 func_8005E54C(u32 arg0, s32 arg1, s32 arg2) {
    /* The per-player points pair: each round's points in the round rows,
       then the per-player totals under them. The target addresses both
       through the one frame slot sp+0x18 (a separate totals array measured
       13-204: memory/grind/func_8005E54C/evidence.md [s4 cont.]). */
    s16 points[2];
    s16 wins[2];
    Env5E54C s;
    T5E098 *tile;
    s32 cur;
    s32 ft4;
    s32 mode_off;
    s32 end_off;
    /* i counts the players (first loop) and then the rounds; j is the
       player and k the mark; each phase restarts them as plain loop indices,
       the counter reuse of func_8005E098 / func_8005F1C8. Separate counters
       per phase measured 8-77 (memory/grind/func_8005E54C/evidence.md [s3]). */
    s16 i;
    s16 j;
    s16 k;
    s16 c;
    s16 y;

    tile = (T5E098 *)arg1;
    s.has_color = 0;
    s.semi = 0;
    s.y = 0;
    cur = arg1 + 0xA0;
    ft4 = arg1 + 0x898;
    mode_off = arg1 + 0xBB8;
    end_off = arg1 + 0xBC4;
    s.ot_idx = arg2;
    for (i = 0; i < 2; i++) {
        if (!(D_8009BD38.unk15 >> i & 1)) {
            s.header = &D_8009B524;
        } else {
            s.header = &D_8009B53C;
        }
        s.x = i * 320;
        s.table = D_8009B554;
        s.sprt_out = cur;
        cur = func_8007352C((s32)&s);
        if (!(D_8009BD38.unk15 >> i & 1)) {
            s.header = &D_8009B530;
            s.table = D_8009B56C;
        } else {
            s.header = &D_8009B548;
            s.table = D_8009B57C;
        }
        s.sprt_out = cur;
        cur = func_8007352C((s32)&s);
    }

    s.semi = 0;
    s.has_color = 0;
    for (i = 0; i < D_8009BD38.unk10 + 3; i++) {
        points[0] = (arg0 >> (i * 4)) & 3;
        points[1] = (arg0 >> (i * 4 + 2)) & 3;
        if (D_8009BD38.unk10 == 2) {
            s.y = i * 24 + 0x44;
        } else if (D_8009BD38.unk10 == 1) {
            s.y = i * 24 + 0x4F;
        } else {
            s.y = i * 34 + 0x4F;
        }
        if (points[0] == 3 || points[1] == 3) {
            s.header = &D_8009B4B0;
            s.x = 0;
            s.y += 2;
            s.table = &D_8009B4BC[D_800A3270[i]];
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
        } else {
            s.header = &D_8009B4E4;
            s.x = 0;
            s.table = &D_8009B514;
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
            for (j = 0; j < 2; j++) {
                s.x = j * 70;
                if (points[j] > *(j ? &points[0] : &points[1])) {
                    s.table = &D_8009B4FC;
                } else if (points[j] < *(j ? &points[0] : &points[1])) {
                    s.table = &D_8009B504;
                } else {
                    s.table = &D_8009B50C;
                }
                s.sprt_out = cur;
                cur = func_8007352C((s32)&s);
            }
            s.y += 5;
            for (j = 0; j < 2; j++) {
                s.header = &D_8009B4F0;
                s.table = &D_8009B51C;
                for (k = 0; k < points[j]; k++) {
                    if (j) {
                        s.x = k * 16 + 0x179;
                    } else {
                        s.x = (1 - k) * 16 + 0xE2;
                    }
                    s.sprt_out = cur;
                    cur = func_8007352C((s32)&s);
                }
            }
        }
    }

    s.header = &D_8009B4E4;
    s.x = 0;
    if (D_8009BD38.unk10 == 2) {
        y = 0xC6;
    } else if (D_8009BD38.unk10 == 1) {
        y = 0xC2;
    } else {
        y = 0xBE;
    }
    s.y = y + 3;
    s.table = &D_8009B514;
    s.sprt_out = cur;
    cur = func_8007352C((s32)&s);
    /* One 32-bit store clears the whole pair (target 0x8005EA44
       `sw $zero,0x18($sp)`); the union spelling measured 197
       (memory/grind/func_8005E54C/evidence.md [s5]). Owner ruling
       2026-09-29 Q36 (no-new-park-categories.md, one cast store on a
       local array). */
    *(s32 *)points = 0;
    for (i = 0; i < D_8009BD38.unk10 + 3; i++) {
        if (((arg0 >> (i * 4)) & 3) != 3) {
            points[0] += (arg0 >> (i * 4)) & 3;
        }
        if (((arg0 >> (i * 4 + 2)) & 3) != 3) {
            points[1] += (arg0 >> (i * 4 + 2)) & 3;
        }
    }
    for (j = 0; j < 2; j++) {
        s.header = &D_8009B4F0;
        s.table = &D_8009B51C;
        k = 0;
        if (k < points[j]) do {
            if (j) {
                s.x = (k >> 1) * 20 + 0x181;
            } else {
                s.x = 0xF2 - (k >> 1) * 20;
            }
            s.y = y + (k & 1) * 12;
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
            k++;
        } while (k < points[j]);
    }
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009B524, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, mode_off);
    mode_off += 0xC;

    s.ot_idx = arg2;
    wins[0] = wins[1] = 0;
    for (i = 0; i < D_8009BD38.unk10 + 3; i++) {
        s.header = &D_8009ADB4;
        s.semi = 0;
        points[0] = (arg0 >> (i * 4)) & 3;
        points[1] = (arg0 >> (i * 4 + 2)) & 3;
        s.col_r = s.col_g = s.col_b = 0x40;
        for (j = 0; j < 2; j++) {
            if (points[j] <= *(j ? &points[0] : &points[1])) {
                if (points[j] != 3) {
                    s.has_color = 1;
                } else {
                    s.has_color = 0;
                }
            } else {
                if (points[j] != 3) {
                    wins[j]++;
                }
                s.has_color = 0;
            }
            c = D_8009BD24[j][i].chr;
            if (c >= 12) {
                c -= 2;
            }
            s.table = UesrWorkDef[c];
            s.x = j * 320 + D_8009B58C[c];
            if (D_8009BD38.unk10 == 2) {
                s.y = i * 24 - 8;
            } else if (D_8009BD38.unk10 == 1) {
                s.y = i * 24 + 3;
            } else {
                s.y = i * 34 + 3;
            }
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
            if (D_8009BD24[j][i].chr == 8) {
                s.table = D_8009ADC0;
                s.sprt_out = cur;
                cur = func_8007352C((s32)&s);
            }
        }
        s.has_color = 0;
        if (points[0] == 3) {
            s.y += 0x4C;
            s.scale_x = 0x100;
            s.x = 0;
            s.semi = 0;
            s.scale_y = 0x400;
            s.y += 6;
            for (j = 0; j < 2; j++) {
                s.header = &D_8009B398[j + 2];
                s.table = D_8009B490[j];
                s.ft4_out = ft4;
                ft4 = func_80073728((s32)&s, 0);
                s.table = &D_8009B490[j][1];
                s.ft4_out = ft4;
                ft4 = func_80073728((s32)&s, 0);
            }
        }
    }

    s.header = &D_8009B398[0];
    s.semi = 0;
    if (D_8009BD38.unk10 == 2) {
        s.y = 0xC9;
    } else if (D_8009BD38.unk10 == 1) {
        s.y = 0xC5;
    } else {
        s.y = 0xC1;
    }
    for (j = 0; j < 2; j++) {
        s.x = j * 70 + 0x113;
        if (wins[j] == 1) {
            s.x += 3;
        }
        s.table = &D_8009B400[wins[j]];
        s.table->unk0 = s.table->unk2 = 0;
        s.sprt_out = cur;
        cur = func_8007352C((s32)&s);
    }

    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    tile->x0 = 8;
    tile->y0 = 0x3A;
    tile->w = 0xDC;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, (s32)tile);
    tile++;
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    tile->x0 = 0x19D;
    tile->y0 = 0x3A;
    tile->w = 0xDC;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, (s32)tile);
    tile++;
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    /* Each arm sets the whole (x0, y0) position: the target stores x0 once
       per arm (0x8005F0E0, 0x8005F0F8, 0x8005F104); one x0 store above the
       if/else measured 10 (memory/grind/func_8005E54C/probes/x0h.c). */
    if (D_8009BD38.unk10 == 2) {
        tile->x0 = 0x5E;
        tile->y0 = 0xC1;
    } else if (D_8009BD38.unk10 == 1) {
        tile->x0 = 0x5E;
        tile->y0 = 0xBD;
    } else {
        tile->x0 = 0x5E;
        tile->y0 = 0xB9;
    }
    tile->w = 0x1C5;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, (s32)tile);
    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009ADB4, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2 * 4, mode_off);
    return end_off - arg1;
}
