# Adversarial verification of the 49 Sony-name claims

Default-refute re-derivation, 2026-08-18. Every row was re-derived from the raw
`tmp/libscan/psyq40/*.LIB` containers and `disc/SLUS_006.63`. `tmp/libscan/symbols.txt` was
treated as a conclusion to be reproduced, not as evidence.

## What a CONFIRM means here

1. **Placement, exhaustively.** For each implicated module the masked-identity search was re-run
   over *every* word-aligned position in the 151040-word text image, not just the anchor-derived
   candidates the original scan considered. Every one of the 11 modules lands in exactly one place.
2. **Symbol at the exact offset.** The claimed name had to appear in the OBJ symbol table at
   precisely `addr - module_base`, as a LOCAL (static) symbol, with no second symbol at that offset
   and no second module covering the address. Off-by-anything would have refuted the row.
3. **Body sanity.** 34 of the 48 rows were disassembled out of the raw EXE and read against the
   claimed name. All 34 sit immediately after a `jr ra` + delay slot, which also confirms each
   claimed address is a real function start rather than an interior label.
4. **Collisions.** Whole-word scan of `src/`, `include/`, all four symbol registries, `regfix.txt`,
   `asmfix.txt`, the maspsx gate lists, `bb2.ld`, `Makefile` and `tools/prologue_config.json`, with
   C comments stripped. The scanner was validated against known-present tokens before its clean
   result was trusted. No live hit for any of the 48 names.

Match quality of the 11 implicated modules: no fully-wildcarded words, no unknown relocation types,
and only 19-45% of words *partially* masked (HI16/LO16 retain 16 bits, REL26 retains 6).

## Verdicts

