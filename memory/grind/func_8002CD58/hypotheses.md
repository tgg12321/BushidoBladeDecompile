# func_8002CD58 hypotheses / measurements

All scores are `sandbox func_8002CD58 --disable all --candidate <file>` on the
main chassis as of 2026-09-25 (HEAD f7306ea2b), target 352 insns. Scratch
variants lived in tmp/cd58/; the instructive ones are banked in rejected/.

## Manual session 2026-09-25 (first attempt on this function)

| step | spelling | score | insns | finding |
|---|---|---|---|---|
| s1 | first full body, one function-scope `mat`, one `sum`, one `dist` | 102 | 352/352 | obj/mat swapped between $s0/$s1 |
| a | `mat` block-local in the success tail | 18 | 352 | target seats mat in $s0 via local-alloc (single-block pseudo); obj then takes $s1 |
| b | no `mat` variable, `(s32 *)(obj + 0xD8)` at each use | 18 | 352 | same effect as a |
| d | `dist` split per site (sum shared) | 21 | 352 | worse: site-1 dist loses the ratan2 a1 preference |
| c | `sum` split into three locals (dist shared) | 6 | 352 | sites 1-2 now $a0 as target; only fallback site 3 sits in $a1 |
| e | sum and dist both split | 9 | 352 | site-3 residual unchanged, site-1 dist regresses |
| f1 | fallback shift order C8, D0, CC | 12 | 354 | schedule changes |
| f2 | FA store before the fallback sum | 12 | 354 | |
| f3 | `sum3 = C8*C8; sum3 += D0*D0;` | 6 | 352 | combine re-merges; same residual |
| f4 | `D0*D0 + C8*C8` | 13 | 352 | |
| f5 | sum shared by sites 2 and 3 | 12 | 352 | rejected/shared-sum-sites2-3-12.c |
| f6 | sum shared by sites 1 and 3 | 12 | 352 | |
| g1 | fallback through `((VECTOR *)(obj + 0xC8))->vx` members | 6 | 352 | identical codegen to c |
| g2 | fallback through block-local x/z locals | 25 | 349 | C8 no longer reloaded; target reloads it, so no locals in the original |
| g3 | `C8 = C8 >> 6` (plain assignment) | 6 | 352 | identical to c |
| g4 | FA store first, then split `+=` sum | 17 | 354 | |
| g5 | FA first, `D0*D0` then `+= C8*C8` | 20 | 354 | |
| h1 | c with the three gte_Lzc islands in inline_o.h form (GCC computes &slot) | 6 | 352 | island spelling does not touch the site-3 register |
| h2 | h1 + inline_o.h full clobber list on every island | 6 | 352 | same |
| i1 | c + `sum3` reused for the table byte at site 3 | **0** | 352 | closes |
| i2 | c + reuse at all three sites | 0 | 352 | more FAKEs than needed |
| j1/j2 | h2 + reuse (site 3 / all sites) | 0 | 352 | inline_o.h full-clobber form also closes (rejected/inline-o-h-full-clobbers-0.c; not used — the in-file authorized island spellings are preferred) |
| k1 | final: no mat/vec pointer locals, sums named len_sq/xz_sq/nxz_sq, reuse at site 3 only | **0** | 352 | candidate.c until the third layer-2 review (hardcoded-preamble Lzc islands) |
| k0 | k1 with a fresh `s32 tbl` at site 3 (the FAKE ablated) | 6 | 352 | rejected/fresh-tbl-site3-6.c |
| k2 | k1 with the three gte_Lzc islands as pure macro text (ldlzc+2 nops / stlzc(&slot), no hardcoded addiu/$v0) | **0** | 352 | = candidate.c after the third layer-2 review; 26 islands |

## Mechanism of the last residual (RTL-verified, tools/rtl_track dumps + BB2_FINDREG_DEBUG)

The fallback's squared xz length (`nxz_sq`, pseudo 78 in k1/k0 .greg) is a
global allocno. In k0 its only hard-reg preference is {5}: expand_preferences
copies it from the `D0*D0` product pseudo, whose set_preference came from the
product's operand (the `D0 >> 6` value, local-allocated to $a1). find_reg's
preference pass then seats it in $a1 while the target has $a0. Reusing
nxz_sq for the table byte adds the `(set (reg 224) (ashift (reg 78) 16))`
insn; 224 is local-allocated to $a0, so set_preference records $a0 on 78
(prefs {4,5}) and find_reg takes the lowest free preferred register, $a0.
Same sum-for-byte reuse as the landed func_8002F2D0 (src/code6cac_b.c).

## Levers ruled out before the reuse

Per-site split sums (c), shared sums (f5/f6), statement order in the fallback
(f1-f4, g4-g5), member vs raw-offset access (g1), plain vs compound shift
(g3), explicit value locals (g2, changes instruction count), and both island
spellings (h1/h2). None moves the site-3 register without the reuse.
