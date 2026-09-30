# -G8 TU text1b_tu1d: landing record + screen evidence (owner rulings Q10, Q44, Q54)

## Landing (2026-09-30, manual path, g8-impl)

Three changes on main, in order:
1. `cheat-cleanup:` 344674809: the unreferenced rodata
   pad `const u32 D_800159CC = 0;` deleted from text1b_tu1c.c. Byte-neutral at -G0 on the unsplit tree
   (object-relative rodata alignment supplies the word between jtbl_800159B0 and jtbl_800159D0).
2. `split:` 6fd3fb9d3: text1b_tu1c.c -> text1b_tu1c.c (..func_8006E49C) | text1b_tu1d.c (func_8006E534..
   func_80073200) | text1b_tu1e.c (func_8007352C..func_80074488), all at -G0. Moved lines verbatim
   (memory/grind/func_80058580/aspsx-align-check/poc/splitc.py). Oracle SHA1 reached.
3. (this record) GP_FILES += text1b_tu1d (Makefile + engine/buildconfig.py); the Q33 union
   `Unk800A3560Slots D_800A3560` (rec[2] + word) as the canonical declaration in include/game.h;
   every consumer respelled through it; func_80070F78 INCLUDE_ASM -> C; sdata_exclude.txt config B
   (D_800A3560 dropped from the func_8006F97C, func_80070F78 and func_80071C4C rows); prong (c): the
   undefined_syms_auto.txt rows D_800A3561/62/63/65 retired (func_80070F78.s was their last assembled
   referrer; the link proves no built source names them), and likewise named_syms.txt's
   g_replay_motion_shared_state_e_plus_1/_plus_2 (0x800A3561/62). Dead gate-list rows removed
   (byte-neutral, oracle SHA1): sdata_syms.txt D_800A3561..D_800A3565; sdata_exclude.txt's
   D_800A3561 (func_8006ECF4) and D_800A3562 (func_8006F100, func_80070188, func_800720FC; the
   func_80071C4C row, which named only D_800A3562, deleted); D_800A3560 dropped from the
   func_80070C70 and func_800720FC rows too (maspsx output identical with and without them), so no
   sdata_exclude row names 0x800A3560..65 any more. DescF97C.pad0C renamed unk0C
   (func_80070F78 writes real values there).

Why one change (the change-3 layer-2 reviewer's reasoning, 2026-09-30): the split landed
separately first (6fd3fb9d3, a pure move at -G0). The rest cannot be split further: prong (iii)
forbids INCLUDE_ASM in a -G8 TU, so func_80070F78 must become C in the same change that adds
text1b_tu1d to GP_FILES; Q44 requires the union for func_80070F78's C (the per-byte model is not
admitted); aggregate-merge prong (c) (one C handle per address) then forces every other consumer's
respelling through the union into that same change; and a -G0 "3a" (union at -G0, func_80070F78
still asm) would need func_80070188's `rec[D_800A3554]` variable-index spelling, a Q22-class dummy
index, which is refused.

Differences from the screen's union.c (tmp/audit-2026-09-29/g8-screen/gen_union.py):
- Declaration in include/game.h, not TU-local (aggregate-merge prong (d)), with the object-model
  evidence and the word-view citation (func_8006E534.s:85, 0x8006E668 `sw $v0,%gp_rel(D_800A3560)($gp)`,
  $v0 = -1 from 0x8006E664).
- func_80070188 cancel arm: `D_800A3560.rec[1].unk0` (the plain slot-1 spelling of the former
  D_800A3563) instead of the banked `rec[D_800A3554].unk0`. At -G0 the variable index was needed
  (evidence.md of func_80070188, "Cancel arm"); in the -G8 TU the constant address is a small-data
  access and both spellings score 0 (sandbox --candidate, 698/698). Consequence: func_80070188 now
  also differs between -G8 and -G0 (listing below); it has 53 gp accesses of its own (prong (i)).
- func_80070F78 `tim` FAKE: re-measured in the landed form, inlined 8 / 9 / 14 (was 9 / 10 / 15);
  comment updated.
- func_80071C4C `dst` FAKEs: still needed (inlined 1 per loop, every lever 2 in total: an `addu`
  operand-order swap at insn 226 / 249). The stale "computed before ctx" mechanism is replaced by the
  dump-proven one, RTL expansion's both_summands "put a multiplication first" (expr.c:5288-5290);
  dumps and levers: memory/grind/_completed/func_80071C4C/hypotheses.md [g8 2026-09-30].
- func_80070F78 r11 locals: landed-form alternative scores banked in r11/README.md (D)(5).

(iv-a): text1b_tu1d emits jtbl_800159B0 (func_8006E534) and jtbl_800159D0 (func_8006ECF4), both
0 mod 8 in the shipped binary; the per-file RODATA_ALIGN2 list is retired by the object-relative
rodata rule (.claude/rules/rodata-object-alignment.md), so no list membership is involved.
text1b_tu1e emits no jump table.

Verification of change 3 (tmp/audit-2026-09-29/g8-impl-change3-verify.txt, not tracked):
verify-oracle --rebuild SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle; sandbox --disable all
0 for all 28 functions of text1b_tu1d and 4 of text1b_tu1e; Makefile-built objects of tu1c/tu1d/tu1e
identical to the engine-built ones (sections, relocations, disassembly); check_completion_integrity
OK; engine test 966/0.

### Both ways on the landed text (screening scope + Q54)

text1b_tu1d.c as landed, compiled through the full per-file pipeline twice (the Makefile's own
command line, `-G8` replaced by `-G0` for the second; maspsx flags are the same for GP and non-GP
files). Only func_80070188 and func_80070F78 change; every other function, including the four
gp-free ones, is identical in words and relocations, so no extern's compiled instructions change
under -G8 outside those two (whose changed accesses are the listed gp symbols, all in sdata_syms.txt).

