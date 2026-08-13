/* func_8001979C - BEST POLICY-CLEAN FORM as of session 8b (synthesis).
 * Floor: sandbox --disable all = 4, build_insns 77 == target_insns 77.
 *
 * Session 8 reached sandbox 0, but only with a construct the layer-1
 * cheat-reviewer FAILed and the driver has since BANNED for this function:
 * `nd = 0xC - bits_left; ... needed = nd;` (a named intermediate whose value
 * is copied into `needed` after the refill).  That form is preserved at
 * memory/grind/func_8001979C/rejected/s8-nd-named-intermediate-score0-layer1-FAIL.c
 * and MUST NOT be re-proposed without an owner ruling.  This file is the best
 * form that carries no banned construct.
 *
 * THE ENTIRE REMAINING RESIDUAL (session 8b measurement, cmp.py against
 * asm/funcs/func_8001979C.s) is TWO instructions - one per bit loop:
 *
 *   target : addu t0,zero,zero ; addiu t4,zero,12 ; addiu t2,zero,32 ; addu t1,t3,zero
 *   ours   : addu t0,zero,zero ; addiu t2,zero,32 ; addiu t4,zero,12 ; addu t1,t3,zero
 *
 * i.e. the two hoisted loop-invariant constants are emitted in the wrong
 * order (loop 2 is the same with 2 instead of 12).  Everything else in the
 * function - every register, every other instruction, both preheaders' giv
 * inits and the whole shift/or/store chain - is already byte-identical.
 *
 * WHY THIS IS NOT REACHABLE BY MOVING STATEMENTS (session 8b, dump-proven,
 * tmp/grind/func_8001979C/s8b/dump_{nond,va,nd}/code6cac.i.jump2):
 *   - loop.c move_movables hoists the two constant loads in their RTL order,
 *     and their RTL order is the expand-time order, which is the order of the
 *     SOURCE STATEMENTS that first use each constant in a register.
 *   - sched.c does NOT reorder these insns: in all three measured forms the
 *     emitted arm is exactly the jump2 RTL order.
 *   - the constant 32 is first used by `hi = 0x20 - bits_left;`, which must be
 *     the arm's FIRST instruction (target: `subu $v1,$t2,$a3`), and the
 *     constant 12 is used only by `needed = 0xC - bits_left;`, which must be
 *     the arm's FIFTH instruction, after the `lw`/`addiu` refill.
 *   - therefore, with one statement per subtraction, whichever subtraction is
 *     written first fixes BOTH the constant order and the subtraction order,
 *     and the two requirements are in direct conflict:
 *       this file (subtraction late) ...... constants wrong, arm right, 4
 *       subtraction hoisted to arm top .... constants right, arm wrong, 4
 *         (rejected/s8b-needed-hoisted-liorder-right-subu-early-score4.c)
 *   - the ONLY way to satisfy both is to separate the constant LOAD from its
 *     SUBTRACTION, which requires a second name for the value.  Two spellings
 *     exist and both are policy-blocked: the banned `nd` named intermediate
 *     (which works because cse.c deletes the early `subu` and re-materialises
 *     it at the copy site, leaving `(set reg 12)` at the early position - see
 *     dump_nd jump2 insn 78), and an opaque constant holder `s32 w = 0xC;`
 *     used only by the subtraction (measured 16 and 18; forbidden family).
 *
 * The four levers this form still carries, each individually load-bearing
 * (session-8 deletion sweep, unchanged here): `hi` carrying its own
 * `0x20 - bits_left` shift amount (global.c allocno_compare floor_log2 bucket
 * -> `hi` keeps $v1); the split OR (sched.c:2464 INSN_LUID tie-break ->
 * `sllv $a2,$a2,$a0` before the `or`); `val` as the new-bits_left intermediate
 * plus the function's final zero (cse.c:7454 cheapest-register rewrite blocked
 * by make_regs_eqv's last-use test at cse.c:856, which is what materialises
 * `subu $v0,$t2,$a0 ; addu $a3,$v0,$zero`); and `neg2` for the third loop's
 * fill value (global.c find_reg conflict graph -> the fill pointer in $v0).
 * The giv rewrite (`dst = base + i * 2;` inside the loop) is session 8's
 * structural closure of D4's walker-init position and is not a lever but
 * ordinary C.
 */
void func_8001979C(s32 arg0, u32 *arg1) {
    s32 bits_left;
    u32 base;
    s32 i;
    u32 cur;
    u32 hi;
    s32 needed;
    u32 dst;
    u32 dst2;
    u32 out;
    s32 val;
    s32 neg2;
    u32 lo;
    u32 lo2;

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    do {
        dst = base + i * 2;
        if (bits_left < 0xC) {
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            arg1++;
            needed = 0xC - bits_left;
            val = 0x20 - needed;
            bits_left = val;
            hi = hi << needed;
            lo = cur >> bits_left;
            cur <<= needed;
            hi = hi | lo;
            *(s16 *)(dst + 0xA) = (s16)hi;
        } else {
            *(s16 *)(dst + 0xA) = (s16)(cur >> 20);
            cur <<= 0xC;
            bits_left -= 0xC;
        }
        i++;
    } while (i < 0x3F);

    i = 0;
    do {
        dst2 = base + i * 2;
        if (bits_left < 2) {
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            arg1++;
            needed = 2 - bits_left;
            val = 0x20 - needed;
            bits_left = val;
            hi = hi << needed;
            lo2 = cur >> bits_left;
            cur <<= needed;
            hi = hi | lo2;
            *(s16 *)(dst2 + 0x8E) = (s16)hi;
        } else {
            *(s16 *)(dst2 + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
        }
        i++;
    } while (i < 0x3F);

    neg2 = -2;
    i = 3;
    out = base + 0x348;
    do {
        *(s32 *)(out + 0x110) = neg2;
        i--;
        out -= 0x118;
    } while (i >= 0);
    val = 0;
    *(s32 *)(base + 0x10C) = val;
}
