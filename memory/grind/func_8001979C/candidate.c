/* func_8001979C - BEST FORM as of session 4 (permuter). Floor: sandbox
 * --disable all = 18 (target_insns 77, build_insns 77) - down from 20 at the
 * end of sessions 2 and 3.  This form is IN PLACE in src/code6cac.c at the end
 * of session 4.
 *
 * It is the session-2/3 form (whose three load-bearing properties are
 * unchanged and still documented below) plus ONE new edit per bit loop, found
 * by the session-4 permuter campaign and then minimised by hand:
 *
 *   4. In each bit loop's refill arm, the `0x20 - bits_left` shift amount is
 *      computed into the OTHER loop's walking pointer - loop 1 writes it into
 *      `dst2`, loop 2 into `dst` - and `val` then reads it (`val = dst2;`).
 *      Each walker is provably dead at that point (`dst2 = base;` re-initialises
 *      it before loop 2, and `dst` is never read after loop 1), so the program
 *      is unchanged.  `needed` is likewise routed through a second local `nd`.
 *      Measured decomposition (engine gradient, sandbox --disable all):
 *        carrier chain in loop 1 alone ............ 20 (neutral)
 *        `nd` holder in loop 1 alone .............. 20 (neutral)
 *        both, loop 1 only (vE) ................... 19
 *        both, both loops (vE3, THIS form) ........ 18
 *      i.e. the effect is cumulative allocation pressure, not a single
 *      instruction being fixed; neither half is load-bearing on its own.
 *      The 2 points bought are in D4 (the preheader/walker-init block), NOT in
 *      D2: the ALLOCDBG numbers for the D2 pair are bit-for-bit unchanged
 *      (`val` = pseudo 83, ord 1, nrefs 19, livelen 15, pri 50666 -> $v1;
 *      `hi` = pseudo 78, ord 7, nrefs 8, livelen 18, pri 13333 -> $v0), which
 *      also proves the carrier reuse does NOT move references off `val` -
 *      `val = dst2;` keeps every one of val's 19 refs and adds refs to the
 *      walker instead.  D2's 14 points are therefore fully intact and remain
 *      the top frontier item.
 *
 * POLICY NOTE (unsettled - read before any candidate-ready submission).  Two
 * constructs in this body have an open family question and MUST be classified
 * in self_vet.md, with a verbatim scope quote plus a file:line/commit
 * precedent, before submission - or a ruling-request emitted instead:
 *   (i)  the duplicated `dst += 2` (byte-neutrality already proven, session 3);
 *   (ii) the session-4 carrier reuse (writing a live-later walking pointer with
 *        an unrelated value purely for allocation pressure).  It plausibly sits
 *        in the frozen "variable reuse for codegen control" family, but that is
 *        a CLAIM, not a finding: nothing in this ledger has cleared it.
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
 *  2. `val` carries BOTH `0x20 - bits_left` (the hi shift amount) and
 *     `0x20 - needed` (the new bits_left).  The second use is session 1's
 *     confirmed D1 lever (it materialises target's `subu $v0,$t2,$a0` +
 *     `addu $a3,$v0,$zero` pair); reusing the same variable for the first keeps
 *     `hi` out of the $t register file (anonymous temp: 31, this: 20/18).
 *
 *  3. Per-loop walking pointers (`dst`, `dst2`, `out`) and their
 *     initialisations placed as early as their loop allows - allocno-priority
 *     arithmetic in global.c (priority = floor_log2(nrefs)*nrefs/livelen); the
 *     walkers must stay BELOW the loop counter so the counter takes $t0 and the
 *     walkers take $t1 (target's assignment).
 *
 * Register assignment matches target exactly: $a3 = bits_left, $t0 = counter,
 * $t1 = walking pointer, $t2 = 0x20, $t3 = base, $t4 = field width, $a2 = cur,
 * $a1 = arg1, $a0 = needed.
 *
 * Residual at score 18 (all of it inside the two bit loops):
 *   D2 (14 pts) - the $v0/$v1 mirror.  Target keeps `hi` in $v1 sharing the
 *       register of the `0x20 - bits_left` temp (`srlv $v1,$a2,$v1`) and puts
 *       `val` + `cur >> bits_left` in $v0; we produce the mirror.  Decided in
 *       global.c allocno_compare between `val` and `hi` (session 3, H3-F).
 *   D4 (4 pts, was 6) - the preheader emits our `move $t1,$t3` before the
 *       `sw`/`lw` preamble instead of last, in both loops.  The `li` order is
 *       no longer a difference in this form.
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
            dst2 = 0x20 - bits_left;
            val = dst2;
            hi = cur >> val;
            cur = *arg1;
            arg1++;
            needed = nd;
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
            nd = 2 - bits_left;
            dst = 0x20 - bits_left;
            val = dst;
            hi = cur >> val;
            cur = *arg1;
            arg1++;
            needed = nd;
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
