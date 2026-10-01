# func_800759D0 — hypotheses / levers (2026-09-25)

## Status
Session 4 (2026-09-25, manual lane): the owner answered the latent-bug question as Ruling 9
amendment (b′) (bcdc1648e). The shared-`cells` body is re-submitted with a rewritten
declaration comment; the (b′)(1)-(3) record is evidence.md "Ruling 9 (b′)" (the placeholder
path is the only reachable write outside the first-cell pattern, and it lands outside the
record). Layer-2 FAILED it as UNDECIDED on (b′)(3): 0xCA8 is past sheet [21] but lands on
byte 8 of an unreferenced record at 0xCA0, and the text does not say whether "inside a record"
means BASE's record or any record. Owner question in borderline.md. The body (0/364, SHA1 ==
oracle) was banked as pending-bprime-0.c. Session 5: the owner ruled "past the end"
(2fc07a100). The unreferenced-address search is recorded in evidence.md; 0xCA8 is unreferenced.
The body is re-submitted as candidate.c. The only change is that the comment now says "into bytes
nothing references". pending-bprime-0.c is removed as superseded; the per-site 25 receipt stays
at rejected/per-site-locals-25.c.
Session 3 (2026-09-25): the shared `cells` body landed under owner Ruling 9 (23045f51f) and was
then FAILED and reverted: Ruling 9 (b)/(f) fail on the reachable 0x14-placeholder path, where
loop 2 reads `table[21] + 0x24` past the end of a 1-header sheet (evidence.md "Ruling 9 FAIL";
rejected/ruling9-cells-placeholder-overrun-0.c). candidate.c is the honest per-site 25 form.
Frontier: the borderline.md 2026-09-25 policy question (judge (b) by the layout the code
assumes?). Otherwise, find a per-site spelling that seats the head and loop-1 sums in $a1
(Session 2 argues none exists).
The rest of this section is the pre-ruling state.
Honest floor **25/364** (per-site `q0..q3`, one local per write).
The 0/364 form (one function-scope `q` written at 4 sites) was FAILED by layer-2 on
2026-09-25: rejected/function-scope-q-multiwrite-0.c (reasoning in its header). The owner
question is logged in docs/grind/borderline.md 2026-09-25 "one role, differing constant
offsets".

## What the 25 is (read from RTL dumps, tmp/f759d0/rtl.py)
- Head (3): target `addiu a1,v1,12` for `table[0] + 0xC`. Per-site, the sum is a
  single-block pseudo; local-alloc ties it to the dying table[0] pseudo -> `addiu v1,v1,12`.
  Moving the sp1C store past the sp2C if/else (so the sum spans blocks) = 29 (the store
  then leaves the beqz delay slot). The store must precede the branch.
- Loop 1 (22): target seats q in $a1, which pushes the table value to $a2, arg1*4 to $a3
  and the cell address to $t0. Per-site, q1 is local-alloc'd to $v0 (it sits inside the
  D_800A36A0-load pseudo's life and wins on priority), so everything shifts down one
  register. Worked through local-alloc: with q1 single-block, $v1 is free during its life,
  so local-alloc can never give it $a1; the target's q must be a pseudo referenced in
  several blocks (global.c). In loop 1 the `s.sp1C = q` store has a true dependence on the
  following `lh` reads through `state` (unknown pointer vs the address-taken `s`), so it
  cannot be moved past the grid branch to make q span blocks.
- Conclusion so far: every multi-block reference to loop 1's / the head's q found is a
  reuse of one variable across sites, which the current rules refuse. Loop-2 and loop-3 q
  are naturally multi-block (computed before the highlight if, stored after).

## Confirmed / refuted
- CONFIRMED: `s32 zero` constant-holder needed (literal 0 = 30, 362 insns). Layer-2 judged
  it OK.
- CONFIRMED: `(D_8009BCF8 + i)->unk0` (layer-2 OK). `D_8009BCF8[i].unk0` = 41.
- CONFIRMED: grid read `((u8 *)D_8009BCF8)[index]` with `index` local (layer-2 OK).
- REFUTED: rec pointer held across the call (123); struct-index grid (67).
- REFUTED (rule): function-scope q reuse (0) and any partial reuse (loops only = 3,
  still a multi-write carrier; extension (B) fails because the record is picked with
  `table[entry + 1]` / `table[arg2[i] + 1]`, not a constant or loop-counter subscript).
- REFUTED: head store after the if/else (29).

## Permuter (2026-09-25, carrier-free chassis = candidate.c, 8 jobs, 3,883 iterations,
## stopped early for host memory pressure)
Base 335 (permuter-weighted). Finds 90/140/180/238/265/285: EVERY one makes the head's or
loop 1's sum a multi-block pseudo by reusing a q local across blocks (best 90:
`q0 = (q1 = s.sp18 + 0xC);` making q1 a head+loop-1 carrier; others reuse q0 for the
arg1 byte term, or store q0 before assigning it). All banned (multi-write carrier /
staged-value borrow); they confirm the data flow the target needs, nothing more.
Workspace: tools/decomp-permuter/nonmatchings/func_800759D0 (built by tmp/f759d0/mkperm.sh).

