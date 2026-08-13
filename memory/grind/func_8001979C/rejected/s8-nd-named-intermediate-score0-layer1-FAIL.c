/* func_8001979C - CLOSING FORM as of session 8 (rederive).  Floor: sandbox
 * --disable all = 0 (target_insns 77, build_insns 77) - down from 4 at the end
 * of session 7 (8 at s6, 12 at s5, 18 at s4, 20 at s2/s3, 24 at recon).  This
 * form is IN PLACE in src/code6cac.c at the end of session 8, and the
 * disassembly of tmp/sandbox/func_8001979C/code6cac.o differs from
 * asm/funcs/func_8001979C.s on exactly four lines, all of which are the
 * comparison script's own masks (the %hi/%lo pair of D_800F1B18 and two
 * branch-target immediates).
 *
 * WHAT CHANGED IN SESSION 8 - D4, the last family, is CLOSED, and it was
 * closed by a STRUCTURAL rederive rather than by another allocation lever.
 *
 * D4 was: our preheader emits the walker init `addu $t1,$t3,$zero` FIRST,
 * ahead of the `sw`/`lw` preamble, where target emits it LAST, after the three
 * hoisted loop-invariant constants
 * (`addu $t0,$zero,$zero ; addiu $t4,zero,<w> ; addiu $t2,zero,0x20`).
 * Sessions 2, 5, 6 and 7 each moved the initialisation later IN SOURCE at four
 * different allocations and measured 31 / 27 / 23 / 19 - always worse, because
 * a late init shortens the walker's live range and lifts it above the loop
 * counter in global.c allocno_compare.
 *
 * THE MECHANISM, NAMED (session 8): an ordinary pre-loop assignment can NEVER
 * land after the hoisted constants, at any source position.  loop.c's
 * move_movables emits each hoisted invariant immediately before loop_start,
 * i.e. after ALL straight-line pre-loop code, so every source statement is
 * upstream of them by construction.  The only insns that can follow the
 * hoisted constants are ones loop.c itself emits later - and strength_reduce,
 * which runs after move_movables in scan_loop, emits a reduced giv's
 * initialisation exactly there.  Therefore target's `addu $t1,$t3,$zero` is
 * not a source-level pointer init at all: it is the INITIALISATION OF A
 * REDUCED GENERAL INDUCTION VARIABLE.
 *
 * The C that produces it: stop writing the walking pointer as a biv with a
 * duplicated `dst += 2` increment, and compute it from the loop counter inside
 * the loop instead -
 *
 *      i = 0;
 *      do {
 *          dst = base + i * 2;
 *          ...  *(s16 *)(dst + 0xA) = (s16)hi;  ...
 *          i++;
 *      } while (i < 0x3F);
 *
 * `dst` is then a DEST_REG giv (add_val = base, mult_val = 2) which loop.c
 * reduces: the init `addu $t1,$t3,$zero` is emitted at the preheader insertion
 * point (after the hoisted `addiu $t4,zero,<w>` / `addiu $t2,zero,0x20`), the
 * increment `addiu $t1,$t1,0x2` lands in the bnez delay slot, and because the
 * reduced giv is the VARIABLE - not the memory address - the `+0xA` / `+0x8E`
 * stays a store displacement and BOTH `sh` instructions are kept.  That is
 * target's D3 shape as well, so the giv rewrite subsumes session 2's
 * two-biv-increment trick entirely.
 *
 * CONSEQUENCE FOR POLICY: the duplicated `dst += 2` in both if-arms - the
 * construct sessions 2-7 carried as policy item (i) - is GONE from the closing
 * form, and so is session 7's `out`-as-OR-carrier (item (iv)): with the giv
 * chassis a plain fresh named intermediate per loop (`lo`, `lo2`) reaches 0,
 * where on the s7 chassis it scored 20.
 *
 * THE FOUR REMAINING LEVERS, each re-measured on THIS chassis by deleting it
 * from the closing form (session-8 sweep, all at build_insns 77 unless noted):
 *   - `hi` carrying its own `0x20 - bits_left` shift amount ....... 0 -> 16
 *     (a fresh dedicated local `amt` for the same value also scores 16, so it
 *     is the REUSE that matters, exactly as session 5 measured)
 *   - the split OR (`hi = hi << needed; lo = cur >> bits_left;
 *     cur <<= needed; hi = hi | lo;`) ............................. 0 -> 55
 *   - `val` as the new-bits_left intermediate + the final zero ..... 0 -> 4
 *     (and build_insns 77 -> 75: D1's copy pair disappears)
 *   - `nd` holding the width subtraction ......................... 0 -> 4
 *     (hoisting `needed = w - bits_left;` to the top of the arm instead of
 *     using `nd` also scores 4)
 *   - `neg2` for the third loop's fill value ...................... 0 -> 2
 *   - the fully natural C (none of the five) ..................... 0 -> 22
 *
 * Register assignment matches target exactly everywhere: $a3 = bits_left,
 * $t0 = counter, $t1 = the reduced walker giv, $t2 = 0x20, $t3 = base,
 * $t4 = field width, $a2 = cur, $a1 = arg1, $a0 = needed, $v1 = hi,
 * $v0 = val / carrier, and in the third loop $v0 = out / $v1 = neg2.
 *
 * Mechanisms for the four remaining levers (all established in sessions 5-7
 * and unchanged by the rederive): `hi`'s shift-amount reuse crosses a
 * floor_log2(nrefs) bucket in global.c allocno_compare so `hi` keeps $v1;
 * the split OR inverts sched.c's final INSN_LUID tie-break at sched.c:2464 so
 * `sllv $a2,$a2,$a0` is emitted before the `or`; `val` blocks cse.c:7454's
 * cheapest-register rewrite via the make_regs_eqv last-use test at cse.c:856,
 * which is what materialises `subu $v0,$t2,$a0 ; addu $a3,$v0,$zero`; `neg2`
 * gives the third loop's fill value its own allocno so find_reg's conflict
 * graph puts the fill pointer in $v0.  Policy classification of each is in
 * memory/grind/func_8001979C/self_vet.md.
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
    s32 nd;
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
            nd = 0xC - bits_left;
            /* FAKE: hi carries its own shift amount before the value, mechanism: global.c allocno_compare (hi crosses the floor_log2 nrefs bucket and keeps $v1), lever-exhaustion: memory/grind/func_8001979C/hypotheses.md [s2] H2-B, [s3] H3-A, [s8] v2_amt */
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            arg1++;
            needed = nd;
            /* FAKE: new bits_left routed through val, mechanism: cse.c:7454 cheapest-register rewrite blocked by make_regs_eqv's last-use test at cse.c:856, lever-exhaustion: hypotheses.md [s1] H-C, [s6] H6-A, [s8] v2_no_val */
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
            nd = 2 - bits_left;
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            arg1++;
            needed = nd;
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

    /* FAKE: named constant holder for the fill value, mechanism: global.c find_reg conflict graph (a separate allocno for -2 is what puts the fill pointer in $v0), lever-exhaustion: hypotheses.md [s5] H5-B, [s6] H6-A, [s8] v2_no_neg2 */
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
