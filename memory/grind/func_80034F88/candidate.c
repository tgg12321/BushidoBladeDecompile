/* s53 SECOND PASS (synthesis, 2026-09-05) -- BODY UNCHANGED (honest floor 10,
 * 49 insns, re-measured on HEAD this session: score 10, 49/49).  The first s53
 * pass's note follows below; its measurements stand.  Four things are new:
 *
 * A. THE TARGET'S BLOCK-0 RELOAD COSTS ZERO INSTRUCTIONS.  This body emits a
 *    LOAD-DELAY NOP at exactly the site where the target emits its block-0
 *    reload (build 4f64 nop <-> target 80034FB4 `lbu $a0,0($v1)`), between the
 *    `lw $v0,0x20($a1)` and the `andi`.  The s51 split-spelling body has no nop
 *    there -- its reload fills the same slot -- so that body's ENTIRE 2-insn
 *    excess (51 vs 49) is the block-0 `lui/addiu` pair, and every other
 *    instruction it emits, flag blocks 1 and 2 and the trailing loop included,
 *    matches the target register-for-register.  The residual is therefore
 *    exactly ONE ADDRESS MATERIALISATION, not a seat race and not a reload.
 *
 * B. AN EXPLICIT BLOCK-0 READ IS INERT HERE.  `v = *q;` with both arms
 *    consuming `v` builds byte-identically to this body (49/10): cse folds it
 *    onto the masked pseudo and combine erases the zero_extend.
 *
 * C. `do { <mask> } while (0);` COSTS ONE NOP AND DOES NOT PRODUCE THE RELOAD
 *    on this chassis (50/12): the wrap displaces `move a1,v0` out of the mask
 *    lbu's delay slot, and cse2 (after_loop non-zero, so the cse.c:8054
 *    NOTE_INSN_LOOP_END break does not apply) re-forwards the read.
 *
 * D. THE SEAT AND THE RELOAD ARE SEPARATELY CONTROLLABLE.  The wrap PLUS the
 *    explicit read seats the address pointer on hard 3 ($v1) -- the target's
 *    block-0 seat, with block 0's store emitted as `sb $v0,0($v1)` exactly as
 *    at target 80034FD0 -- with NO duplicated store and NO reg_n_refs priority
 *    lift, at 51/20, and the read still forwarded.  Three measured points:
 *        split spelling   51/12   reload survives, seat $a0
 *        wrap + read      51/20   seat $v1,        read forwarded
 *        this body        49/10   neither
 *    The open question is a body holding both inside 49 instructions.
 */
