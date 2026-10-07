---
name: permuter-directives
paths: [".claude/rules/permuter-directives.md"]
description: "Encode a rule's recommended maneuver as PERM_* macros so decomp-permuter tests every variant; run campaigns via tools/permuter_campaign.py with fresh-seed windows. Output is PROPOSALS that must clear cheat vetting + layer-2."
metadata:
  type: reference
---

# Permuter directives — from "rule says try X" to "permuter validated X"

`tools/permuter_annotate.py --hint <slug>` writes a PERM_*-annotated PROPOSAL to
`tmp/permuter_candidates/<func>.c` (never `src/`).

**Reach for it** when a manual restructure lowered the floor but plateaus at 2 <= d <= ~30; when a rule lists
2+ spellings of one maneuver; when a statement reorder is the lever. **Never** to find a cheat — the scorer is
cheat-blind — and never compare its weighted score with `sandbox --disable all` distance.

## The macros (source of truth: `tools/decomp-permuter/README.md`)

| Macro | Meaning | Pairs with |
|---|---|---|
| `PERM_GENERAL(a, b, ...)` | one of the alternatives; `(,)` = literal comma | [[shared-end-label]] (`goto end;` vs `return v;`), [[switch-vs-ifchain-branch-sense]], [[cross-jump-store-tail-merge]] exit forms, [[narrow-byte-args-packed-call]] param types |
| `PERM_VAR(name, v)` / `PERM_VAR(name)` | set / expand a meta-variable | diagnostic pin-register sweep ([[register-asm-pins]] — drop the pin before commit) |
| `PERM_LINESWAP(lines)` / `PERM_LINESWAP_TEXT` | permute complete statements / raw lines | [[loop-rotation-two-shift]], [[walking-pointer-serializes-parallel-loads]], [[defer-store-past-later-compute-into-jal-delay]] |
| `PERM_ONCE([key,] code)` | code appears at exactly one of its sites | [[hoist-call-arg-local-flips-jal-delay]] |
| `PERM_RANDOMIZE(code)` | re-enable the random pass in a region (off by default once any multi-choice macro exists) | directed + random |
| `PERM_INT(lo, hi)` | integer sweep | sizes, masks, shifts |
| `PERM_FORCE_SAMELINE(code)` | join to one line | [[loop-note-fixes-delay-slot-steal]] |
| `PERM_IGNORE(code)` / `PERM_PRETEND(code)` | pass through unparsed / parser-only stand-in | GCC extensions pycparser chokes on |

Build `target.o` from `asm/funcs/<func>.s` so the function sits at offset 0 (otherwise branch addresses add
huge constant score noise).

## Invocation

```bash
source .venv/bin/activate
python3 tools/decomp-permuter/import.py src/<file>.c asm/funcs/<func>.s
cp tmp/permuter_candidates/<func>.c tools/decomp-permuter/<dir>/base.c
python3 tools/permuter_campaign.py launch --func <func> --dir tools/decomp-permuter/<dir> \
    --label <chassis-slug> -j 8 --stop-on-zero
python3 tools/permuter_campaign.py harvest --dir tools/decomp-permuter/<dir> --stop --reason "<why>"
```

The wrapper passes `--stack-diffs` by default (without it, frame/stack-offset gaps false-match at score 0).

## Campaign discipline — fresh-seed windows (owner directive 2026-07-07)

- **Every campaign goes through `tools/permuter_campaign.py`** (`launch` / `harvest` / `status`).
- **Stopping rule:** if ~20-30 minutes after a fresh seed no NOVEL find has landed (outside known attractor
  classes), `harvest --stop` and either reseed a structurally different chassis or switch modality. Long tails
  only re-find known attractors.
- **Harvest everything, always,** before the session ends; campaigns never outlive their session (the Grinder
  reaps survivors, losing their finds).

## Vetting permuter output — MANDATORY before surfacing

Every closing form is a proposal: walk the cheats-by-any-spelling checklist ([[no-new-park-categories]]),
confirm no cheat construct from [[inline-asm-policy]] remains, run `python3 tools/check_completion_integrity.py`,
and pass layer-1 + the mandatory layer-2 `cheat-reviewer` ([[review-discipline-before-commit]]). A form that
needs any cheat construct in committed source is REJECTED regardless of sandbox 0 / SHA1.

Related: [[codegen-technique-index]] · [[register-alloc-pure-c]]
