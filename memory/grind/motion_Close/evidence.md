# Evidence bank — motion_Close

## Session 1 (recon, 2026-08-11)

### Baseline
- `canonical motion_Close` → verdict **C**, asm_insns 0, total 26, distance 21
  ("pure-C distance 21 <= 50 — pure-C target").
- `sandbox motion_Close --disable all` at HEAD → **score 21**, target_insns 26,
  **build_insns 13**, rules_dropped 9, cheat_asm_stripped 18.
- `diagnose` → LARGE, "21 differing insn(s) — deep restructure".
- Nine regfix rules at `regfix.txt:115-124`: a `$2 <-> $8` register swap over
  insns 0-4, an `insert "nop" @ 6`, a `reorder 2,3,5,4 @ 2-5`, and six `subst`
  rules that rewrite save/restore offsets (`$31,8`→`$16,4`; `,4(`→`,8(`;
  `$16,0(`→`$31,0xc(`; `,8(`→`,0xc(`; `,4(`→`,8(`; `,0(`→`,4(`).
  Every one of them is a frame-layout or register-name rewrite. There is no
  rule that adds, removes, or reorders a computation.

### The HEAD baseline of 21 was not a measurement of any C form
The committed body used `register void (**p)(void) asm("s0")`, `register s32
count asm("s1")`, `register void (*f)(void) asm("t0")`, plus
`__asm__ volatile("jalr %0" :: "r"(f) : "ra","memory")` and a hardcoded
`__asm__ volatile("addiu $17, $17, -1" : "=r"(count) : "0"(count))`. The
cheat-stripper removes the two `__asm__` blocks, which ARE the loop body, so
the stripped function collapses to 13 instructions with no call and no loop
work. **The 21 was an artifact of a degenerate stripped function.** Before this
session, no honest pure-C form of motion_Close had ever been measured.

### The target, decoded
`normalized_insns(build/src/ings2.o, "motion_Close")` (the oracle-matching
reference) is a textbook "walk a linker-provided table of function pointers"
loop:

```
 0 lui t0        1 lw t0,0(t0)        <- D_800A2668 guard, hoisted above the frame
 2 addiu sp,sp,-16
 3 sw s0,4(sp)   4 sw s1,8(sp)   5 sw ra,12(sp)
 6 beqz t0,end   7 nop
 8 lui s0        9 addiu s0        <- p     = &D_8008D070
10 lui s1       11 addiu s1        <- count = (s32)&D_00000000
12 beqz s1,end  13 nop
14 lw t0,0(s0)  15 addiu s0,s0,4   <- f = *p; p++
16 jalr t0      17 addiu s1,s1,-1  <- f(); count-- in the call delay slot
18 bnez s1,loop 19 nop
20 lw ra,12  21 lw s1,8  22 lw s0,4  23 addiu sp,sp,16  24 jr  25 nop
```

### The honest pure-C form reproduces the target STRUCTURE exactly
Body measured this session (banked as `candidate.c`):
`void (**p)(void); s32 count; if (D_800A2668 != 0) { p = &D_8008D070;
count = (s32)&D_00000000; if (count != 0) { do { void (*f)(void) = *p; p++;
f(); count--; } while (count != 0); } }`

→ **score 20, build_insns 25** (vs target 26). The CFG, the operation sequence,
the `count--` scheduled into the `jalr` delay slot, and the loop shape are all
identical. The entire 20-instruction "difference" is register names and stack
offsets — there is no missing or extra computation anywhere, and the single
instruction-count delta is the one unfilled delay slot described below.

### H1 (CONFIRMED KILL) — the target frame layout is unreachable from pure C
GCC 2.7.2's MIPS frame layout is `[outgoing args][vars][saved regs]`, with the
highest save (`ra`) at `args + vars + gp_size - 4`. Formula validated against
this session's own build, whose C is known to have zero stack locals:
args 16 + vars 0 + gp_size 12 - 4 = **24**, and the build emits `sw ra,24(sp)`. ✓

Applying it to the target: `ra@12`, gp_size 12 (s0, s1, ra) ⇒ **args + vars = 4**,
i.e. an outgoing-argument area of **ZERO**. But gcc-2.7.2's o32 backend defines
`REG_PARM_STACK_SPACE` = 16, so *every* C-level call forces at least 16 bytes of
outgoing-arg area. Minimum pure-C frame for this function is therefore
16 + 12 = 28 → aligned **32**, which is exactly what the honest form emits.
The target's 16 is not reachable.

Corroborated by census (`tmp/grind/motion_Close/s1/census.py`) over **all 854
call-making functions in the oracle-matching `build/src/*.o`**:

    implied (args+vars) distribution:
      4 -> 2 | 16 -> 538 | 20 -> 1 | 24 -> 101 | 32 -> 30 | 40 -> 21 | ...

**Zero** functions have a sub-16 arg area — except the two at 4, which are
`motion_Close` and its sibling `func_80083794` (motion_Open), in the same file.
Both reach it *only* because their `jalr` is inline asm: an `__asm__` block never
sets `current_function_outgoing_args_size`, so no arg area is reserved. In other
words the frame shape the target has is precisely the shape that the cheat
produces and that compiled C provably cannot. This axis is CLOSED — no future
session should spend time trying to shrink the frame from C.

### H2 (KILLED) — declaration order does not move the s0/s1 role assignment
Target: s0 = p, s1 = count. Honest C: s1 = p, s0 = count (roles swapped), and
the scalar temp is v0 where the target uses t0 (both for the guard load and for
the `jalr` target register). Both forms emit the two address materializations in
source order (p first), so emission order is not the discriminator. Probe:
declare `count` before `p`. Result: **score 20, build_insns 25 — unchanged, no
register moved.** Banked as `rejected/decl-order-swap-no-register-effect.c`.

### Two independent hand-written-asm signals (for a future escalation packet)
- **S-a — no outgoing-arg area despite making a call.** Unique in the corpus
  (2 of 854, and both are this function and its sibling). See H1.
- **S-b — an unfilled branch delay slot that GCC would have filled.** The target
  has `beqz t0 / nop` at insns 6-7 with all three register saves already placed
  above it. The honest C form places only two saves, then `beqz v0` with
  `sw s0,16(sp)` **in the delay slot** — GCC's `reorg.c` fills that slot because
  a pending save is available. The target leaves a `nop` there instead, which is
  what `regfix.txt:117` (`insert "nop" @ 6`) exists to re-create. A compiler does
  not waste that slot; a human hand-writing the prologue does.
- Neighborhood corroboration: `motion_Close` sits among `_start`, `InitHeap`
  (a hand-written `addiu $t2,$zero,0xA0 / jr $t2` BIOS trampoline),
  `func_800836B8` (contains a raw `.word 0x0000410D` syscall), `func_800836C8`
  and `bios_FileRead` — i.e. the PsyQ/BIOS crt0 runtime region. The body is the
  classic crt0 constructor/destructor-table walk, and `motion_Open`
  (`func_80083794`) is its once-only-guarded twin.
- NOTE: `tools/scan_hand_coded.py --single motion_Close` could **not** run —
  it requires `asm/funcs/motion_Close.s`, which does not exist because the
  function is C-routed. An escalation session must extract that file first to
  obtain a signal tier.

### Artifacts
- `tmp/grind/motion_Close/s1/dump.py` — side-by-side normalized-instruction dump
  of `build/src/ings2.o` (target) vs `tmp/sandbox/motion_Close/ings2.o` (build).
- `tmp/grind/motion_Close/s1/census.py` — the 854-function outgoing-arg-area
  census that established H1.

### Tree state at end of session 1
`src/ings2.c` was reverted to HEAD. The honest form is preserved in
`candidate.c` only, so the oracle and the `build/src/ings2.o` reference are
left intact for the next session.

- [s1] canonical motion_Close -> verdict C, asm_insns 0, total 26, distance 21, reason 'pure-C distance 21 <= 50'. The gate routed this function C on SIZE alone, having never seen an honest C measurement.

- [s1] sandbox --disable all at HEAD: score 21, target_insns 26, build_insns 13, rules_dropped 9, cheat_asm_stripped 18. build_insns 13 proves the stripped HEAD function is degenerate (no call, no loop).

- [s1] sandbox --disable all with the honest pure-C body: score 20, build_insns 25. New floor 20.

- [s1] Target decoded (normalized_insns of build/src/ings2.o): lui/lw t0<-D_800A2668; addiu sp,-16; sw s0,4/s1,8/ra,12; beqz t0; nop; lui/addiu s0<-&D_8008D070; lui/addiu s1<-&D_00000000; beqz s1; nop; loop: lw t0,0(s0); addiu s0,s0,4; jalr t0; addiu s1,s1,-1; bnez s1; nop; lw ra,12/s1,8/s0,4; addiu sp,16; jr; nop.

- [s1] All nine regfix rules (regfix.txt:115-124) are frame-layout or register-name rewrites only: a $2<->$8 swap over insns 0-4, an `insert "nop" @ 6`, a `reorder 2,3,5,4 @ 2-5`, and six `subst` rules rewriting save/restore offsets. Not one rule adds, removes, or reorders a computation — consistent with a body whose logic is exactly right and whose frame/registers are not.

- [s1] GCC frame-layout formula validated against a known-C build before being applied to the target: honest form has args 16 + vars 0 + gp_size 12 - 4 = 24 and emits `sw ra,24(sp)`.

- [s1] HAND-ASM SIGNAL S-a: no outgoing-arg area despite making a call — 2 of 854 functions in the corpus, and both are this function and its sibling, both only via inline asm. Impossible from compiled C.

- [s1] HAND-ASM SIGNAL S-b: the target's `beqz t0` at insn 6 has an UNFILLED delay slot (nop at 7) with all three register saves already placed above it. The honest C form places two saves then puts `sw s0,16(sp)` IN that delay slot — GCC's reorg.c fills the slot when a pending save is available. regfix.txt:117 (`insert "nop" @ 6`) exists solely to re-create the human's wasted slot.

- [s1] NEIGHBORHOOD: motion_Close sits among _start, InitHeap (a hand-written `addiu $t2,$zero,0xA0 / jr $t2` BIOS trampoline), func_800836B8 (raw `.word 0x0000410D` syscall), func_800836C8 and bios_FileRead — the PsyQ/BIOS crt0 runtime region. Its body is the classic constructor/destructor-table walk and func_80083794 (motion_Open) is its once-only-guarded twin.

- [s1] tools/scan_hand_coded.py --single motion_Close could NOT run: it requires asm/funcs/motion_Close.s, which does not exist because the function is C-routed. A future escalation session must extract that file first to obtain a signal tier — the canonical-asm gate needs a measured STRONG tier (S1/S2/S6), not an asserted one.

## Session 2 (structural, 2026-08-11)

### New floor: 20 -> 17, from statement order alone
The ONLY change against session 1's form is that `count` is assigned before
`p` inside the guard (and declared first). GCC emits the two address
materializations in source order, so assigning `count` first places the `$s0`
lui/addiu pair at insns 7-8 and the `$s1` pair at 9-10 — which is the
target's register ORDER at those four slots (target: `$s0` pair, then `$s1`
pair). Worth 3 points. Note this does NOT contradict s1's H2: declaration
order alone is inert (re-confirmed); it is the BODY assignment order that
moves the emission order.

### 20 structural spellings measured; only statement order moved anything
Sweeps `tmp/grind/motion_Close/s2/sweep.py` (13 forms) and `sweep2.py`
(14 forms). Everything below scored **17** and emitted the identical 25
instructions once the count-first statement order was applied: `*p++`
post-increment read, plain `while` instead of `if`+`do-while`, `--count` in
the loop condition, `for (; count; count--)`, `(*p)()` with no `f` temp,
`p[0]()` + `p = &p[1]`, `f` declared outside the loop, block-local
declarations with initialisers, an early-`return` guard chain, a hoisted
guard local, and `u32` instead of `s32` for count. Scoring **19** (worse):
hoisting the `count` initialiser above the `D_800A2668` guard, decrementing
count before the call, and an index-style walk. Scoring **21/27 insns**: the
end-pointer loop. Conclusion: the loop's C spelling is essentially a free
variable — the residual does not live in it.

### H3 (CONFIRMED KILL) — the target's $s0/$s1 role split is unreachable from C
Measured directly with the instrumented cc1 (`tools/gcc-2.7.2/cc1`,
`BB2_ALLOC_DEBUG=1`; log `tmp/grind/motion_Close/s2/allocsweep.log`). GCC
2.7.2's `global.c` sorts allocnos by
`floor_log2(n_refs) * n_refs / live_length * 10000` (`allocno_compare`,
global.c:642-655) and assigns hard registers in that order, so the
higher-priority allocno takes `$s0`. For every spelling that emits the
target's instruction sequence:

    count (pseudo 72): n_refs=8  live_length=8  pri=30000  -> $s0 ($16)
    p     (pseudo 73): n_refs=7  live_length=7  pri=20000  -> $s1 ($17)

