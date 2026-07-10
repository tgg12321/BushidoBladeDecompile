# LIBSND/LIBSPU version hunt for SLUS_006.63 (Bushido Blade 2)

Date: 2026-07-10. Follow-up to `tmp/libscan/report.md` (PsyQ 4.0 census, 67 verbatim
LIBSND/LIBSPU placements, ~15KB unmatched gaps, 26 queue items unproven).
Method throughout: the proven masked byte compare (HI16/LO16 imm16, REL26 low-26,
FULL32 masked; mismatch counted over fully-unmasked words only), via copies of
`tmp/libscan/scan.py` + new `gap_hunt.py` / `gap_tile.py` / `gap_best.py` here.

## Verdict

**BB2 does NOT link PsyQ 4.1, 4.2, 4.3 or 4.4 sound libraries.** It links a
**4.0-lineage interim build of LIBSND/LIBSPU compiled between 1997-06-06 and
1997-09-22** that was never on any archived SDK disc. Evidence:

| lib build (module datestamps) | source | LIBSND+LIBSPU verbatim vs BB2 | UT_KEYV (litmus, @0x80085A40) |
|---|---|---|---|
| 4.0 May-20-1997 ("40J", sozud mirror = Japan DTL-S2320 layout) | tmp/libscan/psyq40 | **67** | 259 words, **4 mismatch** (near) |
| 4.0 **Jun-06-1997** ("40U", USA DTL-S2002 "Release 2.0" disc) | psyq40u/ | **68** (adds SSSTOP) | 259 words, **4 mismatch** (near) |
| 4.1 Sep-22-1997 (Japan DTL-S2330 disc) | psyq41/ | 13 (stubs only) | 248 words, no match (refactored) |
| 4.2 Jan-21-1998 | psyq42/LIB | 13 | 248 words, no match |
| 4.3 (FoxdieTeam mirror) | psyq43/ | 16 | 248 words, no match |
| 4.4 (FoxdieTeam mirror) | psyq44/ | 12 | 244 words, no match |

- 4.1+ rebuilt the whole sound library with different codegen: even modules that are
  bit-identical between 4.0 and BB2 stop matching in 4.1+. BB2 UT_KEYV keeps the
  4.0 shape (259 words) with exactly 4 opcode diffs (addu->subu x2, sll-shift x2) -- the
  "Ut functions volume inconsistency" fix that the 4.1 changelog (`libchg_e_41.txt`)
  says was applied between 4.0 and 4.1.
- The Jun-06 build (USA R2.0 disc) is strictly closer than May-20: **SSSTOP flips from
  no-match to VERBATIM @0x80085270** (118 words), proving the BB2 build >= Jun-06-1997.
- Gap contents best-match different later versions per module (see below), exactly the
  profile of an intermediate source state. Sony distributed such interim library
  updates through developer support channels (BBS/web patch zips, cf. `LIB/42PATCH`
  inside the 4.2 release); none for the Jun-Sep 1997 window is in any public archive
  reached below.

## New verbatim result (census-grade)

| BB2 vaddr | module | words | Sony symbols | queue item proven |
|---|---|---|---|---|
| 0x80085270..0x80085448 | LIBSND/SSSTOP (4.0 Jun-06 build) | 118 | _SsSndStop (+0x000), SsSeqStop (+0x184), SsSepStop (+0x1AC) | **func_80085270 = _SsSndStop** |

This closes the former 0x80085270..0x8008541C gap; 1 of the 26 gap queue items is now
**proven Sony** (bit-verbatim). Scan record: `matches40u.json`.

## Near-proven identifications in the remaining gaps (small mismatch, exact position)

Mismatch = differing fully-unmasked words / total fully-unmasked words.

| BB2 vaddr | module (best version) | mism | Sony function(s) | queue item |
|---|---|---|---|---|
| 0x80085A40 | LIBSND/UT_KEYV (4.0) | 4/150 (3%) | SsUtKeyOnV @0x80085A40, SsUtKeyOffV @0x80085DD4 | **AllocBukiRmd = SsUtKeyOnV** |
| 0x800871D4 | LIBSND/VM_NOWOF (4.1) | 1/27 (4%) | _SsVmKeyOffNow | **func_800871D4 = _SsVmKeyOffNow** |
| 0x80085FD8 | LIBSND/UT_VVOL (4.3) | 1/14 (7%) | SsUtGetDetVVol | **func_80085FD8 = SsUtGetDetVVol** |
| 0x80086080 | LIBSND/UT_VVOL3 (4.3) | 2/42 (5%) | SsUtGetVVol | **func_80086080 = SsUtGetVVol** |
| 0x80087D10 | LIBSND/VM_SEQ_2 (4.3) | 2/28 (7%) | _SsVmGetSeqLVol @0x80087D10, _SsVmGetSeqRVol @0x80087D58 | **func_80087D10 / func_80087D58** |
| 0x80086018 | LIBSND/UT_VVOL2 (4.3) | 5/19 (26%) | SsUtSetDetVVol | **func_80086014 = SsUtSetDetVVol** |
| 0x80089A54 | LIBSPU/S_SAV (4.1) | 24/133 (18%) | _SpuSetAnyVoice | **coli_HitPauseKatana_2 @0x80089A48 = _SpuSetAnyVoice** |
| 0x800861B8 | LIBSND/VM_ALOC2 (4.0) | 21/76 (28%) | _SsVmDoAllocate | **tslGlobalMemFree_800861BC = _SsVmDoAllocate** |

