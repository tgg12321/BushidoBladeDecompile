# hypotheses — func_8006CCC8 (src/text1b.c)

## H1 (CONFIRMED, s1) — inline field-access dereference beats a cached pointer local
Statement: writing the `D_800A34FC`-offset field access as a fresh
`*(s16 *)((u8 *)D_800A34FC + mask + 0x28)` expression at each of the ~5
syntactic use sites (no intermediate pointer/value local) reproduces
target's frame size (0x48) and register allocation (in particular keeps
`ret` in a register instead of spilling), where a cached `s16 *slot` local
computed once per loop iteration does not.
Mechanism: register pressure — an extra live pseudo for the cached pointer
consumes a callee-save slot that target's allocation doesn't need (target
never keeps this address live across statements; it recomputes it from
`D_800A34FC` + the loop-carried byte offset at each load site, in 3
different basic blocks).
Probe: sandbox --disable all on both variants in the same session.
Result: cached-pointer variant scored 142 (frame 0x58, `ret` spilled to
stack); inline-dereference variant scored 94 (frame 0x48, matches target).
Verdict: CONFIRMED
kill_scope: n/a (this is the accepted lever, not a kill)
measured_on: chassis = src/text1b.c HEAD as edited this session (s1),
no FAKE constructs present in either variant (both are ordinary C).

## H2 (frontier, not yet probed) — arg2 sign-extension is LICM-hoisted; needs defeat-licm-hoist-var-reuse
Statement: our build hoists the full 16-bit sign-extension of the third
parameter (`arg2`) out of the `for (i...)` loop into its own register,
computed once before the loop; target computes it fresh every iteration
(3 insns: sll/sra/srav) while keeping only the RAW parameter value live in
`$s7` across iterations.
Mechanism (named GCC pass, per .claude/rules/defeat-licm-hoist-var-reuse.md):
loop.c's `move_movables` — `(s32)arg2`'s sign-extension is a single-set,
non-trapping, loop-invariant pseudo in a short loop containing a call, so
it is unconditionally hoisted regardless of C statement placement. The
documented pure-C defeat is to make that pseudo's register MULTI-SET by
routing an already-used loop-variant value through the SAME C variable
non-consecutively with the invariant assignment — but target's `$s7`
appears to hold ONLY the raw `arg2` value for the whole function (no other
observed use), so the standard recipe's "reuse the register the target
reuses" step needs a fresh read of the instrumented cc1 `.loop`/`.greg`
dump (`pwsh tools/grinder/dump.ps1 func_8006CCC8`) to identify which
pseudo/register actually gets reused in target (if any — it's also
possible target simply never triggers the hoist because its `$s7` pseudo
is used ONLY inside the loop and thus isn't a movable in the same sense;
that alternative explanation has NOT been ruled out and should be checked
before assuming the var-reuse recipe applies verbatim).
Probe: NOT YET RUN this session — establishing H1 and reaching floor 94
consumed the session's turn budget. Next session's probe: read the
`.loop` dump for both the invariant-hoisted region and confirm via
`BB2_ALLOC_DEBUG`/`.greg` whether `$s7`'s pseudo is single-set or
multi-set in our current build, then try reusing whichever loop-variant
value (if any) target's own asm shows sharing a register with the
sign-extension.
Result: (unset — not probed)
Verdict: (unset)

## Frontier for s2 (<=3, mechanism-grounded)

1. **H2 above** — read `.loop`/`.greg` dumps via
   `pwsh tools/grinder/dump.ps1 func_8006CCC8`, confirm the exact
   loop.c mechanism and target's register-reuse (if any), then measure a
   defeat-licm-hoist-var-reuse-shaped fix. This is the highest-value next
   step: floor is currently 94, and this is the only structural
   difference identified so far in the disassembly.
2. After H2 lands (or is killed), re-diff the full disassembly end-to-end
   (a clean objdump-vs-target instruction walk was only done up through
   the loop header + the D_800A3524 inner-loop block this session; the
   tail of the function — the `field28>=4`/`==4` arm and the trailing
   `func_8006CBD4` call arm — has NOT been individually verified
   instruction-for-instruction yet, only inferred from the m2c structure
   matching the first two arms).
3. If H2's register-reuse recipe doesn't apply cleanly (target's `$s7`
   truly has no second use), consider whether the loop trip-count-2
   `for` should instead be a manually-unrolled pair of statements (i=0
   body; i=1 body) — GCC's `move_movables` profitability heuristic could
   behave differently on a loop-free structure, though this needs
   confirmation the target's asm even IS a compiled loop (it is — the
   `.L8006CD54` label and `bnez ... .L8006CD54` back-edge with `slti
   v0,s1,2` trip test are genuine loop control flow, not unrolled), so
   this fallback is lower priority than #1.

## [s1] Writing the D_800A34FC-offset field access as a fresh *(s16*)((u8*)D_800A34FC+mask+0x28) expression at each syntactic use site (no cached pointer local) reproduces target's frame size (0x48) and keeps `ret` register-allocated, where a cached s16* pointer local computed once per loop iteration causes register pressure that spills `ret` to the stack and grows the frame to 0x58.
- mechanism: global.c register allocation pressure — the extra live pseudo for the cached pointer consumes a callee-save slot target's allocation never needs, since target recomputes the address from D_800A34FC + the loop-carried byte offset independently in 3 different basic blocks rather than caching it.
- probe: sandbox --disable all on both variants (cached-pointer vs inline-dereference) in the same session, compared via objdump of the emitted .o.
- result: cached-pointer variant: score 142, frame sp,-0x58, `ret` spilled (sw/lw through 16(sp)). Inline-dereference variant: score 94, frame sp,-0x48 matching target, `ret` register-allocated ($s8/$fp).
- verdict: CONFIRMED

## [s1] A cached s16 *slot pointer local for the D_800A34FC field, reused across the ~5 accesses within one loop iteration, is NOT the shape of the original source for this chassis (it measurably regresses the score and frame vs the inline-dereference form).
- mechanism: global.c register allocation — see CONFIRMED entry above; the cached-pointer pseudo raises register pressure past what fits in the same 9 callee-save registers target's allocation uses, bumping `ret` out to a stack spill.
- probe: sandbox --disable all on src/text1b.c with the cached-pointer variant applied, this session.
- result: score 142 (vs 94 for the accepted alternative measured in the same session, same chassis), frame grew from target's 0x48 to 0x58.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD as edited this session (s1); no FAKE construct present in either variant (both are ordinary C, no cheat-catalog constructs).

