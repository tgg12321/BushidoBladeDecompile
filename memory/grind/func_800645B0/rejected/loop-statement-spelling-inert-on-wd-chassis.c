/* REJECTED (s17b, 2026-09-02, structural) -- INERT, not one instruction moved.
 *
 * Claim tested: the inner/outer loop STATEMENT form changes the RTL loop notes
 * and basic-block structure around the inner-loop head, and could therefore
 * substitute for the banned `do { } while (0);` wrap's effect on cc1's
 * first-pass scheduler (sched.c adjust_priority / birthing_insn_p).
 *
 * Measured on the WD chassis (`wid = idx2 + idx;` fresh sum destination),
 * honest `sandbox func_800645B0 --disable all`, all at 78/78 insns:
 *   W0 control (both loops `for`)                          3 / 78
 *   W1 inner loop `j = 0; do { ... j += 1; } while (j<4);`  3 / 78   <-- this file
 *   W2 inner loop `j = 0; while (j<4) { ... j += 1; }`      3 / 78
 *   W3 outer loop `i = 0; do { ... i += 4; } while (i<0xF);`3 / 78
 *   W4 both loops in do/while form                          3 / 78
 * Every variant is byte-identical to the control: the residual stays at stream
 * indices 11/12 (loop head) + 65 (back-edge delay slot).
 *
 * Mechanism: GCC 2.7.2 canonicalises all three statement forms to the same
 * bottom-tested RTL loop (one NOTE_INSN_LOOP_BEG, one top label, the exit test
 * at the bottom), because `j` is provably 0 < 4 on entry so no loop guard is
 * emitted.  The wrap's effect came from an EXTRA, nested loop note pair inside
 * the body -- which no loop STATEMENT belongs at that point semantically.
 * This extends session 4's H20 (`for` vs do/while byte-identical) from the
 * old CA chassis to WD, and closes the loop-spelling axis on the chassis that
 * actually carries the residual.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    s32 wid;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        j = 0;
        do {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        j += 1;
        } while (j < 4);
    }
    return 1;
}
