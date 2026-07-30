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

## Live frontier (for s2+)

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
