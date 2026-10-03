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
not a cheat. The rule that decides it is `pre-slim-2026-10-01:.claude/rules/jtbl-rodata-split-infrastructure.md`:
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
  | code6cac_b_tu3.c | func_800343F0, then `INCLUDE_RODATA jtbl_8001084C`, func_800344B4 (moved from func_800344B4 by owner ruling Q65, per-file gp model, cut outcome (i): D_800A3140 is reached gp by func_8002AB08 and by lui/%lo in func_800343F0) | 0x8001081C | after func_80033498 .. up to func_800344B4 |
  | text1a_pre_tu2.c | func_80040D48 | 0x80010DB8 | after func_80040304 .. up to func_80040D48 |
  | text1b_tu2.c (merged into text1b_b.c, owner ruling Q67, Q65 step 11; text1b_b.c now starts here) | func_800747D8 | 0x80015A0C | after func_8006ECF4 .. up to func_800747D8 |
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

## 9. The text1b / text1b_tu1c boundary moves to func_80061064 (2026-09-30, closes section 8's caveat; moved again to func_80060A68 by the Q65 adoption, step 03)

Section 8 placed the one boundary the bytes prove between func_80058580 and func_80065800 at
snd_Init, with text1b_tu1c.c's rodata starting at 0x800158B4, and noted that this position fails
the model once func_80065800's tables are compiled. This section applies the rule's conditions
to that boundary again, with func_80065800's tables as its first compiled item, and moves it.
It supersedes section 8's snd_Init placement; everything else in section 8 stands.

**Existence (condition 1).** Unchanged from section 8: func_80058580's tables (0x8001585C,
0x80015884, 0x8001589C) are phase 4 and func_80065800's (0x800158F8, 0x80015940) phase 0, so they
come from different original files.

**Rodata position (condition 2).** The file holding func_80065800 must start its rodata at an
item start in 0x800158B4..0x800158F8 whose phase equals its first table's, phase 0. The items,
with their owners (the function whose code references them):

| Address | Phase | Item | Owner |
|---|---|---|---|
| 0x800158B4 | 4 | "common_vab start:%08x\n" | snd_LoadCommonVab (printf) |
| 0x800158CC | 4 | "vab id:%d mistake\n" | func_8005C2A8 (printf) |
| 0x800158E0 | 0 | "eff prim over :%d \n" (20 bytes), then 4 zero bytes | func_80061064 (printf) |
| 0x800158F8 | 0 | jtbl_800158F8, then jtbl_80015940 | func_80065800 |

Surviving positions: 0x800158E0 and 0x800158F8. At 0x800158E0 the 4 zero bytes at 0x800158F4
are exactly the object-relative `.align 3` pad before the first table (0x18 past the start). At
0x800158F8 they must be an item of the previous file (an unreferenced "" string, the kind section
4 describes). Both give identical bytes. 0x800158B4 and 0x800158CC (phase 4) do not survive, so
snd_LoadCommonVab and func_8005C2A8 belong to the file before the boundary, text1b.c, whose rodata
starts at 0x8001585C (phase 4) with func_80058580's tables; their strings follow those tables
contiguously and in function order. The position used is 0x800158E0, the earliest byte-equivalent
one (the convention of sites 1 and 3 in section 7); 0x800158F8 is the recorded alternative.

**Text cut (condition 3).** With rodata starting at 0x800158E0, the new file contains
func_80061064 (owner of 0x800158E0) and not func_8005C2A8 (owner of 0x800158CC). The window is
after func_8005C2A8 up to func_80061064; the functions in between own no rodata, so the cut is
byte-neutral anywhere inside it. By the convention (cut at the item that begins the new file's
rodata) it sits at func_80061064's extern block. The Q65 adoption (step 03, per-file-gp-model.md
"Boundary move instead of a merge") moves the cut inside this window to func_80060A68, the one
position that puts every gp user of text1b_tu1c's per-file objects on one side; the rodata start
0x800158E0 is unchanged (func_80060A68 .. func_80060E38 own no rodata).

**Moves only (condition 4).** Done by pre-slim-2026-10-01:memory/grind/func_80065800/tools/move.py:
- snd_Init's extern block .. func_80060E38 (2927 lines, 51 functions) move verbatim from
  text1b_tu1c.c to the end of text1b.c, the file they were split from in section 8. Two
  declarations of the moved block are dropped because text1b.c already has them from
  include/gte.h:27-28 (a second definition does not compile): the CVECTOR and DVECTOR typedefs.
