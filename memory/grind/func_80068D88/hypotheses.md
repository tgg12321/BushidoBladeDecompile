# Hypothesis ledger — func_80068D88

## s1 (2026-08-03, recon modality)

### H1 — CONFIRMED (closed the function)
**Statement.** The entire score-5 residual is a two-pseudo register rename
(`prev_init` <-> `cur_init`, $a1 <-> $a3) that is decided by GCC 2.7.2's
`global.c` allocation ORDER, and that order is fixed by a *pure tie-break on
allocno number*, which in turn follows the LOCAL DECLARATION ORDER in the C
source. Therefore transposing the two declarations — with no other change
anywhere — flips the assignment onto target.

**Mechanism.** `tools/gcc-2.7.2/global.c`:
- `allocno_compare` computes priority as
  `(floor_log2(n_refs) * n_refs / live_length) * 10000 * size`, and when two
  allocnos tie it falls through to `return *v1 - *v2;` — the LOWER-numbered
  allocno sorts first and is allocated first by the `qsort (allocno_order, ...)`
  at global.c:546.
- `find_reg` then hands the first-allocated of two symmetric allocnos the lower
  free hard register.
- GCC creates a local's pseudo when the declaration is expanded, so pseudo
  numbers run in declaration order.

**Probe + measurement.** `cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -da` on
the cheat-stripped sandbox copy of text1b.c; `.greg` dump at
`tmp/grind/func_80068D88/s1/dump/text1b.i.greg`, function at line 41917:

    ;; 10 regs to allocate: 74 77 116 75 79 80 113 76 82 78
    ;; 79 conflicts: 74 75 76 77 78 79 80 2 3 29
    ;; 80 conflicts: 74 75 76 77 78 79 80 2 3 29      <- IDENTICAL to 79's
    ;; Register dispositions: ... 79 in 5   80 in 7 ...

Pseudo->variable map, each independently confirmed by the `addiu` displacement
the register holds in the emitted body:
  74 = outer (a0) | 75 = p_idx (t0, +0x6E) | 76 = p_prev (t2, +0x7C)
  77 = p_cur (a2, +0x80) | 78 = p_matrix (t4, +0x8C)
  79 = prev_init (a1) | 80 = cur_init (a3)
That map is exactly the declaration order of the score-5 form — the direct
confirmation that pseudo number == declaration position.

Because 79 and 80 have byte-identical conflict sets and equal priority, the
sort reaches `*v1 - *v2`, 79 wins, and `find_reg` gives 79 the lower reg $a1.
Target wants `cur_init` in $a1.

**Result.** Declared `cur_init` before `prev_init`; changed nothing else.
`sandbox func_80068D88 --disable all` went **5 -> 0**. Unmasked instruction
diff vs the oracle-matching `build/src/text1b.o`: 81 vs 81 instructions, every
opcode and every register identical; the only differing entries are the four
branch/jump TARGET ADDRESSES, which differ solely because the function sits at
a different offset inside the sandbox object (these are precisely the entries
the scorer masks). VERDICT: **CONFIRMED**.

### H0 — CONFIRMED (inherited, re-measured this session)
**Statement.** The score-5 floor recorded by session 0 is real and reproducible,
and the residual is *purely* a register rename with zero structural difference.

**Probe.** Applied session 0's `candidate.c` verbatim to `src/text1b.c`, ran the
sandbox (score 5), then dumped the unmasked instruction diff
(`tmp/grind/func_80068D88/s1/diff.py`). Exactly five real differences, all in
the two-value swap prologue, all register-name-only:

    idx  1   lw a1,0(gp)        vs  lw a3,0(gp)
    idx  7   lw a3,128(a0)      vs  lw a1,128(a0)
    idx  9   subu v1,a3,a1      vs  subu v1,a1,a3
    idx 22   sw a1,128(a0)      vs  sw a3,128(a0)
    idx 24   sw a3,124(a0)      vs  sw a1,124(a0)

Instruction COUNT, opcode sequence, scheduling, delay slots and every other
register were already identical — i.e. session 0's `p_a2` live-range split had
already closed all structural gap. VERDICT: **CONFIRMED** (and it is what made
H1's single-lever close possible).

### Not a hypothesis, but recorded: the layer-1 review exchange
The in-session `cheat-reviewer` FAILed the first submission on two grounds; the
second ground (that the `p_a2` reload of `D_800A34E4` is a "redundant global
round-trip" with no output difference) is factually contradicted by the target
bytes — target insns 52/53 ARE `lw a0,0(gp)` / `lw v1,0(gp)`, i.e. the original
code really does re-read both globals after publishing them. See evidence.md
for the full exchange and the final verdict.
