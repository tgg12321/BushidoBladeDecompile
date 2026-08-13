/*
 * REJECTED (s3) — "the original simply declared the flag byte volatile and wrote
 * the natural body."  DISPROVEN BY MEASUREMENT, and it matters because this is
 * the ONLY volatile spelling the project has a sanctioned path for
 * (`extern volatile u8 G;` + volatile_extern_allowlist.txt).
 *
 * The cheat-invisible sandbox STRIPS a declaration-level `volatile` (s1 proved
 * this by grepping the sandbox source snapshot), so this form cannot be measured
 * through `sandbox --disable all` at all.  s3 built a forensic scorer for it —
 * tmp/grind/func_80034F88/s3/rawscore.py — which compiles src/ exactly as it
 * stands (all regfix/asmfix rules dropped, NO volatile strip, NO cheat-asm
 * strip) and applies the same engine/score.py distance.  Numbers below are that
 * RAW score; none of them is an honest-floor claim.
 *
 *   decl-volatile + plain symbol accesses, condition first, ternary : raw 32 / 51 insns
 *   decl-volatile + plain symbol accesses, condition first, if/else : raw 28 / 51 insns
 *   decl-volatile + plain symbol accesses, read first,      if/else : raw 28 / 51 insns   <- this file
 *   decl-volatile + THREE `volatile u8 *` locals (s2's body)        : raw 12 / 49 insns
 *   plain decl    + THREE `volatile u8 *` locals (s2's body)        : raw 12 / 49 insns
 *
 * Reading: volatile at the DECLARATION is necessary-but-not-sufficient and by
 * itself is WORSE than the honest non-volatile form (20).  With every access
 * volatile and one shared address pseudo, `combine` cannot fold `%lo` into any
 * of the eight mems, so the build spends 51 insns against a 49-insn target.  The
 * target's THREE separate address bases still have to come from three pointer
 * locals — i.e. the 12 belongs to the pointer-local structure, not to the
 * declaration, and granting the byte an allowlist entry would NOT by itself
 * close the function.
 *
 * Both prongs of the allowlist gate fail anyway — see
 * rejected/volatile-ptr-coercion-score12.c for that research.
 */
//DECL:extern volatile u8 D_80106A73;
void func_80034F88(void) {
    s32 *p;
    u8 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    D_80106A73 &= 0xF8;

    val = D_80106A73;
    c = p[8] & 1;
    val2 = val | 1;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = D_80106A73;
    c = p[8] & 2;
    val2 = val | 2;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    val = D_80106A73;
    c = p[8] & 4;
    val2 = val | 4;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