```
maspsx flags identical for G0/G8 files
G8 scratch == build object
== strict per-function (words + relocations), -G8 vs -G0 ==
func_8006E534: identical (222 insns)
func_8006E8AC: identical (8 insns)
func_8006E8CC: identical (33 insns)
func_8006E950: identical (54 insns)
func_8006EA28: identical (41 insns)
func_8006EACC: identical (80 insns)
func_8006EC0C: identical (58 insns)
func_8006ECF4: identical (209 insns)
func_8006F038: identical (50 insns)
func_8006F100: identical (266 insns)
func_8006F528: identical (277 insns)
func_8006F97C: identical (515 insns)
func_80070188: CHANGED insns 698->700 differing=194 rodata_same=True
    @@ -43 +43 @@
    -1860024c
    +1860024e
    @@ -417 +417 @@
    -1468007c
    +1468007e
    @@ -424,3 +424,3 @@
    -10400074
    -00000000
    -16200073
    +10400076
    +00000000
    +16200075
    @@ -444 +444 @@
    -144000bb
    +144000bd
    @@ -465 +465 @@
    -08000000 R_MIPS_26 func_80070188+0x9dc
    +08000000 R_MIPS_26 func_80070188+0x9e4
    @@ -471 +471 @@
    -08000000 R_MIPS_26 func_80070188+0x874
    +08000000 R_MIPS_26 func_80070188+0x87c
    @@ -479 +479 @@
    -1080003e
    +10800040
    @@ -483 +483 @@
    -1440003a
    +1440003c
    @@ -504 +504 @@
    -08000000 R_MIPS_26 func_80070188+0x874
    +08000000 R_MIPS_26 func_80070188+0x87c
    @@ -508,3 +508,5 @@
    -1462000f
    -00141400
    -93820000 R_MIPS_GPREL16 D_800A3560 imm=0x3
    +14620011
    +00141400
    +3c030000 R_MIPS_HI16 D_800A3560 imm=0x0
func_80070C70: identical (194 insns)
func_80070F78: CHANGED insns 810->816 differing=801 rodata_same=True
    @@ -2,2 +2,2 @@
    -27bdff90
    -afb70064
    +27bdff88
    +afb7006c
    @@ -5,10 +5,10 @@
    -afbf006c
    -afbe0068
    -afb60060
    -afb5005c
    -afb40058
    -afb30054
    -afb20050
    -afb1004c
    -afb00048
    -8c5e0074
    +afbf0074
    +afbe0070
    +afb60068
    +afb50064
    +afb40060
    +afb3005c
    +afb20058
    +afb10054
    +afb00050
    +8c420074
    @@ -16 +16,2 @@
    -8fc40000
    +afa20018
    +8c440000
    @@ -32 +33 @@
    -afa00018
    +afa00020
    @@ -67 +68 @@
    -08000000 R_MIPS_26 func_80070F78+0x124
    +08000000 R_MIPS_26 func_80070F78+0x128
    @@ -76 +77 @@
    -afa90018
func_80071C20: identical (11 insns)
func_80071C4C: identical (270 insns)
func_80072084: identical (10 insns)
func_800720AC: identical (10 insns)
func_800720D4: identical (10 insns)
func_800720FC: identical (690 insns)
func_80072BC4: identical (68 insns)
func_80072CD4: identical (79 insns)
func_80072E10: identical (72 insns)
func_80072F30: identical (39 insns)
func_80072FCC: identical (37 insns)
func_80073060: identical (104 insns)
func_80073200: identical (203 insns)
== address-resolved (linked-byte equivalence), -G8 vs -G0 ==
func_8006E534: identical (222 insns)
func_8006E8AC: identical (8 insns)
func_8006E8CC: identical (33 insns)
func_8006E950: identical (54 insns)
func_8006EA28: identical (41 insns)
func_8006EACC: identical (80 insns)
func_8006EC0C: identical (58 insns)
func_8006ECF4: identical (209 insns)
func_8006F038: identical (50 insns)
func_8006F100: identical (266 insns)
func_8006F528: identical (277 insns)
func_8006F97C: identical (515 insns)
func_80070188: CHANGED insns 698->700 differing(addr-resolved)=194
func_80070C70: identical (194 insns)
func_80070F78: CHANGED insns 810->816 differing(addr-resolved)=798
func_80071C20: identical (11 insns)
func_80071C4C: identical (270 insns)
func_80072084: identical (10 insns)
func_800720AC: identical (10 insns)
func_800720D4: identical (10 insns)
func_800720FC: identical (690 insns)
func_80072BC4: identical (68 insns)
func_80072CD4: identical (79 insns)
func_80072E10: identical (72 insns)
func_80072F30: identical (39 insns)
func_80072FCC: identical (37 insns)
func_80073060: identical (104 insns)
func_80073200: identical (203 insns)
```

---

# -G8 TU evidence: 0x800A3560 consumers, func_8006E534..func_80073200 (Q44 / Q54)

Ledger-ready record for the per-file -G8 route (.claude/rules/compiler-flags-canonical.md,
"Per-file -G8 by proof", prongs (i) and (ii); owner rulings Q44 and Q54, c35f9169e). Calibration only:
cc1psx is never a build path; the committed build compiles with the project cc1 and the oracle SHA1
decides the match ([[cc1psx-calibration-only]], [[no-compiler-divergence]]).

Tree: HEAD c35f9169e. Scratch (read-only on tracked files): tmp/audit-2026-09-29/g8-screen/.
Proposed -G8 TU: `P3_1.c` = func_8006E534's declaration block (`typedef struct SelectEntryE534`) through
the end of func_80073200, with the Q33-style union
`union { Unk800A3560Record rec[2]; s32 word; } D_800A3560` and no D_800A3561..65 handles. Its preamble
is a scratch stand-in (text1b_tu1c head declarations with function bodies stripped), not the landing text.

## Prong (i): gp-relative accesses in the original bytes

Source: the shipped bytes as split in `asm/funcs/<func>.s` (address, word, disassembly). Addresses resolved
through `undefined_syms_auto.txt` + `build/bb2.map`. "$gp words" counts every load/store/addi(u) whose base
register is 28, decoded from the raw word (independent of the `%gp_rel` annotation).

### Summary

