# s9 probes (manual, Claude, 2026-09-28)

Measured with `tmp/c21c/orph.py` (vars / orphan count / code lines differing from the banked
candidate, frame lines included) and `sandbox --disable all` at main bbf3d07de.

| file | what | result |
|---|---|---|
| POLICY-BLOCKED-frame-exact-dtd-tw-xy-4.c | `s16 dtd, tw` as SetDrawMode's dither / texture-window args at the phase-1 and phase-3 calls; `s16 xpos, ypos` at the phase-1, phase-2-head and phase-4-head descriptor stores; literals everywhere else | vars 128 = target frame, cc1 code identical to candidate; **sandbox 4/622** (only the two r3 store-order hunks) |
| POLICY-BLOCKED-frame-plus-col-0.c | the above + the withdrawn per-arm `col` colour local | **sandbox 0/622** (byte proof only) |
| dtd-tw-phase3-5-costs-regs.c | dtd/tw read at the phase-3 AND phase-5 calls | vars 128 but +2 insns: the phase-5 read plants `(use Z)` at the phase-5 label, Z lives across phase 4's calls, gets a callee-saved reg (`move $22,$0`, mode moves s5->s7) |
| s16-level-phase8-orphan-codediff10.c | phase 8 `s16 lv` load, index via lv, one bar test via lv | 1 family-2 orphan (vars 112) but 10 code lines differ |
| orphan-census-summary.txt | every combine-planted `(use (reg))` orphan in every src/*.c TU (134) with its pre-combine chain | see evidence.md s9 |

tools/: `census.py` (orphan census, run under WSL with the venv), `orphdetail.py` (chain for one
orphan), `sotn_zero_narrow.py` (SOTN scan for narrow locals whose every write is 0),
`mkvar.py` / `mkdtd.py` (variant generators; expect `tmp/c21c9/`).

## Layer-2 FAIL remediation (s9, after the Q27 grant)
- `landing-body-q27.c` (ledger root): the body spending owner ruling Q27 (rules 32a6b3626); sandbox
  0/622, full-build SHA1 == oracle. First layer-2: FAIL on `cells` (Ruling 9 (b) census) and `work`
  (Ruling 11 (D) for the full one-variable-per-value spelling). Body unchanged; evidence added:
- `cells-census/`: MOD.BIN resolution (cdfile.py) + sheet census (mod_sheet_census.py/.txt): all 12
  reachable slots are one-header sheets, K = 0xC holds.
- `work-r11/`: w_spl_all (+ function-scope variant), per-value ablations w_spl_p2/p4/p6/lv,
  structural respellings st_*, generators, dump excerpts + BB2_ALLOC_DEBUG for reuse vs split,
  permuter best find (1695). Scores and campaign data: admission.md § `work` (D) on the Q27 body.
