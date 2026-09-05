# hypotheses — func_80034F88

## Resolved this session (s1, recon)

### H1 — KILLED. "Int-typed temporaries force the `lbu` reloads."
*Statement.* The target reloads `D_80106A73` after each store because the
source reads it into an `int`, making the read a `(zero_extend:SI (mem:QI))`
rtx that cse.c cannot satisfy from the QImode value it just stored.
*Mechanism.* cse.c hash lookup is on the whole SET_SRC rtx, so a zero_extend
wrapper would miss the recorded `(mem:QI)` equivalence.
*Probe.* Rewrote the body with `s32 val, val2` and plain symbol accesses;
`sandbox --disable all`.
*Result.* Score 28 (worse than the 24 baseline), 47 insns, objdump shows a
single `lbu` for the entire flag section — the forward still happened.
**KILLED.** Banked at `rejected/symbol-only-int-temps-score28.c`.

### H2 — KILLED (and the axis is permanently closed).
*Statement.* "`D_80106A73` was `volatile` in the original; that is why every
read is a real `lbu` and the address is rematerialized per block."
*Mechanism.* Volatile mems are excluded from cse's value table.
*Probe.* Changed `src/code6cac_b.c:127` to `extern volatile u8 D_80106A73;`
with the plain-symbol body; `sandbox --disable all`.
*Result.* Score 29 and the objdump still showed only ONE `lbu` — because the
cheat-invisible sandbox **strips the `volatile` qualifier** before compiling
(confirmed: `tmp/sandbox/func_80034F88/src/code6cac_b.c:127` reads
`extern u8 D_80106A73;`). A volatile spelling is score-inert here, so it can
never move the honest floor no matter what the two-prong game-state gate would
have said about it. **KILLED — do not re-probe volatile on this function.**

### H3 — CONFIRMED (this is the 24 → 23 drop).
*Statement.* The target's post-store `lbu` reloads are reproducible in pure C
by making the store and the read use *different address expressions* for the
same byte.
*Mechanism.* `cse.c` records a store as an equivalence keyed on the canonical
address rtx; a later load only picks it up if its address hashes identically.
A store through a pointer local and a read through the plain symbol do not.
*Probe.* `*ptr &= 0xF8;` (pointer) followed by `val = D_80106A73;` (symbol).
*Result.* The reload materialises (`sb a0,0(v1)` … `lui a0; lbu a0,0(a0)`),
score 24 → **23**. Extending the mismatch to all three blocks materialises all
three reloads (rejected/ptr-stores-symbol-reads-score24.c). **CONFIRMED.**

## Open frontier (for the next session)

### F1 — the central question. How does target get a SHARED base register for
a load/store pair of the same byte *without* the store being forwarded into
the load?
*Why it matters.* Every remaining point of distance reduces to this. Target
spends `lui`+`addiu` once per block and then uses `0(base)` for both the
reload and the store; our fork only leaves the base unfolded when the address
expression has ≥2 memory uses, and the moment it does, cse forwards the store
into the load and the reload disappears. Solving F1 also fixes the load-delay
`nop` for free (the reload sinks into the `lw v0,0x20(a1)` delay slot once its
base is already live).
*Mechanism to investigate.* Read `tools/gcc-2.7.2/cse.c` and determine the
exact condition under which `cse_insn` DECLINES to record a MEM destination in
the value table (candidates seen in other GCC 2.7.x: the SET_SRC being a
`subreg`, a mode mismatch between the recorded value and the QImode mem, a
`zero_extract` destination from a bitfield insert, or `do_not_record` being
set by the address). Whichever condition it is, find the C construct that
triggers it while leaving the address a single shared pseudo. A bitfield
destination is a live candidate worth testing early: a bitfield insert into a
`u8` container is a `zero_extract` SET_DEST, which several GCC 2.7.x cse
implementations invalidate without recording — and the 0xF8 mask plus three
single-bit flags is exactly the shape of a `:1,:1,:1,:5` bitfield container.
*Next probe.* (a) `grep -n "in_memory\|invalidate\|GET_CODE (dest) == MEM"
tools/gcc-2.7.2/cse.c` and read `cse_insn`'s dest-recording block; then
(b) declare the byte as a 3×1-bit + 5-bit bitfield struct at 0x80106A73 and
measure. Note the fork's bitfields are allocated HIGH-first
(`.claude/rules/bitfield-direction-divergence.md`), so bit0 of the mask
0xF8/0x01/0x02/0x04 constrains the field order — get that right before
concluding a bitfield spelling failed.

### F2 — the `-da` RTL dump has not been run for this function at all.
*Why it matters.* Everything above was inferred from objdump. One `cc1 -da`
run on the sandbox `.i` gives the `.cse`, `.combine` and `.greg` dumps, which
would answer F1 directly (does the mem get recorded? is the fold done by
combine or by the expander?) instead of by hypothesis.
*Next probe.* Reuse `tmp/sandbox/func_80034F88/` inputs, run cc1 with the
build flags plus `-da`, dump into `tmp/grind/func_80034F88/s2/`, and read the
`.cse`/`.combine` dumps around the three `sb` insns. This is cheap and should
be done BEFORE any more blind spelling sweeps.

### F3 — re-derive 0x80106A70..0x80106A73 as one object.
*Why it matters.* `src/code6cac_b.c:4044` already spells
`src1 = (&D_80106A73) - 3;`, so the surrounding code treats the four bytes as
one contiguous region; the current per-byte `extern u8` decls may simply be
splat artefacts. A single declared object changes every address expression in
the function at once and is the only untried change that touches both the flag
blocks and the copy loop.
*Caveat already measured.* Target addresses the flag byte as
`%hi/%lo(D_80106A73)` at offset 0 and the copy destination as
`%lo(D_80106A70)($at)` — so if it is one object, the compiler still emitted
two independent addressings, and a naive `g[3]` spelling will produce
`3(base)` displacements that target does not have. Check that before investing.

## [s1] The target's post-store lbu reloads of D_80106A73 exist because the source reads the byte into an int-typed intermediate, making the read a (zero_extend:SI (mem:QI)) rtx that cse.c cannot satisfy from the QImode value it just stored.
- mechanism: GCC 2.7.2 cse.c hashes the whole SET_SRC rtx, so a zero_extend wrapper would miss the recorded (mem:QI) equivalence and force a real load.
- probe: Rewrote the body with s32 val/val2 and plain-symbol accesses; sandbox func_80034F88 --disable all; objdump of tmp/sandbox/func_80034F88/code6cac_b.o.
- result: Score 28 (worse than the 24 baseline), 47 build insns, and the objdump still shows exactly ONE lbu for the whole flag section - the store was still forwarded into every read. Integer typing of the temporaries is not the lever.
- verdict: KILLED

## [s1] D_80106A73 was volatile in the original source, which is why the target reloads it after every store and rematerialises its address per block.
- mechanism: Volatile mems are excluded from cse.c's value table, so no store-to-load forwarding and no address CSE.
- probe: Changed src/code6cac_b.c:127 to 'extern volatile u8 D_80106A73;' with the plain-symbol body; sandbox func_80034F88 --disable all; then grepped the sandbox's own source snapshot.
- result: Score 29 and still only one lbu. Root cause: the cheat-invisible sandbox STRIPS the volatile qualifier before compiling - tmp/sandbox/func_80034F88/src/code6cac_b.c:127 reads 'extern u8 D_80106A73;'. A volatile spelling is therefore score-inert on this function and can never move the honest floor, independent of the game-state two-prong gate. Axis permanently closed.
- verdict: KILLED

## [s1] The target's lbu reloads are reproducible in pure C by giving the store and the subsequent read DIFFERENT address expressions for the same byte.
- mechanism: cse.c records a store as an equivalence keyed on the canonical address rtx; a later load only picks that equivalence up if its own address hashes identically. A store through a pointer local and a read through the plain symbol do not hash the same, so the load survives.
- probe: '*ptr &= 0xF8;' (pointer store) followed by 'val = D_80106A73;' (symbol read); sandbox + objdump. Then extended the same mismatch to all three flag blocks as a second measurement.
- result: The reload materialises exactly as in target ('sb a0,0(v1)' then 'lui a0; lbu a0,0(a0)') and the floor drops 24 -> 23 (50 build insns vs target 49). Extending the mismatch to all three blocks yields all three reloads but scores 24, because each read then pays its own folded lui and leaves a nop in the lw load-delay slot.
- verdict: CONFIRMED

## Resolved in s2 (structural)

### s1-F1 — RESOLVED / CONFIRMED. "How does target get a shared address base
### for a load/store pair of the same byte without the store being forwarded?"
*Answer.* The mem is VOLATILE. `canon_hash` sets `do_not_record` for a
`MEM_VOLATILE_P` rtx, so `sets[i].src_elt` is 0 for the store, and the
dest-recording block at `tools/gcc-2.7.2/cse.c:7329` skips recording the
destination — the stored value never enters the value table, so the next read
is a real `lbu`. The ADDRESS pseudo is an ordinary non-volatile
`(set (reg) (symbol_ref))`, stays in the table, and is still shared. The two
effects s1 measured as mutually exclusive are only exclusive for NON-volatile
mems.
*Probe.* `x2_single_volptr.c` — `volatile u8 *pbit = &D_80106A73;` used for the
`&= 0xF8` and all three flag blocks. Sandbox + objdump.
*Result.* All FOUR `lbu` reloads materialised for the first time (s1's best had
one), on a single shared unfolded `lui`+`addiu` base. Score 28 at that point
only because the volatile read was placed before the `p[8]` read and stranded a
`nop` in each load-delay slot. **CONFIRMED.**

### s2-H1 — CONFIRMED. Ordering the `p[8]` read before the volatile read fills
### the three load-delay slots.
*Mechanism.* The scheduler will not move a non-volatile load across a volatile
one, so source order pins the `lw`/`lbu` order.
*Probe.* `y1_volptr_cond_first.c` / `z3_volptr_ternary.c` — `c = p[8] & K;`
before `val = *pbit;`.
*Result.* 48 insns with three `nop`s -> 45 insns with none; score 28 -> 15 for
the ternary spelling. **CONFIRMED.**

### s2-H2 — CONFIRMED. Target's three `lui`+`addiu` bases come from three
### separate pointer LOCALS, not from cse basic-block boundaries.
*Mechanism considered and rejected.* `cse_end_of_basic_block` (cse.c:8039)
does end a block at a CODE_LABEL, but lines 8102-8184 extend the block straight
through a conditional branch that skips a block whenever
`LABEL_NUSES (JUMP_LABEL (p)) == 1` — which is the case for all three join
labels. The .cse dump confirms the merge happens across them.
*Probe.* `e1_three_volptrs.c` / `f1_staged_ptrs_before_store.c` — three
distinct `volatile u8 *` locals, each assigned `&D_80106A73` immediately before
the PREVIOUS block's store.
*Result.* Three `lui`+`addiu` pairs materialise in target's positions; build
insn count 45 -> **49, exactly target's 49**; score 15 -> **12**. Assigning the
next pointer before the CONDITION instead of before the STORE scores 18, so the
placement is load-bearing. **CONFIRMED.**

### s2-H3 — KILLED. "The flag byte is a `:1,:1,:1,:5` bitfield container, and a
### ZERO_EXTRACT SET_DEST is what suppresses cse's dest recording."
*Mechanism.* cse.c:7004-7027 does set `sets[i].src_elt = 0` for a
ZERO_EXTRACT/SIGN_EXTRACT destination — the hypothesis was well founded.
*Probe.* `v06_bitfield.c`, HIGH-first field order (`u8 rest:5; u8 b2:1;
u8 b1:1; u8 b0:1;`) so the masks land on 4/2/1 per
`.claude/rules/bitfield-direction-divergence.md`.
*Result.* Score **44** at 64 insns — the worst form measured all session. Two
reasons the carve-out cannot apply: the `src_const` exception at 7009-7016
records CONSTANT bitfield stores anyway, and MIPS has no memory `insv`, so each
field write expands into an and/or read-modify-write pair that the target does
not contain. **KILLED.** `rejected/bitfield-container-score44.c`.

### s2-H4 — KILLED. `do { ... } while (0)` wrapping as a cse-block boundary.
*Mechanism.* `cse_end_of_basic_block` breaks at `NOTE_INSN_LOOP_END` while
`!after_loop` (cse.c:8054), so a loop note between blocks should reset the
value table.
*Probe.* `d1` (per flag block), `d3` (around the whole flag section),
`d2` (per block, with block-scoped volatile pointers).
*Result.* 26, 24 and 30 respectively — all worse than the 15/12 forms the same
session. **KILLED.** `rejected/dowhile0-per-block-wrap-score26.c`.

### s1-F3 — KILLED without a measurement, by the target's own relocations.
The flag accesses carry `R_MIPS_HI16/LO16 D_80106A73` and the copy-loop store
carries `R_MIPS_HI16/LO16 D_80106A70`. Two distinct symbols in the shipped
relocation records means the original source declared two distinct objects;
`src/code6cac_b.c:4044`'s `(&D_80106A73) - 3` is pointer arithmetic between
them, not evidence of a single 4-byte object. **KILLED.**

## Open frontier for s3

### F1 (BLOCKING, and it is a POLICY question, not a codegen one) — is
### `volatile u8 *pbit = &D_80106A73;` a sanctioned construct or a cheat?
Everything below F2 is downstream of this. The whole 23 -> 12 drop rests on it.
Evidence both ways is written up at the end of evidence.md. Short version: the
engine's `volatile_cheats` detector does not strip it (measured), it is none of
the three patterns that module implements, and the sibling function at
src/code6cac_b.c:4030 already spells it this way — but the frozen catalog does
list "volatile-coercion ... by cast", and this is arguably that intent
respelled. D_80106A73 is a game flag byte with three dedicated bit-getters
(named_syms.txt:413-415); whether it clears the `extern volatile T G;`
two-prong IRQ-touched-game-state gate has not been researched.
*Next probe.* Not a measurement — a ruling. If it is sanctioned, F2 is a short
register-allocation grind from a form that already has the right 49
instructions. If it is not, s1's 23 stands as the honest floor and the whole
volatile family has to be discarded, which also re-opens s1-F1 as unanswered.

### F2 — close the last 12 points: register assignment only.
Target: base `v1` for the `&=0xF8` and bit-1 blocks, `a0` for bit-2 and bit-4;
the loaded byte in `a0`,`a0`,`v1`,`v1`; `p` in `a1`; the selected value in `v0`.
Current build (`f1_staged_ptrs_before_store.c`): base `a0` throughout, byte in
`v1`, `p` in `a1`, value in `v0`. So `p` and the value register are already
right and only the base/byte pair is swapped for the first two blocks.
*Mechanism.* In target the FIRST base stays live in `v1` across the bit-1
block's store while the second base is materialised into `a0` ahead of that
store; our allocator reuses `a0` for the new base, forcing the pair to come
after the `sb`. This is an allocno-ordering difference driven by live ranges.
*Next probe.* Vary the live range of the first pointer local — e.g. give
`pbit` a later use, reorder the local DECLARATION order (declaration order
feeds allocno numbering), or move the `pbit2`/`pbit3` assignments one statement
earlier/later — and diff the `.greg` dump against a `-da` run. The harness for
all of this already exists: `tmp/grind/func_80034F88/s2/{sweep,inspect,
install,dump_rtl.sh}`.

### F3 — the permuter has never been run on this function.
With a form that is already at target's exact instruction count and differs
only in register assignment, `tools/permuter_campaign.py` is now a
well-conditioned search rather than a shot in the dark. It was not attempted in
s1 or s2. Gate it behind F1.

## [s2] s1's central open question F1 — the target gets a shared unfolded lui/addiu base for a load/store pair of the same byte WITHOUT the store being forwarded into the load because the memory access is volatile.
- mechanism: GCC 2.7.2 canon_hash sets do_not_record for a MEM with MEM_VOLATILE_P, so sets[i].src_elt stays 0 for a volatile store; the dest-recording block at tools/gcc-2.7.2/cse.c:7308-7340 skips recording the SET_DEST when sets[i].src_elt == 0 (line 7329). The stored value therefore never enters cse's value table and the next read of the byte survives as a real lbu. The ADDRESS is a separate, non-volatile (set (reg) (symbol_ref)) insn, so it stays in the table and is still shared between the load and the store. The two effects s1 measured as mutually exclusive are only exclusive for non-volatile mems.
- probe: Variant x2_single_volptr.c — 'volatile u8 *pbit = &D_80106A73;' used for the &= 0xF8 and all three flag blocks; sandbox func_80034F88 --disable all plus objdump of tmp/sandbox/func_80034F88/code6cac_b.o. Cross-checked against a cc1 -da RTL dump of the non-volatile form (tmp/grind/func_80034F88/s2/rtl/v10/), whose .cse dump shows the four address pseudos merged and the reloads deleted.
- result: All FOUR lbu reloads of D_80106A73 materialised for the first time (s1's best form had exactly one), on a single shared unfolded lui+addiu base — precisely the combination s1 recorded as unreachable. Score 28 at that point only because the volatile read preceded the p[8] read and stranded a nop in each load-delay slot.
- verdict: CONFIRMED

## [s2] Reading p[8] into a temporary BEFORE the volatile flag-byte read fills the three lw load-delay slots that otherwise carry nops.
- mechanism: GCC's scheduler will not move a non-volatile load across a volatile one, so source order pins the relative order of 'lw v0,0x20(a1)' and the volatile 'lbu'. With the volatile read first the lbu cannot sink into the lw's delay slot; with the condition read first it can, which is what the target does.
- probe: Variants y1_volptr_cond_first.c and z3_volptr_ternary.c — 'c = p[8] & K;' emitted before 'val = *pbit;' in every flag block.
- result: 48 build insns with three nops -> 45 build insns with none; score 28 -> 15 for the ternary spelling of the value select (the if/else spelling ties at 15; the s1-era 'val2 = val|K; if (!(p[8]&K)) val2 = val;' spelling scores 27 at the same 45 insns, so the two levers interact).
- verdict: CONFIRMED

## [s2] The target's THREE lui+addiu materialisations of &D_80106A73 come from three separate pointer locals in the source, not from cse basic-block boundaries at the three join labels.
- mechanism: cse_end_of_basic_block (tools/gcc-2.7.2/cse.c:8039) does terminate a block at a CODE_LABEL, but lines 8102-8184 extend the block straight through a conditional branch that skips a block whenever LABEL_NUSES (JUMP_LABEL (p)) == 1 — true for all three join labels here — so the labels are not cse boundaries. Separate C pointer locals, each assigned immediately before the PREVIOUS block's store, put the address computation where the target has it.
- probe: Variants e1_three_volptrs.c and f1_staged_ptrs_before_store.c (three distinct 'volatile u8 *' locals) against z3_volptr_ternary.c (one local), plus the .cse dump at tmp/grind/func_80034F88/s2/rtl/v10/code6cac_b.i.cse showing insns 27/55/83 deleted and the four address pseudos merged into reg 74 across the labels.
- result: Three lui+addiu pairs materialise in the target's positions; build insn count 45 -> 49, exactly the target's 49; score 15 -> 12. Placement is load-bearing: assigning the next block's pointer before the CURRENT block's store scores 12, assigning it before the condition instead scores 18.
- verdict: CONFIRMED

## [s2] s1's leading F1 candidate — the flag byte is a ':1,:1,:1,:5' bitfield container and a ZERO_EXTRACT SET_DEST is what suppresses cse's dest recording.
- mechanism: cse.c:7004-7027 genuinely does set sets[i].src_elt = 0 for a ZERO_EXTRACT/SIGN_EXTRACT destination, so the hypothesis was well founded; but the src_const exception at lines 7009-7016 records CONSTANT bitfield stores anyway, and MIPS has no memory insv pattern, so every field write expands into an and/or read-modify-write that the target does not contain.
- probe: Variant v06_bitfield.c with HIGH-first field order ('u8 rest:5; u8 b2:1; u8 b1:1; u8 b0:1;') so the masks land on 4/2/1 per .claude/rules/bitfield-direction-divergence.md; sandbox --disable all.
- result: Score 44 at 64 build insns — the worst form measured all session, roughly double the target's instruction count. Bitfield re-derivation of D_80106A73 is dead.
- verdict: KILLED

## [s2] Wrapping each flag block in 'do { ... } while (0)' creates a cse basic-block boundary (NOTE_INSN_LOOP_END) that stops the store-to-load forwarding.
- mechanism: cse_end_of_basic_block breaks at NOTE_INSN_LOOP_END while !after_loop (cse.c:8054), and the first cse pass runs before loop.c, so a loop note between blocks should reset the value table.
- probe: Variants d1 (wrap per flag block), d3 (wrap the whole flag section), d2 (per block plus block-scoped volatile pointers); sandbox --disable all.
- result: 26, 24 and 30 respectively — all substantially worse than the 15 and 12 achieved the same session by the volatile forms. The do-while(0) axis is dead on this function.
- verdict: KILLED

## [s2] s1's F3 — 0x80106A70..0x80106A73 was one declared object (a 4-byte array or struct) in the original source.
- mechanism: A single declared object would give one base for the whole region and change every address expression in the function at once.
- probe: Read the target's own relocation records in asm/funcs/func_80034F88.s instead of measuring a spelling.
- result: Refuted directly by the shipped relocations: the flag accesses carry R_MIPS_HI16/LO16 against D_80106A73 while the copy-loop store carries R_MIPS_HI16/LO16 against D_80106A70. Two distinct symbols in the relocation records means the original source declared two distinct objects; src/code6cac_b.c:4044's '(&D_80106A73) - 3' is pointer arithmetic between them, not evidence of one object. No measurement needed.
- verdict: KILLED

## [s2] Non-volatile C spellings can reach below the s1 floor of 23.
- mechanism: Structural levers from the codegen-technique-index: block-local variable splits, block scoping, declaration order, pointer/symbol read-store mixes, type narrowing (u8 vs s32 temps), ternary vs if/else statement re-association.
- probe: 15 non-volatile variants measured through tmp/grind/func_80034F88/s2/sweep.py: v01-v10 and w1-w7.
- result: Best non-volatile result is 23, matching s1's floor, reached by two structurally different forms (v05 at 50 insns, v10/w4 at 45 insns). v10 achieved the shared unfolded base for the first time but forwarded all three stores; nothing non-volatile produced more than one lbu reload. The non-volatile family is exhausted at 23.
- verdict: KILLED

## Resolved in s3 (structural)

### s2-F1 — RESOLVED AGAINST THE CONSTRUCT (policy, not codegen).
*Question.* Is `volatile u8 *pbit = &D_80106A73;` — a local pointer-to-volatile
aliasing a global declared plain `extern u8` — sanctioned or a cheat?
*Answer.* A CHEAT, resolved from the project's own artifacts without an owner
ruling. It is `engine/volatile_cheats.py` pattern 2 ("Inline
`*(volatile T *)&D_globalsym` casts on game-RAM symbols. Treats a plain
game-state global as volatile at the access site, defeating CSE/scheduling.")
spelled in two statements instead of one: same object, same added qualifier,
same CSE effect. That module's pointer-to-volatile carve-out is scoped to
`extern volatile T *name;` declarations whose pointee "is whatever address the
pointer holds (typically a hardware register address loaded at startup)", which
is not a local initialised with the address of a known plain global. The
detector not stripping it is a regex gap, and cheat-checklist T4/T5 say a gap is
not a sanction. The sibling usage at src/code6cac_b.c:4030 is unreviewed
pre-existing source, not a precedent.
*Consequence.* s2's floor of 12 is DISCARDED; banked at
`rejected/volatile-ptr-coercion-score12.c`.

### s3-H1 — KILLED. The sanctioned volatile spelling is available here.
*Statement.* `extern volatile u8 D_80106A73;` plus an entry in
`volatile_extern_allowlist.txt` is the one sanctioned volatile family for a
`D_`-named global, so the axis may be reachable legitimately.
*Probe.* Researched both prongs of
`.claude/rules/legitimate-volatile-interrupt-touched.md` against the codebase.
*Result.* BOTH PRONGS FAIL. Prong 1 (IRQ/MMIO writer, must be cited):
0x80106A73 is KSEG0 game RAM, not the 0x1F801000-0x1F802FFF MMIO range; the six
functions referencing the byte are func_800167AC/BC/D4/EC, func_80034708,
func_80034F88 and func_80035280, none of which is installed via
InterruptCallback / VSyncCallback / irq_EnableInterrupts /
irq_AcknowledgeVblank anywhere in src/, and the only external storer
(func_800167EC) is called synchronously from src/ings.c:414. Prong 2 (use-site
shape, "exact list, not illustrative"): store-then-readback in straight-line
code is not spin-wait, not double-read-across-sequence-point, not
IRQ-mutated-loop-bound. **KILLED** — a measured gate failure, not an open
question.

### s3-H2 — KILLED. A volatile DECLARATION alone would reproduce the target.
*Statement.* If the flag byte were simply declared volatile in the original, the
natural body (plain symbol accesses, no pointer locals) would give the target.
*Mechanism/probe.* The sandbox strips declaration-level volatile, so this was
unmeasurable until s3 built `tmp/grind/func_80034F88/s3/rawscore.py` (same
`engine/score.py` distance, all rules dropped, NO volatile/cheat-asm strip).
Measured four decl-volatile bodies.
*Result.* raw 28-32 at 51 insns — WORSE than the honest non-volatile 20. With
every access volatile, `combine` cannot fold `%lo` into any of the eight mems,
and the target's THREE separate address bases still require three pointer
locals. The 12 belongs to the pointer-local structure, not to the declaration:
plain-decl + three volatile pointers and volatile-decl + three volatile pointers
both score 12. **KILLED** — banked at
`rejected/declvol-plain-symbol-accesses-raw28.c`.

### s3-H3 — CONFIRMED. The non-volatile family was NOT exhausted at 23.
*Statement.* s2's condition-before-read lever, found on the volatile form,
transfers to the non-volatile form; and the copy loop is not actually matching.
*Probe.* 22 variants through `tmp/grind/func_80034F88/s3/probe.py`.
*Result.* `c = p[8] & K;` before `val = D_80106A73;` moves 23/50 -> **22/49**.
The copy loop spelled `*((u8 *)p + i + 0x17)` (induction variable before the
constant displacement) moves 22 -> **20**: it emits `addu v0,a1,v1` in target's
operand order where `+ 0x17 + i` emits `addu v0,v1,a1`, and the insn occurs
twice (loop body + branch delay slot). `((u8 *)p)[i + 0x17]` and
`(&D_80106A70)[i] = ((u8 *)p)[0x17 + i]` do NOT fix it (both 22).
**CONFIRMED — honest floor 20 at 49/49 insns, before s3-H5 took it to 18.**

### s3-H5 — CONFIRMED. Int-typed loaded byte is worth 2 more points, but only
### once the other levers are in place.
*Statement.* s1's H1 ("int-typed temporaries are not the lever", KILLED at 28)
was measured on a form that had none of the levers found since; re-test the type
of the loaded byte on top of the floor-20 form.
*Mechanism.* `s32 val = D_80106A73;` makes the read a `(zero_extend:SI (mem:QI))`
and forces an explicit `andi a0,v1,0xff` truncation before the store, which
lengthens the byte's live range across the block boundary and changes which
value each block's `ori`/`move` pair reads.
*Probe.* Wave G/H (16 variants) through `s3/probe.py`.
*Result.* `s32 val` (with `u8 val2`) scores **18 at 51 insns** — 2 points better
than the 20 despite being 2 instructions longer. `s32` on val2 instead gives
back the gain (20); both `s32` gives 20; declaration order of `c`/`val`/`val2` is
neutral (18 in all three orders tried); read-before-condition is also neutral at
this point (18); ternary (24), positive-sense `if` (26), a single local reused
for load+select (26), one local shared between condition and value (23), pointer
stores (24) and hoisting `p[8]` into a local once (32) are all worse.
**CONFIRMED — honest floor 18, the current state of src/code6cac_b.c.**

### s3-H4 — CONFIRMED (proof, upgrading s2's empirical claim). The residual 20
### points are unreachable without a volatile QImode access.
*Mechanism.* `cse.c:7308-7340` skips recording a SET_DEST only when
`sets[i].src_elt == 0` (line 7329), and `canon_hash` (cse.c:1941-1947) sets
`do_not_record` for a MEM **only** under `MEM_VOLATILE_P`. The remaining
`do_not_record` triggers — PRE/POST inc-dec (no MIPS auto-inc), PC, CC0, CALL,
UNSPEC_VOLATILE, volatile ASM_OPERANDS, hard reg under SMALL_REGISTER_CLASSES —
are unreachable from C in this function.
*Probe.* Read the compiler source, then checked the dichotomy against every
spelling measured in s1-s3 (~30 forms).
*Result.* Two C address expressions for one byte either hash the same (cse
merges the pseudos: one base, the store forwards into the load, the reload dies)
or hash differently (no forward, but two `lui`s and `%lo` folded into each
single-use mem). Target needs BOTH — a shared unfolded base AND a surviving
reload — on the same pair (`lbu a0,0(v1)` immediately after `sb a0,0(v1)`, into
the register that already holds the stored value). Aliasing spellings like
`(&D_80106A70)[3]` do not escape: GCC treats the two expressions as distinct
objects and would MISCOMPILE the read-back, so the original source cannot have
done that either. **CONFIRMED.**

### s2's F3 kill — PREMISE CORRECTED (conclusion survives).
s2 killed "0x80106A70..0x80106A73 is one declared object" by citing "the
target's own relocation records". A shipped PS-EXE has no relocations; splat
renders `%hi(D_80106A73)` by resolving the computed address to the nearest
symbol ([[splat-symbol-names-are-not-evidence]]). The premise is void. F3 stays
dead for the stronger s3-H4 reason instead.

## Open frontier for s4

### F1 — the last honest points are register assignment in the flag blocks, and
### some of them look reachable without touching the reload question.
At floor 18 the build differs from target as follows (side-by-side in
`tmp/grind/func_80034F88/s3/` via `sidebyside.py h0_s32val.c`): the
conditional value lands in `v1` (`ori v1,a0,0x1` / `move v1,a0`) where target
uses `v0` (`ori v0,a0,0x1` / `move v0,a0`); blocks 2 and 3 read the byte with
`move a0,v1` instead of `lbu`; and their stores are `lui at,%hi` + `sb v1,0(at)`
instead of `sb v0,0(base)`. The `v0`/`v1` swap of the SELECTED VALUE is the one
sub-problem that is not obviously downstream of the missing reloads — target's
`v0` is the register `lw v0,0x20(a1)` just used for the condition, so a spelling
that lets the condition temporary and the selected value share one C variable
(or that shortens the condition's live range) may flip it.
*Next probe.* Wave G/H already spent (a) one local shared between condition and
value, (b) declaration order, (c) `s32`/`u8` on both temporaries, (d) hoisting
`p[8]`, (e) ternary / positive-`if` / read-first orderings — see s3-H5 for all
16 numbers; only `s32 val` moved. Untried from here: splitting the three blocks'
temporaries into per-block locals (`val_a`/`val_b`/`val_c`), an explicit
`(u8)`-cast staging variable for the truncation the `s32 val` form emits, a
second natural use of `ptr` late in the function to lengthen its live range, and
swapping which of `c`/`val` is computed into the delay-slot-adjacent statement.
Measure with `s3/probe.py`, read the deltas with `s3/sidebyside.py`.

### F2 — decomp-permuter has still never been run on this function.
It is well-conditioned now for the OPPOSITE reason s2 gave: the form is at
target's exact instruction count and the remaining differences are register
assignment plus three missing loads. A campaign seeded from
`f_L3_srcptr_local.c` is a register-allocation search. Run it in-turn
(`tools/permuter_campaign.py wait --dir <ws>`), harvest, `--stop` it before
ending the turn. No longer gated on any ruling — the seed form is volatile-free.

### F3 — the escalation question this function is converging on (do NOT file it
### from a non-escalation modality; bank it until the driver says so).
If s3-H4 holds, honest pure C cannot produce the target's four `lbu` reloads,
and the two spellings that could are (a) ruled a cheat (s2-F1) and (b) ruled out
by the carve-out's own two-prong gate (s3-H1). That points at an owner question
— "the target bytes say this byte was volatile; the carve-out's prong-2 shape
list does not cover store-then-readback; does the owner extend it, or is this
function OWNER-ACCEPTED INCOMPLETE?" — but the ladder still has forensics,
rederive and synthesis modalities untried, and s3 itself found 3 honest points
that two prior sessions had missed. Grind those first.

## [s3] s2's blocking F1 — is `volatile u8 *pbit = &D_80106A73;` (a local pointer-to-volatile aliasing a global declared plain `extern u8`) a sanctioned construct or a forbidden volatile coercion?
- mechanism: The construct is engine/volatile_cheats.py pattern 2 — its docstring reads 'Inline `*(volatile T *)&D_globalsym` casts on game-RAM symbols. Treats a plain game-state global as volatile at the access site, defeating CSE/scheduling.' A local `volatile u8 *` initialised with `&D_80106A73` is that cast spelled in two statements: same object, same added qualifier, same CSE effect. The module's pointer-to-volatile carve-out is explicitly scoped to `extern volatile T *name;` declarations whose pointee 'is whatever address the pointer holds (typically a hardware register address loaded at startup)', which is not a local aliasing a known plain global.
- probe: Read engine/volatile_cheats.py (docstring + allowlist machinery) and .claude/rules/legitimate-volatile-interrupt-touched.md instead of measuring another spelling; cross-checked against the cheat checklist's family test.
- result: Ruled a CHEAT and resolved in-session rather than escalated: the detector not stripping it is a regex gap, and checklist tests T4/T5 say a detector gap is not a sanction ('it's different because it's spelled with X instead of Y' is the loophole the policy forbids). The sibling usage at src/code6cac_b.c:4030 is unreviewed pre-existing source, not a precedent. s2's floor of 12 is therefore DISCARDED and banked at memory/grind/func_80034F88/rejected/volatile-ptr-coercion-score12.c.
- verdict: KILLED

## [s3] The sanctioned volatile spelling is available for this byte: `extern volatile u8 D_80106A73;` plus an entry in volatile_extern_allowlist.txt.
- mechanism: engine/volatile_cheats.py reads volatile_extern_allowlist.txt at run time and skips listed symbols for pattern 3, so an allowlisted symbol's volatile is NOT stripped by the sandbox and would move the honest floor. Entry requires both prongs of .claude/rules/legitimate-volatile-interrupt-touched.md.
- probe: Researched both prongs against the codebase: enumerated every function referencing D_80106A73 (func_800167AC/BC/D4/EC, func_80034708, func_80034F88, func_80035280), checked which store to it, and grepped src/ for InterruptCallback / VSyncCallback / irq_EnableInterrupts / irq_AcknowledgeVblank installations naming any of them.
- result: BOTH PRONGS FAIL. Prong 1: 0x80106A73 is KSEG0 game RAM, not the 0x1F801000-0x1F802FFF MMIO range, and no referencing function is installed as an IRQ/VSync callback anywhere in src/; the only external storer, func_800167EC, is called synchronously from src/ings.c:414. Prong 2: the use-site shape is store-then-readback in straight-line code, which is not on the rule's exact three-shape list (spin-wait / double-read-across-sequence-point / IRQ-mutated-loop-bound). A measured gate failure, not an open question.
- verdict: KILLED

## [s3] If the flag byte were simply declared volatile in the original, the natural body (plain symbol accesses, no pointer locals) would reproduce the target.
- mechanism: The cheat-invisible sandbox strips declaration-level volatile, so this had never been measurable. s3 built tmp/grind/func_80034F88/s3/rawscore.py — same engine/score.py distance, all regfix/asmfix rules dropped, NO volatile strip and NO cheat-asm strip — purely as a forensic instrument.
- probe: Four decl-volatile bodies (condition-first / read-first x ternary / if-else) plus decl-volatile combined with s2's three volatile pointer locals, scored raw.
- result: raw 28-32 at 51 insns — WORSE than the honest non-volatile 20 reached the same session. With every access volatile, combine cannot fold %lo into any of the eight mems. Plain-decl + three volatile pointer locals and volatile-decl + three volatile pointer locals both score 12, so the 12 belongs to the pointer-local structure, not to the declaration: even if the allowlist grant existed it would not close the function. Banked at rejected/declvol-plain-symbol-accesses-raw28.c.
- verdict: KILLED

## [s3] The non-volatile family is NOT exhausted at 23 — s2's condition-before-read lever transfers to it, and the copy loop that s1/s2 both recorded as already matching does not actually match.
- mechanism: Ordering the p[8] read before the flag-byte read frees the lw's load-delay slot on the non-volatile form too. Separately, `*((u8 *)p + 0x17 + i)` emits `addu v0,v1,a1` while the target has `addu v0,a1,v1` — the operand order follows the source's addition order, and the insn appears twice (loop body + branch delay slot).
- probe: 22 variants through tmp/grind/func_80034F88/s3/probe.py (splice -> sandbox --disable all -> restore), read with s3/sidebyside.py (difflib-aligned build-vs-target instruction listing).
- result: 23/50 -> 22/49 for the ordering lever; 22 -> 20 for the copy loop spelled `*((u8 *)p + i + 0x17)` (induction variable before the constant displacement). Near-identical spellings do NOT work: `((u8 *)p)[i + 0x17]` and `(&D_80106A70)[i] = ((u8 *)p)[0x17 + i]` both stay at 22.
- verdict: CONFIRMED

## [s3] An int-typed loaded byte (`s32 val = D_80106A73;`) is worth further points once the address and ordering levers are in place — reopening s1's H1, which killed int temporaries at 28.
- mechanism: The read becomes a (zero_extend:SI (mem:QI)) and an explicit `andi a0,v1,0xff` truncation appears before each store, lengthening the byte's live range across the block boundary and changing which value each block's ori/move pair reads.
- probe: Waves G and H, 16 variants: type of val and val2, declaration order, ternary vs positive-sense if vs negated if, one local shared between condition and value, pointer stores, hoisting p[8] into a local, read-before-condition.
- result: `s32 val` with `u8 val2` scores 18 at 51 insns — 2 points better than the 20 despite 2 more instructions. `s32 val2` gives the gain straight back (20); both s32 = 20; declaration order is neutral (18 in all three orders); read-first is neutral (18); ternary 24, positive-sense if 26, single reused local 26, shared condition/value local 23, pointer stores 24, hoisted p[8] 32. s1's kill was correct for its form and wrong for this one — the byte's type only pays once the shape is right.
- verdict: CONFIRMED

## [s3] The residual distance is unreachable in pure C without a volatile QImode access — upgrading s2's empirical 'non-volatile is exhausted' to a mechanism-level proof.
- mechanism: cse.c:7308-7340 skips recording a SET_DEST only when sets[i].src_elt == 0 (line 7329), and canon_hash (cse.c:1941-1947) sets do_not_record for a MEM ONLY under MEM_VOLATILE_P. Every other do_not_record trigger — PRE/POST inc-dec (no MIPS auto-inc), PC, CC0, CALL, UNSPEC_VOLATILE, volatile ASM_OPERANDS, hard reg under SMALL_REGISTER_CLASSES — is unreachable from C in this function.
- probe: Read cse.c (canon_hash, cse_insn's dest-recording block, cse_end_of_basic_block) and tested the resulting dichotomy against every spelling measured in s1-s3 (~50 forms in total across the three sessions).
- result: Two C address expressions for one byte either hash the same (cse merges the pseudos: one shared unfolded base, but the store forwards into the load and the reload dies) or hash differently (no forward, but two luis and combine folds %lo into each single-use mem). The target needs BOTH on the same pair — `lbu a0,0(v1)` immediately after `sb a0,0(v1)`, reloading into the register that already holds the stored value. Aliasing spellings such as (&D_80106A70)[3] are not an escape: GCC would treat the two expressions as distinct objects and MISCOMPILE the read-back, so the original source cannot have done that either.
- verdict: CONFIRMED

## Resolved in s4 (permuter)

### s3-F2 — KILLED. "decomp-permuter is well-conditioned on this function and a
### campaign is a register-allocation search that should close the last points."
*Statement.* With a form already at (or within 2 of) the target's instruction
count and the residual being register assignment, a permuter campaign is a
narrow RA search rather than a shape search, so it should find the closing form.
*Mechanism.* decomp-permuter randomizes C-level structure and scores the
compiled function against `target.o` with a weighted metric (regs x5,
reorderings x60, ins/del x100).
*Probe.* Two campaigns via `tools/permuter_campaign.py`, both waited for
IN-TURN and `harvest --stop`ped: seed 1 from the floor-18 51-insn `s32 val`
chassis (34,003 iterations, 30 finds) and seed 2 from the structurally different
49-insn `u8 val` chassis (16,422 iterations, 67 finds). All 97 finds re-scored
with `sandbox func_80034F88 --disable all` via `s4/eval.py` / `eval2.py`.
*Result.* Best honest, semantically-correct find on EITHER seed is **18** — a
tie with the seed, no improvement, from 50,425 total iterations. Seed 2
converged onto seed 1's basin (its 18-point finds are 51-insn `s32`-widened
forms), so the two chassis are one basin. The single sub-floor number (17) is a
miscompile, see s4-H2. **KILLED — the random-permuter axis is closed here.**

### s4-H1 — CONFIRMED (and it is the reusable finding). The permuter's weighted
### score is not a usable gradient for this function.
*Statement.* Permuter score and honest sandbox distance measure different things
here, so campaign progress cannot be read off the permuter's numbers.
*Probe.* Re-scored all 30 seed-1 finds through the engine sandbox and compared
the two metrics pairwise (`s4/eval.json`).
*Result.* Uncorrelated, in fact slightly NEGATIVELY correlated: the permuter's
best find (perm 1070) scores sandbox 18 while its worst (perm 1300) scores 20,
and the numerically best sandbox result came from a mid-table perm 1290.
`--stop-on-zero` can therefore never fire on a form the engine would call
closed. Operational consequence: rank finds on the sandbox, never on the
permuter score. **CONFIRMED.**

### s4-H2 — KILLED. "The permuter's 17-point find is a floor drop."
*Probe.* Read `ws/output-1290-4/source.c` for semantics after it scored 17 at
49 insns.
*Result.* The third block's `D_80106A73 = val2;` store was hoisted INSIDE the
`if (!c)` arm, so the byte is never written when bit 4 of `p[8]` is set and is
written with `val | 4` instead of `val` when it is clear — a behaviour change,
not a spelling. It also carries a `long new_var = 4;` constant holder and an
`s32 *new_var3 = p;` pointer alias. **KILLED**, banked at
`rejected/permuter-17-store-hoisted-into-arm-MISCOMPILES.c`.

### s3-F1's four untried probes — measured; two DEAD, two NEUTRAL. F1 is spent.
*Probe.* `s4/score_files.py` on five standalone chassis files.
*Result.* per-block temporaries `va/vb/vc + ra/rb/rc + ca/cb/cc` = **30**/50;
copy loop through the existing pointer `*(ptr - 3 + i)` = **31**/50; third block
storing via `*ptr = val2;` = **22**/50; explicit `(u8)`-cast staging on the
selected value = **18** (neutral); block 1 read-first with blocks 2-3
condition-first = **18** (neutral). Banked as
`rejected/perblock-temps-score30.c`, `rejected/copyloop-via-ptr-alias-score31.c`
and `rejected/last-store-via-ptr-score22.c`. **KILLED.**
*What the two pointer results teach.* They invert s3's intuition. Lengthening
`ptr`'s live range is exactly what must NOT happen: each extra use keeps the one
address pseudo live further down the function, which is the case where cse
merges it and the target's per-block `lui`+`addiu` rematerialisations vanish.
The floor form's `ptr` having exactly ONE read-modify-write use is load-bearing
in BOTH directions — two memory operands so `combine` cannot fold `%lo`, and no
third operand so the pseudo dies before block 2.

## Open frontier for s5

### F1 — a forensic `-da` read of the FLOOR-18 form itself (never yet taken).
Structural spelling (s2, s3) and permuter (s4) are both spent on this basin:
roughly 60 hand-measured spellings plus 50,425 permuter iterations from two
chassis all bottom out at 18, and s3-H4 gives a compiler-source-level reason
(cse.c:7329 plus canon_hash's `MEM_VOLATILE_P`-only `do_not_record`). What has
NOT been done is an RTL dump of the CURRENT form — s2's only dumps
(`s2/rtl/v10/`) are of a pre-lever variant that no longer resembles it. The
`.greg` dump would say WHY the selected value lands in `v1` where target has
`v0`, in terms of allocno order and conflicts, instead of by spelling roulette.
Cheapest remaining honest measurement, and it is a forensics-modality probe.
*Next probe.* `s2/dump_rtl.sh` against the current src/ body; read
`;; Register dispositions` and the conflict list for the pseudo holding `val2`;
then diff against the same dump for a variant with the condition temporary `c`
eliminated (`if (!(p[8] & K))` inline) to see whether `c`'s live range is what
occupies `v0` in our build.

### F2 — directed permuter (PERM_* macros) is untried; RANDOM permuter is dead.
s4 ran random campaigns only. `tools/permuter_annotate.py --func <f> --hint
<rule-slug>` plus `PERM_*` macros would let a campaign enumerate a specific axis
(e.g. every ordering of the three statements in a flag block, or the
cross-product of temporary types) rather than sample the neighbourhood. Given
s4-H1 (the permuter metric is not a gradient here), a directed campaign's value
is as an exhaustive ENUMERATOR whose output is re-scored by `s4/eval.py` — not
as a search. Worth one campaign, at low expectation, and only after F1.

### F3 — the escalation packet, unchanged; do NOT file it from a
### non-escalation modality.
s3's F3 stands verbatim: if s3-H4 holds, honest pure C cannot produce the
target's four `lbu` reloads, and the two spellings that could are ruled a cheat
(s2-F1) and ruled out by the carve-out's own two-prong gate (s3-H1). s4 adds one
fact to the packet: the permuter, given 50,425 iterations from two structurally
different chassis, also never produced them. The ladder still has forensics,
rederive and synthesis untried — grind those first.

## [s4] decomp-permuter is well-conditioned on this function: the form is at the target's instruction count and the residual is register assignment, so a campaign is a register-allocation search that should close the last points.
- mechanism: decomp-permuter randomizes C-level structure and scores the compiled function against target.o with a weighted metric (regs x5, reorderings x60, ins/del x100). A seed already at target's shape means the space it explores is allocation, not shape.
- probe: Two campaigns via tools/permuter_campaign.py, each launched with -j 8 --stop-on-zero, waited for IN-TURN with `permuter_campaign.py wait`, and harvest --stopped before the session ended: seed 1 from the floor-18 51-insn `s32 val` chassis (34,003 iterations, 30 finds) and seed 2 from the structurally different 49-insn `u8 val` chassis (16,422 iterations, 67 finds). All 97 finds re-scored with `sandbox func_80034F88 --disable all` (tmp/grind/func_80034F88/s4/eval.py, eval2.py). Workspaces built by s4/mkws.sh with a minimal-context base validated to reproduce full-TU codegen instruction-for-instruction.
- result: Best honest, semantically-correct find on either seed is 18 — a tie with the seed, no floor improvement, from 50,425 total iterations. Seed 2 converged onto seed 1's basin (its 18-point finds are 51-insn s32-widened forms), so two structurally distinct chassis are one basin. The random-permuter axis on this function is closed.
- verdict: KILLED

## [s4] The permuter's weighted score tracks the engine's honest sandbox distance closely enough to be used as campaign progress on this function.
- mechanism: Both metrics compare the built function against the same target bytes, so they were expected to move together; --stop-on-zero relies on it.
- probe: Re-scored all 30 seed-1 finds through `sandbox func_80034F88 --disable all` and compared the two metrics pairwise (tmp/grind/func_80034F88/s4/eval.json).
- result: Uncorrelated, in fact slightly NEGATIVELY correlated. The permuter's best find (perm 1070) scores sandbox 18; its worst (perm 1300) scores 20; the numerically best sandbox result came from a mid-table perm 1290. --stop-on-zero can therefore never fire on a form the engine would call closed, and a campaign is a random SAMPLER of the neighbourhood rather than a descent of the metric that counts. Operational rule for every future permuter session here: rank finds on the sandbox score, never on the permuter score; s4/eval.py does the re-scoring and generalises to other functions by changing three constants.
- verdict: CONFIRMED

## [s4] The permuter's single sub-floor find (sandbox 17 at 49 insns) is a genuine floor drop.
- mechanism: It scored below every hand-derived form, at the target's exact instruction count, which is the shape a closing form would have.
- probe: Read ws/output-1290-4/source.c for semantics rather than crediting the score.
- result: It is a MISCOMPILE. The third flag block's `D_80106A73 = val2;` store was hoisted INSIDE the `if (!c)` arm, so the flag byte is never written when bit 4 of p[8] is set, and is written with `val | 4` instead of `val` when it is clear; the target stores unconditionally on the join. It also carries a `long new_var = 4;` constant holder and an `s32 *new_var3 = p;` pointer alias, both of which would need FAKE carve-outs even had the semantics been right. Banked at rejected/permuter-17-store-hoisted-into-arm-MISCOMPILES.c.
- verdict: KILLED

## [s4] The four structural probes s3 left untried on frontier F1 (per-block temporaries, an explicit (u8)-cast staging variable, a second natural use of `ptr` late in the function, and swapping which of c/val is computed adjacent to the delay slot) contain a floor improvement.
- mechanism: Each targets the v0/v1 swap of the selected value, which s3 identified as the one residual sub-problem not obviously downstream of the missing lbu reloads.
- probe: Five standalone chassis files scored with tmp/grind/func_80034F88/s4/score_files.py (splice into src/code6cac_b.c, `sandbox --disable all`, restore).
- result: None improves the floor. Per-block temporaries (va/vb/vc + ra/rb/rc + ca/cb/cc) = 30 at 50 insns; the copy loop addressed through the existing pointer as `*(ptr - 3 + i)` = 31 at 50; the third block storing via `*ptr = val2;` = 22 at 50; the explicit (u8)-cast staging variable = 18 (neutral); block 1 reading the flag before its condition while blocks 2-3 stay condition-first = 18 (neutral). The two pointer results invert s3's intuition: lengthening `ptr`'s live range is exactly what must NOT happen, because each extra use keeps the single address pseudo live further down the function — the case where cse merges it and the target's per-block lui+addiu rematerialisations vanish. The floor form's one read-modify-write use of `ptr` is load-bearing in both directions.
- verdict: KILLED

## [s4] decomp-permuter is well-conditioned on this function: the form is already at the target's instruction count and the residual is register assignment, so a campaign is a register-allocation search that should close the last points (s3's frontier F2).
- mechanism: decomp-permuter randomizes C-level structure and scores the compiled function against target.o with a weighted metric (regs x5, reorderings x60, ins/del x100). A seed already at target's shape means the space it explores is allocation, not shape.
- probe: Two campaigns via tools/permuter_campaign.py, each launched -j 8 --stop-on-zero, waited for IN-TURN with `permuter_campaign.py wait`, and harvest --stopped before the session ended. Seed 1 from the floor-18 51-insn `s32 val` chassis: 34,003 iterations, 30 finds. Seed 2 from the structurally different 49-insn `u8 val` chassis: 16,422 iterations, 67 finds. All 97 finds re-scored with `sandbox func_80034F88 --disable all` via tmp/grind/func_80034F88/s4/eval.py and eval2.py. Workspaces built by s4/mkws.sh, whose minimal-context base was validated to reproduce the full-TU codegen instruction-for-instruction.
- result: Best honest, semantically-correct find on EITHER seed is 18 — a tie with the seed, no floor improvement, from 50,425 total iterations. Seed 2 converged onto seed 1's basin (its 18-point finds are 51-insn s32-widened forms), so two structurally distinct chassis are one basin. The random-permuter axis on this function is closed.
- verdict: KILLED

## [s4] The permuter's weighted score tracks the engine's honest sandbox distance closely enough to be used as campaign progress on this function.
- mechanism: Both metrics compare the built function against the same target bytes, so they were expected to move together; --stop-on-zero relies on it.
- probe: Re-scored all 30 seed-1 finds through `sandbox func_80034F88 --disable all` and compared the two metrics pairwise (tmp/grind/func_80034F88/s4/eval.json).
- result: Uncorrelated, in fact slightly NEGATIVELY correlated. The permuter's best find (perm 1070) scores sandbox 18; its worst (perm 1300) scores 20; the numerically best sandbox result came from a mid-table perm 1290. --stop-on-zero can therefore never fire on a form the engine would call closed, and a campaign here is a random SAMPLER of the neighbourhood rather than a descent of the metric that counts. Operational rule banked for every future permuter session on this function: rank finds on the sandbox score, never on the permuter score.
- verdict: CONFIRMED

## [s4] The permuter's single sub-floor find (sandbox 17 at 49 insns, seed-1 output-1290-4) is a genuine floor drop.
- mechanism: It scored below every hand-derived form, at the target's exact instruction count, which is the shape a closing form would have.
- probe: Read ws/output-1290-4/source.c for semantics rather than crediting the score.
- result: It is a MISCOMPILE. The third flag block's `D_80106A73 = val2;` store was hoisted INSIDE the `if (!c)` arm, so the flag byte is never written when bit 4 of p[8] is set and is written with `val | 4` instead of `val` when it is clear; the target stores unconditionally on the join. It additionally carries a `long new_var = 4;` constant holder and an `s32 *new_var3 = p;` pointer alias, both of which would need FAKE carve-outs even had the semantics been right. Banked at rejected/permuter-17-store-hoisted-into-arm-MISCOMPILES.c so no later session re-finds this 17 and credits it.
- verdict: KILLED

## [s4] The four spellings s3 left untried on frontier F1 — per-block temporaries, an explicit (u8)-cast staging variable, a second natural use of `ptr` late in the function, and swapping which of c/val is computed adjacent to the delay slot — contain a floor improvement.
- mechanism: Each targets the v0/v1 swap of the SELECTED VALUE, which s3 identified as the one residual sub-problem not obviously downstream of the missing lbu reloads.
- probe: Five standalone chassis files scored with tmp/grind/func_80034F88/s4/score_files.py (splice into src/code6cac_b.c, `sandbox func_80034F88 --disable all`, restore src/).
- result: None improves the floor. Per-block temporaries (va/vb/vc + ra/rb/rc + ca/cb/cc) = 30 at 50 insns; the copy loop addressed through the existing pointer as `*(ptr - 3 + i)` = 31 at 50; the third block storing via `*ptr = val2;` = 22 at 50; the explicit (u8)-cast staging variable = 18 (neutral); block 1 reading the flag before its condition while blocks 2-3 stay condition-first = 18 (neutral). Banked as rejected/perblock-temps-score30.c, rejected/copyloop-via-ptr-alias-score31.c and rejected/last-store-via-ptr-score22.c.
- verdict: KILLED

## Resolved in s5 (permuter — DIRECTED)

### s4-F2 — KILLED. "Directed permuter (`PERM_*` macros) can still contribute as
### an exhaustive ENUMERATOR of a specific axis even though random permuter is
### dead."
*Statement.* Random sampling is dead (s4), but manual-mutation mode makes the
permuter enumerate a declared cross-product, and re-scoring every member with
the honest sandbox would close an axis rather than sample it.
*Probe.* Two directed campaigns through `tools/permuter_campaign.py`
(launch / wait IN-TURN / harvest --stop), each with a `PERM_GENERAL`-annotated
base and NO `PERM_RANDOMIZE` so random mutation is disabled: `s5/ws`
(864-form ordering/select/call-order/loop space) and `s5/ws2` (144-form
type/mask/condition-form space). Both campaigns terminated by themselves on
space exhaustion (864 and 288 iterations). In parallel the SAME two
cross-products were enumerated deterministically and every member scored with
`sandbox func_80034F88 --disable all` — `s5/sweep5.py` (864 forms) and
`s5/sweep6.py` (144 forms), results in `s5/sweep5.jsonl` / `s5/sweep6.jsonl`.
*Result.* **1,008 forms scored on the honest metric; best = 18 in both waves; no
form beats, and none but the known chassis ties, the floor.** The directed
campaigns produced exactly ONE output between them (`ws2/output-1290-1`,
permuter weighted score 1290 vs base 1300) and it sandbox-scores 18 — a tie,
banked at `rejected/directed-perm-mixed-condform-score18-TIE.c`. The permuter
axis is now closed in BOTH modes: random (s4, 50,425 iters) and directed (s5,
1,152 iters + 1,008 exhaustive sandbox scores). **KILLED.**

### s5-H1 — CONFIRMED. The per-block ORDER of the condition read and the
### flag-byte read is completely neutral at floor 18.
*Statement.* s3's 23 -> 22 "condition before the flag read" lever should still
be worth a point, and mixing orders per block might be worth more.
*Mechanism.* Ordering pins the relative position of `lw v0,0x20(a1)` and the
`lbu`, which decides whether the load-delay slot is filled.
*Probe.* Wave A enumerates all 8 per-block order combinations against every
other axis (`s5/sweep5.jsonl`).
*Result.* The eight 18-point forms are precisely `call=pc` x `sels=nnn` x
`loop=i17` crossed with ALL EIGHT orderings — the axis contributes ZERO in every
combination. The s3 lever was worth 1 point at floor 23 and was ABSORBED by the
`s32 val` widening (s3-H5) rather than being additive with it. Practical
consequence for later sessions: spell that ordering however reads best; it is
not a lever any more and must not be treated as load-bearing. **CONFIRMED.**

### s5-H2 — CONFIRMED (new, and it is the largest single-axis penalty measured
### on this function). `ptr = &D_80106A73;` must be assigned AFTER the call.
*Statement.* The relative order of `p = func_80077D00();` and
`ptr = &D_80106A73;` is cosmetic.
*Mechanism (measured, not assumed).* It is not cosmetic: creating the address
pseudo BEFORE the call insn makes it live across the call, which reshapes the
whole flag section's allocation and address materialisation.
*Probe.* Wave A crosses both call orders with all 432 other combinations.
*Result.* Minimum over the 432 `ptr`-first forms is **29** (+11 over the floor);
minimum over the 432 call-first forms is 18. This axis had never been probed in
s1-s4. **CONFIRMED — the call-first order is load-bearing and must not be
"tidied".**

### s5-H3 — CONFIRMED. The copy-loop spelling is exactly orthogonal to every
### flag-block spelling.
*Probe.* Wave A pairs each of the 432 flag-section forms with both copy-loop
spellings.
*Result.* `*((u8 *)p + 0x17 + i)` scores EXACTLY +2 over
`*((u8 *)p + i + 0x17)` in all 432 pairs — the delta set is the single value
{2}, no exceptions. s3's copy-loop lever is fully independent of everything
else, which is why it survived every subsequent change. **CONFIRMED.**

### s5-H4 — KILLED. The type / cardinality / condition-shape cross-product
### contains an improvement.
*Statement.* s3 measured val/val2 types only as a 2x2 (s32/u8) on one condition
form; u32, the type of `c`, splitting `c` alone, the mask spelling and the
masked-inside-the-`if` condition form were all untried, and none had ever been
crossed with the others.
*Probe.* Wave B, 144 forms (`s5/sweep6.py`).
*Result.* Best 18, reached only by {`s32 val`, `u32 val`} x {compound, expanded}
mask with `u8 val2`, `s32 c`, one shared `c`, condition pre-masked. Everything
else is worse: `u8 val` 20, `s32 val2` / `u32 val2` 20, `u8 c` 19, three used
`c1/c2/c3` locals 19, condition masked inside the `if` 19 at 52 insns. Three of
these are genuinely new facts: (a) what the loaded byte needs is WORD width, not
signedness — `u32 val` ties `s32 val`; (b) the mask spelling
`*ptr &= 0xF8;` vs `*ptr = *ptr & 0xF8;` is NEUTRAL, so what defeats combine's
`%lo` fold is the address expression having two memory operands, not the
compound-assignment syntax; (c) splitting only `c` costs 1, so s4's per-block
score of 30 was dominated by splitting `val`/`val2`, not `c`. **KILLED.**

## Open frontier for s6

### F1 (unchanged, and now the ONLY instrument left that is not spelling) — a
### forensic `-da` read of the FLOOR-18 form itself.
Permuter is closed in both modes (s4 random, s5 directed) and structural
spelling has now been enumerated exhaustively over two large cross-products
(s5's 1,008 forms) on top of s2/s3's ~60 hand-measured ones. Every one bottoms
at 18. What has still NEVER been done is an RTL dump of the CURRENT body:
s2's only dumps (`s2/rtl/v10/`) are of a pre-lever variant. The `.greg` dump's
`;; Register dispositions` and conflict list would say WHY the pseudo holding
`val2` lands in `v1` where target has `v0` — target's `v0` is the very register
`lw v0,0x20(a1)` just used for the condition, so the standing suspicion is that
our condition temporary `c` holds `v0` across the block.
*Next probe.* `s2/dump_rtl.sh` against the current src/ body; read the
dispositions and the conflict list for the `val2` pseudo; then diff against the
same dump for a variant with `c` eliminated (`if (!(p[8] & K))` inline). Note
s5-H1 makes the ordering axis free, so a `c`-eliminated variant can be dumped
without disturbing anything else. Forensics modality.

### F2 — rederive / synthesis modalities are untried.
The ladder still holds `rederive` (what did the ORIGINAL source look like — the
four `lbu` reloads plus three `lui`+`addiu` bases are a signature that may be
produced by a shape nobody has written yet, e.g. the flag section being a small
loop over a bit-index rather than three unrolled blocks) and `synthesis`. A
bit-index loop over `{1,2,4}` has never been measured on this function; it is
the one structural shape s1-s5 all assumed away, because the target is unrolled
— but GCC unrolls nothing here, so an unrolled TARGET does not imply an
unrolled SOURCE only if the loop were fully peeled, which `-O2` without
`-funroll-loops` will not do. Cheap to falsify and it is the last untried
SHAPE rather than another spelling.

### F3 — the escalation packet, unchanged; do NOT file it from a
### non-escalation modality.
s3's F3 stands verbatim, with s4's permuter negative and now s5's directed-
permuter negative (1,008 exhaustively enumerated forms, 0 improvements) as the
newest lines in it. The ladder still has forensics, rederive and synthesis
untried — grind those first.

## [s5] s4's frontier F2 — directed permuter (PERM_* macros) can still contribute as an exhaustive ENUMERATOR of a specific axis even though the random permuter is dead.
- mechanism: decomp-permuter's manual-mutation mode: a base annotated with multi-choice PERM_GENERAL(...) macros and NO PERM_RANDOMIZE disables random mutation entirely and makes the permuter enumerate the declared cross-product, printing "Will run for N iterations" where N is the space size. Its own weighted metric is not a gradient here (s4-H1), so the value is enumeration, with every member re-scored by the honest sandbox.
- probe: Two directed campaigns via tools/permuter_campaign.py (launch / wait IN-TURN / harvest --stop), each terminating by itself on space exhaustion: tmp/grind/func_80034F88/s5/ws, label directed-perm-general, 864-form space (call/ptr order x per-block condition-vs-read order x per-block select form neg/pos/ternary x copy-loop spelling), 864 iterations; and tmp/grind/func_80034F88/s5/ws2, label directed-types-condform, 144-form space (val/val2/c types x c cardinality x mask spelling x condition form), 288 iterations. In parallel the SAME two cross-products were enumerated deterministically and EVERY member scored with `sandbox func_80034F88 --disable all` via s5/sweep5.py (864 forms) and s5/sweep6.py (144 forms).
- result: 1,008 forms scored on the honest metric; best is 18 in both waves, no improvement on the floor. Wave A distribution: 18 x8, 19 x8, 20 x24, 21 x56, 22 x48, 23 x88, 24 x80, 25 x56, 26 x48, 27 x16, tail to 38. Wave B: 18 x4, 19 x28, 20 x14, 21 x98. The two campaigns produced exactly ONE output between them (ws2/output-1290-1, permuter weighted score 1290 against base 1300 — i.e. the permuter ranked it BETTER than the floor chassis) and it sandbox-scores 18, a tie; banked at rejected/directed-perm-mixed-condform-score18-TIE.c and a third independent confirmation of s4-H1. The permuter axis is now closed in both of its modes: random (s4, 50,425 iterations) and directed (s5, 1,152 iterations plus 1,008 exhaustive sandbox scores).
- verdict: KILLED

## [s5] s3's "condition read before the flag-byte read" lever is still worth a point at floor 18, and mixing the order per block is worth more.
- mechanism: Source order pins the relative position of `lw v0,0x20(a1)` and the flag-byte `lbu`, which decides whether the lw's load-delay slot gets filled; s3 measured the lever as 23 -> 22 on the pre-widening chassis.
- probe: Wave A enumerates all 8 per-block order combinations against every other axis (864 forms, s5/sweep5.jsonl).
- result: The eight 18-point forms are precisely call-first x all-negated-selects x `p + i + 0x17` loop, crossed with ALL EIGHT per-block orderings — the axis contributes ZERO in every combination. The s3 lever was ABSORBED by the s32-val widening (s3-H5) rather than being additive with it. Practical consequence: the ordering is free, so it must not be treated as load-bearing, and a forensics variant may reorder or eliminate the condition temporary without disturbing anything else.
- verdict: CONFIRMED

## [s5] The relative order of `p = func_80077D00();` and `ptr = &D_80106A73;` is cosmetic.
- mechanism: Assigning the pointer local before the call makes its address pseudo live ACROSS the call insn, which reshapes the whole flag section's allocation and per-block address materialisation.
- probe: Wave A crosses both call orders with all 432 other axis combinations.
- result: Refuted, and it is the largest single-axis penalty measured on this function: the minimum over the 432 `ptr`-first forms is 29 (+11 over the floor) while the minimum over the 432 call-first forms is 18. This axis had never been probed in s1-s4. The call-first order is load-bearing and must not be "tidied".
- verdict: KILLED

## [s5] The type / cardinality / condition-shape cross-product (u32 temporaries, the type of `c`, splitting only `c`, the mask spelling, and masking inside the `if`) contains a floor improvement.
- mechanism: s3 measured val/val2 types only as a 2x2 (s32/u8) on a single condition form, and s4's per-block probe split ALL THREE temporaries at once; none of these axes had ever been crossed with the others.
- probe: Wave B, 144 forms, every one scored with `sandbox func_80034F88 --disable all` (s5/sweep6.py, s5/sweep6.jsonl).
- result: Best 18, reached only by {s32 val, u32 val} x {compound mask, expanded mask} with u8 val2, s32 c, one shared c and a pre-masked condition. `u8 val` 20; `s32 val2` and `u32 val2` 20; `u8 c` 19; three used c1/c2/c3 locals 19; condition masked inside the `if` 19 at 52 insns. Three new facts fall out: (a) the loaded byte needs WORD WIDTH, not signedness — `u32 val` ties `s32 val`, so s3-H5's mechanism is the zero_extend, full stop; (b) `*ptr &= 0xF8;` and `*ptr = *ptr & 0xF8;` are NEUTRAL, so what defeats combine's %lo fold is the address expression having two memory operands rather than the compound-assignment syntax; (c) splitting only `c` costs exactly 1, so s4's per-block score of 30 was dominated by splitting val/val2.
- verdict: KILLED

## [s5] The copy-loop spelling interacts with the flag-block spellings.
- mechanism: If the two regions competed for registers or scheduling slots, the copy loop's 2-point lever would vary with the flag section's shape.
- probe: Wave A pairs each of the 432 flag-section forms with both copy-loop spellings and diffs the scores pairwise.
- result: `*((u8 *)p + 0x17 + i)` scores EXACTLY +2 over `*((u8 *)p + i + 0x17)` in all 432 pairs — the delta set is the single value {2}, with no exceptions. The copy loop is fully orthogonal to every flag-block spelling, which is why s3's lever survived every later change and why no future session needs to re-cross it.
- verdict: KILLED

## [s5] s4's frontier F2 — directed permuter (PERM_* macros) can still contribute as an exhaustive ENUMERATOR of a specific axis even though the random permuter is dead.
- mechanism: decomp-permuter's manual-mutation mode: a base annotated with multi-choice PERM_GENERAL(...) macros and NO PERM_RANDOMIZE disables random mutation entirely and makes the permuter enumerate the declared cross-product, printing 'Will run for N iterations' where N is the space size. Its own weighted metric is not a gradient here (s4-H1), so the value is enumeration, with every member re-scored on the honest sandbox.
- probe: Two directed campaigns via tools/permuter_campaign.py (launch / wait IN-TURN / harvest --stop), both terminating by themselves on space exhaustion: tmp/grind/func_80034F88/s5/ws (label directed-perm-general, 864-form space = call/ptr order x per-block condition-vs-read order x per-block select form neg/pos/ternary x copy-loop spelling, 864 iterations) and s5/ws2 (label directed-types-condform, 144-form space = val/val2/c types x c cardinality x mask spelling x condition form, 288 iterations). In parallel the SAME two cross-products were enumerated deterministically and EVERY member scored with `sandbox func_80034F88 --disable all` via s5/sweep5.py (864 forms) and s5/sweep6.py (144 forms).
- result: 1,008 forms scored on the honest metric; best is 18 in both waves, no improvement on the floor. Wave A distribution: 18 x8, 19 x8, 20 x24, 21 x56, 22 x48, 23 x88, 24 x80, 25 x56, 26 x48, 27 x16, tail to 38. Wave B: 18 x4, 19 x28, 20 x14, 21 x98. The two campaigns produced exactly ONE output between them (ws2/output-1290-1, permuter weighted score 1290 against a base of 1300 — i.e. the permuter ranked it BETTER than the floor chassis) and it sandbox-scores 18, a tie; banked at rejected/directed-perm-mixed-condform-score18-TIE.c and a third independent confirmation of s4-H1. The permuter axis is now closed in BOTH of its modes: random (s4, 50,425 iterations) and directed (s5, 1,152 iterations plus 1,008 exhaustive sandbox scores).
- verdict: KILLED

## [s5] s3's 'condition read before the flag-byte read' lever is still worth a point at floor 18, and mixing the order per block is worth more.
- mechanism: Source order pins the relative position of `lw v0,0x20(a1)` and the flag-byte `lbu`, which decides whether the lw's load-delay slot gets filled; s3 measured the lever as 23 -> 22 on the pre-widening chassis.
- probe: Wave A enumerates all 8 per-block order combinations against every other axis (864 forms, s5/sweep5.jsonl).
- result: The eight 18-point forms are precisely call-first x all-negated-selects x `p + i + 0x17` loop, crossed with ALL EIGHT per-block orderings — the axis contributes ZERO in every combination. The s3 lever was worth 1 point at floor 23 and was ABSORBED by the s32-val widening (s3-H5) rather than being additive with it. Practical consequence: the ordering is free, so it must not be defended as load-bearing, and a forensics variant may reorder or eliminate the condition temporary without disturbing anything else.
- verdict: CONFIRMED

## [s5] The relative order of `p = func_80077D00();` and `ptr = &D_80106A73;` is cosmetic.
- mechanism: Assigning the pointer local before the call makes its address pseudo live ACROSS the call insn, which reshapes the whole flag section's allocation and per-block address materialisation.
- probe: Wave A crosses both call orders with all 432 other axis combinations.
- result: Refuted, and it is the largest single-axis penalty measured on this function: the minimum over the 432 `ptr`-first forms is 29 (+11 over the floor) while the minimum over the 432 call-first forms is 18. This axis had never been probed in s1-s4. The call-first order is load-bearing and must not be 'tidied'.
- verdict: KILLED

## [s5] The type / cardinality / condition-shape cross-product (u32 temporaries, the type of `c`, splitting only `c`, the mask spelling, and masking inside the `if`) contains a floor improvement.
- mechanism: s3 measured val/val2 types only as a 2x2 (s32/u8) on a single condition form, and s4's per-block probe split ALL THREE temporaries at once; none of these axes had ever been crossed with the others.
- probe: Wave B, 144 forms, every one scored with `sandbox func_80034F88 --disable all` (s5/sweep6.py, s5/sweep6.jsonl).
- result: Best 18, reached only by {s32 val, u32 val} x {compound mask, expanded mask} with u8 val2, s32 c, one shared c and a pre-masked condition. `u8 val` 20; `s32 val2` and `u32 val2` 20; `u8 c` 19; three used c1/c2/c3 locals 19; condition masked inside the `if` 19 at 52 insns. Three new facts: (a) the loaded byte needs WORD WIDTH, not signedness — `u32 val` ties `s32 val`, so s3-H5's mechanism is the zero_extend, full stop; (b) `*ptr &= 0xF8;` and `*ptr = *ptr & 0xF8;` are NEUTRAL, so what defeats combine's %lo fold is the address expression having two memory operands rather than the compound-assignment syntax; (c) splitting only `c` costs exactly 1, so s4's per-block score of 30 was dominated by splitting val/val2.
- verdict: KILLED

## [s5] The copy-loop spelling interacts with the flag-block spellings.
- mechanism: If the two regions competed for registers or scheduling slots, the copy loop's 2-point lever would vary with the flag section's shape.
- probe: Wave A pairs each of the 432 flag-section forms with both copy-loop spellings and diffs the scores pairwise.
- result: `*((u8 *)p + 0x17 + i)` scores EXACTLY +2 over `*((u8 *)p + i + 0x17)` in all 432 pairs — the delta set is the single value {2}, no exceptions. The copy loop is fully orthogonal to every flag-block spelling, which is why s3's lever survived every later change and why no future session needs to re-cross it.
- verdict: KILLED

---

## s6 — forensics modality

### s6-H1 — "combine folds %lo into a single-use mem, which is why our unfolded base disappears" (INHERITED from s1/s3/s4)
- **statement.** The target's `lui`+`addiu` base register survives because its
  address expression feeds two memory operands; when an address expression feeds
  exactly one memory operand, `combine` folds `%lo` into that operand and the base
  register disappears.
- **mechanism claimed.** A `combine` (combine.c) fold of the address into the mem.
- **probe.** cc1 `-da` dump of the floor-18 form; read `rtl.fn` (pre-cse, i.e.
  straight out of RTL expansion) for the address rtx of each flag-byte access.
- **result.** REFUTED. `rtl.fn` lines 52/87/102/137/152/187 already carry
  `(mem:QI (symbol_ref:SI ("D_80106A73")))` before ANY optimizer pass has run, and
  lines 29/37 already carry `(mem:QI (reg/v:SI 73))` for the `*ptr` accesses. The
  MIPS backend accepts a bare `symbol_ref` as a legitimate address; the
  `lui $at` / `%lo(...)($at)` expansion happens at assembly-output time. `combine`
  never touches this.
- **verdict.** KILLED.
- **consequence.** The address form of every access is fixed by its C spelling at
  expansion time. There is no optimizer decision on this axis, so there is no
  lever on it other than choosing pointer-vs-symbol per access — an axis s1-s5
  already swept exhaustively (23/24/26/27/29 for the mixes).

### s6-H2 — "the reloads die via cse.c:7329 declining to record a SET_DEST" (INHERITED from s2/s3)
- **statement.** cse forwards a store into a following load of the same byte
  unless `sets[i].src_elt == 0` (cse.c:7329), which happens only for a volatile
  MEM (`canon_hash` sets `do_not_record` under `MEM_VOLATILE_P`) — hence only a
  volatile access can produce target's four reloads.
- **probe.** Read `cse.fn` for which reads survive and what replaced the others.
- **result.** PARTIALLY REFUTED. The DICHOTOMY is confirmed but its mechanism is
  simpler and stronger than claimed: block 1's read survives as
  `(mem:QI (symbol_ref))` purely because the preceding store went through
  `(mem:QI (reg 73))` — a structurally different address rtx that `canon_hash`
  hashes differently — while blocks 2 and 3's reads are replaced by
  `(zero_extend:SI (reg/v:QI 75))` because their stores are the identical
  symbol_ref rtx. This is an ordinary hash hit in `cse_insn`, not the
  `src_elt == 0` path. The 7329 path remains volatile-only, so the volatile ruling
  is unaffected, but the non-volatile picture is now correctly stated.
- **verdict.** KILLED as stated; replaced by the address-rtx-identity model.
- **bonus fact.** The two surviving `zero_extend`-from-register insns ARE the two
  `andi a0,v1,0xff` truncations, i.e. the forward is also the entire 51-vs-49
  instruction excess. One defect, not two.

### s6-H3 — "the v0/v1 swap of the selected value is an allocation tie that some C spelling can steer" (the s5 frontier)
- **statement.** `val2` lands in `$v1` where target has `$v0` because its live
  range overlaps the condition temporary's; a spelling that separates them (or
  that shortens `val2`'s live range) will let `find_reg` give it `$v0`.
- **mechanism.** global.c `find_reg` over the `.greg` conflict lists.
- **probe.** Read the `.greg` allocation header for the floor form, then build six
  forms that alter exactly the live ranges involved (if/else both arms, ternary,
  positive-sense if, store duplicated into arms, val2-split, val-split,
  val+val2-split), dump and score each.
- **result.** KILLED. The binding constraint is `75 conflicts: … 2 …`, a conflict
  with HARD REG 2, not with the condition pseudo 76 — the if/else and ternary
  forms remove the 75-76 conflict outright (`75 conflicts: 72 74 75 2 29`) and
  `val2` still gets `$v1`. The hard reg is already taken by `local-alloc`, which
  assigns the block-local `p[8]` load pseudos to `$v0` (`lreg.fn`
  ";; Register 80 in 2. … 84 in 2. 88 in 2. 92 in 2.") before `global_alloc`
  runs. Splitting `val2` or `val` per block does not remove the conflict either —
  it merely moves the selected value to `$a0` — and every one of the six forms
  scores worse than 18 (24, 24, 23, 35, 24, 23, 29).
- **verdict.** KILLED. There is no independent register lever here. The swap is
  downstream of the address/forward decision, exactly like the missing reloads and
  the two extra truncations, and it will resolve with them or not at all.

## [s6] The target's unfolded lui+addiu base survives because its address expression feeds two memory operands; when an address expression feeds exactly one memory operand, combine folds %lo into that operand and the base register disappears. (Inherited from s1 evidence 'Residual defect 1', s3's candidate.c lever 1, and s4.)
- mechanism: A combine.c fold of the address into the mem operand.
- probe: cc1 -da dump of the CURRENT floor-18 body (tmp/grind/func_80034F88/s6/dump.sh + slice.py); read the PRE-CSE dump rtl.fn for the address rtx of every flag-byte access.
- result: REFUTED. rtl.fn lines 52/87/102/137/152/187 already carry (mem:QI (symbol_ref:SI ("D_80106A73"))) before any optimizer pass runs, and lines 29/37 already carry (mem:QI (reg/v:SI 73)) for the two *ptr accesses. The MIPS backend's GO_IF_LEGITIMATE_ADDRESS accepts a bare symbol_ref as an address, so the lui $at / %lo(...)($at) pair is manufactured at ASSEMBLY-OUTPUT time. combine is never involved; no pass converts a register base into a %lo operand or can be defeated into keeping one. The address form of each access is a one-to-one function of its C spelling at expansion time.
- verdict: KILLED

## [s6] The block-2/3 reloads die because cse.c:7329 records a SET_DEST unless sets[i].src_elt == 0, and canon_hash leaves src_elt at 0 only for a MEM_VOLATILE_P mem — hence only a volatile access can produce the target's four lbu reloads. (Inherited from s2/s3.)
- mechanism: cse.c:7308-7340 SET_DEST recording gated on src_elt.
- probe: Read cse.fn (the post-cse slice of the floor-18 dump) for which reads survive and what replaced the ones that did not.
- result: REFUTED AS STATED (the dichotomy is real, its mechanism is not). Block 1's read survives as (mem:QI (symbol_ref)) purely because the preceding store went through (mem:QI (reg 73)) — a structurally different address rtx that canon_hash hashes differently. Blocks 2 and 3's reads are replaced by (zero_extend:SI (reg/v:QI 75)) because their stores are the IDENTICAL symbol_ref rtx: an ordinary hash hit in cse_insn, not the src_elt == 0 path (which stays volatile-only, so the s3 volatile ruling is unaffected). Bonus: those two surviving zero_extend-from-register insns ARE the two andi a0,v1,0xff truncations, i.e. the forward is also the entire 51-vs-49 instruction excess — one defect, not two.
- verdict: KILLED

## [s6] The v0/v1 swap of the selected value is an allocation tie that some C spelling can steer: val2 lands in $v1 because its live range overlaps the condition temporary's, so a spelling that separates them or shortens val2's range will let find_reg give it $v0. (This was the ONLY frontier item s5 left, and the reason forensics was mandated.)
- mechanism: global.c find_reg over the .greg conflict lists.
- probe: Read the .greg allocation header for the floor form; then build, dump and honestly sandbox-score six forms that alter exactly the live ranges involved — if/else both arms, ternary with explicit (u8) casts, positive-sense if, store duplicated into both arms, val2 split per block, val split per block, val+val2 split per block (tmp/grind/func_80034F88/s6/forensic.py).
- result: KILLED. The binding constraint is '75 conflicts: ... 2 ...' — a conflict with HARD REG 2 — not a conflict with the condition pseudo 76. The if/else and ternary forms remove the 75-76 conflict outright ('75 conflicts: 72 74 75 2 29') and val2 STILL gets $v1; the condition pseudo additionally carries an explicit 'preferences: 2'. The hard reg was already taken one pass earlier by local-alloc, which assigns the block-local p[8] load pseudos to $v0 before global_alloc runs (lreg.fn ';; Register 80 in 2. ... 84 in 2. 88 in 2. 92 in 2.'). Splitting val2 or val per block does not remove the conflict either — it only moves the selected value to $a0 — and every one of the six forms scores worse than the floor: if/else 24/51, ternary 24/51, positive-sense if 23/49, store-duplicated-into-arms 35/61, val2-split 24/49, val-split 23/51, val+val2-split 29/49. There is no independent register lever; the swap is downstream of the address/forward decision and will resolve with it or not at all.
- verdict: KILLED


---

## s7 — forensics modality

## [s7] The flag section's SHAPE — a loop over a bit index rather than three unrolled blocks — has never been varied, and a peeled loop would be a genuinely new chassis whose reads were born un-forwardable.
- mechanism: cse's value table is invalidated across a loop back-edge (cse_end_of_basic_block terminates the block at the loop's CODE_LABEL), so a loop-carried read of the flag byte cannot be forwarded from the previous iteration's store. If GCC then peels or unrolls, the peeled form is a new chassis; if it emits a real loop, the shape is refuted by the objdump.
- probe: v9_loop_bitindex (`for (k = 1; k <= 4; k <<= 1)`, pointer accesses) and v10_loop_bitindex_sym (same, plain symbol accesses), both scored with `sandbox func_80034F88 --disable all`, plus a cc1 -da dump of v9 (tmp/grind/func_80034F88/s7/rtl/v9_loop_bitindex/).
- result: KILLED as a shape, CONFIRMED as a mechanism. Both spellings score 31, at 36 and 37 insns against the target's 49: GCC 2.7.2 at -O2 without -funroll-loops neither unrolls nor peels, so the objdump contains one real loop body with a single flag store where the target has three. The cse half of the hypothesis is exactly right, and is the reusable half: v9's cse2.fn still carries `(mem:QI (reg/v:SI 73))` for the loop-carried read, i.e. the back-edge CODE_LABEL boundary survives BOTH cse passes. Banked at rejected/loop-bitindex-shape-score31.c.
- verdict: KILLED

## [s7] The target's four pointer-spelled flag accesses can be reproduced by keeping the address pseudos in DIFFERENT cse quantity classes, e.g. via copy chains, per-block re-derivation, or pointer arithmetic that folds back to the same address (the s6 frontier's canon_reg question).
- mechanism: insert_regs (cse.c:1006-1042) merges a new address pseudo into an existing quantity only when its SET_SRC hashes equal to a REG already in that class, and canon_reg (cse.c:2532-2574) then rewrites the later pseudo to the oldest register of the quantity, which is what makes the store and the following read share one address rtx and forward. Two address expressions whose rtxs differ cannot be merged.
- probe: seven forms scored honestly — v4 three pointer locals all `= &D_80106A73`; v7 copy chain `ptr2 = ptr1; ptr3 = ptr2;`; v5/v6 three distinct symbol+addend pointers ((u8*)&D_80106A70+3, (u8*)&D_80106A71+2) hoisted and staged; w1 four distinct pointers; w2/w3 hybrids of the floor chassis with distinct pointers for blocks 2/3. cc1 -da dump of v6 and a difflib side-by-side of w2 against target.
- result: The mechanism is CONFIRMED exactly as read from the source — v4 (21) and v7 (21) merge to a single base (lui 2, lbu 2), while v5/v6 keep regs 73/74/75 in three separate quantities and materialise every reload (lbu 4, lui 4). The AXIS is KILLED as a source of gain: the best member, w2, TIES the floor at 18 with the target's exact access counts (lbu 5 / sb 5), and the side-by-side shows blocks 2 and 3 matching target instruction-for-instruction except for register names and the LO16 addend (`addiu a2,a2,3` vs `addiu a0,a0,0`). The non-zero addend each distinct rtx must carry costs precisely what its recovered reload gains; four distinct bases (w1) cost more than they buy (28). An addend-0 distinct rtx would need a second declared symbol at 0x80106A73 — the forbidden alias-rename family. Banked at rejected/hybrid-distinct-addend-score18-TIE.c and rejected/distinct-symbol-addend-four-bases-score28.c.
- verdict: KILLED

## [s7] A do-while(0) wrap (or any construct whose cse effect is a loop note) creates a basic-block boundary that stops the store-to-load forwarding — s2 measured the family dead but never established why, so a better-placed spelling might still work.
- mechanism: cse_end_of_basic_block breaks at NOTE_INSN_LOOP_END (cse.c:8054), which should flush the value table between the flag blocks.
- probe: v2 (do-while(0) + three pointer locals), v3 (do-while(0) + one pointer local), v8 (do-while(0) on the floor chassis), scored honestly; then a cc1 -da dump of v3 with the func region sliced out of every pass (tmp/grind/func_80034F88/s7/rtl/v3_dw_one_ptr/fn/).
- result: KILLED with the mechanism named, so no respelling can revive it. The boundary DOES work in cse1 — v3's cse.fn still contains the block-2 and block-3 reloads as `(set (reg 85) (mem:QI (reg/v:SI 73)))` — and is then undone by cse2: the break at cse.c:8054 is guarded by `! after_loop`, and the comment at 8051-8052 is explicit ("If we are running after loop.c has finished, we can ignore the NOTE_INSN_LOOP_END"). cse2 runs after loop.c, re-merges the flag section into one basic block and forwards every store into the following read; cse2.fn has no surviving flag-byte reload. Scores 23/23/20. Only a CODE_LABEL boundary (cse.c:8039, unconditional in both passes) survives. Banked at rejected/dowhile0-boundary-undone-by-cse2-score23.c.
- verdict: KILLED

## [s7] The target's combination of three addend-0 address materialisations AND four surviving lbu reloads is unreachable in non-volatile pure C (the s3-H4 / s6 model that the whole F3 escalation packet rests on).
- mechanism: s3-H4 claimed the dichotomy is total — two C address expressions for one byte either hash the same (merge, one base, store forwards, reload dies) or hash differently (no forward, but a separate base each). s6 restated it as an address-rtx-identity model fixed at RTL expansion.
- probe: p1_loop1_per_block — each flag block wrapped in `for (j = 0; j < 1; j++)` with three pointer locals, each assigned inside the previous block; honest sandbox score plus objdump census of lbu / sb / lui and their relocations.
- result: REFUTED. The form emits lbu 5 / sb 5 / lui 4 — the target's exact counts — with three `lui %hi(D_80106A73)` + `addiu ...,%lo(D_80106A73)` bases at ADDEND 0 and a real `lbu 0(base)` for every flag read, entirely without volatile. The dichotomy is not a property of C address expressions but of cse's basic blocks: a CODE_LABEL boundary flushes the value table, which at once stops the forward AND makes the next `ptr = &D_80106A73;` a non-redundant set that emits its own address pair. The residual 36 points are entirely the loop scaffolding (`addiu a1,a1,1` / `blez a1,...` / `move a1,zero` per block, 9 insns, 58 vs 49). `for (i = 0; i < 1; i++)` is a NOT-sanctioned spelling per the non-extension clause of .claude/rules/no-new-park-categories.md, so the form is an instrument, not a candidate. Banked at rejected/loop1-per-block-TARGET-SIGNATURE-score36.c. The F3 packet must be rewritten: the open question is no longer "can non-volatile C produce the reloads" (it can) but "is there a zero-instruction-cost construct that puts a cse2-surviving CODE_LABEL between the flag blocks".
- verdict: KILLED

## [s7] The flag section's SHAPE — a loop over a bit index rather than three unrolled blocks — is the last untried shape, and a peeled/unrolled loop would be a new chassis whose reads were born un-forwardable.
- mechanism: cse_end_of_basic_block terminates the basic block at the loop's CODE_LABEL, so a loop-carried read of the flag byte cannot be forwarded from the previous iteration's store. If GCC peels or unrolls, the peeled form is a new chassis; if it emits a real loop, the shape is refuted by the objdump.
- probe: v9_loop_bitindex (`for (k = 1; k <= 4; k <<= 1)`, pointer accesses) and v10_loop_bitindex_sym (same with plain symbol accesses), both scored with `sandbox func_80034F88 --disable all`, plus a cc1 -da dump of v9 sliced per pass.
- result: KILLED as a shape, CONFIRMED as a mechanism. Both spellings score 31 at 36 and 37 insns against target's 49 — GCC 2.7.2 at -O2 without -funroll-loops neither unrolls nor peels, so the objdump holds one real loop body with a single flag store where the target has three. The cse half is exactly right and is the reusable half: v9's cse2.fn still carries (mem:QI (reg/v:SI 73)) for the loop-carried read, i.e. a back-edge CODE_LABEL boundary survives BOTH cse passes. Banked at rejected/loop-bitindex-shape-score31.c.
- verdict: KILLED

## [s7] The target's four pointer-spelled flag accesses can be reproduced by keeping the address pseudos in DIFFERENT cse quantity classes (copy chains, per-block re-derivation, or pointer arithmetic folding back to the same address) — the s6 frontier's canon_reg question.
- mechanism: insert_regs (cse.c:1006-1042) merges a new address pseudo into an existing quantity only when its SET_SRC hashes equal to a REG already in that class, and canon_reg (cse.c:2532-2574) then rewrites the later pseudo to the oldest register of the quantity — which is exactly what makes a store and a following read share one address rtx and forward.
- probe: Seven honest-scored forms: v4 (three locals all = &D_80106A73), v7 (copy chain ptr2 = ptr1), v5/v6 (three distinct symbol+addend pointers, hoisted and staged), w1 (four distinct), w2/w3 (floor chassis hybridised with distinct pointers for blocks 2/3); plus a cc1 -da dump of v6 and a difflib side-by-side of w2 against target.
- result: Mechanism CONFIRMED verbatim from the source reading — v4 (21) and v7 (21) merge to a single base (lui 2, lbu 2), while v5/v6 keep regs 73/74/75 in three separate quantities and materialise every reload (lbu 4, lui 4). The AXIS is KILLED as a source of gain: the best member (w2) TIES the floor at 18 with the target's exact access counts, and the side-by-side shows blocks 2 and 3 matching target instruction-for-instruction except for register names and the LO16 addend (addiu a2,a2,3 vs addiu a0,a0,0). Each distinct rtx must carry a non-zero addend, which costs precisely what its recovered reload gains; four distinct bases (w1) cost more than they buy (28). An addend-0 distinct rtx would require a second declared symbol at 0x80106A73 — the forbidden alias-rename family. Banked at rejected/hybrid-distinct-addend-score18-TIE.c and rejected/distinct-symbol-addend-four-bases-score28.c.
- verdict: KILLED

## [s7] A do-while(0) wrap — or any construct whose cse effect is a loop note — creates a basic-block boundary that stops the store-to-load forwarding; s2 measured the family dead empirically but never established why, so a better-placed spelling might still work.
- mechanism: cse_end_of_basic_block breaks at NOTE_INSN_LOOP_END (cse.c:8054), which should flush the value table between the flag blocks.
- probe: v2 (do-while(0) + three pointer locals), v3 (do-while(0) + one pointer local), v8 (do-while(0) on the floor chassis), honestly scored; then a cc1 -da dump of v3 with the function region sliced out of every pass dump (s7/rtl/v3_dw_one_ptr/fn/).
- result: KILLED with the mechanism named, so no respelling can revive it. The boundary genuinely works in cse1 — v3's cse.fn still contains block 2's and block 3's reloads as (set (reg 85) (mem:QI (reg/v:SI 73))) — and is then undone by cse2: the 8054 break is guarded by `! after_loop` and the comment at cse.c:8051-8052 is explicit ('If we are running after loop.c has finished, we can ignore the NOTE_INSN_LOOP_END'). cse2 runs after loop.c, re-merges the flag section into one basic block and forwards every store into the following read; cse2.fn has no surviving flag-byte reload. Scores 23 / 23 / 20. Only a CODE_LABEL boundary (cse.c:8039, unconditional in both passes) survives. Banked at rejected/dowhile0-boundary-undone-by-cse2-score23.c.
- verdict: KILLED

## [s7] The target's combination of three addend-0 address materialisations AND four surviving lbu reloads is unreachable in non-volatile pure C — the s3-H4 / s6 address-rtx-identity model that the entire F3 escalation packet rests on.
- mechanism: s3-H4 claimed a total dichotomy: two C address expressions for one byte either hash the same (merge, one shared base, the store forwards, the reload dies) or hash differently (no forward, but a separate base each). s6 restated it as an address-rtx identity fixed at RTL expansion time.
- probe: p1_loop1_per_block — each flag block wrapped in `for (j = 0; j < 1; j++)` with three pointer locals, each assigned inside the previous block; honest sandbox score plus an objdump census of lbu / sb / lui and their relocations.
- result: REFUTED. The form emits lbu 5 / sb 5 / lui 4 — the target's exact counts — with three lui %hi(D_80106A73) + addiu %lo(D_80106A73) bases at ADDEND 0 and a real lbu 0(base) for every flag read, entirely without volatile. The dichotomy is a property not of C address expressions but of cse's basic blocks: a CODE_LABEL boundary flushes the value table, which at once stops the forward AND makes the next `ptr = &D_80106A73;` a non-redundant set that emits its own address pair. The residual 36 points are entirely the loop scaffolding (addiu a1,a1,1 / blez a1,... / move a1,zero per block; 58 insns vs 49). `for (i = 0; i < 1; i++)` is a NOT-sanctioned spelling per the non-extension clause of .claude/rules/no-new-park-categories.md, so the form is a measurement instrument, never a candidate. Banked at rejected/loop1-per-block-TARGET-SIGNATURE-score36.c.
- verdict: KILLED


---

## s8 � rederive modality

## [s8] There is a SECOND cse mechanism, unrelated to the CODE_LABEL boundary s7 identified, that produces the target's four lbu reloads in non-volatile C at ZERO instruction cost: a MEM store placed inside a conditionally-skipped block.
- mechanism: cse_end_of_basic_block (cse.c:8102-8184) classifies a forward conditional branch over a block as AROUND when the branch's target label has LABEL_NUSES == 1 and there is no CODE_LABEL between the branch and the insn preceding that label; cse_basic_block then calls invalidate_skipped_block (cse.c:7843-7867), which runs note_stores over every insn of the skipped block with invalidate_skipped_set (cse.c:7810-7836). That callback INVALIDATES a MEM destination (note_mem_written + invalidate) instead of recording it, so a store inside the arm never enters the value table and the following read of the same byte cannot be forwarded. No volatile, no label boundary, no extra instruction.
- probe: 16 structurally new bodies through tmp/grind/func_80034F88/s8/probe.py, the relevant family being the conditional store: n1 `if (p[8]&K) *ptr |= K;` (26/47), n3 the same with three staged pointer locals (26/47), m1 `val = *ptr; if (c) *ptr = val|K;` (18/42), m2 the s32-widened spelling (18/42), m5 three staged pointers (18/42), m7 condition-after-read (18/42); plus a cc1 -da dump of m1 sliced per pass at tmp/grind/func_80034F88/s8/rtl/m1_ptr_read_condstore/fn/.
- result: CONFIRMED as a mechanism and measured exactly. m1 emits lbu 4 / sb 5 with ONE lui+addiu base and 42 instructions, and its cse2.fn still carries `(mem:QI (reg/v:SI 73))` for every flag read - i.e. the reloads survive BOTH cse passes with no volatile and no label boundary, which s7's model said required a CODE_LABEL. Best score 18: a TIE with the floor, never a drop. Banked at rejected/condstore-AROUND-reloads-free-score18-TIE.c.
- verdict: CONFIRMED

## [s8] The conditional-store (AROUND) family can be shaped into the target, i.e. it is a usable chassis and not merely an instrument.
- mechanism: If the free reloads can be combined with the target's three addend-0 address materialisations and its value-select move, the chassis would be 7 insns richer and land on the target.
- probe: Read the target's own instruction stream (asm/funcs/func_80034F88.s) for the store shape, then measured every hybrid: m6 (block 1 floor chassis + blocks 2/3 conditional store) 23/45, n7 (same, symbol-spelled) 21/48, m3 symbol read + pointer conditional store 29/45, m4 pointer read + symbol conditional store 28/45, m8/m9 symbol-spelled mask + pointer blocks 33/44.
- result: KILLED, and by a proof rather than a plateau. The AROUND mechanism REQUIRES the store to sit inside the conditional arm; the target's store is unconditional - 0x80034FC4 is `addu $v0,$a0,$zero` on the fall-through path (the else-value of a select) and each block has a single `sb` AFTER its join label. A conditional-store body is therefore 7 instructions short of the target by construction (3 missing value-select moves + 4 missing address-materialisation insns) and every hybrid that restores part of the target's shape gives back more than it gains. The two reload mechanisms are mutually exclusive on this function: conditional store buys free reloads with the wrong store shape; unconditional store has the target's shape but the store is recorded, so only a cse2-surviving CODE_LABEL boundary can stop the forward.
- verdict: KILLED

## [s8] The original source is the obvious human C - plain symbol accesses with a conditional set (`D_80106A73 &= 0xF8; if (p[8] & 1) D_80106A73 |= 1;` ...).
- mechanism: A rederive-modality sanity check: the most natural spelling of the function's specification, never measured in s1-s7 because every prior session started from the inherited pointer/select chassis.
- probe: n2_condstore_sym through s8/probe.py.
- result: KILLED at 35 / 51 insns with lui 8. Every flag access is a bare `(mem:QI (symbol_ref))` that the MIPS backend materialises as its own `lui $at` + `%lo(...)($at)` pair, so the body carries four more lui than the target's four and no shared base at all. The natural spelling is the worst member of the conditional-store family. Banked at rejected/condstore-natural-symbol-score35.c.
- verdict: KILLED

## [s8] Duplicating the STORE into both arms (the sanctioned duplicated-statement-into-arms family) puts a MEM store inside a cse-skipped block while jump2's find_cross_jump merges the two identical tails back into the target's single `sb`.
- mechanism: cse runs before jump2, so it sees two conditional stores and invalidates the skipped one; find_cross_jump (jump.c) then merges identical block suffixes, which should restore the target's one-store-per-block shape byte-neutrally. This is the shape s6 measured at 35 with a symbol store, but never with a pointer store nor with the value staged through a named local.
- probe: q1 (val2 staged, pointer store) 19/46, q2 (direct store expression, pointer) 19/46, q3 (val2, symbol) 35/61, q4 (pointer read, symbol store) 36/57, q5 (symbol read, pointer store) 24/51, q6 (block 1 floor + blocks 2/3 duplicated) 27/56.
- result: KILLED. The reloads do come back (lbu 4) but find_cross_jump merges only ONE of the three duplicate pairs, so the best member carries 7 `sb` where the target has 5 and scores 19 - worse than the floor. Symbol-spelled duplicates are far worse (57-61 insns) because each duplicated store drags its own lui/%lo pair. Banked at rejected/dupstore-into-arms-score19.c.
- verdict: KILLED

## [s8] A C control-flow shape exists that gives the join label between two flag blocks LABEL_NUSES != 1 (or otherwise ends the cse basic block there) at ZERO instruction cost - the s7 frontier's central question.
- mechanism: cse_end_of_basic_block extends the block through a forward conditional branch only when ALL of: follow_jumps/skip_blocks on, path_size < PATHLENGTH-1, the pattern is an IF_THEN_ELSE SET, `LABEL_NUSES (JUMP_LABEL (p)) == 1`, and NEXT_INSN(label) != 0; then either the insn before the label is a BARRIER (follow_jumps, status TAKEN) or it is not a CODE_LABEL and there is no CODE_LABEL between the branch and it (skip_blocks, status AROUND). Failing all of those, the outer `while (p && GET_CODE (p) != CODE_LABEL)` loop terminates the block AT the join label - the one boundary that survives cse2 (the NOTE_INSN_LOOP_END break at cse.c:8053 is guarded by `! after_loop`).
- probe: Read cse.c:8025-8190 line by line and enumerated the three escapes, then measured the two that are expressible without adding a branch: n4, an explicit two-label goto diamond per block (`if (c) goto setK; val2 = val; goto stK; setK: val2 = val|K; stK: store;`), and n8, an arm ending with `goto stK` where stK is the shared store label.
- result: KILLED for every zero-cost spelling, with the reason named for each escape. (a) LABEL_NUSES >= 2 needs a second LABEL_REF, i.e. a second jump insn; every C spelling that adds one either adds a real branch the target does not have, or the added jump targets the immediately following label and jump.c deletes it BEFORE cse1 runs, taking LABEL_NUSES back to 1 - measured: n4 scores 22 at 51 insns with lbu 3, exactly the plain-diamond signature, i.e. no boundary was created; n8 scores 23/49, same. (b) A CODE_LABEL inside the arm does block the AROUND extension, but an unreferenced C label is a dead-goto label pad - the forbidden family - and a referenced one costs its own jump. (c) The follow_jumps BARRIER escape needs the arm to end in an unconditional jump, which is the if/else shape, and there the block is FOLLOWED rather than ended (status TAKEN, no invalidation). Banked at rejected/goto-diamond-no-boundary-score22.c.
- verdict: KILLED

## [s8] The target's THREE addend-0 `lui`+`addiu` materialisations of &D_80106A73 can be obtained independently of the reload question, by staging three pointer locals in the conditional-store chassis where the mem is already un-forwardable.
- mechanism: If the reload no longer depends on the address rtx (the AROUND path having supplied it), the three address locals only have to survive cse's redundant-set deletion.
- probe: n3 (three staged pointers + conditional stores) and m5 (three staged pointers + read-then-conditional-store); objdump lui census on both.
- result: KILLED. Both emit lui 2 - one flag base plus the copy loop's - i.e. cse deletes the second and third `ptr = &D_80106A73;` as redundant sets exactly as it does in the floor chassis. invalidate_skipped_block only invalidates registers SET INSIDE the skipped arm, and the pointer locals are set at top level, so nothing removes their value from the table. Consequence, and it is the sharpest statement of the remaining gap: the target's three addend-0 bases and its four reloads BOTH require the same thing - a cse basic-block boundary between the flag blocks - and neither is obtainable without it. Addend-carrying distinct rtxs (s7's w2 family) remain the only alternative and they pay back exactly what they gain.
- verdict: KILLED

## [s8] There is a second cse mechanism, unrelated to the CODE_LABEL boundary s7 identified, that produces the target's four lbu reloads in non-volatile C at ZERO instruction cost: a MEM store placed inside a conditionally-skipped block.
- mechanism: cse_end_of_basic_block (cse.c:8102-8184) classifies a forward conditional branch over a block as AROUND; cse_basic_block then calls invalidate_skipped_block (cse.c:7843-7867), which runs note_stores with invalidate_skipped_set (cse.c:7810-7836). That callback INVALIDATES a MEM destination instead of recording it, so a store inside the arm never enters the value table and the following read of the same byte cannot be forwarded - no volatile, no label, no extra instruction.
- probe: 16 new bodies through tmp/grind/func_80034F88/s8/probe.py; the family is n1 'if (p[8]&K) *ptr |= K;' (26/47), n3 same with three staged pointers (26/47), m1 'val = *ptr; if (c) *ptr = val|K;' (18/42), m2 s32-widened (18/42), m5 three staged pointers (18/42), m7 condition-after-read (18/42); plus a cc1 -da dump of m1 sliced per pass.
- result: m1 emits lbu 4 / sb 5 with ONE lui+addiu base at 42 insns, and cse2.fn still carries (mem:QI (reg/v:SI 73)) for every flag read - the reloads survive BOTH cse passes with no volatile and no label boundary, which s7's model said required a CODE_LABEL. Best score 18: a tie with the floor, never a drop.
- verdict: CONFIRMED

## [s8] The conditional-store (AROUND) family is a usable chassis for this target, not merely an instrument.
- mechanism: If the free reloads could be combined with the target's three addend-0 address materialisations and its value-select move, the chassis would be 7 insns richer and land on the target.
- probe: Read the target's own instruction stream, then measured every hybrid: m6 (block 1 floor chassis + blocks 2/3 conditional store) 23/45, n7 21/48, m3 29/45, m4 28/45, m8/m9 33/44.
- result: The AROUND mechanism REQUIRES the store inside the arm; the target's store is unconditional - asm/funcs/func_80034F88.s has 'addu $v0,$a0,$zero' at 0x80034FC4 (the else-value of a select on the fall-through path) and exactly one 'sb' per block AFTER the join label. A conditional-store body is 7 insns short by construction (3 missing value-select moves + 4 missing address-materialisation insns) and every hybrid gives back more than it gains. The two reload mechanisms are mutually exclusive here.
- verdict: KILLED

## [s8] The original source is the obvious human C: plain symbol accesses with a conditional set ('D_80106A73 &= 0xF8; if (p[8] & 1) D_80106A73 |= 1;' ...).
- mechanism: Rederive-modality sanity check - the most natural spelling of the function's specification, never measured in s1-s7 because every prior session started from the inherited pointer/select chassis.
- probe: n2_condstore_sym through s8/probe.py.
- result: 35 at 51 insns with lui 8. Every flag access is a bare (mem:QI (symbol_ref)) that the MIPS backend materialises as its own lui $at + %lo($at) pair, so the body carries four more lui than the target's four and no shared base at all - the worst member of the conditional-store family.
- verdict: KILLED

## [s8] Duplicating the STORE into both arms (the sanctioned duplicated-statement-into-arms family) puts a MEM store inside a cse-skipped block while jump2's find_cross_jump merges the two identical tails back into the target's single 'sb'.
- mechanism: cse runs before jump2, so it sees two conditional stores and invalidates the skipped one; find_cross_jump then merges identical block suffixes, which should restore the target's one-store-per-block shape byte-neutrally. s6 measured this only with a symbol store and no staged local.
- probe: q1 (val2 staged, pointer store) 19/46, q2 (direct store expression, pointer) 19/46, q3 (val2, symbol) 35/61, q4 28-36/57, q5 24/51, q6 (block 1 floor + blocks 2/3 duplicated) 27/56.
- result: The reloads come back (lbu 4) but find_cross_jump merges only ONE of the three duplicate pairs, so the best member carries 7 sb where the target has 5 and scores 19 - worse than the floor. Symbol-spelled duplicates are far worse (57-61 insns) because each duplicated store drags its own lui/%lo pair.
- verdict: KILLED

## [s8] A C control-flow shape exists that gives the join label between two flag blocks LABEL_NUSES != 1 (or otherwise ends the cse basic block there) at ZERO instruction cost - the s7 frontier's central question.
- mechanism: cse_end_of_basic_block extends the block through a forward conditional branch only when LABEL_NUSES (JUMP_LABEL (p)) == 1 AND either the insn before the label is a BARRIER (follow_jumps/TAKEN) or it is not a CODE_LABEL and no CODE_LABEL lies between the branch and it (skip_blocks/AROUND). Failing all of those, the outer 'while (p && GET_CODE (p) != CODE_LABEL)' loop ends the block AT the join label - the one boundary that survives cse2, since the NOTE_INSN_LOOP_END break at cse.c:8053 is guarded by '! after_loop'.
- probe: Read cse.c:8025-8190 line by line, enumerated the three escapes, and measured the two expressible without adding a branch: n4 (explicit two-label goto diamond per block) and n8 (arm ending with 'goto stK' to the shared store label).
- result: Every zero-cost spelling is dead, with the reason named per escape. (a) LABEL_NUSES >= 2 needs a second LABEL_REF, i.e. a second jump insn; when the added jump targets the immediately following label jump.c deletes it BEFORE cse1 and LABEL_NUSES falls back to 1 - measured n4 22/51 and n8 23/49, both with lbu 3, the plain-diamond signature, i.e. no boundary was created. (b) A CODE_LABEL inside the arm does defeat the AROUND extension, but an unreferenced C label is a dead-goto label pad (forbidden family) and a referenced one costs its own jump. (c) The BARRIER escape needs the arm to end in an unconditional jump, i.e. the if/else shape, where the block is FOLLOWED rather than ended and nothing is invalidated.
- verdict: KILLED

## [s8] The target's three addend-0 lui+addiu materialisations of &D_80106A73 can be obtained independently of the reload question, by staging three pointer locals in the conditional-store chassis where the mem is already un-forwardable.
- mechanism: If the reload no longer depends on the address rtx, the three address locals only have to survive cse's redundant-set deletion.
- probe: n3 and m5 (three staged pointer locals in the conditional-store chassis), objdump lui census on both.
- result: Both emit lui 2 - one flag base plus the copy loop's - i.e. cse still deletes the second and third 'ptr = &D_80106A73;' as redundant sets, because invalidate_skipped_block only invalidates registers SET INSIDE the skipped arm and the pointer locals are set at top level. The target's three addend-0 bases and its four reloads therefore require the SAME thing - a cse basic-block boundary - so s7's two frontier items are one item.
- verdict: KILLED


---

## s9 - rederive modality

## [s9] The `&&`-condition boundary pays for itself: `if (!c && X)` gives the join label LABEL_NUSES 2, ends the cse basic block there, and should recover one reload plus one addend-0 base per block (~3 insns) against a 1-insn branch cost. (The single frontier item s8 left with a number attached.)
- mechanism: expand_expr for TRUTH_ANDIF_EXPR emits jumpifnot on EACH operand to the SAME false-label, so the join label carries two LABEL_REFs and cse_end_of_basic_block's extension test at tools/gcc-2.7.2/cse.c:8112 (`LABEL_NUSES (JUMP_LABEL (p)) == 1`) declines; neither the BARRIER/TAKEN escape nor the AROUND escape can fire, so the outer `while (p && GET_CODE (p) != CODE_LABEL)` loop terminates the block AT the join label - the one boundary that survives cse2. The second operand used is `ptrN`, a pointer local holding &D_80106A73: always true, so the form is semantically neutral, and GCC 2.7.2 has no SSA/VRP to fold a variable's non-nullness away. It is an INSTRUMENT, never a candidate (cheat-checklist T1/T2: no semantic purpose).
- probe: 18 forms through tmp/grind/func_80034F88/s9/{gen,gen2,gen5}.py + probe.py. Wave A on the floor chassis (a1 all three blocks 29/57, a2 blocks 2+3 27/55, a3 block 1 25/53, a5 the nested-`if (ptr)` spelling of the same boundary 29/57). Wave B/C on a staged-pointer chassis with the target's own materialisation ORDER (ptr2 assigned after block 1's join and before block 1's store, ptr3 after block 2's store): b0 no guard 21/47 lbu 2 lui 2, c1 guard on block 1 only 23/50 lbu 3 lui 3, c2 block 2 only 29/50, c3 block 3 only 23/49 (no effect - there is no fourth base to rematerialise), c4 blocks 1+2 31/53 lbu 4 lui 4, b1 all three 33/55, b2 alternate staging placement 33/55, b3 nested-if spelling 33/55.
- result: CONFIRMED as a mechanism, KILLED as a lever, and this is the first time the trade has a number. The boundary genuinely works with the target's UNCONDITIONAL store - c4 emits lui 4 (three addend-0 `lui %hi(D_80106A73)` + `addiu %lo(D_80106A73)` bases plus the copy loop's) and lbu 4, i.e. exactly what s7/s8 said required either a volatile access, a loop-scaffolded label, or the conditional-store shape the target does not have. But it costs one conditional branch per boundary and the target contains exactly one branch per block: c4 is 53 insns against the target's 49 and scores 31 against the floor's 18. Marginal cost per boundary measured directly: b0 21 -> c1 23 (+2 score, +3 insns) for the first, c1 23 -> c4 31 for the second. Every member of the family is net-negative. Banked at rejected/andand-boundary-two-blocks-score31.c and rejected/andand-boundary-three-blocks-score33.c.
- verdict: KILLED

## [s9] The target's exact access signature (lbu 5 / sb 5) is reachable with the target's unconditional store, with NO boundary construct, NO volatile and NO loop scaffolding - by giving the two reads of a flag block DIFFERENT address SPELLINGS (symbol outside the arm, pointer inside it).
- mechanism: s1-H3's mismatch lever, applied WITHIN a block rather than between blocks. `val2 = D_80106A73 | K;` expands to `(mem:QI (symbol_ref))` and `val2 = *ptr;` inside the arm expands to `(mem:QI (reg))`; the two hash differently in canon_hash, so cse cannot satisfy the second from the first and both survive as real `lbu`. The store is unaffected and stays unconditional.
- probe: wave D/E, 13 forms (gen3.py/gen4.py): d6 symbol-outer + pointer-arm 29/47 lbu 5 sb 5 lui 6; e1 pointer-outer + symbol-arm 28/48 lbu 5 lui 5; e2 d6 with the store through the pointer 24/48 lbu 7 lui 5; e3 d6 + s32 staging 25/47 lbu 5 lui 6; e4 28/48; e5 28/48; e6 mask spelled through the symbol 30/47 lui 7; e7 block-1-as-floor hybrid 25/47.
- result: CONFIRMED as a mechanism, KILLED as a lever. The reload count is free, but every symbol-spelled access carries its own `lui $at` + `%lo(...)($at)` pair, so the family lands at lui 5-7 where the target has 4, and the best member (e2, seven surviving reloads) scores 24 - six worse than the floor. This is the exact complement of the boundary family: mismatched SPELLING buys reloads and pays in address materialisation; a cse block BOUNDARY buys shared addend-0 bases and pays in branches. Banked at rejected/armread-ptr-mismatch-lbu5-score29.c and rejected/armread-ptr-storeptr-lbu7-score24.c.
- verdict: KILLED

## [s9] Combining the two mechanisms yields the target's complete signature at a cost the score can absorb.
- mechanism: two `&&` boundaries (for the addend-0 bases and the block-2/3 reloads) plus the floor's pointer-mask / symbol-read mismatch (for the block-1 reload) should give lbu 5 / sb 5 / lui 4 in a single body.
- probe: wave F (gen5.py): f1 boundaries on blocks 1+2 with block 1's read symbol-spelled 32/54 lbu 5 sb 5 lui 5; f2 boundaries on all three 34/56; f3 all-pointer reads 31/53 lbu 4 lui 4.
- result: KILLED. f1 is the cheapest body ever measured that carries the target's lbu 5 / sb 5 counts alongside an unconditional store, and it scores 32 at 54 insns - fourteen worse than the floor. Its lui is 5, not 4, because the one read that must be symbol-spelled to mismatch the pointer mask drags its own materialisation. The session's sharpest statement: the target's 49 instructions need the mismatch property (for the reloads) AND one shared base per block (for the lui count) simultaneously, and the only construct supplying both is a cse basic-block boundary, whose cheapest C spelling costs one conditional branch per boundary - branches the target does not contain. Banked at rejected/full-target-signature-andand-score32.c.
- verdict: KILLED

## [s9] A fresh m2c re-derivation exposes a chassis no prior session measured: three per-block u8 selects with TWO separate symbol reads per block and no staging local.
- mechanism: rederive-modality mandate. `python3 tools/m2c/m2c.py --target mipsel-gcc-c asm/funcs/func_80034F88.s` reconstructs `var_v0 = D_80106A73 | 1; if (!(temp_a1->unk20 & 1)) { var_v0 = D_80106A73; } D_80106A73 = var_v0;` x3 plus a do-while copy loop with a walking pointer recomputed after the increment. Every session since s2 has carried a single `val` staging local instead, so the two-reads-per-block spelling had only ever been measured as "read duplicated into both arms" on a different chassis (s8, 28).
- probe: wave D, 6 forms (gen3.py): d1 m2c verbatim 29/45 lbu 2 lui 6; d2 m2c selects + the floor's pointer mask 21/48; d3 one shared u8 select var 21/50; d4 + condition staging 20/49; d5 + per-block vars 20/47; d6 the arm read through the pointer 29/47.
- result: KILLED as a chassis, and it is a clean negative for the rederive modality: m2c's own reconstruction scores 29, and its best hybrid with the floor's levers scores 20 - two worse than the floor's 18, at 47 insns (two FEWER than the target's 49). The two-reads spelling is not the missing structure. Note d5 at 47 insns and d1 at 45 confirm again that instruction count is uncorrelated with the honest score here. Banked at rejected/m2c-verbatim-score29.c and rejected/m2c-tworead-condfirst-score20.c.
- verdict: KILLED

## [s9] jump.c's thread_jumps (which runs immediately before cse1) can redirect a second branch onto an existing join label, raising its LABEL_NUSES to 2 without a new insn - the zero-cost boundary s8's enumeration could not reach.
- mechanism: toplev.c:2861 calls `thread_jumps (insns, max_reg_num (), 1)` inside the `if (optimize > 0)` block immediately BEFORE `cse_main (insns, ..., 0, cse_dump_file)`, so any label-use change it makes is visible to cse1's boundary test.
- probe: read toplev.c:2845-2870 to establish the pass order, then checked the precondition against the target's own instruction stream (asm/funcs/func_80034F88.s).
- result: KILLED on the precondition, without a measurement. thread_jumps only redirects a jump when the code at the destination is equivalent to the code at the redirect target under the same register state; the three join labels here are each followed by DIFFERENT code (block 1's join is followed by base2's materialisation and block 1's store; block 2's join by block 2's store and base3's materialisation; block 3's join by block 3's store and the copy-loop setup). There is no pair of equivalent destinations, so no redirection is available and no label's NUSES can rise without a new LABEL_REF - i.e. without a new jump insn. This closes the second of the two lines of attack s8's frontier named.
- verdict: KILLED

## [s9] The '&&'-condition boundary pays for itself: `if (!c && X)` gives the join label LABEL_NUSES 2, ends the cse basic block there, and should recover one reload plus one addend-0 base per block (~3 insns) against a 1-insn branch cost. This was the single frontier item s8 left with a number attached.
- mechanism: expand_expr for TRUTH_ANDIF_EXPR emits jumpifnot on EACH operand to the SAME false-label, so the join label carries two LABEL_REFs and cse_end_of_basic_block's extension test at tools/gcc-2.7.2/cse.c:8112 (`LABEL_NUSES (JUMP_LABEL (p)) == 1`) declines; neither the BARRIER/TAKEN escape nor the AROUND escape can then fire, so the outer `while (p && GET_CODE (p) != CODE_LABEL)` loop terminates the block AT the join label - the one boundary that survives cse2 (the NOTE_INSN_LOOP_END break at cse.c:8053 is guarded by `! after_loop`). The second operand used was `ptrN`, a pointer local holding &D_80106A73: always true, so the construct is semantically neutral and GCC 2.7.2 has no SSA/VRP to fold a variable's non-nullness away. It is an INSTRUMENT, never a candidate - cheat-checklist T1/T2, no semantic purpose.
- probe: 18 forms through tmp/grind/func_80034F88/s9/{gen,gen2,gen5}.py + probe.py, honest `sandbox func_80034F88 --disable all` plus an objdump lbu/sb/lui census. Wave A on the floor chassis (a1 all three blocks 29/57, a2 blocks 2+3 27/55, a3 block 1 25/53, a5 the nested-`if (ptr)` spelling 29/57). Waves B/C on a staged-pointer chassis reproducing the target's own base-materialisation ORDER (ptr2 assigned after block 1's join and before block 1's store, ptr3 after block 2's store): b0 no guard 21/47 lbu 2 lui 2, c1 guard on block 1 only 23/50 lbu 3 lui 3, c2 block 2 only 29/50, c3 block 3 only 23/49, c4 blocks 1+2 31/53 lbu 4 lui 4, b1 all three 33/55, b2 alternate staging placement 33/55, b3 nested-if spelling 33/55.
- result: CONFIRMED as a mechanism, KILLED as a lever, and the trade now has a number. The boundary genuinely works alongside the target's UNCONDITIONAL store - c4 emits lui 4 (three addend-0 `lui %hi(D_80106A73)` + `addiu %lo(D_80106A73)` bases plus the copy loop's) and lbu 4, which s7/s8 believed required a volatile access, loop scaffolding, or the conditional-store shape the target does not have. But it costs one conditional branch per boundary and the target contains exactly one branch per block: c4 is 53 insns against the target's 49 and scores 31 against the floor's 18. Marginal cost measured directly: 21 -> 23 for the first boundary, 23 -> 31 for the second, 31 -> 33 for the third (which buys nothing - there is no fourth base). Every member is net-negative.
- verdict: KILLED

## [s9] The target's exact access signature (lbu 5 / sb 5) is reachable with the target's unconditional store, with NO boundary construct, NO volatile and NO loop scaffolding - by giving the two reads of a flag block DIFFERENT address SPELLINGS (symbol outside the arm, pointer inside it).
- mechanism: s1-H3's mismatch lever applied WITHIN a block instead of between blocks. `val2 = D_80106A73 | K;` expands to `(mem:QI (symbol_ref))` while `val2 = *ptr;` inside the arm expands to `(mem:QI (reg))`; the two hash differently in canon_hash, so cse cannot satisfy the second from the first and both survive as real `lbu`. The store is untouched and stays unconditional.
- probe: Waves D and E, 13 forms (gen3.py / gen4.py): d6 symbol-outer + pointer-arm 29/47 lbu 5 sb 5 lui 6; e1 pointer-outer + symbol-arm 28/48 lbu 5 lui 5; e2 d6 with the store through the pointer 24/48 lbu 7 lui 5; e3 d6 + s32 staging 25/47; e4 28/48; e5 28/48; e6 mask through the symbol 30/47 lui 7; e7 block-1-as-floor hybrid 25/47.
- result: CONFIRMED as a mechanism, KILLED as a lever. The reloads are free, but every symbol-spelled access carries its own `lui $at` + `%lo(...)($at)` pair, so the family lands at lui 5-7 where the target has 4, and its best member (e2, seven surviving reloads) scores 24 - six worse than the floor. This is the exact complement of the boundary family: mismatched SPELLING buys reloads and pays in address materialisation; a cse block BOUNDARY buys shared addend-0 bases and pays in branches.
- verdict: KILLED

## [s9] Combining the two mechanisms yields the target's complete access signature at a cost the score can absorb.
- mechanism: Two `&&` boundaries (for the three addend-0 bases and the block-2/3 reloads) plus the floor form's pointer-mask / symbol-read mismatch (for the block-1 reload) should put lbu 5 / sb 5 / lui 4 into a single body.
- probe: Wave F (gen5.py): f1 boundaries on blocks 1+2 with block 1's read symbol-spelled 32/54 lbu 5 sb 5 lui 5; f2 boundaries on all three 34/56; f3 all-pointer reads 31/53 lbu 4 lui 4.
- result: KILLED. f1 is the cheapest body ever measured that carries the target's lbu 5 / sb 5 counts alongside an unconditional store, and it scores 32 at 54 insns - fourteen worse than the floor. Its lui is 5, not 4, because the one read that must be symbol-spelled to mismatch the pointer mask drags its own materialisation. Sharpest statement of the wall: the target's 49 instructions need the mismatch property (for the reloads) AND one shared base per block (for the lui count) at the same time, and the only construct supplying both is a cse basic-block boundary, whose cheapest C spelling costs one conditional branch per boundary - branches the target does not contain.
- verdict: KILLED

## [s9] A fresh m2c re-derivation exposes a chassis no prior session measured: three per-block u8 selects with TWO separate symbol reads per block and no staging local.
- mechanism: Rederive-modality mandate. `python3 tools/m2c/m2c.py --target mipsel-gcc-c asm/funcs/func_80034F88.s` reconstructs `var_v0 = D_80106A73 | 1; if (!(temp_a1->unk20 & 1)) { var_v0 = D_80106A73; } D_80106A73 = var_v0;` three times plus a do-while copy loop with a walking pointer recomputed after the increment. Every session since s2 has carried a single `val` staging local instead, so the two-reads-per-block spelling had only ever been measured as 'read duplicated into both arms' on a different chassis (s8, 28).
- probe: Wave D, 6 forms (gen3.py): d1 m2c verbatim 29/45 lbu 2 lui 6; d2 m2c selects + the floor's pointer mask 21/48; d3 one shared u8 select var 21/50; d4 + condition staging 20/49; d5 + per-block vars 20/47; d6 the arm read through the pointer 29/47.
- result: KILLED as a chassis, and a clean negative for the rederive modality: m2c's own reconstruction scores 29, and its best hybrid with the floor's levers scores 20 - two worse than the floor's 18, at 47 insns (two FEWER than the target's 49). The two-reads spelling is not the missing structure. d5 at 47 insns and d1 at 45 confirm again that instruction count is uncorrelated with the honest score on this function.
- verdict: KILLED

## [s9] jump.c's thread_jumps, which runs immediately before cse1, can redirect a second branch onto an existing join label and raise its LABEL_NUSES to 2 without a new insn - the zero-cost boundary s8's enumeration could not reach.
- mechanism: toplev.c:2861 calls `thread_jumps (insns, max_reg_num (), 1)` inside the `if (optimize > 0)` block immediately BEFORE `cse_main (insns, ..., 0, cse_dump_file)`, so any label-use change it makes is visible to cse1's boundary test.
- probe: Read toplev.c:2845-2870 to establish the pass order, then checked thread_jumps' precondition against the target's own instruction stream (asm/funcs/func_80034F88.s).
- result: KILLED on the precondition, no measurement needed. thread_jumps only redirects a jump when the destination code is equivalent to the redirect target's code under the same register state; the three join labels here are each followed by DIFFERENT code (block 1's join by base2's materialisation and block 1's store, block 2's join by block 2's store and base3's materialisation, block 3's join by block 3's store and the copy-loop setup). No pair of equivalent destinations exists, so no redirection is available and no label's NUSES can rise without a new LABEL_REF - i.e. without a new jump insn. This closes the second of the two lines of attack s8's frontier named.
- verdict: KILLED

## Resolved in s10 (synthesis)

### s9-F2 — KILLED. "The 49-instruction `u8 val` chassis may reach 18 or below
### once crossed with the levers found after s3."
*Mechanism.* Score on this function is diff ALIGNMENT, not instruction count or
access counts, so a chassis two instructions shorter than the floor form might
align better once given the s4-s9 levers (two-reads-per-block, staged pointer
bases, per-access pointer/symbol mixes).
*Probe.* Wave U — 13 forms through `tmp/grind/func_80034F88/s10/probe.py`.
*Result.* u0/u1/u9/u10 20/49, u4 23/48, u3/u5/u7/u8 24, u6/u11/u12 25/49,
u2 29/47. Best 20; nothing reached the then-floor of 18. The chassis was never
the axis. **KILLED.** `rejected/u8-chassis-49insn-plateau-score20.c`.

### s10-H1 — CONFIRMED. There is a THIRD reload mechanism, it costs nothing, and
### it is compatible with the target's unconditional store.
*Statement.* A read of a just-stored byte survives as a real `lbu` whenever the
VARIABLE that supplied the stored value has been reassigned before the read.
*Mechanism.* cse records a store as an equivalence class containing both the
stored MEM and the register the value came from, and satisfies a later read from
the cheapest live member — the register (which is what emitted the s3-s9 floor
form's `andi a0,v1,0xff` zero-extend-from-register where the target has an
`lbu`). Overwrite that register first and the MEM is the only member left, so
cse must emit the load. No volatile, no CODE_LABEL, no NOTE_INSN_LOOP_END, no
conditional store, no extra instruction.
*Probe.* Spell the block condition and the selected value as ONE local, so each
block's `c = p[8] & K;` kills the previous block's stored value: waves V/W/Z/ZZ,
53 forms.
*Result.* Reloads appear in every flag block from the second onward with the
build at or below the target's instruction count. Floor 18 -> 13 -> 10 -> 9.
**CONFIRMED** — and it supersedes the s7/s8/s9 claim that every reload-producing
construct costs an instruction the target does not have.

### s10-H2 — CONFIRMED (and it REFUTES s6). The `$v0`/`$v1` swap of the selected
### value IS steerable, by variable reuse.
*Statement.* s6 read `.greg` conflicts and concluded the swap was a `local-alloc`
fact, not a C-reachable choice; six live-range forms all scored worse.
*Mechanism.* The target reuses the CONDITION's register for the result
(`lw v0,32(a1)`; `andi v0,v0,K`; `bnez v0`; `ori v0,a0,K`; `move v0,a0`). A
separate `val2` local is what forces the result into `v1`; one local carrying
both puts it in the condition's register.
*Probe.* Wave V (10 forms) and every later wave.
*Result.* All six `ori`/`move` register-naming points flip together. **CONFIRMED
— s6's conclusion is withdrawn.**

### s10-H3 — KILLED. Three separate pointer LOCALS supply the target's three
### addend-0 bases.
*Mechanism.* One pointer local = one base; the target has three, so three locals
should give three.
*Probe.* w1 (assigned at block top), w2 (staged before the previous block's
store), w9 (block-scoped), z6 (declared last), z7 (`p` declared last), z8.
*Result.* All 23, several at exactly 49 instructions: the shape is right but the
allocator gives every base the same hard register and evicts `p` from `a1` into
`a0`, cascading through the function; and with one register for all bases the
second base cannot be materialised before block 1's store as the target's is.
**KILLED.** `rejected/three-pointer-locals-ra-cascade-score23.c`. What does work
is ONE pointer variable re-assigned (z1 10/49) or two variables one of which is
re-assigned (zz5/zz6 9/49).

### s10-H4 — KILLED as a route to the floor, CONFIRMED as a mechanism. Block 1's
### missing reload via the mask's value routed through the reused local.
*Mechanism.* `c = *ptr & 0xF8; *ptr = c;` makes block 1's `c = p[8] & 1;` kill
the mask's stored value, so block 1's read is a real `lbu` — the target's full
`lbu 5` count, with an unconditional store, in non-volatile C at no instruction
cost. Nothing measured in s1-s9 achieved that combination.
*Probe.* Waves X and A2 (16 forms) crossing the spelling with every base
structure; plus wave B's second-pointer read spellings.
*Result.* The reload appears every time, and the score is **33** every time
(x1/x5/x6 33/45, x3 33/47, x2/x4/x8/zz1/a1/a2/a3/a4/a8 33/49, a7 34/50), because
carrying the mask through `c` extends that local's live range across the call
return and permutes the whole allocation (`p` into a2, condition and value into
v1). Reading block 1 through the other pointer variable also produces the reload
at 50 insns / 27 (b1); storing through it is 13 (b2); both is 23/51 (b3).
**KILLED as a route to the floor.** `rejected/maskc-reload-ra-cascade-score33.c`,
`rejected/block1-read-through-second-pointer-score27.c`.

## Open frontier for s11

### F1 — block 1's reload and the target's register assignment are mutually
### exclusive on this chassis; break the tie.
*Why it matters.* It is the ENTIRE residual 9. Every one of the 9 points is
downstream of the single missing `lbu a0,0(v1)`: without it block 1's byte stays
in the mask's register instead of being loaded into a fresh one, which pins the
mask's base into the wrong hard register (build `a2` where the target has `v1`)
and leaves a `nop` in the `lw` load-delay slot. With it (s10-H4) the allocation
permutes and the score triples.
*Mechanism to attack.* The conflict is a live-range one, not a spelling one: the
only local killed at the right point (between the mask's store and block 1's
read) is the condition/value local `c`, and using it for the mask extends its
range across the call return. So the search is for a way to kill the mask's
value register at that point WITHOUT `c` carrying it across the call — e.g. a
mask spelling whose value is dead by construction, a different variable that is
naturally reassigned between the mask and block 1, or a block-1 read whose
address expression differs from the mask's while re-using an already-materialised
base (b1 does the latter but pays an instruction because the second base has to
move up).
*Next probe.* Take `zz5`/`zz6` (9/49) and vary ONLY the mask statement and
block 1's read, holding the base structure fixed: `val` vs `c` vs a third local
for the mask value; the mask read spelled through the symbol and stored through
the pointer (and the reverse); block 1's read hoisted above / sunk below the
condition. Read every result with `s10/sbs.py`, which prints the difflib-aligned
listing — on this function the score IS the count of misaligned positions, so
the listing tells you exactly which insn to chase.

### F2 — the third base needs a spelling a human would write.
*Why it matters.* The 9 rests on a second `ptr2 = &D_80106A73;` that re-assigns
a variable a value it already holds. It fails checklist T1/T2 as written, it is
NOT vetted and NOT self-approved, and if it is ruled out the function falls back
to the CLEAN 13 (`candidate_clean_13.c`), not to the old 18.
*Next probe.* Not a sweep — a derivation. The target materialises `&D_80106A73`
three times with addend 0; find a program structure in which that re-derivation
is what the source naturally says (a per-block scope with its own pointer is
already measured at 23; a helper-shaped re-fetch would cost a call). If no such
structure exists, the correct outcome is a `ruling-request` naming this
construct specifically — not a submission and not a silent carry-forward.

### F3 — the escalation thesis inherited from s7/s8/s9 must be REWRITTEN AGAIN
### before it is ever filed.
s9's packet claimed that every construct supplying the target's reloads costs at
least one instruction the target does not contain, and that the boundary family
is the only source of the addend-0 bases. Both halves are now false:
register-death supplies the reloads for free, and a re-assigned pointer variable
supplies the bases at their exact cost. The function moved 18 -> 9 in a single
session with no new sanctioned-family claim. It is not near exhaustion, and no
escalation packet should be filed from its current state.

## [s10] The 49-instruction `u8 val` chassis reaches 18 or below once crossed with the levers found after s3.
- mechanism: Score on this function is diff ALIGNMENT rather than instruction count or access-signature counts, so a chassis two instructions shorter than the floor form could align better once given the two-reads-per-block, staged-pointer-base and per-access pointer/symbol mixes discovered in s4-s9. No session since s3 had re-explored it.
- probe: Wave U, 13 forms, `tmp/grind/func_80034F88/s10/probe.py` with an objdump lbu/sb/lui census per form.
- result: u0/u1/u9/u10 20/49, u4 23/48, u3/u5/u7/u8 24, u6/u11/u12 25/49, u2 29/47. Best 20; nothing reached even the then-floor of 18. The chassis was never the axis.
- verdict: KILLED

## [s10] A read of a just-stored byte survives as a real lbu whenever the VARIABLE that supplied the stored value is reassigned before the read — a third reload mechanism, at zero instruction cost, compatible with the target's unconditional store.
- mechanism: cse records a store as an equivalence class containing both the stored MEM and the register the value came from, and satisfies a later read from the cheapest LIVE member — the register (which is what emits the s3-s9 floor form's `andi a0,v1,0xff` zero-extend-from-register where the target has an lbu). Overwriting that register first leaves the MEM as the only member, so cse emits the load. No volatile, no CODE_LABEL, no NOTE_INSN_LOOP_END, no conditional store, no extra instruction.
- probe: Spell each block's condition and selected value as ONE local, so `c = p[8] & K;` kills the previous block's stored value; waves V/W/Z/ZZ, 53 forms, honest sandbox plus difflib side-by-side.
- result: Reloads appear in every flag block from the second onward at or below the target's instruction count; honest floor 18 -> 13 (w3, 45 insns) -> 10 (z1, 49) -> 9 (zz5/zz6, 49), and from block 2 onward the build matches the target instruction-for-instruction and register-for-register. Supersedes the s7/s8/s9 claim that every reload-producing construct costs an instruction the target does not have.
- verdict: CONFIRMED

## [s10] s6's finding that the $v0/$v1 swap of the selected value is not a steerable allocation tie.
- mechanism: The target reuses the CONDITION's register for the result (`lw v0,32(a1)`; `andi v0,v0,K`; `bnez v0`; `ori v0,a0,K`; `move v0,a0`). A separate `val2` local is what forces the result into v1; one local carrying both puts it in the condition's register. s6 read the .greg conflict list and concluded local-alloc had already taken $v0, but it had taken it for a val2 that need not exist.
- probe: Wave V (10 forms) and every subsequent wave.
- result: All six `ori`/`move` register-naming points flip together with the reuse spelling. s6's conclusion is withdrawn.
- verdict: CONFIRMED

## [s10] Three separate pointer LOCALS supply the target's three addend-0 address bases.
- mechanism: One pointer local yields one unfolded lui+addiu base; the target materialises three, so three locals staged where the target materialises them should reproduce it (s2's L3 placement finding).
- probe: w1 (block top), w2 (staged before the previous block's store), w9 (block-scoped), z6 (extra pointers declared last), z7 (`p` declared last), z8; plus the two-pointer partial forms z3/z4/z5 and w7.
- result: All 23, several at exactly 49 instructions. The instruction shape is the target's, but the allocator gives every base the same hard register and evicts `p` from a1 into a0, cascading through the function; with one register for all bases the second base cannot be materialised before block 1's store the way the target does. What works instead is ONE pointer variable re-assigned (10/49) or two variables one of which is re-assigned (9/49).
- verdict: KILLED

## [s10] Block 1's missing reload can be bought by routing the mask's stored value through the reused local.
- mechanism: `c = *ptr & 0xF8; *ptr = c;` makes block 1's `c = p[8] & 1;` kill the mask's value, so block 1's read is a real lbu — the target's full lbu 5 count alongside an unconditional store, in non-volatile C, at no instruction cost. Nothing in s1-s9 achieved that combination.
- probe: Waves X and A2 (16 forms) crossing the mask spelling with every base structure, plus wave B's second-pointer read spellings.
- result: The reload appears every time and the score is 33 every time (x1/x5/x6 33/45, x3 33/47, x2/x4/x8/zz1/a1/a2/a3/a4/a8 33/49, a7 34/50), because carrying the mask through `c` extends that local's live range across the call return and permutes the whole allocation (p into a2, condition and value into v1). Block 1 reading through the other pointer variable also produces the reload at 50 insns / 27; storing through it is 13; both is 23/51. The mechanism is real; as a route to the floor it is dead, and the tie between block 1's reload and the target's register assignment is the entire residual 9.
- verdict: KILLED

## [s10] The 49-instruction `u8 val` chassis (s9 frontier item 2, untouched since s3) reaches 18 or below once crossed with the levers found in s4-s9.
- mechanism: Score on this function is diff ALIGNMENT, not instruction count or access-signature counts, so a chassis two instructions shorter than the floor form could align better once given the two-reads-per-block, staged-pointer-base and per-access pointer/symbol mixes.
- probe: Wave U: 13 forms through tmp/grind/func_80034F88/s10/probe.py (honest sandbox + objdump lbu/sb/lui census per form).
- result: u0/u1/u9/u10 20/49, u4 23/48, u3/u5/u7/u8 24, u6/u11/u12 25/49, u2 29/47. Best 20; nothing reached even the then-floor of 18. The chassis was never the axis.
- verdict: KILLED

## [s10] A read of a just-stored byte survives as a real lbu whenever the VARIABLE that supplied the stored value is reassigned before the read - a third reload mechanism, at zero instruction cost, compatible with the target's unconditional store.
- mechanism: cse records a store as an equivalence class containing both the stored MEM and the register the value came from, and satisfies a later read from the cheapest LIVE member - the register, which is exactly what emitted the s3-s9 floor form's `andi a0,v1,0xff` zero-extend-from-register where the target has an lbu. Overwrite that register first and the MEM is the only member left, so cse must emit the load. No volatile, no CODE_LABEL, no NOTE_INSN_LOOP_END, no conditional store, no extra instruction.
- probe: Spell each block's condition and selected value as ONE local (the frozen SOTN 'variable reuse for codegen control' family), so `c = p[8] & K;` kills the previous block's stored value; waves V/W/Z/ZZ, 53 forms, honest sandbox plus difflib side-by-side (s10/sbs.py).
- result: Reloads appear in every flag block from the second onward at or below the target's instruction count. Floor 18/51 -> 21/52 (symbol-spelled) -> 13/45 (all accesses through one pointer local) -> 10/49 (pointer variable re-assigned per block) -> 9/49 (second pointer variable). From flag block 2 to the end of the function the build is identical to the target instruction-for-instruction AND register-for-register. Supersedes the s7/s8/s9 claim that every reload-producing construct costs an instruction the target does not have.
- verdict: CONFIRMED

## [s10] s6's finding that the $v0/$v1 swap of the selected value is NOT a steerable allocation tie.
- mechanism: The target reuses the CONDITION's register for the result (`lw v0,32(a1)`; `andi v0,v0,K`; `bnez v0`; `ori v0,a0,K`; `move v0,a0`). A separate `val2` local is what forces the result into v1; one local carrying both puts it in the condition's register. s6 read the .greg conflict list and concluded local-alloc had already taken $v0 - but it had taken it for a val2 that need not exist.
- probe: Wave V (10 forms) and every subsequent wave; side-by-side listings sbs_f0.txt vs sbs_w3.txt.
- result: All six `ori`/`move` register-naming points flip together with the reuse spelling. s6's conclusion is withdrawn.
- verdict: CONFIRMED

## [s10] Three separate pointer LOCALS supply the target's three addend-0 address bases.
- mechanism: One pointer local yields one unfolded lui+addiu base shared by that block's load and store; the target materialises three, so three locals staged where the target materialises them should reproduce it (s2's L3 placement finding).
- probe: w1 (block top), w2 (staged before the previous block's store), w9 (block-scoped), z6 (extra pointers declared last), z7 (`p` declared last), z8, plus the partial two-pointer forms z3/z4/z5 and w7.
- result: All 23, several at exactly 49 instructions: the instruction shape is the target's, but the allocator gives every base the same hard register and evicts `p` from a1 into a0, cascading through the function, and with one register for all bases the second base cannot be materialised before block 1's store as the target's is. What works instead is ONE pointer variable re-assigned (10/49) or two variables one of which is re-assigned (9/49).
- verdict: KILLED

## [s10] Block 1's missing lbu reload can be bought by routing the mask's stored value through the reused local.
- mechanism: `c = *ptr & 0xF8; *ptr = c;` makes block 1's `c = p[8] & 1;` kill the mask's stored value, so block 1's read is a real lbu - the target's full lbu 5 count alongside an UNCONDITIONAL store, in non-volatile C, at no instruction cost. Nothing measured in s1-s9 achieved that combination.
- probe: Waves X and A2 (16 forms) crossing the mask spelling with every base structure, plus wave B's second-pointer read spellings.
- result: The reload appears every time and the score is 33 every time (x1/x5/x6 33/45, x3 33/47, x2/x4/x8/zz1/a1/a2/a3/a4/a8 33/49, a7 34/50): carrying the mask through `c` extends that local's live range across the call return and permutes the whole allocation (p into a2, condition and value into v1). Block 1 reading through the other pointer variable also produces the reload at 50 insns / 27; storing through it is 13; both is 23/51. Real mechanism, dead as a route to the floor - and the tie it exposes IS the residual 9.
- verdict: KILLED

## [s11] Each if/else join ends cse's extended basic block, so an ordinary DIRECT SYMBOL reference in blocks 2 and 3 re-materialises the flag address by itself - giving the target's third addend-0 base with no redundant pointer re-assignment.
- mechanism: s10's 9-point form buys the target's three bases by re-assigning `ptr2 = &D_80106A73;` a value it already holds, which fails checklist T1/T2. If cse's table is cleared at each join, a plain `D_80106A73 = ...` in blocks 2/3 must recompute the address anyway, and the recomputation is what the source literally says - no construct without semantic purpose anywhere in the body.
- probe: Wave A, 7 forms crossing the pointer/symbol spelling of the mask and of each block (a1-a7) plus b8, all read with sbs.py rather than the score alone.
- result: The join does clear the table, but what a symbol reference re-materialises is NOT a shared base: it emits `lui at` + `lbu %lo(at)` for the read and a second `lui at` + `sb %lo(at)` for the store, i.e. `%lo` folded into each access, never `lui`+`addiu`+two addend-0 accesses. a1 24/49, a2 24/49, a3 32/50, a4 21/52, a5 32/50, a6 24/47, a7 24/49, b8 24/49. An addend-0 base only ever comes from a pointer LOCAL, so an extra base only ever comes from an extra pointer-local ASSIGNMENT.
- verdict: KILLED

## [s11] The target's `bnez; ori (delay); addu; LABEL: sb 0(base)` tail is a jump2 cross-jump merge of a store DUPLICATED into both arms, and spelling it that way re-derives the flag pointer naturally.
- mechanism: find_cross_jump merges identical block suffixes; a per-arm store would explain both the merged `sb` at the label and a per-arm address use, which is the one structure in which re-deriving the pointer is what the source says (s10 frontier item 2).
- probe: Wave C, 10 forms crossing the duplicated store (anonymous value and `c`-carried value) with the pointer and symbol base spellings, plus per-block scoped pointer locals and a block-3-via-`ptr` variant.
- result: The duplicated store emits 48 instructions against the target's 49 - the stores merge but the value copy does not - and never approaches the floor: c1/c2 18/48, c3/c4 30/54, c5 35/55, c6 34/55, c9 19/46, c10 30/52. Per-block scoped pointers reproduce s10's three-locals result exactly (c8 23/49) and block 3 re-using `ptr` is 14/49. The label-shared spelling s1-s10 used is the correct reading of the tail.
- verdict: KILLED

## [s11] Declaration order steers the allocation on the b2 chassis, where the instruction stream is already the target's and only the hard registers differ.
- mechanism: LUID/allocno ordering is the frozen SOTN "named-intermediate declaration order" lever; b2's residual is a pure register permutation over an otherwise exact stream, which is the shape that lever is supposed to move.
- probe: All 24 permutations of the four working locals with `p` first and `i` last, plus 6 orders moving `p` and `i` themselves.
- result: All 30 score exactly 33 at 49 instructions with an identical lbu/sb/lui census. Declaration order has NO gradient on this function - GCC 2.7.2 orders allocnos by refs/live-length, and the pseudo numbering only breaks exact ties, of which there are none here.
- verdict: KILLED

## [s11] Block 1's byte and blocks 2/3's byte must be different C locals, because the target puts them in different hard registers (a0 and v1) and one pseudo cannot hold two.
- mechanism: The target only ever uses v0/v1/a0/a1 in this function; b2's greg dump shows 6 mutually conflicting pseudos taking v1..a3. a0 is shared by block 1's byte and blocks 2/3's base, v1 by base 1 and blocks 2/3's byte - sharing that is only possible if those are separate, non-overlapping values.
- probe: Wave F (8 forms) splitting the byte local one, two and three ways across the register-death and plain-mask chassis; wave J (8 forms) re-running the split plus a condition-local split on the h1 chassis.
- result: The premise is right and the lever is inert. On the register-death chassis the split costs an instruction (f1/f2/f5/f8 50/34, f3 50/28); on the plain-mask chassis it is free but changes nothing (f4 9/49, identical to baseline); on the h1 chassis every split leaves 13 (j1/j2/j3/j7 13). The two bytes do end up in different registers without being different locals, because local-alloc splits the live range itself.
- verdict: KILLED

## [s11] A POINTER COPY (`ptr2 = ptr;`) leaves block 1's read unsatisfiable by cse, buying the missing `lbu` reload at zero instruction cost and without permuting the allocation.
- mechanism: s10 established that the reload survives only when the value the store came from is dead in registers at the read, and that the only local reassigned at that point is the condition/value local - which is why routing the mask through it costs the whole allocation (33). A second POINTER handle attacks the other half of the equivalence: the read's address rtx is a different register, so the class lookup misses and cse must emit the load, while the copy itself is coalesced away and costs nothing.
- probe: Wave H (8 forms): the copy handle for block 1's read only and for its read+store, the byte read duplicated into the arms, and signed-char byte locals. Then wave I (6 forms) and wave K (5 forms) on the winner.
- result: CONFIRMED and it is the session's find. h1 (copy handle for block 1's read AND store, blocks 2/3 through the re-set `ptr2`) is 13 / 49 insns with the target's lbu count, and is instruction- and register-identical to the target from the prologue through block 1's if/else - base 1 v1, mask a0, the reload in the `lw`'s load-delay slot, condition and result v0, `move a1,v0` in place. h2 (read through the copy, store through the original) 15, i5 (four single-assignment handles) 13, i2/i3/i6 (a dedicated third local for blocks 2/3) 18. The arm-duplicated read (h3 32/56, h4 34/53, h5 9/49) and signed-char bytes (h6/h7 23/58) are dead.
- verdict: CONFIRMED

## [s11] h1's residual 13 can be closed by denying blocks 2/3's base the register base 1 vacates at block 1's store.
- mechanism: The build materialises that base into v1 (free the instant block 1 stores) and puts blocks 2/3's byte in a0; the target does the reverse, and because a0 is free there its scheduler hoists the `lui`/`addiu` above block 1's `sb`. Making the base's live range overlap base 1's - by materialising it before block 1's store, by giving it its own local, or by extending base 1 past it - should deny v1 and force the target's assignment.
- probe: Wave I (three and four pointer handles, declaration orders, block 3 via the mask's pointer) and wave K (the base set between the if/else and the store, before the if/else, and with base 1 extended into block 3).
- result: Never below 13. i1 9 (loses the reload entirely), i2/i3/i6 18, i4 14, i5 13, k1 18, k2 13, k3 15, k4 26, k5 23/50. Denying v1 either costs the reload or moves the whole allocation; the swap survives every structural perturbation measured.
- verdict: KILLED as probed - the tie is real and s12 needs a different attack on it, not another placement (see the frontier).

## [s11] Each if/else join ends cse's extended basic block, so an ordinary DIRECT SYMBOL reference in blocks 2 and 3 re-materialises the flag address by itself, giving the target's third addend-0 base with no redundant pointer re-assignment.
- mechanism: s10's 9-point form buys the target's three bases by re-assigning `ptr2 = &D_80106A73;` a value it already holds, which fails checklist T1/T2. If cse's table is cleared at each join, a plain `D_80106A73 = ...` in blocks 2/3 must recompute the address anyway, and the recomputation is what the source literally says.
- probe: Wave A: 7 forms (a1-a7) crossing the pointer/symbol spelling of the mask and of each block, plus b8, each read with sbs.py rather than the score alone.
- result: The join does clear the table, but a symbol reference re-materialises `lui at` + `%lo`-folded access TWICE (once for the read, once for the store) — never `lui`+`addiu`+two addend-0 accesses. a1 24/49, a2 24/49, a3 32/50, a4 21/52, a5 32/50, a6 24/47, a7 24/49, b8 24/49. Consequence for every future session: an addend-0 base can only come from a pointer LOCAL, so an extra base can only come from an extra pointer-local ASSIGNMENT.
- verdict: KILLED

## [s11] The target's `bnez; ori (delay); addu; LABEL: sb 0(base)` tail is a jump2 cross-jump merge of a store DUPLICATED into both arms, and spelling it that way re-derives the flag pointer naturally.
- mechanism: find_cross_jump merges identical block suffixes; a per-arm store would explain both the merged `sb` at the label and a per-arm address use — the one structure in which re-deriving the pointer is what the source says.
- probe: Wave C: 10 forms crossing the duplicated store (anonymous value and `c`-carried value) with the pointer and symbol base spellings, plus per-block scoped pointer locals and a block-3-via-`ptr` variant.
- result: The duplicated store emits 48 instructions against the target's 49 — the stores merge, the value copy does not — and never approaches the floor: c1/c2 18/48, c3/c4 30/54, c5 35/55, c6 34/55, c9 19/46, c10 30/52. Per-block scoped pointers reproduce s10's three-locals result exactly (c8 23/49). The label-shared spelling s1-s10 used is the correct reading of the tail.
- verdict: KILLED

## [s11] Declaration order steers the allocation on the b2 chassis, where the instruction stream is already the target's and only the hard registers differ.
- mechanism: LUID/allocno ordering is the frozen SOTN 'named-intermediate declaration order' lever, and b2's residual is a pure register permutation over an otherwise exact 49-instruction stream — the shape that lever is supposed to move.
- probe: All 24 permutations of the four working locals with `p` first and `i` last, plus 6 orders moving `p` and `i` themselves.
- result: All 30 score exactly 33 at 49 instructions with an identical lbu/sb/lui census. GCC 2.7.2 orders allocnos by refs/live-length; pseudo numbering only breaks exact ties and there are none here. Declaration order has NO gradient on this function — future sessions should not spend measurements on it.
- verdict: KILLED

## [s11] Block 1's byte and blocks 2/3's byte must be different C locals, because the target puts them in different hard registers (a0 and v1) and one pseudo cannot hold two.
- mechanism: The target only ever uses v0/v1/a0/a1 here; b2's greg dump shows 6 mutually conflicting pseudos taking v1..a3. In the target a0 is shared by block 1's byte and blocks 2/3's base, and v1 by base 1 and blocks 2/3's byte — sharing only possible for separate, non-overlapping values.
- probe: Wave F (8 forms) splitting the byte local one, two and three ways across the register-death and plain-mask chassis; wave J (8 forms) re-running the split plus a condition-local split on the h1 chassis.
- result: The premise is right and the lever is inert. On the register-death chassis the split COSTS an instruction (f1/f2/f5/f8 50/34, f3 50/28); on the plain-mask chassis it is free but changes nothing (f4 9/49, identical to baseline); on the h1 chassis every split leaves 13 (j1/j2/j3/j7). The two bytes reach different registers without being different locals, because local-alloc splits the live range itself.
- verdict: KILLED

## [s11] A POINTER COPY (`ptr2 = ptr;`) leaves block 1's read unsatisfiable by cse, buying the missing `lbu` reload at zero instruction cost and without permuting the allocation.
- mechanism: s10 established the reload survives only when the value the store came from is dead in registers at the read, and that the only local re-set at that point is the condition/value local — which is why routing the mask through it costs the whole allocation (33). A second POINTER handle attacks the other half of the equivalence instead: the read's address rtx is a different register, so cse's class lookup misses and it must emit the load, while the copy itself is coalesced away and costs no instruction and no register.
- probe: Wave H (8 forms): the copy handle for block 1's read only and for its read+store, the byte read duplicated into the arms, signed-char byte locals; then waves I (6) and K (5) on the winner.
- result: CONFIRMED. h1 is 13 / 49 insns with the target's lbu count (5), and is instruction- AND register-identical to the target from the prologue through block 1's if/else: base 1 v1, mask a0, the surviving reload sitting in the `lw`'s load-delay slot with no nop, condition and selected value both v0, `move a1,v0` scheduled where the target has it. h2 (read through the copy, store through the original) 15; i5 (four single-assignment handles) 13; i2/i3/i6 (dedicated third local) 18. Arm-duplicated read (h3 32/56, h4 34/53) and signed-char bytes (h6/h7 23/58) are dead.
- verdict: CONFIRMED

## [s11] h1's residual 13 can be closed by denying blocks 2/3's base the register base 1 vacates at block 1's store.
- mechanism: The build materialises that base into v1 (free the instant block 1 stores) and puts blocks 2/3's byte in a0; the target does the reverse, and because a0 is free there its scheduler hoists the `lui`/`addiu` above block 1's `sb`. Making the base's live range overlap base 1's should deny v1 and force the target's assignment.
- probe: Wave I (three and four pointer handles, declaration orders, block 3 via the mask's pointer) and wave K (the base set between the if/else and the store, before the if/else, and with base 1 extended into block 3).
- result: Never below 13. i1 9 (loses the reload entirely), i2/i3/i6 18, i4 14, i5 13, k1 18, k2 13, k3 15, k4 26, k5 23/50. Denying v1 either costs the reload or moves the whole allocation. The swap survives every structural perturbation measured — it is a genuine allocno-priority tie and needs a different attack, not another placement.
- verdict: KILLED

## Resolved in s12 (structural) — the function is MATCHED

### s12-H1 — CONFIRMED. This is the match.
*Statement.* The residual that eleven sessions read as an allocno-priority tie
in blocks 2/3 is not a tie at all: it is a live-range CONFLICT created by the C
source using ONE local for the loaded flag byte across all three blocks, where
the original used more than one.
*Mechanism.* The target materialises block 2's base into a0 one instruction
after block 1's loaded byte dies in a0, and still ahead of block 1's `sb`
(`ori v0,a0,1 / move v0,a0 / lui a0 / addiu a0 / sb v0,0(v1)`). GCC 2.7.2's
global allocator works per-pseudo over the union of a variable's live ranges,
so a single C byte local is one allocno that is live inside block 2's base
range; the two conflict, global.c evicts the base from a0 onto a2, and the byte
is then pinned to a0 for every block. Splitting the byte local removes the
conflict and the target's whole register cascade follows: block 2/3's byte onto
v1 (freed by block 1's `sb`), block 3's base reusing a0.
*Probe.* Wave F — the byte local split four ways (mask+b1 / b2+b3, per block,
mask alone, and a control with one local) across the three best chassis, 24
forms through tmp/grind/func_80034F88/s12/probe.py; then waves G/H/I/J (31 more
forms) to minimise and naturalise the winning body.
*Result.* **Score 0 at 49/49 instructions**, seven structurally distinct bodies.
The submitted form (i5/h2) is four self-contained blocks, each declaring its own
`u8 *q = &D_80106A73;` plus `s32 v` and `s32 c`. `sandbox --disable all` prints
score 0, rules_dropped 30, cheat_asm_stripped 317. **CONFIRMED.**

### s12-H2 — KILLED. The s11 frontier's pointer-copy cross.
*Statement.* s11's free reload mechanism (`ptr2 = ptr;`) crossed with s10's
9-point base structure would give a form that has both block 1's reload and
blocks 2/3's register assignment.
*Probe.* Wave A — the copy handle crossed with every base structure the ledger
records (zz5 two variables, z1 single re-assigned handle, w3 single base, three
copies one per block, a dedicated third local as the copy handle), 13 forms.
*Result.* The copy strictly costs: 13 (a1), 12 (a5/a6), 12 (a7/a8), 18 (a3/a4),
27 (a9); the 9-point chassis without it stays 9 (a2). The two failure regions
were entangled through the shared byte local, not independent. **KILLED** — and
mooted by s12-H1, which closes both regions at once without any copy.

### s12-H3 — KILLED. Duplicated-read-into-arms as a ref-count lever.
*Statement.* Spelling the select as a read duplicated into both arms
(`if (c) { c = *q | K; } else { c = *q; }`) lifts the base pseudo's reg_n_refs
and reorders global.c's allocno priority, which is the frontier's named attack
on the blocks-2/3 assignment.
*Probe.* Wave B — duplicated read in blocks 2/3 only, in all three blocks, on
two mask spellings.
*Result.* 20, 22 and 20 at 53 insns with lbu 178 — the duplicate reads
materialise as real extra loads the target does not have. **KILLED.**

### s12-H4 — CONFIRMED (negative). The base-register ALTERNATION is not
### reachable by moving the base assignments alone.
*Statement.* The target alternates the base register (v1 for the mask+block 1,
a0 for blocks 2/3) because base 2 is materialised while base 1 is still live, so
the lever is to move each base assignment earlier in the source.
*Probe.* Waves C/D/E — the copy handle on the mask (freeing the second handle
for early materialisation), each base assigned before the previous block's
store / before its condition / after its store, with two, three and four locals.
11 forms.
*Result.* Best 12 (d3), and the side-by-side showed every INSTRUCTION already in
the target's position with all 12 points being register naming — which is what
pointed at the byte local and produced s12-H1. Placement is real but secondary:
it fixes the instruction ORDER, and the byte split fixes the register
ASSIGNMENT. **CONFIRMED as a partial lever, exhausted on its own.**

## [s12] The blocks-2/3 base/byte register swap that eleven sessions recorded as an unsteerable allocno-priority tie is a live-range CONFLICT caused by the C source using one local for the loaded flag byte across all three flag blocks.
- mechanism: The target materialises block 2's base into a0 one instruction after block 1's loaded byte dies in a0 and still ahead of block 1's sb (ori v0,a0,1 / move v0,a0 / lui a0 / addiu a0 / sb v0,0(v1)). GCC 2.7.2's global.c allocates per-pseudo over the union of a C variable's live ranges, so one byte local is one allocno that is live inside block 2's base range; they conflict, the base is evicted from a0 onto a2, and the byte is pinned to a0 in every block. Two byte locals remove the conflict and the target's entire register cascade follows.
- probe: Wave F -- the byte local split four ways (mask+block1 / blocks2+3, per block, mask alone, and a one-local control) across the three best chassis, 24 forms through tmp/grind/func_80034F88/s12/probe.py; then waves G/H/I/J (31 further forms) minimising and naturalising the winning body, all scored with sandbox --disable all.
- result: Score 0 at 49/49 instructions, reached by seven structurally distinct bodies. The submitted form is four self-contained blocks each declaring its own 'u8 *q = &D_80106A73;', 's32 v' and 's32 c'; sandbox prints score 0, rules_dropped 30, cheat_asm_stripped 317. One byte local for everything is 13 and the wrong split (mask alone vs the three blocks) is also 13, so the split point is load-bearing.
- verdict: CONFIRMED

## [s12] s11's frontier item 2 -- the free pointer-COPY reload handle crossed with s10's 9-point base structure yields a form with both block 1's reload and blocks 2/3's register assignment.
- mechanism: s11 had only crossed the copy handle with base structures that give block 1 the mask's base; the two failures sat in different regions of the function, so the cross was assumed to be additive.
- probe: Wave A, 13 forms -- the copy handle crossed with zz5's two variables, z1's single re-assigned handle, w3's single base, three copies one per block, and a dedicated third local as the copy handle, on both mask spellings.
- result: The copy strictly costs on a single-byte-local chassis: 13, 12, 12, 18 and 27 against the 9 the same chassis scores without it. The two regions were entangled through the shared byte local rather than independent. Mooted by s12-H1, which closes both regions at once with no copy handle at all.
- verdict: KILLED

## [s12] Duplicating the flag-byte read into both arms of the select lifts the base pseudo's reg_n_refs and reorders global.c's allocno priority in the target's direction.
- mechanism: The frontier's named attack on the blocks-2/3 assignment was to change the REF COUNTS rather than the placement; a read duplicated into two arms is the SOTN-sanctioned spelling for a byte-neutral reg_n_refs lift.
- probe: Wave B -- the read duplicated in blocks 2/3 only, in all three blocks, on two mask spellings, sandbox-scored with an objdump lbu/sb/lui census.
- result: 20, 22 and 20 at 53 build insns with lbu 178 against the target's 176 -- on this function the duplicated reads are not byte-neutral, they materialise as real extra loads. The ref-count axis is dead here.
- verdict: KILLED

## Resolved in s12b (permuter)

## [s12b] The s12 byte-local-split finding is the substantive lever, so it should close the function on a chassis with FEWER than four repeated `u8 *q = &D_80106A73;` handles — i.e. the four repeated declarations the layer-1 reviewer FAILed may be incidental rather than load-bearing.
- mechanism: If the win came from splitting the loaded-byte local (block 1's byte dying before block 2's base is materialised), then the number of pointer handles should be free to drop to the one a human would naturally write, with the split carrying the score.
- probe: Wave K, 8 forms through tmp/grind/func_80034F88/s12/probe.py, byte-local split held fixed in every one and only the handle structure varied: one handle (k1/k5), two (k2), three (k3), two with a copy (k6), one with the mask on the plain symbol (k4), one with blocks 2/3 on the plain symbol (k7), none at all (k8).
- result: KILLED decisively. 23/27/23/23/28/28/29 against 0 for four handles, with the reload census moving monotonically with the handle count (lbu 173 at one handle, 174-175 at two/three, 176 at four). The split is worth 12-13 points ONLY on a chassis that already has four address materialisations; on its own it is worth nothing. The four materialisations are load-bearing and the honest floor without them is 23 — worse than the pre-s10 floor of 18.
- verdict: KILLED

## [s12b] A `static inline` helper produces the target's four separate materialisations of &D_80106A73 from ONE textual declaration of the handle, dissolving the layer-1 objection (four repeated declarations) without changing what the program does.
- mechanism: GCC 2.7.2 at -O2 inlines `static inline` functions (precedent in this tree: src/main.c:2396 `_memcpy`). Each inline instantiation gets its own copy of the body's locals, so a single `u8 *q = &D_80106A73;` inside the helper becomes three independent pointer pseudos at three call sites — the four-materialisation shape, arrived at by ordinary program factoring rather than by repeating a declaration.
- probe: Wave M, 9 forms. m1 (inline setter x3 + a caller-scope block for the mask handle), m3 (caller function-scope mask handle), m4 (helper takes p and computes its own condition), m5 (u8 byte local), m7 (helper takes a u8* parameter, &D_80106A73 passed at each call site), m2 (a second inline helper for the mask so the function body has ZERO handle declarations), m6 (control: static but NOT inline). Sandbox --disable all plus an objdump lbu/sb/lui census; side-by-side against target via s12/sbs.py, saved to s12/sbs_m1.txt.
- result: CONFIRMED as a mechanism, INSUFFICIENT as a match. The inlined chassis reaches 13 at 50 insns with lbu 176 — all four reloads, the matched form's exact access signature, from one declaration (m1 = m3 = m4 = 13; u8 byte local 16; the two-helper form 26; the pointer-parameter form 22; the non-inline control 35 at 37 insns with a real jal, proving the inlining is what creates the pseudos). It does not reach 0 because the inliner substitutes the constant address at each use inside the copied body (integrate.c copy_rtx_and_substitute and its const-equivalence map), leaving every mem with a single-use address that combine then folds %lo into: the build emits "lui a0; lbu a0,0(a0)" and "lui at; sb v0,0(at)" where the target has a shared unfolded lui/addiu base reused by both the reload and the store. Passing the address as an argument does not escape the substitution (m7 = 22).
- verdict: CONFIRMED

## [s12b] With the honest chassis now at target's instruction count minus one and the residual reduced to base folding, a decomp-permuter campaign seeded from the inline-helper chassis can close it.
- mechanism: The remaining difference is address-expression folding plus one instruction, which is the class of local rewrite the permuter's randomizer explores well; s4/s5 killed the permuter on chassis that were further away, so the seed quality was the stated reason it failed.
- probe: tools/permuter_campaign.py launch --func func_80034F88 --dir tmp/grind/func_80034F88/s12/ws --label inline-helper-chassis -j 8 --stop-on-zero; waited in-turn across three windows; sandbox-scored the 16 lowest-permuter-score finds through probe.py; harvest --stop in-session.
- result: KILLED, and this is the THIRD independent kill of the permuter axis on this function. 6,450 iterations took the permuter's own metric from 1885 to 450 (a 4x improvement) while every sandbox-scored find came back 32-43 against the seed's 13. All finds carry sb 160-161 against the correct 164 — the mutations delete flag stores, i.e. the permuter is buying its metric with semantically different code, and it also fails to preserve the `static inline` chassis at all. The permuter's metric is uncorrelated with the honest sandbox on this function regardless of seed quality; seed quality was not the problem.
- verdict: KILLED

## [s12b] A function-like macro is a legitimate way to author the four handles once.
- mechanism: A `#define BB2_SET_FLAG(cond, bit) { u8 *q = &D_80106A73; ... }` invoked three times is textually one declaration site and expands to exactly the body that byte-matches.
- probe: m9, sandbox --disable all.
- result: It scores 0 at 49 insns with lbu 176 — it does byte-match. But it is REJECTED BY CLASSIFICATION, not by measurement: after preprocessing the diff contains the identical four `u8 *q = &D_80106A73;` declarations the driver has banned, so under cheat-checklist T5 ("it is different because it is spelled with X instead of Y is exactly the loophole the policy forbids") it is the banned construct respelled rather than a new attack. Banked as evidence at rejected/macro-respelling-of-banned-four-handle-score0-DO-NOT-SUBMIT.c and deliberately NOT submitted.
- verdict: KILLED

## Open frontier for s13

### F1 — close the inline-helper chassis's 13 by defeating the inliner's constant substitution.
This is the only live axis that reaches the target's access signature from a
construct nobody has objected to. The residual is entirely `%lo` folding: the
inlined copies need their pointer pseudo to survive to cse as a shared base
with two mem uses, instead of being const-substituted into each mem at inline
time. Untried levers, in order of cheapness: (a) read the `.rtl`/`.combine`
dumps of m1 to confirm whether the substitution happens in integrate.c or in
cse/combine — the cc1 -da harness is already wired at s11/dump.sh and s6's
dump scripts; (b) give the helper's handle a third natural mem use, or have the
helper read AND write through `q[0]` in a shape that keeps two live uses at
combine time; (c) nest a second inline accessor inside the helper so the
address crosses two inline boundaries; (d) helper parameter shapes beyond m7's
`u8 *` (e.g. `u8 **`). Note m2 (the fully-factored form with ZERO handle
declarations in the function body) is 26, so the mask handle must stay in the
caller.

### F2 — the classification contradiction, which is now the real blocker and is
### almost certainly the correct outcome for s13 if F1 does not close.
The target bytes are reachable (0 measured twice, by the four-handle body and
by the macro that expands to it). Wave K prices every smaller handle count at
23-29 and the inline route at 13, so the four materialisations are forced. The
layer-1 reviewer's own prescribed remedy was "add the required /* FAKE: ... */
annotation to each of the four `u8 *q = &D_80106A73;` declarations AND document
lever-exhaustion", but the driver's BANNED-CONSTRUCTS list for this function
bans that construct "claimed under the 'C-level pointer alias to a global' /
pointer-alias-fake-exception family" AND "treated as ordinary program logic" —
i.e. it bans the reviewer's own remedy along with its alternative, leaving no
declared path for the only construct that produces the bytes. That is a precise
`ruling-request`: does the FAKE-annotated pointer-alias form remain available
for this function (the reviewer's remedy), is the macro spelling acceptable, or
is neither and the function is OWNER-ACCEPTED INCOMPLETE? Do NOT file it as an
escalation from a non-escalation modality; file it as a ruling-request with the
wave-K price table and the inline-chassis 13 attached as the exhaustion
evidence.

### F3 — the permuter is now dead three times over; do not spend a fourth
### session on it whatever the seed quality.
s4 (50,425 iterations), s5 (1,152 permuter iterations + 1,008 enumerated forms)
and s12b (6,450 iterations from a seed one instruction from target) all
measured the same thing: the permuter's metric moves freely while the honest
sandbox score does not follow, and on this function the randomizer additionally
produces semantically different code (dropped stores) and cannot preserve a
`static inline` chassis.

## [s12] The s12 byte-local-split finding is the substantive lever, so it should close the function on a chassis with FEWER than four repeated `u8 *q = &D_80106A73;` handles - i.e. the four repeated declarations the layer-1 cheat-reviewer FAILed may be incidental rather than load-bearing.
- mechanism: If the win came from splitting the loaded-byte local (block 1's byte dying before block 2's base is materialised), the pointer handle count should be free to drop to the one a human would naturally write, with the split carrying the score.
- probe: Wave K: 8 forms through tmp/grind/func_80034F88/s12/probe.py with the byte-local split held fixed in every one and only the handle structure varied - one handle (k1/k5), two (k2), three (k3), two with a pointer copy (k6), one with the mask through the plain symbol (k4), one with blocks 2/3 through the plain symbol (k7), and none at all (k8). Each scored with `sandbox func_80034F88 --disable all` plus an objdump lbu/sb/lui census.
- result: KILLED decisively: 23 / 27 / 23 / 23 / 28 / 28 / 29 against 0 for four handles, and the reload census moves monotonically with the handle count (lbu 173 at one handle, 174-175 at two/three, 176 at four). The byte-local split is worth 12-13 points ONLY on a chassis that already has four address materialisations; on its own it is worth nothing, and the best honest floor without the four materialisations is 23 - worse than the pre-s10 floor of 18. The four materialisations are therefore forced, not incidental.
- verdict: KILLED

## [s12] A `static inline` helper produces the target's four separate materialisations of &D_80106A73 from ONE textual declaration of the handle, dissolving the layer-1 objection (four repeated declarations) without changing what the program does.
- mechanism: GCC 2.7.2 at -O2 inlines `static inline` functions (precedent in this tree: src/main.c:2396 `_memcpy`). Each inline instantiation gets its own copy of the body's locals, so a single `u8 *q = &D_80106A73;` inside a helper called three times becomes three independent pointer pseudos - the four-materialisation shape (with the caller's mask handle), arrived at by ordinary program factoring rather than by repeating a declaration.
- probe: Wave M: 9 forms - m1 (inline setter x3 + a caller-scope block for the mask handle), m3 (mask handle at caller function scope), m4 (helper takes p and computes its own condition), m5 (u8 byte local in the helper), m7 (helper takes a u8* parameter with &D_80106A73 passed at each call site), m2 (a second inline helper for the mask, so the function body declares no handle at all), m6 (control: static but NOT inline). Sandbox --disable all plus objdump census; side-by-side against target via s12/sbs.py into s12/sbs_m1.txt.
- result: CONFIRMED as a mechanism, INSUFFICIENT as a match. The inlined chassis scores 13 at 50 insns with lbu 176 - all four reloads, the matched form's exact access signature, from one declaration (m1 = m3 = m4 = 13; u8 byte local 16; two-helper form 26; pointer-parameter form 22). The control m6 without `inline` emits a real jal and scores 35 at 37 insns, proving the inlining is what creates the pseudos. It stops at 13 because the inliner substitutes the constant address at each use inside the copied body (integrate.c copy_rtx_and_substitute + its const-equivalence map), leaving every mem with a single-use address that combine folds %lo into: the build emits `lui a0; lbu a0,0(a0)` and `lui at; sb v0,0(at)` where target has one shared unfolded lui/addiu base serving both the reload and the store.
- verdict: CONFIRMED

## [s12] With the honest chassis now one instruction from the target and the residual reduced to address folding, a decomp-permuter campaign seeded from the inline-helper chassis can close it - s4/s5's kills were a seed-quality problem.
- mechanism: The remaining difference is address-expression folding plus one instruction, which is the class of local rewrite the permuter's randomizer explores well.
- probe: tools/permuter_campaign.py launch --func func_80034F88 --dir tmp/grind/func_80034F88/s12/ws --label inline-helper-chassis -j 8 --stop-on-zero; waited in-turn across three `wait` windows; extracted and sandbox-scored the 16 lowest-permuter-score finds through probe.py; harvest --stop in-session with a reason string.
- result: KILLED - the third independent kill of the permuter axis on this function. 6,450 iterations drove the permuter's own metric from 1885 to 450 (a 4x improvement) while every sandbox-scored find came back 32-43 against the seed's 13. Every find also carries sb 160-161 against the correct 164, i.e. the mutations delete flag stores and buy the metric with semantically different code, and the randomizer does not preserve the `static inline` chassis at all. Seed quality was not the problem: the permuter's metric is uncorrelated with the honest sandbox on this function regardless of how close the seed is.
- verdict: KILLED

## [s12] A function-like macro is a legitimate way to author the four handles once, and therefore a way around the layer-1 objection to the four repeated declarations.
- mechanism: A `#define BB2_SET_FLAG(cond, bit) { u8 *q = &D_80106A73; ... }` invoked three times is textually one declaration site and expands to exactly the body that byte-matches.
- probe: Variant m9, sandbox --disable all.
- result: KILLED BY CLASSIFICATION, not by measurement - it does byte-match (score 0 at 49 insns, lbu 176). After preprocessing the diff contains the identical four `u8 *q = &D_80106A73;` declarations the driver has banned, so under cheat-checklist T5 it is the banned construct respelled rather than a new attack. Banked as evidence at rejected/macro-respelling-of-banned-four-handle-score0-DO-NOT-SUBMIT.c and deliberately NOT submitted; src/ was left untouched.
- verdict: KILLED
## Resolved in s13 (permuter)

## [s13] The inline-helper chassis stalls at 13 because integrate.c const-substitutes the helper's pointer into the READ, and the exact stage is identifiable from a cc1 -da dump.
- mechanism: GCC 2.7.2's integrate.c copies an inlined body with `copy_rtx_and_substitute` plus a const-equivalence map (`map->const_equiv_map`, aged by `map->const_age`). The helper's `u8 *q = &D_80106A73;` is a single set of a pseudo to a `symbol_ref`, i.e. a constant equivalence, so every use of q the copier reaches is rewritten to the bare `(symbol_ref)` — until a CODE_LABEL, where integrate bumps `const_age` and drops every equivalence.
- probe: `bash tmp/grind/func_80034F88/s11/dump.sh s13m1` on the m1 chassis installed in src/, then `tmp/grind/func_80034F88/s13/rtlslice.py` to slice func_80034F88 out of the .rtl/.jump/.cse/.loop/.cse2/.combine dumps and count `symbol_ref "D_80106A73"`, `(mem ... symbol_ref)` and `(set (mem:QI ...))` forms per pass.
- result: CONFIRMED, and sharper than the s12b guess. Already at `.rtl` (before any optimisation pass runs) the three INLINED reads are `(set (reg:QI N) (mem:QI (symbol_ref "D_80106A73")))` while the caller's own non-inlined mask block reads `(mem:QI (reg/v:SI 74))`. All FOUR stores — including the three inlined ones — are still `(set (mem:QI (reg/v:SI N)) ...)` at .rtl AND at .combine, because each store sits after the if/else merge label where `const_age` has already been bumped. The counts are stable across rtl/jump/cse/loop/cse2/combine (symref 7 -> 10, mem_with_sym 3 throughout), so cse and combine are NOT the folding agents: the substitution is done at inline time and nothing later undoes it.
- verdict: CONFIRMED

## [s13] The remaining 13 is then a SECOND-stage effect: with the read substituted, each inlined pointer pseudo has one use left, so local-alloc replaces the store's address with the constant too.
- mechanism: GCC 2.7.2 local-alloc.c `update_equiv_regs` records `reg_equiv_constant` for a pseudo that is set exactly once from a constant, and substitutes the constant into its remaining reference(s) (a `symbol_ref` is a legal MIPS memory address, so the replacement validates). The four-handle form escapes this because each of its pseudos carries THREE references (set + read + store) and stays allocated to a hard register; the inlined copies carry two (set + store) after integrate has eaten the read.
- probe: The final-asm side-by-side (tmp/grind/func_80034F88/s12/sbs_m1.txt) read against the .rtl/.combine dumps above.
- result: CONFIRMED. The .combine RTL still has `(set (mem:QI (reg/v:SI 82)) ...)` for every inlined store, yet the emitted asm is `lui at,0x0 / sb v0,0(at)` — the assembler-macro spelling of a store to an absolute symbol. Nothing between combine and final changes the store except local-alloc's equivalence substitution, so the store folding is a downstream CONSEQUENCE of the read folding, not an independent problem. This is why the residual is exactly the target's three shared unfolded `lui`/`addiu` bases.
- verdict: CONFIRMED

## [s13] F1 — the inline-helper chassis closes if the helper's pointer pseudo reaches local-alloc with TWO uses, which any of several natural C shapes should provide.
- mechanism: From the two findings above, the whole 13 reduces to one requirement — keep a second reference on each inlined copy's pointer pseudo. Natural shapes that do it: (a) put the read behind a CODE_LABEL so integrate's `const_age` has already been bumped when the copier reaches it (read duplicated into the two arms of the select, either arm order); (b) do the select on the MASK instead of on the value, so the single read-modify-write sits entirely after the merge label; (c) set the handle inside both arms, so it is not a single-set pseudo at all; (d) route the read through a pointer copy or through a nested inline accessor so the address expression is not the helper's constant-equivalent local.
- probe: Wave N, 8 forms through `tmp/grind/func_80034F88/s13/gen_n.py` + `s13/probe.py --all`, each scored with `sandbox func_80034F88 --disable all` plus the objdump lbu/sb/lui census: n1 (read duplicated into arms), n2 (same, arms swapped), n3 (select on the mask, single RMW after the label), n4 (read through a pointer copy), n5 (handle assigned inside both arms), n6 (nested inline read accessor taking `u8 *`), n7 (mask and flag block 1 sharing one caller-scope handle, blocks 2/3 inlined — the target's own base structure, per sbs_m1.txt where target block 1 reloads through the mask's v1), n8 (n1 crossed with n7).
- result: KILLED. Not one form beats 13: n1 35 (60 insns), n2 38 (66), n3 34 (38), n4 13 (50), n5 42 (68), n6 29 (56), n7 24 (49), n8 36 (55). The mechanism prediction is CONFIRMED — the label route really does keep the pseudo — but it is not free: every form that duplicates the read materialises real extra loads (lbu census 177-179 against the target's 176, and 55-68 build insns against 49), which costs far more than the 1-2 instructions the unfolded base saves. n4 shows the const equivalence propagates straight through a pseudo-to-pseudo copy (`u8 *r = q;` ties m1 exactly at 13/50/176), and n6 shows it survives a second inline boundary (29). n3, the one form with FEWER instructions than target (38), loses the branch structure entirely. n7 is the interesting negative: it has the target's exact instruction count (49) and the target's shared mask/block-1 base, and still scores 24 because three materialisations cannot produce the fourth reload (lbu 175).
- verdict: KILLED

## [s13] The permuter axis, tried a FOURTH time from a structurally different chassis, fails the same way.
- mechanism: Fresh-seed discipline says a basin yields early or not at all and a new seed must be a structurally different chassis, so s13 seeded from n7 (three address materialisations, mask and flag block 1 sharing one caller-scope handle, 49 build insns = the target's exact count) rather than from s12b's m1.
- probe: tools/permuter_campaign.py launch --func func_80034F88 --dir tmp/grind/func_80034F88/s13/ws --label n7-shared-mask-block1-chassis -j 8 --stop-on-zero; one in-turn `wait` window; the six lowest-permuter-score finds extracted and sandbox-scored through s13/probe.py; harvest --stop in-session (tmp/grind/func_80034F88/s13/harvest.json, 4,544 iterations, 73 finds).
- result: KILLED, identically to s4/s5/s12b. The permuter's own metric fell 1670 -> 675 while every sandbox-scored find came back 26-34 against the seed's 24, and every one carries sb 162 against the correct 164 - the randomizer again buys its metric by deleting two of the flag stores. Four kills now span every seed quality from 'far away' (s4/s5) through 'one instruction from target' (s12b) to 'exact instruction count' (s13), so seed structure is definitively not the variable. The permuter's weighted metric is uncorrelated with engine/score.py's distance on this function.
- verdict: KILLED

## Open frontier for s14

### F1 is now CLOSED (killed in s13). The inline-helper route's 13 is a
### two-stage codegen fact, not a spelling problem, and every natural C shape
### that defeats stage 1 costs more instructions than it saves.
The mechanism is fully characterised (integrate.c const-substitution of the
read + local-alloc reg_equiv_constant substitution of the store), and wave N
measured all four families of shape that give the pointer pseudo a second use:
duplicated reads (35/38/36), select-on-the-mask (34), pointer copy (13, the
equivalence propagates), handle set in both arms (42), nested inline accessor
(29), and the three-materialisation caller-shared chassis (24 at the target's
exact 49 insns). Nothing beats 13.

### F2 (unchanged, and now the ONLY live item) - the classification
### contradiction is the real blocker.
The target bytes are reachable and have been measured at 0 twice (the
four-handle body and the macro that expands to it). Wave K priced every smaller
handle count at 23-29; s13's wave N priced every non-repeating route to four
materialisations at 13 or worse. The layer-1 reviewer's own prescribed remedy
(FAKE-annotate the four `u8 *q = &D_80106A73;` declarations under
pointer-alias-fake-exception) and its only alternative (treat them as ordinary
program logic) are BOTH on the driver's banned list for this function, so no
declared path remains for the only construct that produces the bytes. The
correct next outcome is a ruling-request carrying the wave-K price table, the
wave-N kill table, and the integrate.c/local-alloc mechanism as the exhaustion
evidence - not another sweep.

### F3 - the permuter is dead FOUR times over. Do not seed it again on this
### function, whatever the chassis.

## [s13] The inline-helper chassis stalls at 13 because integrate.c const-substitutes the helper's pointer into the flag-byte READ at inline time, and the exact stage is identifiable from a cc1 -da dump.
- mechanism: GCC 2.7.2's integrate.c copies an inlined body with copy_rtx_and_substitute plus a const-equivalence map aged by map->const_age. The helper's `u8 *q = &D_80106A73;` is a single set of a pseudo to a symbol_ref, i.e. a constant equivalence, so every use the copier reaches is rewritten to the bare symbol_ref until a CODE_LABEL bumps const_age and drops the equivalences.
- probe: bash tmp/grind/func_80034F88/s11/dump.sh s13m1 on the m1 chassis installed in src/, then tmp/grind/func_80034F88/s13/rtlslice.py to slice func_80034F88 out of the .rtl/.jump/.cse/.loop/.cse2/.combine dumps and count symbol_ref / (mem ... symbol_ref) / (set (mem:QI ...)) forms per pass.
- result: Already at .rtl the three inlined reads are (mem:QI (symbol_ref "D_80106A73")) while the caller's own non-inlined mask block reads (mem:QI (reg/v:SI 74)). All four stores, including the three inlined ones, are still (mem:QI (reg/v:SI N)) at .rtl AND at .combine, because each store sits after the if/else merge label. Counts are flat across rtl/jump/cse/loop/cse2/combine (symref 7->10, mem_with_sym 3 throughout), so cse and combine are not the folding agents.
- verdict: CONFIRMED

## [s13] The store folding is a SECOND-stage consequence: with the read substituted, each inlined pointer pseudo has one reference left, so local-alloc replaces the store's address with the constant too.
- mechanism: GCC 2.7.2 local-alloc.c update_equiv_regs records reg_equiv_constant for a pseudo set exactly once from a constant and substitutes it into the remaining reference(s); a symbol_ref is a legal MIPS memory address so the replacement validates. The four-handle form escapes this because each of its pseudos carries three references (set + read + store) and stays in a hard register.
- probe: The final-asm side-by-side (tmp/grind/func_80034F88/s12/sbs_m1.txt) read against the .rtl/.combine dumps above.
- result: .combine RTL still stores through (reg/v:SI 82) etc., yet the emitted asm is `lui at,0x0 / sb v0,0(at)` — the absolute-symbol store. Nothing between combine and final touches the store except local-alloc's equivalence substitution, so the store folding is downstream of the read folding rather than an independent problem.
- verdict: CONFIRMED

## [s13] F1 — the inline-helper chassis closes if the helper's pointer pseudo reaches local-alloc with TWO uses, which several natural C shapes should provide.
- mechanism: From the two findings above the whole 13 reduces to keeping a second reference on each inlined copy's pointer pseudo: put the read behind a CODE_LABEL so const_age has already been bumped (read duplicated into the arms), do the select on the mask so the single read-modify-write sits after the merge label, set the handle inside both arms so it is not a single-set pseudo, or route the read through a pointer copy or a nested inline accessor.
- probe: Wave N, 8 forms via tmp/grind/func_80034F88/s13/gen_n.py + s13/probe.py, each scored with `sandbox func_80034F88 --disable all` plus an objdump lbu/sb/lui census (results_wave_n.json).
- result: KILLED — nothing beats 13. n1 duplicated read 35 (60 insns), n2 arms swapped 38 (66), n3 select-on-the-mask 34 (38, branch structure lost), n4 pointer copy 13 (50/lbu 176 — the const equivalence propagates straight through a pseudo-to-pseudo set, exactly tying m1), n5 handle set in both arms 42 (68), n6 nested inline accessor 29 (56), n7 three materialisations with mask+block 1 sharing a caller handle 24 at the target's exact 49 insns and lbu 175, n8 n7 crossed with the duplicated read 36 (55). The label route does keep the pseudo as predicted, but it materialises real extra loads (lbu 177-179 vs the target's 176) that cost far more than the unfolded base saves.
- verdict: KILLED

## [s13] The permuter axis, tried a FOURTH time from a structurally different chassis with the target's exact instruction count, can close the residual.
- mechanism: Fresh-seed discipline says a basin yields early or not at all and a new seed must be a structurally different chassis, so s13 seeded from n7 (three address materialisations, mask and flag block 1 sharing one caller-scope handle, 49 build insns = the target's count) instead of s12b's m1.
- probe: tools/permuter_campaign.py launch --func func_80034F88 --dir tmp/grind/func_80034F88/s13/ws --label n7-shared-mask-block1-chassis -j 8 --stop-on-zero; one in-turn `wait` window; the six lowest-permuter-score finds extracted and sandbox-scored; harvest --stop in-session (harvest.json: 4,544 iterations, 73 finds, stopped=true).
- result: KILLED, identically to s4/s5/s12b. The permuter's own metric fell 1670 -> 675 while the six best finds sandbox-scored 26/27/33/33/33/34 against the seed's 24, and every one carries sb 162 against the correct 164 — the randomizer buys its metric by deleting two flag stores. Four kills now span every seed quality from 'far away' (s4/s5) through 'one instruction from target' (s12b) to 'exact instruction count' (s13).
- verdict: KILLED

## [s14] The target's four `&D_80106A73` address materialisations require four separately DECLARED pointer locals (the banned construct); no other C spelling can produce them.
- mechanism: Wave K (s12b) and wave N (s13) priced every handle-count and every inline-helper shape and concluded the four repeated `u8 *q = &D_80106A73;` declarations, or a macro expanding to exactly them, were the only route.
- probe: cc1 -da dumps of the banned four-declaration form (`w4`) and of the one-handle form (`w1`) with the INSTRUMENTED tools/gcc-2.7.2/cc1, sliced with tmp/grind/func_80034F88/s14/{slice,flat}.py; then a mechanism-derived variant sweep (`r1`-`r24`) scored with the honest sandbox.
- result: REFUTED. What cse keys on is not the number of declared C objects but whether the `symbol_ref` is present in its value table when the address is SET. The two-armed if/else join labels end the cse basic block and flush the table, so a plain RE-ASSIGNMENT of ONE declared `u8 *q` after the flush survives as a real lui/addiu materialisation exactly as a fresh declaration does. A single `u8 *q` re-assigned before the bit-2 and bit-4 blocks scores 10 at 49/49 insns against 13 for the previous best non-banned form.
- verdict: KILLED

## [s14] The three if/else join labels are NOT cse basic-block boundaries on this function (s2's standing conclusion, carried unchallenged since session 2).
- mechanism: s2 cited cse.c:8102-8184, which extends a cse basic block through a conditional branch that skips a block whenever LABEL_NUSES (JUMP_LABEL (p)) == 1, and argued this holds for all three join labels.
- probe: Read the `.cse` dumps of `w4`, `w1` and `r1` directly instead of reasoning about the source: in `w4` the bit-2 and bit-4 blocks' `(set (reg) (symbol_ref "D_80106A73"))` insns SURVIVE cse while the bit-1 block's is rewritten to a register copy, and in `r1` three re-assignments of the same pseudo survive while the one placed before a flush is deleted.
- result: The claim is TRUE for s2's own chassis (the one-armed `val2 = val|K; if (!c) val2 = val;` form, where the branch really does skip a block and fall through) and FALSE for the two-armed `if/else` chassis that every form since s10 has used. On the current chassis the table IS flushed at each join, and that flush is the entire mechanism behind the reloads and the per-block address materialisations.
- verdict: KILLED (as a general claim; retained only for the one-armed chassis)

## [s14] More address handles monotonically improve the score, so a 2- or 3-handle form should sit between the one-handle 29 and the four-handle 0.
- mechanism: Wave K's price table (one 23, two 27, three 23, four 0) was read as noisy but broadly monotone; on the re-assign chassis the same ordering was expected.
- probe: `r19` (two pointer locals: q for the mask + bit-1 block, q2 re-assigned before the bit-2 and bit-4 blocks) and `r20` (three pointer locals) against `r8` (one local re-assigned) — honest sandbox.
- result: 21 and 23 respectively, both far WORSE than the single re-assigned handle's 10 and worse than several one-handle forms. The axis is not monotone in handle count; the one-pseudo-re-set basin at 10 is structurally distinct from the multi-declaration basin.
- verdict: KILLED

## [s14] The residual on the 10-point form is spread across the function and is attackable by re-spelling the mask block or the local declarations.
- mechanism: Prior sessions found the mask spelling (`*ptr &= 0xF8` vs symbol) and declaration order worth several points each.
- probe: Six variants on the 10-point chassis — declaration-order permutations (`r15`, `r16`), the mask split into read/and/write (`r17`), `*q = *q & 0xF8` (`r22`), a `u8 m` staging local (`r23`), and read-before-condition in the bit-1 block (`r24`).
- result: ALL SIX score exactly 10 at 49 insns — a hard plateau. The side-by-side shows why: the bit-2 block, the bit-4 block and the copy loop are instruction- AND register-identical to target, and the entire residual is the mask + bit-1 segment's register assignment (target: base a0, byte v1; build: base v1, byte a0) plus one memory op where target has a load-delay nop. With one pseudo re-set three times, local-alloc gives the pointer a single hard register for the whole function, so the first segment cannot be allocated differently from the rest.
- verdict: KILLED

## [s14] The target's four &D_80106A73 address materialisations require four separately DECLARED pointer locals (the banned construct); no other C spelling can produce them.
- mechanism: s12b's wave K priced every handle count (one 23, two 27, three 23, four 0) and s13's wave N priced all eight inline-helper shapes, and both concluded that four repeated `u8 *q = &D_80106A73;` declarations — or a macro expanding to exactly them — were the only route to the target's address/reload signature.
- probe: cc1 -da dumps of the banned four-declaration form (w4) and the one-handle form (w1) with the INSTRUMENTED tools/gcc-2.7.2/cc1 on the real build flags, sliced with tmp/grind/func_80034F88/s14/{slice,flat}.py; then a mechanism-derived variant sweep (r1-r24) scored with `sandbox func_80034F88 --disable all`.
- result: REFUTED. cse keys on whether the symbol_ref is present in its value table when the address is SET, not on how many C objects are declared. The two-armed if/else join labels end the cse basic block and flush the table, so a plain RE-ASSIGNMENT of ONE declared `u8 *q` after the flush survives as a real lui/addiu materialisation exactly as a fresh declaration does, and the previous block's store no longer hashes equal to the next block's load so the reload survives too. A single `u8 *q` re-assigned before the bit-2 and bit-4 blocks scores 10 at 49 build insns vs 49 target insns, against 13 for s12b/s13's static-inline helper.
- verdict: KILLED

## [s14] The three if/else join labels are NOT cse basic-block boundaries on this function (s2's conclusion, carried unchallenged for eleven sessions).
- mechanism: s2 cited tools/gcc-2.7.2/cse.c:8102-8184, which extends a cse basic block through a conditional branch that skips a block whenever LABEL_NUSES (JUMP_LABEL (p)) == 1, and asserted this holds for all three join labels here.
- probe: Read the .cse dumps of w4, w1 and r1 directly rather than reasoning from the source: check whether each block's `(set (reg) (symbol_ref "D_80106A73"))` survives cse and whether the QI mem count drops.
- result: The claim is TRUE for s2's own one-armed chassis (`val2 = val|K; if (!c) val2 = val;`, where the branch really does skip a block and fall through) and FALSE for the two-armed if/else chassis every form since s10 has used. In w4's .cse dump the bit-2 and bit-4 blocks' symbol_ref sets SURVIVE (regs 82 and 87) while the bit-1 block's is rewritten to a register copy (reg 77 <- reg 74) because it precedes the first flush; w1 keeps one set and loses three of its eight QI mems to store-to-load forwarding; r1 keeps three sets of the SAME pseudo and all eight QI mems. The flush is the entire mechanism behind the reloads and the per-block address materialisations.
- verdict: KILLED

## [s14] Score improves monotonically with the number of address handles, so a 2- or 3-handle form should sit between the one-handle 29 and the four-handle 0.
- mechanism: Wave K's price table was read as broadly monotone in handle count; on the re-assign chassis the same ordering was expected.
- probe: r19 (two pointer locals: q for the mask + bit-1 block, q2 re-assigned before the bit-2 and bit-4 blocks) and r20 (three pointer locals) against r8 (one local re-assigned three times), honest sandbox.
- result: 21 and 23 respectively — both far worse than the single re-assigned handle's 10, and worse than several one-handle forms. The axis is not monotone in handle count; the one-pseudo-re-set basin at 10 is structurally distinct from the multi-declaration basin, which is why eleven sessions of handle-count sweeps never found it.
- verdict: KILLED

## [s14] The residual on the new 10-point form is spread across the function and is attackable by re-spelling the mask block or reordering the local declarations.
- mechanism: Prior sessions found the mask spelling (`*ptr &= 0xF8` vs the plain symbol) and declaration order worth several points each on earlier chassis.
- probe: Six variants on the 10-point chassis: declaration-order permutations (r15, r16), the mask split into read/and/write through the handle (r17), `*q = *q & 0xF8` (r22), a `u8 m` staging local (r23), and read-before-condition in the bit-1 block (r24).
- result: ALL SIX score exactly 10 at 49 insns — a hard plateau. The side-by-side (s14/sbs.py) shows why: the bit-2 block, the bit-4 block and the whole copy loop are instruction- AND register-identical to target, and the entire residual is the mask + bit-1 segment — target puts the base in a0 and the loaded byte in v1, the build swaps them, and target's bit-1 block carries a load-delay nop where the build has a memory op.
- verdict: KILLED

## Resolved in s15 (forensics)

## [s15] The last 10 points are a pure local-alloc question: some OTHER live-range shape for ONE pointer object can give the mask + bit-1 segment a different hard register from the bit-2 / bit-4 segments (s14's live frontier).
- mechanism: s14 proposed that a re-assignment making the pointer dead between segments, an extra local whose live range crosses the first segment, or a different first-use order could give the allocator two allocnos for one declared pointer.
- probe: Instrumented-cc1 dumps (BB2_ALLOC_DEBUG / BB2_SUGG_DEBUG) of r8 (one object, 10), w4 (four declarations, banned, 0), r19 (two objects, 21) and t1 (two objects + declaration order, 10), read against global.c's allocno_compare and find_reg; plus 15 measured variants (t1-t8, u1-u4, v1-v4).
- result: KILLED as stated, and killed structurally rather than by sweep. GCC 2.7.2's global.c has no live-range splitting: one C pointer object is one DECL_RTL pseudo, one allocno and ONE hard register for the entire function. ALLOCDBG confirms it directly - r8's single pointer pseudo 74 has nrefs=10, live_length=29, priority 10344, is allocated at ord=4 ahead of the three byte allocnos (7500) and takes $a0 for the whole function. The target uses TWO different hard registers for the flag-byte base ($v1 for the mask + bit-1 segment, $a0 for bit-2 / bit-4 - note s14 recorded this orientation BACKWARDS). No single-object form can produce two base registers, so 10 is a hard floor for the entire one-handle family regardless of live-range spelling.
- verdict: KILLED

## [s15] The forensics line since s11 is valid: the instrumented cc1 whose dumps every forensics session reads is codegen-identical to the cc1 the sandbox actually builds with.
- mechanism: engine/buildconfig.py:19 compiles with tools/gcc-2.7.2/build/cc1 while the BB2_*_DEBUG-instrumented compiler is tools/gcc-2.7.2/cc1 - two different binaries (md5 8837b7da... vs 29b10d86...) that no session had ever cross-checked.
- probe: tmp/grind/func_80034F88/s15/xcheck.sh - compile the same preprocessed code6cac_b.i with both binaries and diff func_80034F88's emitted asm.
- result: CONFIRMED IDENTICAL for the r8 chassis. The instrumentation is print-only; the dumps are representative of the scored build. Re-run xcheck.sh if either binary is rebuilt.
- verdict: CONFIRMED

## [s15] The target's first-segment register assignment is produced by a copy-preference interaction in global.c, and the number of DECLARED pointer objects matters only through allocno priority.
- mechanism: global.c allocno_compare (lines 643-648) orders allocation by floor_log2(n_refs)*n_refs*10000*size/live_length; find_reg then excludes regs_someone_prefers[allocno] (global.c:1001), i.e. the registers a conflicting not-yet-allocated allocno prefers. In the banned four-declaration form the mask handle is confined to one basic block so local-alloc.c assigns it a hard register first; cse rewrites the bit-1 block's handle set into a register COPY from it (s15/rtl/w4/code6cac_b.i.lreg insn 27), which gives that allocno a copy-preference; and because each pointer allocno then carries only 3 refs over 14 insns its priority is 2142, so all three pointers are allocated AFTER the three byte allocnos (7500) and after `p` (3529). The bit-1 byte is pushed off $v1 by regs_someone_prefers and takes $a0; the pointer then takes $v1. That is exactly the target.
- probe: ALLOCDBG traces for r8 / w4 / r19 / t1 plus .lreg flattening (s15/flatten.py), cross-checked against the emitted asm and asm/funcs/func_80034F88.s.
- result: CONFIRMED, and it is a complete explanation of the residual: the priority table predicts every measured score. r8's pointer at 10344 allocates before the bytes (10); r19's two-object pointer ties the bytes at 7500 and wins on allocno number (21); t1 is the same body with the value locals declared FIRST so the tie goes to the bytes (10); w4's three pointer allocnos at 2142 allocate last (0).
- verdict: CONFIRMED

## [s15] Declaration order at function scope is worth double-digit points on this function, because it decides a priority TIE in global.c's allocno comparator.
- mechanism: allocno_compare ends with `return *v1 - *v2;` - the allocno number, which follows pseudo number, which follows DECL order. When the mask+bit-1 pointer allocno and the three loaded-byte allocnos both price at 7500, declaration order alone decides who picks a hard register first.
- probe: Matched pairs differing ONLY in whether the six value locals are declared before or after the pointer locals: t1 vs t2 (two pointer objects) and v1 vs v3 (three pointer objects); honest sandbox --disable all.
- result: CONFIRMED and large. Two-object chassis 21 -> 10; three-object chassis 13 -> 0. This lever was never isolated in fourteen prior sessions (s3 measured "declaration order is neutral" on the one-object chassis, which is true there - with one pointer allocno at 10344 there is no tie to break).
- verdict: CONFIRMED

## [s15] A THREE-object pointer form reaches the target bytes, so the four banned declarations are not the unique route.
- mechanism: The three ingredients above, spelled with three function-scope pointers: `qm` for the 0xF8 mask (block-local, so local-alloc assigns it first), `q1` set inside the bit-1 block (before the first cse flush, so cse turns it into a copy of qm and it inherits the copy-preference), and `q2` set inside the bit-2 block and re-set inside the bit-4 block (both after a flush, so both survive as real materialisations) - plus the six value locals declared before the pointers to win the priority tie.
- probe: s15/variants/v1.c and v2.c (v2 spells the bit-1 handle `q1 = qm;` explicitly), honest sandbox --disable all plus the objdump census.
- result: BOTH SCORE 0 at 49 build insns vs 49 target insns, lbu 176 / sb 164 / lui 456 - the target's exact access census. NOT SUBMITTED: three handles on one global whose purpose is to give the allocator more allocnos is the same INTENT as the four declarations the driver banned, and cheat-checklist T5 forbids a session from self-approving a respelling. s15 returned ruling-request with these forms attached as the evidence.
- verdict: CONFIRMED (as codegen), CLASSIFICATION-BLOCKED (as a submission)

## [s15] s2's 'stage the next block's pointer before the current block's store' lever transfers to the multi-object chassis.
- mechanism: s2 measured that assigning the next block's handle before the current block's store scored 12 against 18 for assigning it before the condition, and the target's own asm does put block 2's lui/addiu ahead of block 1's sb.
- probe: t5 (two objects, staged), t6 (three objects, staged), t8 (mixed) against t1 and v1 on the same declaration order.
- result: KILLED on this chassis. t5 = 20 with lbu 174 (two reloads lost), t6 = 23, t8 = 10 (no gain). Staging moves the handle's SET to BEFORE the cse value-table flush, which degrades the fresh materialisation into a register copy and lets the next block's load be forwarded away. s2's lever was measured on the one-armed chassis where the join labels are not flushes; it is negative on the two-armed if/else chassis. The 0-scoring form sets each handle INSIDE its own block, after the previous join label.
- verdict: KILLED

## [s15] Spelling the 0xF8 mask through the plain symbol frees the pointer objects to reproduce the target's bases.
- mechanism: If the mask does not consume a pointer handle, the mask+bit-1 pointer allocno loses two refs, which should drop its priority below the byte allocnos' 7500 without needing a third object.
- probe: u1 (`D_80106A73 &= 0xF8;` + two pointer objects), u4 (same, mask written longhand), u2 (three objects), u3 (u1 with pointers declared first).
- result: KILLED. 16 / 16 / 27 / 27, all at 51 insns with lui 458. u1 is the first TWO-object form to produce all four target reloads (lbu 176), but the symbol-spelled mask pays its own address materialisation instead of sharing the bit-1 block's base, costing 2 instructions and 2 luis; the target's mask block and bit-1 block share one unfolded $v1 base.
- verdict: KILLED

## [s16] A second addend-0 base register can be obtained from an ANONYMOUS reference (a direct symbol access or a pointer-arithmetic temporary) instead of from a second declared/assigned C pointer object, so the target's two base live ranges are reachable without touching the banned construct.
- mechanism: The Judge's binding constraint bans declaring or assigning more than ONE C pointer object aliasing D_80106A73. An anonymous reference is neither declared nor assigned; if GCC materialised one into an unnamed pseudo, that pseudo would get its own allocno in global.c and could receive a second hard register — reproducing the target's $v1 (mask + bit-1) / $a0 (bit-2, bit-4) split with one named object.
- probe: Four anonymous spellings on the r8 single-object chassis, scored on the honest sandbox, plus a BB2_ALLOC_DEBUG allocno trace and generated-asm inspection of the best one. a1 (q covers mask+bit-1, bit-2/bit-4 direct), a2 (mask+bit-1 direct, q covers bit-2/bit-4), a3 (only the bit-4 block direct), a4 (bit-2/bit-4 spelled `*((u8 *)&D_80106A70 + 3)`). Artifacts: tmp/grind/func_80034F88/s15c/variants/*.c, s15c/results.json, s15c/rtl/a2/cc1.err, s15c/rtl/a2/code6cac_b.s.
- result: KILLED, at the RTL level and not merely by score. a1 = 28, a2 = 14, a3 = 16, a4 = 28 against the single-object control r8 = 10 (re-measured this session), and every anonymous form raises the lui census to 457/458 against the target's 456 — each anonymous access pays its own materialisation instead of sharing a base. The a2 allocno trace (s15c/rtl/a2/cc1.err:637-646) shows the IDENTICAL pseudo set to the single-object control — 73/78/82/86/77/81/85/74/72, nine allocnos, one pointer pseudo (74, hardreg 4 = $a0) — i.e. replacing three of q's uses with anonymous references created no new pseudo at all. GCC 2.7.2 keeps a direct symbol_ref inline in the MEM address on MIPS, so an anonymous reference never occupies a register and can never be a base. The generated asm confirms: `lbu $2,D_80106A73` / `sb $2,D_80106A73` for the direct segment against `0($4)` for the pointer segment.
- verdict: KILLED

## [s16] Reload can be made to rematerialise the pointer into a different register per reference, so SPILLING one pointer object substitutes for declaring two.
- mechanism: When the single pointer pseudo fails to win a hard register, reload1.c attaches `reg_equiv_constant = (symbol_ref "D_80106A73")` to it (confirmed present at s14/rtl/w1/code6cac_b.i.lreg:43747-43748). Reload could in principle rematerialise that address into a fresh reload register at each reference — two references, two registers, one C object.
- probe: Direct source read of the reload path handling a REG address carrying a `reg_equiv_constant`, `tools/gcc-2.7.2/reload.c:4128-4137`, read verbatim this session.
- result: KILLED in compiler source. `find_reloads_address` short-circuits before any reload register is considered: `if (reg_equiv_constant[regno] != 0 && strict_memory_address_p (mode, reg_equiv_constant[regno])) { *loc = ad = reg_equiv_constant[regno]; return 1; }`. The MEM's ADDRESS is replaced by the constant and no register is allocated. On MIPS a bare symbol_ref is a legitimate memory address, so `strict_memory_address_p` is always true here and the access degenerates to the plain-symbol form — the measured 28-29 family (w1 = 29; re-confirmed by a1 = a4 = 28 this session), never an addend-0 base.
- verdict: KILLED

## [s16] The single-pointer-object floor of 10 is a STRUCTURAL ceiling of GCC 2.7.2, not a search plateau.
- mechanism: The target needs &D_80106A73 in two different hard registers — $v1 across the mask + bit-1 segment, and $a0 set twice across the bit-2 and bit-4 segments (read literally off asm/funcs/func_80034F88.s this session). Hard registers are handed out per ALLOCNO; `tools/gcc-2.7.2/global.c:426` creates exactly one allocno per pseudo (`reg_allocno[i] = max_allocno++;`; the neighbouring `reg_may_share` branch only MERGES allocnos), `find_reg` assigns one hard register per allocno for its entire live range, and 2.7.2 has no live-range-splitting pass. So two hard registers require two pseudos, and — with the two kills above closing both routes to a pseudo that is not a named C object — two pseudos require two C pointer objects, which is precisely the banned construct.
- probe: global.c:426 and reload.c:4128-4137 source verification; the a1-a4 anonymous sweep; the r8 (10) and w1 (29) single-object controls; the target's register assignment read directly from the asm.
- result: CONFIRMED. Every remaining pure-C dial — declaration order, live-range shape, re-assignment count, mask spelling, value-local placement, statement order, and 6,450+ permuter iterations across four campaigns — operates on WHICH single hard register the one pointer allocno receives and on the reload census. None of them can change HOW MANY registers it receives. That is why fifteen prior sessions across six modalities all bottom out at 9/10 with one object and reach 0 with two or more.
- verdict: CONFIRMED

## [s15] A second addend-0 base register can be obtained from an ANONYMOUS reference (a direct symbol access or a pointer-arithmetic temporary) instead of from a second declared/assigned C pointer object, so the target's two base live ranges are reachable without touching the banned construct.
- mechanism: The Judge's binding constraint bans declaring or assigning more than ONE C pointer object aliasing D_80106A73. An anonymous reference is neither declared nor assigned; if GCC materialised one into an unnamed pseudo, that pseudo would get its own allocno in global.c and could receive a second hard register, reproducing the target's $v1 (mask + bit-1) / $a0 (bit-2, bit-4) split with a single named object.
- probe: Four anonymous spellings on the r8 single-object chassis, scored on the honest sandbox, plus a BB2_ALLOC_DEBUG allocno trace and generated-asm inspection of the best one: a1 (q covers mask+bit-1, bit-2/bit-4 direct), a2 (mask+bit-1 direct, q covers bit-2/bit-4), a3 (only the bit-4 block direct), a4 (bit-2/bit-4 spelled *((u8 *)&D_80106A70 + 3)).
- result: a1 = 28, a2 = 14, a3 = 16, a4 = 28 against the re-measured single-object control r8 = 10, and every anonymous form raises the lui census to 457/458 against the target's 456 — each anonymous access pays its own materialisation instead of sharing a base. The a2 allocno trace (s15c/rtl/a2/cc1.err:637-646) lists the IDENTICAL pseudo set to the single-object control (73/78/82/86/77/81/85/74/72 — nine allocnos, one pointer pseudo 74 at hardreg 4), i.e. replacing three of q's uses with anonymous references created no new pseudo at all. GCC 2.7.2 keeps a direct symbol_ref inline in the MEM address on MIPS, so an anonymous reference never occupies a register and can never be a base; the generated asm shows lbu $2,D_80106A73 / sb $2,D_80106A73 for the direct segment against 0($4) for the pointer segment.
- verdict: KILLED

## [s15] Reload can be made to rematerialise the pointer into a different register per reference, so spilling ONE pointer object substitutes for declaring two.
- mechanism: When the single pointer pseudo fails to win a hard register, reload1.c attaches reg_equiv_constant = (symbol_ref "D_80106A73") to it (confirmed present at s14/rtl/w1/code6cac_b.i.lreg:43747-43748). Reload could in principle rematerialise that address into a fresh reload register at each reference — two references, two registers, one C object.
- probe: Direct source read of the reload path that handles a REG address carrying a reg_equiv_constant: tools/gcc-2.7.2/reload.c:4128-4137, read verbatim this session.
- result: find_reloads_address short-circuits before any reload register is considered: `if (reg_equiv_constant[regno] != 0 && strict_memory_address_p (mode, reg_equiv_constant[regno])) { *loc = ad = reg_equiv_constant[regno]; return 1; }`. The MEM's ADDRESS is replaced by the constant and no register is allocated. On MIPS a bare symbol_ref is a legitimate memory address, so strict_memory_address_p is always true here and the access degenerates to the plain-symbol form — the measured 28-29 family (w1 = 29; re-confirmed by a1 = a4 = 28 this session), never an addend-0 base.
- verdict: KILLED

## [s15] The single-pointer-object floor of 10 is a structural ceiling of GCC 2.7.2, not a search plateau.
- mechanism: The target needs &D_80106A73 in two different hard registers — $v1 across the mask + bit-1 segment, and $a0 set twice across the bit-2 and bit-4 segments (read literally off asm/funcs/func_80034F88.s this session). Hard registers are handed out per ALLOCNO; tools/gcc-2.7.2/global.c:426 creates exactly one allocno per pseudo (reg_allocno[i] = max_allocno++;, the neighbouring reg_may_share branch only MERGES allocnos), find_reg assigns one hard register per allocno for its entire live range, and 2.7.2 has no live-range-splitting pass. Two hard registers therefore require two pseudos, and with both non-object routes to a pseudo killed above, two pseudos require two C pointer objects — the banned construct.
- probe: global.c:426 and reload.c:4128-4137 source verification; the a1-a4 anonymous sweep; the r8 (10) and w1 (29) single-object controls; the target's register assignment read directly from asm/funcs/func_80034F88.s.
- result: Confirmed. Every remaining pure-C dial — declaration order, live-range shape, re-assignment count, mask spelling, value-local placement, statement order, and 6,450+ permuter iterations across four campaigns — operates on WHICH single hard register the one pointer allocno receives and on the reload census. None can change HOW MANY it receives. That is why fifteen prior sessions across six modalities all bottom out at 9/10 with one object and reach 0 with two or more.
- verdict: CONFIRMED

## [s16] The single pointer allocno can be steered onto $v1 (the target's block-1 base) by declaration order, live-range length, or an explicit register pin.
- mechanism: global.c allocno_compare orders allocation by pri = floor_log2(n_refs)*n_refs*10000*size/live_length and falls through to allocno number (pseudo declaration order) on a tie; find_reg then excludes regs_someone_prefers. If the pointer's priority or conflict set can be changed, the hard register it wins should change with it.
- probe: eight single-object variants (tmp/grind/func_80034F88/s16/variants, results.json): pointer declared first/middle/last (d1/d2/d3), all temporaries hoisted to function scope (f1), byte pointer for p (pb1), pointer live only across block 1 (seg1), pointer live only across blocks 2+4 (seg23), plus the diagnostic `register u8 *q asm("$3"/"$4"/"$2")` pins.
- result: d1/d2/d3/f1/pb1 all score 10 with an identical instruction stream — declaration order is completely inert. seg1 puts the pointer on $a1 (score 28); seg23 leaves it on $a0 (score 14). All three register pins are IGNORED (GCC 2.7.2 honours a local register variable only where it appears as an asm operand): pin_v1's emitted stream is instruction-identical to the unpinned candidate. No shape, legal or otherwise, moves the allocno to $v1.
- verdict: KILLED

## [s16] $v1 would be a better register for the single pointer than $a0 (the residual might not redistribute symmetrically).
- mechanism: the target's block 1 uses $v1 as the base and $a0 for the loaded byte; blocks 2 and 4 use $a0 as the base and $v1 for the byte. A single allocno serves whichever segment its register matches.
- probe: instruction-aligned side-by-side of the banked candidate against target (s16/sbs.py base.c) — count the lines each choice repairs and breaks.
- result: our build's $a0 makes blocks 2 and 4 EXACTLY identical to target (0 diffs across both) and costs 5 swapped lines in block 1. Switching to $v1 repairs those 5 and breaks the 5 equivalent lines in each of blocks 2 and 4: net +5, i.e. score ~15. The residual is asymmetric in $a0's FAVOUR; the compiler is already picking the better of the two.
- verdict: KILLED

## [s16] The block-1 post-store reload (the target's second `lbu`, our maspsx nop) is reachable from a single C pointer object.
- mechanism: GCC 2.7.2 cse.c hashes a MEM on its ADDRESS RTX. After `sb reg,0(pseudo)` the entry for (mem:QI (reg pseudo)) holds reg, so a later load through the SAME pseudo is satisfied from the register and the reload is folded. Only a different address rtx misses the entry. Blocks 2 and 4 escape this because a two-armed if/else JOIN LABEL flushes the value table between their store and their read (s14); block 1 has no preceding join, and the one intervening insn (`lw v0,32(a1)`) is a LOAD, which does not invalidate.
- probe: every single-object shape measured this session (13 variants) plus the s14/s15 banks; lbu census on the sandbox object against the target's 176.
- result: every single-object form comes in at lbu 175 — exactly one reload short, always block 1's. Restating `q = &D_80106A73;` before block 1 does not help: one C local is one DECL_RTL is one pseudo, so the restatement is a redundant set to the same pseudo and the hash entry survives. A second address pseudo requires a second C object.
- verdict: KILLED

## [s16] m2c's fresh reconstruction of the target reveals a value shape we have not tried (compute the OR first, else-arm overrides).
- mechanism: m2c reconstructs from the asm, so its statement shape is evidence about what the original compiler was fed.
- probe: `python3 tools/m2c/m2c.py --target mipsel-gcc-c asm/funcs/func_80034F88.s`, then three transcriptions: literal (no pointer, m3), on the single-pointer chassis with u8 temporaries (m1) and with s32 temporaries (m2).
- result: m3 = 26 at 45 insns (lui 458); m1 = m2 = 21 at 45 insns. All three come out FOUR instructions short of the target's 49 — GCC folds m2c's two reads of the byte into one and then collapses the diamond. m2c's shape is a worse shape, not an untried lever; candidate.c's symmetric if/else is what reproduces the target's diamond.
- verdict: KILLED

## [s16] In BB2's already-matched corpus, the "one global in two base registers" codegen is reached by some pure-C spelling OTHER than multiple C handles on that global.
- mechanism: if a matched, zero-rule function anywhere in the tree produces two base registers for one symbol from a single C handle, that spelling transplants directly onto func_80034F88 and closes it inside the Judge's constraint.
- probe: tmp/grind/func_80034F88/s16/sibling_scan.py + sibling_scan2.py — sweep all asm/funcs/*.s for a symbol materialised into 2+ distinct registers where both are used as a memory base; cross-reference engine/queue.json to keep only zero-cheat COMPLETED functions; read their C.
- result: 34 functions in the binary have the construct, 9 of them COMPLETED with zero rules. The closest, func_80037F40 (src/code6cac_c_mid.c, commit 89bfc882, D_80106A50 in $v1 and $a2), reaches it with TWO C pointer objects on the one global — `p = (u8 *)&g_file_disc_size;` (:196) and `src = (Quad *)&g_file_disc_size;` (:211) — of different types doing different jobs, so incidental to the match that the commit message never mentions them. No corpus example reaches two base registers from a single C handle. The construct and the multiple-C-object source are one-to-one.
- verdict: KILLED

## [s16] The single pointer allocno can be steered onto $v1 (the target's block-1 base) by declaration order, live-range length, or an explicit register pin.
- mechanism: global.c allocno_compare orders allocation by pri = floor_log2(n_refs)*n_refs*10000*size/live_length and falls through to allocno number (pseudo declaration order) on a tie; find_reg then excludes regs_someone_prefers. If the pointer's priority or conflict set can be changed, the hard register it wins should change with it.
- probe: Eight single-object variants (tmp/grind/func_80034F88/s16/variants, results.json): pointer declared first/middle/last (d1/d2/d3), all temporaries hoisted to function scope (f1), byte pointer for p (pb1), pointer live only across block 1 (seg1), pointer live only across blocks 2+4 (seg23), plus diagnostic `register u8 *q asm("$3"/"$4"/"$2")` pins.
- result: d1/d2/d3/f1/pb1 all score 10 with an identical instruction stream, so declaration order is completely inert. seg1 puts the pointer on $a1 (score 28); seg23 leaves it on $a0 (score 14). All three register pins are IGNORED by GCC 2.7.2 (a local register variable is only honoured where it appears as an asm operand) -- pin_v1's emitted stream is instruction-identical to the unpinned candidate, score 10, same census. No shape, legal or otherwise, moves the allocno to $v1.
- verdict: KILLED

## [s16] $v1 would be a better register for the single pointer than $a0, so the 10-point residual might not redistribute symmetrically.
- mechanism: The target's block 1 uses $v1 as the base and $a0 for the loaded byte; blocks 2 and 4 use $a0 as the base and $v1 for the byte. A single allocno serves whichever segment its register matches.
- probe: Instruction-aligned side-by-side of the banked candidate against target (tmp/grind/func_80034F88/s16/sbs.py base.c), counting the lines each choice repairs and breaks.
- result: Our build's $a0 makes blocks 2 and 4 EXACTLY identical to target (zero diffs across both) at a cost of 5 swapped lines in block 1. Switching to $v1 would repair those 5 and break the 5 equivalent lines in each of blocks 2 and 4: net +5, i.e. a score near 15. The residual is asymmetric in $a0's favour -- the compiler already picks the better of the two.
- verdict: KILLED

## [s16] The block-1 post-store reload (the target's second `lbu`, our maspsx nop) is reachable from a single C pointer object.
- mechanism: GCC 2.7.2 cse.c hashes a MEM on its ADDRESS RTX. After `sb reg,0(pseudo)` the entry for (mem:QI (reg pseudo)) holds reg, so a later load through the SAME pseudo is satisfied from that register and the reload is folded away. Only a different address rtx misses the entry. Blocks 2 and 4 escape this because a two-armed if/else JOIN LABEL flushes the cse value table between their store and their read (the s14 finding); block 1 has no preceding join, and the single intervening insn (`lw v0,32(a1)`) is a LOAD, which does not invalidate.
- probe: All 13 single-object shapes measured this session plus the s14/s15 banks; lbu census on the sandbox object against the target's 176.
- result: Every single-object form comes in at lbu 175 -- exactly one reload short, always block 1's. Restating `q = &D_80106A73;` before block 1 does not help: one C local is one DECL_RTL is one pseudo, so the restatement is a redundant set to the same pseudo and the hash entry survives untouched. A second address pseudo requires a second C object. This is the s15 register-allocator ceiling re-derived independently from the CSE side.
- verdict: KILLED

## [s16] m2c's fresh reconstruction of the target reveals a value shape we have not tried (compute the OR first, else-arm overrides).
- mechanism: m2c reconstructs from the asm, so its statement shape is evidence about what the original compiler was fed.
- probe: `python3 tools/m2c/m2c.py --target mipsel-gcc-c asm/funcs/func_80034F88.s`, then three transcriptions: literal / no pointer (m3), single-pointer chassis with u8 temporaries (m1), same with s32 temporaries (m2).
- result: m3 = 26 at 45 build insns (lui 458); m1 = m2 = 21 at 45 insns. All three land FOUR instructions short of the target's 49 -- GCC folds m2c's two reads of the byte into one and then collapses the diamond. m2c's shape is a worse shape, not an untried lever; candidate.c's symmetric `if (c) c = v | bit; else c = v;` is what reproduces the target's diamond.
- verdict: KILLED

## [s16] Somewhere in BB2's already-matched corpus, the 'one global in two base registers' codegen is reached by a pure-C spelling OTHER than multiple C handles on that global -- a spelling that would transplant onto func_80034F88 inside the Judge's one-object constraint.
- mechanism: A matched, zero-rule function that produces two base registers for one symbol from a single C handle is a direct counter-example to the ceiling and an immediately transplantable form.
- probe: New tools tmp/grind/func_80034F88/s16/sibling_scan.py + sibling_scan2.py: sweep all 1,437 asm/funcs/*.s for a symbol materialised into 2+ distinct registers via lui/addiu where BOTH registers are then used as a memory base (offset($reg)), excluding address-as-a-value cases; cross-reference engine/queue.json to keep only zero-cheat COMPLETED functions; read their C bodies.
- result: 34 functions in the binary carry the construct; 9 are COMPLETED with zero regfix/asmfix rules (SpuSetKey, SpuSetReverbModeParam, func_8002C0DC, func_80037F40, func_8004A940 x2 symbols, func_8008241C, func_80082D34, startIntrDMA, startIntrVSync). The closest, func_80037F40 (src/code6cac_c_mid.c, commit 89bfc882, D_80106A50 in both $v1 and $a2), reaches it with TWO C pointer objects on the one global -- `p = (u8 *)&g_file_disc_size;` at :196 and `src = (Quad *)&g_file_disc_size;` at :211 -- of different types doing different jobs, so incidental that the match commit never mentions them. No corpus example reaches two base registers from a single C handle. The codegen and the multiple-C-object source are one-to-one.
- verdict: KILLED

## [s17] s1's F3 -- 0x80106A70..0x80106A73 was ONE declared 4-byte object -- is still open, because s2 killed it on evidence that does not exist.
- mechanism: s2's kill reads "Refuted directly by the shipped relocations ... Two distinct symbols in the relocation records means the original source declared two distinct objects. No measurement needed." disc/SLUS_006.63 is a PS-X EXE (flat, absolutely linked, no reloc table -- verified from the file magic this session), so there are no relocation records to read. The %hi/%lo symbol names in asm/funcs/func_80034F88.s are splat's reconstruction from absolute lui/addiu immediates against its own auto-name table (undefined_syms_auto.txt:981-984 defines D_80106A70..73 as four consecutive one-byte names), exactly the trap memory/splat-symbol-names-are-not-evidence.md documents.
- probe: Re-opened and answered by measurement instead: variants a1_arr_all (every flag address spelled `&D_80106A70 + 3`), a2_arr_first and a3_arr_last (mixed addend on the single pointer object), scored with `sandbox func_80034F88 --disable all` plus an objdump -dr relocation dump (tmp/grind/func_80034F88/s17/reloc.py).
- result: KILLED, but for the opposite reason and now on real evidence: the model is codegen-NEUTRAL, not refuted. a1_arr_all scores 12 at 49 insns with lbu 175 / sb 164 / lui 456 -- the floor-10 base's exact instruction count and exact census. objdump -r shows a1's three base pairs carry R_MIPS_HI16/LO16 against D_80106A70 with an in-field addend of 3 where base carries them against D_80106A73 with addend 0; 0x80106A70+3 == 0x80106A73 and %hi is 0x8010 either way, so the LINKED bytes are identical and the +2 is a FALSE distance (engine/score.py does not mask R_MIPS_LO16 addends). GCC folds the +3 into the pointer's own `la`, not into a MEM displacement, so every access stays at displacement 0 like the target. The declaration model cannot help or hurt. Banked at rejected/array-model-4byte-object-score12-FALSE-addend.c.
- verdict: KILLED

## [s17] The target's missing block-1 reload can be bought back on the floor-10 chassis for less than the +5 the ledger had it priced at.
- mechanism: cse forwards block 1's flag read from the mask store because both MEMs are (mem:QI (reg q)) -- the identical address rtx. With one pointer object the only differing rtx available is the bare (mem:QI (symbol_ref)), so the question is purely WHERE to place one direct-symbol access so the cse miss costs least.
- probe: Seven placements measured through tmp/grind/func_80034F88/s17/probe.py: e1 block-1 read only, e2 mask store only, e3 block-1 store only, d1 whole mask, d2 whole block 1, d3 whole mask with addend 3, d4 whole block 3; each scored with `sandbox --disable all` plus an objdump lbu/sb/lui census, and e1 read against the target with s17/sbs.py.
- result: CONFIRMED and repriced from +5 to +1. e1 (block 1's READ alone via the symbol) scores 11 at 50 insns with lbu 176 / sb 164 -- the TARGET's exact access census, the first form on this chassis to carry it -- and its whole cost is one extra `lui $at` (lui 457 vs the target's 456). The side-by-side shows `lui v1,%hi / lbu v1,%lo(v1)` where the target has a single `lbu a0,0($v1)`. Others: e2 15, e3 18, d1 16, d2 15, d3 16, d4 16. Banked at rejected/b1-read-direct-symbol-reload-priced-at-1-score11.c.
- verdict: CONFIRMED

## [s17] The +1 reprice opens a NEW route below the floor of 10.
- mechanism: If the reload is only worth 1 point and the base costs 1 point, a placement might exist where the symbol read's lui is absorbed by an address materialisation the target already has.
- probe: The same seven placements, plus the census and side-by-side reading of e1.
- result: KILLED, and the kill is exact rather than empirical. Base spends exactly 1 point on the absent reload; e1 spends exactly 1 point on the lui that buys it. The trade is balanced because the compiler charges one address materialisation either way: to obtain the reload WITHOUT the extra lui, block 1's read must go through a REGISTER whose address rtx differs from the mask store's, which is a second address pseudo, which (one C object is one DECL_RTL is one pseudo) is a second C pointer object -- the construct the Judge banned. Every placement that keeps all four accesses on the single pointer scores 10; every placement that introduces a symbol access scores 11-18. There is no third option.
- verdict: KILLED

## [s17] The function's semantics are simpler than the three-block chassis assumes, and a spec-level re-derivation reaches the target from a different direction.
- mechanism: The three flag blocks are literally "copy bits 0..2 of p[8] into D_80106A73", so the natural spec-level C is one mask-and-merge statement; if GCC expands that into the target's shape the whole chassis question dissolves.
- probe: r1_bitcopy `*q = (*q & 0xF8) | (p[8] & 7);` and r2_accum (three ifs ORing into one value, single store), both through s17/probe.py.
- result: KILLED. r1 scores 34 at 27 insns, r2 scores 30 at 35, against the target's 49. The target performs four lbu and four sb on the flag byte and three `bnez` selects (`andi $v0,$v0,K / bnez / ori $v0,$a0,K / addu $v0,$a0,$zero`); no branchless or single-store spelling can produce that. The original source really did write three separate read-modify-write blocks. Banked at rejected/bitcopy-semantic-simplification-score34.c.
- verdict: KILLED

## [s17] Frontier bullet 2 -- a second address object derived from a genuinely different program value (an address that happens to equal &D_80106A73 but is not spelled as a constant).
- mechanism: The ceiling is the number of address PSEUDOS, not the C spelling; a non-constant expression yielding &D_80106A73 would create a second allocno that local-alloc's update_equiv_regs could not substitute away.
- probe: Read the only non-constant value in the function to its definition: `p = func_80077D00()`, which is `s32* func_80077D00(void) { return &D_8009BD24; }` at src/text1b_b.c:898.
- result: KILLED. p is 0x8009BD24; D_80106A73 - 0x8009BD24 = 0x6AD4F, an arbitrary distance across a segment boundary (.data ends ~0x800A3800, D_80106A73 is bss). No human-written expression relates them, and manufacturing the relationship with an opaque zero is a first-reach lever, explicitly excluded when the bullet was written. The function contains no non-constant expression that yields the flag address.
- verdict: KILLED

## [s17] The -vs- base-register dial, dead on the floor-10 base chassis (s16), is live on the NEW e1 chassis.
- mechanism: s16 killed the dial three ways but only on the base chassis. e1 is structurally different -- block 1s flag read is a direct symbol access, so there is an extra read temporary and the pointer q carries one fewer reference -- and e1s side-by-side shows build and target with the two registers exactly swapped in block 1, so on this chassis the dial looked worth most of the 11.
- probe: Wave F, five bodies through tmp/grind/func_80034F88/s17/probe.py, all single-pointer-object: f1 q assigned before the call, f2 q declared first among the locals, f3 mask also via the symbol so qs live range starts at block 1s store, f4 read hoisted above the condition, f5 u8 sub-word read temporary.
- result: KILLED, identically to s16. f2, f4 and f5 all score 11 at 50 insns with lbu 176 / sb 164 / lui 457 -- bit-for-bit the same result as e1 itself, so declaration order, statement order and temporary width are all inert on this chassis too. f1 (q before the call) is 29 at 52 insns and f3 (mask via symbol) is 15 at 50 with lbu back down to 175, both strictly worse. The dial does not respond to any source-level lever on either chassis; it is a property of local-allocs assignment order, not of the C.
- verdict: KILLED

## [s17] s1's F3 - 0x80106A70..0x80106A73 was ONE declared 4-byte object in the original source - is still open, because s2 killed it on evidence that does not exist.
- mechanism: s2's kill reads 'Refuted directly by the shipped relocations ... Two distinct symbols in the relocation records means the original source declared two distinct objects. No measurement needed.' disc/SLUS_006.63 is a PS-X EXE - flat, absolutely linked, no reloc table - so there are no relocation records to read. The %hi/%lo symbol names in asm/funcs/func_80034F88.s are splat's reconstruction from absolute lui/addiu immediates against its own auto-name table (undefined_syms_auto.txt:981-984 defines D_80106A70..73 as four consecutive one-byte names), exactly the trap memory/splat-symbol-names-are-not-evidence.md documents.
- probe: Re-opened and answered by measurement: variants a1_arr_all (every flag address spelled &D_80106A70 + 3), a2_arr_first and a3_arr_last (mixed addend on the single pointer object), scored with sandbox --disable all plus an objdump -dr relocation dump (tmp/grind/func_80034F88/s17/reloc.py).
- result: KILLED, but for the opposite reason and now on real evidence: the model is codegen-NEUTRAL, not refuted. a1_arr_all scores 12 at 49 insns with lbu 175 / sb 164 / lui 456 - the floor-10 base's exact instruction count and exact census. objdump -r shows a1's three base pairs carry R_MIPS_HI16/LO16 against D_80106A70 with an in-field addend of 3 where base carries them against D_80106A73 with addend 0; 0x80106A70+3 == 0x80106A73 and %hi is 0x8010 either way, so the LINKED bytes are identical and the +2 is a FALSE distance (engine/score.py does not mask R_MIPS_LO16 addends). GCC folds the +3 into the pointer's own la, not into a MEM displacement, so every access stays at displacement 0 like the target. Mixed-addend spellings (a2 10, a3 12) confirm the addend is invisible once the address is in a pointer. Banked at rejected/array-model-4byte-object-score12-FALSE-addend.c.
- verdict: KILLED

## [s17] The target's missing block-1 reload can be bought back on the floor-10 chassis for less than the +5 the ledger had it priced at.
- mechanism: cse forwards block 1's flag read from the mask store because both MEMs are (mem:QI (reg q)) - the identical address rtx. With one pointer object the only differing rtx available is the bare (mem:QI (symbol_ref)), so the question is purely WHERE to place one direct-symbol access so the cse miss costs least.
- probe: Seven placements through tmp/grind/func_80034F88/s17/probe.py: e1 block-1 read only, e2 mask store only, e3 block-1 store only, d1 whole mask, d2 whole block 1, d3 whole mask with addend 3, d4 whole block 3; each with sandbox --disable all plus an objdump lbu/sb/lui census, e1 read against the target with s17/sbs.py.
- result: CONFIRMED and repriced from +5 to +1. e1 (block 1's READ alone via the symbol) scores 11 at 50 insns with lbu 176 / sb 164 - the TARGET's exact access census, the first form on this chassis to carry it - and its whole cost is one extra lui $at (lui 457 vs the target's 456). The side-by-side shows 'lui v1,%hi / lbu v1,%lo(v1)' where the target has a single 'lbu a0,0($v1)'. Others: e2 15, e3 18, d1 16, d2 15, d3 16, d4 16. Banked at rejected/b1-read-direct-symbol-reload-priced-at-1-score11.c.
- verdict: CONFIRMED

## [s17] The +1 reprice opens a NEW route below the floor of 10.
- mechanism: If the reload is only worth 1 point and the base costs 1 point, a placement might exist where the symbol read's lui is absorbed by an address materialisation the target already has.
- probe: The same seven placements, plus the census and side-by-side reading of e1.
- result: KILLED, and the kill is exact rather than empirical. Base spends exactly 1 point on the absent reload; e1 spends exactly 1 point on the lui that buys it. The trade is balanced because the compiler charges one address materialisation either way: to obtain the reload WITHOUT the extra lui, block 1's read must go through a REGISTER whose address rtx differs from the mask store's, which is a second address pseudo, which (one C object is one DECL_RTL is one pseudo) is a second C pointer object - the construct the Judge banned. Every placement that keeps all four accesses on the single pointer scores 10; every placement that introduces a symbol access scores 11-18. There is no third option.
- verdict: KILLED

## [s17] The $v1-vs-$a0 base-register dial, dead on the floor-10 base chassis (s16), is live on the NEW e1 chassis.
- mechanism: s16 killed the dial three ways but only on the base chassis. e1 is structurally different - block 1's flag read is a direct symbol access, so there is an extra read temporary and q carries one fewer reference - and e1's side-by-side shows build and target with the two registers exactly swapped in block 1, so the dial looked worth most of e1's 11.
- probe: Wave F, five single-pointer-object bodies through s17/probe.py: f1 q assigned before the call, f2 q declared first among the locals, f3 mask also via the symbol so q's live range starts at block 1's store, f4 read hoisted above the condition, f5 u8 sub-word read temporary.
- result: KILLED, identically to s16. f2, f4 and f5 all score 11 at 50 insns with lbu 176 / sb 164 / lui 457 - bit-for-bit e1's own result, so declaration order, statement order and temporary width are inert on this chassis too. f1 is 29 at 52 insns and f3 is 15 at 50 with lbu back down to 175, both strictly worse. The dial does not respond to any source-level lever on either chassis; it is a property of local-alloc's assignment order, not of the C.
- verdict: KILLED

## [s17] The function's semantics are simpler than the three-block chassis assumes, and a spec-level re-derivation reaches the target from a different direction.
- mechanism: The three flag blocks are literally 'copy bits 0..2 of p[8] into D_80106A73', so the natural spec-level C is one mask-and-merge statement; if GCC expands that into the target's shape the whole chassis question dissolves.
- probe: r1_bitcopy '*q = (*q & 0xF8) | (p[8] & 7);' and r2_accum (three ifs ORing into one value, single store), both through s17/probe.py.
- result: KILLED. r1 scores 34 at 27 insns, r2 scores 30 at 35, against the target's 49. The target performs four lbu and four sb on the flag byte and three bnez selects (andi $v0,$v0,K / bnez / ori $v0,$a0,K / addu $v0,$a0,$zero); no branchless or single-store spelling can produce that. The original source really did write three separate read-modify-write blocks. Banked at rejected/bitcopy-semantic-simplification-score34.c.
- verdict: KILLED

## [s17] Frontier bullet 2 - a second address object derived from a genuinely different program value, i.e. an address that happens to equal &D_80106A73 but is not spelled as a constant.
- mechanism: The ceiling is the number of address PSEUDOS, not the C spelling; a non-constant expression yielding &D_80106A73 would create a second allocno that local-alloc's update_equiv_regs could not substitute away.
- probe: Read the only non-constant value in the function to its definition: p = func_80077D00(), which is 's32* func_80077D00(void) { return &D_8009BD24; }' at src/text1b_b.c:898.
- result: KILLED. p is 0x8009BD24; D_80106A73 - 0x8009BD24 = 0x6AD4F, an arbitrary distance across a segment boundary (.data ends ~0x800A3800, D_80106A73 is bss). No human-written expression relates them, and manufacturing the relationship with an opaque zero is a first-reach lever, explicitly excluded when the bullet was written. The function contains no non-constant expression that yields the flag address.
- verdict: KILLED

## [s18] Step (d) of the ceiling proof -- "one C object is one DECL_RTL is one pseudo" -- is false for AGGREGATE C objects, so a single declared array or struct holding the address can supply the two address pseudos the target needs without a second C pointer object.
- mechanism: The premise had only ever been verified for scalar pointer locals. A C object of aggregate type is not a DECL_RTL pseudo: a multi-slot or address-taken aggregate is frame-resident (a MEM), and every read of a member loads into a FRESH pseudo -- exactly the "two pseudos from one declared object" that the ceiling argument asserts is impossible. Eighteen sessions in, no aggregate address holder had ever been measured, so the load-bearing step of the whole argument had an untested hole.
- probe: Four bodies on the banked floor-10 chassis with the scalar pointer replaced by an aggregate, scored on the honest sandbox with an lbu/sb/lui census and a full disassembly diff against the scalar control: g1_arr1 (`u8 *qa[1];`, one slot, all four accesses), g2_struct1 (`struct { u8 *b; } s;`), g3_arr2 (`u8 *qa[2];` with mask+block1 on [0] and blocks 2/4 on [1]), g4_struct2 (two-member struct, same split). Artifacts: tmp/grind/func_80034F88/s18/{probe.py,probe2.py,variants/,results.json,asm_a0_base.txt,asm_g1_arr1.txt,asm_g2_struct1.txt,asm_g3_arr2.txt}.
- result: KILLED in both directions, and the hole is now closed by measurement rather than assumption. The SINGLE-slot aggregates g1 and g2 score 10 at 49 insns with census 175/164/456 -- identical to the scalar control a0_base -- and their disassembly is BYTE-FOR-BYTE the same instruction stream (both diffs against asm_a0_base.txt are empty). GCC 2.7.2 scalarises a single-slot, address-never-taken aggregate into exactly ONE pseudo, so it is the scalar form spelled longer, not a second base register. The TWO-slot aggregates g3 and g4 do create two address pseudos and pay for them in the frame: the prologue grows from `addiu sp,sp,-24` to `addiu sp,sp,-32`, the first address is spilled with `sw v1,16(sp)` and reloaded per use, and both land at 58 insns / score 35 -- strictly worse than the banned two-SCALAR-object forms (10 and 21) and nowhere near 0. Repaired step (d): two address pseudos require either two scalar C pointer objects (banned, reaches 0) or a two-slot aggregate (58 insns, 35, cannot reach 0). Banked at rejected/aggregate-address-holder-single-slot-score10.c and rejected/aggregate-address-holder-two-slot-score35.c.
- verdict: KILLED

## [s18] The target's own instruction ORDER proves the ORIGINAL SOURCE held two simultaneously-live address objects for D_80106A73, so the remaining gap is a source-model gap and not a search gap.
- mechanism: asm/funcs/func_80034F88.s emits `lui $a0,%hi(D_80106A73)` (80034FC8) and `addiu $a0,$a0,%lo` (80034FCC) IMMEDIATELY BEFORE `sb $v0,0x0($v1)` (80034FD0). The second base is materialised while the first base is still the live base register of a store that has not been issued. One C pointer local is one DECL_RTL pseudo, so re-assigning it emits `(set (reg q) (symbol_ref))` on the SAME pseudo -- a def of the exact register the pending `sb` reads as its base. No GCC pass moves a def of a pseudo above a use of that pseudo, at any optimisation level, because it changes the program. The order is therefore unreachable from any one-object C program under any dataflow-preserving compiler; it is not an allocator artefact and not a scheduling artefact.
- probe: Read the three instructions directly off the target asm, then checked the emitted order of the banked floor-10 single-object candidate in tmp/grind/func_80034F88/s18/asm_a0_base.txt.
- result: CONFIRMED. Our single-object build emits `sb v0,0(a0)` and only THEN `lui a0` / `addiu a0` -- the target's same two instructions in the only order one object permits -- and no source-level dial across eighteen sessions (declaration order, live-range shape, statement order, staging, mask spelling, temporary width, register pins, 6,450+ permuter iterations) has ever flipped it, nor by the argument above can any. This upgrades the ledger's central claim in kind: it is no longer "our reproduction cannot get two base registers from one C object" but "the original source necessarily contained at least two distinct address objects". It is the escalation packet's lead evidence, because it reframes the classification question from "may we add handles to steer the allocator" to "is reconstructing a provably-multi-object original a coercion at all" -- a question a session still may not self-answer (checklist T5, plus the standing 2026-07-27 ruling's citation requirement).
- verdict: CONFIRMED

## [s18] Step (d) of the ceiling proof — 'one C object is one DECL_RTL is one pseudo' — is false for AGGREGATE C objects, so a single declared array or struct holding the address can supply the two address pseudos the target needs without a second C pointer object.
- mechanism: The premise had only ever been verified for scalar pointer locals. A C object of aggregate type is not a DECL_RTL pseudo: a multi-slot or address-taken aggregate is frame-resident (a MEM), and every read of a member loads into a FRESH pseudo — exactly the 'two pseudos from one declared object' the ceiling argument asserts is impossible. Eighteen sessions in, no aggregate address holder had ever been measured, so the load-bearing step of the whole argument had an untested hole.
- probe: Four bodies on the banked floor-10 chassis with the scalar pointer replaced by an aggregate, scored on the honest sandbox with an lbu/sb/lui census and a full disassembly diff against the scalar control a0_base: g1_arr1 (`u8 *qa[1];`, one slot, all four accesses), g2_struct1 (`struct { u8 *b; } s;`), g3_arr2 (`u8 *qa[2];`, mask+block1 on [0] and blocks 2/4 on [1]), g4_struct2 (two-member struct, same split). Harness tmp/grind/func_80034F88/s18/{probe.py,probe2.py}.
- result: KILLED in both directions, and the hole is now closed by measurement instead of assumption. Single-slot aggregates g1 and g2 score 10 at 49 insns with census lbu 175 / sb 164 / lui 456 — identical to the scalar control — and their disassembly is BYTE-FOR-BYTE the same instruction stream (both diffs against asm_a0_base.txt are empty), so GCC 2.7.2 scalarises a single-slot address-never-taken aggregate into exactly ONE pseudo. Two-slot aggregates g3 and g4 do create two address pseudos and pay for them in the frame: prologue grows from `addiu sp,sp,-24` to `addiu sp,sp,-32`, the first address is spilled with `sw v1,16(sp)` and reloaded per use, both landing at 58 insns / score 35 — strictly worse than the banned two-scalar-object forms (10 and 21) and nowhere near 0. Repaired step (d): two address pseudos require either two scalar C pointer objects (banned, reaches 0) or a two-slot aggregate (58 insns, 35, cannot reach 0).
- verdict: KILLED

## [s18] The target's own instruction ORDER proves the ORIGINAL SOURCE held two simultaneously-live address objects for D_80106A73, so the remaining gap is a source-model gap and not a search gap.
- mechanism: asm/funcs/func_80034F88.s emits `lui $a0,%hi(D_80106A73)` (80034FC8) and `addiu $a0,$a0,%lo` (80034FCC) IMMEDIATELY BEFORE `sb $v0,0x0($v1)` (80034FD0): the second base is materialised while the first base is still the live base register of a store that has not been issued. One C pointer local is one DECL_RTL pseudo, so re-assigning it emits `(set (reg q) (symbol_ref))` on the SAME pseudo — a def of the exact register the pending `sb` reads as its base. No GCC pass moves a def of a pseudo above a use of that pseudo, at any optimisation level, because it changes the program. The order is therefore unreachable from any one-object C program under any dataflow-preserving compiler; it is neither an allocator nor a scheduling artefact.
- probe: Read the three instructions directly off the target asm, then checked the emitted order of the banked floor-10 single-object candidate in tmp/grind/func_80034F88/s18/asm_a0_base.txt.
- result: CONFIRMED. Our single-object build emits `sb v0,0(a0)` and only THEN `lui a0` / `addiu a0` — the target's same two instructions in the only order one object permits — and no source-level dial across eighteen sessions (declaration order, live-range shape, statement order, staging, mask spelling, temporary width, register pins, 6,450+ permuter iterations) has ever flipped it, nor by the argument above can any. The ledger's central claim is upgraded in kind: no longer 'our reproduction cannot get two base registers from one C object' but 'the original source necessarily contained at least two distinct address objects'.
- verdict: CONFIRMED

## [s19] Some structural dimension of the score-10 single-object chassis that has never been varied ON THIS CHASSIS (the other pointer's type, the 0xF8 mask store's spelling/staging, scope flattening, declaration order inside block 1, statement order of block 1's two reads) can move block 1's base register from $a0 to $v1 or restore its missing post-store reload.
- mechanism: The residual was localised by s16-s18 to block 1 alone, as a base-register naming difference plus one absent reload, both consequences of there being exactly one address pseudo (global.c:426 gives one allocno per pseudo and 2.7.2 has no live-range splitting). If any of those dimensions perturbs allocno numbering or the live ranges around that pseudo, find_reg could hand block 1 a different hard register even with one object.
- probe: 11 forms through tmp/grind/func_80034F88/s19/{gen.py,gen2.py,probe*.py} — a_p_as_u8ptr (p declared u8 *, conditions as *(s32 *)(p+0x20)&K), d_flat_scope_reuse (v/c at function scope, reused), i_block1_read_first, n1/n2/n3/n7 (the 0xF8 store as m=*q;*q=m&0xF8 / *q=*q&0xF8 / u8-staged / &=~7), n5_q_declared_first, o3_block1_decl_order, o4_block1_flat — each scored with the honest `sandbox func_80034F88 --disable all` and censused with objdump.
- result: ALL TEN land on exactly score 10, 49 build insns, lbu 175 / sb 164 / lui 456 — identical to the base chassis on every counter. The dimensions are exactly codegen-neutral, not merely equal-scoring: the mask store has five spellings and one instruction stream, and the type of the other live pointer is neutral too. The residual is invariant under every source-level structural dimension that does not change how many address objects exist.
- verdict: KILLED

## [s19] Re-associating block 1's condition (a sign test instead of a mask test), narrowing block 1's cond/result local to u8, or re-associating the copy loop's exit test can recover part of block 1's residual.
- mechanism: Statement re-association and type narrowing are listed structural levers; block 1 is the only defective region, and the loop's induction registers are adjacent to block 1's allocnos.
- probe: b_block1_shift_cond (`c = p[8] << 31;`), m_block1_u8_cond (`u8 c;`), o2_loop_ne_cond (`i != 3`); same harness.
- result: 11 / 11 / 13, all at 49 insns with the base census. The shift form emits sll+bgez where target has andi+beq; the u8 local adds a truncation; `i != 3` changes the loop's compare. Every one is a strict regression on an already-exact region.
- verdict: KILLED

## [s19] Hoisting the flag conditions above the flag stores (one condition, or all three) changes block 1's register assignment.
- mechanism: The `p[8]` load between a flag store and the next flag read is what separates them in cse's insn walk; removing it changes the value-table state at block 1.
- probe: n6_cond_above_mask (block 1's condition only) and c_conditions_hoisted (all three, computed before the mask store).
- result: 13 at 45 insns (lbu 174) and 29 at 39 insns (lbu 173). Hoisting removes the separating memory reference, so cse FORWARDS the flag store into the following read and deletes reloads the target has — the census moves AWAY from the target's lbu 176. This is the same store-to-load forwarding mechanism s1-s3 mapped, seen from the other side: the interleaved `p[8]` reads in the base chassis are load-bearing, not incidental.
- verdict: KILLED

## [s19] Giving a value an earlier live range (the pointer object materialised before the call, or the loop index defined at the top) perturbs allocno ordering usefully.
- mechanism: Allocno priority in global.c is driven by live-range length and reference counts; lengthening a range changes the order in which find_reg assigns hard registers.
- probe: n4_q_before_call (`q = &D_80106A73;` before `p = func_80077D00();`) and o1_i_live_early (`i = 0;` at the top with `for (; i < 3; i++)`).
- result: 29 at 51 insns and 24 at 53 insns. Both regress for the obvious structural reason rather than a subtle one: a value live across the jal must take a callee-saved register, which grows the prologue/epilogue with a save/restore pair the target does not have. Lengthening live ranges is not available as an allocation dial on this function.
- verdict: KILLED

## [s19] Some structural dimension of the score-10 single-pointer-object chassis that has never been varied ON THIS CHASSIS -- the type of the other live pointer, the spelling/staging of the 0xF8 mask store, scope flattening, declaration order (between the two pointer objects and inside block 1), or the statement order of block 1's two reads -- can move block 1's base register from $a0 to $v1 or restore its missing post-store reload.
- mechanism: s16-s18 localised the whole 10-point residual to block 1, as a base-register naming difference plus one absent reload, both consequences of there being exactly one address pseudo (tools/gcc-2.7.2/global.c:426 gives one allocno per pseudo and 2.7.2 has no live-range splitting). If any of these dimensions perturbs allocno numbering or the live ranges around that pseudo, find_reg could hand block 1 a different hard register even from one C object.
- probe: 11 forms through tmp/grind/func_80034F88/s19/{gen.py,gen2.py,probe.py,probe2.py,probe3.py}: a_p_as_u8ptr (p declared u8 *, conditions as *(s32 *)(p+0x20)&K, loop as p+i+0x17), d_flat_scope_reuse (v/c at function scope reused by all three blocks), i_block1_read_first, n1/n2/n3/n7 (the 0xF8 store as m=*q;*q=m&0xF8 / *q=*q&0xF8 / u8-staged / &=~7), n5_q_declared_first, o3_block1_decl_order, o4_block1_flat. Each measured with the honest `sandbox func_80034F88 --disable all` and censused with objdump on tmp/sandbox/func_80034F88/code6cac_b.o.
- result: All TEN land on exactly score 10, 49 build insns, lbu 175 / sb 164 / lui 456 -- identical to the base chassis on every counter, so the dimensions are exactly codegen-neutral rather than merely equal-scoring. The 0xF8 mask store has five spellings and one instruction stream; the type of the other live pointer is neutral too.
- verdict: KILLED

## [s19] Re-associating block 1's condition as a sign test, narrowing block 1's cond/result local to u8, or re-associating the copy loop's exit test recovers part of block 1's residual.
- mechanism: Statement re-association and type narrowing are listed structural levers; block 1 is the only defective region and the copy loop's induction registers are adjacent to block 1's allocnos.
- probe: b_block1_shift_cond (c = p[8] << 31), m_block1_u8_cond (u8 c), o2_loop_ne_cond (i != 3); same harness.
- result: 11 / 11 / 13, all at 49 insns with the base census. The shift form emits sll+bgez where the target has andi+beq, the u8 local adds a truncation, and i != 3 changes the loop compare in a region that was already register-identical to the target. Strict regressions.
- verdict: KILLED

## [s19] Hoisting the flag conditions above the flag stores (block 1's alone, or all three above the 0xF8 mask store) changes block 1's register assignment.
- mechanism: The p[8] load sitting between a flag store and the next flag read is what separates them in cse's insn walk; removing it changes the value-table state at block 1.
- probe: n6_cond_above_mask (block 1 only) and c_conditions_hoisted (all three conditions computed before the mask store).
- result: 13 at 45 insns (lbu 174) and 29 at 39 insns (lbu 173). Hoisting removes the separating memory reference so cse FORWARDS the flag store into the next read and DELETES reloads the target has -- the census moves away from the target's lbu 176, not toward it. The interleaved p[8] reads in the base chassis are load-bearing, not incidental.
- verdict: KILLED

## [s19] Giving a value an earlier live range (the pointer object materialised before the call, or the loop index defined at the top of the function) perturbs allocno ordering usefully.
- mechanism: Allocno priority in global.c is driven by live-range length and reference counts, so lengthening a range changes the order in which find_reg hands out hard registers.
- probe: n4_q_before_call (q = &D_80106A73; before p = func_80077D00();) and o1_i_live_early (i = 0; at the top with for (; i < 3; i++)).
- result: 29 at 51 insns and 24 at 53 insns. Both regress for a structural reason, not a subtle one: a value live across the jal must take a callee-saved register, which grows the prologue/epilogue by a save/restore pair the target does not have. Lengthening live ranges is not available as an allocation dial on this function.
- verdict: KILLED

## s20 (structural) — 2026-08-13

### s20-H1 — the missing block-1 reload can be recovered at zero instruction cost from ONE C pointer object. **CONFIRMED**
- mechanism: The target's second `lbu a0,0(v1)` and our maspsx nop occupy the SAME position (the load-delay slot of `lw p[8]`); we lose the load because cse forwards the mask store into block 1's read. cse.c keys its memory table on the address expression's equivalence class and cannot re-associate `(q + 3) - 3` back to `q`, so a pseudo produced by that round trip carries no equivalence to the store's address pseudo, the recorded MEM value is not matched, and the load survives. local-alloc then coalesces the copy chain onto one hard register, so the arithmetic emits nothing.
- probe: 8 forms creating a fresh address pseudo between the mask store and block 1's read (x1 delta-3 true symbol, y1 reversed order, y2 delta 1, y3 inside block 1's scope, y4 after the condition load, y5 two round trips, y8 in every block, w06 with the `&D_80106A70 + 3` spelling), against 2 controls (v01 redundant constant re-assignment, y6 round trip before the mask store).
- result: x1/y1/y2/y3/y4/w06 all score 10 at 49 insns with lbu 176 / sb 164 / lui 456 — the target's exact access census at the target's exact instruction count. Controls: v01 is exactly neutral (10 / 49 / lbu 175, the re-assignment is copy-propagated away) and y6 leaves the census at 175 (the pseudo must be created BETWEEN store and read). y8 over-applies it and drops to 45 insns / score 13.
- verdict: **CONFIRMED** — and it KILLS s17's pricing claim that the reload costs one extra lui. The construct itself is inadmissible (dead pointer arithmetic; fails T1/T2/T3) and was never installed or proposed; it is banked as a diagnostic chassis at rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c.

### s20-H2 — recovering the reload lowers the score. **KILLED**
- mechanism: If the residual were "5 points of register naming plus 5 points of missing memory access" (the inherited s16/s17 decomposition), then restoring the access at zero instruction cost should remove its share of the distance.
- probe: the same 8 forms as s20-H1, scored with the honest sandbox.
- result: every one scores exactly 10, identical to the base. The reload's position differs from target only in which register it names, so restoring it converts one "nop vs lbu" difference into one "lbu $v1 vs lbu $a0" difference — a wash.
- verdict: **KILLED**. The 10-point residual is entirely (a) block 1's $a0/$v1 naming and (b) the second base's lui/addiu sitting after block 1's store rather than before it. This is a strictly sharper statement than the inherited one and it removes the "missing memory access" half of the residual from the search space.

### s20-H3 — the target's lui/addiu-before-store order is reachable by moving the existing re-materialisation statement earlier. **KILLED**
- mechanism: The base chassis already contains `q = &D_80106A73;` at the top of blocks 2 and 4. Moving one of them to just before block 1's store is pure statement order on an existing statement (no new construct), and would put the pair before the `sb` exactly as the target has it.
- probe: x4 (hoisted, block 2 then inherits), x5 (hoisted, block 2 keeps its own as well), x6 (hoisted before every store).
- result: 21 / 51 insns, 21 / 51, and 25 / 48 insns with lbu 173. All far worse than 10.
- verdict: **KILLED**. If the store goes through the NEW pseudo, the old pseudo dies before it and the two share a hard register, so nothing is gained; cse then forwards that store into the following block's read and deletes two further reloads. The target's order needs the store to use the OLD value while the NEW base is already materialised — two simultaneously live address values. This reproduces s18's instruction-ordering proof from the source side.

### s20-H4 — the $a0/$v1 dial moves on a chassis with extra address pseudos. **KILLED**
- mechanism: The round-trip forms introduce two or four extra pseudos before block 1, changing pseudo numbering, LUIDs and therefore allocno priority order — the plausible route to flipping which of $a0/$v1 the base lands in, after s16 killed the dial three ways on the plain chassis.
- probe: the 8 reload-recovering forms plus y5 (two round trips) and y7 (round trip with the block re-materialisations removed).
- result: all eight score exactly 10 with the same base/value register assignment; y7 collapses to 32 / 50.
- verdict: **KILLED** — a fourth independent kill of the dial.

### s20-H5 — a single addend-spelled materialisation carries the s17 LO16 phantom distance. **KILLED**
- mechanism: s17 measured a form spelling every address `&D_80106A70 + 3` as scoring 2 points above its byte truth, because engine/score.py does not mask R_MIPS_LO16 addends.
- probe: x2 — the base chassis with ONLY the first materialisation spelled `&D_80106A70 + 3`.
- result: 10 / 49 / lbu 175, indistinguishable from the base.
- verdict: **KILLED** as stated. The artefact is per-materialisation, not per-form; w06's 10 is therefore an honest 10 and no hidden floor drop is being masked.

### s20-H6 — remaining block-1 spelling dimensions (signed read, arm duplication, single-local compound-or, inline condition, mask folded into the arms, flag-word staging, displaced 0x70-based addressing). **KILLED**
- mechanism: The last untried block-1 shapes from the codegen-technique-index that do not change how many address objects exist.
- probe: v02/v03 (signed byte read, one block and all three), v06 (mask folded into block 1's arms), v07 (store duplicated into arms), v08 (single local, no else), v09 (inline condition), v10/v11 (flag-word staging, per-block and once), v04/v05/w01/w02/w03/w08 (0x70-based object with displacement 3, six combinations), w04/w05 (neighbour-symbol re-assignment; mask on the symbol), w07 (`q[0]` spelling).
- result: 12 to 32; the only ties at 10 are the ones that change nothing (w07's `q[0]` is the same rtx; x3's pre-mask walk is folded away).
- verdict: **KILLED**. Combined with s19's eighteen forms, no source-level dimension around a single address pseudo moves the score in either direction except by deleting reloads the target has.

## [s20] The missing block-1 reload can be recovered at zero instruction cost from a SINGLE C pointer object.
- mechanism: The target's second `lbu a0,0(v1)` and our maspsx nop occupy the same position -- the load-delay slot of `lw p[8]`; we lose the load because cse forwards the mask store into block 1's read. cse.c keys its memory table on the address expression's equivalence class and cannot re-associate `(q + 3) - 3` back to `q`, so a pseudo produced by that round trip carries no equivalence to the store's address pseudo, the recorded MEM value is not matched, and the load survives. local-alloc then coalesces the copy chain onto one hard register, so the arithmetic itself emits nothing.
- probe: Eight forms creating a fresh address pseudo between the mask store and block 1's read (x1 delta-3 with the true symbol, y1 reversed order, y2 delta 1, y3 inside block 1's scope, y4 after the condition load, y5 two round trips, y8 in every block, w06 with the &D_80106A70+3 spelling), plus two controls: v01 (redundant constant re-assignment in the same place) and y6 (the same round trip placed before the mask store).
- result: x1/y1/y2/y3/y4/w06 all score 10 at 49 insns with lbu 176 / sb 164 / lui 456 -- the target's exact access census at the target's exact instruction count, with the true &D_80106A73 spelling and therefore no LO16-addend artefact. Controls: v01 is exactly neutral (10 / 49 / lbu 175 -- the re-assignment is copy-propagated onto the first pseudo), y6 leaves the census at lbu 175 (the fresh pseudo must be created BETWEEN the store and the read), y8 over-applies it and collapses to 45 insns / score 13.
- verdict: CONFIRMED

## [s20] Recovering that reload lowers the score (i.e. the residual really is 'register naming PLUS one missing memory access', the inherited s16/s17 decomposition).
- mechanism: If the missing access carried its own share of the distance, restoring it at zero instruction cost should remove that share.
- probe: The same eight reload-recovering forms, scored with the honest sandbox --disable all.
- result: Every one scores exactly 10, identical to the base. Restoring the load converts one 'nop vs lbu' difference into one 'lbu $v1 vs lbu $a0' difference -- a wash, because block 1's registers are swapped relative to target.
- verdict: KILLED

## [s20] The target's lui/addiu-before-store order in block 1 is reachable by moving the chassis's EXISTING re-materialisation statement earlier (pure statement order, no new construct).
- mechanism: candidate.c already contains `q = &D_80106A73;` at the top of blocks 2 and 4; hoisting one to just before block 1's store would place the pair before the `sb` exactly as target has it.
- probe: x4 (hoisted, block 2 inherits), x5 (hoisted, block 2 keeps its own too), x6 (hoisted before every store).
- result: 21 at 51 insns, 21 at 51, and 25 at 48 insns with lbu 173 -- all far worse than 10. The store then goes through the NEW pseudo, so the old pseudo dies before it and the two share a hard register (nothing gained), and cse forwards that store into the following block's read, deleting two further reloads.
- verdict: KILLED

## [s20] The $a0/$v1 dial in block 1 moves on a chassis carrying extra address pseudos.
- mechanism: The round-trip forms introduce two or four extra pseudos before block 1, changing pseudo numbering, LUIDs and hence allocno priority order -- the plausible route to flipping the base register after s16 killed the dial three ways on the plain chassis.
- probe: The eight reload-recovering forms plus y5 (two round trips) and y7 (round trip with the block re-materialisations removed).
- result: All eight score exactly 10 with the same base/value register assignment; y7 collapses to 32 / 50 insns.
- verdict: KILLED

## [s20] A single addend-spelled address materialisation carries s17's +2 R_MIPS_LO16 phantom distance.
- mechanism: engine/score.py does not mask R_MIPS_LO16 addends, so s17 measured a body spelling every address `&D_80106A70 + 3` as scoring 2 above its byte truth.
- probe: x2 -- the base chassis with ONLY the first materialisation spelled `&D_80106A70 + 3`.
- result: 10 / 49 / lbu 175, indistinguishable from the base.
- verdict: KILLED

## [s20] One of the remaining untried block-1 spelling dimensions moves the score.
- mechanism: The last codegen-technique-index shapes that do not change how many address objects exist: signed byte read, store duplicated into arms, single local with compound-or and no else, inline condition test, mask folded into the arms, flag-word staging, displaced addressing off a D_80106A70 base, neighbour-symbol re-assignment, mask applied to the symbol.
- probe: v02, v03, v06, v07, v08, v09, v10, v11, v04, v05, w01, w02, w03, w04, w05, w07, w08 (17 forms).
- result: 12 to 32. The only ties at 10 are the forms that change nothing at all (w07's `q[0]` is the same rtx as `*q`; x3's pre-mask walk is folded away).
- verdict: KILLED


## Resolved in s21 (permuter)

## [s21] A decomp-permuter campaign seeded at the HONEST FLOOR — the score-10 single-object chassis, which no prior campaign had ever seeded — can find a sub-10 admissible form, because the four previous permuter kills were all seeded above the floor (s4 at 18, s5 directed at 18, s13 at 13) and are therefore seed-quality kills rather than a statement about the floor's own neighbourhood.
- mechanism: decomp-permuter randomises C-level structure (statement order, temporaries, expression shape, declaration placement) and rescores against target.o. Seeding it at the floor makes the neighbourhood it samples the floor's own, so any structurally adjacent improvement the nineteen hand sessions missed would surface as a find that the honest sandbox re-scores below 10.
- probe: Campaign 1, tmp/grind/func_80034F88/s21/ws, label s21-floor10-single-object, -j 8, launched/waited/harvested with tools/permuter_campaign.py (wait in-turn, harvest --stop before session end). 44,626 iterations, permuter metric 455 -> 340, six finds, all re-scored with `sandbox func_80034F88 --disable all` and admissibility-censused by s21/eval.py (eval_seed1.json). Fresh-seed window closed: no novel find for 546s after 38,015 iterations.
- result: KILLED. Every find is 10 or worse on the honest sandbox: 10 (a re-spelling of the seed), 12, 13, 13, 14, 21. The floor chassis is a local minimum of the random-structural neighbourhood, not only of hand enumeration. Fifth permuter kill on this function, and the first one that is a statement about the floor rather than about the seed. It also reconfirms s4-H1 at maximum contrast: the permuter's best metric find (340) is sandbox 14 and its worst (455) is sandbox 10 — the metrics are anti-correlated here, so --stop-on-zero can never fire and campaign progress cannot be read off permuter numbers.
- verdict: KILLED

## [s21] The e1 chassis (direct-symbol block-1 read, score 11 at 50 insns, the target's exact lbu 176 census) sits in a different basin from the floor chassis and a campaign seeded there can reach an ADMISSIBLE sub-10 form.
- mechanism: e1 already buys the target's access census; s17 priced its missing reload at one extra lui. If the lui is an artefact of the spelling rather than of the compiler charging one address materialisation per access, a structural permutation of e1 should drop it and land under 10 with one address object.
- probe: Campaign 2, tmp/grind/func_80034F88/s21/ws2, label s21-e1-direct-symbol-b1read, -j 6, same launch/wait/harvest protocol. 37,153 iterations, permuter metric 365 -> 255, six finds, re-scored and admissibility-censused (eval_seed2.json). Two finds scored 8; both were read line-by-line and disassembled against the target with s21/sbs.sh.
- result: KILLED as stated, with a substantive by-product. Every SINGLE-OBJECT find on this seed scored 13 or 14 — worse than the floor — so no admissible sub-10 form exists in e1's neighbourhood either. Both sub-10 finds are the BANNED second-address-object construct: output-255-1 adds `new_var = q;` with `*new_var = c;` (a pointer COPY, the spelling the Judge constraint names explicitly, and s11's hand-found reload lever rediscovered independently), and output-355-1 does the same AND deletes block 1's `& 1`, which makes it a miscompile. Both banked under rejected/permuter-s21-*.
- verdict: KILLED

## [s21] The banned second-address-object family, if it were ever sanctioned, would close this function.
- mechanism: nineteen sessions localised the residual to the target holding two simultaneously live address values for D_80106A73, and a three-object body did reach sandbox 0 in s12. The natural inference is that the ban is the only thing between the function and a match, i.e. that any two-object form is on the match's doorstep.
- probe: The cheapest legal two-object form the search produced — e1 plus one pointer copy (output-255-1, semantically correct) — was disassembled instruction-for-instruction against asm/funcs/func_80034F88.s with s21/sbs.sh, and priced against its own seed.
- result: KILLED as an inference. The copy is worth exactly 3 points at +1 insn (e1's 11/50 -> 8/51) and stops at 8, not 0. Its residual is still the whole of block 1: the copy gives a second pseudo whose VALUE COMES FROM THE FIRST, so the stream carries a `move v1,a0` plus a fresh `lui` for the read, where the target materialises its second base from the symbol BEFORE block 1's store and has neither. Only the THREE-object body reached 0. So "two C objects" is not the missing ingredient by itself; what the target has is two INDEPENDENTLY MATERIALISED, simultaneously live address values, and the source-model gap is that specific, not merely a cardinality gap. This is the strongest form of the ceiling statement the function has produced, and it now rests on random search approaching the ceiling from both sides.
- verdict: KILLED

## [s21] A decomp-permuter campaign seeded at the HONEST FLOOR -- the score-10 single-object chassis, which no prior campaign had ever seeded -- can find a sub-10 admissible form, because the four previous permuter kills were all seeded above the floor (s4 at 18, s5 directed at 18, s13 at 13) and are therefore seed-quality kills rather than statements about the floor's own neighbourhood.
- mechanism: decomp-permuter randomises C-level structure (statement order, temporaries, expression shape, declaration placement) and rescores against target.o. Seeding it at the floor makes the sampled neighbourhood the floor's own, so any structurally adjacent improvement nineteen hand sessions missed would surface as a find the honest sandbox re-scores below 10.
- probe: Campaign 1, tmp/grind/func_80034F88/s21/ws, label s21-floor10-single-object, -j 8, launched/waited/harvested with tools/permuter_campaign.py (wait in-turn, harvest --stop before session end). 44,626 iterations, permuter metric 455 -> 340, six finds, all re-scored with `sandbox func_80034F88 --disable all` and admissibility-censused by s21/eval.py (eval_seed1.json). Fresh-seed window closed cleanly: no novel find for 546s after 38,015 iterations.
- result: Every find is 10 or worse on the honest sandbox: 10 (a re-spelling of the seed), 12, 13, 13, 14, 21. The floor chassis is a local minimum of the random-structural neighbourhood, not only of hand enumeration -- the fifth permuter kill on this function and the first that is a statement about the floor rather than about the seed. It also reconfirms s4-H1 at maximum contrast: the permuter's BEST metric find (340) is sandbox 14 and its WORST (455) is sandbox 10, so the two metrics are anti-correlated here, --stop-on-zero can never fire, and campaign progress cannot be read off permuter numbers.
- verdict: KILLED

## [s21] The e1 chassis (direct-symbol block-1 read, score 11 at 50 insns, carrying the target's exact lbu 176 access census) sits in a different basin from the floor chassis, and a campaign seeded there can reach an ADMISSIBLE sub-10 form.
- mechanism: e1 already buys the target's access census, and s17 priced its missing reload at one extra lui. If that lui is an artefact of the spelling rather than of the compiler charging one address materialisation per access, a structural permutation of e1 should drop it and land under 10 with a single address object.
- probe: Campaign 2, tmp/grind/func_80034F88/s21/ws2, label s21-e1-direct-symbol-b1read, -j 6, same launch/wait/harvest protocol. 37,153 iterations, permuter metric 365 -> 255, six finds, re-scored and admissibility-censused (eval_seed2.json). Both sub-10 finds were read line-by-line for semantics and disassembled against asm/funcs/func_80034F88.s with s21/sbs.sh.
- result: Every SINGLE-OBJECT find on this seed scored 13 or 14 -- worse than the floor -- so no admissible sub-10 form exists in e1's neighbourhood either. Both sub-10 finds are the BANNED second-address-object construct: output-255-1 adds `new_var = q;` with `*new_var = c;` (a pointer COPY, the exact spelling the Judge's constraint names, and s11's hand-found reload lever rediscovered independently by random search), and output-355-1 does the same AND deletes block 1's `& 1`, making it a miscompile. Both banked under memory/grind/func_80034F88/rejected/permuter-s21-*.
- verdict: KILLED

## [s21] The banned second-address-object family, if it were ever sanctioned, would close this function -- i.e. the ban is the only thing standing between the function and a byte match.
- mechanism: Nineteen sessions localised the residual to the target holding two simultaneously live address values for D_80106A73, and s12's three-object body did reach sandbox 0. The natural inference is that any two-object form is on the match's doorstep and the gap is one of object CARDINALITY.
- probe: The cheapest legal two-object form the search produced -- e1 plus one pointer copy (output-255-1, semantically correct, sandbox 8 at 51 insns, lbu 176 / sb 164) -- was priced against its own seed and disassembled instruction-for-instruction against the target with tmp/grind/func_80034F88/s21/sbs.sh.
- result: KILLED as an inference. The copy is worth exactly 3 points at +1 instruction (e1's 11/50 -> 8/51) and stops at 8, not 0. Its entire residual is still block 1: the copy yields a second pseudo whose VALUE COMES FROM THE FIRST, so the stream carries `move v1,a0` plus a fresh `lui` for the read, where the target materialises its second base from the symbol itself BEFORE block 1's store and has neither instruction. Only the THREE-object body ever reached 0. So the source-model gap is not cardinality: what the target has is two INDEPENDENTLY MATERIALISED, simultaneously live address values. The ceiling statement now rests on random search approaching it from both sides -- from above (one object cannot get under 10 in 44,626 iterations) and from below (two objects stop at 8 in 37,153).
- verdict: KILLED


## Resolved in s22 (escalation / disposition)

## [s22] The canonical-asm gate (endgame-lock gate 1) can pass for func_80034F88 — i.e. the original was hand-written assembly, which would authorize an inline-asm finish.
- mechanism: tools/scan_hand_coded.py scores eight structural signatures of hand-written PSX asm; the STRONG tier (S1 multu pacing, S2 empty-body branch, S6 BIOS jumptable class) is the bar .claude/rules/endgame-lock-disposition.md sets for canonical-asm authorization.
- probe: `python3 tools/scan_hand_coded.py --single func_80034F88`, run for the first time in this function's 22-session history (the 2026-08-13 16:36 Judge ruling flagged the gate as unevaluated). Output banked at tmp/grind/func_80034F88/s22/scan_hand_coded.txt.
- result: tier=LOW, score=0/8, "no strong hand-coded indicators" — all eight boxes unchecked, including every STRONG signal. The function is 49 instructions of ordinary compiler output whose residual is a register-allocation tie-break, which the rule names explicitly as the shape that must NEVER be papered over with asm.
- verdict: KILLED (gate 1 FAILS)

## [s22] The SOTN-precedent gate (endgame-lock gate 2) can pass for the closing construct — SOTN master ships functions where two or more redundant, simultaneously live C handles on ONE global address are load-bearing for the match.
- mechanism: The only measured route to sandbox 0 on this function is multiple C pointer objects all holding &D_80106A73 (three objects reached 0; two reach only 8). Under the 2026-07-27 ruling such a coercion/spelling family is sanctionable ONLY on an in-hand, citable SOTN-master (or VS/ESA) precedent — "same spirit" and "only lever left" do not qualify.
- probe: First-hand census of the local SOTN master checkout at C:\Users\Trenton\Desktop\sotn-decomp, commit db41b28eee52969244a52cc269c8163d1ed8826a (2026-07-01), 1,675 src/**/*.c files, two scripted passes plus a targeted annotation grep. Scripts and outputs: tmp/grind/func_80034F88/s22/sotn_census.py|.txt (permissive, symbol-level) and sotn_census2.py|.txt (strict, identical-address-expression), sotn_checkout.txt.
- result: NEGATIVE. Permissive pass: 65 functions, essentially all distinct sub-objects (different array elements / struct members) = ordinary logic. Strict pass: 21 functions, every one disqualified — non-matching ports (src/*_psp/, src/pc/, src/saturn/), global-to-global stores, or two genuinely different walkers starting at one node (the best-looking hit, ric/maria pl_blueprints `prim2 = prim1 = &g_PrimBuf[self->primIndex];`, advances prim1 by 16 nodes and then writes different vertices of different primitives through each pointer). The fake/match-annotated address-taking lines in the matched PSX trees are the FakePrim/VertexFake TYPE-PUNNING family, one handle cast to another struct type, not a second handle. The sanctioned pointer-alias-fake-exception's own evidence (`tilemap = &g_Tilemap; // n.b.! unused, required for PSP`) is a SINGLE UNUSED alias, the opposite shape.
- verdict: KILLED (gate 2 FAILS — and per the standing ruling a negative census is a failed gate, not an open question)

## [s22] The tail copy loop's own base pointer is an un-tried, ORDINARY-LOGIC route to a second address pseudo — `u8 *r = &D_80106A70;` owned by the loop (which needs that base anyway) and shared with block 1 via `r[3]`.
- mechanism: A second address pseudo is what the target's two simultaneously live base registers require, and a loop-owned base is the one spelling a human would write for reasons unrelated to codegen, so it would not be a lever-shaped construct.
- probe: Ledger lookup before spending a measurement — the rejected bank already contains this exact form.
- result: Already measured dead at 22: memory/grind/func_80034F88/rejected/base70-disp3-loop-shares-object-score22.c. Sharing the loop's D_80106A70 base with block 1 costs 12 points against the floor.
- verdict: KILLED (previously; re-confirmed as not-un-tried)

## [s22] The canonical-asm gate (endgame-lock gate 1) can pass for func_80034F88 — i.e. the original was hand-written assembly, which would authorize an inline-asm finish.
- mechanism: tools/scan_hand_coded.py scores eight structural signatures of hand-written PSX asm; the STRONG tier (S1 multu pacing, S2 empty-body branch, S6 BIOS jumptable class) is the bar .claude/rules/endgame-lock-disposition.md sets for canonical-asm authorization.
- probe: python3 tools/scan_hand_coded.py --single func_80034F88, run for the first time in this function's 22-session history (the 2026-08-13 16:36 Judge ruling flagged the gate as unevaluated). Output banked at tmp/grind/func_80034F88/s22/scan_hand_coded.txt.
- result: tier=LOW, score=0/8, 'no strong hand-coded indicators' — all eight boxes unchecked, including every STRONG signal. The residual is a register-allocation tie-break, which the rule names explicitly as the shape that must never be papered over with asm.
- verdict: KILLED

## [s22] The SOTN-precedent gate (endgame-lock gate 2) can pass for the closing construct — SOTN master ships functions where two or more redundant, simultaneously live C handles on ONE global address are load-bearing for the match.
- mechanism: The only measured route to sandbox 0 here is multiple C pointer objects all holding &D_80106A73 (three objects reached 0; two reach only 8). Under the 2026-07-27 ruling such a coercion/spelling family is sanctionable ONLY on an in-hand, citable SOTN-master (or VS/ESA) precedent.
- probe: First-hand census of the local SOTN master checkout at C:\Users\Trenton\Desktop\sotn-decomp, commit db41b28eee52969244a52cc269c8163d1ed8826a (2026-07-01), 1,675 src/**/*.c files: a permissive symbol-level pass (s22/sotn_census.py), a strict identical-address-expression pass (s22/sotn_census2.py), and a targeted grep for fake/match/required-annotated address-taking lines in the matched PSX trees.
- result: NEGATIVE. Permissive: 65 functions, essentially all distinct sub-objects (different array elements / struct members) = ordinary logic. Strict: only 21 functions, every one disqualified — non-matching ports (src/*_psp/, src/pc/, src/saturn/), global-to-global stores, or two genuinely different walkers that merely start at the same node (ric/maria pl_blueprints 'prim2 = prim1 = &g_PrimBuf[self->primIndex];' with prim1 then advanced 16 nodes, both pointers writing different vertices of different primitives). The annotated hits are the FakePrim/VertexFake TYPE-PUNNING family (one handle cast to another struct type), not second handles. The sanctioned pointer-alias-fake-exception's own SOTN evidence ('tilemap = &g_Tilemap; // n.b.! unused, required for PSP') is a SINGLE UNUSED alias — the opposite shape.
- verdict: KILLED

## [s22] The tail copy loop's own base pointer is an un-tried, ORDINARY-LOGIC route to a second address pseudo — 'u8 *r = &D_80106A70;' owned by the loop (which needs that base anyway) and shared with block 1 via r[3].
- mechanism: A second address pseudo is what the target's two simultaneously live base registers require, and a loop-owned base is the one spelling a human would write for reasons unrelated to codegen, so it would not be a lever-shaped construct.
- probe: Ledger lookup before spending a measurement — the rejected bank already contains this exact form.
- result: Already measured dead at 22: memory/grind/func_80034F88/rejected/base70-disp3-loop-shares-object-score22.c. Sharing the loop's D_80106A70 base with block 1 costs 12 points against the floor. No other un-tried lever was derivable from the ledger.
- verdict: KILLED

==== s23 (escalation / owner-directive: SOLVER) ====

H-s23-1 — "The owner's 2026-08-24 directive is un-executed, and the ra_solver
suite can type the $a0-vs-$v1 residual that 22 sessions attacked by spelling."
  Mechanism: `extract.py` reads the instrumented cc1's ALLOCDBG/FINDREGDBG
  stream and the `-da` .greg/.flow dumps into a model of global.c's inputs;
  `simulate.py` replays allocno_compare + find_reg; `inverse.py` enumerates the
  minimal input perturbations that would reach a stated goal assignment.
  Probe: extract + simulate + `inverse.py global --swap 74,73` and
  `--goal '{"74": 3}'`, on the floor-10 chassis and on s20's census-exact
  diagnostic chassis. Output banked at tmp/grind/func_80034F88/s23/.
  Result: model validates 9/9 dispositions + sort order MATCH; BOTH inverse
  queries return FORECLOSED with an explicit non-emittable-atom list.
  Verdict: CONFIRMED (the directive was executable and produced a typed verdict).

H-s23-2 — "The block-1 base/value naming is an RA tie-break reachable by some
C-level perturbation of refs / live span / birth order / conflicts /
preferences / calls-crossed."
  Mechanism: those six input classes are the complete set global.c consumes and
  the complete set a C spelling can move; if the goal is reachable at all it is
  reachable by a vector over them.
  Probe: `inverse.py global` over an atom space of 218 single perturbations,
  depth 2, on the exact validated model.
  Result: FORECLOSED both as a swap and as a one-sided goal. Root cause named by
  the tool: `$v1` and `$a0` never appear as hard registers in this function's
  pre-RA RTL, so `global.c set_preference` cannot record a preference for either
  — the preference lever does not exist here.
  Verdict: KILLED. This retires the residual's description as a "tie-break": it
  is not a tie the allocator could have broken the other way.

H-s23-3 — "s20's census-exact chassis (`q = q + 3; q = q - 3;`) creates the
second address pseudo that the target's two base registers require, and only
the register naming then stands in the way."
  Mechanism: dead pointer round-trip forces a fresh address materialisation
  between the mask store and block 1's read; if that fresh rtx survives to
  lreg/greg as its own pseudo, the model gains a second address allocno and the
  target's two-base assignment becomes expressible.
  Probe: install the diagnostic body, `extract.py` it, compare allocno counts.
  Result: 9 allocnos, unchanged; the address object is still the single pseudo
  74, with nrefs 10 -> 15 and livelen 29 -> 30. The recovered `lbu` is extra REFS
  on one allocno, not a second allocno. `--goal '{"74": 3}'` on that model is
  FORECLOSED too.
  Verdict: KILLED — and this is the strongest form yet of the ceiling statement,
  because it is measured in the allocator's own input file rather than inferred
  from the emitted stream: no admissible single-object body of this function
  produces two address allocnos, and one allocno is one hard register for its
  whole live range (tools/gcc-2.7.2/global.c:426, no live-range splitting).

H-s23-4 — "One of the solver's three named unmodelled mechanisms (local-alloc
suggested-register pass / qty_size DImode / reload spill-retry) is live here and
would reopen the search."
  Mechanism: each is a place where the real allocator consults inputs the model
  does not dump, so a FORECLOSED verdict is conditional on them being inert.
  Probe: inspect the extracted model + dumps against each mechanism's precondition.
  Result: qty_size/DImode — no DImode quantity in this function (all 9 allocnos
  mode=SI); reload spill-retry — the function has one spill (the `$ra` save) and
  simulate.py matches 9/9 pre-reload, so no retry block governs the address
  pseudo. The local-alloc SUGGESTED-REGISTER pass is NOT excluded by any
  measurement: its inputs (`qty_phys_copy_sugg` / `qty_phys_sugg`) are
  reported-not-scored because `block_alloc`'s hook does not print them.
  Verdict: two of three KILLED, one OPEN — and the open one is a TOOLING gap in
  `tools/gcc-2.7.2/local-alloc.c` + `tools/ra_solver/local_extract.py`, a surface
  a grind session may not edit. It is the function's only live re-attempt route.

## [s23] The owner's 2026-08-24 queue directive ('solver modality recommended before deep re-grind of RA/scheduler-tiebreak residuals') is executable on this function and can type the $a0-vs-$v1 residual that 22 sessions attacked by spelling.
- mechanism: tools/ra_solver/extract.py reads the instrumented cc1's ALLOCDBG/FINDREGDBG stream plus the -da .greg/.flow dumps into a model of global.c's allocation inputs; simulate.py replays allocno_compare + find_reg; inverse.py enumerates the minimal input perturbations that would reach a stated goal assignment and reports a first-class NEGATIVE when none exists.
- probe: extract.py func_80034F88 code6cac_b; simulate.py; inverse.py global --swap 74,73 and --goal {"74": 3}, on the floor-10 chassis and on s20's census-exact diagnostic chassis. Full console output banked at tmp/grind/func_80034F88/s23/ra_solver_report.txt.
- result: Model validates: 9 allocnos, sort order MATCH, dispositions 9/9 against ground truth, allocation {72:$a1, 73:$v1, 74:$a0, 77:$v1, 78:$v0, 81:$v1, 82:$v0, 85:$v1, 86:$v0}, pseudo 74 = the &D_80106A73 address object (nrefs 10, livelen 29, pri 10344).
- verdict: CONFIRMED

## [s23] Block 1's base/value naming ($v1 base + $a0 value in target, the reverse in our build) is an RA tie-break reachable by some C-level perturbation of refs / live span / birth order / conflicts / preferences / calls-crossed.
- mechanism: Those input classes are the complete set global.c consumes and the complete set a C spelling can move; if the goal assignment is reachable at all it is reachable by a vector over them.
- probe: inverse.py global on the validated model, atom space 218 single perturbations over 8 classes, depth 2 — run both as a swap (--swap 74,73) and as a one-sided goal (--goal {"74": 3}).
- result: FORECLOSED both ways: 'no perturbation of any modelled input, up to depth 2, reaches the target assignment', with 18 (swap) / 8 (goal) preference atoms reported mechanically non-emittable. Root cause named by the tool: $v1 and $a0 never appear as hard registers in this function's pre-RA RTL, so global.c set_preference can never record a preference for either, for any pseudo — this function has no call-argument setup and no hard-reg-returning idiom to put them there.
- verdict: KILLED

## [s23] s20's census-exact diagnostic chassis (q = q + 3; q = q - 3;, which reproduces the target's exact lbu 176 / sb 164 / lui 456 census at 49 insns) creates the second address pseudo that target's two live base registers require, leaving only the register naming in the way.
- mechanism: A dead pointer round-trip forces a fresh address materialisation between the mask store and block 1's read; if that rtx survives to lreg/greg as its own pseudo, the model gains a second address allocno and target's two-base assignment becomes expressible as a goal at all.
- probe: Install the diagnostic body temporarily, extract.py it, compare allocno counts and per-allocno stats against the floor-10 model; then inverse.py global --goal {"74": 3} on the new model.
- result: It does not. Still exactly 9 allocnos; the address object is still the single pseudo 74, merely nrefs 10 -> 15 and livelen 29 -> 30 — the recovered lbu is extra REFS on one allocno, not a second allocno. The goal is FORECLOSED on that model too. So on the one chassis whose instruction multiset matches the target, the residual is purely RA and RA is closed.
- verdict: KILLED

## [s23] One of the three mechanisms the solver names as outside its model (local-alloc's suggested-register pass, qty_size for DImode, reload's spill-retry) is live here and would reopen the search.
- mechanism: Each is a place where the real allocator consults inputs the model does not dump, so a FORECLOSED verdict is conditional on all three being inert for this function.
- probe: Check each mechanism's precondition against the extracted model and dumps.
- result: qty_size/DImode is excluded — all 9 allocnos are mode SI. Reload spill-retry is excluded — the function has one spill (the $ra save) and simulate.py matches 9/9 pre-reload, so no retry block governs the address pseudo. The local-alloc SUGGESTED-REGISTER pass (qty_phys_copy_sugg / qty_phys_sugg) is NOT excluded: block_alloc's hook does not print the suggestion sets, so those rows are reported-not-scored. It is the function's only live re-attempt route, and closing it means editing tools/gcc-2.7.2/local-alloc.c + tools/ra_solver/local_extract.py — outside a grind session's surface.
- verdict: CONFIRMED

## s24 (escalation — owner ruling 1 executed)

### H-s24.1 — KILLED. "Local-alloc's suggested-register pass is what seats the
`&D_80106A73` address object, so a C form that creates a hard-reg copy could
move it."
This was the function's ONLY live frontier item after s23 and the subject of the
owner's 2026-08-30 ruling 1 (instrumentation GRANTED). The instrument exists on
main since commit 70d6c905 (BB2_SUGG_DEBUG + `local_extract.py --suggest`,
model validated 1578/1578 preference and 947/947 assignment across all 32 TUs).
PROBE: `python3 tools/ra_solver/local_extract.py code6cac_b --func func_80034F88
--suggest`, then `inverse.py local --sugg --goal {"74": 3}` on all four blocks.
RESULT: all seven local-alloc quantities carry ncopysugg=0 / nsugg=0 / empty
suggestion sets; zero QTYDBG-SUGG rows exist; all seven find_free_reg calls have
`used == first_used`, which is the identity restriction (local-alloc.c:2205-2213
is the only place the sets are consumed). MECHANISM: combine_regs
(local-alloc.c:1859-1899) records a suggestion only when one side of a tieable
copy is a HARD register, and s23 measured that $v1/$a0 never appear as hard regs
in this function's pre-RA RTL — so the suggestion pass is dead for exactly the
same structural reason global.c set_preference is dead. It was never an
independent second chance. VERDICT: KILLED.

### H-s24.2 — CONFIRMED (structural, stronger than H-s24.1). "The contested
address object is not a local-alloc object at all."
Local-alloc's pseudo set for this function is {76,79,83,87,89,91,93}; s23's
global allocno set is {72,73,74,77,78,81,82,85,86}. DISJOINT. Pseudo 74 (the
`&D_80106A73` address object) is a GLOBAL allocno, so local_alloc never assigned
it and no local-alloc pass — suggested-register or main — is in its causal
chain. This is the same shape Phase 7 found for DispPracticeMenuTex_A. VERDICT:
CONFIRMED; it makes H-s24.1's kill unconditional rather than chassis-relative.

### H-s24.3 — KILLED. "qty_size (the other named Phase-5 hook gap) mis-prices a
DImode quantity here and hides an order lever."
The now-dumped `size` column is 1 for all seven quantities. No DImode quantity
exists, independently reconfirming s23's "all 9 allocnos are mode SI". KILLED.

### Net effect on the function's status
s23's FORECLOSED verdict was explicitly conditional on the suggested-register
pass. That condition is DISCHARGED: the RA residual is now FORECLOSED
UNCONDITIONALLY, and the single decidable question raised by the 2026-08-26
decisions.md entry is answered in the negative. Both endgame-lock gates were
re-run and both still FAIL (scan_hand_coded tier LOW 0/8; SOTN precedent census
NEGATIVE, now from two independent sources). No frontier item remains inside the
sanctioned space: the only route to the target's two simultaneously live base
registers is a second C object aliasing D_80106A73, banned for this function on
2026-08-13, with no SOTN-master precedent.

## [s24] Local-alloc's SUGGESTED-REGISTER pass (qty_phys_copy_sugg / qty_phys_sugg) is what seats the &D_80106A73 address object, so a C form creating a hard-reg copy could move it. This was the function's only live frontier item and the subject of the owner's 2026-08-30 ruling 1 (instrumentation GRANTED).
- mechanism: block_alloc runs a suggested-register pass before the main ascending scan; its inputs were not printed by the old BB2_QTY_DEBUG hook, so ra_solver reported those rows rather than scoring them, and every FORECLOSED verdict was conditional on that pass being inert.
- probe: The granted instrument now exists on main (commit 70d6c905, Phase 7: BB2_SUGG_DEBUG in tools/gcc-2.7.2/local-alloc.c + local_extract.py --suggest, model validated 1578/1578 preference and 947/947 assignment over all 32 TUs; run for camera_set_zoom and DispPracticeMenuTex_A but never for this function). With candidate.c installed (sandbox re-measured at 10, 49/49, rules_dropped 0), ran `python3 tools/ra_solver/local_extract.py code6cac_b --func func_80034F88 --suggest` and `inverse.py local --sugg --goal {"74": 3}` on all four blocks.
- result: func_80034F88 has 7 local-alloc quantities, ALL main-pass rows; zero QTYDBG-SUGG lines exist; the complete SUGGDBG-QTY table reads ncopysugg=0 nsugg=0 copysugg=[] sugg=[] on all seven. All 7 find_free_reg calls have used == first_used, and local-alloc.c:2205-2213 is the only consumer of the suggestion sets, so with empty sets the restriction is the identity map and the pass CANNOT change a seat. Mechanism read in source: combine_regs (local-alloc.c:1859-1899) records a suggestion only when one side of a tieable copy is a HARD register, and s23 measured that $v1/$a0 never appear as hard regs in this function's pre-RA RTL -- the same structural fact that kills global.c set_preference. inverse.py local returns FORECLOSED on all four blocks (55/24/24/55 perturbations).
- verdict: KILLED

## [s24] The contested address object is not a local-alloc object at all, so no local-alloc pass (suggested-register or main) is in its causal chain.
- mechanism: local_alloc assigns quantities that live within one basic block and sets reg_renumber; global_alloc builds allocnos only for pseudos local-alloc left unassigned. If pseudo 74 is a global allocno then local-alloc never touched it.
- probe: Compared the local-alloc pseudo set from the new .local.json/.sugg.json against s23's measured global allocno set.
- result: Local set {76,79,83,87,89,91,93} is DISJOINT from the global allocno set {72,73,74,77,78,81,82,85,86}. Pseudo 74 -- the &D_80106A73 address object whose seat is the whole residual -- is a GLOBAL allocno. Same structural refutation shape Phase 7 recorded for DispPracticeMenuTex_A. This makes the H1 kill unconditional rather than chassis-relative.
- verdict: CONFIRMED

## [s24] qty_size (the other named Phase-5 hook gap) mis-prices a DImode quantity here and hides an order lever.
- mechanism: qty_compare's priority is floor_log2(refs)*refs*size/(death-birth)*10000; the Python model hardcoded size=1, so a DImode (size 2) quantity would be mispriced and could reorder allocation.
- probe: Read the now-dumped `size` column from SUGGDBG-QTY for all seven quantities.
- result: size == 1 on every quantity. No DImode quantity exists, independently reconfirming s23's 'all 9 allocnos are mode SI'. The gap cannot be operating here.
- verdict: KILLED

## [s24] GATE 2: SOTN master ships a PSX/GCC-2.7.2 function holding two simultaneously live C pointer handles on ONE global address (the closing construct for this function).
- mechanism: The frozen sanctioned-family list requires an in-hand SOTN-master precedent (file+line) for a coercion/spelling family; s22's first-hand census was NEGATIVE, and a second independent source was available this session.
- probe: Grouped the 163 PSX (untagged) `pointer_alias` rows of docs/reference/sotn-construct-index.md (commit aa53500226ee84be763f3e8702b27de06456b3a7) by (file, identical RHS address expression), then read the two tightest groups first-hand in the sotn-decomp checkout.
- result: 33 multi-alias groups, every one being one alias per FUNCTION repeated across sibling functions in the same file (line spans 12 to 3373). src/dra/4DA70.c:30/:42 are func_800EDAE4 and func_800EDB08; src/st/rare/e_azaghal.c:475-477/:489-491 are InitPositionLerp and ApplyPositionLerp, each taking three aliases to three DIFFERENT members. Zero PSX functions hold two live handles on one address. Gate 2 FAILS from a second independent source.
- verdict: KILLED

## [s25] The fresh address pseudo that s20 created before block 1 can be moved to the block-1/block-2 boundary, where it would give blocks 2-3 a different hard register from blocks 0-1 out of ONE C object.

- mechanism: s20 measured that `q = q + 3; q = q - 3;` before block 1 makes an
  extra address value appear in the stream (it recovers the target's missing
  `lbu` at zero instruction cost). If that extra value is a genuine second
  address pseudo rather than a folded temp, then placing it at the boundary
  between block 1 and block 2 would split the base's live range in exactly the
  place the target splits it, and the "second base needs a second C object"
  chain (global.c:426, one allocno per pseudo, no live-range splitting) would be
  void - which is what the brief's CONTRADICTION RULE says to re-audit first.
- probe: four placements of the round-trip on the score-10 chassis, each built
  and scored with `sandbox func_80034F88 --disable all`, the boundary form also
  disassembled and diffed against the base chassis stream
  (tmp/grind/func_80034F88/s25/sweep.ps1, build.txt, v3.txt).
- result: KILLED. Round-trip ADDED after block 1's store, with blocks 2/3
  keeping their own `q = &D_80106A73;`: score 10 at 49 insns and the emitted
  instruction stream is BYTE-FOR-BYTE IDENTICAL to the base chassis (empty
  diff). The construct is folded away completely at that position. Round-trip
  REPLACING block 2's re-materialisation: score 11 at 47 insns - the address
  never dies, the second lui/addiu pair is never emitted, the function is two
  instructions short. Replacing block 2's AND block 3's: score 13 at 45 insns.
  Round-trip before block 1 re-measured on HEAD: score 10 at 49 insns
  (s20 reconfirmed). The round-trip's only effect is to make an otherwise
  cse-redundant `q = &D_80106A73;` set survive as its own address pair - a
  re-materialisation ENABLER, not an allocno splitter. The RTL temp is
  copy-propagated into `q`'s pseudo; `q` still has exactly one allocno and
  therefore exactly one hard register for its whole live range.
- verdict: KILLED (instance - four placements of one diagnostic construct on
  the score-10 single-object chassis)

## [s25] Two different address expressions for the same byte (symbol_ref(A73)+0 in blocks 0-1, symbol_ref(A70)+3 in blocks 2-3) give the one C object two allocnos, because cse cannot unify the two rtxs.

- mechanism: cse's value table keys on the address rtx; two non-identical
  constants for the same address would defeat unification, and if the
  non-unified sets were allocated independently the second could land in a
  different seat.
- probe: blocks 2 and 3 re-materialise with `q = &D_80106A70 + 3;`, blocks 0-1
  keep `q = &D_80106A73;`; built and scored.
- result: KILLED. Score 12 at 49 insns - two points WORSE than the base chassis,
  and the two extra points are exactly the differing %hi/%lo symbol in the
  block-2/3 lui/addiu pairs. Still one `q`, still one seat; blocks 2-3 keep the
  same register they had. The form is additionally inadmissible on its own
  terms: `extern u8 D_80106A70;` indexed at +3 is the out-of-bounds declaration
  pun the dispatch brief's auto-scan flags.
- verdict: KILLED (instance - one form on the score-10 single-object chassis,
  no FAKE construct present)

## [s25] Sibling func_80034708 (same TU, same global D_80106A73) holds a spelling of the shared flag-byte access that transplants onto this chassis and drops the floor.

- mechanism: the brief's SIBLING LEDGERS rule - a sibling's candidate is
  inheritance this ledger never wrote, and CD_datasync/CD_sync showed a
  foreclosed sibling holding the shared window's fix.
- probe: read memory/grind/func_80034708/{state.json,evidence.md}; look for a
  candidate.c and for the spelling of every block the two functions share.
- result: KILLED as a transplant, CONFIRMED as corroboration. func_80034708 has
  NO candidate.c and its floor is 542 after one session (s1, 2026-07-07), so
  there is no spelling to transplant. Its target evidence is still worth
  banking: its jump-table cases 8 and 9 spell `D_80106A73 ^= 1` / `^= 2` through
  a single long-lived pointer local (`s5 = &D_80106A73` held across the whole
  phase-B loop). So the original codebase's idiom for this byte is a POINTER
  OBJECT, not a direct symbol access - independent corroboration of
  candidate.c's object model and of s16's 28/29 measurements on the plain-symbol
  family - and func_80034708 holds exactly ONE handle in a function far longer
  than this one.
- verdict: KILLED (instance - transplant unavailable, sibling at floor 542 with
  no candidate.c as of 2026-09-04)

## [s25] FRONTIER RESET (synthesis - the merged attack for the ladder's second cycle)

s25's job was to merge, re-audit and re-aim, and the merge changes the shape of
the remaining question in one specific way, so state the new frontier precisely:

F1 (the residual, now stated as a seat-space argmax rather than a dial).
The whole 10-point residual is blocks 0 and 1 wearing blocks 2/3's register
convention. The target runs TWO conventions because it has TWO simultaneously
live address values; we run ONE because we have one C object, hence one pseudo,
hence one allocno (global.c:426), hence one hard register for the whole live
range. s25 prices the only remaining single-object dial - the $v1/$a0 seat swap
- and it is a NET LOSS (about +8 recovered in blocks 0-1 against about -12 given
back in blocks 2-3, which are currently exact). So the current chassis is not a
point in the one-object seat space that we have failed to improve on; it is the
MAXIMUM of that space. Nothing that reassigns one seat can help, which retires
the entire class of probes that 22 of the first 24 sessions ran.

F2 (what is actually left, and it is not RA). Reaching 0 needs a second address
allocno. Every route to one that does not declare a second C object is now
measured closed: anonymous symbol refs never create a pseudo (s16, 14-28);
reload rematerialises the REG_EQUIV constant instead of allocating
(reload.c:4128-4137, s16); aggregates either scalarise to one pseudo or go
frame-resident (s18, 10 / 35); the census-exact diagnostic chassis creates no
second allocno (s23, still 9 allocnos); local-alloc's suggested-register pass is
inert on a DISJOINT pseudo set (s24); and now dead round-trip arithmetic makes
temps, not allocnos, and is stream-identical at the only boundary that mattered
(s25). The second C object is the standing banned construct, and the two-object
family reaches 8, not 0 (s21) - only the three-object family reaches 0, and the
Judge FAILed it on 2026-08-13.

F3 (the one genuinely unmeasured thing left, and it is a DECLARATION question,
not a spelling question). Everything above assumes the flag byte is reached
through `extern u8 D_80106A73;`. The dispatch brief's DATA MODEL scan says
D_80106A70 is indexed with a computed register and is really an ARRAY (or a
record base) mis-declared as a scalar, and code6cac.c:340-342 writes
D_80106A70/71/72 as three separate scalars while code6cac_c_mid.c:205 casts
`&D_80106A70` to `Quad *`. If the true declaration is one aggregate spanning
0x80106A70..0x80106A73, then the flag byte and the copy-loop destination are
MEMBERS OF THE SAME OBJECT, and the aggregate-merge family (no-new-park-
categories.md, 2026-08-17 entry, five prongs) - not the pointer-alias family -
is the frame in which a second live base could be ordinary C rather than a
second handle. s17 measured the ADDRESS SPELLING `&D_80106A70 + 3` as
codegen-neutral and s25 measured it at 12 as a per-block re-materialisation, but
NEITHER measured the DECLARATION: an actual `extern u8 D_80106A70[4];` (or a
struct) in include/code6cac.h, with code6cac.c:340-342 and
code6cac_c_mid.c:205 updated to match, has never been built. That is a
cross-TU header edit, i.e. an integration-handoff-shaped change rather than a
single-file grind edit, and prong (a) of the aggregate-merge family requires
base-register or stride evidence - which the copy loop's
`lui $at / addu $at,$v1 / sb $v0,%lo(D_80106A70)($at)` arguably supplies for
A70..A72 but NOT for A73, whose accesses use an independent %hi/%lo pair. It is
the only unmeasured axis the ledger can name, and it should be measured before
any further disposition attempt.

## [s25] F3, opened and closed in the same session: declaring 0x80106A70..73 as ONE aggregate (`extern u8 D_80106A70[4];`) makes the flag byte and the copy-loop destination members of one object, which gives the function a second naturally-live base and/or removes the declaration pun.

- mechanism: the dispatch brief's DATA MODEL scan flags D_80106A70 as
  indexed-with-a-computed-register but declared `extern u8 D_80106A70;`, i.e.
  an array mis-declared as a scalar; the sanctioned fix for a use-site pun is at
  the DECLARATION (aggregate-merge family, no-new-park-categories.md 2026-08-17
  entry). Every prior session varied the address SPELLING; none had ever built
  the changed DECLARATION.
- probe: patch include/code6cac.h:472-474 (three u8 scalars -> one
  `extern u8 D_80106A70[4];`), then build two forms - (w2) copy loop spelled
  `D_80106A70[i] = ...` with the flags still via `q = &D_80106A73;`, and (w1)
  the flags ALSO via the array, `q = &D_80106A70[3];`. Score both; disassemble
  w1 and dump its relocations. Header and src restored to HEAD afterwards.
- result: PARTLY CONFIRMED (hygiene), KILLED (lever). w2 = score 10 at 49 insns,
  exactly the base chassis - so removing the pun is CODEGEN-NEUTRAL and free,
  though it needs src/code6cac.c:340-342/:345 and src/code6cac_c_mid.c:205 to
  move with the header, i.e. an integration handoff rather than a grind edit.
  w1 = score 12 at 49 insns, and the disassembly plus `objdump -r` show the only
  differences are three `addiu a0,a0,3` against R_MIPS_LO16 D_80106A70, which
  link to the target's own 0x6A73 immediate - the known false LO16-addend
  distance ([[sandbox-lo16-text-addend-false-distance]], as in s17). So the
  aggregate model is byte-COMPATIBLE with the target for the flag byte too, and
  the target's asm is not evidence against it - but it creates NO second live
  base: one `q`, one pseudo, one allocno, one seat, blocks 0-1 still mirrored.
  Merging the declaration changes which symbol the relocation names and nothing
  else.
- verdict: KILLED (instance - two forms on the score-10 single-object chassis
  with include/code6cac.h patched to `extern u8 D_80106A70[4];`, no FAKE
  construct present)

## [s25] The fresh address pseudo that s20 created before block 1 with a dead round-trip (`q = q + 3; q = q - 3;`) can be moved to the block-1/block-2 boundary, where it would give blocks 2-3 a different hard register from blocks 0-1 out of ONE C object.
- mechanism: s20 measured that the round-trip before block 1 makes an extra address value appear in the stream and recovers the target's missing lbu at zero instruction cost. If that value were a genuine second address pseudo rather than a folded temp, placing it at the block-1/block-2 boundary would split the base's live range exactly where the target splits it, voiding the global.c:426 one-allocno-per-pseudo chain that s23/s24's foreclosures rest on. This was the ledger's weakest foreclosure (its own measurement contradicted the ceiling argument above it), so the brief's CONTRADICTION RULE points at it first.
- probe: Four placements of the round-trip built on the score-10 chassis and scored with `sandbox func_80034F88 --disable all`; the boundary form additionally disassembled (mipsel-linux-gnu-objdump -d on tmp/sandbox/func_80034F88/code6cac_b.o) and diffed against the base chassis stream. Driver: tmp/grind/func_80034F88/s25/sweep.ps1.
- result: Round-trip ADDED after block 1's store with blocks 2/3 keeping their own `q = &D_80106A73;`: score 10 at 49 insns, and the emitted stream is BYTE-FOR-BYTE IDENTICAL to the base chassis (diff of build.txt vs v3.txt is empty) -- folded away completely, no fresh pseudo, no seat change. Round-trip REPLACING block 2's re-materialisation: score 11 at 47 insns (the address never dies, the second lui/addiu pair is never emitted). Replacing block 2's AND block 3's: score 13 at 45 insns. Round-trip before block 1 (s20's own form) re-measured on HEAD: score 10 at 49 insns, reconfirmed. Mechanism the four name together: the round-trip's only observable effect is to make an otherwise cse-redundant `q = &D_80106A73;` set survive as its own address pair -- a re-materialisation ENABLER, not an allocno splitter. The RTL temp is copy-propagated into q's pseudo, so q still has exactly one allocno and one hard register for its whole live range. s20's 'one C object = one pseudo is not a theorem' reading is narrowed to its true scope: extra address temps can appear, they cannot carry a second simultaneously-live base.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 single-object chassis on HEAD (src/code6cac_b.c:3420 INCLUDE_ASM replaced by the body, rules_dropped 0); diagnostic dead-pointer-arithmetic construct present in all four forms, never installed as a candidate

## [s25] Two different address expressions for the same byte -- symbol_ref(D_80106A73)+0 in blocks 0-1 and symbol_ref(D_80106A70)+3 in blocks 2-3 -- give the one C object two allocnos, because cse cannot unify the two rtxs.
- mechanism: cse's value table keys on the address rtx; two non-identical constants for the same address defeat unification, and if the non-unified sets were allocated independently the second could land in a different seat.
- probe: Blocks 2 and 3 re-materialise with `q = &D_80106A70 + 3;` while blocks 0-1 keep `q = &D_80106A73;`; built and scored on the score-10 chassis.
- result: Score 12 at 49 insns -- two points WORSE than the base chassis, the two points being the differing %hi/%lo symbol in the block-2/3 lui/addiu pairs. Still one q, still one seat; blocks 2-3 keep the register they already had and blocks 0-1 stay mirrored. The form is additionally inadmissible on its own terms: `extern u8 D_80106A70;` indexed at +3 is the out-of-bounds declaration pun the dispatch brief's auto-scan flags.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 single-object chassis on HEAD, no FAKE construct present

## [s25] Declaring 0x80106A70..73 as ONE aggregate (`extern u8 D_80106A70[4];`) makes the flag byte and the copy-loop destination members of one object, which gives the function a second naturally-live base and/or removes the declaration pun.
- mechanism: The dispatch brief's DATA MODEL scan flags D_80106A70 as indexed with a computed register but declared as a scalar extern -- an array mis-declared -- and names the aggregate-merge family (no-new-park-categories.md 2026-08-17) as the sanctioned fix at the DECLARATION rather than the use site. Every prior session varied the address SPELLING; none had ever built the changed DECLARATION.
- probe: Patched include/code6cac.h:472-474 from three `extern u8 D_80106A70/71/72;` scalars to `extern u8 D_80106A70[4];` (the sandbox compiles only src/code6cac_b.c, so the out-of-TU consumers do not affect the measurement) and built two forms: w2 = copy loop spelled `D_80106A70[i] = ...` with the flags still via `q = &D_80106A73;`, and w1 = flags also via the array, `q = &D_80106A70[3];`. Scored both; disassembled w1 and dumped its relocations with `objdump -r`. Header and src restored to HEAD at the end of the session.
- result: SPLIT: hygiene CONFIRMED, lever KILLED. w2 = score 10 at 49 insns, exactly the base chassis -- so removing candidate.c's declaration pun (`*(&D_80106A70 + i)` against a scalar extern, a standing layer-1 FAIL finding) is CODEGEN-NEUTRAL and free, though the header change drags src/code6cac.c:340-342/:345 and src/code6cac_c_mid.c:205 with it, making it an integration handoff rather than a grind edit. w1 = score 12 at 49 insns, and the disassembly plus objdump -r show the only differences are three `addiu a0,a0,3` against R_MIPS_LO16 D_80106A70, which link to the target's own 0x6A73 immediate -- the known false LO16-addend distance (sandbox-lo16-text-addend-false-distance), so w1 is byte-identical to the base chassis after link and the target's asm is NOT evidence against the one-object model. But neither form creates a second live base: one q, one pseudo, one allocno, one seat, blocks 0-1 still mirrored. Merging the declaration changes which symbol the relocation names and nothing else.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD with include/code6cac.h:472-474 patched to `extern u8 D_80106A70[4];`, no FAKE construct present; header and src restored to HEAD before session end

## [s25] Sibling func_80034708 (same TU, same global D_80106A73) holds a spelling of the shared flag-byte access that transplants onto this chassis and drops the floor.
- mechanism: The brief's SIBLING LEDGERS rule -- a sibling's candidate is inheritance this ledger never wrote, and the 2026-09-04 CD_datasync/CD_sync post-mortem showed a foreclosed sibling holding the shared window's fix.
- probe: Read memory/grind/func_80034708/state.json and evidence.md; look for candidate.c and for the sibling's spelling of every block the two functions share.
- result: No transplant exists: func_80034708 has NO candidate.c and its floor is 542 after a single session (s1, 2026-07-07). One evidentiary gain banked instead -- its jump-table cases 8 and 9 spell `D_80106A73 ^= 1` / `^= 2` through a single long-lived pointer local (`s5 = &D_80106A73`, held across the whole phase-B loop), so the original codebase's idiom for this byte is a POINTER OBJECT rather than a direct symbol access. That independently corroborates candidate.c's object model and s16's 28/29 measurements on the plain-symbol family, and it supplies no second handle: func_80034708 holds exactly one, in a function far longer than this one.
- verdict: KILLED
- kill_scope: instance
- measured_on: sibling ledger memory/grind/func_80034708/ as of 2026-09-04 (floor 542, 1 session, no candidate.c); no build performed

## [s26] The 10-point residual is an RA seat-count problem (the ledger's F1/F2 framing from s16-s25).

- mechanism: 25 sessions modelled the residual as register allocation -- one
  address pseudo, one allocno (global.c:426), therefore one hard register,
  therefore blocks 0-1 cannot wear a different convention from blocks 2-3.
  Every session from s16 on searched RA inputs (refs / live span / birth order /
  conflicts / preferences / calls-crossed / local-alloc suggestions).
- probe: `tools/ra_solver/inverse_compose.py classify code6cac_b func_80034F88
  --target-object build/src/code6cac_b.o --ours-object
  tmp/sandbox/func_80034F88/code6cac_b.o` -- the triage step the solver
  playbook mandates FIRST and which no prior session had ever run on this
  function. Then `pwsh tools/grinder/dump.ps1 func_80034F88` and a per-pass
  count of `(mem:QI` in the func_80034F88 region of every dump.
- result: KILLED as a framing. classify says **FIRST DIVERGENCE: PRE-RA** on the
  score-10 chassis: `ours only : nop` / `target only : lbu #,0(#)`. The
  instruction MULTISET differs, so by the classifier's own doctrine the RA and
  scheduler models "permute and rename a FIXED set of insns" and searching them
  "would produce fiction". The pass is named by the dumps, not guessed:
  the `(mem:QI` count in the func_80034F88 region is 8 in .rtl, 8 in .jump,
  **7 in .cse**, and 7 thereafter -- **cse.c (the first CSE pass) deletes
  block 1's byte reload** (.rtl insn 29 `(set (reg:QI 80) (mem:QI (reg/v:SI
  74)))`, feeding insn 30 `(set (reg 77) (zero_extend:SI (reg:QI 80)))`). It
  forwards the value stored by block 0's insn 20 `(set (mem:QI (reg 74))
  (subreg:QI (reg 76) 0))` because reg 76 is `and(...,248)` and therefore
  provably zero in its high bits. The residual therefore has TWO independent
  defects, which every prior session conflated: **D1** = one missing byte load,
  upstream of RA (cse.c), and **D2** = the seat convention, which is downstream
  of D1 and only meaningful once D1 is fixed.
- verdict: KILLED
- kill_scope: instance
- measured_on: memory/grind/func_80034F88/candidate.c installed at src/code6cac_b.c:3420 on HEAD, sandbox re-measured score 10 / 49 target / 49 build / rules_dropped 0; no FAKE construct present; dumps produced by tools/gcc-2.7.2/cc1 via tools/grinder/dump.ps1

## [s26] D1 (cse.c deleting block 1's reload) has an ordinary-C fix that keeps ONE pointer object.

- mechanism: cse.c's memory table is keyed on the address REGISTER, and the
  entry is dropped when that register is re-set to a value cse cannot prove
  equal, or when the reading MEM's address rtx is a different pseudo. Eight
  shapes were built to try to reach one of those states honestly, from a single
  `u8 *q`.
- probe: eight bodies built and scored with `sandbox func_80034F88
  --disable all` (tmp/grind/func_80034F88/s26/sweep1.txt), each of the
  plateau ones additionally re-classified with `inverse_compose.py classify`
  (tmp/grind/func_80034F88/s26/probe1.txt):
  w01 read duplicated into both branch arms (`if (c) c = *q|1; else c = *q;`);
  w02 the same in all three flag blocks; w03 block 1 as a conditional
  expression `*q = c ? (*q|1) : *q;`; w04 `do { ... } while (0);` around
  block 1; w05 statement-order swap (`v = *q;` before `c = p[8] & M;`);
  w06 the value local typed `u8` instead of `s32`; w07 `do { *q &= 0xF8; }
  while (0);` around block 0; w08 an explicit `q = &D_80106A73;`
  re-materialisation added to block 1 as well (four sets of the one object).
- result: KILLED for all eight. w01 = 10/49 STILL PRE-RA (cse follows jumps at
  -O2, so the equivalence propagates into both arms and jump2 re-merges them);
  w02 = 25/53; w03 = 12/46; w04 = 10/49 still PRE-RA; w05 = 10/49 still PRE-RA;
  w06 = 19/51; w07 = 15/50 and it ADDS a second nop rather than a load;
  w08 = 10/49 still PRE-RA (cse recognises the re-set as redundant, replaces its
  source with the register already holding the symbol and keeps the same qty, so
  no invalidation happens). The mechanism these eight name together: from ONE
  pointer object every read of the byte after block 0's store is
  `(mem:QI (reg 74))` with reg 74 unchanged, and cse forwards it
  unconditionally. Only two constructs measured this session restore the load,
  and both are inadmissible: a dead pointer round-trip (`q = q + 3; q = q - 3;`)
  and a second C pointer object read in block 1.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 single-object chassis on HEAD; no FAKE construct present in any of the eight forms (w04/w07 carry an un-annotated do-while(0) built as a diagnostic only, never proposed as a candidate)

## [s26] A chassis whose instruction MULTISET matches the target exists, and on it the RA foreclosure has to be re-derived (s23/s24's foreclosures were computed on a PRE-RA-divergent model).

- mechanism: the classifier's doctrine cuts both ways -- if the base chassis is
  PRE-RA-divergent then s23's `inverse.py global` FORECLOSED and s24's
  `inverse.py local` FORECLOSED were computed on a model with the wrong insn
  set, which is exactly the func_80072CD4 defect the tool was written to
  prevent. The re-audit the brief mandates is therefore not a spelling
  re-measurement but a re-run of the RA search on a multiset-matching model.
- probe: build the two dead-round-trip diagnostics (x09 `q = q + 3; q = q - 3;`
  and x10 `q = q + 1; q = q - 1;` before block 1), score them, classify them;
  then on x09 run `extract.py func_80034F88 code6cac_b`, `simulate.py`, and
  `inverse.py global` twice -- goal A = the full blocks-0/1 exchange
  `{"74": 3, "73": 4, "77": 4}` (the FULL target disposition for that block
  group, not a subset), goal B = the address alone `{"74": 3}`.
  Console at tmp/grind/func_80034F88/s26/inverse_x09.txt.
- result: SPLIT -- the premise CONFIRMED, the conclusion UNCHANGED, which is
  the valuable half. x09 and x10 both score 10 at 49 insns and both
  **classify RA, not PRE-RA**: the dead round-trip restores exactly the missing
  `lbu` and the multiset then matches the target's, so a multiset-matching
  chassis is reachable and the residual on it is purely register naming. The
  re-extracted model on x09 validates 9/9 dispositions with sort order MATCH
  (pseudo 74's priority moves 10344 -> 15000, i.e. it rises from 5th to 2nd in
  the allocation order -- the model is genuinely different from s23's). And
  `inverse.py global` still returns **FORECLOSED on both goals**: no
  perturbation of refs / live span / birth order / conflicts / preferences /
  calls-crossed, to depth 2 over the 260-atom space, reaches the target
  assignment, with 18 preference atoms mechanically non-emittable because
  neither $v1 nor $a0 ever appears as a hard register in this function's pre-RA
  RTL. So s23/s24's RA foreclosure is UPGRADED rather than voided: it now stands
  on a chassis whose instruction multiset matches the target, where the
  classifier's "searching this produces fiction" objection does not apply.
- verdict: CONFIRMED

## [s26] The target's own instruction multiset -- not a seat preference -- is what requires more than one C pointer object on D_80106A73, so the standing ban rests on a premise this session measured false.

- mechanism: the ban (and the 2026-08-13 Judge FAIL) classify the repeated
  `u8 *q = &D_80106A73;` handles as a register-allocation lever with no
  observable effect on the function's output. D1 is not a seat: it is one extra
  `lbu` in the target's instruction stream, upstream of RA. If a second handle
  is the only structure that reproduces the target's instruction SET, then the
  handles are what the original source contained, not a coercion applied to an
  otherwise-correct body.
- probe: two diagnostic bodies, scored and classified. y01 = two objects with
  the SECOND used only by blocks 2-3, block 1 still reading through the first
  (`q` for blocks 0-1, `r` for blocks 2-3). y02 = two objects with block 1
  reading through a freshly-set second handle (`r = &D_80106A73;` inside
  block 1, `r` also used by blocks 2-3).
  tmp/grind/func_80034F88/s26/probe3.txt.
- result: CONFIRMED, and it isolates the mechanism precisely. y01 = score 21 and
  still **PRE-RA** with the same `nop`-for-`lbu` delta: adding a second object
  does NOT restore the load if block 1 keeps reading through the first pointer.
  y02 = score 13 and **RA**: the load is restored and the multiset matches the
  target, at NO extra lui/addiu (still 49 insns, still three %hi/%lo pairs --
  cse turns the second handle's set into a copy of the first and flow propagates
  it away, while keeping mem(r) as a separate table entry). So what restores the
  target's missing instruction is specifically *the block-1 read going through a
  different pointer PSEUDO*, and from C the only two ways to get one are a dead
  round-trip on the single object (a cheat) or a second declared object (the
  standing ban). Every remaining sanctioned axis is now measured dead: D1 has no
  one-object honest fix in eight shapes (s26), and D2's RA search forecloses on
  the corrected multiset-matching model (s26) as well as on the old one
  (s23/s24).
- verdict: CONFIRMED

## [s26b] The target's block-3 emission order (address re-materialisation before block 1's store) is reachable by perturbing the scheduler's inputs on the current one-address-pseudo chassis.
- mechanism: sched1/sched2 build each block backwards from the tail; `rank_for_schedule` orders by INSN_PRIORITY, then dependence class relative to the last pick, then INSN_LUID descending. If the triple were a tie, a statement move (LUID change) or a cost change would flip it — that is the entire premise of the sched_solver toolkit and of the ledger's "16/17/18 ordering triple" framing.
- probe: `tools/sched_solver/extract.py code6cac_b` (parity=True, 1464 blocks, 7270 picks) on candidate.c installed at HEAD; target pinned by splicing `asm/funcs/func_80034F88.s` into `code6cac_b.hon.s` (`tmp/grind/func_80034F88/s26/splice_tgt.py`); `goalmap.py --model --pass 1|2`; then `perturb.py --func func_80034F88 --pass 1|2 --block 3 --goal-order ... --depth 2 --max 80` for both intra-block tie spellings of the target order.
- result: KILLED (class). goalmap reports GOAL == OURS (identity) for all 11 blocks in both passes with `moved: 0`. perturb finds vectors only under `del_dep 56 <- 49`: pass 1 five vectors, pass 2 three, and for the goal matching our own tie the pass-2 minimal vector is the single atom `del_dep 56 <- 49` alone. Insn 49 is block 1's store through the address pseudo and insn 56 is that pseudo's re-materialisation, so the model carries `"56": [[49, 14]]` — a REG_DEP_ANTI edge emitted at tools/gcc-2.7.2/sched.c:1738 from `reg_last_uses`. `schedule_block` releases an insn only when all of its LOG_LINKS are satisfied, so no priority/LUID/cost/function-unit atom can invert a dependence edge. Block 6 carries the mirror edge `"90": [[83,14]]` and IS exact, because there the target also stores first — the model reproduces both. The scheduler axis is closed for every C form that keeps one address pseudo; the edge exists iff the store's pseudo and the re-materialisation's pseudo are the same.
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/sched.c:1738
- measured_on: candidate.c score-10 single-object chassis installed at HEAD (src/code6cac_b.c:3420), no FAKE construct beyond candidate.c's own single `u8 *q` pointer alias; sched model tmp/sched_solver_work/code6cac_b.sched.json, parity=True.

## [s26b] A direct (non-pointer) access to `D_80106A73` creates a fresh address pseudo per use, giving the second live address value without declaring a second C pointer object.
- mechanism: on MIPS with -G0 a global's address must reach a register before a byte load/store, so `D_80106A73 |= 2;` would expand to `(set (reg N) (symbol_ref))` + `(mem (reg N))` with a FRESH pseudo N per statement. cse merges those sets only inside one basic block, so blocks 2 and 3 (each behind an if-join label) would keep their own pseudos — deleting the sched.c:1738 anti-dependence and restoring the cse-deleted reload at once, using ordinary C and no second declared object.
- probe: five hybrid bodies mixing the existing `u8 *q` pointer object with direct-symbol blocks, generated into tmp/grind/func_80034F88/s26/variants4/ and swept with `sandbox func_80034F88 --disable all`; then `mipsel-linux-gnu-objdump -d` on the sandbox object for h1 (tmp/grind/func_80034F88/s26/h1.dis).
- result: KILLED (class). The premise is false: `CONSTANT_ADDRESS_P` at tools/gcc-2.7.2/config/mips/mips.h:2369 accepts SYMBOL_REF, so a direct global access is a legitimate address and stays `(mem (symbol_ref))` — no address pseudo is ever allocated. h1's bytes show block 2 as `lui $v0,%hi(sym)` then `lbu $v0,%lo(sym)($v0)`: ONE lui, NO addiu, the %lo folded into the memory operand, 48 insns against the target's 49. The target has three UNFOLDED lui+addiu pairs, which only a pointer-typed value forces. Scores: h1 (ptr 0-1 / sym 2-3) 28 at 48; h2 (ptr for the mask only) 22 at 52; h3 (ptr 0-2 / sym 3) 16 at 49; h4 (h1 + block-1 re-materialisation) 28 at 48; h5 (ptr 0-1, sym 2, ptr 3) 20 at 50. This also retro-explains every plain-symbol and hybrid score already in the bank (14/16/18/20/22/26/28/29/30/35) from one mechanism.
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:2369
- measured_on: candidate.c score-10 single-object chassis at HEAD; the five hybrid bodies carry candidate.c's single `u8 *q` pointer alias and no other FAKE construct; banked at memory/grind/func_80034F88/rejected/hybrid-h[1-5]-*.c.

## [s26] The target's block-3 emission order (the address re-materialisation emitted before block 1's store) is reachable by perturbing the scheduler's inputs -- statement order/LUID, insn cost, priority -- on the current one-address-pseudo chassis.
- mechanism: sched1/sched2 build each basic block backwards from the tail; rank_for_schedule orders by INSN_PRIORITY, then dependence class relative to the last pick, then INSN_LUID descending. If the triple were an equal-priority tie, a statement move or a cost change would flip it -- that is the premise of the sched_solver toolkit and of the ledger's 'ordering triple' framing carried since s25.
- probe: tools/sched_solver/extract.py code6cac_b (parity=True, 128 funcs, 1464 blocks, 7270 picks) on candidate.c installed at HEAD; target pinned by splicing asm/funcs/func_80034F88.s into code6cac_b.hon.s (tmp/grind/func_80034F88/s26/splice_tgt.py -> tmp/sched_map/code6cac_b.tgt.head.s); goalmap.py --model --pass 1|2; perturb.py --func func_80034F88 --pass 1|2 --block 3 --goal-order <both intra-block tie spellings of the target order> --depth 2 --max 80.
- result: goalmap reports GOAL == OURS (identity) for all 11 blocks in BOTH passes, with difflib moved:0 (that half is corroborating rather than decisive: the alignment is weak -- equal 3 / replace 8 / delete 33 / insert 38 -- because register naming differs almost everywhere). perturb is the decisive half: across 95-96 single atoms plus depth-2 pairs, EVERY vector that reaches the target order begins with `del_dep 56 <- 49` (pass 1: 5 vectors; pass 2: 3 vectors), and for the goal matching our own intra-block tie the pass-2 minimal vector is the lone atom `del_dep 56 <- 49`. Insn 49 is block 1's store through the address pseudo, insn 56 is that pseudo's re-materialisation, and the extracted model carries "56": [[49, 14]] -- the REG_DEP_ANTI edge added from reg_last_uses. schedule_block releases an insn only when all of its LOG_LINKS are satisfied, so no priority/LUID/cost/function-unit atom can invert a dependence edge. Block 6 carries the mirror edge "90": [[83, 14]] and IS exact, because there the target also stores first -- the model reproduces both facts. The edge exists exactly when the store's pseudo and the re-materialisation's pseudo are the same one.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c score-10 single-object chassis installed at HEAD (src/code6cac_b.c:3420; sandbox re-measured this session: score 10, 49/49 insns, rules_dropped 0); no FAKE construct beyond candidate.c's own single `u8 *q` pointer alias; model tmp/sched_solver_work/code6cac_b.sched.json at parity=True.
- predicate_cite: tools/gcc-2.7.2/sched.c:1738

## [s26] A direct (non-pointer) access to D_80106A73 creates a fresh address pseudo per use, so spelling the later flag blocks as `D_80106A73 |= 2;` yields the target's second live address value without declaring a second C pointer object.
- mechanism: On MIPS with -G0 a global's address must reach a register before a byte load/store, so each direct access would expand to (set (reg N) (symbol_ref)) + (mem (reg N)) with a fresh pseudo N; cse merges such sets only within one basic block, so blocks 2 and 3 -- each behind an if-join label -- would keep their own pseudos, simultaneously deleting the sched.c:1738 anti-dependence and restoring the cse-deleted block-1 reload, in ordinary C with no second declared object.
- probe: Five hybrid bodies mixing candidate.c's `u8 *q` pointer object with direct-symbol blocks (tmp/grind/func_80034F88/s26/variants4/), swept with `sandbox func_80034F88 --disable all`; then mipsel-linux-gnu-objdump -d of the sandbox object for h1 (tmp/grind/func_80034F88/s26/h1.dis).
- result: The premise is false. CONSTANT_ADDRESS_P accepts SYMBOL_REF, so a direct global access is already a legitimate address: it stays (mem (symbol_ref)) and no address pseudo is allocated. h1's emitted bytes show block 2 as `lui $v0,%hi(sym)` then `lbu $v0,%lo(sym)($v0)` -- one lui, NO addiu, the %lo folded into the memory operand, 48 insns against the target's 49. The target has three UNFOLDED lui+addiu pairs, a shape only a pointer-typed value forces. Scores: h1 (ptr blocks 0-1 / sym 2-3) 28 at 48 insns; h2 (ptr for the mask only) 22 at 52; h3 (ptr 0-2 / sym 3) 16 at 49; h4 (h1 plus the block-1 re-materialisation) 28 at 48; h5 (ptr 0-1, sym 2, ptr 3) 20 at 50. One mechanism now retro-explains every plain-symbol and hybrid score already in the bank (14, 16, 18, 20, 22, 26, 28, 29, 30, 35) without re-deriving any of them.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c score-10 single-object chassis at HEAD; the five hybrid bodies carry candidate.c's single `u8 *q` pointer alias and no other FAKE construct; banked at memory/grind/func_80034F88/rejected/hybrid-h1..h5-*.c.
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:2369

## [s26] The floor recorded by the ledger (10) is still the floor of candidate.c on the current HEAD chassis.
- mechanism: The dispatch brief reported the driver's chassis measurement as unavailable, and every banked spelling conclusion in this ledger is chassis-relative, so the number had to be re-established before any of it could be spent.
- probe: src/code6cac_b.c:3420 INCLUDE_ASM replaced by memory/grind/func_80034F88/candidate.c verbatim (no extra extern needed: include/code6cac.h:472 declares D_80106A70, src/code6cac_b.c:128 declares D_80106A73); `& tools/wteng.ps1 main sandbox func_80034F88 --disable all`.
- result: score 10, target_insns 49, build_insns 49, rules_dropped 0, scorable true. Floor 10 confirmed on HEAD; src restored to HEAD afterwards.
- verdict: CONFIRMED

## [s27] From a single `u8 *q`, some assignment pattern reaches the target's instruction multiset -- four surviving byte loads at three address materialisations.

- mechanism: cse.c's `canon_hash` keys a REG on `reg_qty[regno]`
  (tools/gcc-2.7.2/cse.c:1905), so every `(mem:QI (reg 74))` hashes to one slot
  while reg 74 holds one quantity, and the entry recorded for block 0's store
  (`insert` at cse.c:7338, gated only by `sets[i].src_elt == 0` at cse.c:7328)
  is found by every later load. A new quantity therefore requires setting the
  pseudo to an rtx cse cannot unify with what it already holds -- and such an
  rtx is by construction a fresh address materialisation, i.e. a `lui`+`addiu`
  pair. The hypothesis was that some placement or spelling of the assignments
  breaks that coupling.
- probe: ten bodies varying only which of the four byte-access groups
  re-assigns `q`, and with which of two address spellings (`&D_80106A73`, or
  `(u8 *)((u8 *)&D_80106A70 + 3)` -- same address, non-unifiable rtx).
  `tmp/grind/func_80034F88/s27/gen.py` + `sweep.ps1`; bodies in
  `tmp/grind/func_80034F88/s27/variants/a{1..13}.c`; each scored with
  `sandbox func_80034F88 --disable all`, disassembled to `a*.txt` and
  re-classified with `tools/ra_solver/inverse_compose.py classify`.
- result: KILLED, and it yields two exact identities rather than a saturation
  count. Across the series, **surviving `lbu` == number of assignments to `q`**
  and **build insns == 41 + 2 x (number of assignments)**: 1 set = 47 insns /
  1 lbu / 23; 2 sets = 47 / 2 / 20; 3 sets = 49 / 3 / **10** (candidate.c) and
  49 / 3 / 14-22 for the mixed-spelling variants a1/a2/a5/a8; 4 sets = 51 / 4 /
  12-14 (a3, a4, a11, a12 -- all four restore EVERY byte load, and their
  classify output drops the `nop`-for-`lbu` delta entirely, leaving only
  `ours only: lui/addiu` for the surplus pair). Each assignment buys exactly
  one surviving load and costs exactly one `lui`+`addiu` pair. The target needs
  four loads (0x80034FA0/FB4/FD8/FFC) at three pairs (0x80034F98/FC8/FF0) in 49
  insns; the law gives four loads only at 51 insns and 49 insns only at three
  loads. No assignment pattern of one pointer object sits on the target's
  point.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/cse.c:1905
- measured_on: candidate.c score-10 single-object chassis installed at HEAD
  (src/code6cac_b.c:3420; sandbox re-measured this session: score 10, 49/49,
  rules_dropped 0); every form carries exactly candidate.c's one `u8 *q`
  pointer alias and no other FAKE-family construct; `src/` restored to HEAD.
  Banked at `rejected/one-object-4th-address-set-restores-all-lbu-51insn-score12.c`,
  `rejected/one-object-2-address-sets-2lbu-47insn-score20.c`,
  `rejected/one-object-3-sets-mixed-A70plus3-spelling-49insn-score14.c`.

## [s27] An input other than the address quantity can stop cse forwarding block 0's stored byte into block 1's load, and the original source used it.

- mechanism: `cse_insn` records a store's MEM destination unless
  `sets[i].src_elt == 0` (cse.c:7328); `canon_hash` zeroes it via
  `do_not_record` for a volatile MEM (cse.c:1943), for CALL / UNSPEC_VOLATILE
  (cse.c:1967), and for a non-fixed hard register under SMALL_REGISTER_CLASSES
  (cse.c:1902); `invalidate_memory` (cse.c:7599) drops the entry on a
  CALL_INSN or an ambiguous store; and cse's extended-basic-block paths break
  at code labels. Those five plus the address quantity are the complete input
  set.
- probe: each input checked against the TARGET's own instruction stream rather
  than against ours -- `asm/funcs/func_80034F88.s` between the block-0 `sb` at
  0x80034FAC and the block-1 `lbu` at 0x80034FB4, plus
  `tools/label_census.py --func func_80034F88`
  (`tmp/grind/func_80034F88/s27/label_census.txt`).
- result: KILLED for all five. The target's stream between the store and the
  reload is `sb`, `lw`, `lbu` -- no call (so neither the `do_not_record` CALL
  case nor `invalidate_memory`), no store (so no ambiguous-write invalidation),
  and the label census puts the target's earliest of four labels at insn 16,
  after block 1's compare arms, so there is no cse path boundary there either.
  SMALL_REGISTER_CLASSES is not defined for MIPS. Volatile is the only one of
  the five reachable from C and it is already measured dead as a family
  (12/15/28, `rejected/vol*`). The address quantity (cse.c:1905) is the sole
  surviving input, which is what makes the s27 cost law above complete rather
  than merely empirical.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/cse.c:7328
- measured_on: target asm `asm/funcs/func_80034F88.s` and the s27 dumps sliced
  from `tmp/grind/func_80034F88/dumps/` (`f88.jump`, `f88.cse`) on the
  candidate.c score-10 chassis at HEAD; no FAKE construct present.

## [s27] The s26 frontier route -- carrying the address in a pointer-typed value derived from `p` or from another live object that reaches 0x80106A73, rather than a second object aliasing D_80106A73.

- mechanism: a second live address value is what the target's multiset needs;
  if some already-live object contained the byte, a pointer derived from it
  would supply the second quantity without declaring a second handle.
- probe: the dispatch DATA MODEL and the naming census for any record covering
  0x80106A73; then the one derived spelling that exists,
  `(u8 *)((u8 *)&D_80106A70 + 3)`, built into eight of the ten s27 forms.
- result: KILLED. The only record covering the byte is 0x80106A70..0x80106A73
  itself (three default-colour nibbles + the flag byte); `p`, the
  `func_80077D00()` return, is unrelated memory, and nothing else in the
  function is live at the byte. `&D_80106A70[3]` does create a distinct
  quantity, but it obeys the same cost law -- one full `lui`+`addiu` pair --
  and additionally carries 2-12 points of `addiu #,#,3` vs `addiu #,#,0`
  distance in the emitted pairs. The route is closed by inspection and by
  measurement together.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis at HEAD; forms a1/a2/a3/a5/a7/a8/
  a11/a12 in `tmp/grind/func_80034F88/s27/variants/`; single `u8 *q` in every
  form, no other FAKE-family construct.

## [s27] The single `u8 *q` pointer alias in candidate.c is masking a lever (the mandated FAKE-ablation re-audit).

- mechanism: a lever measured inert while a FAKE carrier occupies its target
  pseudo is not a kill (func_8002EA24 s8), so the ledger's kills had to be
  re-checked with the candidate's only FAKE-family construct removed.
- probe: `tools/fake_ablate.py --func func_80034F88 --file code6cac_b
  --candidate memory/grind/func_80034F88/candidate.c`, then a hand ablation
  (a13: all four byte-access groups spelled as direct `D_80106A73` accesses,
  no pointer object at all), scored and classified.
- result: KILLED. `fake_ablate.py` reports "no FAKE-annotated constructs
  found" -- candidate.c's `u8 *q` is un-annotated, so the tool has nothing to
  strip. The hand ablation measures **score 29 at 47 insns** with three of the
  four `lbu` missing, consistent with the s16 plain-symbol family (28/29) and
  with s26's CONSTANT_ADDRESS_P finding (mips.h:2369). The alias is worth 19
  points and is load-bearing rather than a carrier; nothing is masked behind
  it.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis at HEAD; ablated body banked at
  `rejected/pointer-alias-ablated-plain-symbol-47insn-score29.c`.

## [s27] From a single `u8 *q`, some assignment pattern reaches the target's instruction multiset -- four surviving byte loads at three address materialisations.
- mechanism: cse.c's canon_hash keys a REG on reg_qty[regno] (tools/gcc-2.7.2/cse.c:1905), so every (mem:QI (reg 74)) hashes to one slot while reg 74 holds one quantity, and the entry recorded for block 0's store (insert at cse.c:7338, gated only by sets[i].src_elt == 0 at cse.c:7328) is found by every later load. A new quantity therefore requires setting the pseudo to an rtx cse cannot unify with what it already holds -- and such an rtx is by construction a fresh lui+addiu address materialisation.
- probe: Ten bodies varying only which of the four byte-access groups re-assigns `q`, and with which of two address spellings (&D_80106A73, or (u8 *)((u8 *)&D_80106A70 + 3) -- same address, non-unifiable rtx). tmp/grind/func_80034F88/s27/gen.py + sweep.ps1; variants a1..a13; each scored with `sandbox func_80034F88 --disable all`, disassembled to a*.txt, and re-classified with tools/ra_solver/inverse_compose.py classify.
- result: KILLED with two exact identities rather than a saturation count. surviving lbu == number of assignments to q, and build insns == 41 + 2 x that number: 1 set = 47 insns / 1 lbu / score 23; 2 sets = 47 / 2 / 20; 3 sets = 49 / 3 / 10 (candidate.c) and 49 / 3 / 14-22 for the mixed-spelling forms a1/a2/a5/a8; 4 sets = 51 / 4 / 12-14 (a3, a4, a11, a12 -- all four restore EVERY byte load, and classify drops the nop-for-lbu delta entirely, leaving only 'ours only: lui/addiu' for the surplus pair). Each assignment buys exactly one surviving load and costs exactly one lui+addiu pair. The target needs four loads (0x80034FA0/FB4/FD8/FFC) at three pairs (0x80034F98/FC8/FF0) in 49 insns; the law gives four loads only at 51 insns and 49 insns only at three loads.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c score-10 single-object chassis installed at src/code6cac_b.c:3420 on HEAD (sandbox re-measured this session: score 10, 49 target / 49 build, rules_dropped 0); every form carries exactly candidate.c's one `u8 *q` pointer alias and no other FAKE-family construct; src/ restored to HEAD
- predicate_cite: tools/gcc-2.7.2/cse.c:1905

## [s27] An input other than the address quantity can stop cse forwarding block 0's stored byte into block 1's load, and the original source used it.
- mechanism: cse_insn records a store's MEM destination unless sets[i].src_elt == 0 (cse.c:7328); canon_hash zeroes it via do_not_record for a volatile MEM (cse.c:1943), for CALL / UNSPEC_VOLATILE (cse.c:1967), and for a non-fixed hard register under SMALL_REGISTER_CLASSES (cse.c:1902); invalidate_memory (cse.c:7599) drops the entry on a CALL_INSN or an ambiguous store; and cse's extended-basic-block paths break at code labels. Those five plus the address quantity are the complete input set.
- probe: Each input checked against the TARGET's own instruction stream rather than ours -- asm/funcs/func_80034F88.s between the block-0 sb at 0x80034FAC and the block-1 lbu at 0x80034FB4 -- plus tools/label_census.py --func func_80034F88 (tmp/grind/func_80034F88/s27/label_census.txt), and the .jump/.cse dumps sliced to the function region (tmp/grind/func_80034F88/s27/f88.jump, f88.cse).
- result: KILLED for all five. The dumps show the rewrite literally: .jump insn 29 (set (reg:QI 80) (mem:QI (reg/v:SI 74))) becomes .cse insn 29 (set (reg:QI 80) (subreg:QI (reg:SI 76) 0)) -- cse retargets the load rather than deleting it, and the copy plus its zero_extend fold away downstream. The target's stream between store and reload is sb, lw, lbu: no call (so neither the do_not_record CALL case nor invalidate_memory), no store (no ambiguous-write invalidation), and label_census puts its earliest of four labels (all preds=2) at insn 16, after block 1's compare arms, so no cse path boundary either. SMALL_REGISTER_CLASSES is not defined for MIPS. Volatile is the only one of the five reachable from C and is already measured dead as a family (12/15/28). The address quantity is the sole surviving input.
- verdict: KILLED
- kill_scope: class
- measured_on: target asm asm/funcs/func_80034F88.s plus the s27 dump slices of tmp/grind/func_80034F88/dumps/ on the candidate.c score-10 chassis at HEAD; no FAKE construct present
- predicate_cite: tools/gcc-2.7.2/cse.c:7328

## [s27] The s26 frontier route -- carrying the address in a pointer-typed value derived from `p` or from another live object that reaches 0x80106A73, rather than a second object aliasing D_80106A73.
- mechanism: A second live address value is what the target's multiset needs; if some already-live object contained the byte, a pointer derived from it would supply the second quantity without declaring a second handle.
- probe: The dispatch DATA MODEL and naming census checked for any record covering 0x80106A73; then the one derived spelling that exists, (u8 *)((u8 *)&D_80106A70 + 3), built into eight of the ten s27 forms and measured.
- result: KILLED. The only record covering the byte is 0x80106A70..0x80106A73 itself (three default-colour nibbles plus the flag byte); `p`, the func_80077D00() return, is unrelated memory, and nothing else in the function is live at the byte. &D_80106A70[3] does create a distinct quantity, but it obeys the same cost law -- one full lui+addiu pair -- and additionally carries 2-12 points of 'addiu #,#,3' vs 'addiu #,#,0' distance in the emitted pairs (a2 = 14, a12 = 13, a1/a8 = 22).
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis at HEAD; forms a1/a2/a3/a5/a7/a8/a11/a12 in tmp/grind/func_80034F88/s27/variants/; a single `u8 *q` in every form and no other FAKE-family construct

## [s27] The single `u8 *q` pointer alias in candidate.c is masking a lever (the mandated FAKE-ablation re-audit).
- mechanism: A lever measured inert while a FAKE carrier occupies its target pseudo is not a kill (func_8002EA24 s8), so the ledger's kills had to be re-checked with the candidate's only FAKE-family construct removed.
- probe: tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c, then a hand ablation (a13: all four byte-access groups spelled as direct D_80106A73 accesses, no pointer object at all), scored and classified.
- result: KILLED. fake_ablate.py reports 'no FAKE-annotated constructs found' -- candidate.c's `u8 *q` is un-annotated, so the tool has nothing to strip. The hand ablation measures score 29 at 47 insns with three of the four lbu missing, consistent with the s16 plain-symbol family (28/29) and with s26's CONSTANT_ADDRESS_P finding (mips.h:2369). The alias is worth 19 points and is load-bearing rather than a carrier; nothing is masked behind it.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis at HEAD; ablated body banked at memory/grind/func_80034F88/rejected/pointer-alias-ablated-plain-symbol-47insn-score29.c

## s28 (forensics)

### H28.1 -- KILLED (class) -- bitfield destination reaching cse.c:7004
**Statement.** No bitfield spelling of the flag byte reaches cse.c:7004's
ZERO_EXTRACT/SIGN_EXTRACT SET_DEST gate on this target, so that gate can never be
the input that kept block 1's byte reload alive in the original compilation.
**Mechanism.** cse.c:7004-7027 sets `sets[i].src_elt = 0` (and `src_volatile = 1`)
for a bitfield destination, which makes cse.c:7329 skip `insert (dest, ...)`: the
destination is invalidated but no value is recorded, so a later read of the same
byte is a real load. Reaching it needs a `ZERO_EXTRACT` SET_DEST to survive
expansion, which requires the `insv` expander to succeed.
`tools/gcc-2.7.2/config/mips/mips.md:2901` `FAIL`s unless the field is 32 bits
wide and byte-aligned, so `store_bit_field` falls back to
`store_fixed_bit_field`'s explicit load/and/store RTL.
**Probe.** `b1` (`struct { u8 f5:5; u8 f3:3; }`) and `b1b`
(`struct { u8 f3:3; u8 f5:5; }`), block 0 spelled `((struct FB *)q)->f3 = 0;`.
**Result.** 51 insns / score 13 both; classify still `ours only: nop` /
`target only: lbu #,0(#)`. `grep -c zero_extract` = 0 in `.rtl`, `.jump`, `.cse`.
Block 0's dumped RTL is a plain load / and / store triple
(`tmp/grind/func_80034F88/s28/b1b.jump.block0`).
**kill_scope.** class. **predicate_cite.** `tools/gcc-2.7.2/config/mips/mips.md:2901`.
**measured_on.** candidate.c score-10 chassis at HEAD, no FAKE construct present.

### H28.2 -- KILLED (instance) -- MEM_IN_STRUCT_P asymmetry
**Statement.** Storing through a struct-typed lvalue (`mem/s:QI`) in block 0 and
reading through a plain `u8 *` (`mem:QI`) in block 1 does not stop cse forwarding
the stored value on this chassis.
**Mechanism.** `canon_hash` does not hash `MEM_IN_STRUCT_P` and `exp_equiv_p`
does not compare it, so the two MEMs land in the same class regardless.
**Probe.** `b1b`'s dumped RTL: insn 24 `(set (mem/s:QI (reg 74)) (reg:QI 75))`,
insn 33 `(set (reg:QI 81) (mem:QI (reg 74)))`.
**Result.** Reload still deleted (ours-only nop / target-only lbu) at 51 insns.
**kill_scope.** instance. **measured_on.** b1b bitfield chassis on HEAD, no FAKE
construct present.

### H28.3 -- CONFIRMED -- the CALL gate restores the reload, at +2 instructions
**Statement.** Placing a CALL_INSN between block 0's store and block 1's read
restores the target's fourth `lbu`, but costs two instructions to keep the
pointer alive across the call.
**Mechanism.** `invalidate_memory` (cse.c:7599) drops every non-`RTX_UNCHANGING_P`
memory entry at a CALL_INSN, so the store's `(mem:QI (reg 74))` entry is gone by
the time block 1 reads.
**Probe.** `b2` -- `q = &D_80106A73; *q &= 0xF8; p = func_80077D00();` then blocks
1-3 unchanged.
**Result.** 51 insns, score 29; classify's only shape difference is an extra
`lw #,0x20(#)` / `sw #,0x20(#)` pair, i.e. the lbu multiset matches. The target's
own stream has no call between 0x80034FAC and 0x80034FB4, so this was not the
original's input. Banked at
`rejected/s28-block0-rmw-before-call-invalidates-51insn-score29.c`.

### H28.4 -- CONFIRMED (kill re-audit) -- the dead round-trip's gate is reg_tick
**Statement.** The s20/s25 dead pointer round-trip still measures 49 insns /
score 10 on the current HEAD chassis, with a MATCHING instruction multiset and an
RA-only residual, and its cse mechanism is the `reg_tick` bump in `invalidate`.
**Mechanism.** `q = q + 3; q = q - 3;` emits two real SETs of the address pseudo.
`cse_insn` calls `invalidate` on reg 74, which does `reg_tick[regno]++` at
cse.c:1539; every table entry mentioning reg 74 -- including the `(mem:QI (reg 74))`
recorded for the store -- stops validating, so block 1's load survives. cse folds
the arithmetic itself (REG_EQUAL `D_80106A73 + 3`, then `D_80106A73`), so the
round-trip costs zero instructions.
**Probe.** `b4` = `rejected/s25-roundtrip-before-b1-reconfirms-s20-score10-DEAD-ARITH.c`,
re-measured, plus fresh `-da` dumps sliced to `tmp/grind/func_80034F88/s28/b4.jump`
and `b4.cse`.
**Result.** score 10 / 49 insns; classify FIRST DIVERGENCE: **RA**, `ours: lbu
v1,0(a0)` x2 vs `target: lbu a0,0(v1)` x2 -- the whole residual is the $v1/$a0
seat swap that s26's RA inverse FORECLOSES on this exact chassis. The construct
remains inadmissible (dead arithmetic with no semantic purpose: fails T1, T2 and
T6 of the cheat checklist), so the kill stands; only its mechanism is upgraded
from inferred to RTL-proven.

### H28.5 -- CONFIRMED (cross-function toolchain fact) -- bitfield direction under -mel
**Statement.** On the live toolchain the FIRST-declared bitfield occupies the LOW
bits of its storage unit, the opposite of what
`.claude/rules/bitfield-direction-divergence.md` (2026-06-11) records.
**Probe.** `b1` `struct { u8 f5:5; u8 f3:3; }` with `f3 = 0` emits
`andi #,#,0x1f`; `b1b` `struct { u8 f3:3; u8 f5:5; }` with `f3 = 0` emits
`andi #,#,0xf8`.
**Result.** CONFIRMED. The rule predates the 2026-08-04 `-mel` adoption, which
flips `BYTES_BIG_ENDIAN` and therefore the bitfield allocation direction; its
"declare SDK bitfield structs in FLIPPED field order" advice is stale and would
now produce the wrong layout. Not acted on -- rule files are outside a grind
session's editable surface.

## [s28] No bitfield spelling of the flag byte reaches cse.c:7004's ZERO_EXTRACT/SIGN_EXTRACT SET_DEST gate on this target, so that gate cannot be the cse input that kept block 1's byte reload alive in the original compilation.
- mechanism: cse.c:7004-7027 sets sets[i].src_elt = 0 and src_volatile = 1 for a bitfield destination, which makes cse.c:7329 skip insert (dest, ...): the destination is invalidated but no value is recorded, so a later read of the same byte is a real load. Reaching it requires a ZERO_EXTRACT SET_DEST to survive expansion, which requires the insv expander to succeed; mips.md:2901 FAILs unless the field is 32 bits wide and byte-aligned, so store_bit_field falls back to store_fixed_bit_field's explicit load/and/store RTL.
- probe: Variants b1 (struct { u8 f5:5; u8 f3:3; }) and b1b (struct { u8 f3:3; u8 f5:5; }) spell block 0 as ((struct FB *)q)->f3 = 0; sandbox + inverse_compose.py classify on each, plus fresh -da dumps and grep -c zero_extract over .rtl/.jump/.cse.
- result: Both 51 insns, score 13; classify still reports 'ours only: nop' / 'target only: lbu #,0(#)'. zero_extract count is 0/0/0 in .rtl, .jump and .cse. The dumped block 0 is a plain load/and/store triple (insn 17 (set (reg:QI 75) (mem/s:QI (reg 74))), insn 20 and 248, insn 22, insn 24 (set (mem/s:QI (reg 74)) (reg:QI 75))). Banked at memory/grind/func_80034F88/rejected/s28-bitfield-block0-store-no-zero-extract-51insn-score13.c.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c score-10 single-object chassis at HEAD (src/code6cac_b.c:3420 INCLUDE_ASM replaced by the body), no FAKE-annotated construct present
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.md:2901

## [s28] Storing through a struct-typed lvalue (mem/s:QI) in block 0 while reading through a plain u8 * (mem:QI) in block 1 does not stop cse forwarding the stored value on this chassis.
- mechanism: canon_hash does not hash MEM_IN_STRUCT_P and exp_equiv_p does not compare it, so the two MEMs land in the same equivalence class regardless of the flag.
- probe: b1b's dumped RTL: insn 24 (set (mem/s:QI (reg 74)) (reg:QI 75)) as the store and insn 33 (set (reg:QI 81) (mem:QI (reg 74))) as the reload; measured emitted stream compared with classify.
- result: Reload still deleted -- 'ours only: nop' / 'target only: lbu' at 51 insns, score 13. The MEM flag asymmetry is not a cse-defeating input.
- verdict: KILLED
- kill_scope: instance
- measured_on: b1b bitfield chassis on HEAD (candidate.c body with block 0 respelled as a 3-bit bitfield store), no FAKE-annotated construct present

## [s28] Placing a CALL_INSN between block 0's store and block 1's read restores the target's fourth lbu via cse.c:7599 invalidate_memory, at a cost of two extra instructions.
- mechanism: invalidate_memory drops every non-RTX_UNCHANGING_P memory entry at a CALL_INSN, so the store's (mem:QI (reg 74)) table entry is gone by the time block 1 reads; but the pointer then has to live across the call, which buys an lw/sw pair.
- probe: Variant b2: q = &D_80106A73; *q &= 0xF8; p = func_80077D00(); with blocks 1-3 unchanged. Sandbox + classify.
- result: 51 insns, score 29. classify's only remaining shape difference is an extra 'lw #,0x20(#)' / 'sw #,0x20(#)' pair -- the lbu multiset matches the target, so the gate genuinely fires. Price is +2, identical to a fourth address materialisation (a3/a4/a11/a12, 51 insns). The target's own stream has no call between its sb at 0x80034FAC and its lbu at 0x80034FB4, so this was not the original's input. Banked at memory/grind/func_80034F88/rejected/s28-block0-rmw-before-call-invalidates-51insn-score29.c.
- verdict: CONFIRMED

## [s28] MANDATED KILL RE-AUDIT: the s20/s25 dead pointer round-trip still measures 49 insns / score 10 on the current HEAD chassis with a MATCHING instruction multiset and an RA-only residual, and its cse mechanism is the reg_tick bump inside invalidate().
- mechanism: q = q + 3; q = q - 3; emits two real SETs of the address pseudo. cse_insn calls invalidate on reg 74, which does reg_tick[regno]++ at cse.c:1539; every table entry whose expression mentions reg 74 -- including the (mem:QI (reg 74)) recorded for block 0's store -- stops validating, so block 1's load survives cse. cse folds the arithmetic itself (REG_EQUAL notes D_80106A73 + 3 then D_80106A73), which is why the round-trip costs zero instructions.
- probe: Variant b4 (= rejected/s25-roundtrip-before-b1-reconfirms-s20-score10-DEAD-ARITH.c) re-installed and re-measured on HEAD, with fresh pwsh tools/grinder/dump.ps1 dumps sliced to tmp/grind/func_80034F88/s28/b4.jump and b4.cse. FAKE state: candidate.c carries no /* FAKE */ construct (s27's fake_ablate.py run reported none), and the alias itself was ablated in s27, so no FAKE carrier occupied the pseudo during this measurement.
- result: score 10, 49 insns; classify FIRST DIVERGENCE: RA with 'ours: lbu v1,0(a0)' x2 vs 'target: lbu a0,0(v1)' x2 -- the entire residual is the $v1/$a0 seat swap that s26's RA inverse FORECLOSES on this exact chassis. The RTL confirms the mechanism: .jump insn 35 (set (reg:QI 80) (mem:QI (reg 74))) survives .cse unchanged, whereas on candidate.c the same insn becomes (set (reg:QI 80) (subreg:QI (reg:SI 76) 0)). The construct remains inadmissible (dead arithmetic with no semantic purpose -- fails cheat-checklist T1, T2 and T6), so the standing instance kill is unchanged; only its mechanism is upgraded from inferred to RTL-proven.
- verdict: CONFIRMED

## [s28] On the live toolchain the FIRST-declared bitfield occupies the LOW bits of its storage unit, which is the opposite of what .claude/rules/bitfield-direction-divergence.md records.
- mechanism: The 2026-08-04 -mel adoption flips BYTES_BIG_ENDIAN in cc1, and GCC 2.7.2 derives bitfield allocation direction from it. The rule is dated 2026-06-11 and predates that change.
- probe: b1 (struct { u8 f5:5; u8 f3:3; }) and b1b (struct { u8 f3:3; u8 f5:5; }), both assigning f3 = 0, compared in the emitted stream.
- result: b1 emits andi #,#,0x1f (f5 occupies bits 0-4); b1b emits andi #,#,0xf8 (f3 occupies bits 0-2). First-declared takes the LOW bits. The rule's 'declare SDK bitfield structs in FLIPPED field order' advice is stale post--mel and would now produce the wrong layout. Recorded in the ledger only -- rule files are outside a grind session's editable surface.
- verdict: CONFIRMED


## s29 (rederive)

### H29.1 -- KILLED (instance) -- a second address allocno on a DIFFERENT global

- statement: A second live address allocno that does not alias the flag byte --
  the func_80035280 idiom `u8 *r = q - 3;` (i.e. &D_80106A70), held live across
  the flag blocks and used to spell the copy loop -- reseats blocks 0-1 to the
  target's base=$v1 / value=$a0 convention.
- mechanism: global.c allocates allocnos in priority order; adding a second
  long-lived address allocno changes the conflict graph and the order in which
  $v1 and $a0 are handed out, which is the whole 7-instruction seat residual.
  The idiom is not invented: the inverse function in this TU
  (asm/funcs/func_80035280.s:9, `addiu $a2,$a1,-0x3`) literally derives its
  D_80106A70 pointer from its D_80106A73 pointer this way, so it is ordinary C
  with in-repo provenance.
- probe: v1 (`r` derived at the top, loop spelled `r[i] = ...`) and v2 (`r`
  derived immediately before the loop), built and objdumped;
  tmp/grind/func_80034F88/s29/v1_derived_r_loop.c, v2_derived_r_late.c.
- result: KILLED. v1 = score 19 at 50 insns, v2 = 30 at 48. The allocno is
  created (r is seated in $a2 and the loop's `lui $at,%hi(D_80106A70)` /
  `addu $at,$at,$v1` re-materialisation collapses to an `addu` on r), but the
  first sixteen instructions of v1 are BYTE-FOR-BYTE the base chassis's:
  `lui $a0` / `addiu $a0` / `lbu $v1,0($a0)` / `sb $v1,0($a0)`. The contested
  seat is invariant to address allocnos that do not alias the flag byte, so the
  second allocno the target needs must be a second handle ON 0x80106A73.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD, single `u8 *q` plus one
  derived `u8 *r`; no FAKE-annotated construct present.

### H29.2 -- KILLED (instance) -- re-materialisation moved before the store

- statement: Moving the `q = &D_80106A73;` re-assignment from the head of block
  2 (and of block 3) into the preceding block, between the value select and the
  store, reproduces the target's join-block emission order (`lui`/`addiu` then
  `sb`).
- mechanism: the target emits pair #2 at .L80034FC8, i.e. BEFORE block 1's
  `sb $v0,0($v1)`; s26b showed no scheduler vector reaches that order while one
  pseudo carries the address (REG_DEP_ANTI, tools/gcc-2.7.2/sched.c:1738).
  Placing the SET earlier in the source is the only remaining input to that
  ordering that is not a scheduler perturbation.
- probe: v3 (move into block 1) and v4 (move into blocks 1 and 2),
  tmp/grind/func_80034F88/s29/v3_remat_inside_block1.c,
  v4_remat_inside_b1_b2.c.
- result: KILLED. v3 = 21 at 51 insns -- the set now sits after a branch join,
  where cse no longer unifies it away into the following block, so a FOURTH
  lui/addiu pair appears (the s27 cost law again). v4 = 22 at 49 insns, with the
  join order still `sb` then `lui`/`addiu`, i.e. ours. Source position does not
  move the anti-dependence.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD; single `u8 *q`, three
  assignments, no FAKE-annotated construct present.

### H29.3 -- CONFIRMED (mandated kill re-audit + a pricing correction)

- statement: The s20/s25 dead round-trip form -- the only banked body whose
  instruction multiset matches the target -- still measures score 10 at 49 insns
  on the current HEAD chassis, and restoring block 1's byte reload is worth ZERO
  score points.
- mechanism: the re-audit rule (a lever measured inert while a carrier occupies
  its pseudo is not a kill) plus a first full objdump alignment of the
  round-trip body against the target.
- probe: rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c
  re-installed and measured; objdump of the resulting object aligned against
  asm/funcs/func_80034F88.s.
- result: CONFIRMED. Score 10 at 49 insns, unchanged. With the reload present
  (`lbu $v1,0($a0)` at the target's `lbu $a0,0($v1)` position) the ten differing
  positions are: the two address-materialisation insns, the two byte loads, the
  block-0 store, the `ori`, the `addu`, and the three-instruction rotation of
  `lui`/`addiu`/`sb` at the block-1 join -- every one of them a register-naming
  or ordering consequence of the single missing address allocno. This corrects
  the s25/s28 pricing, which carried the reload at roughly 2 of the 10 points.
- verdict: CONFIRMED

### H29.4 -- CONFIRMED -- func_80035280 fixes the original's object model

- statement: The inverse function in this TU, func_80035280, holds D_80106A73
  through ONE named `u8 *` local and derives its D_80106A70 pointer from it by
  pointer arithmetic; its surviving reloads are explained by intervening stores,
  not by any input the F88 target stream contains.
- mechanism: cse.c:7599 invalidate_memory fires on the `sw $v0,0x20($t0)` that
  35280 executes between its byte reads; F88's target has `sb`, `lw`, `lbu`
  between its store and its reload and therefore no invalidation. 35280 is the
  positive control for the s27/s28 cse input enumeration.
- probe: asm/funcs/func_80035280.s read in full (never examined in 28 sessions,
  despite the dispatch DATA MODEL naming it as an xref of D_80106A73).
- result: CONFIRMED. 0x80035294/98 materialise &D_80106A73 into $a1 once;
  0x8003529C is `addiu $a2,$a1,-0x3`, the D_80106A70 pointer derived from it;
  three `lbu 0($a1)` reads survive at that one pointer. Together with sibling
  func_80034708's single long-lived `s5`, this establishes named `u8 *` locals
  as the code family's idiom for this byte and closes off the plain-symbol
  reading of F88's original for good. It also sharpens what F88's THREE address
  materialisations mean: they partition its four byte-access groups as
  {mask + bit1} / {bit2} / {bit4}, exactly the partition three separately-scoped
  pointer locals produce -- and exactly the partition of the three-object body
  that measured score 0 in s13-s16 and that the standing ban forecloses.
- verdict: CONFIRMED

## [s29] A second live address allocno that does not alias the flag byte -- the func_80035280 idiom `u8 *r = q - 3;` (&D_80106A70), held live across the flag blocks and used to spell the copy loop -- reseats blocks 0-1 to the target's base=$v1 / value=$a0 convention.
- mechanism: global.c allocates allocnos in priority order, so a second long-lived address allocno changes the conflict graph and the order in which $v1 and $a0 are handed out -- which is the entire 7-instruction seat residual. The idiom is not invented: asm/funcs/func_80035280.s:9 (`addiu $a2,$a1,-0x3`) literally derives the D_80106A70 pointer from the D_80106A73 pointer, so it is ordinary C with in-repo provenance.
- probe: v1 (r derived at the top, loop spelled `r[i] = ...`) and v2 (r derived immediately before the loop) built and objdumped: tmp/grind/func_80034F88/s29/v1_derived_r_loop.c, v2_derived_r_late.c.
- result: KILLED. v1 = score 19 at 50 insns; v2 = score 30 at 48 insns. The second allocno is genuinely created (r is seated in $a2 and the loop's `lui $at,%hi(D_80106A70)` / `addu $at,$at,$v1` re-materialisation collapses), but v1's first sixteen instructions are byte-for-byte the base chassis's -- `lui $a0` / `addiu $a0` / `lbu $v1,0($a0)` / `sb $v1,0($a0)`. The contested seat is invariant to address allocnos that do not alias 0x80106A73.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD (score 10, 49/49, rules_dropped 0), single `u8 *q` plus one derived `u8 *r`; no FAKE-annotated construct present

## [s29] Moving the `q = &D_80106A73;` re-assignment from the head of block 2 (and of block 3) into the preceding block, between the value select and the store, reproduces the target's join-block emission order (lui/addiu then sb).
- mechanism: The target emits address pair #2 at .L80034FC8, before block 1's `sb $v0,0($v1)`. s26b showed no scheduler vector reaches that order while one pseudo carries the address (REG_DEP_ANTI, tools/gcc-2.7.2/sched.c:1738); moving the SET earlier in the source is the remaining non-scheduler input to that ordering.
- probe: v3 (move into block 1) and v4 (move into blocks 1 and 2): tmp/grind/func_80034F88/s29/v3_remat_inside_block1.c, v4_remat_inside_b1_b2.c.
- result: KILLED. v3 = score 21 at 51 insns -- the set now sits after a branch join, cse no longer unifies it into the following block, and a fourth lui/addiu pair appears (the s27 cost law). v4 = score 22 at 49 insns with the join order still `sb` then `lui`/`addiu`, i.e. ours. Source position does not move the anti-dependence.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD; single `u8 *q` with three assignments, no FAKE-annotated construct present

## [s29] MANDATED KILL RE-AUDIT: the s20/s25 dead round-trip form -- the only banked body whose instruction multiset matches the target -- still measures score 10 at 49 insns on the current HEAD chassis, and restoring block 1's byte reload is worth ZERO score points.
- mechanism: A lever measured inert while a carrier occupies its pseudo is not a kill, so the closest-to-target instance kill was re-installed and re-measured, then aligned instruction by instruction against asm/funcs/func_80034F88.s for the first time.
- probe: memory/grind/func_80034F88/rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c re-installed at src/code6cac_b.c:3420, `sandbox func_80034F88 --disable all`, then mipsel-linux-gnu-objdump of tmp/sandbox/func_80034F88/code6cac_b.o.
- result: CONFIRMED (kill holds). Score 10 at 49 insns, unchanged. With the reload present -- `lbu $v1,0($a0)` sitting exactly where the target has `lbu $a0,0($v1)` -- the ten differing positions are the two address-materialisation insns, the two byte loads, the block-0 store, the `ori`, the `addu`, and the three-instruction rotation of lui/addiu/sb at the block-1 join. Every one is a register-naming or ordering consequence of the single missing address allocno, which corrects the s25/s28 pricing that carried the reload at ~2 of the 10 points.
- verdict: CONFIRMED

## [s29] func_80035280, this function's inverse in the same TU (unread in 28 sessions despite being a listed D_80106A73 xref), holds the flag byte through ONE named u8* local and derives its D_80106A70 pointer from it by pointer arithmetic; its surviving reloads are explained by intervening stores, not by any input F88's target stream contains.
- mechanism: cse.c:7599 invalidate_memory fires on the `sw $v0,0x20($t0)` that func_80035280 executes between its byte reads. F88's target has `sb`, `lw`, `lbu` between its block-0 store and its block-1 reload -- no store, no invalidation. func_80035280 is therefore the positive control for the s27/s28 cse input enumeration.
- probe: asm/funcs/func_80035280.s read in full and aligned against asm/funcs/func_80034F88.s.
- result: CONFIRMED. 0x80035294/98 materialise &D_80106A73 into $a1 once; 0x8003529C is `addiu $a2,$a1,-0x3`, the D_80106A70 pointer derived from it and walked by its copy loop; three `lbu 0($a1)` reads survive on that one pointer. With sibling func_80034708's single long-lived `s5`, named u8* locals are established as this code family's idiom for the byte, closing the plain-symbol reading of the original. It also sharpens what F88's three address materialisations mean: they partition the four byte-access groups as {mask + bit1} / {bit2} / {bit4}, exactly the partition three separately-scoped pointer locals produce -- and exactly the partition of the score-0 three-object body the standing ban forecloses.
- verdict: CONFIRMED


==== s30 (rederive) ====

H-s30-1. The CD_sync F1 combine-foldable chain-extender (owner ruling
2026-07-01), assigned to this function's SINGLE pointer object `q`, supplies the
missing address allocno -- or at least a reg_n_refs perturbation that flips the
blocks-0/1 seat -- at zero instruction cost.
  PROBE: `q = (u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70));`
  in three placements (all three assignments / first only / blocks 2+3 only),
  measured on the candidate.c score-10 chassis on HEAD.
  RESULT: all three score 14 at 49 build insns. Dumps show the difference is
  folded away before RA: .combine holds three plain
  `(set (reg) (symbol_ref "D_80106A73"))`, .greg puts all three in
  `(reg/v:SI 4 a0)` (one allocno, as base), and `diff base.s a2.s` is four
  frame lines only -- prologue/epilogue 24 -> 32 bytes. The body is
  BIT-IDENTICAL to candidate.c; the entire +4 is a phantom frame slot for the
  folded intermediate.
  VERDICT: KILLED (instance). The lever is byte-neutral in the body here, so it
  cannot reach a body defect.

H-s30-2. A do-while(0) wrap spanning TWO adjacent flag blocks (a placement the
bank never held) reaches the join rotation that a single-block wrap cannot.
  PROBE: mask+block1 jointly; block2+block3 jointly; all three jointly.
  RESULT: 12 at 50 insns / 10 at 49 insns (inert) / 12 at 50 insns.
  VERDICT: KILLED (instance). Any wrap crossing the mask store costs an
  instruction; the wrap confined to the already-exact region is a no-op.

H-s30-3. m2c's inverted diamond (compute the positive value, overwrite in the
negated arm), never crossed with the pointer chassis, restores block 1's reload
because it reads the byte twice in C.
  PROBE: fresh m2c decompile this session; its shape re-spelled on the `q`
  chassis with an s32 temp and with m2c's u8 temp.
  RESULT: both 21 at 45 build insns -- the select collapses and four
  instructions are LOST, moving away from the target's 49.
  VERDICT: KILLED (instance). cse folds the duplicated `*q` read across the
  arms, so the C-level second read never becomes a second lbu.

H-s30-4 (mandated kill re-audit). The chassis and the closest banked forms are
re-measured on HEAD: candidate.c is 10/49/rules_dropped 0, and the blocks-2/3
do-while wrap ties it with the same stream. candidate.c carries no
/* FAKE */-annotated construct, so the ablation arm remains the s27 hand
ablation (banked at 29). The floor of 10 holds.
  VERDICT: CONFIRMED.

## [s30] CD_sync's FAKE-annotated F1 combine-foldable chain-extender, assigned to this function's single pointer object q (no second pointer object, so outside the standing multi-handle ban), supplies the missing address allocno or at least a reg_n_refs perturbation that flips the blocks-0/1 seat at zero instruction cost.
- mechanism: CD_sync's candidate.c claims flow.c records the extra reg_n_refs of the SYMBOL_REF-difference chain before combine.c folds it, giving an allocation change with zero emitted bytes. Transplanted here the fold is total: .combine holds three plain (set (reg) (symbol_ref "D_80106A73")) insns and .greg assigns all three to (reg/v:SI 4 a0) -- one hard reg, one allocno, exactly as the base body (global.c:426 untouched).
- probe: q = (u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)); in three placements (all three assignments / first assignment only / blocks 2+3 only), measured on the candidate.c score-10 chassis on HEAD; then pwsh tools/grinder/dump.ps1 func_80034F88 on the first-only body and a line diff of the emitted assembly against the base body.
- result: All three placements score 14 at 49 build insns. The emitted function bodies are BIT-IDENTICAL to candidate.c: diff tmp/grind/func_80034F88/s30/base.s tmp/grind/func_80034F88/s30/a2.s is four lines, all four frame -- subu $sp,$sp,24 -> 32, sw $31,16($sp) -> 24($sp), and the matching epilogue pair. The whole +4 is a phantom frame slot reserved for the folded-away intermediate (project/phantom-frame-slots-gcc272). The reg_n_refs perturbation that is load-bearing in CD_sync is inert on this chassis, and this closes the sibling-inheritance axis completely (func_80034708 s25, func_80035280 s29, CD_sync/CD_datasync s30).
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD (score 10, 49/49, rules_dropped 0), single u8 *q with three assignments; the chain-extender is the only added construct and no other FAKE-annotated construct is present in the body

## [s30] A do-while(0) wrap spanning TWO adjacent flag blocks -- a placement the 159-form bank never held -- reaches the block-1 join rotation that the already-banked single-block wraps could not.
- mechanism: The do-while(0) family is sanctioned for any codegen effect including register allocation (owner ruling 2026-07-06). A wrap over a two-block span changes where jump.c/reorg.c place the join label relative to the address re-materialisation, which is the 3-insn rotation s29 priced as the second half of the residual.
- probe: Three joint wraps built from the candidate.c body and measured on HEAD: the mask statement plus block 1; block 2 plus block 3; all three blocks together.
- result: mask+block1 = score 12 at 50 build insns; block2+block3 = score 10 at 49 (an exact no-op, the region is already byte-exact); all three = score 12 at 50. Every wrap that spans the mask store costs an instruction against the target's 49. The sanctioned do-while(0) family is now measured in every placement on this chassis (block 0 = 15, block 1 = 10 with the reload lost, per-block = 26, boundary = 23, and the three joint spans here).
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD; single u8 *q with three assignments; the do-while(0) wrap is the only added construct, no other FAKE-annotated construct present

## [s30] m2c's inverted diamond (compute the positive value first, overwrite it in the negated arm), which the bank holds only on the plain-symbol chassis, restores block 1's missing lbu when crossed with the pointer chassis, because the C reads the byte twice.
- mechanism: The fresh m2c decompile emits var = D_80106A73 | 1; if (!(p[8] & 1)) var = D_80106A73; D_80106A73 = var; -- two syntactic reads of the byte per block instead of the one read our select chassis uses. The residual's visible symptom is one missing lbu (build lbu census 175 against the target's 176).
- probe: python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f func_80034F88 run fresh this session, its shape re-spelled on the q chassis with the three q re-assignments intact, in both an s32 temp and m2c's own u8 temp.
- result: Both spellings score 21 at 45 build insns. The diamond COLLAPSES: four instructions are lost against the target's 49, because cse forwards the duplicated *q read across the arms and folds the select into a single arm. The C-level second read never becomes a second lbu. Moves away from the target, not toward it.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD, single u8 *q with three assignments, no FAKE-annotated construct present

## [s30] Mandated kill re-audit: the floor of 10 and the closest banked forms still measure as the ledger records them on the current HEAD chassis.
- mechanism: Instance kills are chassis-relative and FAKE-state-relative; the dispatch brief printed 'measurement unavailable' for the chassis check, so the floor had to be re-established by direct measurement rather than inherited from the ledger.
- probe: candidate.c re-installed at src/code6cac_b.c:3420 and measured with sandbox func_80034F88 --disable all; the tying blocks-2/3 do-while wrap measured on the same chassis; fake_ablate has nothing to strip because candidate.c carries no /* FAKE */-annotated construct (the ablation arm remains s27's hand ablation, banked at 29).
- result: score 10, target_insns 49, build_insns 49, rules_dropped 0 -- the floor of 10 is confirmed by direct measurement for the second consecutive session, and the tying form emits the same stream.
- verdict: CONFIRMED


==== s31 (structural) ====

H-s31-1. Block-local variable splits (a named intermediate for the mask value,
a separate local for the flag word, or both) add pseudos that survive to
register allocation and perturb the allocno set that seats the address object.
  PROBE: v5 (mask intermediate `s32 m`), v6 (flag-word local `s32 f`), v7 (both),
  measured on the candidate.c score-10 chassis on HEAD, with .greg + .s dumps
  per variant (pwsh tools/grinder/dump.ps1 func_80034F88).
  RESULT: all three score 10 at 49 insns and `diff base.s vN.s` is EMPTY. The
  .greg allocator input is character-identical to base -- same nine allocnos
  (73 78 82 86 74 77 81 85 72), same conflict graph, same preferences, same
  dispositions with `74 in 4` ($a0). The added C objects create no allocno.
  VERDICT: KILLED (instance).

H-s31-2. Re-associating the mask so that its value IS block 1's value (dropping
the C-level reload) changes the emitted stream.
  PROBE: v1 (`v = *q & 0xF8; *q = v; ... c = v | 1;`) and v2 (`v = *q;` moved
  before `c = p[8] & K;` in all three blocks), same chassis.
  RESULT: both 10 at 49 insns with an EMPTY asm diff against base. cse already
  forwards the mask value into block 1 on the base body, so the C-level change
  has no RTL to change.
  VERDICT: KILLED (instance).

H-s31-3. Collapsing the three block-scoped `v`/`c` pairs into one function-scope
pair changes the allocator's input enough to re-seat the address object.
  PROBE: v8 (one shared `s32 v, c;`, mask value carried) and v9 (same with the
  classic `*q &= 0xF8;` mask), same chassis.
  RESULT: both 10 at 49 insns, asm BIT-IDENTICAL to base. v8's .greg shows the
  global allocno count drop from NINE to FIVE (`;; 5 regs to allocate:
  73 76 75 74 72`) with a different conflict graph and allocation order -- and
  `74 in 4` ($a0) regardless.
  VERDICT: KILLED (instance).

H-s31-4. The address object is excluded from $v1 because $v0's liveness across
block 0 (the call's return copy is scheduled after the mask sequence) pushes the
block-0 value quantity into $v1 in local-alloc; giving `p` a use before the mask
frees $v0, releases $v1, and lets the address allocno take it.
  PROBE: v11 (block 1's `p[8]` read hoisted above the mask statement), same
  chassis, .greg compared against base.
  RESULT: 14 at 49 insns. $v0 IS freed for the block-0 value (`75 in 2`,
  `76 preferences: 3`), but a different local-alloc pseudo takes $v1 (`77 in 3`),
  `74 conflicts: 72 74 75 76 81 82 85 86 2 3 29` is unchanged, and 74 is still
  in $a0. The hoist costs 4 points on its own.
  VERDICT: KILLED (instance).

H-s31-5. Type narrowing of the OTHER object (`p` as `u8 *` with the flag word
read as `*(s32 *)(p + 0x20)` and the copy loop as `p[i + 0x17]`) reaches the
target's addressing without touching the alias axis.
  PROBE: v3, same chassis.  RESULT: 12 at 49 insns.
  VERDICT: KILLED (instance).

H-s31-6. An init-then-conditional-or spelling (`c = *q; if (p[8] & K) c |= K;
*q = c;`), i.e. an empty else arm, reproduces the target's join-and-store shape.
  PROBE: v4, same chassis.  RESULT: 30 at 44 insns -- five instructions are lost;
  the empty arm collapses the diamond, confirming s1's reading that the store is
  unconditional in the target.
  VERDICT: KILLED (instance).

H-s31-7 (mandated kill re-audit). The chassis and the closest banked form are
re-measured, and the FAKE state of the floor body is checked mechanically.
  PROBE: candidate.c installed on HEAD; rejected/roundtrip-fresh-pseudo-target-
  census-score10-DEAD-ARITH.c (the only banked body whose instruction multiset
  matches the target) re-measured; `tools/fake_ablate.py` run on candidate.c.
  RESULT: candidate.c 10 at 49 insns, rules_dropped 0; the round-trip form 10 at
  49 insns; fake_ablate reports "no FAKE-annotated constructs found ... nothing
  to ablate".
  VERDICT: CONFIRMED.

H-s31-8. The residual can be stated one level below "one missing address
allocno", as an allocator INPUT fact, and s23's reading of the preference lever
is exact.
  PROBE: read the `;; NN conflicts` / `;; NN preferences` headers of the .greg
  dumps for all eleven bodies and `set_preference` in tools/gcc-2.7.2/global.c.
  RESULT: allocno 74 conflicts with HARD registers 2 ($v0) and 3 ($v1) in every
  body -- it is ineligible for $v1 before find_reg runs, because a local-alloc
  pseudo (the block-0 QImode mask value, s24's blk=0 qty=0 row) is seated in $v1
  across its range and local-alloc runs first. Separately, s23's claim that
  set_preference "can never record a preference for either [$v1 or $a0], for any
  pseudo" is FALSE on this chassis: pseudo 77 carries `preferences: 3` and
  78/82/86 carry `preferences: 2`, produced by global.c:1709-1713, which maps a
  copy operand through `reg_renumber` to a hard register once local-alloc has
  seated it. The correct statement is narrower: allocno 74 is never a copy
  operand (it is set from a bare symbol_ref), so no preference can be recorded
  for it; the only copy source that would carry one is an object local-alloc has
  seated in $v1, i.e. a second C object aliasing the byte.
  VERDICT: CONFIRMED.

## [s31] Block-local variable splits (a named intermediate for the mask value, a separate local for the flag word, or both) add pseudos that survive to register allocation and perturb the allocno set that seats the address object.
- mechanism: A fresh once-written once-read local is expanded as its own pseudo; if it survives cse/combine it becomes a new allocno and changes the conflict graph and allocation order that global.c find_reg walks.
- probe: v5 (mask intermediate `s32 m`), v6 (flag-word local `s32 f`), v7 (both) installed at src/code6cac_b.c:3420 and measured with `sandbox func_80034F88 --disable all`; .greg and .s dumps captured per variant with `pwsh tools/grinder/dump.ps1 func_80034F88`.
- result: All three score 10 at 49 build insns and `diff base.s vN.s` is EMPTY. The .greg allocator input is character-identical to base: the same nine allocnos (73 78 82 86 74 77 81 85 72), the same conflict graph, the same preference lines and `74 in 4` ($a0). The added C objects create no allocno.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD (score 10, 49/49, rules_dropped 0), single `u8 *q` with three assignments, no FAKE-annotated construct present

## [s31] Re-associating the mask so that its value IS block 1's value (dropping the C-level reload) changes the emitted stream.
- mechanism: cse.c forwards the stored mask value into block 1 on the base body already, so the C-level reload is not an RTL reload; removing it should be observable only if the forwarding had not happened.
- probe: v1 (`v = *q & 0xF8; *q = v; ... c = v | 1;`) and v2 (`v = *q;` moved before `c = p[8] & K;` in all three blocks), same chassis, with .s dumps.
- result: Both 10 at 49 insns with an EMPTY asm diff against base -- the C change has no RTL to change.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD, single `u8 *q` with three assignments, no FAKE-annotated construct present

## [s31] Collapsing the three block-scoped v/c pairs into one function-scope pair changes the allocator's input enough to re-seat the address object.
- mechanism: Sharing one value variable across all three flag blocks merges live ranges, which changes allocno count, conflicts and the priority order find_reg processes.
- probe: v8 (one shared `s32 v, c;`, mask value carried) and v9 (same with the classic `*q &= 0xF8;` mask), same chassis, .greg + .s dumps for v8.
- result: Both 10 at 49 insns and v8's asm is BIT-IDENTICAL to base. v8's .greg drops the global allocno count from NINE to FIVE (`;; 5 regs to allocate: 73 76 75 74 72`), a different conflict graph and a different allocation order -- and still prints `74 in 4` ($a0).
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD, single `u8 *q` with three assignments, no FAKE-annotated construct present

## [s31] The address object is excluded from $v1 because $v0 is live across block 0 (the call's return copy is scheduled after the mask sequence), pushing the block-0 value quantity into $v1 in local-alloc; giving p a use before the mask frees $v0, releases $v1, and lets the address allocno take it.
- mechanism: local-alloc runs before global-alloc and seats its quantities with find_free_reg in reg_alloc_order; a seated local pseudo becomes a hard-reg conflict for every overlapping global allocno.
- probe: v11 (block 1's `p[8]` read hoisted above the mask statement), same chassis, .greg compared against base.
- result: 14 at 49 insns. $v0 IS freed for the block-0 value (`75 in 2`, `76 preferences: 3`), but a different local-alloc pseudo takes $v1 (`77 in 3`); `74 conflicts: 72 74 75 76 81 82 85 86 2 3 29` is unchanged and 74 is still in $a0. The hoist costs 4 points on its own.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD with block 1's flag-word read hoisted above the mask; single `u8 *q` with three assignments, no FAKE-annotated construct present

## [s31] Type narrowing of the OTHER object -- p declared `u8 *`, the flag word read as *(s32 *)(p + 0x20), the copy loop as p[i + 0x17] -- reaches the target's addressing without touching the alias axis.
- mechanism: Narrowing the pointer type changes the MEM addressing forms cse sees and could change which quantity carries the base register.
- probe: v3, same chassis, `sandbox func_80034F88 --disable all`.
- result: 12 at 49 insns -- two points worse than base.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD with p narrowed to u8 *; single `u8 *q` with three assignments, no FAKE-annotated construct present

## [s31] An init-then-conditional-or spelling (c = *q; if (p[8] & K) c |= K; *q = c;) with an empty else arm reproduces the target's join-and-store shape.
- mechanism: The target emits the ori in the branch delay slot and the sb on the join, which an unconditional store with a two-armed select produces; an empty else arm was the untested alternative spelling of the same semantics.
- probe: v4, same chassis.
- result: 30 at 44 insns -- five instructions LOST; the empty arm collapses the diamond, re-confirming s1's reading that the store is unconditional in the target.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD with all three blocks respelled as init-then-conditional-or; single `u8 *q` with three assignments, no FAKE-annotated construct present

## [s31] Mandated kill re-audit: the chassis and the closest banked form still measure what the ledger records, and the floor body carries no FAKE construct that could be masking a lever.
- mechanism: An instance kill is only valid on the chassis and FAKE state it was measured under, so both are re-established before new probes.
- probe: candidate.c installed on HEAD and measured; rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c (the only banked body whose instruction multiset matches the target) re-measured; `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c` run.
- result: candidate.c 10 at 49 insns, rules_dropped 0; the round-trip form 10 at 49 insns; fake_ablate reports 'no FAKE-annotated constructs found ... nothing to ablate'. The floor of 10 and the banked kills' chassis both hold.
- verdict: CONFIRMED

## [s31] The residual can be stated as an allocator INPUT fact -- allocno 74 conflicts with hard register $v1 -- and s23's claim that global.c set_preference can never record a $v0/$v1 preference for any pseudo here is inexact.
- mechanism: global.c:1709-1713 maps a copy operand through reg_renumber, so a copy between a global allocno and a pseudo local-alloc has already seated becomes a hard-reg preference; and any local-alloc pseudo seated in $v1 across an allocno's live range makes that allocno ineligible for $v1 before find_reg runs.
- probe: Read the `;; NN conflicts` / `;; NN preferences` headers of the .greg dumps for base, v1, v5, v6, v7, v8 and v11, plus set_preference in tools/gcc-2.7.2/global.c.
- result: Every body prints `;; 74 conflicts: ... 2 3 29` -- 74 is ineligible for $v0 and $v1 pre-find_reg; the $v1 holder is the block-0 QImode mask value that local-alloc seats first (s24's `blk=0 qty=0 reg1=76 size=1 mode=1` row). Contra s23, preferences for hard regs DO appear here: `77 preferences: 3` ($v1) and `78/82/86 preferences: 2` ($v0). The narrower true statement is that allocno 74 is never a copy operand (it is set from a bare symbol_ref), so no preference can be recorded for it; the only copy source that would carry one is an object local-alloc has seated in $v1, i.e. a second C object aliasing the byte (the standing banned construct).
- verdict: CONFIRMED

## [s32] Mandated kill re-audit: the floor-10 chassis and two banked instance kills still measure exactly what the ledger records, and the floor body still carries no FAKE construct.
- mechanism: An instance kill is only valid on the chassis and FAKE state it was measured under; both are re-established before any new probe.
- probe: candidate.c installed at src/code6cac_b.c:3420 and measured with `sandbox func_80034F88 --disable all`; `rejected/mask-on-symbol-then-pointer-score16.c` and `rejected/pointer-live-blocks23-only-score14.c` (the two banked forms that move the address allocno's live range, i.e. the forms closest to the target's register geometry) re-installed and re-measured; `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c` run.
- result: candidate.c 10 at 49/49 with rules_dropped 0; mask-on-symbol 16 at 51 insns; pointer-live-blocks23-only 14 at 49 insns -- all three reproduce their banked scores exactly. fake_ablate: 'no FAKE-annotated constructs found ... nothing to ablate'. The chassis and every s31 kill's measurement basis hold.
- verdict: CONFIRMED

## [s32] The frontier's mechanism statement ("allocno 74 is INELIGIBLE for $v1 because of a hard-reg conflict, and no route exists that does not add a second C object") is one-sided: the hard-reg conflict IS removable by structural means alone, and removing it exposes a SECOND, independent barrier -- allocno_compare priority -- that the same structural change creates.
- mechanism: global.c:635-655 orders allocnos by `floor_log2(n_refs)*n_refs / live_length * 10000 * size`; a shorter live range removes the hard-reg conflicts contributed by overlapping local-alloc-seated pseudos (local-alloc runs first and, with no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, hands out $v0 then $v1 in raw register order), but it simultaneously DIVIDES the address allocno's priority denominator down -- the two effects trade against each other.
- probe: .greg + .lreg dumps captured for three bodies with `pwsh tools/grinder/dump.ps1 func_80034F88` and compared allocno-by-allocno (tmp/grind/func_80034F88/s32/greg_base.txt, greg_v1.txt, greg_v2.txt, greg_v3.txt, lreg_base.txt).
- result: base (q live blocks 0-3, 74 = 10 refs across 29 insns): `74 conflicts: 72 74 77 78 81 82 85 86 2 3 29`, `74 in 4` ($a0); block-0 locals 75 (QI, 2 refs/4 insns) and 76 (SI, 3 refs/6 insns) are BOTH seated `in 3` ($v1) and 79 `in 2`. v1 (mask via symbol): the seats merely PERMUTE -- 75,76 in 2 and 79 in 3 -- and the conflict set is byte-identical, `74 in 4` still. v2 (q materialised after the flag-word read): 76 in 3, 75 in 2, conflict set again identical, `74 in 4`. **v3 (q live in blocks 2/3 only): 74 shrinks to 6 refs across 17 insns and its conflict set becomes `72 74 78 81 82 85 86 2 29` -- hard reg 3 ($v1) is GONE.** But 74 is still `in 4`: the allocation order printed is `73 82 86 78 77 81 85 74 72`, 74 is EIGHTH, and by the time find_reg reaches it $v1 is held by 78 (`78 in 3`), 81 and 85 -- all three of which 74 conflicts with. 74's priority is floor_log2(6)*6/17 = 0.71 against 78's floor_log2(5)*5/11 = 0.91 and 81/85's floor_log2(3)*3/4 = 0.75.
- verdict: CONFIRMED

## [s32] Shortening the address object's live range to blocks 2/3 -- the one structural change that provably deletes the $v1 hard-reg conflict -- seats the address in $v1.
- mechanism: with the block-0 local quantities no longer overlapping 74, find_reg should be free to give it $v1 in raw register order.
- probe: rejected/pointer-live-blocks23-only-score14.c installed on the current chassis, measured, and its .greg read (tmp/grind/func_80034F88/s32/greg_v3.txt).
- result: 14 at 49 insns and `74 in 4` -- the seat does NOT move. The conflict is gone but the priority ordering has replaced it: 74 is allocated eighth of nine, after 78/81/85 have taken $v1. The arithmetic threshold is exact -- on a 17-insn live range 74 would need floor_log2(n)*n > 15.45, i.e. EIGHT refs rather than six, to be ordered ahead of 78; and every extra ref through `q` is an extra memory access, i.e. an extra instruction on a body that is already at the target's 49.
- verdict: KILLED
- kill_scope: instance
- measured_on: pointer-live-blocks23-only body on the current HEAD floor-10 chassis (candidate.c = 10, 49/49, rules_dropped 0); single `u8 *q`, two assignments, no FAKE-annotated construct present

## [s32] Delaying q's materialisation below block 1's flag-word read (so the flag-word temps are born and die before the address allocno starts) removes enough block-0 overlap to drop the $v1 hard-reg conflict.
- mechanism: local-alloc seats block-0 quantities in raw register order ($v0 then $v1) and every one of them that overlaps the address allocno's range becomes a hard-reg conflict for it; making the flag-word temps die before the address is born should leave only the mask temps overlapping, and those can take $v0.
- probe: v2 (`c = p[8] & 1;` hoisted above `q = &D_80106A73; *q &= 0xF8;` inside block 1's scope), installed at src/code6cac_b.c:3420, measured and dumped; banked as rejected/s32-q-materialised-after-flagread-score14.c.
- result: 14 at 49 insns. `74 conflicts: 72 74 75 76 81 82 85 86 2 3 29` and `74 in 4` -- unchanged. The block-0 seats permute (76 in 3, 75 in 2) but block 0 retains two local quantities overlapping 74, so hard reg 3 stays in the conflict set. Four points spent for zero allocator movement.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD with q materialised after the block-1 flag-word read; single `u8 *q` with three assignments, no FAKE-annotated construct present

## [s32] Moving the mask off the pointer (block 0 masks the symbol directly, q first assigned in block 1) frees $v1 by removing the mask temps from the address allocno's overlap.
- mechanism: the mask's QI load temp and SI and-result are the two block-0 local quantities the .greg shows seated in $v1 on the base body; masking the symbol directly should give them a different birth point.
- probe: rejected/mask-on-symbol-then-pointer-score16.c re-installed on the current chassis, measured and dumped (tmp/grind/func_80034F88/s32/greg_v1.txt).
- result: 16 at 51 insns -- reproduces the banked score. The mask temps DO move ($v0), but block-0 local 79 takes $v1 in their place; `74 conflicts: ... 2 3 29` is byte-identical to base and `74 in 4` stands. The $v1 occupancy is a property of block 0 having >= 2 local quantities across the address's range, not of which statement produces them.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD with block 0's mask spelled as a direct symbol RMW; single `u8 *q` with three assignments, no FAKE-annotated construct present

## [s32] Mandated kill re-audit: the floor-10 chassis and the two banked instance kills whose forms move the address allocno's live range still measure what the ledger records, and the floor body carries no FAKE construct that could be masking a lever.
- mechanism: An instance kill is only valid on the chassis and FAKE state it was measured under, so both are re-established before any new probe.
- probe: candidate.c installed at src/code6cac_b.c:3420 and measured with `sandbox func_80034F88 --disable all`; rejected/mask-on-symbol-then-pointer-score16.c and rejected/pointer-live-blocks23-only-score14.c re-installed and re-measured; tools/fake_ablate.py run on candidate.c.
- result: candidate.c 10 at 49/49, rules_dropped 0, cheat_asm_stripped 27; mask-on-symbol 16 at 51 insns; pointer-live-blocks23-only 14 at 49 insns -- all three reproduce their banked scores exactly. fake_ablate: 'no FAKE-annotated constructs found ... nothing to ablate'.
- verdict: CONFIRMED

## [s32] The frontier's mechanism statement is one-sided: the hard-reg conflict that keeps the address allocno out of $v1 is removable by a structural change alone, and removing it exposes a second, independent barrier (global.c allocno_compare priority) that the same change creates.
- mechanism: global.c:635-655 orders allocnos by floor_log2(n_refs)*n_refs/live_length*10000*size; tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so local-alloc (which runs first) seats its block-0 quantities in raw register order $v0 then $v1, and each one overlapping the address allocno's range becomes a hard-reg conflict for it. Shortening that range removes the overlap but simultaneously lowers the allocno's priority quotient relative to the allocnos it competes with.
- probe: pwsh tools/grinder/dump.ps1 func_80034F88 on four bodies (base, mask-via-symbol, q-materialised-late, pointer-live-blocks23-only), reading the ';; NN conflicts' / ';; N regs to allocate' / 'Register dispositions' blocks of .greg and the per-pseudo ref/live-length headers of .lreg.
- result: base: 74 = 10 refs across 29 insns, `74 conflicts: 72 74 77 78 81 82 85 86 2 3 29`, `74 in 4` ($a0); block-0 locals 75 and 76 both seated `in 3` ($v1). mask-via-symbol and q-materialised-late: the block-0 seats permute (75,76 -> $v0 with 79 -> $v1; then 76 -> $v1 with 75 -> $v0) and the conflict set and `74 in 4` are byte-identical. pointer-live-blocks23-only: 74 shrinks to 6 refs across 17 insns and its conflict set becomes `72 74 78 81 82 85 86 2 29` -- hard reg 3 is GONE -- yet 74 is still `in 4`, because the printed allocation order `73 82 86 78 77 81 85 74 72` puts it eighth, after 78 (priority 0.91), 81 and 85 (0.75), all of which conflict with 74 and take $v1, against 74's own 0.71.
- verdict: CONFIRMED

## [s32] Shortening the address object's live range to blocks 2 and 3 -- the structural change that provably deletes the $v1 hard-reg conflict -- seats the address in $v1.
- mechanism: With the block-0 local quantities no longer overlapping the address allocno, find_reg is free to hand it $v1 in raw register order.
- probe: rejected/pointer-live-blocks23-only-score14.c installed on the current chassis, measured with sandbox, and its .greg read at tmp/grind/func_80034F88/s32/greg_v3.txt.
- result: 14 at 49 insns, `74 in 4`. The hard-reg conflict is gone but the priority ordering replaces it. The threshold is exact: on a 17-insn live range the address allocno would need floor_log2(n)*n > 15.45, i.e. EIGHT refs rather than six, to be ordered ahead of allocno 78 -- and each extra ref through q measured so far is an extra memory access on a body already at the target's 49 instructions.
- verdict: KILLED
- kill_scope: instance
- measured_on: pointer-live-blocks23-only body on the current HEAD floor-10 chassis (candidate.c = 10, 49/49, rules_dropped 0); single u8 *q, two assignments, no FAKE-annotated construct present

## [s32] Delaying q's materialisation below block 1's flag-word read, so the flag-word temps are born and die before the address allocno starts, removes enough block-0 overlap to drop the $v1 hard-reg conflict.
- mechanism: local-alloc seats block-0 quantities in raw register order and every one overlapping the address allocno's range becomes a hard-reg conflict for it; making the flag-word temps die before the address is born should leave only the mask temps overlapping, and those could take $v0.
- probe: v2 (`c = p[8] & 1;` hoisted above `q = &D_80106A73; *q &= 0xF8;` inside block 1's scope) installed at src/code6cac_b.c:3420, measured and dumped; banked as rejected/s32-q-materialised-after-flagread-score14.c.
- result: 14 at 49 insns. `74 conflicts: 72 74 75 76 81 82 85 86 2 3 29` and `74 in 4` -- unchanged from base. The block-0 seats permute (76 in 3, 75 in 2) but block 0 retains two local quantities overlapping 74. Four points spent for zero allocator movement.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD with q materialised after the block-1 flag-word read; single u8 *q with three assignments, no FAKE-annotated construct present

## [s32] Moving the mask off the pointer (block 0 masks the symbol directly, q first assigned in block 1) frees $v1 by removing the mask's temps from the address allocno's overlap.
- mechanism: The mask's QI load temp and SI and-result are the two block-0 local quantities the base .greg shows seated in $v1; masking the symbol directly gives them a different birth point.
- probe: rejected/mask-on-symbol-then-pointer-score16.c re-installed on the current chassis, measured and dumped (tmp/grind/func_80034F88/s32/greg_v1.txt).
- result: 16 at 51 insns -- reproduces the banked score. The mask temps do move to $v0, but block-0 local 79 takes $v1 in their place; the conflict set is byte-identical to base and `74 in 4` stands. The $v1 occupancy tracks block 0 holding two or more local quantities across the address's range, not which statement produces them.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD with block 0's mask spelled as a direct symbol RMW; single u8 *q with three assignments, no FAKE-annotated construct present

## [s33] The score-0 three-object body wins by raising the allocno COUNT so that global.c's find_reg happens to land the target's $v1/$a0 split (the reading every session since s13 has carried).
- mechanism: claimed as "three allocnos beat one" -- more candidates for $v1 in the priority ordering.
- probe: the banned body installed at src/code6cac_b.c:3420, re-measured (score 0 at 49/49 on HEAD) and dumped for the first time in 33 sessions; .greg / .lreg sliced to tmp/grind/func_80034F88/s33/greg_t3.txt and lreg_t3.txt and compared against greg_base.txt.
- result: FALSIFIED, and replaced by an exact two-step mechanism. The winning body's first handle (qm) is pseudo 80, which is ABSENT from the allocno list `;; 10 regs to allocate: 73 78 79 77 74 75 76 82 72 81` -- it is a block-0-confined LOCAL-ALLOC quantity, seated in $v1 by raw register order because $v0 holds the live func_80077D00 return and mips.h defines no REG_ALLOC_ORDER. cse then rewrites the second handle's `= &D_80106A73` into a register COPY from pseudo 80, which global.c:1709-1713 records as `;; 81 preferences: 3`, and 81's conflict set `72 74 77 81 29` carries no hard-reg 3. Allocno 81 is allocated LAST of the ten and still takes $v1. Count is irrelevant; the local-alloc seat plus the copy preference are the whole mechanism.
- verdict: CONFIRMED (as a correction: the old statement is dead, the new mechanism is measured)

## [s33] The local-alloc $v1 seat that the banned body gets from its first pointer handle can be produced by a compiler temp instead, so a body with ONE declared pointer object can seat the address object in $v1.
- mechanism: local-alloc seats any pseudo whose live range is confined to one basic block, and an address expression that is not CONSTANT_ADDRESS_P at expand time (mips.h:2369) is forced into such a pseudo. cse folds the expression to a plain symbol_ref afterwards, so the temp costs no instruction, and cse rewrites the later `q = &D_80106A73;` into a copy from it, which global.c:1709-1713 turns into a hard-reg preference.
- probe: c1 -- the mask statement spelled `*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) &= 0xF8;` with `q` first assigned inside block 1; installed, measured with sandbox --disable all, dumped (tmp/grind/func_80034F88/s33/greg_c1.txt) and disassembled (c1.txt). Banked as rejected/s33-anon-symdiff-block0-blocks01-EXACT-score13.c.
- result: CONFIRMED. c1's .greg prints `;; 74 preferences: 3` and `74 in 3` with the hard-reg-3 conflict absent (`74 conflicts: 72 74 79 80 83 84 87 88 2 29` against base's `... 2 3 29`) -- the first body in 33 sessions to seat the single declared address object in $v1. Blocks 0 and 1 come out BYTE-EXACT with the target through 0x80034FD0, including the block-1 lbu that has been missing since s1. Zero instruction cost and zero frame cost (prologue stays addiu sp,sp,-24).
- verdict: CONFIRMED

## [s33] With blocks 0-1 seated correctly by the anonymous temp, the remaining blocks can keep their $a0 seats, so the anonymous-temp family reaches the target.
- mechanism: hoped that q's $v1 preference would apply only where the copy chain reaches.
- probe: c1 measured and disassembled; follow-ups c2 (blocks 0 and 1 both anonymous, q for blocks 2/3), c3 (q in block 1 only), c4 (block 0 anonymous, q in blocks 1+2), c5 (q materialised before the anonymous mask). All five banked in rejected/.
- result: KILLED. c1 = 13 at 49, c2 = 21 at 50, c3 = 14 at 48, c4 = 13 at 49, c5 = 10 at 49. The seat swap MOVES rather than closes: q is one C object, hence one pseudo, hence one allocno, hence one hard register, so its $v1 preference also pulls blocks 2/3 (register-exact on candidate.c) onto $v1 where the target uses $a0. All 13 of c1's differing instructions are in blocks 2/3, and the lui/addiu-vs-sb rotation at the block-1 join is a consequence of block 2 sharing $v1, not a separate defect.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c score-10 chassis on HEAD; five bodies c1-c5, each carrying the single declared `u8 *q` plus (in c1-c4) the anonymous symbol-difference address; no FAKE-annotated construct present
- predicate_cite: tools/gcc-2.7.2/global.c:426

## [s33] An anonymous (undeclared) address temp can carry the blocks-2/3 address as well, giving two independently-seated address values from one declared object.
- mechanism: if the block-0 trick works once it should work again in blocks 2 and 3, each producing its own local-alloc temp.
- probe: c2 (blocks 0 and 1 anonymous, q reserved for blocks 2/3) and c3 (q in block 1 only, blocks 0/2/3 anonymous), both installed and measured.
- result: KILLED. c2 = 21 at 50 build insns and c3 = 14 at 48. Block 1's store and every block-2/3 access sit AFTER a diamond join label, and a code label is a cse path boundary (s27 gate 5), so an anonymous address re-materialises there instead of being reused -- c2 pays the extra lui/addiu (50 vs the target's 49) and c3 loses one to over-sharing (48). An anonymous address temp cannot survive a code label; only a named C object can. The target's geometry needs one value spanning blocks 0-1 and a separate value spanning blocks 2-3, i.e. two pseudos each surviving a cse path boundary.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD, bodies c2 and c3; single declared `u8 *q` plus anonymous symbol-difference addresses, no FAKE-annotated construct present

## [s33] KILL RE-AUDIT (mandated): the s20/s25 dead round-trip, the banked form whose instruction multiset matches the target's, still measures 10 on the current chassis.
- mechanism: setting the address pseudo bumps reg_tick at cse.c:1539 and un-validates the (mem:QI (reg 74)) entry, restoring block 1's load at zero instruction cost.
- probe: rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c re-installed at src/code6cac_b.c:3420 and measured; fake_ablate.py re-run against candidate.c.
- result: 10 at 49 build insns -- unchanged for the fifth consecutive session. fake_ablate reports "no FAKE-annotated constructs found ... nothing to ablate" on candidate.c, so no banked kill on this chassis was measured with a FAKE carrier on the contested pseudo.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-05 (candidate.c = 10, 49/49, rules_dropped 0); dead-arithmetic round-trip construct present, no FAKE-annotated construct

# FRONTIER RESET (s33 synthesis) -- supersedes the s32 frontier

The s32 frontier's central item ("raise the address allocno's ref count to eight
on a 17-insn live range without adding an instruction") is RETIRED: s33 reached
the $v1 seat without touching ref counts at all, by supplying a local-alloc copy
source instead. The two-barrier framing is also retired -- barrier (1), the
hard-reg conflict, is now deletable at zero cost (c1's .greg), and barrier (2),
priority, never fires when a preference exists.

F1. Blocks 2-3 need an $a0-seated address value that is a DIFFERENT pseudo from
    the $v1-seated blocks-0/1 value, and both must survive a cse path boundary.
    mechanism: global.c:426 -- one C object is one pseudo is one allocno is one
    hard register; cse path boundaries at code labels force an anonymous address
    to re-materialise (measured c2 = 50 insns, c3 = 48 insns).
    next probe: the only untested shape is a SECOND pseudo that is not a second
    declared object and not anonymous -- e.g. a value the compiler is obliged to
    keep in a register across a label for a reason other than being a named
    local. Read c1's .flow / .greg live-range table for any pseudo already
    crossing the block-1 join, and check whether an ordinary-C expression in
    blocks 2/3 can be made to consume it. If nothing is found, F1 is the
    admissible ceiling and the residual is 10.

F2. c1's 13 points are a pure two-block register rename, and candidate.c's 10
    points are the mirror image in blocks 0-1. Neither body is closer than the
    other in POINTS, but c1 is structurally closer to the target (its block-1
    lbu is present and its join rotation is the only ordering difference).
    mechanism: the sandbox score counts differing instructions, so a two-block
    rename (13) outscores a one-block rename plus a nop (10) even though c1
    reproduces more of the target's structure.
    next probe: measure a c1-based body in which blocks 2 and 3 are spelled to
    prefer $a0 through a copy from an $a0-seated pseudo (the mirror of the
    block-0 trick) -- the call return is in $v0, `p` is in $a1, and nothing in
    the function is naturally seated in $a0 before block 2, so this needs a
    source of an $a0 preference that does not exist yet. Confirm that from
    .greg's preference lines before spending a session on spellings.

F3. Ladder accounting. Owner directive 2026-09-02 requires 20 flat sessions and
    at least 6 distinct modalities in cycle 2 before any disposition; s33 is
    session TWELVE of the cycle (escalation s23/s24, synthesis s25/s33, solver
    s26, forensics s27/s28, rederive s29/s30, structural s31/s32 -- six
    modalities, condition already met at s31). Eight sessions remain.
    next probe: when the driver assigns escalation modality, file the LADDER
    EXHAUSTED (non-endgame residual, floor 10) foreclosure record citing the
    completed cse enumeration (s27/s28), s29's reload pricing, s30's F1
    byte-neutrality, s31/s32's allocator restatement, and s33's proof that both
    halves of the seat split are individually reachable from one object but not
    jointly.

## [s33] The score-0 three-object body wins by raising the allocno COUNT so that global.c's find_reg lands the target's $v1/$a0 split -- the reading every session since s13 has carried.
- mechanism: Claimed as 'three allocnos beat one': more candidates competing for $v1 in allocno_compare's priority ordering.
- probe: The Judge-FAILed body rejected/three-pointer-objects-judge-FAIL-score0.c installed at src/code6cac_b.c:3420, re-measured (score 0 at 49/49 on HEAD) and dumped with pwsh tools/grinder/dump.ps1; .greg/.lreg sliced to tmp/grind/func_80034F88/s33/greg_t3.txt and lreg_t3.txt and compared against greg_base.txt.
- result: Falsified and replaced by an exact two-step mechanism. The winning body's first handle (qm) is pseudo 80, ABSENT from the allocno list ';; 10 regs to allocate: 73 78 79 77 74 75 76 82 72 81' -- a block-0-confined LOCAL-ALLOC quantity that local-alloc seats in $v1 (raw register order; $v0 holds the live func_80077D00 return and mips.h defines no REG_ALLOC_ORDER). cse rewrites the second handle's `= &D_80106A73` into a register COPY from pseudo 80; global.c:1709-1713 records that as ';; 81 preferences: 3'; and 81's conflict set '72 74 77 81 29' carries no hard-reg 3, so it takes $v1 even though it is allocated LAST of the ten. Allocno count is irrelevant.
- verdict: CONFIRMED

## [s33] The local-alloc $v1 seat that the banned body gets from its first pointer handle can be supplied by a compiler temp instead, so a body with ONE declared pointer object can seat the address object in $v1.
- mechanism: local-alloc seats any pseudo confined to one basic block; an address expression that is not CONSTANT_ADDRESS_P at expand (tools/gcc-2.7.2/config/mips/mips.h:2369) is forced into such a pseudo, cse folds the expression to a plain symbol_ref afterwards so the temp costs no instruction, and cse then rewrites the later `q = &D_80106A73;` into a copy from it, which global.c:1709-1713 turns into a hard-reg preference for $v1.
- probe: c1 -- the mask statement spelled `*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) &= 0xF8;` with q first assigned inside block 1. Installed, measured with sandbox --disable all, dumped (tmp/grind/func_80034F88/s33/greg_c1.txt) and disassembled (c1.txt). Banked as rejected/s33-anon-symdiff-block0-blocks01-EXACT-score13.c.
- result: CONFIRMED. c1's .greg prints ';; 74 preferences: 3' and '74 in 3' with the hard-reg-3 conflict absent ('74 conflicts: 72 74 79 80 83 84 87 88 2 29' against base's '... 2 3 29') -- the first body in 33 sessions to seat the single declared address object in $v1. Blocks 0 and 1 are BYTE-EXACT with the target through 0x80034FD0, including the block-1 lbu missing since s1 and the $v1-base/$a0-value convention. Zero instruction cost and zero frame cost (prologue stays addiu sp,sp,-24, unlike s30's chain-extender phantom slot). Score 13 at 49 build insns.
- verdict: CONFIRMED

## [s33] With blocks 0-1 seated correctly by the anonymous local-alloc temp, blocks 2-3 keep their $a0 seats, so a one-declared-object body in this family reaches the target's full register assignment.
- mechanism: Hoped q's $v1 preference would apply only to the uses the cse copy chain reaches, leaving blocks 2/3 on $a0 as they are on candidate.c.
- probe: Five bodies installed and measured on the HEAD floor-10 chassis: c1 (block 0 anonymous, q from block 1), c2 (blocks 0+1 anonymous, q for blocks 2/3), c3 (q in block 1 only, blocks 0/2/3 anonymous), c4 (block 0 anonymous, q in blocks 1+2, block 3 anonymous), c5 (q materialised BEFORE the anonymous mask). All five banked under memory/grind/func_80034F88/rejected/s33-*.c.
- result: c1 = 13/49, c2 = 21/50, c3 = 14/48, c4 = 13/49, c5 = 10/49. The seat swap MOVES rather than closes: all 13 of c1's differing instructions are in blocks 2/3, which candidate.c gets right, and the 3-insn lui/addiu-vs-sb rotation at the block-1 join is a consequence of block 2 sharing $v1 (its address cannot be hoisted above block 1's store), not a separate defect. c5 is the isolating control -- moving q's materialisation above the anonymous expression makes the temp the copy DESTINATION and the body collapses back to exactly the base chassis.
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c score-10 chassis on HEAD (re-measured this session: 10, 49/49, rules_dropped 0, cheat_asm_stripped 27); bodies c1-c5, each with the single declared `u8 *q` plus (c1-c4) an anonymous symbol-difference address; no FAKE-annotated construct present, fake_ablate reports nothing to ablate
- predicate_cite: tools/gcc-2.7.2/global.c:426

## [s33] An anonymous (undeclared) address temp can also carry the blocks-2/3 address, giving two independently-seated address values from one declared object.
- mechanism: If the block-0 local-alloc trick works once it should work again in blocks 2 and 3, each producing its own block-confined temp seated independently.
- probe: c2 (blocks 0 and 1 anonymous, q reserved for blocks 2/3) and c3 (q in block 1 only, blocks 0/2/3 anonymous), both installed at src/code6cac_b.c:3420 and measured with sandbox --disable all.
- result: c2 = 21 at 50 build insns, c3 = 14 at 48. Block 1's store and every block-2/3 access sit AFTER a diamond join label, and a code label is a cse path boundary (the s27 gate-5 enumeration), so an anonymous address re-materialises there instead of being reused: c2 pays an extra lui/addiu against the target's 49 insns and c3 loses one to over-sharing between blocks 2 and 3. An anonymous address temp does not survive a code label on this body; a named C object does.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD, bodies c2 and c3; single declared `u8 *q` plus anonymous symbol-difference addresses, no FAKE-annotated construct present

## [s33] MANDATED KILL RE-AUDIT: the s20/s25 dead round-trip -- the only banked body whose instruction multiset matches the target's -- still measures 10 on the current chassis and is still not a submission route.
- mechanism: Setting the address pseudo makes cse_insn invalidate reg 74, bumping reg_tick at cse.c:1539 and un-validating the (mem:QI (reg 74)) entry, so block 1's load survives cse; both round-trip insns carry REG_EQUAL notes, so the arithmetic folds and the construct costs zero instructions.
- probe: rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c re-installed at src/code6cac_b.c:3420 and measured; tools/fake_ablate.py re-run against memory/grind/func_80034F88/candidate.c.
- result: 10 at 49 build insns -- unchanged for the fifth consecutive session. fake_ablate reports 'no FAKE-annotated constructs found ... nothing to ablate' on candidate.c, so no banked kill on this chassis was measured with a FAKE carrier occupying the contested pseudo. The construct remains dead arithmetic with no semantic purpose (cheat-checklist T1/T2/T6).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-05 (candidate.c = 10, 49/49, rules_dropped 0); dead-arithmetic round-trip construct present, no FAKE-annotated construct

==== s34 (synthesis) ====

MERGED ATTACK (the synthesis this session was dispatched for).

The 33 preceding sessions are consistent and can now be stated as ONE fact plus
one open question.

  FACT. The target holds &D_80106A73 in two hard registers: $v1 for block 0 plus
  block 1 (whose store sits after the join label .L80034FC8) and $a0 for blocks 2
  and 3 (two disjoint ranges, so one pseudo suffices). global.c:426 gives one
  pseudo one allocno one hard register and GCC 2.7.2 has no live-range splitting,
  so the body needs TWO address pseudos, and the $v1 one must be live across a
  code label. candidate.c's single `q` reproduces the $a0 half exactly (blocks 2/3
  register-exact, s16/s32); s33's c1 reproduces the $v1 half exactly (blocks 0/1
  byte-exact) by seating an anonymous local-alloc temp in $v1 and letting cse turn
  q's symbol set into a copy from it (global.c:1709-1713). Neither body gets both,
  because in each of them the two halves are the SAME C object.

  OPEN QUESTION s34 attacked. Can the second carrier be ANONYMOUS -- a compiler
  pseudo with no C name -- so that the Judge's one-named-pointer constraint is
  respected? Two structurally different routes exist and both were measured:
    (A) make the store's address expand BEFORE the branch (ternary RHS), so the
        anonymous pseudo is born in block 0's basic block and merely stays live;
    (B) move the store INSIDE the arms (duplicated-statement-into-arms), so no
        address value has to cross the join at all.
  Both are dead, and for the same reason, established from dumps rather than
  inference: reorg.c:3442 deletes the else-arm store as a redundant delay-slot
  thread insn (it survives every pass through .sched2 and is a NOTE_INSN_DELETED
  at .dbr), which also frees the block-1 reload, costing 2 of the target's 49
  instructions. Leaving the store at the join instead forces the anonymous
  address to re-materialise there (d4/c2 = 50 insns).

  So the anonymous-carrier route is measured closed on this chassis from both
  sides, and the residual is exactly: a second NAMED pointer object, which is the
  standing Judge ban.

FRONTIER RESET (the strongest three for the next ladder pass).

  H-s34-1. The P1 carrier must be a pseudo live across .L80034FC8 that is not
  `q`. Every construct measured so far that produces such a pseudo is a named C
  pointer object. The ONE untested class is a carrier not derived from a
  symbol_ref at all -- an address value computed from a pseudo that already
  crosses the join. c1's .greg names the only such pseudo: 72 = `p` in $a1.
  NEXT PROBE: measure whether any ordinary-C expression of the flag byte's
  address in terms of `p` (the func_80077D00 return) survives cse/combine without
  being folded back to the bare symbol_ref, and at what instruction cost. If every
  such spelling either folds to the symbol (collapsing onto the base chassis) or
  adds an addu/subu, this class is closed and the anonymous-carrier route is
  exhausted in full.

  H-s34-2. The block-1 reload (`lbu $a0,0($v1)` at 80034FB4, the target's 176th
  lbu against our 175) appears exactly when block 0's store and block 1's load are
  spelled through address RTL that cse does NOT equate -- present in c1 and in the
  score-0 banned body, absent in candidate.c. s29 priced restoring it at ZERO
  points on the base chassis, which is why it has never been chased.
  NEXT PROBE: on the base chassis (q for all four blocks, blocks 2/3
  register-exact), find a spelling of block 0's mask that defeats cse's memory
  value tracking without changing the address pseudo, and confirm s29's zero
  pricing by direct measurement rather than by arithmetic on the objdump. If it
  measures BELOW 10 the residual factorisation is wrong and the ledger's "one
  missing address allocno" framing must be re-derived.

  H-s34-3. Ladder accounting: s34 is session thirteen of cycle 2 (six modalities
  reached at s31). Seven flat sessions remain before the owner directive
  2026-09-02 permits any disposition. When the driver assigns `escalation`, the
  record to file is LADDER EXHAUSTED (non-endgame residual, floor 10), citing the
  cse enumeration (s27/s28), the reload pricing (s29), F1 byte-neutrality (s30),
  the allocator restatement (s31/s32), s33's proof that both seats are
  individually reachable from one object, and s34's proof that the anonymous
  second carrier is deleted by reorg on one route and costs an instruction on the
  other.

KILLS RECORDED THIS SESSION (all instance-scoped; chassis = candidate.c at 10/49
on HEAD 2026-09-05, no FAKE construct present in any measured body).

  K-s34-A  Spelling block 1's store as a ternary (`E = c ? (v|1) : v;`) so the
           destination address is expanded before the branch: d1 22/48, d5 18/48,
           d6 26/50, d7 12/46. KILLED.
  K-s34-B  Duplicating block 1's store into both arms so no address crosses the
           join: e1 22/48, e4 22/48, e5 18/48, e6 12/46. KILLED.
  K-s34-C  Mixing an anonymous read with a named store in block 1: f1 14/49,
           f2 13/49. KILLED (f2 confirms s33's c5 from the other side).

## [s34] MANDATED KILL RE-AUDIT: the s20/s25 dead round-trip (the only banked body whose instruction multiset matches the target's) and s33's c1 (the structurally closest body, blocks 0/1 byte-exact) both reproduce their banked scores on today's chassis.
- mechanism: Both bodies re-installed at src/code6cac_b.c:3420 and re-measured with `sandbox func_80034F88 --disable all`; candidate.c itself re-measured first to fix the chassis. candidate.c carries no FAKE-annotated construct (s31/s32/s33 fake_ablate runs, body byte-identical since), so neither kill was measured with a FAKE carrier on the contested pseudo.
- probe: run.ps1 -names cand,c1,rt
- result: cand = 10 at 49 build insns / 49 target insns, rules_dropped 0, cheat_asm_stripped 28; rt = 10 at 49; c1 = 13 at 49. Both kills hold unchanged for the sixth consecutive session.
- verdict: CONFIRMED

## [s34] Spelling block 1's store as a ternary (`E = c ? (v|1) : v;`), so that expand_assignment expands the destination address before the branch and the anonymous address pseudo is born in block 0's basic block, does not produce a body at the target's 49 instructions on this chassis.
- mechanism: expand_assignment evaluates the destination MEM before the RHS, so the address temp exists pre-branch and merely stays live across the join; the intent was a second address carrier with no C name, respecting the one-named-pointer constraint. Measured against control d4, which reproduces s33's c2 at 21/50 exactly.
- probe: Four bodies measured with the s33 anonymous symbol-difference as the address spelling: d1 (block0 anon + block1 ternary, blocks 2/3 q), d5 (all four blocks anon ternary), d6 (block0 anon ternary + blocks 2/3 q ternary), d7 (all-q chassis, block 1 ternary only). Artifacts tmp/grind/func_80034F88/s34/variants/d*.c.
- result: d1 22 at 48 insns, d5 18 at 48, d6 26 at 50, d7 12 at 46. Every ternary body loses instructions instead of gaining the seat; none beats candidate.c's 10.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 (10, 49/49, rules_dropped 0); four ternary-store bodies, single declared `u8 *q`, no FAKE construct present in any of them

## [s34] Duplicating block 1's store into both if-arms (the duplicated-statement-into-arms shape), so that no address value has to cross the join label at all, does not produce a body at the target's 49 instructions on this chassis: the else-arm store is deleted and the block-1 reload with it.
- mechanism: Dump-verified pass attribution rather than inference: in e1 both arm stores (insns 45 and 55) survive .rtl, .jump, .cse, .loop, .cse2, .combine, .flow, .jump2, .lreg, .greg and .sched2 (5 `(set (mem:QI` in the func_80034F88 slice throughout), and insn 55 appears as `(note 51 45 57 "" NOTE_INSN_DELETED)` at .dbr. The delay-slot reorg pass deletes it as a redundant thread insn (tools/gcc-2.7.2/reorg.c:3442, matcher `redundant_insn` at reorg.c:1996). cse never touches these insns.
- probe: Four bodies measured: e1 (block0 anon + block1 arms duplicated, blocks 2/3 q), e4 (arms inverted), e5 (all four blocks anon with duplicated arm stores), e6 (all-q chassis, block 1 arms duplicated); then dump.ps1 on e1 with per-pass QI-store counts. Artifacts tmp/grind/func_80034F88/s34/variants/e*.c and dumps_e1/.
- result: e1 22 at 48 insns, e4 22 at 48, e5 18 at 48, e6 12 at 46 -- scores and counts identical to the matching ternary bodies, i.e. the two routes converge on the same RTL. The emitted assembly shows `beq $2,$0,.L737` with `ori` in the delay slot, ONE `sb`, and `.L737` after it.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05; four duplicated-arm-store bodies, single declared `u8 *q`, no FAKE construct present in any of them

## [s34] Mixing an anonymous read with a named store inside block 1 (read through the symbol-difference expression, store through `q`) does not beat candidate.c on this chassis, and placing `q` before the anonymous read reproduces s33's c1 score exactly.
- mechanism: Once `q` is materialised anywhere in the block-0/1 region, cse rewrites its symbol set into a register copy from the anonymous temp and global.c:1709-1713 records `74 preferences: 3`, so `q` takes $v1 for its WHOLE range including blocks 2 and 3 -- which candidate.c gets right in $a0. This is s33's c5 result seen from the other side.
- probe: f1 (anonymous read, then `q = &D_80106A73;`, then `*q = c;`) and f2 (`q` materialised before the anonymous read). Artifacts tmp/grind/func_80034F88/s34/variants/f1.c, f2.c.
- result: f1 14 at 49 insns, f2 13 at 49 insns (c1's score). Neither reaches 10.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05; two mixed anonymous-read/named-store bodies, single declared `u8 *q`, no FAKE construct present

## [s34] s33 frontier item F1 -- whether any pseudo already crosses the block-1 join for reasons of its own and could be consumed as an address base by blocks 2/3 -- is answered from c1's .greg: exactly one such pseudo exists, 72 = `p` seated in $a1.
- mechanism: `;; Register dispositions: 72 in 5` with `;; 9 regs to allocate: 73 80 84 88 74 79 83 87 72`; 72 is the func_80077D00 return copy, live across the whole body. Every other allocno's live range is confined to one flag block. Consuming 72 as an address base means runtime arithmetic between an unrelated data pointer and a static symbol, which costs instructions and is not ordinary C.
- probe: dump.ps1 on the c1 body; tmp/grind/func_80034F88/s34/dumps_c1/code6cac_b.greg.
- result: F1 closed. Also corrects s33's F2: its premise ('nothing is naturally seated in $a0 before block 2') is FALSE -- 79, 83 and 87 are all `in 4`, and 79 is block 1's loaded byte value in $a0. F2 is closed instead by the absence of any legal C copy from a byte VALUE to an address pseudo.
- verdict: CONFIRMED

## [s35] MANDATED KILL RE-AUDIT: the s20/s25 dead round-trip and s33's c1 -- the two banked bodies closest to the target -- still measure what the ledger records on today's chassis, and the floor body still carries no FAKE construct.

- mechanism: An instance kill is only as good as the chassis and FAKE state it was measured under; the driver's dispatch brief has printed "measurement unavailable" for seven consecutive sessions, so the floor must be re-established by direct measurement each time.
- probe: Installed `memory/grind/func_80034F88/candidate.c` (as variant g0, regenerated byte-for-byte), `rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c` (rt) and `rejected/s33-anon-symdiff-block0-blocks01-EXACT-score13.c` (c1) at `src/code6cac_b.c:3420` in turn and ran `sandbox func_80034F88 --disable all` on each; then ran `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c`.
- result: CONFIRMED, all three. g0 = 10 at 49 build insns (the floor is measured, not inherited). rt = 10 at 49, seventh consecutive session unchanged. c1 = 13 at 49, reproducing s33 and s34 exactly. fake_ablate: "no FAKE-annotated constructs found ... nothing to ablate", so no banked kill on this chassis was measured with a FAKE carrier occupying the contested pseudo.
- verdict: CONFIRMED

## [s35] Some assignment of the four flag-byte access sites between the declared pointer `q` and the anonymous symbol-difference expression, other than the five combinations s33 already measured, reaches the target's two-seat register assignment.

- mechanism: `tools/gcc-2.7.2/global.c:426` gives one pseudo one allocno one hard register, so the seat of the declared object is decided by its live range and by the cse-created copy preference that s33 traced to `global.c:1709-1713`. Which access sites `q` covers is the only free parameter left on the single-object chassis, and s33/s34 had measured only five of its settings.
- probe: Generated and measured the remaining quadrants with `tmp/grind/func_80034F88/s35/gen.py` + `run.ps1` on the HEAD chassis: g1 (q covers blocks 0+1, blocks 2/3 anonymous), g5 (q covers block 0 only, blocks 1/2/3 anonymous), g6 (g1 but with `q` itself initialised through the punned symbol-difference expression, so the cse copy preference exists at q's birth), g7 (base coverage q:0123 with every materialisation spelled through the punned expression), g8 (g1 with two DIFFERENT anonymous spellings in blocks 2 and 3).
- result: KILLED. g1 = 28 at 48; g5 = 25 at 48; g6 = 27 at 48; g7 = 14 at 49; g8 = 28 at 48. With s33's c1/c2/c3/c4/c5 and the base, the q-coverage lattice is now enumerated end to end and its minimum is the base setting q:0123 = candidate.c = 10. All five bodies banked in `rejected/s35-*.c`.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 (g0 control = 10, 49 build insns, rules_dropped 0); single declared `u8 *q`, no FAKE construct present in any variant

## [s35] Two anonymous address expressions written with DIFFERENT nestings escape cse's canonicalisation and each get their own full-address pseudo, which is the form the target's blocks 2 and 3 have.

- mechanism: The target materialises the flag byte's address three times into full-address registers (`lui`+`addiu`, then `lbu`/`sb` at offset 0). An anonymous expression that cse cannot equate with its neighbour would have to be re-materialised, which is the only route to a second full-address value that does not declare a second C object.
- probe: g8 spells block 2 as `*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70))` and block 3 as `*(u8 *)((s32)&D_80106A73 + ((s32)&D_80106A70 - (s32)&D_80106A70))`, with `q` covering blocks 0+1; measured against g1, which uses the single spelling in both blocks.
- result: KILLED. g8 = 28 at 48 build insns, the same score and instruction count as g1: cse canonicalises both nestings to the same address rtx, so the two sites share one value exactly as they do with a single spelling.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05; single declared `u8 *q` covering blocks 0+1, blocks 2/3 anonymous, no FAKE construct present

## [s35] The inherited claim that an anonymous address carrier costs +1 instruction every time its value must cross a code label.

- mechanism: s33's c2 (blocks 0 and 1 both anonymous) measured 50 insns against the target's 49, and s33/s34 generalised that to a per-label cost for anonymous carriers, which is what closed the anonymous route in the ledger's frontier statement.
- probe: Disassembled g1 (`tmp/grind/func_80034F88/s35/g1.txt`) and counted the instructions the anonymously-addressed blocks 2 and 3 actually emit, against the target's own blocks 2 and 3 in `asm/funcs/func_80034F88.s`.
- result: KILLED as a general statement. Each anonymous block emits `lui`/`lbu %lo` for the read and `lui`/`sb %lo` for the store = FOUR instructions, exactly the count of the target's `lui`/`addiu`/`lbu`/`sb`. The anonymous carrier is instruction-count-NEUTRAL in blocks 2/3 and wrong only in FORM (two separate %hi materialisations instead of one reused full address), hence in register assignment. g1's single missing instruction (48 vs 49) is block 1's reload, the same one candidate.c lacks. The +1 seen in c2/d4 belongs specifically to block 1's post-join store, where the address must be produced a second time inside the join block.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-05, variant g1 (q covers blocks 0+1, blocks 2/3 anonymous), no FAKE construct present; disassembly of tmp/sandbox/func_80034F88/code6cac_b.o

## [s35] The address object's seat is a two-valued dial between $v1 (target, blocks 0/1) and $a0 (target, blocks 2/3), so live-range surgery on `q` can only trade one of those two seats for the other.

- mechanism: Every body measured before s35 seated `q` in either $v1 (c1, c4, the banned multi-handle bodies) or $a0 (base, candidate.c), which made the seat look binary and made "shorten q's range" look like a free dial.
- probe: g1's disassembly, where `q` covers blocks 0+1 only -- the shortest range any measured body has given it while still spanning the block-1 join.
- result: KILLED. `q` takes **$a1** and `p` (the func_80077D00 return, which the target keeps in $a1) is displaced to **$a0**. The address object and `p` compete for the same two argument registers, so shortening `q`'s live range can lose the target's `p` seat at the same time it moves the address seat. This is the first body in the 193-form bank where that happens and it re-prices every future "shorten the range" proposal.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-05, variant g1, single declared `u8 *q` covering blocks 0+1, no FAKE construct present

## [s35] A spelling of block 0's mask that restores block 1's reload without introducing a second address pseudo lands BELOW 10, which would falsify the ledger's "one missing address allocno" factorisation.

- mechanism: The inherited frontier held that the block-1 reload (the target's `lbu $a0,0($v1)` at 0x80034FB4, our 175th lbu against the target's 176th) had been priced only by arithmetic on the objdump (s29), never by direct measurement, so a cheap spelling of it might exist.
- probe: Collected every banked body that actually emits the reload and re-priced them against today's chassis measurement of the floor: s17's `rejected/b1-read-direct-symbol-reload-priced-at-1-score11.c`, s34's f1 (anonymous read, named store), and s35's g1 (which lacks the reload and is therefore at 48 insns).
- result: KILLED. Every measured body that restores the reload is above the floor: 11 at 50 (s17), 14 at 49 (s34 f1); and g1 shows the reload is precisely the single instruction separating 48 from 49 in the anonymous quadrants. No spelling below 10 exists in the 193-form bank, so the factorisation stands and the reload is not separable value from the address seat.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-05 (floor control g0 = 10 at 49); bodies s17-b1-read-direct-symbol, s34-f1 and s35-g1, single declared `u8 *q`, no FAKE construct present

## [s36] MANDATED KILL RE-AUDIT: the s20/s25 dead round-trip and s33's c1 -- the two banked bodies closest to the target -- still measure what the ledger records on today's chassis, and the floor body still carries no FAKE construct.
- mechanism: An instance kill is only as good as the chassis and FAKE state it was measured under; the dispatch brief has printed 'measurement unavailable' for seven consecutive sessions, so the floor must be re-established by direct measurement each session.
- probe: Installed candidate.c (regenerated byte-for-byte as variant g0), rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c and rejected/s33-anon-symdiff-block0-blocks01-EXACT-score13.c at src/code6cac_b.c:3420 in turn and ran `sandbox func_80034F88 --disable all` on each; then ran tools/fake_ablate.py on candidate.c.
- result: CONFIRMED, all three. g0 = 10 at 49 build insns; rt = 10 at 49 (seventh consecutive session unchanged); c1 = 13 at 49, reproducing s33/s34 exactly. fake_ablate: 'no FAKE-annotated constructs found ... nothing to ablate'.
- verdict: CONFIRMED

## [s36] Some assignment of the four flag-byte access sites between the declared pointer `q` and the anonymous symbol-difference expression, other than the five combinations s33 already measured, reaches the target's two-seat register assignment.
- mechanism: tools/gcc-2.7.2/global.c:426 gives one pseudo one allocno one hard register, so the declared object's seat is decided by its live range and by the cse-created copy preference s33 traced to global.c:1709-1713. Which access sites q covers is the only free parameter left on the single-object chassis, and s33/s34 had measured only five of its settings.
- probe: Generated and measured the remaining quadrants (tmp/grind/func_80034F88/s35/gen.py + run.ps1) on the HEAD chassis: g1 (q covers blocks 0+1, blocks 2/3 anonymous), g5 (q covers block 0 only), g6 (g1 with q itself initialised through the punned expression), g7 (base coverage with every materialisation punned), g8 (g1 with two different anonymous spellings).
- result: KILLED. g1 = 28/48, g5 = 25/48, g6 = 27/48, g7 = 14/49, g8 = 28/48. With s33's c1-c5 and the base, the lattice is closed and its minimum is the base setting q:0123 = candidate.c = 10. All five bodies banked in rejected/s35-*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 (g0 control = 10, 49 build insns, rules_dropped 0); single declared `u8 *q` in every variant, no FAKE construct present

## [s36] Two anonymous address expressions written with DIFFERENT nestings escape cse's canonicalisation and each get their own full-address pseudo, which is the form the target's blocks 2 and 3 have.
- mechanism: The target materialises the flag byte's address three times into full-address registers (lui+addiu, then lbu/sb at offset 0). An anonymous expression cse cannot equate with its neighbour would have to be re-materialised -- the only route to a second full-address value that does not declare a second C object.
- probe: g8 spells block 2 as (s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70) and block 3 as (s32)&D_80106A73 + ((s32)&D_80106A70 - (s32)&D_80106A70), with q covering blocks 0+1; measured against g1, which uses one spelling in both blocks.
- result: KILLED. g8 = 28 at 48 build insns, the same score and instruction count as g1: cse canonicalises both nestings to the same address rtx, so the two sites share one value exactly as with a single spelling.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05; single declared `u8 *q` covering blocks 0+1, blocks 2/3 anonymous, no FAKE construct present

## [s36] The inherited claim that an anonymous address carrier costs +1 instruction every time its value must cross a code label.
- mechanism: s33's c2 (blocks 0 and 1 both anonymous) measured 50 insns against the target's 49, and s33/s34 generalised that to a per-label cost for anonymous carriers -- the statement that closed the anonymous route in the ledger's frontier.
- probe: Disassembled g1 (tmp/grind/func_80034F88/s35/g1.txt) and counted the instructions the anonymously-addressed blocks 2 and 3 emit, against the target's own blocks 2/3 in asm/funcs/func_80034F88.s.
- result: KILLED as a general statement. Each anonymous block emits lui + lbu %lo for the read and lui + sb %lo for the store = four instructions, exactly the count of the target's lui/addiu/lbu/sb. The carrier is instruction-count-NEUTRAL in blocks 2/3 and wrong only in FORM (two separate %hi materialisations instead of one reused full address). g1's single missing instruction (48 vs 49) is block 1's reload, the same one candidate.c lacks; the +1 in c2/d4 belongs specifically to block 1's post-join store.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-05, variant g1 (q covers blocks 0+1, blocks 2/3 anonymous), no FAKE construct present; disassembly of tmp/sandbox/func_80034F88/code6cac_b.o

## [s36] The address object's seat is a two-valued dial between $v1 and $a0, so live-range surgery on `q` can only trade one of those two seats for the other.
- mechanism: Every body measured before s35 seated q in either $v1 (c1, c4, the banned multi-handle bodies) or $a0 (base, candidate.c), which made the seat look binary and made 'shorten q's range' look like a free dial.
- probe: g1's disassembly, where q covers blocks 0+1 only -- the shortest range any measured body has given it while still spanning the block-1 join.
- result: KILLED. q takes $a1 and p (the func_80077D00 return, which the target keeps in $a1) is displaced to $a0. The address object and p compete for the same two argument registers, so shortening q's live range can lose the target's p seat at the same time it moves the address seat. First body in the 193-form bank where that happens; it re-prices every future 'shorten the range' proposal.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-05, variant g1, single declared `u8 *q` covering blocks 0+1, no FAKE construct present

## [s36] A spelling of block 0's mask that restores block 1's reload without introducing a second address pseudo lands BELOW 10, which would falsify the ledger's 'one missing address allocno' factorisation.
- mechanism: The inherited frontier held that the block-1 reload (the target's lbu $a0,0($v1) at 0x80034FB4, our 175th lbu against the target's 176th) had been priced only by arithmetic on the objdump (s29), never by direct measurement, so a cheap spelling might exist.
- probe: Collected every banked body that actually emits the reload and re-priced them against today's chassis measurement of the floor: s17's rejected/b1-read-direct-symbol-reload-priced-at-1-score11.c, s34's f1 (anonymous read, named store), and s35's g1 (which lacks the reload and is therefore at 48 insns).
- result: KILLED. Every measured body that restores the reload is above the floor: 11 at 50 (s17), 14 at 49 (s34 f1); g1 shows the reload is precisely the single instruction separating 48 from 49 in the anonymous quadrants. No spelling below 10 exists in the 193-form bank, so the factorisation stands and the reload is not separable value from the address seat.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-05 (floor control g0 = 10 at 49); bodies s17-b1-read-direct-symbol, s34-f1 and s35-g1, single declared `u8 *q`, no FAKE construct present

## Resolved in s37 (synthesis)

## [s37] A `static inline` helper function, called once per flag block, produces one address pseudo per call site from a SINGLE textual pointer declaration, and can therefore reach the target's two-address structure without the banned multi-handle construct.
- mechanism: GCC 2.7.2 inlines `inline`-keyword functions at -O2 (the build's CC_FLAGS are `-O2 -G0 ... -mel`); `integrate.c` copies the callee's RTL through `copy_rtx_and_substitute`, allocating FRESH pseudos for each inlined instance. Three calls to a helper holding `u8 *q = &D_80106A73;` therefore yield three address pseudos while the source declares one. The idiom is ordinary C and already ships in this repo at `src/main.c:1069` (`static inline void vmSetStartAddr`) and `src/main.c:2187` (`static inline void _memcpy`), so it is not a respelling of the banked TEXTUAL macro (`rejected/macro-respelling-of-banned-four-handle-score0-DO-NOT-SUBMIT.c`), which expands to four block-scoped declarations at the call sites.
- probe: Eleven bodies through `tmp/grind/func_80034F88/s37/run.ps1` (splice into src/code6cac_b.c:3420 -> `sandbox func_80034F88 --disable all` -> restore), generated by `s37/gen.py` and `s37/gen2.py`: helper carrying its own pointer (h1/h2/h3, and h8/h9/h11 in candidate.c's condition-parameter-reuse shape) and helper on the plain symbol (h4/h5/h7/h10), plus control h6 (no helper, plain symbol everywhere). h2's object disassembled with `s37/dis.sh`.
- result: KILLED as a route to the floor. h1 31/48, h2 30/49, h3 31/48, h4 26/47, h5 30/44, h6 29/47, h7 29/46, h8 24/49, h9 13/50, h10 28/48, h11 16/49 -- best 13, three points ABOVE the base's 10 and one instruction long. Inlining DOES fire (h2's object contains no `jal` to the helper and no out-of-line copy) and DOES emit a second full-address materialisation (`lui a1 / addiu a1` after block 1's store), but cse then merges the inlined copies into ONE store base in `$a1` while combine folds each inlined READ into a bare `lui vX` + `lbu 0(vX)` pair -- the target's reused-full-address read base is lost -- and the helper's address pseudo takes `$a1`, displacing `p` out of the target's `$a1` into `$a0` (the same double loss s35 measured for g1). Six of these are banked in `rejected/`. Side result: the condition-parameter-reused-as-merged-value shape is worth 11-18 points inside the family (h1 31 -> h9 13; h2 30 -> h8 24), so candidate.c's reuse idiom is load-bearing on its own.
- verdict: KILLED

## [s37] The frozen sanctioned family "duplicate-read into branch arms" restores the target's block-1 reload, which the base body is missing.
- mechanism: `.claude/rules/split-read-defeats-hoist.md` -- duplicating a read into each arm defeats GCC hoisting the read to the join. If the arm reads survive cse, block 1 emits its own `lbu` instead of consuming block 0's stored value.
- probe: h15 (`if (c) { c = *q | 1; } else { c = *q; }` on block 1 only, no pre-read), h14 (pre-read kept for the else arm), h16/h17 (both shapes on all three blocks); objdump of h15 compared to the base body's with `diff`.
- result: KILLED, and byte-neutrally so. h15 measures 10 at 49 and its disassembly is BIT-IDENTICAL to the base body's (`diff tmp/grind/func_80034F88/s37/h0.txt tmp/grind/func_80034F88/s37/h15.txt` is empty) -- cse collapses both arm reads back onto block 0's stored value, so the family cannot reach the reload here. h14 19/51, h16 29/55, h17 25/53.
- verdict: KILLED

## [s37] Feeding the trailing copy loop off `q` raises the pointer allocno's reference count enough to reorder it ahead of block 0's byte value in `allocno_compare`, winning it `$v1`.
- mechanism: `tools/gcc-2.7.2/global.c:635` sorts allocnos by `floor_log2(n_refs) * n_refs / live_length`; the copy loop adds three references to `q` per iteration's worth of RTL. The spelling is the idiom the file itself already uses at `src/code6cac_b.c:4044` (`(&D_80106A73) - 3`), so it is ordinary C, not a coercion.
- probe: h13 (`q[i - 3] = ...`) and h12 (`*(q - 3 + i) = ...`) on the candidate chassis.
- result: KILLED. Both measure 30 at 48. The loop gives up its own `lui %hi(D_80106A70) / addu at,at,v1 / sb %lo(D_80106A70)(at)` materialisation -- three instructions of the target's own loop -- which costs far more than any priority lift buys.
- verdict: KILLED

## [s37] CONFIRMED (re-derivation, not a new claim): the single-`q` chassis cannot seat the address correctly because blocks 0/1 and blocks 2/3 need DIFFERENT hard registers.
- mechanism: The base body's `.greg` shows `q` = pseudo 74 seated in `$a0` and block 0's byte value = pseudo 73 seated in `$v1`, with `73 conflicts: 72 73 2 29` -- 73 and 74 do NOT conflict, so `q` loses `$v1` purely to allocation ORDER: `allocno_compare` (global.c:635) ranks the short-lived value above the long-lived pointer, 73 is allocated first, `$v0` is blocked by the live call return, and 73 takes `$v1`. The target needs the address in `$v1` for blocks 0/1 and in `$a0` for blocks 2/3; `global.c:426` gives one pseudo exactly one hard register.
- probe: `pwsh tools/grinder/dump.ps1 func_80034F88` with the base body installed; `tmp/grind/func_80034F88/dumps/code6cac_b.greg`; plus the instruction-level diff of h0's objdump against `asm/funcs/func_80034F88.s`.
- result: CONFIRMED from the dispositions and the priority formula rather than from the conflict-graph argument s31/s32 used, and it adds the missing WHY (order, not conflict). It also localises the residual exactly: blocks 2/3 are register-exact, and the whole 10 points are a `$v1 <-> $a0` rotation in blocks 0/1 plus the target's free block-1 reload sitting in a load-delay slot where the base body emits `nop`.
- verdict: CONFIRMED

## Frontier after s37

- **F1.** No lever measured so far changes `allocno_compare`'s ORDERING between block 0's byte-value allocno (73) and the pointer allocno (74) without also changing the emitted instructions. The formula is `floor_log2(n_refs)*n_refs/live_length`, so the two free parameters are the VALUE allocno's live length (lengthen it and its priority falls) and its reference count. Every reference costs an instruction, but LIVE LENGTH may not: a body in which block 0's masked byte value is still live later in the function (e.g. consumed by block 1's merge instead of being re-read) stretches 73's range at no instruction cost, and 73 in `$a0` with `q` in `$v1` is exactly the target's assignment. Nothing in the bank has attacked 73's live length directly -- every previous probe attacked `q`.
  *Next probe:* measure bodies that extend block 0's value pseudo across the block-1 branch (single shared value local for blocks 0 and 1; block 1's merge sourced from block 0's masked value rather than from a fresh read) and read `.greg` for the ordering of 73 vs 74, not just the score.
- **F2.** The block-1 reload is now known to cost ZERO instructions (it fills the load-delay slot of `lw $v0,0x20($a1)` that the base body wastes on `nop`). Every construct measured to emit it so far also changes the address rtx and loses the seat. The untested combination is a body that keeps the base's address rtx AND creates a load-delay slot that only a reload can fill -- i.e. re-ordering block 1's `lw p[8]` so the byte read is the only insn available to the scheduler for the slot.
  *Next probe:* vary the statement ORDER inside block 1 (condition computed before vs after the byte read, `p[8]` hoisted into a local before block 0) and read `.sched2`/`.dbr` for what fills the slot.
- **F3.** Ladder accounting: this is session fifteen of cycle 2; five flat sessions remain before the owner directive 2026-09-02 permits any disposition. If those are spent without a drop, the record to file is `LADDER EXHAUSTED (non-endgame residual, floor 10)`, citing the cse enumeration (s27/s28), the reload pricing (s29/s37), F1 byte-neutrality (s30), the allocator restatement (s31/s32/s37), s33's two-seats-from-one-object proof, s34's reorg.c:3442 result, s35's end-to-end q-coverage lattice, and s37's inline-helper kill.

## [s37] F1, PROBED AND CLOSED WITHIN THE SAME SESSION -- demoting block 0's byte-value allocno below the pointer allocno in `allocno_compare` by lengthening its live range wins `q` the `$v1` seat.
- mechanism: `tools/gcc-2.7.2/global.c:635` sorts allocnos by `floor_log2(n_refs)*n_refs/live_length`; block 0's short-lived value (pseudo 73) therefore outranks the long-lived pointer (74), is allocated first, finds `$v0` blocked by the live call return and takes `$v1`. Reference count cannot be raised for free, so live length is the only free parameter.
- probe: k1 (block 0's masked value kept in a local and merged by block 1, which then does not re-read the byte at all), k2 (that shared value carried on into block 2), k3 (p[8] hoisted into a local before block 0), k4 (block 1's byte read moved ahead of its p[8] condition read), k5 (block 0's mask split into `m = *q; m = m & 0xF8; *q = m;`). Scores plus objdump diffs against the base body.
- result: KILLED for these five spellings. k1, k4 and k5 all tie the floor at 10/49 and disassemble BIT-IDENTICALLY to the base body (empty `diff`) -- lengthening the value's C-level live range does not lengthen its RTL live range, because cse already forwards block 0's stored value into block 1 in the base body, making the two spellings the same RTL. The two bodies that genuinely do change 73's range change the instruction stream and score worse: k2 13/49, k3 20/47. The bit-identical-tie count for this basin is now ten bodies (s31's seven plus h15, k1, k4, k5).
- verdict: KILLED

## Frontier after s37 (supersedes the F1/F2 list written earlier in this session)

- **F1'.** The allocno-order lever is byte-inert from statement structure (k1/k4/k5 above), so the `$v1 <-> $a0` rotation cannot be fixed by re-shaping block 0/1's value flow. What has NOT been measured is the other side of `allocno_compare`: making the POINTER allocno's live range SHORTER without splitting it into a second object -- e.g. by moving `q`'s first assignment as late as legal while it still covers all four sites. s32 measured range surgery that moved q's first assignment into block 1 (which loses block 0), and s35's g1 measured a q covering only blocks 0/1 (which cost `p` its `$a1` seat); the untried point is a single q whose SET is late but whose USES still span all four blocks.
  *Next probe:* bodies where `q = &D_80106A73;` is placed at the last legal point before block 0's mask (after the `p[8]` read, inside a tighter scope) and where the mask is spelled to defer the address's first reference; read `.greg` for the allocno ORDER line (`;; N regs to allocate: ...`) rather than the score alone, since a reordering that does not change scores still changes the search.
- **F2'.** k3 shows the scheduling probe (hoisting `p[8]`) costs two instructions, so the load-delay slot that the target fills with its block-1 reload cannot be created by moving the `lw`. The slot exists in the base body already (it holds a `nop`); what is missing is an insn cse is willing to leave there. Everything measured that emits the reload also changes the address rtx (s34 f1/f2, s17, s35 g1).
  *Next probe:* read `.dbr` on the base body to see which insns `reorg.c` considered for that slot and why none qualified -- the ledger has never dumped `.dbr` for the BASE body (only for s34's e1), and it is the pass that would place a reload if one existed.
- **F3.** Ladder accounting: this is session fifteen of cycle 2; five flat sessions remain before the owner directive 2026-09-02 permits any disposition. If those are spent without a drop, the record to file is `LADDER EXHAUSTED (non-endgame residual, floor 10)`, citing the cse enumeration (s27/s28), the reload pricing (s29/s37), F1 byte-neutrality (s30), the allocator restatement (s31/s32/s37), s33's two-seats-from-one-object proof, s34's reorg.c:3442 result, s35's end-to-end q-coverage lattice, and s37's inline-helper and allocno-order kills.

## [s37] A `static inline` helper function called once per flag block gives each call site its own address pseudo from a SINGLE textual pointer declaration, so eleven such bodies on this chassis reach the base floor of 10.
- mechanism: GCC 2.7.2 inlines `inline`-keyword functions at -O2 (build CC_FLAGS `-O2 -G0 ... -mel`); integrate.c's copy_rtx_and_substitute allocates FRESH pseudos per inlined instance, so three calls to a helper holding `u8 *q = &D_80106A73;` yield three address pseudos while the source declares one. The idiom is ordinary C and already ships at src/main.c:1069 and src/main.c:2187, and is distinct from the banked TEXTUAL macro (rejected/macro-respelling-of-banned-four-handle-score0-DO-NOT-SUBMIT.c) that expands to four block-scoped declarations.
- probe: Eleven bodies spliced into src/code6cac_b.c:3420 and scored with `sandbox func_80034F88 --disable all` via tmp/grind/func_80034F88/s37/run.ps1 (generators gen.py, gen2.py): pointer-carrying helper (h1/h2/h3 fresh-result shape; h8/h9/h11 candidate.c's condition-parameter-reuse shape) and plain-symbol helper (h4/h5/h7/h10), plus control h6 (no helper, plain symbol at all four sites). h2's object disassembled with s37/dis.sh.
- result: Best is h9 at 13 (50 insns) -- three points above the base floor and one instruction long. Full table: h1 31/48, h2 30/49, h3 31/48, h4 26/47, h5 30/44, h6 29/47, h7 29/46, h8 24/49, h9 13/50, h10 28/48, h11 16/49. Inlining does fire (h2's object has no jal to the helper and no out-of-line copy) and does emit a second full-address materialisation (lui a1 / addiu a1 after block 1's store), but cse merges the inlined copies into ONE store base in $a1 while combine folds each inlined read into a bare lui + lbu %lo pair, losing the target's reused-full-address read base; and the helper's address pseudo takes $a1, displacing p out of the target's $a1 into $a0. Side result: the condition-parameter-reuse shape is worth 11-18 points inside the family (h1 31 -> h9 13; h2 30 -> h8 24). Four members banked in rejected/s37-inline-helper-*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 (h0 control re-measured 10 at 49, rules_dropped 0); eleven inline-helper bodies, single declared pointer per helper, no FAKE construct present (fake_ablate reports none in candidate.c)

## [s37] Demoting block 0's byte-value allocno below the pointer allocno in allocno_compare, by lengthening the value's live range at zero instruction cost, wins `q` the $v1 seat on this chassis.
- mechanism: tools/gcc-2.7.2/global.c:635 sorts allocnos by floor_log2(n_refs)*n_refs/live_length. The base body's .greg shows block 0's value (pseudo 73) allocated FIRST, finding $v0 blocked by the live call return and taking $v1, while q (pseudo 74) is reached fifth and takes $a0 -- and 73 and 74 do not conflict, so the seat is lost to ORDER. Reference count cannot be raised for free, leaving live length as the only free parameter.
- probe: Five bodies from tmp/grind/func_80034F88/s37/gen4.py: k1 (block 0's masked value kept in a local and merged by block 1, which then never re-reads the byte), k2 (that shared value carried on into block 2), k3 (p[8] hoisted into a local before block 0), k4 (block 1's byte read moved ahead of its p[8] condition read), k5 (mask split into `m = *q; m = m & 0xF8; *q = m;`). Scored with sandbox, then each tying body disassembled and diffed against the base body's objdump.
- result: k1, k4 and k5 all tie at 10/49 and disassemble BIT-IDENTICALLY to the base body (diff h0.txt against k1.txt / k4.txt / k5.txt all empty) -- lengthening the value's C-level live range does not lengthen its RTL live range, because cse already forwards block 0's stored value into block 1 in the base body, making the spellings the same RTL. The two bodies that do change 73's range change the instruction stream and score worse: k2 13/49, k3 20/47. The bit-identical-tie count for this basin is now ten bodies (s31's seven plus h15, k1, k4, k5). Three banked in rejected/s37-block0-*.c and rejected/s37-p8-hoisted-*.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05; single declared `u8 *q`, no FAKE construct present; k1/k4/k5 verified bit-identical to the base body by objdump diff

## [s37] The frozen sanctioned family 'duplicate-read into branch arms' restores the target's block-1 reload, which the base body is missing.
- mechanism: .claude/rules/split-read-defeats-hoist.md -- duplicating a read into each arm defeats GCC hoisting the read to the join, so block 1 would emit its own lbu instead of consuming block 0's stored value.
- probe: h15 (`if (c) { c = *q | 1; } else { c = *q; }` on block 1 only, no pre-read), h14 (pre-read kept for the else arm), h16 and h17 (both shapes applied to all three flag blocks); h15's objdump diffed against the base body's.
- result: h15 measures 10 at 49 and its disassembly is bit-identical to the base body's (empty diff) -- cse collapses both arm reads back onto block 0's stored value, so the family cannot reach the reload here. h14 19/51, h16 29/55, h17 25/53. h16 banked at rejected/s37-dupread-both-arms-all-three-blocks-score29.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05; single declared `u8 *q`, no FAKE construct present

## [s37] Feeding the trailing copy loop off `q` (`q[i - 3]`, the idiom src/code6cac_b.c:4044 already uses) raises the pointer allocno's reference count enough to reorder it ahead of block 0's byte value in allocno_compare.
- mechanism: global.c:635's priority key is floor_log2(n_refs)*n_refs/live_length, and the copy loop adds references to q; the spelling is ordinary pointer arithmetic already present in this file, not a coercion.
- probe: h13 (`q[i - 3] = ...`) and h12 (`*(q - 3 + i) = ...`) on the candidate chassis, scored with sandbox.
- result: Both measure 30 at 48. The loop gives up its own `lui %hi(D_80106A70) / addu at,at,v1 / sb %lo(D_80106A70)(at)` materialisation -- three instructions the target does contain -- which costs far more than any priority lift buys. Banked at rejected/s37-q-as-copy-loop-base-score30.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05; single declared `u8 *q`, no FAKE construct present

## [s37] The base body's residual is confined to blocks 0/1 and is a $v1<->$a0 rotation plus a block-1 reload that occupies an already-paid-for load-delay slot; the rotation is caused by allocation ORDER, not by a register conflict.
- mechanism: Instruction-level diff of the base body's own objdump against asm/funcs/func_80034F88.s, plus the base body's .greg: ';; 9 regs to allocate: 73 78 82 86 74 77 81 85 72'; ';; 73 conflicts: 72 73 2 29'; ';; Register dispositions: 72 in 5  73 in 3  74 in 4'. q is pseudo 74 in $a0 and block 0's byte value is pseudo 73 in $v1; they do not conflict, so 73 wins $v1 only because allocno_compare (global.c:635) puts it first.
- probe: `pwsh tools/grinder/dump.ps1 func_80034F88` with the base body installed (dump kept at tmp/grind/func_80034F88/dumps/code6cac_b.greg), and `s37/dis.sh h0` compared line by line with the target listing.
- result: CONFIRMED. Blocks 2/3 are register-exact. The target's `lbu $a0,0($v1)` at 80034FB4 sits in the load-delay slot of `lw $v0,0x20($a1)` where the base body emits a bare `nop`, so the reload costs zero instructions -- a stronger statement than s29's arithmetic pricing. Combined with global.c:426 (one pseudo, one hard register), this re-derives the standing factorisation independently: the chassis needs the address in $v1 for blocks 0/1 and in $a0 for blocks 2/3, which one pseudo cannot supply, and it supplies the missing WHY that s31/s32 left implicit.
- verdict: CONFIRMED

==== s38 (synthesis) ====

MERGED ATTACK AFTER s38 (this supersedes the s37 statement of the mechanism).

The residual is 10 differing instructions, all in blocks 0/1, and it is a pure
$v1 <-> $a0 rotation of (blocks-0/1 address, blocks-0/1 byte value) plus a
`nop`/`lbu` swap that costs nothing (s37 section 1; re-confirmed by direct diff
this session -- b0 and rt differ by exactly that one instruction).

The CAUSE is now read from RTL rather than inferred, and s37's version of it is
withdrawn:

  * NOT allocno priority.  Pseudo 73, the top-priority allocno seated in $v1
    (pri 47142), is the trailing copy loop's induction variable `i`, which the
    TARGET also seats in $v1.  It never competed with `q`, and the two do not
    conflict.
  * IT IS A LOCAL-ALLOC HARD-REGISTER SEAT.  Block 0's masked byte is a
    single-basic-block quantity, so local-alloc assigns it the hard register $v1
    before global-alloc runs -- visible in the .greg RTL as
    `(set (reg:QI 3 v1) (mem:QI (reg/v:SI 4 a0)))`.  `q` is live across it, so
    `q` carries a hard-reg-3 conflict and `find_reg` can never give it $v1.
    $v0 is unavailable to that quantity because the call return is still live
    (the `a1 = v0` copy is scheduled after block 0's store).
  * The priority dial is measured impotent against that conflict: in `rt`, `q`
    rises to pri 15000 and sorts SECOND overall (ord 4 -> 1) and still gets $a0.

Consequence for the search: every remaining lever on the blocks-0/1 seat has to
act on LOCAL-alloc's choice for block 0's byte quantity, not on global-alloc's
sort.  Bodies that lengthen/shorten C-level live ranges (s37 k1/k4/k5) are
byte-inert because cse normalises them to the same RTL; bodies that add
references (rt) move the sort without moving the seat.  The two seats still need
two pseudos (global.c:426) and one C object gives one pseudo, so the standing
factorisation is unchanged -- but the next probe is now specific and cheap.

FRONTIER RESET (strongest three for the next ladder pass):

1. LOCAL-ALLOC, NOT GLOBAL-ALLOC, IS THE CONTESTED PASS.  Find an ordinary-C
   spelling of block 0's read-mask-store whose temporary is NOT a block-local
   quantity that local-alloc seats in hard $v1 before global runs (or one whose
   copy-preference steers it to $a0, as the target has it).
   Mechanism: local-alloc.c's `find_free_reg` / `qty_phys_reg` assignment runs
   before global.c; the resulting `(reg:QI 3 v1)` in the pre-global RTL is what
   creates q's hard-reg-3 conflict row.
   Next probe: the instrumented cc1 prints the quantity table under
   `BB2_QTY_DEBUG=1` (`QTYDBG blk=... qty=... reg1=... birth=... death=...
   refs=... got=...`, local-alloc.c:1585; the local-alloc instrumentation is
   GRANTED for this function by decisions.md:14784).  The hook has NO function
   tag, so `tmp/grind/func_80034F88/s38/qtydbg_b0.txt` covers the whole TU and
   the func_80034F88 rows cannot be picked out; the next session should compile
   a REDUCED TU (this function alone plus its externs) so the QTYDBG rows are
   unambiguous, identify block 0's quantity, and read what determines `got=3`.
   Then look for a C shape that changes it.

2. THE PUN-FREE BODY IS THE REAL CANDIDATE, AND IT NEEDS A HEADER LINE.
   `memory/grind/func_80034F88/candidate_arraydecl_pun_free.c` is bit-identical
   to candidate.c and carries no declaration pun, but only compiles with
   `include/code6cac.h:472` changed to `extern u8 D_80106A70[3];`.
   Mechanism: none -- measured codegen-neutral (empty objdump diff).
   Next probe: none needed; carry it forward.  If a future session ever reaches
   distance 0, submit THAT body and file the header line as an integration
   handoff, because the brief's auto-scan FAILs the punned spelling at layer 1
   regardless of score.

3. LADDER ACCOUNTING.  s38 is session sixteen of cycle 2; four flat sessions
   remain before the owner directive 2026-09-02 permits any disposition.
   Mechanism: owner directive 2026-09-02 (decisions.md 'foreclosure mechanics')
   requires 20 flat sessions and >= 6 distinct modalities; the modality
   condition was met at s31.
   Next probe: when the driver assigns `escalation`, file the LADDER EXHAUSTED
   (non-endgame residual, floor 10) foreclosure record citing the cse
   enumeration (s27/s28), the reload pricing (s29, direct-diff confirmed s38),
   F1 byte-neutrality (s30), the allocator restatement (s31/s32), s33's
   two-seats-individually-reachable result, s34's reorg.c:3442 dump proof,
   s35's end-to-end q-coverage lattice, s37's inline-helper family, and s38's
   correction that the barrier is a local-alloc hard-reg seat rather than a
   global priority order.

## [s38] Pseudo 73 -- the top-priority allocno (nrefs=11, livelen=7, pri=47142) that takes $v1 in the base body, which s37 identified as 'block 0's byte value' and made the centre of its allocation-ORDER explanation -- is actually `i`, the induction variable of the trailing three-byte copy loop.
- mechanism: flow.c weights loop-body references by loop depth, so a 3-iteration loop counter with 4 textual references reaches nrefs=11 over livelen=7 and tops allocno_compare's sort (tools/gcc-2.7.2/global.c:635). The target seats `i` in $v1 as well (asm/funcs/func_80034F88.s, addu $v1,$zero,$zero at 80035014 and addiu $v1,$v1,0x1 at 8003502C), so the seat is correct and uncontested.
- probe: pwsh tools/grinder/dump.ps1 func_80034F88 with the candidate body installed, then read every mention of reg 73 in the func's segment of tmp/grind/func_80034F88/dumps/code6cac_b.lreg: insn 122 (set reg 73 ...), insn 142 (set (mem/s:QI (plus:SI (reg 73) ...))), insn 148 (set reg 73 (plus reg 73 ...)), and (lt:SI (reg 73) ...) -- the loop's init, indexed store, increment and exit test.
- result: CONFIRMED by direct RTL read. s37's mechanism ('the short-lived block-0 value outranks the long-lived pointer, so q loses $v1 to allocation order') describes a competition that does not exist: allocnos 73 and 74 do not conflict, and 73 holds a register the target also gives to the same value.
- verdict: CONFIRMED

## [s38] q (pseudo 74) cannot receive $v1 in the single-pointer-object chassis because LOCAL-alloc seats block 0's masked byte in the hard register $v1 before global allocation runs, giving q a hard-reg-3 conflict; the barrier is that conflict, not global allocno priority.
- mechanism: Block 0's masked byte is a single-basic-block quantity, so local-alloc assigns it a hard register (local-alloc.c find_free_reg / qty_phys_reg) before global.c is entered. $v0 is unavailable to it because the call return is still live -- the `a1 = v0` copy (insn 11) is scheduled after block 0's store (insn 20) -- so it takes $v1. q is live across it and therefore appears in the .greg conflict row `;; 74 conflicts: 72 74 77 78 81 82 85 86 2 3 29` with hard reg 3, which find_reg honours.
- probe: Read the pre-global RTL in tmp/grind/func_80034F88/dumps/code6cac_b.greg: insn 17 `(set (reg:QI 3 v1) (mem:QI (reg/v:SI 4 a0)))`, insn 18 `(set (reg:SI 3 v1) (and:SI (subreg:SI (reg:QI 3 v1) 0) (const_int 248)))`, insn 20 `(set (mem:QI (reg/v:SI 4 a0)) (subreg:QI (reg:SI 3 v1) 0))` -- block 0's value is already a hard register there. Cross-checked against the instrumented-cc1 allocno tables in tmp/grind/func_80034F88/s38/allocdbg_b0.txt and allocdbg_rt_c1.txt.
- result: CONFIRMED. This replaces the inherited priority account with a mechanically different one, and it re-points the search: priority is a dial reachable from C (reference count, live length) while a hard-register conflict created by an earlier pass is not.
- verdict: CONFIRMED

## [s38] Raising q's global allocno priority so that it is allocated before the competing allocnos does not change its hard register on this chassis: in the banked round-trip body q reaches nrefs=15, livelen=30, pri=15000 and moves from sort position 4 to position 1, and still receives $a0.
- mechanism: allocno_compare (tools/gcc-2.7.2/global.c:635) decides only the ORDER in which find_reg is called; find_reg still refuses any hard register in the allocno's hard-reg conflict set, and q's conflict with reg 3 is created earlier by local-alloc (hypothesis 2), so no sort position makes $v1 eligible.
- probe: Installed rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c as variant rt and ran the instrumented cc1 with BB2_ALLOC_DEBUG=1 (tmp/grind/func_80034F88/s38/allocdbg_rt_c1.txt); compared its allocno table and dispositions against the base body's (allocdbg_b0.txt). Sandbox: rt = 10 at 49 insns, unchanged.
- result: KILLED as a seat lever on this chassis. rt's sort position is 1 of 9 (behind only the loop counter) and its hardreg is still 4 ($a0). The same body's objdump differs from the base's by exactly one instruction (our `nop` becomes `lbu $v1,0($a0)`), confirming by direct diff -- not by arithmetic, as s29 did -- that the block-1 reload is instruction-free and lands in the rotated register.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 (b0 = 10, 49/49, rules_dropped 0); variant rt = the banked dead-arithmetic round-trip, single declared `u8 *q`, no FAKE-annotated construct present (fake_ablate: nothing to ablate)

## [s38] Declaring D_80106A70 as an array (`extern u8 D_80106A70[3];` at include/code6cac.h:472) and spelling the trailing copy loop `D_80106A70[i] = *((u8 *)p + i + 0x17);` removes candidate.c's declaration pun at zero codegen cost.
- mechanism: None at codegen level -- the array-element access and the pointer-arithmetic pun lower to the same address rtx, so the emitted stream is unchanged. The change matters for review, not for bytes: the dispatch brief's auto-scan treats the punned spelling as a layer-1 FAIL condition and names the declaration as the sanctioned fix.
- probe: Edited include/code6cac.h:472 to the array declaration, spliced variant a1 (base body with the indexed loop) into src/code6cac_b.c, and ran `sandbox func_80034F88 --disable all` -> 10 at 49 insns; then diffed the two objdumps (`diff tmp/grind/func_80034F88/s38/b0.txt tmp/grind/func_80034F88/s38/a1.txt`) -> empty. Header restored to HEAD afterwards.
- result: CONFIRMED, bit-identical. D_80106A70 is referenced in exactly two places project-wide (the header line and this loop), so the fix is a one-line, one-symbol integration handoff. Banked as memory/grind/func_80034F88/candidate_arraydecl_pun_free.c; any future candidate-ready for this function should be that body plus the header line rather than candidate.c.
- verdict: CONFIRMED

## [s38] Spelling the copy loop's SOURCE operand as an index, `((u8 *)p)[i + 0x17]` instead of `*((u8 *)p + i + 0x17)`, measures 12 at 49 instructions on the array-declared chassis.
- mechanism: The indexed source form changes how the loop's address arithmetic is associated, costing two register matches at the same instruction count; the destination-side array spelling is neutral (hypothesis 4) but the source-side one is not.
- probe: Variant a2 (array declaration + indexed destination + indexed source) spliced and scored with `sandbox func_80034F88 --disable all`.
- result: KILLED for this chassis; keep the pointer-arithmetic spelling on the source operand. Banked as rejected/s38-loop-source-byte-array-indexed-score12.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 with include/code6cac.h:472 temporarily set to `extern u8 D_80106A70[3];`; single declared `u8 *q`, no FAKE-annotated construct present

==== s39 (synthesis) — FRONTIER RESET ====

The residual has been RESTATED this session and the old frontier is retired.
Everything below supersedes the s34–s38 frontier items, which were all phrased
around "q is ineligible for $v1 because a local-alloc pseudo is seated there".
That premise is false for the round-trip spelling of block 0 (see evidence.md
"==== s39 (synthesis) ====" section 1): with
`m = *q; m = m & 0xF8; *q = m;` the entry block contains no local-alloc quantity
and q's `.greg` conflict row no longer carries hard reg 3. The new candidate.c
carries that spelling.

WHAT IS LEFT, EXACTLY. `global.c:635 allocno_compare` sorts allocnos on
`floor_log2(nrefs) * nrefs / live_length`. On the new chassis:

    m (block 0/1 byte value)  nrefs=6   livelen=9   pri=13333   -> $v1
    q (&D_80106A73)           nrefs=10  livelen=28  pri=10714   -> $a0

The target wants q in $v1 and m in $a0. Nothing else in the function differs:
blocks 2/3, the trailing copy loop and the epilogue are register-exact, and the
target's block-1 reload is instruction-free (it fills the load-delay slot our
build wastes on a `nop` — s38 direct diff). So the entire 10-point residual is
"make pri(q) > pri(m)", and the four ways the formula allows it are:

    q nrefs  >= 13  at livelen 28  -> 13928
    q livelen <= 22 at nrefs 10    -> 13636
    m nrefs  <= 4   at livelen 9   ->  8888
    m livelen >= 12 at nrefs 6     -> 10000

All four were probed this session and all four are pinned by a DIFFERENT pass
(evidence.md section 3). The three frontier items below are the routes that
remain, in priority order.

--- FRONTIER 1 (strongest): find a C form in which block 1 consumes m but m's
reference count is 4, not 6.
Mechanism: m's six references are the `zero_extend` def (1), the in-place mask
`m = m & 0xF8` (dest + src = 2), the store `*q = m` (1) and block 1's two arm
uses (2). Dropping to 4 puts pri(m) at 8888, below q's 10714, and (per
global.c:426 one-pseudo-one-hardreg and the ascending free-register scan) q then
takes $v1 and m takes $a0 — the target's pairing. The two obvious reductions are
both measured dead: interposing a copy is coalesced (z2, identical table), and
dropping block 1's use makes m single-basic-block so local-alloc re-seats it in
$v1 (t1, 11 at 50). Untried: a spelling in which the mask is applied WITHOUT a
second reference to m and WITHOUT spawning a second pseudo — i.e. some expression
whose RTL is one `zero_extend` insn followed by one AND insn that reads a
different rtx but writes m. Also untried: whether a `u8`-typed m changes the ref
accounting (all previous u8 experiments were on the `*q &= 0xF8` chassis and
scored 19–23, so they never reached this question).

--- FRONTIER 2: shorten q's live length to <= 22 without removing q from any
block.
Mechanism: live_length in global.c is accumulated over the insns the allocno is
actually LIVE, so dead gaps do not count. q is dead between block 1's store and
block 2's re-materialisation and again between blocks 2 and 3, yet still measures
28. Six more dead insns inside q's span would give pri 13636 > 13333. The only
lever that lengthens a dead gap is moving work OUT of q's live range; the whole
function is 49 instructions and blocks 2/3 are already exact, so the only place
with slack is between block 0's store and block 1's store. Probe: measure
live_length (not score) for bodies that move the p[8] read, the `a1 = v0` copy
and the block-1 condition around, and read the number off `ad.sh` rather than
inferring it from the objdump — x1 showed the number can move by 2 in one step,
and it moved the wrong way, so the derivative is real and steerable.

--- FRONTIER 3 (bookkeeping, not a probe): ladder accounting.
This is session seventeen of cycle 2; the six-modality condition was met at s31.
THREE flat sessions remain before owner directive 2026-09-02 permits any
disposition. If the driver assigns `escalation` after that count is met, the
record to file is the LADDER EXHAUSTED (non-endgame residual, floor 10) form —
but note that s39 materially changed the causal story, so any such record must
cite the priority-gap statement above, NOT the retired "q is ineligible for $v1"
one, and must state the four numeric flip thresholds and which pass pins each.

## [s39] Spelling block 0's read-modify-write as `m = *q; m = m & 0xF8; *q = m;` (one named SImode variable, loaded via zero_extend and masked in place) removes hard register 3 from q's .greg conflict row, so q becomes eligible for $v1.
- mechanism: `*q &= 0xF8;` expands to a QImode load into its own pseudo plus an SImode AND through a subreg (insns 17/19/22 of the k1 dump). That QImode pseudo is referenced in exactly one basic block, so it never enters the global allocno table; local-alloc seats it, and with $v0 held by the call return and no REG_ALLOC_ORDER defined for MIPS in tools/gcc-2.7.2/config/mips/mips.h the ascending free-register scan gives it $v1. q, live across it, inherits the hard-reg-3 conflict. The round-trip spelling emits one `zero_extend:SI (mem:QI)` into the user pseudo and one in-place AND, so the entry block contains no local-alloc quantity at all.
- probe: Installed the s38 candidate (b0) and four round-trip spellings (v1..v4) via tmp/grind/func_80034F88/s39/run.ps1, then `pwsh tools/grinder/dump.ps1 func_80034F88` on each and read the .greg conflict rows and the pre-allocation RTL.
- result: b0/k1 print `;; 75 conflicts: 72 74 75 77 79 80 83 84 2 3 29` (hard reg 3 present). v2/v4 print `;; 75 conflicts: 72 74 75 77 79 80 83 84 2 29` -- hard reg 3 gone. All of v1..v4 measure 10 at 49 and disassemble bit-identically to b0, so the change is invisible to the score and was missed by 19 sessions of score-only ranking. Artifacts: s39/{k1,v2,v4}.greg, s39/{v2,v4}.lreg.
- verdict: CONFIRMED

## [s39] On the round-trip chassis, q's reference count cannot be raised above 10 by adding a redundant `q = &D_80106A73;` re-materialisation in block 1.
- mechanism: cse deletes a set whose destination already holds the same value, so the extra reference never reaches flow.c's reg_n_refs count that global.c:635 allocno_compare divides by live_length.
- probe: Variant w1 = v2 plus `q = &D_80106A73;` at the head of block 1; scored with the sandbox and dumped with the instrumented cc1 (BB2_ALLOC_DEBUG=1, tmp/grind/func_80034F88/s39/ad.sh).
- result: w1 = 10 at 49, and its allocno table is numerically identical to v2's: q still ord=5 pseudo=75 hardreg=4 nrefs=10 livelen=28 pri=10714. The added statement bought zero references.
- verdict: KILLED
- kill_scope: instance
- measured_on: round-trip chassis (candidate.c as rewritten this session), single declared `u8 *q`, no FAKE constructs present; fake_ablate.py reports nothing to ablate

## [s39] On the round-trip chassis, m's reference count of 6 cannot be reduced by interposing a copy of m for block 1's arms to read.
- mechanism: The copy is coalesced away before allocation, so m's references are unchanged and pri(m) stays at 13333, above q's 10714.
- probe: Variant z2 = v2 with block 1 spelled `v = m; c = p[8] & 1; if (c) c = v | 1; else c = v; *q = c;`; sandbox plus instrumented-cc1 allocno table.
- result: z2 = 10 at 49, allocno table identical to v2's (m ord=4 nrefs=6 livelen=9 pri=13333; q ord=5 nrefs=10 livelen=28 pri=10714).
- verdict: KILLED
- kill_scope: instance
- measured_on: round-trip chassis (candidate.c as rewritten this session), single declared `u8 *q`, no FAKE constructs present

## [s39] Removing block 1's use of m -- by having block 1 read D_80106A73 by symbol so cse cannot forward block 0's store -- drops m to 4 references but re-creates the local-alloc $v1 seat, because m then lives in a single basic block again.
- mechanism: local_alloc only handles quantities referenced in one basic block; once block 1 stops consuming m, m qualifies and is seated in $v1 before global-alloc runs, which restores the hard-reg-3 conflict on q that the round-trip spelling removed.
- probe: Variant t1 = v2 with block 1's load spelled `v = D_80106A73;` and its store still through q; sandbox plus instrumented-cc1 allocno table.
- result: t1 = 11 at 50 insns. The allocno table has only 8 entries and m is absent from it entirely; q is back at ord=4 hardreg=4 nrefs=10 livelen=29 pri=10344. This is the second horn of the dichotomy: m consumed by block 1 => global allocno at pri 13333 which outranks q; m not consumed => local quantity which local-alloc seats in $v1 first.
- verdict: KILLED
- kill_scope: instance
- measured_on: round-trip chassis (candidate.c as rewritten this session), single declared `u8 *q`, no FAKE constructs present

## [s39] Computing block 1's condition between m's load and m's mask, to stretch m's live length to 12 or more and demote it below q in allocno_compare, moves m's live length in the opposite direction.
- mechanism: live_length is measured after the first scheduling pass, and the scheduler compacts the reordered range rather than preserving the source-level separation.
- probe: Variants x1 (`m = *q; c0 = p[8] & 1; m = m & 0xF8; *q = m;` then the two-arm select on c0) and x3 (whole flag word hoisted into c0 before block 0's mask); sandbox plus instrumented-cc1 allocno table for x1.
- result: x1 = 14 at 49; its table gives m nrefs=6 livelen=7 pri=17142 -- live length fell from 9 to 7 and pri rose. x3 = 27 at 48. Banked as rejected/s39-cond-hoisted-between-load-and-mask-score14.c and rejected/s39-whole-flagword-hoisted-score27.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: round-trip chassis (candidate.c as rewritten this session), single declared `u8 *q`, no FAKE constructs present

## [s39] Replacing block 1's two-arm select with an init-then-conditional-or shape, to cut one of m's references, loses two instructions.
- mechanism: `c = m; if (cond) c = m | 1;` compiles to the branch-around form, which does not reproduce the target's store-on-the-join geometry (bnez with the ori in the delay slot and an addu on the fall-through).
- probe: Variants w2 (`c = m; if (p[8] & 1) c = m | 1;`) and w3 (`c |= 1`); sandbox scores.
- result: w2 = 13 at 47 insns, w3 = 13 at 47 insns. Banked as rejected/s39-block1-init-then-conditional-or-score13.c and rejected/s39-block1-init-then-compound-or-score13.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: round-trip chassis (candidate.c as rewritten this session), single declared `u8 *q`, no FAKE constructs present

## [s39] MANDATED KILL RE-AUDIT -- s37's k1 (block 0's value named and consumed by block 1) re-measures unchanged on today's chassis, and fake_ablate finds no FAKE carrier on the contested pseudo.
- mechanism: Re-run of the closest-to-target banked form on HEAD, per the kill re-audit rule; its .greg dump is what exposed the s38 mis-attribution.
- probe: Spliced rejected/s37-block0-value-lives-across-block1-score10-BIT-IDENTICAL.c as variant k1, scored it, dumped its .greg, and ran `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c`.
- result: k1 = 10 at 49, unchanged from s37, and bit-identical to b0. Its .greg still carries hard reg 3 in q's conflict row because k1 keeps `m = *q & 0xF8;` as one statement, which still spawns the single-basic-block QImode load pseudo. fake_ablate reports no FAKE-annotated construct in candidate.c.
- verdict: CONFIRMED

==== s40 (synthesis) — FRONTIER RESET ====

s39's frontier items 1 and 2 ("cut m's references to 4", "shorten q's live length
to <= 22") are RETIRED AS ROUTES TO ZERO. They are accurate descriptions of why
blocks 0/1 are register-swapped, but achieving either would relocate the
mismatch to blocks 2/3 rather than remove it — see evidence.md
"==== s40 (synthesis) ====" section 1. The target holds `&D_80106A73` in TWO
general registers ($v1 across blocks 0-1, $a0 across blocks 2-3, simultaneously
live at `.L80034FC8`), and a single C pointer object is a single pseudo with a
single hard register (global.c:1275). The single-object chassis therefore has a
FLOOR of 10, and the ladder's question is no longer "which lever moves the
allocator" but "does any sanctioned family supply a SECOND address pseudo".

--- FRONTIER 1 (the only open technical question): a second register-held
address pseudo without a second declared C pointer object.
Mechanism: `&D_80106A73` becomes a register-held value (the target's `lui`+`addiu`
pair) only where the C source uses the address AS a value; a mem addressed by a
bare `symbol_ref` is a legitimate MIPS address and is expanded through the
assembler temporary `$at` instead (measured this session: a1/a2 both 28 at 48
with an identical stream; the symbol-difference expression constant-folds to the
plain symbol). Measured dead so far: bare symbol (a2), symbol-difference (a1),
the whole s33/s34/s35 anonymous-carrier family, and `static inline` helpers that
declare their own pointer (h1, 30 at 49 — both expansions share one store base
in $a1 and their reads fold to bare lui+%lo).
Next probe: the remaining untried shapes are addresses that are NOT legitimate
constants and so must be forced into a pseudo — an address whose displacement or
base comes from a runtime quantity that provably equals a constant (e.g. derived
from `p`, or from the loop counter after the loop). Every such spelling measured
to date added an instruction (s34), so the probe must be run for INSN COUNT
first (a 49-insn body) and only then for register seating; anything at 48 or 50
is not a candidate. If all such spellings either fold to the bare symbol or add
an instruction, the second-pseudo requirement is reachable only by declaring a
second pointer object, which the standing Judge constraint bans.

--- FRONTIER 2: re-audit the ban itself, not the codegen.
Mechanism: the score-0 body (`rejected/three-pointer-objects-judge-FAIL-score0.c`)
byte-matches and is a Judge-ruled cheat; the layer-1 FAIL that preceded it was an
ANNOTATION-FORMAT failure (four `u8 *q = &D_80106A73;` declarations carrying no
`/* FAKE */` line under pointer-alias-fake-exception prereq 3), and the Judge
constraint that followed closed the multi-handle axis outright. s40 supplies
something neither review had: a measured demonstration that the second pointer
object is not a register-allocation lever at all but the only C construct that
emits the target's second `lui`+`addiu` address materialisation.
Next probe: NOT a code probe. If a future session is assigned `escalation`, the
LADDER EXHAUSTED record must lead with the two-address-register reading of the
target asm and the a1/a2/h1 measurements, because that is the argument that
distinguishes "the ladder has not been worked" from "the residual is one
construct the Judge has ruled out". A `ruling-request` asking whether a second
pointer object may be spelled as ordinary program logic (blocks 2/3 addressed by
their own handle) is the other admissible move — but note the standing constraint
already answers it NO, so only a materially new framing (this one) justifies
asking again.

--- FRONTIER 3 (bookkeeping): ladder accounting.
This is session EIGHTEEN of cycle 2; the six-modality condition was met at s31.
TWO flat sessions remain before owner directive 2026-09-02 permits any
disposition.

## [s40] The target holds `&D_80106A73` in two distinct general registers, simultaneously live, so no body in which a single C pointer object supplies the flag-byte address for all four blocks can produce the target byte stream.
- mechanism: Read off asm/funcs/func_80034F88.s: blocks 0-1 address the byte through $v1 (lui/addiu at 80034F98/80034F9C), blocks 2-3 through $a0 (lui/addiu at 80034FC8/80034FCC and 80034FF0/80034FF4), and the block-2 materialisation is scheduled ABOVE block 1's store `sb $v0,0($v1)` at 80034FD0, so both registers hold the address at once. GCC 2.7.2 creates one allocno per pseudo (global.c:426) and gives each allocno exactly one hard register (global.c:1275, `reg_renumber[allocno_reg[allocno]] = best_reg;`), and 2.7.2 has no live-range splitting, so one pointer object yields one address register for the whole function.
- probe: Re-read the target disassembly block by block against the base body's objdump (s40/b0.txt, re-measured 10 at 49 this session); then measured variant qL (round-trip chassis, trailing loop addressed off `q[i-3]`), which raises q's allocno priority exactly as s39's formula prescribes, and read its allocno table from the instrumented cc1.
- result: qL moves q to nrefs=12 livelen=36 pri=10000 and displaces the loop counter from $v1 to $a0, but q seats in $a1 and the score RISES to 30 at 48 (s40/ad_qL.txt, s40/qL.txt). Winning the priority fight does not produce two address registers; it moves the one address register around. The s39 frontier's target state (q in $v1) would make blocks 0/1 exact and blocks 2/3 wrong — the same 10-point shape, relocated.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/global.c:1275
- measured_on: round-trip chassis (candidate.c as of s39) on HEAD 2026-09-05, single declared `u8 *q`, no FAKE constructs present; fake_ablate.py reports nothing to ablate

## [s40] Letting q cover blocks 0/1 only and spelling blocks 2/3 without a pointer object does not create the target's second register-held address: the bare symbol and the symbol-difference expression both emit %lo-folded memory operands through the assembler temporary $at.
- mechanism: A `symbol_ref` is a legitimate MIPS address, so a mem addressed by it is never forced into a pseudo; the address becomes a register-held value (lui+addiu) only where the C source uses `&D_80106A73` AS a value, i.e. assigns it to a pointer object.
- probe: Variants a1 (blocks 2/3 through `*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70))`) and a2 (blocks 2/3 through the bare symbol) on the round-trip chassis; sandbox scores plus objdump.
- result: a1 = 28 at 48, a2 = 28 at 48, and the two streams are byte-identical — the symbol-difference expression constant-folds to the plain symbol. Blocks 2/3 emit `lui $v0` + `lbu %lo($v0)` and `lui $at` + `sb %lo($at)`: four insns per block, the same COUNT as the target's lui/addiu/lbu/sb, with no address in an allocatable register. q takes $a1 and p takes $a0. Banked as rejected/s40-roundtrip-q-blocks01-anon23-score28.c and rejected/s40-roundtrip-q-blocks01-directsymbol23-score28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: round-trip chassis (candidate.c as of s39) on HEAD 2026-09-05, single declared `u8 *q` covering blocks 0/1, no FAKE constructs present

## [s40] A `static inline` helper that declares its own pointer, expanded for blocks 2/3, does not buy a second register-held address on the round-trip chassis.
- mechanism: Inlining creates a fresh pseudo per expansion, but cse merges the two expansions' address values into one and the reads fall back to %lo-folded operands, so only one address survives allocation.
- probe: Variant h1 = the round-trip chassis with blocks 2/3 replaced by two calls to `static inline void bb2_set_flag(s32 c, s32 bit)` whose body declares `u8 *q = &D_80106A73;`.
- result: h1 = 30 at 49. Inlining fires (no call in the .o), but both expansions share ONE store base and it is $a1 — the same register the caller's q takes — while both reads fold to bare `lui`+%lo, and the block shape degrades to beqz+nop. Reproduces s37's old-chassis result (30 at 49) on the new chassis. Banked as rejected/s40-roundtrip-inline-helper-blocks23-score30.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: round-trip chassis (candidate.c as of s39) on HEAD 2026-09-05, one pointer object in the caller plus the helper's own, no FAKE constructs present

## [s40] MANDATED KILL RE-AUDIT — s39's t1 (block 1 reads the symbol directly, so block 0's value becomes single-basic-block and block 1 reloads from memory) re-measures unchanged on today's chassis.
- mechanism: Re-run of the banked form that is structurally closest to the target — the only one whose block 1 reloads the flag byte the way the target's `lbu $a0,0($v1)` at 80034FB4 does — per the kill re-audit rule.
- probe: Spliced rejected/s39-block1-reads-symbol-directly-score11.c as variant t1, scored it, and ran `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c`.
- result: t1 = 11 at 50, identical to s39. fake_ablate reports no FAKE-annotated construct in candidate.c, so the banked kills on this chassis were not measured with a FAKE carrier occupying the contested pseudo.
- verdict: CONFIRMED

## [s40] The target holds &D_80106A73 in two distinct general registers, simultaneously live, so no body in which a single C pointer object supplies the flag-byte address for all four blocks can produce the target byte stream.
- mechanism: Read off asm/funcs/func_80034F88.s: blocks 0-1 address the byte through $v1 (lui/addiu at 80034F98/80034F9C), blocks 2-3 through $a0 (lui/addiu at 80034FC8/80034FCC and 80034FF0/80034FF4), and the block-2 materialisation is scheduled ABOVE block 1's store `sb $v0,0($v1)` at 80034FD0, so both registers hold the address at once. GCC 2.7.2 creates one allocno per pseudo (global.c:426) and gives each allocno exactly one hard register (global.c:1275, reg_renumber[allocno_reg[allocno]] = best_reg), with no live-range splitting, so one pointer object yields one address register for the whole function.
- probe: Re-read the target disassembly block by block against the base body's objdump (s40/b0.txt, re-measured 10 at 49 this session); then measured variant qL (round-trip chassis with the trailing copy loop addressed off q[i-3]), which raises q's allocno priority exactly as s39's formula prescribes, and read its allocno table from the instrumented cc1 (BB2_ALLOC_DEBUG=1).
- result: qL moves q to nrefs=12 livelen=36 pri=10000 and displaces the loop counter (pseudo 73) from $v1 to $a0, but q seats in $a1 and the score RISES to 30 at 48 insns (s40/ad_qL.txt, s40/qL.txt). Winning the priority fight does not produce two address registers; it moves the one address register around. The state s39's frontier aimed at (q in $v1) would make blocks 0/1 exact and blocks 2/3 wrong -- the same 10-point shape, relocated. s39's frontier items 1 and 2 are therefore retired as routes to zero.
- verdict: KILLED
- kill_scope: class
- measured_on: round-trip chassis (memory/grind/func_80034F88/candidate.c as of s39) on HEAD 2026-09-05, single declared `u8 *q`, no FAKE constructs present; fake_ablate.py reports nothing to ablate
- predicate_cite: tools/gcc-2.7.2/global.c:1275

## [s40] With q covering blocks 0/1 only, spelling blocks 2/3 through the bare symbol (a2) or through the symbol-difference expression (a1) emits %lo-folded memory operands through the assembler temporary $at instead of the target's lui+addiu address register, and both measure 28 at 48 insns.
- mechanism: A symbol_ref is a legitimate MIPS address, so a mem addressed by it is never forced into a pseudo; the address becomes a register-held value (lui+addiu) only where the C source uses &D_80106A73 AS a value, i.e. assigns it to a pointer object.
- probe: Variants a1 (blocks 2/3 through *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70))) and a2 (blocks 2/3 through the bare symbol) on the round-trip chassis; sandbox scores plus objdump.
- result: a1 = 28 at 48, a2 = 28 at 48, and the two streams are byte-identical -- the symbol-difference expression constant-folds to the plain symbol before address selection. Blocks 2/3 emit `lui $v0` + `lbu %lo($v0)` and `lui $at` + `sb %lo($at)`: four insns per block, the same COUNT as the target's lui/addiu/lbu/sb, but with no address in an allocatable register. q takes $a1, p takes $a0. Banked as rejected/s40-roundtrip-q-blocks01-anon23-score28.c and rejected/s40-roundtrip-q-blocks01-directsymbol23-score28.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: round-trip chassis (candidate.c as of s39) on HEAD 2026-09-05, single declared `u8 *q` covering blocks 0/1, no FAKE constructs present

## [s40] Replacing blocks 2/3 with two expansions of a static inline helper that declares its own `u8 *q` measures 30 at 49 on the round-trip chassis: both expansions share one store base in $a1 and their reads fold to bare lui+%lo.
- mechanism: Inlining creates a fresh pseudo per expansion, but cse merges the two expansions' address values into one and the reads fall back to %lo-folded operands, so only one address survives allocation -- and it lands in the same register the caller's q takes.
- probe: Variant h1 = the round-trip chassis with blocks 2/3 replaced by two calls to `static inline void bb2_set_flag(s32 c, s32 bit)` whose body declares `u8 *q = &D_80106A73;`; sandbox score plus objdump.
- result: h1 = 30 at 49. Inlining fires (no call in the .o) but buys no second register-held address, and the block shape degrades to beqz+nop. Reproduces s37's old-chassis result on the new (round-trip) chassis, where it had never been measured. Banked as rejected/s40-roundtrip-inline-helper-blocks23-score30.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: round-trip chassis (candidate.c as of s39) on HEAD 2026-09-05, one pointer object in the caller plus the helper's own, no FAKE constructs present

## [s40] MANDATED KILL RE-AUDIT -- s39's t1 (block 1 reads the symbol directly, so block 0's value becomes a single-basic-block quantity and block 1 reloads the flag byte from memory the way the target's `lbu $a0,0($v1)` at 80034FB4 does) re-measures unchanged on today's chassis.
- mechanism: Re-run of the banked form that is structurally closest to the target, per the kill re-audit rule, together with a FAKE-ablation check on the candidate.
- probe: Spliced rejected/s39-block1-reads-symbol-directly-score11.c as variant t1 and scored it; ran `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c`.
- result: t1 = 11 at 50, identical to s39's measurement. fake_ablate reports `no FAKE-annotated constructs found ... nothing to ablate`, so no banked kill on this chassis was measured with a FAKE carrier occupying the contested pseudo.
- verdict: CONFIRMED

==== s41 (synthesis) - FRONTIER RESET ====

s40's Frontier 1 ("a second register-held address pseudo without a second
declared C pointer object") is CLOSED by measurement this session for the one
shape it still prescribed. Its own next-probe text asked for "an address whose
displacement or base comes from a runtime quantity that provably equals a
constant (e.g. derived from `p`)"; three such bodies were built and measured
(n1 28/48 byte-identical to s40's a1/a2, n2 10/49 byte-identical to the base,
n3 16/50), and the pass dumps show the cancellation completing in combine via
`simplify_plus_minus` (`tools/gcc-2.7.2/cse.c:4250`). Nothing additive survives.

--- FRONTIER 1 (the disposition question, not a code probe): may the flag-byte
address be named more than once in C?
Mechanism: the target materialises `&D_80106A73` into an allocatable register
three times (lui/addiu $v1 at 80034F98 for blocks 0-1; lui/addiu $a0 at 80034FC8
for block 2; lui/addiu $a0 at 80034FF0 for block 3), with two of them
simultaneously live at `.L80034FC8`. Measured over sessions s33-s41, exactly one
C construct emits that lui+addiu pair - an assignment of `&D_80106A73` to a
declared pointer object; every other spelling (bare symbol, symbol difference,
p-derived additive expression, static-inline helper, anonymous carriers) either
folds to a bare `symbol_ref` and is expanded through `$at` with `%lo` folded into
the memory operand, or costs an instruction. One pointer object is one pseudo
(global.c:426) with one hard register (global.c:1275) and no live-range
splitting, and its redundant re-sets are deleted by cse. So the target's byte
stream implies the original C named that address more than once.
Next probe: NOT a code probe - a ruling question, filed by this session as the
`ruling-request` outcome. The standing constraint bans a second pointer object
both as a pointer-alias-family lever and as ordinary program logic; the framing
that is new (s40 + s41) is that the construct is an EMISSION requirement of the
target stream rather than a register-allocation lever, which is the premise the
ban rests on. If the ruling is NO, Frontier 2 is the whole remaining ladder.

--- FRONTIER 2 (what remains if the ruling is NO): the LADDER EXHAUSTED record.
Mechanism: owner directive 2026-09-02 requires 20 flat sessions and >= 6 distinct
modalities before any disposition; s41 is session NINETEEN of cycle 2 and the
modality condition was met at s31. The floor is 10, above ENDGAME_LOCK_MAX_FLOOR
= 5, so the 2026-07-27 standing ruling is not this function's subject and the
record must be titled LADDER EXHAUSTED (non-endgame residual, floor 10).
Next probe: when a session is assigned `escalation`, file that record in
docs/grind/decisions.md leading with (i) the three-materialisation reading of
asm/funcs/func_80034F88.s, (ii) the a1/a2/n1 byte-identity proof that no
non-pointer spelling emits a register-held address, (iii) the n2 proof that
re-setting one object is deleted, (iv) `scan_hand_coded --single func_80034F88`
= tier LOW 0/8 (s22/s23/s24) as the failed canonical-asm gate, and (v) the
negative SOTN-index census as the failed precedent gate; then return
owner-gated.

--- FRONTIER 3 (housekeeping, cheap and repeatedly costly when skipped):
the dispatch digest is six sessions stale (it has now twice announced "session
35" with a s34-era frontier). Every session must run
`grep -n '^==== s' memory/grind/func_80034F88/evidence.md | tail` before
spending a probe on a brief-listed "next probe", or it will re-derive s37-s40.

## [s41] An ordinary-C expression of the flag byte's address written additively in terms of the runtime pointer `p` is cancelled back to a bare symbol_ref, so it produces no second address pseudo and no instruction-count change.
- mechanism: The C source `(u8 *)((s32)p + ((s32)&D_80106A73 - (s32)p))` is reassociated at the tree level into `(p + SYM) - p` and reaches expand as genuine runtime arithmetic (`.rtl` carries `(set (reg 83) (plus p sym))` and `(set (reg 84) (minus (reg 83) p))` per access). The `+p`/`-p` pair is then cancelled by the brute-force PLUS/MINUS reassociator `simplify_plus_minus`, reached from combine through `simplify_binary_operation`; `(minus` counts in the function's section are .rtl 4, .jump 4, .cse 3, .loop 3, .cse2 3, .combine 0, .greg 0.
- probe: Three bodies on the candidate chassis - n1 (blocks 2/3 addressed through the expression), n2 (the single declared `q` re-assigned from it before blocks 2 and 3), n3 (mirror: blocks 0/1 through it, `q` on blocks 2/3) - each scored with `sandbox func_80034F88 --disable all`, disassembled, and compared instruction-for-instruction against s40's a1 and against the base body b0; plus `pwsh tools/grinder/dump.ps1 func_80034F88` on the n1 body for pass attribution.
- result: n1 = 28 at 48 and its stream is BYTE-IDENTICAL to s40's a1/a2 (the symbol-difference and bare-symbol bodies); n2 = 10 at 49 and its stream is BYTE-IDENTICAL to the base body b0, i.e. the re-set is cancelled and then deleted as redundant at zero instruction cost; n3 = 16 at 50. Banked as rejected/s41-blocks23-p-derived-address-folds-score28.c, rejected/s41-q-reset-from-p-derived-expr-deleted-score10-IDENTICAL.c and rejected/s41-blocks01-p-derived-blocks23-q-score16.c.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/cse.c:4250
- measured_on: candidate.c chassis (the s39 round-trip body) on HEAD 2026-09-05, single declared `u8 *q`, no FAKE constructs present; fake_ablate.py reports nothing to ablate

## [s41] MANDATED KILL RE-AUDIT - the two banked forms closest to the target re-measure unchanged on today's chassis, and the chassis itself is still 10 at 49.
- mechanism: Re-run of the closest-to-target instance kills per the kill re-audit rule, with a FAKE-ablation check on the candidate, to confirm that no banked verdict was measured under a chassis or FAKE state that no longer holds.
- probe: Spliced and scored b0 (a regeneration of memory/grind/func_80034F88/candidate.c), t1 (rejected/s39-block1-reads-symbol-directly-score11.c, the only banked form whose block 1 reloads the flag byte from memory like the target's `lbu $a0,0($v1)` at 80034FB4) and rt (rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c, the only banked body whose instruction multiset matches the target's); then ran `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c`.
- result: b0 = 10 at 49, t1 = 11 at 50, rt = 10 at 49 - all three identical to their banked values. fake_ablate reports "no FAKE-annotated constructs found ... nothing to ablate".
- verdict: CONFIRMED

## [s41] The target's byte stream requires the flag byte's address to be named as a C pointer object more than once, which is an emission requirement rather than a register-allocation lever.
- mechanism: `&D_80106A73` becomes a register-held value (the target's lui+addiu pair) only where the C source uses it AS a value assigned to a pointer object; a mem addressed by a bare symbol_ref is a legitimate MIPS address and is expanded through `$at` with `%lo` folded in. One pointer object is one pseudo (global.c:426) with one hard register (global.c:1275) and 2.7.2 has no live-range splitting, and a re-set of that object to the same constant is deleted by cse - so N register-held materialisations require N named assignments.
- probe: Reading asm/funcs/func_80034F88.s block by block (three lui/addiu materialisations: 80034F98 into $v1 for blocks 0-1, 80034FC8 and 80034FF0 into $a0 for blocks 2 and 3, with $v1 and $a0 both live at .L80034FC8), together with the accumulated measurements: s40 a1/a2 (28 at 48, no register-held address), s40 h1 (30 at 49, inline helper shares one base), s41 n1/n2/n3 (this session), the s33-s35 anonymous-carrier family, and the score-0 four-handle body (rejected/three-pointer-objects-judge-FAIL-score0.c) which is the only measured body that emits all three materialisations.
- result: Every non-pointer spelling measured across s33-s41 either folds to a bare symbol_ref or costs an instruction; the only construct that emits the target's second and third lui+addiu materialisations is a further `u8 *q = &D_80106A73;`. This is the ruling question filed as this session's outcome.
- verdict: CONFIRMED

## [s42] MANDATED KILL RE-AUDIT: the two banked bodies closest to the target still measure their banked values on today's chassis, and s37's copy-loop-off-q kill re-measures unchanged on the post-s39 round-trip chassis it was never tested on.
- mechanism: Instance kills are chassis-relative, and s39 REPLACED the chassis (block 0's mask became an SImode round-trip through a named variable), so every kill measured before s39 carries a `measured_on` that no longer describes the body it would be spliced into. s37's h12/h13 (`*(q - 3 + i)` as the trailing copy loop's destination) is the pre-s39 kill sitting closest to a live lever, because the loop is the only free source of extra references to `q` in `allocno_compare`'s priority key (tools/gcc-2.7.2/global.c:635).
- probe: Spliced and scored b0 (a regeneration of memory/grind/func_80034F88/candidate.c), rt (rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c), t1 (rejected/s39-block1-reads-symbol-directly-score11.c) and v1 (s37's h12 shape rebuilt on the round-trip chassis) with `sandbox func_80034F88 --disable all`, then ran `python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c`.
- result: b0 = 10 at 49, rt = 10 at 49, t1 = 11 at 50 -- all identical to their banked values. v1 = 30 at 48, identical to s37's pre-round-trip measurement of the same shape. fake_ablate reports "no FAKE-annotated constructs found ... nothing to ablate", so no banked kill on this chassis was measured with a FAKE carrier occupying the contested pseudo.
- verdict: CONFIRMED

## [s42] Lengthening the VALUE allocnos' live ranges by chaining each flag block's computed value into the next block, instead of re-reading the flag byte, demotes them below `q` in allocno_compare and wins `q` the $v1 seat.
- mechanism: tools/gcc-2.7.2/global.c:635 sorts allocnos by floor_log2(n_refs)*n_refs*size/live_length; s39 measured the residual as a 2619-point gap between the block-0 value m (6 refs / 9 live / pri 13333) and the pointer q (10 refs / 28 live / pri 10714). s39 attacked m's reference count and s37 attacked m's live length from the MIDDLE of block 0 (both failed); the untried direction was extending the value's live range from its END, by making later blocks consume the value rather than re-read memory -- ordinary C, since the store is the only writer of the byte.
- probe: v2 (every block consumes the previous block's value, spelled with two alternating named value locals so that no self-assign appears) and v3 (the same chain over blocks 0-2 only, block 3 re-reading), both on the round-trip chassis, scored with `sandbox func_80034F88 --disable all`.
- result: v2 = 24 at 45 insns, v3 = 24 at 47. The chain does lengthen the value ranges, but it also deletes the flag-byte RELOADS the target contains: with no memory read left in blocks 2/3 there is nothing for cse to forward from, and the body ends up four (v2) or two (v3) instructions SHORT of the target's 49, which reads the flag byte four times (80034FA0, 80034FB4, 80034FD8, 80034FFC). Banked as rejected/s42-value-chained-all-blocks-score24.c and rejected/s42-value-chained-blocks012-score24.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis (b0 re-measured 10 at 49, rules_dropped 0) on HEAD 2026-09-05, single declared `u8 *q`, no FAKE constructs present; fake_ablate reports nothing to ablate

## [s42] Raising `q`'s reference count through the trailing copy loop -- the only free source of extra references to the pointer in this function -- is worth less than the address form the loop gives up.
- mechanism: A reference inside the copy loop is weighted by loop depth in flow.c's REG_N_REFS accounting, so it is the cheapest available lift to q's allocno_compare priority; but the loop's destination operand is D_80106A70, and expressing it as `q - 3 + i` replaces the loop's own `lui %hi / addu $at,$at,$v1 / sb %lo($at)` indexed store -- which the target emits verbatim at 80035020-80035028 -- with a base-pointer store.
- probe: v1 (`*(q - 3 + i) = *((u8 *)p + i + 0x17);`) rebuilt on the post-s39 round-trip chassis and scored, as the chassis-relative re-audit of s37's h12/h13.
- result: 30 at 48 insns, identical to the pre-round-trip measurement. The three-instruction address form is lost and the score rises 20 points. Banked as rejected/s42-copyloop-off-q-roundtrip-chassis-score30.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05, single declared `u8 *q`, no FAKE constructs present

## [s42] TERMINAL SYNTHESIS: with both sides of the allocno_compare gap now measured from every byte-neutral direction, the residual is an EMISSION requirement -- the target's three lui+addiu materialisations of `&D_80106A73` -- and the Judge has closed the only construct that emits them.
- mechanism: One pseudo is one allocno (global.c:426) with one hard register (global.c:1275) and no live-range splitting, so N register-held materialisations of one address require N pseudos; a mem addressed by a bare symbol_ref is a legitimate MIPS address and is expanded through `$at` with %lo folded in, so no non-pointer spelling produces a pseudo at all. Every alternative measured over s16-s42 (bare symbol, symbol difference, p-derived arithmetic cancelled by simplify_plus_minus at tools/gcc-2.7.2/cse.c:4250, static-inline helpers, one- and two-slot aggregates, the `&D_80106A70[3]` array model, anonymous carriers) either folds or costs instructions.
- probe: The full ledger read end-to-end plus this session's six measurements; the Judge's rulings of 2026-09-04 15:09 (docs/grind/decisions.md:22578) and 2026-09-05 06:11 (docs/grind/decisions.md:23322), the latter answering the emission-vs-RA reframing directly and directing the foreclosure record.
- result: The multi-handle axis is FAIL(CONSTRUCT) and not escalatable (zero SOTN-master instances in the s22/s24 census); `scan_hand_coded --single func_80034F88` is tier LOW 0/8 (s22/s23/s24), so the canonical-asm gate fails as well. s42 is session twenty of cycle 2 with >= 6 modalities spent, satisfying the owner's 2026-09-02 directive. The LADDER EXHAUSTED (non-endgame residual, floor 10) record was filed in docs/grind/decisions.md this session and the outcome returned owner-gated.
- verdict: CONFIRMED

## Frontier after s42 (RE-ACTIVATION TRIGGERS, not probes -- the ladder is spent)

- **F1.** A future owner or Judge grant that admits a SECOND C pointer object naming `&D_80106A73` (a widening of pointer-alias-fake-exception from one redundant handle to per-use handles, or a new sanctioned family covering per-block address materialisation). The body is already banked and measures ZERO: rejected/three-pointer-objects-judge-FAIL-score0.c. Nothing else in the function would need to change.
- **F2.** A toolchain finding that some ordinary-C construct not yet tried forces a symbol address into a pseudo. The measured-dead list is in the s42 evidence section; a new candidate should first be shown, in a standalone test, to emit `lui $r,%hi(SYM); addiu $r,$r,%lo(SYM)` into an ALLOCATABLE register before it is worth a session here.
- **F3.** Housekeeping that repeatedly costs sessions: the dispatch digest lags the ledger by several sessions (it announced "session 35" with an s34-era frontier for this run too). Any future session must run `grep -n '^==== s' memory/grind/func_80034F88/evidence.md | tail` before spending a probe on a brief-listed "next probe".

## [s43] MANDATED KILL RE-AUDIT: the base body still measures 10 at 49 on today's chassis, and a redundant same-constant re-assignment of `q` inside block 1 contributes ZERO references to `q`'s allocno.
- mechanism: `REG_N_REFS` is counted by flow.c over the insns that survive into global allocation; a second `q = &D_80106A73;` whose source cse unifies with the live value is deleted before flow re-counts, so it can neither add a materialisation nor lift `q` in `allocno_compare` (tools/gcc-2.7.2/global.c:635). This extends s41's n2 finding (byte-identical output) to the REFERENCE COUNT, which is what the priority formula actually consumes.
- probe: b0 (regeneration of memory/grind/func_80034F88/candidate.c) and d1 (b0 plus `q = &D_80106A73;` as the first statement of block 1), each scored with `sandbox func_80034F88 --disable all`, then `pwsh tools/grinder/dump.ps1 func_80034F88` and the ALLOCDBG allocno table (`wsl bash tmp/grind/func_80034F88/s39/ad.sh`, BB2_ALLOC_DEBUG=1) read for both.
- result: b0 = 10 at 49; d1 = 10 at 49. The two .greg sections are character-for-character identical -- same allocate list `73 78/77 80 84 74 75 79 83 72`, same conflict rows, same dispositions -- and the ALLOCDBG table for b0 is ord=4 pseudo=74 (m) nrefs=6 livelen=9 pri=13333, ord=5 pseudo=75 (q) nrefs=10 livelen=28 pri=10714, i.e. unchanged from s39's numbers. Banked as rejected/s43-redundant-q-reset-block1-deleted-score10-IDENTICAL.c.
- verdict: CONFIRMED

## [s43] Cutting the block-0 flag-byte value's reference count below `q`'s priority does flip the allocno ORDER, but every spelling that achieves the cut splits block 0 into two pseudos, one of which is single-basic-block and is seated in hard reg 3 ($v1) by local-alloc, so `q` gains a hard-reg-3 conflict and still loses the seat.
- mechanism: `allocno_compare` (tools/gcc-2.7.2/global.c:635) ranks by floor_log2(n_refs)*n_refs*size/live_length. On the base body the block-0 value m is ONE multi-block pseudo because the same C variable carries both the load and the mask (`m = *q; m = m & 0xF8; *q = m;`), which costs 6 references (13333) and outranks q (10714). Any spelling that lowers that count separates the loaded byte from the masked byte into two pseudos; whichever of the two is confined to block 0 is a local-alloc quantity, and local_alloc runs before global_alloc and takes the first free hard register in allocation order -- $v0 is occupied by the call's return value across block 0, so it takes $v1, which is exactly the register the target gives to `q`.
- probe: Four bodies on the base chassis, each scored with `sandbox func_80034F88 --disable all` and dumped with `pwsh tools/grinder/dump.ps1 func_80034F88` for the .greg conflict rows and dispositions -- m1 (`m = *q & 0xF8; *q = m;`), m4 (raw byte and masked byte given separate names `m` and `v0`), m3 (raw byte named and multi-block, the mask recomputed inside both block-1 arms), m5 (masked value named inside block 1's scope); plus m2 (block-1 store duplicated into the arms, no named c) as the fifth point.
- result: m1 = 10 at 49 and m4 = 10 at 49 -- in BOTH, the sort order flips so that q precedes the masked value (m1: `73 78 81 85 75 74 80 84 72`, m4: `73 78 81 85 76 75 80 84 72`), and in BOTH q's .greg conflict row gains hard reg 3 (`... 2 3 29` against the base body's `... 2 29`) because a block-0-confined pseudo (m1: 76, m4: 74) is dispositioned `in 3` without appearing in the global allocate list. m3 = 17 at 49: the raw byte becomes multi-block as intended (74, allocate list `73 80 84 88 75 74 83 87 72`) but the STORE's masked temp becomes the block-0 local in $v1 (78 in 3), and p is displaced from $a1 to $a2. m5 = 27 at 52, m2 = 12 at 46. Banked as rejected/s43-mask-folded-single-stmt-localalloc-v1-score10.c, rejected/s43-split-raw-and-masked-names-score10.c, rejected/s43-raw-byte-mask-recomputed-in-arms-score17.c, rejected/s43-masked-value-named-inside-block1-score27.c, rejected/s43-block1-store-duplicated-into-arms-score12.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 re-measured 10 at 49, rules_dropped 0); single declared `u8 *q`, no FAKE constructs present; fake_ablate has nothing to ablate on this chassis (s41/s42)

## [s43] Hoisting block 1's condition above the point where `q` is materialised shortens `q`'s live range and lifts it past the block-0 value in allocno_compare.
- mechanism: q's priority is 3*10/28 = 10714 and the block-0 value's is 2*6/9 = 13333; at 10 references q needs live_length <= 21 to win. The block-1 condition (`lw` of p[8] plus the `andi`) sits inside q's live range in the base body, so computing it before `q = &D_80106A73;` should move two or three insns out of that range at zero instruction cost.
- probe: e1 (`c0 = p[8] & 1;` computed immediately after the call, before `q = &D_80106A73;`, block 1 reading `c = c0;`), scored and read through the ALLOCDBG allocno table.
- result: e1 = 13 at 49. q is pseudo 76 with nrefs=10 livelen=28 pri=10714 -- IDENTICAL to the base body's q -- while p's live length grows 33 -> 35 and the body loses 3 points. The scheduler re-places the hoisted condition inside q's range before live lengths are counted, so a source-level hoist cannot shorten the address allocno's live range. Banked as rejected/s43-block1-condition-hoisted-above-q-score13.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49), single declared `u8 *q`, no FAKE constructs present

## [s43] SYNTHESIS: the residual is now a closed arithmetic inequality with three named exits, all quantified on today's chassis.
- mechanism: The seat the target gives `q` ($v1 for blocks 0/1, with the loop induction variable `i` sharing it and the block-0 value in $a0) is reachable iff, at global-alloc time, (a) q carries no hard-reg-3 conflict AND (b) q outranks the block-0 value in allocno_compare. The base body satisfies (a) and fails (b) by 2619 points; every measured spelling that satisfies (b) violates (a) by creating a block-0-confined local quantity that local_alloc seats in $v1.
- probe: The seven measurements of this session plus the ALLOCDBG tables, reduced to the inequality floor_log2(r)*r/L > 1.3333 for q and 2*r'/L' < 1.0714 for the block-0 value.
- result: The exits are exactly three, with thresholds: (i) q reaches nrefs >= 13 at livelen 28 (three more surviving references to the pointer; same-constant re-sets contribute none, and every real extra use measured costs instructions); (ii) q reaches livelen <= 21 at nrefs 10 (a source-level hoist does not move it -- e1); (iii) the block-0 value reaches livelen >= 12 at 6 refs (>= 10 at 5 refs, >= 8 at 4 refs) WITHOUT splitting into a block-0-confined pseudo. Exit (iii) is the only one whose failure mode is not yet a general predicate: the 4-reference spellings all split, but no measurement yet shows a 5- or 6-reference spelling with a stretched range that survives scheduling.
- verdict: CONFIRMED

## Frontier after s43

- **A.** Exit (iii): a block-0 spelling in which ONE multi-block pseudo carries both the loaded byte and the masked byte (so no local-alloc quantity appears in blocks 0/1, preserving q's clean conflict row) but whose live_length reaches 12 or more. s39's x1/x3 stretched it from the middle and the scheduler compacted it; s43's e1 stretched from the front with the same result. UNTRIED direction: stretch from the END -- a real, ordinary use of the block-0 masked value AFTER block 1's store whose result the target's bytes still contain (the only surviving consumer candidates are the block-1 store value itself and the copy-loop bound).
- **B.** Exit (i): three more surviving references to `q` at zero instruction cost. Measured dead sources: same-constant re-set (s41 n2, s43 d1 -- deleted, zero refs), copy-loop destination `q[i-3]` (s37/s42 -- 30, address form lost), derived pointer `u8 *r = q - 3;` (s29 -- 19 at 50). Untried: a reference to `q` that the target's own stream already contains but our body spells through the symbol.
- **C.** Housekeeping (repeat of s42's F3): the dispatch digest lags the ledger badly -- this run was dispatched as "session 35" with an s34-era frontier and a discarded-session note. Every session must run `grep -n '^## \[s' memory/grind/func_80034F88/hypotheses.md | tail` first. NOTE ALSO: a foreclosure disposition requires `escalation` modality; s42's owner-gated outcome was discarded for filing one from `structural`, so the LADDER EXHAUSTED record it wrote must be re-filed by whichever session the driver dispatches in `escalation`.

## [s41] MANDATED KILL RE-AUDIT: the base body still measures 10 at 49 on today's chassis, and a redundant same-constant re-assignment of q inside block 1 contributes zero references to q's allocno.
- mechanism: REG_N_REFS is counted by flow.c over the insns surviving into global allocation; a second `q = &D_80106A73;` whose source cse unifies with the live value is deleted before flow re-counts, so it can lift nothing in allocno_compare (tools/gcc-2.7.2/global.c:635). This extends s41's n2 finding (byte-identical output) to the reference count itself, which is what the priority formula consumes.
- probe: b0 (regeneration of memory/grind/func_80034F88/candidate.c) and d1 (b0 plus `q = &D_80106A73;` as block 1's first statement) scored with `sandbox func_80034F88 --disable all`, then `pwsh tools/grinder/dump.ps1 func_80034F88` plus the BB2_ALLOC_DEBUG allocno table via tmp/grind/func_80034F88/s39/ad.sh.
- result: b0 = 10 at 49, d1 = 10 at 49; the two .greg sections are character-for-character identical (same allocate list, conflict rows and dispositions). The ALLOCDBG table for b0 reproduces s39's numbers exactly: i (73) 11 refs / 7 live / 47142; m (74) 6 refs / 9 live / 13333; q (75) 10 refs / 28 live / 10714; p (72) 6 refs / 33 live / 3636. Banked as rejected/s43-redundant-q-reset-block1-deleted-score10-IDENTICAL.c.
- verdict: CONFIRMED

## [s41] Cutting the block-0 flag value's reference count flips the allocno order so that q precedes it, but each of the four spellings measured that achieves the cut splits block 0 into two pseudos, one block-confined, which local-alloc seats in $v1 and which re-adds hard reg 3 to q's conflict row.
- mechanism: allocno_compare (tools/gcc-2.7.2/global.c:635) ranks by floor_log2(n_refs)*n_refs*size/live_length. The base body's single named m carries both the load and the mask, costing 6 references (13333) against q's 10714. Lowering that count separates the loaded byte from the masked byte; whichever pseudo is confined to block 0 is a local_alloc quantity, and local_alloc runs first and takes the first free hard register in allocation order -- $v0 is occupied across block 0 by the call's return value, so it takes $v1.
- probe: m1 (`m = *q & 0xF8; *q = m;`), m4 (raw and masked bytes given separate names), m3 (raw byte named and multi-block, mask recomputed in both block-1 arms), m5 (masked value named inside block 1), m2 (block-1 store duplicated into the arms) -- each scored with sandbox and dumped with tools/grinder/dump.ps1 for the .greg allocate list, conflict rows and dispositions.
- result: m1 = 10 at 49 and m4 = 10 at 49: the sort order flips (m1 `73 78 81 85 75 74 80 84 72`, m4 `73 78 81 85 76 75 80 84 72`) and in both q's conflict row gains hard reg 3 (`2 3 29` against the base body's `2 29`) from a pseudo dispositioned `in 3` that is absent from the allocate list (m1: 76, m4: 74). m3 = 17 at 49 (raw byte becomes multi-block as intended, but the store's masked temp becomes the block-0 local in $v1, and p is displaced to $a2). m5 = 27 at 52, m2 = 12 at 46. Banked as rejected/s43-mask-folded-single-stmt-localalloc-v1-score10.c, s43-split-raw-and-masked-names-score10.c, s43-raw-byte-mask-recomputed-in-arms-score17.c, s43-masked-value-named-inside-block1-score27.c, s43-block1-store-duplicated-into-arms-score12.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 re-measured 10 at 49, rules_dropped 0); single declared `u8 *q`; no FAKE constructs present (fake_ablate has nothing to ablate on this chassis, per s41/s42)

## [s41] Hoisting block 1's condition above the point where q is materialised shortens q's live range and lifts it past the block-0 value in allocno_compare.
- mechanism: q's key is 3*10/28 = 10714 and the block-0 value's is 2*6/9 = 13333, so at 10 references q needs live_length <= 21 to win; the block-1 condition (the lw of p[8] plus its andi) sits inside q's live range in the base body, and computing it before `q = &D_80106A73;` should move those insns out of the range at zero instruction cost.
- probe: e1 (`c0 = p[8] & 1;` computed immediately after the call, before q is assigned; block 1 reads `c = c0;`), scored with sandbox and read through the BB2_ALLOC_DEBUG allocno table.
- result: e1 = 13 at 49, and q comes out with nrefs=10 livelen=28 pri=10714 -- digit for digit the base body's -- while p's live length grows 33 to 35. The first scheduling pass re-places the hoisted computation inside q's range before live lengths are counted, the same compaction s39's x1/x3 measured from the middle of the range. Banked as rejected/s43-block1-condition-hoisted-above-q-score13.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49), single declared `u8 *q`, no FAKE constructs present

## [s41] The residual is a two-clause conjunction with exactly three numeric exits on this chassis, and the base body is the only measured spelling that satisfies the conflict-row clause.
- mechanism: q takes the target's $v1 iff (a) q's .greg conflict row carries no hard reg 3 (no block-0-confined local_alloc quantity exists) and (b) q precedes the block-0 value in allocno_compare. The base body satisfies (a) and fails (b) by 2619 points; m1/m3/m4 satisfy (b) and fail (a). Note that q and the loop induction variable i do not conflict in either body, so the target's sharing of $v1 between the address and i is reachable.
- probe: The seven bodies of this session plus their ALLOCDBG tables, reduced to the inequalities floor_log2(r)*r/L > 1.3333 for q and 2*r'/L' < 1.0714 for the block-0 value.
- result: Exit (i): q at nrefs >= 13 with livelen 28 -- known ref sources all dead (same-constant re-set contributes zero refs; `q[i-3]` copy-loop destination 30; `u8 *r = q - 3;` 19 at 50). Exit (ii): q at livelen <= 21 with nrefs 10 -- unmoved by a source-level hoist (e1). Exit (iii): the block-0 value at livelen >= 12 with 6 refs (>= 10 at 5 refs, >= 8 at 4 refs) while remaining ONE multi-block pseudo -- attacked from the middle (s39 x1/x3) and the front (e1), never from the END, which is the frontier this session hands over.
- verdict: CONFIRMED

## [s44] MANDATED KILL RE-AUDIT: the four banked bodies closest to the target all re-measure at their banked values on today's chassis.
- mechanism: Instance kills are chassis-relative; the re-audit rule requires the closest-to-target banked forms to be re-measured on the current chassis and with FAKE constructs ablated before any new probe is spent.
- probe: Spliced and scored b0 (regeneration of memory/grind/func_80034F88/candidate.c), rt (rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c), t1 (rejected/s39-block1-reads-symbol-directly-score11.c) and m1 (rejected/s43-mask-folded-single-stmt-localalloc-v1-score10.c) with `sandbox func_80034F88 --disable all`.
- result: b0 = 10 at 49, rt = 10 at 49, t1 = 11 at 50, m1 = 10 at 49 -- every one identical to its banked value. The chassis is unchanged and, per s41/s42, carries no FAKE construct for fake_ablate.py to remove.
- verdict: CONFIRMED

## [s44] The base body already emits all three of the target's `lui`+`addiu` materialisations of `&D_80106A73` from ONE declared pointer object, so the s41/s42 "three materialisations require three pointer objects" factorisation is wrong and the whole 10-point residual is a single register swap in blocks 0/1.
- mechanism: cse does not unify the same-constant re-assignments of `q` at the heads of blocks 2 and 3 into one register-held value; it re-materialises the address at each one, which is why the base body carries `lui/addiu` at 4f48, 4f7c and 4fa0. What differs from the target is only WHICH hard registers the blocks-0/1 address and the blocks-0/1 flag-byte value receive: the target seats the address in $v1 and the value in $a0, the base body the reverse. The `.L` ordering difference (target schedules mat2 ahead of block 1's store) is a consequence of that swap, because in the target the store uses $v1 and so does not conflict with mat2's destination.
- probe: `mipsel-linux-gnu-objdump -d` on tmp/grind/func_80034F88/s44/b0.o, compared instruction-for-instruction against asm/funcs/func_80034F88.s; the full table is in the s44 section of evidence.md. Blocks 2 and 3, the trailing copy loop, the prologue and the epilogue are register-exact and instruction-exact in both.
- result: Confirmed. Also confirmed by direct measurement for the first time (s29 priced it by arithmetic only): the target's block-1 reload at 80034FB4 occupies the load-delay slot that the base body fills with a `nop`, so both bodies are 49 instructions and both score 10 -- rt HAS the reload, b0 does not, and they are worth the same. The reload is a passenger of the seat assignment, not a lever.
- verdict: CONFIRMED

## [s44] Duplicating block 1's flag-byte read into one of its arms (the duplicated-read-into-arms spelling) stretches nothing, because cse forwards the arm read back to the block-0 value and the body collapses onto the base body byte-for-byte.
- mechanism: cse enters the mem stored by block 0 into its hash table keyed on the address rtx; block 1's arm read hashes to the same address and is replaced by the stored register. The QI census over the cc1 dumps names the pass: the p1 body reaches .rtl and .jump with FOUR flag-byte loads (the count the target has) and leaves .cse with THREE, and the count never changes again through .loop, .cse2, .combine and .greg.
- probe: p1 (block 1's THEN arm reads `*q`, else arm keeps `m`) and p2 (mirror, ELSE arm reads `*q`), each scored with `sandbox func_80034F88 --disable all`, their objects compared with `cmp` against b0.o, plus `pwsh tools/grinder/dump.ps1 func_80034F88` on p1 and a `(mem:QI` census of the `;; Function func_80034F88` slice of every dump.
- result: p1 = 10 at 49 and p2 = 10 at 49, and BOTH objects are byte-identical to b0.o. This closes s43's Frontier A for the only ordinary consumer this function has: after block 1's store the flag byte holds `m | bit`, not `m`, so no later block can legitimately re-read `m`'s value, and a re-read inside block 1 is forwarded away. Banked as rejected/s44-block1-thenarm-duplicate-read-cse-forwarded-score10-IDENTICAL.c and rejected/s44-block1-elsearm-duplicate-read-cse-forwarded-score10-IDENTICAL.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 re-measured 10 at 49), single declared `u8 *q`, no FAKE constructs present; fake_ablate has nothing to ablate on this chassis

## [s44] Deleting block 0's uses of `q` in order to start the pointer's live range later does not move q's allocno: the address constant is still materialised at the top of the function, q keeps $a0, and the symbol-addressed block-0 access costs an extra instruction.
- mechanism: s43's e1 attacked exit (ii) -- q at live_length <= 21 -- by hoisting a computation out of q's range and the scheduler put it back. p5/p6 attack the same exit from the opposite end, by removing q's earliest references entirely so that its live range cannot begin before block 1. The address materialisation is a constant load with no dependences, so the first scheduling pass places it immediately after the call regardless of where the source assigns the pointer.
- probe: p5 (block 0 wholly through the bare symbol, `q` first assigned inside block 1) and p6 (block 0's load through the symbol, `q` assigned immediately before block 0's store), scored with `sandbox func_80034F88 --disable all` and disassembled.
- result: p5 = 12 at 50, p6 = 11 at 50. In both, `lui a0 / addiu a0` is emitted at 4f48-4f4c immediately after the call -- ahead of the symbol-addressed block-0 load -- and q still receives $a0; the symbol-addressed load adds one `lui`. Banked as rejected/s44-block0-wholly-symbol-q-from-block1-score12.c and rejected/s44-block0-load-symbol-q-before-store-score11.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49), single declared `u8 *q`, no FAKE constructs present

## [s44] Handing one flag block back to the bare symbol to shorten `q`'s live range is monotone in the number of blocks handed back and has no minimum below the base body's 10.
- mechanism: A block addressed through the bare symbol emits `lui $at` with `%lo` folded into each memory operand, which removes two references and several insns of live length from q's allocno but also removes the target's `lui`+`addiu` address form for that block.
- probe: p3 (q covers blocks 0/1/2; block 3 through the symbol) and p4 (q covers blocks 0/1/3; block 2 through the symbol), scored with `sandbox func_80034F88 --disable all`; read together with s40's a1/a2, which handed back BOTH blocks 2 and 3 and measured 28 at 48.
- result: p3 = 16 at 49, p4 = 20 at 50. One block back costs 6 points, the other 10, and both back costs 18: the axis is monotone away from the target. Banked as rejected/s44-block3-via-bare-symbol-score16.c and rejected/s44-block2-via-bare-symbol-score20.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49), single declared `u8 *q`, no FAKE constructs present

## [s44] Making the loop counter's allocno live across the flag blocks (so that it holds $v1 and pushes the block-0 value off that seat) costs four instructions and moves the score away from the target.
- mechanism: The .lreg records for the base body show the loop counter as pseudo 73, 11 references across 7 insns (the highest allocno_compare key in the function, 47142), and it takes $v1 before either the block-0 value or `q` is considered; the block-0 value then takes $v1 too because the two do not conflict. Initialising the counter at the top of the function (ordinary split-init, `i = 0;` before the call and `for (; i < 3; i++)`) makes it conflict with every flag-block quantity, which should deny $v1 to the block-0 value.
- probe: p7 (`i = 0;` hoisted above `p = func_80077D00();`, loop spelled `for (; i < 3; i++)`), scored with `sandbox func_80034F88 --disable all`.
- result: p7 = 24 at 53 insns. The counter's live range now spans the call and the whole body, so it also conflicts with `q` and with `p`, and the allocation degrades by four extra instructions. Banked as rejected/s44-loop-counter-init-hoisted-to-top-score24.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49), single declared `u8 *q`, no FAKE constructs present

## [s44] Cutting the block-0 value's reference count by having block 1 name it once (`c = m; if (cond) c = c | 1;`) also shortens its live range, so the allocno_compare key rises instead of falling, and the body loses the target's two-arm shape.
- mechanism: allocno_compare's key is floor_log2(refs)*refs/live_length (tools/gcc-2.7.2/global.c:635); the base body's block-0 value is 6 refs over 9 insns (13333) against q's 10 over 28 (10714). Every reference removed from the value is also a point at which it was still live, so refs and live_length fall together and the quotient does not improve -- the arithmetic reason the s43 four-reference spellings and this five-reference spelling both fail, independent of the local-alloc splitting that s43 identified.
- probe: p8 (block 1 spelled `c = m; if (p[8] & 1) { c = c | 1; }`, one reference to `m` instead of two), scored with `sandbox func_80034F88 --disable all`.
- result: p8 = 13 at 47 insns -- two instructions SHORT of the target, because the else arm's `addu v0,<val>,zero` disappears when the conditional or-in replaces the two-arm value selection. Banked as rejected/s44-block1-single-m-ref-conditional-or-score13.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49), single declared `u8 *q`, no FAKE constructs present

## Frontier after s44

- **A.** THE UNREAD INSTRUMENT: hard-register PREFERENCES, not allocno rank. Every session from s32 on has attacked `allocno_compare`'s sort order; nobody has read the preference records that `local-alloc.c` builds and `global.c` consults when it picks a hard register for an allocno it has already ordered. In the target the blocks-0/1 VALUE shares $a0 with the blocks-2/3 ADDRESS materialisations -- the sharing pattern a copy-derived preference produces -- while in the base body the value shares $v1 with the loop counter. Next probe: dump `.lreg` and `.greg` for b0 and for the score-0 four-handle body side by side and diff the preference/`regs_may_share` records, not the allocate lists; then look for an ordinary-C spelling that gives the block-0 value a copy relationship with a quantity already seated in $a0.
- **B.** The seat swap is now known to be the WHOLE residual (10 points, one register pair) and the reload is known to be free. Any future probe should be judged by whether it changes the hard register of pseudo 74 (the block-0 value) or pseudo 75 (`q`) in the `.greg` dispositions -- a score that stays at 10 with the same two dispositions is the base body wearing a different shirt, and p1/p2 show that can be literally byte-identical. Check `cmp` against b0.o before writing a body up.
- **C.** Housekeeping (third repeat): the dispatch digest lags the ledger badly -- this run was dispatched as "session 35" with an s34-era frontier while the ledger is at s44. Run `grep -n '^## \[s' memory/grind/func_80034F88/hypotheses.md | tail` before spending a probe on a brief-listed "next probe". The LADDER EXHAUSTED record drafted by s42 still needs re-filing by an `escalation`-modality session; s42's owner-gated outcome was discarded for filing one from `structural`, and s44 is `synthesis`, so it does not file one either.

## [s42] MANDATED KILL RE-AUDIT: the four banked bodies closest to the target re-measure at their banked values on today's chassis.
- mechanism: Instance kills are chassis-relative, so the closest-to-target banked forms must be re-measured before any new probe is spent; s41/s42 already established that fake_ablate.py finds no FAKE construct in this chassis to ablate.
- probe: Spliced and scored b0 (regeneration of memory/grind/func_80034F88/candidate.c), rt (rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c), t1 (rejected/s39-block1-reads-symbol-directly-score11.c) and m1 (rejected/s43-mask-folded-single-stmt-localalloc-v1-score10.c) with `sandbox func_80034F88 --disable all`.
- result: b0 = 10 at 49, rt = 10 at 49, t1 = 11 at 50, m1 = 10 at 49 -- every one identical to its banked value. Chassis unchanged.
- verdict: CONFIRMED

## [s42] The base body already emits all three of the target's lui+addiu materialisations of &D_80106A73 from ONE declared pointer object, so the s41/s42 'three materialisations require three pointer objects' factorisation is wrong, and the whole 10-point residual is a single register swap in blocks 0/1 (target: address in $v1, value in $a0; base body: the reverse).
- mechanism: cse does not unify the same-constant re-assignments of q at the heads of blocks 2 and 3 into one register-held value; it re-materialises the address at each one (4f48, 4f7c, 4fa0 in b0.o). The .L ordering difference is a consequence of the seat swap: in the target block 1's store uses $v1 so mat2 into $a0 can be scheduled ahead of it.
- probe: mipsel-linux-gnu-objdump -d on tmp/grind/func_80034F88/s44/b0.o compared instruction-for-instruction with asm/funcs/func_80034F88.s; full table in the s44 section of memory/grind/func_80034F88/evidence.md.
- result: Blocks 2 and 3, the trailing copy loop, the prologue and the epilogue are already register-exact and instruction-exact. Also measured for the first time (s29 priced it by arithmetic only): the target's block-1 reload at 80034FB4 occupies the load-delay slot that b0 fills with a nop, so rt (with the reload) and b0 (without it) are both 49 insns and both score 10 -- the reload is free and is a passenger of the seat assignment, not a lever.
- verdict: CONFIRMED

## [s42] Duplicating block 1's flag-byte read into one of its arms produces an object byte-identical to the base body, because cse forwards the arm read back to the block-0 value.
- mechanism: cse enters the mem stored by block 0 into its hash table keyed on the address rtx; block 1's arm read hashes to the same address and is replaced by the stored register. A (mem:QI census over the cc1 dumps names the pass: the body reaches .rtl and .jump with FOUR flag-byte loads (the count the target has) and leaves .cse with THREE, unchanged thereafter through .loop, .cse2, .combine and .greg.
- probe: p1 (block 1's THEN arm reads *q, else arm keeps m) and p2 (mirror, ELSE arm reads *q) scored with `sandbox func_80034F88 --disable all`, objects compared with cmp against b0.o, plus `pwsh tools/grinder/dump.ps1 func_80034F88` on p1 and a per-pass mem:QI census of the function slice.
- result: p1 = 10 at 49, p2 = 10 at 49, and BOTH objects are byte-identical to b0.o. This closes s43's Frontier A for the only ordinary consumer the function has: after block 1's store the flag byte holds m|bit, not m, so no later block can legitimately consume m, and a re-read inside block 1 is forwarded away. Banked as rejected/s44-block1-thenarm-duplicate-read-cse-forwarded-score10-IDENTICAL.c and rejected/s44-block1-elsearm-duplicate-read-cse-forwarded-score10-IDENTICAL.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 re-measured 10 at 49, rules_dropped 0); single declared `u8 *q`; no FAKE constructs present, fake_ablate has nothing to ablate on this chassis

## [s42] Deleting block 0's uses of q so that the pointer's live range starts later does not move q's allocno: the address constant is still materialised at the top of the function, q keeps $a0, and the symbol-addressed block-0 access costs an extra instruction.
- mechanism: s43's e1 attacked the live-length exit by hoisting a computation out of q's range and the scheduler put it back; p5/p6 attack it from the opposite end by removing q's earliest references entirely. The address materialisation is a dependence-free constant load, so the first scheduling pass places it immediately after the call regardless of where the source assigns the pointer.
- probe: p5 (block 0 wholly through the bare symbol, q first assigned inside block 1) and p6 (block 0's load through the symbol, q assigned immediately before block 0's store), scored with `sandbox func_80034F88 --disable all` and disassembled.
- result: p5 = 12 at 50, p6 = 11 at 50. In both, lui a0 / addiu a0 is emitted at 4f48-4f4c immediately after the call -- ahead of the symbol-addressed block-0 load -- and q still receives $a0. Banked as rejected/s44-block0-wholly-symbol-q-from-block1-score12.c and rejected/s44-block0-load-symbol-q-before-store-score11.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49); single declared `u8 *q`; no FAKE constructs present

## [s42] Handing one flag block back to the bare symbol to shorten q's live range is monotone in the number of blocks handed back and has no minimum below the base body's 10.
- mechanism: A block addressed through the bare symbol emits lui $at with %lo folded into each memory operand, removing two references and several insns of live length from q's allocno but also removing the target's lui+addiu address form for that block.
- probe: p3 (q covers blocks 0/1/2, block 3 through the symbol) and p4 (q covers blocks 0/1/3, block 2 through the symbol), scored with `sandbox func_80034F88 --disable all`, read together with s40's a1/a2 which handed back both blocks and measured 28 at 48.
- result: p3 = 16 at 49, p4 = 20 at 50, both-blocks-back = 28. One block back costs 6 points, the other 10, both cost 18. Banked as rejected/s44-block3-via-bare-symbol-score16.c and rejected/s44-block2-via-bare-symbol-score20.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49); single declared `u8 *q`; no FAKE constructs present

## [s42] Making the loop counter's allocno live across the flag blocks, so that it holds $v1 and denies that seat to the block-0 value, costs four instructions and moves the score away from the target.
- mechanism: The .lreg records for the base body show the loop counter as pseudo 73 (11 references across 7 insns, the function's highest allocno_compare key at 47142); it takes $v1 first, and the block-0 value then also takes $v1 because the two do not conflict. Initialising the counter at the top of the function (ordinary split-init) makes it conflict with every flag-block quantity.
- probe: p7 (`i = 0;` hoisted above `p = func_80077D00();`, loop spelled `for (; i < 3; i++)`), scored with `sandbox func_80034F88 --disable all`.
- result: p7 = 24 at 53 insns: the counter now also conflicts with q and with p, and the allocation degrades by four extra instructions. Banked as rejected/s44-loop-counter-init-hoisted-to-top-score24.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49); single declared `u8 *q`; no FAKE constructs present

## [s42] Cutting the block-0 value's reference count by having block 1 name it once also shortens its live range, so its allocno_compare key rises instead of falling, and the body loses the target's two-arm value selection.
- mechanism: allocno_compare's key is floor_log2(refs)*refs/live_length (tools/gcc-2.7.2/global.c:635); the base body's block-0 value is 6 refs over 9 insns (13333) against q's 10 over 28 (10714). Every reference removed is also a point at which the value was still live, so refs and live_length fall together and the quotient does not improve -- an arithmetic reason distinct from the local-alloc splitting s43 identified.
- probe: p8 (block 1 spelled `c = m; if (p[8] & 1) { c = c | 1; }`, one reference to m instead of two), scored with `sandbox func_80034F88 --disable all`.
- result: p8 = 13 at 47 insns -- two instructions short of the target, because the else arm's `addu v0,<val>,zero` disappears when a conditional or-in replaces the two-arm value selection. Banked as rejected/s44-block1-single-m-ref-conditional-or-score13.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c round-trip chassis on HEAD 2026-09-05 (b0 = 10 at 49); single declared `u8 *q`; no FAKE constructs present

## Frontier after s45

- **A.** LOCAL-ALLOC, NOT GLOBAL-ALLOC. s45's dumps show the discriminator between the base body and the score-0 banned body is which hard register `local-alloc.c` hands block 0's block-confined quantities: in the score-0 body block 0's POINTER is a block-confined quantity seated in `$v1` and block 0's byte VALUE is seated in `$a0`; in every single-object body the block-0 byte value is the block-confined quantity and local-alloc seats it in `$v1`, which is what puts hard reg 3 into `q`'s conflict row. The target's own disposition pair (address `$v1`, value `$a0`) is the score-0 pair. Next probe: read `local-alloc.c`'s quantity ordering (`qty_order`, `qty_compare`) and `find_free_reg`'s `qty_phys_copy_sugg` / `qty_phys_sugg` path, and identify the ordinary-C property of block 0 that makes its byte value take `$a0` rather than `$v1` -- e.g. whether an additional block-0-confined quantity of higher local priority can be made to take `$v1` legitimately, or whether the suggestion for `$a0` comes from a copy the source can create.
- **B.** EXIT #5 (`regs_someone_prefers`). `find_reg` pass 0 excludes registers preferred by lower-priority conflicting allocnos (global.c:1001, built at global.c:911-931). If `q` (75) or `p` (72) carried a hard-reg full preference for reg 3, pseudo 74 would skip `$v1` in pass 0, take `$a0`, and `q` would get `$v1` -- **without changing the allocno order at all**, so none of s39's four numeric exits has to be forced. The only generator is `set_preference` (global.c:1700-1754), which needs a copy insn between `q` and a pseudo local-alloc has already renumbered to reg 3. y1/y2 show an anonymous block-0 address does NOT produce that copy (GCC re-materialises the symbol for `q`). Next probe: find an ordinary-C construct that puts a copy edge between `q` and a block-0-confined quantity without declaring a second pointer object -- the obvious candidate class is a copy through a NON-pointer quantity that is nonetheless in the same reg (e.g. an integer local that block 0 computes and block 1 consumes and that combine turns into a register copy).
- **C.** Do not re-attack the "stretch the block-0 value's live range" exit by statement order: s45 measured `reg_live_length[74]` invariant at 6 refs / 9 insns under two different source orderings (x3, x4), because it is computed after combine re-normalises the insn order.

## [s45] MANDATED KILL RE-AUDIT: the base body re-measures at 10 / 49 on today's chassis, and the two closest-to-target banked bodies (m1, rt) re-measure at 10 / 49 as well; the chassis still carries no FAKE construct for `fake_ablate.py` to ablate.

- probe: `sandbox func_80034F88 --disable all` with `memory/grind/func_80034F88/candidate.c` spliced in; then `.greg`/`.lreg` dumps of `m1` (`rejected/s43-mask-folded-single-stmt-localalloc-v1-score10.c`) and `rt` (`rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c`).
- result: b0 = 10 at 49, m1 = 10 at 49 chassis-consistent, rt = 10 at 49 chassis-consistent. As s41/s42 recorded, no FAKE construct is present in the base chassis, so no banked kill on it was measured with a FAKE carrier occupying the contested pseudo.
- verdict: CONFIRMED.

## [s45] In the base body neither contested allocno carries ANY hard-register preference, and pseudo 74 takes `$v1` purely as the lowest-numbered register free in `find_reg`'s pass 0.

- mechanism: global.c:1052 (`find_reg` pass-0 scan) with `regs_someone_prefers[74]` empty, `hard_reg_copy_preferences[74]` empty and conflicts `{2,29}`.
- probe: `BB2_FINDREG_DEBUG=74` and `=75` on the base body via `tmp/grind/func_80034F88/s45/fr.sh`; `.greg` preference lines for the same body.
- result: pseudo 74 -- conflicts `2 29`, someone_prefers empty, own_copy_prefs empty, pass0_used `0 1 2 16..23 26..31`, so reg 3 is chosen; pseudo 75 -- conflicts `2 3 29`, pass0_used adds 3, so reg 4 is chosen. `.greg` prints preference lines only for 77/80/84 (all `2`).
- verdict: CONFIRMED.

## [s45] There is a FIFTH numeric exit from the seat swap that s39's four-exit enumeration could not see: a hard-reg full preference for reg 3 on a lower-priority conflicting allocno (`q` or `p`) makes pseudo 74 skip `$v1` in pass 0, without any change to the allocno order.

- mechanism: `prune_preferences` (global.c:911-931) builds `regs_someone_prefers[a]` from the `hard_reg_full_preferences` of lower-priority conflicting allocnos; `find_reg` ORs that set into the pass-0 exclusion mask (global.c:1001). Generator: `set_preference` (global.c:1700-1754), whose `reg_renumber[src] >= 0` branch treats a locally-allocated pseudo as a hard register.
- probe: source read plus the FINDREGDBG dumps above (which print `someone_prefers` explicitly), and the score-0 body's `;; 77 preferences: 3` line, which is that generator firing.
- result: the exit is real and is the mechanism the score-0 banned body uses. No ordinary-C generator for it has been found yet on a single-object chassis.
- verdict: CONFIRMED.

## [s45] Every measured body that WINS the allocno priority race loses on `q`'s conflict row for the same reason: the spelling that cuts the block-0 value's reference count also makes block 0's byte value a block-0-confined quantity, which local-alloc seats in `$v1` while `q` is live.

- mechanism: local-alloc seats block-confined quantities before global alloc runs; a locally-allocated quantity live across `q`'s range puts its hard register into `q`'s `hard_reg_conflicts` row (global.c `record_conflicts` via `reg_renumber`).
- probe: `.greg`/`.lreg` of m1 and rt against b0.
- result: m1's order is `73 78 81 85 75 74 ...` (q FIRST) but `75 conflicts: ... 2 3 29`; rt's q (74, 15 refs) is allocated second overall but `74 conflicts: ... 2 3 29`; b0's q row is clean (`... 2 29`) and it loses the order race instead. Block-0-confined quantities in m1 (76) and rt (75, 76) are all seated in hard reg 3.
- verdict: CONFIRMED.

## [s45] Moving statements in the C source between the block-0 value's definition and its uses does not change `reg_live_length` for that pseudo, so s39's "m livelen >= 12" exit is not reachable by statement order.

- mechanism: `reg_live_length` is recomputed by flow.c after combine has re-normalised insn order, so a source-level reordering that combine undoes leaves the allocno geometry untouched.
- probe: x3 (block-1's `p[8]` load hoisted between the value's def and the mask) and x4 (block-0's store moved inside block 1, after the condition), both measured and both dumped.
- result: x3 = 13 at 49; x4 = 13 at 49. x4's `.lreg` reads `Register 74 used 6 times across 9 insns` -- identical to b0 -- and its `.greg` allocation order, conflict rows and dispositions are identical to b0's. Hoisting the condition all the way to function scope (x1/x2) does insert instructions but collapses the body to 45 insns (13); hoisting two conditions (x5) gives 25 at 40.
- verdict: KILLED (instance).
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 (b0 = 10, 49/49, rules_dropped 0); bodies x1-x5, single declared `u8 *q`, no FAKE construct present.

## [s45] Spelling block 0 through the bare symbol so that its address is an anonymous block-confined pseudo does not give `q` a copy preference for reg 3: GCC re-materialises `&D_80106A73` at the top of the function for `q` rather than copying block 0's temp into it.

- mechanism: `set_preference` (global.c:1700-1754) only fires on an actual copy insn; with no named block-0 pointer there is no value for cse to forward into `q`, so the address is re-emitted and no preference edge exists.
- probe: y1 (block 0 = `D_80106A73 = D_80106A73 & 0xF8;`, `q` declared and first assigned at block 1's head) and y2 (same with a named `u8` mask local), measured and dumped.
- result: y1 = 16 at 51, y2 = 16 at 51. y1's `.greg`: `q` is pseudo 74, disposition 4, conflict row `... 2 3 29`, and NO `74 preferences:` line at all; the block-0 quantity in hard reg 3 is 79 (`2 times across 6 insns in block 0`).
- verdict: KILLED (instance).
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05; bodies y1/y2, single declared `u8 *q`, no FAKE construct present.
- kill_scope: instance

## [s43] MANDATED KILL RE-AUDIT: the base body and the two banked bodies closest to the target re-measure at their banked values on today's chassis, and the chassis still carries no FAKE construct for fake_ablate.py to ablate.
- mechanism: Chassis-relative re-measurement of banked scores before spending any new probe, as required when the floor has been flat.
- probe: sandbox func_80034F88 --disable all with memory/grind/func_80034F88/candidate.c spliced over the INCLUDE_ASM line; then .greg/.lreg dumps of m1 (rejected/s43-mask-folded-single-stmt-localalloc-v1-score10.c) and rt (rejected/roundtrip-fresh-pseudo-target-census-score10-DEAD-ARITH.c).
- result: b0 = 10 at 49/49, rules_dropped 0. m1 and rt both dump the allocno geometry their banked entries describe (m1: q allocated FIRST but conflict row carries hard reg 3; rt: q at 15 refs allocated second but conflict row carries hard reg 3). s41/s42 already established fake_ablate has nothing to ablate on this chassis, so no banked kill was measured with a FAKE carrier occupying the contested pseudo.
- verdict: CONFIRMED

## [s43] In the base body neither contested allocno carries any hard-register preference, and pseudo 74 (the block-0 flag-byte value) takes $v1 purely as the lowest-numbered register free in find_reg's pass-0 scan.
- mechanism: global.c:1052 find_reg pass-0 scan, with regs_someone_prefers[74] empty, hard_reg_copy_preferences[74] empty, and hard_reg_conflicts[74] = {2,29}.
- probe: BB2_FINDREG_DEBUG=74 and =75 on the base body through tmp/grind/func_80034F88/s45/fr.sh (instrumented cc1 tools/gcc-2.7.2/cc1, granted by docs/grind/decisions.md:14784), plus the ';; NN preferences:' lines in tmp/grind/func_80034F88/s45/b0/code6cac_b.greg.
- result: pseudo 74 -- conflicts '2 29', someone_prefers empty, own_copy_prefs empty, own_full_prefs empty, pass0_used '0 1 2 16..23 26..31' so reg 3 is the first free candidate. pseudo 75 (q) -- conflicts '2 3 29', pass0_used adds 3, so reg 4 is taken. .greg prints preference lines only for 77/80/84, all for hard reg 2. Nineteen sessions of allocno_compare analysis were reading only half of the selection rule.
- verdict: CONFIRMED

## [s43] A fifth numeric exit from the seat swap exists that s39's four-exit enumeration could not see: a hard-register full preference for reg 3 on a lower-priority conflicting allocno (q or p) makes pseudo 74 skip $v1 in find_reg's pass 0 and land on $a0, leaving $v1 for q, with the allocno order left exactly as it is.
- mechanism: prune_preferences (global.c:911-931) builds regs_someone_prefers[a] from the hard_reg_full_preferences of lower-priority conflicting allocnos; find_reg ORs that set into the pass-0 exclusion mask (global.c:1001). The only generator is set_preference (global.c:1700-1754), whose reg_renumber[src] >= 0 branch treats a pseudo already renumbered by local-alloc exactly like a hard register.
- probe: Source read of global.c against the FINDREGDBG dumps (which print someone_prefers explicitly for pseudos 74 and 75), cross-checked against the score-0 banned body's ';; 77 preferences: 3' line, which is that generator firing.
- result: The exit is real and it is precisely the mechanism the score-0 banned body uses. It does not require winning the priority race that s39/s41/s43 spent three sessions on. No ordinary-C generator for it has been found yet on a single-object chassis -- see the two kills below for the two routes closed this session.
- verdict: CONFIRMED

## [s43] Every measured body that wins the allocno priority race loses on q's conflict row for one shared reason: the spelling that cuts the block-0 value's reference count also makes block 0's byte value a block-0-confined quantity, and local-alloc seats that quantity in $v1 while q is live.
- mechanism: local-alloc runs before global alloc and renumbers block-confined quantities; a renumbered quantity live across q's range puts its hard register into q's hard_reg_conflicts row, which find_reg then honours unconditionally.
- probe: Side-by-side .greg/.lreg of m1 and rt against b0 (tmp/grind/func_80034F88/s45/{b0,m1,rt}/code6cac_b.{lreg,greg}).
- result: m1's allocation order is '73 78 81 85 75 74 80 84 72' -- q IS first -- but '75 conflicts: 72 74 75 78 80 81 84 85 2 3 29'. rt's q (pseudo 74, 15 refs / 30 insns) is allocated second overall but '74 conflicts: ... 2 3 29'. b0's q row is clean ('... 2 29') and it loses the order race instead. The block-0-confined quantities are seated in hard reg 3 in both m1 (76) and rt (75, 76). The score-0 banned body inverts this: block 0's POINTER is the block-confined quantity (seated 3) and block 0's byte VALUE is seated 4, and the blocks-1 global pointer inherits ';; 77 preferences: 3' by copy. The target's own disposition pair -- address $v1, value $a0, block-0 value dead after 80034FAC because block 1 reloads at 80034FB4 -- is the score-0 pair, so the residual is a LOCAL-ALLOC seat question, not a global-alloc rank question.
- verdict: CONFIRMED

## [s43] Moving statements in the C source between the block-0 value's definition and its uses leaves reg_live_length for that pseudo unchanged at 6 references across 9 insns, so s39's 'block-0 value livelen >= 12' exit is not reachable by statement order on this chassis.
- mechanism: reg_live_length is recomputed by flow.c after combine has re-normalised the insn order, so a source-level reordering that combine undoes never reaches the allocno geometry that allocno_compare (global.c:635) sorts on.
- probe: x3 (block-1's p[8] load hoisted between the value's definition and the mask) and x4 (block-0's store moved inside block 1, after the condition), both measured with sandbox --disable all and both dumped; x1/x2 (condition hoisted to function scope) and x5 (two conditions hoisted) measured as the insert-real-instructions comparison.
- result: x3 = 13 at 49, x4 = 13 at 49. x4's .lreg still reads 'Register 74 used 6 times across 9 insns' -- identical to b0 -- and its .greg allocation order ('73 77 80 84 74 75 79 83 72'), conflict rows and dispositions are identical to b0's as well. x1 = 13 at 45, x2 = 13 at 45, x5 = 25 at 40: inserting real instructions does move the geometry but collapses the body. All five banked in memory/grind/func_80034F88/rejected/.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 (b0 = 10, 49/49, rules_dropped 0); bodies x1-x5, single declared `u8 *q`, no FAKE construct present.

## [s43] Spelling block 0 through the bare symbol, so that block 0's address becomes an anonymous block-confined pseudo and q is declared and first assigned at block 1's head, does not give q a copy preference for reg 3.
- mechanism: set_preference (global.c:1700-1754) fires only on an actual copy insn. With no named block-0 pointer there is no value for cse to forward into q, so GCC re-materialises &D_80106A73 at the top of the function for q and no preference edge is ever created.
- probe: y1 (block 0 = `D_80106A73 = D_80106A73 & 0xF8;`, q declared and first assigned at block 1's head) and y2 (same with a named u8 mask local), measured with sandbox --disable all and dumped.
- result: y1 = 16 at 51, y2 = 16 at 51. y1's .greg shows q as pseudo 74 with disposition 4, conflict row '72 74 77 78 81 82 85 86 2 3 29' (hard reg 3 still present), and no '74 preferences:' line at all; the block-0 quantity occupying hard reg 3 is pseudo 79 ('2 times across 6 insns in block 0'). The copy edge the score-0 body gets for free requires block 0's address to be a NAMED object whose value cse can forward, which is the banned multi-handle shape. Both bodies banked in memory/grind/func_80034F88/rejected/.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05; bodies y1/y2, single declared `u8 *q`, no FAKE construct present.

## [s46] The uniform four-block reload spelling with no named `m` at all is byte-identical to the candidate.c chassis, so the extra named local in candidate.c is codegen-inert and the plainest human spelling is already at the floor.
- mechanism: `m` is consumed only inside block 1's arms; combine re-materialises the same value from the reload, so the two bodies converge to the same RTL after combine even though their pre-RA pseudo geometry (and therefore their .lreg/.greg quantity lists) differ.
- probe: a1 (`*q = *q & 0xF8;` then four identical `c = p[8] & K; v = *q; if (c) c = v | K; else c = v; *q = c;` blocks) measured with sandbox --disable all, then `mipsel-linux-gnu-objdump -d` of its object diffed against tmp/grind/func_80034F88/s44/b0.o via tmp/grind/func_80034F88/s46/cmp.sh.
- result: 10 at 49, objdump diff EMPTY (script prints IDENTICAL). a1's .lreg has three block-0-confined quantities (75, 76, 79) against b0's fewer, and a1's .greg allocates q FIRST among the contenders (order `73 78 82 86 74 77 81 85 72`) where b0 allocates it fourth -- so identical bytes do not imply identical allocation. Banked as rejected/s46-uniform-four-block-reload-no-m-BYTE-IDENTICAL-TO-b0-score10.c and recommended as the submission spelling if this function ever becomes submittable.
- verdict: CONFIRMED

## [s46] A block-0-confined local-alloc quantity in this function takes hard reg 2 when $v0 is dead across its span and hard reg 3 otherwise, and it is that reg-3 seat -- not allocno rank -- that puts hard reg 3 into q's conflict row.
- mechanism: find_free_reg's exclusion set is only `regs_live_at` unioned over the quantity's span (local-alloc.c:2169-2171) plus the class complement, and the scan is numeric (local-alloc.c:2247, no REG_ALLOC_ORDER on this target). Local-alloc runs before global-alloc, so no global pseudo is renumbered yet and the only low allocatable register that can appear in regs_live_at is $v0 (2), live from the call to func_80077D00 until the `(set (reg p) (reg:SI 2 v0))` copy. sched1 computes INSN_PRIORITY within a basic block only, so when `p` has no consumer inside block 0 the copy has priority 0 there and is emitted last, keeping $v0 live across block 0's quantities.
- probe: a1 (no use of p in block 0) and a2 (m1's mask-folded body with `c0 = p[8] & 1;` hoisted to the top so p IS consumed in block 0), both measured with sandbox --disable all and both dumped with tools/grinder/dump.ps1 (slices in tmp/grind/func_80034F88/s46/dump_a1 and dump_a2).
- result: a1 = 10 at 49, block-0 quantities 75 and 76 both seated in 3, `74 conflicts: ... 2 3 29`, dispositions `72 in 5  73 in 3  74 in 4 (q)  75 in 3  76 in 3  77 in 3`. a2 = 14 at 49, block-0 quantity 77 seated in **2** -- the first reg-2 block-0 seat in the ledger, confirming the law's converse -- but the byte-value quantity 78 then takes 3 (the next free number) and q (76) still lands in 4. Freeing $v0 relocates the reg-3 seat from one block-0 quantity to another; it does not free $v1 for q.
- verdict: CONFIRMED

## [s46] Making $v0 die early by giving `p` a consumer inside block 0 does not deliver $v1 to q on a single-pointer-object chassis.
- mechanism: with $v0 dead, the first block-0 quantity takes reg 2 and the SECOND takes reg 3 by the same numeric scan (local-alloc.c:2247); q's conflict row therefore still gains 3, and find_reg (global.c:1052) still hands q the next free register, 4.
- probe: a2 = m1's mask-folded body with `c0 = p[8] & 1;` hoisted above the `q`/mask statements and block 1 reading `c = c0;`. Measured with sandbox --disable all and dumped.
- result: 14 at 49 (four points worse than the floor); dispositions `72 in 5  73 in 3  75 in 3  76 in 4 (q)  77 in 2  78 in 3`. Banked as rejected/s46-p-used-in-block0-frees-v0-localseat-2-score14.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 (re-measured 10 at 49 this session; fake_ablate reports nothing to ablate); body a2, single declared `u8 *q`, no FAKE construct present.

## [s46] find_reg carries a SIXTH exit no session had read: after the two-pass scan it overrides best_reg with a register from hard_reg_copy_preferences when that register is free and class-compatible.
- mechanism: global.c:1096-1127 -- `AND_COMPL_HARD_REG_SET (hard_reg_copy_preferences[allocno], used)` followed by a scan that replaces best_reg with a copy-preferred register of the same (or a subset/superset) class. The preferences themselves come only from set_preference (global.c:1709-1735), which fires on a copy insn where one side is a hard register or a pseudo local-alloc has already renumbered.
- probe: source read of global.c cross-checked against a1's .greg, which prints `;; 77 preferences: 3` and disposition `77 in 3` for a global allocno that is copied from a block-0-confined quantity local-alloc seated in 3.
- result: The exit is real and fires in an ordinary single-object body -- but it is inert for the residual for the same reason as exit #5: the only copy edge that could give q a preference for hard reg 3 is a copy from a block-0-confined POINTER temp, which is a second C object aliasing &D_80106A73 and therefore the axis the Judge closed. Recording it completes the enumeration of find_reg's seat routes: rank (s39), conflicts (s41/s43), regs_someone_prefers (s45), copy-preference override (s46).
- verdict: CONFIRMED

## [s46] The target's block-0 disposition (address $v1, value $a0) requires block 0 to contain a block-confined POINTER quantity, which is the structural fact behind the floor of 10 rather than any single allocation heuristic.
- mechanism: under the seat law, a block-0 quantity reaches hard reg 4 only if 2 AND 3 are both excluded across its span. 2 is excluded by $v0 (the `addu $a1, $v0, $zero` copy at 80034FA4 sits inside the block-0 value's span in the target too). 3 can only be excluded by an earlier block-0-confined quantity, and the only candidate in the target's block 0 is the ADDRESS -- which is also read in block 1 (`sb $v0, 0($v1)` at 80034FD0), so it is block-confined only if block 1's pointer is a different pseudo that global alloc seats in 3 by copy preference (global.c:1096-1127 / 1709-1735). That is exactly the score-0 banned body's .greg signature.
- probe: seat-law measurements a1/a2 this session, read against the target disassembly and against the banked z0 (rejected/macro-respelling-of-banned-four-handle-score0-DO-NOT-SUBMIT.c) .lreg/.greg recorded in s45.
- result: Consistent in every body measured across s39-s46: on a single-pointer-object chassis block 0 never contains a pointer quantity, because the pointer's materialisation is hoisted to function entry (s44 p5, s45 y1/y2) and its live range spans all four flag blocks. Every find_reg exit therefore routes through the same missing quantity, which is why rank wins (m1, rt), conflict-row surgery, statement order (x1-x5) and anonymous block-0 addresses (y1/y2) all return to 10 or worse.
- verdict: CONFIRMED

## [s44] The uniform four-block reload spelling with no named `m` at all is byte-identical to the candidate.c chassis, so candidate.c's extra named local is codegen-inert and the plainest human spelling is already at the floor.
- mechanism: `m` is consumed only inside block 1's arms; combine re-materialises the same value from the reload, so the two bodies converge to the same RTL after combine even though their pre-RA pseudo geometry (and their .lreg/.greg quantity lists) differ.
- probe: a1 (`*q = *q & 0xF8;` then four identical `c = p[8] & K; v = *q; if (c) c = v | K; else c = v; *q = c;` blocks) measured with `sandbox func_80034F88 --disable all`, then mipsel-linux-gnu-objdump -d of its object diffed against tmp/grind/func_80034F88/s44/b0.o via tmp/grind/func_80034F88/s46/cmp.sh.
- result: 10 at 49; objdump diff EMPTY (the script prints IDENTICAL). a1's .lreg carries three block-0-confined quantities (75, 76, 79) against b0's fewer, and a1's .greg allocates q FIRST among the contenders (order 73 78 82 86 74 77 81 85 72) where b0 allocates it fourth -- identical bytes do not imply identical allocation. Banked as rejected/s46-uniform-four-block-reload-no-m-BYTE-IDENTICAL-TO-b0-score10.c and recorded in candidate.c's header as the spelling to submit if this function ever becomes submittable.
- verdict: CONFIRMED

## [s44] A block-0-confined local-alloc quantity in this function takes hard reg 2 when $v0 is dead across its span and hard reg 3 otherwise, and that reg-3 seat -- not allocno rank -- is what puts hard reg 3 into q's conflict row.
- mechanism: find_free_reg's exclusion set is only regs_live_at unioned over the quantity's span (local-alloc.c:2169-2171) plus the class complement, and the scan is numeric (local-alloc.c:2247; no REG_ALLOC_ORDER on this target). Local-alloc runs before global-alloc, so no global pseudo is renumbered yet and the only low allocatable register that can appear in regs_live_at is $v0 (2), live from the call to func_80077D00 until the (set (reg p) (reg:SI 2 v0)) copy. sched1 computes INSN_PRIORITY within a basic block only, so when p has no consumer inside block 0 that copy has priority 0 there and is emitted last, keeping $v0 live across block 0's quantities.
- probe: a1 (no use of p in block 0) and a2 (m1's mask-folded body with `c0 = p[8] & 1;` hoisted to the top so p IS consumed in block 0), both measured with sandbox --disable all and both dumped with tools/grinder/dump.ps1 (slices in tmp/grind/func_80034F88/s46/dump_a1 and dump_a2).
- result: a1 = 10 at 49 with block-0 quantities 75 and 76 both seated in 3, `74 conflicts: ... 2 3 29`, dispositions 72 in 5 / 73 in 3 / 74 in 4 (q) / 75 in 3 / 76 in 3 / 77 in 3. a2 = 14 at 49 with block-0 quantity 77 seated in 2 -- the first reg-2 block-0 seat in 46 sessions, which confirms the law's converse -- but the byte-value quantity 78 then takes 3 and q (76) still lands in 4.
- verdict: CONFIRMED

## [s44] Making $v0 die early by giving `p` a consumer inside block 0 (c0 = p[8] & 1 hoisted above the pointer and mask statements) does not deliver $v1 to q on this single-pointer-object chassis; it measures 14 at 49.
- mechanism: With $v0 dead the first block-0 quantity takes reg 2 and the SECOND takes reg 3 by the same numeric scan (local-alloc.c:2247), so q's conflict row still gains 3 and find_reg (global.c:1052) still hands q the next free register, 4.
- probe: a2 measured with `sandbox func_80034F88 --disable all` and dumped with tools/grinder/dump.ps1; dispositions read from tmp/grind/func_80034F88/s46/dump_a2/code6cac_b.greg.
- result: 14 at 49 (four points worse than the floor); dispositions 72 in 5 / 73 in 3 / 75 in 3 / 76 in 4 (q) / 77 in 2 / 78 in 3. Freeing $v0 relocates the reg-3 seat from one block-0 quantity to another instead of freeing $v1. Banked as rejected/s46-p-used-in-block0-frees-v0-localseat-2-score14.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c score-10 chassis on HEAD 2026-09-05 (re-measured 10 at 49 this session; tools/fake_ablate.py reports no FAKE-annotated construct to ablate); body a2, single declared `u8 *q`, no FAKE construct present.

## [s44] find_reg carries a sixth seat exit that no session had read: after the two-pass scan it overrides best_reg with a register drawn from hard_reg_copy_preferences when that register is free and class-compatible.
- mechanism: global.c:1096-1127 -- AND_COMPL_HARD_REG_SET (hard_reg_copy_preferences[allocno], used) followed by a scan that replaces best_reg with a copy-preferred register of the same (or a subset/superset) class. The preferences come only from set_preference (global.c:1709-1735), which fires on a copy insn where one side is a hard register or a pseudo local-alloc has already renumbered.
- probe: Source read of global.c cross-checked against a1's .greg, which prints `;; 77 preferences: 3` with disposition `77 in 3` for a global allocno copied from a block-0-confined quantity that local-alloc seated in 3.
- result: The exit is real and fires in an ordinary single-object body, but it is inert for the residual for the same reason as s45's exit #5: the only copy edge that could give q a preference for hard reg 3 is a copy from a block-0-confined POINTER temp, i.e. a second C object aliasing &D_80106A73, which is the axis the Judge closed. Recording it completes the enumeration of find_reg's seat routes: rank (s39), conflicts (s41/s43), regs_someone_prefers (s45), copy-preference override (s46).
- verdict: CONFIRMED

## [s44] The target's block-0 disposition (address $v1, value $a0) requires block 0 to contain a block-confined POINTER quantity, and no body measured on the single-pointer-object chassis in s39-s46 produces one.
- mechanism: Under the seat law a block-0 quantity reaches hard reg 4 only if 2 AND 3 are both excluded across its span. 2 is excluded by $v0 (the addu $a1,$v0,$zero copy at 80034FA4 sits inside the block-0 value's span in the target too). 3 can only be excluded by an earlier block-0-confined quantity, and the only candidate in the target's block 0 is the ADDRESS -- which is also read in block 1 (sb $v0,0($v1) at 80034FD0), so it is block-confined only if block 1's pointer is a different pseudo that global alloc seats in 3 by copy preference (global.c:1096-1127 / 1709-1735). That is exactly the score-0 banned body's .greg signature.
- probe: The a1/a2 seat-law measurements read against the target disassembly (asm/funcs/func_80034F88.s) and against the banked z0 .lreg/.greg recorded in s45.
- result: Consistent in every body measured s39-s46: on a single-pointer-object chassis block 0 never contains a pointer quantity, because the pointer's materialisation is hoisted to function entry (s44 p5, s45 y1/y2) and its live range spans all four flag blocks. That is why rank wins (m1, rt), conflict-row surgery, statement order (x1-x5) and anonymous block-0 addresses (y1/y2) all return to 10 or worse. This gives s40's 'two registers hold &D_80106A73 simultaneously at .L80034FC8' a mechanism instead of an observation.
- verdict: CONFIRMED

## Frontier after s47

1. **Ruling-request on the arity finding.** s47 proves the target needs two
   pointer pseudos and that C can only supply the second by naming a second
   pointer object.  The Judge closed the multi-handle axis three times, but on
   each occasion the record framed the second handle as a register-allocation
   *lever* (a coercion whose only purpose was to move a seat).  s47 reframes it
   as an *emission* fact read off the target: the shipped code holds the
   address in two registers with disjoint-but-overlapping ranges, which no
   single C object can express.  The precise question to put is whether that
   changes the classification, or whether the ban stands and the function is
   foreclosed at 10.  Do NOT submit a two-handle body -- the brief rejects a
   candidate-ready that re-declares a banned construct before the Judge sees it.
2. **The header-declaration axis (aggregate merge) is the only untried
   non-alias surface.**  `D_80106A70` is declared `extern u8 D_80106A70;` in
   code6cac.h yet indexed with a computed register by the copy loop, and
   `D_80106A73` has no header declaration at all.  Declaring the word as an
   array or a record changes the RTL the flag blocks start from (and would also
   retire the `*(&D_80106A70 + i)` declaration pun that currently makes
   candidate.c layer-1-unsubmittable).  It does not obviously produce a second
   pointer pseudo -- route 3 in the s47 enumeration forecloses on the `0($reg)`
   vs `3($reg)` byte difference -- but the pun must be retired regardless, and
   the aggregate-merge family needs base-register or stride evidence, which the
   copy loop's `addu $at, $at, $v1` at 80035024 supplies.
3. **The LADDER EXHAUSTED record.**  Six modalities were met at s31, the floor
   has been flat at 10 since s26, and s47 supplies the structural predicate the
   record was missing.  When an `escalation`-modality session is dispatched it
   should file `## <date> -- func_80034F88 -- **LADDER EXHAUSTED (non-endgame
   residual, floor 10): FORECLOSED**` (NOT the endgame-lock title: 10 >
   ENDGAME_LOCK_MAX_FLOOR=5, owner ruling 2026-09-02), citing the s47 two-pseudo
   arity proof (mips.h:2300 + local-alloc.c:1080-1115), the six-exit find_reg
   enumeration, scan_hand_coded tier=LOW 0/8, and the negative SOTN census.
   Re-activation triggers: a class grant covering a second address handle, or a
   toolchain finding that gives GCC 2.7.2 live-range splitting.

## [s47] MANDATED KILL RE-AUDIT: the chassis is unmoved and the two closest-to-target banked kills re-measure at their banked values.

- **Probe.** `tmp/grind/func_80034F88/s45x/run.ps1` on `candidate.c` and on the
  two 11-point banked forms; `tools/fake_ablate.py` on `candidate.c`.
- **Result.** candidate.c = **10 at 49**;
  `rejected/s44-block0-load-symbol-q-before-store-score11.c` = **11 at 50**;
  `rejected/s39-block1-reads-symbol-directly-score11.c` = **11 at 50**;
  fake_ablate = "no FAKE-annotated constructs found ... nothing to ablate".
- **Verdict.** CONFIRMED (chassis intact, no banked kill FAKE-contaminated).

## [s47] The target holds the flag-byte address in TWO hard registers ($v1 across blocks 0-1, $a0 across blocks 2-3), so it contains two pointer pseudos, and a chassis with one declared pointer object can put at most one half of the flag blocks in the right register.

- **Mechanism.** GCC 2.7.2 assigns one hard register per pseudo (no live-range
  splitting in global.c or local-alloc.c).  The target's `lui/addiu $v1` at
  80034F98 serves blocks 0 and 1 and dies at the 80034FD0 store, while
  `lui/addiu $a0` at 80034FC8 and again at 80034FF0 serves blocks 2 and 3; the
  ranges overlap at 80034FC8-FD0.
- **Probe.** Read `asm/funcs/func_80034F88.s` against
  `tmp/grind/func_80034F88/s44/b0.o.txt`.
- **Result.** The base body uses `$a0` for all three materialisations and `$v1`
  for the value, so blocks 2/3 are register-exact and blocks 0/1 are exactly
  reversed -- which accounts for the whole 10-point residual and for its
  stability across 20 sessions.
- **Verdict.** CONFIRMED.

## [s47] Spelling blocks 0 AND 1 through the bare symbol while a single declared `u8 *q` covers blocks 2 and 3 -- the exact C shape of the target's two-range geometry with one declared object -- measures 25 at 49 insns, because bare-symbol accesses never acquire a base register.

- **Mechanism.** `GO_IF_LEGITIMATE_ADDRESS` accepts a `SYMBOL_REF` address
  unconditionally on this port (`tools/gcc-2.7.2/config/mips/mips.h:2300`), so
  a bare-symbol MEM needs no pointer pseudo, cse has no cost to eliminate, and
  each access is emitted as its own `lui $at` + memory-op macro pair.
- **Probe.** Bodies `z1a` (block 1 reuses the masked `m`) and `z1b` (block 1
  re-reads the symbol, matching the target's 80034FB4 reload), spliced and
  measured; `z1a`'s object disassembled to
  `tmp/grind/func_80034F88/s45x/z1a.txt`.
- **Result.** Both **25 at 49**.  The objdump shows four separate `lui at`
  macro pairs across blocks 0/1 and a shared `a1` base only in blocks 2/3
  (where the declared pointer is).  Banked as
  `rejected/s45x-blocks01-bare-symbol-q-from-block2-score25.c` and
  `rejected/s45x-blocks01-bare-symbol-reload-q-from-block2-score25.c`.
- **Verdict.** KILLED (kill_scope: class; predicate_cite
  `tools/gcc-2.7.2/config/mips/mips.h:2300`).  The class killed is
  "obtain the second pointer pseudo from bare-symbol accesses"; the predicate is
  the port's acceptance of `SYMBOL_REF` as a legitimate address, which holds for
  every spelling of a bare-symbol access in this function.

## [s47] No pass between flow and reload can split one pointer pseudo across two hard registers, so the compiler-side route to the target's two address registers is closed.

- **Mechanism.** `update_equiv_regs` (local-alloc.c:947-1116) is the only pass
  that acts on a pseudo with a constant equivalence, and its action is
  deletion, not duplication: `reg_equiv_replacement[regno] = SET_SRC (set)` at
  local-alloc.c:1080-1082 when `reg_n_refs[regno] == 2 && reg_basic_block[regno]
  < 0`, then substitution into the single use and `NOTE_INSN_DELETED` on the
  initialiser at local-alloc.c:1102-1115.  Reload rematerialisation performs the
  same substitution, and `q` is never spilled on any measured body.
- **Probe.** Source read of local-alloc.c:1040-1116 this session.
- **Result.** No mechanism found that gives one pseudo two hard registers; the
  only equivalence-driven transformation removes a register.
- **Verdict.** KILLED (kill_scope: class; predicate_cite
  `tools/gcc-2.7.2/local-alloc.c:1080`).

## [s45] MANDATED KILL RE-AUDIT: the chassis is unmoved at 10/49 and the two closest-to-target banked instance kills re-measure at their banked values, with no FAKE construct present to ablate.
- mechanism: tools/fake_ablate.py reports whether any banked kill was measured while a FAKE carrier occupied a contested pseudo; re-splicing the banked bodies re-prices them against today's HEAD.
- probe: tmp/grind/func_80034F88/s45x/run.ps1 on memory/grind/func_80034F88/candidate.c, rejected/s44-block0-load-symbol-q-before-store-score11.c and rejected/s39-block1-reads-symbol-directly-score11.c; python3 tools/fake_ablate.py --func func_80034F88 --file code6cac_b --candidate memory/grind/func_80034F88/candidate.c
- result: candidate.c = score 10, build_insns 49. s44-block0-load-symbol-q-before-store = 11 at 50. s39-block1-reads-symbol-directly = 11 at 50. fake_ablate: 'no FAKE-annotated constructs found in memory/grind/func_80034F88/candidate.c; nothing to ablate' (fourth consecutive session: s41, s42, s46, s47). The chassis has not moved and no banked kill is FAKE-contaminated or chassis-stale.
- verdict: CONFIRMED

## [s45] The target holds the flag-byte address in two different hard registers -- $v1 across blocks 0-1 (materialised 80034F98/F9C, dead at the 80034FD0 store) and $a0 across blocks 2-3 (materialised 80034FC8/FCC and again 80034FF0/FF4) -- so the shipped code contains two pointer pseudos, while the candidate.c chassis contains one and consequently seats blocks 2/3 register-exactly and blocks 0/1 exactly reversed.
- mechanism: GCC 2.7.2 assigns exactly one hard register per pseudo: global_alloc writes reg_renumber[] once per allocno and local_alloc seats a quantity once; there is no live-range splitting pass. Two address registers in the emitted code therefore imply two distinct pseudos in the RTL.
- probe: Read asm/funcs/func_80034F88.s instruction by instruction against the disassembly of the base body at tmp/grind/func_80034F88/s44/b0.o.txt.
- result: Confirmed on both sides. The target's value registers mirror the address swap ($a0 holds the block-0/1 value, $v1 the block-2/3 value). The base body uses $a0 for all three materialisations and $v1 for every value, which is why blocks 2 and 3, the copy loop, the prologue and the epilogue are already exact and the entire residual sits in blocks 0/1. This explains quantitatively why the floor has been exactly 10 and stable since s26: a single-pointer-object chassis can put at most one half of the flag blocks in the right register.
- verdict: CONFIRMED

## [s45] Spelling blocks 0 and 1 through the bare symbol D_80106A73 while a single declared `u8 *q` covers blocks 2 and 3 -- the exact C shape of the target's two-range address geometry using only one declared pointer object -- measures 25 at 49 insns, because on this port a bare-symbol memory access never acquires a base register.
- mechanism: GO_IF_LEGITIMATE_ADDRESS accepts a SYMBOL_REF address unconditionally (CONSTANT_ADDRESS_P accepts SYMBOL_REF), so a bare-symbol MEM needs no pointer pseudo, cse sees no address cost to eliminate, and the assembler expands each access into its own lui $at + memory-op macro pair.
- probe: Bodies z1a (block 1 reuses the masked value m) and z1b (block 1 re-reads the symbol, matching the target's 80034FB4 reload) spliced over INCLUDE_ASM and measured with sandbox --disable all; z1a's object disassembled to tmp/grind/func_80034F88/s45x/z1a.txt.
- result: z1a = 25 at 49, z1b = 25 at 49. The disassembly shows four separate `lui at,0x0` macro pairs across blocks 0/1 and a shared base register (a1) only in blocks 2/3, where the declared pointer object lives. Banked as rejected/s45x-blocks01-bare-symbol-q-from-block2-score25.c and rejected/s45x-blocks01-bare-symbol-reload-q-from-block2-score25.c. Consistent with the older bare-symbol bank (symbol-only-no-pointer 29, s40-roundtrip-q-blocks01-directsymbol23 28, s44-block2-via-bare-symbol 20, s44-block3-via-bare-symbol 16).
- verdict: KILLED
- kill_scope: class
- measured_on: candidate.c-derived score-10 chassis on HEAD 2026-09-05 (base re-measured 10 at 49 this session); single declared `u8 *q`; no FAKE constructs present (fake_ablate reports nothing to ablate)
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:2300

## [s45] No GCC 2.7.2 pass between flow and reload splits one pointer pseudo across two hard registers, so the second address register the target uses cannot be produced by the compiler from a single C pointer object.
- mechanism: update_equiv_regs is the only pass acting on a pseudo that carries a constant equivalence, and its action is deletion rather than duplication: it records reg_equiv_replacement[regno] = SET_SRC (set) when reg_n_refs[regno] == 2 and reg_basic_block[regno] < 0, then substitutes the constant back into the single use and rewrites the initialising insn as NOTE_INSN_DELETED. Reload rematerialisation performs the same substitution, and q is never spilled on any measured body.
- probe: Source read of tools/gcc-2.7.2/local-alloc.c:1040-1116 (the REG_EQUIV promotion and the replacement loop) this session.
- result: No mechanism exists that hands one pseudo two hard registers; the only equivalence-driven transformation removes a register. The other observable effect of that code, reg_live_length[regno] *= 2 at local-alloc.c:1064, only reweights global-alloc priority and does not apply here because q is set three times and so carries no single REG_EQUIV. This closes the last unread route in the enumeration and makes the two-pseudo requirement a C-level obligation.
- verdict: KILLED
- kill_scope: class
- measured_on: compiler source read (tools/gcc-2.7.2/local-alloc.c), cross-checked against the score-10 chassis on HEAD 2026-09-05 where q is never spilled; no FAKE constructs present
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1080

## s48 (solver, 2026-09-05) -- hypotheses

### H48.1 CONFIRMED -- the residual's first divergence is PRE-RA, not RA
**Statement.** Our best body and the target have the same instruction COUNT (49)
but not the same instruction MULTISET: the target emits one `lbu #,0(#)` that we
do not, and we emit one `nop` that it does not.
**Mechanism.** `tools/ra_solver/inverse_compose.py classify` compares
register-blanked objdump streams; a multiset difference is by construction
upstream of both the allocator and the scheduler, which only rename and reorder
a fixed multiset.
**Probe.** `classify code6cac_b func_80034F88 --target-object
build/src/code6cac_b.o --ours-object tmp/grind/func_80034F88/s46/ours_cand.o`.
**Result.** FIRST DIVERGENCE: PRE-RA. Consequence: the s47 arity proof and its
"non-alias ladder exhausted" conclusion are withdrawn -- they were an RA-only
reading of a pre-RA residual.

### H48.2 CONFIRMED -- cse.c is the pass that eats the target's `lbu`
**Statement.** The missing instruction is flag block 1's reload of the flag byte
(target 80034FB4), and it is removed by cse.c store-forwarding the block-0 mask
store into it; combine then folds the surviving zero_extend into a copy.
**Mechanism.** cse records a store's source as the value of the destination MEM
and substitutes it into later loads of the same address value class.
**Probe.** `pwsh tools/grinder/dump.ps1 func_80034F88` on the a1 body; compare
`.rtl` insn 29 `(set (reg:QI 80) (mem:QI (reg 74)))` against `.cse` insn 29
`(set (reg:QI 80) (subreg:QI (reg:SI 76) 0))` and `.combine` (29 becomes
NOTE_INSN_DELETED, 30 becomes `(set (reg 77) (reg 76))`).
**Result.** Confirmed; slices banked at
`tmp/grind/func_80034F88/s46/{rtl,cse,combine}_slice.txt`.

### H48.3 CONFIRMED -- blocks 2/3 keep their reloads only because they start a new cse EBB
**Statement.** The asymmetry between flag block 1 (reload eaten) and flag blocks
2 and 3 (reloads kept) is a cse extended-basic-block partition effect.
**Mechanism.** cse restarts with an empty table at each EBB; blocks 2 and 3
begin after the join `code_label` of the preceding flag test.
**Probe.** The `.cse` dump's own partition header for this function.
**Result.** `;; Processing block from 2 to 46` covers the block-0 store AND
block-1's load; `from 49 to 80` and `from 83 to 114` start blocks 2 and 3.

### H48.4 KILLED (instance) -- keeping one pointer pseudo and reordering the C does not defeat cse store-forwarding
**Statement.** On the a1 chassis, re-assigning `q = &D_80106A73;` at flag block
1's head, and duplicating flag block 1's READ into both arms of its own
conditional, each leave the block-1 reload eliminated (4 `lbu`, 3 `nop`) and the
score at 10.
**Mechanism.** cse keys the memory table on the address VALUE class; two
spellings that both denote the one pointer pseudo hand cse the same class, so
the recorded store value is substituted either way.
**Probe.** v1 and v3, spliced over INCLUDE_ASM and scored with
`sandbox --disable all`; `lbu`/`nop` counted from the sandbox object.
**Result.** v1 = 10 at 49 (4 lbu / 3 nop); v3 = 10 at 49 (4 lbu / 3 nop).
Banked as `rejected/s48-q-reassigned-at-block1-head-still-forwards-score10.c`
and `rejected/s48-block1-split-read-into-arms-still-forwards-score10.c`.
**kill_scope.** instance.
**measured_on.** a1 chassis (uniform four-block reload, single declared `u8 *q`)
on HEAD 2026-09-05; no FAKE construct present.

### H48.5 CONFIRMED -- an address expression cse cannot unify restores the target's exact instruction multiset
**Statement.** Spelling flag block 0's mask through an address expression that
cse cannot prove equal to `&D_80106A73` restores the block-1 reload and yields
the target's multiset (5 `lbu`, 2 `nop`).
**Mechanism.** cse cannot prove `(plus (symbol_ref D_80106A78) (const_int -5))`
equal to `(symbol_ref D_80106A73)`, so the store is not recorded against the
load's address class.
**Probe.** v4 (`*(&D_80106A78 - 5) = *(&D_80106A78 - 5) & 0xF8;`, purely
diagnostic -- a magic-offset pun, never submittable).
**Result.** 15 at 51, 5 `lbu` / 2 `nop`. Costs +2 insns for the second
`lui`/`addiu`. Banked as
`rejected/s48-block0-mask-via-nonunifiable-symbol-RESTORES-RELOAD-score15-51insn.c`.

### H48.6 CONFIRMED -- restoring the reload is by itself sufficient to produce the target's block-0/1 register disposition
**Statement.** Bodies that keep the block-1 reload emit the target's registers
in flag blocks 0 and 1 -- address in `$v1`, value in `$a0`, `move $v0,$a0`,
`sb $v0,0($v1)` -- with no allocator lever, no second declared pointer object
and no FAKE construct.
**Mechanism.** With the reload present, block 1's value is an independent pseudo
instead of a coalesced copy of block 0's, so the two no longer compete for one
seat; the seat the ledger spent s38-s47 trying to buy from `find_reg` falls out.
**Probe.** objdump of v4 and v5.
**Result.** Confirmed on both. This is the direct refutation of the s45/s47
"two pointer pseudos are required" reading.

### H48.7 CONFIRMED -- duplicating only the mask STORE into two arms buys the cse EBB break for one instruction
**Statement.** `m = *q & 0xF8; if (p[8] & 1) { *q = m; } else { *q = m; }`
restores the reload (5 `lbu`) at a total cost of ONE instruction; the branch,
its `lw` and its `andi` are all removed.
**Mechanism.** The condition is textually flag block 1's own `p[8] & 1`, so cse2
unifies the two computations and jump2 cross-jumps the identical arms, leaving
the branch dead. The store's presence inside a conditional arm is what denies
cse a record of the MEM at the join.
**Probe.** v6.
**Result.** 12 at 50, 5 `lbu` / 3 `nop`. Banked as
`rejected/s48-mask-store-only-into-arms-reload-restored-score12-50insn.c`.
This is the closest body yet to the target's SHAPE, though its score is worse
than the 10-point floor.

### H48.8 KILLED (instance) -- a splitting condition other than flag block 1's own test is not absorbed
**Statement.** On the v6 chassis, replacing the splitting condition `p[8] & 1`
with `m` or with the raw flag byte leaves the branch in the output at +4 insns.
**Mechanism.** Only a condition that duplicates an expression the following code
already computes can be unified by cse2 and then deleted by jump2; a fresh
condition has no duplicate to merge with.
**Probe.** v8 (`if (m)`) and v9 (mask fully inside the arms, condition on the
raw byte).
**Result.** v8 = 14 at 53; v9 = 19 at 53. Banked as
`rejected/s48-mask-store-arms-cond-on-m-branch-survives-score14-53insn.c` and
`rejected/s48-mask-in-arms-cond-on-raw-byte-score19-53insn.c`.
**kill_scope.** instance.
**measured_on.** v6 chassis (mask load unconditional, store duplicated into
arms, single declared `u8 *q`) on HEAD 2026-09-05; no FAKE construct present.

### H48.9 CONFIRMED -- v6's one surviving instruction is a sched1 priority artefact, not an allocation one
**Statement.** v6's extra `nop` is an unfilled load-delay slot after the mask's
`lbu`, caused by sched1 emitting the `$v0 -> p` copy before the `lbu`.
**Mechanism.** In the base body BB0 holds `lbu -> andi -> sb -> lw -> andi ->
bnez`, and the `sb` blocks the `lw` by memory dependence, making the `lbu` the
longest-path insn so the copy sinks into its delay slot (the target's
`move $a1,$v0` at 80034FA4). Moving the `sb` into the conditional arms breaks
that chain; the copy's path to the branch becomes longest and it is emitted
first.
**Probe.** objdump of v6 vs the base body a1.
**Result.** Confirmed. The next lever is therefore a sched1 priority question on
the v6 chassis, addressable with `tools/sched_solver` -- a model that DOES own
this residual, unlike the RA models the ledger has been searching.

## [s46] The first divergence between our best body and the target is PRE-RA: both streams are 49 instructions but the multisets differ, ours carrying an extra nop and the target an extra lbu #,0(#).
- mechanism: inverse_compose.py classify compares register-blanked objdump streams sourced from objects; an instruction-multiset difference is by construction upstream of both global/local alloc and sched.c, which only rename and reorder a fixed multiset.
- probe: python3 tools/ra_solver/inverse_compose.py classify code6cac_b func_80034F88 --target-object build/src/code6cac_b.o --ours-object tmp/grind/func_80034F88/s46/ours_cand.o (report tmp/grind/func_80034F88/s46/classify.txt)
- result: FIRST DIVERGENCE: PRE-RA. 'ours only: nop', 'target only: lbu #,0(#)'. Sessions s38-s47 modelled the residual as a pure register-seat problem on a fixed multiset; that framing is wrong, and the s47 'two pointer pseudos are required, therefore the non-alias ladder is exhausted' proof is withdrawn as an RA-only reading of a pre-RA residual.
- verdict: CONFIRMED

## [s46] The missing instruction is flag block 1's reload of the flag byte, and cse.c removes it by store-forwarding the block-0 mask store into it; combine then folds the surviving zero_extend into a register copy and deletes the load.
- mechanism: cse.c records a store's source as the value of the destination MEM and substitutes it into later loads whose address falls in the same value class; combine.c then merges the copy with its zero_extend consumer.
- probe: pwsh tools/grinder/dump.ps1 func_80034F88 on the a1 body; compare .rtl insn 29 against .cse insn 29 and .combine (slices at tmp/grind/func_80034F88/s46/{rtl,cse,combine}_slice.txt).
- result: .rtl insn 29 is (set (reg:QI 80) (mem:QI (reg/v:SI 74))); in .cse it has already become (set (reg:QI 80) (subreg:QI (reg:SI 76) 0)), i.e. the block-0 store's value; in .combine insn 29 is NOTE_INSN_DELETED and insn 30 is (set (reg 77) (reg 76)). Pass attribution is cse.c, read from the dump rather than hypothesised.
- verdict: CONFIRMED

## [s46] Flag blocks 2 and 3 keep their reloads while flag block 1 does not because blocks 2 and 3 begin a fresh cse extended basic block after the join code_label of the preceding flag test.
- mechanism: cse.c restarts with an empty hash table at each extended basic block, so a store recorded in an earlier block is not available for forwarding after a join.
- probe: The .cse dump's own extended-basic-block partition header for func_80034F88.
- result: ';; Processing block from 2 to 46' spans the block-0 store AND block-1's load; ';; Processing block from 49 to 80' and ';; Processing block from 83 to 114' start blocks 2 and 3. The residual is a cse EBB-boundary problem, not an allocator problem.
- verdict: CONFIRMED

## [s46] Re-assigning q at flag block 1's head, and duplicating flag block 1's read into both arms of its own conditional, each leave the block-1 reload eliminated at 4 lbu / 3 nop and the score at 10.
- mechanism: cse keys its memory table on the address VALUE class, so any spelling that still denotes the single pointer pseudo hands cse the same class and the recorded store value is substituted regardless of statement order or arm duplication.
- probe: v1 (q = &D_80106A73; re-inserted at block 1's head) and v3 (block-1 read duplicated into both arms), spliced over INCLUDE_ASM and scored with sandbox --disable all; lbu/nop counted from the sandbox object.
- result: v1 = score 10 at 49 insns, 4 lbu / 3 nop. v3 = score 10 at 49 insns, 4 lbu / 3 nop. Banked as rejected/s48-q-reassigned-at-block1-head-still-forwards-score10.c and rejected/s48-block1-split-read-into-arms-still-forwards-score10.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: a1 chassis (uniform four-block reload, single declared u8 *q, byte-identical to s44/b0.o) on HEAD 2026-09-05; no FAKE construct present, fake_ablate clean four sessions running

## [s46] Spelling flag block 0's mask through an address expression cse cannot unify with &D_80106A73 restores the block-1 reload and reproduces the target's exact instruction multiset of 5 lbu and 2 nop.
- mechanism: cse cannot prove (plus (symbol_ref D_80106A78) (const_int -5)) equal to (symbol_ref D_80106A73), so the mask store is never recorded against the load's address value class and the load survives to the assembler.
- probe: v4: *(&D_80106A78 - 5) = *(&D_80106A78 - 5) & 0xF8; for the mask, q = &D_80106A73 for the three flag blocks. Purely diagnostic -- a magic-offset pun that is not submittable.
- result: score 15 at 51 insns, 5 lbu / 2 nop -- the target's multiset, reached for the first time in 48 sessions. The +2 insns are the second lui/addiu materialisation. Banked as rejected/s48-block0-mask-via-nonunifiable-symbol-RESTORES-RELOAD-score15-51insn.c.
- verdict: CONFIRMED

## [s46] Restoring the block-1 reload is by itself sufficient to produce the target's flag block 0/1 register disposition, with no allocator lever, no second declared pointer object and no FAKE construct.
- mechanism: With the reload present, block 1's value is an independent pseudo rather than a coalesced copy of block 0's, so the two values no longer compete for one hard register; the address then takes $v1 and the value $a0 exactly as the target does.
- probe: objdump of v4 and v5 (tmp/grind/func_80034F88/s46/v4.txt, v5.txt) against asm/funcs/func_80034F88.s at 80034F98-80034FD0.
- result: Both emit lui/addiu $v1, lbu $a0,0($v1), andi $a0, sb $a0,0($v1), lw, lbu $a0,0($v1), andi, bnez, ori $v0,$a0, move $v0,$a0, sb $v0,0($v1) -- the target's registers. This directly refutes the s45/s47 reading that the target's two address registers require two declared pointer pseudos and that the Judge-closed multi-handle axis is the only generator.
- verdict: CONFIRMED

## [s46] Duplicating only the mask STORE into the two arms of if (p[8] & 1), with the mask load and AND left unconditional, restores the block-1 reload at a total cost of one instruction, because the branch, its lw and its andi are all removed.
- mechanism: The splitting condition is textually flag block 1's own p[8] & 1, so cse2 unifies the two computations and jump2 cross-jumps the identical arms, leaving the branch dead; putting the store inside a conditional arm is what denies cse a record of the MEM at the join.
- probe: v6: m = *q & 0xF8; if (p[8] & 1) { *q = m; } else { *q = m; } followed by the unchanged three flag blocks and copy loop.
- result: score 12 at 50 insns, 5 lbu / 3 nop. Closest body yet to the target's SHAPE although its score is above the 10-point floor; the single extra instruction is a load-delay nop, not the branch. Banked as rejected/s48-mask-store-only-into-arms-reload-restored-score12-50insn.c.
- verdict: CONFIRMED

## [s46] On the v6 chassis, replacing the splitting condition p[8] & 1 with m or with the raw flag byte leaves the branch in the emitted code at four extra instructions.
- mechanism: Only a condition that duplicates an expression the following code already computes can be unified by cse2 and then deleted by jump2; a fresh condition has no duplicate to merge with and its compare, load and branch all survive.
- probe: v8 (if (m) { *q = m; } else { *q = m; }) and v9 (mask fully inside the arms, condition on the raw byte).
- result: v8 = score 14 at 53 insns; v9 = score 19 at 53 insns. Banked as rejected/s48-mask-store-arms-cond-on-m-branch-survives-score14-53insn.c and rejected/s48-mask-in-arms-cond-on-raw-byte-score19-53insn.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: v6 chassis (mask load unconditional, mask store duplicated into arms, single declared u8 *q) on HEAD 2026-09-05; no FAKE construct present

## [s46] v6's one surviving extra instruction is an unfilled load-delay slot after the mask's lbu, produced by sched1 emitting the $v0-to-p copy ahead of the lbu once the mask store leaves the entry basic block.
- mechanism: In the base body BB0 holds the chain lbu -> andi -> sb -> lw -> andi -> bnez and the sb blocks the lw by memory dependence, making the lbu the longest-path insn so the copy sinks into its delay slot -- which is what the target does with move $a1,$v0 at 80034FA4. Moving the sb into the conditional arms breaks that chain and the copy's path to the branch becomes the longest.
- probe: objdump of v6 (tmp/grind/func_80034F88/s46/v6.txt) against the base body a1 (tmp/grind/func_80034F88/s46/a1.o.txt).
- result: Base emits lui/addiu $a0, lbu $v1,0($a0), move $a1,$v0, andi; v6 emits lui/addiu $a0, move $a1,$v0, lbu $v1,0($a0), nop, andi. The next lever is a sched1 priority question on the v6 chassis, which tools/sched_solver models exactly -- unlike the RA models the ledger has been searching for ten sessions.
- verdict: CONFIRMED
