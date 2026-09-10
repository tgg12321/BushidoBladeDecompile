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

---

# Session 3 (permuter, 2026-09-10) — floor 101 -> 99

## CONFIRMED

### H6 — a scale-2 array read reaches target's fused `lui %hi / addu / lh %lo` only through the COMPONENT_REF offset path
**Mechanism:** `D_800A3590[var_s0]` on a plain `extern s16 D_800A3590[]` is an ARRAY_REF with a
nonconstant index, which expand_expr (expr.c:4589) rewrites into `*(&array + index*2)`; the address
rtx is `(plus (mult (reg) 2) (symbol_ref))`. MIPS GO_IF_LEGITIMATE_ADDRESS
(config/mips/mips.h) accepts a PLUS only with a REG on one side, and swaps operands only when
`code0 != REG && code1 == REG`, so a MULT + SYMBOL_REF pair is rejected. memory_address
(explow.c:385) then break_out_memory_refs's the SYMBOL_REF into a pseudo and force_operand emits
the scale after it — the exact 312 / 315 / 317 triple s2 read out of text1b.cse, and the reason the
symbol pseudo had lifetime 2. Spelling the read as a record member (`D_800A3590[var_s0].v`) makes
it a COMPONENT_REF, so expand_expr takes the get_inner_reference path, expands the byte offset
FIRST as an ordinary insn (`expand_expr (offset, NULL_RTX, VOIDmode, 0)`), force_reg's it, and
forms `(plus (symbol_ref) (reg))` — which IS legitimate on MIPS. No symbol pseudo, no movable, no
hoist.
**Probe:** declared `typedef struct RecC70 { s16 v; } RecC70; extern RecC70 D_800A3590[];` at both
declaration sites and read `D_800A3590[var_s0].v`; measured `sandbox --disable all`; re-ran
`pwsh tools/grinder/dump.ps1 func_80070C70` and re-read text1b.loop; checked relocations with
`mipsel-linux-gnu-objdump -dr`.
**Result:** 101 -> 99, and build_insns 193 -> 194, i.e. the emitted instruction COUNT now equals
target's 194. `Insn 312: regno 126 (life 2) ... moved to 486` is gone from the loop dump and the
object carries exactly one HI16/LO16 pair for D_800A3590. `extern s16 D_800A3590[][1];` read as
`D_800A3590[var_s0][0]` measures identically 99, and `s16 v[1]` as the record member also measures
99 — the win is owned by the access PATH, not by the record spelling. CONFIRMED.

### H7 — at floor 99 the residual owner is loop.c strength_reduce, not move_movables
**Mechanism:** the gate is loop.c:3824 `v->lifetime * threshold * benefit < insn_count`, with
`threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` (loop.c:3241 — NOT the
`(1 + n_non_fixed_regs)` of loop.c:532 that s2 bracketed) and `benefit` reduced by
`add_cost * bl->biv_count` (add_cost = 2, loop.c:307). s2's 26..29 bracket pins n_non_fixed_regs to
25..28, so this threshold is 28..31.
**Probe:** read the new "Loop from 284 to 415: 48 real insns" section of text1b.loop.
**Result:** the scale-1 D_800A3560 address giv at insn 291 is rejected exactly as in target
("giv of insn 291 not worth while, 0 vs 48" — benefit 2 - 2*1 = 0). The scale-2 D_800A3590 address
giv at insn 319 (benefit 4) is COMBINED with the standalone scale giv at insn 313 (benefit 2) and
reduced to reg 159, producing a fourth induction register (`move s3,zero` / `addiu s3,s3,2`) that
target does not spend — target recomputes `sll $a0, $s0, 1` in-loop at 80070E3C. Arithmetic:
combined 6-2 = 4, product 112..124 vs insn_count 48 → reduced; and even if the combine were
prevented, 4-2 = 2 gives 56..62 vs 48 → still reduced. CONFIRMED.

## KILLED

### K4 — a u16 (or s16) declaration of D_800A3558 produces target's `lhu`+sign-extend / `lh` pair
**Probe:** three spellings on the floor-99 chassis (tmp/grind/func_80070C70/s3/probe2.py):
`extern u16 D_800A3558;` with the bound `(s32)(D_800A35B0 + ((s16)D_800A3558 + 1))` and body
`(D_800A35B0 + (s16)D_800A3558)`; the same with an uncast bound; and `extern s16 D_800A3558;` with
the body `(D_800A35B0 + (s16)(u16)D_800A3558)`.
**Result:** all three measure 99, byte-neutral against the `extern s32` declaration the chassis
carries. Together with s2's K1 (the cast route) this closes the declaration-retyping avenue for
that pair on this chassis. KILLED (instance).
**kill_scope:** instance
**measured_on:** floor-99 chassis (s3 candidate.c body + `extern u8 D_800A3560[];` + the RecC70
record declaration of D_800A3590 at both sites + IconC70 sized 0x20), no FAKE constructs present,
`sandbox --disable all`, 2026-09-10

### K5 — a loop-body statement restatement moves the score at floor 99
**Probe:** six variants (tmp/grind/func_80070C70/s3/probe3.py): dropping the `s32 t` intermediate
and restating both p_static stores from prim.p_geom; two orderings of the three increments;
`s16 v[1]` as the record member read `.v[0]`; `extern s16 D_800A3590[][1];` read `[var_s0][0]`; and
an explicit `(s32)` on the record read before the `<< 4`.
**Result:** all six measure 99 (the `t`-dropping variant emits 195 insns instead of 194 and still
scores 99). This re-confirms at floor 99 what s1 found at 108 — the score is saturated by the
callee-saved seat rotation plus the extra reduced-giv register, so arithmetic-level spelling reads
0. KILLED (instance).
**kill_scope:** instance
**measured_on:** floor-99 chassis as above, no FAKE constructs present, `sandbox --disable all`,
2026-09-10

## OPEN FRONTIER (supersedes s2's F1 — F1 is ANSWERED and spent; F2 is now K4)

### F1' — stop loop.c from reducing the D_800A3590 address giv (the last induction register)
This is the direct successor of s2's F1 and the one thing standing between 99 and the target's
register file. The gate arithmetic (H7) leaves exactly two terms C can touch:
(a) `bl->biv_count` for biv 75 (var_s0): at biv_count 2 the combined benefit falls from 4 to 2 and
    at biv_count 3 to 0. biv_count is the number of induction INCREMENT insns recorded for the biv
    register, so the question is whether any byte-faithful source shape gives var_s0 more than one
    increment site (the two arms of the `code != 5 && code != 16` test are the obvious place to
    look, and duplicated-statement-into-arms is a sanctioned family if a plain form does not do it
    — but check first whether target's single `addiu $s0,$s0,1` at 80070EC8 is even compatible
    with two source increments).
(b) `benefit` itself, which general_induction_var fixes at 4 for a mult-2 DEST_ADDR giv (2 per rtx
    operation). Only a scale-1 access gets benefit 2 and hence the rejection D_800A3560 enjoys.
    Ask whether the target's object model actually makes this access scale-1 — e.g. whether the
    2-byte record is really an element of the SAME 3-byte-stride record array as D_800A3560, read
    through a different member — before assuming the mult-2 shape is forced.
Do NOT re-try the insn_count route: H7 shows it needs a loop 15+ RTL insns larger than ours and
target's loop is smaller.

### F2' — recover D_800A3590's REAL record type (this is what blocks submission, not the floor)
The 2-byte stride is proven by the target's bytes, but `struct RecC70 { s16 v; }` is a
codegen-motivated spelling of that stride, not recovered evidence, and a reviewer will fairly ask
why a one-member struct exists (the `[][1]` array form is equally contrived and measures the same).
The symbol's other consumers are func_8006F100, func_80070188 and func_80070F78 (all still
INCLUDE_ASM) plus src/text1b_b.c:271; their `%hi/%lo(D_800A3590)` and base-register accesses name
the real offsets and widths. Per the brief's DATA MODEL block the census row must keep its
`alias of g_replay_motion_shared_state_d+6` suffix. If those consumers show a wider record with the
halfword at offset 0, the declaration becomes evidence-backed and the construct question closes;
if they show a bare halfword array, this needs a ruling-request before any submission.

### F3 — IconC70's real tail layout (carried forward unchanged from s1/s2)
`s16 sp50[12]` is still a placeholder. Recover from func_80069898's other callers
(func_8006B120, func_8006CFBC, func_800720FC, func_80074488, func_8006F97C). Untouched in s3.

