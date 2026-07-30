# Hypothesis ledger — mk_g2l

Floor history: s1 recon 17 -> **7**.

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
