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

## s2 (structural, 2026-09-22)

- **PASS ATTRIBUTION run per brief mandate.** `pwsh tools/grinder/dump.ps1 func_8002C22C` regenerated
  `tmp/grind/func_8002C22C/dumps/code6cac_b.*`. `grep -n ";; Function func_8002C22C"` located the
  block in `.loop` (line 6365-7445), `.cse` (6352-7439), `.sched` (10014-11884), `.combine`
  (6437-7551). **`.loop` dump has ZERO invariant/hoist/giv notes for this function** — the function
  has NO LOOPS (straight-line code with two if/else pairs, no back-edges), so LICM (loop.c) is
  MECHANICALLY INAPPLICABLE here. This overturns H3's "loop.c LICM" mechanism guess from s1 — LICM
  cannot be the cause because there is nothing for it to operate on.
- **The real mechanism is CSE1's block-extension merge** (`.claude/rules/cse-block-extension-controls-fold-span.md`,
  `cse.c:8102-8184`). Read `.cse` dump lines 6352-6420: at the very top of the function's RTL (before
  any scheduling), each of the six scratchpad-zero-store addresses (0x1F800360/364/368/370/374/378 =
  CONST_INT 528483168/172/176/184/188/192) is materialized into its OWN fresh pseudo register
  (regs 83-87 etc.) via `(set (reg:SI N) (const_int ADDR))` immediately followed by `(set (mem:SI
  (reg:SI N)) (const_int 0))` — i.e. cse1 does NOT merge the six addresses with each other (good,
  they're different constants). But `grep`-ing the SAME constants across the whole function body
  (lines 6352-7439) shows **0x1F800360's address-pseudo (528483168) and 0x1F800364's (528483172) and
  0x1F800370's (528483184) each appear EXACTLY ONCE in the entire dump**, despite being written to
  again inside BOTH if/else arms in the s1 candidate's C — confirming cse1 recognizes the LATER
  in-arm stores to the SAME address as reusing the pseudo computed once at the top, i.e. it is
  treating the pre-branch block and BOTH branch arms as one continuous CSE region
  (`;; Processing block from 2 to 239` spans past the conditional branch — the `cse_end_of_basic_block`
  extension the rule documents, `LABEL_NUSES(JUMP_LABEL)==1` on the else-arm's entry label). This is
  what produced s1's compiled shape: 4 addresses batch-computed once up front (`lui/ori t1/t2/a0/t3`)
  and reused as long-lived pointer registers across both branches, vs target's per-statement
  independent `lui at,0x1f80; ...; sw ...` at EVERY occurrence (confirmed via `--diff` hunks 1-10 on
  the s1 candidate: target NEVER reuses a materialized scratchpad base register across two different
  stores, ours does).
- **STRUCTURAL LEVER — CONFIRMED, measured this session.** Moved the six `*(s32*)0x1F80036x = 0;`
  zero-init statements from BEFORE the first `if` (unconditional, shared by both arms) to the START
  of EACH arm (duplicated verbatim into both `if` and `else` bodies) — semantically identical (both
  arms always ran the zero-init before, since it wasn't itself conditional; now each arm carries its
  own copy). This does NOT fully separate cse1's processing of the two arms (the `.cse` dump still
  shows constants like 528483176 (0x1F800368) reused across the join at least once), but it does
  measurably shrink the SPAN over which the compiler's per-store address gets shared, because the
  in-arm zero-init store is now the FIRST reference to some of these addresses within its own arm
  rather than a shared reference dominating both arms. Measured: **`sandbox --disable all` = 199**
  (`build_insns` 230 -> 246, target 252) — a drop of 12 from s1's 211. `--diff` after this change: 36
  hunks, 32 source-level / 4 operand-only / 0 not-scored; hunk 1 shows our build STILL batches four
  `lui/ori` pairs together before the four zero-stores WITHIN one arm (the merge now happens
  intra-arm, not cross-arm) — the residual mechanism is unchanged (cse1 sharing an address pseudo
  across multiple same-address stores within the arm's now-larger straight-line block), just with a
  smaller span.
- **VOLATILE HYPOTHESIS — CLASS-SCOPE EVIDENCE FOUND (do not re-attempt without new precedent).**
  `grep` for scratchpad address literals across `src/*.c` found `src/code6cac.c:237-238`
  (func_80017FA0, PutRobShadow's neighbor in the SAME file/cluster): its doc comment states
  **"This supersedes the s4 volatile form (Judge FAIL 2026-08-20 02:54, construct BANNED: volatile on
  scratchpad 0x1F800000-0x1F8003FF)"** — a Judge FAIL, on a function touching THIS EXACT scratchpad
  address range, explicitly banning volatile there, and closing the function instead via a genuine
  goto/do-while loop restructuring (loop.c mechanism, unrelated to volatile). This is a directly
  on-point, citable precedent that volatile is NOT an available lever for 0x1F800000-0x1F8003FF absent
  a new IRQ-writer citation — consistent with `.claude/rules/legitimate-volatile-interrupt-touched.md`
  (two-prong gate, no IRQ writer identified for these addresses) and
  `.claude/rules/mmio-volatile-type-level.md` (scratchpad explicitly excluded from the type-level MMIO
  carve-out, which only covers 0x1F801000-0x1F802FFF). H3 (s1, KILLED instance) is therefore
  reinforced, and its `next probe` #2 (grep for a sibling using these addresses) is now answered:
  found, and the sibling's answer is "volatile was tried and Judge-FAILed", not "volatile worked".
- **REJECTED — do-while(0) wrap on one arm's zero-init.** Tried wrapping the if-arm's six-store
  zero-init block in `do { ... } while (0);` (FAKE-annotated per
  `.claude/rules/do-while-zero-exception.md`, hoping the loop-note boundary would trip one of
  `cse_end_of_basic_block`'s three backward-scan escapes). Measured **WORSE**: `sandbox --disable all`
  = 200, `build_insns` unchanged at 246 — net zero benefit (same instruction count, worse masked
  operand score) for an added construct. Reverted; saved to
  `memory/grind/func_8002C22C/rejected/dowhile0-zero-init-single-arm.c`. KILLED (instance).

- [s2] func_8002C22C has NO loops (two straight-line if/else pairs, zero back-edges) confirmed via an empty .loop dump for its RTL block, so any future hypothesis invoking loop.c/LICM for this function is invalid by construction.

- [s2] cse.c's cse_end_of_basic_block extends its processing region across a conditional branch whose jump-target label has LABEL_NUSES==1, forwarding constant-address pseudo computations across if/else joins; this is the confirmed mechanism behind the s1 211-residual (see .claude/rules/cse-block-extension-controls-fold-span.md for the general pattern, cse.c:8102-8184 for the exact predicate).

- [s2] Duplicating the shared pre-branch scratchpad zero-init into both if/else arms is semantically identical to the original (both arms always executed it) and measurably improves the honest floor 211 -> 199 by shrinking the cse1-shared span from cross-arm to intra-arm.

- [s2] The remaining 199-residual's --diff hunk 1 shows the SAME address-batching mechanism (four lui/ori pairs materialized together before four stores) now operating WITHIN one arm rather than across both arms -- the next lever is a further span-shrinking restructuring, not a new mechanism.

- [s2] Volatile is unavailable for the 0x1F800000-0x1F8003FF scratchpad range on this project: a sibling function (func_80017FA0, src/code6cac.c:237-238) already tried it and the Judge FAILed it (2026-08-20 02:54), closing instead via a genuine loop restructuring. Do not re-attempt volatile on this range without a new IRQ-writer citation.

- [s2] do-while(0) wrapping a zero-init block (FAKE-annotated, sanctioned family) measured net-negative (199 -> 200) for this specific placement; not a universal claim about the family, just this instance.

## s3 (2026-09-22) — ground-truth asm read + dead-store removal
- `asm/funcs/func_8002C22C.s` lines 2-18: the ORIGINAL zero-init of the six
  scratchpad words (0x1F800360/364/368/370/374/378) is a SINGLE unconditional
  6x `sw zero` block before the `andi $v0,$v0,1 / beqz` branch — it is never
  duplicated per-arm in the target. This contradicts a naive reading of the
  s2 candidate's per-arm-duplicated 6-word block, but per H5 below, literally
  mirroring that single-block structure in our C measures WORSE (211) than
  the duplicated-per-arm form (196) — the duplication is doing real
  CSE-block-extension-defeat work our fork needs even though it doesn't
  mirror the target's own join topology.
- Of those six words, only FOUR (0x360/364/368/370) are ever read back or
  re-stored with a real value INSIDE either arm. The other two
  (0x1F800374/0x1F800378) get their only real writes from `d_v1`/`d_a0` AFTER
  the if/else join — so a per-arm zero-store to those two addresses is dead
  code with no bearing on the CSE-defeat mechanism. Dropping those two dead
  stores from both arms: floor 199 -> 196 (build_insns 246 -> 238). Ordinary
  dead-code removal, zero FAKE, zero cheat constructs.
- `sandbox --disable all --diff`'s hunk/insn-count based alignment is
  UNRELIABLE evidence for "what the target's own zero-init structure looks
  like" — it re-aligns differently depending on OUR code shape (observed: the
  same target bytes were grouped into a 4-word-then-branch hunk under one of
  our code shapes and a 6-word-then-branch hunk under another). Ground truth
  for target structure is `asm/funcs/<func>.s`, not the diff hunk grouping.

- [s3] asm/funcs/func_8002C22C.s lines 2-18: the target's zero-init of the six scratchpad words (0x1F800360/364/368/370/374/378) is a SINGLE unconditional 6x `sw zero` block before the `andi/beqz` branch, never duplicated per-arm in the original.

- [s3] Of those six words, only 0x360/364/368/370 are ever read back or re-stored with a real value inside either arm; 0x374/0x378 receive their first real write from d_v1/d_a0 after the if/else join.

- [s3] sandbox --disable all --diff's hunk/insn-count-based alignment is unreliable evidence for the target's own zero-init structure -- the same target bytes were grouped into a 4-word-then-branch hunk under one candidate shape and a 6-word-then-branch hunk under another; asm/funcs/<func>.s is the ground truth, not the diff hunk grouping.

- [s3] Despite the target's own source apparently using a single un-duplicated zero-init block, literally mirroring that structure in our C measures worse (211) than the sanctioned per-arm duplicated-statement-into-arms spelling (196) -- confirms the s2-session hypothesis that duplication is doing real CSE-block-extension-defeat work in our fork independent of whether it matches the target's own join topology.

- [s3] Preloading all six per-arm field reads into named locals ahead of the store sequence (matching the target's apparent load order in the .s) also measures worse (211) than reading a0/a1/a3 inline at point of use (196).

- [s4] `--diff` on the 196-floor chassis: 28/36 hunks are source-level, ALL of them in or immediately around the second if/else (accumulate) block; the first block (zero-init + vec-delta) has zero hunks (fully closed). The residual is concentrated entirely in one region.

- [s4] Reading the target's accumulate-region asm directly (not through --diff, which reflows unreliably per s3's finding above): the target FUSES the two source tables' contributions per output field in an interleaved load/add/store sequence (load current t0 slot value + next field's table operand together, store to the PREVIOUS field's slot, then move to the next), not two sequential per-table passes like the candidate's `t0[X] += a; t0[Y] += b; ...; t0[X] += c; t0[Y] += d; ...`. This is the concrete restructuring target for s5 — see frontier.

- [s4] decomp-permuter workspace built for func_8002C22C for the first time: tmp/grind/func_8002C22C/s4/nonmatchings/func_8002C22C (base.c/compile.sh/target.o/settings.toml), rebuild scripts (joinstr.py, asmpragma.py, mktarget.sh) in tmp/grind/func_8002C22C/s4/. Two fidelity bugs found+fixed (multi-line asm string literals breaking pycparser; import.py's asm-stub regex not matching `__asm__ volatile(`); both fixes verified codegen-neutral for func_8002C22C by direct objdump comparison against the unfixed compile. import.py's own target.o build used the wrong assembler (mips-linux-gnu-as -march=vr4300, big-endian tradbigmips) — rebuilt from tools/decomp-permuter/prelude.inc + asm/funcs/func_8002C22C.s with mipsel-linux-gnu-as -march=r3000 (little-endian, matching the project's real toolchain); did not change the permuter's own weighted score (it diffs decoded mnemonics, not raw bytes), but the corrected target.o is what's banked in the workspace for any future campaign.

- [s4] Campaign telemetry (two launch/harvest cycles, both --stop, zero orphaned): base permuter weighted score 13260, best-of found 11665 over 794 iterations in ~28s wall. Re-spliced the single best find into src/code6cac_b.c and measured on the real engine sandbox: still 196/238 build_insns, zero improvement. The find's only structural change (staging `*(s32*)0x1F80005C` through a fresh local before use) is codegen-neutral here. Confirms (again, per func_8002D780's s4 finding) that the permuter's weighted score and the engine's raw honest floor are not comparable and a low-weighted find is not evidence of a real improvement without direct re-measurement.

- [s4] sandbox --disable all --diff on the 196-floor chassis: 36 hunks, 28 source-level concentrated entirely in/around the second if/else accumulate block (t0[0xA8..0xC0/4] += ...); the first block (zero-init + vec-delta) has zero hunks, fully closed.

- [s4] Direct read of the target's accumulate-region asm shows it fuses the two source tables' contributions per output field in an interleaved load/add/store sequence, not two sequential per-table passes like the candidate's current C -- this is the concrete restructuring target for the next structural session, not a register/scheduling lever.

- [s4] Sibling check this session: func_8002D780's candidate.c (memory/grind/func_8002D780/candidate.c) shares zero of func_8002C22C's addresses/globals (0x1F800xxx literals, D_80102xxx, D_800A3824 all absent) -- no transplantable block exists between the two functions despite both being in src/code6cac_b.c; confirmed by direct grep before any probe this session.

- [s4] func_80029454 (same file, floor 1024) has no candidate.c to transplant.

- [s4] main (src/ings.c) is a MIDI-dispatch function (per project memory) sharing no object-model or address-space overlap with this scratchpad/practice-menu function; not a transplant candidate.
