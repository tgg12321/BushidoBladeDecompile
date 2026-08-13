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

## Session 2 (structural, 2026-08-13)

### KILLED

**[s2] H2 — a semantics-preserving C form can move a reference from `count` to
`p` and flip the `$s0`/`$s1` roles.**
- mechanism: `global.c:635 allocno_compare` ranks allocnos by
  `floor_log2(n_refs) * n_refs / live_length * 10000 * size`. The `cc1 -da` dump
  gives the actual inputs: pseudo 72 (`count`) has 8 loop-weighted refs, pseudo 73
  (`p`) has 7, so the numerators are 24 vs 14 and `count` takes `$s0` (hard reg
  16) with `p` taking `$s1` (17). The 8-vs-7 split is `2 out-of-loop + 2 in-loop`
  for `count` versus `1 out-of-loop + 2 in-loop` for `p`, and it is determined by
  the algorithm, not by spelling: `count` must be referenced by its `la`, the
  initial `beqz` guard, the `addiu -1` and the closing `bnez`.
- probe: 18 semantics-preserving structural forms swept through
  `sandbox --disable all` with an objdump check of the loop `lw` base register
  (`tmp/grind/func_80083794/s2/sweep.py`, `sweep_out.json`, `sweep2_out.json`).
  Hoisted load; `for` header; `if` + `do/while(--count)`; `while (count--)`;
  `p[0]()` + explicit `p = p + 1`; call-then-advance; both `la` statement orders;
  each variable initialised at declaration outside the `if`; block-local `q = p`
  split; flag store re-associated after both `la`s; early-return guard (both `la`
  orders); `u32` narrowing of `count`; named flag temp with `flag + 1`;
  end-pointer walk with no counter.
- result: score 18 for the best forms (tying the s1 floor), 21-23 for the rest.
  **`p` is in `$s1` in all 18, the temp is `$v0` in all 18, and the frame is -32
  in all 18.** Zero movement on residual class B. The only way to raise `p`'s ref
  count is to add an instruction, and the 28-instruction target has no room.
- verdict: KILLED (as a structural-modality axis; see the frontier for the one
  remaining non-structural idea).

**[s2] H3 — `ori $rX,$zero,1` is a reachable C-level spelling for the flag store.**
- mechanism: it is not a spelling choice at all. `mipsel-linux-gnu-as
  -march=r3000 -O1 -G0` expands `li $rX,imm` to `ori $rX,$zero,imm` when the
  immediate needs zero extension (0x8000..0xFFFF) and to `addiu $rX,$zero,imm`
  otherwise. cc1 emits `li` for every SImode constant load (`movsi_internal2` in
  the `.greg` dump), and 1 fits `addiu`.
- probe: direct assembler measurement (`astest.sh`, `astest2.sh`) plus a corpus
  census of every `ori $rX,$zero,imm` in `asm/funcs/*.s` (`oriscan.py`,
  `oriimm.py`).
- result: `li 0x8000 -> 34088000`, `li 0xffff -> 3408ffff`, `li 0x1000 ->
  24081000`, `li 1 -> 24080001`. Corpus: 100 instances total; 97 (across 50
  functions) have imm >= 0x8000, which is exactly the assembler-forced range and
  explains the 23 PURE-C functions that made the form look reachable; the only 3
  small-immediate instances are `func_80052788` / `func_800527FC` (both
  `INCLUDE_ASM`, both surrounded by lines annotated `/* handwritten instruction */`
  GTE code) and func_80083794 itself. A C-level `|` cannot produce it either:
  GCC folds an OR against a constant and never allocates a pseudo to `$zero`.
- verdict: KILLED. regfix.txt:110 (`subst "addiu" "ori" @ 6`) is papering over an
  assembler-macro divergence with no C preimage.

### CONFIRMED

**[s2] H1 — the frame-geometry residual is not reachable from any C source under
the frozen pipeline.**
- mechanism: `calls.c:1246-1252` sets `args_size.constant = MAX (args_size.constant,
  reg_parm_stack_space)` for every expanded call; the `MAYBE_REG_PARM_STACK_SPACE`
  zeroing escape is dead because that macro is undefined in `config/mips/mips.h`,
  and the `#ifndef OUTGOING_REG_PARM_STACK_SPACE` cancellation is dead because
  `mips.h:1830` defines it. `mips.h:1822` fixes `REG_PARM_STACK_SPACE` at 16
  regardless of signature or argument count; `calls.c:1400` promotes it into
  `current_function_outgoing_args_size`; `mips.c:4464/4474` adds it to the frame
  below the callee-saves.
