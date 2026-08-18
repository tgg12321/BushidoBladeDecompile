# func_80023648 — recon dossier (2026-08-17)

## 1. IDENTITY
- **Body:** `src/code6cac.c:2785` — `void func_80023648(u8 *arg0)`.
- **Named:** `cpu_helper_80023648` (`named_syms.txt:3259`, first-pass subsystem tag `cpu`, called from
  `cpu_calc_move_pattern_trajectory_80023F08`).
- **What it does:** per-frame CPU-character movement update. Gates on `*(u16*)(arg0+0x6A)` being 0x13/0x1B/0x30.
  If the `0x2C` bitfield has any of `0xF000`, it derives two 0/1 indices from bits 14/15 (each bumped when the
  matching 0x1000/0x2000 bit is clear), indexes a 3-wide s16 table `D_8008EB40[a0*3 + a1]`, and calls
  `func_8001F860` with either `arg0[0x1CA] + val/4` or `arg0[0x1D8] + val`. Then it clamps `|arg0[0x150]|` to 0x400,
  subtracts it from `arg0[0x14E]`, adds `arg0[0x1A]/16` (round-toward-zero via `+15`), clamps the result against
  `(arg0[0x1A] * D_800A310C[D_8008DA08[arg0[0xA]]] << 4) >> 12` and against 0, and finally advances
  `arg0[0xD8] / arg0[0xE0]` by `sin/cos(arg0[0x1CA]) * speed >> 16` using the `Judge` sine table.
- **Size:** target 159 insns; `asm/funcs/func_80023648.s` = 176 lines.

## 2. MEASUREMENT
- `canonical func_80023648` → **verdict `C`**, `asm_insns 0`, `total 159`, `distance 30`.
- `sandbox func_80023648 --disable all` → **score 30**, `target_insns 159`, `build_insns 159`, `rules_dropped 30`.
- Honest per-instruction alignment (`tmp/recon/func_80023648.hdiff.txt`, cheat-disabled `tmp/sandbox/.../code6cac.o`
  vs `build/src/code6cac.o`): **equal 110 / replace 49 / delete 0 / insert 0**.
  **Instruction count, opcode sequence, operand shapes and frame are all identical.** Of the 49 raw replaces, ~19 are
  branch/jump *addresses* only (a `mask=False` artifact of the two objects living at different section offsets);
  the masked score of 30 is exactly the count of genuinely register-differing instructions.
- The residual is **one consistent register permutation** over four pseudos:

  | our reg | target reg | quantity |
  |---|---|---|
  | `v1` | `a2` | the `D_8008EB40` table value and, later, `abs_val` (T37/45/46/47/57/70/72-78) |
  | `a0` | `v1` | `div16` = `arg0[0x1A]>>4` (T80-85) |
  | `a1` | `a0` | `sub_result` / reload of `arg0[0x1A]` (T81/83/87/92/98) |
  | `a2` | `a3` | every `mflo` destination (T101/120/131/141) |

## 3. RULE INVENTORY (30 regfix, 0 asmfix — `regfix.txt:601-635`; header comment "register allocation fixes (36 diffs, score 180->0)")
Every rule is a `subst` of a register name. The file already groups them, and the groups map 1:1 onto the
permutation above:
| Lines | Group (as commented) | Effect |
|---|---|---|
| 601-611 (11 rules) | "Group 1: v1->a2 — table value + abs_val" | our `$3` → target `$6` |
| 613-616 (4) | "Group 2: a0->v1 — div16" | our `$4` → target `$3` |
| 618, 620-623 (5) | "Group 3/3b: a1->a0 (+ v1->a2)" | our `$5` → target `$4` |
| 625-631 (7) | "Group 4: a2->a3 — mflo results" | our `$6` → target `$7` |
| 633-635 (3) | "Group 5: a1->a2 + a2->a3 — speed" | combined rename in the sin/cos block |

There are **no reorder, insert, delete or frame rules, and no cheat-asm inside the body** — no `register … asm()`
pins, no `__asm__` blocks, no barriers. This is the cleanest debt in the set: 30 rules, all pure renames.

## 4. RESIDUAL DIAGNOSIS
Pure **register allocation naming**, with zero scheduling, zero frame and zero constant-folding divergence. The
permutation is not a rotation — `{v1,a0,a1,a2}` → `{a2,v1,a0,a3}` — which is the signature of GCC 2.7.2 assigning
these quantities in a **different priority order**, not of a single extra live value shifting everything. Target's
first quantity (the table value / `abs_val`) is allocated *late* (`a2`) while ours takes the first free caller-saved
register (`v1`); correspondingly the `mflo` results are pushed one slot further out in target (`a3` vs `a2`).
Relevant rule: **`.claude/rules/register-alloc-pure-c.md`**. Because the instruction stream is already
byte-for-byte in the right order, `sched_solver` has nothing to do here and `tools/ra_solver` is directly
applicable — this is close to the ideal input for it (validated model, single-mechanism residual).

## 5. FRONTIER (no `memory/wip/func_80023648/` and no `memory/grind/func_80023648/` — no killed levers on record)
1. **Run the `tools/ra_solver` pipeline** (`extract.py` → `simulate.py`/`validate.py` → `perturb.py` with a spec
   pinning the table-value quantity to `$6`). Per `memory/project/ra-solver-campaign-2026-08-04.md` the solver
   models the whole allocation stack and was validated; a function whose *only* residual is allocation, with
   identical insn order, is exactly the case it was built for. Check `local_alloc.py` too — the four quantities
   here are short-lived and block-local, so this is likely a **local-alloc** ordering question (the same scope
   limit that stalled `func_800858D0` before Phase 5 added local modelling).
2. **Reduce live ranges of the `abs_val` quantity.** Ours reuses one `s32 abs_val` across the clamp *and* the later
   `sub_result` reassignment (`abs_val` is written, then `sub_result` reuses the same slot at line ~2836). Splitting
   them into two separate named locals shortens both ranges and changes the priority (refs/span) that drives
   `find_free_reg`. Cheap, no new construct.
3. **Give the `mflo` results distinct short-lived locals.** All four `mflo`s currently feed straight into a shift;
   target keeps them in `a3` (the last caller-saved). Naming each product into its own local (rather than the shared
   `mult_res`/`speed_prod` pattern) is the standard way to push a quantity later in the allocation order.
4. **`halfword-index-srl-sra` / `strength-reduce-defeat` check on the `row = new_var + (a0 * 3)` index.** Ours emits
   the `a0*3` via `sll/subu`; target's stream matches instruction-for-instruction here, so this is *not* currently
   a divergence — flagged only so a future session does not spend time "fixing" an already-correct region.

## 6. SIBLING LEADS
- **`func_80023F08`** (`cpu_calc_move_pattern_trajectory_80023F08`) is this function's direct caller in the same
  subsystem and file; it shares the `Judge` sine-table + `>>16` scaling idiom. Note `memory/project/slog-kengo-dead-end.md`
  records that `func_80023F08` has **no Kengo equivalent** — do not spend time on cross-game reference hunting for
  this family.
- The `(&Judge)[(x & 0xFFF)]` / `(&Judge)[((x + 0x400) & 0xFFF)]` sin/cos pair appears in several matched movement
  functions in `src/code6cac.c`; grep for `&Judge)[` to find one that already matches and compare which locals it
  gives the `mflo` products — that is direct evidence for hypothesis 3 without any solver work.
