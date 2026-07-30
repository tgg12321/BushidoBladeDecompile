# Hypothesis ledger — func_800477E8

Floor history: s1 baseline **17** (unchanged at end of s1; every s1 probe was
neutral or worse, all reverted, HEAD form restored).

---

## KILLED in s1

### K1 — "The tie is decided by the order the two tied variables are first assigned."
**Mechanism claimed.** GCC 2.7.2 numbers pseudos in order of first use, and
allocnos with equal priority break ties by allocno number, so emitting the
initialisation of the variable that should win `$v1` / `$t1` *first* would flip
the seating.
**Probe.** Swapped `a1 = 0; v1 = 1;` to `v1 = 1; a1 = 0;` at the top of loop1
(and separately hoisted `t2 = 0x2C00;` above the `gpu_CalcClut` call).
**Result.** The a1/v1 statement swap is **completely inert for allocation**:
`sandbox --disable all` stayed at **17**, and the re-diff showed the two
instructions had simply traded emission slots (idx 32/33 now read
`li a1,1` / `move v1,zero`) with the *same* hard registers as before. The t2
hoist was worse: **17 -> 26** with `build_insns 172` (+2), because moving the
constant above the call makes `t2` live across `gpu_CalcClut` and forces a
callee-save/restore pair.
**Verdict.** KILLED. Source statement order between the tied pair does not
touch the allocation tie; it only reorders emission. Do not re-probe.

### K2 — "`v1` is shared across loop1 and loop2 only as an m2c artifact; splitting it into two short-lived locals raises its priority and seats it in `$v1`."
**Mechanism claimed.** global.c priority is ~ `n_refs / live_length`; `v1`'s
range spans nearly the whole function, so it is allocated late and loses `$v1`
to `a1`/`ptr`. Splitting shortens both ranges and should lift priority.
**Probe.** Introduced `s32 v1b;`, used it for the whole loop2 sequence
(`v1b = a2;` / `*p = v1b;`), leaving `v1` to loop1 only.
**Result.** **17 -> 31** (insn count still 170) — nearly double the residual.
**Verdict.** KILLED, and it is *positive* evidence in the other direction: the
original C really did reuse ONE variable across both loops, exactly as HEAD
has it. Any future restructure must PRESERVE that sharing. Do not split `v1`.

### K3 — "Tie A is driven by how the `0x2C00` constant is spelled; materialising it at its use sites instead of in a named local removes the competing allocno."
**Mechanism claimed.** Dropping the `t2` local removes one contestant for `$t1`,
leaving `t1val` to take it.
**Probe.** Deleted the `s32 t2;` declaration and the `t2 = 0x2C00;` statement,
wrote the literal `0x2C00` at both `*s0 = t2;` sites.
**Result.** **17 -> 18**. GCC immediately re-CSE'd the two literals back into a
single pseudo (target and build both still emit exactly one `li ...,11264`), so
the contestant did not go away — the seating of tie A was byte-for-byte
unchanged — and the re-materialised constant scheduled one slot later than
target (`move a0,zero` / `li t1,11264` swapped), adding one diff.
**Verdict.** KILLED. Constant-spelling is not the lever for tie A; GCC's CSE
reconstructs the same allocno regardless of how the constant is written.

---

## KILLED / RESOLVED in s2 (structural)

### K4 — "Declaration order breaks the allocation tie (allocnos are numbered in declaration order and global.c breaks equal priorities by allocno number)."
**Probe.** `decl-v1-first`, `decl-t1val-last`, `a1 declared before a2`, and
`val declared first` among the new loop2 locals — screened on the cc1 `-da`
`.greg` allocation dump.
**Result.** Byte-identical priority order AND dispositions in every case.
**Verdict.** KILLED, strong form. global.c only reaches the allocno-number
tiebreak when two priorities are EXACTLY equal, which no contested pair here is.

### K5 — "loop3's pointer being the same C variable as loop2's row pointer is an m2c artifact; splitting it (or making loop3 an indexed loop) changes the seating."
**Probe.** `ptr-split-loop3`, `loop3-indexed`, `a0-split-loop3`,
`loop2-base-index` (`&ptr[0x10]` / `&ptr[0x11]` instead of pointer arithmetic).
**Result.** Inert or worse; the pointer-split and the indexed rewrite produce an
identical dump to base. `a0` split off loop3 rotates the seating away from
target.
**Verdict.** KILLED. Do not touch loop3.

### K6 — s1's K2 REVERSED: "`v1` must remain ONE variable shared across loop1 and loop2."
**Probe.** Re-measured the split through the allocation dump instead of the
score, then combined it with splitting loop2's accumulator and row counter.
**Result.** Splitting `v1` is what puts it in `$v1` — while it is shared it
CONFLICTS with loop2's walking pointer `ptr`, and `ptr`'s allocno priority
(tight-range induction pseudo, ~1.0-1.6) is unbeatable by a loop-spanning value
pseudo (~0.26), so `ptr` takes `$v1` first every time. With `w`/`r`/`val` split
out and `t2 = 0x2C00; a3 = 0; a0 = 0;` ordering, loop1's seating equals target
EXACTLY.
**Verdict.** s1's K2 constraint is WITHDRAWN. The floor is still 17, but the
residual moved wholesale from loop1 to loop2.

