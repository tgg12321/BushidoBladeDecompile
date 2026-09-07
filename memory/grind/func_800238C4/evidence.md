# Evidence bank — func_800238C4

## s1 (2026-09-07, recon) — chassis re-measure

CHASSIS: asm-until-matched, 0 rules, HEAD 2026-09-07. The banked `candidate.c`
(inherited from the pre-relaunch ledger seed, header claims floor 3) was applied
to `src/code6cac.c` in place of `INCLUDE_ASM("asm/funcs", func_800238C4);` and
measured with `sandbox func_800238C4 --disable all`:

    {"score": 3, "target_insns": 219, "build_insns": 219, "scorable": true,
     "rules_dropped": 0, "cheat_asm_stripped": 21}

FLOOR 3 CONFIRMED on the current chassis. Instruction COUNT is exact (219/219),
so the residual is pure register seating, not a missing/extra insn, and no
control-flow or expression-shape work remains. The migration_pin's floor 48 is
the retired pre-migration chassis number and is superseded.

## OBJECT MODEL: the two flagged globals

The brief flags `D_800A3758` with a SPLIT-AGGREGATE signal (sub-symbol +8 of
`g_leaf_random_buffer` @800A3750). Measured verdict, per symbol:

- **D_800A3758 — MATCHES (measured score contribution 0).** Declared
  `extern u8 D_800A3758;` in `include/code6cac.h`; the target's only reference in
  this function is a BYTE store, `lui $at, %hi(D_800A3758)` /
  `sb $v1, %lo(D_800A3758)($at)` (asm/funcs/func_800238C4.s:209-210). The current
  C `D_800A3758 = ((u16 *)arg0)[2];` reproduces exactly that pair; in the
  normalized objdump diff the only difference on those two lines is objdump's
  `lui at,0` vs `%hi(...)` printing of an *unlinked* R_MIPS_HI16/LO16 reloc, which
  engine/score.py does not count (sandbox score is 3, and the three counted
  differences are all at insn indices 179/180/182 — see the residual section).
  The declared byte shape therefore already produces the target bytes: NO
  declaration fix is indicated for this function, and merging the per-word splat
  pieces into `g_leaf_random_buffer` cannot lower this floor. (Prong (c) of the
  aggregate-merge family also requires keeping the `D_800A3758` row alive for the
  still-INCLUDE_ASM siblings func_8001FBE8 / func_80022580, so a merge here would
  be a cross-function refactor with zero byte effect on the target.)
- **D_800A3769 — MATCHES (measured score contribution 0).** Declared
  `extern u8 D_800A3769;`; target reference is the byte store at
  asm/funcs/func_800238C4.s:207-208, reproduced exactly by
  `D_800A3769 = s1 < 0x400;`. Same reloc-printing caveat, same zero contribution.

No MISMATCH and no MISMATCH-unmeasured rows: both flagged symbols are measured.

## The floor-3 residual, mapped exactly

Normalized objdump-vs-target diff (tool banked at
`tmp/grind/func_800238C4/s1/adiff2.py`; it canonicalises `li`/`move`/`b`/`nop`
macro spellings, decimal-vs-hex immediates and branch targets, so only real
codegen differences survive). Exactly three instructions differ, all in the
basic block at target label `.L80023B90` (asm/funcs/func_800238C4.s:199-202):

    idx  TARGET                      BUILD
    179  lw    $v0, 0($s0)           lw    $v1, 0($s0)
    180  addiu $v1, $zero, 2         addiu $v0, $zero, 2
    182  sh    $v1, 0x286($v0)       sh    $v0, 0x286($v1)

i.e. a straight $v0/$v1 seat SWAP between the parent-pointer load and the
constant 2, on the C statement
`*((s16 *) ((*((u8 **) arg0)) + 0x286)) = 2;` (src/code6cac.c:3058 with the
candidate applied). Everything else in the 219-instruction body is identical.

## MECHANISM — measured, not hypothesised: local-alloc qty priority

