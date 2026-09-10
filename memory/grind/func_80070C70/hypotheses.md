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
