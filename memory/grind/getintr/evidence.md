# getintr — evidence

## 2026-09-23 manual upstream sweep — SOTN reference: build_insns 354 == target, only symbol-offset hunks

- Source: SOTN `src/main/psxsdk/libcd/bios.c` getintr (local clone tmp/sotn @8bd7c77).
  psyz (a438bda) still INCLUDE_ASM.
- candidate.c = SOTN body adapted to BB2 v1.86 bytes: DiskError path is
  `if (CD_debug > 0) printf("DiskError: "); if (CD_debug > 0) printf(com=...)` (not puts),
  error mask 0x1D, CD_status/CD_status1 are s32 (lw/sw). Inline `_memcpy` with the
  null-dst check (same helper as CD_cw's).
- Sandbox score 60 but 0 source-level hunks: all 6 hunks are operand-only offsets on
  `Intr_m.ready/.c` (+1/+2) vs target's split symbols D_800A1495/96. The candidate uses
  MEASUREMENT-ONLY placeholder externs (`*_m` names) because the real declarations
  (CD_intr typedef, `static volatile CD_intr Intr`, `_memcpy`) sit AFTER getintr in
  src/system.c (~line 650+), and `extern u8 CD_status` conflicts with s32 use.
- Symbol map: D_800A147C/80/84/88 = g_cd_index_reg/param_fifo/req_reg/irq_reg;
  D_800A1494..96 = Intr {sync, ready, c}; D_800A11C8 = CD_status1 (named g_cd_init_flag);
  D_800A11CC = CD_nopen; D_800A127C / D_800A137C = SOTN D_80032B68 / D_80032C68;
  D_800F19A0/A8/B0 = the three Result_t buffers (g_cdrom_callback_buf_a/b/_b_plus_8).
- LANDING needs: (1) hoist CD_intr/_memcpy + a tentative `static volatile CD_intr Intr;`
  above getintr; (2) `*(s32 *)&CD_status` accesses (TU precedent: CD_initintr/CD_init);
  (3) rodata re-attribution — the four strings + jtbl_8001622C (0x800161E4..0x80016240)
  are getintr's own literals, but live mid-text1a_b_post_rodata.c while system.o(.rodata)
  links after display.o. Needs the TU re-split recipe.

## 2026-09-23 — COMPLETED-C in f15cad3e4 (layer-2 PASS; oracle SHA1 match)

- Landed with real TU names; CD_intr/_memcpy hoisted above getintr; jtbl_8001622C now
  emitted into system.o(.rodata), linked between text1a_b_post_rodata.o and the new
  text1a_b_tail_rodata.o (split at 0x80016240). candidate.c = landed body.
- NOTE for future system.c switches: system.o(.rodata) now sits at 0x8001622C; any other
  function in system.c that emits rodata must fit that slot ordering.
