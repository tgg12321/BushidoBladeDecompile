# The oracle compiler — identity, derivation, and reproduction

**Status: REPRODUCIBLE, no codegen patch.** Since owner ruling 2026-09-26 (Q17,
`.claude/rules/no-compiler-divergence.md` § "Owner ruling 2026-09-26 (ninth batch,
Q17)") **a compiler patch is a cheat**: the build compiler is the pinned upstream
plus one host-build crash fix that does not change output.

```
upstream : decompals/mips-gcc-2.7.2 @ 43d1cdb67ed135879869b5266f01efaaada5e35a
crashfix : tools/cc1-reorg-negate-rtx-decl.patch  (reorg.c: the 2026-08-24
           host-ABI negate_rtx declaration, owner ruling 262930f1b; +9)
recipe   : every object at -O0 (`-g`), combine.o ALONE at `-O`,
           `-fgnu89-inline` throughout
build it : bash tools/build_oracle_cc1.sh              # verify in scratch
           bash tools/build_oracle_cc1.sh --install    # then swap it in
```

`tools/gcc-2.7.2/build/cc1` (Makefile:12) is the operative binary and is exactly
what that script produces. `tools/gcc-2.7.2/` is gitignored, which is why the
patch is tracked in `tools/`. The script copies the tree to scratch, restores it
to the pinned commit, applies the patch there, and never modifies the live tree.

The binary's own SHA1 is **not** the invariant (host-toolchain metadata leaks
into it). The invariant is the assembly emitted for the project's TUs; the
script's self-check asserts it. `engine verify-oracle --rebuild` against
`62efab4f73f992798c43e8c730aa43baa10bb4fa` remains the authoritative gate.

| binary | SHA1 | what it is |
|---|---|---|
| operative `build/cc1` | `ac80146bf50af58f719868de74c860fcdf617d30` | recipe output: pinned upstream + crash fix only (2026-09-26, Q17) |
| `build/cc1.PRE-RECIPE-aa04d761` | `aa04d7619cd79215788d18d6870ebc6c8d1d822d` | retired NARROW PLUS->IOR compiler, operative 2026-09-25 to 2026-09-26; the self-check's first reference |
| `build/cc1.PRE-RECIPE-0f438e42` | `0f438e42548d29798db86d50a76e54bd6f04b64a` | retired no-rewrite compiler with the crash fix (2026-08-24 to 2026-09-25); the self-check's no-rewrite reference |
| `build/cc1.PRE-CRASHFIX-045c9543` | `ea11be50d12f24c464123b705c29007efc04e4a8` | 2026-08-07 recipe output (suffix is wrong; kept because `oracle/manifest.json` `notes.cc1_build_2026_08_24` cites it) |
| `build/cc1.PRE-RECIPE-045c9543`, `build/cc1.ORACLE-BACKUP`, off-tree `C:\Users\Trenton\bb2-oracle-cc1-backup\cc1.oracle-compiler-045c9543` | `045c9543d39ab8109583b92137c7adde084f7a25` | the historical 2026-05-18 binary; segfaults on 5 current TUs (no crash fix) |
| diagnostic `cc1` | `3f796363f33649712006f2eefb47c2f8c4539ffc` | `BB2_*_DEBUG` hooks, no codegen patch (2026-09-26, Q17) |
| `cc1.PRE-PATCH-888dda67` / `-4096c6fd` / `-8384fd47` | as named | retired diagnostics (narrow patch / pre-crash-fix / unpatched) |

**Verify the backups periodically.** The off-tree backup has been found missing
twice after being hash-verified; re-check it with `sha1sum`, and keep the
in-tree copies as the working redundancy.

## OPEN QUESTION — did the original compiler perform the PLUS->IOR rewrite?

Stock GCC 2.7.2 rewrites `a + b` into `a | b` whenever it can prove the operands
share no bits. **Undetermined** whether the shipped compiler did; the project does
not assert an answer, and since Q17 no compiler change may be made to fit either
reading — a function whose bytes need a different compiler behaviour stays
INCOMPLETE until an honest spelling is found.

Evidence it did **not** (fully): at the real site A in `gnd_disp_loop_ctrl`
(`ings`) the original kept `andi ; addiu`, and 21 C spellings all emit `ori` under
stock GCC and under PsyQ's `cc1psx` on the actual TU. Evidence it **did**: site C
(`main` in `ings`) matches stock's codegen from its natural spelling, and
`func_80073C78`'s UV stores contain the target `ori` pair that stock GCC produces
from the natural `u + du0` body. `sprintf`'s `andi 7 ; addiu 0x30` is neutral (the
rewrite fires there and is undone under every compiler). The 571 `sll;addu` vs 0
`sll;or` multiply idiom is weak evidence (most operands are not provably disjoint).

