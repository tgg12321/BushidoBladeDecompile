# psyz (PsyQ 4.0) systematic sweep vs the BB2 queue — 2026-08-18

Read-only session. Follow-on to `tmp/psyq_provenance.md`, which found 4 queue items by
name-keyed matching and recommended an address/size/structure sweep. This is that sweep.

**Result: 9 queue items have a version-correct matched C seed in psyz — 5 of them new.**

## Method

Join chain, address-keyed rather than name-keyed:

```
engine/queue.json func  ->  BB2 vaddr        (asm/funcs/<name>.s first glabel address)
                        ->  Sony symbol      (tmp/libscan/symbols.txt — XDEFs *and* statics)
                        ->  psyz matched body (name-equal) | psyz INCLUDE_ASM stub | absent
```

plus module attribution from `tmp/libscan/queue_hits.json` module ranges, and — for queue
items in the *non-verbatim* LIBSND/LIBSPU gaps — a masked byte near-match against every
size-compatible `.text` symbol of every PsyQ 4.0 module on disk (`tmp/libscan/psyq40/*.LIB`),
which identifies the Sony function even when BB2 links a different revision.

Every candidate pair then gets a structural sanity check against `asm/funcs/<name>.s`:
instruction count, back-edge count vs C loop count, `jal` targets vs C call names, and
shared distinctive immediates.

Scripts (all under `tmp/psyz_sweep/`): `parse_psyz.py`, `join.py`, `gapid.py`,
`structcheck.py`, `diffwords.py`, `emit.py`.

### Corpus counts (measured, not quoted)

| | count |
|---|---|
| psyz matched C bodies (`decomp/src/**`, non-`INCLUDE_ASM`) | **224** |
| psyz `INCLUDE_ASM` stubs | 2,050 |
| psyz bodies by lib | libgpu 75, libsnd 57, libspu 39, libcd 26, libetc 10, libapi 9, libgte 4, libcard 4 |
| BB2 queue items | 263 (222 active, 41 parked) |
| queue items inside a *verbatim* PsyQ 4.0 module | 25 |
| queue items in the LIBSND/LIBSPU non-verbatim gaps | 14 |
| **queue items with a psyz matched body** | **9** |

The earlier report's "~308 matched bodies" over-counts; the real figure is 224. Its
"92 census queue items" is also stale — that census ran against a 495-item queue on
2026-07-09; the queue is now 263 and most library items have been completed since.

Note: a naive one-pass brace scanner silently loses 21 bodies to brace-count drift inside
`libgpu/sys.c` — including `_clr`, `_dws`, `_drs`, `get_cs`, `get_ce`, `get_dx`, i.e. exactly
the BB2 statics that turned out to be new hits. `parse_psyz.py` uses a two-pass
def-line-boundary scan instead.

## Findings — ordered by queue impact (rules desc, then distance desc)

| # | BB2 func | addr | status | dist | rules | Sony module | psyz source | psyz func | conf | new? |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `_spu_gcSPU` | 0x800896A0 | active | 121 | **104** | LIBSPU/S_M_INT | `libspu/s_m_int.c` | `_spu_gcSPU` | HIGH | known |
| 2 | `func_8007CE0C` | 0x8007CE0C | active | 48 | **23** | LIBGPU/SYS | `libgpu/sys.c` | `_dws` | HIGH | **NEW** |
| 3 | `PutDispEnv` | 0x8007BC08 | active | **298** | 0 | LIBGPU/SYS | `libgpu/sys.c` | `PutDispEnv` | HIGH | known |
| 4 | `func_8007D048` | 0x8007D048 | active | **161** | 0 | LIBGPU/SYS | `libgpu/sys.c` | `_drs` | HIGH | **NEW** |
| 5 | `func_8007CBB0` | 0x8007CBB0 | active | **151** | 0 | LIBGPU/SYS | `libgpu/sys.c` | `_clr` | HIGH | **NEW** |
| 6 | `func_800889D4` | 0x800889D4 | active | **115** | 0 | LIBSPU/SPU | `libspu/spu.c` | `_spu_FwriteByIO` | HIGH | **NEW** |
| 7 | `note2pitch` | 0x80086B38 | active | 49 | 0 | LIBSND/VM_N2P | `libsnd/vm_n2p.c` | `note2pitch` | HIGH | known |
| 8 | `_spu_FiDMA` | 0x80088BA0 | active | 48 | 0 | LIBSPU/SPU | `libspu/spu.c` | `_spu_FiDMA` | HIGH | known |
| 9 | `func_800871D4` | 0x800871D4 | **parked** | 15 | 0 | LIBSND/VM_NOWOF | `libsnd/vm_nowof.c` | `_SsVmKeyOffNow` | MED | **NEW** |