## Full gap map with structural (link-order + size) identifications

Gaps recomputed from union of 4.0J+4.0U verbatim placements (`gap_best.json`).
Confidence: PROVEN (verbatim) > NEAR (table above) > PROBABLE (link order + size fit
+ partial byte evidence) > WEAK.

| gap | bytes | contents |
|---|---|---|
| 0x80083698..0x8008386C, 0x8008387C..0x80083954 | 684 | ang_hosei @0x800836C8, motion_Open @0x80083794, motion_Close @0x80083804 -- **likely game-side sound glue**, no library module fits (best hits are 4-8-word stub shapes; a 1/4-word hit on the new-in-4.1 SSVM SsSetVoiceMask @0x800836AC is too small to credit) |
| 0x800841E0..0x800848AC | 1740 | **CRES + DECRES** (4.1 sizes 196w+232w = 1712B fit): func_800841E0 = **_SsSndCrescendo** (PROBABLE), func_80084500 = **_SsSndDecrescendo** (PROBABLE) -- exactly the modules the 4.1 changelog says were fixed after 4.0 |
| 0x80084974..0x80085064 | 1776 | **MIDIREAD** (4.1 build is exactly 444w = 1776B): saTan4GaugeInit @0x80084974 = **_SsSeqPlay** (PROBABLE), func_80084A7C = **_SsSeqGetEof** (PROBABLE; 4.0 offset +264 = 0x80084A7C exactly), _SsGetSeqData ~ 0x80084CB8 |
| 0x800858D0..0x800859F0 | 288 | **UT_AKO** (69w = 276B fits): title_mv_exec2 @0x800858D0 = **SsUtAllKeyOff** (PROBABLE) |
| 0x80085A40..0x80085E4C | 1036 | **UT_KEYV** fills it exactly (259w = 1036B) -- NEAR-PROVEN, see above |
| 0x80085FD8..0x800863CC | 1012 | UT_VVOL family + VM_ALOC2: SsUtGetDetVVol (NEAR), SsUtSetDetVVol (NEAR), SsUtGetVVol (NEAR), func_80086130 = **SsUtSetVVol** (PROBABLE, UT_VVOL4 in link order; byte fit weak), _SsVmDoAllocate (NEAR) |
| 0x800863DC..0x80086B38 | 1884 | **VM_F + VM_INIT**: action_CheckHitZangeki @0x800863DC = **_SsVmFlush** (PROBABLE), md_game_end @0x80086818 = **_SsVmInit** (PROBABLE -- 4.0 _SsVmInit best-aligns at exactly 0x80086818, and 196w lands at VM_N2P @0x80086B38) |
| 0x80086CF8..0x80087E3C | 4420 | VM sequencer voice manager: func_80086CF8 = **_SsVmKeyOn (VM_KEY)** (PROBABLE, link order), _SsVmKeyOffNow (NEAR), func_800872A4 = **_SsVmKeyOnNow (VM_NOWON)** (PROBABLE, 4.1 places @0x800872B4), func_80087770 = **_SsVmSetVol / _SsVmSetSeqVol region (VM_VOL/VM_SEQ)** (WEAK), func_80087CAC = **_SsVmSeqKeyOff or _SsVmGetSeqVol (VM_SEQ tail)** (WEAK), _SsVmGetSeqLVol/_SsVmGetSeqRVol (NEAR) |
| 0x80089A48..0x80089D10 | 712 | **S_SAV + S_GAV** (4.1 S_SAV 176w = 704B fills it): coli_HitPauseKatana_2 = **_SpuSetAnyVoice** (NEAR), _SpuGetAnyVoice at tail (PROBABLE) |
| 0x8008B488..0x8008BA94 | 1548 | **S_SVA** (4.1 SpuSetVoiceAttr 384w = 1536B fits; 50% word drift): saTan1MainJump @0x8008B488 = **SpuSetVoiceAttr** (PROBABLE) |

## Queue-item scoreboard (the former 26)

