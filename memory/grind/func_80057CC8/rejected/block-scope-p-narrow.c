/* s23 measurement: BLOCK-SCOPE narrowing of the existing single `p` local.
 * NOT an alias holder, NOT a rename, NOT a split — the SAME `p` s16* variable
 * is simply declared inside `{ ... }` around both call sites instead of at
 * function scope. Single-pseudo semantics unchanged (one DECL_RTL).
 *
 * Sandbox --disable all: score=3 (byte-neutral with candidate function-scope).
 *
 * Evidence: scope narrowing is a NULL LEVER on the pseudo-86 copy-preference
 * chain for func_80057CC8. Confirms s7 forensics: pseudo 86 = /v-marked
 * DECL_RTL of C-source `p`; the DECL_RTL binding is not scope-depth sensitive
 * within a single function (no nested-function boundary, no closure). Both
 * scope variants land the identical RA outcome (v1 pref for pseudo 129
 * propagates to 86 exactly the same way).
 *
 * Rejection reason: not an improvement — floor unchanged at 3. Does NOT close
 * axis (1) from Judge (that axis requires eliminating the C-source `p` local
 * entirely so RTL-expand emits ANONYMOUS pseudos at each call-arg site;
 * block-scope still emits a user-declared allocno).
 *
 * Kills the "maybe tighter scope changes DECL_RTL classification" fresh-shape
 * hypothesis that had not been measured before s23. */
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

    {
        s16 *p;
        p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
        ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
        p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
        ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    }

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
