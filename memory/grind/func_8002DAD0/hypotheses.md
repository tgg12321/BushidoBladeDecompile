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

## Frontier (<=3, carried to next session)
1. **dist_sq scratch-block reload/ordering (H3 above)** — read the
   `.cse`/`.greg` dump for this exact block, try the 3 named C
   restructurings in order (a)/(b)/(c), remeasure after each. This alone
   is expected to close most or all of the remaining 25.
2. If H3's dump shows a genuine allocation tie rather than a reload
   question, apply the `local-alloc-death-count-class-wall` /
   `register-alloc-pure-c` lever playbook to the C8-vs-sumsq register
   choice (v1-family vs a-family) — read
   `.claude/rules/register-alloc-pure-c.md` before guessing.
3. Once floor reaches 0: run the mandatory self-vet (6 `__asm__` islands,
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
