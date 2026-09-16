/* REJECTED (s18, 2026-09-16, solver modality). KILLED (instance) — RE-CONFIRMATION
 * of the s10/s12-banked "loop-carried idx2" kill on the CURRENT (post-s14) 42/204
 * chassis, per the ledger's mandatory kill-re-audit (floor was flat 3 sessions
 * s15-s17 before this one).
 *
 * BACKGROUND: s10 (floor 58/198) and s12 (floor 48/198, pointer-carried variant)
 * both killed every spelling of "share i*2 as one C value" (fresh int, loop-
 * carried int, fresh pointer, loop-carried pointer) as WORSE than plain
 * `(&D_x)[i*2]` array indexing. Those measurements predate s14's enumerate-
 * modality win that dropped the floor 48 -> 42/204 via an unrelated reordering,
 * so per the ledger's "chassis changed, re-measure before spending" rule this
 * kill needed re-confirmation on the current body before being trusted further.
 *
 * THIS SESSION: ran `inverse_compose.py classify` fresh on the s17-banked
 * 42/197 chassis — reproduced the unchanged PRE-RA/rtl_shape verdict, then read
 * asm/funcs/func_80056CB8.s directly (full prologue-to-loop-tail region, lines
 * 1-217) to nail down exactly what target's $fp register is. CONFIRMED: $fp is
 * initialized ONCE before the loop as `sll $fp, $v1, 2` (v1 = the raw 0-3
 * facing/select value BEFORE the *2 scale our C's `start` already carries —
 * i.e. $fp = start*2, matching our loop var `i`'s *2-scaled semantics exactly),
 * and incremented by a bare `addiu $fp, $fp, 0x2` at the loop bottom
 * (.L80056FB0), read at BOTH the D_8009A821 lookup (`addu $at,$at,$fp` before
 * the flags lbu) and the D_8009A820 lookup (`addu $at,$at,$fp` before the scale
 * lbu). This is precisely the s10/s12-described genuine loop-carried i*2
 * accumulator, confirmed line-for-line against the CURRENT chassis's target.
 *
 * RE-MEASURED variant (b) from s10 (loop-carried INT induction, mirroring
 * target's structure at the integer level: `s32 idx2; ... for (i = start,
 * idx2 = start * 2; i < start + 2; i++, idx2 += 2) { ...[idx2]... }`) on the
 * CURRENT candidate.c body (s17-banked, unmodified otherwise):
 *   sandbox func_80056CB8 --disable all: score 42 -> 50 (WORSE),
 *   build_insns 197 -> 200 (MORE real instructions, not fewer).
 * Reverted immediately; re-confirmed floor 42/204 exactly reproduces after
 * revert (fresh sandbox run, unmodified body).
 *
 * No FAKE construct present in either the baseline or the tested variant, so
 * tools/fake_ablate.py has nothing to ablate for this kill.
 *
 * CONCLUSION (class-level mechanism, not just this instance): GCC's
 * strength-reduction "is this giv worth reducing" decision
 * (tools/gcc-2.7.2/loop.c:3823, `if (v->lifetime * threshold * benefit <
 * insn_count && ! bl->reversed) { ... v->ignore = 1; ... }`) gates SOLELY on
 * `insn_count` — the loop body's total emitted-instruction count computed
 * internally by GCC — not on which C-level construct names or carries the
 * candidate induction variable's VALUE. Every spelling that keeps the same
 * semantic value (i*2) without shrinking the loop body's overall instruction
 * count feeds the identical `insn_count` into that comparison and therefore
 * cannot flip GCC's decision. This is why all four spellings tried across two
 * chassis generations (58/198 at s10/s12, 42/197 at s18) land WORSE, never
 * better: promoting i*2 to its own named/carried C object doesn't shrink
 * insn_count, it grows it (register pressure / extra increment insns), which
 * moves AWAY from the threshold, not toward it. The lever this residual
 * actually needs is shrinking/reshaping the loop body's total instruction
 * count enough to cross the loop.c:3823 threshold — not re-spelling the index.
 *
 * Do not re-propose any spelling of "i*2 lives in one shared/carried C
 * handle" for this residual on any future chassis derived from the current
 * body shape; re-test only if the loop body's overall instruction count
 * changes substantially (e.g. an unrelated structural win elsewhere in the
 * loop) — src/text1b.c reverted to clean INCLUDE_ASM before finishing.
 * ------------------------------------------------------------------- */
