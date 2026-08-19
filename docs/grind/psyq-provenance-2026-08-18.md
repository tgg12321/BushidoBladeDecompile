# PsyQ library provenance / version-skew research — 2026-08-18

Scope: `_spu_gcSPU` (104 regfix rules), `_spu_note2pitch` (37), `CD_ready` (42).
Read-only session. All artifacts under `tmp/psyq_prov/`.

## Headline

The version-skew warning in [[psyq-reference-c-version-skew]] is **correct for SOTN's
psxsdk but is not the current state of the art.** A dedicated PsyQ decompilation that
targets **PSY-Q 4.0 with GCC 2.7.2** — `Xeeynamo/psyz` — exists, is MIT-licensed, and
appears **nowhere in this repo's memory, closer reports, or grind ledgers**. It is the
version-correct source tree the prior sessions were looking for and did not find.

Its `decomp/src/libspu/s_m_int.c` contains a **matched C body for `_spu_gcSPU`** — the
single heaviest regfix carrier in the queue.

## Prior state (what memory already established — do not re-derive)

`memory/closer/` already settled provenance far beyond the task premise:

- **`memory/closer/libcd-groundtruth.md`**: BB2 links PsyQ 4.0's `BIOS.OBJ` (bios.c
  **v1.86**) **bit-verbatim**. All 1,526 `.text` words masked-identical over
  `0x80080828..0x80082000`. `CD_ready` = that object's `CD_ready` (179 words @
  `0x80081030`). The `.LIB` is on disk at `tmp/closer/psyq/LIBCD_4.0.LIB`.
- **`memory/closer/psyq-library-census.md`** §(a): `_spu_gcSPU` (`LIBSPU/S_M_INT`,
  194 words @ `0x800896A0`) and `_spu_note2pitch` (`LIBSPU/S_N2P` @ `0x8008BA94`) are
  **also in the verbatim list** — i.e. BB2 links stock **PsyQ 4.0 LIBSPU** for both.
- **`memory/closer/libsnd-hunt-report.md`**: the "interim 4.0-lineage build absent from
  every SDK dump" finding applies to the **LIBSND sequencer gap modules**, *not* to any
  of our three targets. All three sit in verbatim 4.0 regions.

**Consequence:** for all three functions the linked library version is known exactly
(PsyQ 4.0) and the ground-truth object is already on disk. There is no unknown-version
problem left. The only question is whether **version-correct C** exists.

## Version mapping of every available source tree

| Source tree | URL | PsyQ version targeted | Toolchain | Covers our 3? |
|---|---|---|---|---|
| **Xeeynamo/psyz `decomp/`** | https://github.com/Xeeynamo/psyz | **4.0** (`PSYQ_VER := 400`) | cc1-psx-272 (GCC 2.7.2), maspsx `--aspsx-version=2.56` | **`_spu_gcSPU` MATCHED**; `CD_ready`, `_spu_note2pitch` = `INCLUDE_ASM` stubs |
| Xeeynamo/sotn-decomp `src/main/psxsdk` | https://github.com/Xeeynamo/sotn-decomp | SOTN's own link — libcd bios.c **v1.77**; libspu is a **later** revision than 4.0 | GCC 2.7.2 PSX | `bios.c` has `CD_ready`; `s_n2p.c` present but **wrong algorithm revision** |
| sozud/psy-q-decomp | https://github.com/sozud/psy-q-decomp | **3.6** (`src/3.6/` only) | original Win16/Win32 SN compilers, whole-OBJ matching | **no** `s_m_int.c`, **no** `s_n2p.c`, **no** `bios.c` (only `cd/c_002.c`) |
| ps2dev/ps2sdk `iop/sound/libspu2` | https://github.com/ps2dev/ps2sdk | PS2 IOP libspu2 (same Sony module layout: `s_m_int.c`, `s_m_f.c`, `s_m_m.c`) | modern gcc, reimplementation | structural cross-check only, not byte-targeted |
| OpenDriver2/PsyCross, Gh0stBlade/libValkyrie | https://github.com/OpenDriver2/PsyCross | none — API-compatible **reimplementations** for PC | n/a | **no value** for matching |

