---
name: kengo-cross-reference
paths: ["named_syms.txt", "docs/naming/**", "kengo_matches.csv"]
description: "Workflow for using Kengo (PS2 sibling) as a decomp reference. Run kengo_ref.py first for named stubs; verify by call structure / constants / control flow before trusting. False renames: ignore reference, decomp from BB2 asm alone — but still finish the BB2 function."
metadata:
  type: naming
---

# Kengo reference workflow

Always run `python3 tools/kengo_ref.py <func> --bb2` for named stubs. 3 patterns:

1. **Rename verification** (always do first): compare call structure. Genuine match = similar branch count, calls roughly line up, same constants. False rename = Kengo has named SDK calls but BB2 has zero, different domain. If the Kengo name is wrong, ignore the Kengo reference and decompile from BB2's asm alone — but you still finish the BB2 function. (You do not skip the BB2 function; the Kengo cross-reference is just unhelpful for it.)
2. **Callee cascading:** Kengo named jal targets at structurally matching positions = the anonymous BB2 calls. One verified named function yields 3-10 new renames.
3. **Algorithm guide:** for verified matches, Kengo reveals: what function does, control flow, return values, struct offsets. PS2-specific calls (`sceVu0RotMatrixX`) just mean "PS1 uses equivalent GTE/GPU code here" — use BB2's asm for those parts.

Rule of thumb: same `jal` count + at least one Kengo callee name recognizable from BB2 context → use the reference. Many more Kengo named calls than BB2 anonymous calls → size collision, ignore the reference and decompile from BB2's asm.
