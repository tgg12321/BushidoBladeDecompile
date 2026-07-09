# W4 — Structure provenance verdict: D_800A125C..D_800A1497 (2026-07-09)

## Verdict: SEPARATE objects (with a twist that supersedes the question)

Ruling 1's deferred question — "is D_800A125C..0x800A1497 one object?" — is
answered **NO** by source-level identity evidence (stronger than any binary
inference): the range is PsyQ libcd bios.c v1.86's separate statics
(see [[libcd-identity]] / memory/closer/libcd-identity.md):

- `D_800A125C` = `char *CD_intstr[8]` — 8 string pointers ("NoIntr", "DataReady",
  "Complete", "Acknowledge", ... verified against the initialized words in
  asm/data/7D920.data.s:30843 pointing into the 0x800161xx string pool).
- 0x800A127C..0x800A145F = the four `static int` command-property tables
  (D_80032B68/BE8/C68/CE8 in sotn-decomp's bios.c — 0/1 flags + param counts;
  BB2's data at 13FC has the same 0/1/2/3 value profile). The
  "g_palette_32_*" names in named_syms.txt are BOGUS (zero code refs).
- `D_800A147C..1490` = the `static volatile u8* CD_reg` MMIO pointer statics
  (147C initialized to 0x1F801800).
- `D_800A1494/95/96` = `static volatile CD_intr Intr` — **these three ARE one
  object**: a 3-byte struct `{u8 sync; u8 ready; u8 c;}`.
- `D_800A1498` = `CD_init_struct` (its word 0 points at D_800A1494 = &Intr —
  independent confirmation that 1494 is an object base).

## Consequences for Ruling 1

1. The h5 lever `(u8*)tbl + (&D_800A1494 - D_800A125C) + 1` spans TWO unrelated
   objects (a string table and the Intr struct). Per Ruling 1's SEPARATE
   branch: **the h5 form is a fakematch and cpu_side_move_dir_4 cannot be
   COMPLETED with it.**
2. The community-standard merged-declaration escape does NOT apply to
   125C+1494, but DOES apply narrowly to 1494/95/96: declare
   `extern volatile CD_intr D_800A1494;` and access `.sync/.ready/.c` — honest
   single-object arithmetic, matching the original source.
3. The required honest respelling was dump/sandbox-measured this session:
   - direct member access (P2): masked 38 — KILLED;
   - volatile pointer chassis (P3/P4): masked 28 — KILLED;
   - i.e. the p79 refs>=5 requirement (s97/s98) is NOT met by the honest
     member-access family on CD_sync. CD_sync's endgame remains open, now
     targeted at reproducing the v1.86 source idiom (not at inventing one).
4. For marionation_Exec (=CD_ready) the same identity evidence yielded an
   immediate floor drop 4→2 with count parity 179/179 (volatile idx_1496 —
   original volatile semantics blocks the region-3 dbr fill). See
   memory/closer/candidates/marionation_p6_volatile1496.c.

## What would still strengthen/decide anything

Nothing for the one-struct question — it is settled by source identity.
The remaining open item is a JUDGE/OWNER question: whether the P6
partial-volatile spelling (only idx_1496 volatile while idx_1494/95 stay
non-volatile views of the same volatile struct) is acceptable, or whether the
full-volatile form (P5, masked 8) must be driven to 0 instead.