### K7 — "Finish the job: split the remaining shared locals (`a0`, `ptr`) too."
**Probe.** `mid+loop2-own-counter`, `mid+loop3-own-ptr+own-counter`,
`split-loop2-all`, `split-everything`.
**Result.** 17 -> 40 and 17 -> 36. `a0` must stay ONE allocno across all three
loops — that shared reference count is exactly what earns it `$a0`, which is
where target has it in all three loops.
**Verdict.** KILLED. The correct partition is ASYMMETRIC: split loop2's
value/accumulator/row-counter, keep `a0` and `ptr` shared.

### K8 — "Tie A ($t1/$t2) is reachable by a structural lever."
**Probe.** Every structural variant this session (7 rounds, ~30 forms) was
checked for the t1val/t2 disposition as well; none moved it. Also confirmed from
`asm/funcs/func_800477E8.s` that target's two `gpu_CalcClut(0x10,0x1E0)` results
really are two distinct call results (`addu $s1,$v0,$zero` sits in the 4th call's
DELAY slot, so it captures the 3rd call's value), so the arms cannot legitimately
share one value.
**Verdict.** KILLED for the structural modality. t1val has 2 refs, t2 has 3, with
identical live ranges; nothing short of changing those counts flips it, and both
arms genuinely store 0x2C00. Worth 5 of the 17.

---

## Live frontier (for s3+)

### F4 — Tie B's REMAINING half: inside loop2, the stored value must out-prioritise the walking pointer.
**State.** With the s2 candidate applied, loop1 is exact and the only register
errors are: `val`->$a3(7) but target wants $v1(3); `ptr`->$v1(3) but target
wants $a1(5); `w`->$a1(5) wants $a2(6); `r`->$a2(6) wants $a3(7). `a0`->$a0(4)
and `p`->$v0(2) are already correct.
**Mechanism.** Required allocation order inside loop2 is `p, val, a0, ptr, w, r`
(each then takes the lowest free register: 2, 3, 4, 5, 6, 7). Actual order puts
`ptr` fourth OVERALL in the whole function and `val` second-to-last: `val` has
only 2 references (`val = w;` and `*p = val;`) against `ptr`'s 4-8, and both are
tight-ranged, so `floor_log2(2)*2/len` loses badly to `floor_log2(4)*4/len`.
This is the SAME inversion loop1 had, and loop1's fix (remove the conflict by
splitting) is not available here because `val` and `ptr` are genuinely both live
across the inner loop.
**Next probe.** Attack `ptr`'s live_length rather than `val`'s ref count: `ptr`
is currently defined immediately before loop2, so its range is ~8 insns. Try
forms that legitimately lengthen it (e.g. deriving loop2's row base from a
pointer that is also the natural cursor for the `D_800EF0xx` init block above,
if the semantics support it), and forms that give `val` a genuine third
reference from the surrounding code. Screen everything on the .greg dump first
(`VSET=variantsN python3 sweep.py`) — the score alone hid the loop1 solution for
a whole session.

### F5 — Tie A needs a reference-count change that structure cannot supply; it is the natural target of a `forensics` or `rederive` session.
**Mechanism.** t1val 2 refs vs t2 3 refs, identical live ranges ⇒ t2 wins $t1.
If the counts were EQUAL the tie would break on allocno number and t1val (76)
beats t2 (83), which is the seating target wants — so the question is what
original C gives the 0x2C00 constant only two references, or the second CLUT
value three. Note the arms' payloads: the `a3>=5` arm stores
{s3val, t2, s1val, -0xC1, -0x100, -0x3FC1, -0x4000} and the `else` arm stores
{s2val, t2, t1val, -0x40C1, -0x4100, -0x7FC1, -0x8000} — the SECOND element is
`t2` in both, and the negative constants differ by exactly 0x4000 between arms.
A rederive that expresses the arms as a table/base-plus-offset rather than two
literal store sequences would change these reference counts wholesale.
**Next probe.** In a rederive session, re-express the two arms from the
primitive layout (tpage, clut, uv pairs) rather than as duplicated store runs,
and re-read the .greg dump for the t1val/t2 dispositions.

### F6 — Make the candidate's statement ordering non-arbitrary before it ever reaches the Judge.
**Mechanism.** The s2 candidate depends on `t2 = 0x2C00; a3 = 0; a0 = 0;` in
that exact order (other orders cost 8-9 points) purely through
allocno_live_length. That is ordinary statement order, not a coercion, but it is
unmotivated as written.
**Next probe.** Find a natural loop shape that yields the same live range for the
row counter — but note a full nested `for()` rewrite of loop1 was measured to
create two EXTRA induction pseudos (t2 -> $t4, t1val -> $t3), so re-read the dump
after any loop-shape change.

## Superseded frontier (s1's F1/F2/F3)

F1 (tie A by reference count) — measured this session, see K8; the "check whether
target's two gpu_CalcClut calls share a value" sub-question is answered NO.
F2 (lower a1's and ptr's priority) — half-solved: a1 is now correct; the ptr half
survives as F4.
F3 (rederive) — still live, and now much better targeted: only loop2 and tie A
remain, and evidence.md records the exact seating each must reach.

