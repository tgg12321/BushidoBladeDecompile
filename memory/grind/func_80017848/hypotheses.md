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

## Session s3 (structural, 2026-08-18) — floor 16 -> 14; s2 frontier item 1 KILLED, new axis found

## [s3] s2 frontier #1: re-spelling the two >=0 TOP GUARDS so their slots pseudo dies at the compare will force the loop preheader to emit target's own `lw a0,0xC(s2)` reload.
- mechanism: s2 argued the preheader reload exists only when the top guards' slots pseudo is dead at the merge (target: dead at insn 20); since the live-read count is a source-structure property, the top guards' spelling should be the lever.
- probe: tmp/grind/func_80017848/s3/variants.py — 8 top-guard spellings with the 17-form loop held byte-fixed: inline reads (control), per-slot record-pointer locals dead after the compare, a named `slots` local feeding both guards, `&&`-flattened single if, explicit `goto scan` inversion (return-0 on the fall-through, exactly target's block order), pointer-arithmetic address form, guard value read into a local. Later re-crossed at the 14-form (variants8.py: top-guard association x loop-guard association x 3 base forms).
- result: SEVEN of eight scored 17 (identical to the control); the pointer-arithmetic form scored 19. The top guards are inert — the CFG restructure that puts return-0 on the fall-through changes nothing, and at the 14-form the top guards' association order is inert too. The preheader reload is not controlled from there.
- verdict: KILLED

## [s3] The loop ENTRY GUARD's count-address association order is a distinct regime switch, independent of whether a `u8 *` local is live across the guard.
- mechanism: cse canonicalises `(plus off ptr)` from the source's association order; writing the guard's count address pointer-first makes the address a temporary whose register the count load's destination can reuse, which is exactly target's `addu v0,a1,a0; lw v0,0x1C(v0)` (dest == address reg). Shift-first leaves the address pseudo live as the loop base and the reload never appears.
- probe: variants3.py (Q0-Q7, 8 loop shapes x 2 top-guard spellings) isolated Q4 = the only spelling differing from Q0 by association order; then variants4.py (8 base forms, all 16) and variants8.py (2x2x3) confirmed association is the only moving part.
- result: pointer-first `*(s32 *)((s32)slots + (slot_a << 6) + CNT)` = 14; shift-first `*(s32 *)((slot_a << 6) + (s32)slots + CNT)` = 16; inline read with no local = 34. s2's "two rigid regimes at 17 and 34" was an artifact of every s2 spelling being shift-first. The inherited "index expression must be i-first" rule governs the BODY index only and must not be generalised to the guard's count address.
- verdict: CONFIRMED

## [s3] Hoisting the single `slots` read above the two >=0 top guards (so those guards consume it) is worth 1-3 further points.
- mechanism: it removes one live read from the loop region while keeping the guards' value available, which is what lets the loop guard's pointer-first address be a dead temporary; every extra read reaching the preheader is either folded by cse or coalesced by local-alloc and costs points.
- probe: variants5.py (T0-T6: where the hoisted local is read and who consumes it), variants6.py (U1-U8: re-read per loop vs once, two hoisted locals, hoist above the top guards), variants7.py (V0-V7: which read feeds the loop guard vs the loop base), variants9.py (Y0-Y6: re-assignment placement, bound routed through the pointer local).
- result: hoist above the top guards = 14; hoist just before loop 1 = 15; no hoist (loop guards read inline) = 16; re-read before each loop = 17; two hoisted locals = 15; bound read through the pointer local instead of the base local = 18/19.
- verdict: CONFIRMED

## [s3] Target's uncoalesced `move a3,a0` slots copy can be produced by giving the C a second/third textual read of *(u8**)(ctx+0xC) in the preheader region.
- mechanism: cse replaces a redundant load with a reg-reg copy rather than deleting it; a second source-level read should therefore materialise the copy, and s2's forensics showed the C-level read count survives to the final asm.
- probe: ~20 spellings across variants2.py (P1/P2/P8), variants3.py (Q1/Q3), variants4.py (R1/R5/R7 — two and three fresh reads, explicit copy chains), variants7.py (V4/V5 — a per-loop read feeding the guard and a second feeding the base), variants9.py (Y1-Y4 — re-assignment of the same local before each loop).
- result: no spelling produced the copy. Every added read is folded by cse or coalesced by local-alloc; the ones that survive as insns cost 1-3 points (17-19) instead of the 2 the copy would buy. Base spelling is entirely inert at 14 (int-cast / pointer-first / pointer arithmetic / copy chain / fully inline-LICM all identical).
- verdict: KILLED

### Frontier for s4 (start from candidate.c, which is now the 14-form)
1. **The copy is a register-ALLOCATION outcome, not a source-structure one.** Both remaining
   insns (`lw a0,0xC(s2)` for the guard address + `move a3,a0` for the base) live in one
   preheader; ~20 read-count spellings failed to create either. The untried instrument is
   the allocator dumps: cc1 `-da` on the 14-form and read `.lreg`/`.greg` to see which
   pseudo pair is being coalesced and what preference/conflict would stop it. s2 left this
   probe unspent (it was frontier item 2 there, still unspent now) and it is the only lever
   that names the property directly rather than guessing spellings.
2. **Loop-1's skip edge is the suspected conflict source.** In target the blez at insn 36
   jumps to .L8001791C, which is loop-2's guard *after* loop-1's exit reload — so the
   loop-1 preheader's a0/a1 are live-out along the skip edge and conflict with the base
   pseudo, which is a mechanism that would explain a non-coalescable copy. Our build lets
   both paths share one reload. Probe: a C shape where loop 2's guard is reachable from
   loop 1's skip WITHOUT re-reading (e.g. loop 2's count read hoisted next to loop 1's, or
   the two loops fused under one guard chain), then check whether `move a3,a0` appears.
3. **Permuter has never been run on this function.** The residual is now 2 insns in a
   126-insn function with everything else byte-exact — the regime where decomp-permuter is
   most likely to find the spelling a human sweep cannot. s1-s3 were all hand sweeps; a
   fresh-seed campaign (tools/permuter_campaign.py, per [[permuter-fresh-seed-discipline]])
   seeded with candidate.c is the cheapest untried modality.

## [s3] s2 frontier #1: re-spelling the two >=0 TOP GUARDS so their slots pseudo dies at the compare will force the loop preheader to emit target's own lw a0,0xC(s2) reload.
- mechanism: s2 argued the preheader reload exists only when the top guards' slots pseudo is dead at the merge (target: dead at insn 20), and that since the live-read count is a source-structure property the top guards' spelling is the lever.
- probe: tmp/grind/func_80017848/s3/variants.py: 8 top-guard spellings with the 17-form loop held byte-fixed (inline reads control, per-slot record-pointer locals dead after the compare, a named slots local, &&-flattened single if, explicit `goto scan` inversion putting return-0 on the fall-through exactly as target does, pointer-arithmetic address form, guard value read into a local, goto+record-pointers). Re-crossed at the 14-form in variants8.py (top-guard association x loop-guard association x 3 base forms).
- result: Seven of eight scored 17, identical to the control; the pointer-arithmetic form scored 19. The CFG restructure that puts return-0 on the fall-through changed nothing. At the 14-form the top guards' association order is inert as well. The preheader reload is not controlled from the top guards.
- verdict: KILLED

