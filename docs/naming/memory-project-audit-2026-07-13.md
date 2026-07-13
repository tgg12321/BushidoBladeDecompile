# memory/project/ func-specific note freshness audit — 2026-07-13

Audited the 11 `memory/project/func-*.md` per-function investigation notes for
freshness against `engine/queue.json`, git completion history, and live
`memory/wip/` + `memory/grind/` ledgers.

## Method

- Queue membership: `engine/queue.json` stores func names **uppercase**
  (`func_8007B844`), so a lowercase lookup falsely reports NOT-IN-QUEUE — verified
  each against the canonical casing.
- Completion: `git log --grep=<func>` for a `Match: … COMPLETED-C` commit +
  `inline_asm_canonical.txt` membership.
- Supersession: checked for a `memory/grind/<func>/` ledger (none of these 7
  functions have one) and a live `memory/wip/<func>/` checkpoint.

## Function state summary

| Function | Queue status | WIP entry? | Grind ledger? | Completed? |
|---|---|---|---|---|
| func_8007B844 | parked | yes | no | no |
| func_8007C7A0 | parked | yes | no | no |
| func_8007C86C | parked | yes | no | no |
| func_8007C938 | **off queue** | no | no | **COMPLETED-C** (`0c0e56a7`) |
| func_8007CA00 | parked | no | no | no |
| func_8007CBB0 | parked | yes | no | no |
| func_8007CE0C | parked | no | no | no |

Parked ≠ done: under [[no-park-permanently]] a parked item is still INCOMPLETE and
remains on the queue, so its investigation note is still load-bearing. Only a
function that reached a COMPLETED state (off the queue, zero rules, byte-match)
renders its note stale.

## Per-note verdict

| Note | Verdict | Rationale |
|---|---|---|
| func-8007b844-conditional-dead-store.md | KEEP | func_8007B844 parked + live WIP (session 15); note records the borderline-cheat closing form NOT to re-attempt. |
| func-8007c7a0-arg1-preload-lever.md | KEEP (promote-candidate) | Parked + live WIP. Floor-progression lever note. |
| func-8007c7a0-named-intermediate-lever.md | KEEP (promote-candidate) | Parked + live WIP. Lever note. |
| func-8007c7a0-sotn-duplicate-read-lever.md | KEEP (promote-candidate) | Parked + live WIP. SOTN duplicate-read lever. |
| func-8007c86c-clean-form-floor.md | KEEP | func_8007C86C parked + live WIP. |
| func-8007c86c-permuter-ub-cheat-findings.md | KEEP | Parked + live WIP; documents forbidden UB-cheat closing forms (rejection set). |
| func-8007c86c-sotn-duplicate-read-lever.md | KEEP (promote-candidate) | Parked + live WIP. SOTN duplicate-read lever (sibling of C7A0). |
| func-8007c938-investigation.md | **ARCHIVE** | func_8007C938 is COMPLETED-C (`0c0e56a7`), renamed `initdrawoffset_helper_8007C938`, off queue, zero rules. Note's own frontmatter already says `status: completed-c`. |
| func-8007ca00-m2c-shape-floor.md | KEEP | func_8007CA00 parked; no WIP dir but the note IS the current floor record (13→10). |
| func-8007cbb0-pure-c-draft.md | KEEP | func_8007CBB0 parked + live WIP; documents the honest 52 pure-C floor + semantic decomp. |
| func-8007ce0c-semantic-investigation.md | KEEP | func_8007CE0C parked; no WIP dir but the note IS the current semantic-frame record. |

## Actions taken

- **Archived** `func-8007c938-investigation.md` →
  `memory/project/archived-2026-07-13/func-8007c938-investigation.md` via `git mv`,
  with a HEAD note explaining the archival (function COMPLETED-C in `0c0e56a7`,
  renamed, off queue) and preserving the generalizable finding below the banner.

Nothing else moved — the other 10 notes cover functions that are still INCOMPLETE
(parked, on the queue), so they remain current.

## Interesting patterns

- **Sibling cluster C7A0 / C86C.** Six of the eleven notes cover just two functions,
  which are near-identical siblings (constant `0xE3` vs `0xE4`). Both share the exact
  same structural ceiling: a missing target `move a3,v0` "park-merge" instruction →
  `$a2`-vs-`$a3` X-preserve register cascade that no pure-C lever closes. The lever
  notes for the two functions are near-mirror images (arg1-preload, named-intermediate,
  SOTN duplicate-read each appear on both). This is a strong signal that the *shared
  ceiling* — not the individual levers — is the durable fact; consider consolidating the
  six into one sibling-cluster note keyed on the park-merge ceiling when either function
  eventually resolves.

- **PROMOTE-CANDIDATE (flagged only, per instructions — no rule file created):** the
  **SOTN duplicate-read precompute lever** appears on both C7A0 (session D, floor 16→12)
  and C86C (session 4, floor 20→12) as a post-policy-update (2026-06-02) allowed
  technique. A cross-function, repeatedly-applied codegen maneuver is exactly the
  profile of a `.claude/rules/` technique entry. Caveat: it may already overlap existing
  index entries (`split-read-defeats-hoist`, `duplicated-statement-into-arms`) — verify
  before minting a new slug. Owner sign-off territory; left in place this pass.

- The `named-intermediate` lever (C97C-style `r_e3`) similarly recurs across C7A0/C86C
  and is described as derived from a third function (C97C) — another cross-function
  technique worth the same overlap check against `register-alloc-pure-c`.
