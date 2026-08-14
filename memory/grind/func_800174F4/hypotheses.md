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

## CONFIRMED (session 3)

**H5 � the $s0/$s1 flip between the loop counter and `h` is decided by
`global.c:allocno_compare` priority, not by local-alloc (CORRECTS s2's K4).**
Mechanism: priority = floor_log2(n_refs)*n_refs/live_length*10000. The counter
(pseudo 85) has n_refs 4; its live_length is 9 when `i = 0;` sits inside the
`if (h != 0)` guard (pri 8888) and 10 when it sits one statement earlier
(pri 8000). `h` (pseudo 73) is constant at n_refs 7 / live_length 16 /
pri 8750. Whichever is higher is allocated first and takes $s0(16).
Probe: cc1 `-dg` dumps of the floor-8 and floor-14 forms compared header to
header, plus BB2_ALLOC_DEBUG priority tables. Result: the ONLY difference in
the entire `.greg` header is the allocno order list (`84 120 85 73 ...` vs
`84 120 73 85 ...`); conflict lists are byte-identical. CONFIRMED, and it
re-opens global.c as the lever surface that s2 closed.

## KILLED (session 3 � do NOT re-propose; forms banked in rejected/)

**K8 � the natural structured loop (`for (i = 0; i < h; i++)` or the
equivalent `while`) is the original source shape.** Both score 39 with 135
insns: GCC 2.7.2's loop rotation emits a two-instruction entry test
(`slt` + branch) where target has a single `beqz $s1`, and the body loses an
instruction. The goto-form bottom-tested loop with an explicit `if (h != 0)`
guard is load-bearing. KILLED. `rejected/natural-for-loop.c`,
`rejected/for-header-init-infinite-body.c` (the `for (i = 0; ; )` hybrid, 33).

**K9 � `env` (the drawenv-buffer address variable) doubles as the case-1/2
loop counter.** Target reuses $s0 for both roles, which reads like one C
variable. Measured on all four init placements: 22 (init in guard), 21
(pre-`rand()`), 21 (fade-block top), 21 (after `h = v0 + 4;`). The $s0 sharing
in target is the allocator reusing a dead register, not a shared C variable.
KILLED. `rejected/env-var-doubles-as-counter.c`

**K10 � raise the counter allocno's n_refs (to 6+) so it keeps $s0 with a
pre-call initialisation.** The only honest way to add refs without adding
instructions is variable reuse: share one function-scope `s32 i` between the
case-1/2 loop counter and the case-20 `D_800A37C0` counter (disjoint switch
arms). Result: 18 with the init in the guard, 24 with it pre-`rand()` � the
shared pseudo's live_length grows faster than its ref count helps, and the
case-20 code degrades. KILLED. `rejected/counter-shared-with-case20.c`

**K11 � lower `h`'s allocation priority by lengthening its live range at the
case-20 end** (hoist `h = D_800A37A8[a0_val];` to the top of its else block).
The arithmetic requires live_length > 22.7 (from 16) to drop `h` under the
pre-call counter's 6153. Result: 14 on the floor-8 base and 19 combined with a
pre-call init � the hoist reorders case 20's stores and costs more than it
buys. KILLED. `rejected/h-table-load-hoisted-case20.c` (this joins K5, which
killed the same idea from the definition end.)

**K12 � an extra `i = 0;` before `v0 = rand();` (keeping the in-guard one)
fills the jal delay slot for free.** A dead store to a local is a sanctioned
family, so it was measurable; it scores 14. The dead pre-store still starts the
counter allocno's live range, so the priority loss is identical to the
single-pre-call-store form and nothing is gained. KILLED.
`rejected/i-init-duplicated-dead-store.c`

**K13 � placing `i = 0;` anywhere earlier in case 1/2** (before the
`g_disp_fade` test with `i` block-scoped to the case: 18 / 137 insns; at the
very top of case 1/2 before `func_8005D46C`: 15 / 136). Both are worse than
both the floor-8 and the floor-14 placements. KILLED.
`rejected/i-init-top-of-case.c`, `rejected/i-init-first-stmt-of-fade-block.c`

## LIVE FRONTIER (for session 4) � F4/F5 restated with session-3 arithmetic

**F4' � the 2-point delay-slot residue, now an inequality.** `i = 0;` must be
the first statement of the `if (g_disp_fade != 0)` block for reorg.c's
backward search to pull `move s0,zero` into the `jal rand` delay slot. From
there the counter allocno has live_length ~13 and priority 6153, losing $s0 to
`h` (8750) and costing 6 points to buy 2. The flip needs EITHER counter
n_refs >= 6 (K10 killed the two honest reuse spellings) OR `h` live_length
> 22.7 (K5 and K11 killed both ends of that). Untried surfaces: (a) change
`h`'s TYPE or the case-20 arithmetic so `h`'s n_refs drops from 7 to 4
(floor_log2(4)*4/16 = 5000 < 6153) WITHOUT splitting the variable � e.g.
spelling the `|= 0x8000` and the table load as a single expression, or
computing the func_80060414 argument without a second read of `h`; (b) a form
in which the counter is an induction variable GCC strength-reduces, changing
its ref count in RTL rather than in C. Both are cheap to measure with
`tmp/grind/func_800174F4/s3/alloc.sh` reading the priority directly instead of
inferring it from the score.

**F5' � cluster (A), 6 points, is a two-condition problem with a named
mechanism.** The selector allocno needs BOTH reg 3 and reg 4 in
`hard_reg_conflicts U regs_someone_prefers` for `global.c:find_reg` to hand it
$a1(5); it currently has conflicts {2,29} and empty preference sets. The
working precedent inside the same function is pseudo 99, which skips reg 4
purely because a lower-priority CONFLICTING allocno (101) prefers 4. So the
shape to search for is one where the switch selector conflicts with an
argument-preferring allocno while still dying before `func_8005D46C` � every
spelling measured so far either folds the copy into the load (8, no change) or
pushes the selector across the call into a callee-save (29/138). Verify any
candidate with `findreg.sh <tag> 120` BEFORE scoring it: the dump tells you
directly whether the exclusion sets moved, which the score does not.

**F6 � the directed permuter campaign (s2's F3) is still unrun** and is now
seeded from a 136-vs-136, 8-point, register-name-only base. Build the
single-function `target.o` from `asm/funcs/func_800174F4.s` + the permuter
prelude per `.claude/rules/difficult-is-not-impossible.md`, seed from
`memory/grind/func_800174F4/candidate.c`, and run a fresh-seed campaign with
`tools/permuter_campaign.py` � launching, `wait --dir <ws>` and
`harvest --stop` all inside one turn.

## [s3] The $s0/$s1 flip between the case-1/2 loop counter and `h` is decided downstream of global.c (session 2's K4).
- mechanism: s2 read the BB2_ALLOC_DEBUG tables of the floor-8 and floor-14 forms as byte-identical and concluded local-alloc quantity assignment / reload must be the discriminator.
- probe: cc1 `-dg` dumps (`;; N regs to allocate`, per-allocno conflict lists, `;; Register dispositions`) of the floor-8 form and the floor-14 pre-guard form, compared header to header, plus fresh BB2_ALLOC_DEBUG priority tables for both.
- result: The tables are NOT identical - the allocno ORDER swaps (`84 120 85 73 ...` vs `84 120 73 85 ...`), which is precisely the $s0/$s1 flip. Counter pseudo 85: livelen 9 / pri 8888 (floor 8) vs livelen 10 / pri 8000 (floor 14); `h` pseudo 73 constant at nrefs 7 / livelen 16 / pri 8750. Conflict lists identical. The lever is global.c:allocno_compare.
- verdict: KILLED

- ## [s3] The natural structured loop (`for (i = 0; i < h; i++)`, or `i = 0; while (i < h)`) is the original source shape and reproduces target's guard + bottom-tested loop.
- mechanism: GCC 2.7.2's loop rotation turns a top-tested loop into an entry guard plus a bottom-tested loop, which is target's shape.
- probe: Both spellings measured with `sandbox func_800174F4 --disable all` on the floor-8 base.
- result: Both 39 with 135 insns. The rotation emits a two-instruction entry test (`slt` + branch) where target has a single `beqz $s1`, and the loop body loses an instruction. The goto-form loop with the explicit `if (h != 0)` guard is load-bearing.
- verdict: KILLED

## [s3] `env`, the drawenv-buffer address variable, doubles as the case-1/2 loop counter (target reuses $s0 for both roles).
- mechanism: One C variable serving both roles would fuse them into one allocno, reproducing target's $s0 reuse directly.
- probe: Four init placements measured (inside the guard, pre-`rand()`, fade-block top, after `h = v0 + 4;`).
- result: 22 / 21 / 21 / 21, all 136 insns. The $s0 sharing in target is the allocator reusing a register that is dead after `PutDrawEnv`, not a shared C variable.
- verdict: KILLED

## [s3] Raising the loop counter allocno's n_refs to 6+ lets it keep $s0 with a pre-call initialisation, closing the 2-point delay-slot residue.
- mechanism: priority = floor_log2(n_refs)*n_refs/live_length; at the pre-call live_length of ~13, n_refs 6 gives 9230 > `h`'s 8750 while n_refs 4 gives only 6153.
- probe: The only honest ref-adding spelling is variable reuse - one function-scope `s32 i` shared between the case-1/2 loop counter and the case-20 `D_800A37C0` counter (disjoint switch arms, 7 refs). Measured with the init in the guard and pre-`rand()`.
- result: 18 and 24. The shared pseudo's live_length grows faster than the ref count helps and case 20's codegen degrades.
- verdict: KILLED

## [s3] Lowering `h`'s allocation priority by lengthening its live range at the case-20 end lets a pre-call `i = 0;` keep $s0.
- mechanism: `h` must drop below the pre-call counter's 6153; with n_refs 7 that needs live_length > 22.7 against its actual 16.
- probe: `h = D_800A37A8[a0_val];` hoisted to the first statement of the case-20 else block, alone and combined with a pre-`rand()` `i = 0;`.
- result: 14 and 19. The hoist reorders case 20's stores and costs more than the priority change buys. Joins K5, which killed the same idea from the definition end.
- verdict: KILLED

## [s3] A duplicated `i = 0;` (one before `v0 = rand();` for the delay slot, one inside the guard for the register) buys the delay slot for free.
- mechanism: A dead store to a local is a sanctioned family; if only the LAST store started the allocno's live range, the pre-call copy would fill the jal delay slot at no register cost.
- probe: Both stores present, measured with `sandbox func_800174F4 --disable all`.
- result: 14, 136 insns - identical to the single pre-call store. The dead store still starts the counter allocno's live range, so the priority loss is unchanged.
- verdict: KILLED

## [s3] Cluster (A): the switch selector can be pushed off $v1 by any statement-order or block-scope spelling.
- mechanism: global.c:find_reg hands out the lowest class register not in `hard_reg_conflicts U regs_someone_prefers`; reaching $a1(5) requires BOTH 3 and 4 to be in that union.
- probe: Instrumented cc1 with BB2_FINDREG_DEBUG=120 on the floor-8 form, reading the selector allocno's complete decision state.
- result: conflicts {2, 29}; someone_prefers, own_copy_prefs and own_full_prefs all EMPTY; it takes 3 as the lowest free class register. The same dump shows the exclusion mechanism working for pseudo 99 (skips 4 because lower-priority conflicting allocno 101 prefers 4, lands on 5), so the route exists but needs a conflicting lower-priority argument-preferring allocno - unobtainable without the selector living across `func_8005D46C`, which forces it callee-save (the measured 29/138 form).
- verdict: KILLED

## [s3] Session 2's K4 — that the counter/h $s0/$s1 flip is decided downstream of global.c (local-alloc quantities / reload) because the allocno tables of the floor-8 and floor-14 forms are byte-identical.
- mechanism: s2 compared BB2_ALLOC_DEBUG per-allocno stats (order, n_refs, live_length, priority) between the two forms and read them as identical, concluding global.c could not be the discriminator and pointing the next probe at local-alloc.c:1518/1585 and :1437/:1447/:2215.
- probe: Full cc1 `-dg` dumps of both forms compared header to header (`;; N regs to allocate` order list, every per-allocno conflict list, `;; Register dispositions`), plus fresh BB2_ALLOC_DEBUG priority tables via tmp/grind/func_800174F4/s3/alloc.sh.
- result: The headers are NOT identical: the allocno ORDER list swaps two entries — `84 120 85 73 99 98 101 74 81` (floor 8) vs `84 120 73 85 99 98 101 74 81` (floor 14) — and that swap IS the $s0/$s1 flip (85 in 16 / 73 in 17 vs 73 in 16 / 85 in 17). Every conflict list is byte-identical. The priorities differ: counter pseudo 85 has n_refs 4, live_length 9, pri 8888 in the floor-8 form and live_length 10, pri 8000 in the floor-14 form, against `h` pseudo 73 constant at n_refs 7, live_length 16, pri 8750. The lever is global.c:allocno_compare after all; s2 missed the order swap.
- verdict: KILLED

## [s3] The natural structured loop — `for (i = 0; i < h; i++)` or `i = 0; while (i < h) { ...; i++; }` — is the original source shape and reproduces target's entry guard plus bottom-tested loop.
- mechanism: GCC 2.7.2's loop rotation converts a top-tested loop into an entry guard plus a bottom-tested loop, which is exactly target's shape, and `0 < (unsigned short)h` should fold to target's single `beqz $s1`.
- probe: Both spellings measured with `sandbox func_800174F4 --disable all` on the floor-8 base.
- result: Both score 39 with 135 insns (vs 136 target). The rotation emits a two-instruction entry test (`slt` + branch) where target has a single `beqz $s1`, and the loop body loses an instruction. The goto-form loop with the explicit `if (h != 0)` guard is load-bearing, not a decompiler artifact.
- verdict: KILLED

## [s3] `env`, the drawenv-buffer address variable, doubles as the case-1/2 loop counter — target reuses $s0 for both roles, which reads like one C variable.
- mechanism: One C variable serving both roles fuses them into a single allocno and would reproduce target's $s0 reuse directly instead of relying on the allocator to reuse a dead register.
- probe: Four init placements measured: `env = 0;` inside the `if (h != 0)` guard, before `v0 = rand();`, as the first statement of the fade block, and after `h = v0 + 4;`.
- result: 22 / 21 / 21 / 21, all 136 insns — every placement is far worse than the split form's 8. The $s0 sharing in target is the allocator reusing a register that is dead after PutDrawEnv, not a shared C variable.
- verdict: KILLED

## [s3] Raising the loop counter allocno's n_refs to 6 or more lets it keep $s0 even with a pre-`rand()` initialisation, which would close the 2-point delay-slot residue.
- mechanism: priority = floor_log2(n_refs)*n_refs/live_length*10000. At the pre-call live_length of ~13, n_refs 6 gives 9230 > `h`'s 8750, while the actual n_refs 4 gives only 6153.
- probe: The only honest ref-adding spelling that costs no instructions is variable reuse: one function-scope `s32 i` shared between the case-1/2 loop counter and the case-20 `D_800A37C0` counter (disjoint switch arms, 7 refs). Measured with the init inside the guard and pre-`rand()`.
- result: 18 (init in guard) and 24 (init pre-call), both 136 insns. The shared pseudo's live_length grows faster than the ref count helps, and case 20's codegen degrades.
- verdict: KILLED

## [s3] Lowering `h`'s allocation priority by lengthening its live range at the case-20 end lets a pre-`rand()` `i = 0;` keep $s0.
- mechanism: To drop `h` below the pre-call counter's priority of 6153 with n_refs 7 requires live_length > 22.7, against its measured 16; hoisting the case-20 table load is the one placement s2's K5 (which hoisted the DEFINITION) did not cover.
- probe: `h = D_800A37A8[a0_val];` hoisted to the first statement of the case-20 else block, alone and combined with a pre-`rand()` `i = 0;`.
- result: 14 and 19. The hoist reorders case 20's stores and costs more than the priority change buys. Together with K5 this closes both ends of the `h`-live-range axis.
- verdict: KILLED

## [s3] A duplicated `i = 0;` — one before `v0 = rand();` to fill the jal delay slot, one inside the guard to keep the register — buys the delay slot for free.
- mechanism: A dead store to a local is a sanctioned family; if only the last store started the allocno's live range, the pre-call copy would be free.
- probe: Both stores present, measured with `sandbox func_800174F4 --disable all`.
- result: 14, 136 insns — identical to the single pre-call store. The dead store still starts the counter allocno's live range, so the priority loss is unchanged and the sanctioned dead-store family buys nothing here.
- verdict: KILLED

## [s3] Cluster (A), the 6-point switch selector, can be pushed off $v1 toward target's $a1 by some statement-order, block-scope or locals spelling.
- mechanism: global.c:find_reg hands out the lowest register of the class that is not in `hard_reg_conflicts` union `regs_someone_prefers`; reaching $a1(5) requires BOTH reg 3 and reg 4 to be in that union.
- probe: Instrumented cc1 with BB2_FINDREG_DEBUG=120 on the floor-8 form, reading the selector allocno's complete decision state (conflicts, someone_prefers, own_copy_prefs, own_full_prefs, pass0_used).
- result: conflicts {2, 29}; someone_prefers EMPTY; own_copy_prefs EMPTY; own_full_prefs EMPTY — it takes reg 3 as the lowest free class register, deterministically. The same dump shows the exclusion mechanism working for pseudo 99 (hard conflicts {2,3}, skips 4 because lower-priority conflicting allocno 101 prefers 4, lands on 5), so the route exists but needs a conflicting lower-priority argument-preferring allocno, which the selector cannot acquire without living across func_8005D46C and being forced callee-save (s2's measured 29/138 form).
- verdict: KILLED

## Session 4 (permuter, 2026-08-13/14)

### H-S4-1 — CONFIRMED (floor 8 -> 2)
- statement: Cluster (A), the 6-point switch-selector residue ($v1 vs target's $a1 at insns 28/30/31/33/35/41), closes if the switch-selector local is given a SECOND live range later in the function — specifically if the same C variable that holds `g_disp_enable` for the `switch` is re-defined in case 20 to hold `D_800A37A0` (replacing the separate `a1_val` local outright).
- mechanism: global.c:find_reg allocates the lowest-numbered register of the class outside `hard_reg_conflicts U regs_someone_prefers`. s3 measured the single-def selector allocno with conflicts {2,29} and all three preference sets empty, so it deterministically took $v1(3), and s3 stated the requirement for $a1(5) as "3 AND 4 must both be in that union". A second def extends the SAME pseudo's live range across the case-20 argument setup, which supplies that exclusion without pushing the selector across `func_8005D46C` (the callee-save failure mode s2 measured at 29/138).
- probe: chassis-3 permuter campaign proposed `mode = D_800A37A0; s32 a1_val = mode;` (tmp/perm_ings3/output-145-1, 74 s after seed, permuter 145 vs base 175); the form was then measured honestly with `sandbox func_800174F4 --disable all` and hand-reduced.
- result: `s32 mode` with a single def = 8 (re-confirms s2's K6). With the case-20 re-definition = **2**, 136 insns. With `a1_val` deleted and case 20 reading `mode` directly = **2**, 136 insns, one local fewer than the floor-8 form. The `(s32)` cast on the func_8005D554 result is codegen-neutral (2 either way).
- verdict: CONFIRMED

### H-S4-2 — KILLED
- statement: With cluster (A) closed, the allocation landscape has changed enough that a pre-call `i = 0;` (the placement reorg.c needs in order to fill the `rand()` jal delay slot with `move s0,zero`) might no longer cost the counter its $s0.
- mechanism: s3's closed-form inequality — priority = floor_log2(n_refs)*n_refs/live_length*10000; a pre-call init takes the counter allocno to live_length ~13 / priority 6153 against `h`'s 8750, so `h` wins $s0. The hope was that the selector's new live range perturbs the allocno order enough to change the outcome.
- probe: three `i = 0;` placements re-measured on the floor-2 base — before `v0 = rand();`, between `v0 = rand();` and `v0 &= 3;`, and after `h = v0 + 4;` (rejected/floor2-base-i0-*.c).
- result: 8, 8, 8 — all 136 insns, i.e. all three still pay exactly the 6-point callee-save cluster on top of the surviving 2-point delay-slot residue. The inequality is untouched by the cluster-(A) win.
- verdict: KILLED

### H-S4-3 — KILLED (the modality's own stopping question)
- statement: An undirected fresh-seed permuter campaign from the floor-8 base closes the register-name residual (the inherited F6).
- mechanism: decomp-permuter randomizes statement order, temporaries and types and hill-climbs on a weighted score (registers 5, reorderings 60, insert/delete 100) against a single-function target.o.
- probe: two fresh-seed campaigns on structurally different floor-8 chassis (`if (h != 0) {...}` guard, and its `if (h == 0) break;` early-exit spelling), 46045 + 41859 iterations, 21 + 20 minutes, both harvested and stopped in-session.
- result: both converged within 100 s to the SAME form — `i = 0;` hoisted to just after `v0 = rand();` — and produced nothing novel afterwards. That form is the already-rejected pre-call-init attractor and measures sandbox 8, i.e. WORSE. Root cause measured: the permuter's weighting makes trading the 2-point delay-slot reordering for the 6-point register cluster look like a 65-point gain while it is a 6-point honest loss. Undirected campaigns on this function are anti-aligned with the honest metric; only a chassis that already carries the lever (H-S4-1) produced anything useful.
- verdict: KILLED

## [s4] Cluster (A) (6 pts, insns 28/30/31/33/35/41, ours $v1 vs target $a1) closes if the switch-selector local is given a SECOND live range later in the function - specifically if the same C variable that holds g_disp_enable for the switch is re-defined in case 20 to hold D_800A37A0, replacing the separate a1_val local outright.
- mechanism: global.c:find_reg allocates the lowest-numbered class register outside hard_reg_conflicts U regs_someone_prefers. s3 measured the single-def selector allocno at conflicts {2,29} with all three preference sets empty, so it deterministically took $v1(3), and stated the $a1(5) requirement as '3 AND 4 both in that union'. A second def extends the SAME pseudo's live range across the case-20 argument setup, supplying that exclusion without pushing the selector across func_8005D46C (the callee-save failure mode s2 measured at 29/138).
- probe: chassis-3 permuter campaign proposed `mode = D_800A37A0; s32 a1_val = mode;` (tmp/perm_ings3/output-145-1, 74 s after seed, permuter 145 vs base 175); measured honestly with `sandbox func_800174F4 --disable all`, then hand-reduced by deleting the a1_val intermediate.
- result: s32 mode with a single def = 8 (re-confirms s2's K6). With the case-20 re-definition = 2, 136 insns. With a1_val deleted and case 20 reading `mode` directly = 2, 136 insns and one local FEWER than the floor-8 form. Dropping the (s32) cast on the func_8005D554 result is codegen-neutral (2 either way).
- verdict: CONFIRMED

## [s4] With cluster (A) closed the allocation landscape has changed enough that a pre-call `i = 0;` (the placement reorg.c needs to fill the rand() jal delay slot with `move s0,zero`) may no longer cost the counter its $s0.
- mechanism: s3's closed-form inequality: priority = floor_log2(n_refs)*n_refs/live_length*10000; a pre-call init takes the counter allocno to live_length ~13 / priority 6153 against `h`'s 8750, so `h` wins $s0. The hope was that the selector's new live range perturbs the allocno order.
- probe: three `i = 0;` placements re-measured on the floor-2 base - before `v0 = rand();`, between `v0 = rand();` and `v0 &= 3;`, and after `h = v0 + 4;` (memory/grind/func_800174F4/rejected/floor2-base-i0-*.c).
- result: 8, 8, 8 - all 136 insns, i.e. all three still pay exactly the 6-point callee-save cluster on top of the surviving 2-point delay-slot residue. The inequality is untouched by the cluster-(A) win.
- verdict: KILLED

## [s4] An undirected fresh-seed permuter campaign from the floor-8 base closes the register-name residual (the inherited F6).
- mechanism: decomp-permuter randomizes statement order, temporaries and types and hill-climbs on a weighted score (registers 5, reorderings 60, insert/delete 100) against a single-function target.o built from asm/funcs/func_800174F4.s at offset 0.
- probe: two fresh-seed campaigns on structurally different floor-8 chassis (`if (h != 0) {...}` guard = tmp/perm_ings, and the `if (h == 0) break;` early-exit spelling = tmp/perm_ings2), 46045 + 41859 iterations, 21 + 20 minutes, both harvested and stopped in-session.
- result: both converged within 100 s to the SAME form - `i = 0;` hoisted to just after `v0 = rand();` - and produced nothing novel afterwards. That form is the already-rejected pre-call-init attractor and measures sandbox 8, i.e. WORSE. Root cause measured: the permuter's weighting makes trading the 2-point delay-slot reordering for the 6-point register cluster look like a 65-point gain while it is a 6-point honest loss. Undirected campaigns on this function are anti-aligned with the honest metric; only a chassis that already carried the lever produced anything useful.
- verdict: KILLED
