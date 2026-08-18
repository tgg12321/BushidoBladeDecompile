# func_8002D518 — recon dossier (2026-08-17)

## 1. IDENTITY
- **Body:** `src/code6cac_b.c:1122` — `s32 func_8002D518(s32 threshold, s32 r_sq, s32 *p1, s32 *p2)`.
  Called three times from the same file (`:1324`, `:1331`, `:1333`).
- **Named in comments as `saTan5TakeAnim2`** (the regfix/asmfix rule comments use that name; it is described there
  as a "154-insn line-sphere intersection").
- **What it does:** 2-D line-segment vs circle intersection test. First two cheap AABB rejections on the X and Z
  components of `p1`/`p2` against `±threshold`. Then it forms `ax/az` deltas, the squared terms, the dot product
  `(ax*x1 + az*z1) * 2 >> 9`, `c_val = ((x1² + z1²) - r_sq) >> 9`, and the discriminant
  `disc = dot2_9² - ((dist_sq >> 9) * (c_val << 2))`. If `disc < 0` it returns 0. Otherwise it takes a square root:
  small `disc` (< 0x400) reads the lookup table `D_8008D118` directly; large `disc` uses the **GTE leading-zero-count
  coprocessor op** to pick a shift, then indexes the same table. It finishes with two divisions producing the two
  roots and returns whether the near root lies within range.
- **Size:** target 144 insns; `asm/funcs/func_8002D518.s` = 170 lines.

## 2. MEASUREMENT
- `canonical func_8002D518` → **verdict `ASM-PARTIAL`**, `asm_insns 2 / 144`, regions `[91,91]` and `[93,93]`,
  reasons "GTE/cop2 op (mtc2)" and "GTE/cop2 op (swc2)". It stays in the **active** queue bucket (only
  ASM-STRUCTURAL / ASM-WHOLE go to `authorize`), and GTE/cop2 ops are canonical inline asm by policy
  (`.claude/rules/inline-asm-allowed.md`).
