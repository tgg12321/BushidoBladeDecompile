# func_80036140 — evidence (manual lane, slotB2, 2026-09-26)

CD audio/XA playback state machine in src/code6cac_b2_post.c (same original module as
cdrom_SetMix / cdrom_ReadyCallback / func_80036940). Head: while a CD-mix fade is active
(D_800A3854 > 0) it interpolates each of the four CdlATV bytes between g_cd_atv (current) and
D_800A36B8 (target) by D_800A3840/D_800A3854, calls CdMix(&local), steps the counter and, at the
end, copies the target ATV over the current one. Body: `switch (D_80101E60.unk02)` over states
0x10..0x1E (15-entry ADDR_VEC, jtbl_80010938; case source order 10..16, 1C, 1D, 1E, 17..1B
follows the target block order). Every CdSync/CdReady case shares the tail
`else if (ret == 5 /*CdlDiskError*/) state = 0x17;` (cross-jumped in the target).

## Floors (all measured this session)
| body | model | score |
|---|---|---|
| candidate.c (split symbols, byte-wise ATV copy) | current headers (sandbox) | **50/512** |
| rejected/atv-cast-pun-36.c (`*(CdlATV_T *)&g_cd_atv = *(CdlATV_T *)&D_800A36B8`) | current headers (sandbox) | 36 — BANNED construct (per-use pointer pun) |
| integration/record-merge-only-body-16.c | ReplayCamRec extended to 0x80101EA7, split ATV bytes | 16 (scratch whole-file build, all 32 other fns 0) |
| integration/full-model-body.c | record merge + CdlATV merge, -G0 | 34 (siblings cdrom_SetMix 10, func_80035F78 6) |
| same | record merge + CdlATV merge, **-G8** | 14 (siblings 6/6) — only the +1/+2/+3 ATV bytes gp-rel'd |
| same | record merge + CdlATV merge, -G0 + `.comm` knowledge | 22 (cdrom_SetMix 5) |
| same | record merge + CdlATV merge, **-G8 + `.comm` knowledge for g_cd_atv / D_800A36B8** | **2** — the ONLY diff is the jtbl `lw %lo(jtbl)` operand (rodata placement artifact); all 32 other functions in the file 0 |

