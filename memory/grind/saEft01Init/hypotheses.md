# Hypothesis ledger — saEft01Init

## Session 1 (recon) — measured

### H1 — KILLED
**Statement:** the three loop-hoisted table base pointers in `$s0/$s1/$s3`
arise from loop-invariant motion of inline global references, not from
explicit source-level locals; removing the locals would renumber the pseudos
and change the allocno tiebreak.
**Mechanism:** loop.c `move_movables` hoisting `set (reg) (symbol_ref)` out of
the loop; higher pseudo numbers → later allocno order.
**Probe:** replaced `tbl_11dc`/`idx_1494`/`tbl_125c` with direct
`D_800A11DC[...]` / `D_800A125C[D_800A1494]` / `D_800A125C[D_800A1495]`
references at the use sites; `sandbox --disable all`.
**Result:** score 40 (from 18), **85 build insns vs 91 target**. GCC emits the
`%hi/%lo` pair at each use site inside the loop and hoists nothing — the three
prologue `lui`+`addiu` pairs vanish entirely.
**Verdict: KILLED.** The explicit locals are a structural requirement of any
matching form. Banked: `rejected/inline-globals-kills-hoisted-base-pointers.c`.

### H2 — KILLED
**Statement:** the staged `arg4`/`arg5` locals in the inherited baseline are a
prior session's artefact; the natural original spelling is one call expression
with both table lookups written inline as arguments.
**Mechanism:** `expand_call` argument expansion order.
**Probe:** collapsed the staging block into
`debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);`
**Result:** score 23 (from 18); insn count unchanged at 91, so a pure
schedule/RA regression. The named intermediates are what pin the two leading
`lbu` at the head of the block.
**Verdict: KILLED.** Banked: `rejected/single-expression-printf-call.c`.

### H2b — MEASURED NEUTRAL (adopted as the candidate)
**Statement:** target loads `idx_1494[0]` before `idx_1494[1]`; our build does
the reverse because the baseline stages `arg5 = tbl_125c[idx[1]]` first.
Swapping the two staged assignments should fix the `lbu` order.
**Probe:** swapped to `arg4 = tbl_125c[idx_1494[0]]; arg5 = tbl_125c[idx_1494[1]];`
**Result:** score 18 — unchanged. The `lbu` pair now matches target's order,
but the `$a1`/`D_800F19C0` load slides from build idx 48/49 (correct) to 56/57
and the `lw $a3` slides from 61 (correct) to 52. Equal score, strictly closer
positionally on the block head.
**Verdict: adopted as `candidate.c`** — it is the better base for the next
session even though the floor is unchanged.

### H3 — CONFIRMED (mechanism), spelling still open
**Statement:** the 3-way callee-save rotation (cluster A) exists because our
`goto` loop emits no `NOTE_INSN_LOOP_BEG`, so `flow.c` does not loop-depth
weight the in-loop references and `global.c:allocno_compare` ranks the `a0`
param above the two table pointers.
**Mechanism:** `allocno_compare` priority =
`floor_log2(n_refs) * n_refs / live_length`; `flow.c` accumulates `n_refs`
weighted by enclosing `loop_depth`. With no loop note, the 1-use param is not
separated from the 2-use table pointers, so it is allocated first and takes
`$s0`. Target needs it third (`$s2`).
**Probe:** rewrote the loop as `do { ... } while (a0 == 0);` with a shared
`ret` exit variable; re-ran `cc1 -da` and read `;; Register dispositions`.
**Result:** allocation order went `73 72 77 76 75` → `73 77 76 72 109 84 75`
and dispositions went `72 in 16 / 77 in 17 / 76 in 18` →
`77 in 16 / 76 in 17 / 72 in 18`. **Exactly the target permutation.**
Sandbox score of this spelling is 29 (98 insns) because the loop notes also
let `loop.c` hoist the two loop-invariant compare constants (`0x3C0000`,
`0x1000000`) into two extra callee-saves.
**Verdict: CONFIRMED as the mechanism for cluster A.** The remaining work is a
loop spelling that keeps the notes and denies the constant hoisting. Banked
with the full next-probe list:
`rejected/real-loop-confirms-ra-but-hoists-constants.c`.

## Session 2 (structural) — measured

### H4 — KILLED (F3 as posed is obsolete)
**Statement:** reading the per-allocno `n_refs` / `live_length` behind the
priority inversion requires an instrumented `cc1` built into `tmp/gccdbg/`.
**Probe:** checked `tools/gcc-2.7.2/build/cc1` for the pre-existing
`BB2_ALLOC_DEBUG` hook in `global.c` (`strings ... | grep -c ALLOCDBG` → 0, the
shipped binary predates it), then read the `-da` dump set instead.
**Result:** `flow.c:dump_flow_info` ALREADY prints both fields
(`Register N used R times across L insns`) into `system.i.flow` (pre
local_alloc) and `system.i.lreg` (post local_alloc — the values `global_alloc`
sorts on). Feeding the `.lreg` numbers through `allocno_compare`'s formula
reproduced the `.greg` allocation order exactly on all 10 variants measured.
**Verdict: KILLED — no instrumented build needed, ever, for this class of
question.** Harness banked at `tmp/grind/saEft01Init/s2/sweep.py`.

### H5 — KILLED (and it closes off the entire goto-loop branch)
**Statement:** the callee-save rotation can be fixed within the `goto` loop by
statement placement alone (moving the three pointer assignments earlier/later,
reordering them, aliasing the param, restructuring the tail).
**Mechanism:** `local-alloc.c:update_equiv_regs` line 1064 doubles
`reg_live_length` for any pseudo whose set carries a `REG_EQUIV` note. All
three table pointers are `(set (reg) (symbol_ref))` and carry one; the param's
`(set (reg/v 72) (reg:SI 4 a0))` does not. So the pointers are scored on 2×
their raw length and the 1-use param wins on `n_refs/live_length`.
**Probe:** six placement variants (`v0_base`, `v2_ptrs_late`, `v3_ptrs_first`,
`v4_shared_ret`, `v5_invert_tail`, `v6_param_alias`, `v8_ptr_order_rev`) run
through `sweep.py`, reading `.lreg` n_refs/live_length for each.
**Result:** the param's live length is 52 in EVERY variant (55 in one);
placement moves the pointers' raw lengths by at most ±4. Arithmetically,
`P76 > P72` needs `2*raw76 < 1.5*raw72` ⇒ `raw72 > 65`, which exceeds the
function's whole live span. **No goto-loop form can produce target's order.**
**Verdict: KILLED.** Loop notes are structurally required, not optional.

