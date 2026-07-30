/* REJECTED (s3, structural).  Why it is dead: if loop2's row counter is not READ
 * inside the loop body, cc1's check_dbra_loop reverses the loop into a countdown
 * and target's `slti $v0,$a3,0x9` disappears — build_insns 169 against target's
 * 170, and no register seating can recover a missing instruction.
 *
 * Emitted tail with this form (cc1 .s):
 *      li   $7,0x8            <- counter preloaded with 8, counting DOWN
 *      ...
 *      addu $6,$6,2000
 *      addu $7,$7,-1
 *      bgez $7,.L93           <- no slt at all
 * Target keeps:
 *      addiu $a2,$a2,0x7D0
 *      addiu $a3,$a3,0x1
 *      slti  $v0,$a3,0x9
 *      bnez  $v0,.L80047A0C
 *
 * This form scored 11 with build_insns 169 and had OTHERWISE PERFECT loop1+loop2
 * dispositions, which is what made the diagnosis unambiguous: the deficit is the
 * instruction, not the allocation.  The fix is to give the counter an honest
 * in-body reference, which the accepted form does with the `a3 * 0x11` row index
 * (that index is also what makes the row pointer compiler-made, so the same
 * change buys the $a1/$a2 ordering).
 *
 * Corollary for successors: whenever a real loop's counter is used ONLY by its own
 * increment and test, expect cc1 to reverse it and expect to be one insn short.
 */

    a3 = 0;
    w = 0;
    ptr = &D_800EF59C[0];
    do {
        val = w;
        for (a0 = 0x10; a0 >= 0; a0--) {   /* a3 never read in the body */
            ptr[a0] = val;
        }
        w += 0x7D0;
        ptr += 0x11;
        a3 += 1;
    } while (a3 < 9);
