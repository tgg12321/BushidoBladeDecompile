# Evidence bank — motion_SetExMotion

- [s1] Queue entry: distance=1453, verdict=ASM-PARTIAL, rules=1. The single rule is asmfix.txt:220 `motion_SetExMotion: replace_with_asmfile "asm/funcs/motion_SetExMotion.s"` — whole body is original asm; src/text1b.c:14815 is a stub returning 0.

- [s1] Structure (tmp/blitz/motion_SetExMotion_map.txt): 1456 insns @0x80065800-0x80066EBC, frame 0x58, saves s0-s7+fp+ra (all callee-saves). Single jr $ra at 0x80066EB8, returns constant 1 (u8 to callers).

- [s1] Signature: u8 motion_SetExMotion(s32 mode), mode 0..0x11 (18 values). 18 call sites in src/text1b.c:14624-14800 each pass a distinct constant 0..0x11 — per-effect wrapper functions.

- [s1] Two 18-entry jumptables: jtbl_800158F8 (dispatch at 0x800659DC, switch on mode for color/texture setup) and jtbl_80015940 (dispatch at 0x80066160, switch on mode for poly corner placement). Both defined as const u32 arrays in src/text1a_b_pre_rodata.c:365,387. Both switches gated by `sltiu $v0,$s3,0x12; beqz`.

- [s1] GTE region 0x80065874-0x80065958 (asm lines 29-91, all splat-marked 'handwritten instruction'): ctc2 $0-$4 = SetRotMatrix from *D_800A3474; ctc2 $5-$7 = SetTransVector from s0+0x14; lwc2 $0,$1 vertex from s0+0x64; rtps; swc2 $14 (SXY2) -> *D_800A34B8; swc2 $8 -> s0+0x10; cfc2 $31 (FLAG) -> *D_800A34CC; mfc2 $19 (SZ3) >>2 -> *D_800A34D0. Canonical GTE inline-asm category per inline-asm-policy — this is why verdict is ASM-PARTIAL. A call to func_8007EA0C is interleaved mid-region (args s0+0x54, s0+0x14).

- [s1] Full m2c decompile SUCCEEDS: tmp/blitz/motion_SetExMotion_m2c.c (637 lines, 0 structural failures; only the cop2 ops are M2C_ERROR placeholders). Required synthesizing the jtbls as .s (tmp/blitz/make_jtbl_s.py -> tmp/blitz/motion_SetExMotion_jtbl.s) because rodata segments were retired to C arrays.

- [s1] Semantics: draws a 0x28-byte POLY_FT4 effect prim. s0=*D_800A34EC (effect state struct), s2=*D_800A37D4 (prim cursor), advances by 0x28 with pool-bound check `-((s2-D_800A3720)*0x33333333)>>3 < 0x1C1` (magic div by 40); D_800A37D4 written back at exit. ot_Link(D_800A374C+4, prim) links into OT.

- [s1] The lone backward j (0x80066E24 -> .L80065988, span 1319 insns) is NOT a while loop: switch-2 cases 6/7/10/11 end with `if (mode < 9) { ot_Link; advance prim; mode += 2; goto top; }` — re-runs the whole body for the paired effect (mode+2). Loop label sits AFTER the GTE/RTPS region (at the D_800A34A8=0x20 store, 0x80065988).

- [s1] Switch 1 has cross-case control flow: case 5's else-arm falls into case 0's body (m2c lines 230-241: shared `var_v0_4 = &D_8009B8C8` + store block); cases 3/4 share tail block_43 with cases 8/9 (`*D_800A34A8 = var_v1; *D_800A34AC = var_v1`) via different var_v1 (0xC0 vs 0x100); cases 10/11 tail block_35 also reached from 6/7 via goto. Original source used gotos/shared labels across cases.

- [s1] Switch 2 case group 6/10 vs 7/11: case 6/10 head does a conditional `*s4 = -*s4` then FALLS INTO 7/11 (m2c lines 550-571) — a genuine C fallthrough after an if.

- [s1] Only one real div instruction (signed div for *D_800A34B0 / *D_800A34D0 = H*1000/SZ3 screen-scale); all other divisions are strength-reduced multiplies: /105, /3, /15, /10, /255, /4551 (m2c shows them literally).

- [s1] Second small loop 0x8006640C-0x80066458 (19 insns) = the do{...}while(var_s1<4) in switch-2 case 1/2/16/17: 4 corners, sign flip by (i&1)/(i&2), func_8007E8AC(vec, dst) per corner after motutil_GetWalkDir + gte_SetRotMatrix(sp10).

