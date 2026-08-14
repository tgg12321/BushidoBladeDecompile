# Hypotheses — func_800174F4

## CONFIRMED (session 1)

**H1 — the inherited `a0_temp` copy local in the case-1/2 loop is spurious.**
Mechanism: the extra local gave the call's first argument its own pseudo,
which regmove/local-alloc tied to `$v0` (the callee's return, copied into it
on the back edge), forcing a `move v0,s2` + `move a0,v0` pair where target
passes `$s2` straight into `$a0`. Probe: delete the local, keep the
`goto`-form loop. Result: floor unchanged at 25 but the copy pair disappeared
and the loop body became one insn shorter — the local is dead weight and its
removal is a prerequisite for H2/H3. CONFIRMED.

**H2 — the case-1/2 loop counter must be its OWN local, not a reuse of the
drawenv-address variable.** Mechanism (global.c:allocno_compare, priority =
floor_log2(n_refs)*n_refs/live_length): reusing one C variable for the
buffer address AND the counter fuses them into one long-lived, many-ref
allocno whose priority beats the 0xF0/height allocno, so it wins $s0 and
pushes the height web to $s1 — the exact inverse of target. Splitting the
counter out shortens the buffer-address allocno's live range, flipping the
order. Probe: `s32 i;` declared in the case-1/2 block, `i = 0; ... i++ ...`.
Result: **floor 25 -> 18**; the five top-of-function $s0/$s1 swaps and the
case-20 `div` register swap all resolved. CONFIRMED.

**H3 — the loop's `i++` belongs AFTER the call statement, not before it.**
Mechanism: with `i++` first, GCC's loop rotation hoists the increment into
the pre-header (filling the guard branch's delay slot) and duplicates it on
the back edge, leaving the arg copy `move a0,s2` to fill the `jal` delay
slot. Target has the inverse: `move a0,s2` at the top of the loop body
(duplicated into the back-edge `j` delay slot by reorg.c's
fill_slots_from_thread) and `addiu s0,s0,1` in the `jal` delay slot. Writing
the increment after the call reproduces target's block order exactly.
Probe: move `i++;` below `prim = func_8005D554(...)`. Result: **floor 18 ->
14, build_insns 135 -> 136 (== target)**, and every remaining diff is a
register-name substitution. CONFIRMED.

## KILLED (session 1 — do NOT re-propose; forms banked in rejected/)

**K1 — rewrite the case-1/2 loop as `do { ... } while (i < h);`.**
Result: floor 25 -> **34**, build_insns 135. GCC rotates the loop differently
AND materialises an `andi a0,a0,0xffff` (the `unsigned short` limit stops
being provably narrow once it is the do-while condition operand), and the
limit migrates out of the callee-saves entirely. The target's shape is the
`goto`-form bottom-tested loop with a separate entry guard. KILLED.
`rejected/do-while-loop-rotation.c`

**K2 — give every role its own local (`i` counter, `n` limit, `tv` table
value all split out of `h`).** Result: floor 25 -> **32**, build_insns 133
(three instructions LOST). The `h` reuse across {0xF0/div, loop limit, table
value} is load-bearing: target keeps all three in $s1, which only happens
when they are webs of one variable. KILLED.
`rejected/distinct-locals-every-role.c`

**K3 — split only the table value (`tv`) out of `h`, keeping the counter
split from H2.** Result: floor 18 -> **32**, build_insns 133. Same loss as
K2; isolates the damage to the `tv` split specifically. KILLED.
`rejected/split-table-value.c`

## LIVE FRONTIER (for the next session)

**F1 — cluster (B): flip the two remaining callee-save webs (8 of the 14).**
Ours: counter->$s1, loop-limit web->$s0, table-value web->$s0.
Target: counter->$s0, loop-limit web->$s1, table-value web->$s1.
Mechanism: `global.c:allocno_compare` orders allocnos by
floor_log2(n_refs)*n_refs/live_length and hands out `reg_alloc_order` in that
order, so whichever of {counter, limit-web} is processed first takes $s0.
Next probe: get exact `n_refs` / `live_length` / allocno numbers for these
webs out of a cc1 `-dg` dump of the CURRENT (floor-14) source — the session-1
dump `tmp/grind/func_800174F4/s1/ings.i.greg` was taken on the BASELINE body,
so re-dump first. Then look for an honest C shape that lengthens the counter's
live range or shortens the limit web's (e.g. where the guard `if (h != 0)`
sits relative to `i = 0`, whether `i = 0` precedes or follows the `rand()`
call, whether the guard tests `h` or the freshly-computed `v0 + 4`).
Statement ORDER around the loop pre-header is the untried surface; declaration
order/naming is NOT (it does not enter the priority formula, only the
tie-break, and these webs are not tied).

