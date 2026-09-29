# Consumers of 0x8009BD24..0x8009BD3B (D_8009BD24 record table + D_8009BD38 flag word)

Built 2026-09-29 for the func_8005E54C landing (laneC). The raw grep is consumers_raw.txt, from tools/consumers.py
(run before the landing edit, so the line numbers there are pre-landing). Line numbers below are for the landed
src/text1b.c.

## What the landing changes (text1b.c only)

- **D_8009BD24:** `extern u8 D_8009BD24[];` becomes `Unk8009BD24Record D_8009BD24[2][5]` (2-byte records
  `{ u8 chr; u8 unk1; }`), 0x14 bytes. Before the landing the declaration was an incomplete u8 array. The new
  extent ends exactly at 0x8009BD38.
- **D_8009BD38:** `extern s32 D_8009BD38;` becomes `Unk8009BD38Flags D_8009BD38` (a u32 bitfield word). The
  named fields stop at bit 23; byte 3 is left unnamed.
- Both declarations stay TU-local in text1b.c, where they were before the landing.
- No splat symbol is merged or retired. Each declaration covers exactly the bytes its own label covered before
  (D_8009BD24 up to D_8009BD38; D_8009BD38's 4 bytes).
- D_8009B490[2][2] is the only merge (D_8009B490 + D_8009B498). It goes to include/game.h, the same one-stride
  shape as D_8009B5F0.

## Direct symbol consumers (every C and asm reference)

| function | where | access | status |
|---|---|---|---|
| func_8005E54C | text1b.c (this landing) | `D_8009BD24[j][i].chr`; `D_8009BD38.unk10/.unk15` | new C |
| func_8005F1C8 | text1b.c:4950, 5027-5072 | `.unk14 + 1`, `.unk12 == 2` x6 (were `(u32)D >> 14 & 1`, `(D & 0x3000) == 0x2000`) | respelled, byte-identical |
| func_80060414 | text1b.c:5439 | `D_8009BD24[0][0].chr < 0xC` (was `D_8009BD24[0]`) | respelled, byte-identical |
| func_80060CB8 | text1b.c:5905 | `v = D_8009BD38.unk0;` (was `& 0xF`) | respelled, byte-identical |
| func_80077894 | text1b.c:15854 | `D_8009BD38.unk0 = result;` (was `s32 *p = &D_8009BD38;` + mask read-modify-write) | respelled, byte-identical (the `la`-form RMW comes out of store_bit_field) |
| func_80077904 | text1b.c:15865 | `D_8009BD38.unk0 * 2` | respelled, byte-identical |
| func_80077984 | text1b.c:15878 | passes `&D_8009BD24[0][0].chr` (u8 *) to func_8006E534 (was the decayed u8 array) | respelled, no cast, byte-identical |
| func_80077820 | text1b.c:15830 | `func_80068F70(a0, (s32 *)&D_8009BD24)` | text unchanged (pre-existing address cast at a call) |
| func_80077A80 | text1b.c:15914 | `func_800770B8(a0, (s32)&D_8009BD24, ...)` | text unchanged (pre-existing address-to-s32 at a call) |
| func_80077D00 | text1b_b.c:868-870 | `extern s32 D_8009BD24; return &D_8009BD24;` | text unchanged (other TU; returns the block address as s32 *) |
| func_80077B30 | text1b_b.c:802, 828, 857 | reads/writes byte 3 of the flag word as `extern u8 D_8009BD3B` (asm: `D_8009BD38 + 0x3`) | untouched; byte 3 is not named by the new flags type |

Whole-TU check: 456/457 functions in text1b.o identical (func_8005E54C differs only in link-equivalent relocation
addends), full rebuild SHA1 == oracle, text1b_b.c unchanged. No asm function that references either symbol is
still INCLUDE_ASM (all are C; see consumers_raw.txt).

## Pointer views of the same bytes (pre-existing, untouched)

All of these reach the 0x8009BD24 block through a pointer to its start:
- func_80077984 passes the block's address to func_8006E534, the only caller (text1b.c:15878), which stores it
  in D_800A3568 (text1b.c:11661).
- func_80077D00 returns the block's address.

| view | where | bytes reached | what they are |
|---|---|---|---|
| `*(s32 *)(arg2 + 0x14) & 0xF` | func_8006E534 text1b.c:11659 | +0x14 | D_8009BD38 (flag word) |
| `*(s32 *)(D_800A3568 + 0x14)` reads / RMW | func_8006E534 11679, func_8006F100 12143, func_80070188 12716/12776, func_80071C4C 12952/12993-12994 | +0x14 | D_8009BD38 |
| `((Cfg720FC *)D_800A3568)->unk14_4 = ...` | func_800720FC 13315/13317 | +0x14 bits 4-9 | D_8009BD38 (unk4 here) |
| `*(u8 *)(D_800A3568 + dst)`, `+ dst + 1` | func_80071C4C 13019/13029 | +0..0x13 | D_8009BD24 records (chr, unk1) |
| `*(s32 *)(D_800A3568 + 0x20) & 1` | func_8006F528 12349 | +0x20 | beyond both symbols (0x8009BD44) |
| `func_80077D00()[5] & 0xF` | func_8005C2A8 text1b.c:3959 | +0x14 | D_8009BD38 |
| `p = func_80077D00(); p[8]` | func_80069A30 9158, func_80069A8C 9174, func_8006E8CC 11760; code6cac_b3_post.c:64/149/248 (`p[8]`); code6cac_c2.c:673 | +0x20 | beyond both symbols |

These views treat the whole block, which is at least 0x24 bytes (+0x20 is read), as a word array through an
untyped pointer. They existed before this landing, and this landing touches none of them.

Modelling the block as one struct would merge D_8009BD24, D_8009BD38, D_8009BD3B.. and D_8009BD44 and respell
every view above. That would require the pointer type of D_800A3568, func_8006E534's parameter and
func_80077D00's return value, across four files. It is a separate aggregate-merge landing and is not done here.

The landing's position: it retypes two symbols, each to the shape its own symbol accesses show. It merges
nothing, so the aggregate-merge prongs (c)/(d) are not triggered. It adds no cast: func_80077984's argument
became `&D_8009BD24[0][0].chr`, and text1b_b.c is untouched, so no new `(s32 *)` return cast.
