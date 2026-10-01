---
name: canonical-gate-distance-not-evidence
paths: ["engine/canonical.py", "inline_asm_canonical.txt"]
description: "Large pure-C distance is NOT hand-asm evidence. ASM-SUSPECT/ASM-STRUCTURAL is the gate's best guess; keep pushing pure C unless scan_hand_coded shows real S1/S2/S6/S7/S8 signals."
metadata:
  type: rule
---

# Distance is not evidence — keep pushing pure C

A 500-3000-instruction function accumulates RA/scheduling drift well past any distance threshold without being
hand-written. The 2026-06-09 audit found 26/26 distance-routed ASM-STRUCTURAL items had ZERO hand-coded signals.
`engine/canonical.py` therefore requires BOTH distance > 500 AND `scan_hand_coded` tier >= POSSIBLE for
ASM-STRUCTURAL; otherwise large distance is ASM-SUSPECT.

- **ASM-SUSPECT**: grind pure C ([[register-alloc-pure-c]], [[cross-jump-call-merge]],
  [[cross-jump-store-tail-merge]], [[defeat-licm-hoist-var-reuse]], [[split-read-defeats-hoist]], …).
- **ASM-STRUCTURAL**: two converging signals, still not proof. Route to the canonical-asm grant path
  ([[judge-sole-gate]]: STRONG evidence + Judge verdict, driver-written grant, logged to
  docs/grind/borderline.md). Lay out which of S1/S2/S6/S7/S8 actually fired; the verdict + driver tier
  re-verification decide, never agent inference.

Before reaching for canonical asm:
1. `python3 tools/scan_hand_coded.py --single <func> --json` — LOW or TIGHT_C with score <= 2 is not evidence.
2. Read the asm: standard prologue, GP-rel loads, strength-reduced index math, JAL+delay-slot, struct stores =
   compiled C (`tools/scan_hand_coded.py`).
3. "It's a hard function" is not a no-C-form case ([[no-new-park-categories]]); the matching C exists and the
   toolchain is frozen ([[no-compiler-divergence]]).

Multi-session is not canonical-asm (e.g. `func_80023F08`, `func_80058580`).
