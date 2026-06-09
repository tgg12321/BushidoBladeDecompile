# Rodata Cleanup Project — End-to-End Plan

**Document status**: AUTHORED 2026-06-08. Self-contained reference for any
future agent picking up this work. Resume-from-anywhere supported.

**One-line summary**: BB2's splat config carved compiler-generated rodata
(jump tables, strings, constants) into `asm/data/*.rodata*.s` blocks that link
EARLIER than C-file rodata sections. Functions whose pure-C compilation would
emit rodata at those addresses can't match. The fix requires per-cluster
en-bloc decomp + `bb2.ld` re-attribution.

---

## 1. Problem statement

### 1.1 What's broken

GCC 2.7.2 emits `.rodata` for switch jump tables, string literals, and
floating-point constants. The compiled rodata is referenced at specific virtual
addresses determined by the linker. BB2's `bb2.ld` places **12
`asm/data/*.rodata*.o` segments** EARLIER than any C-file `.o(.rodata)` segment.

Consequence: a function whose `switch` statement should emit a jump table at
address `0x800108CC` (because that's where the original game's binary has it)
can't have GCC emit the table from its `.c` file — the `.c` file's `.rodata`
section lives much later in the link, at a wrong address.

### 1.2 Why this exists

When splat was set up for BB2 (early in the project), it carved the original
game's `.rodata` section into 12 `asm/data/*.rodata*.s` blocks based on
address ranges, NOT based on source-file ownership. Splat couldn't know which
`.c` file each piece of rodata belonged to without disassembly + cross-reference
analysis that hadn't been done yet. So it grouped rodata by address proximity
and emitted bridging asm.

The current workaround: 145 functions are committed as `replace_with_asmfile`
stubs (the C body is replaced verbatim by the target asm at compile time),
and 24 `asmfix.txt` rules bridge the one jtbl-infra function
(`replay_camera_rob_back_loose2`).

### 1.3 Why SOTN doesn't have this

SOTN's splat setup carved rodata into per-source-file segments from day one
(they had earlier disassembly evidence). They re-split rodata as evidence
surfaces about original TU boundaries (the "evidence-based re-attribution"
mentioned throughout this doc). BB2 hasn't done that re-attribution at scale.

---

## 2. Definition of "done"

The project is complete when:

1. All 12 `asm/data/*.rodata*.s` files are either DELETED or contain only
   verifiably non-attributable data (e.g. orphaned data with no clear C
   owner).
2. All 145 `replace_with_asmfile` stubs are either:
   - Matched in pure C (preferred — SOTN bar) with their rodata emitted by
     cc1 into their `.c` file's rodata section
   - Authorized as COMPLETED-INLINE-ASM-CANONICAL (if hand-coded signals
     genuinely qualify; per the 2026-06-07 audit, ZERO of 4 tested were
     canonical-asm candidates)
3. All 24 `asmfix.txt` jtbl-infra rules retired.
4. `bb2.ld` reflects the actual original-source TU layout (rodata attributed
   per .c file with explicit address placement where needed).
5. Oracle SHA1 == `62efab4f73f992798c43e8c730aa43baa10bb4fa` throughout.

**Definition of "NOT done"**: any function still on a `replace_with_asmfile`
bridge counts as incomplete. The bridge is sanctioned-as-infrastructure
([[no-new-park-categories]]), but it's not COMPLETED-C and not
COMPLETED-INLINE-ASM-CANONICAL — it's INCOMPLETE in the project's three-state
model ([[completion-standard]]).

---

## 3. Current state inventory (measured 2026-06-08)

### 3.1 Rodata files in `bb2.ld`

```
build/asm/data/800.rodata_pre.o(.rodata);                  /* tiny: ~5 strings */
build/asm/data/800.rodata_post.o(.rodata);                 /* 6 KB: 1 jtbl, 66 strs */
build/asm/data/101C.rodata_pre.o(.rodata);                 /* 5 KB: 7 jtbls, 4 strs ← target of 4-sibling cluster */
build/asm/data/101C.rodata_pre_post.o(.rodata);            /* 2 KB: 0 jtbls, 0 strs */
build/asm/data/101C.rodata_c_pre.o(.rodata);               /* trivial */
build/asm/data/101C.rodata_post.o(.rodata);                /* trivial */
build/asm/data/101C.rodata_c2_post.o(.rodata);             /* trivial: 1 str */
build/asm/data/101C.rodata_text1a_DB8.o(.rodata);          /* trivial */
build/asm/data/101C.rodata_text1a_a.o(.rodata);            /* 105 KB: 0 jtbls, 900 strs (string-only) */
build/asm/data/101C.rodata_text1a_b_pre.o(.rodata);        /* 20 KB: 12 jtbls, 7 strs */
build/asm/data/101C.rodata_text1a_b_post.o(.rodata);       /* 23 KB: 4 jtbls, 97 strs */
build/asm/data/101C.rodata_main_post.o(.rodata);           /* 3 KB: 4 jtbls, 6 strs */
```

