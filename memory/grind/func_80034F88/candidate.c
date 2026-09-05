/* s56 (solver, 2026-09-05) -- BODY UNCHANGED (honest floor 10, 49 insns,
 * re-measured on HEAD this session with this exact body: score 10, 49/49).
 * s56 spent the whole session inside tools/ra_solver and it converted the
 * 56-session residual from "a three-cycle register rotation" into ONE
 * arithmetic inequality over global.c's allocno_compare.  Read items 1-4
 * before proposing any register-seat lever; three of the four 1-atom solver
 * vectors are now BYTE-FORECLOSED by counting, not by opinion.
 *
 * 0. WHICH CHASSIS THE SOLVER RUNS ON.  This body (score 10) classifies
 *    PRE-RA -- it has no block-0 reload (ours has a nop where the target has
 *    an lbu), so no RA model of it can be aimed at the target.  The RA-typed
 *    chassis is s55's rejected/s55b-twoalias-TARGET-VALUE-PAIRING-49insn-
 *    score15.c: `goal_from_tgt.py classify` reports FIRST DIVERGENCE: RA on
 *    it, with exactly three substitutions, $a1->$v1 x7, $v1->$a0 x7,
 *    $a2->$a1 x6, and NO shape difference at all.  (The score-14 u8 body is
 *    NOT the right solver chassis: it classifies PRE-RA because of its
 *    `andi $v0,$v1,0xff` vs the target's `move`.)  Any future RA work on this
 *    function starts by installing the score-15 body, not this one.
 *
 * 1. THE ALLOCATION MODEL, MEASURED (tmp/grind/func_80034F88/s56/
 *    model_s15.json, from tools/ra_solver/extract.py on the score-15 body).
 *    Eight allocnos; ord = global.c allocation order, pri = allocno_compare:
 *      ord0 73 loop index      refs 11 len  7 pri 47142 -> $v1   (target $v1 OK)
 *      ord1 81 loop addr temp  refs 10 len 12 pri 25000 -> $v0   (target $v0 OK)
 *      ord2 74 mask+reload     refs  7 len  8 pri 17500 -> $v1   (target $a0 XX)
 *      ord3 76 block-0 cond    refs  7 len  9 pri 15555 -> $v0   (target $v0 OK)
 *      ord4 80 block-1/2 value refs  6 len 10 pri 12000 -> $v1   (target $v1 OK)
 *      ord5 79 handle B (q)    refs  6 len 19 pri  6315 -> $a0   (target $a0 OK)
 *      ord6 75 handle A (t)    refs  5 len 28 pri  3571 -> $a1   (target $v1 XX)
 *      ord7 72 p               refs  6 len 34 pri  3529 -> $a2   (target $a1 XX)
 *    The priority formula is EXACT on all eight rows (this is the fit, not a
 *    guess):  pri = floor_log2(nrefs) * nrefs * size * 10000 / live_length.
 *    Note the floor_log2 step: it is why nrefs 15 and nrefs 16 behave
 *    completely differently (3x vs 4x).
 *
 * 2. THE ENTIRE RESIDUAL IS ONE INEQUALITY.  find_reg gives 74 the register
 *    $v1 only because $v1 is still free when 74 is reached: 74's hard
 *    conflicts are {$v0, $ra} and its allocno conflicts are {72,75,76} --
 *    it does NOT conflict with 73, which already holds $v1.  Make $v1
 *    unavailable to 74 and everything else falls out for free (74 -> $a0,
 *    then 75 takes the vacated $v1, then 72 takes $a1).  There are exactly
 *    two ways to make $v1 unavailable:
 *      (a) give 74 a conflict with 73 (the loop index, allocated first), or
 *      (b) get 75 allocated BEFORE 74, i.e. pri(75) > pri(74) = 17500;
 *          74 already conflicts with 75, so 75 in $v1 forces 74 off it.
 *    `inverse.py global model_s15.json --goal {full 8-pseudo disposition}
 *    --depth 2` agrees: minimal solution size 1 atom, six vectors, all of
 *    them a spelling of (a) or (b) -- conflict_add(73,74) cost 2;
 *    refs_down(74) 7->2 or 7->1 cost 6/7; refs_up(75) 5->16 or 5->17 cost
 *    12/13.  No live-length atom appears because the search bound is +/-8 and
 *    75 needs len <= 5 (report: tmp/grind/func_80034F88/s56/inverse_full.txt).
 *
 * 3. THREE OF THE FOUR VECTOR FAMILIES ARE BYTE-FORECLOSED BY COUNTING.
 *    - refs_down(74) to 2 or 1.  Allocno 74's seven references are, one for
 *      one, operands the TARGET's own stream contains: lbu $a0 (1), andi
 *      $a0,$a0 (2), sb $a0 (1), the reload lbu $a0 (1), ori $v0,$a0 (1),
 *      addu $v0,$a0 (1).  Deleting a reference deletes one of the target's
 *      operands, so no byte-neutral C form can lower this count.
 *    - refs_up(75) to 16/17.  MEASURED this session, not argued: the only
 *      byte-neutral reference-adder available (duplicated-statement-into-arms
 *      with a jump2 cross_jump re-merge) does NOT re-merge here.  Duplicating
 *      block 0's `*t = c;` into both arms gives 50 insns / score 12 -- ours
 *      emits `sb; beqz; j` where the target emits `bnez` -- and buys exactly
 *      +1 ref (5 -> 6) and -4 live length (28 -> 24), pri 3571 -> 5000.  Ten
 *      more references means ten more instructions.  Banked as
 *      rejected/s56-twoalias-dup-store-into-arms-NOT-MERGED-50insn-score12.c.
 *    - conflict_add(73,74).  Requires the trailing copy loop's index to be
 *      live across block 0.  s55 spelled it (`i = 0;` hoisted above block 0,
 *      `for (; i < 3; i++)`) and measured 52 insns / score 31.  74 dies at
 *      its last arm read, which sched1 places BEFORE block 1's `la`, so no
 *      later value can be made to overlap it without a new instruction.
 *    - the fourth, pri(75) > 17500 by shortening 75's live range, needs
 *      live_length <= 5 at nrefs 5 (2*5*10000/5 = 20000).  75 is live from
 *      its `la` to block 0's store, which is the whole of block 0; the
 *      measured length is 28 and the smallest value any s55/s56 shape
 *      produced is 24.
 *
 * 4. WHAT THE DUPLICATION SHAPE DID BUY, AND WHY IT IS THE NEXT LEAD.  On the
 *    50-insn duplicated-arm shape the disposition is
 *    {72:$a1, 73:$v1, 74:$v1, 75:$a0, 76:$v0, 79:$a0, 80:$v1, 81:$v0}: p
 *    (72) has REACHED its target seat $a1 for the first time, and the
 *    conflict graph changed (75 no longer conflicts with 79, because block-0's
 *    store no longer sinks below block-1's `la`).  The residual there is the
 *    single 74<->75 swap, and on THAT model `inverse.py` reports a cheaper
 *    bar: refs_down(74) 7->3 suffices (pri 3750 < 75's 5000), not 7->2.
 *    Report: tmp/grind/func_80034F88/s56/inverse_v1.txt.  The open question
 *    the next session should ask is whether some OTHER byte-neutral shape
 *    reproduces that conflict-graph change (75 not conflicting with 79, p in
 *    $a1) at 49 instructions -- the swap bar drops with it.
 */
