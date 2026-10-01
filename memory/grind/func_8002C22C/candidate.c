/* Accumulates a character's shadow vectors in PSX scratchpad RAM.  Everything
 * this function writes lives in the one record at 0x1F8002B8 that `scr` points
 * at (an s32 view; each index is the byte offset / 4): two 3-word vectors at
 * +0xA8 and +0xB8 (the +0xB4 and +0xC4 words are not touched, so both are
 * 16-byte-strided like a PsyQ VECTOR), and the 3-word result at +0x13C.
 * +0xA8 sums two of the scratchpad points SPAD holds per character (unk48[k]
 * when bit k of D_800A3824 is set, else unk00[k]); +0xB8 sums the matching
 * pair func_8002C61C copied into g_practice_menu_table[k] (unk_234 / unk_210).
 *
 * The first block's accesses come out absolute (`lui $at; sw $x,off($at)`)
 * while the second block's identical spelling comes out as register+
 * displacement (`sw $x,off($scr)`) - what the target does in both places.
 * cse decides that, not the source: find_best_addr
 * (tools/gcc-2.7.2/cse.c:2622) folds a `(plus reg const)` address to an
 * absolute one when that register's constant value is in cse's table, and
 * cse clears the table at the head of every extended-basic-block path it
 * processes (new_basic_block, cse.c:766).  scr and rec1 are set to their
 * constants at the top of the function, and no path reaching the second
 * if/else begins there, so the first block's references fold and the
 * second block's do not.
 *
 * Both if/else blocks are uniform per-arm statement runs.  The stores that
 * appear after each join in the target are jump.c cross-jumping merging the
 * arms' instruction-identical tails, not source-level statements.
 */
void func_8002C22C(void) {
    s32 *scr = (s32 *)0x1F8002B8;
    /* FAKE: pointer alias to g_practice_menu_table[1] (pointer-alias-fake-exception).
     * The target holds record 1's base in a register from entry (lui/addiu $t1)
     * and reads its unk_210 / unk_234 words at displacements off it, while
     * record 0's words are absolute.  Without the pointer every access is a
     * symbol+offset constant address, which GO_IF_LEGITIMATE_ADDRESS
     * (tools/gcc-2.7.2/config/mips/mips.h:2286) accepts as is, so no base
     * register exists: direct g_practice_menu_table[1] form 26 (+10 insns),
     * one table-base pointer for both records 13.  Ledger:
     * memory/grind/func_8002C22C/manual-2026-10-01/scores.txt */
    PracticeMenuRec *rec1 = &g_practice_menu_table[1];

    scr[0xA8/4] = 0;
    scr[0xAC/4] = 0;
    scr[0xB0/4] = 0;
    scr[0xB8/4] = 0;
    scr[0xBC/4] = 0;
    scr[0xC0/4] = 0;

    if (D_800A3824 & 1) {
        scr[0xA8/4] = SPAD->unk48[0][0].x;
        scr[0xAC/4] = SPAD->unk48[0][0].y;
        scr[0xB0/4] = SPAD->unk48[0][0].z;
        scr[0xA8/4] += SPAD->unk48[0][1].x;
        scr[0xAC/4] += SPAD->unk48[0][1].y;
        scr[0xB0/4] += SPAD->unk48[0][1].z;
        scr[0xB8/4] = g_practice_menu_table[0].unk_234[0].x;
        scr[0xBC/4] = g_practice_menu_table[0].unk_234[0].y;
        scr[0xC0/4] = g_practice_menu_table[0].unk_234[0].z;
        scr[0xB8/4] += g_practice_menu_table[0].unk_234[1].x;
        scr[0xBC/4] += g_practice_menu_table[0].unk_234[1].y;
        scr[0xC0/4] += g_practice_menu_table[0].unk_234[1].z;
    } else {
        scr[0xA8/4] = SPAD->unk00[0][0].x;
        scr[0xAC/4] = SPAD->unk00[0][0].y;
        scr[0xB0/4] = SPAD->unk00[0][0].z;
        scr[0xA8/4] += SPAD->unk00[0][1].x;
        scr[0xAC/4] += SPAD->unk00[0][1].y;
        scr[0xB0/4] += SPAD->unk00[0][1].z;
        scr[0xB8/4] = g_practice_menu_table[0].unk_210[0].x;
        scr[0xBC/4] = g_practice_menu_table[0].unk_210[0].y;
        scr[0xC0/4] = g_practice_menu_table[0].unk_210[0].z;
        scr[0xB8/4] += g_practice_menu_table[0].unk_210[1].x;
        scr[0xBC/4] += g_practice_menu_table[0].unk_210[1].y;
        scr[0xC0/4] += g_practice_menu_table[0].unk_210[1].z;
    }
    if (D_800A3824 & 2) {
        scr[0xA8/4] += SPAD->unk48[1][0].x;
        scr[0xAC/4] += SPAD->unk48[1][0].y;
        scr[0xB0/4] += SPAD->unk48[1][0].z;
        scr[0xB8/4] += rec1->unk_234[0].x;
        scr[0xBC/4] += rec1->unk_234[0].y;
        scr[0xC0/4] += rec1->unk_234[0].z;
        scr[0xA8/4] += SPAD->unk48[1][1].x;
        scr[0xAC/4] += SPAD->unk48[1][1].y;
        scr[0xB0/4] += SPAD->unk48[1][1].z;
        scr[0xB8/4] += rec1->unk_234[1].x;
        scr[0xBC/4] += rec1->unk_234[1].y;
        scr[0xC0/4] += rec1->unk_234[1].z;
    } else {
        scr[0xA8/4] += SPAD->unk00[1][0].x;
        scr[0xAC/4] += SPAD->unk00[1][0].y;
        scr[0xB0/4] += SPAD->unk00[1][0].z;
        scr[0xB8/4] += rec1->unk_210[0].x;
        scr[0xBC/4] += rec1->unk_210[0].y;
        scr[0xC0/4] += rec1->unk_210[0].z;
        scr[0xA8/4] += SPAD->unk00[1][1].x;
        scr[0xAC/4] += SPAD->unk00[1][1].y;
        scr[0xB0/4] += SPAD->unk00[1][1].z;
        scr[0xB8/4] += rec1->unk_210[1].x;
        scr[0xBC/4] += rec1->unk_210[1].y;
        scr[0xC0/4] += rec1->unk_210[1].z;
    }
    scr[0x13C/4] = ((scr[0xA8/4] * 3) + scr[0xB8/4]) >> 4;
    scr[0x140/4] = ((scr[0xAC/4] * 3) + scr[0xBC/4]) >> 4;
    scr[0x144/4] = ((scr[0xB0/4] * 3) + scr[0xC0/4]) >> 4;
}
