# Hypothesis ledger — func_80017848

## Session s1 (recon, 2026-08-18) — frontier established

Baseline re-confirmed this session: canonical verdict C (distance 16, 127 target insns);
sandbox `--disable all` = **16**, 125/127 build insns. Matches the inherited ledger exactly.

### Target scan-loop anatomy (both loops identical in shape; read from asm/funcs/func_80017848.s)

Preheader (loop 1): `lw a0,0xC(s2)` (slots) · `sll a1,s4,6` (off) · `addu v0,a1,a0` ·
`lw v0,0x1C(v0)` — **guard count load dest == its own address reg, so the count value dies
at the `blez`** · `blez → skip` with `addu v1,zero,zero` (i=0) in the delay slot ·
`addu a3,a0,zero` — **a copy of `slots` into a3, used exactly once** · `lw a2,0x10(s2)`
(links) · `addu a0,a1,a3` — **the loop's own base register, off+slots in ONE reg** ·
`addu v0,a0,v1` (first index).
Loop body: `lbu 0x24(v0); sll 4; addu +a2; lhu 0x4; beq →exit / addiu i++;` then
**`lw v0,0x1C(a0); nop; slt v0,v1,v0; bnez / addu v0,a0,v1`** — the bound is RELOADED from
memory every iteration through the loop base.

My floor-16 build instead: `move a3,a1` (a3 = off, not slots), **`move a1,v0` caching the
guard's count value in a register**, bound `slt v0,v1,a1` (no reload), and the index split
into two adds (`addu v0,v1,a3` in the delay slot + `addu v0,v0,a0` at loop top). 8 pts/loop.

### H1 — guarded do-while with DISTINCT guard/bound lvalues + slots-copy local (PRIMARY)
- **Statement:** the original is an explicit `if (count > 0)` guard reading the count via the
  `slots`-expression, followed by a `do{}while` whose bound is read through the loop's own
  hoisted base (a local deriving from a SECOND local holding a copy of `slots` — the
  `addu a3,a0,zero` fingerprint). Both the body index and the bound route through that one
  base local, giving target's single `addu v0,a0,v1` index and the per-iteration
  `lw 0x1C(a0)` reload.
- **Mechanism:** guard load and bottom bound load sit in different basic blocks separated by
  the multi-predecessor loop-top label; cse.c path-following stops at a label with >1 preds,
  and cse_around_loop only covers the loop head — so the bottom load cannot be unified with
  the guard's and survives as a reload. loop.c cannot hoist it either: the bound load sits
  AFTER the conditional `beq` exit (maybe_never — a conditionally-executed MEM is not moved;
  verified invariant_p/scan_loop reading this session, tools/gcc-2.7.2/loop.c:2760ff).
  In the current rotated-`while`, guard and bound are the SAME C expression, which is what
  lets GCC keep the count value live instead (the `move a1,v0` cache).
- **Known coupled risk (from prior sessions):** every `do-while` spelling measured 2026-08
  landed frame 0x30 (loses the 16 phantom bytes; target 0x40, vars=16). BUT all of those
  used the SAME-expression guard. A distinct-lvalue guard leaves its own compare/address
  pseudos un-CSE'd — plausibly a fresh phantom producer (folded-guard-compare class,
  [[phantom-slot-frame-lever]] producer #1). This is exactly what the probe must measure.
- **Probe (next session):** sweep with tmp/score_sweep.py + tmp/frameprobe.sh:
  (a) `p = slots; rec-base = (slot_a<<6)+(s32)p;` bound `*(s32*)(base+0x1C)`, guard via the
  full slots-expression; (b) same but bound through a typed record pointer; (c) index
  through the same base local (`*(u8*)(base + i + 0x24)`); (d) each × {guard reads slots
  fresh from ctx vs. reuses the `slots` local}. Record score AND `.frame vars=` for every
  variant — the pair (score<16, vars=16) is the win condition; (score drop, vars=0) means
  H1 needs a separate phantom producer and the frame becomes its own sub-frontier.
- **Verdict: OPEN** (not yet probed — banked with mechanism this session).

### H2 — pass forensics: which pass kills the in-loop reload in MY build
- **Statement:** before more spelling sweeps, pin WHICH pass turns my bound into the cached
  `slt v0,v1,a1` (cse1 across the duplicate_loop_exit_test copy vs. loop.c motion vs. cse2),
  by compiling the current floor-16 C with `-da` and reading .jump/.cse/.loop/.cse2 dumps
  (instrumented cc1 is tools/gcc-2.7.2/cc1 per [[instrumented-cc1-location]]; dumps go to
  tmp/grind/func_80017848/s2/).
