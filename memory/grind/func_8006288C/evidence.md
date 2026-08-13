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


## Session 2 (structural, 2026-08-13) - floor 2 -> 0  (MATCH)

### The closing lever: store first, constant holder second
s1's H6 swept the constant holder (`new_var = 1;`) across all 8 init-block
positions but held `D_800F1138 = 1;` FIXED at index 6. Every holder position
that was early was therefore also BEFORE the store, and cse folded the two
constant-1 materialisations (`sw $t3,D_800F1138`), dropping the function to 41
instructions. The cell H6 never covered is **store FIRST, holder SECOND**: the
holder is then still after the store (no fold, 42 insns kept) while its RTL
LUID sits near the top of the init block.

    D_800F1138 = 1;
    i = 0;
    one = 1;
    flag_p = &D_800F0C04;
    off_s16 = 0;
    off_s32 = 0;
    src_a = (s32 *)D_800A347C;
    src_b = (u16 *)D_800A3478;
    do { mask = one << i; ... } while (i < 6);

This emits the target's exact init block -
`addu $a0,$zero,$zero | addiu $t3,$zero,1 | lui $t2 | addiu $t2 |
addu $a1,$zero,$zero | addu $v1,$zero,$zero | lw $t0 | lw $t1 |
addiu $v0,1 | lui $at | sw` - and the honest sandbox
(`--disable all`, 6 rules dropped, 332 cheat-asm insns stripped) prints
**score 0, build_insns 52, target_insns 52**.

Note `i = 0;` must come BEFORE `one = 1;` (variant `a_storefirst_one1`, holder
first, emits `addiu $t3` at slot 1 and `addu $a0` at slot 2 - inverted), and
the holder must be at init index 2, not 3 (`a_storefirst_one3`, holder after
`flag_p`, emits it at slot 4).

### The literal-1 axis is dead (10/10 measured)
Sweep set 6 (`tmp/grind/func_8006288C/s2/variants6.py`) compiled `mask = 1 << i`
with the `D_800F1138 = 1;` store at each of the 7 init-block positions, plus a
`while (1) { ... break; }` spelling, plus `i` initialised last, plus the whole
init block reversed. Every one of the 10 parks `addiu $t3,$zero,1` at
init-block slot 6. With a literal the shift base is loop-invariant, loop.c
hoists its `(set reg 1)` into the TAIL of the preheader, and sched.c's first
pass places it there; no ordering of the other statements moves it. So the
constant holder is load-bearing and there is no no-holder form.

### The declared-type axis is dead (frontier item 2 from s1)
Sweep set 5 axis B: `i`, `off_s32`, `off_s16` and `mask` re-declared
`unsigned int` (individually and all together) are **codegen-identical** to the
`s32` base - same 9 registers, same instruction count, same init-block order.
`short` spellings are strictly worse: `short off_s16/off_s32` costs 2 extra
instructions and drops to 7/9 registers; `short i` costs 3 extra and drops to
4/9. allocno_size never changes because GCC 2.7.2 promotes all of these to
SImode pseudos. Axis closed.

### Tooling added this session
`tmp/grind/func_8006288C/s2/sweep2.py` extends the s1 rig with a **prologue
order** score (the s1 residual was a slot, not a register name): it expands
cc1's `la`/`li`/`move` macros and the `-G0` `lw $8,SYM` macro form the way the
assembler does, and prints the emitted `(mnemonic, dest-reg)` sequence so a
variant's init block can be compared against the target's directly.
`variants5.py` (16 variants: init-order x holder-position, plus the type axis)
and `variants6.py` (10 literal-1 variants) are the sets.

- [s2] Applying the s1 candidate to src/text1b.c reproduces the s1 floor exactly: sandbox --disable all score 2, build_insns 52, target_insns 52. (The s1 form was NOT in the tree at session start; HEAD carried the pre-grind label+goto form at distance 23.)

- [s2] MATCH: moving `D_800F1138 = 1;` to init-block position 1 and the constant holder to position 3 (immediately after `i = 0;`), with `mask = one << i`, gives honest sandbox distance 0 with 52/52 instructions.

- [s2] The holder's init-block position is exact: index 1 (before `i = 0;`) inverts slots 1 and 2; index 3 (after `flag_p`) emits the constant at slot 4. Only index 2 matches.

- [s2] The literal-1 (no-holder) axis is exhausted: 10 variants - the store at each of 7 init positions, a while(1)/break loop spelling, `i` initialised last, and the init block reversed - all park `addiu $t3,$zero,1` at init-block slot 6.

- [s2] The declared-type axis is exhausted: u32 spellings of i/off_s32/off_s16/mask are codegen-identical to s32 (GCC 2.7.2 promotes them all to SImode pseudos, so allocno_size never changes); `short` spellings cost 2-3 extra instructions and lose 2-5 register assignments.

- [s2] The function still carries its 6 regfix rules; this session did not touch regfix.txt. The 0 is the cheat-invisible score (rules_dropped: 6, cheat_asm_stripped: 332), so rule retirement is the operator's normal `retire` step, not a dependency of the C form.
