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

### F4 — NEW (session 2): the walk pointer needs ~9 byte-neutral references
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
