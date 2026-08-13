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

## Session 3 (structural, 2026-08-13)

### KILLED

**[s3] H4 — the target's prologue callee-save ORDER (residual class D) is
reachable jointly with the score-18 floor form.**
- mechanism: `tools/gcc-2.7.2/config/mips/mips.c:4680` emits the callee-save run
  from a single loop `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)`
  that serves the prologue (`store_p`, mips.c:5045) and the epilogue
  (mips.c:5175/5359) alike, so cc1 always emits both runs descending. The only
  producer of an ascending prologue is the post-reload scheduler reordering the
  stores, and the only pressure that makes it do so is an in-block
  anti-dependence: a body instruction in the SAME basic block that writes `$sN`
  must follow `sw $sN`, so the scheduler hoists that store to unblock it. Every
  one of the 21 matched-pure-C contiguous ascending runs in the corpus has that
  shape (e.g. `func_80069A30`, `player_Destroy`, `func_8004046C`:
  `sw $s0,0x10($sp); sw $ra,0x14($sp); jal ...; addu $s0,$a0,$zero`).
- probe: (1) exact 28-instruction side-by-side accounting of the score-18
  residual (`tmp/grind/func_80083794/s3/sidebyside.md`), which isolated class D
  in the first place; (2) two corpus scans — `savescan.py` over all 1437
  `asm/funcs/*.s` (566 multi-save prologues: 193 descending, 114 ascending, 259
  mixed) and `contig.py` restricting to contiguous ascending runs (38 runs, 21 in
  zero-rule pure-C functions, ALL of length 2; func_80083794's length-3 run is
  unique in the executable); (3) a direct measurement — hoist both `la` pairs
  ahead of the `if` so the `$s0`/`$s1` writes land in the entry block, then
  sandbox + objdump.
- result: the hoisted form DOES flip the prologue to the target's ascending order
  (`sw s0,16(sp); lui s0; addiu s0; sw s1,20(sp); lui s1; addiu s1; bnez;
  sw ra,24(sp)`) — class D is genuinely C-reachable in isolation — but scores
  **23 against the 18 floor**. The constraint is COUPLED: the only writes to
  `$s0`/`$s1` in this function ARE the two `la` pairs, so the anti-dependence
  exists only when the `la`s sit in the entry block, whereas target emits both
  `la` pairs AFTER the `bnez`, in the post-branch block, where they exert no
  scheduling pressure on the prologue. Target exhibits ascending saves AND
  post-branch `la`s simultaneously; cc1 + this scheduler yields one or the other.
  Banked: `rejected/hoisted-la-flips-save-order-but-costs-5.c`.
- verdict: KILLED (jointly unreachable; the isolated reachability is CONFIRMED
  and is why it is worth recording rather than assuming impossibility).

### CONFIRMED

**[s3] C3 — the residual is now fully accounted instruction by instruction, and
the target's frame is byte-exactly cc1's LEAF-function frame.**
- mechanism: `mips.c:4464-4476`, `total_size = var_size + args_size +
  extra_size` (each `MIPS_STACK_ALIGN`ed) with `gp_reg_size` on top.
- probe: objdump the sandbox `.o` against `asm/funcs/func_80083794.s` and
  tabulate all 28 positions (`sidebyside.md`); decompose the target's 0x10 frame.
- result: 9 of 28 instructions match; 19 differ; the scorer counts 18 (the four
  masked `la` instructions are compared on destination register only). Target's
  0x10 frame = `var_size 0 + args_size 0 + extra_size 0 + 12 bytes of saves`
  rounded to 16 — precisely what cc1 emits for a LEAF function with three
  callee-saves, for a body containing a `jalr`. This is a tighter phrasing of
  s2's H1 than "the 16-byte outgoing-arg block is missing": the entire frame is
  leaf-shaped.
- verdict: CONFIRMED

## Live frontier after session 3 (ordered)

**F1 — DISPOSITION, not grinding (unchanged, now with a third uniqueness
result).** Three independent corpus-uniqueness results and two no-C-form proofs:
the frame (1/1437 calling functions lack the outgoing-arg block, and the frame is
byte-exactly leaf-shaped), the small-immediate `ori` (3/3 instances in the
executable are hand-written asm), and now the contiguous length-3 ascending
prologue save run (1/1437 — every other contiguous ascending run in the corpus is
length 2). Identity (SN Systems / PsyQ crt0 `__main`), placement (immediately
after `_start`), and neighbourhood (four adjacent functions already owner-
authorized canonical asm for ings2.c, grant 2026-08-06) agree. The countervailing
fact remains `scan_hand_coded.py` tier=LOW 0/8 — a scanner with no frame-geometry
signal, no assembler-macro signal, and no prologue-order signal, i.e. blind to all
three anomalies. That tension is the owner's to resolve at `escalation`.

**F2 — residual class B ($s0/$s1 roles) via the sanctioned
[[duplicated-statement-into-arms]] ref-lift.** Unchanged from s2, and session 3
adds a reason for caution: the residual is now known to contain FOUR coupled
classes, and closing B alone leaves A, C and D. Requires a ruling before any such
form is written.

