/* s63 (synthesis, 2026-09-05) -- FLOOR 9 (unchanged; chassis re-measured 9,
 * 49/49, this session).  The BODY below is unchanged from s62 (it is still the
 * lowest-scoring form on record).  What changed is the DIAGNOSIS, and it is
 * the sharpest it has ever been -- read this header before touching the C.
 *
 * INTEGRATION HANDOFF (unchanged from s62): this body needs the split
 * declaration in include/code6cac.h -- `extern u8 D_80106A70[3];` absorbing
 * D_80106A71/D_80106A72 (their two consumers in src/code6cac.c converted to
 * element form) with `extern u8 D_80106A73;` left as its own scalar.  Measured
 * byte-neutral again this session (the apply script is
 * tmp/grind/func_80034F88/s63/apply.py).
 *
 * 1. THE RESIDUAL IS EXACTLY A 3-CYCLE REGISTER ROTATION, AND A 49/49 BODY
 *    THAT REPRODUCES THE TARGET'S WHOLE INSTRUCTION STREAM EXISTS.  Add the
 *    s57-family cse2 invalidator (a dead re-set of the stored-value local
 *    between block 0's store and its reload) to this body and the build
 *    reproduces the target's entire 49-instruction stream, instruction for
 *    instruction, INCLUDING the block-0 reload in the load-delay slot of the
 *    flag lw and block 1's la in its target position.  The only difference is
 *    a 3-cycle register rotation, worth 15 engine points but exactly three
 *    register substitutions:
 *        ours          target        role
 *        $a1     ->    $v1           block-0 address object (`q`)
 *        $v1     ->    $a0           block-0 value
 *        $a2     ->    $a1           `p` (the func_80077D00 result)
 *    Blocks 1-2's address ($a0) and value ($v1), the loop index ($v1) and the
 *    $v0 temp already match.  The 9-point score of the body below is a
 *    DIFFERENT, structurally farther state (no reload, load-delay nop); the
 *    15-point rotation body is the one to grind.  It is banked at
 *    rejected/s63-mask-in-blocks12-local-BEST-RA-49insn-score15.c.
 *
 * 2. THE WHOLE ROTATION IS ONE SEAT: block 0's ADDRESS allocno must reach $v1.
 *    Simulated on the extracted model: if it does, every other register in the
 *    function falls into the target seat by find_reg's own ascending scan
 *    (value -> $a0, p -> $a1).  Nothing else needs to move.
 *
 * 3. THE s63 LAW -- BLOCK 0's MASK VALUE TAKES $v1 AHEAD OF THE ADDRESS BY
 *    THREE DIFFERENT MECHANISMS, AND THE C SPELLING ONLY CHOOSES WHICH.
 *    Measured this session, three ways, all 49 insns / score 15:
 *      (a) mask + reload in ONE local  -> one value allocno, 7 refs / livelen
 *          8 / pri 17500, allocated at ord 2 and takes $v1 on PRIORITY.
 *          (hard conflicts of the address object stay clean: [2, 29].)
 *      (b) mask in its OWN local, reload in a second local -> the value
 *          allocno is 3 refs / livelen 4 / pri 7500 (the ref cut works!) but
 *          the mask becomes a BLOCK-LOCAL quantity that local-alloc seats in
 *          $v1, which re-adds hard reg 3 to the address object's conflicts
 *          ([2, 3, 29]).  LOCAL-ALLOC takes the seat.
 *      (c) mask carried in the SAME local blocks 1-2 use -> no block-local
 *          quantity at all (address hard conflicts back to [2, 29]) and the
 *          value allocno stays 3 refs / livelen 4, but the mask carrier is now
 *          allocno 74 (10 refs / pri 23076), it CONFLICTS with the address
 *          object, and it takes $v1 at ord 1.  A CONFLICT takes the seat.
 *    (a)(b)(c) are the whole spelling axis for the mask.  Every one of them
 *    puts something in $v1 before the address object gets its turn.
 *
 * 4. THE TARGET SEATS ITS MASK VALUE IN $a0 (80034FA0-80034FAC: lbu $a0 /
 *    andi $a0 / sb $a0), i.e. the target is chassis (b) -- a separate mask
 *    carrier, block-local, and LOCAL-ALLOC gave it $a0, not $v1.  So the one
 *    unanswered question in this function is now a local_alloc question and
 *    not a global.c question:
 *
 *      why does block_alloc's find_free_reg pick $a0 over $v1 for block 0's
 *      mask quantity in the original, when on our (b) chassis it picks $v1?
 *
 *    find_free_reg's `used` set is fixed_reg_set | union(regs_live_at
 *    [birth..death]); $v0 is already excluded on our chassis (the
 *    func_80077D00 return value is still live across block 0's lbu, which is
 *    why every extracted address allocno carries hard conflict 2).  Either
 *    something makes $v1 live across the mask window in the original, or the
 *    quantity is taking a SUGGESTED register through qty_phys_copy_sugg /
 *    qty_phys_sugg (the pass local_alloc.py reports but does not score).
 *    tools/ra_solver/local_extract.py --suggest dumps exactly those rows.
 *
 * 5. WHAT THE INVERSE SOLVER SAYS ABOUT (c) (tmp/grind/func_80034F88/s63/
 *    inverse_z2.txt): minimum 2 atoms, and every emitted vector pairs
 *    "calls_crossed 74: 0->1" with "refs_up 76: 5->8..15".  Both atoms are
 *    expensive-to-impossible in C here; (c) is therefore the wrong chassis to
 *    grind even though its RA state looks the cleanest.  Grind (b), and grind
 *    it at local-alloc.
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
