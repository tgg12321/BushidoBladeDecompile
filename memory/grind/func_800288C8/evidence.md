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
