# Hypothesis ledger — func_8003EDC0

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: rotated after recon only; INCLUDE_ASM (`src/code6cac_c2.c:2103`); banked body is a placeholder, no attempt. Callers `text1a_c2.c:56,162,200,236` as `(u16 stream ptr, 7)`. Listed in `sdata_funcs.txt:91` (D_800A3368, D_800A3230 gp-relative).
- DECODE [asm read; C shape I]: four phases over an s16 stream with -1 terminators.
  - P1: (pos, tok) pairs fill 16-byte records D_800A4750[n]: +0=0xC, +2=arg1, +4=tok&0x7FFF, +6=0, +7=(tok&0x8000)?8:0, +8=(pos%32)*2000-32000, +0xC=(pos/32)*2000-32000; then `D_800A3368 = (s16)n`.
  - P2: 0x68-byte records D_800A6690[n]: zero/4 inits, three u32 fields built `lo` then `|= hi<<16`, h10/h12/h14, a b0 flag; calls `D_800F66A0[rec->h8](rec+0x10, rec+0x18)` (same pattern as text1a_c.c:1549); b58=0.
  - P3: 32x32 D_800A7FE0 grid = -1 (write a natural count-up loop; GCC loop reversal should give the count-down).
  - P4: `grid[pos/32][pos%32] = n`, copy tokens into D_800A87E0[n++] until bit 0x8000; `D_800A3230 = max(D_800A3230, n)`; if >= 1000 call func_80052C10(); zero D_800A3678/7A/7C.
  - Each P1/P2 header loads twice (lhu + lh) -> suggests `while (*p != -1) { pos = *p++; ... } p++;` with `s16 *p`.
- CONSTRAINTS: aggregate-merge-family.md / aggregate-declaration-views.md (one declaration per TU). Matched siblings func_8003E6D8 / func_8003EB84 (code6cac_c2.c:1841-2101) use `extern u8 D_800A4750[]` / `D_800A6690[]` byte views (code6cac_c2.c:55-59). D_800A3678 is `s32` in text1a_c.c:1270 but s16 367A/367C in code6cac.h:320-321 - probably an SVECTOR [I].
- BLOCKER: none proven; size + typing decision.
- PLAN:
  1. `canonical`, then `sandbox --diff` on a straight transcription keeping the existing u8[] views with cast field stores (baseline).
  2. Struct records (16-byte, 0x68-byte); respell the two siblings onto them in the same session (they must re-measure 0).
  3. Iterate on the P1/P2 double-load headers and the s16 counter truncations.
- DEPENDS: same-TU siblings. Q65 per-file gp model (banked, not applied) would tie the gp access to D_800A3368/D_800A3230 to this file defining them - re-measure if Q65 lands first.
- ODDS/LANE: medium-large (234 insns), ~55% [I]. Grinder-suitable: recon then structural.
