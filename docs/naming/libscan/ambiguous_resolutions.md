# Disambiguation of the 10 AMBIGUOUS libscan rows

> Archived 2026-08-07 from `tmp/ambiguous_ties/resolutions.md` (task #23 — `tmp/` is
> unbacked). The referenced helper scripts (`dump_mod.py`, `findsym.py`, `offs.py`,
> `bysize.py`) remain in `tmp/ambiguous_ties/`; the two scanner filters proposed at the
> end are now IMPLEMENTED in `tools/libscan/manifest.py` and independently reproduce
> every verdict below — see the verification addendum at the end of this file.

Method: three independent evidence channels, applied in this order.

1. **Module relocation targets** (strongest). Every tied module is a thin wrapper whose
   `.text` contains exactly one `type=74` (REL26 / `jal`) relocation. The masked scan blanks
   that word, which is precisely why the modules tie. Un-masking it — i.e. asking *which
   external symbol does this module call* and *what does the placed code actually jal to* —
   separates them, because the two candidates call **different** library functions and those
   callees are themselves independently placed.
2. **Contiguity / layout**. Placed modules form unbroken runs; a candidate that would force an
   overlap with an already-placed neighbour is impossible.
3. **Reference reachability**. A library module is only pulled by the linker if something
   references a symbol it defines. Zero callers + zero data references ⇒ not a linked module.

Tooling written for this: `tmp/ambiguous_ties/dump_mod.py`, `findsym.py`, `offs.py`, `bysize.py`
(all read `tmp/libscan/psyq40/*.LIB` through `tmp/closer/psyq_lib.py`).

---

## Verdicts

| addr | verdict | name |
|---|---|---|
| 0x80046B20 | **RESOLVED — reject both** | not Sony code; keep `snd_StopSystemSe_80046B20` |
| 0x80083A18 | **RESOLVED** | `SsInit` |
| 0x80083B30 | **RESOLVED** | `SsQuit` |
| 0x80084948 | **RESOLVED** | `_SsSndPlay` (the real one) |
| 0x8008541C | **RESOLVED — not _SsSndPlay** | `SsSepStop` |
| 0x80085F98 | **RESOLVED** | `SsUtReverbOff` |
| 0x80085FB8 | **RESOLVED** | `SsUtReverbOn` |
| 0x800885AC | **RESOLVED** | `SpuInit` |
| 0x8008AD64 | **RESOLVED** | `SpuRead` |
| 0x8008ADC4 | **RESOLVED** | `SpuWrite` |

No row remains STILL-AMBIGUOUS.

---

### 0x80046B20 — GsGetVcount vs GsClearVcount → **neither**

Body: `jal func_80078B04` with `$a0 = 0xF2000001` (= `RCntCNT1`).

* GS_008 (`GsGetVcount`) references `GetRCnt`; GS_009 (`GsClearVcount`) references `ResetRCnt`.
  `func_80078B04` indexes a counter table and returns `lhu` — it **reads**, never writes, so the
  callee is `GetRCnt`. That eliminates `GsClearVcount`.
* But `GsGetVcount` is eliminated too, by reachability: **0x80046B20 has zero `jal` callers and
  zero word-sized data references anywhere in the 606 KB image**. A standalone 9-word archive
  member defining only `GsGetVcount` cannot be pulled into the link by nothing.
* Corroborating: LIBGS places 7 of 201 modules and every one of those is a 4–9-word stub with
  8–23 candidate addresses — i.e. LIBGS is *not linked at all*. Every genuinely-placed LIBSND /
  LIBSPU / LIBAPI module sits at ≥ 0x80078948; 0x80046B20 sits inside the game's own sound text.

**Conclusion:** BB2's own (dead, never-called) one-liner `x() { return GetRCnt(RCntCNT1); }`,
which is byte-identical to Sony's `GsGetVcount` because it is the same one-liner. Keep the
existing alias. Do not rename.

