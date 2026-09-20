/* REJECTED (session 1, recon) — regressed the honest floor.
 * Baseline (candidate.c): `s32 sound = field64; if (sound==0) sound+=4; else sound-=sound;`
 * measures sandbox --disable all score 6, build_insns 208 (== target_insns).
 *
 * Motivation: hunks 17-19 of the classed diff (target insns 99/101/103) show
 * target computes the test value into $v0 (`lbu v0,0x64(v0)`) and sets the
 * call-arg $a0 separately per branch (delay-slot `addiu a0,zero,4` on the
 * taken path, explicit `addu a0,zero,zero` on the fallthrough) — i.e. two
 * logically distinct quantities (a flag/test value and a result value), while
 * our build's `lbu a0,100(v0)` computes directly into the call-arg register.
 * Tried splitting into two real C locals so the test value and the result
 * value are visibly distinct, matching that structural read:
 *
 *     s32 flag = MENU_800747D8->field64;
 *     s32 sound;
 *     if (flag == 0) { sound = 4; } else { sound = 0; }
 *     func_8005C650(sound, 0x7F, 0x7F);
 *
 * MEASURED (session 1): sandbox --disable all score 10, build_insns 205 (3
 * insns SHORT of target's 208) — worse on both axes. GCC folded the
 * flag/sound split down to fewer instructions than either the baseline or
 * the target, so this is not simply "the compiler needs two named locals" —
 * something about the fold environment differs from what produces target's
 * shape. kill_scope: instance (this exact two-local if/else spelling, on
 * this chassis, no FAKE constructs, candidate.c otherwise unchanged).
 */
