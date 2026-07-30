# Evidence bank — func_800477E8

## s1 (recon, 2026-07-30) — baseline map

**Gate / floor.** `canonical` → verdict **C**, 170 target insns, distance 17.
`sandbox --disable all` → **score 17**, `build_insns == target_insns == 170`,
`rules_dropped: 2`. There is **no structural gap at all**: our build emits the
exact same 170 instructions in the exact same order as target. Every one of the
17 residual diffs is a *register name*. (`diagnose` reports "LARGE / 26 differing
insn(s) / deep restructure" — that count is the UNMASKED diff and is misleading:
9 of those 26 are branch/jump target literals that differ only because the
sandbox object places the function at a different offset. The honest count is 17.)

**The two regfix rules describe the gap exactly** (regfix.txt:125-127):
```
# func_800477E8: swap t1/t2 and v1/a1 registers
func_800477E8: $9 <-> $10 @ 26-57
func_800477E8: $3 <-> $5 @ 32-135
```
So the whole function reduces to **two independent register-allocation ties**,
each resolved the wrong way round.

**Instruction-level diff** (build | target), branch-offset noise removed;
produced by `tmp/grind/func_800477E8/s1/diff.py` against `build/src/sound.o`:
```
 26  move t2,v0        | move t1,v0        <- t1val = gpu_CalcClut(0x10,0x1E0)
 28  li   t1,11264     | li   t2,11264     <- t2 = 0x2C00
 32  move v1,zero      | move a1,zero      <- a1 = 0
 33  li   a1,1         | li   v1,1         <- v1 = 1
 39  sh   t1,0(s0)     | sh   t2,0(s0)     <- *s0 = t2   (a3>=5 arm)
 55  sh   t1,0(s0)     | sh   t2,0(s0)     <- *s0 = t2   (else arm)
 57  sh   t2,0(s0)     | sh   t1,0(s0)     <- *s0 = t1val
 77  or   v0,a1,v1     | or   v0,v1,a1     <- v0 = v1 | a1  (do-while(0) arm)
 78  or   v0,a1,v1     | or   v0,v1,a1     <- v0 = v1 | a1  (else arm)
 93  addiu v1,v1,256   | addiu a1,a1,256   <- a1 += 0x100
 97  addiu a1,a1,1     | addiu v1,v1,1     <- v1 += 1
135  lui   v1,0x0      | lui   a1,0x0      <- ptr = &D_800EF59C[0]
136  addiu v1,v1,0     | addiu a1,a1,0     <- (same)
137  move  a1,a2       | move  v1,a2       <- v1 = a2       (loop2)
139  addiu v0,v1,64    | addiu v0,a1,64    <- p = ptr + 0x10
140  sw    a1,0(v0)    | sw    v1,0(v0)    <- *p = v1
148  addiu v1,v1,68    | addiu a1,a1,68    <- ptr += 0x11
```
Note idx 77/78 are NOT an operand-order divergence — both sides emit the source
order `v1 | a1`; the operands only *look* swapped because the two allocnos are
swapped. There is no commutativity lever here.

**Tie A — `$t1` vs `$t2`.** Contestants are the C locals `t1val`
(= `gpu_CalcClut(0x10,0x1E0)`, 2 refs: its def + one use in the `else` arm) and
`t2` (= constant `0x2C00`, 3 refs: its def + one use in EACH arm). Both are
caller-saved-eligible because no call follows their definitions. Build gives
`t2 -> $t1` and `t1val -> $t2`; target wants `t1val -> $t1`, `t2 -> $t2`.
Since MIPS `REG_ALLOC_ORDER` reaches `$t1` before `$t2`, whichever allocno GCC
*allocates first* wins `$t1` — i.e. build has `t2` out-prioritising `t1val`,
consistent with `t2` having the higher reference count (global.c priority is
~ `log2(n_refs) * n_refs * size / live_length`).

**Tie B — `$v1` vs `$a1`.** This is ONE mis-seated allocno, not two: the C local
`v1` is reused in BOTH loop bodies (loop1 `v1 = 1; v1 += 1;` and loop2
`v1 = a2; *p = v1;`), so it is a single pseudo whose live range spans nearly the
whole function. Its partners are two *different*, disjoint allocnos: `a1` in
loop1 and `ptr` in loop2. Build seats `v1 -> $a1` with both `a1` and `ptr`
taking `$v1`; target seats `v1 -> $v1` with `a1` and `ptr` on `$a1`. Long live
range => low `n_refs/live_length` priority => `v1` is allocated late and loses
`$v1`. Target's allocation implies the ORIGINAL C also shared one variable
across both loops (see the KILLED split probe) but gave it enough priority to
win `$v1`.