**Same reasoning very likely applies to 0x800469A0 (`_SpuCallback`, LIBSPU/S_CB)** — currently
classified `CONTRADICTED_ALIAS`, so not being renamed, but the row is also *wrong on the merits*:
S_CB's single external reference is `InterruptCallback`, whereas the placed code calls
`func_80045510` (game text, arg `9`), and 0x800469A0 likewise has zero callers and zero data
references. Recommend demoting it from `libscan-verbatim` to rejected.

### 0x80083A18 — SsInit vs SsInitHot → **SsInit**

Both modules are 48 bytes with three `jal` relocs at 0x08/0x10/0x18. They differ only in the
second: SSINIT_C calls `SpuInit`, SSINIT_H calls `SpuInitHot`.

Placed code: `jal 0x80082AC0` (`ResetCallback`), `jal 0x800885AC`, `jal 0x80083A48`.
0x800885AC is resolved below as `SpuInit`, and 0x80083A48 is LIBSND/SSINIT (`_SsInit`, 58 words,
independently placed). Layout confirms: SSEND 0x80083954 +49w ends exactly at 0x80083A18,
which +12w ends exactly at 0x80083A48 where `_SsInit` begins.

### 0x80083B30 — GsSetProjection / SendPAD / SsQuit / SsSetNoiseOff → **SsQuit**

This is an 8-word `f(void){ g(); }` stub, hence the 23-address 4-way tie — layout and callee
settle it. SSQUIT's one external reference is `SpuQuit`; the placed code calls `func_800892F8`,
which is LIBSPU/S_Q (`SpuQuit`), independently placed at exactly 0x800892F8. Layout: SSINIT ends
at 0x80083B30, and 0x80083B30 + 8 words = 0x80083B50 = SSSATTR's placement — a gapless fit inside
the LIBSND run. `GsSetProjection`, `SendPAD` and `SsSetNoiseOff` all take arguments the placed
code never touches, and LIBGS is not linked.

### 0x80084948 vs 0x8008541C — the `_SsSndPlay` double placement

**Both placements are real functions, but only 0x80084948 is `_SsSndPlay`.** PLAY is an 11-word
`f(short,short)` forwarder with a masked `jal`, so it matches *any* two-short forwarder.

PLAY (0x2C bytes) defines `_SsSndPlay` at offset 0 and calls `_SsSeqPlay`, which is defined at
**offset 0 of LIBSND/MIDIREAD, a 0x6F4-byte (1780-byte) module**.

* If PLAY is at 0x80084948, `_SsSeqPlay` = 0x80084974, so MIDIREAD spans 0x80084974–0x80085068.
  The next placed module is MIDITIME at 0x80085064 — a 4-byte discrepancy, consistent with the
  known LIBSND version skew (MIDIREAD itself did not byte-match). **Fits.**
* If PLAY is at 0x8008541C, `_SsSeqPlay` = 0x80085270 and MIDIREAD would span to 0x80085964,
  swallowing SSSV (0x80085448) and SSTICK (0x80085544), both independently placed. **Impossible.**