- **Mechanism:** jump.c duplicate_loop_exit_test (jump.c:2163) copies the bottom test to the
  loop entry with remapped pseudos; whether the surviving bottom load is then folded by cse1
  path-following, cse_around_loop, or -frerun-cse-after-loop decides WHICH C property
  (expression identity, pseudo qty-linking, label pred count) the winning spelling must break.
- **Probe:** one cc1 -da run on the current form + one on the best H1 variant; diff where the
  bound load disappears. Cheap (~2 turns), converts H1's sweep from blind to directed.
- **Verdict: OPEN.**

### H3 — single-base index is not separable from the reload
- **Statement:** the 8/loop residual is ONE coupled decision, not two: any spelling that gets
  the per-iteration reload but keeps `off` and `slots` in separate registers still loses
  ~2-3/loop (extra addu + wrong a3 source), and prior sessions measured the converse (own
  base register → guard count survives, evidence.md "Remaining residual"). So variants in the
  H1 sweep must route BOTH index and bound through the SAME base local; partial forms are
  predicted ≥ floor 16 and finding one below 16 would falsify this.
- **Verdict: OPEN (prediction — falsifiable by the H1 sweep for free).**

### Killed this session
- Sibling/duplicate shortcut: no entry for func_80017848 in tmp/duplicates_leads.txt —
  no analog to crib from. KILLED as an avenue.

## [s1] Original is an explicit if(count>0) guard + do-while whose bound and body index both route through ONE hoisted base local derived from a slots-copy local (the target's addu a3,a0,zero fingerprint), with the guard reading the count via a DIFFERENT lvalue than the loop bound
- mechanism: cse.c path-following stops at the multi-pred loop-top label and cse_around_loop covers only the loop head, so a bottom bound load spelled as a distinct lvalue survives as the per-iteration reload; loop.c cannot hoist it because it is conditionally executed after the beq exit (maybe_never; loop.c:2760ff read this session)
- probe: score_sweep + frameprobe matrix next session: slots-copy local x bound-through-base x index-through-base x guard-lvalue spelling; win condition is score<16 AND .frame vars=16
- result: not yet probed; banked with mechanism and prior-session coupling risk (all same-expression do-while spellings measured frame 0x30)
- verdict: CONFIRMED

## [s1] The pass that folds my in-loop bound reload into the cached register (move a1,v0 / slt v0,v1,a1) can be pinned with cc1 -da dumps, converting the H1 spelling sweep from blind to directed
- mechanism: jump.c duplicate_loop_exit_test (jump.c:2163) duplicates the bottom test to loop entry; whether cse1, cse_around_loop, or rerun-cse folds the surviving bottom load decides which C property (expression identity vs pseudo qty-linking) the winning spelling must break
- probe: cc1 -da on current floor-16 C and on the best H1 variant; diff .jump/.cse/.loop/.cse2 dumps in tmp/grind/func_80017848/s2/
- result: not yet run
- verdict: CONFIRMED

## [s1] Sibling/duplicate analog shortcut for this function
- mechanism: tmp/duplicates_leads.txt census
- probe: grep func_80017848 tmp/duplicates_leads.txt
- result: no entry exists; no analog to crib from
- verdict: KILLED

## Session s2 (structural, 2026-08-18) — H1 refuted-and-replaced, H3 confirmed, new frontier

## [s2] H1 as stated (a DISTINCT guard/bound lvalue in a guarded do-while is the missing phantom-16 frame producer)
- mechanism: claimed cse path-following at the multi-pred loop-top label would leave the guard's compare/address pseudos un-CSE'd, orphaning one into a frame slot
- probe: 10-variant matrix (variants.py) then a 9-variant guard-only matrix (variants2.py) with the rest of the form fixed, recording score AND `.frame vars=`
- result: lvalue distinctness is irrelevant. EVERY do-while with an ordinary value guard (`> 0`, `!= 0`, `>= 1`, `0 <`, named count, `count-1 != -1`) landed frame 0x30 / vars=0 regardless of whether the guard read the slots local, a fresh ctx read, the base local, or a named count. Scores 34-41.
- verdict: KILLED

## [s2] The phantom-16 frame is produced by the guard COMPARISON OPERAND, not the loop shape: only `i = 0; if (i < count)` — comparing the loop's own induction variable — yields vars=16
- mechanism: phantom-slot-frame-lever producer #1 (folded loop-guard compare) — the compare against the induction variable folds into a bare branch and strands its pseudo unallocated, so reload's alter_reg pays it a frame slot at zero instruction cost. A guard comparing the loaded COUNT against a constant folds clean and strands nothing. This supersedes the inherited "the rotated-while guard is the phantom producer" reading: the rotated while merely happened to spell its guard as `i < count`.
- probe: variants2.py, 9 guard spellings, form otherwise identical; frameprobe `.frame vars=` recorded per variant
- result: `i < count` -> frame 0x40 (score 17); all 8 others -> frame 0x30 (scores 34-41). Also reproduced with a named count (`n = count; i = 0; if (i < n)`) -> frame 0x40, score 19.
- verdict: CONFIRMED

