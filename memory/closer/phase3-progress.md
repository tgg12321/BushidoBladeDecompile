# Closer Phase 3 — PsyQ psxsdk adoption progress ledger

Session 1 (2026-07-09). Reference tree: sparse clone at `tmp/sotn-decomp`
(src/main/psxsdk only). Work list: memory/closer/psyq-queue-hits.json.

## KEY METHOD FINDING — struct adoption vs the sandbox metric

Adopting Sony's module-state STRUCT (the honest form) makes every member
access emit relocs as `D_<base>+addend` while the pristine `build/src/*.o`
reference spells them as per-member symbols with addend 0. **Identical
linked bytes, nonzero masked sandbox score.** The sandbox is therefore NOT
the right gate for struct adoptions — the full build + per-region EXE diff
is (`tmp/closer/exe_diff_regions.py` localizes differing words per function
via build/bb2.map). Proven: func_800826CC scored sandbox 39 with ZERO
differing EXE words.

**PROOF OF THIS SESSION'S BATCH: full build SHA1 == oracle
(62efab4f73f992798c43e8c730aa43baa10bb4fa) with all edits in place**
(measured 2026-07-09, `wteng main build` → MATCH, exe_diff_regions → 0
differing words). HEAD-carried rules for these functions are now inert
no-ops (their text anchors no longer match, e.g. saEft00Add_sub 151-152)
or unchanged (saEft00Add's, whose body kept HEAD parity).

## Completed honestly this session (byte-proven at the oracle, edits in src/)

| queue item | Sony name | what was fixed |
|---|---|---|
| cdrom_GetCmdName | CdComstr | goto-end accumulator → Sony early-return, u8 param (sandbox 0) |
| cdrom_GetResultName | CdIntstr | same (sandbox 0) |
| cdrom_ConfigSPU | CD_initvol | outer-volatile ptr + staged locals → Sony faithful body, chained vol[] init (sandbox 0) |
| func_800826CC | CdRead | 6 asm("sym") alias renames + volatile saved_ptr device → volatile CdlREAD struct, direct switch stores, direct saEft00Add call |
| func_800827D0 | CdReadSync | fn-ptr cast workaround → Sony while(1) body + honest CdReady wrapper call |
| saEft00Add_sub | CdReadBreak | (was PARKED dist 4!) pointer device → volatile struct members; rules now inert |
| sys_SetTimer | CdReadMode | volatile cast on g_sys_timer → volatile struct member (2 accesses CSE the address = target's lui+addiu form; plain `*p` was measured 2 insns SHORT — the queue note's suggested fix is WRONG) |
| spu_ResetMotionEntry | _SsSndNextSep | dead `s32 offset` removed; body already Sony's mixed score-ptr/full-index shape (next.c) |

Non-queue upgrades in the same proof: Vu0SetLightColMatrix_800801E8/80080208
(CdSync/CdReady) now forward (mode, result) honestly (were void(void) with
register-passthrough accident); tslTmlGetHeda (CdDataCallback) returns s32;
the `saEft00Add_ret`/`func_80080660_ret` asm() function aliases deleted.

## The CdlREAD struct (cdread.c module state)

Declared in src/system.c + src/ings2.c (same layout): base symbol
`D_800A14D0`, members sectors/buf/p/mode/size/cnt/t2/t1/pos/cbsync/cbready/
cbdata/tslmode at +0..+0x30 (= D_800A14D0..D_800A1500; g_sys_timer is an
absolute-symbol alias of D_800A1500 used by other TUs). CD_ReadCallbackFunc
is the preceding word D_800A14CC. SOTN ref: `static volatile cdreadStruct
D_80032DBC` in src/main/psxsdk/libcd/cdread.c. IRQ-mutated by cb_read
(local at 0x80082050) → volatile is original semantics.
**Anchor MUST be D_800A14D0** (not 14CC): CdRead's codegen materializes
&sectors as the struct base (`lui v0; addiu v0,%lo(D_800A14D0)`); a
CC-anchored struct loses that insn (measured, 4-byte EXE shrink).

## saEft00Add (cd_read_retry) — still INCOMPLETE, banked findings

- HEAD body restored verbatim (keeps its ~10 regfix + 2 asmfix rules,
  volatile casts, `__asm__("")`); one interim de-volatile cast
  `*(s32 *)&D_800A14D0.sectors` keeps HEAD parity (volatile read there
  costs +1 insn — measured).
- Faithful Sony early-return shape (cdread.c cd_read_retry): measured 81
  differing EXE words — the goto-end chassis is much closer to target.
  All-volatile members also cross-jump-merge the three `cnt=-1` exit tails
  (-28 bytes) — HEAD's mixed volatile/plain spellings keep them apart.
  Honest closure needs the mixed-exit treatment on the struct chassis.
  Candidate banked: memory/closer/candidates/saeft00add_sony_shape.c.

## Metric notes for the next session

- `wteng main build` writes build/bb2.exe but does NOT regenerate
  build/src/*.o — sandbox stays anchored to pristine HEAD objects.
- Sandbox scores on struct-adopted functions = (real diffs) + (reloc addend
  artifacts). Use exe_diff_regions.py for truth.
- LF endings: Write/Edit tools produced clean builds throughout.

## Batch 2 (same session): main.c/gpu.c/display.c — all sandbox-0 AND in the
## final oracle-green build

| queue item | Sony name | fix |
|---|---|---|
| func_8008AE7C | SpuSetTransferMode | dead v0++/v0-- if-chain → Sony's switch + return transMode |
| func_8008B400 | SpuGetAllKeysStatus | opaque one/three/two → literals (they hoist naturally); `limit=24` kept (= Sony's inlined `max` var, real+used); indexed `status[voice]` stores (walking ptr is loop.c strength-reduction) |
| func_800892F8 | SpuQuit | g_spu_init_flag/g_spu_timer are `void (* volatile)()` = Sony's _spu_transferCallback/_spu_IRQCallback (libspu_internal.h:39,42 types them volatile) — original semantics, not coercion |
| func_8008ADC4 | SpuWrite | same declaration fix (func_8008AD64 SpuRead also 0) |
| spu_IrqHandler | SpuInitMalloc | always-true wrapper → `if (num > 0) {...} return 0;` with size staged first (4.0 statement order; SOTN's older order mismatches — branch shape needs positive-path-first) |
| func_80085448 | SsSetSerialVol | s32 buf[10] → real SpuVolume/SpuCommonAttr structs (sizeof 0x28 == old frame; the "32 bytes" claim in the reopen note is wrong) |
| single_game_getEnemyCharId | ratan2 | dead var_v0 decl removed |
| gpu_SetMode | ResetGraph | new_var=-1 stage + decl removed — plain literals already match |

**FINAL PROOF: full build SHA1 == oracle with ALL batch-1+2 edits in place;
exe_diff_regions = 0 differing words (2026-07-09, end of session 1).**

## Banked (findings measured, HEAD body restored / untouched)

- **func_8008BEA4 / func_8008C184 (SioAnsyncRead/Write, LIBCOMB)**: the
  always-true `if((flag&&flag)&&flag){do{}while(0);}` is load-bearing: it
  keeps target's strictly-ordered B04→B00 stores + un-hoisted base load.
  MEASURED: plain removal 4; all-volatile decls 9 (flag loses its la
  form); volatile only B04/B00 4 (volatile did NOT hold store order!);
  bare do-while(0) fence 4; swapped source order 4 (scheduler swaps
  back deterministically). Next avenue: volatile SioAsync struct at
  D_800F1AFC (+0 flag, +4 buf, +8 len — one IRQ-shared Sony block, same
  pattern as CdlREAD) — requires respelling cpu_side_move_dir_3's uses
  (rule-carrying! needs byte-parity care, incl. one non-volatile
  cast like saEft00Add's .sectors). NO asm() renames (tried, self-reverted
  — forbidden family).
- **func_80082C58 (startIntr, LIBETC intr.c)**: full Sony struct mapped:
  intrEnv_t at D_800A1578 {u16 interruptsInitialized; u16 inInterrupt;
  Callback handlers[11] (+0x04); u16 enabledInterruptsMask (+0x30); u16
  savedMask; s32 savedPcr; jmp_buf buf (+0x38, D_800A15B4 = buf[1] =
  JB_SP); s32 stack[1024] (+0x68)}; sotn intr.c:10-19 + startIntr():79.
  flag_ptr = s0b-60 == &intrEnv.interruptsInitialized. Whole-module
  conversion needed (siblings func_80082D34=trapIntr, D_80083418=
  trapIntrDMA, D_8008359C=setIntrDMA share the symbols; some carry rules
  → byte-parity discipline like CDREAD).
- **gpu_SetDispMask (SetDispMask, LIBGPU sys.c v1.129)**: needs the SYS
  .data-block struct (base 0x8009BE24; debug byte +0x52; DISPENV cache
  +0xBC; memset arg = debug_base + 0x6A). Same Ruling-1-clean single-
  object treatment; design against tmp/libscan/psyq40/LIBGPU.LIB.
- **saEft00Add (cd_read_retry)**: see candidates/saeft00add_sony_shape.c.