- probe: read the unconditional code path in the frozen compiler source (this is
  s1's probe (b), answered without needing the frame notes), cross-checked against
  the 18 measured C forms (frame -32 in every one) and s1's corpus scan.
- result: any function that expands a call gets >= 16 bytes of outgoing-argument
  area. The target's entire frame is 16 bytes and already holds 12 bytes of saves
  (`s0@4, s1@8, ra@0xC`), so it cannot contain that block. s1's probe (c) is
  subsumed by the corpus scan (1/1437).
- verdict: CONFIRMED — and therefore honest distance 0 is unreachable in pure C
  for this function, independently of residual classes B and C.

## Live frontier after session 2 (ordered)

**F1 — DISPOSITION, not grinding.** Two independent no-C-form proofs (frame
geometry from the compiler source; small-immediate `ori` from the assembler) plus
two corpus-uniqueness results (1/1437 calling functions without an outgoing-arg
area; 3/3 small-immediate `ori` instances are hand-written asm) say these bytes
were not produced by GCC 2.7.2 from C. Identity (SN Systems / PsyQ crt0 `__main`),
placement (immediately after `_start`), and neighbourhood (four adjacent functions
already owner-authorized canonical asm for ings2.c, grant 2026-08-06) all agree.
The countervailing fact is that `scan_hand_coded.py` returns tier=LOW 0/8 — but
the scanner has no frame-geometry signal and no assembler-macro signal, i.e. it
cannot see either of the two anomalies this function actually exhibits. That
tension is the open question; it is the owner's to resolve, and it is what the
standing endgame gate (1) will turn on when the driver reaches `escalation`.

**F2 — the one non-structural idea left for residual class B** (worth one probe
if a later modality wants it, but note it CANNOT reach distance 0 on its own
because class A is proven unreachable): the sanctioned
[[duplicated-statement-into-arms]] family is documented as "the proven byte-free
ref-lift for global-RA priority walls". It needs 2+ control-flow arms to duplicate
a real statement into; func_80083794 has exactly one arm, so the family does not
obviously apply, and using it would require the full FAKE-annotation +
lever-exhaustion prerequisites. Do not reach for it without a ruling.

**F3 — do NOT re-run any of these.** Killed or answered with measurements:
declaration order (s1 K2), the pin/asm form (s1 K1), the two `la` statement orders
(s1 C1 + s2 sweep), all 18 structural forms (s2), the `ori` spelling (s2 H3), the
frame minimality question (s1 (a), s2 H1 (b)+(c)), `scan_hand_coded` (s1),
duplicate-lead lookup (s1).

## [s2] H2 — $s0/$s1 are swapped relative to target because count outranks p in allocno priority, and a semantics-preserving C form that moves a reference from count to p will flip them.
- mechanism: global.c:635 allocno_compare ranks allocnos by floor_log2(n_refs)*n_refs/live_length*10000*size. cc1 -da on this TU gives the actual inputs: pseudo 72 (count) 'used 8 times across 8 insns; crosses 1 call' -> hard reg 16 ($s0); pseudo 73 (p, flagged 'pointer') 'used 7 times across 7 insns' -> hard reg 17 ($s1). Numerators 24 vs 14 — a 1.7x gap live_length cannot invert. The 8-vs-7 split is loop-weighted 2-out+2-in for count against 1-out+2-in for p, and it is forced by the algorithm: count must be referenced by its la, the initial beqz guard, the addiu -1 and the closing bnez.
- probe: Built a sweep harness (tmp/grind/func_80083794/s2/sweep.py + variants.json) that splices a body into src/ings2.c, runs `sandbox func_80083794 --disable all`, objdumps the produced .o and records the score plus which hard register is the loop's `lw` base. Swept 18 semantics-preserving structural forms: hoisted load into a named local; for(;count;count--) header; if + do/while(--count); while (count--); p[0]() with explicit p = p + 1 and count = count - 1; call-then-advance; both statement orders of the two la's; count and p each initialised at declaration outside the if; block-local q = p split; flag store re-associated after both la's; early-return guard in both la orders; u32 narrowing of count; named flag temp with D_800A2668 = flag + 1; end-pointer walk with no counter at all.
- result: Best forms tie the session-1 floor at 18; the rest score 21-23 (the score-22 family differs only in the emission order of the two masked la pairs, i.e. session-1 finding C1, and is strictly worse). p is in $s1 in ALL 18 forms, the temp is $v0 in ALL 18, and the frame is -32 in ALL 18. Zero movement on residual class B. The only way to raise p's ref count is to add an instruction, and the 28-instruction target has no room for one.
- verdict: KILLED

## [s2] H3 — `ori $rX,$zero,1` is a reachable C-level spelling for the flag store rather than an assembler-macro divergence; landing it retires regfix.txt:110.
- mechanism: It is not a spelling choice at all. mipsel-linux-gnu-as -march=r3000 -O1 -G0 expands `li $rX,imm` to `ori $rX,$zero,imm` only when the immediate needs zero extension (0x8000..0xFFFF), and to `addiu $rX,$zero,imm` otherwise. cc1 emits `li` for every SImode constant load (movsi_internal2, visible in the .greg dump) and the constant 1 fits addiu. A genuine C-level `|` cannot reach it either: GCC folds an OR against a constant operand and never allocates a pseudo to $zero.
- probe: Direct assembler measurement (tmp/grind/func_80083794/s2/astest.sh, astest2.sh) plus a full corpus census of every `ori $rX,$zero,imm` in the 1437 asm/funcs/*.s files, classified by function disposition and by immediate magnitude (oriscan.py, oriimm.py).
- result: Assembler: li 0x8000 -> 34088000 (ori), li 0xffff -> 3408ffff (ori), li 0x1000 -> 24081000 (addiu), li 1 -> 24080001 (addiu). Corpus: 100 instances total. 97 instances across 50 functions have imm >= 0x8000 — exactly the assembler-forced range, which is why 23 of those functions are matched PURE-C with zero rules and made the form look reachable to session 1's file-count heuristic. Only 3 instances have imm < 0x8000: func_80052788 and func_800527FC (both INCLUDE_ASM, both with neighbouring lines annotated /* handwritten instruction */ GTE mtc2 code) and func_80083794 itself. regfix.txt:110's `subst "addiu" "ori" @ 6` is papering over an assembler-macro divergence with no C preimage.
- verdict: KILLED

