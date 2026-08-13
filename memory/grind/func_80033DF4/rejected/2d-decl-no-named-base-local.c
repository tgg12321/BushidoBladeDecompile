/* REJECTED - 2D array decls but NO named table-pointer locals. Floor 11.
 * `D_8008EC24[row][entry]` used directly (no `u8 (*ranks)[5] = D_8008EC24;`
 * declared at the top of the else-block).  Measured this session: score 11.
 * The 12 residual instructions are the base-address materialization landing
 * AFTER the D_8008D9EC lookup instead of inside its load-delay window, which
 * also swaps $v0/$v1 across the whole tail.  The named pointer local moves
 * the lui/addiu pair to an earlier LUID so sched1 places it in the gap.
 */
        row = (&D_8008D9EC)[(&D_8008D538)[(s8)D_8010277C]] == 0;
        entry &= 0xFF;
        D_800A38DE = D_8008EC24[row][entry];
        D_800A38EC = table[1];
        D_800A38ED = table[2];
        D_800A38EE = table[3];
        D_8010277D = D_8008E908[row][entry];
