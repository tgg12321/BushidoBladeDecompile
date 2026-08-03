# Hypothesis ledger — func_80060544

## Session s1 (2026-08-03, modality: recon) — floor 18 -> 4

### CONFIRMED

**H1 — one-expression address arithmetic splits the address pseudo from its consumer.**
Mechanism: `s.p_geom = (s32 *)((s32)&D_8009B770 + idx);` gives cc1 two pseudos (the
`la` result and the sum); local_alloc puts the `la` in $v1 and the sum in $v0, while
target has both in $v0. Probe: rewrite as split-init accumulation
(`geom = (s32)&D_8009B770; geom += idx; s.p_geom = (s32 *)geom;`), which is one
self-updating pseudo. Result: floor 18 -> 15. CONFIRMED.

**H2 — the same split affects a multi-arm pointer select.** Mechanism: three per-arm
`s.p_static = &D_800...;` stores are three independent pseudos, all landing in $v1;
target reuses $v0 in every arm, which is what a single pseudo written in three arms
produces. Probe: introduce ONE shared `s32 stat;` and write
`stat = (s32)&D_800...; s.p_static = (s32 *)stat;` in all three arms.
Result: floor 15 -> 8, all three `la`/`sw` clusters flip to $v0. CONFIRMED.

**H3 — target's `3` is a C-level value set before the loop, not a loop.c-hoisted CSE
constant.** Mechanism: target's prologue orders the inits `i=0`, `$s5=3`, `idx=0`.
loop.c emits preheader movables immediately before the loop start — i.e. AFTER both
inits — so a hoisted constant provably cannot land between them (and the literal-3 C
did in fact emit it last, which is the `reorder 17,18,15,16 @ 15-18` regfix rule).
Probe: declare `s32 last;`, set `last = 3;` between `i = 0;` and `idx = 0;`, and use it
in the two equality tests only. Result: floor 8 -> 4, prologue now identical. CONFIRMED.
Caveat for the Judge: `last` is a live, twice-read constant-holder local. It is not a
dead store, and reads naturally as "index of the last/special-cased element", but it
does sit in the [[named-local-fake-exception]] family. If the Judge rules it needs the
`/* FAKE */` annotation + documented lever exhaustion, that is a one-comment fix, not a
dead axis.

### KILLED

**H4a — the extra `move s5,s7` is a droppable param alias ([[drop-param-alias-local]]).**
KILLED. Removing the `new_var = arg1;` carrier and using `arg1` directly in the
`ot_Link` index regressed 18 -> 48 (from the session-start base) and 4 -> 10 (from the
improved base), and did not even reduce the instruction count. See
`rejected/drop-param-alias-new_var.c`.

**H4b — the carrier fails to coalesce with the arg1 param because its live range starts
mid-function; hoisting its init to the prologue will coalesce them.** KILLED. Hoisting
`new_var = arg1;` to just after `prev = arg0;` regressed 8 -> 14, and additionally
routing `s.arg2_field` through the carrier (so a single C name is the only reader of
`arg1`) also gave 14. The carrier's LATE source position is load-bearing. See
`rejected/hoist-new_var-init.c`.

## Session s2 (2026-08-03, modality: structural) — floor 4 -> 2

### CONFIRMED

**H4' (supersedes H4) — the extra `move $s5,$s7` is a droppable param alias AFTER
ALL, but the deletion must be PAIRED with shortening the neighbouring end_off
carrier chain.** Mechanism: the `arg1` pseudo and the `end_off` pseudo are in a
callee-save priority race. Target assigns arg1 -> `$s7` and end_off -> `$s8`.
Deleting the `new_var` alias alone flips that assignment (arg1 -> `$s8`,
end_off -> `$s7`) and grows a *different* spurious `move $s5,$s7` — the exact
measurement s1 recorded as H4a KILLED. The end_off value travelled through FOUR
pseudos (`end_off -> new_var6 -> new_var4 -> reassignment of new_var3 -> return`);
removing the `new_var4` hop and the `new_var3` reassignment shortens it to two,
which drops that allocno's ref count enough to lose the race, so arg1 keeps `$s7`
and the final `sll $a0,$s7,0x2` matches with no copy at all.
Probe: a 7-cell lattice over {carrier kept, carrier dropped} x {chain length 4, 3,
2, 1}, each measured with `sandbox --disable all` (full table in evidence.md §6).
Result: floor 4 -> 2, and build_insns 134 -> 133 == target_insns. CONFIRMED.
The chain has an OPTIMUM LENGTH of exactly two pseudos — collapsing it to ONE and
dropping the carrier gives 6; going straight to `new_var3` gives 12.

### KILLED

**H5a — the `move $a1,$zero` placement in the `Case3` arm is reachable by
reordering the arm's statements.** KILLED. Every permutation is strictly worse
than the current order (`stat` / `p_static` store / `pad0C` store / call, = 2):
pad0C-first 4, p_static-last 4, reversed equality test (`last == i`) 3,
materialising the `D_8009B7D0` address in the PREDECESSOR block (so the arm holds
no la at all) 4, and Case3 storing the table address directly instead of through
the shared `stat` carrier 39 (insns 136 — the shared carrier must cover all three
arms or none). See `rejected/case3-arm-statement-reorderings.c`.

**H5b — the placement is reachable by hoisting the call's arguments into named
locals ([[hoist-call-arg-local-flips-jal-delay]]).** KILLED, and the *shape* of
the kill is the informative part: every respelling measures EXACTLY 2/133,
unchanged — GCC folds them all back to the same RTL. Probed: both args hoisted
into arm-top locals; only the struct-pointer hoisted; the constant carried in a
named local (a constant-holder, measured as a diagnostic only, never proposed);
the call result via an extra pseudo; the stored `mid_off` value via an extra
pseudo. A function-wide `S544 *sarg = &s;` is separately catastrophic (44 / 38,
insns 137 — it forces a callee-save home for `&s` instead of the per-call
`addiu $a0,$sp,0x18`). See `rejected/case3-call-arg-respellings.c`.

## Live frontier (for the next session)

**[s2: H4 and H6 below are CLOSED / OBSOLETE — see the s2 section above. H4 was
closed by the paired m2c-carrier deletion; the live frontier is H5 only, restated
at the end of this file. The H4 text is kept for provenance.]**

**H4 — the extra `move $s5,$s7` + the consequent `sll $a0,$s5,0x2` (target:
`sll $a0,$s7,0x2`). 2 of the remaining 4.** — CLOSED in s2.
What is known: the copy is emitted after loop 1; `new_var` (the `arg1` carrier read by
the `ot_Link` index) is load-bearing in its current LATE position (H4a/H4b killed both
the removal and the hoist). So the copy is not a naive redundant alias — target keeps
`arg1` in $s7 from the prologue to the final `sll` with no second carrier, while our
build allocates a second callee-save and copies into it late.
Next probes, in order: (a) `-da` greg dump of the function and read
`;; Register dispositions:` for the carrier pseudo — find what it conflicts with that
forces a distinct hard reg from the arg1 pseudo ([[register-alloc-pure-c]] step 0);
(b) vary the carrier's *type* / the expression it feeds (`D_800A374C + new_var * 4`
vs a pre-scaled `new_var = arg1 * 4;` vs indexing) — a different consumer shape changes
the pseudo's conflict set; (c) move the carrier assignment to the other side of the
`s.byte29/2A/2B/28` stores, which is the only other statement group in that region.

**H5 — the `move $a1,$zero` scheduled LATE in the `Case3` arm. 2 of the remaining 4.**
Target's `Case3` block orders: `addu $a1,$zero,$zero` FIRST, then the `D_8009B7D0`
`la`/`sw` pair, then `jal func_80073728` with `sw $s2,0x24($sp)` in the delay slot. Our
build emits the `la`/`sw` pair first and the `a1` zeroing after. Mechanism: this is a
sched1 placement of a call-argument constant. Next probes: (a) reorder the arm's C so
the `s.pad0C = mid_off;` store sits between the `stat` store and the call (currently it
is immediately before the call), which changes what sched1 has available to fill;
(b) hoist the constant argument into a named local declared/assigned earlier in the arm
([[hoist-call-arg-local-flips-jal-delay]] is the sibling recipe for the delay-slot case);
(c) check whether `func_80073728`'s second parameter is genuinely a constant 0 in every
caller or whether it carries a value in the original (a named local would explain the
early materialisation naturally).

**H6 — apply H1/H2's lever family to the OTHER functions in the queue.** Not about this
function, but the finding generalises: any regfix cluster that is purely
`subst "la $3,SYM" "la $2,SYM"` (address pseudo renamed $v1 -> $v0) is a candidate for
split-init accumulation / a shared named carrier, and both are sanctioned pure-C forms.
Worth a `scan-redundant`-style sweep for that rule shape.

## THE FRONTIER AFTER s2 — ONE hypothesis, 2 points, everything else matches

