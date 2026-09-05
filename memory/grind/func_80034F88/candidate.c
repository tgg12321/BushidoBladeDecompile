/* s49 (rederive, 2026-09-05) -- CHASSIS CHANGED (again).  Same honest floor
 * (score 10, 49 insns, byte-for-byte the same emitted instruction ORDER as the
 * s48 body) but a strictly closer ALLOCATION state, and for the first time in
 * this ledger the residual is a single measured NUMBER rather than a search.
 *
 * WHAT CHANGED FROM s48.  Two source edits, both ordinary C:
 *   1. the block-0 mask is split into two statements
 *          m = *q;
 *          m &= 0xF8;
 *      instead of the single `m = *q & 0xF8;`.
 *   2. block 0's arms consume `m` directly instead of re-reading `v = *q;`
 *      (the re-read was cse-forwarded to `m` anyway and only survived as a
 *      register-to-register copy insn).
 *
 * WHY THAT MATTERS -- measured, from tmp/grind/func_80034F88/s49 dumps:
 *   - s48 body (.greg):  ;; 75 conflicts: ... 2 3 29     <- hard 3 BLOCKED
 *   - this body (.greg): ;; 75 conflicts: ... 2 29       <- hard 3 FREE
 *   Reg 75 is the &D_80106A73 pointer allocno; the target seats it at hard 3
 *   ($v1) in blocks 0/1, we seat it at hard 4 ($a0).  In every previous body in
 *   this ledger hard 3 was in 75's CONFLICT set, put there by a block-0-local
 *   quantity that local-alloc had already seated at $v1.  Edit (1) deletes that
 *   block-0-local quantity (the QImode temp holding the raw lbu result), and
 *   edit (2) deletes the copy insn that kept the masked value block-0-local.
 *   The conflict is gone.  75 is now legal at hard 3.
 *
 * WHAT STILL BLOCKS IT -- pure global.c ORDERING, with exact numbers:
 *   global_alloc sorts allocnos by allocno_compare (global.c),
 *       pri = floor_log2(n_refs) * n_refs / live_length * 10000
 *   and on this body:
 *       reg 74 (the masked value m):  refs=6  len=9   pri=13333
 *       reg 75 (the &D_80106A73 ptr): refs=10 len=28  pri=10714
 *   so the order is "73 77 80 84 74 75 79 83 72" -- 74 is allocated BEFORE 75,
 *   takes the lowest free hard reg (2 is out, the call return $v0 is live), i.e.
 *   hard 3, and 75 then falls to hard 4.  The target's compile had the opposite
 *   order.  Everything else in the function is already exact.
 *
 * THE MEASURED DILEMMA the next session inherits (both horns measured this
 * session; see hypotheses.md s49):
 *   (a) ONE-STATEMENT mask (`m = *q & 0xF8;`, and every equivalent spelling
 *       tried: `& ~7`, `& -8`, `& 0x1F8`, `& 0xFFF8`, `(*q>>3)<<3`):
 *       allocno ORDER is already correct (75 before 74), but fold narrows the
 *       load to QImode and leaves a 2-ref block-0-local temp which local-alloc
 *       seats at hard 3 -> 75 conflicts with 3 -> $a0.  Hard 2 is unavailable to
 *       that temp because the call return $v0 is still live at the mask lbu.
 *   (b) TWO-STATEMENT mask (this body): no block-0-local temp, but the masked
 *       value is a 6-ref/9-length global allocno that outranks the 10-ref/
 *       28-length pointer allocno.
 *   To win, a form must have NO block-0-local quantity AND pri(75) > pri(74):
 *   either refs(74) <= 4 at len 9, or len(74) >= 12 at refs 6, or refs(75) >= 13
 *   at len 28, or len(75) <= 22 at refs 10.
 *
 * Also proven this session (see rejected/): restoring the target's block-1
 * reload does NOT move the seats.  Both escapes (a volatile-qualified read and
 * a non-unifiable address expression) put the lbu back and BOTH still emit the
 * address in $a0 and the value in $v1.  The reload is a separate, smaller
 * residual than the seat, not its cause.
 *
 * Ordinary C: one declared pointer object `q`, three re-assignments, no FAKE
 * construct, no pun other than the pre-existing `*(&D_80106A70 + i)` loop line
 * (a DECLARATION-level issue, unchanged from every prior candidate).
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
