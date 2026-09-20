# SELF-VET — func_8006ECF4 (session 3, progress outcome — floor 60 -> 11, not yet candidate-ready)

CONSTRUCTS: struct-init statement reorder (H4); goto-based shared-label
control-flow transcription for the s.p1 dispatch region (H5); pad-byte store
statement reorder (H6); if/else split of a call argument into two literal
call sites (H7). None of these are no-semantic-purpose constructs — every
statement in the diff computes or stores a real, consumed value, and every
branch/goto corresponds 1:1 to a real conditional edge present in the target
disassembly.

## T1 semantic purpose
Every changed construct has an observable effect matching the target's own
control flow / value assignments: the goto labels (H5) route to blocks that
compute real values (`s.p1`) actually read later by `func_80073728`; the two
call sites (H7) each perform the real call with the real literal mode
argument the corresponding branch used in the original. Reordering assignment
statements (H4/H6) changes nothing semantically (same final struct state) but
matches the ACTUAL store order the original compiler produced from its own
source — this is ordinary statement-order C, not a coercion.

## T2 human-programmer test
A programmer transcribing this exact disassembly (which a decomp is,
definitionally) would write the goto-to-shared-label pattern for H5/H7
because that IS the control-flow shape shown by `jr`-free explicit branches
in the asm (bnez/beqz targeting shared labels .L8006EFA4/.L8006EF98,
beqz/j choosing between two `jal` call sites) — not something a reader would
ask "why is this here?" about.

## T3 GCC-internals justification
No construct's justification rests on a compiler-internals mechanism as the
semantic reason for the statement's existence. The goto/if-else shapes ARE
the real control flow (asm evidence, cited by line in evidence.md); GCC's
cross-jump/tail-merge behavior is mentioned only to EXPLAIN why the
duplicated-statement spelling (s2's version) cost extra instructions, not as
the reason the new spelling is "needed" absent real semantics.

## T4 permuter/search provenance
No permuter or auto-search tool was used this session. Every lever was
derived by directly reading asm/funcs/func_8006ECF4.s and comparing against
the diff tool's hunk classifications, then hand-transcribing the real control
flow / store order into C.

## T5 family check
H5 and H7 match the SOTN-sanctioned "mixed exit forms" (goto endK + inline
return/distinct labels) and "unconditional-common-store duplication into both
branch arms" (F7 survey) families in no-new-park-categories.md — but note
neither construct NEEDS the family sanction to be legitimate: they are the
literal, truthful translation of the disassembly's real branch structure, not
a no-semantic-purpose wrapper. H4/H6 are plain statement-order changes with
no family at all (ordinary C, no no-semantic-purpose content).

## T6 naming-announces-intent
No new identifiers introduced this session (b2, i, s3, s0, sel, a2, rectbuf,
p1_idx/p1_fallback/p1_done labels — all pre-existing from s2 or named for
their real role in the control flow, e.g. `p1_idx` names the block that
computes the index-based p1 value, `p1_fallback` the block that computes the
fallback p1 value — no `pad`/`dummy`/`unused`/`spill`/`tail`/`slack` naming
anywhere).

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: mixed exit forms (goto-based shared-label control flow)
  SCOPE: "deliberately mix `goto endK` with inline `return` to defeat `find_cross_jump`. SOTN ships this verbatim in `SsVabOpenHeadWithMode` (`src/main/psxsdk/libsnd/vs_vh.c`)."
  PRECEDENT: .claude/rules/no-new-park-categories.md (SOTN-accepted techniques section, "Mixed exit forms" bullet)

  FAMILY: unconditional-common-store duplication into both branch arms
  SCOPE: "duplicating common-tail stores into both if/else arms where cross-jump may or may not re-merge them — sanctioned at the CONSTRUCT level regardless of which GCC pass the duplication feeds ... SOTN ships 25 fully-identical-arm if/else constructs and 1,275 identical-store-in-both-arms sites in matched code."
  PRECEDENT: .claude/rules/no-new-park-categories.md (2026-08-18 additions, F7 survey entry)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Every statement in this
session's diff has a truthful, directly-consumed semantic reading (real
values, real control-flow edges matching the disassembly); nothing in this
diff is dead code, a padding device, or a no-semantic-purpose coercion, so no
`/* FAKE */` annotation applies.

NOTE: this is a `progress` outcome (floor 11, not 0) — this self-vet is
written proactively per the ledger's own discipline (banking a clean
construct trail for whichever future session closes the remaining 3
operand-only + 5 source-level hunks), not because this session is claiming
`candidate-ready`.