psyz paths are relative to `tmp/psyq_prov/psyz/decomp/src/`. Machine-readable:
`tmp/psyz_sweep.csv`.

### Structural evidence per pair

| BB2 func | BB2 insns / back-edges / jals | psyz lines / loops / calls | verdict |
|---|---|---|---|
| `_spu_gcSPU` | 194 / 6 / — | 67 / 7 / — | 5-of-5 phase match already established in `tmp/psyq_provenance.md`; shares 0x2FFFFFFF, 0x40000000, 0x80000000 |
| `func_8007CE0C` | 143 / 2 / `func_8007DC68`, `func_8007DC9C` | 42 / 2 / `set_alarm`, `get_alarm`, `CLAMP`, `read` | back-edges == loops; `func_8007DC9C` is census-confirmed `get_alarm`; shares 0xB0000000 |
| `PutDispEnv` | 298 / 0 / `func_8007CA00`, `memcpy`, `sys_GetVideoMode` | 63 / 0 / `get_dx`, `memcpy`, `GetVideoMode`, `GPU_printf` | exact call-graph match (`func_8007CA00` == census `get_dx`); shares 0x260, 0x3FF, 0xFFF |
| `func_8007D048` | 161 / 3 / `func_8007DC68`, `func_8007DC9C` | 45 / 3 / `set_alarm`, `get_alarm`, `CLAMP` | back-edges == loops; shares the 0x01000200 / 0x04000003 DMA words |
| `func_8007CBB0` | 151 / 0 / `gpu_GetInfo`, `gpu_StartDmaList` | 39 / 0 / `_param`, `_cwc`, `write`, `CLAMP` | 8 shared GP0 command constants (0xE1/0xE3/0xE4/0xE5/0xE6000000, 0xE4FFFFFF, 0x7FF, 0xFFFFFF) |
| `func_800889D4` | 115 / 4 / `_spu_Fw1ts`, `printf` | 49 / 4 / `_spu_Fw1ts`, `printf`, `SPUR`, `SPUW` | back-edges == loops, jal set identical; shares 0x400, 0x7FF |
| `note2pitch` | 49 / 0 / — | 32 / 0 / — | BB2's 0x2AAAAAAB is the `/12` magic from psyz's `octave = semitones / 12`; psyz's 192-entry `pitch_table` is already in BB2 data at `D_800A26E4` |
| `_spu_FiDMA` | 48 / 1 / `DeliverEvent`, `_spu_Fw1ts` | 25 / 1 / `DeliverEvent`, `_spu_Fw1ts` | jal set identical; shares `DeliverEvent(0xF0000009, …)` |
| `func_800871D4` | 52 / 0 / — | 21 / 0 / — | masked byte near-match: **4 differing words out of 52** vs PsyQ 4.0 `LIBSND/VM_NOWOF:_SsVmKeyOffNow` |

## Transplant caveats

- **`_spu_gcSPU`** — psyz's phase-1 inner loop carries an empty `if (i) { // FAKE }` block;
  must clear `cheat-reviewer` under the fake-exception rules before commit, and may prove
  unnecessary under BB2's aspsx 2.34. Catalogued cc1 ICE risk (structured pointer-walk scans
  SIGSEGV cc1 here); the array-indexed psyz form may sidestep it but this is unmeasured.
  Globals: `_spu_memList` <- `D_800A2D40` (a *pointer* object), `_spu_AllocLastNum` <-
  `D_800A2D3C`. Do **not** cache the loop bound in a local.
- **`func_8007CBB0` (`_clr`)** — psyz writes a packet **aggregate**; BB2's asm addresses the
  same memory as 13 separate per-word globals `D_800F1858..D_800F1888` (splat's split). This
  is the [[split-scalars-hide-aggregate]] / [[splat-symbol-names-are-not-evidence]] situation:
  the aggregate is the right C, the per-word `D_` names are not evidence against it.
- **`func_800889D4`, `_spu_FiDMA`** — psyz accesses the SPU register file through `SPUR`/`SPUW`
  macros over a `union SpuUnion* _spu_RXX`; BB2 spells the same accesses as raw `D_` globals
  (`D_800A2CDC`, `D_800A2CF4`, …). psyz's `timeout > 0xF00` compiles to BB2's `sltiu 0xF01`.
- **`PutDispEnv`, `_dws`, `_drs`, `_clr`** — depend on psyz's `libgpu` macro set (`CLAMP`,
  `RECT_EQ`, `range`, `ctl`, `CMD_*`) and on sibling statics that are themselves BB2 queue
  items or already-named BB2 functions (`get_dx` = `func_8007CA00`, `get_alarm` =
  `func_8007DC9C`, `set_alarm` = `func_8007DC68`). Transplanting `_dws`/`_drs` and
  `get_alarm`/`set_alarm` as a unit is likely more productive than one at a time.
