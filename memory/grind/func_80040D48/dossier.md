# func_80040D48 — recon dossier (2026-08-17)

## 1. IDENTITY
- **Body:** `src/text1a_pre.c:563` —
  `void func_80040D48(s32 a0, s32 a1, s32 *a2, s16 *a3, s16 *arg4, s32 arg5)`.
- **Named:** `mode_helper_80040D48` (`named_syms.txt:3454`, first-pass tag, called from `mode_handler_05_NoOp`).
- **What it does:** per-entity pose/skeleton reset. Looks up an object base `s4 = D_800A9A10[a0]`, returns if null,
  copies a 3-element s16 vector from `a3` into `s4+0x3C..0x40` and a 3-element s32 vector from `a2` into
  `s4+0x78..0x80`, then switches on `a1`. Case 0 is the heavy path: it zeroes six fields at `s4+0x94`, then runs a
  17-iteration loop over `D_80094CFC` writing three copied/negated s16 fields per 0x68-byte joint slot, a second
  descending loop zeroing `+6` in each slot, then fills two 6-field blocks at `s4+0x7E4` and calls the function
  pointer `D_800F66A0[0]` twice. Cases 1..6 are small or empty; a trailing block walks a linked list and stamps
  104-byte-strided entries.
- **Size:** target 272 insns; `asm/funcs/func_80040D48.s` = 287 lines. **The largest of the six.**

## 2. MEASUREMENT
- `canonical func_80040D48` → **verdict `C`**, `asm_insns 0`, `total 272`, `distance 34`.
- `sandbox func_80040D48 --disable all` → **score 34**, `target_insns 272`, `build_insns 272`, `rules_dropped 34`.
- The sandbox emits no per-region breakdown, so I disassembled the cheat-disabled object
  (`tmp/sandbox/func_80040D48/text1a_pre.o`) against `build/src/text1a_pre.o` via
  `engine.score.normalized_insns(mask=False)`. Full alignment: `tmp/recon/func_80040D48.hdiff.txt`.
  Aggregate: **equal 229 / replace 43 / delete 0 / insert 0**.
  Instruction count is *exact* (272 = 272) and there is not a single insert or delete — every divergence is an
  in-place operand difference. Nine of the 43 raw replaces are branch-address-only artifacts of the two objects
  sitting at different section offsets; the masked score of 34 is the real count. Four residual clusters:
  1. **Entry block (T14–T17, T28):** target does `lw a0,0(at)` → `beqz a0,…` → **`move s4,a0` in the delay slot**,
     and defers `addiu s5,s4,44` to T28 where it fills a load-delay slot. Ours loads straight into `s4`
     (`lw s4,0(at)`), puts `addiu s6,s4,44` in the delay slot at H17, and leaves a **`nop` at H28**.
     Also an **`s5`/`s6` role swap**: target has `s5 = s4+44` and `s6 = ` the stack param; ours is the reverse.
  2. **Loop base anchor (T50, T65, T75, T85):** target `addiu a0,s3,104` with stores at `16/18/20(a0)`;
     ours `addiu a0,s3,124` with stores at `-4/-2/0(a0)`. Identical effective addresses, different
     induction-variable anchor.
  3. **Tail list-walk registers (T200–T246):** target keeps the list cursor in the **callee-saved `s5`**
     (`lw s5,64(a3)`, then eight `lw …(s5)`); ours uses `a2`. Target uses `a2`/`a0` for the two 104-strided write
     cursors where we use `t0`/`a0`, and `a0`/`a1` are swapped on the sentinel compare.
  4. **Scattered naming:** `mflo t0` vs `mflo t1` (T162), and `s5`↔`s6` at T165 / T250 / T251 / T254.

## 3. RULE INVENTORY (34 regfix, 0 asmfix — `regfix.txt:1787-1827`)
| Lines | Kind | Papers over |
|---|---|---|
| 1787-1789 (3) | register swap `$21 <-> $22` @15, @163, @240-244 | the **s5/s6 role swap** (clusters 1 + 4) |
| 1792-1794 (3) | subst `$20`→`$4` ×2, plus rewrite `addu $22,$20,44` → `addu $20,$4,$0` @14-17 | commented "structural — move s4=a0 into beqz delay slot" (cluster 1) |
| 1796 (1) | subst `nop` → `addu $21,$20,44` @28 | fills the load-delay slot target fills (cluster 1) — materially a **lost-codegen insert**; see §4 |
| 1799-1802 (4) | subst base `s3+124`→`s3+104`, offsets `-4/-2/0`→`16/18/20` | the **loop base anchor** (cluster 2) |
| 1805-1806 (2) | subst `$9`→`$8` @160-161 | `mflo` destination naming |
| 1807-1827 (21) | subst `$8`→`$6`, `$6`→`$21`, `$4`→`$6`, `$5`→`$4` across @194-236 | the **tail list-walk permutation** (cluster 3) |

