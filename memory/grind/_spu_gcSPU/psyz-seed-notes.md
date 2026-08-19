# _spu_gcSPU — psyz seed lead (staged 2026-08-18, apply to memory/grind/_spu_gcSPU/notes.md at boundary)

VERSION-CORRECT SOURCE FOUND: Xeeynamo/psyz (MIT, PsyQ 4.0 decomp — the exact
library version BB2 links per memory/closer/libcd-groundtruth.md +
psyq-library-census.md) has a MATCHED body: decomp/src/libspu/s_m_int.c.
Local copy: tmp/psyq_prov/psyz_s_m_int.c (full clone tmp/psyq_prov/psyz/).
Full analysis: tmp/psyq_provenance.md (2026-08-18).

Verified 5-for-5 loop-phase match vs asm/funcs/_spu_gcSPU.s (194 insns).
Transplant plan:
1. Keep psyz's shape: plain `for` loops + ARRAY INDEXING `_spu_memList[i]`
   (NOT the current pointer chassis) — per-phase induction variables come free,
   which is what 121->25 of manual local-splitting approximated by hand.
2. Loop bound stays a GLOBAL READ in the for-condition (`i <= _spu_AllocLastNum`)
   — do NOT cache into a local; target reloads %hi/%lo(D_800A2D3C) at every
   phase head and back-edge (swap stores may alias it).
3. D_800A2D40 is `SPU_MALLOC *_spu_memList` (a POINTER global — `lw` then index);
   sibling SpuMalloc in the same TU already uses this spelling; unify.
4. RISK: catalogued cc1 ICE (memory/closer/phase3-progress.md — structured
   pointer-walk scans SIGSEGV cc1 in this function). The array-indexed form is
   NOT that shape and may sidestep it; measure first.
5. POLICY: psyz phase-1 inner loop has an empty `if (i) { // FAKE }` block —
   route through cheat-reviewer under dead-store-fake-exception /
   named-local-fake-exception BEFORE committing; test whether it is even needed
   under aspsx 2.34 (psyz targets 2.56).

Negative results (do not re-derive): CD_ready has NO version gap (bios.c v1.86
verbatim, object at tmp/closer/psyq/LIBCD_4.0.LIB; floor-4 residual is codegen
reproduction). _spu_note2pitch has NO source seed at any version (psyz stubs all
of s_n2p.c); self-decomp only. Do not confuse libsnd note2pitch (table-lookup,
psyz-matched, separate queue item d=49) with libspu _spu_note2pitch.
