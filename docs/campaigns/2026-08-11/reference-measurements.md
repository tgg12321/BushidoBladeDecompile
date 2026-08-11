# Campaign 2 — SOTN psxsdk reference-body transplant measurements

Measured 2026-08-11 against `main` (Grinder stopped). Every score is the **honest
cheat-free** number: `engine sandbox <func> --disable all` (regfix/asmfix rules
dropped, cheat-asm stripped). Reference tree: `C:\Users\Trenton\Desktop\sotn-decomp`,
`src/main/psxsdk/**` only.

**Nothing was committed. The tree was fully restored** — see "Tree state" at the end.

Adapted bodies are in `tmp/campaign2/bodies/<func>.c`, each carrying its own
adaptation notes and residual analysis in a header comment.

---

## Headline

| | |
|---|---|
| **MATCH-READY (score 0, oracle-verified)** | **`_SsSeqPlay`** |
| Large improvements (all seven still-INCLUDE_ASM targets that were attempted) | `func_80080828` 354→30 · `PutDispEnv` 298→77 · `func_8007D048` 161→44 · `func_8007CBB0` 151→77 · `func_8007D6D8` 187→69 |
| Moderate improvements | `_spu_gcSPU` 121→57 · `CdControl` 25→17 · `CdControlF` 23→19 |
| Reference is a different build (recorded as a negative) | `_spu_note2pitch`, `_spu_pitch2note`, `sprintf` |
| Reference adds nothing over BB2's current body | `CD_sync`, `MoveImage` |
| Not transplanted this pass | `CD_cw`, `CD_ready`, `CD_datasync`, `func_8007DC9C`, `func_8007D3F8`, `func_8007CE0C` |

---

## Results

