"""Respelling of every C consumer of the 0x800A3560 records for the
aggregate merge (u8 D_800A3560[] + D_800A3561/D_800A3564 scalars ->
one array of 3-byte records). (old, new, expected_count)."""
DECL_OLD = 'extern u8 D_800A3560[];\nextern u8 D_800A3561;\nextern u8 D_800A3564;\n'
REC_DECL = ('typedef struct {\n    u8 unk0;\n    u8 unk1;\n    u8 unk2;\n} Rec800A3560;\n'
            'extern Rec800A3560 D_800A3560[2];\n')

FAKE_REC = """                    /* FAKE: named intermediate for player i's 3-byte D_800A3560
                     * record offset (named-intermediate entry, no-new-park-categories.md).
                     * mechanism: loop.c scan_loop -- the inline `D_800A3560[i * 3]`
                     * expands the symbol load BEFORE the index insns (expr.c:4659
                     * INDIRECT_REF, EXPAND_SUM MULT), so that pseudo lives 3 insns and
                     * threshold*savings*lifetime >= insn_count (loop.c:1631) hoists it,
                     * leaving `lui/addiu/addu/lbu 0()`; with the index already in a
                     * pseudo it lives 1 insn, stays put, and combine folds it into the
                     * target's `lbu %lo(D_800A3560)(at)`. Lever exhaustion: inline index,
                     * `*(D_800A3560 + i * 3)` and `D_800A3560[i + i * 2]` all 31/515
                     * (memory/grind/func_8006F97C/hypotheses.md). */
                    s32 rec = i * 3;

                    if (D_800A3560[rec] == 0xFF) {
"""
FAKE_CTX_70C70 = """            s32 ctx = var_s0 * 3; /* FAKE: named intermediate for the D_800A3560 byte
                     * index, mechanism: loop.c strength_reduce reduces ctx itself as the
                     * giv to a byte OFFSET biv (the target's `addu $at,$at,$s2` /
                     * `addiu $s2,$s2,3`) instead of reducing the full ADDRESS giv the
                     * inlined index builds; lever-exhaustion: memory/grind/func_80070C70/
                     * hypotheses.md [s6] 30-variant index sweep - inlined index 53/193 vs
                     * this named local 49/194 (rejected/inlined-index-for-chassis-53.c) */
            code = D_800A3560[ctx];
"""

def reps(decl_new):
    return [
        (DECL_OLD, decl_new, 1),
        ('extern u8 D_800A3560[];\nextern SelectEntryE534 D_8009BC40[][6];\n',
         'extern SelectEntryE534 D_8009BC40[][6];\n', 1),
        ('extern s16 D_800A3558;\nextern u8 D_800A3560[];\nextern s16 D_800A3590[];\n',
         'extern s16 D_800A3558;\nextern s16 D_800A3590[];\n', 1),
        ('INCLUDE_ASM("asm/funcs", func_80070F78);\nextern u8 D_800A3561;\n',
         'INCLUDE_ASM("asm/funcs", func_80070F78);\n', 1),
        # func_8006E534
        ('    D_800A3561 = D_8009BC40[0][0].value;\n    D_800A3564 = D_8009BC40[0][2].value;\n',
         '    D_800A3560[0].unk1 = D_8009BC40[0][0].value;\n    D_800A3560[1].unk1 = D_8009BC40[0][2].value;\n', 1),
        # func_8006ECF4, func_8006F100
        ('D_800A3560[i * 3 + 1]', 'D_800A3560[i].unk1', 2),
        ('D_800A3560[i * 3 + 2]', 'D_800A3560[i].unk2', 1),
        # func_8006F100, func_80071C20, func_80071C4C
        ('D_8009BC7C[D_800A3561]', 'D_8009BC7C[D_800A3560[0].unk1]', 3),
        # func_8006F97C
        (FAKE_REC, '                    if (D_800A3560[i].unk0 == 0xFF) {\n', 1),
        ('    D_800A32E8 = D_800A3564;\n', '    D_800A32E8 = D_800A3560[1].unk1;\n', 1),
        # func_80070C70
        (FAKE_CTX_70C70, '            code = D_800A3560[var_s0].unk0;\n', 1),
        # func_80071C4C
        ('        s32 ctx = i * 3;\n\n        if (D_800A3560[ctx] != 5 && D_800A3560[ctx] != 16) {\n',
         '        if (D_800A3560[i].unk0 != 5 && D_800A3560[i].unk0 != 16) {\n', 1),
        ('D_800A3560[ctx + 2] * 4', 'D_800A3560[i].unk2 * 4', 1),
        ('D_8009BC7C[D_800A3560[ctx + 1]]', 'D_8009BC7C[D_800A3560[i].unk1]', 1),
        ('            s32 dst = i * 10;\n            s32 ctx = i * 3;\n\n            *(u8 *)(D_800A3568 + dst) = D_800A3560[ctx];\n',
         '            s32 dst = i * 10;\n\n            *(u8 *)(D_800A3568 + dst) = D_800A3560[i].unk0;\n', 1),
        ('            s32 dst = i * 10;\n            s32 ctx = i * 3;\n\n            *(u8 *)(D_800A3568 + dst + 1) = D_800A3560[ctx + 2];\n',
         '            s32 dst = i * 10;\n\n            *(u8 *)(D_800A3568 + dst + 1) = D_800A3560[i].unk2;\n', 1),
        # func_800720FC
        ('                s32 ctx = i * 3;\n                s32 slot;\n', '                s32 slot;\n', 1),
        ('slot = D_800A3560[ctx];', 'slot = D_800A3560[i].unk0;', 1),
        ('D_800A3560[ctx + 2] = 0xFF;', 'D_800A3560[i].unk2 = 0xFF;', 1),
        ('other = i == 0 ? 3 : 0;\n                    D_800A3560[other + 2] = 0xFF;\n                    D_800A3560[ctx] = 0xFF;\n',
         OTHER_FORMS[os.environ.get('OTHER_FORM', 'o1')] + '                    D_800A3560[i].unk0 = 0xFF;\n', 1),
        ('    s32 c;\n    s32 other;\n    s16 *timer;\n', '    s32 c;\n    s16 *timer;\n', 1),
    ]

import os
OTHER_FORMS = {
    'o0': 'other = i == 0 ? 1 : 0;\n                    D_800A3560[other].unk2 = 0xFF;\n',
    'o1': 'D_800A3560[i == 0 ? 1 : 0].unk2 = 0xFF;\n',
    'o2': 'D_800A3560[i == 0].unk2 = 0xFF;\n',
    'o3': 'D_800A3560[!i].unk2 = 0xFF;\n',
    'o4': 'D_800A3560[i != 0 ? 0 : 1].unk2 = 0xFF;\n',
}