**Pre-existing sanctioned construct in the body (inherited from HEAD, not added
by s1):** `do { v0 = v1 | a1; } while (0);` at src/sound.c:746, inside the
`a3 & 1` arm. It is a `do-while(0)` under [[do-while-zero-exception]]; it is
NOT annotated `/* FAKE */` in HEAD. A future session that reaches a candidate
must either annotate it per the rule or eliminate it — flagging it now so the
Judge surface is known in advance.

**Body provenance.** src/sound.c:674-816 is a verbatim m2c transcription: every
local is named after the hard register m2c saw (`s0/s3val/s2val/s1val/t1val/a3/
a0/a2/a1/t0/v1/t2/v0/ptr/p`), control flow is `goto inner:` / `goto inner2:` /
`goto loop3:` rather than real loops, and the two `gpu_CalcClut(0x10,0x1E0)`
calls at lines 694-695 are byte-identical duplicates. The function fills a
primitive/OT buffer at `D_800A33D0` with 8 rows x 16 columns of 11 halfwords
(tpage, clut, uv pairs), then initialises `D_800EF0xx` and two tables
(`D_800EF59C`, `D_800EF558`), and returns the bytes written. The packed values
are UV coordinates: `(a2 | t0)` starts 0x1213 and `(v1 | a1)` starts 0x0001,
each advancing by 0x0101 per inner iteration (low byte `+1`, high byte `+0x100`).

- [s1] canonical gate: verdict C, 170 target insns, distance 17 — a pure-C target, no ASM routing question.

- [s1] sandbox --disable all = 17 with build_insns == target_insns == 170. There is NO structural gap: our build already emits the same 170 instructions in the same order as target. All 17 residual diffs are register names.

- [s1] engine `diagnose` reports 'LARGE / 26 differing insn(s) / deep restructure' for this function — that count is WRONG as a difficulty signal. 9 of those 26 are branch/jump target literals that differ only because the sandbox object places the function at a different offset than build/src/sound.o. The honest count is 17. Do not treat the diagnose LARGE verdict as evidence for a restructure or a park.

- [s1] The gap is exactly two independent register-allocation ties, matching the two regfix rules verbatim (regfix.txt:125-127, '$9 <-> $10 @ 26-57' and '$3 <-> $5 @ 32-135').

- [s1] Tie A ($t1/$t2): contestants are 't1val' (= gpu_CalcClut(0x10,0x1E0), 2 refs) and 't2' (= constant 0x2C00, 3 refs — def plus one use in EACH arm). Build seats t2->$t1 and t1val->$t2; target wants t1val->$t1 and t2->$t2. Consistent with t2's higher reference count winning the earlier slot in REG_ALLOC_ORDER.

- [s1] Tie B ($v1/$a1) is ONE mis-seated allocno, not two: the C local 'v1' is used in BOTH loop bodies (loop1 'v1 = 1; v1 += 1;' and loop2 'v1 = a2; *p = v1;') so it is a single long-lived pseudo, and its competitors are two DISJOINT allocnos — 'a1' in loop1 and 'ptr' in loop2. Build seats v1->$a1 with both a1 and ptr on $v1; target wants v1->$v1 with a1 and ptr on $a1.

- [s1] The 'or v0,a1,v1' vs 'or v0,v1,a1' diff at idx 77/78 is NOT an operand-order/commutativity divergence — both sides emit the source order 'v1 | a1' and the operands only look swapped because the two allocnos are swapped. There is no commutativity lever there.

- [s1] POSITIVE constraint from the killed split probe: target's allocation requires ONE variable shared across loop1 and loop2, exactly as HEAD has it. Any future restructure must PRESERVE that sharing.

- [s1] Body provenance: src/sound.c:674-816 is a verbatim m2c transcription — every local is named for the hard register m2c observed (s0/s3val/s2val/s1val/t1val/a3/a0/a2/a1/t0/v1/t2/v0/ptr/p), control flow is goto-based rather than real loops, and lines 694-695 issue two byte-identical gpu_CalcClut(0x10,0x1E0) calls. Semantically the function fills a primitive/OT buffer at D_800A33D0 with 8 rows x 16 columns of 11 halfwords (tpage, clut, packed UVs advancing 0x0101 per column), then initialises D_800EF0xx and the D_800EF59C / D_800EF558 tables, returning the byte count written.

- [s1] Judge surface known in advance: the body inherits an un-annotated 'do { v0 = v1 | a1; } while (0);' at src/sound.c:746. It falls under .claude/rules/do-while-zero-exception but carries no /* FAKE */ annotation in HEAD. Whoever reaches distance 0 must annotate or eliminate it before proposing the candidate.

- [s1] src/sound.c was restored to HEAD via `git checkout --` at end of session; working tree carries no s1 source edits, and the reverted form re-measured at 17.

## s2 (structural, 2026-07-30) — the allocation dump, and loop1 SOLVED

