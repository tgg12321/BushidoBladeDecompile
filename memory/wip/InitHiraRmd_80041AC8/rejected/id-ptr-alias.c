/* REJECTED — cheat-reviewer FAIL 2026-06-10. DO NOT RE-DERIVE.
 *
 * Form: pointer-local alias for the post-store reload. Sandbox 0,
 * build_insns 75 = target, full-register objdump diff 0, retire SHA1 ==
 * oracle. Mechanically perfect — and a cheat.
 *
 * Violated tests (reviewer verdict): Test 2 (human-programmer: a human
 * caches the VALUE, not the ADDRESS, of an element already accessible as
 * arg0[4]), Test 3 (justification is pure GCC internals: expr.c
 * INDIRECT_REF marks pointer-indexed loads MEM_IN_STRUCT_P=1, firing
 * sched.c true_dependence's in-struct escape so sched1 hoists the reload
 * above `sh D_800A9A20`; a plain pointer-var deref keeps /s=0 so the
 * dependence holds), Test 5 (family: alias-with-no-semantic-content,
 * same shape as the forbidden param-local-alias / alias-rename family).
 * defeat-combine-symbol-fold does NOT cover it (that rule's flag_ptr is
 * a DISPLACED pointer pre-computation; this is a zero-displacement alias).
 */
  s16 *id_ptr;
  /* ... */
  id_ptr = &arg0[4];
  D_800A9A20 = arg0[4];
  var_s0 = (s16 *) D_80094DF0[D_80094E08[*id_ptr]];
