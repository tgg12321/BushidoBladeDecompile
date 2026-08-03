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

## Live frontier (for the next session)

**H4 — the extra `move $s5,$s7` + the consequent `sll $a0,$s5,0x2` (target:
`sll $a0,$s7,0x2`). 2 of the remaining 4.**
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
