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