| func | BB2 file | reference source | adaptation | baseline | best | per-variant | residual class | verdict |
|---|---|---|---|---|---|---|---|---|
| `_SsSeqPlay` | `src/main.c` | `libsnd/seqread.c:28-51` | struct fields → BB2 byte offsets (+0x90/+0x54/+0x52, 0xB0 stride, D_80106F28 table); `_SsGetSeqData` → `func_80084CC0` | 28 (15 rules) | **0** | 0 | none | **MATCH-READY** — `retire` dropped all 15 rules, full build SHA1 == `62efab4f…` (oracle) |
| `func_80080828` (`getintr`) | `src/system.c` | `libcd/bios.c:115-208` | near-verbatim; CD registers → D_800A147C/80/84/88, Intr → D_800A1494/95/96, results → D_800F19A0/A8/B0, `_memcpy` inlined as BB2's guarded byte loop | 354 (INCLUDE_ASM) | **30** | 30 | 9 insns: 3 of 5 switch arms materialise `&Intr.sync` (lui+addiu+sb) where our build uses the 2-insn %lo form. Insns 0–155 are already identical | NEAR — best ratio in the campaign |
| `func_8007D048` (`_drs`) | `src/display.c` | `libgpu/sys.c:660-712` + BB2's own sibling `func_8007CE0C` | GPU regs/DMA from the sibling; adds the 0x08000000 ready-to-send spin; DMA dir 0x04000003 / chcr 0x01000200 | 161 (INCLUDE_ASM) | **44** | 44 | clamp head: target keeps a copy our build folds into the `slt`; callee-save renames; target frame 0x48 vs ours 0x38. Shared with sibling `func_8007CE0C` (which sits at 48) | NEAR |
| `_spu_gcSPU` | `src/main.c` | `libspu/s_m_m.c:88-153` | verbatim; `_spu_memList[]` → `((Entry*)D_800A2D40)[]`, `D_80033560` → `D_800A2D3C` | 121 (104 rules) | **57** | 57 | 7 insns short — target re-derives the `ml[i]`/`ml[j]` element addresses more often than the indexed form does. Algorithm confirmed identical | NEAR — would retire 104 rules |
| `func_8007D6D8` (`_exeque`) | `src/display.c` | `libgpu/sys.c:797-857` | older revision: chcr early-out returns 1 before masking; 0x60-stride parallel arrays D_80103680/84/88; no null-func arm | 187 (INCLUDE_ASM) | **69** | 69 | 34 insns short — target re-loads the queue-out index `D_8009BF7C` at every slot access; plain C CSEs them. (The sibling `func_8007D3F8` spells this `*(volatile s32*)&…`, a coercion cheat, deliberately not used) | NEAR |
| `PutDispEnv` | `src/display.c` | `libgpu/sys.c:336-402` | `->ctl` → `D_8009BE6C[4]`, shadow env → D_8009BEE0; BB2 compares all 4 halfwords (SOTN's `LOW()` compares word pairs), v_start floor 0x10, v_end `+2`/0x102, PAL `+0x13` | 298 (INCLUDE_ASM) | **77** | 77 | the 8 shadow compares: target does `lhu` + `sll 16`/`sra 16`, our build folds u16-read+(s16)-cast to one `lh`; plus one callee-save choice for `cmd` | NEAR |
| `func_8007CBB0` (`_clr`) | `src/display.c` | `libgpu/sys.c:567-605` | older revision: fixed packet global `D_800F1858[13]`, otag `0x08000000|&buf[9]`, clamps vs `D_8009BE78/7A - 1`, `_param(n)` → `gpu_GetInfo(n)`, `_cwc` → `gpu_StartDmaList`; BB2 emits an extra 0xE1000000 texpage word | 151 (INCLUDE_ASM) | **77** | 77 | register allocation across the two packet-fill arms + the same 2-insn clamp-head fold as `_drs` | NEAR |
| `CdControl` | `src/system.c` | `libcd/sys.c:87-112` | `static inline cd_cw` adopted; `CD_cbsync` → `g_cd_callback_a`, `D_80032A24[]` → `g_cd_sector_buf[]`, `CdStatus()&ShellOpen` → inlined `g_cd_mode & 0x10`; BB2 returns `ret+1` not `==0` | 25 (3 rules) | **17** | v1 (SOTN `while(count--)`) **17** · v2 (goto-loop) 28 · v3 (hybrid) 32 | LICM: our build parks the literal `1` of the `idx != 1` compare in a 9th callee-save across the loop; target re-materialises `li $v0,1` each iteration. Plus RA | IMPROVED (also retires the `asm("s7")` pin) |
| `CdControlF` | `src/system.c` | `libcd/sys.c:112` | same `cd_cw` inline | 23 | **19** | v1 **19** · v2 25 · v3 32 | same as `CdControl` | IMPROVED (retires the `asm("s6")` pin) |
| `CD_sync` | `src/system.c` | `libcd/bios.c:232-258` | printf-args spelling only (BB2 already inlines set_alarm/get_alarm/callback that SOTN calls out) | 7 (5 rules) | 7 | both args inline (SOTN spelling) 14 · one arg hoisted 7 (= baseline) | unchanged: a 15-insn scheduling reorder + `$v1`↔`$a0` rename in the timeout printf-arg region | REFERENCE ADDS NOTHING |
| `MoveImage` | `src/display.c` | `libgpu/sys.c:269-279` | none applied | 21 (19 rules) | 21 | — | BB2's body already *is* the target shape. SOTN's revision builds a 5-word `param[]` **local**; BB2's writes the fixed globals D_8009BF24/28/2C and passes a pre-initialised packet at `&D_8009BF24 - 8`. Residual is a cross-jump merge of the two `return -1` tails | REFERENCE ADDS NOTHING |
| `_spu_pitch2note` | `src/main.c` | `libspu/s_n2p.c:56-96` | verbatim (reference has no library symbols, only arithmetic) | 64 | 81 (**worse**) | 81 | SOTN scans forward for the last set bit; BB2's target scans the **complement** in reverse (`~arg2 & 0xFFFF`, shift 15→0, stop on first clear). Build is 11 insns **longer** than target | DIFFERENT-BUILD |
| `_spu_note2pitch` | `src/main.c` | `libspu/s_n2p.c:29-54` | not transplantable | 39 (37 rules) | — | — | SOTN's revision is a 193-entry pitch **table** lookup. BB2's target contains no such table: it uses the `0x2AAAAAAB` divide-magic plus an iterative multiply chain (5 `mult`/`multu`). Different algorithm, not different symbols | DIFFERENT-BUILD |
| `sprintf` | `src/text1b_b.c` | `libc/sprintf.c:21-307` | not transplantable | 535 (INCLUDE_ASM) | — | — | Different implementation: BB2's target calls **memchr + memmove + strlen**; SOTN's revision calls only `strlen` and open-codes the padding. The transplant precondition (same implementation, different symbol names) fails | DIFFERENT-BUILD |

### Not transplanted this pass

Measured baselines only — no reference body was applied, so no verdict is claimed.

| func | file | baseline | note |
|---|---|---|---|
| `CD_cw` | `src/system.c` | 263 (INCLUDE_ASM) | **The best remaining opportunity.** Reference is `libcd/bios.c:291-340`. The full symbol map it needs is already derived in `bodies/func_80080828.c` (same module, same globals), so a follow-up pass should be fast |
| `func_80080828`-adjacent `CD_ready` | `src/system.c` | 56 (42 rules) | Same revision skew as `CD_sync` (BB2 inlines set_alarm/get_alarm/callback). `CD_sync` at distance 7 showed the reference does not move this family |
| `CD_datasync` | `src/system.c` | 18 (15 rules) | parked; reference `libcd/bios.c:459-478` |
| `func_8007DC9C` (`get_alarm`) | `src/display.c` | 9 (4 rules) | parked (RA/sched endgame-lock); reference is 10 lines |
| `func_8007D3F8` (`_addque2`) | `src/display.c` | 43 (36 rules) | reference `libgpu/sys.c:745-795` |
| `func_8007CE0C` (`_dws`) | `src/display.c` | 48 (23 rules) | reference `libgpu/sys.c:608-656`; its residual is the same clamp head as `_drs`, which the `_drs` measurement already characterises |

---

## What the reference is actually worth

The campaign supports the standing framing in `memory/reference/psyq-reference-c-version-skew.md`
— the reference is a lead, not an answer — but sharpens it in three ways:

1. **The reference's value is CONTROL FLOW, not symbols.** `_SsSeqPlay` closed to a byte
   match purely because SOTN's nested `do { do { … } while (delta == 0); … } while (acc < n);`
   plus the if/else-if/else ladder is the shape GCC 2.7.2 emits. The arithmetic was already
   right in BB2's body; the 15 regfix rules were compensating for the goto-loop spelling.
2. **The skew is per-function, not per-library.** Inside LIBCD/BIOS, `getintr` transplants
   almost perfectly (354 → 30) while `sprintf` in LIBC is a different implementation
   entirely. Inside LIBSPU/S_N2P, `_spu_pitch2note` and `_spu_note2pitch` are both different
   builds. Deciding on the library is not safe; each function needs its own check.
3. **A cheap pre-check predicts the skew: compare the CALL SETS.** `sprintf` was ruled out
   in one command (`grep -oE "jal +\w+" asm/funcs/sprintf.s` → memchr/memmove/strlen, vs
   SOTN's strlen-only). Same for `_spu_note2pitch` (target has no rodata table). Running
   that check before writing any C would have saved most of the dead-end effort here.

A fourth, narrower finding: where BB2 has already decompiled one member of a sibling pair,
that sibling is a better source of BB2 vocabulary than the reference is. `func_8007D048`
(`_drs`) was written from BB2's own `func_8007CE0C` (`_dws`) with SOTN supplying only the
deltas, and it landed at 44.

## Caveat on `bodies/func_80080828.c`

That body carries `asm("D_800A…")` alias renames. They are a **measurement scaffold only**,
present solely because `src/system.c` re-declares those symbols further down with different
types. Alias renames are in the cheat catalog. Landing that body for real requires
reconciling the duplicate declarations to one canonical decl per symbol — the types used
in the body (D_800A11C4 as `s32`, D_800A11D5 as `u8`, D_800A11DC as `s32[]`) are the ones
the target asm actually loads, so the reconciliation is the honest fix.

## Tree state

`git status --short` at end of campaign:

```
 M metrics/events.jsonl
```

`metrics/events.jsonl` is the engine's own silent capture (append-only, committed source
of truth per `metrics/README.md`) — it is written by every `engine` invocation and is not
a campaign edit. **Every `src/` file was restored via `git checkout`**; `regfix.txt` was
also restored after the `_SsSeqPlay` retire proof. Nothing was committed.
