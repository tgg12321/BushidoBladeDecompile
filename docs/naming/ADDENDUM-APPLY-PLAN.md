# Naming addendum wave — apply plan (task #23)

Prepared 2026-08-07 in an isolated worktree (branch `worktree-agent-a6952f8c8c9d2f4eb`);
**nothing here has touched main**. This document is the exact ordered procedure for the
future MAIN-tree session that applies it. Evidence base:
`docs/naming/libscan/ambiguous_resolutions.md` (+ per-row notes in
`docs/naming/libscan/rename_manifest.csv`); address restriction:
`docs/naming/libscan/addendum_addresses.txt`.

## What the addendum contains

**Part A — the wave proper (12 RENAMEs + 1 RESET, one `naming_wave.py` batch):**

| addr | today | becomes | evidence class |
|---|---|---|---|
| 0x80083A18 | `func_80083A18` (+`sys_Shutdown` aliases) | `SsInit` | RESOLVED_RELOC_TARGET |
| 0x80083B30 | `func_80083B30` (+`spu_Reset` alias) | `SsQuit` | RESOLVED_RELOC_TARGET |
| 0x80084948 | `func_80084948` (+`saTan4GaugeInit_wrapper` alias) | `_SsSndPlay` | RESOLVED_LAYOUT |
| 0x80084974 | `func_80084974` — **LIVE QUEUE ITEM (active, dist 28)** | `_SsSeqPlay` | RELOC_CHAIN_ID |
| 0x80085270 | `func_80085270` (+`spu_channel_reset` alias) | `_SsSndStop` | RELOC_CHAIN_ID |
| 0x800853F4 | `func_800853F4` (pure C, src/main.c:690) | `SsSeqStop` | RELOC_CHAIN_ID |
| 0x8008541C | `func_8008541C` (pure C, src/main.c:694) | `SsSepStop` | RESOLVED_LAYOUT |
| 0x80085F98 | `func_80085F98` (+`obj_ResetReverbMode` alias) | `SsUtReverbOff` | RESOLVED_RELOC_TARGET |
| 0x80085FB8 | `func_80085FB8` (+`mario_helper` alias) | `SsUtReverbOn` | RESOLVED_RELOC_TARGET |
| 0x800885AC | `func_800885AC` (+`spu_Init` aliases) | `SpuInit` | RESOLVED_RELOC_TARGET |
| 0x8008AD64 | `func_8008AD64` (+`spu_TransferDirect_capped` alias) | `SpuRead` | RESOLVED_RELOC_TARGET |
| 0x8008ADC4 | `func_8008ADC4` (+`spu_TransferData_capped` alias) | `SpuWrite` | RESOLVED_RELOC_TARGET |
| 0x800469A0 | `_SpuCallback` | **RESET** -> `func_800469A0` | REJECTED_UNREACHABLE |

Deliberate no-ops: **0x80046B20** keeps `snd_StopSystemSe` (both Sony candidates
rejected); **0x80082AC0** is already `ResetCallback` (applied by fe40a52b; the handoff's
"current name wrong" note was stale — verified against the placed LIBETC/INTR module and
the vector-dispatch body).

**Flag for the owner in the commit message:** the 0x800469A0 RESET *reverses one row of
the fe40a52b wave* (`_SpuCallback` was applied from a placement that fails the new
reachability filter). Not blocking under names-require-evidence — the evidence is cited —
but it must be visible, not buried.

**Part B — the 9 mid-function XDEF boundary fixes** (separate, owner-gated in part;
section below). **Part C — optional data-symbol names** (owner-optional; last section).

## Part A — ordered steps (main tree)

1. **Preconditions.**
   - Grinder STOPPED (`pwsh tools/grinder/status.ps1`), no locks; working tree clean;
     ONE tree writer for the whole procedure.
   - `& tools/wteng.ps1 main verify-oracle` — green before starting.
   - This branch's prep commits merged to main (they touch only `docs/naming/**`,
     `tools/libscan/manifest.py` — no build inputs).
