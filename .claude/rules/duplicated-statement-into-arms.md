---
name: duplicated-statement-into-arms
paths: [".claude/rules/duplicated-statement-into-arms.md"]
description: "SANCTIONED (owner 2026-07-01): duplicating a REAL statement into 2+ arms instead of label-sharing, incl. when cross-jump re-merges it byte-neutrally (reg_n_refs lift). Byte-neutral, exhaustion, FAKE annotation, review."
metadata:
  type: rule
---

# Duplicated statement into arms — the sanctioned spelling

**Owner ruling 2026-07-01.** Writing the SAME real statement in two or more control-flow arms —
instead of sharing one copy via a label/goto — is a legitimate matching technique, **including**
when:

- GCC's jump2 cross-jump re-merges the copies so the final bytes are identical to the shared-label
  form, and
- the duplication's surviving effect is the extra `reg_n_refs` count flow.c records
  (allocno-priority lift for global RA), and
- the label placement among the copies is chosen to steer the merge DIRECTION.

Evidence: routine SOTN style (`src/boss/bo4/doppleganger.c` — one assignment across 7 and 11 arms);
match-annotated redundant duplicates (`src/dra/42398.c`, `src/dra/menu.c:1993,2009,2017`). The
duplicated and shared spellings compile to identical bytes, so when only the duplicated one
reproduces the target's allocation pin-free, that is evidence the original was duplicated.

## Prerequisites (cheat-reviewer FAILs if any is missing)

1. **The statement is REAL on its path** (a genuine def/effect the path needs — dead stores are
   governed by [[dead-store-fake-exception]]).
2. **Byte-neutrality verified**: the emitted function is byte-identical vs the canonical reference
   (objdump diff and/or full SHA1) — the duplication must NOT materialize extra instructions.
3. **Lever-exhaustion documented** when used as a last-resort RA lever (ledger / commit body).
4. **`/* FAKE: <reason> */` annotation** on the duplicated copy when the duplication is
   match-motivated.
5. **Layer-1 + layer-2 review** per [[review-discipline-before-commit]].

## Scope clarifications

- **Control-transfer tails (owner ruling 2026-08-06):** covers duplicating a multi-statement tail
  ending in a control transfer — including a loop tail with its conditional branch — when every
  prerequisite holds (SOTN: e_shop.c:986-1009, vs_vh.c:69-128).
- **Calls, byte-identical only (owner ruling 2026-09-30, Q47):** a statement containing a call may be
  duplicated only when (1) the call is real on each path — the callee and arguments the target has
  there (a never-executed or fabricated call stays REFUSED, absent a Q55 citation of matched SOTN
  code; [[no-new-park-categories]] "Fabricated dead call site"), (2) cross-jump merges the copies so
  the function is byte-identical (an extra `jal` the target lacks fails), (4) FAKE annotation when
  match-motivated; (3) and (5) unchanged. Record: docs/grind/decisions.md 2026-09-30 OWNER RULING —
  duplicated calls into arms, byte-identical only.
- **Non-extension:** not dead stores, not any duplication that survives into the final bytes (a real
  code change). Other spellings need their own evidence.
- Measured: dead stores are INERT for a global-RA ref-lift (flow deletes them before counting) —
  real duplication is the working spelling.

## Example (motion_SetMotion)

`sel2 = D_800A3350;` duplicated into case 0, label moved to case 13/17's copy → sel2 reg_n_refs 3→4
→ RA seats sel2→`$s2` / result→`$s3` as the target; cross-jump re-merges the tails (byte-diff
empty). Label placement was load-bearing: the mirror labeling flipped the layout.

Related: [[split-read-defeats-hoist]] (READ-duplication sibling) · [[register-alloc-pure-c]] ·
`pre-slim-2026-10-01:memory/project/sotn-family-research-2026-07-01.md`