**H5' — target's `Case3` block schedules the `addu $a1,$zero,$zero` call-argument
set-up at the FRONT of the block (adjacent to the `a0` set-up that reorg then
steals into the predecessor branch's delay slot), while our build leaves it glued
to the `jal`. This is decided inside cc1, in a basic block whose instruction SET
is now identical to target's; no C-level respelling of that block reaches it.**

State: `sandbox --disable all` == 2, `build_insns` == `target_insns` == 133. Every
register, every store, every delay slot matches. The single non-branch diff hunk
is a delete/insert pair for `move a1,zero`
(`tmp/grind/func_80060544/s2/pairs_floor2.txt`).

Pre-reorg block orders (derived in evidence.md §7):
  target: `[a0=&s, a1=0, la, sw p_static, sw pad0C, jal]`
  ours:   `[a0=&s, la, sw p_static, a1=0, sw pad0C, jal]`

What s2 eliminated (do NOT re-run): all six statement orderings of the arm, all
five call-argument/result respellings, the shared-`stat`-carrier removal for this
arm, and a function-wide named `&s` pointer. See the two `rejected/` files.

Next probes, in order — this is now a FORENSICS question, not a structural one:
(a) `cc1 <build-flags> -da` on the sandbox's preprocessed file and read the
    `.sched` / `.sched2` dumps for that basic block: does sched1 even move the
    insn, or is the difference already present in the `.jump`/`.combine` RTL (i.e.
    an EMISSION-order difference from `expand_call`, not a scheduling one)? That
    single fact decides which pass is the target and is the highest-value probe
    available. `tmp/grind/func_80060544/s2/sweep.py --dump <variant>` gives the
    instruction-level diff for any candidate cheaply.
(b) If it IS sched1: the tie is between `sw p_static` (depth 1 from the block
    start) and `a1=0` (depth 0). Target picks the `sw` earlier in the backward
    schedule, ours picks `a1`. Find the C shape that changes the dependence
    height of the p_static store — e.g. making the stored value depend on one
    more computed pseudo INSIDE the block (note: routing it through a plain copy
    is already measured inert, so it must be a real computation).
(c) If it is EMISSION order: look for another BB2 function that calls a
    two-argument routine with a constant second argument immediately after a
    struct-field store, and diff its block order against target — a matched
    sibling settles what the original C looked like.
(d) A permuter campaign on this function is now cheap and well-seeded: the base is
    2 points with a matching instruction count, so the search space is tiny.
    Follow [[permuter-fresh-seed-discipline]].

## [s1] The $v1-instead-of-$v0 address cluster exists because writing the whole address expression as ONE expression (`s.p_geom = (s32 *)((s32)&D_8009B770 + idx);`) gives cc1 two pseudos - the `la` result and the sum - where target has one.
- mechanism: local_alloc assigns the short-lived `la` pseudo $v1 and the sum $v0; target's `lui $v0 / addiu $v0 / addu $v0,$s1,$v0` is a single self-updating pseudo. Split-init accumulation (var = a; var += b) collapses them into one pseudo, which then gets $v0.
- probe: Rewrote as `geom = (s32)(&D_8009B770); geom += idx; s.p_geom = (s32 *)geom;` and ran `sandbox func_80060544 --disable all`.
- result: Floor 18 -> 15; the D_8009B770 la/addu cluster now matches target exactly ($v0).
- verdict: CONFIRMED

## [s1] The same pseudo-splitting governs the three per-arm static-table stores (D_8009B7D8 / D_8009B800 / D_8009B7D0), which our build allocates to $v1 in all three arms.
- mechanism: Three direct `s.p_static = &D_800...;` stores are three independent pseudos; target reuses $v0 in every arm, which is what ONE pseudo written in three branch arms produces. Routing all three arms through a single shared named local collapses them.
- probe: Declared one `s32 stat;` and wrote `stat = (s32)(&D_800...); s.p_static = (s32 *)stat;` in each of the three arms; re-ran the sandbox.
- result: Floor 15 -> 8; all three la/sw clusters flipped $v1 -> $v0, matching target.
- verdict: CONFIRMED

## [s1] Target's `3` in $s5 was set by the C source before the loop, NOT hoisted out of the loop by loop.c as a CSE'd constant.
- mechanism: Target's prologue orders the inits `i=0` / `$s5=3` / `idx=0`. loop.c emits preheader movables immediately before the loop start, i.e. AFTER both zero-inits - which is exactly what the literal-3 C emitted (and is what the `reorder 17,18,15,16 @ 15-18` regfix rule was papering over). A value landing BETWEEN the two inits can only come from source order.
- probe: Declared `s32 last;`, set `last = 3;` between `i = 0;` and `idx = 0;`, used it for the two equality tests only (`i == last`, `i != last`); the `i < 3` / `i < 4` tests stay slti immediates. Re-ran the sandbox.
- result: Floor 8 -> 4; the prologue now matches target instruction-for-instruction.
- verdict: CONFIRMED

## [s1] The extra `move $s5,$s7` (and the consequent `sll $a0,$s5,0x2` where target has `sll $a0,$s7,0x2`) is a droppable param alias per the documented [[drop-param-alias-local]] recipe.
- mechanism: The recipe says a param->local alias occupies a second callee-save and that dropping the alias frees the param register for direct reuse at the later use site.
- probe: Removed the `new_var = arg1;` carrier and its declaration and used `arg1` directly in `ot_Link(D_800A374C + (arg1 * 4), ...)`. Measured from BOTH bases.
- result: Regressed 18 -> 48 from the session-start base and 4 -> 10 from the improved base, with build_insns unchanged at 134 in the second case (so it does not even remove the `move`). The carrier is load-bearing. Banked in rejected/drop-param-alias-new_var.c.
- verdict: KILLED

## [s1] The carrier fails to coalesce with the arg1 param because its live range starts mid-function; starting it in the prologue will let the two coalesce and let the final `sll` read $s7 directly.
- mechanism: Coalescing of a copy requires overlapping/adjacent live ranges with no conflict; a carrier initialised in the prologue alongside `prev = arg0;` should merge with the incoming-arg pseudo.
- probe: Variant A: moved `new_var = arg1;` to just after `prev = arg0;`. Variant B: same, plus routed the struct field through it (`s.arg2_field = new_var;`) so a single C name is the only reader of arg1. Both measured from the floor-8 base.
- result: Both regressed 8 -> 14. The carrier's LATE source position (between the `p1` and `p0` initialisations) is load-bearing in the opposite direction. Banked in rejected/hoist-new_var-init.c.
- verdict: KILLED

## [s2] H4' (supersedes s1's H4/H4a) — the extra `move $s5,$s7` and the consequent `sll $a0,$s5,0x2` ARE removable by dropping the `new_var` arg1 alias, but only when PAIRED with shortening the neighbouring end_off carrier chain from four pseudos to two.
- mechanism: The arg1 pseudo and the end_off pseudo are in a callee-save priority race. Target assigns arg1 -> $s7 and end_off -> $s8. Dropping the alias alone flips that assignment (arg1 -> $s8, end_off -> $s7) and grows a DIFFERENT spurious `move $s5,$s7` — which is exactly the regression s1 measured and recorded as KILLED. end_off travelled through four pseudos (end_off -> new_var6 -> new_var4 -> a reassignment of new_var3 -> return); removing the new_var4 hop and the new_var3 reassignment shortens the chain to two, dropping that allocno's ref count enough to lose the race, so arg1 keeps $s7 and the final shift reads it directly with no copy.
- probe: A 7-cell lattice over {alias kept, alias dropped} x {end_off chain length 4, 3, 2, 1}, each measured with `sandbox func_80060544 --disable all` via tmp/grind/func_80060544/s2/sweep.py. Results from the floor-4 base: baseline 4/134; drop-alias-only 10/134; collapse-chain-only 13/134; one-hop-only 11/134; end_off-straight-to-new_var3-only 14/134; collapse+drop 6/133; straight+drop 12/133; ONE-HOP+drop 2/133.
- result: Floor 4 -> 2 with build_insns 134 -> 133, which EQUALS target_insns for the first time in this function's history. The `move $s5,$s7` and the `sll $a0,$s5,0x2` both disappear. The end_off chain has an optimum length of exactly two pseudos — both shorter (6) and longer (12) forms lose. Neither deletion is a coercion: both remove dead m2c plumbing and the resulting C is strictly simpler than the form it replaces.
- verdict: CONFIRMED

## [s2] H5a — the `addu $a1,$zero,$zero` that target schedules FIRST in the Case3 arm (and our build emits last) can be moved by reordering that arm's statements.
- mechanism: sched1/emission placement of a call-argument constant inside one basic block; what the pass has available to place depends on the arm's statement order.
- probe: Six orderings measured with `sandbox --disable all`: baseline (stat / p_static store / pad0C store / call) = 2; pad0C store first = 4; p_static store last = 4; reversed equality test `last == i` = 3; the D_8009B7D0 address materialised in the PREDECESSOR block so the arm holds no la at all = 4; Case3 storing the table address directly instead of through the shared `stat` carrier = 39 (insns 136).
- result: Every permutation is strictly WORSE than the baseline order; none moves the a1 set-up to the front of the block. The 39/136 result additionally establishes that the s1 shared-`stat` carrier must cover all three arms or none. Banked in memory/grind/func_80060544/rejected/case3-arm-statement-reorderings.c.
- verdict: KILLED

## [s2] H5b — the placement is reachable by hoisting the call's arguments into named locals, per the [[hoist-call-arg-local-flips-jal-delay]] sibling recipe.
- mechanism: A named local for a late-materialised call argument changes where the argument set-up is emitted relative to the surrounding stores.
- probe: Five respellings measured: both arguments hoisted into locals at the top of the arm; only the struct-pointer argument hoisted; the constant argument carried in a named local (a constant-holder, run as a DIAGNOSTIC only and never proposed as a closing form); the call result routed through an extra pseudo; the stored `mid_off` value routed through an extra pseudo. Separately, a function-wide `S544 *sarg = &s;` assigned in the prologue, used for the func_80073728 call alone and for every call.
- result: All five respellings measure EXACTLY 2 with insns 133 — completely inert, GCC folds every one back to the same RTL. That no-change result is the informative one: the a1 set-up's position is fixed by the call expansion, not by how the argument is spelled. The function-wide named pointer is separately catastrophic (44 and 38, insns 137) because a long-lived pointer pseudo forces a callee-save home for `&s` instead of the per-call `addiu $a0,$sp,0x18` that both target and our baseline emit. Banked in memory/grind/func_80060544/rejected/case3-call-arg-respellings.c.
- verdict: KILLED

## Session s3 (2026-08-03, modality: structural) — floor 2 -> 2 (no change; two axes killed, one counter-example found)

### KILLED

**H5c — the `move $a1,$zero` placement is reachable by reshaping the CONTROL
FLOW that creates the Case3 basic block (the dispatch), rather than the arm's
contents.** KILLED, and informative: every reshape moves the instruction COUNT
off target's 133. Measured from the floor-2 base: natural if/else-if chain 18
(insns 126); if/else with the `i == last` arm tested first 22 (122); real
`switch (i)` with cases 1/2 shared 36 (137); the goto ladder with the
`i == last` test hoisted to the front 14 (126). The m2c four-way ladder
(`i<3 / i>0 / i==0 / i==last`, three labelled arms, shared `Skip:` join) is
therefore not an artifact to be cleaned up — it is the original's block
structure. Banked in `rejected/case3-dispatch-and-loop-reshapes.c`.

**H5d — the placement is reachable by reshaping the enclosing LOOP, or by
declaration order / carrier type / the call's cast.** KILLED. `while (i < 4)`
and swapping the two increments are exactly INERT (2 / 133); `idx` incremented
at the top of `Skip` gives 4 (132); a `for (i = 0; i < 4; i++)` header gives 6
(and destroys the s1 `last = 3` prologue lever). `s32 *stat` instead of
`s32 stat` + casts, `stat` declared first, `stat` declared last, and
`func_80073728((GameObj *)(&s), 0)` (the spelling every other call site of this
callee in text1b.c uses) are all exactly INERT at 2 / 133. Same file.

**H5' probe (b) — "if sched1 owns it, raise the dependence height of the
`sw p_static` inside the block."** KILLED with measurements. `stat += idx;`
(one extra real insn, 3 / 134) and `stat += idx; stat += prev;` (two, 4 / 135)
both insert the new chain insns BEFORE the a1 set-up and leave it exactly where
it was — immediately preceding the `sw` that precedes the `jal`. At chain
heights 1, 2 and 3 the a1 set-up never migrates to the front of the block; a
pure INSN_PRIORITY tie would have flipped by height 2. (`stat += i - last;`
folds away entirely — `i == last` is provable in that arm — so those two probes
are not evidence.) Diagnostics only, never candidate forms; banked in
`rejected/sched-height-probes-diagnostic.c`.

### CONFIRMED (new fact, not a closing lever)

**H7 — our own toolchain DOES emit both argument set-ups at the FRONT of a
`func_80073728` call block; the Case3 block is the exception, not the rule.**
`func_8005D46C` (src/text1b.c:12718) calls the same callee twice in the same
shape (a run of struct-field stores, then `ret = func_80073728((GameObj *)(&s),
0);`). Its cheat-free sandbox object emits `addiu a0,sp,16 / move a1,zero` as
the first two instructions of the call's basic block, ahead of the entire store
cluster — which is exactly the order func_80060544's TARGET has and our build
of func_80060544 does not. Probe: `sandbox func_8005D46C --disable all`, then
objdump the sandbox object (excerpt in `tmp/grind/func_80060544/s3/results.txt`).
So the placement is neither a toolchain limitation nor a property of the callee;
it is a property of this particular basic block. The visible difference is size
and chain depth — the sibling's block is ~20 insns with several independent
chains of height >= 4 and nine field stores, the Case3 block is five insns with
one chain of height 2 — but note that s3 separately measured that *adding*
chain height to the Case3 block does not reproduce the sibling's behaviour, so
"make the block bigger" is not by itself the lever.

## THE FRONTIER AFTER s3 — still ONE hypothesis, 2 points, and it is now a FORENSICS question

State unchanged: `sandbox --disable all` == 2, `build_insns` == `target_insns`
== 133, single non-branch diff hunk = delete/insert of `move a1,zero` in the
Case3 arm. What s3 adds is that the C-level structural surface is now measured
flat in EVERY direction anyone has named: intra-arm statement order (s2), call
argument/result respelling (s2), dispatch shape (s3), loop shape (s3),
declaration order (s3), carrier type (s3), call-site cast (s3), and
dependence height (s3). Nothing at the C level moves the insn.

The precise open question, in scheduler terms: GCC 2.7.2's sched.c schedules a
block BACKWARD from its end. Both builds place `sw p_static` first (it ends up
immediately before the `jal`). At the NEXT step our build picks the `a1`
set-up and target's build picks the address chain. `rank_for_schedule`
(tools/gcc-2.7.2/sched.c:2399) compares (1) INSN_PRIORITY, (2) a three-way
class relative to `last_scheduled_insn` — data-dependent = 1 (worst),
anti/output = 2, independent-or-latency-1 = 3 (best) — and (3) INSN_LUID, with
the HIGHER LUID preferred. The chain insns are data-dependent on the just-placed
`sw` (class 1) and the a1 set-up is independent (class 3); the a1 set-up is
emitted by expand_call after the stores, so it also holds the higher LUID. Both
tiebreaks favour a1. Therefore target's build must have beaten a1 on
INSN_PRIORITY outright — and s3 measured that adding chain height does NOT
achieve that, which means the difference is in a1's OWN priority (or in whether
the a1 set-up is even in the ready list at that step), not in the chain's.

