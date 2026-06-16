/* REJECTED: ptr-noop-cheat — p++; p-- pointer no-op defeats GCC boolean fold.
 * cheat-reviewer verdict: FAIL (2026-06-15).
 * Tests failed: Test 1 (no semantic purpose), Test 2 (human-programmer),
 * Test 3 (GCC-internals coercion), Test 5 (no SOTN sanction for this family).
 * The p++; p-- is a dead pointer arithmetic round-trip: net effect on p is
 * zero, no memory reads or writes. Sole purpose: disturb GCC's boolean-fold
 * that converts `if (x&1) return 1; return 0;` -> `return x&1;`.
 * Not sanctioned (pointer-rmw-global-sanctioned covers RMW with actual derefs,
 * not pointer arithmetic no-ops). Do not revisit. */
s32 func_80078EC0(void) {
    s32 *p = (s32 *)D_8009BD88;
    if ((p[1] & 1) == 0) return 0;
    if ((p[0] & 1) != 0) {
        p++;
        p--;
        return 1;
    }
    return 0;
}