## [s3] The loop ENTRY GUARD's count-address association order is a distinct regime switch, independent of whether a u8 * local is live across the guard (s2 recorded only two regimes, 17 and 34).
- mechanism: cse canonicalises (plus off ptr) from the source's association order; written pointer-first the guard's count address becomes a temporary whose register the count load's destination reuses (target: addu v0,a1,a0; lw v0,0x1C(v0), dest == address reg), so the loop base must be recomputed. Written shift-first the address pseudo stays live as the loop base and no reload is ever emitted.
- probe: variants3.py (Q0-Q7: 8 loop shapes x 2 top-guard spellings) isolated Q4, which differs from Q0 only by association order; variants4.py (8 base forms) and variants8.py (2 x 2 x 3) then held association fixed and varied everything else.
- result: pointer-first `*(s32 *)((s32)slots + (slot_a << 6) + CNT)` = 14; the identical value shift-first `(slot_a << 6) + (s32)slots + CNT` = 16; inline read with no local = 34. s2's 'two rigid regimes at 17/34' was an artifact of every s2 spelling being shift-first. The inherited 'index expression must be i-first' rule governs the BODY index only.
- verdict: CONFIRMED

## [s3] Where the single `slots = *(u8**)(ctx+0xC)` read is hoisted is worth a further 1-3 points; hoisting it ABOVE the two >=0 top guards so those guards consume it is optimal.
- mechanism: It removes one live read from the loop region while keeping the guards' value available, which is what allows the loop guard's pointer-first count address to be a dead temporary; every additional read reaching the preheader is folded by cse or coalesced by local-alloc and costs points instead of buying target's copy.
- probe: variants5.py (T0-T6: where the hoisted local lives and who consumes it), variants6.py (U1-U8: once vs per-loop re-read, two hoisted locals, hoist above the top guards), variants7.py (V0-V7: which read feeds the loop guard vs the loop base), variants9.py (Y0-Y6: re-assignment placement, bound routed through the pointer local).
- result: hoist above the top guards = 14; hoist just before loop 1 = 15; no hoist (loop guards read inline) = 16; re-read before each loop = 17; two hoisted locals = 15; bound read through the pointer local rather than the base local = 18/19.
- verdict: CONFIRMED

## [s3] Target's uncoalesced `move a3,a0` slots copy can be produced by giving the C a second or third textual read of *(u8**)(ctx+0xC) in the preheader region.
- mechanism: cse replaces a redundant load with a reg-reg copy rather than deleting it, and s2's -da forensics showed the C-level read count survives to the final asm, so a second source-level read should materialise the copy.
- probe: ~20 spellings: variants2.py P1/P2/P8 (second read before the guard, inside the guard body, three reads), variants3.py Q1/Q3 (fully-inline LICM base, two-step copy chain), variants4.py R1/R5/R7 (two and three fresh reads, explicit copy chains), variants7.py V4/V5 (a per-loop read feeding the guard plus a second feeding the base), variants9.py Y1-Y4 (re-assigning the same local before each loop).
- result: No spelling produced the copy. Every added read is folded by cse or coalesced by local-alloc; the ones that survive as real insns cost 1-3 points (17-19) rather than the 2 the copy would buy. Base spelling is entirely inert at the 14-form: int-cast, pointer-first, pointer-arithmetic, copy chain and fully-inline (loop.c LICM) all score 14.
- verdict: KILLED

## s4 (2026-08-18) — permuter modality

### H-s4-1 — KILLED (and it kills a whole way of running this function)
**Statement:** "The remaining 2-insn gap is in the regime where decomp-permuter outperforms
hand sweeps, and permuter has never been run on this function" (the s3 frontier item).
Permuter has now been run, properly, on a workspace that reproduces the real build exactly.
**Probe:** built + validated a per-function workspace (`s4/mkws.sh`, self-checks at
125 vs 127 insns), launched a telemetered campaign from the s3 candidate
(`permuter_campaign.py launch --dir tmp/perm_ings_s4 -j 10`), waited in-turn, harvested
~25 saved outputs, and re-scored EVERY one with `sandbox --disable all`.
**Result:** best engine distance across all permuter output = **14** = exactly the starting
floor. The permuter's own best find (score 405 vs base 855) is engine **18**.
**Verdict:** KILLED as a *directed* lever. The premise "with 125/127 byte-exact the search
space a permuter explores is now tiny" was wrong in an instructive way: the space is tiny,
but the permuter's compass points out of it (see H-s4-2). A permuter campaign on this
function is a proposal generator whose ranking must be discarded and replaced with the
engine's, and at that point it is strictly weaker than the exhaustive directed sweep in
H-s4-3, which covers the same axes deterministically. Do not spend another session's
modality on an undirected campaign here.

### H-s4-2 — CONFIRMED (the session's most transferable finding)
**Statement:** The permuter's default scorer is anti-correlated with the engine's honest
distance for a function whose residual is a small instruction DELETION near the top of the body.
**Mechanism:** the permuter weights reorderings at 60 and ins/del at 100 and aligns the two
instruction streams globally. Two missing instructions early in the body displace the whole
tail, so the aligner charges ~14 reorderings (855 ≈ 14x60) even though 125 of 127 instructions
are byte-identical. The gradient therefore rewards mutations that re-sync the aligner by
CHANGING the already-correct tail, and punishes staying in the correct basin. The engine's
metric counts differing instructions with branch/jump targets masked and has no displacement
term, so the two metrics disagree by construction, not by accident.
**Probe:** the 20-row permuter-score vs engine-distance table in evidence.md E-s4-2, produced
by `batch_score.ps1` over every saved output of campaign A.
**Verdict:** CONFIRMED. Note the practical corollary: `permuter_campaign.py launch` does not
expose `--keep-prob`, so campaigns always run decomp-permuter's default 0.6 hill-climb, which
is precisely the behaviour that walks away from the basin here. A future session that wants a
usable campaign on a deletion-residual function needs either restart-from-base sampling or
engine-metric rescoring in the loop; the second is what s4 did by hand.

