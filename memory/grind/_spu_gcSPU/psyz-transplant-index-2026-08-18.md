# psyz -> BB2 transplant drafts — 2026-08-18

Read-only session. Nothing under `src/`, `asm/`, or any symbol/config file was
touched; nothing was compiled, sandboxed, or built. These are **drafts to be
measured**, not verified matches.

Upstream context: `tmp/psyz_sweep.md` (the 9 seeded queue items) and
`tmp/psyq_provenance.md` (the `_spu_gcSPU` deep analysis). psyz is
`tmp/psyq_prov/psyz/` — PsyQ 4.0, MIT, GCC 2.7.2, the same Sony objects BB2
links verbatim.

## Files

| draft | BB2 target | lives in | today | dist | rules | psyz source |
|---|---|---|---|---|---|---|
| `_spu_gcSPU__nofake.c` | `_spu_gcSPU` @0x800896A0 | `src/main.c:2009` | hand-written C | 121 | **104** | `libspu/s_m_int.c` |
| `_spu_gcSPU__fake.c` | same — variant B, do not try first | | | | | same |
| `_libgpu_sys_shared_decls.h` | preamble for the three below | `src/display.c` | — | — | — | `libgpu/sys.c` |
| `func_8007CBB0__clr.c` | `func_8007CBB0` @0x8007CBB0 | `src/display.c:682` | `INCLUDE_ASM` | 151 | 0 | `libgpu/sys.c:706` |
| `func_8007CE0C__dws.c` | `func_8007CE0C` @0x8007CE0C | `src/display.c:689` | C + 2 cheats | 48 | **23** | `libgpu/sys.c:745` |
| `func_8007D048__drs.c` | `func_8007D048` @0x8007D048 | `src/display.c:772` | `INCLUDE_ASM` | 161 | 0 | `libgpu/sys.c:787` |
| `func_800889D4__spu_FwriteByIO.c` | `func_800889D4` @0x800889D4 | `src/main.c:1682` | `INCLUDE_ASM` | 115 | 0 | `libspu/spu.c:111` |
| `_spu_FiDMA.c` | `_spu_FiDMA` @0x80088BA0 | `src/main.c:1693` | `INCLUDE_ASM` | 48 | 0 | `libspu/spu.c:161` |

Every draft carries its own header comment with the full per-symbol mapping,
the asm cross-check that justifies it, and its open questions. The sections
below are the cross-cutting parts only.

## Suggested measurement order

1. **`_spu_FiDMA`** — 48 insns, 0 rules, no known lever, and it shares the SPU
   register-file idiom with `func_800889D4`. It is the cheapest possible test
   of "does a psyz seed transplant at all under our cc1 + aspsx 2.34". If this
   one does not move, stop and diagnose the harness, not the C.
2. **`func_800889D4`** — same idiom, larger. Confirms the SPU mapping.
3. **`func_8007D048` (`_drs`)** — the libgpu unit's *safe* half. It shares its
   entire 47-instruction prologue with `_dws` but has none of `_dws`'s
   constant-fold risk, so it isolates whether the clamp+divide spelling lands.
4. **`func_8007CE0C` (`_dws`)** — only after `_drs`. Carries the one real
   unknown (below) and 23 rules.
5. **`func_8007CBB0` (`_clr`)** — needs the `GpuDrEnv` aggregate decl first.
6. **`_spu_gcSPU`** — highest payoff (104 rules) but the highest variance:
   a catalogued cc1 ICE and a policy question. Do it with a full session.

## Corrections to the upstream reports

**`tmp/psyq_provenance.md`'s global mapping is incomplete, not wrong.** It says
`_spu_memList <- D_800A2D40` and `_spu_AllocLastNum <- D_800A2D3C`. The
addresses are right, but BB2 **already has names** for those words, and the
current `_spu_gcSPU` body re-declares them as fresh `D_` externs:

| addr | BB2 name | Sony name | proof |
|---|---|---|---|
| 0x800A2D38 | `g_spu_voice_key_a` | `_spu_AllocBlockNum` | `spu_DmaTransfer` (really `SpuFree`, main.c:2164) loops `i < key_a`; psyz `SpuFree` loops `i < _spu_AllocBlockNum` |
| 0x800A2D3C | `g_spu_voice_key_b` | `_spu_AllocLastNum` | `SpuMalloc` (main.c:1986) spells `(key_b << 3) + (s32)_spu_memList`, i.e. `_spu_memList[_spu_AllocLastNum]` |
| 0x800A2D40 | `g_spu_voice_key_c` | `_spu_memList` | `#define _spu_memList ((SpuMemRec *)g_spu_voice_key_c)` already at main.c:1922 |

`SpuInitMalloc` (main.c:1905) against psyz `s_m_init.c` pins all three at once.
The drafts use the existing names; the `typedef struct Entry` and the two
`extern s32 D_800A2D3C` / `D_800A2D40` lines at `src/main.c:2005-2008` should be
deleted along with the body they serve.

**`tmp/psyz_sweep.md`'s target list overstates the libgpu unit.** It lists
`get_alarm=func_8007DC9C`, `set_alarm=func_8007DC68` and `get_dx=func_8007CA00`
as part of the unit. All three **already have matching C** in `src/display.c`
(lines 969, 965, 626) — they are not `INCLUDE_ASM` and need no transplant.
`func_8007DC9C` is still a queue item (4 regfix rules), but psyz leaves
`get_alarm` as `INCLUDE_ASM`, so psyz cannot seed it either way. The unit is
three functions, not six.

