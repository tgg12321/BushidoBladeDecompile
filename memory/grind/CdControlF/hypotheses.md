# CdControlF — hypothesis frontier

Floor history: **23 (s1 start) → 11 (s1 end)**. Structure is exact (75/75
instructions, same order); the residual is register assignment only.

## KILLED in session 1 (do not re-run)

- **H-s1-a: declaration order steers allocation.** KILLED. All 120 declaration
  permutations score identically (14) with the init order held fixed.
  `sweep2_results.json`.
- **H-s1-b: the inherited `new_var`/`new_var2` DImode chain is load-bearing.**
  KILLED. The 11-scoring candidate has no such chain.
- **H-s1-c: narrowing spellings (`(u8)a0`, `elem[0]`, byte-offset pointer math)
  move the allocation.** KILLED — all neutral at 14. `sweep3_results.json`.
- **H-s1-d: initialising `result` late helps.** KILLED, and it is actively
  harmful (24–31). `result` must be initialised first-or-early.

## CONFIRMED in session 1

- **H-s1-e: init-statement order is the allocation lever.** CONFIRMED — the
  same 5 statements span 14…31 by ordering alone.
- **H-s1-f: CdControlB's `base = g_cd_sector_buf; elem = base + idx;` two-step
  is the right address spelling.** CONFIRMED — fixes the v0/v1 scratch pair and
  drops 14 → 11.

## Live frontier (session 2+)