- text1b_tu1c.c's header is rebuilt by the section 7 split tool
  (memory/grind/func_80058580/aspsx-align-check/poc/splitc.py): its include block, the earlier
  declarations the remaining functions use, verbatim and in order, then declarations derived from
  definitions.
- D_800158E0 (24 bytes, text unchanged) moves from text1a_b_pre_rodata_b.c to text1b_tu1c.c in
  place of func_80061064's `extern s32 D_800158E0;` (the two would conflict), and func_80065800's
  two transcribed tables move verbatim to just before its INCLUDE_ASM line, as section 8 did for
  func_80058580's. text1a_b_pre_rodata_b.c keeps the two sound-bank strings; its comments are
  updated.

| File | Text | Rodata |
|---|---|---|
| text1b.c | text1a_c2 + text1a_b + text1a_b_pre_rodata + sound merged in ahead of it (owner ruling Q67, Q65 step 09), then unchanged head, then snd_Init .. func_80060768 | 0x800152B4..0x800158B4 (step 09: text1a_c2 0x800152B4, text1a_b 0x800153B4, text1a_b_pre_rodata 0x800153F0, sound none, text1b 0x8001585C = func_80058580's tables) |
| text1a_b_pre_rodata_b.c | none (transcribed data) | 0x800158B4..0x800158E0, the sound-bank strings |
| text1b_tu1c.c | func_80060A68 .. the function before func_8006E534 (func_80060A68 .. func_80060E38 moved here by owner ruling Q65: per-file gp model, boundary move in the recorded window) | 0x800158E0.. (D_800158E0, func_80065800's tables, func_8006B578's table and string) |

**Checks.** Full rebuild: EXE sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa (the oracle);
text1b_tu1c.o .rodata at 0x800158E0 (0xd0 bytes), text1a_b_pre_rodata_b.o at 0x800158B4 (0x2c).
Implicit function declarations (pre-slim-2026-10-01:memory/grind/func_80065800/tools/implicit_cmp.sh, cc1 with
`-Wimplicit`): the union over text1b.c and text1b_tu1c.c is the same 24 names before and after.
Per file, func_80062020 and srand move from text1b_tu1c.c to text1b.c with their only implicit
call sites, and LoadImage's text1b_tu1c.c entry disappears because text1b.c already declared it
implicitly before the move. Records relocated (file field only, section 7 procedure,
pre-slim-2026-10-01:memory/grind/func_80065800/tools/relocate.py): the queue items func_8005C8A8 and func_8005D554,
and the grind state.json of func_8005C8A8, func_8005D554, func_8005D814, func_8005E54C and
func_8005F1C8. No canonical_asm_regions entry or scope_allow line names a moved function.

## 10. Inter-object padding: module-end pads in their module's asm (owner rulings Q101/Q102/Q104, 2026-10-03)

Rule: rodata-object-alignment.md § Inter-object padding. Sony's assembler padded each hand-written asm
module's `.text` to 16 bytes; PSYLINK concatenated modules at 4-byte granularity, so the pad words are
module bytes (LIBSCAN word counts include them) and sit at the end of the module's last asm. Spans are
from docs/naming/libscan/libsyms.json; each pad ends exactly at its module's end. Survey and byte proofs:
docs/grind/handoff-2026-10-03-restructure.md Part A.

| Module | Span | Pad words | Where the pad lives |
|---|---|---|---|
| LIBAPI A39 | 800789D8..800789E8 | 1 | asm/funcs/SetSp.s |
| LIBAPI SENDPAD | 80079000..800790C0 | 3 | asm/funcs/func_800790A4.s, after the 4-word stub, under `dlabel D_800790B4` |
| LIBAPI CHCLRPAD | 800790C0..80079120 | 1 | asm/funcs/_remove_ChgclrPAD.s |
| LIBGTE MSC00 | 8007E08C..8007E11C | 2 | asm/funcs/InitGeom.s |
| LIBGTE MSC01 | 8007E11C..8007E1AC | 3 | asm/funcs/SquareRoot0.s |
| LIBGTE MSC06 | 8007E1AC..8007E43C | 1 | asm/funcs/LoadAverageCol.s |
| LIBGTE MSC09 | 8007E43C..8007E4DC | 3 | asm/funcs/SquareRoot12.s |
| LIBGTE MTX_000 | 8007E4DC..8007E5EC | 1 | asm/funcs/MulMatrix0.s |
| LIBGTE MTX_00A | 8007E8DC..8007EA0C | 3 | asm/funcs/ScaleMatrixL.s |
| LIBGTE MTX_01 | 8007EA0C..8007EB4C | 2 | asm/funcs/ApplyRotMatrixLV.s |
| LIBGTE MTX_03 | 8007EB4C..8007EC5C | 1 | asm/funcs/MulMatrix.s |
| LIBGTE MTX_04 | 8007EC5C..8007ED6C | 1 | asm/funcs/MulMatrix2.s |
| LIBGTE MTX_08 | 8007EDBC..8007EEEC | 3 | asm/funcs/ScaleMatrix.s |
| LIBGTE REG04 | 8007EF6C..8007EF8C | 3 | asm/funcs/ReadSZfifo3.s |
| LIBGTE REG09 | 8007EF8C..8007EF9C | 1 | asm/funcs/ReadGeomScreen.s (whole-body asm, owner ruling Q104) |
| LIBGTE REG12 | 8007EFDC..8007EFFC | 2 | asm/funcs/SetGeomOffset.s (whole-body asm, owner ruling Q104) |
| LIBGTE REG13 | 8007EFFC..8007F00C | 1 | asm/funcs/SetGeomScreen.s (whole-body asm, owner ruling Q104) |
| LIBGTE SMP_00 | 8007F00C..8007F21C | 1 | asm/funcs/Lzc.s |
| LIBGTE SMP_02 | 8007F21C..8007F24C | 1 | asm/funcs/RotTransPers.s |
| LIBGTE SMP_03 | 8007F24C..8007F2AC | 3 | asm/funcs/RotTransPers3.s |
| LIBGTE SMP_04 | 8007F2AC..8007F2DC | 2 | asm/funcs/RotTrans.s |
| LIBGTE CMB_00 | 8007F2DC..8007F35C | 2 | asm/funcs/RotTransPers4.s |
| LIBGTE FGO_01 | 8007F35C..8007F5EC | 1 | asm/funcs/RotMatrix.s |
| LIBGTE FGO_03 | 8007F5EC..8007F87C | 1 | asm/funcs/RotMatrixZYX.s |
| LIBGTE FGO_04 | 8007F87C..8007FA1C | 2 | asm/funcs/RotMatrixX.s |
| LIBGTE FGO_05 | 8007FA1C..8007FBBC | 2 | asm/funcs/RotMatrixY.s |
| LIBGTE FGO_06 | 8007FBBC..8007FD5C | 2 | asm/funcs/RotMatrixZ.s |
| LIBGTE PATCHGTE | 8007FEDC..8007FF7C | 1 | display.c's file-scope `_patch_gte` block, after `.word 0x40026800` |

## 11. code6cac_c_ab.c merged into code6cac_c2.c; code6cac_c_ab_pad.c retired (Part A R1/R2, 2026-10-03)

- **R1.** c_ab's mid-file `.section .rodata` `.word 0` (added in f2d6c1995 under the retired per-file
  `.align 2` sed) was deleted: under the object-relative model the next table's `.align 3` emits the
  same zero word (object identical in .text, .rodata 0xCC, relocations and symbols).