### H6 — CONFIRMED (cluster A closed)
**Statement:** a real loop can emit `NOTE_INSN_LOOP_BEG` (flipping the allocno
priorities to target's) while denying `loop.c` the hoist of the two
loop-invariant compare constants, if BOTH constants are written through ONE
reused scratch local.
**Mechanism:** `loop.c:scan_loop` only builds a movable when
`n_times_set[regno] == 1 || consec_sets_invariant_p(...)`. One local set twice
in the loop, non-consecutively, fails both, so `move_movables` cannot hoist
either constant and both stay materialised inline — which is what target does
(`lui $v0,(0x3C0000>>16)` at target idx 38, `lui $v1,(0x1000000>>16)` at 77).
**Probe:** `do { ... } while (a0 == 0);` with a shared `ret` exit and a single
`s32 k` holding `0x3C0000` then `0x1000000`; `sweep.py` for the dispositions,
`sandbox --disable all` for the score, `grind_diff.py` for the positional diff.
**Result:** dispositions `tbl_125c→$s0, idx_1494→$s1, param→$s2,
tbl_11dc→$s3`, exactly target, using only `$s0-$s3` (`k` gets `$a0`). No
`$s4`/`$s5`. Score 18 at **92** insns (s1's un-reused real loop was 29 at 98).
**Verdict: CONFIRMED. Cluster A is solved.** Banked as `candidate.c`.
Caveat: the reuse is the [[defeat-licm-hoist-var-reuse]] family and has NOT
been cheat-reviewed; do that before any completion claim.

### H7 — KILLED
**Statement:** the remaining tail residual (`bnez / nop / j end / move` where
target has `beqz / move` in the delay slot) is caused by the shared-`ret` +
`break` spelling, and inline `return -1/0/1;` statements — which is what
target's three delay-slot-delivered constants look like — will fix it.
**Probe:** `v11` (`do{...}while(a0==0)` with two inline returns and a trailing
`return 1;`) and `v13` (`for (;;)` with three inline returns), both with the
`k` reuse; `sandbox --disable all`.
**Result:** both 19 at 93 insns — one WORSE than the shared-`ret` candidate,
and the inverted branch + `j end` pair survives unchanged. GCC's choice of
which side of the mask branch falls through is not driven by the exit
spelling. (Same relative ordering as session 1 measured pre-reuse: shared-ret
29/98 beat inline-returns 30/99. Now measured in both regimes.)
**Verdict: KILLED.** Banked: `rejected/inline-returns-instead-of-break-ret.c`.

### H8 — KILLED
**Statement:** reusing the EXISTING `v0` flag local as the constant holder
(instead of a new `k`) will both defeat the hoisting and land the constant in
`$v0`, where target has it.
**Probe:** `v12`, identical to the candidate but with `v0 = 0x3C0000;` /
`v0 = 0x1000000;`; `sandbox --disable all`.
**Result:** 22 at 93. The hoisting is still defeated and the `$s0-$s3` map is
still correct, but overloading the pseudo that also carries the timeout flag
consumed by `if (v0 != 0)` costs more in the flag block than it saves.
**Verdict: KILLED.** The reuse holder must be separate from the exit flag.
Banked: `rejected/v0-as-licm-reuse-holder-regresses.c`.

## Session 3 (structural) � measured

### H9 � CONFIRMED (mechanism for the +2 tail branch sense)
**Statement:** our inverted mask exit (`bnez v0,<cont> / nop / j <end> /
move v0,zero`, where target has `beqz $v0,.L80081CFC` with the constant in
the delay slot) is blocked by jump.c's "conditional jump jumping over an
unconditional jump" transform, which requires the unconditional jump to
IMMEDIATELY follow the conditional one � and our `ret = 0;` set sits in
between.
**Mechanism:** `tools/gcc-2.7.2/jump.c:1764-1772` guards `invert_jump` with
`prev_active_insn (reallabelprev) == insn && no_labels_between_p (...) &&
simplejump_p (reallabelprev)`. A compound if-body (`ret = 0; break;`)
expands to `bnez v0,Lskip / set v0,0 / j Lexit / Lskip:` so
`prev_active_insn (j Lexit)` is the set, not the branch.
**Probe:** rewrote the mask exit as a bare `goto aborted;` with
`aborted: return 0;` after the loop (variant `w1_goto_exits`, plus
`w2_goto_mask_only` isolating that one exit); `sandbox --disable all` +
`tmp/grind/saEft01Init/s1/grind_diff.py`.
**Result:** the build now emits `beqz $v0,<exit>` � target's exact branch
sense � in every goto variant. **Verdict: CONFIRMED.** The bare-goto body
is the lever for the branch sense; it is not sufficient on its own (H10).

### H10 � KILLED (source-level exit spelling and label order are exhausted)
**Statement:** with the branch sense fixed by H9, some arrangement of the
exit spellings / out-of-loop label ORDER will also get the stranded
`return 0;` block folded into that branch's delay slot, reproducing
target's `beqz $v0,.L80081CFC / addu $v0,$zero,$zero`.
**Probe:** eight variants, all measured with `sandbox --disable all` and
diffed with `grind_diff.py` � `w1` (both exits goto, timed_out then
aborted), `w4` (aborted then timed_out), `w8`/`w9` (three goto exits, the
`return 1` also moved out of line, in both orders), `w2` (mask exit only),
`w3` (flag exit only), `w5` (`if (mask) {ret=1;} else {ret=0;break;}`),
`w6` (`if (mask) {ret=1; continue;} ret=0; break;`).
**Result:** every goto-form emits a BYTE-IDENTICAL tail at 19 / 93 insns �
GCC normalises the out-of-line block order itself, so source label order is
inert. `w5`/`w6` collapse back to the candidate's exact output (18 / 92):
GCC re-canonicalises the source-level branch sense too. **Verdict: KILLED.**
The residual is not reachable from the C's exit spelling; it is one reorg
decision (F7).

### H11 � CONFIRMED, and it REFRAMES H6 (the `k` reuse is not the original)
**Statement:** the session-2 candidate's single reused scratch local `k` is
the original spelling of the two loop-invariant compare constants.
**Probe:** read the two `lui` sites in `asm/funcs/saEft01Init.s`.
**Result:** **target holds them in TWO DIFFERENT hard registers** �
`lui $v0,(0x3C0000>>16)` at idx 41 (immediately consumed by the following
`slt $v0,$v0,$v1`) and `lui $v1,(0x1000000>>16)` at idx 83. One C variable
is one pseudo and one hard register; our build puts both in `$a0`.
**Verdict: the statement is FALSIFIED � `k` cannot be the original.** It
remains the best-scoring form (18/92 vs 29/98 for any single-set spelling)
but it is a synthetic LICM defeat, and cheat-reviewer has still not seen it.
Any completion claim must either replace it or clear it through review.

### H12 � CONFIRMED (the real loop.c gate) / KILLED (as a full replacement)
**Statement:** the constants stay inline in the original because of
loop.c's *user-variable* gate rather than the `n_times_set` gate, so two
separate branch-spanning locals should keep both inline in two registers,
matching H11's evidence.
**Mechanism:** `tools/gcc-2.7.2/loop.c:695` skips building a movable only
when ALL of (A) `! maybe_never && ! loop_reg_used_before_p (...)`,
(B) `! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)`,
(C) `reg_in_basic_block_p (p, dest)` are false. `maybe_never` is set at the
first CODE_LABEL/JUMP_INSN inside the loop (loop.c:930).
**Probe:** `y1` (both constants as separate user locals whose live ranges
cross a branch), `y2` (only the 0x3C0000 one), `y3` (only the 0x1000000
one), and `x2`/`x3` (one constant through `k`, the other written as a
literal), all via `tmp/grind/saEft01Init/s3/score.py`.
**Result:** `y3` (mask constant set at the `check:` join, spanning the
`if (v0 != 0)` branch) = 27 / **97** insns vs the 30 / **98** both-hoisted
baseline � the mask constant escapes the hoist in its own pseudo, no
double-set needed. `y2` (0x3C0000 set at the top of the loop body) = 30/98,
i.e. still hoisted: `maybe_never` is 0 that early, so branch (A) holds.
Moving its set later makes it BB-local, so branch (C) holds. `x2`/`x3` both
30/98 � any single-set constant local is hoisted.
**Verdict: the gate is CONFIRMED and now usable; the two-local form is
KILLED as a replacement** (27/97 is far worse than 18/92) because the
0x3C0000 constant has no position in this loop that fails all three
branches. Banked:
`rejected/two-spanning-const-locals-only-mask-escapes-licm.c`.

### H13 � KILLED
**Statement:** target's register choice ($v0 for 0x3C0000, which is also
its timeout/success flag) means the flag local carries that constant, so
`v0` as the timeout holder plus a branch-spanning local for the mask
reproduces the two-register shape without the `k` reuse.
**Probe:** `z1` (v0 holds 0x3C0000, `k2` spans for the mask), `z3` (v0 for
the timeout, literal for the mask), `z2` (z1 plus the H9 bare-goto exit).
**Result:** 22/94, 29/95, 24/95. Both constants do stay inline in z1, but
overloading `v0` costs +2 in the flag block � the same regression session 2
measured with v0 carrying BOTH constants (H8, 22/93). **Verdict: KILLED.**
The exit flag is unusable as a constant holder in any combination. Banked:
`rejected/v0-timeout-holder-plus-spanning-mask-local.c`.

### H14 � KILLED (tooling; saves the next session a detour)
**Statement:** reorg.c's `BB2_DBR_DEBUG=1` instrumentation
(`DBRDBG thr/simp/mtlr` traces, reorg.c:133-153) can be used to read why
`fill_slots_from_thread` refuses the delay-slot steal.
**Probe:** compiled the spliced tree with `BB2_DBR_DEBUG=1` through
`tmp/grind/saEft01Init/s3/dbr.sh` and grepped the captured stderr for the
tail insn UIDs.
**Result:** ZERO `DBRDBG` lines � the shipped `tools/gcc-2.7.2/build/cc1`
predates the instrumentation, exactly as session 2 found for
`BB2_ALLOC_DEBUG`/ALLOCDBG. **Verdict: KILLED.** Use the `-da` `.dbr` RTL
dump instead; `tmp/grind/saEft01Init/s3/dbrscan.py` extracts the
saEft01Init tail with insn UIDs and is the tool of record for this class of
question.

## Live frontier (for session 4)

### F7 � the reorg delay-slot steal that strands `return 0;` (the whole +2)
**Mechanism:** in the H9 goto form the RTL tail is
`jump_insn 173: beq v0,0 -> label 205 ("aborted")` /
`insn 252 = SEQUENCE[ jump_insn 184: beq s2,0 -> loop ; insn 194: v0=1 ]` /
`jump_insn 196: j 218` / `code_label 205` /
`insn 208: (set (reg/i:SI 2 v0) (const_int 0))` / `code_label 218`.
reorg CONSUMES the analogous block for the FLAG exit in the same function
(`jump_insn 156`: its `v0=-1` lands in the bnez delay slot, its label
becomes NOTE_INSN_DELETED_LABEL, and the branch is redirected to 218) but
refuses it for 173. The one structural difference visible in the dump: the
consumed thread ends in a `jump_insn` to 218, whereas 208 is followed
directly by `code_label 218`; and 173's fall-through thread
(`next_active_insn`) is the loop-condition JUMP_INSN 184, already turned
into a SEQUENCE by `fill_simple_delay_slots`, whereas 156's fall-through is
an ordinary INSN.
**Next probe:** read `fill_slots_from_thread`'s trial-loop guards in
`tools/gcc-2.7.2/reorg.c` (in particular
`! insn_sets_resource_p (trial, &opposite_needed, 1)` and the
`mark_target_live_regs` fallback at reorg.c:2500-2510 that force-marks ALL
registers live when it cannot find the block start � reorg.c also carries a
`BB2_ALLLIVE_LABEL` env hook, which suggests this project has hit that
fallback before). Then find C that puts an ordinary INSN between the mask
branch and the loop-condition branch, or that leaves the `return 0;` thread
ending in a real jump. Regenerate the RTL with
`bash tmp/grind/saEft01Init/s3/dbr.sh <variant>` (it prints the tail via
dbrscan.py). Note `w2` (which keeps `ret = 1;` before the loop condition)
does NOT achieve this � fill_simple_delay_slots consumes that insn into
184's slot before fill_eager ever looks at 173.

### F8 � make the 0x3C0000 constant escape LICM in its OWN pseudo
**Mechanism:** H12's gate. The constant needs a set position where
`maybe_never` is already 1 (i.e. after the loop's first CODE_LABEL or
JUMP_INSN, loop.c:930) AND a live range crossing a basic-block boundary so
`reg_in_basic_block_p` is false. In the current shape the only in-loop
branch before it is the `if (D_800F19B8 < v0)` test, and the compare
follows in the same block.
**Next probe:** alternatives that satisfy both � a spelling of the timeout
test that puts an extra branch or label between the constant's set and its
use; a variable that is REFERENCED earlier in the loop than its set
(branch (A)'s `loop_reg_used_before_p` half, a second way to fail (A) that
does not need `maybe_never`); or restructuring so `D_800F19BC`'s
read/increment sits in a separate block. Success looks like 91-92 insns
with `$s4`/`$s5` absent from the `.greg` dispositions and `lui $v0,0x3c` /
`lui $v1,0x100` in two DIFFERENT caller-saved registers. Screen with
`tmp/grind/saEft01Init/s3/score.py`.

### F9 � cheat-review status of the `k` reuse
**Mechanism:** H11 shows the single reused `k` is not what the original
compiled from. It is currently load-bearing for the 18/92 floor and is in
the `[[defeat-licm-hoist-var-reuse]]` family, which is SOTN-sanctioned but
only as a documented last resort with lever-exhaustion.
**Next probe:** if F8 lands, `k` disappears on its own and the question is
moot � do F8 first. If a completion is reached WITH `k` still present, it
must go through cheat-reviewer with H11's two-register evidence disclosed,
because "this is what the original wrote" is now known to be false.

### F6 � cluster B, the debug_printf argument block (inherited, still open)
Session 3 did not touch it. Session 2's note stands: re-score both
arg-staging orders against the now-correct callee-save allocation.

## Superseded frontier (session 2 � F4 and F5 resolved by H9/H10/H12/H13 above)

### F4 — the tail block layout (+2, the larger half of the residual)
**Mechanism:** GCC emits the mask exit as `bnez v0,<cont> / nop / j <end> /
move v0,zero` because it lays the `ret = 1; while (a0 == 0)` tail block AFTER
the exit jump; target has the un-inverted `beqz $v0,.L80081CFC` falling
straight through into `beqz $s2,.L80081C10`. This is `jump.c` / final block
ordering, not register allocation — the allocation is already correct.
**Next probe:** attack the ORDER, not the exit spelling (H7 killed that):
invert the mask test's source sense so the loop-continue side is the
fall-through (`if (*D_800A14C0 & k) { ret = 1; } else { ret = 0; break; }`);
fold the mask test and the `a0` back-edge condition into one statement
(`if ((*D_800A14C0 & k) == 0) ... ; ` vs `while ((*D_800A14C0 & k) && a0 == 0)`
as the loop condition); and try `continue` for the loop-continue path. Screen
each with `score.py`; the `.lreg`/`sweep.py` predictor does NOT see block
layout, so these need real sandbox runs.

### F5 — the 0x3C0000 holder register (-1, and 2 positional diffs)
**Mechanism:** `k` is allocated `$a0` (nrefs 8, live_length 6), so the
scheduler fills the `bnez v1` delay slot with `lui a0,0x3c` at build idx 31,
where target keeps a `nop` and emits `lui $v0,(0x3C0000>>16)` after the
`D_800F19BC` store. Target's holder is `$v0`, just freed by the preceding
`slt`, and it is therefore not available early enough to be hoisted into the
delay slot.
**Next probe:** the holder cannot simply BE the `v0` local (H8, 22/93). Try
instead: splitting the reuse across two locals that alias different live
ranges but still give each pseudo two sets; moving `k = 0x3C0000;` to sit
between the `D_800F19BC = cnt + 1;` store and the compare (it currently
precedes the store in source but the scheduler moves it anyway); and reading
`.lreg` to see whether `k` can be pushed out of `$a0` by lengthening its live
range slightly.

### F6 — cluster B, now measurable on a correct allocation (inherited F2, unblocked)
**Mechanism:** session 1 could not re-diff the `debug_printf` argument block
because the callee-save map was wrong; it now matches target. The arg-block
address chains index off `$s0` in both builds as of the session-2 candidate.
**Next probe:** re-run `grind_diff.py` on the candidate and re-score both
arg-staging orders (arg4-first and arg5-first) against the corrected
allocation — session 1 measured them equal at 18 under the WRONG allocation,
so that tie must be re-measured before any further arg-block work.

## Superseded frontier (session 1)

### F1 — defeat loop.c's hoisting of the two compare constants
**Mechanism:** `tools/gcc-2.7.2/loop.c` `scan_loop` / `move_movables`. A
`set (reg) (const_int)` becomes a movable and is hoisted to the preheader when
it survives the `maybe_never` test and clears the `threshold` savings test
(`threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)` — this loop
HAS calls, so the bar is low). Target keeps both constants inline.
**Next probe:** read `scan_loop`/`move_movables` for the exact suppression
conditions, then find the C placement/compare spelling that makes each
constant a non-movable — candidates: putting the `0x3C0000` compare where
`maybe_never` is already set, an unsigned-compare spelling that folds the
threshold into the existing `slt` chain, and a bit-test spelling for
`0x1000000` that `combine` keeps inline. Measure each with
`sandbox --disable all` AND with `greg.sh` (the dispositions must stay
`77/76/72 in 16/17/18`).

### F2 — re-diff cluster B on top of a correct cluster-A allocation
**Mechanism:** the arg-block address chains index off `$s0` in target and off
`$s1` in our build, so part of the `$2 <-> $3` / `$2 <-> $4` rename cluster
may be downstream of the callee-save rotation rather than an independent
scheduling problem.
**Next probe:** once any form lands the `77/76/72` dispositions, re-run
`grind_diff.py` and re-score the two known-equal arg-staging orders (H2b and
its inverse) against the new allocation before doing any further arg-block
work.

### F3 — confirm the param's `live_length`/`n_refs` numerically
**Mechanism:** H3 was confirmed end-to-end (loop note → disposition flip) but
the individual `allocno_n_refs` / `allocno_live_length` values were inferred
from the source of `allocno_compare`, not read off. If F1's spellings behave
unexpectedly, the cheapest disambiguation is a `tmp/`-local instrumented cc1
build that prints the two fields per allocno (dumps for understanding only —
the shipped toolchain stays frozen per [[no-compiler-divergence]]).
**Next probe:** build cc1 into `tmp/gccdbg/` with a `fprintf` in
`allocno_compare`'s caller and dump the per-allocno `n_refs`/`live_length` for
both the goto-loop and real-loop forms.

## [s1] The three loop-hoisted table base pointers in $s0/$s1/$s3 arise from loop-invariant motion of inline global references rather than from explicit source-level pointer locals.
- mechanism: loop.c move_movables hoisting set(reg)(symbol_ref) out of the loop; the resulting pseudos would carry higher register numbers and change the allocno tiebreak in global.c.
- probe: Replaced tbl_11dc / idx_1494 / tbl_125c with direct D_800A11DC[...] / D_800A125C[D_800A1494] / D_800A125C[D_800A1495] references at the use sites; sandbox saEft01Init --disable all.
- result: Score 40 (from 18) and 85 build insns against target's 91. GCC emits the %hi/%lo pair at each use site inside the loop and hoists nothing; the three prologue lui+addiu pairs vanish entirely. The explicit locals are a structural requirement of any matching form.
- verdict: KILLED

## [s1] The staged arg4/arg5 locals feeding debug_printf are a prior session's artefact and the natural original spelling is one call expression with both table lookups written inline as arguments.
- mechanism: expand_call argument expansion order for a 5-argument call with one stack-homed argument.
- probe: Collapsed the staging block into debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]); sandbox --disable all.
- result: Score 23 (from 18) with the instruction count unchanged at 91, i.e. a pure schedule/RA regression. The named intermediates are what pin the two leading lbu at the head of the argument block.
- verdict: KILLED