Pass attribution was done by reading the dumps, not by guessing. The RTL at
`.lreg` (tmp/grind/func_800238C4/dumps/code6cac.lreg:36352-36372) is:

    (insn 522 ... (set (reg:SI 211) (mem:SI (reg/v:SI 72))))          ; parent ptr
    (insn 524 ... (set (reg:HI 212) (const_int 2)))                   ; the 2
    (insn 526 ... (set (mem:HI (plus (reg:SI 211) (const_int 646)))
                       (reg:HI 212)))                                 ; the sh
    (jump_insn 528 ...)

Both pseudos are born in this block and die on insn 526, so the seat is decided
by local-alloc, not by global-alloc or reload. The instrumented cc1
(tools/gcc-2.7.2/cc1, BB2_QTY_DEBUG=1; raw capture
tmp/grind/func_800238C4/s1/qtydbg.txt:1237-1238) prints the actual quantities:

    QTYDBG blk=41 ord=0 qty=1 reg1=212 birth=6 death=8 refs=2 got=2
    QTYDBG blk=41 ord=1 qty=0 reg1=211 birth=4 death=8 refs=2 got=3

Read against `qty_compare_1` (tools/gcc-2.7.2/local-alloc.c:1660), whose priority
is `floor_log2(n_refs) * n_refs * size / (death - birth) * 10000`:

    const 2 (reg 212): 1*2*1 / (8-6) * 10000 = 10000  -> ord 0, takes $2
    pointer (reg 211): 1*2*1 / (8-4) * 10000 =  5000  -> ord 1, takes $3

The build's seating is therefore fully explained: the constant's quantity is
strictly SHORTER-LIVED than the pointer's (it is born one insn later and dies on
the same insn), so it always wins the priority sort and takes the first free
register. The target wants the opposite seating, which requires the POINTER
quantity to sort first.

Only two inputs can produce that, both readable straight off the formula:
  (a) invert the birth order — materialise the constant BEFORE the pointer load,
      making the pointer the short-lived quantity (10000 vs 5000). sched2 runs
      after allocation and would still be free to hoist the `lw` back ahead of the
      `li` to cover the load-delay slot, which is exactly the target's emission
      order (`lw`, `li`, `j`, `sh`-in-delay-slot).
  (b) raise the pointer's `n_refs` to 4 — `floor_log2(4)*4*1 / 4 = 20000` beats
      10000. The target uses the parent pointer exactly once, so there is no
      natural second/third/fourth reference available.
Note that a TIE does not help: `qty_compare_1`'s tie-break is `*q1 - *q2`, the
quantity number, which is assigned in birth order, so whichever quantity is born
first wins a tie — equalising the spans by moving the constant earlier makes the
constant qty 0 and hands it the tie.

## Probes measured this session (all against the floor-3 chassis)

| probe | form | score | verdict |
|---|---|---|---|
| P1 | pointer local at the use site: `s16 *parent = *(s16 **)arg0; parent[0x143] = 2;` | 3 | NO-OP, residual asm bit-identical |
| P3 | duplicate the parent store into both arms of `if (s1 < 0x400)` | 9 | WORSE by 6 — no cross-jump re-merge |
| P7 | named constant intermediate: `s16 two = 2; ... = two;` | 3 | NO-OP, temp is constant-propagated away before lreg |

