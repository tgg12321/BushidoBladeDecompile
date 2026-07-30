# Hypothesis ledger — func_80040CB8

## Session s1 (2026-07-30, modality: recon)

### H1 — CONFIRMED. The 37th instruction is a sentinel-typing artefact.
**Statement.** The inherited pinned form's one-instruction surplus (37 vs 36) is
a `sll/sra` pair sign-extending the `-1` comparand, present only because the
sentinel was declared `s16`.
**Mechanism.** `lh` already delivers a sign-extended value, but comparing two
`s16`-typed operands makes cc1 materialise a sign-extension of the register-held
constant before the `beq`. Typing the sentinel `s32` (or comparing against a
plain `int`) removes the pair, and as a side effect frees `$v0` so the `lh`
destination lands in `$v0` exactly as target has it.
**Probe.** `s16 none` → `s32 none`; sandbox --disable all.
**Result.** 37 → 36 instructions; the emitted opcode/displacement sequence
becomes identical to `asm/funcs/func_80040CB8.s`.
**Verdict.** CONFIRMED.

### H2 — CONFIRMED. Any GCC-recognised loop form destroys the addressing.
**Statement.** The loop must be written as a `goto` back-edge, not as
`for`/`while`/`do-while`.
**Mechanism.** A recognised loop emits `NOTE_INSN_LOOP_BEG`, enabling loop.c
strength reduction, which creates a third induction pointer at `arg0+0x8B8` to
serve the `-0x57..-0x4C` displacement cluster and demotes `arg0+0x90C` to the
single `+0x58` store. That is three `addiu rX,rX,0x68` increments per iteration
against target's two — a two-instruction surplus no register choice can recover.
**Probe.** Same body as `do { … } while (i < 0x12);`; sandbox --disable all.
**Result.** 38 insns, score 25 (vs 36/13 for the goto form). Banked as
`rejected/do-while-loop-form-triggers-strength-reduction.c`.
**Verdict.** CONFIRMED (the goto form is required).

### H3 — CONFIRMED. The register permutation is an allocno-priority ordering
problem, and declaration position is a real lever on it.
**Statement.** With all long-lived pseudos mutually conflicting and MIPS having
no `REG_ALLOC_ORDER`, each variable's hard register is decided solely by its
rank under `global.c:allocno_compare`
(`floor_log2(n_refs) * n_refs / live_length`). Shortening `slot`'s live range by
declaring it last lifts it past `tbl` and `link` and fixes seven registers.
**Mechanism.** `slot` had `n_refs 5 / live_length 31` → priority 0.323, just
below `tbl` (0.348) and `link` (0.333), so it was allocated fifth and got `$t0`
instead of `$a2`. Declaring it last drops its live_length under the 28.7
crossover, re-ranking to ent, copy, slot, tbl, link, i, one, kind, none.
**Probe.** Move `s8 *slot = …0x8B4;` from first to last declaration; sandbox.
**Result.** score 21 → **13**; `slot=$a2, tbl=$a3, link=$t0, i=$t1, one=$t2,
kind=$t3, none=$t4` all land on target. Disassembly `s1/build4.txt`.
**Verdict.** CONFIRMED.

### H4 — KILLED. The `id` copy is not a source-level intermediate.
**Statement.** Target's `addu $v1,$v0,$zero` in the beq delay slot requires an
explicit `s16 cur = id;` intermediate in the C.
**Mechanism (why it looked plausible).** Two distinct registers hold the id — the
`lh` destination used by the compare, and a copy consumed by
`sh …,-0x56($a1)`. That reads like two C variables.
**Probe.** Deleted the intermediate and stored `id` directly.
**Result.** Byte-identical output (36 insns, score 21 both ways). GCC generates
the copy itself from the HImode store of the compared value.
**Verdict.** KILLED — the intermediate is noise; do not re-add it.