/* s53 (synthesis, 2026-09-05) -- BODY UNCHANGED (honest floor 10, 49 insns,
 * re-measured on HEAD this session: score 10, 49/49).  s53 is a merge pass; it
 * names the ACTUAL mechanism behind flag blocks 1 and 2's surviving re-reads,
 * which every session from s46 onward has mis-attributed, and it converts the
 * block-0 residual into a single sharply-stated impossibility-shaped gap.
 *
 * 1. THE RE-READ MECHANISM IS REG-INVALIDATION, NOT THE JOIN LABEL AND NOT THE
 *    ADDRESS SPELLING.  Blocks 1 and 2 each re-read the flag byte in this body
 *    (`lbu` survives) while block 0 does not.  s46-s52 attributed that to cse
 *    store-forwarding being broken by the control-flow join.  It is not.  The
 *    surviving reads exist because each of those blocks re-executes
 *    `q = &D_80106A73;`, and a SET of reg q makes cse invalidate every table
 *    entry containing reg q -- including the `(mem:QI (reg q))` value the mask
 *    store recorded.  PROOF (this session): take s51's split-spelling body and
 *    DELETE block 1's `q = &D_80106A73;` while leaving the join label and every
 *    other statement in place.  Block 1's `lbu` disappears and is replaced by a
 *    load-delay nop (build 4f88), exactly the way block 0's does in this body.
 *    Banked as rejected/s53-splitspell-drop-b1-reassign-b1-read-FORWARDED-
 *    49insn-score14.c (49 insns, score 14 -- the first 49-insn body in this
 *    ledger that carries a genuine non-forwarded flag-byte re-read).
 *
 * 2. THE CONSEQUENCE IS A CO-LOCATION LAW, AND IT IS EXACTLY WHAT THE TARGET
 *    VIOLATES.  Under mechanism (1) the invalidation and the address
 *    materialisation are the SAME statement, so on every form measured in 53
 *    sessions a surviving re-read at site S is accompanied by an `la` pair
 *    (lui/addiu) at site S.  The target has FOUR non-forwarded `lbu`s of
 *    D_80106A73 (80034FA0 mask, 80034FB4 block 0, 80034FD8 block 1, 80034FFC
 *    block 2) but only THREE `la` pairs (80034F98 -> $v1, 80034FC8 -> $a0,
 *    80034FF0 -> $a0): its block-0 re-read at 80034FB4 has no materialisation
 *    of its own and shares $v1 with the mask.  So the target needs an
 *    invalidation of the mask's recorded memory value that costs ZERO
 *    instructions, and no such generator has been found: an address-value
 *    change costs an la (s51 split-spelling, 51 insns/12, re-measured this
 *    session and byte-for-byte unchanged), a same-value re-assignment is
 *    deleted by cse and invalidates nothing (s52, 49/10), and cse's own
 *    src_elt==0 escapes (cse.c:7327) are libcall block / ZERO_EXTRACT dest /
 *    volatile source, all three measured shut on this target by s52.
 *
 *    Note the la's assemble identically either way: %hi(D_80106A70) ==
 *    %hi(D_80106A73) == 0x8010 and %lo(D_80106A70)+3 == %lo(D_80106A73) ==
 *    0x6A73, so `&D_80106A70 + 3` is a BYTE-IDENTICAL spelling of
 *    `&D_80106A73`.  The split-spelling cost is purely the extra la, never the
 *    la's contents.
 *
 * 3. THE PRIORITY MODEL'S LAST UNMEASURED COMBINATION IS DEAD.  s52's frontier
 *    proposed pairing g1 (q re-used as the trailing loop's base: pointer refs
 *    12, len 28, pri 12857) with h1 (block 1's condition hoisted into block 0:
 *    masked value len 9 -> 10, pri 13333 -> 12000), on the arithmetic that
 *    12857 > 12000 flips the seat with no duplicated store in blocks 1/2.
 *    Measured: 49 insns, score 26, NO flip.  h1 does not simply lengthen the
 *    masked value -- it restructures block 0's branch into `beqz/j` with the
 *    two arms split across a jump, and it pushes `p` out of $a1 into $a2, so
 *    the allocno set global_alloc sorts is not the set the model assumed.
 *    Banked as rejected/s53-g1-plus-h1-no-seat-flip-49insn-score26.c.
 *
 * 4. WHAT A FUTURE SESSION SHOULD ATTACK.  The residual is now ONE question:
 *    what ordinary-C statement, between `*q = m;` and flag block 0's read of
 *    the same byte, invalidates cse's recorded `(mem:QI (reg q))` without
 *    emitting an instruction?  Everything that invalidates reg q emits an la;
 *    everything that invalidates memory emits a store.  The two untried
 *    surfaces are (a) invalidation of the MEM entry by an aliasing write that
 *    the target already contains -- there is none between 80034FAC and
 *    80034FB4 -- and (b) a DECLARATION-level change that makes the mask store
 *    and the block-0 read two different rtx while sharing one address pseudo,
 *    which exp_equiv_p (cse.c:2051) forbids because it compares only operands
 *    and MEM_IN_STRUCT_P / RTX_UNCHANGING_P are flags, not operands.  If both
 *    are confirmed shut, the honest disposition of this function is a LADDER
 *    EXHAUSTED foreclosure record at floor 10, not another spelling pass.
 */
