# Evidence bank — func_80073200

## s1 (recon)

OBJECT MODEL: D_800A374C MATCHES (measured indirectly via compile — sandbox
score 115, no distance attributable to this symbol). D_800A374C is declared
`extern s32 D_800A374C;` at every other use site in src/text1b.c (e.g.
func_8005D46C-adjacent block line 3295, func_80060768 block line 3618) and is
consistently used as `D_800A374C + <byte-offset-expr>` passed as the raw s32
first argument to `AddPrim(s32, s32)`. asm/funcs/func_80073200.s reads it 4x
via `lui %hi(D_800A374C); lw %lo(D_800A374C)` then adds a byte offset
(0x70 literal once, `(val<<2)` three times) before the AddPrim call — byte-
identical convention to the established text1b.c declaration. No divergent
type/shape signal; the existing `extern s32 D_800A374C;` declaration is
correct for this TU and needs no fix. This satisfies the DATA MODEL mandate
for the func_80073200 recon session (the digest's only flagged global).

- Canonical gate: `verdict C`, `hand_coded_tier LOW` — ordinary pure-C
  target, no ASM-region/ASM-structural signal (canonical func_80073200 output).
- No sibling/duplicate leads found for func_80073200 in
  `tmp/duplicates_leads.txt`.
- func_80073200 was still `INCLUDE_ASM` (no prior C body) — this is the
  first session to write any C for it. m2c reference generated:
  `tmp/authoring/func_80073200.m2c.c` (via `tmp/run_m2c.sh func_80073200`).
- Callee `func_80073728(s32, s32)` — declared 2-arg (address, index) at every
  OTHER call site in text1b.c (func_8005D46C, func_8005FA98, func_800600C8,
  func_8005E098-adjacent, func_80060650-adjacent), always called with a
  constant `0` second arg elsewhere. func_80073200's asm uniquely varies the
  second arg 0/1/2/3 across a 4-iteration unrolled sequence (m2c agrees:
  `func_80073728(&sp18, 0)`, `(&sp18, 1)`, `(&sp18, 2)`, `(&sp18, 3)`) — the
  m2c-suggested 3rd arg is spurious (a2 register is stale from an earlier
  zeroing before SetDrawMode, never actually passed); kept the 2-arg
  signature already established codebase-wide.
- func_80073200's local "prim struct" at sp+0x18 matches the SAME field
  layout as `S46C` (src/text1b.c ~3182: p0@0, p1@4, pad08@8, ret@0xC,
  zero10@0x10, one14@0x14, zero18@0x18, zero1C@0x1C, c20@0x20, c24@0x24,
  byte28@0x28) used by func_8005D46C/func_8005FA98 as the `func_80073728`
  argument struct — offsets 0x0/0x4/0xC exactly match our target's sp18/
  sp1C/sp24 writes before each func_80073728 call. Did NOT adopt the S46C
  typedef literally this session (wrote raw sp-local scalars instead,
  matching m2c's per-field decomposition) — next session should consider
  switching to the named S46C struct if it helps close remaining hunks,
  but the field VALUES/order used are consistent with it either way.
- `ctx = *(s32 **)(D_800A35A8 + 0x5C)` — a per-context struct pointer at
  fixed offset 0x5C from the global `D_800A35A8` "current" pointer; offset
  0x5C is read elsewhere only by other still-INCLUDE_ASM functions
  (func_8006F528 asm/funcs) so no established field name is available yet.
- Draft first-cut C landed in src/text1b.c (see WIP note below) drops the
  sandbox score 203 -> **115** (target_insns 203, build_insns 131,
  cheat_asm_stripped 134 baseline unaffected).
- `sandbox --diff` (34 hunks) shows the residual is DOMINATED by
  source-level hunks (only 4 of 34 are operand-only: hunks 10/12/14/20/28/30
  — register-seat only; everything else needs a real C-structure fix).
  One hunk (5) is not-scored (masked branch-target diff) — do not chase.
- Concrete structural finding from the diff: target's frame is 96 bytes
  using FOUR callee-saved regs (s0-s3, `.frame vars=88`ish); our build only
  uses 48 bytes / two callee-saved regs (s0,s1) — we are computing far less
  redundantly than the target, meaning several of our simplifications
  (folding a compile-time-constant offset into `addiu` instead of a
  `lw`-from-stack-then-add, or reusing one local rather than writing a
  second `sw` for a byte field) are COLLAPSING structure the target keeps
  separate. Two concrete instances identified (see hypotheses.md H1/H2):
  (a) the `sp41/sp42/sp43` color-byte if/else — m2c stages the shared
  `sp42`-equivalent write OUTSIDE the if/else via a temp (`var_v0`), we
  wrote it duplicated INSIDE both arms — target keeps a single post-if
  store (hunk 3/4 region: target's un-diffed store to `sb 66(sp)` sits
  after the branch merge, ours duplicates the whole triplet inside each arm);
  (b) `s1 = base2 + 0xC` — target computes this UNCONDITIONALLY right after
  loading `base2` (before testing `D_800A3580 < 4`), we compute it INSIDE
  the `if` block only — asm confirms (`addiu s1,v1,0xC` sits before the
  `beqz` branch in asm/funcs/func_80073200.s:37-40).

- [s1] OBJECT MODEL: D_800A374C MATCHES — extern s32 D_800A374C; is the correct declaration for this TU, consistent with every other text1b.c use site (D_800A374C + byte-offset-expr passed as raw s32 to AddPrim(s32,s32)); no declaration fix needed.

- [s1] canonical func_80073200: verdict C, hand_coded_tier LOW — ordinary pure-C target, no ASM-region/ASM-structural routing signal.

- [s1] func_80073200 had zero prior C (still whole-body INCLUDE_ASM at session start) — this session wrote the first-ever draft body.

- [s1] m2c reference generated at tmp/authoring/func_80073200.m2c.c; its guessed 3rd arg to func_80073728 is spurious (stale a2 register, not an actual passed value) — kept the codebase-wide-established 2-arg func_80073728(s32,s32) signature.

- [s1] Draft C compiles clean and drops sandbox score 203 -> 115 (target_insns 203, build_insns 131).

- [s1] sandbox --disable all --diff: 34 hunks, 1 not-scored (masked branch-target, hunk 5, correctly not chased), ~6 operand-only (real register-seat diffs, hunks 10/12/14/20/28/30), the rest (~27) source-level — the remaining gap is C structure, not a register-allocation/scheduling lever.

- [s1] Target frame is 96 bytes / 4 callee-saved regs (s0-s3); our build is 48 bytes / 2 callee-saved regs (s0,s1) — our C is collapsing structure (redundant loads/stores, compile-time-folded offsets) the target keeps separate as live stack state.

- [s1] Concrete fold identified: our C's `AddPrim(D_800A374C + (sp2C * 4), ...)` with sp2C set to the literal 0x14 just above gets constant-folded by GCC into `addiu a0,a0,80`; target reloads sp2C from the stack at that point (`lw a0,44(sp)`) and computes the shift/add at runtime — evidence the original source kept more live/non-foldable structure between the assignment and the use (repeats identically at the two other AddPrim call sites in the function, hunks 15-18 and 31-33).

## s2 (structural)

- [s2] SIBLING CHECK: func_8007352C/func_800600C8/func_80060768 (named in this
  session's sibling-ledger block) have NO memory/grind/<func>/ directory —
  their ledgers record "candidate: src/text1b.c (the MATCHED body, on main)",
  i.e. their matched bodies ARE the committed src/text1b.c text with nothing
  external to transplant. The genuinely useful transplant targets this
  session were the OTHER address-taken-aggregate callers of
  func_80073728/func_8007352C already on main in the same TU:
  func_8005D46C/func_8005FA98 (S46C, src/text1b.c:3178-3190),
  func_80069AE4 (S_69AE4, :5960-5963), func_80069F80 (S_69F80, :6160-6164).

- [s2] ROOT CAUSE of the s1-diagnosed AddPrim compile-time fold (H3,
  CONFIRMED): GCC 2.7.2's alias/escape analysis treats address-of-local
  escape PER-C-VARIABLE, not per-stack-byte-range. The s1 draft declared
  sp18/1C/20/24/28/2C/30/34/38/3C/40/41/42/43 as 14 SEPARATE bare-scalar
  locals and only took `&sp18`; GCC therefore only treated `sp18` as
  address-exposed to the four intervening `func_80073728(&sp18, N)` calls,
  leaving `sp2C` (and friends) free to be constant-propagated across those
  calls even though the ORIGINAL target frame layout puts all 14 at
  contiguous offsets sp+0x18..sp+0x43 (a single 0x2C-byte object). Declaring
  one `S73200 s;` struct (field-for-field identical to the already-on-main
  `S_69AE4`/`S_69F80` structs used by func_80069AE4/func_80069F80 for the
  SAME two callees with the SAME `(s32)&s, 0`-style call convention) and
  replacing every `spNN` with `s.spNN` made the WHOLE aggregate
  address-exposed by the SAME `&s` used in the func_80073728 calls — GCC
  now reloads every field after a call instead of folding it, matching
  target exactly on that axis. Measured: sandbox score 115 -> 26
  (target_insns 203, build_insns 131 -> 203 — build_insns now EXACTLY
  target_insns), hunk count 34 -> 22, source-level hunks 27 -> 12. This is
  ordinary C (an address-taken local struct declaration — no FAKE
  construct, no annotation needed, same shape already on main 4x in this
  file) — not a new construct family.

- [s2] H2 (color-byte var_v0 staging) KILLED as an instance: staging the
  inner if/else's `sp42` write through a shared post-merge `var_v0` (matching
  m2c's shape) vs. duplicating `sp41=..;sp42=..;` in both arms compiles
  BYTE-IDENTICALLY on this chassis — the hunk-3/4 residual named in s1 is NOT
  attributable to that spelling choice; some other structural difference
  (likely subsumed by/related to the H3 struct fix, or a genuine
  register-seat tie) accounts for it. Superseded by the H3 struct rewrite
  regardless (that hunk region also closed/reclassified after H3).

- [s2] H4 (CONFIRMED, small): naming `tmp + 0xC` / `idx + 0xC` as a local
  `tbl` before the `s.sp1C = tbl;` store — matching the IDENTICAL
  `tbl = p1 + 0xC; s.sp1C = tbl;` shape already on main at
  src/text1b.c:6249-6250/6257-6258/6352-6353 for the SAME S_69AE4/S_69F80
  struct family — reclassified the two remaining source-level hunks at that
  location (target `addiu s1,v0,12; sw s1,28(sp)` vs our inlined `addiu
  v0,v0,12; sw v0,28(sp)`) to operand-only (register-seat: target keeps the
  value in `s1`, ours in `v1`). Score 26 -> 24.

- [s2] Block-scoping the `v1`/`idx` locals inside the `if (D_800A3580 < 2)`
  block (vs. function-scope, matching the s1 draft's original declaration
  style) measured score-INERT: 24 -> 24, no hunk-shape change. KILLED as an
  instance (see hypotheses.md); reverted to keep the committed candidate
  minimal.

- [s2] CURRENT RESIDUAL (24, target_insns == build_insns == 203): target uses
  4 callee-saved regs (s0-s3, frame 96 bytes) vs. our 2 (s0,s1, frame 88
  bytes) — target holds the repeated `0x12` literal (`s.sp2C = 0x12;`, set
  once before and once again inside the `D_800A3580 < 2` block) in a THIRD
  persistent register (`s3`) spanning BOTH func_8007352C call blocks, while
  our allocation only spans one block at a time (`s1`/`v1`). This is a
  genuine register-allocation/liveness question, not a source-level
  omission (insn counts are equal) — needs a `.greg`/`-da` dump next
  session before proposing a specific lever (see codegen-technique-index →
  register-alloc-pure-c). One remaining apparent 5-insn "insert" (hunk 16,
  the D_800A3580<2 re-test) is very likely a downstream scheduling artifact
  of this same register-seat gap, not an independent third lever — verify
  that first before treating it separately.

- [s2] Sibling ledgers func_8007352C/func_800600C8/func_80060768 have no memory/grind/<func>/ directory left; their note 'candidate: src/text1b.c (the MATCHED body, on main)' means nothing external needed transplanting — the genuinely useful transplant targets this session were the OTHER address-taken-aggregate callers of func_80073728/func_8007352C already on main in this TU: func_8005D46C/func_8005FA98 (S46C), func_80069AE4 (S_69AE4), func_80069F80 (S_69F80).

- [s2] S73200's field layout (sp18,sp1C,sp20,sp24,sp28,sp2C,sp30,sp34,sp38,sp3C,sp40,sp41,sp42,sp43) is IDENTICAL to the already-committed S_69AE4 (src/text1b.c:5960-5963) and the first 14 fields of S_69F80 (src/text1b.c:6160-6164) — both used elsewhere in the same TU with the identical func_80073728/func_8007352C callees and (s32)&s call convention. This is not a novel construct; it is the SAME address-taken-local-aggregate pattern already accepted 4x in this file.

- [s2] Confirmed root cause of the s1-diagnosed AddPrim compile-time-fold residual: GCC 2.7.2 alias/escape analysis is per-C-DECLARATION, so 14 separate bare-scalar locals only exposed the ONE addressed variable (sp18) to the func_80073728 calls, letting sibling scalars like sp2C fold across the calls; one aggregate whose address is taken exposes the whole object, forcing reloads that match target.

- [s2] sandbox --disable all --diff after the struct rewrite: target_insns == build_insns == 203 exactly (was 203 vs 131 before) — every remaining hunk is a REPLACE, not an insert/delete of unequal total length, confirming nothing is structurally missing or extra anymore.

- [s2] Remaining residual (24) is dominated by a register-seat/callee-save gap: target uses 4 callee-saved regs (s0-s3, 96-byte frame) vs ours 2 (s0,s1, 88-byte frame) — target keeps the repeated 0x12 literal live in a third persistent register (s3) spanning both func_8007352C call blocks; ours only spans one block at a time. This needs a .greg/-da dump, not yet root-caused this session.

- [s2] One apparent 5-insn insert (hunk 16, the D_800A3580<2 re-test) is very likely a downstream scheduling/alignment artifact of the register-seat gap above rather than an independent missing statement, since target_insns == build_insns overall.

- [s3] H5 CONFIRMED: naming the repeated `s.sp2C = 0x12;` literal (written
  once unconditionally, once inside `if (D_800A3580 < 2)`) as a single fresh
  `s32 v12 = 0x12;` local declared+initialized at the TOP of the function
  and read (never reassigned) at both sites closed the ENTIRE frame-
  size/callee-save residual named in the s2 frontier: score 24 -> 17,
  target's 96-byte-frame / 4-callee-saved-reg (s0-s3) shape now matches
  exactly (prologue `sw ra,88(sp); sw s3,84(sp)` / epilogue restore both
  reproduced). This confirms the s2 frontier's root-cause guess: target's
  compiler DOES keep this literal live in a persistent (callee-saved)
  register spanning both `func_8007352C` call blocks, and a fresh
  once-written local read twice reproduces that register-allocation
  decision in our fork too.

- [s3] Residual side-effect of H5: GCC also rematerializes `li s3,0x12` at
  the earliest legal point in the function's CFG (target_insns diff
  position 30, where target has a bare `nop`) IN ADDITION TO the correct
  later position — one extra instruction (build_insns 204 vs target 203).
  This is a genuine GCC-internals question (global/reload constant
  rematerialization for a globally-allocated pseudo with a REG_EQUIV
  constant note) not yet root-caused against `tools/gcc-2.7.2/{global,
  reload,reload1}.c` this session — next session should read the .greg
  dump for the v12 pseudo specifically
  (tmp/grind/func_80073200/dumps/text1b.greg, produced this session via
  `pwsh tools/grinder/dump.ps1 func_80073200`) before proposing a further
  lever.

- [s3] Three placement variants of the v12 initializing STATEMENT were
  measured: (a) top-of-body initializer (H5, confirmed, score 17); (b) bare
  assignment at the deep point-of-first-use (H5b, KILLED — reproduces the
  pre-H5 baseline exactly, score 24, no persistent register); (c) a
  still-early-but-not-absolute-top assignment right after
  `s1 = base2 + 0xC;` (H5d, KILLED — WORSE, score 22, build_insns 207). Only
  the absolute-top initializer form gets the persistent-register benefit
  without making things worse. Declaration ORDER among sibling locals
  (H5c, KILLED) has zero effect, confirming it's the STATEMENT's emission
  position (top-of-body initializer vs later assignment), not the C
  declaration-list ordering, that drives global.c's allocation decision.

- [s3] CURRENT RESIDUAL (17, build_insns 204 vs target_insns 203): (1) the
  phantom early `li s3,0x12` rematerialization described above (source of
  the +1 insn and its two downstream cascade `not-scored`/operand-only
  branch-offset hunks); (2) the pre-existing `s1`/`v1` register-seat tie at
  the `tbl+0xC` store (unchanged since s2, hunks matching target's
  `addiu s1,v0,12; sw s1,28(sp)` vs ours `addiu v1,v0,12; sw v1,28(sp)`,
  appearing at BOTH the s1/idx call-block sites); (3) the D_800A3580<2
  test's scheduling: target computes `lh v1,%gp_rel(D_800A3580); slti
  v1,v1,2; beqz v1,...` EARLY (interleaved into the delay slot after the
  preceding `lw v0,0x18($s0)`) using v1, keeping the value in v1 through to
  where it's stored later (`li v0,1; sw s3,44(sp)` at the branch target);
  ours computes the equivalent test LATER, using v0, with an extra
  load-delay nop. (2) and (3) are unchanged from the s2 floor-24 diff —
  this session's H5 lever closed the frame gap ONLY, cleanly separable
  from these two.

- [s3] sandbox --disable all --diff on the s2 floor-24 chassis showed 8 source-level / 10 operand-only / 4 not-scored hunks; the operand-only hunks at target-insn positions 0/3/22/195/200 were exactly the frame-size (sp,-96 vs -88) and callee-save (ra/s3) pair the s2 frontier named as the register-seat gap.

- [s3] Reading asm/funcs/func_80073200.s directly (lines 120-124 and 159) shows target computes `addiu $s3,$zero,0x12` exactly ONCE and reuses the same register value at the second `sw $s3,0x2C($sp)` store -- confirming the s2 frontier's guess that the original C held this repeated literal in one persistent variable.

- [s3] Introducing `s32 v12 = 0x12;` as a top-of-body initializer (read at both s.sp2C = v12 sites, never reassigned) is confirmed to reproduce target's persistent-s3-register allocation and closes the entire frame-size residual: score 24 -> 17.

- [s3] The remaining +1 build_insns (204 vs target's 203) is an extra `li s3,0x12` rematerialized at the earliest legal point in the function's CFG (diff position 30) where target has a bare `nop` -- isolated to this one construct; every other hunk from the floor-24 diff besides the frame-size ones is unchanged at floor 17.

- [s3] Three placement variants of the v12 assignment statement were measured (top initializer / deep point-of-use / mid-function-early) -- only the top-of-body initializer gets the persistent-register benefit; the other two are both worse or no-better, and declaration ORDER among sibling locals (independent of statement position) has zero effect.

- [s3] pwsh tools/grinder/dump.ps1 func_80073200 was run this session and produced tmp/grind/func_80073200/dumps/text1b.{greg,sched,combine,...} for the CURRENT (H5, floor-17) chassis -- not yet read in detail for the v12-specific rematerialization root cause; that is next session's first step.

- [s3] Sibling ledgers (func_80069F80, func_8007352C, main/src/ings.c, func_800600C8, func_80069AE4, func_80060768) are all COMPLETED-C with closed ledgers; their candidate.c files are just the matched body already on main in this same TU (already consulted/transplanted at s2 -- the S73200 struct shape IS the transplanted pattern from func_80069AE4/func_80069F80). No new transplant material from siblings this session.