- **R2, existence.** c_ab's object started at 0x80010CA4 (phase 4) and c2's first rodata item is a jump
  table at 0x80010D74 (phase 4). The 4 zero bytes at 0x80010D70 are exactly the `.align 3` pad one
  object holding both rodata runs emits; the split needed a phantom 4-byte object
  (code6cac_c_ab_pad.c). The c_ab|c2 boundary at 0x8003B9D0 was the early code6cac_c|c2 split
  (97eb39533, "fix extern conflicts"); 9cb130a8e then cut c_ab out at func_8003AB44 (a CU split for one
  jump table). No rodata evidence supports the 0x8003B9D0 boundary.
- **Survivors** (condition 2; all oracle-identical, survey `memory/grind/restructure-2026-10-03/pad-survey/`):
  the full merge (cut at func_8003AB44, the c_mid|c_ab cut); a cut before func_8003ACB8 with c_ab's
  Q65 tentative commons in both parts; a cut before func_8003AE5C with the commons in the first part
  or both. Cuts with the commons only in the second part fail (func_8003AB44 / func_8003ACB8 reach them
  gp-relative). Positions inside c_mid were not tested; c_mid's `.sdata` and c2's are separated by
  asm/data/93A0E in bb2.ld, so the gp model keeps c_mid distinct.