Next probes, in order:
(a) `cc1 <build-flags> -da` on the sandbox's preprocessed file; read the
    `.sched` dump for the Case3 block and record the actual INSN_PRIORITY of
    the a1 set-up and of the address chain. NOTE: `tools/gcc-2.7.2/sched.c`
    carries env-gated instrumentation — `BB2_RANK_DEBUG=1` makes
    `rank_for_schedule` print `RANKDBG last=<uid> y=<uid> cls=<n> x=<uid>
    cls2=<n> val=<n>` for every comparison (sched.c:2436-2446) — so the
    comparison that decides this is directly observable IF the built binary is
    the instrumented one (check `strings` for RANKDBG; per
    [[instrumented-cc1-location]] the instrumented binary is
    `tools/gcc-2.7.2/cc1`, NOT `tools/gcc-2.7.2/build/cc1`).
(b) Do the same for the SIBLING `func_8005D46C`, where our build gets the
    order RIGHT, and diff the two priority/class tables. Two blocks, same
    callee, same compiler, opposite outcomes — that diff is the answer.
(c) Only if (a)+(b) show the a1 set-up is emitted late by expand_call rather
    than chosen late by the scheduler: the question becomes what C statement
    shape makes expand_call emit the second argument earlier, and the sibling
    is again the reference.
(d) A permuter campaign remains cheap and extremely well-seeded (base 2 with a
    matching instruction count). Follow [[permuter-fresh-seed-discipline]].

## [s3] H5c - the `move $a1,$zero` placement in the Case3 arm is reachable by reshaping the CONTROL FLOW that creates the block (the i-dispatch), rather than the arm's contents.
- mechanism: The Case3 basic block is created by the m2c goto ladder (i<3 / i>0 / i==0 / i==last with three labelled arms and a shared Skip: join). A different dispatch spelling changes block boundaries, predecessor structure and LUID order, which is what feeds both the scheduler and reorg's delay-slot steal.
- probe: Four reshapes measured with `sandbox func_80060544 --disable all` from the floor-2 base via tmp/grind/func_80060544/s3/sweep3.py: natural if/else-if chain; if/else with the (i == last) arm tested first; a real `switch (i)` with cases 1/2 shared; the goto ladder with the (i == last) test hoisted to the front.
- result: 18 (insns 126), 22 (122), 36 (137), 14 (126) respectively - every reshape regresses AND moves the instruction count off target's 133. The goto ladder is the original's block structure, not an m2c artifact to be cleaned up.
- verdict: KILLED

## [s3] H5d - the placement is reachable by reshaping the enclosing loop, or by declaration order / carrier type / the call-site cast.
- mechanism: Loop form changes where the increments and the induction-variable inits sit relative to the prologue; declaration order and type change pseudo creation order and LUIDs; the (GameObj *) cast is the spelling every other func_80073728 call site in text1b.c uses.
- probe: Eight variants measured from the floor-2 base: `while (i < 4)`; increments swapped; `idx` incremented at the top of Skip; `for (i = 0; i < 4; i++)`; `s32 *stat` instead of `s32 stat` plus casts; `stat` declared first; `stat` declared last; `func_80073728((GameObj *)(&s), 0)`.
- result: while-form 2/133, increments swapped 2/133, stat-as-pointer 2/133, stat declared first 2/133, stat declared last 2/133, GameObj cast 2/133 - all EXACTLY inert; idx-incremented-early 4 (insns 132); for-header 6 (and it destroys the s1 `last = 3` prologue lever, which needs `i = 0;` and `idx = 0;` as separate statements with `last = 3;` between them).
- verdict: KILLED

## [s3] H5' probe (b) from the s2 frontier - if sched1 owns the placement, raising the dependence height of the `sw p_static` INSIDE the Case3 block will push the a1 set-up to the front (target's order).
- mechanism: GCC 2.7.2 sched.c schedules a block backward from its end; rank_for_schedule compares INSN_PRIORITY (longest path to block end) first. If the a1 set-up (priority 1) is beating the address chain in a near-tie, lengthening the chain should flip the comparison.
- probe: Chain height raised with loop-carried values GCC cannot fold: `stat += idx;` (one extra insn) and `stat += idx; stat += prev;` (two). Instruction-level diff read with sweep3.py --dump. Control probes `stat += i - last;` and `stat += (i - last) * 2;` measured 2/133 because i == last is provable in that arm and GCC deletes them - they are not evidence.
- result: 3 (insns 134) and 4 (insns 135). In BOTH the added chain insns are inserted BEFORE the a1 set-up and the a1 set-up does not move: at chain heights 1, 2 and 3 it stays pinned to the slot immediately preceding the `sw` that precedes the `jal`. A priority tie would have flipped by height 2. Diagnostics only, never candidate forms.
- verdict: KILLED

## [s3] H7 - our own toolchain can emit both argument set-ups at the FRONT of a func_80073728 call block; the Case3 block is the exception, not a toolchain limitation.
- mechanism: If a sibling call site with the same callee and the same statement shape (struct-field stores then the call) already schedules args-first in OUR build, then nothing about the compiler or the callee prevents target's order - the decision is block-local.
- probe: `sandbox func_8005D46C --disable all` (src/text1b.c:12718, two call sites of the same callee), then objdump the cheat-free sandbox object and read the call blocks.
- result: Our build emits `addiu a0,sp,16 / move a1,zero` as the first two instructions of the call's basic block, ahead of the entire nine-store cluster - exactly the order func_80060544's target has and our func_80060544 build does not. Visible difference: the sibling's block is ~20 insns with several independent chains of height >= 4; the Case3 block is five insns with one chain of height 2 (but s3 separately measured that ADDING chain height to Case3 does not reproduce it, so 'make the block bigger' is not by itself the lever). Excerpts in tmp/grind/func_80060544/s3/results.txt.
- verdict: CONFIRMED

## Session s4b (2026-08-03, modality: permuter)

### H-P1 — "A directed permuter campaign that cannot use staging carriers can still reach 0."
**Statement.** The judge-FAILed score-0 form is a dead-store staging carrier
(`end_off = stat; s.p_static = (s32 *)end_off;`). If the `move a1,zero` placement is
reachable by any structurally different C shape, a campaign with the whole
extra-assignment/staging mutation family disabled should find it.
**Mechanism.** decomp-permuter's staging family (`perm_temp_for_expr`,
`perm_duplicate_assignment`, `perm_chain_assignment`, `perm_long_chain_assignment`,
`perm_add_self_assignment`) is exactly the set of passes that manufacture extra
assignments through existing or new locals. Zeroing their weights leaves the reordering,
expression-shape, type, condition and structural passes to search alone.
**Probe.** `tools/permuter_campaign.py launch --label no-staging-A -j 6` on the validated
`tmp/perm_60544` chassis (base score 60 == sandbox floor 2); three in-turn `wait` windows;
`harvest --stop`.
**Result.** 31,745 iterations / 20.5 min. No find below base. One sideways find at 60.
**Verdict: KILLED.**

### H-P2 — "The full mutation space contains score-0 forms other than the staging carrier."
**Statement.** Re-enabling every pass and re-seeding should enumerate additional distinct
score-0 forms, at least one of which might survive the Judge.
**Mechanism.** The first s4 campaign found its score-0 quickly with default weights, so the
basin was assumed cheap and possibly wide; a longer default-weight run should show whether
it is one point or a family.
**Probe.** `launch --label full-space-B -j 6` on the same chassis, defaults except
`perm_inline = 0.0`; three in-turn `wait` windows; `harvest --stop`.
**Result.** 32,075 iterations / 23 min. No score-0 at all this time — the first s4 find was
luck, not a wide basin. One sideways find at 60, textually identical to campaign A's.
**Verdict: KILLED.**

### Why this axis is structurally hostile to the permuter (record this, do not re-derive)
The permuter's score for this function is QUANTISED. The whole residual is a single
reordering, worth 60 points; every register, every store, every delay slot already matches.
So the reachable scores are 60 (unchanged) and 0 (matched) with nothing in between: there
is no gradient to climb and the permuter degenerates into uniform random sampling of C
respellings. 63,820 samples found exactly one point at 0 across the project's whole history
of this function, and that point is a dead store the Judge rejected. Any future permuter
work on func_80060544 must therefore change the CHASSIS (a structurally different starting
form), not the iteration count — repeating this search is measured waste.

## [s4] A directed permuter campaign that cannot use staging carriers can still reach score 0 for func_80060544.
- mechanism: The judge-FAILed score-0 form is a dead store into an already-consumed local (`end_off = stat; s.p_static = (s32 *)end_off;`). decomp-permuter's staging family (perm_temp_for_expr, perm_duplicate_assignment, perm_chain_assignment, perm_long_chain_assignment, perm_add_self_assignment) is exactly the set of passes that manufacture such extra assignments; zeroing their weights leaves the reordering, expression-shape, type, condition and structural passes to search alone, so any find would be a structurally different route to the same bytes.
- probe: Built and validated the workspace tmp/perm_60544 (mkws.sh: reduced TU, cheat-free pipeline with regfix/regfix_stage2/asmfix omitted, per-function region extraction; validation gate = instruction-identical to the cheat-free sandbox build, 133 insns, single `move a1,zero` displacement, base score 60). Launched via tools/permuter_campaign.py (label no-staging-A, -j 6) with the staging family zeroed in settings.toml; waited three in-turn windows; harvest --stop.
- result: 31,745 iterations / 20.5 min. No find below base. Exactly one sideways find at score 60 (== base) at t+80s.
- verdict: KILLED