## H2 (s2, CONFIRMED) — defeat-licm-hoist-var-reuse closes the arg2 sign-extension hoist
Statement: the s1 gap (our build hoisting `(s32)arg2`'s sign-extension out
of the `for (i...)` loop into a callee-save, where target recomputes it
inline every iteration via `sll/sra/srav`) is fixed by making the
sign-extension's pseudo MULTI-SET, per [[defeat-licm-hoist-var-reuse]]:
added `s32 t;`, set `t = arg2;` at the top of the loop (feeds `lim`), and
non-consecutively later in the SAME loop body (the `field28>=0` tail arm)
set `t = i;` and pass `t` (not `i` directly) to `func_8006CBD4`. The second
set is a REAL, USED value (the already-live loop-variant `i`, read
immediately by the call) — not a fabricated/dead write.
Mechanism (named GCC pass, confirmed via dump not inferred): loop.c
`move_movables` requires `n_times_set[regno] == 1` (loop.c:705) for a
pseudo to be admitted as a movable in `scan_loop`. Before the fix, cc1's
RTL expand phase allocated ONE pseudo per C variable and the sign-extension
of `arg2` had a single textual assignment point → single-set → movable →
hoisted (confirmed via `tmp/grind/func_8006CCC8/dumps/text1b.rtl` insns
71/72, `(set (reg:SI 95) (ashift (subreg (reg/v:HI 74) 0) 16))` /
`(set (reg:SI 94) (ashiftrt (reg:SI 95) 16))` with a `REG_EQUAL
(sign_extend:SI (reg/v:HI 74))` note, and `tmp/grind/func_8006CCC8/dumps/text1b.loop`
"Loop from 59 to 557: 155 real insns. ... Insn 71: regno 95 ... savings 2
moved to 604. Insn 72: regno 94 ... cond forces 71 savings 1 moved to
605"). After the fix, `t`'s pseudo has TWO textual SET points in the loop
body (the `t = arg2;` at top and the `t = i;` in the tail arm), so
`n_times_set > 1` and loop.c never admits it as a movable — the
ashift/ashiftrt sequence stays inline at the loop top exactly where target
has it (confirmed post-fix in `tmp/grind/func_8006CCC8/dumps/text1b.s`
`.L997: sll $2,$23,16 / sra $2,$2,16 / sra $2,$2,$17` — compare target
`asm/funcs/func_8006CCC8.s:39-41` `.L8006CD54: sll $v0,$s7,16 / sra
$v0,$v0,16 / srav $v0,$v0,$s1`, structurally identical, inside the loop).
Probe: sandbox --disable all before/after, same session.
Result: floor 94 -> 91 (this lever alone).
Verdict: CONFIRMED
kill_scope: n/a (accepted lever)
measured_on: src/text1b.c HEAD as edited this session (s2); no FAKE
construct — `t` is a fresh local holding two genuinely different, both-USED
values (arg2's promoted value; i's promoted value), which is the
[[defeat-licm-hoist-var-reuse]] / [[no-new-park-categories]] "variable
reuse for codegen control" SOTN-accepted family (own frozen entry, not a
last-resort FAKE-gated family — no annotation required per that entry's
own text and the SOTN `idxSub = idxSub;` / `randy = ...; baseX = randy;`
precedent it cites).

## H3 (s2, CONFIRMED) — `i` is a plain 32-bit int in the original source, not s16
Statement: widening the loop counter `i` from `s16` to `s32` (leaving every
other type unchanged) drops the floor from 91 to 77 and brings build_insns
from 193 to 185 — UNDER target's 189, meaning the s16 declaration was
forcing extra sign-extension instructions our build emitted that target's
does not.
Mechanism: target's asm never sign/zero-extends its `i`-holding register
($s1) anywhere in its lifetime — it is used directly as a 32-bit shift
amount (`srav $v0,$v0,$s1`, func_8006CCC8.s:41) and passed directly as a
call argument with a bare register-to-register move
(`addu $a0,$s1,$zero`, func_8006CCC8.s:117) — no `sll/sra` pair anywhere
near either use. A `s16 i` would require cc1 to insert exactly such a pair
whenever `i`'s HImode value needs to appear in a full-width SImode context
(e.g. the `t = i;` promotion, or the shift-amount operand in stricter
cases) — the s16 declaration was producing that avoidable pair at the
`func_8006CBD4(t, *arg1)` call site pre-fix (dump confirmed:
`tmp/grind/func_8006CCC8/dumps/text1b.s:16832-16836`,
`sll $4,$18,16 / jal func_8006CBD4 / sra $4,$4,16` with `s16 i`, vs a bare
move after widening to `s32 i`). `lim` was also tried as `s32` in the same
session (neutral, score unchanged) and reverted to the m2c-reconstructed
`s16` since there is no measured reason to deviate there.
Probe: sandbox --disable all, `i` as s16 vs s32, same session, same H2 fix
applied in both.
Result: s16 → 91 (H2 lever alone); s32 → 77.
Verdict: CONFIRMED
kill_scope: n/a (accepted lever)
measured_on: src/text1b.c HEAD as edited this session (s2); no FAKE
construct — this is an ordinary type correction (narrow-type mismatch
fix), not a coercion; both `i` and `t` hold real, semantically-consumed
values throughout.

