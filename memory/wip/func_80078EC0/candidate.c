/* func_80078EC0 (text1b_b.c) — clean candidate, floor 3 (HEAD 10), ZERO cheats.
 * Removed the 2 register pins (p asm("$3"), volatile ret asm("$2")) AND made the
 * 5 regfix rules (frame-removal + $4->$2 renames) unnecessary: the three-return
 * form gets BOTH registers right (p->$v1, ret->$v0, frameless) on its own.
 * Residual 3 = GCC folds `if(p[0]&1) return 1; return 0;` -> `return p[0]&1;`
 * (boolean simplification); target keeps the verbose `bnez; li v0,1; move v0,zero`.
 * Not byte-identical (3 short, build 13 vs target 16). Clean WIP only. */
s32 func_80078EC0(void) {
    s32 *p = (s32 *)D_8009BD88;
    if ((p[1] & 1) == 0) return 0;
    if ((p[0] & 1) != 0) return 1;
    return 0;
}
