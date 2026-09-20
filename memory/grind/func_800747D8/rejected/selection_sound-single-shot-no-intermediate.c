/* REJECTED (session 1, recon) — third respelling attempt at the same hunk
 * 17-19 residual, same result as its two siblings in this directory.
 * Removes even the `flag` local, testing the field directly:
 *
 *     s32 sound;
 *     if (MENU_800747D8->field64 == 0) { sound = 4; } else { sound = 0; }
 *     func_8005C650(sound, 0x7F, 0x7F);
 *
 * MEASURED (session 1): sandbox --disable all score 10, build_insns 205 —
 * IDENTICAL to both other two-value spellings. All three "natural" if/else
 * respellings collapse to the SAME wrong (3-insns-short) shape regardless of
 * how many named locals are introduced or which branch holds the literal;
 * the baseline's `sound -= sound;` self-subtract remains the only spelling
 * found so far that reaches build_insns == target_insns (208) with a small
 * (6-point) residual. kill_scope: instance (all three spellings, this
 * chassis, no FAKE constructs). This is reasonably strong evidence (3/3
 * natural forms fold identically) that whatever GCC pass is folding these to
 * 205 insns doesn't care about C-level variable count/placement here — next
 * session should read the .combine/.jump dumps for this block before trying
 * a 4th hand-written spelling blind.
 */