/* s55 SECOND PASS (synthesis, 2026-09-05) -- BODY UNCHANGED (honest floor 10,
 * 49 insns).  This pass SPENDS the 2026-09-05 11:01 Judge PASS (decisions.md
 * :23437), which narrowed the multi-handle closure to permit EXACTLY TWO
 * annotated `= &D_80106A73` alias objects on the s55 ordinary-C chassis.  The
 * grant works, and it changes the residual completely.  Read this first; the
 * closest body is now rejected/s55b-twoalias-u8-masked-value-in-a0-49insn-
 * score14.c, NOT this one.
 *
 * 1. TWO HANDLES, BLOCK-SCOPED AFTER THE CALL, REPRODUCE THE TARGET'S ENTIRE
 *    49-INSTRUCTION STREAM IN ORDER.  Handle `t` (mask + flag block 0) and
 *    handle `q` (flag blocks 1 and 2), each `u8 *X = &D_80106A73;` with a
 *    direct initializer, each declared at the head of ITS OWN inner block
 *    placed AFTER `p = func_80077D00();`.  Result: 49 insns, three la pairs,
 *    the block-0 reload, and -- for the first time in 55 sessions -- flag
 *    block 1's address materialised at 80034FC8 BEFORE flag block 0's store
 *    at 80034FD0, exactly as the target does.  Every opcode is the target's;
 *    the ONLY residual is register naming.
 *    (rejected/s55b-twoalias-blockscoped-EXACT-INSN-ORDER-49insn-score17.c)
 *    THE DECLARATION SITE IS LOAD-BEARING: initialising both handles at
 *    FUNCTION scope makes them live across the call, so they are forced into
 *    $s0/$s1 with save/restore -- 53 insns, score 35
 *    (tmp/grind/func_80034F88/s55/t1.c).
 *
 * 2. THE VALUE PAIRING IS ALSO REACHABLE.  The target pairs {masked value,
 *    block-0 reload} in one register ($a0) and {p[8], condition, result} in
 *    another ($v0).  The s54/s55 invalidation chassis pairs them the OTHER way
 *    (masked+cond+result together), because the cse invalidator must re-set
 *    the STORED value's variable.  Splitting them needs a re-set of `m` whose
 *    value nothing consumes -- a dead store to a LOCAL, the sanctioned
 *    dead-store family (`c = p[8]; m = c; m = *t;`).  With it the build's
 *    pairing is the target's: 49 insns, score 15
 *    (rejected/s55b-twoalias-TARGET-VALUE-PAIRING-49insn-score15.c).  Typing
 *    the block-0 value `u8` splits the masked value into its own allocno,
 *    which then lands on hard 4 = $a0, the target's register: score 14, the
 *    session best (rejected/s55b-twoalias-u8-masked-value-in-a0-49insn-
 *    score14.c).  Its remaining cost is that the u8 else-arm emits
 *    `andi $v0,$v1,0xff` where the target emits `addu $v0,$a0,$zero`.
 *
 * 3. THE WHOLE RESIDUAL IS NOW ONE THREE-CYCLE REGISTER ROTATION.
 *        build: t=$a1  masked=$v1  p=$a2
 *        target: t=$v1 masked=$a0  p=$a1
 *    and it is a single global.c allocation-ORDER fact, measured with
 *    tools/ra_solver/extract.py (models tmp/grind/func_80034F88/s55/
 *    model_t6.json, model_t7.json):
 *      - every block-0 VALUE carries a hard-2 conflict, because sched1 sinks
 *        the `p = $v0` call-result copy BELOW the mask store, so $v0 is live
 *        across the mask.  Blocked from $v0, the first block-0 value takes
 *        $v1 -- the seat the target gives the pointer.
 *      - the pointer allocno is nrefs 5 / livelen 28 / pri 3571 and is
 *        allocated SECOND-TO-LAST, after every block-0 value (pri 15000 ..
 *        27500).  Priority is scale-invariant (all lengths divide out), so no
 *        compaction of block 0 can reorder them: beating pri 17500 needs
 *        nrefs(t) >= 16 at len 28, and lowering the value below pri 3571 needs
 *        nrefs 3 at len >= 9, which a masked+reload value cannot have.
 *      - the pointer's live range STRICTLY CONTAINS every block-0 value's, so
 *        no third allocno can conflict with the value without also
 *        conflicting with the pointer.
 * 4. THE INVERSE SOLVER'S MINIMAL ATOM WAS TRIED AND COSTS INSTRUCTIONS.
 *    `tools/ra_solver/inverse.py global model_t6.json --goal {75:3,74:4,72:5}`
 *    returns a 1-atom solution: add a conflict between the loop index (73,
 *    pri 47142, hard 3) and the block-0 value (74).  Spelling it as `i = 0;`
 *    before block 0 with `for (; i < 3; i++)` costs three instructions
 *    (52 insns, score 31) because the index is then live across the whole
 *    body: rejected/s55b-loop-index-init-hoist-conflict-atom-52insn-score31.c.
 *    The other atoms are refs_up(t) 5 -> 16/17 and refs_down(value) 7 -> 2.
 */