- [s1] 24 calls / 11 unique callees: math_Sin x9, math_Cos x5, ot_Link x2, func_8007EA0C, gte_GetH, initPolyFT4, gpu_SetRawTexture, gpu_SetSemiTransp, motutil_GetWalkDir, gte_SetRotMatrix, func_8007E8AC. gte_SetRotMatrix is called AND inlined-via-ctc2 in the prologue — the prologue block is hand-inline GTE, the case-1 call is the library fn.

- [s1] No WIP checkpoint existed (memory/wip/ has only README). Ledger was a bare init_ledger skeleton (session_count=0) from a prior aborted blitz.

- [s1] regfix.txt contains NO rules for this function (grep clean) — the only cheat is the asmfix replace_with_asmfile.

## 2026-09-30 -- laneA manual session 2 (reset name func_80065800): first full C body, sandbox 57

candidate.c is a complete C body. It uses dev alias names TMRx/POSx for the per-mode arrays at
0x800F0BA8 (s16[18]) / 0x800F0CA0 (12-byte records[18]), which this TU still declares as ~70
per-word scalars (D_800F0BA8.., D_800F0CA0..). A real landing needs that aggregate merge across
the ~40 consumers in text1b_tu1c.c, plus moving jtbl_800158F8 / jtbl_80015940 out of
src/text1a_b_pre_rodata_b.c. text1b_tu1c.o(.rodata) links right after it, and this function's
two compiler tables land first in it, at 0x800158F8.
Score 57 at 1452/1454 insns (alias-symbol addends account for ~7 operand-only hunks).
Progression (tmp/f65800/): m2c draft 423 -> 405 (rgb store order, case 5 / case 0 shared else) ->
369 (duplicated size stores, n local in case 12) -> 216 (explicit shared-quad code) -> 112 (y
read as `*(s32 *)D_800A34B8 >> 16` = lh 2(a0), the sibling idiom) -> 91 (12-15 / 8-9 `goto quad`
into case 3/4's coordinate code) -> 57 (w/h locals in the corner loop; separate scale locals).
Findings:
- 11-ish GTE islands (inline_c.h: SetRotMatrix, SetTransMatrix(outer), ldv0, rtps (post-DMPSX
  0x4A180001), stsxy, stdp, stflg, stszotz).
- Cross-jump ordering (BB2_XJUMP_DEBUG, tmp/f65800/dB/cc1.err): jump2 tries chain partners in
  reverse insn order, and jumps redirected to NEW labels leave the chain. With plain duplicated
  code, case 5's last two insns grab the 12-15 / 8-9 / 3-4 tails, giving three copies instead of
  one. An explicit `goto quad` (own-label cross-jump against 3/4's code before `quad`) reproduces
  the target's single copy exactly.
- fold-const.c split_tree reassociates `w * (X * 25)` into `(w * 25) * X`; the target keeps
  (X*25)*w, which we got with a separate local (sw / sh). This is a named-intermediate
  candidate, so it needs FAKE paperwork or a better spelling.
- Residual real diffs: case 10 t-pointer insn order; case 12 then-arm reload of TMR (CSE path);
  loop init order (move s1,zero / move s3,s7).

## 2026-09-30 -- laneA manual session 2 (cont.): sandbox 0 on the merged TU; landing prepared

candidate.c == the body staged in src/text1b_tu1c.c (verbatim). Sandbox --disable all on the
spliced src: 0 (1454/1454, 0 source-level / 0 operand-only hunks); full build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa (lock.ps1 rebuild).
Landing package (memory/grind/func_80065800/tools/land.py, run from the repo root under the lock):
- Aggregate merge: include/game.h declares `s16 D_800F0BA8[18]` (per-mode timers) and
  `Unk800F0C10Record D_800F0CA0[18]` (per-mode position records); the 78 per-word externs in
  text1b_tu1c.c are gone and every consumer (func_80064E90..func_800657B0, 28 functions) uses
  element/member access. Each consumer sandboxes 0. Only func_80065680 changed shape: its u16
  pointer local became `s16 *` (the element type), so `v0 = *v1 + 1; *v1 = v0;` (now lh, target
  lhu) is spelled `*v1 = *v1 + 1;` (HImode add, lhu), and the `(s16)` cast on D_800F0BA8[14] is gone.
- Alias rows retired: 62 undefined_syms_auto.txt rows (D_800F0BAA..D_800F0BCA,
  D_800F0CA4..D_800F0D74) and 65 named_syms.txt g_motion_ex_* rows naming bytes inside the two
  arrays; the prose in the rows that stay now names D_800F0BA8[k] / D_800F0CA0[r].
- jtbl_800158F8 / jtbl_80015940 deleted from src/text1a_b_pre_rodata_b.c: this function's two
  switch tables are now compiler output at the head of text1b_tu1c.o(.rodata), which bb2.ld
  links right after text1a_b_pre_rodata_b.o, landing at 0x800158F8 / 0x80015940 (build SHA1).
- Canonical row + 8 region hashes (the eight inline_c.h islands, gate ASM-PARTIAL 15/1454).
Closing the last residuals (see the evidence/ files):
- case 12-15: no pointer local; D_800F0BA8[arg0] at every read with the D_800A3488 store before
  n = 10 - D_800F0BA8[arg0] reproduces the target's second timer load (evidence/case12-store-order.txt).
- case 10/11: `tbl = D_800F0BA8; t = tbl + arg0;` (pointer alias, FAKE) gives the base its own
  register ahead of the shift (evidence/case10-base-register.txt: .rtl/.greg for this form,
  `t = D_800F0BA8; t += arg0;` 7, `&D_800F0BA8[arg0]` 8, no pointer 11, base at entry 8).
- case 5 / case 0: a plain duplicated case 0 body after case 5 gives the same bytes as the old
  `case 0:` label inside case 5's else (sandbox 0, build SHA1); the plain form landed.
- sw / sh / w / h: FAKE named intermediates (evidence/named-locals.txt: inline forms 14 / 13 /
  11 / 92; the ?: corner spelling 12).

## 2026-09-30 -- BANKED at session close (landing prepared, not reviewed)
The owner closed the session before layer-2 ran. The landing package above was staged, then
reverted (src/, include/, *.txt, regions json back to HEAD by reverse-applying laneA's own
patch); main still carries INCLUDE_ASM. Floor: sandbox 0 (1454/1454) with the merge applied,
full build SHA1 == oracle (measured under the lock, 2026-09-30). To resume: under the landing
lock run `python3 memory/grind/func_80065800/tools/land.py` (it reads tmp/f65800/final.c = a copy
of candidate.c), append landing/row.txt to inline_asm_canonical.txt, add the 8 region hashes
(landing/addrh.py), rebuild, sandbox the 28 consumers, precheck, layer-2. Commit message drafts:
landing/msg_auth.txt, landing/msg_match.txt. layer2 hash of the staged body was c428aa424743db26.

## 2026-09-30 (evening) -- laneC: package re-measured on current main; the TU cut must move

- candidate.c: dropped the unused block-scope `extern s32 D_800A3724;` (no codegen effect).
- Package still holds after the rodata-object-alignment adoption and the Q62 COMMON model:
  tools/run.sh (scratch TU via tools/mk.py, msbx.py) scores func_80065800 4/1454, the only
  scored hunk being the jtbl-2 `lw v0,72(at)` addend (build/ still carries the transcribed
  tables; reference artifact), and all 28 consumers of the merged bytes
  (func_80064E90..func_800657B0) 0 (func_800645B0 / func_800646E8, also scored 0, touch only
  D_800F0BCC / D_800F0D78, outside both arrays). Full scratch build of
  land.py (tools/scratch.sh pkg, /tmp/laneC/pkg): EXE sha1 == oracle 62efab4f...; text1b_tu1c.o
  .rodata at 0x800158F8.
- TU boundary (rodata-object-alignment rule, docs/grind/rodata-align-2026-09-30.md section 8
  caveat): func_80065800's tables are phase 0 (0x800158F8, 0x80015940), so its TU starts its
  rodata at 0x800158E0 or 0x800158F8. The strings at 0x800158B4 (snd_LoadCommonVab) and
  0x800158CC (func_8005C2A8) then belong to text1b.c's TU (rodata from 0x8001585C, phase 4), and
  the snd_Init cut (rodata start 0x800158B4, phase 4) is no longer a surviving position once the
  tables are compiled. 0x800158E0 is D_800158E0 "eff prim over :%d \n" (20 bytes, func_80061064's
  printf) + 4 zero bytes; with the TU starting there the zeros are exactly the .align 3 pad before
  the tables. 0x800158F8 needs the zeros to be an unreferenced "" item in text1b.c's TU. Both give
  identical bytes; the earliest (0x800158E0, cut at func_80061064) is the rule's convention.
- tools/move.py (after land.py): snd_Init's extern block .. func_80060E38 (2925 lines, 51
  functions) appended verbatim to text1b.c; text1b_tu1c.c's header rebuilt by the rodata-align
  splitc.py; the moved block's CVECTOR/DVECTOR typedefs dropped (text1b.c has them from
  include/gte.h:27-28); D_800158E0[24] moved verbatim into text1b_tu1c.c in place of
  func_80061064's conflicting `extern s32 D_800158E0;`. Scratch build (tools/scratch.sh move,
  /tmp/laneC/move): EXE sha1 == oracle; text1b_tu1c.o .rodata 0x800158E0 (0xd0),
  text1a_b_pre_rodata_b.o 0x800158B4 (0x2c). Awaiting the orchestrator's go/no-go (it touches
  text1b.c and the records of func_8005C8A8 / func_8005D554 / func_8005D814 / func_8005E54C /
  func_8005F1C8).

