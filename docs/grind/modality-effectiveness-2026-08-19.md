# Grinder modality effectiveness — a data pass over the journal

**Scope:** `docs/grind/journal.md` (1068 lines, 2026-07-06 → 2026-08-19), `docs/grind/decisions.md`
(249 judge decisions), `git log` Match: commits, and the offline `bb2_metrics` Postgres layer for
permuter cost telemetry.
**Read-only.** No tracked file was modified. Scripts used: `tmp/research/analyze{,2,3,4}.py`,
`closer.py`, `floorbefore.py`, `cov.py`, `dump_fails.py`, `verify_closer.py`.

---

## 0. Read this first — two facts that change how every table below should be read

**(a) Modality is almost perfectly collinear with session number.** The driver's ladder
(`tools/grinder/grindlib.py:20`) is a fixed rotation starting at session 2:

```
LADDER = [structural, structural, permuter, permuter, forensics, forensics, rederive, rederive, synthesis]
```

Empirically that is exactly what ran — 95/95 recon at s1, 150/154 structural at s2–s3, 86/94 permuter
at s4–s5, and so on. So a raw "modality X has a Y% drop rate" number is *mostly* measuring **how deep
into a function the ladder had gotten**, not the modality's intrinsic power. Every comparison below is
either explicitly deconfounded or explicitly flagged as confounded.

**(b) The session that produces the match emits no `floor=` line.** For all 46 completed functions that
have session lines, the last logged session number is exactly `declared_count - 1` (44/46; 2 off by
1–2). Functions completing in 1 session (40 of them) and most 2-session completions (65 of 80) have
**no session lines at all**. Consequence: **the floor-drop tables measure only the sessions that did
*not* finish the function.** They are survivor-biased against every modality, and most heavily against
whichever modality tends to close. Section 3 reconstructs the closing session separately, and it
reverses the naive permuter conclusion.

---

## 1. Per-modality floor movement (raw — confounded, shown for completeness)

661 transitions where a session had a prior session on the same function.

| modality | sessions (n) | dropped floor | drop rate | mean drop | median drop | max drop | regressions |
|---|---|---|---|---|---|---|---|
| structural | 219 | 43 | **19.6%** | 6.30 | 4.0 | 23 | 2 |
| permuter | 155 | 11 | 7.1% | 5.64 | 3.0 | 20 | 2 |
| forensics | 138 | 6 | 4.3% | 3.67 | 3.5 | 5 | 1 |
| rederive | 103 | 3 | 2.9% | 3.00 | 2.0 | 6 | 0 |
| synthesis | 46 | 4 | 8.7% | 7.00 | 7.5 | 9 | 0 |
| **all** | **661** | **67** | **10.1%** | 5.85 | 4.0 | 23 | 5 |

`recon` never appears here — it is always session 1, so it has no prior floor to move.

### Drop rate by session position (the confound, made visible)

| modality | s2 | s3 | s4–5 | s6–10 | s11–20 | s21+ |
|---|---|---|---|---|---|---|
| structural | 28/71 (39%) | 15/68 (22%) | — | 0/3 | 0/35 | 0/40 |
| permuter | 0/1 | 0/1 | 11/86 (13%) | 0/1 | 0/22 | 0/44 |
| forensics | 1/1 | 0/1 | 0/7 | 4/68 (6%) | 1/18 (6%) | 0/43 |
| rederive | — | — | 0/1 | 3/47 (6%) | 0/15 | 0/40 |
| synthesis | — | — | — | 4/20 (20%) | 0/8 | 0/18 |
| **all** | **29/73 (40%)** | **15/70 (21%)** | **11/94 (12%)** | **11/139 (8%)** | **1/98 (1%)** | **0/185 (0%)** |

**Session depth, not modality, is the dominant variable.** The drop rate falls monotonically from 40%
at s2 to 0% past s15, and no modality escapes it.

### Where the floor reduction actually comes from

| band | transitions | drops | total floor points removed |
|---|---|---|---|
| s2–s3 (structural) | 143 | 44 | **276** |
| s4–s5 (permuter) | 94 | 11 | 62 |
| s6–s7 (forensics) | 73 | 5 | 21 |
| s8–s9 (rederive) | 48 | 3 | 9 |
| s10 (synthesis) | 18 | 3 | 21 |
| s11–s19 (2nd cycle) | 90 | 1 | 3 |
| s20+ | 195 | 0 | **0** |

