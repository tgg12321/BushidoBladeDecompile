# Owner ruling 2026-09-07: the near-verbatim library tier is admitted, audit-gated

**Ruling (owner, in conversation, 2026-09-07):** the eight LIBSND/LIBSPU identifications the
2026-07-10 hunt graded NEAR (module bit-identical to a Sony build except 1-5 words at exact
placement; `memory/closer/libsnd-hunt-report.md`) may be named, **after an audit** to the same
default-refute standard as the 2026-08-18 Sony-static verification. This supersedes the
2026-08-18 HELD-BY-POLICY row for `_SsVmKeyOffNow` in
`naming-verification-2026-08-18.md`. The owner's false-positive directive still governs:
a row lands only if it survives the audit; PLAUSIBLE rows stay unapplied.

## What the audit found (tools: `tmp/near_audit/audit.py`, `web.py`; archived here as
`near_audit_report.md`, `callee_web.md`, `callee_web_data.md`)

Two evidence classes fell out, one stronger than expected:

1. **`libscan-xref`** (byte-level, VERIFIED-grade): an ACCEPTED bit-verbatim module's
   external `jal` relocation names its callee (the OBJ XREF record), and the EXE word at
   that relocation encodes where the linker resolved it. The function at that address IS
   that symbol by construction of linking; the callee's own body never enters into it.
   Same channel the 2026-08-07 ambiguous-tie resolutions used, applied exhaustively to
   all 171 accepted placements. Every callee agrees with our tree except the rows in the
   manifest.
2. **`libscan-near`** (CORROBORATED): the function's own body matches a Sony build of the
   module except a handful of words that are explainable source-level edits (the UT_KEYV
   `addu->subu` / shift-amount pair = the `_svm_voice` record stride change 52->54 bytes, the
   same edit 4.1 VM_NOWOF already carries; the verifier corrected an earlier reading that
   tied it to the changelog's "Ut volume" fix) or scheduler swaps (`jr ra` / delay-slot store), with the placement
   unique in the whole image (second-best position's mismatch is an order of magnitude
   higher) and the XDEF at exact offset 0 of a function start.

A third finding: 17 functions whose **C definition in `src/` still carries the pre-libscan
misnomer** although the glabel and aliases were corrected on 2026-08-07 (`SpuFree` is
defined as `spu_DmaTransfer`, `_SpuInit` as `spu_InitEx`, `GetVideoMode` as
`sys_GetVideoMode`, ...). The census read the glabel and reported them KEEP, so the live
names in the code were never audited. `build_census.py` now compares the link-map
object symbol against the glabel and emits RENAME for the desync.

The verifier's per-XDEF sub-function scans strengthened several rows beyond the whole-module
figures (SsUtSetDetVVol 0/26 at exactly 0x80086014 from the 4.0 sub-function; SsUtGetDetVVol
0/14 in 4.1-4.4; _SsVmGetSeqLVol/RVol 0/17 each) and added _SsVmGetSeqVol (0/24 at 0x80087CAC).

## Not admitted

- `SsUtKeyOffV` at 0x80085DD4 is a genuine second entry point inside `func_80085A40`
  (preceded by `jr ra` + delay slot) — a splat boundary follow-up per
  `boundary_fixes.md`, not a rename.
- PROBABLE rows of the hunt that no verbatim caller pins (link-order + size fit only):
  `SsUtAllKeyOff`, `SsUtSetVVol`, `_SsVmKeyOn`, `SpuSetVoiceAttr`, `_SsSeqGetEof` — stay
  auto-named until a caller or a closer build proves them.
- The 67 Sony **global** names the data web recovered (`callee_web_data.md`; e.g.
  `_svm_cur` = 0x801027F0, `_ss_score` = 0x80106F28, `_spu_RXX` = 0x800A2CDC) are a
  separate data-naming wave: they touch `src/` externs and several existing game-flavoured
  names (`g_weapon_frame_*`, `g_satan1_slot_state_*`) that are demonstrably the sound
  driver's voice-manager state.
