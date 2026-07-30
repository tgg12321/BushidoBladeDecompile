# Hypothesis ledger — mk_g2l

Floor history: s1 recon 17 -> **7**; s2 structural 7 -> **0 (MATCH)**.

## CONFIRMED (session 1)

**C1 — the whole gap is one register rotation seeded by the compare-value's
holder.** Holding the `0x86`-field compare value in a variable that also carries
the call's first argument (session 1 used the dead `a0` parameter) simultaneously
corrects `ptr` -> `$t0`, the field-check obj -> `$v1`, `stack_v1` -> `$v1`, and the
compare value -> `$a0`. Probe: the one-line change; measured 17 -> 7. Mechanism:
the allocation cascade documented in evidence.md; no insn count/order change.

## KILLED (session 1) — do NOT re-propose

**K1 — two-local obj split.** Giving the field-check region its own obj local
(`chk_obj`) separate from the call-path obj does NOT move the floor: byte-identical
output at floor 17, and score-identical (7) at floor 7. It only changes which
WRONG register the call-path obj gets (`$a0` split vs `$v1` un-split; target wants
`$v0`). Rejected form: `rejected/two-local-obj-split-inert.c`.

**K2 — dropping the param-alias locals.** Using `stack_a2` / `stack_v1` directly
instead of the `arg_a2` / `arg_v1` copies makes it WORSE: floor 17 -> 18. Rejected
form: `rejected/param-alias-drop-worse.c`.

## LIVE FRONTIER (ordered)