## 2026-09-30 (evening, cont.) -- laneC: boundary commit prepared (orchestrator GO on option 1)

The text1b | text1b_tu1c boundary move (tools/move.py, now also moving func_80065800's two
transcribed tables verbatim to just before its INCLUDE_ASM line: without that, D_800158E0 in
text1b_tu1c.o would land after them and the scratch build mismatches) applied on main under the
landing lock: lock.ps1 rebuild EXE sha1 == oracle; text1b_tu1c.o .rodata 0x800158E0 (0xd0).
tools/verbatim_check.py: text1b.c == HEAD + moved block (minus the CVECTOR/DVECTOR typedefs),
text1b_tu1c.c body == HEAD tail with the two relocations. tools/implicit_cmp.sh: union of implicit
declarations equal (24 names). Records via tools/relocate.py. Record: rodata-align doc section 9.
The Match follows the boundary layer-2; land.py must then delete the tables from text1b_tu1c.c
(not text1a_b_pre_rodata_b.c).

## 2026-09-30 (night) -- laneC: boundary landed (5c543ce1d, layer-2 rev-boundary PASS); Match prepared

tools/land.py now deletes the transcribed tables from text1b_tu1c.c (where 5c543ce1d put them).
tools/land_text1b.py drops the four unused per-word externs D_800F0D30/34/3C/40 that the boundary
move carried into text1b.c (bytes inside D_800F0CA0[12..13]; rows retired by land.py). On the
spliced tree under the landing lock: lock.ps1 rebuild EXE sha1 == oracle; `sandbox func_80065800
--disable all` 0 (1454/1454, 0 hunks); canonical ASM-PARTIAL 15/1454;
check_completion_integrity OK. The staged body == candidate.c. Canonical row appended from
landing/row.txt; region hashes via landing/addrh.py (8). Messages: tmp/func_80065800/msg_auth.txt,
msg_match.txt (Pure-C attempts blocks added).

