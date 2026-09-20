/* REJECTED (session 1, recon) — same regression as the two-var-split sibling
 * in this directory, tried from the opposite direction (initialize the
 * call-arg to the delay-slot-filled default, override in the branch, mirror
 * the asm's literal control flow instead of the value's data flow):
 *
 *     s32 flag = MENU_800747D8->field64;
 *     s32 sound = 4;
 *     if (flag != 0) { sound = 0; }
 *     func_8005C650(sound, 0x7F, 0x7F);
 *
 * MEASURED (session 1): sandbox --disable all score 10, build_insns 205 —
 * IDENTICAL score and insn count to the two-var-split form (rejected/
 * selection_sound-two-var-split.c), so GCC is folding both spellings to the
 * same (wrong-shape) output regardless of which branch holds the literal.
 * kill_scope: instance (this exact spelling, this chassis, no FAKE
 * constructs). Frontier note: baseline's `sound -= sound;` self-subtract
 * outperforms both natural two-local spellings by 4 score points and 3
 * insns — that is itself a smell (self-subtract instead of `= 0` is not
 * something a human would write from a spec) and is flagged for the next
 * session's cheat-vet pass once the residual closes further; for now it is
 * the closest known-honest baseline and was not touched.
 */
