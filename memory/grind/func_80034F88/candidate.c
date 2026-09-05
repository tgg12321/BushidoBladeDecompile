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