### F1 — Tie A closes by making `t1val` out-prioritise `t2` on reference count, not on ordering.
**Mechanism.** global.c `allocno_compare` ranks by roughly
`log2(n_refs) * n_refs * size / live_length`. `t2` has 3 refs (def + one use in
each arm) and `t1val` has 2 (def + one use in the `else` arm only), with
essentially identical live ranges, so `t2` is allocated first and takes `$t1`
(first in `REG_ALLOC_ORDER` of the two). Flipping requires changing the ref
counts or the live length, NOT the statement order (K1) or the constant's
spelling (K3).
**Next probe.** Sink `t2`'s definition to the top of the `do` loop body
(`t2 = 0x2C00;` as the first loop statement alongside `t0`/`a2`) so its live
length shrinks to one iteration while its ref count stays 3 — measure whether
the priority ratio flips in `t1val`'s favour or against it. Then the mirror
probe: give `t1val` a genuine third reference by using it in BOTH arms (the
`a3>=5` arm currently stores `s1val`, and `s1val`/`t1val` are the results of two
*byte-identical* `gpu_CalcClut(0x10,0x1E0)` calls — check `asm/funcs` whether
target's two calls really do take identical arguments; if so the arms may
legitimately be sharing one value and the second call belongs elsewhere).
Read `.claude/rules/register-alloc-pure-c.md` and take a `cc1 -da` `.greg` dump
into `tmp/grind/func_800477E8/s2/` to read `;; Register dispositions:` and the
conflict list directly instead of inferring priorities.

### F2 — Tie B closes by LOWERING `a1`'s and `ptr`'s priority, since `v1`'s long shared range is required (K2).
**Mechanism.** `v1` must stay one allocno spanning both loops, so its
`n_refs/live_length` cannot be raised much. The seating flips if its two
disjoint competitors — `a1` (loop1) and `ptr` (loop2) — each fall below it.
Both are currently loop-carried induction pseudos with tight ranges and high
ratios.
**Next probe.** For loop2, fold `ptr` away as a separate induction variable:
target's idx 139/148 (`addiu v0,ptr,64` then `ptr += 0x11`) is the classic
walking-pointer shape, so try driving the inner loop off `p` alone with the row
base recomputed from an index, or conversely walk `ptr` with post-increment per
[[walking-pointer-serializes-parallel-loads]]. For loop1, `a1` advances by
0x100 while `v1` advances by 1 and they are only ever consumed as `v1 | a1`;
probe deriving `a1` from the existing `a0` inner counter (`a0 << 8`) so it stops
being a loop-carried allocno at all — but note target DOES emit
`addiu a1,a1,256`, so `a1` must remain loop-carried; the probe is about its
reference profile, not its existence.

### F3 — The real answer is a rederive: the body is raw m2c output and the whole variable partitioning is artificial.
**Mechanism.** Not a codegen mechanism but a search-space argument, and it is
the strongest signal in the function: our build already reproduces all 170
instructions in target's exact order, so the C is *structurally* correct and
only the variable-to-allocno mapping is wrong. That mapping is precisely what a
verbatim m2c transcription gets arbitrarily — m2c invents one local per hard
register it observed, with `goto` control flow and register-derived names. The
original was a UV/tpage primitive-fill loop (8 rows x 16 columns x 11 halfwords
into `D_800A33D0`, packed UVs advancing 0x0101 per column) whose natural C would
have real loops and a different, smaller set of variables.
**Next probe.** In a `rederive` modality session, rewrite lines 674-816 from the
semantics up: real `for` loops over row/column, named `u`/`v`/`tpage`/`clut`
locals, the `D_800EF59C` / `D_800EF558` fills as ordinary indexed loops. Hold
two constraints from s1 fixed: (a) the loop1 `v1`-equivalent must be the SAME C
variable as the loop2 one (K2), and (b) the instruction sequence is already
correct, so any rewrite must be checked to keep `build_insns == 170` before its
register seating is judged. Also decide the fate of the inherited un-annotated
`do { v0 = v1 | a1; } while (0);` at src/sound.c:746 (see evidence.md).

## [s1] The $v1/$a1 and $t1/$t2 ties are decided by the source order in which the two tied variables are first assigned, so swapping the initialisers flips the seating.
- mechanism: GCC 2.7.2 numbers pseudos in order of first use and global.c breaks equal-priority allocno ties by allocno number; MIPS REG_ALLOC_ORDER reaches $v1 before $a1 and $t1 before $t2, so the earlier-numbered allocno should win the lower register.
- probe: Swapped 'a1 = 0; v1 = 1;' to 'v1 = 1; a1 = 0;' at the top of loop1, and separately hoisted 't2 = 0x2C00;' above the second gpu_CalcClut call. Measured each with sandbox --disable all and re-ran the instruction diff.
- result: a1/v1 swap: 17 -> 17, insn count unchanged; the re-diff showed idx 32/33 had merely traded emission slots ('li a1,1' / 'move v1,zero') with the SAME hard registers. t2 hoist: 17 -> 26 with build_insns 172 (+2) because t2 becomes live across the call and forces a callee-save/restore pair.
- verdict: KILLED