Scratch tools (integration/): xb.py = whole-file build with an include-override dir, optional
`--g8` (CC_FLAGS_GP) and `--comm a,b` (prepends `.comm sym,4` lines before maspsx = what cc1 emits
for a TU-defined tentative definition), scores EVERY function in the file vs build/src; diff.py =
scored hunks; gp_off_census.py = census below. Headers/src used: integration/*.patch.

## Proven facts
1. **The CD-read globals 0x80101E78..0x80101EA7 are aggregate members.** Target accesses to
   E88/E8C/E9C/EA4 are `la reg,sym; lw 0(reg); ...; sw 0(reg)` (and s0 = &E88 kept across a call in
   case 0x15). GCC 2.7.2 only forms that when the MEM comes from a COMPONENT_REF/ARRAY_REF
   (explow.c memory_address force_reg; DECL_RTL of a plain scalar is used directly) — tmp test
   t1/t3: every scalar spelling (++x, x++, x+=1, x=x+1) is direct, struct member / array element is
   `la`+0(reg). Independent (census, 2026-09-24, pre-dates this session) base+offset evidence in the
   ORIGINAL binary: func_80036940 80036A74 `s0 = &D_80101E8C; s0 -= 0x20` (-> g_cd_loc 0x80101E6C) and
   80036B08 `a1 = &D_80101E98 - 0x2C` (-> 0x80101E6C). cse can only relate two constant addresses
   (use_related_value) when they share one symbol base, so 0x80101E6C..0x80101E99 is ONE object.
   E9A/E9C/E9E/EA0/EA4 (past E99): only this function's member-form codegen (+ game_FrameLoop takes
   &E9E) — no independent base+offset evidence found (scan of every addiu %lo formation into
   0x80101E58..0x80101EA7 across asm/funcs). Same class as the func_800620B8 open question.
   Extending ReplayCamRec through 0x80101EA7 (integration/code6cac.h.patch) keeps all 32 other
   functions in the file at score 0 (only consumers: code6cac_b2_post.c; grep src/ include/).
2. **The ATV copy needs a CdlATV aggregate.** `lwl/lwr/swl/swr` at 80036308 is move_by_pieces of an
   unaligned 4-byte BLKmode object = struct assignment `g_cd_atv = D_800A36B8`. With the splat
   per-byte symbols the only C for it is a per-use pointer pun (auto-FAIL, dossier). CdMix takes
   CdlATV* (libcd.h), cdrom_SetMix stores all 4 bytes then CdMix(&g_cd_atv) (naming manifest
   2026-09-24 CONFIRM rows 0x800A3718/19).
3. **The CdlATV merge cannot match in the current build model — two independent gaps:**
   a. *Cost model (-G0).* In the target val0 of both objects is a gp-rel direct access and the copy
      re-materializes both addresses (`la a1/a0`). Under -G0 cc1 prices symbol_ref at 2 vs reg 1
      (mips_address_cost), so cse keeps the forced address pseudo and shares it between the val0
      read and the copy across the CdMix call (s1/s0). Under -G8 the objects are small data
      (SYMBOL_REF_FLAG, cost 1) and the sharing disappears — measured.
   b. *Assembler rule for `sym+N`.* Target: base byte gp-rel, bytes +1..+3 `lui/%lo` (in
      cdrom_SetMix, func_80035F78 and here). maspsx gp-rel's `sym+N` for every injected sdata extern
      unless the symbol is `.comm` in the TU (`gp_allowed = gp_allow_offset or sym not in
      comm_symbols`, maspsx/__init__.py:786) — i.e. ASPSX never gp-rel'd an offset into a COMMON
      symbol. So in the original TU these two objects were tentative definitions (`CdlATV x;`).
      Global "no gp for extern+offset" is NOT an option: census (gp_off_census.py) finds 9 matched
      sites that need it (text1a_post func_80041E10/80041EB0 g_anim_select+2/+4; text1b
      func_80061C00 D_800A34F0+2, func_8006E534 D_800A3588+2/D_800A358C+2).
   Defining the objects in C (real `.comm`) is not viable: cc1 emits `.comm sym,4,1` which maspsx
   cannot parse (same as CD_cw), and maspsx would then emit .bss storage in b2_post.o, conflicting
   with the undefined_syms_auto.txt address.
4. **-G8 on this file is byte-neutral for every other function's instructions** (all 32 score 0),
   BUT -G8 makes cc1 buffer function bodies (TARGET_FILE_SWITCHING), so the file-scope INCLUDE_ASM of
   func_80036940 floats to the top of the object (measured: func_80036940 at .text+0). A -G8 landing
   needs func_80036940 out of the TU (LINKED_ASM_FUNCS like save_vc_ctrl) or a TU split around it.
5. **Rodata placement.** GCC emits the 15-entry table into code6cac_b2_post.o(.rodata) after the two
   func_80035828 tables (0x4C bytes at 0x800108EC). With .align 3 it lands at +0x50 = 0x8001093C; the
   target is 0x80010938. The original placement (0x938 and func_80036940's table at 0x978 after a
   zero pad word) says the CD module is its own original TU whose rodata starts 8-aligned at
   0x80010938. Landing options: (i) TU split so func_80036140's TU rodata starts at 0x938 (faithful;
   rodata_post keeps a leading zero word until func_80036940 lands), or (ii) add code6cac_b2_post to
   RODATA_ALIGN2_FILES (precedent func_800747D8 / text1b) + leading zero word in rodata_post. Either
   way delete jtbl_80010938 from src/code6cac_b_rodata_post.c.

## Constructs in the full-model body (for the eventual reviewer)
- local `CdlATV atv` (sp+0x10) built field by field, `CdMix(&atv)`; `g_cd_atv = D_800A36B8;`
- `ret = CdSync(1, &g_cd_result)` then `if (ret == 2) ... else if (ret == 5) state = 0x17;`
- state 0x13: `CdControlF(D_80101E60.unk34 ? 0x1B : 3, 0)`; 0x1B: `unk02 = unk08 ? 0 : 0x10`
- `if (D_80101E60.unk3C++ > 0x3C)`; `D_80101E60.unk44 -= 4; if (<= 0)`; one block-local `s32 pos`
  in case 0x16 (single role: CdPosToInt result). No multiply-written locals besides `ret` (one
  role: CdSync/CdReady status, rewritten per case) — check Ruling 5/6 if a reviewer objects.
- externs added locally: CdSync, CdReady, g_cd_result (+_plus_0x3, _plus_0x5) as u8 (8-byte
  CdlResult split by splat; base gp-rel, +3/+4/+5 lui — same .comm story; the split spelling matches
  and no aggregate is needed for it).
