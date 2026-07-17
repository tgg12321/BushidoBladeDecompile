/* REJECTED s2b (2026-07-17, structural): two remaining ordering axes on the
 * clean+stop floor-4 base, both measured INERT (sandbox = 4, unchanged):
 *   (a) uninitialized-decl order fully reversed
 *       (ret, cur_off, size, walker, v1 instead of v1, walker, size, cur_off, ret)
 *       — local pseudo numbering does not touch the entry-copy LUIDs.
 *   (b) initialized-decl permutation with dest's initializer FIRST
 *       (dest, slots, count) — changes RTL emission order of the derived-local
 *       defs (lw/addu before addiu s5/move s6), but sched1 renormalizes the
 *       entry block to the same schedule; the arg-copy LUID tie-break is
 *       unaffected.
 * KILLED: with H2 (initializers-as-statements = 13) and these two, the entire
 * declaration/initializer geometry axis is measured closed. The residual 4 is
 * exactly the first two save+def prologue pairs swapped (see
 * tmp/grind/hirahira_w_frie/s2/sandbox_objdump_floor4.txt lines 1455-1475 vs
 * asm/funcs/hirahira_w_frie.s:3-6).
 *
 * (a) fragment:
 *   s32 ret; s32 cur_off; s32 size; s32 *walker; s32 v1;
 * (b) fragment:
 *   s32 *dest = (s32 *)((s32)base + base[1]);
 *   s32 *slots = base + 1;
 *   s32 count = 0;
 */