- **`note2pitch`** — this is LIBSND/VM_N2P's table lookup, **not** LIBSPU's `_spu_note2pitch`
  curve-scan (a separate, still-unmatched queue item). Do not confuse them.
- **`func_800871D4`** — currently `parked` as OWNER-ACCEPTED INCOMPLETE (standing ruling
  2026-07-27, `docs/grind/decisions.md`), so it is not queue-active work; and the struct
  divergence below applies.

## Where BB2 diverges from stock PsyQ 4.0

1. **LIBSND `struct SpuVoice` grew 2 bytes.** Both non-verbatim near-matches in LIBSND
   (`_SsVmKeyOffNow` @0x800871D4 and `SsUtKeyOnV` @0x80085A40) differ from stock 4.0 in the
   *same four instructions*, and only those — the `_svm_voice[]` index strength-reduction:

   | | stock PsyQ 4.0 | BB2 |
   |---|---|---|
   | | `sll v0,v1,1` → 2v | `sll v0,v1,3` → 8v |
   | | `addu v0,v0,v1` → 3v | `subu v0,v0,v1` → 7v |
   | | `sll v0,v0,2` → 12v | `sll v0,v0,2` → 28v |
   | | `addu v0,v0,v1` → 13v | `subu v0,v0,v1` → 27v |
   | | `sll v0,v0,2` → **52v** | `sll v0,v0,1` → **54v** |

   psyz's `struct SpuVoice` (`src/libsnd/libsnd_private.h:115`) ends at 0x34 = **52 bytes**;
   BB2's linked LIBSND uses **54**. Any psyz `libsnd` body that indexes `_svm_voice[]` needs a
   54-byte struct, not psyz's. This is a concrete, cheap correction — and it is the *only*
   difference in these two functions, which supports the census's "BB2 links a newer
   (4.1–4.3-era) LIBSND/LIBSPU build" conclusion with an actual mechanism rather than an
   inference.

2. **The rest of the LIBSND/LIBSPU gap is genuinely a different build.** Of the 14 queue items
   in the non-verbatim gaps, only 2 (the pair above) near-match any PsyQ 4.0 symbol at all.
   The other 12 — `func_80083794`, `func_800841E0`, `func_80084500`, `func_800858D0`,
   `func_800861BC`, `func_800863DC`, `func_80086818`, `func_80086CF8`, `func_800872A4`,
   `func_80087770`, `func_80089A48`, `func_8008B488` — have **no** size-compatible 4.0 symbol
   under a 35 %-differing-word threshold. Size-only "matches" for these are coincidence
   (differing-word ratios ≈ 1.0). They need self-decomp; psyz cannot seed them.

3. **Verbatim regions have no divergence** — that was already ground truth
   (`memory/closer/libcd-groundtruth.md`); nothing here contradicts it.

## Confirmed dead ends (in-library queue items with NO psyz body)

All 17 remaining queue items inside verbatim 4.0 modules resolve to a Sony symbol that psyz
leaves as `INCLUDE_ASM`. No seed exists at the correct version for any of them:

`CD_ready` (42 rules), `_spu_note2pitch` (37), `func_8007D3F8`/`_addque2` (36),
`CD_datasync` (15), `func_8008BF04`/`SioSyncroRead` (6), `CD_sync` (5),
`func_8007DC9C`/`get_alarm` (4), `func_8008C1E8`/`SioSyncroWrite` (1),
`_comb_control` (d=763), `sprintf` (535), `prnt` (418), `func_80080828`/`getintr` (354),
`CD_cw` (263), `func_8007D6D8`/`_exeque` (187), `_spu_pitch2note` (64), `_spu_Fr_` (43),
`_spu_2pitch` (36).

This *strengthens* the prior report's ruling on `CD_ready` and `_spu_note2pitch`: the
world's only PsyQ-4.0-targeted decomp has not cracked them either.

## Artifacts

| path | contents |
|---|---|
| `tmp/psyz_sweep.csv` | the 9 findings, machine-readable |
| `tmp/psyz_sweep/psyz_funcs.json` | all 224 psyz bodies + 2,050 stubs, with structure metrics |
| `tmp/psyz_sweep/join.json` | all 263 queue items with addr / lib / module / Sony symbol |
| `tmp/psyz_sweep/gapid.json` | near-match ranking for the 14 gap items |
| `tmp/psyz_sweep/structcheck.json` | per-pair structural comparison |