Cheat-asm carried in the body (invisible to the rule count, stripped by the sandbox, and equally disqualifying for
COMPLETED-C): one `register s32 a0_s7 asm("s7") = a0;` parameter pin at `src/text1a_pre.c:564`. No `__asm__` blocks
and no scheduling barriers. `src/text1a_pre.c` carries only 2 register pins file-wide, so attribution is unambiguous.

## 4. RESIDUAL DIAGNOSIS
Three distinct mechanisms, in descending weight:

- **Induction-variable re-anchoring (cluster 2, 4 rules).** The C already writes the intended form
  (`a4p = s3 + 0x68;` then `*(s16 *)(a4p + 0x10) = …`), but GCC's `loop.c` strength reduction rewrites the basic
  induction variable so the base lands on `+0x7C` — the *highest* referenced offset — and addresses the other two
  stores with negative displacements. Rules: **`strength-reduce-defeat`**, with
  **`walking-pointer-serializes-parallel-loads`** as the companion for holding the cursor at the low anchor.
- **Delay-slot / load-delay placement (cluster 1, 4 rules).** Target reads the table entry into a *separate*
  pseudo (`a0`), tests *that* for null, and moves it into `s4` in the branch delay slot; the `s4+44` computation
  then slides down to fill a later load-delay nop. Ours collapses the load and the object pointer into one pseudo,
  so there is nothing available for the delay slot and the `+44` gets hoisted into it instead. Rules:
  **`goto-end-prologue-delay-slot`**, **`param-local-alias-prologue-pair-flip`**. Note that regfix line 1796
  (`subst "nop" "addu $21,$20,44"`) is a **lost-codegen insert in `subst` clothing** — see
  `.claude/rules/lost-codegen-insert-cheat.md`. It must be closed by restructuring the C, and must **not** be
  re-spelled as a hardcoded-register `__asm__` (`.claude/rules/inline-asm-injection.md`).
- **Register allocation (clusters 3 + 4, 23 of the 34 rules).** The tail list-walk cursor is **callee-saved** in
  target (`s5`) and caller-saved in ours (`a2`) — target promoted it across the calls in that block and we did not.
  That is an allocation-class difference, not a naming one, and it drags the `s5`/`s6` swap along with it.
  Rule: **`register-alloc-pure-c`**.

## 5. FRONTIER (no `memory/wip/func_80040D48/` and no `memory/grind/func_80040D48/` — nothing killed on record)
1. **Split the table load from the null test** — highest confidence; targets cluster 1's 4 rules and the `s5`/`s6`
   swap at once. Write the lookup as a tested temporary, then assign the object pointer from it, so the object
   pointer is a *second* pseudo. That is the only shape that can produce target's
   `lw a0,0(at)` / `beqz a0` / `move s4,a0`. Then **move the `s5 = s4 + 0x2C;` statement later** (after the `a2[2]`
   store rather than between the `a2[0]` and `a2[1]` stores) so it sinks into the T28 load-delay slot instead of
   the branch delay slot.
2. **Re-anchor the case-0 loop cursor at the low offset** — targets cluster 2's 4 rules. In order of cheapness:
   (a) reorder the three stores inside the loop so the `+0x10` store is *last*, changing which reference the biv
   rewrite anchors on; (b) make `a4p` advance-then-use with a pre-biased base, per
   `walking-pointer-serializes-parallel-loads`; (c) add a use of `a4p` at displacement 0 inside the loop so `+0x68`
   becomes the natural anchor. Try (a) first — one-line reorder, no new construct, no reviewer exposure.
3. **Drop the `register s32 a0_s7 asm("s7")` pin and re-measure.** It is cheat-asm that has to go regardless, and
   `.claude/rules/register-asm-pins.md` records that pins routinely perturb allocation. The tail-block `a2` → `s5`
   promotion (cluster 3, 21 of the 34 rules) may be downstream of it. Measure the honest shape *without* the pin
   before attempting any RA work on cluster 3, so you are not solving an artifact.
4. **Only after 1–3**, run `tools/ra_solver` on the tail block for the `a2` → `s5` promotion. Carry the caveat
   recorded in `memory/wip/func_800858D0/meta.json`: the solver models global allocation exactly but was blind to
   local-alloc until its Phase 5, so confirm `local_extract.py` / `local_alloc.py` cover this block before
   concluding "no vector exists".

## 6. SIBLING LEADS
- The 0x68-strided joint-slot loop and the `D_800F66A0` function-pointer pair also appear in the **already-matched**
  function immediately preceding this one in `src/text1a_pre.c` (the `slot += 0x68` / `ent += 0x68` block ending at
  ~line 552). Same stride, same shape, already at zero — its cursor spelling is direct evidence for hypothesis 2.
- `D_800A9A10[]` is the shared per-entity object table. Grep `D_800A9A10` across `src/` for a matched function that
  does the same null-checked lookup; its spelling settles hypothesis 1 without a search.
