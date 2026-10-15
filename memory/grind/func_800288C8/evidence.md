# Evidence bank — func_800288C8

## s1 (recon) — chassis is already closed; this is a review-pending function, not a gap

`canonical func_800288C8` -> ASM-PARTIAL, 465 insns, regions [130,130] [133,133] (2 cop2 insns),
reason: mixed C/asm grant in inline_asm_canonical.txt. `sandbox func_800288C8 --disable all`
(this session) -> `{"score": 0, "target_insns": 465, "build_insns": 465}`. `verify-oracle` (this
session, full tree) -> `{"ok": true}` i.e. full-build SHA1 == oracle with this body in place.
So all three of inline-asm-policy's 2026-09-25 "owner_cluster_grants.txt row" gate conditions
(sandbox 0; full-build SHA1 == oracle; a fresh layer-2 PASS) have their first two already met;
only the fresh layer-2 PASS is outstanding. The `inline_asm_canonical.txt` row for this function
(line ~406) is ALREADY WRITTEN — it documents the body as inline_o.h-class gte_Lzc, cites owner
ruling 2026-09-26, and states "canonical gate route ASM-PARTIAL ... manual lane, layer-2
cheat-reviewer gate." This supersedes the 2026-09-25 decisions.md entry ("no row granted now")
which predates the 2026-09-26 ruling that created the row.

Read `asm/funcs/func_800288C8.s`: it is the PRE-struct splat disassembly (flat `D_80101F32`
style symbols for every `Unk80101EC8Record` field access) and does NOT match the current C
body's symbol names at all (current C uses `D_80101EC8[idx].field` via the already-declared
`Unk80101EC8Record` struct). Confirmed not a bug: `D_80101F32 == D_80101EC8 + 0x6A` exactly
(0x101EC8 + 0x6A = 0x101F32), i.e. the flat symbol is just the un-aggregated form of
`D_80101EC8[0].unk_6A`, which the current C already reads correctly (sandbox 0 proves the
struct access reproduces the same bytes). The `.s` file is leftover from before the struct was
declared and is not used by the sandbox scorer (which diffs the built `.o` against the oracle
EXE bytes directly, not this stale reference file).

OBJECT MODEL: every DATA MODEL signal below is a field of `Unk80101EC8Record` (already declared,
`include/**/*.h`) or an already-declared global; none needs a declaration fix. MATCHES (measured
score 0) for all, since the whole function — which touches every one of these fields through the
typed struct access already — scores sandbox 0 this session:
- D_80101F32, D_8010237E, D_80102334, D_80101EE8, D_80102016, D_80102462, D_80102092,
  D_801024DE, D_8010259A, D_8010214E, D_80101ED6, D_80102322, D_80101FFC, D_80102004,
  D_80102448, D_80102450 — all are raw splat field-offset symbols for `D_80101EC8[0]` /
  `D_80101EC8[1]` members (`Unk80101EC8Record`), already accessed through the struct in
  src/main/17AFC.c's func_800288C8 body (unk_6A, unk_14E, unk_1C8.vy, unk_0E, unk_20,
  unk_134.vx/vz, unk_286, unk_F4.x). MATCHES (sandbox 0/465 this session).
- D_800A3144 (no header decl; `s32[]` radius table, indexed `D_800A3144[i]`/`[j]`), D_800A3876,
  D_800A38A8 (both declared `extern s16` in bb2.h), g_sqrt_table_u8 (declared in bb0.h) — all
  already used correctly. MATCHES (sandbox 0/465 this session).

Three FAKE-annotated locals already in the body, each with an inline comment citing a specific
ordinary-c-judge-decidable family:
- `dist` — Ruling 11, multi-write local, every write a real computation (squared-distance sum,
  then one of two square-root computations on mutually exclusive branches) plus one ordinary
  divide-by-zero clamp (`if (dist == 0) dist = 1;`, ordinary C, not itself a Ruling-11 concern).
- `tbl` — Ruling 11 (C)(3) Q28 exception (GTE-macro input copy): `tbl = dist;` is read only as
  the bare input operand of the admitted `gte_Lzc` macro unit below it, then `tbl` later holds
  the real table-byte computation. Comment cites "Q28" explicitly.
- `shift` (block-scoped, inside the `else` arm) — `shift = lzc_out; shift = 0x16 - (shift & ~1);`
  reads lzc_out's value exactly once before overwriting; this is the Ruling 4 (2026-09-02)
  compound-assignment-split shape (`v = a; v = f(v);`), which ordinary-c-judge-decidable states
  needs NO FAKE annotation at all ("truthful C under Ruling 1(3) when no statement is dead").
  The existing `/* FAKE: */` label on `shift` is conservative/over-cautious, not false — it
  still asserts nothing untrue — but a fresh layer-2 reviewer may reasonably drop it as
  unnecessary. Flagging for the session that runs layer-2, not changing src/ this session
  (recon modality; no src edits made).

The GTE island (6 `__asm__` statements implementing `gte_Lzc(tbl, &lzc_out)`) matches the
GTE leaf-wrapper / inline_o.h-class shape character-for-character per the existing
`inline_asm_canonical.txt` row; nothing to re-derive.

CONCLUSION: this function is NOT a gap needing a codegen lever. It is sitting at sandbox 0 /
full-build-SHA1-match, with its canonical-asm row already written, waiting only on a fresh
layer-2 (or Judge) PASS. Recommended next action: submit for review (candidate-ready), not
further codegen search.

