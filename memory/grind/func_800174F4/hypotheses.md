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

## CONFIRMED (session 2)

**H4 — `i = 0;` belongs INSIDE the `if (h != 0)` guard block, not in the loop
pre-header.** Mechanism: the initialisation's position decides which of
{counter, loop-limit} ends up in $s0. With `i = 0;` in the guard block the
counter takes $s0 and both the loop-limit and the D_800A37A8[] table-value
webs take $s1 — target's exact assignment. Probe: three pre-header positions
(first statement of the fade block, after `v0 = rand();`, after
`h = v0 + 4;`) vs inside the guard. Result: **floor 14 -> 8**, insns 136
throughout; all three pre-header positions measure 14. CONFIRMED.

## KILLED (session 2 — do NOT re-propose; forms banked in rejected/)

**K4 — session-1's F1 mechanism ("global.c:allocno_compare priority decides
the $s0/$s1 split").** Instrumented cc1 (BB2_ALLOC_DEBUG) on five variants:
the floor-8 form and the floor-14 pre-call forms have BYTE-IDENTICAL allocno
tables (order, n_refs, live_length, priority) yet differ by 6 points of
register naming. The flip is decided downstream of global.c — local-alloc
quantities / reload. The mechanism as stated is dead; the phenomenon is real.
`rejected/i-init-before-call.c`

**K5 — lengthening the `h` allocno's live range to lower its priority.**
`h = 0xF0;` hoisted to the top of the function / before `env` / to just
before SetDefDrawEnv, on two bases = six combinations, all 14, and the cc1
dump shows `h`'s live_length does not move at all (the constant is
materialised where the scheduler wants it). KILLED.
`rejected/h-def-hoisted.c`

**K6 — session-1's F2 mechanism ("give the selector an $a1 argument-position
preference via expand_preferences").** Ten honest selector-local spellings
(cache-and-switch; u8 local; local re-read at the loop site, at the else-call
site, at all three sites; no switch-local at all) ALL score exactly 8 with
136 insns — combine folds the pseudo→$a1 copy into the load before global
alloc, so no copy insn survives for expand_preferences to see. The two
spellings that DO create a surviving copy both cost instructions: passing the
cached local as func_8005D554's 2nd argument = 29 (138 insns), one read
serving both the 0xFF early-out and the switch = 21 (137 insns). KILLED as
stated. `rejected/selector-local-as-call-arg.c`,
`rejected/selector-single-read.c`

**K7 — assorted structural respellings of the loop and its types**, each
measured once and worse than 8: loop tail `if (i < h) goto inner_loop;` (14),
guard on `(v0 + 4)` (15), counter as the rand() temp (12 / 137 insns),
counter narrowed to `unsigned short` (16 / 137), `h` widened to `s32` (20),
count-down loop (35 / 133), counter declared at function scope (8, no
change), `h` carrying the rand() temp (8, no change). KILLED.
`rejected/{guard-on-v0-expression,counter-carries-rand-temp,counter-u16,h-widened-s32,countdown-loop}.c`

## LIVE FRONTIER (for the next session)

> **NOTE (session 2): F1 and F2 below are the SESSION-1 statements and are now
> partly historical.** F1's cluster (B) is CLOSED by H4 (the callee-save webs
> match; only the 2-point delay-slot residue of it remains, see F4). F1's and
> F2's stated MECHANISMS are both KILLED (K4, K6). Read F4/F5 first.