`count` is structurally one reference richer than `p` — init, outer guard
test, in-loop decrement (set+use), loop test, versus init, in-loop load,
in-loop bump — and the target's own instruction sequence contains all of the
same references, so a hypothetical matching C source would have the same
counts. **The target has `p` in `$s0`: the inverse of what global.c produces.**
The `floor_log2` step at 8 makes this a wide margin (3*8 vs 2*7), not a
tie-break: it is not reachable by nudging.

Only two spellings out of the whole sweep flipped the roles, and both emit
instructions the target does not contain, so neither can match:
- **end-pointer loop** — p n_refs=11 pri=36666 takes `$s0`; costs the `end`
  materialisation + add (27 insns vs target 26), score 21. Banked as
  `rejected/end-pointer-loop-costs-two-insns.c`. This is the honest,
  human-plausible spelling, and it is the one that proves the bind: the form
  that fixes the registers necessarily breaks the instruction sequence.
- **guard laundered through p** (`if (p != &D_8008D070 + count)`) — p
  n_refs=9 pri=30000 takes `$s0`; emits `sll/addu/beq` (27 insns), score 17.
  Also a cheat on T1/T2/T3 (the predicate is exactly `count != 0`; its only
  purpose is to add a reference so global.c's priority tips). Not proposed;
  banked as `rejected/role-flip-guard-laundering.c`.

Any *byte-free* way to give `p` an eighth reference is by definition a
construct with no emitted effect whose sole function is to move GCC's
allocator — a forbidden coercion. So this axis is CLOSED on both the
measurement and the policy side.

### F2 closed: the 17-point residual attributes exhaustively
Block-level attribution of the session-2 form against the target, summing to
exactly the measured score of 17:

    guard-load temp ($t0 vs $v0), 2 insns            2
    frame size (addiu sp,-16 vs -32)                 1
    register-save block (offsets + order)            3
    beqz delay slot (target nop vs GCC's sw s0)      2
    inner guard register (beqz s1 vs beqz s0)        1
    loop body registers (lw/addiu/addiu/bnez)        4
    epilogue restore offsets + sp adjust             4
                                                    --
                                                    17

Every one of the seven buckets is (a) the frame — dead by H1, (b) the
register roles — dead by H3, or (c) the wasted delay slot — hand-asm signal
S-b. There is no unexplained instruction, and no bucket is a live grind
axis. The nine regfix rules at regfix.txt:115-124 are a complete description
of the gap.

### Session-2 artifacts
- `tmp/grind/motion_Close/s2/sweep.py` + `sweep.log` — 13-form structural sweep (found the 20->17 statement-order lever).
- `tmp/grind/motion_Close/s2/sweep2.py` + `sweep2.log` — 14-form round-2 sweep on top of the new floor.
- `tmp/grind/motion_Close/s2/allocsweep.py` + `allocsweep.log` — instrumented-cc1 allocno priority table per spelling (the H3 kill).
- `tmp/grind/motion_Close/s2/greg.sh` + `ings2.i.greg` — RTL global-alloc dump confirming pseudo 72=count -> $16, pseudo 73=p -> $17.

### Tree state at end of session 2
`src/ings2.c` reverted to HEAD (session-1 precedent), so `build/src/ings2.o`
stays valid as the target reference. The 17-scoring form lives only in
`memory/grind/motion_Close/candidate.c`.

- [s2] New honest floor 17 (from 20): assigning `count` before `p` inside the guard aligns the two address materializations with the target's $s0-then-$s1 emission order. Declaration order alone remains inert (s1 H2 re-confirmed) — it is the body statement order that matters.

- [s2] 20 structural spellings measured (sweep.py + sweep2.py): *p++, plain while, --count in the condition, for-loop, (*p)() with no temp, p[0]()+p=&p[1], f hoisted out of the loop, block-local initialised decls, early-return guards, hoisted guard local, u32 count — ALL score 17 with byte-identical output. Worse: count hoisted above the D_800A2668 guard (19), decrement before the call (19), index walk (19), end-pointer loop (21/27 insns). The loop's C spelling is a free variable; the residual is not in it.

- [s2] H3 KILLED with the instrumented cc1 (BB2_ALLOC_DEBUG=1): global.c ranks allocnos by floor_log2(n_refs)*n_refs/live_length; count measures n_refs=8 live_length=8 pri=30000 and takes $s0, p measures n_refs=7 live_length=7 pri=20000 and takes $s1, in every spelling that emits the target's instruction sequence. The target has p in $s0 — the inverse. The floor_log2 step at 8 makes the margin wide (3*8 vs 2*7), so it is not a tie-break to be nudged.

- [s2] The only two spellings that flip the roles both emit instructions absent from the target: the end-pointer loop (p pri=36666, 27 insns, score 21) and a guard laundered through p (p pri=30000, 27 insns, score 17, and a T1/T2/T3 cheat). Any byte-free eighth reference for p would be a pure allocator-coercion construct — forbidden. Axis closed on measurement AND policy.

- [s2] HAND-ASM SIGNAL S-c: the target's register ASSIGNMENT is not producible by global.c from any C source carrying the target's instruction sequence (H3). This is independent of S-a (zero outgoing-arg area, H1) and S-b (unfilled delay slot). Three independent impossibility signals now stand.

- [s2] F2 CLOSED: the 17-point residual attributes exhaustively to 7 blocks summing to exactly 17 — guard-load temp 2, frame size 1, save block 3, beqz delay slot 2, inner guard register 1, loop-body registers 4, epilogue offsets 4. Every bucket is dead-by-H1, dead-by-H3, or hand-asm signal S-b. No unexplained instruction remains.

- [s1] Tree state: src/ings2.c reverted to HEAD at end of session (git status shows only metrics/events.jsonl and the untracked ledger dir), so the oracle and the build/src/ings2.o reference are left intact. The honest form lives in memory/grind/motion_Close/candidate.c.

- [s2] New honest floor 17 (was 20), measured this session with `sandbox motion_Close --disable all` and the form banked in memory/grind/motion_Close/candidate.c. build_insns 25 vs target 26.

- [s2] The 20->17 lever is BODY STATEMENT ORDER, not declaration order: assigning `count` before `p` inside the guard. Session 1's H2 (declaration order alone) is re-confirmed inert — the two probes are different and both results stand.

- [s2] 27 structural spellings measured across two sweeps; 11 of them are byte-identical to the best form at 17, 3 score 19, 1 scores 21. The loop's C spelling is effectively a free variable — the residual does not live in it.

- [s2] Instrumented-cc1 measurement (BB2_ALLOC_DEBUG=1): motion_Close's two callee-saved allocnos are count (n_refs=8, live_length=8, pri=30000, gets $16) and p (n_refs=7, live_length=7, pri=20000, gets $17), invariant across every spelling that emits the target's instruction sequence. GCC 2.7.2 global.c:642-655 is the ranking function.

- [s2] The target's register assignment (p in $s0) is therefore not producible by global.c from ANY C source carrying the target's instruction sequence — a third independent hand-written-asm signal (S-c), alongside S-a (zero outgoing-arg area despite making a call, unique in an 854-function corpus) and S-b (an unfilled beqz delay slot that reorg.c demonstrably fills).

- [s2] The two role-flipping spellings are banked as rejected forms: rejected/end-pointer-loop-costs-two-insns.c (honest, human-plausible, but materialises an end pointer the target never forms — 27 insns) and rejected/role-flip-guard-laundering.c (emits sll/addu/beq absent from the target AND fails cheat tests T1/T2/T3; recorded so no future session re-derives it).

- [s2] The bind is now explicit: the C form that fixes the registers necessarily breaks the instruction sequence, and the C form that keeps the instruction sequence necessarily loses the registers.

- [s2] src/ings2.c was reverted to HEAD at end of session (session-1 precedent), so build/src/ings2.o remains a valid target reference; the 17-scoring form lives only in memory/grind/motion_Close/candidate.c.

## Session 3 (structural, 2026-08-11)

### THE HEADLINE: H3 is FALSIFIED — the $s0 = p / $s1 = count role split IS reachable from C

Session 2 killed H3 on the strength of a real measurement (for every spelling
emitting the target's instruction sequence, `count` has n_refs=8/live_length=8
-> pri 30000 and takes `$s0`, while `p` has 7/7 -> pri 20000 and takes `$s1`)
plus the inference that a 30000-vs-20000 margin is "wide, not a tie-break to be
nudged". The measurement is correct and reproduced this session. The inference
is wrong, because it stopped reading `allocno_compare` one line early
(`tools/gcc-2.7.2/global.c:633-654`):

```
  if (pri2 - pri1)
    return pri2 - pri1;

  /* If regs are equally good, sort by allocno,
     so that the results of qsort leave nothing to chance.  */
  return *v1 - *v2;
```

On an EXACT priority tie the winner is the LOWER ALLOCNO NUMBER, which follows
pseudo-register creation order, which for block locals follows DECLARATION
order. Session 1's H2 ("declaration order is inert") was measured when there
was no tie to break — with a 30000/20000 gap the tie-break code never runs.
Create the tie and declaration order becomes decisive.

The tie is created by carrying the `D_800A2668` guard value in `p` before `p`
is loaded with the table address: that lifts p to n_refs=9, live_length=9,
`floor_log2(9)*9/9*10000` = 30000 — exactly count's priority. The full 2x2,
each cell measured with sandbox AND with the instrumented cc1
(`BB2_ALLOC_DEBUG=1`, `tmp/grind/motion_Close/s3/tiebreak.py`):

    cell                       p refs/len   count refs/len   $s0    score
    count declared 1st, no guard carry   7/7  30000->count   8/8    count  17
    p declared 1st,     no guard carry   7/7  20000          8/8    count  17
    count declared 1st, guard in p       9/9  30000 (tie)    8/8    count  17
    p declared 1st,     guard in p       9/9  30000 (tie)    8/8    P      16

Both halves are load-bearing: the tie alone does nothing, the declaration order
alone does nothing, together they flip the roles. **New honest floor: 16.**

### What the flip buys and what it costs (the 16-point form, banked as candidate.c)

The loop body now carries the target's registers exactly — `lw` from `$s0`,
`addiu $s0,$s0,4`, `addiu $s1,$s1,-1`, `bnez $s1` — and the inner guard branches
on `$s1` as the target does. But `p` is callee-saved, so making it hold the
guard drags the guard LOAD into `$s0`, which must then be saved before the load.
The target loads the guard into `$t0` (caller-saved) before the frame exists.
So the prologue gets worse by nearly what the loop body gained:

     0 addiu sp,sp,-32      | target:  0 lui  t0
     1 sw   s0,16(sp)       |          1 lw   t0,0(t0)
     2 lui  s0              |          2 addiu sp,sp,-16
     3 lw   s0,0(s0)        |          3 sw s0,4 / 4 sw s1,8 / 5 sw ra,12
     4 sw   ra,24(sp)       |          6 beqz t0 ; 7 nop
     5 beqz s0 ; 6 sw s1,20 |          8 lui/9 addiu s0   (= p)
     7 lui/8 addiu s1       |         10 lui/11 addiu s1  (= count)
     9 lui/10 addiu s0      |         12 beqz s1 ; 13 nop
    11 beqz s1 ; 12 nop     |         14 lw t0,0(s0) ; 15 addiu s0,s0,4
    13 lw v0,0(s0) ; 14 addiu s0,s0,4 | 16 jalr t0 ; 17 addiu s1,s1,-1
    15 jalr ; 16 addiu s1,s1,-1       | 18 bnez s1 ; 19 nop
    17 bnez s1 ; 18 nop               | 20-25 epilogue (offsets 12/8/4, sp+16)
    19-24 epilogue (24/20/16, sp+32)

Net 17 -> 16.

### The new bind (this is the session-4 frontier)

With the roles flipped, the two address materialisations are emitted
count-pair-then-p-pair while the target emits p-pair-then-count-pair. Moving
`p = &D_8008D070;` above `count = ...` fixes the ORDER — and destroys the
PRIORITY TIE that fixes the ROLES: count is born one statement later, its live
length falls 8 -> 7 and its priority RISES 30000 -> 34285, while p's live length
grows 9 -> 10 and its priority FALLS 30000 -> 27000. count takes `$s0` back and
the score returns to 20. Measured twice (t5_pdecl_pguard_ptrorder,
w5_pfirst_count_inside); banked as
`rejected/pfirst-body-order-returns-s0-to-count.c`.

### The dual approach — lowering count instead of raising p — is measured dead

Staging the outer guard test through a separate short-lived local removes
count's outer-test reference (8 -> 7 refs) but shortens its live length 8 -> 6 at
the same time, so its priority goes UP (30000 -> 23333) while p's goes DOWN
(20000 -> 15555, live length stretched to 9 by the extra local). Score 22.
Measured in all three declaration/body-order arrangements — identical table,
identical score, so the tie-break never engages. Banked as
`rejected/staged-guard-local-shortens-count-liverange.c`. **General lesson:
references and live length shrink together, so you cannot lower an allocno's
priority by deleting references; the only workable direction on this function is
adding references to the allocno you want to win.**

### 18 further structural spellings measured; the loop body remains a free variable

`sweep3.py` (12 forms, all disjoint from the s1/s2 banked list): guard carried in
count / in p / in a hoisted `f`, p assigned inside the inner guard, count as s16,
count as u8, combined `*p++` with `--count`, guard folded into an `&&` condition
chain, `f` hoisted and re-read at the loop tail, `while` with the decrement before
the call, nested block declarations with initialisers, and a `u32 *` walk with a
cast at the call. All scored 17 or worse (s16 23, u8 21, p-inside-inner 20,
&&-chain 20, f-hoisted 20, decrement-before-call 19). `ordersweep.py` (6 forms on
top of the role-flipped body): post-increment + pre-decrement, `f` hoisted out of
the loop, early-return guard, a scalar-cast guard test, and a plain `while` are
ALL byte-identical at 16. The loop's C spelling is confirmed a free variable in
both families.

### RETRACTION: hand-asm signal S-c is withdrawn

Session 2 recorded S-c — "the target's register ASSIGNMENT is not producible by
global.c from any C source carrying the target's instruction sequence" — as a
third independent hand-written-asm signal. It is now measured false: this
session produced the target's assignment from C, with the target's instruction
sequence intact in the loop body. **Any future escalation packet for this
function must cite only S-a (zero outgoing-arg area, H1) and S-b (the unfilled
beqz delay slot), and must not repeat S-c.** S-a and S-b are untouched by this
session and still stand.

### OPEN POLICY QUESTION (raised, not self-answered)

The 16-point form contains
`p = (void (**)(void))D_800A2668; if (p != 0) { ... p = &D_8008D070; ... }`.
"Variable reuse for codegen control" is on the FROZEN SOTN-sanctioned list
(`.claude/rules/no-new-park-categories.md`, §SOTN-accepted techniques), and the
guard test itself is a real, semantically required test the target performs.
Against that: holding an s32 flag in a function-pointer-pointer has no
observable effect (T1), no human would write the cast (T2), and the whole
mechanism is stated in terms of global.c's allocno tie-break (T3). The form
carries no `/* FAKE */` annotation and no lever-exhaustion citation. It was NOT
proposed as a candidate and MUST NOT be submitted without a ruling — and in any
case H1 means no form in this family can reach distance 0. Its value is
evidentiary: it proves the register axis is open.

### Session-3 artifacts
- `tmp/grind/motion_Close/s3/sweep3.py` + `sweep3.log` — 12-form structural sweep.
- `tmp/grind/motion_Close/s3/allocsweep3.py` + `allocsweep3.log` — instrumented-cc1 allocno tables for the variable-reuse forms (found the priority TIE).
- `tmp/grind/motion_Close/s3/tiebreak.py` + `tiebreak.log` — the decisive 2x2: declaration order x guard-carry, score + allocno table per cell (floor 17 -> 16).
- `tmp/grind/motion_Close/s3/rolesweep.py` + `rolesweep.log` — the dual approach (lower count instead of raising p), measured dead at 22.
- `tmp/grind/motion_Close/s3/ordersweep.py` + `ordersweep.log` — 6 body-order / loop-shape forms on top of the role-flipped body.

### Tree state at end of session 3
`src/ings2.c` reverted to HEAD (every sweep restores it in a `finally:` block),
so `build/src/ings2.o` remains a valid target reference. The 16-scoring form
lives in `memory/grind/motion_Close/candidate.c`; the policy-clean 17-scoring
form is preserved beside it as `candidate_policy_clean_17.c`.

- [s3] H3 FALSIFIED. global.c's allocno_compare (tools/gcc-2.7.2/global.c:633-654) falls through to `return *v1 - *v2;` — the ALLOCNO NUMBER, i.e. pseudo creation order, i.e. DECLARATION order — whenever two allocnos have EQUAL priority. Session 2 read only the priority formula and concluded a 30000-vs-20000 margin was unnudgeable; session 1 measured declaration order as inert, but only in the no-tie regime where the tie-break code never executes.

- [s3] Carrying the D_800A2668 guard value in `p` lifts p to n_refs=9 / live_length=9 / pri=30000 — an EXACT tie with count's 8/8/30000 — while emitting byte-identical output (17, 25 insns) when count is declared first. Declaring `p` first then flips the tie-break and p takes $s0. New honest floor 16.

- [s3] The 2x2 is fully measured and both halves are load-bearing: {count declared first, no guard carry} 17; {p declared first, no guard carry} 17; {count declared first, guard in p} 17 (tie, count wins on allocno); {p declared first, guard in p} 16 (tie, p wins on allocno).

- [s3] In the 16-point form the loop body registers match the target exactly (lw from $s0, addiu $s0,$s0,4, addiu $s1,$s1,-1, bnez $s1) and the inner guard branches on $s1. The cost is that p is callee-saved, so the guard load itself lands in $s0 and forces `sw s0` above it plus `sw s1` into the beqz delay slot; the target loads the guard into $t0 before the frame exists.

- [s3] NEW BIND (session-4 frontier): with the roles flipped, the target's emission order wants the p pair FIRST, but assigning p before count shortens count's live length 8->7 (pri 30000->34285) and stretches p's 9->10 (pri 30000->27000), destroying the tie and returning $s0 to count (score 20). Order and roles are coupled through live_length.

- [s3] The dual lever — lowering count's priority instead of raising p's — is measured dead: staging the outer guard test through a separate local drops count to 7 refs but also drops its live length to 6, so its priority RISES to 23333 while p's falls to 15555 (score 22, identical in all three decl/body-order arrangements). General rule: refs and live length shrink together, so priority cannot be lowered by deleting references.

- [s3] 18 further structural spellings measured (sweep3.py 12, ordersweep.py 6). In the role-flipped family, post-increment+pre-decrement, `f` hoisted out of the loop, an early-return guard, a scalar-cast guard test and a plain `while` are ALL byte-identical at 16 — the loop's C spelling is a free variable in this family too. Worse in the s2 family: count as s16 (23), count as u8 (21), p assigned inside the inner guard (20), guard folded into an && chain (20), f hoisted and re-read at the loop tail (20), decrement before the call (19).

- [s3] HAND-ASM SIGNAL S-c IS RETRACTED. Session 2 claimed the target's register assignment was not producible by global.c from any C carrying the target's instruction sequence; session 3 produced it. Escalation packets must cite S-a and S-b only. S-a (zero outgoing-arg area despite a call, 2 of 854 and both inline-asm) and S-b (the unfilled beqz delay slot reorg.c demonstrably fills) are untouched and still stand.

- [s3] H1 is untouched and still caps this function: REG_PARM_STACK_SPACE=16 forces frame >= 32 for any pure-C body containing a call, versus the target's 16. No form in the role-flipped family can reach distance 0, so the 16-point form was NOT proposed as a candidate.

- [s3] OPEN POLICY QUESTION recorded, not self-answered: is `p = (void (**)(void))D_800A2668; if (p != 0)` inside the frozen "variable reuse for codegen control" family, or is it a T1/T2/T3 coercion? It has no /* FAKE */ annotation and no lever-exhaustion citation, and it was not submitted. A future session must obtain a ruling before building on it.

- [s3] [s3] H3 FALSIFIED. tools/gcc-2.7.2/global.c:633-654 (allocno_compare) falls through to `return *v1 - *v2;` - the ALLOCNO NUMBER, i.e. pseudo creation order, i.e. DECLARATION order - whenever two allocnos have EQUAL priority. Session 2 read only the priority formula and called a 30000-vs-20000 margin unnudgeable; session 1 measured declaration order as inert, but only in the no-tie regime where that code never runs.

- [s3] [s3] Carrying the D_800A2668 guard value in `p` lifts p to n_refs=9 / live_length=9 / pri=30000 - an EXACT tie with count's 8/8/30000 - while emitting byte-identical output (score 17, 25 insns) when count is still declared first. Declaring `p` first then flips the tie-break and p takes $s0. New honest floor 16 (was 17).

- [s3] [s3] The 2x2 is fully measured and both halves are load-bearing: {count declared first, no guard carry} 17; {p declared first, no guard carry} 17; {count declared first, guard in p} 17 (tie, count wins on the lower allocno); {p declared first, guard in p} 16 (tie, p wins on the lower allocno).

- [s3] [s3] In the 16-point form the loop-body registers match the target exactly - lw from $s0, addiu $s0,$s0,4, addiu $s1,$s1,-1, bnez $s1 - and the inner guard branches on $s1. The cost is that p is callee-saved, so the guard LOAD itself lands in $s0 and forces `sw s0,16(sp)` above it plus `sw s1,20(sp)` into the beqz delay slot; the target loads the guard into $t0 (caller-saved) before the frame exists.

- [s3] [s3] NEW BIND (the session-4 frontier): with the roles flipped the target's emission order wants the p pair FIRST, but assigning p before count shortens count's live_length 8->7 (pri 30000->34285) and stretches p's 9->10 (pri 30000->27000), destroying the tie and returning $s0 to count (score 20). Emission order and register roles are coupled through live_length.

- [s3] [s3] The dual lever - lowering count's priority instead of raising p's - is measured dead: staging the outer guard test through a separate local drops count to 7 refs but also drops its live_length to 6, so its priority RISES to 23333 while p's falls to 15555 (score 22, identical in all three decl/body-order arrangements). General rule: refs and live_length shrink together, so priority cannot be lowered by deleting references.

- [s3] [s3] 18 further structural spellings measured (sweep3.py 12, ordersweep.py 6), all disjoint from the 27 banked in s1/s2. In the role-flipped family five distinct loop spellings are byte-identical at 16; in the s2 family everything was 17 or worse (s16 23, u8 21, p-inside-inner 20, &&-chain 20, f-hoisted-and-re-read 20, decrement-before-call 19). The loop's C spelling is a free variable in both families.

- [s3] [s3] HAND-ASM SIGNAL S-c IS RETRACTED. Session 2 recorded, as a third independent impossibility signal, that the target's register ASSIGNMENT is not producible by global.c from any C carrying the target's instruction sequence. Session 3 produced it. Future escalation packets for motion_Close must cite S-a and S-b ONLY and must not repeat S-c.

- [s3] [s3] S-a (zero outgoing-arg area despite making a call - 2 of 854 call-making functions in the oracle build, and both are this function and its inline-asm sibling) and S-b (the unfilled beqz delay slot that reorg.c demonstrably fills from a pending save) are untouched by this session and still stand.

- [s3] [s3] H1 is untouched and still caps the function: gcc-2.7.2's o32 REG_PARM_STACK_SPACE=16 forces frame >= 16+12 = 28 -> 32 for any pure-C body containing a call, versus the target's 16-byte frame with a zero arg area. No form in the role-flipped family can reach distance 0, so the 16-point form was deliberately NOT proposed as a candidate.

- [s3] [s3] OPEN POLICY QUESTION recorded and NOT self-answered: the 16-point form contains `p = (void (**)(void))D_800A2668; if (p != 0) { ... p = &D_8008D070; ... }`. 'Variable reuse for codegen control' is on the frozen SOTN-sanctioned list (.claude/rules/no-new-park-categories.md, SOTN-accepted techniques) and the guard test is a real, semantically required test the target performs; against that, holding an s32 flag in a function-pointer-pointer has no observable effect (T1), no human would write the cast (T2), and the mechanism is stated purely in terms of global.c's allocno tie-break (T3). No /* FAKE */ annotation, no lever-exhaustion citation, not submitted.

- [s3] [s3] Tree state: src/ings2.c is reverted to HEAD (every sweep restores it in a finally: block), so build/src/ings2.o remains a valid target reference. The 16-scoring form is banked in memory/grind/motion_Close/candidate.c with a loud not-policy-vetted header; the best policy-clean form (17) is preserved beside it as memory/grind/motion_Close/candidate_policy_clean_17.c so a session can fall back if the ruling goes against the reuse.

## Session 4 (permuter, 2026-08-11)

### THE HEADLINE: floor 16 is now reachable in the POLICY-CLEAN family
Session 3 reached 16 only through a construct it could not classify (the
D_800A2668 guard value carried in `p` via a function-pointer-pointer cast, to
manufacture an exact allocno priority TIE that declaration order then broke).
Session 4 reaches the same 16 with a strict priority WIN and no type punning:

    /* FAKE: loop-note reference weighting seats p in $s0 ... */
    do { p = &D_8008D070; } while (0);

**Mechanism, measured with the instrumented cc1 (BB2_ALLOC_DEBUG=1).** flow.c
weights REG_N_REFS by loop depth; the NOTE_INSN_LOOP_BEG/END pair the wrap
emits makes its body depth 1, so the `p` reference inside it counts twice.
`p`'s live_length does not change (the wrapped statement is where `p` was
already born), so global.c's `floor_log2(n_refs)*n_refs/live_length` moves
`p` from 7/7 = 20000 to 8/7 = 34285, overtaking count's unchanged 8/8 = 30000.
The role split becomes a plain priority win — no tie, no declaration-order
dependence (the form scores 16 with either declaration order: sweep5 d0/d2).

This is exactly frontier F4's probe 2 ("any form where p reaches parity
WITHOUT holding the guard leaves the guard load in a caller-saved temp"), and
it is CONFIRMED: the guard load stays in `$v0`, the prologue is not made worse
to pay for the flip, and the loop body carries the target's registers exactly.

### Provenance: this is a genuine permuter find, re-measured
`tools/permuter_campaign.py` campaign on the policy-clean 17-point chassis
(`tmp/grind/motion_Close/s4/wsA`, base score 468) returned `output-463-1`:
`do { count = ...; p = ...; } while (0);` around the two initialisers. The
permuter's weighted score is not the engine distance, so every proposal was
re-measured with `sandbox motion_Close --disable all`: that form scores **16**
(sweep4.log, `a1_dw0_inits`). The follow-up sweep minimised it to a
single-statement wrap around `p = &D_8008D070;` alone — same score, smallest
device (sweep5.log, `d4_dw0_p_only`), which is what `candidate.c` now holds.

### The workspace had to be hand-built (reusable recipe)
`asm/funcs/motion_Close.s` does not exist (the function is C-routed), so the
usual `import.py` / `mar_perm_workspace.sh` path cannot produce a `target.o`.
`tmp/grind/motion_Close/s4/target.s` is a hand transcription of
`objdump -dr build/src/ings2.o` over the motion_Close region, and
`setup.sh` VERIFIES it byte-for-byte (32 normalized lines incl. relocations)
against that region before building the workspace. `compile.sh` is the honest
pipeline (cc1 `-mel` + prologue_fix + maspsx + multu_pad, no regfix/asmfix),
i.e. the same cheat-invisible view the sandbox scores. A minimal 5-declaration
TU reproduces the full-file codegen exactly (25 insns, same sequence), so the
permuter did not have to chew on all 814 lines of ings2.c.

### Three campaigns, and what each one bought
- **wsA** (policy-clean 17 chassis, base 468, ~11k iterations): the a1 find
  above (463) plus one sub-base junk find (453) that deletes
  `count = (s32)&D_00000000;` entirely and reads `count` uninitialised —
  banked as `rejected/permuter-uninit-count-semantic-break.c`.
- **wsB** (session-3 role-flipped 16 chassis, base **835**, ~4k iterations,
  61 outputs): its best find, 468, is the *clean* chassis' base score — the
  mutations that improve the tie-pun form are the ones that DELETE the pun
  (the best output keeps only an rvalue cast inside the comparison). The
  role-flip basin drains back into the clean family; it is not a separate
  basin worth re-seeding. NB the permuter's weighted metric ranks the two
  chassis in the OPPOSITE order to the engine (835 vs 468 for sandbox 16 vs
  17), because reorderings cost 60 and ins/del cost 100 — do not use permuter
  score as a proxy for engine distance on this function.
- **wsC** (the new clean do-while(0) 16 chassis, base 463): launched and ran,
  produced no output before the WSL drvfs mount failed (below).

### The 16-point residual, re-attributed
    frame size + save/restore offsets + save order   ~8   dead by H1
    guard temp $v0 vs $t0 (the load AND the jalr reg)  2   OPEN, never probed
    beqz delay slot (target nop vs our sw s0,16)       2   hand-asm signal S-b
    materialisation order (count-pair before p-pair)   2   the F4 bind
    inner-guard / positional remainder                 2

The `$v0`-vs-`$t0` bucket is now the largest never-probed item: local-alloc
picks the caller-saved temp for both the guard load and the jalr target, and
no session has yet read the `.lreg` dump for it.

### The bind survives the change of mechanism
Assigning `p` before `count` still inverts everything: p's live_length grows
7 -> 8 (34285 -> 30000) and count's shrinks 8 -> 7 (30000 -> 38571), count
retakes `$s0`, score 20. Measured in both declaration orders (sweep5
`d1_dw0_pfirst`, `d3_dw0_pdecl_pfirst`); banked as
`rejected/dw0-pfirst-order-inverts-priority.c`. Nesting the wrap (`d5`) lifts
BOTH allocnos (p 9/7 = 38571, count 10/8 = 37500) and leaves the score at 16,
so nesting buys nothing here and prerequisite 3 of the do-while(0) rule never
needs to be invoked.

### Other measured forms this session (all 16 or worse)
Score 16: `d0` (wrap both inits), `d2` (wrap both inits, p declared first),
`d4` (wrap p's init only — the candidate), `d5` (nested wrap), `d9` (f hoisted
out of the loop), `d10` (plain `while` loop), `d11` (`(*p++)()`),
`b3` (role-flip chassis + trailing empty wrap), `c2` (guard read through a
`p`-typed pointer to D_800A2668 — still a type pun, no better than `d4`).
Score 17: `a2` (wrap count's init only), `a3`/`d7` (wrap the whole guard body —
lifts both allocnos equally), `a4` (trailing empty wrap), `b1` (wrap the inits
in the role-flip chassis — the wrap and the pun fight each other),
`d6` (wrap + `break` on the inner guard). Score 19: `d8` (guard duplicated so
the inits sit in their own wrap — 27 insns). Score 20: `d1`, `d3` (p first),
`c1` (both inits hoisted above the D_800A2668 guard).

### POLICY: the session-3 open question is now MOOT for reaching 16
`.claude/rules/do-while-zero-exception.md:23-24` (owner ruling 2026-07-06,
verbatim): "**`do { <any body> } while (0);` — including empty bodies — is a
sanctioned pure-C match device for ANY codegen effect, including register
allocation.**" Prerequisite 1 (inline FAKE annotation naming the observed
effect) is satisfied in `candidate.c`; prerequisite 2 explicitly does not gate
single-level wraps; prerequisite 3 (nesting justification) does not apply.
Precedent: the same file's confirmed marionation_Exec application
(`.claude/rules/do-while-zero-exception.md:99`). So no future session needs an
owner ruling on the s3 type-pun in order to work the 16-point family — the s3
form is retained only as `candidate_s3_tiepun_16.c`, as evidence about the
tie-break mechanism, and should not be built on.

