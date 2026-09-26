extern s32 func_8006E480(s32, s32);
extern s32 func_8007352C(s32 *);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern void LoadImage(s32, s32);
extern s32 g_gpu_ot_ptr;

typedef struct {
    s16 x, y, w, h;
} Rect77D94;
extern Rect77D94 D_800A32FC;

/* 0x2C-byte draw descriptor func_8007352C consumes (EnvA layout). */
typedef struct {
    s32 header;
    s32 table;
    s32 out;
    s32 pad0C;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 pad20, pad24;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
} Env77D94;

typedef struct {
    s16 on, off;
} Win77D94;

typedef struct {
    u8 pad00[0x14];
    s32 table;
    s32 *hdr18;
    s32 *hdr1C;
    s32 *hdr20;
    s32 *hdr24;
    s32 *hdr28;
    Win77D94 *win2C;
    s16 *in30;
    s16 *out34;
    s32 img38[5];
} Ctx77D94;

void func_80077D94(s32 *arg0) {
    Env77D94 s;
    Rect77D94 rect;
    s16 *in;
    s16 *out;
    s32 table;
    /* FAKE: constant-holder local (named-local-fake-exception). abr is the
       tpage blend-mode bits func_8006E480 adds to every sprite's tpage.
       Mechanism: a pseudo live across the draw calls is seated once in
       callee-saved $s7 (`addiu s7,zero,0x20` in the prologue branch's
       delay slot, then `addu a1,s7,zero` at all 5 call sites). The inline
       literal re-materializes `li a1,0x20` per call: 12/465 vs 0/468.
       Same shape as func_80078654's `zero` and func_80070C70's `c60`.
       Lever exhaustion: memory/grind/func_80077D94/evidence.md. */
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
    /* FAKE (duplicated-statement-into-arms): both fade arms store their own
       has_color/r/g/b; the compiler cross-jumps the identical tails. The
       shared-tail spelling (arms set v only, the else skips the stores with
       a goto) measures 23/470 vs 0/468.
       Lever exhaustion: memory/grind/func_80077D94/evidence.md. */
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
                /* FAKE: named intermediate for the TIM-pointer slot offset
                   (0x38 + 4i). Mechanism: loop.c strength-reduces it to
                   the target's giv ($s1 = 0x38, += 4). Written inline
                   (`((Ctx77D94 *)D_800A35F8)->img38[i]` or
                   `D_800A35F8 + i * 4 + 0x38`), fold moves 0x38 into the
                   load displacement and the giv is not reduced: 9/467.
                   Lever exhaustion: memory/grind/func_80077D94/evidence.md. */
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
            /* FAKE (duplicated-statement-into-arms): each arm stores its own
               r/g/b chain; the compiler cross-jumps the identical tails back
               into one. One shared chain after the if/else puts the value in
               a separate pseudo and costs a `move` at the join: 26/467 vs
               0/468. Lever exhaustion: memory/grind/func_80077D94/evidence.md. */
            if (D_800A35F0 < w->on + 60) {
                s.has_color = 1;
                v = ((D_800A35F0 - w->on) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else if (D_800A35F0 >= w->off && D_800A35F0 < w->off + 60) {
                /* FAKE: named intermediate for the frame 60 ticks back.
                   Mechanism: inline, fold reassociates off - (cnt - 60)
                   into (off + 60) - cnt; the target computes cnt - 60
                   first (`addiu v0,a1,-60`). Inline: 16/469;
                   60 - (cnt - off): 16/469.
                   Lever exhaustion: memory/grind/func_80077D94/evidence.md. */
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
