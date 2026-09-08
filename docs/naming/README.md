# Function naming — census, taxonomy, and reset policy

This directory holds two related things:

1. **The naming CENSUS** (`function-names.csv`) — one row per function in the binary,
   recording what it is called today, **where that name came from**, how strong the
   evidence is, and what should happen to it. This is the standing campaign, opened
   2026-08-07. Described below.
2. **The naming PROPOSAL database** — the older analyzer output that proposed names for
   then-unnamed functions. Still live as an evidence source (the census cites it) and
   documented in its own section at the end of this file.

---

## The owner directive (2026-08-07)

> The existing names are **not trusted**. Kengo-based names previously misled agents
> badly. **A false-positive name costs more than an auto name.** Names failing an
> evidence bar will be **RESET** to `func_80XXXXXX` form.

The reasoning: an auto name (`func_80017200`) makes no claim, so it cannot mislead. A
wrong semantic name (`ang_hosei` on a file-I/O trampoline) actively sends the next agent
down a false trail, and every downstream doc, ledger, and commit message inherits the
error. So the burden of proof sits on the *name*, not on the doubt.

**Confirmed misleads that motivated the campaign** (all reproduced in the census):

| Name | What it actually is | Where recorded |
|---|---|---|
| `ang_hosei` | Marionation engine call, `break 0, 263` (0x107) — a **file-I/O trampoline**, not angle correction. Call sites `src/ings.c:141,143,170` pass file descriptors. | `inline_asm_canonical.txt:353`; `docs/grind/auth-packets-2026-08-06.md` batch 2 |
| `game_2d_CheckLifeGaugeNoDisp` | A **LIBGTE 3×3 matrix × vector multiply** leaf (`ctc2`/`mvmva`/`swc2`) — not a UI predicate. Zero general-purpose computation. | `inline_asm_canonical.txt:346`; same auth packet |
| `cpu_set_move_command_and_dir_for_no_action_2` | The **sole `jal` target of `_start`** (`asm/funcs/_start.s:42`). A crt0's final call is `main()`. | this census — see RENAME below |
| `katinuki_game_get_katinuki_max_num_*` (5) | `gpu_EnableDisplay`, `gpu_DisableDisplay`, and three 1-line wrappers. Size-only Kengo matches. | `named_syms.txt:271-275`; `kengo_name_decisions.csv` |
| `tslSmdSendVu1Code_*`, `Vu0SetLightColMatrix_*` | **VU0/VU1 are PS2-only.** These cannot name PS1 code at all. | `named_syms.txt:276,277,2493-2496` |

---

## Campaign status

**Phase 1 — census (done).** Read-only with respect to the build: `function-names.csv`
enumerates every function, attributes each name to a recorded evidence path, tiers it, and
recommends KEEP / RESET / RENAME. Regenerable; it reads the tree rather than remembering it.

**Phase 2 — the waves (reset wave + libscan wave LANDED; addendum PREPARED).**
`tools/naming_wave.py` applies the RESET/RENAME rows across every surface where a name is
a key, oracle-verified per batch, never hand-applied. Applied so far:

| Wave | Commit | What landed |
|---|---|---|
| phase-2 reset wave | `2651e2e5` (2026-08-07) | **300 RESET + 2 RENAME** (`main`, `gpu_EnableDisplay`/`gpu_DisableDisplay` family) — the SUSPECT/Kengo purge, byte-neutral |
| libscan wave | `fe40a52b` (2026-08-07) | **334 evidence-backed renames** from the PsyQ bit-verbatim library scan: 51 hard misnames corrected, 182 alias fixes, 73 fills, 28 style rows to bare Sony names (owner ruling). Census gained the `libscan-verbatim` -> VERIFIED tier |
| addendum wave | `ef3919bd` (2026-08-10) | **12 RENAMEs + 1 RESET** from the ambiguous-tie resolutions + reloc-chain bonus IDs (`docs/naming/libscan/addendum_addresses.txt`). Part B: 5/9 XDEF splits obsolete (`6df437b6`), 4 genuine splits applied same day (`113abc0e`/`b6c73b0f`/`4ffc63aa`); only the 2 owner-gated re-derivations remain — see `docs/naming/ADDENDUM-APPLY-PLAN.md` |
| apiscan wave | `naming: apiscan wave` (2026-09-07) | **21 RENAMEs** from the API-restatement class (`docs/naming/apiscan/`): 10 fills of auto glabels + 11 retirements of meaningless/wrong aliases (`pad_file_*`, `bios_helper`, `irq_helper`, `obj_helper`, `sys_helper`, `mode_helper`). Names restate the VERIFIED BIOS/PsyQ calls + literal device strings in the body; every row re-derived by a fresh default-refute verifier before landing. Census gained the `apiscan-restatement` -> CORROBORATED origin |