### H1 is untouched and still caps the function
No form in any of these families can reach distance 0: REG_PARM_STACK_SPACE=16
forces frame >= 32 for any pure-C body containing a call, against the target's
16-byte frame with a zero outgoing-arg area. Nothing this session changes that,
and no `candidate-ready` was claimed.

### Session-4 artifacts
- `tmp/grind/motion_Close/s4/target.s` + `setup.sh` — the hand-built,
  byte-verified permuter target and workspace builder (reusable for any
  C-routed function with no `asm/funcs/*.s`).
- `tmp/grind/motion_Close/s4/wsA|wsB|wsC/` — the three campaign workspaces
  (base.c, compile.sh, target.o, campaign.log, output-*).
- `tmp/grind/motion_Close/s4/sweep4.py` + `sweep4.log` — sandbox + allocno
  re-measurement of the permuter proposals (found the clean 16).
- `tmp/grind/motion_Close/s4/sweep5.py` + `sweep5.log` — 12-form follow-up
  sweep that minimised the device and re-killed the p-first order.

### Tree state at end of session 4
`src/ings2.c` is reverted to HEAD (both sweeps restore it in a `finally:`
block; `git status` shows only `metrics/events.jsonl`), so
`build/src/ings2.o` remains a valid target reference.

### INFRASTRUCTURE INCIDENT (not a finding, but the next session should know)
Late in the session the WSL `drvfs` mount of `/mnt/c` began returning
`OSError: [Errno 5] Input/output error` under the load of three concurrent
6/5/5-job campaigns, which killed all three permuter processes and then took
the whole WSL service down (`wsl --shutdown` hung; subsequent `wsl`
invocations return `Wsl/Service/E_UNEXPECTED`). No campaign outlived the
session — they were killed by the failure, not left simmering. **Correction to the first
draft of this note:** the `harvest --stop` telemetry WAS written after all —
`tools/permuter_campaign.py` runs fine under Windows Python (it only needs the
workspace directories), so all three campaigns were harvested from there and
then cleared with `deactivate-all`. That harvest is what surfaced the 383 find
recorded below, so it was worth doing rather than skipping. Two Windows-side
caveats for the next session: `harvest --stop` reports `stopped: false /
procs_killed: 0` because it cannot signal a Linux pid (the processes were
already dead — `pid_alive_at_harvest: false`), which leaves the registry entry
`active`, so follow it with `deactivate-all`; and `status` crashes outright on
Windows (`os.kill(pid, 0)` on a Linux pid raises `WinError 5`). Practical
lesson on the mount: do not run three campaigns at 5-6 jobs each against
`/mnt/c` simultaneously on this machine.