All three are banked under `rejected/` with their measured scores in the header.
P7 is the important negative: it was the direct spelling of lever (a), and it
fails not at allocation but earlier — cse/combine folds the constant back into
the store and deletes the pseudo, so expand order is untouched (the residual asm
is bit-identical to candidate.c's). Any plain `= 2` holder local, in any integer
width, will be folded the same way.

The inherited `rejected/parent-ptr-hoisted-before-ifelse-score6.c` is consistent
with this model and now has an explanation: hoisting `u8 *parent` above the
`if (s1 < 0x400)` makes the pointer live ACROSS blocks, which removes it from
local-alloc entirely and hands it to global-alloc (+3 over the floor).

- [s1] OBJECT MODEL: D_800A3758 - MATCHES (measured score contribution 0). The brief flags it SPLIT-AGGREGATE (sub-symbol +8 of g_leaf_random_buffer @800A3750), but the target's only reference in this function is a byte store, `lui $at, %hi(D_800A3758)` / `sb $v1, %lo(D_800A3758)($at)` (asm/funcs/func_800238C4.s:209-210), which the existing `extern u8 D_800A3758;` declaration plus `D_800A3758 = ((u16 *)arg0)[2];` reproduces exactly. In the normalized objdump diff the only difference on those lines is objdump printing an unlinked R_MIPS_HI16/LO16 reloc as `lui at,0`, which engine/score.py does not count - the sandbox score is 3 and all three counted differences sit at instruction indices 179/180/182, in a different basic block. D_800A3769 - MATCHES (measured score contribution 0), same byte-store shape at asm/funcs/func_800238C4.s:207-208, reproduced by `D_800A3769 = s1 < 0x400;`. No MISMATCH and no MISMATCH-unmeasured rows: both flagged symbols are measured. Consequence: NO declaration fix is indicated for this function, and an aggregate merge of the per-word splat pieces into g_leaf_random_buffer cannot lower this floor (it would also be a cross-function refactor, since prong (c) requires keeping the D_800A3758 row alive for the still-INCLUDE_ASM siblings func_8001FBE8 and func_80022580).

- [s1] CHASSIS RE-MEASURE: the inherited candidate.c body, applied over the INCLUDE_ASM line in src/code6cac.c, measures score 3 with target_insns 219, build_insns 219, scorable true, rules_dropped 0 on the current chassis. Floor 3 is CONFIRMED and the migration_pin.json floor of 48 is the retired pre-migration number, now superseded. Instruction count is exact, so no control-flow, expression-shape or missing/extra-instruction work remains - the whole residual is register seating.

- [s1] RESIDUAL MAPPED EXACTLY: exactly three instructions differ, all inside the four-instruction basic block at target label .L80023B90 (asm/funcs/func_800238C4.s:199-202). Target: `lw $v0, 0($s0)` / `addiu $v1, $zero, 2` / `sh $v1, 0x286($v0)`. Build: the same three with $v0 and $v1 exchanged. The source of all three is the single C statement `*((s16 *) ((*((u8 **) arg0)) + 0x286)) = 2;`. The normalized diff tool is banked at tmp/grind/func_800238C4/s1/adiff2.py - it canonicalises li/move/b/nop macro spellings, decimal-vs-hex immediates and branch targets so only real codegen differences survive.

- [s1] PASS ATTRIBUTION BY DUMP, NOT BY GUESS: the .lreg RTL (tmp/grind/func_800238C4/dumps/code6cac.lreg:36352-36372) shows both pseudos born and dying inside this block - (insn 522) sets reg:SI 211 from mem (the parent pointer), (insn 524) sets reg:HI 212 to const_int 2, (insn 526) is the HImode store through reg 211 + 646, and both carry REG_DEAD on insn 526. Because both live and die in one block the seat is decided by LOCAL-alloc; global-alloc and reload are not involved.

- [s1] MECHANISM MEASURED WITH THE INSTRUMENTED CC1: running tools/gcc-2.7.2/cc1 with BB2_QTY_DEBUG=1 prints the actual quantity records (tmp/grind/func_800238C4/s1/qtydbg.txt:1237-1238): `blk=41 ord=0 qty=1 reg1=212 birth=6 death=8 refs=2 got=2` and `blk=41 ord=1 qty=0 reg1=211 birth=4 death=8 refs=2 got=3`. Read against qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1660), whose priority is floor_log2(n_refs)*n_refs*size/(death-birth)*10000: the const-2 quantity scores 1*2*1/2*10000 = 10000 and the pointer quantity scores 1*2*1/4*10000 = 5000, so the constant sorts first and takes the first free register ($v0) while the pointer takes $v1. This fully explains the build's seating and is the exact inverse of what the target needs.

