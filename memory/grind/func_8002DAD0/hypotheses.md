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
1. **dist_sq final-pseudo register seat (a0 vs a1)** — `.greg` dump for
   this session (tmp/grind/func_8002DAD0/dumps/code6cac_b.greg, function
   at line 11463) shows the dist_sq pseudo (reg 75) with `preferences: 5`
   and no conflict against hard reg 4; a sibling pseudo (reg 78, likely
   `dz` or `angle1`) has `preferences: 2 5` and DOES conflict with hard
   reg 4 (`;; 78 conflicts: 72 78 4 29`). Target wants dist_sq in a0 (hard
   reg 4). Read `local-alloc.c`'s preference-assignment code
   (`reg_pref`/`reg_pref_class`) to find WHAT copy/call insn is stamping
   preference 5 onto reg 75, then apply a `register-alloc-pure-c` lever
   (block-local split / narrow type / reordering the dz-block's internal
   statement order) to shift that preference to 4 instead. Two orderings
   already tried and measured IDENTICAL (byte-for-byte) at floor 6 — see
   evidence.md — so the next attempt needs to change WHICH statement
   creates the reg75/reg78 preference, not just reshuffle statement
   order within the already-explored positions.
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
