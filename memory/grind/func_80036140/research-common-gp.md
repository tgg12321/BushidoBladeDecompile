# func_80036140 — research: the maspsx COMMON / `sym+k` gp question (2026-09-26, research-only worker)

Scope: evidence for the owner's open item (a) in docs/grind/borderline.md 2026-09-26 func_80036140
("tell maspsx that g_cd_atv / D_800A36B8 were COMMON in the original TU"), plus an honest check of
the other landing requirements. Nothing in src/, include/, the Makefile, the pipeline lists or
tools/maspsx/ was touched. Tools and raw outputs: `research-common-gp/` (copy it to
`tmp/research36140/` to rerun; the ASPSX binary comes from
`https://github.com/mkst/esa/releases/download/psyq-binaries/psyq3.5.tar.gz`, untarred into
`tmp/research36140/psyq/`, the same archive upstream maspsx's `aspsx/download.sh` uses).

## 0. The new fact: the ORIGINAL assembler was run

The earlier claim ("ASPSX never gp'd an offset into a COMMON symbol") rested on a comment in
maspsx's code. This session ran the real **Psy-Q ASPSX 2.34** (psyq3.5, it prints "Psy-Q ASPSX
version 2.34") under dosemu2 (`aspsx.sh`), on hand-written probes (`run_asm_tests.py`,
`asm_probe_results.txt`) and on the real **cc1psx** output (`pipe.sh` = cc1psx wrapper then ASPSX).
With `-G8`, the assembler picks gp addressing as follows:

| how the TU declares the symbol | base access `sym` | offset access `sym+k` |
|---|---|---|
| `.comm` (tentative definition `T x;`) declared before use | **gp** | **NOT gp** (lui/%lo) |
| `.lcomm` (`static T x;`) declared before use | gp | gp |
| defined in `.sdata` (initialized) | gp | gp |
| `.extern sym, size` (`extern T x;`), exactly as cc1psx emits it | **NOT gp** | NOT gp |
| `.comm` / `.lcomm` declared AFTER the instruction | NOT gp | NOT gp |

The `.comm`/`.lcomm`/`.sdata` rows agree with upstream maspsx's own ASPSX regression test
(`tools/maspsx/aspsx/test_gp_offset.py`: 2.34 = `GP_OFFSET_TEST_RESULT_NO_GP_COMM`). The
`.extern` row and the declared-after-use row are new. Two consequences:

1. **In the shipped binary, every gp access is to a symbol the accessing file defined itself**
   (tentative, static or initialized). A variable only declared `extern` is never gp-relative.
   The project's `sdata_syms`/`sdata_funcs`/`sdata_exclude` lists stand in for that per-file
   knowledge, because our C declares everything `extern`.
2. **Any gp access in the original bytes means that file went through cc1 with -G8.** At -G0,
   cc1psx writes the `.comm`/`.lcomm` lines at the END of the file (seen in
   `c/atv_comm.c` at -G0: the declarations come after the code). ASPSX decides one instruction at
   a time, so a declaration that comes later gets no gp. At -G8, cc1psx holds the function bodies
   back until after the declarations (the same body buffering the ledger's fact 4 saw in our cc1).
   This confirms the premise of prong (i) of the -G8 ruling, using the original tools.

## 1. The census

`census.py` walks every `asm/funcs/*.s` (the split shipped EXE), resolving symbols through
`build/bb2.elf` + the symbol files. It records every load/store/`la` into the gp window. Totals:
5,507 accesses; 2,271 gp-relative, in 325 functions. The gp-accessed range is 0x800A30DC..0x800A3928.
Initialized small data ends at 0x800A3308: `__SN_ENTRY_POINT` clears 0x800A3308..0x801078E0.
The census tells direct `lui`+`%lo` accesses apart from indexed ones (`lui; addu idx; %lo(sym)(reg)`),
which were a false-positive source.

**COMMON-offset signature** (in one function: base gp, `base+k` direct lui): 26 sites, 17
addresses, 7 functions (`census.txt`, rows `OFFSET-SIG`):
- the two ATV records: 0x800A36B9..BB (func_80035F78, func_80036140) and 0x800A3719..1B
  (cdrom_SetMix, func_80036140)
- g_cd_result+4 (0x800A3764, func_80036140)
- comb_WaitRead8 / func_8003A728: 0x800A368C, 369C, 36C2, 36C4, 36D2, 36D4
- func_80044504: 0x800A370C
- func_80016D78: 0x800A3745, 3746, 3774

**None of those 17 offset addresses is gp-accessed by any function anywhere in the binary.**
Every function listed except func_80036140 is already COMPLETED-C, using split per-byte externs
that are not in `sdata_syms.txt`. They need no gate.

**Does "COMMON + offset ⇒ no gp" hold with zero exceptions? Yes.** It is the assembler's own rule,
reproduced on the real binary, and the census finds no contradiction.

**The "9 matched sites" are NOT counterexamples.** They are consistent with the rule. They are the
9 `sym+k` operands in our compiled C that maspsx gp's today (`gp_off_census.py`, rerun this
session: TOTAL 9):
- `g_anim_select+2/+4` (func_80041E10, func_80041EB0), 4 sites. The symbol is at 0x800A3238,
  below 0x800A3308, so it is initialized small data in the image. An initialized object defined in
  that file gets gp at any offset. All accessors are in text1a_post.