**The decisive tool: a cc1 `-da` dump of the REAL src/sound.c.** No standalone
reconstruction is needed. `tmp/grind/func_800477E8/s2/greg.sh` runs the exact
Makefile cpp+cc1 flags with `-da` over `src/sound.c` and drops
`sound.i.greg` next to it. (src/sound.c has a pre-existing arity error in an
UNRELATED function, `func_800470B0` calling `func_80052930`, which makes cc1
exit non-zero — the dumps are still written in full, so ignore the exit status.)
The `;; Function func_800477E8` section gives three things directly:
  * `;; 15 regs to allocate: ...` — the allocnos in **allocation priority order**
  * `;; NN conflicts: ...`        — the full conflict graph
  * `;; Register dispositions:`   — pseudo -> hard reg
This turns the whole problem from guesswork into simulation: `find_reg` assigns
each allocno, in that order, the LOWEST-numbered hard register not held by a
conflicting allocno (MIPS gcc-2.7.2 defines no `REG_ALLOC_ORDER`).

**Pseudo map at HEAD** (they follow declaration order):
72=s0 73=s3val 74=s2val 75=s1val 76=t1val 77=a3 78=a0 79=a2 80=a1 81=t0 82=v1
83=t2 84=v0 85=ptr 86=p.

**HEAD baseline, verbatim from the dump:**
```
;; 15 regs to allocate: 84 72 86 85 78 82 79 77 80 81 83 76 75 74 73
;; Register dispositions: 72->16 73->19 74->18 75->17 76->10 77->7 78->4
                          79->6 80->3 81->8 82->5 83->9 84->2 85->3 86->2
```
i.e. priority order v0, s0, p, ptr, a0, v1, a2, a3, a1, t0, t2, t1val.

