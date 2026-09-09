# Evidence bank — _spu_pitch2note

## s1 (2026-09-08, recon) — baseline, target decode, mechanism reads

OBJECT MODEL: the target touches NO globals (asm/funcs/_spu_pitch2note.s has no
%hi/%lo/%gp_rel references; it is pure register arithmetic on three u16 params).
No DATA MODEL signals; no declaration fix applies. MATCHES trivially.

CHASSIS: HEAD src/main.c has `INCLUDE_ASM("asm/funcs", _spu_pitch2note);` at
main.c:3113, directly after the COMPLETED-C `_spu_note2pitch` candidate and
before `SpuGetVoiceVolume`. No prototype anywhere in include/ (grep
"pitch2note" include/ is empty); no caller in asm/funcs (grep "jal.*_spu_pitch2note"
is empty) - the function is an exported library entry with no in-EXE caller, so
the prototype is free (s1 used `s32 _spu_pitch2note(u16 cen_note, u16 cen_fine,
u16 pitch)`; the three `andi $r,0xFFFF` widenings at 0x8008BC74/BCB0/BD5C/BD74
confirm all three params are u16).

canonical: verdict C, hand_coded_tier LOW (no S1/S2/S6), pure-C distance 59 at
the first honest body. sandbox --disable all: v1 = 59 (target 74 insns).

TARGET ALGORITHM (decoded from asm/funcs/_spu_pitch2note.s):
  1. search = ~pitch & 0xFFFF; scan shift 15..0 for the first CLEAR bit of
     search (= highest set bit of pitch); bit = shift; break. (`srav` = signed
     shift -> search is int-promoted u16; `andi 0xFFFF` after the `nor`.)
  2. oct = bit - 12 ($t7); scale = 1 << bit ($t6); curve = 0x1000 ($t0);
     target = pitch ($a2, sltu => unsigned).
  3. outer 0..47 ($t2): lower = scale*curve (mult before loop label + reorg
     peel in the back-edge slot, mflo $t4); curve = curve*0x103B>>12 (shift
     chain, srl => u32); upper = scale*curve (mflo $v1); step = (upper-lower)>>5
     ($t1, srl); base = outer<<5 ($t5, LICM-hoisted out of the inner loop);
     acc = 0 ($t3); next = step ($a3).
  4. inner 0..31 ($a0): v0 = lower+acc; v1 = lower+next (BOTH adds in the
     first BB, before the first branch); v0>>=12; if (target < v0) goto next;
     v1>>=12 (in the bnez delay slot); if (target < v1) -> hit (bnez straight
     to .L8008BD44 with `addu v0,t5,a0` = base+inner in the delay slot -
     reorg fill_eager stole the single-insn hit block and deleted it);
     next: next += step; inner++; slti; bnez top with `acc += step` in the
     back-edge delay slot (fill_simple from before the branch).
  5. exhaust: outer++; bnez outer-loop with `mult t6,t0` peel; li v0,0x600.
  6. found (.L8008BD44): quot = result/128 (signed: bgez/addiu 127/sra 7),
     rem = result - (quot<<7), note = cen_note + quot + oct*12,
     fine = cen_fine + rem, return (note<<8)|fine.

