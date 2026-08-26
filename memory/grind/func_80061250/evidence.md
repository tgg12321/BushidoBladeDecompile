# Evidence bank — func_80061250

## s1 (2026-08-26, recon) — MATCH FOUND; full-build SHA1 == oracle; sandbox floor 2 is a symbol-alias artifact

- **Target shape (asm/funcs/func_80061250.s, 59 insns):** head = flag test/clear on the
  D_800F1154 flag block bytes +5/+6 (splat names byte +5 as its own symbol `D_800F1159`),
  with ONE base register ($v1) la-materialized once (`lui/addiu %hi/%lo(D_800F1159)`) and
  every flag access offset off it (`lbu 0/1($v1)`, `sb 1/0($v1)`, re-read of byte 0 after
  the clear); `addiu $a0,$v1,1` computed in a branch delay slot and later stored to
  D_800F1180; per-arm FULL constant materialization (lui/ori 0x210009 in one arm, lui-in-
  delay-slot/ori 0x21000A in the other — no shared `lui 0x21`); L2B0 arm reloads the s32*
  handle from gp-rel D_800A3468 and stores &flags to +0x14 off it; tail = jal func_80060A68
  then 3-load/3-store copy `0/4/8($s0)` → D_800F1140/44/48 with the 0xFF0060 mask store to
  gp-rel D_800A3464 interleaved between load 3's lui/ori and the last store.
- **The function is a structural clone of COMPLETED-C sibling func_8006156C**
  (src/text1b.c:3343, Judge PASS 2026-07-22 21:27; find_duplicates similarity 1.000,
  59/59). 6156C's target anchors its base at `D_800F1154+0x1` with the SAME la-base +
  offset-access + per-arm-constant shape, and its committed C spells the flags as
  `(&D_800F1154)[1]/[2]` with a `*p++` walking-pointer tail.