---

## The universe: what counts as a function

**1,429 unique functions** (corrected 2026-08-07 by the task-#19 duplicate survey —
`docs/TASK19-PROTO-ANALYSIS.md`). Derivation, because the number is quoted
inconsistently elsewhere:

| Source | Count | Note |
|---|---|---|
| `asm/funcs/*.s` files | 1,437 (2026-08-07 snapshot — the duplicate pairs were subsequently deleted and asm-until-matched moves the file count both ways; regenerate the census before quoting) | splat's per-function split — the authoritative enumeration (post-wave count) |
| minus `D_8007E08C.s` | 1,436 | data-as-code blob, not a function |
| minus 7 stale duplicate pairs | **1,429** | 2 same-glabel pairs (`cdrom_FramesToBcd.s`/`func_800806A4.s`, `stage_InitCollision.s`/`func_8003F274.s`) **plus 5 same-address pairs invisible to glabel scans** (`cpu_get_dist`/`func_8003032C`, `cpu_set_move_command_and_dir`/`func_80030A2C`, `cpu_check_same_dir_timer`/`func_8003339C`, `replay_camera_Init`/`func_80036D98`, `special_camera_get_rot_dir`/`func_80037348`). All 7 pairs instruction-identical, none INCLUDE_ASM-referenced; worth deleting separately. |

`engine/queue.json` is *not* a universe source: it lists only functions still
carrying a cheat (incomplete-only).

### A name lives in up to three layers

This is the subtlety that makes a naive census wrong. A function can be called one thing
by the linker and another thing by a human reader:

1. **`glabel`** in `asm/funcs/<name>.s` — linker-authoritative.
2. **The C definition** in `src/*.c` — what a reader of the source sees.
3. **Alias registry lines** in `named_syms.txt` / `symbol_addrs.txt`, conventionally
   suffixed `<name>_<ADDR>`.

**961 functions whose `glabel` is still `func_XXXXXXXX` nevertheless carry a semantic
alias** at the same address. A census keyed only on `glabel` would have mislabelled all
961 as harmless AUTO. The census is therefore keyed on **address**, records all three
layers (`glabel`, `current_name`, `aliases`, `name_layer`), and tiers the **strongest
semantic claim** visible to a reader — because that is the claim that can mislead.

---

## Confidence tiers

Counts below are **current** — the census is regenerable and reflects the tree as it stands,
including whatever the phase-2 wave has already applied. See "Wave progress" below.

| Tier | Count (pre-wave) | Meaning | Default action |
|---|---:|---|---|
| **VERIFIED** | 10 (now ~350: libscan) | The name is **fact**: a bit-verbatim PsyQ module XDEF (`libscan-verbatim`), an in-binary string, a hardware-defined role, or a PsyQ syscall signature. | KEEP |
| **CORROBORATED** | 86 | Body behaviour and/or call graph affirmatively agrees with the name's claim, with a citation; no contradicting evidence. | KEEP |
| **INFERRED** | 907 | Plausible from behaviour but unreviewed, or a generic/descriptive name whose claim is weak. **Not defended — just not contradicted.** | KEEP (review) |
| **SUSPECT** | 304 | Kengo-derived provenance, **or** a recorded contradiction, **or** the claim conflicts with observed behaviour. | **RESET** (applied by the reset wave) |
| **AUTO** | 129 | `func_80XXXXXX` / splat-generated. No claim, no risk. | KEEP |

