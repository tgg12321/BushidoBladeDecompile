/* func_80086014 -- best form as of grind session 1 (recon).
 * Honest floor: sandbox --disable all == 10  (was 16 at session start).
 * IN PLACE in src/main.c:990 as of the end of session 1.
 *
 * This is 100% pure C: zero regfix/asmfix rules, zero register pins, zero
 * inline asm, zero volatile, zero dead code. It REPLACED an inherited
 * pin-and-barrier-laden body (5 `register asm("$N")` pins + a
 * `__asm__ volatile("" ::: "memory")` barrier + a `volatile` local), all of
 * which the cheat-invisible sandbox strips, and which scored WORSE (16) than
 * this clean form (10) once stripped -- the pins were buying nothing even on
 * their own terms, because the stripped `volatile` local forced 6 spill
 * insns through the stack.
 *
 * Semantic model (recovered this session): a 24-entry table (0x18 = the bound)
 * with a 16-byte stride. Two s16 fields per slot live at the addresses splat
 * named D_80102A78 and D_80102A7A (hence the `[idx * 8]` s16-element
 * spelling of a 16-byte stride), plus a per-entry flag byte in the parallel
 * array D_800F65E0 where bits 0-1 are set to mark the slot dirty. The twin
 * setter func_80086130 (src/main.c:1033, also queued, also pin-laden) writes
 * the SAME two fields with both values pre-scaled by 129 ((v<<7)+v); the
 * matched getter func_80086080 (src/main.c:1014) reads them back and divides
 * by 129. So x/y here are raw, unscaled coordinates.
 *
 * WHAT THIS FORM ALREADY GETS EXACTLY RIGHT -- do not perturb it blindly:
 * register allocation is instruction-for-instruction identical to target,
 * including the non-obvious `move a3,a1` param-save in the bnez delay slot
 * (GCC needs it because the byte offset `idx*16` is computed into $a1,
 * clobbering the param) and the $v1 index / $a1 offset / $a0 flags choices.
 * That fell out of the naive spelling for free. The remaining 10 is NOT an
 * allocation problem.
 *
 * THE ENTIRE RESIDUAL (25 built insns vs 27 target insns):
 *   1. THE FRAME (2 of the 2 missing insns). Target opens with
 *      `addiu $sp,$sp,-8` and closes with `addiu $sp,$sp,8`, and NEVER issues
 *      a single $sp-relative load or store -- a PHANTOM frame
 *      ([[phantom-frame-slots-gcc272]]). We emit no frame at all (cc1 reports
 *      `vars= 0`). The twin func_80086130 has the identical 8-byte phantom
 *      frame, so whatever source shape causes it is a property of this
 *      function family, and solving it here solves both.
 *   2. TWO SCHEDULING PLACEMENTS. We hoist `move v0,zero` (the `return 0`
 *      value) to the top of the taken block; target emits it mid-block, right
 *      after the flag `lbu`. And we hoist the flag `lbu` above the first `sh`;
 *      target issues it after that store. Both are cc1 sched1 decisions --
 *      GCC can freely reorder because the store to D_80102A7A provably cannot
 *      alias the load from D_800F65E0 (distinct symbols).
 *
 * Session 1 measured the frame axis hard; see hypotheses.md for the full
 * gradient table and the two banked kills. Short version: local SCALAR
 * spelling is completely inert here (10 spellings, all `vars= 0`, all
 * byte-identical), while a >=4-byte local AGGREGATE does reach the exact
 * target frame signature (`vars= 8, regs= 0/0, args= 0`) with zero stack
 * traffic -- but every aggregate spelling found so far pays >=4 extra
 * instructions for a mode-punning pack/unpack round-trip, so none of them is
 * a net win yet. That is the live frontier, not a wall.
 */
s32 func_80086014(s16 idx, s16 x, s16 y)
{
    if ((u16)idx < 0x18) {
        D_80102A7A[idx * 8] = y;
        D_80102A78[idx * 8] = x;
        D_800F65E0[idx] |= 3;
        return 0;
    }
    return -1;
}
