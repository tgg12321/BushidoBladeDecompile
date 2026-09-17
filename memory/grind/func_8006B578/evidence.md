# Evidence bank — func_8006B578

## OBJECT MODEL (recon, session 1)
- **D_800A350C** (census: "s16 pad-decode result base, 4 entries 350C/350E/3510/3512"): decl is
  a plain `extern s16 D_800A350C;` (matches existing split-scalar declarations already used by
  several OTHER functions in this file, e.g. `func_800692C0` at text1b.c:5803 — a function that
  appears to already be matched/committed C, not INCLUDE_ASM). func_8006B578 only ever takes
  `&D_800A350C` (address-of) and passes it straight through to `func_800692C0` as an `s16 *`
  argument; it never indexes past the base. Address-of a scalar and address-of element 0 of an
  array are byte-identical, so the split-scalar declaration is **not a source of divergence for
  this function** — MATCHES (measured: after applying the real function body with this
  declaration unchanged, 0 of the remaining 7 source-level diff hunks touch this symbol or its
  address computation). Merging D_800A350C/350E/3510/3512 into a real `s16[4]` array is a
  separate, project-wide, multi-file, multi-TU aggregate-merge undertaking (many already-matched
  callers in text1b.c/text1b_b.c reference the split scalars) — out of scope for this function's
  own byte match and NOT required to close it. Do not attempt the merge on this function's account.
