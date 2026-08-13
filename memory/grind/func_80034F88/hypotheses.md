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
