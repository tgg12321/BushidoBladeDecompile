# SELF-VET — func_800480C0
CONSTRUCTS: volatile u32 pre_pad[8] (unwritten leading frame pad), arg0 = 0 (dead store to a PARAM)
## T1 semantic purpose: pre_pad — none beyond frame reservation (that is the sanctioned family's whole point; it reproduces target's allocated-but-untouched 32-byte vars region). arg0 = 0 — none (dead store to a param; family-sanctioned).
## T2 human-programmer: neither would be written from the spec; both are FAKE-annotated for exactly that reason and sit inside frozen families.
## T3 GCC-internals justification: yes — pre_pad: function.c assign_stack_local frame_offset reservation; arg0=0: cse2 canonical-register substitution. Both are the named mechanisms the families REQUIRE in the annotation; the constructs are submitted only under those families, not as ordinary C.
## T4 permuter/search provenance: none — hand-derived by transplanting the accepted in-file twin func_80047FBC (identical shape); no permuter run this session.
## T5 family check: pre_pad -> phantom-frame-slot volatile pad local (frozen list, owner ruling 2026-08-18); arg0=0 -> dead-store-fake-exception (LOCALS/PARAMS). Neither matches a forbidden-family row: the pad is volatile (not the `(void)&local` / non-volatile unused-array coercion), the store is to a PARAM (not a global), no pins/asm/barriers.
## T6 naming-announces-intent: `pre_pad` announces pad intent — this is the family's mandated spelling (the engine allowlist keys on the exact name `pre_pad`, engine/volatile_cheats.py:746-769, and every sanctioned sibling uses it). `arg0` is the param's own name.
SANCTIONED-FAMILY-CLAIMS:
  FAMILY: phantom-frame-slot volatile pad local (unwritten leading local pad)
  SCOPE: "an **unwritten leading local pad** — spelled `volatile`, FAKE-annotated, with documented lever exhaustion and layer-2 review — is sanctioned for frames whose residual is provably a single allocated-but-untouched leading region confirmed by frame-term forensics."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:334`
  PRECEDENT: `engine/volatile_cheats.py:758`
  PRECEDENT: `docs/reference/sotn-construct-index.md:103`
  FAMILY: dead-store-fake-exception (dead store to a PARAM)
  SCOPE: see `.claude/rules/dead-store-fake-exception.md` — LOCALS/PARAMS only, FAKE required; same lever Judge-PASSed on in-file sibling func_80047FBC.
  PRECEDENT: `docs/grind/decisions.md:981`
  PRECEDENT: `272e47c4`
ANNOTATION-CONFORMANCE: both constructs carry full /* FAKE: <what>, mechanism: <pass>, lever-exhaustion: <where> */ blocks in memory/grind/func_800480C0/candidate.c (pre_pad: assign_stack_local, exhaustion = sibling ledgers decisions.md:7401-7660 + s1 ladder; arg0=0: cse2 canonical-register substitution, exhaustion = decisions.md:981 + s1 ladder score 32 without it).
NOTE: this session returns owner-gated (integration handoff), not candidate-ready — the sandbox cannot print 0 without the engine allowlist row. This vet is provided for the operator's layer-2 review.
