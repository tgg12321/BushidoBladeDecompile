# Evidence bank — func_8006288C

## Session 1 (recon, 2026-08-13)

### Function shape (settled)
`func_8006288C` (src/text1b.c, 52 target instructions, canonical verdict **C**,
`pure-C distance 23 <= 50 — pure-C target`). Semantics: a 6-slot allocator.

    D_800F1138 = 1;
    for slot i in 0..5:
        if (D_800A3460 & (1 << i)) continue;       # slot taken
        copy 3 words  from *(s32*)D_800A347C into  &D_800F0FB8 + i*12
        copy 3 halfwords from *(u16*)D_800A3478 into &D_800F10A0 + i*8
        D_800A3460 |= (1 << i);
        ((s16*)&D_800F0C04)[i] = 0;
        break;
    return 1;                                       # ALWAYS 1, both exits

Both exit paths return the constant 1 (`jr $ra` / `addiu $v0,$zero,1` in the
delay slot). The three destination bases are addressed as
`sw $v0, %lo(SYM)($at)` after `lui $at,%hi(SYM); addu $at,$at,<byte-offset reg>`
— i.e. the C is a **byte-offset accumulator** (`*(s32*)((s32)&SYM + off)`),
NOT array indexing (see KILLED H2 below).

### The gap was 100% register assignment + one emission-order slot
Our build and the target both emit **52 instructions in the identical order**;
the whole distance-23 was register naming. The 6 regfix rules in regfix.txt
(`$5 <-> $8/$10/$7/$9/$6` plus `reorder 4,1,2,3 @ 1-4`) are exactly that
permutation plus the prologue-order rotation — they are pure paperwork over an
allocation difference, no manufactured instructions.

Target register assignment (extracted from asm/funcs/func_8006288C.s):

    i=$4(a0)   one=$11(t3)  flag_p=$10(t2)  off_s16=$5(a1)  off_s32=$3(v1)
    src_a=$8(t0)  src_b=$9(t1)  flagword=$7(a3)  mask=$6(a2)

### The allocation model (measured, not inferred)
The instrumented cc1 (`tools/gcc-2.7.2/cc1`, NOT `build/cc1`) with
`BB2_ALLOC_DEBUG=1` prints per-allocno `nrefs`, `livelen` and the
global.c `allocno_compare` priority
`floor_log2(nrefs)*nrefs/livelen * 10000` (see tools/gcc-2.7.2/global.c).
All 9 pseudos go through global_alloc; MIPS `reg_alloc_order` then hands out
$3,$4,$5,$6,$7,$8,$9,$10,$11 in descending priority order, so **the register
assignment is entirely determined by the priority sort**.

Baseline (pre-session in-tree form, distance 23):

    ord pseudo var        hardreg nrefs livelen pri
    0   77     off_s32    3        6     22     5454
    1   76     i          4        5     19     5263
    2   74     src_a      5        4     19     4210
    3   78     off_s16    6        6     31     3870
    4   75     src_b      7        4     24     3333
    5   73     flag_p     8        4     33     2424
    6   79     mask       9        3     15     2000
    7   81     flagword   10       3     16     1875
    8   72     one        11       2     30     666

Target requires the order `off_s32, i, off_s16, mask, flagword, src_a, src_b,
flag_p, one` — i.e. the three loop-body-local pseudos (off_s16, mask,
flagword) must outrank the three loop-invariant pointers (src_a, src_b,
flag_p).

### THE LEVER (CONFIRMED — distance 23 -> 2)
Spelling the loop as a real `do { ... } while (i < 6);` with an early
`goto out;` out of the taken-slot body, instead of the label+`goto loop_top`
form with the latch in an `else` arm, reproduces **all 9 target register
assignments exactly** (9/9) and drops the honest sandbox distance
**23 -> 2**. `while (1) { ... break; ... if (i >= 6) break; }` is codegen-
identical to the do-while form (also 9/9). A `for (i = 0; i < 6; i++)` loop
gets 7/9 (it swaps `i` and `off_s32` between $3 and $4).

Mechanism: the loop spelling changes where GCC places the latch block and the
loop notes, which changes every pseudo's `reg_live_length`, which reorders the
`allocno_compare` sort. This is ordinary program structure — the emitted code
is the same 52 instructions; nothing dead, nothing coerced.

