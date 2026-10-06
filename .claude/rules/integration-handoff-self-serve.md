---
name: integration-handoff-self-serve
paths: ["tools/grinder/**", "docs/grind/*.md", "engine/queue.py"]
description: "Owner ruling 2026-08-19 — bytes-proven INTEGRATION HANDOFFs are pipeline-executable: the driver widens scope_allow.txt / clears Judge-superseded bans itself on a Judge ESCALATE(integration-handoff). Only the severe-blocker list pends the owner."
metadata:
  type: rule
---

# Integration handoffs are pipeline-executable (owner ruling 2026-08-19)

Owner: *"I don't want anything pending my decision anymore ideally, except for the most severe
of blockers."* An INTEGRATION HANDOFF is a function whose honest bytes are PROVEN
(`sandbox --disable all` == 0 AND full-build SHA1 == oracle with the banked form) but whose fix
touches a surface a grind session may not stage. It is a pipeline disposition, not an
owner-pending state.

## The mechanism (driver-executed; sessions may not do it)

Sessions still may NOT touch `tools/grinder/scope_allow.txt`, `memory/grind/<func>/state.json`
or any surface outside their scope. On a Judge ESCALATE the DRIVER acts:

1. **Scope widening.** `escalate_kind=integration-handoff` + `scope_paths=[...]` ⇒ the driver
   runs `grindlib.py add-scope-allow`, appending the per-function `scope_allow.txt` line ONLY
   for allowed classes: shared headers (`include/**/*.h`, `src/**/*.h`), sibling TUs (`src/**/*.c`), root-level
   `*.txt` allowlists except the denylist below. The function stays ACTIVE and the next session
   lands the fix through the FULL gates (sandbox-0 re-verify, scope check, layer-1, Judge,
   full-build SHA1). A scope line widens scope, never standards.
2. **Superseded construct bans.** When a Judge ruling narrows/supersedes an earlier ban it emits
   `unban_construct=<substring>` and the driver clears matching `banned_constructs` via
   `grindlib.py unban` (the narrowing itself stays in `judge_constraints`).

## Denylist for add-scope-allow (always refused)

- `inline_asm_canonical.txt` (own evidence-gated grant path,
  [[canonical-asm-authorization-recipe]]).
- The maspsx fidelity-gate lists (`maspsx_prefill_label_funcs.txt`, `maspsx_comm_syms.txt`,
  `expand_lb_funcs.txt`, `expand_dest_funcs.txt`, `multu_funcs.txt`, `multu_pad_funcs.txt`).
  **This list is a doc/code pair with `_SCOPE_GRANT_DENY` in `tools/grinder/grindlib.py`; keep
  them in sync.**
- Anything under `tools/`, `engine/`, `.claude/`, `docs/`, `memory/`, `asm/`, `disc/`;
  `Makefile`, `*.ld`, `splat.yaml` (enforced by path-class regex).

## What still pends the owner

Changes to the completion bar ([[completion-bar]]); any change to the oracle, substrate or build flags ([[no-compiler-divergence]]); retiring or
weakening a guard, gate or the Judge's policy; disc assets or the original EXE.

Related: [[judge-sole-gate]] · [[rotation-not-foreclosure]]