- [s1] THE TWO LEVERS THE FORMULA ALLOWS, both read straight off qty_compare_1: (a) invert the birth order so the constant is materialised before the pointer load, which makes the POINTER the shorter-lived quantity (10000 vs 5000) and hands it the first free register - sched2 runs after allocation and would still be free to hoist the lw back ahead of the li to cover the load-delay slot, which is precisely the target's emission order (lw, li, j, sh-in-delay-slot); or (b) raise the pointer's n_refs to 4, since floor_log2(4)*4*1/4 = 20000 beats 10000 - but the target references the parent pointer exactly once, so there is no natural second reference available. A TIE does not help: qty_compare_1's tie-break is *q1 - *q2, the quantity number, assigned in birth order, so whichever quantity is born first wins a tie.

- [s1] The inherited rejected form parent-ptr-hoisted-before-ifelse-score6.c (score 6) is now explained rather than merely recorded: hoisting `u8 *parent` above the `if (s1 < 0x400)` makes the pointer live across basic blocks, which removes it from local-alloc entirely and hands it to global-alloc.

- [s1] tools/ra_solver and inverse_compose.py classify have NOT been run on this residual. It is precisely the shape they exist for - a two-quantity local-alloc seat assignment whose priority inputs (refs 2/2, spans 4/2, class GENERAL_REGS) are now fully measured - so a REACHABLE/FORECLOSED verdict with a ranked lever vector is available cheaply to the next session and is worth more than another hand-spelled probe.

## s2 (2026-09-07, structural) — MATCHED, floor 3 -> 0

CHASSIS RE-MEASURE: the inherited s1 candidate.c body re-applied over the
INCLUDE_ASM line measured score 3 / 219 target insns / 219 build insns / 0 rules,
so the s1 floor of 3 was reproduced exactly before any probe ran.

SIBLING SWEEP: the only sibling named by this ledger, func_80022580 (src/code6cac.c,
still active), has NO candidate.c — its ledger is a from-scratch recon of a 619-insn
init function that shares no basic block, no local and no expression with this target
(its only overlap is the unrelated global D_800A3758). There was nothing to transplant,
so no transplant hypothesis was banked.

### The quantity arithmetic, closed

s1 measured the two competing quantities in block 41 with the instrumented cc1 and
attributed the seat swap to local-alloc's priority sort. s2 re-read the allocator and
found the full decision procedure, which is narrower than s1 recorded:

  * block_alloc does NOT qsort when the block has few quantities. For next_qty <= 3 it
    uses a hand-rolled sort (tools/gcc-2.7.2/local-alloc.c:1539-1563).
  * For next_qty == 2 that sort is a single `if (qty_compare (0, 1) > 0) EXCHANGE (0, 1);`.
    qty_compare (local-alloc.c:1640) returns pri2 - pri1, so the exchange happens only when
    the SECOND quantity's priority is STRICTLY greater. A TIE therefore leaves qty 0 —
    the earlier-born quantity — allocated first. (s1's note that "a tie does not help"
    was derived from the qsort/qty_compare_1 path, which this block never reaches.)
  * BB2_SUGG_DEBUG (new capture, tmp/grind/func_800238C4/s2/qtydbg.txt:2431-2432) shows both
    quantities carry ncopysugg=0 nsugg=0 size=1 altclass=0, so the suggested-register pass
    that runs before the priority pass is inert here and the seat is decided by
    floor_log2(refs)*refs*size/(death-birth) alone.

