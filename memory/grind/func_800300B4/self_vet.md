# SELF-VET — func_800300B4  (s11, escalation modality / re-adjudication, 2026-09-02)

CONSTRUCTS: (1) four PsyQ SDK GTE macro-body `__asm__` islands — gte_SetRotMatrix,
gte_ldlv0, gte_rtv0 (`.word 0x4A486012`), gte_stlvnl — in the older-SDK
`move $12, %0` materialize-then-copy spelling; (2) one `do { ... } while (0);`
wrap around the gte_stlvnl island, FAKE-annotated.

## T1 semantic purpose
(1) Islands: yes. The GTE (cop2) coprocessor transfers `ctc2`/`mtc2`/`lwc2`/`swc2`
and the MVMVA command word have no C analogue; without them the function does not
rotate the vector at all. Each island is the published body of a named Sony PsyQ
libgte inline macro — the original source called `gte_SetRotMatrix(r)`,
`gte_ldlv0(r)`, `gte_rtv0()`, `gte_stlvnl(r)`, and the island text IS that macro's
expansion. Removing them removes the function's behaviour.
(2) do-while(0) wrap: no semantic effect — it is a codegen device and is declared as
such under the sanctioned family, with the mandatory FAKE annotation. Ablation
measured this session: wrap present = sandbox 0, wrap removed = sandbox 13 at the
same instruction count (83/83), i.e. it is a pure register-seat effect
(`tmp/grind/func_800300B4/s11/sandbox_nowrap.txt`).

## T2 human-programmer
(1) Yes, verbatim: a PS1 programmer writes `gte_ldlv0(&obj->lv); gte_rtv0();` etc.
The islands are exactly the SDK macros a human would call; the preferred future form
recorded by the owner ruling is a BB2-local GTE macro header so the source reads
`gte_ldlv0(vec)` with the same bytes. No GPR instruction outside the named macro
bodies is inside any island.
(2) No — a human would not write the do-while(0). It is declared FAKE, not passed off
as program logic.

## T3 GCC-internals justification
(1) Islands: NO GCC-internals reasoning. They are admitted because they are Sony SDK
macro text for operations with no C form, under the owner cluster grant — the
justification is provenance (PsyQ 4.5 `inline_c.h`), not the allocator.
(2) The do-while(0) wrap IS mechanism-justified (flow.c loop-note reference weighting
raising the `&mac` refs so local-alloc seats it in `$s2` ahead of `arg0`). That is
permitted and required for this construct: the sanctioned family mandates a named
GCC-pass mechanism inside the FAKE annotation, and the annotation carries it.

## T4 permuter/search provenance
Neither construct came from a permuter win. The islands are transcribed from the PsyQ
4.5 `inline_c.h:101-110` macro body and are character-identical to the owner-granted
func_800203B4 islands (`src/code6cac.c:1860-1872`, `inline_asm_canonical.txt:367`).
The s4 permuter campaign (24.6k ordinary-C-only iterations) never produced either
construct; it is banked as `rejected/permuter-ordinary-c-best-24k-iters-s4-338w.c`.

## T5 family check
(1) Islands → cop2-addressing-preamble cluster, condition 3 as CLARIFIED by owner
Ruling A 2026-09-02. func_800300B4 is an ENUMERATED registry carrier
(`tools/grinder/owner_cluster_grants.txt:23`). Not a first reach: the identical
gte_ldlv0 pack was merged today under the same door for func_8002FF20
(decisions.md 2026-09-02 10:58 PASS). The four superseded bans in
`state.json.superseded_bans` are exactly this construct; `banned_constructs` is empty.
(2) do-while(0) → its own frozen-list family, FAKE-annotated. No forbidden family is
matched: zero register pins, zero hardcoded-`$N` injection of C-expressible work
outside a macro body, zero scheduling barriers, zero volatile coercion, zero dead
locals/pads/self-assigns, zero alias renames.

## T6 naming-announces-intent
No `pad`/`dummy`/`unused`/`spill`/`slack`/`_buf` names. Locals are `mac`, `dir`, `mtx`,
`playerData`, `mat`, `lookup` — every one is read and consumed by the function's own
logic.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: cop2 addressing-preamble cluster, condition 3 (template = the named Sony PsyQ
    GTE macro body) — owner Ruling A 2026-09-02
  SCOPE: "condition 3's \"template\" is the body of the named Sony PsyQ GTE macro the island reproduces. GPR instructions that are the macro's own published text — e.g. `gte_ldlv0`'s `lhu/lhu/sll/or` VX0/VY0 pack (PsyQ 4.5 `inline_c.h:101-110`) — are part of the template and ADMITTED. Nothing outside the named macro body may be in the island; every island comment must cite the macro name and header line."
  PRECEDENT: .claude/rules/cop2-addressing-preamble-cluster.md:163
  PRECEDENT: tools/grinder/owner_cluster_grants.txt:23
  PRECEDENT: inline_asm_canonical.txt:367

  FAMILY: do { ... } while (0); match device (owner ruling 2026-07-06)
  SCOPE: "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient justification. Hard line unchanged: no regfix/pins/inline-asm/semantic-lie C."
  PRECEDENT: .claude/rules/do-while-zero-exception.md:6

ANNOTATION-CONFORMANCE:
  /* FAKE: do-while(0) wrap around gte_stlvnl, mechanism: flow.c loop-note ref weighting (loop_depth doubles the &mac def+asm refs so local-alloc seats it in $s2 ahead of arg0), lever-exhaustion: memory/grind/func_800300B4/hypotheses.md H14/H24 + the s5 class kill (no FAKE-free C form reaches the four call-crossing seats, local-alloc.c:1666) */
  what = the do-while(0) wrap around the gte_stlvnl island; mechanism = flow.c
  loop-note reference weighting feeding local-alloc's seat order; lever-exhaustion =
  hypotheses.md (H14/H24 single- and nested-wrap geometry sweep, the s5 class kill that
  no FAKE-free C form reaches the four call-crossing seats, and the s9 pressure/order
  closure). Single-level wrap — no nesting, so the nested-wrap justification prong does
  not apply. Ablation measured this session: 0 with, 13 without.

HONEST BUCKET: COMPLETED-INLINE-ASM-CANONICAL (allowlist line required), never
COMPLETED-C — mandated by owner Ruling A 2026-09-02 for every cluster carrier. The
driver must route a PASS through the registry grant door (Ruling C 2026-09-02).

MEASUREMENTS THIS SESSION (HEAD 171b3142 + this diff):
  sandbox func_800300B4 --disable all == 0 (83/83, rules_dropped 0)  [s11/sandbox_s11.txt]
  verify-oracle --rebuild --allow-dirty: build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle  [s11/verify_oracle_s11.txt]
  scan_hand_coded --single: tier=LOW 1/8 (S4 only) — recorded for completeness; the
  admission door here is the owner cluster REGISTRY row, not the scanner
  [s11/scan_hand_coded.txt]
