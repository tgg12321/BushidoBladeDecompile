/* [s56 REDERIVE - cell K2, 4 at 127/127.  THE MOST ADVANCED FORM ON FILE.
 *
 * Every one of the 127 instructions matches the target in opcode, operands and
 * order EXCEPT the seat of the two preheader copies: target `addu a3,a0,zero` /
 * `addu a0,a1,a3`, this body `addu v0,a0,zero` / `addu a0,a1,v0`, once per loop.
 * Nothing else differs.  The loop-1 exit tail, loop 2's guard load, the call
 * block, rec_a/rec_b and the epilogue are all target-exact.
 *
 * WHAT CLOSED THE s55 TWO-GATE CONFLICT (evidence.md E-s56-1..4):
 *   s55 found that combine.c:914 (`! all_adjacent && use_crosses_set_p (src,
 *   INSN_CUID (insn))`) refuses to merge the preheader copy `p = q` into the
 *   base add when the copy's SOURCE q is set between the two insns, and that
 *   the only free intervening instruction is the ctx+0x10 lnk load.  s55
 *   spelled that load INTO q (cell V3) and lost every seat, because q then held
 *   lnk through the whole loop body.  This body writes the lnk load into q and
 *   immediately copies it out (`q = *(u8 **)(ctx + 0x10); lnk = q;`): combine
 *   merges the load into `lnk = q` (lnk = mem, q's second value vanishes), but
 *   combine never resets reg_last_set[] for a deleted i2 (combine.c:9804 is the
 *   only writer), so when it later reaches the base add `q = sh + p` the gate
 *   still sees a set of q after the copy and keeps the copy.  Zero instructions
 *   are added, q's live range is exactly U1's (ptr -> copy, then base through
 *   the loop), lnk is its own pseudo (a2), and the copy survives WITHOUT any
 *   second reader.  The two gates s55 called mutually exclusive are both bought.
 *
 * WHAT IS LEFT (E-s56-4, K1/K2 RA-solver runs 14/14 and 15/15 exact):
 *   the copy destination's seat.  In K1 (per-loop p/p2) it is a LOCAL quantity
 *   and local-alloc's ascending scan seats it in v0 (the s48 mechanism).  In
 *   this body (ONE variable p for both loops) it is a GLOBAL allocno (pseudo 78,
 *   pri 13333, hard_conf {v1,sp}, someone_prefers {}, prefs {v0}) and global.c's
 *   pass-0 scan still returns v0 because nothing excludes v0, a0, a1 or a2:
 *   p is born at the copy where q dies and dies at the add where q is reborn,
 *   so it never overlaps a0; sh and lnk sort below it.  Target's a3 needs p to
 *   conflict with v0 AND a0, i.e. to outlive the base add (a reader after the
 *   add that leaves no bytes - the E-s44-3 requirement, unchanged) - OR a copy
 *   preference for a3 from a hard-reg move that is deleted as a no-op.
 *
 * Sibling transplant (func_8005BA8C H8, entry cursor `c = ctx;`) measured on
 * this chassis = 8 at 127/127: it moves the s2 prologue pair.  KILLED.
 */
s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *link;
    u8 *slots;
    u8 *p;
    u8 *q;
    u8 *lnk;
    u8 *q2;
    u8 *lnk2;
    u8 *rec_a;
    u8 *rec_b;
    s32 sh;
    s32 sh2;
    s32 t;
    s32 t2;
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

    sh = slot_a << 6;
    i = 0;
    q = *(u8 **)(ctx + 0xC);
    t = sh + (s32)q;
    t = *(s32 *)(t + 0x1C);
    if (i < t) {
        p = q;
        q = *(u8 **)(ctx + 0x10);
        lnk = q;
        q = (u8 *)(sh + (s32)p);
        do {
            if (*(u16 *)((*(u8 *)(q + i + 0x24) << 4) +
                         (s32)lnk + 0x4) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(q + 0x1C));
    }

    sh2 = slot_a << 6;
    i = 0;
    q2 = *(u8 **)(ctx + 0xC);
    t2 = sh2 + (s32)q2;
    t2 = *(s32 *)(t2 + 0x20);
    if (i < t2) {
        p = q2;
        q2 = *(u8 **)(ctx + 0x10);
        lnk2 = q2;
        q2 = (u8 *)(sh2 + (s32)p);
        do {
            if (*(s16 *)((*(u8 *)(q2 + i + 0x2C) << 4) +
                         (s32)lnk2 + 0x6) == slot_b) {
                return 0;
            }
            i++;
        } while (i < *(s32 *)(q2 + 0x20));
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