- **Proven Sony now**: func_80085270 (= _SsSndStop / SSSTOP, verbatim vs 4.0 Jun-06).
- **Near-proven Sony (small opcode drift from an adjacent lib build)**: AllocBukiRmd
  (SsUtKeyOnV), func_800871D4 (_SsVmKeyOffNow), func_80085FD8 (SsUtGetDetVVol),
  func_80086014 (SsUtSetDetVVol), func_80086080 (SsUtGetVVol), func_80087D10
  (_SsVmGetSeqLVol), func_80087D58 (_SsVmGetSeqRVol), coli_HitPauseKatana_2
  (_SpuSetAnyVoice), tslGlobalMemFree_800861BC (_SsVmDoAllocate) -- 9 items.
- **Probable Sony with name**: func_800841E0 (_SsSndCrescendo), func_80084500
  (_SsSndDecrescendo), saTan4GaugeInit (_SsSeqPlay), func_80084A7C (_SsSeqGetEof),
  title_mv_exec2 (SsUtAllKeyOff), func_80086130 (SsUtSetVVol), action_CheckHitZangeki
  (_SsVmFlush), md_game_end (_SsVmInit), func_80086CF8 (_SsVmKeyOn), func_800872A4
  (_SsVmKeyOnNow), saTan1MainJump (SpuSetVoiceAttr) -- 11 items.
- **Weak/unnamed (Sony sequencer region, exact function uncertain)**: func_80087770,
  func_80087CAC -- 2 items.
- **Reclassified as likely game glue (not Sony)**: ang_hosei, motion_Open,
  motion_Close -- 3 items.

## Acquisition record

Obtained and scanned (all under `tmp/libsnd_hunt/`):
- **4.2** LIBSND/LIBSPU (build 1998-01-21): archive.org item
  `play-station-programmer-tool-runtime-library-version-4.2.7z` (383KB, libs+includes only).
- **4.3, 4.4**: `github.com/FoxdieTeam/psyq_sdk` (`psyq_4.3/lib/libsnd.lib` etc.).
- **4.1** (build 1997-09-22): archive.org `ps1_sdks` item, `Programmer Tool - Runtime
  Library Version 4.1 (Japan)_DTL-S2330_redump.zip` -- Track 1 pulled via archive.org
  on-the-fly zip extraction, ISO9660-parsed with `iso_extract.py` (raw 2352 MODE2 bins).
- **4.0 USA "Release 2.0"** (LIBSND build 1997-06-06, LIBSPU 1997-05-20): same item,
  `Programmer Tools - Run-time Library 4.0 (USA) (Release 2.0)_DTL-S2002_redump.zip`.
- sozud/psy-q has only 3.3/3.5/3.6/4.0 (no 4.1+); its 4.0 = May-20-1997 builds.
- Checked and empty-handed: arthus.net Psy-Q index + addons (sound/ = samples only),
  PSXSOUND.zip (tools only), PSYQ_SDK.zip (contains 4.3- and 4.4-vintage libs, verified
  via ranged central-directory read), DTL-S3030/3035/3040 (artist-tool/GNU discs),
  ps1_sdks Floppies.rar (1995-era ISA-board libs), CodeWarrior for PlayStation R3
  (no Sony libs bundled), lab313ru/psx_psyq_signatures (no 4.0<->4.1 intermediate),
  mkst/psy-q-decomp (targets 3.5), psxdev.net (site defunct).

### Best remaining acquisition leads (for the owner)

1. **Pool7 mirror (PSXDEV Discord)**: https://mega.nz/folder/mchwFBSB#UlEdCSFJd3Np2e86maYBSA
   (referenced by psx.arthus.net/sdk/Psy-Q/sources) -- may hold BBS-era interim lib patches.
2. Sony 1997 developer-support BBS/web patch zips ("libsnd/libspu update", Jul-Aug 1997,
   analogous to the `42PATCH`/`J421*.ZIP` zips inside the 4.2 release) -- ask in PSXDEV
   Discord / Assembler-adjacent archives; this exact artifact would 100%-match UT_KEYV
   and the remaining gaps.
3. Contemporary Japanese titles built Jul-Sep 1997 with matched decomps could confirm the
   same interim build exists in the wild (evidence only, not an acquisition).

## Files

- `matches40u.json`, `matches41.json`, `matches42.json`, `matches43.json`, `matches44.json`
  -- full masked-compare scans per version (schema of tmp/libscan/matches.json).
- `gap_best.json` / `gap_best_out.txt` -- best placement per module per version, plus 4.0
  link order annotated with status.
- `gap_matches.json`, `gap_tiling.json` -- tolerant gap scans.
- `psyq40u/ psyq41/ psyq42/ psyq43/ psyq44/` -- the acquired LIBSND/LIBSPU binaries.
- `iso_extract.py`, `iso_walk.py`, `zip_cd.py`, `scan.py` (copy), `gap_*.py`, `queue_gaps.py`.
- Disc images deleted after lib extraction (re-fetch via URLs above if needed).
