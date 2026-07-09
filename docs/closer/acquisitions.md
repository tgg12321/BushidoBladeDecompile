# Closer acquisitions — owner shopping list (2026-07-09, Phase 1)

## REFRAME (read first)

The two contested functions are NOT game code: they are **PsyQ libcd bios.c
v1.86** (`CD_sync` / `CD_ready`; `saEft01Init` = `CD_cw`) — proven by string
xref (`$Id: bios.c,v 1.86 1997/03/28 makoto` at 0x80016274 plus the
"CD_sync"/"CD_ready"/"CD_cw" name strings the functions store into the Alarm
struct). See `memory/closer/libcd-identity.md`.

Therefore the multi-build triangulation target is **any PS1 executable linking
libcd bios.c v1.86**, not merely BB2 variants. Every such EXE contains the SAME
functions compiled from the same source — each one is another compile of the
answer. The locator (`tmp/closer/locate_window.py`, validated on the local USA
image and SLUS_006.63; reports the bios.c version of any image/EXE) makes
extraction a one-command operation the moment an image lands in `evidence/`.

## What to acquire (priority order)

1. **Bushido Blade 2 (Japan)** — catalog **SLPS-01210** (Square, 1998-03-05,
   redump-verified). Built ~5 months before USA; near-certainly the same libcd
   v1.86. Extract: run the locator; pull SLPS_012.10; disassemble the CD_sync/
   CD_ready windows. Payoff: a second compile of the identical source with a
   different surrounding link context — confirms which residual bytes are
   context-free (source-forced) vs allocation-context artifacts.
2. **Bushido Blade 2 Taikenban / demo** (check Hidden Palace "Bushido Blade 2"
   entries and the Square "PREVIEW 6" / Dengeki demo discs of spring 1998).
   Prototypes sometimes ship with .SYM files — a SYM naming CD_sync would also
   let us batch-rename the whole libcd region.
3. **Bushido Blade 1 (USA SCUS-94180 / JP SLPS-00654, 1997)** — same developer;
   earlier libcd (likely v1.7x). Payoff: version-delta triangulation of bios.c
   (v1.7x vs v1.86 diff isolates what makoto changed — i.e. whether the
   idx-pointer staging in v1.86's CD_ready/CD_sync is a source change).
4. **Any redump PS1 title with libcd v1.86** already owned — run the locator
   over any locally available images first; zero acquisition cost. (Kengo is
   PS2/libcdvd — not applicable.)

## No-download research targets (web, allowed)

- **Open-source matched decomps of libcd**: sotn-decomp
  `src/main/psxsdk/libcd/bios.c` (fetched already → `tmp/closer/sotn_bios.c`,
  **v1.77**). Search other decomps (REDRIVER2/OpenDriver2, mgs, croc, etc.) for
  a bios.c closer to or exactly v1.86 — an exact-version matched decomp would
  BE the source. PsyQ 4.x-era libcd (1998+) is v1.9x+; the v1.86 sweet spot is
  PsyQ 3.6/4.0-era titles (mid-1997..mid-1998).
- **PsyQ SDK library archives** (psx.dev / archive.org SDK mirrors): LIBCD.LIB
  from PsyQ 3.6/4.0 contains BIOS.OBJ compiled from v1.8x — its .obj bytes are
  a direct compile of the source (linkable/diffable without any game disc).
  This is likely the SINGLE HIGHEST-VALUE acquisition: the exact original
  object code for the exact source version, with symbol names.

## Extraction recipe (ready to run)

```bash
python3 tmp/closer/locate_window.py "evidence/<image>.bin"   # anchors + bios.c version
# then: extract main EXE from disc filesystem (tools/extract_iso.py),
# locate_window.py on the EXE gives vaddrs; disassemble +/- 0x400 bytes around
# the code that lui/addiu-references the "CD_sync"/"CD_ready" string vaddrs.
```
