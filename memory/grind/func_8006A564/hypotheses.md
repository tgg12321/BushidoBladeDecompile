# Hypotheses — func_8006A564

## H1 (session 1) — srl vs sra on the packed-color halving ops
**Statement:** casting the halved color value through `(u32)` before `>>1`
makes GCC emit `srl` instead of `sra`, matching target at both halving
sites.
**Mechanism:** GCC's shift-right codegen picks `sra`/`srl` based on the
shifted operand's signedness at the RTL level; `s32 v0` defaults to
arithmetic shift, `(u32)v0` forces logical.
**Probe:** objdump `tmp/sandbox/func_8006A564/text1b.o` before/after the
cast; grep for `sra`/`srl` at the two halving sites.
**Result:** CONFIRMED — both sites now emit `srl s0,s0,0x1` byte-identical
to `asm/funcs/func_8006A564.s:101` and `:148-149`. Sandbox masked score
unchanged at 137 (this diff was not the dominant residual — see H2).

## H2 (session 1, frontier — NOT yet probed) — register-allocation mapping
**Statement:** the +5 instruction / 137-point residual is entirely a
register-allocation mapping difference: our build's callee-save set is
{s0,s1,s2,s3,s4,s5} mapped {arg1,scratch,arg1... } vs target's
{s0,s1,s2,s3,s4} mapped {scratch,arg1,arg0,arg2,flag-local}. Fixing the
source structure (declaration order of `tile`/`obj2`/`v0`/`s4`, or
parameter-liveness shape) so GCC's global allocator assigns the same 5
registers (dropping our extra 6th) should close most or all of the
137-point gap.
**Mechanism (untested):** GCC 2.7.2's `global.c` allocno-priority ordering
is sensitive to first-use order and live-range length of each pseudo;
right now `tile` is reused/reassigned many times (long live range,
crosses all 3 tile-draw blocks + the final section) which may be pushing
it to a scratch register with a longer conflict list, forcing one extra
value to spill to a callee-save slot instead of reusing a temp already
freed by DCE.
**Next probe:** `pwsh tools/grinder/dump.ps1 func_8006A564` then read the
`.greg`/`.lreg` dump to see which pseudo lands in the extra register (s5
equivalent) and why it doesn't get folded into an already-freed hard reg;
try (a) declaring `s4`-equivalent local FIRST (before `tile`/`obj2`/`v0`)
to bias LUID, (b) splitting `tile`'s live range at the `obj2 = ...` /
`tile = *(obj2+0x1C)` reassignment into a distinctly-named local (the
asm literally reuses $s0 for two logically different pointers — a fresh
C name at that reassignment point may look "more natural" to the
allocator AND is arguably the more truthful spelling: it's a different
object). Do NOT try register-asm pins or scheduling barriers — plain
declaration-order / naming levers only, per the register-alloc-pure-c
technique family.

## H2 (session 2) — CONFIRMED: block-local var split closes the register-mapping gap
**Result:** Fully confirmed. Splitting `v0` into per-tile-draw-block `{ }`
scopes (fresh `s32 v0;` per block) dropped the hard-reg set from
{s0,s1,s2,s3,s4,s5} to exactly {s0,s1,s2,s3,s4} and the per-variable mapping
came out matching target with no additional declaration-order work needed.
Sandbox 137 -> 74. See evidence.md [s2] for the full `.greg` dump citation.
**verdict:** CONFIRMED (kill_scope n/a -- this is a positive/CONFIRMED
result, not a kill).

## H3 (session 2) — KILLED: splitting the AND expression does not fix v0/v1 coloring
**Statement:** rewriting `v0 = D_800A34F8 & 0xF;` as two statements
(`v0 = D_800A34F8; v0 &= 0xF;`) will make cc1 reuse the load's register for
the in-place AND, matching target's `andi $v0,$v0,0xF` (same reg as the
preceding `lw $v0,...`).
**Mechanism (hypothesized):** GCC's `combine`/local-alloc might coalesce a
load-then-immediately-consumed temp into the same hardreg as its single
user when they're separate insns with an obvious single-def/single-use
chain, more readily than when the RHS is a single compound expression.
**Probe:** Applied the 2-statement form at all 4 `D_800A34F8 & 0xF` sites,
re-measured via `sandbox --disable all`.
**Result:** Score went from 68 to 72 (WORSE). Reverted to the single-
expression form (`v0 = D_800A34F8 & 0xF;`).
**verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c s2 final chassis (block-local var split + all
s2 field-offset fixes applied), zero FAKE constructs present.

## Frontier for next session
1. **The systemic v0<->v1 register-coloring swap (68-point residual, ALL
   of it).** Read the `.lreg`/`.combine` dumps (not yet read this session
   -- `pwsh tools/grinder/dump.ps1 func_8006A564` already regenerates them
   in tmp/grind/func_8006A564/dumps/) for the FIRST `D_800A34F8 & 0xF` site
   specifically: does cc1 emit the load and the AND as two RTL insns with
   two DIFFERENT pseudo REGNOs that then get colored to v0/v1 in the
   "wrong" order relative to target, or does it coalesce them into one
   pseudo that then gets colored v1 instead of v0? The two diagnoses need
   different levers: (a) if two pseudos, a decl-order / statement-order
   change might bias local-alloc's processing order (which pseudo is
   "seen first" and gets first pick of the lowest free hardreg); (b) if
   one coalesced pseudo, the issue is a global.c allocno-priority
   tiebreak, and the lever is more likely an unrelated nearby live range
   changing conflict counts. H3 already KILLED the "split the expression
   into two statements" attempt at the SOURCE level, so this needs the
   RTL-level read before the next lever guess, not another blind
   statement-shape experiment (see the PASS ATTRIBUTION contract in the
   session brief -- guessing pass attribution across sessions is exactly
   the failure mode it exists to prevent).
2. **The single block3 scheduling tie** (`sb $v0,0x6($s0)` vs
   `addu $a0,$s0,$zero` ordering) -- low weight, likely secondary to #1;
   revisit after the coloring issue is understood, since fixing #1 may
   shift RTL insn numbering enough to change this tie's resolution for
   free.
3. If #1's RTL-level diagnosis doesn't yield an obvious C lever, run the
   permuter on `asm/funcs/func_8006A564.s` (clean single-function target
   per `difficult-is-not-impossible.md` §3) directed at register-mapping
   variants (`--stop-on-zero`) -- not yet attempted in this ledger's
   2 sessions (R3 in asm-until-matched.md caps this at 2 permuter
   sessions ever for this function; none spent yet).

## [s1] Casting the halved packed-color value through (u32) before >>1 makes GCC emit srl instead of sra, matching target at both color-halving sites (block 3 and the final color-select block).
- mechanism: GCC 2.7.2 picks arithmetic vs logical right-shift codegen from the shifted operand's C signedness; s32 defaults to sra, (u32) forces srl.
- probe: objdump tmp/sandbox/func_8006A564/text1b.o before/after the cast; diff against asm/funcs/func_8006A564.s:101,105,148,149.
- result: Both halving sites now emit `srl s0,s0,0x1` byte-identical to target. Sandbox masked score unchanged at 137 (this was a real but non-dominant instruction-level fix; the register-allocation gap in H2 dominates the residual).
- verdict: CONFIRMED