## [s2] H1 — the frame-geometry residual (~8 of the 18 instructions) is not reachable from any C source under the frozen pipeline, because current_function_outgoing_args_size cannot be 0 for a function that emits a call.
- mechanism: calls.c:1246-1252 (expand_call, constant-args branch) does args_size.constant = MAX (args_size.constant, reg_parm_stack_space) guarded only by #ifdef REG_PARM_STACK_SPACE. The one zeroing escape immediately below it — #ifdef MAYBE_REG_PARM_STACK_SPACE / if (reg_parm_stack_space == 0) args_size.constant = 0 — is dead, because MAYBE_REG_PARM_STACK_SPACE is not defined anywhere in config/mips/mips.h (the only matches in the tree are inside calls.c itself, lines 647 and 1248). The #ifndef OUTGOING_REG_PARM_STACK_SPACE cancellation is dead too, because mips.h:1830 defines OUTGOING_REG_PARM_STACK_SPACE. mips.h:1822 fixes REG_PARM_STACK_SPACE at (MAX_ARGS_IN_REGISTERS*UNITS_PER_WORD) - FIRST_PARM_OFFSET = 16 - 0 = 16 with no dependence on signature or argument count; calls.c:1400 promotes it into current_function_outgoing_args_size; mips.c:4464/4474 adds it to the frame below the callee-saves.
- probe: Session 1's probe (b) — read the frame path in the frozen compiler source. It turned out not to need the -da frame notes at all: the code path is unconditional. Cross-checked against the 18 measured C forms (frame -32 in every single one) and against session 1's corpus scan. Probe (c) (grep the ori files for a sub-16 save offset) is subsumed by that scan, which already found exactly 1/1437 calling functions with a lowest save offset below 16 — this one.
- result: Any function whose body expands a call gets >= 16 bytes of outgoing-argument area. The target's entire frame is 16 bytes (addiu $sp,-0x10) and already holds 12 bytes of register saves (s0@4, s1@8, ra@0xC), so it demonstrably does not contain that block. Confirmed: honest distance 0 is unreachable in pure C for this function, independently of residual classes B and C.
- verdict: CONFIRMED