Plus the BIG ones that don't appear in bb2.ld directly but are referenced via
other segments (TBD — sub-audit required):
```
asm/data/800.rodata.s                 /* 206 KB: 63 jtbls, 1091 strs */
asm/data/101C.rodata.s                /* 177 KB: 49 jtbls, 1019 strs */
```

**Totals**: 140 jtbls + 3,196 strings across 14 files.

### 3.2 Stub functions (replace_with_asmfile)

```
$ grep -c "replace_with_asmfile" asmfix.txt
145
```

### 3.3 Distribution of stub state (from 2026-06-08 audit at tmp/audit_19/)

Of 145 stubs sampled by C-body statement count:

| Category | Count | Description |
|---|---|---|
| Empty body `{}` | 34 | Never decompiled |
| Minimal body (<5 statements) | 91 | Stub placeholder, possibly with `(void)argN;` casts only |
| Substantial body (5+ statements) | 19 | Audited 2026-06-08: NONE were free-matches; 3 were mid-gap (30-50 dist), 3 large-gap (>100), 13 were placeholder stubs misidentified by stmt count |

**Critical finding from 2026-06-08 audit**: stmt-count is a misleading proxy.
The 13 "5 stmt" functions were placeholder bodies like:
```c
s32 func_80052D00(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/...).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
```

A future inventory pass should grep for the literal comment `Body replaced by
asmfix replace_with_asmfile` to distinguish stub placeholders from real
incomplete work.

### 3.4 jtbl-infra confirmed cases

```
$ grep -l "jtbl_" asmfix.txt | xargs grep -c "^[a-zA-Z_].*:" | head
asmfix.txt: 24 rules (all on replay_camera_rob_back_loose2)
```

Only `replay_camera_rob_back_loose2` currently carries jtbl-infra asmfix
rules (the 24-rule bridge for `jtbl_800108CC`). It's the canonical example
in `[[jtbl-rodata-split-infrastructure]]`.

---

## 4. The dependency chain (why this can't be a one-shot)

### 4.1 The blocker pattern

The `asm/data/101C.rodata_pre.s` block contains rodata for **5 functions**:
- `func_80034708` (jtbl_8001086C, jtbl_8001089C) — code6cac_b.c
- `replay_camera_rob_back_loose2` (jtbl_800108CC) — code6cac_b2.c (only one
  with C body + jtbl-infra rules)
- `func_80035828` (jtbl_800108EC, jtbl_80010924) — code6cac_b2.c stub
- `special_camera_set_win_cam` (jtbl_80010938) — code6cac_b2.c stub
- `special_camera_Exec` (jtbl_80010978) — code6cac_b2.c stub

To retire this block:
1. ALL 5 functions must have their bytes emitted (either pure-C cc1
   compilation OR canonical-asm `__asm__("glabel ... .rodata ...")` blocks).
2. Each function's emitted rodata must land at the address its asm reference
   expects.
3. `bb2.ld` must be restructured so the C files containing these functions
   have their `.rodata` placed BEFORE the next downstream rodata segment.

**You cannot retire one function at a time** because:
- Removing `replay_camera_rob_back_loose2`'s jtbl-infra rules without also
  emitting the other 4 jtbls produces link errors (the other 4 are still
  referenced from inside the asm/data block).
- Decompiling `special_camera_Exec` to pure C without coordinating with the
  other 4 (verified 2026-06-08 at WIP `memory/wip/special_camera_Exec/`) hits
  link failure because cc1 emits a NEW jtbl at `code6cac_b2.o(.rodata)`'s late
  address, while the external `jtbl_80010978` reference still points to the
  asm/data address.

### 4.2 The multi-block dependency

