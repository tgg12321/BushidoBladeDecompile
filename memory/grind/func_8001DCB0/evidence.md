# func_8001DCB0 — evidence (manual session 2026-09-25)

Result: COMPLETED-C candidate. `sandbox --disable all --candidate` = 0 (469/469,
0 source-level, 0 operand-only, 57 not-scored branch-displacement hunks), and
the full build with the body spliced == oracle 62efab4f.

## Path
- First honest draft (per-use byte-offset puns on the record fields): 59.
- Record fields through a real array symbol (`g_practice_menu_table[i].unk_*`):
  469/469. GCC then addresses each field as `lui at,%hi(sym+off); addu at,at,s1`
  with one byte-offset IV, which is the target's shape. A cast base
  (`((Rec *)&D_80101EC8)[i]`) strength-reduced into per-field pointer IVs
  (+3 callee-saved regs): 31.
- The tbl join: the target's two arms each compute `row*6`/`row*8` and cross-jump
  into one `addu v0,v0,a0; addu v0,v0,v1; lbu`. This is exactly what two 2-D
  array reads `D_8008E6A4[r][c]` / `D_8008E5CC[r][c]` in the if/else arms
  produce. Single pointer+offset spellings put tbl/col in the wrong regs.
- `addr = 0x80190800` between func_8004939C() and func_80020D38(): the target's
  lui sits in 939C's delay slot and the ori in 20D38's, so the constant is
  materialised between the two calls.

## Declarations (prep commit 7f74ea6a5, byte-neutral, oracle-verified)
PracticeMenuRec +0xA/+0xE/+0x14/+0x5E fields. D_8008D538/D578/D9EC/EB80 are
flat arrays; D_8008E5CC[][8] and D_8008E6A4[][6] are 2-D. The Match commit adds
D_800A3100[][4] and D_800A3904 (u16) to code6cac.h.

## Lesson bytes 0x8010277C..0x80102781: one single-base struct (measured)
Final form: `typedef struct { u8 unk_0[2]; u8 unk_2[2]; u8 unk_4[2]; }
PlayerBytePairs; extern PlayerBytePairs D_8010277C;` in code6cac.h. Every
consumer is re-spelled, and D_8010277D..81 are retired from C. The full build
== oracle.

- THREE SEPARATE arrays (`s8 D_8010277C[2]`, `D_8010277E[2]`, `D_80102780[2]`)
  are NOT byte-neutral. func_80033BC0 grows by 6 insns: its
  `[1] = y; if (...) [1] = 5;` CSEs the `D_8010277E+1` address into $a1.
- A SINGLE-BASE aggregate is byte-neutral. The layer-2 reviewer measured
  `u8 [6]`, `u8 [3][2]` and a struct of three `[2]` in isolation, all
  identical to the scalar form, and the full oracle build confirms the struct.
- The shipped EXE has no relocations. Nothing observable distinguishes
  `%lo(D_8010277F)` from `%lo(D_8010277C+3)`, so the first attempt's claim
  that "0x8010277F was its own symbol" was an unsupported inference, withdrawn.
  The rejected pun body is in rejected/lesson-scalar-pun-0.c.
- u8 elements, not s8: u8 keeps the pointer locals at code6cac.c:1887 and
  code6cac_c_ab.c:416/429/457/512 cast-free. Signed reads use (s8) value
  casts, which the tree already uses for these bytes, and the reads of the
  formerly TU-local `extern s8` 7D/7F/81 got the same cast to keep lb.

## Call-result locals
A function-scope `s32 v;` written 7 times (once per func_80021A98 site)
compiles byte-identically to block-local `s32 v` per site (layer-2 round 3
measured cc1 output identical, tmp/l2rev_dcb0/). The multi-write form has no
admission ruling, so the block-local form lands (Ruling 1(4)).

## Not tried / not needed
No permuter, no FAKE constructs, no do-while wraps.

## Retro-audit correction (owner Q49), 2026-09-30 (laneG) — independent base+offset evidence
The retro-audit (2026-09-29 batch_04, 40adc7d6c) found the one-object argument above under-evidenced on
aggregate-merge prong (a): the func_8003B5A4 walk covers only 0x7D/0x7F, and "three separate arrays are not
byte-neutral" is compiler-necessity reasoning, admitted only later (Q2, 262db111c) and then with dumps and cc1psx.
Independent evidence that 0x8010277C.. is ONE object exists in the original binary and was not cited:
asm/funcs/func_80034708.s:31-33 forms `$s5 = &D_8010277C` (`lui/addiu %hi/%lo(D_8010277C)`) and reads
`lb 0x0($s5)`; :141 reads `lb 0xA($s5)` (0x80102786) and :152 `lb 0xB($s5)` (0x80102787) through the same base
register — base+offset addressing across 0x7C..0x87 (prong (a), "base+offset addressing in the original binary").
Current object model: the PlayerBytePairs declaration landed here was superseded by `PracticeParams D_80102778`
(include/code6cac.h:724-740; unk_4[6] covers 0x7C..0x81), whose own layout evidence is
memory/grind/func_80034708/evidence.md [s4]-[s5]. func_8001DCB0's body is unchanged; no code change needed.
