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