0x8008541C is instead the tail wrapper of LIBSND/**SSSTOP**, which occupies 0x80085270–0x80085448
(right after REPLAY, right before SSSV):

| SSSTOP xdef | lib offset | placed addr | placed offset |
|---|---|---|---|
| `_SsSndStop` | 0 | 0x80085270 | 0 |
| `SsSeqStop` | 396 (0x18C) | 0x800853F4 | 0x184 |
| `SsSepStop` | 436 (0x1B4) | 0x8008541C | 0x1AC |
| (end) | 480 (0x1E0) | 0x80085448 | 0x1D8 |

A uniform 8-byte shortfall in `_SsSndStop` (version skew) with the two wrappers identical.
Behaviour matches exactly: 0x800853F4 sign-extends one short and calls the core with `$a1 = 0`
(`SsSeqStop(seq) → _SsSndStop(seq, 0)`); 0x8008541C sign-extends two shorts and forwards both
(`SsSepStop(sep, seq)`). It is the two-short forwarder shape that produced the false PLAY hit.

Bonus identifications: 0x80085270 = `_SsSndStop`, 0x800853F4 = `SsSeqStop`, 0x80084974 = `_SsSeqPlay`.

### 0x80085F98 / 0x800885AC — SpuInit vs SsUtReverbOff, and 0x80085FB8 — SpuInitHot vs SsUtReverbOn

UT_ROFF/UT_RON call `SpuSetReverb(0)` / `SpuSetReverb(1)`; S_I/S_IH call `_SpuInit(0)` / `_SpuInit(1)`.
All four are 8-word `f(void){ g(const); }` stubs — identical once the `jal` is masked.

* 0x80085F98 (`$a0 = 0`) and 0x80085FB8 (`$a0 = 1`) both call **0x80089D60**, which is LIBSPU/S_SR
  = `SpuSetReverb`, independently placed there. Semantics confirm: with arg 0 it reads offset
  0x1AA of the SPU register block (SPUCNT) and does `andi 0xFF7F`, clearing bit 7 — the reverb
  enable bit. ⇒ **0x80085F98 = SsUtReverbOff, 0x80085FB8 = SsUtReverbOn.**
  Layout: UT_REV ends at 0x80085F98, +8w = 0x80085FB8, +8w = 0x80085FD8.
* 0x800885AC (`$a0 = 0`) calls **0x800885CC**, which is LIBSPU/S_INI (`_SpuInit`, 372-byte module),
  independently placed there. ⇒ **0x800885AC = SpuInit.**
  Layout: VS_VTC (`SsVabTransCompleted`) ends at 0x800885AC, +8w = 0x800885CC = `_SpuInit`.
  Independently corroborated by SSINIT_C's reloc chain at 0x80083A18 (above), which requires
  `SpuInit` to be at exactly 0x800885AC.

`SpuInitHot` and `SsInitHot` are simply not linked into this build.

### 0x8008AD64 / 0x8008ADC4 — SpuRead vs SpuWrite

Three independent channels, all agreeing:

1. **Reloc.** S_R's `jal` at text offset 0x28 targets `_spu_Fr`; S_W's targets `_spu_Fw`.
   0x8008AD64+0x28 = `jal func_80089024`; 0x8008ADC4+0x28 = `jal func_80088F9C`.
   `func_80088F9C` branches on a global (`_spu_transferMode`) and, when set, diverts to
   `func_800889D4`, which writes the transfer address to SPU register offset 0x1A6 and pushes
   data through the FIFO — the CPU→SPU I/O path. `SpuSetTransferMode(SPU_TRANSFER_BY_IO)` only
   affects writes; there is no I/O-mode read. ⇒ `func_80088F9C` = `_spu_Fw`.
2. **Layout + archive order.** S_GKS (`SpuGetKeyStatus`) ends at 0x8008AD64; +24w = 0x8008ADC4;
   +24w = 0x8008AE24 = S_STSA (`SpuSetTransferStartAddr`). The LIBSPU run is laid out in archive
   order over this whole stretch (S_GKS idx49 → S_R idx52 → S_W idx53 → S_STSA idx55 → S_STM 57
   → S_ITC 60 → S_IT 62 → S_SCA 65), so the lower address is S_R.
3. **Shared globals.** Both wrappers reference D_800A2D14 (`_spu_transferCallback`) and
   D_800A2D10 (`_spu_inTransfer`) at exactly the reloc offsets 0x30/0x34 and 0x44/0x48 that S_R
   and S_W both declare — a consistency check that the placement is genuine.

⇒ **0x8008AD64 = SpuRead, 0x8008ADC4 = SpuWrite.**

Bonus: `_spu_Fr` = 0x80089024, `_spu_Fw` = 0x80088F9C, `_spu_transferCallback` = D_800A2D14,
`_spu_inTransfer` = D_800A2D10.

---

## Note on the matcher

Every tie in this set came from modules of 4–12 words whose only distinguishing word is a
masked `jal`. Two cheap filters would have prevented most of them:

* **Reachability filter** — reject a placement whose address has no `jal` caller and no word-sized
  data reference (kills 0x80046B20 and 0x800469A0).
* **Reloc-target consistency** — after placing a module, check that each REL26 reloc's actual
  target is consistent with the referenced symbol's own placement (resolves every remaining tie
  here mechanically, and catches the 0x8008541C false positive).

---

## Verification addendum (task #23 prep, 2026-08-07)

Both proposed filters are now implemented in `tools/libscan/manifest.py` (reachability =
filter 1, reloc-target consistency = filter 2; `BB2_EXE` / `LIBSCAN_OUT` env overrides
added for sandbox runs). A sandboxed re-run against the current tree **independently
reproduced every verdict in this document**:

- `REJECTED_UNREACHABLE`: LIBSPU/S_CB @0x800469A0, LIBGS/GS_008 and GS_009 @0x80046B20 —
  zero j/jal callers and zero word-sized data refs to any exported address.
- `RESOLVED_RELOC_TARGET` (exactly one candidate whose REL26 callees all land on their
  placed addresses): 0x80083A18 SsInit, 0x80083B30 SsQuit, 0x80085F98 SsUtReverbOff,
  0x80085FB8 SsUtReverbOn, 0x800885AC SpuInit, 0x8008AD64 SpuRead, 0x8008ADC4 SpuWrite.
- The PLAY double placement stays mechanically undecided (its callee `_SsSeqPlay` is in
  the non-verbatim MIDIREAD module, so filter 2 has nothing to check); the layout
  argument above remains the deciding evidence for 0x80084948/_SsSndPlay and
  0x8008541C/SsSepStop. Filter 1 does flag the 0x8008541C placement as reference-free,
  consistent with SsSepStop being dead code linked in as part of SSSTOP.

Spot-verifications against the worktree at prep time (asm/funcs + src):

- `func_80083A18` body is `jal ResetCallback; jal spu_Init(=0x800885AC); jal _SsInit` —
  exactly SSINIT_C's reloc chain.
- `func_80083B30` body is `jal SpuQuit` (0x800892F8).
- `func_80084948` sign-extends two shorts and forwards to `func_80084974` (PLAY shape);
  `src/main.c:690/694` confirm `func_800853F4(a0){func_80085270(a0,0)}` = SsSeqStop and
  `func_8008541C(a0,a1){func_80085270(a0,a1)}` = SsSepStop.
- `func_80085F98`/`func_80085FB8` call `SpuSetReverb(0)`/`SpuSetReverb(1)`;
  `func_800885AC` calls `_SpuInit(0)`; `func_8008AD64`/`func_8008ADC4` call
  `_spu_Fr`/`_spu_Fw` after capping the size arg at 0x7EFF0.
- `_SpuCallback` (0x800469A0) calls `func_80045510(9, a0)` — game text, not
  `InterruptCallback`; its only occurrence outside its own definition is nothing (dead).
- `func_80046B20` calls `GetRCnt(0xF2000001)` and has no reference anywhere; its C
  definition is `snd_StopSystemSe` (src/sound.c:238) — keep, rename nothing.

One correction to the handoff's phrasing: "0x80082AC0 is likely ResetCallback (current
name wrong)" was ALREADY RESOLVED by the fe40a52b wave — the LIBETC/INTR module is placed
verbatim at 0x80082AC0 with XDEF `ResetCallback` @ +0, the body dispatches through
`[D_800A2600]+0xC` in the INTR vector-table idiom, and `asm/funcs/ResetCallback.s` exists
today. No addendum row is needed for it.

Bonus data identifications recorded here but NOT part of the function-name wave (data
symbols are outside `tools/naming_wave.py`'s scope): `D_800A2D14` = `_spu_transferCallback`,
`D_800A2D10` = `_spu_inTransfer` (S_R and S_W both declare them at reloc offsets
0x30/0x34 and 0x44/0x48, matching the placed code). See
`docs/naming/data_evidence/D_800A2D10.md` / `D_800A2D14.md`.