| # | function | words | gp accesses (%gp_rel) | $gp words | symbols |
|---|---|---|---|---|---|
| 1 | func_8006E534 | 222 | 45 | 45 | D_800A32E8, D_800A32E9, D_800A3554, D_800A3558, D_800A3560, D_800A3561, D_800A3564, D_800A3568, D_800A356C, D_800A3570, D_800A3578, D_800A357C, D_800A3580, D_800A3588, D_800A358A, D_800A358C, D_800A358E, D_800A3598, D_800A359C, D_800A35A0, D_800A35A8, D_800A35AC, D_800A35B0, D_800A35B4, D_800A35B8, D_800A35BC, D_800A35C4 |
| 2 | func_8006E8AC | 8 | 1 | 1 | D_800A35AC |
| 3 | func_8006E8CC | 33 | 0 | 0 | (none: gp-free, Q54) |
| 4 | func_8006E950 | 54 | 0 | 0 | (none: gp-free, Q54) |
| 5 | func_8006EA28 | 41 | 0 | 0 | (none: gp-free, Q54) |
| 6 | func_8006EACC | 80 | 11 | 11 | D_800A3548, D_800A354C, D_800A3580, D_800A35A0, D_800A35A4, D_800A35A8, D_800A35BC, D_800A35C0, D_800A35C4 |
| 7 | func_8006EC0C | 58 | 13 | 13 | D_800A3570, D_800A3578, D_800A3580, D_800A3584 |
| 8 | func_8006ECF4 | 209 | 16 | 16 | D_800A32E8, D_800A32E9, D_800A3554, D_800A3568, D_800A35A8, D_800A35B0, D_800A35BC, D_800A35C4 |
| 9 | func_8006F038 | 50 | 3 | 3 | D_800A3550 |
| 10 | func_8006F100 | 266 | 16 | 16 | D_800A3550, D_800A3558, D_800A355C, D_800A3561, D_800A3568, D_800A3584, D_800A35A8, D_800A35B0, D_800A35BC |
| 11 | func_8006F528 | 277 | 17 | 17 | D_800A3568, D_800A3570, D_800A3578, D_800A35A8, D_800A35C0, D_800A35C4 |
| 12 | func_8006F97C | 515 | 25 | 25 | D_800A32E8, D_800A32E9, D_800A3554, D_800A3564, D_800A35A8, D_800A35B0, D_800A35BC, D_800A35C4 |
| 13 | func_80070188 | 698 | 53 | 53 | D_800A354C, D_800A3554, D_800A3558, D_800A3560, D_800A3561, D_800A3563, D_800A3564, D_800A3565, D_800A3568, D_800A3578, D_800A3580, D_800A3584, D_800A358A, D_800A358E, D_800A3592, D_800A35A0, D_800A35A8, D_800A35B0, D_800A35B4, D_800A35BC, D_800A35C4, D_800A35C8, D_800A35CA |
| 14 | func_80070C70 | 194 | 8 | 8 | D_800A3558, D_800A35A8, D_800A35B0, D_800A35BC |
| 15 | func_80070F78 | 810 | 81 | 81 | D_800A354C, D_800A3550, D_800A3554, D_800A3558, D_800A355C, D_800A3561, D_800A3562, D_800A3563, D_800A3565, D_800A3568, D_800A3578, D_800A3584, D_800A35A8, D_800A35B0, D_800A35BC, D_800A35C4, D_800A35C8, D_800A35CA |
| 16 | func_80071C20 | 11 | 1 | 1 | D_800A3561 |
| 17 | func_80071C4C | 270 | 30 | 30 | D_800A3550, D_800A3554, D_800A3558, D_800A3561, D_800A3568, D_800A3578, D_800A3584, D_800A3598, D_800A359C, D_800A35A0, D_800A35A8, D_800A35B0, D_800A35BC |
| 18 | func_80072084 | 10 | 1 | 1 | D_800A35A8 |
| 19 | func_800720AC | 10 | 1 | 1 | D_800A35A8 |
| 20 | func_800720D4 | 10 | 1 | 1 | D_800A35A8 |
| 21 | func_800720FC | 690 | 62 | 62 | D_800A354C, D_800A3568, D_800A3578, D_800A3580, D_800A3584, D_800A3598, D_800A359C, D_800A35A0, D_800A35A8, D_800A35B0, D_800A35BC, D_800A35C0, D_800A35C4, D_800A35C8, D_800A35CA |
| 22 | func_80072BC4 | 68 | 1 | 1 | D_800A35C4 |
| 23 | func_80072CD4 | 79 | 1 | 1 | D_800A35C4 |
| 24 | func_80072E10 | 72 | 4 | 4 | D_800A3580 |
| 25 | func_80072F30 | 39 | 0 | 0 | (none: gp-free, Q54) |
| 26 | func_80072FCC | 37 | 1 | 1 | D_800A3580 |
| 27 | func_80073060 | 104 | 6 | 6 | D_800A3580 |
| 28 | func_80073200 | 203 | 5 | 5 | D_800A3580, D_800A35A8, D_800A35C4 |
| | **total** | | **403** | | 40 distinct addresses |

### Outside neighbours (adjacent in address order, staying in -G0 TUs)

- before: **func_8006E49C**: 38 words, 0 `%gp_rel` accesses, 0 $gp-based words. gp-free, so it shares no
  gp-relative symbol with the TU.
- after: **func_8007352C**: 127 words, 0 `%gp_rel` accesses, 0 $gp-based words. gp-free, so it shares no
  gp-relative symbol with the TU.

### Binary-wide census of the listed set

Every `asm/funcs/*.s` outside the range was scanned for gp-relative accesses to any of the 40 addresses
above: **none**. Every one of them is gp-accessed only by functions in this range, anywhere in the binary.
Under the ASPSX rule measured in memory/grind/func_80036140/research-common-gp.md (Sony ASPSX 2.34
gp-addresses only symbols the assembling file itself defines; `.extern` never), this is consistent with one
original file defining all of them and holding every gp accessor.

### -G0 score under the existing build (func_80070F78, the function whose bytes need -G8)

- Union body (memory/grind/func_80070F78/rejected/union-merge-final-67.c) in this TU at -G0 (scratch, sdata_exclude
  config B): engine score 61, 816/810 insns. At -G8: 15, 810/810, every remaining hunk the gp name artifact
  (`%gp_rel(D_800A3560+k)` vs `%gp_rel(D_800A356k)`, same address; linked bytes identical).
- Why -G0 cannot produce the listed accesses: memory/grind/func_80070F78/evidence.md "Mechanism at -G0" (a member
  access at a constant offset is forced through a register by explow.c memory_address while cse is expected;
  only a <= 8-byte small-data object keeps the constant address direct, which is what the assembler makes gp).
  The per-byte model that does reach 1/810 at -G0 is not admitted (Q44).
- Every other function of the range matches under the union at both -G0 and -G8 (name artifacts only).

### Full listing (every gp access, original bytes)

#### func_8006E534: 45