- `D_800A34F0+2` (func_80061C00), 1 site. D_800A34F0 is uninitialized, and no other function in
  the binary touches it. The consistent readings are a `static` array (`.lcomm` gets gp at offset)
  or two separate variables.
- `D_800A3588+2`, `D_800A358C+2` (func_8006E534), 4 sites. Uninitialized. Every accessor
  (8006E534, 8006ECF4, 8006F97C, 80070188) is in text1b, which fits a `static` array.

So a GLOBAL "never gp an offset into an extern" rule would be *wrong*, not just harmful. It would
treat these initialized or static objects as COMMON. The faithful model needs a per-symbol storage
class, and that is what a gate supplies.

## 2. Proposal: a gated COMMON list (fidelity class, precedent = the prefill-label gate)

- **Gate file:** `maspsx_comm_syms.txt`, in the `sdata_exclude.txt` format, keyed by function:
  `func: sym, sym`. Meaning: "in this function's original file, these symbols were COMMON
  (tentative definitions)".
- **Flag:** `--comm-syms=maspsx_comm_syms.txt` on both MASPSX_FLAGS and MASPSX_FLAGS_GP, mirrored in
  `engine/buildconfig.py`.
- **The exact maspsx rule** (`maspsx_comm_gate.diff`, about 20 lines): the three existing
  `gp_allowed = self.gp_allow_offset or symbol not in self.comm_symbols` sites become
  `... or not self._is_comm(symbol)`. `_is_comm` returns true when the symbol came from a `.comm`
  line (upstream behaviour, unchanged) or is listed for `self.current_func`. The gate only ever
  REMOVES gp, and only from `sym+N` operands of listed symbols inside listed functions. It emits no
  storage, adds or removes no instruction, and never touches the base access.
- **Entries:** `cdrom_SetMix`, `func_80035F78`, `func_80036140` : `g_cd_atv, D_800A36B8`
  (`maspsx_comm_syms.txt`). The three functions come from one original file.
- **Proof it is inert on today's tree:** `bothways.sh` compiles every `src/*.c` with the Makefile's
  exact per-file recipe (taken from `make -n -B`), once with the stock maspsx and once with the gated
  copy plus these entries. Result: **34/34 objects byte-identical**, 0 failed. The `.s` files do
  not go through maspsx, so the link is identical too. Caveat: this was measured on the shared
  working tree with other workers' staged edits in it, so it compares both ways on the same inputs.
  A landing still needs `verify-oracle --rebuild`.
- **Proof it does its job:** `ctl.sh` is the positive control (stock maspsx `%gp_rel(g_cd_atv+1)`
  vs gated `sb $5,g_cd_atv+1`, which the assembler expands to `lui $at` + `%lo`, the target shape).
  `xb2.py` rebuilds the prior worker's full model (`integration/*.patch`, -G8) with the gate in
  place of the earlier `.comm` injection, which also emitted storage:
  **func_80036140 2 (the jtbl operand only), the other 32 functions in the file all 0.** Without
  the gate: 14, and 6/6 for cdrom_SetMix / func_80035F78.
- **Adding entries later:** tag `[infra-rule: maspsx-comm]`. Each new entry needs two pieces of
  evidence: (1) the target function shows base gp plus a direct non-gp `base+k` for the symbol, and
  (2) the original toolchain (cc1psx + ASPSX 2.34, `pipe.sh` + `cmp.py`) reproduces that from a
  tentative definition. Register it as a gate list is registered: engine/cheats.py
  MASPSX_GATE_LISTS "fidelity", PIPELINE_DEPS, the oracle watch, the dossier, the root allowlist
  and a row in the maspsx-gate-lists rule.
- **Rejected alternative:** real tentative definitions in C (`CdlATV g_cd_atv;`). Our cc1 writes a
  3-argument `.comm x,4,1` that maspsx can't parse, and it would emit storage that collides with the
  linker-script address. Faithful in spirit, but a larger build change for the same bytes.

**The original toolchain reproduces the target from this model** (`orig_toolchain_results.txt`,
relocation immediates masked):

| probe (cc1psx -G8 → ASPSX 2.34) | cdrom_SetMix (18 words) | func_80035F78 (12 words) |
|---|---|---|
| `CdlATV` tentative definition (COMMON) | **0 differ** | **0 differ** |
| `extern CdlATV` | 19 differ | 12 differ |
| `static CdlATV` | 13 differ | n/a |
| four separate `u8` tentative definitions | 13 differ | n/a |
| COMMON, but cc1psx at -G0 | 22 differ | 14 differ |