**The 2026-09-25 study** (scratch-only, formerly `pre-slim-2026-10-01:docs/ORACLE-COMPILER-STUDY-2026-09-25.md`,
`ac5d2b0bb`) found that one condition explains every compilable site — rewrite
except for a plain `(plus REG CONST_INT)` ("narrow") — and that a second condition
(`exprop`: rewrite only when an operand is neither REG nor CONST_INT, or a REG
operand is known zero) fits too; they differ only on register+register sums whose
single use is a return value, call argument or copy. GCC 2.5.8/2.7.2/2.8.1 all
rewrite unconditionally, and GCC 3.x's narrowing fails at func_80073C78 and site
C, so neither is a recovered historical compiler. `cc1psx` rewrites at site A,
which the original did not, so the game was not built with `tools/cc1psx.exe`'s
behaviour there (or site A's C is still wrong). The study also found that `main`'s
granted FAKE same-pseudo chain (`pre-slim-2026-10-01:.claude/rules/chained-accumulation-fake-exception.md`)
was needed only because of the then no-rewrite patch (study § 6.2). Register+register
disjoint-sum site list: `python3 tools/rrscan_plus_ior.py <out.tsv>` (547 sites
at 2026-09-25).

### Owner ruling 2026-09-25 — keep the patch; a narrower patch may be STUDIED, not adopted

Superseded by Q17. It kept the then no-rewrite patch, refused the `|`-for-`+`
spelling (func_80073C78 layer-2 FAIL, `rejected/ior-spelling-patch-workaround.c`
— still refused), and authorized the scratch-only study above. Full text:
`pre-slim-2026-10-01:docs/ORACLE-COMPILER.md` (record: decisions.md 2026-09-25
OWNER RULING — oracle compiler).

### Owner ruling 2026-09-25 (second batch) — adopt the narrowed condition, after a register-plus-register scan

Superseded by Q17. It adopted "narrow" after `tools/rrscan_plus_ior.py` found no
site discriminating between narrow and `exprop`, with `exprop` kept as a live
alternative. Full text at the tag above.

### Adoption record (2026-09-25, owner ruling `bcdc1648e`)

Executed in `9bc64b751`: `tools/cc1-plus-to-ior-narrow.patch` replaced
`tools/cc1-no-plus-to-ior.patch`, the crash fix became the tracked recipe input
`tools/cc1-reorg-negate-rtx-decl.patch`, `build/cc1` became `aa04d761…`
(`simplify_rtx` 0x3a6a); oracle SHA1, engine test 720/720, fixtures 5/5. Reverted
by Q17 the next day (func_800174F4, the one function depending on it, went back
to `INCLUDE_ASM`). Full per-site record at the tag above.

## The diagnostic compiler

`tools/gcc-2.7.2/cc1` is the same GCC carrying the `BB2_*_DEBUG` hooks that
`ra_solver` / `sched_solver` read their dumps from, built from the hooked live
sources with no codegen patch, so it agrees with the oracle on every TU. The live
`reorg.c` already carries the crash-fix declaration. Rebuild with
`bash tools/build_diagnostic_cc1.sh [--install]`. Hook inertness holds: a cc1
built from pristine reverted sources is behaviourally identical to the
instrumented one on every TU (`cc1_hooks.patch.md` has the details and the
two-behavioural-knob declaration rule); the oracle build reverts the hooks anyway.

## Tracked state manifest of the gitignored compiler tree

Edits inside `tools/gcc-2.7.2/` leave no trace in git — that is how the original
`combine.c` patch went unnoticed for three months. `tools/build_oracle_cc1.sh`
and `tools/build_diagnostic_cc1.sh` carry this manifest and **refuse to run** if
the sources drift. Verify with `sha1sum` from `tools/gcc-2.7.2/`; any drift must
be re-recorded here, with rationale, in the same change.

```
80c8088750a91b37ef53bea6da51d402c58801c8  flow.c            (BB2 debug hooks)
6f23c5eeeabc97f3049b2d414ab42b6f38b891f6  function.c        (BB2 debug hooks)
46af0a314da8ad6dcb27890ca9b2003006c70ced  global.c          (BB2 debug hooks)
9b8f822a79a1945ac4b58ebfb243017d67b828c5  jump.c            (BB2 debug hooks)
3fb248a6b2c85cd7e9e57b19f5df7daf62b3d5fe  local-alloc.c     (BB2 debug hooks incl. SUGG)
7ddde6b0f2b65172c5cc83be6165789f445953d9  reload1.c         (BB2 debug hooks)
6e1cf6a97c169204304efd7427e066facb49d69f  reorg.c           (BB2 hooks + 2 BEHAVIORAL knobs, inert unless set, + the 2026-08-24 crash-fix block)
3668555e9cb7970b335a505aca4cfdda26e8fc49  sched.c           (BB2 debug hooks)
24c5952113d88cbb96f5c9f7e7152147d1efb8a7  combine.c         (pristine)
3f796363f33649712006f2eefb47c2f8c4539ffc  cc1               (instrumented DIAGNOSTIC binary, no codegen patch)
ac80146bf50af58f719868de74c860fcdf617d30  build/cc1         (THE ORACLE COMPILER — recipe output)
```

Binary lines re-recorded 2026-10-01 from the live tree (sources unchanged since the
2026-09-25 re-record, when `reorg.c` moved `73a15a52…` → `6e1cf6a9…` by exactly the
crash-fix block). Stale `.bb2bak` leftovers are the untracked-edit pattern this
manifest exists to catch — clean them, don't create more.

## History

- **2026-05-18** — `build/cc1` built during a compiler-patch experiment (the
  `combine.c` PLUS->IOR removal) and silently adopted as the project compiler.
- **2026-08-07** — reproducibility investigation + forensics (formerly
  `pre-slim-2026-10-01:docs/grind/cc1-forensics-2026-08-07.md`) identified it as stock GCC 2.7.2 minus
  PLUS->IOR. Owner first elected stock, then (`d99ab6a6`, on Phase-0 evidence) kept
  the patch with the upstream pinned and the recipe scripted (SOTN pattern).
- **2026-08-24** — host-ABI `negate_rtx` crash fix (owner ruling `262930f1b`,
  adopted `fea9fa2ac`).
- **2026-09-25** — the study above; narrow condition adopted (`bcdc1648e`, `9bc64b751`).
- **2026-09-26 (Q17)** — a compiler patch is a cheat: build compiler returned to
  pinned upstream + crash fix; func_800174F4 re-queued.