`psyz` is explicitly a spin-off of sotn-decomp's psxsdk that re-bases it onto PsyQ 4.0;
its README states 4.1+ "proved difficult", so 4.0 is precisely where its effort landed —
the same version BB2 links. License: MIT (`decomp/LICENSE`).

### Compile-config deltas between psyz and BB2 (relevant, not blocking)

| | psyz decomp | BB2 |
|---|---|---|
| cc1 | `bin/cc1-psx-272` | `tools/gcc-2.7.2/cc1` |
| CFLAGS | `-G0 -w -O2 -g -fno-builtin -funsigned-char -fpeephole -ffunction-cse -fpcc-struct-return -fcommon -fverbose-asm -msoft-float` | `-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel` |
| maspsx | `--aspsx-version=2.56` | `--aspsx-version=2.34` |

Most psyz flags are GCC 2.7.2 defaults spelled explicitly. The meaningful difference is
the ASPSX version (nop/reorder behaviour). This does **not** invalidate the C: psyz's
match target is the *same Sony 4.0 object* BB2 links, merely converted to ELF, so the
instruction stream psyz reproduces is BB2's target instruction stream.

## Per-function assessment

### 1. `_spu_gcSPU` — 104 rules, honest distance 121, WIP floor 25. **BEST LEAD BY FAR**

Version gap: **ZERO.** psyz matched it against PsyQ 4.0. Source banked at
`tmp/psyq_prov/psyz_s_m_int.c`.

The psyz body is five sequential loops over `SPU_MALLOC *_spu_memList` (a *pointer*
global) bounded by `int _spu_AllocLastNum`:

1. merge-adjacent-free scan (with an inner `for (j=i+1;;j++)` skip of `0x2FFFFFFF`
   sentinels), `continue` on merge so `i` is not advanced
2. zero-size entries → `0x2FFFFFFF`
3. bubble-sort by `addr & 0x0FFFFFFF`, `break` on `addr & 0x40000000`
4. first-sentinel compaction against `_spu_memList[_spu_AllocLastNum]`
5. backward tail coalesce `for (i = _spu_AllocLastNum - 1; i >= 0; i--)`

**Verified against `asm/funcs/_spu_gcSPU.s` (194 insns) — 5-for-5 phase match:**

| psyz loop | BB2 asm guard → loop head | signature evidence |
|---|---|---|
| 1 | `bltz $v0,.L80089780` → `.L800896D4` | inner `.L800896F0` self-loop, `bne $v0,$t2` vs `$t2 = 0x2FFFFFFF`, unconditional `j .L800896F0` = the `for(j;;j++)` shape; merge path `j .L80089760` skips the `i++` = `continue` |
| 2 | `bltz $v0,.L800897C8` → `.L800897A0` | `bnez` on size, store of `0x2FFFFFFF` |
| 3 | `bltz $v1,.L80089894` → `.L800897F4`/`.L80089828` | `$t6 = 0x40000000` break tests on both loops; `and $t4(0x0FFFFFFF)` + **`sltu`** (unsigned) compare; 6-store swap |
| 4 | `bltz $a1,.L80089918` → `.L800898C0` | `bne $v1,$a3` = `addr == 0x2FFFFFFF` |
| 5 | `bltz $t1,.L800899A0` → `.L80089950` | `bgez $t1` backward counter |

**Concrete divergences a future session must bridge** (BB2's current `src/main.c` body vs
psyz):

- **Addressing level is wrong in BB2's C.** BB2 hand-writes a pointer chassis
  (`Entry *base, *cur, *p, *outer, *inner, *e`) with `goto p1_scan` / `goto p1_cont`.
  psyz uses plain array indexing `_spu_memList[i]` and lets GCC's strength reduction
  produce the walking pointers. The target asm agrees with psyz: the walking pointer
  appears only in the *inner* loop (`addiu $a0,$a0,0x8`) while the *outer* index is
  rebuilt each phase (`sll $v0,$a2,3; addu $a0,$v0,$t5`).