## [s1] The local 'v1' being shared across loop1 and loop2 is an m2c transcription artifact; its whole-function live range starves its allocno priority, so splitting it into two short-lived locals will seat it in $v1.
- mechanism: global.c allocno_compare ranks roughly by log2(n_refs)*n_refs*size/live_length, so a pseudo spanning nearly the whole function is allocated late and loses the first-in-REG_ALLOC_ORDER register to tighter-ranged competitors (a1 in loop1, ptr in loop2).
- probe: Added 's32 v1b;' and used it for the entire loop2 sequence ('v1b = a2;' / '*p = v1b;'), leaving 'v1' to loop1 only. Measured sandbox --disable all.
- result: 17 -> 31 (build_insns still 170) — nearly double the residual.
- verdict: KILLED

## [s1] Tie A ($t1/$t2) closes by removing the competing 't2' allocno: delete the local and write the 0x2C00 literal at both store sites so only 't1val' contests $t1.
- mechanism: With one fewer allocno in the conflict set, the survivor takes the first available register in REG_ALLOC_ORDER.
- probe: Deleted the 's32 t2;' declaration and the 't2 = 0x2C00;' statement, wrote the literal 0x2C00 at both '*s0 = t2;' sites. Measured sandbox --disable all and re-ran the instruction diff.
- result: 17 -> 18 (build_insns still 170). GCC re-CSE'd the two literals into a single pseudo — both build and target still emit exactly one 'li ...,11264' — so the contestant did not disappear and tie A's seating was byte-for-byte unchanged; the only effect was the constant scheduling one slot later than target ('move a0,zero' and 'li t1,11264' traded places), adding one diff.
- verdict: KILLED

## [s2] The allocation can be read and predicted directly instead of inferred: a cc1 -da dump of the real src/sound.c exposes the allocno priority order, the full conflict graph and the pseudo->hardreg map for func_800477E8.
- mechanism: GCC 2.7.2 global.c dumps ';; N regs to allocate:' (allocnos in allocation-priority order), ';; NN conflicts:' and ';; Register dispositions:' into the .greg dump. MIPS gcc-2.7.2 defines no REG_ALLOC_ORDER, so find_reg gives each allocno, in that order, the lowest-numbered hard reg not held by a conflicting allocno - which makes the whole seating a deterministic simulation over the printed data.
- probe: tmp/grind/func_800477E8/s2/greg.sh runs the exact Makefile cpp+cc1 flags with -da over src/sound.c. (src/sound.c has a pre-existing arity error in the UNRELATED func_800470B0, so cc1 exits non-zero; the dumps are still written in full.) Baseline: ';; 15 regs to allocate: 84 72 86 85 78 82 79 77 80 81 83 76 75 74 73' with dispositions 76->10 77->7 78->4 79->6 80->3 81->8 82->5 83->9 84->2 85->3 86->2.
- result: Baseline priority order = v0, s0, p, ptr, a0, v1, a2, a3, a1, t0, t2, t1val. Simulating find_reg against the printed conflict sets until the assignment equals target's gives the REQUIRED order: v0, s0, p, v1, a0, a1, a2, a3, t0, t1val, t2 (ptr anywhere after a0). Three inversions to fix: v1 above ptr and above a0; a1 above a2/a3; t1val above t2.
- verdict: CONFIRMED

## [s2] s1's K2 is wrong: loop1's 'v1' must be SPLIT from loop2's stored value, not shared, because while shared it conflicts with loop2's walking pointer and can never win $v1.
- mechanism: allocno_compare ranks by floor_log2(n_refs)*n_refs*size/live_length. 'ptr' is a tight-range induction pseudo (~1.0-1.6); a value pseudo spanning both loops is ~0.26. ptr is therefore allocated 4th in the whole function and takes $v1 before v1 is ever considered. The conflict is what blocks v1; removing the conflict (splitting) is the only lever, since v1's priority cannot be raised by a factor of ~6.
- probe: Re-measured K2's split through the .greg dump instead of the score, then combined it with splitting loop2's accumulator and row counter into their own locals ('w','r','val') and ordering the pre-loop inits as 't2 = 0x2C00; a3 = 0; a0 = 0;'. Screened with sweep.py (dump-only, no src/ edits), scored with score.py (apply -> sandbox --disable all -> restore).
- result: Split alone: v1->$v1(3) CORRECT but a1/a2/a3 rotate (score 29). Split + loop2 accumulator split: v1->$v1 AND a1->$a1 both correct (score 25). Split + full loop2 split + init ordering: loop1 seating equals target EXACTLY for a0/a1/a2/a3/t0/v1/v0/s0/s1val/s2val/s3val, score 17 with build_insns 170. s1's 'must preserve sharing' constraint is WITHDRAWN.
- verdict: CONFIRMED

## [s2] Declaration order breaks the allocation tie (allocnos are numbered in declaration order and global.c breaks equal priorities by allocno number).
- mechanism: global.c allocno_compare returns 'v1 - v2' (allocno number, lower first) only when the two computed priorities are exactly equal.
- probe: decl-v1-first, decl-t1val-last, a1-declared-before-a2, and val-declared-first among the new loop2 locals - all screened on the .greg dump, before and after the splits.
- result: Byte-identical priority order AND byte-identical dispositions in every case. No contested pair in this function has exactly equal priority, so renumbering never reaches the tiebreak.
- verdict: KILLED

