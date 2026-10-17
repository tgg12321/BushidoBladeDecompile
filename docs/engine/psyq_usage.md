# PsyQ library usage

The Sony and SN Systems code linked into the main EXE lives in
`src/main/psxsdk/<lib>/`, one file per library module. Each file's top comment
gives the module's `.text` span and the evidence for it, usually a verbatim
match against PsyQ 4.0 `PSX/LIB` from LIBSCAN
(`docs/naming/libscan/matches.json`). Everything else in `src/main/` is game
code.

## Version

The 175 retained verbatim modules all match PsyQ 4.0 `.LIB` modules
(`.text`, relocations and exports). 79 of them match no 3.3, 3.5 or 3.6
module (docs/naming/libscan/psyq_versions.txt). That dates the libraries, not
the compiler (see AGENTS.md). Three modules carry CVS ids in their
`.rodata`:

- libgpu `sys.c,v 1.129` (src/main/psxsdk/libgpu/sys.c:86);
- libcd `bios.c,v 1.86` (src/main/psxsdk/libcd/bios.c:191);
- libetc `intr.c,v 1.76` (src/main/psxsdk/libetc/intr.c:14).

LIBSND is an interim build between 4.0 and 4.1
(src/main/psxsdk/libsnd/vm_init.c:1-4).

## Libraries linked

| Library | Files | What the game uses it for |
| --- | --- | --- |
| libgpu | 3: `sys.c`, `prim.c`, `ext.c` | `ResetGraph`, `SetDispMask`, `DrawSync`, `ClearOTagR`, `DrawOTag`, `PutDrawEnv`, `PutDispEnv`, `LoadImage`, `ClearImage`, `SetDefDrawEnv` / `SetDefDispEnv`, `GetTPage` / `GetClut`, `AddPrim`; see [gpu_pipeline.md](gpu_pipeline.md) |
| libgte | 37 | `InitGeom`, `SetGeomOffset`, `SetGeomScreen`, `SetRotMatrix`, `SetTransMatrix`, `RotMatrix*`, `MulMatrix*`, `ApplyMatrix*`, `RotTransPers*`, `rsin` / `rcos`, `ratan2`. Most are canonical inline-asm (GTE) bodies |
| libetc | 5: `intr.c`, `intr_vb.c`, `intr_dma.c`, `vsync.c`, `vmode.c` | `ResetCallback`, `InterruptCallback`, `DMACallback`, `VSyncCallback`, `StopCallback`, `CheckCallback`, `SetIntrMask`, `VSync`, `SetVideoMode` |
| libapi | 42 | BIOS trampolines (events, `open` / `read` / `write` / `close`, `firstfile` / `nextfile`, `Exec`, `InitHeap`, `SetMem`, `EnterCriticalSection` / `ExitCriticalSection`, `ChangeClearPAD`); pad (`pad.c`: `InitPAD`, `StartPAD`, `StopPAD`); root counters (`counter.c`: `SetRCnt`, `GetRCnt`, `StartRCnt`, `StopRCnt`, `ResetRCnt`) |
| libcd | 4: `event.c`, `sys.c`, `bios.c`, `cdread.c` | `CdInit`, `CdControl` / `CdControlF` / `CdControlB`, `CdSync`, `CdReady`, `CdReadyCallback`, `CdGetSector`, `CdMix`, `CdRead` / `CdReadSync`, `CdIntToPos` / `CdPosToInt`; see [file_io.md](file_io.md) |
| libcard | 10 | `InitCARD`, `StartCARD`, `StopCARD`, `_bu_init` and the card BIOS calls behind `memcard.c` |
| libspu | 32 | `SpuInit`, `SpuSetKey`, `SpuSetReverb*`, `SpuSetCommonAttr`, `SpuMalloc` / `SpuFree`, `SpuWrite` / `SpuRead`, the `_spu_*` register layer in `spu.c` |
| libsnd | 46 | `SsInit`, `SsVabOpenHead*` / `SsVabTransBody`, `SsUtKeyOnV` / `SsUtKeyOffV`, `SsSetSerialAttr` / `SsSetSerialVol`, the `_SsVm*` voice manager, the SEQ player; see [sound.md](sound.md) |
| libcomb | 1: `comb.c` | the link-cable SIO driver. The game's `comb_*` session layer (28708.c) calls `AddCOMB` / `DelCOMB` and `_comb_control`, and opens the driver's device with BIOS `open` for `read` / `write` |
| libc2 | 13 | `printf`, `sprintf`, `puts`, `putchar`, `memcpy`, `memmove`, `memchr`, `strcpy`, `strlen`, `rand`, `toupper` / `tolower`, `setjmp` |
| libsn | 2: `73E98.c`, `7407C.c` | the SN runtime: `__SN_ENTRY_POINT`, `__main`, `__do_global_dtors`, and the PC file server `PCopen`, `PCclose`, `PClseek`, `PCread` / `_SN_read` |

The game calls some of these directly at boot:

- `sys_Init` (src/main/6CF8.c:175) runs `ResetCallback`, `InitPAD`,
  `StartPAD` and `ChangeClearPAD(0)`, then `disp_Init`, `cdrom_Init`,
  `memcard_Init` and `rcnt_StartCnt1Wrapper`.
- `disp_Init` (src/main/6CF8.c:157) is the libgpu / libgte set-up.
- `memcard_Init` (src/main/memcard.c:28) is the libcard and event set-up.
- `snd_InitAndLoadCommonVab` (src/main/6CF8.c:265) is the sound set-up.

## BIOS trampolines

The libapi and libcard trampolines are written with `BIOS_A_FUNCTION` /
`BIOS_B_FUNCTION` / `BIOS_C_FUNCTION` (`include/bios.h`), one per Sony
module. Each one jumps through the A0 / B0 / C0 table with its function
number, for example `BIOS_B_FUNCTION(open, 0x32)` and
`BIOS_B_FUNCTION(read, 0x34)`. These have no C form and are canonical.

## SN Systems runtime

- `__SN_ENTRY_POINT` is the EXE entry. See
  [memory_layout.md](memory_layout.md) for what it sets up.
- `__main` walks the empty `.ctors` table
  (src/main/psxsdk/libsn/73E98.c:16-21).
- `PCopen` / `PCclose` / `PClseek` / `PCread` talk to the SN PC file
  server, which needs a development host. The game's only callers,
  `pcdrv_LoadFile` and `pcdrv_LoadSectors` (src/main/6CF8.c:47, :78), are
  themselves unreferenced. Disc and memory-card I/O use libcd and the BIOS
  file calls instead.

## The FMV overlay

`STR\MOVOVL.EXE` is a separate PS-X EXE that `sys_Exec` loads and runs
(src/main/cdrom.c:209). Its contents are described in
[docs/formats/MOVOVL.md](../formats/MOVOVL.md). Its libraries are not part
of this tree's `src/`.
