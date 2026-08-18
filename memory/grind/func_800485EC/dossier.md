# func_800485EC — recon dossier (2026-08-17)

## 1. IDENTITY
- **Body:** `src/text1b.c:392` (`void func_800485EC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)`).
  Forward-declared `extern void func_800485EC(s32,s32,s32,s32,s32,s32);` at `src/text1b.c:358` and `src/text1b_b.c:63`.
  Sole in-file caller: `src/text1b.c:389`, passing `(entry_off, a1_arg3, (s16)a, (s16)b, c, d)`.
- **Named:** `gpu_SetTexAttr_800485EC` (`named_syms.txt:3689`) — "parses src prim header (type 0x10), fills dst
  descriptor with uv + clut(gpu_CalcClut) + tpage(gpu_CalcTPage)".
- **What it does:** walks a TIM/prim-style header at `arg0`. Bails unless `*(u8*)arg0 == 0x10`. Reads a flags word,
  writes `flags & 7` to `dst+0`, and if `flags & 8` walks a sized sub-block (`size>>2<<2`) writing u/v, a s16, a
  pointer at `dst+0x1C` and a CLUT via `GetClut`. Then unconditionally consumes another 8-byte header, writing
  `dst+2/4/8/6/0x18` and a TPage via `GetTPage`.
- **Size:** target 68 insns; `asm/funcs/func_800485EC.s` = 73 lines.

## 2. MEASUREMENT
- `canonical func_800485EC` → **verdict `C`**, `asm_insns 0`, `total 68`, `distance 31` ("pure-C distance 31 <= 50").
- `sandbox func_800485EC --disable all` → **score 31** (honest, cheat-free floor), `target_insns 68`,
  `build_insns 66`, `rules_dropped 25`.
- The sandbox reports no per-region breakdown, so I disassembled the cheat-disabled object
  (`tmp/sandbox/func_800485EC/text1b.o`) against `build/src/text1b.o` with `engine.score.normalized_insns(mask=False)`.
  Full alignment in `tmp/recon/func_800485EC.hdiff.txt`. Aggregate: **equal 40 / replace 23 / delete 7 / insert 7**.
  Three residual clusters:
  1. **Prologue+entry block (T5–T15 vs H5–H15):** target saves `s3` then `s2`, and issues `lbu v1,0(s1)` **before**
     the `move s2,a2 / li v0,16 / lhu a0,56(sp) / lhu a1,60(sp)` group. We save `s2` then `s3` and hoist `li v0,16`
     ahead of the `lbu`. Register naming `v1` (target) vs `t0` (ours) for the header byte.
  2. **Stack-param width (T12–T13):** target `lhu a0,56(sp)` / `lhu a1,60(sp)`; ours `lw a0,56(sp)` / `lw a1,60(sp)`.
     Same offsets — pure load-width divergence.
  3. **The walking pointer (T22–T57, the bulk):** target keeps a live cursor in `v1` and advances it
     (`move v1,s1` → `lw v0,0(v1)` → `addiu v1,v1,8` → `lhu v0,2(v1)` → `lw v0,0(v1)` → `addiu v1,v1,4` →
     `sw v1,28(s0)`), and does the same in the else-path (`addiu v1,s1,8` … `addiu v1,v1,4` … `sw v1,24(s0)`).
     Our honest build **folds the cursor into base+constant** (`lhu v0,10(s1)`, `lw v1,8(s1)`, `addiu v0,s1,12`),
     which is where all 7 deletes and 7 inserts come from — the `addiu` advance instructions simply don't exist.

## 3. RULE INVENTORY (25 regfix, 0 asmfix — `regfix.txt:2438-2462`)
| Lines | Kind | Papers over |
|---|---|---|
| 2439, 2440 | subst `lw`→`lhu` @11,12 | **stack-param load width** (residual cluster 2) |
| 2442 | reorder `9,7,5,10,15,6,8,11,12,13,14` @5-15 | **entry-block scheduling + save order** (cluster 1) |
| 2438, 2441, 2444, 2445, 2447-2450, 2457 | subst `$8`→`$3` (×9) | rename our `t0` cursor to target's `v1` — the walking-pointer register (cluster 3) |
| 2443, 2446 | subst base/reg on stores @21,26 | cursor-vs-`s16` base selection |
| 2451-2454, 2458, 2459, 2461, 2462 | subst `$2`↔`$3` pairs | v0/v1 rotation downstream of the cursor |
| 2455, 2456 | subst pair @43,44 (swap `addu $8,$17,8` ↔ `lh $4,0($16)`) | **statement-order swap** at the else-path cursor init |
| 2460 | reorder `54,52,53` @52-54 | else-path load/advance ordering |