## [s2] loop3's pointer being the same C variable as loop2's row pointer is an m2c artifact, and splitting it (or rewriting loop3 as an indexed loop) will move the seating.
- mechanism: Halving 'ptr's reference count should drop its allocno priority below the value pseudo's.
- probe: ptr-split-loop3 (loop3 walks its own 's32 *q'), loop3-indexed (for (a0=0;a0<0x11;a0++) D_800EF558[a0] = (a0<<7)&0xFFF;), loop2-base-index (&ptr[0x10] / &ptr[0x11] instead of pointer arithmetic), a0-init-inside-outer.
- result: All produce a dump identical to base (same priority order, same dispositions) and floor stays 17. Peeling loop3's pointer off does not lower ptr below val even after the loop2 splits.
- verdict: KILLED

## [s2] Once loop2's value/accumulator/row-counter are split, splitting the remaining shared locals ('a0' and 'ptr') finishes the job.
- mechanism: Symmetry with the v1 split: shorter live ranges should keep lifting priorities toward target.
- probe: mid+loop2-own-counter (loop2 gets its own 's32 n'), mid+loop3-own-ptr+own-counter, split-loop2-all, split-everything - dump-screened and scored.
- result: 17 -> 40 and 17 -> 36. 'a0' must stay ONE allocno shared by loop1's column counter, loop2's inner counter and loop3's index: that shared reference count is exactly what earns it $a0, which is where target puts it in all three loops. A fresh tight-range counter pseudo instead grabs $v1(3) and displaces the whole loop2 seating.
- verdict: KILLED

## [s2] Tie A ($t1/$t2) is reachable by some structural lever, possibly because target's two gpu_CalcClut(0x10,0x1E0) calls share one value.
- mechanism: t1val (2 refs: def + one store in the else arm) loses $t1 to t2 (3 refs: def + one store in EACH arm) because 1*3/L > 1*2/L at identical live ranges. Equal counts would tie and break on allocno number, where t1val (76) beats t2 (83) - which is target's seating.
- probe: Checked the t1val/t2 disposition in every one of ~30 structural variants across 7 rounds; separately read asm/funcs/func_800477E8.s to settle whether the two CalcClut results are one value.
- result: No structural variant moved tie A. Target does 'jal gpu_CalcClut / addu $s1,$v0,$zero' with the addu in the DELAY slot, so $s1 captures the THIRD call's result and $t1 the FOURTH - two genuinely distinct call results, so the arms cannot legitimately share one. Both arms genuinely store 0x2C00, so t2's third reference cannot be removed honestly either. Tie A is worth 5 of the 17.
- verdict: KILLED

---

## KILLED / RESOLVED in s3 (structural) — floor 17 -> 5

### K9 — CONFIRMED, and it is the session's whole result: "loop2's allocation is unreachable because the m2c goto-loop shape denies its pseudos reference weight; writing loop2 as real loop constructs unlocks it."
**Mechanism.** flow.c weights each register reference by `loop_depth`, which comes
from `NOTE_INSN_LOOP_BEG` notes that the front end emits ONLY for real loop
constructs. m2c's `goto outer2:` / `goto inner2:` loops emit none, so the loop2
pseudos were scored unweighted: stored value 2 refs / 7 insns = 0.29 against the
walking pointer's 8 / 21 = 1.14. No split, declaration order or statement order
can beat a factor of 4 — which is exactly why s1 and s2 both stalled at 17.
**Probe.** Rewrote loop2 as `do { ... for (a0 = 0x10; a0 >= 0; a0--) ... } while
(...)`, screened on the .greg/.lreg dumps with the closed-form priority formula,
then scored for real. Then followed the arithmetic to its conclusion over rounds
8-15 (~40 variants).
**Result.** The same references become 5-7 weighted and the order reverses.
Floor 17 -> 14 -> 11 -> 6 -> 5, with build_insns held at 170 the whole way.
**Verdict.** CONFIRMED. The accepted form is in candidate.c and its loop1, loop2
and loop3 dispositions ALL equal target; the residual 5 is exactly tie A.

### K10 — s2's K6/K7 partition CORRECTED: "loop2's row counter must be its own local."
**Result.** The opposite, once loop2 is a real loop: it must be loop1's `a3`.
Sharing gives 18 refs / 86 insns = 0.84, which is low enough that it is allocated
LAST of the loop2 group and therefore takes $a3, as target has it. A fresh local
scores 1.17, is allocated third, takes $a0's register and rotates the entire group
(score 14 vs 5). s2's split of the STORED VALUE stands; s2's split of the COUNTER
is withdrawn. Final partition: share `a0` and `a3` with loop1, keep `w` and `val`
separate, and let the row pointer and the inner walking pointer be compiler-made.
**Verdict.** KILLED (the fresh-counter form), see
rejected/loop2-fresh-row-counter-rotates-seating-14-not-5.c.

