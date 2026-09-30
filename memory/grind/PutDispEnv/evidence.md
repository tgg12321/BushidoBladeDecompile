# PutDispEnv — evidence (manual session 2026-09-23)

COMPLETED-C in de62daf26 (layer-2 cheat-reviewer PASS; oracle SHA1 match).

- Reference: Xeeynamo/psyz decomp/src/libgpu/sys.c `PutDispEnv` (local copy
  tmp/psyz-ref). Adopted near-verbatim; first measurement scored 28.
- The whole 28 came from binding `&g_gpu_disp_env` to a local pointer: GCC
  CSE'd the base into $s4 (extra save/restore, `lhu N(s4)` instead of
  per-access `%hi/%lo`). Addressing the global directly through a
  `(*(_dispenv *)&g_gpu_disp_env)` view (the reference's `info.disp` is a
  static global) scored 0.
- Residual sandbox hunks are masked reloc addends only (target relocs name
  split D_8009BEE8/…, ours g_gpu_disp_env+off; same address).
- `candidate.c` is the landed body, kept for provenance.

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The de62daf26 landing FAILed the 2026-09-29 retro-audit: macro-hidden `(volatile _dispenv_rect *)` cast on the non-IRQ RAM global g_gpu_disp_env (DISP_RECT_EQ), plus the `(*(_dispenv *)&g_gpu_disp_env)` per-use pun; 'Sony semantics' came from psyz, a decompilation. Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", PutDispEnv);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. Removed with the body: the PutDispEnv-private _dispenv_rect/_dispenv typedefs and the DISP_RECT_EQ/CLAMP/info macros (+ their #undefs); no other code uses them. Kept: the four extern/prototype lines the landing added (D_8009BE77, D_80015FF8, GetVideoMode, get_dx), now unused before get_dx's definition.

## 2026-09-30 -- laneA fix-forward (candidate.c)

Frontier = the two REOPENED objections. Both removed without a macro or an object pun:
- **Per-use pun `(*(_dispenv *)&g_gpu_disp_env)` removed**: the splice retypes the declaration
  `extern u8 g_gpu_disp_env;` (display.c:25) to `extern DISPENV g_gpu_disp_env;` with the public
  LIBGPU.H `RECT`/`DISPENV` layout, so every access is plain member access. Only other consumer in
  src/ is GetDispEnv (`memcpy(a0, &g_gpu_disp_env, 0x14)`, unchanged text, void* param).
- **Macro-hidden volatile rect cast removed**: open-coded `*(volatile s16 *)&g_gpu_disp_env.screen.x`
  per field (8 sites), FAKE-annotated, on Q55 (matched-SOTN precedent overrides the
  interrupt-touched-only refusal) citing tmp/sotn-decomp `src/main/psxsdk/libspu/s_m_m.c:48 @aa53500`
  (SpuMalloc, `c` segment in config/splat.us.main.yaml:242): `*(volatile int *)&_spu_memList[var_s2].addr`,
  a use-site volatile read of a non-IRQ RAM global member. SOTN's PSX build compiles with
  bin/cc1-psx-26 (tools/builds/gen.py:777), i.e. its PS1 build, not our exact GCC 2.7.2.
  Byte evidence (also meets Q48's shape): every global-side field load in both rect compares is
  `lhu; sll 16; sra 16` while the env-side operand of the same compare is `lh`.
  Simpler spellings measured (alias harness, tmp/PutDispEnv/): plain member reads 75 (289/298 insns,
  lh folded; banked rejected/nonvolatile-75.c, alias-harness text with g_gpu_disp_env_x); u16 fields + (s16) cast 75; (s16)(u16) cast 75;
  `((volatile RECT *)&g.screen)->x` and `extern volatile DISPENV` both score like the chosen form (20 = alias artifact only) (not chosen: a
  type-level claim with no IRQ writer; the use-site form is the one SOTN ships).
- isinter word compare `*(s32 *)&g_gpu_disp_env.isinter != *(s32 *)&env->isinter` kept (single lw
  +0x10 both sides): SOTN `src/main/psxsdk/libgpu/sys.c:367 @aa53500` LOW() = `*(s32*)&` (common.h:73),
  same statement in SOTN's matched PutDispEnv.
- Mode-width chain respelled to SOTN's nested `if (w > 280) { ... }` (byte-identical to the old
  empty-arm chain). The h test keeps an empty then-arm = SOTN sys.c:394 verbatim form
  (`env->pad0 ? 288 : 256`); FAKE-annotated. Measured alternatives: `if (h > (pad0?288:256))` 25
  (302 insns); `!(h <= ...)` 24; `h > (!pad0 ? 256 : 288)` 24; `(pad0?288:256) < h` 25.
