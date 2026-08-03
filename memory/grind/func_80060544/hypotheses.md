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