### K11 — "any real-loop spelling of loop2 will do."
**Mechanism/result.** Two extra constraints, both measured:
(a) the counter must be READ in the body, else cc1's `check_dbra_loop` reverses
the loop into a countdown and target's `slti $v0,$a3,0x9` vanishes (build_insns
169, score 11 with otherwise perfect dispositions);
(b) the row pointer must NOT be a source variable. loop.c inserts iv inits into
the preheader, i.e. after all source statements, so a source-level pointer is
always initialised BEFORE a compiler-made accumulator — while target's preheader
is `a3 = 0; w = 0; lui/addiu $a1`, pointer LAST. Indexing the row
(`p = &D_800EF59C[a3 * 0x11]; p[a0] = val;`) satisfies both at once: the `a3 * 0x11`
is the in-body read, and it makes loop.c strength-reduce the row address into $a1
and the inner index into the $v0 walking pointer.
**Verdict.** KILLED for the naive spellings; the surviving form is the candidate.
See rejected/loop2-counter-unused-in-body-cc1-reverses-loop-169-insns.c.

### K12 — s2's "fragile arbitrary statement order" caveat RETIRED.
**Result.** With `a3` shared into loop2 the best pre-loop order is
`a3 = 0; t2 = 0x2C00; a0 = 0;` — which is TARGET's own preheader order and also
HEAD's. Worth exactly one instruction (score 6 -> 5). s2's `t2; a3; a0;` ordering
was an artifact of s2's fresh row-counter local, so there is no unmotivated
ordering left in the candidate for the Judge to object to.
**Verdict.** RESOLVED.

### K13 — s2's K8 ("tie A is unreachable by any structural lever") OVERTURNED ON MECHANISM, still open on cost.
**Mechanism.** t1val 3 weighted refs / 76 insns = 0.0395; t2 5 / 150 = 0.0667, so
t2 is allocated first and takes the lower $t1. t2's live_length is DOUBLE t1val's
because t2 is used in both arms and stays live on both paths while t1val is dead
along the `a3 >= 5` path.
**Probe.** Lifted the shared `*s0 = t2;` store out of the two arms so t2 has ONE
in-loop reference (3 refs / 150 = 0.020).
**Result.** t1val -> $t1(9) and t2 -> $t2(10) — EXACTLY target — with every other
disposition still correct. But factoring needs a second test of `a3 >= 5` and cc1
emits 171 instructions against target's 170 (score 10-13); caching the condition
in a local does not avoid it.
**Verdict.** Mechanism CONFIRMED, form REJECTED on cost. See
rejected/tieA-factor-t2-store-solves-seating-but-costs-one-insn.c for the full
arithmetic and the six spellings already measured dead.

### K14 — "raise loop1's reference weighting by making its inner goto a real loop; the model says that flips tie A."
**Mechanism.** Depth-3 weighting would give t1val 2*4/76 = 0.105 against t2's
2*7/150 = 0.093 — t1val wins. The model's prediction is correct as far as it goes.
**Result.** Dead anyway: cc1 manufactures TWO extra induction pseudos for the
recognised inner loop and they take $t1 and $t2, pushing t1val to $t3(11) and t2
to $t4(12). Score 10 (do-while) / 13 (for), insns 170 — strictly worse than the
goto shape's 5. Same failure mode s2 recorded for a full nested-for rewrite of
loop1, now with two more data points: it is the inner loop's mere recognition that
creates the extra ivs, not the amount of rewriting.
**Verdict.** KILLED. The loop shapes are ASYMMETRIC and must stay that way: loop2
real, loop1's inner goto. See
rejected/loop1-inner-real-loop-adds-two-pseudos-10-not-5.c.

---

## Live frontier (for s4+)

### F7 — Tie A closes with a spelling of "t2 is read once per iteration" that costs no instruction.
**State.** This is the ONLY remaining defect: floor 5, and all five diffs are tie
A. The mechanism is proven (K13) — one in-loop reference to t2 seats both
registers exactly as target — so the search is now narrow and fully specified: any
form in which the 0x2C00 store happens twice in the emitted code but reads t2's
pseudo once, at 170 instructions.
**Mechanism.** priority = floor_log2(n_refs)*n_refs/live_length; t2 at 5/150 beats
t1val at 3/76, t2 at 3/150 loses to it, and an exact tie would also work because
allocno numbering favours t1val (76 < 83). Screen on the .lreg numbers with
tmp/grind/func_800477E8/s3/sweep2.py before scoring.
**Next probe.** Already dead: two-ifs factoring (171), condition cached in a local
(171), literal in both arms (CSE rebuilds one pseudo, 170 but unchanged seating),
literal in one arm only (171, score 3 — the best score seen, and the closest miss),
v0 copy (171), `s0[1] = t2;` hoisted with `s0 += 2` (167). Untried directions: a
form where the two arms differ in which VARIABLE holds 0x2C00 while both variables
come from one def (watch for the alias-rename cheat family — a second C handle for
the same value under a different name is NOT allowed); expressing the arms so the
duplicated stores come from cross-jumping a single source sequence; and attacking
t1val's live_length downward instead (3 refs needs length <= 45 against its
current 76, so anything that confines t1val to fewer basic blocks wins).