The counts are the 2026-08-07 **pre-wave** census snapshot, kept as the campaign record;
the census is regenerable and self-heals after each wave — **regenerate before quoting**.

### What earns VERIFIED

**The libscan chain (added 2026-08-07, now the largest VERIFIED source).** A PsyQ 4.0
`.LIB` module whose entire `.text` is bit-identical to a span of the shipped EXE (reloc
fields masked) carries Sony's own `XDEF` records; `module_placement_vaddr + xdef_offset`
is the exported symbol's BB2 address. Dispositive byte evidence, not inference. Method +
artifacts: `tools/libscan/` + `docs/naming/libscan/` (the manifest makes every claim
auditable). Two acceptance filters (reachability, reloc-target consistency) were added
after the ambiguous-tie post-mortem — `docs/naming/libscan/ambiguous_resolutions.md`.

**A second, independent chain — the BIOS jumptable decode** (`docs/naming/bios_decode/`,
promoted 2026-08-07): a BIOS trampoline's `$t1` index against the A0/B0/C0 jumptable is
machine fact; `build_census.py` appends it as a cross-check evidence line. It corrected
`FlushCache`/`WaitEvent`/`DelDrv` in the wave and **retired the old 27-item `bios_local`
proximity heuristic (26/27 wrong — nothing may cite it)**.

(The MOVOVL.EXE overlay scan is archived at `docs/naming/movovl_scan/` — 91.2% verbatim
Sony library text; a 264-symbol overlay map if the overlay is ever decompiled.)

The three original (pre-libscan) evidence kinds, all dispositive:

- **Hardware-defined role (2).** `_start` at `0x800836EC` per the executable header, with
  crt0 shape (BSS zero, `$sp`/`$gp`/`$fp` setup, `jal main`, `break 0, 1`); and `main`,
  identified as the sole `jal` target of `_start` other than `bios_InitHeap`.
- **In-binary self-identifying string (6).** The function loads a rodata string equal to
  its own name — PsyQ's debug/assert strings, embedded in the routine itself.
  `gpu_SetDispMask` loads `D_80015F04` = `"SetDispMask(%d)...\n"`. Also `gpu_DrawSync`,
  `gpu_LoadImage`, `gpu_StoreImage`, `gpu_ClearOTag`, `gpu_DrawOTag`.
- **PsyQ syscall signature (2).** `EnterCriticalSection` / `ExitCriticalSection` — BIOS
  trampoline shape, syscalls #1/#2.

**VERIFIED outranks Kengo provenance.** Several of these names also appear in
`kengo_matches.csv`; that only means Kengo reused the same PsyQ SDK name. In-binary fact
is not demotable by suspicion. (The census applies this precedence explicitly — an
earlier draft let the SUSPECT rule silently overwrite VERIFIED rows.)

### What makes something SUSPECT

304 rows, from three independent triggers:

- **Kengo-derived provenance (242).** The name traces to `kengo_matches.csv`, to
  `kengo_name_decisions.csv`, or to a Kengo-banded section of the
  `tools/rename_funcs.py` rename map. Per the owner directive this is *by itself*
  disqualifying, regardless of the match's stated confidence. Includes **disambiguated
  variants** (`foo_2`, `foo_3`, `fooB`, `Disp..._A`) — re-using one Kengo claim at a
  second address is the single most common false-positive shape, per this directory's
  own prior README.
- **PS2-only symbol families (13).** `tsl*`, `su[A-Z]*`, `Vu0*`, `Vu1*` are Kengo/PS2
  engine names. VU0/VU1 hardware does not exist on PS1, so these cannot be right.
- **Recorded contradiction (49).** A `MISNAMED` / `wrong` flag in `named_syms.txt` or an
  entry in `MISNOMERS.md` condemning that specific symbol.

**A misname flag condemns only the symbol it is attached to.** Where the flag sits on a
*sibling* alias, the currently-displayed name is frequently the **correction** that flag
points to — e.g. `syscall_wrapper_break_800164F8` is flagged MISNAMED and
`breakpoint_trap_loop` is its fix. Demoting the fix because of its own flag would be
backwards. Those rows carry a `CONTEXT:` note instead of a demotion.

