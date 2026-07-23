/* REJECTED (s4, permuter modality, 2026-07-23) — the floor-1 permuter's ONLY
 * route to score 0 is the forbidden or-tree-shape-shift operand swap.
 *
 * Campaign: permuter_campaign.py from the floor-1 base.c (candidate.c form),
 * clean single-function workspace (base.o == floor-1 bytes, func at offset 0,
 * target.o from asm/funcs/ at offset 0). base weighted score = 10 (the one
 * operand-order instruction). Two fresh-seed windows:
 *   window 1 (--stop-on-zero): score 0 at iteration 17 -> output-0-1.
 *   window 2 (no stop-on-zero, ~2378 iters): 13 total output-0-* dirs,
 *     6 distinct normalized forms.
 * EVERY one of the 13 zeros contains the same load-bearing mutation:
 *     arg0 = base + off;   ->   arg0 = off + base;
 * The 6 "distinct" variants differ ONLY by permuter cosmetic noise layered on
 * top of the swap (a `new_var` alias temp for *(s32*)arg0; a redundant
 * `(long long)` cast; a `do { arg0 += 2; } while (0)` wrapper) — none of these
 * is the closer; the swap is present in all. Random search NEVER lowered the
 * score below 10 by any non-swap route.
 *
 * VETTING (no-new-park-categories 4-test checklist): the swap `off + base` vs
 * `base + off` computes the identical value; its ONLY effect is to flip the
 * commutative addu operand emission (build addu v1,v1,v0 -> target addu
 * v1,v0,v1). No semantic purpose; a human would not distinguish them; the
 * justification references GCC operand canonicalization. => or-tree-shape-shift,
 * FORBIDDEN (byte-0). Same construct already banked as
 * rejected/offbase-operand-shuffle-cheat.c (s3). The permuter merely
 * re-confirmed it exhaustively.
 *
 * CONCLUSION: the permuter axis is DEAD. No legitimate (non-swap) off-first
 * form keeping the walker in v1 exists in the permuter's reach — corroborating
 * the s3 structural proof that off-first and v1-walker are mutually exclusive
 * except via the swap. Representative found form below (output-0-1). */
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base, count, entry, a, b, c, d, off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    arg0 = off + base;          /* CHEAT: or-tree-shape-shift commutative swap */
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
