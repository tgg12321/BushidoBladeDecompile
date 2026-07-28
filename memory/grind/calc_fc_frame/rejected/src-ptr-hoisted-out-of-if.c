/* REJECTED (s1, floor 18, insn count 71 vs 72): hoisting
 * `src_ptr = (s32 *)src_base;` out of the if-block (to anchor addiu a0,a0,4
 * before the branch) lets GCC coalesce src_ptr with src_base and deletes the
 * `move s1,a0` copy entirely — structurally diverges. src_ptr must stay
 * inside the `if (val != -2)` block. The correct anchor for keeping
 * `addiu a0,a0,4` out of the beq delay slot is the unconditional
 * `val = *fp; fp++;` before the if: fp++ sits between the lh and the branch
 * in RTL, becomes reorg's nearest eligible backward slot candidate, and
 * fills the slot (matching target, where the slot insn executes on both
 * paths — proof the original incremented fp unconditionally). */
    src_base += 4;
    src_ptr = (s32 *)src_base;   /* WRONG: coalesces, kills move s1,a0 */
    data_ptr = (s32)orig_dest + (count + 2) * 4;
