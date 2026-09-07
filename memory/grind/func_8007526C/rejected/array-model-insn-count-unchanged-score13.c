/* REJECTED (s1 2026-09-07) -- ordinary C, semantically the census data model spelled as the
 * u16[row][2] table it actually is (t[2][i]=+0x8, t[3][i]=+0xC, t[4][i]=+0x10, t[5][i]=+0x14,
 * t[6][i]=+0x18, t[14][i]=+0x38, t[15][i]=+0x3C, instance stride 2 bytes).
 *
 * MEASURED: sandbox score 13, build_insns 93 -- IDENTICAL to memory/grind/func_8007526C/candidate.c
 * in every respect, and the .loop dump reports the SAME "Loop from 14 to 543: 92 real insns."
 * Rejected as an insn_count lever (frontier F2): array-indexed addressing does NOT inflate the
 * loop's RTL insn count at loop.c scan_loop time, because cse1 runs BEFORE loop.c and collapses
 * every `base + row*4 + i*2` into the single `base + i*2` pseudo that candidate.c writes by hand,
 * leaving the constant row offset inside the MEM (a valid MIPS `plus(reg, const)` address that
 * costs no separate insn).  Kept only as the record that the data-model-faithful spelling and the
 * byte-offset spelling are the same compile.
 */
void func_8007526C(void) {
    u16 (*t)[2];
    s32 i;

    t = (u16 (*)[2])D_800A36A0;
    i = 0;
    do {
        switch ((u8)t[4][i]) {
        case 1:
            t[2][i] = t[2][i] + 0xA;
            t[3][i] = t[3][i] + 0xA;
            if ((s16)t[3][i] >= 0xC8) {
                if ((t[4][i] >> 8) == 0) {
                    t[4][i] = t[4][i] + 1;
                }
                t[2][i] = 0;
                t[3][i] = 0xC8;
                t[5][i] = t[6][i];
                t[15][i] = t[14][i];
            }
            break;
        case 3:
            t[3][i] = t[3][i] + 0xA;
            if ((s16)t[3][i] >= 0xC8) {
                t[2][i] = 0xC8;
                t[3][i] = 0xC8;
                t[5][i] = t[6][i];
                t[15][i] = t[14][i];
                if ((t[4][i] >> 8) == 0) {
                    t[4][i] = t[4][i] + 1;
                }
            }
            break;
        case 2:
            t[3][i] = t[3][i] - 0xA;
            if ((s16)t[3][i] <= 0) {
                t[2][i] = 0;
                t[3][i] = 0;
                t[4][i] = 0;
            }
            break;
        case 4:
            t[2][i] = t[2][i] - 0xA;
            t[3][i] = t[3][i] - 0xA;
            if ((s16)t[3][i] <= 0) {
                t[2][i] = 0;
                t[3][i] = 0;
                t[4][i] = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
