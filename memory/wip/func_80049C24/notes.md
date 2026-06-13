# func_80049C24 — RA-priority diff, closing form pending user policy

## TL;DR (2026-06-13)

Honest pure-C distance is 8 = 6 register-swap diffs ($s7↔$fp) + 2 nor-scheduling diffs at .L80049D14.

**RA diagnosis (instrumented cc1, BB2_ALLOC_DEBUG):**
HEAD's allocno priorities (formula `log2(refs)*refs/livelen * 10000`, global.c:611):
- pseudo 79 (var_fp): refs=3, livelen=73, pri=410 → allocated first → picks $s7 (HEAD's allocator order)
- pseudo 77 (var_s7): refs=3, livelen=91, pri=329 → allocated second → picks $fp

Target wants var_s7 in $s7 and var_fp in $fp — the opposite. To flip, var_s7's priority must exceed var_fp's.

**Closing form found (sandbox=0):** see candidate.c.
- Change #1 (`var_s7 = arg0; var_s7 += temp_v0;`) inflates pseudo 77's refs to 5, pri to 1075, flipping the order.
- Change #2 (`hdr = ~var_s0;` hoisted early, then `hdr = (u32)hdr >> 31;` later) gives sched1 freedom to place `nor` above the `move v1,s3 / addiu s3,s3,4` pair.

**Status:** cheat-reviewer NEEDS_USER on Change #1 (same-variable split-init accumulation; intermediate IS live, so not Lever D dead-store, but the explicit purpose is refs-count manipulation; sits adjacent to SOTN-sanctioned variable-reuse but doesn't exactly match cited evidence). Change #2 PASSes independently.

## Resume recipe

1. Read meta.json + this file.
2. **Wait on user policy verdict** for Change #1.
   - If user PASSes the same-variable split-init for refs manipulation: apply candidate.c to src/text1b.c lines 1126-1221, run `sandbox func_80049C24 --disable all` (expect 0), `retire func_80049C24` (drops 10 rules, full SHA1 gate), `queue done func_80049C24`, commit `Match: func_80049C24 ...`.
   - If user FAILs: try Lever A — shrink var_fp's livelen via fresh block-local at one of its uses (would need to push livelen > 91 to flip; currently 73). Permuter-directed search from baseline could help.
3. Either way, delete `memory/wip/func_80049C24/` once function reaches COMPLETED state.

## Levers tested this session (full table in meta.json `rejected_forms`)

| # | Lever | Score | Outcome |
|---|---|---|---|
| baseline | HEAD | 8 | n/a |
| v1 | swap decl order | 8 | no RA change |
| v2 | swap assignment order | 8 | no RA change |
| v3 | inline var_s7 at call sites | 56 | spilled — saves arg0/temp_v0 as long-life |
| v4 | inline var_fp at call sites | 63 | same problem, worse |
| v5 | block-scoped decls | 8 | block scope doesn't shrink RTL livelen |
| v6 | reuse temp_v0 for var_s7 | 17 | RA flips correctly but breaks scheduling |
| v7 | split var_s7 only | 2 | RA fixed; nor still wrong |
| **v8** | split var_s7 + split hdr | **0** | full close — current candidate.c |
| v9 | split var_s7 + var_fp + hdr | 0 | also closes; symmetric pointer-init pattern |
| v10 | split hdr only | 6 | confirms nor split alone is worth 2 |
| v11 | `register` keyword on var_s7 | 8 | no-op in this GCC |

## Why Change #1 is borderline

`var_s7 = arg0; var_s7 += temp_v0;` is a 2-step decomposition of `var_s7 = arg0 + temp_v0`. Final emitted code: 1 `addu` insn (combine folds the split). The split's ONLY purpose is the refs-count bump in RTL upstream of combine. Per cheats-by-spelling policy ([[no-new-park-categories]]), "constructs whose only purpose is to change GCC's analysis without that purpose appearing in the emitted output" are cheats by any spelling.

But: the intermediate `var_s7 == arg0` IS live (feeds the `+=`). NOT a dead store. NOT covered by Lever D's forbidden-pattern list. NOT in the SOTN-sanctioned catalog by name — closest sanctioned patterns are `randy = basePoint.x; baseX = randy;` (different-variable routing) and `idxSub = idxSub;` (no-op self-assign).

Reviewer's verdict NEEDS_USER. Per "NEEDS_USER is final" rule (review-discipline-before-commit.md), the question goes to the user — not re-adjudicated here.
