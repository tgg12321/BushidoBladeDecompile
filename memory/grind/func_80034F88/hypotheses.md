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