- **Measured spelling ladder (sandbox --disable all, 59-insn target):**
  - `(&D_800F1159)[0]/[1]` + walking-pointer tail: **19** (62 insns). Tail matched target
    EXACTLY (the `*p++` folds to `0/4/8($s0)` with the mask interleaved right); head
    diverged — every flag access folded to absolute `lui/%lo` macro addressing, no base
    register formed. Banked: rejected/s1-absolute-fold-19.c.
  - `u8 *flag = &D_800F1159;` + flag[k]: **25** (59 insns). Base register formed for the
    [0] accesses and the stored address value, but all three [+1] accesses folded to
    absolute `D_800F1159+1`, AND the two 0x2100xx constants shared one hoisted
    `lui 0x21` in the beqz delay slots (target materializes each arm's constant fully).
    Banked: rejected/s1-ptr-local-plus1-folds-25.c.
  - `volatile u8 *flag = &D_800F1159;` (+ volatile extern): sandbox-INERT — the ungranted
    volatile is stripped before scoring (cheat_asm_stripped 169→170); D_800F1159 is NOT in
    volatile_extern_allowlist.txt. Raw cc1 probe (tmp/grind/func_80061250/s1/
    probe_volatile.s) shows volatile fixes the offset-0 accesses but the +1 accesses STILL
    fold absolute and the constants still share the lui. Not needed anyway (see below);
    volatile is NOT the mechanism. Banked: rejected/s1-volatile-stripped-inert.c.
  - **`(&D_800F1154)[5]/[6]` (the 6156C-mirror block-offset spelling): sandbox 2, 59/59,
    and `build` full-clean SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (MATCH,
    measured THIS session with the candidate in src/text1b.c).** This is the current
    src/text1b.c body and memory/grind/func_80061250/candidate.c.
- **Mechanism of the shape (measured, not guessed):** cc1 (GCC 2.7.2) keeps a la-anchored
  base register for a flag-block access run ONLY when the anchor's cse equivalence is a
  COMPOUND constant (`(const (plus D_800F1154 5))`). With a plain-symbol anchor
  (`&D_800F1159`, addend 0) cse substitutes the symbol into every derived MEM address and
  folds the accesses to absolute assembler-macro addressing — measured in both the [0]/[1]
  spelling (19) and the pointer-local spelling (25, +1 folds only). The compound-equiv
  anchor also stops the cross-arm `lui 0x21` share (per-arm full li, matching target).
  This is exactly why sibling 6156C's committed `(&D_800F1154)[1]` spelling matched: its
  first-access anchor is symbol+1 ≠ plain symbol.
- **The residual sandbox 2 is a scoring artifact, not a byte difference:** the two diffs
  are the two `addiu` instructions carrying `%lo(D_800F1154)+5` (imm field 5, reloc vs
  D_800F1154) where target.s carries `%lo(D_800F1159)+0` (imm field 0, reloc vs
  D_800F1159). D_800F1159 = 0x800F1159 = D_800F1154+5 (undefined_syms_auto.txt:516), so
  both resolve to identical instruction words at link — proven by the full-build SHA1
  MATCH above. engine/score.py deliberately does NOT mask named-symbol reloc addends
  (auto-memory project/sandbox-lo16-text-addend-false-distance, FIXED-scope note: "Named-
  symbol relocs are NOT masked"), so NO C spelling that anchors off D_800F1154 can score
  below 2, and (measured above) no D_800F1159-addend-0 spelling reproduces the target
  shape. The sandbox floor for this function is artifact-locked at 2 with the bytes
  already exact.
- Canonical verdict C (queue record; family verdict per docs/grind/decisions.md:1347).
- Zero regfix/asmfix rules; the retired chassis' `register asm("$2"/"$3")` pins are GONE
  (the walking-pointer tail replaces them, same cure as 6156C); no volatile, no FAKE
  constructs — every statement in the candidate is semantic and mirrors the Judge-PASSed
  6156C body verbatim (offsets/codes/mask differ only as the data differs).
- Artifacts: tmp/grind/func_80061250/s1/cc1_probe.sh (raw-pipeline cc1 probe harness),
  tmp/grind/func_80061250/s1/probe_volatile.s (raw cc1 output, volatile spelling).

## s1 re-run (2026-08-26, recon; HEAD 351b99be) — sandbox 0 + oracle MATCH re-proven; the "artifact floor 2" claim is FALSIFIED

- The banked candidate (identical body, `(&D_800F1154)[5]/[6]` block-offset spelling +
  `*p++` tail) was re-applied to src/text1b.c, replacing `extern volatile u8 D_800F1159;`
  + `INCLUDE_ASM("asm/funcs", func_80061250);` at old lines 3271-3272.
- **`sandbox func_80061250 --disable all` printed score 0, 59/59 insns** (rules_dropped 0,
  cheat_asm_stripped 169 — all from other functions; nothing stripped from this body).
- **Full clean `build`: SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa — oracle MATCH**,
  measured this session with the body in place.
- The discarded prior s1's central claim — "engine/score.py does not mask named-symbol
  reloc addends, so no D_800F1154-anchored spelling can score below 2" — does NOT
  reproduce on this chassis. target.s still relocates its two flag-base addius against
  D_800F1159 (asm/funcs/func_80061250.s:5-6,31-32) and the C against D_800F1154+5, yet
  the scorer prints 0. No engine/score.py or symbol-file commit landed in between (last
  scorer commit is 4dfc7223, the 2026-08-07 section-symbol masking fix), so the prior
  "2" was a mis-measurement or a transient state, not a scorer property. The three
  escalation remedies proposed in the discarded decisions.md entry (scorer fix /
  aggregate merge / manual integration) are all UNNECESSARY — the normal pipeline path
  is open.
- Spelling-ladder kills from the prior s1 (rejected/s1-absolute-fold-19.c,
  s1-ptr-local-plus1-folds-25.c, s1-volatile-stripped-inert.c) remain banked and valid
  as measured comparisons; only the artifact interpretation of the final form is
  superseded.
- self_vet.md written this session (all six tests PASS; no FAKE constructs, no
  sanctioned-family claims — every construct mirrors Judge-PASSed sibling
  func_8006156C's committed body).

## s2 (2026-08-26, recon; HEAD 95de1421) — sandbox 0 re-proven; citation-only layer-1 FAIL cured with post-application line numbers

- The banked candidate body was re-applied to src/text1b.c (replacing
  `extern volatile u8 D_800F1159;` + `INCLUDE_ASM("asm/funcs", func_80061250);`
  at pre-edit lines 3271-3272; body now at src/text1b.c:3272-3299, extern at 3271).
- **`sandbox func_80061250 --disable all` printed score 0, 59/59** (rules_dropped 0,
  cheat_asm_stripped 169 — all other functions' legacy debt; nothing stripped from
  this body). Measured THIS session with the edit in place.
- **Root cause of the s1 layer-1 FAIL identified and cured:** the prior self-vet
  cited sibling func_8006156C at PRE-application line numbers (3342-3369 / 3348);
  inserting the ~28-line candidate above the sibling shifted it to 3370-3397, so
  the reviewer (reading the post-application file) found the citations did not
  resolve. The reviewer explicitly verified the underlying claim TRUE (constraint
  bank: "Construct is legitimate... citation_only: true"). s2's self_vet.md
  re-greps every citation AFTER the edit: sibling body src/text1b.c:3370-3397,
  flag test `(&D_800F1154)[1]` at 3375, our test `[5]` at 3277, sibling Match
  commit 450f69d1 (2026-07-22). The banned-construct entry (re-declaring the
  stale 3348/3342-3369 citations) is satisfied by NOT reusing those numbers.
- No new spellings probed — none needed; the C is done, the residual was
  bookkeeping fidelity.

## s3 (2026-08-26, recon; HEAD ecc1e876; dispatched as "session 1" of the post-discard renumbering) — ARRAY spelling measured sandbox 0; recon map for the aggregate question; ruling-request filed

- **Context inherited:** two layer-1 FAILs are banked. FAIL 1 (14:56) was citation-only.
  FAIL 2 (15:03) is SUBSTANTIVE and now a driver-enforced ban: the pointer-pun spelling
  `(&D_800F1154)[5]/[6]` (and address-of forms) off a single-byte `extern u8 D_800F1154;`
  fails T3/T4 (ladder-selected for its CSE base-register anchoring) and, by analogy, the
  aggregate-merge family's prongs. The reviewer's own prescribed next action: "properly
  invoke the aggregate-merge family: declare the D_800F1154 flag block as a real
  header-level aggregate" (docs/grind/decisions.md:14502 + banned-constructs bank).
- **KEY MEASUREMENT (this session):** the ARRAY spelling — block-scope
  `extern u8 D_800F1154[];` + `D_800F1154[5]/[6]` reads/writes + `&D_800F1154[5]/[6]`
  publishes (a real ARRAY_REF over a declared aggregate, not the banned pun over a
  scalar) — applied over the INCLUDE_ASM at src/text1b.c:3271-3272 scored
  **sandbox 0, 59/59** (rules_dropped 0, cheat_asm_stripped 169 all from other
  functions). Same compound-const cse anchor (const (plus D_800F1154 5)) → identical
  RTL to the s1/s2 form whose full-build SHA1 == oracle was already proven (s1 re-run).
  Probe copy: tmp/grind/func_80061250/s3/probe_array_spelling_sandbox0.c. The src edit
  was REVERTED after measurement; src is back to INCLUDE_ASM, tree clean.
- **Recon map of the 0x800F1152..0x800F115F block (for the aggregate question):**
  - Splat symbols (undefined_syms_auto.txt:511-518): D_800F1150, 1151, 1152, 1154,
    1158, 1159, 115B, 115C.
  - Naming census (named_syms.txt): 0x800F1154 = g_text1b_glyph_buf_b (a BUFFER —
    aggregate model, line 827); 0x800F1158 = g_text1b_glyph_buf_b_plus_4 (line 2507);
    0x800F1159 = g_text1b_slot_d_flags "u8[2] busy-flag pair" (line 1913);
    0x800F115B/115C = g_text1b_glyph_data_b1/b2 (lines 2205-2206);
    0x800F1152 = g_text1b_slot_b_flags u8[2] (line 1328).
  - Committed C consumers in src/text1b.c (line numbers at HEAD ecc1e876):
    func_800618B4 :3507 uses `extern u8 D_800F1152[];` with [0]/[1] flag indexing
    (the SAME test-and-clear shape as ours, array spelling, committed);
    func_8006156C :3343 uses the legacy pointer-pun `(&D_800F1154)[1]/[2]` off a
    scalar extern (Judge PASS 2026-07-22, pre-dates the 15:03 ruling);
    func_800619A4 :3544 publishes `&D_800F1158` (extern s32, address-only use —
    the s32 type is decl noise); func_800619F0 :3559 uses
    `extern u8 D_800F1154[];` + publishes `D_800F1154 + 3` (ARRAY spelling with
    offset, committed and integrated — the direct precedent for our form);
    siblings at :3293-:3333 (old numbering) publish &D_800F115B / &D_800F115C+k.
    text1b_b.c:591-599 carries a mirror extern block incl. the stale
    `extern volatile u8 D_800F1159;` chassis debt.
  - Asm references: D_800F1159 appears ONLY in asm/funcs/func_80061250.s (self —
    deleted at integration) and asm/text1b.s (NOT linked: bb2.ld links only
    build/src/text1b.o; verified by grep of bb2.ld this session). So at integration
    the last D_800F1159 reference vanishes and a complete symbol merge
    (prong (c): remove merged symbols from splat config) is mechanically possible.
    D_800F1154 is still referenced by INCLUDE_ASM siblings func_8006133C.s (and
    remains as the merged base symbol — no conflict).
- **Why this is a ruling-request, not a candidate-ready:** the array spelling closes
  the bytes, but its classification is genuinely contested between two adjacent
  positions: (A) ordinary C — it adopts the TU's EXISTING committed object model
  (func_800619F0's array extern predates this session; census says buffer), no
  family claim needed; (B) the 15:03 reviewer's prescription + aggregate-merge
  prong (d) ("spelled at the canonical declaration in the shared header, never
  TU-local") + prong (c) (complete merge, symbols removed from config) require the
  FULL 5-prong merge first — which touches sibling bodies (respell 6156C's pun),
  a shared header, text1b_b.c, and undefined_syms_auto.txt, i.e. surfaces beyond
  this session's scope (driver scope-widening per integration-handoff-self-serve
  would be needed). Submitting under (A) risks layer-1 FAIL #3 ("respelled banned
  construct" — the bytes and mechanism are identical to the banned pun; only the
  type-level object claim differs); submitting under (B) is impossible in-session
  (scope) and fails prong (d) if spelled TU-local. Per the role prompt ("torn
  between two adjacent families → ruling-request; one ruling is cheaper than a
  FAILed candidate"), s3 files the ruling-request with the measurement in hand.
- (s4 note) Extent sub-question banked for the ruling: a minimal merge (D_800F1154+D_800F1159
  → `extern u8 D_800F1154[7or8];`) leaves D_800F1158/115B/115C as separate handles
  overlapping the census's buffer model (1158 is address-only s32 — 4-byte handle
  overlapping bytes 4-7); a full-block record (u8[8+] or struct) is census-cleaner
  but forces respelling MORE committed siblings. The evidence supports "flat u8
  byte buffer" (every access in the family is byte reads/writes or interior
  address publishes; no multi-byte load/store of the block exists in target asm).

## s4 (2026-08-26, recon; HEAD 2b38d6f6) — INTEGRATION HANDOFF EXECUTED: all three edits staged, sandbox 0 AND full-build SHA1 == oracle measured together

- **What changed vs s3:** nothing about the C. s3 measured the array spelling at
  sandbox 0 but could not stage the D_800F1159 retirement (out of scope) and so filed
  a ruling-request. The grinder Judge answered it on 2026-08-26
  (docs/grind/decisions.md:14504 ruling, :14520 ESCALATE packet): option (A) — the
  array-extern spelling IS ordinary C and does NOT require the 5-prong aggregate-merge
  invocation — dispositioned as an integration-handoff, and the DRIVER then wrote the
  scope grant `func_80061250 src/text1b.c src/text1b_b.c undefined_syms_auto.txt`
  (tools/grinder/scope_allow.txt:34). s4 inherited that grant and simply executed it.
- **The three edits staged this session (all inside the grant; the Judge's binding
  constraint at decisions.md:14543 requires them in ONE commit):**
  1. `src/text1b.c` — `extern volatile u8 D_800F1159;` + `INCLUDE_ASM("asm/funcs",
     func_80061250);` (pre-edit lines 3271-3272) replaced by the C body, now at
     src/text1b.c:3271-3298.
  2. `src/text1b_b.c:591` — the dead `extern volatile u8 D_800F1159;` deleted (that TU
     had zero uses of the symbol).
  3. `undefined_syms_auto.txt:516` — `D_800F1159 = 0x800F1159;` deleted.
  Exactly one C handle now exists for bytes 0x800F1159-0x800F115A: `D_800F1154[5]/[6]`.
- **Reference audit re-run at HEAD before editing** (grep over the tree, excluding
  memory/docs/tmp/metrics): D_800F1159 appeared in exactly five places —
  undefined_syms_auto.txt:516, the two dead externs, `asm/text1b.s` (NOT linked; bb2.ld
  links build/src/text1b.o), and `asm/funcs/func_80061250.s` (this function's own
  INCLUDE_ASM source, which stops being assembled the moment the C lands). The Judge's
  independent audit is confirmed exactly.
- **MEASURED THIS SESSION, with all three edits in place:**
  - `sandbox func_80061250 --disable all` → **score 0, target_insns 59, build_insns 59,
    rules_dropped 0** (cheat_asm_stripped 169 = other functions' legacy debt; nothing
    stripped from this body).
  - `build` (full clean) → **SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle,
    MATCH**. This is the load-bearing measurement for the RETIREMENT specifically: the
    oracle still matches with the splat symbol deleted and both externs gone, which is
    what proves the D_800F1159 handle was genuinely dead.
- **Citation hygiene (the s1 layer-1 FAIL's root cause) handled:** every file:line in
  self_vet.md was re-grepped AFTER the edits were applied, against the file a reviewer
  will actually read. Post-edit anchors: our body src/text1b.c:3271 (array decl :3272,
  flag test :3277); sibling func_8006156C src/text1b.c:3370-3397 (its `(&D_800F1154)[1]`
  pun at :3375 — the legacy pre-ruling form, deliberately NOT copied); committed
  array-extern precedents src/text1b.c:3557 + :3563 (func_800619F0), :3505
  (func_800618B4 over D_800F1152), :3589 (D_800F1164); census named_syms.txt:827.
- **Residual bookkeeping, NOT a blocker (banked for whoever runs the naming census
  next):** `named_syms.txt:1913` still carries `g_text1b_slot_d_flags = 0x800F1159`
  (a census NAME claim, not a splat symbol definition). It is inert — the full clean
  build matches the oracle with it present and undefined_syms_auto.txt's line gone —
  and named_syms.txt is outside this session's scope grant, so it was left untouched.
  Under the census's own buffer model (0x800F1154 = g_text1b_glyph_buf_b) the two flag
  bytes are interior bytes of that buffer, so the entry is arguably redundant now.
- No new spellings were probed this session and none were needed: the C was settled by
  s3's measurement and the Judge's ruling. s4's work was the surface execution plus the
  proof that the retirement is oracle-safe.