- **Chosen:** the full merge (removes a boundary, adds none). code6cac_c2.c = c_ab's text verbatim, then
  c2's after its include block. Two declarations reconciled: `func_80020D38` is `void` (its definition,
  src/code6cac_tu2.c), `D_800A3894` is `u8 *` (as in c_ab, c_mid and tu2; c2's `s32` extern dropped).

## 12. Data-only rodata files folded into their owners (restructure step 3, Q106 D4, 2026-10-03)

Object compare `memory/grind/restructure-2026-10-03/step3/objcmp.py` (sections, relocations, globals vs the pre-fold objects) plus the oracle link.
- **ings_strings.c → main/6CF8.c** (0x80010000+0x68): the 5 strings' only referrers are 6CF8.c (printf sites; the
  build date via D_800A30E0); 6CF8.o had no other .rodata and links first. Any position in the file gives the same
  bytes; definitions at the top.
- **code6cac_b_rodata_post.c** (0x800109B0+0x28), split by referrer and alignment: `g_str_memcard_fmt`, `D_800109BC`
  → main/memcard.c (sole referrers memcard_CountFiles/ReadFile/WriteFile; memcard.o's .rodata slot). `D_800109C8`
  (read only by memcard_Format, 28708.c, whose rodata starts after 28514.o's) → the top of main/28514.c as
  `[] = "bu%1d%1d:"`: its 6 trailing zero bytes are the `.align 3` of func_80037D14's jump table at 0x800109D8 when
  both share one object. Survivor, also oracle-identical: all three in memcard.c with `D_800109C8[16]`, the pad
  spelled into the array, which also needs code6cac.h's `extern s32 D_800109C8` retyped (Phase 2 conflict list).
- **text1a_b_pre_rodata_b.c → main/3AB48.c** (0x800158B4+0x2C): referrers snd_LoadCommonVab and func_8005C2A8
  (3AB48.c); contiguous after func_80058580's tables, 3AB48.o's only other .rodata. Every position after
  func_80058580 gives the same bytes; each string replaces its user's extern.
- **text1a_b_mid_rodata.c** (0 bytes) deleted.
- **code6cac_b_rodata_pre.c (now main/d_1068.c) not folded:** its zero word at 0x80010868 is the `.align 3` pad of
  24F08.c's first table only in one object holding 24BF0.o's and 24F08.o's rodata (0x4C → 0x50), but 24F08.c is
  cc1 -G8 by proof and 24BF0.c is not; a 4-byte const in the -G8 file would sit in .sdata under cc1psx.

## 13. Mixed files split at their PsyQ module starts (restructure step 4, Q106 D3, 2026-10-03)

Object compare `memory/grind/restructure-2026-10-03/step4/splitcmp.py` (the parts' sections concatenated in
link order vs the unsplit object: bytes, every relocation at its shifted offset, every global at the same
offset) plus the oracle link. Cuts sit at LIBSCAN module starts (docs/naming/libscan/matches.json).
- **4a text1b_b.c → main/64FD8.c + 29 `psxsdk/libapi/` + 5 `psxsdk/libc2/` files.** Cuts at 0x80078948 (game |
  LIBAPI C67) and every module start through LIBC2 PRINTF (0x80079208..0x80079244, ending where PRNT starts).
  Rodata: the object's 0x5C bytes are the game functions' jump tables (`.rel.rodata` targets all below
  0x80078948); no library part has `.rodata`, `.data`, `.sdata` or `.sbss`, so no position to choose and no
  survivors. gp: the library parts define no data and reach only extern symbols, which are never gp under
  maspsx `-G8` or `-G0`; their `-G0` (PSYQ_LIBRARY_FILES) form is proven identical by the compare (only the
  5 `j` targets against the `.text` section symbol re-base) and the oracle. Kept in one file: C68 + SENDPAD
  (`libapi/c68.c`), because asm/funcs/FlushCache.s runs from 0x80078FF0 across SENDPAD's start 0x80079000
  to 0x80079028 (the `_SendPAD` mid-function XDEF, docs/naming/libscan/boundary_fixes.md).
- **4b main_post.c → main/d_7D870.c + `psxsdk/libapi/a71.c`, `a72.c`.** Cuts at LIBAPI A71 (0x8008D050), A72
  (0x8008D060) and A72's end 0x8008D070, where the game data begins (g_data_start, g_module_func_tbl, the first
  8 bytes of g_sqrt_table_u8; the rest of that table opens asm/data/7D920.data.s). No part has `.rodata` or
  data sections; no survivors. The one asm block that held DelDrv and the data is cut at 0x8008D070, each
  half keeping the block's `.set` wrapper; DelDrv's symbol size becomes its module's 0x10 (was 0xC0, the
  data included), which no EXE byte records. a71/a72 are `-G0` (PSYQ_LIBRARY_FILES), d_7D870 stays `-G8`;
  neither form can matter (no gp-capable access) and the compare shows identical bytes.
