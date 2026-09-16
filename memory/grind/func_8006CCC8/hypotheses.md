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
