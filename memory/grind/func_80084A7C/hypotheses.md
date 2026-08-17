# Hypothesis ledger — func_80084A7C

## Session 1 (recon, 2026-08-17)

### H1 — CONFIRMED (and banked into candidate.c)
**Statement.** The `addu` operand-order mismatch at the eight `+0x98` flag
sites is a plain C operand-order artifact, not a codegen phenomenon: writing
`offset + *base_ptr + 0x98` instead of `*base_ptr + offset + 0x98` makes GCC
emit target's operand order.
**Mechanism.** GCC 2.7.2's RTL expansion preserves the source operand order of
a `PLUS` when neither side is a constant; no canonicalization pass reorders it
here because both operands are pseudos.
**Probe.** Edited all eight sites; re-ran `sandbox --disable all` and the
normalized disassembly diff.
**Result.** Score 26 → 26 (neutral), but every flag-site `addu` changed from
`addu $vX, $vX, $a3` to `addu $vX, $a1, $vX`, i.e. target's shape
`addu $vX, $a2, $vX` modulo register name. One diff axis eliminated; the
residual is now purely register naming.
**Verdict. CONFIRMED.** Keep this spelling in every future form.

### H2 — KILLED
**Statement.** Declaration order of `offset` vs `base_ptr` biases which of
`$a2`/`$a3` each one is allocated (the SOTN-sanctioned "named-intermediate
declaration order" / LUID lever).
**Mechanism.** Earlier-declared locals get lower LUIDs and are reached first by
`global.c`'s allocno ordering, which in principle steers hard-reg choice.
**Probe.** Declared `offset` first, both with and without the `shifted`
intermediate; sandbox + normalized diff on both.
**Result.** 26 → 32 and 26 → 33. The diff shows declaration order moved the
*emission order* of the two entry chains (the multiply chain floated above the
table-index chain, costing 5 extra top-of-function mismatches) and changed the
hard-register assignment **not at all**: `base_ptr` stayed in `$a2`, `offset`
stayed in `$a3` with the same `move $a1, $a3` coalesce failure, and every
downstream site was byte-for-byte the same as the base_ptr-first form.
**Verdict. KILLED.** Declaration order does not reach this allocation decision.
Banked as `rejected/decl-order-offset-before-base-ptr.c`. Do not re-propose.

### H3 — KILLED (inherited, re-confirmed by this session's diff)
**Statement.** Re-materializing `tbl[a0]` inline at each flag site (dropping the
`base_ptr` CSE handle) reaches target.
**Result.** Pre-session-1 WIP measured 39 vs 26 — regression. This session's
diff explains why: target *does* re-load `*base_ptr` per site, but it does so
through a **held pointer in `$a3`** (`lw $v1, 0($a3)`), which is exactly what
the `base_ptr` handle produces. Inlining the whole `&D_80106F28[a0]` address
computation at each site re-materializes the `lui/addiu/sll/sra` chain too.
**Verdict. KILLED.** Banked as `rejected/void-ptr-table-index-inline.c`.

### H4 — KILLED
**Statement.** Splitting the sign-extension out of the multiply
(`s32 idx = (s16)a1; s32 offset = idx * 0xB0;`) gives the `offset` allocno its
own named pseudo whose definition is the multiply chain's final `sll`, letting
it coalesce with the block-local destination and removing the `move $a1, $a3`.
**Mechanism.** A separately-named intermediate raises the chance that the
strength-reduced chain's last insn writes the variable's pseudo directly rather
than a temp that needs a join copy.
**Probe.** Applied the split; sandbox + normalized diff.
**Result.** Score 26 → 26 and the disassembly is **byte-for-byte identical** to
the un-split form — `cse`/`combine` fold `idx` away completely before allocation,
so no new pseudo ever exists. `move $a1, $a3` still present, `$a2`/`$a3` still
swapped.
**Verdict. KILLED.** Naming the sign-extension is invisible to allocation here.
(F3 below is superseded by this result — do not re-run it.)

## Live frontier for session 2+

The residual is one causal chain (see evidence.md): our build retires `$a1`
early via `move $s1, $a1`, global-alloc therefore parks the `offset` allocno in
`$a1` (costing a `move $a1, $a3` coalesce failure and the 146th instruction),
and the `$a2`/`$a3` pair ends up swapped relative to target. Attack the chain,
not the symptom. Register pins, hardcoded-`$N` asm, and anything else that
merely renames the registers are cheats and are out of scope by construction.

**F1 (highest value, and the mandated Step-0 diagnosis).** Run the
`cc1 -da` `.greg` dump for `main.c` and read `;; Register dispositions:` plus
the conflict lists for the `offset` and `base_ptr` allocnos. Establish
*why* `offset` is given `$a1`: is it an allocno-priority ordering
(`reg_n_refs` / live-length), or an explicit copy preference from
`expand_preferences`? The answer selects the lever. This is the recipe in
`.claude/rules/register-alloc-pure-c.md` and it has not been run on this
function. Needs a preprocessed `.i` — the sandbox does not keep one, so
reproduce the `cpp` invocation from `engine/buildconfig.py`.

**F2.** Attack the `s1 = a1` copy's placement directly. Target performs it
*after* `lw $v1, 0($a3)`; ours performs it before the whole entry chain. Try C
forms that keep the `a1` parameter live deeper into the entry block (e.g.
deriving `offset` from a *later* read of the parameter, or ordering the
`base`/`base_ptr` statements so the multiply chain's last use of `a1` sits
after the table load). If `$a1` is still occupied when global-alloc reaches
the `offset` allocno, it must take `$a2`, which is target's assignment and
also removes the copy.

**F3.** Attack the coalesce rather than the assignment: make the `offset`
allocno and the multiply chain's destination the same pseudo. Note H4 already
killed the cheapest spelling of this (a named `idx` intermediate is folded away
entirely), so the remaining shapes must change the *arithmetic*, not just the
naming — e.g. a form whose final operation is not the `sll ,4`
(`offset = (s16)a1 * 0x10 * 0xB` and similar re-associations), which changes
which insn defines the value that crosses the block boundary. Measure with
`diffit.py`, not the scalar score: H1/H4 both moved structure at a flat 26, so
the score alone will mislead you on this function.

## [s1] The addu operand-order mismatch at the eight +0x98 flag sites is a plain C operand-order artifact: writing 'offset + *base_ptr + 0x98' instead of '*base_ptr + offset + 0x98' makes GCC emit target's operand order.
- mechanism: GCC 2.7.2 RTL expansion preserves source operand order of a PLUS when both operands are pseudos; no canonicalization pass reorders it here.
- probe: Edited all eight flag-site expressions; re-ran 'sandbox func_80084A7C --disable all' plus a normalized disassembly diff (tmp/grind/func_80084A7C/s1/diffit.py) against asm/funcs/func_80084A7C.s.
- result: Score flat at 26, but every flag-site addu changed from 'addu $vX,$vX,$a3' to 'addu $vX,$a1,$vX' — i.e. target's shape 'addu $vX,$a2,$vX' modulo register name. One whole diff axis removed; the residual is now purely register naming. Banked in candidate.c.
- verdict: CONFIRMED

## [s1] Declaration order of 'offset' vs 'base_ptr' biases which of $a2/$a3 each is allocated (the SOTN-sanctioned named-intermediate declaration-order / LUID lever).
- mechanism: Earlier-declared locals get lower LUIDs and are reached earlier by global.c's allocno ordering, which in principle steers hard-reg choice.
- probe: Declared 'offset' before 'base_ptr', both with and without the 'shifted' intermediate; sandbox + normalized diff on each.
- result: 26 -> 32 (with 'shifted') and 26 -> 33 (without). The diff shows the change moved only the EMISSION ORDER of the two entry chains (the *0xB0 multiply chain floated above the table-index chain, costing 5 extra top-of-function mismatches) and changed the hard-register assignment not at all: base_ptr still $a2, offset still $a3 plus the same 'move $a1,$a3', all downstream sites byte-identical. Banked as rejected/decl-order-offset-before-base-ptr.c.
- verdict: KILLED

## [s1] Splitting the sign-extension out of the multiply (s32 idx = (s16)a1; s32 offset = idx * 0xB0;) gives the offset allocno a pseudo defined directly by the multiply chain's final sll, letting it coalesce and removing the extra 'move $a1,$a3'.
- mechanism: A separately-named intermediate can make the strength-reduced chain's last insn write the variable's pseudo rather than a block-local temp needing a join copy.
- probe: Applied the split in src/main.c; sandbox --disable all + normalized disassembly diff.
- result: Score 26 -> 26 and the emitted code is byte-for-byte IDENTICAL to the unsplit form — cse/combine fold 'idx' away before allocation so no new pseudo ever exists. 'move $a1,$a3' still present, $a2/$a3 still swapped. Reverted.
- verdict: KILLED

## Session 2 (structural, 2026-08-17)

### H5 — CONFIRMED (the F1 diagnosis)
**Statement.** The `offset` allocno is given `$a1` for a knowable reason that a
`.greg` dump names, and that reason selects the lever.
**Mechanism.** GCC 2.7.2 `global.c` assigns allocnos in priority order honouring
`expand_preferences` copy hints and the per-allocno hard-reg conflict sets.
**Probe.** Built `tmp/grind/func_80084A7C/s2/greg.py` (pipeline-faithful cpp +
`cc1 -da`, function section extracted) and diffed the plain form against a form
that fixes the allocation.
**Result.** Allocno 82 is `offset`. Plain: `;; 82 conflicts: ... 2 3 4 29` — no
conflict with hard reg 5 and no preference line — so it takes `$a1`, and the
multiply chain's final `sll` (block-local, local-alloc gave it `$a3`) needs the
join copy `move $a1,$a3`. Fixed: `;; 82 conflicts: ... 2 3 4 5 29` plus
`;; 82 preferences: 6` — it takes `$a2`, target's register, and the copy is
gone. Confirms session 1's causal reading exactly: it is ONE decision, and both
halves of the residual follow from it.
**Verdict. CONFIRMED.**

### H6 — CONFIRMED (banked, natural, keep unconditionally)
**Statement.** BB2's own matched sibling `spu_SetMotionState` (src/main.c:303),
which reads the same `D_80106F28` table with the same 0xB0 stride, spells the
entry in the ORIGINAL's idiom, and adopting it verbatim fixes the table-address
load's schedule slot.
**Mechanism.** The two named intermediates (`s32 shifted`, `s32 *addr`) change
the LUID/emission order enough that sched places the symbol-address load
between `sll v0,a0,16` and `sra v0,v0,14`, which is target's order.
**Probe.** cc1-only sweep over EIGHT address spellings (`sweep3.py`) plus the
sibling cross-product (`sweep5.py`); sandbox confirmation.
**Result.** All eight non-sibling spellings — index form, pointer add, reversed
PLUS operand order, `* 4`, `<< 2`, a named `tbl` alone, a named `idx` alone,
the current inline form — emit BYTE-IDENTICAL code (cse/combine canonicalises
the address expression). Only the two-intermediate sibling idiom moves the
`la`; `addr` alone overshoots (la at index 1, before the `sll`). Worth 26 -> 24
on its own, and it is a prerequisite for reaching 0.
**Verdict. CONFIRMED.** In candidate.c and in the no-cheat fallback form.

### H7 — CONFIRMED-BUT-UNSANCTIONED (the ruling question)
**Statement.** Re-associating / re-spelling the `*0xB0` chain changes which
insn defines the value crossing the entry-block boundary and can hand the
`offset` allocno target's register (session 1's F3).
**Mechanism.** Extra RTL temporaries in the offset expression shift pseudo and
allocno numbering; combine then folds the arithmetic back, so the emitted chain
can be unchanged while `global.c`'s conflict/preference tables differ.
**Probe.** cc1-only sweep over 18 offset spellings (`sweep2.py`, `sweep6.py`),
fingerprinting the chain's final destination register and the presence of
`move $5,$7`; sandbox on the winners.
**Result.** EIGHT spellings reach the target allocation, but only the negation
pair (`-((s16)a1 * -0xB0)` / `-(-0xB0 * (s16)a1)`) ALSO leaves the emitted
multiply chain byte-identical to target; the rest change the arithmetic and
cost 5-11 points of chain mismatch. With the sibling idiom + the negation the
honest sandbox distance is **0** (145 insns == target). Every NATURAL spelling
tested (`(s16)a1 * 0xB0`, `a1 * 0xB0`, `(s32)a1 * 0xB0`, `(s16)(u16)a1 * 0xB0`,
a named `s16 ch`, a named `s32 idx`, and four factorisations `0xB*0x10`,
`0x58*2`, `0x2C*4`, `0x16*8`) leaves the allocno in `$a1`.
**Verdict. CONFIRMED as a byte-lever, NOT SUBMITTED.** The negation pair has no
observable effect, no human-programmer rationale, and a GCC-internals-only
mechanism; no sanctioned family covers an inline arithmetic no-op used as an RA
lever (the nearest, "opaque arithmetic variables", is a NAMED variable
defeating a bit-test transform). First reach => `ruling-request`.

