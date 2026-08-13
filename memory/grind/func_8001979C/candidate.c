/* func_8001979C - BEST FORM as of session 6 (forensics).  Floor: sandbox
 * --disable all = 8 (target_insns 77, build_insns 77) - down from 12 at the end
 * of session 5, 18 at s4, 20 at s2/s3, 24 at recon.  This form is IN PLACE in
 * src/code6cac.c at the end of session 6.
 *
 * WHAT CHANGED IN SESSION 6 (two coupled edits, both outside the bit loops):
 *
 *   A. The third loop's `-2` gets its OWN local `neg2` instead of reusing
 *      `val`.  This closes D6: the third loop is now byte-identical to target
 *      (`addiu $v1,$zero,-2 ; addiu $t0,$zero,3 ; addiu $v0,$t3,0x348 ;
 *      sw $v1,0x110($v0)`).  Sessions 3-5 measured this five ways and rejected
 *      it every time because on its own it costs D1 (build_insns 75).
 *
 *   B. `val` is given a use AFTER the third loop - it carries the final zero:
 *          val = 0;
 *          *(s32 *)(base + 0x10C) = val;
 *      This restores D1 (`subu $v0,$t2,$a0 ; addu $a3,$v0,$zero` in both
 *      loops), so A becomes affordable.  See the mechanism below; the two
 *      edits are only useful together (A alone = 12/build 75, and B is
 *      meaningless without A).
 *
 * THE D1 MECHANISM, NAMED EXACTLY (session-6 forensics; this replaces every
 * earlier "D1 is an allocation/ref-count effect" statement in the ledger).
 * D1's copy is created or destroyed in **cse.c**, not in the allocator:
 *
 *   cse.c:7454 "Special handling for (set REG0 REG1) where REG0 is the
 *   cheapest" rewrites `val = 0x20 - needed; bits_left = val;` into a single
 *   `subu bits_left,...` - the copy vanishes - but ONLY when
 *       qty_first_reg[reg_qty[val]] == bits_left,
 *   i.e. only when cse made `bits_left` the canonical register of the
 *   quantity.  make_regs_eqv (cse.c:826-863) makes the new register canonical
 *   only if it lives past the current basic block AND
 *       uid_cuid[regno_last_uid[bits_left]] > uid_cuid[regno_last_uid[val]]
 *   (cse.c:856).  So the copy SURVIVES exactly when `val` has a reference
 *   LATER IN THE FUNCTION than `bits_left`'s last reference.  In sessions 2-5
 *   that later reference was the third loop's `-2`; edit B replaces it with
 *   the final zero store, which is later still.
 *
 * Residual at score 8 - two 4-point families, both already characterised:
 *   D4 (4 pts) - our preheader emits the walker init `move $t1,$t3` BEFORE the
 *       `sw`/`lw` preamble; target emits it LAST.  Unchanged since s2 and
 *       re-killed at three different allocations (P6 at floor 20, vTieLateInit
 *       at floor 12, s6 vLateInit at floor 8).
 *   D5 (4 pts) - inside each arm target emits `sllv $a2,$a2,$a0`
 *       (`cur <<= needed`) BEFORE the `or`; we emit the `or` first.  Pure
 *       scheduling: moving the C statement is codegen-neutral (s5
 *       vTieShiftEarly).
 *
 * Register assignment matches target exactly everywhere: $a3 = bits_left,
 * $t0 = counter, $t1 = walking pointer, $t2 = 0x20, $t3 = base, $t4 = field
 * width, $a2 = cur, $a1 = arg1, $a0 = needed, $v1 = hi, $v0 = val, and in the
 * third loop $v0 = out / $v1 = neg2.
 *
 * POLICY NOTE (unsettled - read before any candidate-ready submission).  Three
 * constructs need a family classification with a VERBATIM scope quote plus a
 * file:line or commit-hash precedent in self_vet.md (or a ruling-request):
 *   (i)   the duplicated `dst += 2` in both if-arms (byte-neutrality PROVEN in
 *         session 3 - cross-jump re-merges it and both target and our build
 *         carry a single `addiu $t1,$t1,0x2` in the bnez delay slot);
 *   (ii)  variable reuse: `hi` as its own shift-amount carrier, `val` as the
 *         D1 intermediate;
 *   (iii) NEW in s6 - routing the final `0` through `val`
 *         (`val = 0; *(s32 *)(base + 0x10C) = val;`).  The store is REAL and
 *         its value is real, so this is not a dead store; it is the
 *         "variable reuse for codegen control" family.  It is nevertheless
 *         the construct in this form a reviewer is most likely to ask "why is
 *         this here?" about, and it has NOT been cleared.
 *
 * ---------------------------------------------------------------- sessions 2-5
 *  1. `dst += 2` is duplicated into BOTH if-arms instead of sitting once at the
 *     loop bottom.  This gives the walking-pointer biv TWO increments, so
 *     loop.c computes `benefit -= add_cost * bl->biv_count` with biv_count 2,
 *     the combined DEST_ADDR giv's benefit falls to <= 0, strength_reduce sets
 *     v->ignore, all_reduced goes 0 and maybe_eliminate_biv is never reached.
 *     The biv survives and the +0xA / +0x8E stays a store displacement
 *     (target's D3 form) while both `sh` instructions are kept.
 *
 *  2. Per-loop walking pointers (`dst`, `dst2`, `out`) with each
 *     initialisation placed as early as its loop allows - global.c
 *     allocno_compare priority floor_log2(nrefs)*nrefs/livelen; the walkers
 *     must stay BELOW the loop counter so the counter takes $t0.
 *
 *  3. `needed` is routed through the second local `nd` (session 4); removing
 *     it cost 2 points at floor 12.
 *
 *  4. The merged field value is routed through `hi` before the store and the
 *     `0x20 - bits_left` shift amount is computed into `hi` itself (session 5).
 *     That took `hi` across a floor_log2 bucket (nrefs 8 -> 16, pri 13333 ->
 *     32000), gave it $v1, and closed D2's 14-point $v0/$v1 mirror.
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
            arg1++;
            needed = nd;
            val = 0x20 - needed;
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