## [s2] Confining the reused `s32 v0;` local to a fresh block-scoped declaration per tile-draw block (instead of one function-wide declaration) drops the global allocator's hard-reg set from {s0,s1,s2,s3,s4,s5} to exactly {s0,s1,s2,s3,s4}, matching target's callee-save mask and per-variable register mapping with no further declaration-order work.
- mechanism: GCC 2.7.2's global.c allocates registers per-pseudo based on live-range/conflict analysis; a single source-level variable reused across every basic block in the function is treated as one long-lived pseudo whose live range crosses many blocks, forcing promotion to an extra callee-saved register. Block-scoping the declaration gives each instance a short, block-confined live range.
- probe: Read tmp/grind/func_8006A564/dumps/text1b.greg before/after the split; compare `;; Hard regs used:` line and the pseudo->hardreg disposition table against asm/funcs/func_8006A564.s's prologue mask/register moves.
- result: Sandbox --disable all score 137 -> 74 (build_insns 204 -> 202). Hard regs used dropped to exactly {2,3,4,5,6,7,16,17,18,19,20,29,31} (no s5/21), and pseudo->hardreg mapping matched target's arg0->s2, arg1->s1, arg2->s3, tile->s0, flag-local->s4 exactly.
- verdict: CONFIRMED

## [s2] Splitting the combined expression `v0 = D_800A34F8 & 0xF;` into two statements (`v0 = D_800A34F8; v0 &= 0xF;`) will make cc1 color the AND's result into the same hard register as the preceding load, matching target's `andi $v0,$v0,0xF` in-place pattern and closing (part of) the systemic v0<->v1 coloring swap that accounts for the whole 68-point residual.
- mechanism: Hypothesized: GCC's local-alloc/combine might coalesce a load-then-immediately-consumed temp into its single user's hardreg more readily when they are separate RTL insns from separate C statements than when the RHS is one compound expression.
- probe: Applied the 2-statement split at all 4 `D_800A34F8 & 0xF` sites in the s2 candidate (chassis: block-local var split + all field-offset fixes already applied, build_insns==target_insns==199, floor 68), re-measured via `& tools/wteng.ps1 main sandbox func_8006A564 --disable all`.
- result: Score regressed from 68 to 72 (worse). Reverted to the single-expression form, confirmed back at 68.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c s2 final chassis (block-local var split applied, all s1-bug field-offset fixes applied), zero FAKE constructs present

## H4 (session 3) — KILLED: splitting the AND into a genuinely fresh
separately-named local does not fix the v0/v1 coloring
**Statement:** unlike H3 (which reused the SAME variable `v0` for both the
raw load and the AND, `v0 = D_800A34F8; v0 &= 0xF;`), introducing a
genuinely FRESH, separately-named local for the raw load
(`s32 raw; raw = D_800A34F8; v0 = raw & 0xF;`) might let cc1 treat the two
values as more clearly distinct and color the AND-result into the load's
hard register.
**Mechanism (hypothesized):** a fresh named local might get different
`regclass`/local-alloc preference treatment than a variable already
carrying two roles.
**Probe:** applied at site 1 only, re-measured via `sandbox --disable all`;
also re-dumped `.greg` to check the actual RTL.
**Result:** score UNCHANGED (68 -> 68). The `.greg` dump showed IDENTICAL
RTL insns/pseudo numbers to the pre-split form — GCC's combine pass folds
the extra named copy back to the exact same two-insn shape
(`load into pseudo A; and(A,15) into pseudo B`) regardless of whether A is
named `raw` or is anonymous. Naming does not change RTL shape once combine
runs.
**verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c s2 final chassis (floor 68), single site,
zero FAKE constructs present.

## H5 (session 3) — CONFIRMED: removing the named mask-compute variable
before the branch (inlining the AND directly in the if-condition) drops
the floor 68 -> 60
**Statement:** replacing `v0 = D_800A34F8 & 0xF; if (v0 == arg2) {...}`
with `if ((D_800A34F8 & 0xF) == arg2) {...}` at all 4 sites — so that `v0`
(used only inside/after the branch arms for the tile-draw byte values) is
NEVER assigned by the mask compute at all, and the AND result becomes an
anonymous compiler temp consumed only by the comparison — measurably
lowers the sandbox score.
**Mechanism:** NOT fully attributed this session. A register-normalized
objdump diff (`tmp/grind/func_8006A564/s3/diff.py`) against
`asm/funcs/func_8006A564.s` after this change shows the v0<->v1 coloring
swap identified in session 2 is STILL PRESENT, unchanged, at all 4 sites —
so this fix is NOT closing the coloring swap. The `.greg` dump
(`tmp/grind/func_8006A564/dumps/text1b.greg`, func_8006A564 section) after
this change shows the identical pseudo->hardreg disposition as before
(raw-load pseudo -> hardreg 2/v0 via local-alloc; AND-result pseudo ->
hardreg 3/v1 via global-alloc). The 8-point win must come from some OTHER,
distinct diff this session did not isolate.
**Probe:** applied incrementally (site 1 alone: 68->66; all 4 sites:
68->60) via `sandbox --disable all`; re-dumped and diffed against target
to check whether the coloring swap closed (it did not).
**Result:** CONFIRMED as a real, measured improvement (68 -> 60,
build_insns unchanged at 199==199). Reverted the companion "reverse the
comparison operand order" variant (`arg2 == (D_800A34F8&0xF)`), which
measured WORSE (60 -> 64) and was discarded — see H6.
**verdict:** CONFIRMED

## H6 (session 3) — KILLED: reversing the comparison operand order
regresses the score
**Statement:** per [[compare-operand-order-register]], writing
`arg2 == (D_800A34F8 & 0xF)` (parameter first) instead of
`(D_800A34F8 & 0xF) == arg2` (global-derived value first) might bias RTL
emission order and help the coloring swap, mirroring the rule's local-vs-
global lever.
**Mechanism (hypothesized):** cc1's RTL evaluation order for `==` may
depend on LHS/RHS position the same way `<`/`>` does per the cited rule.
**Probe:** applied at all 4 sites atop the H5 chassis (floor 60), measured
via `sandbox --disable all`.
**Result:** score regressed 60 -> 64 (worse). Reverted to
`(D_800A34F8 & 0xF) == arg2`, confirmed back at 60.
**verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c s3 chassis (H5 applied, floor 60), zero FAKE
constructs present.

## Frontier for next session
1. **The v0/v1 coloring swap is STILL the entire residual pattern visible
   in the target-vs-build diff** (confirmed unchanged by H5's 8-point win,
   which must be closing something else — not yet isolated which
   instructions). Next session should diff session-2's build.dis.txt
   against session-3's build.dis.txt (not each against target) to find
   exactly which instructions changed between floor 68 and floor 60, to
   properly attribute H5's mechanism before trying further coloring levers.
