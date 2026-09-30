# SelWork — Q57 (d) whole-program layout search (laneD, 2026-09-30)

Rule: `.claude/rules/no-new-park-categories.md`, "Trailing alignment padding (owner ruling 2026-09-30,
twenty-ninth batch, Q57)", conditions (a)-(d). Struct: `SelWork` (since round 2, 2026-09-30: include/game.h, the one definition of the select work area
D_800A36A0 points at; it replaces S_800747D8, SelWork_800768DC and func_80077374's local GaugeWork).
Round 2 named the formerly padded 0x00-0x07 and 0x24-0x33 (f00, f04, f24, pad28, f2C, f30) at the offsets the
code accesses; every offset and the size are unchanged, and the scan (re-run on the round-2 tree, output
identical) covers include/*.h.

## Sizes (a)/(b)

- Without the unions: the members run from 0x00 to the end of `s16 f7E[2][5]` at 0x7E + 0x14 = **0x92**.
  The struct is 2-aligned, so sizeof = **0x92**. This is the size the old SelWork_800768DC had.
- With the unions: `f10` and `f14` are `union { s16 half[2]; s32 word; }`. The s32 raises the struct's
  alignment to 4, so sizeof = **0x94**. The added bytes are **[0x92, 0x94)**, which is the compiler's own
  trailing padding.
- (a) Every member offset is unchanged: f10 at 0x10 and f14 at 0x14 are exactly where the s16[2] pairs
  were, and every other member sits at the offset the code's accesses show (see the member table in
  laneD-2026-09-30.md).
- (b) No member is declared or widened for 0x92..0x93. The last member is f7E, ending at 0x92.

## (c) Build

Recorded with the landing: full-build SHA1 and a sandbox 0 for every changed function (see
laneD-2026-09-30.md, "Landing").

## (d) The search: method, and every hit with the step that decided it

Tool: `laneD-2026-09-30/q57scan.py` (this directory). Its output as of 2026-09-30 is in
`laneD-2026-09-30/q57scan.txt`.
- **Part A.** Every asm/funcs function that loads `%gp_rel(D_800A36A0)`: 16 functions. For each, the tool
  tracks the loaded register and the registers derived from it by `addiu` (constant offset) and `addu`
  (index). It lists every load and store made through them, with offset and width, and the defining
  instruction of every index. It also flags any tracked register that is stored as a value or moved into
  an argument register.
- **Part B.** Every arithmetic immediate 0x92 or 0x94 in all asm/funcs.
- **Part C.** The C side: src/*.c and include/*.h are searched for `sizeof(` any of the three struct names,
  and for any of them declared as an array or as a non-pointer member.

### (1) Containing layout. No hit.

- **Only instance.** D_800A36A0 (asm/data/91C98.data.s:4529, initial word 0) is the only pointer to this
  object. Its only writer is func_800770B8 0x8007713C `sw $s1,%gp_rel(D_800A36A0)`, and the value it
  stores is func_8006E49C's return. func_8006E49C is src/text1b_tu1c.c:8808; it returns the end address
  of the arena it lays out, `base4 + 0x1FB0`. So the object is one work area, not an array element.
- **Index strides in Part A.** Every `addu` into a D_800A36A0-derived register has an index that is one of:
  - player*2 (`sll 1`),
  - player*4 (`sll 2`),
  - (s16)x*2 (`sra 15`),
  - player*10 + j*2 (func_800770B8 0x800771B0, func_80077374 `addiu 0x6A` + player*10),
  - player + 0x68 (bytes).
  Each of these indexes a member array inside the struct. None is a multiple of 0x92 or 0x94.
- **C side.** No C declaration puts SelWork (or S_800747D8 / SelWork_800768DC) in an array or inside
  another aggregate. Part C: no hits.

### (2) Size or stride uses. 11 immediates found (Part B), all decided by step 2, "Unrelated".

All 11 are 0x92 or 0x94 immediates in functions that neither load D_800A36A0 nor receive a
D_800A36A0-derived pointer. The only callees that receive such a pointer are listed under (3) below, and
none of them is one of these 11.
- func_8003FECC 0x8003FF1C `addiu $v0,$v0,0x94`
- func_80040A78 0x80040A7C `addiu $v1,$a0,0x94`
- func_80040B44 0x80040B50 `addiu $t7,$t6,0x94`
- func_80040CB8 0x80040CD0 `addiu $t0,$a0,0x94`
- func_80040D48 0x80040DC4 `addiu $s3,$s4,0x94`
- func_80041188 0x800411E0 `addiu $t0,$v0,0x94`
- func_80041688 0x800416B4 `addiu $v1,$s0,0x94`
- func_80060768 0x800609C4 `li 0x92`
- func_80065800 0x80065854 `addiu $fp,$s0,0x94`
- func_8006A880 0x8006A928 `li 0x92`
- func_8006ECF4 0x8006EDE4 `li 0x94`

Other size uses:
- The C side has no `sizeof(SelWork)` in any spelling.
- No copy, clear or allocation covers the object: func_8006E49C hands out the address and no length.
- func_800770B8 clears the object member by member; its highest store is `sh 0x7E + player*10 + j*2`,
  which ends at 0x92.

### (3) Accesses in or into [0x92, 0x94). No hit.

- **Part A offsets.** Constant offsets through a D_800A36A0-derived base, over all 16 functions: 0x00 to
  0x69. The tool flags no access whose byte range reaches 0x92 ("REACHES" lines: none).
- **Highest indexed accesses**, with their bounds:
  - f6A / f7E at 0x6A / 0x7E + player*10 + j*2, with j < f65 + 3. f65 stays in 0..f64:
    - func_800747D8 wraps f65 to 0 at f64 and to f64 below 0;
    - func_800770B8 stores f64 = min(count) - 3, capped at 2 (0x800772E8 to 0x80077308).
    So j <= 4, and the last byte is 0x7E + 10 + 8 + 1 = 0x91.
  - f7E[p][f3C - 1] (func_800768DC 0x7C base): f3C <= f65 + 3.
- **Tracked pointer stored or passed as a bare argument ("ESCAPE" lines).** None.
- **Derived pointers passed to callees:**
  - base + 0x40 + player*4 goes to func_800692C0's arg2 (src/text1b_tu1c.c:3815). It touches arg2[0]
    and arg2[1] (`i < 2`): 4 bytes inside f40.
  - base + 0x6A + player*10 goes to func_800759D0 / func_80075F80 / func_800768DC / func_8007636C as the
    `s16 *` list (from func_80077374). They index it below f65 + 3 <= 5: inside f6A.
- **Data or pointer words.** D_800A36A0 is the only data word referring to the object, and it holds 0 in
  the image (asm/data/91C98.data.s:4530). The object's address exists only at run time (the arena
  func_80076FF8 returns, a0[1]), so no word in the EXE or on the disc can point into it. Outside
  asm/funcs, asm/text1b.s is the only other file naming D_800A36A0, and it is not linked.

Every hit was decided by step 2 ("unrelated"). No hit falls under step 1, 3, 4 or 6. The size (0x94) is
used nowhere, so step 5 has no hit either.
