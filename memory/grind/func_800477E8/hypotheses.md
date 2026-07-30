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
