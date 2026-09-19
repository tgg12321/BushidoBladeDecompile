# func_8002DAD0 — hypothesis ledger

## [s1] H1 — literal transcription using func_8002E838's proven idioms closes most of the function
**Statement:** Writing the function as a literal field-offset transcription
(vecA/vecB diff blocks, GTE OP cross-product island, near-zero bailout,
LZCS/log2 block copied from func_8002E838, identity-matrix + RotMatrixY/X
+ two gte_ldv0/MVMVA/stlvnl tail islands copied from func_8002E838)
produces a near-complete match.
**Mechanism:** all six cop2 islands are verbatim PsyQ SDK macro bodies
already proven byte-exact in func_800203B4 and func_8002E838; the
surrounding C is ordinary straight-line arithmetic with no scheduling
tricks needed once the islands are right.
**Probe:** wrote candidate.c, applied to src/code6cac_b.c, ran
`sandbox func_8002DAD0 --disable all`.
**Result:** 204 → 44 on first full write (with vecA/vecB pointers cached
in local `s32 *` variables promoted to $s2/$s3 — extra 8 bytes of frame
and 2 extra saved-reg spill/reload pairs vs target, per the diff).
**Verdict:** CONFIRMED (the core structural derivation is right; the
extra floor was a secondary reg-alloc artifact, fixed by H2).

## [s1] H2 — target recomputes `obj+0xA8`/`obx+0xB8` addresses fresh at each cop2 island instead of caching a pointer local
**Statement:** The target does NOT keep `&obj[0xA8]` / `&obj[0xB8]` live
across the whole function in a callee-saved register; it recomputes
`addiu v0,s1,0xA8` / `addiu v0,s1,0xB8` freshly at both the first-use site
(the cross-product islands) and the last-use site (the tail rotate
islands), matching a source that passes `(s32 *)(obj + 0xA8)` as an
inline-asm operand expression at each call site rather than storing it in
a `s32 *vecA` local.
**Mechanism:** GCC 2.7.2's `global.c` only promotes a pseudo to a
callee-saved hard reg when it's live across a call/asm boundary AND the
source gives it a named binding; an un-named recomputed address
expression at each site never becomes a global pseudo, so no $s2/$s3 is
allocated and the frame drops from 48 to 40 bytes (matching target's
`addiu sp,sp,-40`).
**Probe:** removed the `s32 *vecA;`/`s32 *vecB;` locals, replaced all 4
uses with the raw `(obj + 0xA8)` / `(obj + 0xB8)` cast expressions
inline. Re-ran sandbox.
**Result:** 44 → 25. `build_insns` dropped from 202 to 201 (frame-size
hunks 1-2, 5, 18-23 from the pre-fix diff — the `sw s2/s3`,
`addiu s1,168`-hoist, and epilogue `lw s2/s3` hunks — are ALL gone from
the post-fix diff; the remaining 14 hunks are entirely inside the
dist_sq scratch block).
**Verdict:** CONFIRMED, kill_scope: class. Caching a cop2-island address
operand in a named pointer local that survives across an intervening
call/asm block is the WRONG spelling whenever the target reference shows
the same `addiu $vN,$sM,K` / `move $12,vN` preamble repeated verbatim at
every use site (a mechanical, re-checkable signature — grep the target
.s file for the identical `addiu` immediate appearing more than once).
predicate_cite: tools/gcc-2.7.2/global.c (pseudo-to-hardreg promotion
requires a live-across-call pseudo; a fresh untied address expression at
each site never creates one — this is `global.c`'s general
`local_alloc`/`global_alloc` behavior, not a per-function fact, hence
class scope). This generalizes: ANY future cop2-cluster function should
try the "recompute the address at each island, don't cache a pointer
local across calls" spelling FIRST when the frame is a few bytes bigger
than target's.

