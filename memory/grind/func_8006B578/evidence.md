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
