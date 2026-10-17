# Sound

Game sound has three parts:

- a small game layer in 4BC3C.c, 6CF8.c, 267A8.c and 26730.c, which loads
  VAB banks into numbered slots and queues one-shot sounds;
- Sony libsnd (`src/main/psxsdk/libsnd/`), which manages VABs, voices and
  sequences;
- Sony libspu (`src/main/psxsdk/libspu/`), which drives the SPU hardware and
  SPU RAM.

CD audio (CD-DA and XA) is played by the CD state machine and mixed in
through the CD attenuators. See [file_io.md](file_io.md). The banks on disc
are described in [docs/formats/](../formats/README.md).

## Set-up

`snd_InitAndLoadCommonVab` (src/main/6CF8.c:265) runs these steps:

1. `snd_Init`.
2. `snd_LoadCommonVab(0x801D8800)`. A size of 0xD01 or more calls
   `sys_Panic`.
3. Copies the bank's main-RAM part (sound table + VabHdr) to 0x8010DB00 and
   rebases slot 0 there with
   `snd_VabFakeOpen(0xFFF35300, 0)` (0x8010DB00 - 0x801D8800).
4. `func_8005C614`, which sets `SsSetMVol(0x7F, 0x7F)`, stereo and auto
   key-off mode 0.
5. Sets `D_800A3906 = 1`. That flag lets `cdrom_WaitIdle` call
   `func_8005C6D0` on each pass (`main` calls it every frame regardless,
   src/main/6CF8.c:458).

`sys_Exec` (src/main/cdrom.c:209) runs it again after the FMV overlay
returns. `func_80016D78` (src/main/6CF8.c:285) runs it at boot.

## VAB slots (4BC3C.c)

