# SELF-VET — func_80062020

Diff: `src/text1b.c` — the `INCLUDE_ASM("asm/funcs", func_80062020);` line at 3853 is
replaced by the pure-C body (annotated). No other file is touched.

Measured this session on the live chassis:
  `sandbox func_80062020 --disable all` -> score 0, build_insns 38, target_insns 38,
  rules_dropped 0, cheat_asm_stripped 167 (all from OTHER functions in text1b.c).
  `verify-oracle` -> ok true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.

CONSTRUCTS: (1) row pointer local `p = (s32 *)((u8 *)&D_800F1198 + ofs);` consumed by
the two tail-column stores `p[2] = 0; p[1] = 0;`; (2) the column-a store written as
`*(s32 *)((u8 *)&D_800F1198 + ofs) = 0;` — the same symbol+byte-offset expression the
copy loop above it uses for column a — rather than as `p[0] = 0;`. (3) `ofs` carries the
terminator row's byte offset after the loop (banked s2 lever, already on the floor-4 body).
No dead code, no unused declaration, no volatile, no cast-away-const, no asm, no pins,
no rules, no `register` qualifier, no dead store, no self-assign. Every statement in the
function performs a store the target performs; every local is read.

## T1 semantic purpose
Each of the three epilogue stores writes a DIFFERENT lvalue (row+0, row+4, row+8) and all
three appear in the target. `p` is read twice. Removing any construct removes a real store
or a real address computation. The only construct whose form (not existence) is a choice is
(2): the column-a store must exist and must write row+0; what is chosen is which of two
equally-valid C spellings of that address is used. Nothing here is semantically empty —
this is not an ADDED construct, it is a spelling selected among natural spellings of a
statement the function genuinely needs. That is the distinction the governing rule draws.

## T2 human-programmer
Yes. The table D_800F1198/119C/11A0 is a 3-column row array; column a packs `x*2 | flag`
with bit 0 as the list-terminator flag (established s6 from the consumer's arithmetic).
The copy loop in this same function writes column a as
`*(s32 *)((u8 *)&D_800F1198 + ofs) = t;` and the table's ONLY consumer (func_800620B8,
six sites) addresses all three columns the same per-column symbol+byte-index way. A 1998
programmer clearing the terminator row — zero the two data columns through a convenient
row pointer, then write the flag column through the project idiom for that column —
produces exactly this text. A reader asks "why two forms?" only if they assume uniformity
was a goal; the comment on the function answers it and forbids the "simplification".

## T3 GCC-internals justification
The MECHANISM is cited (MIPS `legitimize_address` at RTL expansion: `(plus (symbol_ref S)
(reg X))` is a legal address so the symbol never enters a register — LO_SUM; adding a
non-zero constant makes it illegal, so GCC folds K into the symbol and force_regs
`la(S+K)`), and it is cited exactly where the governing rule REQUIRES it: as criterion-1
mechanism-level proof that the target bytes are unreachable from any single spelling
class. It is NOT the semantic justification of the code — the semantic justification is T2.
This is the one family where a named-pass mechanism is mandatory rather than a smell; the
rule's confirmed precedent (InitHiraRmd_80041AC8) is built the same way
(MEM_IN_STRUCT_P / `true_dependence` / sched.c).

## T4 permuter/search provenance
The distance-0 point was originally surfaced by an s2 hand probe and independently
re-found by the s4 permuter campaign; but it is NOT accepted on that basis. It is
accepted on the s5/s6/s7 derivation (the two-shape theorem), which PREDICTS the score of
any candidate shape before compiling it and was predictively validated on two previously
unmeasured tree-node classes. Seven node classes measured; every force_reg shape scores 4,
every symbol-keeping shape scores 5/6/14/15; none mixes. This is a proof, not a search
artifact, and there is no empty construct for a detector to miss.

## T5 family check
Checked against the forbidden catalog line by line. It is NOT: a register pin, `__asm__`,
a scheduling barrier, INLINE_MOVE_ALIASING, volatile coercion of any spelling, an unused
local array, a dead param assign, a dead conditional store, an empty-body `if`, an
`if (1)` wrap, a dead goto/label pad, a DImode chain, a `goto end` accumulator, a
param-local alias declaration-order trick, an opaque `s32 one = 1;`, an `asm("sym")`
rename, a redundant width cast, or a linker-script reorder. The nearest forbidden neighbour
is "same-lvalue dual spelling" — and it does not apply on the facts: row+0 is written
ONCE. `p[0]` is never written; `p` is an alias to the row that the column-a store does not
route through. The s6 SOTN-family claim for that alias+direct shape was FAILed by the
Judge 2026-08-25 and is NOT re-raised here. The family claimed below is the one the OWNER
designated for this function on 2026-08-30.

