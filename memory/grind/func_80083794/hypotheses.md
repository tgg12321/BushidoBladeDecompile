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

## Session 6 (forensics, 2026-08-13)

### H7 — "target's unfilled RETURN delay slot (`addiu $sp,0x10; jr $ra; nop`) is a hand-assembly fingerprint" — **KILLED**
Mechanism proposed: reorg.c normally sinks the epilogue stack restore into the
`jr $ra` delay slot, so an unfilled return slot with a legal filler right above
it would be a reorg.c-inconsistent shape.
Probe: `tmp/grind/func_80083794/s6/epilogue_scan.py`, census of all 1434
`asm/funcs/*.s`.
Result: 838 functions have EXACTLY target's shape (restore before `jr`, slot =
`nop`); only 94 sink the restore into the slot. It is the pipeline's norm here.
**Verdict: KILLED — not evidence, do not cite it.**

### H8 — "the crt0/libgcc identity can be corroborated from OUTSIDE the main executable" (frontier F1's named next probe) — **KILLED (probe answered negative)**
Mechanism proposed: `disc/STR/MOVOVL.EXE` is a second, independently linked
PS-EXE built by the same PsyQ toolchain; if it carried the same `__main` /
`__do_global_ctors` object, that would be direct external corroboration that the
bytes come from a prebuilt library object rather than compiled project C.
Probe: `tmp/grind/func_80083794/s6/movovl_scan.py` (opcode-exact word signatures
for the leaf frame and for the ctor-walk loop, plus an ascending-save-run scan),
followed by objdump of the overlay's crt0 region; plus two web searches for a
primary SN Systems / PsyQ crt0 `__main` listing.
Result: MOVOVL.EXE has 0 hits for either signature; its single ascending >=3
`sw`-to-`$sp` run is `sw zero,64/72/80($sp)` inside vsprintf; its entry
(0x801DA084) is `lui gp / addiu gp / j 0x801D91CC` with no ctor machinery. The
same scan on the main EXE hits exactly twice — func_80083794 and its twin. No
primary crt0 listing was found online.
**Verdict: KILLED — external corroboration is measured-unavailable by the only
offline route that existed. Do NOT re-run this probe.**

### H9 — "residual class E: the branch delay slot's PROVENANCE is itself unreachable" — **CONFIRMED**
Mechanism: target's `bnez $t0` delay slot holds `ori $t0,$zero,0x1`, which
clobbers the register the branch tests, so it must have been pulled backwards
from the conditional arm. `reorg.c` can only do that in its fall-through scan
(reorg.c:3075), guarded at reorg.c:3048 by `slots_filled != slots_to_fill` —
reached only if the BACKWARD scan (reorg.c:2960) failed. Target has three
eligible callee-saves immediately ahead of the branch, so the backward scan
cannot fail.
Probe: instrumented cc1 with `BB2_DBR_DEBUG=1`
(`tmp/grind/func_80083794/s6/dbr.sh` -> `dbr_mini.log`) + a corpus census
(`slotscan.py`).
Result: `DBRDBG simp insn=11 trial=73 refset=0 setset=0 setneed=0` / `elig=1` —
the nearest preceding save (`sw $16,16($sp)`) is eligible and IS consumed into
the slot, as the emitted asm shows. Corpus: 39/1434 functions do keep an intact
>=3 save run with a non-save filler, but every one of those fillers is an
ENTRY-BLOCK computation sitting nearest the branch (35x `addiu $s3,$a0,-0x1`),
never an arm insn clobbering the tested register.
**Verdict: CONFIRMED — a fifth no-C-form / corpus-uniqueness result, and one
that is INDEPENDENT of class D (fixing the save order would not stop reorg from
eating the third save).**

## Live frontier after session 6

### F1 — the question remains DISPOSITION, not grinding; the evidence is now FIVE independent mechanism results
(A) a byte-exact cc1 LEAF frame for a body containing a `jalr`, with the 16-byte
outgoing-arg block unconditional (calls.c:1246-1252 + mips.h:1822/1830 +
mips.c:4464/4474; corpus 1/1437); (B) an ANTI-PRIORITY register assignment that
global.c:635 `allocno_compare` cannot produce from any correct C (s5); (C) an
`ori $t0,$zero,1` GNU as cannot emit from `li 1` (corpus 3/3 small-immediate
instances hand-written); (D) a contiguous length-3 ASCENDING prologue save run
that mips.c:4680 emits descending and that only sched2 could reverse, under an
anti-dependence this function's block structure cannot supply (corpus 1/1437,
now confirmed at pass level from the RTL dumps); and NEW (E) a branch delay slot
filled from the conditional arm with an insn clobbering the branch's own test
register, while three eligible saves sit adjacent — impossible under reorg.c's
backward-scan-first order (corpus: 39 intact-run functions, all with entry-block
fillers, none with an arm insn). Against that stands `scan_hand_coded.py`
tier=LOW 0/8 — a scanner with no frame-geometry, no assembler-macro, no
prologue-order, no allocation-priority and no delay-slot-provenance signal, i.e.
blind to all five. s5's arithmetic still caps any future search at >= 9.
**Next probe:** none that is a measurement. When the driver reaches `escalation`
modality, file the docs/grind/decisions.md entry weighing the five mechanism
results and the proven >= 9 lower bound against the failed scan_hand_coded gate.

### F2 — external corroboration is CLOSED (was F1's named next probe)
The second executable does not carry the object (H8), and no primary PsyQ crt0
listing is available. Do not spend another session on it.
**Next probe:** none.

### F3 — cumulative kill list; do NOT re-run any of these
Everything in s1-s5's F3 list, plus: the return-delay-slot-nop signal (H7, it is
the corpus norm at 838/1434) and the MOVOVL/external-corroboration probe (H8).
Also do not re-open the duplicated-statement-into-arms ref-lift (s5 F2), the
do-while(0) wrap (byte-neutral at 18), the volatile / address-taken dead-local
guards (cheats, 29 insns), or any `n = count;` copy split (copy-propagation folds
it).

