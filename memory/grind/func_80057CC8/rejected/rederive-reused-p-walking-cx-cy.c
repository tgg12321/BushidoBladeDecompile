/* [s26] REJECTED: Reused-p walking pointer form — reuse the same `p` local
 * for the cx/cy base pointer AND the prev/next neighbor pointers.
 *
 * Rederive angle: target asm at 80057D0C emits `addu $v0, $v0, $a2` for the
 * cx/cy base addu with $v0 dest (in-place coalesce), then AGAIN at 80057D54
 * emits `addu $v0, $v0, $a2` for the p1 addu with $v0 dest. Both target the
 * same hard reg. Hypothesis: target's C reuses the same `p` local for the
 * cx/cy reads AND the neighbor reads (three SETs of pseudo 86 in BB4).
 *
 * Sandbox --disable all measurement: score=12, target=111 build=112 (+1 insn).
 *
 * Mechanism: adding a third SET of pseudo 86 (for the cx/cy base at BB4 top)
 * makes reg_n_deaths[86]==3 (already >=2 from s21 local-alloc.c:472 kill —
 * still triggers global-alloc bail-out). set_preference now walks FIRST
 * PLUS operand for THREE SETs, so pre-prune hard_reg_preferences[86] is
 * broader — but the p1 addu STILL misses coalescing AND we've added an
 * insn to the function (the second `p = ...` for prev now reloads from
 * a table copy rather than the natural cx/cy source path). Regression, not
 * a lever.
 *
 * Adjacent form to but DIFFERENT from s18 cur-pointer-cx-cy (which introduced
 * a SEPARATE `u16 *cur` local, creating a fresh pseudo — that was +1 to
 * score 4). This form reuses `p` — pseudo 86 gets three SETs instead of two,
 * a genuinely novel rederive angle not measured across s1-s25. Rejected on
 * measurement, no cheat-class concerns (semantically legitimate reuse).
 *
 * KILL: three-SET-of-p walking-pointer form is measurably worse than the
 * two-SET candidate baseline. The extra cx/cy binding of `p` doesn't
 * redirect pseudo 86's copy-pref origin (still {v1} from pseudo 129 via
 * insn 124's addsi3 as s6/s7/s15/s16/s24 confirmed) and adds an insn.
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
    s16 new_var;
    s16 *p;
    u16 cy;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    p = (s16 *)((s32)table + arg1 * 4);
    cx = *(u16 *)p;
    cy = *(u16 *)((s32)p + 2);

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

    p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
