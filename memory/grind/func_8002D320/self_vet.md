# SELF-VET — func_8002D320

CONSTRUCTS: GTE vector island (single __asm__ volatile: addu-$t4 preamble + lwc2 x2 + 2 nops + mvmva .word 0x4A486012), GTE store island (single __asm__ volatile: addu-$t4 preamble + swc2 $25/$26/$27), GTE LZCS island (single __asm__ volatile: addu-$t4 + mtc2 $t4,$30 + 2 nops + addu $t4,$sp + swc2 $31, "=m"(sp_var)), FAKE staged-z tail (`if (y + x < min_y) { z = 0; return z; }`), sequential reuse of local `x` (rotated X -> squared distance -> discriminant -> sqrt)

## T1 semantic purpose
- The three cop2 islands: real computation with NO C analog (GTE mvmva vector transform, LZCS/LZCR leading-zero count feeding the sqrt table walk). The bytes they emit ARE the function's behavior; splat tags three of the target's cop2/preamble instructions "handwritten instruction".
- staged-z: the staged value 0 is REAL and CONSUMED — it is the function's return value, read by the `return z;` immediately following. Not byte-invisible: without it the emitted tail is a different (folded) instruction sequence; with it the build emits target's diamond. This is a FAKE-family construct and is claimed as such below, with annotation.
- x reuse chain: every write to `x` is a real value read downstream (range compare, r_sq compare, table index, final tail arithmetic). No dead code anywhere in the body.

## T2 human-programmer
- Islands: yes — a 1998 PSX programmer hand-wrote exactly these (the target's own bytes show the redundant $t4 copy + unfilled delay nops GCC never emits).
- staged-z: a reader would ask why `z = 0; return z;` instead of `return 0;` — that is the FAKE smell, which is why it carries the mandatory annotation and a sanctioned-family claim rather than being passed off as natural C.
- Everything else (early returns, min/max Y envelope, one-variable computation chain): natural hand-written C; the chain reuse mirrors how the original overwrote one working register ($a0).

## T3 GCC-internals justification
- Islands: justified by program semantics (cop2 ops), not by any GCC pass.
- staged-z: yes — the justification is jump.c's store-flag if-conversion single-set precondition. That is precisely why it is claimed under a sanctioned FAKE family with annotation, not smuggled as natural C.
- No other construct's presence is GCC-motivated.

## T4 permuter/search provenance
No permuter or auto-search ran this session. Every form was hand-derived from the target asm, the brief, and the twin func_8002EA24's banked ledger, and each was measured individually in the cheat-invisible sandbox.

## T5 family check
- Islands: canonical inline asm (GTE/cop2) — the ALLOWED category of inline-asm-policy, in the exact narrow spelling the 2026-07-30 Judge ruling on the twin demanded (address computed in C, bound as "r" operand; template = $t4 addressing preamble + cop2 ops only; no addiu/general-purpose ops in template; "$12" clobber; no pins, no barriers, no hardcoded operand-register templates). Cluster membership row exists for this function.
- staged-z: matches the staged-value-reused-variable sanctioned family (borrowed EXISTING local, value real and immediately read, borrow provably safe). It is NOT the dead-store family (nothing is dead), NOT a named-intermediate (z is a borrow, not fresh), NOT an arms-duplication.
- No forbidden family is touched: zero register pins, zero scheduling barriers, zero hardcoded-$N operand templates, zero volatile coercion, zero dead locals/arrays/stores, zero alias renames, no do-while(0), no build-time rewriting.

## T6 naming-announces-intent
No pad/dummy/unused/spill/slack names. `sp_var` is the LZCR output slot the island genuinely writes and the C genuinely reads (`lzcr = sp_var;` -> shift computation) — same name and role as in the two authorized siblings func_800274BC / func_8002D518.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: staged-value-reused-variable
  SCOPE: "SANCTIONED 2026-07-03 — a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:46

  FAMILY: canonical cop2 inline asm — addu-$t4 addressing-preamble cluster (2026-08-17 owner cluster ruling)
  SCOPE: "the 26 queued functions sharing the `addu $t4,$aN,$zero` + cop2 idiom (28 total in the 0x8001-0x8003 band) inherit this disposition subject to the same mechanical per-function check ... which the Judge may apply without re-escalation."
  PRECEDENT: .claude/rules/cop2-addressing-preamble-cluster.md:73

ANNOTATION-CONFORMANCE: the staged-z construct carries, in src/code6cac_b.c directly above the statement:
  /* FAKE: stages the returned 0 through z -- a real value, read by the
   * return on the next statement -- whose own value (the rotated Z) was
   * last read at the `z * z` above and is dead here.  Mechanism: jump.c's
   * store-flag if-conversion requires a SINGLE-SET arm; the two-statement
   * arm keeps target's unfolded 0/1 diamond (bnez; move v0,zero delay;
   * addiu v0,1) instead of folding it to `slt` + `xori v0,v0,1`.
   * Family: [[staged-value-reused-variable]].  Lever-exhaustion:
   * memory/grind/func_8002D320/hypotheses.md session 1 (four pure-C tail
   * shapes measured: plain early-return, result-carrier nest, goto-reject,
   * inverted sense) + the twin func_8002EA24's session-2 six-shape tail
   * census on the identical diamond. */
carrying what (the staged 0) + mechanism (jump.c store-flag single-set precondition) + lever-exhaustion (hypotheses.md s1 + twin census). The canonical islands are not FAKE-class and carry no FAKE annotation (none owed); liveness argument for the borrow: z's last real read is `z * z` in the distance sum; z is not read between that point and the staging store, and never after the return.