The struct copy `g_cd_atv = D_800A36B8;` compiles through the original toolchain to exactly the
target's `la a1; la a0; lwl/lwr 3/0(a1); nop; swl/swr 3/0(a0)` (`c/copy_comm.c`).

## 3. The other requirements, assessed

- **CdlATV merge, prong (a): strong independent evidence, not compiler necessity.** Three pieces:
  (1) CdlATV is the PsyQ libcd type, and CdMix takes `CdlATV *`. For g_cd_atv, the committed naming
  census already describes a 4-byte CdlATV block (apiscan wave 9a7da73f5, 2026-09-07, plus the
  2026-09-24 data manifest). That is a "committed naming-census schema" that predates this work.
  (2) Base+offset addressing in the original binary: under the real ASPSX, "byte 0 gp, bytes 1-3
  not" can only come from ONE COMMON object accessed at +1..+3. Separate variables give all-gp or
  no-gp, as the table shows. The only other way is a contrived mix of `u8 a;` with
  `extern u8 b, c, d;`. (3) The unaligned 4-byte copy is a struct assignment. D_800A36B8 has
  (2) and (3) but no naming-census row. The merge still depends on the COMMON gate: the
  aggregate-merge amendment says a merge that depends on it is not admitted until the owner rules.
- **ReplayCamRec extension over 0x80101E9A..EA7:** the only evidence is this function's
  member-form codegen (`la` + `0(reg)`). No base+offset formation crosses those labels
  (hypotheses.md, open item 2). That is the compiler-necessity class, so it needs the full (a1),
  (a2) and (a4′)(1)-(5) package for a mixed-field struct. What is still missing: banked RTL dumps
  naming the pass and source location, a **cc1psx split-vs-merged run of the same TU (a2)**, and
  the member-by-member table (a4′)(4). It can be done with this toolchain; it is not done yet.
- **func_80036940 and the -G8 ruling: a real conflict.** The original bytes put gp accesses in
  func_80035828, cdrom_SetMix, func_80035F78, cdrom_Init, func_80036140 and **func_80036940**
  (g_cd_result, 0x800A3760) (`gpfuncs.py`). By §0.2, func_80036940 was compiled -G8 too, in the same
  original file. Ruling (i) requires that no outside neighbour gp-access a symbol in the function's
  set. func_80036940 shares g_cd_result with func_80036140, so a -G8 file holding func_80036140
  without func_80036940 fails (i). Putting func_80036940 inside fails (iii), because it is still
  INCLUDE_ASM (rotated, distance 274, no ledger). Linking it as asm (LINKED_ASM_FUNCS) is barred by
  (v). The ruling also needs every function in the -G8 file to have gp accesses of its own. The
  gp-less snd_SerialMixOn, cdrom_FlushInit and cdrom_ReadyCallback sit between cdrom_SetMix and
  func_80036140, so the one-file layout would have to become several small -G8/-G0 files. Under
  the current rules, func_80036140 cannot land before func_80036940 reaches COMPLETED-C.
- **Jump table at 0x80010938:** mechanical once the file split exists. The new file's .rodata
  starts right after func_80035828's tables, which end at 0x80010938, a multiple of 8. By (iv-a),
  that file stays off RODATA_ALIGN2_FILES. `jtbl_80010938` is removed from
  code6cac_b_rodata_post.c, which keeps its leading zero word until func_80036940 lands.

## 4. Bottom line for the owner (plain language)

The original assembler (Sony's ASPSX 2.34) was run directly, so this is no longer an inference.
It treats a variable declared in a file without `extern` and without a value (the "COMMON" kind)
like this: the first byte uses the fast global-pointer shortcut, but "variable + 1, 2, 3" never
does. Variables that are static or given an initial value get the shortcut for every byte. Our
assembler shim can't tell those kinds apart, so it gives the shortcut to every byte. A small
opt-in list (3 functions × 2 variables) fixes that for exactly these cases. Measured: every other
source file builds byte-identical with the list in place.

Decisions needed:
1. "Add a per-function list that tells the assembler shim which variables were COMMON in the
   original file, proven by the original assembler reproducing the shipped bytes?" (recommended)
2. "func_80036140 and its neighbour func_80036940 share a variable the original compiled in -G8
   mode. Should we (a) wait until func_80036940 is decompiled and move both into one -G8 file
   (recommended, needs no new exception), or (b) allow the -G8 file to hold gp-less functions
   whose bytes are identical at -G0 and -G8?"

Confidence: **high** on the assembler rule and the census (the real binary was run; 0 exceptions
in 2,271 gp accesses), on byte-neutrality (34/34 objects), and on the CdlATV model (exact
reproduction by the original toolchain). **Medium** that func_80036140 lands soon: the record
extension still needs its (a2)/(a4′) package, and func_80036940 must land first.