| addr | claimed name | grade | verdict | evidence |
|---|---|---|---|---|
| `0x80078da0` | `SetPatchPad` | VERBATIM | **CONFIRM** | LIBAPI/PAD uniquely placed at 0x80078BE0 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x1c0; body: EnterCriticalSection, then installs 0x80078E58 (_Pad1) and 0x80078EC0 (_IsVSync) into a callback table - mutually corroborates all three names |
| `0x80078e20` | `RemovePatchPad` | VERBATIM | **CONFIRM** | LIBAPI/PAD uniquely placed at 0x80078BE0 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x240; body: EnterCriticalSection, clears the SAME table SetPatchPad fills, ExitCriticalSection - the exact inverse of SetPatchPad |
| `0x80078e58` | `_Pad1` | VERBATIM | **CONFIRM** | LIBAPI/PAD uniquely placed at 0x80078BE0 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x278; body: zeroes field+10 of the PAD global then runs a 10-iteration poll |
| `0x80078ec0` | `_IsVSync` | VERBATIM | **CONFIRM** | LIBAPI/PAD uniquely placed at 0x80078BE0 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2e0; body: reads bit0 of field+4 of the PAD global - a vsync predicate |
| `0x8007b3a8` | `checkRECT` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x52c; body: dispatches on the GPU mode byte and range-checks RECT fields |
| `0x8007c4b8` | `SetDrawEnv2` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x163c |
| `0x8007c748` | `get_mode` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x18cc; body: builds a GP0 0xE1xxxxxx word = the draw-MODE command |
| `0x8007c7a0` | `get_cs` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x1924; body: clamps a signed halfword arg against a global limit (clip start) |
| `0x8007c86c` | `get_ce` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x19f0; body: the same clamp idiom as get_cs - its twin |
| `0x8007c938` | `get_ofs` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x1abc; body: builds a GP0 0xE5xxxxxx word = the drawing-OFFSET command |
| `0x8007c97c` | `get_tw` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x1b00; body: reads a RECT (lbu +0, lh +4) and builds a texture-window word |
| `0x8007ca00` | `get_dx` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x1b84; body: dispatches on the GPU mode byte and reads RECT fields |
| `0x8007cab0` | `_status` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x1c34; body: loads the GPU ctx pointer and returns *ptr (status word) |
| `0x8007cac8` | `_otc` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x1c4c; body: ORs 0x08000000 into the GPU ctl word - OT/DMA clear path |
| `0x8007cbb0` | `_clr` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x1d34; body: clamps a RECT against the same global limit as get_cs - region clear |
| `0x8007ce0c` | `_dws` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x1f90 |
| `0x8007d048` | `_drs` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x21cc |
| `0x8007d2cc` | `_ctl` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2450; body: writes a0 to GP1 and caches the byte at index a0>>24 - the setter half |
| `0x8007d2f4` | `_getctl` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2478; body: reads back the cached ctl byte indexed by the arg - getter half of _ctl |
| `0x8007d308` | `_cwb` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x248c; body: writes GP1 0x04000000 then loops words from a0 into the GP0 port |
| `0x8007d358` | `_cwc` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x24dc; body: writes GP1 0x04000002 (linked-list DMA) and programs the DMA MADR |
| `0x8007d3a4` | `_param` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2528; body: ORs the arg with 0x10000000 (GP1 info request), reads back, masks 24 bits |
| `0x8007d3d4` | `_addque` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2558; body: tail-calls 0x8007D3F8 (_addque2) with a2=0 - the wrapper half |
| `0x8007d3f8` | `_addque2` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x257c |
| `0x8007d6d8` | `_exeque` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x285c |
| `0x8007d9c4` | `_reset` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2b48 |
| `0x8007db20` | `_sync` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2ca4 |
| `0x8007dc68` | `set_alarm` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2dec; body: calls VSync(-1) at 0x800828CC, stores deadline+240 plus a flag |
| `0x8007dc9c` | `get_alarm` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2e20; body: calls VSync(-1) and compares against the deadline set_alarm stored |
| `0x8007de08` | `_version` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x2f8c; body: writes GP1 0x10000007 (GPU info) and masks the reply to 24 bits |
| `0x8007dee4` | `memset` | VERBATIM | **CONFIRM** | LIBGPU/SYS uniquely placed at 0x8007AE7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x3068; body: byte-fill loop (sb a1,0(a0)) counted by a2 - literally memset(a0,a1,a2) |
| `0x80080014` | `def_cbsync` | VERBATIM | **CONFIRM** | LIBCD/EVENT uniquely placed at 0x8007FF7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x98; body: tail-calls the event helper with class 0xF0000003, spec 0x20 |
| `0x8008003c` | `def_cbready` | VERBATIM | **CONFIRM** | LIBCD/EVENT uniquely placed at 0x8007FF7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0xc0; body: same helper, spec 0x40 (body identical to def_cbread - see caveat) |
| `0x80080064` | `def_cbread` | VERBATIM | **CONFIRM** | LIBCD/EVENT uniquely placed at 0x8007FF7C (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0xe8; body: same helper, spec 0x40 (body identical to def_cbready - see caveat) |
| `0x80080828` | `getintr` | VERBATIM | **CONFIRM** | LIBCD/BIOS uniquely placed at 0x80080828 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x0; body: sets a CD status flag byte and reads the CD interrupt status |
| `0x8008241c` | `cd_read_retry` | VERBATIM | **CONFIRM** | LIBCD/CDREAD uniquely placed at 0x80082050 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x3cc |
| `0x80082a14` | `v_wait` | VERBATIM | **CONFIRM** | LIBETC/VSYNC uniquely placed at 0x800828CC (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x148; body: spins comparing a vsync counter global against the arg |
| `0x80082c58` | `startIntr` | VERBATIM | **CONFIRM** | LIBETC/INTR uniquely placed at 0x80082AC0 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x198; body: shares the INTR globals with trapIntr |
| `0x80082d34` | `trapIntr` | VERBATIM | **CONFIRM** | LIBETC/INTR uniquely placed at 0x80082AC0 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x274; body: touches the same INTR globals (0x800A1578 / 0x800A2604) as startIntr |
| `0x80083c34` | `_SsStart` | VERBATIM | **CONFIRM** | LIBSND/SSSTART uniquely placed at 0x80083C34 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x0; body: 999-iteration delay spin then writes an 0xF2000002 SPU word |
| `0x800880e8` | `SsVabOpenHeadWithMode` | VERBATIM | **CONFIRM** | LIBSND/VS_VH uniquely placed at 0x80088058 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x90 |
| `0x800889d4` | `_spu_FwriteByIO` | VERBATIM | **CONFIRM** | LIBSPU/SPU uniquely placed at 0x80088740 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x294 |
| `0x80089214` | `_spu_FsetDelayW` | VERBATIM | **CONFIRM** | LIBSPU/SPU uniquely placed at 0x80088740 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0xad4; body: masks 0x0F000000 and ORs 0x20000000 into the SPU DMA ctrl word |
| `0x80089240` | `_spu_FsetDelayR` | VERBATIM | **CONFIRM** | LIBSPU/SPU uniquely placed at 0x80088740 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0xb00; body: same register, ORs 0x22000000 - the sibling half of the W/R pair |
| `0x8008bea4` | `SioAnsyncRead` | VERBATIM | **CONFIRM** | LIBCOMB/COMB uniquely placed at 0x8008BE04 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0xa0 |
| `0x8008bf04` | `SioSyncroRead` | VERBATIM | **CONFIRM** | LIBCOMB/COMB uniquely placed at 0x8008BE04 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x100 |
| `0x8008c184` | `SioAnsyncWrite` | VERBATIM | **CONFIRM** | LIBCOMB/COMB uniquely placed at 0x8008BE04 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x380 |
| `0x8008c1e8` | `SioSyncroWrite` | VERBATIM | **CONFIRM** | LIBCOMB/COMB uniquely placed at 0x8008BE04 (exhaustive: 1 placement in 151040 words); LOCAL symbol at exact +0x3e4 |
| `0x800871d4` | `_SsVmKeyOffNow` | STRUCTURAL | **HELD-BY-POLICY** | excluded by owner directive regardless of evidence; 4 of 52 words differ and it sits in no verbatim module, so the placement chain cannot be re-derived at all |

## Counts

- CONFIRM: 48
- REFUTE: 0
- HELD-BY-POLICY: 1

## Caveats recorded against otherwise-confirmed rows

- **`def_cbready` (0x8008003C) / `def_cbread` (0x80080064)** have byte-identical bodies (same
  helper, same 0x40 spec), so a body-level check cannot tell them apart. The distinction rests
  entirely on the two symbol-table offsets inside a module whose placement is exact and unique.
  That chain is sound, but if the module placement were ever wrong these two names would swap
  silently rather than fail loudly.
- **`memset` (0x8007DEE4)** is the one name that lands in the compiler's builtin namespace. The
  project compiles every file with `-fno-builtin` (Makefile:36-39), and a cc1 differential on the
  real body plus three constant-size call sites produced identical assembly under both names, so
  the rename is codegen-neutral. Live call sites: `src/display.c:77,917,918`, `src/gpu.c:584,592,593`.

## Blockers on APPLYING this set (independent of name truth)

1. **All 48 census rows are `action=KEEP`.** `tools/naming_wave.py` only honours an override when
   the census action is RESET/RENAME/empty, so as written the whole set is silently inert.
2. **`0x80082c58` (`startIntr`) is absent from the census entirely** - it exists only as
   `func_80082C58` in `src/ings2.c:206`. naming_wave.py iterates census rows, so this row can never
   be applied by the sanctioned tool.
3. **The Grinder is running.** naming_wave.py rewrites `engine/queue.json`, `regfix.txt`,
   `asmfix.txt` and the gate lists, and several targets are queue-active or parked
   (func_8007CBB0, func_8007D048, func_8007D6D8, func_8007DC9C, func_80080828, func_800889D4,
   func_8008C1E8, and `copy` @0x8007D2CC). Application needs a grinder-idle window.
