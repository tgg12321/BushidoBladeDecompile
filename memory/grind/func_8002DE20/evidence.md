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

Final candidate.c: 0 (506/506). Full-build SHA1 == oracle 62efab4f with it spliced (2026-09-26, lock-held landing build, reverted after).

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

## Role analysis of cross_a / cross_b (renamed from side_a/side_b, 2026-09-26, orchestrator request)
Every write stores the same quantity: the 2D cross product (edge vector) x (point - edge start)
for the edge under test, i.e. which side of that edge the point lies on. cross_a is always the
first point of the pair, cross_b the second. Every read is the same consumer shape,
`(cross_a ^ cross_b) >= 0` ("both points on the same side"), in the block of the write. No write
is a constant, a copy, a staged value or a split computation; no value is carried across tests.
- Ruling 5 (full text): passes 1(c)/1(d)/1(e)/2(a)-(d)/3; FAILS 1(a) as written (the consumer is
  a sign test, not a struct member or call-argument slot) and 1(b) (writes are different cross
  products, not one template + subscript selector). 1(f): names now state the role.
- Ruling 5 extension: fails (A) (writes not textually identical).
- Ruling 6: not a record pointer; fails (C)/(D).
- Ruling 8: vmNoiseOn only. Ruling 9: fails (a) (no struct-member consumer) and (b) (no base +
  constant offsets). Ruling 10: no public original source.
- Precedent that is a ruling, not a commit: docs/grind/decisions.md:25750, Judge final call PASS
  2026-09-08 on func_8002E6B0 (same file, same test idiom): "two function-scope cross-product
  locals re-assigned per edge (same quantity each time, not a variable-reuse borrow)". That Judge
  classified the pair as primary variables holding one quantity, not carriers. It predates Ruling 5
  (2026-09-23), which is written for "a fresh local written more than once"; whether a primary
  one-quantity variable falls under Ruling 5 at all is the question layer-2 has to decide (compare
  func_80043454 `count`, layer-2 PASS 2026-09-26, commit 3b2e8b8f0, "primary variable").
- Honest distinction from the banned carriers: y1 fed dx then dy (two consumers); `c` held
  different templates incl. a constant; src/idx re-loaded an unchanged value; tmp in func_8002D780
  held `z2 - z0` then a LUT value. Here each write is a new cross product consumed by the same test.

## Registry row (cluster grant) — NOT authorized by any landed ruling
func_8002DE20 is named in the 2026-08-17 cluster census, but every owner_cluster_grants.txt row
since 2026-09-21 was owner-instructed per function (9bdfcc6cc, 789ce34d7, 1de410a11, eeda6664b,
cd61ed9f6). Ruling 3 (decisions.md:26863) names func_80018094 alone; layer-2 FAILed func_80018300's
landing for a row self-added "under Ruling 3 terms" (eeda6664b body). The islands here are the
inline_o.h class (move $12 preamble + "$12"-"$15","memory" clobbers), which the 2026-09-25 scorer
ruling routes to "an owner-instructed row" (inline-asm-policy.md). So the row needs the owner.

## Layer-2 FAIL (2026-09-26, manual lane) — body banked as rejected/layer2-fail-cross-pair-inline-o-islands-0.c
(1) cross_a/cross_b = banned multi-write carrier under Ruling 1: no Ruling 5/6/8/9/10 admits it.
    decisions.md:25750 (func_8002E6B0 Judge call) is a one-body call that predates Ruling 5 and
    does not transfer; the owner declined "Allow as a class" 2026-09-24; the mechanism is allocator
    effect (Ruling 5 "Known weakness": never sufficient); the borderline question is unanswered and
    cannot be spent (ruling-record-lands-before-code).
(2) The islands are the inline_o.h preamble form: excluded from the 2026-09-23 route; per
    inline-asm-policy.md:360-364 they need an owner-instructed owner_cluster_grants.txt row; header
    provenance unpinned (one copy only: tmp/croc-ref, $PSLibId unexpanded).
Cleared by the reviewer: the struct view (backed by target bytes), mid_i, the final return form,
the z nudge. candidate.c stays the 0/506 body as the reference byte-proof; it is NOT landable.

