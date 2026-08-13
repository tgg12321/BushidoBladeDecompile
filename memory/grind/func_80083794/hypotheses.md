# Hypothesis ledger — func_80083794

## Session 1 (recon, 2026-08-13)

### KILLED

**K1 — the inherited pin/inline-asm form is anywhere near the honest floor.**
Mechanism: the sandbox strips cheat-asm, so the `register ... asm("s0"/"s1"/"t0")`
pins plus the hardcoded `__asm__("jalr %0")` / `__asm__("addiu $17,$17,-1")` body
left only 16 of 28 instructions standing. Probe: `sandbox --disable all` on the
inherited body → 23 with build_insns 16. Replacing the whole thing with ordinary
C (`while (count != 0) { (*p++)(); count--; }`) gives build_insns 28 and score 22
immediately. **Verdict: KILLED — the pin form is strictly worse and carries no
information; do not reconstruct it.**

**K2 — local *declaration* order steers the `$s0`/`$s1` assignment here.**
Mechanism: LUID / allocno numbering bias (the SOTN-sanctioned "named-intermediate
declaration order" lever). Probe: declared `s32 count;` before `void (**p)(void);`
with the assignment order held fixed → score stayed exactly 18, and the
disassembly was byte-identical. **Verdict: KILLED for this function.** (The
*assignment* order is NOT inert — see C1 — but the declaration order is.)

### CONFIRMED

**C1 — assignment order of the two `la`s is a live 4-instruction lever.**
Mechanism: the two `lui/addiu` symbol-address pairs are emitted in source order,
and the sandbox scorer masks the relocation payload, so only the destination
register of each pair is compared. Probe: moving `count = (s32)&D_00000000;`
ahead of `p = &D_8008D070;` moved score 22 → 18 (4 insns: 2 `lui` + 2 `addiu`).
**Caveat for the next session: this is a scorer artifact, not a semantic win.**
Our `$s0` holds `count` where target's `$s0` holds `p`; the pairs "match" only
because the symbol is masked. The loop body (`lw $v0,0($s1)` vs `lw $t0,0($s0)`)
still shows the roles inverted, so a true match still requires flipping the roles
(H2) — at which point the `la` emission order must flip back.

**C2 — the target frame has NO o32 outgoing-argument area despite making a call,
and it is the only such function in the executable.** Mechanism + measurement in
evidence.md (§Residual class A): `REG_PARM_STACK_SPACE` is unconditionally 16 for
this config (mips.h:1811/1822), `compute_frame_size` places saves above
`args_size` (mips.c:4466/4476), and the corpus scan finds 1/1437 calling
functions with a lowest save offset below 16 — this one.

## Live frontier (ordered)

**H1 — The frame-geometry residual is not reachable from any C source under the
frozen pipeline, because `current_function_outgoing_args_size` cannot be 0 for a
function that emits a call.**
*Mechanism:* GCC 2.7.2 mips.h `REG_PARM_STACK_SPACE(FNDECL) =
(MAX_ARGS_IN_REGISTERS*UNITS_PER_WORD) - FIRST_PARM_OFFSET(FNDECL)` with
`FIRST_PARM_OFFSET == 0`, plus `OUTGOING_REG_PARM_STACK_SPACE`; `compute_frame_size`
(mips.c:4450-4476) then lays saved registers above that block. Roughly 8 of the 18
residual instructions (`addiu $sp` ×2, 3 saves, 3 restores) are pure offset/size
diffs attributable to this one block.
*Next probe:* this is a NEGATIVE claim and must be attacked before it is believed —
recon deliberately did not certify it. Concrete falsification attempts, in order:
(a) [ALREADY ANSWERED, negative] our own build of this function IS the minimal
case — zero parameters, zero stack locals, one zero-argument indirect call — and
it still got the full 16-byte area (frame 0x20 = 16 args + 12 saves + 4 pad). Do
not re-run this; go straight to (b)/(c);
(b) instrument with `cc1 -da` and read the `.greg` / frame notes to see whether
any construct (e.g. the call being the sole statement, or `__builtin`-style
lowering) drives `current_function_outgoing_args_size` to 0;
(c) grep the 53 `ori $rX,$zero,imm` files for one that ALSO shows a small frame and
diff its C. If (a)+(b) both come back negative, H1 is confirmed and the function's
disposition question is genuinely open — but note the standard canonical-asm gate
already FAILED (scan_hand_coded tier=LOW, 0/8), so a `ruling-request` naming the
frame-uniqueness evidence is the correct move, NOT a self-declared exhaustion.

**H2 — `$s0`/`$s1` are swapped because `count` (4 refs) outranks `p` (3 refs) in
allocno priority; a C form that raises `p`'s ref count or lowers `count`'s will
flip them.**
*Mechanism:* `global.c` allocno priority is driven by `reg_n_refs` weighted by
loop depth; both pseudos are loop-resident, so the tiebreak is ref count.
*Next probe:* forms that keep the semantics but move a reference from `count` to
`p` — e.g. hoisting the load (`void (*f)(void) = *p; p++; count--; f();`), a
`for (; count; count--)` header, `do { ... } while (--count);`, or expressing the
guard through the pointer. Each is one sandbox run; measure `$s0`'s role in the
loop body (`lw $?,0($s?)`) rather than trusting the masked `la` pairs (see C1).
Worth a directed permuter sweep once 2-3 manual forms are banked.

**H3 — `ori $rX,$zero,1` (target) vs `li`/`addiu` (ours) for the flag store is a
reachable C-level spelling, not an assembler-macro divergence.**
*Mechanism:* unknown; 53/1437 `asm/funcs` files contain `ori $rX,$zero,imm`, so
the pipeline demonstrably emits the form somewhere. Candidate mechanisms: an
`IOR` that survives to RTL with a `$zero`-allocated operand, or a constant whose
`movsi` expansion picks the logical form.
*Next probe:* pick 2-3 of those 53 files that are already COMPLETED-C, find the C
statement that produced the `ori`, and copy the spelling class. This is a cheap
1-2 turn forensics probe and it retires regfix.txt:110 if it lands.

## [s1] The inherited register-pin + hardcoded-$17 __asm__ body is anywhere near the honest floor.
- mechanism: The cheat-invisible sandbox strips register-asm pins and hardcoded-$N __asm__ before scoring, so the pinned loop body contributes nothing.
- probe: sandbox func_80083794 --disable all on the inherited body vs an ordinary-C rewrite.
- result: Inherited form: score 23 with build_insns 16 of 28 (loop body gone after stripping). Ordinary C `while (count != 0) { (*p++)(); count--; }`: build_insns 28, score 22 on first try.
- verdict: KILLED

## [s1] Local declaration order steers which of p/count gets $s0 vs $s1.
- mechanism: LUID / allocno numbering bias — the SOTN-sanctioned named-intermediate declaration-order lever.
- probe: Declared `s32 count;` before `void (**p)(void);` with assignment order held fixed; re-ran sandbox and diffed the disassembly.
- result: Score stayed exactly 18 and the disassembly was byte-identical. Declaration order is inert for this function.
- verdict: KILLED

## [s1] Assignment (statement) order of the two symbol-address `la` pairs is a live lever.
- mechanism: The two lui/addiu pairs are emitted in source order and the sandbox scorer masks the relocation payload, so only each pair's destination register is compared.
- probe: Moved `count = (s32)&D_00000000;` ahead of `p = &D_8008D070;` and re-ran the sandbox.
- result: Score 22 -> 18 (4 instructions: 2 lui + 2 addiu). CAVEAT banked in the ledger: this is a scorer artifact, not a semantic win — our $s0 holds count where target's $s0 holds p, and the loop body still shows the roles inverted.
- verdict: CONFIRMED

## [s1] The target frame has no o32 outgoing-argument area despite containing a call, and that is unique in the executable.
- mechanism: GCC 2.7.2 mips.h:1822 REG_PARM_STACK_SPACE = (MAX_ARGS_IN_REGISTERS*UNITS_PER_WORD) - FIRST_PARM_OFFSET, with FIRST_PARM_OFFSET == 0 (mips.h:1811) => 16 bytes unconditionally; compute_frame_size (mips.c:4466, 4476) then places callee-saves above args_size.
- probe: Read the target frame (addiu $sp,-0x10; s0@4 s1@8 ra@0xC) vs our build (addiu $sp,-0x20; s0@16 s1@20 ra@24), then scanned all 1437 asm/funcs/*.s for calling functions whose lowest register-save offset is < 16 (tmp/grind/func_80083794/s1/framescan.py).
- result: Exactly 1 hit across the whole executable: func_80083794 itself. Every other calling function carries the 16-byte area. Our own build of this function is already the minimal case (no params, no stack locals, one zero-arg indirect call) and still gets the full 16 bytes, so 'shrink the frame with simpler C' is already answered negative.
- verdict: CONFIRMED
