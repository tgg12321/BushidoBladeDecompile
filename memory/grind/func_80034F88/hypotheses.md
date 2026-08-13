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
