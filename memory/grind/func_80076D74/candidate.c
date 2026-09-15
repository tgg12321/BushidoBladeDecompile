/* func_80076D74 - best form after session 2 (permuter re-run, 2026-09-15): sandbox --disable all = 0 (161 target insns),
 * re-measured THIS session as tmp/grind/func_80076D74/s2/body_p0_record.c.
 *
 * REQUIRES the record-table declaration (per-word-splat -> aggregate merge family, .claude/rules/no-new-park-categories.md:238-262),
 * canonical placement include/game.h, prepared as memory/grind/func_80076D74/record_table_decl.patch:
 *     typedef struct { u8 unk0; u8 unk1; } Unk8009BCF8Record;
 *     extern Unk8009BCF8Record D_8009BCF8[20];
 * together with removal of `extern u8 D_8009BCF8; extern u8 D_8009BCF9;` from src/text1b.c:2227-2228 AND src/text1b_b.c:237-238,
 * and retirement of the `D_8009BCF9 = 0x8009BCF9;` row in undefined_syms_auto.txt:79 (its only referrer is asm/funcs/func_80076D74.s,
 * which goes away when this lands; delete the row in the same commit, or keep it suffixed per the 2026-09-03 amendment).
 * Evidence (evidence.md s2): 0x8009BCF8..0x8009BD1F = 20 two-byte records (D_8009BD20 follows); func_800759D0 takes the table base
 * into $s6 (lui/addiu) and reads it with a shift-1 stride index; func_80075F80 and this function read with shift-1 stride indexes;
 * byte 1 of each record is the identity sequence 0,1,2,3,... The BANNED `extern u8 D_8009BCF8[][2]` + `[idx][1]` pair table is NOT
 * used here; the record-table `.unk1` read produces the same bytes through expr.c get_inner_reference (offset idx*2 in a reg, the
 * .unk1 byte folded into the symbol constant -> lbu %lo(D_8009BCF8+1)($at) == %lo(D_8009BCF9)).
 * The header / text1b_b.c / undefined_syms_auto.txt edits are OUTSIDE the grind surface: a ruling-request was filed (s2) asking whether
 * this declaration clears prongs (a)-(e) and how the out-of-surface edits land. For sandbox MEASUREMENT only, the decl can be placed
 * TU-local via tmp/grind/func_80076D74/s2/decl.py (same codegen; not the proposed final form).
 *
 * Tail (s1 residual 5, epilogue) closed by a single-level do { } while (0) wrap of the final arg0[6] += 0xC statement (sanctioned
 * family, .claude/rules/do-while-zero-exception.md, FAKE-annotated inline; layer-1 PASSED this construct 2026-09-15 12:46).
 * Mechanism: sched.c loop_notes attach NOTE_INSN_LOOP_END to the next insn (the return copy), which then depends on every earlier
 * set/use in the block, so it cannot be hoisted into the lw load-delay slot; the increment temp takes $v0; tail = lw/nop/addiu/sw/move.
 * Ordinary-C alternatives measured dead: u8 ret two-copy chain = 5 (s2); s32 arg0 + cast offsets = 5 (s1); permuter campaigns on the
 * s32-ret (34.7k iters) and u8-ret (43.3k iters) no-FAKE chassis find only do-while(0) forms (s2).
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
                hdr->cells[i][j][0] = D_8009BCF8[*(s16 *)(D_800A36A0 + i * 10 + (j << 1) + 0x6A)].unk1;
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
