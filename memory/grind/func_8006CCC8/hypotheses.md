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

## Frontier for s3 (<=3, mechanism-grounded)

1. **H4 above** — the inner j-loop record-update shape mismatch is now the
   only unresolved structural gap identified. Read `.cse`/`.combine` dumps
   for the loop's insn range, and diff `asm/funcs/func_8006CCC8.s` lines
   130-200 instruction-for-instruction against
   `tmp/grind/func_8006CCC8/dumps/text1b.s` lines 16692-16780 before
   restructuring.
2. Once H4 lands (or is killed), continue the end-to-end disassembly
   verification that was never finished in s1: the function's tail past
   `.L8006CE80` (target lines ~120-200) has not been individually walked
   against our current build's equivalent region this session either.
3. If H4's natural-geometry restructure doesn't close it, re-run
   `pwsh tools/grinder/dump.ps1 func_8006CCC8` fresh (the s2 dumps are
   against the s2-final body; a further structural change invalidates
   them) and re-derive from the fresh `.loop`/`.greg` output rather than
   reusing s2's dumps.

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
