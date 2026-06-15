/* func_80078A68 (text1b_b.c) — item-slot init. HEAD body.
 * Matches ONLY with 1 regfix rule (reorder 12,11 @11-12) that undoes cc1's
 * delay-slot fill choice: cc1 moves `sh a1,8(v1)` DOWN into the outer beqz
 * delay slot; target eager-fills the slot with `andi v0,a2,0x10` (the
 * then-block's first insn) and keeps the store before the branch.
 * The choice is in cc1 reorg.c::fill_simple_delay_slots itself (raw cc1 output
 * already places sh in the slot — not a maspsx artifact). No pure-C form found
 * that flips the heuristic. See notes.md. */
s32 func_80078A68(s32 arg0, s32 arg1, s32 arg2) {
    s32 a3;
    s32 t0;
    s32 v0;
    s32 base;
    t0 = arg0 & 0xFFFF;
    a3 = 0x48;
    if (t0 >= 3) {
        return 0;
    }
    base = (t0 * 0x10) + D_8009BD6C;
    *((s16 *) (base + 4)) = 0;
    *((s16 *) (base + 8)) = (s16) arg1;
    if (((u32) t0) < 2U) {
        if (arg2 & 0x10) {
            a3 = 0x49;
        }
        v0 = arg2 & 0x1000;
        if (!(arg2 & 1)) {
            a3 |= 0x100;
        }
    } else {
        v0 = arg2 & 0x1000;
        if (t0 == 2) {
            ;
            if (!(arg2 & 1)) {
                a3 = 0x248;
            }
        }
    }
    if ((arg2 & 0x1000) != 0) {
        a3 |= 0x10;
    }
    *((s16 *) (((t0 * 0x10) + D_8009BD6C) + 4)) = a3;
    return 1;
}
