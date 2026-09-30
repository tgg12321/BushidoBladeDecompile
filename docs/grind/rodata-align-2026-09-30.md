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
  - text1a_pre.o: func_80040304 (4) | func_80040D48 (0) | text1a_c func_80042504 (4)
  - text1b.o: func_8006B578..8006ECF4 (0) | func_800747D8, func_80077374 (4)
  - text1b_b.o: func_80077B30, func_80077D94 (4) | prnt (0) | sprintf (4)
  (Correction, same day: an earlier draft also listed the CD module in code6cac_b5 and the
  transcribed tables in text1a_b_pre_rodata.c here. Neither fails the model today: code6cac_b5 is
  already its own object starting at phase 0, and transcribed tables are `.word` data with no
  `.align 3`. The second one matters later; see section 4, "Predicted".)
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

## 4. Evidence check at every site (2026-09-30, second pass)

Question from the owner: pursue this only if it makes sense, fits the established rules, and is
not a cheat. The rule that decides it is `.claude/rules/jtbl-rodata-split-infrastructure.md`:
evidence-based TU re-attribution is the legitimate (SOTN) path; a reorder chosen to force a match
is banned. So at each site the test is whether the boundary comes from evidence rather than from
the bytes it produces.

Tools (`memory/grind/func_80058580/aspsx-align-check/poc/`): `owners.py` maps every data symbol
to the functions whose target asm (`asm/funcs/*.s`, independent of our C) references it;
`sitewin.py` shows one site's candidate window; `rodump.py` dumps the original bytes.

**Emission order, measured.** CC1PSX and our cc1 both emit a TU's rodata in function order:
each function's string literals, then its jump table (`order-test.c`, `order-test.cc1psx.s`,
`order-test.cc1.s`). Strings nobody references ("SOUND ID:%d\n", "CHANBARA", "PRACTICE", `""`)
are what GCC 2.7.2 leaves behind for debug prints it later deletes as dead code: the string is
emitted at RTL expansion, before jump optimisation removes the call.

**Three facts decide each site.**
1. *Existence.* Two jump tables at different phases cannot come from one original file. Every
   site below is a phase change, so each boundary's existence is read from the original bytes.
2. *Rodata position.* The new file must start at an item start whose phase equals its first
   table's phase. Item ownership removes more candidates: a TU's rodata is contiguous, and a
   string literal is local to its TU.
3. *Text position.* Splitting a file keeps every section in the same order, so where the text cut
   falls inside the window changes no byte.

| # | Our object | Phase change (table owners) | Rodata start of the new file | How it is fixed |
|---|---|---|---|---|
| 1 | code6cac.o | func_80019568 (0) / func_8001C8DC (4) | 0x800100A4, 0x800100B4 or 0x800100C4 | Any of the three: all are phase 4 and give identical bytes. |
| 2 | code6cac_b.o | func_80026DA4 (4) / func_8002738C (0) | 0x80010478 | Ownership. The only other phase-valid start, 0x80010498, would put "ILLEGAL GUN MOTION : %d\n" in the previous file, but its user func_8002A458 comes after func_8002738C. |
| 3 | code6cac_b.o | func_80033498 (0) / func_800344B4 (4) | 0x8001081C or 0x80010834 | Either (0x80010828 is phase 0); both give identical bytes. |
| 4 | text1a_pre.o | func_80040304 (4) / func_80040D48 (0) | 0x80010DB8 | Forced: no bytes between the two tables. |
| 5 | text1b.o | func_8006ECF4 (0) / func_800747D8 (4) | 0x80015A0C | Forced: no bytes between the two tables. |
| 6 | text1b_b.o | func_80077D94 (4) / prnt (0) | 0x80015A68 | Forced: no bytes between. prnt is its own PsyQ LIBC module. |
| 7 | text1b_b.o | prnt (0) / sprintf (4) | 0x80015C7C | Ownership: sprintf's own two hex strings start it (0x80015C90 is phase 0; 0x80015CA4 would leave sprintf's strings in prnt's module). |