## s1 cont. — the above was already Judge-FAILed before this turn; the real finding is a rule conflict

`state.json` review_ledger shows the body described above (the one currently in `src/`,
identical to `candidate.c` and to `rejected/judge-fail-1015-1452.c`) already went to the Judge
this session cycle and got a **FAIL** at 2026-10-15 19:52 (docs/grind/decisions.md:1304-1306,
"final call"). The FAIL is NOT on items 1/4/5 (byte match, existing types, simplest form) or on
the three FAKE locals (`dist`/`tbl`/`shift` — self_vet.md T1-T6 above, unchanged) — it is
specifically on the GTE island's SPELLING: `queue.json`'s `reopen_reason` (owner 2026-10-08,
commit 41a986d5f) flags "the bytes are right; the source form is not" and the recorded
`judge_constraints` in state.json demands: *"Land gte_Lzc as a macro call from a verbatim PsyQ
header (gtemac.h/inline_o.h in include/), not as written-out __asm__ statements."*

Checked whether that target form is actually reachable with current infrastructure:
- `find include -iname "*inline*" -o -iname "*gtemac*"` -> **no matches**. No verbatim PsyQ
  header (`inline_o.h`/`gtemac.h`) exists anywhere under `include/`.
- `engine/gtemacro.py` `PINNED` (line ~82-231) holds the `gte_Lzc` macro text only as a Python
  string table used to RECOGNIZE/match written-out `__asm__` statement runs against the pinned
  text (`match_unit`, `unit_spans`) — it is a detector, not a C header; `#include`-ing it is not
  a thing, and there is no C preprocessor macro definition anywhere in the tree that a call-site
  `gte_Lzc(tbl, &lzc_out);` could expand through.
- `.claude/rules/inline-asm-policy.md` line 51 (2026-09-23 ruling, condition 3, which the
  2026-09-26 class-grant route's condition D explicitly re-incorporates) states the OPPOSITE of
  the Judge's constraint, verbatim: *"Inline and the only asm: written out in `src/**/*.c` (no
  `#include`, BB2-local header or macro-by-name); everything else is ordinary C passing normal
  review."* This is the CURRENT text (confirmed by direct Read this session, not a stale
  memory) — not a narrower/older scoping that the brief's "CURRENT SCOPE" block would override
  (inline-asm-policy's own `description:` line, extracted at dispatch, repeats the "verbatim
  PsyQ GTE macro islands under the 2026-09-23/26 rulings" framing with no macro-call carve-out).

So the Judge's final-call constraint and the written, currently-in-force inline-asm-policy.md
text directly contradict each other on the one axis the FAIL turns on (written-out-statements
vs. `#include`+macro-call), and the macro-call form the Judge wants has zero supporting
infrastructure (no header file, no gtemacro.py recognition path for a call-site form) for a
grind session to build — creating a new verbatim PsyQ header under `include/` and teaching the
detector/scorer to recognize a macro-call island is an `engine:`/infra change, which a grind
session may not make (CLAUDE.md: never edit `engine/`, `.claude/rules/`; `docs/grind/decisions.md:1306`
reaches the identical conclusion independently: "the rule text must be reconciled before this
can close... That is owner/operator infrastructure"). This is not a codegen-lever question;
no amount of C-side search in this function reaches a form outside that contradiction. Session
disposition: `ruling-request`, not `candidate-ready` (resubmitting the identical FAILed body
would be rejected with no review per the body-hash-keyed review-loop rule) and not further
probing (there is no third C spelling to try — the two forms the policy texts disagree about
are the written-out form already on `src/` and the macro-call form that doesn't exist yet).

- [s1] sandbox func_800288C8 --disable all (this session) -> score 0, target_insns 465, build_insns 465, cheat_asm_stripped 0 -- confirms the already-banked s1 finding still holds.

- [s1] canonical func_800288C8 -> ASM-PARTIAL with an existing inline_asm_canonical.txt row (line ~406) citing the 2026-09-26 verbatim-macro-island ruling; the mixed C/asm grant is already written.

- [s1] inline-asm-policy.md:51 (2026-09-23 route condition 3, re-incorporated by the 2026-09-26 class grant condition D) states written-out-only, no #include/macro-by-name -- the literal opposite of what the 2026-10-08 reopen_reason / Judge final-call constraint demands for this function.

- [s1] docs/grind/decisions.md:1304-1310 records the Judge FAIL and a follow-up ruling entry (14:57) that already answered the policy question in favor of the later-dated 2026-10-08 direction (option a) -- i.e. the owner-facing question this ledger previously flagged as open has already been answered; there is no new ruling_question to ask this session.

- [s1] The three FAKE-annotated locals (dist, tbl, shift) are not the blocker: each cites a specific ordinary-c-judge-decidable family (Ruling 11 real-computation for dist, Ruling 11 (C)(3) Q28 GTE-input-copy for tbl, Ruling 4 compound-assignment-split for shift) and were not the ground of the Judge's FAIL per decisions.md:1310.

- [s1] The rule-conflict and infra gap are already fully documented in this ledger (evidence.md s1/s1 cont., hypotheses.md H2/H3); this session re-verified both are still accurate as of 2026-10-15 rather than re-deriving them.
