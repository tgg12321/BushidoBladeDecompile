# func_8002DE20 — evidence (manual lane slotA2, 2026-09-26)

Queue: distance 506 (cold, no prior ledger), verdict ASM-PARTIAL (18/506 cop2 insns in 6 regions).
Cluster member: `.claude/rules/cop2-addressing-preamble-cluster.md` (census row, SetRotMatrix /
long-vector sub-family). No `tools/grinder/owner_cluster_grants.txt` row as of 2026-09-26.

## Floor history (this session, `sandbox --disable all`, candidate substituted)
| step | score | what changed |
|---|---|---|
| first u8*-offset transcription | 159 | — |
| index macro `obj + i*12 + 0x118` | 119 | loop giv base obj+12 / disp 0x120 now right |
| struct-member array (obj->pts[i][k]) | 99 | `addu t2,t0,v0` operand order (obj first) — only a member-array access gives it; `(s32(*)[3])(obj+0x118)`, `Vec3i*` casts, `obj + 0x118 + i*12` all stay idx-first (102) |
| final group `if/if return 1; return 0;` | 20 | GCC store-flags the last test (`nor/srl`), no shared ret-0 block (the `return A && B` / `if (<0) return 0; return X>=0` forms keep the after-loop `return 0` wrong or the result in $a0) |
| `mid_i = 3 - min_i - max_i` | 8 | fold emits (3 - max) - min; max_i then outranks min_i in global.c (7 refs, 62 vs 63 insns) -> $t3/$t5 as target |
| island clobbers "$12","$13","$14","$15","memory" | **0** | reload spill regs: target reloads four LO/HI results into $s0; $t6/$t7 are in bad_spill_regs only when an asm names them (reload1.c regs_explicitly_used) |

Final candidate.c: 0 (506/506), also 0 with dz split into dz_a/dz_b, VECTOR/SVECTOR/Vec3i members.

## What the target proves
- Object model: the three rotated points are an array member of the object (obj + i*12 + 0x120 with
  `addu tX,t0,v0`, obj first). u8*-offset forms cannot produce that operand order (measured 102).
- The islands' clobber lists include $14/$15 (see floor table). That is inline_o.h's own clobber
  list ("$12","$13","$14","$15","memory" on every statement), i.e. the islands are PsyQ DMPSX
  inline_o.h gte_ldv0 / gte_rtv0 / gte_stlvnl (croc-ref copy tmp/croc-ref/include/psyq/inline_o.h
  sha256 27a4abd6...81a9d6: gte_ldv0 :16, gte_rtv0 :1353, gte_stlvnl :2422; $PSLibId unexpanded).
  Provenance still has to be pinned (4.3 copy + second copy) before an auth row.
- The same-side tests use ONE function-scope pair of cross-product variables for all 12 tests
  (the same idiom as func_8002E6B0 in this file, `cross_center`/`cross_point`, on main since
  2026-08-19). Mechanism: a pseudo referenced in several blocks is global, so local-alloc cannot
  tie the xor temp to it (local-alloc.c combine_regs needs a local qty) — target `xor v0,a0,v1`
  untied everywhere; and in the last group the global pair conflicts with nothing that forces $v0,
  so the return value keeps $v0.

## Receipts for the reuse (all on the final chassis, 506/506 insns)
| spelling | score | file |
|---|---|---|
| one fresh block-scoped pair per test (one write each) | 90 | rejected/one-local-per-test-score90.c |
| no variables, cross products inline in the condition | 90 | rejected/inline-cross-expressions-score90.c |
| fresh pairs, only the final group reuses a function-scope pair | 72 | rejected/reuse-final-group-only-score72.c |
| one pair per group (block scope), reused inside the group | 54 | rejected/reuse-per-group-scope-score54.c |
| one function-scope pair for all 12 tests | 0 | candidate.c |
Before the clobber fix: fresh-per-test 107 vs reuse 20 (same shape).

## Policy status
side_a/side_b fail ordinary-c-judge-decidable Ruling 5 as written (consumer is a sign test, not a
struct member/arg slot; writes are different expressions), and Rulings 6/9/10 do not apply.
Filed as a policy-question in docs/grind/borderline.md (2026-09-26). Everything else is ordinary C:
struct view with unk pads (bytes-decided layout), loop min/max scan, divide-by-zero guard counter
(D_800A314C, gp-relative, sdata_syms.txt), dz_a/dz_b each one role with a zero clamp.