### Origin taxonomy

Every non-AUTO name is attributed to a recorded evidence path:

| Origin | Count | Tier it implies |
|---|---:|---|
| `naming-analyzer(proposals_resolved / proposals / residual_named)` | 860 | INFERRED, or CORROBORATED for `manual_re` + confidence=high |
| `kengo-derived` (incl. PS2-only prefixes) | 265 | SUSPECT |
| `splat-auto` | 129 | AUTO |
| `misname-flag` | 49 | SUSPECT |
| `psyq-idiom-scan` (`known_psyq_stdlib.txt` body-shape match) | 39 | CORROBORATED |
| `apiscan-restatement` (name restates VERIFIED library calls + literal strings; `docs/naming/apiscan/`) | 21 | CORROBORATED |
| `psyq-family-prefix` | 38 | INFERRED — prefix only |
| `legacy-renamer-map(verified band)` | 29 | CORROBORATED |
| `unattributed` | 16 | INFERRED — provenance unknown |
| `in-binary-string` / `psyq-signature` / `hardware-role` | 10 | VERIFIED |

Two deliberate judgements, both erring toward *less* confidence:

- **A PsyQ family prefix alone does not corroborate anything.** `spu_WriteReg`,
  `sys_VSync`, `gpu_SendPacket` assert a specific SDK entry point on the strength of
  their own prefix. Those are INFERRED, not CORROBORATED. The exception is the
  `cdrom_*` cluster, whose bodies were spot-checked against the
  `g_cd_index_reg`/`g_cd_irq_reg`/`g_cd_dma_ctrl` register-pointer block
  (`kengo-rename-audit-2026-07-13.md`, apply record).
- **Recorded provenance outranks name shape.** Where an analyzer CSV records where a
  name actually came from, that wins over what the name looks like — so a
  `sys_`-prefixed name invented by the analyzer is attributed to the analyzer and
  carries a note that its prefix asserts more than its evidence supports.

The 16 **unattributed** names (`obj_Init*`, `seq_*`, `player_*`, `game_Frame*`,
`game_Cleanup`, `debug_printf`) all entered in one commit — `4c7390cc`, 2026-04-10,
"readability: rename 29 func_ functions to semantic names", Opus-authored, recording no
per-function evidence. Notably the *GPU/BIOS* names from that same commit later proved
VERIFIED by in-binary strings, which partially validates the pass's method; the
`obj_`/`seq_`/`player_` names have no such confirmation. Per the standing
`verify-opus-handoff-claims` directive these need independent re-derivation, but they
carry no contradiction, so they are INFERRED rather than SUSPECT.

---

## Recommended actions

Historical (pre-wave) totals: KEEP 1,132 / RESET 304 / RENAME 1. **Both waves have
landed** (see the wave table above), so a fresh census run reports the residue, not these
numbers. Outstanding as of 2026-08-07: the **addendum wave** (12 RENAMEs + 1 RESET,
prepared — `docs/naming/ADDENDUM-APPLY-PLAN.md`) and the 9 mid-function XDEF splat
boundary fixes (`docs/naming/libscan/boundary_fixes.md`, apply-time work, owner-gated
sequencing in the same plan).

### Wave progress

Because the census is regenerated from the tree, applied rows simply leave the action
set — a re-run is the progress report. Regenerate before quoting any figure.

**The RENAME (applied):** `0x80017200`
`cpu_set_move_command_and_dir_for_no_action_2` → **`main`**. `asm/funcs/_start.s` calls it and
it is the crt0's only call other than `bios_InitHeap`; a crt0's final call is `main()`. The
old name had no basis: `kengo_matches.csv` matched the address to `gnd_land_hit_char_tsuba`
with `combined_score=0.00`, which is neither the applied name nor a usable signal. Compiling a
function *literally named* `main` was the wave's highest-risk step — GCC's
`expand_main_function()` injects `jal __main` in some configurations — so it was **measured,
not assumed**: this cc1 emits byte-identical output under either name, zero `__main`
references.

### The reset set (304) — APPLIED (`2651e2e5`)

