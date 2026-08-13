# EVIDENCE — func_80033DF4 (src/code6cac_b.c)

## Session 1 (2026-08-13, modality: recon) — floor 23 → 0

### Baseline
- `sandbox func_80033DF4 --disable all` at HEAD: **score 23**, `target_insns 124`,
  `build_insns 124`. Instruction COUNT was already exact at HEAD, so the entire gap
  was register-assignment + scheduling-position, never a missing/extra-insn problem.
- HEAD carried 8 `register T x asm("$N")` pins (state/v1, tableIndex/a1, and two
  v0/v1/a0 triples in the if-branch, plus v0/v1/a0/a2/table in the else-branch) and
  3 asmfix.txt rules (asmfix.txt:17-19 — a `delete_between` + a ~50-instruction
  `insert_before` blob that hand-writes the entire else-branch, + a `replace_first`).
  All are score-inert under `--disable all` (`rules_dropped: 3`,
  `cheat_asm_stripped: 346`).
- Target asm: `asm/funcs/func_80033DF4.s`, 124 instructions, one `jal func_80033D38`,
  frame 0x18 with only `$ra` saved.

### Where the 23 lived
A normalized target-vs-build instruction diff (tool: `tmp/grind/func_80033DF4/s1/dif.py`,
objdumps `tmp/sandbox/func_80033DF4/code6cac_b.o` and parses `asm/funcs/*.s`) showed
indices 0–71 (the `state == 0x64` branch) already IDENTICAL with the pins stripped —
i.e. every pin in the if-branch was inert. All 23 differing instructions sat in the
else-branch, indices 72–119.

### Fact 1 — the two lookup tables are 2D with stride 5
Target computes, ONCE, `$a0 = flag*5` (`sll $a0,$v0,2; addu $a0,$a0,$v0`) and reuses
that same `$a0` for BOTH table reads:
  - `addu $v1,$a0,$v1; addu $v1,$v1,$a2; lbu $v0,0($v1)`   → D_8008EC24
  - `addu $a0,$a0,$v0; addu $a0,$a0,$a2; lbu $v1,0($a0)`   → D_8008E908
That is `T[flag][entry]` for `u8 T[][5]`, with GCC CSE-ing the row offset across the
two accesses. `D_8008E908` and `D_8008EC24` are referenced NOWHERE else in src/ or
include/ (they were declared file-locally at src/code6cac_b.c:88-89), so the decls
could be corrected to `extern u8 D_XXXXXXXX[][5];` without touching any other function.

### Fact 2 — flat `(&SYM)[i+j]` indexing loses because GCC folds the symbol
With the scalar decl and a fused index, GCC emits the folded addressing form
`lui $at,%hi(sym); addu $at,$at,$idx; lbu $rd,%lo(sym)($at)` (3 insns, no base GPR).
Target materializes the base into a real GPR (`lui/addiu` pair) and adds `row` then
`entry` to it. Switching the decls to `[][5]` and writing `T[row][entry]` changes the
association to `(base + row*5) + entry`, forces the base materialization, and dropped
the floor 23 → **11**. (Banked: `rejected/flat-scalar-extern-fused-index.c`.)

### Fact 3 — the last 11 were the base materialization's SCHEDULING position
At floor 11 the residual was a contiguous window, target indices 89–100. Target order:
```
88  lbu $v0,%lo(D_8008D538)($at)      <- first lookup lands
89  lui  $v1,%hi(D_8008EC24)          <- base materialized IN the load-delay window
90  addiu $v1,$v1,%lo(D_8008EC24)
93  lbu $v0,%lo(D_8008D9EC)($at)      <- second, dependent lookup
94  andi $a2,$a2,0xFF
95  sltiu $v0,$v0,0x1
```
Our build put `andi $a2,...` and the `$at` address arithmetic in that window and only
materialized the base AFTER the second lookup — which additionally swapped `$v0`/`$v1`
across the entire tail (that swap is most of the 11).

Declaring the table bases as named pointer locals at the TOP of the else-block
(`u8 (*ranks)[5] = D_8008EC24;` / `u8 (*moves)[5] = D_8008E908;`) gives the
lui/addiu pair an earlier LUID, sched1 then places it in the D_8008D538 load-delay
window, the $v0/$v1 assignment falls out correctly, and the floor goes **11 → 0**.
Measured negative for the same code WITHOUT the named locals: score 11
(banked: `rejected/2d-decl-no-named-base-local.c`). Declaring them mid-block (after a
`u8 chr = ...` split of the chained lookup) ALSO reaches 0, but the top-of-block
declaration is the more idiomatic spelling and needs no `chr` intermediate.

### Fact 4 — every register pin in this function was inert
Removing all 8 pins and rewriting both if-branch bodies with plain named locals
(`s32 *flags; s32 word; s32 mask;`) held the score at 0. The `tableIndex` variable no
longer has to be reused as the bitmask holder — a separate `mask` local produces the
same bytes. The `s32 *flags = &D_80106A50; word = *flags; ...; *flags = word | mask;`
read-modify-write spelling is retained (it is what produces target's single
`lui/addiu $a0` base + `lw 0($a0)` / `sw 0($a0)` pair) and is the exact shape
sanctioned by `.claude/rules/pointer-rmw-global-sanctioned.md:36`.

### Final state at end of session 1
`sandbox func_80033DF4 --disable all` → **score 0**, 124/124 insns, ZERO register pins,
ZERO inline asm, edits in place in `src/code6cac_b.c`. The 3 asmfix.txt rules
(asmfix.txt:17-19) are now dead weight and must be retired by the operator/driver —
they are outside this session's allowed surface.