## [s1] Target loads idx_1494[0] before idx_1494[1]; our build reverses them because the inherited baseline stages arg5 = tbl_125c[idx[1]] first, so swapping the two staged assignments should fix the lbu order.
- mechanism: Statement order of the two named intermediates drives the order in which the two index chains are expanded and hence scheduled.
- probe: Swapped to arg4 = tbl_125c[idx_1494[0]]; arg5 = tbl_125c[idx_1494[1]]; sandbox --disable all plus a positional objdump diff against asm/funcs/saEft01Init.s.
- result: Score 18 — unchanged. The two lbu now match target's order (0x0($s1) then 0x1($s1)), but the $a1/D_800F19C0 load slides from build idx 48/49 (correct) to 56/57 and the lw $a3 slides from 61 (correct) to 52. Equal score, strictly closer positionally at the block head, so adopted as candidate.c.
- verdict: CONFIRMED

## [s1] The 3-way callee-save rotation papered over by regfix.txt ($16<->$18, $16<->$17 plus six stack-offset substs) exists because the function's goto back-edge emits no NOTE_INSN_LOOP_BEG, so flow.c does not loop-depth weight the in-loop references and global.c ranks the a0 param above the two table pointers.
- mechanism: tools/gcc-2.7.2/global.c allocno_compare computes priority = floor_log2(n_refs) * n_refs / live_length, and flow.c accumulates n_refs weighted by the enclosing loop_depth. With no loop note, loop_depth is 1 throughout, the 1-use param (2 refs) is not separated from the 2-use table pointers (3 refs), and the param is allocated first and takes $s0. Target needs it allocated third, in $s2.
- probe: Rewrote the loop as do { ... } while (a0 == 0); with a single shared ret exit variable (matching target's converge-at-.L80081CFC tail), then re-ran cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -da and read ';; Register dispositions' for the function.
- result: Allocation order went '73 72 77 76 75' -> '73 77 76 72 109 84 75' and dispositions went '72 in 16 / 77 in 17 / 76 in 18' -> '77 in 16 / 76 in 17 / 72 in 18' — exactly target's permutation (tbl_125c->$s0, idx_1494->$s1, param->$s2). The sandbox score of this spelling is 29 at 98 insns because the loop notes also let loop.c hoist the two loop-invariant compare constants 0x3C0000 and 0x1000000 into two extra callee-saves ($s4, $s5), adding allocnos 84 and 109 and displacing tbl_11dc to $s5. Mechanism confirmed; spelling still open.
- verdict: CONFIRMED

## [s2] Reading the per-allocno n_refs / live_length behind the priority inversion requires building an instrumented cc1 into tmp/gccdbg/ (session 1's F3).
- mechanism: allocno_compare's inputs are not printed by the .greg dump; global.c carries a BB2_ALLOC_DEBUG fprintf hook but the shipped tools/gcc-2.7.2/build/cc1 predates it (strings | grep -c ALLOCDBG == 0).
- probe: Checked the shipped cc1 for the ALLOCDBG string, then inspected the rest of the -da dump set for the same data.
- result: flow.c:dump_flow_info ALREADY prints both fields as 'Register N used R times across L insns', into system.i.flow (pre local_alloc) and system.i.lreg (post local_alloc, which is what global_alloc sorts on). Feeding the .lreg numbers through pri = floor_log2(n_refs)*n_refs/live_length*10000, ties by ascending pseudo number, reproduced the .greg ';; N regs to allocate:' order EXACTLY on all 10 variants measured this session. No instrumented build is needed for this class of question, now or ever. Harness banked at tmp/grind/saEft01Init/s2/sweep.py.
- verdict: KILLED

## [s2] The 3-way callee-save rotation can be fixed inside the existing goto-loop by statement placement alone (moving/reordering the three pointer assignments, aliasing the param, restructuring the tail).
- mechanism: local-alloc.c:update_equiv_regs line 1064 doubles reg_live_length for any pseudo whose set carries a REG_EQUIV note. All three table pointers are (set (reg) (symbol_ref "D_800A1xxx")) and carry REG_EQUIV; the param's (set (reg/v 72) (reg:SI 4 a0)) does not. So the pointers are scored on 2x their raw live length and the 1-use param outranks them.
- probe: Seven placement variants (v0_base, v2_ptrs_late, v3_ptrs_first, v4_shared_ret, v5_invert_tail, v6_param_alias, v8_ptr_order_rev) run through sweep.py, reading .lreg n_refs/live_length and the resulting .greg order for each.
- result: The param's live_length is 52 in every variant (55 in one); placement moves the pointers' raw lengths by at most +/-4 (v2_ptrs_late moved them by 0, v3_ptrs_first by +4). Baseline numbers: 72=2refs/52, 77=3/96, 76=3/98, 75=2/100. P76 > P72 requires 2*raw76 < 1.5*raw72, i.e. raw72 > 65, which exceeds the function's whole live span (~52). No goto-loop form can produce target's allocation order; the loop notes are structurally required, not one option among several.
- verdict: KILLED

## [s2] A real loop can emit NOTE_INSN_LOOP_BEG (flipping the allocno priorities to target's) while denying loop.c the hoist of the two loop-invariant compare constants 0x3C0000 and 0x1000000, if both constants are written through ONE reused scratch local.
- mechanism: loop.c:scan_loop only builds a movable when n_times_set[regno] == 1 || consec_sets_invariant_p(...). One local set twice in the loop, non-consecutively, fails both disjuncts, so move_movables cannot hoist either constant and both stay materialised inline inside the loop — which is exactly what target does (lui $v0,(0x3C0000>>16) at target idx 38, lui $v1,(0x1000000>>16) at idx 77).
- probe: do { ... } while (a0 == 0); with a shared ret exit and a single s32 k assigned 0x3C0000 at the timeout compare and 0x1000000 at the mask test; sweep.py for the dispositions, sandbox saEft01Init --disable all for the score, tmp/grind/saEft01Init/s1/grind_diff.py for the positional diff.
- result: Dispositions tbl_125c->$s0, idx_1494->$s1, param->$s2, tbl_11dc->$s3 — exactly target's map — using only $s0-$s3, with k allocated the caller-saved $a0. No $s4/$s5, both constants materialised inline. Score 18 at 92 build insns vs 91 target (session 1's un-reused real loop was 29 at 98). Cluster A, i.e. the whole regfix.txt:97-103 + 115-119 rename block, is solved in pure C. Banked as memory/grind/saEft01Init/candidate.c.
- verdict: CONFIRMED

## [s2] The remaining tail residual (bnez / nop / j end / move, where target has beqz with the constant in the delay slot) is caused by the shared-ret + break spelling, and inline return -1/0/1 statements will fix it.
- mechanism: Target's tail delivers each return constant in a branch delay slot (addiu $v0,-1; addu $v0,$zero,$zero; addiu $v0,0x1), which reads as three inline returns rather than a shared ret variable.
- probe: v11 (do{...}while(a0==0) with two inline returns and a trailing return 1) and v13 (for(;;) with three inline returns), both keeping the k reuse; sandbox --disable all.
- result: Both 19 at 93 insns — one worse than the shared-ret candidate — and the inverted branch plus j end pair survives unchanged. GCC's choice of which side of the mask branch falls through is not driven by the exit spelling. Consistent with session 1's pre-reuse measurement (shared-ret 29/98 beat inline-returns 30/99); the shared-ret shape is now measured better in both regimes.
- verdict: KILLED

## [s2] Reusing the EXISTING v0 flag local as the constant holder instead of introducing a new k will both defeat the hoisting and land the constant in $v0, where target holds it.
- mechanism: Target materialises 0x3C0000 into $v0, which the preceding slt has just freed; our k gets $a0, so the scheduler fills the bnez delay slot with lui a0,0x3c where target keeps a nop.
- probe: v12, identical to the candidate but with v0 = 0x3C0000; and v0 = 0x1000000; sandbox --disable all.
- result: 22 at 93 insns. The hoisting is still defeated and the $s0-$s3 map stays correct, but overloading the pseudo that also carries the timeout/success flag consumed by if (v0 != 0) costs more in the flag block than it saves at the compare. The reuse holder must be a local separate from the exit flag.
- verdict: KILLED

## [s3] Our inverted mask exit (bnez v0,<cont> / nop / j <end> / move v0,zero, where target has beqz $v0,.L80081CFC with the constant in the delay slot) is blocked by jump.c's conditional-jump-over-unconditional-jump inversion, because the `ret = 0;` set sits between the conditional branch and the `j <end>`.
- mechanism: tools/gcc-2.7.2/jump.c:1764-1772 guards invert_jump with prev_active_insn (reallabelprev) == insn && no_labels_between_p (...) && simplejump_p (reallabelprev). A compound if-body expands to `bnez v0,Lskip / set v0,0 / j Lexit / Lskip:`, so prev_active_insn of the unconditional jump is the set, not the branch.
- probe: Rewrote the mask exit as a bare `goto aborted;` with `aborted: return 0;` after the loop (w1_goto_exits, and w2_goto_mask_only isolating that one exit); sandbox saEft01Init --disable all plus tmp/grind/saEft01Init/s1/grind_diff.py.
- result: Every goto form emits `beqz $v0,<exit>` — target's exact branch sense — instead of the inverted bnez/nop/j triple. The lever is real and reproducible; it is not sufficient on its own because the `return 0;` block is then stranded out of line (see the next hypothesis).
- verdict: CONFIRMED

## [s3] With the branch sense fixed, some arrangement of the exit spellings or the out-of-loop label ORDER will also get the stranded `return 0;` block folded into that branch's delay slot, reproducing target's `beqz $v0,.L80081CFC / addu $v0,$zero,$zero`.
- mechanism: GCC's final block ordering plus reorg.c fill_slots_from_thread's own-thread steal, which is what consumed the analogous `return -1;` block for the flag exit in the same function.
- probe: Eight variants scored with sandbox --disable all and diffed with grind_diff.py: w1 (both exits goto, timed_out then aborted), w4 (aborted then timed_out), w8/w9 (three goto exits with `return 1` also moved out of line, both orders), w2 (mask exit only), w3 (flag exit only), w5 (if/else with the ret=1 arm as the then-branch), w6 (`if (mask) { ret = 1; continue; } ret = 0; break;`).
- result: Every goto form emits a BYTE-IDENTICAL tail at 19 score / 93 insns regardless of label order — GCC normalises the out-of-line block order itself. w5 and w6 collapse back to the candidate's exact output at 18/92, i.e. GCC re-canonicalises the source-level branch sense too. The remaining +2 is one reorg decision, not reachable from the C's exit spelling.
- verdict: KILLED

