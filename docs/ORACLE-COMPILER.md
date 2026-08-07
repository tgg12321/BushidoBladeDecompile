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

## Tracked state manifest of the gitignored compiler tree (2026-08-07)

The `simplify_rtx` finding was possible because edits inside the gitignored
`tools/gcc-2.7.2/` tree leave no trace. This manifest makes the current
state detectable: verify with `sha1sum` from `tools/gcc-2.7.2/`; any drift
from these hashes means the diagnostic tree changed and must be re-recorded
here (with rationale) in the same change.

```
80c8088750a91b37ef53bea6da51d402c58801c8  flow.c            (BB2 debug hooks)
6f23c5eeeabc97f3049b2d414ab42b6f38b891f6  function.c        (BB2 debug hooks)
46af0a314da8ad6dcb27890ca9b2003006c70ced  global.c          (BB2 debug hooks)
9b8f822a79a1945ac4b58ebfb243017d67b828c5  jump.c            (BB2 debug hooks)
3fb248a6b2c85cd7e9e57b19f5df7daf62b3d5fe  local-alloc.c     (BB2 debug hooks incl. SUGG)
7ddde6b0f2b65172c5cc83be6165789f445953d9  reload1.c         (BB2 debug hooks)
73a15a5245d2e7ad55fa9e3c42d724488b1892d6  reorg.c           (BB2 hooks + 2 BEHAVIORAL knobs, inert unless set)
3668555e9cb7970b335a505aca4cfdda26e8fc49  sched.c           (BB2 debug hooks)
24c5952113d88cbb96f5c9f7e7152147d1efb8a7  combine.c         (UNMODIFIED upstream — cannot produce shipped simplify_rtx)
8384fd47cb51da369462a0ba0b83590eea88513a  cc1               (instrumented DIAGNOSTIC binary)
045c9543d39ab8109583b92137c7adde084f7a25  build/cc1         (THE ORACLE COMPILER — frozen)
```

Hook inertness proof: a cc1 built from pristine reverted sources is
byte-identical in behavior to the fully instrumented cc1 on all 32 TUs
(cc1_hooks.patch.md carries the details + the two behavioral-knob
declaration rule). Stale `.bb2bak` leftovers in the tree are exactly the
untracked-edit pattern this manifest exists to catch — clean them, don't
create more.

## Open owner decision

Whether to (a) keep the frozen unreproducible binary as the permanent
baseline (protected by backups + this hash), or (b) migrate to a
reproducible baseline — re-matching the ~7 divergent sites so the whole
toolchain can be rebuilt from source (a configuration-fidelity decision of
the same class as the 2026-08-04 `-mel` adoption; NOT to be made to fix a
diagnostic gap). Until decided: (a) is the operative state; do NOT replace,
rebuild over, or "fix" `build/cc1`.