/* s55 (synthesis, 2026-09-05) -- BODY UNCHANGED (honest floor 10, 49 insns,
 * re-measured on HEAD this session: score 10, 49/49).  s55 does two things:
 * it removes the FAKE-shaped dead store from the closest-to-target chassis,
 * and it converts the 55-session residual into a STRUCTURAL contradiction
 * with the standing multi-handle ban.  Read this before probing anything.
 *
 * 1. THE s54 INVALIDATION IS AVAILABLE IN PLAIN ORDINARY C.  s54 reached the
 *    target's block-0 instruction sequence (49 insns, score 13) only with a
 *    dead store `m = p[8];` whose value nothing consumed.  The same cse
 *    invalidation is produced by SPLITTING the condition across two ordinary
 *    statements, so that every value written is consumed:
 *        m = *q & 0xF8;  *q = m;
 *        m = p[8];       -- re-set of the stored value's variable: invalidates
 *        v = *q;         -- survives as a real lbu, no la of its own
 *        m &= 1;         -- split-init / compound assignment: ORDINARY C
 *        if (m) { m = v | 1; } else { m = v; }  *q = m;
 *    Measured 49 insns / score 13, byte-identical to s54's dead-store body.
 *    Banked as rejected/s55-ordinaryC-mreuse-invalidation-NO-DEAD-STORE-
 *    49insn-score13.c.  Four spellings of it (single/two-statement mask,
 *    separate or shared result variable, function- or block-scoped reload)
 *    all measure 49/13, so 13 is a hard floor for that whole family.
 *
 * 2. THE ORDER OF THE INVALIDATOR AND THE READ IS LOAD-DELAY-SLOT CRITICAL.
 *    The invalidating set must precede the read, but the CONDITION's `andi`
 *    must follow it: `m = p[8] & 1; v = *q;` (invalidator and andi fused into
 *    one statement) is 50 insns / 13, because the andi takes the lw's
 *    load-delay slot and the reload can no longer fill it.  Splitting the
 *    condition is what buys the 49th instruction.
 *
 * 3. EVERY REMAINING DIFFERENCE IS INSIDE THE MASK + BLOCK 0, AND IT IS A
 *    THREE-WAY REGISTER ROTATION PLUS ONE ORDERING.  Aligned objdump of the
 *    49/13 body against the target (tmp/grind/func_80034F88/s55/build_c1.txt):
 *    flag blocks 1 and 2, the trailing loop and the epilogue are byte-exact;
 *    12 of the 13 points are the register naming of build 4f48..4f78 --
 *        build:  q=$a0   mask/cond/result=$v1   reload=$v0
 *        target: q=$v1   mask=$a0, reload=$a0   cond/result=$v0
 *    -- and the 13th is that the target materialises flag block 1's address
 *    (80034FC8) BEFORE flag block 0's store (80034FD0), where the build emits
 *    the store first.
 *
 * 4. THE TARGET HOLDS &D_80106A73 IN TWO HARD REGISTERS AT ONCE, WHICH ONE C
 *    POINTER OBJECT CANNOT DO.  At 80034FC8 the target loads the address into
 *    $a0 while $v1 still holds the same address and is still needed by the
 *    store at 80034FD0.  The target's three `la` pairs land on $v1, $a0, $a0
 *    -- two distinct hard registers for one address.  A C local that never has
 *    its address taken gets exactly one pseudo (tools/gcc-2.7.2/stmt.c:3387)
 *    and a pseudo gets exactly one hard register (global.c:1275); GCC 2.7.2
 *    has no live-range splitting.  So NO body with a single C pointer object
 *    aliasing &D_80106A73 can emit those bytes, whatever its priorities are.
 *    That also explains item 3's ordering point directly: with one pseudo the
 *    block-1 `la` is anti-dependent on block 0's store (sched.c:1720) and can
 *    never be scheduled above it.
 *
 * 5. CONSEQUENCE FOR THE STANDING BAN.  s50 finding 3 and s54 concluded from
 *    SCORES (two-object bodies measured 21..26, then 50/24) that the
 *    multi-handle ban "is not what is costing the residual".  That inference
 *    is now contradicted by structure: the ban forbids the only object model
 *    the target's bytes admit.  The two-object bodies measured badly because
 *    their priorities were wrong on the chassis they were measured on, not
 *    because the axis is wrong.  s55 therefore files a ruling-request rather
 *    than another spelling pass; see hypotheses.md [s55].
 */
