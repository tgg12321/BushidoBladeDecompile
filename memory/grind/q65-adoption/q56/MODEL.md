# Q56 follow-up (A): a per-file gp model instead of sdata_syms / sdata_funcs / sdata_exclude — proof of concept

Scratch only: `/tmp/q56/model` (a `git archive` copy of `a739dbd20`; toolchain symlinked). No worktree, no
tracked main-tree edit, no commit. Every change the POC makes is in `tmp/q56/model.diff`.

## Result

**The per-file model reproduces the oracle.** A clean full build of the POC tree has exe SHA1
`62efab4f73f992798c43e8c730aa43baa10bb4fa`, with `--sdata-syms`, `--sdata-funcs` and `--sdata-exclude`
removed from the build: no list of symbols, functions or per-function exceptions. The model is Sony
ASPSX's own rule: a file uses gp for a small symbol only if the file defines it. Every gp decision it
makes was checked against the real ASPSX 2.34 (below). It needs:

1. **One maspsx change**, a fidelity bug fix that ASPSX verifies and that is byte-neutral on today's build (§3).
2. **maspsx's existing upstream `-G8` switch** on both flag lines. With it, gp is decided by the file's own
   `.comm` / `.lcomm` / `.sdata` definitions of 8 bytes or less. No new model code is needed.
3. **Definitions in the C files**: 323 declarations in 28 files (301 distinct symbols). §2 says which
   kinds are final C and which are POC stand-ins.
4. **Two file splits** where one of our files is really two original files (§4). These are the only
   contradictions. The oracle gp choices of 2 accesses in two functions (`func_80044800`,
   `func_800343F0`) cannot come from any single-file assembly.

## 1. Sony ASPSX evidence (Psy-Q ASPSX 2.34 from the psyq3.5 archive, run under dosemu2 with `tmp/research36140/aspsx.sh`)

**Rule probes** (`tmp/q56/aspsx_probes.py` → `tmp/q56/aspsx_probe_results.txt`), all at `-G8`:

| probe | ASPSX output | meaning |
|---|---|---|
| `.comm cv,4`; `lb cv`; `lb cv+1` | gp, lui | tentative def: gp at base only |
| `.lcomm lv,4` | gp, gp | static: gp at every offset |
| `.sdata sv: .word 0` | gp, gp | initialized: gp at every offset |
| `.extern ev,4` / undeclared | lui, lui | extern: never gp |
| two `.ent` functions, `.comm s2,4` | gp in both | **the decision is per file** |
| `.comm big,12` / `.comm eight,8` | lui / gp | -G8 threshold (≤ 8 bytes) |
| `.comm ix,4`; `lbu $2,0($4)`; `sb $2,ix($3)` | lbu, lui, addu, sb: **no nop** | indexed store: $at expansion, not gp |
| same with `sb $2,dx` (direct) | lbu, nop, sb(gp) | direct store: gp plus the load-delay nop |

**Whole program** (`tmp/q56/aspsx_wholeprog.py` → `aspsx_wholeprog.txt`). For each of the 41 C file
parts of the POC, the probe holds:
- every macro load/store with a symbol operand that cc1 emits (**6,337 accesses**);
- the file's own definitions, placed first as cc1psx -G8 places them, with the initialized/static
  stand-ins given as `.sdata` objects.

Each probe was assembled by ASPSX and by the POC maspsx. **ASPSX made the same gp decision as the POC
on all 6,337 (2,095 gp), with 0 disagreements in 41/41 files.** The POC maspsx output is what builds to
the oracle.

**Negative controls** (`aspsx_negctl.py` → `aspsx_negctl.txt`):
- With the definitions removed (every symbol `extern`, as our C declares today), ASPSX gives gp to **0 of
  6,337**. The definitions are what drive the gp choice.
- `text1a_c` unsplit: ASPSX gives gp to all 3 `D_800A3820` accesses. The shipped `func_80044800` has 2 of
  them non-gp.
