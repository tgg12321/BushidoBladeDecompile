# memory/wip/ — checkpointed candidate bodies for INCOMPLETE functions

A WIP entry is a **session-survivable checkpoint** of in-progress work on a
function that has not yet reached COMPLETED-C. The build is untouched (the
WIP files are outside the cpp/cc1/maspsx pipeline); the entry records the
best candidate C body, the measured sandbox score it produces, the technique
used, the remaining structural gap, and the next-step hypotheses.

A fresh agent picking up a queued function with a WIP entry can resume from
the documented floor in one read — no need to re-derive the prior session's
levers from memory notes or git log.

## The contract

| | |
|---|---|
| **In the build?** | NO. `memory/wip/` is not on any compile/link path. `verify-oracle --rebuild` is unaffected. |
| **In git?** | YES. The directory is tracked (`memory/` is not gitignored). Survives session reset, branch switch, agent handoff. |
| **Authoritative?** | NO. The committed `src/` is the canonical source. WIP candidates are evidence, not active code. |
| **Cheat-gated?** | YES at commit time. The cheat-reviewer verdict is recorded in `meta.json`. A WIP carrying a known cheat MUST be marked `reviewer.verdict = "FAIL"` with the evidence — preserved as a "do not re-derive this" warning, not as a usable candidate. |
| **Lifecycle** | Created on first measured progress. Updated on subsequent sessions that lower the floor. Deleted when the function reaches COMPLETED-C (the entry's lessons migrate to a `rules/` doc if they generalize). |

## Layout per function

```
memory/wip/<func_name>/
├── candidate.c          # The candidate C body — just the function (and any necessary local decls).
│                        # Copy-pasteable into src/<file>.c when resuming.
├── meta.json            # Engine-readable metadata: scores, technique, reviewer verdict, hypotheses.
├── notes.md             # Human-readable summary: pointer to the deeper memory/project/ notes
│                        # if any, key observations from this session, what's been ruled out.
└── rejected/            # OPTIONAL: forms that hit lower scores but failed cheat-reviewer.
    └── <slug>.c         # Preserved as "future agents should not re-derive this" warning.
```

## meta.json schema

```json
{
  "function": "func_8007XXXX",            // matches engine/queue.json entries
  "file": "display",                       // src stem (no .c extension)
  "scores": {
    "head_floor": 16,                      // sandbox --disable all on HEAD's src
    "candidate_floor": 12,                 // sandbox --disable all on candidate.c
    "metric": "sandbox --disable all (masked Levenshtein)",
    "build_insns": 50,                     // candidate's build_insns (optional)
    "target_insns": 51                     // target.s insn count (optional)
  },
  "sessions": [                            // append per session, never overwrite
    {
      "date": "YYYY-MM-DD",
      "lever": "short human-readable name",
      "lever_rule": "rule-slug",           // optional .claude/rules/<slug>.md reference
      "floor_was": 20,
      "floor_now": 12,
      "session_id": "uuid"                  // optional CLAUDE_SESSION_ID for metrics
    }
  ],
  "reviewer": {
    "verdict": "PASS" | "FAIL" | "NEEDS_USER" | null,  // null = not invoked yet
    "date": "YYYY-MM-DD",
    "evidence": "one-line summary of the reviewer's reasoning"
  },
  "remaining_gap": {
    "summary": "what structurally separates candidate from target",
    "rule_count_at_head": 21,              // existing regfix/asmfix on HEAD
    "rules_required_for_full_match": "description of what rule changes would be needed if candidate were committed"
  },
  "next_hypotheses": [
    "concrete next-step lever (permuter from candidate base, ALLOCDBG probe, sibling cross-ref, etc.)"
  ],
  "rejected_forms": [
    {
      "form": "short description",
      "score": 10,
      "reason": "why this is a cheat — point at the violated rule"
    }
  ],
  "memory_notes": [                        // related memory/project/ notes
    "memory/project/func-XXXX-foo.md"
  ]
}
```

## How agents use it

### Starting work on a function

1. `engine queue next` shows the top item. If a WIP exists, the hook + CLI surface
   the candidate floor and a pointer to `memory/wip/<func>/notes.md`.
2. **Read `meta.json` first.** Know the candidate floor, the reviewer status,
   the rejected forms (don't re-derive them), the next hypotheses.
3. **Apply `candidate.c` to `src/<file>.c`** if you want to start from the
   prior best floor instead of HEAD. Confirm with
   `engine sandbox <func> --disable all` — score should match
   `scores.candidate_floor`.
4. **Iterate.** If you find a lower floor, update `candidate.c`, append to
   `sessions[]`, update `scores.candidate_floor` and `remaining_gap`.
5. **Closing out.** If you reach COMPLETED-C, apply to `src/` for real, run
   `retire` + `queue done`, then **delete `memory/wip/<func>/`** (the entry's
   purpose is served). If a lesson generalizes, lift it into a path-scoped
   rule under `.claude/rules/`.

### Saving progress (no full match)

1. **Vet your candidate against the cheat catalog yourself first** (cheats-by-
   any-spelling — every construct must have semantic purpose).
2. **Invoke `cheat-reviewer`** (per `.claude/rules/review-discipline-before-
   commit.md`). The verdict goes in `meta.json`.
3. **Write `candidate.c`** (just the function body + any local decls it needs)
   and **update `meta.json`** (`sessions[]` append, `scores.candidate_floor`,
   `remaining_gap`, `next_hypotheses`).
4. **Commit** under `wip:` prefix. The build is unaffected so the commit is
   safe; the worktree-cleanliness guard ignores `memory/`.

### Discarding a candidate (reviewer FAILED)

If the cheat-reviewer returns FAIL on the candidate you were about to save,
DO NOT save it as `candidate.c`. Save it under `rejected/<slug>.c` with a
header comment naming the violated rule. This preserves the "future agents
shouldn't re-derive this" lesson without polluting the active candidate.

## Why not just use memory/project/ notes?

`memory/project/func-XXXX-*.md` is great for the narrative — what was tried,
what worked, why a particular gap remains. But:

- The candidate C body lives inside a markdown code block, not in a file the
  next agent can `cp` to `src/`.
- Multiple notes accumulate per function over many sessions; the "current
  best" can be hard to identify without reading all of them.
- The score, rule count, and reviewer status are scattered across prose; no
  machine-readable signal for the engine to surface in `queue next`.

WIP entries are the **structured, machine-readable, copy-pasteable** form of
that same evidence. The deep memory notes stay where they are; the WIP entry
points at them via `memory_notes`.

## Why the build is unaffected

`memory/wip/candidate.c` is not on any compile path. `splat.yaml`, `bb2.ld`,
the Makefile, the engine pipeline, and the per-file CPP/cc1/maspsx commands
all look at `src/`, `include/`, `asm/`, and the project's pipeline `*.txt`
files. None of them recurse into `memory/`. The oracle build is preserved.

The `park_src_guard` commit-msg hook blocks `park:` commits that modify
build-pipeline files; it does NOT block `memory/wip/` changes. A `wip:`
commit that only touches `memory/wip/<func>/` is always safe.