### H-s4-3 — KILLED (exhaustively, by measurement, not by sampling)
**Statement:** Some combination of the loop-preheader spelling axes that s1-s3 only swept
one-at-a-time reaches below 14.
**Mechanism:** the last three levers found on this function (do-while shape, guard comparison
operand, guard association order) were each invisible until someone wrote that exact spelling,
so combinations plausibly hid another.
**Probe:** generated the full symmetric cross-product of the five preheader axes
(guard-count base x preheader-p x base-spelling x body-index base x do-while-bound base =
72 variants, `s4/gen_variants.py`) and engine-scored all 72 in one pass (`s4/sweep.sh`,
raw results in `s4/sweep_results.txt`).
**Result:** min 14, max 19. Twelve cells tie at 14; zero cells go below.
**Verdict:** KILLED. The preheader spelling family is now closed by exhaustion rather than by
sampling. New sub-finding worth carrying: cells `a1_b0_*_{d0,d2}_e1` reach 14 with a FRESH
guard read, so s3's pointer-first/hoisted-guard rule is one sufficient route to 14 and not a
necessary condition — 14 is a plateau over the family, which is the signature of a constraint
living below the C source (register allocation), exactly as the s3 frontier predicted.

### H-s4-4 — CONFIRMED, and it is the floor drop (14 -> 12)
**Statement:** Randomization CONFINED to the residual region, with the permuter used purely as
a proposal generator and the ENGINE used as the objective, can find preheader structure that
neither the hand sweeps (s1-s3, ~105 forms) nor the full spelling cross-product (H-s4-3, 72
cells) could reach.
**Mechanism:** the winning form leaves the spelling family entirely — it introduces a SECOND
live pointer variable (`base = slots;`) that carries the hoisted pointer into the two scan-loop
entry guards, and is then re-pointed at the per-slot record inside each preheader. Every cell
of the H-s4-3 cross-product routed the guards through one of the two pointers that already
existed, so no amount of respelling inside that family could express this. It is the frozen-list
"variable reuse for codegen control" family; `base` is read by both loop guards before it is
reassigned, so it is a live variable, not a dead store or a holder.
**Probe:** chassis B = the validated workspace with the residual region wrapped in
`PERM_RANDOMIZE` (`s4/mk_perm_b.py`); 733 s / 21911 iterations / 130 saved outputs; every
output re-scored with `sandbox --disable all` (`s4/score_outputs.sh`); then a 4-cell cleanup
sweep to find the most readable spelling of the winner.
**Result:** two outputs at engine **12**, both at permuter score 845 — the permuter's own
WORST decile, ten points off base. Cleanup sweep: top guards through `base` = 12, through
`slots` = 12, mixed (as emitted) = 12, loop guards back onto `slots` = **19**. Chosen form
(both top guards through `slots`) re-verified in `src/ings.c` at **12**, 125/127 insns.
**Verdict:** CONFIRMED. Two compounding lessons for the pipeline: (i) scope the randomization
to the residual region so the randomizer cannot damage what is already exact — chassis B found
in 12 minutes what chassis A could not find at all; (ii) the engine metric must be the
objective, applied as a rescoring pass over every saved output. Trusting the permuter's own
ranking would have discarded both 12-cells as near-worst results.

## [s4] The remaining 2-insn gap is in the regime where decomp-permuter outperforms hand sweeps, and an undirected campaign from the s3 candidate will close or narrow it (the s3 frontier item).
- mechanism: s1-s3 were hand-authored spelling matrices (~105 forms) and each of the last three levers was invisible until someone wrote that exact spelling; with 125/127 insns byte-exact the space a permuter explores should be tiny.
- probe: Built and self-validated a per-function permuter workspace mirroring the real build exactly (tools/grind/func_80017848/s4/mkws.sh; prints 'base insns: 125  target: 127'), launched a telemetered campaign from the s3 candidate (permuter_campaign.py launch --dir tmp/perm_ings_s4 -j 10), waited in-turn, harvested ~25 saved outputs, and re-scored EVERY one with sandbox --disable all.
- result: Best engine distance across all campaign-A output = 14, exactly the starting floor. The permuter's own best find (score 405 vs base 855, a nominal 53% improvement) is engine distance 18  -  four instructions WORSE than where it started.
- verdict: KILLED

## [s4] decomp-permuter's default scorer is anti-correlated with the engine's honest distance for a function whose residual is a small instruction DELETION near the top of the body.
- mechanism: The permuter weights reorderings at 60 and ins/del at 100 and aligns the two instruction streams globally. Two missing instructions early in the body displace the whole tail, so the aligner charges ~14 reorderings (855 ~= 14 x 60) even though 125 of 127 instructions are byte-identical. The gradient therefore rewards mutations that re-sync the aligner by CHANGING the already-correct tail. The engine metric counts differing instructions with branch/jump targets masked and has no displacement term, so the two disagree by construction.
- probe: Re-scored every saved permuter output with sandbox --disable all and tabulated permuter score against engine distance (tmp/grind/func_80017848/s4/batch_score.ps1; 20-row table in evidence.md E-s4-2, full 130-row table for chassis B in tmp_perm_ings_s4b_engine_scores.txt).
- result: Ranking inversion is systematic, not noisy: perm 405 -> engine 18, perm 480 -> 16, perm 575 -> 14, perm 775 -> 39, perm 845 -> 12, perm 855 (base tie) -> 14. The two BEST forms found all session sit at permuter score 845, i.e. in the permuter's worst decile.
- verdict: CONFIRMED

## [s4] Some combination of the loop-preheader spelling axes that s1-s3 only swept one-at-a-time reaches below 14.
- mechanism: The last three levers found on this function (do-while shape, guard comparison operand, guard association order) were each invisible until someone wrote that exact spelling, so combinations plausibly hid another.
- probe: Generated the full symmetric cross-product of the five preheader axes (guard-count base x preheader-p x base-spelling x body-index base x do-while-bound base = 72 variants, s4/gen_variants.py) and engine-scored all 72 in one pass (s4/sweep.sh -> sweep_results.txt).
- result: Minimum 14, maximum 19. Twelve distinct cells tie at 14; zero cells go below. New sub-finding: cells a1_b0_*_{d0,d2}_e1 reach 14 with a FRESH guard read, so s3's 'the guard must consume the hoisted slots' is a sufficient route to 14, not a necessary one  -  14 was a broad plateau over the family, not a knife-edge.
- verdict: KILLED

