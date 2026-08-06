# hirahira_w_ctrl_2 — WIP (current state 2026-08-05, round 3)

`src/text1a_c.c:407`, stem **`text1a_c`** (not `text1a`). A 3x3 rotation-matrix builder:
six `Judge[]` reads, ten `mult`s, nine `a1[N]` halfword stores, no calls, no loop.
Measure: `wsl bash tmp/csz/hw_sweep.sh <variant>` (driver `tmp/csz/hw_var.py`), loads
`python tmp/csz/hw_loads.py <asm> <label>`, frame `wsl bash tmp/csz/gn_frame.sh text1a_c hirahira_w_ctrl_2`.

## STATUS: a byte-exact candidate exists but was FAILED by layer-2. NOT committed.

`memory/wip/hirahira_w_ctrl_2/candidate_0.diff` measures:
**sandbox `--disable all` = 0**, `build_insns` 122 == target 122, `.frame $sp,48 vars=40
regs=2/0` == target, and `retire` dropped **all 63 regfix rules** with full-build
**SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle**. It also REMOVES the
pre-existing volatile-coercion cheat. Tree was reverted (`src/text1a_c.c` + `regfix.txt`)
and `verify-oracle` re-confirmed `build_matches: true`.

**TWO independent layer-2 `cheat-reviewer` runs both returned FAIL. Do not re-submit
this construct without an owner-confirmed sanctioning pass.**

Round-3 FAIL (three grounds): no annotations; no lever-exhaustion ledger; and **the
stated mechanism did not explain `sinB`**. The third was correct and I resolved it
(mechanism now fully derived from the RTL — see below).

Round-4 FAIL (fresh reviewer, mechanism explained, annotations written, owner's
provisional ruling disclosed to it): it FAILED on tests 1/2/3/4/5 and stated that the
owner's ruling, being provisional, **cannot substitute for an independent clearance or
for a completed SOTN-evidence pass for this family**. Its decisive point is test 3:
the entire justification — mine and the in-source comment's — is `combine.c` internals
(LOG_LINKS, `can_combine_p`, insn numbers, sched1's hoist), not program logic, and
resolving `sinB` "doubles down on the exact defect rather than curing it". It also
notes the form was found by probe search and rationalised afterwards.

`next_action`: revert (done), keep the function INCOMPLETE, and if the family is to be
sanctioned it needs its own SOTN-master-branch evidence pass submitted SEPARATELY from
any match commit and owner-confirmed first.

## Baselines

| form | score | insns (target 122) | frame |
|---|---|---|---|
| HEAD (carries the volatile cheat) | 58 | 118 | vars 48 |
| honest (volatile removed) | **62** | 116 | vars 48 |
| candidate_0 | **0** | 122 | vars 40 |

## The load-form mapping (solved, round 2)

Target's six `Judge` reads by index-expression provenance (`$a2`=`a0[1]`=angB,
`$t0`=`a0[2]`=angC, `$v1`=`a0[0]`=angA): **cosB (idx18) and cosC (idx21) are `lh`**;
sinB (29), sinA (36), sinC (45), cosA (63) are **`lhu` + `sll 16` + `sra 16`**. cosB/cosC
are exactly the pair feeding the first `mult` (idx23). The honest build emits all six as
`lh`. The three `a0[N]` param reads already match target (3x `lhu`, two sign-extended).

## The lever, and the ordering constraints (measured)

Cited precedent: the SIBLING `replay_camera_rob_back_loose3` (`src/text1a_c.c:336`, banked
at 12, zero rules/pins/volatile), whose comment block at `src/text1a_c.c:283-297` documents
a `u16` staging local + a REAL store moved between the raw load and the `(s16)` cast:
`can_combine_p` refuses to fold a MEM load into a later user across an insn that may write
memory, so `simplify_shift_const` never collapses
`ashiftrt(ashift(zero_extend(mem),16),16)` into `sign_extend(mem)` = `lh`. sched1 runs
after combine and hoists the store back out, so it costs nothing — which is why target's
nine `sh` stores all sit at the tail (idx 104-127).

Measured constraints, in the order they were discovered:

- **Staging alone is inert.** `honest+u16sin` / `honest+u16all` = 62/116, all six still `lh`.
- **The relocated store is the whole lever.** Taking `a1[2] = sinB;` back to the tail from
  the candidate: **0 -> 62 and 122 -> 116 insns**. Nothing else changed.
- **Each angle's MASKED (sin) read must precede its sign-extended (cos) read**, or GCC folds
  the param load to `lh` and re-loads for the mask (probe1: 11 halfword loads, vars 56;
  probe2: one param read still `lh $3,2($4)`).
- **cosB/cosC must have their sign-extending USE (`cosB_cosC = cosB * cosC`) before the
  store**, else they are protected too and become `lhu` — probe4 put one store ahead of
  that multiply and got **all six** as `lhu`, 42/125.

Probe ladder: probe1 (flip confirmed, +2 param loads) -> probe2 (3 `lh`/3 `lhu`, 121 insns)
-> probe3 (vars 40, 30/119) -> probe3+cosAstage (19/120) -> **probe5 (0/122)**.

## The mechanism, now fully derived (round 4) — TWO sub-mechanisms, not one

From the candidate's `.combine` dump (`wsl bash tmp/csz/gn_da.sh text1a_c`, then
`wsl python3 tmp/csz/hw_rtl.py tmp/csz/rtl_text1a_c/text1a_c.i.combine hirahira_w_ctrl_2`):

- **sinB — multi-use, not the memory barrier.** `a1[2] = sinB;` stores 16 bits of a
  sign-extension of `rawB`, which IS `rawB`, so RTL-gen emits the store as a direct use of
  the HImode variable: insn 113 is
  `(set (mem/s:HI (plus (reg/v:SI 73) (const_int 4))) (reg/v:HI 77))`. `rawB` (reg 77) then
  has two uses — insn 113 and the shift at insn 126 — so **no LOG_LINK is built from 126
  back to the load at insn 29 (insn 126's LOG_LINKS is literally `(nil)`)** and combine
  never attempts the fold.
- **sinC / sinA / cosA — the sibling's barrier.** Their casts sit after insn 113, so
  `can_combine_p` refuses to fold the MEM load across an insn that may write memory.

**Control (decisive):** move `a1[2] = sinB;` back to the tail, everything else identical →
the `.combine` dump shows **6x `sign_extend(mem)` and ZERO surviving `movhi` loads**, and
the `a1[2]` store becomes `(subreg:HI (reg:SI 153) 0)` — it consumes the already
sign-extended value, so `rawB` has a single use. Score 0 -> 62, insns 122 -> 116.

## Standing observation for whoever picks this up

Target's own bytes contain `lhu` + `sll 16` + `sra 16` at four sites. Our compiler folds
that chain to `lh` whenever the load has a single use and no memory write intervenes. So
**the original source must itself have contained something that prevented the fold** — the
question is not whether to prevent it but what natural C did so. That does not make the
current construct acceptable (two reviewers say it is not); it means the search should look
for the program-logic shape that has this effect, per the round-4 `next_action`.

## Next

1. Do NOT re-submit the store-relocation + staging form. Two independent FAILs.
2. If the family is to be sanctioned: a separate SOTN-master-branch evidence pass, its own
   review, owner-confirmed, BEFORE it can back any completion
   ([[review-discipline-before-commit]] — never in the match commit).
3. Otherwise keep searching for a program-logic-motivated structure. Honest floor stays 62.
4. **Do NOT commit src.** Owner runs the gate.
