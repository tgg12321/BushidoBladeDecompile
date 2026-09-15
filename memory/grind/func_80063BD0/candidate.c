/* memory/grind/func_80063BD0/candidate.c -- 2026-09-15 re-dispatch (driver session 1,
 * recon) after the 2026-09-15 02:51 layer-1 FAIL.  Honest 0/144 + full-tree oracle
 * SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa measured THIS session with
 * memory/grind/func_80063BD0/candidate_merge.patch applied to a clean HEAD.  The
 * patch touches exactly: include/game.h (Unk800F0EC8Record typedef + `extern
 * Unk800F0EC8Record D_800F0EC8[][10];`), src/text1b.c (this body; TU-local
 * `extern s32 D_800F0EC8[][10][3];` and the three scalar externs removed;
 * `extern u32 D_800A344C[]` widening x2; func_80060C60 `D_800A344C[0] = 0;`),
 * src/text1b_b.c (three unused scalar externs D_800F0EC8/ECC/ED0 removed),
 * undefined_syms_auto.txt (D_800F0ECC / D_800F0ED0 rows suffixed `alias of
 * D_800F0EC8+N; retire with func_80063E10`).  The last two files and the header
 * are outside the default grind scope -> INTEGRATION HANDOFF via ruling-request.
 */
extern s32 D_800A3478;
extern SVECTOR D_800F1000[][10];
/* func_80063BD0 (src/text1b.c) -- MATCHING FORM.  Honest distance 0 / 144
 * (sandbox --disable all, zero cheat-asm, zero rules) measured in grind
 * session s1 (2026-09-15, recon modality) with this exact body in place, and
 * re-measured 0/144 + full-tree oracle SHA1 in the 2026-09-15 re-dispatch with
 * the record table declared header-canonically (include/game.h
 * Unk800F0EC8Record D_800F0EC8[][10]; the TU-local flat `[][10][3]` spelling
 * was layer-1 FAILed 2026-09-15 02:51 and is banned for this function).
 *
 * Slot allocator for lane `idx`: D_800A344C[idx] counts live entries, and
 * D_800A3454[idx] is the per-slot in-use bitmask.  While fewer than 10
 * entries are live, take the lowest free bit, mark it, and fill that slot's
 * SVECTOR (D_800F1000[idx][slot]) and 3-word record (D_800F0EC8[idx][slot])
 * from the source pointers D_800A3478 / D_800A347C.  Once the lane is full,
 * the counter wraps through 10..19 and the slot is overwritten in rotation.
 *
 * Shape notes (each alternative was measured, see
 * memory/grind/func_80063BD0/hypotheses.md):
 *  - `for` loop with the found-arm INSIDE the loop and `break`: the loop's
 *    duplicated exit test (jump.c duplicate_loop_exit_test) plus the arm's
 *    skip label is what keeps the D_800A344C base copy in the preheader
 *    (cse.c cse_around_loop stops scanning at the first CODE_LABEL); a
 *    `goto found` arm after the loop measured 4 (base coalesced).
 *  - `bits`/`mask` read before the test: the array read must be expanded
 *    before the `1 << i` so loop.c hoists the D_800A3454 address ahead of
 *    the constant 1 (their preheader order is the loop-body order).
 *  - `|= mask` (not `= bits | mask` -- both measure 0; `|=` is the natural
 *    spelling).  A single trailing `return 1` that the else-arm falls into
 *    keeps `li v0,1` out of the else-arm block, which frees v0 there.
 */
u8 func_80063BD0(s32 idx) {
    s32 bits;
    s32 mask;
    s32 i;

    if (D_800A344C[idx] < 10) {
        D_800A344C[idx]++;
        for (i = 0; i < D_800A344C[idx]; i++) {
            bits = D_800A3454[idx];
            mask = 1 << i;
            if (!(bits & mask)) {
                D_800A3454[idx] |= mask;
                D_800F1000[idx][i].vy = ((u16 *)D_800A3478)[1];
                D_800F1000[idx][i].vx = D_800F1000[idx][i].vz = 0;
                D_800F0EC8[idx][i].unk0 = ((s32 *)D_800A347C)[0];
                D_800F0EC8[idx][i].unk4 = ((s32 *)D_800A347C)[1];
                D_800F0EC8[idx][i].unk8 = ((s32 *)D_800A347C)[2];
                break;
            }
        }
    } else {
        D_800A344C[idx] = (D_800A344C[idx] + 1) % 10 + 10;
        D_800F1000[idx][D_800A344C[idx] - 10].vy = ((u16 *)D_800A3478)[1];
        D_800F1000[idx][D_800A344C[idx] - 10].vx = D_800F1000[idx][D_800A344C[idx] - 10].vz = 0;
        D_800F0EC8[idx][D_800A344C[idx] - 10].unk0 = ((s32 *)D_800A347C)[0];
        D_800F0EC8[idx][D_800A344C[idx] - 10].unk4 = ((s32 *)D_800A347C)[1];
        D_800F0EC8[idx][D_800A344C[idx] - 10].unk8 = ((s32 *)D_800A347C)[2];
    }
    return 1;
}