## [s4] Randomization CONFINED to the residual region, with the permuter used purely as a proposal generator and the ENGINE used as the objective, can find preheader structure that neither the hand sweeps nor the full spelling cross-product could reach.
- mechanism: The winning form leaves the spelling family entirely: it introduces a SECOND live pointer variable (`base = slots;`) that carries the hoisted pointer into the two scan-loop entry guards and is then re-pointed at the per-slot record inside each preheader. Every cell of the 72-cross-product routed the guards through one of the two pointers that already existed, so no respelling inside that family could express it. Frozen-list 'variable reuse for codegen control' family  -  `base` is READ by both loop guards before it is reassigned, so it is a live variable, not a dead store or a constant-holder.
- probe: Chassis B = the validated workspace with the residual region wrapped in PERM_RANDOMIZE so the randomizer cannot touch the 125 byte-exact instructions (s4/mk_perm_b.py); 733 s / 21911 iterations / 130 saved outputs; every output re-scored with sandbox --disable all (s4/score_outputs.sh); then a 4-cell cleanup sweep to pick the most readable spelling of the winner.
- result: Two outputs at engine 12 (output-845-1, output-845-3), both at permuter score 845. Cleanup sweep: both top guards through `base` = 12, both through `slots` = 12, mixed as emitted = 12, loop entry guards moved back onto `slots` = 19. Chosen form re-verified in src/ings.c at sandbox distance 12, 125 build insns vs 127 target.
- verdict: CONFIRMED

## s5 frontier (2026-08-18, floor 10)

### KILLED this session
- **H-s5-A** "Hoist the ctx+0xC read above each entry guard so ONE pointer feeds the guard
  address and the loop base (target's preheader order)." KILLED — 14..35 over ~40 cells,
  best 17 (7 worse than 10). cse folds `p + (slot_a<<6)` to one pseudo and the second addu
  plus the copy both vanish; the build drops to 122 insns. See E-s5-3.
- **H-s5-B** "Write target's `move a3,a0` as a C-level intermediate pointer copy." KILLED —
  32 cells, `cp0` and `cp1` score IDENTICALLY in every pair. GCC 2.7.2 coalesces the copy
  unconditionally. See E-s5-4.
- **H-s5-C** "The top-guard pointer must not be a named variable, so it dies at the join and
  the loops re-read." KILLED — inline-expression top guards tie at 10, never beat it. See
  E-s5-5.
- **H-s5-D** (carried from s4 frontier) "A THIRD live pointer variable / a different
  assignment of pointers across the guard sites extends the s4 lever." KILLED as an
  independent lever — the `pv2`/`share`/`q` axes in all three s5 sweeps (a distinct pointer
  variable for loop 2, and a distinct copy variable per loop) are inert at the 10-cells and
  strictly harmful elsewhere (`_q_` cells reach 31-34). The s4 lever's value was the
  guard-vs-top-guard SPLIT, not the pointer count.

### Live frontier for s6
- **H-s5-E. The residual is one property: target computes `ptr + (slot_a<<6)` TWICE per
  scan loop (guard address, then loop base) with the pointer live across the entry-guard
  branch; we compute it once.**
  *Mechanism:* in target the guard address and the loop base are separate pseudos that cse
  never unified, and the pointer pseudo is live-out on the guard's taken edge, so
  local-alloc cannot coalesce the copy that bridges them. Both of the obvious C spellings
  are now measured dead: one shared pointer (H-s5-A) makes cse fold them, and an explicit
  copy (H-s5-B) is coalesced away. What has NOT been tried is forcing the two computations
  to be non-equivalent to cse — e.g. deriving the guard address from a pointer expression
  whose value cse cannot prove equal to the base's (a different base object, a different
  cast chain, or a read of `ctx+0xC` placed so the two live in different extended basic
  blocks).
  *Next probe:* the .lreg/.greg dump read that s1-s4 all banked and none has spent, now
  regenerable for the 10-form in seconds via `tmp/grind/func_80017848/s4/dump.sh`. Grep the
  func_80017848 region of `ings_pp.c.cse` FIRST (does cse fold the two addu's, and at which
  insn?), then `.lreg`/`.greg` for the pseudo pair and its conflict/preference records.
  That distinguishes "cse folded them" from "they were never separate" and names which pass
  to attack. Do NOT open another preheader spelling sweep — three sessions and ~230 cells
  say that family is a plateau.

- **H-s5-F. The 4 top-block differing instructions are downstream of H-s5-E, not an
  independent lever.**
  *Mechanism:* target's second `bltz` has a `nop` delay slot because the first insn of its
  target block is `lw a0,0xC(s2)`; ours has `sll a1,s4,6` there (the pointer is already
  live, so only the shift is needed), and `reorg` steals it into the slot and retargets the
  branch to label+4. Fix H-s5-E and the delay slot should follow for free — so do NOT spend
  a session attacking the delay slot directly.
  *Next probe:* after any H-s5-E movement, re-diff with `s5/dis.sh` and check whether the
  `bltz` delay slot became a `nop` without further work. If it did not, `reorg.c`'s
  `fill_slots_from_thread` non-own-thread path is the named mechanism to read.

- **H-s5-G. Chassis-relative rejection: re-measure cheap per-axis spellings after every
  structural lever lands.**
  *Mechanism:* s3 rejected shift-first guard spelling with a correct measurement on the s3
  chassis; on the s4 chassis the same spelling is worth 2 instructions (E-s5-1). Rejected
  forms in this ledger are conditional on the chassis they were measured against.
  *Next probe:* whenever s6+ lands a new structural lever, re-run `s5/gen.py`-style
  sweeps over the cheap axes (guard operand order, base spelling, guard source, read
  placement) on the NEW chassis before declaring any of them dead. One sweep is ~50 cells
  and one blocking call.


## s5 CORRECTION (supersedes the s5 frontier item H-s5-G and the E-s5-1 claim)

- **H-s5-G is WITHDRAWN.** It asserted that rejected forms are chassis-relative,
  citing shift-first guard spelling as worth 2 instructions on the s4 chassis. That
  measurement was contaminated by an unused local declaration in the sweep template.
  Clean: pointer-first 12, shift-first 13. s3's pointer-first conclusion stands.
- **H-s5-H (new, and the practical one for s6).** Generated-variant sweeps on this
  function must prune the declaration block per cell; a fixed declaration block
  makes dead declarations look like spelling wins and is worth up to 3 instructions
  of false signal here. Use `s5/gen_clean.py` as the template shape, and re-apply
  the banked candidate from a clean tree and re-score it before writing any outcome.
- The three KILLS (H-s5-A hoisted read, H-s5-B explicit copy, H-s5-C top-guard
  variable) and the H-s5-D kill all stand: each was measured as matched pairs or at
  margins far larger than the contamination, with the identical declaration block on
  both sides.
- The live frontier is unchanged and is H-s5-E: spend the cc1 -da dump probe
  (`s4/dump.sh`, then read `ings_pp.c.cse`, then `.lreg`/`.greg`) to find out whether
  cse folds the guard-address addu into the loop-base addu. Floor stays 12.

