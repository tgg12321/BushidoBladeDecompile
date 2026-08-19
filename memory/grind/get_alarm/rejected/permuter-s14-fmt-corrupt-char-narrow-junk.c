/* REJECTED — chassis-5 (s14) permuter sub-floor find output-500-1 (permuter score
 * 500 < 630 floor). CHEAT-BY-SPELLING, same family as banked
 * permuter-alias-longlong-junk.c. Three independent defects:
 *
 *  1. SEMANTICALLY WRONG format string. The first debug_printf's fmt arg becomes
 *     `new_var3 = new_var2` where new_var2 = &(*g_gpu_dma_madr) = g_gpu_dma_madr,
 *     NOT &g_str_gpu_timeout. The permuter's masked-Levenshtein scorer normalizes
 *     symbol refs, so swapping the fmt address for g_gpu_dma_madr false-matches
 *     more target bytes by accident (identical to the s1 arg-swap artifact and the
 *     s4 pointer-alias junk). It passes the WRONG format pointer at runtime.
 *  2. `char new_var = D_8009BF78 - D_8009BF7C;` — a width coercion (narrow to char),
 *     same forbidden family as the banked long-long width coercions
 *     (s4 permuter-alias-longlong-junk, s5 axisA-permuter-longlong-width-coercion,
 *     s13 (long long)D_8009BF70).
 *  3. `new_var3 = new_var2` — a dead pointer-alias self-assign with no semantic
 *     purpose (no human writes it); exists only to launder the fmt substitution.
 *
 * Axis A (D_8009BF68[0]) is UNTOUCHED here — still inline combine-folded to 2 insns.
 * The score improvement is entirely a fmt-corruption false-match, not a legitimate
 * axis-A materialization or axis-B reorder. Rejected per cheats-by-any-spelling
 * (.claude/rules/no-new-park-categories.md + inline-asm-policy).
 */
s32 func_8007DC9C(void)
{
  s32 temp_v0;
  s32 *new_var2;
  s32 *new_var3;
  s32 temp_v1;
  char new_var;
  new_var2 = &g_str_gpu_timeout;
  if ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0)))
  {
    new_var = D_8009BF78 - D_8009BF7C;
    new_var2 = &(*g_gpu_dma_madr);
    {
      s32 diff = new_var & 0x3F;
      s32 dead = *g_gpu_stat_reg;
      debug_printf(new_var3 = new_var2, diff, *g_gpu_stat_reg, *g_gpu_dma_chcr, *new_var2);
      (void) dead;
    }
    debug_printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
    temp_v0 = motion_make_table(0);
    D_8009BF7C = 0;
    D_8009BF88 = temp_v0;
    D_8009BF78 = D_8009BF7C;
    *g_gpu_dma_chcr = 0x401;
    *D_8009BF64 |= 0x800;
    *g_gpu_stat_reg = 0x02000000;
    *g_gpu_stat_reg = 0x01000000;
    motion_make_table(D_8009BF88);
    return -1;
  }
  return 0;
}
