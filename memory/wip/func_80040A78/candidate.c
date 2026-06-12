/* Candidate pure-C body for func_80040A78 — sandbox --disable all = 1.
 *
 * HEAD's text1a.c uses two `register asm("$N")` pins (var_a1 in $a1,
 * var_v1 in $v1) to anchor the loop's register allocation AND prevent
 * GCC 2.7.2's loop reversal (check_dbra_loop in loop.c). The pins are
 * forbidden cheat-asm per [[inline-asm-policy]] / [[register-asm-pins]].
 *
 * HEAD's honest pure-C floor (sandbox --disable all on the de-pinned
 * source): 7. The remaining diff is GCC strength-reducing the count-up
 * loop to count-down (with `bgez`) because var_a1 is signed s32 and
 * `check_dbra_loop` only fires for signed LT compares (loop.c line 5849,
 * `GET_CODE(comparison) == LT`).
 *
 * This candidate's lever: change var_a1 from `s32` to `u32`.
 *   - Unsigned counter → comparison emits LTU → `check_dbra_loop`'s
 *     `GET_CODE(comparison) == LT` check fails → no loop reversal.
 *   - GCC emits the natural count-up form: `addiu a1,a1,1; sltiu v0,a1,20;
 *     bnez v0,loop`. Registers match target ($v1=var_v1, $a1=var_a1).
 *   - Score 7 → 1; build_insns matches target_insns (51).
 *
 * The remaining single diff is `sltiu` vs target's `slti` (4 bits of one
 * opcode byte). Source: target's compare is signed LT (`slti`), which
 * requires var_a1 be signed in the C — but signed var_a1 triggers
 * check_dbra_loop's reversal (as above). The two requirements are
 * mutually exclusive without a register-asm pin (which would prevent
 * reversal AND keep signed semantics, but the pin is the forbidden
 * mechanism we're escaping).
 *
 * Lever B-adjacent: per [[register-alloc-pure-c]], type change to
 * influence codegen IS sanctioned (the canonical Lever B example narrows
 * to char/short). s32→u32 is a sibling change — unsigned is the natural
 * type for a non-negative loop counter; semantic purpose is correct.
 *
 * Apply: replace the function body in src/text1a.c (lines 372-404 at
 * commit 4dd017d8) with this body verbatim. Confirm floor: `engine
 * sandbox func_80040A78 --disable all` should report score 1, build_insns
 * 51, target_insns 51.
 */
/* kengo:MED  |  my_rob/rob_life_ctrl  |  96i  |  x2 size collision */
void func_80040A78(s32 arg0) {
    u32 var_a1;
    s32 var_v1;

    var_a1 = 0;
    var_v1 = arg0 + 0x94;
    *(s32 *)(arg0 + 0x18F4) = arg0 + 0x2B4;
    *(s32 *)(arg0 + 0x18F8) = arg0 + 0x24C;
    *(s32 *)(arg0 + 0x18FC) = arg0 + 0x1E4;
    *(s32 *)(arg0 + 0x1900) = arg0 + 0x454;
    *(s32 *)(arg0 + 0x1904) = arg0 + 0x3EC;
    *(s32 *)(arg0 + 0x1908) = arg0 + 0x384;
    *(s32 *)(arg0 + 0x190C) = arg0 + 0x17C;
    *(s32 *)(arg0 + 0x1910) = arg0 + 0x114;
    *(s32 *)(arg0 + 0x1914) = arg0 + 0x72C;
    *(s32 *)(arg0 + 0x1918) = arg0 + 0x6C4;
    *(s32 *)(arg0 + 0x191C) = arg0 + 0x5F4;
    *(s32 *)(arg0 + 0x1920) = arg0 + 0x58C;
    *(s32 *)(arg0 + 0x1924) = arg0 + 0x524;
    *(s32 *)(arg0 + 0x1928) = arg0 + 0xAC;
    *(s32 *)(arg0 + 0x192C) = arg0 + 0x31C;
    *(s32 *)(arg0 + 0x1930) = arg0 + 0x4BC;
    *(s32 *)(arg0 + 0x1934) = arg0 + 0x794;
    *(s32 *)(arg0 + 0x1938) = arg0 + 0x65C;
    *(s32 *)(arg0 + 0x193C) = arg0 + 0x7FC;
    *(s32 *)(arg0 + 0x1940) = arg0 + 0x864;
    do {
        *(s32 *)(arg0 + 0x1994) = var_v1 + 0x18;
        var_v1 += 0x68;
        var_a1 += 1;
        arg0 += 4;
    } while (var_a1 < 0x14);
}
