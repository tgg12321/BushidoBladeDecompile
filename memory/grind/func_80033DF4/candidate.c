/* func_80033DF4 - COMPLETED-C candidate (grind session 1, 2026-08-13)
 * sandbox --disable all == 0 with ZERO register pins and ZERO inline asm.
 * Requires the file-local extern decls to be the 2D form:
 *     extern u8 D_8008E908[][5];
 *     extern u8 D_8008EC24[][5];
 * (they were `extern u8 D_XXXX;` scalar + pointer-arith before).
 * Floor path this session: 23 (pinned HEAD) -> 23 (pins removed, flat index)
 *   -> 11 (2D array decls) -> 0 (named table-pointer locals at block top).
 */
s32 func_80033DF4(void) {
    u8 state;
    s32 tableIndex;

    state = D_800A38E2;
    tableIndex = state & 0xFF;
    if (tableIndex == 0x64) {
        D_800A36F0 = 0;
        D_800A3781 = 0;

        if (D_800A3858 < 0x6979) {
            s32 *flags;
            s32 word;
            s32 mask;

            mask = 0x20;
            if ((&D_8008D9EC)[(&D_8008D538)[(s8)D_8010277C]] != 0) {
                mask = 0x10000;
            }
            flags = &D_80106A50;
            word = *flags;
            D_800A36F0 = (u32)(word & mask) < 1;
            *flags = word | mask;
        }

        if (D_800A380C == 0) {
            s32 *flags;
            s32 word;
            s32 mask;

            mask = 0x1000000;
            if ((&D_8008D9EC)[(&D_8008D538)[(s8)D_8010277C]] != 0) {
                mask = 0x4000000;
            }
            flags = &D_80106A50;
            word = *flags;
            D_800A3781 = (u32)(word & mask) < 1;
            *flags = word | mask;
        }

        func_80033D38();
        D_800A3834 = 4;
        return 0;
    } else {
        u8 *table = &cpu_practice_honmokuroku_data_tbl + (tableIndex * 4);
        u8 (*ranks)[5] = D_8008EC24;
        u8 (*moves)[5] = D_8008E908;
        s32 entry;
        s32 row;

        D_800A38E2 = state + 1;
        D_800A376B = 0;
        entry = table[0];
        *((u8 *)&D_800A384C) = entry;
        row = (&D_8008D9EC)[(&D_8008D538)[(s8)D_8010277C]] == 0;
        entry &= 0xFF;
        D_800A38DE = ranks[row][entry];
        D_800A38EC = table[1];
        D_800A38ED = table[2];
        D_800A38EE = table[3];
        D_8010277D = moves[row][entry];
