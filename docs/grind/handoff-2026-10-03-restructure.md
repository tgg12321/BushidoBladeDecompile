# Hand-off: padding retirement + Phase 1 restructure (2026-10-03)

For a fresh agent on `main`. Work it top to bottom. Each step is a small commit that is byte-identical
(`verify-oracle --rebuild` SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`), passes `engine test`, and gets
a fresh adversarial `cheat-reviewer` PASS where a function body, rule or gate file changes.

## 0. Read first

- **Owner direction (2026-10-03, docs/grind/decisions.md "OWNER DIRECTION"):** the project is fully
  matched (queue empty). The owner is retiring special rules and moving the code toward pure C, a faithful
  representation of the original source, and **human readability**. When a choice comes up, take the
  option that removes exceptions, aliases, asm and FAKEs and makes the code clearer. New exceptions come
  only by owner ruling.
- **Standing workflow** (CLAUDE.md, AGENTS.md): engine via `& tools/wteng.ps1 main <cmd>` from PowerShell;
  scripts beyond one command go under `tmp/`; build files are LF; never `make setup`; `bb2.ld` is
  hand-maintained. **The Grinder stays stopped** (restarting it needs the owner). Take the reintegration
  lock (`& tools/reintegrate_lock.ps1`) before mutating main. A ruling is committed (`rules:`) before the
  code that uses it. Commit messages follow docs/COMMIT_CONVENTIONS.md; history goes in commit bodies, not
  new .md files.
- **Ask the owner in plain language** (one question; what happens on yes and on no; no assembly jargon).
- **State at hand-off:** the 2026-10-03 session landed rules f0abd598b (Q96-Q103), the Q96-Q100 cheat
  cleanups (0a35f87af, 45e5c6982, 53f84da6b, 24f5f0834, 8e3d4bd99), and naming Q103 (4ac3546c6 functions,
  5a471b1dd data/members/typedef: PracticeMenuRec is now Unk80101EC8Record, g_practice_menu_table is D_80101EC8). Q104-Q106 below are recorded in the `rules:` commit that adds this
  file.
- **Evidence and scripts** are banked in `memory/grind/restructure-2026-10-03/` (`pad-survey/`,
  `restructure-survey/`). The scripts write into `tmp/pad-survey/` and `tmp/restructure-survey/`. Copy them
  there and re-run them against the current tree before relying on their outputs. Paths below written as
  `tmp/...` refer to those regenerated outputs.

## 1. Owner decisions in force

- **Q101/Q102** (rodata-object-alignment.md § Inter-object padding): `PAD_NOPS_n` and file-scope rodata
  `.word 0` are layout debt to retire byte-neutrally. A pad at a Sony module end belongs in that module's
  own asm.
- **Q104:** ReadGeomScreen, SetGeomOffset and SetGeomScreen (LIBGTE REG09/REG12/REG13) become whole-body
  canonical asm with their module's pad nops. This is what Sony shipped: only hand-written asm modules are
  padded to 16. It retires the last PAD_NOPS uses (Part A step 6).
- **Q105:** typed-restatement's T5 closed relation-noun list is replaced by a principle: a generic, non-game
  noun whose meaning holds in every write and read of the matched code. Game nouns still need an accepted
  class. Follow-up (Part B): re-check `kind` (Obj80106A78), `type[2]` / `valid[2]` (PadState) and
  pad_ResetStateMarkValid (0x80019534) under it. `age` and `.pad` stay refused on their facts.
- **Q106: Phase 1 restructure decisions** (Part C § 6, as resolved):

  | # | Decision |
  |---|---|
  | D1 | SOTN layout: `src/main/psxsdk/<lib>/<module>.c` for Sony code, `src/main/<romoff>.c` for game code (file names per D6); Sony prototypes in `include/psxsdk/lib*.h` |
  | D2 | One file per verbatim Sony module (about 185 files incl. BIOS stubs and gap files) |
  | D3 | The LIBSCAN verbatim module span is TU-boundary evidence for library code, including re-cuts of today's mid-module cuts (gpu\|display, system\|ings2). Unidentified gap regions stay one file per gap |
  | D4 | A data-only rodata file folds into its proven owning TU (sole referrer + contiguity + link order) |
  | D5 | Game boundaries with no evidence either way stay. Only evidence moves a boundary |
  | D6 | **Game file names:** a subsystem name where the file's functions already carry accepted names (e.g. memcard, cdrom, comb/link cable); otherwise the ROM-offset name. Names are claims: state the supporting function names in the commit body |
  | D7 | Headers: Sony headers plus identical multi-file game declarations. Local externs are allowed (SOTN norm). Conflicting declarations (108 symbols) are deferred to Phase 2 |
  | D8 | `bb2.ld` stays hand-maintained, plus a consistency checker (every TU linked; one object order across sections) |
  | D9 | Historical `src/<stem>.c` citations in docs and ledgers stay as written, resolved by a tag plus `tools/tu_renames.tsv`. Update current-state docs only |
  | D10 | Delete (the survey listed it as optional; the orchestrator recommended yes to the owner; scheduled in Part C step 6) the unlinked splat segment dumps `asm/{6CAC,text1a,text1b,text2,text3,text4}.s` (check every tool that edits them first) |

## 2. Work order

- **A. Padding retirement** (Part A below, steps 0-6). It must land before restructure steps 4a/4d, because
  the pads sit on module boundaries.
- **B. Q105 re-check** of the four refused naming rows (naming lane; `tools/naming_wave.py` for functions,
  data_wave / scripted C edits for members; layer-2 on changed bodies).
  **Status: done** (37d769472 type[2]/valid[2]/kind; 0c813f2d7 0x80019534 refused on T4).
- **C. Phase 1 restructure** (Part C below). Tag `pre-restructure-<date>` first, then freeze the tree for
  the moves.
- **Later:** Phase 2 types (make Unk80101EC8Record / Obj80106A78 the "Entity"; ~1,238 raw-offset casts;
  fix the 108 conflicting declarations and the u8 * / s32 * callee prototypes). Phase 3 names (owner to
  decide whether game-code naming moves from evidence classes to SOTN-style "explainable from the code +
  adversarial review").

## 3. Open debt carried (not in A-C)

- **Owner question (pending):** should typed-restatement's T4 verb list become a principle like Q105's
  nouns? It blocks only 0x80019534 → pad_ResetStateMarkValid ("Mark" is not a listed verb; refused 0c813f2d7).
- func_80031B24 hands `&D_800A37E8` to func_800274BC / func_80032854 unlabelled (outside Q96).
- Q97's struct form is unmeasured; Q96-Q99 retire as soon as a one-object spelling matches.
- Hygiene rows in the 2026-10-03 commit bodies (`git log --grep="Hygiene debt" --since=2026-10-03`).

---

# Part A: PAD_NOPS / rodata padding retirement (survey 2026-10-03)

**Status: LANDED 2026-10-03** (no PAD_NOPS or rodata `.word 0` pad left in src/; per-site record in
docs/grind/rodata-align-2026-09-30.md §§ 10-11). Step 0 e5d16f841; 1 844536803; 2 d384bd32c (also defines
D_800790B4 in func_800790A4.s); 3 5558a0dac; 4 54fdd38ea; 5 8ba809bed + rules 4907fca49; 6 d6f8170cf (auth);
adjacent: `.set` blocks 191f39673, D_8007E08C `nonmatching` marker e38643786; oracle re-lock 7398e90f1.
Not done: folding D_8007E08C into InitGeom.s (tools read a function's address from the first address column
of its .s; see e38643786). Open: build_oracle_cc1.sh's narrow-patch self-check expectation is stale
(code6cac_tu2 differs; 8ba809bed); engine test "a queued item with no record to drop" fails while the queue is empty.

Read-only survey. All experiments are in `tmp/pad-survey/` (objects built with `cc.sh`, a copy of the
Makefile C pipeline, and linked with `link.sh` into `tmp/` only; `build/` untouched). Baseline relink of
the existing `build/` objects = oracle `62efab4f…` (control `base2`, `ctllink`).

### 1. Findings

**Inventory.** 34 `PAD_NOPS_n;` uses = 33 logical sites (text1b_b.c:2998/2999 are one 6-nop run), 61 nops:
display.c 25, text1b_tu1b.c 5, text1b_b.c 4. Ten files `#define` the macros. Seven of them never use them:
code6cac.c, code6cac_b.c, code6cac_b2_post.c, code6cac_c.c, code6cac_c_mid.c, code6cac_c_ab.c,
code6cac_c2.c. Every preceding function is COMPLETED-INLINE-ASM-CANONICAL (all 33 are in
inline_asm_canonical.txt). None is in the queue.

**The nops are module bytes, and the link adds no alignment.** `docs/naming/libscan/matches.json` places
each Sony module bit-for-bit, and the verified word counts include the pads. Example: LIBGTE MSC00 is 36 words
= D_8007E08C (2) + InitGeom (32) + 2 nops. Every module-evidenced gap ends exactly at its module's end (table,
"ends at gap end"). Census over all 192 placed modules: the 77 asm-supplied modules (census.py / census.tsv) all have sizes that are
multiples of 16. Sony C modules don't: 64 of 105 are not multiples of 16 (GEO_00 0xCC, MEMCPY 0x34, COUNTER
0x178, …). No module we hold as C is followed by a gap, except REG09/REG12/REG13. Those C bodies are ours;
their modules carry the asm signature (sites #12-14). So Sony's assembler padded each asm module's `.text` to 16
bytes, and PSYLINK concatenated modules at 4-byte granularity.

**What the 0xC phase is relative to.** Nothing. MSC00 starts at 0x8007E08C because the C modules before it are
unpadded: LIBAPI C73 GPU_cw @0x8007DF10 (0x10), then GEO_00 rsin/sin_1 (0xCC), then GEO_01 rcos (0xA0). Every
later LIBGTE asm module is a multiple of 16 in size, so the 0xC phase carries through to CdInit (LIBCD EVENT
@0x8007FF7C). The phase isn't 0, which rules out absolute linker alignment. RATAN (ratan2, C) is 0x180, a
multiple of 16 by coincidence. The same holds for LIBAPI: A39 (SetSp) at phase 8 is followed by `open` at
phase 8.

**text1b_tu1b sites have no module evidence.** These are game code with no libscan placement. Next-function
phases are 4, 4, 0, 8, 0. A 16-byte module model is refuted: 0x80052C28..0x80052D00 is 0xD8 and has no phase-0
function start. An 8-byte model is refuted by the 12-byte and 8-byte gaps. So these follow rule item 2.

**Existing in-tree convention for module padding: inside the module's own asm, before `endlabel`.**
- `include/bios.h` BIOS_FUNCTION: "followed by one nop of padding", inside glabel/endlabel.
- `ings2.c:322-344` `_96_remove`: LIBAPI C114's two leading module words and its 3 pad nops sit in one canonical
  block.
- `asm/funcs/setjmp.s` (2 nops), `asm/funcs/SetGeomScreen.s` (1 nop, unused because the C body is linked),
  `asm/funcs/FlushCache.s` (C68's pad nop).

**Engine/tooling impact.** There is no PAD_NOPS special case other than the generic macro-asm stripper
(`engine/volatile_cheats.py:1386 find_macro_asm_defs`) and its synthetic test (`engine/test_engine.py:1355`).
Both stay valid; no engine change is needed. Other impacts:
- `engine/layer2.py` keys an asm-supplied function by its block text plus the included `.s`, so every edited
  `.s` or block moves that function's layer-2 key. `queue done` and the departures audit judge only the body
  that left the queue, so nothing blocks. Each step still needs a fresh layer-2 (cheat-cleanup class).
- `engine/completion.py:source_issues` returns `[]` for a whole-body canonical asm function, so stale
  `tools/canonical_asm_regions.json` rows are harmless (delete them anyway in step 6).
- Symbol `.size` grows by the pad, because it goes inside endlabel. `score._o_func_table` and the integrity
  size buckets just follow the new size; only canonical functions are affected.
- No `size:` annotations exist in the symbol files for these names.

### 2. Site table (PAD_NOPS)

Addresses are from `build/bb2.elf` (`nm -n -S`). Modules are from `docs/naming/libscan/libsyms.json`.

| # | site | n | preceding | addr..end | next @ | mod16/mod8 | PsyQ module (span) | body | representation |
|---|---|---|---|---|---|---|---|---|---|
| 1 | display.c:1116 | 2 | InitGeom | 8007E094..8007E114 | 8007E11C | C/4 | LIBGTE MSC00 8007E08C..8007E11C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 2 | display.c:1118 | 3 | SquareRoot0 | 8007E11C..8007E1A0 | 8007E1AC | C/4 | LIBGTE MSC01 8007E11C..8007E1AC (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 3 | display.c:1159 | 1 | LoadAverageCol | 8007E3BC..8007E438 | 8007E43C | C/4 | LIBGTE MSC06 8007E1AC..8007E43C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 4 | display.c:1161 | 3 | SquareRoot12 | 8007E43C..8007E4D0 | 8007E4DC | C/4 | LIBGTE MSC09 8007E43C..8007E4DC (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 5 | display.c:1170 | 1 | MulMatrix0 | 8007E4DC..8007E5E8 | 8007E5EC | C/4 | LIBGTE MTX_000 8007E4DC..8007E5EC (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 6 | display.c:1196 | 3 | ScaleMatrixL | 8007E8DC..8007EA00 | 8007EA0C | C/4 | LIBGTE MTX_00A 8007E8DC..8007EA0C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 7 | display.c:1204 | 2 | ApplyRotMatrixLV | 8007EA0C..8007EB44 | 8007EB4C | C/4 | LIBGTE MTX_01 8007EA0C..8007EB4C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 8 | display.c:1211 | 1 | MulMatrix | 8007EB4C..8007EC58 | 8007EC5C | C/4 | LIBGTE MTX_03 8007EB4C..8007EC5C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 9 | display.c:1221 | 1 | MulMatrix2 | 8007EC5C..8007ED68 | 8007ED6C | C/4 | LIBGTE MTX_04 8007EC5C..8007ED6C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 10 | display.c:1244 | 3 | ScaleMatrix | 8007EDBC..8007EEE0 | 8007EEEC | C/4 | LIBGTE MTX_08 8007EDBC..8007EEEC (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 11 | display.c:1249 | 3 | ReadSZfifo3 | 8007EF6C..8007EF80 | 8007EF8C | C/4 | LIBGTE REG04 8007EF6C..8007EF8C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 12 | display.c:1251 | 1 | ReadGeomScreen | 8007EF8C..8007EF98 | 8007EF9C | C/4 | LIBGTE REG09 8007EF8C..8007EF9C (ends at gap end) | C + GTE island (canonical) | Q104: whole-body .s (orig. asm module) incl. pad |
| 13 | display.c:1274 | 2 | SetGeomOffset | 8007EFDC..8007EFF4 | 8007EFFC | C/4 | LIBGTE REG12 8007EFDC..8007EFFC (ends at gap end) | C + GTE island (canonical) | Q104: whole-body .s (orig. asm module) incl. pad |
| 14 | display.c:1278 | 1 | SetGeomScreen | 8007EFFC..8007F008 | 8007F00C | C/4 | LIBGTE REG13 8007EFFC..8007F00C (ends at gap end) | C + GTE island (canonical) | Q104: whole-body .s (orig. asm module) incl. pad |
| 15 | display.c:1315 | 1 | Lzc | 8007F200..8007F218 | 8007F21C | C/4 | LIBGTE SMP_00 8007F00C..8007F21C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 16 | display.c:1318 | 1 | RotTransPers | 8007F21C..8007F248 | 8007F24C | C/4 | LIBGTE SMP_02 8007F21C..8007F24C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 17 | display.c:1325 | 3 | RotTransPers3 | 8007F24C..8007F2A0 | 8007F2AC | C/4 | LIBGTE SMP_03 8007F24C..8007F2AC (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 18 | display.c:1328 | 2 | RotTrans | 8007F2AC..8007F2D4 | 8007F2DC | C/4 | LIBGTE SMP_04 8007F2AC..8007F2DC (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 19 | display.c:1334 | 2 | RotTransPers4 | 8007F2DC..8007F354 | 8007F35C | C/4 | LIBGTE CMB_00 8007F2DC..8007F35C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 20 | display.c:1342 | 1 | RotMatrix | 8007F35C..8007F5E8 | 8007F5EC | C/4 | LIBGTE FGO_01 8007F35C..8007F5EC (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 21 | display.c:1353 | 1 | RotMatrixZYX | 8007F5EC..8007F878 | 8007F87C | C/4 | LIBGTE FGO_03 8007F5EC..8007F87C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 22 | display.c:1364 | 2 | RotMatrixX | 8007F87C..8007FA14 | 8007FA1C | C/4 | LIBGTE FGO_04 8007F87C..8007FA1C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 23 | display.c:1372 | 2 | RotMatrixY | 8007FA1C..8007FBB4 | 8007FBBC | C/4 | LIBGTE FGO_05 8007FA1C..8007FBBC (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 24 | display.c:1381 | 2 | RotMatrixZ | 8007FBBC..8007FD54 | 8007FD5C | C/4 | LIBGTE FGO_06 8007FBBC..8007FD5C (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 25 | display.c:1524 | 1 | _patch_gte | 8007FEDC..8007FF78 | 8007FF7C | C/4 | LIBGTE PATCHGTE 8007FEDC..8007FF7C (ends at gap end) | file-scope glabel block (canonical) | nop into the block |
| 26 | text1b_tu1b.c:74 | 1 | math_MidpointS16x3U8x2 | 8004C388..8004C400 | 8004C404 | 4/4 | none (game code) | INCLUDE_ASM (canonical) | item 2: move nops to end of its .s |
| 27 | text1b_tu1b.c:86 | 1 | func_8004DDB4 | 8004DDB4..8004E560 | 8004E564 | 4/4 | none (game code) | INCLUDE_ASM (canonical) | item 2: move nops to end of its .s |
| 28 | text1b_tu1b.c:135 | 2 | math_SquareRoot0 | 800526A0..80052718 | 80052720 | 0/0 | none (game code) | INCLUDE_ASM (canonical) | item 2: move nops to end of its .s |
| 29 | text1b_tu1b.c:182 | 1 | func_80052C10 | 80052C10..80052C24 | 80052C28 | 8/0 | none (game code) | INCLUDE_ASM (canonical) | item 2: move nops to end of its .s |
| 30 | text1b_tu1b.c:186 | 3 | gte_ReadIR1IR2Sra2 | 80052CD4..80052CF4 | 80052D00 | 0/0 | none (game code) | INCLUDE_ASM (canonical) | item 2: append nops (its .s has no endlabel) |
| 31 | text1b_b.c:2786 | 1 | SetSp | 800789D8..800789E4 | 800789E8 | 8/0 | LIBAPI A39 800789D8..800789E8 (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |
| 32 | text1b_b.c:2998+2999 | 6 | func_800790A4 | 800790A4..800790A8 | 800790C0 | 0/0 | LIBAPI SENDPAD 80079000..800790C0 (ends at gap end) | INCLUDE_ASM (canonical) | 3 nops = rest of the 4-word stub, 3 = module pad (see note) |
| 33 | text1b_b.c:3001 | 1 | _remove_ChgclrPAD | 800790C0..8007911C | 80079120 | 0/0 | LIBAPI CHCLRPAD 800790C0..80079120 (ends at gap end) | INCLUDE_ASM (canonical) | move nops to end of its .s |

Note on #32. `_send_pad` (asm/funcs/_send_pad.s:10-25) copies the words from func_800790A4 up to D_800790B4,
so the patch stub is 4 words: `and $v0,$v0,$s5` plus 3 zero words. 0x800790B4..C0 is SENDPAD's 16-byte pad.
D_800790B4 is today an absolute in undefined_syms_auto.txt:17. A faithful `.s` puts the stub's 3 zero words
inside func_800790A4, then `dlabel D_800790B4`, then 3 pad nops (and removes the absolute). Byte-identical
either way.

### 3. Representation chosen, and alternatives rejected

**Module-end sites (#1-11, #15-25, #31-33).** The pad belongs to the module's `.text` (libscan word counts), so
it goes at the end of the module's last asm, inside endlabel, which is the in-tree convention. A TU split by
itself can't reproduce the gap:
- GNU ld with `SUBALIGN(2)` adds nothing.
- `ALIGN(16)` is absolute and gives phase 0, not 0xC.
- A per-module object with a trailing `.balign 16` works only for asm objects. It would need display.c split
  into about 40 objects for no byte or fidelity gain over the literal nops.

So the faithful split *is* the nops at the module's own end. Record each site (module, span, pad words) in
docs/grind/rodata-align-2026-09-30.md, as rule item 1 asks.

Rule wording: done. Rule item 1 now reads "the nops go into that module's own asm (its last asm/funcs/*.s)".

**Game-code sites (#26-30).** Rule item 2. These are mechanically identical to the module-end sites.

**C-bodied LIBGTE modules (#12-14, REG09/REG12/REG13).** The pad can't live in C. Options:
- (A) Whole-body `.s`, matching the original hand-written module. Evidence: the 16-byte padding is the
  asm-module signature, and no Sony C module in BB2 shows it. **Byte-proven** (`tmp/pad-survey/var`). It is a
  reclassification from "C + islands" to canonical-body. ReadGeomScreen and SetGeomScreen are pure GTE leaf
  wrappers (inline-asm-policy auto-authorize class). SetGeomOffset computes (`sll` ×2), so it needs an
  evidence or owner route. **Adopted: owner ruling Q104.**
- (B) One TU per module, assembled without `-no-pad-sections`. Gas pads `.text` to 16; tested on REG09:
  `.text` = 0x10, `cfc2; jr; nop; nop`. It needs a per-object Makefile list (a new gate list) and about 5 more
  display.c splits. It also misdescribes the code: Sony's C modules were never padded. Rejected.
- (C) Linker relative pad (`. = start + ((. - start + 15) & ~15)` per site). Hardcoded per-site linker layout.
  Rejected.

### 4. Rodata pads

**R1, code6cac_c_ab.c:332-334 mid-file `.word 0`.** Redundant under the 2026-09-30 object-relative model. It
was added in f2d6c1995 under the retired per-file `.align 2` sed. Deleting it gives an object identical in
`.text`, `.rodata` (0xCC), relocations and symbols (`c_ab_orig.o` vs `c_ab_nopad.o`). Table C's `.align 3`
regenerates the zero word at +0x74. Pure delete.

**R2, src/code6cac_c_ab_pad.c (4 bytes @0x80010D70) and bb2.ld:59.** c2's first rodata item is a jump table at
0x80010D74 (phase 4). c_ab's object starts at 0x80010CA4 (phase 4). The 4 zero bytes are exactly the `.align 3`
pad that one object holding both rodata runs emits. The split form needs a phantom 4-byte object. The c_ab|c2
boundary dates from regfix-era CU splitting (9cb130a8e, "split … so each CU emits one jtbl with asm rodata
interposed").

Byte proof: `merged.c` (c_ab without R1's word, then c2) linked in place of c_ab.o + c_ab_pad.o + c2.o gives the
oracle (`mergedlink.exe`).

Survivors per "New TU boundaries" condition 2 (all oracle-identical):
- full merge, cut at func_8003AB44 (today's c_mid|c_ab cut);
- cut before func_8003ACB8, with c_ab's Q65 tentative commons in both parts;
- cut before func_8003AE5C, with commons in the first part or both.

Cuts with the commons only in the second part fail. That is gp-model evidence: AB44 and ACB8 reach those
commons gp-relative. Positions inside c_mid were not tested. c_mid's `.sdata` and c2's are separated by
asm/data/93A0E in bb2.ld:186-188, so the gp model already keeps c_mid distinct.

Recommended: full merge (removes a boundary, adds none). State the other survivors in the record. Two
conflicting declarations must be reconciled:
- `D_800A3894`: `u8 *` in c_ab, c_mid and tu2 vs `s32` in c2:53.
- `func_80020D38`: `s32` in c_ab:63 vs `void` in c2:160 and tu2:1797.

The experiment kept c_ab's declarations; check against the definition or header (item 4).

### 5. Ordered implementation (each step independently `verify-oracle --rebuild` + `engine test`)

0. Delete the 7 unused `#define PAD_NOPS_n` triples (files listed in §1). Zero bytes, no function keys move.
1. R1: delete code6cac_c_ab.c:332-334. Object-identical (proven).
2. text1b_b.c #31-33 (LIBAPI A39/SENDPAD/CHCLRPAD): pads into SetSp.s, func_800790A4.s (+ optional
   D_800790B4 label, dropping its absolute) and _remove_ChgclrPAD.s; remove the 4 uses and the defines.
   Layer-2 on 3 functions.
3. display.c #1-11, #15-25: pads into 21 `.s` files, plus one `nop` before `.set reorder` in the _patch_gte
   block. Add the per-site module table to docs/grind/rodata-align-2026-09-30.md. Keep the display.c defines
   (3 uses remain). Layer-2 on 22 functions.
4. text1b_tu1b.c #26-30: pads into 5 `.s` files (gte_ReadIR1IR2Sra2.s: append after `jr; nop`); remove the
   defines. Layer-2 on 5.
5. R2: merge code6cac_c_ab.c into code6cac_c2.c (moves only plus the 2 declaration reconciliations). Delete
   src/code6cac_c_ab_pad.c and the c_ab/c_ab_pad lines in bb2.ld (58-59, 106, 147, 238). Then:
   - Refresh tools/cc1_tu_expectation.txt (`bash tools/build_oracle_cc1.sh --record-manifest`) and
     oracle/manifest.json via the usual flow.
   - Fix stale references: tools/audit_rodata_blocks.py:49 comment, tools/classify_inline_asm.py:179 docstring,
     tools/kengo_match.py:74, the gp-model doc table rows, and the rule's `paths:` glob `src/code6cac_c_ab*.c`.
   - Record existence, survivors and the chosen cut in the rodata-align doc.
6. Under owner ruling Q104: REG09/REG12/REG13 C bodies become INCLUDE_ASM (SetGeomScreen.s already
   has its nop; +1 ReadGeomScreen.s, +2 SetGeomOffset.s). Update those inline_asm_canonical rows (mixed to
   canonical-body), drop their canonical_asm_regions.json rows, then delete display.c's defines. `auth:` +
   layer-2.

Steps 0-4 and 6 together were byte-proven in one shot: `tmp/pad-survey/var/{display,text1b_tu1b,text1b_b}.c`
(generated by `gen.py`) plus `tmp/pad-survey/asm/*.s`. Every section of all three objects is identical to the
control, and the link equals the oracle (`varlink.exe`).

### 6. Open risks / adjacent debt

- Layer-2 keys move for about 30 completed canonical functions (asm text changes). These are cheat-cleanup
  commits with a fresh adversarial review each. Brief the reviewer with the libscan spans and the byte proofs
  above. Every function's own instruction bytes are unchanged.
- At survey time the tree carried other lanes' edits; they have all landed since (0a35f87af..5a471b1dd).
  Rebase each step on a clean tree; `gen.py` is reusable but was run against the tree as of 03:00.
- Symbol sizes of 29 functions grow by their pads. If any tool keys on `.size` (none found besides
  `score._o_func_table` and the integrity size buckets), its numbers shift.
- Same-class layout debt (not among the 34):
  - display.c:1105-1113 is a file-scope `__asm__ .include "asm/funcs/D_8007E08C.s"`, a `nonmatching` dlabel.
    Those 2 words are MSC00's leading module words, like C114's in ings2.c:327-333. Fold them into the module
    asm (InitGeom.s before glabel) and drop the NON_MATCHING marker.
  - display.c:1129-1139, 1142-1152 and 1296-1306 are zero-byte `.set`-only `__asm__` blocks; delete them.
- Symbol hygiene found along the way: FlushCache.s spans LIBAPI C68 (0x10) plus the first 0x28 bytes of SENDPAD
  (`_SendPAD` XDEF @0x80079000), and func_800790A4's size is 4 but its stub is 16 bytes.

---

# Part C: Phase 1 restructure, layout only (survey 2026-10-03)

Read-only survey. Nothing tracked was edited and nothing was built into `build/`. The scratch inputs
are all in `tmp/restructure-survey/`:
- `mapsec.py` / `mapsec.txt`: per-object section placement from `build/bb2.map`, plus where libscan
  modules land per object.
- `nmall.sh` → `objfuncs.txt`, `elf_nm.txt`: symbols per object and per address.
- `summ.py` / `summ.txt`: per-file function spans.
- `subsys.py` / `subsys.txt`: subsystem evidence (name prefixes, Sony calls).
- `externs.py` / `externs.txt` / `decls.json`: the declaration census.

The SOTN reference is the clone `sotn-decomp@db41b28`.

### 0. Key facts that shape the plan

- **Renaming or moving a .c file is byte-neutral by construction.** No `__FILE__` or `.incbin` is
  used. Every `#include` resolves through `-Iinclude`, and there are no `src/*.h`. `INCLUDE_ASM` paths
  are relative to the working directory. `.main` is extracted with `objcopy -O binary -j .main`, so
  `STT_FILE` and `.file` differences can't reach the EXE. What does matter: the `bb2.ld` object order
  in every section, and the per-file flag membership. Those are `GP_FILES` (cc1 -G8),
  `PSYQ_LIBRARY_FILES` (maspsx without -G8) and `EXPAND_LB_FILES`.
- **Splits, merges and re-cuts are not neutral by construction.** Within one TU the following can
  change bytes:
  - identical string literals are shared;
  - a `static` function or object loses or gains its linkage;
  - implicit vs prototyped calls;
  - jump-table `.align 3` is object-relative, so a new object start changes table padding;
  - under -G8, cc1 emits all data before all functions (TARGET_FILE_SWITCHING).
  Each one needs its own oracle proof, and for game code an evidence class under
  `rodata-object-alignment.md` / `per-file-gp-model.md`.
- **No game C object has `.data` or `.bss` input.** All initialized data is in `asm/data/7D920.data.s`
  (0x14374 bytes), `91C98.data.s` and the 17 small `93xxx` sdata/sbss fillers. BSS is COMMON or
  absolute symbols. Only `system.o` has `.data` (CD_intr, 3 bytes). So layout work is about
  .rodata, .text, .sdata and .sbss. Moving data into C is a later phase.
- **The current file names are actively misleading:**

  | File | Actually holds | Note |
  |---|---|---|
  | `main.c` | LIBSND/LIBSPU | The game's `main()` is in `ings.c`. |
  | `system.c` | LIBCD SYS/BIOS/CDREAD | |
  | `display.c` | LIBGPU SYS tail + LIBGTE + LIBCD EVENT | |
  | `gpu.c` | LIBC memmove + LIBCARD + LIBGPU EXT/PRIM + SYS head | |
  | `ings2.c` | LIBCD/LIBETC/LIBAPI + SN runtime + LIBSND | |

  `code6cac_*`, `text1a_*` and `text1b_*` are splat segment names plus split suffixes.
- **Layer-2 records survive moves.** They are keyed by addr + definition-body hash, not by file
  (`engine/layer2.py:10-50`). They break only where a definition's tokens change, e.g. a `static`
  removed during a split. `queue.json` has 0 items.

### 1. Inventory (49 files, 58,080 lines; addresses from build/bb2.map at HEAD)

The Flags column means: GP = in GP_FILES (cc1 -G8), LIB = in PSYQ_LIBRARY_FILES (maspsx -G0),
LB = in EXPAND_LB_FILES. Every other file is cc1 -G0 with maspsx -G8.

Boundary evidence classes used throughout:
- **PHASE**: a jump-table phase change, proven. Sites 1-7 in docs/grind/rodata-align-2026-09-30.md
  §4/§7, plus the §9 boundary.
- **GP**: a per-file gp split or merge (gp-model doc §4, Q65/Q67).
- **G8**: a cc1 -G8-by-proof boundary (compiler-flags-canonical; Q89/Q94).
- **LBRUN**: an edge of the EXPAND_LB run, where the maspsx flag differs per file.
- **LIBSCAN**: a verbatim PsyQ module span (memory/closer/psyq-library-census.md).
- **LEGACY**: a splat segment edge or a tool-era "CU split", with no evidence either way.

#### 1a. Game code (.text 0x800164F8..0x80078948 plus a 0xB0 data tail at 0x8008D070)

| # | File | Lines | .text (size) | Other sections | Contents / subsystem evidence | Start boundary evidence | Flags |
|---|---|---|---|---|---|---|---|
| 1 | ings.c | 970 | 800164F8 (+1AA8) | .sdata 800A30DC | `main()`, sys_Init/GameInit/Panic, pcdrv_*, gpu_InitDisplay, obj_*, rng_*, scratchpad_* (boot + main loop) | first text object; LEGACY (splat) | – |
| 2 | ings_strings.c | 26 | – | .rodata 80010000+68 | ings.c's debug/build-date strings, referenced only by ings.c | rodata-cleanup sub-TU (2026-06-09), not original | – |
| 3 | code6cac.c | 1352 | 80017FA0 (+17FC) | .rodata 80010068+3C | 12 funcs, pad_ResetState | LEGACY (splat 6CAC) | – |
| 4 | code6cac_tu2.c | 5466 | 8001979C (+D608) | .rodata +388, sdata, sbss | 94 funcs: bitstream decode, math_*, ratan2/RotMatrix-heavy | PHASE site 1 | – |
| 5 | code6cac_b.c | 304 | 80026DA4 (+558) | .rodata 8001042C | 1 func | LEGACY (2026-03-24 asm-limit split); also LBRUN start | LB |
| 6 | code6cac_b_tu2.c | 6627 | 800272FC (+D0F4) | .rodata +3A4, sdata | 80 funcs, math_LerpAngle; ratan2 ×28 | PHASE site 2 | LB |
| 7 | code6cac_b_tu3.c | 133 | 800343F0 (+318) | .rodata 8001081C | 2 funcs | PHASE site 3 moved by GP (Q65 step 2) | LB |
| 8 | code6cac_b_rodata_pre.c | 3 | – | .rodata 80010868+4 | one zero word (`_bb2_101C_pre_lead`) | rodata-cleanup artifact; an alignment pad under the object-relative model (Q101/102 class) | – |
| 9 | code6cac_b3.c | 178 | 80034708 (+880) | .rodata, sdata | func_80034708 | G8 | GP, LB |
| 10 | code6cac_b3_post.c | 253 | 80034F88 (+4B0) | – | 4 funcs | G8 (b3 end); LBRUN end | LB |
| 11 | code6cac_b2_post.c | 676 | 80035438 (+AF8) | .rodata, sdata | bits_*; Q65 merge group (b2_pre + replay_camera_rob_back_loose2 + b2_post) | LBRUN; GP merge | – |
| 12 | code6cac_b4.c | 46 | 80035F30 (+78) | – | cdrom_SetMix, func_80035F78 | G8 | GP |
| 13 | code6cac_b4_post.c | 81 | 80035FA8 (+198) | sdata | cdrom_Init/FlushInit/ReadyCallback, snd_SerialMixOn | G8 (b4 end) | – |
| 14 | code6cac_b5.c | 367 | 80036140 (+C48) | .rodata 80010938 | 2 funcs: CD stream stepper (CdControlF ×13) | G8 | GP |
| 15 | code6cac_b5_post.c | 269 | 80036D88 (+864) | – | cdrom_* ×8, game_FrameLoop, sys_Exec | G8 (b5 end) | – |
| 16 | code6cac_b_rodata_post.c | 5 | – | .rodata 800109B0+28 | "bu%1d%1d:*"…, referenced only by code6cac_c.c | rodata-cleanup sub-TU; sits exactly where code6cac_c's rodata goes | – |
| 17 | code6cac_c.c | 419 | 800375EC (+728) | – | memcard_* ×10 (events, open/read/write) | LEGACY (asm-limit split) | – |
| 18 | code6cac_c0.c | 116 | 80037D14 (+1F4) | .rodata 800109D8 | 1 func (_card_info/_load/_clear) | LEGACY (2026-04-14 CU split) | – |
| 19 | code6cac_c_mid.c | 2018 | 80037F08 (+2C3C) | .rodata +2B8, sdata, sbss | memcard_Format, comb_* ×10 (link cable / SIO) | LEGACY (CU split) | – |
| 20 | code6cac_c_ab.c | 644 | 8003AB44 (+E8C) | .rodata 80010CA4+CC | 17 funcs | LEGACY (CU split). Pad-survey R2: merging into c2 is byte-proven | – |
| 21 | code6cac_c_ab_pad.c | 24 | – | .rodata 80010D70+4 | `.word 0` pad | retired by the R2 merge | – |
| 22 | code6cac_c2.c | 2703 | 8003B9D0 (+47FC) | .rodata, sdata, sbss | stage_* (collision, lighting), bitstream_ReadBits, game_GetCharData; Q65 group c2 + config | GP merge | – |
| 23 | text1a_pre.c | 549 | 800401CC (+B7C) | .rodata, sdata, sbss | gpu_AddDrawMove + 11 | G8 (owner 2026-08-05) | GP |
| 24 | text1a_pre_tu2.c | 331 | 80040D48 (+7B4) | .rodata 80010DB8 | 4 funcs | PHASE site 4 | GP |
| 25 | text1a_svc.c | 32 | 800414FC (+40) | – | save_vc_ctrl | LEGACY (a tooling split for file-scope asm under -G8, 2026-10-01) | GP |
| 26 | text1a_post.c | 628 | 8004153C (+FC8) | sdata, sbss | player_Destroy/SetCharId + 19 | LEGACY inside the -G8 run | GP |
| 27 | text1a_c.c | 1347 | 80042504 (+22FC) | .rodata 80010DD4 | math_RotMatrix*, gpu_OffsetTex* | G8 (the run ends) | – |
| 28 | text1a_c_tu2.c | 820 | 80044800 (+18E4) | sdata, sbss | seq_* + 41 | GP split (Q65 step 4) | – |
| 29 | text1a_filepaths.c | 574 | – | .rodata 80010DEC+44C8 | 17.6 KB of file-path strings; owning TU unproven | rodata-cleanup sub-TU | – |
| 30 | text1b.c | 2550 | 800460E4 (+4264) | .rodata 800152B4+13C, sdata, sbss | Q67 merge (text1a_c2 + text1a_b + sound + text1b head): game_Init…, camera_*, stage_*, snd_AllocSe | GP merge (Q67); tail is the Q89 cut | GP |
| 31 | text1a_b_pre_rodata.c | 325 | – | .rodata 800153F0+46C | D_800153F0 block | Q94: its own .rodata-only -G0 file between the Q89 parts | – |
| 32 | text1b_tu1b.c | 6233 | 8004A348 (+16720) | .rodata 8001585C+58, sdata, sbss | 160 funcs (71 canonical INCLUDE_ASM): math/gte helpers, snd_* sound-bank loader | G8 (Q89 cut) | – |
| 33 | text1a_b_pre_rodata_b.c | 18 | – | .rodata 800158B4+2C | 2 strings owned by snd_LoadCommonVab and func_8005C2A8, both in text1b_tu1b | rodata-align doc §9 | – |
| 34 | text1b_tu1c.c | 6487 | 80060A68 (+DACC) | .rodata 800158E0+D0, sdata, sbss | 147 funcs: effects/primitives, game_Cleanup | PHASE §9 + GP boundary move | – |
| 35 | text1b_tu1d.c | 2331 | 8006E534 (+4FF8) | .rodata 800159B0, sdata, sbss | 28 funcs | G8 | GP |
| 36 | text1b_tu1e.c | 633 | 8007352C (+12AC) | – | 5 funcs | G8 (tu1d end) | – |
| 37 | text1b_b.c | 3059 | 800747D8 (+4A6C) | .rodata 80015A0C, sdata, sbss | game code 800747D8..80078948 (Q67: text1b_tu2 + text1b_b), **then Sony LIBAPI stubs/COUNTER/PAD/PATCH/C68/SENDPAD/CHCLRPAD and LIBC2 memcpy/rand/strcpy/strlen/printf, 80078948..80079244** | PHASE site 5 | mixed: maspsx -G8 |
| 38 | text1a_b_mid_rodata.c | 4 | – | 0 bytes | empty marker file | – | – |
| 39 | main_post.c (data tail) | (80) | 8008D070..8008D120 | – | g_data_start, g_module_func_tbl, g_sqrt_table_u8: game data in .text | – | – |

#### 1b. Sony library (.text 0x80078948..0x8008D070, LIBSCAN census)

| File | .text | .rodata | Modules (link order) | Cut quality |
|---|---|---|---|---|
| text1b_b.c (tail) | 80078948..80079244 | none | LIBAPI C67 C112 C159 A08 A09 A11 A12 A36 A37 A39 A50 A52 A53 A54 A65 A66 A67 A91 COUNTER PAD A18-A21 L02 L03 PATCH C68 SENDPAD CHCLRPAD; LIBC2 MEMCPY RAND STRCPY STRLEN PRINTF | head = PHASE 5 (game); tail = PHASE 6 (prnt) |
| text1b_b_tu2.c (363) | 80079244..80079A30 | 80015A68+214 | LIBC2 PRNT CTYPE MEMCHR PUTCHAR | PHASE 6 / 7 = module starts |
| text1b_b_tu3.c (337) | 80079A30..8007A28C | 80015C7C+DC | LIBC(2) SPRINTF: exactly 1 module | clean |
| text1a_b_post_rodata.c (240) | – | 80015D58+4D4 | strings of LIBGPU EXT/PRIM/SYS, LIBCD EVENT/SYS/BIOS (owners: gpu.c, display.c, system.c only) | sub-TU |
| gpu.c (523) | 8007A28C..8007B244 | none | LIBC MEMMOVE; LIBCARD C171 C172 CARD A78 A80 INIT A74 A75 A76 END; LIBGPU EXT PRIM; **SYS head 8007AE7C..** | tail cut is **mid-module** (LIBGPU/SYS) |
| display.c (1563) | 8007B244..8008008C | none | **LIBGPU SYS rest**; LIBAPI C73; LIBGTE ×37 (GEO_00..PATCHGTE); LIBCD EVENT | head mid-module; 25 PAD_NOPS (pad survey) |
| system.c (1130) | 8008008C..8008289C | 8001622C+14 (getintr jtbl); .data CD_intr | LIBAPI A07; LIBCD SYS BIOS; LIBC2 PUTS; **LIBCD CDREAD head** | tail cut mid-module (CDREAD) |
| text1a_b_tail_rodata.c (94) | – | 80016240+180 | strings of LIBCD BIOS, PUTS, CDREAD, LIBETC VSYNC/INTR/INTR_DMA | sub-TU |
| ings2.c (611) | 8008289C..80083BE4 | none | **CDREAD tail**; LIBETC VSYNC; LIBAPI L10; LIBETC INTR; LIBAPI C114 A23 A24 A25; LIBC2 SETJMP; LIBETC INTR_VB INTR_DMA VMODE; *gap 80083698..8008386C: PCopen/PCclose/PClseek/__SN_ENTRY_POINT/__main/__do_global_dtors (SN runtime, not scanned)*; LIBAPI C57; *gap 8008387C..80083954*; LIBSND SSEND SSINIT(_C/_H) SSINIT SSNOFF SSSATTR | head mid-module |
| main.c (4035) | 80083BE4..8008BE04 | 800163C0+DC | LIBSND SSSMV..VS_VTC (32 verbatim + 9 gaps of newer-build LIBSND, named by xref/near tier, e.g. _SsSndCrescendo 800841E0); LIBSPU S_INI..S_GVEX (29 verbatim + 2 gaps); LIBAPI A13, A10 | module-aligned at both ends |
| comb.c (503) | 8008BE04..8008D050 | 8001649C+5C | LIBCOMB COMB: exactly 1 module | clean |
| main_post.c (80) | 8008D050..8008D070 | – | LIBAPI A71 AddDrv, A72 DelDrv (+ the game data tail, row 39) | mixed (Q69 follow-up) |

Rodata order already follows module link order:
- post_rodata: GPU strings → LIBCD SYS → BIOS data.
- system.o: getintr's jtbl at 0x8001622C.
- tail_rodata: BIOS strings "CD_sync"… → PUTS → CDREAD → VSYNC v_wait → INTR trapIntr → INTR_DMA.
- main.o, then comb.o.

The transcribed library strings can therefore move into their modules without reordering anything.

### 2. Proposed target layout (SOTN model)

SOTN reference: `src/main/psxsdk/<lib>/<module>.c` uses the lowercase PsyQ object names, and a BIOS
stub is a 1-line file (`libapi/a07.c`). Game TUs are address-named (`src/dra/42398.c`, the ROM file
offset). Data-only TUs are `d_<off>.c`. Headers live in `include/psxsdk/lib*.h`, plus one overlay
header and per-lib `*_internal.h`. Per-file overrides are a first-line `//!` comment; there are no
per-directory flags.

```
src/main/                       # the SLUS_006.63 executable (room for src/movovl/ later)
  6CF8.c  87A0.c  9F9C.c  175A4.c  17AFC.c  24BF0.c  24F08.c  25788.c  25C38.c  26730.c  267A8.c
  26940.c 27588.c 27DEC.c 28514.c  28708.c  2B344.c  2C1D0.c  309CC.c  31548.c  31CFC.c  31D3C.c
  32D04.c 35000.c 368E4.c 3AB48.c  51268.c  5ED34.c  63D2C.c  64FD8.c  7D870.c
  d_1068.c (until Q101/102 retires it)  d_15EC.c (file paths)  d_5BF0.c (Q94 block)
  (address names shown; per D6 a file whose functions carry accepted names takes a subsystem name instead)
  psxsdk/libapi/{a07,a08,...,c67,c68,c73,c112,c114,c159,l02,l03,l10,counter,pad,patch,sendpad,chclrpad,a71,a72}.c
  psxsdk/libc2/{memcpy,rand,strcpy,strlen,printf,prnt,ctype,memchr,putchar,puts,setjmp,sprintf,memmove}.c
  psxsdk/libcard/{c171,c172,card,a74,a75,a76,a78,a80,init,end}.c
  psxsdk/libgpu/{ext,prim,sys}.c
  psxsdk/libgte/{geo_00,geo_01,msc00,msc01,msc06,msc09,mtx_000,...,fgo_06,ratan,patchgte}.c
  psxsdk/libcd/{event,sys,bios,cdread}.c
  psxsdk/libetc/{vsync,intr,intr_vb,intr_dma,vmode}.c
  psxsdk/libsn/<off>.c          # SN runtime gap (PCopen..__do_global_dtors); module spans not scanned
  psxsdk/libsnd/{ssend,ssinit,...,vs_vtc}.c + libsnd/<off>.c gap files
  psxsdk/libspu/{s_ini,spu,...,s_gvex}.c + libspu/<off>.c gap files
  psxsdk/libcomb/comb.c
include/psxsdk/{libapi,libc,libcard,libcd,libetc,libgpu,libgte,libsnd,libspu,libcomb,kernel}.h
include/{common,game,...}.h     # game headers, §3
```

Naming conventions:
- **Game TUs** are named by the ROM file offset of their first `.text` byte. That is splat's and
  SOTN's convention, and the same one `asm/data/7D920.data.s` already uses
  (offset = vaddr − 0x80010000 + 0x800). The current-to-target map is:

  | Current | Target | Current | Target | Current | Target |
  |---|---|---|---|---|---|
  | ings | 6CF8 | code6cac_b5 | 26940 | text1a_c_tu2 | 35000 |
  | code6cac | 87A0 | code6cac_b5_post | 27588 | text1b | 368E4 |
  | code6cac_tu2 | 9F9C | code6cac_c | 27DEC | text1b_tu1b | 3AB48 |
  | code6cac_b | 175A4 | code6cac_c0 | 28514 | text1b_tu1c | 51268 |
  | code6cac_b_tu2 | 17AFC | code6cac_c_mid | 28708 | text1b_tu1d | 5ED34 |
  | code6cac_b_tu3 | 24BF0 | code6cac_c_ab | 2B344 | text1b_tu1e | 63D2C |
  | code6cac_b3 | 24F08 | code6cac_c2 | 2C1D0 | text1b_b game part | 64FD8 |
  | code6cac_b3_post | 25788 | text1a_pre | 309CC | main_post data tail | 7D870 |
  | code6cac_b2_post | 25C38 | text1a_pre_tu2 | 31548 | | |
  | code6cac_b4 | 26730 | text1a_svc | 31CFC | | |
  | code6cac_b4_post | 267A8 | text1a_post | 31D3C | | |
  |  | | text1a_c | 32D04 | | |

  **D6 (resolved, Q106): subsystem file names now** where the file's functions already carry accepted
  names (e.g. 27DEC → memcard.c, whose 10/10 named functions are memcard_*); otherwise the ROM-offset
  name shown in the tree above. State the supporting function names in the commit body. Each file gets
  a one-paragraph top comment stating its boundary evidence class from §1 (a comment, not a new doc).
- **Library files** are named for the PsyQ module, only where LIBSCAN places it verbatim (177
  placements).
  - Gaps whose functions carry Sony names (by xref or near tier) become one address-named file per
    gap, inside the lib directory. Gaps stay one file per gap (Q106 D3).
  - Duplicate identities resolve by `docs/naming/libscan/ambiguous_resolutions.md`: SSINIT_C/_H,
    SEND/GS_106/SSNOFF/SSQUIT, S_R/S_W, two PLAY placements.
  - LIBC vs LIBC2: every C module matches LIBC2, and SPRINTF/MEMMOVE match both. The recommendation
    is `libc2/` for all of them, recorded in the file comment.

**Per-boundary verdicts**

*Rename-only.* The TU text is unchanged and the boundary is kept as-is. This is byte-neutral by
construction and covers all of 1a except the folds below, plus comb.c → `libcomb/comb.c` and
text1b_b_tu3.c → `libc2/sprintf.c`. To preserve: each `bb2.ld` line in place in every section;
GP/LIB/LB membership carried as paths; LF endings.

*Evidence-backed changes (each needs oracle + review; neutral expected, proven per step):*

| Change | Evidence class | What must be preserved / risk |
|---|---|---|
| ings_strings → into ings (6CF8.c) | string ownership: sole referrer, first in .rodata and .text | definitions at file top (rodata before function literals); reconcile the extern spellings; ings is cc1 -G0 |
| b_rodata_post → into code6cac_c (27DEC.c) | string ownership (`g_str_memcard_fmt`, D_800109BC used only there); occupies c's rodata slot | reconcile `extern s32 D_800109BC` vs `const char[12]` |
| pre_rodata_b → into text1b_tu1b (3AB48.c) | ownership (rodata §9 table); contiguous after 8001585C+58 | the arrays must sit after func_80058580 (0x80058580) and before snd_LoadCommonVab (0x8005B7C4); reconcile `extern s32 D_800158B4` |
| delete text1a_b_mid_rodata.c | contributes 0 bytes | none |
| text1b_b split: game 64FD8.c / LIBAPI modules / LIBC2 memcpy..printf | LIBSCAN; already-approved Q69 follow-up (docs/grind/handoff-2026-09-30.md:83, open-work item 6) | the lib parts become LIB (-G0 maspsx). Q69 measured neutrality for the -G8 form only, so prove the -G0 form. The 10 INCLUDE_ASM and 4 PAD_NOPS (#31-33) move with their modules; the pad-survey step 2 goes first |
| main_post split: libapi/a71, a72 + 7D870.c | LIBSCAN (A71/A72 end at 0x8008D070) | the data tail stays game (-G8; measured neutral, Q69) |
| text1b_b_tu2 → prnt/ctype/memchr/putchar | LIBSCAN + rodata ownership (prnt's strings at 80015A68) | locate CTYPE/MEMCHR/PUTCHAR rodata, if any, inside the 0x214 bytes |
| gpu.c + display.c re-cut into ~55 module files | LIBSCAN. **This crosses a LEGACY cut** (8007B244 is mid-LIBGPU/SYS) | needs the PAD_NOPS retirement first (pad-survey steps 3 and 6; REG09/12/13 under Q104) |
| system.c + ings2.c re-cut | LIBSCAN. **Crosses the mid-CDREAD cut 8008289C** | shared static helpers (`_memcpy`, set_alarm, …) go to `libcd_internal.h` (SOTN) only if that is byte-identical and moves-only (rule condition 4). Duplicating a helper or changing a `static` to extern is outside condition 4: leave that module unsplit and record it as an owner question. CD_intr `.data` goes with bios.c. **Open risk R1** applies |
| main.c → libsnd / libspu modules + gaps | LIBSCAN. Gap-internal cuts: xref/near tier only | `static` across modules (e.g. SsSeqCalledTbyT, static in SSCALL but used by SSSTART): making it extern is outside rule condition 4, so that cut waits for an owner ruling (record it as a question; keep the modules together meanwhile). Open risk R1 applies to main.o's tables |
| code6cac_c_ab + c_ab_pad → into c2 | pad-survey R2: byte-proven; existence by GP | owned by the Q101/102 lane; listed only for ordering |

*Stay merged (only plausible, no evidence). No split or merge in Phase 1:*
- the big game TUs (17AFC 6.6K lines, 3AB48 6.2K, 51268 6.5K, 9F9C 5.5K);
- every LEGACY game boundary (ings|code6cac, c|c0|c_mid|c_ab, the svc/post edges, …);
- text1a_filepaths, which has no proven owner.

`per-file-gp-model` says "legacy boundaries never move". A merge of two adjacent LEGACY files that
happens to be byte-identical is still a claim with no evidence (D5). Note that many LEGACY edges are
also flag edges (G8 or LBRUN), and merging those changes bytes anyway.

### 3. Header consolidation

**Census** (`externs.py`, a regex census, not a parser):
- 2,339 declarations in src: 2,129 `extern` plus 210 file-scope prototypes, covering 1,542 distinct
  symbols.
- 1,248 symbols are declared in exactly 1 file; 294 in 2 to 11 files.
- 108 symbols have more than one spelling. Some differ only in parameter names. The real type
  conflicts include:
  - `g_gpu_ot_ptr`: s32 vs u8*
  - `VSync`: void vs s32
  - `printf`: 4 forms
  - `rand`: s32() vs u16(void)
  - `D_800A38B4`: 4 types
  - `D_800A3894`: s32 vs u8* (blocks the R2 merge)
  - `math_RotMatrixZYX`: 3 typings
  - `LoadImage`: 3 typings
  - `game_FrameLoop`: void vs s32
- 64 src declarations duplicate a header declaration; only 3 of those disagree with it.
- 97 are block-scope externs: text1b_tu1c 75, code6cac_c_mid 16, text1b_b 8. They are stylistic
  remains of per-function landings.
- One extern is FAKE-labelled: `ings2.c:173 _96_remove`. The Q99 admission in `system.c:942` and the
  Q63/Q73/Q96-Q98 admissions are cross-symbol views and aliases, catalogued in
  `aggregate-merge-family.md`. They stay where they are.

**Plan**
1. `include/psxsdk/lib*.h` hold the Sony prototypes. A prototype moves there only when every
   declaring file spells it identically, or SOTN's spelling is object-identical in every file. The
   current Sony-flavoured headers fold in:
   - `libcd.h` → `psxsdk/libcd.h`
   - Sony parts of `gpu.h`, `gte.h`, `bios.h` → `psxsdk/libgpu.h`, `psxsdk/libgte.h`,
     `psxsdk/libapi.h` / `kernel.h`
2. Game headers. `code6cac.h` (1141 lines, named after a segment) and `game.h`, `gpu.h`, `sound.h`,
   `system.h` become:
   - `include/game.h`: types and structs; the Phase 2 Entity type lands here.
   - one declarations header for the executable (SOTN `dra.h` style) for multi-file, identical game
     symbols.
   - optional prefix headers only where the prefix is an accepted name (cdrom_, memcard_, snd_, …),
     about 120 symbols.

   Single-file externs stay local. That is SOTN-normal (`libcd/c_011.c` has 24), and hoisting 1,248
   of them buys nothing but risk.
3. The 108 conflicting spellings stay where they are. They are Phase 2's typed-reconciliation
   worklist (completion-bar item 4): each one is an evidence-typed, byte-identical commit, as Q65
   step 6/10 did.
4. Drop the 61 local duplicates of identical header declarations.
5. Block-scope externs stay. Hoisting is allowed only per file with an object-identical proof; it
   is optional hygiene.

**Codegen risks of moving a declaration** Every header step needs a per-file object compare of all
sections, relocations and symbols, not only the oracle.
- **cc1 -G8 files** (9 GP files): gp-relativity depends on the declared size of each referenced
  extern. `extern s32 S` is small; `extern s32 S[]` is not. A header must reproduce each referenced
  symbol's size class exactly.
- **Implicit to prototyped calls:** adding a prototype changes argument promotion (s16/u8 parameters
  truncate or extend) and return extension. Bank each file's implicit-declaration set before and
  after; a tool existed at `pre-slim-2026-10-01:memory/grind/func_80065800/tools/implicit_cmp.sh`.
- **K2 statics** must never appear in a header: `extern T S;` before `static T S;` is a hard error.
  This is already the per-file-gp-model rule.
- **No definitions in headers** (per-file-gp-model). K1 `T S;` tentatives stay in their defining
  files.
- **Declaration order:** none affects bytes for externs. A block-scope extern hoisted next to a
  differently-typed sibling in another function of the same file is a compile error, so the build
  catches it.

### 4. Tooling impact (everything keyed on src stems or paths)

**Design.** A "TU id" is the path under `src/` without `.c`, in posix form (`main/psxsdk/libgpu/sys`).
Every existing `f"src/{stem}.c"` and `f"build/src/{stem}.o"` construction keeps working with it. The
template is `engine/departures.py:307-310`, which already uses `glob("**/*.c")` +
`relative_to("src")`. Bare basenames must go: `sys.c` exists in both libgpu and libcd.

**What breaks, by severity**

*Build/link (wrong bytes or link failure):*
- `bb2.ld`: 190 `build/src/<stem>.o` lines over 49 objects (rodata 47, text 39, data 38, bss 38, sdata 17, sbss 11).
- `Makefile:71-72`: a non-recursive `wildcard src/*.c`; moved files silently drop out.
- `Makefile:125,133,138` and `engine/buildconfig.py:76-81`: the GP/LIB/LB stem lists. If they keep
  bare stems, the flags silently stop applying.
- `tools/psyq_library_files.py:43-57`: the `"(\w+)"`, `build/src/(\w+)\.o` and `os.listdir` patterns.

*Engine discovery (raises "not found"):*
- `fixtures._file_index` (`engine/fixtures.py:58-68`) globs `build/src/*.o`.
  - It is **already unsafe today**. `build/src` holds 6 stale objects: code6cac_b2_pre, config,
    sound, text1a_b, text1a_c2, replay_camera_rob_back_loose2.
  - The glob is sorted and the last match wins, so a stale object can shadow the live owner.
  - Fix: iterate only the objects `bb2.ld` links.
- `pipeline.c_stems()`, which feeds build_all, parity, oracle, orchestrator, queue and the integrity
  audit.
- `datamodel._src_texts`, `dossier:66`, `buildstamp:54`, `volatile_cheats:1746`,
  `layer2.locate_stem` fallback.

*Records keyed by stem (rewrite through the rename map):*
- `tools/canonical_asm_regions.json`: 49 grants. Re-hash the islands from the new file before
  relocating each one, as was done in rodata §7.
- `tools/cc1_tu_expectation.txt`: re-record with `build_oracle_cc1.sh --record-manifest`, after
  fixing its `ls src/*.c`.
- `oracle/manifest.json`: 52 corpus keys; re-lock.
- `queue.json` `file`: 0 items.
- Grind ledgers: about 1,400 informational `file` fields. Rewrite them, or leave them for the rename
  map to resolve.

*Grinder / tools:*
- `grind.ps1` (`src/$stem.c`) works with ids; posix slashes are needed at `:755`.
- `grindlib.py:2254-2267, 2945-2971` (listdir) and `:184-191` (the `len(parts)==2` citation check).
- `dump.ps1:36` and `reopen_regressions.py:46`.
- About 25 analysis tools use non-recursive globs: audit_asm_cheats, classify_inline_asm,
  desync_audit, reviewer_precheck, naming_wave, kengo_*, data_wave, rename_funcs, and others.

*Rules and docs:*
- `.claude/rules` `paths:` globs: `src/*.c` → `src/**/*.c` in 6 rules. The file-specific globs in
  `packed-multiply-cluster` and `rodata-object-alignment` also change.
- `doc_budget_guard.py` `SRC_PROBE` → a nested path.
- 2,738 citations in 289 docs. Keep them historical: resolve at a tag plus the rename map, and update
  only current-state docs, rules and skills.

*Unaffected:*
- The root `*_funcs.txt` / `*_syms.txt` gate lists and `inline_asm_canonical.txt`, which are keyed
  by function or symbol.
- Hooks: `park_src_guard`, `worktree_contamination_guard` and `metrics` already match nested paths.
- Layer-2 records.

**Minimal engine change (step 0)**

One new module, `engine/tus.py`:
- `linked_tus()`: the ordered TU ids parsed from `bb2.ld`'s `build/src/(.+)\.o` lines.
- `src_tus()`: `rglob` over `src`.
- `check()`: every src TU is linked and every linked TU exists.
- `RENAMES`: loaded from a tracked rename map `tools/tu_renames.tsv` (old id → new id).

Call-site changes:
- `c_stems`, `_file_index`, `_src_texts`, `dossier`, `buildstamp`, `layer2.locate_stem` and
  `volatile_cheats` all call `engine/tus.py`.
- buildconfig sets hold ids.
- `Makefile`: `C_FILES := $(shell find src -name '*.c')`; lists hold ids.
- `tools/restructure/move_tu.py` does one move per invocation:
  - `git mv`
  - rewrite every `bb2.ld` section line in place
  - Makefile + buildconfig lists
  - canonical_asm_regions (with island re-hash)
  - cc1 expectation / oracle manifest refresh hooks
  - append to `tu_renames.tsv`

**Tests** (`engine test`):
- `c_pipeline_cmd` for a nested id;
- two TUs with the same basename in different dirs: no collision in `_file_index` / `_src_texts` /
  flag sets;
- `tus.check()` detects an unlinked or a missing TU;
- `_file_index` ignores a stale unlinked `.o`;
- buildconfig mirrors the Makefile with path ids;
- `psyq_library_files --check` on nested ids;
- a `move_tu.py` round trip on a scratch tree gives an identical link.

### 5. Ordered steps (each is one commit: `verify-oracle --rebuild` + `engine test` + reviewer PASS where policy-relevant)

Preconditions:
- Every in-flight lane is landed: Q96-Q103 cheat-cleanups, Q103 naming, the pad-survey Q101/Q102
  steps 0-6 (Part A). Start from a clean tree; the 2026-10-03 cheat-cleanup and naming lanes have landed.
- The Grinder is stopped. A quiet window exists, because renames conflict with every concurrent
  edit.
- Tag `pre-restructure-<date>`.

| Step | Commit(s) | Risk | Effort |
|---|---|---|---|
| 0 | engine/tools: TU ids, `tus.py`, linked-set discovery (fixes stale-object shadowing), Makefile recursion, path-valued flag lists, psyq_library_files, grinder/grindlib, the ~25 tool globs, rule `paths:` globs, doc probe, `move_tu.py`, tests. No src change; SHA1 must be unchanged | low | 1-2 sessions, 2-3 commits |
| 1 | Pilot moves: comb.c → `main/psxsdk/libcomb/comb.c`, text1b_b_tu3.c → `main/psxsdk/libc2/sprintf.c` | very low | small |
| 2 | Rename every game TU and data-only file to its §2 name: subsystem name per D6 where supported, else address name (pure `git mv` + `bb2.ld`/list rewrite; no content edits, to keep git rename detection), plus records relocation | very low | 1 to 3 commits |
| 3 | Folds with ownership evidence: ings_strings, b_rodata_post, pre_rodata_b; delete mid_rodata. Each with its declaration reconciliation and an object compare | low | 3 to 4 commits |
| 4a | text1b_b: game / LIBAPI / LIBC2 module split (docs/grind/handoff-2026-09-30.md:83, open-work item 6) | medium | 1-2 commits |
| 4b | main_post: a71/a72 + 7D870.c | low | 1 |
| 4c | text1b_b_tu2: prnt/ctype/memchr/putchar | low-medium (rodata) | 1 |
| 4d | gpu.c + display.c re-cut into libc2/libcard/libgpu/libapi/libgte/libcd-event modules; post_rodata distributed. After pad-survey step 6 | medium | 2-3 |
| 4e | system.c + ings2.c re-cut into libapi/libcd/libc2/libetc/libsn-gap/libsnd head; tail_rodata distributed | medium-high (R1) | 2-3 |
| 4f | main.c → libsnd/libspu modules + gap files | medium | 2 |
| 5a | `include/psxsdk/*.h` (Sony prototypes, identical spellings only) | medium | 2-3 |
| 5b | game headers: `code6cac.h` and friends → game.h + an executable declarations header; drop the 61 duplicates; multi-file identical game declarations | medium | 2-3 |
| 6 | D10 dump deletion (after checking every tool that edits them); current-state docs, rules, skills, STATUS; rename-map pointer in AGENTS.md | low | 1 |

**Status (2026-10-03):** step 0 done: 7bdb18ff6 (bb2.ld: 0-byte section lines into one order),
5f4387b95 (engine/tus.py, linked-set `_file_index`, `tus-check`, verify-oracle `layout_problems`),
dcf6ac4f4 (Makefile recursion; a flag-list id that is no TU stops the build), 412872a5f (tool globs,
`tools/move_tu.py`), 468ced368 (rule `paths:` globs), a38713186 (oracle re-lock). Step 1 done:
2c35cbd7d (comb -> main/psxsdk/libcomb/comb), 60985b4e9 (text1b_b_tu3 -> main/psxsdk/libc2/sprintf).
Every later move: `tools/move_tu.py OLD NEW` (dry run), then `--apply`, then `make clean-check` and
`verify-oracle --rebuild`; re-run `oracle-lock` after a batch. Open: `build_oracle_cc1.sh` /
`build_diagnostic_cc1.sh` hard-code the -G8 set as `text1a_pre|text1a_post` (stale; step 2 renames
those ids); nested `src/` citations are not existence-checked by grindlib's self-vet check.

Total: about 25 to 30 commits, 5 to 8 focused sessions. Step 4 (library rodata placement per
module) and step 5 (per-file header proofs) dominate.

**Order with later phases:**
- Phase 2 (Entity typing, ~1,238 raw-offset casts) should start after steps 2 and 5b. Then the type
  lands once in `game.h`, and git history shows the moves separately from the content edits.
- The 108 conflicting declarations are Phase 2's entry worklist.
- Phase 3 naming can turn the remaining address-named game files into subsystem names file by file.

### 6. Owner decisions

Resolved 2026-10-03 as Q106: see § 1 of this hand-off (D6 as amended by the orchestrator for readability and approved by the owner with the plan).

### 7. Open risks

- **R1. Library jump-table phases.** Sony built its libraries with its own compiler, so their tables
  need not follow the object-relative `.align 3` model the game follows.
  - Example: getintr's jtbl at 0x8001622C (phase 4) follows BIOS-owned strings from ~0x800161B8
    (phase 0). Today `system.o` starts exactly at the table.
  - A bios.c holding both may not reproduce the bytes. If not, those module rodata objects can't be
    unified without a new owner rule.
  - Run the rodata-align tools (`pre-slim-2026-10-01:memory/grind/func_80058580/aspsx-align-check/poc/{owners,sitewin}.py`, restored
    with `git show`; no jump-table phase checker is banked, so write one) per library module before step 4e. The same check applies to main.o's tables
    (4f) and prnt's (4c).
- **R2.** Splits can duplicate a previously shared string literal or change a `static` symbol's
  linkage. The oracle catches the bytes; a `static` change moves that function's layer-2 key, so a
  fresh review is needed.
- **R3.** Header moves into the 9 cc1 -G8 files, and prototypes for implicitly-declared callees, can
  change codegen. Mitigation: a per-file object compare plus implicit-set banking (§3).
- **R4.** Tree contention. Renames conflict with every concurrent edit: one lane at a time on the tree,
  Grinder off, during Phase 1.
- **R5.** `build/src` holds stale objects that the current `_file_index` can already mis-attribute.
  Fix it in step 0, and do a clean build (`make clean` + `verify-oracle --rebuild`) after every move
  step.
- **R6.** `PAD_NOPS` and canonical asm blocks in display.c and text1b_b.c are tied to module
  boundaries. Part A (incl. Q104 for REG09/12/13) must land before 4a and 4d.
- **R7.** Rules and docs drift. Rules matching `src/*.c` stop auto-loading for nested files until
  their globs change (step 0). The 60 KB budget check probe must follow.
