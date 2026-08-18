/* CANDIDATE - func_80017848, s4 (2026-08-18).  sandbox --disable all = 12
 * (was 16 for s1/s2, 14 after s3; this is the second drop and the first one
 * produced by the permuter modality).  125/127 build insns.
 *
 * WHAT S4 ADDED (one lever, and it is the whole delta 14 -> 12):
 *   `base = slots;` immediately after the slots read, so the TWO SCAN-LOOP ENTRY
 *   GUARDS read their count through `base` while the two >=0 top guards keep
 *   reading through `slots`.  `base` is then re-pointed at the per-slot record
 *   inside each loop preheader exactly as before.  This is ordinary variable
 *   reuse - `base` is a live, read pointer at every point (both loop guards
 *   consume it before the reassignment), not a dead store or a holder - and it
 *   is the frozen-list "variable reuse for codegen control" family.
 *
 *   Measured alternatives at the same node (all engine-scored this session):
 *     both top guards through `base`      -> 12   (equivalent)
 *     both top guards through `slots`     -> 12   (THIS FORM - the cleanest)
 *     as the permuter emitted it (mixed)  -> 12   (equivalent)
 *     loop guards moved back onto `slots` -> 19   (the lever is load-bearing)
 *
 * INHERITED LEVERS (do not re-derive - s2/s3):
 *   (1) source-level do-while  -> target's per-iteration bound reload + one-addu index;
 *   (2) entry guard spelled `i = 0; if (i < count)` -> the phantom-16 frame;
 *   (3) the loop entry guard's count address written POINTER-FIRST, and the `slots`
 *       read hoisted ABOVE the two >=0 top guards so those guards consume it too.
 *   s4 note: (3) is SUFFICIENT for 14 but not NECESSARY - the s4 72-cell
 *   cross-product found fresh-read-guard cells that also reach 14.  14 was a
 *   plateau over that whole family; 12 required leaving the family.
 *
 * REMAINING RESIDUAL (2 insns short of 127, 12 differing):
 *   target keeps an uncoalesced copy of the reloaded slots pointer
 *   (`lw a0,0xC(s2)` + `move a3,a0` feeding `addu a0,a1,a3`) in each scan-loop
 *   preheader.  s3/s4 evidence says this is a register-ALLOCATION property
 *   (a coalescing failure), not a source-spelling one.  The cc1 -da .lreg/.greg
 *   dumps for this form are on disk (tmp/grind/func_80017848/s4/) and UNREAD -
 *   that is the next probe, not another spelling sweep.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 i;
    s32 dist;

    if (slot_a == slot_b) {
        return 0;
    }

    slots = *(u8 **)(ctx + 0xC);
    base = slots;
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    i = 0;
    if (i < *(s32 *)((s32)base + (slot_a << 6) + 0x1C)) {
        p = *(u8 **)(ctx + 0xC);
        base = (u8 *)((slot_a << 6) + (s32)p);
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
    }

    i = 0;
    if (i < *(s32 *)((s32)base + (slot_a << 6) + 0x20)) {
        p = *(u8 **)(ctx + 0xC);
        base = (u8 *)((slot_a << 6) + (s32)p);
        do {
            if (*(s16 *)((*(u8 *)(base + i + 0x2C) << 4) +
                         (s32) * (u8 **)(ctx + 0x10) + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x20));
    }

    dist = math_Distance3D((s32 *)(*(u8 **)(ctx + 0xC) + (slot_a << 6)),
                           (s32 *)(*(u8 **)(ctx + 0xC) + (slot_b << 6)));
    link = *(u8 **)(ctx + 0x10) + (*(s16 *)(ctx + 0x6) << 4);
    *(s32 *)(link + 0x0) = dist;
    *(s32 *)(link + 0x8) = dist * 3;
    *(s32 *)(link + 0xC) = arg1;
    *(s32 *)(link + 0x4) = (slot_a << 16) | slot_b;

    rec_a = (u8 *)((slot_a << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_a + 0x1C);
    *(s32 *)(rec_a + 0x1C) = i + 1;
    *(u8 *)(rec_a + i + 0x24) = *(u16 *)(ctx + 0x6);

    rec_b = (u8 *)((slot_b << 6) + (s32) * (u8 **)(ctx + 0xC));
    i = *(s32 *)(rec_b + 0x20);
    *(s32 *)(rec_b + 0x20) = i + 1;
    *(u8 *)(rec_b + i + 0x2C) = *(u16 *)(ctx + 0x6);

    *(s16 *)(ctx + 0x6) = *(u16 *)(ctx + 0x6) + 1;
    return 1;
}
