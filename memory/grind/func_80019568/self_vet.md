# SELF-VET — func_80019568

Diff surface: `src/code6cac.c` lines 365-499 replaced with the s3 body
(`memory/grind/func_80019568/candidate.c`). Measured `sandbox func_80019568
--disable all` = **score 0**, `build_insns 141 == target_insns 141`, this session,
with the edits in place in `src/`. No `__asm__`, no `register T x asm("$N")` pin, no
`volatile`, no regfix/asmfix/rule/engine/tool/Makefile/*.ld file touched.

CONSTRUCTS: (C1) per-iteration record pointers `u8 *rec = &pk[i * 8]; s16 *o = &sp.output[i];` at the top of the loop-1 body; (C2) `s32 enable;` flag local set to 1 in the valid arm and 0 in the invalid arm and stored to `o[2]` inside each arm; (C3) `s32 *p = &D_80102790;` used for a read-modify-write of that global in the tail.

## T1 semantic purpose:
- C1: YES, real. `rec` and `o` are the address of this iteration's 8-byte packet record and
  of this iteration's output slot. Every subsequent access in the loop body reads through
  them; removing them means re-spelling `pk[i*8+k]` / `sp.output[i]` at each of the six use
  sites. This is a value a reader needs and the program consumes.
- C2: The stored VALUE (1 / 0) is real and lands in target's bytes (`addiu $v0,$zero,1;
  sh $v0,4($a2)` and `sh $zero,4($a2)`). The VARIABLE holding it is not observable:
  `o[2] = 1;` / `o[2] = 0;` is behaviourally identical. So the local itself has no
  observable effect -> it is a codegen construct and is FAKE-annotated as such.
- C3: The pointer's VALUE is real and materialised in target's bytes (`lui $v0,%hi(D_80102790);
  addiu $v0,$v0,%lo(D_80102790)`), and it is dereferenced for both a load and a store. But
  writing `D_80102790` directly is behaviourally identical, so the handle is a codegen
  construct and is FAKE-annotated as such.

## T2 human-programmer:
- C1: YES, unreservedly. Hoisting `rec`/`o` out of six subscript expressions is what any
  programmer writes for a per-record loop; a reader would not ask "why is this here?".
- C2: A flag local named `enable`, assigned per branch and stored, is ordinary idiomatic C —
  but a reader could reasonably ask why it is not just `o[2] = 1;`. It is therefore treated
  as a construct, not as ordinary C. There is, however, POSITIVE evidence that the original
  source had exactly this variable: in the same loop the literal `4` IS written bare in two
  arms, and target hoists it into `$t2` in the prologue (loop.c combine_movables), while the
  `1` is NOT hoisted. The only difference GCC 2.7.2 can see between those two constants is
  `n_times_set` on the pseudo, i.e. whether a declared variable carried the value. So the
  bytes themselves say the original held this value in a variable and the `4` in a literal.
- C3: Read-modify-write through a pointer local is a 1990s idiom that real programmers use
  even for plain globals; this is the exact shape the owner sanctioned on 2026-06-10.

## T3 GCC-internals justification:
Named honestly, because they ARE the mechanism for C2/C3, and both sit inside sanctioned
families where a named pass is a REQUIRED prerequisite (not a substitute for one):
- C1's justification is program logic first (see T2); its loop.c effect (a single "add 0" giv
  leader per scale group tripping the worthwhile test at loop.c:3824, so biv `i` survives the
  loop.c:4034 elimination gate) is a consequence, and C1 carries no family claim and no FAKE.
- C2: `loop.c:702-716` — scan_loop only creates a movable when `invariant_p(src)` and
  (`n_times_set == 1` or `consec_sets_invariant_p`). Two non-consecutive sets in the two arms
  give `n_times_set == 2` with non-adjacent sets, so no movable is created and the `li 1`
  stays in the loop, filling target's lhu load-delay slot.
- C3: address materialisation at expand time — a MEM whose address is a bare `symbol_ref` is
  a legitimate MIPS address, so GCC never cse's it into a register; a pointer VALUE forces the
  `la` (lui+addiu) pair that target uses for both the `lw 0($v0)` and the `sw 0($v0)`.

## T4 permuter/search provenance:
NONE of the three came from a permuter or any automated search. No permuter campaign was run
this session (no permuter artifacts exist for it). C1 was derived in s2 by reading
`tools/gcc-2.7.2/loop.c` and confirmed against `tmp/grind/func_80019568/dumps/code6cac.loop`.
C2 was derived this session from the same loop.c gate plus the `4`-is-hoisted / `1`-is-not
asymmetry in the target asm. C3 was derived from the target asm's `la` + `0($v0)` pair and
from the s2 kills of the aggregate (H8) and statement-order (H9) explanations.

## T5 family check:
- C1: no family. Ordinary C; nothing in the forbidden catalog covers "hoist a subscript base
  into a local that every use reads through".
- C2: constant-holder / named scalar local — the frozen SOTN family "constant-holder / dead
  scalar locals", rule `.claude/rules/named-local-fake-exception.md`. NOT the named-intermediate
  family (that requires once-written, and `enable` is written once per arm, i.e. twice in the
  function). NOT the variable-reuse family (`enable` is a fresh local with ONE meaning, not an
  existing local borrowed for a second unrelated value; the variable-reuse spelling of this
  same slot was measured separately and is strictly worse — 8 vs 0). Declared and annotated
  under named-local-fake-exception; see the honest caveat at the end of this vet.
- C3: pointer alias to a global, in its read-modify-write shape — frozen family "C-level
  pointer alias to a global"; the exact zero-displacement RMW shape is separately
  user-sanctioned in `.claude/rules/pointer-rmw-global-sanctioned.md`. It is NOT an
  `asm("Sym")` alias rename (forbidden), NOT a volatile-cast alias, NOT a single-use alias,
  NOT a chain of pointer re-spellings: it has exactly one load use and one store use with real
  program logic between them.

## T6 naming-announces-intent:
No name in the diff is `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`,
`slack` or `_frame_pad`. Names are `rec` (the packet record), `o` (this slot's output),
`enable` (the slot-enabled flag), `p` (the RMW pointer — the neutral name the RMW rule names
explicitly). Every one of them is read: `rec` 4 times, `o` 5 times, `enable` twice, `p` twice.
There are no declaration-only, address-of-only or `(void)`-discarded locals. The s2 candidate's
dead `s16 *output;` declaration was REMOVED this session and the floor stayed 0, so no dead
local is load-bearing here.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: constant-holder / named scalar local (C2, `s32 enable`)
  SCOPE: "a local variable whose only purpose is codegen influence — holding a constant in a register across calls/statements, or existing as a declaration that biases register allocation — is a sanctioned last-resort matching lever under the prerequisites below."
  PRECEDENT: docs/reference/sotn-construct-index.md:945

  FAMILY: C-level pointer alias to a global — read-modify-write shape (C3, `s32 *p`)
  SCOPE: "a pointer local to a global used for an actual READ-MODIFY-WRITE sequence (load through it, compute, store through it) — the pointer has at least one load AND one store use."
  PRECEDENT: .claude/rules/pointer-rmw-global-sanctioned.md:36

  FAMILY: C-level pointer alias to a global — general (C3, second citation)
  SCOPE: "a local pointer that provides a second C handle to a global — where using the global directly would be semantically identical — is a sanctioned last-resort matching lever under the prerequisites below."
  PRECEDENT: docs/reference/sotn-construct-index.md:681

ANNOTATION-CONFORMANCE:
  /* FAKE: constant-holder flag local for output[i+2]; two non-consecutive sets
   * (1 in the valid arm, 0 in the invalid arm) stop scan_loop from creating a
   * movable for the `li 1`.  mechanism: loop.c:702-716 (movable requires
   * n_times_set==1 or consec_sets_invariant_p); the literal 4, which IS written
   * as a bare literal in two arms, is hoisted into $t2 in target, so target's
   * un-hoisted 1 proves the original held it in a variable.
   * lever-exhaustion: memory/grind/func_80019568/hypotheses.md H6/H10/H12 -
   * bare `o[2] = 1;` measures 8 (build 142, li hoisted); the variable-reuse
   * spelling measures 8 (build 141); this form measures 0. */
  /* FAKE: read-modify-write handle for D_80102790 - one `la` address kept in a
   * register for both the load and the store instead of two independent
   * %hi/%lo symbol MEMs.  mechanism: address materialization / CSE shape at
   * expand time (a MEM whose address is a symbol_ref never gets its address
   * cse'd into a register on MIPS).  lever-exhaustion:
   * memory/grind/func_80019568/hypotheses.md H8 (aggregate KILLED), H9
   * (statement reordering INERT), H11 (direct multi-read of the global
   * measures 11). */
  Both carry what + a named GCC-pass mechanism + a lever-exhaustion pointer, per the
  template. C1 carries no annotation because it carries no family claim (ordinary C).

## Honest caveats for the reviewer (stated, not hidden)
1. FAKE prerequisite (a) is "the full modality ladder demonstrably spent". This function has
   run three sessions (recon, structural, structural); the ladder is not literally exhausted.
   What IS documented, per-residual and measured, is in `hypotheses.md`: for C2 —
   H4 (s16 counter, 74), H6 (variable-reuse spelling, 8), H10 (bare literal, 8/142); for
   C3 — H8 (aggregate KILLED by sibling func_800194F4's plain %lo stores), H9 (statement
   reordering INERT), H11 (direct multi-read of the global, 11). Both constructs are the
   unique measured closers of their residual, not a first reach.
2. C2 is a two-valued flag rather than the rule's canonical `s32 k = 1;` held across a call.
   It is a scalar local carrying a constant to a store, which is the family's shape and
   inside its "scalars only" bound (no array, no frame coercion), but it is not verbatim the
   rule's worked example. If the reviewer holds that the constant-holder family does not
   reach a two-valued per-arm flag, the correct disposition is a ruling on C2 alone — C1 and
   C3 are unaffected and the byte-proof stands.