**86% of all floor reduction (338 of 392 points) happens in sessions 2–5**, which are only 36% of
sessions. Past s15, across 227 transitions and five modalities, the floor moved **zero** times.

---

## 2. Deconfounded: the second ladder cycle

In cycle 2+ (s11 onward) all five modalities rotate at comparable depth, so they are directly
comparable. This is the cleanest within-project comparison available.

| modality | cycle 1 (s2–s10) n / drops / rate | cycle 2+ (s11+) n / drops / rate |
|---|---|---|
| structural | 142 / 43 / **30.3%** | 77 / 0 / 0.0% |
| permuter | 89 / 11 / 12.4% | 66 / 0 / 0.0% |
| forensics | 77 / 5 / 6.5% | 61 / 1 / 1.6% |
| rederive | 48 / 3 / 6.2% | 55 / 0 / 0.0% |
| synthesis | 20 / 4 / **20.0%** | 26 / 0 / 0.0% |
| **all** | 376 / 66 / 17.6% | **285 / 1 / 0.4%** |

**The second and later ladder cycles are worth essentially nothing.** 285 sessions produced one drop
of 3 points. This is the single strongest signal in the dataset, and it is modality-independent — the
ladder repeating is the problem, not which rung it repeats.

Restricting further to the 27 functions that ran ≥2 distinct modalities after s6 (controlling for the
function as well as the depth) gives the same ordering: synthesis 4/46 (8.7%), rederive 3/102 (2.9%),
forensics 2/110 (1.8%), structural 0/80, permuter 0/67.

---

## 3. Who actually closes functions (correcting for fact (b))

Because the closing session emits no floor line, I reconstructed it from the ladder: a function
completing in N sessions was closed by `recon` if N=1, else `LADDER[(N-2) % 9]`. Then I read the floor
entering that final session, from the last logged session, to separate *genuine* closes from
*landing* sessions (where the answer was already at floor 0 and the last session only committed it).

| closer modality | completions closed | share | share of ≥4-session completions | entered final session at floor 0 (a "landing", not a close) |
|---|---|---|---|---|
| structural | 90 | 60% | 1/22 (5%) | 0 of 25 measurable |
| recon (s1) | 40 | 26% | — | n/a |
| **permuter** | **13** | **9%** | **13/22 (59%)** | **4 of 13** |
| forensics | 4 | 3% | 4/22 (18%) | 0 of 4 |
| rederive | 3 | 2% | 3/22 (14%) | 0 of 3 |
| synthesis | 1 | 1% | 1/22 (5%) | 0 of 1 |

Combining drops and closes into one "win rate" per modality:

| modality | logged sessions | floor drops | closes | total sessions | wins | win rate |
|---|---|---|---|---|---|---|
| recon | 0 | 0 | 40 | 40 | 40 | 100% |
| structural | 219 | 43 | 90 | 309 | 133 | **43.0%** |
| permuter | 155 | 11 | 13 | 168 | 24 | 14.3% |
| synthesis | 46 | 4 | 1 | 47 | 5 | 10.6% |
| forensics | 138 | 6 | 4 | 142 | 10 | 7.0% |
| rederive | 103 | 3 | 3 | 106 | 6 | 5.7% |

**This is the finding I would not have predicted from section 1.** Permuter's per-session floor-drop
rate is a weak 7%, but the permuter slots (s4/s5) are where 13 of the 22 hard (≥4-session) completions
landed — and 9 of those 13 entered the session at a non-zero floor (2, 4, 4, 4, 5, 5, 6, 9, 12), so
they are genuine closes, not landings. **Permuter looks bad on floor movement and good on finishing.**
Section 5 weighs those against each other.

*Confidence: medium.* The closer modality is inferred from the ladder, not observed — `ladder_skip`
and the 2026-08-18 permuter gate can shift the mapping, per-function ledgers are deleted on completion
so I could not verify directly, and a session mandated `permuter` may have closed the function by
structural means. The floor-entering-0 check removes the most obvious false credits (4 of 13) but not
all of them.

---

## 4. Plateau breaking

Drop rate as a function of how many consecutive flat sessions preceded it. **Confounded by depth**
(a long flat run means a deep session), so read the *relative* ordering within a column, not the
absolute numbers.