- [s4] New POLICY-CLEAN floor-16 form: a single-level `do { p = &D_8008D070; } while (0);` wrap (FAKE-annotated) reaches the target's $s0=p / $s1=count role split by a STRICT allocno priority win — p 8 refs / live_length 7 = 34285 vs count 8/8 = 30000 — instead of session 3's manufactured tie. flow.c's loop-depth weighting of REG_N_REFS counts the wrapped reference twice without lengthening p's live range. Measured with sandbox (16) and the instrumented cc1 (BB2_ALLOC_DEBUG=1); scores 16 in BOTH declaration orders, so unlike the s3 form it does not depend on the allocno tie-break.

- [s4] Frontier F4 probe 2 is CONFIRMED: reaching p's priority without making p hold the guard leaves the guard load in a caller-saved temp ($v0), so the prologue is not degraded to pay for the role flip (session 3's form dragged the guard load into callee-saved $s0 and paid `sw s0` + `sw s1` in the beqz delay slot). Same score, strictly better structure and no unclassified construct.

- [s4] The session-3 OPEN POLICY QUESTION (is `p = (void (**)(void))D_800A2668;` variable-reuse or coercion?) is MOOT for reaching floor 16: the do-while(0) route gets there with a construct whose sanction is explicit — `.claude/rules/do-while-zero-exception.md:23-24` sanctions `do { <any body> } while (0);` for ANY codegen effect including register allocation, single-level wraps are not exhaustion-gated, and the inline FAKE annotation is present. No ruling needs to be sought before working this family.

- [s4] Permuter provenance: the lever came from campaign wsA (policy-clean 17 chassis, base score 468, ~11k iterations) as output-463-1 (`do { count = ...; p = ...; } while (0);`), then was minimised by sweep5 to the single-statement wrap. The campaign's only other sub-base find (453) deletes count's initialiser and reads count uninitialised — semantically broken, banked as rejected/permuter-uninit-count-semantic-break.c. Permuter finds on this function are PROPOSALS; its weighted score even ranks the two chassis opposite to the engine (835 vs 468 for sandbox 16 vs 17).

- [s4] Campaign wsB (seeded from the s3 role-flipped tie-pun form, base 835, ~4k iterations, 61 outputs) converged back to the clean family: its best output (468) is the clean chassis' BASE score and its mutations delete the pun. The tie-pun form is not a distinct basin worth re-seeding.

- [s4] The order/roles bind survives the change of mechanism: with the do-while(0) win in place, assigning p before count still grows p's live_length 7->8 (34285->30000) and shrinks count's 8->7 (30000->38571), count retakes $s0 and the score returns to 20 — measured in both declaration orders (sweep5 d1/d3). Closing the 2-point materialisation-order bucket needs p at ~11 weighted refs against count's 38571, or F4b's lengthen-count-without-referencing-count.

- [s4] Nesting the wrap is inert here: `do { do { ... } while (0); } while (0);` lifts BOTH allocnos (p 9/7 = 38571, count 10/8 = 37500) and leaves the score at 16, so the do-while(0) rule's prerequisite-3 (nested-wrap justification) never needs to be invoked on this function.

- [s4] 23 further forms measured this session (sweep4 11, sweep5 12), disjoint from the 45 banked in s1-s3. Nothing beat 16. The residual re-attributes as: frame/save-restore ~8 (dead by H1), guard temp $v0-vs-$t0 2 (OPEN — the largest never-probed bucket; no session has read the .lreg local-alloc dump for it), beqz delay slot 2 (hand-asm signal S-b), materialisation order 2 (the F4 bind), positional remainder 2.