### F1 — swap the `result` / `elem` allocno order (the whole 11-point residual)
**Statement.** `result` is currently allocated before `elem` (s5 vs s6); target
has `elem`=s5, `result`=s6. Every one of the 11 remaining differing instructions
mentions one of those two values, so a single swap closes the function.
**Mechanism.** GCC 2.7.2 `global.c:635 allocno_compare` ranks by
`floor_log2(n_refs)*n_refs / live_length`. Our `result` scores 3/L, our `elem`
scores 2/L. The original's C must give `elem` the higher rank — a spelling with
one more genuine `elem` reference, or one that lengthens `result`'s live range
(live_length is the divisor, so a longer-lived `result` sinks below `elem`).
**Next probe.** Take a `cc1 -O2 ... -da` dump of the candidate
(`tmp/grind/CdControlF/s1/dump.sh` already builds `system.i`; add `-da` and read
the `.greg` file's `;; Register dispositions` + allocno priority list) and read
the ACTUAL n_refs / live_length pair for the two pseudos instead of the
hand-count above. Then sweep honest respellings that raise `elem`'s reference
count — e.g. walking the pointer (`if (*elem != 0)` becoming a form that reads
`elem` in both the guard and the CD_cw argument path), or hoisting the loop's
`g_cd_callback_a` stores in a way that extends `result`'s range across the loop.
No dead stores, no holders — this is a genuine-reference question.

### F2 — the PsyQ prototype question (`u_char com, u_char *param`)
**Statement.** The real Sony libcd signature is
`int CdControlF(u_char com, u_char *param)`; ours is `s32 CdControlF(s32, s32)`.
A pointer-typed `param` and a `u_char`-typed `com` change what CD_cw's argument
setup looks like and could change the liveness of the `a0` copy (s3), which in
turn changes the allocno ordering `result`/`elem` sits inside.
**Mechanism.** Parameter types drive the incoming-arg copy pseudos and their
`reg_n_refs`; the s3 raw-`a0` copy exists precisely because `a0 & 0xFF` is needed
twice. A `u_char` param would fold one of those.
**Next probe.** Re-type the params (and `CD_cw`'s declared prototype, which is
already `s32 CD_cw(s32, void *, void *, s32)`) to the PsyQ shape and re-run the
sweep-4 init-order sweep over it. Cross-check against sotn-decomp's
`src/main/psxsdk/libcd/sys.c`, which the file's existing comments already cite as
the C reference for CdSync/CdReady in this same TU.

### F3 — port the winning form to the sibling `CdControl`
**Statement.** `CdControl` (queue-active, distance 25, still carrying a
`register s32 result asm("s7")` pin) is the same function with a third argument.
Whatever closes CdControlF should close it.
**Mechanism.** Same allocno-ordering question, one extra live value.
**Next probe.** After CdControlF closes, replay sweep-1/sweep-4 against
CdControl with the winning spelling as the seed. (Out of scope for a CdControlF
session — recorded so the work is not re-derived.)

## [s1] The residual is not structural — target and build emit the same 75 instructions and differ only in which callee-saved register each value occupies.
- mechanism: Register allocation only: GCC 2.7.2 global.c assigns the same RTL to different hard regs; no insn count, branch, frame-size or jal-site difference exists.
- probe: objdump -d tmp/sandbox/CdControlF/system.o vs asm/funcs/CdControlF.s, instruction-by-instruction
- result: 75 vs 75 instructions, identical opcode sequence; every differing instruction differs only in an s-register name. Target: s0=count s1=a1 s2=idx s3=a0 s4=saved s5=elem s6=result; session-start build had result at s2 and everything else shifted up one.
- verdict: CONFIRMED

## [s1] Init-statement order steers the callee-saved allocation.
- mechanism: Allocno creation/priority order in global.c follows first-use order in the RTL, which follows initialiser statement order.
- probe: Full 60-permutation sweep of the five initialiser statements (constraint idx-before-elem), each scored with sandbox --disable all (tmp/grind/CdControlF/s1/sweep_results.json)
- result: Scores span 14..31. Eight orders reach 14, all with `result` initialised first-or-early and `elem` initialised last. Baseline order scored 23.
- verdict: CONFIRMED

## [s1] Declaration order steers the allocation too.
- mechanism: Proposed: declaration order biases pseudo numbering / LUID.
- probe: All 120 declaration permutations with the init order pinned to a 14-scoring winner (tmp/grind/CdControlF/s1/sweep2_results.json)
- result: Every one of the 120 scored exactly 14. Declaration order is completely inert for these scalars — GCC 2.7.2 creates the pseudo at first use, not at declaration.
- verdict: KILLED

## [s1] The matched sibling CdControlB's two-step address spelling (`base = g_cd_sector_buf; elem = base + idx;`) is the right form for elem.
- mechanism: The one-step `&g_cd_sector_buf[idx]` emits `sll v1,s2,2 / lui v0 / addiu v0 / addu elem,v1,v0`; target emits `lui v1 / addiu v1 / sll v0,s2,2 / addu elem,v0,v1`. The named base pointer materialises the symbol into v1 and the scaled index into v0, matching target's scratch-register pairing.
- probe: Variant sweep A_base_two_step then a 240-permutation init-order re-sweep over the resulting 6-local form (sweep3_results.json, sweep4_results.json)
- result: 14 -> 11. v0/v1 now match target exactly. Confirmed floor of the two-step form is 11 across all 240 legal init orders.
- verdict: CONFIRMED

## [s1] The inherited `int new_var = 3; unsigned long long new_var2 = new_var; count = new_var2;` DImode chain, and narrowing respellings ((u8)a0, elem[0], byte-offset pointer math), carry allocation signal.
- mechanism: Proposed: DImode widening / narrowing casts change the pseudo's mode and therefore its allocno size and rank.
- probe: Variants v2 (plain `count = 3;`), F_u8_param, C_elem_deref_local, D_byte_offset, each swept (sweep_results.json, sweep3_results.json)
- result: All neutral — the 11-scoring candidate reaches its floor with a plain `count = 3;` and no casts. The DImode chain is not load-bearing and is deleted from the candidate.
- verdict: KILLED

## [s1] The `register s32 result asm("s6")` pin at HEAD was buying score.
- mechanism: The sandbox strips cheat-asm before scoring, so a pin is score-inert by construction.
- probe: Pin-free variant v1 (identical body otherwise) scored against the pinned HEAD form
- result: Both score 23. The pin bought nothing; the candidate deletes it, removing a cheat-asm construct from src/system.c.
- verdict: KILLED
