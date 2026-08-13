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
