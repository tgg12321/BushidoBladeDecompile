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

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: rotated; INCLUDE_ASM (`src/code6cac_b_tu2.c:843`). `candidate.c` (= `rejected/tbl-copy-six-stmt-lzc-90.c`) matched the oracle through the real pipeline; sandbox said 90 only because the old scorer stripped header move/nop statements. Layer-2 FAIL 2026-09-25 had two causes, BOTH since fixed by rulings: `engine/gtemacro.py` now scores header-exact units as written (LZC family pinned); the 2026-09-26 class grant (`.claude/rules/inline-asm-policy.md:93-106`) covers verbatim inline_o.h islands; Q28 (`docs/grind/owner-rulings-2026-09-26.md:236-243`) admits the `tbl = dist` copy under Ruling 11 (C)(3) and names this function. -> Ready to un-rotate.
- CONSTRAINTS: islands character-exact against PINNED (`swc2  $31,($12)`, not the candidate's `0($12)`; class-grant clause (C) forbids respelling). Per-function row condition `docs/grind/decisions.md:1014-1036`. `&D_80101EC8 + off` byte puns must go (aggregate-merge-family (c)). Local `HitScratch` vs the TU's `ScrPad` = two views of one storage (Q21, aggregate-declaration-views).
- BLOCKER: proven C; what remains is an old-convention body plus an unbuilt Ruling 11 package.
- PLAN:
  1. Copy func_8002A458's two six-statement gte_Lzc islands verbatim (`src/code6cac_b_tu2.c:~1430-1466`). Rename `tbl` generically (A458's `temp2`), comment as Ruling 11 (C)(3) Q28; put `dist` (squared, root, clamped to 1) under Ruling 11 like A458's `temp`.
  2. Replace `HitScratch` with `ScrPad`: `u8 unk78[0xA8-0x78]` (`src/code6cac_b_tu2.c:966`) -> `LeafPos unk78[2][2]`; use `SPAD->unk78[c][k]`, `SPAD->unkA8[c][n]`; drop the moved Vec3i typedef.
  3. Record reads as `g_practice_menu_table[k]` fields: `unk_6A` (u16), `unk_0E`, `unk_20`, `unk_14E`, 0x1CA = `unk_1C8.vy`, `&...unk_F4`, 0x134/0x13C = `unk_134.vx/.vz`; `(u8*)&D_8008D118` -> `g_sqrt_table_u8[...]`. Watch cse related-value addressing (floor went 54 -> 37 when this changed).
  4. `sandbox --disable all --candidate`; `rad` FAKE paperwork (7-word ablation banked); Ruling 11 (D) proof (priority/tie analysis in evidence.md "Why header-exact").
  5. `queue unpark` it; after landing retire alias rows `undefined_syms_auto.txt:589,596,656,660,664,675`.
- DEPENDS: clash/facing tail is shared in meaning with func_8002AB08 - settle field spellings here first. ScrPad change also touches func_8002C61C, func_80029454 (check byte-neutral). Uses func_8002C22C's +0x210/+0x234 members.
- ODDS/LANE: 1-2 sessions, ~65% [I]. Manual.