## H4 (frontier, not yet probed) — inner `for (j...)` record-update loop structural mismatch
Statement: at floor 77 (187 insns differing per `diagnose`), the two
`D_800A3524` record-update arms (`field28==3` and `field28==4`, the
`for (j=0;j<3;j++)` loops) do not yet match target's instruction shape.
Target (`asm/funcs/func_8006CCC8.s:130-160`, loop `.L8006CEAC`)
unconditionally loads BOTH `+0x17` and `+0x1A`/`+0x1D` every iteration and
selects the i==0-vs-else half via which register feeds the AND (the mask
`0xF << fade` is precomputed ONCE before the loop via `sllv`, matching our
build's `sll $6,$22,$21` fold — that part already matches). Our build
(`tmp/grind/func_8006CCC8/dumps/text1b.s:16694-16720`) instead branches on
`i == 0` at the TOP of the loop body into two near-duplicate
lbu/andi/lbu/andi sequences (masks 0xF0 vs 0x0F applied to DIFFERENT loads
each arm) that converge on a shared `.L1030` combine+store tail — same
final byte effect, different insn count/shape (extra branch + duplicated
loads).
Mechanism: NOT YET ATTRIBUTED to a specific pass this session — candidate
guesses (unverified): (a) cse.c common-subexpression sharing of the two
lbu loads across the i==0 branch in target (both loads always execute,
letting the AND source register alone select the byte) is a `[[split-read-defeats-hoist]]`-flavored
inversion — i.e. target may NOT branch on `i==0` inside the C at all, and
instead select via a ternary/arithmetic expression on the MASK
(`fade`/nibble side) rather than an `if`; or (b) the current C's `if (i==0)
{...} else {...}` duplicates the FULL store statement per
[[duplicated-statement-into-arms]] when a single expression with a
computed nibble-select (e.g. `(i == 0) ? 0xF0 : 0x0F`) would let GCC fold
to ONE load+mask+store sequence matching target's unconditional-load
shape.
Probe: NOT YET RUN — read `.cse`/`.combine` dumps for this loop's insn
range (grep `tmp/grind/func_8006CCC8/dumps/text1b.cse` /
`.combine` for the `D_800A3524`-loop insn numbers, cross-referenced against
`.rtl`) before attempting a restructure by hand; also re-read
`asm/funcs/func_8006CCC8.s` lines 160-200 (not yet transcribed this
session) to confirm the SECOND occurrence of this loop (the field28==4 arm,
0x1D offset) has the identical shape before assuming both arms need the
same fix.

## H4 (s3, PARTIALLY CONFIRMED — structural match achieved, score unchanged) — hoisting the two record-byte loads before the i==0 branch reproduces target's unconditional-load-then-select-mask shape, but does NOT move the sandbox score
Statement: restructuring the inner `for (j...)` body from
`if (i==0) { *(rec+0x17) = (*(rec+0x17)&0xF0) + (*(rec+0x1A)&mask); } else { ... &0xF ... }`
(which recomputes BOTH `*(rec+0x17)` and `*(rec+0x1A)` inside EACH arm) to
hoisted-load form —
```
masked = *(rec + 0x1A) & (0xF << fade);
byte17 = *(rec + 0x17);
if (i == 0) { *(rec+0x17) = (byte17 & 0xF0) + masked; }
else        { *(rec+0x17) = (byte17 & 0xF)  + masked; }
```
— makes the emitted asm structurally match target's own shape
insn-for-insn: target (`asm/funcs/func_8006CCC8.s:130-146`, `.L8006CEAC`)
loads `+0x1A` then `+0x1A`-masked into `a0` UNCONDITIONALLY, then loads
`+0x17` into `v0` UNCONDITIONALLY, THEN branches on `$s1` (our `i`) to
select which nibble-mask (`0xF0` vs `0xF`) applies to `v0`, before adding
`a0` and storing. Post-fix, our build
(`tmp/grind/func_8006CCC8/dumps/text1b.s:16694-16721`) does exactly this:
one `lw`+`addu` for the record base, `lbu 26(rec)` (the 0x1A byte),
`lbu 23(rec)` (the 0x17 byte), THEN `bne $17,0,...` selecting `andi
0xF0`/`andi 0xF` on the 0x17 value, `and` for the mask (in the branch
delay slot instead of before the branch — the one remaining difference,
a scheduling placement not a structural one), then combine+store. Applied
identically to the `field28==4` arm (`+0x1D` field).
Mechanism: ordinary C restructuring — no GCC-pass coercion. The prior
form's `if/else` duplicated BOTH field reads into each arm, so cc1's
per-block cse1 (whose scope is the basic block) could not share the reads
across the branch; hoisting the reads into two named locals BEFORE the
branch gives cc1 exactly one read of each field, matching target's own
(apparently identical) source shape.
Probe: `sandbox --disable all` before/after, applied to BOTH the
field28==3 and field28==4 arms, same session. Also side-probed swapping
the two hoisted reads' declaration order (`byte17` first vs `masked`
first) — no score change either way (scheduler-order-neutral here);
reverted to `masked` first (matches target's own 0x1A-before-0x17 read
order, so it's the more faithful spelling even though neutral).
Result: `build_insns` dropped 185 -> 183 (2 duplicate `lbu`s removed,
matching target's load count exactly for this region) but the sandbox
LEVENSHTEIN score stayed EXACTLY 77 both before and after. Hand-walking
the resulting disassembly against target line-by-line for this loop (both
the field28==3 and field28==4 arms) and the function's tail
(`asm/funcs/func_8006CCC8.s:160-209`, the epilogue + outer-loop
increment) shows NO remaining structural mismatch in those regions —
register roles match target's exactly (our raw register numbers $20/$23/
$22/$21/$19/$18/$17/$16/$fp/$31 ARE target's $s4/$s7/$s6/$s5/$s3/$s2/$s1/
$s0/$fp/$ra numerically, not just by role), and the prologue reg-save
order + interleaved `lui`/`lw`/`ori` sequence at function entry also
matches target instruction-for-instruction once macro pseudo-ops are
accounted for. The unchanged score therefore means the remaining 77-worth
of edit-distance is NOT in the loop-body/tail regions walked this
session — it must be in a region not yet hand-verified (candidates: the
`field28<=0`/`>=lim` increment-decrement arm bodies at the top of the
outer loop, lines ~55-95 of `asm/funcs/func_8006CCC8.s`, which the s2
evidence entry claimed "matched structurally" but was NOT verified via
the objdump-level scorer, only by eye) or is a genuine scheduling-order
difference (e.g. the `and`-in-delay-slot placement noted above) whose
score weight this session could not isolate (the `engine.score` levenshtein
diff tool requires WSL objdump and could not be run standalone from this
Windows-side session without the worktree_contamination_guard blocking a
bare `from engine import` — see artifacts).
Verdict: KILLED (the specific claim "hoisting the two loads alone closes
H4's remaining gap" is false — the restructure is real and matches target
shape in the walked regions, but leaves the score unchanged)
kill_scope: instance
measured_on: src/text1b.c HEAD as edited this session (s3); no FAKE
construct — `masked` and `byte17` are fresh locals each holding one real,
once-read consumed value (satisfies the named-intermediate SOTN family
even under its stricter historical prong, though this doesn't need that
family's sanction since it's a completely ordinary read-then-branch
restructuring, not a no-semantic-purpose device).

## Frontier for s4 (<=3, mechanism-grounded)

1. **Run the engine's own `engine.score.normalized_insns` levenshtein
   diff for func_8006CCC8 between `build/src/text1b.o` (reference) and
   the sandbox's `tmp/sandbox/func_8006CCC8/text1b.o` (built) INSIDE WSL**
   (this session could only attempt it from the Windows-side Bash tool,
   which `tools/hooks/worktree_contamination_guard.py` blocks for any bare
   `from engine import` — needs `wsl bash -c 'source .venv/bin/activate &&
   python3 -c "..."'` or a proper `.py` file run via the documented WSL
   path). This gives the EXACT opcode-level diff ops (replace/insert/delete)
   instead of eyeballing objdump, and will show definitively where the
   77-worth of edit distance actually sits — the s3 hand-walk covered the
   inner j-loop (both field28==3/4 arms) and the function's tail/epilogue
   and found no visible mismatch there, so the residual is most likely in
   the two increment/decrement arms at the top of the outer `for(i...)`
   loop (`*arg1 & (0x1000<<shift)` / `*arg1 & (0x4000<<shift)`,
   `asm/funcs/func_8006CCC8.s` roughly lines 55-95) which s2's evidence
   entry claimed "matched structurally" WITHOUT this objdump-level
   verification — re-verify that claim first with the diff tool once it's
   runnable.
2. If the diff tool identifies the increment/decrement arms as the actual
   residual, hand-walk `asm/funcs/func_8006CCC8.s:55-95` against
   `tmp/grind/func_8006CCC8/dumps/text1b.s:16558-16660` instruction-for-
   instruction the way s3 did for the j-loop.
3. If the diff isolates the `and`-in-delay-slot placement noted in H4
   (s3) as a real scored difference (not just a display artifact), that
   is a pure scheduling-order question — read `.sched`/`.sched2` dumps for
   the `.L1012`/`.L8006CEAC` loop's insn range before attempting any
   further C-level reorder (per PASS ATTRIBUTION discipline — do not
   guess).

## [s2] Making the (s32)arg2 sign-extension pseudo multi-set (adding `s32 t;` set from `arg2` at the loop top and, non-consecutively, from the already-used loop-variant `i` in the tail arm before it feeds func_8006CBD4) removes it from loop.c scan_loop's movable set, so it is no longer hoisted out of the for(i...) loop and instead recomputes inline every iteration matching target.
- mechanism: loop.c move_movables requires n_times_set[regno]==1 (loop.c:705) to admit a pseudo as a movable in scan_loop; giving the sign-extension's pseudo a second textual SET point in the loop body defeats that admission test.
- probe: sandbox --disable all before/after; cross-checked tmp/grind/func_8006CCC8/dumps/text1b.rtl (insns 71/72 SET reg95/reg94 with REG_EQUAL sign_extend note) and .loop dump (Insn 71/72 ... moved to 604/605) before the fix, and tmp/grind/func_8006CCC8/dumps/text1b.s post-fix showing the sll/sra/srav sequence relocated inside .L997 (the loop top) matching asm/funcs/func_8006CCC8.s .L8006CD54.
- result: floor 94 -> 91
- verdict: CONFIRMED

