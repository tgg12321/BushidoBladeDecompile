# Diagnostics archive

Saved results from one-shot measurements that informed project-direction
decisions. Each file is the raw output of a diagnostic tool run at a
specific date, kept for reproducibility and as data for future agents
who might re-question the same hypothesis.

## `cc1psx_cheatasm_diagnostic_16funcs.csv` (2026-05-18)

**Question:** Would patching `decompals/mips-gcc-2.7.2` to match Sony's
proprietary `cc1psx` (the original compiler that built the BB2 binary)
retire many of the 240 cheat-asm functions, or are cheat-asm functions mostly
held back by C-source-reconstruction issues that no compiler change would
fix?

**Method:** For 16 representative cheat-asm functions sampled across all
source files, run `tools/cc1psx_diagnostic.py`:

1. Auto-extract pure-C from `src/<file>.c` via
   `tools/extract_pure_c.py` (strips `register T x asm("$N")` pins,
   `__asm__` blocks, `INLINE_MOVE_ALIASING:` comment blocks).
2. Compile the pure C through the project pipeline (cpp → cc1 →
   prologue_fix → maspsx → as) with **both**:
   - `tools/gcc-2.7.2/build/cc1` (decompals — what BB2 builds with)
   - `tools/cc1psx.exe` via `tools/cc1psx_wrapper.sh` (Sony's
     proprietary original — runs under dosemu2)
3. Compare each compiler's instruction-word output against target
   bytes extracted from `disc/SLUS_006.63` at the function's file
   offset (from the splat-produced asm/funcs/<func>.s comments).
4. Categorize:
   - **BOTH_MATCH**: pure C already matches under both compilers
     (function is misclassified cheat-asm)
   - **COMPILER_FIXABLE**: cc1psx matches, decompals doesn't (patching
     decompals would help)
   - **C_SOURCE_ISSUE**: neither compiler matches (C reconstruction
     is wrong; no compiler can match this)
   - **DECOMPALS_ONLY**: decompals matches but cc1psx doesn't (rare;
     would mean cc1psx itself has a bug)

**Result:**

| Category | Count | % |
|---|---:|---:|
| COMPILER_FIXABLE | **0** | **0%** |
| C_SOURCE_ISSUE | 15 | 94% |
| BOTH_MATCH | 1 | 6% |
| DECOMPALS_ONLY | 0 | 0% |

**Interpretation:** Compiler-patching would not retire any of the 16
sampled cheat-asm functions. The gap is C-source reconstruction, not
compiler divergence. Even with Sony's actual original compiler, the
pure-C reconstructions fail.

This disconfirms the hypothesis that motivated investigating compiler
patches in the first place. Future agents proposing the compiler-patch
route should review this data first.

**Caveats:**

- 16 functions is a sample, not the full 240-function population.
  Sampling was broad (all source files, sizes 30-211 insns) but not
  random. A larger sample could find a COMPILER_FIXABLE function we
  missed.
- 5 of the 15 C_SOURCE_ISSUE cases produced `compile_failed` because
  the auto-extractor stripped inline asm that turned out to be
  load-bearing (computing values, not just hinting register
  allocation). Those cases say "this isn't a simple-strip case" rather
  than a clean cc1psx-vs-decompals comparison. To rule out
  COMPILER_FIXABLE definitively for them, you'd construct base.c
  manually.
- The 1 BOTH_MATCH case (`func_8003FE40`) is a misclassification by
  `tools/classify_inline_asm.py` — the register-asm pin it counts as
  cheat-asm isn't load-bearing for matching. Trivial cleanup.

**Reproducibility:**

```bash
# Re-run the same 16 functions:
bash tools/diagnose_batch.sh \
    func_800483DC func_8008BC60 ang_near_dif func_80016E60 \
    func_80033DF4 func_8003D39C single_game_VoiceContorol func_80083A48 \
    func_8004C388 func_80047A90 func_80034200 func_80037B00 \
    func_8007352C func_8003FE40 func_80089EB0 func_80070C70

# Or expand the sample:
bash tools/diagnose_batch.sh <more funcs>
```

**Related:**

- Commit `32b2da9` — initial diagnostic batch
- `memory/rules/compiler-patch-low-roi.md` — rule derived from this data
- `memory/rules/cc1psx-calibration-only.md` — prior project-level
  decision not to switch the build to cc1psx
- `.claude/rules/inline-asm-policy.md` — what cheat-asm means + how it's
  surfaced as the SOTN-bar gap metric
- `.claude/rules/attempts-log-gate.md` — the only enforcement
  mechanism for the only path with measured non-zero ROI for
  cheat-asm retirement: per-function pure-C retry