Some blocks are independent (their owners are all in one .c file, easy
re-attribution). Others span multiple .c files and require coordinated work
across the source tree.

**Pre-audit hypothesis**: the two BIG files (`800.rodata.s` 206 KB,
`101C.rodata.s` 177 KB) probably span MANY .c files and contain rodata for
dozens of functions each. They're the heavy items.

---

## 5. Project phases (per-cluster work)

Run these in roughly the order listed. Each phase MUST end with oracle SHA1 ==
`62efab4f73f992798c43e8c730aa43baa10bb4fa`.

### Phase 0: Full rodata block inventory

**Goal**: For each of the 14 `asm/data/*.rodata*.s` files, enumerate:
- What addresses it covers (rodata range)
- What jtbls + strings it contains (with addresses)
- Which functions reference each (cross-grep `asm/funcs/*.s` and src/*.c)
- Per-function: is the function currently a stub, pure-C-matched, or
  canonical-asm-authorized?

**Tooling**: write `tools/audit_rodata_blocks.py` that produces a CSV with
columns: `file, address, type, symbol, owner_function, owner_file, owner_status`.

**Output**: `tmp/rodata_block_inventory.csv` (committed to repo? optional —
it's mostly diagnostic).

**Why first**: the phase plan beyond this is hypothetical until we know what's
actually in each block.

### Phase 1: Cluster classification

Group functions by which rodata block they depend on. For each cluster:
- Number of functions
- Number of jtbls/strings
- Current state distribution (stub / matched / authorized)
- Cross-file? (Same `.c` file = easier. Multi-file = harder.)

**Decision per cluster**:
- **SINGLE-FUNCTION cluster**: only one function owns the rodata. Easy
  re-attribution — convert the function to pure C with cc1 emitting the
  rodata naturally; delete the asm/data block; update `bb2.ld`.
- **SINGLE-FILE cluster**: multiple functions but all in one .c file.
  Medium difficulty — match all the functions, ensure their combined `.rodata`
  ends up at the right address.
- **MULTI-FILE cluster** (like the 5-function `101C.rodata_pre.s` cluster):
  hardest. May require .c file restructuring to colocate the functions in one
  file, OR placing multiple .c file `.rodata` segments at specific addresses
  in `bb2.ld`.

### Phase 2: Per-stub decomp (the bulk of the work)

For each stub function:
1. Read target asm (`asm/funcs/<func>.s`)
2. Read Kengo cross-reference if available (it identifies which functions had
   C counterparts in the PS2 successor engine)
3. m2c the target for an initial pure-C reconstruction
4. Iterate: edit source, `sandbox <func> --disable all`, apply pure-C levers
   (see Section 6 for the lever index)
5. Once sandbox distance → 0: `verify-oracle --rebuild` must keep SHA1 green
6. Cheat-reviewer PASS required ([[review-discipline-before-commit]])
7. Commit as `Match: <func> (<file>.c) — COMPLETED-C`

**Recipe for difficult cases**: if pure-C distance doesn't close after standard
levers, fall back to:
- WIP entry at `memory/wip/<func>/` (don't commit forced cheats)
- Document the floor + tested levers + next hypotheses
- Defer to a future session

**Anti-pattern**: do NOT add register-asm pins, `__asm__` barriers, dead
stores, volatile coercion casts, OR-tree-shape-shifts, or any other expanded-
catalog cheat from `[[inline-asm-policy]]`. Today's no-new-regfix guard blocks
new regfix entries; the cheat-reviewer blocks cheat-class C constructs.

### Phase 3: Per-cluster en-bloc re-attribution

Once all functions in a cluster are matched-in-pure-C:

1. **Identify the target placement** for the cluster's combined `.rodata`.
   GCC emits each .c file's rodata in source order at one address. The cluster's
   .c file(s) need `.rodata` at the address where the asm/data block currently lives.

2. **Restructure `bb2.ld`**: remove the asm/data block, replace with the .c file's
   rodata segment(s). If multiple .c files are involved, place them in order such
   that each segment lands at the right address.

3. **Verify cascade**: every later rodata segment shifts. Run
   `verify-oracle --rebuild` end-to-end. If oracle fails, identify which
   function's rodata moved + decide whether to cascade-fix (only if straightforward).

4. **Retire** any remaining asmfix bridging rules (jtbl-infra) — they're no longer
   needed because cc1 now emits the jtbls in the right place.

5. **Commit** as `engine: retire <block> via en-bloc rodata re-attribution`.
   Body lists the functions retired, the asm/data block deleted, the bb2.ld
   changes.

### Phase 4: Validate + iterate

After each cluster retirement, re-run the inventory (Phase 0) to refresh state
and pick the next cluster.

**Smallest-first strategy**: tackle the trivial-rodata blocks first
(`101C.rodata_pre_post.s` 2 KB / 0 jtbls; `101C.rodata_c_pre.s` ~50 bytes; etc.)
to build pattern recognition + tooling familiarity before the big files
(`800.rodata.s` 206 KB; `101C.rodata.s` 177 KB).

---

## 6. Pure-C lever index (for Phase 2 decomp work)

The levers below close the typical gap between m2c output and SHA1 match.
All are SOTN-sanctioned. Reference each rule by its `.claude/rules/` filename.

### 6.1 Register-allocation levers (when gap is reg-rename / scheduling)

- **`register-alloc-pure-c.md`** — Levers A (block-local var split), B (narrow
  integer type), C (loop-local precompute). The core RA playbook. Lever D
  (dead-param-assign) is now FORBIDDEN — don't reach for it.
- **`narrow-byte-args-packed-call.md`** — `u8` params for byte-packed call args
  + hi/lo statement split.
- **`narrow-stack-param-subword-offset.md`** — `u16 arg4_lo = *(u16 *)&arg4;`
  for stack-param subword offset.
- **`u16-global-lhu-lbu-low-byte.md`** — `*(u8 *)&G` for narrow access.
- **`halfword-index-srl-sra.md`** — byte-offset cast to avoid combine's
  srl-canonicalization.
- **`restore-discarded-return-displaces-v0.md`** — restore `void` impls to
  return the call's value when caller captures it.

### 6.2 Scheduling levers (when gap is delay-slot fills / insn ordering)

- **`store-before-jal.md`** — C statement structure drives jal delay-slot fill.
- **`loop-rotation-two-shift.md`** — natural `for` loop + opaque `one` var.
- **`loop-note-fixes-delay-slot-steal.md`** — real `while`/`do` form
  defeats reorg.c misprediction.
- **`compare-operand-order-register.md`** — `local OP global` not
  `global OP local` for RTL evaluation order.

### 6.3 Code-shape levers (when gap is structural — switches, dispatches)

- **`shared-end-label.md`** — single shared `end:` defeats per-case
  constant-fold.
- **`switch-vs-ifchain-branch-sense.md`** — real `switch` matches GCC's
  jtbl/decision-tree codegen.
- **`cross-jump-call-merge.md`** — arg-count distinct between fn-ptr calls
  defeats merge.
- **`cross-jump-store-tail-merge.md`** — mix exit forms (goto vs inline
  return) defeats store-tail merge.
- **`split-read-defeats-hoist.md`** — duplicate reads into branch arms.

### 6.4 Loop/hoist levers

- **`defeat-licm-hoist-var-reuse.md`** — reuse scratch var to defeat LICM.
- **`store-const-reload-cse.md`** — drop saved-local reload; re-read global
  at comparison.

### 6.5 Misc

- **`sandbox-zero-retire-fails.md`** — when sandbox 0 hides a source-cheat
  barrier; remove it + retire.
- **`prologue-fix-redundant-reorder.md`** — sometimes a prologue regfix
  reorder rule is just undoing prologue_fix; remove both.
- **`maspsx-label-nop-gate.md`** — add to allowlist for legitimate load-delay
  nop emission.

### 6.6 do-while-zero (last resort)

- **`do-while-zero-exception.md`** — NARROW exception for LABEL_OUTSIDE_LOOP_P
  / reorg.c invert-jump peephole. Annotate with `/* FAKE: ... */`. Used only
  after demonstrated lever-exhaustion + cheat-reviewer PASS.

### 6.7 Cheats to NEVER use

- Register-asm pins (`register T x asm("$N")`)
- `extern volatile T G;` (narrow carve-out exists in
  `legitimate-volatile-interrupt-touched.md`; default is forbidden)
- `*(volatile T *)&G` casts
- `arg0 = 0;` dead-param-assign
- `if (cond) v = x; v = x;` dead-conditional-store
- `s32 buf[N];` dead-local-array
- `(void)&local;` address-coerce
- Inline-asm `__asm__("addu $N,...")` hardcoded register
- `move %0, %1` operand-pinning
- `__asm__("" ::: "memory")` scheduling barrier
- OR-tree-shape-shift (operand permutation in `a | b | c`)
- Chain-extender (combine-foldable extra arithmetic)
- Dead-goto label-pad
- DImode chain

All catch-net'd by the `engine/volatile_cheats.py` + `audit_asm_cheats.py`
detectors + the cheat-reviewer agent.

---

## 7. Tooling

### 7.1 Existing (use these)

- `& tools/eng.ps1 canonical <func>` — verdict the function's category
- `& tools/eng.ps1 sandbox <func> --disable all` — honest pure-C distance
- `& tools/eng.ps1 verify-oracle --rebuild` — full SHA1 gate
- `& tools/eng.ps1 retire <func>` — drops rules + SHA1 gates
- `& tools/eng.ps1 queue done <func>` — records COMPLETED
- `tools/probe_func_labels.sh` — get cc1's `.L<N>` label sequence for a func
- `tools/normalize_lf.py` — fix CRLF in build files (from 2026-06-07 incident)
- The `no_new_regfix_guard.py` commit-msg hook — blocks new rule additions
- The `park_src_guard.py` commit-msg hook — blocks `park:` commits modifying src
- cheat-reviewer agent (`subagent_type: cheat-reviewer`) — adversarial review

### 7.2 To author for this project

#### `tools/audit_rodata_blocks.py` (Phase 0)

Inventory each `asm/data/*.rodata*.s`:
- Parse the file for symbol definitions (`dlabel`, `dlabel jtbl_`, `.asciz`, etc.)
- For each symbol, find references in `asm/funcs/*.s` via grep
- For each referencing function, look up its current status from
  `engine/queue.json`
- Emit CSV: `block_file, symbol, type, owner_func, owner_file, owner_status`

#### `tools/cluster_rodata.py` (Phase 1)

Group rows from the inventory CSV into clusters by `block_file`. For each cluster,
emit:
- Function count
- Cross-file status (all same .c file, or multi-file)
- All-stub state (every owner is `replace_with_asmfile`), partial
  (mix of matched + stub), or all-matched (= ready for re-attribution)

#### `tools/re_attribute_rodata.py` (Phase 3 helper)

Given a cluster:
- Compute the bb2.ld changes needed
- Diff candidate vs current bb2.ld
- Verify the math (segment addresses + sizes add up)
- Don't apply automatically — write to a tmp file for review

### 7.3 Inspect target asm

```
asm/funcs/<func>.s     # disassembled target
asm/data/<block>.s     # rodata block (the problem)
bb2.ld                 # link order
splat.yaml             # splat config
```

---

## 8. Anti-patterns (what NOT to do)

### 8.1 Speculative rodata reorder

If you change `bb2.ld` to make oracle pass WITHOUT corresponding evidence of
TU attribution, that's a structural cheat — same category as regfix offset
paperwork. Forbidden per `[[no-new-park-categories]]`.

Test: does your `bb2.ld` change reflect a discovery about original TU
boundaries (evidence-based) OR a desire to make the build work (speculative)?
Only the former is acceptable.

### 8.2 Force-matching individual functions in a multi-owner cluster

Verified 2026-06-08 with `special_camera_Exec` WIP — decomping one function
in a multi-owner cluster hits link failure. The whole cluster needs en-bloc
work. Don't commit individual stub→pure-C transitions if the function shares
a rodata block with other stubs.

Exception: SINGLE-FUNCTION clusters (where only one function owns the
rodata block) can be done individually.

### 8.3 Adding asmfix rules to "fix" link failures

If decomp produces a link failure, the answer is NOT a new asmfix rule.
Today's no-new-regfix-rules guard blocks net additions to asmfix.txt; the
escape hatch `[infra-rule: jtbl-infra]` is for newly-DISCOVERED infrastructure
(rare), not for bandaging incomplete decomp work.

### 8.4 Authorizing as canonical-asm to avoid the work

The 2026-06-08 audit found ZERO of 4 stub siblings qualified for canonical-asm
authorization. They lack S1-S8 signals + Kengo cross-references prove the
original was C. Authorizing them anyway would be a speculative attribution
to force a SHA1 match — same category as the regfix paperwork cheats.

---

## 9. Decision tree (resume from any state)

```
Are you starting cold? → Read this entire doc + .claude/rules/jtbl-rodata-split-infrastructure.md
                       → Read .claude/rules/no-new-park-categories.md
                       → Read tmp/4-sibling-audit-2026-06-08.md if it exists

Has Phase 0 (full inventory) been completed? (Check for tmp/rodata_block_inventory.csv or similar)
  NO  → Start with Phase 0. Write tools/audit_rodata_blocks.py + run it. Commit the CSV as engine:.
  YES → Go to next question.

Has Phase 1 (cluster classification) been completed? (Check for tmp/rodata_clusters.csv)
  NO  → Run Phase 1 from the inventory data. Pick next cluster.
  YES → Go to next question.

Pick a cluster to work on. Sort by these criteria (preferred order):
  1. SINGLE-FUNCTION clusters first (easiest)
  2. SINGLE-FILE clusters second (medium)
  3. Smallest MULTI-FILE cluster third (build pattern)
  4. Larger MULTI-FILE clusters last

For the chosen cluster:
  Are all owner functions already matched? (No stubs in the cluster)
    YES → Execute Phase 3 (en-bloc re-attribution). Skip Phase 2 for this cluster.
    NO  → Go to Phase 2. Match the stubs first (one per worker session).

Within Phase 2 for a chosen stub:
  Is the function jtbl-blocked? (Removing replace_with_asmfile → link error)
    YES → This stub depends on cluster cohorts. Match ALL cohorts first, then re-attribute en-bloc.
    NO  → Standard decomp work (see Section 6 levers).
```

---

## 10. Resume-from-anywhere contract

If you (future agent) are picking this up cold, here's the canonical resume
procedure:

1. **Read this doc end to end** (don't skip).
2. **Verify oracle**: `& tools/eng.ps1 verify-oracle` (no rebuild). SHA1 must be
   `62efab4f73f992798c43e8c730aa43baa10bb4fa`. If not, stop and figure out
   what's broken before starting any rodata work.
3. **Read the related rules**:
   - `.claude/rules/jtbl-rodata-split-infrastructure.md`
   - `.claude/rules/no-new-park-categories.md`
   - `.claude/rules/canonical-asm-authorization-recipe.md`
   - `.claude/rules/inline-asm-policy.md`
   - `.claude/rules/no-new-regfix-rules.md`
4. **Read prior WIP entries**:
   - `memory/wip/special_camera_Exec/` (the 2026-06-08 attempt at the
     `101C.rodata_pre.s` cluster — has the candidate.c that was reverted)
5. **Read the audit reports**:
   - `tmp/4-sibling-audit-2026-06-08.md`
   - `tmp/audit_19/` (the 19-substantial-body audit results)
6. **Decide phase**: check Section 9's decision tree.
7. **Work the chosen phase**. Commit incrementally. Oracle green at every commit.

**Commit message conventions** (per `docs/COMMIT_CONVENTIONS.md`):
- `engine: ...` — tool/infrastructure changes (CSV, helper scripts, bb2.ld
  restructuring)
- `Match: <func> (<file>.c) — COMPLETED-C` — individual function match (Phase 2)
- `engine: retire <block> via en-bloc rodata re-attribution` — cluster
  completion (Phase 3)
- `cheat-cleanup: ...` — if you find pre-existing asmfix-cheat patterns
  alongside rodata work

---

## 11. Estimated effort

Order-of-magnitude scale, assuming 1 worker session = 30-90 min:

| Phase | Effort |
|---|---|
| Phase 0 (inventory) | 1 session (write the tool + run it) |
| Phase 1 (classification) | 1 session (write the tool + run it) |
| Phase 2 (per-stub decomp) | **~145 sessions** (one per stub, average) |
| Phase 3 (per-cluster re-attribution) | ~12-15 sessions (one per cluster) |
| Phase 4 (final validation) | 1-2 sessions |

**Total**: ~160-170 worker sessions over the project lifetime. This is a
multi-month project at current pace (~10-15 worker sessions per active day).

**Acceleration paths**:
- Find Kengo cross-references early — they tell you which functions had C
  counterparts (high-confidence pure-C-reachable signals).
- Tackle structurally-similar function clusters together (siblings often
  share patterns).
- The first few cluster retirements build pattern-recognition that makes
  later ones faster.

---

## 12. Known unknowns + future research

### 12.1 Are the BIG files (800.rodata.s, 101C.rodata.s) actually linked?

The bb2.ld grep above only found 12 explicit `asm/data/*.rodata*.o`
references, and the two largest files (`800.rodata.s` and `101C.rodata.s`)
aren't in that list. Either:
- They're linked via wildcards or other indirection (check `bb2.ld` carefully)
- They're not actually used by the build (= we can ignore them)
- They're consumed by the segment-`pre`/`post` variants (split at splat time
  into the variants that ARE linked)

**Phase 0 must determine which**. The answer changes the project scope
significantly.

### 12.2 String-only blocks

`101C.rodata_text1a_a.s` has 900 strings + 0 jtbls (105 KB). Strings are
generally easier to re-attribute than jtbls because they're typically
referenced as `lui+addiu` constant addresses, not as switch dispatch targets.
A string can move to any .c file's `.rodata` as long as the address relocations
update.

**Hypothesis**: string-only blocks may be retirable with simpler re-attribution
than jtbl-bearing blocks. Phase 0 should test this.

### 12.3 Multi-file clusters

Some clusters genuinely span multiple .c files (as the 5-function
`101C.rodata_pre.s` cluster does — code6cac_b.c + code6cac_b2.c). For these,
options include:
- (a) Restructure source files to colocate the cluster's functions in one .c
  file (heavy)
- (b) Use multiple `.o(.rodata)` segments in `bb2.ld` at adjacent addresses
- (c) Other

**Phase 3 must develop the recipe for this case**. The 5-function cluster is
the first test bed.

---

## 13. Success metrics + checkpoints

### 13.1 Per-cluster

- All functions in cluster reach COMPLETED-C or COMPLETED-INLINE-ASM-CANONICAL
- The asm/data block file is deleted from repo + `bb2.ld`
- Oracle SHA1 stays green
- All asmfix bridging rules for the cluster retired

### 13.2 Project-wide

- 0 `asm/data/*.rodata*.s` files remain in `bb2.ld`
- 0 `replace_with_asmfile` rules in `asmfix.txt`
- 0 jtbl-infra rules in `asmfix.txt`
- 145 stubs → 0 stubs
- Oracle SHA1 still green (the original `62efab4f73f992798c43e8c730aa43baa10bb4fa`)

### 13.3 Process metrics (per session)

- Each session ends with oracle green
- Each session produces ONE of: cluster retirement, stub match, WIP
  checkpoint, or inventory/tooling artifact
- Zero cheats committed (no register-asm pins, no volatile coercion, no
  catalog-class constructs)

---

## 14. Related project work that compounds

### 14.1 The 5 WIP cluster (B844/C7A0/C86C/C97C/CBB0)

These 5 are NOT rodata-blocked. They're register-rotation / scheduling walls
documented across 15+ rounds. They have NO relationship to this project.

### 14.2 The legitimate-volatile carve-out (2026-06-08)

Some functions may use `extern volatile T G;` correctly under the narrow
carve-out (`legitimate-volatile-interrupt-touched.md`). If a stub function
references an IRQ-touched global, the carve-out may apply.

### 14.3 The {lbl#N} migration (2026-06-08)

Drift-robust label slots in regfix/asmfix. The infrastructure benefits all
match work but doesn't directly retire stubs.

---

## 15. Pointers for the next agent

When you're ready to start:

1. **First useful command**: `python3 tools/audit_rodata_blocks.py > tmp/rodata_inventory.csv`
   (if the tool doesn't exist yet — author it first, see Section 7.2)

2. **Start small**: pick the smallest single-function cluster from the
   inventory. Land that retirement. Build confidence + tooling.

3. **Don't tackle 800.rodata.s or 101C.rodata.s first**. They're the largest
   blocks. Work your way up.

4. **Track in `memory/project/rodata-cleanup-progress.md`** — append a
   checkpoint per cluster retired. Lets future agents (and you-future) resume.

5. **When in doubt, default to "don't commit a cheat"**. WIP entries are
   fine. Stub→matched is the goal, but stub→WIP is acceptable progress.

This is real, important architectural work. It's not flashy, but it
permanently retires a class of project debt. Treat each cluster retirement
as a real milestone.

---

**End of plan. Resume contract per Section 10. Good luck.**
