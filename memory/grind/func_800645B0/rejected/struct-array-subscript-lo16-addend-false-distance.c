/* REJECTED (grind session 7): the struct-array SUBSCRIPT spelling.
 * WHY IT IS DEAD: not because of the codegen -- this is 78 instructions with a
 * TRUE byte residual of 3 (the loop top only) -- but because it can never be
 * DEMONSTRATED at sandbox 0.  `D_800F0D78[idx].unk4` relocates against
 * %lo(D_800F0D78) with addend 4 where the target relocates against
 * %lo(D_800F0D7C) with addend 0; D_800F0D7C == D_800F0D78 + 4 and videoDec ==
 * D_800F0D78 + 8 so the LINKED WORDS ARE IDENTICAL, but engine/score.py
 * compares disassembly text and prints `sw v1,4(at)` vs `sw v1,0(at)`.  It
 * scores 5 / 78 and two of those five points are unremovable.  Same class as
 * [[sandbox-lo16-text-addend-false-distance]].  It also requires changing the
 * file-scope declarations from three per-word `extern s32` scalars to
 * `extern VideoDecSlot D_800F0D78[];`, which erases the named symbol videoDec
 * (named_syms.txt:1065).  The usable carrier of the same pseudo set is the
 * per-symbol cast form with inline index arithmetic: chassis_jd_inline_index_arith.c.
 * (This file records the FUNCTION body only; the declaration edit it needs is
 * in tmp/grind/func_800645B0/s7/sweep22.py.)
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                D_800F0D78[idx].unk0 = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                D_800F0D78[idx].unk4 = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                D_800F0D78[idx].unk8 = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
