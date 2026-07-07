# Evidence bank — func_8003FE40

- Audit diagnosis (regressions.md): Two named-constant variables neg1=-1 and neg1b=-1 fail the human-programmer and naming tests; they are register-coercion artifacts (originated as asm("t1")/asm("t0") explicit pins) that survived both cleanup passes. Owner should verify: sandbox with literal -1 substituted for both — if score stays 0, variables are inert noise and should be cleaned up; if score rises, they are still load-bearing allocation coercion requiring a clean redo.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Constructs located: src/config.c:410 `s32 neg1 = -1;` (compared at :416) and :417 `s32 neg1b = -1;` (compared at :425). The func_8003FE40 body (:393-433) carries NO other suspect constructs -- no pins, no asm, no volatile (the register-asm pins at config.c:293-298 belong to the preceding function).

- [s1] [fable-blitz 2026-07-07] Target-byte evidence of load-bearing: -1 is materialized TWICE -- `addiu $t1, $zero, -0x1` (asm/funcs/func_8003FE40.s:18, once, after the bltz guard) and `addiu $t0, $zero, -0x1` (s:24, INSIDE the outer loop on the beq fall-through, re-executed every outer iteration). $t1 dominates s:31's compare site and is never clobbered, so a single shared register would suffice -- the redundant second materialization implies two distinct C-level -1 entities, exactly what the two named locals provide.

- [s1] [fable-blitz 2026-07-07] Declaration sites map 1:1 to materialization sites: neg1 declared in the `count >= 0` block (:410) <-> s:18 after the bltz (s:16); neg1b declared inside `if (count != neg1)` (:417) <-> s:24 on the beq fall-through (s:22). This placement correspondence is the natural loop.c/cse.c consequence of block-local declarations.

- [s1] [fable-blitz 2026-07-07] Expected divergence mode for the judge's literal-substitution probe: with literal -1 in both compares, cse.c/loop.c plausibly unify the (const_int -1) into ONE register -- dropping the s:24 addiu and retargeting `bne $v1, $t0` (s:31) to $t1. If the sandbox instead stays 0, the locals are inert noise and get deleted per the diagnosis.

- [s1] [fable-blitz 2026-07-07] Naming-test counterpoint: `neg1`/`neg1b` name their VALUE (like SOTN's sanctioned `s16 three = 3;` constant-holder in [[named-local-fake-exception]], 2026-07-01), not coercion intent (pad/dummy/spill). If load-bearing, the construct family is SANCTIONED last-resort with FAKE annotation + exhaustion -- the diagnosis's 'requiring a clean redo' fork tensions with that rule.

- [s1] [fable-blitz 2026-07-07] History note per diagnosis: these originated as asm("t1")/asm("t0") pins and were converted to named locals in cleanup -- i.e. the named-local form ALREADY IS the pure-C replacement of a hard-forbidden construct; the open question is only annotation/renaming vs literal cleanup.