### F8 — Judge surface to clear before this ever reaches distance 0.
**State.** The body still carries HEAD's un-annotated `do { v0 = v1 | a1; } while
(0);`. It falls under .claude/rules/do-while-zero-exception but carries no
`/* FAKE */` annotation. Whoever reaches 0 must annotate or eliminate it. Also
worth a fresh look now that loop1's arms are the only goto-shaped part left: the
do-while(0) exists to keep the `a3 & 1` arms' two ORs in target's order, and a
different arm spelling might make it unnecessary.

### F9 — Generalise the s3 finding beyond this function.
**Mechanism.** The lesson that unlocked this function is not function-specific:
m2c goto-shaped loop bodies get NO loop_depth reference weighting, so their
allocation priorities are systematically wrong and no amount of variable splitting
inside them can fix it. Every remaining queue item transcribed verbatim from m2c
with `goto`-driven loops is a candidate for the same treatment.
**Next probe.** Worth raising with the owner as a project-level rule/memory rather
than spending this function's sessions on it.

## Superseded frontier (s2's F4/F5/F6)

F4 (loop2's stored value vs the walking pointer) — SOLVED, see K9/K10/K11.
F5 (tie A needs a reference-count change) — the diagnosis was exactly right and is
now proven; it survives as F7, narrowed to a cost problem rather than a mechanism
problem. Note F5's suggestion to re-express the arms from the primitive layout is
still the most promising untried direction.
F6 (make the statement ordering non-arbitrary) — RESOLVED, see K12.

## [s3] loop2's allocation is unreachable not because of the variable partition but because m2c's goto-shaped loops emit no NOTE_INSN_LOOP_BEG, so flow.c never weights loop2's references by loop_depth; writing loop2 as real loop constructs raises the same pseudos' priorities and puts the group in play.
- mechanism: GCC 2.7.2 global.c allocno_compare ranks by floor_log2(n_refs)*n_refs*size/live_length, and flow.c increments REG_N_REFS by loop_depth per reference. loop_depth comes from NOTE_INSN_LOOP_BEG, which the front end emits only for real loop constructs. With the goto shape loop2's stored value has 2 refs / 7 insns = 0.29 against the walking pointer's 8 / 21 = 1.14 -- a factor of 4 that no split, declaration order or statement order can close, which is exactly why s1 and s2 both stalled at 17. Computing that formula over the cc1 -da .lreg dump's 'Register NN used R times across L insns' numbers reproduces the .greg printed allocation order EXACTLY for every variant, so the seating became a closed-form calculation rather than a search.
- probe: Built tmp/grind/func_800477E8/s3/sweep2.py (prints allocation order + dispositions + per-pseudo priority + cc1 insn count for a whole variant list, never touching src/) and screened ~50 loop2 forms across rounds 8-19, scoring the survivors with s2's score.py (apply -> sandbox --disable all -> restore).
- result: Real loop constructs lift the loop2 pseudos from 0.29-1.14 to 1.0-1.8 and reverse the allocation order. Floor moved 17 -> 14 -> 11 -> 6 -> 5 with build_insns held at 170 throughout. The accepted form's loop1, loop2 AND loop3 dispositions all equal target on the .greg dump.
- verdict: CONFIRMED

## [s3] loop2's row counter must be its own local (s2's K6/K7 partition).
- mechanism: s2 reasoned that a shorter live range always lifts priority, so every loop2 quantity should be split out of loop1's locals.
- probe: Re-measured the counter as a fresh local vs shared with loop1's a3, on top of the real-loop loop2, on the .greg dump and then for score.
- result: The opposite holds once loop2 is a real loop. Shared with a3 the allocno scores 18 refs / 86 insns = 0.84, low enough to be allocated LAST of the loop2 group and therefore take $a3 as target has it. A fresh local scores 7/12 = 1.17, is allocated third, takes $a0's register and rotates the whole group: score 14 instead of 5. s2's split of the STORED VALUE stands; s2's split of the COUNTER is withdrawn.
- verdict: KILLED

## [s3] any real-loop spelling of loop2 will do once the weighting is unlocked.
- mechanism: If reference weighting is the only obstacle, the loop shape details should not matter.
- probe: Measured explicit-walking-pointer, ptr-advanced-early, indexed-row, flat-indexed and LICM-manufactured-value spellings, reading both the dispositions and build_insns.
- result: Two further constraints, both measured. (a) The counter must be READ inside the body: otherwise cc1's check_dbra_loop reverses the loop into a countdown and target's 'slti $v0,$a3,0x9' vanishes -- build_insns 169 against 170, score 11 even with otherwise perfect dispositions. (b) The row pointer must NOT be a source variable: loop.c inserts iv initialisations into the preheader, i.e. after all source statements, so a source-level pointer is always initialised BEFORE a compiler-made accumulator, whereas target's preheader is 'addu $a3,$zero,$zero; addu $a2,$zero,$zero; lui/addiu $a1' with the pointer LAST. Indexing the row (p = &D_800EF59C[a3 * 0x11]; p[a0] = val) satisfies both at once and is the only spelling that reaches 5.
- verdict: KILLED

## [s3] s2's caveat stands that the candidate depends on an arbitrary pre-loop statement order that a Judge would object to.
- mechanism: s2 measured 't2 = 0x2C00; a3 = 0; a0 = 0;' as worth 9 points over HEAD's order and flagged it as unmotivated.
- probe: Re-measured all three orderings on the new body, where a3 is shared into loop2.
- result: The best order is now 'a3 = 0; t2 = 0x2C00; a0 = 0;' -- which is TARGET's own preheader order and also HEAD's -- worth exactly one instruction (score 6 -> 5). s2's ordering was an artifact of s2's fresh row-counter local. There is no unmotivated statement ordering left in the candidate.
- verdict: KILLED

## [s3] tie A (t1val/t2 for $t1/$t2) is unreachable by any structural lever (s2's K8).
- mechanism: t1val has 3 weighted refs over a 76-insn live range (1*3/76 = 0.0395); t2 has 5 over 150 (2*5/150 = 0.0667), so t2 is allocated first and takes the lower $t1. t2's range is double t1val's because t2 is used in BOTH arms and stays live on both paths while t1val is dead along the a3>=5 path.
- probe: Lifted the shared '*s0 = t2;' store out of loop1's two arms so t2 has ONE in-loop reference (3 refs / 150 = 0.020), with and without caching the condition in a local; also measured literal-in-both-arms, literal-in-one-arm, v0-copy, hoisted-'s0[1] = t2', inverted branch sense and swapped CLUT results.
- result: Factoring produces t1val->$t1(9) and t2->$t2(10) -- EXACTLY target -- with every other disposition still correct, so the mechanism is proven and s2's K8 is overturned on mechanism. It is rejected only on COST: the second test of 'a3 >= 5' makes cc1 emit 171 instructions against target's 170 (score 10-13). Literal in one arm only scores 3, the best score seen anywhere, but also at 171. Literal in both arms leaves the seating unchanged at 170 (CSE rebuilds one pseudo with the same 2-use profile), confirming s1's K3 in the new body.
- verdict: CONFIRMED

## [s3] converting loop1's inner goto-loop into a real loop raises its reference weighting to depth 3 and flips tie A.
- mechanism: At depth 3 the model gives t1val 2*4/76 = 0.105 against t2's 2*7/150 = 0.093, so t1val would win $t1. The arithmetic is correct as far as it goes.
- probe: Converted the inner 'goto inner' to a do-while and separately to a for(), keeping every statement identical, and read the dispositions plus the score.
- result: Dead anyway: cc1 manufactures TWO extra induction pseudos for the recognised inner loop and they take $t1 and $t2, pushing t1val out to $t3(11) and t2 to $t4(12). Score 10 (do-while) / 13 (for) at 170 insns, strictly worse than the goto shape's 5. Same failure mode s2 recorded for a full nested-for rewrite of loop1. The loop shapes are ASYMMETRIC and must stay so: loop2 real, loop1's inner goto -- loop2's counters are consumed as an index so cc1 builds the ivs target has, loop1's are consumed as packed UV values so cc1 builds ivs target does not have.
- verdict: KILLED

## [s4] tie A can be closed by raising loop1's reference WEIGHTING rather than by changing reference COUNTS, using a do-while(0) wrap that emits a loop note without creating induction variables.
- mechanism: s3 already computed the right target arithmetic ("at one extra unit of loop_depth, t1val is 2*4/76 = 0.105 against t2's 2*7/150 = 0.093, so t1val wins $t1") but reached for the wrong construct to produce it: converting loop1's inner goto-loop into a real do-while/for. cc1 then recognises a real loop and manufactures TWO induction pseudos that take $t1/$t2 themselves. A `do { ... } while (0);` wrapped around loop1's body emits NOTE_INSN_LOOP_BEG (so flow.c adds the weight unit to every reference inside) while loop.c discards it as a non-loop, so no induction variable is created. It is the weighting half of the real-loop lever with none of the iv side effect.
- probe: decomp-permuter, chassis tmp/grind/func_800477E8/s4/wsA (full-TU compile, function-region extraction, base = s3's candidate, permuter base score 25). Random whole-function mode found a score-0 form after 30 s / ~200 iterations (911 iterations total before exit). The only semantic edit in the find is a do-while(0) around loop1's body from `a1 = 0;` through `a3 += 1;`, with `a0 = 0;` left outside it. Re-derived by hand, applied to src/sound.c and measured with `sandbox func_800477E8 --disable all`, then confirmed against the closed-form model on a fresh cc1 -da dump of the real src/sound.c.
- result: score 0, build_insns == target_insns == 170. The dump shows t1val (pseudo 76) refs 3 -> 4, live_length 76, priority 0.1053, seated in $t1(9); t2 (pseudo 83) refs 5 -> 7, live_length 148, priority 0.0946, seated in $t2(10) -- target's seating, exactly the numbers s3 predicted. The increment is worth +1 to t1val (one in-loop reference) and +2 to t2 (stored in both arms), and because floor_log2 stays at 2 for both while t2's live range is double t1val's, the increment flips the comparison.
- verdict: CONFIRMED

## [s4] HEAD's inherited `do { v0 = v1 | a1; } while (0);` inside the `a3 & 1` arm is load-bearing and must be kept (and therefore the s4 wrap is a NESTED do-while(0) needing the extra justification of prerequisite 3).
- mechanism: s1 flagged the inherited wrap as an outstanding un-annotated Judge surface and every session since carried it forward untested, assuming it was doing work.
- probe: Removed it (`v0 = v1 | a1;` plain) on top of the s4 wrap and re-ran `sandbox --disable all`.
- result: score 0, 170 insns -- unchanged. It does nothing. It is now deleted, the candidate contains exactly ONE do-while(0), the wrap is single-level, and the nested-wrap documentation duty does not arise. The s1/s2/s3 "outstanding un-annotated do-while(0)" Judge surface is closed.
- verdict: KILLED
