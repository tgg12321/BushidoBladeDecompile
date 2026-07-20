/* REJECTED s4 permuter find (score=10, best in ~32k-iter chassis-1 basin).
 *
 * Mutation: in loop1, stage the s16-nonneg comparison result through the
 * existing function-scope local `b` before consuming it:
 *
 *   b = (*(s16 *)(p + 2)) >= 0;
 *   if (b) { if (arg1) p[1] |= 1; else p[1] &= ~1; }
 *
 * Why permuter liked it: b's per-iteration write extends its live range
 * across loop1, perturbing register-pressure at the FALSE-arm entry
 * (@0x15cc). Lowers permuter score 40 -> 10 (partial; not sandbox 0).
 *
 * Why REJECTED: b's stored value is DEAD — every iteration overwrites the
 * previous, the final loop-exit value is immediately overwritten by
 *   b = *((u8*)player + 0x1A);
 * in the FALSE arm (and unused entirely in the TRUE arm). The construct's
 * only purpose is RA/scheduling coercion via a dead store to an existing
 * local. Matches [[dead-store-fake-exception]] carve-out shape but violates
 * its "live code only, zero dead stores" prerequisite AND the
 * [[staged-value-reused-variable]] rule's same clause. It is a cheat by any
 * spelling per [[no-new-park-categories]].
 *
 * Sibling variants in the same chassis are the same family (all sub-40 finds):
 *   - output-20-1: `b = arg1;` reused across all conditional flag pokes
 *   - output-20-2: `b = (v<<16)|(v<<8);` re-parenthesizes TRUE-arm OR (also
 *     [[or-tree-shape-shift]] FORBIDDEN)
 *   - output-30-1: `b = *q; *q = b | 1;` staging in loop2
 *   - output-30-2/3: `b = v<<8;` / `b = v<<16;` OR-tree respellings
 *   - output-30-4: `if (b = *(s32*)(q+0x57)) == 0` + `if (1) {}` dead-code
 *   - output-40-1: `i = b; gnd_load_tex(i | ...);` alias rename via `i`
 *   - output-40-2: `new_var = (u8*)player;` pointer-alias
 *
 * All chassis-1 basin outputs are variable-reuse / dead-store / OR-tree-
 * shape-shift / dead-code patterns. Zero legitimate closing forms found in
 * ~32k iters. Basin is a cheat basin.
 */
void gnd_init_80041688(s32 arg0, s32 arg1) {
    s32 *player;
    s32 i;
    u8 *p;
    u8 *q;
    s32 b, r, g, v;
    volatile s32 sp10[8];
    extern s32 func_800486FC(void);

    player = (s32 *)g_player_ptrs[arg0];
    if (player == NULL) return;

    p = (u8 *)player + 0x94;
    if (arg1) {
        p[1] |= 1;
    } else {
        p[1] &= ~1;
    }

    i = 1;
loop1:
    p += 0x68;
    b = (*(s16 *)(p + 2)) >= 0;   /* CHEAT: dead store to steer RA */
    if (b) {
        if (arg1) p[1] |= 1;
        else      p[1] &= ~1;
    }
    i++;
    if (i < 18) goto loop1;

    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1;
    else      *q &= ~1;
    q += 0x68;
    goto loop2;
after2:

    if (func_800486FC()) {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        v = func_8004881C(b, g, r);
        gnd_load_tex((v << 16) | (v << 8) | v);
    } else {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        gnd_load_tex(b | ((r << 16) | (g << 8)));
    }
    (void)sp10;
}