## [s3] A scale-2 array read reaches the target's fused `lui %hi / addu / lh %lo` addressing when the reference is spelled as a COMPONENT_REF (a record member, or a nested ARRAY_REF with a constant inner index) instead of a bare ARRAY_REF with a nonconstant index.
- mechanism: `D_800A3590[var_s0]` on a plain `extern s16 D_800A3590[]` is an ARRAY_REF with a nonconstant index; expand_expr (tools/gcc-2.7.2/expr.c:4589) rewrites it into `*(&array + index*2)`, so the address rtx is `(plus (mult (reg) (const_int 2)) (symbol_ref))`. MIPS GO_IF_LEGITIMATE_ADDRESS (tools/gcc-2.7.2/config/mips/mips.h) accepts a PLUS only with a REG on one side and a CONST_INT or CONSTANT_ADDRESS_P term on the other, and swaps operands only under `code0 != REG && code1 == REG`; a MULT plus a SYMBOL_REF matches neither, so the address is rejected. memory_address (tools/gcc-2.7.2/explow.c:385) then runs break_out_memory_refs, which force_reg's the SYMBOL_REF into a pseudo, and force_operand emits the index scale after it -- exactly s2's insn 312 / 315 / 317 triple, and the reason the symbol pseudo's LOOP_REG_LIFETIME was 2 and loop.c:1631 hoisted it. Spelling the read as `D_800A3590[var_s0].v` makes it a COMPONENT_REF, so expand_expr takes the get_inner_reference path: the byte offset is folded into an `offset` tree, expanded FIRST as an ordinary insn via `expand_expr (offset, NULL_RTX, VOIDmode, 0)`, force_reg'd, and combined as `(plus (symbol_ref) (reg))`, which IS legitimate on MIPS (the macro's swap branch fires). No symbol pseudo is created, so there is no loop-invariant movable to hoist.
- probe: Declared `typedef struct RecC70 { s16 v; } RecC70; extern RecC70 D_800A3590[];` at both declaration sites in src/text1b.c (l.2129, l.6500) and read `D_800A3590[var_s0].v`; measured `& tools/wteng.ps1 main sandbox func_80070C70 --disable all`; re-ran `pwsh tools/grinder/dump.ps1 func_80070C70` and re-read the `Loop from 284 to 415` section of tmp/grind/func_80070C70/dumps/text1b.loop; checked relocations with `mipsel-linux-gnu-objdump -dr` on tmp/sandbox/func_80070C70/text1b.o.
- result: score 101 -> 99, target_insns 194, build_insns 193 -> 194 -- the emitted instruction COUNT now equals the target's exactly. The dump line `Insn 312: regno 126 (life 2), move-insn savings 1  moved to 486` is gone, and the object now carries exactly one R_MIPS_HI16 / R_MIPS_LO16 pair for D_800A3590 instead of a preheader `la` plus the in-loop pair. Two other spellings that reach the same path measure identically 99 (`extern s16 D_800A3590[][1];` read `D_800A3590[var_s0][0]`, and a record member declared `s16 v[1]` read `.v[0]`), so the win belongs to the access PATH and not to the particular record spelling. This also explains s1's unexplained asymmetry: D_800A3560 has element size 1, its offset is a bare pseudo, the PLUS comes out REG-plus-SYMBOL and is accepted directly, which is why that access matched target from the start.
- verdict: CONFIRMED

## [s3] At floor 99 the remaining extra callee-saved register is created by loop.c strength_reduce, which combines the standalone index-scale giv at insn 313 with the D_800A3590 address giv at insn 319 and reduces them into a fourth induction register, while the scale-1 D_800A3560 address giv at insn 291 is rejected exactly as the target's bytes require.
- mechanism: The gate is tools/gcc-2.7.2/loop.c:3824, `v->lifetime * threshold * benefit < insn_count` marks a giv not worth while, where threshold is `(loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` at loop.c:3241 -- a DIFFERENT constant from move_movables' `(1 + n_non_fixed_regs)` at loop.c:532 that s2 bracketed to 26..29 -- and benefit is the recorded benefit minus `add_cost * bl->biv_count` with add_cost = rtx_cost of a reg+reg PLUS = 2 (loop.c:307). The loop calls func_8007352C so the loop_has_call multiplier is 1; s2's bracket pins n_non_fixed_regs to 25..28 and hence this threshold to 28..31. With insn_count 48 and lifetime 1: insn 291 has benefit 2 - 2 = 0, product 0 < 48, rejected (the dump prints exactly `giv of insn 291 not worth while, 0 vs 48`); the combined 313+319 pair has benefit 6 - 2 = 4, product 112..124 >= 48, reduced.
- probe: Read the regenerated `Loop from 284 to 415: 48 real insns` section of tmp/grind/func_80070C70/dumps/text1b.loop (saved as tmp/grind/func_80070C70/s3/loop_decisions_at_99.txt) against the emitted object, and read the two threshold/benefit sites in tools/gcc-2.7.2/loop.c.
- result: The dump reads `Insn 313: giv reg 127 src reg 75 benefit 2 ... mult 2 add 0`, `Insn 319: dest address src reg 75 benefit 4 ... mult 2 add (symbol_ref "D_800A3590")`, `giv at 319 combined with giv at 313`, `giv at 313 reduced to (reg:SI 159)`. The reduced giv is emitted as `move s3,zero` plus `addiu s3,s3,2`; the target instead recomputes `sll $a0, $s0, 1` inside the loop at 80070E3C (in the delay slot of the first beq). That single register is the whole 0x88-versus-0x80 frame delta and the five-seat callee-saved rotation rides on top of it. The insn_count route is closed by the same arithmetic: even with the combine prevented, benefit 4 - 2 = 2 gives 56..62 against insn_count 48, so rejection would need a loop 15+ RTL insns larger than ours and the target's loop is smaller than ours.
- verdict: CONFIRMED

## [s3] Re-declaring D_800A3558 as u16 (with the loop bound cast or uncast) or as s16 reproduces the target's paired `lhu` + `sll 16` + `sra 16` body read and `lh` bound read.
- mechanism: s2's K1 showed GCC 2.7.2's convert_to_integer folds `(short)(unsigned short)x` to `(short)x`, so the cast route cannot split the two loads; the remaining candidate was that the symbol is a genuine u16 OBJECT, which would give a zero-extending HImode load that must then be sign-extended for the body while a separately spelled signed read supplies the bound.
- probe: Three spellings measured on the floor-99 chassis via tmp/grind/func_80070C70/s3/probe2.py: `extern u16 D_800A3558;` with bound `(s32)(D_800A35B0 + ((s16)D_800A3558 + 1))` and body `(D_800A35B0 + (s16)D_800A3558)`; the same with the bound spelled `(s32)(D_800A35B0 + (D_800A3558 + 1))`; and `extern s16 D_800A3558;` with body `(D_800A35B0 + (s16)(u16)D_800A3558)`.
- result: All three measure 99, byte-neutral against the `extern s32 D_800A3558;` the chassis carries. Banked as memory/grind/func_80070C70/rejected/u16-decl-D_800A3558-lhu-lh-pair.c. Together with s2's K1 this spends the declaration-retyping avenue for that 2-3 insn pair on this chassis; it stays invisible in the score while the allocation residual saturates it.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-99 chassis (s3 candidate.c body + `extern u8 D_800A3560[];` + the RecC70 record declaration of D_800A3590 at both sites + IconC70 sized 0x20), no FAKE constructs present, `sandbox --disable all`, 2026-09-10

## [s3] A loop-body statement restatement or an alternative record spelling moves the score off 99 on this chassis.
- mechanism: s1 measured at floor 108 that the score is saturated by the callee-saved seat rotation, so arithmetic-level probes read 0; the question was whether the s3 declaration win changed that, and whether the two contrived record spellings differ from each other.
- probe: Six variants measured via tmp/grind/func_80070C70/s3/probe3.py: dropping the `s32 t` intermediate and restating both p_static stores from prim.p_geom; var_s0's increment moved ahead of var_s3's; var_s0's increment moved after ctx_or_var_s2's; the record member declared `s16 v[1]` read `.v[0]`; `extern s16 D_800A3590[][1];` read `D_800A3590[var_s0][0]`; and an explicit `(s32)` on the record read before the `<< 4`.
- result: All six measure 99 (the t-dropping variant emits 195 insns instead of 194 and still scores 99). Banked as memory/grind/func_80070C70/rejected/loop-body-restatements-at-99.c. Two consequences: the saturation finding still holds at 99, so the next session should attack the allocation and not the arithmetic; and the two contrived declarations of D_800A3590 are interchangeable, which is why the record-type question (frontier F2') is an evidence question rather than a codegen one.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-99 chassis (s3 candidate.c body + `extern u8 D_800A3560[];` + the RecC70 record declaration of D_800A3590 at both sites + IconC70 sized 0x20), no FAKE constructs present, `sandbox --disable all`, 2026-09-10

## [s3] A permuter campaign on this function's chassis produces a usable spelling proposal for the remaining residual.
- mechanism: Mandated modality. Two campaigns were run through tools/permuter_campaign.py with telemetry, the second re-seeded on the improved chassis per the 2026-09-01 chassis rule.
- probe: Campaign A on the floor-101 chassis (label s3-floor101-chassis): base permuter score 5083, 12,658 iterations, 562 outputs, best 4198. Campaign B re-seeded on the floor-99 chassis (label s3-floor99-componentref-chassis): base permuter score 4778, 31,512 iterations, 1,241 new outputs, best 3498. Both harvested with --stop; workspace tmp/perm_70c70, logs copied to tmp/grind/func_80070C70/s3/campaign_b.log.
- result: Neither campaign yielded a usable proposal. Campaign A's best find (output-4198-1) lifts `new_var = prim.p_geom;` ACROSS a later reassignment of prim.p_geom, so it captures *(ctx+4) and then uses it where the source needs *(ctx) -- semantically wrong, a scorer artifact rather than a proposal, recorded so no later session re-chases it. Campaign B's best find (output-3498-1) reaches its score entirely by introducing the permuter's synthetic `inline_fn` / `inline_fn2` helper functions, which fails the human-programmer test outright and is not a decomp form. The useful positive datum is that the base permuter score fell 5083 -> 4778 across the declaration change, i.e. the s3 win registers in the permuter metric too. Recommendation for any future campaign on this function: set `perm_inline = 0.0` in settings.toml's weight_overrides (as tools/mar_perm_workspace.sh does), because otherwise perm_inline dominates the output stream with unusable finds.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-101 chassis (campaign A) and floor-99 chassis (campaign B), both with default permuter weights including perm_inline, --stack-diffs on by default, 8 jobs, 44,170 iterations combined, no FAKE constructs present, 2026-09-10

---

# Session 4 (permuter, 2026-09-10) — floor 99 -> 61

## CONFIRMED

### H8 — three source increment sites for var_s0 raise `bl->biv_count` to 3, which drives the D_800A3590 address giv's benefit to 0 and makes loop.c REJECT the reduction that s3 (H7) pinned as the residual
**Mechanism:** tools/gcc-2.7.2/loop.c:3824 marks a giv "not worth while" when
`v->lifetime * threshold * benefit < insn_count`, where the benefit used is the recorded benefit
MINUS `add_cost * bl->biv_count` (add_cost = rtx_cost of a reg+reg PLUS = 2, loop.c:307) and
`threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs)` (loop.c:3241), bracketed by s2/s3 to
28..31. `bl->biv_count` is the number of induction INCREMENT insns recorded for the biv register,
so it is directly controlled by how many times the C source writes `var_s0 += 1`.
  - biv_count 1 (s3 chassis): combined 313+319 giv benefit 6 - 2 = 4, product 112..124 >= insn_count
    48 → REDUCED into a fourth induction register (`move sN,zero` / `addiu sN,sN,2`).
  - biv_count 2: benefit 6 - 4 = 2, product 56..62 >= 48 → still reduced (and measured worse, 101).
  - biv_count 3: benefit 6 - 6 = 0, product 0 < insn_count → REJECTED, which is what the target's
    bytes require (target recomputes `sll $a0, $s0, 1` in-loop at 80070E3C).
**Probe:** rewrote the loop so `var_s0 += 1;` appears in all three arms — both arms of the inner
mode `if`, and a plain `else` arm on the `code != 5 && code != 16` test — while `var_s3 += 0x16C;`
and `ctx_or_var_s2 += 3;` stay in a SINGLE shared fall-through block after the outer `if/else`.
Measured `sandbox --disable all`; re-ran `pwsh tools/grinder/dump.ps1 func_80070C70` and re-read the
`Loop from 284 to 460` section of tmp/grind/func_80070C70/dumps/text1b.loop.
**Result:** 99 -> 61, build_insns 194 -> 192. The loop dump now prints
`Insn 331: giv reg 127 src reg 75 benefit 2 ... mult 2 add 0`,
`Insn 337: dest address src reg 75 benefit 4 ... mult 2 add (symbol_ref "D_800A3590")`,
`giv at 337 combined with giv at 331`, then `giv of insn 331 not worth while, 0 vs 63` — the
reduction is GONE (s3's dump printed `giv at 313 reduced to (reg:SI 159)` at the same site), and
the scale-1 D_800A3560 giv at insn 291 is rejected harder than before (`-124 vs 63`). CONFIRMED.

### H9 — WHICH statement is duplicated matters as much as the site count; only `var_s0 += 1` may be duplicated
**Mechanism:** every extra duplicated statement is either paid for in emitted insns (cross-jump
does not re-merge across the scheduler's hoisting of the increments into the arms) or creates a NEW
loop-invariant movable. Duplicating the whole call tail puts `prim.code = 1` in the loop twice, and
move_movables then hoists the constant 1 into a SEVENTH callee-saved register (`li s4,1` in the
preheader) that the target does not spend.
**Probe:** a 9-variant sweep at biv_count 3 (tmp/grind/func_80070C70/s4/probe2.py, probe3.py,
probe4.py), all measured `sandbox --disable all`.
**Result:** whole tail + all increments in both mode arms 88 (200 insns); the same with an `else`
skip arm 88 (200); all three increments in the arms 93 (195); `prim.code = 1` shared with link+call
duplicated 77 (198); only the call duplicated 98 (197); only `var_s0 += 1` duplicated but with a
`continue`-style skip arm that still carries its own copies of the other two increments 65 (195);
and the winner — only `var_s0 += 1` duplicated, skip arm spelled as a plain `else`, the other two
increments shared — 61 (192). CONFIRMED.

## KILLED

### K6 — two source increment sites (biv_count 2) are enough to stop the giv reduction
**Probe:** three 2-site spellings on the floor-99 chassis (tmp/grind/func_80070C70/s4/probe.py):
a `continue`-style skip arm carrying all three increments plus the shared tail copy; the same with
the skip arm as an `if/else`; and a `for (; cond; var_s3 += 0x16C, var_s0 += 1, ctx += 3)` control.
**Result:** 101, 101 and 102 respectively — all WORSE than the 99 one-site chassis, and the loop
dump still reduces the giv (benefit 6 - 2*2 = 2, product 56..62 >= insn_count 48). The arithmetic
at loop.c:3824 needs benefit <= 1, i.e. biv_count >= 3. KILLED (instance).
Banked as `rejected/two-increment-sites-biv-count-2.c`.
**kill_scope:** instance
**measured_on:** floor-99 chassis (s3 candidate.c body + `extern u8 D_800A3560[];` + the RecC70
record declaration of D_800A3590 at both sites + IconC70 sized 0x20), no FAKE constructs present,
`sandbox --disable all`, 2026-09-10

### K7 — a u16 / s16 declaration of D_800A3558 reproduces the target's `lhu` + `sll 16` + `sra 16`
body read on the NEW biv_count-3 chassis (re-measurement of s3's K4, which was chassis-relative)
**Probe:** four spellings on the floor-61 chassis (tmp/grind/func_80070C70/s4/probe5.py):
`extern u16 D_800A3558;` with body `(D_800A35B0 + (s16)D_800A3558)`; the same with the bound
spelled `(s32)(D_800A35B0 + (s16)(D_800A3558 + 1))`; `extern s16 D_800A3558;` with body
`(D_800A35B0 + (s16)(u16)D_800A3558)`; and the u16 declaration with the loop-entry test unchanged.
**Result:** 61 / 67 / 61 / 61 — every byte-neutral spelling still measures 61 and still emits a
single `lh` in the body block; only the uncast-bound variant moves, and it moves the wrong way
(67, 195 insns). s3's K4 therefore survives the chassis change. KILLED (instance).
**kill_scope:** instance
**measured_on:** floor-61 chassis (s4 candidate.c body + `extern u8 D_800A3560[];` + the RecC70
record declaration of D_800A3590 at both sites + IconC70 sized 0x20), no FAKE constructs present,
`sandbox --disable all`, 2026-09-10

## OPEN FRONTIER (supersedes s3's F1' — F1' is ANSWERED and spent)

### F4 — the last two missing insns are the target's tail-block `lhu` of D_800A3558 plus its
in-body `sll 16 / sra 16` sign-extension
We emit 192 insns; the target emits 194. The pair we lack is at 80070ECC (`lhu $a2,
%gp_rel(D_800A3558)`, in the loop TAIL block, alongside the condition's own `lh $v0` of the SAME
address at 80070ED0) and 80070E78/E7C (`sll $v0,$a2,16` / `sra $v0,$v0,16` in the NEXT iteration's
body). Declaration retyping is spent (K7 above, s3 K4, s2 K1): every u16/s16 spelling emits one
`lh` because combine folds the sign_extend into the load when both live in the SAME basic block.
The structural fact the target's bytes show is that its zero-extending load lives in a DIFFERENT
basic block from its sign-extension, which is exactly what defeats that fold. Next probe: find a
byte-faithful source shape that separates them — e.g. reading the value into a variable whose live
range crosses the loop back-edge, or re-spelling the loop-continuation test so the body's read is
the one CSE keeps in the tail block. Read the .combine dump for the fold before hand-spelling.

### F5 — the callee-saved seat rotation at floor 61
Target: arg0=$s1, var_s0=$s0, var_s3=$s3, ctx_or_var_s2=$s2, c60=$s4 (six saved slots incl. $ra,
frame 0x80). The floor-61 build still rotates these. Now that the extra induction register is gone
this is a clean 5-seat permutation with no spurious allocno, i.e. exactly the shape
`tools/ra_solver/inverse_compose.py classify` is for. Do that BEFORE any more hand spelling.

### F6 — recover the REAL types behind RecC70 and IconC70 (carried from s1/s2/s3, still unspent)
Unchanged and still the thing that blocks submission independently of the floor. RecC70 from
func_8006F100 / func_80070188 / func_80070F78 / src/text1b_b.c:271; IconC70's tail from
func_80069898's other callers (func_8006B120, func_8006CFBC, func_800720FC, func_80074488,
func_8006F97C).

### H10 - the permuter campaign on the biv_count-3 chassis DID produce two usable proposals (61 -> 56)
**Mechanism:** mandated modality, run per the 2026-09-01 chassis rule on a structurally NEW chassis
(the biv_count-3 loop), and with `perm_inline = 0.0` in settings.toml's `[weight_overrides]` -
s3's explicit recommendation, because otherwise the permuter's synthetic `inline_fn` helpers
dominate the output stream with forms that fail the human-programmer test outright.
**Probe:** `tools/permuter_campaign.py launch --func func_80070C70 --dir tmp/perm_70c70_s4
--label s4-biv3-chassis-floor61 -j 8`; base permuter score 4228 (4778 at floor 99, 5083 at 101);
25,626 iterations over 869 s; 288 new outputs; best 3325. Harvested with `--stop`. The best find
(`output-3325-1`) was read by hand, its cheat-smelling part discarded (`var_s0 = (unsigned long
long) 0` is a redundant width cast, forbidden family F2), and its two REAL changes were re-spelled
by hand and measured individually with `sandbox --disable all`
(tmp/grind/func_80070C70/s4/probe6.py).
**Result:** (i) writing `prim.link = *(s32 *)(arg0 + 0x10);` BEFORE `prim.code = 1;` at the FIRST
func_8007352C call site: 61 -> 58 (192 insns), plain statement reordering, ordinary C.
(ii) reading `*(s32 *)(ctx_or_var_s2 + 8)` into a named intermediate `new_var` before the
`*(arg0 + 0x18) += 0xC` store and assigning it to prim.p_geom after: 59 alone (190 insns);
together with (i): 56 (190 insns). Both adopted into candidate.c. This is the first campaign on
this function to yield a usable proposal - the difference from s3's two 0-find campaigns is the
chassis (a basin that still had the extra induction register in it had nothing to offer) plus the
`perm_inline = 0.0` override. CONFIRMED.

## [s4] Three source increment sites for var_s0 raise bl->biv_count to 3, which drives the combined D_800A3590 address giv's benefit to 0 and makes loop.c strength_reduce reject the reduction that s3 (H7) identified as the whole 0x88-vs-0x80 frame delta.
- mechanism: tools/gcc-2.7.2/loop.c:3824 marks a giv 'not worth while' when v->lifetime * threshold * benefit < insn_count, where the benefit used is the recorded benefit minus add_cost * bl->biv_count (add_cost = rtx_cost of a reg+reg PLUS = 2, loop.c:307) and threshold = (loop_has_call ? 1 : 2) * (3 + n_non_fixed_regs) (loop.c:3241) = 28..31 here. bl->biv_count counts the induction INCREMENT insns recorded for the biv register, so the C source controls it directly. biv_count 1 -> benefit 6-2=4 -> product 112..124 >= insn_count -> reduced; biv_count 3 -> benefit 6-6=0 -> product 0 < insn_count -> rejected.
- probe: Rewrote the loop so `var_s0 += 1;` appears in all three arms (both arms of the inner mode if, plus a plain else on the code != 5 && code != 16 test) while `var_s3 += 0x16C;` and `ctx_or_var_s2 += 3;` stay in one shared fall-through block (the target's .L80070EC4 shape). Measured `sandbox --disable all`; regenerated the cc1 -da dumps with `pwsh tools/grinder/dump.ps1 func_80070C70` and re-read the `Loop from 284 to 460` section of text1b.loop; checked the emitted loop with `mipsel-linux-gnu-objdump -dr`.
- result: 99 -> 61, build_insns 194 -> 192. text1b.loop now prints `giv at 337 combined with giv at 331` then `giv of insn 331 not worth while, 0 vs 63`; s3's `giv at 313 reduced to (reg:SI 159)` is gone and insn 291's scale-1 D_800A3560 giv is rejected harder (-124 vs 63). The extra induction register (`move sN,zero` / `addiu sN,sN,2`) is no longer emitted; we now recompute `sll $a0, var_s0, 1` in the loop like the target.
- verdict: CONFIRMED

## [s4] Which statement is duplicated matters as much as the number of increment sites: only `var_s0 += 1` may be duplicated, and the skip arm must be a plain else with no increments of its own.
- mechanism: Every additional duplicated statement is either paid for in emitted insns (jump2 cross-jumping does not re-merge the copies, because the scheduler hoists the increments up into the arms ahead of the merge point) or creates a new loop-invariant movable. Duplicating the call tail puts `prim.code = 1` in the loop twice, and move_movables then hoists the constant 1 into a SEVENTH callee-saved register (`li s4,1` in the preheader) that the target does not spend.
- probe: Nine biv_count-3 spellings measured with `sandbox --disable all` (tmp/grind/func_80070C70/s4/probe2.py, probe3.py, probe4.py).
- result: whole call tail + all three increments in both mode arms 88 (200 insns); same with an else skip arm 88 (200); all three increments in the arms 93 (195); prim.code shared, link+call+increments duplicated 77 (198); only the call duplicated 98 (197); only `var_s0 += 1` duplicated but with a continue-style skip arm carrying its own copies of the other two increments 65 (195); only `var_s0 += 1` duplicated with an else skip arm and the other two increments shared 61 (192). Banked as rejected/over-duplicated-arms-at-biv3.c.
- verdict: CONFIRMED

## [s4] The permuter campaign on the biv_count-3 chassis, run with perm_inline = 0.0, produces usable spelling proposals.
- mechanism: Mandated modality, seeded on a structurally new chassis per the 2026-09-01 chassis rule, with s3's recommended weight override so the permuter's synthetic inline_fn helpers stop dominating the output stream.
- probe: tools/permuter_campaign.py launch --func func_80070C70 --dir tmp/perm_70c70_s4 --label s4-biv3-chassis-floor61 -j 8; waited in-turn with `permuter_campaign.py wait`; harvested with --stop. 25,626 iterations in 869 s, 288 new outputs, base permuter score 4228, best 3325. The best find was hand-read and its real changes re-spelled and measured individually (probe6.py).
- result: Two usable proposals. (i) writing `prim.link = *(s32 *)(arg0 + 0x10);` BEFORE `prim.code = 1;` at the first func_8007352C call site (plain reordering, ordinary C): 61 -> 58. (ii) reading `*(s32 *)(ctx_or_var_s2 + 8)` into a named intermediate `new_var` before the `*(arg0 + 0x18) += 0xC` store and assigning it to prim.p_geom after: 59 alone. Together: 56 at 190 insns. The find's third change, `var_s0 = (unsigned long long) 0`, is a redundant width cast (forbidden family F2) and was discarded rather than adopted.
- verdict: CONFIRMED

## [s4] Two source increment sites for var_s0 (biv_count 2) stop the D_800A3590 address giv from being reduced.
- mechanism: At biv_count 2 the combined giv's benefit is 6 - 2*2 = 2 and the loop.c:3824 product is 56..62 against insn_count 48, so the giv is still reduced; only benefit <= 1 (biv_count >= 3) rejects it.
- probe: Three 2-site spellings on the floor-99 chassis (tmp/grind/func_80070C70/s4/probe.py): a continue-style skip arm carrying all three increments plus the shared copy; the same with the skip arm as an if/else; and a `for (; cond; var_s3 += 0x16C, var_s0 += 1, ctx += 3)` control.
- result: 101, 101 and 102 - all worse than the 99 one-site chassis, and the loop dump still reduces the giv. Banked as rejected/two-increment-sites-biv-count-2.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-99 chassis (s3 candidate.c body + extern u8 D_800A3560[]; + the RecC70 record declaration of D_800A3590 at both sites + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s4] A u16 or s16 declaration of D_800A3558 reproduces the target's tail-block lhu plus in-body sll 16 / sra 16 pair on the new biv_count-3 chassis (re-measurement of s3's chassis-relative K4).
- mechanism: combine folds a sign_extend into a zero-extending load when both insns sit in the SAME basic block, so any declaration spelling collapses to one lh; the target's lhu at 80070ECC sits in the loop TAIL block and its sign-extension at 80070E78/E7C in the next iteration's body block, which is what defeats the fold.
- probe: Four spellings on the floor-61 chassis (tmp/grind/func_80070C70/s4/probe5.py): extern u16 with body (s16)D_800A3558; the same with the bound spelled (s32)(D_800A35B0 + (s16)(D_800A3558 + 1)); extern s16 with body (s16)(u16)D_800A3558; and the u16 declaration with the loop-entry test unchanged.
- result: 61 / 67 / 61 / 61 - every byte-neutral spelling still emits a single lh in the body block. s3's K4 survives the chassis change; the remaining 2-insn shortfall (190 vs 194) is this pair.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-61 chassis (s4 candidate.c loop + extern u8 D_800A3560[]; + the RecC70 record declaration of D_800A3590 at both sites + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s5] Spelling the second loop as a `for (var_s0 = 0; var_s0 < <bound>; var_s0++)` whose secondary counters are expressions of var_s0 makes jump.c's duplicate_loop_exit_test fire, which is the precondition for cse.c's cse_around_loop to place the D_800A3558 read in the loop TAIL block (frontier F4) and for strength_reduce to emit the var_s3/ctx initialisations after the guard (the target's 80070E20/E24).
- mechanism: jump.c:2163 `duplicate_loop_exit_test` only applies to a loop whose NOTE_INSN_LOOP_BEG is followed by an unconditional jump, i.e. a source loop with the test at the TOP (`for`/`while`), never an `if (...) { do { ... } while (...); }`. It copies the exit test in front of the loop (that copy IS the target's `blez` guard) and marks the ORIGINAL test's registers REG_LOOP_TEST_P (jump.c:2253). cse.c:8581 then calls `cse_around_loop` (cse.c:7741), and `cse_set_around_loop` (cse.c:7909) replaces a loop-head SET_SRC with a REG_LOOP_TEST_P register whose value the tail's test already computed, emitting a copy in the preheader. That is the only mechanism in GCC 2.7.2 that can put a load in the loop tail and consume it at the top of the next iteration, which is exactly the target's `lhu $a2, %gp_rel(D_800A3558)` at 80070ECC feeding `sll/sra 16` at 80070E78/E7C. Separately, expressing var_s3 (`0x50 + i*0x16C`) and ctx (`i*3`) as functions of var_s0 makes them GIVs, and strength_reduce emits reduced-giv initialisations at loop_start -- i.e. AFTER the duplicated guard, which is where the target's `addiu $s3,$zero,0x50` / `addu $s2,$zero,$zero` sit and where no if-guarded do/while chassis can put them.
- probe: Eight structural chassis variants (tmp/grind/func_80070C70/s5/struct/, S1-S8) crossing {for, while, for-with-explicit-accumulators} x {RecC70 record, plain s16 array} x {s32, u16 declaration of D_800A3558}, scored with `sandbox --disable all` via tmp/grind/func_80070C70/s5/sweep.py; then the frame/decl/bound refinements (s5/frame/, s5/decl/, s5/bound/); diffed against asm/funcs/func_80070C70.s with s5/cmp2.py.
- result: 56 -> 57 at 193 insns (up from 190) for the first for-loop chassis, then 53 at 193 after dropping the redundant `(s32)` cast on the bound. The emitted code now carries the tail-block `lhu` + next-iteration `sll 16/sra 16` pair (F4 CLOSED) AND the target's exact callee-saved seat assignment arg0=$s1, var_s0=$s0, var_s3=$s3, ctx=$s2, c60=$s4 (F5 CLOSED). Every FAKE construct the ledger had accumulated (s4's three-arm `var_s0 += 1`, the `new_var` named intermediate, the RecC70 one-member record) is GONE: the 53 body is ordinary C.
- verdict: CONFIRMED

## [s5] D_800A3590 is a plain `extern s16 D_800A3590[]` halfword array indexed by the loop counter, not a record; the s3-era RecC70 COMPONENT_REF spelling is actively harmful on the correct chassis.
- mechanism: s3 adopted `typedef struct RecC70 { s16 v; }` because on the do/while chassis it routed the read through expand_expr's COMPONENT_REF/get_inner_reference path and bought 2 points. On the for-loop chassis the target's own addressing (`sll $a0,$s0,1` in the delay slot at 80070E3C, then `lui $at,%hi(D_800A3590) / addu $at,$at,$a0 / lh $v0,%lo(D_800A3590)($at)`) is what a plain halfword array indexed by the loop counter emits directly.
- probe: S1/S3/S5 (record) vs S2/S4/S6 (plain array) in tmp/grind/func_80070C70/s5/struct/, and D2 vs D1 in s5/decl/.
- result: record 94/94/94 and 90; plain array 57/57/57 and 53. Banked as rejected/rec-record-decl-on-for-chassis-94.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-53 for-loop chassis (s5 candidate.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s5] Re-associating or re-casting the loop bound, the body condition, the D_800A3560 index and the prim.mode expression moves the score off 53 on the for-loop chassis.
- mechanism: exhaustive spelling sweep of the four expressions that carry the residual, 7 bound forms x 3 condition forms x 2 index forms x 2 mode forms = 84 spellings.
- probe: tmp/grind/func_80070C70/s5/bound/ swept with s5/sweep.py; log tmp/grind/func_80070C70/s5/bound.log.
- result: histogram {53: 36, 57: 36, 58: 12}. The ONLY live axis is the bound's redundant `(s32)` cast: every uncast form (`D_800A35B0 + (s16)D_800A3558 + 1` and its rotations) is 53, every `(s32)(...)` form is 57, and one shape is 58. Condition order, `var_s0 * 3` vs `3 * var_s0`, and `0x50 + var_s0 * 0x16C` vs `var_s0 * 0x16C + 0x50` are ALL byte-neutral. Banked as rejected/s32-cast-loop-bound-57.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-53 for-loop chassis (s5 candidate.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s5] The 172-spelling naming/declaration-order enumeration of the pre-loop block moves the floor-56 do/while chassis below 55.
- mechanism: mandated `enumerate` modality. The block from `var_s0 = 0` through the loop-entry `if` was written in fully-named form (dst, geom, base, shown, limit) and swept with tools/spelling_enum.py over the keep-vs-inline and declaration-order axes (--no-swaps).
- probe: tmp/grind/func_80070C70/s5/enum_region1.c -> s5/enum1/ (172 variants), swept with s5/sweep.py; log s5/enum1.log.
- result: histogram {55: 1, 58: 2, 59: 9, 60: 28, 63: 26, 64: 106}; best 55 (v169) against the 56 baseline. The entire naming/ordering space of that block on the do/while chassis is worth one point, which is what motivated attacking the loop's STRUCTURE instead.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-56 chassis (s4 candidate.c body + extern u8 D_800A3560[]; + the RecC70 record declaration of D_800A3590 + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s5] Hoisting the loop-body block-scoped locals (`u8 code`, `s32 t`) to function scope, or inlining them, removes the +0x18 frame overshoot.
- mechanism: The floor-53 build's frame is 0x98 against the target's 0x80. The extra 0x18 is three pseudos (118, 168, 171) that appear only as `(use (reg))` insns created between the .flow and .lreg dumps, carry NO conflicts in .greg, receive no hard register, and are given 8-byte stack slots at sp+104/112/120. Hypothesis under test: they are block-scoped locals kept alive across the loop.
- probe: tmp/grind/func_80070C70/s5/frame/ U1 (hoist t), U2 (hoist code), U3 (hoist both), U4 (inline t).
- result: 57 / 57 / 57 / 59 against the 57 baseline -- block scope is byte-neutral and inlining `t` is worse. The three dead pseudos are NOT the block-scoped locals; they are the leftovers of duplicate_loop_exit_test's `reg_map` copies (jump.c:2246).
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-57 for-loop chassis (S4_for_arr_u16 body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + extern u16 D_800A3558; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s5] Spelling the second loop as `for (var_s0 = 0; var_s0 < <bound>; var_s0++)` with the secondary counters written as expressions of var_s0 makes jump.c's duplicate_loop_exit_test fire, enabling cse_around_loop to place the D_800A3558 read in the loop tail block and strength_reduce to emit the var_s3/ctx initialisations after the guard.
- mechanism: jump.c:2163 duplicate_loop_exit_test requires a NOTE_INSN_LOOP_BEG followed by an unconditional jump (a top-test for/while loop, never an if-guarded do/while). It copies the exit test in front of the loop -- that copy is the target's blez guard at 80070E18 -- and marks the original test's registers REG_LOOP_TEST_P at jump.c:2253. cse.c:8581 then calls cse_around_loop (cse.c:7741), whose cse_set_around_loop (cse.c:7909) rewrites a loop-head SET_SRC to a REG_LOOP_TEST_P register the tail test already loaded. That is the only GCC 2.7.2 mechanism that puts a load in the loop tail and consumes it at the top of the next iteration, i.e. the target's lhu $a2,%gp_rel(D_800A3558) at 80070ECC feeding sll/sra 16 at 80070E78/E7C. Separately, var_s3 = 0x50 + i*0x16C and ctx = i*3 become GIVs, so strength_reduce emits their initialisations at loop_start, after the duplicated guard, which is where the target's addiu $s3,$zero,0x50 / addu $s2,$zero,$zero sit at 80070E20/E24.
- probe: Eight structural chassis variants (tmp/grind/func_80070C70/s5/struct/, S1-S8) crossing {for, while, for-with-explicit-accumulators} x {RecC70 record, plain s16 array} x {s32, u16 declaration of D_800A3558}, then the frame/decl/bound refinements (s5/frame/, s5/decl/, s5/bound/), all scored with `sandbox --disable all` and diffed against asm/funcs/func_80070C70.s with s5/cmp2.py.
- result: 56 -> 57 at 193 insns (up from 190) on the first for-loop chassis, then 53 at 193 once the bound's redundant (s32) cast is dropped. The build now emits the tail-block lhu plus the next-iteration sll 16 / sra 16 pair (frontier F4 closed) and the target's exact callee-saved seats arg0=$s1, var_s0=$s0, var_s3=$s3, ctx=$s2, c60=$s4 (frontier F5 closed). s4's three-arm `var_s0 += 1`, the `new_var` named intermediate and the RecC70 record are all retired: the 53 body is ordinary C with no FAKE construct of any kind.
- verdict: CONFIRMED

## [s5] D_800A3590 declared as the s3-era one-member RecC70 record keeps the floor at or below the plain `extern s16 D_800A3590[]` array spelling on the for-loop chassis.
- mechanism: s3 adopted the record because on the do/while chassis it routed the read through expand_expr's COMPONENT_REF/get_inner_reference offset path and bought two points. On the for-loop chassis the target's own addressing (sll $a0,$s0,1 at 80070E3C, then lui $at,%hi / addu $at,$at,$a0 / lh $v0,%lo($at)) is exactly what a plain halfword array indexed by the loop counter emits.
- probe: S1/S3/S5 (record) versus S2/S4/S6 (plain array) in tmp/grind/func_80070C70/s5/struct/, and D2 versus D1 in s5/decl/.
- result: record 94/94/94 and 90; plain array 57/57/57 and 53. Banked as rejected/rec-record-decl-on-for-chassis-94.c. This also closes the s1-s4 frontier item asking for RecC70's real field list: there is no record, D_800A3590 is a plain s16[].
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-53 for-loop chassis (s5 candidate.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s5] Re-associating or re-casting the loop bound, the body condition, the D_800A3560 index or the prim.mode expression moves the score off 53 on the for-loop chassis.
- mechanism: Exhaustive spelling sweep of the four expressions carrying the residual: 7 bound forms x 3 condition forms x 2 index forms x 2 mode forms.
- probe: 84 variants generated into tmp/grind/func_80070C70/s5/bound/ and swept with s5/sweep.py; log s5/bound.log.
- result: Histogram {53: 36, 57: 36, 58: 12}. The only live axis is the bound's redundant (s32) cast: every uncast form is 53, every (s32)(...) form is 57. Condition operand order, var_s0 * 3 versus 3 * var_s0, and 0x50 + var_s0 * 0x16C versus var_s0 * 0x16C + 0x50 are all byte-neutral. Banked as rejected/s32-cast-loop-bound-57.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-53 for-loop chassis (s5 candidate.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s5] The 172-spelling naming and declaration-order enumeration of the pre-loop block takes the floor-56 do/while chassis below 55.
- mechanism: Mandated enumerate modality: the block from var_s0 = 0 through the loop-entry if was written in fully-named form (dst, geom, base, shown, limit) and swept over tools/spelling_enum.py's keep-versus-inline and declaration-order axes.
- probe: tmp/grind/func_80070C70/s5/enum_region1.c -> s5/enum1/ (172 variants) swept with s5/sweep.py; log s5/enum1.log.
- result: Histogram {55: 1, 58: 2, 59: 9, 60: 28, 63: 26, 64: 106}; best 55 against the 56 baseline. The whole naming/ordering space of that block on the do/while chassis is worth one point, which is what redirected this session at the loop's structure.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-56 chassis (s4 candidate.c body + extern u8 D_800A3560[]; + the RecC70 record declaration of D_800A3590 + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s5] Hoisting the loop-body block-scoped locals (u8 code, s32 t) to function scope, or inlining t, removes the 0x18 frame overshoot on the for-loop chassis.
- mechanism: The build's frame is 0x98 against the target's 0x80. The extra 0x18 is three pseudos (118, 168, 171) that appear only as (use (reg)) insns created between the .flow and .lreg dumps, carry no conflicts in .greg, receive no hard register, and take 8-byte stack slots at sp+104/112/120.
- probe: tmp/grind/func_80070C70/s5/frame/ U1 (hoist t), U2 (hoist code), U3 (hoist both), U4 (inline t), scored with sandbox --disable all.
- result: 57 / 57 / 57 / 59 against the 57 baseline: block scope is byte-neutral and inlining t is worse. The three dead pseudos are not the block-scoped locals; they are the leftovers of duplicate_loop_exit_test's reg_map copies (jump.c:2246).
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-57 for-loop chassis (S4_for_arr_u16 body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + extern u16 D_800A3558; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s6] The 0x18 frame overshoot on the s5 for-loop chassis is three reload spill slots for pseudos that carry a reference but no set, created by jump.c's duplicate_loop_exit_test and materialised by combine.c's USE-at-label handling of orphaned REG_DEAD notes.
- mechanism: jump.c:2246 (`reg_map[REGNO (reg)] = gen_reg_rtx (GET_MODE (reg))`) inside duplicate_loop_exit_test gives a NEW pseudo to every exit-test insn whose destination pseudo has its first AND last uid inside the exit code. combine then folds/deletes the copies it can (the guard block's load + sign-extend pair collapses to a single `lh`), and combine.c:10839 parks each orphaned REG_DEAD note on a fresh `(use (reg))` insn at the following CODE_LABEL. Those pseudos have REG_N_REFS 1 and no set, win no hard register in .greg, and reload's alter_reg assigns each an 8-byte stack slot.
- probe: (a) instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_FRAME_DEBUG=1 via tmp/grind/func_80070C70/s6/framedbg.sh, which prints the whole assign_stack_local census with a ctx tag per slot; (b) pass-by-pass grep of tmp/grind/func_80070C70/dumps/text1b.{rtl,jump,cse,loop,combine,lreg,greg} for the pseudo numbers and for `(use (reg:SI N))`.
- result: census on the for chassis is stack_temp 48 (prim) + stack_temp 32 (icon) + spill_new_p116 8 + spill_new_p165 8 + spill_new_p170 8 = 104 = the `vars= 104` cc1 reports, against the target's `vars= 80`. Regs 165/170 are absent from .rtl and present from .jump onward; the `(use (reg))` insns first appear in .combine. On the if-guarded do/while chassis the census stops after icon: vars= 80, frame 128, byte-identical prologue and epilogue.
- verdict: CONFIRMED

## [s6] Re-declaring D_800A3558, re-associating the loop bound, or re-spelling the body condition removes the three spill slots on the top-test for chassis.
- mechanism: exhaustive spelling sweep of the two expressions that make up the loop-exit test, measured on the `vars=` gradient (cc1's own .frame comment) rather than the sandbox score, so the frame is isolated from the rest of the codegen.
- probe: tmp/grind/func_80070C70/s6/bc/ (10 bound forms x 4 condition forms = 40 variants) and tmp/grind/func_80070C70/s6/k/ (3 declarations x 3 bounds x 3 conditions = 27 variants), swept with tmp/grind/func_80070C70/s6/varsweep.sh.
- result: all 27 of the k sweep are vars= 104. In the bc sweep every `<` form is 104 and every `<=` / `>=` form is 96 (one spill slot fewer, because the `+ 1` intermediate disappears from the exit test) -- but the `<=` forms score 79 on the sandbox, far worse than 49. No spelling of the bound, the condition or the declaration reaches vars= 80 while the loop keeps its test at the top.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-53/49 top-test for chassis (s5 candidate.c body plus the s6 `s32 ctx = var_s0 * 3;` named index + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, cc1 vars= gradient and sandbox --disable all, 2026-09-10

## [s6] Naming the D_800A3560 index in a fresh once-written/once-read local (`s32 ctx = var_s0 * 3;`) makes loop.c's strength_reduce reduce that giv to a byte OFFSET instead of a full ADDRESS, which is the target's `lui $at,%hi / addu $at,$at,$s2 / lbu %lo($at)` plus `addiu $s2,$s2,3` form.
- mechanism: with the multiply inlined into the array reference, expand_expr builds the whole address `symbol + i*3` as one expression and loop.c reduces the ADDRESS giv, emitting `lui $s2 / addiu $s2 / lbu 0($s2)`. Naming the product makes `ctx` itself the giv that strength_reduce reduces, leaving the symbol to be re-added by legitimize_address at each reference.
- probe: tmp/grind/func_80070C70/s6/struct/ -- 5 index spellings (inlined index, named local, comma-initialised second biv, pointer-arithmetic deref, pointer local) x 2 secondary-counter spellings x 3 declarations of D_800A3558 = 30 variants, scored with sandbox --disable all.
- result: {49: 6, 52: 6, 53: 6, 56: 6, 88: 3, 90: 3}. Named local (i2) and pointer local (i5) are 49 at 194 insns (the target's instruction count) against 53 at 193 for both inlined spellings; the comma-initialised second biv is 88 at 196 insns. This closes the s4/s5 frontier item "the target reduces D_800A3560's giv to a byte OFFSET where we reduce it to a full ADDRESS".
- verdict: CONFIRMED

## [s6] The second loop must be an if-guarded do/while, not the s5 top-test for: it is worth 10 points because it removes the three spill slots, at the cost of the tail-load CSE that only a top-test loop can reach.
- mechanism: cse.c:7909 cse_set_around_loop is the only transform that can move a loop-head memory read into the previous iteration's tail, and its gate at cse.c:7936 is `REG_LOOP_TEST_P (src_elt->exp)`; REG_LOOP_TEST_P is set in exactly one place in GCC 2.7.2, jump.c:2253, inside duplicate_loop_exit_test, which requires a NOTE_INSN_LOOP_BEG followed by an unconditional jump -- a top-test loop. The same transform is what creates the pseudos that become the three spill slots.
- probe: tmp/grind/func_80070C70/s6/g/ -- 8 loop-structure variants (top-test for, while, for(;;)+break, for+break, `<=`, `!(>)`, reversed `>`, if-guarded do/while) on the s6 named-index chassis, scored on both the vars= gradient and sandbox --disable all; then refined with the 27-variant s6/h/ declaration x bound x condition sweep and the 10-variant s6/l/ body sweep.
- result: if-guarded do/while = vars 80 (frame 128, prologue and epilogue byte-identical to the target) and score 43, then 40 with the reassociated body condition and 39 with the `||` operands swapped, at 194 insns == the target. Every top-test form = vars 104 (frame 152) and score 49 at best. for(;;)+break and for+break = 62; `<=` and `!(>)` = 79. s5's structural conclusion is therefore SUPERSEDED: the `for` chassis was a local optimum that was paying 14 prologue/epilogue instructions for 4 body instructions.
- verdict: CONFIRMED

## [s6] Body-level spelling of the second loop (the p_static addend order, the mode expression order, the index product order, hoisting `ctx` to function scope, inlining `t`) moves the do/while chassis below 39.
- mechanism: mandated enumerate modality -- 10 body spellings on the 40-point do/while chassis, one axis at a time.
- probe: tmp/grind/func_80070C70/s6/l/ swept with tmp/grind/func_80070C70/s6/sweep.py.
- result: histogram {39: 1, 40: 8, 43: 1}. The only live axis is the `||` operand order in the mode test: `(D_800A35BC == 2) || (((s16)D_800A3558 + D_800A35B0) != 0)` is 39 at 194 insns, the other order 40 at 190. `(D_800A3590[var_s0] << 4) + t` instead of `t + (...)` is 43. `3 * var_s0`, `var_s0 * 0x16C + 0x50`, `D_800A3590[var_s0] * 16`, hoisting `ctx` to function scope and re-ordering `t`'s two stores are all byte-neutral at 40.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-40 if-guarded do/while chassis (s6 candidate.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + extern s32 D_800A3558; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s6] The 0x18 frame overshoot on the s5 for-loop chassis is three reload spill slots for pseudos that carry a reference but no set, created by jump.c's duplicate_loop_exit_test and materialised by combine.c's USE-at-label handling of orphaned REG_DEAD notes.
- mechanism: jump.c:2246 (reg_map[REGNO(reg)] = gen_reg_rtx(GET_MODE(reg))) inside duplicate_loop_exit_test gives a NEW pseudo to every exit-test insn whose destination pseudo has its first AND last uid inside the exit code. combine folds and deletes the copies it can (the guard block's load + sign-extend pair collapses to one lh) and combine.c:10839 parks each orphaned REG_DEAD note on a fresh (use (reg)) insn at the following CODE_LABEL. Those pseudos have REG_N_REFS 1 and no set, win no hard register in .greg, and reload's alter_reg gives each an 8-byte stack slot.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_FRAME_DEBUG=1 via tmp/grind/func_80070C70/s6/framedbg.sh, printing the full assign_stack_local census with a ctx tag per slot; plus a pass-by-pass grep of tmp/grind/func_80070C70/dumps/text1b.{rtl,jump,cse,loop,combine,lreg,greg} for the pseudo numbers and for (use (reg:SI N)).
- result: Census on the for chassis: stack_temp 48 (prim) + stack_temp 32 (icon) + spill_new_p116 8 + spill_new_p165 8 + spill_new_p170 8 = 104, matching cc1's own 'vars= 104' against the target's 'vars= 80'. Regs 165/170 are absent from .rtl and present from .jump onward; the (use (reg)) insns first appear in .combine. On the if-guarded do/while chassis the census stops after icon: vars= 80, frame 128, prologue and epilogue byte-identical to the target. This also independently proves IconC70's 0x20 size (48 + 32 = 80 exactly, nothing left over).
- verdict: CONFIRMED

## [s6] Naming the D_800A3560 index in a fresh once-written/once-read local (s32 ctx = var_s0 * 3;) makes loop.c's strength_reduce reduce that giv to a byte OFFSET instead of a full ADDRESS, reproducing the target's lui $at,%hi / addu $at,$at,$s2 / lbu %lo($at) plus addiu $s2,$s2,3.
- mechanism: With the multiply inlined into the array reference, expand_expr builds the whole address symbol + i*3 as one expression and loop.c reduces the ADDRESS giv (lui $s2 / addiu $s2 / lbu 0($s2)). Naming the product makes ctx itself the giv that strength_reduce reduces, leaving the symbol to be re-added by legitimize_address at each reference.
- probe: tmp/grind/func_80070C70/s6/struct/ -- 5 index spellings (inlined index, named local, comma-initialised second biv, pointer-arithmetic deref, pointer local) x 2 secondary-counter spellings x 3 declarations of D_800A3558 = 30 variants, scored with sandbox --disable all.
- result: {49: 6, 52: 6, 53: 6, 56: 6, 88: 3, 90: 3}. Named local and pointer local are 49 at 194 insns (the target's count) against 53 at 193 for both inlined spellings; a comma-initialised second biv is 88 at 196. Closes the s4/s5 frontier item about the D_800A3560 giv. NOTE: ctx is a NAMED INTERMEDIATE and must be vetted against that family's 6 prongs before any candidate-ready.
- verdict: CONFIRMED

## [s6] The second loop must be an if-guarded do/while rather than the s5 top-test for: it is worth 10 points because it removes the three spill slots, at the cost of the tail-load CSE that only a top-test loop can reach.
- mechanism: cse.c:7909 cse_set_around_loop is the only transform that can move a loop-head memory read into the previous iteration's tail, and its gate at cse.c:7936 is REG_LOOP_TEST_P (src_elt->exp). REG_LOOP_TEST_P is set in exactly one place in GCC 2.7.2 -- jump.c:2253, inside duplicate_loop_exit_test -- which requires a NOTE_INSN_LOOP_BEG followed by an unconditional jump, i.e. a top-test loop. The same transform creates the pseudos that become the three spill slots, so the two target features are coupled through one pass.
- probe: tmp/grind/func_80070C70/s6/g/ -- 8 loop-structure variants scored on both the cc1 vars= gradient and sandbox --disable all; refined by the 27-variant s6/h/ declaration x bound x condition sweep and the 10-variant s6/l/ body sweep.
- result: if-guarded do/while = vars 80 (frame 128, prologue and epilogue byte-identical) and score 43, then 40 with the reassociated body condition and 39 with the || operands swapped, at 194 insns == the target. Every top-test form = vars 104 (frame 152) and 49 at best; for(;;)+break and for+break = 62; <= and !(>) = 79. s5's structural conclusion is superseded: the for chassis was a local optimum paying 14 prologue/epilogue instructions for 4 body instructions.
- verdict: CONFIRMED

## [s6] Re-declaring D_800A3558, re-associating the loop bound, or re-spelling the body condition removes the three spill slots while the loop keeps its test at the top.
- mechanism: Exhaustive spelling sweep of the two expressions that make up the loop-exit test, measured on the vars= gradient (cc1's own .frame comment) so the frame is isolated from the rest of the codegen.
- probe: tmp/grind/func_80070C70/s6/bc/ (10 bound forms x 4 condition forms = 40 variants) and tmp/grind/func_80070C70/s6/k/ (3 declarations x 3 bounds x 3 conditions = 27 variants), swept with tmp/grind/func_80070C70/s6/varsweep.sh.
- result: All 27 of the k sweep are vars= 104. In the bc sweep every < form is 104 and every <= / >= form is 96 (one spill slot fewer, because the + 1 intermediate leaves the exit test) -- but those <= forms score 79 on the sandbox against 49. Nothing in that space reaches vars= 80.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-53/49 top-test for chassis (s5 candidate.c body plus the s6 `s32 ctx = var_s0 * 3;` named index + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, cc1 vars= gradient and sandbox --disable all, 2026-09-10

## [s6] Body-level spelling of the second loop (the p_static addend order, the mode expression order, the index product order, hoisting ctx to function scope, re-ordering t's two stores) moves the do/while chassis below 39.
- mechanism: Mandated enumerate modality -- 10 body spellings on the 40-point do/while chassis, one axis at a time.
- probe: tmp/grind/func_80070C70/s6/l/ swept with tmp/grind/func_80070C70/s6/sweep.py.
- result: Histogram {39: 1, 40: 8, 43: 1}. The only live axis is the || operand order in the mode test: (D_800A35BC == 2) || (((s16)D_800A3558 + D_800A35B0) != 0) is 39 at 194 insns, the other order 40 at 190. (D_800A3590[var_s0] << 4) + t instead of t + (...) is 43. 3 * var_s0, var_s0 * 0x16C + 0x50, D_800A3590[var_s0] * 16, hoisting ctx to function scope and re-ordering t's two stores are all byte-neutral at 40.
- verdict: KILLED
- kill_scope: instance
- measured_on: floor-40 if-guarded do/while chassis (s6 candidate.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + extern s32 D_800A3558; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s6] The declared type of D_800A3558 (extern s32 with an (s16) cast, extern u16, extern s16) changes the score on either chassis.
- mechanism: The target's guard block loads the symbol twice, once as lhu (kept live and sign-extended in the loop body by sll 16 / sra 16) and once as lh (folded by combine inside the guard block). The hypothesis was that a halfword declaration is what produces the lhu.
- probe: All three declarations crossed with 3 bound forms x 3 condition forms on both chassis: tmp/grind/func_80070C70/s6/h/ (27 variants, sandbox) and tmp/grind/func_80070C70/s6/k/ (27 variants, vars= gradient), plus the 30-variant s6/struct/ sweep.
- result: Byte-neutral everywhere: all 9 bound x condition pairs score identically for d32 / du16 / ds16 on the do/while chassis (three-way ties at 40/41/42/43/44/45), all 27 k variants are vars= 104 on the for chassis, and the struct sweep's 10 triples are three-way ties. The lhu does not come from the declaration. The ordinary extern s32 D_800A3558; is kept.
- verdict: KILLED
- kill_scope: instance
- measured_on: both the floor-49 top-test for chassis and the floor-40 if-guarded do/while chassis (s6 candidate.c bodies + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all and the cc1 vars= gradient, 2026-09-10

---

# Session 7 (synthesis, 2026-09-10) - floor held at 39; the CHASSIS QUESTION IS REOPENED

Chassis re-measured at dispatch: candidate.c + the three companion edits = 39 at 194 insns,
identical to s6's record. Nothing in the ledger was stale.

## THE MERGED PICTURE (this is the synthesis; read this before anything else)

The target's second loop is a TOP-TEST loop on which jump.c's duplicate_loop_exit_test fired.
Proof from the target's own bytes, not from inference: the guard block at 80070DF4-80070E18
(`lhu $a2` / `lw $a1` / `lh $v0` / `addiu $v0,$v0,1` / `addu $v0,$a1,$v0` / `blez`) is an
insn-for-insn COPY of the loop tail test at 80070ECC-80070EE4
(`lhu $a2` / `lh $v0` / `lw $a1` / `addiu $v0,$v0,1` / `addu $v0,$a1,$v0` / `slt` / `bnez`),
with the counter folded to zero so the `slt`+`bnez` collapses to `blez`. That is exactly what
duplicate_loop_exit_test emits. The `lhu $a2` in BOTH blocks is used by neither test - it is
the loop-BODY's read of D_800A3558 (consumed at 80070E78 by `sll $v0,$a2,16 / sra / addu
$v0,$a1,$v0`), placed there by cse.c's cse_set_around_loop, whose gate REG_LOOP_TEST_P is set
only inside duplicate_loop_exit_test. So the target's two headline features - the exact 0x80
frame AND the loop-carried $a1/$a2 - COEXIST on a top-test chassis. s6's conclusion that they
are mutually exclusive was drawn from our builds, not from the target, and is wrong.

## CONFIRMED

### H11 - cse_set_around_loop FIRES on a top-test spelling of the CURRENT (s6) body, and reproduces the target's loop-carried register structure exactly
**Mechanism:** cse.c:7909 cse_set_around_loop, reached from cse.c:8581 after the basic block
ending at NOTE_INSN_LOOP_END; it rewrites a loop-head SET whose SET_SRC is a MEM already in the
hash table as equal to a REG_LOOP_TEST_P register, and inserts a copy after the matching
pre-loop load. REG_LOOP_TEST_P is set at jump.c:2253 inside duplicate_loop_exit_test.
**Probe:** rewrote candidate.c's if-guarded do/while as
`for (var_s0 = 0; var_s0 < D_800A35B0 + (s16)D_800A3558 + 1; var_s0++)` with the body byte-for-byte
unchanged (tmp/grind/func_80070C70/s7/v/toptest.c), measured, then dumped with
`pwsh tools/grinder/dump.ps1 func_80070C70` and read tmp/grind/func_80070C70/dumps/text1b.s.
**Result:** the emitted loop is the target's shape. Guard: `lw $5,D_800A3558 / lw $6,D_800A35B0
/ lh $2,D_800A3558 / addu / addu 1 / blez`. Body: NO reload of either symbol - it consumes the
carried registers with `sll $2,$5,16 / sra $2,$2,16 / addu $2,$2,$6`, which is the target's
80070E78-80070E80 verbatim modulo register names. Tail: `lw $6 / lh $2 / lw $5 / addu / addu 1 /
slt / bnez`, the target's 80070ECC-80070EE4 verbatim modulo `lhu`-vs-`lw`. This closes s6's
frontier item (i) as a MECHANISM question: the transform is reachable and we have it.

### H12 - the entire remaining cost of the top-test chassis is the 24 extra frame bytes
**Mechanism:** score accounting, not inference. Aligned instruction diff of the best top-test
variant (d32_b1_cy, 49 at 194 insns) against the target via tmp/grind/func_80070C70/s7/sbs.py.
**Result:** 14 of the diffs are the six `sw`, six `lw`, `addiu $sp,$sp,-152` and `addiu
$sp,$sp,152` prologue/epilogue offsets - pure consequences of vars=104 vs the target's vars=80.
The rest is ~4 register-seat diffs in the body (a0/a2 vs a1/a2 on the carried pair, v0/v1 on the
`t = prim.p_geom + 0xC` pair) plus the two known scheduling ties. On the do/while chassis at 39
the frame is already exact but the ENTIRE carried-register structure is missing (T96-T157 in the
same diff). So the top-test chassis is 24 frame bytes away from being far better than 39, and the
do/while chassis is a whole pass away.

### H13 - the three spill slots on the top-test chassis are pseudos 116, 165 and 170, and they are named
**Mechanism:** duplicate_loop_exit_test remaps every exit-test pseudo whose first and last uid are
both inside the exit code (jump.c:2229-2253) to a fresh pseudo in the copied guard; combine then
folds the copies it can and parks the orphaned REG_DEAD notes on `(use (reg))` insns; reload's
alter_reg gives each refs-but-never-set pseudo an 8-byte slot.
**Probe:** BB2_FRAME_DEBUG census via tmp/grind/func_80070C70/s7/framedbg.sh + fd.py, plus a
targeted read of the func_80070C70 region of text1b.jump and text1b.combine.
**Result:** census is stack_temp 48 (prim) + stack_temp 32 (icon) + spill_new_p116 +
spill_new_p165 + spill_new_p170 = 104. In .combine the three orphans are literally
`(insn 541 (use (reg:SI 170)))`, `(insn 540 (use (reg:SI 165)))`, `(insn 542 (use (reg/s:SI 116)))`.
In .jump the copied guard block is insns 478-485: 164 = `(mem:SI D_800A3558)`,
165 = `(ashift (164) 16)`, 166 = `(ashiftrt (165) 16)` (REG_EQUAL sign_extend), 167 =
`(mem:SI D_800A35B0)`, 168 = `(plus 167 166)`, 169 = `(plus 168 1)`, 170 = `(lt reg75 169)`.
combine folds 164/165/166 into one `lh` (orphaning 165) and folds the `lt` into `blez`
(orphaning 170). 116 is a `reg/s` pointer pseudo from the pre-loop region. The target's guard
keeps six insns and no orphan, so in the target either these pseudos were never remapped (they
had a use outside the exit code at jump time) or combine did not delete their copies.

## KILLED

### K8 - the declared type of D_800A3558 stops the ashift/ashiftrt fold that orphans pseudo 165
**Probe:** all four declarations (`extern s32` + `(s16)` cast, `extern u16` + `(s16)` cast,
`extern s16` bare, `extern s16` + `(s16)(u16)` double cast) crossed with 2 bound associations and
2 `||` orders = 16 variants on the TOP-TEST chassis
(tmp/grind/func_80070C70/s7/w/, swept by s7/sweep.py), plus a BB2_FRAME_DEBUG census on the s16
variant.
**Result:** every declaration is byte-neutral at each (bound, condition) point: 49/49/49/49 at
b1_cy, 51/52/52/52 at b1_cx, 57 at b2_cx, 54 at b2_cy. The s16 census is spill_new_p116 +
spill_new_p165 + spill_new_p170, identical pseudo numbers and identical vars=104. The RTL the
front end builds for this expression does not depend on the declared type, which is the same
finding K1/K7/s6 recorded on three earlier chassis - now re-confirmed on the chassis where
cse_set_around_loop fires. KILLED (instance).

### K9 - parenthesising the loop bound as `D_800A35B0 + (<read> + 1)` reproduces the target's `addiu $v0,$v0,1` before the `addu`
**Probe:** the b2 half of the 16-variant s7/w sweep (the target's tail does
`lh $v0 / addiu $v0,$v0,1 / addu $v0,$a1,$v0`, ours does `addu / addu 1`).
**Result:** every b2 variant is WORSE - 57 at cx and 54 at cy against 51 and 49 - and adds an
insn (195 vs 194). The re-association buys the right two insns and loses more elsewhere.
KILLED (instance). Banked as rejected/toptest-bound-parenthesised-plus1-54.c.

### K10 - the `||` operand order that wins on the do/while chassis also wins on the top-test chassis
**Probe:** the cx/cy halves of the same sweep. cx = `(D_800A35BC == 2) || ((read + D_800A35B0) != 0)`
(the s6 winner, 39 on do/while); cy = the operands swapped, which is the target's emission order
(80070E78 tests the sum first, 80070E8C tests D_800A35BC second).
**Result:** on the top-test chassis cy beats cx everywhere: 49 vs 51-52, and cy is the variant that
reaches 194 insns while cx is 193. The winning `||` order is CHASSIS-DEPENDENT. KILLED (instance):
the s6 conclusion that cx is the better order does not transfer.

## OPEN FRONTIER (replaces s6's; s6 F(i) is now a solved mechanism, not a question)

### F7 - kill the three orphaned `(use (reg))` pseudos on the top-test chassis
This is now the WHOLE function. 116/165/170 are named and their RTL is in H13. Concrete probes,
cheapest first: (a) give pseudo 170 a use outside the exit code - the remap at jump.c:2244 only
happens when `regno_last_uid[REGNO(reg)]` lies inside the exit code, so any exit-test sub-value
that the loop body also reads is left alone; (b) shrink the exit test so combine has nothing to
fold: a bound with no `+1` (`var_s0 <= D_800A35B0 + (s16)D_800A3558`) removes pseudo 169 - s6
measured `<=` at 79 on the OLD chassis, which is void here, so re-measure it on the top-test cse
chassis; (c) find out what pseudo 116 is (a `reg/s` from the PRE-loop region - it may be the
`prim.p_geom + 0xC` pointer and therefore killable independently of the loop); (d) read
tools/gcc-2.7.2/reload1.c alter_reg to confirm a refs-but-no-set pseudo cannot be given a hard
register by any C-level change, before spending more on (a)/(b).

### F8 - the `t = prim.p_geom + 0xC` two-live-register site (carried, unchanged)
Target emits `addiu $v1,$v0,0xC` (two live registers) at three sites where we emit
`addiu $v0,$v0,0xC` (coalesced). At the first site the target's ORDER is the tell: it computes
the add BEFORE storing prim.p_geom (`lw $v0,4($s2) / addiu $v1,$v0,12 / sw $v0,0x18($sp) /
sw $v1,0x1C($sp)`), so the p_geom value is still live at the add; we store first, so the value
dies at the add and local-alloc coalesces. Probe: spell the pre-loop block so the p_static value
is computed before the p_geom store (`s32 g = *(s32 *)(ctx + 4); prim.p_static = g + 0xC;
prim.p_geom = g;`) and see whether the scheduler puts the two stores back in the target's order.

### F9 - IconC70's real tail layout (carried from s1/s2/s3, still unspent)
`s16 sp50[12]` is a placeholder; the 0x20 SIZE is proven twice over (frame arithmetic + the
BB2_FRAME_DEBUG census). Recover the real fields from func_80069898's other callers
(func_8006B120, func_8006CFBC, func_800720FC, func_80074488, func_8006F97C) before submission.

## [s7] The target's second loop is a TOP-TEST loop on which jump.c's duplicate_loop_exit_test fired, and its guard block is an insn-for-insn copy of its tail test.
- mechanism: duplicate_loop_exit_test (jump.c:2161) copies the exit test in front of the loop; the copy's counter is the initial value so the `slt`+`bnez` folds to `blez`. The `lhu $a2` present in BOTH blocks is used by neither test - it is the loop body's read of D_800A3558, placed there by cse.c:7909 cse_set_around_loop, whose only gate (cse.c:7936 REG_LOOP_TEST_P) is set only at jump.c:2253 inside duplicate_loop_exit_test.
- probe: Read asm/funcs/func_80070C70.s 80070DF0-80070EE8 directly and matched the guard block (lhu $a2 / lw $a1 / lh $v0 / addiu +1 / addu / blez) against the tail test (lhu $a2 / lh $v0 / lw $a1 / addiu +1 / addu / slt / bnez), then cross-checked the body's consumer at 80070E78-80070E80 (sll $v0,$a2,16 / sra / addu $v0,$a1,$v0).
- result: The two blocks are the same six insns. The target therefore has the exact 0x80 frame AND the loop-carried $a1/$a2 on ONE chassis, so s6's structural conclusion that the frame and the tail-load CSE are mutually exclusive is a property of our builds, not of the compiler. The chassis question is reopened in favour of the top-test form.
- verdict: CONFIRMED

## [s7] Spelling the second loop as a top-test `for` on the CURRENT s6 body makes cse_set_around_loop fire and reproduces the target's loop-carried register structure exactly.
- mechanism: cse.c:8581 calls cse_around_loop after the block ending at NOTE_INSN_LOOP_END; cse_set_around_loop (cse.c:7909) rewrites a loop-head SET whose SET_SRC is a MEM equal in the hash table to a REG_LOOP_TEST_P register, and inserts a copy after the matching pre-loop load.
- probe: tmp/grind/func_80070C70/s7/v/toptest.c - candidate.c's if-guarded do/while rewritten as `for (var_s0 = 0; var_s0 < D_800A35B0 + (s16)D_800A3558 + 1; var_s0++)` with the body unchanged; measured with sandbox --disable all and dumped with pwsh tools/grinder/dump.ps1 func_80070C70.
- result: 51 at 193 insns, and the emitted code is the target's structure: guard `lw $5,D_800A3558 / lw $6,D_800A35B0 / lh $2,D_800A3558 / addu / addu 1 / blez`; body with NO reload, consuming `sll $2,$5,16 / sra / addu $2,$2,$6`; tail `lw $6 / lh $2 / lw $5 / addu / addu 1 / slt / bnez`. The best variant of the family is 49 at 194 insns (d32_b1_cy, banked as memory/grind/func_80070C70/chassis-toptest-cse-49.c).
- verdict: CONFIRMED

## [s7] The entire remaining cost of the top-test chassis over the target is the 24 extra frame bytes.
- mechanism: aligned instruction-level diff, not inference.
- probe: tmp/grind/func_80070C70/s7/sbs.py (engine.score.normalized_insns + difflib) on the 49-point top-test variant against build/src/text1b.o.
- result: 14 of the diffs are the six callee-saved `sw`, six `lw`, and the two `addiu $sp` insns - all pure vars=104-vs-80 offset differences. The remainder is ~4 register-seat diffs (a0/a2 vs a1/a2 on the carried pair, v0/v1 on the `prim.p_geom + 0xC` pair) plus the two known scheduling ties. The same diff on the do/while 39 chassis shows the frame already exact but the whole carried-register structure (target T96-T157) missing.
- verdict: CONFIRMED

## [s7] The three spill slots on the top-test chassis are pseudos 116, 165 and 170, orphaned by combine after duplicate_loop_exit_test remapped them.
- mechanism: jump.c:2229-2253 remaps every exit-test pseudo whose first and last uid are inside the exit code; combine folds what it can and parks the orphaned REG_DEAD notes on `(use (reg))` insns; reload's alter_reg gives each refs-but-no-set pseudo an 8-byte slot.
- probe: BB2_FRAME_DEBUG census (tmp/grind/func_80070C70/s7/framedbg.sh + fd.py) plus a targeted read of the func_80070C70 region of tmp/grind/func_80070C70/dumps/text1b.jump and .combine.
- result: census = stack_temp 48 + stack_temp 32 + spill_new_p116 + spill_new_p165 + spill_new_p170 = vars 104. combine holds `(insn 541 (use (reg:SI 170)))`, `(insn 540 (use (reg:SI 165)))`, `(insn 542 (use (reg/s:SI 116)))`. jump's copied guard is insns 478-485: 164 = (mem:SI D_800A3558), 165 = (ashift 164 16), 166 = (ashiftrt 165 16) with a REG_EQUAL sign_extend, 167 = (mem:SI D_800A35B0), 168 = (plus 167 166), 169 = (plus 168 1), 170 = (lt reg75 169). combine folds 164/165/166 into one `lh` (orphaning 165) and folds the `lt` into `blez` (orphaning 170).
- verdict: CONFIRMED

## [s7] The declared type of D_800A3558 stops the ashift/ashiftrt fold that orphans pseudo 165 on the top-test chassis where cse_set_around_loop fires.
- mechanism: the fold that orphans 165 is combine collapsing (mem:SI) + ashift + ashiftrt into one sign-extending halfword load; a halfword declaration should emit the sign_extend directly and leave nothing to fold.
- probe: four declarations (extern s32 with an (s16) cast, extern u16 with an (s16) cast, extern s16 bare, extern s16 with an (s16)(u16) double cast) crossed with 2 bound associations and 2 `||` orders = 16 variants in tmp/grind/func_80070C70/s7/w/, swept by tmp/grind/func_80070C70/s7/sweep.py; plus a BB2_FRAME_DEBUG census on the s16 variant.
- result: byte-neutral at every point - 49/49/49/49 at b1_cy, 51/52/52/52 at b1_cx, 57x4 at b2_cx, 54x4 at b2_cy - and the s16 census is the identical spill_new_p116 / p165 / p170 triple at vars=104. This is the fourth chassis on which the declaration has measured neutral (K1 s2, K7 s4, s6, s7).
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (s7 toptest.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all + BB2_FRAME_DEBUG census, 2026-09-10

## [s7] Parenthesising the loop bound as `D_800A35B0 + ((s16)D_800A3558 + 1)` reproduces the target's `addiu $v0,$v0,1`-before-`addu` tail order on the top-test chassis.
- mechanism: the target's tail is `lh $v0 / addiu $v0,$v0,1 / addu $v0,$a1,$v0` (the +1 binds to the loaded halfword); ours is `addu / addu 1` (left-associated).
- probe: the b2 half of the 16-variant tmp/grind/func_80070C70/s7/w sweep.
- result: every b2 variant is worse and one insn longer - 57 at cx and 54 at cy against 51 and 49 at b1, at 194/195 insns. Banked as rejected/toptest-bound-parenthesised-plus1-54.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (s7 toptest.c body, all four D_800A3558 declarations, + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s7] The `||` operand order that wins on the if-guarded do/while chassis (D_800A35BC first) also wins on the top-test chassis.
- mechanism: s6 measured `(D_800A35BC == 2) || ((read + D_800A35B0) != 0)` as the single live body axis, worth 1 point (40 -> 39) on the do/while chassis. The target's emission order is the opposite: it tests the sum first at 80070E78 and D_800A35BC second at 80070E8C.
- probe: the cx/cy halves of the 16-variant tmp/grind/func_80070C70/s7/w sweep.
- result: reversed. On the top-test chassis the target's order (cy, sum first) wins everywhere: 49 vs 51-52 at b1 and 54 vs 57 at b2, and cy is the half that reaches the target's 194 insns while cx sits at 193. The winning operand order is chassis-dependent, so s6's body-spelling histogram must be re-run if the chassis moves.
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (s7 toptest.c body, all four D_800A3558 declarations x 2 bound associations, + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s7] The target's second loop is a top-test loop on which jump.c's duplicate_loop_exit_test fired: its guard block at 80070DF4-80070E18 is an insn-for-insn copy of its loop tail test at 80070ECC-80070EE4, and the `lhu $a2` present in both blocks is used by neither test but by the loop body at 80070E78, i.e. it was placed there by cse.c's cse_set_around_loop.
- mechanism: duplicate_loop_exit_test (jump.c:2161) copies the exit test in front of the loop with the counter at its initial value, so the copy's slt+bnez folds to blez. cse_set_around_loop (cse.c:7909) rewrites a loop-head SET whose SET_SRC is a MEM equal in the hash table to a REG_LOOP_TEST_P register; REG_LOOP_TEST_P is set only at jump.c:2253 inside duplicate_loop_exit_test. The two target features s6 believed were mutually exclusive (the exact 0x80 frame and the loop-carried $a1/$a2) therefore coexist on one chassis in the target itself.
- probe: Direct read of asm/funcs/func_80070C70.s 80070DF0-80070EE8, matching the guard block (lhu $a2 / lw $a1 / lh $v0 / addiu +1 / addu / blez) against the tail test (lhu $a2 / lh $v0 / lw $a1 / addiu +1 / addu / slt / bnez) and the body consumer (sll $v0,$a2,16 / sra / addu $v0,$a1,$v0), cross-read against tools/gcc-2.7.2/jump.c:2161-2260 and cse.c:7730-7960, 8565-8585.
- result: The guard and the tail are the same six insns. s6's structural conclusion that the frame and the tail-load CSE are mutually exclusive is a property of our builds, not of the compiler, and the top-test chassis is back in play.
- verdict: CONFIRMED

## [s7] Spelling the second loop as a top-test `for` on the CURRENT (s6) body makes cse_set_around_loop fire on our build and reproduces the target's loop-carried register structure: guard loads, a body with no reload, and a tail that re-loads both symbols.
- mechanism: cse.c:8581 calls cse_around_loop after the basic block ending at NOTE_INSN_LOOP_END; cse_set_around_loop then replaces the loop-head read with the REG_LOOP_TEST_P register and inserts a copy after the matching pre-loop load.
- probe: tmp/grind/func_80070C70/s7/v/toptest.c (candidate.c's if-guarded do/while rewritten as `for (var_s0 = 0; var_s0 < D_800A35B0 + (s16)D_800A3558 + 1; var_s0++)`, body unchanged), measured with sandbox --disable all, then `pwsh tools/grinder/dump.ps1 func_80070C70` and a read of tmp/grind/func_80070C70/dumps/text1b.s.
- result: 51 at 193 insns; the emitted guard is `lw $5,D_800A3558 / lw $6,D_800A35B0 / lh $2,D_800A3558 / addu / addu 1 / blez`, the body carries no reload and consumes `sll $2,$5,16 / sra / addu $2,$2,$6` (the target's 80070E78-E80 verbatim modulo register names), and the tail is `lw $6 / lh $2 / lw $5 / addu / addu 1 / slt / bnez` (the target's 80070ECC-EE4 verbatim modulo lhu-vs-lw). Best variant of the family is 49 at 194 insns, banked as memory/grind/func_80070C70/chassis-toptest-cse-49.c.
- verdict: CONFIRMED

## [s7] The entire remaining cost of the top-test chassis over the target is the 24 extra frame bytes: 14 of its 49 diffs are prologue/epilogue offset differences caused by vars=104 vs the target's vars=80.
- mechanism: Score accounting from an aligned instruction diff, not inference. reload's alter_reg gives each refs-but-never-set pseudo an 8-byte stack slot, which moves every callee-saved save/restore offset and both `addiu $sp` insns.
- probe: tmp/grind/func_80070C70/s7/sbs.py (engine.score.normalized_insns + difflib) on the 49-point variant against build/src/text1b.o, and the same diff on the 39-point do/while chassis for comparison.
- result: Top-test chassis: 14 frame-offset diffs plus about four register-seat diffs (a0/a2 vs a1/a2 on the carried pair, v0/v1 on the prim.p_geom+0xC pair) plus the two known scheduling ties. do/while chassis at 39: frame exact, but the whole carried-register structure (target insns 96-157) missing. Killing the 24 bytes on the top-test chassis is worth more than any body spelling on the do/while chassis.
- verdict: CONFIRMED

## [s7] The three spill slots on the top-test chassis are pseudos 116, 165 and 170; they appear in .combine as literal `(use (reg))` insns, and their RTL origin is the copied guard block at .jump insns 478-485.
- mechanism: jump.c:2229-2253 remaps every exit-test pseudo whose regno_first_uid and regno_last_uid both lie inside the exit code to a fresh pseudo in the copied guard; combine folds what it can and parks the orphaned REG_DEAD notes on `(use (reg))` insns at the following CODE_LABEL; reload's alter_reg then gives each an 8-byte slot.
- probe: BB2_FRAME_DEBUG census via tmp/grind/func_80070C70/s7/framedbg.sh + fd.py, plus a targeted read of the func_80070C70 region of tmp/grind/func_80070C70/dumps/text1b.jump and text1b.combine.
- result: Census = stack_temp 48 (prim) + stack_temp 32 (icon) + spill_new_p116 + spill_new_p165 + spill_new_p170 = vars 104. combine holds `(insn 541 (use (reg:SI 170)))`, `(insn 540 (use (reg:SI 165)))`, `(insn 542 (use (reg/s:SI 116)))`. jump's copied guard is 164=(mem:SI D_800A3558), 165=(ashift 164 16), 166=(ashiftrt 165 16), 167=(mem:SI D_800A35B0), 168=(plus 167 166), 169=(plus 168 1), 170=(lt reg75 169); combine folds 164/165/166 into one lh (orphaning 165) and folds the lt into blez (orphaning 170).
- verdict: CONFIRMED

## [s7] Changing the declared type of D_800A3558 (extern s32 with an (s16) cast, extern u16 with an (s16) cast, extern s16 bare, extern s16 with an (s16)(u16) double cast) removes the ashift/ashiftrt fold that orphans pseudo 165, on the top-test chassis where cse_set_around_loop fires.
- mechanism: The fold that orphans 165 is combine collapsing (mem:SI) + ashift + ashiftrt into one sign-extending halfword load; a halfword declaration should emit the sign_extend directly and leave nothing for combine to delete.
- probe: Four declarations x 2 bound associations x 2 `||` orders = 16 variants in tmp/grind/func_80070C70/s7/w/, swept by tmp/grind/func_80070C70/s7/sweep.py with sandbox --disable all, plus a BB2_FRAME_DEBUG census on the s16 variant.
- result: Byte-neutral at every point: 49/49/49/49 at b1_cy, 51/52/52/52 at b1_cx, 57 four ways at b2_cx, 54 four ways at b2_cy. The s16 census is the identical spill_new_p116 / p165 / p170 triple at vars=104, so the RTL the front end builds does not depend on the declared type. This is the fourth distinct chassis on which this axis has measured dead (s2 K1, s4 K7, s6, s7). Banked as rejected/toptest-u16-decl-D_800A3558-byte-neutral-49.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (tmp/grind/func_80070C70/s7/v/toptest.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all plus the BB2_FRAME_DEBUG census, 2026-09-10

## [s7] Parenthesising the loop bound as `D_800A35B0 + ((s16)D_800A3558 + 1)` to reproduce the target's `addiu $v0,$v0,1`-before-`addu` tail order improves the top-test chassis.
- mechanism: The target's tail binds the +1 to the loaded halfword (lh $v0 / addiu $v0,$v0,1 / addu $v0,$a1,$v0); our left-associated bound emits addu then addu 1.
- probe: The b2 half of the 16-variant tmp/grind/func_80070C70/s7/w sweep.
- result: Every b2 variant is worse and one instruction longer: 57 at cx and 54 at cy against 51 and 49 at b1, at 194/195 insns against 193/194. The re-association buys the two right insns and loses more elsewhere. Banked as rejected/toptest-bound-parenthesised-plus1-54.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (s7 toptest.c body, all four D_800A3558 declarations, + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s7] The `||` operand order that wins on the if-guarded do/while chassis (D_800A35BC tested first, s6's 39) also wins on the top-test chassis.
- mechanism: s6 measured the operand order as the single live body axis on the do/while chassis, worth one point. The target's own emission order is the opposite: it tests the D_800A3558 sum first at 80070E78 and D_800A35BC second at 80070E8C.
- probe: The cx/cy halves of the 16-variant tmp/grind/func_80070C70/s7/w sweep.
- result: Reversed on this chassis. The target's order (cy, sum first) wins everywhere: 49 vs 51-52 at b1 and 54 vs 57 at b2, and cy is the half that reaches the target's 194 insns while cx sits at 193. The winning operand order is chassis-dependent, so s6's ten-variant body histogram must be re-run whenever the chassis moves.
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (s7 toptest.c body, all four D_800A3558 declarations x 2 bound associations, + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s8] The `t = prim.p_geom + 0xC` two-live-register residual (F8) is reachable by computing the p_static value before the p_geom store.
- mechanism: local-alloc coalesces an add's destination onto its source when the source dies at that insn. The target's first site is `lw $v0,4($s2) / addiu $v1,$v0,12 / sw $v0,0x18($sp) / sw $v1,0x1C($sp)` - the add precedes the p_geom store, so the loaded value is still live at the add and two registers are needed. Our source stores p_geom first, so the value dies at the add and the add coalesces onto $v0.
- probe: `s32 g = *(s32 *)(ctx_or_var_s2 + 4); prim.p_static = g + 0xC; prim.p_geom = g;` at the +0xC site (tmp/grind/func_80070C70/s8/w/d1_f8_toptest.c, d2_f8_dw.c) and the same shape additionally at the +0x48 site (d3/d4), on BOTH chassis.
- result: Backwards on both chassis. do/while 39 -> 40 (one site) and 39 -> 41 (both sites); top-test 49 -> 50 and 49 -> 51. Instruction count stays 194 in all four. The scheduler does not restore the target's store order and the extra named local costs a seat. Banked as rejected/f8-pgeom-add-before-store-40-dw.c and rejected/f8-pgeom-add-before-store-50-tt.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: both the floor-39 if-guarded do/while chassis (memory/grind/func_80070C70/candidate.c body) and the floor-49 top-test for chassis (tmp/grind/func_80070C70/s7/w/d32_b1_cy.c body), each with extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s8] An ASYMMETRIC D_800A3558 spelling - a signed-halfword declaration read bare in the loop bound (the target's `lh`) and double-cast `(s16)(u16)` in the loop body (the target's `lhu` + `sll`/`sra`) - removes the ashift/ashiftrt pair that combine folds, and with it the orphaned spill pseudos.
- mechanism: The target reads D_800A3558 TWICE in the loop tail from the same gp offset - `lhu $a2,%gp_rel(D_800A3558)` at 80070ECC and `lh $v0,%gp_rel(D_800A3558)` at 80070ED0 - and the body sign-extends the lhu value with `sll $v0,$a2,16 / sra $v0,$v0,16` at 80070E78. That is two DIFFERENT C spellings of the same object at the two use sites, which s7's sweep never produced: it always applied one spelling to both sites. A bare signed-halfword read emits `lh` directly, so the exit test would carry no ashift/ashiftrt pair for combine to fold, and that fold is what orphans pseudos 116 and 165.
- probe: 12 asymmetric variants on the top-test chassis (tmp/grind/func_80070C70/s8/x/, generated by s8/gen2.py): declaration in {extern s16, extern u16} x bound spelling in {bare, (s16), (s16)(u16)} x body spelling in {bare, (s16), (s16)(u16)}, bound != body; plus the do/while analogue. Swept with sandbox --disable all, and a BB2_FRAME_DEBUG census on tt_s16_bbare_ys16u and tt_u16_bs16_ybare.
- result: Byte-neutral. Every s16-declared variant scores exactly 49 at 194 insns (the s32 baseline), the u16-declared ones 49/50/52, and the do/while analogue is still 39. The frame census on the s16 asymmetric form is the IDENTICAL triple - spill_new_p116 + spill_new_p165 + spill_new_p170 at vars=104 - and the u16 form gives the same triple renumbered (p116/p164/p169). The front end's RTL for this exit test does not depend on the declared type or on the per-site cast spelling; this is now the fifth chassis and the 29th spelling on which this axis has measured dead (s2 K1, s4 K7, s6, s7 K8, s8). Banked as rejected/asymmetric-s16-u16-cast-byte-neutral-49.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (tmp/grind/func_80070C70/s7/w/d32_b1_cy.c body) and the floor-39 if-guarded do/while chassis, both with extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all plus BB2_FRAME_DEBUG censuses, 2026-09-10

## [s8] Carrying the loop's two memory values in ordinary C locals reproduces the target's loop-carried $a1/$a2 structure without the frame cost.
- mechanism: The target loads D_800A3558 and D_800A35B0 in the guard block, consumes them from registers in the body, and RELOADS both in the loop tail. Spelling that literally in C - locals assigned from the globals and read by the body - should ask for the same register structure directly instead of depending on cse_set_around_loop, and therefore without duplicate_loop_exit_test's pseudo remap and its three orphaned spill slots.
- probe: Three shapes x two chassis (tmp/grind/func_80070C70/s8/y/, generated by s8/gen3.py): g1 = locals read at the TOP of the loop body and used only by the body's `||` condition; g2 = the same locals also used as the loop bound; g3 = locals read once before the loop and RE-READ at the BOTTOM of the body (the target's literal shape), with the bound reading the locals.
- result: All six are worse than their chassis baseline. do/while: g1 = 42 at 192 insns, g2 = 59, g3 = 42 at 191 insns (against 39 at 194). top-test: g1 = 52, g2 = 60, g3 = 63 (against 49 at 194). The dropped instruction counts show the reads being CSE'd or hoisted rather than reloaded - a C-level local is exactly what loop.c and cse.c are free to hoist, whereas the target's reloads survive because they are a MEM the compiler chose to re-materialise. Banked as rejected/carried-value-locals-in-body-42.c and rejected/carried-value-locals-reread-at-tail-42.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: both the floor-39 if-guarded do/while chassis and the floor-49 top-test for chassis (bodies as above), extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s8] Shrinking or hoisting the top-test loop's exit expression removes the orphaned spill pseudos at an acceptable cost.
- mechanism: The three 8-byte slots come from pseudos that combine orphaned inside the exit test. Fewer/simpler sub-expressions in the exit test means fewer pseudos for duplicate_loop_exit_test to remap and fewer folds for combine to orphan.
- probe: Seven variants on the top-test chassis: `var_s0 <= D_800A35B0 + (s16)D_800A3558` (no +1), `var_s0 != ... + 1`, the fully hoisted bound `n = D_800A35B0 + (s16)D_800A3558 + 1`, and four partial hoists (n = (s16)D_800A3558 + 1 / D_800A35B0 + 1 / (s16)D_800A3558 / D_800A35B0), each measured with sandbox --disable all and the fully hoisted one with a BB2_FRAME_DEBUG census.
- result: Every variant is far worse. `<=` and `!=` are 79 at 192 insns (the whole cse_set_around_loop structure collapses). The full hoist is 60 at 187 insns; its census confirms the mechanism - only ONE orphan survives (spill_new_p164) and vars falls 104 -> 88 - but the loop then needs a SEVENTH callee-saved register (`regs= 7/0` against the target's 6/0), which costs more than the 16 bytes saved. The partial hoists are 54, 58, 62, 62 at 190-196 insns. So the exit test's complexity IS the cause of the orphans, and every C-level way of reducing it measured on this chassis trades the 24 frame bytes for a worse residual. Banked as rejected/toptest-le-bound-no-plus1-79.c, rejected/fully-hoisted-bound-7th-saved-reg-60.c, rejected/partial-hoist-bound-54.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (tmp/grind/func_80070C70/s7/w/d32_b1_cy.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all plus a BB2_FRAME_DEBUG census on the hoisted variant, 2026-09-10

## [s8] Pseudo 116 is the ashift intermediate of the (s16) sign-extension inside the loop's OWN tail exit test, and all three orphans are combine's distribute_notes emitting a `(use (reg))` at a CODE_LABEL.
- mechanism: combine.c:10834-10840 - when distribute_notes cannot find an insn after the combined insn that references a dying register and it reaches a CODE_LABEL first, it emits `(use (reg))` after that label and parks the REG_DEAD note there. reload1.c:2382 alter_reg then gives the pseudo an 8-byte stack slot because `reg_renumber[i] < 0 && reg_n_refs[i] > 0`: the pseudo has references but no set anywhere, so flow makes it live from function entry, it conflicts with everything, and global-alloc can never seat it.
- probe: Read of the func_80070C70 region of tmp/grind/func_80070C70/dumps/text1b.jump (regenerated this session with `pwsh tools/grinder/dump.ps1 func_80070C70` on the top-test chassis) for every mention of reg 116, cross-read against tools/gcc-2.7.2/jump.c:2226-2258, combine.c:10800-10870 and reload1.c:2360-2400.
- result: `.jump` insn 267 is `(set (reg/s:SI 116) (ashift:SI (reg/s:SI 114) (const_int 16)))` and insn 268 is `(set (reg/s:SI 115) (ashiftrt:SI (reg/s:SI 116) (const_int 16)))` with a REG_EQUAL sign_extend - i.e. 116 is the sign-extension of D_800A3558 in the loop's OWN tail test (insns 266-272), not a pre-loop pointer as s7 guessed, and 165 is the identical insn inside the guard copy. combine folds each mem+ashift+ashiftrt triple into one `lh` and folds the guard's `lt` into `blez`, orphaning 116, 165 and 170. This is the exact predicate any future probe of F7 must defeat: the exit test must contain no multi-insn expression that combine will fold, WITHOUT losing the cse_set_around_loop structure that the same exit test's complexity provides.
- verdict: CONFIRMED

## [s8] Inlining the named local `t` in the second loop's body is chassis-dependent and is worth a point on the top-test chassis.
- mechanism: `t` is a named intermediate for `prim.p_geom + 0xC`; writing `prim.p_static = prim.p_geom + 0xC;` twice instead removes one pseudo from the loop body and changes the seat local-alloc gives the addend.
- probe: tmp/grind/func_80070C70/s8/w/e1_inline_t_toptest.c and e2_inline_t_dw.c, sandbox --disable all plus a BB2_FRAME_DEBUG census on the top-test form.
- result: top-test 49 -> 48 at 194 insns (banked as memory/grind/func_80070C70/chassis-toptest-cse-48.c, the new best top-test form); do/while 39 -> 41 at 195 insns. The frame census on the improved top-test form is unchanged (the same three orphans, renumbered p116/p168/p173, vars=104), so the point comes from a body seat, not from the frame.
- verdict: CONFIRMED

## [s8] OPEN FRONTIER (replaces s7's F7/F8; F9 carried unchanged)

### F10 - the exit test must be simultaneously complex enough for cse_set_around_loop and fold-free for combine
s8 measured both ends of that trade and both are worse: simplifying the exit test costs the cse structure (79/60/54-62), and every declaration/cast spelling of the sign-extension is byte-neutral (29 spellings, 5 chassis). The untried third option is to keep the exit test's MEM reads (which is all cse_set_around_loop needs - it matches a MEM against a REG_LOOP_TEST_P register) while moving the SIGN-EXTENSION and the `+1` out of the comparison into the induction variable itself, e.g. iterate the counter over a range whose upper end is a bare `D_800A35B0` read and whose offset is folded into the counter's initial value and into every in-loop use of `var_s0`. That changes the meaning of every `var_s0` use site in the body (the D_800A3560 index, the D_800A3590 index, and the `0x50 + var_s0 * 0x16C` mode), so it is a whole-body re-spelling, not a bound tweak - budget a full session for it. Next probe: first read tools/gcc-2.7.2/cse.c:7909-7960 to confirm which register in the exit test must carry REG_LOOP_TEST_P (the MEM's destination, not the comparison result), which decides whether the `+1` and the sign-extension can leave the test at all.

### F11 - the do/while chassis at 39 and the top-test chassis at 48 are 9 apart and their deficits are disjoint
The do/while has the target's exact frame (vars 80, regs 6/0) and none of the carried registers; the top-test has all of the carried registers and 24 bytes of orphan spill. s8 killed the obvious bridge in both directions (carried values as C locals on the do/while; every exit-test simplification on the top-test). What has NOT been tried is attacking the do/while chassis' missing structure from the PRE-loop side: the target's guard block loads `lhu $a2` and `lw $a1` BEFORE the `blez`, which on a do/while chassis would have to come from the if-guard's own condition being spelled so that its loads survive into the loop body. Next probe: sweep if-guard condition spellings on the 39 chassis (the guard currently reads `D_800A35B0 + (s16)D_800A3558 + 1 > 0`) against the target's guard insn order at 80070DF4-80070E18, and check with a .cse dump whether any of them leaves the guard's loads live into the loop.

### F9 - IconC70's real tail layout (carried from s1/s2/s3, still unspent)
`s16 sp50[12]` is a placeholder; the 0x20 SIZE is proven twice over (frame arithmetic + the BB2_FRAME_DEBUG census). Recover the real fields from func_80069898's other callers (func_8006B120, func_8006CFBC, func_800720FC, func_80074488, func_8006F97C) before submission.

## [s8] Computing the p_static value before the p_geom store (s32 g = *(s32 *)(ctx + 4); prim.p_static = g + 0xC; prim.p_geom = g;) reproduces the target's two-live-register `addiu $v1,$v0,0xC` at the pre-loop sites (frontier F8).
- mechanism: local-alloc coalesces an add's destination onto its source when the source dies at that insn. The target's first site is `lw $v0,4($s2) / addiu $v1,$v0,12 / sw $v0,0x18($sp) / sw $v1,0x1C($sp)`: the add precedes the p_geom store, so the loaded value is still live at the add and two registers are needed. Our source stores p_geom first, so the value dies at the add and local-alloc coalesces.
- probe: tmp/grind/func_80070C70/s8/w/d1_f8_toptest.c, d2_f8_dw.c (the +0xC site) and d3_f8both_toptest.c, d4_f8both_dw.c (both pre-loop sites), swept with sandbox --disable all on both chassis.
- result: Backwards on both chassis: do/while 39 -> 40 (one site) and 39 -> 41 (both sites); top-test 49 -> 50 and 49 -> 51. Instruction count stays 194 in all four, so nothing was gained and a seat was lost. The scheduler does not restore the target's store order. Banked as rejected/f8-pgeom-add-before-store-40-dw.c and rejected/f8-pgeom-add-before-store-50-tt.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: both the floor-39 if-guarded do/while chassis (memory/grind/func_80070C70/candidate.c body) and the floor-49 top-test for chassis (tmp/grind/func_80070C70/s7/w/d32_b1_cy.c body), each with extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s8] An asymmetric D_800A3558 spelling - a signed-halfword declaration read bare in the loop bound (the target's `lh`) and double-cast (s16)(u16) in the loop body (the target's `lhu` plus `sll`/`sra`) - removes the ashift/ashiftrt pair that combine folds and with it the orphaned spill pseudos.
- mechanism: The target reads D_800A3558 twice per loop tail from the same gp offset with two different widths (lhu $a2 at 80070ECC, lh $v0 at 80070ED0) and sign-extends the lhu value by hand in the body at 80070E78. That is two different C spellings of one object at two use sites, a region of the space s7 never entered because it always applied one spelling to both sites. A bare signed-halfword read emits `lh` directly, so the exit test would carry no ashift/ashiftrt pair for combine to fold - and that fold is what orphans pseudos 116 and 165.
- probe: 12 asymmetric variants in tmp/grind/func_80070C70/s8/x/ (declaration in {extern s16, extern u16} x bound spelling in {bare, (s16), (s16)(u16)} x body spelling in the other two) plus the do/while analogue, swept with sandbox --disable all, and BB2_FRAME_DEBUG censuses on tt_s16_bbare_ys16u.c and tt_u16_bs16_ybare.c via tmp/grind/func_80070C70/s8/fdrun.py.
- result: Byte-neutral. Every s16-declared variant is exactly 49 at 194 insns (identical to the s32 baseline); the u16-declared ones are 49/50/52; the do/while analogue is still 39. The frame census on the s16 asymmetric form is the IDENTICAL triple spill_new_p116 + spill_new_p165 + spill_new_p170 at vars=104, and the u16 form gives the same triple renumbered (p116/p164/p169). The RTL the front end builds for this exit test does not depend on the declared type or on the per-site cast. This is the fifth chassis and the 29th spelling on which the axis has measured dead (s2 K1, s4 K7, s6, s7 K8, s8). Banked as rejected/asymmetric-s16-u16-cast-byte-neutral-49.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (tmp/grind/func_80070C70/s7/w/d32_b1_cy.c body) and the floor-39 if-guarded do/while chassis, both with extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all plus BB2_FRAME_DEBUG censuses, 2026-09-10

## [s8] Carrying the loop's two memory values in ordinary C locals reproduces the target's loop-carried $a1/$a2 structure without duplicate_loop_exit_test's frame cost.
- mechanism: The target loads D_800A3558 and D_800A35B0 in the guard block, consumes them from registers in the body, and reloads both in the loop tail. Spelling that literally in C should ask for the same register structure directly instead of depending on cse_set_around_loop, and therefore without the pseudo remap that produces the three orphaned spill slots.
- probe: Three shapes x two chassis in tmp/grind/func_80070C70/s8/y/ (generated by s8/gen3.py): g1 = locals read at the top of the loop body and used only by the body's || condition; g2 = the same locals also used as the loop bound; g3 = locals read once before the loop and re-read at the bottom of the body, the target's literal shape, with the bound reading the locals.
- result: All six are worse than their chassis baseline. do/while: 42 at 192 insns, 59, 42 at 191 insns (against 39 at 194). top-test: 52, 60, 63 (against 49 at 194). The falling instruction counts show the reads being CSE'd or hoisted rather than reloaded: a C local is exactly what loop.c and cse.c are free to hoist, whereas the target's per-iteration reloads survive because they are a MEM the compiler chose to re-materialise. Banked as rejected/carried-value-locals-in-body-42.c and rejected/carried-value-locals-reread-at-tail-42.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: both the floor-39 if-guarded do/while chassis and the floor-49 top-test for chassis (bodies as above), each with extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all, 2026-09-10

## [s8] Shrinking or hoisting the top-test loop's exit expression removes the orphaned spill pseudos at an acceptable cost (frontier F7 probes (a) and (b)).
- mechanism: The three 8-byte slots come from pseudos combine orphaned inside the exit test; fewer and simpler sub-expressions in the test means fewer pseudos for duplicate_loop_exit_test to remap and fewer folds for combine to orphan.
- probe: Seven variants on the top-test chassis: `var_s0 <= D_800A35B0 + (s16)D_800A3558` (no +1), `var_s0 != ... + 1`, the fully hoisted bound `n = D_800A35B0 + (s16)D_800A3558 + 1`, and four partial hoists (n = (s16)D_800A3558 + 1 / D_800A35B0 + 1 / (s16)D_800A3558 / D_800A35B0). tmp/grind/func_80070C70/s8/w/b1_le.c, b2_ne.c, b3_hoist.c and s8/z/h1-h4, all with sandbox --disable all, plus a BB2_FRAME_DEBUG census on b3_hoist.c.
- result: Every variant is far worse. `<=` and `!=` are 79 at 192 insns - the whole cse_set_around_loop structure collapses. The full hoist is 60 at 187 insns and its census confirms the mechanism exactly (only ONE orphan survives, spill_new_p164, and vars falls 104 -> 88) but the loop then needs a seventh callee-saved register, regs= 7/0 against the target's 6/0, which costs more than the 16 bytes saved. The four partial hoists are 54, 58, 62, 62 at 190-196 insns. The exit test's complexity IS the cause of the orphans, and every C-level reduction of it measured here trades 24 frame bytes for a worse residual. Banked as rejected/toptest-le-bound-no-plus1-79.c, rejected/fully-hoisted-bound-7th-saved-reg-60.c, rejected/partial-hoist-bound-54.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: top-test for chassis where cse_set_around_loop fires (tmp/grind/func_80070C70/s7/w/d32_b1_cy.c body + extern u8 D_800A3560[]; + extern s16 D_800A3590[]; + IconC70 sized 0x20), no FAKE constructs present, sandbox --disable all plus a BB2_FRAME_DEBUG census on the hoisted variant, 2026-09-10

## [s8] Pseudo 116 is the ashift intermediate of the (s16) sign-extension inside the loop's OWN tail exit test, and all three orphaned spill pseudos are combine's distribute_notes emitting a `(use (reg))` insn at a CODE_LABEL.
- mechanism: combine.c:10834-10840 - when distribute_notes cannot find an insn after the combined insn that references a dying register and reaches a CODE_LABEL first, it emits `(use (reg))` after that label and parks the REG_DEAD note there. reload1.c:2382 alter_reg then allocates a stack slot whenever reg_renumber[i] < 0 && reg_n_refs[i] > 0 && reg_equiv_constant[i] == 0 && reg_equiv_memory_loc[i] == 0, and a pseudo with references but no set is live from function entry in flow's view, so it conflicts with everything and global-alloc can never seat it.
- probe: Regenerated the dumps on the top-test chassis with `pwsh tools/grinder/dump.ps1 func_80070C70`, then read every mention of reg 116 in the func_80070C70 region of tmp/grind/func_80070C70/dumps/text1b.jump, cross-read against tools/gcc-2.7.2/jump.c:2226-2258, combine.c:10800-10870 and reload1.c:2360-2400.
- result: text1b.jump insn 267 is `(set (reg/s:SI 116) (ashift:SI (reg/s:SI 114) (const_int 16)))` and insn 268 is `(set (reg/s:SI 115) (ashiftrt:SI (reg/s:SI 116) (const_int 16)))` with a REG_EQUAL sign_extend. So 116 is the sign-extension of D_800A3558 in the loop's own tail test (insns 266-272) - NOT a pre-loop pointer pseudo as s7 guessed - and 165 is the identical insn inside the guard copy that duplicate_loop_exit_test created. combine folds each mem+ashift+ashiftrt triple into one `lh` and folds the guard's `lt` into `blez`, orphaning 116, 165 and 170. The 24 bytes are not an allocator preference: they follow mechanically once the `(use)` insns exist, so any future F7 probe must prevent the fold rather than hope for a better seat.
- verdict: CONFIRMED

## [s8] Inlining the named local `t` in the second loop's body is worth a point on the top-test chassis and costs two on the do/while chassis.
- mechanism: `t` is a named intermediate for `prim.p_geom + 0xC`; writing `prim.p_static = prim.p_geom + 0xC;` twice instead removes one pseudo from the loop body and changes the seat local-alloc gives the addend.
- probe: tmp/grind/func_80070C70/s8/w/e1_inline_t_toptest.c and e2_inline_t_dw.c with sandbox --disable all, plus a BB2_FRAME_DEBUG census on the top-test form.
- result: top-test 49 -> 48 at 194 insns, the new best top-test form, banked as memory/grind/func_80070C70/chassis-toptest-cse-48.c; do/while 39 -> 41 at 195 insns. The frame census on the improved top-test form is unchanged (the same three orphans, renumbered p116/p168/p173, vars=104), so the point comes from a body seat and not from the frame. Another instance of the s7 finding that body-spelling wins are chassis-dependent.
- verdict: CONFIRMED

## [s9] The score is the WRONG gradient for the top-test chassis: at the object-instruction-shape level the top-test body's entire residual against the target is the 24 frame bytes.
- mechanism: engine/score.py's distance counts every differing instruction, so 13 prologue/epilogue insns whose only difference is a +/-24 stack offset dominate and mask a two-shape improvement elsewhere. tools/ra_solver/goal_from_tgt.py classify compares the two objects' register-blanked instruction-shape MULTISETS instead, which is offset-blind for the loads/stores that differ only in seat and shows exactly which shapes exist in one stream only.
- probe: Installed each chassis body into src/text1b.c with tmp/grind/func_80070C70/s9/install.py, ran `sandbox func_80070C70 --disable all` to refresh tmp/sandbox/func_80070C70/text1b.o, then `python3 tools/ra_solver/goal_from_tgt.py classify text1b func_80070C70` (build/src/text1b.o carries the INCLUDE_ASM target bytes, so it is the target object).
- result: CONFIRMED and it re-ranks the chassis. chassis-toptest-cse-48.c (extern s32 + (s16) cast at both sites, score 48) reports 13 frame shapes PLUS `ours only: lw #,0(#) x2` against `target only: lhu #,0(#) x2`. The s16-bare-bound and u16-both-cast bodies (score 49) report NO load-shape difference at all - their whole residual is the 13 frame shapes plus one `addiu #,#,12` against one `nop`. So a body that scores one point WORSE is two instruction-shapes CLOSER, and the top-test chassis is now exactly 24 frame bytes plus one insn from the target. Banked as memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c.
- verdict: CONFIRMED

## [s9] Both chassis classify PRE-RA: neither the RA solver nor the scheduler solver has any purchase on this function's residual.
- mechanism: goal_from_tgt.py classify walks the layers in order (PRE-RA -> RA -> SCHED -> IDENTICAL) and reports the FIRST divergence; a shape-multiset difference means the two streams do not even contain the same instructions, so no register assignment or emission order can reconcile them and the tool prints `next tool: none - the residual is upstream of every model`.
- probe: classify run on the floor-39 if-guarded do/while body (memory/grind/func_80070C70/candidate.c), on chassis-toptest-cse-48.c, and on the s16/u16 top-test bodies.
- result: All four report FIRST DIVERGENCE: PRE-RA, `next tool: none`. The do/while residual is the missing cse_set_around_loop structure (ours has an extra lw + lh + 2 nops + beq/beqz where the target has 2 lhu + sll + sra + bnez/bne); the top-test residual is the frame. Neither is an allocation seat or a scheduling tie, so tools/ra_solver's global/local/reload models and tools/sched_solver are inapplicable to this function at its present distance. This is a typed verdict for the solver LAYER, not for the function: it says spend sessions on the C-level structure, not on seat/order search.
- verdict: CONFIRMED

## [s9] GCC 2.7.2 emits the ashift/ashiftrt pair for every 16-bit signed memory read under -O regardless of the C declaration or cast, because the MIPS extendhisi2 expander force_not_mem's its operand before it can use the memory pattern.
- mechanism: tools/gcc-2.7.2/config/mips/mips.md:2340-2359, the `extendhisi2` define_expand. Its body is `if (optimize && GET_CODE (operands[1]) == MEM) operands[1] = force_not_mem (operands[1]);` followed by `if (GET_CODE (operands[1]) != MEM) { ... emit_insn (gen_ashlsi3 (temp, op1, 16)); emit_insn (gen_ashrsi3 (operands[0], temp, 16)); DONE; }`. With optimization on, the MEM is unconditionally forced into a HImode pseudo FIRST, so the `extendhisi2_internal` memory pattern (which would emit `lh` in one insn) is never reached from expand. The single `lh` in the final assembly is produced later, by combine re-folding `(mem:HI)` + ashift + ashiftrt back into `(sign_extend:SI (mem:HI))`.
- probe: Installed tmp/grind/func_80070C70/s9/d/tt_s16_b_b.c - `extern s16 D_800A3558;` read completely BARE at both use sites, no cast anywhere - ran `pwsh tools/grinder/dump.ps1 func_80070C70` and read the func_80070C70 region of tmp/grind/func_80070C70/dumps/text1b.rtl (the pre-jump expand output). Cross-read mips.md:2340-2402.
- result: Insn 266 is `(set (reg:HI 114) (mem:HI (symbol_ref:SI ("D_800A3558"))))`, insn 267 `(set (reg:SI 116) (ashift:SI (subreg:SI (reg:HI 114) 0) (const_int 16)))`, insn 268 `(set (reg:SI 115) (ashiftrt:SI (reg:SI 116) (const_int 16)))` with a REG_EQUAL sign_extend. The identical triple appears at insns 333-335 for the second D_800A3558 read and at 319-321 for the `D_800A3590[var_s0]` array read. A six-variant declaration x per-site-cast matrix on BOTH chassis (tmp/grind/func_80070C70/s9/d/) scores 39/39/39/39/39/39 on the do/while and 48/49/49/49/54/49 on the top-test, and the BB2_FRAME_DEBUG census returns the IDENTICAL orphan triple spill_new_p116 + spill_new_p168 + spill_new_p173 at vars=104 for every top-test member. This is the PREDICATE that explains 30+ byte-neutral spellings measured across s2/s4/s6/s7/s8: the declared type never reaches the expander, because expand always builds the shift pair. Any future probe that hopes to remove the ashift/ashiftrt pair by re-declaring or re-casting a 16-bit read is answered by mips.md:2346.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.md:2346
- measured_on: both the floor-39 if-guarded do/while chassis (memory/grind/func_80070C70/candidate.c body) and the top-test for chassis (memory/grind/func_80070C70/chassis-toptest-cse-48.c body), each with extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all plus BB2_FRAME_DEBUG censuses and a cc1 -da .rtl dump, 2026-09-10

## [s9] Re-spelling the loop body's `if ((code != 5) && (code != 16))` guard changes the CODE_LABEL topology around the loop tail and gives combine's orphaned REG_DEAD notes a home.
- mechanism: combine.c:10830-10846 emits the `(use (reg))` insn ONLY when the scan for an insn that references the dying register reaches a CODE_LABEL without finding one (`if (REG_NOTE_KIND (note) == REG_DEAD && place == 0 && tem != 0)`). The label the scan hits was assumed to be the join label of the body's `if`, so moving that label - by inverting the guard into an early `continue`, by splitting it into two sequential `continue`s, or by nesting it as two `if`s - should change which insn `tem` lands on.
- probe: tmp/grind/func_80070C70/s9/e/c1_continue.c (`if ((code == 5) || (code == 16)) continue;`), c2_continue_ne.c (two sequential `continue`s), c3_split_if.c (two nested `if`s), all on the shape-exact top-test chassis, swept with sandbox --disable all plus BB2_FRAME_DEBUG censuses on c1 and c3.
- result: Completely inert. All three score 49 at 194 insns, and both censuses return the same spill_new_p116 + spill_new_p168 + spill_new_p173 at frame_offset 88/96/104 with `.frame $sp,152 # vars= 104, regs= 6/0, args= 24`. The label the scan reaches is therefore not the body `if`'s join label; the remaining candidate is the loop's own exit/continue label created by jump.c's duplicate_loop_exit_test, which no body-level re-spelling of this guard moved. Banked as rejected/body-label-topology-inert-49.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the shape-exact top-test for chassis (memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c body) with extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all plus BB2_FRAME_DEBUG censuses, 2026-09-10

## [s9] KILL RE-AUDIT (mandated by the flat-floor rule): the s8 declaration/cast kill re-measured on the CURRENT chassis with FAKE state checked.
- probe: The six-variant matrix in tmp/grind/func_80070C70/s9/d/ re-run against the driver-confirmed HEAD chassis. base39.c (the banked candidate body) was re-measured at exactly 39 / 194 insns this session BEFORE any probe, confirming the chassis has not moved.
- result: Re-confirmed and now EXPLAINED rather than merely observed. Every do/while member is 39, every top-test member is 48-54 with the s32 baseline still best on SCORE but the s16/u16 members best on SHAPE. The candidate carries no FAKE constructs at all, so there was no carrier to ablate and tools/fake_ablate.py has nothing to strip on this body. The kill stands and is upgraded from instance to class by the mips.md:2346 predicate above.
- verdict: CONFIRMED

## [s9] OPEN FRONTIER (replaces s8's F10; F11 sharpened, F9 carried unchanged)

### F12 - the three orphans are a CODE_LABEL-placement artifact of the loop's own exit label, so the lever is the loop's exit topology, not the exit test's arithmetic
s8's F10 premise ("move the sign-extension and the +1 out of the exit test") is dead on arrival: mips.md:2346 makes the ashift pair unavoidable for any 16-bit read, and the target's own exit test contains the identical `lh` + `addiu 1` + `addu` (80070E08-80070E14 in the guard copy, 80070ED0-80070EDC in the tail), so the target's compile went through the same force_not_mem + shift-pair + combine-refold sequence we do. The target therefore had the same three fold candidates and still got vars=80, which means its REG_DEAD notes found homes at combine.c:10826 (`reg_referenced_p (XEXP (note, 0), PATTERN (tem))`) instead of falling through to the `(use)` emission at combine.c:10834. s9 proved the body `if`'s join label is not the label the scan hits. Next probe: read tmp/grind/func_80070C70/dumps/text1b.combine for the insn IMMEDIATELY BEFORE each `(use (reg 116/168/173))` (in the s9 dumps they sit after insns 203, 544 and 405) and identify which CODE_LABEL `tem` is, then attack THAT label - the candidates are the shape of the loop's continue/exit edge (what statement ends the loop body, whether the last statement is a call, whether the increment is written in the for-header or at the bottom of the body, whether the loop is followed immediately by a call) rather than anything about the exit test's arithmetic.

### F11 - the do/while chassis at 39 and the shape-exact top-test chassis at 49 have disjoint deficits (carried, sharpened by s9)
s9's classify run quantifies both: the do/while's residual is a 6-shape PRE-RA difference (ours `lw` + `lh` + 2 `nop` + `beq` + `beqz`, target 2 `lhu` + `sll` + `sra` + `bnez` + `bne`) - i.e. exactly the missing cse_set_around_loop hoist and nothing else; the top-test's residual is 13 frame shapes plus one `addiu #,#,12` against one `nop`. The top-test side is now the SHORTER road: it needs one structural fix (F12) where the do/while needs the cse hoist that s7/s8 measured unreachable on a do/while. Next probe: work F12 on chassis-toptest-shapeexact-49.c and do not spend further sessions trying to import the cse structure onto the do/while.

### F9 - IconC70's real tail layout (carried from s1/s2/s3, still unspent)
`s16 sp50[12]` is a placeholder; the 0x20 SIZE is proven twice over (frame arithmetic + the BB2_FRAME_DEBUG census). Recover the real fields from func_80069898's other callers (func_8006B120, func_8006CFBC, func_800720FC, func_80074488, func_8006F97C) before submission.

## [s9] GCC 2.7.2 emits the ashift/ashiftrt pair for every 16-bit signed memory read under -O regardless of the C declaration or per-site cast, because the MIPS extendhisi2 expander force_not_mem's its operand before the one-insn memory pattern can apply.
- mechanism: tools/gcc-2.7.2/config/mips/mips.md:2340-2359, the extendhisi2 define_expand: `if (optimize && GET_CODE (operands[1]) == MEM) operands[1] = force_not_mem (operands[1]);` then, the operand no longer being a MEM, `emit_insn (gen_ashlsi3 (temp, op1, 16)); emit_insn (gen_ashrsi3 (operands[0], temp, 16)); DONE;`. The one-insn extendhisi2_internal memory pattern that emits a bare `lh` is unreachable from expand under optimization; every `lh` in the final assembly (ours and the target's) is combine re-folding (mem:HI)+ashift+ashiftrt back into (sign_extend:SI (mem:HI)).
- probe: Installed tmp/grind/func_80070C70/s9/d/tt_s16_b_b.c (extern s16 D_800A3558 read completely BARE at both use sites, no cast anywhere), ran `pwsh tools/grinder/dump.ps1 func_80070C70`, and read the func_80070C70 region of the pre-jump .rtl dump (banked as tmp/grind/func_80070C70/s9/text1b.rtl.s16bare). Cross-read mips.md:2340-2402. Backed by a six-variant declaration x per-site-cast matrix (tmp/grind/func_80070C70/s9/d/) swept on BOTH chassis with sandbox --disable all plus BB2_FRAME_DEBUG censuses.
- result: The expand output for a bare s16 read is insn 266 `(set (reg:HI 114) (mem:HI (symbol_ref:SI ("D_800A3558"))))`, insn 267 `(set (reg:SI 116) (ashift:SI (subreg:SI (reg:HI 114) 0) (const_int 16)))`, insn 268 `(set (reg:SI 115) (ashiftrt:SI (reg:SI 116) (const_int 16)))` with a REG_EQUAL sign_extend. The identical triple appears at insns 333-335 for the second D_800A3558 read and at 319-321 for the D_800A3590[var_s0] array read. The matrix scores 39/39/39/39/39/39 on the do/while chassis and 48/49/49/49/54/49 on the top-test chassis, and every top-test member's frame census returns the identical orphan triple spill_new_p116 + spill_new_p168 + spill_new_p173 at vars=104. This single predicate explains the 30+ byte-neutral declaration/cast spellings measured across s2, s4, s6, s7 and s8, and it also means the target's own build paid the same combine fold: its exit test at 80070E08 and 80070ED0 is the folded `lh`, and its body at 80070E78 is the unfolded `sll`/`sra` pair on a cse-hoisted `lhu`. Banked as rejected/decl-axis-reaudit-byte-neutral-s9.c.
- verdict: KILLED
- kill_scope: class
- measured_on: both the floor-39 if-guarded do/while chassis (memory/grind/func_80070C70/candidate.c body, re-measured at exactly 39/194 this session) and the top-test for chassis (memory/grind/func_80070C70/chassis-toptest-cse-48.c body), each with extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all plus BB2_FRAME_DEBUG censuses and a cc1 -da .rtl dump, 2026-09-10
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.md:2346

## [s9] Re-spelling the loop body's `if ((code != 5) && (code != 16))` guard moves the CODE_LABEL that combine's distribute_notes scan reaches, and so gives the orphaned REG_DEAD notes a home instead of a `(use (reg))` insn.
- mechanism: combine.c:10830-10846 emits the `(use (reg))` insn only when the backward scan for an insn referencing the dying register reaches a CODE_LABEL without finding one (`if (REG_NOTE_KIND (note) == REG_DEAD && place == 0 && tem != 0)`), and reload1.c:2382 alter_reg then gives every resulting refs-but-never-set pseudo an 8-byte stack slot. The body `if`'s join label sits immediately before the loop tail, so it was the obvious candidate for `tem`.
- probe: Three re-spellings on the shape-exact top-test chassis: tmp/grind/func_80070C70/s9/e/c1_continue.c (invert the guard into an early `continue`), c2_continue_ne.c (two sequential `continue`s), c3_split_if.c (two nested `if`s). All swept with sandbox --disable all, plus BB2_FRAME_DEBUG censuses on c1 and c3 via tmp/grind/func_80070C70/s8/fdrun.py.
- result: Completely inert. All three score 49 at 194 insns, identical to the unmodified chassis, and both censuses return the same spill_new_p116 + spill_new_p168 + spill_new_p173 at frame_offset 88/96/104 with `.frame $sp,152 # vars= 104, regs= 6/0, args= 24`. The label the scan reaches is therefore not the body `if`'s join label; the remaining candidate is the loop's own exit/continue label created by jump.c's duplicate_loop_exit_test, which none of these three body-level re-spellings moved. Banked as rejected/body-label-topology-inert-49.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the shape-exact top-test for chassis (memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c body) with extern u8 D_800A3560[]; extern s16 D_800A3590[]; IconC70 sized 0x20, no FAKE constructs present, sandbox --disable all plus BB2_FRAME_DEBUG censuses, 2026-09-10

## [s9] The engine score is a misleading gradient for the top-test chassis: at the object-level instruction-shape layer, the top-test body's entire residual against the target is the 24 bytes of frame overshoot plus one instruction.
- mechanism: engine/score.py counts every differing instruction, so the 13 prologue/epilogue insns that differ only by a +/-24 stack displacement dominate the number and can mask a real structural improvement elsewhere. tools/ra_solver/goal_from_tgt.py classify compares register-blanked instruction-shape MULTISETS instead, which is blind to those displacements' seats and reports exactly which shapes exist in one stream only.
- probe: Installed each chassis body into src/text1b.c with tmp/grind/func_80070C70/s9/install.py, refreshed tmp/sandbox/func_80070C70/text1b.o with `sandbox func_80070C70 --disable all`, then ran `python3 tools/ra_solver/goal_from_tgt.py classify text1b func_80070C70` (build/src/text1b.o carries the INCLUDE_ASM target bytes and is the target object).
- result: chassis-toptest-cse-48.c (extern s32 + an (s16) cast at both sites, score 48) reports 13 frame shapes PLUS `ours only: lw #,0(#) x2` against `target only: lhu #,0(#) x2`. The score-49 bodies whose D_800A3558 is `extern s16` read bare in the bound with an `(s16)` cast in the body, or `extern u16` with an `(s16)` cast at both sites, report NO load-shape difference at all: their whole residual is the 13 frame shapes plus one `addiu #,#,12` against one `nop`. A body that scores one point worse is two instruction-shapes closer. Banked as memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c, which supersedes chassis-toptest-cse-48.c as the structural best.
- verdict: CONFIRMED

## [s9] Both chassis classify PRE-RA, so neither tools/ra_solver's allocation models nor tools/sched_solver can move this function at its present distance.
- mechanism: goal_from_tgt.py classify walks PRE-RA -> RA -> SCHED -> IDENTICAL and reports the first divergence; a shape-multiset difference means the two streams do not contain the same instructions, so no register assignment and no emission order can reconcile them, and the tool prints `next tool: none - the residual is upstream of every model`.
- probe: classify run on the floor-39 do/while body, on chassis-toptest-cse-48.c, and on the s16 and u16 top-test bodies. `tools/ra_solver/inverse_compose.py classify` was tried first per the solver playbook and correctly refused: func_80070C70 is zero-rule, so its text-stream target is absent and the tool would print a fictitious PRE-RA verdict; goal_from_tgt.py is the correct entry point for this function.
- result: All four report FIRST DIVERGENCE: PRE-RA with `next tool: none`. The do/while residual is a 6-shape difference (ours: lw x1, lh x1, nop x2, beq x1, beqz x1; target: lhu x2, sll x1, sra x1, bnez x1, bne x1) which is precisely the missing cse_set_around_loop hoist and nothing else. The top-test residual is the frame. This is a typed verdict for the solver LAYER, not for the function: future sessions should spend on C-level structure, not on seat or ordering search.
- verdict: CONFIRMED

## [s9] KILL RE-AUDIT (mandated by the flat-floor rule): s8's declaration/cast kill, re-measured on the current chassis with FAKE state checked.
- mechanism: An instance kill is only as good as the chassis and FAKE state it was measured under, so the closest-to-target banked kill was re-run before any new probe.
- probe: The six-variant declaration x per-site-cast matrix in tmp/grind/func_80070C70/s9/d/ re-run on the driver-confirmed HEAD chassis, with base39.c (the banked candidate body) re-measured first.
- result: base39.c measures exactly 39 at 194 insns, so the chassis has not moved since s8. Every do/while member of the matrix is 39; every top-test member is 48-54. The candidate body carries no FAKE constructs at all, so there was no carrier to ablate and tools/fake_ablate.py has nothing to strip on it. The s8 kill stands, and s9 upgrades it from an instance kill to a class kill with the mips.md:2346 predicate.
- verdict: CONFIRMED

## s10 (forensics, 2026-09-10) - floor 39, unchanged

Chassis re-measured at dispatch: `memory/grind/func_80070C70/candidate.c` = 39 / 194 insns
(`sandbox --disable all`). `memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c` measured 56 as
banked because of the `//@sub` placement bug (see evidence.md); 49 after the fix.

### H-s10-1 CONFIRMED - the orphan `(use (reg))` is a *guaranteed* 8-byte frame slot, not an allocation outcome
STATEMENT: a pseudo whose only remaining reference is combine's orphan `(use (reg))` insn is typed
ST_REGS by regclass and therefore can never receive a hard register; reload's alter_reg then allocates
it a fresh 8-byte stack slot.
MECHANISM: combine.c:2089 (newi2pat => elim_i2 = 0) -> combine.c:10741 not taken -> backward scan
combine.c:10756 -> combine.c:10834 `emit_insn_after (gen_rtx (USE, ...))` -> regclass leaves the pseudo
with no constraint-derived preference and prints `ST_REGS or none` -> ST_REGS is the fp-status class
(mips.h:1386), disjoint from GR_REGS -> reg_renumber < 0 -> reload1.c:2403 `spill_new_p<N>`,
`assign_stack_local`, 8 bytes each.
PROBE: `tmp/grind/func_80070C70/dumps/text1b.lreg` (lines 69/121/129 of the func-section extract
`s10/w/sec.lreg`) + `s10/w/framedbg.txt` + the two compiler sources.
CONSEQUENCE FOR FUTURE SESSIONS: do NOT spend RA-solver or seat-rotation effort on the 24 bytes. There
is no allocation state in which those pseudos get a register.

### H-s10-2 CONFIRMED - `tem` identity for all three orphans (answers s9's frontier F12)
STATEMENT: p116's `tem` is the second loop's own CONTINUE label (code_label 405, preceded by
NOTE_INSN_LOOP_CONT); p168's and p173's `tem` is the FIRST loop's bottom back-edge `jump_insn 203`, not
a CODE_LABEL at all.
MECHANISM: the scan loop at combine.c:10756-10758 terminates on anything that is not INSN/CALL_INSN, so
a JUMP_INSN is a legal terminator even though the comment at combine.c:10828 says "CODE_LABEL".
PROBE: `s10/w/sec.combine` - `(insn 545 405 500 (use (reg/s:SI 116)))`, `(insn 544 203 543 (use (reg:SI 173)))`,
`(insn 543 544 209 (use (reg:SI 168)))`; `s10/w/sec.jump` insns 266-268 (tail test) and 481-488 (guard copy).

### H-s10-3 KILLED (instance) - loop topology does not move the orphan count on the top-test chassis
STATEMENT: on the shape-exact top-test for chassis, re-spelling the second loop's topology - a fresh
loop variable, hoisting the init out of the for-header, moving the increment to the bottom of the body
with an empty third clause, the while-form, inverting the body guard into an early `continue`, and two
operand reorderings of the bound expression, and `!(i >= bound)` - leaves p116/p168/p173 and
`vars= 104` bit-identical in all eight spellings measured.
MEASURED ON: the shape-exact top-test for chassis
(`memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c` body, `extern s16 D_800A3558` bare in the
bound + `(s16)` cast in the body), zero FAKE constructs present, instrumented cc1 + BB2_FRAME_DEBUG.
Bodies `s10/w/v1_freshvar.c` .. `s10/w/v12_ge_break.c`; census `s10/frame_census.txt`.
RESULT: this closes s9's F12 next-probe as written. The orphan is created by the newi2pat split of the
16-bit compare, wherever that compare sits; the label the scan happens to stop at is downstream of that
and is not a C-reachable knob.

### H-s10-4 CONFIRMED - the ONLY input that moves the orphan count is the bound's sign-extension
STATEMENT: on the top-test chassis, the orphan count is 3 for every sign-extending bound, 2 when the
compare is `!=` (no `slt` to fold into the branch, so p173 never exists), and 1 when the bound reads
`extern u16 D_800A3558` BARE (a zero-extend, so there is no ashift/ashiftrt pair to fold and p116/p168
never exist).
PROBE: `s10/frame_census.txt`. `tt_u16_b_c.c` = 1 orphan / vars=88 / sandbox score 54.
`v6_ne.c` = 2 orphans / vars=96. `v5_boundlocal.c` = 1 orphan / vars=88 but `regs= 7/0`, i.e. it buys
the frame back and immediately spends a seventh callee-saved register.
NOTE: this does NOT contradict s9's class kill on mips.md:2346. That kill says the declared type and
the per-site cast cannot produce a one-insn `lh` from expand. H-s10-4 is about whether a SIGN-extend
happens at all at that site - a different question, and one the declaration does control.

### H-s10-5 KILLED (instance) - the top-test chassis is not "one structural fix" from the target
STATEMENT: the score-49 top-test body's object-level residual against build/src/text1b.o is larger than
the 13 frame insns s9 recorded: it also carries a $a1/$a2 seat swap at both the guard and the tail,
several $v0-vs-$v1 destinations on the `addiu #,#,12` / `sw #,28(sp)` pairs, and delay-slot/load-order
differences (the target keeps a `nop` after `lw v1,0(gp)` where we fill it).
MEASURED ON: the shape-exact top-test for chassis (body as in H-s10-3), zero FAKE constructs, objdump
diff `tmp/grind/func_80070C70/s9/ours.txt` vs `tgt.txt` regenerated this session via `s9/dis.sh`.

### H-s10-6 CONFIRMED - the do/while chassis already reproduces the target's frame exactly
STATEMENT: candidate.c and all six do/while declaration variants measure 0 spill slots and
`.frame $sp,128 # vars= 80, regs= 6/0, args= 24` - the target's frame, byte for byte.
PROBE: `s10/frame_census.txt`.
STRATEGIC CONSEQUENCE: s9's ranking ("the top-test chassis is now the shorter road, stop trying to
import the cse structure onto the do/while") is not supported on the frame axis. The do/while chassis
is missing exactly one thing (the cse_set_around_loop hoist of the D_800A3558/D_800A35B0 reads out of
the loop into the guard) and is otherwise at the target's frame; the top-test chassis has the hoist and
pays 24 bytes plus a seat swap plus scheduling for it.

### s10 TOOLING BANKED (re-use it, do not rebuild it)
`tmp/grind/func_80070C70/s10/probe.py` + `probe.sh` - installs a body and runs ONLY cpp|cc1 (the
instrumented cc1 with BB2_FRAME_DEBUG=1), then prints `spills=<n> <pseudo list> <.frame line>`. ~8s per
body against ~40s for a sandbox run, and it answers frame/orphan questions that score cannot. Invoke as
`wsl bash tmp/grind/func_80070C70/s10/probe.sh <body.c> [<body.c> ...]` from the repo root.
`tmp/grind/func_80070C70/s10/scan.py <passname>` - extracts the func_80070C70 section out of a `-da`
dump into `s10/w/sec.<passname>` and lists every `(use (reg N))` in it.

## [s10] A pseudo whose only remaining reference is combine's orphan `(use (reg))` insn is typed ST_REGS by regclass, so global_alloc can never give it a hard register and reload's alter_reg always allocates it a fresh 8-byte stack slot - the 24-byte overshoot is three such slots and is not an allocation outcome that any RA lever can change.
- mechanism: combine.c:2089 sets `elim_i2 = (newi2pat || i2dest_in_i2src || i2dest_in_i1src ? 0 : i2dest)`, so when try_combine splits a 3-insn combination back into two insns (newi2pat != 0) the dying i2dest is NOT on the eliminate list; combine.c:10741 is then not taken, the REG_DEAD note falls into the backward scan at combine.c:10756-10758 (which skips the setter because the combination already turned it into a NOTE), and combine.c:10834 emits `(use (reg))` after the terminating insn. regclass finds no constraint-derived preference for a USE-only pseudo and prints `ST_REGS or none`; ST_REGS is the MIPS fp-status class (tools/gcc-2.7.2/config/mips/mips.h:1386), disjoint from GR_REGS, so reg_renumber stays < 0 and reload1.c:2382-2404 alter_reg calls assign_stack_local with from_reg == -1 ("No known place to spill from => no slot to reuse"), 8 bytes each (mode=4/SImode, alignment 8).
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) -da dumps on the shape-exact top-test chassis. tmp/grind/func_80070C70/s10/w/sec.lreg lines 69/121/129 read verbatim: `Register 116 used 4 times across 1 insns in block 11; ST_REGS or none.` / `Register 168 ... ST_REGS or none.` / `Register 173 ... ST_REGS or none.` BB2_FRAME_DEBUG=1 (tmp/grind/func_80070C70/s10/w/framedbg.txt) prints spill_new_p116/p168/p173 at frame_offset 88/96/104, size=8 each, then round_frame at 104; the emitted .frame line is `$sp,152 # vars= 104, regs= 6/0, args= 24` against the target's `$sp,128 # vars= 80`.
- result: CONFIRMED. Every step of the chain is now a cited dump line or a cited compiler source line rather than an inference. The practical consequence for future sessions is that the 24 bytes are NOT reachable from the RA side at all - tools/ra_solver, seat rotation and scheduler work cannot touch them, because the pseudos are not in GR_REGS to begin with. Only stopping combine from emitting the USE can remove them.
- verdict: CONFIRMED

## [s10] The insn that terminates combine's backward REG_DEAD scan is the second loop's own CONTINUE label (code_label 405, preceded by NOTE_INSN_LOOP_CONT) for pseudo 116, and the FIRST loop's bottom back-edge jump_insn 203 - not a CODE_LABEL at all - for pseudos 168 and 173.
- mechanism: The scan loop at combine.c:10756-10758 continues only while `GET_CODE (tem) == INSN || GET_CODE (tem) == CALL_INSN`, so a JUMP_INSN terminates it exactly like a CODE_LABEL does, even though the comment at combine.c:10828 speaks only of hitting a CODE_LABEL. p116 is the ashift of the loop's own tail exit test (.jump insns 266/267/268); p168 and p173 belong to the duplicate_loop_exit_test guard copy of the second loop (.jump insns 481-488), which sits far downstream, so its scan walked back over roughly fifteen insns and two CALL_INSNs before stopping at insn 203.
- probe: tmp/grind/func_80070C70/s10/w/sec.combine: `(insn 545 405 500 (use (reg/s:SI 116)))` immediately after `(code_label 405 404 545 924 "")`; `(insn 544 203 543 (use (reg:SI 173)))` and `(insn 543 544 209 (use (reg:SI 168)))` immediately after `(jump_insn 203 ...)`. Cross-read with tmp/grind/func_80070C70/s10/w/sec.jump insns 266-268 and 481-488.
- result: CONFIRMED, and it answers s9's frontier item F12 verbatim ("identify the CODE_LABEL tem resolves to"). It also shows the post-combine guard copy is `lhu; lh; lw; addu; addiu; li 0; blez`, with `(set (reg/v:SI 75) (const_int 0))` re-emitted as the newi2pat between the addiu and the blez - the visible fingerprint of the split that orphans p173, and a sequence the target's own assembly carries too.
- verdict: CONFIRMED

## [s10] On the shape-exact top-test for chassis, re-spelling the second loop's topology leaves the three orphan pseudos and the 104-byte vars region bit-identical in eight of eight spellings measured: a fresh loop variable, the init hoisted out of the for-header, the increment moved to the bottom of the body with an empty third clause, the while-form, the body guard inverted into an early continue, two operand reorderings of the bound expression, and the bound written as !(i >= bound).
- mechanism: The orphan is created by the newi2pat split of the 16-bit compare (combine.c:2089), wherever that compare sits in the insn stream. The label or jump the backward scan happens to stop at is downstream of that decision, so it is not a knob any loop-topology spelling reaches.
- probe: tmp/grind/func_80070C70/s10/probe.py + probe.sh (instrumented cc1 + BB2_FRAME_DEBUG=1, one cc1 run per body, no assembler). Bodies tmp/grind/func_80070C70/s10/w/v1_freshvar.c, v2_initout.c, v3_incbottom.c, v4_while.c, v5_boundlocal.c, v6_ne.c, v7_ne_cont.c, v8_cont.c, v9_reorder1.c, v10_reorder2.c, v11_ne_reorder.c, v12_ge_break.c. Full table in tmp/grind/func_80070C70/s10/frame_census.txt.
- result: KILLED. This closes s9's F12 next-probe as it was written ("attack THAT label from C: what statement ends the loop body, whether the increment is written in the for-header or at the bottom of the body, and what immediately follows the loop"). Two of the twelve bodies did move the count, and both move it via the 16-bit read rather than via topology - see the next hypothesis.
- verdict: KILLED
- kill_scope: instance
- measured_on: the shape-exact top-test for chassis (memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c body: extern s16 D_800A3558 read bare in the loop bound with an (s16) cast in the loop body), zero FAKE constructs present, instrumented cc1 with BB2_FRAME_DEBUG=1

## [s10] The only input shape that changes the orphan count on the top-test chassis is whether the loop bound's 16-bit read is sign-extended: an `extern u16 D_800A3558` read bare in the bound yields 1 orphan and vars=88, a `!=` compare yields 2 orphans and vars=96, and every sign-extending bound yields 3 orphans and vars=104.
- mechanism: A bare u16 read is a zero-extend, so mips.md's extendhisi2 ashift/ashiftrt pair is never emitted at that site and neither p116 nor p168 exists to be orphaned. A `!=` compare gives combine no `slt` to fold into the branch, so p173 (the lt of the guard copy) never exists. Both are upstream of combine's note-placement, unlike the loop's shape.
- probe: tmp/grind/func_80070C70/s10/frame_census.txt. tt_u16_b_c.c: 1 orphan (p171), vars=88, sandbox --disable all score 54. v6_ne.c / v11_ne_reorder.c: 2 orphans (p116, p167), vars=96. v5_boundlocal.c (bound hoisted into a local): 1 orphan, vars=88, but regs= 7/0 - it buys the frame back and immediately spends a seventh callee-saved register.
- result: CONFIRMED as the mechanism, but none of the three is a net win: the u16-bare bound emits an lhu where the target has an lh and scores 54 against the chassis's 49; the != bound changes the branch shape; the bound-in-a-local trades 16 frame bytes for a whole extra callee-saved register. All three banked under memory/grind/func_80070C70/rejected/. This does not contradict s9's class kill on mips.md:2346 - that kill is about the declared type never producing a one-insn lh from expand, which is a different question from whether a sign-extension is emitted at that site at all.
- verdict: CONFIRMED

## [s10] The floor-39 do/while chassis and all six of its declaration variants reproduce the target's frame exactly - 0 spill slots and `.frame $sp,128 # vars= 80, regs= 6/0, args= 24` - while all five sign-extending top-test bodies measure 3 orphans and vars=104.
- mechanism: An if-guarded do/while is not converted by jump.c's duplicate_loop_exit_test, so there is no guard copy to supply p168/p173, and its tail test's fold places its REG_DEAD note successfully. The 24 bytes are therefore a cost the top-test chassis incurs in exchange for the cse_set_around_loop hoist, not a defect shared by both chassis.
- probe: 26-body frame census, tmp/grind/func_80070C70/s10/frame_census.txt (harness tmp/grind/func_80070C70/s10/probe.py). Covers memory/grind/func_80070C70/candidate.c, all twelve s9/d/ declaration variants and the twelve s10 top-test spellings.
- result: CONFIRMED, and it demotes s9's strategic ranking. s9 recorded 'the top-test chassis is now the shorter road ... stop spending sessions trying to import the cse structure onto the do/while body'. On the frame axis that is measurably backwards: the do/while chassis is already AT the target's frame and is missing exactly one thing (the hoist), whereas the top-test chassis has the hoist and pays 24 frame bytes plus an $a1/$a2 seat swap plus scheduling differences for it.
- verdict: CONFIRMED

## [s10] The score-49 top-test body's object-level residual against build/src/text1b.o is larger than the 13 frame insns plus one addiu-against-nop that s9 recorded: it also carries an $a1/$a2 seat swap at both the guard block and the loop tail, several $v0-versus-$v1 destinations on the `addiu #,#,12` / `sw #,28(sp)` pairs, and delay-slot and load-ordering differences.
- mechanism: Object-level diff of the same-dialect disassembly of both streams; the seat swap is visible as target `lhu a2,0(gp)` / `lw a1,0(gp)` / `sll v0,a2,0x10` / `addu v0,a1,v0` against our `lhu a1,0(gp)` / `lw a2,0(gp)` / `sll v0,a1,0x10` / `addu v0,a2,v0` at both program points, and the scheduling difference as a target `nop` after `lw v1,0(gp)` that our build fills with `sw v1,28(sp)`.
- probe: tmp/grind/func_80070C70/s9/dis.sh regenerated this session; diff of tmp/grind/func_80070C70/s9/ours.txt against tmp/grind/func_80070C70/s9/tgt.txt with the top-test chassis installed.
- result: KILLED as a framing. A future session should not plan on the top-test chassis being 'two instruction shapes from the target once the frame is fixed'; fixing the frame leaves a real seat swap and real scheduling work behind it.
- verdict: KILLED
- kill_scope: instance
- measured_on: the shape-exact top-test for chassis (memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c body after the //@sub hygiene fix, re-measured at score 49 / 194 insns this session), zero FAKE constructs present

## [s10] The banked memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c carried its header comment above its //@sub directive, so the installer never applied the substitution and the file as banked measured 56 rather than the recorded 49.
- mechanism: tmp/grind/func_80070C70/s9/install.py consumes //@sub directives only with `while b.startswith('//@sub ')` - i.e. only at the very start of the file - and then slices the body from `b.index(SIG)`, silently discarding anything before the signature including a //@sub line that sits below a comment.
- probe: Installed the banked file verbatim and measured `sandbox func_80070C70 --disable all` = 56; installed tmp/grind/func_80070C70/s9/d/tt_s16_b_c.c (byte-identical body, //@sub on line 1) and measured 49; confirmed src/text1b.c carried `extern s32 D_800A3558;` in the first case.
- result: CONFIRMED and FIXED: the banked file now carries the //@sub on line 1 with a header note explaining the constraint. Standing rule recorded in evidence.md - in any banked candidate the //@sub directives must be the first lines of the file, above any comment.
- verdict: CONFIRMED

## [s11] The second loop's tail bound must be parenthesised as `D_800A35B0 + ((s16)D_800A3558 + 1)` while the guard copy of the same bound must NOT be, and this asymmetry is worth a point on the if-guarded do/while chassis.
- mechanism: The target's tail test is `lh $v0,%gp_rel(D_800A3558); addiu $v0,$v0,1; addu $v0,$a1,$v0` - the constant 1 is added to the sign-extended halfword and the D_800A35B0 word is the OTHER addend of the final `addu`. Written unparenthesised, C's left associativity gives `(D_800A35B0 + sext) + 1`, which emits `addu` first and `addiu` second on the wrong operand. GCC 2.7.2 does not reassociate the mixed sign-extend/word tree back.
- probe: tmp/grind/func_80070C70/s11/w/*.c scored with tmp/grind/func_80070C70/s11/score.sh. w7_paren_tailonly.c = 38/194 insns; w1_paren.c (both guard and tail) = 41/196; x4_g_paren.c (guard only, on the 38 chassis) = 41/196; base = 39/194.
- result: CONFIRMED. Banked as lever L1 in candidate.c. The guard-side form is banked as rejected/s11-guard-bound-parenthesised-too-32.c.
- verdict: CONFIRMED

## [s11] Sequencing the `+0xC`/`+0x48` add as its own statement into a named local BEFORE both struct stores (`g = *(s32*)(ctx+N); t = g + K; prim.p_geom = g; prim.p_static = t;`) reproduces the target's two-live-register `addiu $v1,$v0,12 / sw $v0,0x18($sp) / sw $v1,0x1C($sp)` form at all three sites, and is worth 7 points at the two pre-loop sites plus 2 at the in-loop site.
- mechanism: GCC 2.7.2 schedules before register allocation. When the add is written as `prim.p_static = prim.p_geom + 0xC;` the value is read back out of the just-stored member, the scheduler is free to emit `sw` then `addiu`, the source pseudo dies at the add, and local-alloc gives the add the SAME hard register (`addiu $v0,$v0,12`). Emitting the add as a separate statement between the load and both stores keeps both pseudos live across it, so they must get different hard registers.
- probe: tmp/grind/func_80070C70/s11/u/*.c. u1_t_both.c = 22, u2_t_s1.c = 25, u3_t_s2.c = 25 against the 29-point r1 chassis; z3_g_both_body.c = 36 against the 38-point w7 chassis. The reversed store order (`prim.p_static = g + 0xC; prim.p_geom = g;`) is 40 - s8's result, reproduced - banked as rejected/s11-pstatic-stored-before-pgeom-40.c.
- result: CONFIRMED. Banked as levers L2 and L4 in candidate.c.
- verdict: CONFIRMED

## [s11] Reading `prim.link` before storing `prim.code` at both loop call sites is worth 7 points.
- mechanism: The target emits `lw $v1,0x10($s1)` ahead of `sw $v0,0x2C($sp)` in both loops (80070D80/D88 and 80070EAC/EB4), and lands the link value in the `jal` delay slot from a register that the `li 1` did not clobber. With the stores in the other source order GCC assigns both to $v0 and the delay slot carries the call's own result register.
- probe: tmp/grind/func_80070C70/s11/q/q3_linkfirst.c = 31 (from 36); tmp/grind/func_80070C70/s11/r/r1_loop1_linkfirst.c = 29 (from 31).
- result: CONFIRMED. Banked as lever L3 in candidate.c.
- verdict: CONFIRMED

## [s11] Iterating the second loop with a walking pointer over D_800A3560 (`u8 *p = D_800A3560; code = *p; ...; p += 3;`) scores 91-93 at 195 insns on the if-guarded do/while chassis, against 38 for the index-times-3 offset form.
- mechanism: An incremented pointer is an ADDRESS biv, so loop.c materialises `%hi(D_800A3560)+%lo` into a register in the preheader and the loop indexes off it. The target instead re-emits `lui $at,%hi(D_800A3560); addu $at,$at,$s2; lbu %lo(D_800A3560)($at)` every iteration, i.e. $s2 is an OFFSET giv that is re-added to a fresh %hi each time, which is what `s32 ctx = var_s0 * 3;` produces.
- probe: tmp/grind/func_80070C70/s11/v/v1_ptrwalk.c = 91/195, v3_ptr_modeacc.c = 93/195, v6_ptr_both.c = 93/195 (a second walking pointer over D_800A3590), v5_ptr_modeacc_le.c = 95/193. Banked as rejected/s11-walking-pointer-D_800A3560-91.c and rejected/s11-walking-pointer-plus-mode-accumulator-93.c.
- result: KILLED.
- verdict: KILLED
- kill_scope: instance
- measured_on: the if-guarded do/while chassis (memory/grind/func_80070C70 s10 candidate body, floor 39, and its s11 w7 derivative at 38), zero FAKE constructs present, `sandbox --disable all`

## [s11] Spelling the second loop's mode as an explicit accumulator (`m = 0x50; ... prim.mode = m; ... m += 0x16C;`) is BYTE-NEUTRAL against `prim.mode = 0x50 + var_s0 * 0x16C`.
- mechanism: loop.c's strength reduction already reduces the `0x50 + i*0x16C` giv to the target's `addiu $s3,$s3,0x16C` at the biv increment site, so the source-level accumulator adds nothing the compiler was not already doing.
- probe: tmp/grind/func_80070C70/s11/v/v2_modeacc.c = 39 at 194/194 insns against base.c = 39 at 194/194.
- result: KILLED as a lever (it is a legitimate alternative spelling, but it buys nothing, so the simpler expression stays).
- verdict: KILLED
- kill_scope: instance
- measured_on: the if-guarded do/while chassis (s10 candidate body, floor 39), zero FAKE constructs present

## [s11] Carrying D_800A3558 and D_800A35B0 in C locals that are re-assigned at the bottom of the loop body - the literal C spelling of the target's loop-carried $a1/$a2 - scores 55 to 59 at 197 insns on the 38-point do/while chassis.
- mechanism: The source-level carry adds a third and fourth live value across the whole loop, and GCC 2.7.2 emits the reload as an extra pair of insns in BOTH the guard and the tail rather than folding it into the test's own loads, so the body saves two insns and the loop's edges pay five. The target's structure has the SAME load in the tail serving both the test and the next body, which only cse can produce.
- probe: tmp/grind/func_80070C70/s11/y/*.c: y1_u16_carry.c (u16 declaration, `(s16)h` in the body) = 59/197, y2 (reload before the increment) = 59/197, y3 (s16 declaration) = 59/197, y4 (operands reversed) = 59/197, y5 (plain s32 locals) = 55/194. Banked as rejected/s11-carry-two-globals-in-locals-59.c.
- result: KILLED. Reproduces s8's kill of the same idea on a chassis 17 points lower, so the kill is not an artifact of the old chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: the if-guarded do/while chassis at 38 (tmp/grind/func_80070C70/s11/w/w7_paren_tailonly.c body), with the D_800A3558 declaration substituted per variant, zero FAKE constructs present

## [s11] The prologue tie between `addiu $a0,$sp,24` and `addu $s0,$zero,$zero` is not reachable from the source position of `var_s0 = 0;`.
- mechanism: Both insns are ready at the same cycle with no data dependence on anything in flight; GCC 2.7.2's sched1 breaks the tie by INSN_PRIORITY, which is derived from the length of each insn's dependence chain to the end of the block, not from source order. `addiu $a0,$sp,24` feeds a `jal` argument several insns later and `addu $s0,$zero,$zero` feeds the first loop, and the source position of the assignment does not change either chain.
- probe: tmp/grind/func_80070C70/s11/a/a1..a5 - `var_s0 = 0;` placed before `g = ...`, after `g = ...`, after `t = ...`, after `prim.p_geom = g;`, after `prim.p_static = t;`, and immediately before the do-loop. All six score 22 at 194/194 insns with identical output.
- result: KILLED. The residual (ii) in candidate.c stands; a future session should not spend probes on statement placement for it.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (memory/grind/func_80070C70/candidate.c as written by s11), zero FAKE constructs present

## [s11] The second loop's TAIL bound must be parenthesised as `D_800A35B0 + ((s16)D_800A3558 + 1)` while the GUARD copy of the same bound must stay unparenthesised, and that asymmetry is worth a point on the if-guarded do/while chassis.
- mechanism: The target's tail test is `lh $v0,%gp_rel(D_800A3558); addiu $v0,$v0,1; addu $v0,$a1,$v0` - the constant 1 is added to the sign-extended halfword and the D_800A35B0 word is the other addend of the final addu. C's left associativity makes the unparenthesised form `(D_800A35B0 + sext) + 1`, which emits addu first and addiu on the wrong operand; GCC 2.7.2 does not reassociate the mixed sign-extend/word tree back. s7 killed this same parenthesisation, but only on the TOP-TEST chassis (57/54 there against 51/49) - that kill was chassis-relative and this session's re-audit found the lever live on the do/while.
- probe: tmp/grind/func_80070C70/s11/w/*.c via tmp/grind/func_80070C70/s11/score.sh: w7_paren_tailonly.c = 38 at 194/194 against base.c = 39; w1_paren.c (both copies) = 41 at 196; x4_g_paren.c (guard only, on the 38 chassis) = 41 at 196.
- result: CONFIRMED and banked as lever L1 in memory/grind/func_80070C70/candidate.c. The guard-side form is banked as rejected/s11-guard-bound-parenthesised-too-32.c.
- verdict: CONFIRMED

## [s11] Sequencing the +0xC / +0x48 add as its own statement into a named local before both struct stores (`g = *(s32*)(ctx+N); t = g + K; prim.p_geom = g; prim.p_static = t;`) reproduces the target's two-live-register `addiu $v1,$v0,12 / sw $v0,0x18($sp) / sw $v1,0x1C($sp)` at all three p_static sites and is worth 9 points.
- mechanism: GCC 2.7.2 schedules before register allocation. Written as `prim.p_static = prim.p_geom + 0xC;` the value is read back out of the just-stored member, sched1 is free to emit the sw before the addiu, the source pseudo dies at the add, and local-alloc gives the add the SAME hard register (`addiu $v0,$v0,12`). Emitting the add as a separate statement between the load and both stores keeps two pseudos live across it, so they must get different hard registers. This is NOT s8's killed form: s8 measured the REVERSED STORE ORDER (`prim.p_static = g + 0xC; prim.p_geom = g;`), which is still worse (40); keeping the target's store order and lifting only the ADD out is the win.
- probe: tmp/grind/func_80070C70/s11/u/u1_t_both.c = 22, u2_t_s1.c = 25, u3_t_s2.c = 25 against the 29-point r1 chassis; tmp/grind/func_80070C70/s11/z/z3_g_both_body.c = 36 against the 38-point w7 chassis. The reversed order was re-measured this session at tmp/grind/func_80070C70/s11/q/q1_static_first_both.c = 40.
- result: CONFIRMED and banked as levers L2 and L4 in candidate.c; the reversed form is banked as rejected/s11-pstatic-stored-before-pgeom-40.c.
- verdict: CONFIRMED

## [s11] Reading prim.link before storing prim.code at both loop call sites is worth 7 points.
- mechanism: The target emits `lw $v1,0x10($s1)` ahead of `sw $v0,0x2C($sp)` in both loops (80070D80/D88 and 80070EAC/EB4) and carries the link value into the jal delay slot in a register the `li 1` did not clobber. With the stores in the other source order GCC assigns both to $v0 and the delay slot carries the call's own result register instead.
- probe: tmp/grind/func_80070C70/s11/q/q3_linkfirst.c = 31 (from 36, second loop); tmp/grind/func_80070C70/s11/r/r1_loop1_linkfirst.c = 29 (from 31, first loop).
- result: CONFIRMED and banked as lever L3 in candidate.c.
- verdict: CONFIRMED

## [s11] Iterating the second loop with a walking pointer over D_800A3560 (`u8 *p = D_800A3560; code = *p; ...; p += 3;`) scores 91 to 95 at 195 insns on the if-guarded do/while chassis, against 38 for the index-times-3 offset form.
- mechanism: An incremented pointer is an ADDRESS biv, so loop.c materialises %hi(D_800A3560)+%lo into a register in the preheader and the loop indexes off it. The target instead re-emits `lui $at,%hi(D_800A3560); addu $at,$at,$s2; lbu %lo(D_800A3560)($at)` every iteration, i.e. $s2 is an OFFSET giv re-added to a fresh %hi each time, which is what `s32 ctx = var_s0 * 3;` produces.
- probe: tmp/grind/func_80070C70/s11/v/v1_ptrwalk.c = 91/195, v3_ptr_modeacc.c = 93/195, v6_ptr_both.c = 93/195 (a second walking pointer over D_800A3590), v5_ptr_modeacc_le.c = 95/193.
- result: KILLED. Banked as rejected/s11-walking-pointer-D_800A3560-91.c and rejected/s11-walking-pointer-plus-mode-accumulator-93.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the if-guarded do/while chassis (s10 candidate body at floor 39 and its s11 w7 derivative at 38), zero FAKE constructs present, sandbox --disable all

## [s11] Spelling the second loop's mode as an explicit accumulator (`m = 0x50; ... prim.mode = m; ... m += 0x16C;`) is byte-neutral against `prim.mode = 0x50 + var_s0 * 0x16C`.
- mechanism: loop.c's strength reduction already reduces the `0x50 + i*0x16C` giv to the target's `addiu $s3,$s3,0x16C` at the biv increment site, so the source-level accumulator adds nothing the compiler was not already doing.
- probe: tmp/grind/func_80070C70/s11/v/v2_modeacc.c = 39 at 194/194 insns against base.c = 39 at 194/194.
- result: KILLED as a lever - a legitimate alternative spelling that buys nothing, so the simpler expression stays in candidate.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the if-guarded do/while chassis (s10 candidate body, floor 39), zero FAKE constructs present, sandbox --disable all

## [s11] Carrying D_800A3558 and D_800A35B0 in C locals re-assigned at the bottom of the loop body - the literal C spelling of the target's loop-carried $a1/$a2 - scores 55 to 59 at 197 insns on the 38-point do/while chassis.
- mechanism: The source-level carry adds two more live values across the whole loop, and GCC 2.7.2 emits the reload as an extra pair of insns in BOTH the guard and the tail rather than folding it into the test's own loads: the body saves two insns and the loop's edges pay five. The target's structure has the same load in the tail serving both the exit test and the next iteration's body, which only cse can produce.
- probe: tmp/grind/func_80070C70/s11/y/y1_u16_carry.c (u16 declaration, (s16)h in the body) = 59/197, y2 (reload before the increment) = 59/197, y3 (s16 declaration) = 59/197, y4 (operands reversed) = 59/197, y5 (plain s32 locals) = 55/194.
- result: KILLED, and it reproduces s8's kill of the same idea on a chassis 17 points lower, so that kill is not an artifact of the old chassis. Banked as rejected/s11-carry-two-globals-in-locals-59.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the if-guarded do/while chassis at 38 (tmp/grind/func_80070C70/s11/w/w7_paren_tailonly.c body) with the D_800A3558 declaration substituted per variant, zero FAKE constructs present, sandbox --disable all

## [s11] The prologue tie between `addiu $a0,$sp,24` and `addu $s0,$zero,$zero` does not move for any of six source positions of `var_s0 = 0;`.
- mechanism: Both insns are ready in the same cycle with no data dependence on anything in flight; GCC 2.7.2's sched1 breaks the tie by INSN_PRIORITY, derived from the length of each insn's dependence chain to the end of the block rather than from source order. `addiu $a0,$sp,24` feeds a jal argument several insns later and `addu $s0,$zero,$zero` feeds the first loop, and moving the assignment changes neither chain.
- probe: tmp/grind/func_80070C70/s11/a/a1..a5 plus the candidate body - `var_s0 = 0;` placed before `g = ...`, after `g = ...`, after `t = ...`, after `prim.p_geom = g;`, after `prim.p_static = t;`, and immediately before the do-loop. All six score 22 at 194/194 insns with identical output.
- result: KILLED. Residual (ii) in candidate.c stands and a future session should not spend probes on statement placement for it.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (memory/grind/func_80070C70/candidate.c as written by s11), zero FAKE constructs present, sandbox --disable all

## [s11] Spelling the mode test's || operands in the target's order (the sum tested first, D_800A35BC second) is worse on the do/while chassis at every score level measured, including the new 22-point one.
- mechanism: The target's block really is sum-first (`bnez $v0,.L80070E9C` on the sum, then the D_800A35BC compare), so this is a case where the object shape and the score disagree: putting the sum first also removes four instructions elsewhere (190 built against the target's 194), and that shortfall costs more than the branch-order match gains.
- probe: tmp/grind/func_80070C70/s11/a/a6_orswap.c = 30 at 190/194 against candidate.c = 22 at 194/194; tmp/grind/func_80070C70/s11/t/t3_orswap.c = 34 against r1 = 29; s6 measured the same direction (39 vs 40).
- result: KILLED as a lever on this chassis. Banked as rejected/s11-or-operands-sum-first-30.c. NOTE for future sessions: s7 measured the OPPOSITE on the top-test chassis (sum-first wins there, 49 vs 51), so this is genuinely chassis-coupled and must be re-measured after any chassis change.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (candidate.c body) and its 29-point r1 ancestor, zero FAKE constructs present, sandbox --disable all

## [s12] Porting s11's ordinary-C levers L2 (named geom/static temp pair at both pre-loop sites), L3 (prim.link read before the prim.code store at both call sites) and L4 (the same temp pair inside the second loop body) onto the shape-exact TOP-TEST for chassis carries that chassis past the if-guarded do/while's floor of 22.
- mechanism: s11 measured L2/L3/L4 worth 17 points on the do/while chassis (39 -> 22). s7 had already shown that body-spelling wins are chassis-dependent, but the top-test chassis was last measured with the pre-s11 body (49), so its 24-byte orphan-spill penalty (about 13 insn shapes, H-s10-1) was being compared against a body that had not been given the levers. If the levers were worth anything like 17 points there too, the penalty might be affordable.
- probe: tmp/grind/func_80070C70/s12/a/a0..a6.c, scored with tmp/grind/func_80070C70/s12/score.sh (sandbox --disable all). a0 = the unmodified chassis; a1 = +L3; a2 = +L3+L2; a3 = +L3+L2+L4; a4 = a3 with the || operands in the target's order; a5 = a4 with the bound parenthesised; a6 = a2 with the || reorder.
- result: KILLED. The levers DO port and are worth 18 points there (49 -> 42 -> 36 -> 31), one more than on the do/while, but 31 is still 9 worse than the do/while's 22. Every additional lever tried on top of a3 made it worse: the || reorder is 36, the parenthesised bound is 41 (and 195 insns - the for-chassis has only one written bound, so the +1 no longer reassociates for free), and three parenthesisation variants b1/b2/b3 are 36/36/35 at 195-196 insns. So the chassis question s7-s11 kept reopening is now answered with a number rather than a strategic argument: the do/while chassis is ahead by 9 with both chassis carrying the same body levers. Banked as rejected/s12-toptest-with-L2L3L4-31.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the shape-exact top-test for chassis (memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c body, extern s16 D_800A3558 bare in the bound + (s16) cast in the body) with s11's L2/L3/L4 applied, zero FAKE constructs present beyond `s32 c60`, sandbox --disable all

## [s12] The three frame-buying levers s10 measured on the 49-point top-test body (bound hoisted into a local, bare-u16 bound, != bound) become net-positive once the body carries s11's L2/L3/L4 and the chassis is at 31.
- mechanism: s10 proved each lever removes orphan spill pseudos (bound-in-a-local: 1 orphan, vars=88; bare u16: 1 orphan, vars=88; !=: 2 orphans, vars=96) but each spends something else - a seventh callee-saved register, an lhu where the target has lh, a different branch shape. Those trades were priced against a body 18 points worse than the current one, and the CURRENT-SCOPE rule requires chassis-relative conclusions to be re-measured when the chassis moves.
- probe: tmp/grind/func_80070C70/s12/a/c1.c (bound in a local), c2.c (extern u16 D_800A3558 bare in the bound), c3.c (!= compare), c4.c (both c1 and c2), each built on a3 and scored with s12/score.sh.
- result: KILLED. c1 = 51 at 187 insns, c2 = 34 at 194, c3 = 62 at 192, c4 = 51 at 187. All four are worse than a3's 31, and c1/c4 lose seven instructions outright. The frame bytes are still not worth what they cost on this chassis. Banked as rejected/s12-toptest-bound-in-local-51.c and rejected/s12-toptest-u16-bare-bound-34.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 31-point top-test for chassis (tmp/grind/func_80070C70/s12/a/a3.c body), zero FAKE constructs beyond `s32 c60`, sandbox --disable all

## [s12] Carrying the second loop's two globals in C locals - the literal transcription of the target's $a1/$a2 structure - wins on the 22-point do/while chassis, where s11 could only measure it against the 39-point body.
- mechanism: the target loads D_800A3558 (as a raw lhu) and D_800A35B0 in the second loop's guard block, consumes them from registers in the body, and reloads both in the tail. The registers are $a1/$a2, i.e. CALLER-saved, so the live range crosses the loop back edge but never the body's jal. Spelling that directly in C - two locals assigned in the guard region and re-assigned at the bottom of the body, with the body's || test reading the locals - asks for the same structure without needing cse_set_around_loop, which the do/while chassis cannot reach.
- probe: tmp/grind/func_80070C70/s12/d/d1..d6.c (six spellings: extern u16 / extern s16 / extern s32 for D_800A3558, both operand orders of the body's sum, both assignment orders at the bottom, and the tail test reading either the local or the global) plus tmp/grind/func_80070C70/s12/e/e3..e8.c (only D_800A35B0 carried) and e4.c (only D_800A3558 carried).
- result: KILLED. All six two-local spellings score exactly 42 at 195 instructions and are byte-identical to one another - the declared type, the operand order and the assignment order are all inert here, the same inertness s9's mips.md:2346 class kill predicts for the 16-bit read. Carrying only D_800A3558 is 40 at 196. Carrying only D_800A35B0 in a local `nb` is the family's best at 27/194, and its four sub-spellings (assignment above vs below var_s0++, both operand orders in the body test, the tail reading the global instead of the local) are all 27 as well. Dropping the bottom re-assignment entirely is 40 at 197. So the family is still 5 points behind plain re-reads even at its best, but the gap has closed from s11's 55-59-vs-39 to 27-vs-22, which is worth knowing: the family tracks the chassis rather than being flatly wrong. Banked as rejected/s12-carried-locals-on-22-chassis-42.c and rejected/s12-carry-D_800A35B0-only-27.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (memory/grind/func_80070C70/candidate.c as written by s12), zero FAKE constructs beyond `s32 c60`, sandbox --disable all

## [s12] The `s32 var_s3 = 0xA;` constant-holder local in the s11 candidate is load-bearing.
- mechanism: the target keeps 10 in $s3 (li s3,10; sw s3,44(sp)) and 96 in $s4 (li s4,96; move a1,s4) across the whole function, which is what a named local produces. s11 carried both as named locals without ever measuring whether the literal spelling costs anything - and every unnecessary constant-holder is a construct that would need its own named-local FAKE vet at submission.
- probe: tmp/grind/func_80070C70/s12/e/f2.c (prim.code = 0xA written literally, var_s3 and its declaration deleted) and f1.c (func_8006E480(prim.p_geom, 0x60) written literally, c60 deleted), both scored against the 22-point body.
- result: KILLED for var_s3, CONFIRMED for c60. f2 scores 22 at 194 instructions - identical to the body that declares var_s3 - so that constant-holder is removable at zero cost and this session's candidate.c drops it. f1 scores 29 at 191 instructions, i.e. c60 is worth three instructions and 7 points and must stay. The candidate now carries exactly one constant-holder local instead of two.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (s11 candidate.c body), sandbox --disable all

## [s12] IconC70 is a 0x20-byte record, so the trailing 24 bytes of the target's vars=80 frame belong to the icon struct and the placeholder `s16 sp50[12]` member list only needs its FIELD NAMES recovered.
- mechanism: s1's frame arithmetic, s6's BB2_FRAME_DEBUG census and s10's stack_temp mode=26 size=32 all agreed the local aggregate after prim occupies 32 bytes, and every session since s1 has modelled that as one 0x20 icon record with four known halfwords and 24 unknown trailing bytes.
- probe: read the CALLEE. asm/funcs/func_80069898.s dereferences its $a1 argument (copied to $s1 at 800698A8) at exactly four offsets - lhu $v0,0x0($s1), 0x2, 0x4, 0x6 - three times over, feeding each quadruple into a TILE prim at sh ...,0x8/0xA/0xC/0xE($s0). No other $s1-relative access exists in the function. Then read a SIBLING CALLER: asm/funcs/func_8006B120.s has addiu $sp,$sp,-0x68 with $s0 saved at 0x50 and the outgoing-args region at 0x0..0x17, so its vars region is 0x18..0x4F = 56 bytes = prim(48 at 0x18) + icon(8 at 0x48), and it passes addiu $a1,$sp,0x48 after storing only 0x48/0x4A/0x4C.
- result: KILLED. The icon record is EIGHT bytes - four 16-bit fields at +0/+2/+4/+6 - not 0x20. func_8006B120 allocates exactly eight bytes for it and puts its first callee-saved register immediately above. The 24 bytes at sp+0x50..0x67 in func_80070C70 therefore belong to a DIFFERENT local, and s12 measured that the local must be REFERENCED: substituting an evidence-sized 8-byte IconC70 plus an unreferenced s32 sp50[6] scores 36 and the frame FALLS from 128 to 104 - GCC 2.7.2 drops the unreferenced aggregate outright (this is the opposite of the phantom-frame-slot behaviour recorded in the Claude memory project/phantom-frame-slots-gcc272, which describes live-but-untouched scalars). func_800720FC shows the same idiom with more room: frame 0x98, $s0 at 0x70, icon still at 0x48, leaving 32 spare bytes. Banked as rejected/s12-icon8-plus-separate-24byte-local-36.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (s12 candidate.c body) for the frame measurement; the callee/sibling evidence is static asm reading and is chassis-independent

## [s12] Two cheap statement-position levers aimed at the remaining prologue tie and the guard block.
- mechanism: (a) the target emits move s0,zero BEFORE the second loop's guard loads, whereas we emit it after the blez; writing var_s0 = 0; above the if should move it. (b) the target materialises addiu $a0,$sp,24 very early, right after lw s2,100(v1); taking &prim into a pointer local should let that address be computed at the point of the declaration rather than at each call.
- probe: tmp/grind/func_80070C70/s12/e/e1.c and e2.c (var_s0 hoisted, on the plain body and on the carried-locals body); tmp/grind/func_80070C70/s12/g/g1.c and g2.c (s32 *pp = (s32 *)&prim; declared after ctx_or_var_s2 and at the top of the body), plus g3.c (the duplicated prim.zero1C/prim.mode pair reordered) and g4.c (the ctx_or_var_s2 read hoisted to the top).
- result: KILLED. e1 = 54 at 193 insns, e2 = 71 at 194, g1 = g2 = 38 at 197, g3 = 24, g4 = 60. Every one is worse than 22. The pointer local costs three instructions outright, and hoisting var_s0 = 0; above the guard loses one. Banked as rejected/s12-var_s0-zero-hoisted-above-guard-54.c and rejected/s12-prim-pointer-local-38.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (s12 candidate.c body), zero FAKE constructs beyond `s32 c60`, sandbox --disable all

## [s12] Porting s11's ordinary-C levers L2 (named geom/static temp pair at both pre-loop sites), L3 (prim.link read before the prim.code store at both call sites) and L4 (the same temp pair inside the second loop body) onto the shape-exact TOP-TEST for chassis carries that chassis past the if-guarded do/while's floor of 22.
- mechanism: s11 measured L2/L3/L4 worth 17 points on the do/while chassis (39 -> 22), and s7 had shown body-spelling wins are chassis-dependent. The top-test chassis was last priced with the pre-s11 body (49), so its 24-byte orphan-spill penalty (about 13 insn shapes, H-s10-1: combine newi2pat -> orphan (use (reg)) -> ST_REGS -> reload spill slot) was being compared against a body that had never been given the levers.
- probe: tmp/grind/func_80070C70/s12/a/a0..a6.c and b1..b5.c, scored with tmp/grind/func_80070C70/s12/score.sh (engine sandbox --disable all, cheat-asm stripped).
- result: The levers DO port and are worth 18 points on the top-test chassis (49 -> 42 with L3 -> 36 with L2 -> 31 with L4), one point more than they are worth on the do/while. But 31 is still 9 worse than 22, so the top-test chassis' frame penalty remains unaffordable. Everything stacked on top of a3 was worse: the || operand reorder into the target's order is 36 at 193 insns, the parenthesised bound is 41 at 194 (and the three other parenthesisation spellings b1/b2/b3 are 36/36/35 at 195-196 insns, because the for-chassis writes the bound only once so the +1 no longer reassociates for free). Objdump-diffing a3 against build/src/text1b.o confirms the top-test body genuinely HAS the cse_set_around_loop structure the do/while lacks - lhu a2 / lw a1 in the guard and the tail, sll/sra/addu in the body - and that its whole remaining residual is the 152-vs-128 frame plus an a0/a1/a2 seat permutation. It is still the losing side. Banked as rejected/s12-toptest-with-L2L3L4-31.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the shape-exact top-test for chassis (memory/grind/func_80070C70/chassis-toptest-shapeexact-49.c body: extern s16 D_800A3558 bare in the bound, (s16) cast in the body) with s11's L2/L3/L4 applied; zero FAKE constructs present beyond the inherited s32 c60 constant holder; sandbox --disable all

## [s12] The three frame-buying levers s10 measured on the 49-point top-test body (bound hoisted into a local, bare-u16 bound, != bound) become net-positive once the body carries s11's L2/L3/L4 and the chassis is at 31.
- mechanism: s10 proved each lever removes orphan spill pseudos (bound-in-a-local 1 orphan / vars=88, bare u16 1 orphan / vars=88, != 2 orphans / vars=96) at the cost of a seventh callee-saved register, an lhu where the target has lh, or a different branch shape. Those trades were priced against a body 18 points worse than the current one, and the dispatch brief's CURRENT SCOPE rule requires chassis-relative conclusions to be re-measured when the chassis moves.
- probe: tmp/grind/func_80070C70/s12/a/c1.c (bound in a local), c2.c (extern u16 D_800A3558 read bare in the bound), c3.c (!= compare), c4.c (c1 and c2 together), each built on the 31-point a3 body and scored with s12/score.sh.
- result: c1 = 51 at 187 insns, c2 = 34 at 194, c3 = 62 at 192, c4 = 51 at 187. All four are worse than a3's 31, and the bound-in-a-local variants lose seven instructions outright to the seventh callee-saved register. The frame bytes are still not worth what they cost. Banked as rejected/s12-toptest-bound-in-local-51.c and rejected/s12-toptest-u16-bare-bound-34.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 31-point top-test for chassis (tmp/grind/func_80070C70/s12/a/a3.c body), zero FAKE constructs beyond the inherited s32 c60, sandbox --disable all

## [s12] Carrying the second loop's two globals in C locals - the literal transcription of the target's $a1/$a2 register structure - wins on the 22-point do/while chassis, where s11 could only measure it against the 39-point body.
- mechanism: The target loads D_800A3558 (as a raw lhu into $a2) and D_800A35B0 (into $a1) in the second loop's guard block, consumes them from registers in the body, and reloads both in the tail. Both are CALLER-saved registers, so the live range crosses the loop back edge but never the body's jal - which is exactly what a C local assigned in the guard and re-assigned at the bottom of the body asks for, without needing cse_set_around_loop (unreachable on a do/while: cse.c:7933 gates on REG_LOOP_TEST_P and jump.c:2253 is its only setter, inside duplicate_loop_exit_test, which needs a top-test loop).
- probe: tmp/grind/func_80070C70/s12/d/d1..d6.c (six spellings: extern u16 / extern s16 / extern s32 for D_800A3558, both operand orders of the body's sum, both assignment orders at the bottom, tail test reading either the local or the global), plus s12/e/e3.c and e5..e8.c (only D_800A35B0 carried) and e4.c (only D_800A3558 carried).
- result: All six two-local spellings score exactly 42 at 195 instructions and are byte-identical to one another - declared type, operand order and assignment order are all inert, matching the inertness s9's mips.md:2346 class kill predicts for any 16-bit read. Carrying only D_800A3558 is 40 at 196. Carrying only D_800A35B0 in a local is the family's best at 27 at 194 insns, and its four sub-spellings (assignment above vs below var_s0++, both operand orders in the body test, the tail reading the global) are all 27. Dropping the bottom re-assignment entirely is 40 at 197. So the family is still behind plain re-reads, but the gap has closed from s11's 16-20 points (55-59 against 39) to 5 points (27 against 22): it tracks the chassis rather than being flatly wrong, which matters if the chassis moves again. Banked as rejected/s12-carried-locals-on-22-chassis-42.c and rejected/s12-carry-D_800A35B0-only-27.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (memory/grind/func_80070C70/candidate.c as written by s12), zero FAKE constructs beyond the inherited s32 c60, sandbox --disable all

## [s12] The `s32 var_s3 = 0xA;` constant-holder local inherited from the s11 candidate is load-bearing and cannot be written as a literal.
- mechanism: The target keeps 10 in $s3 (li s3,10 / sw s3,44(sp)) and 96 in $s4 (li s4,96 / move a1,s4) across the whole function, which is what a named local produces. s11 introduced both as named locals without measuring whether the literal spelling costs anything - and each unnecessary constant-holder is a construct that would need its own named-local FAKE vet under .claude/rules/named-local-fake-exception.md at submission.
- probe: tmp/grind/func_80070C70/s12/e/f2.c (prim.code = 0xA written literally, var_s3 and its declaration deleted) and f1.c (func_8006E480(prim.p_geom, 0x60) written literally, c60 deleted), both against the 22-point body.
- result: f2 scores 22 at 194 instructions, identical to the body that declares var_s3, so that constant-holder is removable at zero cost - and this session's candidate.c drops it, halving the constant-holder vet surface. f1 scores 29 at 191 instructions, so c60 IS load-bearing (three instructions, seven points) and stays. Recorded so that no future session re-adds var_s3 or wastes a submission cycle arguing for it.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (s11 candidate.c body), sandbox --disable all

## [s12] IconC70 is a 0x20-byte record, so the trailing 24 bytes of the target's vars=80 frame belong to the icon struct and the placeholder `s16 sp50[12]` member list only needs its field NAMES recovered.
- mechanism: s1's frame arithmetic, s6's BB2_FRAME_DEBUG census and s10's stack_temp mode=26 size=32 all agreed that the local aggregate following prim occupies 32 bytes, and every session since s1 has modelled that as one 0x20 icon record with four known halfwords plus 24 unknown trailing bytes. The dispatch brief carries recovering the member list as the single largest correctness risk.
- probe: Read the callee and a sibling caller instead of guessing. asm/funcs/func_80069898.s copies its $a1 argument to $s1 at 800698A8 and dereferences it at exactly four offsets - lhu $v0,0x0($s1) / 0x2 / 0x4 / 0x6 - three times over, feeding each quadruple into a TILE prim at sh ...,0x8/0xA/0xC/0xE($s0); there is no other $s1-relative access in the function. asm/funcs/func_8006B120.s has addiu $sp,$sp,-0x68 with $s0 saved at 0x50 and the outgoing-args region at 0x0..0x17. Frame substitution measured with tmp/grind/func_80070C70/s12/h/h1..h3.c.
- result: The icon record is EIGHT bytes - four 16-bit fields at +0/+2/+4/+6 - not 0x20. func_8006B120's vars region is 0x18..0x4F = 56 bytes = prim(48 at 0x18) + icon(8 at 0x48), with its first callee-saved register immediately above the icon, and it passes addiu $a1,$sp,0x48 after storing only 0x48/0x4A/0x4C. So the 24 bytes at sp+0x50..0x67 in func_80070C70 belong to a DIFFERENT local - and a REFERENCED one: substituting an evidence-sized 8-byte IconC70 plus an unreferenced s32 sp50[6] scores 36 and the frame FALLS from 128 to 104, i.e. GCC 2.7.2 drops the unreferenced aggregate outright (h1/h2/h3 all 36 regardless of declaration position or element type). This is the opposite of the live-but-untouched-scalar behaviour in the Claude memory project/phantom-frame-slots-gcc272. func_800720FC shows the same idiom with more room (frame 0x98, $s0 at 0x70, icon still at 0x48, 32 spare bytes), so the extra local is a text1b-wide idiom and is inheritance for the active queue sibling func_8006F97C, which calls the same func_80069898. Banked as rejected/s12-icon8-plus-separate-24byte-local-36.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (s12 candidate.c body) for the frame measurement; the callee and sibling-caller evidence is static asm reading and is chassis-independent

## [s12] Two cheap statement-position levers reach the remaining prologue tie and the second loop's guard block: hoisting `var_s0 = 0;` above the if-guard, and materialising &prim into a pointer local.
- mechanism: (a) the target emits move s0,zero BEFORE the guard's loads whereas we emit it after the blez, so writing var_s0 = 0; above the if should move it; (b) the target materialises addiu $a0,$sp,24 very early, right after lw s2,100(v1), so taking &prim into a pointer local should let that address be computed once at the declaration rather than at each of the three call sites. s11 had measured six positions of var_s0 = 0; inside the guarded region, all 22 and byte-identical, but never a position outside it.
- probe: tmp/grind/func_80070C70/s12/e/e1.c and e2.c (var_s0 hoisted, on the plain body and on the carried-locals body); s12/g/g1.c and g2.c (s32 *pp = (s32 *)&prim; declared after ctx_or_var_s2 and at the top of the body); g3.c (the duplicated prim.zero1C / prim.mode pair reordered); g4.c (the ctx_or_var_s2 read hoisted to the top of the body).
- result: e1 = 54 at 193 insns, e2 = 71 at 194, g1 = g2 = 38 at 197, g3 = 24 at 194, g4 = 60 at 194. Every one is worse than 22; the pointer local costs three instructions outright and hoisting var_s0 = 0; above the guard loses one. Banked as rejected/s12-var_s0-zero-hoisted-above-guard-54.c and rejected/s12-prim-pointer-local-38.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (s12 candidate.c body), zero FAKE constructs beyond the inherited s32 c60, sandbox --disable all

## s13 (structural, 2026-09-10) - floor 22, unchanged; the icon-record correctness frontier is CLOSED

### H-s13-1 CONFIRMED - the sp+0x48 object is a `u16 rect[]` array, not a struct, and the 24 "mystery" bytes are its unwritten tail
s12's frontier item #1 called the 24 bytes at sp+0x50..0x67 "a hard prerequisite for any
submission" and its `IconC70 { s16 sp48, sp4A, sp4C, sp4E; s16 sp50[12]; }` placeholder
"KNOWN WRONG".  It is resolved from code already on main, with no new probe needed to find it:
**func_80069898 is already decompiled in this same TU** (src/text1b.c:5413) with the prototype
`void func_80069898(GameObj *arg0, u16 *arg1, s32 arg2)` - arg1 is a `u16 *` and the body reads
arg1[0], arg1[1], arg1[2], arg1[3].  Both callers already on main pass a plain local ARRAY:
func_8006BB68 (src/text1b.c:5822) declares `u16 rect[4];` and writes it in the order
rect[2] = 0xAF; rect[0] = 0xE8; rect[1] = 0x25; rect[3] = 1; - and func_8006DD94
(src/text1b.c:6065) does the same.  func_80070C70's target asm stores to sp+0x4C, sp+0x48,
sp+0x4A, sp+0x4E in **exactly that order** with the values 0xE7, 0xCC, 0x25, 1.  So the local
is a `u16 rect[]` whose address is passed, and the 24 trailing bytes are simply the array's
UNWRITTEN TAIL - the same OVERSIZED-LOCALS shape already accepted on main for func_8006DD94
(src/text1b.c:6032) and func_80041BF4 (src/text1a_post.c:404, `s16 rect[8]`).
MEASURED: `u16 rect[16]` (0x48..0x67 exactly) = 22/194 and BYTE-IDENTICAL to the s12
IconC70+sp50[12] body (opcode-column objdump diff empty over all 194 insns).  `s16 rect[16]` =
22.  `u16 rect[15]` = 22 (declared length is a RANGE [15,16] because stmt.c:3419 8-aligns a
BLKmode automatic - the same range caveat the func_8006DD94 record documents).
`u16 rect[4]` (written size only) = 36/194, so the 24 bytes are load-bearing.  This is
consistent with s12's measurement that an UNREFERENCED `s32 sp50[6]` is dropped: this tail is
referenced because the whole array is address-taken at the func_80069898 call.
CONSEQUENCE: the candidate no longer invents a type.  The `typedef struct IconC70` block and
the local re-declaration `extern s32 func_80069898(s32 a0, s32 *p, s32 mode);` (which shadowed
the real prototype with an `s32 *` second parameter) are both DELETED at submission - they are
expressed as //@sub directives on lines 1-2 of candidate.c.
Probe files: tmp/grind/func_80070C70/s13/p/R1_rect_u16_16.c, R2_rect_u16_4.c, R3_rect_s16_16.c,
R4_rect_u16_15.c.  Negative control banked as rejected/s13-rect4-written-size-only-36.c.

### H-s13-2 KILLED (instance) - the first loop body's statement-order axis is exhaustively swept and ABC is the unique optimum
All 6 orders of {prim.mode = var_s0 << 6; prim.link = *(s32 *)(arg0 + 0x10); prim.code = 0xA;}
crossed with both orders of the two trailing increments (var_s0 += 1 / prim.p_geom += 0xC) =
12 bodies, measured on the 22-point if-guarded do/while chassis: ABC = 22 (the candidate),
ACB = 24, CAB = 24, BAC = 29/193, BCA = 30/193, CBA = 30/193.  The increment order is
byte-neutral in all six.  s12's frontier item #3 listed "the four statements of the FIRST loop
body" as having had only two of six orders measured; all six are now measured and none beats
the incumbent.  Files tmp/grind/func_80070C70/s13/p/L1_*.c; representative banked as
rejected/s13-first-loop-body-order-sweep-29.c.

### H-s13-3 KILLED (instance) - the pre-guard statement positions do not move the guard block
The other half of s12's frontier item #3.  `prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);` has
exactly three legal positions in the pre-guard run (it cannot precede the SetDrawMode call that
reads prim.p_geom as an argument): after the AddPrim tail increment (the candidate) = 22/194,
before that increment = 25/197, immediately after SetDrawMode = 26/196.  The guard block's
instruction order is unchanged in all three.  Files p/PG_*.c; banked as
rejected/s13-preguard-pgeom-position-25.c.

### H-s13-4 KILLED (instance) - the scalar declaration-order axis is completely inert on this chassis
All 24 permutations of the four body scalar declarations (`s32 var_s0; s32 t; u8 code; s32 g;`)
are BYTE-IDENTICAL at 22/194.  This was the last cheap hypothesis for the prologue tie
(`addiu $a0,$sp,24` before vs after `move $s0,$zero`): pseudo numbering / declaration LUID does
not reach that sched1 priority tie.  Combined with s11's six source positions of `var_s0 = 0;`
and s12's pointer-local (38) and hoist-above-guard (54) probes, the prologue tie has now
resisted 33 distinct spellings.  Files p/DC_*.c; representative banked as
rejected/s13-scalar-decl-order-24-perms-inert.c.

### H-s13-5 KILLED (instance) - defeating LICM on the bound by reusing an existing local does not buy back the top-test chassis' orphan spill pseudos
The variable-reuse family (.claude/rules/defeat-licm-hoist-var-reuse.md: a multi-set pseudo is
not a loop.c movable, so the invariant is not hoisted) was the one untried route at making the
top-test chassis' three combine-orphaned pseudos not exist without paying for a fresh carrier.
Spelling the for-header bound as an assignment into an EXISTING body-written local -
`for (var_s0 = 0; var_s0 < (t = D_800A35B0 + D_800A3558 + 1); var_s0++)` - measures 69/194 on
the s12 31-point shape-exact top-test chassis, and 61/194 with `g` as the carrier.  Both are
30-38 points worse than the 31 incumbent, i.e. the hoist was not what was costing the frame.
On the 22-point do/while chassis the same construct is inert-to-worse (`g` 22 byte-identical,
`t` 23).  Files p/LR_*.c; banked as rejected/s13-licm-defeat-bound-var-reuse-61.c.

### H-s13-6 CONFIRMED (re-derivation, no new information) - two of s13's opening probes reproduced already-banked s11 kills
Before reading hypotheses.md end-to-end s13 re-derived the `||` operand swap (the target's mode
test branches on the sum first: `bnez` on `$a1 + (s16)$a2` at 28fb4, then `bne` against 2) and
the guard-copy parenthesisation, measuring 30 and 25 - matching rejected/s11-or-operands-sum-
first-30.c and rejected/s11-guard-bound-parenthesised-too-32.c.  Recorded so the next session
does not spend the same three probes: the target's branch TOPOLOGY in the mode test is the
sum-first order, our incumbent 22-point body emits the D_800A35BC-first order, and that
structural divergence is nevertheless the CHEAPER of the two on the score metric because the
sum-first spelling ripples into the pre-loop block ($v1 -> $a1 on the `addiu #,#,72` /
`sw #,28(sp)` pair, 2 points) and loses the loop-body scheduling that currently aligns.

### s13 OBJECT-LEVEL NOTE - why the carried-locals family cannot reach the target's $a1/$a2
Recorded because s11/s12 both measured this family without stating the register-lifetime fact
that explains the result.  $a1 and $a2 are CALLER-SAVED and the second loop body contains a
`jal` (func_8007352C at 28fe8), so the target's $a1/$a2 do NOT survive the call: the tail at
28ffc-29004 RELOADS all three values (`lhu a2,0(gp)`, `lh v0,0(gp)`, `lw a1,0(gp)`) every
iteration, and the guard block supplies the entry copies.  The literal C transcription of that
- two locals reassigned at the bottom of the body - is exactly what s12 measured at 42/195
(and 27/194 carrying D_800A35B0 alone).  The reason it costs is not the reload but the extra
insn: the target's shape needs three gp loads in the tail where our re-read spelling needs two,
and it pays that back inside the loop body, a trade the C spelling cannot express because GCC
CSEs the two locals' initialisers against the guard's own reads.

## [s13] The object at sp+0x48 in func_80070C70 is a u16 rect[] array whose unwritten tail occupies the 24 bytes at sp+0x50..0x67, not the invented IconC70 struct with an s16 sp50[12] placeholder member.
- mechanism: Not codegen - source correctness recovered from code already on main. func_80069898 is decompiled at src/text1b.c:5413 as void func_80069898(GameObj *arg0, u16 *arg1, s32 arg2) and reads arg1[0..3]; its two existing callers (func_8006BB68 src/text1b.c:5822, func_8006DD94 src/text1b.c:6065) each declare a local u16 rect[4] and write it in the order rect[2], rect[0], rect[1], rect[3] - the exact offsets (sp+0x4C, 0x48, 0x4A, 0x4E) and order func_80070C70's target asm uses. The trailing 24 bytes are the array's unwritten tail, referenced because the whole array is address-taken at the call, which is why GCC 2.7.2 keeps them (s12 measured that an UNREFERENCED s32 sp50[6] is dropped, frame 128 -> 104). stmt.c:3419 8-aligns the BLKmode automatic, so the declared length is a range.
- probe: Replaced IconC70 icon; with u16 rect[N]; on the 22-point if-guarded do/while chassis and measured N = 4, 15, 16, with the sp50 member and the shadowing extern s32 func_80069898(s32,s32*,s32) re-declaration removed via //@sub. Byte-identity checked by diffing the objdump opcode column of the two text1b.o files over all 194 instructions.
- result: u16 rect[16] = 22/194 and BYTE-IDENTICAL to the s12 IconC70+sp50[12] body (empty opcode-column diff, 194 lines). s16 rect[16] = 22. u16 rect[15] = 22 (declared length is a RANGE [15,16], the same caveat recorded for func_8006DD94). u16 rect[4] = 36/194, so the 24 tail bytes are load-bearing. The candidate now invents no type, and the construct is the same OVERSIZED-LOCALS shape already accepted on main at src/text1b.c:6032 and src/text1a_post.c:404 (extending the LIVE address-taken object, not adding a dead pad).
- verdict: CONFIRMED

## [s13] The first loop body's four statements have an order, among the six orders of its three struct stores crossed with both orders of its two trailing increments, that beats the candidate's ABC order on the if-guarded do/while chassis.
- mechanism: sched1 / reorg outcome downstream of what else is ready in the same cycle; s12's frontier item 3 recorded that only two of the six orders had ever been measured and that the neighbouring duplicated prim.zero1C/prim.mode pair moved by 2 points when reordered.
- probe: Generated all 12 bodies (6 statement orders x 2 increment orders) with tmp/grind/func_80070C70/s13/gen1.py and scored each with the s12 sandbox harness.
- result: ABC (the candidate) = 22/194 is the unique optimum; ACB = 24, CAB = 24, BAC = 29/193, BCA = 30/193, CBA = 30/193. The two trailing increments (var_s0 += 1 / prim.p_geom += 0xC) are byte-neutral in all six. The axis is exhaustively swept.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (memory/grind/func_80070C70/candidate.c body as inherited from s12), FAKE constructs present: the s32 c60 = 0x60; constant-holder local and the oversized icon/rect local; sandbox --disable all

## [s13] Moving prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8); to one of its other two legal positions in the pre-guard run changes the guard block's instruction order in our favour.
- mechanism: Same sched1 readiness question as the first-loop sweep; the statement's load is what the target issues as lw v1,8(s2) before the guard branch with the store sw v1,24(sp) in the delay slot, so its source position was the remaining unswept half of s12's frontier item 3.
- probe: Enumerated the three legal positions (it cannot precede the SetDrawMode call that reads prim.p_geom as an argument) with tmp/grind/func_80070C70/s13/gen2.py and scored each.
- result: After the AddPrim tail increment (the candidate) = 22/194; immediately before that increment = 25/197; immediately after SetDrawMode = 26/196. The guard block's instruction order is unchanged in all three.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (s12 candidate.c body), FAKE constructs present: s32 c60 = 0x60; and the oversized icon/rect local; sandbox --disable all

## [s13] Permuting the declaration order of the four body scalars (var_s0, t, code, g) moves the prologue tie between addiu $a0,$sp,24 and move $s0,$zero.
- mechanism: Declaration order sets pseudo numbers and hence LUIDs, which is GCC 2.7.2 sched.c's tie-break when two ready insns carry equal INSN_PRIORITY - and the prologue divergence is exactly a two-insn swap of two independent ready insns in the same basic block.
- probe: Generated all 24 permutations with tmp/grind/func_80070C70/s13/gen4.py and scored each on the 22-point chassis.
- result: All 24 permutations are BYTE-IDENTICAL at 22/194. The scalar declaration-order axis is completely inert here. Combined with s11's six source positions of var_s0 = 0; and s12's pointer-local (38) and hoist-above-guard (54) probes, the prologue tie has now resisted 33 distinct spellings.
- verdict: KILLED
- kill_scope: instance
- measured_on: the 22-point if-guarded do/while chassis (s12 candidate.c body), FAKE constructs present: s32 c60 = 0x60; and the oversized icon/rect local; sandbox --disable all

## [s13] Spelling the second loop's bound as an assignment into an existing body-written local, so the bound's pseudo is multi-set and loop.c cannot treat it as a movable, removes the three combine-orphaned spill pseudos that cost the top-test chassis 24 frame bytes.
- mechanism: .claude/rules/defeat-licm-hoist-var-reuse.md - a multi-set pseudo is not a loop.c movable, so the invariant is not hoisted; s10's H-s10-4 had found that the only input shape that moves the orphan count on the top-test chassis is whether the bound's 16-bit read is sign-extended, which made the hoisted-and-split sign-extension the suspect carrier of the orphans.
- probe: On the s12 31-point shape-exact top-test for chassis (tmp/grind/func_80070C70/s12/a/a3.c) rewrote the for-header bound as (t = D_800A35B0 + D_800A3558 + 1) and as (g = ...), reusing locals the loop body already writes; also measured both on the 22-point do/while chassis. tmp/grind/func_80070C70/s13/gen3.py.
- result: Top-test chassis: t carrier = 69/194, g carrier = 61/194, against the 31 incumbent - 30 to 38 points worse, so the hoist was not what was buying the frame. do/while chassis: g = 22 (byte-identical, the store is dead), t = 23. The variable-reuse family has no purchase on this function's orphan pseudos.
- verdict: KILLED
- kill_scope: instance
- measured_on: both the s12 31-point shape-exact top-test for chassis (tmp/grind/func_80070C70/s12/a/a3.c, which carries the extern s16 D_800A3558 //@sub) and the 22-point if-guarded do/while chassis; FAKE constructs present on both: s32 c60 = 0x60; and the oversized icon/rect local; sandbox --disable all
