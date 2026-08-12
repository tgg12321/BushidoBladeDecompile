/* func_800645B0 (src/text1b.c) — grind session 2 (recon modality), 2026-08-12.
 *
 * HONEST PURE-C SANDBOX DISTANCE: 3  (`sandbox func_800645B0 --disable all`,
 * target_insns 78 / build_insns 78, rules_dropped 1).
 *
 * PROVENANCE. Session 1 reached distance 0, but its last lever (relocating
 * `j += 1;` to sit between `idx = i + j;` and `val = 1;`) was FAILed by the
 * layer-1 cheat-reviewer as an unsanctioned statement-reorder chosen only to
 * win a first-pass-scheduler tie, and is now a BANNED construct for this
 * function. This file is session 1's constructs 1-4 (all ruled legitimate)
 * with `j += 1;` back at its natural position, PLUS one new construct:
 *
 *   `idx = idx2 + idx;`  — the slot index variable is reused to hold the
 *   *3 (12-byte-stride) word index for the three parallel-array stores.
 *
 * WHY THAT CONSTRUCT, MECHANISM-FIRST (not a reorder — a different attack):
 * tools/gcc-2.7.2/sched.c:birthing_insn_p returns `reg_n_sets[dest] == 1`,
 * and adjust_priority raises such an insn's INSN_PRIORITY to max_priority.
 * The .sched dump for the inner-loop block (basic block 2, insns 28/31/33/
 * 36/39/40/42) shows this verbatim:
 *
 *   with `idx` single-set:   ready list at T-6: 31 (1) 28 (7f000001), now 28 31
 *   with `idx` multi-set:    ready list at T-6: 31 (1) 28 (1),        now 31 28
 *
 * The scheduler is a BACKWARD list scheduler, so the insn picked at T-6 is
 * placed LATER in the block. With the bonus, `addu idx,i,j` (insn 28) is
 * picked at T-6 and `li val,1` (insn 31) lands first in the block — and
 * reorg.c then steals that first insn into the inner back-edge delay slot,
 * where the target instead has `addu $s0,$s3,$a0`. Removing the bonus leaves
 * a plain INSN_LUID tie-break, which restores source order (addu first) and
 * closes that residual WITHOUT touching statement order.
 *
 * The construct is also what the target itself does: $s0 holds i+j, then the
 * (idx*2 + idx) sum, then the <<2 byte offset — one register, three roles.
 * Family: variable reuse for codegen control ([[defeat-licm-hoist-var-reuse]],
 * frozen SOTN-accepted list), same family already used for `val`.
 *
 * WHAT REMAINS (3 points, fully characterised — see evidence.md):
 *   idx | target                   | this build
 *   18  | jal rand                 | sll  s1,s0,0x1
 *   19  |  sll  s1,s0,0x1 (delay)  | jal  rand
 *   20  | addu s0,s1,s0            | addu s0,s0,s1
 * i.e. (a) the target emits the *3 sum AFTER the first rand() call, so
 * reorg.c fills the jal delay slot with the `sll`; here both the sll and the
 * sum precede the call, so the sum fills the slot instead; and (b) the
 * commutative PLUS operands are (idx, idx2) here but (idx2, idx) in target.
 *
 * INTEGRATION NOTE: regfix.txt:2521 (`func_800645B0: reorder 3,1,2 @ 1-3`)
 * is the function's only rule; the sandbox drops it. Retiring it is the
 * operator/driver's job, not a grind session's.
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
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            val = 1;
            mask = val << idx;
            j += 1;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                idx = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (idx << 2))) = (((s32 *)D_800A347C)[0] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