**H1 — the compare-value holder does not have to be the `a0` PARAMETER; any
single variable serving both the 0x86 compare and the call's first argument
should produce the same allocation.** Mechanism: what the cascade needs is one
pseudo carrying both roles (so it takes `$a0` via the call-arg preference and
frees `$v1`/`$t0` for the obj and `ptr`); nothing in `expand_preferences` /
`allocno_compare` cares that the pseudo originated as an incoming parameter.
Next probe: declare `s32 id;` (a named local that IS the value passed as
`func_80032854`'s first argument), use it for the `0x86` compare and for every
`*(s16 *)(obj + 4)` assignment, and pass `id`. Measure. If it also reads 7, the
candidate should switch to that spelling — it is ordinary C (one local, one
purpose per path) and sidesteps the parameter-reuse policy question entirely.
If it reads worse, the parameter identity is load-bearing and the reuse needs the
variable-reuse carve-out treatment (documented exhaustion + `/* FAKE */`
annotation + layer-2), which is a Judge/owner question, not a self-approval.

**H2 — kill the `$a0`/`$a1` hard-reg preference that the shared
`call_with_field4` load merges into the call-path obj pseudo, so it falls back
to the alloc-order first-free register `$v0` (= target).** Mechanism (exact,
verified): `global.c:797-840 expand_preferences()` IORs the preference sets of
the DEST allocno and any `REG_DEAD` allocno of the same `single_set`, but ONLY
`if (! CONFLICTP(a1,a2) && ! CONFLICTP(a2,a1))`. Our insn 153
`(set (reg 72) (sign_extend (mem (plus (reg 78) 4))))` + `REG_DEAD (reg 78)`
hands 72's {a0,a1} to 78. Two ways to break it, both pure structure:
(a) make the two pseudos CONFLICT — i.e. find a C form in which the arg/compare
value is live at the point the call-path obj is loaded (note the self-conflict
case is free: the conflict matrix lists each allocno as conflicting with itself,
so a `x = *(s16 *)(x + 4)`-shaped single_set is skipped by the merge — worth
testing whether a form exists where the obj variable is also the arg variable on
SOME paths only);
(b) remove the `REG_DEAD` note by giving the obj pointer a genuine later use.
Next probe: (a) first — enumerate forms where the call-path obj's def sits inside
the live range of the arg/compare value, and check `78 preferences:` in
`base.i.greg` after each; the dump tells you immediately whether the merge was
blocked, without needing the score to move.

**H3 — the 2-diff stack-param load order (`0x28(sp)` before `0x2C(sp)`).**
Mechanism: both loads sit in the entry block; the emitted order is the
scheduler's, and `arg_a2` (consumed at the jal, i.e. the longest path to exit)
should out-prioritise `arg_v1` (consumed at the `beqz` two insns later) — target
shows a2-first, which IS the priority-scheduling order, so our v1-first order is
the anomaly. Next probe: read `base.i.sched` for `mk_g2l` and compare the two
loads' `INSN_PRIORITY`; then try (i) keeping only the `arg_a2` alias local while
testing `stack_v1` directly at its point of use (K2 dropped BOTH aliases at once
— the single-alias variants were never measured separately), and (ii) moving the
`arg_a2` initialiser above/below the early-out to shift its LUID.

## [s1] The entire 17-diff gap is register allocation, not control flow or scheduling: build and target both emit 51 instructions and every diff is a register name.
- mechanism: sandbox --disable all reports target_insns 51 / build_insns 51 at every floor; all 10 regfix rules for mk_g2l (regfix.txt:242-257) are register renames / operand-order paperwork ($3<->$8, $3<->$4 @13-29, four subst "$4,"->"$2,", subst 4($4)->4($2), reorder 4,3).
- probe: engine sandbox + tmp/grind/mk_g2l/s1/dis.py index-aligned objdump vs asm/funcs/mk_g2l.s
- result: Confirmed. Register map: ptr v1(ours)/t0(target), stack_v1 t0/v1, obj a0/v1+v0, f86 t0/a0; cmd, arg_a2, a3, f8E, f88 already match.
- verdict: CONFIRMED

## [s1] Target holds the 0x86-field compare value in $a0 — the same register the call's first argument occupies — so one source variable carried both roles; making the compare value share the (dead-after-early-out) a0 parameter should unwind the whole rotation.
- mechanism: One pseudo serving both the compare and the call's first arg takes $a0 through the call-arg copy preference, which frees $v1 for the field-check obj and pushes ptr out to $t0; stack_v1 then shares $v1 with the obj (disjoint live ranges) exactly as target does.
- probe: s16 f86 = *(s16*)(obj+0x86) -> a0 = *(s16*)(obj+0x86); sandbox --disable all
- result: Floor 17 -> 7 in one edit. Four register families corrected simultaneously: ptr->t0, field-check obj->v1, stack_v1->v1, compare value->a0, all matching target.
- verdict: CONFIRMED

## [s1] Target's use of $v1 for the field-check obj and $v0 for the four call-path objs means the original source used two distinct obj variables, so splitting our single v1_obj local should reproduce it.
- mechanism: Two C variables -> two allocnos -> independent register choices.
- probe: Added a separate chk_obj local for the field-check region; sandbox at floor 17 and again at floor 7, plus index-aligned objdump.
- result: KILLED. At floor 17 the split produced byte-identical output (fully inert). At floor 7 it is score-inert (still 7) and only changes which WRONG register the call-path obj takes: split -> $a0, single-variable -> $v1 (target wants $v0). Banked as rejected/two-local-obj-split-inert.c; the simpler single-variable form is the candidate.
- verdict: KILLED

## [s1] The arg_a2 / arg_v1 param-alias locals hold registers hostage; dropping them (per drop-param-alias-local) should free a register and lower the floor.
- mechanism: Removing an alias copy removes a pseudo and shortens a live range.
- probe: Deleted both alias locals, tested stack_v1 directly and passed stack_a2 directly; sandbox --disable all.
- result: KILLED — floor 17 -> 18, strictly worse. The alias locals are load-bearing. Banked as rejected/param-alias-drop-worse.c. NB both were dropped together; the single-alias variants remain unmeasured.
- verdict: KILLED

## Session 2 (structural) - CLOSED

**H1 - CONFIRMED.** The compare-value holder does not have to be the `a0`
PARAMETER. An ordinary named `s32 id;` local serving both the `0x86` compare
and the call's first argument measures the same floor 7. Parameter identity is
not load-bearing; session 1's variable-reuse policy question is retired.
(Superseded anyway - the final form has no arg variable at all.)

**H2 - CONFIRMED, and it is what closed the function.** The call-path obj
pseudo takes `$a0` only because `expand_preferences()` merges the arg pseudo's
{a0,a1} preference into it. Neither of session 1's two proposed break-methods
(force a CONFLICT, or remove the REG_DEAD note) was needed: the merge has a
third precondition, `reg_allocno[REGNO (SET_DEST (set))] >= 0`, i.e. the
single_set must write a PSEUDO. Deleting the shared argument variable - write
the call at each site with the field-4 read inline, no `id`, no shared
`do_call:` label - makes the load write the argument HARD reg instead, the
merge is skipped, and the obj falls to the alloc-order first-free register
`$v0`. Floor 7 -> 2, instruction count unchanged (jump2 cross-jumps the
duplicated tails back together after allocation).

**H3 - CONFIRMED, with session 1's K2 overturned for the new structure.** The
2-diff stack-param load order is caused by the `arg_a2` param-alias local.
Sweep on the duplicated-call form: keep both = 2, swap declaration order = 2,
drop `arg_v1` only = 2, drop `arg_a2` only = **0**, drop both = **0**. K2's
"the alias locals are load-bearing" was true only of the shared-arg structure.
Committed form drops both.

## KILLED (session 2) - do NOT re-propose

**K3 - a shared C variable for the call's first argument.** Any form that
funnels all five paths through one arg local (whether that local is the dead
`a0` parameter, a fresh `id`, or a split `chk_obj`/`v0_obj` pair alongside it)
is capped at floor 7: the arg pseudo's {a0,a1} hard-reg preference is merged
into the call-path obj by `expand_preferences`, and the obj can then never
reach `$v0`. Rejected form: `rejected/shared-arg-pseudo-inherits-a0-pref.c`
(the best shared-arg spelling, floor 7, every allocno but one matching target).

**K4 - alias-local declaration order as a scheduling lever.** Swapping
`u8 *arg_a2` / `s32 arg_v1` declaration order is score-inert (2 either way);
only removing the `arg_a2` copy moves the entry-block load order.

## [s2] The compare-value holder need not be the a0 parameter - any single variable serving both the 0x86 compare and the call's first argument gives the same allocation.
- mechanism: The cascade needs one pseudo carrying both roles so it takes $a0 via the call-arg copy preference; nothing in expand_preferences/allocno_compare cares that the pseudo began as an incoming parameter.
- probe: Declared `s32 id;`, used it for the 0x86 compare and every `*(s16 *)(obj + 4)` assignment, passed it as arg 1; sandbox --disable all.
- result: Floor 7 - identical to session 1's dead-a0-parameter spelling. Parameter identity is not load-bearing; the SOTN variable-reuse / FAKE-annotation policy question is retired without a carve-out.
- verdict: CONFIRMED

## [s2] Blocking the expand_preferences merge that hands $a0/$a1 to the call-path obj pseudo makes it fall back to the alloc-order first-free register $v0 (target), closing 5 of the 7 residual diffs.
- mechanism: global.c:797-840 merges the hard-reg preference sets of a single_set's DEST allocno and any REG_DEAD allocno of the same insn. Session 1 identified two break conditions (CONFLICTP, or no REG_DEAD note); the operative third one is the guard `reg_allocno[REGNO (SET_DEST (set))] >= 0` - the merge only fires when the destination is a PSEUDO. A shared arg variable across a shared call label forces that pseudo to exist; duplicating the call into each arm with the field-4 read written inline lets the load combine straight into the argument HARD register, so the merge is skipped entirely.
- probe: First measured the split form's greg dump to prove only allocno 78 was wrong (78 preferences: 4 5; disposition 78 in 4; no conflict with hard reg 2). Then removed the `id` variable and the shared `do_call:` label, writing `func_80032854(*(s16 *)(*ptr + 0x4), K, ...); return;` in each of the five arms; sandbox --disable all.
- result: Floor 7 -> 2, build_insns unchanged at 51 (jump2 cross-jumps the duplicated `lh $a0,0x4($vN)` / `jal` suffixes back together after allocation, reproducing target's shared .L80027970 / .L80027974 tails). Only the stack-param load-order pair survived.
- verdict: CONFIRMED

## [s2] The 2-diff stack-param load order is caused by the arg_a2 param-alias local, not by scheduling priority that C cannot reach.
- mechanism: Both loads sit in the entry block; the `u8 *arg_a2 = stack_a2;` copy inserts an extra pseudo whose LUID ordering makes the scheduler emit `lw 0x2C($sp)` (stack_v1) before `lw 0x28($sp)` (stack_a2). Target is a2-first.
- probe: Five-variant sweep on the duplicated-call form, each sandboxed: both aliases kept, declaration order swapped, drop arg_v1 only, drop arg_a2 only, drop both.
- result: 2 / 2 / 2 / 0 / 0. The arg_a2 copy alone is the cause; declaration order is inert. Session 1's K2 ("the alias locals are load-bearing, dropping both costs a point") was true only of the shared-arg structure and does not survive the restructure. Committed form drops both aliases. FUNCTION MATCHES: sandbox score 0.
- verdict: CONFIRMED