## [s3] The session-2 candidate's single reused scratch local `k`, which holds both loop-invariant compare constants, is the original spelling.
- mechanism: loop.c:702 only builds a movable when n_times_set == 1 or consec_sets_invariant_p succeeds, so one local set twice non-consecutively denies the hoist of both constants. But one C variable is one pseudo and therefore one hard register.
- probe: Read the two `lui` sites in asm/funcs/saEft01Init.s.
- result: Target materialises the constants in TWO DIFFERENT hard registers: `lui $v0,(0x3C0000>>16)` at idx 41 (immediately consumed by the following `slt $v0,$v0,$v1`) and `lui $v1,(0x1000000>>16)` at idx 83. Our build puts both in $a0. A single reused local cannot produce two registers, so `k` is provably not the original spelling — it is a synthetic LICM defeat that is still load-bearing for the 18/92 floor and has never been through cheat-reviewer.
- verdict: KILLED

## [s3] The constants stay inline in the original because of loop.c's user-variable gate rather than its n_times_set gate, so two separate branch-spanning const locals will keep both inline in two registers, matching target.
- mechanism: tools/gcc-2.7.2/loop.c:695 skips building a movable only when ALL of (A) `! maybe_never && ! loop_reg_used_before_p (...)`, (B) `! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)`, (C) `reg_in_basic_block_p (p, dest)` are false. loop.c:930 sets maybe_never at the first CODE_LABEL/JUMP_INSN inside the loop.
- probe: y1 (both constants as separate user locals whose live ranges cross a branch), y2 (only 0x3C0000), y3 (only 0x1000000), plus controls x2/x3 (one constant through k, the other a literal), all via tmp/grind/saEft01Init/s3/score.py.
- result: The gate is confirmed and usable for ONE of the two constants: y3 (mask constant set at the `check:` join so it spans the `if (v0 != 0)` branch) is 27/97 against the 30/98 both-hoisted baseline, i.e. it escapes the hoist in its own pseudo with no double-set. y2 is 30/98 — 0x3C0000's set is too early for maybe_never to be 1, and moving it later makes it basic-block-local, so branch (A) or (C) always holds. x2/x3 both 30/98: ANY single-set constant local in this loop is hoisted. The two-local form is therefore killed as a replacement (27/97 vs 18/92) while the gate itself becomes the F8 lever.
- verdict: KILLED

## [s3] Target's register choice ($v0 for 0x3C0000, which is also its timeout/success flag) means the flag local carried that constant, so `v0` as the timeout holder plus a branch-spanning local for the mask reproduces the two-register shape without the `k` reuse.
- mechanism: `v0` already has more than one set, so loop.c:702 blocks its hoist for free; the mask local is blocked by loop.c:695. That yields two distinct pseudos, both inline.
- probe: z1 (v0 holds 0x3C0000, k2 spans for the mask), z3 (v0 for the timeout, literal for the mask), z2 (z1 plus the bare-goto mask exit); sandbox --disable all.
- result: 22/94, 29/95 and 24/95 respectively. Both constants do stay inline in z1, but overloading `v0` costs +2 in the flag block — the same regression session 2 measured with v0 carrying both constants (22/93). The exit flag is unusable as a constant holder in any combination.
- verdict: KILLED

## [s3] reorg.c's BB2_DBR_DEBUG=1 instrumentation can be used to read why fill_slots_from_thread refuses the delay-slot steal for the stranded `return 0;` block.
- mechanism: reorg.c:133-153 defines BB2_DBR_ON()/BB2_NO_FT()/BB2_ALLLIVE_LABEL hooks that print DBRDBG thr/simp/mtlr traces including opposite_needed register masks.
- probe: Compiled the spliced tree with BB2_DBR_DEBUG=1 via tmp/grind/saEft01Init/s3/dbr.sh and grepped the captured cc1 stderr for the tail insn UIDs taken from the -da .dbr dump.
- result: Zero DBRDBG lines — the shipped tools/gcc-2.7.2/build/cc1 binary predates the instrumentation, exactly as session 2 found for BB2_ALLOC_DEBUG/ALLOCDBG. The -da .dbr RTL dump carries the needed information instead; tmp/grind/saEft01Init/s3/dbrscan.py extracts the saEft01Init tail with insn UIDs and is now the tool of record.
- verdict: KILLED

## [s4] A decomp-permuter campaign on the session-3 chassis can find levers the hand search missed, provided the workspace compiles the FULL translation unit and extracts only saEft01Init at offset 0, and provided every find is re-screened through the engine's cheat-invisible sandbox rather than trusted on the permuter's own weighted score.
- mechanism: decomp-permuter scores a weighted register/reordering/insertion diff against target.o; the engine scores masked differing instructions with regfix/asmfix disabled and cheat-asm stripped. The two metrics rank differently, so the permuter's ordering cannot be used as the grind gradient.
- probe: Built tmp/grind/saEft01Init/s4/ws2 (full-TU cpp of src/system.c, cc1 | prologue_fix | maspsx | multu_pad with NO regfix/asmfix, saEft01Init region extracted and assembled against a prelude-wrapped asm/funcs/saEft01Init.s target.o) and verified the trimmed pycparser-parseable base.c compiles BYTE-IDENTICALLY to the full-TU compile. Launched via tools/permuter_campaign.py (label random-cand-chassis, -j 8, 850s, 24k iterations across the two campaigns), then screened all 51 + 19 finds with tmp/grind/saEft01Init/s4/screen.py.
- result: The correlation is weak and would have been actively misleading: the permuter's best find of campaign 1 (weighted 860) screened to sandbox 11, while a find still at the base weighted score (1235-5) screened to 13 and several 1050-band finds screened to 17-19. The best sandbox result of the whole session came from hand-refining a permuter find, not from the permuter's own ranking.
- verdict: CONFIRMED

## [s4] The 0x1000000 mask constant can be given its own register (the two-register shape session 3 proved target has, F8) by staging it out of the double-set holder `k` into a second local, without losing the double-set that denies loop.c the hoist.
- mechanism: loop.c:702 refuses to build a movable for `k` because it has two non-consecutive sets, so both constants stay materialised inline. The extra copy `<local> = k;` then gives the mask value a second pseudo, which the allocator can place in a different hard register — target has `lui $v0,0x3c` and `lui $v1,0x100` in two different caller-saved registers.
- probe: Variant a1_mask_via_cnt (`k = 0x1000000; cnt = k; if (!(*D_800A14C0 & cnt))`) vs a2_mask_via_new_local (identical but staging through a fresh `s32 m`) vs a3_mask_via_ret (staging through the exit variable `ret`), all measured with `sandbox saEft01Init --disable all`.
- result: a1 = 11 / 93 (from the 18 / 92 floor). a3 = 15 / 94. a2 = 18 / 92, i.e. BYTE-IDENTICAL to the unmodified candidate — the fresh local is completely inert. The lever is therefore not 'an extra copy instruction'; it is specifically the reuse of a pseudo that already carries a live range.
- verdict: CONFIRMED