| address | instruction | symbol | resolved |
|---|---|---|---|
| 8006E570 | `sb $v0, %gp_rel(D_800A32E8)($gp)` | D_800A32E8 | 0x800a32e8 |
| 8006E578 | `sw $s0, %gp_rel(D_800A35AC)($gp)` | D_800A35AC | 0x800a35ac |
| 8006E580 | `sw $s1, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 8006E584 | `sh $zero, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 8006E588 | `sh $zero, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8006E58C | `sb $zero, %gp_rel(D_800A32E9)($gp)` | D_800A32E9 | 0x800a32e9 |
| 8006E590 | `sw $s2, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006E594 | `sw $s0, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |
| 8006E598 | `sw $s0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006E5A0 | `sw $v0, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 8006E5AC | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 8006E5D8 | `lw $a1, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |
| 8006E5E4 | `lw $a1, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |
| 8006E5F0 | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 8006E60C | `lw $a1, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |
| 8006E618 | `lw $a1, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |
| 8006E628 | `lw $a0, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |
| 8006E634 | `lw $a1, %gp_rel(D_800A35AC)($gp)` | D_800A35AC | 0x800a35ac |
| 8006E63C | `sw $a0, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |
| 8006E660 | `sw $v0, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |
| 8006E668 | `sw $v0, %gp_rel(D_800A3560)($gp)` | D_800A3560 | 0x800a3560 |
| 8006E670 | `sh $zero, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006E674 | `sh $zero, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 8006E678 | `sh $zero, %gp_rel(D_800A357C)($gp)` | D_800A357C | 0x800a357c |
| 8006E67C | `sh $zero, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 8006E680 | `sw $zero, %gp_rel(D_800A35A0)($gp)` | D_800A35A0 | 0x800a35a0 |
| 8006E684 | `sh $zero, %gp_rel(D_800A3588)($gp)` | D_800A3588 | 0x800a3588 |
| 8006E688 | `sh $zero, %gp_rel(D_800A358C)($gp)` | D_800A358C | 0x800a358c |
| 8006E68C | `sh $v0, %gp_rel(D_800A358A)($gp)` | D_800A358A | 0x800a358a |
| 8006E690 | `sh $zero, %gp_rel(D_800A358E)($gp)` | D_800A358E | 0x800a358e |
| 8006E694 | `sb $v1, %gp_rel(D_800A3561)($gp)` | D_800A3561 | 0x800a3561 |
| 8006E698 | `sb $a0, %gp_rel(D_800A3564)($gp)` | D_800A3564 | 0x800a3564 |
| 8006E700 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 8006E770 | `lh $v1, %gp_rel(D_800A3588)($gp)` | D_800A3588 | 0x800a3588 |
| 8006E774 | `lh $a0, %gp_rel(D_800A358C)($gp)` | D_800A358C | 0x800a358c |
| 8006E77C | `sh $v0, %gp_rel(D_800A35B8)($gp)` | D_800A35B8 | 0x800a35b8 |
| 8006E784 | `sh $v0, %gp_rel(D_800A35B4)($gp)` | D_800A35B4 | 0x800a35b4 |
| 8006E7C8 | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006E7D8 | `lh $v1, %gp_rel(D_800A358A)($gp)` | D_800A358A | 0x800a358a |
| 8006E7DC | `lh $a0, %gp_rel(D_800A358E)($gp)` | D_800A358E | 0x800a358e |
| 8006E848 | `sh $zero, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 8006E84C | `sh $zero, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 8006E870 | `lw $a0, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |
| 8006E87C | `sw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006E880 | `sw $v1, %gp_rel(D_800A356C)($gp)` | D_800A356C | 0x800a356c |

#### func_8006E8AC: 1

| address | instruction | symbol | resolved |
|---|---|---|---|
| 8006E8BC | `lw $v1, %gp_rel(D_800A35AC)($gp)` | D_800A35AC | 0x800a35ac |

#### func_8006E8CC: 0

none

#### func_8006E950: 0

none

#### func_8006EA28: 0

none

#### func_8006EACC: 11

| address | instruction | symbol | resolved |
|---|---|---|---|
| 8006EAFC | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 8006EB08 | `sw $a0, %gp_rel(D_800A3548)($gp)` | D_800A3548 | 0x800a3548 |
| 8006EB0C | `sw $a1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 8006EB10 | `sw $v0, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 8006EB20 | `sw $v0, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 8006EB2C | `lw $a1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006EB54 | `lw $v1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006EBAC | `sw $v0, %gp_rel(D_800A35A4)($gp)` | D_800A35A4 | 0x800a35a4 |
| 8006EBB4 | `lhu $v1, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 8006EBD4 | `lh $v0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 8006EBF8 | `lw $v0, %gp_rel(D_800A35A0)($gp)` | D_800A35A0 | 0x800a35a0 |

#### func_8006EC0C: 13

| address | instruction | symbol | resolved |
|---|---|---|---|
| 8006EC10 | `lbu $v1, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 8006EC58 | `lhu $v0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006EC64 | `sh $v0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006EC7C | `lhu $v0, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |
| 8006EC80 | `lhu $a0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 8006EC84 | `sh $v1, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006EC88 | `sh $v0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 8006EC98 | `sh $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 8006ECA4 | `lh $v1, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006ECC0 | `lhu $v0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006ECCC | `sh $v0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006ECDC | `sh $zero, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006ECE0 | `sh $zero, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |

#### func_8006ECF4: 16

| address | instruction | symbol | resolved |
|---|---|---|---|
| 8006ED3C | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006ED44 | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8006EDBC | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006EE38 | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006EE4C | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006EE60 | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006EE74 | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006EE88 | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006EEA8 | `lbu $v0, %gp_rel(D_800A32E8)($gp)` | D_800A32E8 | 0x800a32e8 |
| 8006EEB8 | `lbu $v1, %gp_rel(D_800A32E9)($gp)` | D_800A32E9 | 0x800a32e9 |
| 8006EEBC | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8006EF24 | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006EF6C | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 8006EF7C | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 8006EFF8 | `lh $v1, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8006EFFC | `lw $a0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |

#### func_8006F038: 3

| address | instruction | symbol | resolved |
|---|---|---|---|
| 8006F05C | `lhu $v0, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 8006F070 | `lhu $v1, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 8006F080 | `lhu $v1, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |

#### func_8006F100: 16

