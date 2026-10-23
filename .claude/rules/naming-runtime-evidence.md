---
name: naming-runtime-evidence
paths: ["docs/naming/**", "tools/trace/**", ".claude/agents/naming-reviewer.md", ".claude/rules/naming-runtime-evidence.md"]
description: "Owner ruling Q124 (2026-10-23): a recorded runtime observation (trace) is corroborating evidence for a name the code already supports (naming-bar N1, N3 tier 3); it never establishes a name on its own and never overrides Sony/SOTN names."
metadata:
  type: rule
---

# Runtime observation as naming evidence (owner ruling Q124)

Extends [[naming-bar]] N3. A runtime observation corroborates a name the code already supports
(N1, tier 3); it never establishes a name on its own. A narrower name needs code evidence for
the narrower part too; the trace only corroborates it. On its own it can back a NO_NAME or a
neutral KEEP. It counts when ALL hold:

1. **Recorded, reproducible:** the trace script, the emulator name and version, the BIOS, the
   disc image SHA1, the starting point (savestate or memory-card file SHA1, plus the scripted
   input sequence) and the captured output; the capture's path + SHA1 is in the manifest row.
2. **One of the admitted kinds, read narrowly:**
   - a hit map (which game mode, screen, stage or event reaches the function or touches the
     data). An "only X" claim lists every mode, screen and stage the run covered and shows zero
     hits outside X. A function hit in every mode gets no role from a hit map.
   - stub-and-observe (the function patched to return). It counts only if the game keeps
     running and the lost effect is the function's own output (its writes or its callees'), not
     something downstream of a crash, hang or corrupted state.
   - a value watch (what a field or `D_` object holds across named game events).
3. **Claims only what was observed** ([[naming-bar]] N4). A conflict with the code fails the
   row; tiers 1-2 always win.
4. **Same review:** the fresh default-FAIL `naming-reviewer` re-reads the capture against the
   claim and checks its SHA1; re-running it is optional.

Never evidence: an observation without its capture, a single unexplained hit, or a name for code
the trace never reached (dead code stays neutral). Instruction-identical twins need code evidence
to tell them apart.