2. **GCC 2.7.2's MIPS backend has no `REG_ALLOC_ORDER` override**
   (confirmed: grepped `tools/gcc-2.7.2/config/mips/mips.h`, no match) — so
   local-alloc's hard-reg search is the default ascending numeric order.
   The raw-load pseudo (block-local, single def/use, dies same insn) is
   handled by LOCAL-ALLOC and grabs the first free int-class hardreg (v0)
   before GLOBAL-ALLOC (which runs after local-alloc, per GCC's pass
   ordering) ever processes the AND-result pseudo (which crosses the
   if/else merge and is therefore NOT local-alloc-eligible) — global-alloc
   is then forced to v1. For target to have colored these the other way,
   target's compile must not treat the raw-load temp as purely local the
   way ours does. Untried: hoisting all 4 sites' `D_800A34F8` read into ONE
   shared read outside the loop-unrolled tile blocks (probably changes
   control flow too much / not applicable — the 3 tile blocks + final
   section are NOT a loop); or restructuring the arm-value variable (`v0`)
   so it does NOT itself cross the if/else as one pseudo (try genuinely
   separate per-arm value locals combined through a different mechanism
   than one shared block-scoped `v0` — NOT yet attempted).
3. **The single block3 scheduling tie** (unchanged from session 2,
   unattempted this session) — low weight, revisit after the coloring
   issue is understood.

## [s3] A genuinely fresh, separately-named local for the raw D_800A34F8 load (distinct from H3's same-variable split) will make cc1 color the AND result into the load's hard register.
- mechanism: Hypothesized: a fresh named local might get different regclass/local-alloc preference treatment than a variable already carrying two roles.
- probe: Applied `s32 raw; raw = D_800A34F8; v0 = raw & 0xF;` at site 1, re-measured via sandbox --disable all, re-dumped .greg to compare RTL.
- result: Score unchanged (68 -> 68). The .greg dump showed byte-identical pseudo numbers and hardreg assignments to the pre-split form -- combine folds the extra named copy back to the exact same two-insn shape regardless of the C-level name given to the load result.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c s2 chassis (floor 68), single site, zero FAKE constructs present

## [s3] Removing the named mask-compute variable entirely -- writing `if ((D_800A34F8 & 0xF) == arg2) {...}` instead of `v0 = D_800A34F8 & 0xF; if (v0 == arg2) {...}` -- at all 4 sites lowers the sandbox score from the session-2 floor.
- mechanism: Not fully attributed this session. A register-normalized objdump diff (tmp/grind/func_8006A564/s3/diff.py) confirms the v0<->v1 coloring swap from session 2 is STILL PRESENT unchanged at all 4 sites after this fix, so the improvement is NOT from closing that swap; the true mechanism is an unidentified secondary diff, left as the top frontier item.
- probe: Applied incrementally (site 1 alone, then all 4 sites) via `& tools/wteng.ps1 main sandbox func_8006A564 --disable all`; re-dumped .greg and diffed the rebuilt .o against target to confirm whether the coloring swap closed (it did not).
- result: CONFIRMED as a real, measured improvement: 68 -> 66 (site 1 alone), 68 -> 60 (all 4 sites), build_insns unchanged at 199==199 both before and after.
- verdict: CONFIRMED

## [s3] Reversing the comparison's operand order (`arg2 == (D_800A34F8 & 0xF)` instead of `(D_800A34F8 & 0xF) == arg2`) at all 4 sites, per the compare-operand-order-register lever, will help the coloring swap.
- mechanism: Hypothesized by analogy to .claude/rules/compare-operand-order-register.md, which documents this lever for `<`/`>` comparisons biasing RTL evaluation order; the rule itself notes it may not generalize to all comparison shapes.
- probe: Applied at all 4 sites atop the floor-60 chassis, measured via sandbox --disable all.
- result: Score regressed 60 -> 64 (worse). Reverted to `(D_800A34F8 & 0xF) == arg2`, confirmed back at 60.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c s3 chassis (floor 60), zero FAKE constructs present

## H7 (session 4) — KILLED: the permuter-found `new_var` split of the
final else-arm's two byte stores (record-copy block) does not affect the
honest score
**Statement:** splitting `*(u8*)(arg1+0x2A) = v0; *(u8*)(arg1+0x29) = v0;`
into `s32 new_var = v0; *(u8*)(arg1+0x2A) = new_var; *(u8*)(arg1+0x29) =
new_var;` in the 4th block's else arm (the record-copy block, `arg1+0x28`
region) — found by a directed permuter campaign as part of a combined
mutation (paired with an incorrect `short v0` retype) that dropped the
permuter's own weighted score 530 -> 330.
**Mechanism:** Not a real mechanism — hand-isolating just the `new_var`
split (excluding the incorrect retype, since the pseudo is genuinely
`s32`-width per the target's full-word `lw`s) and re-measuring showed the
permuter's own score improvement was ENTIRELY attributable to the
(excluded) retype, not the split.
**Probe:** Applied the `new_var` split alone to src/text1b.c, measured via
`sandbox --disable all`.
**Result:** Score unchanged (60 -> 60). Reverted immediately.
**verdict:** KILLED
**kill_scope:** instance
**measured_on:** candidate.c s3/s4 chassis (floor 60), single site, zero
FAKE constructs present

## Session 4 permuter campaign — plateau evidence (not a hypothesis, a
process record)
A from-scratch permuter workspace (`tmp/grind/func_8006A564/s4/perm/`,
first campaign ever for this function) ran ~4439 iterations / ~183s
wall / 4 workers before being harvested + stopped. The permuter's own
weighted score plateaued hard at 520 (12 of 25 harvested finds sit at
exactly that value) despite dozens of distinct random mutations tried;
the one outlier drop to 330 was purely a false-positive combination (see
H7) with zero honest-score effect. This is evidence the coloring-swap
residual is NOT reachable by decomp-permuter's default random-mutation
search from this chassis — consistent with the ledger's own note that
"the permuter cannot express... chassis swaps" for a residual whose fix
requires restructuring which C-level pseudo crosses the if/else block
boundary (a semantic/structural rewrite, not a local expression
mutation). Per the CHASSIS RULE, a same-chassis re-seed is not a valid
future probe; the next permuter attempt (1 remaining under the R3
2-session cap) should target a STRUCTURALLY DIFFERENT chassis — e.g. one
of the frontier's untried per-arm-local restructurings — not this same
base.c.

## Frontier for next session
1. **(unchanged, still top priority)** Restructure the arm-value variable
   (`v0`, used inside/after each if/else) so it is NOT itself a single
   pseudo crossing the branch. The `.greg` conflict list this session
   showed the arm-value pseudo (e.g. pseudo 72) DOES conflict with
   hardreg 2 (v0) despite v0's mask-compute use dying at the branch --
   the source of that conflict is still unexplained and needs a
   `.lreg`/liveness-focused read (not yet done) before the next
   structural attempt, since blindly trying "genuinely separate per-arm
   locals" without understanding WHY v0 is already excluded risks
   repeating session 3's H4 dead end (a fresh local name alone does
   nothing -- combine/RA look at value flow, not declaration identity).