| address | instruction | symbol | resolved |
|---|---|---|---|
| 8006F100 | `lh $v0, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 8006F13C | `sh $v0, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 8006F154 | `sh $v0, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 8006F160 | `sh $zero, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 8006F194 | `lh $v0, %gp_rel(D_800A355C)($gp)` | D_800A355C | 0x800a355c |
| 8006F1A8 | `sh $v0, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |
| 8006F1AC | `lh $v1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 8006F1C4 | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006F1C8 | `lw $a0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006F214 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 8006F22C | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 8006F250 | `lbu $v0, %gp_rel(D_800A3561)($gp)` | D_800A3561 | 0x800a3561 |
| 8006F4CC | `lh $v1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 8006F4D0 | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006F4E8 | `lhu $v0, %gp_rel(D_800A355C)($gp)` | D_800A355C | 0x800a355c |
| 8006F4F4 | `sh $v0, %gp_rel(D_800A355C)($gp)` | D_800A355C | 0x800a355c |

#### func_8006F528: 17

| address | instruction | symbol | resolved |
|---|---|---|---|
| 8006F52C | `lw $v1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006F568 | `lbu $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 8006F588 | `lh $v0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006F62C | `lh $v0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006F6CC | `lhu $v0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006F6E8 | `lw $v1, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 8006F6EC | `lhu $a0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006F730 | `lw $v1, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 8006F7B0 | `lw $v0, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 8006F7B4 | `lw $a1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006F7C4 | `lh $v0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006F7D0 | `lh $v0, %gp_rel(D_800A3570)($gp)` | D_800A3570 | 0x800a3570 |
| 8006F7E0 | `lw $v0, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 8006F7E4 | `lw $a1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006F818 | `lw $v1, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 8006F81C | `lw $a1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006F888 | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |

#### func_8006F97C: 25

| address | instruction | symbol | resolved |
|---|---|---|---|
| 8006F980 | `lw $v1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 8006FA3C | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006FA48 | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8006FA94 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006FAE0 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006FB24 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006FB5C | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006FB98 | `lh $v1, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8006FB9C | `lw $a0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006FBE0 | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006FBE8 | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8006FC34 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006FC80 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006FCC4 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006FCFC | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8006FD38 | `lh $v1, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8006FD3C | `lw $a0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006FDF4 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 8006FE80 | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8006FE84 | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8006FEA4 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80070134 | `lbu $v0, %gp_rel(D_800A3564)($gp)` | D_800A3564 | 0x800a3564 |
| 80070138 | `lhu $v1, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8007013C | `sb $v0, %gp_rel(D_800A32E8)($gp)` | D_800A32E8 | 0x800a32e8 |
| 80070140 | `sb $v1, %gp_rel(D_800A32E9)($gp)` | D_800A32E9 | 0x800a32e9 |

#### func_80070188: 53

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80070194 | `lw $v1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80070214 | `lhu $a3, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80070220 | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80070224 | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 800702BC | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 800702D8 | `lw $v1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 80070444 | `lw $v1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 80070494 | `lh $v1, %gp_rel(D_800A35B4)($gp)` | D_800A35B4 | 0x800a35b4 |
| 8007057C | `lhu $v0, %gp_rel(D_800A35B4)($gp)` | D_800A35B4 | 0x800a35b4 |
| 80070628 | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80070720 | `lw $a0, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 800707F4 | `sh $v0, %gp_rel(D_800A35C8)($gp)` | D_800A35C8 | 0x800a35c8 |
| 800707FC | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80070804 | `sh $v0, %gp_rel(D_800A35CA)($gp)` | D_800A35CA | 0x800a35ca |
| 80070810 | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80070834 | `lbu $v0, %gp_rel(D_800A3561)($gp)` | D_800A3561 | 0x800a3561 |
| 80070854 | `sh $t0, %gp_rel(D_800A358A)($gp)` | D_800A358A | 0x800a358a |
| 80070860 | `sh $zero, %gp_rel(D_800A358A)($gp)` | D_800A358A | 0x800a358a |
| 80070864 | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80070868 | `sh $zero, %gp_rel(D_800A358E)($gp)` | D_800A358E | 0x800a358e |
| 8007087C | `lh $v1, %gp_rel(D_800A358A)($gp)` | D_800A358A | 0x800a358a |
| 80070898 | `sb $v0, %gp_rel(D_800A3564)($gp)` | D_800A3564 | 0x800a3564 |
| 800708F4 | `lw $a0, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 80070908 | `lh $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80070950 | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80070960 | `sw $v0, %gp_rel(D_800A35A0)($gp)` | D_800A35A0 | 0x800a35a0 |
| 8007096C | `lh $v1, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 8007097C | `lbu $v0, %gp_rel(D_800A3563)($gp)` | D_800A3563 | 0x800a3563 |
| 8007098C | `sh $zero, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80070990 | `sb $s6, %gp_rel(D_800A3560)($gp)` | D_800A3560 | 0x800a3560 |
| 8007099C | `sb $s6, %gp_rel(D_800A3563)($gp)` | D_800A3563 | 0x800a3563 |
| 800709FC | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80070A24 | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80070A7C | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80070B3C | `lhu $a3, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80070B48 | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80070B4C | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80070B64 | `lh $v0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 80070B74 | `lbu $v1, %gp_rel(D_800A3560)($gp)` | D_800A3560 | 0x800a3560 |
| 80070B84 | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80070B9C | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80070BAC | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80070BC8 | `sh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80070BCC | `lbu $v1, %gp_rel(D_800A3563)($gp)` | D_800A3563 | 0x800a3563 |
| 80070BDC | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80070BF4 | `lh $v1, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80070BF8 | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80070C0C | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80070C14 | `sh $a0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80070C18 | `sh $zero, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80070C24 | `sb $v0, %gp_rel(D_800A3565)($gp)` | D_800A3565 | 0x800a3565 |
| 80070C2C | `sh $v0, %gp_rel(D_800A3592)($gp)` | D_800A3592 | 0x800a3592 |
| 80070C30 | `sh $a0, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |

#### func_80070C70: 8

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80070C84 | `lw $v1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80070DF4 | `lhu $a2, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80070DFC | `lw $a1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80070E08 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80070E8C | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80070ECC | `lhu $a2, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80070ED0 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80070ED4 | `lw $a1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |

