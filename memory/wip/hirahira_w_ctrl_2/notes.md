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

**Layer-2 `cheat-reviewer` returned FAIL.** Three grounds, all fair:
1. No `/* FAKE */` annotations on the staging locals or the relocated store.
2. No lever-exhaustion ledger for this exact form (round-2 notes said "do NOT commit").
3. **The substantive one — my stated mechanism does not explain `sinB`.** See below.
Per [[review-discipline-before-commit]] a FAIL is never bypassed. Escalated to the owner
for a construct ruling; do not re-submit without resolving item 3.

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

## THE OPEN MECHANISM QUESTION (blocks re-submission)

In `candidate_0` the store sits **after** `sinB`'s own cast:
```c
sinB = (s16)rawB;
a1[2] = sinB;
sinC = (s16)rawC;  sinA = (s16)rawA;  cosA = (s16)rawcosA;
```
By the cited mechanism only `rawC`/`rawA`/`rawcosA` are protected — yet the load census
shows **`sinB` is `lhu` too** (`lhu $11,Judge($6)`), and removing the store reverts
everything to 62/116. So the store is load-bearing for `sinB` as well, by some route that
"a memory write between the load and the cast" does not describe. Until that is explained,
this is an empirically-found arrangement, not an understood technique, and the reviewer is
right to refuse it. Resolve by dumping `.combine` / `.flow` for `rawB`'s pseudo and finding
what actually blocks `simplify_shift_const` — do NOT re-submit on the analogy alone.

## Next

1. Explain `sinB` (above). That is the gate.
2. Then: `/* FAKE */` annotations naming the exact pass per site, and an owner ruling on
   whether relocating a REAL required store for combine effect is a sanctioned family or a
   new one needing its own SOTN evidence pass ([[review-discipline-before-commit]]: a new
   family never ships in the match commit).
3. **Do NOT commit src.** Owner runs the gate.
