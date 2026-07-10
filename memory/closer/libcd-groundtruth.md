# GROUND TRUTH: BB2 links PsyQ 4.0's BIOS.OBJ verbatim (Closer W3, 2026-07-09)

## Acquisition

PsyQ SDK library binaries fetched from the `sozud/psy-q` GitHub repo (the corpus
behind `sozud/psy-q-decomp`, the WIP PsyQ matching decomp) — per-version originals,
tiny downloads (52-59 KB each), no disc images needed:

- `tmp/closer/psyq/LIBCD_3.3.LIB` (52,567 B) — no `$Id: bios.c` rcsid present
- `tmp/closer/psyq/LIBCD_3.5.LIB` (58,909 B) — `bios.c,v 1.77 1996/05/13 suzu`
- `tmp/closer/psyq/LIBCD_3.6.LIB` (59,052 B) — `bios.c,v 1.80 1996/09/11 ayako`
- `tmp/closer/psyq/LIBCD_4.0.LIB` (54,543 B) — **`bios.c,v 1.86 1997/03/28 makoto` = EXACT BB2 version**

Source URLs: `https://raw.githubusercontent.com/sozud/psy-q/master/<ver>/PSX/LIB/LIBCD.LIB`
(also mirrored in the archive.org `ps1_sdks` item as full Runtime Library disc images,
and at `https://psx.arthus.net/sdk/Psy-Q/` for 4.4-4.7).

## Tools (this session)

- `tmp/closer/psyq_lib.py` — SN LIB ("LIB\x01") + OBJ ("LNK\x02") parser: modules,
  sections, XDEF/XREF/local symbols, relocations. Key format gotcha: **relocation
  u16 offsets are relative to the most recent BYTES block, not the section start.**
- `tmp/closer/compare_groundtruth.py` — masks reloc-affected fields (HI16/LO16
  imm16, REL26 low-26, FULL32) and diffs OBJ words vs the BB2 asm/EXE words.

## The verdict (measured)

`LIBCD_4.0.LIB` module `BIOS` (bios.c v1.86): sections `.rdata` 592 B (contains the
same rcsid string BB2 carries at 0x80016274), `.text` 6,104 B, `.data` 792 B, `.bss` 36 B.

**Whole-module comparison: all 1,526 .text words are masked-identical to BB2 at
`0x80080828..0x80082000` — ZERO differing words** (995 words compared fully bit-exact,
531 reloc-masked words identical in every non-linker-filled bit). Same instruction
sequence, same scheduling, same register allocation. BB2 did not recompile bios.c —
**it linked Sony's prebuilt PsyQ 4.0 BIOS.OBJ unchanged.** The SDK object IS the
ground truth for every function in this window.

Per-function (XDEF-anchored):
- `CD_sync` (.text+0x588, 160 words) == `cpu_side_move_dir_4` @ 0x80080DB0 — masked-identical
- `CD_ready` (.text+0x808, 179 words) == `marionation_Exec` @ 0x80081030 — masked-identical
- XDEFs confirm the offsets: CD_sync/CD_ready/CD_cw/CD_vol/CD_flush/CD_initvol/
  CD_initintr/CD_init/CD_datasync/CD_getsector/CD_getsector2/CD_set_test_parmnum;
  locals `getintr` (.text+0), `callback` (.text+0x16F4), `Result`/`Alarm` in .bss
  (Alarm @ .bss+0x18 → D_800F19B8, confirming the identity memo's Alarm_t mapping).

## Full BB2 symbol map for the BIOS.OBJ window (obj offset → BB2 vaddr → BB2 name)

| libcd symbol | obj .text+ | BB2 vaddr | BB2 name |
|---|---|---|---|
| getintr (local) | 0x0000 | 0x80080828 | func_80080828 |
| CD_sync | 0x0588 | 0x80080DB0 | cpu_side_move_dir_4 |
| CD_ready | 0x0808 | 0x80081030 | marionation_Exec |
| CD_cw | 0x0AD4 | 0x800812FC | **tslTm2LoadImage** |
| CD_vol | 0x0EF0 | 0x80081718 | func_80081718 |
| CD_flush | 0x0F78 | 0x800817A0 | func_800817A0 |
| CD_initvol | 0x1058 | 0x80081880 | func_80081880 |
| CD_initintr | 0x114C | 0x80081974 | func_80081974 |
| CD_init | 0x119C | 0x800819C4 | func_800819C4 |
| CD_datasync | 0x1388 | 0x80081BB0 | **saEft01Init** |
| CD_getsector | 0x14F4 | 0x80081D1C | func_80081D1C |
| CD_getsector2 | 0x15F4 | 0x80081E1C | func_80081E1C |
| CD_set_test_parmnum | 0x16E4 | 0x80081F0C | (unnamed) |
| callback (local) | 0x16F4 | 0x80081F1C | (unnamed) |

**CORRECTIONS to memory/closer/libcd-identity.md:**
1. `saEft01Init` == **CD_datasync**, NOT CD_cw (sizes: saEft01Init = 91 words = 364 B
   = exactly CD_datasync's 0x14F4-0x1388; CD_cw is 263 words at 0x800812FC).
2. `tslTm2LoadImage` (the one at 0x800812FC) == **CD_cw**. (The memo's
   "tslTm2LoadImage_2 == puts" is a different symbol and untouched by this.)

## Version bracket (evolution evidence)

PsyQ 3.5 (v1.77) and 3.6 (v1.80) CD_sync/CD_ready are the SAME length (160/179 words)
and differ from BB2 in only 4 / 2 words respectively — every diff is the `li`
spelling `ori rt,$zero,imm` (0x34xxxxxx) vs BB2/4.0's `addiu rt,$zero,imm`
(0x24xxxxxx) at the CdlComplete/CdlDiskError constant loads. Identical C, identical
scheduling/RA across v1.77→v1.86; only the assembler-era constant-load spelling
changed (consistent with the maspsx `--aspsx-version=2.34` behavior our pipeline uses).

## Implications for the grind

1. The 155-session grind targets are **Sony library objects, not Lightweight code**.
   The canonical source is Sony's bios.c v1.86 (sotn-decomp's psxsdk bios.c v1.77 is
   the same code modulo the li-spelling-only delta measured above — its C is a
   faithful reference for v1.86's shape).
2. Ground truth is now on disk: any candidate C for cpu_side_move_dir_4 /
   marionation_Exec / saEft01Init / tslTm2LoadImage and the 9 sibling functions can
   be validated against the ORIGINAL object, and the whole window
   0x80080828-0x80082000 is a single-object link unit (one bios.c TU, compiled by
   Sony with GCC 2.7.2 + ASPSX, .rdata/.text/.data/.bss all accounted for).
3. This is the strongest possible provenance evidence for any authorization /
   disposition ruling on these queue items: byte-identity to a shipping Sony SDK
   object, XDEF-name-confirmed.
