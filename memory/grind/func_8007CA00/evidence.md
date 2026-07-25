# Evidence bank — func_8007CA00

## Baseline (s1 recon, 2026-07-24)
- canonical: verdict **C**, 0 asm insns, 44 total, distance 13 (pure-C target).
- sandbox --disable all: **floor 13** at session start (13 regfix rules, all score-inert).
- Function: `s32 func_8007CA00(s16 *arg0)` in src/display.c:687. Reads globals
  `D_8009BE74` (switch selector, u8) and `D_8009BE77` (u8 flag).

## Semantics (from asm/funcs/func_8007CA00.s)
- switch(D_8009BE74): case1, case2, default.
- case1 & BE77!=0: return `0x400 - arg0[2] - arg0[0]`.
- case1 & BE77==0 / default: return `arg0[0]` (lh, signed).
- case2 & BE77!=0: return `0x400 - (arg0[2]/2) - arg0[0]`  (signed /2 via lhu+sll16+sra16+srl31+addu+sra1).
- case2 & BE77==0: return `((s16)arg0[0]) / 2`  (same signed /2 idiom, loaded lhu).
- **Shared tail CA40**: `subu v0,v0,v1; j CAA8; subu v0,v0,a0` = `0x400 - <val> - arg0[0]`,
  reached by case1 (fallthrough) AND case2/BE77!=0 (`j CA40`). Both keep arg0[0] in $a0.

## What the 13 regfix rules were faking (regfix.txt 2584-2591, 2708-2711)
- `lh $3,0($4)`->`lh $3,4($4)` and `lh $4,4($4)`->`lh $4,0($4)`: swap the two case1 loads
  (build put arg0[0]->v1, arg0[2]->a0; target the reverse).
- `insert j .LCA00_TAIL` + labels + `delete @28-30`: reconstruct the full 2-subu shared tail
  (build only merged the last subu, duplicating the first).
- `$2<->$3 @30-36` + `sra $2,$2,1`->`sra $2,$3,1 @38`: v0<->v1 swap in the case2/BE77==0 division.

## Measured probes (s1)
- **Variant A (explicit `goto sub` single shared tail): floor 17 — WORSE.** KILLED.
  Forcing label-sharing degrades GCC layout+RA vs natural duplication. -> rejected/goto-shared-tail-worse.c
- **Variant B (pre-subtract into temp so arg0[0] loads LAST -> stays in $a0): floor 11 — IMPROVED (-2).** CONFIRMED.
  Case1 loads now match target exactly (`lh v1,4(a0); lh a0,0(a0)`). -> candidate.c

## Remaining 11 diffs (objdump of variant B build, tmp/grind/func_8007CA00/s1/variantB_display.o)
1. **Case2/BE77==0 signed-/2 division: v0<->v1 swap (~6 diffs).** Build works the idiom in v1
   (lhu v1; sll v1; sra v0,v1; srl v1,v1; addu v0,v0,v1; sra v0,v0), target in v0 (return reg):
   lhu v0; sll v0; sra v1,v0; srl v0,v0; addu v1,v1,v0; sra v0,v1. Pure register-allocation.
2. **Tail-merge incompleteness / block placement (~5 diffs, +1 insn: build 45 vs target 44).**
   Target places shared tail CA40 right after case1 (case1 falls through), case2/BE77!=0 does
   `j CA40` (delay `li v0,0x400`). Build places the shared tail AFTER case2; case1 does `j 1394`
   with the first `subu v0,v0,v1` in the delay slot, and the first subu is DUPLICATED (1358 + 1390).

- [s1] canonical: verdict C, 0 asm insns, 44 total, distance 13 (pure-C target).

- [s1] sandbox --disable all floor = 13 at start (13 score-inert regfix rules); dropped to 11 with variant B.

- [s1] Semantics: switch(D_8009BE74); case1/BE77!=0 => 0x400 - arg0[2] - arg0[0]; case2/BE77!=0 => 0x400 - (arg0[2]/2 signed) - arg0[0]; case2/BE77==0 => ((s16)arg0[0])/2 signed; else arg0[0].

- [s1] Target shares tail CA40 (subu v0,v0,v1; j CAA8; subu v0,v0,a0) between case1 (fallthrough) and case2/BE77!=0 (j CA40); both keep arg0[0] in $a0.

- [s1] regfix rules decode to: (a) case1 load-offset/register swap, (b) full 2-subu tail-merge reconstruction, (c) v0<->v1 swap in case2/BE77==0 division.

- [s1] Variant B remaining 11 diffs = ~6 (case2/BE77==0 division v0<->v1 swap, pure RA) + ~5 (tail-merge incompleteness/block placement, +1 duplicated subu, build 45 vs target 44 insns).