### H5 — KILLED. Literal constants cannot replace the three holder locals in the
goto form.
**Statement.** `-1` / `3` / `1` can be written as literals, avoiding
constant-holder locals entirely.
**Mechanism (why it looked plausible).** In the `do-while` probe GCC's LICM did
hoist all three literals into loop-invariant registers, exactly as target holds
them in `$t4`/`$t3`/`$t2`.
**Probe.** Literals substituted into the goto form; sandbox.
**Result.** 35 insns (one instruction SHORT of target), score 23. Without a loop
note there is no LICM, and cse/combine folds one constant away instead of
holding it. Banked as `rejected/literal-constants-no-licm-in-goto-form.c`.
**Verdict.** KILLED for the goto form. NB this leaves the three holder locals
load-bearing, which means a future candidate-ready session must satisfy
`.claude/rules/named-local-fake-exception.md` (FAKE annotation + documented
lever exhaustion) or find a spelling that removes the need. Flagged, not
resolved, in s1.

---

## Live frontier (for the next session)

### F1 — Break the slot coupling by raising `slot`'s n_refs from 5 to 6.
**Mechanism.** `slot` needs priority above `tbl`'s 0.348 while keeping its
initialiser FIRST in the prologue (target emits `addiu $a2,$a0,0x8B4` as
prologue insn #1, we emit it last). At live_length 31, `n_refs 6` gives
`floor_log2(6)*6/31 = 12/31 = 0.387`, clearing both `tbl` (0.348) and `link`
(0.333) without moving the declaration. The `+=` counts as two refs, so today's
five are: init, `*slot = kind`, `slot += 0x68` (×2), post-loop terminator.
**Next probe.** Find a SEMANTIC sixth reference — i.e. one that a programmer
would write and that leaves the 36 emitted instructions unchanged. Candidates to
measure: writing the post-loop terminator through a `slot`-derived expression
that costs no extra insn; expressing the increment as `slot = slot + 0x68`
combined with a differently-spelled `*slot` store; or moving the `+0x01 = 0`
byte store from `ent - 0x57` onto `slot + 1` and checking whether maspsx/cc1
still emit it as `sb $zero,-0x57($a1)` (it must, or the probe is dead).
Reject anything whose only purpose is to add a reference — that is the
forbidden shape, not a lever.

### F2 — Invert the `ent`/`copy` pair (the last two register diffs).
**Mechanism.** Target wants `copy=$v1(3)`, `ent=$a1(5)`; we get the reverse
because `ent` (priority 1.50) outranks the GCC-generated copy pseudo (1.00) and
first-fit hands it the lower register. `ent` cannot be demoted (11 refs would
need live_length > 33; the loop body is 22 insns). So the copy pseudo must be
promoted above 1.50 — `n_refs 4 / live_length 3` gives 2.67.
**Next probe.** Determine from the `.greg` RTL what the copy pseudo's three
refs actually are, then look for a C spelling of the id handling that gives it a
fourth ref within the same 3-insn window without adding an instruction. A second
angle worth measuring first: confine the copy to a single basic block so
`local_alloc` (which runs before `global_alloc`) assigns it — a block-local
pseudo takes `$v1` early and blocks `ent` from reg 3 without any priority
fight. The `.lreg` dump distinguishes these: block-local pseudos print
"in block N" (82/83/85 do), the copy pseudo does not.

### F3 — A real-loop spelling that resists strength reduction.
**Mechanism.** H2 killed real loops because loop.c strength reduction re-bases
the displacement cluster. But a real loop would restore LICM and remove the need
for the three constant-holder locals (H5), which is the only cheat-adjacent
construct left in the candidate. If the two cursors are spelled so that
strength reduction finds nothing to reduce — e.g. genuine typed `struct *`
cursors with all displacements as real member offsets, so the address
expressions are already single-register+constant and there is no derived
induction variable to create — both problems close at once.
**Next probe.** Reconstruct the 0x68-byte element as a real struct (field map is
in evidence.md; matched sibling `func_80040400` in the same file writes the same
field set), declare both cursors as `struct * `, write a `for (i = 0; i < 18;
i++)` loop, and check the `.loop` dump for whether any new giv is created.

## [s1] The inherited pinned form's 37th instruction is a sll/sra sign-extension pair caused by declaring the -1 sentinel s16.
- mechanism: lh already sign-extends its destination, but an s16-vs-s16 comparison makes cc1 materialise a sign-extension of the register-held constant before the beq. Typing the sentinel s32 removes the pair and frees $v0 so the lh destination lands in $v0 as target has it.
- probe: s16 none -> s32 none in src/text1a.c; engine sandbox func_80040CB8 --disable all.
- result: 37 -> 36 build insns; emitted opcode/displacement sequence becomes identical to asm/funcs/func_80040CB8.s.
- verdict: CONFIRMED

## [s1] The loop must be written as a goto back-edge; any GCC-recognised loop form (for/while/do-while) cannot match.
- mechanism: A recognised loop emits NOTE_INSN_LOOP_BEG, enabling loop.c strength reduction, which invents a third induction pointer at arg0+0x8B8 for the -0x57..-0x4C displacement cluster and demotes arg0+0x90C to serving only the +0x58 store. That is three addiu rX,rX,0x68 increments per iteration against target's two.
- probe: Same body rewritten as do { ... } while (i < 0x12); sandbox --disable all; disassembly compared to target.
- result: 38 build insns, score 25 (vs 36 insns / score 13 for the goto form). Banked as rejected/do-while-loop-form-triggers-strength-reduction.c and tmp/grind/func_80040CB8/s1/build2.txt.
- verdict: CONFIRMED

## [s1] The whole remaining gap is an allocno-priority ordering problem, and declaration position is a real lever: declaring the 0x8B4 cursor LAST fixes seven of nine registers.
- mechanism: All ten long-lived pseudos mutually conflict and MIPS defines no REG_ALLOC_ORDER, so find_reg is ascending first-fit and each variable's register is decided solely by its rank under global.c:allocno_compare = floor_log2(n_refs)*n_refs/live_length. The 0x8B4 cursor scored 2*5/31 = 0.323, just under tbl (2*4/23 = 0.348) and link (2*4/24 = 0.333), so it was allocated fifth and got $t0 instead of $a2. Declaring it last drops its live_length below the 28.7 crossover and re-ranks it third.
- probe: Read cc1 -O2 -G0 -mcpu=3000 -mips1 -da .lreg/.greg dumps for per-pseudo n_refs/live_length and the ';; 11 regs to allocate' order line; then moved the 0x8B4 cursor's initialiser from first to last declaration and re-ran sandbox.
- result: Dump order was exactly 80 81 83 79 72 78 73 74 77 76 75, matching the computed priorities. After the move: score 21 -> 13, with slot=$a2, tbl=$a3, link=$t0, i=$t1, one=$t2, kind=$t3, none=$t4 all on target.
- verdict: CONFIRMED

## [s1] Target's addu $v1,$v0,$zero id copy requires an explicit s16 intermediate in the C source.
- mechanism: Two registers hold the id (the lh destination used by the compare, and a copy consumed by sh ...,-0x56($a1)), which reads like two C variables.
- probe: Deleted the s16 intermediate and stored id directly; sandbox --disable all.
- result: Byte-identical output both ways (36 insns, score 21). GCC generates the copy itself from the HImode store of the compared value.
- verdict: KILLED

## [s1] The three loop-invariant constants (-1, 3, 1) can be written as literals instead of constant-holder locals.
- mechanism: In the do-while probe LICM did hoist all three literals into loop-invariant registers exactly as target holds them in $t4/$t3/$t2, so literals looked sufficient.
- probe: Substituted literals for none/kind/one inside the required goto-loop form; sandbox --disable all.
- result: 35 build insns (one SHORT of target's 36), score 23. Without a loop note there is no LICM, and cse/combine folds one constant away rather than holding it. Banked as rejected/literal-constants-no-licm-in-goto-form.c.
- verdict: KILLED