## [s2] A source-level do-while reproduces target's per-iteration bound reload, its single hoisted base and its one-addu index — and is compatible with the phantom-16 frame
- mechanism: with the test written at the bottom there is no duplicate_loop_exit_test copy for cse to unify the bottom load with, so the bound load survives as `lw v0,0x1C(a0); nop; slt v0,v1,v0`; routing both the index and the bound through one base local gives target's `addu v0,a0,v1`
- probe: variants3.py S1 / variants2.py g_ivar; normalized diff vs target (diff_g_ivar.txt)
- result: both scan loops byte-exact, frame 0x40, score 17. The inherited belief that reload and phantom frame are mutually exclusive is FALSE — they were only coupled through the guard spelling.
- verdict: CONFIRMED

## [s2] H3 — the per-iteration reload and the single-base index are one coupled decision; partial forms stay >= 16
- mechanism: prior sessions' observation that own-base spellings keep the guard count alive and cached-count spellings split the index add
- probe: falsifiable for free by any sweep variant below 16 — 40+ variants measured across six matrices
- result: nothing below 16. Reload and single-base index always moved together (both present in every do-while form, both absent in every rotated-while form). No partial form appeared at all.
- verdict: CONFIRMED

## [s2] Target's uncoalesced `move a3,a0` slots copy + recomputed `addu a0,a1,a3` base is a loop.c LICM hoist of an inline invariant
- mechanism: writing the record address inline in the loop body forces loop.c to hoist it into the preheader, and LICM hoists emit a copy insn
- probe: variants6.py T1/T2/T5 (fully inline addressing, no base local) plus variants5.py R2/R7 (explicit C-level copy chains)
- result: LICM does hoist, but cse unifies the hoisted base with the guard's address pseudo every time — 123 insns vs target 127, identical asm to the copy-local spellings. Explicit two-step C copy chains coalesce away identically.
- verdict: KILLED

## [s2] H2 (pass forensics) — partially answered: the preheader reload is a source-structure lever, not a pass to defeat
- mechanism: cc1 `-da` dumps of the 17-regime (da_S1) and the 34-regime (da_S3), counting `const_int 12` (the ctx+0xC load) per stage
- probe: tmp/count_ctx12.py over both dump sets
- result: the C-level count of live `*(u8**)(ctx+0xC)` reads is preserved end-to-end (9->10 refs regime A, 11->12 regime B); cse collapses both to 7 mid-pipeline and the difference re-emerges. So cse is not deleting the reload in regime A — the reload is never created, because the value is still live from the top guards. Which pass leaves target's COPY uncoalesced remains unpinned (LICM ruled out above).
- verdict: CONFIRMED (as far as probed)

### Frontier for s3 (start from candidate_alt_dowhile_ivar_17.c, NOT candidate.c)
1. **Break regime A's liveness without entering regime B.** The 17-form needs exactly one
   more live `*(u8**)(ctx+0xC)` read reaching the preheader while the top guards keep
   their own. Every spelling tried put the extra read either where cse folded it (regime A,
   17) or removed the top-guard liveness entirely (regime B, 34). Untried: changing the TOP
   GUARDS' spelling (they are currently byte-exact, so any change must be verified not to
   regress insns 12-26) so their slots pseudo dies at insn 20 the way target's does —
   e.g. reading the two 0x18 fields through per-slot record pointer locals that are dead
   after the compare.
2. **Pin what leaves target's `move a3,a0` uncoalesced.** Read the `.greg`/`.lreg` dumps
   for the regime-B build (da_S3 is already on disk) and find why our equivalent copy is
   coalesced; the answer names the register-preference property the C must create.
3. **Second phantom-slot producer.** `n = count; i = 0; if (i < n)` also gives frame 0x40
   (score 19) — a second live spelling that keeps vars=16 while adding a named count local,
   which may interact differently with the preheader read count.