/* s52 (structural, 2026-09-05) -- BODY UNCHANGED (honest floor 10, 49 insns,
 * re-measured on HEAD this session).  s52 closes the block-1 reload question
 * that s50 and s51 left open, and it closes it against us.
 *
 * 1. THE PASS IS cse, AND THE GUARD IS NAMED.  Pass attribution is now
 *    dump-proven rather than argued: with an explicit re-read in flag block 0
 *    (`w = *q;` between the mask store and the arms) the QI memory reference
 *    count in the cc1 dumps goes 8 -> 7 exactly at .cse and never changes
 *    again (tmp/grind/func_80034F88/s52/cnt.py over .rtl/.jump/.cse/.loop/
 *    .cse2/.flow/.combine/.sched/.lreg/.greg).  cse_insn records the store's
 *    memory destination in the value table (cse.c:7308-7376) and the ONLY
 *    conditions that suppress that record are `sets[i].src_elt == 0`
 *    (cse.c:7327), which is reached for an in_libcall_block, for a
 *    ZERO_EXTRACT / SIGN_EXTRACT SET_DEST (the bit-field carve-out at
 *    cse.c:7004-7027), or for a volatile source.  Nothing a value-side source
 *    spelling can do reaches any of them.
 *
 * 2. THE VALUE-SIDE SPELLINGS ARE MEASURED SHUT.  The re-read survives cse's
 *    substitution only as `zero_extend(subreg:QI(m))`, which combine then
 *    folds to `m` whenever m is provably 8-bit clean -- and every spelling
 *    that breaks that proof costs a DIFFERENT instruction, never an lbu:
 *    s32 re-read 49/10 (folded to nothing), u8 re-read 51/19 (materialises as
 *    `andi $x,$y,0xff`, not a load), u16 re-read 49/10, `m &= ~7` 49/10,
 *    `(s8)(m & 0xF8)` 53/15 (sll/sra pair).  A second, signed-char write
 *    handle (BANNED two-object shape, measured as diagnosis only) is also
 *    49/10.
 *
 * 3. THE BIT-FIELD ESCAPE IS MEASURED SHUT.  Declaring D_80106A73 as a
 *    bit-field record -- the one non-volatile route cse.c itself names -- does
 *    not produce a ZERO_EXTRACT dest on this target: GCC 2.7.2 MIPS has no
 *    insv, so store_bit_field expands to word arithmetic.  The .rtl dump
 *    contains ZERO `zero_extract` and ZERO `mem:QI` (all accesses become
 *    SImode lw/sw), and the two natural bit-field bodies score 26/47 and
 *    35/40.  So on a SHARED address the target's 80034FB4 reload has no
 *    ordinary-C generator on this chassis; s51's split-spelling body remains
 *    the only generator and it costs the extra la pair (51/12).
 *
 * 4. THE SEAT RACE IS NOW A NUMERIC MODEL WITH BOTH SIDES PINNED.  Seven
 *    duplicated-store placements were measured with refs/live_length/priority
 *    read from .lreg/.greg before the score.  The pointer allocno must exceed
 *    the masked value's fixed pri 13333 (refs 6, len 9):
 *        b0 dup only            refs 11 len 27 -> 12222   NO FLIP   46/14
 *        b0 + b1 dup            refs 12 len 26 -> 13846   FLIP      47/19
 *        all three dup          refs 13 len 25 -> 15600   FLIP      48/21
 *        b0 one-arm + b1b2 dup  refs 11 len 24 -> 13750   FLIP      48/21
 *        q reused as loop base  refs 12 len 28 -> 12857   NO FLIP   49/17
 *    and block 0 CANNOT supply the lift on its own, because cse deletes the
 *    else arm's `*q = m;` as a same-value re-store (d2 emits a one-armed
 *    `beqz / ori / sb`, 46 insns).  Every measured flip therefore buys the
 *    target's block-0 seat with a duplicated store in flag block 1 or 2,
 *    which is exactly where those blocks' bytes already match.
 *
 * 5. LIVE LENGTH IS INVARIANT UNDER SOURCE STATEMENT ORDER.  Hoisting flag
 *    blocks 1 and 2's condition above their `q = &D_80106A73;` leaves the
 *    pointer at len 27/28 and is byte-neutral (49/10, identical to this
 *    body); sched1 re-establishes the order.  The one placement that DID move
 *    a live length -- hoisting block 1's condition into block 0, masked value
 *    len 9 -> 10, pri 13333 -> 12000 -- costs an extra simultaneously-live
 *    value that pushes `p` out of $a1 into $a2 (49/25).  Two more insns of
 *    length would reach pri 10000 and flip the seat; no shape that adds them
 *    without adding a live value has been found.
 */