#### func_80070F78: 81

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80070F78 | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80071014 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071018 | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8007103C | `lh $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 8007109C | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 800710C0 | `lh $v1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 800710C4 | `lw $a0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 800710DC | `lhu $a1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 800710E0 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 800710E4 | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 800710FC | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80071144 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80071154 | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80071178 | `lbu $v0, %gp_rel(D_800A3561)($gp)` | D_800A3561 | 0x800a3561 |
| 80071228 | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80071248 | `lw $v1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 800712A0 | `lw $a0, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 80071300 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80071330 | `sb $zero, %gp_rel(D_800A3565)($gp)` | D_800A3565 | 0x800a3565 |
| 80071334 | `sb $zero, %gp_rel(D_800A3562)($gp)` | D_800A3562 | 0x800a3562 |
| 80071350 | `lw $v1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80071388 | `lw $v1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 800713D4 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 800713E4 | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80071408 | `lbu $v0, %gp_rel(D_800A3561)($gp)` | D_800A3561 | 0x800a3561 |
| 8007147C | `lw $v1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 800714D0 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 800714E0 | `sh $zero, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 800714E4 | `sb $s6, %gp_rel(D_800A3562)($gp)` | D_800A3562 | 0x800a3562 |
| 800714F8 | `sb $s6, %gp_rel(D_800A3565)($gp)` | D_800A3565 | 0x800a3565 |
| 800714FC | `sb $s6, %gp_rel(D_800A3562)($gp)` | D_800A3562 | 0x800a3562 |
| 80071500 | `sh $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80071504 | `sh $zero, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |
| 80071510 | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80071520 | `sb $s6, %gp_rel(D_800A3563)($gp)` | D_800A3563 | 0x800a3563 |
| 80071548 | `lw $a0, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 8007155C | `lh $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 8007156C | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 8007158C | `sh $v0, %gp_rel(D_800A35C8)($gp)` | D_800A35C8 | 0x800a35c8 |
| 80071594 | `sh $v0, %gp_rel(D_800A35CA)($gp)` | D_800A35CA | 0x800a35ca |
| 800715AC | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 800715DC | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80071698 | `lh $v1, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 800716B8 | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 800716E0 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071730 | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 800717E0 | `lh $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80071818 | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80071854 | `lw $a1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 800718CC | `lw $v1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 800718FC | `sh $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80071908 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071910 | `sh $zero, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |
| 80071914 | `sb $s6, %gp_rel(D_800A3565)($gp)` | D_800A3565 | 0x800a3565 |
| 80071918 | `sb $s6, %gp_rel(D_800A3562)($gp)` | D_800A3562 | 0x800a3562 |
| 80071924 | `sh $zero, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071928 | `sb $s6, %gp_rel(D_800A3563)($gp)` | D_800A3563 | 0x800a3563 |
| 80071948 | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8007196C | `lhu $a1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071970 | `lh $v1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071974 | `lw $a0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 8007198C | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 800719C0 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 800719C8 | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80071A10 | `lh $v1, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80071A58 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80071A6C | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80071A90 | `lbu $v0, %gp_rel(D_800A3561)($gp)` | D_800A3561 | 0x800a3561 |
| 80071B34 | `lh $v1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071B38 | `lw $a0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80071B50 | `lbu $v0, %gp_rel(D_800A3562)($gp)` | D_800A3562 | 0x800a3562 |
| 80071B60 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80071B78 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80071B88 | `sh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071B8C | `lbu $v0, %gp_rel(D_800A3565)($gp)` | D_800A3565 | 0x800a3565 |
| 80071BAC | `lh $v1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071BB0 | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80071BD4 | `sh $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80071BDC | `sh $v0, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |
| 80071BE4 | `sh $v0, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 80071BE8 | `sh $zero, %gp_rel(D_800A355C)($gp)` | D_800A355C | 0x800a355c |

#### func_80071C20: 1

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80071C20 | `lbu $v0, %gp_rel(D_800A3561)($gp)` | D_800A3561 | 0x800a3561 |

#### func_80071C4C: 30

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80071C58 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071C5C | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80071C60 | `lw $a0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80071CE4 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80071CFC | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80071D20 | `lbu $v0, %gp_rel(D_800A3561)($gp)` | D_800A3561 | 0x800a3561 |
| 80071EB8 | `lh $v1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071EBC | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80071ED4 | `lhu $v0, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 80071EE0 | `sh $v0, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 80071F04 | `sh $v0, %gp_rel(D_800A3550)($gp)` | D_800A3550 | 0x800a3550 |
| 80071F08 | `sh $s0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80071F14 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80071F38 | `lw $a1, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80071F40 | `sw $v1, %gp_rel(D_800A35A0)($gp)` | D_800A35A0 | 0x800a35a0 |
| 80071F68 | `sh $v0, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |
| 80071F80 | `sh $v0, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |
| 80071F84 | `sh $s0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 80071F88 | `sh $s0, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 80071F94 | `sw $v0, %gp_rel(D_800A35A0)($gp)` | D_800A35A0 | 0x800a35a0 |
| 80071F9C | `lh $v0, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80071FA0 | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80071FCC | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80071FDC | `lh $v1, %gp_rel(D_800A3554)($gp)` | D_800A3554 | 0x800a3554 |
| 80071FE0 | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80071FF8 | `lh $v0, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 80071FFC | `lw $v1, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80072028 | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80072038 | `lh $v1, %gp_rel(D_800A3558)($gp)` | D_800A3558 | 0x800a3558 |
| 8007203C | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |

#### func_80072084: 1

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80072084 | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |

#### func_800720AC: 1

| address | instruction | symbol | resolved |
|---|---|---|---|
| 800720AC | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |

#### func_800720D4: 1

| address | instruction | symbol | resolved |
|---|---|---|---|
| 800720D4 | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |

#### func_800720FC: 62

| address | instruction | symbol | resolved |
|---|---|---|---|
| 800720FC | `lw $v0, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80072100 | `lw $v1, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 800721A4 | `lw $v1, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 800721A8 | `lw $a1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 800721F8 | `lbu $v1, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80072208 | `lw $v1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80072234 | `lh $v0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 80072238 | `lh $v1, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 80072250 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 8007226C | `lh $v0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 8007227C | `lh $v1, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 800722D0 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 800722E8 | `lh $v0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 800722EC | `lh $v1, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 800723B8 | `lw $a1, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 800723D4 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 800723F8 | `lbu $v1, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80072414 | `lhu $a0, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |
| 8007242C | `lhu $v1, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 80072508 | `lw $a0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8007255C | `lw $a1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 8007258C | `lw $v1, %gp_rel(D_800A35C0)($gp)` | D_800A35C0 | 0x800a35c0 |
| 800726C8 | `lw $v1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 800726DC | `lbu $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80072700 | `lw $v1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 80072714 | `lh $v0, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 80072734 | `lh $v0, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 80072744 | `lh $v0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 80072748 | `lh $v1, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 8007276C | `sh $v0, %gp_rel(D_800A3584)($gp)` | D_800A3584 | 0x800a3584 |
| 80072778 | `sh $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 8007278C | `lh $v0, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 80072798 | `sh $v0, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 800727A0 | `lw $v1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 800727C0 | `lhu $v0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 800727E8 | `lhu $v0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 800727F4 | `sh $v0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 800727F8 | `lh $v1, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 8007280C | `sh $zero, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 80072820 | `sh $v0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 80072824 | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 800728E8 | `lh $v0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 800728F8 | `lh $v0, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 80072908 | `lbu $v0, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 80072934 | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 800729D4 | `lh $v1, %gp_rel(D_800A3578)($gp)` | D_800A3578 | 0x800a3578 |
| 800729E4 | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80072A08 | `lw $v1, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 80072A60 | `sh $zero, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 80072A84 | `sh $v0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 80072A88 | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80072A90 | `sh $v0, %gp_rel(D_800A35C8)($gp)` | D_800A35C8 | 0x800a35c8 |
| 80072A98 | `sh $v0, %gp_rel(D_800A35CA)($gp)` | D_800A35CA | 0x800a35ca |
| 80072AA8 | `lw $v0, %gp_rel(D_800A35B0)($gp)` | D_800A35B0 | 0x800a35b0 |
| 80072AC4 | `lw $v0, %gp_rel(D_800A354C)($gp)` | D_800A354C | 0x800a354c |
| 80072ADC | `lh $v0, %gp_rel(D_800A359C)($gp)` | D_800A359C | 0x800a359c |
| 80072AE0 | `lh $v1, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 80072AE4 | `lh $a3, %gp_rel(D_800A3598)($gp)` | D_800A3598 | 0x800a3598 |
| 80072B1C | `lw $v1, %gp_rel(D_800A35BC)($gp)` | D_800A35BC | 0x800a35bc |
| 80072B2C | `lw $a0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80072B54 | `lw $v0, %gp_rel(D_800A3568)($gp)` | D_800A3568 | 0x800a3568 |
| 80072B74 | `sw $v0, %gp_rel(D_800A35A0)($gp)` | D_800A35A0 | 0x800a35a0 |