## slotE 2026-09-26 — islands un-joined + Ruling 11 package (candidate.c REPLACED)
candidate.c is now the slotE body (the layer-2-failed body stays banked as
rejected/layer2-fail-cross-pair-inline-o-islands-0.c). Changes: (1) the nine GTE islands are the
pinned PsyQ 4.3 inline_o.h header statements written SEPARATELY, character for character except
two tool-forced deviations — `0($12)` for `($12)` (maspsx parse) and gte_rtv0's `.word
0x4A486012` for the DMPSX placeholder `.word 0x0000013f` (no DMPSX pass): full listing and proofs
in islands.md; `vin` dropped (macro arg `&obj->unkF8`). (2) cross_b's test-11 value split into its
own single-write local `cross_ab2` (its ablation is byte-identical; target's `xor v1,a0,v1` there
is the tie to a local operand 2). (3) (F) annotation on cross_a/cross_b.
Scores: `--disable all --keep-cheat-asm` 0/506, 0 hunks. Default strip 10 (497 insns): engine
PINNED lacks these macros + recognizer gaps (islands.md "What else landing needs").
NOT oracle-built in this form (bank-only task; the predecessor was oracle-proven and the sandbox
objects are identical, 0 hunks).
Ruling 11 (D) proof for cross_a/cross_b: ruling11.md (dumps of the exact candidate.c and its
derived per-value twin r11/final_pv.c, mechanism local-alloc.c :470-478 / :1824-1827 /
:1905-1922, necessity argument, 23 single-value ablations all > 0, structural respellings 90,
permuter from the per-value body). Open point for layer-2: (B)(2) path-wise re-store of a4/b7.

## 2026-09-26 landing staged (lock held by slotE; NOT committed, awaiting layer-2 of the whole set)
Rules: ae96881fc (inline-asm-policy.md § Per-function grant: func_8002DE20). Staged:
- maspsx fix: tools/maspsx/maspsx/__init__.py + tests/test_empty_offset.py. Byte-neutral over all
  36 objects: pristine HEAD maspsx copy vs fixed, same per-file pipeline, HEAD TU for code6cac_b
  (tmp/func_8002DE20/neutral2.log). maspsx unittests: 138 run, the same 2 pre-existing failures.
- engine: gtemacro.py + test_engine.py. engine test 801/0 (771 on HEAD).
- auth: inline_asm_canonical.txt row + owner_cluster_grants.txt row (DMPSX word only).
- Match: src/code6cac_b.c splice + canonical_asm_regions.json (30 hashes).
Results: lock.ps1 rebuild -> build_sha1 62efab4f... == oracle; sandbox --disable all (default
strip) 0/506 (0 source/operand hunks); completion.source_issues == []; canonical ASM-PARTIAL.
Precheck: tmp/func_8002DE20/precheck.txt. Its one flag, "hardcoded-$N asm", is the header's
own `$12`/`$0`.. cop2 register text. Messages: tmp/func_8002DE20/landing/msg_{maspsx,engine,auth,match}.txt.
Incident (fixed): a Windows-python rewrite truncated inline_asm_canonical.txt and wrote
owner_cluster_grants.txt as cp1252. Both were restored from HEAD and the rows re-appended in
UTF-8 via WSL. The staged diffs are +1 line and +10 lines.

## 2026-09-26 layer-2 FAIL of the Q11 set (3 gaps) — lock released, tree reverted
Accepted: the maspsx fix + its neutrality method, the engine change in substance, both rows,
the islands, and the commit split. Owner, on (B)(2): a write is banned only if it is redundant on
ALL paths (rule text pending; do not cite until the lead confirms the commit). Gaps closed:
(1) tree-wide recognizer before/after record -> recognizer_treewide.md; (2) jump2 cross-jump /
duplicated-into-arms family -> ruling11.md addendum 2 (join-point dup 171/148/106/162/204;
the 7 single-predecessor tests have no arms; T2 hoist 158); (3) Pure-C attempts block added to
landing/msg_match.txt. The staged set is saved in tmp/func_8002DE20/staged/ (+ staged.patch).
Revert: 8 files restored and the new test file deleted; lock.ps1 rebuild -> oracle; lock released.

## LANDED 2026-09-26 — COMPLETED-INLINE-ASM-CANONICAL (manual lane, slotE)
Layer-2 PASS on the whole Q11 set. Commits: maspsx 56d7fba01, engine 3ee2634d9, auth 06659977d,
Match 4eb7f2312, queue ce66fba5c. `queue done` ok (SHA1 62efab4f...), check_completion_integrity OK.
Rulings spent: inline-asm-policy.md § Owner ruling 2026-09-26 (inline_o.h class) + § Per-function
grant: func_8002DE20 (ae96881fc); Ruling 11 for cross_a/cross_b with the (B)(2) clarification
9a0543e05. This ledger is closed; the proof files (ruling11.md, islands.md,
recognizer_treewide.md, r11/, tools/) stay as the record.
