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
