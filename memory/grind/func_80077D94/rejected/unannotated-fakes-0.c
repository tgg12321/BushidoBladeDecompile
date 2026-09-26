/* REJECTED 2026-09-25 by layer-2 cheat-reviewer (FAIL). Bytes were exact
 * (sandbox 0/468, oracle green); the objection was disclosure: abr (constant
 * holder), j and t (named intermediates) and the per-arm colour chains are
 * match-motivated and carried no FAKE annotation / exhaustion receipts, and
 * the ledger was not in the commit. Fixed in the landed body by annotation +
 * evidence.md ablations; the C is otherwise identical. */
void func_80077D94(s32 *arg0) {
    Env77D94 s;
    Rect77D94 rect;
    s16 *in;
    s16 *out;
    s32 table;
    s32 abr;
    s32 *hp;
    s32 i;
    s32 j;
    s32 x;
    s32 v;
    Win77D94 *w;

    s.ot_idx = 2;
    s.y = 0x1E;
    s.semi = 0;
    in = ((Ctx77D94 *)D_800A35F8)->in30;
    table = ((Ctx77D94 *)D_800A35F8)->table;
    out = ((Ctx77D94 *)D_800A35F8)->out34;
    abr = 0x20;
    if (D_800A35F0 < in[D_800A3600] + 60 && D_800A35F0 >= in[D_800A3600]) {
        v = ((D_800A35F0 - in[D_800A3600]) << 7) / 60;
        s.has_color = 1;
        s.col_r = s.col_g = s.col_b = v;
    } else if (D_800A35F0 >= out[D_800A3600] && D_800A35F0 < out[D_800A3600] + 60) {
        v = ((60 - (D_800A35F0 - out[D_800A3600])) << 7) / 60;
        s.has_color = 1;
        s.col_r = s.col_g = s.col_b = v;
    } else {
        s.has_color = 0;
    }

    if (D_800A35F0 < out[5] + 60) {
        switch (D_800A3600) {
        case 0:
            if (D_800A35F0 + 1 >= out[D_800A3600] + 60) {
                if (D_800A35F0 + 1 >= in[D_800A3600 + 1]) {
                    D_800A3600 = D_800A3600 + 1;
                }
            } else if (D_800A35F0 >= in[D_800A3600]) {
                hp = ((Ctx77D94 *)D_800A35F8)->hdr18;
                for (i = 0; i < 5; hp++, i++) {
                    s.header = *hp;
                    s.table = table;
                    s.x = i << 7;
                    s.out = arg0[4];
                    arg0[4] = func_8007352C(&s.header);
                    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
                    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
                    arg0[6] += 0xC;
                }
            }
            break;
        case 2:
            if (D_800A35F0 + 1 >= out[D_800A3600] + 60) {
                if (D_800A35F0 + 1 >= in[D_800A3600 + 1]) {
                    D_800A3600 = D_800A3600 + 1;
                }
                break;
            }
            hp = ((Ctx77D94 *)D_800A35F8)->hdr24;
            for (i = 0, x = 0x156; i < 2; x += 0x80, hp++, i++) {
                s.header = *hp;
                s.x = x;
                s.table = table;
                s.out = arg0[4];
                arg0[4] = func_8007352C(&s.header);
                SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
                AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
                arg0[6] += 0xC;
            }
            /* fall through */
        case 1:
            if (D_800A35F0 + 1 >= in[D_800A3600 + 1] && D_800A3600 == 1) {
                D_800A3600 = 2;
            }
            if (D_800A35F0 < out[2] && D_800A35F0 >= in[2]) {
                s.has_color = 0;
            }
            hp = ((Ctx77D94 *)D_800A35F8)->hdr20;
            for (i = 0, x = 0x2B; i < 2; x += 0x80, hp++, i++) {
                s.header = *hp;
                s.x = x;
                s.table = table;
                s.out = arg0[4];
                arg0[4] = func_8007352C(&s.header);
                SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
                AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
                arg0[6] += 0xC;
            }
            break;
        case 3:
            rect = D_800A32FC;
            for (i = 0; i < 5; i++) {
                j = i * 4 + 0x38;
                LoadImage((s32)&rect, *(s32 *)(D_800A35F8 + j) + 0x220);
                DrawSync(0);
                rect.x += 0x40;
            }
            D_800A3600 = D_800A3600 + 1;
            break;
        case 4:
            if (D_800A35F0 + 1 < out[D_800A3600] + 60) {
                hp = ((Ctx77D94 *)D_800A35F8)->hdr1C;
                for (i = 0; i < 5; hp++, i++) {
                    s.header = *hp;
                    s.table = table;
                    s.x = i << 7;
                    s.out = arg0[4];
                    arg0[4] = func_8007352C(&s.header);
                    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
                    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
                    arg0[6] += 0xC;
                }
            }
            break;
        }
    }

    s.ot_idx = 1;
    s.y = 0;
    s.x = 0;
    hp = ((Ctx77D94 *)D_800A35F8)->hdr28;
    for (i = 0; i < 21; i++) {
        w = (Win77D94 *)(i * 4 + (s32)((Ctx77D94 *)D_800A35F8)->win2C);
        if (D_800A35F0 < w->off + 60 && D_800A35F0 >= w->on) {
            if (D_800A35F0 < w->on + 60) {
                s.has_color = 1;
                v = ((D_800A35F0 - w->on) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else if (D_800A35F0 >= w->off && D_800A35F0 < w->off + 60) {
                s32 t = D_800A35F0 - 60;
                s.has_color = 1;
                v = ((w->off - t) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else {
                s.has_color = 1;
                v = 0x72;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            }
            s.header = hp[i];
            s.table = s.header + 0xC;
            s.out = arg0[4];
            arg0[4] = func_8007352C(&s.header);
            SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
            AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
            arg0[6] += 0xC;
        }
    }
}
