/* s61 (structural, 2026-09-05) -- FLOOR 9.  First improvement in 60 sessions.
 *
 * The 60-session floor of 10 is broken by spending the Judge's TWO-OBJECT
 * grant (decisions.md constraint: "At most TWO C pointer objects may alias
 * &D_80106A73 (one carrying the mask + flag block 0, one carrying flag blocks
 * 1 and 2), each declared with a direct `= &D_80106A73` initializer, each
 * carrying its own /* FAKE ... *​/ annotation at the declaration, with neutral
 * names") ON TOP OF the s59 aggregate declaration + the s60 value model.  The
 * frontier note from s60 said the s55b two-object measurement was void because
 * it had been taken on the old scalar-symbol chassis, where the address
 * allocnos additionally carried a local-alloc hard-reg-3 conflict.  Redone on
 * the aggregate chassis, it measures 9, not the 15/17 s55b saw.
 *
 * ============================ REQUIRED DECLARATION =========================
 * Unchanged from s59/s60.  This body does NOT compile against HEAD's header;
 * it needs one header edit plus two element-form use-site edits in the other
 * TU (an integration handoff; reverted at session end so the tree is clean):
 *
 *     include/code6cac.h:472
 *       -  extern u8 D_80106A70;
 *       +  extern u8 D_80106A70[4];
 *     src/code6cac.c:340   D_80106A70 = 0x11;  ->  D_80106A70[0] = 0x11;
 *     src/code6cac.c:345   (D_80106A70 & 0xF)  ->  (D_80106A70[0] & 0xF)
 *
 * s59 measured the declaration byte-neutral project-wide; s60 and s61 re-used
 * it across ~35 further builds without incident.
 *
 * ============================ WHAT IS LEFT (9 of 49) =======================
 * Blocks 1 and 2 and the trailing loop are now BYTE-EXACT, in the target's
 * order, including the target's la-before-the-block-0-store hoist at
 * .L80034FC8.  Every one of the nine residual instructions is inside block 0:
 *
 *   ours                       target
 *   lui   $a2                  lui   $v1          <- address seat
 *   addiu $a2                  addiu $v1
 *   lbu   $v1, 0($a2)          lbu   $a0, 0($v1)  <- value seat
 *   andi  $v1, $v1, 0xF8       andi  $a0, $a0, 0xF8
 *   sb    $v1, 0($a2)          sb    $a0, 0($v1)
 *   nop                        lbu   $a0, 0($v1)  <- the block-0 reload
 *   ori   $v0, $v1, 1          ori   $v0, $a0, 1
 *   addu  $v0, $v1, $zero      addu  $v0, $a0, $zero
 *   sb    $v0, 0($a2)          sb    $v0, 0($v1)
 *
 * i.e. exactly TWO defects remain: (1) the block-0 address object `q` and the
 * block-0 value `u` hold each other's target hard registers, and (2) the
 * block-0 reload is still folded away by cse2 (the load-delay nop stands in
 * its place, which is why the instruction count is 49 either way).
 *
 * ============================ DEFECT 1 IS A PRIORITY ORDER =================
 * Measured with tools/ra_solver/extract.py on THIS body
 * (tmp/grind/func_80034F88/s61/C.model.json):
 *
 *   ord pseudo  hardreg  nrefs livelen  pri
 *    0    73 i     $v1      11     7    47142
 *    1    77 u     $v1       8    10    24000     <- takes $v1 before q is seen
 *    2    75 c     $v0      15    19    23684
 *    3    74 v     $v1       6    10    12000
 *    4    81 r     $a0       6    19     6315     <- target seat, correct
 *    5    72 p     $a1       6    34     3529     <- target seat, correct
 *    6    76 q     $a2       4    28     2857     <- allocated LAST
 *
 * The conflict graph ALREADY ADMITS the target assignment: 76 does not
 * conflict with 73 or 74, and 77 does not conflict with 81, so {q,v,i} may all
 * sit in $v1 and {u,r} may both sit in $a0.  Nothing but the allocation ORDER
 * prevents it -- if 76 were reached before 77, find_reg would hand 76 $v1
 * (73's seat, no conflict) and 77 would then be pushed to $a0 (81's seat, no
 * conflict), which is precisely the target.  tools/ra_solver/inverse.py agrees:
 * goal {"76": 3} has a ONE-ATOM solution, `[refs_down] pseudo 77: refs 8->2`
 * (the only other atom, calls_crossed on 77, is semantically unreachable --
 * block 0's value is computed after the only call).
 *
 * FAMILY.  Two pointer objects, each initialised directly from the aggregate,
 * neither copied from the other, each annotated -- the exact shape the Judge
 * granted.  `u`/`v`/`c` are ordinary locals; no dead store, no pad, no pun.
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
        u8 *q = &D_80106A70[3];
        s32 u;

        u = *q;
        u = u & 0xF8;
        *q = u;
        u = *q;
        c = p[8] & 1;
        if (c) {
            c = u | 1;
        } else {
            c = u;
        }
        *q = c;
    }
    {
        /* FAKE: the address object for flag blocks 1 and 2, mechanism: as
         * above -- a second allocno is what lets $a0 carry blocks 1-2 while
         * $v1 carries block 0.  lever-exhaustion: as above. */
        u8 *r = &D_80106A70[3];

        v = *r;
        c = p[8] & 2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *r = c;

        r = &D_80106A70[3];
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
