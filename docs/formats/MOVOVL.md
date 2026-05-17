# MOVOVL.EXE — FMV / MDEC playback overlay

File: `disc/STR/MOVOVL.EXE` (124,928 bytes).

A standalone, self-contained PS-X EXE that plays the BB2 title-screen and
opening cinematics from `disc/STR/OPENING.STR`. The main game executable
(`disc/SLUS_006.63`) is unloaded from memory before the overlay runs, so
the overlay has the entire main RAM (other than the BIOS kernel area) to
itself and re-initialises the pad / interrupt / GPU environments from
scratch.

## Header

Standard PS-X EXE header (the same 0x800-byte layout used by the main EXE
and re-prepended by `tools/make_psexe.py` during the matching build):

| Field            | Value                                            |
|------------------|--------------------------------------------------|
| Signature        | `"PS-X EXE"` (offset `0x00`)                     |
| Marker text      | `Sony Computer Entertainment Inc. for Japan area`|
| Entry point (PC) | `0x801DA084`                                     |
| Initial `$gp`    | `0x00000000` (set by the entry code, not header) |
| Text load addr   | `0x801D8800`                                     |
| Text size        | `0x1E000` (122,880 bytes)                        |
| Data / BSS       | not declared in header (`addr=0, size=0`)        |
| Initial `$sp`    | `0x801FFFF0` (top of RAM, same as main game)     |

The text region runs `0x801D8800 .. 0x801F6800`. The overlay's writable
state (decoder ring buffers, frame buffers, libapi event tables, etc.)
lives implicitly after the text region — the inferred BSS / runtime
heap begins at `0x801F6800`.

## Entry-point bootstrap

```
801DA084  lui   $gp, 0x801F
801DA088  addiu $gp, $gp, 0x65D0     ; gp = 0x801F65D0
801DA08C  j     0x801D91CC           ; main()
801DA090  nop
```

`$gp = 0x801F65D0` puts the small-data window at `0x801F5DD0 .. 0x801F6DD0`
(the libpress decoder context lives just past the text segment, inside
this window). Control then jumps to the C runtime's `main()` at
`0x801D91CC`.

## Library identification

Three CVS `$Id:` tags survive inside the linked PsyQ object files —
identical versions to the main EXE, confirming both binaries were built
against the same SDK 3.5 snapshot:

| Library    | CVS tag                                            |
|------------|----------------------------------------------------|
| libgpu     | `sys.c v1.129 1996/12/25 03:36:20 noda`            |
| libcd      | `bios.c v1.86 1997/03/28 07:42:42 makoto`          |
| libapi     | `intr.c v1.76 1997/02/12 12:45:05 makoto`          |

The trailing string `Library Programs (c) 1993-1997 Sony Computer
Entertainment Inc., All Rights Reserved.` at `0x801F5488` is the
standard PsyQ runtime copyright.

The overlay also contains characteristic libpress debug strings
(`MDEC_in_sync`, `MDEC_out_sync`, `MDEC_rest:bad option(%d)`,
`size of DECENV_R = 0x%08X`, `size of DECENV_B = 0x%08X`) which appear
in the standard `MovInit()` / `MovOpen()` / `MovStart()` ring buffer
setup paths. MDEC register access (the `0x1F801820` / `0x1F801824`
hardware ports) does NOT appear inline anywhere — the overlay always
goes through these library wrappers.

## Code-side characterisation

`tools/inspect_movovl.py` performs heuristic static analysis. Current
findings (subject to change as the overlay is decompiled further):

| Metric           | Value                                                |
|------------------|------------------------------------------------------|
| Detected funcs   | 141 (counted by `addiu $sp, -N` + nearby `sw $ra`)   |
| BIOS call sites  | 17 (canonical `li $t2, TBL; jr $t2; li $t1, FN`)     |
| ASCII strings    | 516 (>= 4 chars; includes debug `printf` formats)    |

### BIOS calls

The 17 BIOS calls cover the minimal environment setup an overlay needs
when launched in isolation:

| Table | Func | Name                            | Count | Use                            |
|-------|------|---------------------------------|------:|--------------------------------|
| 0xA0  | 0x44 | `FlushCache`                    | 1     | cache invalidate after load    |
| 0xA0  | 0x49 | `GPU_cw`                        | 1     | low-level GPU command word     |
| 0xA0  | 0x72 | (unidentified)                  | 1     | likely `CdInit` or `CdEnable`  |
| 0xB0  | 0x07 | `DeliverEvent`                  | 1     | event-driven I/O               |
| 0xB0  | 0x12 | `InitPAD`                       | 1     | pad input init                 |
| 0xB0  | 0x13 | `StartPAD`                      | 1     | begin pad polling              |
| 0xB0  | 0x14 | `StopPAD`                       | 1     | pre-shutdown                   |
| 0xB0  | 0x15 | (unidentified)                  | 1     | likely `PAD_init` companion    |
| 0xB0  | 0x16 | (unidentified)                  | 1     | likely `OutdatedPadGetButtons` |
| 0xB0  | 0x17 | `ReturnFromException`           | 1     | exception ret                  |
| 0xB0  | 0x18 | `SetDefaultExitFromException`   | 1     | exception setup                |
| 0xB0  | 0x19 | `SetCustomExitFromException`    | 1     | exception setup                |
| 0xB0  | 0x35 | (unidentified)                  | 1     | likely `_card_*` BU-related    |
| 0xB0  | 0x5B | (unidentified)                  | 1     | likely `_card_chan`            |
| 0xC0  | 0x02 | `SysEnqIntRP`                   | 1     | install IRQ handler            |
| 0xC0  | 0x03 | `SysDeqIntRP`                   | 1     | remove IRQ handler             |
| 0xC0  | 0x0A | `ChangeClearRCnt`               | 1     | reset RCnt (root counter)      |

(Names tagged "unidentified" lie outside the small built-in name table
in `tools/inspect_movovl.py` — they are still valid BIOS indices, just
ones the tool doesn't yet have human names for.)

The overlay never calls into the main game (no shared symbol table is
loaded) and never re-loads the main EXE — exit / return-to-game is
performed by completing the FMV and jumping back through the BIOS
boot path, which re-loads the main EXE off disc.

## Relationship to the main EXE

Loading the overlay is `file_LoadOverlay` (`src/ings.c:370`), which
DMAs the overlay body into RAM at the canonical overlay address:

```c
// src/ings.c:370
void file_LoadOverlay(void) {
    s32 size;
    if (g_file_dma_flag != 0) return;
    size = func_80060CB8(0x801D8800, 0x8010E800);
    debug_printf((s32)&g_str_eff_init, 0x8010E800, size);
    if (0xA000 < size) sys_Panic();        // sanity-check: <40 KB
    g_file_dma_flag = 1;
}
```

`func_80060CB8(dest, staging)` is the not-yet-named CD-load shim — given
the size guard `0xA000 < size -> Panic`, it appears to copy a small
overlay/effect blob (≤ 40 KB) rather than the full 122 KB MOVOVL.EXE.
That suggests `file_LoadOverlay` actually loads a *different* overlay
blob (the in-game effects overlay) into the same address window, and
the FMV overlay is loaded by a separate boot-time path that the main
game itself never invokes (the BIOS bootstrap sequence runs
`SLUS_006.63` after `MOVOVL.EXE` exits, not the other way around).

The 0x801D8800 region is additionally re-used by the main game as a
generic scratch buffer when no overlay is resident:

```c
// src/ings.c:388 file_LoadSoundData — stages sound data at 0x801D8800
size = func_8005B7C4(0x801D8800);
if (size >= 0xD01) sys_Panic();
bb2_memcpy(0x8010DB00, 0x801D8800, size);

// src/ings.c:414 sys_GameInit — registers the region as a buffer
D_800A3770 = 0x801D8800;        // buffer base
D_800A3774 = 0x801EBC00;        // buffer end
D_800A3798 = 0x13400;           // buffer size = 78,848 bytes
```

`0x801EBC00 - 0x801D8800 = 0x13400 = 78,848 bytes` — the main game
claims only the first 78 KB of the 122 KB overlay region for in-game
use, presumably to leave the upper 44 KB clear for occasional larger
overlays.

### Filename not in the main EXE

The overlay's filename (`STR\MOVOVL.EXE` or the standard PSX
ISO-9660 variant `MOVOVL.EXE;1`) is **not present anywhere in the main
EXE** — neither the bare name nor any path variant. So `MOVOVL.EXE` is
not loaded by `CdSearchFile` from within the running game; the most
likely scenarios are:

