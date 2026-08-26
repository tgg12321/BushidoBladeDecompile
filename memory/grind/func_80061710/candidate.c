/* func_80061710 — MATCHED FORM: sandbox --disable all = 0 (46/46, zero rules,
 * zero register pins, zero cheat-asm).  Found by grind s5 (synthesis modality,
 * 2026-08-25) and left in place in src/text1b.c.
 *
 * HOW THE v0<->v1 WALL FELL (s1-s4 all believed it architectural):
 *   The COMPLETED-C sibling func_8006156C (src/text1b.c:3296) has a BYTE-
 *   IDENTICAL tail shape to 710's — three arg0[] loads interleaved with a
 *   lui/ori constant store to D_800A3464 — and it matches in pure C.  Its tail
 *   is spelled with a WALKING POINTER and an INLINE constant written LAST:
 *       p = arg0;
 *       D_800F1140 = *p++;  D_800F1144 = *p++;  D_800F1148 = *p;
 *       D_800A3464 = 0xFF8080;
 *   Transplanting that exact spelling into 710 puts the three loads in $v0 and
 *   the 0x10FF10 constant in $v1, interleaved — target's layout.  Every prior
 *   session used `t = arg0[N]; D_800F114X = t;` with a named `mask` local, which
 *   is what produced the v0<->v1 rename; the s1 note "walking pointer adds an
 *   addiu" (inherited from sibling 611A4 s2) is FALSE on this chassis — GCC
 *   folds the increments into 0/4/8($s0) offsets exactly as target does.
 *
 * The switch head then needs `val`/`q` factored OUT of the two arms into a
 * SHARED trailing block (target literally shares that block at .L8006176C),
 * with `default: goto done;` skipping it.  Writing the constant inline inside
 * each case instead (v9b) swaps the head's $v0/$v1 roles -> 12.
 *
 * The `s32 *v1` pointer alias is load-bearing: the direct-global form
 * (rejected/v9e-no-pointer-alias-direct-global-floor5.c) measures 5, because
 * the alias is what keeps &D_800F116C in one pseudo ($a0) live across the
 * switch instead of being re-materialised per use.  Annotated with a FAKE comment per
 * .claude/rules/pointer-alias-fake-exception.md (in-repo precedent: the same
 * declaration in COMPLETED-C func_8006156C, src/text1b.c:3297).
 *
 * NO constant-staging.  The refused F1 "constant-staging through a REUSED LIVE
 * local" family (docs/grind/borderline.md 2026-08-18, survey WEAK) is absent
 * from this form entirely: `val` is a per-arm merge value consumed by the
 * shared block and dead before the tail; the tail has no temp at all.
 */
void func_80061710(s32 *arg0, s32 arg1) {
    /* FAKE: local pointer alias to D_800F116C, mechanism: base-register
     * allocation / address-materialization caching in local-alloc (the alias
     * gives GCC one pseudo holding &D_800F116C, kept live in $a0 across the
     * switch instead of being re-materialized per use), lever-exhaustion:
     * memory/grind/func_80061710/hypotheses.md (s1-s4: structural, mask-position
     * sweep, native permuter all measured dead) + s5 direct-global form
     * (tmp/grind/func_80061710/s5/v9e_noalias.c) measured sandbox 5. */
    s32 *v1 = (s32 *)&D_800F116C;
    s32 *p;
    u8 *q;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000E;
        q = &D_800F115C + 2;
        break;
    case 1:
        val = 0x21000F;
        q = &D_800F115C + 3;
        break;
    default:
        goto done;
    }
    *q = 0;
    D_800F1180 = (s32)q;
    *v1 = val;
done:
    func_80060A68();
    p = arg0;
    D_800F1140 = *p++;
    D_800F1144 = *p++;
    D_800F1148 = *p;
    D_800A3464 = 0x10FF10;
}
