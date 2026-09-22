# Evidence bank — func_8002C22C (PutRobShadow, src/code6cac_b.c)

## s1 (recon, 2026-09-22)

- **Function identity**: kengo sibling name `am_rmd/PutRobShadow`, MED confidence,
  252 target instructions. Called from `func_8002C61C` (practice-mode dispatch,
  `mode == ...` chain) right after `func_800288C8()`, guarded by
  `D_800A3824 = func_80029454(); if (D_800A3824 < 0) goto do_calc;` — so
  `D_800A3824` is a signed return-code-turned-bitfield reused as a flag word in
  THIS function (bits 0 and 1 tested independently, reloaded fresh via `lhu` for
  the second test at asm line 122-125 — the original C re-reads the global rather
  than caching it in a local; our candidate mirrors that by testing
  `D_800A3824 & 1` and `D_800A3824 & 2` as two independent statements, not a
  cached local).
- **archived prior attempt**: `memory/grind/func_8002C22C/pre-include-asm-body.c`
  is a real prior decomp (Campaign 4 chassis-refresh casualty, never built) with
  49 volatile-coercion casts on scratchpad addresses — FORBIDDEN per
  `.claude/rules/mmio-volatile-type-level.md`, which explicitly EXCLUDES
  scratchpad RAM (0x1F800000-0x1F8003FF) from the MMIO type-level carve-out
  ("Explicitly NOT covered: scratchpad RAM ... volatile there is game-state
  coercion unless an IRQ writer is cited under the two-prong gate"). Its C
  SHAPE (control flow, field offsets, the two if/else selectors) is real
  archaeology and was reused for s1's candidate; its volatile casts were NOT.

## OBJECT MODEL (DATA MODEL signal — mandatory per brief)

OBJECT MODEL: D_801020D8 (flagged CENSUS-VS-DECL: census "+0x210 vec3 A position
X (record 0)" vs header `extern s32 D_801020D8;`) — MATCHES. Measured: s1
candidate (this ledger) uses the plain scalar `extern s32` decl as-is for
D_801020D8 and its five sibling record-0 fields (D_801020DC/E0/E4/E8/EC) and the
six record-0-B fields (D_801020FC/80102100/04/08/0C/10); `sandbox --disable all`
scores 211 with this decl unchanged, and `--diff` shows zero hunks attributable
to mis-resolution of these symbols (every divergent hunk is scheduling/hoist of
scratchpad loads around the branch, not symbol-address mismatch — see H3 below).
Static asm evidence independently confirms MATCHES: every one of these twelve
fields is loaded via its OWN `lui %hi(sym)/lw %lo(sym)` relocation pair in
`asm/funcs/func_8002C22C.s` (lines 44-63, 77-108) — never via a base register +
offset from a shared struct pointer — which is exactly what discrete scalar
`extern s32` declarations compile to. The census's "vec3 A / record 0" wording
is descriptive game-state semantics, not evidence of a missing C aggregate; no
`split-scalars-hide-aggregate` merge applies here (contrast D_80102314 below,
which DOES need base+offset addressing and is declared accordingly).

The brief additionally flagged `D_801020D8` as CENSUS-VS-DECL: census calls it "+0x210 vec3 A
position X (record 0)" while the header declares plain `extern s32 D_801020D8;`.
Checked against `asm/funcs/func_8002C22C.s` (the only ground truth available,
since the function had zero C before this session):

- **D_800A3824** (decl `extern s16`): asm uses `lhu` (unsigned half-word load) at
  both use sites (lines 3-4, 122-123 of the .s) — **MATCHES** the declared type.
  No sandbox measurement possible pre-candidate (no C existed); post-candidate
  (s1, this session) the whole function scores 211/252 with this decl used
  as-is, no divergence traced to this symbol in the diff (see below) — **MATCHES**.
- **D_801020D8, D_801020DC, D_801020E0, D_801020E4, D_801020E8, D_801020EC**
  (record-0 A-vec3 pos+delta) and **D_801020FC, D_80102100, D_80102104,
  D_80102108, D_8010210C, D_80102110** (record-0 B-vec3 pos+delta): asm loads
  EVERY ONE of these via its OWN `lui %hi(sym)/lw %lo(sym)` pair (asm.s lines
  44-63, 77-108) — never via a base register + offset from any shared struct
  pointer. This is direct static evidence the compiler emitted them as
  independent global symbols, which is exactly what the current scalar
  `extern s32` declarations already produce. **MATCHES** — the census's "vec3 A
  / record 0" framing is descriptive metadata about the game-state semantics,
  not evidence the C needs an aggregate/struct declaration here. No aggregate
  merge is warranted for these six-plus-six symbols (measured: s1 candidate
  using them as plain scalars reproduces this region of the asm essentially
  verbatim — hunks 2-9 of the `--diff` output are clean above the branch,
  the residual starts at the *scheduling* of loads before the branch, not at
  symbol resolution).
- **D_80102314** (census: "P2 of g_practice_menu_table (P1 = 0x80101EC8)"; no
  header decl at all): asm accesses this ONE via `lui %hi(D_80102314); addiu
  %lo(D_80102314)` into `$t1` (asm.s line 5-6) and THEN every field off it via
  `lw $aX, OFF($t1)` for OFF in {0x210,0x214,0x218,0x21C,0x220,0x224} (else arm)
  and {0x234,0x238,0x23C,0x240,0x244,0x248} (if arm) — genuine base+offset
  addressing, confirmed by `grep` finding ZERO named symbols at
  D_80102314+{0x210..0x248} in `undefined_syms_auto.txt`. Cross-checked against
  `func_8002C61C` (same file, `src/code6cac_b.c:920-923`): that function
  literally does `u8 *s1 = &D_80101EC8; u8 *s0 = s1 + 0x44C;` — and
  0x80101EC8 + 0x44C == 0x80102314 exactly. **CONCLUSION: D_80102314 IS
  record[1] of the same 2-element practice-menu-table array whose record[0]
  base is D_80101EC8** (stride 0x44C), corroborating the census's "P1/P2"
  framing. MATCHES the object model census asserts, once declared — this
  session declared it as `extern s32 D_80102314;` (scalar, address-of'd then
  offset-indexed, exactly mirroring how the asm materializes the address once
  and reuses it) rather than expressing it as `((RecordT*)&D_80101EC8)[1]`,
  because the asm's `lui/addiu D_80102314` is a DIRECT symbol reference, not
  arithmetic from D_80101EC8 — so a separate named symbol is the byte-accurate
  spelling, an aggregate `RecordT g_table[2]` would require the compiler to
  compute `&g_table[1]` as `&g_table[0] + 0x44C`, a DIFFERENT (and unobserved)
  addressing form. Declared `extern s32 D_80102314;` FILE-LOCAL in
  `src/code6cac_b.c` this session (NOT the header — `code6cac.h` is outside
  this function's `tools/grinder/scope_allow.txt` grant; promoting it needs a
  scope-grant session, see frontier).
- No other flagged symbol in the DATA MODEL block is touched by this function's
  body (the vec3-B fields at D_801020FC.. are read but not written; D_80102314
  is read via base+offset only, never written).

## s1 measurement (this session)

- Chassis check baseline: 252 (whole-body INCLUDE_ASM, no C).
- Candidate translation of the archived C shape, with EVERY scratchpad
  `volatile` cast stripped to a plain cast (hypothesis: scratchpad is not
  MMIO-range hardware and GCC 2.7.2's weak alias analysis on raw-address
  casts might reproduce the target's redundant same-address stores without
  needing volatile at all) + `D_80102314` declared and used with
  offset-indexed access exactly as the asm shows:
  **`sandbox --disable all` = 211** (target 252 insns, build 230 insns — down
  from the 252-insn no-C-body floor). Saved as
  `memory/grind/func_8002C22C/candidate.c` (and staged live in
  `src/code6cac_b.c` for the next session, which the scope check permits since
  the src file is this function's own).
- `--diff` on the s1 candidate: 33 hunks, 27 source-level / 6 operand-only / 0
  not-scored. The FIRST source-level hunks (1-5) show our build's cc1
  aggressively **hoisting/duplicating scratchpad loads out of the two if/else
  arms and materializing them BEFORE the branch** (e.g. our hunk 3 loads
  `D_801020FC`(offset 0x4C-equivalent)/`0x48`/`0x50`/`D_80102108` etc. ahead of
  the `beqz` that target still guards) — target keeps every scratchpad
  load/store strictly INSIDE its owning if/else arm, straight-line, in asm
  source order (no hoisting at all). This is consistent with GCC treating the
  plain (non-volatile) casts as safely reorderable across the branch once it
  proves no aliasing, which the ORIGINAL compiler's source apparently didn't
  let it do — i.e., volatile (or an equivalent scheduling barrier the source
  legitimately created) IS doing real work in the target, but scratchpad falls
  outside the MMIO carve-out and this project has no IRQ-writer evidence for a
  legitimate-volatile-interrupt-touched two-prong grant on these addresses.

- [s1] OBJECT MODEL: D_801020D8 (flagged CENSUS-VS-DECL) and its eleven record-0 siblings MATCH the existing plain extern s32 scalar declarations -- every field loads via its own lui/lo relocation pair in the asm, never base+offset from a shared pointer, so no aggregate merge is warranted.

- [s1] D_80102314 (not in header at all) is record[1] of the same 2-elem practice-menu table as D_80101EC8 (record[0]), stride 0x44C, corroborated by func_8002C61C's own `u8 *s1=&D_80101EC8; u8 *s0=s1+0x44C;` in the same TU (0x80101EC8+0x44C==0x80102314 exactly). Declared file-local extern s32 D_80102314 in src/code6cac_b.c and used with pointer+offset indexing (d_tbl[OFF/4]) matching the asm's own single lui/addiu-then-offset addressing form.

- [s1] s1 candidate (archived C shape from memory/grind/func_8002C22C/pre-include-asm-body.c, all 49 volatile-coercion casts on scratchpad addresses stripped to plain casts) measures sandbox --disable all = 211 (target 252, build 230 insns), staged live in src/code6cac_b.c this session and confirmed by direct re-measurement.

- [s1] --diff on the s1 candidate: 33 hunks total, 27 source-level / 6 operand-only / 0 not-scored -- the residual is dominated by source-level divergence (scheduling/ordering of scratchpad loads relative to the two if/else branches), not by register-allocation seats.

- [s1] Dumps generated this session (pwsh tools/grinder/dump.ps1 func_8002C22C) to tmp/grind/func_8002C22C/dumps/code6cac_b.{cse,cse2,loop,sched,sched2,flow} but NOT yet read in detail -- next session's first move per the PASS ATTRIBUTION mandate.

- [s1] No sibling ledger transplant was applicable: func_80029454 (same file, floor 1024, active) has no candidate.c to transplant; func_8002C61C (same file, COMPLETED-C) is the matched body already on main and its addressing pattern (s1+0x44C) was the direct evidentiary basis for this session's D_80102314 declaration, so its contribution is already folded into the s1 candidate.