## [s2] The loop counter `i` is a plain 32-bit int in the original source, not s16 — widening its C declaration from s16 to s32 (with the H2 fix already applied, no other change) drops the floor from 91 to 77 and takes build_insns from 193 to 185, under target's 189 insns for the first time this ledger.
- mechanism: target's $s1 (the register holding i) is used directly as a 32-bit srav shift-amount operand (asm/funcs/func_8006CCC8.s:41) and passed to func_8006CBD4 with a bare addu move (asm/funcs/func_8006CCC8.s:117) with no sign/zero-extension anywhere in its lifetime; declaring i as s16 forced cc1 to insert an avoidable sll/sra promotion pair (confirmed pre-fix at tmp/grind/func_8006CCC8/dumps/text1b.s:16832-16836) whenever i's value needed to appear in a full-width SImode context (here, the t=i; assignment feeding the call).
- probe: sandbox --disable all, i declared s16 vs s32, same session, H2 fix held constant in both variants. Also side-probed lim (s16->s32): no score change, reverted to s16.
- result: s16 -> 91 (H2 alone); s32 -> 77
- verdict: CONFIRMED

## [s2] A cached s16 *slot pointer local for the D_800A34FC field, reused across the ~5 accesses within one loop iteration, is not the shape of the original source for this chassis.
- mechanism: global.c register allocation pressure — the extra live pseudo for the cached pointer consumes a callee-save slot target's allocation never needs, since target recomputes the address independently in 3 different basic blocks rather than caching it.
- probe: sandbox --disable all on src/text1b.c with the cached-pointer variant re-applied this session as part of chassis re-confirmation (banked from s1, not re-derived).
- result: score 142 (frame sp,-0x58, ret spilled) vs 94/91/77 for the accepted alternatives measured across s1/s2 on the same chassis
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD as edited this session (s2), re-confirming the s1 finding; no FAKE construct present in either variant (both are ordinary C, no cheat-catalog constructs).

## [s3] Hoisting the *(rec+0x1A)&mask (and +0x1D for the field28==4 arm) and *(rec+0x17) reads out of the if(i==0){...}else{...} arms into two fresh locals (masked, byte17) read once before the branch, so the branch only selects the nibble mask, reproduces target's unconditional-both-loads-then-select-by-branch shape for the inner j-loop on this chassis and lowers the sandbox score below 77.
- mechanism: ordinary C restructuring -- the prior form duplicated both field reads into each if/else arm, so cc1's per-basic-block cse1 could not share them across the branch; hoisting to two named locals before the branch gives cc1 exactly one read of each field, matching target's apparent source shape. No specific GCC pass coercion invoked.
- probe: sandbox --disable all before/after the restructure, applied to both the field28==3 and field28==4 arms, same session; re-dumped and hand-walked tmp/grind/func_8006CCC8/dumps/text1b.s:16694-16728 against asm/funcs/func_8006CCC8.s:130-160 instruction-for-instruction.
- result: build_insns dropped 185 -> 183 and the walked disassembly now matches target insn-for-insn (including exact numeric register identity, not just role-equivalence), but the sandbox LEVENSHTEIN score stayed EXACTLY 77 before and after. The specific claim that this fix alone would close (or measurably lower) the remaining gap is therefore false, even though the restructure is real, correct, and matches target's observed shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD as edited this session (s3), H2+H3 fixes from s2 held constant; no FAKE construct present -- masked and byte17 are fresh locals each holding one real, once-read consumed value (ordinary read-then-branch restructuring, not a no-semantic-purpose device).

## [s3] With the inner j-loop, function tail/epilogue, and function entry/prologue all now hand-verified (this session) as matching target's disassembly instruction-for-instruction (including exact register-number identity), the remaining floor-77 edit distance must live in the outer for(i...) loop's field28<=0/>=lim increment-decrement arms (*arg1 & (0x1000<<shift) / *arg1 & (0x4000<<shift), asm/funcs/func_8006CCC8.s roughly lines 55-95), which the s2 ledger entry claimed matched 'structurally' by eye only, never through the engine's actual score.normalized_insns levenshtein diff tool.
- mechanism: n/a -- this is a verification/localization claim about where the residual instruction-level diff sits, not a codegen-pass hypothesis.
- probe: Attempted to run engine.score.normalized_insns directly this session both via the Windows-side Bash tool (python3 -c "from engine import score; ...") and via the PowerShell tool.
- result: Bash attempt BLOCKED by tools/hooks/worktree_contamination_guard.py (flags any bare 'from engine import' with no wteng.ps1 pin as a wrong-repo risk). PowerShell attempt failed with FileNotFoundError because objdump is a WSL-only binary not on the Windows PATH -- engine/score.py's _objdump() calls cfg.OBJDUMP which resolves to a WSL path. Neither attempt produced the diff; the localization claim itself is UNVERIFIED (not confirmed, not killed) -- it is inferred from the hand-walk coverage, not measured with the actual scoring tool. Recorded honestly as unproven; the concrete next step (run from WSL) is in the frontier.
- verdict: KILLED
- kill_scope: instance
- measured_on: n/a -- this hypothesis was about tooling access this session, not a chassis measurement; the underlying localization claim remains open and is carried to the s4 frontier rather than asserted as fact.

## [s4] Splitting the outer-loop initializations into separate statements in target's exact LUID order (i=0; nib=0xF; fade=0; shift=0; mask=0 -- matching target's s1,s6,s5,s2,s3 register-init order) instead of a for-loop init-clause plus repeated 0xF literals, and reading field28 ONCE into a named `field` local reused for all three compares instead of re-reading the same pointer expression per compare, together reproduce enough of target's LICM/CSE structure to drop the honest floor from 77 to 63.
- mechanism: named-intermediate declaration-order family (SOTN-accepted, no-new-park-categories.md) -- the fresh `nib` local biases GCC's loop-invariant-constant hoist to the same LUID slot target's own `li $s6,0xF` occupies; the single `field` read is ordinary CSE-by-source (one read, reused) matching target's single `lh $v1` reused across three compares.
- probe: sandbox --disable all, before/after; masked objdump diff via tmp/grind/func_8006CCC8/s4/diff_probe.py
- result: measured 77 -> 63 this session (src/text1b.c HEAD s4, no FAKE construct -- ordinary fresh-local + single-read reuse).
- verdict: CONFIRMED

