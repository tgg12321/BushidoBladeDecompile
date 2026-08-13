/*
 * func_80034F88 — best HONEST form as of grind session s3 (structural).
 * Honest sandbox floor (`sandbox func_80034F88 --disable all`): **18**
 *   (s1 best 23 / 50 insns; s2's 12 is DISCARDED — see "the volatile ruling"
 *    below; the committed baseline is 24 and carries three forbidden
 *    `asm volatile("" ::: "memory")` scheduling barriers.)
 * Build: 51 insns against a 49-insn target.  The 49-insn spelling of this same
 * form (`u8 val` instead of `s32 val`) scores 20 — fewer instructions, worse
 * distance; the score is what counts.
 *
 * =====================================================================
 * WHY THIS FORM (three measured levers, all volatile-free)
 * =====================================================================
 * 1. `u8 *ptr = &D_80106A73; *ptr &= 0xF8;` — a C-level pointer alias to a
 *    global (FROZEN SOTN-accepted family).  The read-modify-write gives the
 *    address expression TWO memory uses, so `combine` cannot fold `%lo` into
 *    either one and the target's unfolded `lui`+`addiu` base survives; it also
 *    mismatches the following symbol read of the same byte, so the first `lbu`
 *    reload materialises.  Spelling the mask as plain `D_80106A73 &= 0xF8;`
 *    costs 9 points (29 at the same insn count) because the base is then folded
 *    away and the reload disappears with it.
 * 2. Condition BEFORE the flag-byte read (`c = p[8] & K;` then
 *    `val = D_80106A73;`).  Worth 1 point and 1 insn over the s1 ordering
 *    (23/50 -> 22/49): the `lw v0,0x20(a1)` issues first, so its load-delay slot
 *    can be filled instead of stranding work behind it.  This lever was found in
 *    s2 on the volatile form and had never been applied to the non-volatile one.
 * 3. Copy loop spelled `*((u8 *)p + i + 0x17)` — the induction variable BEFORE
 *    the constant displacement.  Worth 2 points (22 -> 20): it emits
 *    `addu v0,a1,v1` (p + i) in target's operand order, where
 *    `*((u8 *)p + 0x17 + i)` emits `addu v0,v1,a1`.  The pair appears twice (the
 *    loop body and the branch delay slot), hence 2 points.  s1/s2 recorded the
 *    copy loop as "already matching in every form measured" — that was wrong.
 * 4. `s32 val` for the loaded byte (with `u8 val2` for the selected value).
 *    Worth 2 more points (22 -> 20 -> 18) at the cost of 2 instructions: the
 *    read becomes a `(zero_extend:SI (mem:QI))` and an explicit
 *    `andi a0,v1,0xff` truncation appears before each store, which lengthens the
 *    byte's live range across the block boundary and changes which value each
 *    block's `ori`/`move` pair reads.  s1 killed "int-typed temporaries" at 28,
 *    but that was measured before any of levers 1-3 existed; the type of the
 *    loaded byte only pays once the address and ordering shape are right.
 *    Making `val2` `s32` as well gives the gain straight back (20).
 *
 * =====================================================================
 * THE REMAINING 18 POINTS, AND THE VOLATILE RULING THAT GATES THEM
 * =====================================================================
 * All 18 are in the three flag blocks.  The target reads the flag byte back
 * from memory FOUR times, each `lbu`/`sb` pair sharing one unfolded base
 * register (`lbu a0,0(v1)` immediately after `sb a0,0(v1)` — it even reloads
 * into the register that already holds the stored value).  s3 established from
 * the compiler source that this is not reachable without a volatile access:
 *   - `cse.c:7329` skips recording a SET_DEST when `sets[i].src_elt == 0`, and
 *     `canon_hash` (cse.c:1941-1947) sets `do_not_record` for a MEM only when
 *     `MEM_VOLATILE_P` is set.  The other `do_not_record` triggers (PRE/POST
 *     inc-dec, PC, CC0, CALL, UNSPEC_VOLATILE, volatile ASM_OPERANDS, and
 *     hard-reg under SMALL_REGISTER_CLASSES) are unreachable from C here.
 *   - Two C address expressions for the same byte either hash the same (cse
 *     merges the pseudos -> one base -> the store forwards into the load and the
 *     reload dies) or hash differently (no forward, but two `lui`s and `%lo`
 *     folded into each single-use mem).  Aliasing expressions such as
 *     `(&D_80106A70)[3]` would additionally be treated as a distinct object by
 *     GCC's alias analysis and MISCOMPILE the read-back, so they cannot be what
 *     the original source did either.
 * s2's floor of 12 came from `volatile u8 *pbit = &D_80106A73;` on a global
 * declared plain `extern u8`.  s3 rules that construct OUT: it is
 * `engine/volatile_cheats.py` pattern 2 (`*(volatile T *)&D_globalsym` on a
 * game-RAM global) respelled in two steps, and the detector's silence is a
 * regex gap, not a sanction.  The sanctioned spelling —
 * `extern volatile u8 D_80106A73;` plus the `volatile_extern_allowlist.txt`
 * grant — was researched and FAILS BOTH PRONGS of
 * `.claude/rules/legitimate-volatile-interrupt-touched.md`: no IRQ/VSync writer
 * exists (all six functions that touch the byte are ordinary synchronous game
 * logic; the only one that stores it, func_800167EC, is called from
 * src/ings.c:414, not installed as a callback), and the use-site shape
 * (store-then-readback) is not on the rule's exact three-shape list.
 * See `rejected/volatile-ptr-coercion-score12.c` and hypotheses.md s3-H1..H4.
 *
 * =====================================================================
 * s4 (permuter modality) — form UNCHANGED, and now defended on two more axes
 * =====================================================================
 * s4 did not move the floor and did not change a character of this body.  What
 * it did was close the two axes that could have replaced it:
 *  - decomp-permuter ran on this function for the first time: 50,425 iterations
 *    across two structurally different chassis (this 51-insn `s32 val` form,
 *    34,003 iters / 30 finds; and the 49-insn `u8 val` form, 16,422 iters /
 *    67 finds).  Every find was re-scored with the honest sandbox; the best
 *    honest, semantically-correct find on either seed TIES this form at 18.
 *    The 49-insn chassis converged onto this one (its 18-point finds are
 *    51-insn `s32`-widened forms), so the two are a single basin.
 *  - The four spellings s3 listed as "untried" on frontier F1 are measured:
 *    per-block temporaries 30, copy loop through `ptr` 31, third block storing
 *    via `*ptr` 22, `(u8)`-cast staging 18 (neutral), block-1-read-first 18
 *    (neutral).  Banked in rejected/.
 * The two pointer results sharpen WHY lever 1 above is spelled the way it is:
 * giving `ptr` any use beyond the single read-modify-write keeps its address
 * pseudo live further down the function, cse then merges it, and the target's
 * per-block `lui`+`addiu` rematerialisations vanish.  ONE read-modify-write use
 * is load-bearing in both directions — two memory operands so `combine` cannot
 * fold `%lo`, and no third operand so the pseudo dies before block 2.  Do not
 * "clean up" that pointer, and do not give it another use.
 * NB (s4): the permuter produced exactly one sub-floor number, a 17, and it is
 * a MISCOMPILE (the third block's store hoisted inside the `if` arm) — see
 * rejected/permuter-17-store-hoisted-into-arm-MISCOMPILES.c.  The sandbox
 * scores bytes, not behaviour.
 *
 * =====================================================================
 * s5 (permuter modality, DIRECTED) — form UNCHANGED; three of the levers
 * above are now known to be free, and two more are proven load-bearing
 * =====================================================================
 * s5 ran the permuter's OTHER mode (manual mutation via PERM_GENERAL, random
 * mutation disabled) as an exhaustive enumerator, and scored the same two
 * cross-products deterministically with the honest sandbox: 1,008 forms in
 * total (864 + 144), plus 1,152 permuter iterations across the same spaces.
 * Best = 18 in both waves. The only campaign output (permuter score 1290 vs a
 * base of 1300 — the permuter ranked it BETTER) sandbox-scores 18, a tie; see
 * rejected/directed-perm-mixed-condform-score18-TIE.c.
 * What changed in how this body must be READ:
 *  - Lever 2 above (condition BEFORE the flag-byte read) is NO LONGER A LEVER.
 *    All eight per-block orderings score 18. It was worth 1 point at floor 23
 *    and was absorbed by lever 4's `s32 val` widening. Spell it however reads
 *    best; do not defend it.
 *  - `p = func_80077D00();` BEFORE `ptr = &D_80106A73;` is load-bearing and was
 *    never previously probed: assigning the pointer first costs a MINIMUM of
 *    +11 (29 over all 432 such forms). Do not tidy that order.
 *  - Lever 1's `*ptr &= 0xF8;` may equally be spelled `*ptr = *ptr & 0xF8;`
 *    (both 18). What defeats combine's `%lo` fold is the address expression
 *    having TWO memory operands, not the compound assignment.
 *  - `s32 val` may equally be `u32 val` (both 18); `u8 val` is 20. The lever is
 *    WORD WIDTH (the zero_extend), not signedness. `u8 val2` and a single
 *    word-typed `c` are both required (s32 val2 / u32 val2 / u8 c / split
 *    c1,c2,c3 all cost 1-2).
 *  - The copy loop is exactly orthogonal: `+ 0x17 + i` costs exactly +2 in all
 *    432 pairings, delta set {2}.
 *
 * =====================================================================
 * s6 (forensics) — form UNCHANGED; two of the mechanisms quoted ABOVE are
 * WRONG and are corrected here (read this before trusting levers 1 and 4)
 * =====================================================================
 * The first cc1 `-da` dump taken of THIS body (s2's dumps were of a pre-lever
 * volatile variant) corrects the model this header has carried since s1:
 *  - "combine folds `%lo` into a single-use mem" is FALSE. The pre-cse dump
 *    already contains `(mem:QI (symbol_ref "D_80106A73"))` for every plain
 *    symbol access and `(mem:QI (reg 73))` for every `*ptr` access, straight
 *    out of RTL expansion — MIPS accepts a bare symbol_ref as an address and
 *    the `lui $at` / `%lo(...)($at)` pair is manufactured at assembly-output
 *    time. No optimizer pass is involved, so lever 1 works for a DIFFERENT
 *    reason than stated: `*ptr &= 0xF8;` is spelled through a pointer, so its
 *    two accesses are `(mem (reg))`, and the following SYMBOL-spelled read
 *    hashes differently and therefore survives as a real `lbu`.
 *  - the block-2/3 reloads die by an ORDINARY cse hash hit (identical
 *    `(mem:QI (symbol_ref))` rtx on store and load), not by the
 *    `cse.c:7329 src_elt == 0` path, which is volatile-only. The two
 *    `zero_extend`-from-register insns that replace them are exactly the two
 *    `andi a0,v1,0xff` truncations, i.e. the forward is ALSO the whole 51-vs-49
 *    instruction excess.
 *  - the `$v0`/`$v1` swap of `val2` is NOT a steerable allocation tie. `.greg`
 *    shows `75 conflicts: … 2 …` (hard reg $v0) while the condition pseudo
 *    carries `preferences: 2`; the hard reg was taken by `local-alloc` for the
 *    block-local `p[8]` loads before `global_alloc` ran. Six forms that alter
 *    the live ranges (if/else both arms 24, ternary 24, positive-sense if 23,
 *    store duplicated into arms 35, val2-split 24, val-split 23, both-split 29)
 *    all keep the conflict and all score worse. Banked in rejected/.
 * Net: all three residual defect classes are downstream of ONE decision —
 * pointer-vs-symbol spelling per access, fixed at expansion — so do not look
 * for a separate register or scheduling lever.
 *
 * =====================================================================
 * s7 (forensics) — form UNCHANGED at 18, but the model above is now
 * INCOMPLETE in an important way: the target's access signature IS
 * reachable without volatile
 * =====================================================================
 * s7 attacked the two axes s6 left open and closed both, and in doing so found
 * the first non-volatile form whose flag section has the target's exact memory
 * and address signature (lbu 5 / sb 5 / lui 4 — see
 * rejected/loop1-per-block-TARGET-SIGNATURE-score36.c).
 *  - The mechanism is a cse BASIC-BLOCK BOUNDARY, and only one kind works.
 *    cse.c:8039 ends the block at a CODE_LABEL unconditionally, so a label
 *    boundary is honoured by BOTH cse passes; cse.c:8051-8056 ends it at a
 *    NOTE_INSN_LOOP_END only while `! after_loop`, so a do-while(0)/loop-note
 *    boundary works in cse1 and is then undone by cse2. Measured both ways in
 *    the RTL dumps (s7/rtl/v3_dw_one_ptr/fn/{cse,cse2}.fn).
 *    A fresh basic block flushes the value table, which simultaneously (a)
 *    stops the store being forwarded into the next block's read and (b) makes
 *    the next `ptr = &D_80106A73;` a non-redundant set, so it emits its own
 *    lui+addiu with addend 0 — the two target properties s1-s6 believed were
 *    mutually exclusive in non-volatile C.
 *  - The qty-class-breaking family (distinct symbol+addend address rtxs, which
 *    insert_regs at cse.c:1006-1042 cannot merge) also produces the reloads,
 *    but only TIES this form at 18: the non-zero LO16 addend costs exactly what
 *    each recovered reload gains. See rejected/hybrid-distinct-addend-score18-
 *    TIE.c (18), distinct-symbol-addend-four-bases-score28.c (28).
 *  - The bit-index LOOP shape — the last untried SHAPE — is dead: GCC 2.7.2 at
 *    -O2 neither unrolls nor peels, so it emits a 36-insn real loop against a
 *    49-insn unrolled target (31). rejected/loop-bitindex-shape-score31.c.
 * So do NOT read the paragraphs above as "only volatile can do this". The
 * correct statement is narrower: what is still missing is a boundary construct
 * that costs ZERO instructions.
 *
 * =====================================================================
 * s8 (rederive) — form UNCHANGED at 18; a SECOND, cheaper reload mechanism
 * exists, and the reason it cannot be used here is now proven
 * =====================================================================
 * s8 re-derived the flag section from scratch (16 structurally new shapes,
 * none a tweak of this body) and found that s7's "only a CODE_LABEL boundary
 * can produce the reloads" is INCOMPLETE.  There is a second, entirely
 * different cse path that produces all four reloads for FREE:
 *  - `if (p[8] & K) *ptr |= K;` — the store inside the conditional arm.
 *    cse_end_of_basic_block (cse.c:8102-8184) classifies the arm as AROUND,
 *    and cse_basic_block then calls invalidate_skipped_block (cse.c:7843) ->
 *    invalidate_skipped_set (7810-7836), which INVALIDATES a MEM written in a
 *    skipped block instead of recording it.  The stored value therefore never
 *    enters the value table and the next read is a real `lbu`.  Measured:
 *    score 18 at 42 insns, lbu 4 / sb 5, ONE base, no volatile, no label,
 *    and cse2.fn still carries `(mem:QI (reg/v:SI 73))` for every flag read
 *    (rtl/m1_ptr_read_condstore/fn/cse2.fn).
 *  - It is nevertheless UNUSABLE for this target, and that is a proof, not a
 *    tuning failure: the mechanism needs the store INSIDE the arm, while the
 *    target's store is unconditional — asm/funcs/func_80034F88.s carries
 *    `addu $v0,$a0,$zero` at 0x80034FC4 on the fall-through path and a single
 *    `sb $v0,0($v1)` AFTER the join label in every block.  The AROUND form is
 *    7 insns short of the target (3 missing value-select moves + 4 missing
 *    address-materialisation insns) and it cannot grow them.
 * So the two reload mechanisms are mutually exclusive on this function:
 * conditional store -> free reloads but the wrong store shape; unconditional
 * store -> the target's shape but the store is recorded, and only a
 * cse2-surviving CODE_LABEL boundary can stop the forward.
 * Also measured dead this session: the store duplicated into both arms
 * (19 at 46 insns — find_cross_jump merges only one of the three pairs,
 * leaving 7 sb), the plain-symbol conditional store (35), the read
 * duplicated into both arms (28), an explicit two-label goto diamond (22 —
 * jump.c rebuilds the ordinary diamond before cse1, so no boundary), and the
 * mask spelled through the symbol with the blocks through the pointer (33).
 */
