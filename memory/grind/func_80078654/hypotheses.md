# HYPOTHESES — func_80078654

## RESOLVED

### H1 — CONFIRMED (floor 23 -> 19, insn count 115 -> 116)
**Statement.** The `v` / `v_copy` pair plus the inline `move` asm in the
inherited body is not just a cheat, it is the WRONG shape. The target's
fade-value clamp is a single SHORT-typed local: `sll $v0,$v0,16; bgez` is
GCC 2.7.2's canonical test of `(s16)x < 0`, and the `move $v1,$v0` /
`move $v0,$v1` pair around it is just GCC preserving the value across the
destructive `sll`.

**Mechanism.** A `short` local forces the sign test to be done on the
low half-word (shift-left-16 then test the sign bit) rather than a direct
`bltz`, and forces GCC to copy the un-shifted value to a second pseudo
because the `sll` clobbers its source register. The `move $v0,$v1` restore
then gets duplicated into the branch delay slot and the else-arm.

**Probe.** Replaced
```c
s32 v_copy;
v = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
__asm__ volatile("move %0, %1" : "=r"(v_copy) : "r"(v));
if ((s32)(v << 16) < 0) { v_copy = 0; }
s.r = (s.g_ = (s.b_ = (u8) v_copy));
```
with
```c
s16 sv;
sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
if (sv < 0) { sv = 0; }
s.r = (s.g_ = (s.b_ = (u8) sv));
```
(and deleted the now-unused `s32 v;`).

**Result.** Sandbox `--disable all` 23 -> 19. Build insn count 115 -> 116,
exactly matching target. The entire clamp region (target insns 38-46) now
matches byte-for-byte. **CONFIRMED — this is in candidate.c.** As a bonus
the function no longer contains any inline asm at all.

### H2 — KILLED (floor 23 -> 38, +5 insns)
**Statement.** Rewriting the trailing `goto`-loop as a real `while` loop
restores NOTE_INSN_LOOP_BEG/END, which makes flow.c weight the loop
pointer's `reg_n_refs` by loop_depth and should flip the $s0/$s1 inversion.

**Mechanism.** `flow.c:2081,2329,2515,2725` do `reg_n_refs[regno] +=
loop_depth`, and `basic_block_loop_depth` is derived purely from the
front-end's loop notes (flow.c:456,471), which a goto-loop never emits.

**Result.** KILLED, twice over. (a) loop.c ROTATES the `while` loop: it
duplicates the exit test to the top and deletes the target's entry
`j .L800787F8`, giving 121 insns vs 116, a fourth live callee-save ($s3),
and frame growth 88 -> 96. (b) It did not flip the allocation anyway — the
loop pointer still landed in $s1. Form banked at
`rejected/while-loop-triggers-loop-rotation.c`.

### H3 — KILLED analytically (do not re-run)
**Statement.** Some loop-depth ref weighting `w` exists at which var_s0
outranks arg0.

**Result.** KILLED. With `arg0 = 7 + 6w` refs and `var_s0 = 1 + 4w` refs
(their out-of-loop / in-loop split, read off the target bytes) and live
lengths 98 vs 91, `floor_log2(n)*n/len` keeps arg0 ~1.6-2.0x ahead of
var_s0 for every `w >= 1`. Ref weighting is not a route to the flip.

### H4 — KILLED by measurement (session 2; this was F1's named next probe)
**Statement.** Holding the `D_800A3610` base and the table-walk pointer in ONE
variable (`var_s0 = D_800A3610; s.a = var_s0[0xF]; ...; var_s0 += 5;`) lets the
walk pointer inherit the base's early references and long live range, lifting
its allocno priority above the parameter's and flipping $s0/$s1.

**Probe.** Applied the merge to src/text1b_b.c and read the priorities directly
off the instrumented cc1 (`BB2_ALLOC_DEBUG=1` on tools/gcc-2.7.2/cc1, exact
build flags via tmp/grind/func_80078654/s2/flowprobe.sh).

