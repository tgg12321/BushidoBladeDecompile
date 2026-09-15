/* func_80076D74 - session 2 (structural) FINAL form, sandbox --disable all = 0 (161 target insns).
 * REQUIRES the file-scope declaration change in src/text1b.c line 2227:
 *     extern u8 D_8009BCF8;   -->   extern u8 D_8009BCF8[][2];
 * (D_8009BCF8/D_8009BCF9 are one u8 pair table; [x][1] == the target's %lo(D_8009BCF9)(at) byte.)
 * s1 residual (5, epilogue) closed by a single-level do { } while (0) wrap of the final
 * arg0[6] += 0xC statement (sanctioned family, .claude/rules/do-while-zero-exception.md,
 * FAKE-annotated inline). Mechanism: sched.c sched_analyze attaches NOTE_INSN_LOOP_END to the
 * next insn (the return copy) as loop_notes, which makes that insn depend on every earlier
 * set/use in the block; the copy therefore cannot be hoisted into the lw load-delay slot,
 * the increment temp takes $v0, and the tail becomes lw/nop/addiu/sw/move exactly as target.
 * Ordinary-C alternatives measured dead: u8 ret (two-copy chain) = 5; s32 arg0 + cast offsets = 5 (s1).
 * See evidence.md / hypotheses.md.
 */
typedef struct {
    u8 cells[2][5][2];  /* 0x00: [row][col][{glyph, attr}] */
    u32 pad10 : 10;     /* 0x14 */
    u32 f10 : 2;
    u32 f12 : 2;
    u32 f14 : 1;
    u32 f15 : 2;
} S_80076D74;

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
    cnt = (u16 *)(D_800A36A0 + 0x36);
    v = *cnt + 8;
    *cnt = v;
    if (v >= 0xFF) {
        *cnt = 0xFF;
        hdr = *(S_80076D74 **)D_800A36A0;
        hdr->f10 = *(u8 *)(D_800A36A0 + 0x65);
        ret = 1;
        if (*(u8 *)(D_800A36A0 + 0x66) < 3) {
            sel = *(u8 *)(D_800A36A0 + 0x66) - 1;
        } else {
            sel = *(u8 *)(D_800A36A0 + 0x66) - 2;
        }
        hdr->f12 = sel;
        hdr->f14 = *(u8 *)(D_800A36A0 + 0x67);
        hdr->f15 = *(u8 *)(D_800A36A0 + 0x68) + *(u8 *)(D_800A36A0 + 0x69) * 2;
        for (i = 0; i < 2; i++) {
            for (j = 0; j < *(u8 *)(D_800A36A0 + 0x65) + 3; j++) {
                hdr->cells[i][j][0] = D_8009BCF8[*(s16 *)(D_800A36A0 + i * 10 + (j << 1) + 0x6A)][1];
                hdr->cells[i][j][1] = *(u16 *)(D_800A36A0 + i * 10 + (j << 1) + 0x7E);
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
    AddPrim(D_800A374C, (GameObj *)p);
    p += 0x10;
    arg0[5] = (s32)p;
    SetDrawMode(arg0[6], 1, 0, 0x40, 0);
    AddPrim(D_800A374C, (GameObj *)arg0[6]);
    do { /* FAKE: do-while(0) wrap, loop-end note pins the return copy after the sw so the increment temp takes v0; mechanism: sched.c loop_notes dependence on the first insn after NOTE_INSN_LOOP_END; lever-exhaustion: memory/grind/func_80076D74/hypotheses.md s1-s2 */
        arg0[6] += 0xC;
    } while (0);
    return ret;
}