Was: all remaining SUSPECT rows. Resetting restores `func_80XXXXXX` at the address and
**deletes the semantic claim**, including its `named_syms.txt` alias lines; misname-flag
text is **preserved as a comment** on the reset symbol so the finding is not lost with
the name (the applied wave did this — see the `/* RENAME 0x…: retired name … */`
comments in `named_syms.txt`).

---

## ⚠ The rename cascade — why phase 2 must be tooled, never hand-edited

Function names are not cosmetic in this tree. They are **keys** into the build pipeline.
A reset wave touching the remaining 304 rows (295 distinct names) hits every one of these surfaces:

| Surface | SUSPECT names referenced | Why it breaks |
|---|---:|---|
| `named_syms.txt` | 187 | the alias registry itself |
| `engine/queue.json` | 92 | the worklist keys on function name |
| `regfix.txt` | 67 | **rules are keyed by function name** — a desync silently drops the rule |
| **`sdata_funcs.txt`** | **51** | **GP-relative addressing breaks if these desync** (`tools/rename_funcs.py` documents this explicitly) |
| `asmfix.txt` | 18 | as regfix |
| `sdata_exclude.txt` | 17 | as sdata |
| `inline_asm_canonical.txt` | 11 | canonical-asm authorizations are name-keyed; a desync un-authorizes a function |
| `undefined_syms_auto.txt` | 9 | splat symbol resolution |
| `maspsx_label_nop_funcs.txt`, `regfix_stage2.txt` | 3 each | pipeline gate lists |
| `bb2.ld`, `volatile_extern_allowlist.txt` | 2 each | `bb2.ld` is **hand-maintained** — see below |
| `src/*.c` | 244 | definitions and call sites |
| `include/*.h` | 139 | declarations |
| `asm/funcs/*.s` | 281 | `glabel` / `endlabel` — **and the filename must match the `INCLUDE_ASM` argument** |
| `memory/` (719 files) | — | 227 `grind/` + 89 `wip/` per-function ledgers, directory-named by function |

Non-negotiables for the phase-2 tool:

1. **Oracle verification on every batch.** A rename is symbol-level only, so the full
   build+link SHA1 must still equal `62efab4f73f992798c43e8c730aa43baa10bb4fa`. Any
   drift means a surface was missed. Batch, verify, roll back on mismatch.
2. **Never run `make setup`.** `bb2.ld` is hand-maintained (CLAUDE.md, `splat.yaml`).
3. **LF line endings** on every build file touched.
4. **`glabel` and the `.s` filename must move together** with the `INCLUDE_ASM`
   argument, or the linker gets an undefined symbol. `tools/rename_funcs.py` renames
   `jal` sites but **skips glabels** — a naive reuse of it will produce exactly this
   break (the 2026-07-13 audit deleted 8 known-wrong entries from that map specifically
   so a blind `--apply` could not reintroduce them).
5. **Do not run it while the Grinder is running** — foreign dirt in the working tree
   makes the driver's scope check discard sessions
   ([[grinder-clobbers-uncommitted-edits]]).

---

## Rulings that override the census: `action-overrides.csv`

`function-names.csv` is **generated**, so a hand-edit to an `action` or
`proposed_name` cell is silently reverted by the next `build_census.py` run.
Rulings that differ from what the census derives therefore live in
[`action-overrides.csv`](action-overrides.csv), which `tools/naming_wave.py`
applies **on top of** the census at wave time. It is keyed by address, or by
name for rows whose census address is unusable.

**If you are regenerating the census, read that file first** — it is the only
record of decisions the generator cannot re-derive, and a regen will not
reproduce them. Current entries (owner-delegated rulings, 2026-08-07):
`save_vc_ctrl` EXCLUDED from the wave, and the `gpu_EnableDisplay` /
`gpu_DisableDisplay` pair re-classed from RESET to RENAME. Each row carries its
own rationale. If `build_census.py` ever learns a rule directly, drop the
corresponding row.

---

## Regenerating the census

```bash
python3 docs/naming/build_census.py     # rewrites docs/naming/function-names.csv
```

