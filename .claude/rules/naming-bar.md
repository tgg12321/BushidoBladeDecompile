---
name: naming-bar
paths: [".claude/rules/naming-bar.md", ".claude/agents/naming-reviewer.md", "docs/naming/**", "tools/naming_wave.py", "tools/data_wave.py", "tools/naming_keycheck.py"]
description: "Owner ruling Q111 (2026-10-04, step 0 answers 2026-10-07): the Phase 3 naming bar, SOTN-equivalent. A name lands when it explains from the code across every use and a fresh naming-reviewer passes it; evidence classes settle conflicts, they do not gate. Neutral names beat guesses. Waves are tooled and identifier-only. Not a completion-bar item."
metadata:
  type: rule
  tier: hygiene
---

# The naming bar (owner ruling Q111)

Applies to every name a naming wave lands or keeps: functions, globals, types, struct members,
file names. It never decides a function's completion state ([[completion-bar]] does).

## N1 — the SOTN bar

A name is admissible when a reader can explain it from the code and a fresh `naming-reviewer`
agrees. Every read, write and call of the named thing bears the name out:
- **function:** the verb covers the whole effect of the body (its writes, calls and return);
  the noun is what the body touches; nothing it does contradicts the name.
- **global / member:** every access, in every TU and in the asm of `INCLUDE_ASM` / canonical
  bodies, agrees in role, width and lifetime.
- **type:** one layout; every named field is named by its accesses.

One contradicting use fails the name.

## N2 — neutral beats a guess

Without enough evidence, keep the neutral name (`func_8XXXXXXX`, `D_8XXXXXXX`, `Unk<addr>Rec`,
`unk_XX`). A neutral name makes no claim. A false name sends the next reader down a false trail,
so it costs more than a neutral one (owner directive 2026-08-07).

## N3 — evidence: conflict order, not a gate

No class is required. Where the sources disagree, the higher one wins, and a conflict the
order does not settle fails the row:
1. Sony's own name: verbatim PsyQ XDEF / libscan-xref, in-binary string, hardware role.
2. SOTN or psyz name for the same PsyQ routine or data (BB2's libraries are 4.0-era,
   `docs/naming/libscan/psyq_versions.txt`); a Sony static only after its verbatim placement
   is shown.
3. Explained from the code: computation, typed restatement, or call-graph role (every caller
   uses it the same way, and its callees and data establish the subsystem).

A string the function prints is context, not identity. Never evidence: a family prefix
alone, a name another rename invented, comments, earlier agent claims not re-derived,
size-only matches. Library code keeps Sony's spelling.

## N4 — prefixes and specificity

A subsystem prefix (`camera_`, `model_`, `stage_`, `snd_`, `cd_`...) must itself be explained
(callees, data, or callers of that subsystem). Claim no more than the code shows:
`camera_Update`, not `camera_UpdateReplayTilt`, when "replay" and "tilt" are guesses.

## N5 — existing names

An existing INFERRED or unattributed name meets the same bar. If it fails N1, it is RESET
or renamed. It is not kept for history.

## N6 — file names

An address-named game TU (`9F9C.c`) takes a subsystem name only when its whole content is that
subsystem: the admitted names and data of every function in it agree. A mixed file keeps its
ROM-offset name (most large game files hold several original TUs that the bytes cannot
separate). The move uses `tools/move_tu.py`, and the top comment keeps its boundary evidence.

## The wave

1. **Manifest:** a miner writes one row per op: address, kind, old -> new, every use with its
   `file:line`, and the counter-evidence considered. Function rows go in
   `docs/naming/phase3/<wave>/func_manifest.csv` with `evidence_class` `sotn-review` (census
   origin `sotn-review`, tier CORROBORATED), `RESET` rows `reset-contradicted`, and an N5 KEEP
   (an existing name that passes) as a `sotn-review` row with `proposed_name` = the current name.
   It upgrades the census tier and is not a pair. Data rows go in `data_manifest.csv` for
   `tools/data_wave.py`.
2. **Review:** a fresh `naming-reviewer` checks each manifest. It may set `verdict` CONFIRM only
   on rows it PASSes. Rows it FAILs are dropped or re-mined, never patched by hand.
3. **Census:** `python3 docs/naming/build_census.py`.
4. **Apply:** with a clean tree and the Grinder stopped, run `tools/naming_wave.py --from-census
   --only <addrs>` (dry run, then `--apply`) and `tools/data_wave.py --manifest-csv <csv>`.
   Members and types are edited in C; the keycheck takes global pairs, so a member rename needs a
   name unique in the tree (or waits for per-scope pairs). Then run the comment pass,
   `python3 tools/naming_keycheck.py --pairs <...> --sub-comments`. A comment naming a renamed
   identifier follows it, so no comment keeps a retired name.
5. **Gates:** `verify-oracle --rebuild` (SHA1 == oracle); `tus-check`;
   `tools/check_completion_integrity.py`; `engine test`; then
   `python3 tools/naming_keycheck.py --pairs <old=new,...>`. It fails closed. Every changed,
   added, deleted or renamed path must be the base with the pairs substituted, using the wave
   tools' own substitutions:
   - C and header tokens, and each comment in its place, change only by the pairs. A moved
     comment or a `#define NAME(` spacing change fails.
   - Each moved `layer2.jsonl` must equal naming_wave's retarget of the old record. Every other
     moved ledger file moves unchanged, and nothing else under `memory/` changes.
   - The census CSV must equal a fresh `build_census.py` run (working tree only; it reads
     `build/bb2.map`, so it runs after the rebuild). `docs/naming/phase3/**` is the
     only free path. Other text files (registries, gate lists, tools) change only by the pairs.
   - `.claude/**` and the tool itself never change.
   - A new non-auto name must not already exist, and an auto name must carry the old name's
     address.
   - No old name may survive in a build file (C comments included).

   These are **the wave's surfaces**.
6. **Moved layer-2 keys:** a completed body whose key moved only by the wave's pairs (the
   keycheck proves it) needs no new cheat review. Instead, a second, post-apply `naming-reviewer`
   pass checks the diff and the keycheck report (R6). That pass is required for every wave.
7. **Commit:** one commit per wave, subject `naming: <wave>`. The body gives the manifest path,
   both review verdicts, the oracle line, the keycheck line and moved-key count, and the census
   regenerated in the same commit.

Locals and parameters are out of scope until the keycheck takes per-function pairs.