The model is contradicted nowhere. At every site the bytes prove the boundary exists. Where more
than one rodata position survives the evidence, all of them produce the same bytes, so the bytes
never choose between plausible layouts. The text cut is byte-neutral everywhere: its range is
recorded, and the placement inside it is a documented convention.

**Predicted, not needed today** (these tables are still transcribed `.word` data):
func_80058580's file must start at 0x8001541C or 0x8001585C (identical bytes; 0x8001585C follows
"Destruction tiny model.\n", owned by func_80054604). func_80065800's tables need a file starting
at 0x800158E0 or 0x800158F8. Under the model, the zero words between func_80058580's tables are
alignment padding, so no hand-written zero pad is needed when it becomes C.

## 5. Proof of concept: byte-identical with no per-file sed

`mkpoc.py`, run on a scratch copy of HEAD (`/tmp/claude-0/poc`; never on main), applies:
- `RODATA_ALIGN2_FILES` emptied (the per-file `.align 3 -> .align 2` sed is gone);
- one uniform rule for every C object: after `as`, `objcopy --set-section-alignment .rodata=4`;
- the five objects split at the section 4 positions (sites 1-7). The split is done on the
  assembler stream (`splitasm.py`), a proxy for splitting the .c file at the same point.

Result (`poc-result.txt`): **the linked .bin is identical to the current oracle build's .bin**
(sha1 42fce5af..., the same file the EXE with SHA1 62efab4f... is built from). The rodata
placement differs from main only in the new part boundaries.

## 6. What adoption would take (as proposed to the owner)

- Split the five .c files for real at the section 4 boundaries (moves only, no code changes),
  adding the parts to `bb2.ld` next to their originals in every section list.
- Makefile and `engine/pipeline.py`/`engine/buildconfig.py`: replace the per-file sed with the
  uniform rule, with an `engine test` case for it.
- Full oracle verify, a layer-2 review of the substrate change, and a `docs/grind/decisions.md`
  record.
- Follow-up candidates it enables (each judged on its own): `code6cac_b_rodata_pre.c`'s
  `_bb2_101C_pre_lead` zero word is an alignment pad under the model; func_80058580's tables.

## 7. Adopted (owner ruling 2026-09-30, "Yes go ahead")

Rule: `.claude/rules/rodata-object-alignment.md`. Record: docs/grind/decisions.md 2026-09-30 OWNER
RULING — object-relative rodata alignment.

- **Build rule.** `Makefile`: `RODATA_ALIGN2_FILES` and its sed are gone; every C object gets
  `$(OBJCOPY) $(RODATA_OBJ_ALIGN) $@` after `as`. `engine/pipeline.py` / `engine/buildconfig.py`
  mirror it, pinned by `test_rodata_object_alignment` in `engine/test_engine.py`.
- **The seven splits** (moves only, done by `memory/grind/func_80058580/aspsx-align-check/poc/splitc.py`; each new file starts with the parent's
  include block, then the parent's earlier declarations its functions use, verbatim and in order,
  then `extern` declarations derived from definitions in the parent). Each new part follows its
  parent in every section list of `bb2.ld`; the -G8 and expand-lb per-file settings are inherited.

  | New file | Starts at (text) | Rodata start | Text window (cut is byte-neutral inside it) |
  |---|---|---|---|
  | code6cac_tu2.c | `INCLUDE_RODATA D_800100A4`, then func_8001979C | 0x800100A4 | after func_80019568 .. up to func_8001C8DC |
  | code6cac_b_tu2.c | `INCLUDE_RODATA D_80010478`, then func_800272FC | 0x80010478 | after func_80026DA4 .. up to func_8002738C |
  | code6cac_b_tu3.c | `INCLUDE_RODATA jtbl_8001084C`, func_800344B4 | 0x8001081C | after func_80033498 .. up to func_800344B4 |
  | text1a_pre_tu2.c | func_80040D48 | 0x80010DB8 | after func_80040304 .. up to func_80040D48 |
  | text1b_tu2.c | func_800747D8 | 0x80015A0C | after func_8006ECF4 .. up to func_800747D8 |
  | text1b_b_tu2.c | prnt (its strings D_80015A68/7C/84) | 0x80015A68 | PsyQ module start of prnt |
  | text1b_b_tu3.c | sprintf | 0x80015C7C | PsyQ module start of sprintf |

  Convention for the text cut: at the item that begins the new file's rodata (for sites 1 and 3 the
  earliest byte-equivalent position, where the INCLUDE_RODATA blob already sits), or at the PsyQ
  module start for library code.
