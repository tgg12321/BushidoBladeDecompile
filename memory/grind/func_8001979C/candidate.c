/* func_8001979C - BEST FORM as of session 7 (forensics).  Floor: sandbox
 * --disable all = 4 (target_insns 77, build_insns 77) - down from 8 at the end
 * of session 6 (12 at s5, 18 at s4, 20 at s2/s3, 24 at recon).  This form is
 * IN PLACE in src/code6cac.c at the end of session 7.
 *
 * WHAT CHANGED IN SESSION 7 - the D5 family (4 of the 8 remaining points) is
 * CLOSED.  In each refill arm the single `hi = (hi << needed) | (cur >>
 * bits_left);` statement is split so that `cur <<= needed;` sits BETWEEN the
 * `cur >> bits_left` read and the OR:
 *
 *      hi = hi << needed;
 *      <carrier> = cur >> bits_left;
 *      cur <<= needed;
 *      hi = hi | <carrier>;
 *      *(s16 *)(dst + 0xA) = (s16)hi;
 *
 * The OR's operand ORDER is preserved (hi | carrier), so this is NOT the
 * or-tree reshape forbidden by .claude/rules/or-tree-shape-shift.md; only the
 * position of the independent `cur <<= needed` statement changes.
 *
 * D5's MECHANISM, NAMED EXACTLY (session-7 forensics, sched.c):
 *   In the refill arm's basic block the OR (insn 87) and `cur <<= needed`
 *   (insn 92) are BOTH ready at the same backward-scheduling step with
 *   INSN_PRIORITY == 2, and BB2_RANK_DEBUG shows they are also in the same
 *   dependency class (cls=3 vs the last scheduled insn, the store).  So
 *   sched.c's rank_for_schedule falls all the way through to its final
 *   tie-break at tools/gcc-2.7.2/sched.c:2464,
 *       return INSN_LUID (tmp) - INSN_LUID (tmp2);
 *   which prefers the LARGER LUID (the originally-later insn - scheduling runs
 *   backwards, so this minimises movement).  With the one-statement OR, the
 *   `cur <<= needed` RTL follows the OR, wins the tie and is emitted after it.
 *   Splitting the statement puts `cur <<= needed` BEFORE the OR in the insn
 *   stream, which inverts the tie-break and reproduces target's
 *   `sllv $a2,$a2,$a0 ; or $v1,$v1,$v0`.  Verified in both the .sched (pass 1)
 *   and .sched2 dumps - the tie is already decided in sched1.
 *
 * WHY THE CARRIERS ARE WHAT THEY ARE (this is the load-bearing part).  The
 * split costs `hi` two extra references per arm (nrefs 24 -> 32, weighted x2
 * in-loop).  With a carrier that is a fresh local live in BOTH loops, that new
 * pseudo becomes a global allocno competing for the $v0/$v1 pair and `hi`
 * loses $v1 - D2's 14-point mirror comes straight back (measured: score 20).
 * Reusing an allocno that ALREADY exists keeps the pair intact: loop 1's
 * carrier is `out` (the third loop's walking pointer, provably dead there -
 * `out = base + 0x348;` re-initialises it before the third loop) and loop 2's
 * carrier is the fresh `lo`, which stays local to loop 2 and never becomes a
 * global allocno at all.  ALLOCDBG for this form: pseudo 78 (`hi`) ord 0,
 * nrefs 32, livelen 24, pri 66666 -> hardreg 3 ($v1); pseudo 83 (the D1
 * intermediate `val`) ord 1, pri 60000 -> hardreg 2 ($v0); pseudo 82 (`out`)
 * ord 3, nrefs 11, livelen 8 -> hardreg 2.  The mirror-image assignment of
 * carriers (`lo` in loop 1, `out` in loop 2) scores 4 as well; `out` in BOTH
 * loops scores 24 and a fresh local in both scores 20.
 *
 * Residual at score 4 - ONE family, D4, unchanged since session 2:
 *   our preheader emits the walker init `addu $t1,$t3,$zero` FIRST, ahead of
 *   the `sw`/`lw` preamble, where target emits it LAST (after
 *   `addu $t0,$zero,$zero ; addiu $t4,zero,<w> ; addiu $t2,zero,0x20`).
 *   Two lines per loop.  Session 7 re-killed the late-init source form for the
 *   FOURTH time, now at this allocation: 4 -> 19 for late-init-both,
 *   late-init-loop1-only and late-init-loop2-only alike.
 *
 * Register assignment matches target exactly everywhere: $a3 = bits_left,
 * $t0 = counter, $t1 = walking pointer, $t2 = 0x20, $t3 = base, $t4 = field
 * width, $a2 = cur, $a1 = arg1, $a0 = needed, $v1 = hi, $v0 = val / carrier,
 * and in the third loop $v0 = out / $v1 = neg2.
 *
 * POLICY NOTE (unsettled - read before any candidate-ready submission).  Four
 * constructs need a family classification with a VERBATIM scope quote plus a
 * file:line or commit-hash precedent in self_vet.md (or a ruling-request):
 *   (i)   the duplicated `dst += 2` in both if-arms (byte-neutrality PROVEN in
 *         session 3 - cross-jump re-merges it and both target and our build
 *         carry a single `addiu $t1,$t1,0x2` in the bnez delay slot);
 *   (ii)  variable reuse: `hi` as its own shift-amount carrier, `val` as the
 *         D1 intermediate;
 *   (iii) routing the final `0` through `val` (s6);
 *   (iv)  NEW in s7 - using the third loop's walking pointer `out` as loop 1's
 *         OR carrier.  This is the session-4 "carrier" construct returning in
 *         a new place: a variable whose declared role is a pointer is written
 *         with an unrelated bitstream fragment purely because reusing an
 *         existing allocno preserves the $v0/$v1 assignment.  It is the
 *         construct in this form most likely to draw a "why is this here?"
 *         from a reviewer.  A policy-clean fallback exists and is banked:
 *         `val` in loop 1 + a fresh `lo` in loop 2 scores 5
 *         (rejected/s7-d5-val-plus-lo-carriers-score5-policy-clean.c).
 *
 * ---------------------------------------------------------------- sessions 2-6
 *  1. `dst += 2` duplicated into BOTH if-arms gives the walking-pointer biv TWO
 *     increments, so loop.c's `benefit -= add_cost * bl->biv_count` drives the
 *     combined DEST_ADDR giv's benefit to <= 0, all_reduced goes 0 and
 *     maybe_eliminate_biv never runs: the biv survives and +0xA / +0x8E stays a
 *     store displacement (target's D3 form) with both `sh`s kept.
 *  2. Per-loop walking pointers (`dst`, `dst2`, `out`) with each init as early
 *     as its loop allows - global.c allocno_compare priority
 *     floor_log2(nrefs)*nrefs/livelen; the walkers must stay BELOW the counter.
 *  3. `needed` routed through the second local `nd` (session 4).
 *  4. The merged field value routed through `hi`, and the `0x20 - bits_left`
 *     shift amount computed into `hi` itself (session 5) - that took `hi`
 *     across a floor_log2 bucket and closed D2's 14-point $v0/$v1 mirror.
 *  5. A separate `neg2` for the third loop (closes D6) paid for by giving `val`
 *     a LATER last use - the final zero store - because cse.c:7454's
 *     cheapest-register swap deletes D1's copy pair unless `val`'s last
 *     reference is later than `bits_left`'s (make_regs_eqv test, cse.c:856).
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
            arg1++;
            needed = nd;
            val = 0x20 - needed;
            bits_left = val;
            hi = hi << needed;
            out = cur >> bits_left;
            cur <<= needed;
            hi = hi | out;
            *(s16 *)(dst + 0xA) = (s16)hi;
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
            arg1++;
            needed = nd;
            val = 0x20 - needed;
            bits_left = val;
            hi = hi << needed;
            lo = cur >> bits_left;
            cur <<= needed;
            hi = hi | lo;
            *(s16 *)(dst2 + 0x8E) = (s16)hi;
            dst2 += 2;
        } else {
            *(s16 *)(dst2 + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
            dst2 += 2;
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