- **Loop bound must stay a global read, not a cached local.** Target re-loads
  `%hi/%lo(D_800A2D3C)` at every phase head *and* at every outer-loop back-edge
  (`.L8008987C` reloads into `$v1`), because the swap stores may alias it. BB2's current
  C caches it (`cnt = D_800A2D3C;`) in phase 2 onward. psyz's `i <= _spu_AllocLastNum`
  in the `for` condition produces the observed reload exactly.
- **`D_800A2D40` is a pointer object** (`lw $t0,%lo(D_800A2D40)($t0)`), matching psyz's
  `extern SPU_MALLOC *_spu_memList` (`libspu_private.h:180`). BB2's sibling `SpuMalloc`
  in the same file already uses the `_spu_memList` spelling; `_spu_gcSPU` does not.
  Unify them.
- **Loop rotation should not be hand-written.** BB2 spells `if (guard >= 0) { i = 0;
  do {...} while (D_800A2D3C >= i); }`; psyz writes the plain `for` and lets GCC rotate.

This directly explains the WIP campaign's own conclusion (`memory/wip/_spu_gcSPU/notes.md`:
"our `base` was ONE pseudo, livelen 103, `$t3` everywhere, against a target using a
different register per phase — only reachable if the original had per-phase variables").
Array-indexed `for` loops give per-phase induction variables *by construction*, which is
what 121→25 of manual local-splitting was approximating by hand.

**Policy flag for the Judge:** psyz's phase-1 inner loop carries an empty
`if (i) { // FAKE }` block. That is a sotn-style fake-match construct and must clear
`cheat-reviewer` under the fake-exception rules ([[dead-store-fake-exception]] /
[[named-local-fake-exception]]) before it can be committed — it is not automatically
sanctioned here. It may also prove unnecessary under BB2's aspsx 2.34.