/* s51 (structural, 2026-09-05) -- BODY UNCHANGED (honest floor 10, 49 insns).
 * s51 re-scopes the residual again; read this before the s50 note below.
 *
 * THE RESIDUAL IS A TWO-CONDITION RACE, AND THE TWO CONDITIONS ARE MUTUALLY
 * EXCLUSIVE ON EVERY BLOCK-0 SHAPE MEASURED.  The &D_80106A73 allocno reaches
 * the target's block-0 seat (hard 3 = $v1) only if it is BOTH (i) sorted ahead
 * of the block-0 masked value by global.c allocno_compare AND (ii) free of
 * hard 3 in its conflict row.  This body buys (ii): the two-statement mask
 * expands the load as a zero_extend straight into the masked-value pseudo, so
 * block 0 holds no extra quantity and the pointer's conflict row is clean --
 * but the masked value then has refs=6 len=9 pri=13333 against the pointer's
 * refs=10 len=28 pri=10714, so the masked value is allocated first and takes
 * $v1.  Every shape that cuts the masked value to refs=4 (single-statement
 * mask `m = *q & 0xF8;`, or a named `raw = *q; m = raw & 0xF8;`) buys (i) --
 * the pointer really does sort first, `;; regs to allocate: 73 78 81 85 75 74`
 * -- but each does it by creating a short-lived block-0 quantity that
 * local-alloc, which runs BEFORE global_alloc, seats at $v1, putting hard 3
 * into the pointer's conflict row and losing (ii).  s44's seat law explains
 * the seat: a block-0-confined quantity takes $v0 when $v0 is dead across its
 * span and $v1 otherwise, and $v0 here still carries func_80077D00's return
 * value.  So the next lever is NOT a second allocno (s50's framing): it is
 * either a refs<=5 block-0 shape that spawns no pseudo, or making $v0 dead
 * across the temp's span.
 *
 * ALSO SETTLED IN s51: the target's block-1 reload (`lbu` at 80034FB4, where
 * this body emits a load-delay nop) is a pure cse address-equality effect.
 * Spelling the mask's address `(&D_80106A70 + 3)` and flag block 0's
 * `&D_80106A73` -- the same byte, two SYMBOL_REFs cse cannot equate -- emits
 * that reload and reproduces the target's whole block-0 instruction sequence,
 * at the price of one extra la pair (51 insns, score 12; banked as
 * rejected/s51-splitspell-mask-A70p3-EMITS-TARGET-RELOAD-51insn-score12.c).
 * It needs no volatile and no aliasing story.  Conversely the address SPELLING
 * on its own is codegen-NEUTRAL on this chassis (10/49 in every placement),
 * which voids the s16/s17/s25 "spelling is not a free dial" kills.
 */
