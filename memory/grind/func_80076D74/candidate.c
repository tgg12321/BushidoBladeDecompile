/* func_80076D74 - Judge-CLEARED body (decisions.md 2026-09-15 13:47 ruling PASS, review-ledger hash ffd478b35c7a6afd). Submit VERBATIM.
 * s3 (permuter, 2026-09-15): re-measured sandbox --disable all = 0 on HEAD, and BYTES PROVEN ON MAIN - a full clean link with this body
 * in src/text1b.c (record-table declaration TU-local at text1b.c:2227-2235 for the measurement) gives SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * == oracle (tmp/grind/func_80076D74/s3/fullbuild.log). Declaration placement is byte-neutral.
 *
 * FINAL FORM per the 13:47 Judge ruling (operator-applied, outside the grind surface; see docs/grind/decisions.md s3 INTEGRATION HANDOFF entry):
 *   include/game.h (after the Unk800F0EC8Record decl):  typedef struct { u8 unk0; u8 unk1; } Unk8009BCF8Record;  extern Unk8009BCF8Record D_8009BCF8[20];
 *   src/text1b.c:2227-2228 and src/text1b_b.c:237-238: delete `extern u8 D_8009BCF8;` / `extern u8 D_8009BCF9;`
 *   undefined_syms_auto.txt:79: DELETE the `D_8009BCF9 = 0x8009BCF9;` row (not suffix); keep line 1252 `D_8009BCF8 = 0x8009BCF8;`
 *   memory/grind/func_80076D74/record_table_decl.patch carries all three hunks (game.h add, text1b_b.c delete, undefined_syms_auto.txt:79 DELETE) - corrected per the ruling.
 * s2-rerun (2026-09-15, driver session 2, third dispatch): bytes RE-PROVEN from clean HEAD 493ad9e97 - sandbox 0 at 161/161 (tmp/grind/func_80076D74/s2/handoff/
 * candidate_head.sandbox.txt) and full tmp-only link SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (s2/handoff/fullbuild.log). Filed as
 * OWNER-ESCALATION - INTEGRATION HANDOFF in docs/grind/decisions.md (scope grant: include/game.h src/text1b_b.c undefined_syms_auto.txt).
 * Object-model evidence (Judge-verified): asm/data/7D920.data.s:23762-23808 = 0x28 bytes = 20 two-byte records; func_800759D0.s:107-108 base in $s6,
 * :142-144 column 0 at 2-byte stride; func_80075F80.s:165-167 column 0 at 2-byte stride; func_80076D74.s:91-93 column 1 at 2-byte stride.
 * The BANNED `extern u8 D_8009BCF8[][2]` pair table is NOT used; the S_80076D74 typedef is emitted exactly ONCE (apply with tmp/grind/func_80076D74/s2/apply2.py).
 *
 * Tail (s1 residual 5, epilogue) closed by a single-level do { } while (0) wrap of the final arg0[6] += 0xC statement (sanctioned family,
 * .claude/rules/do-while-zero-exception.md, FAKE-annotated inline; layer-1 PASSED 2026-09-15 12:46, Judge PASS 13:47).
 * Mechanism: sched.c loop_notes attach NOTE_INSN_LOOP_END to the next insn (the return copy), which then depends on every earlier
 * set/use in the block, so it cannot be hoisted into the lw load-delay slot; the increment temp takes $v0; tail = lw/nop/addiu/sw/move.
 * Lever exhaustion (ordinary C, all 5 unless noted): u8 ret two-copy chain (s2); s32 arg0 + cast offsets (s1); slot pointer `s32 *dm` (s3);
 * packet-pointer round trip (s3, 21); branch-on-ret return (s3); permuter campaigns on s32-ret (34.7k), u8-ret (43.3k) and branch-on-ret
 * (31.6k) no-FAKE chassis find only do-while(0) forms (s2, s3). FAKE ablation this session (wrap removed, nothing else) = 5.
 * s3 LANDING (2026-09-15, driver session 3): scope grant in force (tools/grinder/scope_allow.txt:79); record_table_decl.patch applied to HEAD b67c82f88, text1b.c:2227-2228
 * scalar externs deleted, this body applied via s2/apply2.py; tmp full link (text1b.o + text1b_b.o rebuilt) SHA1 == oracle (s3/land/fullbuild.log); verify-oracle --rebuild
 * --allow-dirty green; sandbox 0 at 161/161. NOTE: against the stale INCLUDE_ASM reference the sandbox prints a FALSE 2 (engine/score.py cannot resolve the
 * deleted D_8009BCF9 row on the reference side; s3/land/rescore.py proves seeding it gives 0) - refresh build/ first. Edits left in place; candidate-ready.
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
