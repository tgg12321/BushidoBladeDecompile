/* REJECTED - cell X1 (s18, second dispatch, modality rederive): score 4 at
 * 126/127 instructions.  TARGET-SHAPED LOOP-1 EXIT TAIL: target re-materialises
 * BOTH values it needs after loop 1 on the exit edge - `lw $a0,0xC($s2)` then
 * `sll $a1,$s4,6` (asm/funcs/func_80017848.s:56-57) - and the join block
 * .L8001791C then uses them for loop 2's guard and, after the copy, its base.
 * This cell writes exactly that: the loop-1 exit tail is `p = *(u8 **)(ctx+0xC);
 * sh = slot_a << 6;` and `sh2` is deleted so loop 2's guard and base both read
 * the recomputed `sh`.
 * Result 4 / 126 insns - loop 1 comes out ONE INSTRUCTION SHORT, the exact
 * signature of E-s16-2: with a fresh read (rather than the candidate's `p = q`)
 * downstream of loop 1's base add, cse canonicalises the base add onto the COPY
 * DEST, combine then deletes the copy, and loop 1 loses target's
 * `addu $a3,$a0,$zero`.
 * Value: it isolates the shift-placement variable out of s12's symmetric
 * chassis.  s12's symmetric cell (fresh-read tail, sh2 recomputed just before
 * loop 2's guard) is also 4/126; moving the recompute up onto the exit edge, so
 * that it sits exactly where target's `sll $a1,$s4,6` is, changes nothing.  The
 * +1 of the symmetric chassis is entirely the tail read, never the shift site.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *lnk;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *base;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 t;
    s32 i;
    s32 dist;

    if (slot_a == slot_b) {
        return 0;
    }

    slots = *(u8 **)(ctx + 0xC);
    if (*(s32 *)((slot_a << 6) + (s32)slots + 0x18) >= 0) {
        if (*(s32 *)((slot_b << 6) + (s32)slots + 0x18) >= 0) {
            return 0;
        }
    }

    p = *(u8 **)(ctx + 0xC);
    sh = slot_a << 6;
    i = 0;
    t = sh + (s32)p;
    t = *(s32 *)(t + 0x1C);
    if (i < t) {
        q = *(u8 **)(ctx + 0xC);
        lnk = *(u8 **)(ctx + 0x10);
        base = (u8 *)(sh + (s32)q);
        do {
            if (*(u16 *)((*(u8 *)(base + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(base + 0x1C));
        p = *(u8 **)(ctx + 0xC);
        sh = slot_a << 6;
    }

    i = 0;
    if (i < *(s32 *)(sh + (s32)p + 0x20)) {
        base = (u8 *)(sh + (s32) * (u8 **)(ctx + 0xC));
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