## T6 naming-announces-intent
Names are `i` (row count), `ofs` (byte offset), `t` (copied word), `p` (row pointer). No
`pad`/`dummy`/`unused`/`spill`/`tmp_pad`/`slack`. Every name denotes a value that is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: proven-spelling-class reconstruction (user policy 2026-06-10), applied to this
    function by owner ruling 6a of the 2026-08-30 escalation batch
  SCOPE: "A same-bytes respelling — a C form that emits byte-identical instructions to the current form and differs only in its effect on GCC's internal analysis — is sanctioned ONLY when ALL of the following hold:"
  PRECEDENT: `.claude/rules/proven-spelling-class-reconstruction.md:14`
  PRECEDENT: `docs/grind/decisions.md:14836`

  Scope note, stated plainly rather than glossed: the rule's scope sentence was written for
  the case where the respelling emits the SAME bytes as the current form and moves only
  GCC-internal analysis. Here the respelling MOVES the bytes (score 4 -> 0) onto the target.
  Ruling 6a resolves that gap explicitly for this function — "Admissible ONLY under the
  existing proven-spelling-class-reconstruction rule (user policy 2026-06-10): a fresh
  Judge/cheat-reviewer must verify the byte-derived derivation meets ALL four criteria
  (mechanism-level proof the target bytes are unreachable from a uniform spelling; plain
  natural C; most-plausible variant, annotated; demonstrated last lever)" — so the four
  criteria, not the framing sentence, are the operative bar, and they are answered here:

  (1) MECHANISM-LEVEL PROOF THE TARGET BYTES ARE UNREACHABLE FROM A UNIFORM SPELLING.
      Target's epilogue addresses ONE row two ways: cols b,c as `8($v0)`/`4($v0)` off a
      force_reg'd row base, col a as `lui $at,%hi(D_800F1198); addu $at,$at,$v1; sw
      $zero,%lo(D_800F1198)($at)` (asm/funcs/func_80062020.s:35-39). MIPS
      `legitimize_address` accepts `(plus (symbol_ref S) (reg X))` as an address — the
      symbol stays out of a register (LO_SUM); it does NOT accept
      `(plus (symbol_ref S) (reg X) (const K))`, folding K into the symbol and force_reg'ing
      `la(S+K)`. So a shared base+disp REQUIRES the row address to exist as a pointer VALUE
      before any constant is applied, and a LO_SUM store REQUIRES it not to. The treatment is
      chosen per expression shape and is uniform across that shape's accesses. Both
      treatments appear on the same row address in the target => the original C wrote that
      address in two expression shapes. Measured, not asserted: 7 tree-node classes
      (pointer local, struct COMPONENT_REF abc/cba, 1-element-array member, 2D array
      abc/cba, `*p = 0` INDIRECT_REF, offset-0 union member) — every force_reg shape scores
      4 with all three stores base+disp; every symbol-keeping shape emits a separate
      `la(sym+4K)` per column with NO shared base (scores 5/6/14/15). The last two classes
      were PREDICTED then measured (s7 of the ledger, i.e. the 2026-08-25 session). Store-order
      permutation is dead independently (s3: c,b,a=4; c,a,b=5; a,b,c=5). RTL evidence: the
      `.rtl` post-expand dump shows `p[0]` already `(set (mem (reg 76)) 0)` at insn 112 — the
      decision is made at EXPAND, so no later pass (CSE, combine, RA, sched) can be steered
      to change it.
  (2) PLAIN, NATURAL C. See T2. No dead code, no unused declarations, no type change to any
      global, no asm, no rules. The exact failure mode the rule calls out — "a type change
      to a global that nothing else justifies" — is avoided: the globals keep their
      splat-given scalar declarations; the spelling difference is local to this function.
  (3) MOST HUMAN-PLAUSIBLE REPRESENTATIVE, ANNOTATED. The proven class is "col a written in
      a symbol-relative shape while cols b,c go through a row pointer". Its members differ
      only in how the symbol-relative address is spelled. The representative chosen is
      character-for-character the expression the copy loop 12 lines above uses for the same
      column, which is also the shape all six consumer sites use — i.e. the variant that
      makes the function internally consistent rather than the one that reads as a special
      case. Rejected alternatives: re-declaring D_800F1198 as an array (forbidden by the
      rule's own "what this does NOT sanction" clause), and `(&D_800F1198)[ofs >> 2] = 0`
      (index arithmetic no reader would write). The function carries a 9-line comment naming
      the rule, the mechanism, and an explicit instruction not to simplify the store to
      `p[0]`.
  (4) EXHAUSTIVELY THE LAST LEVER. Six prior sessions, six distinct modalities, floor flat
      at 4 since s2: structural CSE-defeat KILLED (s3), permuter KILLED (s4 — 2 fresh-seed
      basins, ~46k iters), aggregate/tree-shape KILLED (s5, 5 shapes), solver measured
      INAPPLICABLE (s5 — residual is PRE-RA, 35 insns vs 38, so no RA/sched seat exists to
      solve), forensics/object-model recovery KILLED (s6 — consumer refutes any flag/data
      split; whole-function 2D model scores 24). Two escalations filed (2026-07-24,
      2026-08-25) and adjudicated. The cheat-reviewer/Judge has ruled on the candidate forms
      (2026-08-25 FAIL of the SOTN-alias FAMILY claim, which expressly preserved the
      derivation), and the policy question was surfaced to the owner and answered on
      2026-08-30 by ruling 6a. That is the rule's criterion 4 satisfied literally, including
      its "the policy question was surfaced to the user" prong.

ANNOTATION-CONFORMANCE: the governing rule's criterion 3 mandates "annotate it with a
comment citing this rule", not the `/* FAKE: ... */` template (this family is not a FAKE
family — no construct here is semantically empty). The emitted annotation, verbatim, is the
9-line block comment immediately above `void func_80062020(s32 *arg0)` in src/text1b.c:

  /* Terminator row is cleared through two address expressions: a row pointer for
   * the two tail columns, and the same symbol+byte-offset form the copy loop (and
   * the table's only consumer, func_800620B8) uses for column a. Reconstructed
   * spelling class, NOT a codegen tweak: see .claude/rules/proven-spelling-class-
   * reconstruction.md. MIPS legitimize_address accepts (symbol_ref + reg) as an
   * address but folds (symbol_ref + reg + const) into la(sym+K) and force_regs it,
   * so a single expression shape can emit EITHER a shared base+disp row for all
   * three columns OR a per-column symbol-relative address for all three -- never
   * target's mix of both on one row. Do not "simplify" the last store to p[0]. */

It carries what (the two-expression terminator write), the named mechanism (MIPS
legitimize_address at RTL expansion), the rule citation, and the anti-simplification
instruction the rule asks for.
