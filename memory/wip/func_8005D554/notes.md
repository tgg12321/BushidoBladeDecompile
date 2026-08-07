# gnd_land_hit_char_tsuba — WIP (current state 2026-08-05, harvest rotation 6)

`src/text1b.c:12751`. HEAD baseline: honest pure-C distance **65**, 88 rules, canonical
ASM-SUSPECT. Measure: `wsl bash tmp/csz/gn_score.sh` (variant driver `tmp/csz/gn_v3.py`,
sweep `tmp/csz/gn_sweep2.sh`), frame census `wsl bash tmp/csz/gn_frame.sh text1b <func>`.

## Candidates (banked as diffs; tree left clean)

| file | score | insns | frame |
|---|---|---|---|
| `candidate_56.diff` | 56 | 178 vs 176 | vars 56 (target 64) |
| `candidate_40.diff` | 40 | 176 vs 176 | vars 56 (target 64) |
| `candidate_8.diff` | **8** | **176 vs 176** | **vars 64 == target** |

`candidate_8` = `candidate_40` + the loop guard written in the loop variable
(`if (i < ((D_800A326C + 1) * 2))` instead of `if (((D_800A326C + 1) * 2) > 0)`)
+ a named displaced pointer (`p_b2ec = p_b2e0 + 0xC; base_offset = (s32*)(p_b2ec + stride)`).
Guard alone: 40 -> 10. Adding `p_b2ec`: 10 -> 8.
Still needs the family's `/* FAKE */` annotation on the split-init accumulation
before any commit — nothing was committed.

## THE FRAME DELTA IS CLOSED — mechanism proven, not inferred

New instrument (this session): a **FRAMEDBG hook in cc1's `assign_stack_local`**
printing every frame allocation with a caller tag (`stack_temp` / `spill_new_p<N>` /
`spill_grow_p<N>` / `put_reg_into_stack` / `assign_parms_*` / `round_frame`).
Patch recorded in `tools/ra_solver/cc1_hooks.patch.md` §5; parity-checked OK on
text1b + main. Census tool: `tmp/csz/gn_census.py <framedbg.log> <asm.s> --only-phantom`
(pairs allocations against emitted sp traffic, treats `addiu $rX,$sp,K` as address-taken).

At candidate_40 our 56 bytes decomposed EXACTLY as:
`stack_temp 48B` (the `S46C s` local, 44 rounded to 8) + `spill_new_p88 8B`.
Target's 64 = the same two + **one more 8-byte block that no instruction touches**
(target sp[72..79); confirmed no `lw/sw` and no `addiu $rX,$sp,72`).

**Producer, measured on an in-tree witness (`func_8004954C`, same TU):** a comparison
pseudo BORN as a reg-vs-reg `slt` at RTL-generation that **combine later folds into a
zero-compare**. In the witness `.flow` holds `(set (reg 80) (lt (reg 75) (reg 73)))` +
`(if_then_else (eq (reg 80) 0) ...)`; combine const-props reg 75 = 0 and rewrites the
branch to `(le (reg 73) (const_int 0))` — a bare `blez`. Pseudo 80 keeps `reg_n_refs > 0`
but has **zero surviving RTL refs and an empty conflict list**, so `global_alloc` skips it
(`reg_live_length < 0`) and reload's `alter_reg` gives it an 8-byte slot nothing accesses.
Signature: `;; N conflicts:` empty AND N absent from `;; Register dispositions:`
(`tmp/csz/gn_orph2.py <dump>.greg [func]`).

Our literal-zero guard `(...) > 0` is folded by the FRONT END, so no pseudo is ever born
(the fold/emit dichotomy). Writing the guard in `i` — what jump.c's
`duplicate_loop_exit_test` produces from a `for`/`while` loop — births the pseudo, combine
folds it back to the identical `blez`, and the slot appears **at zero instruction cost**:
176 insns both sides, `vars` 56 -> 64, `.frame` 112 -> 120. This supersedes the round-5
"phantom producer #1 is unreachable here" negative, which tested spellings of the guard
*expression* rather than its *operand form*.

## Residual at score 8 — three groups, no RA renames left

1. `target addu v0,s0,s8` vs `ours addu v0,s8,s0` — operand order of
   `s.p0 = (void *)(p_b2e0 + stride)` (1 insn).
2. ×2 (one per loop half): target fills the `lw v1,%gprel(D_800A3418)` load-delay slot
   with `addiu a2,s4,-K` and hoists `move a1,zero` earlier; ours issues `addiu a2,s4,-K`
   before `addiu a0,sp,16` and fills the slot with `move a1,zero`.
