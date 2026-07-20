# GRINDER CIRCUIT-BREAK — 2026-07-19 22:59 — RESOLVED 2026-07-19

**Reason:** 3 consecutive invalid sessions on func_800611A4 (all
`SCOPE VIOLATION — ?? nonmatchings/`)

**Root cause:** `tools/decomp-permuter/import.py:224` runs
`os.makedirs("nonmatchings/")` relative to the current directory. It is
normally invoked from `tools/decomp-permuter/` (which gitignores its own
`nonmatchings/`), but a grind permuter session ran it from the repo ROOT,
creating a root-level `nonmatchings/`. The root `.gitignore` did not cover it,
so `git status --porcelain` showed `?? nonmatchings/`; `nonmatchings/` is not in
the driver's `AllowedDirtyPattern`, so the scope check discarded the session.
Three permuter respawns in a row each recreated it → circuit-break.

**Fix (committed 2026-07-19):**
1. `.gitignore`: added `/nonmatchings/` — a permuter scratch artifact is never
   committed, and now can't be seen by the scope check regardless of where the
   permuter runs.
2. `tools/grinder/grind.ps1`: `Reap-PermuterOrphans` (runs every session
   boundary) now also removes any stray root `nonmatchings/`, so it can't
   accumulate.

This is the permuter-modality sibling of the other config/artifact stop-causes;
see memory `grinder-nonmatchings-scope-break`.