## [s4] Restructuring the field28 dispatch from the flat `if(field==3){}else if(field<4){}else if(field==4){}` chain to the nested `if(field!=4){ if(field!=3){ if(field<4){default} } else {case3/+0x1A} } else {case4/+0x1D}` chain reproduces target's exact branch senses (beq/bne polarity) at all three field28 compares AND target's exact physical block ordering (default nearest, then +0x1A case, then +0x1D case, each reached by the correct forward-jump vs fallthrough).
- mechanism: GCC 2.7.2's block-layout convention for nested if/else -- the innermost condition's TRUE arm lands inline (fallthrough), the outer alternative's arm is pushed to a forward-jump target at the end of the enclosing block. A flat if/else-if chain (uniform branch-if-false-to-next-test convention) does not reproduce this layout. Derived by hand-reading the full target disassembly (asm/funcs/func_8006CCC8.s, all 207 lines) and matching branch senses/label distances directly, not asserted from theory alone.
- probe: sandbox --disable all; masked objdump diff (WSL engine.score.normalized_insns via diff_probe.py) at each sub-step
- result: measured 63 -> 40 (first nesting attempt, outer test field!=3) -> 39 (final: outer test field!=4 with case4 as the outer-else, which correctly swapped which case block -- +0x1A vs +0x1D -- lands second vs third in file order, matching target). Both sub-steps measured this session on src/text1b.c HEAD s4; no FAKE construct (ordinary nested if/else, every arm's guard condition is semantically required -- field<4 still excludes field>=5, which target skips entirely).
- verdict: CONFIRMED

## [s4] Combining the nested `if (field < 4) { if (field >= 0 && mask) {...} }` into a single flattened `if (field < 4 && field >= 0 && mask) {...}` condition regresses the honest floor.
- mechanism: n/a -- measured codegen difference between two ordinary, semantically-identical C conditional spellings; no GCC-internals claim made, just an observed regression.
- probe: sandbox --disable all, immediately before/after the single swap (all other source held constant)
- result: 39 -> 65 on this exact swap; reverted to the nested form (kept in candidate.c). Regressed form saved to memory/grind/func_8006CCC8/rejected/combined-lt4-ge0-condition.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD as edited this session (s4), same chassis as the 39-floor candidate immediately before/after; no FAKE construct present in either the nested or combined form (ordinary conditional expressions).

## [s4] Rewriting the C-level pointer-arithmetic operand order for the repeated `*(s16 *)(... D_800A34FC ... + mask + 0x28)` address expression as `mask + (u8 *)D_800A34FC + 0x28` (mask-register-first) instead of `(u8 *)D_800A34FC + mask + 0x28` (base-pointer-first) changes the emitted `addu` operand order to match target's `addu $x,$s3,$v0` (mask-first) instead of the base-first order our build otherwise emits.
- mechanism: Candidate mechanism was GCC's tree-level commutative-operand ordering for pointer-plus-int expressions (fold-const.c) preserving C source operand order into the emitted addu.
- probe: masked objdump diff (diff_probe.py) at the three addu sites (target[53],[72],[84]) before/after the source-level operand-order swap
- result: No change: the addu operand order in our build stayed base-first (v0,s3) in both the mask-first and base-first C spellings -- confirmed still present in the final floor-39 diff (tmp/grind/func_8006CCC8/s4/diff_probe_final_floor39.log). GCC re-canonicalizes the commutative add independent of source spelling in this shape; the lever does not work here.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD s4 candidate (floor 39, both the mask-first and base-first spellings of the address expression measured); no FAKE construct (ordinary pointer arithmetic).

## [s5, permuter] Dropping the `byte17` named intermediate and reading `*(rec + 0x17)` directly at each of the two i==0/else branch-arm use sites (instead of one read before the branch) drops the honest floor from 39 to 23, in BOTH the field28==3 (+0x1A) and field28==4 (+0x1D) inner for(j) record loops.
- mechanism: SOTN-sanctioned "duplicate-read into branch arms" family ([[split-read-defeats-hoist]], no-new-park-categories.md) — ordinary C; the byte read has no side effects and each call executes exactly one of the two arms, so reading it once before the branch vs. once per arm (only one of which ever executes) is behaviorally identical. Candidate mechanism for WHY this changes codegen: cc1's per-basic-block CSE (cse.c) shares a hoisted-before-branch read across both arms as one pseudo with one hard-register home; duplicating the read into each arm gives each arm its own load, freeing register allocation to choose different (target-matching) homes per arm rather than one shared home.
- probe: Found by a directed decomp-permuter campaign against a hand-built full-TU permuter workspace (tmp/grind/func_8006CCC8/s5/perm_ws — the s4-blocked `nonmatchings/`-based `import.py` workspace is still broken by an upstream base.c-pruning conflicting-declaration bug; worked around by flattening src/text1b.c through the project's own cpp+CC_FLAGS pipeline into a self-contained base.c and hand-writing compile.sh/target.o, mirroring tools/mar_perm_workspace.sh's precedent for marionation_Exec). Permuter's own weighted score dropped from base 1480 to a best find of 870 (output-870-2) after ~1900 iterations on the floor-39 chassis. The exact permuter mutation (which left a stray empty `;` statement, a text-diff artifact of its deletion mechanism) was NOT applied verbatim — the equivalent clean-C form (delete the `byte17` local and its one statement; read `*(rec + 0x17)` inline at both use sites) was hand-applied to src/text1b.c and independently measured via `sandbox --disable all`.
- result: 39 -> 23 (target_insns=189, build_insns 184 -> 188)
- verdict: CONFIRMED

## [s5, permuter] Two other permuter finds this session are dead ends — one cheat-shaped (address-of a local purely to force a different addressing mode), one outright incorrect (reuses the LIVE outer-loop `mask` accumulator as inner-loop scratch, corrupting the next outer iteration's address computation).
- mechanism: n/a — vetting/correctness findings about permuter output, not a codegen-pass hypothesis. The address-of form (output-1130-1, score 1130, worse than the accepted fix) matches the forbidden Lever-D-adjacent "steer RA with no observable behavior change" intent by analogy (no frozen family covers "&scalar purely to force addressing"); the mask-reuse form (output-1105-1 lineage) is a genuine runtime-behavior bug, not merely a cheat, since `mask` is read again by the outer for-loop's own update/address-computation after the inner loop returns.
- probe: Read both diff.txt files (tmp/grind/func_8006CCC8/s5/perm_ws/output-1130-1/diff.txt, output-1105-1/diff.txt) and hand-traced `mask`'s liveness across the outer for(;i<2;...) loop body.
- result: Neither surfaced to the Judge; both saved to memory/grind/func_8006CCC8/rejected/ (address-of-local-ra-steer.c, corrupts-outer-mask.c) per the no-new-park-categories.md vetting-checklist discipline ("recognize the find as a cheat AND NOT surface it").
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD floor-23 chassis (post the accepted s5 fix); the address-of form was scored by the permuter's own weighted metric only (1130, never applied to src/text1b.c or sandbox-measured, since it was rejected on cheat-vetting grounds before being tried); the mask-reuse form was rejected on correctness inspection before any measurement. No FAKE construct present in the ACCEPTED s5 fix (ordinary duplicate-read, SOTN family, no annotation prerequisite).

## [s5, permuter] Two directed permuter campaigns (~1900 iters on the floor-39 chassis, ~4000 iters re-seeded on the floor-23 chassis, 6 parallel jobs each, --stop-on-zero) plateau at permuter-internal weighted score ~870 with no further improvement after the split-read fix was applied; every additional novel find at or above that score was either a duplicate of the accepted fix's shape, cheat-shaped (rejected above), or incorrect (rejected above).
- mechanism: n/a — campaign-exhaustion observation, not a GCC-pass claim. Consistent with s4's diagnosis that the remaining floor-23 residual is the addu operand-order canonicalization (already measured dead as a source-level lever in s4) plus a local-alloc.c register-preference tie in the for(j) loop (not yet probed with tools/ra_solver).
- probe: tools/permuter_campaign.py launch/wait/harvest --stop, both campaigns; logs in tmp/grind/func_8006CCC8/s5/perm_ws/campaign.log and the per-output diff.txt/score.txt files.
- result: floor-39 chassis campaign: base 1480, best find 870 (the accepted split-read fix), ~10 total finds, no sub-870 result. floor-23 chassis (re-seeded after applying the fix): base 1190, best find 870 (re-discovery of a variant of the same already-applied fix, confirming it as the permuter's local optimum for this region), ~9 total finds across ~4000 iterations, no further improvement.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD, both the floor-39 pre-fix chassis and floor-23 post-fix chassis, permuter's own weighted scorer (not the engine sandbox metric — see [[scoring-systems]] for why the two numbers aren't directly comparable); no FAKE construct present in either chassis measured.

## [s5] Dropping the byte17 named intermediate and reading *(rec + 0x17) directly at each of the two i==0/else branch-arm use sites (instead of one read before the branch) drops the honest floor from 39 to 23, in BOTH the field28==3 (+0x1A) and field28==4 (+0x1D) inner for(j) record loops.
- mechanism: SOTN-sanctioned duplicate-read-into-branch-arms family (split-read-defeats-hoist); cc1's cse.c shares a hoisted-before-branch read as one pseudo with one hard-register home, duplicating the read into each arm frees register allocation to choose different per-arm homes matching target.
- probe: Directed decomp-permuter campaign against a hand-built full-TU workspace; found output-870-2 (permuter weighted score 1480->870); the equivalent clean-C form was hand-applied to src/text1b.c and independently measured via sandbox --disable all.
- result: 39 -> 23 (target_insns=189, build_insns 184 -> 188)
- verdict: CONFIRMED

## [s5] Two other permuter finds this session are dead ends: an address-of-a-local RA-steering construct with no semantic purpose, and a mutation that reuses the LIVE outer-loop mask accumulator as inner-loop scratch, corrupting the next outer iteration's address computation.
- mechanism: n/a -- vetting/correctness findings about permuter output, not a codegen-pass claim.
- probe: Read both diff.txt outputs (output-1130-1, output-1105-1 lineage) and hand-traced mask's liveness across the outer for(;i<2;...) loop.
- result: Neither surfaced to the Judge; both saved to memory/grind/func_8006CCC8/rejected/ (address-of-local-ra-steer.c, corrupts-outer-mask.c).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD floor-23 chassis (post the accepted s5 fix); address-of form scored only by the permuter's own weighted metric (1130), never sandbox-measured since rejected on cheat-vetting grounds before being tried; mask-reuse form rejected on correctness inspection before any measurement. No FAKE construct present in the accepted s5 fix.

## [s5] Two directed permuter campaigns (~1900 iters on the floor-39 chassis, ~4000 iters re-seeded on the floor-23 chassis, 6 parallel jobs each, --stop-on-zero) plateau at permuter-internal weighted score ~870 with no further improvement after the split-read fix was applied.
- mechanism: n/a -- campaign-exhaustion observation. Consistent with s4's diagnosis that the remaining floor-23 residual is the addu operand-order canonicalization (already measured dead as a source-level lever in s4) plus a local-alloc.c register-preference tie in the for(j) loop.
- probe: tools/permuter_campaign.py launch/wait/harvest --stop on both chassis; logs in tmp/grind/func_8006CCC8/s5/perm_ws/campaign.log and per-output diff.txt/score.txt files.
- result: floor-39 chassis: base 1480, best find 870 (the accepted fix), no sub-870 result. floor-23 chassis (post-fix): base 1190, best find 870 (re-discovery of a variant of the same fix), no further improvement across ~4000 iterations.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD, both the floor-39 pre-fix chassis and floor-23 post-fix chassis, permuter's own weighted scorer; no FAKE construct present in either chassis measured.

## [s6, rederive] Sibling func_80056CB8 reached COMPLETED-C at its s72; re-checked and confirmed still signature/struct-idiom disjoint from func_8006CCC8 (dispatch brief's forced-rederive transplant probe).
- mechanism: n/a -- transplant-applicability check, not a codegen-pass hypothesis.
- probe: Read src/text1b.c:1813 (func_80056CB8's matched, committed body) -- `void func_80056CB8(s32 arg0)`, a hit-detection routine over pt0/pt1/hit0/hit1 stack arrays, no D_800A34FC/D_800A3524 reference.
- result: Confirmed disjoint (1-arg void hit-detection vs our 3-arg s32-return D_800A34FC/D_800A3524 record-update function). No transplantable construct; consistent with every prior cross-check since func_80056CB8's own s54.
- verdict: KILLED
- kill_scope: instance
- measured_on: n/a -- this is a signature/object-model comparison, not a chassis measurement.

## [s6, rederive] Rewriting the repeated `*(s16 *)(mask + (u8 *)D_800A34FC + 0x28)` byte-pointer address expression as typed s16-array indexing `((s16 *)((u8 *)D_800A34FC + 0x28))[i]` (using the outer loop counter `i` directly instead of a separately-tracked `mask` local incremented by 2/iteration) flips the emitted addu operand order at all three D_800A34FC-relative address computations to index-first, matching target, and drops the honest floor from 23 to 20.
- mechanism: GCC's commutative-add operand-order canonicalization differs between a direct pointer-int PLUS (`base + mask`, s4's byte-pointer-arithmetic spelling -- canonicalizes to base-first regardless of C source order, confirmed dead as a lever in s4) and the MULT-then-PLUS shape array-subscript lowering produces (`base + i*sizeof(s16)`) -- the latter preserves index-first operand order into the emitted addu. `mask` was semantically always `i * sizeof(s16)`; this is a more direct, equally truthful spelling of the same value, not a coercion (mask variable is removed, not hidden).
- probe: sandbox --disable all before/after; pwsh tools/grinder/dump.ps1 func_8006CCC8 + hand-read of tmp/grind/func_8006CCC8/dumps/text1b.s at the three address-computation sites (all three now emit `addu $reg,$idx,$base` matching asm/funcs/func_8006CCC8.s's `addu $a0,$s3,$v0` shape).
- result: 23 -> 20 (target_insns=189, build_insns 188 unchanged)
- verdict: CONFIRMED

## [s6, rederive] Reordering the outer for-loop's update clause from `i++, shift += 0x10` to `shift += 0x10, i++` (pure statement-order swap of two independent compound assignments, both still execute every iteration) drops the honest floor from 20 to 18.
- mechanism: target's tail sequence (asm/funcs/func_8006CCC8.s:189-193) updates shift's register ($s2) BEFORE i's register ($s1), with the (now-eliminated) mask-tracking register's update in the loop-back-edge branch's delay slot -- strong evidence the original source's for-loop update-clause listed shift's update before i's. With the mask variable eliminated by the s6 array-index fix (FIX 1 above), matching the relative order of the two REMAINING update terms was the available lever.
- probe: sandbox --disable all before/after; WSL engine.score.normalized_insns masked-opcode diff (tmp/grind/func_8006CCC8/s6/diff_probe.py) confirmed the addiu-order mismatch this fix targeted (target[170:171]='addiu s2,s2,16' vs ours[169:170]='addiu s3,s3,2' at floor 20) is gone at floor 18.
- result: 20 -> 18 (target_insns=189, build_insns 188 unchanged)
- verdict: CONFIRMED

## [s6, rederive] Re-hoisting the byte17 named intermediate (s3-style single read of *(rec+0x17) before the i==0/else branch) on the floor-20 chassis regresses to 36; a flat if/else-if dispatch chain in target's literal compare order (==3, <4, ==4) on the same chassis regresses to 54.
- mechanism: byte17 hoist -- n/a, register-allocation trade-off (see candidate.c header + rejected/byte17-hoist-on-floor20-chassis.c for full detail); the split-read-into-arms form (s5) remains strictly better on every chassis tested in this ledger, now including floor-20/18. Flat dispatch chain -- n/a; GCC 2.7.2's nested-if/else block-LAYOUT convention (which arm falls through vs is a forward jump), not the literal source compare order, is what the s4 nested form actually matches; restated in literal target compare order it does NOT reproduce target's physical layout.
- probe: sandbox --disable all, each variant applied in isolation to the floor-20 chassis, immediately reverted after measurement.
- result: byte17 hoist: 20 -> 36 (build_insns 188 -> 184). Flat literal-order chain: 20 -> 54 (build_insns unchanged at 188). Both reverted; candidate.c keeps the s5 split-read + s4 nested nested-if forms. Side-probed the mirror nested form (outer test field!=3 instead of field!=4) on the floor-18 chassis: scored an identical 18 (neutral).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD s6, floor-20 chassis (both regression probes) and floor-18 chassis (the neutral field!=3 side-probe); no FAKE construct present in any variant tested (all ordinary C restructurings).

## Frontier for s7 (<=3, mechanism-grounded)

1. **Field28-dispatch-chain residual (~8 of the remaining 18 floor points).** The masked-opcode diff (tmp/grind/func_8006CCC8/s6/diff_probe.py output, saved this session) shows target inserts extra insns around the field==4 compare / field>4 "skip" tail (`li v0,0x40; j @; addiu s5,s5,4`) that our nested-if layout doesn't reproduce verbatim, even though overall block ordering matches. Re-run PASS ATTRIBUTION against the s6 dumps (already fresh in tmp/grind/func_8006CCC8/dumps/, generated this session) and read `.jump`/`.jump2` for this region -- cross-jump block merging is the leading candidate mechanism (target's "j @; addiu s5,s5,4" tail looks like a cross-jump-merged shared exit, per [[cross-jump-store-tail-merge]]).
2. **j-loop split-read-vs-hoisted-read RA trade-off (~10 of the remaining 18 points).** Confirmed AGAIN this session (2nd confirmation, s5 was the 1st) that split-read-into-arms beats the hoisted-single-read form despite the hoisted form matching target's literal asm shape insn-for-insn. This is a genuine local-alloc.c register-preference question, not a simple shape-match fix -- run `tools/ra_solver` / `inverse_compose.py classify` on this specific loop's register-conflict graph in a future solver-modality session (carried forward unchanged from s4/s5's own diagnosis, now with 2 independent confirmations instead of 1).
3. Re-verify the function's prologue/epilogue and outer-loop increment-decrement arms (the `field<=0`/`>=lim` arms) haven't drifted from their s3-verified matching shape now that the address-computation spelling changed function-wide -- s3's hand-walk predates the s6 addressing rewrite and should be re-confirmed against the fresh dumps before assuming it's still exact.

## [s6] Sibling func_80056CB8 (this session's forced-rederive trigger, reached COMPLETED-C at its own s72) is confirmed still signature/struct-idiom disjoint from func_8006CCC8 via its matched body at src/text1b.c:1813 -- no transplantable construct.
- mechanism: n/a -- transplant-applicability check, not a codegen-pass hypothesis.
- probe: Read src/text1b.c:1813, func_80056CB8's matched committed body.
- result: 1-arg void hit-detection routine over pt0/pt1/hit0/hit1, no D_800A34FC/D_800A3524 reference -- disjoint from func_8006CCC8's 3-arg s32-return record-update body.
- verdict: KILLED
- kill_scope: instance
- measured_on: n/a -- signature/object-model comparison, not a chassis measurement

## [s6] Rewriting the repeated *(s16 *)(mask + (u8 *)D_800A34FC + 0x28) byte-pointer address expression as typed s16-array indexing ((s16 *)((u8 *)D_800A34FC + 0x28))[i] (eliminating the separate mask local) flips all three D_800A34FC-relative addu operand orders to index-first, matching target, and drops the honest floor from 23 to 20.
- mechanism: Array-subscript lowering produces a MULT(index,scale)-then-PLUS RTL shape rather than a direct pointer-int PLUS; GCC's commutative-add canonicalization preserves index-first operand order for that shape where it did not for the byte-pointer-arithmetic PLUS shape s4 had measured dead.
- probe: sandbox --disable all before/after; pwsh tools/grinder/dump.ps1 func_8006CCC8 + hand-read of tmp/grind/func_8006CCC8/dumps/text1b.s at the three address-computation sites.
- result: 23 -> 20 (target_insns=189, build_insns 188 unchanged); all three addu sites now emit addu $reg,$idx,$base matching target's addu $a0,$s3,$v0 shape.
- verdict: CONFIRMED

## [s6] Reordering the outer for-loop's update clause from i++, shift += 0x10 to shift += 0x10, i++ (pure statement-order swap, both terms still execute every iteration) drops the honest floor from 20 to 18.
- mechanism: Target's tail sequence updates shift's register ($s2) before i's register ($s1), with the now-eliminated mask-tracking register's update in the loop-back-edge branch's delay slot -- evidence the original source's update-clause listed shift before i.
- probe: sandbox --disable all before/after; WSL engine.score.normalized_insns masked-opcode diff (tmp/grind/func_8006CCC8/s6/diff_probe.py) confirmed the targeted addiu-order mismatch is gone at floor 18.
- result: 20 -> 18 (target_insns=189, build_insns 188 unchanged)
- verdict: CONFIRMED

## [s6] Re-hoisting the s3-style byte17 named intermediate (single read of *(rec+0x17) before the i==0/else branch, matching target's own unconditional-both-loads asm shape) on the floor-20 chassis regresses the honest floor to 36.
- mechanism: n/a -- register-allocation trade-off; the split-read-into-arms form (s5) folds to fewer insns when hoisted (184 vs 188) but scores worse by the levenshtein metric, re-confirming s5's original 39-vs-23 finding on a materially more-advanced chassis.
- probe: sandbox --disable all, hoisted-byte17 variant applied in isolation to the floor-20 chassis, then reverted.
- result: 20 -> 36 (build_insns 188 -> 184); reverted. Saved to memory/grind/func_8006CCC8/rejected/byte17-hoist-on-floor20-chassis.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD s6, floor-20 chassis; no FAKE construct present (ordinary C restructuring)

## [s6] Rewriting the field28 dispatch as a flat if/else-if chain in target's literal runtime compare order (==3 first, <4 second, ==4 third) on the floor-20 chassis regresses the honest floor to 54.
- mechanism: GCC 2.7.2's nested-if/else block-LAYOUT convention (which arm falls through vs. is a forward jump), not the literal C-level compare order, is what the s4 nested form actually matches; restated in literal target compare order the C no longer reproduces target's physical block layout.
- probe: sandbox --disable all, flat-chain variant applied in isolation to the floor-20 chassis, then reverted. Also side-probed the mirror nested form (outer test field != 3 instead of field != 4) on the floor-18 chassis.
- result: Flat literal-order chain: 20 -> 54 (build_insns unchanged at 188); reverted. Saved to memory/grind/func_8006CCC8/rejected/flat-target-literal-compare-order-dispatch.c. Mirror nested form (field != 3 outer): scored an identical 18 (neutral, not separately saved).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c HEAD s6, floor-20 chassis (flat-chain probe) and floor-18 chassis (neutral field!=3 side-probe); no FAKE construct present in any variant

## [s7, synthesis] The field28 dispatch is a `switch (field)` with cases 0/1/2, 3, 4 (bodies in that order); GCC 2.7.2's stmt.c decision tree (emit_case_nodes over {[0..2],[3],[4]}) emits target's contiguous `beq 3 / slti 4 + bnez / beq 4 / j default` test group and the `bltz` low-bound range check that s4-s6 had been spelling as an explicit `field >= 0 &&` guard.
- mechanism: stmt.c expand_end_case decision-tree path (tools/gcc-2.7.2/stmt.c:4806-4818, threshold 5 with no casesi on mips) + emit_case_nodes (stmt.c:5580); the [0..2] leaf gets a low-bound check only (node_has_high_bound, stmt.c:5494, satisfied by root low == 2+1).
- probe: sandbox --disable all, nested-if s6 body (18) vs switch012 body, same session, same chassis otherwise.
- result: 18 -> 10, build_insns 188 -> 189 (= target); the entire dispatch cluster left the masked-opcode diff.
- verdict: CONFIRMED

## [s7, synthesis] The j-loop record update selects the nibble MASK constant with a ternary over a single +0x17 read -- `*(rec+0x17) = (u8)((*(rec+0x17) & ((i == 0) ? 0xF0 : 0xF)) + masked);` -- which is target's exact single-lbu / bnez / andi-0xF0-or-0xF / addu / sb shape.
- mechanism: ordinary C; expand emits one MEM read, a conditional jump on i selecting one of two AND immediates, one add, one store -- exactly target's block; no named local for the byte means no extra pseudo competing in local-alloc (the mechanism behind the s3/s5/s6/s7 58-vs-0 gap).
- probe: five spellings J1..J5 measured on the floor-10 switch chassis (see evidence.md s7 table).
- result: J4 (this form) = 0; J3 (ternary over two reads) = 10 = split-read bytes; J1/J2/J5 (any `byte17` local) = 58.
- verdict: CONFIRMED

## [s7, synthesis] Hoisting the +0x17 byte into a named local (`byte17`), whether consumed by if/else, by a ternary, or spelled with array subscripts, on the s7 switch-dispatch chassis regresses the score from 10 to 58 (build_insns 187).
- mechanism: local-alloc/global register assignment shifts once the byte has its own pseudo live across the i==0 branch (same direction as the s5 39-vs-23 and s6 20-vs-36 measurements, third chassis).
- probe: J1, J2, J5 in tmp/grind/func_8006CCC8/s7/jvariant.py, sandbox --disable all each.
- result: 58 / 58 / 58 vs 10 (split-read) and 0 (constant-select ternary).
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c working tree, s7 switch-dispatch chassis (floor 10 before the j-loop probe), no FAKE construct present anywhere in the body.

## [s7, synthesis] The s2 defeat-licm-hoist-var-reuse lever `t` (t = arg2 at loop top, t = i before the func_8006CBD4 call) is no longer needed on the switch chassis: `lim = ((arg2 >> i) & 1) ? 4 : 5;` + `func_8006CBD4(i, *arg1)` measures 0 as well.
- mechanism: measured only; the sign-extension of arg2 stays inline in the loop on this chassis without the multi-set trick (not dump-attributed -- bytes matched).
- probe: J4 body with and without `t`, sandbox --disable all.
- result: 0 and 0. The `t`-free body is the candidate (zero family claims).
- verdict: CONFIRMED

## [s7, synthesis] Spelling the nibble mask as the literal `0xF << fade` instead of the `nib = 0xF` variable moves the `li $s6,0xF` from target's statement-order position (between the i = 0 and fade = 0 inits) to the end of the loop preheader.
- mechanism: loop.c move_movables hoists the invariant constant load to the preheader end; a source-level assignment is emitted in statement order. Target's position matches the assignment, so the original had the variable.
- probe: text1b_J4_not_nonib (literal) vs the nib body, sandbox --disable all.
- result: literal = 2 (the two diff entries are that one `li s6,15` moving), variable = 0.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c working tree, s7 switch + J4 chassis, no FAKE construct present.

## Frontier for s8 (<=3)
1. None for the bytes: floor 0 measured this session with the body resident. Next step is review (layer-1 cheat-reviewer, then Judge). The only construct a reviewer may question is the `nib = 0xF` mask variable; the byte evidence for it (preheader init ORDER) is in evidence.md s7 and self_vet.md T5. If it is classified as constant-holder rather than ordinary C, the correct move is a `ruling-request` on that exact question, NOT a respelling (the literal form measures 2, and the body is keyed by content).
2. If the Judge wants pass attribution for the dispatch, read tmp/grind/func_8006CCC8/dumps/text1b.jump after `pwsh tools/grinder/dump.ps1 func_8006CCC8` for the case012 block inversion (not done this session; bytes matched).

## [s7, solver] Candidate.c (switch dispatch + ternary nibble select + `nib = 0xF` mask variable) measures sandbox 0 (189/189) on the current HEAD chassis with the body applied by tmp/grind/func_8006CCC8/s7/apply2.py.
- mechanism: n/a -- chassis re-measurement (the brief's HEAD floor was "measurement unavailable").
- probe: apply2.py cand; sandbox func_8006CCC8 --disable all, at session start and end.
- result: 0 both times; the caller's stale `extern void` prototype is deleted (definition precedes it).
- verdict: CONFIRMED

## [s7, solver] Removing the `nib` mask variable (literal `0xF << fade`) leaves a residual that inverse_compose classifies as SCHED (same instructions and registers, 3 preheader slots reordered) and that sched_solver perturb reaches with exactly one vector: `luid_move 547 -> immediately before 54`, i.e. the constant-15 set emitted as a source statement between `i = 0` and `fade = 0`.
- mechanism: loop.c move_movables hoists the literal's (const_int 15) set (insn 547, tmp/grind/func_8006CCC8/s7/loop_literal_func.txt) to the preheader END; target's `addiu $s6,$zero,0xF` is in statement order. sched2 cannot move it (all five preheader sets are independent, ties keep LUID order), so the only realisation of the vector is a source statement, which is the mask variable itself.
- probe: s7/classify_literal.txt, s7/sched_literal_perturb.txt (--atoms luid,luid_move --depth 2, 30 atoms + pairs).
- result: literal = 2; REACHABLE with a single vector whose C spelling is `nib = 0xF;` at that position (the driver-banned construct); no second vector.
- verdict: CONFIRMED

## [s7, solver] Four nib-free or re-positioned mask spellings on the score-0 chassis: literal `0xF << fade` (2), literal with inits reordered fade/shift/i (4), walking mask `nib <<= 4` without a shift counter (29, 184 insns), per-outer-iteration `msk = 0xF << fade` local (21, 188 insns), and `nib = 0xF` placed before `i = 0` (2).
- mechanism: the first two and the last are the preheader emission-order fact above; the walking mask and the hoisted per-iteration mask remove the in-loop `sllv $a2,$s6,$s5` target performs every j-iteration.
- probe: tmp/grind/func_8006CCC8/s7/sweep.ps1 via apply2.py variants; banked as rejected/literal-nibble-mask-0xF-shl-fade.c, literal-mask-inits-reordered-fade-shift-i.c, walk-the-mask-nib-shl4-no-fade.c, per-outer-iteration-mask-local-msk.c, nib-init-before-i-init.c.
- result: none reaches 0; the best nib-free form is the plain literal at 2.
- verdict: KILLED
- kill_scope: instance
- measured_on: src/text1b.c working tree, s7 switch + ternary-mask chassis (candidate.c = 0), no FAKE construct present in any variant.

## Frontier for s8 (<=3)
1. RULING PENDING (this session's outcome): whether `nib = 0xF` (read mask, `nib << fade`, held in $s6 across calls, byte-evidenced statement position) is ordinary C or a constant-holder needing `/* FAKE: ..., mechanism: loop.c move_movables vs statement-order emission, lever-exhaustion: hypotheses.md s7 */` under named-local-fake-exception. Once ruled, submit candidate.c EXACTLY (annotated or not per the ruling) -- do not respell. SOTN const_holder precedents: docs/reference/sotn-construct-index.md:2635 (handFlag = 0x80000000) and :2658 (InitDistRandRangeX = 0xF).
2. Nothing else is open for the bytes: 0 measured this session with the body resident.
