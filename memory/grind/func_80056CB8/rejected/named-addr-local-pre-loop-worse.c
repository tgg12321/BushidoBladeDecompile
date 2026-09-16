/* REJECTED s23 -- named `s32 addr;` local, assigned 0x1F8002B8 ONCE
 * before the loop (mirroring the target's own single pre-loop
 * materialization discovered by a fresh read of
 * asm/funcs/func_80056CB8.s:1-30 this session -- target stores it to a
 * stack slot and reloads via `lw` before each call), passed to both
 * func_80053614 call sites instead of the bare repeated literal.
 *
 * Two declaration-order variants measured on the s22-banked 38/198
 * chassis, BOTH worse than the bare-literal baseline:
 *   - addr declared LAST (after start/limit/i): score 38 -> 45/204,
 *     build_insns 198 -> 199.
 *   - addr declared FIRST (before start/limit/i): score 38 -> 44/204,
 *     build_insns 198 -> 199.
 * Both move build_insns toward target's 204 (+1 real insn) but regress
 * net score -- register-identity changes elsewhere outweigh the
 * insn-count gain. See hypotheses.md s23.
 */

void func_80056CB8(s32 arg0) {
    s32 pt0[4];
    s32 pt1[4];
    s32 hit0[4];
    s32 hit1[4];
    s32 work[4];
    /* variant A: addr declared first */
    s32 addr;
    s32 start;
    s32 limit;
    s32 i;

    addr = 0x1F8002B8;
    start = (*(u16 *)(arg0 + 0x3E8) & 3) * 2;
    limit = start + 2;
    for (i = start; i < limit; i++) {
        /* ... unchanged body, both func_80053614 calls pass `addr` instead
         * of the literal 0x1F8002B8 ... */
    }
}
