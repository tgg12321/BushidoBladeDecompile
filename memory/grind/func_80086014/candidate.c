/* func_80086014 -- MATCHING pure-C form, found in grind session 2 (structural).
 *
 * STATUS: byte-identical to target.  Full clean-driver build with this body in
 * src/main.c:990 produced SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == the
 * oracle, and the linked disassembly at 0x80086014 matches all 27 words of
 * asm/funcs/func_80086014.s (capture: tmp/grind/func_80086014/s2/linked.txt).
 * Zero regfix/asmfix rules, zero register pins, zero inline asm, zero volatile,
 * zero dead code, zero FAKE constructs.
 *
 * The cheat-invisible sandbox still prints score 1 for this body.  That single
 * point is a scoring artifact, not a real diff: the sandbox compares an UNLINKED
 * object, where the y-store assembles as `sh $a2, %lo(D_80102A78+2)($at)` and so
 * carries immediate 0x0002 plus a relocation, while target's dump carries the
 * already-resolved immediate 0x2A7A (`%lo(D_80102A7A)`).  The linker resolves
 * D_80102A78+2 to 0x80102A7A, so the emitted word is a4262a7a either way -- see
 * the linked capture above.
 *
 * THE ONE STRUCTURAL CHANGE vs session 1's floor-10 body: the y half of the pair
 * is written as element +1 of the SAME base symbol (`D_80102A78[idx * 8 + 1]`)
 * instead of through the interior splat symbol `D_80102A7A[idx * 8]`.  That is
 * the object model: one 24-entry table with a 16-byte stride whose first two s16
 * members are the pair; `D_80102A7A` is just splat's name for base+2, and the
 * original source would only have had one handle for the table.  Writing it that
 * way is what closes BOTH halves of session 1's residual at once:
 *
 *   1. THE 8-BYTE PHANTOM FRAME (the 2 missing instructions).  With one base and
 *      a constant delta INSIDE the variable index expression, cc1 expands a
 *      separate address pseudo for the second access and combine then folds that
 *      arithmetic into the store's %lo displacement, replacing the insn with a
 *      bare `(use (reg:SI N))`.  A use-only pseudo is classified "ST_REGS or
 *      none" in .lreg, so global_alloc cannot allocate it and reload1.c's
 *      alter_reg hands it a spill slot -- get_frame_size() counts 4 bytes,
 *      MIPS_STACK_ALIGN rounds to 8, and no load or store is ever emitted.
 *      That is target's `addiu $sp,-8` / `addiu $sp,8` with zero $sp traffic.
 *      Mechanism established by breaking on assign_stack_local under gdb
 *      (tmp/grind/func_80086014/s2/gdb.sh) against the in-tree witness
 *      snd_GetFadeCurve (src/sound.c:993 = func_80047E5C), which carries the
 *      same phantom frame from the same shape (`arr[v1]` + `arr[v1 + 1]`).
 *   2. THE TWO SCHEDULING PLACEMENTS.  The extra address pseudo also changes the
 *      dependence graph sched1 sees, so the flag `lbu` now issues AFTER the first
 *      `sh` and `move $v0,$zero` lands right after the `lbu`, exactly as target
 *      has them.  No barrier, no volatile, no pin -- session 1 had proven those
 *      cost 6 points rather than buying anything.
 *
 * Everything session 1 got right is preserved: register allocation is still
 * instruction-for-instruction identical, including the `move a3,a1` param-save
 * in the bnez delay slot.
 *
 * NOTE FOR THE TWIN: func_80086130 (src/main.c:1033, still queued, still
 * pin-laden) writes the same two fields pre-scaled by 129 and has the IDENTICAL
 * 8-byte phantom frame.  The same single-base `+ 1` spelling is the first thing
 * to try there.
 */
s32 func_80086014(s16 idx, s16 x, s16 y)
{
    if ((u16)idx < 0x18) {
        D_80102A78[idx * 8 + 1] = y;
        D_80102A78[idx * 8] = x;
        D_800F65E0[idx] |= 3;
        return 0;
    }
    return -1;
}
