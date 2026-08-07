# The oracle compiler — identity, backups, and the reproducibility finding

**`tools/gcc-2.7.2/build/cc1` is the compiler the oracle match depends on.**

- SHA1: `045c9543d39ab8109583b92137c7adde084f7a25`
- Size: 4,263,208 bytes · dated 2026-05-18 (13 minutes after the compiler
  repo's single clone event)
- Gitignored (like all of `tools/gcc-2.7.2/`); until 2026-08-07 exactly ONE
  copy existed on disk.

**Backups (2026-08-07):**
- `tools/gcc-2.7.2/build/cc1.ORACLE-BACKUP` (same directory)
- `C:\Users\Trenton\bb2-oracle-cc1-backup\cc1.oracle-compiler-045c9543`
  (off-tree)
- Verify any copy with `sha1sum` against the hash above before trusting it.

## The 2026-08-07 reproducibility finding (full report in the session record)

`build/cc1` **cannot be reproduced from this repo by any known recipe.** Six
candidate causes eliminated with evidence (local source mods, host compiler
version, configure/triple, source revision, optimization level, stale
objects): every rebuildable cc1 — including a pristine-source full clean
build — diverges from `build/cc1` on exactly two TUs (`ings`, `code6cac_b`;
7 instruction lines, constant-folding + `ori`/`addu` class) and AGREES with
the instrumented diagnostic cc1 on all 32. A from-scratch build also fails
to link under the modern host compiler without `-fgnu89-inline`
(`is_reserved_word` in c-gperf.h is `__inline` without `static`).

Consequences:
- If this binary were lost, the full-build SHA1 match could not be
  reproduced without re-matching the ~7 affected instruction sites in those
  two TUs under a rebuildable compiler.
- `ra_solver` dumps come from the (rebuildable, instrumented) diagnostic
  cc1, so for `ings`/`code6cac_b` they describe a compiler the project does
  not ship — enforced at point of use by `UNFAITHFUL_STEMS` in
  `tools/ra_solver/local_extract.py`. Treat solver evidence for functions in
  those TUs as UNAVAILABLE, not wrong (`saTan2KabutoWareMove` is the active
  consumer).

## Open owner decision

Whether to (a) keep the frozen unreproducible binary as the permanent
baseline (protected by backups + this hash), or (b) migrate to a
reproducible baseline — re-matching the ~7 divergent sites so the whole
toolchain can be rebuilt from source (a configuration-fidelity decision of
the same class as the 2026-08-04 `-mel` adoption; NOT to be made to fix a
diagnostic gap). Until decided: (a) is the operative state; do NOT replace,
rebuild over, or "fix" `build/cc1`.
