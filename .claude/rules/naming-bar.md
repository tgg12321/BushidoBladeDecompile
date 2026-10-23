---
name: naming-bar
paths: [".claude/rules/naming-bar.md", ".claude/agents/naming-reviewer.md", "docs/naming/**", "tools/naming_wave.py", "tools/data_wave.py", "tools/naming_keycheck.py"]
description: "Owner ruling Q111 (2026-10-04, step 0 answers 2026-10-07): the Phase 3 naming bar, SOTN-equivalent. A name lands when it explains from the code across every use and a fresh naming-reviewer passes it; evidence classes settle conflicts, they do not gate. Neutral names beat guesses. Waves are tooled and identifier-only."
metadata:
  type: rule
  tier: hygiene
---

# The naming bar

Applies to every name a naming wave lands or keeps: functions, globals, types, members,
locals, parameters, file names. It never decides a function's completion state
([[completion-bar]] does).

## N1 — the SOTN bar

A name is admissible when a reader can explain it from the code and a fresh `naming-reviewer`
agrees. Every read, write and call of the named thing bears the name out:
- **function:** the verb covers the whole effect of the body (its writes, calls and return);
  the noun is what the body touches; nothing it does contradicts the name.
- **global / member:** every access, in every TU and in the asm of `INCLUDE_ASM` / canonical
  bodies, agrees in role, width and lifetime.
- **type:** one layout; every named field is named by its accesses.
- **local / parameter:** every use in its function bears it out; a `/* FAKE */` or no-purpose
  local keeps a neutral name ([[completion-bar]] item 3).

One contradicting use fails the name.

## N2 — neutral beats a guess

Lacking evidence, keep the neutral name (`func_8XXXXXXX`, `D_8XXXXXXX`, `Unk<addr>Rec`,
`unk_XX`). A neutral name makes no claim; a false one misleads.

## N3 — evidence: conflict order, not a gate

No class is required. Where the sources disagree, the higher one wins, and a conflict the
order does not settle fails the row:
1. Sony's own name: verbatim PsyQ XDEF / libscan-xref, in-binary string, hardware role.
2. SOTN or psyz name for the same PsyQ routine or data (BB2 libs 4.0,
   `docs/naming/libscan/psyq_versions.txt`); a Sony static only after its verbatim placement
   is shown.
3. Explained from the code: computation, typed restatement, or call-graph role (every caller
   uses it the same way, and its callees and data establish the subsystem).
   Runtime: [[naming-runtime-evidence]].

A string the function prints is context, not identity. Never evidence: a family prefix
alone, a name another rename invented, comments, earlier agent claims not re-derived,
size-only matches. Library code keeps Sony's spelling.

## N4 — prefixes and specificity

A subsystem prefix (`camera_`, `model_`, `stage_`, `snd_`, `cd_`...) must itself be explained
(callees, data, or callers of that subsystem). Claim no more than the code shows:
`camera_Update`, not `camera_UpdateReplayTilt`, when "replay" and "tilt" are guesses.

## N5 — existing names

An existing INFERRED or unattributed name meets the same bar. If it fails N1, it is RESET
or renamed, not kept for history.

## N6 — file names

An address-named game TU (`9F9C.c`) takes a subsystem name only when its whole content is that
subsystem: the admitted names and data of every function in it agree. A mixed file keeps its
ROM-offset name (most large game files hold several original TUs the bytes cannot
separate). The move uses `tools/move_tu.py`, and the top comment keeps its boundary evidence.

## The wave