2. **Regenerate the census** (WSL): `python3 docs/naming/build_census.py`.
   Expect exactly: 12 new RENAME rows at the Part-A addresses (from the annotated
   `rename_manifest.csv`), the `_SpuCallback` RESET row (targeted override), and
   `BIOS jumptable cross-check` evidence lines appearing on ~41 rows (the promoted
   `docs/naming/bios_decode/` tier — evidence-append only, no action changes).
   Review `git diff docs/naming/function-names.csv`; commit the regen
   (`naming: census regen — addendum resolutions + bios_decode tier`).
3. **Dry-run the wave** (WSL, repo root):
   ```
   python3 tools/naming_wave.py --from-census \
       --only-file docs/naming/libscan/addendum_addresses.txt \
       --manifest tmp/addendum_wave.json
   ```
   Review the manifest. Expected touchpoints worth eyeballing:
   - `engine/queue.json`: `func_80084974` -> `_SsSeqPlay` (live active item — key move);
   - `memory/grind/func_80084974/` ledger dir rename, if present;
   - `src/main.c` definitions of `func_800853F4`/`func_8008541C` and all call sites;
   - alias retirements in `named_syms.txt` (sys_Shutdown, spu_Reset,
     saTan4GaugeInit_wrapper, spu_channel_reset, obj_ResetReverbMode, mario_helper,
     spu_Init, spu_TransferDirect_capped, spu_TransferData_capped, and the RESET of
     `_SpuCallback` in `src/sound.c:193` + its `.s` file rename).
   Preflight collisions were checked at prep time: none of the 12 target names is bound
   anywhere in the symbol registries or rule files. If preflight disagrees, STOP and
   reconcile — do not `--drop-failing` through it.
4. **Apply:** same command + `--apply`.
5. **Gates, in order:**
   - `python3 tools/desync_audit.py` — the standing post-wave gate (hard-fail on
     INCLUDE_ASM/queue desyncs; baseline pre-existing debt was 33/2 at fe40a52b).
   - `& tools/wteng.ps1 main verify-oracle --rebuild` — the wave's claim is
     byte-neutrality; SHA1 must equal `62efab4f73f992798c43e8c730aa43baa10bb4fa`.
     Any drift = a missed surface: `git checkout` rollback, diagnose, redo.
   - `python3 tools/check_completion_integrity.py` — invariants hold.
   - `& tools/wteng.ps1 main queue status` — sanity: same counts, `_SsSeqPlay` present.
6. **Commit** (single commit, `naming:` prefix per `docs/COMMIT_CONVENTIONS.md`), body
   listing the 13 ops + the fe40a52b-reversal flag + oracle SHA1. Include the manifest
   counts from `tmp/addendum_wave.json`. Multi-line message via `git commit -F tmp/msg.txt`.
7. **Aftercare:** if `memory/grind/func_80084974/` was renamed, make sure it is IN the
   commit ([[grinder-stale-digest-uncommitted-ledger]] — an uncommitted ledger gets the
   function re-dispatched from a stale digest).

## Part B — the 9 mid-function XDEF boundary fixes (apply-time, sequenced)

Source: `docs/naming/libscan/boundary_fixes.md` (the table of nine + the two
span-larger-than-name cases). These move **build inputs**, not just symbols —
`naming_wave.py` cannot do them, and `make setup` is forbidden (`bb2.ld` is
hand-maintained). Each split is a surgical edit, oracle-verified individually.

**Queue state at prep time** (regenerate before executing): hosts still queued are
`func_800889D4` (active, hosts `_spu_FiDMA` + `_spu_Fr_`), `func_80086818` (active,
hosts `note2pitch`), `func_8008B488` (active, hosts `_spu_2pitch`). `func_8008AF9C`
(hosts `SpuRGetAllKeysStatus`) has left the queue since boundary_fixes.md was written.
`SsStart` (was func_80083E9C, hosts `SsStart2` + `SsSeqCalledTbyT`) and `setjmp` (hosts
`longjmp`) are not queued.

