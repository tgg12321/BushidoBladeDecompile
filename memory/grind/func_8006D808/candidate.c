void func_8006D808(s32 *arg0, s32 *arg1, s32 *arg2, s32 arg3, s32 arg4) {
    EnvA s;
    /* FAKE: oversized digit array (dead-vars-local-array.md OVERSIZED-LOCALS
       carve-out, owner ruling 2026-07-13) - only d[0]/d[1] are used; d[2..] is
       the unwritten tail of a LIVE object.  Frame-math proof from the TARGET
       BYTES ALONE: frame 0x88; ten callee-saves ($s0-$s7,$fp,$ra at
       sp+0x60..0x87 => 40 bytes) and a 0x18 outgoing-args area (the 5-arg
       SetDrawMode stores its 5th arg at sp+0x10), so the locals region is
       0x88 - 0x28 - 0x18 = 72 bytes.  The only bytes it ever touches are the
       0x2C EnvA descriptor (sp+0x18..0x43), d[0..1] (sp+0x48..0x4B) and the
       reload spill of the hoisted `0 < n` inner-loop guard (sp+0x58);
       sp+0x4C..0x57 is never read, written or addressed in
       asm/funcs/func_8006D808.s.  The fully-written form (s16 d[2]) measures
       vars= 64 => 0x80 != 0x88, so no fully-written locals set gives the
       target frame.  n.b.! d's slot is 8-aligned (stmt.c:3419 clamps a BLKmode
       automatic to BIGGEST_ALIGNMENT), so the declared length is recoverable
       only as a RANGE: s16 d[5]..d[8] (10..16 bytes) are byte-identical
       (d[5] and d[8] both sandbox 0); d[4] and below give vars= 64.  d[5] is
       the smallest member.  The live-object choice follows the family model:
       EnvA + a separate 8-aligned s16 array at sp+0x48, as in func_8006D3DC
       (u16 rect[4]); same carve-out as the caller func_8006DD94.
       Lever-exhaustion: memory/grind/func_8006D808/hypotheses.md. */
    s16 d[5];
    s16 i;
    s16 k;
    s16 v;
    s16 idx;
    s16 n;
    s32 p;
    s32 w;

    s.ot_idx = arg3;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.has_color = 0;
    s.col_r = s.col_g = s.col_b = 0xA0;
    s.header = (s32 *)arg2[2];
    for (i = 0; i < 3; i++) {
        s.header = (s32 *)arg2[i];
        s.table = (s8 *)((s32)s.header + 0xC);
        s.out = *arg0;
        *arg0 = func_8007352C((s32)&s);
    }
    s.header = (s32 *)arg2[0];
    SetDrawMode(*arg1, 1, 0, func_8006E480((s32)s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3 * 4, *arg1);
    *arg1 += 0xC;

    n = 4;
    if (arg4 == -1) {
        n = 3;
    }
    for (i = 0; i < n; i++) {
        idx = *(u8 *)(D_800A3524 + (i << 2) + 0x24);
        if (idx >= 12 && idx < 22) {
            idx -= 2;
        }
        s.header = (s32 *)arg2[3];
        s.table = (s8 *)(arg2[4] + 24 + idx * 24);
        w = ((s16 *)arg2[7])[idx];
        s.y = i * 26;
        s.x = w;
        if (i == 3) {
            s.x = w - 50;
            s.y = 98;
        }
        s.out = *arg0;
        *arg0 = func_8007352C((s32)&s);
        if (idx == 8) {
            s.table = (s8 *)arg2[4];
            s.out = *arg0;
            *arg0 = func_8007352C((s32)&s);
        }
    }

    s.header = (s32 *)arg2[3];
    SetDrawMode(*arg1, 1, 0, func_8006E480((s32)s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3 * 4, *arg1);
    *arg1 += 0xC;

    for (i = 0; i < 3; i++) {
        for (k = 0; k < n; k++) {
            if ((D_800A36AC & 1) && ((k == arg4 && k < 3) || k == 3)) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            switch (i) {
            case 0:
                d[0] = d[1] = *(u8 *)(D_800A3524 + (k << 2) + 0x21);
                break;
            case 1:
                d[0] = d[1] = *(u8 *)(D_800A3524 + (k << 2) + 0x22);
                break;
            case 2:
                d[0] = d[1] = *(u8 *)(D_800A3524 + (k << 2) + 0x23);
                break;
            }
            v = d[1] / 10;
            d[0] = d[0] % 10;
            d[1] = v % 10;
            p = arg2[5];
            s.header = (s32 *)p;
            *(s16 *)(p + 8) = d[0] * 24;
            s.table = (s8 *)arg2[6];
            s.x = i * 58 + 24;
            s.y = k * 26 + 2;
            if (k == 3) {
                s.x = i * 58 + 3;
                s.y = 100;
            }
            s.out = *arg0;
            *arg0 = func_8007352C((s32)&s);
            *(s16 *)((s32)s.header + 8) = d[1] * 24;
            s.x -= 24;
            s.out = *arg0;
            *arg0 = func_8007352C((s32)&s);
        }
    }

    s.header = (s32 *)arg2[5];
    SetDrawMode(*arg1, 1, 0, func_8006E480((s32)s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3 * 4, *arg1);
    *arg1 += 0xC;
}