2. Diff session-2's build.dis.txt against session-3/4's (both already-built
   binaries, not each against target) to finally isolate exactly which
   instructions the 68->60 win touched -- still not done across 2
   sessions now.
3. The single block3 scheduling tie (unchanged, low priority, revisit
   after coloring is understood).

## [s4] The permuter-found `new_var` split of the final else-arm's two byte stores in the 4th (record-copy) block (`s32 new_var = v0; *(u8*)(arg1+0x2A) = new_var; *(u8*)(arg1+0x29) = new_var;` instead of two direct `v0` stores) improves the honest score.
- mechanism: None confirmed -- the permuter's own weighted score improvement (530 -> 330) for the combined mutation this was extracted from was entirely attributable to an accompanying incorrect `short v0` retype (wrong: the pseudo is genuinely s32-width per target's full-word lw loads), not to the new_var split itself.
- probe: Hand-applied the new_var split alone (excluding the incorrect retype) to src/text1b.c, measured via `sandbox --disable all`.
- result: Score unchanged, 60 -> 60. Reverted immediately; confirmed back at 60.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c s3/s4 chassis (floor 60), single site, zero FAKE constructs present

## Session 5 (enumerate, 2026-09-16)

H6: Systematic spelling_enum.py sweep of block 1's flat tail (4 stores each
    reusing the single block-scoped `v0`), 150 spellings (inline-subset x
    decl-order x commutative-swap), swept via a hand-rolled wteng-loop
    (sweep_variants.py itself is blocked by worktree_contamination_guard.py
    with no wteng-passthrough form for standalone scripts).
    RESULT: CONFIRMED. The fully-inlined spelling (no named locals at all
    in the tail, each value written as an anonymous expression at its one
    use site) is the UNIQUE score-minimizing form: histogram 47(1) 49(7)
    51(14) 52(26) 54(102) out of 150 -- every named-local variant scored
    >= 49, only full inlining hit 47. Applied to block 1: 60 -> 47.
    kill_scope: n/a (CONFIRMED, not KILLED)
    measured_on: candidate.c s4 chassis (floor 60), zero FAKE constructs

H7: Transplanting the SAME full-inline pattern (hand-derived, not
    independently enumerated) to block 2's and block 3's tails (identical
    shape: `v0 = load; store; v0 = load2; ...; v0 = v0+K; store;`).
    RESULT: CONFIRMED but PARTIAL. 47 -> 45. Register-normalized diff shows
    block 1 is now FULLY register-clean (target's v0 everywhere our build
    also uses v0) -- the session-2/3/4 "v0/v1 coloring swap" is CLOSED for
    block 1 specifically; its remaining diff is pure instruction ORDER
    (scheduling), not register choice. Blocks 2/3 only partially fixed:
    the swap FLIPPED (before: both of the block's two loaded values got
    v1/wrong; after: one gets v0/right, the other still v1/wrong) rather
    than fully resolving, unlike block 1.
    kill_scope: n/a (CONFIRMED, not KILLED)
    measured_on: candidate.c s5 chassis (floor 47 -> 45), zero FAKE
    constructs

H8: Block 2's tail as two SEPARATE fresh named locals (`o18`, `o1c`, each
    written once, read once) instead of full inlining -- tests whether a
    named (not anonymous) form still gets the register right while being
    "more natural" C.
    RESULT: KILLED (instance). Score improved 45 -> 42 but build_insns
    dropped from 199 to 194 (broke exact instruction-count parity with
    target -- combine folded something away this spelling shouldn't have
    lost). Reverted; the fully-inlined form is the correct chassis.
    kill_scope: instance
    measured_on: candidate.c s5 chassis (block 1+3 already inlined, block 2
    tested with this form in isolation), zero FAKE constructs present

