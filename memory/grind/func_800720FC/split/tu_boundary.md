# SUPERSEDED 2026-09-29 — no split is needed

The layer-2 review of the split failed it on evidence (2026-09-29). It noted that
§2 was the Q21 (1) necessity claim itself, and it pointed to D_800A3518 /
D_800A350C, which show both access forms inside one TU. Following that lead, the
investigation found a single-declaration spelling: a local pointer to the timer
array, under pointer-alias-fake-exception, see ../timers/README.md. It reaches the
target with text1b.c's existing `extern s16 D_800A35C8[];`. The split was
unstaged and is not proposed. The rest of this file is kept as the record of what
was submitted.

On the review's counter-examples: D_800A3518 / D_800A350C are scalars. The
lui/addiu forms in 693CC / B898 / E068 are `&D_800A3518` taken as a call argument,
and the gp_rel forms are loads of the value. Neither is two constant-index
element stores of one array in a loop with calls, so they say nothing about
array versus scalars. §2's reading of func_8006F100's lui/addiu base as proof of a
non-small array is weak for the same reason. The rodata-alignment finding (§1)
stands on its own. Nothing here depends on it now.

# text1b.c TU boundary at func_8006F97C — evidence (2026-09-29, manual s2)

Split: `src/text1b.c` keeps 0x80047ED0..0x8006F97C (func_80047ED0 .. func_8006F528);
new `src/text1b_mid.c` holds 0x8006F97C..0x80077B30 (func_8006F97C .. func_80077B00),
linked right after text1b.o in all four bb2.ld sections.

## 1. Rodata alignment: the original text1b range was TWO objects (strong, mechanical)

text1b's rodata run 0x80015988..0x80015A3C, in the shipped EXE:

| addr | item | owner |
|---|---|---|
| 0x80015988 | 6-entry ADDR_VEC | switch at 0x8006B6B8 |
| 0x800159A0 | "warning\n" (16 bytes) | used by func_8007352C (see §5) |
| 0x800159B0 | 7-entry ADDR_VEC | func_8006E534 |
| 0x800159CC | zero word | padding |
| 0x800159D0 | 15-entry ADDR_VEC | func_8006ECF4 |
| 0x80015A0C | 5-entry ADDR_VEC | func_800747D8 |
| 0x80015A20 | zero word | padding |
| 0x80015A24 | 6-entry ADDR_VEC | func_80077374 |

GCC 2.7.2 emits `.align 3` before every ADDR_VEC (final.c:1515-1518). The linker
placed object rodata sections at only 4-byte alignment. The assembler pads to 8
relative to the section's start, not the absolute address.
- Measured from 0x80015988 (the section start, 0 mod 8), the tables at +0x28
  and +0x48 are 8-aligned. The zero word at 0x800159CC is the pad before +0x48.
- func_800747D8's table sits at +0x84, which is 4 mod 8, with no pad. It cannot
  be in the same section as the tables before it.
- Its section starts at 0x80015A0C. Measured from there, func_80077374's table
  is at +0x18, 8-aligned, and the zero word at 0x80015A20 is its pad.

So one object ends at or after func_8006ECF4 and another begins at or before
func_800747D8. This places the boundary between 0x8006ECF4 and 0x800747D8. It does
not say where in that range.

## 2. D_800A35C8 / D_800A35CA declaration model (strong, compiler mechanism)

Census: census.txt / census.py (every gp/hi/lo access, 0x8006E500..0x80073300).

- func_8006F100 (0x8006F100) forms `lui/addiu %hi/%lo(D_800A35C8)` as an array
  base. That is a non-small array in its TU.
- func_80070188, func_80070F78 and func_800720FC store to D_800A35C8 AND
  D_800A35CA directly, gp_rel, inside player loops.
- With a single array declaration, cse.c use_related_value relates the +2 address
  to the base register, and loop.c hoists it (measured: sandbox 6; cc1psx does
  the same, tmp/cc1psx/func_800720FC/psx.o). The direct pair needs two symbol
  bases in those functions' TU.

This places the boundary between 0x8006F100 and 0x80070188: after F100, at or
before 70188.

## 3. Picking F97C among {func_8006F528, func_8006F97C, func_80070188} (WEAK)

§1 and §2 leave three candidates. Only call locality distinguishes them (jal census):
- func_8006F528 is called only by func_8006EACC, the replay-menu dispatcher on the
  pre side. func_8006EC0C, the other helper EACC calls, is also on the pre side.
- func_80070188 is called only by func_8006F97C. Putting a boundary between them
  would separate a helper from its only caller.
- func_8006F97C itself is entry [0] of the handler table D_8009BC1C. That table's
  other entries are 70C70, F100, 71C4C, 72084, 720AC and 720D4, spread over both
  sides, so the table decides nothing.

Therefore: F528 goes with its caller (pre), 70188 goes with its caller F97C
(post), and the boundary sits before F97C. No access form in F528 or F97C
distinguishes the two sides: F528 reads D_800A3578 with lbu, F97C reads D_800A3560
indexed, and both forms occur on both sides. The choice rests on call locality
alone and is weak. 0x80070188 is the other defensible point.

## 4. Not evidence

- **D_800A3578** (lh in 70188/70F78/720FC, lhu in EC0C) looks like a flip, but
  retyping text1b's `extern u16 D_800A3578;` to s16 is byte-neutral for the whole
  object (retype.py). EC0C reads it through a `u16 word` local. It does not
  discriminate.
- **Padding nops** between functions: none anywhere in 0x80052D00..0x80077B30.
  The objects are 4-aligned (SUBALIGN(2)), so no gap is expected. Uninformative.

## 5. Open anomaly (not relied on)

"warning\n" (D_800159A0) sits in the FIRST object's rodata, between the tables of
the 0x8006B6B8 switch and func_8006E534. Its only user is func_8007352C, after
the boundary. GCC places a literal in the rodata stream at its function's position.
No single-boundary model makes it func_8007352C's own literal. The pre part keeps
it as a named const, as main already did, and the post part reaches it through
`extern`. It fits a const global defined in the first TU. It does not bear on
where the boundary falls.

## Move check

- src/text1b.c (new) is the byte-identical prefix of the pre-split file: lines
  1..12036, ending after func_8006F528.
- src/text1b_mid.c = a head block + the byte-identical remainder: lines
  12037..15080, 3044 lines.
- The head block is generated by tusplit.py and itemised in report.txt:
  - the 6 include/define lines;
  - verbatim copies of text1b.c's file-scope extern/typedef items whose names the
    moved code uses (closed transitively, identical duplicates kept once);
  - for each text1b.c function DEFINED before the split and named after it, the
    prototype its definition provided (ANSI header + `;`).
  - No object definition is copied. D_800159A0 is already declared `extern`
    inside the moved text.
- Object check (check.py): the 408 functions compile to the same instructions and
  relocations. The only exceptions are branch offsets and section-relative jump-table
  addends. Sizes: .text 0x27AAC + 0x81B4 = 0x2FC60, .rodata 0x84 + 0x30 = 0xB4.
- Build flags: text1b_mid copies text1b's memberships (RODATA_ALIGN2_FILES only;
  no GP / NO_SR / EXPAND_LB / EXPAND_LH). It must be on RODATA_ALIGN2 anyway:
  its first table (func_800747D8) is at 0x80015A0C, 4 mod 8.
  engine/buildconfig.py mirrors the Makefile.
- verify-oracle --rebuild: SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.
  engine test: 862 passed.
