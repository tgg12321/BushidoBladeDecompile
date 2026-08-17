/* CANDIDATE — func_80084A7C, grind session 2 (structural), 2026-08-17.
 *
 * HONEST SANDBOX DISTANCE 0 (`sandbox func_80084A7C --disable all`:
 * score 0, target_insns 145, build_insns 145, 11 regfix rules dropped,
 * 79 cheat-asm lines stripped).  Measured this session with these edits in
 * src/main.c.  Floor history: 26 (s1) -> 0 (s2).
 *
 * THIS FORM IS **NOT** CLEARED FOR SUBMISSION.  Session 2 returned
 * `ruling-request`, not `candidate-ready`, because ONE of the two levers below
 * is an arithmetic no-op with no semantic purpose and no sanctioned family:
 *
 *   LEVER 1 (legitimate, natural — keep unconditionally).  The entry block is
 *   spelled with BB2's OWN sibling idiom for this exact table.  src/main.c:303
 *   `spu_SetMotionState` is an already-matched neighbour reading the same
 *   D_80106F28 table with the same 0xB0 stride, and it spells the access as
 *       s32 shifted = a0 << 16;
 *       s32 *addr = (s32 *)&D_80106F28;
 *       s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
 *   Adopting that idiom verbatim moves the D_80106F28 address load from AFTER
 *   the `sra v0,v0,14` to BETWEEN `sll v0,a0,16` and the `sra`, which is
 *   target's schedule.  Worth exactly 2 points (26 -> 24) on its own.  Every
 *   other spelling of the address (index form, pointer add, named tbl, named
 *   idx, `<<2`, reversed operand order) is byte-identical to the old one —
 *   cse/combine canonicalises them all; ONLY the two-named-intermediate
 *   sibling idiom moves the schedule.
 *
 *   LEVER 2 (the open question — `-((s16)a1 * -0xB0)`).  Arithmetically
 *   identical to `(s16)a1 * 0xB0`; combine cancels the negation pair long
 *   before register allocation, so the EMITTED multiply chain is byte-identical
 *   to the plain form.  What it changes is the pseudo/allocno numbering, and
 *   through that the global-alloc outcome: with it, the `offset` allocno
 *   acquires a conflict with hard reg $a1 and a copy preference for $a2, lands
 *   in $a2 (target's register), and the failed-coalesce `move $a1,$a3` (our
 *   146th instruction) disappears.  Without it the same allocno has no $a1
 *   conflict and no preference, takes $a1, and both halves of the residual
 *   reappear.  It has NO observable effect on the function's behaviour, so it
 *   fails cheat-checklist tests T1/T2/T3 on its face; the nearest sanctioned
 *   family is "opaque arithmetic variables" (`s32 one = 1;`, SOTN's
 *   `(Random() & 3) + 1 - 1`), which is about defeating a bit-test transform
 *   with a NAMED variable, not about an inline no-op negation pair steering
 *   register allocation.  First reach of that spelling => owner ruling, per the
 *   frozen-list non-extension clause.  See the outcome JSON's ruling_question.
 *
 * The best form containing NO unsanctioned construct is this file minus lever 2
 * (`s32 offset = (s16)a1 * 0xB0;`), banked as
 * rejected/natural-offset-no-negation-floor24.c — honest floor 24.
 */
/* kengo:MED  |  sa_tan4/saTan4GaugeInit  |  66i */
void func_80084A7C(s16 a0, s16 a1) {
    s32 shifted = a0 << 16;
    s32 *addr = (s32 *)&D_80106F28;
    s32 *base_ptr = (s32 *)((u8 *)addr + (shifted >> 14));
    s32 offset = -((s16)a1 * -0xB0);
    u8 *base = (u8 *)(*base_ptr + offset);
    s32 val;
    u32 threshold;

    val = base[0x21] + 1;
    threshold = base[0x20];
    base[0x21] = val;

    if (threshold == 0) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(offset + *base_ptr + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
        }
        return;
    }

    if ((u8)val < threshold) {
        *(s32 *)(base + 0x88) = 0;
        base[0x1C] = 0;
        *(s32 *)(base + 0x90) = 0;
        if (*(s32 *)(offset + *base_ptr + 0x98) & 0x400) {
            *(s32 *)base = *(s32 *)(base + 0xC);
            *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
        } else {
            *(s32 *)base = *(s32 *)(base + 4);
            *(s32 *)(base + 8) = *(s32 *)(base + 4);
        }
        return;
    }

    *(s32 *)(offset + *base_ptr + 0x98) &= ~1;
    *(s32 *)(offset + *base_ptr + 0x98) &= ~8;
    *(s32 *)(offset + *base_ptr + 0x98) &= ~2;
    *(s32 *)(offset + *base_ptr + 0x98) |= 0x200;
    *(s32 *)(offset + *base_ptr + 0x98) |= 4;
    base[0x14] = 0;

    if (*(s32 *)(offset + *base_ptr + 0x98) & 0x400) {
        *(s32 *)(base + 8) = *(s32 *)(base + 0xC);
    } else {
        *(s32 *)(base + 8) = *(s32 *)(base + 4);
    }

    if (base[0x22] != 0xFF) {
        base[0x14] = 0;
        spu_ResetMotionEntry(base[0x22], base[0x23]);
        spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    }
    spu_NotifyChannel((s16)(a0 | (a1 << 8)));
    *(s32 *)(base + 0x90) = *(s16 *)(base + 0x54);
}