## [s4] The full permuter mutation space contains score-0 forms for func_80060544 other than the judge-FAILed staging carrier.
- mechanism: The first s4 campaign found its score-0 quickly under default weights, which would suggest a wide, cheap basin; a longer default-weight run should enumerate whether that point is one isolated form or a family with a Judge-survivable member.
- probe: Same validated chassis, relaunched with defaults except perm_inline = 0.0 (label full-space-B, -j 6); three in-turn wait windows; harvest --stop.
- result: 32,075 iterations / 23 min. No score-0 at all, and nothing else below base. One sideways find at score 60 whose function body is textually IDENTICAL to campaign A's, so both chassis converged on the same equal-score alternative spelling. The first s4 score-0 was luck, not a wide basin.
- verdict: KILLED

## [s4] The permuter can hill-climb this function's residual at all.
- mechanism: The permuter's weighted score is regs x5, reorderings x60, ins/del x100. Every register, store and delay slot in func_80060544 already matches target and build_insns == target_insns == 133; the entire residual is ONE reordering.
- probe: Read the campaign base score (60) off both campaign_meta.json files and compared it with the sandbox floor (2), then observed the score distribution of 63,820 sampled forms.
- result: The score is QUANTISED: the only reachable values are 60 (the one misplacement, unchanged) and 0 (matched). No intermediate score was observed in 63,820 samples because none exists. The permuter therefore degenerates to uniform random sampling of C respellings with no gradient to follow, which is the structural reason this axis is dry.
- verdict: CONFIRMED

## Session s5 (2026-08-03, modality: permuter) — floor 2 -> 2

### KILLED

**H-F1 (the s3/s4 frontier) — "target's build won the Case3 `move a1,zero`
placement on INSN_PRIORITY outright; the deciding quantity is the a1 set-up's own
INSN_PRIORITY (or its presence in the ready list)."**
Mechanism assumed by s3: `rank_for_schedule` (tools/gcc-2.7.2/sched.c:2399) compares
INSN_PRIORITY first, then a 3-way class relative to `last_scheduled_insn`, then
INSN_LUID; s3 argued both tiebreaks favour the a1 set-up in our build, so target must
have beaten it on priority.
Probe: ran the INSTRUMENTED `tools/gcc-2.7.2/cc1` (NOT build/cc1) with
`BB2_PRIO_DEBUG=1 BB2_RANK_DEBUG=1 -da` on BOTH reduced-TU variants — `v_base.c`
(our floor-2 form, a1 emitted late) and `v_reuse.c` (the judge-FAILed score-0 staging
form, a1 emitted first). Script `tmp/grind/func_80060544/s5/rank_probe.sh`; dumps in
`tmp/grind/func_80060544/s5/rank_base/` and `rank_reuse/`.
Result: **every insn in the Case3 block has `final_pri=1` in BOTH variants**, and
**every RANKDBG comparison inside that block reports `cls=3 x=... cls2=3 val=0`** —
i.e. priority is a universal tie and the class test is a universal tie, in the WINNING
variant as well as the losing one. Mapping the two UID sets onto each other
(base 136/139/142/145/147/149 <-> reuse 136/142/145/148/150/152) shows the two builds
perform the SAME four comparisons with the SAME results. Nothing in the priority or
class data distinguishes them. **Verdict: KILLED.** INSN_PRIORITY is not the lever and
must not be probed again; GCC 2.7.2's own comment at sched.c:1472 explains why ("when
all instructions have a latency of 1 ... all instructions will end up with a priority
of one"), so on this target the whole block is always priority 1.

**H-F2 — "the flip is an EMISSION-order difference (expand_call emitting the second
argument earlier), not a scheduling one"** (the s2 frontier's alternative (c)).
Probe: `tmp/grind/func_80060544/s5/passorder.py` prints, for every `-da` dump of both
variants, the linear order of the `D_8009B7D0` set and the `a1` set inside the Case3
block. Result: in BOTH variants the order is la-then-a1 through .rtl, .jump, .cse,
.loop, .cse2, .flow and .combine, and it is only in **.sched (sched1)** that the reuse
variant flips to a1-then-la; base never flips. **Verdict: KILLED** — emission order is
identical in both; sched1 owns the decision, exactly one pass, and every later dump
(.lreg/.greg/.jump2/.sched2/.dbr) merely carries the sched1 order forward.

**H-F3 — "a deleted-insn corpse (NOTE_INSN_DELETED) in the Case3 block is what flips
sched1."** This was the natural reading of the .flow RTL: base's block is
label/la/sw/sw/a0/a1/call, and the score-0 reuse variant's block is byte-identical
except for one extra `(note NOTE_INSN_DELETED)` sitting between the la and the
`sw p_static` (the corpse of the coalesced staging copy). If a note slot shifts the
scheduler's bookkeeping, ANY construct leaving a corpse there would flip it — including
several that are not dead stores.
Probe: `tmp/grind/func_80060544/s5/note_sweep.py` — nine Case3-arm variants compiled
with the instrumented cc1, each measured for (a) emitted order of `move $5,$0` vs the
`la`, (b) NOTE_INSN_DELETED count inside the block at .flow, (c) asm line count.
Result:
```
V0_base              a1-after  notes=0   (control)
V1_endoff_before     A1-FIRST  notes=1   (the judge-FAILed form; the ONLY flip)
V2_endoff_after      a1-after  notes=1
V3_geom_before       a1-after  notes=1
V5_newlocal_before   a1-after  notes=0
V6_double_store      a1-after  notes=1
V7_self_assign       a1-after  notes=0
V9_regeom_restore    a1-after  notes=0   (118 asm lines — adds a real insn)
V11_endoff_2hop      a1-after  notes=1
```
Four distinct variants produce the corpse note and do NOT flip. **Verdict: KILLED** —
the note is a side effect, not the cause.

### CONFIRMED (new, and it is the whole s5 finding)

**H-F4 — the flip is specific to the CARRIER IDENTITY and the HOP COUNT, not to any
structural property of the block.** The only Case3-arm form in the whole measured
space that flips the schedule is `end_off = stat;` placed IMMEDIATELY before the
`s.p_static` store with EXACTLY one hop. Staging the same value through the same
variable one statement LATER (V2), through a different multiply-assigned dead local
`geom` (V3), through a fresh local (V5), or through `end_off` and then `new_var6` (V11)
all leave the a1 set-up where it was. This is the mechanism-level explanation of s4's
H9 carrier-specificity observation, and it is bad news for every remaining C-level
axis: the property that decides this function's last two points is a sched1 bookkeeping
difference keyed to one particular pseudo's identity, reachable (so far) only by a
dead store into an already-consumed local — the exact construct the Judge FAILed.

### The permuter campaign (mandated modality)

**H-P3 — a STRUCTURALLY DIFFERENT chassis (not the floor-2 candidate) reaches 0 by a
clean route.** Built `tmp/perm_60544_alt` (`tmp/grind/func_80060544/s5/mkalt.sh`):
pointer-typed `stat` carrier (`s32 *stat` + `s.p_static = stat;`), `while (i < 4)` loop
head instead of `do/while`, and the `stat` declaration moved to the end of the
declaration list — three changes s3 measured individually INERT (2/133), so the chassis
starts at the same floor but with different pseudo numbering, different types and a
different loop head. Validation gate passed: 133 insns, single `move a1,zero`
displacement, permuter base score 60. Launched with the staging/extra-assignment
mutation family ZEROED (a find inside that family would be the judge-FAILed construct
and therefore worthless).
Result: see the s5 evidence entries — the campaign is recorded there with its
iteration count and finds.

## [s5] The s3/s4 frontier: target's build won the Case3 `move a1,zero` placement on INSN_PRIORITY outright, so the deciding quantity is the a1 set-up's own INSN_PRIORITY (or whether it is in the ready list at that step).
- mechanism: rank_for_schedule (tools/gcc-2.7.2/sched.c:2399) compares INSN_PRIORITY, then a 3-way class relative to last_scheduled_insn, then INSN_LUID. s3 argued both tiebreaks favour the a1 set-up in our build, so target must have beaten it on priority.
- probe: Ran the INSTRUMENTED tools/gcc-2.7.2/cc1 (not build/cc1) with BB2_PRIO_DEBUG=1 BB2_RANK_DEBUG=1 -da on BOTH reduced-TU variants: v_base.c (our floor-2 form, a1 emitted late) and v_reuse.c (the judge-FAILed score-0 staging form, a1 emitted first). Script tmp/grind/func_80060544/s5/rank_probe.sh; dumps in s5/rank_base/ and s5/rank_reuse/.
- result: EVERY insn in the Case3 block reports final_pri=1 in BOTH variants, and EVERY rank_for_schedule comparison inside that block reports cls=3 cls2=3 val=0 — a total tie on the priority test and on the class test, in the WINNING variant as well as the losing one. Mapping the UID sets onto each other (base 136/139/142/145/147/149 <-> reuse 136/142/145/148/150/152) shows the two builds perform the SAME four comparisons with the SAME results. GCC 2.7.2's own comment at sched.c:1472 explains it: on a latency-1 target priority()-1 collapses to 1 for the whole block.
- verdict: KILLED

## [s5] The flip is an EMISSION-order difference (expand_call emitting the second argument earlier), not a scheduling one — the s2 frontier's alternative (c).
- mechanism: If the a1 set-up's RTL position already differed before sched1, the lever would be the C statement shape that makes expand_call emit the constant argument earlier, and the sibling func_8005D46C would be the reference.
- probe: tmp/grind/func_80060544/s5/passorder.py prints, for every -da dump of both variants, the linear order of the D_8009B7D0 set and the a1 set inside the Case3 block.
- result: Both variants emit la-then-a1 through .rtl, .jump, .cse, .loop, .cse2, .flow and .combine; only the score-0 variant flips to a1-then-la in .sched (sched1), and every later dump (.lreg/.greg/.jump2/.sched2/.dbr) carries that order forward. Emission order is identical in the winning and losing builds; sched1 owns the decision, exactly one pass.
- verdict: KILLED