/* s54 (synthesis, 2026-09-05) -- BODY UNCHANGED (honest floor 10, 49 insns,
 * re-measured on HEAD this session: score 10, 49/49).  This body is still the
 * LOWEST-SCORING form, but it is no longer the closest one structurally: read
 * rejected/s54-mreuse-invalidation-TARGET-BLOCK0-STRUCTURE-49insn-score13.c
 * before probing anything.
 *
 * 1. THE s53 "CO-LOCATION LAW" IS BROKEN.  s53 concluded that every surviving
 *    (non-forwarded) re-read of D_80106A73 costs its own `lui/addiu` pair,
 *    because the only invalidator known to this ledger was re-executing
 *    `q = &D_80106A73;`.  There is a SECOND cse invalidation route and it is
 *    free: cse records a store's MEM destination in the value class of the
 *    SOURCE register (cse.c:7308-7376), so re-setting the C variable that held
 *    the stored value -- ordinary variable re-use, `m = p[8];` right after
 *    `*q = m;` -- calls invalidate(reg m), strips the register out of that
 *    class, and leaves the `(mem:QI (reg q))` entry with no register member.
 *    The next read of `*q` then has nothing cheaper to fold to and stays a
 *    real lbu, while reg q is untouched and no second address materialisation
 *    is emitted.
 *
 * 2. THE RESULTING BODY IS 49 INSNS / SCORE 13 AND HAS THE TARGET'S BLOCK-0
 *    SHAPE EXACTLY: three la pairs, four non-forwarded lbu, the reload in the
 *    lw's shadow (no load-delay nop), flag blocks 1 and 2, the trailing loop
 *    and the epilogue byte-exact.  Its entire residual is the block-0 REGISTER
 *    SEAT: build has q on $a0 with the values on $v1/$v0, the target has q on
 *    $v1 with the values on $a0/$v0.
 *
 * 3. THE SEAT IS ONE MEASURED INEQUALITY.  global.c allocno_compare =
 *    floor_log2(nrefs)*nrefs/livelen*10000.  On that chassis (ALLOCDBG via
 *    tools/ra_solver/extract.py, tmp/grind/func_80034F88/s54/model_t3.json):
 *        q  nrefs 11  len 29  pri 11379  -> hard 4 ($a0)
 *        m  nrefs  4  len  4  pri 20000  -> hard 3 ($v1), the only conflicting
 *                                           allocno that outranks q
 *    so the flip needs nrefs(q) >= 15, or m at nrefs 4 with livelen >= 8 (or
 *    nrefs 3 at any length).  Nine lifts measured this session all miss:
 *    else-arm-consumes-m moves m to pri 14285 but pushes p into $a2 (49/17);
 *    `if (m & 1)` inline 49/27; duplicated stores 50/16, 51/19, 51/23, 50/20
 *    (each costs an instruction -- the arms differ, so cross-jump does not
 *    re-merge them).
 *
 * 4. TWO DEAD ENDS CLOSED.  A constant re-set (`m = 0;` / `m = 1;`) invalidates
 *    nothing: six such bodies build byte-identically to THIS one (49/10), so
 *    the sanctioned dead-store family is inert on this residual.  And the
 *    BANNED two-handle shape, re-measured on the new reload chassis for
 *    diagnosis only, is 50/24 -- worse than the single-handle 49/13, so the
 *    multi-handle ban is still not what costs the 10 points.
 */
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
