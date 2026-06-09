# Rodata Cleanup Project — End-to-End Plan

> **STATUS (2026-06-09): Phase A COMPLETE — Phase B IN PROGRESS.**
>
> **Phase A — block retirement (§12.2 first metric): DONE.** All 12
> `asm/data/*.rodata*.o(.rodata)` entries retired from `bb2.ld`. Oracle
> SHA1 unchanged (`62efab4f73f992798c43e8c730aa43baa10bb4fa`).
>
> **Phase B — per-function follow-up (§12.1 + §12.2 remaining metrics): NOT
> DONE.** The functions whose rodata was relocated are still INCOMPLETE:
> - 24 jtbl-infra asmfix rules on `replay_camera_rob_back_loose2` still
>   bridge GCC's emitted jtbl to the external `jtbl_800108CC` (which now
>   lives in `src/code6cac_b_rodata.c` instead of the asm/data block, but
>   the bridge mechanism remains the same).
> - 145 `replace_with_asmfile` stubs unchanged. The sub-TU split pattern
>   (see §5 Phase 3 expansion below) decoupled rodata cleanup from
>   per-function decomp — the stubs can now be matched through the normal
>   engine queue without rodata coordination.
>
> The per-cluster retirement log + recipe descriptions are in
> `memory/project/rodata-cleanup-progress.md` (authoritative retrospective).
>
> **Critical post-Phase-A note**: `bb2.ld` is now HAND-MAINTAINED. Do NOT
> regenerate it via `make setup` / splat — see the warning at the top of
> `splat.yaml` for recovery procedure if it happens accidentally.

**Document status**: REVISED 2026-06-09 after Phase A completion. Originally
AUTHORED 2026-06-08, REVISED 2026-06-09 (pre-execution refinements), then
REVISED again post-Phase-A to reflect the sub-TU split pattern that emerged
during execution + the deferred Phase B work.

**One-line summary**: BB2's splat config carved compiler-generated rodata
(jump tables, strings, constants) into `asm/data/*.rodata*.s` blocks that link
EARLIER than C-file rodata sections. Functions whose pure-C compilation would
emit rodata at those addresses can't match. The fix is per-cluster en-bloc
decomp + `bb2.ld` re-attribution, driven by EVIDENCE about original TU
boundaries — never speculative reorder.

---

## 1. Scope discipline (read first)

This is the **rodata-cleanup** project. It is NOT "decompile every remaining
stub." The two are correlated but not identical:

- **Rodata-cleanup goal**: retire every `asm/data/*.rodata*.s` block from
  `bb2.ld` by re-attributing the rodata to the C files that own it. Project
  completes when **zero** `asm/data/*.rodata*.o(.rodata)` entries remain in
  `bb2.ld`.
- **Stub-elimination is a side-effect, not the goal.** Many of the 145
  `replace_with_asmfile` stubs are rodata-blocked and will be matched as
  cluster owners get re-attributed; others are just undecompiled functions
  unrelated to rodata. The latter belong to the engine queue, not to this
  project.

Keep the success metric anchored to rodata-block retirement (§13). If a stub
has no rodata at all, finishing it is queue work, not this project.

---

## 2. Problem statement

### 2.1 What's broken

GCC 2.7.2 emits `.rodata` for switch jump tables, string literals, and
floating-point constants. The compiled rodata is referenced at specific virtual
addresses determined by the linker. BB2's `bb2.ld` interleaves
`asm/data/*.rodata*.o(.rodata)` segments with C-file `.o(.rodata)` segments
(see §3.1 for the actual layout). The `asm/data` blocks live at addresses
EARLIER than the matching C file's `.rodata` would end up.

Consequence: a function whose `switch` statement should emit a jump table at
address `0x800108CC` (because that's where the original game's binary has it)
can't have GCC emit the table from its `.c` file — the `.c` file's `.rodata`
section lives much later in the link, at a wrong address.

### 2.2 Why this exists

When splat was set up for BB2 (early in the project), it carved the original
game's `.rodata` section into `asm/data/*.rodata*.s` blocks based on address
ranges, NOT based on source-file ownership. Splat couldn't know which `.c`
file each piece of rodata belonged to without disassembly + cross-reference
analysis that hadn't been done yet. So it grouped rodata by address proximity
and emitted bridging asm.

The current workaround: 145 functions are committed as `replace_with_asmfile`
stubs (the C body is replaced verbatim by the target asm at compile time),
and 24 `asmfix.txt` rules bridge the one jtbl-infra function
(`replay_camera_rob_back_loose2`).

### 2.3 Why SOTN doesn't have this

