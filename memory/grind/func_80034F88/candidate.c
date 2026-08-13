/*
 * func_80034F88 — grind session s15 (forensics modality).
 *
 * ============================ READ THIS FIRST ============================
 * THIS FORM MEASURES **SCORE 0** ON THE HONEST SANDBOX
 * (`sandbox func_80034F88 --disable all`, 49 build insns vs 49 target insns,
 * lbu 176 / sb 164 / lui 456 — the target's exact access census).  It is
 * s15/variants/v1.c.  It was NOT installed in src/ and it was deliberately NOT
 * submitted as `candidate-ready`.  s15 returned `ruling-request` instead.
 *
 * WHY IT IS BLOCKED.  The driver's BANNED-CONSTRUCTS list for this function
 * bans "four separate `u8 *q = &D_80106A73;` local pointer declarations (one
 * per flag block)", both as a claimed pointer-alias-fake-exception AND as
 * "ordinary program logic".  This form is NOT that construct literally — it
 * declares THREE function-scope pointers (`qm` for the 0xF8 mask, `q1` for the
 * bit-1 block, `q2` re-assigned for the bit-2 and bit-4 blocks) — but it is
 * plainly in the SAME INTENT FAMILY: multiple C handles on one global existing
 * to give GCC's allocator more allocnos.  Under cheat-checklist T5 ("it is
 * different because it is spelled with X instead of Y is exactly the loophole
 * the policy forbids") a session may not self-approve that.  The additional
 * lever it carries — declaring the six value locals BEFORE the pointers — is
 * from the frozen sanctioned "named-intermediate declaration order" family, but
 * it is chosen here for a documented allocator reason (see below), which is
 * itself a T3 signal.  So: DO NOT INSTALL AND SUBMIT THIS WITHOUT AN OWNER
 * RULING.  The ruling question is in s15's outcome JSON and in evidence.md.
 *
 * The best form s15 considers unambiguously outside the ban is s15/variants/
 * t1.c (TWO pointer objects + the declaration-order lever) at score 10 — the
 * same floor s14 reached with one re-assigned pointer.  10 is therefore the
 * recorded honest floor.
 *
 * ======================= WHY IT WORKS (s15 forensics) =======================
 * Read out of instrumented-cc1 dumps (tmp/grind/func_80034F88/s15/rtl/*, the
 * BB2_ALLOC_DEBUG / BB2_SUGG_DEBUG traces).  Three named GCC decisions, in
 * order:
 *
 * 1. cse.c value-table flush at the two-armed if/else join labels (the s14
 *    finding, unchanged): a pointer SET placed after a flush survives as a real
 *    lui/addiu materialisation and stops the previous block's store forwarding
 *    into the next block's load.  That is what produces lbu 176 / lui 456.
 *
 * 2. cse.c rewrites a pointer set that is NOT after a flush into a REGISTER
 *    COPY.  `q1 = &D_80106A73;` inside the bit-1 block sits before the first
 *    flush, so the .lreg RTL carries `(set (reg/v:SI 77) (reg/v:SI 74))` — a
 *    copy from the mask block's pointer pseudo.  The mask pseudo is confined to
 *    one basic block, so local-alloc.c assigns it a hard register before
 *    global.c ever runs, and the copy gives the bit-1 pointer allocno a
 *    copy-preference for that same hard register.
 *
 * 3. global.c `allocno_compare` orders allocation by
 *        pri = floor_log2(n_refs) * n_refs * 10000 * size / live_length
 *    and `find_reg` excludes `regs_someone_prefers[allocno]` — the registers a
 *    conflicting, not-yet-allocated allocno prefers.  With the pointer split
 *    into per-segment allocnos their priorities fall to ~2142-3076, so the
 *    three loaded-byte allocnos (pri 7500) are allocated FIRST, and the bit-1
 *    byte is pushed off $v1 (preferred by the copy-linked pointer allocno) onto
 *    $a0.  That is exactly the target's first segment: base $v1, byte $a0,
 *    with base $a0 / byte $v1 in the bit-2 and bit-4 segments.
 *
 * With ONE pointer object (s14's floor-10 form) the pointer is ONE pseudo =
 * ONE allocno = ONE hard register for the whole function, so the first segment
 * can never be allocated differently from the other two: GCC 2.7.2's global.c
 * has no live-range splitting.  That is why 10 is a hard floor for every
 * single-object shape, whatever its live-range spelling.
 *
 * The declaration-order lever is the tie-break in the same comparator: when the
 * mask+bit-1 pointer allocno and the byte allocnos tie at pri 7500,
 * `allocno_compare` falls through to `return *v1 - *v2;` — the allocno number,
 * which follows pseudo number, which follows declaration order.  Declaring the
 * value locals before the pointers is worth 11 points on the two-object chassis
 * (t2 = 21 -> t1 = 10) and 13 on this three-object chassis (v3 = 13 -> v1 = 0).
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 va;
    s32 vb;
    s32 vc;
    s32 ca;
    s32 cb;
    s32 cc;
    u8 *qm;
    u8 *q1;
    u8 *q2;

    p = func_80077D00();
    qm = &D_80106A73;
    *qm &= 0xF8;

    {
        q1 = &D_80106A73;
        ca = p[8] & 1;
        va = *q1;
        if (ca) {
            ca = va | 1;
        } else {
            ca = va;
        }
        *q1 = ca;
    }

    {
        q2 = &D_80106A73;
        cb = p[8] & 2;
        vb = *q2;
        if (cb) {
            cb = vb | 2;
        } else {
            cb = vb;
        }
        *q2 = cb;
    }

    {
        q2 = &D_80106A73;
        cc = p[8] & 4;
        vc = *q2;
        if (cc) {
            cc = vc | 4;
        } else {
            cc = vc;
        }
        *q2 = cc;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