## [s5] Systematic spelling_enum.py sweep (150 spellings: inline-subset x decl-order x commutative-swap) of block 1's flat post-if/else tail (4 stores each reusing the single block-scoped v0) finds the fully-inlined spelling (no named locals at all, each value written as an anonymous expression at its one use site) as the unique score-minimizing form.
- mechanism: GCC 2.7.2 allocno/pseudo classification differs between a named local that is written then read (gets its own pseudo, subject to local-alloc/global-alloc's hard-register search) and an anonymous rvalue folded directly into its consuming store (no pseudo materializes at all for the pure-constant case; for the load+add case combine folds load+add+store into fewer RTL steps that expose a different, apparently less-conflicted pseudo to global-alloc).
- probe: python3 tools/spelling_enum.py --candidate tmp/grind/func_8006A564/s5/enum_src.c --out tmp/grind/func_8006A564/s5/enum (150 variants); tmp/grind/func_8006A564/s5/run_sweep.ps1 splices each into src/text1b.c and scores via '& tools/wteng.ps1 main sandbox func_8006A564 --disable all'.
- result: Histogram over 150 variants: 47(1) 49(7) 51(14) 52(26) 54(102). Only v149.c (fully inlined) hit 47; every variant retaining any named local for the tail scored >= 49. Applying v149's pattern to block 1: sandbox score 60 -> 47, build_insns stayed 199==199 (target). Register-normalized objdump diff (tmp/grind/func_8006A564/s5/diff2.py) confirms block 1's v0/v1 register choice now matches target EXACTLY throughout; its only remaining diff is instruction ORDER (a scheduling tie), not register allocation.
- verdict: CONFIRMED

## [s5] Transplanting the same full-inline pattern (v0 = load; store; v0 = load2; v0 = v0+K; store; collapsed to store = load; store = const; store = const; store = (load2+K);) to block 2's and block 3's tails, which have the identical surface shape as block 1's.
- mechanism: Same as H6 -- removing the named v0 pseudo from the tail changes which values become distinct pseudos vs fold into their consuming store, changing global-alloc's conflict graph for the block.
- probe: Manual edit of block 2 and block 3 tails in src/text1b.c to the same inlined form as block 1's winning spelling; measured via '& tools/wteng.ps1 main sandbox func_8006A564 --disable all' after each edit.
- result: Score 47 -> 45 (both edits combined). Only a PARTIAL fix for blocks 2/3, unlike block 1's full resolution: before the edit both of the block's two loaded values (the arg1+0x18-based one and the arg1+0x1C-based one) got v1 (both wrong vs target's v0); after the edit the swap FLIPPED -- one now gets v0 (right), the other still gets v1 (wrong) -- rather than both becoming correct. Net improvement of one fewer register diff per block, but blocks 2/3 are not fully closed the way block 1 is. build_insns held at 199==199 throughout.
- verdict: CONFIRMED

## [s5] Block 2's tail written as two SEPARATE fresh named locals (s32 o18; s32 o1c; each written once and read once, in the same source order as the inlined form) instead of full inlining -- tests whether a named-but-fresh spelling also gets the register right while reading more naturally.
- mechanism: If the win were purely about freshness (not about avoiding a named pseudo altogether), two single-use fresh locals should score identically to the fully-inlined form; if the win is about avoiding pseudo materialization entirely, a fresh named local should score worse than inlining but still potentially better than the reused-v0 form.
- probe: Edited block 2's tail to 's32 o18; s32 o1c; o18 = *(s32*)(arg1+0x18); *(s16*)(tile+8)=o18; *(s16*)(tile+0xC)=0x78; *(s16*)(tile+0xE)=1; o1c = *(s32*)(arg1+0x1C)+0xE; *(s16*)(tile+0xA)=o1c;' and re-measured.
- result: Sandbox score DID drop further (45 -> 42) but build_insns broke exact parity (199 -> 194, i.e. 5 fewer instructions than target) -- combine folded away real target-present instructions, meaning this spelling opens a NEW diff elsewhere in the function even though the masked score looks lower. Reverted in favor of the fully-inlined form (candidate.c), which keeps 199==199. Saved to memory/grind/func_8006A564/rejected/block2-separate-locals-breaks-parity.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c session-5 chassis (blocks 1 and 3 already using the fully-inlined tail form; block 2 tested in isolation with the two-fresh-locals form), zero FAKE constructs present

## [s6] Systematic spelling_enum.py sweep (65 spellings: inline-subset x decl-order, --no-swaps) of block 2's OWN flat post-if/else tail (the arg1+0x18 / arg1+0x1C dual-value tail), run INDEPENDENTLY of block 1's transplanted winner as the s5 frontier called for.
- mechanism: block 2's tail has a genuinely different two-value liveness overlap than block 1's (block 1's tail values did not overlap in liveness the same way), so its optimal spelling was hypothesized to potentially differ from block 1's transplanted "fully inline everything" winner.
- probe: ENUM-marked block 2's tail (`s32 valA..valD` fully-named form covering the arg1+0x18 load, the two constants, and the arg1+0x1C+0xE load) in tmp/grind/func_8006A564/s6/enum_block2.c; `python3 tools/spelling_enum.py --candidate ... --out tmp/grind/func_8006A564/s6/enum2 --no-swaps` (65 variants, swaps axis found nothing additional -- `--list` reported the identical count with and without `--swaps`); scored all 65 via tmp/grind/func_8006A564/s6/sweep.py (a pwsh-7-based Python driver -- `pwsh.exe` not legacy `powershell.exe`, whose non-UTF8 default codepage garbles wteng.ps1's unicode glyphs and throws a parser error; see EVIDENCE).
- result: Histogram over 65 variants: 41(1) 42(2) 43(4) 44(21) 45(10) 46(27). Several variants scored BELOW the current 45 baseline (as low as 41), but build_insns for every one of them was < 199 (196, 194, 195, 192, 191 ...) -- i.e. every lower-scoring spelling breaks the exact target_insns==199 parity, the SAME failure mode H8/s5 already identified for block 2's separate-locals form (combine folds something the target doesn't fold, opening a new diff elsewhere that the masked sandbox score doesn't see). Only ONE variant (v64.c, the fully-inlined form -- i.e. the CURRENT candidate.c baseline transplanted from block 1) preserves build_insns==199, and it scores 45, matching the current floor exactly. The already-applied transplant IS the score-minimizing form for this region under the exact-parity constraint.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c session-5/6 chassis (blocks 1 and 3 in their s5 form, block 2's tail swept in isolation across all 65 spellings), zero FAKE constructs present

## [s6] Systematic spelling_enum.py sweep (65 spellings, --no-swaps) of block 3's OWN flat post-if/else tail (the arg1+0x18+0x40 / arg1+0x1C+0xD dual-value tail), independently of block 1's transplanted winner, same method as block 2's sweep this session.
- mechanism: same as the block-2 sweep above -- block 3's tail also has its own two-value liveness shape distinct from block 1's.
- probe: ENUM-marked block 3's tail in tmp/grind/func_8006A564/s6/enum_block3.c; spelling_enum.py --no-swaps (65 variants) written to tmp/grind/func_8006A564/s6/enum3; scored via the same tmp/grind/func_8006A564/s6/sweep.py driver.
- result: Histogram: 40(1) 41(2) 42(2) 43(21) 44(30) 45(9). Same shape as block 2's sweep: several variants score below 45 (down to 40) but ALL of them break build_insns==199 parity (196, 193, 195, 192, 191 ...). Only v64.c (the fully-inlined transplanted form, i.e. the current candidate.c baseline) holds exact parity at score 45.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c session-5/6 chassis (blocks 1 and 2 in their s5/s6 form, block 3's tail swept in isolation across all 65 spellings), zero FAKE constructs present

## [s6] Re-dumped block 1's .sched pass output on the CURRENT (session-5) chassis via `pwsh tools/grinder/dump.ps1 func_8006A564` (the s5-era dump had gone stale in tmp/ scratch) to identify the EXACT rank_for_schedule tie-break named in the s5 frontier as a hypothesis ("dependence-class compare, per [[sched-rank-class-tie-wall]]").
- mechanism: [[sched-rank-class-tie-wall]] predicts that when two ready insns are equal-priority AND are the two inputs of the same nearest successor, GCC falls through to a dependence-CLASS compare. Block 1's tail sits in "basic block number 1 from 32 to 48" of the fresh .sched dump.
- probe: `grep -n "func_8006A564" tmp/grind/func_8006A564/dumps/text1b.sched` -> function starts at dump line 70410; read the basic-block-1 scheduling trace (dump lines ~70436-70452) directly.
- result: The actual dump text is NOT a dependence-class compare. At T-4 the ready list has insn 36 (priority 1) and insn 46 (priority 1) tied; the scheduler's own comment reads `;; insn 36 has a greater potential hazard, now 36 46` -- i.e. cc1's `rank_for_schedule` picked insn 36 first specifically because of a FUNCTION-UNIT / LOAD-LATENCY hazard estimate (`insn_cost`/hazard heuristic in sched.c), not the dependence-class fallback the s5 frontier guessed. This REFINES (does not confirm) the s5 hypothesis: [[sched-rank-class-tie-wall]]'s specific "two inputs of the same nearest successor -> class compare" shape does not appear to be the operative tie-break here; the operative one is the hazard estimate, which is a genuinely different (and less-studied in this ledger) tie-break class.
- verdict: KILLED (the specific dependence-class-compare mechanism named in the s5 frontier, for THIS tie)
- kill_scope: instance
- measured_on: fresh dump.ps1 run against candidate.c's current (unchanged this session) src/text1b.c body, zero FAKE constructs present