## [s5] On the s4 `base = slots;` chassis, the two scan-loop entry guards want their count address written SHIFT-FIRST rather than POINTER-FIRST, contradicting s3's banked conclusion.
- mechanism: Shift-first makes GCC emit `addu v0,a1,a0` with the `sll` result in the LEFT operand, which is target's operand order at both scan-loop preheaders; pointer-first emits `addu v0,a0,a1`.
- probe: First measured by the 56-cell s5/gen.py sweep (shift-first cells 10, pointer-first cells 12) and INITIALLY BANKED AS CONFIRMED. Then re-measured cleanly at the end of the session, because re-applying the banked candidate from a `git checkout`d tree scored 13 where the sweep log said 10. s5/gen_clean.py regenerates the four cells {pointer-first, shift-first} x {base spelling} with the declaration block pruned to exactly the variables each cell uses.
- result: KILLED, and the earlier CONFIRMED verdict is WITHDRAWN. The sweep templates (gen.py, gen2.py, gen3.py) emitted a FIXED declaration block including `u8 *q;` (and in gen3 `u8 *r;`) for every cell, so cells that never referenced them carried dead declarations. Clean numbers: pointer-first + no dead local = 12; shift-first + no dead local = 13; shift-first + `u8 *q;` = 10; pointer-first + `u8 *q;` = 12. The dead declaration is worth 3 instructions on the shift-first chassis and 0 on the pointer-first one; the operand order is worth nothing on its own and is a 1-instruction REGRESSION. s3's pointer-first conclusion stands and the honest floor is 12, unchanged. The dead declaration is not shippable (cheat-checklist T1/T2/T6, dead-local family) and is banked as rejected/unused_local_decl_q_contaminates_sweeps.c as an artifact to avoid, not a lever to spend.
- verdict: KILLED

## [s5] Hoisting the `*(u8 **)(ctx + 0xC)` read ABOVE each entry guard, so ONE pointer feeds both the guard's count address and the loop base, reproduces target's preheader order (`lw a0,0xC(s2)` before the guard) and closes the residual.
- mechanism: Target loads the pointer before the guard, uses it for the guard address, keeps it live across the `blez`, copies it (`move a3,a0`) and recomputes the loop base from the copy - so `ptr + (slot_a<<6)` is computed twice. The hypothesis was that a single C-level pointer read placed before the guard would produce that shape.
- probe: Axis G=1 in s5/variants (28 cells) and axis K=1 in s5/variants3 (24 cells), each engine-scored; plus a full instruction-level diff of the best such form (s5/probe3.sh on variants3/k11_c0_cp0_s1.c).
- result: Every cell scores 14-35; the best is 17, i.e. SEVEN instructions worse than the 10-form. The diff explains it: with one C pointer feeding both, cse folds `p + (slot_a<<6)` into a single `addu a0,v1,v0`, the guard address and the loop base become the same pseudo, and BOTH the second addu and the copy disappear - the build falls to 122 insns against target's 127. The form gets target's ORDER right and its instruction COUNT further wrong.
- verdict: KILLED

## [s5] Target's uncoalesced `move a3,a0` can be reproduced by writing the copy explicitly in C as a named intermediate pointer (`q = p; base = q + (slot_a << 6);`).
- mechanism: If GCC kept a C-level pointer copy as a distinct pseudo, the `move` would materialize and the loop base would be recomputed from the copy exactly as in target.
- probe: Axis `cp` in s5/gen3.py: all 32 cells generated in matched pairs (with/without the copy) across both the read-after-guard and read-before-guard chassis, with and without a distinct copy variable per loop; every pair engine-scored via s5/sweep3.sh.
- result: cp0 and cp1 score IDENTICALLY in every single pair (10/10, 17/17, 23/23, 31/31). GCC 2.7.2 coalesces the copy unconditionally, so a C copy statement is codegen-inert here. There is no C-level handle on target's copy via this route.
- verdict: KILLED

## [s5] Target's top-guard pointer dies at the join (it is v1 and is never reused) because no C variable holds it; removing the `slots` variable and spelling the top guards as two inline `*(u8 **)(ctx + 0xC)` reads will make the pointer die and force the loops to re-read it.
- mechanism: A named live variable keeps the value in a register across the join, so the loop preheaders never need their own load; an inline expression consumed only by the two top guards should let cse fold the pair inside the top-guard extended basic block and then let the value die.
- probe: s5/gen2.py generated all 40 cells of the no-`slots`-variable family (top guards inline) across the same G/o/c/pv2/base-seed axes; s5/sweep2.sh engine-scored them (s5/sweep2_results.txt).
- result: Best cells TIE at 10 and none beats it; the 11-cells mirror the 10-cells exactly one operand-order step away. Whether the top-guard pointer is a named variable or an inline expression is codegen-inert - the `base` seed keeps the value live either way.
- verdict: KILLED

## [s5] (Carried from the s4 frontier) A THIRD live pointer variable, or a different assignment of the existing pointers across the four guard sites, extends the s4 two-pointer lever the rest of the way.
- mechanism: s4's lever worked by giving the loop entry guards a pseudo distinct from the one the top guards use, which changed what local-alloc could coalesce; the obvious next notch is routing more of the six sites through more named pointers.
- probe: Covered as the `pv2` axis in s5/gen.py and s5/gen2.py (loop 2 takes its own pointer `q`) and the `share`/`cp` axes in s5/gen3.py (a distinct source pointer AND a distinct copy variable per loop) - 128 engine-scored cells in total across the three sweeps.
- result: Inert at every 10-cell and strictly harmful elsewhere: the `_q_` cells reach 31-34 in sweep 1 and 34 in sweep 2. The s4 lever's value was the guard-vs-top-guard SPLIT, not the number of pointer variables; there is no further notch on this axis.
- verdict: KILLED

## [s5] A directed permuter campaign scoped to the residual region of the 10-form finds a body below distance 10.
- mechanism: The residual is now 10 differing instructions concentrated in the two scan-loop preheaders and the top-guard block; PERM_RANDOMIZE over exactly those regions lets the randomizer explore the residual without disturbing the 125 already-byte-exact instructions.
- probe: Two campaigns on the validated s4 workspace built from the 10-form, launched via tools/permuter_campaign.py with telemetry: chassis C (s4/mk_perm_b.py region - the two scan loops only) and chassis D (new s5/mk_perm_d.py region - widened to include the top-guard block, because 4 of the 10 differing insns live there and every prior chassis froze it). 633 s each, 13447 + 13293 iterations, 88 outputs, every one re-scored with `sandbox --disable all` (s5/score_outputs.sh). A third campaign, chassis E, was run on the structurally different read-before-guard base as the fresh-seed reseed. All campaigns harvested with --stop. Chassis E was built from the read-before-guard form (variants3/k11_c0_cp0_s1.c, 122 insns, engine 17) with the widened region - a structurally different base, per the fresh-seed rule - and ran 31600 iterations to 205 outputs.
- result: Nothing below 10. Distributions - C: 3x10, 1x11, 4x12, 5x13, 5x14, 6x15, 7x16, 3x17, tail to 27; D: 4x10, 7x12, 6x13, 4x14, 6x15, 7x16, 2x17, 2x18, tail to 35. E-s4-2's anti-correlation reproduced exactly: each campaign's best permuter-ranked find (620) is not among the engine-best cells, while chassis D's mid-ranked output-685-1 ties the base at 10 with a structurally different body. 2 of 88 outputs could not be scored (apply_find.py could not locate the function in the permuter's reformatted source) - a bounded, recorded gap. Chassis E: 1x10, 7x11, 1x12, 15x13, 13x14, 15x15, 20x16, 22x17, tail to 121 (4 apply-fails, 1 unscorable); its single best cell TIES 10 and came from permuter score 540, mid-ranking again. Across s4+s5 that is 4 chassis, ~80k iterations and 423 engine-scored outputs with zero finds below the running floor; both of this session's floor drops came from directed hand sweeps, not the randomizer. permuter_campaign.py status reports 0 live campaigns.
- verdict: KILLED