3. `target lui/addiu %hi/%lo(D_8009B390)` vs `ours lui %hi(D_8009B388); addiu a3,a3,8`.

## Settled negatives — do NOT re-run

- Everything in the round-5 list still holds (S46C is not under-sized; pointer
  reassociation of the `p_b2e0 + stride + 0xC` chain is inert; def placement cannot buy
  refs; the exec_game identity/holder levers do not transfer). Phantom producer #2
  (combine orphan-USE) has no site — combine emits zero bare `(use (reg N))` here.
- **`p_b390 = p_b388 + 2` is load-bearing** (mechanism in the RA-spec section below).
  Measured: `&D_8009B390` with p_b390 kept 43/174; no p_b390 variable 45/174; both
  pointers from symbols, either birth order 43/174; `p_b388` derived from `p_b390`
  14/176; `&p_b388[2]` 8/176 (identical to `+ 2`).
- **Base_offset association is exhausted except the named form.** `p_b2e0 + 0xC + stride`,
  `(p_b2e0 + 0xC) + stride`, `stride + (p_b2e0 + 0xC)` are all inert at 10; only the
  named `p_b2ec` intermediate moves it (to 8).
- **`s.p0` operand order resists spelling**: `stride + p_b2e0` / `&p_b2e0[stride]` stay at 8.
- **Split-init accumulation on all four offset statements is still optimal** under the new
  guard (re-measured, since the old sweep predates it): re-fusing site 1 or 3 gives
  11/177, site 2 or 4 gives 9/176, sites 2+4 gives 16/176, sites 1+3 gives 16/178.
- **Reversing the accumulation order** (start from the `D_800A3418` term) is worse:
  a2 sites 16/176, a0 sites 16/178, both 24/178.
- **A real `while` loop** also reaches `vars=64` but costs 2 instructions (29/174) —
  loop.c hoists differently. The `if (i < ...) do {...} while (i < ...)` form is the one
  that keeps 176.

## Group 3 has an exact RA spec now (ra_solver re-run at candidate_8: 18/18 exact)

candidate_8: p85 (`p_b388`) refs 4 livelen 99 **pri 425** -> hard 23; p87 (`p_b2e0`)
refs 4 livelen 102 **pri 412** -> hard 30; p89 **pri 315** and p86 **pri 161**
unallocated (p89 is the `64(sp)` spill, p119 is the guard phantom).

Symbol-named variant (`a4+c1`), also 18/18 exact: p85 loses the `p_b390 = p_b388 + 2`
reference, refs 4 -> 3, **pri 425 -> 159**, so it falls below p87. p87 then takes 23,
**p89 (315) takes 30**, and the spill disappears — that is the whole 176 -> 174.

So the spec is: **p85 must stay above p87's 412 while `p_b390` is symbol-named**, which
needs a fourth *loop-depth-1* reference to `p_b388` (the def is worth 1, the in-loop
`s.p1 = p_b388` is worth 2). Lowering p87 instead does not work: dropping it to refs 3
puts both below p89's 315, and p89 would then be allocated. No natural fourth depth-1
use of `p_b388` exists in the current variable set — every candidate is a dead store.
Read that as evidence the ORIGINAL's variable structure around `D_8009B2E0`/`base_offset`
differs from ours, and search there rather than respelling the `p_b390` line.

## sched_solver verdict on group 2: INCONCLUSIVE, not clean

`perturb.py --func gnd_land_hit_char_tsuba --goal-from-target text1b` prints no differing
blocks for either pass — but that is a tool gap, not a proof. Its own summary reads
`cc1=168 hon=173 tgt=176` (honest stream 173 where the sandbox measures 176) and
`align hon->tgt` reports `moved 0` where we can see two moved pairs, so the goal mapper
failed to align this function and emitted no goals. The one usable signal is its
`filled delay slots` line: four ours/target pairs that are not a constant offset
(48/101, 102/91, 170/161, 365/352), pointing at a **reorg.c delay-slot fill** difference,
which the toolkit explicitly cannot model.

## Next

1. Fix or work around the goalmap alignment for this function before trusting any
   scheduling verdict; otherwise attack group 2 as a reorg.c fill choice.
2. Group 3: hunt the original's `base_offset` / `p_b2e0` structure per the spec above.
3. **Do NOT commit src.** The owner runs the gate.
