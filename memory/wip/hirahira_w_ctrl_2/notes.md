# hirahira_w_ctrl_2 — WIP (current state 2026-08-05, round 5) — BANKED at floor 62

`src/text1a_c.c:407`, stem **`text1a_c`** (not `text1a`). A 3x3 rotation-matrix builder:
six `Judge[]` reads, ten `mult`s, nine `a1[N]` halfword stores, no calls, no loop.
Measure: `wsl bash tmp/csz/hw_sweep.sh <variant>` (driver `tmp/csz/hw_var.py`), loads
`python tmp/csz/hw_loads.py <asm> <label>`, frame `wsl bash tmp/csz/gn_frame.sh text1a_c hirahira_w_ctrl_2`.

## STATUS: a byte-exact candidate exists but was FAILED by layer-2. NOT committed.

`candidate_0.diff` measures **sandbox `--disable all` = 0**, `build_insns` 122 == target,
`.frame $sp,48 vars=40 regs=2/0` == target, and `retire` dropped **all 63 regfix rules**
with full-build **SHA1 `62efab4f...` == oracle**. It also removes the pre-existing
volatile cheat. Tree reverted; `verify-oracle` re-confirmed `build_matches: true`.

**TWO independent layer-2 `cheat-reviewer` runs both returned FAIL. Do not re-submit
this construct without an owner-confirmed sanctioning pass.**

Round-3 FAIL: no annotations; no exhaustion ledger; and **the mechanism did not explain
`sinB`** — correct, and since resolved (below). Round-4 FAIL (fresh reviewer, mechanism
explained, annotations written, owner's provisional ruling disclosed): tests 1/2/3/4/5,
decisive point test 3 — the justification is `combine.c` internals, not program logic, and
explaining `sinB` "doubles down on the exact defect rather than curing it". It held that a
*provisional* owner ruling cannot substitute for independent clearance or a completed
SOTN-evidence pass. `next_action`: revert (done), function stays INCOMPLETE.

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

## Measured ordering constraints (rounds 2-3)

Cited precedent: the SIBLING `replay_camera_rob_back_loose3` (`src/text1a_c.c:336`, banked
at 12, zero rules/pins/volatile), documented at `src/text1a_c.c:283-297`.

- **Staging alone is inert** — 62/116, all six still `lh`.
- **The relocated store is the whole lever** — taking it back to the tail: 0 -> 62, 122 -> 116.
- **Each angle's masked (sin) read must precede its sign-extended (cos) read**, else the
  param load folds to `lh` and re-loads for the mask.
- **cosB/cosC need their multiply BEFORE the store**, else they are protected too and all
  six become `lhu` (42/125).
- Ladder: probe1 -> probe2 (121 insns) -> probe3 (vars 40) -> +cosAstage (19) -> probe5 (0/122).

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

## Round 5 — the natural-shape round (bounded, run 2026-08-05). NOTHING LANDED.

Tested the lead's compass: program-logic shapes where the raw value genuinely has a second
use or the store order follows the data flow. Driver `tmp/csz/hw_m2.py`, runner
`hw_m2run.sh`; all forms are volatile-free with **no staging locals at all**.

The honest second use exists and is real: **m[2] IS sinB**, so `a1[2] = sinB;` is a genuine
16-bit use of the HImode value — no `u16 rawB` needed. Store-position sweep on the
otherwise-untouched body: right after the `sinB` read **66**/115; after all three sines
**87**/118; after the cos pair **63**/122 (correct insn count); **after the first product
`cosB_cosC` — 22/119**, the best natural form found.

**But the "emit each element as it becomes known" idiom does NOT converge.** Interleaving
the other element stores at their earliest valid points makes it worse, not better:
`+a1[0]` 46/117, `+a1[1]` 49/117, `+a1[5]` 49/117, `+a1[8]` 49/117, and `a1[0]+a1[1]`
without the m[2] move 38/116. A full rewrite into consistent compute-and-store order
(`tmp/csz/hw_nat.py`) is 91/114. So target is NOT reproduced by the uniform natural idiom —
only by one store at one specific position, which is precisely the double-FAILed construct.

**Reported floor stays 62** (volatile removed, no statement repositioning). The 22/119 form
is recorded as a measurement, NOT a floor: it is cheat-free of staging locals and volatile,
but it still rests on repositioning one store for combine effect, i.e. the same category
two reviewers rejected. Do not treat it as bankable without the family ruling.

## Standing observation for whoever picks this up

Target's own bytes contain `lhu` + `sll 16` + `sra 16` at four sites. Our compiler folds
that chain to `lh` whenever the load has a single use and no memory write intervenes. So
**the original source must itself have contained something that prevented the fold** — the
question is not whether to prevent it but what natural C did so. That does not make the
current construct acceptable (two reviewers say it is not); it means the search should look
for the program-logic shape that has this effect, per the round-4 `next_action`.

## Next — BANKED at floor 62; do not sink further rounds here

1. Blocked on the owner's family ruling. Do NOT re-submit the store-relocation form
   (two independent FAILs) and do NOT treat 22/119 as a floor.
2. If the family is sanctioned: a separate SOTN evidence pass, its own review,
   owner-confirmed, before it can back any completion — never in the match commit.
3. If it is NOT sanctioned, the natural-idiom space is measured-negative (round 5) and the
   next untried axis is the `Judge` declared type itself (`extern s16 Judge[]` at four
   sites in this file) under [[header-type-correction-from-use-sites]]'s four prongs —
   a `u16` table would make the sign-extension explicit at every use site by TYPE rather
   than by statement position. That is a header change, so it needs its own review.
4. **Do NOT commit src.** Owner runs the gate.
