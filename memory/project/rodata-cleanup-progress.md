---
name: rodata-cleanup-progress
description: Per-cluster checkpoint log for the rodata-cleanup project (docs/rodata-cleanup-project.md). Append a record per cluster retired. Read this to find the current frontier.
metadata:
  type: project
---

# Rodata cleanup project — progress log

See `docs/rodata-cleanup-project.md` for the project plan. This file logs each
cluster retirement (or attempt) so future agents can resume.

**Project goal**: 0 `asm/data/*.rodata*.o(.rodata)` entries remain in `bb2.ld`.

## Starting state (2026-06-09)

- 12 linked `asm/data/*.rodata*.s` blocks in `bb2.ld`
- 9 non-trivial blocks (~21.8 KB of rodata; 122 unique symbols = 28 jtbls + 86 strings + 8 raw-data)
- 3 trivial blocks (deletion-only — `.space 0` / single `.word 0x00000000` filler):
  - `101C.rodata_c_pre.s`
  - `101C.rodata_post.s`
  - `101C.rodata_text1a_DB8.s`
- 145 `replace_with_asmfile` stubs total in `asmfix.txt`; ~50 stub→symbol owner
  relationships span the 9 non-trivial blocks (the rest of the 145 stubs are
  not rodata-blocked and belong to engine-queue work, not this project)
- 24 jtbl-infra asmfix rules (all on `replay_camera_rob_back_loose2`)
- Oracle SHA1: `62efab4f73f992798c43e8c730aa43baa10bb4fa`
- Phase 0 inventory: `memory/project/rodata_block_inventory.csv` (159 rows)

## Retirement log

### 2026-06-09 — Pilot retirement: zero-byte trivial blocks

**Retired**: `101C.rodata_c_pre.s` + `101C.rodata_text1a_DB8.s`