/* s50 (rederive, 2026-09-05) -- BODY UNCHANGED from s49 (honest floor 10,
 * 49 insns), but the 50-session-old seat residual is now SOLVED as a mechanism
 * and re-scoped as a DIFFERENT problem.  Read this header before probing.
 *
 * 1. THE SEAT IS NO LONGER THE PROBLEM.  s49 left the residual as a single
 *    global.c allocno_compare comparison: reg 74 (the block-0 masked value,
 *    refs=6 len=9 pri=13333) outranks reg 75 (the &D_80106A73 pointer allocno,
 *    refs=10 len=28 pri=10714), so 74 is allocated first, takes hard 3 ($v1),
 *    and 75 falls to hard 4 ($a0) -- the inverse of the target's block-0 seat.
 *    s49 banked that as an instance kill (H49.3).
 *
 *    s50 OVERTURNS that kill.  Duplicating the flag-block store into BOTH arms
 *    of flag blocks 1 and 2 --
 *        if (c) { *q = v | 2; } else { *q = v; }
 *    instead of computing into `c` and storing once at the join -- adds one
 *    reference to the pointer allocno per block.  Measured on the s49 chassis:
 *        reg 75: refs 10 -> 12, live_length 28 -> 26, pri 10714 -> 13846
 *    which is above reg 74's 13333, so global_alloc's sort order becomes
 *    ";; regs to allocate: 73 77 75 74 ..." and the dispositions become
 *        75 in 3   74 in 4
 *    i.e. THE TARGET'S BLOCK-0 SEAT.  The emitted block 0 is then
 *        la $3,D_80106A73 / lbu $4,0($3) / move $5,$2 / andi $4,$4,0xf8 /
 *        sb $4,0($3) / lw $2,32($5) / andi $2,$2,1 / bne / ori $2,$4,1 /
 *        move $2,$4 / sb $2,0($3)
 *    which is register-for-register the target's 80034F98..80034FD0, the first
 *    time in this ledger.  (rejected/s50-dupstore-blocks12-*, score 18.)
 *    The lever is the SANCTIONED duplicated-statement-into-arms family, whose
 *    rule doc names exactly this mechanism ("the effect is a reg_n_refs
 *    priority lift", cross-jump re-merging the copies).
 *
 * 2. WHY THE SCORE GOT WORSE ANYWAY, AND WHAT THE REAL RESIDUAL IS.
 *    The target holds &D_80106A73 in TWO hard registers:
 *        $v1  for the mask + flag-block-0 (80034F98 .. 80034FD0)
 *        $a0  for flag blocks 1 and 2     (80034FC8 .. 8003503C)
 *    A body with ONE declared pointer object has ONE allocno for that address
 *    and therefore ONE hard register.  Both attainable seats were measured this
 *    session on the current chassis:
 *        hard 4 ($a0)  = this body, score 10, flag blocks 1/2 exact, block 0 wrong
 *        hard 3 ($v1)  = dupstore lift, score 16-18, block 0 exact, blocks 1/2 wrong
 *    So the floor-10 residual is NOT an allocation-priority problem any more.
 *    It is: "produce a second allocno holding &D_80106A73, live only over the
 *    mask + flag-block-0 region, without declaring a second C pointer object"
 *    (the second-object axis is CLOSED by Judge ruling).
 *
 * 3. THE TWO-OBJECT AXIS DOES NOT TRIVIALLY WIN EITHER (diagnosis only; every
 *    such body is banned and is banked in rejected/ with a BANNED tag).  Six
 *    two-object bodies (t for mask+flag0, q for flags 1/2), with and without
 *    the refs lift on either object, measure 21..26 -- never below the
 *    single-object 10.  Reason, from .greg: with two objects the priority order
 *    among {t, m, q} is wrong -- pri(t) is 3333..4545 (refs 4..5, len 22..24)
 *    against pri(m) 13333 -- so m is still allocated first and still takes hard
 *    3, and t lands on hard 4.  Reaching the target from two objects would need
 *    pri(t) > 13333, i.e. refs(t) >= 13 at len 22.  So the standing ban is NOT,
 *    on present evidence, the thing that is costing the 10 points; do not file a
 *    ruling-request on the strength of the ban alone.
 *
 * 4. WHAT ELSE DIED THIS SESSION.  Direct-symbol block 0 rebuilt on the s49
 *    chassis (rejected/s50-anon-direct-*) is 25..30, because a plain
 *    `D_80106A73` reference compiles to the `lbu $x,D_80106A73` assembler macro
 *    (2 insns via $at) and never to the target's lui/addiu/lbu register form --
 *    so the second address allocno cannot come from a direct symbol reference.
 *    Hoisting block 0's condition above the mask shortens reg 74's live range
 *    9 -> 7 (pri 13333 -> 17142), the wrong direction (score 14).
 *
 * Ordinary C: one declared pointer object `q`, three re-assignments, no FAKE
 * construct, no pun other than the pre-existing `*(&D_80106A70 + i)` loop line
 * (a DECLARATION-level issue, unchanged from every prior candidate; the DATA
 * MODEL signal says D_80106A70 should be declared as a 4-byte array/record and
 * that is the untried structural lead -- see hypotheses.md s50 frontier).
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m &= 0xF8;
    *q = m;

    {
        s32 c;

        c = p[8] & 1;
        if (c) {
            c = m | 1;
        } else {
            c = m;
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