### Residual (2 as of end of session 1)
The only remaining difference is the position of `li $t3, 1` (the `1` used as
the `sllv` shift base) in the init block:

    target: move a0,zero | li t3,1 | lui t2 | addiu t2 | move a1,zero | move v1,zero | lw t0 | lw t1 | li v0,1 | lui at | sw
    ours:   move a0,zero | lui t2   | addiu t2 | move a1,zero | move v1,zero | li t3,1 | lw t0 | lw t1 | li v0,1 | lui at | sw

i.e. our `li t3,1` lands at slot 5 instead of slot 1. Note the target
materializes the constant 1 **twice** in the prologue (`li t3,1` for the shift
base and `li v0,1` for the `D_800F1138 = 1` store) and a third time for the
return — cse did NOT fold them.

Measured constraint (sweep set 3, 8 variants): placing `new_var = 1;` anywhere
**before** the `D_800F1138 = 1;` statement makes cse fold the two constants
(`sw $t3, D_800F1138`) and the function drops to **41** instructions — wrong
shape. Only `new_var = 1;` **after** the store keeps 42 cc1 instructions, and
in that position the scheduler parks `li t3,1` at slot 5. So the target's
early-and-unfolded `li t3,1` cannot come from a plain `new_var = 1;`
assignment at any source position. It most likely comes from a construct that
creates the constant register **after cse2 runs** — e.g. loop.c hoisting the
invariant `1` out of `1 << i` into the loop preheader.

### Tooling built this session (reusable)
`tmp/grind/func_8006288C/s1/` holds a **standalone cc1 rig** that reproduces
the in-tree register allocation exactly (verified against the sandbox build),
so variants cost ~2 s each instead of a full text1b.c compile:
  - `head.c` — typedefs + externs
  - `sweep.py` — compiles each variant with the exact project cc1 flags,
    extracts the variable->hard-register map from the emitted asm, scores it
    against the target map, and (with `-v`) prints the ALLOCDBG priority table
  - `variants*.py` — the variant sets; `python3 sweep.py variants2`
Run under WSL: `bash tools/wsl.sh 'cd tmp/grind/func_8006288C/s1 && python3 sweep.py <set>'`

- [s1] canonical verdict C, pure-C distance 23 <= 50; 52 target instructions; 6 regfix rules ($5 <-> $8/$10/$7/$9/$6 plus reorder 4,1,2,3 @ 1-4) which are exactly a register permutation plus a prologue rotation — no manufactured instructions.

- [s1] Our build and the target emit the same 52 instructions in the same order; the entire pre-session distance-23 was register naming.

- [s1] Target register assignment (extracted from asm/funcs/func_8006288C.s): i=$4, one=$11, flag_p=$10, off_s16=$5, off_s32=$3, src_a=$8, src_b=$9, flagword=$7, mask=$6.

- [s1] Baseline ALLOCDBG priority table (pseudo/var/hardreg/nrefs/livelen/pri): 77 off_s32 $3 6/22/5454; 76 i $4 5/19/5263; 74 src_a $5 4/19/4210; 78 off_s16 $6 6/31/3870; 75 src_b $7 4/24/3333; 73 flag_p $8 4/33/2424; 79 mask $9 3/15/2000; 81 flagword $10 3/16/1875; 72 one $11 2/30/666. Target requires off_s16/mask/flagword to outrank src_a/src_b/flag_p.

- [s1] All 9 pseudos go through global_alloc (none are local_alloc), so allocno_compare fully determines the assignment.

- [s1] The instrumented cc1 with the BB2_ALLOC_DEBUG hook is tools/gcc-2.7.2/cc1, NOT tools/gcc-2.7.2/build/cc1 (build/cc1 is the clean one used by the Makefile).

- [s1] A standalone single-function cc1 rig reproduces the in-tree register allocation exactly (verified against the sandbox build), so a variant costs ~2 s instead of a full text1b.c compile.

- [s1] Current best form is in src/text1b.c (uncommitted) at distance 2 with 52/52 instructions; the only differing slot is the position of li $t3,1 in the init block (target slot 1, ours slot 5).

- [s1] The target materialises the constant 1 three times (shift base, the D_800F1138 = 1 store, the return value) and cse folds none of them.