## [s6] Block 2's own flat post-if/else tail (arg1+0x18 / arg1+0x1C dual-value store sequence), enumerated independently via spelling_enum.py --no-swaps (65 inline-subset x decl-order spellings), has a spelling that beats the block-1-transplanted form's score of 45 while preserving build_insns==199 exact parity with target.
- mechanism: Different two-value liveness overlap than block 1's tail could plausibly favor a different named-local/inlining pattern for global-alloc's conflict graph.
- probe: ENUM-marked block 2's tail in tmp/grind/func_8006A564/s6/enum_block2.c; python3 tools/spelling_enum.py --no-swaps (65 variants) to tmp/grind/func_8006A564/s6/enum2; scored all 65 via tmp/grind/func_8006A564/s6/sweep.py (splices each into src/text1b.c, calls '& tools/wteng.ps1 main sandbox func_8006A564 --disable all', restores original after).
- result: Histogram: 41(1) 42(2) 43(4) 44(21) 45(10) 46(27). Several variants score below 45 (as low as 41) but every one of them has build_insns < 199 (196/194/195/192/191), breaking exact instruction-count parity with target -- the same failure mode session 5's separate-locals experiment (H8) already found. Only v64.c (the already-applied fully-inlined form transplanted from block 1) holds build_insns==199, scoring 45.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c session-5/6 chassis (blocks 1 and 3 in s5 form, block 2's tail swept in isolation across all 65 spellings), zero FAKE constructs present

## [s6] Block 3's own flat post-if/else tail (arg1+0x18+0x40 / arg1+0x1C+0xD dual-value store sequence), enumerated independently via spelling_enum.py --no-swaps (65 spellings), has a spelling that beats the block-1-transplanted form's score of 45 while preserving build_insns==199 exact parity with target.
- mechanism: Same as block 2's hypothesis above -- block 3's tail also has its own two-value liveness shape distinct from block 1's.
- probe: ENUM-marked block 3's tail in tmp/grind/func_8006A564/s6/enum_block3.c; spelling_enum.py --no-swaps (65 variants) to tmp/grind/func_8006A564/s6/enum3; scored via the same sweep.py driver.
- result: Histogram: 40(1) 41(2) 42(2) 43(21) 44(30) 45(9). Same shape as block 2: variants score as low as 40 but ALL break build_insns==199 parity (191-196). Only v64.c (the current fully-inlined transplanted form) holds exact parity at score 45.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c session-5/6 chassis (blocks 1 and 2 in s5/s6 form, block 3's tail swept in isolation across all 65 spellings), zero FAKE constructs present

## [s6] Block 1's remaining tail residual is a scheduling tie resolved by rank_for_schedule falling through to a dependence-CLASS compare between two equal-priority ready insns that are the two inputs of the same nearest successor, per the [[sched-rank-class-tie-wall]] mechanism the s5 frontier named as the likely explanation.
- mechanism: sched.c rank_for_schedule's documented fallback when priority() ties: compare by dependence class.
- probe: Re-ran 'pwsh tools/grinder/dump.ps1 func_8006A564' (the s5-era .sched dump had gone stale in tmp/ scratch) and read block 1's basic-block-1 scheduling trace directly (dump lines ~70436-70452, function starts at line 70410).
- result: The dump's own comment at the T-4 tie between insn 36 and insn 46 (both priority 1) reads 'insn 36 has a greater potential hazard, now 36 46' -- i.e. the tie was broken by a function-unit/load-latency HAZARD estimate, not a dependence-class compare. The specific mechanism the s5 frontier guessed does not match the dump for this tie.
- verdict: KILLED
- kill_scope: instance
- measured_on: fresh dump.ps1 run against the unchanged (this session) candidate.c body in src/text1b.c, zero FAKE constructs present

## Session 7 (structural, 2026-09-16)

H9: Applying the s5-proven "drop the reused single-scope v0, write inline
    expressions at each single-use site" transplant pattern to block 4
    (the record-copy block, the ONLY tile-draw/record block never given
    this treatment) will drop the floor below 45.
    RESULT: CONFIRMED, in 4 independently-measured increments:
    (a) top-of-block single-use `v0 = *(tile+0x28); *(arg1+0)=v0;`
        inlined to a single store: measured but not isolated in isolation
        first (applied together with (b) below in initial pass) -- see
        evidence.md for the exact per-edit deltas actually measured.
    (b) if-arm's two single-use halving values (load+shift+store each,
        `v0`/`v1`) fully inlined (no named locals): 45 -> 39.
    (c) else-arm's 0x28 literal broadcast (`v0=0x28; 3x store`) inlined to
        3 direct-literal stores: 39 -> 35.
    (d) the two single-use `v0 = *(arg0+0x14); *(arg1+8)=v0;` loads
        (one before each func_8007352C call) inlined directly: 35 -> 29.
    Every increment re-measured via `sandbox --disable all`; build_insns
    held at 199==199 (target) throughout all 4 edits.
    kill_scope: n/a (CONFIRMED, not KILLED)
    measured_on: candidate.c s6 chassis (floor 45) -> s7 chassis (floor 29
    after all 4 edits), zero FAKE constructs present

H10: The multi-use boundary condition from s5/s6 (never inline a value
     used 2+ times -- risks duplicate loads that break exact build_insns
     parity) also holds for block 4's two remaining named-v0 groups.
     Tested two specific instances:
     (a) the `v0 = *(arg1+0)+0xC; ...; *(arg1+4)=v0;` group (preceded by
         two untouched DEAD reads `v0=*(arg1+0);` `v0=*(arg1+0x1C);`)
         inlined to `*(arg1+4) = *(arg1+0)+0xC;`: MEASURED WORSE, 29 -> 34
         (build_insns unchanged, 199==199 -- this is a pure register/order
         regression, not a parity break, unlike (b)).
     (b) the `v0 = *(tile+0x2C); *(arg1+0)=v0; *(arg1+4)=v0+0xC;` group
         (v0 genuinely used twice) fully inlined as two separate
         `*(tile+0x2C)` reads: MEASURED WORSE on build_insns, 199 -> 201
         (duplicate load NOT commoned by CSE across the intervening
         store to arg1+0) -- the same parity-break failure mode s5/s6
         already found for blocks 2/3's multi-use spellings.
     Also tested REORDERING (a)'s three statements (dead-reads before vs
     after the `*(arg1+0x18)=0` store): NO CHANGE either way (stays 29) --
     statement order is not the lever for this group, only the
     inline-vs-not axis, which is already at its measured optimum
     (un-inlined, current candidate.c form).
     verdict: KILLED (both instances)
     kill_scope: instance
     measured_on: candidate.c s7 chassis (floor 29 after H9's 4 wins
     applied), single site each, zero FAKE constructs present

## [s7] Applying the s5-proven "drop the reused single-scope v0, write inline expressions at each single-use site" transplant pattern to block 4 (the record-copy block, the only tile-draw/record block never given this treatment) will drop the floor below 45.
- mechanism: Same as s5/s6: a named local written then read gets its own pseudo subject to local-alloc/global-alloc's hard-register search; an anonymous rvalue folded directly into its consuming store either materializes no pseudo (constant case) or lets combine fold load+use into a shorter RTL sequence exposing a different, less-conflicted pseudo to global-alloc.
- probe: Applied 4 independent single-use inlines (top-of-block *(tile+0x28) load, if-arm's two halving values, else-arm's 0x28 literal broadcast, the two *(arg0+0x14) loads before each func_8007352C call), re-measuring via '& tools/wteng.ps1 main sandbox func_8006A564 --disable all' after each edit.
- result: CONFIRMED across 4 increments: 45 -> 39 -> 35 -> 29. build_insns held at 199==199 (target) after every single edit.
- verdict: CONFIRMED