## [s6] Residual class E: the PROVENANCE of target's branch delay-slot filler is itself unreachable under this pipeline, independently of the save-order class D.
- mechanism: Target position 7-8 is `bnez $t0,.L800837EC` with `ori $t0,$zero,0x1` in the delay slot. That filler writes $t0, the very register the branch tests, so it cannot be an entry-block insn (it would destroy the comparison) — it is the conditional arm's `initialized = 1` materialisation pulled backwards from the fall-through path. In tools/gcc-2.7.2/reorg.c, fill_simple_delay_slots can only do that in its fall-through scan (reorg.c:3075), and reorg.c:3048 guards that scan with `if (slots_filled != slots_to_fill && ...)` — it runs ONLY when the backward scan (reorg.c:2960, 'Now, scan backwards from the insn to search for a potential delay-slot candidate') found nothing. Target has three eligible callee-save stores immediately in front of the branch, so the backward scan cannot fail.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_DBR_DEBUG=1) on the minimal-TU floor form -> tmp/grind/func_80083794/s6/dbr_mini.log; plus a corpus census of intact save runs ahead of an early branch (tmp/grind/func_80083794/s6/slotscan.py over all 1434 asm/funcs/*.s).
- result: The log shows `DBRDBG simp insn=11 trial=73 refset=0 setset=0 setneed=0` then `DBRDBG simp insn=11 trial=73 elig=1`: insn 11 is the flag-test branch, trial 73 is `sw $16,16($sp)`, the nearest preceding prologue save; it is eligible and IS consumed into the slot, exactly as the emitted asm shows (`bne $2,$0,.L2` / delay `sw $16,16($sp)`). Census: 1346 files have no >=3-save run early, 13 fill the branch slot with another save, 36 have no branch after the run, and 39 keep an intact run with a non-save filler — but all 39 fillers are ENTRY-BLOCK computations that sat nearest the branch (35x `addiu $s3,$a0,-0x1` from the incoming $a0, 2x `lh $s7,0x60($s2)`). Spot-checked func_8004C994 (zero regfix/asmfix rules, not in the queue, i.e. matched pure C): 8 saves intact, slot `addiu $s3,$a0,-0x1`. No shipped function except func_80083794 and its twin fills a branch delay slot with an insn that clobbers the branch's own test register while eligible saves sit adjacent.
- verdict: CONFIRMED