**Per-split procedure** (one split per oracle run):

1. Locate the host's `INCLUDE_ASM` line: `grep -rn "INCLUDE_ASM.*<host>" src/`.
2. Split `asm/funcs/<host>.s` at the XDEF address: the host file keeps the lines up to
   (not including) the boundary; create `asm/funcs/<SonyName>.s` with
   `glabel <SonyName>` + the remaining `/* addr */` lines moved VERBATIM + `endlabel`.
   Fix the host's `endlabel`. LF endings, edited via WSL.
3. Add an `INCLUDE_ASM` line for the new function immediately after the host's, same
   file, same section — link order must reproduce the original byte layout exactly.
4. Register the boundary for any future deliberate re-split: `symbol_addrs.txt` gets
   `<SonyName> = 0x<addr>; // type:func` (splat is NOT re-run now).
5. `verify-oracle --rebuild` — must be byte-identical (nothing moved, only file
   boundaries). Mismatch = link-order or span error; roll back.
6. Commit per split or per small batch (`fix:` or `naming:` prefix, citing
   boundary_fixes.md).

**Ordering and gates:**

1. Start with `_SendPAD` out of `FlushCache` (0x80078FF0/0x80079000) — smallest, no
   queue interaction, and it cures the FlushCache span-larger-than-name defect.
2. `SsStart2` + `SsSeqCalledTbyT` out of `SsStart` (two boundaries, one host).
3. `SpuRGetAllKeysStatus` out of `SpuSetCommonAttr` (0x8008AF9C — no longer queued).
4. **Grinder-quiet + queue regen required:** `note2pitch` out of `func_80086818`,
   `_spu_FiDMA` + `_spu_Fr_` out of `func_800889D4`, `_spu_2pitch` out of
   `func_8008B488`. Splitting a queued function changes its honest distance and the
   worklist contents: run `& tools/wteng.ps1 main queue regen` after, and expect the
   hosts' distances to drop (their spans shrink). Do NOT do these while the Grinder
   runs ([[grinder-clobbers-uncommitted-edits]]).
5. **OWNER-GATED:** `longjmp` out of `setjmp`. `setjmp` holds an
   `inline_asm_canonical.txt` authorization (`sp_manipulation`) — splitting changes
   what that authorization covers, so the authorization must be RE-DERIVED for both
   halves (canonical gate + owner sign-off), not inherited. Also note the commit-msg
   `detector_config_guard`: editing `inline_asm_canonical.txt` entries may need
   `[allow-config-symbol-loss]` with justification.
6. `DelDrv` (0x8008D060): the tail past 0x8008D070 is the all-zero padding blob, not a
   second function — defer unless the padding is being reclassified anyway (lowest
   value, lowest risk).

After all splits: regenerate the census (the universe changed); optionally re-run
`tools/libscan/manifest.py` **deliberately** (it will reclassify the nine as CONFIRM and
lose the pre-fix record — the committed snapshot + boundary_fixes.md are the history).

### APPLY-TIME RE-VERIFICATION (2026-08-10) — Part B is 5/9 OBSOLETE, 4/9 BLOCKED

A main-tree session took Part B up and re-verified every row against HEAD before
editing. **No split was applied.** The plan above was written 2026-08-07 against a
tree that has since moved; its per-split procedure is now actively unsafe for five
of the nine XDEFs. Findings:

**The decisive test** is not the `boundary_fixes.md` host span but whether
`asm/funcs/<host>.s` is a *build input at all*. It is one only if some `src/*.c`
carries `INCLUDE_ASM("asm/funcs", <host>)` — the Makefile does NOT wildcard
`asm/funcs/` (`LINKED_ASM_FUNCS := save_vc_ctrl` is the sole opt-in). Measured at
HEAD (208 INCLUDE_ASM hosts total):

