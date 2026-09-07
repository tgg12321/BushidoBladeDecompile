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
