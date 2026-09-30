# Jump-table alignment in the original toolchain (investigation, 2026-09-30)

Question from the owner: func_80058580's three jump tables sit 4 mod 8 with zero words between
them. Would a GLOBAL fix (modelling how Sony's tools align data) be legitimate, or a cheat?
This note records what was measured. It decides nothing: adopting any model is a substrate
change that needs its own owner ruling.

Evidence and scripts: `memory/grind/func_80058580/aspsx-align-check/`.

## 1. Sony's own tools: alignment is relative to each object (CONFIRMED)

PsyQ 3.5 binaries (ASPSX 2.34, PSYLINK 2.37, CC1PSX) from
`github.com/mkst/esa/releases/download/psyq-binaries/psyq3.5.tar.gz` (the source the maspsx
test harness uses), run under headless DOSBox (`dos.sh`).

- **CC1PSX** emits `.rdata` then `.align 3` before every switch table (`cc1psx-switch.s`), the
  same directive our cc1 emits.
- **ASPSX + PSYLINK** (`t1.sh`, `t3.sh`): object A = one word; object B = word, `.align 3`, word;
  object C = `.align 3`, word. Linked A,B,C at 0x80010000:

      80010000 11111111   A
      80010004 b0b0b0b0   B starts 4 mod 8: the linker does not 8-align the object
      80010008 00000000   pad
      8001000C b1b1b1b1   B's .align 3 -> 8 relative to B's start, i.e. 4 mod 8 absolute
      80010010 c0c0c0c0   C

  GNU as/ld would put B's second word at 0x80010008. Sony's pads relative to the object and
  places objects on 4-byte boundaries. func_80058580's tables (0x8001585C / 84 / 9C with a zero
  word after the first two) are exactly this pattern.

## 2. Our tree under that model: 7 sites disagree, and they form clean runs

Under the model every jump table in one original file has the same phase (address mod 8) as that
file's start. `jtsurvey.sh` marks every `.align 3` site in a byte-identical build; `jtphase.py`
lists all 85 tables (compiled + transcribed) by address, phase, our object and owning function
(`jtphase.txt`).

- No function's tables span two phases (42 functions with tables; 0 mixed).
- Building with the model applied to OUR file boundaries (`align_exp.sh global`: no per-file
  `.align 3 -> .align 2` sed, object rodata 4-aligned) does NOT match: 5 objects grow (28 bytes,
  7 tables). Every one is a phase change inside one of our files:
  - code6cac.o: func_80019568 (0) | func_8001C8DC.. (4)
  - code6cac_b.o: func_80026DA4 (4) | func_8002738C..func_80033498 (0) | func_800344B4.. (4)
  - code6cac_b5: the CD module (func_80036140, func_80036940) is phase 0 after phase-4 neighbours
  - text1a_pre.o: func_80040304 (4) | func_80040D48 (0) | text1a_c func_80042504 (4)
  - text1b.o: func_8006B578..8006ECF4 (0) | func_800747D8, func_80077374 (4)
  - text1b_b.o: func_80077B30, func_80077D94 (4) | prnt (0) | sprintf (4)
  - the transcribed INCLUDE_ASM tables in text1a_b_pre_rodata.c: 0x8001541C/545C and
    func_80058580's 0x8001585C/84/9C (4) | 0x800158F8/5940 (0)
- Where a boundary is independently checkable it lands on a natural one: prnt and sprintf are
  separate PsyQ LIBC objects; the CD-module steppers begin a new phase.

Reading: the model is consistent with every table in the binary, provided the original file
boundaries differ from ours at these points. Our read-only-data boundaries come from the
2026-06-09 rodata re-attribution, which placed data where it was convenient; the per-file sed
(`RODATA_ALIGN2_FILES`) compensates for that.

## 3. What adopting it would take, and where it would become a cheat

- **Global, uniform rule** (keep in-object `.align 3` padding; place each object's `.rodata` on a
  4-byte boundary, e.g. `objcopy --set-section-alignment .rodata=4`), replacing the per-file sed.
  This models the original tools, like maspsx; it is not per-function.
- **File boundaries from evidence.** At each phase change above, the original file boundary lies
  somewhere between the two functions. Its exact byte position must come from data ownership
  (which function references each const/string between them), text/data adjacency and library
  object boundaries. Choosing a boundary only because it makes the bytes line up is the banned
  speculative rodata reorder (`.claude/rules/no-new-park-categories.md`). Where ownership
  evidence is ambiguous, the model gives no licence.
- **Owner ruling** before any of it lands (substrate change).