**F4 — the floor-8 residue of cluster (B): 2 pts of delay-slot placement
(insns 50 and 54).** Target fills the `rand()` jal delay slot with
`move s0,zero` (the `i = 0;`) and leaves a nop in the guard branch's delay
slot; ours is the mirror image. Read from
`tools/gcc-2.7.2/reorg.c:fill_simple_delay_slots`: a CALL_INSN's slot is
filled by the BACKWARD search, and the forward "beyond the insn" search
cannot reach our guard-block `i = 0;` (the intervening `andi`/`addiu` both
reference the call's $v0 result and the search cannot cross the `beqz`).
So target's `i = 0;` is PRE-call — the exact placement that costs us $s0 for
the counter (14). The two goals are in direct tension; resolving it means
finding a C shape where the counter keeps $s0 with a pre-call
initialisation. Next probe: instrument LOCAL-alloc, not global —
`BB2_QTY_DEBUG` (local-alloc.c:1518/1585) and `BB2_SUGG_DEBUG`
(local-alloc.c:1437/1447/2215) on the floor-8 form vs
`rejected/i-init-before-call.c`, and read which quantity gets $s0 and why
(`qty_phys_sugg` / suggested-register propagation is the likely
discriminator). This is a FORENSICS probe, not a structural one.

**F5 — cluster (A): the switch selector wants $a1, we emit $v1 (6 of the 8;
the sole regfix rule's exact scope).** The expand_preferences story (F2) is
dead (K6): no honest spelling leaves a pseudo→$a1 copy alive at global-alloc
time. What remains, measured this session: `config/mips/mips.h` defines no
REG_ALLOC_ORDER, so `global.c:find_reg` hands out the lowest-numbered free
register of the class; the selector allocno (nrefs 6, live_length 10,
allocated 2nd overall) takes $v1(3) because $v0(2) is already held. Reaching
$a1(5) needs regs 2, 3 AND 4 excluded by CONFLICT. Next probe: find an honest
C shape in which the selector's live range extends past the
`func_8005D46C` call region (target insns 43-45: `jal` + `move a0,s2` in the
delay slot + `lbu v1` for g_disp_fade) so that it conflicts with the $a0 arg
pseudo and the $v1 fade pseudo simultaneously — e.g. a spelling in which the
dispatch value is genuinely still needed after that call. Verify the
conflict hypothesis FIRST by reading the conflict matrix out of a cc1 `-dg`
`.greg` dump (`;; Register dispositions` + the conflict lists) for the
floor-8 form: if the selector allocno's conflict set already contains 2/3/4
the hypothesis is wrong and the answer lies in local-alloc instead.

**F1 [session 1, superseded] — cluster (B): flip the two remaining callee-save webs (8 of the 14).**
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

## [s2] The case-1/2 loop counter's initialisation `i = 0;` belongs INSIDE the `if (h != 0)` guard block rather than in the loop pre-header, and its position is what decides which of {counter, loop-limit} takes $s0.
- mechanism: The counter's live range starts inside the guard-taken block instead of spanning the rand() call, which flips the downstream register assignment so the counter takes $s0 and both the loop-limit web and the D_800A37A8[] table-value web take $s1 - target's exact assignment.
- probe: Four placements of `i = 0;` measured with `sandbox func_800174F4 --disable all`: first statement of the fade block (the session-1 form), after `v0 = rand();`, after `h = v0 + 4;`, and inside the `if (h != 0)` guard.
- result: Inside the guard: floor 14 -> 8, build_insns 136 == target_insns 136. All three pre-header placements measure 14. The aligned diff shows insns 50/52/53/58/60/116/119/120 all resolved; only insns 28/30/31/33/35/41 (the switch selector) and a 2-point delay-slot placement at insns 50/54 remain.
- verdict: CONFIRMED

## [s2] Session 1's F1 mechanism - that global.c:allocno_compare priority (floor_log2(n_refs)*n_refs/live_length) decides the $s0/$s1 split between the counter and the loop-limit web.
- mechanism: Would require the two competing allocnos to change relative priority between the matching and non-matching spellings.
- probe: Instrumented cc1 (BB2_ALLOC_DEBUG, hooks at tools/gcc-2.7.2/global.c:379 and :605) run on five variants: the session-1 baseline, the floor-8 in-guard form, and three pre-call `i = 0;` forms; per-allocno order/n_refs/live_length/priority tables compared.
- result: The floor-8 form and the floor-14 pre-call forms produce BYTE-IDENTICAL allocno tables (ord 0-8 identical, same pseudo numbers, same n_refs, same live_length, same priority) yet differ by 6 points of $s0/$s1 naming. Only the session-1 baseline differs (pseudo 85: live_length 13 / pri 6153 / ord 3 vs live_length 9 / pri 8888 / ord 2). Global allocno priority therefore cannot be the discriminator; the flip happens downstream in local-alloc quantity assignment / reload.
- verdict: KILLED

## [s2] Session 1's F2 mechanism - that reading the dispatch global into a named local gives the switch-selector pseudo an argument-position preference for $a1 through global.c:expand_preferences.
- mechanism: expand_preferences propagates hard-reg preferences only BETWEEN two pseudo allocnos, so an $a1 preference requires a `(set (reg a1) (reg <selector>))` copy insn surviving to global-alloc time.
- probe: Ten honest spellings measured: cache-and-switch on a named local; the same with the local typed u8; local re-read from the global at the loop call site, at the else call site, and at all three sites; no switch-local at all; plus the two spellings that DO create a surviving copy (pass the cached local as func_8005D554's 2nd argument; one read serving both the 0xFF early-out and the switch).
- result: All ten preference-seeking spellings score exactly 8 with 136 insns - byte-identical output, because combine folds the pseudo-to-$a1 copy into the load before global alloc, so no copy insn ever reaches expand_preferences. The two spellings that keep a real copy cost instructions: as-call-argument = 29 (138 insns, the local goes callee-save and target's per-iteration `lbu a1` becomes a register move); single-read = 21 (137 insns, target reads the global twice at insns 4 and 28).
- verdict: KILLED

## [s2] Lengthening the `h` allocno's live range (by hoisting `h = 0xF0;` earlier) lowers its allocation priority enough to let a pre-call `i = 0;` keep $s0 - i.e. it would satisfy the delay-slot requirement and the register requirement at once.
- mechanism: priority = floor_log2(n_refs)*n_refs/live_length, so a longer live range lowers priority and moves the allocno later in the allocation order.
- probe: `h = 0xF0;` placed at the top of the function, immediately before `env = (s32)sp20;`, and immediately before the SetDefDrawEnv call, on each of two bases (pre-call `i = 0;` and the session-1 baseline) - six combinations, all scored, plus BB2_ALLOC_DEBUG dumps of two of them.
- result: All six score 14, and the cc1 dump shows the `h` allocno's live_length does not move at all (the 0xF0 constant is materialised where the scheduler wants it, not where the C statement sits). The axis is inert.
- verdict: KILLED

## [s2] Assorted structural respellings of the loop and of the two locals' types reach below 8.
- mechanism: Loop-shape and type-width changes alter the compare, the induction variable's refs, and the narrowing masks GCC emits.
- probe: Seven forms measured on the floor-8 base: loop tail respelled `if (i < h) goto inner_loop;`; guard testing `(v0 + 4)` instead of `h`; the counter local doubling as the rand() temp; the counter narrowed to `unsigned short`; `h` widened to `s32`; a count-down loop; the counter declared at function scope; `h` carrying the rand() temp.
- result: None improves: 14, 15, 12 (137 insns), 16 (137 insns), 20, 35 (133 insns), and two that are exactly 8 with identical output (function-scope counter, `h` as the rand() temp). Floor 8 stands.
- verdict: KILLED