**Result.** KILLED. The merge does exactly what it was designed to do and is
still 1.63x short: walk pointer nrefs 5 -> 8, livelen 91 -> 98, pri
1098 -> **2448**, versus arg0's unchanged **3979**. Sandbox `--disable all`
19 -> 22 (worse; insn count stays 116 — the extra `addiu` does not land where
the target's does). 8 refs is the ceiling for this dataflow because
`D_800A3610` is dereferenced only once (`[0xF]`) outside the loop. Banked at
`rejected/base-merge-walk-pointer-pri-2448.c`. NB the analytic model predicted
2449 against a measured 2448, so the priority model can now be trusted to
predict a variant's outcome without a build.

### H5 — KILLED (session 2): the non-priority routes through find_reg
**Statement.** The target's allocation could be reached without a priority flip
— via `find_reg`'s pass-0 already-used-register preference, via a hard-reg
conflict on $s0, via a callee-save copy preference, or by an earlier
higher-priority allocno taking $s0 that the walk pointer could then share.

**Mechanism / probe.** Read all of `find_reg` + `prune_preferences` +
`global_alloc`'s seeding (global.c:289-1300) and measured `seed_used` for this
function with BB2_ALLOC_DEBUG.

**Result.** KILLED, every branch (full detail in evidence.md §SESSION 2):
pass 0's candidate set is measured to contain no callee-save at all
(`seed_used=0..15,24..29,31,32..51,64..67`); pass 1 is a pure ascending
first-fit with no cost comparison between free callee-saves;
`prune_preferences` strips arg0's only copy preference ($a0) and pure C cannot
make a callee-save a copy preference; the local-alloc eviction path needs
`best_reg < 0`; and no allocno can be disjoint from the walk pointer because
the walk pointer is live across the whole body (its def must precede block A,
else `D_800A3610` needs a second gp-rel load after block A's calls, which the
target does not have).

### H6 — KILLED (session 2): live_length is not inflatable by CFG shape
**Statement.** `zero`'s livelen of 170 in a ~116-insn function shows flow.c's
fixpoint iteration double-counts insns, so a CFG reshape could inflate arg0's
live length (pri 3979 falls below the walk pointer's 1098 at length > 355).

**Probe.** BB2_FLOW_DEBUG per-increment dumps for pseudos 72 and 74, segmented
per function and per basic block (tmp/grind/func_80078654/s2/seg.py).

**Result.** KILLED. 104 increments over 104 distinct insns for pseudo 72, 103
over 103 for pseudo 74, **zero repeated insns** — no double counting. The 170
comes from a different place entirely: `sched.c:5106` overwrites
`reg_live_length` with its post-scheduling recount, then
`local-alloc.c:1058-1064` DOUBLES it for any pseudo carrying a `REG_EQUIV` note
(170 = 2 x 85). That doubling is a real 2x priority-demotion lever, but it is
unreachable for arg0: the auto-note needs a MEM-sourced single set in ONE basic
block (local-alloc.c:1051-1055), and arg0 is a register-passed parameter
(`addu $s1,$a0,$zero`) spanning every block.

## LIVE FRONTIER

### F1 — SUPERSEDED by H4/H5/H6 (kept for the record)
The 19 remaining points are ONE two-way callee-save inversion whose cause is
measured exactly (evidence.md): allocno priorities 3979 (arg0: 13 refs /
98 live) vs 1098 (var_s0: 5 refs / 91 live), where the target needs the
second to sort first. The solved bounds say a flip needs var_s0 at >=13
refs or arg0 at <=5 refs, and BOTH ref counts are pinned by the target's own
emitted memory accesses. So the original C's dataflow differed.
**Next probe.** Hunt for a C shape that changes the PSEUDO PARTITION rather
than the statement order: e.g. does the original hold the `D_800A3610` base
and the walk pointer in ONE variable (so the walk pointer inherits the
base's early refs and long live range)? Does it reach `arg0[3]`/`arg0[5]`
through a shape that costs fewer `reg_n_refs` on the parameter pseudo?
Drive each variant with `BB2_ALLOC_DEBUG=1` on `tools/gcc-2.7.2/cc1` and
read the `pri=` numbers directly — that is a far finer gradient than the
sandbox score, which only reports the binary outcome of the sort.

### F4 - KILLED (session 3). Kept below for the record; do NOT re-run.
The duplication axis was measured on both shapes the CFG admits (D1: walk-only
initialiser into both arms; D2: whole loop into both arms). Neither is
byte-neutral (find_cross_jump re-merges neither: +2 and +39 insns), and the
reference arithmetic runs the wrong way - the loop body holds 6 arg0
references against 4 walk references, so duplication grows the parameter 1.5x
faster and the priority ratio has an asymptotic floor of 1.5. Walk-only
duplication yields +1 reference per junction and the function has exactly one
junction. Full numbers in evidence.md SESSION 3.

### F4 (original text) - session 2: the walk pointer needs ~9 byte-neutral references
The model is now exact and validated to 0.04% (H4). It says the ONLY remaining
way to flip the sort is to give the walk pointer **>= 14** RA-time references
(it has 5, and 8 is the ceiling for any reference-merging partition), while
arg0 keeps its 13 (pinned by the target's own emitted `12(sN)`/`20(sN)`
accesses). So ~9 references must exist at `flow`/`global_alloc` time and NOT
materialize as bytes.
**Mechanism.** The one sanctioned, documented byte-neutral ref-lift is
[[duplicated-statement-into-arms]]: `jump2`'s `find_cross_jump` runs AFTER
register allocation, so a REAL statement duplicated into two control-flow arms
is counted twice by `reg_n_refs` and then re-merged to identical bytes. All
other post-RA passes were checked for created references and none apply
(reorg only moves insns into delay slots here — no `0xC($s1)`/`0x14($s1)` sits
in a delay slot in the target, so all 12 buffer refs existed at RA time).
**Next probe.** Find a duplication of the walk-pointer statements
(`s.a = var_s0[0]`, `var_s0++`, `var_s0[1] != -1`) across arms that cross-jump
re-merges to the target's exact bytes, and measure `nrefs` for pseudo 73 with
BB2_ALLOC_DEBUG after each attempt (one cc1 run per form, no build needed).
Judge feasibility FAST: +9 refs is a large ask for a 3-statement loop body, and
the function's only junction is the block-A/skip join at `.L80078774`; if two
or three forms cannot get past ~10 refs, this axis should be reported dead and
the ladder moved on (F3 permuter, then forensics), not ground further.

### F2 — CLOSED (session 2): sibling census
RUN AND CLOSED in session 2 — 190 target functions copy `$a0` into `$s1` or
higher; the nearest COMPLETED-C sibling `func_80078824` (src/text1b_b.c:1059,
same file, same cluster) holds its parameter in $s1 and `arg0 + 0x58` in $s0
*because the parameter is referenced only 3 times*. It confirms the priority
mechanism and shows the target shape needs a reference-POOR parameter; no
sibling anywhere exhibits a reference-RICH parameter in $s1. Do not re-run.

Original text: `func_80078654` is one of a family in text1b_b.c that build the same
`S78654` descriptor and call the same
`func_8007352C`/`func_8006E480`/`SetDrawMode`/`AddPrim` quartet (see
`func_80078824`, `func_800788B0`, and the neighbours around src/text1b_b.c
line 900-1100). **Next probe.** Find a SIBLING in this cluster that is
already COMPLETED-C and holds both a parameter pointer and a walk pointer in
callee-saves, and diff its `.greg` / `BB2_ALLOC_DEBUG` numbers against ours.
A matched sibling is direct evidence of the dataflow shape the original
author used, and it costs one cc1 run.

### F3 — permuter on the now-clean base
The base is materially better than it was (19, exact insn count, zero inline
asm), which is a much better seed than any prior session had. **Next probe.**
`tools/permuter_campaign.py` with a single-function `target.o` built from
`asm/funcs/func_80078654.s` + `tools/decomp-permuter/prelude.inc` (drop the
`.set gp=64` line for r3000) so the function sits at offset 0 and the score
is the real weighted diff. Honour the fresh-seed stopping rule
([[permuter-fresh-seed-discipline]]) and WAIT for it in-turn
(`permuter_campaign.py wait --dir <ws>`) — never end a turn on a live
campaign.

## [s1] The fade-value clamp's v/v_copy pair plus the inline __asm__ move is the wrong shape; the target's clamp is a single SHORT-typed local, whose sign test GCC 2.7.2 emits as sll-16 + bgez with a copy around the destructive sll.
- mechanism: A `short` local forces the sign test onto the low half-word (shift-left-16 then test the sign bit) instead of a direct bltz, and forces GCC to preserve the un-shifted value in a second pseudo because the sll clobbers its source; the restoring move is then duplicated into the branch delay slot and the else-arm by jump2/reorg.
- probe: Replaced `s32 v_copy; ...; __asm__ volatile("move %0, %1" : "=r"(v_copy) : "r"(v)); if ((s32)(v << 16) < 0) v_copy = 0;` with `s16 sv; sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15); if (sv < 0) sv = 0;` and deleted the now-unused `s32 v;`. Measured with `sandbox func_80078654 --disable all` plus a normalized target-vs-build disassembly differ.
- result: Sandbox floor 23 -> 19; build instruction count 115 -> 116, exactly matching target; the whole clamp region (target insns 38-46) now matches instruction-for-instruction; the function now contains zero inline asm.
- verdict: CONFIRMED

## [s1] Rewriting the trailing goto-loop as a real `while` loop restores the front-end loop notes, so flow.c weights the walk pointer's reg_n_refs by loop_depth and flips the $s0/$s1 allocation inversion.
- mechanism: flow.c:2081,2329,2515,2725 do `reg_n_refs[regno] += loop_depth`, and basic_block_loop_depth is derived solely from NOTE_INSN_LOOP_BEG/END (flow.c:456,471), which a goto-spelled loop never emits — so today every in-loop reference is counted at weight 1.
- probe: Rewrote `goto check; loop: ...; check: if (var_s0[1] != -1) goto loop;` as `while (var_s0[1] != -1) { ... }` and re-ran the sandbox and the disassembly differ.
- result: Floor 23 -> 38 (worse). loop.c ROTATES the while-loop: it duplicates the exit test to the top (lw v1,4(sN); li v0,-1; beq) and deletes the target's entry `j .L800787F8`, giving 121 insns vs 116, a fourth live callee-save ($s3), and frame growth 88 -> 96 bytes. The allocation inversion was not fixed either — the walk pointer still landed in $s1. Banked as rejected/while-loop-triggers-loop-rotation.c.
- verdict: KILLED

## [s1] Some loop-depth reference weighting w exists at which the walk pointer outranks the arg0 pointer in allocno_compare, so the inversion is reachable by making GCC see the loop.
- mechanism: allocno_compare (tools/gcc-2.7.2/global.c) sorts by floor_log2(n_refs)*n_refs/live_length*10000*size and find_reg then hands out callee-saves in ascending hard-reg order, so the higher-priority allocno takes $s0.
- probe: Read the two pointers' in-loop / out-of-loop reference split directly off the target bytes (arg0 = 7 + 6w, var_s0 = 1 + 4w) with measured live lengths 98 and 91, and evaluated allocno_compare's formula over all w >= 1.
- result: arg0 stays 1.6x to 2.0x ahead of var_s0 at every weight. Reference weighting is arithmetically incapable of producing the flip. Do not re-run any loop-note / loop-depth variant for this purpose.
- verdict: KILLED

## [s2] Holding the D_800A3610 base and the table-walk pointer in ONE variable (var_s0 = D_800A3610; s.a = var_s0[0xF]; ...; var_s0 += 5;) lets the walk pointer inherit the base's early references and long live range, lifting its allocno priority above the parameter's and flipping the $s0/$s1 inversion. (This was the ledger's F1 named next probe.)
- mechanism: global.c allocno_compare sorts by floor_log2(n_refs)*n_refs/live_length*10000*size and find_reg hands out callee-saves ascending, so raising the walk pointer's reg_n_refs raises its priority; merging the base into the same variable adds the base's dereference and the +5 increment as extra references to that pseudo.
- probe: Applied the merge to src/text1b_b.c and read allocno priorities directly off the instrumented cc1 (BB2_ALLOC_DEBUG=1 on tools/gcc-2.7.2/cc1, exact build flags, tmp/grind/func_80078654/s2/flowprobe.sh), plus sandbox --disable all.
- result: KILLED. The merge works as designed and is still 1.63x short: walk pointer nrefs 5 -> 8, livelen 91 -> 98, pri 1098 -> 2448 vs arg0's unchanged 3979. Sandbox floor 19 -> 22 (worse), insn count stays 116. 8 refs is the ceiling for this dataflow because D_800A3610 is dereferenced only once ([0xF]) outside the loop. Analytic prediction was 2449 vs measured 2448, so the priority model now predicts variants without a build. Banked at rejected/base-merge-walk-pointer-pri-2448.c.
- verdict: KILLED

## [s2] The target's allocation is reachable WITHOUT a priority flip, through one of find_reg's non-priority routes: pass 0's already-in-use-register preference, a hard-reg conflict on $s0, a callee-save copy preference, or a higher-priority allocno taking $s0 whose live range is disjoint from the walk pointer's so the walk pointer can share $s0.
- mechanism: find_reg (global.c:952-1300) runs two passes: pass 0's candidate set is regs_used_so_far minus regs_someone_prefers minus conflicts (so no NEW callee-save save/restore is created), pass 1 is a first-fit walk of reg_alloc_order; afterwards a copy/full preference of the same class can override best_reg, and a local-alloc eviction path exists when best_reg < 0. regs_used_so_far is seeded from regs_ever_live + call_used_regs + local-alloc renumberings (global.c:344-372).
- probe: Read all of find_reg, prune_preferences and global_alloc's seeding, and measured this function's actual pass-0 seed with BB2_ALLOC_DEBUG (ALLOCDBG seed_used=) plus the conflict/preference dumps.
- result: KILLED on every branch. Measured seed_used = 0..15,24..29,31,32..51,64..67 — every call-clobbered register and NO callee-save, so pass 0 can never award $s0/$s1 here and both pointers are decided by pass 1's ascending first-fit (no cost comparison between two free callee-saves exists anywhere in find_reg). prune_preferences (global.c:899-909) strips call-clobbered regs from a call-crossing allocno's preferences, removing arg0's only copy preference ($a0), and pure C cannot make a callee-save a copy preference (that needs a forbidden register-asm pin). The eviction path needs best_reg < 0, which never occurs. The $s0-sharing route is impossible because the walk pointer is live across the ENTIRE body: its def must precede block A, since D_800A3610 is a global pointer and block A contains four calls, so any later def forces a second lw %gp_rel(D_800A3610) and the target has exactly one (insn 5).
- verdict: KILLED

## [s2] reg_live_length is inflatable by CFG shape — `zero` shows livelen 170 in a ~116-insn function, which would mean flow.c's fixpoint iteration double-counts insns, and arg0's priority 3979 falls below the walk pointer's 1098 once its live length exceeds 355.
- mechanism: flow.c increments reg_live_length once per insn a reg is live (flow.c:1685) plus once per set (flow.c:2087); if life analysis re-processes blocks until convergence, insns inside loops could be counted several times.
- probe: BB2_FLOW_DEBUG per-increment dumps for pseudos 72 (arg0) and 74 (zero), segmented per function and per basic block with tmp/grind/func_80078654/s2/seg.py.
- result: KILLED. Pseudo 72: 104 increments over 104 DISTINCT insns. Pseudo 74: 103 over 103 distinct insns. Zero repeated insns for either, so there is no double counting and CFG shape cannot inflate a live length. The 170 has a different cause, now identified: sched.c:5106 overwrites reg_live_length with its post-scheduling recount, then local-alloc.c:1058-1064 DOUBLES it for any pseudo carrying a REG_EQUIV note (170 = 2 x 85), which is also why `zero` sorts last into $s2. That doubling is a genuine 2x priority-demotion lever but is unreachable for arg0: the auto-note requires a MEM-sourced single set in ONE basic block (local-alloc.c:1051-1055) and arg0 is a register-passed parameter (addu $s1,$a0,$zero) spanning every block.
- verdict: KILLED

## [s2] A COMPLETED-C sibling in the same draw-primitive cluster exhibits the target's allocation shape (parameter pointer in the higher callee-save, walk pointer in the lower) and its C is direct evidence of the dataflow the original author used. (Ledger F2.)
- mechanism: Matched siblings compiled by the same frozen toolchain from adjacent-style C are the cheapest ground truth for which pseudo partition GCC 2.7.2 turns into which allocation.
- probe: Census over all 1437 asm/funcs/*.s for functions copying the incoming $a0 into $s1 or higher while also defining $s0, cross-referenced against src/ C bodies and engine/queue.json status (tmp/grind/func_80078654/s2/census.py).
- result: CONFIRMED that such siblings exist (190 functions), and the axis is CLOSED as a source of a new dataflow. The nearest COMPLETED-C sibling is func_80078824 (src/text1b_b.c:1059 — same file, same cluster, same D_800A3610/D_800A360C globals): it holds its parameter in $s1 and the derived pointer arg0 + 0x58 in $s0 precisely BECAUSE the parameter is referenced only 3 times while the derived pointer has 5 refs over a shorter range. It confirms the s1 priority mechanism and shows the target shape requires a reference-POOR parameter; no sibling exhibits a reference-RICH parameter in $s1, so there is no new partition to copy.
- verdict: CONFIRMED

## [s3] The sanctioned [[duplicated-statement-into-arms]] family can lift the walk pointer's RA-time reg_n_refs from 5 to >= 14 byte-neutrally, because jump2's find_cross_jump runs after register allocation and re-merges the duplicated arms to identical bytes, flipping the $s0/$s1 allocation. (Ledger F4, the named next probe.)
- mechanism: flow.c counts a reference once per insn mentioning the pseudo; find_cross_jump (jump.c, run by jump2 after global_alloc) merges identical block suffixes that reach a common label, so a real statement written into two arms is counted twice at RA time and emitted once. allocno_compare then sorts by floor_log2(n_refs)*n_refs/live_length, and the higher-priority allocno takes $s0 under find_reg's ascending first-fit.
- probe: Built the two duplication shapes this CFG admits and read the allocno table off the instrumented cc1 (BB2_ALLOC_DEBUG=1 on tools/gcc-2.7.2/cc1, exact build flags, tmp/grind/func_80078654/s3/probe.sh) plus the sandbox score for each. D1 = the walk-only initialiser `var_s0 = D_800A3610 + 5;` duplicated into both arms of the single junction; D2 = the entire trailing walk loop duplicated into both arms.
- result: KILLED on two independent grounds. (a) NOT BYTE-NEUTRAL: find_cross_jump re-merges neither shape - D1 sandbox 19 -> 25 with build_insns 116 -> 118 (a second lw %gp_rel(D_800A3610), new pseudo 76 in $v1), D2 sandbox 19 -> 58 with build_insns 116 -> 155 (the whole second loop copy survives). The sanctioned family's byte-neutrality prerequisite fails at the first measurement. (b) THE ARITHMETIC RUNS THE WRONG WAY: measured D2 gives arg0 nrefs 13 -> 19 / pri 3979 -> 5671 against walk nrefs 5 -> 9 / pri 1098 -> 2125, because the loop body - the only region containing walk references - holds SIX arg0 references against FOUR walk references. Duplication therefore grows the parameter 1.5x faster than the walk pointer, giving the priority ratio an asymptotic floor of 1.5; k=2 (ratio 2.67) is the best point on the curve and k=3 is worse (arg0 crosses floor_log2's 16-ref step). Walk-only duplication yields +1 reference per junction (D1: 5 -> 6 refs, pri 1098 -> 1411) and the function has exactly one junction, against a requirement of +9. Forms banked at rejected/dup-loop-into-arms-no-crossjump-merge.c and rejected/dup-walk-init-into-arms-second-gp-load.c.
- verdict: KILLED

## [s3] The s2 flip bound is one-sided ("walk >= 14 refs OR arg0 <= 5 refs"); because floor_log2 steps, the true condition is a 2-D frontier on the (arg0, walk) reference pair, and a joint move - lowering arg0 a little while raising walk a little - reaches it where either single-sided move cannot.
- mechanism: allocno_compare's priority is floor_log2(n)*n/live_length, and floor_log2 is a step function, so a reference count that drops below a power-of-two boundary loses a whole multiplier. At arg0 = 8 refs / len 98 the parameter scores 2448, which a walk pointer with only 8 refs / len 91 (2637) already beats.
- probe: Solved the 2-D condition over the reachable (a, w) lattice and checked each reachable move against the measured data: s2's split measurement (block-A-only arg0 holder: 7 refs / ~30 length / pri ~4666) for the arg0-lowering direction, and this session's D1/D2 measurements for the walk-raising direction.
- result: CONFIRMED that the bound is 2-D and materially looser than s2 stated - but the relaxation is unreachable in practice, so the conclusion is unchanged. arg0's 13 references are one def plus the target's own twelve `12($sN)`/`20($sN)` accesses; the only mechanism that lowers a single pseudo's count is splitting the accesses across pseudos, and every split produces a SHORTER-lived, HIGHER-priority half that takes $s0 itself (measured 4666 in s2). Both axes of the 2-D frontier are therefore closed by measurement, not by the one-sided arithmetic alone.
- verdict: CONFIRMED

## [s3] The sanctioned duplicated-statement-into-arms family can lift the walk pointer's RA-time reg_n_refs from 5 to >= 14 byte-neutrally, because jump2's find_cross_jump runs after register allocation and re-merges the duplicated arms to identical bytes, flipping the $s0/$s1 allocation. (Ledger F4, the named next probe.)
- mechanism: flow.c counts a reference once per insn mentioning the pseudo; find_cross_jump (jump.c, run by jump2 after global_alloc) merges identical block suffixes that reach a common label, so a real statement written into two arms is counted twice at RA time and emitted once. allocno_compare then sorts by floor_log2(n_refs)*n_refs/live_length and find_reg hands out callee-saves in ascending order, so the higher-priority allocno takes $s0.
- probe: Built the two duplication shapes this CFG admits and measured each with the instrumented cc1 (BB2_ALLOC_DEBUG=1 on tools/gcc-2.7.2/cc1, exact build flags, tmp/grind/func_80078654/s3/probe.sh) plus sandbox --disable all. D1 = the walk-only initialiser 'var_s0 = D_800A3610 + 5;' duplicated into both arms of the function's single junction; D2 = the entire trailing walk loop duplicated into both arms.
- result: KILLED on two independent grounds. (a) NOT BYTE-NEUTRAL: find_cross_jump re-merges neither shape — D1 sandbox 19 -> 25, build_insns 116 -> 118 (a second lw %gp_rel(D_800A3610), appearing as new pseudo 76 in $v1); D2 sandbox 19 -> 58, build_insns 116 -> 155 (the whole second loop copy survives). The sanctioned family's byte-neutrality prerequisite fails at the first measurement. (b) THE ARITHMETIC RUNS THE WRONG WAY: D2 measured arg0 nrefs 13 -> 19 / livelen 98 -> 134 / pri 3979 -> 5671 against walk nrefs 5 -> 9 / livelen 91 -> 127 / pri 1098 -> 2125, because the loop body — the only region containing walk references — holds SIX arg0 references against FOUR walk references. Duplication therefore grows the parameter 1.5x faster than the walk pointer, giving the priority ratio an asymptotic floor of 1.5; k=2 (ratio 2.67) is the best point on the curve and k=3 is worse (arg0 crosses floor_log2's 16-reference step: 4*25/170 = 0.588 vs 3*13/163 = 0.239). Walk-only duplication yields exactly +1 reference per junction (D1: 5 -> 6 refs, pri 1098 -> 1411) and the function has exactly ONE junction, against a requirement of +9. Forms banked at rejected/dup-loop-into-arms-no-crossjump-merge.c and rejected/dup-walk-init-into-arms-second-gp-load.c.
- verdict: KILLED

## [s3] The s2 flip bound is one-sided ('walk >= 14 refs OR arg0 <= 5 refs'); because floor_log2 steps, the true condition is a 2-D frontier on the (arg0, walk) reference pair, so a joint move — lowering arg0 a little while raising walk a little — reaches the flip where either single-sided move cannot.
- mechanism: allocno_compare's priority is floor_log2(n)*n/live_length and floor_log2 is a step function, so a reference count dropping below a power-of-two boundary loses a whole multiplier. At arg0 = 8 refs / len 98 the parameter scores 2448, which a walk pointer with only 8 refs / len 91 (2637) already beats — far short of the 14 references s2's one-sided bound demanded.
- probe: Solved the 2-D condition over the reachable (a, w) lattice and checked each reachable move against measured data: s2's split measurement (block-A-only arg0 holder, 7 refs / ~30 length / pri ~4666) for the arg0-lowering direction, and this session's D1/D2 measurements for the walk-raising direction.
- result: CONFIRMED that the bound is 2-D and materially looser than s2 stated, but the relaxation is unreachable, so the conclusion is unchanged and now rests on measurement rather than one-sided arithmetic. arg0's 13 references are one def plus the target's own twelve 12($sN)/20($sN) accesses; the only mechanism that lowers a single pseudo's count is splitting the accesses across pseudos, and every split produces a SHORTER-lived, HIGHER-priority half that takes $s0 itself. Both axes of the frontier are therefore closed.
- verdict: CONFIRMED

## [s4] A permuter campaign seeded from the clean floor-19 base reaches the $s0/$s1 flip where hand-derivation provably cannot, because random and directed structural mutation explores pseudo partitions and duplication shapes no analytic enumeration covers. (Ledger F3, the named next probe.)
- mechanism: decomp-permuter mutates the C source (statement reordering, temp introduction/removal, expression and cast respelling, &x[i] vs *(x+i), type changes) and scores each variant against target.o with the weighted permuter metric; with target.o built from asm/funcs/func_80078654.s + the r3000-ified prelude the function sits at offset 0, so the score is the real per-function diff and the search has a usable gradient.
- probe: Built the clean single-function rig (tmp/grind/func_80078654/s4/mkws.sh � validated base score 108 whose objdump diff prints exactly the twelve known $s0/$s1 lines) and ran two campaigns via tools/permuter_campaign.py with telemetry, honouring the fresh-seed stopping rule and waiting in-turn. Chassis A = random from the clean floor-19 base (39,636 iterations / 1005 s). Chassis B = random seeded from the s2 H4 base/walk-pointer merge, the only measured shape that lifts the walk pointer's allocno priority (1098 -> 2448), i.e. the structurally different chassis nearest the 2-D flip frontier (49,448 iterations / ~19 min).
- result: KILLED. 89,084 iterations, ZERO score-improving finds on either chassis. Chassis A never beat its own base of 108; its two saved finds are equal-score and both are semantics-CHANGING rewrites (var_s0 = &arg0[5]; SetDrawMode(*var_s0, ...) and var_s0 = &arg0[3]; s.c = *var_s0;) that clobber the walk pointer inside the loop, so neither is a usable form. Chassis B's basin is strictly worse (base 331, best find 191, no novel find in the final 9-minute window) and does not tunnel back toward 108. The negative is strong rather than unlucky: the permuter's mutation set is exactly the class of edits that preserves the emitted memory accesses, and the s1-s3 priority model proves every such edit leaves reg_n_refs(arg0) = 13 and reg_n_refs(walk) <= 8, which cannot flip allocno_compare. Both campaigns harvested and stopped in-session.
- verdict: KILLED

## [s4] A permuter campaign seeded from the clean floor-19 base reaches the $s0/$s1 allocation flip where hand-derivation provably cannot, because random and directed structural mutation explores pseudo partitions and duplication shapes no analytic enumeration covers. (Ledger F3, the named next probe.)
- mechanism: decomp-permuter mutates the C source (statement reordering, temp introduction/removal, expression and cast respelling, &x[i] vs *(x+i), type changes) and scores each variant against target.o with the weighted permuter metric; with target.o built from asm/funcs/func_80078654.s + tools/decomp-permuter/prelude.inc minus its '.set gp=64' line, the function sits at offset 0 so the score is the real per-function weighted diff (108 for the base) instead of ~340k of branch-address noise, giving the search a usable gradient.
- probe: Built a clean single-function rig (tmp/grind/func_80078654/s4/mkws.sh) whose compile.sh reproduces the exact build pipeline (cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel | prologue_fix | maspsx --expand-div --aspsx-version=2.34 + all five gate lists | multu_pad | as -march=r3000 -no-pad-sections -O1 -G0) over a standalone 6-typedef base.c holding only this function, and self-validates by objdump-diffing base.o against target.o. Ran two campaigns through tools/permuter_campaign.py (telemetry, -j 8, --stop-on-zero, --stack-diffs), honouring the fresh-seed stopping rule and waiting IN-TURN via 'permuter_campaign.py wait'. Chassis A = random from the clean floor-19 base. Chassis B = random seeded from the s2 H4 base/walk-pointer merge, the only measured shape that lifts the walk pointer's allocno priority (nrefs 5 -> 8, pri 1098 -> 2448) and therefore the structurally different chassis nearest the 2-D flip frontier. Both harvested with --stop before the session ended.
- result: KILLED. 89,084 iterations total, ZERO score-improving finds on either chassis. Chassis A (39,636 iterations / 1005 s) never beat its own base score of 108; its two saved finds are equal-score (108) and both are semantics-CHANGING permuter rewrites that clobber the walk pointer inside the loop ('var_s0 = &arg0[5]; SetDrawMode(*var_s0, ...)' and 'var_s0 = &arg0[3]; s.c = *var_s0;'), so neither is a usable form. Chassis B (49,448 iterations / ~19 min) started at 331, reached only 191, produced no novel find in its final 9-minute window, and never tunnelled back toward chassis A's 108 — a strictly worse basin. The negative is strong rather than unlucky: the permuter's mutation set is exactly the class of edits that preserves the emitted memory accesses, and the s1-s3 priority model proves every such edit leaves reg_n_refs(arg0) = 13 and reg_n_refs(walk) <= 8, which cannot flip allocno_compare.
- verdict: KILLED

## [s4] The 19-point residual is entirely the single two-way callee-save inversion the ledger describes, and that claim is not an artefact of the engine sandbox's own scorer.
- mechanism: engine sandbox and decomp-permuter score through completely independent pipelines (engine/score.py's masked instruction diff vs the permuter's weighted objdump diff against a separately assembled target.o); an agreement between them on WHICH instructions differ is independent corroboration rather than a repeat of the same measurement.
- probe: mkws.sh's self-validation step objdump-diffs the rig's base.o against the rig's target.o with relocations and hex literals normalized away, and prints the full instruction-level diff.
- result: CONFIRMED. The diff prints exactly twelve lines, all of them the known $s0/$s1 exchange (sw s0,72(sp)/move s0,a0 vs sw s1,76(sp)/move s1,a0; addiu s1,v1,20 vs addiu s0,v1,20; and the 12($sN)/20($sN)/0($sN) buffer and walk accesses), with identical instruction counts (116 == 116) and no other codegen delta. Permuter base score 108.
- verdict: CONFIRMED

## [s5] Directed permutation — hand-written PERM_* macros enumerating a chosen cross-product of genuine alternatives rather than uniformly sampling spellings — reaches the $s0/$s1 flip where s4's two random campaigns could not. (Ledger frontier item 3, recorded there as "measured-thin rather than untried".)
- mechanism: PERM_GENERAL / PERM_LINESWAP expand to EVERY listed alternative, so a small set of semantically-equivalent partition choices is covered exhaustively instead of sampled; combined with PERM_RANDOMIZE regions the search is a directed cross-product with random mutation layered on top, which is a materially different regime from s4's uniform random sampling.
- probe: Built tmp/grind/func_80078654/s5/ws (settings.toml/compile.sh/target.o copied from the validated s4 rig, so the function sits at offset 0 and the score is the real weighted diff) with a base.c carrying six directed axes: three walk-initialiser partitions, three block-A shapes including the `s32 *ot = &arg0[5]` arg0-lowering sub-pointer, four loop-head read shapes including the double-read `s.b = var_s0[0] + 0xC;` ref-lift candidate, a PERM_LINESWAP over the `var_s0++` / `arg0[5] += 0xC` pair, four loop-test spellings, and two PERM_RANDOMIZE regions. Launched via tools/permuter_campaign.py (label s5-directed-perm, -j 8, --stop-on-zero, --stack-diffs), waited IN-TURN with `permuter_campaign.py wait`, harvested with --stop.
- result: KILLED. Base score 108 (identical to the s4 chassis-A base, confirming the default expansion is the floor-19 form). 39,950 iterations in 1152 s, ZERO score-improving finds. Two equal-score (108) finds at 147 s and 584 s, then a full 9-minute window with no novel find, at which point the fresh-seed rule was honoured and the campaign was stopped (procs_killed 9). Both finds reproduce s4's failure mode exactly and are unusable for the same reason: each clobbers the walk pointer inside the loop (`var_s0 = &arg0[3];` and `var_s0 = &arg0[5];`), i.e. they are semantics-CHANGING, not respellings. Two campaigns under two different mutation regimes converging on the same semantics-breaking equal-score attractor means the attractor will absorb any future campaign on this chassis family; a third has negative expected value.
- verdict: KILLED

## [s5] The arg0-LOWERING quadrant of the s3 2-D frontier is reachable by hoisting a sub-pointer: `s32 *ot = arg0 + 5;` at function scope, with every `arg0[5]` rewritten as `*ot`, strips four references per block off the parameter pseudo and should drop its allocno priority below the walk pointer's. (No prior session built this shape — s2 measured only a block-SCOPED split and s4 seeded only the walk-RAISING chassis, so this was the last unmeasured quadrant.)
- mechanism: allocno_compare sorts by floor_log2(n_refs)*n_refs/live_length; arg0's 13 references are one def plus twelve emitted accesses, eight of which are `0x14($sN)`. Routing those eight through a separate pointer pseudo should leave arg0 with 5 references and a priority below the walk pointer's 1098, letting the walk pointer take $s0 under find_reg's ascending first-fit.
- probe: tmp/grind/func_80078654/s5/eval.sh on the standalone chassis (instrumented cc1 BB2_ALLOC_DEBUG allocno table plus a full-pipeline objdump diff against the s4 rig's target.o), banked as rejected/ot-subpointer-arg0-lowering-4th-callee-save.c.
- result: KILLED — and it yields the sharpest bound this ledger has. arg0's references DO drop 13 -> 6 and its priority 3979 -> 1212, but the walk pointer is STILL BELOW at 1086 (nrefs 5, livelen 92), because the comparison is 2*6/99 against 2*5/92 and floor_log2(6) == floor_log2(5) == 2. A seven-reference reduction of the parameter is therefore NOT enough; on the measured live lengths arg0 must reach <= 5 references (2*5/99 = 1010 < 1086), i.e. ELEVEN of its twelve emitted accesses must leave the pseudo. Worse, the extracted `ot` pseudo behaves exactly as s2/s3 predicted for every split — shorter-lived (85) with more references (9), priority 3176 — so it takes $s0 itself and forces a FOURTH callee-save ($s3); 119 insns against target's 116, 53 objdump diff lines.
- verdict: KILLED

## [s5] Expressing the table walk as an INDEX rather than a pointer (`s32 i = 5; ... D_800A3610[i]; i++; if (D_800A3610[i + 1] != -1)`) changes the pseudo partition in a way no permuter mutation can invent, and may give the walk allocno the reference count the flip needs.
- mechanism: the permuter mutates spellings (&x[i] vs *(x+i), temp introduction, statement order) but never changes an induction variable's TYPE; an index form makes the array base a separate long-lived pseudo and the index a short-lived one, a genuinely different partition of the same dataflow.
- probe: tmp/grind/func_80078654/s5/eval.sh on the standalone chassis, banked as rejected/index-walk-strength-reduce-plus4-insns.c.
- result: KILLED at the first measurement. loop.c's strength reduction re-creates a pointer induction variable but keeps the array base live alongside it, so the function grows to 120 insns against target's 116 (50 diff lines), and the walk allocno gets WORSE rather than better: pseudo 73 nrefs 5 -> 4, livelen 91 -> 94, pri 1098 -> 851, while arg0 stays pinned at 13 references (pri 3861).
- verdict: KILLED

## [s5] The ledger's named rederive probe ("run m2c on asm/funcs/func_80078654.s and write the reconstruction VERBATIM from its output") is executable in this tree.
- mechanism: m2c reconstructs C from MIPS asm and its output is evidence about the original pseudo partition, which is the one seed no permuter chassis has had.
- probe: `python3 -m m2c.main --target mipsel-gcc-c asm/funcs/func_80078654.s` under the repo .venv, plus a filesystem search for an m2c checkout.
- result: KILLED AS WRITTEN — m2c is NOT installed. The module is absent from .venv (ModuleNotFoundError: No module named 'm2c'), there is no ~/m2c checkout, and only historical artefacts remain (tmp/blitz/m2c_*.c, include/m2c_context.h). A future rederive session must FIRST install decompals/m2c or hand-derive the reconstruction from the target asm; planning that modality around an m2c invocation will lose a session to this discovery otherwise.
- verdict: KILLED

## [s5] Directed permutation — hand-written PERM_* macros enumerating a chosen cross-product of genuine alternatives rather than uniformly sampling spellings — reaches the $s0/$s1 flip where s4's two random campaigns could not. (Ledger frontier item 3, recorded there as 'measured-thin rather than untried'.)
- mechanism: PERM_GENERAL / PERM_LINESWAP expand to EVERY listed alternative, so a small set of semantically-equivalent partition choices is covered exhaustively instead of sampled; with PERM_RANDOMIZE regions layered on top the regime is materially different from s4's uniform random sampling.
- probe: Built tmp/grind/func_80078654/s5/ws (settings.toml/compile.sh/target.o copied from the validated s4 rig so the function sits at offset 0 and the score is the real weighted diff) with a base.c carrying six directed axes: three walk-initialiser partitions; three block-A shapes including the `s32 *ot = &arg0[5]` arg0-lowering sub-pointer; four loop-head read shapes including the double-read `s.b = var_s0[0] + 0xC;` ref-lift candidate; a PERM_LINESWAP over the `var_s0++` / `arg0[5] += 0xC` pair; four loop-test spellings; two PERM_RANDOMIZE regions. Launched via tools/permuter_campaign.py (label s5-directed-perm, -j 8, --stop-on-zero, --stack-diffs), waited IN-TURN with `permuter_campaign.py wait`, harvested with --stop.
- result: Base score 108, identical to the s4 chassis-A base (confirming the default expansion is the floor-19 form). 39,950 iterations in 1152 s, ZERO score-improving finds; equal-score (108) finds at 147 s and 584 s, then a full 9-minute window with no novel find at all, at which point the fresh-seed rule was honoured and the campaign stopped (procs_killed 9, alive=false at status). Both finds reproduce s4's failure mode exactly and are unusable for the same reason: each clobbers the walk pointer inside the loop (`var_s0 = &arg0[3];` in output-108-1, `var_s0 = &arg0[5];` in output-108-2), i.e. they are semantics-CHANGING, not respellings. Two campaigns under two different mutation regimes converging on the same semantics-breaking equal-score attractor means that attractor absorbs any future campaign on this chassis family.
- verdict: KILLED

## [s5] The arg0-LOWERING quadrant of the s3 2-D frontier is reachable by hoisting a sub-pointer: `s32 *ot = arg0 + 5;` at function scope with every `arg0[5]` rewritten as `*ot` strips four references per block off the parameter pseudo and should drop its allocno priority below the walk pointer's. (The last unmeasured quadrant — s2 measured only a block-SCOPED split, s4 seeded only the walk-RAISING chassis.)
- mechanism: allocno_compare sorts by floor_log2(n_refs)*n_refs/live_length; arg0's 13 references are one def plus twelve emitted accesses, eight of them `0x14($sN)`. Routing those eight through a separate pointer pseudo should leave arg0 near 5 references and below the walk pointer's 1098, letting the walk pointer take $s0 under find_reg's ascending first-fit.
- probe: tmp/grind/func_80078654/s5/eval.sh on the standalone chassis — instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1) allocno table plus a full-pipeline objdump diff against the s4 rig's target.o. Banked as memory/grind/func_80078654/rejected/ot-subpointer-arg0-lowering-4th-callee-save.c.
- result: KILLED, and it yields the sharpest bound this ledger has. arg0's references DO drop 13 -> 6 and its priority 3979 -> 1212, but the walk pointer is STILL BELOW at 1086 (nrefs 5, livelen 92): the comparison is 2*6/99 against 2*5/92 and floor_log2(6) == floor_log2(5) == 2. A seven-reference reduction of the parameter is therefore not enough; on the measured live lengths arg0 must reach <= 5 references (2*5/99 = 1010 < 1086), i.e. ELEVEN of its twelve emitted accesses must leave the pseudo. Worse, the extracted `ot` pseudo behaves exactly as s2/s3 predicted for every split — shorter-lived (85), more references (9), priority 3176 — so it takes $s0 itself and forces a FOURTH callee-save ($s3); 119 insns against target's 116, 53 objdump diff lines.
- verdict: KILLED

## [s5] Expressing the table walk as an INDEX rather than a pointer (`s32 i = 5; ... D_800A3610[i]; i++; if (D_800A3610[i + 1] != -1)`) changes the pseudo partition in a way no permuter mutation can invent, and may give the walk allocno the reference count the flip needs.
- mechanism: The permuter mutates spellings (&x[i] vs *(x+i), temp introduction, statement order) but never changes an induction variable's TYPE; an index form makes the array base a separate long-lived pseudo and the index a short-lived one — a genuinely different partition of the same dataflow.
- probe: tmp/grind/func_80078654/s5/eval.sh on the standalone chassis. Banked as memory/grind/func_80078654/rejected/index-walk-strength-reduce-plus4-insns.c.
- result: KILLED at the first measurement. loop.c's strength reduction re-creates a pointer induction variable but keeps the array base live alongside it, so the function grows to 120 insns against target's 116 (50 diff lines), and the walk allocno gets WORSE rather than better: pseudo 73 nrefs 5 -> 4, livelen 91 -> 94, pri 1098 -> 851, while arg0 stays pinned at 13 references (pri 3861).
- verdict: KILLED

## [s5] The ledger's named rederive probe ('run m2c on asm/funcs/func_80078654.s and write the reconstruction VERBATIM from its output') is executable in this tree.
- mechanism: m2c reconstructs C from MIPS asm and its output is evidence about the original pseudo partition — the one seed no permuter chassis has had.
- probe: `python3 -m m2c.main --target mipsel-gcc-c asm/funcs/func_80078654.s` under the repo .venv, plus a filesystem search for an m2c checkout.
- result: KILLED AS WRITTEN — m2c is NOT installed. The module is absent from .venv (ModuleNotFoundError: No module named 'm2c'), there is no ~/m2c checkout, and only historical artefacts remain (tmp/blitz/m2c_*.c, include/m2c_context.h). A future rederive session must FIRST install decompals/m2c or hand-derive from the target asm; planning that modality around an m2c invocation would otherwise lose a session to this discovery.
- verdict: KILLED