MECHANISM READS (dumps in tmp/grind/_spu_pitch2note/dumps/ for v1 and v11;
loop-pass commentary saved at tmp/grind/_spu_pitch2note/s1/v1.loop.txt):
  - loop.c giv accounting for `lower + step*j` / `lower + step*(j+1)` (v1):
    insn 128 `lower + (step*j)` IS a giv (mult step, add lower) -> reduced to a
    fresh biv INITIALISED TO lower (that is the `move a3,t3` seat in v1; the
    target has no such init). insn 138 `lower + (step*(j+1))` is NOT a giv:
    simplify_giv_expr's PLUS case (loop.c:5096) rejects an add_val that is the
    sum of two invariant REGs (lower + step), so it stays as an add of the
    reduced inner giv. The target keeps `lower` as a separate add on BOTH
    sides with accumulators initialised to 0 and step -> explicit loop-carried
    accumulators (`acc`, `next`): mult-1 givs `lower + acc` are dropped by the
    "not worth while" gate at loop.c:3823 (lifetime*threshold*benefit <
    insn_count) and remain as in-loop adds. Measured: v2 (explicit acc/next)
    reproduced the two zero/step-initialised bivs and the `addu v0,t4,t3` shape.
  - unroll.c:3051 final_biv_value refuses any loop with
    loop_number_exit_count != 0. A `cents++` running counter read after a
    `goto found` exit therefore cannot be eliminated (v9/v10 keep an
    `addiu cents,1` in the inner loop and lose the `sll t5` base); the target's
    `li v0,0x600` + `base + inner` come from an explicit `result = 0x600`
    after the loops and `result = (outer<<5)+inner` in the hit block (v7).
  - reorg fill_simple_delay_slots (reorg.c:2969ff) stops the backward search at
    a CODE_LABEL. With the if/else form (v7/v13) the then-arm {next+=; acc+=}
    ends in `j Lend` (over the else/hit block) and Lend sits between the
    increments and `inner++`, so the back-edge bnez cannot take `acc += step`
    from before and fill_eager peels the loop-top `addu v0,t3,t1` instead
    (the ONLY structural residual left in v13's inner loop). The target has no
    label/jump between the increments and `inner++`: its hit block is
    out-of-line and its increments fall straight into the loop increment.
  - The hit block IS stolen into the `bnez -> found` slot once `result` is
    allocated to $v0 (v13: `addu v0,t5,a0` in the slot, block deleted). In v4
    `result` sat in $a2 (= live `target`) so the steal was refused; the fix was
    the epilogue order (named `quot`/`rem` first, v7), which shortened
    `result`'s live range so global.c gave it $v0.
  - v11 (hit label placed after the loops + `goto found`): a jump.c pass
    between the .jump and .cse2 dumps (toplev.c:2923/2929 jump_optimize calls,
    i.e. after loop_optimize) RELOCATED the scan loop's break block
    {bit = shift; j after_scan} out of line to a fresh label (262) placed after
    the `j found` barrier, immediately before `hit:`, and inverted the scan
    branch (bnez->beqz). v13 (no `hit:` label) keeps the block inline exactly as
    the target does. The mechanism that minted label 262 was NOT identified
    this session (jump.c candidates that call get_label_after: 1531, 1718,
    2324; range-swap "if (foo) bar; else break;" at jump.c:1840ff is disabled
    post-reload but live at the 2923/2929 calls). Whatever relocates a
    `{X; jump L}` block after a barrier is plausibly the same mechanism that
    put the target's hit block out of line - the frontier item.

SCORE LADDER (sandbox --disable all, target 74 insns; forms in
tmp/grind/_spu_pitch2note/s1/vN.c):
  v1 59  natural `lower + step*j` / `step*(j+1)`, `&&`-goto inline hit, packed return
  v2 60  explicit acc/next accumulators, shift at the compare
  v3 55  v2 + `lo = (lower+acc)>>12; hi = (lower+next)>>12;` before the if
  v4 55  v3 as if/else (`if (t<lo || t>=hi) {incs} else {result; goto found}`)
  v5 60  v3 + `goto hit` with `hit: result = (outer<<5)+inner;` after the loops (base not hoisted)
  v6 51  v4 + `fine` computed before `note`
  v7 36  v4 + named `quot = result/128; rem = result%128;` then note/fine  <- epilogue fully matched
  v8 54  v4 + quot/rem named but packed return expression
  v9 45  v7 with `cents++` counter (`&&`-goto)         - counter NOT eliminated
  v10 45 v7 with `cents++` counter (if/else)           - same
  v11 41 v7 + `hit:` after loops with `base = outer<<5` named at inner-loop entry - scan block relocated, `j found` kept
  v12 41 v11 with `base = outer * 32` at outer-loop top
  v13 36 v7 with `next += step; acc += step;` (increment order swapped)  <- BEST, candidate.c
  v14 57 v13 no-else + `result = 0x600` initialised before the loops
  v15 57 v14 as `if {hit} else {incs}`
  v16 58 v13 (if/else) + `result = 0x600` before the loops
  v17 41 v11 with swapped increments

v13 RESIDUAL (36; side-by-side at tmp/grind/_spu_pitch2note/s1/v13_sbs.txt):
  structural - only the inner-loop back-edge delay slot (target: `addu t3,t3,t1`
  = acc+=step from before the branch; ours: loop-top `addu v0,t3,t1` peel) and
  the order `addu a3 / addu t1 / addiu a0` vs target `addu a3 / addiu a0 / [slot acc]`.
  seats - pitch copy $a3 vs $a0; bit $a0 vs $v1; shift $v1 vs $t2; curve $t2 vs
  $t0; lower $t3 vs $t4; outer $t4 vs $t2; acc $t1 vs $t3; step $t0 vs $t1
  (a cascade; likely resolves once the label/jump between the increments and
  `inner++` is gone and the pseudo set matches). Prologue `move t8,a0` (cen_note
  -> $t8) and the whole epilogue already match.

- [s1] OBJECT MODEL: no globals touched by the target (no %hi/%lo/%gp_rel in asm/funcs/_spu_pitch2note.s); no DATA MODEL signals; MATCHES trivially

- [s1] canonical: verdict C, hand_coded_tier LOW; no header prototype and no in-EXE caller (exported LIBSPU/S_N2P entry) so the prototype s32(u16,u16,u16) is free; three andi 0xFFFF widenings confirm u16 params

- [s1] Target algorithm fully decoded (evidence.md): reverse scan of ~pitch&0xFFFF for the highest set bit, 48x32 0x103B curve walk with two loop-carried accumulators, explicit result=0x600 on exhaustion, /128 %128 divmod, (note<<8)|fine pack

- [s1] loop.c:5096 rejects `lower + step*(j+1)` as a giv (add_val = sum of two invariant regs) while `lower + step*j` is reduced whole with lower folded into its init; explicit accumulators give the target's shape

- [s1] reorg.c:2969 backward delay-slot search stops at a CODE_LABEL: the if/else then-arm's `j Lend` puts a label between the increments and inner++, so the back-edge slot gets the loop-top peel instead of `acc += step` - the only structural residual in v13

- [s1] v13 residual seats: pitch copy $a3 vs $a0; bit $a0 vs $v1; shift $v1 vs $t2; curve $t2 vs $t0; lower $t3 vs $t4; outer $t4 vs $t2; acc $t1 vs $t3; step $t0 vs $t1

- [s1] A pre-reload jump_optimize pass (between .jump and .cse2) can relocate a `{X; jump L}` block reached by fallthrough to a fresh label after a barrier (seen on v11's scan block, label 262); mechanism not yet identified - candidates jump.c:1531/1718/2324 get_label_after sites and the range swap at jump.c:1840ff


## s2 (2026-09-08, structural) — floor 36 -> 21, insn multiset now EXACT

SCORE LADDER (sandbox --disable all; forms in tmp/grind/_spu_pitch2note/s2/):
  v13 36  s1 candidate (baseline re-measured this session: 36, 75 insns)
  v20 35  v13 + hit arm first (`if (target >= lo && target < hi) {hit} incs;`)
  v21 24  v13 + sibling curve spelling (`curve *= 0x103B; curve >>= 12;`)
  v22 23  v20 + v21 together  <- 74 insns == target
  v23 36  miss-arm with `continue` (label still between incs and inner++)
  v24/v26/v27/v28 23  four declaration-order permutations - BYTE-IDENTICAL streams
  v31 23  acc/next init order swapped        v32 23  `base = outer << 5;` named
  v34 23  acc = 0 at top of outer body       v35 38  `target = pitch;` first statement
  v40 22  scale before oct                   v41 24  bit = 0 before search
  v42 23  scan loop as while + explicit decrement
  v43 24  target before curve                v44 24  `scale = 1; scale <<= bit;`
  p00..p23  21..25  all 24 permutations of the four pre-loop assignments
  p21 21  `target; scale; curve; oct;`  <- BEST, candidate.c
  q1 21 / q2 21 (outer-body order, inert on p21)   q3 23 (bit=0 first, on p21)

WHAT THE SIBLING GAVE US: `_spu_2pitch` (COMPLETED-C, src/main.c, the inlinee
of the matched `_spu_note2pitch`) walks the SAME 0x103B curve and spells the
step as two compound assignments, `ratio *= 0x103B; ratio >>= 12;`.  Our s1
body had the folded `curve = (curve * 0x103B) >> 12;`.  Transplanting the
sibling's split spelling alone is worth 12 points (36 -> 24).  This is the
sibling-ledger mandate paying off literally: the fix was sitting in a
COMPLETED-C function twelve lines above ours in the same file.

S1'S F1 IS CLOSED.  Putting the HIT arm first and letting the two increments
fall through to the loop increment removes the CODE_LABEL that s1 measured
between `acc += step` and `inner++`; reorg.c:2969 fill_simple_delay_slots then
takes `acc += step` into the back-edge delay slot exactly as the target does,
and build_insns drops to 74 == target_insns.

CLASSIFIER (tools/ra_solver/inverse_compose.py classify main _spu_pitch2note
--target-object build/src/main.o --ours-object tmp/sandbox/_spu_pitch2note/main.o):
  "FIRST DIVERGENCE: RA - same instructions, different registers".  The
  register-blanked multisets are identical; only 5 seats and 2 in-block
  orderings remain.

INSTRUMENTED-cc1 GOTCHA (cost half a turn, do not re-derive): tools/gcc-2.7.2/cc1
SEGFAULTS on the full preprocessed src/main.c (exit 139 after 59 .ent, last
function _spu_FiDMA) - ra_solver/extract.py therefore fails with "FATAL:
_spu_pitch2note not in .ent list (59 functions)".  Workaround used here: a SOLO
TU (tmp/grind/_spu_pitch2note/s2/solo/solo.c = `#include "common.h"` + the
function) compiles clean and produces the SAME allocation as the in-tree build
(verified register-for-register against the sandbox stream), so the solver runs
on it.  extract_solo.py (ROOT + src dir repointed) lives in the s2 scratch dir,
NOT in tools/.

RA MODEL RESULT (the s3 frontier):
  simulate.py on the solo model is EXACT - 21/21 dispositions, sort order MATCH,
  one reload-retry note (pseudo 86: 65->3).  Our priority order is
  shift 41250 > ... > bit 7692 > ... > pitch-copy 1176.
  inverse.py global with the full 6-seat goal, and with the single flip
  shift $v1 -> $t2, both return the depth-2 NEGATIVE ("no perturbation of any
  modelled input - refs / live span / birth order / conflicts / preferences /
  calls-crossed - reaches the target assignment"), and it names the unmodelled
  candidates: local-alloc's suggested-register pass, qty_size, reload retry.
  A hand what-if applying exactly the livelen deltas implied by the two observed
  sched1 inversions is also 0/6.  So the remaining seats are NOT reachable by
  moving refs/live-ranges from C on this chassis; the next instrument is
  local_extract.py --suggest (BB2_SUGG_DEBUG) on the solo TU.

- [s2] Sibling transplant: `_spu_2pitch`'s `ratio *= 0x103B; ratio >>= 12;`
  spelling of the curve step is worth -12 on this function (36 -> 24)

- [s2] Hit-arm-first + fall-through increments closes s1's F1: no CODE_LABEL
  between `acc += step` and `inner++`, back-edge delay slot matches, 74/74 insns

- [s2] Declaration order of the 21 locals is inert here (4 permutations, all 23,
  identical streams); pre-loop STATEMENT order is not (24 permutations, 21..25)

- [s2] tools/gcc-2.7.2/cc1 (instrumented) segfaults on full main.i; the solo-TU
  workaround reproduces the allocation exactly and is the way to run ra_solver
  on any src/main.c function

- [s2] ra_solver inverse.py depth-2 NEGATIVE for the 5-seat residual and for the
  single shift $v1->$t2 flip; forward model exact 21/21 - the residual's inputs
  are outside the global model (suggested-register pass / reload retry)

- [s2] Sibling transplant is worth -12 alone: _spu_2pitch (COMPLETED-C, src/main.c, the inlinee of the matched _spu_note2pitch) spells the shared 0x103B curve step as `ratio *= 0x103B; ratio >>= 12;`; our s1 body had the folded `curve = (curve*0x103B)>>12;`

- [s2] Floor 36 -> 21 this session; build_insns is now 74 == target_insns 74 and inverse_compose classify reports FIRST DIVERGENCE: RA (register-blanked multisets identical)

- [s2] s1's F1 (back-edge delay slot / CODE_LABEL between the increments and inner++) is CLOSED by putting the hit arm first

- [s2] Declaration order of the locals is inert here (4 permutations, identical streams); pre-loop STATEMENT order is not (24 permutations, 21..25, unique min p21)

- [s2] tools/gcc-2.7.2/cc1 (the instrumented one) SEGFAULTS on the full preprocessed src/main.c - exit 139 after 59 .ent, last function _spu_FiDMA - so ra_solver/extract.py fails with 'FATAL: _spu_pitch2note not in .ent list (59 functions)'. A solo TU (#include common.h + the function body) compiles clean and reproduces the in-tree allocation register-for-register; extract_solo.py (ROOT and src dir repointed) is banked in the s2 scratch dir

- [s2] ra_solver forward model on the solo TU is EXACT (21/21 dispositions, sort order MATCH); pseudo map banked in hypotheses.md (76 pitch-copy, 79 bit, 80 shift, 85 lower, 88 acc, 92 outer)

- [s2] Statement order inside the outer-loop body and `bit = 0` placement are inert or worse on the p21 base (q1 21, q2 21, q3 23)

- [s3] FLOOR 21 -> 20.  The single change is the split of the UPPER inner-loop
  bound: 'hi = lower + next; hi >>= 12;' with the lower bound left folded
  ('lo = (lower + acc) >> 12;').  Splitting the lower bound instead, or both,
  measures 21.  Body: memory/grind/_spu_pitch2note/candidate.c.
- [s3] The full split-compound-assignment sweep over every remaining folded
  expression is banked (hypotheses.md H16): only the 'hi' split helps; the
  'step', 'result' and 'return' splits cost 13 / 15 / 24 points and the last two
  ALSO add insns (75 / 76 vs 74).
- [s3] The 24-permutation pre-loop order sweep was re-run on the new loop shape:
  'target; scale; curve; oct;' (s2's p21 = s3's q00) is still the unique minimum
  at 20; the spread is 20..24.
- [s3] Residual at 20 (pairdiff: tmp/grind/_spu_pitch2note/s3/r2b_pairdiff.txt),
  74/74 insns, insn multiset matches:
    seats  pitch-copy $a3 vs $a0 | shift $a0 vs $t2 | lower $t3 vs $t4 |
           acc $t2 vs $t3 | outer $t4 vs $t2
    order  ours 'andi a2,a3,0xffff' at 15 and 'oct' at 19; target 'oct' at 15
           and the andi at 20, after 'move outer,zero'
    order  ours emits the UPPER bound's addu before the LOWER bound's; the
           target emits the lower first.  No source ordering of the two bounds
           flips it (h2/h3/e2/e3 all 20 with identical streams) - it is sched1.
- [s3] The target seats the pitch copy in $a0 - an INCOMING-ARG register freed by
  'move t8,a0' (cen_note) at insn 0 - while we take $a3, an ordinary free
  call-clobbered register.  That is the signature of local-alloc's
  copy-suggestion pass (qty_phys_copy_sugg), which is the one modelled RA input
  s2's inverse.py run did not carry.  It is the s4 F1 probe.
- [s3] Eliminating the pitch copy is NOT the answer: deriving 'search' from
  'target' (d1) does remove it - 73 insns - and scores 22.  The target's 74
  insns include the copy.
- [s3] Sweep harness for the next session: tmp/grind/_spu_pitch2note/s3/apply.py
  (splices a body file over the INCLUDE_ASM line in a pristine
  'git show HEAD:src/main.c', LF-safe) + sweep.ps1 (applies + sandboxes a list of
  variant names) + v/*.c (57 measured variants including q00..q23).

- [s3] Floor 21 -> 20 on the HEAD chassis; the inherited s2 body was re-measured at 21 before any probe, so the chassis is unchanged from the ledger.

- [s3] The winning body is the s2 candidate with exactly one edit: hi = lower + next; hi >>= 12; replacing hi = (lower + next) >> 12;. 74 build insns == 74 target insns; the insn multiset matches.

- [s3] Residual at 20 is a 5-seat RA permutation (pitch-copy $a3 vs $a0, shift $a0 vs $t2, lower $t3 vs $t4, acc $t2 vs $t3, outer $t4 vs $t2) plus two order inversions: (andi target / move outer,zero) and (addu lo / addu hi).

- [s3] NEW: the target emits the LOWER bound's addu before the UPPER bound's; ours emits the upper first, and no source ordering of the two bound statements flips it (h2/h3/e2/e3 produce identical streams) - it is sched1, not the C order.

- [s3] NEW: the target seats the pitch copy in $a0, an incoming-arg register freed by move t8,a0 (cen_note) at insn 0, while we take the ordinary free $a3. That is the signature of local-alloc's copy-suggestion pass (qty_phys_copy_sugg), the one modelled RA input s2's inverse.py run did not carry.

- [s3] Eliminating the pitch copy is not the answer: d1 removes it (73 insns) and scores 22; the target's 74 insns include the copy.

- [s3] Sweep harness for the next session: tmp/grind/_spu_pitch2note/s3/apply.py (splices a body file over the INCLUDE_ASM line in a pristine git show HEAD:src/main.c, LF-safe) + sweep.ps1 (applies + sandboxes a list of variant names) + v/*.c (57 measured variants including q00..q23).

- [s3] src/main.c was restored to HEAD at the end of the session; the 20-floor body lives in memory/grind/_spu_pitch2note/candidate.c.

## s4 (2026-09-08, permuter) — floor 20 -> 19

CHASSIS RE-MEASURED FIRST: the inherited s3 candidate re-measured at 20 on HEAD
before any probe, so the chassis is unchanged from the ledger.

PERMUTER WORKSPACE (new, reusable): tmp/perm_p2n/ built by
tmp/grind/_spu_pitch2note/s4/mkws.sh.  base.c = the common.h typedefs + the
candidate body (a SOLO TU, the s2-validated workaround for cc1's segfault on
the full main.i); compile.sh = the real pipeline with the CURRENT canonical
flags (-O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin
-mel -msoft-float, prologue_fix, maspsx 2.34 with --prefill-label-funcs,
multu_pad); target.o = prelude_r3k.inc + asm/funcs/_spu_pitch2note.s.  Validated
at build time: 74 base insns == 74 target insns and the objdump diff reproduces
the in-tree residual register-for-register.  The two stale helpers
tools/mar_perm_workspace.sh / tools/mar_perm_compile.sh are NOT usable as-is:
they predate -mel/-msoft-float and --prefill-label-funcs.

CAMPAIGN 1 (tmp/perm_p2n, label r2b-chassis-s4, -j 8, --stop-on-zero):
37,051 iterations / 1,105 s / 103 finds; base permuter score 580, best 230.
No zero.  Harvested and stopped in-session.

CALIBRATION — THE PERMUTER'S SCORE AND THE SANDBOX DISTANCE ARE DECOUPLED HERE.
38 finds were spliced back into src/main.c and measured with
`sandbox _spu_pitch2note --disable all`.  The permuter's weighted score
(regs x5, reorderings x60) ranges 230..580 across those finds while the honest
sandbox distance ranges 19..60, and the ranking does not agree:
  permuter 230 -> sandbox 20      permuter 485 -> sandbox 20 (x3)
  permuter 385 -> sandbox 21/20   permuter 565 -> sandbox 19 (x2)  <- the winners
  permuter 480 -> sandbox 60      permuter 580 -> sandbox 20 (the base itself)
So on an RA-seat residual of this shape the permuter cannot be steered by its
own score: the useful product of a campaign is the POPULATION of structurally
distinct finds, each of which must be sandbox-measured individually.  A campaign
that is left to descend its own gradient walks away from the answer.

THE WIN CAME FROM A SEMANTICALLY BROKEN FIND.  output-565-1 and output-565-7
measured 19 (one below the standing floor) by hoisting `acc = 0;` OUT of the
outer loop entirely - illegal, because acc must reset to 0 on every outer
iteration.  What it proved is that the EMISSION POSITION of `move acc,zero` is
worth exactly one point.  A 9-variant hand sweep (a1..a9) then found the legal
spelling that reproduces the position: put the accumulator initialisations in
the inner `for` init clause as a comma expression, with `acc = 0` AFTER
`inner = 0`.
  19: a1 `for (inner = 0, acc = 0, next = step; ...)`
      a3 `for (inner = 0, next = step, acc = 0; ...)`
      a4 `next = step;  for (inner = 0, acc = 0; ...)`
      a8 `for (next = step, inner = 0, acc = 0; ...)`
  20: a2 / a6 (acc = 0 first in the init clause), a5 (acc = 0 left outside the
      init clause), a7 / a9 (acc = 0 before inner = 0)
All nine build 74 insns.  a1 is the new candidate.

RE-SWEEPS ON THE NEW (a1) LOOP SHAPE:
  - pre-loop statement order, all 24 permutations (s4 p00..p23): range 19..23,
    `target; scale; curve; oct;` still the unique minimum.  Third consecutive
    session in which this order survives a loop-shape change.
  - naming `base = outer << 5`: byte-neutral at 19 when it sits INSIDE the
    for-init (c1 `inner, base, acc, next`; c2 `next` outside; c4 `inner, base,
    next, acc`); 20 when it sits outside the for-init (c3, c5).  So the LICM
    hoist of `outer << 5` is not steerable from C on this chassis.

RESIDUAL AT 19 (tmp/grind/_spu_pitch2note/s4/a1_pairdiff.txt, 74/74 insns):
  seats  pitch-copy $a3 vs $a0 | shift $a0 vs $t2 | lower $t3 vs $t4 |
         acc $t2 vs $t3 | outer $t4 vs $t2   (unchanged from s3)
  order  ours `andi a2,a3,0xffff` at 16 and `oct` at 20; target `oct` at 16 and
         the andi at 21, after `move outer,zero`  (unchanged from s3)
  order  ours emits `move acc,zero` at 33 then the LICM-hoisted
         `sll base,outer,5` at 34; the target emits the sll at 33 and
         `move acc,zero` at 34.  This pair is ONE apart and is the newest,
         cheapest-looking order residual - the for-init sweep moved acc into
         this neighbourhood but did not cross the sll.
  order  ours emits the UPPER bound's addu before the LOWER bound's; the target
         emits the lower first  (unchanged from s3, provably immune to C order)

- [s4] FLOOR 20 -> 19.  The single change is moving the inner loop's accumulator
  initialisations into the for-init comma expression with `acc = 0` after
  `inner = 0`: `for (inner = 0, acc = 0, next = step; inner < 0x20; inner++)`.
  Ordinary C, no FAKE construct, 74/74 insns.
- [s4] The permuter's weighted score does NOT track the engine's honest sandbox
  distance on this function (230 -> 20 while 565 -> 19); a campaign here must be
  mined as a population of distinct forms, each sandbox-measured, not steered by
  its own gradient.
- [s4] A semantically INVALID permuter find can still be the pointer: the 19 was
  first seen as an illegal hoist of `acc = 0` out of the outer loop, which
  localised the point to the emission position of `move acc,zero`, and the legal
  for-init spelling then reproduced it.
- [s4] Reusable permuter workspace for this function: tmp/perm_p2n (built by
  tmp/grind/_spu_pitch2note/s4/mkws.sh, solo-TU base + real pipeline compile.sh
  + validated target.o).  tools/mar_perm_workspace.sh is stale (pre -mel,
  pre -msoft-float, pre --prefill-label-funcs).
- [s4] Pre-loop order `target; scale; curve; oct;` survives a third loop-shape
  change (24 permutations re-measured, 19..23, unique minimum).
- [s4] Naming `base = outer << 5` is byte-neutral at 19 inside the for-init and
  costs a point outside it; the LICM hoist of `outer << 5` is not steerable.

- [s4] [s4] FLOOR 20 -> 19. The single change over the s3 candidate is `for (inner = 0, acc = 0, next = step; inner < 0x20; inner++)` replacing the two separate statements `acc = 0; next = step;` before `for (inner = 0; ...)`. Ordinary C (a comma expression in a for-init), no FAKE construct, 74 build insns == 74 target insns.

- [s4] [s4] The lever was discovered, not invented: two campaign-1 finds (output-565-1, output-565-7) measured 19 by hoisting `acc = 0;` OUT of the outer loop entirely - a SEMANTIC BREAK, since acc must reset on every outer iteration - which localised the missing point to the emission POSITION of `move acc,zero`. The a1..a9 hand sweep then found the legal spelling that reproduces that position.

- [s4] [s4] The permuter's weighted score does NOT track the engine's honest sandbox distance on this residual: 230 -> 20, 480 -> 60, 485 -> 20, 565 -> 19, 580 (the base) -> 20 in campaign 1, and 380 -> 20 vs 575 -> 19 in campaign 2. The useful product of a campaign here is the POPULATION of structurally distinct forms, each sandbox-measured; a campaign left to descend its own gradient walks away from the answer.

- [s4] [s4] Reusable permuter workspace for this function: tmp/perm_p2n (built by tmp/grind/_spu_pitch2note/s4/mkws.sh) - solo-TU base.c (the s2-validated workaround for cc1's segfault on the full main.i), a compile.sh carrying the CURRENT canonical flags (-mel -msoft-float, maspsx --prefill-label-funcs, multu_pad), and a target.o validated at build time to reproduce the in-tree residual register-for-register. tools/mar_perm_workspace.sh and tools/mar_perm_compile.sh are STALE (pre -mel, pre -msoft-float, pre --prefill-label-funcs) and must not be copied blind.

- [s4] [s4] Residual at 19 (tmp/grind/_spu_pitch2note/s4/a1_pairdiff.txt, 74/74 insns): the same 5 seats as s3 (pitch-copy $a3 vs $a0, shift $a0 vs $t2, lower $t3 vs $t4, acc $t2 vs $t3, outer $t4 vs $t2) plus three order inversions - (andi target / oct), (move acc,zero / sll base,outer,5, now only ONE insn apart at 33/34), and (addu upper-bound / addu lower-bound).

- [s4] [s4] Pre-loop order `target; scale; curve; oct;` survives a third loop-shape change (24 permutations re-measured on the a1 base, spread 19..23, unique minimum).

- [s4] [s4] Naming `base = outer << 5` is byte-neutral at 19 when it sits inside the for-init and costs a point outside it; the LICM hoist of `outer << 5` is not steerable from C on this chassis.

- [s4] [s4] src/main.c was restored to HEAD at the end of the session; the 19-floor body lives in memory/grind/_spu_pitch2note/candidate.c. Both campaigns were harvested and stopped in-session (tmp/perm_p2n 37,051 iters, tmp/perm_p2n2 8,845 iters, both pid dead, registry clean).

## s5 (2026-09-08, permuter modality) — MATCH.  Floor 19 -> 18 -> 7 -> 6 -> 4 -> 2 -> 0

- [s5] `sandbox _spu_pitch2note --disable all` = 0 (74/74 insns) AND
  `verify-oracle` = `build_matches: true` with the pure-C body in src/main.c.
  Final body: memory/grind/_spu_pitch2note/candidate.c (= tmp/grind/_spu_pitch2note/s5/b_E4.c).
- [s5] THE UNLOCK: the target holds the scan-loop counter and the outer-loop
  counter in the SAME register ($t2: `li t2,15` / `addiu t2,t2,-1` in the scan
  loop, `move t2,zero` at the outer-loop init).  Spelling that in C — ONE
  counter variable driving both loops — collapsed the entire 5-seat residual
  that four sessions had recorded as the immovable core (pitch-copy $a3 vs $a0,
  shift $a0 vs $t2, lower $t3 vs $t4, acc $t2 vs $t3, outer $t4 vs $t2): after
  m2 every register in the function matches and only order inversions remain.
  Floor 19 -> 7 in one edit.  The DECLARATION SLOT matters: keeping the outer
  counter's slot and deleting the scan counter's (m2) is 7; the mirror (m1,
  keeping the scan slot) is 19.
- [s5] The pointer to it was a permuter find, not a hypothesis: campaign s5-x7
  (tmp/perm_p2n_x7/output-570-3) named the scan test's shifted value into the
  outer counter (`outer = search >> shift; if (!(outer & 1))`) — legal, because
  the outer counter is dead before its own loop — and measured 18.  Hand-vetted
  as k1 on four chassis (a1/x1/x7/x8): 18 on all four.  A FRESH local for the
  same intermediate (k2) is 19, `result` (k6) 19, `lower` (k7) 20, `inner`/`acc`
  (k5/k8) 33/32 — so the point belonged to the outer counter's QUANTITY, which
  is what made the shared-counter hypothesis obvious on the pairdiff.
- [s5] Every order sweep this project had banked was invalidated by the new
  loop shape and had to be re-run — and all three flipped:
    * pre-loop statement order (q00..q23 on m2, range 6..8): `oct` must come
      FIRST now (q04/q18/q20/q21 = 6).  The `target; scale; curve; oct;` order
      that had won three consecutive sessions measures 7 here.
    * `target = pitch` in the outer for-init (t1) = 4 vs 6 as a plain statement.
      s4 had measured the same move at 20 vs 19 (i.e. a LOSS) on the old shape.
    * the (addu upper / addu lower) inversion, recorded in s2/s3/s4 as immune to
      source order, is fixed by splitting BOTH bounds with the low bound's add
      first (L6) = 2.  s3 had measured the lower-bound split (r2a) as a loss.
- [s5] The last inversion (`move acc,zero` before the LICM-hoisted
  `sll base,outer,5`), which s4 priced as the cheapest remaining residual and
  measured NOT steerable, IS steerable on the m2 shape: naming the base index in
  the inner for-init (C1, `base = outer << 5` first after `inner = 0`) = 0.  The
  identical construct on the s4 chassis was byte-neutral at 19.  The lesson the
  ledger should carry: "not steerable" verdicts are chassis-relative in the
  strongest sense — this one was false one chassis later.
- [s5] Simplest-known-form cleanups, all measured byte-neutral at 0 (so the
  simplest lands, per .claude/rules/ordinary-c-judge-decidable.md Ruling 4):
  the shared counter renamed `i` (E2), `base = i * 32` for `i << 5` (E3), and
  the `target` copy dropped entirely in favour of comparing `pitch` directly
  (E4).  `target = pitch` as a plain pre-loop statement is 2 (E1), so the copy
  had to be in the outer for-init or absent.
- [s5] Campaign telemetry: six campaigns, all harvested and stopped in-session.
  Chassis seeds x8/x7/x5 (17,693 / 17,655 / 17,643 iters), then k1a1/k1x8/k1x7
  on the 18-floor chassis, then m2 (13,871) and L6 (7,171, --stop-on-zero).  The
  useful product was again the POPULATION, not the gradient: the two x8 finds
  that scored 18 are semantically broken (they clobber the live outer counter as
  a bound carrier) and are banked in rejected/; the ONE find that mattered
  (x7 output-570-3) had permuter score 570 out of a 365..575 spread — i.e. near
  the WORST of its own campaign, re-confirming s4's kill of permuter-score
  steering on this function.
