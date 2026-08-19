---
name: integration-handoff-self-serve
paths: ["tools/grinder/**", "docs/grind/*.md", ".claude/rules/*.md", "engine/queue.py"]
description: "Owner ruling 2026-08-19 — bytes-proven INTEGRATION HANDOFFs are pipeline-executable, never owner-pending; the driver widens scope_allow.txt / clears Judge-superseded bans itself on a Judge ESCALATE(integration-handoff) verdict. Only the severe-blocker list still pends the owner."
metadata:
  type: rule
---

# Integration handoffs are pipeline-executable (owner ruling 2026-08-19)

> Owner, 2026-08-19: "I don't want anything pending my decision anymore
> ideally, except for the most severe of blockers."

Extends [[judge-sole-gate]] (2026-08-18) from Judge verdicts to **integration
surfaces**. An INTEGRATION HANDOFF — a function whose honest bytes are PROVEN
(`sandbox --disable all` == 0 AND full-build SHA1 == oracle with the banked
form applied) but whose fix touches a surface a grind session may not stage —
is a **pipeline-executable disposition**, not an owner-pending state.

**Why this exists:** on 2026-08-19 three proven matches (SioSyncroWrite,
func_8001B748, func_8002D518) sat terminally parked overnight on one-line
surface grants the owner had to hand-apply. Every technical question had
already been answered under frozen policy; the park was pure latency. All
three were integrated to COMPLETED-C the next morning with zero policy
deviations — the parks bought nothing.

## The mechanism (driver-executed, session-forbidden)

Sessions still may NOT touch `tools/grinder/scope_allow.txt`,
`memory/grind/<func>/state.json`, or any surface outside their scope — a
session that could widen its own surface could grant itself anything. The
DRIVER executes the remedy, gated by the Judge, mirroring the canonical-asm
grant path:

1. **Scope widening.** On a Judge **ESCALATE** with
   `escalate_kind=integration-handoff` and `scope_paths=[...]`, the driver
   calls `grindlib.py add-scope-allow`, which appends the per-function
   `scope_allow.txt` line ONLY for paths in the allowed classes:
   - shared headers (`include/*.h`), sibling TUs (`src/*.c`),
   - root-level rule/allowlist `*.txt` files (e.g.
     `volatile_extern_allowlist.txt`) EXCEPT the denylist below.
   The function then STAYS ACTIVE: the next session lands the fix through the
   FULL normal gates (driver sandbox-0 re-verify, scope check, layer-1
   default-FAIL, Judge default-FAIL, full-build SHA1). A scope line widens
   scope, never standards — every widened path is both scope-checked and
   staged, so the committed tree is exactly the byte-verified tree.
2. **Superseded construct bans.** When a Judge ruling explicitly narrows or
   supersedes an earlier ban (e.g. "banned only under family X; resubmission
   under family Y authorized"), the Judge emits `unban_construct=<substring>`
   and the driver clears the matching `banned_constructs` entries via
   `grindlib.py unban`. The narrowing lives in `judge_constraints` as before;
   only the mechanical tripwire is cleared. (Root cause of the func_8002D518
   deadlock: `grindlib` had `ban` but no `unban`, so a Judge-authorized
   resubmission was auto-discarded twice by the stale tripwire.)

## Path denylist for add-scope-allow (severe-blocker class, always refused)

- `inline_asm_canonical.txt` — has its own evidence-gated grant path
  (scan_hand_coded STRONG tier, [[canonical-asm-authorization-recipe]]).
- The maspsx fidelity-gate lists (`maspsx_label_nop_funcs.txt`,
  `expand_lb_funcs.txt`, `expand_dest_funcs.txt`, `multu_funcs.txt`,
  `multu_pad_funcs.txt`) — assembler-behavior gates are substrate-adjacent.
- Anything under `tools/`, `engine/`, `.claude/`, `docs/`, `memory/`, `asm/`,
  `disc/`; `Makefile`, `*.ld`, `splat.yaml` — enforced by path-class regex.

## What STILL pends the owner (the "most severe" list)

- Extensions to the frozen sanctioned-family list
  ([[no-new-park-categories]]) — `family-extension` escalations keep the
  log-and-refuse routing.
- Any change to the oracle, the substrate (cc1/maspsx/linker/Makefile), or
  build flags ([[no-compiler-divergence]]).
- Retiring or weakening a guard, gate, or the Judge's default-FAIL policy.
- Anything touching disc assets or the original EXE.

## Related

[[judge-sole-gate]] · [[no-user-escalation]] · [[endgame-lock-disposition]] ·
[[no-park-permanently]] — this ruling removes the largest remaining source of
solved-but-parked functions.
