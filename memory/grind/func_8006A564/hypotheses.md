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