- [s4] A permuter workspace CAN be built for a C-routed function with no asm/funcs/*.s: transcribe the target region from `objdump -dr build/src/<file>.o`, assemble it, and verify the normalized disassembly (including relocation lines) against that region before use. tmp/grind/motion_Close/s4/{target.s,setup.sh} do exactly this and are reusable. A minimal 5-declaration TU reproduced ings2.c's full-file codegen for this function exactly, so the permuter did not have to process the whole 814-line file.

- [s4] INFRASTRUCTURE: three concurrent campaigns (6+5+5 jobs) against /mnt/c broke the WSL drvfs mount (Errno 5) and then the WSL service itself (Wsl/Service/E_UNEXPECTED). All three campaigns died with it — none outlived the session — but harvest --stop telemetry could not be written, so tmp/permuter_campaigns.json holds three unharvested launches for an operator to clear.

### LATE FIND, UNMEASURED — the end-of-session harvest surfaced a 383
The `harvest --stop` telemetry (run from Windows Python after WSL died) shows
campaign wsA also produced **`output-383-1` at 754 s** — permuter weighted
score 383 against that workspace's base of 468, a much larger drop than the
463 find that produced this session's candidate. It appeared after the last
directory listing of the session and could NOT be re-measured with the engine
sandbox, because by then the WSL drvfs mount and then the WSL service itself
had failed (see the infrastructure note above). It is banked verbatim as
`memory/grind/motion_Close/unmeasured_lead_s4_383.c` and is the FIRST thing
session 5 should measure.

Its two constructs, and why they must be measured separately:
1. `count = D_800A2668 != 0;` — the guard staged into `count`, which is then
   overwritten with the table length inside the branch. Same idea as session
   3's guard carry but through the s32 local rather than a type-punned
   function-pointer-pointer, so it carries no cast and sits inside the frozen
   "variable reuse for codegen control" family. It also composes with this
   session's do-while(0) wrap, which neither the permuter nor this session
   tried in combination.
2. `D_800A2668++; D_800A2668--;` — a redundant RMW pair on the guard global.
   This is a FIRST REACH of a family nothing in the frozen list covers. If GCC
   folds the pair (likely — two adjacent RMWs on the same non-volatile location
   with nothing between them), it emits no bytes and its only function is to
   move GCC's analysis: checklist T1 and T2 both fail and it is a coercion by
   any spelling. If it does NOT fold, it stores to a global the target never
   stores to, i.e. a behavioural difference rather than a match. Either way it
   is not submittable as-is; if it proves load-bearing the correct outcome is
   `ruling-request`, never self-approval.

Also from the final harvest: wsA ran 754+ s and produced 3 novel finds
(468/453/383); wsB ran ~320 s to 3 novel finds, all in the 807-833 band, i.e.
still far above the clean chassis' 468 base — reinforcing that the s3 tie-pun
basin is the wrong place to search; wsC ran 892 s / 1043 iterations from base
463 with **zero** finds, which is the clean do-while(0) chassis' own basin
reporting empty (a valid negative: the immediate neighbourhood of the floor-16
candidate holds nothing better under random mutation).

- [s4] LATE UNMEASURED LEAD: campaign wsA's best find is output-383-1 (permuter 383 vs base 468, at 754 s), surfaced only by the end-of-session harvest telemetry and never re-measured with the sandbox because WSL had died. Banked as memory/grind/motion_Close/unmeasured_lead_s4_383.c. It combines (a) `count = D_800A2668 != 0;` guard staging through the s32 local — a cast-free relative of session 3's guard carry, inside the sanctioned variable-reuse family, and untried in combination with the do-while(0) wrap — and (b) `D_800A2668++; D_800A2668--;`, a redundant RMW pair on the guard global that is a first reach of an uncovered family and fails T1/T2 if it folds (and diverges behaviourally if it does not). Session 5 must measure the two constructs SEPARATELY and must not submit (b) without a ruling.

- [s4] Campaign wsC (the clean do-while(0) floor-16 chassis, base 463) ran 892 s / 1043 iterations and produced ZERO finds — a valid negative: the immediate random-mutation neighbourhood of the new candidate holds nothing better. wsB's three finds all sat in the 807-833 band against the clean chassis' 468 base, confirming the s3 tie-pun basin is the wrong place to search.

- [s4] New POLICY-CLEAN floor-16 form banked as memory/grind/motion_Close/candidate.c: `do { p = &D_8008D070; } while (0);` with an inline FAKE annotation. Score 16, build_insns 25 vs target 26, measured twice (sweep4 a1 variant, sweep5 d4).

- [s4] Instrumented-cc1 table for the candidate (BB2_ALLOC_DEBUG=1): p n_refs=8 live_length=7 pri=34285 -> $16; count n_refs=8 live_length=8 pri=30000 -> $17. A strict win, not a tie — the form scores 16 with either declaration order, so it does not use global.c's allocno-number tie-break at all.

- [s4] Session 3's OPEN POLICY QUESTION (is `p = (void (**)(void))D_800A2668;` sanctioned variable reuse or a coercion?) is MOOT for reaching floor 16 and no ruling need be sought before working this family. The s3 form is preserved as memory/grind/motion_Close/candidate_s3_tiepun_16.c for its evidence about the tie-break and should not be built on.

- [s4] Policy basis, quoted verbatim from .claude/rules/do-while-zero-exception.md:23-24: "**`do { <any body> } while (0);` — including empty bodies — is a sanctioned pure-C match device for ANY codegen effect, including register allocation.**" Prerequisite 1 (inline FAKE annotation naming the observed effect) satisfied; prerequisite 2 explicitly does not gate single-level wraps; prerequisite 3 not applicable (single level). Precedent: the same file's confirmed marionation_Exec RA-weighting application at line 99.

- [s4] 16-point residual re-attribution: frame size + save/restore offsets + save order ~8 (dead by H1); guard temp $v0 vs $t0 for both the load and the jalr register 2 (OPEN — the largest never-probed bucket, no session has read the .lreg local-alloc dump for it); beqz delay slot 2 (hand-asm signal S-b); materialisation order count-pair-before-p-pair 2 (the F4 bind); inner-guard/positional remainder 2.

- [s4] 23 further forms measured this session (sweep4 11, sweep5 12), disjoint from the 45 banked in s1-s3. Nothing beat 16. Also 16: wrap both inits (either declaration order), nested wrap, f hoisted out of the loop, plain `while` loop, `(*p++)()`, role-flip chassis + trailing empty wrap, guard read through a p-typed pointer. 17: wrap count's init only, wrap the whole guard body, trailing empty wrap, wrap inside the role-flip chassis, wrap + break on the inner guard. 19: guard duplicated so the inits sit in their own wrap (27 insns). 20: p assigned first (both declaration orders), both inits hoisted above the D_800A2668 guard.

- [s4] H1 is untouched and still caps the function: REG_PARM_STACK_SPACE=16 forces frame >= 16+12 = 28 -> 32 for any pure-C body containing a call, versus the target's 16-byte frame with a zero outgoing-arg area. No form in any family measured so far can reach distance 0, and no candidate-ready was claimed.

- [s4] Permuter workspaces for C-routed functions: transcribe the target region from `objdump -dr build/src/<file>.o`, assemble, and verify the normalized disassembly INCLUDING relocation lines against that region before use (tmp/grind/motion_Close/s4/target.s + setup.sh do this and are reusable). Branch-target text differs between an offset-0 object and the real one even when the encodings are identical, so compare encodings, not mnemonic operands.

- [s4] src/ings2.c is reverted to HEAD (both sweeps restore it in a finally: block); `git status` shows only metrics/events.jsonl, so build/src/ings2.o remains a valid target reference for the next session.

- [s4] LATE UNMEASURED LEAD (top of the frontier): the end-of-session harvest telemetry shows campaign wsA also produced output-383-1 at 754 s — permuter weighted score 383 against base 468, a much larger drop than the 463 find that produced this session's candidate. It appeared after the last directory listing and could NOT be re-measured with the sandbox because WSL had already failed. Banked verbatim as memory/grind/motion_Close/unmeasured_lead_s4_383.c with a full policy warning: it combines cast-free guard staging through `count` (sanctioned variable-reuse family, and untried in combination with the do-while(0) wrap) with `D_800A2668++; D_800A2668--;`, a first reach of an uncovered family that fails T1/T2 if it folds and diverges behaviourally if it does not. Session 5 must measure the two constructs separately and must not submit the second without a ruling.

- [s4] Final harvest numbers: wsA 754+ s, 3 novel finds (468/453/383); wsB ~320 s, 3 novel finds all in the 807-833 band — far above the clean chassis' 468 base, reinforcing that the s3 tie-pun basin is the wrong place to search; wsC 892 s / 1043 iterations from base 463 with ZERO finds, a valid negative saying the immediate random-mutation neighbourhood of the new floor-16 candidate holds nothing better.

- [s4] INFRASTRUCTURE INCIDENT: three concurrent campaigns (6+5+5 jobs) against /mnt/c broke the WSL drvfs mount (OSError Errno 5), which killed all three permuter processes and then wedged the WSL service itself (`wsl --shutdown` hung; later `wsl` calls return Wsl/Service/E_UNEXPECTED). No campaign outlived the session — they were killed by the failure, not left simmering — and all three WERE harvested and deactivated before the session ended, from Windows Python (tools/permuter_campaign.py needs only the workspace directories). That harvest is what surfaced the 383 find. Windows-side caveats for the next session: `harvest --stop` reports stopped:false / procs_killed:0 because it cannot signal a Linux pid (the processes were already dead, pid_alive_at_harvest:false), so the registry entry stays `active` and must be cleared with `deactivate-all`; and `status` crashes on Windows (os.kill(pid,0) raises WinError 5). WSL itself is left wedged and likely needs a host reboot. Do not run three campaigns at 5-6 jobs each against /mnt/c simultaneously on this machine.

- [s5b] FLOOR 13 CONFIRMED AND BANKED. A prior session-5 run reached 13 but was discarded by the driver on foreign dirt, and it never updated candidate.c — the form survived only as a comment line inside rejected/pfirst-wrap-depth-1-and-2-insufficient.c. Session 5b reconstructed it (p assigned FIRST, a THREE-level `do { do { do { p = &D_8008D070; } while (0); } while (0); } while (0);` wrap, then `count = (s32)&D_00000000;`), applied it to src/ings2.c and independently re-measured it: `sandbox motion_Close --disable all` -> score 13, target_insns 26, build_insns 25. candidate.c now carries this form with its full mechanism and policy write-up, so the floor is no longer one discarded session away from being lost.

- [s5b] FRONTIER F5 IS STRUCTURALLY KILLED — the guard-load and jalr temps can NEVER be moved from $v0 to the target's $t0 by any pure-C form that also emits the target's instruction sequence. Both temps are BLOCK-LOCAL pseudos (74 in block 0, 75 in block 2 — tmp/grind/motion_Close/s5b/f13.c.lreg), so local-alloc.c owns them; greg's dispositions read `74 in 2  75 in 2`. local-alloc.c:2249-2262 and global.c:1057-1062/1203-1209 both scan hard registers in ASCENDING NUMERIC ORDER (`#ifdef REG_ALLOC_ORDER ... #else int regno = i; #endif`), and REG_ALLOC_ORDER is not defined anywhere in the MIPS backend, so $v0 (hard reg 2) is the first allocatable GR_REGS register and is always taken unless excluded. Reaching $t0 (hard reg 8) requires regs 2,3,4,5,6,7 ALL excluded over both live ranges — i.e. six live call-clobbered values — but the target's byte stream contains no value in $v0/$v1/$a0-$a3 anywhere (the call takes no args and returns void), so producing them would add strictly more distance than it removes. Banked as rejected/f5-t0-unreachable-alloc-scan-order.c.

- [s5b] THE FLOOR-13 RESIDUAL IS NOW 11/13 PROVEN UNREACHABLE by two independent mechanisms. Decomposition from tmp/grind/motion_Close/s5b/wsA/_base.txt vs _tgt_mine.txt: ~7 points of frame size + save offsets + restore offsets, dead by H1 (REG_PARM_STACK_SPACE = 16 forces frame >= 32 against the target's 16); 4 points of $v0-vs-$t0 on the guard lui/lw, the body lw and the jalr, dead by F5 above; leaving 2 open points (the beqz delay slot — target nop, ours `sw s0` — and the prologue save ORDER). This is evidence FOR an eventual escalation of motion_Close, not a disposition: the driver decides exhaustion, the modality ladder still has untried rungs, and 2 points remain nominally open.

- [s5b] PERMUTER CAMPAIGN on the floor-13 chassis (tmp/grind/motion_Close/s5b/wsA, label pfirst_wrap3_floor13, base score 413, 6 jobs): 55,660 iterations over 1421 s produced exactly TWO novel finds, and BOTH are dead. `output-259-1` (permuter 259, the session's best drop) adds `volatile unsigned int new_var; new_var = 0;` — re-measured with the engine it scores 13, i.e. IDENTICAL to the chassis, with build_insns 25 -> 27; it is also an unused-local frame-coercion cheat outside the written-never-read carve-out (the target has no such dead store), so it was rejected in-session and never surfaced. `output-370-1` (permuter 370) stages the guard through a local plus a pointer alias to that local — the guard-staging half is already dead (rejected/staged-guard-local-shortens-count-liverange.c, re-confirmed at 17 by the discarded s5 run). Banked as rejected/volatile-dead-local-engine-neutral.c.

- [s5b] FOURTH confirmation that permuter weighted score does NOT track engine distance on motion_Close: s3 chassis 835 permuter / 16 engine vs clean chassis 468 / 17; s4's 383 find measured 18; s5b's 259 find measures 13 (no change). Any future campaign on this function must re-measure every find with `sandbox --disable all` before treating it as progress, and a large permuter drop here is not evidence of anything on its own.

- [s5] Floor 13 measured twice this session with the floor-13 body in src/ings2.c: `sandbox motion_Close --disable all` -> score 13, target_insns 26, build_insns 25. candidate.c now carries this form (it previously carried session 4's floor-16 form); the prior session-5 run that first reached 13 was discarded on foreign dirt and had left the form only as a comment line inside rejected/pfirst-wrap-depth-1-and-2-insufficient.c.

- [s5] The floor-13 residual decomposes (tmp/grind/motion_Close/s5b/wsA/_base.txt vs _tgt_mine.txt) as ~7 points of frame size + save offsets + restore offsets (dead by H1), 4 points of $v0-vs-$t0 on the guard lui/lw, the body lw and the jalr (dead by F5, killed this session), and 2 open points: the beqz delay slot (target nop, ours `sw s0`) and the prologue save ORDER. 11 of 13 are now proven unreachable by two independent mechanisms.

- [s5] local-alloc.c:2249-2262 and global.c:1057-1062 / 1203-1209 both select the first free hard register in ascending numeric order; `grep -n REG_ALLOC_ORDER tools/gcc-2.7.2/config/mips/*.h` returns nothing and regclass.c:112 defines reg_alloc_order only under that #ifdef. $v0 is therefore the first allocatable GR_REGS register for every block-local temp in this function.

- [s5] f13.c.lreg on the floor-13 chassis: `Register 74 used 2 times across 4 insns in block 0` (guard load) and `Register 75 used 4 times across 4 insns in block 2` (call target) are block-local, so local-alloc owns both; f13.c.greg: `;; Register dispositions: 72 in 17  73 in 16  74 in 2  75 in 2` - count/p correctly in $s1/$s0, both temps in $v0.

- [s5] Permuter campaign telemetry: base_score 413, 55,660 iterations, 1421 s, 6 jobs, finds_total 2, best_new_score 259, stopped with --stop (procs_killed 7, deactivate-all clean). Both finds re-measured or classified in-session; neither improved the engine floor.

- [s5] FOURTH confirmation that permuter weighted score does not track engine distance on motion_Close: s3 chassis 835 permuter / 16 engine vs clean chassis 468 / 17; s4's 383 find measured 18; s5b's 259 find measures 13 (no change). Every find on this function must be re-measured with the sandbox before it means anything.

- [s5] H1 unchanged and still caps the function: gcc-2.7.2's o32 backend reserves REG_PARM_STACK_SPACE = 16 bytes of outgoing-arg area for every C-level call, so any pure-C body containing a call has frame >= 28 -> 32 against the target's 16 (census of all 854 call-making functions in the oracle build: none below 16). No form in this family can reach distance 0, so no candidate-ready is possible here.

- [s6] FRONTIER F7 IS STRUCTURALLY KILLED IN BOTH HALVES — the prologue SAVE ORDER and the empty beqz DELAY SLOT are each unreachable from any pure C that also emits the target's basic-block shape. With that, ALL 13 residual points of the floor-13 form are accounted for by a frozen-toolchain mechanism (H1 frame ~7, F5 $v0-vs-$t0 4, F7 2) and motion_Close has NO nominally-open gradient left.

- [s6] SAVE ORDER, mechanism. `tools/gcc-2.7.2/config/mips/mips.c:save_restore_insns` emits the callee-saved stores in ONE fixed direction — `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)` — with the source comment "Save registers starting from high to low. The debuggers prefer at least the return register be stored at func+4". There is no flag, no target hook and no C-level input that reverses that loop: cc1 ALWAYS emits `sw ra` first and `sw s0` last. Raw cc1 output for the floor-13 chassis (tmp/grind/motion_Close/s6/f13.s) is `sw $31,24($sp)` / `sw $17,20($sp)` / `sw $16,16($sp)`; the target is the exact inverse (`sw s0,4` / `sw s1,8` / `sw ra,12`). The ONLY thing that can reorder them afterwards is sched2, which is a per-BASIC-BLOCK list scheduler and therefore needs a dependence inside the prologue block.

- [s6] SAVE ORDER, corpus measurement (tmp/grind/motion_Close/s6/save_order_scan.py + .log, 1788 oracle-build functions, 606 with >=2 callee saves). Shapes: 146 descending (cc1-native), 334 mixed, 126 ASCENDING. So ascending IS reachable — the naive "mips.c loop ⇒ structural kill" reading is WRONG and must not be re-asserted. What produces it: in EVERY ascending case the prologue block also contains a WRITE to the same callee-saved register — overwhelmingly an incoming-argument copy (`sw s0,N(sp)` / `move s0,a0` / `sw s1,M(sp)` / `move s1,a1` / `sw ra,K(sp)`). The store reads sN and the copy writes it, so sched.c's WAR anti-dependence forces each `sw sN` ahead of its `move sN,aM`, which reverses the emission order. tmp/grind/motion_Close/s6/asc_prologues.py isolates the 29 functions whose save mask is exactly motion_Close's {s0,s1,ra} and prints their prologues; 28 of the 29 carry that arg-copy interleave.

- [s6] SAVE ORDER, why motion_Close cannot have it. Filtering those 29 for "no write to s0/s1 anywhere in the prologue block" leaves EXACTLY TWO functions in the whole corpus: motion_Close and its sibling func_80083794 — the same cheat-carrying crt0 pair, i.e. zero compiled-C instances. motion_Close is `void motion_Close(void)`: it has no parameters, so no argument copy exists, and its only writes to s0/s1 are the two address materialisations which the TARGET ITSELF places AFTER the guard branch (f88-f94, i.e. in a different basic block from the saves at f74-f7c). GCC 2.7.2 schedules per basic block, so no dependence can exist between the saves and those writes, the ready list preserves cc1's emission order, and the saves come out ra,s1,s0. Moving the materialisations before the branch to manufacture the anti-dependence would place them where the target does not have them (already measured dead in s3: "count hoisted above the D_800A2668 guard" = 19).

- [s6] EMPTY DELAY SLOT, mechanism. reorg.c's `fill_simple_delay_slots` (tools/gcc-2.7.2/reorg.c:2941-3012) scans BACKWARD from the branch — `for (trial = prev_nonnote_insn (insn); ! stop_search_p (trial, 1); ...)` — stopping only at a label or jump, and takes the first insn that neither references nor sets the branch's resources and survives `try_split` + `eligible_for_delay` (so a 2-insn `la` pseudo is ineligible, which is why the INNER guard's slot stays empty in our build too). In the target's guard-branch block the branch tests $t0 and is preceded by `addiu sp,sp,-16`, `sw s0,4(sp)`, `sw s1,8(sp)`, `sw ra,12(sp)` — four single-insn candidates, none of which sets $t0. reorg would therefore ALWAYS fill that slot. Our own floor-13 build is the demonstration: f13.c.dbr shows `(insn 116 (sequence[ (jump_insn 11 ...) (insn 109 = sw s0,16(sp)) ]))`, i.e. the last prologue save moved into the slot.

- [s6] EMPTY DELAY SLOT, corpus measurement (tmp/grind/motion_Close/s6/prologue_scan.py + .log). Across all 1788 functions there are 118 CONDITIONAL branches immediately preceded by a callee-saved `sw`; only SIX leave the slot empty, and one of the six is motion_Close itself. Inspecting the other five (tmp/grind/motion_Close/s6/empty_slot_cases.log): in every one the preceding `sw` is itself already occupying the delay slot of an EARLIER jal/branch (func_800278C0 `sw ra` in a beq slot; func_80036E34 and snd_LoadBgm `sw s0` in a jal slot; func_80074488 `sw s2` in a `j` slot; func_8002304C is prologue_config-rewritten), so the conditional branch actually begins a fresh basic block with nothing eligible before it. Zero counterexamples: no compiled function in the corpus leaves a conditional-branch delay slot empty while an eligible single insn precedes it in the same block. motion_Close's target has four such insns and a nop — that is not compiler output.

- [s6] F8 / hand-coded signal tier MEASURED at last (the probe sessions 1-4 could not run because asm/funcs/motion_Close.s does not exist). Synthesised asm/funcs/motion_Close.s from the s4 byte-verified target.s, ran `python3 tools/scan_hand_coded.py --single motion_Close`, then DELETED the file (no repo dirt). Result: **tier=LOW, score 0/8, "no strong hand-coded indicators"** — S1 multu pacing, S2 empty-body branch, S6 BIOS jumptable all negative, S3/S4 skipped as too short (26 < 40 insns). The sibling func_80083794 (54 insns, real asm/funcs file) also scores tier=LOW 0/8. Log: tmp/grind/motion_Close/s6/scan_hand_coded.log. The endgame-lock canonical-asm gate requires STRONG S1/S2/S6 — it is therefore a MEASURED FAILED GATE, not an open question, and any escalation entry must say so.

- [s6] Floor re-measured at 13 with candidate.c applied to src/ings2.c (`sandbox motion_Close --disable all` -> score 13, target_insns 26, build_insns 25); src/ings2.c was restored to HEAD before the session ended, so the tree still byte-matches the oracle.

- [s6] Floor unchanged at 13 and re-measured this session with candidate.c applied to src/ings2.c: `sandbox motion_Close --disable all` -> score 13, target_insns 26, build_insns 25. src/ings2.c was restored to HEAD before the session ended, so the tree still byte-matches the oracle; the only working-tree change is metrics/events.jsonl.

- [s6] The floor-13 residual is now 13/13 mechanism-explained, up from 11/13: ~7 points frame size + save/restore offsets (H1, REG_PARM_STACK_SPACE=16 forces frame 32 against the target's 16), 4 points $v0-vs-$t0 (F5, ascending first-free hard-reg scan with no MIPS REG_ALLOC_ORDER), 2 points save order + delay slot (F7, killed this session). No unexplained instruction remains anywhere in the function.

- [s6] CORRECTION for the ledger: 'mips.c emits saves high->low, therefore the target's ascending order is impossible' is WRONG as stated — 126 of 606 multi-save oracle-build functions emit ascending saves. The correct kill is narrower and stronger: ascending order requires a WAR anti-dependence inside the PROLOGUE BASIC BLOCK (an incoming-argument copy into the saved register), and motion_Close is void(void) with its s0/s1 writes in a later block, which is why it and its sibling are the only two of 1788 functions showing ascending saves without such a write.

- [s6] reorg.c's backward delay-slot scan also explains, for free, why the INNER guard's slot is a nop in BOTH the target and our build: the preceding insn is a 2-insn `la` pseudo, ineligible for a delay slot. That nop is not a residual point and must not be chased.

- [s6] New reusable artifacts for the corpus: tmp/grind/motion_Close/s6/save_order_scan.py (per-function prologue save-order shape over build/src/*.o) and prologue_scan.py (conditional branches preceded by a callee-save store, with delay-slot occupancy). Both run in ~40 s and answer 'is this prologue shape compiler-reachable' for any function, not just this one.

- [s6] Session 6 ran the whole forensics modality without editing any file outside src/ings2.c (reverted), memory/grind/motion_Close/ and tmp/grind/motion_Close/s6/. The asm/funcs/motion_Close.s needed by scan_hand_coded was created and deleted within the session.

## [s7] H1 is now a BACKEND claim, not a census claim (F9 killed)

Sessions 1-6 supported H1 with a corpus census: of the 854 call-making functions
in the oracle-matching build, none has an outgoing-argument area below 16 bytes
except motion_Close and its sibling func_80083794, and both reach it only because
their `jalr` is inline asm. That is an argument about OUR build. Session 7 closed
the complementary question by reading the frozen compiler and then measuring it.

**Reading.** `mips.c:compute_frame_size` derives the arg area solely from
`current_function_outgoing_args_size` (`args_size = MIPS_STACK_ALIGN (cfoas)`,
mips.c:4466), with one adjustment that only ever RAISES it (`if (args_size == 0 &&
current_function_calls_alloca) args_size = 4*UNITS_PER_WORD`). `cfoas` is written
in exactly three places outside save/restore bookkeeping: `calls.c:1400-1401`
(expand_call), `calls.c:2400-2401` and `calls.c:2750-2751` (emit_library_call),
and `integrate.c:1358/3066` (propagating an inlinee's value by MAX). All of them
apply `args_size.constant = MAX (args_size.constant, reg_parm_stack_space)`
first, and on MIPS `reg_parm_stack_space` is a compile-time constant 16:
`REG_PARM_STACK_SPACE` (mips.h:1822) is `MAX_ARGS_IN_REGISTERS(4) *
UNITS_PER_WORD(4) - FIRST_PARM_OFFSET`, and `FIRST_PARM_OFFSET` is the `#else`
arm of an `#if 0` block, i.e. 0. The two macros that could zero it out
(`MAYBE_REG_PARM_STACK_SPACE`, `FINAL_REG_PARM_STACK_SPACE`) are undefined in
`config/mips/`, and `OUTGOING_REG_PARM_STACK_SPACE` IS defined, which kills the
`#ifndef`-guarded subtraction at calls.c:1253 that would otherwise remove it.

**Enumeration.** Every `emit_call_insn` site in the compiler: calls.c:387/398/403
(emit_call_1, shared by expand_call and emit_library_call — accounted);
integrate.c:1807, unroll.c:1948, loop.c:1801/1845 (copies of an already-expanded
call — accounted, and integrate.c:1358 carries the inlinee's value across);
expr.c:8361/8382 in `expand_builtin_apply` — the ONLY site that never touches
cfoas.

**Measurement** (tmp/grind/motion_Close/s7/f9probe.sh, f9probe2.sh; emitted
assembly under tmp/grind/motion_Close/s7/out/), canonical cc1 flags:

| probe | shape | `.frame` |
|---|---|---|
| p1 | motion_Close-shaped indirect call through a table | `$sp,32` vars=0 regs=3 **args=16** |
| p2 | plain direct call | `$sp,24` **args=16** |
| p3 | `__builtin_apply` | `$fp,72` vars=56 regs=3 **args=0** |
| p4 | UNPROTOTYPED call | `$sp,24` **args=16** |
| p5 | call to `__attribute__((const))` fn | `$sp,24` **args=16** |
| p6 | leaf, no call | `$sp,0` **args=0** |
| p7 | call inlined from a static callee | `$sp,24` **args=16** |
| p8 | call inside a loop (LICM/unroll copy path) | `$sp,24` **args=16** |

The `__builtin_apply` hole is real and is the only one, but it is unusable: it
buys `args=0` at the price of a frame POINTER, a 72-byte frame, 56 bytes of vars,
`$fp` in the save mask and 34 emitted instructions, versus the target's 16-byte
frame, `{s0,s1,ra}` mask and 26 instructions. It is also a GCC extension that no
1998 PsyQ crt0 would contain.

**Consequence.** H1 now reads: *the MIPS o32 backend of the frozen compiler
reserves a 16-byte outgoing-argument area for every call it expands, as a
compile-time constant applied by a MAX on every path that can set the frame's arg
size; no C source that emits a call can produce this function's 16-byte frame.*
That is the "no C input to this compiler produces these bytes" form the
no-new-park-categories carve-outs require, and it is the sentence an escalation
entry should cite. It also disposes of F9's own suggested escapes by measurement:
an unprototyped call, a const-attributed call, an inlined call and a call in a
loop all emit args=16.

## [s7] The residual is a table now, not a narrative

`tmp/grind/motion_Close/s7/residual_table.md` pairs all 26 target instructions
against the 25 build instructions at the current floor and classifies every
differing position. 13 differing positions; measured score 13; buckets are 5 ×
F5 ($v0-vs-$t0), 6.5 × H1 (frame size + save/restore offsets), 1.5 × F7a/F7b
(save order + empty beqz delay slot). No unexplained instruction remains
anywhere in the function.

- [s7] Floor re-measured this session at 13 with candidate.c applied to src/ings2.c: score 13, target_insns 26, build_insns 25, rules_dropped 9, cheat_asm_stripped 10. src/ings2.c was then restored to HEAD so the oracle build is untouched; the form lives in memory/grind/motion_Close/candidate.c.

- [s7] REG_PARM_STACK_SPACE on MIPS is the compile-time constant 16 for every fndecl: mips.h:1822 defines it as MAX_ARGS_IN_REGISTERS(4, mips.h:1888) * UNITS_PER_WORD(4) - FIRST_PARM_OFFSET, and FIRST_PARM_OFFSET is the #else arm of an #if 0 block at mips.h:1801-1812, i.e. 0. It does not depend on the callee, its prototype, or its argument list.

- [s7] The two macros that could zero the reservation (MAYBE_REG_PARM_STACK_SPACE at calls.c:648/1249, FINAL_REG_PARM_STACK_SPACE at calls.c:1193) are undefined in config/mips/; OUTGOING_REG_PARM_STACK_SPACE IS defined (mips.h:1830), so the #ifndef subtraction at calls.c:1253 is dead. The live code is calls.c:1245 MAX -> calls.c:1400 store into current_function_outgoing_args_size, unconditional in the args_size.var==0 branch and not gated on must_preallocate.

- [s7] Complete enumeration of emit_call_insn sites in the frozen compiler: calls.c:387/398/403 (emit_call_1 — expand_call and emit_library_call, the latter MAXing at calls.c:2393-2401 and 2743-2751); integrate.c:1807, unroll.c:1948, loop.c:1801/1845 (copies of an already-expanded call; integrate.c:1358 propagates the inlinee's OUTGOING_ARGS_SIZE by MAX); expr.c:8361/8382 in expand_builtin_apply — the only site that never touches cfoas.

- [s7] mips.c:4466 computes the frame's arg area as MIPS_STACK_ALIGN(current_function_outgoing_args_size) and reads nothing else; the only adjustment (mips.c:4473-4474, args_size==0 && current_function_calls_alloca -> 4*UNITS_PER_WORD) can only RAISE it.

- [s7] MEASURED frames (canonical cc1 flags, tmp/grind/motion_Close/s7/f9probe.sh + f9probe2.sh, asm in s7/out/): indirect table call .frame $sp,32 args=16 | direct call $sp,24 args=16 | UNPROTOTYPED call $sp,24 args=16 | __attribute__((const)) call $sp,24 args=16 | call inlined from a static callee $sp,24 args=16 | call inside a loop $sp,24 args=16 | leaf with no call $sp,0 args=0 | __builtin_apply $fp,72 vars=56 regs=3/0 args=0 with 34 instructions and a frame pointer.

- [s7] H1 now reads, in the form the no-new-park-categories carve-outs require: the MIPS o32 backend of the frozen compiler reserves a 16-byte outgoing-argument area for every call it expands, as a compile-time constant applied by a MAX on every path that can set the frame's arg size, so no C source that emits a call can produce this function's 16-byte frame (target frame 16 with ra@12 back-solves to a ZERO arg area; the pure-C minimum is 16 args + 12 saves = 28 -> aligned 32, which is exactly what every honest form emits).

- [s7] The 26-row residual table (tmp/grind/motion_Close/s7/residual_table.md) pairs target against build at the floor of 13 and lands 13 differing positions — 5 x F5 ($v0-vs-$t0), 6.5 x H1 (frame size + save/restore offsets), 1.5 x F7a/F7b (ascending save order + empty beqz delay slot). The count equals the engine's score exactly, so no instruction in the function is unexplained.

- [s7] All three residual mechanisms are now 'the compiler cannot emit this' arguments carrying their own measurement: H1 (backend enumeration + 8 compiled probes, s7), F5 (local-alloc.c:2249-2262 / global.c:1057-1062,1203-1209 ascending first-free scan with no MIPS REG_ALLOC_ORDER, s5b), F7a/F7b (block-local scheduling: an ascending save order needs a prologue-block WAR anti-dependence a void(void) function cannot have — 2/1788 corpus instances, both this crt0 pair; an empty conditional-branch delay slot needs a block with no eligible single insn while the target's holds four — 0/118 compiled-C counterexamples, s6).

- [s8] Floor re-measured this session at 13 with candidate.c applied to src/ings2.c: score 13, target_insns 26, build_insns 25, rules_dropped 9, cheat_asm_stripped 10. The floor did not move; no lower form was found.

- [s8] The TU-level axis (frontier F10) is CODEGEN-INERT for motion_Close. Nine whole-region variants of src/ings2.c (tmp/grind/motion_Close/s8/tusweep.py) replaced BOTH motion_Close and its sibling func_80083794 per variant. Rewriting both functions as one parameterised `static __inline__ ctor_walk(void)` body (v3_shared_inline_both) scores 13; defining motion_Close BEFORE its sibling in the TU (v7_order_swap) scores 13; the control scores 13. They are not merely score-equal � the instrumented cc1 reports the SAME allocno table in all three (p 10 refs / live_length 8 / pri 37500 -> $s0; count 8 refs / 7 / 34285 -> $s1) and the emitted instruction streams are identical.

- [s8] GCC 2.7.2 inlines a `static __inline__` helper BEFORE flow.c counts references, so an inlined body and a written-out body are the same input to every pass that decides this function. Measured by crossing both inlined chassis with do-while(0) wrap depths 0/1/2/3 (tmp/grind/motion_Close/s8/depthsweep.py): depth 0 p 7 refs/8 = 17500 score 20; depth 1 p 8/8 = 30000 score 20; depth 2 p 9/8 = 33750 score 20; depth 3 p 10/8 = 37500 score 13 � value for value the direct-body ladder banked in s5b. F4b (reach 13 with a wrap depth below 3) is therefore NOT reachable via an inlined chassis; depth 3 stays necessary and minimal on every chassis measured.

- [s8] Passing the walk's state through helper PARAMETERS is strictly worse than letting the helper own it: `static __inline__ ctor_walk(s32 count)` scores 16, `static __inline__ ctor_walk(void (**p)(void), s32 count)` scores 20. A plain non-inlined `static` helper scores 23 and leaves motion_Close at 15 instructions with `.frame $sp,24 args=16` � the call simply moves to the other function and H1's 16-byte arg area moves with it.

- [s8] F7a is KILLED UNCONDITIONALLY, upgrading session 6's signature-conditional disproof. Giving motion_Close real incoming parameters (tmp/grind/motion_Close/s8/paramsweep.py � an ABI lie, measured only to answer the mechanism question) produces exactly the prologue-block WAR anti-dependence s6 named as the missing ingredient, and the saves STILL come out descending: w4_param_both_used emits `sw s1,20 / move s1,a0 / sw s0,16 / move s0,a1 / beqz / sw ra,24` (score 21) and w5_param_p_used emits `sw s1,20 / move s1,a0 / sw ra,24 / beqz / sw s0,16` (score 19). The anti-dependence changes what the saves are interleaved WITH; it does not change their relative order. Parameters that are dead (w1/w2/w3) are optimised away and score 13, identical to the void(void) control.

- [s8] The backend reason for that: `save_restore_insns` at tools/gcc-2.7.2/config/mips/mips.c:4680 is `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)`, with gp_offset initialised to the top of the save area (mips.c:4623) and decremented by UNITS_PER_WORD after each emitted store (mips.c:4710). The loop direction is unconditional � no target flag, no frame-pointer case and no ABI case selects the other direction, and the comment at mips.c:4609-4612 states the high-to-low order is a deliberate debugger/epilogue convention. GP saves are therefore ALWAYS emitted ra, s1, s0; only a block-local scheduling move can reorder them, and the scheduler breaks ties between two structurally symmetric stores by program order, which is that same descending order. The target's `sw $s0,4 / sw $s1,8 / sw $ra,12` is not producible from any C input to this compiler.

- [s8] Note on the F7a offsets, recorded to stop a future session mis-reading the residual table: the offset-to-register MAPPING is the SAME in target and build (ra highest, then s1, then s0). Only the ORDER of the three store instructions in the stream differs, and the offsets differ solely by the 16-byte H1 shift. F7a is an emission-order fact, not a frame-layout fact.

- [s8] Floor re-measured this session at 13 with candidate.c applied to src/ings2.c (score 13, target_insns 26, build_insns 25, rules_dropped 9, cheat_asm_stripped 10); src/ings2.c restored to HEAD afterwards so the oracle build is untouched.

- [s8] Rewriting BOTH motion_Close and func_80083794 as one parameterised `static __inline__ ctor_walk(void)` body scores 13 with an allocno table and instruction stream identical to the direct body — a structurally different C shape that changes nothing.

- [s8] Defining motion_Close BEFORE func_80083794 in the TU scores 13, identical stream: TU position is inert.

- [s8] GCC 2.7.2 inlines a `static __inline__` helper before flow.c counts references, measured by the wrap-depth ladder 20/20/20/13 for depths 0/1/2/3 on both inlined chassis — value for value the direct-body ladder banked in s5b. F4b cannot be reached by moving to an inlined chassis.

- [s8] Passing walk state through helper PARAMETERS is strictly worse: helper taking count 16, helper taking (p, count) 20. A plain non-inlined static helper scores 23 and leaves motion_Close at 15 insns with .frame $sp,24 args=16 — H1's 16-byte arg area follows the call to whichever function makes it.

- [s8] With two real incoming-argument copies in the prologue block (w4_param_both_used) the saves are still emitted s1-before-s0; the WAR anti-dependence changes what the saves are interleaved with, never their relative order.

- [s8] tools/gcc-2.7.2/config/mips/mips.c:4680 — `for (regno = GP_REG_LAST; regno >= GP_REG_FIRST; regno--)`, gp_offset from the top of the save area (mips.c:4623), decremented per store (mips.c:4710); unconditional, with the high-to-low order documented as deliberate at mips.c:4609-4612. The target's ascending save order is not producible from any C input to this compiler.

- [s8] Recorded to stop a future misreading of the s7 residual table: F7a is an emission-ORDER fact, not a frame-layout fact — the offset-to-register mapping already matches the target (ra highest, then s1, then s0) and the offsets differ only by the H1 16-byte shift.

- [s9] F11 KILLED: ten declaration forms of the three touched globals (array-typed table, array-typed count, both arrays, `extern char` count, `void *` count, const-qualified table, u32 guard, reversed extern order) all score 13 with build_insns 25 and the byte-identical allocno table (p 10 refs / live_length 8 / pri 37500 -> $s0; count 8/7 / 34285 -> $s1). Under -G0 an extern of any type is addressed by a lui/addiu %hi/%lo pair; the count symbol's value is never loaded (only its address taken) so its type cannot affect a load width; and the two symbols are at unrelated addresses (0x8008D070, 0x00000000) so no struct/array wrapping can collapse the two address materialisations. Log: tmp/grind/motion_Close/s9/declsweep.log.

- [s9] With F10 (s8) and F11 (s9) both dead, EVERY input to cc1 outside the function body proper is measured codegen-inert for motion_Close: signature, inlining, callee structure, TU order, and now declarations. The only remaining variable is the body, and the body is bounded below at 13 by three backend-level disproofs (H1 REG_PARM_STACK_SPACE, F5 ascending hard-reg scan, F7a mips.c:4680 save order + F7b delay-slot eligibility).

- [s9] F4b ADVANCED - floor 13 now reachable with a TWO-level do-while(0) wrap instead of three, by changing the loop construct. A `goto` loop emits no NOTE_INSN_LOOP_BEG/END for the walk, so flow.c counts every reference raw (p 4, count 5) instead of loop-weighted (p 10, count 8), and the margin p must close shrinks accordingly. Ladder on the goto chassis: depth 0 p 4/8 = 10000 score 20; depth 1 p 5/8 = 12500 score 20; depth 2 p 6/8 = 15000 -> $s0 score 13. A plain `while` loop behaves like the do-while loop (20/20/20/13), so the win is specifically the absence of loop notes.

- [s9] The depth-2 requirement on the goto chassis is idiom-independent: `*p++`, `p[0]` + separate bump, `--count`, and decrement-before-call all give 20/20/13 for depths 0/1/2 (call-through `p[-1]()` is worse throughout at 21/21/16). Declaration order of the two locals is inert at every depth, so this win - unlike session 3's - does not depend on global.c's allocno-number tie-break.

- [s9] Depth <= 1 is structurally out of reach with the outer guard intact: p assigned first (required for the target's address-materialisation order) leaves count at 5 raw refs / live_length 7 = 14285 and p at 5/8 = 12500, so p is behind on live_length and needs 6 weighted refs. The one chassis that drops count to 4 refs - a top-tested `goto test;` loop, which deletes the outer `if (count != 0)` guard - lets p win at depth 1 but scores 14, since the target has that guard.

- [s9] candidate.c replaced with the two-level-wrap goto-loop form (score 13, build_insns 25, re-measured this session). The superseded three-level do-while-loop form is NOT disproven - it is dominated at equal score by a lighter policy device - and its mechanism record is preserved in candidate.c's header and in rejected/pfirst-wrap-depth-1-and-2-insufficient.c.

- [s9] src/ings2.c was restored to HEAD after every sweep; the tree is unmodified and the oracle build is untouched. No candidate-ready is claimed: H1 bounds the entire family away from distance 0.

- [s9] Ten declaration forms of the three touched globals — array-typed table, array-typed count, both arrays, `extern char` count, `void *` count, const-qualified table, u32 guard, reversed extern order — all score 13 with build_insns 25 and the byte-identical allocno table (p 10/8 = 37500 -> $s0; count 8/7 = 34285 -> $s1). Log: tmp/grind/motion_Close/s9/declsweep.log.

- [s9] With F10 (s8) and F11 (s9) both dead, EVERY input to cc1 outside the function body proper is now measured codegen-inert for motion_Close: signature, inlining, callee structure, TU order, and declarations. The only remaining variable is the body, and the body is bounded below at 13 by three backend-level disproofs (H1 REG_PARM_STACK_SPACE; F5 ascending first-free hard-reg scan; F7a mips.c:4680 unconditional high-to-low save emission plus F7b delay-slot eligibility).

- [s9] A `goto` loop emits no NOTE_INSN_LOOP_BEG/END for the walk, so flow.c counts the function's references raw (p 4, count 5) instead of loop-weighted (p 10, count 8). Ladder on that chassis: depth 0 p 4/8 = 10000 score 20; depth 1 p 5/8 = 12500 score 20; depth 2 p 6/8 = 15000 -> $s0 score 13. A plain `while` loop behaves like the do-while loop (20/20/20/13), so the win is specifically the absence of loop notes, not the branch shape.

- [s9] The depth-2 requirement is idiom-independent: `*p++`, `p[0]` + separate bump, `--count`, and decrement-before-call all give 20/20/13 for depths 0/1/2; call-through `p[-1]()` is worse throughout at 21/21/16. Declaration order of the two locals is inert at every depth, so this win does not depend on global.c's allocno-number tie-break.

- [s9] Depth <= 1 is structurally out of reach with the outer guard intact: p assigned first (required for the target's address-materialisation order) leaves count at 5 raw refs / live_length 7 = 14285 and p at 5/8 = 12500 — p is behind on live_length, so it needs 6 weighted refs. The one chassis dropping count to 4 refs (a top-tested `goto test;` loop, which deletes the outer `if (count != 0)` guard) lets p win at depth 1 but scores 14, since the target has that guard.

- [s9] candidate.c replaced with the two-level-wrap goto-loop form, re-measured this session at score 13 / target_insns 26 / build_insns 25. The superseded three-level do-while-loop form is NOT disproven — it is dominated at equal score by a lighter policy device — and its mechanism record is preserved in candidate.c's header and in rejected/pfirst-wrap-depth-1-and-2-insufficient.c.

- [s9] No candidate-ready is claimed and none is possible in this family: H1 is a backend-level disproof (REG_PARM_STACK_SPACE is the compile-time constant 16 on every call-expansion path), so no pure-C body containing a call can reach the target's zero-byte outgoing-argument area.

- [s9] src/ings2.c was restored to HEAD after every sweep; the working tree carries only the motion_Close ledger updates plus the untracked metrics append. The oracle build is untouched.

## Session 10 (synthesis, 2026-08-12)

### THE HEADLINE: the C-side search space for motion_Close is now closed, and the closure is a measurement rather than an inference

Session 10's mandate was synthesis: re-read the whole ledger, merge it into one
attack, and reset the frontier. Two things were measured on the way, and both
tightened the packet rather than opening anything.

1. **F12 KILLED.** The loop-CONSTRUCT surface — the one input session 9 proved
   was a real lever — is exhausted. Six chassis x three wrap depths, every cell
   read with the instrumented cc1: `for(;;)`+break, `while(1)`+break,
   `for(;;)`+continue and `for (; count != 0; )` all produce the byte-identical
   allocno table at every depth (count 8 weighted refs / live_length 7 = 34285;
   p 7/8 = 17500 at depth 0, 8/8 = 30000 at 1, 9/8 = 33750 at 2) and all score
   20 at depths 0-2. They are the do-while-LOOP family exactly: to flow.c, a
   `break` exit, a `continue` back edge, a middle-clause test and a bottom test
   are the same thing — one NOTE_INSN_LOOP_BEG/END pair around the same body,
   hence the same references at the same weight. Only the `goto` loop escapes
   the note, and it alone reaches 13, at depth 2, reproducing session 9 to the
   digit. The outer once-through `for (i = 0; i < 1; i++)` is worse on every
   axis (a third allocno in $s2, 30 emitted instructions against 25, best cell
   18) as well as being a construct the role prompt names as NOT sanctioned by
   the do-while(0) carve-out.

2. **The s7 residual table verified against the CURRENT candidate.** The table
   was derived on the three-level do-while chassis that session 9 superseded.
   Equal scores do not prove equal residual composition, so the current
   candidate chassis' emitted stream was diffed against the stream the table was
   written from (`tmp/grind/motion_Close/s5b/wsA/_base.txt`). They are the same
   25 instructions in the same order with the same registers, the same
   `.frame $sp,32,$31 # vars= 0, regs= 3/0, args= 16`, the same filled beqz
   delay slot and the same descending save order. The table transfers verbatim.

### The merged attack, stated once

Ten sessions have varied, and measured dead, every class of input to cc1 that
this function has:

| input class | axis | verdict |
|---|---|---|
| body: statement order | s2 | the ONE lever that ever moved anything (20 -> 17) |
| body: register roles via allocno priority | s3, s4, s5 | moved 17 -> 16 -> 13; bottomed out |
| body: loop / pointer idiom | s2, s3, s9 | free variable — inert |
| body: loop CONSTRUCT | s9 (lever), s10 (exhausted) | goto chassis minimal at wrap depth 2 |
| whole-TU shape, signature, inlining | F10, s8 | codegen-inert |
| global declarations | F11, s9 | codegen-inert (all ten forms) |
| compiler flags | frozen | not an available axis |

and the 13 points that remain are, in full:

| points | bucket | disproof |
|---|---|---|
| ~6.5 | frame size + six save/restore offsets | H1 — REG_PARM_STACK_SPACE is a compile-time 16 applied by MAX on every call-expansion path (s1 census, s7 backend) |
| 5 | $v0 where the target uses $t0 | F5 — ascending first-free hard-reg scan, no MIPS REG_ALLOC_ORDER (s5b) |
| ~1.5 | ascending save order + empty beqz delay slot | F7a mips.c:4680 unconditional high-to-low emission (s6, s8); F7b reorg.c must fill a slot with four eligible insns before it, 0/1788 counterexamples (s6) |

There is no unexplained instruction and no unexplored C-side surface. The
function's remaining gap is not a hard search problem; it is a statement about
what GCC 2.7.2's MIPS backend can emit at all.

### What this session deliberately did NOT do

It did not dispose of the function. The mandated modality was `synthesis`, and
exhaustion is the driver's call, not a session's: the frontier reset below marks
motion_Close as a fully determined escalation candidate and leaves the routing
to the driver. When `escalation` modality is assigned, the packet is already
written and both endgame-lock gates FAIL (scan_hand_coded tier LOW 0/8 with
S1/S2/S6 negative — a measured failed gate; and no coercion/spelling family with
a citable SOTN-master precedent addresses a frame-layout / register-scan-order
residual), so under the owner's 2026-07-27 standing auto-ruling the terminal
entry is REFUSED / OWNER-ACCEPTED INCOMPLETE with nothing pending on the owner.

### Session-10 artifacts

- `tmp/grind/motion_Close/s10/f12sweep.py` + `f12sweep.log` — the 18-cell
  loop-construct x wrap-depth sweep, with the per-allocno table for every cell.
- `tmp/grind/motion_Close/s10/f12_*.s` — emitted assembly per cell (18 files);
  `f12_G_goto_tail_d2.s` is the current candidate chassis and is what the
  residual-table verification was run against.
- `memory/grind/motion_Close/rejected/f12-loop-construct-surface-inert.c`.

### Tree state at end of session 10

`src/ings2.c` restored to HEAD by the sweep's `finally` block and verified clean
with `git status --porcelain` (only ledger files and `metrics/events.jsonl`
modified). No build-pipeline file was touched. Floor unchanged at 13;
`memory/grind/motion_Close/candidate.c` still holds the session-9 two-level
goto-chassis form, now with a session-10 note recording that its emitted stream
was verified against the s7 residual table.

- [s10] F12 is dead: four real loop constructs (for(;;)+break, while(1)+break, for(;;)+continue, for(; count != 0; )) produce the identical allocno table to the do-while/while family at every wrap depth 0-2 and all score 20; to flow.c they are one NOTE_INSN_LOOP_BEG/END pair around the same body, so the exit spelling is codegen-inert.

- [s10] The goto chassis is unique on the whole loop-construct surface, not merely the best of the three constructs session 9 tried: it is the only one that emits no loop note for the walk, and it alone reaches 13 (at wrap depth 2, p 6 raw refs / live_length 8 = 15000 vs count 5/7 = 14285).

- [s10] An outer once-through `for (i = 0; i < 1; i++)` around the walk is strictly dominated as a weighting device: it adds a third allocno ($s2), grows the function from 25 to 30 emitted instructions and scores 18 at best - and it is an unsanctioned construct, so it could not be proposed even had it won.

- [s10] The s7 residual table now provably describes the banked candidate.c form (the two-level goto chassis), not just the superseded three-level do-while chassis: the emitted streams are identical, so all 13 residual points remain attributed 5 to F5, ~6.5 to H1 and ~1.5 to F7a/F7b with nothing unexplained.

- [s10] Merged closure statement for the next session: every class of input to cc1 that this function has - body statement/declaration order, register-role devices, pointer/loop idiom, loop construct, whole-TU shape and signature (F10, s8), global declarations (F11, s9) - is now measured, and the only lever that ever moved the floor bottoms out at 13.

- [s10] Both endgame-lock AND-gates are already measured FAILED for this function: scan_hand_coded returns tier LOW 0/8 with S1/S2/S6 all negative (s6, and the same for sibling func_80083794), and no coercion/spelling family with a citable SOTN-master precedent addresses a residual made of frame layout and hard-register scan order.

- [s10] src/ings2.c was restored to HEAD by the sweep's finally block and verified clean with git status --porcelain; no build-pipeline file was touched this session.