**F2 — cluster (A): the switch selector wants $a1, we emit $v1 (6 of the 14).**
This is the sole surviving regfix rule's exact scope (`$3 <-> $5 @ 27-41`).
Target reads `lbu a1,%gp_rel(D_800A3768)` for the `switch` value AND re-reads
the same global into `$a1` inside the case-1/2 loop as func_8005D554's second
argument. Hypothesis: in the original C the switch is performed on an
expression whose pseudo carries a call-argument preference into $a1 (GCC
2.7.2 `global.c:expand_preferences` propagates arg-position preferences
through copies), e.g. the selector is read into a named local that is then
passed as the second argument, rather than each site re-reading the global
independently. Next probe: read `g_disp_enable` once into a local `mode`,
`switch (mode)`, and pass `mode` (not the global) to func_8005D554 /
func_8005D46C-adjacent calls — then check whether the re-read `lbu` inside the
loop survives (target HAS a per-iteration `lbu a1`, so a naive single-read
hoist will lose an instruction; the variant to test is a single read for the
switch plus the per-iteration re-read still spelled as the global).

**F3 — if F1/F2 both plateau: run a directed permuter campaign** from the
floor-14 base (it is a clean 136-vs-136 register-name-only residual, which is
exactly the shape decomp-permuter's register scoring handles well). Build the
single-function `target.o` from `asm/funcs/func_800174F4.s` + prelude per
`.claude/rules/difficult-is-not-impossible.md` §3 so the base score is the
real weighted diff. Not attempted in session 1.

## [s1] The inherited `a0_temp` copy local in the case-1/2 loop is spurious and blocks target's direct $s2 -> $a0 argument pass.
- mechanism: The extra local gets its own pseudo which regmove/local-alloc ties to $v0 (the callee return copied into it on the back edge), forcing `move v0,s2` + `move a0,v0` where target passes $s2 straight into $a0.
- probe: Delete the local, keep the goto-form loop; sandbox --disable all.
- result: Floor unchanged at 25 but the copy pair disappeared and the loop body shortened by one insn; prerequisite for H2/H3.
- verdict: CONFIRMED

## [s1] The case-1/2 loop counter must be its own local, not a reuse of the drawenv-address variable.
- mechanism: global.c:allocno_compare orders allocnos by floor_log2(n_refs)*n_refs/live_length; fusing buffer-address + counter into one long-lived many-ref allocno makes it outrank the 0xF0/height allocno, so it takes $s0 and pushes the height web to $s1 -- the inverse of target.
- probe: Declare `s32 i;` in the case-1/2 block and use it for the counter, leaving `env` for the buffer address only.
- result: Floor 25 -> 18; the five top-of-function $s0/$s1 swaps and the case-20 `div` register swap all resolved.
- verdict: CONFIRMED

## [s1] The loop's `i++` belongs after the call statement, not before it.
- mechanism: With `i++` first, loop rotation hoists the increment into the pre-header (filling the guard branch's delay slot) and duplicates it on the back edge, leaving the arg copy to fill the jal delay slot. Target has `move a0,s2` as the first loop-body insn (duplicated into the back-edge j delay slot by reorg.c fill_slots_from_thread) and `addiu s0,s0,1` in the jal delay slot.
- probe: Move `i++;` below `prim = func_8005D554(...)`; sandbox --disable all.
- result: Floor 18 -> 14 and build_insns 135 -> 136 (== target); every remaining diff is a register-name substitution, zero ins/del/reorder.
- verdict: CONFIRMED

## [s1] Spelling the case-1/2 loop as `do { ... } while (i < h);` reproduces target's loop.
- mechanism: Would rely on GCC emitting the same bottom-tested loop from the structured form as from the goto form.
- probe: Rewrite the loop as do-while with the `if (h != 0)` guard; sandbox --disable all.
- result: Floor 25 -> 34 (insns 135). GCC rotates differently and materialises `andi a0,a0,0xffff` (the unsigned-short limit stops being provably narrow as the loop-condition operand); the limit leaves the callee-saves. Banked as rejected/do-while-loop-rotation.c.
- verdict: KILLED

## [s1] Giving every role its own local (counter i, limit n, table value tv all split out of `h`) is the natural original source and will fix the callee-save swaps.
- mechanism: Would rely on RA coalescing independent short-lived pseudos into the same hard regs as target.
- probe: Split i, n and tv into distinct locals; sandbox --disable all.
- result: Floor 25 -> 32 with build_insns 133 (three instructions lost). Target keeps 0xF0/div-dividend, loop limit and table value ALL in $s1, which only happens when they are webs of ONE C variable. Banked as rejected/distinct-locals-every-role.c.
- verdict: KILLED

## [s1] Splitting only the D_800A37A8[] table value out of `h` (keeping the H2 counter split) fixes the case-20 tail register names.
- mechanism: Would rely on a fresh short-lived allocno for the table value landing in $s1.
- probe: Declare `unsigned short tv;` in the case-20 else block; sandbox --disable all.
- result: Floor 18 -> 32, build_insns 136 -> 133. Isolates the K2 damage to the tv split specifically; the `h` reuse for the table value is load-bearing. Banked as rejected/split-table-value.c.
- verdict: KILLED