| host | INCLUDE_ASM? | XDEFs | status |
|---|---|---|---|
| `FlushCache` | **no** | `_SendPAD` | obsolete — see below |
| `SsStart` | **no** | `SsStart2`, `SsSeqCalledTbyT` | obsolete — already C |
| `SpuSetCommonAttr` | **no** | `SpuRGetAllKeysStatus` | obsolete — already C |
| `setjmp` | **no** | `longjmp` | owner-gated (unchanged) |
| `func_80086818` | yes | `note2pitch` | genuine, blocked |
| `func_800889D4` | yes | `_spu_FiDMA`, `_spu_Fr_` | genuine, blocked |
| `func_8008B488` | yes | `_spu_2pitch` | genuine, blocked |

**Obsolete (5 XDEFs).** These hosts are no longer asm at all — their bytes come from
compiled C (or, for `FlushCache`, a whole-body canonical `__asm__`), so their
`asm/funcs/*.s` files are reference-only leftovers that no longer feed the build.
Three of the XDEFs already exist as correctly-named C functions:
`SsStart2` (`src/main.c:232`, static), `SsSeqCalledTbyT` (same TU, decompiled with
`SsStart`), `SpuRGetAllKeysStatus` (`src/main.c:3006`, static). For these, **step 3
of the per-split procedure — "add an INCLUDE_ASM line for the new function" — would
duplicate already-compiled code and break the link.** Do not run it.

**`_SendPAD` is additionally owner-gated, which the plan missed.** `FlushCache` holds
an `inline_asm_canonical.txt` authorization (line 63) whose justification is
*specifically* the dual-block shape: "the tail is unreachable from the `jr`, so this
cannot be expressed in C." All 14 words are emitted by a single whole-body `__asm__`
under one `glabel FlushCache` / `endlabel FlushCache` in `src/text1b_b.c:1736`.
Splitting `_SendPAD` out means splitting that authorized block in two, so the
authorization must be RE-DERIVED for both halves — exactly the reasoning that made
`setjmp`/`longjmp` owner-gated in step 5. Treat `FlushCache` the same way.

**Blocked (4 XDEFs).** `note2pitch`, `_spu_FiDMA`, `_spu_Fr_`, `_spu_2pitch` are the
only genuine build-input splits left, and all three hosts are live `active` queue
items (distances 249 / 206 / 423 at the time of writing). The plan's own step 4
requires `queue regen` afterwards; that session was under a standing instruction not
to run `queue regen` or perturb live queue state, and the Grinder was due to launch
immediately after. Applying a split without the regen leaves the host's queued
distance describing a span it no longer owns — a stale-digest hazard for the next
grind ([[grinder-stale-digest-uncommitted-ledger]]). These need a genuinely
Grinder-quiet window with regen permitted; they are a scope call for the owner, not
something to force through.

**Net:** Part B's remaining actionable work is 4 XDEFs across 3 queued hosts, plus
two owner-gated authorization re-derivations (`longjmp` out of `setjmp`, `_SendPAD`
out of `FlushCache`). The other three rows should be struck — the boundary they
describe has already been fixed by decompilation rather than by re-splitting.

## Part C — optional, owner-choice

- **Data names** `D_800A2D14` -> `_spu_transferCallback`, `D_800A2D10` ->
  `_spu_inTransfer` (evidence: `docs/naming/data_evidence/D_800A2D1{0,4}.md`). Data
  symbols are outside `naming_wave.py`; applying means editing the symbol registries +
  every use, then oracle-verify. Small, but it is hand-work — bundle with some other
  oracle run if wanted.
- **`func_80085FB8.s` span note:** after the rename, the `SsUtReverbOn.s` file still
  contains ~0x150 bytes of further un-glabeled LIBSND UT_* code past +0x20 (same class
  as Part B but with no XDEF at the inner boundaries — module locals). Recorded in the
  manifest row note; no action proposed.

## Rollback

Every step above is a normal git commit on main with the oracle as its gate;
`naming_wave.py --apply` refuses a dirty tree, so `git checkout .` (pre-commit) or
`git revert <commit>` (post-commit) is always clean. The wave manifest
(`tmp/addendum_wave.json`) lists every file it edited.