## [s1] H3 (OPEN, next session) — dist_sq scratch block needs a forced C8 reload between the shift-store and the square
**Statement:** Target reloads `obj+0xC8` from memory (`lw v1,200(s1)`)
immediately after storing the shifted value back (`sra v1,v1,6; sw
v1,200(s1); lw v1,200(s1)`) instead of reusing the value already in a
register — i.e. cse.c is NOT folding the store+reload into a single kept
register the way our candidate's identical-looking C does. The rest of
the block (mult ordering: square C8 first via `mult v1,v1`, THEN load+mflo
interleaved with D0's load+shift+square, THEN store CC and D0 back,
THEN compute `0x800-angle1` and the sum-of-squares, THEN branch) is a
pure scheduling/operand-order tie once the reload is forced — hunks
4/8/10/11 in the current (floor-25) diff are all "operand-only" copies of
this same C8-vs-D0-register tie (v1-family register chosen for C8 vs
a1/a3-family for D0/sumsq), meaning they'll very likely resolve for free
once the reload forces the right value into the right pseudo.
**Mechanism (untested, to attribute with dumps next session):** candidate
C's `*(s32*)(obj+0xC8) = *(s32*)(obj+0xC8) >> 6; dist_sq = *(s32*)(obj+0xC8)
* *(s32*)(obj+0xC8);` re-reads the SAME fixed-offset MEM twice from
source, but cse.c's `canon_rtx`/`equiv_class` machinery still proves the
second read equals the just-stored value (no intervening call/label/
volatile) and reuses the register — exactly the `store-const-reload-cse`
symptom in .claude/rules (codegen-technique-index: "store-then-reload
folded to `li` → re-read the GLOBAL instead of caching a local" — but our
candidate is ALREADY doing that and cse STILL folds it, so the technique
as stated doesn't close this instance; needs the `.cse`/`.combine` dump
read before guessing further).
**Probe:** none run yet this session (turn budget). NEXT SESSION: run
`pwsh tools/grinder/dump.ps1 func_8002DAD0`, read the `.cse1`/`.combine`
dump around the obj+0xC8 shift/store/mult statements, confirm which pass
folds the reload and what source shape defeats it (candidates to try,
in order: (a) an intervening call/asm between the store and the reload —
but there ISN'T one in target between `sw v1,200(s1)` and `lw v1,200(s1)`,
so this may not be a CSE question at all but a REGISTER-ALLOCATION
question: cse keeps v1 as the SSA value, but `local-alloc`/`global-alloc`
DECIDES to spill it to $s1+0xC8 and reload rather than extend its live
range — i.e. the reload might be an ordinary alloc-driven spill artifact,
not something the C source controls at all, in which case this residual
is a scheduling/allocation tie like the other operand-only hunks and may
resolve automatically once (b) is tried; (b) restructure the block so
`dist_sq`'s first term is computed from a genuinely fresh local (`s32 x =
*(s32*)(obj+0xC8) >> 6; *(s32*)(obj+0xC8) = x; dist_sq = x * x;` — i.e.
do NOT re-read the field at all, mirror func_8002E838's exact single-axis
shape more literally) and compare; (c) reorder the CC/D0 loads relative to
the mult/mflo pair to match target's exact interleave (lw a1,0xD0 BEFORE
mflo a2, not after) by moving the `s32 dz = ...` declaration earlier.
**Result:** N/A — not yet measured.
**Verdict:** OPEN (no kill recorded; this is the live frontier item for
session 2).

## [s2] H3 RESOLVED — the C8 reload is CSE's unconditional varying-address invalidation, triggered by an intervening write to a DIFFERENT field
**Statement:** Target's `lw v1,200(s1)` reload of `obj+0xC8` immediately
after storing the shifted value back is NOT a register-allocation spill
(the address is the object field, not a stack slot) and is NOT defeated
by an intervening READ (reads don't invalidate CSE's memory table). It IS
produced by cc1's `cse.c` `note_mem_written`/`invalidate_memory`: for a
MEM store through a varying (pointer-relative) address,
`invalidate_memory` removes EVERY table entry `p` for which
`cse_rtx_addr_varies_p(p->exp)` is true — **unconditionally, regardless of
whether the new write's own address matches** (cse.c:7564-7577 sets
`writes_ptr->var=1` for any `PLUS`-address SImode store;
`invalidate_memory` at cse.c:1701-1719 then nukes any `in_memory` table
entry whose expression itself has a varying address, via the bare
`cse_rtx_addr_varies_p (p->exp)` disjunct — not gated on `nonscalar` or
`all`). So: shift+store C8 (inserts a fresh C8=reg cache entry) -> shift
and STORE D0 (a second varying-address write -> nukes the C8 entry, since
ITS OWN address also varies, even though C8 and D0 are different fields)
-> read C8 for the square: cache miss, genuine reload. D0's OWN read for
its square hits ITS OWN just-inserted entry (nothing wrote after it),
hence D0 never reloads — matching target exactly (C8 reloads, D0
doesn't). The `sched.c`/alias-analysis-based INSTRUCTION SCHEDULER later
is free to relocate D0's actual `sw` far downstream (same-base,
different-constant-offset accesses provably don't conflict for real
aliasing purposes, unlike CSE's conservative model), which is why the
*final* asm shows D0's store scheduled after CC's — the causal write for
the C8 invalidation is real in RTL/CSE order even though it appears late
in the scheduled `.s` output.
**Mechanism:** `tools/gcc-2.7.2/cse.c:1701-1719` (`invalidate_memory`) +
`cse.c:7539-7579` (`note_mem_written`). Read directly this session (see
evidence.md for the exact excerpt).
**Probe:** rewrote the dist_sq block to shift+STORE C8, then shift+STORE
D0 (both real writes, no temp), THEN compute `dist_sq = C8*C8 + D0*D0` in
one expression, THEN shift+store CC. Re-ran sandbox --disable all --diff.
**Result:** 25 -> 7 (target_insns 204, build_insns 204; 2 source-level + 1
operand-only + 6 not-scored, from 5+5+4). The C8 reload appeared exactly
as hoped. Confirmed the theory is right in direction; residual shape
(D0/CC store ordering, still source-level) needed one more iteration.
**Verdict:** CONFIRMED, kill_scope: class (the underlying CSE mechanism is
`cse.c`'s general invalidation rule, not a per-function fact — ANY future
function with a "target reloads field A right after storing it, but
doesn't reload field B" symptom should try "add a second varying-address
write between A's store and A's read" as hypothesis #1).
predicate_cite: tools/gcc-2.7.2/cse.c:1716 (the bare
`cse_rtx_addr_varies_p (p->exp)` disjunct in `invalidate_memory`'s removal
condition, unconditional on `nonscalar`/`all`).

## [s2] H4 — D0 needs a genuine deferred-store temp (not a second immediate write) to match target's "square now, store later" shape; CC's write is the C8 invalidator instead
**Statement:** Splitting D0 into "read+shift into a temp `dz` (no store
yet), use `dz*dz` directly for the square (ordinary register reuse, no
memory involved), defer the actual `*(obj+0xD0)=dz` store to the very end
of the block" reproduces target's D0 handling (single load, immediate
square, deferred store) exactly, PROVIDED some other write still occurs
between C8's store and C8's reload-for-square to serve as the invalidator
per H3. Moving CC's shift+store to right after C8's (before the dz-block)
fills that role.
**Mechanism:** Same cse.c:1701-1719/7539-7579 rule as H3, applied with CC
(not D0) as the invalidating write; D0 itself never touches memory before
its own deferred store, so it can never suffer a false-reload.
**Probe:** `*(C8)=C8>>6; *(CC)=CC>>6; { s32 dz=*(D0)>>6; dist_sq =
C8*C8+dz*dz; *(D0)=dz; }`. Re-ran sandbox --disable all --diff.
**Result:** 25 -> 6 (target_insns 204, build_insns 204; **0 source-level**,
5 operand-only, 4 not-scored). Every remaining hunk is either masked
(branch-target addend noise, explicitly `not-scored`) or a plain `a0`<->`a1`
register-seat rename on dist_sq's final pseudo (propagated through
`sltiu`/`addu`/`bltz`/`move`/`srlv`). Tried moving CC's write to just
before the dz-read instead of just after C8's write (same
invalidation-ordering effect) — measured byte-identical, score unchanged
at 6 (see evidence.md); kept the simpler (CC-immediately-after-C8) form.
**Verdict:** CONFIRMED, kill_scope: instance (this exact block shape,
measured on this chassis, no FAKE constructs — the block is ordinary C
with one deferred-store temp).

## Frontier (<=3, carried to next session)
1. **dist_sq final-pseudo register seat (a0 vs a1) — mechanism still unattributed after s3.**
   Five more C-level orderings/stagings were tried in s3 (operand swap, store-then-compute
   reorder, compound-assignment split both directions, named-intermediate for the c8 term,
   declaration-order reshuffle) — all either regressed or measured byte-identical. Hand-tracing
   `tools/gcc-2.7.2/global.c`'s `set_preference`/`expand_preferences`/`prune_preferences`
   against the `.lreg`/`.greg` dumps ruled out the two insns that DO call `set_preference` with
   reg75 as an operand (insns 183 and 225 — both traced to a hard-reg-2/v0 preference that gets
   PRUNED by conflict, not a hard-reg-5/a1 preference), so the actual source of `preferences: 5`
   on allocno 75 is NOT one of the obvious direct-copy or dead-note-merge paths. NEXT SESSION:
   either (a) request/emit a `ruling-request` asking whether a temporary read-only diagnostic
   print inside `tools/gcc-2.7.2/global.c`'s `set_preference` (dumping allocno+hardreg+insn-UID
   at the point a bit is SET, before `prune_preferences` runs) is in-scope for this session's
   surface — the existing `BB2_ALLOC_DEBUG` hook at global.c:379/605 is read-only diagnostic
   instrumentation already present in the tree (not part of the build), and this would be the
   same kind of addition, but the grind contract's "never edit tools/" line reads as covering
   it, hence flagging rather than just doing it; or (b) exhaustively hand-simulate
   `global_conflicts`'s FULL per-insn scan order (not just the insns directly touching reg75)
   to find an indirect chain (e.g. a THIRD allocno recognized as sharing a `may_share`/coalesce
   slot) — this needs reading `global.c`'s `global_conflicts` function start-to-finish against
   the FULL insn stream (not just the reg75-touching subset), which s3 did not have budget for.
2. Once floor reaches 0: run the mandatory self-vet (6 `__asm__` islands,
   all verbatim SDK macro bodies already authorized for func_800203B4 /
   func_8002E838 — cite those two functions' ledgers/commits as
   precedent, NOT a fresh canonical-asm request) before `candidate-ready`.

## [s1] A literal field-offset transcription (vecA/vecB diff blocks, GTE OP cross-product island, near-zero bailout, LZCS/log2 block, identity-matrix + RotMatrixY/X + two gte_ldv0/MVMVA/stlvnl tail islands) all copied from the already-matched sibling func_8002E838 (same file) and func_800203B4 (src/code6cac.c) produces a near-complete match.
- mechanism: All six cop2 islands in this function are verbatim reuses of PsyQ SDK macro bodies (gte_SetRotMatrix partial-diagonal preamble, GTE OP, gte_ldv0, MVMVA sf=1/mx=rotation/v=V0, gte_stlvnl) already proven byte-exact in func_800203B4 and func_8002E838; the surrounding C is ordinary straight-line arithmetic.
- probe: Wrote candidate.c from the asm transcription, applied to src/code6cac_b.c in place of INCLUDE_ASM, ran sandbox --disable all.
- result: 204 -> 44 on first full write (vecA/vecB pointers cached in s32* locals promoted to $s2/$s3, 48-byte frame vs target's 40).
- verdict: CONFIRMED

## [s1] Caching the cop2-island base addresses (obj+0xA8 for vecA, obj+0xB8 for vecB) in named s32* locals that stay live across the intervening ratan2/RotMatrixX/RotMatrixY calls is the wrong spelling: the target recomputes the addiu $vN,s1,K address fresh at every island use site instead of keeping it in a callee-saved register, so caching it forces GCC to allocate two extra saved registers ($s2/$s3) and an 8-byte-larger frame that the target does not have.
- mechanism: GCC 2.7.2's global.c only promotes a pseudo to a callee-saved hard register when it is a genuinely live-across-call named binding; passing the raw (obj + 0xA8)/(obj + 0xB8) cast expression directly as each inline-asm operand, recomputed at each site, never creates such a pseudo, so no $s2/$s3 gets allocated and the frame naturally drops to match target's addiu sp,sp,-40.
- probe: Removed the s32 *vecA; s32 *vecB; locals from candidate.c, replaced all 4 use sites with the raw (obj + 0xA8)/(obj + 0xB8) expressions inline as the asm operand, re-ran sandbox --disable all --diff.
- result: 44 -> 25; build_insns 202 -> 201; every frame-size/saved-register hunk (sw s2/s3 prologue, addiu-hoist-before-use, lw s2/s3 epilogue, addiu sp,sp,+48 vs +40) present in the v1 diff is completely gone from the v2 diff. The remaining 14 hunks (5 source-level, 5 operand-only, 4 not-scored/masked) are ALL inside the single straight-line dist_sq scratch block (target instruction range 88-119) immediately after the first ratan2 call -- nothing before or after that range differs.
- verdict: CONFIRMED

## [s2] Target's genuine reload of obj+0xC8 (lw v1,200(s1) right after storing the shifted value there) is produced by cc1's cse.c invalidate_memory unconditionally nuking any pointer-relative-address cache entry on ANY subsequent pointer-relative store -- not by a register-allocation spill and not defeatable by re-reading the field through the same statement shape our session-1 candidate used.
- mechanism: tools/gcc-2.7.2/cse.c:7539-7579 (note_mem_written sets writes_ptr->var=1 for any PLUS-address SImode store) + cse.c:1701-1719 (invalidate_memory removes any in_memory table entry p for which cse_rtx_addr_varies_p(p->exp) is true, unconditionally -- not gated on the triggering store's own address). A second pointer-relative write (to ANY field) between C8's shift-store and its read-for-square forces the reload; nothing else does.
- probe: Restructured src/code6cac_b.c's dist_sq block to: write C8 shifted; write D0 shifted (real write, no temp); dist_sq = C8*C8 + D0*D0 in one combined expression; write CC last. Ran sandbox --disable all --diff.
- result: score 25 -> 7 (target_insns 204, build_insns 204); hunks dropped from 5 source-level/5 operand-only/4 not-scored to 2 source-level/1 operand-only/6 not-scored. Confirmed the reload appeared exactly as predicted for C8, and D0 correctly did NOT reload (cache hit on its own just-inserted entry).
- verdict: CONFIRMED

## [s2] Splitting D0's shift into a genuine deferred-store temp (read+shift into a fresh local dz, square dz*dz by ordinary register reuse with no memory write at all, defer the actual *(obj+0xD0)=dz store to the end of the block) reproduces target's D0 handling exactly (single load, immediate square, deferred store), while moving CC's shift-store to serve as the C8-cache invalidator instead of D0's own write.
- mechanism: Same cse.c:1701-1719/7539-7579 rule as the prior hypothesis, with CC (not D0) as the invalidating write; D0 never touches memory before its own deferred store so it can never suffer a false reload of its own.
- probe: Rewrote the block as: write C8 shifted; write CC shifted; { s32 dz = D0>>6; dist_sq = C8*C8 + dz*dz; write D0 = dz; }. Ran sandbox --disable all --diff. Also tried moving CC's write to inside the block (after dz's read, before the dist_sq= statement) as a second ordering test.
- result: Both orderings measured byte-identical: score 25 -> 6 (target_insns 204, build_insns 204); 0 source-level, 5 operand-only (a0<->a1 register-seat rename on dist_sq's final pseudo, propagated through sltiu/addu/bltz/move/srlv), 4 not-scored (masked branch-target-address cascade).
- verdict: CONFIRMED

## [s2] The two-shift-then-two-separate-dist_sq+=-statements ordering (write C8; dist_sq=C8*C8; write D0; dist_sq+=D0*D0; write CC) does not force any reload because the invalidating write for a field must occur BEFORE that field's own read, not merely somewhere in the same block.
- mechanism: cse.c's invalidate_memory only nukes the cache entry retroactively at the time of a later store; a store occurring AFTER a read cannot retroactively invalidate a read that already happened and was folded by CSE at that earlier point in the insn stream.
- probe: Rewrote the block as: write C8; dist_sq=C8*C8 (single statement, no intervening write yet); write D0; dist_sq+=D0*D0; write CC. Ran sandbox --disable all --diff.
- result: score 25 -> 10 (worse than the 6/7 achieved by the other orderings); target_insns 204, build_insns 201; 3 source-level hunks -- neither C8 nor D0 reloaded, confirming ordering-relative-to-the-read is what matters, not merely presence of multiple writes somewhere in the block.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact statement ordering only

## [s2] Doing all three shifts as real immediate writes upfront (C8, D0, CC all written via *(field)=field>>6 before any read), then computing dist_sq=C8*C8+D0*D0 as one combined expression, matches target.
- mechanism: Same cse.c rule, but a THIRD write (CC) occurring after D0's own write and before D0 is read for its square re-invalidates D0's cache entry too, undoing the desired cache-hit-for-D0 behavior.
- probe: Rewrote the block as: write C8; write D0; write CC; dist_sq=C8*C8+D0*D0. Ran sandbox --disable all --diff.
- result: score 25 -> 10; target_insns 204, build_insns 206; 3 source-level hunks -- D0's reload reappeared (its own store got invalidated by CC's later write before D0 was read for the square), confirming D0 must stay as a deferred-store temp (H4) rather than a second immediate write when a third write follows it.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact statement ordering only

## [s3] H5 (traced, mechanism NOT conclusively isolated) — dist_sq's `preferences: 5` in global.c's allocno table is real but its originating insn could not be identified by hand-tracing global.c's set_preference/expand_preferences against the .lreg/.greg dumps
**Statement:** Confirmed via `tmp/grind/func_8002DAD0/dumps/code6cac_b.{lreg,greg}` that
pseudo 75 (dist_sq) is defined once (insn 166: `dist_sq = pseudo129(c8sq) + pseudo130(dzsq)`,
both operands still unresolved pseudos at global-scan time) and used at insn 183 (`sltiu
<1024`, dest resolves to hard v0/reg2 — this DOES call `set_preference` and stamps a
preference for hard reg 2 onto allocno 75 by my trace, but `prune_preferences` (global.c:882)
removes it again because reg75 conflicts with v0/reg2 per the conflicts list, which matches
the dump showing `75 conflicts: 72 75 140 2 3 12 29` including `2`). Insn 225 (the false-branch
`(u32)dist_sq >> shift` — dest resolves to hard v0/reg2 too) traces the same way and is
pruned the same way. Neither traced path explains the surviving `preferences: 5`. Ran
`BB2_ALLOC_DEBUG=1` (existing instrumentation, `tools/gcc-2.7.2/global.c:379/605`, NOT
modified this session — read-only env-gated hook already present) via
`tmp/grind/func_8002DAD0/s3/allocdbg.py`: confirms allocation ORDER (pseudo72 first, pseudo75
second at pri=9230/nrefs=6/livelen=13, landing hardreg=5) and that hard regs 4/5/6/7 (a0-a3)
are ALL in the function's `seed_used` set (call-used regs, not a live-range fact) — so within
`find_reg`'s default ascending-register trial order neither a0 nor a1 should be preferred over
the other absent a real preference bit, yet the dump insists one exists. The mechanism remains
unattributed at the exact-insn level.
**Mechanism:** tools/gcc-2.7.2/global.c `set_preference` (line 1670) + `expand_preferences`
(line 828) + `prune_preferences` (line 882) are the candidate machinery; which specific insn's
scan sets the surviving bit is unresolved.
**Probe:** four C-level lever attempts, each measured with `sandbox --disable all` (and one
with `--diff` to confirm hunk-for-hunk identity):
  1. Swap the addition operand order (`dz*dz + c8*c8` instead of `c8*c8 + dz*dz`).
  2. Move the `*(obj+0xD0) = dz;` store BEFORE the `dist_sq = ...` computation (statement
     reorder within the block, keeping the combined-expression form).
  3. Split the combined expression into two compound-assignment statements
     (`dist_sq = c8*c8; dist_sq += dz*dz;`), both orders tried (c8-first and dz-first).
  4. Introduce a fresh named local for the c8 term (`s32 dx = *(obj+0xC8); dist_sq = dx*dx +
     dz*dz;`) — the named-intermediate/declaration-order lever family.
  5. Reorder the function's own local declarations (move `dist_sq`'s decl to the end of the
     declaration list, after `dist`/`angle1`/`angle2`).
**Result:** (1) and (3-dz-first) regressed to score 13; (2), (3-c8-first), (4), (5) measured
byte-identical to the floor-6 baseline (confirmed via `--diff` for case 4: same 9 hunks,
same `target[108]/[113]/[117]/[119]/[129]` a0-vs-a1 hunks, byte-for-byte). None of the five
tested spellings broke the a1 seat.
**Verdict:** KILLED (all five, individually)
**kill_scope:** instance
**measured_on:** src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, each of the
five statement/declaration orderings listed above, this chassis only (the H3/H4-winning
CSE-invalidation block structure held constant; only the dist_sq computation's internal
shape/decl order varied)

## Session-3 summary
Floor held at 6 (chassis-check-confirmed at dispatch and re-confirmed after every edit this
session). No C-level reordering/staging of the dist_sq computation moved the a0/a1 register
seat in either direction (all either neutral or regressive). The BB2_ALLOC_DEBUG instrumented
dump (already present in tools/gcc-2.7.2/global.c, not edited this session) shows the
allocation order and priority numbers but not a per-preference-bit attribution; a real answer
needs either a NEW read-only debug hook inside global.c's `set_preference`/`expand_preferences`
(printing allocno + hard reg + calling insn UID every time a preference bit is set, before
pruning) — which this session did NOT add, since tools/ edits are outside this session's
allowed surface — or a fully mechanical hand-simulation of `global_conflicts`'s per-insn scan
order that budget did not allow this session. src/code6cac_b.c was reverted to the committed
`INCLUDE_ASM("asm/funcs", func_8002DAD0);` line before this session ended (asm-until-matched:
C lands on main only at COMPLETED-C); candidate.c is unchanged from session 2 (still the
floor-6 form, still the best known).

## [s3] Swapping the dist_sq addition operand order (dz*dz + c8*c8 instead of c8*c8 + dz*dz) breaks or improves the a0/a1 register-seat tie on dist_sq's final pseudo.
- mechanism: Operand order in a PLUS RTL expression affects which sub-expression global.c's set_preference (XEXP(src,0) only) considers for preference stamping.
- probe: Edited src/code6cac_b.c's dist_sq statement to `dist_sq = dz * dz + *(s32 *)(obj + 0xC8) * *(s32 *)(obj + 0xC8);`, ran sandbox --disable all.
- result: score regressed 6 -> 13
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact operand order only

## [s3] Moving the *(obj+0xD0)=dz store to occur BEFORE the dist_sq=... computation (instead of after) breaks the a0/a1 tie.
- mechanism: Statement order changes LUID and RTL insn-stream position of the store relative to the dist_sq PLUS insn, which set_preference/expand_preferences scan in program order.
- probe: Reordered the block to store dz to obj+0xD0 first, then compute dist_sq. Ran sandbox --disable all.
- result: score unchanged at 6, hunks not independently re-diffed for this specific case (superseded by case 4's --diff which confirmed byte-identity for the equivalent-shape variant)
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact statement order only

## [s3] Splitting the combined dist_sq expression into two compound-assignment statements (dist_sq = c8*c8; dist_sq += dz*dz;) breaks the a0/a1 tie.
- mechanism: Compound-assignment split (sanctioned ordinary C per ordinary-c-judge-decidable Ruling 4) creates two separate SET insns instead of one PLUS insn, changing which insns' operands are visible to set_preference.
- probe: Rewrote as two statements, c8-term first. Ran sandbox --disable all.
- result: score unchanged at 6
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact split (c8-term first) only

## [s3] Same compound-assignment split as above but with the dz-term (dist_sq = dz*dz; dist_sq += c8*c8;) instead of the c8-term first.
- mechanism: Same as above, reversed term order.
- probe: Rewrote as two statements, dz-term first. Ran sandbox --disable all.
- result: score regressed 6 -> 13
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact split (dz-term first) only

## [s3] Introducing a fresh named local for the c8 term (s32 dx = *(obj+0xC8); dist_sq = dx*dx + dz*dz;) — the named-intermediate/declaration-order lever family — breaks the a0/a1 tie.
- mechanism: A fresh single-write local changes the RTL pseudo allocated for the c8 value (a genuinely-named pseudo instead of an unnamed re-derived MEM-load), which could alter set_preference's operand resolution at the dist_sq PLUS insn.
- probe: Rewrote the block with `s32 dx = *(s32*)(obj+0xC8);` then `dist_sq = dx*dx + dz*dz;`. Ran sandbox --disable all --diff.
- result: score unchanged at 6; --diff confirmed all 9 hunks byte-for-byte identical to the baseline candidate, including the exact same target[108]/[113]/[117]/[119]/[129] a0-vs-a1 hunks
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact named-intermediate form only

## [s3] Reordering the function's own local declarations (moving `s32 dist_sq;`'s declaration to the end of the decl list, after angle1/angle2/dist) breaks the a0/a1 tie via LUID bias.
- mechanism: Declaration order in GCC 2.7.2 affects pseudo-register LUID assignment, which can bias tie-breaking in global allocation priority ordering.
- probe: Moved the `s32 dist_sq;` declaration to the end of the local variable list. Ran sandbox --disable all.
- result: score unchanged at 6
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact declaration order only

## [s4] Directed permuter campaign on the a0/a1 register-seat tie requires a bespoke per-function permuter workspace, which needed three real tooling fixes before it could compile the chassis at all.
- mechanism: N/A — this is an infrastructure-construction finding, not a C-source hypothesis. Three independent bugs blocked `tools/permuter_campaign.py launch` from ever reaching a live search on this function:
  1. decomp-permuter's bundled pycparser fork restricts GNU extended-asm operands to `unary_expression` (`asm_operand : ... unified_string_literal LPAREN unary_expression RPAREN`, `tools/decomp-permuter/perm_pycparser/c_parser.py:2109`) — it cannot parse `"r"(obj + 0xA8)` (a pointer-arithmetic/additive expression), only a workspace-local rewrite to `"r"(&obj[0xA8])` (array-subscript, which reduces through postfix_expression -> unary_expression) satisfies its grammar. This is a permuter-tooling limitation, not a real candidate-C defect; the committed candidate.c keeps the original `obj + 0xA8` spelling.
  2. Running a Python `open(...).write()` text-mode rewrite of the workspace's preprocessed base.c via the Windows-side Bash tool (not inside WSL) silently converted the whole file to CRLF line endings (confirmed via `file` + byte-count diff: 4670/4670 lines flipped) — this is the documented `windows-python-crlf-write-text` failure mode; it broke every downstream `awk`/regex line-anchor match (`$` doesn't match before a trailing `\r`) and produced spurious cc1 parse errors starting exactly at the edited region. Fix: only ever rewrite workspace .c files from inside a WSL-launched script (`newline="\n"` / binary mode), never via a bare `python3 -c` through the Windows Bash tool.
  3. A full-TU compile of `src/code6cac_b.c` must feed cpp's output to cc1 via **stdin pipe** (`cpp ... | cc1 - -o ...` or `cc1 ... < file.i`), matching the Makefile's pipe-based invocation exactly — passing the preprocessed file as a **filename argument** to cc1 instead produces a corrupted/empty body for func_8002DAD0 specifically (cc1's error-recovery state from an unrelated pre-existing parse error many lines earlier, in `func_80035280`, propagates differently depending on stdin-vs-argv input mode). Confirmed by direct A/B compile of the identical preprocessed content through both invocation forms.
- probe: Built `tmp/perm_8002DAD0/` (base.c preprocessed from `src/code6cac_b.c`, compile.sh mirroring `tools/mar_perm_workspace.sh`'s full-TU-compile + per-function-extraction pattern, target.o built from the RAW ORACLE BYTES at VA 0x8002DAD0-0x8002DE20 via `objcopy -I binary` + section rename to `.text` — NOT from assembling `asm/funcs/func_8002DAD0.s`, since that file's ASPSX-mnemonic GTE ops (`mvmva 1,0,0,3,0`, `op 0`) have no GNU-`as`-compatible spelling and there is no maspsx-equivalent translator for standalone .s files). After fixing bugs 1-3 above, `bash tmp/perm_8002DAD0/compile.sh tmp/perm_8002DAD0/base.c func_8002DAD0 tmp/perm_8002DAD0/base.o` reproduces the real 204-insn body (verified byte-identical instruction count to the engine sandbox's own build). Then attempted `python3 tools/permuter_campaign.py launch --func func_8002DAD0 --dir tmp/perm_8002DAD0 --label s4_regseat -j 4 --stop-on-zero`.
- result: launch still fails — a FOURTH, unresolved pycparser front-end issue: after fix 1, the permuter's own preprocessor/lexer throws `Illegal character '"'` inside the GTE asm string-literal blocks (multiple adjacent `"instr\n"` string-literal-per-physical-line templates, which are valid ISO C adjacent-string-literal concatenation but appear to defeat whatever line-oriented pre-pass the permuter runs before pycparser proper). A same-session attempt to mechanically collapse each multi-line adjacent-string run into one single-line string literal (regex-based, `tmp/grind/func_8002DAD0/s4/fix_asm_strings.py`) still compiled correctly through the real toolchain (204 insns, byte-identical) but did NOT fix the permuter's lexer error, and a naive regex boundary risked corrupting unrelated code elsewhere in the 4670-line TU (observed: a later launch attempt's error pointed at a plain C statement `sq = x * x + z * z;` inside a bogus merged-string span from a DIFFERENT, false-positive-matched function) — this path was abandoned this session rather than risk shipping a mis-transcribed workspace. No permuter search iterations ever ran; the campaign never reached the point of generating or scoring a single mutation.
- verdict: KILLED
- kill_scope: instance
- measured_on: N/A — this is a tooling/workspace-construction result, not a measurement of the candidate C on the real toolchain. The real-toolchain floor (6, same 5 operand-only a0/a1 hunks) was independently re-confirmed this session via `sandbox func_8002DAD0 --disable all --diff` before any permuter work began — see evidence.md.

## Frontier update [s4]
The permuter workspace for func_8002DAD0 now EXISTS and compiles correctly
(`tmp/perm_8002DAD0/{base.c,compile.sh,target.o,settings.toml}` — base.c is
currently in the POST-string-collapse-attempt state, i.e. NOT safe to launch
as-is; see below). A future permuter session should NOT re-derive workspace
construction from scratch — instead:
1. Regenerate `base.c` fresh from `mipsel-linux-gnu-cpp` on `src/code6cac_b.c`
   (do NOT reuse the checked-in tmp/ copy — it carries the abandoned
   string-collapse regex's partial edits and is not verified byte-safe
   outside func_8002DAD0's own region).
2. Apply ONLY the `obj + 0xNN` -> `&obj[0xNN]` operand rewrite inside
   func_8002DAD0's two `"r"(obj + ...)` asm operands (this workspace-only
   respelling is required for the permuter's pycparser fork and does not
   change real codegen — verified byte-identical insn count/objdump both
   ways on the real toolchain).
3. Solve the "Illegal character '"'" pycparser lexer error properly: either
   (a) find the ACTUAL cause in decomp-permuter's own preprocessing (it may
   be model/version-specific — check `tools/decomp-permuter/src/preprocess.py`
   or equivalent for how it strips/retokenizes comments and strings before
   handing off to pycparser, rather than hand-rolling a regex collapse), or
   (b) as a narrower workaround, rewrite JUST func_8002DAD0's own six asm
   blocks (not a whole-file regex) to single-line string literals by hand,
   verifying with a targeted diff that no other function's text was touched.
4. Only after compile.sh AND a permuter launch both succeed against a clean
   base.c should a real campaign run (fresh-seed discipline, 20-30 min,
   `tools/permuter_campaign.py wait` in-turn, `harvest --stop` before session
   end).
5. Always write any workspace .c file mutation via a WSL-launched script
   (never a bare Windows-side `python3 -c`) — see the CRLF-corruption entry
   above; this cost the majority of this session's turns once already.

## [s4] Directed permuter campaign on the a0/a1 register-seat tie requires a bespoke per-function permuter workspace, which needed three real tooling fixes (asm-operand grammar restricted to unary_expression, a Windows-Python CRLF-corruption trap, a stdin-vs-argv cc1 full-TU compile bug) before it could even compile the chassis, and a fourth unresolved pycparser lexer issue then blocked the campaign from ever launching.
- mechanism: N/A — infrastructure-construction finding, not a C-source/GCC-pass hypothesis. See memory/grind/func_8002DAD0/hypotheses.md [s4] for the full mechanism of each of the three fixed bugs and the one unresolved blocker.
- probe: Built tmp/perm_8002DAD0/ (base.c preprocessed from src/code6cac_b.c, compile.sh mirroring tools/mar_perm_workspace.sh, target.o built from raw oracle EXE bytes via objcopy since asm/funcs/func_8002DAD0.s's ASPSX GTE mnemonics have no GNU-as encoding). Fixed the asm-operand grammar (obj+0xNN -> &obj[0xNN]), the CRLF corruption (rewrite via WSL script only), and the cc1 stdin-vs-argv bug; workspace then compiled the real 204-insn body correctly. Attempted tools/permuter_campaign.py launch --func func_8002DAD0 --dir tmp/perm_8002DAD0 --label s4_regseat -j 4 --stop-on-zero.
- result: Launch failed both before and after a same-session attempt to mechanically collapse the GTE asm blocks' multi-line adjacent string literals into single-line strings (tmp/grind/func_8002DAD0/s4/fix_asm_strings.py) — decomp-permuter's pycparser fork still threw 'Illegal character "' during its own preprocessing/lexing pass, and the regex-based collapse risked corrupting unrelated code elsewhere in the 4670-line TU (a later attempt's error pointed at a plain statement in a different, false-positive-matched function), so it was abandoned rather than risk a mistranscribed workspace. Zero permuter search iterations ran; no mutation was ever generated or scored.
- verdict: KILLED
- kill_scope: instance
- measured_on: N/A (tooling/workspace-construction result, not a measurement of candidate C on the real toolchain). The real-toolchain floor (6, same 5 operand-only a0/a1 hunks as s2/s3) was independently re-confirmed this session via sandbox --disable all --diff before any permuter work began.

## [s5, enumerate] Exhaustive tool-based spelling sweep of both structurally-flat sub-blocks touching dist_sq/dist (tools/spelling_enum.py + tools/sweep_variants.py)
- mechanism: N/A auto-search coverage confirmation, not a single GCC-pass claim per hypothesis. Two ENUM regions were swept, each fully self-contained inside its own `{ }` block (safe for the tool's flat def-before-use permuter, which does not understand nested control flow — confirmed by a first mis-scoped attempt below).
- probe A (dist_sq compute block): region = `s32 dz = *(s32*)(obj+0xD0) >> 6; dist_sq = *(s32*)(obj+0xC8) * *(s32*)(obj+0xC8) + dz*dz; *(s32*)(obj+0xD0) = dz;` marked INSIDE its existing enclosing `{ }` (a first attempt that stripped the enclosing braces produced a structurally different candidate that scored 27/191 insns purely from the missing block-scope nesting — a tooling-construction artifact, not a finding about dist_sq itself; corrected before any real sweep). `spelling_enum.py --no-swaps` -> 2 distinct spellings (axis1: keep `dz` named, or fully inline it into the `dist_sq=` expression; axis2 n/a, only one valid decl order; axis3 swaps n/a — neither multiplicand is a bare identifier or parenthesised group, so the commutative-swap regex finds nothing, confirmed via `--list` with swaps enabled: count unchanged at 2). `sweep_variants.py --json` scored both: named-dz (current candidate form) = 6/204 (== baseline, exact byte match to what's on disk), fully-inlined-dz = 6/204 (byte-identical insn count, neutral).
- probe B (shift/tbl/dist compute block, the else-arm's trailing compound, entirely inside its own `{ }` after the lzcr conditional): region = `s32 shift = 0x16 - (lzcr & ~1); s32 tbl = *(((u8*)&D_8008D118) + ((u32)dist_sq >> shift)); dist = (u32)(tbl<<16) >> (0x13 - ((u32)shift>>1));`. `spelling_enum.py --no-swaps` -> 4 distinct spellings (axis1: independently keep/inline `shift` and `tbl`, giving the 4 combinations of {named,inlined}x{named,inlined} respecting def-before-use since `tbl`'s expr uses `shift`; axis3 swaps n/a — no `*`/`+` binary op appears in the region at all, confirmed unchanged count with swaps enabled). All 4 swept: 6/204 each, byte-identical to baseline.
- result: 6 of 6 total generated spellings across both flat sub-blocks measured EXACTLY 6/204 (or the one mis-scoped 27/191 outlier, which was a tooling artifact from stripping block-scope braces, not a real spelling variant, and was corrected out before scoring). Zero spelling in either region's exhaustively-enumerated space moves the floor off 6. This is consistent with s2/s3's diff classification (0 source-level hunks, 5 operand-only a0/a1 register-seat hunks, 4 not-scored) and with s3's hand-picked probes of the same block (compound-split both directions, named-intermediate, decl-order) — the tool-based sweep now covers the FULL space those hand probes sampled from, not just the specific forms tried, and confirms the same conclusion by construction rather than by sample.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact 2-region enumerated spelling space (6 variants total) on the current chassis (floor 6, candidate.c body applied)

## [s5] Exhaustive tool-enumerated spelling sweep (named vs. fully-inlined dz, all def-before-use-valid orderings, commutative-swap axis checked and found inapplicable) of the dist_sq compute block `{ s32 dz = ...; dist_sq = ...; *(obj+0xD0) = dz; }` produces 2 distinct spellings, both scoring 6/204 (byte-identical to the current candidate).
- mechanism: N/A -- auto-search coverage result, not a GCC-pass claim. spelling_enum.py's axis1 (name-vs-inline) and axis3 (commutative swap) exhausted this region's textual spelling space by construction.
- probe: tools/spelling_enum.py --candidate tmp/grind/func_8002DAD0/s5/enum_base.c --out tmp/grind/func_8002DAD0/s5/enum2 --no-swaps (2 variants); tools/sweep_variants.py --func func_8002DAD0 --file code6cac_b --variants tmp/grind/func_8002DAD0/s5/enum2 --json
- result: Both variants (named dz, fully-inlined dz) scored 6/204, matching the baseline exactly. A first attempt that stripped the block's enclosing braces (structurally different nesting, not a real spelling variant) mis-scored 27/191 -- corrected before real measurement; noted in hypotheses.md so no future session mistakes it for a finding about dist_sq.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact 2-spelling enumerated space (dist_sq compute block only) on the current chassis (floor 6)

## [s5] Exhaustive tool-enumerated spelling sweep (independent name-vs-inline on `shift` and `tbl`, all 4 combinations respecting def-before-use; commutative-swap axis checked and found inapplicable -- no `*`/`+` operator appears in the region) of the shift/tbl/dist compute block `{ s32 shift=...; s32 tbl=...; dist=...; }` produces 4 distinct spellings, all scoring 6/204.
- mechanism: N/A -- auto-search coverage result. Same tool, same axes, disjoint region from the dist_sq block above (entirely inside its own compound statement, no nested control flow crossed).
- probe: tools/spelling_enum.py --candidate tmp/grind/func_8002DAD0/s5/enum_base.c --out tmp/grind/func_8002DAD0/s5/enum_shift --no-swaps (4 variants, --list confirms swap axis adds nothing); tools/sweep_variants.py --func func_8002DAD0 --file code6cac_b --variants tmp/grind/func_8002DAD0/s5/enum_shift --json
- result: All 4 variants (named/named, named/inlined, inlined/named, inlined/inlined for shift/tbl) scored 6/204, identical to baseline and to each other.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/code6cac_b.c func_8002DAD0, pure C, zero FAKE constructs, this exact 4-spelling enumerated space (shift/tbl/dist compute block only) on the current chassis (floor 6)

## [s6, synthesis] H6 CONFIRMED - dist_sq's $a1 seat is an expand_preferences merge from the dying `dz*dz` product, and it disappears when the Z-delta carrier is a multi-basic-block variable
**Statement:** dist_sq's allocno carries `own_full_prefs: 5` and an EMPTY
`own_copy_prefs`, so the bit was stamped by global.c set_preference's `copy == 0`
path (arithmetic RHS, first operand unwrapped) onto the allocno of the `dz*dz`
product, and then merged onto dist_sq by global.c expand_preferences because the
product dies in the insn that defines dist_sq and the two allocnos do not
conflict. The stamp is only possible because a fresh block-local `dz` is a
local-alloc quantity with a real `reg_renumber`; a carrier variable referenced in
more than one basic block is a global allocno (`reg_renumber == -1` at
global_conflicts time), so no preference exists and find_reg keeps its natural
ascending pick of $a0, which is what the target uses.
**Mechanism:** tools/gcc-2.7.2/global.c:1484 (mark_reg_store -> set_preference),
:1670 (set_preference, `copy=0` for an 'e'-format RHS), :828
(expand_preferences REG_DEAD merge), :882 (prune_preferences), :1056-1083
(find_reg's ascending pass-0/pass-1 scan) and :1130 (the preference override).
**Probe:** (a) ran the pre-existing read-only hook `BB2_FINDREG_DEBUG=75` through
the project's exact cpp|cc1 pipeline with the instrumented cc1
(tmp/grind/func_8002DAD0/s6/findreg.sh) and read the preference sets directly;
(b) confirmed every link of the chain against the -da .lreg/.greg dumps
(`;; Register dispositions: ... 125 in 5 126 in 5`, insn 164's mult, insn 166's
two REG_DEAD notes, the 75/130 conflict lists); (c) measured four carrier
variants plus a control.
**Result:** CONTROL (fresh function-scope `s32 dz2;` still used in only one basic
block) = 6, unchanged - so declaration scope is not the lever. `dist` as carrier
= 5; `angle2` as carrier = 5 (both kill all five a0/a1 operand-only hunks).
`dist` as carrier WITH the load split from the shift onto the same variable
(`dist = *(s32 *)(obj + 0xD0); dist >>= 6;`) = **0**; `angle2` with the same
split = **0**. `verify-oracle` then reported build_matches=true with
build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (the locked oracle).
**Verdict:** CONFIRMED

## [s6] H7 CONFIRMED - the second residual after the carrier fix is a two-pseudo load/shift, closed by splitting the load from the shift onto the same variable
**Statement:** With the Z delta carried by a global-allocno variable, the
unsplit `dist = *(s32 *)(obj + 0xD0) >> 6;` still produces two pseudos - a
single-block load temp and the multi-block carrier - so the load lands in $v1
and the shift writes $a1 (`lw v1,208(s1); sra a1,v1,6`) where the target uses one
register for both (`lw a1,208(s1); sra a1,a1,6`), and the D0 store schedules
after the 0xFA store instead of before it.
**Mechanism:** one C variable is one pseudo in GCC 2.7.2, so writing the load and
the shift into the SAME variable collapses the pair; ordinary split-init /
compound-assignment splitting, no exception needed
([[split-init-accumulation-sanctioned]]).
**Probe:** measured `dist = *(s32 *)(obj + 0xD0) >> 6;` (5, banked at
rejected/s6-dist-reuse-unsplit-load-score5.c) against
`dist = *(s32 *)(obj + 0xD0); dist >>= 6;` (0). Also measured `dist = dist >> 6;`
as a spelling variant of the same split: also 0 (identical bytes); `>>=` kept as
the more idiomatic form.
**Verdict:** CONFIRMED

## [s6] KILL RE-AUDIT of the s3/s5 instance kills - all 11 stand as INSTANCE kills and none was void, but they were all measured on the WRONG level
**Statement:** Every s3/s5 kill varied the SHAPE of the dist_sq arithmetic while
holding the carrier a fresh single-basic-block block-local. On the s6 mechanism
that variable choice is the thing that stamps the preference, so the entire s3/s5
search space sat inside one fixed value of the deciding parameter; the kills are
accurate for that parameter value and were correctly scoped `instance`. No FAKE
construct was present on the s2-s5 chassis, so `tools/fake_ablate.py` had nothing
to ablate; the re-audit was performed instead by re-measuring the chassis
(6, matching the driver's dispatch-time chassis check exactly) and by running the
CONTROL variant, which reproduces a kill-equivalent result (6) from the opposite
direction and pins the cause.
**Mechanism:** see H6.
**Probe:** re-measured the s2-s5 candidate.c on the current chassis (6) and the
fresh-function-scope-local control (6) before any new probe.
**Verdict:** CONFIRMED (the kills stand; their scope was the limiting factor,
not their accuracy)

## Frontier (reset by s6)
The function is MATCHED and the frontier is now integration-only:
1. **inline_asm_canonical.txt row.** func_8002DAD0 has no row of its own. Its
   six islands are character-identical to func_8002E838 (:373) and
   func_800203B4 (:367), and .claude/rules/cop2-addressing-preamble-cluster.md:76
   names func_8002DAD0 as a cluster member. `canonical func_8002DAD0` =>
   ASM-PARTIAL, 29/204 insns canonical-asm. Writing that row is an
   operator/driver step (the file is outside a grind session's surface).
2. **D_8008D118 declaration.** The byte LUT has no header declaration anywhere;
   the TU-local `extern u8 D_8008D118;` (src/code6cac_b.c:278) plus
   `*(((u8 *)&D_8008D118) + idx)` is the shipped idiom of the matched
   COMPLETED-C func_8002D320 in this same file (:1154, :1170) and 6 further
   sites. A proper `extern u8 D_8008D118[];` declaration is a TU-wide change
   touching 9 already-matched functions - a separate, deliberate integration
   task, not something to slip into this match.
3. **Generalization.** The H6 mechanism is not specific to this function. Any
   residual of the shape "one pseudo, one register off, operand-only hunks only"
   should start with `BB2_FINDREG_DEBUG=<pseudo>`: empty `own_copy_prefs` with a
   non-empty `own_full_prefs` means the bit came from set_preference's copy=0
   path and travelled through an expand_preferences REG_DEAD edge, and the C-level
   fix is to change the basic-block span of the variable feeding the arithmetic.
   Worth promoting to a technique rule under codegen-technique-index.

## [s7] The s6 body reproduces distance 0 on a fresh apply - the match is not chassis-luck
**Statement:** Re-applying memory/grind/func_8002DAD0/candidate.c (byte-identical
to the s6 body, including the single FAKE-annotated `dist` staging) to the
`INCLUDE_ASM("asm/funcs", func_8002DAD0);` site in src/code6cac_b.c reproduces
`sandbox --disable all` == 0 (204/204) and a full-build SHA1 equal to the locked
oracle, in a session that shares nothing with s6 but the file on disk.
**Mechanism:** none needed - this is a reproduction check, not a lever. It exists
because the driver's dispatch-time chassis check and the s6 ledger disagreed
(chassis said 0, ledger floor said 6), and a chassis discontinuity is exactly the
situation where a banked spelling conclusion must be re-measured rather than
quoted.
**Probe:** tmp/grind/func_8002DAD0/s6/apply.py -> `sandbox func_8002DAD0
--disable all` -> 0; `canonical func_8002DAD0` -> ASM-PARTIAL 29/204;
`verify-oracle` -> build_matches true, 62efab4f73f992798c43e8c730aa43baa10bb4fa.
**Verdict:** CONFIRMED

## [s7] A self_vet SCOPE quote wrapped across physical lines is worth ZERO quotes to the driver
**Statement:** `tools/grinder/grindlib.py`'s `_SCOPE_LINE` regex anchors the
closing quote to end-of-line, so a `SCOPE:` quotation that wraps onto a second
physical line is not counted at all, and a candidate-ready session whose family
count exceeds its single-line scope count is discarded before any review runs -
which is the only reason s6's proven match did not land.
**Mechanism:** grindlib.py:83 `_SCOPE_LINE`, consumed by
grindlib.py:validate_self_vet (the `len(scopes) < len(fams)` branch at
grindlib.py:119).
**Probe:** rewrote all three SCOPE quotes as single physical lines and ran the
driver's validator directly (tmp/grind/func_8002DAD0/s6/check_vet.py) ->
`VALIDATOR ok= True`, reason empty. Before the rewrite the same call reported the
3-families/2-scopes failure verbatim.
**Verdict:** CONFIRMED

## Frontier (unchanged from s6 - integration-only)
The function is MATCHED; see the s6 frontier above. Nothing new was opened.
