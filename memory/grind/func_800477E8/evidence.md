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
