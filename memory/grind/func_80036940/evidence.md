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
