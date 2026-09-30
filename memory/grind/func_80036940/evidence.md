# func_80036940 — evidence (manual lane, slotG, 2026-09-26)

CD-read state machine in src/code6cac_b2_post.c (the CD module; old WIP name special_camera_Exec,
Kengo nm_special_cam/special_camera_Exec). `if (state >= 0x10) func_80036140(); else switch (state)`
over 0..13 (14-entry ADDR_VEC jtbl_80010978, index = state, no bias): setmode 0xA0 -> CdSync ->
CdControl(CdlSetloc) -> CdSync -> CdReadyCallback(cdrom_ReadyCallback)+CdControlF(CdlReadN) ->
wait for the read (sectors_remaining 0 / <0 / timeout) -> error path (CdlNop status poll,
CdlInit/CdlReset style 0x13 + 4x VSync(4)). The prior WIP (memory/wip/func_80036940/, 2026-06-08,
floor 69) predates the jtbl rodata cleanup and the naming waves; its body was rewritten from the
asm this session.

## Floors (all measured 2026-09-26; score = masked distance, whole-file scratch build, xb.py)
| body / model | -G0 | -G8 | -G8 + COMMON(g_cd_atv,D_800A36B8) |
|---|---|---|---|
| candidate v1, CURRENT headers (split E78..E98 scalars), sandbox `--disable all` | 41 | – | – |
| same body, record extended (model: func_80036140's integration patches) | 12 | 12 | 12 |
| + E58-based outer record (E5C a member) — model2 (with 36140's full model) | 2 | 2 | 2 |
| + same, record-merge only, func_80036140 still INCLUDE_ASM — model3/model4 | **2** | **2** | n/a |

The residual 2 is ONLY the jump-table dispatch `lw v0,%lo(jtbl)(at)` operand (reference names the
external jtbl_80010978; ours is a section-relative .rodata reloc) — the same score-2 artifact as
func_8006B578 / func_80035828 (decisions.md). Every other function in code6cac_b2_post.c scores 0
in model4 (-G0 and -G8), including cdrom_StartAudio with its `(u8 *)s0 - 0xA` pun respelled as
`D_80101E58.filter`. So func_80036940 is instruction-complete in BOTH the -G0 in-place model and
the -G8 model; g_cd_result's gp read comes from the existing sdata_funcs.txt membership at -G0
(func_80036940 was already listed) and from -G8 small data in the -G8 model.

## Proven facts
1. **E6C..E99 is one object** (base+offset in the ORIGINAL bytes, predates this session — census
   2026-09-24, func_80036140 ledger fact 1): 80036A74 `s0 = &E8C; addiu s0,s0,-0x20` (-> E6C, the
   CdlLOC `pair`) and 80036B08 `addiu a1,s0,-0x2C` with s0 = &E98 (-> E6C). cse relates two
   constant addresses only when they share one symbol base.
2. **E58..E62 is one object** (base+offset in the ORIGINAL bytes): cdrom_StartAudio 800370AC
   `addiu a1,s0,-0xA` / 800370B0 `sb v0,-0xA(s0)` with s0 = &E62 (-> E58, the CdlFILTER passed to
   CdControlB(CdlSetfilter=0xD, ...)). E5C lies inside that object.
3. E60..E77 is already one record on main (ReplayCamRec, owner-granted FAKE structure 2026-08-10),
   linking (2) and (1): the whole E58..E99 run is one C object.
4. **The object extends through E9B and E9A is a member**: it has 4-byte members (E5C, E8C, ...),
   so its size is a multiple of 4 and it covers at least E58..E9B; E9A is accessed as a halfword
   (func_80036140), so it is a member, not a separate variable (a separate object cannot sit
   inside another object's size).
5. **Member form needs the aggregate.** Target increments of E98/E8C/E5C are
   `la reg,sym; lX 0(reg); ...; sX 0(reg)` — only from a COMPONENT_REF (explow.c memory_address
   force_reg); scalar spellings are direct (36140 ledger fact 1). Measured: with E5C a scalar
   the two `++E5C` sites stay direct (score 12); as `D_80101E58.unk04` they match.
6. **Switch shape:** the table starts at index 0 with no bias subtraction, so the switch needs a
   case at 0: `case 0: break;` (idle state; cdrom_IsIdle tests state == 0). Without it GCC
   subtracts 2 (score 16). case 0 alone is enough (0/1/7 all point at the end label).
7. **`ret`**: one block-scoped `s32 ret` per case (written once, read by `== 2` / `== 5`);
   identical score to a function-scope multi-write local, so no multi-write local exists.
8. **Rodata placement (in-place landing):** GCC emits the table into code6cac_b2_post.o(.rodata)
   after func_80035828's two tables (0x4C bytes at 0x800108EC). Target: jtbl_80010938 (still
   func_80036140's, INCLUDE_ASM, hand-transcribed with a trailing zero word) at 0x938, then
   jtbl_80010978 at 0x978, then rodata_post's strings at 0x9B0. Moving the jtbl_80010938[16]
   array verbatim from code6cac_b_rodata_post.c into code6cac_b2_post.c (after func_80035828)
   gives 0x4C + 0x40 = 0x8C -> 0x978 only with `.align 2` for cc1's table: code6cac_b2_post
   belongs on RODATA_ALIGN2_FILES by the mechanical census test anyway (jtbl_800108EC is 4 mod 8;
   decisions.md 2026-09-20 func_800747D8 census). With `.align 3`: 0x90 -> 0x97C (wrong).

## Scratch tools (tools/ here; run from the repo root under WSL)
- `xb.py` whole-file scratch build + per-function score vs build/src (`--base`, `--inc`, `--g8`,
  `--comm`, `--diff`); `mk_model4.py` builds the in-place landing model (header + src).

## LANDED 2026-09-26 — COMPLETED-C (687f92c3d, queue 78ea341ca)
Landed at -G0 in place in src/code6cac_b2_post.c (layer-2 PASS after one paperwork FAIL: prong (c)
named_syms.txt rows + honest ReplayCamRec/CdState annotations). Oracle SHA1 on full rebuild;
sandbox 0, 274/274. The E58..E9B CdState merge, rodata move (jtbl_80010938[16] now in
code6cac_b2_post.c) and RODATA_ALIGN2 membership landed with it.

**Next (owner Q10 plan, not done yet):** func_80036940 moves TOGETHER with func_80036140 into one
-G8 TU once func_80036140 is COMPLETED-C and the pending maspsx COMMON gate ruling lands (both
gp-read g_cd_result, 0x800A3760, so ruling (i) forbids splitting them). For that joint landing:
- this body already scores 2 (jtbl operand only) in the -G8 model (tools/xb.py `--g8`);
- still to bank for ruling (ii): cc1psx -G8 vs -G0 on the new TU for func_80036940's two
  `lbu %gp_rel(g_cd_result)` reads (80036C24, 80036C74);
- until then g_cd_result's gp read comes from func_80036940's pre-existing sdata_funcs.txt row;
- func_80036140's body must respell D_80101E60.* as D_80101E58.rec.* (record now in the header);
  its E9C..EA7 extension (unk3C.., expected_pos, unk44) still needs its own evidence;
- when func_80036140 lands, delete the transcribed jtbl_80010938[16] from code6cac_b2_post.c
  (the compiler emits it) — the trailing zero word at 0x80010974 must then come from somewhere
  (open: see hypotheses.md) — and retire the `alias ... retire with func_80036140` rows in
  undefined_syms_auto.txt / named_syms.txt (g_cd_loc).

## Q43 (2026-09-30): CdState shrunk to 0x80101E58..0x80101E99
Owner ruling Q43 + layer-2 (q2-review) HOLDS-WITH-CONDITIONS. Proven-fact 3 above ("E60..E77 is
one record by the 2026-08-10 FAKE grant") is superseded: the E60/E62<->E6C link is now proven by
cdrom_StartAudio's sched1 memory dependence (Q2 (a1)/(a2)); evidence, dumps, cc1/cc1psx runs and the
(a4') member table: memory/grind/cdrom_StartAudio/evidence.md. This function's own contribution is
fact 1 (E6C..E99 by 80036A74 / 80036B08); it does NOT need the E62<->E6C link (splits at 64/68 score
0 for it — tmp q2-80036940 investigation), and cuts inside E6C..E99 cost it 13/13/11/14 (74/78/98/6C;
memory/grind/cdrom_StartAudio/runs/span_sweep/span_cc1.txt). The earlier E9C..EA7 extension rested on
func_80036140, which went back to INCLUDE_ASM; those bytes are declared separately again (no proof
places them in the object; not proven separate), and this function's `sw` at 80036AAC stores to `g_cdread_expected_pos` (s32, a scalar). On the Q43 tree:
`sandbox func_80036940 --disable all` = 0 (274/274), full build == oracle.
