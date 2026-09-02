# Evidence bank — func_800861BC

## s1 (recon, 2026-09-02) — identity, mechanism, bytes proven

### Identity
- func_800861BC is Sony LIBSND vmanager `_SsVmDoAllocate` (psyz
  `tmp/psyq_prov/psyz/decomp/src/libsnd/vm_aloc2.c`, non-matching there:
  `INCLUDE_ASM` outside `__psyz`). BB2 deltas vs the psyz body: `_svm_voice`
  stride 54 (not 52; matches the note2pitch sweep finding), `_svm_pg` held in
  the s32 global D_800FF6A0 (declared s32 at src/main.c:1233 by a matched
  sibling — cast, not redeclared), adsr writes indexed `voiceOffset+4/+5`
  through the flat s16 view `D_80102A78[]` (psyz uses `[voice].adsr[i]`;
  BB2's asm reads voiceOffset for them), 16-entry `_svm_envx_hist` loop.
- Symbol map (all verified against the target asm offsets):
  `_svm_cur` = struct at 0x801027F0 (psyz `struct struct_svm`, 0x20 bytes):
  +0x01 vabId (D_801027F1), +0x06 prog (D_801027F6), +0x07 fake_program
  (D_801027F7), +0x0C tone (D_801027FC), +0x16 seq_sep_no (D_80102806),
  +0x18 tone_vag_idx (D_80102808), +0x1A voice (D_8010280A), +0x1C
  voiceOffset (D_8010280C), +0x1E field_0x1e (D_8010280E).
  `_svm_sreg_buf` = D_80102A78 (s16 view; +6/+8/+10 = start addr / adsr1 /
  adsr2 = D_80102A7E/80/82), `_svm_sreg_dirty` = D_800F65E0,
  `_svm_envx_hist` = D_80107898[16], `_svm_damper` = D_800F66F8,
  `_svm_tn` = D_80101BC8 (VagAtr*), `_svm_voice[].unk6` = D_800F4E1E.
- Object-model evidence for the struct that PREDATES this grind (prong a of
  the aggregate-merge family): (1) the target binary derives &D_8010280A as
  `addiu $t1, $v1, -2` off `$v1 = &D_8010280C` (asm/funcs/func_800861BC.s
  lines 5-6) — base+offset addressing across two splat names; (2) psyz's
  header-canonical `struct struct_svm` (libsnd_private.h:150-171) places every
  one of the nine BB2 fields at exactly the BB2 offset; (3) the matched sibling
  func_800871D4 (src/main.c ~1179) already documents `D_8010280A <-
  _svm_cur.voice`; (4) named_syms.txt:3604 records D_8010280A as the recorded
  voice index of the stride-54 `_svm_voice` table.

### Mechanism (read from the s1 dumps, tmp/grind/func_800861BC/dumps/)
- `main.rtl`: EVERY `_svm_cur.<field>` access expands as
  `(set (reg) (const (plus (symbol_ref D_801027F0) off)))` + `(mem (reg))`.
  Cause: explow.c `memory_address` — "By passing constant addresses thru
  registers we get a chance to cse them" — `force_reg` on any CONSTANT_ADDRESS
  reached through `change_address` (struct offset). Plain scalar globals use
  DECL_RTL `(mem (symbol_ref))` directly and never get a pseudo. This is WHY a
  per-word-scalar spelling can never produce the target's `addiu $t1,$v1,-2`
  without a pointer pun: the mechanism requires two offsets of ONE symbol.
- `main.cse`: within one (extended) basic block cse.c `use_related_value`
  rewrites every later `_svm_cur` address as `(plus anchor delta)` where
  anchor = the FIRST materialized `_svm_cur` address of that block (e.g. tail
  block: r172 = sym+26 = voice, then voiceOffset = (plus r172 2), fake_program
  = (plus r172 -19), tone = (plus r172 -14)).
- `main.cse2` (after loop): every `(plus anchor delta)` with delta != 0 is
  re-folded to `(mem (const sym+off))` (direct lui/%lo); only exact repeats
  `(mem anchor)` keep the register (reg address cost 1 < sym+K cost 2,
  mips.c address cost). Surviving anchors in the final asm: $v1 = &voiceOffset
  (top block; loop's &voice = $v1-2 hoisted by loop.c), and the post-join
  block's anchor.
- The ONLY real divergence of the psyz-shaped body (H1, sandbox 26): post-join
  anchor. Ours anchored on `voice` (dirty|=8 is the first tail statement);
  target anchors on `voiceOffset` (`la $a1` used by both adsr reads) while
  both `voice` reads are direct. Hence in the target the dirty|=8 statement
  is NOT in the post-join block: it sits in the if/else arms (its arm-block
  address folds to a constant because the arm anchor is tone_vag_idx) and
  jump2 cross-jumping merged the two identical arm tails (`lui/addu/sh` +
  the dirty sequence) into the shared tail after `.L800862D8`. Two spellings
  realise that: a literal duplication into both arms (v4, FAKE family) or a
  `static inline` helper "write start-address shadow + mark dirty" called from
  both arms (v8, ordinary C; src/main.c already has `static inline` sites).

### Measurements (chassis: asm-until-matched HEAD 2829a7b0; TU-local
### struct_svm at D_801027F0 for scoring; 132/132 insns in every run)
| form | sandbox --disable all | real (filt.py) | addend noise |
|---|---|---|---|
| H1 psyz shape, struct | 26 | 13 | 13 |
| v2 unused local `voice = _svm_cur.voice` (psyz FAKE line) | 26 | 13 | 13 |
| v2b local voice used in dirty\|=8 | 26 | 13 | 13 |
| v5 adsr1 statement before dirty\|=8 | 43 | — | — |
| v7 dirty\|=8 before the if/else | 37 | — | — |
| v3 retired separate-scalar + pointer-alias body | 37 | — | — |
| **v4 dirty\|=8 duplicated into both arms** | **18** | **0** | 18 |
| **v8 static inline helper in both arms** | **18** | **0** | 18 |
- "addend noise" = hunks where the only difference is the LO16 immediate
  (`lh a1,26(a1)` vs `lh a1,0(a1)`): struct field addend vs splat per-word
  symbol. engine/score.py:61 deliberately does NOT mask named-symbol HI16/LO16
  addends, so NO struct spelling can sandbox-0 while the reference object
  (built from asm/funcs/*.s) names D_8010280C etc. Precedent: e788983a
  (func_8003B9D0) accepted with sandbox residual 3 of the same kind on the
  oracle SHA1.
- BYTES PROVEN twice: `verify-oracle` build_sha1 == 62efab4f... with (a) v4 +
  TU-local struct in src/main.c (s1/verify_oracle.txt; func_800861BC is a T
  symbol in build/src/main.o, no asm object linked), and (b) the COMPLETE
  merge: struct decl + all nine per-word symbols replaced at every main.c
  consumer (lines 923, 1148, 1190, 1218-1248, 1266, 1306-1313; diff banked as
  memory/grind/func_800861BC/merged_main_s1.diff) — s1/verify_oracle_merged.txt
  ok:true, build_matches:true. Prong (e) byte-neutrality for every consumer is
  therefore measured, not assumed. No other TU references these symbols
  (grep src/ include/).
- v8 measured only in the TU-local-struct chassis (sandbox 18 / real 0); its
  oracle run was not separately repeated (identical instruction stream to v4
  per pairdiff, and v4 is oracle-proven).

### Integration notes for the operator / next session
- Header-canonical decl: include/sound.h (only libsnd-ish header; 16 lines)
  or a new include/libsnd.h. Field types as in candidate.c (u8 + short;
  `-funsigned-char` makes psyz's `char` == u8). `voice` must be `short`:
  this function's first read is `lh`; the sibling's `lhu` (func_800871D4)
  comes from its u16 local, measured neutral under `short`.
- func_80087CAC (src/main.c:1213) carries a FAKE pointer alias to D_80102806
  (= `_svm_cur.seq_sep_no`); under the struct its natural field access would
  force_reg the address by the same explow.c mechanism — plausibly retires
  that FAKE (UNMEASURED; the merge test kept the pointer form `ptr =
  &D_801027F0.seq_sep_no`, which is byte-neutral).
- The 2026-08-17 ruling's asymmetry test ("if the struct form fails where the
  flat array closes → fold-escape tell") is passed: the struct form closes
  directly; no flat-array or pointer-pun form ever closed.

## s1 re-run (recon, 2026-09-02, second dispatch) — header form proven; escalation re-filed

Context: the first s1 was DISCARDED by the driver (owner-gated claim without an
OWNER-ESCALATION-titled entry). Its ledger files survived on disk and are the
inheritance above; nothing in them was re-derived — every number below was
RE-MEASURED this session on HEAD ea3f0a59 (asm-until-matched chassis, no FAKE
constructs anywhere in the diff).

### Measurements (chassis: HEAD ea3f0a59; header-form tree = integration_patch.diff)
| step | result | artifact |
|---|---|---|
| canonical func_800861BC | verdict C, hand_coded_tier LOW (distance 132 = size) | — |
| HEAD sandbox (INCLUDE_ASM) | no_c_body, 132 target insns | — |
| header form, sandbox --disable all | **18** at 132/132, rules_dropped 0 | s1/v9_header_form_pairdiff.txt |
| header form, filt.py | **real=0 noise=18** (all 18 hunks = same insn, LO16 immediate only) | same |
| header form, verify-oracle --rebuild --allow-dirty | ok:true, build_sha1 == oracle 62efab4f… | s1/verify_oracle_header.txt |
| nm build/src/main.o | `T func_800861BC` (compiled from C) | — |

### What the header form changes vs the discarded session's TU-local form
- `struct struct_svm` + `extern struct struct_svm D_801027F0;` now live in
  include/sound.h (prong d), and src/main.c gains `#include "sound.h"`
  (no name conflicts: g_snd_* / SsSetSerialAttr are not declared in main.c).
- BB2's existing `ProgAtr` typedef (u16 reserved2 / u16 reserved3 — BB2's
  own split of PsyQ's u32 reserved2, used by the matched sibling at
  main.c ~1374) moved from src/main.c into include/sound.h so it precedes
  func_800861BC; the function reads `.reserved2` / `.reserved3` directly.
  This replaced the discarded session's second, conflicting TU-local
  `ProgAtr` (u32 reserved2 + `((u16 *)&…)[0/1]` cast). Byte-neutral:
  18 / real 0 / oracle unchanged.
- Nine per-word externs deleted from C; 26 struct-field references in
  main.c; `grep D_801027F[1-9A-F]|D_8010280[0-9A-F] src/ include/` = none.
- Ten asm/funcs/*.s files still reference the per-word names, so their
  undefined_syms_auto.txt lines stay (exactly as e788983a kept them).
- `cheat_asm_stripped` reported by the sandbox went 20 (HEAD) -> 19 with the
  patch applied; not investigated (score unaffected, function has no asm).

### Why the candidate gate cannot print 0 for this function (driver source, this session)
- tools/grinder/grind.ps1:578-583 `Invoke-CandidatePath` requires
  `"score": 0` from `sandbox --disable all`, whose reference is
  build/src/main.o = the INCLUDE_ASM'd asm/funcs/func_800861BC.s naming nine
  per-word symbols at addend 0 (engine/sandbox.py:72).
- engine/score.py:61 masks only SECTION-relative HI16/LO16 addends; named-symbol
  addends are compared. Our struct spelling addresses every field as
  `D_801027F0+N`, N != 0 (the +0 field is never touched) -> 18 immediates differ.
- f45b33cd (2026-08-22): Fix A (session scope check honours scope_allow.txt)
  LANDED; Fix B (oracle-SHA1 acceptance at an addend-only residual) DECLINED,
  ground 2 being "No function is currently known to need Fix B". This
  function is that case. Post-landing, `engine build` regenerates the
  reference from C and the sandbox reads 0 (f45b33cd measured this on
  func_80038170), so a landed match survives `queue regen`.
- Precedent disposition: func_80038170 — Judge 2026-08-20 policy-question,
  owner cbcfda04 authorized the operator landing, landed 2fb471ac.

### Session end state
- Tree reverted to HEAD (src/main.c, include/sound.h) and build/ rebuilt
  from HEAD so no stale C-compiled reference object is left behind (the
  "false-zero recipe" the func_80038170 ledger killed).
- Escalation re-filed in docs/grind/decisions.md under the OWNER-ESCALATION —
  INTEGRATION HANDOFF title; outcome owner-gated with that entry as
  escalation_ref.

- [s1] Header-form tree banked as memory/grind/func_800861BC/integration_patch.diff (253 lines; touches exactly include/sound.h and src/main.c): sandbox --disable all = 18 at 132/132, rules_dropped 0; filt.py real=0 noise=18; verify-oracle --rebuild --allow-dirty ok:true, SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (tmp/grind/func_800861BC/s1/verify_oracle_header.txt); nm build/src/main.o shows T func_800861BC.

- [s1] canonical func_800861BC: verdict C, hand_coded_tier LOW (distance 132 is size, not hand-asm evidence). HEAD sandbox: no_c_body (INCLUDE_ASM), 132 target insns.

- [s1] Aggregate-merge prongs on the landed tree: (a) asm/funcs/func_800861BC.s:4-6 derives &D_8010280A as addiu $t1,$v1,-2 off &D_8010280C; psyz libsnd_private.h:148-172 struct_svm places all nine fields at the BB2 offsets; named_syms.txt:3604; sibling func_800871D4 comment. (b) struct with psyz field names. (c) zero per-word C handles remain (grep src/ include/); ten asm/funcs/*.s still reference the names so undefined_syms_auto.txt lines stay, as in e788983a. (d) include/sound.h. (e) oracle SHA1 on the exact tree.

- [s1] BB2's pre-existing ProgAtr typedef (u16 reserved2 / u16 reserved3) moved from src/main.c to include/sound.h; the body reads .reserved2/.reserved3 directly — byte-neutral vs the discarded session's cast form (18 / real 0 / oracle). This also removes the duplicate conflicting TU-local ProgAtr the discarded session's diff carried.

- [s1] Driver facts confirmed in source: grind.ps1:578-583 candidate gate requires sandbox score 0; f45b33cd landed Fix A (scope_allow grants honoured by the session scope check) and declined Fix B; engine/score.py:61 deliberately compares named-symbol LO16 addends; after landing, engine build regenerates the reference from C and the sandbox reads 0 (f45b33cd measured this on func_80038170), so queue regen will not resurrect the match.

- [s1] Session end state: src/main.c and include/sound.h reverted to HEAD; build/ rebuilt from HEAD (verify-oracle --rebuild) so no stale C-compiled reference object is left for a later session to false-zero against.

- [s1] Escalation entry filed: docs/grind/decisions.md '2026-09-02 — func_800861BC — OWNER-ESCALATION — INTEGRATION HANDOFF' with operator steps (git apply integration_patch.diff; build; layer-2 cheat-reviewer; queue done).

## s2 (structural, 2026-09-02) — INTEGRATION LANDED AND RE-MEASURED

The pipeline scope grant executed after s1 (commit 12a31f6d) added
`func_800861BC include/sound.h` to tools/grinder/scope_allow.txt, so the banked
integration patch is now inside the session's allowed surface. s2 applied it and
re-measured everything from scratch:

- `git apply memory/grind/func_800861BC/integration_patch.diff` applies clean at
  HEAD (include/sound.h + src/main.c only, 77 insertions / 37 deletions).
- `sandbox func_800861BC --disable all` immediately after applying = **18** at
  132/132, rules_dropped 0 (tmp/grind/func_800861BC/s2/sandbox.txt). This is the
  number the ledger recorded as the "floor" in s1, and s2 proves it is NOT a
  residual: it is stale-reference noise. `build/src/main.o` at that moment was
  still the pre-patch object, whose relocations name the per-word splat symbols
  at addend 0, while the new object names `D_801027F0` with a field addend;
  engine/score.py compares named-symbol addends, so all 18 hunks are addend-only.
  (Same mechanism as [[sandbox-lo16-text-addend-false-distance]].)
- `verify-oracle --rebuild --allow-dirty` (the dirty tree IS the intended new
  reference): ok true, build_sha1 = expected =
  62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true
  (tmp/grind/func_800861BC/s2/verify_oracle.txt). `mipsel-linux-gnu-nm
  build/src/main.o` -> `000025d8 T func_800861BC`: the function is compiled from
  C in main.o, not linked from an asm object.
- `sandbox func_800861BC --disable all` AFTER the rebuild = **0** at 132/132,
  rules_dropped 0, cheat_asm_stripped 19 (other functions in the TU)
  (tmp/grind/func_800861BC/s2/sandbox_post_rebuild.txt). Zero honest, cheat-blind
  distance with the cheat stripper on.

So the honest floor for func_800861BC is 0, not 18. The 18 was an artifact of
measuring an aggregate-merge rewrite against a reference object built before the
merge; any future session that sees 18 right after applying a symbol-shape change
should regenerate build/ before believing it.

Merge completeness (prong c) as measured: `grep -rn` over src/ and include/ finds
exactly one C handle (`D_801027F0`) for the nine merged locations and zero
residual per-word references. The undefined_syms_auto.txt lines for those names
must STAY for now — 11 files under asm/ still reference them, including the
still-INCLUDE_ASM'd siblings func_80086818, func_80086CF8, func_800872A4
(tmp/grind/func_800861BC/s2/asm_consumers.txt). They retire when those siblings
reach C; that file is also outside this function's scope grant.
