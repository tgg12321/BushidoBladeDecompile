# SELF-VET — func_8003DE14

Session 38c (structural). Body = memory/grind/func_8003DE14/candidate.c, applied verbatim to
src/code6cac_c2.c. Measured THIS session: `sandbox func_8003DE14 --disable all` = 0 / 173 build
insns, rules_dropped 0; full-tree `verify-oracle` ok=true with build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle
(tmp/grind/func_8003DE14/s38/verify_oracle_s38c.log:3-4).

This body carries a Judge PASS ruling on record (docs/grind/decisions.md 2026-09-11 06:05,
body=2852b631a415e9ee): "the body in memory/grind/func_8003DE14/candidate.c MAY be submitted with
the s21 extender present." The only defect that ruling named was cosmetic and comment-only (the s21
annotation's stale "floors at 8" exhaustion pointer); it is refreshed in this submission to cite the
s38b b1/b2 measurement (0 vs 7 / 173). Comments are body-key-neutral, so the cleared body is
unchanged.

CONSTRUCTS: bound-detour chain extender `j < rect[2] * rect[3] + rect[2] - rect[2]`; s21 chain
extender `LoadImage((s32)rect, ((s32)dst_buf + j) - j)`; named intermediate `s32 gm = g_src & 0x3E0;`;
mixed exit forms (`goto loop_check` in the early arms, fall-through in the blend arm).

## T1 semantic purpose
- Bound detour: algebraically null; the loop trip count is identical with and without it. It has no
  semantic purpose — that is exactly why it is FAKE-annotated and claimed under the combine-foldable
  chain-extender clause, whose whole subject is a detour with zero emitted bytes. Measured
  byte-neutral: 173 build insns with and without (b1/b4 sweep, hypotheses.md s38 H38-1).
- s21 extender: same shape on the LoadImage argument; `((s32)dst_buf + j) - j` is the address of
  dst_buf. Zero emitted bytes (173 both ways). FAKE-annotated for the same reason.
- `gm`: a REAL value — the green channel's masked result — that is computed and consumed by the
  store on the next line either way. Naming it changes no semantics; it relocates where g_src dies.
- Mixed exits: ordinary control flow; `goto loop_check` is the loop's own continue.

## T2 human-programmer
- `gm` and the mixed exits: yes. Naming the masked green channel before an OR-merge of four channel
  fields, and jumping to the shared increment from the two early-out arms, is how a human writes
  this blend loop.
- The two detours: NO — a human would not write `+ rect[2] - rect[2]` or `((s32)dst_buf + j) - j`.
  That is precisely why each carries a /* FAKE: */ annotation naming the pass and the exhaustion
  ledger, and why each is claimed under a named frozen family rather than presented as ordinary C.
  The honest answer to this test is the annotation, not a denial.

