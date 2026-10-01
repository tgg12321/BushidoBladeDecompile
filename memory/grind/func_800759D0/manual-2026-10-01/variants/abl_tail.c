/* Character-select grid renderer (select-screen case 2 of func_80077374; the
 * draw half of func_80075F80), called once per player per frame.
 *   arg0 = draw context (arg0[0] root table, arg0[4] sprite chain,
 *          arg0[6] DR_MODE cursor)
 *   arg1 = select page into D_8009BCF8 (10 cells per page)
 *   arg2 = this player's pick list (-1 = cleared slot)
 *   arg3 = player index (0/1)
 * Draws the page frame, the page's 10 character cells (selectable cells as
 * sprites with the cursor cell highlighted, taken cells via func_80075830,
 * unselectable cells via func_80075830), the picks made so far, then the
 * f65+3 slot sprites, and closes with two DR_MODE/AddPrim pairs. */
void func_800759D0(s32 *arg0, s32 arg1, s16 *arg2, s32 arg3) {
    S_80074488 s;
    s32 *table;
    s32 *page2;
    s32 color;
    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
       func_8006E480's second argument at all three call sites is kept in
       callee-save $fp (target: `addu $fp,$zero,$zero` in the entry block,
       `addu $a1,$fp,$zero` at each call) instead of being re-materialized;
       global.c gives the once-set constant pseudo a callee-save because it
       crosses every call, and cse only folds a constant within the entry
       extended basic block. A literal 0 measures 30/364 (362 insns); see
       memory/grind/func_800759D0/hypotheses.md. Same shape as the siblings
       func_800753D8 (`zero`) and func_8007636C (`mode`). */
    s32 zero;
    s16 i;
    /* the sheet's cell array. The code treats each sheet it draws here as
       12-byte SprtHdrA headers followed by 8-byte SprtEntA cells: the head
       treats the table[0] page sheet as one header (cells at +0xC), and
       loops 1-3 treat their sheets as three headers, normal then one cursor
       highlight per player at +12/+24 (cells at +0x24). Loop 2 applies that
       three-header view to every pick slot, including the 0x14 placeholder
       func_80075F80 stores for an unavailable cell, whose sheet the code
       elsewhere draws as one header (func_80075830); there the +0x24 runs
       past the sheet into bytes nothing references. Assumed layout, path
       census, that anomaly path and the unreferenced-address search:
       memory/grind/func_800759D0/evidence.md "Ruling 9 (b')". */
    s32 cells;

    zero = 0;
    s.sp28 = 0;
    s.sp40 = 0;
    table = *(s32 **)(arg0[0] + 0x14);
    s.sp18 = table[0];
    s.sp30 = arg3 * 240 + 0x88;
    s.sp34 = 0x33;
    cells = s.sp18 + 0xC;
    s.sp1C = cells;
    if (arg3 != 0) {
        s.sp2C = 0x16;
    } else {
        s.sp2C = 0xC;
    }
    if (arg1 != 0) {
        s.sp1C += *(u8 *)(s.sp18 + 2) * 8;
    }
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
    arg0[6] += 0xC;

    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;
    s.sp41 = s.sp42 = s.sp43 = color;
    if (arg3 != 0) {
        s.sp2C = 0x14;
    } else {
        s.sp2C = 0xA;
    }
    s.sp30 = arg3 * 240;
    for (i = arg1 * 10; i < arg1 * 10 + 10; i++) {
        /* i indexes the whole table flat (arg1 * 10 + cell), from the first record. */
        /* SOTN: src/dra/62DEC.c:1091 @aa53500 */
        u8 entry = (&D_8009BCF8[0][0] + i)->unk0;

        if (D_8009BCE4[entry] & 1) {
            s.sp18 = table[entry + 1];
            cells = s.sp18 + 0x24;
            s.sp1C = cells;
            if (D_8009BCF8[arg1][SELWORK->f1C[arg3] * 5 + SELWORK->f20[arg3]].unk0 == (&D_8009BCF8[0][0] + i)->unk0) {
                s.sp40 = 1;
                s.sp18 = s.sp18 + 12 + arg3 * 12;
            } else {
                s.sp40 = 0;
            }
            s.sp34 = 0;
            s.sp20 = arg0[4];
            arg0[4] = func_8007352C((s32)&s);
            if (D_8009BCE4[(&D_8009BCF8[0][0] + i)->unk0] & (4 << arg3)) {
                func_80075830(arg0, i, arg3, 1);
            }
        } else {
            func_80075830(arg0, i, arg3, 0);
        }
    }

    for (i = 0; i < SELWORK->f3C[arg3] + 1; i++) {
        if (arg2[i] >= 0) {
            s.sp18 = table[arg2[i] + 1];
            cells = s.sp18 + 0x24;
            if (i != SELWORK->f3C[arg3]) {
                s.sp40 = 0;
            } else {
                s.sp40 = 1;
                s.sp18 = s.sp18 + 12 + arg3 * 12;
            }
            s.sp34 = i * 17;
            s.sp1C = cells;
            s.sp1C += *(u8 *)(s.sp18 + 2) * 8;
            s.sp20 = arg0[4];
            arg0[4] = func_8007352C((s32)&s);
        }
    }

    s.sp40 = 0;
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        table = *(s32 **)(arg0[0] + SELWORK->f65 * 4 + 0x20);
        s.sp18 = table[i];
        cells = s.sp18 + 0x24;
        if (i == SELWORK->f3C[arg3]) {
            s.sp18 = s.sp18 + 12 + arg3 * 12;
        }
        s.sp1C = cells;
        s.sp30 = arg3 * 240;
        s.sp34 = i * 17;
        if (arg3 != 0) {
            s.sp2C = 0x14;
        } else {
            s.sp2C = 0xA;
        }
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
    }

    page2 = *(s32 **)(arg0[0] + 0x14);
    s.sp18 = page2[1];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
    arg0[6] += 0xC;
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4 - 4, arg0[6]);
    arg0[6] += 0xC;
}
