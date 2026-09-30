s32 func_80076D74(s32 *arg0) {
    u8 *p;
    S_80076D74 *hdr;
    u16 *cnt;
    s16 v;
    s16 i;
    s16 j;
    s32 sel;
    s32 ret;

    ret = 0;
    cnt = &SELWORK->f36;
    v = *cnt + 8;
    *cnt = v;
    if (v >= 0xFF) {
        *cnt = 0xFF;
        hdr = SELWORK->f00;
        hdr->f10 = SELWORK->f65;
        ret = 1;
        if (SELWORK->f66 < 3) {
            sel = SELWORK->f66 - 1;
        } else {
            sel = SELWORK->f66 - 2;
        }
        hdr->f12 = sel;
        hdr->f14 = SELWORK->f67;
        hdr->f15 = SELWORK->f68[0] + SELWORK->f68[1] * 2;
        for (i = 0; i < 2; i++) {
            for (j = 0; j < SELWORK->f65 + 3; j++) {
                hdr->cells[i][j][0] = D_8009BCF8[SELWORK->f6A[i][j]].unk1;
                hdr->cells[i][j][1] = SELWORK->f7E[i][j];
            }
        }
    }
    p = (u8 *)arg0[5];
    SetTile((GameObj *)p);
    *(u8 *)(p + 4) = *cnt;
    *(u8 *)(p + 5) = *cnt;
    *(u8 *)(p + 6) = *cnt;
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0xC) = 0x280;
    *(s16 *)(p + 0xE) = 0xF0;
    SetSemiTrans((GameObj *)p, 1);
    AddPrim(g_gpu_ot_ptr, (GameObj *)p);
    p += 0x10;
    arg0[5] = (s32)p;
    SetDrawMode(arg0[6], 1, 0, 0x40, 0);
    AddPrim(g_gpu_ot_ptr, (GameObj *)arg0[6]);
    do { /* FAKE: do-while(0) wrap, loop-end note pins the return copy after the sw so the increment temp takes v0; mechanism: sched.c loop_notes dependence on the first insn after NOTE_INSN_LOOP_END; lever-exhaustion: memory/grind/func_80076D74/hypotheses.md s1-s2 */
        arg0[6] += 0xC;
    } while (0);
    return ret;
}