With the pointer at refs 2 / span 4 (pri 5000) and the constant at refs 2 / span 2
(pri 10000), the constant is allocated first and takes $v0. The three ways to invert that
are exhaustively enumerable from the formula, and s2 measured all three:

  (a) make the constant born earlier — KILLED at the SCHEDULER, not at cse. A standalone
      4-line reproduction (tmp/grind/func_800238C4/s2/probe/) with a deliberately
      non-foldable value shows sched1 HOISTS the load above an earlier-emitted, ready,
      same-consumer def: the MIPS memory function unit gives a load a ready-delay of 2
      (tools/gcc-2.7.2/config/mips/mips.md:157-159) versus 1 for an arith def, and
      rank_for_schedule (tools/gcc-2.7.2/sched.c:2408) sorts on INSN_PRIORITY first, so the
      load's priority is strictly higher and it is always emitted first. Source-level
      ordering of the two defs cannot survive sched1.
  (b) raise the pointer's refs to 4 — no natural second reference exists; the target uses
      the parent pointer once.
  (c) lower the constant's priority — impossible for a two-quantity block: a live pseudo
      always has at least 2 refs (its def and its use), and both quantities die on the same
      insn, so the constant's span is always strictly shorter than the pointer's.

Nine address spellings and nine expression-context spellings of the statement were
measured in the standalone harness (batch.py / batch2.py / batch3.py): every one produced
refs 2/2 with births 2/4 and the constant in $2. Address spelling, pointer locals, index
form, struct form, enum constant, `1 + 1`, register storage class and store width are all
measured no-ops for this seat.

### The lever that worked — a THIRD quantity, and the hand-rolled sort's third comparison

The standalone harness found the exception: adding an unrelated statement AFTER the store,
in the same basic block, flips the seat (probe X2_after_unrelated). With three quantities
the case-3 arm of the hand-rolled sort runs

    if (qty_compare (0, 1) > 0) EXCHANGE (0, 1);
    if (qty_compare (1, 2) > 0) EXCHANGE (2, 1);
    if (qty_compare (0, 1) > 0) EXCHANGE (0, 1);   /* case-2 fall-through */

and those comparisons take LITERAL QUANTITY NUMBERS, not the contents of qty_order. When
the third quantity ties the constant at 10000, the second comparison is a no-op and the
third comparison UNDOES the first exchange, leaving the parent pointer at qty_order[0]. It
is allocated first, takes $v0, and the constant takes $v1 — the target seating.

The function already contained a statement that could legitimately live in that block:
the common tail `*(s32 *)(arg0 + 0x74) = *(s32 *)(arg0 + 0xBC);` that every arm except the
0x11 arm falls into. Duplicating it, with its control transfer, into the first arm
(`... = 2; *(s32 *)(arg0 + 0x74) = *(s32 *)(arg0 + 0xBC); goto skip_74;`) puts the 0xBC
load (refs 2, span 2, pri 10000) in block 41 after the store, and jump2 cross-jump
re-merges the duplicated copy with the shared one so the instruction count is unchanged.

MEASURED: `sandbox func_800238C4 --disable all` = score 0, target_insns 219,
build_insns 219, rules_dropped 0. Full build `verify-oracle` = ok, build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle. func_800238C4 byte-matches in pure C.

The construct is the sanctioned duplicated-statement-into-arms family in its 2026-08-06
control-transfer-tail scope, FAKE-annotated in the body with the mechanism and the
lever-exhaustion pointer; the reasoning is written out in self_vet.md.

### Precedent found in the shipped binary (independent confirmation of the model)

A corpus scan of all asm/funcs/*.s for `lw rP / addiu rC, zero, K / s? rC, off(rP)`
(tmp/grind/func_800238C4/s2/findpat.py) returned 53 instances. 51 of them seat the
constant in the LOWER register, exactly like the pre-fix build. The exception at
func_80065800.s:102-107 seats the pointer in $v0 — and there the constant 0x20 is stored
through TWO different pointers, which gives its quantity refs 3 and span 6 (pri 5000),
tying the first pointer's 5000 and handing the pointer the first register by the same
tie-keeps-qty-0 rule. The shipped binary therefore contains an independent instance of the
priority arithmetic derived here.