- **4c text1b_b_tu2.c → `psxsdk/libc2/prnt.c` + `ctype.c`, `memchr.c`, `putchar.c`.** Cuts at LIBC2 CTYPE
  (0x800798CC), MEMCHR (0x8007992C) and PUTCHAR (0x8007997C..0x80079A30). Rodata: all 0x214 bytes at
  0x80015A68 are PRNT's (the three named digit/"(null)" arrays, 0x30, then prnt's 121-entry switch table at
  +0x30, phase 0 of an object that still starts at 0x80015A68); CTYPE, MEMCHR and PUTCHAR have none (CTYPE's
  `_ctype_` table is in asm/data .data). No position to choose, no survivors; the table's phase is
  unchanged (R1 does not arise). All four parts were already `-G0`.
- **4d gpu.c + display.c → `psxsdk/libgpu/prim.c` (ex gpu.c), `sys.c` (ex display.c), `ext.c`,
  `psxsdk/libc2/memmove.c` and 10 `psxsdk/libcard/` files.** Cuts at every module start from LIBC2 MEMMOVE
  (0x8007A28C) through LIBGPU SYS (0x8007AE7C). The old gpu|display cut at 0x8007B244 was mid-SYS: SYS's head
  (ResetGraph..GetGraphDebug) moves in front of sys.c, which now holds all of SYS 0x8007AE7C..0x8007DF10 (D3
  re-cut) and, until the next 4d commit, still carries C73/LIBGTE/LIBCD EVENT to 0x8008008C.
  Neither old object had `.rodata`, data or a jump table (the switches in LoadTPage/ResetGraph compile to
  branches), so no position to choose, no survivors, no R1 phase. All parts `-G0` as both files were. No
  `static` in either file; no string literal (every string is a named array in text1a_b_post_rodata.c).
  Compare: old gpu.o+display.o vs the 14 parts (splitcmp.py with a joined BEFORE): `.text` 0x5E00 identical
  bar 88 `j`/`jal` encodings against the `.text` section symbol re-basing, 986 relocations, 174 globals.
  Then sys.c's tail → `psxsdk/libapi/c73.c`, 37 `psxsdk/libgte/` files (GEO_00..PATCHGTE, LIBSCAN names
  lowercased) and `psxsdk/libcd/event.c`, cut at every module start 0x8007DF10..0x8007FF7C; sys.c is then
  exactly SYS. No `.rodata` or data in the range; each module-end pad already sits in its module's own asm
  (§10), so every part ends at its module end. Compare: old gpu.o+display.o vs all 53 parts: identical as
  above (`.text` 0x5E00, 986 relocations, 174 globals).
  Rodata (D4): text1a_b_post_rodata.c's LIBGPU and LIBCD EVENT strings move to their sole readers, in link
  order: 0x80015D58..0x80015E28 → prim.c (DumpTPage/DumpClut/DumpDrawEnv/DumpDispEnv), 0x80015E28..0x8001605C
  → sys.c (first the rcsid "$Id: sys.c,v 1.129 …", which SYS's device table D_8009BE2C points at by address,
  then the SYS functions' strings), 0x8001605C..0x80016074 → event.c (CdInit). bb2.ld .rodata: prim, sys,
  event, then the data file, which keeps the LIBCD SYS/BIOS strings 0x80016074..0x8001622C for step 4e. Each
  receiving object starts where its first string sits; no jump table, so no phase. The one array spanning a
  module boundary, D_80015E1C[64], is cut into D_80015E1C[12] (PRIM) + D_80015E28[52] (SYS), same bytes.
  Survivor: the rcsid could also end PRIM's object (identical bytes); it goes to SYS on its text, the device
  table pointer and SOTN's libgpu/sys.c rcsid; every other array has one position. Compare: old post_rodata.o+gpu.o+display.o vs the 54 parts: `.text` 0x5E00 and
  `.rodata` 0x4D4 identical, 986 relocations identical, 217 globals at the same offsets; the only symbol
  differences are that cut (D_80015E1C's size 64 → 12, new D_80015E28).