## Session 2 (manual lane, 2026-09-25) — floor still 25; why no per-site form can close
Allocation-dump finding (tmp/f759d0/rtl.py on rejected/function-scope-q-multiwrite-0.c, greg
dump): the shared q is pseudo 80, allocated to $a1, and its ONLY hard-register conflicts are
{v0, v1, a0, sp}. In the target's loop-1 then-block, over q's def->store range
(`addiu a1,a2,0x24` .. `sw a1,0x1C(sp)`), $v1 and $a0 hold no live value (entry dies at the
`sll v0,v1,2` in the beqz delay slot; `lh a0` comes after the store). So even a per-site q made
multi-block by some other means would conflict only with $v0 there, and global.c's find_reg pass 0
(registers already used, in register order) would give it $v1, not $a1. The $v1/$a0 exclusions come
from the OTHER sites: loop 2 keeps the table value in $v1 and arg3*2 in $a0 live across its q.
Likewise at the head a lone q gets $v0/$v1, never $a1. Conclusion: $a1 at the head and in loop 1
is the union of conflicts of ONE pseudo shared across the sites. The target was compiled from a
shared variable (as in the same author's func_800753D8 `body` and func_8007636C `q` on main).
No per-site spelling, and no construct that only makes one site multi-block, can reach it.
This is provenance evidence for the borderline.md "one role, differing constant offsets" question.

Measured this session (all from candidate.c, `sandbox --disable all`):
- head: `q0 = table[0] + 0xC; s.sp1C = q0; s.sp18 = table[0];` (keep table[0] live past q) — 37 (365 insns)
- head: q0 computed before the sp18/sp30/sp34 stores, stored after — 25 (no change)
- loop 1: `q1 = table[entry + 1] + 0x24;` before the sp18 store — 25 (no change)
- cc1psx self-disproof (`engine cc1psx-check`): per-site candidate psx 90 vs ours 25; shared-q
  form psx 63 vs ours 0. Not closer: SOURCE-SIDE.
- Ruled out by analysis: Ruling 6 (the four sites are sequential, not mutually exclusive regions);
  Ruling 5 extension (B) (loop 1 and loop 2 pick the record with `table[entry + 1]` /
  `table[arg2[i] + 1]`, and loop 3 reassigns `table` in its body); a shared record pointer
  (`hdr`) instead of q (the table value sits in $v1 at head/loops 2-3 but $a2 in loop 1, so one
  shared pseudo cannot match); a typed sheet struct (`SprtHdrA hdr[3]` + cells at +0x24, the
  head sheet has one header, cells at +0xC) is a good semantic spelling but creates no
  shared pseudo, so it cannot move the allocation.

## Frontier
- Session 2 showed that making the head/loop-1 sum multi-block is not enough anyway: the
  $a1 seat needs the conflict union of one pseudo shared across the sites (see Session 2).
  Ruling 6's regions do not qualify (the sites are sequential). The function is blocked on the
  owner's answer to the borderline.md question, not on an unfound spelling of the per-site form.
- Only remaining non-policy directions: a legal shared variable (one that meets Ruling 5 or
  its extension). None found: the head's +0xC and the loops' +0x24 are different templates,
  and the loops' record picks fail extension (B).
- If the owner allows the one-role reuse (borderline.md), the 0 form lands as-is
  (re-review with the ruling cited; rename q to a role name for R5 1(f)).

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: INCLUDE_ASM (reopened da429a8c2, then 803d0fea1 for the byte pun). `rejected/joint-l2-r1.c` scored 0 with a layer-2 PASS (c6fea6b4656345dd) but never landed because partner func_80075F80 failed. It still uses raw `D_800A36A0 + 0x34 / +arg3*2+0x3C / +0x65` and a `state = (s16 *)(arg3*2 + (s32)D_800A36A0)` pointer, so after SelWork (4fd37366e) both body and PASS are stale. Honest floor without the shared `cells`: 25/364.
- CONSTRAINTS: per-use byte pun `((u8 *)D_8009BCF8)[idx]` refused (Q37/Q38); flat `[20]` misses in every spelling (3/11/30/67). `cells` stands on Ruling 9 (b') + the past-the-end clarification (2fc07a100); paperwork in evidence.md. `zero` is a FAKE constant holder (named-local-fake-exception). SOTN cite `src/dra/62DEC.c:1091` is tagged @aa53500 but `.claude/rules/sotn-precedent-suffices.md:13-22` names the local clone @db41b28 - re-verify and cite db41b28.
- BLOCKER: data model only - `D_8009BCF8` must be `Unk8009BCF8Record D_8009BCF8[2][10]` (target scales page and cell separately). The body itself is solved.
- PLAN (one landing with func_80075F80):
  1. `include/game.h:130`: `D_8009BCF8[20]` -> `[2][10]`; fix the comment above it.
  2. Respell completed func_80076D74 to `D_8009BCF8[0][slot].unk1` (`src/text1b_tu2.c:823`; its PASS 1429aa83406b5900 is stale anyway); trim its stale comment :756-777.
  3. Port joint-l2-r1 to SelWork members `f34`, `f1C[arg3]`, `f20[arg3]`, `f3C[...]` (`.half[...]` once the f3C union lands), `f65`; drop the `state` pointer (the one unmeasured risk).
  4. `sandbox --diff`; fresh layer-2 jointly with func_80075F80.
- DEPENDS: shares the D_8009BCF8 decl and the f3C union with func_80075F80. The Q65 series (steps 10-11) merges text1b_tu2.c into text1b_b.c (Q67): landing before Q65 means regenerating the series, after means re-measuring in text1b_b.c - orchestrator's call.
- ODDS/LANE: 1-2 sessions for the joint landing, ~60% [I]. Manual (headers, SOTN cite, multi-function commit).
