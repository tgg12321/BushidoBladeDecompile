# D_8008EB40 row-pointer alias: exhaustion and mechanism (laneH 2026-09-30)

Context: rev-judge FAILed func_800233AC / func_80023648 / func_800325E0 in round 1 because the
re-certified bodies still read D_8008EB40, D_800A310C, D_8008DA08 and g_sqrt_table_u8 through
scalar declarations. The orchestrator ruled the remedy: array declarations and an
all-consumer respell, dropping the second handles where possible; for D_8008EB40, the
row-pointer local with pointer-alias-fake-exception paperwork (option A).

## The object
asm/data/7D920.data.s dlabel D_8008EB40, 0x8008EB40..0x8008EB53 (0x14 bytes).
- As s16: FE00 0000 0200 | FC00 0000 0400 | FA00 0800 0600 | 0000.
- 3 x 3 s16 = 0x12 bytes; the last 2 bytes (0x0000) are word-alignment padding before
  D_8008EB54. Both readers index it [a0][a1] with a0, a1 in 0..2 (each is a pad-bit pair
  plus 0 or 1).
- Declaration: `extern s16 D_8008EB40[3][3];` in include/code6cac.h.

## Lever exhaustion (prerequisite 1)
Each spelling was measured object-level on all 33 TUs that include code6cac.h
(tmp/laneH/harness.py, edits.py stages), on top of the Judge and sqrt stages. The changed
object is code6cac_tu2.o in every case. Both functions are affected the same way.

| spelling | result |
|---|---|
| `row = D_8008EB40[a0_idx];` (2-D, no handle) | 1 differing |
| `a1_val = D_8008EB40[a0_idx][a1_idx];` (2-D, no handle, no row) | 1 differing |
| flat `s16 D_8008EB40[10]`, `row = D_8008EB40 + a0_idx * 3;` (no handle) | 1 differing |
| flat `s16[10]`, `s16 *tbl = D_8008EB40; row = tbl + a0_idx * 3;` (handle) | 0 differing |
| **2-D, `s16 (*tbl)[3] = D_8008EB40; row = tbl[a0_idx];` (handle; LANDED)** | 0 differing |

The diff in every handle-free form, in both functions: the row offset
(sll 1 / addu / sll 1 = a0 * 6) is emitted BEFORE lui/addiu %hi/%lo(D_8008EB40) instead of
after it. Register seats follow from that: v1 and a0 swap, and the sll is duplicated into
the bnez delay slot.

## Mechanism (prerequisite 2), dump-proven
Instrumented cc1 (tools/gcc-2.7.2/cc1), `-dr -ds -dl`, build flags. Script:
dumps/dump.sh; slicer: dumps/rtlslice.py; excerpt: dumps/rtl_slice.txt.
- **Handle-free** (`row = D_8008EB40[a0_idx]`), .rtl (expand):
  - insns 48/50/51 compute a0 * 2, * 3, * 6 into pseudos 85-87;
  - THEN insn 53 `(set (reg 88) (symbol_ref "D_8008EB40"))`;
  - insn 55 adds the two.
  - func_80023648 is the same: insns 66/68/69, then 71.
- **Row-pointer local** (`s16 (*tbl)[3] = D_8008EB40;`), .rtl:
  - insn 43 `(set (reg/v 81) (symbol_ref "D_8008EB40"))` is emitted at the declaration
    statement, BEFORE the a0 * 6 insns (50 onward);
  - func_80023648: insn 65 at `tbl = D_8008EB40;`, before 69/71.
- **Decision.** expr.c expand_expr, case PLUS_EXPR, EXPAND_SUM path (expr.c:5187-5246):
  - The address `&D_8008EB40[a0]` is PLUS(array base, a0 * 6). Operand 0 (the ADDR_EXPR) is
    expanded at expr.c:5245 under EXPAND_SUM to a bare SYMBOL_REF, a CONSTANT_P rtx, and
    emits no insn.
  - Operand 1 emits its multiply insns.
  - The sum is returned as a PLUS and is put in a register only when the address is
    legitimized afterwards (memory_address, explow.c:385, via force_operand, expr.c:3685),
    so the symbol load lands after the index insns.
  - With the local, the symbol is expanded by its own assignment statement, earlier in the
    insn stream. The target has lui/addiu D_8008EB40 BEFORE the row multiply, so the
    original evidently held the table address in a variable first.

## Annotation (prerequisite 3)
`/* FAKE: pointer alias to D_8008EB40 (pointer-alias-fake-exception) ... */` is at each
declaration (func_800233AC's `tbl` initializer; func_80023648's `tbl` declaration). In
func_80023648 the old name `new_var` is renamed `tbl` (identifier only).

## Earlier record
docs/grind/decisions.md:14423 (2026-08-26 19:07, Judge final call PASS for func_80023648)
treated the `new_var = &D_8008EB40` base as ordinary. It is cited, but the paperwork is
carried anyway (orchestrator instruction).

## Review (prerequisite 4)
Layer-2 on the combined landing (laneH, 2026-09-30).