- `sandbox func_8002D518 --disable all` → **score 33**, `target_insns 144`, `build_insns 141`, `rules_dropped 33`.
- **CAVEAT — measured against a dirty TU.** At measurement time (2026-08-17) another agent in this session had
  uncommitted edits to `src/code6cac_b.c` (39 insertions / 54 deletions, confined to `func_8002FDB0` at lines
  1875-1955, i.e. well clear of this function's body at 1122-1300). The edits are in the *same translation unit*,
  so a compilation-context shift cannot be fully ruled out
  (`memory/project/stranded-branch-work-not-transplantable.md` documents how real that effect is here).
  **Re-run `sandbox func_8002D518 --disable all` on a clean tree before trusting the 33 as the honest floor.**
  The other five functions in this dossier set were measured against clean TUs.
- Honest alignment (`tmp/sandbox/func_8002D518/code6cac_b.o` vs `build/src/code6cac_b.o`;
  `tmp/recon/func_8002D518.hdiff.txt`): **equal 90 / replace 63 / delete 4 / insert 1**. About 12 of the raw
  replaces are branch-address-only artifacts. Four residual clusters:
  1. **The GTE region is 3 instructions short (T90, T92, T93).** Target has
     `move t4,a0` / `mtc2 …` / `move t4,sp` / `swc2 $31,0(t4)`; our honest stream retains the `mtc2` but has lost
     both `move t4,…` setups and the `swc2`. That is the entire 144-vs-141 shortfall. **Verify this first** — the
     C at `src/code6cac_b.c:1184-1191` expresses the sequence as `register s32 t4_v asm("t4")` pins driving two
     `__asm__ volatile(".word 0x…")` blocks, which is precisely the shape the cheat-invisible sandbox strips
     (`cheat_asm_stripped: 320` for this object). If so, **3 of the 33 points are a measurement artifact of the
     spelling, not a real codegen gap.**
  2. **Statement-order swap in the distance term (T67–T72).** Ours emits `addu a2,t0,a2` three instructions early
     (H67); target emits it at T70 as `addu a1,t0,a2`, after the `c_val` computation. This is exactly what regfix
     `reorder 76,77,78,75` restores.
  3. **Statement-order swap in the root computation (T110–T115).** Target order is
     `addu a0,v0,a2` → `sll a0,a0,0x8` → `sll v1,a1,0x1` → `div`; ours computes the denominator shift *first*
     (`sll a0,a2,0x1` → `addu v1,v0,a1` → `sll v1,v1,0x8` → `div`). Restored by regfix `reorder 119,120,118`.
  4. **A lost delay-slot fill and a pervasive register permutation.** At T82 target has `move a0,a2` (the `disc`
     value) in the `beqz` delay slot where we emit a `nop`. Everywhere else the four scratch quantities are
     permuted: our `a0`↔target `a2` on `disc`, our `v0`↔target `a2` on `sqrt_val`, our `v1`/`a0` swapped on the
     two division operands, our `a2`↔target `a1` on the result.

## 3. RULE INVENTORY (33 regfix + 2 asmfix)
**regfix.txt:1223-1280 (31 rules, all individually commented).** Grouped by effect:
| Lines | Kind | Papers over |
|---|---|---|
| 1223 (1) | `reorder 76,77,78,75 @ 75-78` | **cluster 2** — the `dist_sq` statement-order rotation |
| 1252 (1) | `reorder 119,120,118 @ 118-120` | **cluster 3** — denominator vs numerator order |
| 1234 (1) | `subst "nop.*" "addu $4,$6,$zero" @ 90` | **cluster 4's delay slot** — a textbook **lost-codegen insert** (`addu rD,rS,$zero` is the exact flagged pattern) |
| 1225-1226, 1228-1230, 1232, 1236, 1238, 1240, 1242, 1244 (11) | subst register renames | `dist_sq` a2→a1, `disc` a0→a2, `sqrt_val` v0→a2 (cluster 4) |
| 1246-1250, 1254-1276 (16) | subst register renames | the division-expansion operand permutation and `mflo` destinations (cluster 4) |
| 1278-1280 (2) | subst register renames | the returned `result` a2→a1 |

**asmfix.txt:38-39 (2 rules).** A mid-body label splice: `insert_before` synthesises
`.LsaTan5TakeAnim2_epilogue:` before the `addu $sp,$sp,8` epilogue, and `replace_first` retargets an early-out `j`
at it. The committed comment explains why: an inlined callee shifted the local labels enough that the early-out
jump started landing on the value-move instead of the shared epilogue. Under
`memory/rules/asmfix-all-debt-end-state.md` (owner ruling 2026-08-06) **all** asmfix entries are debt, including
this one.

Cheat-asm in the body: `register s32 t4_v asm("t4")` plus two `__asm__ volatile(".word 0x…")` blocks with no
operand placeholders (`src/code6cac_b.c:1184-1191`). Note that `src/code6cac_b.c` carries 98 register pins
file-wide — the heaviest pin density of the six target files — so honest measurement of *anything* in this file is
sensitive to pin stripping.

## 4. RESIDUAL DIAGNOSIS
Three mechanisms, and one measurement question that has to be settled before the other two are worth costing.

- **Spelling of the GTE sequence (cluster 1).** The `.word`-plus-hardcoded-`$t4`-pin form is not canonical inline
  asm — `.claude/rules/inline-asm-injection.md` is explicit that a template with hardcoded registers and no `%N`
  placeholders is the injection signature, and the sandbox strips it. The canonical GTE spelling uses
  constraint-bound placeholders (`.claude/rules/gte-3x3.md`, `.claude/rules/scratchpad-gte.md`,
  `.claude/rules/inline-asm-allowed.md`). Until this is re-spelled, **the honest floor of 33 is not comparable to
  the other five functions'** — 3 points of it are the stripped sequence.
- **List-scheduling statement order (clusters 2 + 3, 2 reorder rules).** Both are ordinary source-statement
  ordering: GCC 2.7.2's scheduler emits the arithmetic in source order within the block, so moving the statements
  in the C moves the instructions. `tools/sched_solver` (`memory/project/sched-solver-campaign-2026-08-05.md`,
  6978/6978 validated) has a goal-mapper mode designed for exactly this and reduced an identical pair of swaps to
  two ordinary statement moves in `func_800858D0`.
- **Register allocation (cluster 4, ~29 rules).** A four-way permutation across the discriminant / sqrt / division
  quantities. Rule: **`register-alloc-pure-c`**. Untouchable until clusters 1–3 are settled, because both the GTE
  region and the statement order change live ranges.

## 5. FRONTIER (no `memory/wip/func_8002D518/` and no `memory/grind/func_8002D518/` — nothing killed on record)
1. **Re-spell the GTE sequence with constraint-bound placeholders and re-measure the honest floor.** This is the
   prerequisite step, not an optimisation: it tells you whether the true floor is 33 or 30, and it removes a
   construct that would fail the completion gate anyway. Use the accepted GTE spelling from `gte-3x3` /
   `scratchpad-gte`. If a placeholder form genuinely cannot express the op, the canonical route is to authorise the
   two-instruction region in `inline_asm_canonical.txt` via the `canonical` gate (which already reports it as
   ASM-PARTIAL / GTE) — **not** to keep the `.word` + pin form.
2. **Apply `tools/sched_solver`'s goal-mapper to the two reorder clusters.** Run `perturb.py --goal-from-target`
   restricted to spellable atoms (`--atoms luid,luid_move`), exactly as was done for `func_800858D0`. The two
   regfix `reorder` rules describe the target permutation precisely, so the goal is already known — the question is
   only which C statement move produces it. Expect two ordinary statement moves: sink the `dist_sq` computation
   below `c_val`, and hoist the `t1_num` addition above the denominator shift.
3. **Retire the asmfix label splice by fixing its cause, not its symptom.** The comment says an *inlined callee*
   shifted the local labels. Check whether the early-out path can share the epilogue through a
   `goto end;` restructure — `.claude/rules/shared-end-label.md` is the standard remedy and would remove both
   asmfix rules and the fragile `replace_first` regex.
4. **Only last**, the RA permutation. Note the delay-slot `move a0,a2` (regfix line 1234) is a lost-codegen insert;
   the fix is to make `disc` live across the branch in C (assign it to the variable the taken path uses *before*
   the test), not to inject the move.

## 6. SIBLING LEADS
- **The two other line-sphere tests in the same file**, at `src/code6cac_b.c:1324/1331/1333`'s caller — the caller
  runs the same `sqrt_val`/`dist` pair through this function three times, and the surrounding matched code
  (`:1100-1120`, the min/max-Y clamp block) shows the accepted spelling for the same clamp-and-compare idiom.
- **`func_80053614`** (called from `src/code6cac.c:2776`) is another matched geometry routine using the same
  `0x1F8002B8` scratchpad address family; if the GTE re-spelling in hypothesis 1 needs a precedent for the
  scratchpad-based cop2 store, that is where to look (`.claude/rules/scratchpad-gte.md`).
- Anything else in `src/code6cac_b.c` that uses `D_8008D118` (the shared square-root table) will show how the
  small-vs-large `disc` branch is conventionally spelled here.