| modality | after 0–1 flat | after 2 flat | after 3+ flat | after 5+ flat |
|---|---|---|---|---|
| structural | 43/146 (29%) | 0/2 | 0/71 (0%) | 0/70 (0%) |
| permuter | 7/42 (17%) | 3/33 (9%) | 1/80 (1%) | 0/59 (0%) |
| forensics | 3/17 (18%) | 1/16 (6%) | 2/105 (2%) | 0/70 (0%) |
| rederive | 2/3 (67%) | 0/2 | 1/98 (1%) | 0/82 (0%) |
| **synthesis** | 0/2 | — | **4/44 (9%)** | **3/41 (7%)** |
| **all** | 55/210 (26%) | 4/53 (8%) | 8/398 (2%) | 3/322 (1%) |

**Synthesis is the only modality with a non-zero drop rate deep into a plateau.** It is also the only
modality whose yield does *not* collapse: 4/44 after 3+ flat sessions against an all-modality baseline
of 8/398 (2%), and 3/41 after 5+ flat against a baseline of 3/322 (1%) — of which synthesis *is* all
three. Its drops are also the largest in the dataset (mean 7.0, median 7.5, individual drops of
12→5, 18→9, 11→3, 6→2).

Counter-evidence, and it matters: synthesis's yield is **entirely** in its first slot. At s10 (the
first synthesis rung) it is 3/18; at s>10 it is **0/26**. So the honest reading is not "synthesis
breaks plateaus" but "**the first synthesis pass breaks plateaus; repeats do not**" — same decay
every other modality shows, just from a higher starting point and one cycle later.

*Confidence: low-to-medium on the magnitude* (n=46 transitions, 4 drops total), *high on the
direction* (it is the only modality with any deep yield at all, and the effect survives the
depth control in section 2, where synthesis leads cycle 1 at 20% behind only structural).

---

## 5. Permuter specifically

The 2026-08-18 gate (`grindlib.py:680-697`) skips **all** subsequent permuter slots once the first
permuter session on a function was zero-yield. The data supports going further.

**Yield decays hard by occurrence, and is zero after the second:**

| permuter session for a function | n | drops | rate |
|---|---|---|---|
| 1st | 50 | 8 | 16.0% |
| 2nd | 41 | 3 | 7.3% |
| **3rd or later** | **64** | **0** | **0.0%** |

By exact slot: s4 = 8/46 (17.4%, mean drop 6.5), s5 = 3/40 (7.5%, mean drop 3.3), s6–s11 = 0/1,
**s12+ = 0/66**.

**Retrospective counterfactual — the current gate applied to the whole history:**

- would skip **97 of 155** permuter transitions (63%)
- keeps **9 drops totalling 58 floor points** (all at s4/s5 first-permuter)
- loses **2 drops totalling 4 floor points** (`ang_hosei_80056FE8` s5 −1, `motion_Close` s5 −3)

**A stricter gate — one permuter session per function, ever:**

- skips **105** transitions
- loses **3 drops totalling 10 points** (the two above plus `func_8001979C` s5 −6)
- i.e. 8 more sessions saved than the current gate, at a cost of 6 additional floor points

**Cost, from the metrics DB** (independent of the journal): 322 `permuter-launch`, 355
`permuter-harvest`, 1615 `permuter-wait` events across 69 functions. **249 of 322 launches (77%) went
to functions that never completed** — concentrated on `cpu_side_move_dir_4` (27), `func_80045294`
(15), `marionation_Exec` (12), `saEft01Init` (11), `func_8007DC9C` (10), `func_8002EA24` (10). 83% of
all permuter sessions were spent on functions permuter never once helped.

**But — and this is the tension — permuter closed 13 of 22 hard completions (§3), 9 of them genuinely.**
Those closes are concentrated in the s4/s5 first-permuter slots that both gates preserve. So the
recommendation is not "cut permuter"; it is "cut permuter *repeats*", which the data says are worth
literally zero past occurrence #2.

*On whether the 2026-08-18 gate can be evaluated yet: no.* Only 8 permuter transitions have run since
it landed (2 drops). That is far too few to say anything; the case for tightening rests entirely on
the pre-gate history above.

---

## 6. FAIL pattern

### Judge decisions (`docs/grind/decisions.md`), by week

