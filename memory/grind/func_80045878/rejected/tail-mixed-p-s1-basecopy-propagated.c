/* func_80045878 — REJECTED (Gap B base-copy lever, s2 2026-07-23). structural.
 *
 * Goal: materialize target's tail base copy `addu v0,s1,zero` (stores via a
 *   caller-save base copy) by keeping s1 the long-lived record pseudo while a
 *   separate tail base survives copy-prop (frontier H-B).
 *
 * Form tried (the tail only; rest = HEAD):
 *     {
 *         s16 *p = s1;
 *         p[11] = a0 + 3; p[2] = a0; p[4] = a1; p[10] = a0; p[8] = a0;
 *     }
 *     *((s32 *)(((s32) s1) + 0x18)) = 0x8000;   // s1 still referenced -> live
 *
 * RESULT (sandbox --disable all): score 10, build_insns 107 (== HEAD, no move).
 *
 * WHY killed: cse still copy-propagates `p = s1` — even with s1 referenced in
 *   the trailing word store, `p` and `s1` are the same available value, so cse
 *   substitutes s1 for every p[] use and the copy vanishes (identical to the
 *   WIP single-set `s16 *p = s1` result). The target's `addu v0,s1,zero` is a
 *   local_alloc LIVE-RANGE-SPLIT artifact of the join block (.L800459DC has two
 *   preds: the `j` from 0x80045944 + fall-through), NOT a C-level pointer copy.
 *   No clean C copy structure reproduces it — the copy source is always
 *   available so cse always propagates. Also tried: SI temp for a0+3
 *   (`s32 t=a0+3; s1[11]=t;`) -> build 106 (removes the (HI)a0 truncation) but
 *   score stays 10; not the target's direction either.
 */
