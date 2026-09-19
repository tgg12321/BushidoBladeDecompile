# SELF-VET — func_8002DAD0

CONSTRUCTS: (1) FAKE dist-reuse (the scaled Z delta staged through the function's existing, currently-dead `dist` local before the `dist_sq` computation); (2) nine `__asm__ volatile` GTE/cop2 islands, each the verbatim expansion of a named Sony PsyQ DMPSX `inline_o.h` macro (gte_ldopv1, gte_ldopv2 + gte_op0, gte_stlvnl x3, gte_Lzc, gte_SetRotMatrix, gte_ldlv0 x2 + gte_rtv0-class MVMVA).

The C body is byte-identical to the session-6/7/8 body (same driver body hash); session 6 (this one) changed only the island COMMENTS, to cite the header the islands actually come from. Measured this session: `sandbox func_8002DAD0 --disable all` -> score 0, 204/204 insns.

## T1 semantic purpose
FAKE dist-reuse: no observable effect beyond what a fresh temp would produce — it exists to steer register allocation, which is why it is FAKE-annotated. The nine asm islands: full semantic purpose — they ARE the GTE computation (cross product, leading-zero count, rotation-matrix load, vector rotate). cop2 ops have no C form, and the non-cop2 instructions inside them (the `move $12,%0` address staging, the `lhu/sll/or` V0 pack, the delay `nop`s) are not program logic: they are the Sony macro's own body, measured load-bearing — respelling one island without its preamble moves the score 0 -> 4 (rejected/s6-stlvnl-pct0-offset-score4.c).
## T2 human-programmer
The islands: yes — a PsyQ programmer writes `gte_ldopv1(p); gte_op0(); gte_stlvnl(q);` and the compiler emits exactly these instructions; our body is that expansion written out, because the macro set is not in this project's headers. The FAKE dist-reuse: no — a programmer would use a fresh local; it is disclosed as a match device.
## T3 GCC-internals justification
FAKE dist-reuse: yes, explicitly — global.c set_preference/expand_preferences is the named mechanism (cited in the annotation with file:line). The islands: no — their justification is provenance (a named Sony macro body at a header line), never a GCC pass.
## T4 permuter/search provenance
Neither construct came from an unvetted auto-search hit. The dist-reuse form was hand-derived and cross-checked by the s5 exhaustive `tools/spelling_enum.py` sweep; the islands are transcriptions of Sony's own macro bodies, verified line by line against `inline_o.h` this session (table in memory/grind/func_8002DAD0/psyq_inline_o_provenance.md).
## T5 family check
FAKE dist-reuse -> staged-value-reused-variable (a real, immediately-used value staged through an existing currently-dead local), FAKE-annotated with lever-exhaustion. Islands -> canonical GTE/cop2 inline asm under the two-category inline-asm policy, as a named member of the 2026-08-17 owner cop2-addressing-preamble cluster ruling. Nothing here is a register pin, a hardcoded-$N injection into compiler output, a scheduling barrier, or a volatile coercion.
## T6 naming-announces-intent
No `pad`/`dummy`/`unused`/`spill`/`sp_*`/`_buf`/`tail`/`slack` naming in the diff. `dist` is a real, meaningfully-named local carrying the function's distance value; `sp_tmp` is the LZCR landing slot the Sony `gte_stlzc` macro stores through, and it is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: staged-value-reused-variable (the FAKE dist-reuse)
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:3

  FAMILY: canonical GTE/cop2 inline asm (cop2-addressing-preamble cluster member)
  SCOPE: "Two-category inline-asm policy: CANONICAL (GTE/cop2/BIOS/HW) is authentic and fine; CHEAT (register pins, INLINE_MOVE_ALIASING, scheduling barriers) is forbidden — a function carrying any cheat-asm is INCOMPLETE."
  PRECEDENT: .claude/rules/inline-asm-policy.md:4 ; cluster membership row .claude/rules/cop2-addressing-preamble-cluster.md:76 ; island-to-macro table memory/grind/func_8002DAD0/psyq_inline_o_provenance.md:30

ANNOTATION-CONFORMANCE:
  /* FAKE: the scaled Z delta is staged through the function's existing `dist` local (whose distance value is only assigned by the if/else below, so `dist` is dead at this point) instead of a fresh block-local temp, mechanism: GCC 2.7.2 global.c expand_preferences (tools/gcc-2.7.2/global.c:828) ... lever-exhaustion: memory/grind/func_8002DAD0/hypotheses.md s3 + s5 — 11 banked instance kills plus the exhaustive tools/spelling_enum.py sweep of both flat blocks touching dist_sq/dist, all measured 6. */
  The annotation in candidate.c carries all three required parts: WHAT (staging the Z delta through `dist`), MECHANISM (global.c expand_preferences / set_preference, with file:line), and LEVER-EXHAUSTION (hypotheses.md s3 + s5, 11 instance kills + the enumerate sweep). The nine islands are not FAKE constructs and carry no FAKE annotation; each carries its Sony macro name and `inline_o.h` line range instead, per cop2-addressing-preamble-cluster condition 3.
