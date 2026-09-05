/* s62 (structural, 2026-09-05) -- FLOOR 9, on a CLEANER object model.
 *
 * Score is unchanged from s61 (9, 49/49) but two things improved and both are
 * inheritance for the next session:
 *
 * 1. THE DECLARATION IS NOW HONEST.  s59-s61 declared `extern u8
 *    D_80106A70[4];` and reached the flag byte as element [3] -- an array whose
 *    fourth element is a semantically unrelated census-named flag byte.  This
 *    session measured the SPLIT model the data model actually supports:
 *
 *      include/code6cac.h:472-474
 *        -  extern u8 D_80106A70;
 *        -  extern u8 D_80106A71;
 *        -  extern u8 D_80106A72;
 *        +  extern u8 D_80106A70[3];
 *      src/code6cac.c:340-342  D_80106A70/71/72 = ...  ->  D_80106A70[0..2] = ...
 *      src/code6cac.c:345      the same three reads   ->  element form
 *
 *    plus `extern u8 D_80106A73;` staying the separate scalar it already is in
 *    src/code6cac_b.c:128.  This is exactly the symbol pair the target's three
 *    la pairs name (all three are %hi/%lo(D_80106A73); the loop's base is
 *    %hi(D_80106A70)).  Measured byte-neutral: func_8001945C, func_80019488 and
 *    func_80037F40 all sandbox at score 0 under it, and func_80034F88 measures
 *    9 -- identical RA model to the [4] form (order [73,77,75,74,81,72,76],
 *    same nrefs/livelen/pri).  The [4] form should not be carried forward.
 *
 * 2. THE BLOCK-0 VALUE PSEUDO IS HALFWAY TO ITS TARGET PRIORITY.  Letting the
 *    two arms read `*q` directly (duplicate-read-into-arms, ordinary C) instead
 *    of a reloaded local drops the block-0 value allocno from 8 refs / pri
 *    24000 (s61) to 6 refs / livelen 9 / pri 13333, WITHOUT re-introducing the
 *    local-alloc hard-reg-3 block on the address allocno.  Address allocno 76
 *    is at pri 3076 (livelen 26).  See evidence E62.2/E62.3 for why every other
 *    ref-reduction spelling measured this session (C/D2/D5/F/H1) buys the ref
 *    cut at the price of a $v1-seated block-local, which forecloses the seat
 *    outright.
 *
 * RESIDUAL (unchanged in shape from s61, 9 of 49, all inside block 0):
 *   ours                       target
 *   lui/addiu $a2              lui/addiu $v1     <- address seat
 *   lbu   $v1, 0($a2)          lbu   $a0, 0($v1) <- value seat
 *   andi  $v1, $v1, 0xF8       andi  $a0, $a0, 0xF8
 *   sb    $v1, 0($a2)          sb    $a0, 0($v1)
 *   nop                        lbu   $a0, 0($v1) <- the block-0 reload
 *   ori   $v0, $v1, 1          ori   $v0, $a0, 1
 *   move  $v0, $v1             addu  $v0, $a0, $zero
 *   sb    $v0, 0($a2)          sb    $v0, 0($v1)
 *
 * THE TWO DEFECTS ARE ONE DEFECT (s62's main finding).  The target's reload
 * `lbu $a0, 0($v1)` at 80034FB4 sits in the LOAD-DELAY SLOT of
 * `lw $v0, 0x20($a1)`; it can only sit there because its destination ($a0)
 * differs from the lw's ($v0).  K1/K3 (mask carried in `c`, so the flag read
 * clobbers the stored value's pseudo and cse2 cannot forward) DO restore the
 * reload -- but with our seats the reload lands in $v0, collides with the lw,
 * and the scheduler emits the nop anyway: 50 insns, score 34.  So fixing the
 * seats and restoring the reload are not two independent points; the reload is
 * free the moment the value sits in $a0.
 *
 * AND THE SEAT ARITHMETIC CLOSES ON ITSELF (E62.5).  With the reload present
 * the address allocno gains a fifth reference (5 refs / livelen 28 -> pri
 * 3571, measured on K1), while a 3-reference block-0 value whose def is the
 * reload has pri 30000/livelen.  In the TARGET's own emission order the reload
 * stands ~5 insns above the last arm read, i.e. livelen ~10 -> pri ~3000 <
 * 3571: the address allocno is reached FIRST by global.c's descending-priority
 * loop, takes $v1 (no conflict with 73/74), and the value is pushed to $a0
 * (no conflict with 81/82, which is why blocks 1-2 keep $a0).  The target
 * state is self-consistent; what is missing is a cse2 invalidator that leaves
 * the reload at a source position ABOVE the flag read, so its live range is
 * long instead of 4.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 v;
    s32 c;

    p = func_80077D00();
    {
        /* FAKE: block-0's own address object, mechanism: global.c:1275 assigns
         * exactly one hard register per allocno and GCC 2.7.2 does no
         * live-range splitting, so the target's two address seats ($v1 in
         * block 0, $a0 in blocks 1-2) are only reachable with two allocnos.
         * lever-exhaustion: the single-object chassis is measured flat at 10
         * across s53-s60 (memory/grind/func_80034F88/hypotheses.md), and the
         * pointer-object-free array spelling costs an instruction
         * (rejected/s59b-...-50insn-score24.c). */
        u8 *q = &D_80106A73;
        s32 u;

        u = *q;
        u = u & 0xF8;
        *q = u;
        c = p[8] & 1;
        if (c) {
            c = *q | 1;
        } else {
            c = *q;
        }
        *q = c;
    }
    {
        /* FAKE: the address object for flag blocks 1 and 2, mechanism: as
         * above -- a second allocno is what lets $a0 carry blocks 1-2 while
         * $v1 carries block 0.  lever-exhaustion: as above. */
        u8 *r = &D_80106A73;

        v = *r;
        c = p[8] & 2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *r = c;

        r = &D_80106A73;
        v = *r;
        c = p[8] & 4;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *r = c;
    }

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = *((u8 *)p + i + 0x17);
    }
}