- Alias-harness score 20 = 8 operand-only reloc-addend hunks from the unresolved alias symbol only;
  the real splice is measured under the lock.
- Unchanged from the byte-exact landed body: `get_dx((s16 *)env)` (get_dx's matched prototype),
  `memcpy((s32)&...)` (display.c:14 prototype), the open-coded clamps (old CLAMP macro expanded).
- Prepared for landing 2026-09-30 (laneA, under the landing lock): candidate.c (+ provenance
  header) spliced into src/display.c with the RECT/DISPENV typedefs and the
  `extern DISPENV g_gpu_disp_env;` retype. Spliced-src sandbox --disable all = 0 (298/298,
  0 hunks); lock.ps1 rebuild SHA1 = oracle. layer2 hash 3e7973088e3625f9. Awaiting layer-2.

## 2026-09-30 -- layer-2 round 1 FAIL (l2-PutDispEnv-r1, body 3e7973088e3625f9) and round-2 fixes

Recorded in layer2.jsonl; body banked rejected/l2-r1-fail-3e7973088e3625f9.c. HELD: the 8 volatile
reads (Q50/Q55, s_m_m.c:48), the isinter LOW compare (sys.c:367), the empty arm (sys.c:394).
FAILED on, and fixed in round 2 (all under the landing lock, same session):
1. Clamp reuse had no inline SOTN tag. Added `SOTN: src/main/psxsdk/libgpu/sys.c:358 @aa53500`.
2. Prong (c): retired undefined_syms_auto.txt D_8009BEE2/E6/EA/EE and named_syms.txt
   g_gpu_disp_env_field_e2/e6/ea/ee_*. No live referrer: PutDispEnv.s is no longer INCLUDE_ASM'd,
   asm/text2.s is not in bb2.ld/Makefile, and 7D920.data.s defines its own dlabels. One C handle:
   `memset(&g_gpu_disp_env, ...)` in SetDispMask measured 14 (36/39). The target addresses the
   bytes off the debug_level base (asm/funcs/SetDispMask.s:22 `addiu $a0,$s1,0x6A`), so the bytes
   are a member of the GPU state block. GpuCtx.disp_env is now typed DISPENV and the memset takes
   `&...->disp_env`. Sandbox: SetDispMask 0, GetDispEnv 0.
3. Prong (d): RECT/DISPENV + `extern DISPENV g_gpu_disp_env;` moved to include/gpu.h.
4. SOTN tag comments reworded to name no SOTN symbols.
Round-2 splice: PutDispEnv sandbox 0 (298/298, 0 hunks); lock.ps1 rebuild SHA1 = oracle.

## 2026-09-30 -- layer-2 round 2 FAIL (l2-PutDispEnv-r2, body 3e7973088e3625f9; SetDispMask c57216b13ab924fa -> bc25eca16a8a4137)

Recorded in layer2.jsonl. Banked: rejected/l2-r2-fail.c (body) + rejected/l2-r2-fail-splice.patch
(the whole staged splice: display.c, gpu.h, named_syms.txt, undefined_syms_auto.txt). Reverted
(reverse-applied patch, index + tree); rebuild SHA1 = oracle; landing lock released.
Verified clean by the reviewer: citations, retired rows, sandbox 0 on all three functions.
Findings = the new frontier:
1. Prong (c): no single C object over 0x8009BEE0..+0x14 -- g_gpu_disp_env, the display.c
   `((GpuCtx *)&g_gpu_type)->disp_env` view, and gpu.c ResetGraph's GpuConfig `(u8 *)s0 + 0x6C`.
2. `(GpuCtx *)&g_gpu_type` is a per-use pun over an extern u8 (already on main in COMPLETED
   SetDispMask), TU-local typedef, no SOTN citation. Retyping it extends it.
3. D_8009BE77 is a second name for the byte g_gpu_dither (gpu.h) names.
Required fix: ONE real object for the 0x80-byte GPU state block at 0x8009BE74 in include/gpu.h
(DISPENV disp_env at +0x6C), every consumer through members (SetDispMask, PutDispEnv, GetDispEnv,
ResetGraph, PutDrawEnv/DrawOTagEnv base+0xE, all others), byte-neutral per consumer, alias rows
retired, name from evidence. Landing = PutDispEnv match + cheat-cleanup of changed consumers.

## 2026-09-30 -- round 3: the GPU state-block merge (prepared under the landing lock, laneA)

The reviewer's hypothesis held. With ONE object, cse's related-value addressing reproduces the
target's base-relative forms on its own: `&g_gpu_ctx.disp_env` = base+0x6A in SetDispMask, and
the `g_gpu_ctx.draw_env = *env` struct copy = base+0xE plus the 16-byte movstr loop in
PutDrawEnv/DrawOTagEnv. The `u8 *base` locals, the `_drawenv_q` manual copy loop and the
`(GpuCtx *)&g_gpu_type` pun become unnecessary. First measured with an alias harness
(tmp/PutDispEnv/gs/*.c: every natural spelling = only the unresolved-alias addend hunk), then on
the real splice.
- include/gpu.h: RECT, DR_ENV, DRAWENV, DISPENV (LIBGPU.H layouts) and `GpuCtx` (0x80 bytes), with
  `extern GpuCtx g_gpu_ctx;` as the one C handle for 0x8009BE74..0x8009BEF3. The evidence comment
  names ResetGraph's 0x80 clear and the base-relative addressing. Member names restate the owning
  API: type, queue_mode, debug_level, reverse, width, height, unk08, drawsync_cb, draw_env, disp_env.
- Every consumer is respelled through members. display.c (23 bodies): ClearOTag ClearOTagR DrawOTag
  DrawOTagEnv DrawSync DrawSyncCallback GetDispEnv GetDrawEnv PutDispEnv(new) PutDrawEnv SetDispMask
  SetDrawEnv SetDrawEnv2 _addque2 _clr _drs _dws checkRECT get_ce get_cs get_dx get_mode get_ofs.
  gpu.c (6): GetGraphDebug GetGraphType ResetGraph SetGraphDebug SetGraphQueue SetGraphReverse.
  Rewritten (not only renamed): SetDispMask (pointer local dropped), PutDrawEnv/DrawOTagEnv (typed
  DRAWENV *env, struct copy), ResetGraph (GpuConfig pun + s0/idx locals dropped, sizeof()),
  SetGraphReverse/SetGraphDebug/SetGraphQueue (pointer-alias locals dropped).
- Removed: display.c's TU-local GpuCtx typedef and per-field externs, gpu.c's GpuConfig and
  `extern u8 g_gpu_interlace;`, gpu.h's per-field externs.
- Retired rows (C-only aliases; asm/data/7D920.data.s still carries its own dlabels for the
  storage; _exeque.s's D_8009BE7C/D_8009BE80 need no rows): named_syms g_gpu_type..g_gpu_disp_env
  (9 rows, replaced by g_gpu_ctx), g_gpu_loop_flag, g_gpu_disp_env_field_*; undefined_syms_auto
  D_8009BE74/75/76/77/78/7A, D_8009BEE2/E6/EA/EE, g_gpu_type/interlace/draw_mode/draw_env/disp_env;
  symbol_addrs g_gpu_* (9 rows, replaced by g_gpu_ctx).
- Result: lock.ps1 rebuild SHA1 = oracle on the first try. sandbox --disable all = 0 with 0 hunks
  on all 29 changed functions (tmp/PutDispEnv/gs/final_summary.txt).
- Mechanism, dump-verified by the round-3 layer-2 reviewer (l2-PutDispEnv-r3):
  memory/grind/PutDispEnv/evidence/cse-related-value.txt. In a SetDispMask probe, .rtl insn 32
  `const(g_gpu_ctx+108)` becomes `plus(reg,106)` in .cse (cse.c use_related_value :1781).
- Open cleanup leads, deliberately NOT in this landing because they would change reviewed bodies:
  (b) type drawsync_cb as a function pointer (DrawSyncCallback/_addque2 treat it as u32);
  (c) the `(u32 *)g_gpu_dev_table` word views in display.c/gpu.c (see the gpu.h GpuDevTable
  comment on MEM_IN_STRUCT_P) -> GpuDevTable member calls, each to be measured.
- Stale prose in named_syms.txt (the retired-name notes at 0x8007B244 / 0x8007B3A8 / 0x8007BAB4 and
  the g_gpu_init_msg_buf comment) now names g_gpu_ctx members, a comment-only change.
