/* func_80048530 — BYTE-0 pure-C form (s5, 2026-08-20).
 * sandbox --disable all = 0; build_insns 47 == target_insns 47;
 * rules_dropped 5 (the legacy regfix stack is now retirable).
 *
 * This is the s3 floor-1 candidate (fresh-assign walker + walking-pointer
 * c/d reads — see s3 evidence for why that shape is load-bearing) with the
 * sole residual closed by the off-first operand order `arg0 = off + base`,
 * sanctioned by the 2026-08-20 owner ruling (or-tree-shape-shift carve-out:
 * single justified target-matching operand order;
 * .claude/rules/or-tree-shape-shift.md:143-196; commit a5a5c1ec; this
 * function explicitly unparked to spend it, commit 04a187a7).
 *
 * Carve-out compliance (all five prerequisites):
 *  1. lever exhaustion: ledger s1-s4 (natural order = 1 off; all non-swap
 *     off-first spellings misroute 22/20/12; permuter 13/13 zeros = swap;
 *     canonical-asm LOW 1/8 refused; cc1psx also base-first).
 *  2. mechanism dump-proven: expand_binop preserves source operand order
 *     for two equal-precedence commutative pseudos; combine/greg never
 *     swap; greg insn 28 = (set v1 (plus v0 v1)) -> addu $v1,$v0,$v1
 *     (tmp/grind/func_80048530/s5/mechanism-proof.md).
 *  3. FAKE annotation at the site (below).
 *  4. commutative semantics identical; insn-count neutral (47 == 47).
 *  5. layer-1/layer-2 review via the pipeline (self_vet.md written s5).
 *
 * MEASUREMENT CAUTION (s3): the score is sensitive to the c/d spelling —
 * always the walking-pointer read (`c=*p; p+=2; d=*p;`), never a fixed
 * `*(p+2)` offset, or the walker misroutes and c folds to lh. */
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base, count, entry, a, b, c, d, off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    /* FAKE: operand order chosen to match target (off + base, not base + off);
     * mechanism: RTL expansion's commutative-operand canonicalization
     * (expand_binop) keeps two equal-precedence pseudos in source order, and
     * no later pass (combine/sched) reorders the addu operands — so only the
     * off-first source spelling emits target's `addu $v1,$v0,$v1`;
     * lever-exhaustion: memory/grind/func_80048530/ s1-s4 — every natural
     * ordering and every non-swap off-first spelling measured dead
     * (natural base+off = 1 insn off; off+=base / mem-inline / fresh-walker
     * misroute the walker, scores 22/20/12; cc1psx also emits base-first from
     * the natural order); sanctioned by the 2026-08-20 owner ruling in
     * .claude/rules/or-tree-shape-shift.md (single justified target-matching
     * operand order). */
    arg0 = off + base;
    count = *(s32 *)arg0;
    arg0 += 4;
    if (arg2 >= (u32)count) return -1;
    arg0 += arg2 * 0xC;
    entry = *(s32 *)arg0;
    arg0 += 4;
    a = (s32)*(u16 *)arg0;
    arg0 += 2;
    b = (s32)*(u16 *)arg0;
    arg0 += 2;
    c = (s32)*(u16 *)arg0;
    arg0 += 2;
    d = (s32)*(u16 *)arg0;
    entry += base;
    func_800485EC(entry, arg3, (s16)a, (s16)b, (s16)c, (s16)d);
    return count;
}
