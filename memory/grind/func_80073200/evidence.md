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