The game keeps 16 VAB slots in two arrays: `g_vab_rec_ptr[16]` (the loaded
pack, `Unk8005C2A8Pack`, include/game.h:180-191) and `g_vab_vb_sbaddr[16]`
(the body's SPU address). A pack is four words. The first three are
file-relative offsets: the u32 sound table, the `VabHdr`, and the body. The
fourth is the body size in SPU RAM.

| Function | What it does |
| --- | --- |
| `snd_Init` | clears the 16 slots, `SsInit`, all voices off (`func_800858D0`), reverb off, `SsSetReservedVoice(0)`, `SsSetTickMode(1)`, resets the 24-entry request pool, `SsStart` |
| `snd_Quit` | voices and reverb off, `SsEnd`, `SsQuit`, clears the slots |
| `snd_LoadCommonVab(dst)` | reads group-2 file 1 into `dst` and loads it as slot 0 with `func_8005C2A8`, at SPU address 0x1010 |
| `func_8005C2A8(pack, slot, scratch)` | closes the slot. For a slot other than 0 it sets the next SPU address to the end of the resident banks and compacts SPU RAM (`func_8005C074`). It relocates the pack's offsets, opens it with `snd_VabOpen`, then records the slot and `SsUtGetVBaddrInSB(slot)`. Prints `"vab id:%d mistake"` on failure |
| `snd_VabOpen(pack, slot)` | `SsVabOpenHeadSticky` at the next SPU address, then `SsVabTransBody` |
| `snd_VabFakeOpen(delta, slot)` | moves a slot's pack in main RAM by `delta` and re-registers it with `SsVabFakeHead` / `SsVabFakeBody` |
| `func_8005C074(slot, buf)` | sorts slots 1..15 by SPU address and moves every body after the first gap down to close it (`snd_MoveVabBody`) |
| `snd_MoveVabBody(buf, slot, dst, src)` | copies a body inside SPU RAM through `buf` (`SpuRead`, then `SpuWrite`) and re-fakes its head |
| `func_8005B8B8` / `func_8005B9FC` / `func_8005BA8C` | load group-2 files 0x5D + 0x5E into slots 8 and 4, file 8 into slot 9, and file `9 + arg1` into the three slots in `g_vab_id_list` |
| `snd_CloseVab1`, `snd_CloseListedVabs`, `func_8005B644`, `func_8005B6AC`, `func_8005B868`, `func_8005B9C4`, `func_8005B72C` | `SsVabClose` on fixed slots (`func_8005B644(a0)`: slot `3*a0 + 1`; `func_8005B72C`: slots 1..15, plus reverb off and a pool reset), clearing their records |
| `func_8005BE84(n)` / `func_8005BF3C` | set reverb type `D_8009AD1C[n][0]` (`SpuClearReverbWorkArea`, `SsUtReverbOn`) and depth `2n + 1` / reverb off |

## One-shot sounds

`func_8005C650(id, volr, voll)` (4BC3C.c) queues sound `id`. It puts
`&D_8009AA70[id]`, a pair (slot, index), into the first free entry of the
24-entry pool `D_800EFB78` (`Unk800EFB78Entry`, include/game.h:840-849).

`func_8005C6D0` (4BC3C.c) flushes the pool once per frame. For each
queued entry whose slot is loaded, it takes the next voice from the running
cursor that `SpuGetKeyStatus` reports free. It reads the slot's sound table
word `w = table[index]` and calls:

```
SsUtKeyOnV(voice, slot, w & 0x7F, (w >> 7) & 0xF,
           (w >> 11) & 0x7F, (w >> 18) & 0x7F, voll, volr)
```

The arguments after `slot` are program, tone, note and fine pitch. A request
for slot 6 plays slot 3 when the two are the same pack. Every entry is
cleared as it is visited. `func_8005C650` has callers in 11 game TUs.

## CD audio mix

- `snd_SerialMixOn` (src/main/267A8.c:9) turns on the CD input with
  `SsSetSerialAttr(0, 0, 1)` and `SsSetSerialVol(0, 0x7F, 0x7F)`.
- `cdrom_SetMix(a, b, c, d)` (src/main/26730.c:13) sets the four
  CD-to-SPU attenuators (`CdlATV` `val0..val3`) at once through `CdMix`.
- `cdrom_StartMixFade(len, ...)` (src/main/26730.c:24) sets a target that
  `cdrom_UpdateAudio` reaches over `len` steps.

[file_io.md](file_io.md) describes how CD-DA and XA tracks are started and
how they stop, including XA's early stop inside its last 0x96 sectors.

## libsnd and libspu

These are Sony modules. Each file's top comment gives its span and evidence.
The parts the game relies on:

- **VAB management**: `SsVabOpenHead*`, `SsVabTransBody`, `SsVabFakeHead` /
  `SsVabFakeBody`, `SsVabClose`, `SsUtGetVBaddrInSB`. `SsVabOpenHeadWithMode`
  takes each body's SPU RAM with `SpuMalloc`
  (src/main/psxsdk/libsnd/vs_vh.c:115).
- **Voice manager** `_SsVm*`:
  - `_SsVmInit` calls `SpuInitMalloc(0x20, D_800F19D0)`
    (src/main/psxsdk/libsnd/vm_init.c:16).
  - `_SsVmVSetUp` selects the VAB header, program and tone tables
    (`_svm_vab_vh`, `_svm_vab_pg`, `_svm_vab_tn`).
  - `_SsVmKeyOnNow` / `_SsVmKeyOffNow` key voices on and off.
  - `SsUtKeyOnV` / `SsUtKeyOffV` key a chosen voice.
  - `func_800858D0` (src/main/psxsdk/libsnd/760D0.c:11) resets and keys off
    every voice up to `_SsVmMaxVoice`. The game calls it before most VAB
    changes.
- **Sequencer**: `_SsStart` (src/main/psxsdk/libsnd/ssstart.c:14) installs
  the tick handler `_snd_seq_tick_env.unk8`. Depending on the mode set by
  `SsSetTickMode`, it does this through `VSyncCallback` or through
  `InterruptCallback` after programming a root counter (`SetRCnt` with
  0xF2000002, or 0xF2000003 in mode 5). `func_80084CC0`
  (src/main/psxsdk/libsnd/midiread.c:89, Sony's `_SsGetSeqData`) reads one
  sequence event with MIDI-style status bytes and running status.
- **SPU RAM**: `SpuMalloc` / `SpuFree` / `_spu_gcSPU`
  (src/main/psxsdk/libspu/s_m_m.c, s_m_f.c, s_m_int.c) keep the allocation
  list `_spu_memList` (`_spu_AllocBlockNum`, `_spu_AllocLastNum`). They
  allocate SPU RAM, not voices.
- **Hardware**: `spu.c` is the register layer. `_spu_init` sets up the SPU
  and DMA channel 4, `_spu_RXX` points at 0x1F801C00, and `_spu_t` /
  `_spu_Fw` / `_spu_Fr` do transfers. `SpuSetKey`, `SpuGetKeyStatus`,
  `SpuSetReverb*`, `SpuSetCommonAttr`, `SpuSetTransferStartAddr`, `SpuWrite`,
  `SpuRead` and `SpuIsTransferCompleted` sit on top of it.