Note also `memory/closer/phase3-progress.md` records a **cc1 ICE** ("structured
pointer-walk scans SIGSEGV cc1 in this function; goto-spelled scan compiles"). The psyz
form is array-indexed rather than a structured pointer walk, so it may sidestep the ICE —
but this must be measured, and is the main technical risk to the transplant.

### 2. `_spu_note2pitch` — 37 rules, distance 39. **No source seed exists anywhere**

Version gap: none (BB2 links 4.0 `LIBSPU/S_N2P`), but **psyz leaves all of `s_n2p.c`
unmatched** (`_spu_2pitch`, `_spu_note2pitch`, `_spu_pitch2note` are all `INCLUDE_ASM`).
SOTN's `s_n2p.c` is a later, table-lookup revision — already killed in session 2 and
re-confirmed here. sozud has no `s_n2p.c` at all.

So the world's only PsyQ-4.0-targeted decomp has not cracked this module either. That is
an independent difficulty signal, and it means **self-decomp from the ground-truth object
is the only path** — confirming `phase3-progress.md:784`. Best seed remains BB2's own.

**Adjacent win, though:** psyz *does* match `libsnd/vm_n2p.c` `note2pitch` /
`note2pitch2` at 4.0 (the 192-entry `pitch_table[]` starting `0x1000, 0x100E, 0x101D,
0x102C, 0x103B…`). BB2's queue has a **separate, distinct** item `note2pitch`
(`LIBSND/VM_N2P` @ `0x80086B38`, distance 49, 0 rules) — that one has a version-correct
matched seed available now. Do not confuse the two: LIBSND `note2pitch` = table lookup;
LIBSPU `_spu_note2pitch` = the 4.0 curve-scan.

### 3. `CD_ready` — 42 rules, distance 56, grind floor 4 after 57 sessions. **Version skew already ~zero; not a provenance problem**

- BB2's bytes = PsyQ 4.0 `BIOS.OBJ` bios.c **v1.86**, verbatim, object on disk.
- SOTN's `bios.c` (**v1.77**) differs from v1.86 across the whole module by only the
  `li` spelling (`ori rt,$zero,imm` vs `addiu rt,$zero,imm`) — **2 words in `CD_ready`**,
  and that is an assembler-era artifact, not a C-level difference. Per
  `libcd-groundtruth.md` the C, scheduling and register allocation are identical across
  v1.77→v1.86.
- **psyz (correct 4.0 version) leaves `CD_ready` unmatched** — as it does `CD_sync`,
  `CD_cw`, `CD_datasync`, `getintr` and `callback`. Only `CD_vol`, `CD_flush`,
  `CD_initvol`, `CD_initintr`, `CD_init`, `CD_set_test_parmnum` are matched there.

**Assessment: there is no version gap left to close on `CD_ready`.** The correct-version
C is effectively already known and in the tree. The residual floor of 4 is a codegen
reproduction problem (the sched2 T-14 tie / reorg delay-slot steal already named in
`memory/grind/CD_ready/evidence.md` s6–s7), not a source-provenance problem. Further
provenance hunting on this function has **negative expected value**; stop looking.

## Broader queue impact (bonus)

Cross-referencing psyz's matched C bodies against `engine/queue.json` (263 items) found
**4 live queue items with a version-correct matched seed**:

| psyz file | function | BB2 status | rules | distance |
|---|---|---|---|---|
| `libspu/s_m_int.c` | `_spu_gcSPU` | active | **104** | 121 |
| `libgpu/sys.c` | `PutDispEnv` | active | 0 | **298** |
| `libsnd/vm_n2p.c` | `note2pitch` | active | 0 | 49 |
| `libspu/spu.c` | `_spu_FiDMA` | active | 0 | 48 |

`PutDispEnv` is notable: distance 298, an existing ledger at `memory/grind/PutDispEnv/`,
and psyz's body is a full PsyQ 4.0 match (itself derived from a decomp.me PsyQ 3.5
scratch, https://decomp.me/scratch/7H6Nk).

psyz also carries 308 matched C bodies overall across libapi/libc2/libcd/libetc/libgpu/
libsnd/libspu — worth a systematic sweep against the 92 census queue items in verbatim
library regions, beyond the 4 exact-name hits above (BB2 names and Sony names have only
partially converged, so name-keyed matching under-counts).

## Recommended next steps

1. **Drive `_spu_gcSPU` with the psyz body.** Transplant `tmp/psyq_prov/psyz_s_m_int.c`
   onto BB2's globals (`_spu_memList` ← `D_800A2D40`, `_spu_AllocLastNum` ← `D_800A2D3C`,
   `SpuMemRec`/`SPU_MALLOC` typing already present for `SpuMalloc` in `src/main.c`), keep
   plain `for` loops and array indexing, do **not** cache the bound in a local, and
   measure with `sandbox --disable all`. Expect the 104 rules to be retired wholesale if
   it lands. Risk: the catalogued cc1 ICE; mitigation: the array-indexed form is not the
   structured pointer walk that triggered it.
2. **Route the `// FAKE` block through `cheat-reviewer` before any commit**, and test
   whether it is even needed under aspsx 2.34.
3. **Stop provenance work on `CD_ready` and `_spu_note2pitch`.** For `CD_ready` the
   version-correct source is already effectively in tree; for `_spu_note2pitch` no source
   exists at any version and self-decomp is the only path.
4. **Sweep psyz systematically** against the 92 census queue items by address/size, not
   just by name — 308 matched 4.0 bodies is a large untapped surface.
5. **Register a memory correction**: [[psyq-reference-c-version-skew]] should gain a line
   that psyz (PsyQ 4.0, MIT) is the version-correct tree for LIBCD/LIBSPU/LIBSND, so
   future sessions do not repeat the SOTN-only search that produced the skew finding.

## Artifacts

- `tmp/psyq_prov/psyz_s_m_int.c` — matched `_spu_gcSPU` (PsyQ 4.0)
- `tmp/psyq_prov/psyz_bios.c` — psyz libcd bios.c (CD_ready stubbed)
- `tmp/psyq_prov/psyz_s_n2p.c` — psyz libspu s_n2p.c (all stubbed)
- `tmp/psyq_prov/psyz/` — full shallow clone (MIT)
