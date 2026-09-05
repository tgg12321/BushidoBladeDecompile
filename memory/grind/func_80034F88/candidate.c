/* s58 NOTE (forensics, 2026-09-05): this body is still the best-SCORING form
 * (49 insns, honest floor 10) and is kept as candidate.c for that reason, but it
 * is NOT the structurally closest one any more.  s58 built
 * rejected/s58-twoalias-ORDER-EXACT-3cycle-rotation-49insn-score15.c, which
 * reproduces the target's ENTIRE instruction stream in shape AND order (block
 * 1's `la` above block 0's store, plus the block-0 reload) and whose only
 * residual is a 3-cycle register rotation.  It scores 15 because the metric
 * prices register substitutions, not structure.  Start RA work from THAT body;
 * see evidence.md E58.1-E58.8 and hypotheses.md s58.  s58 also ablated the
 * FAKE-annotated dead re-set below: the grid is flat (10/10), so the lever buys the
 * instruction shape and the divergence class, not distance.
 */
/* s57 (forensics, 2026-09-05) -- NEW CHASSIS.  Honest floor still 10, 49
 * instructions, but this body is NOT the s49..s56 body: it is the first form in
 * 57 sessions that reproduces the TARGET'S BLOCK-0 INSTRUCTION SHAPE (the
 * 80034FB4 reload) with ONE C pointer object, ZERO alias handles, and no
 * FAKE construct other than a single dead store to a local.
 * `goal_from_tgt.py classify` on it prints FIRST DIVERGENCE: RA with 11
 * register substitutions ($v1 -> $a0 x6, $a0 -> $v1 x5) and NO
 * instruction-shape difference at all.  The previous candidate (kept as
 * memory/grind/func_80034F88/candidate_s56_prera_score10.c) classified PRE-RA:
 * it was missing the reload outright.  Any RA/solver work must start HERE.
 *
 * THE PASS ATTRIBUTION, MEASURED THIS SESSION (dumps in tmp/grind/func_80034F88/s57/):
 *
 * 1. The pass that deletes block 0's reload is CSE2 -- GCC 2.7.2's second cse
 *    run (-frerun-cse-after-loop) -- NOT cse1 and NOT combine.  Ledger entries
 *    from s52/s54 said "cse forwards the store"; that is true only of the
 *    SECOND run.  Evidence: with `q = &D_80106A73;` re-assigned between the
 *    mask store and the block-0 read, f88.cse.txt still carries
 *    `(insn 31 (set (reg:QI 79) (mem:QI (reg 75))))` -- the reload is ALIVE
 *    after cse1 -- while f88.cse2.txt has rewritten it to
 *    `(set (reg:QI 79) (subreg:QI (reg 74) 0))` and combine then folds the
 *    whole pair into `(set (reg 78) (reg 74))`, i.e. a nop in the delay slot.
 *
 * 2. WHY cse1 keeps it and cse2 does not.  cse1 processes the redundant
 *    `reg75 = symbol_ref("D_80106A73")` set, and invalidating that register
 *    drops every hash entry whose MEM is addressed by it -- so the store's
 *    recorded value is gone and the following load survives.  cse1 THEN
 *    deletes that set as redundant.  By cse2 the invalidator is no longer in
 *    the stream, nothing invalidates the MEM, and the store forwards.  This is
 *    the general law for this function: AN ADDRESS-SIDE INVALIDATOR CANNOT BE
 *    FREE, because exactly the sets cse1 can delete are the ones that cost no
 *    instruction.  (Measured: rejected/s57-readdr-reassign-block0-cse2-refolds-*.c,
 *    49/10, byte-identical to the s56 candidate.)
 *
 * 3. THE FREE INVALIDATOR IS VALUE-SIDE, AND IT DOES NOT HAVE TO TOUCH THE
 *    MASK.  cse records the stored MEM in the value class of the STORE'S
 *    SOURCE pseudo (cse.c:7310-7327: `sets[i].src_elt == 0` => the dest is not
 *    recorded at all).  So splitting block 0 into three locals --
 *        raw = *q;  mv = raw & 0xF8;  *q = mv;  mv = raw;  v = *q;
 *    -- keys the MEM on `mv`, and the dead re-set of `mv` (NOT of the mask
 *    value, NOT of the reload) invalidates that class before cse2 reaches the
 *    read.  The dead set is trivially dead and flow/combine delete it, so it
 *    costs zero instructions: 49 insns, and 80034FB4's `lbu` appears.
 *    This is the invalidator the s55/s56 frontier asked for and could not name.
 *
 * 4. THE RE-SET MUST CHANGE THE VALUE CLASS.  `mv = raw;` (49/10, this body)
 *    and `mv = 0;` (49/10, tmp/.../s57/vC.c) both work; `mv = raw & 0xF8;` --
 *    the same value -- does NOT (rejected/s57-same-value-reset-*.c: the nop is
 *    back).  Dropping the re-set entirely also loses the reload
 *    (rejected/s57-split-raw-mv-v-no-reset-*.c).  The split alone is not the
 *    lever; the class-changing dead store is.
 *
 * 5. WHAT IS LEFT.  ra_solver model tmp/grind/func_80034F88/s57/model_vD.json:
 *      73 (block-0 value chain) nrefs 11 len  7 pri 47142 -> hard 3 ($v1)
 *      74 (&D_80106A73 pointer) nrefs 11 len 29 pri 11379 -> hard 4 ($a0)
 *      72 (p)                   nrefs  6 len 34 pri  3529 -> hard 5 ($a1)  [target seat]
 *    The pointer is allocated AFTER the block-0 value chain, so the value takes
 *    $v1 and the pointer takes $a0 -- the exact inverse of the target's block 0,
 *    while blocks 1 and 2 (address $a0, value $v1) already match.  The residual
 *    is therefore the SAME one-inequality seat race the ledger has carried
 *    since s49, but now on a chassis with the target's instruction stream and
 *    no FAKE alias objects.
 *
 * FAMILY.  The single `mv = raw;` is a dead store to a LOCAL -- the sanctioned
 * dead-store family (.claude/rules/dead-store-fake-exception.md), annotated
 * below.  No pointer alias object beyond the one `q` the ledger has always
 * carried; the BANNED multi-handle axis is untouched.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;

    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;

    {
        s32 raw;
        s32 mv;
        s32 v;
        s32 c;

        raw = *q;
        mv = raw & 0xF8;
        *q = mv;
        /* FAKE: dead re-set of the stored-value local, mechanism: it changes
         * reg mv's quantity so cse2 (-frerun-cse-after-loop) cannot forward
         * the preceding sb into the following lbu (cse.c:7310-7327 keys the
         * stored MEM on the source pseudo's value class); the set is
         * trivially dead and flow deletes it, so it costs no instruction.
         * lever-exhaustion: address-side invalidators measured in s57
         * (rejected/s57-readdr-reassign-block0-cse2-refolds-*.c) and the
         * split-without-re-set and same-value-re-set controls both lose the
         * reload; see the s57 header above and hypotheses.md s57. */
        mv = raw;
        v = *q;
        c = p[8] & 1;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 4;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
