/* REJECTED (s2, 2026-08-20) — score 25 (73 insns, 4 saves) on the
 * post-migration chassis. Two-carrier attempt: t carries {1, var_s0+1},
 * u carries {0x18, D_80101BCC reload}. The killer is `t = var_s0 + 1;`:
 * reading var_s0 in int context forces a sign-extend BEFORE the increment;
 * CSE reuses the loop-top sign-extend pseudo, which then lives across BOTH
 * calls -> a 4th callee-save + an extra insn. Target increments the RAW
 * register (addiu v0,s0,1) and sign-extends the temp afterwards, so any
 * spelling that reads var_s0 as an int on the increment path is dead.
 * (The related do-while refinement `offset = 1; buf[0] = offset << ...` with
 * `t = var_s0; u = D_80101BCC; while (t < u)` measured 13 — best do-while
 * form found — but the whole do-while family is superseded by the
 * goto-spelled loop in candidate.c, which measures 0.) */
void func_800858D0(void) {
    s32 buf[16];
    s16 var_s0;
    s32 offset;
    s16 ff;
    s32 t;
    s32 u;

    buf[1] = 0x60093;
    var_s0 = 0;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;
    if (D_80101BCC != 0) {
        ff = 0xFF;
        do {
            offset = (s16)var_s0 * 54;
            u = 0x18;
            *(s16 *)((u8 *)&D_800F4E1A + offset) = u;
            *(s16 *)((u8 *)&D_800F4E1E + offset) = 0;
            *(s16 *)((u8 *)D_800F4E28 + offset) = ff;
            *(s16 *)((u8 *)&D_800F4E2A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2E + offset) = ff;
            t = 1;
            buf[0] = t << (s16)var_s0;
            func_8008B488(buf);
            D_8010280A = var_s0;
            func_800871D4(1);
            t = var_s0 + 1;
            var_s0 = t;
            u = D_80101BCC;
        } while ((s16)t < u);
    }
}