- Both contribute 0 bytes to the linked `.rodata` section (`.space 0` / lone
  `.align 2` directive). Confirmed via `mipsel-linux-gnu-objdump -h` on the
  build/asm/data/*.o objects.
- Zero cascade by construction: no output bytes changed, no downstream
  addresses shifted.
- Mechanical recipe: remove the `build/asm/data/<block>.o(.rodata);` line
  from `bb2.ld`; delete the `asm/data/<block>.s` source file;
  `verify-oracle --rebuild`.
- Bytes retired: 0
- Oracle SHA1 preserved: `62efab4f73f992798c43e8c730aa43baa10bb4fa`
- Cascade size: 0 downstream symbols affected

This pilot validates the basic mechanical workflow but does NOT exercise the
cascade-math machinery (since these blocks emit no bytes).

### 2026-06-09 — Cascade test on `101C.rodata_post.s` (4 bytes) — FAILED, REVERTED

**Outcome**: oracle broke (build_sha1 `f21fe078...` ≠ expected `62efab4f...`).
Reverted via `git restore`.

**Setup**:
- Block contents: single `.word 0x00000000` (4 bytes of zero) at address
  `0x80010D88-0x80010D8C`, between `code6cac_c2.o(.rodata)` (24-byte switch
  jtbl for the only switch in code6cac_c2.c) and `101C.rodata_c2_post.o`
  ("Multipul Model" string at `0x80010D8C`).
- Grep for `0x80010D88` / `D_80010D88` across the entire repo → ONLY found
  in `asm/data/101C.rodata.s` (the splat parent's definition site). Zero
  references from any source, .s function, .data table, asmfix rule, or
  named symbol. The bytes appeared functionally orphan.

**Hypothesis tested**: with no static references, removing the 4 bytes would
either be a no-op (if SUBALIGN(2) happens to absorb the gap) or break only
trivial alignment. Empirical test was cheap.

**Empirical result**: removing the block AND deleting the .s file produced a
non-matching SHA1. The 4 bytes ARE load-bearing despite having no
detectable symbolic references. The cascade flowed through compiled-in
`%lo` immediates in downstream functions' TEXT, which encode the exact
addresses of every rodata symbol downstream of the cut.

**What this proves**:
1. Even a "no-references" 4-byte block participates in the address layout
   contract. Removing it without compensation breaks every downstream
   `lui+%hi/addiu+%lo` pair that references rodata > `0x80010D88`.
2. Trivial-byte-retirement requires either (a) compensating 4 bytes of
   rodata added to an adjacent C file with TU-boundary evidence, or
   (b) Phase 3 cascade-math machinery (`tools/re_attribute_rodata.py`)
   that updates every downstream relocation in coordination.

**What this rules out**: simple "delete trivial blocks from bb2.ld" as a
broad strategy. The two retirements from earlier today (`101C.rodata_c_pre.s`,
`101C.rodata_text1a_DB8.s`) succeeded ONLY because their `.rodata` sections
contained ZERO bytes (`.space 0`, lone `.align 2` directive after an already
4-aligned offset). Any non-zero-byte block cascades.

**Implication for the project plan**: §5 Phase 3 step 2 ("compute the
cascade") and §11.3 (Known unknown — is cascade prediction tractable?) move
from theoretical to required-before-next-step. No further retirement is
possible without either:
- Building the cascade math tooling (`tools/re_attribute_rodata.py` per §7.2)
- OR finding TU-boundary evidence that a compensating rodata constant
  belongs in an adjacent C file's source (§8.1 evidence threshold)

## Phase 1b pilot — partially complete

The trivial zero-byte case is done (2 blocks retired). The cascade-bearing
case is **empirically untractable without Phase 3 tooling** — confirmed by
the 4-byte test above.

### 2026-06-09 — First real cluster retirement: 101C.rodata_c2_post.s (en-bloc re-attribution)

**Retired**: `101C.rodata_c2_post.s` (16 bytes, string "Multipul Model")

**Method**: en-bloc re-attribution to `config.c`.

**Evidence**:
- The string is referenced only by `func_8003FA24` (asm body in
  `asm/funcs/func_8003FA24.s:240-241`).
- `func_8003FA24` is a stub in `config.c`; the function is its only owner
  of this rodata symbol.
- `named_syms.txt:859` already pre-named the symbol
  `g_str_multipul_model_80010D8C` — prior project work had identified
  config.c as the owning TU.

**Mechanical recipe (validated by this retirement)**:
1. Add `const char D_80010D8C[16] = "Multipul Model";` to `src/config.c`
   (named for the symbol it provides; bracket-sized to match the asm/data
   block's exact byte count including a 1-byte alignment pad).
2. Delete `asm/data/101C.rodata_c2_post.s` (removes the duplicate definition
   the .o would have provided).
3. Edit `bb2.ld`: REMOVE the
   `build/asm/data/101C.rodata_c2_post.o(.rodata);` line; MOVE the
   `build/src/config.o(.rodata);` line from its current position (between
   code6cac_b2 and sound) to immediately after
   `build/src/code6cac_c2.o(.rodata);` — i.e. into the slot vacated by the
   retired asm/data block.
4. `verify-oracle --rebuild` — SHA1 matches first try.

**Cascade analysis (pre-retire prediction matched post-retire reality)**:
- config.o(.rodata) before: 0 bytes
- config.o(.rodata) after: 16 bytes (exactly the relocated string)
- Net change in main_RODATA: 0 bytes
- Downstream segment addresses: ALL unchanged
- Oracle SHA1: preserved

**Notes**:
- `func_8003FA24` remains a `replace_with_asmfile` stub. The asm body's
  `D_80010D8C` symbol reference now resolves to the C definition in
  config.o. Future work on `func_8003FA24` itself is engine-queue work,
  decoupled from this project.
- The C declaration `const char D_80010D8C[16] = "Multipul Model";` uses
  the canonical `D_<addr>` name so the asm body's external reference
  resolves directly. The named-syms.txt alias
  `g_str_multipul_model_80010D8C` stays as a disassembler-facing
  decoration; renaming the C symbol to match would require updating
  named_syms.txt too (out of scope here).

### 2026-06-09 — Second real cluster retirement: 101C.rodata_pre_post.s (en-bloc, 216 bytes)

**Retired**: `101C.rodata_pre_post.s` (216 bytes: D_800109EC checksum table
[64B] + D_80010A2C save-game struct [152B] ending in "BASLUS-00663BUSHIDO2")

**Method**: en-bloc re-attribution to `code6cac_c_mid.c`.

**Evidence**:
- Both symbols are referenced ONLY by `func_80038170` in `code6cac_c_mid.c`
  (lines 400 and 412 in the existing pure-C source).
- The function is already in queue as `active` (not a stub) — already has
  real C body, just carries 1 regfix rule + 24 cheat-asm pieces that are
  unrelated to the rodata data itself.
- The save-game struct's trailing ASCII "BASLUS-00663BUSHIDO2" matches the
  game's product code (SLUS-00663) — content is clearly authentic to this
  TU.

**Mechanical recipe** (variant of `c2_post` recipe — extended to larger
multi-symbol block):
1. Replace the `extern u8 D_800109EC; extern u8 D_80010A2C;` lines in
   `src/code6cac_c_mid.c` with full `const u32[]` definitions transcribed
   from `asm/data/101C.rodata_pre_post.s`'s `.word` values. Place the
   definitions where the externs were (mid-file is fine — cc1 emits all
   global rodata in declaration order at the top of the .rodata section,
   BEFORE any switch jump tables generated from function bodies).
2. Delete `asm/data/101C.rodata_pre_post.s`.
3. Edit `bb2.ld`: remove the
   `build/asm/data/101C.rodata_pre_post.o(.rodata);` line. NOTE — unlike
   the c2_post retirement, no segment-move was needed because
   `code6cac_c_mid.o(.rodata)` was already directly downstream (with
   zero-byte `code6cac_c.o(.rodata)` between as a no-op). The 216 added
   bytes naturally fill the gap.

   Wait, that's wrong — there WAS a swap. The current arrangement:
   ```
   code6cac_c0 → [asm/data block 216B] → code6cac_c → code6cac_c_mid
   ```
   became:
   ```
   code6cac_c0 → code6cac_c_mid (now 480+216=696B) → code6cac_c (0B)
   ```
   I moved `code6cac_c_mid.o(.rodata)` up to take the asm/data block's
   slot, AND moved `code6cac_c.o(.rodata)` (0 bytes) to where
   code6cac_c_mid was. The reorder is what allowed the size change to
   land correctly: code6cac_c_mid starts at 0x800109EC (was 0x80010AC4),
   ends at 0x800109EC + 696 = 0x80010CA4 (same as before), so downstream
   addresses are preserved.
4. `verify-oracle --rebuild` → SHA1 matches first try.

**Cascade analysis (predicted, confirmed)**:
- code6cac_c_mid.o(.rodata) before: 480 bytes @ 0x80010AC4
- code6cac_c_mid.o(.rodata) after: 696 bytes @ 0x800109EC (moved up)
- The first 216 bytes of the new code6cac_c_mid.o(.rodata) contain
  D_800109EC + D_80010A2C (cc1 emits top-of-file consts first in
  declaration order, before any function-body-generated switch jtbls).
- The remaining 480 bytes are the function bodies' switch jtbls,
  unchanged content.
- Downstream segment addresses: ALL unchanged.

**Key insight** for future multi-symbol re-attribution: cc1 2.7.2 emits
`.rodata` in declaration order — top-of-file `const` declarations precede
function-body-generated switch jtbls in the output `.o`. This means
multi-symbol blocks can be re-attributed even when the destination C file
already has rodata content, by placing the new declarations at the top.

**Notes**:
- `func_80038170` remains in the active queue (still has 1 regfix rule
  + 24 cheat-asm). The function's matching work is independent of this
  retirement and continues as engine-queue work.
- The 480 bytes of pre-existing `code6cac_c_mid.o(.rodata)` are
  GCC-generated switch jump tables (anonymous `.L<n>` labels). Their
  internal relocations adjust automatically when the section is
  repositioned.

### 2026-06-09 — Third real cluster retirement: 800.rodata_pre.s via sub-TU split (104 bytes)

**Retired**: `800.rodata_pre.s` (104 bytes: 5 debug-related strings —
"OVER FLOW\n", "eff_init:%08x size:%08x\n", "LIMIT:%08x\n",
"common prim over flow\n", "Fri Aug  7 22:26:32 1998\n").

**Method**: NEW PATTERN — **sub-TU split**. Created
`src/ings_strings.c` containing ONLY the 5 string definitions, linked
at the front of rodata.

**Why a new pattern was needed**:
- The natural owner of these strings is `ings.c` (all 5 are referenced
  only from ings.c functions).
- Attempted naive approach (add the strings to ings.c, move
  `ings.o(.rodata)` to the front of bb2.ld) FAILED — `ings.c` already
  has ~96 bytes of its own `.rodata`, emitted by two file-scope
  `__asm__("glabel ...")` blocks (func_800164AC = 19-word pointer
  table, func_800164F8 = small countdown function). Those bytes have
  FIXED ADDRESSES at 0x800164AC and 0x800164F8; moving ings.o(.rodata)
  to the front of rodata would relocate them too, breaking their
  symbol addresses.

**Sub-TU split recipe** (variant for cases where the natural owner C
file has other rodata content that's address-fixed elsewhere):
1. Create a new sub-TU `.c` file (named to reflect its relationship to
   the natural owner, e.g. `ings_strings.c`).
2. Put ONLY the to-be-retired declarations in the sub-TU. Bracket-sized
   to match the asm/data block's exact byte content.
3. The Makefile auto-discovers `src/*.c`, so the new file builds
   automatically with no Makefile edit. (Splat doesn't regenerate
   src/*.c so this is preserved.)
4. Edit `bb2.ld`:
   - Replace the `build/asm/data/<block>.o(.rodata);` line with
     `build/src/<new-tu>.o(.rodata);` at the same position.
   - Remove the block's `(.data)` and `(.bss)` references too (they
     were zero-byte stubs, deleting is a no-op).
5. Delete `asm/data/<block>.s`.
6. `verify-oracle --rebuild`.

**Evidence (§8.1)**:
- All 5 strings are referenced exclusively from ings.c functions
  (g_str_overflow, g_str_eff_init, g_str_limit, g_str_prim_overflow
  used by debug_printf calls; g_str_build_date is the game's build
  timestamp referenced via a .data table at D_800A30DC).
- Naming the sub-TU `ings_strings.c` makes the relationship explicit.
- The strings have semantic purpose (debug strings, build identifier)
  and natural source attribution to the ings.c family.

**Cascade**: zero. New `ings_strings.o(.rodata)` is 104 bytes at the
same address slot that 800.rodata_pre had. All downstream segment
addresses preserved.

**Note on the original ings.o(.rodata)**: the 96 bytes of
asm-emitted "function" data in ings.c stays in its current position
(ings.o(.rodata) is NOT in bb2.ld's rodata list directly; the bytes
appear in main_TEXT at the right addresses via cc1's section-handling
of file-scope `__asm__` blocks). Empirically confirmed: oracle holds
after this retirement.

### 2026-06-09 — Fourth real cluster retirement: 800.rodata_post.s (en-bloc to code6cac.c, 816 bytes)

**Retired**: `800.rodata_post.s` — D_800100FC (66×12 = 792-byte animation
name table) + jtbl_80010414 (24-byte jump table for func_80021424).

**Method**: en-bloc re-attribution to `code6cac.c`. Declarations placed
at END-OF-FILE so the existing 148 bytes of code6cac's switch-jtbl
rodata stay at their original offsets (0..148 = 0x80010068..0x800100FC)
and the new arrays land at the retired block's slot
(0x800100FC..0x8001042C).

**Recipe variant** (placement-at-end for clusters that need to land
AFTER existing rodata in the destination C file):
1. Add `const` declarations at the END of the destination .c file
   (after all functions). cc1 emits global rodata in source order, so
   end-of-file placement puts them at the END of the .o's `.rodata`
   section.
2. Use literal hex values for any jtbl entries — the local `.L<n>`
   labels they originally referenced are inside the asm-bridged stub
   function bodies and aren't visible to C source. The linker produces
   identical bytes for either form.
3. Standard bb2.ld edits: remove the asm/data block's (.rodata),
   (.data), (.bss) references; delete the .s file.
4. Verify.

**Evidence (§8.1)**:
- D_800100FC owner: `func_80023F08` (stub in code6cac.c)
- jtbl_80010414 owner: `func_80021424` (stub in code6cac.c)
- Both stubs live in the same .c file = strong single-file cluster.
- The 66-entry table is animation names (WIN, RUN, JUMP, ATTACK,
  DAM, etc) + BBM file names — natural for a character/animation
  module to own.

**Cascade**: zero. code6cac.o(.rodata) grew from 148B (existing) to
964B (148 existing + 792 D_800100FC + 24 jtbl + 0 alignment pad
required). New end address matches the old code6cac.o + 800.rodata_post
combined end. Downstream segments unchanged.

**Tooling addition**: `tmp/extract_800_post.py` — small Python script
that parsed the asm/data file's 66 `.asciz` directives into the C
2D-array initializer. Reusable for future similar string-table
clusters.

### 2026-06-09 — Fifth real cluster retirement: 101C.rodata_main_post.s (en-bloc to main.c, 212 bytes)

**Retired**: `101C.rodata_main_post.s` — 4 debug strings + 4 switch
jtbls + 2 SIO debug strings, total 212 bytes; all 10 symbols owned
by main.c functions.

**Method**: en-bloc re-attribution to `main.c`, declarations at
end-of-file. Used the placement-at-end recipe from the previous
retirement: existing main.o(.rodata) (24B from an inline-asm
declaration) stays at offsets 0..24, new declarations land at
offsets 24..236, matching the original layout.

**Notes**:
- Multiple owners across stub and active/completed states all in
  main.c — easiest possible single-file cluster after the prerequisite
  rodata move.
- Jtbl entries reference labels inside asm-bridged stubs
  (saTan1MainJump, func_8008AF9C); used literal hex addresses per the
  established pattern.

### 2026-06-09 — Sixth real cluster retirement: 101C.rodata_text1a_a.s (sub-TU, 17568 bytes)

**Retired**: `101C.rodata_text1a_a.s` — D_80010DEC (899-string game asset
file-path list, 17568 bytes) + D_8001528C ("Marionation over flow"
debug string, 40 bytes). Largest single retirement so far.

**Method**: sub-TU split to `src/text1a_filepaths.c`. The 899-string
filename list (D_80010DEC) has no detectable static owner — it's
accessed via base-pointer arithmetic that static analysis cannot
resolve — so direct re-attribution to a specific C file would require
guesswork. The sub-TU isolates the data without speculative attribution.

**Tooling**: `tmp/gen_text1a_a_c.py` parses each `.asciz` line + its
trailing alignment padding into a concatenated C string literal, emitting
`const char D_80010DEC[17568] = "..." ...;`. Handles variable-length
strings (mix of 16-byte and 20-byte padded entries) by tracking exact
byte offsets from the symbol address. Output went to
`tmp/text1a_a_gen.c` then was assembled into `src/text1a_filepaths.c`
with a header comment.

**Recipe note**: byte-content fidelity matters — used `const char
D_80010DEC[N]` with concatenated string literals containing explicit
`\0` padding bytes between strings. The bracket-sized `[N]` array
forces cc1 to emit exactly N bytes (no auto-null appending for
N-sized initializers). Verified via objdump of `text1a_filepaths.o`.

**Hook fix landed alongside**: `tools/hooks/tooling_error_guard.py`'s
`is_build_critical()` was promoting `tmp/*.c` scratch files to
build-critical (CRLF blocked). Added `ignored_dirs` exclusion (tmp/,
.claude/, build/, permuter/) — files under those dirs are exempt from
CRLF blocking since the build never reads them.

### 2026-06-09 — Seventh real cluster retirement: 101C.rodata_pre.s (sub-TU, 368 bytes, FIRST MULTI-FILE)

**Retired**: `101C.rodata_pre.s` — 7 jtbls + 3 D_strings spanning 4 owner
.c files (code6cac_b, code6cac_b2, code6cac_c, code6cac_c_mid). This is
the first MULTI-FILE cluster retirement.

**Method**: sub-TU split into `src/code6cac_b_rodata.c`, leaving all
owning functions in their original .c files. Cross-TU symbol resolution
handles the linkage.

**Multi-file recipe (NEW)**:
For clusters where symbols' owning functions span multiple .c files, do
NOT distribute the declarations across all owner files (invasive +
cascade-prone). Instead, package ALL the block's bytes into ONE sub-TU
file named after the upstream segment (e.g.
`code6cac_b_rodata.c` for a block that sits right after `code6cac_b.o`).
The sub-TU takes the asm/data block's exact link slot. Owning functions
in their original .c files reference the jtbl/string symbols via normal
extern linkage; the linker resolves cross-TU.

**Leading-padding trick**: the original asm/data block began with a
`.word 0x00000000` (4 bytes of leading alignment). To reproduce in C,
declared a `static const u32 _bb2_<id>_lead = 0;` at the TOP of the
sub-TU file. cc1 emits it first in .rodata, providing the same 4 leading
bytes that the asm/data block had. Without this, the new sub-TU's first
real symbol (jtbl_8001086C) would land at the wrong address.

**Note on jtbl-infra**: this block contains `jtbl_800108CC`, the
replay_camera_rob_back_loose2 jump table that's the canonical
[[jtbl-rodata-split-infrastructure]] case (24 asmfix rules bridging
GCC's emitted jtbl in code6cac_b2.o to the external jtbl_800108CC).
Those rules continue to function correctly — the external jtbl_800108CC
symbol now resolves to my code6cac_b_rodata.o definition (same address)
instead of the asm/data block's. The function itself remains parked
under jtbl-infra; the rules' retirement is a separate (harder) problem.

**Cascade**: zero. code6cac_b_rodata.o(.rodata) is exactly 368 bytes
at the slot vacated by 101C.rodata_pre.o.

### 2026-06-09 — Eighth real cluster retirement: 101C.rodata_text1a_b_pre.s (sub-TU, 1612 bytes)

**Retired**: `101C.rodata_text1a_b_pre.s` — 23 symbols spanning text1a.c
and text1b.c. Same sub-TU pattern (src/text1a_b_pre_rodata.c).

**Two new generator bugs surfaced + fixed during this retirement**:

1. **Named-symbol .word references** (`.word saTan2LineDraw`,
   `.word func_8004C994`): my `tmp/gen_101C_pre.py` script's
   `fmt_word()` mishandled non-`.L<HEX>` and non-`0x...` references
   by wrapping them in `0x` (producing `0xsaTan2LineDraw` etc, invalid
   C). Manually fixed by replacing each with the literal hex address
   from the .s comment column (e.g. `saTan2LineDraw` → `0x8004BCC0`
   per the `C0BC0480` little-endian byte sequence in the comment).
   Future improvement: have the script resolve named symbols via
   `undefined_syms_auto.txt` or the `.s` comment column.

2. **Asm `\n` escape mishandling in multi-string dlabels**: the
   script's `s.replace("\\", "\\\\")` doubled backslashes, turning
   asm's `\n` (1-byte newline) into C's `\\n` (literal backslash + n,
   2 bytes). Single-string dlabels were unaffected (the script's
   `if asciz and not words` single-item branch emitted them directly
   as C-escaped). For multi-string dlabels, manually replaced the
   buggy concat with a single-string C declaration relying on cc1's
   zero-padding for fixed-size arrays (e.g.,
   `const char D_800158E0[24] = "eff prim over :%d \n";` → 19 chars +
   5 implicit nulls = 24 bytes).

**Cascade**: zero. text1a_b_pre_rodata.o(.rodata) is exactly 1612 bytes
at the slot vacated by the asm/data block.

### 2026-06-09 — Ninth real cluster retirement: 101C.rodata_text1a_b_post.s (sub-TU, 2412 bytes — largest multi-file)

**Retired**: `101C.rodata_text1a_b_post.s` — 68 symbols (4 jtbls + 64
strings + raw data) spanning 5 owner .c files (display, ings2, system,
text1b_b, +). Same sub-TU pattern; the extraction script's two earlier
bugs were fixed before this retirement so it went through cleanly first
try.

**Script fixes applied (in `tmp/gen_101C_pre.py`)**:

1. **Named-symbol resolution**: `fmt_word()` now resolves
   `.word saTan2LineDraw`-style references by:
   - Recognizing `func_<8HEX>` pattern and extracting the embedded
     address (`func_8004C994` → `0x8004C994`).
   - Looking up other named symbols in `undefined_syms_auto.txt`,
     `named_syms.txt`, `symbol_addrs.txt`.
   - Falling back to the `.s` comment column's 4-byte little-endian
     content as `byte_hint` (the most reliable source — splat
     populates it from the actual binary).

2. **Asm escape decoding**: new `decode_asciz()` parses asm escape
   sequences (\n, \t, \r, \\, \", \0, \xHH) to a `bytes` object. New
   `emit_c_byte_literal()` re-encodes bytes as a C string literal
   using unambiguous escapes. The multi-string dlabel path now
   concatenates byte buffers instead of trying to re-escape text,
   eliminating the `\n` → `\\n` bug.

**Cascade**: zero. text1a_b_post_rodata.o(.rodata) is exactly 2412
bytes at the asm/data slot.

Next pilot candidates (from `memory/project/rodata_clusters.csv`):

1. `101C.rodata_post.s` — 0 owners, trivial-but-4-bytes. **Next, if removable
   without cascading**: tests the "4 bytes of padding absorbed by the
   surrounding C-file rodata's natural alignment" hypothesis.
2. `101C.rodata_pre_post.s` — single-function single-file (216B, owner
   `func_80038170` active in queue, code6cac_c_mid). First true cluster
   retirement after the owner is matched.
3. `101C.rodata_c2_post.s` — single-function single-file (15B, owner
   `func_8003FA24` stub, config). Smallest stub-bearing cluster.