FAIL classification is **hand-done for all 34 August FAILs** (I read each ruling's lead paragraph);
the keyword classifier I first wrote over-counted the citation bucket and I discarded it.

| week (Mon) | PASS | FAIL | fail rate | construct-class | citation/paperwork-class | other |
|---|---|---|---|---|---|---|
| 2026-08-03 | 8 | 8 | 50% | 8 | 0 | 0 |
| 2026-08-10 | 15 | 15 | 50% | 13 | 1 | 1 (bytes-not-proven) |
| 2026-08-17 | 6 | 11 | 65% | 7 | 4 | 0 |
| **August total** | **30** | **34** | **53%** | **28** | **5** | **1** |
| July (for baseline) | 149 | 37 | **20%** | ~27 | **0** | ~10 (scope/authority) |

The five citation/paperwork-class August FAILs, in full — this is the exact population today's fixes
target:

1. `2026-08-08 func_80021A98` — constructs substantively legitimate, self-vet's C1 family citation
   quotes text that does not appear at the cited location.
2. `2026-08-13 func_80034F88` — four `u8 *q = &D_80106A73;` pointer aliases missing the `/* FAKE */`
   annotation the worker's own cited rule mandates.
3. `2026-08-17 func_800453E0` — "otherwise well-documented pure-C recovery" but cites a **nonexistent
   rule file** as the precedent for a load-bearing step.
4. `2026-08-19 func_8001B748` (ruling) — family *is* on the frozen list; the spelling misses its
   mandatory prongs (c) and (d).
5. `2026-08-19 func_8002D518` — load-bearing FAKE construct cited under the wrong sanctioned family;
   the correct family was never invoked or checked.

**Baseline for next week: 5 of 34 (15%) of August judge FAILs, and 4 of 11 (36%) in the week of
08-17, were citation/paperwork-class.** July was 0 of 37 — this failure mode did not exist before the
self-vet artifact (`grindlib.py:34`, 2026-08-07) created a document that can itself be wrong.

Two cautions on that baseline. First, a large share of the 28 construct-class FAILs *also* mention a
mismatched family citation — but there the citation is the **disguise for** the cheat ("laundered
through a mis-scoped family citation", "relabeled to dodge the substring banned-construct check"), not
the defect. Fixing citation hygiene will not convert those to PASSes, and if next week's numbers show
citation-class falling while construct-class holds, that is the expected outcome, not a failure of the
fix. Second, the July→August fail-rate jump from 20% to 53% is **not** attributable to the citation
class (0 → 5); it tracks the queue moving onto harder functions and the concentration below.

### Concentration

FAILs are not spread evenly. **24 of 71 all-time judge FAILs (34%) fall on the six functions with ≥3
FAILs each**: `func_80021A98` (5), `func_80060A68` (5), `func_8001B138` (4), `func_800645B0` (4),
`func_80037540` (3), `func_8007C7A0` (3). These are respelling loops — the same construct resubmitted
under successive family labels (the `func_80021A98` ledger shows a construct submitted three times
under three labels explicitly to dodge the substring ban check).

### Journal LAYER-1 / judge FAIL lines

| week | FAILs logged |
|---|---|
| 2026-08-03 | 8 |
| 2026-08-10 | 13 |
| 2026-08-17 | 6 (partial week, through 08-19) |

All-time: 35 lines, 8 in July, 27 in August.

---

## 7. Sessions to completion

From `git log` Match: commits carrying a grinder session count (151 unique functions; the other 99
`Match:` commits are pre-grinder or operator-integrated and carry no count). The journal's
"COMPLETED-C after N sessions" lines give an identical distribution.

| sessions | functions | share | cumulative |
|---|---|---|---|
| 1 | 40 | 26.5% | 26.5% |
| 2 | 80 | 53.0% | **79.5%** |
| 3 | 9 | 6.0% | 85.4% |
| 4 | 11 | 7.3% | 92.7% |
| 5 | 2 | 1.3% | 94.0% |
| 6 | 3 | 2.0% | 96.0% |
| 7 | 1 | 0.7% | 96.7% |
| 8 | 2 | 1.3% | 98.0% |
| 9 | 1 | 0.7% | 98.7% |
| 10 | 1 | 0.7% | 99.3% |
| 11 | 1 | 0.7% | 100% |

mean 2.33, median 2, max 11. **Four out of five functions finish inside the two structural rungs.**

Trend — the queue is getting harder as the easy items drain:

| month | completions | mean sessions | needing ≥4 sessions |
|---|---|---|---|
| 2026-07 | 128 | 2.16 | 16 (12%) |
| 2026-08 | 23 | 3.26 | 6 (26%) |

Unique-function match throughput: May 13, June 59, July 133, August 45.

### The cost side

| | functions | sessions | share of all sessions |
|---|---|---|---|
| completed (with logged sessions) | 46 | 126 | 16% |
| never completed | 60 | 641 | **84%** |
| never completed, ≥20 sessions each | 8 | 344 | **45%** |

Those eight:

| function | sessions | floor first → last | sessions that moved it |
|---|---|---|---|
| cpu_side_move_dir_4 | 101 | 2 → 2 | **0** |
| marionation_Exec | 56 | 4 → 4 | **0** |
| func_80045294 | 45 | 2 → 2 | **0** |
| func_8007DC9C | 40 | 9 → 9 | **0** |
| main | 32 | 2 → 0 | 1 |
| func_80057CC8 | 27 | 3 → 3 | **0** |
| func_80017848 | 22 | 16 → 3 | 5 |
| func_80034F88 | 21 | 23 → 10 | 3 |

Six of the eight have a floor that **has never moved once**, across a combined 279 sessions.

---

## 8. Recommendations

Each carries the numbers and an explicit confidence. All are ladder/scheduling changes — none touches
the completion standard, the no-deferral directive, or the Judge's authority.

### R1 — Stop the ladder from cycling. Replace repeats with a distinct deep-dive mode. (highest value)

**Numbers:** cycle 2+ (s11 onward) is **1 drop in 285 transitions (0.4%)** for a total of 3 floor
points; past s15 it is **0 in 227**. Every modality is 0.0% there except forensics at 1.6% (1/61).
Meanwhile 45% of all grinder sessions go to eight functions whose floors have collectively moved
9 times in 344 sessions, six of them never.

**Recommendation:** after one full ladder cycle (s2–s10) with no floor movement, do not start cycle 2.
The `_exhaustion_ready` → `escalation` path already exists; this is a proposal to reach it sooner and
to make the post-cycle-1 state something other than "run the same nine rungs again." The owner's
no-deferral directive says a stuck item **changes modality, never target** — cycling the same ladder
is not a modality change, and the data says it is not one in effect either.

**Confidence: very high.** 285 transitions, one drop. This is the largest and cleanest effect in the
dataset and it survives every control I applied.

### R2 — Move synthesis from s10 to roughly s6, before forensics and rederive

**Numbers:** synthesis is the only modality with deep-plateau yield — 4/44 (9%) after 3+ flat sessions
and 3/41 (7%) after 5+, against all-modality baselines of 2% and 1% (and synthesis is *all three* of
the 5+ drops). Its drops are the biggest in the dataset (mean 7.0, median 7.5). In the depth-controlled
cycle-1 comparison it is second only to structural (20.0% vs 30.3%), well ahead of the two rungs that
currently precede it: forensics 6.5% and rederive 6.2%. Those two occupy **four consecutive rungs
(s6–s9)** and between them produced 8 drops worth 30 floor points in 125 transitions.

**Recommendation:** reorder cycle 1 to `structural, structural, permuter, permuter, synthesis,
forensics, forensics, rederive, rederive` — i.e. one synthesis pass at s6, before four rungs of
mechanism work. Synthesis re-reads the whole ledger and resets the frontier; doing that *before*
forensics/rederive spend four sessions gives those rungs a better-aimed frontier, and the ordering
costs nothing if synthesis whiffs.

**Confidence: medium.** The direction is well-supported and consistent across three independent cuts,
but n is small (46 synthesis transitions, 4 drops) and the s10-vs-s>10 split (3/18 vs 0/26) means the
effect is specifically "the *first* synthesis pass," which is exactly what this change reschedules —
so the mechanism and the intervention match. Watch for the possibility that synthesis works *because*
it runs late with a fat ledger to consolidate; if a s6 synthesis pass yields nothing over ~15 trials,
revert.

### R3 — Tighten the permuter gate to one session per function, unconditionally

**Numbers:** 3rd-or-later permuter session for a function: **0 drops in 64**. 2nd: 3/41 (7.3%). 1st:
8/50 (16.0%). By slot, s12+ permuter is 0/66. The current gate (skip repeats only after a zero-yield
first) would have skipped 97 sessions and lost 2 drops (4 floor points). A hard one-per-function cap
skips **105** sessions and loses **3 drops (10 points)** — 8 more sessions saved for 6 more floor
points, and the marginal session it removes is the 2nd permuter after a *successful* first, which is
the only case the current gate lets through.

**Recommendation:** cap at one permuter session per function. Preserve the s4 slot exactly as is — it
is where 8 of the 11 permuter drops and, per §3, most of the genuine permuter closes live. Do **not**
cut permuter overall: it closed 13 of 22 hard completions, 9 of them from a non-zero floor.

**Confidence: high on the 3rd+ cut** (0/64 is unambiguous), **medium on collapsing the 2nd slot** —
that trades 6 floor points across three functions for 8 sessions, which is a judgement call about
session cost, not a fact the data settles. If the 2nd slot is kept, the current gate is already the
right shape.

### R4 — Route the repeat-FAIL loop away from resubmission

**Numbers:** 34% of all judge FAILs (24 of 71) land on six functions with ≥3 FAILs each. August's fail
rate is 53% against July's 20%. The rulings state the mechanism explicitly — the same banned construct
resubmitted under successive family labels, in one case three times under three labels specifically to
dodge the driver's substring check.

**Recommendation:** on a function's **second** judge FAIL naming the same banned construct family,
force the next session to a different modality than the one that produced both candidates, and treat
"same mechanism, new family label" as a driver-side discard before the Judge sees it (extending
`check_banned_constructs`, which currently matches on declared-construct substrings — the rulings show
workers rewording around it deliberately). Each Judge round-trip is expensive and the current design
lets a worker spend several on one dead construct.

**Confidence: medium-high** on the concentration fact (exact count from parsed decisions); **medium**
on the intervention, since I have not measured how often a *modality* change after a repeat FAIL
actually produced a different construct.

### R5 — Instrument the closing session

**Numbers:** §3's entire finding — that permuter closes 59% of hard functions while ranking 4th on
floor movement — rests on a ladder *inference*, because the completing session writes no `floor=` line
and the per-function ledger is deleted on completion. 105 of 151 completions have no session lines at
all. The one discriminator I could apply (floor entering the final session) showed 4 of 13
permuter-slot closes were mere landings — so roughly a third of the inferred credit was wrong, and I
cannot rule out more.

**Recommendation:** have the driver emit a final `s<N> [modality] floor=0` journal line (or record the
closing modality on the `COMPLETED-C` line) before it tears the ledger down. One line per completion
makes every modality-effectiveness question above answerable directly instead of inferentially, and
this is the measurement that would let R2 and R3 be evaluated next month rather than argued.

**Confidence: high** that the gap exists and is load-bearing (verified: 44 of 46 covered completions
have max logged session == declared − 1); the recommendation is a measurement fix, not a behavioural
bet.

---

## 9. What I could not measure, and why

- **Whether a "permuter" session actually ran the permuter.** The modality is what the driver
  *mandated*; the journal text does not reliably say what the worker did. A permuter-slot close may
  have come from structural work done in the same session. This is the main uncertainty in §3.
- **The true closing modality** — see R5. Inferred from the ladder; `ladder_skip` and the 08-18 gate
  can shift the mapping, and completed-function ledgers (`memory/grind/<func>/`) are deleted, so no
  independent check exists. All 13 s4/s5 completions I tried to verify returned NO LEDGER.
- **Session cost.** No wall-clock or token cost per session is recorded anywhere I could reach
  (`agent_runs` exists in the DB but does not join to modality). Every "sessions saved" figure above
  is a session *count*, not a cost — and sessions are certainly not equal-cost, since permuter sessions
  block on campaigns (1615 `permuter-wait` events).
- **Modality data in the metrics DB.** There is none. `engine_events` has no modality or floor column;
  the only columns matching `%modal%|%floor%|%session%` are two `session_id`s. The journal is the sole
  source for everything in §§1–4, and it is a hand-appended prose file, so a malformed line is silently
  invisible (2 of 769 `floor=` lines did not parse; I did not chase them, they are <0.3%).
- **Permuter iteration counts.** `permuter-launch` events carry no `iters` in `extra`, so permuter
  *effort* per session is unmeasured — only launch/harvest/wait counts.
- **Whether floor is comparable across functions.** Floor is a raw instruction distance; a drop of 4 on
  a 50-instruction function and on a 400-instruction one are treated identically in every mean above.
- **July's FAIL classification** is approximate (I hand-read all 34 August FAILs but only skimmed
  July's 37 lead paragraphs). The July citation-class count of 0 is the one July figure I am confident
  in, because that failure mode requires the self-vet artifact, which did not exist until 2026-08-07.
- **Pre-grinder work.** The journal starts 2026-07-06. The 99 `Match:` commits without a session count
  and the May/June throughput (13/59 functions) are outside every modality table here.
