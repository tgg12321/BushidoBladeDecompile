/* REJECTED — same cheat class as block-scope-alias-p1.c (layer-1 FAIL
 * 2026-06-16, layer-2 FAIL 2026-06-22) and permuter-long-new_var2-p1-alias.c
 * (s4 permuter find, rejected same-day). This is the s5 directed permuter
 * (PERM_RANDOMIZE + PERM_GENERAL, iter 237) score-0 find — different type
 * (s16*) than the s4 find (long), same lifetime-shaping intent.
 *
 * Cheat lens per [[no-new-park-categories]]:
 *   1. No semantic purpose — new_var2 is a temporary holder for the p1
 *      pointer-address expression, immediately copied to p. A human
 *      programmer writes `p = <expr>;` directly.
 *   2. Dead in emitted output — GCC forwards the value; the local exists
 *      only to shift the pseudo lifetime so RA picks v0 for the p1 addu
 *      dest (matching target's v0 vs our v1 without this alias).
 *   3. Necessary only because permuter said so — the ONLY reason to write
 *      this is that permuter found it lowers score.
 *   4. GCC-internals justification — "shifts pseudo lifetime for RA
 *      copy-pref" is pure GCC-internals reasoning; no program logic.
 *
 * KILL: the directed-permuter frontier item ("PERM_GENERAL over reload C
 * form with a wider PERM_VAR net including opaque-one-style arithmetic
 * wrappers") reproduces s4's finding: the p1 addu RA-coalescing gap CAN
 * be closed at score 0, but ONLY by lifetime-shaping alias forms — which
 * are the cheat-by-any-spelling class rejected 3+ times prior on this
 * function. No legitimate structural lever surfaces from PERM_GENERAL
 * alternatives, PERM_RANDOMIZE mutations, or their cross-product.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    s16 *new_var2;
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
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s32)(s16)prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    /* CHEAT: pointer-alias holder for p1 lifetime shaping (s5 permuter). */
    new_var2 = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    p = new_var2;
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