/*
 * =====================================================================
 * s9 (rederive) - form UNCHANGED at 18; the boundary trade is now PRICED
 * =====================================================================
 * s9 measured the one frontier item s8 left open ("does the `&&` boundary pay
 * for itself?") and the answer is no, with numbers.  A cse basic-block
 * boundary IS reachable with this function's unconditional store - spelling a
 * flag block's condition `if (!c && ptrN)` gives the join label LABEL_NUSES 2,
 * which makes cse_end_of_basic_block's extension test at cse.c:8112 decline,
 * and the block ends AT the join in BOTH cse passes.  Two such boundaries
 * reproduce the target's three addend-0 `lui`+`addiu` bases and its block-2/3
 * reloads (lui 4 / lbu 4, exactly target's counts) - but each costs one
 * conditional branch, and the target has exactly one branch per block.  The
 * measured ladder: no boundary 21/47, one 23/50, two 31/53, three 33/55.
 * Separately, the target's lbu 5 / sb 5 counts are reachable with NO boundary
 * at all by spelling a block's two reads differently (symbol outside the arm,
 * pointer inside it) - 29/47, best member 24 - but then every symbol-spelled
 * access drags its own `lui $at` + `%lo(...)($at)` and the lui count goes to
 * 5-7.  Combining both (f1: two boundaries + the block-1 mismatch) gives the
 * target's complete access signature at 54 insns and scores 32.
 * So the residual 18 is now understood as a THREE-WAY trade, not a wall with
 * one missing lever: address materialisation belongs to the boundary family,
 * the reloads belong to the mismatched-spelling family, and this body - which
 * has neither signature - still wins because it is only 2 instructions over
 * the target while every signature-correct form is 5 or more.
 * A fresh m2c re-derivation (rederive mandate) was also measured this session
 * and is NOT the missing structure: m2c's own output scores 29, and its best
 * hybrid with this body's levers scores 20.
 * Do not re-derive any of this; see hypotheses.md s9 and rejected/andand-*,
 * rejected/armread-*, rejected/m2c-*, rejected/full-target-signature-*.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = D_80106A73;
    val2 = val | 1;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    val = D_80106A73;
    val2 = val | 2;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 4;
    val = D_80106A73;
    val2 = val | 4;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