Read-only; it writes exactly one file. Inputs: `asm/funcs/*.s`, `src/*.c`,
`named_syms.txt`, `symbol_addrs.txt`, `kengo_matches.csv`, `kengo_name_decisions.csv`,
`tools/rename_funcs.py`, `known_psyq_stdlib.txt`, `inline_asm_canonical.txt`,
`engine/queue.json`, `disc/SLUS_006.63` (string extraction), the `docs/naming/*.csv`
proposal tables, and `git log -S` for otherwise-unattributed names.

### `function-names.csv` schema

| Column | Meaning |
|---|---|
| `address` | `0x8XXXXXXX` — the primary key |
| `current_name` | the strongest semantic name a reader sees |
| `glabel` | the linker-authoritative symbol in `asm/funcs/` |
| `name_layer` | which layer `current_name` came from |
| `aliases` | all semantic aliases at this address |
| `insns` | instruction count |
| `src_location` | best-effort `src/<file>:<line>` (heuristic; may point at a declaration) |
| `origin` | evidence path the name came from |
| `tier` | VERIFIED / CORROBORATED / INFERRED / SUSPECT / AUTO |
| `evidence` | short citation chain |
| `action` | KEEP / RESET / RENAME |
| `proposed_name` | for RENAME rows |
| `queued` | function is still on `engine/queue.json` |

---

# Naming Proposal Database (pre-existing — retained)

Evidence-backed function-name proposals for the unnamed `func_XXXXXXXX` functions. The
initial 1217-function analyzer pass was fully resolved as of 2026-07-13 (see
`proposals_audit_2026-07-13.md`); this dir stays as the analyzer's regenerable output for
future passes and as the archive of applied/superseded proposals. **The census above now
supersedes this as the primary naming worklist**, and cites these tables as an evidence
source.

## State as of 2026-07-13

- **High confidence:** 0 pending (all 10 resolved).
- **Medium confidence:** 0 pending (all 111 resolved).
- **Low confidence still pending:** 23 (weak-evidence `_local_`/`_func_` stubs).
- Full audit: `proposals_audit_2026-07-13.md`; resolved rows: `proposals_resolved.csv`.

## Files

- `proposals.csv` — canonical machine-readable proposal table
  (address, current_name, proposed_name, confidence, evidence_summary, evidence_detail_file)
- `proposals_high_confidence.md` / `proposals_medium_confidence.md` — review tables
- `proposals_resolved.csv` — rows already applied (or applied under a different name)
- `proposals_audit_2026-07-13.md` — applied-as-proposed / applied-differently / pending counts
- `psyq_library_matches.md` — PsyQ stdlib / BIOS jumptable / syscall wrapper proposals
- `subsystem_clusters.md` — call-graph cluster analysis
- `data_symbols_quick_wins.md` — naming hints for `D_*` symbols by access pattern
- `methodology.md` — analyzer design, evidence kinds, scoring, caveats
- `MISNOMERS.md` — names demonstrated wrong by body analysis
- `LEGACY_RENAMER_AUDIT.md`, `kengo-rename-audit-2026-07-13.md` — rename-map audits
- `evidence/<func_name>.md` — per-function evidence detail

## Regenerating the proposal tables

```bash
python3 tools/propose_function_names.py            # rebuild proposals.csv + evidence/
python3 tools/render_naming_docs.py                # rebuild markdown views
python3 tools/diff_naming_proposals.py OLD.csv NEW.csv
```

## Standing caveats

- **Size-only Kengo matches were the #1 source of false-positive renames**
  (`katinuki_game_get_katinuki_max_num_*`, 5 functions). The census now treats *all*
  Kengo-derived names as SUSPECT.
- **Address-suffixed names indicate uncertainty.** A proposed `foo_8001ABCD` means "best
  candidate, not a promise." Drop the suffix only after body inspection.
- **Per-function evidence files are the source of truth** for why a proposal exists.

## Sony-static names (owner ruling 2026-08-18 c)

MODULE_LOCAL_STATIC identities from verbatim-placed PsyQ modules are applied
ONLY after an adversarial default-refute verification pass (see
manifest_report.md addendum + naming-verification-2026-08-18.md). The
verified names live in rename_manifest.csv `proposed_name`; the census
re-derives the RENAME actions from there on regen.