## T3 GCC-internals justification
Both detours are justified by GCC internals, openly: flow.c computes `reg_n_refs` BEFORE combine
folds the detour away, so the fold leaves the ref count behind. For the bound detour the consumer is
local-alloc.c:1669-1684 `qty_compare_1` (the two block-local halfword loads of rect[2]/rect[3]:
refs 6 -> 12 flips the sort so the earlier-born load takes $v0, dumped in
tmp/grind/func_8003DE14/s38/qty_win.log:623-624). For the s21 extender the consumer is global.c's
`allocno_compare` (j: 11 refs/livelen 59 = 5593 -> 15/73 = 6164, crossing `complement`'s fixed 6111,
dumped in s38/qty_b1.log and qty_b2.log at ord=15/16). Under T3 that is the cheat SIGNAL for an
unsanctioned construct — which is why these are not offered as ordinary C. The combine-foldable
chain-extender clause is the one frozen family whose OWN TEXT defines the construct by this
mechanism ("its only surviving effect is the extra `reg_n_refs` count flow.c records before the
fold") and whose extra prerequisite is a byte-neutrality verification, which is satisfied and
measured. `gm` needs no GCC-internals defense to be legitimate C; its annotation states the
allocno-priority mechanism because the named-intermediate entry requires the mechanism be named.

## T4 permuter/search provenance
No permuter output is in this body. Every construct came from a dumped, named-predicate derivation
(s32/s33/s36/s37 closed forms) and each was confirmed by an ablation sweep, not by a detector gap:
b1 = 0, b2 (s21 removed) = 7, b3 (`gm` removed) = 17, b4 (both removed) = 22, all at 173 insns. The
s38b sweep of 56 ordinary-C and sanctioned-family alternatives bottoms out at 3/173.

## T5 family check
- Both detours: **combine-foldable chain-extender**, .claude/rules/dead-store-fake-exception.md:51-70
  (scope extension, owner ruling 2026-07-01), including its extra prerequisite — verified the fold
  emits zero bytes and materializes no address (173 == 173, no new insn). They are not
  DImode-chain / OR-tree-shape-shift: nothing is re-associated to change an RTL tree shape, and no
  new value or width is introduced.
- `gm`: **named-intermediate declaration order**, no-new-park-categories.md:204-215 with the
  2026-08-17 clarification. Six prongs: once-written, once-read, value real and consumed by the
  store, byte-neutral (173 == 173), fresh (not borrowed), destination not live-pre-initialized.
- Mixed exits: ordinary C (`.claude/rules/cross-jump-store-tail-merge.md`), no FAKE needed.
- Judge-constraint compliance: the banned multi-write fresh carrier `h` is ABSENT — this body has no
  carrier of any kind, and the `total` borrow the 2026-09-11 04:48 ruling offered is not used either.

## T6 naming-announces-intent
No `pad`/`dummy`/`unused`/`spill`/`slack` names. Names in the diff: rect, count, src_buf, dst_buf,
color_info, saved_y, target_color, blend_base, total, src, dst, factor, complement, pixel, px,
r_src, g_src, sum, rp, gp, gm, new_y — every one describes the value it holds. `gm` = green mask;
it is read by the store on the following line, so it is neither unused, discarded, nor address-taken.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: combine-foldable chain-extender (dead-store-fake-exception scope extension)
  SCOPE: "**combine-foldable chain-extender** (scope extension, owner ruling 2026-07-01 same-day): a LIVE store/computation routed through an algebraically-equivalent detour that combine folds back to the direct form with ZERO emitted bytes — its only surviving effect is the extra `reg_n_refs` count flow.c records before the fold."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:51
  FAMILY: named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID."
  PRECEDENT: .claude/rules/no-new-park-categories.md:204
  FAMILY: mixed exit forms (goto + fall-through), ordinary C
  SCOPE: "mix `goto endK` with inline `return` to defeat `find_cross_jump`. SOTN ships this verbatim in `SsVabOpenHeadWithMode` (`src/main/psxsdk/libsnd/vs_vh.c`)."
  PRECEDENT: .claude/rules/no-new-park-categories.md:198

ANNOTATION-CONFORMANCE:
  /* FAKE: `gm` names the green channel's masked result so that g_src dies at the mask instead of at
     the store; mechanism: global.c allocno priority (prio = nrefs*40000/live_length, dumped via
     BB2_ALLOC_DEBUG) ... lever-exhaustion: memory/grind/func_8003DE14/hypotheses.md s29-s35 ... +
     s36 waves x ... */                                     (src/code6cac_c2.c, blend block)
  /* FAKE: the inner loop's bound is routed through the algebraically equivalent detour
     `+ rect[2] - rect[2]`, which combine folds back to the direct `rect[2] * rect[3]` with ZERO
     emitted bytes ... Mechanism: local-alloc.c:1669-1684 `qty_compare_1` ... Lever-exhaustion:
     memory/grind/func_8003DE14/hypotheses.md s24-s37 ... */          (before the `while` bound)
  /* FAKE: j chain extender on the dst_buf argument (s21); mechanism: combine.c folds the +j/-j pair
     away but flow.c's reg_n_refs for j is counted before it, lifting j's allocno priority so the
     $t4/$t5 seat pair matches; lever-exhaustion: memory/grind/func_8003DE14/hypotheses.md s21-s38b
     - on THIS no-carrier chassis the extender-free body measures 7/173 against this body's 0/173
     ... and the s38b sweep of 56 ordinary-C and sanctioned-family alternatives ... bottoms out at
     3/173. */                                                        (before the LoadImage call)
  All three carry what + named GCC-pass mechanism + lever-exhaustion pointer.