#### func_80072BC4: 1

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80072C24 | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |

#### func_80072CD4: 1

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80072D0C | `lw $v0, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |

#### func_80072E10: 4

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80072E4C | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 80072E7C | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 80072EAC | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 80072ED8 | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |

#### func_80072F30: 0

none

#### func_80072FCC: 1

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80072FE4 | `lh $v0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |

#### func_80073060: 6

| address | instruction | symbol | resolved |
|---|---|---|---|
| 800730A4 | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 800730EC | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 8007310C | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 8007312C | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 8007315C | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 8007319C | `lh $a0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |

#### func_80073200: 5

| address | instruction | symbol | resolved |
|---|---|---|---|
| 80073204 | `lw $v1, %gp_rel(D_800A35A8)($gp)` | D_800A35A8 | 0x800a35a8 |
| 80073290 | `lh $v0, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 800732A4 | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |
| 80073448 | `lh $v1, %gp_rel(D_800A3580)($gp)` | D_800A3580 | 0x800a3580 |
| 8007345C | `lw $v1, %gp_rel(D_800A35C4)($gp)` | D_800A35C4 | 0x800a35c4 |

#### func_8006E49C (outside neighbour): 0

none

#### func_8007352C (outside neighbour): 0

none

## Prong (ii): the original compiler agrees (cc1psx calibration)

### Method