## [s6] Target's unfilled RETURN delay slot (`addiu $sp,$sp,0x10; jr $ra; nop`, with the stack restore NOT sunk into the slot) is a hand-assembly fingerprint.
- mechanism: reorg.c would normally sink the epilogue stack restore into the `jr $ra` delay slot (the ubiquitous PS1 epilogue), so an unfilled return slot with a legal filler sitting directly above it would be a reorg.c-inconsistent shape.
- probe: tmp/grind/func_80083794/s6/epilogue_scan.py — census of the return delay slot across all 1434 asm/funcs/*.s.
- result: 838 of 1434 shipped functions have EXACTLY target's shape (restore before `jr $ra`, slot = nop); 288 are leaves with a nop slot; 158 have some other filler; only 94 sink the restore into the slot. The shape is this pipeline's norm, not an anomaly. Banked so no future session mistakes it for evidence.
- verdict: KILLED

## [s6] The crt0/libgcc identity of func_80083794 can be corroborated from OUTSIDE the main executable — specifically from disc/STR/MOVOVL.EXE, a second, independently linked PS-EXE built by the same PsyQ toolchain (this was frontier F1's named next probe).
- mechanism: If the same `__main` / `__do_global_ctors` object were linked into the overlay, its presence in a second executable would be direct evidence that the bytes come from a prebuilt library object rather than from compiled project C.
- probe: tmp/grind/func_80083794/s6/movovl_scan.py (opcode-exact word signatures for the leaf frame `addiu $sp,-0x10 / sw $s0,4 / sw $s1,8 / sw $ra,0xC` and for the ctor-walk loop `lw $t0,0($s0); addiu $s0,$s0,4; jalr $t0; addiu $s1,$s1,-1`, plus an ascending-save-run scan) over both executables, followed by objdump of the overlay's crt0 region (tmp/grind/func_80083794/s6/movovl_head.txt); plus two web searches for a primary SN Systems / PsyQ crt0 `__main` listing.
- result: MOVOVL.EXE: 0 hits for the leaf-frame signature, 0 hits for the ctor-walk loop, and its single contiguous ascending >=3 sw-to-sp run is `sw zero,64/72/80($sp)` inside the overlay's vsprintf, not a prologue; its entry at 0x801DA084 is `lui gp / addiu gp / j 0x801D91CC`, with no ctor machinery anywhere. The identical scan over the main EXE hits exactly twice — func_80083794 and its twin at 0x80083804. The web searches returned no primary crt0 listing (only the secondary observation that crt0 objects are the PsyQ objects that fail PSYQ->ELF conversion). The probe is answered NEGATIVE: external corroboration is unavailable by the only offline route that existed.
- verdict: KILLED

## [s6] Class D (prologue callee-save emission order) is decided by mips.c's save_restore_insns and can only be reversed by the post-reload scheduler — s3 asserted this from compiler source; forensics must confirm it at pass level.
- mechanism: mips.c:4680 is a single `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)` loop serving both the prologue (store_p) and epilogue (!store_p) call sites, so cc1 emits both runs descending; only schedule_insns (sched2) could reorder them, and it does so only under an in-block anti-dependence.
- probe: Full cc1 -da RTL dump of the minimal-TU floor form (tmp/grind/func_80083794/s6/dump.sh -> dump/mini.i.{greg,jump2,sched2,dbr}), inspecting the sp-relative insns per pass.
- result: The three saves are ABSENT from .greg and first appear in .jump2 as RTL insns 69/71/73 in descending offset order (`sw $ra,24 / sw $s1,20 / sw $s0,16`) — i.e. inserted by prologue threading after register allocation. .sched2 leaves the sequence 67-69-71-73 untouched (the post-reload scheduler does not reorder the run absent an anti-dependence). .dbr then pulls insn 73 into the branch delay slot. s3's mechanism claim is confirmed with dump evidence rather than source reading alone.
- verdict: CONFIRMED

## Session 7 (forensics, 2026-08-13)

### H10 — "the five residual classes are decompals-fork-vs-cc1psx DIVERGENCE, not evidence about the target's provenance" — **KILLED**
This is the counter-explanation every prior session's mechanism argument left
formally open, because s1-s6 read `tools/gcc-2.7.2/` — the FORK — for all of
their proofs (calls.c/mips.h/mips.c for A, global.c for B, GNU `as` for C,
mips.c:4680 for D, reorg.c for E). `.claude/rules/difficult-is-not-impossible.md`
mandates settling it empirically with cc1psx before any pessimistic claim, and
six sessions had not.
Probe: `tmp/grind/func_80083794/s7/probe_cc1psx.sh` + `olevels.sh` — compile the
exact score-18 floor body (`s7/mini.i`) with the ORIGINAL PsyQ compiler
(`tools/cc1psx_wrapper.sh` -> `cc1psx.exe`, banner `GNU C 2.7.2.SN.1 [AL 1.1,
MM 40] Sony Playstation`) at the canonical flags, and sweep `-O0..-O3` on both
compilers. DIAGNOSTIC ONLY ([[cc1psx-calibration-only]],
[[no-compiler-divergence]]) — nothing is proposed for the build.
Result: cc1psx reproduces **all five residual classes exactly as our fork does** —
`.frame $sp,32,$31 # args= 16` (class A), `$16`=count / `$17`=p / temp `$2`
(class B), `li $2,0x00000001` (class C), saves DESCENDING `sw $31,24 / sw $17,20
/ sw $16,16` (class D), and the flag-test branch's delay slot filled with the
nearest preceding SAVE, `bne $2,$0,$L2` / `sw $16,16($sp)` (class E). Modulo
label spelling the two compilers emit the same instruction sequence. `args= 16`
holds at every optimization level on both compilers.
**Verdict: KILLED — the residual is not fork divergence. The compiler that
shipped this game produces our code, not target's, and does so from the very C
this ledger already carries.**

### H11 — "class C's `ori $t0,$zero,1` is ASPSX 2.34's expansion of `li 1`, i.e. ordinary toolchain output rather than hand-written asm" — **KILLED**
s2 measured GNU `as`; the original assembler was ASPSX 2.34 (maspsx emulates it),
and cc1psx's output confirms the compiler emits `li $2,0x00000001`, so the whole
question reduces to what ASPSX did with a small-immediate `li`.
Probe: `tmp/grind/func_80083794/s7/li_form_census.py` — census both expansion
forms across all 1434 shipped `asm/funcs/*.s`.
Result: **5537** `addiu $rX,$zero,imm` with imm < 0x8000 against **3**
`ori $rX,$zero,imm` with imm < 0x8000. ASPSX expanded small-immediate `li` to
`addiu` 5537 times out of 5540. The three exceptions are `func_80052788` and
`func_800527FC` (both `ori $rX,$zero,0x1000`, both `INCLUDE_ASM` GTE bodies whose
neighbouring lines carry `/* handwritten instruction */` annotations) and
func_80083794's own `ori $t0,$zero,0x1`.
**Verdict: KILLED — the assembler explanation is dead at 5537:3, and small-
immediate `ori` remains a 3-for-3 hand-written-assembly fingerprint.**

### H12 — "class E can be tested WITHIN the target region itself, without any corpus census" — **CONFIRMED**
Mechanism: `asm/funcs/func_80083794.s` lines 32-59 hold the unlabelled twin at
0x80083804 — the identical routine with the flag test inverted and no
`initialized = 1` store. Its branch `beqz $t0,.L80083854` (0x8008381C) has the
SAME three callee-saves immediately ahead of it as the scored body, and its delay
slot is `nop`. reorg.c's `fill_simple_delay_slots` backward scan (reorg.c:2960)
is per-function-stateless, so if it can leave that slot empty with three eligible
saves adjacent, it must leave the scored body's slot empty too — and if it fills
one it fills both.
Probe: `tmp/grind/func_80083794/s7/twin.sh` on `s7/twin.i` (the minimal TU with
`if (D_800A2668 != 0)` and no flag store) through our fork AND cc1psx.
Result: BOTH compilers emit `beq $2,$0,.L2` with `sw $16,16($sp)` in the slot.
Target fills the scored body's slot with an insn pulled out of the conditional
arm that clobbers the branch's own test register, and leaves the twin's slot
empty. No compiler in reach produces either half of that pair.
**Verdict: CONFIRMED — a within-region control experiment, independent of the
39-function corpus census s6 used, and the sixth independent result.**

## Live frontier after session 7

### F1 — the question is DISPOSITION, not grinding; the evidence is now SIX independent results and the fork-divergence counter-explanation is closed
(A) a byte-exact cc1 LEAF frame for a body containing a `jalr`, with the 16-byte
outgoing-arg block unconditional (calls.c:1246-1252 + mips.h:1822/1830 +
mips.c:4464/4474; corpus 1/1437); (B) an ANTI-PRIORITY register assignment
global.c:635 `allocno_compare` cannot produce from any correct C (s5); (C) an
`ori $t0,$zero,1` neither GNU `as` nor ASPSX 2.34 emits from `li 1` (corpus
5537:3, and 3/3 small-immediate instances hand-written); (D) a contiguous
length-3 ASCENDING prologue save run that `save_restore_insns` emits descending
and sched2 leaves alone (corpus 1/1437, confirmed at pass level from the RTL
dumps); (E) a branch delay slot filled from the conditional arm by an insn
clobbering the branch's own test register while three eligible saves sit adjacent
(reorg.c backward-scan-first; corpus 39/39 intact-run functions use entry-block
fillers); and NEW (F) the same-region twin control experiment (H12) plus, cutting
across all of them, the **cc1psx differential (H10)**: the ORIGINAL PsyQ compiler,
at every optimization level, produces our code and not target's. Against that
still stands only `scan_hand_coded.py` tier=LOW 0/8 — a scanner with no
frame-geometry, no assembler-macro, no prologue-order, no allocation-priority and
no delay-slot-provenance signal, i.e. blind to every anomaly this function
exhibits. s5's arithmetic still caps any future search at >= 9 honest residual.
**Next probe:** none that is a measurement. When the driver reaches `escalation`
modality, file the `docs/grind/decisions.md` entry weighing the six mechanism/
corpus results plus the cc1psx differential against the failed
`scan_hand_coded` gate, and return `owner-gated` citing it. Per the owner's
standing auto-ruling (2026-07-27), both endgame gates fail (scan tier is LOW, not
STRONG; there is no SOTN-master precedent to cite for any coercion family that
would close this), so that entry is terminal-by-standing-ruling, not a pending
packet.

### F2 — external corroboration remains CLOSED (s6 H8)
The second executable does not carry the object and no primary PsyQ crt0 listing
is available. Do not spend another session on it. Session 7 adds the sharper
point that external corroboration is no longer NEEDED for the provenance
question: the cc1psx differential answers it from inside the repo.

### F3 — cumulative kill list; do NOT re-run any of these
Everything in s1-s6's F3 list, plus s7's three: do NOT re-argue fork divergence
(H10, measured negative on both compilers at four optimization levels), do NOT
re-open the ASPSX `li`-expansion explanation for the `ori` (H11, 5537:3), and do
NOT re-derive the class-E delay-slot argument from the corpus alone — the twin
control experiment (H12) is the cheaper and stronger form of it. Also unchanged:
declaration order, the pin/asm form, both `la` statement orders, the 18
structural forms, the `ori` spelling, frame minimality by three routes, the whole
prologue-save-order axis incl. the hoisted-`la` form, randomized permuter search
from four chassis against both the real and the normalized target, the six
counter-ref-split forms, the return-delay-slot-nop signal (corpus norm at
838/1434), the MOVOVL probe, the duplicated-statement-into-arms ref-lift, the
do-while(0) wrap (byte-neutral at 18), and the volatile / address-taken
dead-local guards (cheats, 29 insns).

## [s7] The five residual classes are decompals-fork-vs-cc1psx DIVERGENCE rather than evidence about the target's provenance.
- mechanism: Sessions 1-6 derived every class from tools/gcc-2.7.2/ — the decompals FORK (calls.c:1246-1252 + mips.h:1822/1830 + mips.c:4464/4474 for class A; global.c:635 for B; GNU as for C; mips.c:4680 for D; reorg.c:2960/3048/3075 for E). That leaves the counter-explanation formally open: a skeptic can say the ORIGINAL compiler would have produced target's shape from the same C. .claude/rules/difficult-is-not-impossible.md mandates settling exactly that empirically with cc1psx before any pessimistic claim, and no prior session had run it.
- probe: tmp/grind/func_80083794/s7/probe_cc1psx.sh and olevels.sh. Fed the exact score-18 floor body (s7/mini.i, s6's minimal TU verified to compile to the same 28 instructions as the full-TU sandbox object) to tools/cc1psx_wrapper.sh -> tools/cc1psx.exe, the ORIGINAL PsyQ cc1 (banner: 'GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation'), at the canonical CC_FLAGS minus the GNU-only tokens it does not accept (-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w), and swept -O0/-O1/-O2/-O3 on BOTH compilers. Diagnostic only per .claude/rules/cc1psx-calibration-only.md + no-compiler-divergence.md; no build change is proposed.
- result: cc1psx reproduces ALL FIVE residual classes exactly as our fork does. Class A: `.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16, extra= 0` (target is a 0x10 leaf frame with args=0). Class B: `la $16,D_00000000` / `la $17,D_8008D070` / `lw $2,0($17)` — $s0=count, $s1=p, temp $v0, the same inversion (target has $s0=p, $s1=count, temp $t0). Class C: `li $2,0x00000001`, not `ori`. Class D: `sw $31,24($sp) / sw $17,20($sp) / sw $16,16($sp)` — descending (target ascending). Class E: `bne $2,$0,$L2` with `sw $16,16($sp)` in the delay slot — the nearest preceding save, not an arm insn. Modulo label spelling ($L2 vs .L2) and the directive preamble, the two compilers emit the same instruction sequence. The -O sweep: `args= 16` at EVERY level on BOTH compilers (-O0 gives .frame $fp,40 vars= 8 args= 16; -O1/-O2/-O3 give .frame $sp,32 vars= 0 args= 16), with cc1psx descending at every optimizing level.
- verdict: KILLED

## [s7] Class C's `ori $t0,$zero,1` is ASPSX 2.34's expansion of `li 1` — i.e. ordinary toolchain output rather than a hand-written-assembly fingerprint.
- mechanism: s2 measured GNU as (li 1 -> addiu), but the original toolchain assembled with ASPSX 2.34, which maspsx only emulates. cc1psx's output confirms the ORIGINAL compiler also emits `li $2,0x00000001` for this store, so the entire question reduces to which expansion ASPSX chose for a small non-negative immediate. cc1/cc1psx emit `li` for every SImode constant load, so the shipped executable is a complete census of ASPSX's choice.
- probe: tmp/grind/func_80083794/s7/li_form_census.py — count `addiu $rX,$zero,imm` and `ori $rX,$zero,imm` by immediate magnitude across all 1434 asm/funcs/*.s.
- result: 5537 `addiu $rX,$zero,imm` with imm < 0x8000 against 3 `ori $rX,$zero,imm` with imm < 0x8000 (plus 97 ori with imm >= 0x8000, the assembler-forced zero-extension range s2 already characterised, and 741 addiu with other immediates). ASPSX 2.34 expanded small-immediate `li` to `addiu` 5537 times out of 5540. The three exceptions are func_80052788 and func_800527FC (both `ori $rX,$zero,0x1000`, both INCLUDE_ASM GTE bodies whose neighbouring lines are annotated /* handwritten instruction */) and func_80083794's own `ori $t0,$zero,0x1`.
- verdict: KILLED

## [s7] Class E (delay-slot provenance) can be tested WITHIN the target region itself, without relying on a corpus census.
- mechanism: asm/funcs/func_80083794.s lines 32-59 hold the unlabelled twin at 0x80083804 — the identical routine with the flag test inverted and no `initialized = 1` store. Its branch `beqz $t0,.L80083854` (0x8008381C) sits behind the SAME three callee-save stores (sw $s0,0x4 / sw $s1,0x8 / sw $ra,0xC) as the scored body, and its delay slot is `nop`. reorg.c's fill_simple_delay_slots carries no state across functions, so a pass that leaves the twin's slot empty with three eligible saves adjacent must leave the scored body's slot empty too; a pass that fills one fills both. The shipped code does neither consistently: it fills the scored body's slot with an insn pulled out of the conditional arm that clobbers the branch's own test register, and leaves the twin's slot empty.
- probe: tmp/grind/func_80083794/s7/twin.sh on s7/twin.i — the same minimal TU with `if (D_800A2668 != 0)` and no flag store — compiled through BOTH our fork (tools/gcc-2.7.2/build/cc1, canonical CC_FLAGS) and cc1psx.
- result: Both compilers emit `beq $2,$0,.L2` with `sw $16,16($sp)` in the delay slot, i.e. both steal the nearest preceding save exactly as they do for the scored body. Neither leaves the nop the target has. Two instances of one branch shape, 0x70 bytes apart in the shipped code, with identical eligible fillers and opposite outcomes — one filled from the conditional arm, one empty.
- verdict: CONFIRMED

## [s7] The five residual classes are decompals-fork-vs-cc1psx DIVERGENCE rather than evidence about the target's provenance.
- mechanism: Sessions 1-6 derived every class from tools/gcc-2.7.2/ - the decompals FORK (calls.c:1246-1252 + mips.h:1822/1830 + mips.c:4464/4474 for class A; global.c:635 for B; GNU as for C; mips.c:4680 for D; reorg.c:2960/3048/3075 for E). That leaves the counter-explanation formally open: a skeptic can say the ORIGINAL compiler would have produced target's shape from the same C. .claude/rules/difficult-is-not-impossible.md mandates settling exactly that empirically with cc1psx before any pessimistic claim, and no prior session had run it.
- probe: tmp/grind/func_80083794/s7/probe_cc1psx.sh + olevels.sh: fed the exact score-18 floor body (s7/mini.i) to tools/cc1psx_wrapper.sh -> tools/cc1psx.exe, the ORIGINAL PsyQ cc1 (banner 'GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation'), at the canonical CC_FLAGS minus the GNU-only tokens it does not accept (-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w), then swept -O0/-O1/-O2/-O3 on BOTH compilers. Diagnostic only per .claude/rules/cc1psx-calibration-only.md and no-compiler-divergence.md; no build change proposed.
- result: cc1psx reproduces ALL FIVE classes exactly as our fork does. A: '.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16, extra= 0' against target's 0x10 leaf frame with args=0. B: la $16,D_00000000 / la $17,D_8008D070 / lw $2,0($17) - $s0=count, $s1=p, temp $v0, the same inversion (target: $s0=p, $s1=count, temp $t0). C: 'li $2,0x00000001', not ori. D: sw $31,24 / sw $17,20 / sw $16,16 - descending (target ascending). E: 'bne $2,$0,$L2' with 'sw $16,16($sp)' in the delay slot - the nearest preceding save, not an arm insn. Modulo label spelling the two compilers emit the same instruction sequence. -O sweep: args= 16 at EVERY level on BOTH compilers (-O0: .frame $fp,40 vars= 8 args= 16; -O1/-O2/-O3: .frame $sp,32 vars= 0 args= 16), cc1psx descending at every optimizing level.
- verdict: KILLED

## [s7] Class C's ori $t0,$zero,1 is ASPSX 2.34's expansion of `li 1` - ordinary toolchain output rather than a hand-written-assembly fingerprint.
- mechanism: s2 measured GNU as (li 1 -> addiu), but the original toolchain assembled with ASPSX 2.34, which maspsx only emulates. cc1psx confirms the ORIGINAL compiler also emits `li $2,0x00000001` for this store, so the whole question reduces to which expansion ASPSX chose for a small non-negative immediate. cc1/cc1psx emit `li` for every SImode constant load, so the shipped executable is a complete census of that choice.
- probe: tmp/grind/func_80083794/s7/li_form_census.py - count `addiu $rX,$zero,imm` and `ori $rX,$zero,imm` by immediate magnitude across all 1434 asm/funcs/*.s.
- result: 5537 addiu with imm < 0x8000 against 3 ori with imm < 0x8000 (plus 97 ori with imm >= 0x8000, the assembler-forced zero-extension range, and 741 addiu with other immediates). ASPSX expanded small-immediate li to addiu 5537 times out of 5540. The three exceptions are func_80052788 and func_800527FC (both ori $rX,$zero,0x1000, both INCLUDE_ASM GTE bodies with neighbouring /* handwritten instruction */ annotations) and func_80083794's own ori $t0,$zero,0x1.
- verdict: KILLED

## [s7] Class E (delay-slot provenance) can be tested WITHIN the target region itself, without relying on a corpus census.
- mechanism: asm/funcs/func_80083794.s lines 32-59 hold the unlabelled twin at 0x80083804 - the identical routine with the flag test inverted and no `initialized = 1` store. Its branch beqz $t0,.L80083854 (0x8008381C) sits behind the SAME three callee-save stores as the scored body and its delay slot is nop. reorg.c's fill_simple_delay_slots carries no state across functions, so a pass that leaves the twin's slot empty with three eligible saves adjacent must leave the scored body's slot empty too, and a pass that fills one fills both.
- probe: tmp/grind/func_80083794/s7/twin.sh on s7/twin.i (the minimal TU with `if (D_800A2668 != 0)` and no flag store), compiled through BOTH our fork (tools/gcc-2.7.2/build/cc1, canonical CC_FLAGS) and cc1psx.
- result: Both compilers emit 'beq $2,$0,.L2' with 'sw $16,16($sp)' in the delay slot - both steal the nearest preceding save exactly as they do for the scored body, and neither leaves the nop the target has. Two instances of one branch shape, 0x70 bytes apart in shipped code, identical eligible fillers, opposite outcomes: one filled from the conditional arm with an insn clobbering the branch's own test register, one empty.
- verdict: CONFIRMED


## Session 8 (rederive, 2026-08-13)

### H13 -- "a fresh m2c decompile or a corpus sibling/transplant yields a structurally DIFFERENT C shape for func_80083794" (the mandated rederive modality) -- **KILLED**
Probe: (a) `tools/m2c/m2c.py --target mipsel-gcc-c -f func_80083794
asm/funcs/func_80083794.s`; (b) a name+shape sweep for a libgcc/crt0 ctor-walk
sibling across the 3754-scratch decomp.me corpus.
Result: (a) m2c reconstructs the existing candidate body verbatim -- the 28
target instructions admit exactly one dataflow, so there is no alternative shape
to try; (b) 40 name/shape candidates, none of them a ctor-table walk, and no
matched `__main` anywhere in the corpus -- there is no transplant source.
Artifacts: s8/m2c_rederive.txt, s8/frame_probe.txt.

### H14 -- "residual class A (the 16-byte o32 outgoing-argument block) is a property of OUR fork's mips backend rather than of the toolchain family" -- **KILLED (class A corroborated externally)**
Probe: over the 1740 `is_matching` gcc2.7.2/psyq3.5 decomp.me scratches, take
every target that contains a `jal`/`jalr` and has callee-save stores (n=1246)
and histogram the LOWEST callee-save offset.
Result: 1019 at exactly +16, 108 at +24, 37 at +32, 12 at +40, 11 at +48, ...
and ZERO below +16. Every matched calling function in an independent corpus of
already-solved-from-C functions keeps the bottom 16 bytes clear. Target stores
$s0 at +4. Artifact: s8/frame_probe.txt.

### H15 -- "class C's `ori $t0,$zero,1` is a hand-written-assembly fingerprint (s7's reading of the 5537:3 census)" -- **KILLED as an inference; the reachability measurement stands**
Probe: count small-immediate `ori $rX,$zero,imm` in matching corpus scratches,
classify each such function's `la` flavour, and trace the ori/addiu decision
through our own toolchain source.
Result: 591 matched scratches contain the shape; 84 of them are in functions
whose `la`s are entirely addiu-flavoured like BB2's, so the mix target shows is
attested. cc1 never emits ori for a constant (mips.md:1899-1908 `iorsi3` needs a
register operand; mips.c `mips_move_1word` emits `li\t%0,%X1`); our maspsx's
`expand_load_immediate` maps `0 < imm <= 0x7FFF` to addiu and only 0x8000-0xFFFF
to ori, with the in-source comment "ori is actually addiu on ASPSX 2.56+". So
the spelling is an ASPSX-VERSION property. Class C remains UNREACHABLE for us
(no C form changes it, floor arithmetic untouched), but its evidentiary meaning
changes from "hand-written" to "assembled by a different assembler mode than the
other 5537 constant-load sites in this executable" -- a prebuilt/foreign-object
fingerprint. Artifacts: s8/ori_probe.txt, s8/ori_style_probe.txt.

### H16 -- "class E as stated by s6 is too strong: a delay slot that clobbers its own branch's test register is exotic" -- **KILLED, and class E is sharpened and corroborated**
Probe: count the shape in the matched corpus, then re-count restricted to BB2's
actual configuration (a run of adjacent preceding callee-save stores available
to reorg.c's backward scan).
Result: the general shape is ORDINARY -- 484 of 1740 matching scratches have it.
But of those 484, 480 have ZERO adjacent preceding callee-save stores, 4 have
exactly one, and NONE has two or more. func_80083794 has three. The narrow claim
(reorg.c reached the fall-through scan at 3048/3075 despite eligible backward
candidates at 2960) survives with 0 counterexamples in an independent corpus.
Artifact: s8/classE_probe.txt.

### C6 -- class D is C-reachable, as s3 measured -- **CONFIRMED externally**
33 matching scratches emit a >=3-deep ascending callee-save run (vs 143
descending, 331 mixed). Ascending IS producible from C; s3's JOINT
unreachability (the ascending basin costs +5 from the floor form) is the
operative constraint, not the emission order by itself.
Artifact: s8/corpus_census.txt.

## Live frontier after session 8

### F1 -- the question is DISPOSITION, not grinding; SEVEN independent results, two of them external control corpora, and a single unifying explanation
Sessions 1-7 argued five residual classes from the decompals fork's own source
plus BB2-internal censuses, then closed the fork-divergence counter-explanation
with cc1psx. Session 8 adds the first evidence from OUTSIDE this repo that is
also outside this binary: 1740 already-matched gcc2.7.2/psyq3.5 functions on
decomp.me, giving 0/1246 counterexamples for class A and 0/484 for class E in
BB2's exact configuration -- and it replaces the weakest link in the chain
(class C read as handwriting) with a stronger, testable one (class C as an
ASPSX-version/foreign-object fingerprint). All five classes now collapse into ONE
claim: **func_80083794's bytes were linked in from a prebuilt PsyQ/SN object
(crt0/libgcc `__main`), not compiled from this project's C.**
Next probe: not a measurement. When the driver assigns `escalation` modality,
file the docs/grind/decisions.md entry on the seven results (five mechanism
arguments, the cc1psx differential, the two external corpus censuses) against
the failed `scan_hand_coded` gate (tier LOW 0/8), and return owner-gated citing
it. Note for whoever writes it: the prebuilt-object reading is NOT the same
claim as "hand-written asm", so the canonical-asm gate does not get easier --
it is the `no C in this build produced these bytes` claim, which is what the
entry should actually say.

### F2 -- external corroboration of the crt0 identity: PARTIALLY REOPENED and then answered
s6 closed the OFFLINE route (MOVOVL.EXE has no __main) and s7 made it
unnecessary. s8 shows a third route existed all along and is now spent: the
decomp.me corpus contains no matched `__main`/ctor-walk from any PS1 project, so
there is no external C to transplant and no external byte-level twin to diff.
Next probe: none. Do not re-open.

### F3 -- cumulative kill list; every item is answered with a measurement
s1-s7's kill list, plus s8's: do NOT re-run m2c on this function (it returns the
candidate body verbatim), do NOT hunt for a `__main` sibling/transplant (the
decomp.me corpus has none), do NOT re-argue class A or class E from BB2's binary
alone (an independent 1740-function matched corpus now says 0/1246 and 0/484),
and do NOT repeat s7's "the ori is hand-written" inference -- the corpus shows
591 matched instances of that spelling and our own maspsx source shows it is an
ASPSX-version property, not a C-level or handwriting one.

## [s8] A fresh m2c decompile or a corpus sibling/transplant yields a structurally DIFFERENT C shape for func_80083794 (the mandated rederive modality).
- mechanism: m2c reconstructs the original dataflow from the target bytes rather than from our C, so it can expose a shape our hand-derivations missed; and a libgcc/crt0 __main matched by another PS1 project would be a direct transplant source.
- probe: tools/m2c/m2c.py --target mipsel-gcc-c -f func_80083794 asm/funcs/func_80083794.s (s8/m2c_rederive.txt); plus a name+shape sweep for __main / __do_global_ctors / crt0 / ctor-walk (jalr with addiu $sN,$sN,0x4) over all 3754 decomp.me scratches (s8/frame_probe.txt).
- result: m2c returns the existing candidate body verbatim - same guard, same flag store, same walking pointer with descending counter, same do/while under an if (count != 0) pre-test; the only deltas are byte-unit pointer arithmetic and reading the link-time-absolute count as literal 0. The 28 target instructions admit exactly one dataflow. The sibling sweep returns 40 candidates by name or shape, none of them a ctor-table walk, and no matched __main anywhere in the corpus.
- verdict: KILLED

## [s8] Residual class A (the mandatory 16-byte o32 outgoing-argument block) is a property of OUR decompals fork's mips backend rather than of the GCC-2.7.2/PsyQ toolchain family.
- mechanism: If the block were fork-specific, some function matched from real C by another project under the same compiler class would show a callee-save stored inside the bottom 16 bytes of its frame - which is what target does ($s0 at +4 of a 0x10 frame).
- probe: Over the 1740 is_matching gcc2.7.2/psyq3.5 decomp.me scratches, take every target containing jal/jalr with callee-save stores (n=1246) and histogram the lowest callee-save offset (tmp/grind/func_80083794/s8/frame_and_sibling_probe.py -> s8/frame_probe.txt).
- result: 1019 at exactly +16, 108 at +24, 37 at +32, 12 at +40, 11 at +48, 9 at +56, ... and ZERO below +16. Not one matched calling function in an independent corpus of already-solved-from-C functions stores a callee-save inside the argument block. Class A is corroborated from outside BB2's binary.
- verdict: KILLED

## [s8] Class C's ori $t0,$zero,1 is a hand-written-assembly fingerprint (session 7's reading of the 5537-to-3 BB2 census).
- mechanism: If the ori spelling were exclusive to hand-written asm, it should be rare-to-absent in functions other people have MATCHED from real C under this toolchain family; if instead it is what an older ASPSX mode emits for an ordinary li, it is a toolchain-version fingerprint and says nothing about handwriting.
- probe: Count small-immediate ori $rX,$zero,imm across matching corpus scratches and classify each such function's la flavour (s8/ori_probe.txt, s8/ori_style_probe.txt); then trace the ori/addiu decision through our own frozen toolchain source (mips.md iorsi3 at 1899-1908, mips.c mips_move_1word, tools/maspsx/maspsx/__init__.py expand_load_immediate at 213-241).
- result: 591 matched scratches contain the shape, 84 of them inside functions whose la's are entirely addiu-flavoured exactly like BB2's (e.g. co4Jn/func_80089174: addiu $s0,$s0,%lo(D_800B2384) plus jal ... / ori $a0,$zero,0x1 in the delay slot, from the plain C argument func_8009CF78(1, ...)). cc1 never emits ori for a constant load - iorsi3 needs a register operand and mips_move_1word emits li %0,%X1 - and maspsx maps 0<imm<=0x7FFF to addiu, 0x8000-0xFFFF to ori, with the in-source comment 'ori is actually addiu on ASPSX 2.56+'. The spelling is an ASPSX-VERSION property. Class C stays UNREACHABLE under our frozen pipeline (floor arithmetic untouched), but its meaning changes from 'hand-written' to 'assembled by a different assembler mode than the other 5537 constant-load sites in this executable'.
- verdict: KILLED

## [s8] Class E as session 6 stated it is too strong - a branch delay slot holding an insn that clobbers the branch's own test register is exotic.
- mechanism: reorg.c fills from the backward scan (2960) first and only falls through to the arm scan (3048/3075) when the backward scan finds nothing; the diagnostic configuration is therefore 'arm filler chosen DESPITE adjacent eligible callee-saves', not the clobbering filler by itself.
- probe: Count the general shape across the 1740 matching scratches, then re-count restricted to BB2's configuration (>= 2 adjacent preceding callee-save stores) - tmp/grind/func_80083794/s8/classE_probe.py -> s8/classE_probe.txt.
- result: The general shape is ORDINARY: 484 of 1740. But of those 484, 480 have ZERO adjacent preceding callee-save stores, 4 have exactly one, and NONE has two or more. func_80083794 has three (sw $s0,0x4 / sw $s1,0x8 / sw $ra,0xC). The loose version of the claim is dead; the narrow version survives with 0 counterexamples in an independent corpus.
- verdict: KILLED

## [s8] Class D (a >=3-deep ascending prologue callee-save run) is C-reachable under this toolchain family, as session 3 measured in isolation.
- mechanism: mips.c:4680's single GP_REG_LAST->FIRST loop emits both prologue and epilogue descending, so only the post-reload scheduler can reverse a run - which it does under an in-block anti-dependence; if that path is real, other matched projects should exhibit ascending runs.
- probe: Classify the prologue save-offset order of every matching corpus scratch with >=3 early callee-save stores (s8/corpus_census.txt).
- result: 33 ascending vs 143 descending vs 331 mixed. Ascending is genuinely producible from C, confirming s3's hoisted-la measurement. The operative constraint for this function therefore remains s3's JOINT unreachability (the ascending basin costs +5: 18 -> 23), not the emission order by itself.
- verdict: CONFIRMED

## Session 9 (escalation) — the frontier after disposition

H-S9-1 (CONFIRMED): the honest pure-C floor is 18 and the byte-match on main is held by
9 regfix rules + a cheat-asm body. Probe: two `sandbox --disable all` runs this session, one
against HEAD's committed pin form (23, build_insns 16) and one against candidate.c applied
verbatim (18, build_insns 28 == target_insns 28); `grep -c` over regfix.txt/asmfix.txt gives
9/0. Nothing about the floor moved in nine sessions.

H-S9-2 (KILLED — gate 1): "the function qualifies for canonical-asm authorization on
hand-coded signals." `scan_hand_coded.py --single` returns tier=LOW score=0/8 with every
signal negative. The provenance argument (prebuilt PsyQ/SN object) is orthogonal to what the
scanner measures — the scanner looks for HAND-WRITTEN asm idioms, and a compiled-by-a-
different-toolchain library object has none of them. Gate closed.

H-S9-3 (KILLED — gate 2): "some sanctioned coercion family could close the residual if we
found the right precedent." No family is applicable in principle: the dominant residual is
that our frame is 16 bytes LARGER than target's (mandatory o32 outgoing-args block for a
call-expanding function), and every family on the frozen SOTN list adds frame/instructions
rather than removing them. There is therefore no closing construct to cite a precedent for,
and no file:line citation is possible. Gate closed.

H-S9-4 (CONFIRMED, new): func_80083794 is byte-contiguous with `_start` (which ends at
0x80083790 with `break 0,1`) and `_start` is already carried as `INCLUDE_ASM("asm/funcs",
_start)` in src/ings2.c. The two functions are adjacent members of the same linked crt0
region. Corroborates the prebuilt-object conclusion at essentially zero cost; does not open
either gate.

### Live frontier after session 9 (for the record — the function is PARKED, terminal)
F1 — Nothing is pending on the owner. The disposition entry in docs/grind/decisions.md is
terminal under the 2026-07-27 standing ruling. Do NOT re-open this function to grind another
variant: the floor is arithmetically bounded below by 9 and empirically flat at 18 across
five modalities and 265,869 permuter iterations, and the residual is attributed to a
prebuilt object rather than to any C we can write.

F2 — The ONLY thing that could ever change the disposition is an owner POLICY decision, not
a grind result: whether the crt0 region containing `_start` + func_80083794 should be routed
the way `_start` already is (INCLUDE_ASM / canonical). That is explicitly the owner's call
and nothing in the pipeline waits on it. A future session must not self-authorize it.

F3 — All prior kill lists (sessions 1-8, F3 in the s8 ledger) remain in force verbatim. In
addition: do NOT re-run scan_hand_coded on this function (LOW 0/8, banked as an artifact),
and do NOT re-derive the frame-direction argument in gate 2 — it is a one-line consequence
of calls.c:1246-1252 plus target's 16-byte frame and is written down above.

## [s9] The honest pure-C floor is still 18, and the byte-match on main is held by 9 regfix rules plus the committed cheat-asm body.
- mechanism: sandbox --disable all strips regfix rules and cheat-asm, so it reports the honest distance for whatever C is in src/ings2.c; HEAD still carries the session-0 register-pin + hardcoded-$17 __asm__ form, while candidate.c carries the honest floor form.
- probe: Two sandbox runs this session: HEAD body -> score 23, build_insns 16, target_insns 28, rules_dropped 9, cheat_asm_stripped 18. candidate.c applied verbatim to src/ings2.c -> score 18, build_insns 28, target_insns 28. Then `grep -c func_80083794 regfix.txt asmfix.txt` -> 9 and 0. src/ings2.c reverted with git checkout -- afterwards; tree clean.
- result: floor 18 for the ninth consecutive session; 9 regfix rules, 0 asmfix rules
- verdict: CONFIRMED

## [s9] GATE 1 - the function qualifies for canonical-asm authorization on hand-coded signals.
- mechanism: tools/scan_hand_coded.py scores eight hand-written-asm idioms (multu pacing, empty-body branches, spill absence, front-loaded loads, sibling clustering, BIOS jumptable, unsaved $sN, redundant mask-before-shift); STRONG tier requires S1/S2/S6.
- probe: python3 tools/scan_hand_coded.py --single func_80083794 (artifact tmp/grind/func_80083794/s9/scan_hand_coded.txt)
- result: HAND_CODED: tier=LOW score=0/8 (54 insns) - all eight signals negative, including all three that carry weight. The scanner looks for hand-written idioms; a library object compiled by a different toolchain has none, so the provenance argument cannot open this gate.
- verdict: KILLED

## [s9] GATE 2 - some sanctioned SOTN family could close the residual if the right precedent were cited.
- mechanism: The dominant residual (class A) is a frame-size difference in the wrong direction: cc1 emits the mandatory 16-byte o32 outgoing-argument block for any function that expands a call (calls.c:1246-1252 + mips.h:1822/1830 + mips.c:4464/4474, MAYBE_REG_PARM_STACK_SPACE undefined for MIPS), while target's whole frame is 16 bytes already holding 12 bytes of callee-saves.
- probe: Enumerate the frozen sanctioned-family list (.claude/rules/no-new-park-categories.md:164-208) against the required effect: written-never-read local array, constant-holder/dead scalar locals, dead stores/self-assigns, C-level pointer aliases, duplicated statement into arms, do-while(0) wrap, sub-word param reads, mixed exit forms, named-intermediate declaration order.
- result: Every family can only ADD frame bytes or ADD instructions; none can REMOVE a compiler-mandated argument area. There is therefore no candidate closing construct, hence no file:line precedent to cite. Gate fails not for want of evidence but for want of an applicable construct.
- verdict: KILLED

## [s9] func_80083794 is an adjacent member of the same linked crt0 region as _start, which the project already carries as asm.
- mechanism: If these bytes came from a prebuilt PsyQ/SN crt0+libgcc object (the conclusion sessions 6-8 converged on), the object's other functions should be contiguous in .text and should show the same not-from-our-C character; _start is the canonical crt0 entry stub.
- probe: Read asm/funcs/_start.s bounds and the src/ings2.c carrier lines: _start spans 0x800836EC-0x80083790 and ends `jal main; nop; break 0,1`; func_80083794 starts at the very next word; src/ings2.c:610 is INCLUDE_ASM("asm/funcs", _start).
- result: Byte-contiguous, and the immediately preceding function of the same region is already routed as asm rather than pure C. Direct provenance corroboration at near-zero cost; it is not a scan_hand_coded signal and not a SOTN precedent, so it does not open either gate.
- verdict: CONFIRMED