Layer-2 round 1 (rev-65800, body b572e1adbcd9f19c, 2026-09-30): FAIL, text only; recorded in
layer2.jsonl. Fixed: the gte_rtps island comment gives both words (0x0000007f -> 0x4A180001);
the function-header comment and msg_match.txt say only modes 6/7 re-run (guard `arg0 < 9`,
target 0x80066DC4 slti $v0,$s3,0x9); consumer count 28 (func_800645B0 / func_800646E8 touch only
D_800F0BCC / D_800F0D78). Advisory: evidence/named-locals.txt now cites cc1 -dr dumps of both sw
forms (evidence/sw-rtl/, tools/dump_sw.sh). landing/msg_*.txt refreshed from the staged messages.

## 2026-09-30 -- LANDED (COMPLETED-INLINE-ASM-CANONICAL)
Layer-2 round 2 PASS (rev-65800-r2, body b572e1adbcd9f19c; round 1 rev-65800 FAIL on text only,
fixed): 1454/1454 0 hunks, 8/8 region hashes, islands verbatim inline_c.h with the rtps word under
the 2026-09-24 extension, aggregate-merge prongs (28 consumers), FAKE prerequisites (tbl, sw, sh,
w, h; sw -dr dump), text fixes. Commits: 5c543ce1d (TU boundary at func_80061064, rev-boundary
PASS), ace33f8d9 (auth row), 1f6023c7d (Match; staged body == candidate.c), 3b7664be4 (queue
done). check_completion_integrity OK after landing.
