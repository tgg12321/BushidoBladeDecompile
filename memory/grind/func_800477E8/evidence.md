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