1. `mipsel-linux-gnu-cpp` (the build's CPP flags) on `P3_1.c` -> `psx/P3_1.i`.
2. Storage classes as the ORIGINAL file must have had them (research-common-gp.md: ASPSX gp-addresses only
   symbols defined in the assembling file; `.comm` gets gp at the base only, `.lcomm`/`.sdata` at any offset).
   `psx/mkdef.py` -> `psx/P3_1.def.i`: each listed symbol's first `extern` becomes a definition, later
   `extern`s are dropped. Objects the target reads at `sym+k` (D_800A3560 union, D_800A3588[2], D_800A358C[2],
   D_800A3590[2], D_800A35C8[2]) -> `static` (.lcomm); D_800A32E8/9 (below 0x800A3308, initialized small data)
   -> initialized; every other listed symbol -> tentative (.comm). Log: `psx/P3_1.def.log`. Calibration input only.
3. `bash tools/cc1psx_wrapper.sh -O2 <G> -funsigned-char -quiet -mcpu=3000 -mips1 -msoft-float -w < P3_1.def.i`
   at -G8 and -G0. Banner in both outputs: `# GNU C 2.7.2.SN.1 [AL 1.1, MM 40] Sony Playstation compiled by GNU C`,
   `# Cc1 arguments (-G value = 8|0, Cpu = 3000, ISA = 1)`. stderr empty both runs.
4. Assembled with the ORIGINAL Sony ASPSX 2.34 (`tmp/func_80036140/r/aspsx.sh`, dosemu2; banner
   "Psy-Q ASPSX version 2.34", 0 errors) at the matching -G; the PsyQ LNK object is parsed
   (`lnk2.py` = tmp/func_80036140/lnk.py + local symbols + expression constants) and compared per function with
   the shipped words (`psxcal.py`: relocation immediates masked, sequence-aligned; gp accesses resolved to
   absolute addresses including section-relative `.sbss/.sdata + k` expressions).

### Result

| function | target words | gp target | cc1psx -G8 gp | gp addresses | cc1psx -G0 gp | masked-word diffs cc1psx -G8 | same, our cc1 -G8 via ASPSX |
|---|---|---|---|---|---|---|---|
| func_8006E534 | 222 | 45 | 45 | EQUAL | 0 | 18 | 16 |
| func_8006E8AC | 8 | 1 | 1 | EQUAL | 0 | 1 | 0 |
| func_8006E8CC | 33 | 0 | 0 | EQUAL | 0 | 3 | 3 |
| func_8006E950 | 54 | 0 | 0 | EQUAL | 0 | 7 | 6 |
| func_8006EA28 | 41 | 0 | 0 | EQUAL | 0 | 1 | 1 |
| func_8006EACC | 80 | 11 | 11 | EQUAL | 0 | 11 | 1 |
| func_8006EC0C | 58 | 13 | 13 | EQUAL | 0 | 10 | 9 |
| func_8006ECF4 | 209 | 16 | 16 | EQUAL | 0 | 16 | 19 |
| func_8006F038 | 50 | 3 | 3 | EQUAL | 0 | 8 | 5 |
| func_8006F100 | 266 | 16 | 16 | EQUAL | 0 | 107 | 17 |
| func_8006F528 | 277 | 17 | 17 | EQUAL | 0 | 53 | 25 |
| func_8006F97C | 515 | 25 | 25 | EQUAL | 0 | 119 | 36 |
| func_80070188 | 698 | 53 | 53 | EQUAL | 0 | 146 | 47 |
| func_80070C70 | 194 | 8 | 8 | EQUAL | 0 | 41 | 19 |
| func_80070F78 | 810 | 81 | 81 | EQUAL | 0 | 203 | 90 |
| func_80071C20 | 11 | 1 | 1 | EQUAL | 0 | 2 | 2 |
| func_80071C4C | 270 | 30 | 30 | EQUAL | 0 | 92 | 21 |
| func_80072084 | 10 | 1 | 1 | EQUAL | 0 | 0 | 0 |
| func_800720AC | 10 | 1 | 1 | EQUAL | 0 | 1 | 1 |
| func_800720D4 | 10 | 1 | 1 | EQUAL | 0 | 1 | 1 |
| func_800720FC | 690 | 62 | 62 | EQUAL | 0 | 151 | 59 |
| func_80072BC4 | 68 | 1 | 1 | EQUAL | 0 | 10 | 10 |
| func_80072CD4 | 79 | 1 | 1 | EQUAL | 0 | 19 | 19 |
| func_80072E10 | 72 | 4 | 4 | EQUAL | 0 | 8 | 6 |
| func_80072F30 | 39 | 0 | 0 | EQUAL | 0 | 5 | 5 |
| func_80072FCC | 37 | 1 | 1 | EQUAL | 0 | 4 | 4 |
| func_80073060 | 104 | 6 | 6 | EQUAL | 0 | 34 | 19 |
| func_80073200 | 203 | 5 | 5 | EQUAL | 0 | 35 | 23 |
| **total** | | **403** | **403** | 28/28 EQUAL | **0** | 1106 | 464 |

- **At -G8 cc1psx emits every listed gp access**: 403/403, and per function the multiset of
  accessed ADDRESSES equals the target's (28/28), including all of func_80070F78's 14 constant-offset slot
  accesses (target D_800A3561 x3, D_800A3562 x5, D_800A3563 x2, D_800A3565 x4).
- **At -G0 it emits none**: 0 gp accesses (ASPSX -G0); also 0 when the -G0 compiler output is
  assembled with ASPSX -G8 (cc1psx -G0 writes the `.comm`/`.lcomm` lines after the code, and ASPSX decides
  per instruction).
- Compiler-level form, func_80070F78 (direct `op $r,D_800A3560+k` operands, i.e. the gp-capable form):
  cc1psx -G8: +1 x3, +2 x5, +3 x2, +5 x4 (14 = the target's 14 gp slot accesses); cc1psx -G0: +1 x3, +2 x2,
  +5 x1 (6; the rest go through a `lui/addiu` base register). Our cc1 gives exactly the same counts at both
  -G values on the same input.

### How cc1psx's -G8 output compares with the target, vs our cc1's

- cc1psx is not byte-identical to the target: 1106 masked-word differences over 5118 target words through
  ASPSX. Our cc1 on the same input through the same ASPSX: 464 (differences between ASPSX and the project maspsx post-processing, not analysed further; plus
  D_800A32E8/9: our ELF cc1 puts initialized small data in `.data`, cc1psx in `.sdata`, so that harness
  leaves 6 of our 403 unconverted). Through the project pipeline (maspsx + sdata lists) our cc1 is
  linked-byte identical for all 28.
- The cc1psx-vs-our-cc1 difference is allocation/scheduling skew, not the gp model (`psx/skew-G8.txt`):
  21/28 functions have the same instruction multiset after renaming registers; the other 7 differ by
  0-9 instructions in length (func_80070F78 666 vs 670). This matches the skew recorded for the earlier -G8
  calibrations (func_80034708 F12, func_80036140 calib: "known cc1psx/KMC scheduling skew").
- So cc1psx agrees with the target and with our cc1 on the property prong (ii) tests (which accesses are
  gp-relative, at which addresses), and differs from both only in allocation/scheduling.

## Q54: gp-free functions byte-identical at -G8 and -G0

| function | our cc1, project pipeline (-G8 TU vs -G0 build) | cc1psx + ASPSX (-G8 vs -G0) |
|---|---|---|
| func_8006E8CC | identical (words + relocations) | identical words; relocations identical after resolving `.text+N` to function+offset |
| func_8006E950 | identical (words + relocations) | identical words; relocations identical after resolving `.text+N` to function+offset |
| func_8006EA28 | identical (words + relocations) | identical words; relocations identical after resolving `.text+N` to function+offset |
| func_80072F30 | identical (words + relocations) | identical words; relocations identical after resolving `.text+N` to function+offset |

- Our cc1: `cmp2.py` P3_1.o (-G8) vs build/src/text1b_tu1c.o (-G0): identical; also the union source built both
  ways (union_B_G0 vs union_B_G8): only func_80070F78 differs in the whole range.
- cc1psx: `g8g0same.py` def.psx-G8.obj vs def.psx-G0.obj. func_8006EA28 identical outright; func_8006E8CC,
  func_8006E950 and func_80072F30 have identical words, and their only relocation difference is the
  section-relative target of an intra-TU `j`/`jal` (`.text+980` vs `.text+1164` etc.), which resolves to the same
  function+offset in both objects (func_8006E8CC, func_8006E8CC+0x3c, func_80072F30+0x64).

## Reproduce (WSL, repo root)

```
D=tmp/audit-2026-09-29/g8-screen; P=$D/psx
python3 $D/mkdef.py
for G in -G8 -G0; do bash tools/cc1psx_wrapper.sh -O2 $G -funsigned-char -quiet -mcpu=3000 -mips1 -msoft-float -w \
    < $P/P3_1.def.i > $P/def.psx$G.s; done
python3 $D/psxcal.py $P/def.psx-G8.s $P/def.psx-G8.obj -G8
python3 $D/psxcal.py $P/def.psx-G0.s $P/def.psx-G0.obj -G0
python3 $D/g8g0same.py $P/def.psx-G8.obj $P/def.psx-G0.obj
python3 $D/gen_evidence.py
```