- **Checks.** Full clean build: `build/bb2.bin` sha1 42fce5aff1490a579e919b68af56ebc5b0dc657f
  (unchanged) and the header-prefixed EXE sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa (the oracle).
  The implicit-function-declaration set of each original file equals the union over its parts
  (no function lost a declaration). `engine test`: 811 pass, 1 fail (the pre-existing
  path-contains-space environment check).
- **Records relocated** (file field only): 12 queue items, 16 grind `state.json` files, one
  grinder scope line, and 24 reviewed assembly-region grants in `tools/canonical_asm_regions.json`,
  each moved only after its island hashes recomputed from the new file equalled the reviewed ones.

## 8. func_80058580's tables and the text1b.c / text1b_tu1c.c boundary (2026-09-30, after adoption)

Goal: let func_80058580's three switch tables (0x8001585C..0x800158B4) be compiler-emitted at their
real address. Byte-neutral while the function is INCLUDE_ASM.

**The one boundary the bytes prove.** func_80058580's tables are phase 4; func_80065800's
(0x800158F8, 0x80015940) and func_8006B578's (0x80015988) are phase 0. So an original TU boundary
lies between func_80058580 and func_80065800 (rule condition 1). Its rodata position lies in
0x800158B4..0x800158F8; the text cut is byte-neutral inside the window. Placed at snd_Init (new
file `text1b_tu1c.c` from snd_Init's extern block to the end of the old file): the next rodata item,
0x800158B4 "common_vab start:%08x\n", and 0x800158CC "vab id:%d mistake\n" belong to the sound-bank
loader that starts at snd_Init (it calls SsInit / SsSetTickMode / SsSetReservedVoice). Alternatives
with identical bytes: the cut anywhere after func_80058580 up to func_80060E38.

**No boundary before func_80058580.** Every table from text1a_c's start (0x80010DD4) through
0x8001589C is phase 4, and the item just before its tables (0x80015840 "Destruction tiny model.\n")
belongs to func_80054604 in text1b.c, so nothing proves a boundary there. func_80058580 stays the last
item of `text1b.c`, and its three tables moved verbatim from `text1a_b_pre_rodata.c` to just before
its INCLUDE_ASM line. text1b.o had no other rodata, so its rodata now starts at 0x8001585C with those
tables. (A first version split func_80058580 into its own file; layer-2 FAILed it under condition 1
and this is the remedy.)

| File | Text | Rodata |
|---|---|---|
| text1b.c | unchanged, still ends with func_80058580 | 0x8001585C..0x800158B4 (the three tables) |
| text1a_b_pre_rodata_b.c | none (transcribed data, not a TU) | 0x800158B4..0x80015988, the old file's tail |
| text1b_tu1c.c | snd_Init .. the function before func_800747D8 | 0x80015988.. (the tables formerly in text1b.o) |

Caveat: under the model text1b_tu1c.c spans at least two original TUs. snd_LoadCommonVab's
phase-4 strings (0x800158B4) cannot share a TU with func_80065800's phase-0 tables (0x800158F8 is 0x44
past them). This is harmless while those items stay transcribed, and must be resolved (another
evidence-placed boundary between them) when func_80065800 is compiled.

Check (`poc/tbltest.sh`, scratch copy): with the session-2 C candidate in place of the transcribed
arrays, text1b.o's compiled .rodata is 0x58 bytes at 0x8001585C with zero words at +0x24 and +0x3C,
the original's layout (jtbl_8001585C[9], jtbl_80015884[5]). The pads come from the object-relative
`.align 3`; nothing hand-written.
