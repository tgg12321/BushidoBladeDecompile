# Hypothesis ledger - func_80070C70

Floor history: 194 (HEAD, no C body) -> 108 (de-cheated retired chassis + array declarations)
-> 101 (IconC70 sized 0x20). All measured in s1, 2026-09-10, `sandbox --disable all`.

## CONFIRMED

### H1 - D_800A3560 and D_800A3590 are arrays, not scalars (declaration fix)
**Mechanism:** the target's own relocations. `lui %hi(SYM); addu $at,$at,<scaled index>;
lbu/lh %lo(SYM)($at)` is the fused array-access form GCC 2.7.2 emits for `extern T SYM[]`;
it cannot be produced from a scalar declaration without a pointer pun (banned).
**Probe:** changed both declarations in src/text1b.c to `extern u8 D_800A3560[];` /
`extern s16 D_800A3590[];` and indexed `D_800A3560[ctx_or_var_s2]` / `D_800A3590[var_s0]`,
replacing the retired chassis' `__asm__("la %0,D_800A3590")` cheat and its `(&D_800A3560)[i]` pun.
**Result:** part of the 194 -> 108 measurement; both accesses now emit the target's exact
addressing form. CONFIRMED.

### H2 - IconC70 is 0x20 bytes, not 8
**Mechanism:** frame layout. GCC 2.7.2 lays the locals region out contiguously; target's frame is
0x80 = 0x18 outgoing args + 0x50 locals + 0x18 (six callee-saved). `prim` is at 0x18 and `icon` at
0x48 in both builds, so `icon` must be declared 0x20 bytes wide for the locals region to be 0x50.
**Probe:** padded IconC70 from 8 to 0x20 bytes.
**Result:** 108 -> 101, frame 0x70 -> 0x88. CONFIRMED (the size; the tail's FIELD LAYOUT is still
a placeholder and must be recovered before submission - see the caveat in candidate.c).

## KILLED

### H3-a - a named intermediate for the D_800A3590 read defeats the giv reduction
**Probe:** `s32 h; h = D_800A3590[var_s0]; prim.p_static = t + (h << 4);` in place of the inline
`t + (D_800A3590[var_s0] << 4)`.
**Result:** 101 -> 101, byte-identical codegen; the .loop dump still prints
`giv at 319 reduced to (reg:SI 159)`. The reduction decision is taken on the RTL giv, which both
spellings produce identically. KILLED (instance: this chassis, floor-101 form, no FAKE constructs
present). Banked as `rejected/named-intermediate-for-3590-read.c`.

## OPEN FRONTIER

### H3 - the 7th callee-saved register is loop.c's hoist of `&D_800A3590` at insn 312
**Mechanism (dump-attributed, `tmp/grind/func_80070C70/dumps/text1b.loop:44793+`):**
`move_movables` hoists the loop-invariant symbol address into pseudo 126 with `savings 1`; that
gives the array-address giv an `add (reg:SI 126)` term; the giv then combines with the load's giv
at 319 and clears the reduction threshold that the bare `mult 2 add 0` giv at 315 fails
("not worth while, 0 vs 50"); the reduced giv becomes an induction pointer that occupies a 7th
callee-saved register. Target hoists nothing and re-materializes the address inside the loop.
**Next probe:** kill the hoist, not the load. The savings is 1 - this is a marginal decision, so
anything that raises the loop's register pressure or shortens pseudo 126's life should flip it.
Ordinary-C avenues to try FIRST, in order: (a) restructure the inner `if` so the address is only
reachable on one path / the invariant's life shrinks; (b) sink the whole
`prim.p_static = ...` computation into the two arms of the mode `if` (duplicated-statement family
is available if the plain form does not do it); (c) re-check with
`tools/ra_solver/inverse_compose.py classify` whether the seat rotation below is even reachable
while a 6th callee-saved allocno exists. Only after those are spent does
`.claude/rules/defeat-licm-hoist-var-reuse.md` become the relevant family - and it is FAKE-gated,
so it needs this exhaustion ledger to be real first.

### H4 - the residual is a 5-seat callee-saved rotation and should go to the solver, not to probes
**Mechanism:** local-alloc/global-alloc seat assignment. arg0 wants $s1 (we give $s0), var_s0 wants
$s0 (we give $s1), ctx wants $s2 (we give $s4), var_s3 wants $s3 (we give $s2), c60 wants $s4 (we
give $s5). Because almost every scored insn names a register, this rotation is what the 101 is
made of - which is why the s1 diagnostic that changed the array's index variable moved the score
by exactly 0.
**Next probe:** `tools/ra_solver` (global.c + local-alloc models) with the extra allocno from H3
both present and absent, to get a typed REACHABLE/FORECLOSED verdict and a ranked C-lever vector
before any more hand spelling. Do H3 first if the solver says the 6-allocno case is FORECLOSED.

### H5 - recover IconC70's real tail layout
**Mechanism:** correctness, and it unblocks a clean submission. The 0x20 size is proven by frame
arithmetic but candidate.c currently spells the tail as `s16 sp50[12]`, which is a placeholder, not
evidence. func_80069898's other callers (func_8006B120, func_8006CFBC, func_800720FC,
func_80074488, func_8006F97C - the last is an active queue sibling) write the fields this function
leaves alone; their asm names the real offsets and widths.
**Next probe:** read the `$a1`-relative stores in those five callers, build the true field list,
and replace the placeholder. Propagate the corrected type to func_8006F97C's ledger.

## [s1] D_800A3560 and D_800A3590 are ARRAYS, not the scalars the headers declare, and correcting the declarations reproduces the target's exact lui %hi / addu / lbu-lh %lo addressing.
- mechanism: Target's own relocations: 80070E28-80070E30 does lui %hi(D_800A3560), addu $at,$at,$s2, lbu %lo(D_800A3560)($at) with $s2 stepping by 3 per iteration; 80070E5C-80070E64 does the same shape for D_800A3590 with $a0 = var_s0<<1 and lh. That fused form is what GCC 2.7.2 emits for `extern T SYM[]`; a scalar declaration can only reach it through a pointer pun, which is a banned construct.
- probe: Changed both declarations in src/text1b.c (l.2124/l.6499 for D_800A3560, l.2129/l.6500 for D_800A3590) to array form and indexed D_800A3560[ctx_or_var_s2] / D_800A3590[var_s0], replacing the retired chassis' (&D_800A3560)[i] pun and its __asm__("la %0,D_800A3590") cheat. Measured with sandbox --disable all.
- result: Part of the 194 -> 108 drop. Both accesses now emit the target's addressing form exactly. Also resolves the brief's SPLIT-AGGREGATE signal negatively: the relocation names D_800A3590 itself, not g_replay_motion_shared_state_d(800A358A)+6, so prong (a) of the aggregate-merge family is NEGATIVE here and merging would change the %hi/%lo symbol away from what the bytes require.
- verdict: CONFIRMED

## [s1] IconC70 is declared 0x20 bytes wide in the original, not the 8 bytes the decomp-authored typedef gives it.
- mechanism: GCC 2.7.2 lays the locals region out contiguously. Target's frame is 0x80 = 0x18 outgoing args (SetDrawMode takes 5 args, the 5th at 0x10) + 0x50 locals + 0x18 for six callee-saved slots (s0,s1,s2,s3,s4,ra). prim sits at 0x18 and icon at 0x48 in both target and our build, so icon must span 0x48..0x68 = 0x20 for the locals region to be 0x50.
- probe: Padded IconC70 from 8 to 0x20 bytes and re-ran sandbox --disable all. Cross-checked the consumer: func_80069898 reads only offsets 0x0/0x2/0x4/0x6 of its $a1 argument (asm/funcs/func_80069898.s:23-35), so the tail is genuinely untouched by this call path.
- result: 108 -> 101, and our frame moved 0x70 -> 0x88. The SIZE is confirmed; the tail's FIELD LAYOUT is still a placeholder (s16 sp50[12]) and is flagged in candidate.c as something that must be recovered from func_80069898's other callers before any submission.
- verdict: CONFIRMED

## [s1] Introducing a named intermediate local for the D_800A3590[var_s0] read, i.e. `s32 h; h = D_800A3590[var_s0]; prim.p_static = t + (h << 4);`, changes the strength-reduction decision on the array-address giv.
- mechanism: Hypothesis was that splitting the load out of the address arithmetic would change the RTL giv's benefit accounting in loop.c strength_reduce and stop the reduction that costs us a 7th callee-saved register.
- probe: Applied that exact rewrite to src/text1b.c on the floor-101 chassis and re-ran sandbox --disable all; re-read tmp/grind/func_80070C70/dumps/text1b.loop.
- result: 101 -> 101, no codegen change at all. The dump still prints `giv at 319 reduced to (reg:SI 159)`. Both spellings lower to the same RTL giv, so the decision is untouched. Banked as memory/grind/func_80070C70/rejected/named-intermediate-for-3590-read.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-101 chassis (de-cheated body + array declarations + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s1] Changing which induction variable feeds the D_800A3590 address (indexing by the +3 biv ctx_or_var_s2 instead of by var_s0) moves the score.
- mechanism: Diagnostic only, deliberately semantics-breaking: D_800A3560's access escapes strength reduction because its index is itself a biv rather than a giv of the loop counter, so re-indexing D_800A3590 the same way should have restored the fused addressing form and freed the extra register.
- probe: Replaced D_800A3590[var_s0] with D_800A3590[ctx_or_var_s2] on the floor-108 chassis, measured sandbox --disable all, then reverted.
- result: 108 -> 108, exactly zero movement, and the prologue still saved 7 callee-saved registers with a 0x70 frame. This is the session's most useful negative: the score is saturated by the register-seat rotation, so arithmetic-level spelling probes will keep reading 0 until the allocation is fixed. Next sessions should attack the allocation, not the arithmetic.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-108 chassis (de-cheated body + array declarations, IconC70 still 8 bytes), no FAKE constructs present, sandbox --disable all, 2026-09-10

---

# Session 2 (structural, 2026-09-10) — floor unchanged at 101

## CONFIRMED

### H3-mech — the LICM gate on `&D_800A3590` is loop.c:1631 and its threshold is bracketed 26..29
**Mechanism:** `move_movables` accepts a movable when
`(threshold * savings * m->lifetime) >= insn_count` (loop.c:1631); `threshold` is
`(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` (loop.c:532) and this loop calls func_8007352C,
so the multiplier is 1 and threshold is a build constant. For insn 312 savings = 1, lifetime = 2.
**Probe:** added throwaway statements to the inner loop body to move `insn_count`, re-running
`pwsh tools/grinder/dump.ps1 func_80070C70` and reading the decision line each time. Five points:
insn_count 50 moved, 52 moved, 60 not desirable, 64 not desirable, 73 not desirable; and the
life-1 movables at insns 295/299 are "not desirable" already at insn_count 50.
**Result:** 26 <= threshold <= 29. CONFIRMED. Full table + arithmetic in
`tmp/grind/func_80070C70/s2/licm_threshold_bracket.md`.

### H3-suff — killing the hoist is SUFFICIENT to recover the target's addressing form
**Mechanism:** without reg 126 the address giv loses its invariant `add` term, the 317/319 combine
never happens, and the address is emitted in the fused `lui %hi / addu / lh %lo` shape.
**Probe:** in the 60-insn diagnostic build (hoist rejected) read the emitted assembly.
**Result:** `lh $2,D_800A3590($2)` at `tmp/grind/func_80070C70/dumps/text1b.s:16616` — the
assembler macro for target's exact 80070E5C-80070E64 sequence — with no `la` of D_800A3590 anywhere
in the preheader. The scale-1 control in the same loop reads `lbu $3,D_800A3560($19)` (line 16595).
CONFIRMED: the whole 101 residual chain is downstream of the single decision on insn 312.

### H3-life — reg 126's lifetime is 2 because the index scale is emitted between its set and its use
**Mechanism:** read from `tmp/grind/func_80070C70/dumps/text1b.cse`: insn 312 sets reg 126 to the
symbol, insn 315 is `(ashift (reg 75) (const_int 1))` (the element-size-2 scale), insn 317 is the
address plus. LOOP_REG_LIFETIME therefore spans two luids. D_800A3560, whose element size is 1, has
no scale insn and no symbol pseudo at all — its address is the fused
`(mem (plus (reg 74) (symbol_ref)))` at insn 291.
**Result:** CONFIRMED by direct RTL read. This is the actionable corollary: at lifetime 1 the
product is 26..29, below insn_count 50, and the movable is rejected.

## KILLED

### K1 — the `lhu`/`lh` pair on D_800A3558 is reachable by re-typing the declaration and casting
**Probe:** `extern s16 D_800A3558;` with the loop bound spelled `(s32)(D_800A35B0 + (D_800A3558 + 1))`
and the body spelled `(D_800A35B0 + (s16)(u16)D_800A3558)`, to try to reproduce target's
`lhu $a2` + `sll 16` + `sra 16` in the body against `lh $v0` in the bound.
**Result:** 101 -> 101, and the body still emits a single `lh $a1,%gp_rel(D_800A3558)`. GCC 2.7.2's
convert_to_integer folds `(short)(unsigned short)x` to `(short)x` whether x is declared s32 or s16,
so the cast spelling cannot split the two loads. KILLED (instance).

### K2 — a statement re-association or pointer spelling of the D_800A3590 read defeats the hoist
**Probe:** four variants measured against the floor-101 chassis via
`tmp/grind/func_80070C70/s2/probe.py`: `prim.p_static = (D_800A3590[var_s0] << 4) + t;`;
`*(D_800A3590 + var_s0)`; the split-init pair `prim.p_static = prim.p_geom + 0xC;
prim.p_static += D_800A3590[var_s0] << 4;`; and `D_800A3590[var_s0 + 0]`.
**Result:** all four measure 101 and all four still emit the preheader
`addiu s3,s3,%lo(D_800A3590)` hoist. The address expansion is decided in `memory_address` on the
ARRAY_REF's `mult` offset and none of these spellings changes that tree. KILLED (instance).

### K3 — giving D_800A3590 an explicit array bound changes the address expansion
**Probe:** `extern s16 D_800A3590[64];` (both declaration sites) and `[1]`, measured on the
floor-101 chassis; objdump-checked for the `la` form.
**Result:** 101, and `9368: addiu s3,s3,0 / R_MIPS_LO16 D_800A3590` is still there — the hoist
survives. A completed array type does not change `get_inner_reference`'s scale-2 offset.
KILLED (instance).

## OPEN FRONTIER (supersedes s1's H3 next-probe list, which is now spent)

### F1 — drop reg 126's lifetime from 2 to 1 so loop.c:1631 rejects the movable
This is the one lever the s2 measurements point at directly, and unlike the insn_count route it is
not blocked by arithmetic: threshold is at most 29 and insn_count is 50, so lifetime 1 loses by a
wide margin. reg 126's life is 2 solely because insn 315 (the `ashift` by 1 that scales the index)
is emitted between insn 312 (the symbol move) and insn 317 (the address plus). Any C shape in which
the scaled index is ALREADY in a pseudo when the address is expanded leaves 312 and 317 adjacent.
Next probe, in order: (a) find a second, earlier use of `var_s0 * 2` or of `D_800A3590[var_s0]`'s
address that the target's bytes also justify, so CSE has the scale available; (b) read
`tools/gcc-2.7.2/explow.c` `memory_address` and `expand_expr`'s ARRAY_REF path end-to-end and
identify which source shapes reach the reg-first PLUS that the scale-1 D_800A3560 access already
gets — that access is a working in-function control, so the fused form is demonstrably reachable
for this compiler in this loop; (c) if the answer turns out to require a construct outside ordinary
C, emit a ruling-request rather than spelling around it.

### F2 — the target's two differently-signed reads of D_800A3558
Target reads %gp_rel(D_800A3558) as `lhu` (80070DF4, 80070ECC; feeding sll/sra/addu in the body)
and as `lh` (80070E08, 80070ED0; feeding the loop bound). K1 killed the cast route. The remaining
candidates are a `u16` declaration of the symbol with the bound spelled to force a signed load, or
a `u16` intermediate object. Worth one measurement, but it is a 2-3 insn shape issue and it will
stay invisible in the score while the register rotation saturates it.

### F3 — IconC70's real tail layout (carried forward unchanged from s1)
The 0x20 size is proven by frame arithmetic; `s16 sp50[12]` in candidate.c is still a placeholder
and must be replaced from func_80069898's other callers (func_8006B120, func_8006CFBC,
func_800720FC, func_80074488, func_8006F97C) before any submission. Untouched this session.

## [s2] The hoist of &D_800A3590 into a loop-invariant pseudo is decided by loop.c:1631 `(threshold * savings * m->lifetime) >= insn_count`, and for this build threshold lies between 26 and 29 inclusive.
- mechanism: loop.c:532 sets threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs); the inner loop calls func_8007352C so the multiplier is 1 and threshold is a build constant no C can move. For insn 312, `(set (reg:SI 126) (symbol_ref "D_800A3590"))`, the dump reports savings 1 and m->lifetime 2, so the movable is accepted whenever 2*threshold >= insn_count.
- probe: Added throwaway statements to the inner do/while body purely to move insn_count, re-running `pwsh tools/grinder/dump.ps1 func_80070C70` and reading both the 'Loop from 284 to NNN: K real insns.' header and the insn-312 decision line in tmp/grind/func_80070C70/dumps/text1b.loop. Every diagnostic was reverted after its reading; the chassis was restored and re-measured at 101 afterwards.
- result: Five measured points on one chassis: insn_count 50 -> 'moved to 486'; 52 -> 'moved to 491'; 60 -> 'not desirable'; 64 -> 'not desirable'; 73 -> 'not desirable'. The life-1 movables in the same loop (insns 295 and 299) read 'not desirable' already at insn_count 50. Solving the inequality: moved at 52 gives threshold >= 26, rejected at 60 gives threshold <= 29. Two exact consequences follow. (a) The insn_count route needs 53-59+ RTL insns in the loop and is unreachable by byte-faithful C, because the target's own loop is SMALLER in RTL terms than ours - it has neither the symbol move nor a separate address plus. (b) The lifetime route is wide open: at m->lifetime == 1 the product is 26..29 against insn_count 50, so the movable would be rejected outright. Full table and arithmetic in tmp/grind/func_80070C70/s2/licm_threshold_bracket.md.
- verdict: CONFIRMED

## [s2] Rejecting the loop.c movable at insn 312 is by itself sufficient to make the D_800A3590 read come out in the target's fused lui %hi / addu / lh %lo form.
- mechanism: Without reg 126 the address giv at insn 317 loses its loop-invariant `add (reg:SI 126)` term, so it can neither be recognised as a giv nor combine with the load's giv at 319; the reduction 'giv at 319 reduced to (reg:SI 159)' never happens and no induction pointer is created to occupy a 7th callee-saved register.
- probe: Read the emitted assembly of the 60-insn diagnostic build, in which loop.c had printed 'not desirable' for insn 312, and compared it against the scale-1 D_800A3560 control in the same loop.
- result: tmp/grind/func_80070C70/dumps/text1b.s:16616 emits `lh $2,D_800A3590($2)` - the ASPSX/gas macro that expands to `lui $at,%hi(D_800A3590); addu $at,$at,$2; lh $v0,%lo(D_800A3590)($at)`, byte-for-byte the target's shape at 80070E5C-80070E64 - and there is no `la` / `lui+addiu` of D_800A3590 anywhere in the preheader. Line 16595 shows the control, `lbu $3,D_800A3560($19)`. This closes the causal chain s1 left open: hoist -> invariant add term -> 317/319 giv combine -> reduced giv -> 7th callee-saved induction pointer -> frame 0x88 vs target 0x80 -> the 5-seat rotation stacked on top. All of it hangs off this one decision.
- verdict: CONFIRMED

## [s2] reg 126's m->lifetime is 2 because the element-size-2 index scale insn is emitted between the symbol move and the address plus, and the scale-1 sibling access in the same loop never creates a symbol pseudo at all.
- mechanism: RTL expansion. A scale-1 ARRAY_REF offset is a bare pseudo, the PLUS canonicalises reg-first, `(plus reg symbol_ref)` is a legitimate MIPS address and no pseudo is created for the symbol. A scale-2 offset is a `mult`, the PLUS comes out symbol-first, memory_address (explow.c) cannot accept it, and force_reg'ing the whole address materialises the symbol into a pseudo whose live range then spans the intervening scale insn.
- probe: Read the pre-loop RTL in tmp/grind/func_80070C70/dumps/text1b.cse for both accesses in the inner loop.
- result: insn 291 is `(set (reg/v:QI 118) (mem/s:QI (plus:SI (reg/v:SI 74) (symbol_ref "D_800A3560"))))` - fused, no symbol pseudo. For D_800A3590 the sequence is insn 312 `(set (reg:SI 126) (symbol_ref "D_800A3590"))`, insn 315 `(set (reg:SI 128) (ashift:SI (reg/v:SI 75) (const_int 1)))`, insn 317 `(set (reg:SI 129) (plus:SI (reg:SI 128) (reg:SI 126)))`, insn 319 `(set (reg:HI 130) (mem/s:HI (reg:SI 129)))`. LOOP_REG_LIFETIME for reg 126 therefore spans exactly the two luids 312->317. This explains s1's unexplained asymmetry between the two array accesses, and it names the next lever precisely: make the scaled index already available when the address is expanded, 312 and 317 become adjacent, lifetime falls to 1, and loop.c:1631 rejects the movable.
- verdict: CONFIRMED

## [s2] Re-associating or re-spelling the D_800A3590 read - `(D_800A3590[var_s0] << 4) + t`, `*(D_800A3590 + var_s0)`, the split-init pair `prim.p_static = prim.p_geom + 0xC; prim.p_static += D_800A3590[var_s0] << 4;`, or `D_800A3590[var_s0 + 0]` - defeats the hoist on this chassis.
- mechanism: Hypothesis was that the operand order of the address PLUS, or the point at which the array reference is expanded inside the enclosing expression, is what force_reg's the symbol, so changing the surrounding expression would reach the reg-first PLUS that the scale-1 access already gets.
- probe: Four variants driven by tmp/grind/func_80070C70/s2/probe.py: each patched into src/text1b.c over the restored floor-101 chassis, measured with `sandbox func_80070C70 --disable all`, then checked by objdump for the preheader `la` of D_800A3590.
- result: All four measured 101 and all four still emit the hoisted `addiu s3,s3,%lo(D_800A3590)` in the preheader. The decision is taken on the ARRAY_REF's `mult` offset inside memory_address, which none of these spellings alters. Banked as memory/grind/func_80070C70/rejected/reassociate-3590-read-no-hoist-change.c. Note the split-init variant is byte-neutral rather than harmful, so it remains a free alternative spelling of the two prim.p_static stores if a reviewer ever objects to the `s32 t` form.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-101 chassis (s1 candidate.c body + `extern u8 D_800A3560[];` + `extern s16 D_800A3590[];` at both sites + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s2] Giving D_800A3590 a complete array type - `extern s16 D_800A3590[64];` or `[1]` at both declaration sites - changes the address expansion and removes the hoist.
- mechanism: Hypothesis was that a completed array type would let get_inner_reference take a constant-bounds path and fold the scaling differently.
- probe: Both declaration sites changed together over the restored floor-101 chassis, measured with `sandbox func_80070C70 --disable all`, then objdump-checked for the `la` form.
- result: 101, and `9368: addiu s3,s3,0` with `R_MIPS_LO16 D_800A3590` is still present - the hoist survives. A completed array type does not change the scale-2 offset. Banked as memory/grind/func_80070C70/rejected/array-bound-decl-D_800A3590.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-101 chassis (s1 candidate.c body + both array declarations + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s2] Re-declaring D_800A3558 as `extern s16` and spelling the loop bound as `(D_800A3558 + 1)` against a body cast of `(s16)(u16)D_800A3558` reproduces the target's two differently-signed reads of that symbol.
- mechanism: The target reads %gp_rel(D_800A3558) as `lhu $a2` at 80070DF4 and 80070ECC, feeding `sll 16 / sra 16 / addu $a1` in the body, and separately as `lh $v0` at 80070E08 and 80070ED0, feeding the loop bound. Hypothesis was that a halfword declaration plus a zero-extending cast in the body would split the two loads the way the target's bytes show.
- probe: Changed the declaration in src/text1b.c and rewrote both loop-bound occurrences, then measured `sandbox func_80070C70 --disable all` and grepped the objdump for `lhu` / `sra`.
- result: 101 -> 101, and the body still emits a single `lh $a1,%gp_rel(D_800A3558)`; no `lhu` and no `sra` anywhere in the function. GCC 2.7.2's convert_to_integer folds `(short)(unsigned short)x` to `(short)x` whether x is declared s32 or s16, so a cast spelling on a single integer declaration of that symbol cannot produce the pair. Banked as memory/grind/func_80070C70/rejected/s16-decl-D_800A3558-split-loads.c. What remains untried for this shape is a u16 declaration of the symbol, or a u16 intermediate object.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-101 chassis (s1 candidate.c body + both array declarations + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10