- **jtbl_80015988**: no header decl (it's a rodata jump table, not a C symbol); the C `switch`
  statement over `(D_800A34F8 >> 10) & 7` with cases 0..5 reproduces it automatically via splat's
  existing rodata placement. MATCHES — no diff hunk anywhere touches the jtbl address or the
  `jr`/`lw %lo(jtbl...)` sequence itself.

## True signature (corrected from stale placeholder)
The pre-existing `void func_8006B578(s32 arg0, s32 arg1, s32 arg2, s32 arg3);` stub was WRONG —
the only real call site (text1b.c, `func_8006B898`, `func_8006B578(&arg0, &arg1);`) passes exactly
**2 pointer arguments**. The asm itself only ever touches `$a1` (never `$a0`/`$a2`/`$a3`), consistent
with a 2-param signature where the extra two never existed. Corrected signature used:
`s32 func_8006B578(s32 *arg0, s32 *arg1)` (arg0 unused, matches "asm proves at least 2 args" note
literally — it's exactly 2, not "at least 2"). This is NOT a codegen lever, it's a factual
correction; verified by re-running sandbox after the fix (see floor history).

## Structure (from asm/funcs/func_8006B578.s, corroborated against sibling func_8006B92C which is
apparently already-matched C in the same file with an almost identical shape — same
`func_800692C0(&sp10,0,D_800A34FC+0xC,&D_800A350C)` call, same `ret>>16` dispatch idea, same
`D_800A34F8` bitfield increment/decrement pattern, same trailing `*arg1&0x400040` / `*arg1&0x100010`
flag checks calling `func_8005C650`):

1. `v = *arg1; sp10 = (v&0xFFFF)|(v>>16); ret = func_800692C0(&sp10, 0, D_800A34FC+0xC, &D_800A350C);`
2. `switch (ret>>16)` — case 1: toggle/advance a 3-bit field at `D_800A34F8` bits 10-12 (mask
   0x1C00) with a `==0x1400` short-circuit, `func_8005C650(0,0x7F,0x7F)`, `D_800A3514=0`.
   case 2: mirror image (decrement, `==0` short-circuit). No default action.
   **Must be a real `switch`, not if/else-if** — the target's branch layout for this dispatch is a
   3-way beq/beq/fallthrough shape that an if/else-if chain emits with the WRONG branch sense
   (`bne` instead of `beq`) and instruction count; a real `switch(hi){case 1: ...; case 2: ...;}`
   fixed this exactly (see floor history s1: 71→35 from this one change).
3. `idx = (u32)D_800A34F8 >> 10 & 7; if (idx < 6) switch(idx) { ... }` — **the shift MUST be on a
   `u32`-typed read**; the file's only extern visible at this point in the TU
   (`extern s32 D_800A34F8;` at text1b.c:5644-ish) is signed, so a bare `D_800A34F8 >> 10` emits
   `sra` where target wants `srl`. Casting the read to `(u32)` before the shift fixes it (part of
   the 71→35 drop).
4. Switch cases 0/1/2 each: `if ((ret & 0xFF) != 0) { toggle bit N of ((s32*)D_800A3524)[8] via
   a3 = f & ~bitmask; bit = ((f>>N)&1)^1; (optionally <<N); a3 |= bit; store; func_8005C650(0,0x7F,0x7F); }`
   then **fall through (no separate flag check inside the case) into the shared post-switch
   block** — a first attempt that ALSO added an inline `if (*arg1&0x400040) {...; goto
   do_field_inc;}` per-case was WRONG: that flag check belongs ONLY to the shared post-switch
   code cases 0/1/2 fall into, not to the cases themselves (confirmed by the asm: `.L8006B6B8`
   etc. either take the `beqz` to `L8006B760` directly, or execute the bit-toggle and then
   fall through `L8006B754`'s tail straight into `L8006B760` — there's exactly ONE
   `*arg1&0x400040` check per case-0/1/2 path, not two). Fixing this dropped
   score 35→34 and floor insn-count to 199 (one BELOW target's 200 — over-merged somewhere, or
   target genuinely has an extra insn we don't reproduce yet; not yet diagnosed).
5. Switch cases 3/4/5: each does its OWN `if (*arg1 & 0x400040) { func_8005C650(1,0x7F,0x7F); ...;
   var_s2 = K; }` then jumps PAST the shared post-switch block straight to the final tail
   (case 3 additionally sets `D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) | 0x4000`).
6. Shared post-switch block (reached by fallthrough from case 0/1/2, or directly when `idx >= 6`):
   `if (*arg1 & 0x400040) { func_8005C650(1,0x7F,0x7F); field=((D_800A34F8>>10)&7)+1;
   D_800A34F8=(D_800A34F8&~0x1C00)|((field&7)<<10); }`
7. Final tail (`tail:` label, everyone converges here): `if (*arg1 & 0x100010) {
   func_8005C650(2,0x7F,0x7F); var_s2 = 1; } return var_s2;`

## Floor history this session (session 1, recon)
- start (placeholder INCLUDE_ASM stub body): 200
- literal first-pass translation (correct signature, structure above, if/else-if for hi dispatch,
  raw `D_800A34F8` shifts, extra flag-check duplicated into cases 0/1/2): **71**
- `switch(hi)` instead of if/else-if + `(u32)` cast on the two `D_800A34F8 >> 10` reads: **35**
- removed the erroneous duplicate `*arg1&0x400040` check from cases 0/1/2 (bug fix, not a lever):
  **34** (199 insns vs target's 200 — one under, not yet explained)

## Remaining residual (7 source-level hunks at floor 34, from `sandbox --diff`)
- Hunk around target-insn 122-126: target's instruction stream shows the ENTIRE shared
  post-switch block (`beqz; call; D_800A34F8 field update; sw; j; lui`) appearing to be inlined
  differently than ours around case 2's tail — our version differs starting there in COUNT
  (target block is much longer than what shows on our side at the same offset), suggesting
  target's cross-jump/inlining choice for the case-2 → shared-block transition differs from ours.
  NOT diagnosed yet — likely a cross-jump-store-tail-merge shape (see
  `.claude/rules/codegen-technique-index.md` § Cross-jump). Needs a `.sched`/dump read before
  guessing further (PASS ATTRIBUTION step not yet done this session — turn-budget ran out).
- Hunk at target-insn ~155/157/158: target has a `nop` then later
  `sw v0,0(gp); j 23980; li s2,2` as a group we don't reproduce in the same shape — ours instead
  emits `li s2,2` earlier and an extra `j`. This is in case-3's tail (the `D_800A34F8=...0x4000;
  var_s2=2;` arm) — the store/jump/const-load ordering differs from target's. Candidate next
  lever: reorder the statements in case 3 (store D_800A34F8 AFTER setting var_s2, or vice versa)
  and re-measure; NOT yet tried.
- We are 1 instruction UNDER target's count (199 vs 200) with the current form — worth checking
  whether target keeps a `nop` (delay-slot filler) we've optimized away structurally, rather than
  chasing a "missing" real instruction.

- [s1] OBJECT MODEL: D_800A350C (census: s16 pad-decode result base, 4 entries 350C/350E/3510/3512) -- MATCHES. func_8006B578 only ever takes &D_800A350C (address-of the base) and passes it through to func_800692C0 as s16*; it never indexes past the base, so the existing split-scalar `extern s16 D_800A350C;` declaration (shared with several already-apparently-matched sibling functions in the same TU, e.g. func_800692C0) is byte-identical to an array declaration for this function's purposes. 0 of the 7 remaining source-level diff hunks touch this symbol. Merging D_800A350C/350E/3510/3512 into a real array is a separate project-wide multi-TU undertaking and is NOT required to close this function.

- [s1] OBJECT MODEL: jtbl_80015988 -- MATCHES. No header decl needed (rodata jump table); the C switch over (D_800A34F8>>10)&7 with cases 0..5 reproduces it automatically via existing splat rodata placement. No diff hunk touches the jtbl address or the jr/lw-%lo(jtbl) sequence.

- [s1] The stale INCLUDE_ASM placeholder's 4-param prototype (s32 arg0..arg3) did not match the only real call site (func_8006B898: `func_8006B578(&arg0, &arg1);`, 2 pointer args) or the asm (only $a1 ever touched). Corrected to `s32 func_8006B578(s32 *arg0, s32 *arg1)`.

- [s1] Sibling func_8006B92C (same file, text1b.c:6563) is a near-identical smaller FSM (same func_800692C0 call shape, same D_800A34F8 bitfield increment/decrement pattern, same trailing *arg1&0x400040 / *arg1&0x100010 tail) and served as a structural precedent/sanity-check throughout.

- [s1] Current floor 34 (199 insns vs target 200, 1 UNDER target) with 7 source-level hunks remaining, concentrated around: (a) the case-2-to-shared-post-switch-block transition (target's instruction layout differs in count/shape there, not yet diagnosed with a pass-attribution dump), (b) case 3's tail store/branch/const-load ordering (target groups `sw; j; li s2,2` differently than our current statement order).

- [s1] No cheat/FAKE construct of any kind was used or is present in the current candidate — pure ordinary C (switch statements, u32 casts, real scratch locals for real intermediate values, one forward `goto tail` per case 3/4/5 that mirrors the asm's own explicit jumps past the shared block).

- [s2] sandbox --disable all --diff at the floor-34 candidate.c baseline: 25 hunks total, 7 source-level, 0 operand-only, 18 not-scored (masked branch/jump target noise, do not chase).

- [s2] All 7 source-level hunks (15-24, minus the not-scored ones interleaved) cluster around target insns 122-186, the switch-cases-3/4/5-to-shared-tail transition region.

- [s2] Direct read of asm/funcs/func_8006B578.s lines 60-219 confirms the jump table (jtbl_80015988) dispatches cases 0-5 to labels .L8006B6B8/.L8006B6E8/.L8006B720/.L8006B7B4/.L8006B7FC/.L8006B828 respectively, matching candidate.c's case order exactly (no signature/order error).

- [s2] Target keeps FOUR separate, byte-identical-looking inline copies of `lui v1,0x400040-hi; lw v0,0(s1); ori v1,...; and v0,v0,v1; beqz v0,SKIP; li a0,1; li a1,127; jal func_8005C650; li a2,127` — at .L8006B760 (fallback, >=6 case), .L8006B7B4 (case3), .L8006B7FC (case4), .L8006B828 (case5) — while our floor-34 build shares ONE copy across (at least some of) these sites, reached via jumps (visible as the extra `j 96a0` insert + huge-target-block-vs-tiny-ours-branch mismatch in diff hunks 15-16).

- [s2] Case5's target block (.L8006B828) ends with NO jump instruction at all -- it falls straight through into .L8006B850 (the shared tail entry) because it happens to be laid out immediately before it; this is consistent with case5 also literally writing `goto tail;` in the original C (a trivial jump-to-next-instruction elision), not evidence of a different C shape for case5.

- [s2] pwsh tools/grinder/dump.ps1 func_8006B578 ran clean this session (harmless pre-existing K&R-style redeclaration warnings only, same warnings the file has always emitted); dumps for the whole text1b.c TU are in tmp/grind/func_8006B578/dumps/ (func_8006B578 starts at text1b.jump2:53481) but were NOT read in detail for this specific region before the session's turn budget ran low -- flagged as the sharpened next probe.

- [s2] Chassis check confirmed at session start: sandbox --disable all == 34 with candidate.c applied, matching the ledger's last recorded floor exactly (no drift).

## Session 3 (permuter modality) — floor 34 -> 2, 0 source-level hunks left

- [s3] CHASSIS-CHECK CORRECTION: the ledger's "chassis measures 34 right now"
  dispatch line was WRONG — candidate.c was never actually applied to
  src/text1b.c between s2 and s3 (src still carried the raw INCLUDE_ASM stub,
  which sandboxes at 200). Applying candidate.c verbatim reproduced the
  documented 34 exactly, so s1/s2's measurements were real; only the
  inter-session persistence assumption was stale. Re-applying is now step 0
  for the next session too — always re-check `sandbox --disable all` right
  after applying candidate.c, don't trust the dispatch-line number blindly.

- [s3] ROOT CAUSE of the floor-34 plateau (frontier item 1 from s2, now
  RESOLVED): read `asm/funcs/func_8006B578.s` lines 60-219 directly (ground
  truth, not the jump2/combine dumps — those weren't needed this time) and
  found TWO separate structural bugs in the s1/s2 candidate, both now fixed:
  1. **idx>=6 must skip the shared 0x400040 block entirely.** The asm's
     `sltiu v0,v1,0x6; beqz v0,.L8006B850` sends idx>=6 STRAIGHT to the final
     tail check (`.L8006B850`/`.L8006B854`, the `*arg1&0x100010` block) —
     it does NOT execute the shared `*arg1&0x400040` field-increment code that
     case 0/1/2 fall into. The s1/s2 candidate's `if (idx<6){switch}` followed
     unconditionally by the shared-0x400040 `if` block let idx>=6 fall through
     into that shared code too — a genuine SEMANTIC bug, not just a layout
     issue (confirmed by objdump: with the OLD structure, idx>=6 compiled to
     `beqz v0,969c` — jumping INTO the shared block's start; with the fix, it
     compiles to `beqz v0,96e8` — the tail's own check, matching target's
     `.L8006B850` jump exactly). Fix: `if (idx>=6) goto tail;` before the
     switch, instead of wrapping the switch in `if (idx<6){...}`.
  2. **The shared 0x400040+field-increment block is physically laid out
     BETWEEN case 2 and case 3 in target, not after the whole switch.** Target
     asm: case 0 and case 1 each end with an explicit `j .L8006B754` (the
     toggle-call merge point); case 2 falls into `.L8006B754` naturally
     (no jump); `.L8006B754` flows straight into `.L8006B760` (the shared
     `*arg1&0x400040` check + field-increment) with NO branch in between;
     `.L8006B760` then does `j .L8006B854` to reach the tail, skipping
     `.L8006B850`'s own (now-redundant) `lui` — and ONLY THEN does the asm
     lay out `.L8006B7B4`/`7FC`/`828` (case 3/4/5). The s1/s2 candidate wrote
     the shared code AFTER the switch's closing brace in source, so GCC (which
     lays out switch case bodies in source/case order) placed case 3/4/5
     BEFORE the shared code, needing an extra `j 96a0; lui v1,0x40` glue
     hunk to reach it (the exact "hunk 15 extra insert" flagged at floor 34).
     Fix: moved the shared `*arg1&0x400040` block INSIDE the switch, as case
     2's fallthrough consequence (case 0 and case 1 `goto shared_400040;`,
     case 2 falls through into a `shared_400040:` label written immediately
     after its own body, before `case 3:`). This reproduces the target's
     physical ordering exactly (GCC lays out switch bodies in source order,
     so `shared_400040:` now sits between case 2 and case 3 in the emitted
     code, matching target byte-for-byte in this region).
  Combined effect measured via `sandbox --disable all`: 34 -> 11 (fix 1
  alone, applied first) -> 2 (fix 2, applied after). `--diff` after fix 2
  shows: `target 200 insns · ours 200 insns · 22 hunk(s) — 0 source-level ·
  0 operand-only · 22 not-scored`.

- [s3] A THIRD, independent register-allocation-shaped diff surfaced between
  fix 1 and fix 2 (score 11, hunk 16 of that intermediate diff): the shared
  field-increment `D_800A34F8 = (f2 & ~0x1C00) | (((f2>>10)&7)+1)<<10;` was
  written as `u32 f2=D_800A34F8; s32 c2=((f2>>10)&7)+1; D_800A34F8=(f2&~0x1C00u)|((c2&7)<<10);`
  — computing the SHIFT/FIELD part (c2) before the MASK part. Target's asm
  computes the mask AND first (`li v1,-7169; and v1,v0,v1`) THEN the shift
  (`srl v0,v0,0xa; ...`), reusing v0 for the field after v1 already holds the
  masked base — exactly mirroring case 1/2's OWN increment/decrement style
  (`u32 m = a0 & ~0x1C00; s32 c = ((a0>>10)&7)+1; m |= (c&7)<<10;`). Rewriting
  the shared block to compute the mask FIRST (`u32 m2 = f2 & ~0x1C00u;` before
  `s32 c2 = ...`) closed this without any register-pin or reordering trick —
  ordinary C statement-order match. Measured: 11 -> 2.

- [s3] THE ONLY REMAINING RESIDUAL (score 2, 0 hunks flagged as source-level
  or operand-only by `sandbox --diff`) is a scorer-visible but NOT
  source-level artifact: `tmp/grind/func_8006B578/diag_score.py` (normalized
  instruction dump via `engine.score.normalized_insns`) isolates it to
  exactly 2 instructions — the switch's jump-table address load:
  ```
  ours:   lui at,@.rodata      /  lw v0,@.rodata(at)
  target: lui at,0x0           /  lw v0,0(at)
  ```
  Root cause (confirmed via `nm`, not guessed): `jtbl_80015988` — the literal
  6-entry case-label address table this switch's jump table must match — is
  declared as a REAL C array (`const u32 jtbl_80015988[6] = {0x8006B6B8, ...};`)
  in a DIFFERENT translation unit, `src/text1a_b_pre_rodata.c:409-416`, and
  compiles to a GLOBAL, DEFINED symbol there (`nm build/src/text1a_b_pre_rodata.o`
  -> `00000598 R jtbl_80015988`; `nm build/src/text1b.o` -> `U jtbl_80015988`,
  i.e. text1b.o references it as UNDEFINED/external). Every one of the 6
  table entries is a hardcoded address that is a case label OF THIS FUNCTION
  and nothing else (`grep`-confirmed: no other function/file references
  jtbl_80015988 or any of the 6 addresses) — single-owner evidence this table
  belongs with func_8006B578's own TU, not with text1a_b_pre_rodata.c's grab-bag
  of leftover pre-cleanup rodata literals. A normal `switch` statement in C
  can NEVER make GCC reference a foreign externally-declared symbol for its
  jump table — GCC always synthesizes its OWN local jump table into the
  compiling TU's `.rodata`, referenced via a section-relative relocation
  (masked to `@.rodata` by the sandbox scorer, by design, since it's normally
  layout noise). The reference build (`build/src/text1b.o`, still built from
  the ORIGINAL hand-written-for-this-address asm, not from any C) references
  the symbol `jtbl_80015988` BY NAME (an `R_MIPS_HI16`/`R_MIPS_LO16` pair
  against that named external symbol) — which is why the diff shows target's
  raw immediate as literal `0x0` (unresolved: `jtbl_80015988` isn't listed in
  `named_syms.txt`/`undefined_syms_auto.txt`, so `engine/score.py`'s
  `_resolve_named_pair` can't look up its address and leaves the immediate
  raw) instead of a normalized `@hi(...)`/`@lo(...)` token. **This is the
  `jtbl-rodata-split-infrastructure` carve-out shape** (`.claude/rules/no-new-park-categories.md`
  / `codegen-technique-index.md` "jtbl-infra"): "the GCC-emitted jtbl can't be
  at the address the function references because rodata link order puts the
  asm/data block before the C file's .rodata. There is literally no pure-C
  form that resolves this without a project-wide rodata reorder." The
  no-new-park-categories rule's own evidence-based-reattribution carve-out
  says this is fixable, legitimately, by MOVING the `jtbl_80015988` array
  declaration out of `src/text1a_b_pre_rodata.c` and into `src/text1b.c`
  (deleting it from the pre-rodata grab-bag file, since single-owner evidence
  supports the TU reassignment) so GCC's own switch-generated table lands in
  THIS TU's `.rodata` at the correct link position — but that edit touches a
  SECOND file (`src/text1a_b_pre_rodata.c`), which is OUTSIDE this session's
  single-function/single-file mandate ("Work ONLY func_8006B578 in
  src/text1b.c"). This is the sole live frontier item for the next session
  (which should either get authorization to touch both files in one atomic
  edit, or file a `ruling-request` asking whether the cross-file move is
  in-scope for a single-function grind session).

- [s3] candidate.c (session 3) reaches sandbox --disable all score 2, 200/200 insns, 0 source-level hunks and 0 operand-only hunks in sandbox --diff — the entire prior floor-34 residual (7 source-level hunks) is closed.

- [s3] The only 2 differing normalized instructions (per tmp/grind/func_8006B578/diag_score.py) are the switch's jump-table lui/lw address load, masked to @.rodata in our build vs an unresolved literal 0x0 in the reference build.

- [s3] jtbl_80015988 (the 6-entry case-label address table this switch's jump table must match) is defined as a real C array in src/text1a_b_pre_rodata.c:409-416, a DIFFERENT translation unit than func_8006B578's home src/text1b.c; nm confirms build/src/text1a_b_pre_rodata.o defines it (00000598 R jtbl_80015988) while build/src/text1b.o references it as undefined external (U jtbl_80015988).

- [s3] Every one of jtbl_80015988's 6 entries (0x8006B6B8, 0x8006B6E8, 0x8006B720, 0x8006B7B4, 0x8006B7FC, 0x8006B828) is a case-label address belonging exclusively to func_8006B578 — grep across src/*.c and asm/funcs/*.s found no other reference to the symbol or any of its 6 literal values, i.e. single-owner evidence supporting a TU reassignment per no-new-park-categories.md's evidence-based-reattribution carve-out.

- [s3] src/text1b.c has been reverted to its pre-session INCLUDE_ASM stub (git status --short src/text1b.c is empty) — no C landed on main this session, consistent with asm-until-matched (C lands only once, at COMPLETED-C, and this function is not yet a byte match).

- [s3] No FAKE/cheat construct was used, proposed, or is present anywhere in this session's candidate — every construct is ordinary C with a real semantic reading (see self_vet.md T1-T6).

## Session 4 (permuter modality) — re-confirmed H7, no permuter surface, floor unchanged at 2

- [s4] Re-applied candidate.c to src/text1b.c at session start (it had reverted
  to the INCLUDE_ASM stub between sessions, as expected under asm-until-matched);
  sandbox --disable all reproduced score 2 exactly, no drift.

- [s4] sandbox --disable all --diff: 22 hunks, 0 source-level, 0 operand-only,
  22 not-scored. There is no C-mutation surface anywhere in the function body
  for a permuter (or any hand-written variant sweep) to act on.

- [s4] tmp/grind/func_8006B578/s4/diag_masked.py (direct call into
  engine.score.normalized_insns(mask=True) on build/src/text1b.o vs
  tmp/sandbox/func_8006B578/text1b.o) independently reproduced H7's finding:
  the ENTIRE masked diff is 2 instructions — `lui at,@.rodata`/`lw
  v0,@.rodata(at)` (ours, section-relative reloc against our own
  GCC-synthesized local jump table) vs `lui at,0x0`/`lw v0,0(at)` (target, an
  unresolved named-symbol reloc against jtbl_80015988, which is not present in
  named_syms.txt/undefined_syms_auto.txt/undefined_funcs_auto.txt).

- [s4] Re-read engine/score.py's `_resolve_named_pair` and
  `_mask_section_addend` this session: a section-relative HI16/LO16 (ours) and
  a named-symbol HI16/LO16 (target's) are handled by two DIFFERENT masking
  code paths that never converge to the same token, regardless of whether
  jtbl_80015988 is added to LD_SYM_FILES. Adding it would only let the
  TARGET side resolve to `@hi(addr)`/`@lo(addr)`; OUR side would remain
  `@.rodata` — still a mismatch. Ruling out "just add it to named_syms.txt" as
  a one-file fix.

- [s4] No permuter campaign was launched this session — 0 source-level/0
  operand-only hunks means there is nothing in the function body a mutation
  search could change to close this residual; running one would be a
  mechanically wasted session (see H8, hypotheses.md).

- [s4] src/text1b.c reverted to its pre-session INCLUDE_ASM stub before
  session end (git status --short src/text1b.c empty) — no C landed on main,
  consistent with asm-until-matched.

- [s4] No FAKE/cheat construct was used, proposed, or is present anywhere this
  session.

- [s4] Chassis check: reapplying memory/grind/func_8006B578/candidate.c to src/text1b.c reproduces sandbox --disable all score 2 exactly (no drift from session 3).

- [s4] sandbox --disable all --diff on the reapplied candidate: 200/200 insns, 22 hunks, 0 source-level, 0 operand-only, 22 not-scored (all masked branch/jump-target cascade artifacts).

- [s4] tmp/grind/func_8006B578/s4/diag_masked.py independently reproduces H7: the only 2 differing masked-normalized instructions are the switch's jump-table address load — ours 'lui at,@.rodata / lw v0,@.rodata(at)' (section-relative reloc against our own GCC-synthesized local jump table) vs target 'lui at,0x0 / lw v0,0(at)' (unresolved named-symbol reloc against jtbl_80015988).

- [s4] Re-reading engine/score.py's _resolve_named_pair + _mask_section_addend confirms a section-relative reloc and a named-symbol reloc are masked via two different code paths that never produce an equal token, so adding jtbl_80015988 to named_syms.txt (LD_SYM_FILES) would only resolve the TARGET side to an @hi/@lo(addr) token while OUR side stays @.rodata — still a mismatch, not a one-file fix.

- [s4] jtbl_80015988 (const u32[6], the case-label address table) is declared and defined in src/text1a_b_pre_rodata.c:409-416, a different TU than func_8006B578's home src/text1b.c; nm confirms build/src/text1a_b_pre_rodata.o defines it while build/src/text1b.o references it as an undefined external — this cross-TU split is the entire remaining residual (H7, session 3), unchanged this session.

- [s4] src/text1b.c reverted to its pre-session INCLUDE_ASM stub before session end; git status --short shows only memory/grind/func_8006B578/{evidence,hypotheses}.md changed by this session (plus the pre-existing metrics/events.jsonl churn) — no C landed on main.

- [s5] Re-applied memory/grind/func_8006B578/candidate.c to src/text1b.c at session start (chassis was INCLUDE_ASM stub, per the ledger's STALE HEAD CLAIMS warning) and re-measured: sandbox --disable all --diff reproduces score 2, 200/200 insns, 22/22 hunks not-scored, 0 source-level, 0 operand-only — identical class breakdown to s3/s4.

- [s5] All 22 not-scored hunks are branch/jump target address mismatches (e.g. target 'beq v1,v0,23718' vs ours 'beq v1,v0,9484') — consistent with the banked H7/H8 finding that the residual is the cross-TU jtbl_80015988 address-resolution artifact, not a codegen difference.

- [s5] Three structural levers tried this session (declaration order, type narrowing, statement re-association) produced either no change or a measured regression, corroborating H8's class kill that no in-file C structure change can reach the jtbl_80015988 residual.

- [s5] src/text1b.c was left byte-identical to its pre-session state (verified via git diff --stat showing 0 changes) after reverting the candidate application back to the INCLUDE_ASM stub.

- [s6, enumerate] Re-applied candidate.c, re-measured: sandbox --disable all --diff reproduces score 2, 200/200 insns, 22/22 hunks not-scored, 0 source-level, 0 operand-only — identical class breakdown to s3/s4/s5. No new drift.
- [s6] Directly (not from prior-session text) confirmed the cross-TU mechanism via fresh binary inspection: `mipsel-linux-gnu-nm build/src/text1a_b_pre_rodata.o` shows `00000598 R jtbl_80015988` (defined); `mipsel-linux-gnu-nm build/src/text1b.o` shows `U jtbl_80015988` (undefined external — this is the target-side INCLUDE_ASM build, not our candidate). `grep -n jtbl_80015988 src/text1b.c` returns zero hits — no C in the file can name the symbol.
- [s6] `mipsel-linux-gnu-objdump -dr tmp/sandbox/func_8006B578/text1b.o` on OUR compiled candidate shows the second switch's dispatch load as `lui at,0x0`/R_MIPS_HI16 `.rodata` + `lw v0,0(at)`/R_MIPS_LO16 `.rodata` (GCC's own fresh local table). `grep -n jtbl_80015988 asm/funcs/func_8006B578.s` shows target's dispatch as `lui $at,%hi(jtbl_80015988)` / `lw $v0,%lo(jtbl_80015988)($at)` (named-symbol reloc against the table at 0x80015988, a genuinely separate pre-existing object, not merely "adjacent").
- [s6] tools/gcc-2.7.2/stmt.c:4704 (`table_label = gen_label_rtx ();`, inside `expand_end_case` at stmt.c:4681) — GCC's switch/case-table emission always allocates a FRESH internal label via `gen_label_rtx()`; there is no code path that takes a name from the C source. This is the mechanism backing H9's original class-kill claim, now re-derived independently this session rather than cited from memory.
- [s6] The systematic-spelling-sweep protocol (mandated modality this session) requires an identified source-level or operand-only hunk to define the ENUM-BEGIN/END region; `sandbox --diff` shows 0 of either, so no region exists to wrap or sweep — spelling_enum/sweep_variants were not run because there is nothing for them to search (running them would just re-measure score 2 on an unchanged switch, wasting a campaign).
- [s6] src/text1b.c reverted to the INCLUDE_ASM stub before session end; git status --short confirms only memory/grind/func_8006B578/{evidence,hypotheses}.md changed (plus pre-existing metrics/events.jsonl churn) — no C landed on main.
- [s6, correction] The immediately-preceding [s6] bullet block ("spelling_enum/sweep_variants were not run...") was written by a PRIOR same-session attempt that was discarded by the driver validator (missing a numeric-measurement hypothesis) before this continuation. That claim is SUPERSEDED: this continuation DID mark a sub-block (candidate.c case-0 bit-toggle sequence, lines 64-71) with ENUM-BEGIN/END and ran `tools/spelling_enum.py` (10 variants; 4 syntactically valid: v00 identical, v01 decl-order swap, v02/v03 inlined-CSE forms; 6 invalid — the tool's compound-assignment axis emits non-compilable lvalues like `(f & ~1u) |= bit;` for this shape). The two non-trivial valid forms were measured directly against `sandbox --disable all`: v02 (inline `f`, drop the shared local) scores 2/200 insns — IDENTICAL to baseline; v01 (declare `bit` before `a3`) scores 11/203 insns — a real regression. See hypotheses.md H10 for the full record. The underlying conclusion is unchanged (0 source-level/operand-only hunks remain; the jtbl_80015988 cross-TU residual is the only live lever) but this session DID produce a numeric spelling-sweep measurement, not merely a repeated class-kill citation.

- [s6] sandbox --disable all --diff on the re-applied candidate.c: 200/200 insns, 22 hunks, all 22 classed not-scored (masked branch/jump-target address artifacts), 0 source-level, 0 operand-only.

- [s6] tools/spelling_enum.py on the case-0 bit-toggle sub-block (3 named locals f/a3/bit, 0 assignments, 4 anchors) emits 10 distinct spellings; only 4 (v00 identical, v01 decl-order swap, v02/v03 inline-CSE) are syntactically valid C — the remaining 6 are non-compilable (the tool's compound-assignment axis emits invalid lvalues like '(f & ~1u) |= bit;' for this shape, a tool limitation on this region, not a spelling candidate).

- [s6] v02 (inlined f) measured score=2/200 insns, identical to baseline; v01 (bit before a3) measured score=11/203 insns, a regression — both direct sandbox measurements this session, artifacts in tmp/grind/func_8006B578/s6/.

- [s6] After both measurements the candidate was restored to its exact s3/s4/s5 form and re-verified at score=2/200 insns before reverting src/text1b.c to the INCLUDE_ASM stub; git status --short at session end shows only the ledger files + pre-existing metrics/events.jsonl churn changed.

- [s6] No source-level or operand-only hunks exist in the current diff (0/22 of each) — the enumerate modality's search precondition (a region with a real spelling-space residual) is not met by the function's current frontier; the one sub-block tested was chosen as the most structurally complex candidate region, not because diff evidence pointed at it specifically.

## Session 7 (synthesis modality) — floor 2 held; residual proven to be a rodata-PLACEMENT problem, not a codegen problem

- [s7] Chassis re-established: `src/text1b.c` carried the INCLUDE_ASM stub at session start
  (as always under asm-until-matched). Re-applied `memory/grind/func_8006B578/candidate.c`
  verbatim and re-measured `sandbox func_8006B578 --disable all` => **score 2, 200 target
  insns / 200 build insns**, and `--diff` => 22 hunks, **0 source-level · 0 operand-only ·
  22 not-scored** — identical to s3/s4/s5/s6. No drift across four sessions.

- [s7] **NEW, DECISIVE — the GCC-synthesized jump table is CONTENT-IDENTICAL to
  jtbl_80015988.** `objdump -sr -j .rodata tmp/sandbox/func_8006B578/text1b.o` on our
  compiled candidate shows a 24-byte `.rodata` section with six `R_MIPS_32 .text`
  relocations and word contents `0x9554 0x9584 0x95bc 0x9650 0x9698 0x96c4`. The target
  table (`src/text1a_b_pre_rodata.c:409-416`, `jtbl_80015988`) is
  `0x8006B6B8 0x8006B6E8 0x8006B720 0x8006B7B4 0x8006B7FC 0x8006B828`. Entry-by-entry the
  difference is the SAME constant **0x80062164** for all six (6B6B8-9554 = 6B6E8-9584 =
  6B720-95BC = 6B7B4-9650 = 6B7FC-9698 = 6B828-96C4 = 0x80062164) — i.e. exactly the
  section-base delta between our object's `.text` and the linked address of
  `func_8006B578`. Our switch therefore emits the target's jump table byte-for-byte; the
  ONLY thing wrong with it is WHICH translation unit's `.rodata` it lands in.

- [s7] **NEW — every one of the 22 not-scored hunks is one single uniform object-offset
  shift.** A script over the captured `--diff`
  (`tmp/grind/func_8006B578/s7/diff_baseline.txt`) extracted the trailing address operand of
  every `target`/`ours` hunk pair: all 22 deltas are **0x1A294**, with zero non-address
  hunks. That is the offset difference between where `func_8006B578` sits inside the
  reference `build/src/text1b.o` (built from the whole-file asm) and where it sits inside our
  sandbox object. There is no branch-sense, no scheduling, and no allocation divergence
  anywhere in the body. Combined with the previous bullet, the compiled body of
  func_8006B578 is byte-equivalent to the target modulo (a) object placement and (b) the
  jump-table relocation SYMBOL (`%hi(jtbl_80015988)` vs `%hi(.rodata)`).

- [s7] **NEW — in-project precedent for the exact infrastructure fix this function needs.**
  `func_80077B30` (`src/text1b_b.c:825`) is an already-COMPLETED-C function in this same
  rodata cluster whose outer `switch (D_800A35E4)` has six cases (0..5) and therefore emits a
  24-byte GCC jump table. `bb2.ld` gives it a dedicated slot: line 59
  `build/src/text1a_b_pre_rodata.o(.rodata);`, line 60
  `build/src/text1b_b.o(.rodata);`, line 61
  `build/src/text1a_b_post_rodata.o(.rodata);`. The last symbol in the pre file is
  `jtbl_80015A24` (6 words @ 0x80015A24, ending 0x80015A3C); the first symbol in the post
  file is `jtbl_80015A54` (@ 0x80015A54). The 0x18 = 24-byte hole between them is EXACTLY
  where func_80077B30's compiler-generated table lands. So the rodata-cleanup project already
  established, and the linker script already implements, the "split the extracted grab-bag
  rodata sub-TU in two and give the real owning C file its own `.rodata` slot in the middle"
  pattern. func_8006B578 needs the identical treatment one cluster earlier, at 0x80015988.

- [s7] `build/src/text1b.o(.rodata)` is currently at `bb2.ld:66` — between `sound.o` and
  `gpu.o`, i.e. ~0x1000 bytes past the cluster — and `src/text1b.c` today declares no
  `const`/string-literal data at all (`grep -n "^const\|^static const" src/text1b.c` => no
  hits), so that slot currently contributes ZERO bytes. Compiling func_8006B578 as C adds 24
  bytes there, which both (i) puts the table at the wrong address and (ii) shifts every
  later `.rodata` input and the whole of `.text`, so a full-build SHA1 cannot match while
  that slot stays at line 66. This is why the honest floor sits at 2 and why no in-file C
  change can move it.

- [s7] The rodata that genuinely belongs to `src/text1b.c` looks like a contiguous run inside
  the pre-rodata grab-bag starting at `jtbl_80015988`: 0x80015988 (this function's table),
  `D_800159A0` ("warning\n", 16B), `jtbl_800159B0` (8 words -> 0x8006E5D8 etc.),
  `jtbl_800159D0` (15 words), `jtbl_80015A0C` (6 words -> 0x800748F0 etc. — note those
  targets are outside text1b.c's address range, so the run's END is not yet established).
  An operator implementing the fix should re-derive the exact boundary from the per-symbol
  address/owner inventory in `docs/rodata-cleanup-project.md` rather than trusting this
  sketch.

- [s7] KILL RE-AUDIT (mandated): `python3 tools/fake_ablate.py --func func_8006B578 --file
  text1b --candidate memory/grind/func_8006B578/candidate.c` reports "no FAKE-annotated
  constructs found ... nothing to ablate" — the candidate is and has always been FAKE-free,
  so every banked instance kill was measured with no FAKE carrier occupying any pseudo. The
  two closest-to-target instance kills were re-measured on the CURRENT chassis:
  s5's `hi` narrowed from `s32` to `s16` => **score 2 / 200 insns** (unchanged, kill holds as
  score-neutral); s6's v02 "inline `f`, drop the shared local" in the case-0 bit-toggle block
  => **score 2 / 200 insns** (unchanged, kill holds as score-neutral). Neither is an
  improvement and neither is a regression; both re-confirm the body is already at its
  in-file optimum.

- [s7] STALE-HEAD-CLAIM warning from the dispatch brief addressed: `candidate.c`'s header
  comment now carries an explicit migration banner stating that `src/text1b.c` on main holds
  the `INCLUDE_ASM` stub and that this file is the applied-during-measurement body, not HEAD
  state.

- [s7] `src/text1b.c` reverted to its pre-session INCLUDE_ASM stub before session end; no C
  landed on main. No FAKE/cheat construct was used, proposed, or is present anywhere.

## Session 7b (synthesis, 2026-09-16 — the re-run after the driver discarded the first s7 for returning `owner-gated` in a non-`escalation` modality)

**Chassis re-confirmation.** `memory/grind/func_8006B578/candidate.c` spliced into `src/text1b.c`
(replacing lines 6544-6548: the stale 4-param prototype + `INCLUDE_ASM`) and measured with
`sandbox func_8006B578 --disable all --diff`: **score 2, 200 target insns / 200 build insns, 22
hunks, 0 source-level · 0 operand-only · 22 not-scored**. Every one of the 22 hunks is a
branch/jump target pair differing by the constant object-vs-image delta (e.g. `beq v1,v0,23718`
vs `beq v1,v0,9484`). Artifact: `tmp/grind/func_8006B578/s7/diff_baseline_s7b.txt`.
`tools/fake_ablate.py --func func_8006B578 --file text1b --candidate .../candidate.c` →
"no FAKE-annotated constructs found; nothing to ablate", so the s5/s6 instance kills were all
measured on a chassis byte-identical to the current one with no FAKE carrier occupying any
pseudo — they stand as recorded and needed no re-measurement.

**The score-2 pin, now cited in the scorer's own source (H7/H14 hardened).**
`engine/buildconfig.py:93` defines `LD_SYM_FILES = ["undefined_funcs_auto.txt",
"undefined_syms_auto.txt", "named_syms.txt"]`, and `jtbl_80015988` appears in NONE of them
(it is a C `const u32[6]` in `src/text1a_b_pre_rodata.c:409`, so no splat symbol file lists it).
`engine/score.py:62` — "Named-symbol HI16/LO16 are NOT masked"; `engine/score.py:71` masks only
`R_MIPS_HI16`/`R_MIPS_LO16` **against section symbols**, rewriting the field to `@.rodata`
(`score.py:140`). The reference object's dispatch pair carries a NAMED reloc against
`jtbl_80015988` (unresolvable, so its immediate is compared raw); ours carries a
SECTION-relative reloc, masked to `@.rodata`. Two different code paths, no possible equal
token → the sandbox gradient for this function is pinned at exactly 2 and the full-build oracle
is the only instrument that can certify it. This is the known
`score-symtab-blind-to-asm-data-dlabels` class, now confirmed for a symbol that lives in C
rodata rather than an `asm/data` dlabel.

**NEW: the rodata ownership inventory the previous session left unverified is now measured.**
Method: for every rodata symbol in `src/text1a_b_pre_rodata.c` with an address in
0x80015900-0x80015B00, take its first table entries, resolve each entry address to the
enclosing function by grepping `asm/funcs/*.s` for that instruction address, then find that
function's `INCLUDE_ASM` owner in `src/*.c`:

| rodata symbol | first target | enclosing function | owning TU |
|---|---|---|---|
| `jtbl_80015940` (0x80015940) | 0x80066168 | `func_80065800` | `src/text1b.c` |
| `jtbl_80015988` (0x80015988) | 0x8006B6B8 | **`func_8006B578`** | `src/text1b.c` |
| `D_800159A0` (0x800159A0, "warning\n") | — | — | (string, unattributed) |
| `jtbl_800159B0` (0x800159B0) | 0x8006E5D8 | `func_8006E534` | `src/text1b.c` |
| `jtbl_800159D0` (0x800159D0) | 0x8006EE74 | `func_8006ECF4` | `src/text1b.c` |
| `jtbl_80015A0C` (0x80015A0C) | 0x800748F0 | `func_800747D8` | `src/text1b.c` (`INCLUDE_ASM` at src/text1b.c:8122) |
| `jtbl_80015A24` (0x80015A24) | 0x80077438 | `func_80077374` | `src/text1b.c` |
| (0x80015A3C-0x80015A54 hole, filled by `bb2.ld:60` `text1b_b.o(.rodata)`) | 0x80077B30 | `func_80077B30` | `src/text1b_b.c` (COMPLETED-C) |

Two corrections to the previous session's sketch, which it flagged as unverified past
`jtbl_800159B0`:
1. `jtbl_80015A0C` is **not** outside text1b.c's range — `0x800748F0` sits inside
   `func_800747D8`, which `src/text1b.c:8122` owns. The whole run
   **0x80015940-0x80015A3C is text1b.c-owned rodata**, seven symbols, one of them a string.
2. The cluster begins one symbol EARLIER than the sketch assumed: `jtbl_80015940`
   (`func_80065800`, text1b.c) precedes our table.

`mipsel-linux-gnu-objdump -h build/src/text1b.o` shows a single `.text` section and **no
`.rodata` section at all** today, confirming the premise that the `bb2.ld:66`
`build/src/text1b.o(.rodata)` slot currently contributes zero bytes and can be moved without
disturbing any other placement.

**Consequence for the integration handoff (sharper than the previous entry).** The original
object model is `text1a_b_pre_rodata → text1b → text1b_b → text1a_b_post_rodata`, and
text1b.o's rodata run is 0x80015940-0x80015A3C. Because GCC emits switch tables in the order
their `switch` statements appear in the TU, decompiling text1b.c's functions to C **must**
proceed so that the emitted tables stay in ascending address order; today only
`func_8006B578`'s table exists, so the `text1b.o(.rodata)` slot must sit exactly at 0x80015988
(between `jtbl_80015940` and `D_800159A0`), and `jtbl_80015988` must be deleted. When
`func_80065800` later lands as C, the slot moves back to 0x80015940 and `jtbl_80015940` is
deleted too. Each additional text1b.c switch-carrying function shrinks the extracted remainder
until the whole run belongs to text1b.o and the pre/post split collapses.

- [s7] Chassis re-measured this session: memory/grind/func_8006B578/candidate.c spliced over src/text1b.c:6544-6548 gives sandbox --disable all score 2, 200 target insns / 200 build insns, 22 hunks, 0 source-level / 0 operand-only / 22 not-scored; every hunk is a branch or jump target differing by the constant object-vs-image delta.

- [s7] tools/fake_ablate.py --func func_8006B578 --file text1b --candidate memory/grind/func_8006B578/candidate.c: 'no FAKE-annotated constructs found; nothing to ablate' - the body is ordinary C and the banked instance kills are on-chassis.

- [s7] The score-2 pin is now cited in the scorer's own source: engine/buildconfig.py:93 (LD_SYM_FILES lists no file containing jtbl_80015988), engine/score.py:62 (named-symbol HI16/LO16 not masked), engine/score.py:71 and engine/score.py:140 (only section-relative HI16/LO16 are rewritten to @.rodata).

- [s7] Rodata ownership inventory (new, verified): 0x80015940 jtbl -> func_80065800, 0x80015988 jtbl -> func_8006B578, 0x800159A0 warning-string, 0x800159B0 jtbl -> func_8006E534, 0x800159D0 jtbl -> func_8006ECF4, 0x80015A0C jtbl -> func_800747D8, 0x80015A24 jtbl -> func_80077374 - every table in 0x80015940-0x80015A3C belongs to a src/text1b.c function.

- [s7] Correction to the s7 integration-handoff entry: jtbl_80015A0C's targets are INSIDE text1b.c (func_800747D8, INCLUDE_ASM at src/text1b.c:8122), and the text1b.c rodata run begins at jtbl_80015940, one symbol before this function's table.

- [s7] build/src/text1b.o currently has a single .text section and NO .rodata section (mipsel-linux-gnu-objdump -h), confirming the bb2.ld:66 text1b.o(.rodata) slot contributes zero bytes today.

- [s7] The extern-table computed-goto route is mechanically self-defeating: jump.c:185 seeds LABEL_NUSES from LABEL_PRESERVE_P, so blocks entered only by a computed goto with no address-taken label are deleted (measured: 95 insns instead of 200).

- [s7] Object-model order corroborated by an already-COMPLETED-C precedent: bb2.ld:60 inserts build/src/text1b_b.o(.rodata) between text1a_b_pre_rodata.o (bb2.ld:59) and text1a_b_post_rodata.o (bb2.ld:61) to fill the 0x18-byte hole at 0x80015A3C with func_80077B30's compiler-generated table.

## Session 8 (solver modality) — 2026-09-16

Floor re-confirmed at 2 with `memory/grind/func_8006B578/candidate.c` applied to
`src/text1b.c`: `sandbox func_8006B578 --disable all` → score 2, 200 target insns /
200 build insns; `--diff` → 22 hunks, **0 source-level · 0 operand-only · 22
not-scored** (tmp/grind/func_8006B578/s8/diff_baseline.txt). Chassis unchanged since
session 3. `src/text1b.c` was reverted to `INCLUDE_ASM("asm/funcs", func_8006B578);`
before session end, per asm-until-matched.

### 1. The solver axis is now CLOSED by a typed verdict (not by inference)

`python3 tools/ra_solver/inverse_compose.py classify text1b func_8006B578
 --target-object build/src/text1b.o --ours-object tmp/sandbox/func_8006B578/text1b.o`
(object mode — the asm-until-matched-safe path required for a zero-rule function)
returns:

    FIRST DIVERGENCE: PRE-RA
      next tool: no backend — the residual is upstream of every model

so `tools/ra_solver/inverse.py` (RA) and `tools/ra_solver/inverse_sched.py` /
`perturb.py` (scheduler) are both mechanically inapplicable here: they permute and
rename a FIXED instruction multiset, and the classifier reports the multisets as
disagreeing. Full output: `tmp/grind/func_8006B578/s8/classify.txt`. No future session
should spend a pass running the RA or scheduler backends against this function on this
chassis.

### 2. The PRE-RA label is a relocation-rendering artifact, NOT an RTL-shape difference

The classifier blanks registers but not relocation symbols. Its own "shapes present in
ONE stream only" list is two instructions:

    ours only  : lui #,@.rodata      target only: lui #,0x0
    ours only  : lw #,@.rodata(#)    target only: lw #,0(#)

Two independent measurements show these are the same instructions with the same
relocations against different symbols:

* **Instruction-stream delta** (`tmp/grind/func_8006B578/s8/delta.py`, output
  `delta.txt`): 200 vs 200 insns; 22 unmasked differences, every one a branch or jump
  target; the set of DISTINCT target deltas is exactly `{0x1a294}` — one constant
  object-position offset with no second delta anywhere. Masked-diff count is 2, at insn
  #74 (`lui at`) and #76 (`lw v0`). This is the first time the "all 22 differ by one
  constant" claim (s7 H13) has been verified programmatically over the whole stream
  rather than by sampling.
* **Relocation tables** (`mipsel-linux-gnu-objdump -r --section=.text`): the reference
  object carries `R_MIPS_HI16 jtbl_80015988` at 0x237d0 and `R_MIPS_LO16 jtbl_80015988`
  at 0x237d8; ours carries `R_MIPS_HI16 .rodata` at 0x953c and `R_MIPS_LO16 .rodata` at
  0x9544. Same two relocation TYPES, same two instruction slots, and the offsets differ
  by the same constant 0x1a294. Only the relocation's SYMBOL differs — external table
  symbol vs our own section — which is precisely the cross-TU rodata ownership residual
  characterized in H7/H13/H14.

Consequence: the `classify` output's `cse_merge` / `cse_split` lever menus are NOT
applicable to this function, and no front-end / CSE / combine lever sits behind the
PRE-RA label. The body's RTL is the target's RTL.

### 3. Mandated kill re-audit — both s5 "score-neutral" kills upgraded to byte-equivalent

The re-audit mandate exists because a kill measured under a stale chassis or with a FAKE
carrier occupying a pseudo is not a kill. Here there is no FAKE carrier at all
(`tools/fake_ablate.py --func func_8006B578 --file text1b --candidate
memory/grind/func_8006B578/candidate.c` → "no FAKE-annotated constructs found …
nothing to ablate"), but there IS a subtler hazard specific to this function: the
gradient is pinned at 2 by the reloc artifact and `engine/score.py` masks the branch
targets and the two dispatch operands, so a variant could in principle change a MASKED
operand and still read "score-neutral". Since this function will be certified by the
full-build SHA1 oracle rather than by the sandbox, the correct re-audit instrument is
the byte/delta comparison, not the score.

Both s5 neutral forms were rebuilt on the current chassis and measured both ways:

| variant | sandbox | build insns | distinct branch deltas | masked diffs |
|---|---|---|---|---|
| candidate.c (form of record) | 2 | 200 | {0x1a294} | #74 lui at, #76 lw v0 |
| v_s16hi (`s16 hi`) | 2 | 200 | {0x1a294} | #74 lui at, #76 lw v0 |
| v_declrev (locals reversed to var_s2, hi, ret, sp10, v) | 2 | 200 | {0x1a294} | #74 lui at, #76 lw v0 |

Both s5 kills SURVIVE and are strengthened: these are not merely score-neutral, they are
byte-equivalent — each reproduces the target byte-for-byte modulo the same single
constant position delta and the same two reloc-symbol hunks as candidate.c. Neither
respelling perturbs allocation, scheduling or frame layout (note `sp10` is address-taken
and passed to `func_800692C0`, so the declaration permutation did not move its frame
slot either). candidate.c stays the form of record because its provenance is documented;
an operator performing the integration handoff may treat the two respellings as
interchangeable. Sources: `tmp/grind/func_8006B578/s8/v_s16hi.c`, `v_declrev.c`;
measurements `delta_v_s16hi.txt`, `delta_v_declrev.txt`.

### 4. What this session does NOT change

The blocking surface is unchanged and is still the one described in the 2026-09-16
`docs/grind/decisions.md` integration-handoff entry: `jtbl_80015988` lives in
`src/text1a_b_pre_rodata.c:409-416` while `bb2.ld:66` places
`build/src/text1b.o(.rodata)` about 0x1000 bytes past 0x80015988. Session 8 adds no new
attack on that; it removes the RA and scheduler axes from the ledger's search space and
hardens the bytes-proven claim with whole-stream and relocation-table evidence. Note the
driver DISCARDED session 7 as invalid (an `owner-gated`/rotation disposition requires
`escalation` modality), so the terminal-sounding language in that decisions.md span
carries no standing; this session's outcome is `progress` and the item stays active.

- [s8] Floor re-confirmed at 2 this session with candidate.c applied: sandbox --disable all reports score 2, 200 target insns / 200 build insns; --diff reports 22 hunks, 0 source-level / 0 operand-only / 22 not-scored (tmp/grind/func_8006B578/s8/diff_baseline.txt). Chassis unchanged since session 3.

- [s8] Typed solver verdict: inverse_compose.py classify (object mode) returns FIRST DIVERGENCE PRE-RA with 'no backend - the residual is upstream of every model'. The RA and scheduler solver backends are mechanically inapplicable to this function on this chassis.

- [s8] The four 'shapes present in ONE stream only' the classifier reports are two instructions: ours `lui #,@.rodata` / `lw #,@.rodata(#)` vs target `lui #,0x0` / `lw #,0(#)`.

- [s8] objdump -r proves those two are the SAME instructions with the same relocation types: target has R_MIPS_HI16 jtbl_80015988 at 0x237d0 and R_MIPS_LO16 jtbl_80015988 at 0x237d8; ours has R_MIPS_HI16 .rodata at 0x953c and R_MIPS_LO16 .rodata at 0x9544. The offsets differ by the same 0x1a294 as every other hunk; only the relocation SYMBOL differs (external table symbol vs our own section).

- [s8] Whole-stream verification (new this session, previously only sampled): of 200 vs 200 instructions, 22 unmasked differences exist, every one a branch or jump target, and the set of DISTINCT target deltas is exactly {0x1a294} - one constant object-position offset, no second delta anywhere. Masked-diff count is exactly 2 (insn #74 lui at, insn #76 lw v0).

- [s8] tools/fake_ablate.py --func func_8006B578 --file text1b --candidate memory/grind/func_8006B578/candidate.c reports 'no FAKE-annotated constructs found ... nothing to ablate' - the body is ordinary C with no FAKE carrier on any pseudo, so every kill measured on this chassis is FAKE-free.

- [s8] Kill re-audit result: the s5 `s16 hi` narrowing and the s5 reversed-declaration-order form are BYTE-EQUIVALENT to candidate.c, not merely score-equal (both: score 2, 200 build insns, delta set {0x1a294}, masked diffs exactly #74/#76).

- [s8] The blocking surface is unchanged from the 2026-09-16 decisions.md integration-handoff entry: jtbl_80015988 lives in src/text1a_b_pre_rodata.c:409-416 while bb2.ld:66 places build/src/text1b.o(.rodata) roughly 0x1000 bytes past 0x80015988. Session 8 adds no new attack on that surface; it removes the RA and scheduler axes from the search space and hardens the bytes-proven claim.

- [s8] Session 7 was DISCARDED by the driver as invalid (an owner-gated/rotation disposition requires escalation modality, not synthesis), so the terminal-sounding language in that decisions.md span carries no standing; the item stays active and this session's outcome is progress.

- [s8] src/text1b.c was reverted to the INCLUDE_ASM stub before session end; git status --short shows nothing outside the session surface.

## Session 9 (forensics) — byte-certification of the integration handoff + pass attribution

Chassis re-measured: `candidate.c` applied to `src/text1b.c` -> `sandbox func_8006B578
--disable all` = **score 2, target_insns 200, build_insns 200**, `--diff` = 22 hunks all
classed `not-scored`. Unchanged since session 3.

**Why the driver's dispatch-time CHASSIS CHECK said "measurement unavailable".** Not a
tooling outage and not a chassis regression: `memory/grind/func_8006B578/candidate.c`
carried a stray cp1252 byte `0x97` (a lone em-dash, introduced by an earlier session's
Windows-side write) at offset 3192. `engine/inlineasm.py:393` reads the candidate-applied
`src/text1b.c` with `encoding="utf-8"`, so the moment the body is spliced in the sandbox
raises `UnicodeDecodeError: 'utf-8' codec can't decode byte 0x97`. The file is now
sanitized to pure ASCII apart from valid UTF-8 em-dashes, and the chassis measures 2
again. Any future session that writes ledger C from the Windows side must keep
`candidate.c` UTF-8-decodable or the driver's chassis probe silently fails.

### A. The emitted jump table is byte-identical to the target's extracted table
Artifact: `tmp/grind/func_8006B578/s9/rodata_certification.txt`.

`objdump -h/-s/-r` on the candidate's own object `tmp/sandbox/func_8006B578/text1b.o`:
  * `.rodata` size **0x18** (24 B = 6 words), alignment **2**3** (8 bytes).
  * six `R_MIPS_32` relocations against `.text`, addends
    `0x9554 0x9584 0x95BC 0x9650 0x9698 0x96C4`.
  * `objdump -t`: `func_8006B578` is at `.text+0x9414`; it links at `0x8006B578` on main,
    so `text1b.o(.text)` base = **0x80062164**.
  * addend + base = `0x8006B6B8 0x8006B6E8 0x8006B720 0x8006B7B4 0x8006B7FC 0x8006B828`,
    which is *element-for-element* the extracted `const u32 jtbl_80015988[6]` at
    `src/text1a_b_pre_rodata.c:409-416`.
Session 7 asserted this content-identity from the C array; session 9 measures it from the
compiled object including relocation type, slot and addend. CONFIRMED.

### B. The rodata hole is exact and costs zero padding
`objdump -t/-h build/src/text1a_b_pre_rodata.o`: `.rodata` size `0x64C`, alignment `2**2`.
  * `jtbl_80015940` @ `.rodata+0x550` size `0x48` -> ends at `+0x598` (= `0x80015988`)
  * `jtbl_80015988` @ `.rodata+0x598` size `0x18` -> ends at `+0x5B0` (= `0x800159A0`)
  * `D_800159A0`    @ `.rodata+0x5B0` size `0x10`
Deleting `jtbl_80015988` leaves a hole of exactly 24 bytes at `0x80015988`. `0x80015988 %
8 == 0`, so our 8-byte-aligned `.rodata` lands with **zero** leading padding, ends exactly
at `0x800159A0`, and the post-split half of the TU (4-byte alignment) resumes there with
zero padding. The `bb2.ld` move is therefore byte-neutral for every symbol outside the
hole — the last unmeasured risk in the integration-handoff plan is now closed.
(The `2**3` alignment is not ours to choose: mips.h's case-vector alignment emits
`.align 3` ahead of the vector, visible in `dumps/text1b.s`.)

### C. Pass attribution: no optimization pass owns the residual
Artifact: `tmp/grind/func_8006B578/s9/pass_attribution.txt`; dumps in
`tmp/grind/func_8006B578/dumps/` (`pwsh tools/grinder/dump.ps1 func_8006B578`).
  * `dumps/text1b.rtl:51828` — `(jump_insn 474 473 475 (addr_vec:SI[ ... ]))`. The ADDR_VEC
    exists in the FIRST RTL dump, i.e. it is built by RTL generation
    (`stmt.c expand_end_case` -> the `casesi` expander), before any optimization pass runs.
  * It is unchanged through `.jump .cse .cse2 .loop .combine .flow .lreg .greg .sched
    .sched2 .jump2 .dbr`.
  * `final.c/final_scan_insn` emits `lw $2,.L988($2)` plus the `.rodata`/`.align 3`/
    `.word .L974..` vector; maspsx/ASPSX expands that single macro load into the
    `lui at,%hi / lw v0,%lo(at)` PAIR which IS the entire score-2 residual.
Consequence: there is no pass-input shape to enumerate for this function. Any C that still
compiles to a table dispatch yields the same local label and the same macro load (only the
label's final ADDRESS varies, and the linker script owns that); any C that does not compile
to a table dispatch loses the target's dispatch entirely (s7 H17: score 116 / 95 insns).

### D. Stale-chassis kill re-audit (mandated)
The s2 kills were measured on the superseded floor-34 body (two structural bugs later fixed
in s3), so they were the stale ones. Re-measured the closest of them on the CURRENT chassis:
inlining the shared `0x100010` tail-check + `return var_s2` into ALL THREE of cases 3/4/5
(removing every `goto tail` from them) now measures **score 15 at 200/200 insns**, versus
s2's 44 at 202/200. The kill stands — the form is still a regression — but its character
changed: on the current chassis it is a pure ordering/operand divergence, not an
instruction-count divergence. Banked at
`memory/grind/func_8006B578/rejected/all-three-cases-inline-tail-recheck-s2-kill-on-current-chassis-score15.c`.
`tools/fake_ablate.py` has nothing to ablate in either body (no FAKE constructs exist here).

- [s9] Chassis re-measured this session: candidate.c applied to src/text1b.c gives sandbox --disable all score 2, target_insns 200, build_insns 200, and --diff reports 22 hunks ALL classed not-scored (0 source-level, 0 operand-only).

- [s9] Our candidate's object emits .rodata of exactly 0x18 bytes (6 words) at alignment 2**3, with six R_MIPS_32 relocations against .text and addends 0x9554/0x9584/0x95BC/0x9650/0x9698/0x96C4.

- [s9] func_8006B578 is at .text+0x9414 in our object and at 0x8006B578 in the shipped EXE, so text1b.o(.text) links at base 0x80062164; the six table addends plus that base equal the extracted jtbl_80015988[6] words exactly (0x8006B6B8 0x8006B6E8 0x8006B720 0x8006B7B4 0x8006B7FC 0x8006B828).

- [s9] In build/src/text1a_b_pre_rodata.o the neighbouring symbols sit at .rodata+0x550 (jtbl_80015940, 0x48), +0x598 (jtbl_80015988, 0x18) and +0x5B0 (D_800159A0, 0x10), and that section's alignment is 2**2 - so deleting jtbl_80015988 leaves a 24-byte hole at 0x80015988 that our 8-aligned .rodata fills with zero padding on both sides.

- [s9] The 8-byte alignment is mips.h's case-vector alignment, not a choice of ours: dumps/text1b.s emits '.section .rodata / .align 3 / .align 2' ahead of the .L988 vector.

- [s9] dumps/text1b.rtl:51828 shows the ADDR_VEC as jump_insn 474 in the FIRST RTL dump, and it is unchanged through .jump .cse .cse2 .loop .combine .flow .lreg .greg .sched .sched2 .jump2 .dbr - so the jump table is created by stmt.c expand_end_case at RTL generation and no optimization pass touches it.

- [s9] final.c emits the dispatch as the single macro load 'lw $2,.L988($2)'; the lui/lw pair the scorer counts is maspsx/ASPSX's expansion of that one instruction, which is why the masked residual is exactly 2.

- [s9] Kill re-audit on the current chassis: the session-2 'inline the shared tail into all three of cases 3/4/5' variant now measures 15 at 200/200 insns (s2 recorded 44 at 202/200). The kill stands; its instruction-count component is gone.

- [s9] Tooling: the ledger's candidate.c carried a stray cp1252 0x97 byte that made engine/inlineasm.py's utf-8 read of the candidate-applied src/text1b.c throw, which is the cause of the dispatch-time 'measurement unavailable' chassis reading. Sanitized this session.