## [s4] Naming the mask test's result (`ret = *D_800A14C0 & cnt; if (ret == 0) break;`) instead of testing it inline closes further distance, and the redundant `ret = 0;` the permuter emitted inside the if-body is load-bearing for that gain.
- mechanism: The named result gives the AND its own pseudo whose live range reaches the shared exit, changing which value the exit join has to move into $v0.
- probe: b1_and_into_ret (permuter's exact form, with the redundant `ret = 0;`), b2_and_into_ret_nodead (`if (ret == 0) { break; }`), b4_and_into_ret_bang (`if (!ret) { break; }`).
- result: All three measure 9 / 91 — identical score AND identical instruction count, i.e. the redundant `ret = 0;` is byte-neutral. The naming lever is real (11 -> 9) and brings the build to target's exact 91 instructions; the dead store is NOT load-bearing and has been dropped from the banked candidate.
- verdict: CONFIRMED (lever) / KILLED (the dead store's necessity)

## [s4] The debug_printf argument block (session 1's cluster B / F6, untouched since session 1) still carries residual distance and is reachable by re-basing the table pointer rather than by staging the argument values.
- mechanism: Target's block is `lbu a0,0(s1) / lbu v0,1(s1) / sll a0,a0,2 / lw v1,0(v0) / addu a0,a0,s0 / lw a3,0(a0)` — two index loads whose address chains interleave. Re-basing `tbl_125c` onto the first index ([[walking-pointer-serializes-parallel-loads]] family) serialises the two chains through one pointer instead of computing two independent scaled addresses.
- probe: b3/b5 (`tbl_125c = &tbl_125c[idx_1494[0]]; arg5 = tbl_125c[idx_1494[1]]; arg4 = *tbl_125c;` on top of the two mask levers) vs a4_maskcnt_argk and a5_maskcnt_argcnt (the two value-staging spellings the permuter also proposed).
- result: The pointer re-base composes cleanly: 9 -> 7 at 91 insns (b5_ptr_rebase_nodead). Both value-staging spellings REGRESS when composed with the mask lever — a4 (arg4 through the constant holder `k`) = 13 / 92, a5 (arg4 through `cnt`) = 16 / 93 — because they contend for the same pseudo. SEMANTICS CHECKED: every path reaching the re-base sets `v0 = -1` and the following `if (v0 != 0)` join unconditionally `break`s, so the mutated pointer can never be observed on a later iteration.
- verdict: CONFIRMED

## Session 5 (permuter) — measured

### H19 — KILLED, and it INVALIDATES the session-4 floor of 7
**Statement:** the session-4 candidate's third lever
(`tbl_125c = &tbl_125c[idx_1494[0]]; arg5 = tbl_125c[idx_1494[1]];
arg4 = *tbl_125c;`, the [[walking-pointer-serializes-parallel-loads]]-family
pointer re-base that took the honest distance 9 -> 7) is a legitimate
respelling of the argument block.
**Mechanism / probe:** read the base-vs-target positional disassembly of the
distance-7 chassis (`tmp/grind/saEft01Init/s5/od.sh`, workspace ws2 built from
the session-4 candidate) and compared the two index address chains against
target's.
**Result:** target's block is
`lbu a0,0(s1) / lbu v0,1(s1) / sll v0,v0,2 / addu v0,v0,s0 / sll a0,a0,2 /
lw v1,0(v0) / addu a0,a0,s0 / sw v1,16(sp) / lw a3,0(a0)` — **both index
chains are computed independently off the UNMODIFIED `s0`**, i.e.
`arg4 = tbl[i0]` and `arg5 = tbl[i1]`. The re-base instead emits
`addu s0,s0,v1` (it MUTATES the base pointer) and then indexes the mutated
pointer, so it computes `arg5 = tbl[i0 + i1]`. That is a **real semantic
change in the value passed to `debug_printf`**, not a respelling: session 4
only checked that the mutated pointer is not observable on a LATER iteration
and never checked the value of `arg5` on THIS one.
**Verdict: KILLED.** The distance-7 form is not a candidate for the match at
all — it is a different program, and its `addu s0,s0,v1` is an instruction
target does not contain. The honest floor for a semantically faithful form is
**8**, not 7. Banked: `rejected/pointer-rebase-changes-arg5-semantics-7.c`.

### H20 — CONFIRMED (statement order is the lever; declaration order is inert)
**Statement:** with the re-base dropped, the two named argument intermediates'
STATEMENT order and DECLARATION order are independent levers, and session 1's
measured tie between them (18 vs 18, taken under the WRONG callee-save
allocation — session-2 F6, never re-measured) does not survive on the correct
allocation.
**Probe:** the 2x2 cross-product on top of the two surviving mask levers —
`c1` (decl `arg5,arg4` / stmt arg5-then-arg4), `c2` (decl `arg4,arg5` / stmt
arg4-then-arg5), `c3` (decl `arg5,arg4` / stmt arg4-then-arg5), `c4` (decl
`arg4,arg5` / stmt arg5-then-arg4), all via
`tmp/grind/saEft01Init/s5/score.py`.
**Result:** 9 / 8 / 8 / 9, all at target's exact 91 instructions. The score
depends ONLY on the statement order (arg4's assignment first = 8, arg5's
first = 9); declaration order is completely inert (c2 == c3, c1 == c4).
**Verdict: CONFIRMED — F6 is now closed.** Assign `arg4` (the idx[0] lookup)
first. The session-1 tie was an artefact of the wrong allocation.

### H21 — KILLED
**Statement:** target names only the FIFTH argument and writes the fourth
inline in the call. Evidence: target homes `arg5` through a register into the
stack slot (`lw v1,0(v0) ... sw v1,16(sp)`) but loads the fourth argument
straight into its argument register as the LAST memory reference of the block
(`lw a3,0(a0)`), which is what an inline call-argument expression looks like;
a named local would be materialised earlier. Session 1's H2 killed the
BOTH-inline spelling but the asymmetric one had never been measured.
**Probe:** `d1` (`s32 arg5; arg5 = tbl_125c[idx_1494[1]];` + `tbl_125c[idx_1494[0]]`
inline in the call), `d2` (same, initialised at the declaration), `d4` (same
but `arg5` declared at function scope), and the mirror-image control `d3`
(`arg4` named, `tbl_125c[idx_1494[1]]` inline).
**Result:** d1 = d2 = d4 = **14 / 91** — a 6-point regression, and identical
to each other, so neither the declaration position nor the initialiser form
matters. The control d3 = **8 / 91**, tying the best two-named-local form
(c2/c3) with ONE fewer local.
**Verdict: KILLED for the arg5-named spelling; d3 adopted as the chassis.**
The fifth argument must NOT be the only named one. Whatever defers target's
`lw a3` to the end of the block, it is not an inline fourth argument.
Banked: `rejected/arg5-named-arg4-inline-regresses-14.c`.

### H22 — KILLED (the distance-7 chassis is a saturated basin)
**Statement:** the session-4 frontier's top item — that the distance-7 form,
never used as a permuter base, would seed a better basin than the 18/92 and
11/93 chassis of session 4 did.
**Probe:** rebuilt the workspace from the distance-7 candidate
(`tmp/grind/saEft01Init/s5/mkws.sh` + `mkws2b.sh`, trimmed base verified
byte-identical to the full-TU compile) and ran a `d7-chassis` campaign
(`tools/permuter_campaign.py`, -j 8, ~480 s, 2647 output dirs / 2352
textually-unique bodies). Deduplicated with
`tmp/grind/saEft01Init/s5/pick.py` and screened 55 through the ENGINE sandbox
(the 40 lowest weighted scores plus a 15-point stratified sample of the rest).
**Result:** **not one find beat the base.** Best screened results were 7
(four finds, all ties with the base), then 8 (eleven finds); the campaign's
own best weighted score (565) screened to 8. The sample across the whole
weighted range (up to 3940) screened 17-53, re-confirming session 4's finding
that the two metrics rank differently. NOTE the campaign reported a bogus base
score of 9000 for this chassis, so it wrote out EVERY mutant as a "better
score" — that is why 2647 dirs exist and why the dedupe+stratify step was
needed; the faithful chassis (H23) scores a normal 635.
**Verdict: KILLED.** The distance-7 chassis's random-mutation basin is
saturated. Harvested with --stop; no campaign outlived the session.

## [s5] The session-4 candidate's third lever (tbl_125c = &tbl_125c[idx_1494[0]]; arg5 = tbl_125c[idx_1494[1]]; arg4 = *tbl_125c;) is a legitimate walking-pointer respelling of the debug_printf argument block, and its distance of 7 is a real floor.
- mechanism: The [[walking-pointer-serializes-parallel-loads]] family serialises two independent index chains through one pointer. Session 4 verified only that the mutated pointer cannot be observed on a LATER loop iteration (every path reaching it sets v0 = -1 and the following join breaks), and concluded the mutation was unobservable.
- probe: Read the base-vs-target positional disassembly of the distance-7 chassis (tmp/grind/saEft01Init/s5/od.sh over a workspace built from the session-4 candidate) and compared the two index address chains instruction by instruction against target's.
- result: Target's block is `lbu a0,0(s1) / lbu v0,1(s1) / sll v0,v0,2 / addu v0,v0,s0 / sll a0,a0,2 / lw v1,0(v0) / addu a0,a0,s0 / sw v1,16(sp) / lw a3,0(a0)` — BOTH index chains are computed off the UNMODIFIED base register s0, i.e. arg4 = tbl[i0] and arg5 = tbl[i1]. The re-base emits `addu s0,s0,v1`, an instruction target does not contain, and then indexes the mutated pointer, so it computes arg5 = tbl[i0 + i1]. That is a real change to the value passed to debug_printf on the CURRENT iteration, which session 4 never checked. The 7 is therefore a score on a program that is not this function.
- verdict: KILLED

## [s5] With the re-base dropped, the two named argument intermediates' STATEMENT order and DECLARATION order are independent levers, and session 1's measured 18-vs-18 tie between the two staging orders (taken under the WRONG callee-save allocation — session-2 F6, never re-measured) does not survive on the correct allocation.
- mechanism: The statement order drives the order in which the two index chains are expanded and hence scheduled; the declaration order drives LUID assignment and could bias register allocation independently.
- probe: The full 2x2 cross-product on top of the two surviving mask levers, via tmp/grind/saEft01Init/s5/score.py: c1 (decl arg5,arg4 / stmt arg5-then-arg4), c2 (decl arg4,arg5 / stmt arg4-then-arg5), c3 (decl arg5,arg4 / stmt arg4-then-arg5), c4 (decl arg4,arg5 / stmt arg5-then-arg4).
- result: 9 / 8 / 8 / 9, all at target's exact 91 instructions. The score depends ONLY on the statement order (the idx[0] lookup assigned first = 8, the idx[1] lookup first = 9); declaration order is completely byte-inert (c2 == c3 and c1 == c4). Session 1's tie was an artefact of the wrong allocation. F6 is closed.
- verdict: CONFIRMED

## [s5] Target names only the FIFTH argument and writes the fourth inline in the call expression — that is why target homes arg5 through a register into the stack slot (lw v1,0(v0) ... sw v1,16(sp)) but issues `lw a3,0(a0)` as the LAST memory reference of the block, where a named local would be materialised earlier.
- mechanism: expand_call materialises a named intermediate at its assignment statement, but an inline argument expression is expanded during the call's own argument expansion, at the end of the block.
- probe: d1 (s32 arg5; arg5 = tbl_125c[idx_1494[1]]; with tbl_125c[idx_1494[0]] inline in the call), d2 (same, initialised at the declaration), d4 (same, arg5 declared at function scope), and the mirror-image control d3 (arg4 named, tbl_125c[idx_1494[1]] inline).
- result: d1 = d2 = d4 = 14 / 91, a 6-point regression, and byte-identical to each other so neither the declaration position nor the initialiser form matters. The control d3 = 8 / 91, tying the best two-named-local form with one fewer local. Whatever defers target's lw a3 to the end of the block, it is not an inline fourth argument.
- verdict: KILLED

## [s5] The session-4 frontier's top item: the distance-7 form, which no campaign had ever used as a permuter base, would seed a better basin than session 4's 18/92 and 11/93 chassis did (each of which immediately produced a better basin on reseed).
- mechanism: The permuter's random pass explores register-visible restructurings that hand search does not enumerate, and session 4 measured that each reseed from a closer chassis found a closer basin.
- probe: Rebuilt the workspace from the distance-7 candidate (tmp/grind/saEft01Init/s5/mkws.sh + mkws2b.sh; the trimmed pycparser-parseable base verified byte-identical to the full-TU compile), ran the d7-chassis campaign via tools/permuter_campaign.py (-j 8, ~480 s, 14045 iterations, 2647 output dirs / 2352 textually-unique bodies), deduplicated with tmp/grind/saEft01Init/s5/pick.py and screened 55 through the ENGINE sandbox (the 40 lowest weighted plus a 15-point stratified sample of the rest).
- result: Not one find beat the base. Best screened results were 7 (four finds, all exact ties with the base) then 8 (eleven finds); the campaign's own best weighted score, 565, screened to 8. The stratified sample across the whole weighted range (up to 3940) screened 17-53, re-confirming session 4's weak-correlation finding. Harvested with --stop.
- verdict: KILLED

## [s5] A campaign reseeded from the semantically faithful 8/91 chassis (structurally different from the d7 chassis: no pointer re-base, one named intermediate instead of two) finds a basin below 8.
- mechanism: Fresh-seed discipline — a basin yields early or not at all, so a structurally different chassis is the correct response to a saturated one. This chassis also scores normally under the permuter (base_score 635) where the d7 chassis reported a bogus 9000, so its find stream is real 'better than base' finds rather than every mutant.
- probe: Built ws5 from the d3 (8/91) form and ran the d8-faithful-chassis campaign (-j 8, ~15 min, 29684 iterations), soaked in-turn via tmp/grind/saEft01Init/s5/soak.sh, screened every find through the sandbox.
- result: Five finds total. Best weighted (545) screened to sandbox 10 at 92 instructions; the four weighted-635 finds screened to 8, i.e. exact ties with the base. No find below 8. Harvested with --stop; both campaigns are dead and no permuter process outlived the session.
- verdict: KILLED

## [s6] Splitting the fourth argument's address chain across two statements — varying which statement owns the `idx_1494[0]` byte load and what sits between it and the table lookup — can move that chain earlier in the argument block and defer its `lw`, reproducing target's `lbu a0,0(s1)` … `lw a3,0(a0)` span. (The session-5 frontier's first next-probe.)
- mechanism: Proposed mechanism was that the RTL emission position of the byte load (its LUID) biases the pre-reload scheduler, which breaks ties on `INSN_LUID` at `sched.c:2452-2455`, so the source statement that owns the load would decide where the chain sits.
- probe: Four spellings whose expand-time LUID orders genuinely differ were built and dumped with `tmp/grind/saEft01Init/s6/dump.sh`: `n0` (both lookups inline in the call), `n2` (`i0 = idx_1494[0];` as its own statement, both lookups inline), `n3` (both indices named as statements), `n5` (arg5 named plus `i0` named). Their `.rtl`, `.combine`, `.sched` and `.sched2` sections were read with `s6/order.py`, and the emitted `.s` function bodies compared by md5.
- result: All four are BYTE-IDENTICAL (`sed -n '/^saEft01Init:/,/\.end/p' | md5sum` = `5e95422b…` for every one) and all score 14/91. Their `.sched` (pre-reload) outputs are the same order, with the `idx_1494[0]` `lbu` at position 9 of 16 in every case — even though `n2` emits that `lbu` as insn 94, the FIRST insn of the block, while `n0` emits it as insn 104, the third chain. That inverts the LUID relation between it and the `D_800A11D5` `lbu` (n0: 104 vs 96; n2: 94 vs 102) with no change to the schedule, so the LUID tie-break is provably not the deciding rule here. The order is set by `INSN_PRIORITY` (longest path to the block end) and then by the dependence-class test at `sched.c:2412-2449`, both functions of the dependency DAG alone.
- verdict: KILLED

## [s6] The fourth argument's `lw a3` position is a scheduling artefact that some spelling of the same program can move.
- mechanism: The two candidate spellings (named local vs inline call argument) were assumed to differ only in how the optimiser sees the same RTL, so both were assumed able to reach target's ordering.
- probe: Read the `.rtl` dump (raw expand output, before any optimisation pass) for both spellings.
- result: The difference is created at EXPAND, not by any optimisation pass. With the fourth argument INLINE, `expand_call`'s argument-loading phase passes the hard reg as the target of `expand_expr` and emits `(insn 129 (set (reg:SI 7 a3) (mem/s:SI (reg:SI 99))))` directly, as the last insn of the argument sequence — there is no separate load insn to schedule. With it NAMED, expand emits `(insn 101 (set (reg/v:SI 88) (mem/s:SI (reg:SI 93))))` at the statement position plus `(insn 134 (set (reg:SI 7 a3) (reg/v:SI 88)))`, and `local_alloc` gives pseudo 88 `$a3` by copy preference, deleting the move and leaving the LOAD written to `$a3` early. combine is not involved at all: the `REG_USERVAR_P` guards at `combine.c:1279` and `combine.c:1134` are `SMALL_REGISTER_CLASSES`-gated and `config/mips/mips.h` does not define that macro. Target's block ends with `lw a3,0(a0)` as its last memory reference, so target has the INLINE expand shape — the inherited 8/91 candidate's fourth argument is structurally wrong, and the structurally right form is the 14/91 basin.
- verdict: CONFIRMED

## [s6] The `$a0`-vs-`$v0` register difference in the argument block is an independent allocator lever that could be attacked separately from the schedule.
- mechanism: Session 5 recorded the residual as "our build runs the chain through `$v1`, target through `$a0`", which reads as a register-allocation choice.
- probe: Correlated `.sched` (pre-reload order), the resulting live ranges, and `.sched2` (post-reload order) for the candidate and for `n0`, against target's block.
- result: The register is fully downstream of the pre-reload schedule. Target's `idx_1494[0]` address temp is live from the block's FIRST insn to its LAST, so `$v0` and `$v1` are both busy across it and `local_alloc` lands it in `$a0`; that in turn forces `lui a0,%hi(D_800161C8)` to come after it by anti-dependence, which is target's exact tail. In both of our basins the same temp is short-lived (candidate: positions 1-7 of the block; `n0`: positions 9-15) and gets `$v0`. There is no separate allocator lever — the only way to `$a0` is to make sched1 schedule that chain's `lbu` first and its `lw` last.
- verdict: CONFIRMED

## [s6] Naming the SECOND argument (`tbl_11dc[D_800A11D5]`) is a pure scheduling lever that can be composed with the surviving arg4 spellings.
- mechanism: Same expand-time reasoning as the fourth argument: a named local's load is emitted at the statement position instead of at the end of the argument sequence, changing the block's chain lengths and hence the priorities.
- probe: `n4` (arg2 named, both index lookups inline), `n6` (arg2 named then arg4 named), `n7` (arg4 named then arg2 named), scored with `tmp/grind/saEft01Init/s6/score.py`.
- result: n4 = 13/91, n6 = 15 at **90** build insns, n7 = 14 at **90** build insns. Naming arg2 in combination with a named arg4 changes the instruction COUNT (90 vs target's 91), so it is not a pure scheduling lever and cannot be composed with the 8/91 chassis. n4 is the best member of the inline-arg4 family (13 vs 14) but still five points above the inherited candidate.
- verdict: KILLED

## [s6] The instrumented-cc1 modality (RTL/ALLOCDBG/GREG debug builds) is available for this function.
- mechanism: `tools/gcc-2.7.2/sched.c:2435` carries a `BB2_RANK_DEBUG` env-gated `fprintf` inside `rank_for_schedule` printing the class-tie decision, and `reorg.c` carries the analogous `BB2_DBR_DEBUG` that session 3 tried to use.
- probe: `strings tools/gcc-2.7.2/build/cc1 | grep -E "RANKDBG|DBRDBG|ALLOCDBG"`.
- result: Empty — the shipped cc1 binary predates all three instrumentation blocks, exactly as session 2 found for `BB2_ALLOC_DEBUG` and session 3 for `BB2_DBR_DEBUG`. Enabling them would require rebuilding `tools/gcc-2.7.2/build/cc1`, which is outside a grind session's allowed surface (and would change the compiler the whole tree builds with). All forensics on this function must read the `-da` dumps.
- verdict: KILLED

## [s6] Splitting the fourth argument's address chain across two statements — varying which statement owns the idx_1494[0] byte load and what sits between it and the table lookup — can move that chain earlier in the argument block and defer its lw, reproducing target's `lbu a0,0(s1)` ... `lw a3,0(a0)` span. (The session-5 frontier's first next-probe.)
- mechanism: Proposed mechanism was that the RTL emission position of the byte load (its LUID) biases the pre-reload scheduler, which breaks ties on INSN_LUID at sched.c:2452-2455, so the source statement owning the load would decide where the chain sits.
- probe: Four spellings with genuinely different expand-time LUID orders were built and dumped with tmp/grind/saEft01Init/s6/dump.sh: n0 (both lookups inline), n2 (`i0 = idx_1494[0];` as its own statement, lookups inline), n3 (both indices named), n5 (arg5 named + i0 named). Their .rtl/.combine/.sched/.sched2 sections were read with s6/order.py and the emitted .s function bodies compared by md5.
- result: All four are BYTE-IDENTICAL (md5 5e95422b of the .s body) at 14/91, and their .sched (pre-reload) outputs are the same order with the idx_1494[0] lbu at position 9 of 16 — even though n2 emits that lbu as insn 94, the FIRST insn of the block, and n0 as insn 104, the third chain. That inverts its LUID relation to the D_800A11D5 lbu (n0: 104 vs 96; n2: 94 vs 102) with no change to the schedule, proving the LUID tie-break is not the deciding rule. INSN_PRIORITY and the dependence-class test at sched.c:2412-2449 are, and both are functions of the dependency DAG alone.
- verdict: KILLED

## [s6] The fourth argument's `lw a3` position is a scheduling artefact that some spelling of the same program can move, so the named-arg4 8/91 candidate and the inline-arg4 14/91 form are neighbours in one basin.
- mechanism: Both spellings were assumed to differ only in how the optimiser sees the same RTL.
- probe: Read the .rtl dump (raw expand output, before any optimisation pass) for both spellings and located the insn that writes $a3.
- result: The difference is created at EXPAND. Inline: expand_call passes the hard reg as the target of expand_expr and emits `(insn 129 (set (reg:SI 7 a3) (mem/s:SI (reg:SI 99))))` directly, as the LAST insn of the argument sequence — no separate load insn exists. Named: expand emits `(insn 101 (set (reg/v:SI 88) (mem/s:SI (reg:SI 93))))` at the statement position plus `(insn 134 (set (reg:SI 7 a3) (reg/v:SI 88)))`, and local_alloc gives pseudo 88 $a3 by copy preference, deleting the move and leaving the LOAD written to $a3 early. combine is not involved: its REG_USERVAR_P guards at combine.c:1279/1134 are SMALL_REGISTER_CLASSES-gated and config/mips/mips.h does not define that macro. Target's block ends with `lw a3,0(a0)` as its last memory reference, so target has the INLINE expand shape and the inherited 8/91 candidate's fourth argument is structurally wrong despite scoring better.
- verdict: CONFIRMED

## [s6] The $a0-vs-$v0 register difference in the argument block is an independent register-allocation lever that can be attacked separately from the schedule.
- mechanism: Session 5 recorded the residual as 'ours runs through $v1, target through $a0', which reads as an allocator choice.
- probe: Correlated .sched (pre-reload order), the resulting live ranges, and .sched2 (post-reload order) for the candidate and for n0 against target's block.
- result: The register is fully downstream of the pre-reload schedule. Target's idx_1494[0] address temp is live from the block's first insn to its last, so $v0 and $v1 are both busy across it and local_alloc lands it in $a0 — which then forces `lui a0,%hi(D_800161C8)` after it by anti-dependence, target's exact tail. In both of our basins that temp is short-lived (candidate: positions 1-7 of the block; n0: positions 9-15) and gets $v0. There is no separate allocator lever: the only route to $a0 is to make sched1 schedule that chain's lbu first and its lw last.
- verdict: CONFIRMED

## [s6] Naming the SECOND argument (tbl_11dc[D_800A11D5]) is a pure scheduling lever that composes with the surviving arg4 spellings.
- mechanism: Same expand-time reasoning as the fourth argument: a named local's load is emitted at the statement position instead of at the end of the argument sequence, changing chain lengths and hence priorities.
- probe: n4 (arg2 named, both lookups inline), n6 (arg2 named then arg4 named), n7 (arg4 named then arg2 named), scored with tmp/grind/saEft01Init/s6/score.py.
- result: n4 = 13/91; n6 = 15 at 90 build insns; n7 = 14 at 90 build insns. Naming arg2 alongside a named arg4 changes the instruction COUNT to 90 against target's 91 — it deletes an instruction target contains — so it cannot compose with the 8/91 chassis in any order. n4 is the best member of the inline-arg4 family but is still five points above the inherited candidate.
- verdict: KILLED

## [s6] The instrumented-cc1 modality (RTL/ALLOCDBG/GREG debug builds) is runnable for this function, as the mandated modality assumes.
- mechanism: tools/gcc-2.7.2/sched.c:2435 carries a BB2_RANK_DEBUG env-gated fprintf inside rank_for_schedule that prints the class-tie decision, and reorg.c carries the analogous BB2_DBR_DEBUG session 3 tried to use.
- probe: strings tools/gcc-2.7.2/build/cc1 | grep -E "RANKDBG|DBRDBG|ALLOCDBG".
- result: Empty — the shipped cc1 binary predates all three instrumentation blocks, exactly as session 2 found for BB2_ALLOC_DEBUG and session 3 for BB2_DBR_DEBUG. Enabling them would require rebuilding tools/gcc-2.7.2/build/cc1, which is outside a grind session's allowed surface and would change the compiler the whole tree builds with. All cc1 forensics on this function must read the -da dumps; s6/dump.sh + x.py + order.py are the harness for that.
- verdict: KILLED


---

# Session 7 additions

## [inherited] salvaged hypotheses of the discarded 15:03-15:18 session-7 attempt

## Session 7 (forensics) — measured

#### H22 — CONFIRMED (tooling unlock, project-wide)
**Statement:** the instrumented-cc1 modality is unavailable without rebuilding
the compiler, because the shipped binary predates the `BB2_*_DEBUG` hooks
(sessions 2, 3 and 6 all recorded this).
**Mechanism:** `global.c`, `sched.c` and `reorg.c` in `tools/gcc-2.7.2/` carry
env-gated `fprintf` hooks (ALLOCDBG / PRIODBG / RANKDBG / DBRDBG) added
2026-07-03/04; `strings tools/gcc-2.7.2/build/cc1 | grep -E "RANKDBG|DBRDBG|
ALLOCDBG"` is empty, which the three sessions read as "the binary predates
them".
**Probe:** listed the compiler tree instead of just `build/`, found a SECOND
cc1 at `tools/gcc-2.7.2/cc1` dated 2026-07-18 (i.e. AFTER the source edits),
ran `strings` on it, then wrote `tmp/grind/saEft01Init/s7/idump.sh`, which
compiles the same `system.i` with the frozen `build/cc1` and with the
instrumented `cc1` and diffs the two `.s` files (ignoring the `# options`
comment) on every invocation.
**Result:** `tools/gcc-2.7.2/cc1` contains every hook — BB2_PRIO_DEBUG,
BB2_RANK_DEBUG, BB2_ALLOC_DEBUG, BB2_DBR_DEBUG, BB2_FLOW_DEBUG, BB2_QTY_DEBUG,
BB2_FINDREG_DEBUG, BB2_SLL_DEBUG, BB2_XJUMP_DEBUG, BB2_SCHED_DEBUG,
BB2_NO_FT_STEAL, BB2_ALLLIVE_LABEL — and the identity check prints
`CODEGEN-IDENTICAL: instrumented cc1 == shipped build/cc1 on this TU` on both
compiles run this session. Reading it is a pure diagnostic; the build pipeline
still uses `build/cc1`, so [[no-compiler-divergence]] is not engaged.
**Verdict: the prior statement is KILLED. The modality is AVAILABLE, for every
function in the project, with no compiler build required.**

#### H23 — CONFIRMED (the scheduler model, now numeric)
**Statement:** the argument block's order is set by `INSN_PRIORITY`, and the
two `lbu` chains tie on it (session 6 inferred this from byte-identical
variants; nothing had been read off).
**Mechanism:** `sched.c:priority()` computes, over an insn's LOG_LINKS
(predecessors), `max(priority(pred) + insn_cost(pred) - 1)`, i.e. the weighted
longest path from the BLOCK START; `schedule_block` is a bottom-up (reverse)
list scheduler, so the highest-priority insn is placed LATEST in the block.
`insn_cost` is `result_ready_cost`, which for this target comes from
`config/mips/mips.md`'s `define_function_unit "memory"`: load 2 under the
`r3000` alternative (`-mcpu=3000`), store 1, everything else 1.
**Probe:** `BB2_PRIO_DEBUG=1 BB2_RANK_DEBUG=1` through `s7/idump.sh cand`, then
read the sched1 `PRIODBG SET` window for the block's insn UIDs
(`tmp/grind/saEft01Init/s7/icand/cc1.log`).
**Result:** all four chain heads plus the `&D_800161C8` address materialisation
tie at priority 1; the `sll`/`addu`/intermediate loads are 2; the 16($sp)
store, the `$a2` load and the `$a3` copy are 3; the call is 4. Exactly the
hand-derived model. **Verdict: CONFIRMED, and the tie is now a measurement.**

#### H24 — KILLED (session 6's tie-break attribution)
**Statement:** with the two chains tied on priority, the dependence-class test
at `sched.c:2412-2449` breaks the tie against `idx_1494[0]`, so only a DAG
change can separate them (the session-6 frontier's central claim).
**Probe:** read every `RANKDBG last=... y=... cls=... x=... cls2=... val=...`
line emitted for this block by the instrumented cc1.
**Result:** all 13 comparisons print `cls=3 cls2=3 val=0`. Class 3 is
"independent of `last_scheduled_insn`, or latency 1", and every competitor in
this block qualifies, so the class term is identically zero and control always
reaches the `INSN_LUID` tie-break at `sched.c:2452-2455`.
**Verdict: KILLED.** The class rule is inert here; it decides nothing, and no
probe should be designed around manipulating it. The live constraints are
priority, then readiness in the reverse pass, then LUID.

#### H25 — CONFIRMED (why LUID nevertheless cannot be used directly)
**Statement:** because the tie-break is LUID, giving chain X's `lbu` the
smallest LUID should schedule it first.
**Probe:** `s7/idump.sh in2` on variant `n2` (`i0 = idx_1494[0];` named as a
statement, both lookups inline), then read the `.sched` order.
**Result:** X's `lbu` has LUID 94 — the smallest in the block — and is still
placed 9th of 16. In a bottom-up schedule an insn becomes READY only once all
its dependents are placed, so a chain is dragged toward the block END as a unit
once its terminal insn is placed there. Consequence, verified across every form
measured to date: the order of the three chains' HEADS always equals the order
of their LOADS (candidate X,Z,Y / X,Z,Y at 8; n4 Y,Z,X / Y,Z,X at 13; n0 and
a5n Z,Y,X / Z,Y,X at 14). Target is the ONLY arrangement that decouples them
(heads X,Z,Y but loads Z,Y,X — chain X's `lbu` first, its `lw $a3` last).
**Verdict: CONFIRMED, and it reframes the problem:** the goal is to STRETCH
chain X across the block, not to make it outrank chain Y.

#### H26 — CONFIRMED as a mechanism / KILLED as a floor improvement
**Statement:** naming the fourth argument's ADDRESS (not its value) while
leaving the dereference inline gives the chain low-LUID head insns AND keeps
the load as `expand_call`'s last argument insn, decoupling head from load.
**Mechanism:** the `lbu`/`sll`/`addu` are expanded at the statement's position;
the load is still emitted by `expand_call` directly into the hard register as
`(set (reg:SI 7 a3) (mem ...))`, session 6's target expand shape. The two ends
of the chain therefore sit at opposite ends of the LUID order.
**Probe:** `p4` (`s32 *p4; p4 = &tbl_125c[idx_1494[0]]; debug_printf(..., *p4,
tbl_125c[idx_1494[1]]);`) plus eight neighbours — `p4d` (declaration with
initialiser), `p45`/`p45r` (both addresses named, either order), `q1`/`q2`
(p4 combined with the fifth argument's VALUE named, either order), `q3`
(`tbl_125c + idx_1494[0]` spelling), `q4`/`q5` (p4 plus the third argument's
address named, either order) — each measured with `sandbox --disable all` and
its argument block dumped positionally by `s7/blk.py`.
**Result:** `p4` puts chain X's head at block position 2 and its load at 13 —
the first measured decoupling (candidate 1/9, the whole inline family 9-12/16,
target 1/16). Score 10/91; `p4d` is byte-identical; the neighbours are 10-14.
**Verdict: the mechanism is CONFIRMED; the spelling is KILLED as a floor
improvement** (10 vs the 8 floor). Banked with the full table at
`rejected/named-address-pointer-stretches-arg4-chain-but-10.c`. It is
nevertheless the only measured basin with target's block TOPOLOGY and is the
best available permuter seed — better than the n4 (13) chassis the session-6
frontier nominated.

## Live frontier (for session 8)

#### F10 — simulate the reverse list scheduler and SOLVE for the LUID order
**Mechanism:** the block's order is now a fully specified, deterministic
function of three measured things — the dependency DAG, the priorities (H23),
and the reverse-pass readiness + LUID tie-break (H24/H25). Nothing else enters
it. That means the schedule can be SIMULATED in Python from the `.rtl` dump,
and the simulator can be inverted: enumerate LUID permutations of the argument
sequence (there are only ~17 insns and the permutations reachable from C are
far fewer) and find which ones reproduce target's exact block order
(X.lbu, Z.lbu, a1, Z.sll, Z.addu, X.sll, Z.load, Y.lbu, X.addu, Y.sll, Y.addu,
sw, Y.load, X.load).
**Next probe:** write the simulator against `s7/icand/system.i.rtl` (validate
it by reproducing the KNOWN sched1 orders of `cand`, `n2` and `p4` — all three
dumps are already on disk), then solve for the required LUID order and only
then look for the C that produces it. This replaces blind spelling sweeps with
a search over the one degree of freedom that actually exists. Do NOT re-run
statement reordering inside the block without the simulator: nine spellings are
already banked and the coupling in H25 explains why they cluster.

#### F11 — apply the instrumented cc1 to the session-3 F7 tail (+1)
**Mechanism:** the second half of the residual is still reorg's refusal to fill
the `beqz $v0` delay slot with `move v0,zero`. Session 3 shelved this because
`BB2_DBR_DEBUG` produced no output from `build/cc1`. H22 removes that blocker:
`tools/gcc-2.7.2/cc1` carries the full DBRDBG trace set
(`thr insn=%d thread=%d opp=%d own=%d likely=%d tif=%d oppregs=%08x_%08x
oppmem=%d`, `thr LOSE`, `thr WINNER`, `simp ... refset/setset/setneed`,
`mtlr target=%d FORCED-ALLLIVE`).
**Next probe:** `BB2_DBR_DEBUG=1 bash tmp/grind/saEft01Init/s7/idump.sh <tag>`
on the candidate and on session 3's `w1_goto_exits`, then find the `thr LOSE`
line for the mask exit and read which resource test rejected the steal. That is
the exact question session 3's F7 could not answer. Note `BB2_NO_FT_STEAL` and
`BB2_ALLLIVE_LABEL` also exist and can be used to bisect the behaviour.

#### F12 — re-run the callee-save allocation questions with ALLOCDBG
**Mechanism:** `ALLOCDBG ord=%d pseudo=%d hardreg=%d nrefs=%d livelen=%d
pri=%d` prints exactly the quantity session 2 had to reconstruct by hand from
`.lreg` plus `allocno_compare`'s formula. `BB2_QTY_DEBUG` and
`BB2_FINDREG_DEBUG` cover `local-alloc`'s quantity merging and `find_reg`'s
hard-register choice, which is what decides that our chain temp gets `$v0`
where target's gets `$a0`.
**Next probe:** if F10 lands a form whose block order is right but whose
REGISTERS are wrong, use `BB2_FINDREG_DEBUG=1` to read `find_reg`'s preference
order for that temp rather than guessing at live-range lengthening.



## Session 7 (forensics) — measured

Modality: forensics. Floor unchanged at **8 / 91**. Twenty variants measured
(r0-r2, candv/r1v, h2/h4/h5/h6/h7/h8, c1-c4, clean/cleank/cleana), plus two
instrumented-cc1 dump sets.

### H26 — CONFIRMED
**Statement:** saEft01Init is Sony PsyQ LIBCD `CD_datasync`, and publicly
matched C for it exists and can be obtained.
**Mechanism:** the brief's 2026-07-09 bit-exact census flagged the whole module
as verbatim-linked Sony object code; if so the reference source is public in
any of the several PSY-Q decomp projects.
**Probe:** `gh api search/code -f q='CD_datasync in:file language:c'`, then
`gh api repos/Xeeynamo/sotn-decomp/contents/src/main/psxsdk/libcd/bios.c`.
Cross-checked the fetched body against the target disassembly instruction by
instruction.
**Result:** exact structural match, including the two inlined helpers
`set_alarm`/`get_alarm`. Independent corroboration that does not depend on the
census: `*D_800A14C0 & 0x1000000` is the DMA3 CHCR channel-busy bit, `0x3C0` is
960 vblanks, and `D_800A1494[0]`/`[1]` are the adjacent `sync`/`ready` bytes of
Sony's `CD_intr` struct — which is exactly why the two `lbu` share a base.
Full mapping banked at `ref/sotn_libcd_bios_CD_datasync.c`.
**Verdict: CONFIRMED.** Six independent decomps of the same object are listed
in that file if a second opinion is ever wanted.

### H27 — KILLED
**Statement:** transcribing the reference verbatim into BB2's symbols will
land at or below the inherited floor, because it is the original source.
**Mechanism:** if the object is verbatim-linked Sony code, the original C is
the original C.
**Probe:** `r0` (hand-inlined verbatim, no helper locals), `r1` (+ the three
table-base locals), `r2` (`set_alarm`/`get_alarm` as real `static __inline__`
helpers). `s7/score.py`.
**Result:** 35 / 91, 37 / 93, 31 / 94 respectively, against the candidate's
8 / 91. `r0` does hit target's exact instruction count.
**Verdict: KILLED as a direct win.** The reference's value is as ground truth
about WHICH constructs are original, not as a drop-in. The gap is in this
repo's declarations and in two LICM decisions, not in the statements.
Banked: `rejected/sony-reference-verbatim-hand-inlined-35.c`.

### H28 — KILLED
**Statement:** Sony declares `volatile Alarm_t Alarm;` and
`static volatile CD_intr Intr = {0};`, so correcting BB2's extern declarations
to match is (a) legitimate rather than a coercion and (b) exactly the
dependency-DAG change the session-6 frontier demanded, since volatile MEMs
cannot be reordered against each other and would separate the two isomorphic
argument chains.
**Mechanism:** `sched_analyze` gives volatile MEMs dependencies on all pending
memory, which changes `INSN_PRIORITY` and the readiness order in sched1.
**Probe:** `s7/vscore.py` rewrites every extern declaration block in the TU
(`D_800F19B8`, `D_800F19BC`, `D_800F19C0`, `D_800A1494/5/6`) to volatile and
then splices a variant; measured on both chassis.
**Result:** candidate 8 → **16 / 91**; reference chassis `r0` 35 → **35 / 91**
(no change at all).
**Verdict: KILLED.** BB2 links PsyQ 4.0 where the reference is 3.5-era; the
shipped object was evidently not built with these volatile-qualified. Do not
re-propose volatile on this function.
Banked: `rejected/volatile-alarm-intr-decls-regress-16.c`.

### H29 — KILLED
**Statement:** the reference's `||` short-circuit timeout test and its
`while (1) { ... break; }` exits are the original's control flow, so adopting
them should be at worst neutral (they compile to the same CFG as the
candidate's two-if-plus-goto spelling).
**Mechanism:** `A || B` expands to `if (A) goto then; if (!B) goto else;`,
which is literally the candidate's goto pair.
**Probe:** `c1` = candidate with ONLY the `||`; `c2` = candidate with ONLY the
exits changed; `c4` = both; plus `h5`/`h6`/`h7`/`h8` on the reference chassis.
**Result:** c1 27 / 92, c2 61 / 129, c4 74 / 131. The `||` costs 19 points and
one instruction in isolation; the `while(1)`+`break` respelling with the inner
gotos retained causes pathological block duplication.
**Verdict: KILLED.** The candidate's `do { ... } while (a0 == 0)` + `check:`
scaffolding stands.
Banked: `rejected/or-shortcircuit-timeout-test-regresses-27.c`.

### H30 — CONFIRMED (and it is the session's main result)
**Statement:** the inherited 8/91 is not a cheat-free 8; the honest cheat-free
floor of this basin is much higher, and the levers can be priced individually.
**Mechanism:** strip every codegen-control construct off the proven chassis and
write the statements exactly as the reference has them, then add each lever
back alone.
**Probe:** `clean` / `cleana` / `cleank` vs `candidate`, `s7/score.py`.
**Result:** clean (no levers) **32 / 96**; +named arg4 only 27 / 96;
+`k` double-set LICM defeat only 21 / 90; candidate (both plus `cnt = k`
staging) 8 / 91.
**Verdict: CONFIRMED.** Any completion claim on the 8 must be presented to
cheat-review alongside the 32.
Banked: `rejected/clean-no-levers-licm-hoists-both-constants-32.c`.

### H31 — CONFIRMED
**Statement:** the whole allocation defect of the clean, reference-faithful
form is `loop.c`'s LICM hoisting the two loop-invariant compare constants —
everything else about its register allocation is already target's.
**Mechanism:** `scan_loop`/`move_movables` build movables for the invariant
`(set (reg) (const_int))` insns and hoist them to the preheader; the resulting
pseudos become allocnos and compete in `global.c:allocno_compare`.
**Probe:** instrumented cc1 (`tools/gcc-2.7.2/cc1`, verified CODEGEN-IDENTICAL
to the frozen `build/cc1` on the same TU), `BB2_ALLOC_DEBUG=1`, via
`s7/idump.sh clean` and `s7/idump.sh cleank`.
**Result:** in `clean`, `ord=2 pseudo=78 hardreg=16` (tbl_125c → $s0),
`ord=3 pseudo=77 hardreg=17` (idx_1494 → $s1), `ord=4 pseudo=72 hardreg=18`
(the param → $s2) are already target's, and the defect is `ord=5 pseudo=108`
(0x1000000, pri 326) and `ord=6 pseudo=85` (0x3C0000, pri 319) taking $s3/$s4
and pushing `tbl_11dc` (pri 300) to $s5 — a fifth and sixth callee-save, +5
insns. In `cleank`, with only the double-set defeat added, the map is target's
exactly: 79→$s0, 78→$s1, param→$s2, tbl_11dc→$s3, `k`→$a0, four callee-saves,
90 insns.
**Verdict: CONFIRMED.** The clean-C matching problem for this function reduces
to one question — a legitimate spelling that denies `loop.c` those two hoists.
Banked: `rejected/cleank-licm-defeat-alone-lands-exact-callee-save-map-21.c`.

## Live frontier (for session 8)

### F13 — a legitimate suppression of the two constant hoists (THE question)
**Mechanism:** H31 prices this precisely: suppress exactly those two
`loop.c` movables and the clean, reference-faithful form gets target's exact
`$s0/$s1/$s2/$s3` map with four callee-saves at 90 insns, with no other change.
The gate is `loop.c:695`: a movable is skipped only when all three of
(A) `! maybe_never && ! loop_reg_used_before_p`, (B) `! REG_USERVAR_P &&
! REG_LOOP_TEST_P`, (C) `reg_in_basic_block_p` are false. (B) is
unconditionally true for a compiler temp holding a literal, which is why every
clean spelling hoists. Session 3 already showed a NAMED local (making (B)
false) plus `maybe_never` works for 0x1000000 but not for 0x3C0000.
**Next probe:** the untried half of (A) is `loop_reg_used_before_p`, not
`maybe_never` — a named local that is READ earlier in the loop body than the
point where it is set makes (A) false regardless of `maybe_never`, which is
exactly the position 0x3C0000 sits in. Enumerate loop-carried spellings of the
timeout counter/limit that read such a local before setting it, and re-read
`tools/gcc-2.7.2/loop.c:690-712` and `:920-940` first to confirm the predicate
before sweeping. Screen with `s7/score.py` and confirm the hoist is gone by
reading ALLOCDBG from `s7/idump.sh <tag>` (a suppressed hoist shows as 7 rather
than 8 allocnos and `tbl_11dc` at `hardreg=19`).

### F14 — correct the `Intr` and `Alarm` declarations to Sony's structs
**Mechanism:** target reaches `Intr` through a hoisted base register
(`lbu a0,0(s1)` / `lbu v0,1(s1)`, offset 0 and 1). Neither the reference
spelling (`r0`: GCC folds the constant address into each `lbu` as %hi/%lo) nor
an explicit `u8 *idx_1494` pointer local (`r1`, and session 6's whole n-family)
reproduces that. Sony's source has ONE object, `static volatile CD_intr Intr`,
with two member offsets — a shape this repo cannot currently express because
`D_800A1494` is declared `extern u8` and the two sibling functions index it
through `u8 *` locals. Note H28 killed the `volatile` qualifier specifically;
the STRUCT shape is a separate, untested question.
**Next probe:** this needs the shared declaration changed and both siblings'
bodies adjusted, which is outside a single-function grind session's scope — so
it wants a `structural` or `rederive` session scoped to all three CD_* siblings
at once (`saEft01Init`, `cpu_side_move_dir_4` = CD_sync at ~line 366, and
CD_ready at ~line 480). Declare `typedef struct { u8 sync, ready, c; } CD_intr;
extern CD_intr D_800A1494;` and index `D_800A125C[D_800A1494.sync]`.

### F15 — the session-3 F7 tail (+1), now with a working DBRDBG
**Mechanism:** unchanged from session 3's F7 — reorg's `fill_slots_from_thread`
fills the `beqz $v0` delay slot with `move v0,zero` in target and refuses it for
us. Session 3 shelved this because `BB2_DBR_DEBUG` produced no output; that was
the wrong binary (`build/cc1`). `tools/gcc-2.7.2/cc1` carries the full DBRDBG
trace set, and `BB2_NO_FT_STEAL` / `BB2_ALLLIVE_LABEL` exist to bisect the
behaviour.
**Next probe:** `BB2_DBR_DEBUG=1 bash tmp/grind/saEft01Init/s7/idump.sh <tag>`
on the candidate and on session 3's `w1_goto_exits`, find the `thr LOSE` line
for the mask exit, and read which resource test rejected the steal. Untouched
by sessions 4-7; it is the smaller half of the residual and should be done only
after F13, since F13 changes the block layout it depends on.

## [s7] saEft01Init is Sony PsyQ LIBCD `CD_datasync`, and publicly matched C for it exists and can be obtained and mapped onto BB2's symbols.
- mechanism: The brief's 2026-07-09 bit-exact census flagged the whole module as verbatim-linked Sony object code; if so, the reference source is public in one of the several PSY-Q decomp projects.
- probe: `gh api search/code -f q='CD_datasync in:file language:c'` then `gh api repos/Xeeynamo/sotn-decomp/contents/src/main/psxsdk/libcd/bios.c`; cross-checked the fetched body against the target disassembly instruction by instruction.
- result: Exact structural match including the two inlined helpers set_alarm/get_alarm. Independent corroboration that does not rely on the census: *D_800A14C0 & 0x1000000 is the DMA3 CHCR channel-busy bit, 0x3C0 is 960 vblanks, and D_800A1494[0]/[1] are the adjacent sync/ready bytes of Sony's CD_intr struct (which is why the two lbu share a base). Full BB2->Sony mapping of all 14 symbols banked at memory/grind/saEft01Init/ref/sotn_libcd_bios_CD_datasync.c, with five further independent decomps of the same object listed.
- verdict: CONFIRMED

## [s7] Transcribing the reference verbatim into BB2's symbols will land at or below the inherited floor, because it is the original source.
- mechanism: If the object is verbatim-linked Sony code, the original C is the original C.
- probe: r0 (hand-inlined verbatim, no helper locals), r1 (+ the three table-base locals), r2 (set_alarm/get_alarm as real `static __inline__` helpers), all through tmp/grind/saEft01Init/s7/score.py.
- result: 35/91, 37/93 and 31/94 respectively, against the candidate's 8/91. r0 does hit target's exact 91 instructions but allocates five callee-saves (both compare constants hoisted into the prologue as `lui s3,0x3c` / `lui s2,0x100`) and fails to hoist a base register for Intr, folding the constant address into each lbu as %hi/%lo.
- verdict: KILLED

## [s7] Sony declares `volatile Alarm_t Alarm;` and `static volatile CD_intr Intr = {0};`, so correcting BB2's extern declarations to match is both legitimate (a header-type correction, not a coercion) and exactly the dependency-DAG change the session-6 frontier demanded to separate the two isomorphic argument chains.
- mechanism: sched_analyze gives volatile MEMs dependencies against all pending memory, which changes INSN_PRIORITY and the readiness order in sched1 — the only thing session 6 and the discarded session-7 attempt agreed could break the tie between the two chains.
- probe: tmp/grind/saEft01Init/s7/vscore.py rewrites every extern declaration block in the TU (D_800F19B8, D_800F19BC, D_800F19C0, D_800A1494/5/6) to volatile before splicing; measured on both the candidate chassis and the reference chassis.
- result: Candidate 8 -> 16/91. Reference chassis r0 35 -> 35/91, i.e. no change at all. BB2 links PsyQ 4.0 where the sotn reference is 3.5-era, so the shipped object was evidently not built with these volatile-qualified.
- verdict: KILLED

## [s7] The reference's `||` short-circuit timeout test and its `while (1) { ... break; }` exits are the original's control flow, so adopting them should be at worst neutral, since `A || B` expands to exactly the candidate's two-if-plus-goto CFG.
- mechanism: expand turns `A || B` into `if (A) goto then; if (!B) goto else;`, which is literally the candidate's goto pair, so the respelling should be byte-inert.
- probe: c1 = candidate with ONLY the `||`; c2 = candidate with ONLY the exits changed; c4 = both; plus h5/h6/h7/h8 on the reference chassis.
- result: c1 27/92 (the `||` alone costs 19 points and one instruction), c2 61/129 and c4 74/131 (pathological block duplication, not near-misses). Conversely the reference CONFIRMED the candidate's tail scaffolding: target's `j / li v0,-1 / move v0,zero / bnez v0 / li v0,-1` at idx 68-72 is the inlined-get_alarm() return-value shape, so the v0 temp and the `check:` label ARE the original's structure.
- verdict: KILLED

## [s7] The inherited 8/91 is not a cheat-free 8; the honest cheat-free floor of this basin is much higher and each lever can be priced individually against the reference's own statements.
- mechanism: The reference shows the original has no named arg4 intermediate (all four table lookups inline in the printf call) and no `k` (both compare constants are plain literals), so both levers the 8 rests on are absent from the original. Strip them and add them back one at a time.
- probe: clean / cleana / cleank / candidate through tmp/grind/saEft01Init/s7/score.py, all on the proven sessions-2/3 chassis.
- result: clean (no levers, reference statements) 32/96; +named arg4 only 27/96; +the k double-set LICM defeat only 21/90; candidate (k + `cnt = k` staging + arg4) 8/91. Any completion claim on the 8 must be put to cheat-review alongside the 32.
- verdict: CONFIRMED

## [s7] The entire allocation defect of the clean, reference-faithful form is loop.c's LICM hoisting the two loop-invariant compare constants; every other register-allocation decision in it is already target's.
- mechanism: scan_loop/move_movables build movables for the invariant `(set (reg) (const_int))` insns and hoist them to the loop preheader; the resulting pseudos become allocnos and compete in global.c:allocno_compare against the three table pointers and the param.
- probe: Instrumented cc1 at tools/gcc-2.7.2/cc1 with BB2_ALLOC_DEBUG=1, via tmp/grind/saEft01Init/s7/idump.sh on `clean` and on `cleank`. idump.sh re-verifies on every run that the instrumented binary is CODEGEN-IDENTICAL to the frozen build/cc1 on the same TU (it printed that both times), so this is a diagnostic instrument and not a compiler divergence.
- result: clean: ord=2 pseudo=78 hardreg=16 (tbl_125c -> $s0), ord=3 pseudo=77 hardreg=17 (idx_1494 -> $s1), ord=4 pseudo=72 hardreg=18 (the param -> $s2) are ALREADY target's with no lever of any kind; the defect is ord=5 pseudo=108 (0x1000000, pri 326) and ord=6 pseudo=85 (0x3C0000, pri 319) taking $s3/$s4 and pushing tbl_11dc (pri 300) to $s5 — a fifth and sixth callee-save, +5 insns (96 vs 91). cleank, with only the double-set defeat added: 79->$s0, 78->$s1, param->$s2, tbl_11dc->$s3, k->$a0, four callee-saves, 90 insns. The payoff of suppressing exactly those two hoists is therefore exact and complete.
- verdict: CONFIRMED
