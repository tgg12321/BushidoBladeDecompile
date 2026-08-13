/* func_8001979C - BEST FORM as of session 2 (structural). Floor: sandbox
 * --disable all = 20 (target_insns 77, build_insns 77) - down from 24 at
 * the end of session 1, and the FIRST form that reaches instruction parity
 * AND target's exact register assignment at the same time.
 *
 * This form is IN PLACE in src/code6cac.c at the end of session 2.
 *
 * What session 2 changed vs the session-1 baseline, and why (all three are
 * load-bearing; removing any one regresses the score - measurements in
 * evidence.md):
 *
 *  1. `dst += 2` is duplicated into BOTH if-arms instead of sitting once at
 *     the loop bottom.  This gives the walking-pointer biv TWO increments, so
 *     loop.c computes `benefit -= add_cost * bl->biv_count` with biv_count 2,
 *     the combined DEST_ADDR giv's benefit falls to <= 0, strength_reduce sets
 *     v->ignore, all_reduced goes 0 and maybe_eliminate_biv is never reached.
 *     The biv therefore survives and the +0xA / +0x8E stays a store
 *     displacement - target's exact D3 form - while both `sh` instructions are
 *     kept.  This closed D3, which session 1 could only reach with a single
 *     store.
 *
 *  2. `val` carries BOTH `0x20 - bits_left` (the hi shift amount) and
 *     `0x20 - needed` (the new bits_left).  The second use is session 1's
 *     confirmed D1 lever (it materialises target's `subu $v0,$t2,$a0` +
 *     `addu $a3,$v0,$zero` pair); reusing the same variable for the first
 *     keeps `hi` out of the $t register file, which the separate-temp
 *     spelling did not (measured: score 31 with an anonymous temp vs 20 here).
 *
 *  3. Per-loop walking pointers (`dst`, `dst2`, `out`) and their
 *     initialisations placed as early as their loop allows.  This is pure
 *     allocno-priority arithmetic in global.c: priority is
 *     floor_log2(nrefs)*nrefs/livelen.  The duplicated increment adds 4
 *     weighted refs to each walker, and the walkers must stay BELOW the loop
 *     counter in priority so that the counter takes $t0 and the walkers take
 *     $t1 (target's assignment).  Measured priorities in this form:
 *     bits_left 17313 ($a3) > counter 14000 ($t0) > dst2 13928 ($t1) >
 *     dst 12580 ($t1).  Moving either walker init one statement later pushes
 *     that walker above the counter and the whole $t file rotates.
 *
 * Register assignment now MATCHES target exactly: $a3 = bits_left,
 * $t0 = counter, $t1 = walking pointer, $t2 = 0x20, $t3 = base,
 * $t4 = field width, $a2 = cur, $a1 = arg1, $a0 = needed.
 *
 * Residual at score 20 (all of it inside the two bit loops):
 *   D2  $v0/$v1 mirror.  Target keeps `hi` in $v1 SHARING the register of the
 *       `0x20 - bits_left` temp (`srlv $v1,$a2,$v1`) and puts `val` +
 *       `cur >> bits_left` in $v0.  We produce the mirror: temp $v1, hi $v0,
 *       val $v1, `cur >> bits_left` $v1.  The share fails because our `val`
 *       is still live after the srlv (it is re-assigned later in the arm), so
 *       its quantity conflicts with hi's.
 *   D4  preheader order + walker-init placement.  Target emits
 *       `move $t0,zero ; li $t4,<w> ; li $t2,0x20 ; move $t1,$t3` with the
 *       walker init LAST; we emit the walker init first (loop 2) or hoisted
 *       above the `sw`/`lw` preamble (loop 1), and the two `li`s in the
 *       opposite order.  The early init is currently load-bearing for the
 *       allocno priority (point 3), so D4 and the priority requirement are
 *       coupled - see hypotheses.md F1.
 *
 * SESSION 3 (structural) — body UNCHANGED, floor still 20. Session 3 probed
 * eleven structural variants of this body (tie the shift amount into `hi`;
 * a third named local for it, declared first and last; the same tie through
 * the dead `out` walker; three refill-placement moves; `val` carrying the
 * third temp, or the shifted `hi`; `needed` hoisted to the top of the arm; a
 * separate `-2` holder for the third loop) and every one of them scored
 * 20-45, i.e. neutral or worse. This body is a local optimum for the arm.
 *
 * Session 3's substantive result is a MECHANISM CORRECTION that the next
 * session should work from: the D2 `$v0`/`$v1` mirror is NOT a local-alloc
 * `combine_regs` tie (the BB2_QTY_DEBUG local-alloc tables are byte-identical
 * between this form and a form whose registers differ). It is decided in
 * global.c `allocno_compare`, between exactly two pseudos:
 *
 *     val  -> pseudo 83, nrefs 19, livelen 15, pri 50666  -> takes $v1
 *     hi   -> pseudo 78, nrefs  8, livelen 18, pri 13333  -> takes $v0
 *
 * Target needs the reverse. `val`'s nrefs is 19 because this body reuses the
 * one local three ways (shift amount, the `0x20 - needed` D1 intermediate,
 * and the third loop's `-2`). Removing the `-2` reuse does lower the priority
 * — and also destroys D1 (build_insns falls back to 75), so the two are
 * coupled. The reachable direction is to ADD references to `hi`, not to
 * remove them from `val`. Full numbers, tooling and the eleven measurements
 * are in evidence.md / hypotheses.md (session 3).
 *
 * Also settled in session 3: the duplicated `dst += 2` is verifiably
 * BYTE-NEUTRAL — target itself emits a single `addiu $t1,$t1,0x2` in the
 * bnez delay slot (asm/funcs/func_8001979C.s:45 and :67) because cross-jump
 * re-merges the two arm tails, and our build emits exactly one too. That
 * discharges the byte-neutrality prerequisite of the
 * duplicated-statement-into-arms family for the eventual self-vet.
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

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];
    dst = base;

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    do {
        if (bits_left < 0xC) {
            val = 0x20 - bits_left;
            hi = cur >> val;
            cur = *arg1;
            arg1++;
            needed = 0xC - bits_left;
            val = 0x20 - needed;
            bits_left = val;
            *(s16 *)(dst + 0xA) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
            dst += 2;
        } else {
            *(s16 *)(dst + 0xA) = (s16)(cur >> 20);
            cur <<= 0xC;
            bits_left -= 0xC;
            dst += 2;
        }
        i++;
    } while (i < 0x3F);

    dst2 = base;
    i = 0;
    do {
        if (bits_left < 2) {
            val = 0x20 - bits_left;
            hi = cur >> val;
            cur = *arg1;
            arg1++;
            needed = 2 - bits_left;
            val = 0x20 - needed;
            bits_left = val;
            *(s16 *)(dst2 + 0x8E) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
            dst2 += 2;
        } else {
            *(s16 *)(dst2 + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
            dst2 += 2;
        }
        i++;
    } while (i < 0x3F);

    val = -2;
    i = 3;
    out = base + 0x348;
    do {
        *(s32 *)(out + 0x110) = val;
        i--;
        out -= 0x118;
    } while (i >= 0);
    *(s32 *)(base + 0x10C) = 0;
}