## s6 (2026-08-18) — forensics

### H-s6-1 — CONFIRMED (and it retires the chassis)
**Statement.** The inherited 12-form's `base = slots;` reuse makes loop 2's entry guard
address `ptr + 2*slot_a*64 + 0x20` on the loop-1-ran path, where target addresses
`ptr + slot_a*64 + 0x20`; the form is therefore semantically divergent and cannot reach 0.
**Mechanism.** `base` is re-pointed to `(slot_a << 6) + (s32)p` in loop 1's preheader and
loop 2's guard still reads through it. Target reloads the pointer at `0x80017914` in loop
1's exit tail precisely to avoid that.
**Probe.** `cc1 -da` cse dump (insn 146 consumes reg/v79 set at insn 86) plus the build /
target instruction diff (`s6/build.txt` vs `s6/target.txt`).
**Verdict.** CONFIRMED divergent. Chassis retired; `rejected/base_reuse_loop2_guard_is_semantically_divergent.c`.

### H-s6-2 — CONFIRMED
**Statement.** cse.c's extended-basic-block equivalence table is what collapses the
E-s5-3 "hoisted read" family, and the collapse is defeatable by sourcing the base's addend
from a pseudo that was live before the guard block's join label.
**Mechanism.** The loop preheader is the fall-through successor of the guard block, so it
is in the same EBB; `(plus shift p)` is already in the hash table from the guard-address
insn and cse rewrites the preheader insn to a register copy (hoist dump insn 83). A
pre-join pseudo (`slots`) has no known value in the post-join EBB, so no fold occurs.
**Probe.** `s6/hoist/ings_pp.c.cse` insn 83 vs `s6/G/ings_pp.c.combine` insns 71/86.
**Verdict.** CONFIRMED. Variants D/G reproduce target's pre-guard `lw` and both addus; the
cost is keeping `slots` live (15 vs B's 14).

### H-s6-3 — CONFIRMED (and it corrects E-s5-4's attribution)
**Statement.** No C-level copy statement can materialize target's `addu a3,a0,zero`,
because combine.c deletes it before register allocation — not because the allocator
coalesces it.
**Mechanism.** `try_combine` substitutes a single-use register copy into its use and the
dead copy insn is removed; the copy is `NOTE_INSN_DELETED` already in the `.combine` dump.
**Probe.** Variant G (`q = slots;` on the fold-defeated chassis, i.e. the one regime where
E-s5-4's kill does not apply by construction) — `s6/G/ings_pp.c.combine` note 82; score 15
with 125 build insns, identical to D.
**Verdict.** CONFIRMED. Do not re-propose a copy statement in any spelling.

---

## Live frontier for s7+

1. **The last 2 instructions are the two `addu a3,a0,zero` copies, and they are a
   whole-function live-range property, not a preheader spelling.**
   *Mechanism.* In target the loop-1 pre-guard pointer `a0` is live-out on the entry
   guard's **taken** edge: the `blez` at `0x800178C8` jumps to `.L8001791C`, which consumes
   `a0` as loop 2's guard operand. The preheader therefore cannot overwrite `a0` with the
   loop base, so the allocator emits the copy and then `addu a0,a1,a3`. Every form we have
   built lets the guard pointer die at the branch, so the allocator overwrites it and the
   copy never appears.
   *Next probe.* Build a form where ONE pointer variable is read fresh after each join,
   used by that loop's entry guard, AND still read by the NEXT guard on the skip path — so
   that its live range spans the taken edge. Concretely: a single `p` assigned before loop
   1's guard and re-assigned only in loop 1's exit tail (i.e. after the `do/while`, not
   before loop 2's guard), which is exactly the data flow the target asm shows. Start from
   variant D (`rejected/base_from_pre_join_pseudo_defeats_cse_fold_costs_1.c`), not from B,
   because D already has the pre-guard read and the un-folded addus. Read
   `tmp/grind/func_80017848/s6/G/ings_pp.c.greg` for reg/v77's conflict and preference
   records before writing C, and re-dump with `s6/probe.sh` after each attempt.

2. **Every s3/s4/s5 conclusion that was measured ON the divergent chassis must be treated
   as chassis-relative and re-measured before it is spent.**
   *Mechanism.* The divergence deleted two pointer reloads, which shifted register
   pressure and scheduling for the whole preheader region; rankings measured against a
   125-insn build that was 2 insns short for a semantic reason do not transfer to a
   correct 125-insn build. s5 already learned the same lesson in a weaker form (its
   "shift-first" conclusion was chassis-relative to s4's `base = slots;`).
   *Next probe.* Before re-using ANY banked spelling conclusion, re-run it as a matched
   pair on the current candidate with `s6/score.sh`. Cheap (one blocking call per ~6 cells)
   and it is the only thing that makes an inherited conclusion trustworthy. The kills that
   DO survive untouched are the pass-level ones (H-s6-2, H-s6-3) because they are RTL
   facts, not score comparisons.

3. **The top-block 4-instruction shadow is still downstream, and still should not be
   attacked directly.**
   *Mechanism.* Unchanged from s5: target's second `bltz` has a `nop` delay slot because
   the first instruction of its target block is `lw a0,0xC(s2)`; ours has `sll a1,s4,6`
   there and `reorg`'s `fill_slots_from_thread` steals it, retargeting the branch to
   label+4. Variant D already puts a `lw` first in that block, so this may fall for free.
   *Next probe.* After ANY movement on frontier item 1, re-diff with `s6/dis.sh` and check
   whether the `bltz` delay slot became a `nop` on its own. Only if it did NOT should
   `reorg.c`'s non-own-thread `fill_slots_from_thread` path be read as its own mechanism.

## [s6] The s3/s4/s5 chassis (base = slots; base re-pointed in loop 1's preheader; loop 2's entry guard still reading its count through base) is semantically divergent from the target and therefore cannot produce target's bytes at any distance.
- mechanism: On the path where loop 1's guard passed, `base` holds ptr + slot_a*64, so loop 2's guard addresses ptr + 2*slot_a*64 + 0x20. Target addresses ptr + slot_a*64 + 0x20 on that path: at 0x80017914, in loop 1's exit tail, it does `lw a0,0xC(s2)` then `sll a1,s4,6` before falling into .L8001791C's `addu v0,a1,a0 ; lw v0,0x20(v0)`. Byte-identical code implies identical semantics, so a divergent C body cannot close.
- probe: cc1 -da cse dump sliced to the function (tmp/grind/func_80017848/s6/dump/ings_pp.c.cse): insn 86 sets reg/v79 = (plus reg92 reg/v78) in loop 1's preheader and insn 146, loop 2's guard address, is (set reg111 (plus reg/v79 reg110)) consuming that stale pseudo. Cross-checked against the build-vs-target instruction diff (s6/build.txt vs s6/target.txt): our loop-2 guard is `sll a1,s4,6 ; addu v0,a0,a1 ; lw v0,32(v0)` with a0 = loop 1's base; target's is `lw a0,0xC(s2) ; sll a1,s4,6 ; addu v0,a1,a0 ; lw v0,0x20(v0)`.
- result: Divergence confirmed twice over. The 12 score was 2 instructions of false credit: the two 'missing' insns (125 vs 127) ARE the two pointer reloads the divergence deleted. Seven semantically-correct forms measured; best is 14, and HEAD's untouched form is 16.
- verdict: CONFIRMED

## [s6] cse.c's extended-basic-block equivalence table is what collapses the E-s5-3 'hoisted read' family, and the collapse is defeatable by sourcing the loop base's addend from a pseudo that was live BEFORE the guard block's join label.
- mechanism: The loop preheader is the fall-through successor of the guard block, so it lies in the same extended basic block; (plus shift p) is already in cse's hash table from the guard-address insn, so cse rewrites the preheader insn to a register copy and the second addu never reaches the allocator. A pre-join pseudo has no known value in the post-join EBB (cse's table is reset at a join), so no fold occurs.
- probe: Dumped the E-s5-3 hoisted form (s6/hoist/ings_pp.c.cse): insn 71 `reg93 = reg92 + reg/v78` is the guard address, insn 83 is `(set reg/v79 reg93)` — a copy, not an addu. Then built variants D and G (guard through the fresh post-join read, base through the pre-join `slots` pseudo) and dumped G: s6/G/ings_pp.c.combine shows insn 71 `reg94 = reg/v78 + reg93` and insn 86 `reg/v80 = reg93 + reg/v77` as two independent addsi3_internal insns.
- result: Fold defeated. D/G's build contains BOTH of target's structural features the 12-form was faking: the pre-guard `lw v0,12(s2)` and two separate addus. Cost is keeping `slots` live across both loops, so D/G score 15 vs B's 14.
- verdict: CONFIRMED

## [s6] No C-level copy statement can materialize target's `addu a3,a0,zero`, and s5's attribution of this to register-allocator coalescing (E-s5-4) is wrong — combine.c deletes the copy before allocation.
- mechanism: combine.c's try_combine substitutes a single-use register copy into its one use and the now-dead copy insn is deleted; local-alloc never sees it. E-s5-4 measured only on chassis where cse had already folded the base addu away, so it could not distinguish the two passes.
- probe: Variant G writes `q = slots; base = (u8 *)((slot_a << 6) + (s32)q);` on the fold-defeated chassis — the one regime where E-s5-4's kill does not apply by construction. In s6/G/ings_pp.c.combine the copy's insn is already `(note 82 ... NOTE_INSN_DELETED)`. Scored end-to-end: G = 15 with 125 build insns, byte-for-byte the same shape as D.
- result: Copy statement is completely inert. Two named passes now cover the whole 'make a copy appear' family: cse.c when the two expressions are equal, combine.c when they are not.
- verdict: CONFIRMED

## [s6] Repairing the divergence by giving loop 2's guard its own fresh pointer read is cheap and keeps the 12-form's other levers.
- mechanism: A fresh `base = *(u8 **)(ctx + 0xC);` immediately before loop 2's guard restores target's data flow without touching the do-while shape, the `i = 0; if (i < count)` guard shape, or the hoisted slots read.
- probe: Variant A scored from a clean tree (s6/score.sh, s6/scores.txt).
- result: A = 15, one worse than variant B's 14. Reading both scan-loop guards through the never-re-pointed `slots` (B) is the cheaper repair.
- verdict: KILLED

## s7 (2026-08-18) - forensics

### H-s7-1. CONFIRMED. The loop-1 exit-tail reload is C-expressible and produces target's skip-past-the-reload branch.
Mechanism: one pointer variable `p` read before loop 1's guard and re-read only in loop 1's
exit tail; loop 2's guard reads it too, so on the skip path it consumes the pre-guard value
and GCC lets the `blez` branch into the middle of loop 2's guard block.
Probe: variant J, `s7/v/J.c`, built and disassembled with addresses (`blez v0,1444` skips
`lw a0,12(s2)` at `1440`).  Result: control flow matches target exactly.  Verdict: CONFIRMED.

### H-s7-2. CONFIRMED. The fold-defeat lever's SIGN was chassis-relative; on the corrected chassis it is worth 3 instructions.
Mechanism: with the exit-tail reload in place, the entry guard's address and the loop base
are the same expression in the same cse EBB unless the base's addend is a pseudo that was
live before the join.  Sourcing it from `slots` leaves both `addsi3`s alive.
Probe: J (both through `p`) = 14 / 124 insns; O (base through `slots`) = 11 / 125 insns;
s7/O/ings_pp.c.combine insns 146 and 158.  Verdict: CONFIRMED - floor 14 -> 11.

### H-s7-3. KILLED. The cse fold can be defeated by WHERE the base is written.
Probe: variant M (base assigned inside the do-loop body) and N (no base local at all).
Both build to 124 insns / 14, identical to J.  loop.c hoists, cse2 folds.  Verdict: KILLED.

### H-s7-4. KILLED. An explicit `shift` local carried across loop 1's guard edge buys the skipped `sll`.
Probe: K (J chassis) = 16, X (O chassis) = 17, against O = 11.  Verdict: KILLED.

### H-s7-5. KILLED. The preheader's lw/addu order is source-controllable.
Probe: Y1 (explicit `links` local read before the base) = 15; Y2 (after) = 11 = O.
Verdict: KILLED - scheduling artifact.

### H-s7-6. KILLED. The guard/base roles are interchangeable.
Probe: V (guards through the pre-join `slots`, bases through the fresh `p`) = 16; W (only
loop 2's base through `p`) = 14; S (only loop 1's base through `slots`) = 14.  The guard
must consume the fresh read and the base must consume the pre-join read, in that assignment
and in BOTH loops.  Verdict: KILLED (asymmetric and swapped assignments).

### FRONTIER FOR s8
1. The residual is exactly one dead register copy per scan loop (`addu a3,a0,zero`,
   read once by the next `addu` and never again).  s6 closed both C-level routes to a copy
   by name (cse.c folds it when the expressions are equal, combine.c's try_combine
   propagates and deletes it when they are not), so the copy has to come from a pass.  The
   forensics question for s8 is narrow and answerable: WHICH GCC 2.7.2 pass emits a
   redundant reg-reg move into a loop preheader?  Read `tools/gcc-2.7.2/loop.c`
   `move_movables` (the `m->move_insn` path emits `(set new old)` at the loop start) and
   `local-alloc.c` `block_alloc`/`combine_regs` (the tie-the-output-to-a-dying-input path)
   against `s7/O/ings_pp.c.loop`, `.lreg` and `.greg`, and ask what source shape makes the
   base's addend NOT die at the base's own insn.  In O it dies there
   (`REG_DEAD reg/v77` on insn 158).
2. Register naming: ours keeps `slots` live (slots=a2, links=a1); target lets its top-guard
   `slots` read die after the two `>=0` guards (slots=v1) and uses links=a2.  If the copy in
   frontier item 1 is solved by giving the base's addend its own short live range, the
   naming should follow for free - do not attack it separately first.
3. Do NOT re-open: source-level copy statements (s6, pass-level kill), base placement
   (H-s7-3), an explicit `shift` local (H-s7-4), explicit `links` locals (H-s7-5), swapped
   guard/base roles (H-s7-6).  And do not re-measure a banked spelling conclusion without
   re-running it on the CURRENT chassis first - s7 is the second session in a row where a
   banked verdict flipped sign after the chassis changed.

## [s7] The pointer reload that target executes between the two scan loops is a SOURCE-LEVEL reload in loop 1's exit tail, and writing it there reproduces target's control flow, where loop 1's entry-guard branch skips the reload and lands in the middle of loop 2's guard block.
- mechanism: Target's blez at 0x800178C8 targets .L8001791C = instruction index 53, skipping both `lw a0,0xC(s2)` and `sll a1,s4,6` (indices 51/52) and landing on `addu v0,a1,a0`, the second insn of loop 2's guard. That is only expressible if ONE pointer variable feeds both entry guards and is re-read only in loop 1's exit tail (after the do/while, inside the if): on the skip path loop 2's guard consumes the pre-guard value, on the loop-ran path the reload. Semantically the reload is a no-op because the scan loops contain no stores, so this is plain equivalent C.
- probe: Variant J (tmp/grind/func_80017848/s7/v/J.c) built and disassembled with addresses: `blez v0,1444` where 1440 is `lw a0,12(s2)` and 1444 is the `sll` - the reload is skipped on the guard's taken edge, exactly target's shape.
- result: Control flow matches target; J itself scores 14 at 124 build insns (the preheader is wrong for a separate reason, see H-s7-2).
- verdict: CONFIRMED

## [s7] s6's cse-fold-defeat lever (source the loop base's addend from the PRE-JOIN `slots` read rather than from the post-join fresh read) was banked with the wrong sign: it cost 1 instruction on s6's chassis but is worth 3 on the chassis that carries the exit-tail reload.
- mechanism: With the exit-tail reload in place, the entry guard's address and the loop base are both `(plus shift p)` and sit in the SAME cse extended basic block (the preheader is the fall-through successor of the guard block), so cse.c rewrites the base into a copy of the guard address in loop 1 and reuses the guard's address register outright as the base in loop 2 - 124 insns. Making the base's addend a pseudo that was live BEFORE the join (`slots`) leaves the two addends un-equatable and both addsi3s survive.
- probe: J (guards AND bases through `p`) vs O (guards through `p`, bases through `slots`), both scored from a clean tree with s7/score.sh; plus the RTL at tmp/grind/func_80017848/s7/O/ings_pp.c.combine, insn 146 `reg111 = reg110 + reg/v78` (guard) and insn 158 `reg/v79 = reg110 + reg/v77` (base) as two independent addsi3_internal insns.
- result: J = 14 at 124 insns; O = 11 at 125 insns. O re-verified end to end by re-applying memory/grind/func_80017848/candidate.c to a git-checkout'd src/ings.c and re-scoring: 11.
- verdict: CONFIRMED

## [s7] The cse fold can be defeated by WHERE the base assignment is written (inside the loop body instead of the preheader) rather than by the addend's live range.
- mechanism: If the base is written inside the do-loop body, cse's first pass sees it in a different extended basic block from the guard (the loop top is a join), so the equivalence table should be empty for that expression.
- probe: Variant M (base assigned inside both do-loop bodies) and variant N (no base local at all, the full expression inlined in the loop), scored from a clean tree.
- result: M builds byte-identically to J (124 insns, 14) and N is also 14: loop.c hoists the invariant into the preheader and the second cse pass folds it exactly as cse1 would have. Placement is irrelevant; only the addend's live range matters.
- verdict: KILLED

## [s7] An explicit `shift` local assigned before loop 1's guard and recomputed in loop 1's exit tail buys the `sll` that target also skips on the guard's taken edge.
- mechanism: Target's skip edge bypasses both `lw a0,0xC(s2)` and `sll a1,s4,6`, so the shift must be live across that edge and recomputed on the loop-ran path - the same shape as the pointer.
- probe: Variant K (that shift local on the J chassis) and variant X (the same on the O chassis), scored from a clean tree against O = 11.
- result: K = 16, X = 17. The extra live pseudo costs far more than the one `sll` it saves; letting GCC recompute the shift at the join is correct.
- verdict: KILLED

## [s7] The preheader's instruction ORDER (target: copy / lw links / addu base; ours: addu base / lw links) is controllable from the source by hoisting the links read into an explicit per-loop local.
- mechanism: If the links pointer is read into a named local placed ahead of the base assignment, the RTL order should follow the source order into the preheader.
- probe: Variant Y1 (links local read BEFORE the base) and Y2 (AFTER), scored from a clean tree.
- result: Y1 = 15, Y2 = 11 (identical to O without the local). The order is a scheduling artifact and the local is inert at best.
- verdict: KILLED

## [s7] The guard / base roles are interchangeable - it does not matter which of the two pointer reads (pre-join `slots` vs fresh post-join `p`) feeds the entry guard and which feeds the loop base.
- mechanism: Both reads load the same memory and the same value, so either assignment should defeat the cse fold equally.
- probe: Variant V (guards through `slots`, bases through `p`), W (only loop 2's base through `p`), S (only loop 1's base through `slots`), scored from a clean tree against O = 11.
- result: V = 16, W = 14, S = 14. The guard must consume the FRESH read (it is what target's cross-jump edge carries into loop 2) and the base must consume the PRE-JOIN read, in that assignment and in BOTH loops.
- verdict: KILLED