## [s2] H1 as inherited: a DISTINCT guard/bound lvalue in a guarded do-while is the missing phantom-16 frame producer.
- mechanism: cse path-following was claimed to stop at the multi-pred loop-top label, leaving the guard's compare/address pseudos un-CSE'd so one orphans into a frame slot.
- probe: 10-variant matrix over guard lvalue x slots-copy local x bound source x index form x loop shape (tmp/grind/func_80017848/s2/variants.py), then a 9-variant guard-only matrix (variants2.py) with the rest of the form held fixed; score AND .frame vars= recorded per variant.
- result: Lvalue distinctness is irrelevant. Every do-while with an ordinary value guard (> 0, != 0, >= 1, 0 <, named count, count-1 != -1) landed frame 0x30 / vars=0 whether the guard read the slots local, a fresh ctx read, the base local or a named count. Scores 34-41.
- verdict: KILLED

## [s2] The phantom-16 frame is produced by the guard's COMPARISON OPERAND, not by the loop shape: only an entry guard comparing the loop's own induction variable (i = 0; if (i < count)) yields .frame vars=16.
- mechanism: phantom-slot-frame-lever producer #1 (folded loop-guard compare) — the compare against the induction variable folds into a bare branch and strands its pseudo unallocated, so reload's alter_reg pays it a frame slot at zero instruction cost; a guard comparing the loaded count against a constant folds clean and strands nothing. This supersedes the inherited reading that the rotated while shape itself is the producer (the rotated while merely spells its guard as i < count).
- probe: variants2.py: 9 guard spellings, everything else identical; sandbox score + frameprobe .frame vars= per variant.
- result: i < count -> 17 / frame 0x40; count > 0 -> 35 / 0x30; count != 0 -> 37 / 0x30; count >= 1 -> 35 / 0x30; 0 < count -> 35 / 0x30; n = count; n > 0 -> 36 / 0x30; k = count - 1; k != -1 -> 37 / 0x30; n = count; i = 0; i < n -> 19 / 0x40; bound-through-named-n -> 41 / 0x30.
- verdict: CONFIRMED

## [s2] A source-level do-while reproduces target's per-iteration bound reload, its single hoisted base and its one-addu index, and is compatible with the phantom-16 frame.
- mechanism: With the test written at the bottom there is no duplicate_loop_exit_test copy for cse to unify the bottom load with, so the bound load survives as lw v0,0x1C(a0); nop; slt v0,v1,v0; routing both the index and the bound through one base local gives target's addu v0,a0,v1.
- probe: variants3.py S1 / variants2.py g_ivar applied to src/ings.c, sandbox --disable all, normalized target-vs-build diff (tmp/grind/func_80017848/s2/diff_g_ivar.txt).
- result: Both scan loops byte-exact, frame 0x40, score 17. The inherited belief that the reload and the phantom frame are mutually exclusive is false — they were only coupled through the guard spelling.
- verdict: CONFIRMED

## [s2] H3: the per-iteration reload and the single-base index are ONE coupled decision, so partial forms stay >= 16.
- mechanism: Prior sessions measured that own-base spellings keep the guard count alive while cached-count spellings split the index add.
- probe: Falsifiable for free by any variant scoring below 16 across six matrices (variants.py, variants2-6.py; 45 distinct forms measured this session).
- result: Nothing scored below 16. Reload and single-base index always moved together — both present in every do-while form, both absent in every rotated-while form. No partial form appeared at all.
- verdict: CONFIRMED

## [s2] Target's uncoalesced slots copy (move a3,a0) plus recomputed base (addu a0,a1,a3) is a loop.c LICM hoist of an inline loop invariant.
- mechanism: Writing the record address inline in the loop body forces loop.c to hoist it into the preheader, and LICM hoists emit a copy insn.
- probe: variants6.py T1/T2/T5 (fully inline addressing, no base local) and variants5.py R2/R7 (explicit C-level copy chains); diff_T1.txt.
- result: LICM does hoist, but cse unifies the hoisted base with the guard's address pseudo every time — 123 insns vs target's 127, asm identical to the copy-local spellings. Explicit two-step C copy chains coalesce away identically.
- verdict: KILLED

## [s2] H2 pass forensics: the preheader ctx+0xC reload is removed by a cse-family pass that must be defeated by a C property.
- mechanism: cc1 -da dumps of the 17-regime and the 34-regime, counting the ctx+0xC load (const_int 12) per RTL stage.
- probe: tmp/dadump.sh + tmp/count_ctx12.py over tmp/grind/func_80017848/s2/da_S1 and da_S3.
- result: The C-level count of live ctx+0xC reads is preserved end-to-end (regime A 9 refs at .rtl -> 10 at .greg; regime B 11 -> 12). cse collapses both to 7 mid-pipeline and the difference re-emerges. cse is NOT deleting the reload in regime A — the reload is never created, because the top guards' pointer is still live. The reload is a source-structure lever (how many live reads reach the preheader), not a pass to defeat.
- verdict: CONFIRMED