**F3 — do NOT re-run any of these.** Cumulative kill list across s1/s2/s3:
declaration order (s1 K2); the pin/asm form (s1 K1); both statement orders of the
two `la` pairs (s1 C1 + s2 sweep); all 18 structural forms in
`rejected/structural-refcount-forms-do-not-flip-s0-s1.c` (s2 H2); the `ori`
spelling (s2 H3); frame minimality via simpler C (s1 (a)), via the compiler
source (s2 H1 (b)) and via the corpus (s1/s2 (c)); `scan_hand_coded` (s1); the
duplicate-lead lookup (s1); and now the prologue-save-order axis (s3 H4) —
including the specific hoisted-`la` form, which is measured at 23 and banked in
`rejected/hoisted-la-flips-save-order-but-costs-5.c`.

## [s3] H4 — the target's ascending prologue callee-save order (new residual class D) is reachable jointly with the score-18 floor form.
- mechanism: mips.c:4680 emits the callee-save run from a SINGLE loop `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)` serving both the prologue (store_p, mips.c:5045) and the epilogue (mips.c:5175/5359), so cc1 always emits both runs descending. Only the post-reload scheduler can reverse a run, and the only pressure that makes it do so is an in-block ANTI-DEPENDENCE — a body insn in the same basic block writing $sN must follow `sw $sN`, so the scheduler hoists that store. All 21 matched-pure-C contiguous ascending runs in the corpus have exactly that shape (func_80069A30 / player_Destroy / func_8004046C: `sw $s0,0x10($sp); sw $ra,0x14($sp); jal ...; addu $s0,$a0,$zero`).
- probe: (1) exact 28-instruction side-by-side accounting (tmp/grind/func_80083794/s3/sidebyside.md) which isolated class D; (2) two corpus scans over all 1437 asm/funcs/*.s — savescan.py (566 multi-save prologues: 193 descending, 114 ascending, 259 mixed) and contig.py (38 contiguous ascending runs, 21 of them in zero-rule pure-C functions, ALL length 2; func_80083794's length-3 run is unique); (3) direct measurement — hoist both la pairs ahead of the `if` so the $s0/$s1 writes land in the entry block, then sandbox --disable all + objdump.
- result: the hoisted form DOES flip the prologue to target's ascending order (sw s0,16 / lui s0 / addiu s0 / sw s1,20 / lui s1 / addiu s1 / bnez / sw ra,24) — class D is genuinely C-reachable in isolation — but scores 23 against the 18 floor. The constraint is COUPLED: the only writes to $s0/$s1 in this function ARE the two la pairs, so the anti-dependence exists only when the la's sit in the entry block, while target emits both la pairs AFTER the bnez where they exert no scheduling pressure. Target has ascending saves AND post-branch la's simultaneously; cc1 + this scheduler gives one or the other. Banked as rejected/hoisted-la-flips-save-order-but-costs-5.c.
- verdict: KILLED

## [s3] C3 — the score-18 residual is fully accounted instruction by instruction, and the target's frame is byte-exactly cc1's LEAF-function frame.
- mechanism: mips.c:4464-4476, total_size = var_size + args_size + extra_size (each MIPS_STACK_ALIGNed) with gp_reg_size added on top.
- probe: objdump the sandbox .o against asm/funcs/func_80083794.s and tabulate all 28 positions (sidebyside.md); decompose the target's 0x10 frame against the formula.
- result: 9 of 28 instructions match, 19 differ, scorer counts 18 (the four masked la instructions are compared on destination register only). Target's 0x10 frame = var_size 0 + args_size 0 + extra_size 0 + 12 bytes of callee-saves, rounded to 16 — precisely cc1's LEAF-function frame with three callee-saves, for a body containing a jalr. Tighter than s2's phrasing: the whole frame is leaf-shaped, not merely missing the arg block.
- verdict: CONFIRMED

## [s3] H4 - the target's ascending prologue callee-save order (new residual class D) is reachable jointly with the score-18 floor form.
- mechanism: tools/gcc-2.7.2/config/mips/mips.c:4680 emits the callee-save run from a SINGLE loop 'for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)' that serves both the prologue (store_p, called at mips.c:5045) and the epilogue (mips.c:5175/5359), so cc1 emits BOTH runs descending. Only the post-reload scheduler can reverse a run, and the only pressure that makes it do so is an in-block ANTI-DEPENDENCE: a body instruction in the same basic block that writes $sN must be scheduled after 'sw $sN', so the scheduler hoists that store to unblock it. All 21 matched-pure-C contiguous ascending runs in the corpus have exactly that shape (func_80069A30 / player_Destroy / func_8004046C all emit 'sw $s0,0x10($sp); sw $ra,0x14($sp); jal ...; addu $s0,$a0,$zero').
- probe: (1) Built the exact 28-instruction side-by-side accounting of the score-18 residual by objdumping tmp/sandbox/func_80083794/ings2.o against asm/funcs/func_80083794.s (tmp/grind/func_80083794/s3/diff28.sh, ours.txt, sidebyside.md) - this is what isolated class D, which s1/s2 had folded into the frame class. (2) Two corpus scans over all 1437 asm/funcs/*.s: savescan.py (prologue save-run direction) and contig.py (contiguous ascending runs, cross-referenced against engine/queue.json + regfix.txt + asmfix.txt + inline_asm_canonical.txt + src/*.c to identify zero-rule pure-C functions). (3) Direct measurement: hoisted both 'la' pairs ahead of the 'if' so the $s0/$s1 writes land in the entry block, then sandbox --disable all + objdump of the prologue.
- result: savescan: 566 shipped functions have a multi-save prologue run - 193 descending (the cc1-natural shape), 114 ascending, 259 mixed/split by delay-slot stealing, so ascending is common and the axis is live rather than structurally forbidden. contig: 38 CONTIGUOUS ascending runs, 21 of them in functions matched pure C with zero rules (the pipeline demonstrably emits ascending runs) - but ALL 21 are runs of LENGTH 2, and func_80083794 is the only function in the executable with a contiguous ascending run of LENGTH 3 (the duplicate listing in contig.txt is its own unlabelled twin body at 0x80083804). Direct measurement: the hoisted-la form DOES flip the prologue to the target's ascending order (sw s0,16 / lui s0 / addiu s0 / sw s1,20 / lui s1 / addiu s1 / bnez / sw ra,24), so class D is genuinely C-reachable in isolation, but it scores 23 against the 18 floor. The constraint is COUPLED: the only writes to $s0/$s1 in this function ARE the two 'la' pairs, so the anti-dependence exists only when the la's sit in the entry block, whereas target emits both la pairs AFTER the bnez where they exert no scheduling pressure on the prologue. Target exhibits ascending saves AND post-branch la's simultaneously; cc1 + this scheduler gives one or the other. Banked as memory/grind/func_80083794/rejected/hoisted-la-flips-save-order-but-costs-5.c.
- verdict: KILLED

## [s3] C3 - the score-18 residual is fully accounted instruction by instruction, and the target's frame is byte-exactly cc1's LEAF-function frame rather than merely missing the outgoing-argument block.
- mechanism: mips.c:4464-4476: total_size = var_size + args_size + extra_size (each MIPS_STACK_ALIGNed), with gp_reg_size laid on top and the callee-saves placed above args_size.
- probe: Tabulated all 28 instruction positions of our build against target (tmp/grind/func_80083794/s3/sidebyside.md) and decomposed the target's 0x10 frame against the formula.
- result: 9 of the 28 instructions match; 19 differ; the scorer counts 18 (the four masked 'la' instructions at positions 11-14 are compared on destination register only - session-1 finding C1). Target's 0x10 frame decomposes as var_size 0 + args_size 0 + extra_size 0 + 12 bytes of callee-saves rounded to 16 - precisely the frame cc1 emits for a LEAF function with three callee-saves, for a body that contains a jalr. That is a tighter phrasing of session 2's H1: the whole frame is leaf-shaped, not just missing a block.
- verdict: CONFIRMED

## Session 4 (permuter, 2026-08-13)

### KILLED

**[s4] H5 — an unbiased randomized search over the C-form space around the
score-18 floor finds a form below 18 (i.e. the structural sweep of s2 simply
missed the right spelling).**
- mechanism: decomp-permuter's randomizer explores a much larger and less
  human-biased region of the C-form space than a hand-enumerated sweep — it
  mutates declaration order, statement order, temporaries, casts, loop forms,
  branch inversion, expression association and inlining. If s2's 18 hand-written
  forms were merely an unlucky sample of a space that does contain a sub-18
  form, a six-figure-iteration search seeded on the floor form should find it.
- probe: TWO campaigns via `tools/permuter_campaign.py` (telemetry in
  metrics/events.jsonl), 107,064 iterations total, on a purpose-built workspace
  (`tmp/grind/func_80083794/s4/mkws.sh`) whose `target.o` is assembled from
  `asm/funcs/func_80083794.s` lines 1-31 + `prelude.inc` (the file also carries
  the unlabelled twin body at 0x80083804, which must be truncated or it doubles
  the metric) so the function sits at offset 0 and the permuter score is the
  real per-function diff; `--stack-diffs` left ON by default, which matters here
  because the dominant residual IS a frame-size/stack-offset shift and the
  default scorer would normalize it away and false-match at 0.
  Campaign A `ws` (label `minimal-tu-clean-c`): base = the score-18 floor form,
  base_score 383, 48,633 iterations / ~22 min, best 340.
  Campaign B `ws2` (label `class-D-hoisted-la-chassis`): base = the s3
  hoisted-`la` form that DOES produce target's ascending prologue save order,
  base_score 1168, 58,431 iterations / ~22 min, best 383. Both harvested and
  stopped in-session; no campaign outlived the session.
- result: THREE closing-form families, all banked in
  `rejected/permuter-s4-proposals-cheat-or-byte-neutral.c`, none below the
  floor. (1) permuter-best 340 is `volatile unsigned char new_var;` +
  `D_800A2668 == (1 * (new_var = 0))` — a dead volatile frame-slot local
  smuggled through a multiply-by-one; a cheat by the expanded catalog AND
  measured at 29 emitted instructions against target's 28, i.e. strictly worse
  on the honest metric (frame -40, $s0/$s1 still inverted, no `ori`). Its
  permuter score fell only because the extra `sb` made the three callee-save
  stores contiguous, which the permuter's diff rewards. (2) permuter 378 is the
  sanctioned `do { while (...) {...} } while (0);` wrap — spliced into
  src/ings2.c and measured in the honest sandbox at **score 18, build_insns 28**,
  byte-identical to the floor; the 5-point permuter delta is label-numbering
  noise. (3) permuter 383 forms are staged-value / named-intermediate
  respellings that tie the base exactly. The minimum honest score observed
  across 107k iterations is 18.
- verdict: KILLED. The score-18 floor is not a sampling artifact of session 2's
  hand-enumeration; it survives an unbiased six-figure randomized search from
  two different chassis.

### CONFIRMED

**[s4] C4 — session 3's COUPLED-constraint claim (class D and the floor form are
jointly unreachable) is corroborated by search, not only by mechanism.**
- mechanism: s3 argued from mips.c:4680 + the post-reload scheduler's
  anti-dependence requirement that the ascending prologue save order and
  post-branch `la`s cannot coexist. That was an argument about the compiler; it
  had never been tested by an unbiased search seeded inside the ascending-save
  basin.
- probe: campaign B above — seed the permuter on the hoisted-`la` chassis (which
  demonstrably HAS target's ascending save order) and let it run 58,431
  iterations, checking whether it can recover the 5 lost points while keeping
  class D.
- result: it cannot. The basin starts at permuter score 1168 and the best form
  found in ~22 min is 383 — exactly campaign A's BASE score — and that form
  (`ws2/output-383-1`) is one that has walked back OUT of the ascending-save
  basin (`void (**new_var)(void) = &D_8008D070; ... p = new_var;`). The search
  independently rediscovers the floor basin rather than finding a joint form.
- verdict: CONFIRMED (as corroboration; the mechanism argument remains the
  primary evidence).

## Live frontier after session 4 (ordered)

**F1 — DISPOSITION, not grinding (unchanged; now with the search axis closed
too).** Three corpus-uniqueness results and two no-C-form proofs (see the s3
frontier) plus, now, a negative result from the one modality that could have
shown the hand-enumeration was unlucky: 107k permuter iterations from two
chassis never go below the score-18 floor, and the only forms that beat the
floor on the PERMUTER's metric are a catalog cheat (measurably worse honestly)
and a byte-neutral sanctioned wrapper. Every sanctioned C axis — structural,
prologue-order, spelling, and now randomized search — is measured dead.
`scan_hand_coded.py` remains tier=LOW 0/8 and remains blind to all three
anomalies (no frame-geometry, no assembler-macro, no prologue-order signal).
That tension is the owner's to resolve when the driver reaches `escalation`.

**F2 — residual class B via the sanctioned duplicated-statement-into-arms
ref-lift.** Unchanged from s2/s3, and session 4 adds a datum against it: the
permuter, which mutates freely, never produced ANY form that flips $s0/$s1 in
107k iterations. Still requires a ruling before any such form is written, and
still cannot reach distance 0 alone (classes A, C, D remain).

**F3 — do NOT re-run any of these.** Cumulative kill list across s1-s4:
declaration order (s1 K2); the pin/asm form (s1 K1); both statement orders of
the two `la` pairs (s1 C1 + s2 sweep); all 18 structural forms (s2 H2); the
`ori` spelling (s2 H3); frame minimality via simpler C / compiler source /
corpus (s1 (a), s2 H1); the prologue-save-order axis incl. the hoisted-`la`
form (s3 H4); `scan_hand_coded` (s1); the duplicate-lead lookup (s1); and now
randomized permuter search from BOTH the floor chassis and the class-D chassis
(s4 H5/C4), including the three specific proposal families banked in
`rejected/permuter-s4-proposals-cheat-or-byte-neutral.c`. In particular do NOT
re-propose the `do { ... } while (0)` wrap (measured byte-neutral at 18) or the
`volatile` dead-local guard (cheat, and 29 insns).

## [s4] An unbiased randomized search over the C-form space around the score-18 floor finds a form below 18 — i.e. session 2's 18 hand-enumerated structural forms were merely an unlucky sample of a space that does contain a sub-18 form.
- mechanism: decomp-permuter's randomizer explores a far larger and less human-biased region than a hand-enumerated sweep: it mutates declaration order, statement order, temporaries, casts, loop forms, branch inversion, expression association and inlining. If the floor were a sampling artifact, a six-figure-iteration search seeded on the floor form should escape it.
- probe: Built a correct per-function permuter workspace (tmp/grind/func_80083794/s4/mkws.sh): target.o assembled from asm/funcs/func_80083794.s lines 1-31 + decomp-permuter's prelude.inc with '.set gp=64' stripped for r3000 — the .s file also carries the unlabelled twin body at 0x80083804 and assembling it whole gives a 56-instruction target against a 28-instruction base, which makes the score meaningless. Minimal-TU base.c (typedef + three externs + the body) verified to compile through the real cpp|cc1 -mel|prologue_fix|maspsx|multu_pad|as pipeline to the same 28 instructions as the full-TU sandbox object; the prelude's '.set noat' cancelled with '.set at' because 'la $16,D_00000000' needs $at and the real build assembles with at enabled. --stack-diffs left ON (wrapper default) because the dominant residual IS a frame/stack-offset shift and the default scorer would normalize it away and false-match at 0. Campaign A (tmp/grind/func_80083794/s4/ws, label minimal-tu-clean-c): base = the score-18 floor form, permuter base_score 383, 48,633 iterations over ~22 min. Campaign B (tmp/grind/func_80083794/s4/ws2, label class-D-hoisted-la-chassis): base = the s3 hoisted-la form, base_score 1168, 58,431 iterations over ~22 min. Both launched/harvested via tools/permuter_campaign.py (telemetry to metrics/events.jsonl), both stopped with --stop in-session; status shows both alive:false. Every proposal re-scored on the HONEST metric (sandbox --disable all with the body spliced into src/ings2.c, and objdump instruction counts).
- result: Three closing-form families, none below the floor. (1) Campaign A best, permuter 340 (found twice, at 522 s and 992 s): 'volatile unsigned char new_var;' with 'if (D_800A2668 == (1 * (new_var = 0)))' — a dead volatile frame-slot local smuggled through a multiply-by-one. It is a cheat by the expanded catalog (volatile-coercion / dead-local family; 'new_var' is itself the naming-announces-intent signal) AND it is strictly worse honestly: 29 emitted instructions against target's 28, frame -40 vs the floor's -32, $s0/$s1 still inverted, no 'ori'. Its permuter score fell only because the extra 'sb $zero,16($sp)' made the three callee-save stores contiguous, which the permuter's diff algorithm rewards. (2) permuter 378: 'do { while (count != 0) { (*p++)(); count--; } } while (0);' — spliced into src/ings2.c and sandboxed at score 18, build_insns 28, byte-identical to the floor; the 5-point permuter delta is label-numbering noise. (3) permuter 383: staged-value / named-intermediate respellings ('count = 1; D_800A2668 = count;', 'void (**new_var)(void) = &D_8008D070; ... p = new_var;') that tie the base exactly. Minimum honest score observed across 107,064 iterations: 18.
- verdict: KILLED

## [s4] Session 3's COUPLED-constraint claim — that the target's ascending prologue save order (residual class D) and the score-18 floor form are jointly unreachable — survives an unbiased search seeded INSIDE the ascending-save basin, not just the mechanism argument from mips.c:4680 plus the post-reload scheduler's anti-dependence requirement.
- mechanism: s3 argued from the compiler source that cc1 emits both prologue and epilogue save runs descending (one loop, GP_REG_LAST -> GP_REG_FIRST) and that only an in-block anti-dependence makes the scheduler reverse the prologue run; for this function the only $s0/$s1 writes are the two 'la' pairs, so class D requires entry-block 'la's while target emits them post-branch. That is an argument about the compiler and had never been tested by search from the class-D side.
- probe: Campaign B: seed the permuter on the hoisted-la chassis (which demonstrably HAS target's ascending save order — verified by objdump: sw s0,16 emitted first) and run 58,431 iterations, watching whether it can recover the 5 lost points while keeping class D.
- result: It cannot. The class-D basin starts at permuter score 1168 and the best form found in ~22 min is 383 — exactly campaign A's BASE score, never below it — and that form (ws2/output-383-1) has walked back OUT of the ascending-save basin by re-introducing a named intermediate for the pointer ('void (**new_var)(void) = &D_8008D070; ... p = new_var;'). The search independently rediscovers the floor basin instead of finding a joint form, which is corroboration by search of the coupling argument.
- verdict: CONFIRMED

## Session 5 (permuter, 2026-08-13)

### KILLED

**[s5] H6 — residual class B ($s0/$s1 roles) is reachable by LOWERING `count`'s
allocno priority (splitting its references across two pseudos), the mirror image
of session 2's killed "raise `p`'s ref count" attack.**
- mechanism: `global.c:635 allocno_compare` ranks by
  `floor_log2(n_refs)*n_refs/live_length`, and s2 measured 8 loop-weighted refs
  for `count` against 7 for `p`. s2 attacked the numerator on `p`'s side and
  found every route costs an instruction the 28-insn target has no room for.
  The untried mirror is to give the counter's references to two different
  pseudos so neither outranks the pointer. Session 5's isolated-objective
  campaign surfaced this side of the lever for the first time:
  `s5/ws4/output-478-1` is the first form in five sessions whose loop reads
  through `$s0` (`lw v0,0(s0)`, `beqz s1`, `bnez s1`) — target's roles.
- probe: (1) disassembled the permuter find through the real pipeline
  (`s5/dis4.sh`) and read what it actually does; (2) measured six
  semantics-preserving spellings of the same intent in the honest sandbox
  (`s5/sweep.py` + `s5/variants.json`, raw `s5/sweep_out.json`), each checked
  by objdump for the loop's `lw` base register, the temp register and the frame
  size: guard-on-count/loop-on-copy with `--n`; plain copy rename; guard +
  end-pointer loop; copy placed after the `p` assignment; guard + copy + while;
  guarded pre-decrement countdown.
- result: the permuter find flips class B ONLY because it tests `new_var` while
  decrementing `count`, making the decrement dead code that flow deletes — it is
  an infinite loop and its body is missing target's `addiu $s1,$s1,-0x1`
  entirely. Of the six correct forms: scores 23 / 18 / 22 / 22 / 24 / 18, with
  `p` in `$s1`, the temp in `$v0` and the frame at -32 in **every one**; two of
  them merely tie the floor. GCC's copy-propagation folds `n = count;` back into
  a single pseudo, so a semantics-preserving "split" is not a split at all.
  Banked: `rejected/count-ref-split-forms-do-not-flip-s0-s1.c`.
- verdict: KILLED. Both directions of the allocno-priority lever are now
  measured dead, and the residual class B is characterised more sharply than
  before: target references `count` four times and `p` three times — the same
  split our build has — while assigning them the opposite hard registers, i.e.
  target's assignment is ANTI-priority under this allocator and is not produced
  by any correct C source, not merely by none we have found.

### CONFIRMED

**[s5] C5 — the permuter modality is closed for this function by arithmetic:
no pure-C form can score below 9, so no campaign can ever reach distance 0.**
- mechanism: eight of target's 28 instructions reference the frame — position 3
  (`addiu $sp,-0x10`), 4/5/6 (`sw $s0,0x4` / `$s1,0x8` / `$ra,0xC`), 23/24/25
  (the matching `lw`s) and 26 (`addiu $sp,0x10`). Class A (s2 H1, proven from
  calls.c:1246-1252 + mips.h:1822/1830 + mips.c:4464/4474) makes every one of
  those immediates unreachable, because any function expanding a call carries
  the 16-byte outgoing-argument block. Position 8's `ori $t0,$zero,0x1` is a
  ninth by the class-C assembler proof (s2 H3).
- probe: position accounting against s3's `sidebyside.md`, cross-checked by
  building the normalized-target workspace (`s5/mkws3.py`) — with exactly those
  two classes rewritten to our reachable form, the floor build matches 15 of 28
  positions instead of 9, and the entire prologue/epilogue frame block matches.
- result: honest residual >= 9 (>= 8 discounting the single position the scorer
  masks) for ANY pure-C source under the frozen pipeline, against a floor of 18.
  The maximum a future search could buy is classes B + D, i.e. roughly half the
  residual — and B is now killed from both directions while D is jointly
  unreachable (s3 H4, corroborated by s4 C4 and again by s5's ws4 campaign,
  which never re-entered the sub-308 region from the class-D basin).
- verdict: CONFIRMED

## Live frontier after session 5 (ordered)

**F1 — DISPOSITION, not grinding.** Unchanged in direction, stronger in
evidence. FOUR corpus/mechanism results now say these bytes did not come from
GCC 2.7.2 compiling C: the leaf-shaped frame containing a `jalr` (1/1437 calling
functions), the small-immediate `ori` (3/3 instances in the executable are
hand-written asm), the contiguous length-3 ascending prologue save run (1/1437),
and now the ANTI-PRIORITY register assignment (target gives the 4-ref counter
the LOWER-priority hard register while the 3-ref pointer takes `$s0`, which
`global.c:635` cannot produce from any correct C). Session 5 adds the closing
arithmetic: even a perfect search buys at most classes B+D and can never go
below 9. `scan_hand_coded.py` remains tier=LOW 0/8 and remains blind to all four
anomalies (no frame-geometry, no assembler-macro, no prologue-order, no
allocation-priority signal).
*Next probe:* not a measurement. At `escalation` modality, file the
docs/grind/decisions.md entry weighing the four uniqueness/mechanism results and
the proven >=9 lower bound against the failed `scan_hand_coded` gate. If an
intervening modality is assigned, the ONE untouched evidence class remains
corroborating the crt0 identity from OUTSIDE this executable (a known SN Systems
/ PsyQ crt0 `__main` listing, or the same function in another PsyQ-3.5 title's
asm) — a forensics/rederive task, not a C-form task.

**F2 — residual class B is now CLOSED, not merely low-priority.** s2 killed the
"raise `p`'s refs" direction; s5 killed the "lower `count`'s refs" direction and
showed the only form that flips the roles does so by deleting a semantically
required decrement. The sanctioned duplicated-statement-into-arms ref-lift
(previously F2) is moot: this function has one control-flow arm, the residual
contains four coupled classes, and closing B alone still leaves >= 9 by C5. Do
not open it.

**F3 — do NOT re-run any of these.** Cumulative kill list across s1-s5:
declaration order (s1 K2); the pin/asm form (s1 K1); both statement orders of
the two `la` pairs (s1 C1 + s2 sweep); all 18 structural forms (s2 H2); the
`ori` spelling (s2 H3); frame minimality via simpler C / compiler source /
corpus (s1 (a), s2 H1); the prologue-save-order axis incl. the hoisted-`la` form
(s3 H4); randomized permuter search from the floor chassis and the class-D
chassis against the REAL target (s4 H5/C4); and now randomized search from both
chassis against the NORMALIZED target plus the six counter-ref-split forms
(s5 H6/C5). Do NOT re-propose the `do {...} while (0)` wrap (byte-neutral at
18), the `volatile` dead-local guard (cheat; 29 insns), the address-taken dead
local `s32 *new_var3 = &new_var2` (same cheat family, same 29 insns), or any
`n = count;` copy-split (copy-propagation folds it).

## [s5] H6 — residual class B ($s0/$s1 roles) is reachable by LOWERING count's allocno priority — splitting the counter's references across two pseudos — the mirror image of session 2's killed 'raise p's ref count' attack.
- mechanism: global.c:635 allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length; s2 measured 8 loop-weighted refs for count against 7 for p and killed every route that raises p's count (each costs an instruction the 28-insn target has no room for). The untried mirror is to give the counter's references to two different pseudos. Session 5's isolated-objective permuter campaign surfaced exactly that: tmp/grind/func_80083794/s5/ws4/output-478-1 is the FIRST form in five sessions whose loop reads through $s0 (lw v0,0(s0); beqz s1; bnez s1) — target's register roles.
- probe: (1) disassembled the permuter find through the real pipeline (s5/dis4.sh) and read what it actually does; (2) measured six SEMANTICS-PRESERVING spellings of the same ref-split intent in the honest sandbox (s5/sweep.py + s5/variants.json; raw s5/sweep_out.json), each checked by objdump for the loop's lw base register, the temp register and the frame size — guard-on-count/loop-on-copy with --n; plain copy rename; guard + end-pointer loop; copy after the p assignment; guard + copy + while; guarded pre-decrement countdown.
- result: the permuter find flips class B ONLY because it TESTS new_var while decrementing count, so the decrement is dead code that flow deletes, stripping two in-loop references off the counter; the form is an infinite loop and its emitted body is missing target's addiu $s1,$s1,-0x1 entirely. The six correct forms score 23 / 18 / 22 / 22 / 24 / 18 with p in $s1, the temp in $v0 and the frame at -32 in EVERY one (two merely tie the floor). GCC's copy-propagation folds `n = count;` back into a single pseudo, so a semantics-preserving split is not a split. Banked as memory/grind/func_80083794/rejected/count-ref-split-forms-do-not-flip-s0-s1.c. Sharper characterisation of class B: target references count four times (la, beqz, addiu -1, bnez) and p three times (la, lw base, addiu +4) — the SAME split our build has — yet assigns them the opposite hard registers, i.e. target's assignment is anti-priority under this allocator and is not produced by any correct C source.
- verdict: KILLED

## [s5] C5 — the permuter modality is closed for this function by arithmetic rather than by exhaustion: no pure-C form can score below 9 honest residual instructions, so no campaign of any length can reach distance 0.
- mechanism: eight of target's 28 instructions reference the frame — position 3 (addiu $sp,-0x10), 4/5/6 (sw $s0,0x4 / $s1,0x8 / $ra,0xC), 23/24/25 (the matching lw's) and 26 (addiu $sp,0x10). Class A (s2 H1) proves every one of those immediates unreachable: calls.c:1246-1252 MAXes args_size against reg_parm_stack_space unconditionally for MIPS, mips.h:1822 fixes it at 16, mips.c:4464/4474 lays the saves above it, so any function expanding a call carries the 16-byte block. Position 8's `ori $t0,$zero,0x1` is a ninth unreachable instruction by the class-C assembler proof (s2 H3).
- probe: position accounting against s3's sidebyside.md, cross-checked by building the normalized-target workspace (s5/mkws3.py): with exactly those two classes rewritten to the form our pipeline can emit, the floor build matches 15 of 28 positions instead of 9, and the whole prologue/epilogue frame block matches.
- result: honest residual >= 9 for any pure-C source under the frozen pipeline (>= 8 discounting the single position the scorer masks), against the current floor of 18. The most any future search can buy is classes B + D — and B is now killed from both directions (s2 H2, s5 H6) while D is jointly unreachable (s3 H4, corroborated by s4 C4 and again by s5's ws4 campaign, which never re-entered the sub-308 region from inside the class-D basin).
- verdict: CONFIRMED

## [s5] Residual class B ($s0/$s1 roles) is reachable by LOWERING count's allocno priority — splitting the counter's references across two pseudos — the mirror image of session 2's killed 'raise p's ref count' attack.
- mechanism: global.c:635 allocno_compare ranks by floor_log2(n_refs)*n_refs/live_length; s2 measured 8 loop-weighted refs for count against 7 for p and killed every route that raises p's count. The untried mirror is to give the counter's references to two pseudos so neither outranks the pointer. Session 5's isolated-objective campaign surfaced exactly that shape: tmp/grind/func_80083794/s5/ws4/output-478-1 is the FIRST form in five sessions whose loop reads through $s0 (lw v0,0(s0); beqz s1; bnez s1) — target's register roles.
- probe: (1) Disassembled the permuter find through the real cpp|cc1 -mel|prologue_fix|maspsx|multu_pad|as pipeline (tmp/grind/func_80083794/s5/dis4.sh) and read what it actually computes. (2) Measured six SEMANTICS-PRESERVING spellings of the same ref-split intent in the honest sandbox (tmp/grind/func_80083794/s5/sweep.py + variants.json, raw sweep_out.json), each checked by objdump for the loop's lw base register, the temp register and the frame size: guard-on-count/loop-on-copy with --n; plain copy rename; guard + end-pointer loop; copy placed after the p assignment; guard + copy + while; guarded pre-decrement countdown.
- result: The permuter find flips class B ONLY because it TESTS new_var while decrementing count: the decrement becomes dead code that flow deletes, stripping two in-loop references off the counter and inverting the allocno priority. It is an infinite loop (the permuter does not preserve semantics) and its emitted body is missing target's addiu $s1,$s1,-0x1 entirely. The six correct forms score 23 / 18 / 22 / 22 / 24 / 18 with p in $s1, the temp in $v0 and the frame at -32 in EVERY one; two merely tie the floor, none beats it. GCC's copy-propagation folds `n = count;` back into a single pseudo, so a semantics-preserving split is not a split. Banked as memory/grind/func_80083794/rejected/count-ref-split-forms-do-not-flip-s0-s1.c.
- verdict: KILLED

## [s5] The permuter modality is closed for this function by arithmetic rather than by exhaustion: no pure-C form can score below 9 honest residual instructions, so no campaign of any length can reach distance 0.
- mechanism: Eight of target's 28 instructions reference the frame — position 3 (addiu $sp,-0x10), 4/5/6 (sw $s0,0x4 / $s1,0x8 / $ra,0xC), 23/24/25 (the matching lw's) and 26 (addiu $sp,0x10). Class A (s2 H1) proves every one of those immediates unreachable: calls.c:1246-1252 MAXes args_size against reg_parm_stack_space unconditionally for MIPS (the MAYBE_REG_PARM_STACK_SPACE zeroing escape is dead), mips.h:1822 fixes it at 16, and mips.c:4464/4474 lays the callee-saves above it, so ANY function expanding a call carries the 16-byte outgoing-argument block. Position 8's `ori $t0,$zero,0x1` is a ninth unreachable instruction by the class-C assembler proof (s2 H3: GNU as expands `li 1` to addiu, and all 3 small-immediate ori instances in the executable are hand-written asm).
- probe: Position accounting against session 3's sidebyside.md, cross-checked by building a normalized-target permuter workspace (tmp/grind/func_80083794/s5/mkws3.py): the shipped target with EXACTLY those two classes rewritten to the form our pipeline can emit (addiu $sp,-0x10 -> -0x20; saves 0x4/0x8/0xC -> 0x10/0x14/0x18; ori $t0,$zero,0x1 -> li $t0,1) and nothing else changed.
- result: Against the normalized target the floor build matches 15 of 28 positions instead of 9, and the entire prologue/epilogue frame block matches — confirming the 9 unreachable positions exactly. Honest residual is therefore >= 9 for any pure-C source under the frozen pipeline (>= 8 discounting the single position the scorer masks), against the current floor of 18. The most any future search can buy is classes B + D, and B is now killed from both directions (s2 H2, s5 H6) while D is jointly unreachable (s3 H4, corroborated by s4 C4 and again by s5's ws4 campaign).
- verdict: CONFIRMED

## [s5] With the two proven-unreachable classes removed from the OBJECTIVE (so the randomizer's gradient is no longer dominated by constant noise), a fresh search finds a form that improves classes B and/or D honestly.
- mechanism: Session 4's campaigns scored against the shipped target, where ~9 instructions can never match; that constant penalty dominates the permuter's alignment-based diff and explains why its best finds were frame-junk forms. Removing it should let the real B/D signal drive the search.
- probe: Two campaigns via tools/permuter_campaign.py against the normalized target, both harvested with --stop in-session (status reports alive:false for both PIDs; no campaign outlived the session). ws3 'normalized-target-BD-isolation': base = the score-18 floor form, base_score 308, 95,501 iterations, ~30 min. ws4 'class-D-chassis-vs-normalized-target': base = the s3 hoisted-la form, base_score 970, 63,304 iterations, ~15 min. Every closing proposal disassembled through the real pipeline and checked for frame size, loop lw base register and prologue save order.
- result: No. ws3's best (263) was found at 190 s and never beaten in the following ~27 minutes; it is the s4-340 cheat family re-found under two new spellings — `volatile unsigned short new_var; if (D_800A2668 == (new_var = 0))` and `s32 new_var2; s32 *new_var3 = &new_var2; if (*new_var3 == 0)` — and both measure 29 emitted instructions, frame -40, $s0/$s1 still inverted, save order still descending, i.e. they touch neither class B nor class D. Even with the frame noise removed from the objective, the permuter's alignment diff still rewards adding a frame slot; that is a property of the metric, not a lead. ws4 never re-entered the sub-308 region (base 970, best 478).
- verdict: KILLED