## [s5] A deleted-insn corpse (NOTE_INSN_DELETED) sitting between the la and the p_static store is what flips sched1 — so ANY construct leaving a corpse there, including several that are not dead stores, would reproduce target's order.
- mechanism: At .flow the winning and losing RTL are identical except for one (note NOTE_INSN_DELETED) at that position — the corpse of the coalesced staging copy — which would shift the scheduler's per-insn bookkeeping.
- probe: tmp/grind/func_80060544/s5/note_sweep.py: nine Case3-arm variants compiled with the instrumented cc1, each measured for emitted order of `move $5,$0` vs the `la`, NOTE_INSN_DELETED count inside the block at .flow, and asm length.
- result: V0 base a1-after notes=0; V1 end_off staged immediately before the store A1-FIRST notes=1 (the only flip, and it is the judge-FAILed form); V2 same staging one statement LATER a1-after notes=1; V3 staging through `geom` a1-after notes=1; V5 staging through a fresh local a1-after notes=0; V6 duplicated s.p_static store a1-after notes=1; V7 self-assignment a1-after notes=0; V9 redundant s.p_geom re-store a1-after notes=0 (adds a real insn, 118 asm lines); V11 two-hop end_off->new_var6 a1-after notes=1. Four distinct variants produce the corpse and do NOT flip.
- verdict: KILLED

## [s5] The flip is keyed to CARRIER IDENTITY and HOP POSITION — one specific pseudo (`end_off`), staged with exactly one hop, immediately before the p_static store — and not to any structural property of the block.
- mechanism: sched1 bookkeeping tied to one pseudo's identity. This is the mechanism-level explanation of s4's H9 carrier-specificity observation (only end_off reaches 0; geom, also multiply-set and dead, is inert).
- probe: Same nine-variant note_sweep.py table read as a carrier/position matrix rather than a corpse-count matrix.
- result: Only V1 flips. Same variable one statement later (V2), a different multiply-assigned dead local (V3), a fresh local (V5), and two hops through the same variable (V11) are all inert. This is why every C-level structural axis has measured flat since s2, and it is why the only known route to 0 remains the judge-FAILed dead store.
- verdict: CONFIRMED

## [s5] A permuter campaign seeded from a STRUCTURALLY DIFFERENT chassis (not the floor-2 candidate) can still reach 0 by a clean route — the s4 frontier's third item, and the mandated modality for this session.
- mechanism: The permuter's score for this function is quantised (60 or 0), so its hit probability depends on the starting form's mutation-distance to a matching form, i.e. on the chassis rather than on iteration count. A chassis built from respellings s3 measured individually inert starts at the same floor with different pseudo numbering, types and loop head.
- probe: Built tmp/perm_60544_alt via tmp/grind/func_80060544/s5/mkalt.sh — pointer-typed `stat` carrier (s32 *stat, s.p_static = stat, no casts), `while (i < 4)` loop head instead of do/while, `stat` declared last. Validation gate passed: 133 insns, single `move a1,zero` displacement, permuter base score 60. Launched via tools/permuter_campaign.py (label alt-chassis-C, -j 6) with perm_inline plus the whole staging/extra-assignment family zeroed (perm_temp_for_expr, perm_duplicate_assignment, perm_chain_assignment, perm_long_chain_assignment, perm_add_self_assignment), so any find would be outside the judge-FAILed family. Two in-turn wait windows, then harvest --stop.
- result: 32,859 iterations / 1,293 s. Exactly ONE find, at score 60 (== base), at t+104 s: a do-while(0) wrapper around the goto ladder plus (&s)->p_static instead of s.p_static — the same noise class as s4's two sideways finds. Nothing below base. Campaign stopped inside the session; ps aux confirms zero permuter processes remain. Cumulative across s4+s5: three chassis, 96,679 sampled forms, exactly one score-0 point in the function's whole history, and that point is the judge-FAILed dead store.
- verdict: KILLED

## Session s6 (2026-08-03, modality: forensics) — floor 2 -> 2 (the mechanism is now NAMED and PREDICTIVE)

s6 answered the s5 frontier question outright. The instrument was already in the
tree and unused: `tools/gcc-2.7.2/sched.c` carries a THIRD env-gated hook,
`BB2_SCHED_DEBUG` (sched.c:3691 and :3950), which prints, for every scheduling
step of every block, `SCHEDDBG PICK clock=<n> picked=<uid> (pri=<n> luid=<n>)`
followed by the FULL ready-list array with each entry's priority and LUID. That
is exactly the "instrument schedule_block's ready-list array directly" probe the
s5 frontier asked for, and it needed no change to `tools/` — s5 only knew about
`BB2_PRIO_DEBUG` and `BB2_RANK_DEBUG`.

### CONFIRMED — H-F5: the deciding quantity is an adjust_priority()/birthing_insn_p() promotion of the Case3 `la` to LAUNCH_PRIORITY, and it is reachable only by a DEAD staging store into a carrier that has exactly ONE other assignment.

**The measurement.** `tmp/grind/func_80060544/s6/sched_probe.sh` compiles both
reduced-TU variants (`v_base.c` = our floor-2 candidate, `v_reuse.c` = the
judge-FAILed score-0 staging form) with `BB2_SCHED_DEBUG=1 -da`. The Case3 arm is
block 9 in both, 7 insns, and the two blocks map onto each other one-for-one by
INSN_REF_COUNT (base 136/139/142/145/147/149/151 <-> reuse
136/142/145/148/150/152/154, the reuse list carrying one extra NOTE at 139):

| role | base uid (luid) | reuse uid (luid) |
|---|---|---|
| `la` D_8009B7D0 | 136 (0) | 136 (0) |
| `sw` p_static   | 139 (1) | 142 (2) |
| `sw` pad0C      | 142 (2) | 145 (3) |
| `a0 = fp+24`    | 145 (3) | 148 (4) |
| `a1 = 0`        | 147 (4) | 150 (5) |
| `call`          | 149 (5) | 152 (6) |
| post-call copy  | 151 (6) | 154 (7) |

sched1 schedules the block BACKWARD (first picked = last emitted). Both builds
pick identically for the first four steps — post-call copy, call, `sw` pad0C,
`sw` p_static. The divergence is one single step, clock=5:

```
base : ready was: [ 147(p=1,l=4) 145(p=1,l=3) 136(p=1,l=0) ]          -> picks 147 (a1 = 0)
reuse: ready was: [ 136(p=2130706433,l=0) 150(p=1,l=5) 148(p=1,l=4) ] -> picks 136 (the la)
```

`2130706433` is `0x7f000001` = `LAUNCH_PRIORITY` (sched.c:187). So in the winning
variant the `la` does not win a tie — it is not in the tie at all; it has been
PROMOTED. The only path to that value for a not-yet-scheduled insn is
`adjust_priority()` (sched.c:2531-2575), called on each insn as it becomes ready,
which for `n_deaths == 0` (always, per GCC's own comment at sched.c:2544 —
REG_DEAD notes are gone by then) does
`if (birthing_insn_p (PATTERN (prev))) INSN_PRIORITY (prev) = max_priority;`
where `max_priority` is `MAX (INSN_PRIORITY (ready[0]), INSN_PRIORITY (insn))`
(sched.c:2601) and `insn` is the just-scheduled insn, which schedule_block has
temporarily set to LAUNCH_PRIORITY (sched.c:3985). `birthing_insn_p`
(sched.c:2496) is `bb_live_regs[dest] && reg_n_sets[dest] == 1`.

**Why this closes the s5 frontier.** s5 correctly found that INSN_PRIORITY is a
universal 1 and that every `rank_for_schedule` comparison inside the block
reports `cls=3 cls2=3 val=0`. What s5 could not see is that `RANKDBG` prints
BEFORE the third tiebreak: `rank_for_schedule` then returns
`INSN_LUID (y) - INSN_LUID (x)`, i.e. HIGHER LUID SORTS FIRST. The a1 set-up is
emitted by expand_call after the stores, so it always holds the highest LUID of
the three remaining insns and always wins the tie. The ONLY way the `la` gets
scheduled before it is the birthing promotion. The frontier's "ready-list arrival
order / LOG_LINKS edges" hypothesis is therefore also dead: arrival order and
dependence edges are identical in both builds (same picks for clocks 1-4, same
ready-set membership at clock 5); the differing quantity is one insn's priority.

**The predicate, measured.** `tmp/grind/func_80060544/s6/carrier_setcount.py`
stages the Case3 address through nine different existing locals and reports the
emitted order plus whether the `la` shows up in a ready list with
LAUNCH_PRIORITY. `others` = number of OTHER assignments that local already has:

```
carrier    others  order     dest/launch          size
fresh      0       a1-after  dest=r76 launch=0    asm_lines=117
end_off    1       A1-FIRST  dest=r77 launch=1    asm_lines=117
last       1       a1-after  dest=r76 launch=0    asm_lines=117
new_var3   1       a1-after  dest=r87 launch=0    asm_lines=117
new_var6   1       a1-after  dest=r82 launch=0    asm_lines=117
geom       2       a1-after  dest=r74 launch=0    asm_lines=117
idx        2       a1-after  dest=r81 launch=0    asm_lines=117
mid_off    2       A1-FIRST  dest=r78 launch=1    asm_lines=113  (semantics broken)
prev       2       a1-after  dest=r83 launch=0    asm_lines=117
```

