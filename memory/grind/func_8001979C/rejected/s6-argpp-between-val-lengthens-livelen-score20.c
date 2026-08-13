/* func_8001979C - BEST FORM as of session 5 (permuter).  Floor: sandbox
 * --disable all = 12 (target_insns 77, build_insns 77) - down from 18 at the
 * end of session 4, 20 at the end of sessions 2/3, 24 at recon.  This form is
 * IN PLACE in src/code6cac.c at the end of session 5.
 *
 * WHAT CHANGED IN SESSION 5 (two edits, both inside the refill arm of each bit
 * loop; the session-2/3 skeleton below is otherwise untouched):
 *
 *   A. The merged field value is routed through `hi` before the store:
 *          hi = (hi << needed) | (cur >> bits_left);
 *          *(s16 *)(dst + 0xA) = (s16)hi;
 *      instead of storing the expression directly.  This is session 3's
 *      never-executed F1(a) probe.  It gives `hi` two more references per arm
 *      (weighted x2 in-loop), taking it from nrefs 8 to nrefs 16 - across a
 *      floor_log2 bucket, exactly as the session-4 arithmetic predicted - so
 *      global.c allocno_compare now ranks `hi` (pri 32000) above the
 *      short-lived temps and it takes $v1.  Floor 18 -> 16.
 *
 *   B. With `hi` in $v1, session 3's vC tie becomes correct instead of
 *      mirrored: the `0x20 - bits_left` shift amount is computed INTO `hi`
 *      and immediately consumed by the shift
 *          hi = 0x20 - bits_left;
 *          hi = cur >> hi;
 *      which reproduces target's `subu $v1,$t2,$a3 ; srlv $v1,$a2,$v1`
 *      (one register for both).  In session 3 this landed in $v0 and cost 2
 *      points; at the session-5 allocation it lands in $v1 and buys 4.
 *      Floor 16 -> 12.  D2 IS NOW CLOSED: both bit loops are byte-identical
 *      to target through the whole shift/or/store chain.
 *
 *   Edit B also DELETES the session-4 "carrier" construct (the `0x20 -
 *   bits_left` shift amount computed into the other loop's walking pointer and
 *   read back through `val`).  That construct was the ledger's open policy
 *   question - writing a later-live walking pointer with an unrelated value
 *   purely for allocation pressure - and it is simply gone from this form.
 *
 * Residual at score 12 (three families, all measured, none inside the arm's
 * shift/or chain any more):
 *   D4 (4 pts) - our preheader emits `move $t1,$t3` (the walker init) BEFORE
 *       the `sw`/`lw` preamble; target emits it LAST.  Unchanged since s2;
 *       the init placement is load-bearing for the walker/counter priority
 *       window (moving it late is session 2's P6).
 *   D5 (4 pts, NEW) - inside each arm target emits `sllv $a2,$a2,$a0`
 *       (`cur <<= needed`) BEFORE the `or`; we emit the `or` first.  Pure
 *       scheduling: moving the C statement above the store is codegen-neutral
 *       (vTieShiftEarly, 12).
 *   D6 (4 pts, NEW) - the third (`-2` fill) loop is now $v0/$v1 mirrored
 *       (target `li $v1,-2` + `addiu $v0,$t3,0x348`, ours the reverse).  It
 *       matched at scores 20 and 18; the session-5 allocation change flipped
 *       it.  `val` (which holds the -2) is now the $v0 allocno.
 *
 * Register assignment matches target exactly for every long-lived value:
 * $a3 = bits_left, $t0 = counter, $t1 = walking pointer, $t2 = 0x20,
 * $t3 = base, $t4 = field width, $a2 = cur, $a1 = arg1, $a0 = needed,
 * $v1 = hi.
 *
 * POLICY NOTE (still unsettled - read before any candidate-ready submission).
 * The session-4 carrier construct is gone, but TWO constructs still need a
 * family classification with a VERBATIM scope quote plus a file:line or
 * commit-hash precedent in self_vet.md (or a ruling-request instead):
 *   (i)  the duplicated `dst += 2` in both if-arms (byte-neutrality proven in
 *        session 3: cross-jump re-merges it, target and our build both carry a
 *        single `addiu $t1,$t1,0x2` in the bnez delay slot);
 *   (ii) the reuse of `hi` as the shift-amount carrier and of `val` as both
 *        the D1 intermediate and the third loop's `-2` holder ("variable reuse
 *        for codegen control" - a frozen-list family, but the claim is not yet
 *        backed by a quoted scope sentence + precedent in this ledger).
 *
 * ---------------------------------------------------------------- sessions 2-3
 *  1. `dst += 2` is duplicated into BOTH if-arms instead of sitting once at
 *     the loop bottom.  This gives the walking-pointer biv TWO increments, so
 *     loop.c computes `benefit -= add_cost * bl->biv_count` with biv_count 2,
 *     the combined DEST_ADDR giv's benefit falls to <= 0, strength_reduce sets
 *     v->ignore, all_reduced goes 0 and maybe_eliminate_biv is never reached.
 *     The biv therefore survives and the +0xA / +0x8E stays a store
 *     displacement - target's exact D3 form - while both `sh` instructions are
 *     kept.
 *
 *  2. `val` carries `0x20 - needed` (session 1's confirmed D1 lever: it
 *     materialises target's `subu $v0,$t2,$a0` + `addu $a3,$v0,$zero` pair)
 *     and is reused for the third loop's `-2`.  Removing either use costs the
 *     D1 pair (session 3's vL, session 5's vTieNeg2Hi / vTieNeg2Needed: all
 *     build_insns 75).
 *
 *  3. Per-loop walking pointers (`dst`, `dst2`, `out`) and their
 *     initialisations placed as early as their loop allows - allocno-priority
 *     arithmetic in global.c (priority = floor_log2(nrefs)*nrefs/livelen); the
 *     walkers must stay BELOW the loop counter so the counter takes $t0 and the
 *     walkers take $t1 (target's assignment).
 *
 *  4. `needed` is routed through the second local `nd` (session 4).  Still
 *     load-bearing at this floor: removing it costs 2 points (vTieNoNd, 14).
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

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];
    dst = base;

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    do {
        if (bits_left < 0xC) {
            nd = 0xC - bits_left;
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            needed = nd;
            val = 0x20 - needed;
            arg1++;
            bits_left = val;
            hi = (hi << needed) | (cur >> bits_left);
        *(s16 *)(dst + 0xA) = (s16)hi;
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
            nd = 2 - bits_left;
            hi = 0x20 - bits_left;
            hi = cur >> hi;
            cur = *arg1;
            needed = nd;
            val = 0x20 - needed;
            arg1++;
            bits_left = val;
            hi = (hi << needed) | (cur >> bits_left);
        *(s16 *)(dst2 + 0x8E) = (s16)hi;
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