Also carried in the C body itself (invisible to the rule count, stripped by the sandbox, **and equally disqualifying
for COMPLETED-C**): four `register … asm("s1"/"s0"/"s2"/"s3")` parameter pins (`src/text1b.c:394-397`), one
`register u32 size asm("v0")` (line 410), and a scheduling barrier `asm volatile("" : "=r"(v1) : "0"(v1))` at
line 424. The barrier exists precisely to stop the cursor fold described in cluster 3 — the honest score is what
the code looks like once it is removed.

## 4. RESIDUAL DIAGNOSIS
The dominant mechanism is **GCC 2.7.2 folding a walking pointer into base+displacement addressing** — induction/
address CSE collapsing `p += 8; … p += 4` into constant offsets off the original base, because after the last read
the cursor is dead except for the single `*(s32*)(dst+0x1C) = v1 + 4` use, which GCC computes as a fresh `addiu`
from `s1` rather than keeping the cursor live. Matching rule: **`.claude/rules/walking-pointer-serializes-parallel-loads.md`**.
Secondary: **`narrow-stack-param-subword-offset`** (the `lw` vs `lhu` on the two stack-homed params) and the entry-block
scheduling/save-order group, which is ordinary list-scheduling + reload save placement (`register-alloc-pure-c`).
This is *not* a frame or constant-folding problem: frame layout and insn count agree closely (68 vs 66) and the two
missing insns are exactly the two `addiu vN,vN,4` cursor advances.

## 5. FRONTIER (ranked; no prior WIP/grind ledger exists for this function — nothing killed yet)
1. **Advance-then-store the cursor** (highest confidence; explains 14 of the 31 edits directly). Replace
   `*((s32 *) (s0 + 0x1C)) = v1 + 4;` with `v1 += 4; *((s32 *) (s0 + 0x1C)) = v1;` and likewise
   `*((s32 *) (s0 + 0x18)) = v1 + 4;` → `v1 += 4; *((s32 *) (s0 + 0x18)) = v1;`. This makes the cursor multi-set and
   live across the store, which is exactly the shape `walking-pointer-serializes-parallel-loads` says defeats the
   fold — and it emits target's `addiu v1,v1,4` in both paths. Should be tried **with the line-424 barrier removed**,
   since the barrier is the thing being replaced.
2. **Declare arg4/arg5 as `u16`** (independent, cheap, high confidence). Target's `lhu a0,56(sp)` / `lhu a1,60(sp)`
   sit at the *same* offsets as our `lw` (little-endian low half), and both values are consumed only as
   `*((u16*)(s0+0xA)) = arg4` / `+0xC = arg5`. Changing the definition's last two params to `u16` (and the two
   `extern` decls at `text1b.c:358` / `text1b_b.c:63` to match) should retire rules 2439/2440 outright. Cross-check
   `header-type-correction-from-use-sites`.
3. **Drop the four parameter register pins and re-measure.** They are cheat-asm and must go anyway; measuring the
   honest shape *without* them tells you whether the s2/s3 save-order inversion (cluster 1) is caused by the pins
   forcing an allocation order, or is a genuine reload question. Do this before any RA work — it may move the floor
   on its own.
4. **Only then**, the entry-block schedule: get `lbu v1,0(s1)` ahead of the two stack-param loads by sinking the
   `arg4`/`arg5` reads to their use sites inside the `flags & 8` arm (they are only used there). This is the
   `split-read-defeats-hoist` shape and would retire rule 2442's reorder.

## 6. SIBLING LEADS
- Same-file `func_8004876C` (`src/text1b.c:451`) already ships the **walking-pointer idiom in its accepted form**
  (`var_t1 = arg0; … temp_a1 = *var_t1; var_t1 += 1;`) — read it as the reference spelling for hypothesis 1.
- The `GetClut`/`GetTPage` wrapper family (`src/gpu.c:263-329`, `src/sound.c:690-693`) shows how the SDK call
  arguments are conventionally typed; useful if hypothesis 2's `u16` change disturbs the `GetClut` argument shape.
- `src/text1b.c:4133` and `:7266` write the same `dst+0x1C` / `dst+0x20` descriptor fields — same struct, so any
  header-type correction should be checked against those use sites too.
