/* MATCHING form (s29, 2026-08-20): `sandbox func_80057CC8 --disable all` = 0,
 * target_insns 111 == build_insns 111, rules_dropped 0; full `build` sha1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.
 *
 * Delta from the 28-session floor-3 baseline (commit 25af3133, layer-2 PASSed):
 * the PREV-neighbour lookup no longer builds a second value into the pointer
 * local `p`; it reads the vertex pair directly as `table[(s16)prev_idx * 2]` /
 * `[... + 1]`. Nothing was ADDED - a construct was REMOVED (the baseline`s dead
 * `s16 new_var` carrier is also gone, proven byte-inert). `p` survives as the
 * single-SET next-neighbour pointer exactly as in the baseline.
 *
 * Why that closes the 3: every prior session`s `p` carried TWO SETs, so its
 * DECL_RTL pseudo (86) tripped local-alloc.c:472`s reg_n_deaths==1 test and was
 * punted to global-alloc, where the copy preference from pseudo 129 pinned it
 * to $v1 (`addu v1,v0,a2`) instead of target`s coalesced `addu v0,v0,a2`.
 * One SET, no bail-out, correct coalesce.
 *
 * Why the second read of the table base (`*(s16 **)(arg0 + 4)` inside `p`s
 * address expression, inherited verbatim from the PASSed baseline) must stay:
 * it is in the TARGET BYTES - asm/funcs/func_80057CC8.s:17 `lw $a2,0x4($s2)`
 * and :50 `lw $a0,0x4($s2)` are two independent loads of that field. Reusing
 * the already-loaded `table` there instead costs an instruction: measured
 * score 30 / build_insns 112 (tmp/grind/func_80057CC8/s29b/v7.c and v9.c).
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 *p;
    u16 cy;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    ang_prev = ratan2(table[(s16) prev_idx * 2] - (s16) cx, table[(s16) prev_idx * 2 + 1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = ratan2(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(*(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