## The one structural change a measuring session must make

`func_8007CBB0` (`_clr`) needs the **`GpuDrEnv` aggregate** at 0x800F1858,
replacing splat's 13 per-word names `D_800F1858 .. D_800F1888`
([[split-scalars-hide-aggregate]], [[splat-symbol-names-are-not-evidence]]).

- **What to do:** add `typedef struct { u32 tag; u32 code[15]; } GpuDrEnv;` and
  `extern GpuDrEnv D_800F1858;` to `src/display.c`.
- **What NOT to do:** nothing in `asm/data/`, `undefined_syms_auto.txt`,
  `symbol_addrs.txt` or `named_syms.txt` changes. `D_800F1858` is a
  linker-script absolute symbol; widening its C type links unchanged, and the
  other 12 names simply stay unreferenced (they already are).
- **Why the aggregate still reproduces the per-word hi/lo pairs:** for
  `D_800F1858.code[3]`, GCC 2.7.2 at -G0 emits `lui $at,%hi(D_800F1858)` plus
  `sw rt,%lo(D_800F1858+0x10)($at)`; %hi is 0x800F and %lo is 0x1868 — the same
  eight bytes the disassembler renders as `%hi/%lo(D_800F1868)`.
- **Positive evidence for the aggregate:** the target materialises
  `&code[8]` into `$a2` (`lui` + `addiu`) and then *reuses* `$a2` for the
  `code[8] = 0x03FFFFFF` store (`sw $a3, 0x0($a2)`). Thirteen independent
  scalars cannot produce that reuse; an aggregate with an address-taken member
  produces it by construction.

`func_8007CE0C` and `func_8007D048` additionally need `GpuRect` (`s16 x,y,w,h`),
which replaces the local `_GpuChunkHdr_CE0C` typedef at `src/display.c:683-687`.
That typedef is display.c-local — no symbol-file involvement at all.

## Risk register

| target | biggest risk | how it shows up | first response |
|---|---|---|---|
| `_spu_gcSPU` | **cc1 ICE.** `memory/closer/phase3-progress.md` records a cc1 SIGSEGV on a structured pointer-walk scan in this function. The array-indexed psyz form probably sidesteps it — unmeasured. | cc1 segfaults during the build | it is the catalogued failure, not a new bug; fall back to variant B, or to the goto-spelled scan for whichever phase trips it |
| `_spu_gcSPU` | **the `if (i) { }` FAKE block is not pre-approved.** psyz's precedent is not this project's authorization. | variant A stalls with a phase-1-only residual | measure variant B, then route it through a fresh adversarial `cheat-reviewer` under the fake-exception rules **before** any commit |
| `func_8007CE0C` | **`var_s4 = 0` should constant-fold, but the target keeps it live.** Target sets `addu $s5,$zero,$zero` at 0x8007CE3C and branches `beqz $s5` at 0x8007CF68, roughly 75 instructions later. GCC 2.7.2 at -O2 normally folds exactly this — which is why the current body carries an `asm("s5")` pin and an empty-`__asm__` barrier. psyz matched the plain form under cc1-psx-272 with `-g`. | the `0xB0000000` arm vanishes and `$s5` is never allocated | measure the plain form first. If it folds, the pin is **not** the answer: check every caller of 0x8007CE0C for a third `$a2` argument (that would settle it as a parameter); the file-scope-static hypothesis is already ruled out by the asm (`addu`, not `lw`); then try do-while(0) ([[do-while-zero-exception]], already used two functions up at display.c:645). Changing `CC_FLAGS` is forbidden ([[no-compiler-divergence]]) — a `-g` compile is a diagnostic only |
| `func_8007CBB0` | the `GpuDrEnv` decl is the whole bet; if the aggregate is wrong, the distance barely moves | residual is a large permutation of *correct* stores with wrong base registers | re-read the `$a2`-reuse argument above before abandoning the aggregate; a store-order permutation with the right bases is a scheduler problem, not a source problem |
| `func_8007D048` | lowest of the libgpu three — shares its prologue with `_dws` and has no flag variable | | |
| `func_800889D4` | the `printf` declaration at `src/main.c:83` is `void printf(s32*, s32*)`, while the strings are both `extern s32` (line 81) and `const char[]` (line 1583) | compile error on the printf calls | copy exactly what the already-matched `_spu_init` call site does (`main.c:1611`); do not add a new declaration |
| `_spu_FiDMA` | lowest of all; 48 instructions, 0 rules | | |

## Things deliberately NOT done here

- No `struct SpuVoice` work. The brief flagged BB2's 54-byte versus psyz's
  52-byte divergence; none of these six targets touches `_svm_voice[]`, so it
  does not apply. It applies to `func_800871D4` (`_SsVmKeyOffNow`), which is
  **parked** as OWNER-ACCEPTED INCOMPLETE (ruling 2026-07-27) and is not
  queue-active work.
- No `union SpuUnion` was introduced. BB2's established idiom for the SPU
  register file is `*(volatile u16 *)(D_800A2CDC + off)`, and the
  already-matched `_spu_init` uses it about thirty times; a union would be a
  second C handle for the same memory.
- No compile, no sandbox, no build, no permuter run, and no edit to any tracked
  file. Every distance and rule count in this document is quoted from
  `tmp/psyz_sweep.md`, not re-measured.