### H8 — KILLED
**Statement.** Declaring `base` and assigning it in a separate statement (the
sibling's `u8 *entry; entry = ...;` shape) is an available structural lever.
**Probe.** cc1-only sweep + one sandbox run.
**Result.** In THIS function the assignment would sit ahead of the remaining
declarations (`s32 val; u32 threshold;`), which is a C89 violation: cc1 stops
and emits a 34-instruction stub (sandbox 135). Not a lever here — and any
future sweep row reporting ~34 build_insns is this parse failure, not codegen.
**Verdict. KILLED.**

## Live frontier for session 3+

**F1 (the whole question).** Does the owner accept `-((s16)a1 * -0xB0)`? The
bytes are proven at distance 0 with it. If ACCEPTED (presumably as a
FAKE-annotated last-resort inside a named family), the function closes
immediately: apply `memory/grind/func_80084A7C/candidate.c`, add the required
`/* FAKE: ... */` annotation, self-vet, and submit.

**F2 (if the negation is REFUSED).** Resume from
`rejected/natural-offset-no-negation-floor24.c` (floor 24, entry schedule
already target-correct). The remaining problem is stated precisely: make the
`offset` allocno conflict with hard reg `$a1` — equivalently, keep the
parameter `$a1` live past the offset def, i.e. make the `s1 = a1` copy schedule
AFTER `lw $v1,0($a3)` as target does — or give it a copy preference for `$a2`,
using only natural C. Untried angles: change what the later `a1` reads look
like (the two `spu_NotifyChannel((s16)(a0 | (a1 << 8)))` sites are the only
consumers of the saved copy — e.g. compute the channel word once into a named
local, or read it from a different expression shape), and change the callee-save
pressure so the copy's sched priority differs.

**F3.** The sibling `spu_SetMotionState` is a matched, ORIGINAL-STYLE reference
for this whole table-access family and it was not consulted before session 2.
Any future BB2 function touching `D_80106F28` should start by copying its
idiom; likewise check for other matched siblings before sweeping spellings.

## Session 3 (structural, 2026-08-17)

### H9 — CONFIRMED (the closing hypothesis)
**Statement.** The residual is an allocno-PRIORITY TIE between the `offset` and
`base_ptr` global allocnos, not a property of either value; therefore removing
one of the two C handles — deleting the `offset` local and writing the stride
multiply at its use sites — removes the tie and hands the multiply's value
target's register with no join copy.
**Mechanism.** `global.c`'s `allocno_compare` ranks allocnos by
`floor_log2(n_refs) * n_refs / live_length * size` and falls back to the raw
allocno NUMBER when priorities are equal. `offset` and `base_ptr` each carried
about ten refs across the same span, so the tie decided the hard-reg order:
`offset` was reached while $a1 was free (our early `move $s1,$a1` retires the
parameter) and took it, forcing the failed coalesce with the multiply chain's
block-local $a3 destination (`move $a1,$a3`, the 146th instruction) and the
$a2/$a3 swap. With no named holder the multiply's value is a single-def quantity
whose definition IS the chain's final `sll`; it lands in $a2 (target's register),
`base_ptr` keeps $a3, and there is no copy insn to delete.
**Probe.** 50 forms measured cc1-only against a gold asm (session 2's distance-0
output, whose bytes are target's) with `tmp/grind/func_80084A7C/s3/sweep.py`;
the winner confirmed with `sandbox func_80084A7C --disable all` and with the s1
normalized objdump diff.
**Result.** Sandbox score **0**, build_insns 145 == target_insns 145; objdump
diff clean apart from the unlinked object's unresolved `D_80106F28` relocation
pair. Three other forms also reach 0 (base computed from `(offset << 4)` with
`offset` finalised afterwards; the multiply duplicated at `base` and at the
`offset` holder; `offset` holding `a1*0xB` with `<< 4` at every use) — the
submitted form is the one that ADDS nothing and reads as ordinary C.
**Verdict. CONFIRMED.** In `candidate.c`; self-vet written.

### H10 — CONFIRMED as a mechanism, superseded as a submission
**Statement.** Same-variable split-init staging of `offset`
(`offset = (s16)a1; offset = offset * 0xB0;`) shortens the variable's live range
enough to lift its `allocno_compare` priority above `base_ptr`'s and fix the
$a2/$a3 swap.
**Result.** Confirmed by the `.greg` dispositions (`82 in 6 / 80 in 7` — offset
in $a2, base_ptr in $a3 — and no `move $5,$7`), but the two-def structure costs
one extra instruction: the chain's final `sll` writes a temp that is then copied
into the variable, and copy-propagation rewrote only the two early-branch uses,
so both pseudos stay live and the copy survives. Seven spellings of the staging
measured, all with the same 1-insn residual. A SEPARATE stage variable
(`s32 idx = (s16)a1;`) is completely inert, so the lever is the two-def
structure, not the extra name.
**Verdict. CONFIRMED (mechanism), NOT SUBMITTED** — H9 closes with no extra
instruction. Banked as `rejected/offset-split-init-two-def-join-copy.c`.

### H11 — KILLED
**Statement.** Declaring the signature the three call sites actually use
(`void func_80084A7C(s16 a0, s16 a1, u8 a2, u8 *a3)`, per the
`(void (*)(s16,s16,u8,u8 *))` casts at main.c:451,503,547) makes the incoming
$a2/$a3 hard regs live-in and changes the allocation.
**Result.** BYTE-IDENTICAL to the two-parameter form (golddiff 45 == baseline,
same join copy, same swap). Unused parameters never acquire a live range here.
**Verdict. KILLED.** Banked as `rejected/four-param-signature-inert.c`; confirms
session 1's note that the prototype contradiction is not part of the residual.

### H12 — KILLED
**Statement.** The remaining structural axes on the entry block reach the
allocation: statement/declaration order, base as a pointer add or offset-first,
a `u8 *` entry handle, `u32` type narrowing, re-deriving `base_ptr` from `addr`
at the flag sites, and the sibling `_SsSeqPlay` one-expression base.
**Result.** Every one is byte-identical to the baseline (golddiff 45);
declaration-order permutations measure 45-51 and are never better.
cse/combine canonicalise the address expression, exactly as session 2 found for
the eight address spellings.
**Verdict. KILLED.** Do not re-sweep these axes on this function.

## Live frontier for session 4+

None: the honest floor is 0 in natural C and the form is submitted as
`candidate-ready`. If layer 1 or the Judge FAILs it, the fallback ladder is the
other three 0-diff forms in evidence.md's table (same allocation, different
spelling — each adds something the submitted form does not, so they are strictly
worse on the checklist), then H10's split-init form, which is one instruction
short. Integration note for the operator: the 11 regfix rules at
`regfix.txt:719-742` exist only to paper over the residual this form removes, so
`retire func_80084A7C` should drop all 11.
