/* REJECTED (grind s2, 2026-08-03) — respelling the func_80073728 CALL ARGUMENTS
 * is completely inert.  GCC folds every named-local form back to the same RTL,
 * so the [[hoist-call-arg-local-flips-jal-delay]] recipe does NOT reach the
 * residual `addu $a1,$zero,$zero` placement (H5).
 *
 * Every variant below measured `sandbox --disable all` == 2 with insns 133,
 * i.e. IDENTICAL to the baseline — not worse, but not one point better either.
 * A no-change result is the informative one here: it says the a1 set-up's
 * position is fixed by the call expansion, not by how the argument is spelled.
 *
 *   (1) both arguments hoisted into locals at the top of the arm:
 *         S544 *sarg; s32 zarg;
 *         Case3:
 *             sarg = &s;
 *             zarg = 0;
 *             stat = (s32)(&D_8009B7D0);
 *             s.p_static = (s32 *)stat;
 *             s.pad0C = mid_off;
 *             mid_off = func_80073728(sarg, zarg);
 *
 *   (2) only the struct-pointer argument named (`sarg = &s;` at the top of the
 *       arm, `func_80073728(sarg, 0)`).
 *
 *   (3) the constant argument carried in a named local declared with the other
 *       locals and assigned at the top of the arm (`zero_arg = 0;`,
 *       `func_80073728(&s, zero_arg)`) — measured at the floor-4 base, also
 *       no change.  (This one is a constant-holder local, i.e. the
 *       [[named-local-fake-exception]] family; it was measured as a DIAGNOSTIC
 *       to learn the mechanism, never proposed as a closing form.  It is dead
 *       anyway.)
 *
 *   (4) the call RESULT routed through an extra pseudo
 *       (`midtmp = func_80073728(&s, 0); mid_off = midtmp;`) — no change.
 *
 *   (5) the pad0C STORED VALUE routed through an extra pseudo
 *       (`midtmp = mid_off; s.pad0C = midtmp;`) — no change.
 *
 * SEPARATELY REJECTED, and badly — a function-wide named struct pointer:
 *
 *   score 44, insns 137 — `S544 *sarg; sarg = &s;` in the prologue, used for
 *     BOTH func_80073728 and the in-loop func_8007352C calls.
 *   score 38, insns 137 — same, used only for the func_80073728 call.
 *   Both grow the function by four instructions: a long-lived pointer pseudo
 *     forces a callee-save home for `&s` instead of the per-call
 *     `addiu $a0,$sp,0x18` that target (and our baseline) emit.  Do not retry.
 */