## [s7] Inlining the *(arg1+0)+0xC group (v0=*(arg1+0)+0xC; ...; *(arg1+4)=v0;) into a single expression, while leaving its two preceding dead reads (v0=*(arg1+0); v0=*(arg1+0x1C);) untouched, will further lower the score.
- mechanism: Hypothesized by analogy to H9's other 4 single-use wins in the same block.
- probe: Applied `*(arg1+4) = (*(arg1+0) + 0xC);` in place of the named v0 assign+store, at the floor-29 chassis, measured via sandbox --disable all.
- result: Score regressed 29 -> 34 (build_insns unchanged at 199==199, so this is a register/scheduling regression, not a parity break). Reverted, reconfirmed 29.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c s7 chassis (floor 29, H9's 4 wins already applied), single site, zero FAKE constructs present

## [s7] Reordering the same group's 3 statements (moving the *(arg1+0x18)=0 store before vs after the two dead reads) will change the score.
- mechanism: Statement order can affect which insn a scheduler/allocator processes first.
- probe: Swapped the store to before the two dead reads, measured via sandbox --disable all; reverted and reconfirmed.
- result: No change (stays 29) in either order -- statement order is not a lever for this group; the inline-vs-not axis is the only one that moves it, and inlining already measured worse.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c s7 chassis (floor 29), zero FAKE constructs present

## [s7] Fully inlining the v0 = *(tile+0x2C); *(arg1+0)=v0; *(arg1+4)=v0+0xC; group (v0 used TWICE) as two separate *(tile+0x2C) reads will lower the score without breaking parity.
- mechanism: Hypothesized that GCC's CSE might common the duplicate load across the single intervening store (which writes a different address, arg1+0, so no aliasing conflict).
- probe: Replaced both uses with direct *(tile+0x2C) reads, measured via sandbox --disable all.
- result: build_insns broke exact parity: 199 -> 201 (2 extra instructions -- CSE did NOT common the duplicate load across the intervening store). Confirms the s5/s6-established boundary condition ("never inline a value used 2+ times") also holds for block 4. Reverted, reconfirmed 199==199 / score 29.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c s7 chassis (floor 29, H9's 4 wins already applied), single site, zero FAKE constructs present

## [s8] 2026-09-16 — solver

Chassis: the working tree's unbanked score-7 body (see evidence.md [s8]);
zero FAKE constructs present in every measurement below; exact insn parity
(build_insns 199 == target_insns 199) held in every variant unless noted.
Sweep harness: `tmp/grind/func_8006A564/s8/{gen,genb,genc,gend,gene}.py`
generate whole-file variants into `s8/variants/`, `s8/sweep.ps1` swaps each into
`src/text1b.c`, runs `sandbox --disable all`, and restores the baseline.

### CONFIRMED

**H-A1 (CONFIRMED, floor 7 -> 3).** Rewriting cluster A's group as
`*(arg1+0x18) = 0; v0 = *(arg1+0); v0 += 0xC; *(arg1+0x1C) += 0xF;
*(arg1+4) = v0;` closes all four of cluster A's mismatched instructions.
Mechanism: the compound-assignment split plus the hoisted constant store changes
the sched1 LUID/readiness order of the two load->add->store chains, so the
arg1+0 chain is emitted first exactly as in the target. Measured 7 -> 3, exact
parity, on the chassis above with zero FAKE constructs. A second form (a10 —
dead reads retained, arg1+0x18 store moved to the END of the group) also
measures 3; the banked candidate uses the dead-read-free form.

**H-A2 (CONFIRMED).** The two dead reads carried since s2
(`v0 = *(arg1+0); v0 = *(arg1+0x1C);`) are NOT load-bearing: a02/a03 (both dead
reads deleted) measure 7 on the score-7 chassis, identical to the control a00/a01
(dead reads present, either order). The candidate therefore drops them, leaving
the body with no dead stores at all.

### KILLED (all INSTANCE, all measured on the s8 chassis, zero FAKE constructs)

**H-B1.** Naming cluster B's `+0xC` value in a fresh block-scope local
(`v0 = *(tile+0x2C); v1 = v0 + 0xC; *(arg1+0) = v0; *(arg1+4) = v1;`) does not
move cluster B's 3-instruction residual: measured 3 (unchanged), exact parity.
Also measured unchanged at 3: a same-variable compound split (`*(arg1+0) = v0;
v0 += 0xC;`), a copy-then-compound (`v1 = v0; v1 += 0xC;`), a re-read from
memory (`*(arg1+4) = *(arg1+0) + 0xC;`), the a11-shaped variant, and
reversed-declaration `v1`/`v0` naming. 8 variants (b00..b07), all 3.
Mechanism (dump-read, not inferred): `.rtl`/`.combine` already emit the add
before the store for these spellings; sched1 moves it, because the add is
released with `LAUNCH_PRIORITY` (`sched.c:187`, set `sched.c:4049`) when its
consumer is scheduled and therefore wins `rank_for_schedule`'s first test
(`sched.c:2418`) against the equal-class store.

**H-B2.** Moving cluster B's `*(arg1+8) = *(arg0+0x14);` store relative to the
two record stores measures strictly worse AND breaks exact parity: ahead of both
stores (c01, c08) = score 8 / build_insns 197; between them (c02, c06) = score 6
/ build_insns 198. cse commons the `arg0+0x14` read across the moved store.

**H-B3.** Transplanting the matched sibling func_8006A1A0's spelling of the
identical shape (`p2 = tile[0x2C]; tbl = p2 + 0xC; dst0 = p2; dst4 = tbl;`) onto
this chassis does not reproduce the sibling's instruction order: function-scope
`tbl`, function-scope `p2 + tbl`, `tbl`/`p2` declared first, block-scope pair,
and an `s32 *rec` view of `tile` — 5 variants (d01..d05), all measured 3, exact
parity. The sibling's destination is a stack struct at a fixed frame address;
ours is a pointer parameter, so the store's alias/dependence position differs.

**H-B4.** Giving cluster B's destination stores `MEM_IN_STRUCT_P` shape (a `s32
*dst = (s32 *)arg1;` local with `dst[0]`/`dst[1]`, an inline `((s32 *)arg1)[i]`
index, a `u8 *d = arg1;` re-base, and a combined dst-pointer + named-`v1` form)
does not move cluster B: 5 variants (e01..e05), all 3, exact parity. Extending
the same pointer view to the third store (`dst[2]`, c05) measures 4 — worse.

### Frontier for s9 (cluster B, 3 insns, the entire remaining residual)

1. The sched1 LAUNCH_PRIORITY release is the named mechanism. The add is boosted
   *because its only consumer is the very insn just scheduled*. Untried axis: a
   second, later consumer of the `+0xC` value (one that appears in the target's
   own bytes — e.g. if the value stored at arg1+4 is genuinely re-read further
   down), or a spelling in which the arg1+0 store is itself released in the same
   cycle so both carry the boost and the LUID tie-break (`sched.c:2463`,
   higher-LUID-first) decides. Read `schedule_insn` (`sched.c:3650-3880`) to see
   exactly which released insns inherit the boost before spelling more variants.
2. Nothing in the local spelling space of that 4-statement group is left
   untried (28 measured spellings across b/c/d/e sweeps, all 3 or worse); the
   next lever has to change the DEPENDENCE STRUCTURE around the group, not its
   text. Candidates: what the second `func_8007352C` call's argument is, and
   whether `tile` (the `obj2->0x1C` record pointer) is re-derived rather than
   held live across the first call.
3. Blocks 1-3 and cluster A now match instruction-for-instruction; the score-3
   residual is entirely cluster B. Any future edit outside that group is a
   regression risk, not an opportunity — re-run `s8/objdiff.sh` after any change.

## [s8] Rewriting the final block's first record group as `*(arg1+0x18)=0; v0=*(arg1+0); v0+=0xC; *(arg1+0x1C)+=0xF; *(arg1+4)=v0;` closes all four mismatched instructions of residual cluster A (the two-load / two-add ordering pair).
- mechanism: The compound-assignment split plus hoisting the constant store to the head of the group changes the sched1 readiness/LUID order of the two load->add->store chains, so the arg1+0 chain is emitted first, as in the target. Ordinary C per ordinary-c-judge-decidable.md Ruling 4.
- probe: 12-variant sweep of that group (tmp/grind/func_8006A564/s8/gen.py + sweep.ps1), each measured with `sandbox func_8006A564 --disable all`.
- result: score 7 -> 3, target_insns 199 == build_insns 199 throughout. Two variants reach 3 (a10 keeps the dead reads and moves the arg1+0x18 store to the end; a11, adopted, deletes them). The register-normalized objdump diff confirms cluster A is now instruction-identical to target; the only remaining mismatches are cluster B's 3 insns (tmp/grind/func_8006A564/s8/final_objdiff.txt).
- verdict: CONFIRMED

## [s8] The two dead reads carried in this body since session 2 (`v0 = *(arg1+0); v0 = *(arg1+0x1C);`) do not affect the emitted bytes on this chassis.
- mechanism: Both are CSE'd into the real loads that follow, so deleting them leaves the same RTL. Removing them makes the candidate free of dead stores, pads and FAKE constructs entirely.
- probe: Variants a02/a03 (both dead reads deleted) vs a00/a01 (present, in both orders), measured on the score-7 chassis.
- result: all four measure score 7 with exact parity 199==199; the adopted score-3 form (a11) carries no dead read at all.
- verdict: CONFIRMED

## [s8] Naming or re-spelling the +0xC value inside cluster B's group (fresh block-scope local, same-variable compound split, copy-then-compound, memory re-read, reversed declaration order, pointer-local destination) leaves cluster B's 3-instruction residual at score 3 on this chassis.
- mechanism: Dump-read: .rtl/.combine already emit the add before the store for these spellings (insn chain 436 lw, 439 add, 442 sw 0(s1), 445 sw 4(s1)); sched1 moves it because the add is released carrying LAUNCH_PRIORITY (tools/gcc-2.7.2/sched.c:187, assigned at sched.c:4049) when its consumer is scheduled, so rank_for_schedule's first test (INSN_PRIORITY, sched.c:2418) picks it and the class/LUID tests are never reached. schedule_block is backward, so the pick places the add after the store.
- probe: 8-variant sweep b00..b07 (tmp/grind/func_8006A564/s8/genb.py) plus a fresh `pwsh tools/grinder/dump.ps1 func_8006A564` whose .rtl/.combine/.sched/.sched2 traces for block 12 were read directly.
- result: all 8 variants measure score 3 with build_insns 199; sched2 shows the region dependence-forced after coalescing (every ready list single-membered).
- verdict: KILLED
- kill_scope: instance
- measured_on: s8 chassis (candidate.c as banked this session, floor 3, cluster A closed), zero FAKE constructs present

## [s8] Moving cluster B's `*(arg1+8) = *(arg0+0x14);` store ahead of or between the two record stores measures worse than the control on this chassis.
- mechanism: cse commons the arg0+0x14 read across the relocated store, dropping real instructions and breaking exact parity.
- probe: Variants c01/c08 (store first) and c02/c06 (store between) from tmp/grind/func_8006A564/s8/genc.py.
- result: c01/c08 score 8 with build_insns 197; c02/c06 score 6 with build_insns 198; control 3 with 199. Rejected form saved to memory/grind/func_8006A564/rejected/blockB-arg8-store-hoisted-breaks-parity.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: s8 chassis (floor 3), zero FAKE constructs present

## [s8] Transplanting the matched sibling func_8006A1A0's spelling of the same load/+0xC/two-store shape (function-scope tbl, function-scope p2+tbl, tbl-declared-first, block-scope pair, s32* view of the record) onto this chassis leaves cluster B at score 3.
- mechanism: func_8006A1A0 emits the target shape (lw v0,12(s3); addiu v1,v0,12; sw v0,24(sp); sw v1,28(sp)) from that C, but its destination is a stack struct at a fixed frame address while ours is a store through the pointer parameter arg1, so the store's dependence position in sched1 differs.
- probe: 5-variant sibling transplant sweep d01..d05 (tmp/grind/func_8006A564/s8/gend.py), plus an objdump read of func_8006A1A0 in build/src/text1b.o to establish the sibling's emitted shape.
- result: all 5 measure score 3 with build_insns 199 and an identical objdiff residual.
- verdict: KILLED
- kill_scope: instance
- measured_on: s8 chassis (floor 3), zero FAKE constructs present

## [s8] Giving cluster B's destination stores a MEM_IN_STRUCT_P shape (s32* local with dst[0]/dst[1], inline ((s32*)arg1)[i] indexing, u8* re-base, dst-pointer combined with a named +0xC local) leaves cluster B at score 3 on this chassis.
- mechanism: INDIRECT_REF over PLUS sets MEM_IN_STRUCT_P and changes the alias exemption at sched.c:834-839, but the add's LAUNCH_PRIORITY release is unaffected, so the same pick order results.
- probe: 5-variant sweep e01..e05 (tmp/grind/func_8006A564/s8/gene.py); the extended form applying the pointer view to the third store (c05) was measured separately.
- result: e01..e05 all score 3 with build_insns 199; c05 (pointer view extended to dst[2]) measured 4, i.e. worse.
- verdict: KILLED
- kill_scope: instance
- measured_on: s8 chassis (floor 3), zero FAKE constructs present
