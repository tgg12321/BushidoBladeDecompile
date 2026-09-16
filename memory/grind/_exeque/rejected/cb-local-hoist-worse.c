/* _exeque -- s5 rejected form: hoist the D_8009BE80 callback pointer into
 * a named local `cb` BEFORE the guard test, then call `cb()` instead of
 * `((s32 (*)(void))D_8009BE80)()`, hoping the extra live value changes
 * register pressure around the jalr enough to block the delay-slot fill.
 *
 * Measured (session 5, sandbox --disable all): sandbox score got WORSE,
 * 2 -> 11 (build_insns 186 -> 184 -- fewer instructions than target, i.e.
 * this changed codegen structurally, not just the delay-slot fill).
 * Reverted immediately; not a viable direction.
 */
    if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000)) {
        s32 *p = &D_8009BE7C;
        s32 (*cb)(void) = (s32 (*)(void))D_8009BE80;
        if (*p != 0 && D_8009BE80 != 0) {
            *p = 0;
            cb();
        }
    }