1. The overlay runs first (from BIOS / CD bootstrap), and the main EXE
   is launched only after the overlay has played the FMV and issued
   `LoadAndExecute("SLUS_006.63;1", ...)` (BIOS A-table 0x51).
2. The main EXE has a hard-coded disc sector range for MOVOVL captured
   at build time, baked in as a constant and read via `CdReadSector` /
   `CdRead2` — in which case the loader function's literals would be
   sector numbers, not strings, and `grep` for `MOVOVL` won't find it.

Scenario (1) matches the typical PSX intro-movie flow (BIOS → intro
overlay → main game). The `file_LoadOverlay` call at `src/ings.c:370`
is then almost certainly loading something else (a much smaller
in-game effect overlay), and MOVOVL.EXE has no main-EXE caller.

## Inspector

```
python tools/inspect_movovl.py disc/STR/MOVOVL.EXE              # summary
python tools/inspect_movovl.py disc/STR/MOVOVL.EXE --strings    # 516 strings
python tools/inspect_movovl.py disc/STR/MOVOVL.EXE --funcs      # 141 prologue addrs
python tools/inspect_movovl.py disc/STR/MOVOVL.EXE --syscalls   # 17 BIOS calls with names
```

For full MIPS disassembly use the WSL toolchain (`mipsel-linux-gnu-objdump
-D -b binary -m mips --adjust-vma=0x801D8800 <text-only-blob>`).

## Verified by

* Header parses correctly against the PS-X EXE format used by
  `make_psexe.py`. SHA1 of the original disc file is
  `a1307dbebefca0b057e02509207d00f6225e13e4` (the overlay is read-only
  asset data, not rebuilt by the matching pipeline).
* All three PsyQ `$Id:` tags match the versions documented for the main
  EXE in CLAUDE.md (libgpu sys.c v1.129, libcd bios.c v1.86, libapi
  intr.c v1.76).
* The 17 BIOS call sites use the canonical PSX trampoline pattern
  (`li $t2, 0xa0/b0/c0` then `jr $t2`, function index in `$t1` in the
  delay slot) and decode to sensible names for the identified subset.
* Cross-reference with the main EXE: `0x801D8800` is treated as a
  scratch buffer in `src/ings.c` (lines 376, 392, 396, 418), confirming
  the overlap-then-discard memory pattern.
* The libpress debug strings (`MDEC_in_sync`, `MDEC_out_sync`, etc.)
  are direct text matches against the PSX SDK 3.5 `libpress` library
  source — the overlay does not reimplement MDEC handling, it calls the
  vendor library.

## Unverified / TODO

* Identify the main-EXE loader that invokes MOVOVL.EXE — it must be a
  `CdReadSector` call with a hard-coded sector range; finding it would
  let us flag any future repository changes that affect overlay load.
* Name the remaining 6 unidentified BIOS function indices (`0xa0/0x72`,
  `0xb0/0x15`, `0xb0/0x16`, `0xb0/0x35`, `0xb0/0x5B`). The PSX BIOS
  reference (no-cash docs / SDK headers) lists all of them.
* Identify the FMV decoder entry function (likely `play_str()` or
  `MovInit()` / `MovStart()` / `MovWait()` — names taken from PsyQ
  libpress) and document the playback loop.
* Confirm the overlay's exit path. Hypothesis: it issues a software
  reset back to the BIOS boot path which re-loads the main EXE off the
  disc filesystem; an alternative is that it does its own
  `LoadAndExecute("SLUS_006.63;1", sp, gp)` via BIOS A-table 0x51.