**The order target REQUIRES** (derived by simulating find_reg against the printed
conflict sets until the assignment equals target's):
`v0, s0, p, v1, a0, a1, a2, a3, t0, t1val, t2`, with `ptr` anywhere after `a0`.
So three inversions had to be fixed: v1 above ptr AND above a0; a1 above a2/a3;
t1val above t2.

**Target's own seating, read off asm/funcs/func_800477E8.s:** loop1
v0=2 v1=3 a0=4 a1=5 a2=6 a3=7 t0=8 t1val=9 t2=10 s0=16 s1val=17 s2val=18
s3val=19; loop2 p=$v0(2) value=$v1(3) counter=$a0(4) ptr=$a1(5) accum=$a2(6)
row=$a3(7); loop3 pointer=$v1(3) index=$a0(4).

**The two gpu_CalcClut calls really are two distinct calls.** Target does
`jal gpu_CalcClut / addu $s1,$v0,$zero` (delay slot ⇒ captures the PREVIOUS
call's result) then `addu $t1,$v0,$zero` — so s1val = 3rd call, t1val = 4th
call, exactly as the C has it. The s1 frontier question "do the arms legitimately
share one value" is answered NO; do not merge them.

- [s2] LOOP1 IS SOLVED. The form saved in candidate.c reproduces target's ENTIRE
  loop1 register seating (a0->$a0, a1->$a1, a2->$a2, a3->$a3, t0->$t0, v1->$v1,
  v0->$v0, s0/s1val/s2val/s3val correct). Floor is still 17 because the 12
  loop1 diffs were traded for 12 loop2 diffs, but the residual is now confined
  to (a) loop2's four locals and (b) tie A (t1val/t2). Verified by the .greg
  dispositions, not inferred.

- [s2] s1's K2 ("v1 must remain ONE variable shared across loop1 and loop2") is
  OVERTURNED. K2 judged the split by the SCORE (17->31) rather than the seating.
  The dump shows the split is a PREREQUISITE: while `v1` is shared it conflicts
  with loop2's walking pointer `ptr`, and `ptr` is a tight-range induction
  pseudo whose allocno priority (floor_log2(4)*4/~8 ~= 1.0-1.6) can never be
  beaten by a loop-spanning value pseudo (~0.26), so `ptr` takes $v1 first,
  unconditionally. Splitting removes the conflict and `v1` lands in $v1.

- [s2] The correct variable partition is ASYMMETRIC and must be respected
  exactly: loop2's row counter, accumulator and stored value MUST be their own
  locals (they are semantically unrelated quantities m2c merged by hard
  register), while `a0` (column counter / loop2 counter / loop3 index) and `ptr`
  (loop2 row pointer / loop3 pointer) MUST stay shared — splitting `a0` costs
  17 -> 40, and splitting `ptr` off loop3 is completely inert.

- [s2] Declaration order is INERT for every contested pair in this function,
  before AND after the splits (measured on the dump: byte-identical priority
  order and dispositions). global.c only falls back to the allocno number when
  two priorities are EXACTLY equal, and none of the contested pairs here are.

- [s2] Statement order among the three pre-loop initialisations IS load-bearing
  and is currently the fragile part of the candidate: `t2 = 0x2C00; a3 = 0;
  a0 = 0;` scores 17 with loop1 exact, `a3 = 0; t2 = 0x2C00; a0 = 0;` (HEAD's
  order) swaps a3 with t0, and `t2; a0; a3;` scores 26. Mechanism is
  allocno_live_length: a3 has 6 refs to t0's 5 and only out-ranks t0 while its
  live range stays short. A successor should try to reach the same seating from
  a natural loop shape before proposing this ordering to the Judge.

- [s2] A full nested-for() rewrite of loop1 is NOT free: it makes GCC create two
  EXTRA induction pseudos, pushing t2 to $t4 and t1val to $t3. Any rederive
  session must re-read the .greg dump after changing loop shape rather than
  assuming the shape is neutral.

- [s2] Tie A is a pure reference-count gap that no structural lever reached:
  t1val has 2 references (its def plus one store in the `else` arm), t2 has 3
  (its def plus one store in EACH arm), with essentially identical live ranges,
  so t2's priority (1*3/L) beats t1val's (1*2/L) and t2 takes the lower $t1.
  Equalising the counts would need a third reference to t1val or the removal of
  one of t2's two stores, and both arms genuinely store 0x2C00, so no honest
  restructure of this body changes the counts. Tie A is worth 5 of the 17.

- [s2] Harness for successors (reuse it, do not rebuild it):
  `tmp/grind/func_800477E8/s2/sweep.py` screens a list of source variants purely
  through the cc1 -da dump — it copies src/sound.c to tmp, applies literal
  (old,new) edits, compiles, and prints the allocation order + dispositions. It
  NEVER touches src/, and one run screens ~8 variants in seconds.
  `score.py` (same directory, same variant files) applies each variant to
  src/sound.c for real, runs `sandbox --disable all`, and restores src/sound.c
  in a `finally:` block. Variant sets live in variants.py .. variants7.py and
  are selected with `VSET=variantsN`.

- [s2] src/sound.c was left byte-identical to HEAD at end of session
  (`git status --porcelain -- src/` empty); every measurement was made through
  score.py's apply/restore cycle or through the tmp-only dump path.

- [s2] The cc1 -da dump of the REAL src/sound.c is the right instrument for this function - no standalone reconstruction is needed. tmp/grind/func_800477E8/s2/greg.sh runs the exact Makefile cpp+cc1 flags with -da; src/sound.c contains a pre-existing arity error in the UNRELATED function func_800470B0 (calling func_80052930), so cc1 exits non-zero, but every dump file is still written in full - ignore the exit status.

- [s2] Pseudo map at HEAD (they follow declaration order): 72=s0 73=s3val 74=s2val 75=s1val 76=t1val 77=a3 78=a0 79=a2 80=a1 81=t0 82=v1 83=t2 84=v0 85=ptr 86=p.

- [s2] Target's seating, read off asm/funcs/func_800477E8.s: loop1 v0=2 v1=3 a0=4 a1=5 a2=6 a3=7 t0=8 t1val=9 t2=10 s0=16 s1val=17 s2val=18 s3val=19; loop2 p=$v0(2) value=$v1(3) counter=$a0(4) ptr=$a1(5) accumulator=$a2(6) row=$a3(7); loop3 pointer=$v1(3) index=$a0(4).

- [s2] LOOP1 IS SOLVED. memory/grind/func_800477E8/candidate.c reproduces target's entire loop1 seating (a0->$a0, a1->$a1, a2->$a2, a3->$a3, t0->$t0, v1->$v1, v0->$v0, s0/s1val/s2val/s3val correct), verified on the .greg dispositions rather than inferred. Floor is still 17 only because the 12 loop1 diffs were traded for 12 loop2 diffs; the residual is now 12 (loop2) + 5 (tie A).

- [s2] The correct variable partition is ASYMMETRIC and must be respected exactly: loop2's row counter, accumulator and stored value MUST be their own locals (semantically unrelated quantities that m2c merged by hard register), while 'a0' and 'ptr' MUST stay shared. Splitting a0 costs 17 -> 40; splitting ptr off loop3 is completely inert.

- [s2] Statement order among the three pre-loop initialisations is load-bearing and is the fragile part of the candidate: 't2 = 0x2C00; a3 = 0; a0 = 0;' scores 17 with loop1 exact; HEAD's 'a3 = 0; t2 = 0x2C00; a0 = 0;' swaps a3 with t0; 't2; a0; a3;' scores 26. Mechanism is allocno_live_length - a3 has 6 refs to t0's 5 and only out-ranks t0 while its live range stays short. It is ordinary statement order, not a coercion, but a successor should try to reach the same seating from a natural loop shape before proposing it to the Judge.

- [s2] A full nested-for() rewrite of loop1 is NOT shape-neutral: GCC creates two EXTRA induction pseudos and pushes t2 to $t4(12) and t1val to $t3(11). Any rederive session must re-read the .greg dump after changing loop shape.

- [s2] Remaining loop2 errors with the candidate applied: val->$a3(7) wants $v1(3); ptr->$v1(3) wants $a1(5); w->$a1(5) wants $a2(6); r->$a2(6) wants $a3(7). a0->$a0(4) and p->$v0(2) are already correct. The required loop2 allocation order is p, val, a0, ptr, w, r.

- [s2] Reusable harness for successors, do NOT rebuild it: tmp/grind/func_800477E8/s2/sweep.py screens a list of source variants purely through the cc1 -da dump (copies src/sound.c to tmp, applies literal (old,new) edits, compiles, prints allocation order + dispositions) and NEVER touches src/; one run screens ~8 variants in seconds. score.py applies each variant to src/sound.c for real, runs sandbox --disable all, and restores src/sound.c in a finally: block. Variant sets are variants.py .. variants7.py, selected with VSET=variantsN.

- [s2] Scores measured this session (all build_insns 170): HEAD 17; split-a2-only 23; split-a2+decl-a1-first 23; split-v1-only 29 (s1's K2 form); split-v1+split-a2 25; loop2-fully-split 25; loop2-fully-split+declorder 25; loop2-split-keep-a3 25; fullsplit+a3-init-last 26; fullsplit+a3-init-mid 17 (the candidate); mid+loop3-own-ptr 17; mid+val-declared-first 17; mid+loop2-head-shape 24; mid+loop2-own-counter 40; mid+loop3-own-ptr+own-counter 36.

- [s2] src/sound.c was left byte-identical to HEAD at end of session (git status --porcelain -- src/ empty). Every measurement went through score.py's apply/restore cycle or the tmp-only dump path.

## s3 (structural, 2026-07-30) — floor 17 -> 5; loop1 AND loop2 now exactly target

**The model closed.** GCC 2.7.2 global.c `allocno_compare` ranks allocnos by
`floor_log2(n_refs) * n_refs * size / live_length`, and `n_refs` / `live_length`
are printed verbatim by the cc1 `-da` **`.lreg`** dump as
`Register NN used R times across L insns`. Computing that expression over the
`.lreg` numbers reproduces the `.greg` `;; N regs to allocate:` priority order
**exactly, every time**, for every variant measured this session. Combined with
`find_reg`'s "lowest-numbered hard reg not held by a conflicting allocno" (MIPS
gcc-2.7.2 defines no `REG_ALLOC_ORDER`) and the printed conflict sets, the whole
seating is now a closed-form calculation rather than a search. `size` is 1 for
every allocno here, so it drops out. An EXACT tie falls back to allocno number
(lower wins), which is reachable and is the intended endgame for tie A.

**The lever s1 and s2 both missed: reference weighting is loop-note driven.**
flow.c weights each reference by `loop_depth`, and `loop_depth` is driven by
`NOTE_INSN_LOOP_BEG`, which the front end emits only for real loop constructs.
m2c's `goto inner2:` / `goto outer2:` loops emit NONE, so every loop2 pseudo was
being scored with unweighted references — which is why loop2's stored value sat
at priority 0.29 and could never out-rank the walking pointer's 1.14, and why two
sessions of statement-order and split probes could not move it. Writing loop2 as
a real `do { ... for (...) ... } while (...)` nest raises the same pseudos to
1.4-1.8 and puts the whole group in play. This is a general lesson for this
codebase, not a fact about this function: **an m2c goto-loop body has
systematically wrong allocation priorities, and no amount of variable splitting
inside it can fix them.**

**Loop2's two extra registers are compiler-made, and that is load-bearing.**
Target's loop2 preheader is `addu $a3,$zero,$zero; addu $a2,$zero,$zero;
lui/addiu $a1` — the row POINTER's def comes LAST, after both counters. loop.c
inserts induction-variable initialisations into the preheader, i.e. after all
source statements, so a compiler-made iv can only ever be initialised after a
source-level pointer. Therefore in the original the row pointer is the
compiler-made iv and the accumulator is the source variable, not the other way
round. Indexing the row (`p = &D_800EF59C[a3 * 0x11]; p[a0] = val;`) makes loop.c
strength-reduce the row address into `$a1` and the inner index into the `$v0`
walking pointer, and only then does `addiu $a1,$a1,0x44` land in the branch delay
slot as target has it. Source-level row pointer forms put the accumulator and the
pointer in each other's registers, every time (score 14).

- [s3] FLOOR 17 -> 5. memory/grind/func_800477E8/candidate.c scores 5 with
  build_insns == target_insns == 170. Loop1, loop2 and loop3 register seating all
  equal target on the .greg dispositions; the residual 5 is EXACTLY tie A
  (t1val/t2) and nothing else, confirmed instruction-by-instruction with
  tmp/grind/func_800477E8/s1/diff.py against build/src/sound.o (pass that as
  argv[1] — the script's default reference build/asm/6CAC.o no longer contains
  this function and raises KeyError).

- [s3] The allocation priority formula floor_log2(n_refs)*n_refs/live_length over
  the .lreg "used R times across L insns" numbers reproduces cc1's printed
  allocation order exactly. Use tmp/grind/func_800477E8/s3/sweep2.py, which prints
  order + dispositions + per-pseudo priority for a whole variant list in one run
  and never touches src/. This makes every future probe on this function a
  prediction, not a guess.

- [s3] References are weighted by loop_depth, and goto-shaped loops emit no
  NOTE_INSN_LOOP_BEG, so they get NO weighting. This is why s1/s2 could not move
  loop2: with the goto shape the stored value has 2 refs (priority 0.29) against
  the walking pointer's 8 (1.14), and 2 refs can never win. Real loop constructs
  raise the same references to 5-7 and reverse the order.

- [s3] loop2's row counter must be the SAME C variable as loop1's a3. Sharing
  drags its priority to 0.84 (18 refs / 86 insns) so it is allocated LAST of the
  loop2 group and takes $a3, as target has it. A fresh local scores 1.17, is
  allocated third, and rotates the whole group (score 14 instead of 5). This
  REVERSES s2's split for loop2's counter while keeping s2's split for the stored
  value — the partition is: share a0 and a3 with loop1, keep w and val separate,
  and let the row pointer and inner pointer be compiler-made.

- [s3] loop2's counter must also be READ inside the body. With it used only by its
  own increment and test, cc1's check_dbra_loop reverses the loop into a countdown
  and drops target's `slti $v0,$a3,0x9` — build_insns 169, one short of 170, score
  11 even with otherwise perfect dispositions. The `a3 * 0x11` row index supplies
  that reference and simultaneously makes the row pointer compiler-made, so one
  change buys both requirements.

- [s3] The pre-loop initialisation order is TARGET's own order,
  `a3 = 0; t2 = 0x2C00; a0 = 0;` (target preheader: addu $a3,$zero,$zero /
  addiu $t2,$zero,0x2C00 / addu $a0,$zero,$zero), worth exactly one instruction
  (score 6 -> 5). s2's caveat that the candidate depended on an arbitrary
  `t2; a3; a0;` ordering is RETIRED: that ordering was an artifact of s2's fresh
  row-counter local, and with a3 shared the order target itself emits is the best.

- [s3] TIE A's mechanism is PROVEN and s2's K8 is overturned on mechanism (though
  not yet on cost). Giving t2 a SINGLE in-loop reference — lifting the shared
  `*s0 = t2;` out of loop1's two arms — produces t1val->$t1(9) and t2->$t2(10),
  i.e. target, with every other disposition still correct. t2's priority is
  2*5/150 = 0.0667 against t1val's 1*3/76 = 0.0395; t2's live_length is double
  because t2 is used in BOTH arms and stays live on both paths while t1val is dead
  along the a3>=5 path. Factoring drops t2 to 3 refs / 150 = 0.020 and it loses.
  The obstacle is purely COST: factoring needs a second test of `a3 >= 5` and cc1
  emits 171 instructions instead of 170 (score 10-13), and caching the condition
  in a local does not avoid it.

- [s3] Tie A alternatives measured this session: literal 0x2C00 in both arms —
  CSE rebuilds one pseudo with the same 2-use profile, score 6, seating unchanged
  (re-measurement of s1's K3 in the new body, same verdict); literal in the a3>=5
  arm with t2 in the else arm — score 3 but build_insns 171, the best score seen
  anywhere and still not a path to 0 because the second `li` is a real extra
  instruction; both arms storing a v0 copy of t2 — score 39; `s0[1] = t2;` hoisted
  ahead of the branch with `s0 += 2` in the arms — 167 insns, score 10; inverting
  the arms' branch sense — score 18; swapping which CLUT result each arm stores —
  score 7.

- [s3] Converting loop1's INNER goto-loop into a real do-while or for is dead, and
  for an instructive reason. The model predicts it flips tie A (depth-3 weighting
  gives t1val 2*4/76 = 0.105 against t2's 2*7/150 = 0.093), but cc1 manufactures
  TWO extra induction pseudos for the inner loop which take $t1 and $t2 and push
  t1val to $t3(11) and t2 to $t4(12): score 10 (do-while) / 13 (for), insns 170.
  Same failure mode s2 saw with a full nested-for rewrite of loop1. So the loop
  shapes are ASYMMETRIC — loop2 MUST be real loops, loop1's inner MUST stay a
  goto — because loop2's counters are consumed as an index (cc1 builds the ivs we
  need) while loop1's are consumed as packed UV values (cc1 builds ivs we do not
  want).

- [s3] Also dead this session: splitting loop3's index off a0 (score 32); giving
  loop2 its own inner counter (the compiler-made walking pointer then loses $v0);
  fully flat indexing `D_800EF59C[a3*0x11 + a0]` with no row-base local at all
  (156 cc1 insns, build 173, score 27); letting LICM manufacture the stored value
  from `a3 * 0x7D0` with no source-level accumulator (score 14 — the accumulator
  then becomes compiler-made and swaps $a1/$a2); making loop2's outer a for()
  instead of a do-while (the row pointer lengthens by one insn and loses).

- [s3] Harness added: tmp/grind/func_800477E8/s3/sweep2.py (dump screener with
  priority arithmetic and a cc1 insn count; VSET=variantsN selects the variant
  list, reused from s2's directory) and tmp/grind/func_800477E8/s3/apply.py
  (splices a candidate body into src/sound.c in place). s2's score.py is unchanged
  and still the real-measurement path. Variant lists variants8.py .. variants19.py
  live alongside s2's in tmp/grind/func_800477E8/s2/.

- [s3] src/sound.c was restored to HEAD at end of session; every measurement went
  through score.py's apply/restore cycle or the tmp-only dump path.

- [s3] FLOOR 17 -> 5. memory/grind/func_800477E8/candidate.c scores 5 with build_insns == target_insns == 170. Loop1, loop2 and loop3 register seating all equal target on the cc1 -da .greg dispositions; the residual 5 is EXACTLY tie A and nothing else, confirmed instruction-by-instruction against build/src/sound.o.

- [s3] The honest diff at floor 5 (branch-offset noise removed) is: idx 26 'move t2,v0' vs 'move t1,v0'; idx 39 and 55 'sh t1,0(s0)' vs 'sh t2,0(s0)'; idx 57 'sh t2,0(s0)' vs 'sh t1,0(s0)'; plus the li of 0x2C00 landing in $t1 rather than $t2. All five are tie A.

- [s3] The allocation model is now closed-form: priority = floor_log2(n_refs)*n_refs*size/live_length over the .lreg dump's 'Register NN used R times across L insns' numbers reproduces cc1's printed ';; N regs to allocate:' order EXACTLY, for every variant measured this session; size is 1 for all allocnos here. find_reg then gives each allocno the lowest-numbered hard reg not held by a conflicting allocno (MIPS gcc-2.7.2 defines no REG_ALLOC_ORDER), and an EXACT priority tie falls back to allocno number, lower first.

- [s3] GENERAL LESSON, not function-specific: flow.c weights every register reference by loop_depth, and loop_depth comes from NOTE_INSN_LOOP_BEG notes that the front end emits only for real loop constructs. An m2c goto-shaped loop body therefore gets NO reference weighting at all, its allocation priorities are systematically wrong, and no amount of variable splitting inside it can fix them. Every queue item transcribed verbatim from m2c with goto-driven loops is a candidate for the same treatment.

- [s3] Target's loop2 preheader order ('addu $a3,$zero,$zero; addu $a2,$zero,$zero; lui/addiu $a1', pointer LAST) is diagnostic: loop.c inserts iv initialisations after all source statements, so the row pointer being initialised last proves it is the compiler-made induction variable and the accumulator is the source variable. This is what forced the indexed-row form and it is why source-level row-pointer forms always swap $a1/$a2.

- [s3] cc1's check_dbra_loop reverses a real loop whose counter is used only by its own increment and test, dropping the compare instruction. That cost one instruction (build_insns 169) with otherwise perfect dispositions, and the fix -- an honest in-body read of the counter -- is the same 'a3 * 0x11' row index that makes the row pointer compiler-made.

- [s3] The candidate's pre-loop initialisation order is TARGET's own order, 'a3 = 0; t2 = 0x2C00; a0 = 0;', worth exactly one instruction. s2's flagged 'arbitrary statement order' Judge surface is therefore retired.

- [s3] Tie A is a reference-count problem and is REACHABLE: giving t2 a single in-loop reference seats t1val->$t1 and t2->$t2 exactly as target, with every other disposition still correct. The only obstacle is that every spelling of that found so far costs one instruction (171 vs 170).

- [s3] Also measured dead this session: splitting loop3's index off a0 (score 32); a fresh inner counter for loop2 (the compiler-made walking pointer then loses $v0); fully flat indexing D_800EF59C[a3*0x11 + a0] with no row-base local (build 173, score 27); letting LICM manufacture the stored value from a3 * 0x7D0 with no source-level accumulator (score 14); loop2's outer as a for() rather than a do-while (row pointer lengthens by one insn and loses); inverting the arms' branch sense (18); routing both arms' t2 store through v0 (39); 's0[1] = t2;' hoisted ahead of the branch with 's0 += 2' in the arms (167 insns, score 10).

- [s3] src/sound.c was restored to HEAD at end of session (git status --porcelain -- src/ empty). Every measurement went through score.py's apply/restore cycle or the tmp-only dump path.

- [s3] Harness for successors: tmp/grind/func_800477E8/s3/sweep2.py (dump screener with the priority arithmetic and a cc1 insn count, VSET=variantsN, never touches src/) and tmp/grind/func_800477E8/s3/apply.py (splices a candidate body into src/sound.c). s2's score.py is the real-measurement path and is unchanged. Variant lists variants8.py .. variants19.py sit next to s2's in tmp/grind/func_800477E8/s2/. NOTE: s1/diff.py needs build/src/sound.o passed as argv[1] -- its default reference build/asm/6CAC.o no longer contains this function.

## s4 (permuter, 2026-07-30) — floor 5 -> 0; MATCHED

- [s4] FLOOR 5 -> 0. `sandbox func_800477E8 --disable all` prints score 0 with
  build_insns == target_insns == 170 and rules_dropped 2, with
  memory/grind/func_800477E8/candidate.c applied to src/sound.c. The form is
  s3's body plus ONE `do { ... } while (0);` wrap around loop1's body (from
  `a1 = 0;` through `a3 += 1;`, with `a0 = 0;` left outside it), annotated
  inline `/* FAKE: ... */` per .claude/rules/do-while-zero-exception.md.

- [s4] The wrap closes tie A by RAISING REFERENCE WEIGHTING, not by changing
  reference counts — which is what s1, s2 and s3 all searched for. flow.c
  weights each register reference by loop_depth, and a do-while(0) emits
  NOTE_INSN_LOOP_BEG so every reference inside loop1's body gains one weight
  unit, while loop.c discards the construct as a non-loop so NO induction
  variable is manufactured. That is precisely the difference from s3's killed
  probe (making loop1's inner goto-loop a real loop), where cc1 created two
  extra iv pseudos that took $t1/$t2 themselves.

- [s4] Measured on a fresh cc1 -da dump of the real src/sound.c (dumper:
  tmp/grind/func_800477E8/s4/dumpstat.py over tmp/grind/func_800477E8/s4/dump/v.i):
  t1val (pseudo 76) refs 3 -> 4, live_length 76, priority 0.1053, seat $t1(9);
  t2 (pseudo 83) refs 5 -> 7, live_length 148, priority 0.0946, seat $t2(10).
  Both equal target. The increment is +1 for t1val (one in-loop reference) and
  +2 for t2 (stored in BOTH arms); floor_log2 stays at 2 for both and t2's live
  range is double t1val's, so the increment flips the comparison. s3's
  closed-form model predicted these exact numbers — s3 had the arithmetic right
  and only the construct wrong.

- [s4] HEAD's inherited `do { v0 = v1 | a1; } while (0);` in the `a3 & 1` arm is
  INERT: removing it leaves score 0 / 170 insns. It is deleted from the
  candidate, so the function now carries exactly ONE do-while(0), the s4 wrap is
  SINGLE-LEVEL, and prerequisite 3 (nested-wrap justification) does not apply.
  The s1/s2/s3 "outstanding un-annotated do-while(0)" Judge surface is closed.

- [s4] The permuter is a viable instrument on this function and the workspace is
  reusable: tmp/grind/func_800477E8/s4/mkws.sh builds a decomp-permuter
  workspace from the CURRENT src/sound.c — full-TU cpp with the Makefile flags,
  strip_other_fns (NOTE: that script rewrites its input file IN PLACE and prints
  nothing), a compile.sh that runs the real cc1 | prologue_fix | maspsx |
  multu_pad pipeline and extracts only func_800477E8's region, and a target.o
  assembled from asm/funcs/func_800477E8.s. It self-validates by diffing base vs
  target. Campaign telemetry: base_score 25 (permuter weighting, 5 register
  diffs x 5), 911 iterations, ONE find, score 0, at 30 s after launch — the
  basin yielded almost immediately, consistent with the fresh-seed discipline.

- [s4] INTEGRATION NOTE for the operator/driver: regfix.txt:125-127 still carries
  the two rules for this function (`$9 <-> $10 @ 26-57`, `$3 <-> $5 @ 32-135`).
  They now describe swaps that the C no longer needs, so a normal (rules-enabled)
  build will be WRONG until `retire func_800477E8` removes them. The honest
  cheat-free distance is 0; the retire + full-build SHA1 check is the remaining
  step and it is on a surface a grind session may not touch.