SOTN's splat setup carved rodata into per-source-file segments from day one
(they had earlier disassembly evidence). They re-split rodata as evidence
surfaces about original TU boundaries (the "evidence-based re-attribution"
this project codifies). BB2 hasn't done that re-attribution at scale.

---

## 3. Current state inventory (measured 2026-06-09)

### 3.1 Rodata segments actually linked by `bb2.ld`

The 12 `asm/data/*.rodata*.o(.rodata)` segments interleaved with C-file
rodata in `bb2.ld`:

```
build/asm/data/800.rodata_pre.o(.rodata);                  /* tiny: ~5 strings */
build/asm/data/800.rodata_post.o(.rodata);                 /* 6 KB: 1 jtbl, 66 strs */
build/asm/data/101C.rodata_pre.o(.rodata);                 /* 5 KB: 7 jtbls, 4 strs ← 5-function multi-owner cluster */
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

### 3.2 The two BIG `asm/data/*.rodata.s` files — splat parents, NOT link inputs

`asm/data/800.rodata.s` (206 KB) and `asm/data/101C.rodata.s` (177 KB) are
**splat parent segments** that get auto-split at build time into the
`_pre`/`_post`/etc. variants in §3.1. They do NOT appear in `bb2.ld`
themselves. The 12 variants above are the actual link inputs.

```bash
$ grep -E "800\.rodata|101C\.rodata" bb2.ld | grep -v "_pre\|_post\|_text1a\|_c_pre\|_c2_post\|_main"
# (no output — the parent .s files are not separately linked)
```

This is the answer to what the prior draft listed as a Phase-0 known unknown.

### 3.3 Stub functions (`replace_with_asmfile`)

```
$ grep -c "replace_with_asmfile" asmfix.txt
145
```

Distribution from the 2026-06-08 sampling at `tmp/audit_19/`:

| Category | Count | Description |
|---|---|---|
| Empty body `{}` | 34 | Never decompiled |
| Minimal body (<5 statements) | 91 | Stub placeholder, possibly with `(void)argN;` casts only |
| Substantial body (5+ statements) | 19 | Deep-audited: NONE were free-matches; 3 mid-gap (30-50 dist), 3 large-gap (>100), 13 were placeholder stubs misidentified by stmt count |

**Cluster ownership of the 91 minimal-body stubs is currently UNKNOWN** —
Phase 0 (§5) determines which are rodata-blocked vs. just-undecompiled. That
classification is what bounds the actual scope of this project (see §1).

A future inventory pass should grep for the literal comment `Body replaced by
asmfix replace_with_asmfile` to distinguish stub placeholders from real
incomplete work.

### 3.4 jtbl-infra confirmed cases

```
$ grep -l "jtbl_" asmfix.txt | xargs grep -c "jtbl_"
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
   have their `.rodata` placed at addresses that match the original.

**You cannot retire one function at a time** because:
- Removing `replay_camera_rob_back_loose2`'s jtbl-infra rules without also
  emitting the other 4 jtbls produces link errors (the other 4 are still
  referenced from inside the asm/data block).
- Decompiling `special_camera_Exec` to pure C without coordinating with the
  other 4 (verified 2026-06-08 at WIP `memory/wip/special_camera_Exec/`) hits
  link failure because cc1 emits a NEW jtbl at `code6cac_b2.o(.rodata)`'s late
  address, while the external `jtbl_80010978` reference still points to the
  asm/data address.

### 4.2 The cascade problem

Restructuring `bb2.ld` shifts addresses for every rodata symbol downstream
of the edit point. Every later function's `lui+%hi / addiu+%lo` reference
picks up new immediate values — which means **TEXT changes**, not just rodata.

This is not an edge case. It is the entire mechanism. A cluster retirement
that successfully relocates one block's rodata to its proper C-file owner
will, in general, perturb the encoded immediates in every function whose
rodata sits downstream of the cluster.

The oracle still has to come out green. That means either:
- The relocation is byte-zero (rodata moves to a C-file segment that GCC
  emits at the SAME address it lived at in the asm/data block — possible
  when the C file's rodata can be placed in `bb2.ld` such that its absolute
  address lands where the old asm/data block was), or
- Every cascading downstream change comes out byte-identical too (rare —
  requires the downstream offsets to remain effectively unchanged).

**Implication for Phase 3**: do not treat a cluster retirement as a single
edit. It is a coordinated restructure of the linker script that has to
preserve every downstream rodata-relocation's resolved address.

### 4.3 Multi-block dependencies

Some blocks are independent (their owners are all in one .c file, easy
re-attribution). Others span multiple .c files and require coordinated work
across the source tree.

---

## 5. Project phases (per-cluster work)

Run these in roughly the order listed. Each phase MUST end with oracle SHA1 ==
`62efab4f73f992798c43e8c730aa43baa10bb4fa`.

### Phase 0: Full rodata block inventory

**Goal**: For each of the 12 `asm/data/*.rodata*.s` files linked by `bb2.ld`,
enumerate:
- What addresses it covers (rodata range)
- What jtbls + strings it contains (with addresses)
- Which functions reference each (cross-grep `asm/funcs/*.s` and `src/*.c`)
- Per-function: is the function currently a stub, pure-C-matched, or
  canonical-asm-authorized?

**Tooling**: write `tools/audit_rodata_blocks.py` that produces a CSV with
columns: `file, address, type, symbol, owner_function, owner_file,
owner_status`.

**Output**: `memory/project/rodata_block_inventory.csv` (committed — the
project's resume contract depends on this surviving a clean checkout).

**Why first**: every later phase is hypothetical until we know what's
actually in each block. In particular, the 91 minimal-body stubs' cluster
membership determines whether they're in this project's scope or the
engine queue's.

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
  Medium difficulty — match all the functions, ensure their combined
  `.rodata` ends up at the right address.
- **MULTI-FILE cluster** (like the 5-function `101C.rodata_pre.s` cluster):
  hardest. May require .c file restructuring to colocate the functions in
  one file, OR placing multiple .c file `.rodata` segments at specific
  addresses in `bb2.ld`.

**Output**: `memory/project/rodata_clusters.csv` (committed).

### Phase 1b: Pilot cluster

Before adopting the per-cluster recipe at scale, execute ONE end-to-end
cluster retirement as a pilot. Use the smallest SINGLE-FUNCTION cluster
identified in Phase 1.

The pilot proves:
- The Phase 3 cascade math (§5 Phase 3) actually works in practice
- The `bb2.ld` re-attribution can preserve downstream addresses
- The retirement commit + oracle gate is mechanically clean

If the pilot reveals that single-function retirements perturb downstream
encodings in ways the oracle catches, the project recipe needs revision
BEFORE scaling. Do not commit to Phase 2 work across multiple clusters
until the pilot lands.

### Phase 2: Per-stub decomp

For each stub function within a cluster targeted for retirement:
1. Read target asm (`asm/funcs/<func>.s`)
2. Read any available cross-reference evidence (e.g. comparable code from
   sibling functions, decomps of related games — pointers in §7.3)
3. m2c the target for an initial pure-C reconstruction
4. Iterate: edit source, `sandbox <func> --disable all`, apply pure-C levers
   (see §6 for the lever index)
5. Once sandbox distance → 0: `verify-oracle --rebuild` must keep SHA1 green
6. Cheat-reviewer PASS required ([[review-discipline-before-commit]])
7. Commit as `Match: <func> (<file>.c) — COMPLETED-C`

**Recipe for difficult cases**: if pure-C distance doesn't close after standard
levers, fall back to:
- WIP entry at `memory/wip/<func>/` (don't commit forced cheats)
- Document the floor + tested levers + next hypotheses
- Defer to a future session

**WIP escalation**: a function that accumulates ≥3 WIP sessions without
closing the gap should be escalated to the user with the candidate floor,
the tested levers, and a recommendation (continue / authorize as
canonical-asm / structural blocker requiring policy decision). WIP is a
checkpoint mechanism, not a long-term parking lot.

**Anti-pattern**: do NOT add register-asm pins, `__asm__` barriers, dead
stores, volatile coercion casts, OR-tree-shape-shifts, or any other expanded-
catalog cheat from `[[inline-asm-policy]]`. The cheat-reviewer blocks these.

### Phase 3: Per-cluster en-bloc re-attribution

Once all functions in a cluster are matched-in-pure-C **OR** when Phase 2 is
deliberately skipped via the sub-TU split pattern (see §5 Phase 3b below):

1. **Identify the target placement** for the cluster's combined `.rodata`.
   GCC emits each .c file's rodata in source order at one address. The
   cluster's .c file(s) need `.rodata` at the address where the asm/data
   block currently lives.

2. **Compute the cascade**: enumerate every rodata symbol downstream of the
   edit point. For each, predict whether the restructure changes its
   resolved address. If any downstream address changes, the restructure
   does NOT preserve the oracle as-is — you need to also move downstream
   C-file rodata segments to compensate, OR the restructure is unsafe.

3. **Restructure `bb2.ld`**: remove the asm/data block, replace with the
   .c file's rodata segment(s). If multiple .c files are involved, place
   them in order such that each segment lands at the right address.

4. **Verify**: `verify-oracle --rebuild` end-to-end. If oracle fails,
   identify which function's rodata moved + cascade-fix or revert.

5. **Retire** any remaining asmfix bridging rules (jtbl-infra) — they're
   no longer needed because cc1 now emits the jtbls in the right place.

6. **Commit** as `engine: retire <block> via en-bloc rodata re-attribution`.
   Body lists the functions retired, the asm/data block deleted, the
   bb2.ld changes, and the cascade impact (which downstream segments
   were verified unchanged or compensated).

### Phase 3b: Sub-TU split (the pattern that actually worked at scale)

Pattern that emerged during execution and ended up being the dominant
recipe (6 of the 12 retirements used it). It DECOUPLES rodata cleanup from
per-function decomp — preserves the project's core goal (block retirement)
while leaving stub-elimination to the engine queue.

**When to use over Phase 3 en-bloc**:
- Natural owner C file has fixed-address rodata content that can't move
  (file-scope `__asm__("glabel ...")` blocks for canonical-asm functions
  whose addresses are pinned, e.g. ings.c's `func_800164AC`).
- Symbols span multiple owner .c files whose bb2.ld positions are fixed
  (multi-file clusters like `101C.rodata_pre.s`, `101C.rodata_text1a_b_*.s`).
- Symbol has no detectable static owner (orphan strings like the 16KB
  filename list in `101C.rodata_text1a_a.s`).
- Symbol has no semantically meaningful C declaration form (the 4-byte
  zero pad in `101C.rodata_post.s`).

**Recipe**:
1. Create a new `.c` file in `src/` named to reflect its role — e.g.
   `src/ings_strings.c`, `src/text1a_filepaths.c`, `src/code6cac_b_rodata.c`,
   `src/code6cac_c_ab_pad.c`. The Makefile auto-discovers `src/*.c` so no
   Makefile edit needed.
2. Populate the file with `const u32[]` / `const char[]` declarations for
   each symbol from the asm/data block, byte-for-byte matching the original.
   Use `tools/extract_rodata_to_c.py <asm/data/block.s>` to auto-generate.
3. For symbols with no meaningful C declaration form (raw alignment pad,
   linker-inserted zero word), use file-scope `__asm__(".section .rodata\n
   .word ...\n")` to emit bytes directly through cc1's asm pass-through.
4. For leading-padding cases (asm/data block began with `.word 0` for
   alignment): declare `static const u32 _bb2_<id>_lead = 0;` at the TOP
   of the sub-TU so cc1 emits it first.
5. Standard bb2.ld edit: replace the
   `build/asm/data/<block>.o(.rodata);` line with
   `build/src/<sub_tu>.o(.rodata);` at the same position. Remove the
   block's `(.data)` and `(.bss)` references if present (zero-byte stubs
   — deleting is a no-op).
6. Delete `asm/data/<block>.s`.
7. `verify-oracle --rebuild`.

**What sub-TU split LEAVES on the table** (deferred to Phase B work):
- The owning functions remain INCOMPLETE in the queue (still
  `replace_with_asmfile` stubs, still carrying asmfix rules).
- `jtbl-infra` bridging rules (e.g. the 24 on
  `replay_camera_rob_back_loose2`) continue to function correctly — the
  external symbol they bridge to now lives in the sub-TU instead of the
  asm/data block, but the bridge mechanism is unchanged.

Per `§1 Scope discipline`: this is fine. The rodata-cleanup project's
success metric is block retirement, not stub elimination. Stub elimination
is engine-queue work that the cleanup unblocks but does not perform.

### Phase 4: Validate + iterate

After each cluster retirement, re-run the inventory (Phase 0) to refresh
state and pick the next cluster.

**Smallest-first strategy** (within Phase 1b's pilot, then ongoing):
tackle the trivial-rodata blocks first (`101C.rodata_pre_post.s` 2 KB / 0
jtbls; `101C.rodata_c_pre.s` ~50 bytes; etc.) to build pattern recognition
+ tooling familiarity before the heavier blocks
(`101C.rodata_text1a_a.s` 105 KB strings; `101C.rodata_text1a_b_post.s`
23 KB).

---

## 6. Pure-C lever index (for Phase 2 decomp work)

The levers below close the typical gap between m2c output and SHA1 match.
All are SOTN-sanctioned. Authoritative catalog is `.claude/rules/`; this
section is a quick map, not a replacement.

### 6.1 Register-allocation levers (when gap is reg-rename / scheduling)

- **`register-alloc-pure-c.md`** — Levers A (block-local var split), B
  (narrow integer type), C (loop-local precompute). The core RA playbook.
  Lever D (dead-param-assign) is FORBIDDEN — don't reach for it.
- **`narrow-byte-args-packed-call.md`** — `u8` params for byte-packed call
  args + hi/lo statement split.
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
- **`maspsx-label-nop-gate.md`** — add to allowlist for legitimate
  load-delay nop emission.

### 6.6 do-while-zero (last resort)

- **`do-while-zero-exception.md`** — NARROW exception for
  LABEL_OUTSIDE_LOOP_P / reorg.c invert-jump peephole. Annotate with
  `/* FAKE: ... */`. Used only after demonstrated lever-exhaustion +
  cheat-reviewer PASS.

### 6.7 Cheats — authoritative catalog in `[[inline-asm-policy]]`

Do not duplicate the cheat catalog here; it evolves. Consult
`.claude/rules/inline-asm-policy.md` for the current forbidden list. As of
2026-06-09 it covers register-asm pins, `extern volatile T G;` (narrow
carve-out in `legitimate-volatile-interrupt-touched.md`),
`*(volatile T *)&G` casts, dead-param-assign, dead-conditional-store,
dead-local-array, address-coerce, hardcoded-`$N` inline asm, operand-pinning
`move %0, %1`, scheduling barriers, OR-tree-shape-shift, chain-extender,
dead-goto label-pad, and DImode chain. All catch-net'd by
`engine/volatile_cheats.py` + `audit_asm_cheats.py` + the cheat-reviewer
agent.

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

Inventory each `asm/data/*.rodata*.s` file linked by `bb2.ld`:
- Parse the file for symbol definitions (`dlabel`, `dlabel jtbl_`,
  `.asciz`, etc.)
- For each symbol, find references in `asm/funcs/*.s` via grep
- For each referencing function, look up its current status from
  `engine/queue.json`
- Emit CSV: `block_file, symbol, type, owner_func, owner_file, owner_status`

#### `tools/cluster_rodata.py` (Phase 1)

Group rows from the inventory CSV into clusters by `block_file`. For each
cluster, emit:
- Function count
- Cross-file status (all same .c file, or multi-file)
- All-stub state (every owner is `replace_with_asmfile`), partial
  (mix of matched + stub), or all-matched (= ready for re-attribution)

#### `tools/re_attribute_rodata.py` (Phase 3 helper)

Given a cluster:
- Compute the bb2.ld changes needed
- Predict the downstream cascade (which other rodata symbols' resolved
  addresses change)
- Diff candidate vs current bb2.ld
- Verify the math (segment addresses + sizes add up; downstream
  symbols predicted to land at unchanged addresses)
- Don't apply automatically — write to a tmp file for review

This is the genuinely hard tool. It must model `ld`'s segment layout
precisely enough to predict the cascade before `verify-oracle` does.

### 7.3 Inspect target asm

```
asm/funcs/<func>.s     # disassembled target
asm/data/<block>.s     # rodata block (the problem)
bb2.ld                 # link order
splat.yaml             # splat config
```

### 7.4 Cross-reference evidence

When evidence about original TU boundaries is needed
(§5 Phase 1 / §8.1), candidate sources include:
- Adjacency in the original disassembly (rodata addresses adjacent to
  functions that reference each other)
- Single-owner jtbls (a jtbl referenced from exactly one function is
  strong evidence that function owned it in the original source)
- Sibling decomps of related games on the same engine (the BB2 engine
  was reused in Kengo on PS2 — if the comparable Kengo source is
  available, function names and TU groupings transfer; if not, this
  bullet is not applicable)
- Comparable already-matched siblings in this codebase where the
  splat grouping has been refined

The evidence threshold is **the source layout was probably X, here's
why**, not **moving the rodata makes the oracle green**. The latter is
forbidden (§8.1).

---

## 8. Anti-patterns (what NOT to do)

### 8.1 Speculative rodata reorder

If you change `bb2.ld` to make oracle pass WITHOUT corresponding evidence
of TU attribution, that's a structural cheat — same category as regfix
offset paperwork. Forbidden per `[[no-new-park-categories]]`.

Test: does your `bb2.ld` change reflect a discovery about original TU
boundaries (evidence-based) OR a desire to make the build work
(speculative)? Only the former is acceptable.

### 8.2 Force-matching individual functions in a multi-owner cluster

Verified 2026-06-08 with `special_camera_Exec` WIP — decomping one
function in a multi-owner cluster hits link failure. The whole cluster
needs en-bloc work. Don't commit individual stub→pure-C transitions if
the function shares a rodata block with other stubs.

Exception: SINGLE-FUNCTION clusters (where only one function owns the
rodata block) can be done individually.

### 8.3 Adding asmfix rules to "fix" link failures

If decomp produces a link failure, the answer is NOT a new asmfix rule.
The no-new-regfix-rules guard blocks net additions to asmfix.txt; the
escape hatch `[infra-rule: jtbl-infra]` is for newly-DISCOVERED
infrastructure (rare), not for bandaging incomplete decomp work.

### 8.4 Authorizing as canonical-asm to avoid the work

The 2026-06-08 audit found ZERO of 4 stub siblings qualified for
canonical-asm authorization. They lack S1-S8 signals + cross-references
suggest the original was C. Authorizing them anyway would be a
speculative attribution to force a SHA1 match — same category as the
regfix paperwork cheats.

### 8.5 Treating this project as a queue override without explicit policy

The engine queue is pre-ordered easiest-first by honest pure-C distance.
CLAUDE.md says: "no triage and no cherry-picking — work the top active
item." This project's per-cluster decision tree (§9) selects work by
rodata-block ownership, NOT by queue order. **That's a workflow override.**

Before driving this project, get an explicit policy decision from the
user:
- (a) Run this project on its own track, suspending the queue, OR
- (b) Treat this doc as reference material for when the queue surfaces
  rodata-blocked items, OR
- (c) Some specific carve-out (e.g. "work the queue normally, but when
  the top item is rodata-blocked, switch to this project's recipe").

Default in the absence of an explicit decision: (b). Don't drop the
queue without authorization.

---

## 9. Decision tree (resume from any state)

```
Are you starting cold? → Read this entire doc + .claude/rules/jtbl-rodata-split-infrastructure.md
                       → Read .claude/rules/no-new-park-categories.md
                       → Confirm queue-override policy with user (per §8.5)

Has Phase 0 (full inventory) been completed?
  Check: memory/project/rodata_block_inventory.csv exists
  NO  → Start with Phase 0. Write tools/audit_rodata_blocks.py + run it. Commit the CSV.
  YES → Go to next question.

Has Phase 1 (cluster classification) been completed?
  Check: memory/project/rodata_clusters.csv exists
  NO  → Run Phase 1 from the inventory data. Pick next cluster.
  YES → Go to next question.

Has Phase 1b (pilot cluster) been completed?
  Check: at least one asm/data/*.rodata*.o entry has been retired from bb2.ld
  NO  → Execute the pilot. Smallest single-function cluster from the inventory.
        Validate the cascade math. Land it before scaling.
  YES → Go to next question.

Pick a cluster to work on. Sort by these criteria (preferred order):
  1. SINGLE-FUNCTION clusters first (easiest)
  2. SINGLE-FILE clusters second (medium)
  3. Smallest MULTI-FILE cluster third (build pattern)
  4. Larger MULTI-FILE clusters last

For the chosen cluster:
  Are all owner functions already matched? (No stubs in the cluster)
    YES → Execute Phase 3 (en-bloc re-attribution). Skip Phase 2 for this cluster.
    NO  → Go to Phase 2. Match the stubs first.

Within Phase 2 for a chosen stub:
  Is the function jtbl-blocked? (Removing replace_with_asmfile → link error)
    YES → This stub depends on cluster cohorts. Match ALL cohorts first, then re-attribute en-bloc.
    NO  → Standard decomp work (see §6 levers).

After each stub match or cluster retirement:
  Run verify-oracle. SHA1 must == 62efab4f73f992798c43e8c730aa43baa10bb4fa.
  Commit. Update memory/project/rodata-cleanup-progress.md (§13.3).
```

---

## 10. Resume-from-anywhere contract

If you (future agent) are picking this up cold, here's the canonical resume
procedure:

1. **Read this doc end to end** (don't skip).
2. **Verify oracle**: `& tools/eng.ps1 verify-oracle` (no rebuild). SHA1
   must be `62efab4f73f992798c43e8c730aa43baa10bb4fa`. If not, stop and
   figure out what's broken before starting any rodata work.
3. **Read the related rules**:
   - `.claude/rules/jtbl-rodata-split-infrastructure.md`
   - `.claude/rules/no-new-park-categories.md`
   - `.claude/rules/canonical-asm-authorization-recipe.md`
   - `.claude/rules/inline-asm-policy.md`
   - `.claude/rules/no-new-regfix-rules.md`
4. **Read committed project artifacts** (these survive a clean checkout):
   - `memory/project/rodata_block_inventory.csv` (Phase 0 output)
   - `memory/project/rodata_clusters.csv` (Phase 1 output)
   - `memory/project/rodata-cleanup-progress.md` (per-cluster checkpoint log)
5. **Read prior WIP entries**:
   - `memory/wip/special_camera_Exec/` (the 2026-06-08 attempt at the
     `101C.rodata_pre.s` cluster — has the candidate.c that was reverted)
6. **Read prior audits if they survive** (tmp/ is gitignored — may be absent):
   - `tmp/4-sibling-audit-2026-06-08.md`
   - `tmp/audit_19/` (the 19-substantial-body audit results)
   If these are missing from your checkout, re-derive what you need from
   the current `asmfix.txt` + `engine/queue.json` state. The committed
   artifacts in step 4 are the authoritative ones.
7. **Confirm queue-override policy with user** (§8.5).
8. **Decide phase**: check §9's decision tree.
9. **Work the chosen phase**. Commit incrementally. Oracle green at every commit.

**Commit message conventions** (per `docs/COMMIT_CONVENTIONS.md`):
- `engine: ...` — tool/infrastructure changes (CSV, helper scripts, bb2.ld
  restructuring)
- `Match: <func> (<file>.c) — COMPLETED-C` — individual function match (Phase 2)
- `engine: retire <block> via en-bloc rodata re-attribution` — cluster
  completion (Phase 3)
- `cheat-cleanup: ...` — if you find pre-existing asmfix-cheat patterns
  alongside rodata work

---

## 11. Known unknowns + future research

### 11.1 Multi-file cluster recipe

Some clusters genuinely span multiple .c files (as the 5-function
`101C.rodata_pre.s` cluster does — code6cac_b.c + code6cac_b2.c). For
these, options include:
- (a) Restructure source files to colocate the cluster's functions in
  one .c file (heavy)
- (b) Use multiple `.o(.rodata)` segments in `bb2.ld` at adjacent
  addresses
- (c) Other approaches discovered during the pilot

**The pilot (Phase 1b) does NOT settle this** — the pilot tackles a
single-function cluster. The multi-file recipe needs its own validation
once Phase 2 reaches a multi-file cluster. The 5-function cluster above
is the first test bed.

### 11.2 String-only blocks

`101C.rodata_text1a_a.s` has 900 strings + 0 jtbls (105 KB). Strings are
generally easier to re-attribute than jtbls because they're typically
referenced as `lui+addiu` constant addresses, not as switch dispatch
targets. A string can move to any .c file's `.rodata` as long as the
address relocations update.

**Hypothesis**: string-only blocks may be retirable with simpler
re-attribution than jtbl-bearing blocks. Phase 0 / pilot should test
this.

### 11.3 The cascade-preservation question

Phase 3 step 2 asks the agent to predict which downstream rodata symbols'
addresses change after a cluster restructure. The Phase 1b pilot must
validate whether this prediction is mechanically tractable, or whether
it's an iterative trial-and-revert process. The answer shapes how
`tools/re_attribute_rodata.py` (§7.2) is designed.

---

## 12. Success metrics

### 12.1 Per-cluster

- All functions in cluster reach COMPLETED-C or
  COMPLETED-INLINE-ASM-CANONICAL
- The asm/data block file is deleted from repo + `bb2.ld`
- Oracle SHA1 stays green
- All asmfix bridging rules for the cluster retired

### 12.2 Project-wide (rodata-cleanup goal — the project's actual target)

- **0 `asm/data/*.rodata*.o(.rodata)` entries in `bb2.ld`**
- 0 jtbl-infra rules in `asmfix.txt`
- Oracle SHA1 still green (`62efab4f73f992798c43e8c730aa43baa10bb4fa`)

### 12.3 Stub-elimination (correlated but NOT the rodata-cleanup metric)

A function being a `replace_with_asmfile` stub does NOT automatically
make it this project's work. Per §1, stub-elimination is the engine
queue's domain. If this project retires every cluster but some
`replace_with_asmfile` rules remain on functions whose rodata was
non-blocking (or zero), those rules belong to the queue, not here.

### 12.4 Weighted progress (intermediate)

Tracked in `memory/project/rodata-cleanup-progress.md`. Per-cluster
checkpoint records:
- Block retired (name + bytes)
- Owner functions retired (count)
- Downstream cascade size (count of rodata symbols whose resolved
  address shifted, even if compensated)
- Asmfix rules retired (count)

Aggregate progress: **bytes of `asm/data/*.rodata*.s` removed from
`bb2.ld`** divided by total at project start. This is the legible
mid-project metric — distinguishes "retired one big block" from
"retired several trivial blocks."

### 12.5 Process discipline (per session)

- Each session ends with oracle green
- Each session produces ONE of: cluster retirement, stub match, WIP
  checkpoint, or inventory/tooling artifact
- Zero cheats committed (no register-asm pins, no volatile coercion, no
  catalog-class constructs per `[[inline-asm-policy]]`)

---

## 13. Related project work that compounds

### 13.1 The legitimate-volatile carve-out (2026-06-08)

Some functions may use `extern volatile T G;` correctly under the narrow
carve-out (`legitimate-volatile-interrupt-touched.md`). If a stub
function references an IRQ-touched global, the carve-out may apply.

### 13.2 The `{lbl#N}` migration (2026-06-08)

Drift-robust label slots in regfix/asmfix. The infrastructure benefits
all match work but doesn't directly retire stubs.

### 13.3 Independent work — NOT part of this project

The 5 WIP cluster (B844/C7A0/C86C/C97C/CBB0) is register-rotation /
scheduling walls, NOT rodata-blocked. Mentioned here only to flag the
contrast — those WIPs are queue work, not this project's.

---

## 14. Pointers for the next agent

When you're ready to start:

1. **Confirm the queue-override policy with the user** (§8.5). Do not
   skip this.

2. **First useful command**: write and run
   `tools/audit_rodata_blocks.py`, commit the output as
   `memory/project/rodata_block_inventory.csv`.

3. **Then run the pilot** (Phase 1b). The smallest single-function
   cluster. Land that retirement. The pilot is what proves the recipe
   actually works before scaling.

4. **Track in `memory/project/rodata-cleanup-progress.md`** — append a
   checkpoint per cluster retired. Lets future agents (and you-future)
   resume.

5. **When in doubt, default to "don't commit a cheat"**. WIP entries
   are fine for individual stubs. Stub→matched is the goal, but
   stub→WIP is acceptable progress within Phase 2. WIP escalation rule
   (§5 Phase 2): ≥3 sessions without closing → escalate to user.

This is real architectural work. It permanently retires a class of
project debt. Treat each cluster retirement as a real milestone.

---

## 15. Phase B — per-function follow-up (2026-06-09 onward)

Phase A retired all 12 asm/data rodata blocks via sub-TU split. The
following work is what `§12` strict reading still wants:

### 15.1 `replay_camera_rob_back_loose2` — jtbl-infra rule retirement (CURRENT)

**State**: function is already pure-C matched (verdict `C`, distance 0)
but carries 24 asmfix rules that bridge GCC's per-function emitted jtbl
to `jtbl_800108CC`. With Phase A complete, `jtbl_800108CC` is defined in
`src/code6cac_b_rodata.c` instead of the asm/data block — but the
asmfix bridge still applies because GCC still emits its own jtbl when
compiling the switch.

**Goal**: delete the 24 asmfix rules. The function reaches COMPLETED-C
(or COMPLETED-INLINE-ASM-CANONICAL if the cluster's structure forces it).

**Two candidate paths** (try in order):

(a) **Restructure the switch to use the external jtbl directly.** Replace
the C `switch` (which cc1 compiles into a private jtbl) with a
function-pointer table lookup that explicitly indexes into the existing
external `jtbl_800108CC` array. cc1 then has no reason to emit its own
jtbl, and the asmfix `delete_between .rodata .text` rule's target
disappears (no jtbl to delete) along with the rename rules. Risk: the
restructured C may have different codegen for the dispatch itself (the
asmfix `rename` rules also map case-target `.L<n>` labels — those
labels' addresses depend on cc1's compilation, so the restructure must
preserve them).

(b) **Canonical-asm authorization.** If (a) doesn't reach byte-match
within reasonable effort, propose `replay_camera_rob_back_loose2` for
authorized canonical-inline-asm per
`[[canonical-asm-authorization-recipe]]`. Requires user judgment call
+ adding to `inline_asm_canonical.txt`.

**Success criterion**:
- 0 rules for `replay_camera_rob_back_loose2` in `asmfix.txt` AND
- `queue done replay_camera_rob_back_loose2` succeeds (oracle SHA1 match) AND
- Function appears in `inline_asm_canonical.txt` if path (b) taken,
  or in neither queue nor canonical-asm if path (a) taken.

### 15.2 The 145 `replace_with_asmfile` stubs (ENGINE-QUEUE WORK)

**State**: unchanged from project start — 145 stubs in `asmfix.txt`. Some
were rodata-blocked (their rodata is now in sub-TUs); some weren't.

**Goal**: per `§1 Scope discipline`, this is NOT this project's success
metric. It's the engine queue's domain. After Phase A + the
`replay_camera_rob_back_loose2` close-out in §15.1, every stub is now
unblocked at the rodata level and can be worked through the normal
queue flow.

**No new tooling required** — `engine queue next` will surface stubs in
the standard easiest-first order. The cleanup made them tractable; it
does not perform them.

### 15.3 Final retire condition

The rodata-cleanup project closes out completely (strict §12 reading)
when:
- §15.1 lands (24 rules gone, function COMPLETED) AND
- `grep -c 'jtbl_' asmfix.txt` returns 0

The 145 stubs can keep their own pace afterward as engine-queue work.

---

**End of plan. Resume contract per §10.**
