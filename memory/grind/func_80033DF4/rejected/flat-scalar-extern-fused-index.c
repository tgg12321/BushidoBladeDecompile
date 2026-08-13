/* REJECTED - flat scalar extern + fused (row+entry) index. Floor 11, not 0.
 * With `extern u8 D_8008EC24;` (scalar) and `(&D_8008EC24)[row + entry]`,
 * GCC folds the symbol into the load: `lui $at,%hi(sym); addu $at,$at,$idx;
 * lbu $rd,%lo(sym)($at)`.  Target instead MATERIALIZES the table base into a
 * GPR (`lui $v1,%hi; addiu $v1,$v1,%lo`) and adds row then entry to it.
 * The 2D declaration `extern u8 D_8008EC24[][5];` + `D_8008EC24[row][entry]`
 * produces the (base + row*5) + entry association and drops the floor to 11;
 * the remaining 11 was the base-materialization SCHEDULING position.
 */
        row = (&D_8008D9EC)[(&D_8008D538)[(s8)D_8010277C]] == 0;
        entry &= 0xFF;
        row = row * 5;
        D_800A38DE = (&D_8008EC24)[row + entry];
        D_800A38EC = table[1];
        D_800A38ED = table[2];
        D_800A38EE = table[3];
        D_8010277D = (&D_8008E908)[row + entry];