A naive "reg_n_sets == 1" reading is FALSIFIED by this table (`last`,
`new_var3`, `new_var6` all have exactly one other set and do not fire). The
model that fits every row is a CONJUNCTION:

  (1) the carrier must be DEAD after the `s.p_static` store — never read again
      before it is overwritten. Only then do flow/combine delete the copy and
      RETARGET the `la`'s destination onto the carrier's pseudo (visible in the
      `.flow` dump: base's `la` sets `reg 75` = `stat`; the `end_off` variant's
      sets `reg 77` = `end_off`, with the copy's corpse note left behind); and
  (2) the carrier must have EXACTLY ONE other assignment, so that combine's
      decrement of `reg_n_sets` on deleting the copy (combine.c:2309/2332) lands
      on 1 and satisfies birthing_insn_p. A fresh local lands on 0; a
      twice-assigned local lands on 2.

`last` / `new_var3` / `new_var6` fail (1) — each is genuinely read later, so the
copy survives. `geom` / `idx` / `prev` fail (2). `mid_off` satisfies both only
because staging through it destroys the function (the following
`s.pad0C = mid_off;` then stores the table address; 113 lines). `end_off` is the
ONLY local in this function satisfying both — which IS s4's H9 carrier
specificity and s5's H-F4, now explained rather than merely observed.

**The predicate CONFIRMED in both directions**
(`tmp/grind/func_80060544/s6/predicate_test.py`, diagnostics only):

```
P0_base                a1-after  dest=r75  launch=0  notes=0  asm_lines=117
P1_manufactured_dead   A1-FIRST  dest=r75  launch=1  notes=1  asm_lines=117
P2b_endoff_live_later  a1-after  dest=r77  launch=0  notes=1  asm_lines=117
```

P1 manufactures a brand-new local with exactly one prologue assignment and no
later read, and stages through it: it flips, at the SAME instruction count —
a second, independent route to target's block order, and it is a dead store, so
it is a DIAGNOSTIC and never a candidate. P2b keeps `end_off` as the carrier but
gives it a genuine later read (returning through it instead of `new_var6`): the
copy is then live, the retarget does not happen, and the flip disappears. Both
predictions of the model were made before the runs and both held.

**The consequence — the important part for the ladder.** The complete case
analysis for this block is now closed:
  - INSN_PRIORITY tie: universal 1 (s5), immovable by dependence height (s3).
  - class tie: universal 3 because every insn on this target has latency 1 (s5).
  - LUID tiebreak: the a1 set-up always has the highest LUID, because expand_call
    emits argument set-up at the call and emission order is identical in the
    winning and losing builds (s5 H-F2). No statement ordering changes this — s2
    measured all six intra-arm orderings and all five call-argument respellings.
  - therefore the ONLY remaining lever is the birthing promotion, and the
    promotion requires a carrier that is DEAD after the store.
A carrier that is dead after the store IS a dead store. So on the measured
evidence there is no live-data pure-C spelling of this basic block that
reproduces target's instruction order; every route to the bytes passes through
the construct family the Judge already FAILed
(`rejected/judge-fail-0803-1310.c`). This is the strongest statement yet of why
every C-level axis has measured flat since s2, and it is the analysis an
escalation would have to cite — but the driver, not this session, decides
exhaustion, so s6 returns `progress` with the mechanism banked.

### KILLED — H-F6: the Case3 `la`'s destination can be made a single-set pseudo by giving Case3 its own address local instead of sharing `stat`.
Mechanism: birthing_insn_p tests `reg_n_sets[dest] == 1`, and the shared `stat`
carrier is assigned in all three arms (reg 75, 3 sets), so a per-arm local is the
obvious clean way to satisfy the predicate without any dead store.
Probe: `tmp/grind/func_80060544/s6/arm_sweep.py`, four forms (Case3-only local;
all three arms with their own locals; the OTHER two arms with their own locals so
the shared name becomes Case3-only; Case3-only pointer-typed local), each
measured for emitted order, `.flow` destination pseudo and its set count,
LAUNCH_PRIORITY appearance, and function size, with the judge-FAILed staging form
as a positive control.
Result: all four DO produce a single-set destination pseudo and NONE fires the
promotion (launch=0); all four cost +2 to +5 instructions (119/122/122/119 vs the
base's 117). The s1 H2 shared carrier is load-bearing and cannot be split per
arm. Banked in `rejected/case3-single-set-carrier-locals.c`.
Verdict: KILLED.

## THE FRONTIER AFTER s6

State unchanged: `sandbox --disable all` == 2, build_insns == target_insns == 133,
single non-branch diff hunk = the `move a1,zero` placement in the Case3 arm.
What s6 adds is a PREDICTIVE, sub-second test for any future candidate: compile
the reduced TU with `BB2_SCHED_DEBUG=1` and check whether the Case3 `la` appears
in a ready list with `p=2130706433`. If it does not, the form cannot match, and
no sandbox run is needed. Four artifacts do this out of the box
(`sched_probe.sh`, `arm_sweep.py`, `carrier_setcount.py`, `predicate_test.py`).

Remaining ladder rungs are rederive / synthesis. The one C-level question the
model leaves genuinely open — and it is narrow — is whether some restructuring of
the FUNCTION (not of the Case3 arm) can make a variable that is genuinely live in
the original semantics happen to be dead at exactly this point while still
carrying exactly one other assignment. Every such candidate is now a one-command
test.

## [s6] The Case3 `move a1,zero` placement is decided by the sched1 ready-list ARRIVAL ORDER / the LOG_LINKS dependence edges built by sched_analyze (the s5 frontier), rather than by any per-insn quantity.
- mechanism: s5 had shown INSN_PRIORITY is a universal 1 and every rank_for_schedule comparison reports cls=3 cls2=3 val=0, so the sorted ready list looked as if it were determined entirely by its pre-sort contents and order, which are a function of the dependence graph.
- probe: Ran the instrumented tools/gcc-2.7.2/cc1 with BB2_SCHED_DEBUG=1 -da on both reduced-TU variants (v_base.c = our floor-2 candidate, v_reuse.c = the judge-FAILed score-0 staging form) via tmp/grind/func_80060544/s6/sched_probe.sh, and read the full per-step ready-list arrays for the Case3 block (block 9, 7 insns in both) with tmp/grind/func_80060544/s6/blk.py. BB2_SCHED_DEBUG is a THIRD env-gated hook in sched.c (lines 3691 and 3950) that s3/s5 did not know about; it prints `SCHEDDBG PICK clock=<n> picked=<uid> (pri=<n> luid=<n>)` plus the entire ready array with priorities and LUIDs, so no change to tools/ was needed.
- result: Clocks 1-4 are IDENTICAL in both builds (post-call copy, call, sw pad0C, sw p_static), so the dependence edges, the ready-set membership and the arrival order are identical. The divergence is a single step, clock=5: base's ready list is [147(p=1,l=4) 145(p=1,l=3) 136(p=1,l=0)] and it picks 147 (the a1 set-up); reuse's is [136(p=2130706433,l=0) 150(p=1,l=5) 148(p=1,l=4)] and it picks 136 (the la). Exactly one insn's PRIORITY FIELD differs; nothing about the graph does.
- verdict: KILLED

## [s6] H-F5 -- the deciding quantity is a LAUNCH_PRIORITY promotion of the Case3 `la` by adjust_priority()/birthing_insn_p(), and it is reachable only by a staging store into a carrier that is DEAD after the s.p_static store and that has EXACTLY ONE other assignment in the function.
- mechanism: 2130706433 == 0x7f000001 == LAUNCH_PRIORITY (sched.c:187). schedule_block temporarily sets the just-scheduled insn to LAUNCH_PRIORITY (sched.c:3985); schedule_insn computes max_priority = MAX(INSN_PRIORITY(ready[0]), INSN_PRIORITY(insn)) (sched.c:2601) and calls adjust_priority on each newly-ready insn; adjust_priority (sched.c:2531) always takes its n_deaths==0 branch (GCC's own comment at sched.c:2544: REG_DEAD notes are already gone) and does `if (birthing_insn_p (PATTERN (prev))) INSN_PRIORITY (prev) = max_priority;`. birthing_insn_p (sched.c:2496) is `bb_live_regs[dest] && reg_n_sets[dest] == 1`. A staging copy that is dead lets flow/combine delete the copy and RETARGET the la's destination onto the carrier's pseudo, and combine DECREMENTS reg_n_sets when it deletes a set (combine.c:2309/2332) -- so a carrier with exactly one other assignment lands on the 1 that birthing_insn_p tests.
- probe: tmp/grind/func_80060544/s6/carrier_setcount.py staged the Case3 address through nine existing locals, reporting emitted order, the la's .flow destination pseudo, whether that UID appears in a ready list with p=2130706433, and the function's asm size. Then tmp/grind/func_80060544/s6/predicate_test.py tested the model in both directions with predictions stated in advance: P1 = a manufactured brand-new local with exactly one prologue assignment and no later read (predicted FLIP); P2b = `end_off` kept as the carrier but given a genuine later read, returning through it instead of new_var6 (predicted NO FLIP). Both are diagnostics, never candidate forms.
- result: Carrier table (others = pre-existing assignments): fresh/0 inert; end_off/1 FLIPS launch=1 at 117 asm lines; last/1, new_var3/1, new_var6/1 all inert (each is genuinely read later, so the copy survives and the retarget never happens); geom/2, idx/2, prev/2 inert; mid_off/2 flips only by destroying the function (113 lines -- the following s.pad0C = mid_off then stores the table address). A naive `reg_n_sets == 1` model is FALSIFIED by the last/new_var3/new_var6 rows; the conjunction model fits every row. P1 flipped at the same 117-line size (a second, independent route to target's block order -- and still a dead store); P2b stopped flipping. end_off is the ONLY local in this function satisfying both conditions, which is the mechanism behind s4's H9 and s5's H-F4 carrier specificity.
- verdict: CONFIRMED

## [s6] H-F6 -- the Case3 `la`'s destination can be made a single-set pseudo (satisfying birthing_insn_p) cleanly, by giving Case3 its own address local instead of sharing the `stat` carrier with the other two arms.
- mechanism: The shared `stat` carrier introduced by s1's H2 is assigned in all three arms (reg 75, three sets), so birthing_insn_p's reg_n_sets test fails. A per-arm local is the obvious pure-C way to reach reg_n_sets == 1 with no dead store anywhere.
- probe: tmp/grind/func_80060544/s6/arm_sweep.py: four forms measured with the instrumented cc1 -- Case3-only local; all three arms with their own locals; the OTHER two arms with their own locals so the shared name becomes Case3-only; Case3-only pointer-typed local (no casts) -- each reporting emitted order, the .flow destination pseudo and its textual set count, LAUNCH_PRIORITY appearance, and asm size, with the judge-FAILed staging form as the positive control (which registers A1-FIRST / launch=1 / 117 lines, validating the harness).
- result: All four DO produce a single-set destination pseudo and NONE fires the promotion (launch=0), and all four grow the function: 119, 122, 122, 119 asm lines against the base's 117. Single-set-ness is neither necessary nor sufficient; the s1 H2 shared carrier is load-bearing and cannot be split per arm. Banked in memory/grind/func_80060544/rejected/case3-single-set-carrier-locals.c.
- verdict: KILLED

## [s6] CORRECTION to s5's reading of rank_for_schedule: `every comparison reports val=0` means the comparator returned 0, so the ready order is decided purely by pre-sort contents.
- mechanism: s5 inferred a total tie from the RANKDBG output. But the RANKDBG fprintf (sched.c:2436-2446) sits between the class test and the LUID test.
- probe: Read tools/gcc-2.7.2/sched.c:2399-2456 directly and cross-checked against the measured clock-5 ready arrays, where the a1 set-up consistently sits at position 0 with the highest LUID of the three remaining insns.
- result: rank_for_schedule falls through to `return INSN_LUID (y) - INSN_LUID (x)`, i.e. the HIGHER LUID sorts FIRST. The a1 set-up, emitted by expand_call after all the stores, always holds the highest LUID at clock 5, so the tie is always resolved in its favour deterministically. This does not overturn any s5 verdict -- it sharpens why they were all dead, and it is what makes the birthing promotion the sole remaining lever.
- verdict: CONFIRMED

## Session s7 (2026-08-03, modality: forensics) — floor 2 -> 2 (the mechanism is now COMPLETE: two passes, not one)

s7 did what a forensics session is for: it found and explained the one
measurement in the ledger that CONTRADICTED the standing model, and the
explanation closes the case analysis rather than merely extending it.

### CONFIRMED — H-F7: the s6 arm_sweep anomaly is loop.c INVARIANT HOISTING. A dedicated single-set address local for the Case3 arm is loop-invariant, so the `la` is hoisted into loop 1's preheader and is not in the Case3 block at all when sched1 runs.

**The contradiction s6 left standing.** s6's model is
`birthing_insn_p == bb_live_regs[dest] && reg_n_sets[dest] == 1`, and s6's own
`arm_sweep.py` measured four forms (Case3-only local, all-three-arms-own-locals,
other-two-arms-own-locals, Case3-only pointer-typed local) that DO give the `la`
a single-set destination pseudo — and all four reported `launch=0`. Under the
stated model they should all have fired. s6 recorded the anomaly ("single-set-ness
is neither necessary nor sufficient") without explaining it.

**Probe.** `tmp/grind/func_80060544/s7/anom.py` re-reads the existing s6 dumps and
prints the `la`'s uid/destination/set-count from `.flow` AND `.combine` (the last
dump before sched1); then the emitted `out.s` of `aw_W0_base`, `aw_W1_case3_own`
and `aw_W4_case3_own_ptr` was checked for the POSITION of the `la` relative to
loop 1's head label.

**Result.**
```
aw_W0_base          la D_8009B7D0 at out.s:89  — INSIDE the loop (block .L12 = Case3)
aw_W1_case3_own     la D_8009B7D0 at out.s:42  — BEFORE .L2, i.e. in the PREHEADER
aw_W4_case3_own_ptr la D_8009B7D0 at out.s:42  — same
```
The address goes into a callee-save (`la $22,D_8009B7D0`) and lives across the
whole loop, which is exactly where those forms' +2 instructions come from. There
is no `la` in the Case3 block for adjust_priority to promote, so `launch=0` is
not a failure of the predicate — the insn under test has left the building.
**Verdict: CONFIRMED**, and it converts s6's "neither necessary nor sufficient"
into a precise two-pass mechanism (below).

### CONFIRMED — H-F8: the promotion is reachable with NO dead store when the Case3 arm is PEELED out of loop 1 — and the peel costs five instructions, so it cannot match.

**Statement.** R1 (the `la` must still be in the Case3 block at sched1) and R2
(its destination must have `reg_n_sets == 1`) pull against each other only
BECAUSE the arm is inside a loop. Take the `i == 3` iteration out of the loop and
a dedicated single-set local is no longer hoistable: both conditions hold at once
with no staging copy anywhere.

**Probe.** `tmp/grind/func_80060544/s7/peel_sweep.py`. Y1 = loop 1 runs `i = 0..2`
with the two ordinary arms, and the `i == 3` iteration (geom recompute, its own
address local `c3`, the `s.pad0C` store and the `func_80073728` call) is written
out after the loop. Y2 = the same peel with the peeled arm still using the shared
`stat` carrier, as a control that isolates the peel's own cost.

**Result.**
```
Y1_peel_own_local    A1-FIRST  la dest r75/1set   launch=1  asm_lines=112
Y2_peel_shared_stat  a1-after  la dest r75/3sets  launch=0  asm_lines=112
```
Y1 is the FIRST form in this function's history that reproduces target's Case3
block order with no dead store and no coercion — a third, independent
confirmation of the birthing-promotion model, and the first CLEAN one. It is
nevertheless rejected: the peel costs five instructions (112 asm lines against
the base's 117 == target's 133 insns), and Y2 shows the -5 is the peel's own cost,
not the local's. s3 had already established by instruction count that the
four-way in-loop ladder is the original's block structure; Y1/Y2 confirm it from
a second direction. **Verdict: CONFIRMED (mechanism) / KILLED (as a route).**
Banked in `rejected/case3-carrier-relocation-and-peel.c`.

### KILLED — H-F9 (the s6 frontier): relocating the carrier's OTHER assignment and its later read can make a genuinely-live variable dead at exactly the `s.p_static` store.

**Probe.** `tmp/grind/func_80060544/s7/reloc_sweep.py`, same sub-second gate:
```
X0_base (control)          a1-after  r75/3sets  launch=0  la_in_loop=1  117
X2_endoff_after_loops      a1-after  r77/2sets  launch=0  la_in_loop=1  117
X3_endoff_between_loops    A1-FIRST  r77/1sets  launch=1  la_in_loop=1  115
X4_endoff_after_nostage    a1-after  r75/3sets  launch=0  la_in_loop=1  115
X5_newvar3_late            a1-after  r87/2sets  launch=0  la_in_loop=1  117
X9_stage_ctl (judge-FAIL)  A1-FIRST  r77/1sets  launch=1  la_in_loop=1  117
```
**Result.** Moving `end_off`'s definition DOWNSTREAM of the store, to just before
the initTexPage tail (X2), leaves the staging copy undeleted (2-set destination)
and kills the promotion outright. Doing the same for `new_var3` (X5) likewise.
Moving `end_off`'s definition to sit between the two loops (X3) does flip — but
the relocation ITSELF costs two instructions (115; X4 shows the -2 is the
relocation, not the staging), so it can never reach target's 133 — and its Case3
statement is still a dead store, i.e. still the judge-FAILed family. There is no
relocation that makes a genuinely-read variable dead at exactly the p_static
store while leaving the rest of the function on target's instruction count.
**Verdict: KILLED.**

### THE COMPLETED CASE ANALYSIS (this is the s7 headline)

Everything the scheduler can do at that step is now enumerated, with the
mechanism named at each rung:
  - INSN_PRIORITY: universal 1 inside the block, in the winning build too (s5);
    immovable by dependence height (s3).
  - ready-list class: universal 3, because every insn on this target has
    latency 1 (s5).
  - LUID tiebreak: `rank_for_schedule` falls through to
    `INSN_LUID (y) - INSN_LUID (x)`, higher LUID first, and the `a1` set-up —
    emitted by expand_call after all the stores — always holds the highest LUID
    of the three insns left at clock 5 (s6). Emission order is identical in the
    winning and losing builds (s5 H-F2) and is inert to all six intra-arm
    statement orderings and all five call-argument respellings (s2).
  - therefore the ONLY override is adjust_priority's birthing promotion of the
    `la` to LAUNCH_PRIORITY (s6), which needs BOTH:
      (R1) the `la` still in the Case3 block at sched1 — with the arm inside
           loop 1 that forces a destination pseudo set MORE THAN ONCE in the
           loop (the shared `stat`), because a single-set address local is
           loop-invariant and loop.c hoists it (s7 H-F7); and
      (R2) `reg_n_sets[dest] == 1` at sched1 — which with a multiply-set `stat`
           can only arise from combine RETARGETING the `la` onto another pseudo,
           which requires deleting a copy, which requires the copied value never
           to be read. **That is a dead store, by definition, not by policy.**
  - the one shape that dissolves R1-vs-R2 without a dead store is peeling the
    arm out of the loop, and that costs five instructions (s7 H-F8).

So for this function, with target's loop structure held fixed (and s3 pinned that
structure by instruction count), reproducing target's Case3 instruction order in
pure C requires a dead store. This is no longer an induction over failed
respellings — it is a closed case analysis over the two passes that decide the
outcome, with a measured positive control at each branch.

## THE FRONTIER AFTER s7

State unchanged: `sandbox --disable all` == 2, build_insns == target_insns == 133,
single non-branch diff hunk = the `move a1,zero` placement in the Case3 arm.

What is left is NOT another scheduler probe. The remaining rungs are rederive /
synthesis, and s7 sharpens what they must produce: a reconstruction of the
ORIGINAL C whose Case3 arm naturally leaves a once-assigned local dead at the
static-table store. The most plausible reading, given the evidence, is that the
original's `end_off`-equivalent was a variable the programmer set up in the
prologue and re-used as a scratch handle in the Case3 arm (an idiom that is a
dead store to a modern reader but ordinary in 1997 hand-written C). Any such
reconstruction is one sub-second `BB2_SCHED_DEBUG` run from a verdict via
`s7/reloc_sweep.py` (add a row) — and if one is found, it still has to clear both
the 117-asm-line size gate and a fresh adversarial cheat-reviewer.

## [s7] The s6 arm_sweep anomaly — four forms whose Case3 `la` has a single-set destination pseudo, none of which fires the LAUNCH_PRIORITY promotion — is unexplained under the birthing_insn_p model.
- mechanism: A dedicated (Case3-only or per-arm) address local is assigned exactly once in the whole function, and the assigned value `&D_8009B7D0` is LOOP-INVARIANT. loop.c hoists it into loop 1's preheader, so at sched1 there is no `la` in the Case3 block at all — the promotion has no candidate, which is why `launch=0` despite the predicate being satisfied on paper. The shared `stat` carrier is immune to the hoist precisely because it is set in all three arms (three sets inside the loop).
- probe: tmp/grind/func_80060544/s7/anom.py re-read the s6 dumps for the la's uid/destination/set-count in .flow and .combine, and the emitted out.s of aw_W0_base / aw_W1_case3_own / aw_W4_case3_own_ptr was inspected for the la's position relative to loop 1's head label.
- result: base emits `la $2,D_8009B7D0` at out.s:89, inside the Case3 block (.L12); both single-set forms emit `la $22,D_8009B7D0` at out.s:42, BEFORE the loop head .L2 — hoisted into the preheader and held in a callee-save across the whole loop, which is exactly where their +2 instructions come from. The birthing predicate was never the thing that failed.
- verdict: CONFIRMED

- [s7] The Case3 promotion requires TWO conditions that pull against each other while the arm is inside loop 1: (R1) the `la` must still be in the block at sched1, which forces a destination pseudo set more than once inside the loop (the shared `stat`), because any single-set address local is loop-invariant and loop.c hoists it; and (R2) `reg_n_sets[dest] == 1` at sched1, which with a multiply-set `stat` can only come from combine retargeting the la onto another pseudo, which requires deleting a copy, which requires the copied value never to be read — a dead store by definition.

## [s7] Peeling the `i == 3` iteration out of loop 1 dissolves the R1/R2 conflict: a dedicated single-set address local for the peeled arm is no longer loop-invariant-hoistable, so the promotion can fire with no dead store anywhere.
- mechanism: loop.c can only hoist out of a loop. With the Case3 body written after the loop, `c3 = (s32)&D_8009B7D0;` stays where it is, keeps `reg_n_sets == 1`, and is live into the following `s.p_static` store — satisfying birthing_insn_p directly.
- probe: tmp/grind/func_80060544/s7/peel_sweep.py — Y1 (peeled arm with its own local `c3`) and Y2 (peeled arm still using the shared `stat`, as a control isolating the peel's own cost), both measured with the instrumented cc1 gate.
- result: Y1 = A1-FIRST, la dest single-set, launch=1, 112 asm lines. Y2 = a1-after, la dest 3 sets, launch=0, 112 asm lines. Y1 is the first form ever measured that reproduces target's Case3 block order with NO dead store — a clean, independent confirmation of the birthing model — but the peel costs five instructions against the base's 117 (== target's 133 insns), and Y2 shows the -5 is the peel itself. So the mechanism is confirmed and the route is dead. Banked in rejected/case3-carrier-relocation-and-peel.c.
- verdict: CONFIRMED (mechanism) / KILLED (as a route to the match)

## [s7] The s6 frontier — some relocation of the carrier's OTHER assignment and its later read can make a variable that is genuinely live in the function's semantics happen to be dead at exactly the s.p_static store, satisfying birthing_insn_p without a dead store.
- mechanism: The predicate is a conjunction of a liveness property and a set-count property, both of which are function-global; s6 only ever varied WHICH local was staged, never WHERE that local's other definition and its later read sat. Moving the definition downstream of the Case3 store should preserve deadness-at-the-store while keeping the variable genuinely read.
- probe: tmp/grind/func_80060544/s7/reloc_sweep.py, six variants on the sub-second instrumented-cc1 gate: end_off's definition moved to just before the initTexPage tail with staging kept (X2) and without (X4, cost control); moved to between the two loops (X3); new_var3's definition moved down to just before its own uses with staging through it (X5); base and the judge-FAILed staging form as negative/positive controls.
- result: X2 leaves the staging copy UNDELETED (la destination still 2 sets) and does not fire — moving the other definition downstream of the store breaks the combine retarget outright. X5 likewise (2 sets, launch=0). X3 fires (1 set, LAUNCH_PRIORITY, A1-FIRST) but the relocation itself costs two instructions (115 asm lines; X4 shows the -2 is the relocation, not the staging), so it cannot reach target's 133 — and its Case3 statement is still a dead store. No relocation makes a genuinely-read variable dead at the store while leaving the function on target's instruction count.
- verdict: KILLED

## [s7] The s6 arm_sweep anomaly — four forms whose Case3 `la` has a single-set destination pseudo, none of which fires the LAUNCH_PRIORITY promotion — has an explanation, and it is not a failure of birthing_insn_p.
- mechanism: A dedicated (Case3-only or per-arm) address local is assigned exactly once in the whole function and the value &D_8009B7D0 is LOOP-INVARIANT, so loop.c hoists the la into loop 1's preheader. At sched1 there is no la in the Case3 block at all, so adjust_priority has no candidate to promote. The shared `stat` carrier is immune to the hoist precisely because it is set in all three arms (three sets inside the loop).
- probe: tmp/grind/func_80060544/s7/anom.py re-read the existing s6 dumps for the la's uid/destination/set count in .flow and .combine; the emitted out.s of aw_W0_base, aw_W1_case3_own and aw_W4_case3_own_ptr was then inspected for the la's position relative to loop 1's head label .L2.
- result: Base emits `la $2,D_8009B7D0` at out.s:89, inside the Case3 block (.L12). Both single-set forms emit `la $22,D_8009B7D0` at out.s:42 — BEFORE .L2, i.e. hoisted into the preheader and held in a callee-save across the whole loop, which is exactly where their +2 instructions come from. The birthing predicate was never what failed.
- verdict: CONFIRMED

## [s7] The s6 frontier: relocating the staging carrier's OTHER assignment (and its later read) can make a variable that is genuinely live in the function's semantics happen to be dead at exactly the s.p_static store, satisfying birthing_insn_p without a dead store.
- mechanism: The predicate is a conjunction of a liveness property and a set-count property, both function-global; s6 only varied WHICH local was staged, never WHERE its other definition and later read sat. Moving the definition downstream of the Case3 store should preserve deadness-at-the-store while keeping the variable genuinely read.
- probe: tmp/grind/func_80060544/s7/reloc_sweep.py — six variants on the sub-second instrumented-cc1 gate (BB2_SCHED_DEBUG=1, reduced TU tmp/perm_60544/v_base.c), reporting emitted order, the la's destination pseudo and its set count in .combine (the last dump before sched1), LAUNCH_PRIORITY appearance, whether the la is still inside loop 1, and asm line count: end_off's definition moved to just before the initTexPage tail with staging (X2) and without (X4, cost control); moved to between the two loops (X3); new_var3's definition moved down to just before its own uses with staging through it (X5); plus base and the judge-FAILed staging form as negative/positive controls.
- result: X2 leaves the staging copy UNDELETED (la destination still 2 sets) and does not fire — a downstream definition breaks combine's retarget outright; X5 likewise (2 sets, launch=0). X3 does fire (1 set, LAUNCH_PRIORITY, A1-FIRST) but the relocation itself costs two instructions (115 asm lines; X4, the same relocation without staging, is also 115), so it can never reach target's 133 — and its Case3 statement is still a dead store. No relocation makes a genuinely-read variable dead at exactly the p_static store while leaving the function on target's instruction count.
- verdict: KILLED

## [s7] Peeling the i == 3 iteration out of loop 1 dissolves the R1/R2 conflict, letting a dedicated single-set address local stay in the Case3 block and fire the promotion with no dead store anywhere.
- mechanism: loop.c can only hoist out of a loop. With the Case3 body written after the loop, `c3 = (s32)&D_8009B7D0;` stays put, keeps reg_n_sets == 1, and is live into the following s.p_static store — satisfying bb_live_regs[dest] && reg_n_sets[dest] == 1 directly, with no staging copy to delete.
- probe: tmp/grind/func_80060544/s7/peel_sweep.py — Y1 (loop runs i = 0..2 with the two ordinary arms; the i == 3 iteration written out after the loop with its own address local) and Y2 (the same peel with the peeled arm still using the shared `stat` carrier, as a control isolating the peel's own cost).
- result: Y1 = A1-FIRST, la destination single-set, launch=1, 112 asm lines. Y2 = a1-after, 3 sets, launch=0, 112 asm lines. Y1 is the FIRST form in this function's history that reproduces target's Case3 block order with no dead store and no coercion — a third, independent and first CLEAN confirmation of the birthing-promotion model. It is dead as a route: the base is 117 asm lines == target's 133 instructions, so the peel is five instructions short, and Y2 shows the -5 is the peel itself. This re-confirms s3's instruction-count finding that the four-way in-loop ladder is the original's block structure, from a completely different direction.
- verdict: CONFIRMED

## Session s8b (2026-08-03, modality: rederive) — floor 2 -> **0**, and the 15:57 Judge remediation discharged

Dispatched with a digest that ended at s7, this session re-derived the deciding
pass from the other end of the lever ladder (the untouched m2c body at git HEAD)
before discovering that the earlier s8 session had already found the loop.c
route and had been FAILed at 15:57 on TWO COMMENT DEFECTS ONLY.  The
re-derivation stands as independent confirmation and produced three facts the
ledger did not have.

### CONFIRMED — H-R5: the a1 misplacement is not intrinsic to the Case3 block; s1's shared-`stat` lever introduced it.
Probe: `tmp/grind/func_80060544/s8/mkbodies.py` reconstructs the untouched m2c
body from `git show HEAD:src/text1b.c` and re-applies s1/s2's four levers in
every combination; `s8/gate.py` measures each.  Result: L0 (untouched m2c, three
DIRECT `s.p_static = &D_800…;` stores) already emits `move $5,$0` BEFORE the
`la D_8009B7D0` — target's order — and so does every combination that omits the
shared carrier (L1, L3, L4, L13, L14, L34, L134).  Every combination that
includes it (L2, L123, L124, L1234, L234) emits la-first.  Corroboration from
the rule corpus: the 11 regfix rules the sandbox drops for this function contain
a prologue reorder, six `la/sw $3 -> $2` substs, a `delete @ 77` and an `sll`
subst — and NO reorder rule for the Case3 a1 set-up, which is what one would
expect if the pre-s1 form had the order right.

### CONFIRMED — H-R6: the direct-store form's correct order is a reload-spill artifact, and target's build did not take that path.
Probe: read the `.combine` and `.greg` dumps of L0/L1 (`s8/g_L1/`).  The three
arm `la`s are hoisted by loop.c into the preheader (insns 340/342/344 setting
regs 92/93/94, emitted before `NOTE_INSN_LOOP_BEG`), so they are long-lived
pseudos; the `.greg` conflict table shows regs 92/93/94 conflicting with hard
regs 2/4/5, `Spilling reg 3.` is printed twice, and none of the three appears in
the register dispositions.  Reload then rematerialises `la $3` at each use from
the REG_EQUAL constant — which is why the la is back inside the arm in the
emitted asm even though `.combine` has it in the preheader, and why the a1
set-up (the only real insn left in the block at sched1 besides the store and the
call) is scheduled first WITHOUT any LAUNCH_PRIORITY promotion.
KILL for the obvious follow-up: this route cannot be steered to `$v0`.  Reload
chooses spill registers least-used-first, `$v1` is unused in our build, and
**target contains zero `$v1` references in the whole function** while using `$v0`
45 times.  A target build that had spilled would have used `$v1` exactly as ours
does.  So target's arm addresses are ordinary in-block pseudos allocated `$v0`,
and the direct-store chassis is dead as a route to 0 — not merely worse.

### CONFIRMED — H-R7: the loop.c hoist-block lever is general in its receiver and free only when the receiver is dead.
Probe: `s8/mkuses.py` + `gate.py`.  A Case3-arm address local `c3` with NO later
mention is hoisted (launch=0, 119 lines).  Adding a mention of `c3` in any later
basic block blocks the hoist and fires the promotion; the receiving variable is
irrelevant — `mid_off`, `geom`, `stat`, `end_off`, and the tail placement all
give launch=1 at **117** asm lines (== target's 133 instructions), and the
`mid_off` tail spelling was sandbox-verified at **score 0 / 133 / 133**.  This
contrasts sharply with s5's carrier-IDENTITY specificity, which applies only to
the in-arm staging route.
The negative half is the important one: every hoist-blocking mention that is
itself LIVE costs instructions target does not have — `p1 = (s32 *)(c3 + 0x70)`
= 119, `s.pad0C = c3;` in the `Skip` join = 120, and the earlier s8 session's
route B (post-loop static-table read) = sandbox 11 / 132.  The reason is
structural: a live carrier must survive loop 1 in a callee-save, whereas target's
Case3 address dies two instructions after the la.  With s7's peel result (the
only other dead-store-free route, five instructions short) this is now the third
independent direction from which a live-data spelling has been measured
impossible at target's instruction count.

### THE DISPOSITION
`candidate.c` is the byte-proven 15:56 body with COMMENT-ONLY fixes, exactly as
the 15:57 ruling instructed ("a re-submission with the two comment fixes and no
code change will be re-measured identically"):
  1. `last = 3;` now carries its own `/* FAKE: constant-holder … */` annotation
     naming the prologue-init-order mechanism, the lever exhaustion (s1-s8) and
     [[named-local-fake-exception]];
  2. the `c3` FAKE comment no longer calls the line "the Case3 static-table
     handle" — it states that the stored value is arbitrary and never read and
     that the line exists solely to give the pseudo an earlier reference before
     loop.c runs, then names loop.c:693-701 / loop.c:1062 and
     sched.c:2496/2531/2601.
A comment-stripped comparison against `rejected/judge-fail-0803-1556.c` confirms
the code is byte-for-byte the same (96 significant lines, no diff), and the
sandbox re-measured score 0 / 133 / 133 with the file in place.

## THE FRONTIER AFTER s8b
1. **Nothing structural is open.**  The remaining question is a REVIEW question,
   not a search question: whether the [[dead-store-fake-exception]] construct
   passes now that both annotation defects are fixed.  The 15:57 ruling states
   the construct is accepted in substance and the bytes are not in question, so
   a fresh Judge pass on this exact file is the next step.
2. **If the family is rejected after all**, the fallback is
   `memory/grind/func_80060544/prior-floor2-candidate.c` (the s2 form, distance
   2 / 133, whose only fake is the annotated `last = 3;`), and the function
   becomes an endgame-lock/escalation candidate whose analysis is already
   written (s6 + s7's completed case analysis, plus s8b's H-R6/H-R7 kills).
   Do NOT re-open the structural search: the direct-store chassis (H-R6), the
   live-carrier spellings (H-R7), the peel (s7 H-F8), the relocations (s7 H-F9),
   the permuter (s4/s5, 96,679 samples), and every C-level structural axis
   (s2/s3) are all measured dead.
3. **Do NOT re-derive the mechanism a third time.**  Read `rejected/*.c` headers
   and `grep 80060544 docs/grind/decisions.md` before any probing — the
   dispatched digest lagged a full session on this function and cost s8b roughly
   half its turns re-finding loop.c:693-701.