- `code6cac_b_tu2` unsplit: ASPSX gives gp to `func_800343F0`'s `sw D_800A3140`, which the shipped code
  has non-gp.

## 2. What the declarations are (323 in 28 files)

The rule used: for every symbol that an oracle object reaches gp-relative, that file gets a definition if
it has none. The POC spells it `__typeof__(S) S;`, a tentative definition of the symbol's own declared
type. GNU ld resolves it to the symbol-file address without allocating it (owner ruling Q62). Seven
symbols the C declares only at block scope get `T S;` built from that `extern` line instead.

| kind (by address and use) | declarations | status in the POC |
|---|---|---|
| bss (≥ 0x800A3308), base access only | 266 | **final form**: the ordinary tentative definition Q62 admits (zero bytes, symbol-file address) |
| initialized small-data region (< 0x800A3308), original bytes zero | 26 | stand-in. The original was an initialized definition in `.sdata` (the region lies inside the image, before bss) |
| initialized small-data region, original bytes nonzero | 23 | stand-in. Q62 refuses a tentative definition of a nonzero object; the real form is `T S = value;` in the file |
| initialized, reached gp at an offset (`D_800A3174`, `g_anim_select`) | 2 | stand-in through `--poc-noncomm-syms`; real form is the initialized `.sdata` definition |
| bss reached gp at an offset (`D_800A34F0`; `D_800A3560`/`3588`/`358C`/`3590`/`35C8`) | 6 | stand-in through `--poc-noncomm-syms`; the original was `static` (`.lcomm`, gp at every offset) |

Without the `--poc-noncomm-syms` stand-in, 59 functions differ, and every difference is lost gp on those
8 symbols' offset accesses (verified by a build). So the offset rule is real, and those objects cannot be
tentative definitions.

**Mapping to original files.** The initialized objects must each be defined in exactly one file. The data
supports that:
- 47 of the 50 initialized-region symbols are reached from one object only.
- The addresses of all 50 rise in link order of the files that gp them. `ings` 0x800A30DC →
  `code6cac_tu2` 0x800A30EC → … → `text1b_b` 0x800A3304. That is what PSYLINK concatenating each object's
  `.sdata` would produce.
- The other 3 are reached from **adjacent** objects that must then be one original file:
  - `D_800A31D8`/`D_800A31DA`: `code6cac_b2_pre` + `replay_camera_rob_back_loose2` + `code6cac_b2_post`
    (0x80035438–0x80035F30, contiguous).
  - `D_800A322C`: `code6cac_c2` + `config` (0x8003B9D0–0x800401CC, contiguous).
- Merging each group creates no per-file contradiction (`mergecheck.py`).

## 3. The maspsx change (the only one the model needs)

`_uses_gp`, the helper that decides whether a load needs a load-delay nop before the next instruction,
treated an INDEXED operand `sym($reg)` as gp when `sym` is small data defined in the file. The expansion
code itself never makes such an operand gp; it goes through `$at`. With the lists this was masked,
because such symbols were gp-eligible only inside `sdata_funcs`. Under the model it put a spurious nop
into `func_8003047C`.

- **Fix:** return False when the operand has a base register.
- **ASPSX:** agrees (probe `indexed_after_load_comm`: no nop).
- **Byte-neutral today:** today's list model plus only this fix builds to the oracle
  (`tmp/q56/fixonly.sh`, full clean build `62efab4f…`).

