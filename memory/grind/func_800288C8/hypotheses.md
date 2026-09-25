# func_800288C8 — hypotheses (manual session 1, 2026-09-25)

Scores: "sb N" = sandbox --disable all (combined islands); "real N" =
differing words on the real pipeline vs build/src/code6cac_b.o
(tmp/f288/fo.sh; relocation-addend-only pairs ignored).

## Killed / measured
| spelling | result |
|---|---|
| `s32 *pt = (s32 *)(0x1F800078 + i*0x18)` pointer per row | sb 81; offset-0 store gets its own giv |
| scratchpad struct at 0x1F800078 (pt first) | sb 111-134; `.x` (offset 0) address forced to a register |
| struct copy for pt[c][0] | lw/lw/lw/sw/sw/sw instead of interleaved; KILLED |
| islands with "$12" clobber only | sb 103; $t5-$t7 free, rad_j lands in $t5 (target $s0) |
| mode/F4 via D_8010237E symbol + &D_80101EC8 args | sb 54: no related-value pointer ($s0 = &mode) |
| separate dist_sq (no in-place) | sb 34 / real 20 (reach add scheduled before dz) |
| fresh LZC copy (`lzc_in = dist`) | sb 17 / real 2 (copy in $v1) |
| LZC copy in `lz` / `shift` | sb 20 / 22 |
| reach before dx / between dy,dz / j-term first | sb 45 / 29 / 17 |
| rad[i] hoisted to outer body (`rad_i`) | sb 62 |
| `reach = D[i]; reach += D[j];` | sb 38 |
| init orders (hits last, chained, hits before loop 1) | sb 15-19; hits-late moves `move s3,zero` |
| force/px/pz named temps, split sqrt stores, `>= 0x81` | no change in any priority |
| nested-if range/r2 checks, per-axis continue | identical RTL priorities |
| do-while(0): 35 single placements, 91 pairs | none reaches i12/radp/hits/i = s1..s4 |
| header-exact islands, `tbl` fresh copy | real 2 ($v1) |
| header-exact, no staging copy | real: missing move + hits/i swap |

## Open
- The sandbox cannot score header-exact islands (engine/inlineasm.py
  strips cop2-free statements, which here are verbatim macro text). That is
  the owner-level question; the body itself is proven on the real pipeline.

## Layer-2 FAIL (2026-09-25) — rejected/tbl-copy-six-stmt-lzc-90.c
Body = header-exact six-statement gte_Lzc, oracle-green (SHA1 match),
sandbox 90 (scorer strips the header's cop2-free move/nop statements).
Ruled SOUND: island text + hashes, in-place `dist` (staged-value bounds
met), `rad`, hits-before-i declaration order, `lz` split, Vec3i move.
FAILED:
1. Islands: no landed ruling admits a nonzero sandbox (cluster check 1 and
   the 2026-09-23 route condition 4 both require 0), and the inline_o.h class
   needs its own owner-instructed row in tools/grinder/owner_cluster_grants.txt
   (func_80018300 / func_8002CD58 / func_8002DAD0 each have one); the
   inline_asm_canonical.txt line was a self-grant. Owner question logged:
   docs/grind/borderline.md 2026-09-25 func_800288C8 policy-question.
2. `tbl = dist`: copies a still-live variable, computes nothing, and runs on
   the table path unread — func_8002A458's failed `lzc_in` renamed.
Frontier:
- LEAD: func_80018300 (src/code6cac.c, landed) reached sandbox 0 with the
  same inline_o.h form (joined move+mtc2 / bare-nop / move+swc2 islands).
  Study how its stripped and unstripped builds agree; it may show a way to
  get the 4 extra loop RTL insns (or an equivalent priority shift) that the
  sandbox also sees.
- The a0 seat of the LZC copy still needs an honest source (fresh copy -> $v1).
- NOTE: HitScratch (struct at 0x1F800000: other[10], pt[2][2], joint[2][22])
  contradicts the committed ProbeScr view in the same file
  (src/code6cac_b.c:1171-1172, `Vec3i j[22]` blocks from 0x1F800078,
  stride 0x108, used by func_8002C61C as SCR[i].j[5..9]). By this function's
  evidence the per-char joint blocks start at 0x1F8000A8 and 0x78..0xA8 holds
  the four body points, so ProbeScr's j[k] = joint[k-4]. Reconcile the two
  views (one declaration, C61C re-indexed) before landing, or disclose.