1. **Manifest:** a miner writes one row per op: address, kind, old -> new, every use with its
   `file:line`, and the counter-evidence considered. Function rows go in
   `docs/naming/phase3/<wave>/func_manifest.csv` with `evidence_class` `sotn-review` (census
   origin `sotn-review`, tier CORROBORATED), `RESET` rows `reset-contradicted`, and an N5 KEEP
   (an existing name that passes) as a `sotn-review` row with `proposed_name` = the current name.
   It upgrades the census tier and is not a pair. Data rows go in `data_manifest.csv` for
   `tools/data_wave.py`. Type, member and local rows go in `type_manifest.csv`,
   `member_manifest.csv`, `local_manifest.csv` (data_manifest's columns); their CONFIRMed pairs
   (`Old=New`, `T.m=n`, `f:l=n`) are the wave's `pairs.txt`.
2. **Review:** a fresh `naming-reviewer` checks each manifest. It may set `verdict` CONFIRM only
   on rows it PASSes. Rows it FAILs are dropped or re-mined, never patched by hand.
   Neutral resets: [[naming-bulk-reset]] (Q119, Q120).
3. **Census:** `python3 docs/naming/build_census.py`.
4. **Apply:** with a clean tree and the Grinder stopped, run `tools/naming_wave.py --from-census
   --only <addrs>` (dry run, then `--apply`), commit it temporarily (data_wave refuses
   paths already dirty), run `tools/data_wave.py --manifest-csv <csv> --apply`, then
   `git reset --soft HEAD~1` so the wave lands as one commit. The pairs are every `old_names` /
   `olds` entry of the two tools' `--manifest` JSON (an auto name and its alias may share a new
   name).
   Types, members and locals are edited in C (`T.m=n`: the declaration of `m` in `T` and its
   `.`/`->` accesses, via `tools/naming_member_apply.py <pairs.txt>`, which lists lines to
   hand-edit; `f:l=n`: one binding in the unique definition of `f`). Then the comment pass,
   `python3 tools/naming_keycheck.py --pairs <global pairs> --sub-comments`, and hand-edit,
   in the wave, every comment still naming an old member or local.
5. **Gates:** `verify-oracle --rebuild` (SHA1 == oracle); `tus-check`;
   `tools/check_completion_integrity.py`; `engine test`; then stage the whole wave (`git add -A`)
   and run `python3 tools/naming_keycheck.py --from-manifests <naming_wave.json>,<data_wave.json>
   --pairs-file <pairs.txt>`. It checks what a commit would store (an unstaged or intent-to-add
   path fails), failing closed. Every changed, added, deleted or renamed path must be the base
   with the pairs substituted, using the wave tools' own substitutions:
   - C and header tokens, and each comment in its place, change only by the pairs, as is or as
     `tools/format.py` lands them (a longer name can re-wrap or hoist a comment). Any other moved
     comment, or a `#define NAME(` spacing change, fails.
   - Each moved `layer2.jsonl` must equal naming_wave's retarget of the old record. Every other
     moved ledger file is unchanged, and nothing else under `memory/` changes.
   - The census CSV must equal a fresh `build_census.py` run (working tree only, after the
     rebuild: it reads `build/bb2.map`). `docs/naming/phase3/**` is the
     only free path. Registries change line by line, only into what naming_wave / data_wave
     emit for that line. Other text files (gate lists, tools) change only by the pairs.
   - `.claude/**`, `movovl/**` (not oracle-covered) and the tool never change.
   - Global pairs: a new non-auto name must not already exist in a build or tool file; an auto
     name must carry the old name's address; old names may share a new name only when they name
     one address; no old name survives in a build file (C comments included).
     `movovl/` definitions of, and in-module references to, the same routine of the same verbatim
     Sony module pass (Q130); other movovl hits block.
   - `T.m=n`: `n` is no member, type, macro, paste or keyword name in `src/` / `include/` C
     (base and post-global trees), save that members the code shows alike may share `n` in one
     wave; accesses resolve to `T` only by the oracle build.
   - `f:l=n`: `n` is fresh in `f`, at file scope and in enums (post-global tree), in no macro,
     no keyword; the cc1 differential proves the binding. Scoped comment edits are notes (R6).

   These are **the wave's surfaces**.
6. **Moved layer-2 keys:** a completed body whose key moved only by the wave's pairs (the
   keycheck proves it) needs no new cheat review; a second, post-apply `naming-reviewer`
   pass checks the diff and the keycheck report (R6), required for every wave.
7. **Commit:** one commit per wave, subject `naming: <wave>`. The body gives the manifest path,
   both review verdicts, the oracle line, the keycheck line and moved-key count, and the census
   regenerated in the same commit.