The POC also carries a POC-only hook (`--poc-noncomm-syms`, 8 symbols) that stands in for the
initialized/static definitions of §2. A real adoption would replace it with:
- the definitions themselves;
- maspsx handling `.local`+`.comm` as `.lcomm` (today it deliberately raises: "uninitialized static …
  needs its own ruling").

## 4. Contradictions: accesses no single-file assembly can produce

Found with the oracle objects (every direct lui/%lo load/store to a symbol that the same object also
reaches gp) and confirmed by the POC build diff and the ASPSX negative controls:

| symbol | gp user (address) | non-gp direct user in the same file | resolution in the POC |
|---|---|---|---|
| `D_800A3820` | `func_80044504` (text1a_c, `sw` at 0x80044634) | `func_80044800` (text1a_c, `lw`/`sw` at 0x80044AC4/0x80044AD4) | split `text1a_c` before `func_80044800`. No gp symbol is shared across the cut (`cutcheck.py`); each part defines only its own gp symbols |
| `D_800A3140` | `func_8002AB08` (code6cac_b_tu2, INCLUDE_ASM, queued) | `func_800343F0` (code6cac_b_tu2, `sw`) | split `code6cac_b_tu2` before `func_800343F0` (its last function). Clean cut |

With both splits (done in the POC on the pre-maspsx stream by `splitcc1.py`, parts linked consecutively
in `bb2.ld`, as the 2026-09-30 rodata POC did), the full build hits the oracle.

**Checked and not contradictions:**
- `g_cd_atv` / `D_800A36B8` (code6cac_b4): gp at base, lui at +1..+3. This is exactly the COMMON rule
  (Q62) with a tentative definition.
- `g_gpu_ot256_ptr` (text1b): gp in `func_80048BA4`, lui/%lo in `func_80051D08`/`func_80051ED4`. Those
  two are canonical hand-written asm (`inline_asm_canonical.txt`), consistent with explicit `%hi/%lo` in
  hand-written asm or with those functions sitting in their own file. It does not constrain any C.
- 4 symbols are reached gp only by INCLUDE_ASM/asm text in their object and are never declared by the C
  (`D_800A3230`, `D_800A3250`, `D_800A326C`, `D_800A3418`). They need no declaration until those
  functions become C.

## 5. Size of a real adoption (not done; each part is its own reviewed change)

- **maspsx:** the `_uses_gp` indexed fix (one guard), plus `.lcomm` handling for statics.
- **Makefile + `engine/buildconfig.py` mirror:** drop three flags, add `-G8`. Retire `sdata_syms.txt`
  (319), `sdata_funcs.txt` (326) and `sdata_exclude.txt` (105 rows).
- **C declarations:** 266 tentative definitions of bss objects (final form). 49 initialized definitions
  (+2 with offsets) moved out of `asm/data/91C98.data.s` into their files' `.sdata` in link order: a
  data split like the rodata cleanup, with the same kind of `bb2.ld` work. 6 `static` bss objects, which
  need bss placement at fixed addresses.
- **File boundaries:** split `text1a_c` and `code6cac_b_tu2`. The two adjacent groups above are one
  original file each: merge, or at least keep them contiguous.
- **Order caveat:** our cc1 at -G0 writes `.comm` at the end of the stream and maspsx reads the whole
  file, so it is order-blind. ASPSX is not (a definition after use gets no gp: `comm_after`). Every file
  with gp accesses was -G8 in the original (defs first), which is how the ASPSX check placed them. Making
  that literal (per-file `-G8` cc1) is the separate per-file -G8 question, not needed for bytes.
- **Governance:** a global model of documented ASPSX behaviour, like Q62 and the rodata-align adoption. It
  needs an owner ruling, a byte-neutral substrate commit, `engine test` and a fresh layer-2 review.

Scripts (all in `tmp/q56/`):
- analysis: `model_analyze.py`
- POC: `model_setup.py`, `model_build.sh`, `model_rebuild.sh`, `model_fixundecl.py`, `model_blockdecl.py`,
  `model_split.py`, `splitcc1.py`, `model_run3.sh`
- checks: `model_diff.py`, `cutcheck.py`, `mergecheck.py`, `initsyms.py`, `interleave.py`
- ASPSX: `aspsx_probes.py`, `aspsx_wholeprog.py`, `aspsx_negctl.py`, `psyqobj.py` (a complete Psy-Q LNK
  .text reader; `tools/maspsx/aspsx/util.py` stops at the first chunk)
