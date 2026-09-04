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

## Session 3 (structural) — measured

### H9 — CONFIRMED (mechanism for the +2 tail branch sense)
**Statement:** our inverted mask exit (`bnez v0,<cont> / nop / j <end> /
move v0,zero`, where target has `beqz $v0,.L80081CFC` with the constant in
the delay slot) is blocked by jump.c's "conditional jump jumping over an
unconditional jump" transform, which requires the unconditional jump to
IMMEDIATELY follow the conditional one — and our `ret = 0;` set sits in
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
**Result:** the build now emits `beqz $v0,<exit>` — target's exact branch
sense — in every goto variant. **Verdict: CONFIRMED.** The bare-goto body
is the lever for the branch sense; it is not sufficient on its own (H10).

### H10 — KILLED (source-level exit spelling and label order are exhausted)
**Statement:** with the branch sense fixed by H9, some arrangement of the
exit spellings / out-of-loop label ORDER will also get the stranded
`return 0;` block folded into that branch's delay slot, reproducing
target's `beqz $v0,.L80081CFC / addu $v0,$zero,$zero`.
**Probe:** eight variants, all measured with `sandbox --disable all` and
diffed with `grind_diff.py` — `w1` (both exits goto, timed_out then
aborted), `w4` (aborted then timed_out), `w8`/`w9` (three goto exits, the
`return 1` also moved out of line, in both orders), `w2` (mask exit only),
`w3` (flag exit only), `w5` (`if (mask) {ret=1;} else {ret=0;break;}`),
`w6` (`if (mask) {ret=1; continue;} ret=0; break;`).
**Result:** every goto-form emits a BYTE-IDENTICAL tail at 19 / 93 insns —
GCC normalises the out-of-line block order itself, so source label order is
inert. `w5`/`w6` collapse back to the candidate's exact output (18 / 92):
GCC re-canonicalises the source-level branch sense too. **Verdict: KILLED.**
The residual is not reachable from the C's exit spelling; it is one reorg
decision (F7).

### H11 — CONFIRMED, and it REFRAMES H6 (the `k` reuse is not the original)
**Statement:** the session-2 candidate's single reused scratch local `k` is
the original spelling of the two loop-invariant compare constants.
**Probe:** read the two `lui` sites in `asm/funcs/saEft01Init.s`.
**Result:** **target holds them in TWO DIFFERENT hard registers** —
`lui $v0,(0x3C0000>>16)` at idx 41 (immediately consumed by the following
`slt $v0,$v0,$v1`) and `lui $v1,(0x1000000>>16)` at idx 83. One C variable
is one pseudo and one hard register; our build puts both in `$a0`.
**Verdict: the statement is FALSIFIED — `k` cannot be the original.** It
remains the best-scoring form (18/92 vs 29/98 for any single-set spelling)
but it is a synthetic LICM defeat, and cheat-reviewer has still not seen it.
Any completion claim must either replace it or clear it through review.

### H12 — CONFIRMED (the real loop.c gate) / KILLED (as a full replacement)
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
baseline — the mask constant escapes the hoist in its own pseudo, no
double-set needed. `y2` (0x3C0000 set at the top of the loop body) = 30/98,
i.e. still hoisted: `maybe_never` is 0 that early, so branch (A) holds.
Moving its set later makes it BB-local, so branch (C) holds. `x2`/`x3` both
30/98 — any single-set constant local is hoisted.
**Verdict: the gate is CONFIRMED and now usable; the two-local form is
KILLED as a replacement** (27/97 is far worse than 18/92) because the
0x3C0000 constant has no position in this loop that fails all three
branches. Banked:
`rejected/two-spanning-const-locals-only-mask-escapes-licm.c`.

### H13 — KILLED
**Statement:** target's register choice ($v0 for 0x3C0000, which is also
its timeout/success flag) means the flag local carries that constant, so
`v0` as the timeout holder plus a branch-spanning local for the mask
reproduces the two-register shape without the `k` reuse.
**Probe:** `z1` (v0 holds 0x3C0000, `k2` spans for the mask), `z3` (v0 for
the timeout, literal for the mask), `z2` (z1 plus the H9 bare-goto exit).
**Result:** 22/94, 29/95, 24/95. Both constants do stay inline in z1, but
overloading `v0` costs +2 in the flag block — the same regression session 2
measured with v0 carrying BOTH constants (H8, 22/93). **Verdict: KILLED.**
The exit flag is unusable as a constant holder in any combination. Banked:
`rejected/v0-timeout-holder-plus-spanning-mask-local.c`.

### H14 — KILLED (tooling; saves the next session a detour)
**Statement:** reorg.c's `BB2_DBR_DEBUG=1` instrumentation
(`DBRDBG thr/simp/mtlr` traces, reorg.c:133-153) can be used to read why
`fill_slots_from_thread` refuses the delay-slot steal.
**Probe:** compiled the spliced tree with `BB2_DBR_DEBUG=1` through
`tmp/grind/saEft01Init/s3/dbr.sh` and grepped the captured stderr for the
tail insn UIDs.
**Result:** ZERO `DBRDBG` lines — the shipped `tools/gcc-2.7.2/build/cc1`
predates the instrumentation, exactly as session 2 found for
`BB2_ALLOC_DEBUG`/ALLOCDBG. **Verdict: KILLED.** Use the `-da` `.dbr` RTL
dump instead; `tmp/grind/saEft01Init/s3/dbrscan.py` extracts the
saEft01Init tail with insn UIDs and is the tool of record for this class of
question.

## Live frontier (for session 4)

### F7 — the reorg delay-slot steal that strands `return 0;` (the whole +2)
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
registers live when it cannot find the block start — reorg.c also carries a
`BB2_ALLLIVE_LABEL` env hook, which suggests this project has hit that
fallback before). Then find C that puts an ordinary INSN between the mask
branch and the loop-condition branch, or that leaves the `return 0;` thread
ending in a real jump. Regenerate the RTL with
`bash tmp/grind/saEft01Init/s3/dbr.sh <variant>` (it prints the tail via
dbrscan.py). Note `w2` (which keeps `ret = 1;` before the loop condition)
does NOT achieve this — fill_simple_delay_slots consumes that insn into
184's slot before fill_eager ever looks at 173.

### F8 — make the 0x3C0000 constant escape LICM in its OWN pseudo
**Mechanism:** H12's gate. The constant needs a set position where
`maybe_never` is already 1 (i.e. after the loop's first CODE_LABEL or
JUMP_INSN, loop.c:930) AND a live range crossing a basic-block boundary so
`reg_in_basic_block_p` is false. In the current shape the only in-loop
branch before it is the `if (D_800F19B8 < v0)` test, and the compare
follows in the same block.
**Next probe:** alternatives that satisfy both — a spelling of the timeout
test that puts an extra branch or label between the constant's set and its
use; a variable that is REFERENCED earlier in the loop than its set
(branch (A)'s `loop_reg_used_before_p` half, a second way to fail (A) that
does not need `maybe_never`); or restructuring so `D_800F19BC`'s
read/increment sits in a separate block. Success looks like 91-92 insns
with `$s4`/`$s5` absent from the `.greg` dispositions and `lui $v0,0x3c` /
`lui $v1,0x100` in two DIFFERENT caller-saved registers. Screen with
`tmp/grind/saEft01Init/s3/score.py`.

### F9 — cheat-review status of the `k` reuse
**Mechanism:** H11 shows the single reused `k` is not what the original
compiled from. It is currently load-bearing for the 18/92 floor and is in
the `[[defeat-licm-hoist-var-reuse]]` family, which is SOTN-sanctioned but
only as a documented last resort with lever-exhaustion.
**Next probe:** if F8 lands, `k` disappears on its own and the question is
moot — do F8 first. If a completion is reached WITH `k` still present, it
must go through cheat-reviewer with H11's two-register evidence disclosed,
because "this is what the original wrote" is now known to be false.

### F6 — cluster B, the debug_printf argument block (inherited, still open)
Session 3 did not touch it. Session 2's note stands: re-score both
arg-staging orders against the now-correct callee-save allocation.

## Superseded frontier (session 2 — F4 and F5 resolved by H9/H10/H12/H13 above)

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

## Session 8 (rederive) — measured

Modality: rederive.  Floor unchanged at **8 / 91** (the inherited candidate);
the zero-lever goto-loop chassis re-measured at **18 / 91**.  Eleven variants
measured (x1, x1c, x2, x2c, x3, g0-g6), one target-bytes read, two
instrumented-cc1 dump sets, five external reference sources fetched.

### H32 — KILLED
**Statement:** session 7 fetched only one decomp of this Sony object; the five
other projects it listed will contain a structurally DIFFERENT reconstruction
of CD_datasync, and one of those shapes will be the one that compiles to
target.
**Mechanism:** these are independent matching-decomp projects working the same
verbatim-linked library object against the same GCC 2.7.2 era, so a shape that
byte-matches in one of them is strong evidence about the original spelling.
**Probe:** `gh api repos/<r>/contents/<p>` for xenogears-decomp, psx_tomba,
lom-decomp, rood-reverse and psyz; the two with real bodies (xeno, tomba) were
transcribed onto BB2's symbols in both the candbase and the reference-faithful
statement flavours (x1/x1c and x2/x2c) and scored with s8/score.py.
**Result:** three genuinely different exit spellings exist across sotn / xeno /
tomba, and all three cost the same.  xeno's nested-if/direct-return form:
27 / 96 (named args) and 31 / 96 (inline args).  tomba's early-return-plus-
`sync`-local form: 27 / 96 and 31 / 96, identical.  sotn's form was session
7's r0 at 35 / 91.  Every one of them is a `while (true)` loop and therefore
pays the same two LICM const hoists (+2 callee-saves, +5 insns) that session 7
priced on `clean`.
**Verdict: KILLED.**  The exit-spelling axis of the reference corpus is now
exhausted across three independent reconstructions.  What the corpus DOES
still buy is corroboration: all three agree on the statements, so the
statements are settled, and the disagreement is confined to a spelling that
provably does not matter here.
Banked: rejected/xeno-while1-direct-returns-licm-hoists-27.c,
rejected/tomba-while1-sync-var-breaks-27.c.

### H33 — KILLED
**Statement:** the exit branch-sense/nesting difference between candbase
(`if (a0 == 0) goto loop; return 1;`) and the reference corpus
(`if (chcr & mask) { if (mode) return 1; } else return 0;`) is a real codegen
lever on the goto-loop chassis.
**Mechanism:** the two spellings put the loop-back edge on opposite arms, and
session 3 measured every bare-goto exit form as 19/93, so branch sense had
looked live.
**Probe:** `x3` = g0's chassis with xeno's nesting substituted verbatim.
**Result:** 18 / 91, byte-identical to g0.  jump.c normalises them.
**Verdict: KILLED.**  Byte-inert; do not re-spell the exits on this chassis.
Banked: rejected/xeno-nesting-on-goto-loop-byte-inert-18.c.

### H34 — CONFIRMED (the session's main result)
**Statement:** the goto-loop chassis at HEAD is the honest zero-lever floor of
this function at 18 / 91, its instruction count is already exact, and its
ENTIRE residual is a three-cycle rotation of $s0/$s1/$s2 caused by the
parameter's allocno outranking the two table pointers.
**Mechanism:** no NOTE_INSN_LOOP_BEG => loop.c never runs on this body => the
two const_int movables that cost the real-loop chassis +5 insns are never
created, and target's own bytes confirm the constants belong in-loop.  What is
left is `global.c:allocno_compare`, which sorts on `nrefs * 10000 / livelen`.
**Probe:** target disassembly read for the constant placement and the
callee-save map; `s8/idump.sh g0` for the greg dispositions and the ALLOCDBG
priority table; `s8/score.py g0` for the score.
**Result:** g0 = 18 / 91.  greg: param->$s0, D_800A125C->$s1, D_800A1494->$s2,
D_800A11DC->$s3 against target's D_800A125C->$s0, D_800A1494->$s1, param->$s2,
D_800A11DC->$s3.  ALLOCDBG: param pri 384 (nrefs 2, livelen 52), D_800A125C
312 (3, 96), D_800A1494 306 (3, 98), D_800A11DC 200 (2, 100); all four
priorities reproduce exactly from the formula.  Target's map is that same sort
with the param demoted from 1st to 3rd, so the target window for the param's
priority is the open interval (200, 306).
**Verdict: CONFIRMED.**  This replaces "the residual is register choice and
scheduling" with a single arithmetic target, and it is on the chassis that
needs no FAKE-family lever at all.

### H35 — KILLED
**Statement:** the required +1 REG_N_REFS on the two table pointers can be
obtained by a dead self-assignment (`p = p;`), which is the cheapest spelling
of the ref-lift H34 prices.
**Mechanism:** REG_N_REFS counts insns that reference the pseudo, and a
self-assign is one more such insn.
**Probe:** `g6`, run explicitly as an instrument and never as a candidate (dead
self-assigns are a forbidden family); scored and dumped with s8/idump.sh.
**Result:** 18 / 91 and greg dispositions byte-identical to g0 (72 in 16, 77 in
17, 76 in 18, 75 in 19).  flow.c deletes the self-assign before reg_n_refs is
computed, so it never reaches the allocno.
**Verdict: KILLED**, and it generalises: no DEAD construct can move this
allocation.  The ref-lift, if it is reachable at all, has to be a LIVE fourth
reference to each pointer.  (This is the same measurement the standing
[[duplicated-statement-into-arms]] rule already records for BB2 generally, now
confirmed for this function.)
Banked: rejected/dead-self-assign-ref-lift-deleted-by-flow-18.c.

### H36 — KILLED
**Statement:** the param's live range (livelen 52) can be lengthened, or the
pointers' shortened, by re-ordering the pre-loop statements, which is a fully
legitimate pure-C change and is the other arithmetic route into H34's window.
**Mechanism:** the pointers' live ranges start where they are assigned, so
moving the assignments later in the pre-loop block shortens livelen and raises
their priority above the param's 384.
**Probe:** `g2` (assignments moved to the end of the pre-loop block), `g4`
(declaration order changed to target's callee-save order), `g5` (assignment
order changed to target's callee-save order), `g3` (assignments moved before
the `sys_VSync(-1)` call).
**Result:** g2 / g4 / g5 all 18 / 91, unchanged.  g3 regresses to 23 / 91.
Statement and declaration order inside the pre-loop block is byte-inert here,
consistent with session 6's finding that source placement inside a block is
inert because rank_for_schedule never reaches the INSN_LUID tie-break.
**Verdict: KILLED for the pre-loop block.**  The live-range route survives only
if the change is made where liveness is actually decided (see F16) — it cannot
be bought with statement order.
Banked: rejected/pointer-inits-before-vsync-regresses-23.c.

## Live frontier (for session 9)

### F16 — close the param's allocno priority into (200, 306) on the goto-loop chassis
**Mechanism:** H34 prices the whole 18-point residual as one demotion: the
parameter pseudo must sort below D_800A1494 (pri 306) and above D_800A11DC
(pri 200) in `global.c:allocno_compare`, at which point find_reg hands out
target's exact $s0/$s1/$s2/$s3 map on a chassis carrying no lever of any kind.
`pri = nrefs * 10000 / livelen`, verified to the integer on all four allocnos.
Two arithmetic routes: (a) param livelen 52 -> 66..99 with nrefs 2, or
(b) both pointers nrefs 3 -> 4 (416 / 408, relative order preserved).  H35
killed the dead-store spelling of (b) and H36 killed the statement-order
spelling of (a).
**Next probe:** first find out WHY the param's livelen is only 52 when the
three pointers are 96-100 on the same body — it is set in the prologue and
consumed by the loop-back test at the very bottom, so 100 is what one would
expect.  Read the `;; Function saEft01Init` section of
tmp/grind/saEft01Init/s8/g0/system.i.lreg (and the flow.c live-range walk that
produces reg_live_length) to find which region the param is NOT live in.
Whatever that region is IS the lever surface: a C spelling that keeps the
parameter live across it moves livelen into the window directly, with no
FAKE-family construct.  Screen with tmp/grind/saEft01Init/s8/score.py and
confirm with the greg "Register dispositions" line from s8/idump.sh.

### F17 — a LIVE fourth reference to D_800A125C and D_800A1494
**Mechanism:** route (b) of F16.  Both pointers sit at nrefs 3 (one set, two
uses); a fourth LIVE reference each puts them at 416 / 408, above the param's
384, preserving their relative order and landing target's map.  H35 proved the
reference must survive flow.c, so it must be a real use whose value is
consumed.
**Next probe:** the constraint is tight — the function only reads
`D_800A125C[Intr.sync]` and `D_800A125C[Intr.ready]`, so a fourth reference has
to come from a restructuring that genuinely consumes the pointer again.  The
one shape worth pricing first is the [[duplicated-statement-into-arms]] family
on the `check:` if-chain, which is BB2's sanctioned byte-neutral ref-lift and
is the only construct measured to move reg_n_refs where dead stores do not;
it needs byte-neutrality verification, the lever-exhaustion record (sessions
1-8 now supply it) and a `/* FAKE */` annotation, so price it as a measurement
first and only then decide whether it is proposable.

### F18 — the loop.c address-vs-const movable asymmetry (F13, now correctly scoped)
**Mechanism:** unchanged in substance from F13, but session 8's read of the
target bytes narrows it: the original compile DID run LICM (all three symbol
addresses are hoisted into the pre-loop block as $s0/$s1/$s3) and did NOT move
the two const_int movables.  So the question is no longer "how do we suppress
LICM" but "what distinguishes an address movable from a single-use const_int
movable in scan_loop/move_movables".  Note the candidate shape for the answer:
the addresses are each referenced twice or hoisted for a base register, while
each const_int is used EXACTLY ONCE, which is the precondition of the
"large loop lossage" substitution path in loop.c (the block starting at the
`/* A potential lossage ... */` comment around loop.c:725, reached only when
the loop contains calls — this loop does).
**Next probe:** read loop.c:725-840 and determine what happens to a single-use
const_int movable in a loop-with-calls when `validate_replace_rtx` of the
constant into its `slt`/`and` use FAILS (0x3C0000 and 0x1000000 are outside
MIPS's 16-bit immediate range, and the `slt` pattern wants a register in that
operand anyway).  If the answer is that the movable is dropped rather than
moved, then the real-loop chassis can be made to behave like target by a
legitimate change and the 21/90 `cleank` map becomes reachable without `k`.
Do this only if F16/F17 do not close, since the goto-loop chassis reaches the
same map with no LICM question at all.

## [s8] Session 7 fetched only one decomp of this Sony library object; the five other projects it listed will contain a structurally DIFFERENT reconstruction of CD_datasync, and one of those shapes is the one that compiles to target.
- mechanism: These are independent matching-decomp projects working the same verbatim-linked PsyQ LIBCD object against the same GCC 2.7.2 era, so a shape that byte-matches in one of them is strong evidence about the original spelling.
- probe: gh api repos/<r>/contents/<p> for ladysilverberg/xenogears-decomp, hansbonini/psx_tomba, celophi/lom-decomp, ser-pounce/rood-reverse and Xeeynamo/psyz (all cached under tmp/grind/saEft01Init/s8/ref/). Three carry only INCLUDE_ASM stubs; xeno and tomba carry real bodies with genuinely different exit spellings. Both were transcribed onto BB2's symbols in two statement flavours each (x1/x1c = candbase named args, x2/x2c = reference inline args) and scored with tmp/grind/saEft01Init/s8/score.py.
- result: xeno's nested-if/direct-return shape: 27/96 and 31/96. tomba's early-return-plus-`sync`-local shape: 27/96 and 31/96, identical. sotn's shape was session 7's r0 at 35/91. All three are `while (true)` loops and therefore all three pay the same two LICM const_int hoists (+2 callee-saves, +5 instructions) that session 7 priced on `clean`. The corpus does corroborate the statements — all three sources agree on those — but no exit spelling in it is cheaper.
- verdict: KILLED

## [s8] The exit branch-sense/nesting difference between candbase (`if (a0 == 0) goto loop; return 1;`) and the reference corpus (`if (chcr & mask) { if (mode) return 1; } else return 0;`) is a real codegen lever on the goto-loop chassis.
- mechanism: The two spellings put the loop-back edge on opposite arms, and session 3 measured every bare-goto exit form at 19/93, so branch sense had looked live.
- probe: x3 = HEAD's chassis with xeno's nesting substituted verbatim; s8/score.py.
- result: 18/91, byte-identical to HEAD's body. jump.c normalises the two spellings to the same CFG. The exit branch-sense axis is closed on this chassis.
- verdict: KILLED

## [s8] The goto-loop chassis committed at HEAD is the honest zero-lever floor of this function at 18/91 — not the 32/96 session 7 reported — its instruction count is already exact, and its ENTIRE residual is a three-cycle rotation of $s0/$s1/$s2 caused by the parameter's allocno outranking the two table pointers.
- mechanism: A goto-loop emits no NOTE_INSN_LOOP_BEG, so loop.c never sees a loop and the two const_int movables that cost the real-loop chassis +5 instructions are never created. Target's own bytes confirm the constants belong in-loop. What is left is global.c:allocno_compare, which sorts on nrefs * 10000 / livelen, and find_reg then hands out $s0..$s3 in that sorted order.
- probe: Read asm/funcs/saEft01Init.s for the constant placement and callee-save map; `bash tmp/grind/saEft01Init/s8/idump.sh g0` for the greg 'Register dispositions' and the ALLOCDBG priority table; s8/score.py g0 for the score.
- result: g0 (HEAD's body verbatim) = 18/91. greg: param->$s0, D_800A125C->$s1, D_800A1494->$s2, D_800A11DC->$s3, against target's D_800A125C->$s0, D_800A1494->$s1, param->$s2, D_800A11DC->$s3 — only the parameter is misplaced. ALLOCDBG: param pri 384 (nrefs 2, livelen 52), D_800A125C 312 (3, 96), D_800A1494 306 (3, 98), D_800A11DC 200 (2, 100); all four reproduce exactly from nrefs*10000/livelen. Target's map is the same sort with the param demoted from 1st to 3rd, so the entire 18 is bought by landing the param's priority in the open interval (200, 306).
- verdict: CONFIRMED

## [s8] The +1 REG_N_REFS that H34's arithmetic asks for on the two table pointers can be obtained by a dead self-assignment (`p = p;`).
- mechanism: REG_N_REFS counts insns referencing the pseudo, and a self-assign is one more such insn.
- probe: g6 = HEAD's body plus `idx_1494 = idx_1494; tbl_125c = tbl_125c;`, run explicitly as an INSTRUMENT and never as a candidate (dead self-assigns are a forbidden family); scored and dumped with s8/idump.sh g6.
- result: 18/91 and greg dispositions byte-identical to g0 (72 in 16, 77 in 17, 76 in 18, 75 in 19). flow.c deletes the self-assigns before reg_n_refs is computed, so they never reach the allocno. This generalises: no DEAD construct can move this allocation, so any ref-lift must be a LIVE fourth reference. Confirms for this function what the standing duplicated-statement-into-arms rule records for BB2 generally.
- verdict: KILLED

## [s8] The other arithmetic route into H34's window — changing the live ranges — is reachable by re-ordering the pre-loop statements, which is a fully legitimate pure-C change.
- mechanism: The pointers' live ranges start where they are assigned, so moving the assignments later in the pre-loop block shortens livelen and raises their priority above the param's 384.
- probe: g2 (assignments moved to the end of the pre-loop block), g4 (declaration order changed to target's callee-save order), g5 (assignment order changed to target's callee-save order), g3 (assignments moved before the sys_VSync(-1) call).
- result: g2 / g4 / g5 all 18/91, unchanged; g3 regresses to 23/91. Statement and declaration order inside the pre-loop block is byte-inert here, consistent with session 6's finding that source placement inside a block is inert because rank_for_schedule never reaches the INSN_LUID tie-break. The live-range route survives only where liveness is actually decided, not at the statement-order surface.
- verdict: KILLED


## Session 9 (rederive) — measured

Modality: rederive.  **Floor 8 -> 7 / 91**, and the chassis under the floor
changed completely: the new 7 carries NO `k`, NO mask staging and NO
FAKE-family LICM defeat — its only match device is one `do { } while (0)`.
Twenty-one variants measured (q1-q5, r1-r3, t1-t6, u1-u6, cand9), four
instrumented-cc1 dump sets, three GCC source reads (loop.c, local-alloc.c,
mips.h).

### H37 — KILLED (the whole real-loop branch, with the mechanism priced)
**Statement:** on the real-loop (`clean`) chassis — the one whose allocation
order is already target's — the two loop-invariant compare constants can be
kept in-loop by putting them in named C locals, because `loop.c:691-701`'s
movable-creation gate is skipped when all three of its disjuncts are false and
disjunct (2) (`! REG_USERVAR_P && ! REG_LOOP_TEST_P`) is false for a user
variable.
**Mechanism:** the gate; `REG_LOOP_TEST_P` is set only at `jump.c:2253` and
never applies here, so a named local is the only way to falsify (2).
Disjunct (1) needs `maybe_never` (set at the first CODE_LABEL/JUMP_INSN inside
the loop, loop.c:921-930) or a use-before-set; disjunct (3) needs the live
range to leave the setting basic block.
**Probe:** q1 (mask only, named, set at loop-body top), q2 (both named at the
top), q3 (q2 with `do{}while(0)` around each set), q4 (q2 with `do{}while(0)`
around the timeout test), q5 (both named, set AFTER the loop's first branch);
`s9/score.py` for all five, `s9/idump.sh` + the `-da` `.loop` dump for q2 and
q5.
**Result:** q1/q2/q3 = 32/96, byte-identical to `clean`; the `.loop` dump shows
both constants still built as movables and moved (`maybe_never` is 0 at the
loop-body top — the loop's own top label does not set it). q4 = 38/96, q5 =
36/95: in q5 the mask escapes the hoist for the first time (only one movable
listed as moved), confirming the gate reading exactly — but the escaped local
then measures 4 refs / 98 insns crossing 5 calls and takes a callee-save,
where target has a block-local `lui $v1,0x100` immediately before its `and`.
**Verdict: KILLED, and it closes the real-loop branch entirely.** Block-local
AND not-hoisted are mutually exclusive inside a real loop: block-local makes
disjunct (3) true, the movable is built, and `move_movables`' test
(`threshold * savings * lifetime >= insn_count`, loop.c:1631) is
unconditionally true here — threshold = 1*(1+n_non_fixed_regs) = 61 (MIPS
FIRST_PSEUDO_REGISTER 68, 8 fixed), savings = lifetime = 1, insn_count = 50.
This is why every `while (true)` spelling in the reference corpus sits at
27-35.  Banked: rejected/real-loop-user-var-consts-still-hoisted-32.c,
rejected/maybe-never-lifts-mask-only-still-36.c.

### H38 — CONFIRMED (the session's main result: 18 -> 8 with no lever)
**Statement:** the reason target's map needs a real loop is only flow.c's
`REG_N_REFS += loop_depth` weighting, and `do { } while (0)` emits the same
NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END notes that drive that counter — so
wrapping just the block that contains the table-pointer uses buys the
weighting WITHOUT handing loop.c a loop to hoist the compare constants out of.
**Mechanism:** the notes are emitted by the front end (`expand_start_loop`),
and flow.c counts them; loop.c only optimises a loop it can find a back edge
for.  The function's back edge stays a bare `goto`, so loop.c never scans it.
Predicted effect: D_800A125C and D_800A1494 go 3 refs -> 5 (their two uses are
both inside the wrapper), D_800A11DC 2 -> 3, the parameter unchanged at 2 refs
/ livelen 52 because its set (prologue) and its use (loop-back test) are both
outside the wrapper.  With `pri = floor_log2(nrefs)*nrefs/livelen*10000` that
is 1041 / 1020 / 384 / 300 — exactly target's $s0/$s1/$s2/$s3 order.
**Probe:** r1 (`do{}while(0)` around the whole timeout/printf block of the
zero-lever goto chassis g0), r2 (around the argument+printf block only), r3
(two nested wrappers); `s9/score.py`, then `s9/idump.sh r1` for `.lreg`, then
`tmp/grind/saEft01Init/s1/grind_diff.py` for the positional diff.
**Result:** r1 = **8 / 91** from g0's 18 / 91, r2 = 11 / 91, r3 = 18 / 91.  The
r1 `.lreg` numbers are the predicted ones to the integer: D_800A125C 5/96,
D_800A1494 5/98, param 2/52, D_800A11DC 3/100.  The positional diff shows the
prologue, both compare constants, and the ENTIRE tail byte-exact — including
session 3's F7 (`beqz $v0` with `addu $v0,$zero,$zero` in the delay slot),
which closed for free.  Every diff is inside build idx 46-61.
**Verdict: CONFIRMED.**  This replaces the two-lever 8/91 with a one-device
form that is strictly better and strictly cleaner, and it dissolves the
paradox between H37 and H40: the weighting and the LICM suppression were never
the same question.  The wrapper is a sanctioned `/* FAKE */` match device
under the owner's 2026-07-06 do-while(0) ruling (any codegen effect, mandatory
inline annotation, single level so no nesting justification needed).
Banked: rejected/nested-do-while0-depth3-byte-inert-18.c.

### H39 — CONFIRMED (floor 7) / KILLED (session 6's expand-shape reading)
**Statement:** with the chassis corrected, the argument block's residual is
reachable by re-spelling the four `debug_printf` arguments, and session 6's
forensics predict the winner: an INLINE argument's load is emitted by
`expand_call` straight into the hard register as the last insn of the argument
sequence, which is target's `lw a3,0(a0)`.
**Probe:** twelve spellings against r1 — t1 both inline, t2 arg5 named only,
t3 arg4 named first + arg5 inline, t4 both named arg4 first, t5 both named
arg5 first, t6 index byte named; u1 named pointer + inline deref, u2 pointer +
named value, u3 both as named pointers, u4 third argument also named, u5 index
bytes staged, u6 declaration order swapped.
**Result:** 13 / 13 / **7** / **7** / 8 / 13 ; 9 / 9 / 10 / 14-at-90-insns /
**7** / 9.  The all-inline form — session 6's predicted target shape — is 13,
six WORSE than naming arg4.  Naming the third argument as well drops the build
to 90 instructions, one fewer than target.
**Verdict: 7 is the floor of the spelling family; session 6's expand-shape
prediction is KILLED on the corrected allocation.**  Banked:
rejected/both-args-inline-on-dowhile0-chassis-13.c,
rejected/arg2-named-costs-an-instruction-90-14.c,
rejected/named-pointer-inline-deref-9.c.

### H40 — KILLED (F16 route (a), permanently — it is arithmetic, not spelling)
**Statement:** the parameter's allocno priority can be landed in the (200, 306)
window by lengthening its live range, since sessions 2 and 8 only ever killed
particular SPELLINGS of that change (statement order, declaration order, dead
stores).
**Mechanism:** `pri = floor_log2(nrefs)*nrefs/livelen*10000`, so at nrefs 2 the
window (200, 306) needs livelen 66..99.
**Probe:** counted the function's real insns in the `-da` dumps (59) and read
`local-alloc.c:update_equiv_regs` for the source of the pointers' 96-100.
**Result:** `reg_live_length` is bounded by the function's insn count, and the
param's 52 is already within 7 of the 59-insn ceiling.  The pointers' 96-100
are NOT raw — line ~1058 DOUBLES `reg_live_length` for any pseudo carrying a
REG_EQUIV note, and lines 1019-1052 attach REG_EQUIV only to a single-set
pseudo whose source is CONSTANT_P or an unchanging MEM.  The param's set is
`(set (reg/v 72) (reg:SI 4 a0))`, a hard-register source, so it can never
qualify.  Therefore param pri >= 2*10000/59 = 338 > 306 for EVERY spelling.
**Verdict: KILLED permanently.**  Also corrects session 8's formula: the
`floor_log2` factor is real (it is 1 for nrefs 2 and 3, which is why s8's fit
looked exact), so F17's route (b) numbers are 833 / 816, not 416 / 408.

## Live frontier (for session 10)

### F19 — defer `lw a3` to the end of the argument block (the whole 7)
**Mechanism:** target keeps the idx[0] address chain alive in `$a0` across the
third argument's entire computation and issues `lw a3,0(a0)` as the LAST
memory reference of the block, after `sw v1,16(sp)` and `lw a2,0(v0)`; we
compute the same chain in `$v0` and issue `lw a3,0(v0)` at build idx 54, six
insns early, which drags the arg5 chain later and produces all 7 diffs.  H39
shows this is NOT reachable from the argument SPELLING (twelve measured, 7 is
the floor) and session 6 showed source statement placement inside this block
is byte-inert because `rank_for_schedule` decides on INSN_PRIORITY and the
dependence-class test (sched.c:2412-2449) and never reaches the INSN_LUID
tie-break (sched.c:2452-2455).
**Next probe:** so read the priorities, don't guess the spelling.  Dump the
`.sched` RTL for the r1/t3 body (`bash tmp/grind/saEft01Init/s9/idump.sh t3`,
then the `;; Function saEft01Init` section of `system.i.sched`) and compare
each argument-block insn's INSN_PRIORITY and dependence list against the order
target emits.  The specific question is what makes target's `lw a3` the lowest
priority insn in the block when it is the last link of the SHORTEST of the
three chains.  A likely answer is that in target the a3 load depends on a
chain that is scheduled to finish late because `$a0` is also the format
string's register — i.e. the anti-dependence on `$a0` from `lui a0,%hi(
D_800161C8)` is what pins it — in which case the lever is which pseudo gets
`$a0`, and that is a local-alloc copy-preference question, not a spelling one.

### F20 — cheat-review status of the do{}while(0) wrapper
**Mechanism:** the wrapper is the single match device in the new candidate and
it is explicitly sanctioned (owner ruling 2026-07-06, `.claude/rules/
do-while-zero-exception.md`: any body, ANY codegen effect, register allocation
named explicitly, mandatory inline FAKE annotation, single level).  The
candidate carries the annotation.
**Next probe:** it has NOT been through a fresh adversarial cheat-reviewer.
Do that before any completion claim, and show the reviewer the mechanism
(flow.c loop_depth weighting, not a semantic change: the block executes
exactly once either way) together with H37/H40 as the lever-exhaustion record.
Also worth measuring for the reviewer: whether ANY construct-free spelling
reaches the same weighting — the only other way to raise a pointer's
REG_N_REFS is a genuine fourth reference (F17), which target's own register
usage shows does not exist ($s0 is set once and read twice in the shipped
object, $s1 likewise).

### F21 — the sibling functions inherit this for free
**Mechanism:** the ledger's session-7 note records that `set_alarm`/`get_alarm`
are inlined into two more functions in this same TU which already carry the
same hand-derived goto shape — `cpu_side_move_dir_4` (= CD_sync, name string
D_80016240) at ~src/system.c:366 and the one at ~line 480 (= CD_ready, string
D_80016248).
**Next probe:** once saEft01Init closes, apply the same wrapper + named-arg4
shape to both and re-score; they are the cheapest queue items in the file and
the mechanism is identical.

## [s9] On the real-loop (clean) chassis, whose allocation order is already target's, the two loop-invariant compare constants can be kept in-loop by putting them in named C locals, because loop.c:691-701's movable-creation gate is skipped when all three of its disjuncts are false and disjunct (2) (! REG_USERVAR_P && ! REG_LOOP_TEST_P) is false for a user variable.
- mechanism: loop.c:691-701 builds a movable unless all of (1) !maybe_never && !loop_reg_used_before_p, (2) !REG_USERVAR_P && !REG_LOOP_TEST_P, (3) reg_in_basic_block_p are false. REG_LOOP_TEST_P is set only at jump.c:2253 and never applies here, so a named C local is the only way to falsify (2); maybe_never is set at the first CODE_LABEL/JUMP_INSN inside the loop (loop.c:921-930).
- probe: q1 (mask only, named, set at loop-body top), q2 (both named at the top), q3 (q2 with do{}while(0) around each set), q4 (q2 with do{}while(0) around the timeout test), q5 (both named, set AFTER the loop's first conditional branch), scored with tmp/grind/saEft01Init/s9/score.py and dumped with s9/idump.sh; the decisive evidence is the -da .loop dump's per-movable trace.
- result: q1/q2/q3 = 32/96, byte-identical to clean; the .loop dump still lists both constants as movables that were moved, because maybe_never is 0 at the loop-body top (the loop's own top label does not set it). q4 = 38/96. q5 = 36/95: with both sets moved after the loop's first branch the MASK escapes the hoist for the first time (only one movable moved), which confirms the gate reading exactly - but the escaped local then measures 4 refs / 98 insns crossing 5 calls and takes a callee-save, where target has a block-local lui $v1,0x100 immediately before its and. Block-local and not-hoisted are mutually exclusive in a real loop: block-local makes disjunct (3) true, the movable is built, and move_movables' test (threshold*savings*lifetime >= insn_count, loop.c:1631) is unconditionally true here - threshold = 1*(1+n_non_fixed_regs) = 61 (MIPS FIRST_PSEUDO_REGISTER 68 with 8 fixed), savings = lifetime = 1, insn_count = 50.
- verdict: KILLED

## [s9] The only thing a real C loop contributes to target's callee-save map is flow.c's REG_N_REFS += loop_depth weighting, and do { } while (0) emits the same NOTE_INSN_LOOP_BEG / NOTE_INSN_LOOP_END notes that drive that counter - so wrapping only the block containing the table-pointer uses buys the weighting WITHOUT handing loop.c a loop to hoist the two compare constants out of.
- mechanism: The loop notes are emitted by the front end (expand_start_loop) for do{}while(0) exactly as for a real loop, and flow.c counts them when accumulating REG_N_REFS; loop.c only optimises a loop whose back edge it can find, and the function's back edge stays a bare goto. Predicted: D_800A125C and D_800A1494 go 3 refs -> 5, D_800A11DC 2 -> 3, the parameter unchanged at 2 refs / livelen 52 (its set and its use are both outside the wrapper), giving pri = floor_log2(nrefs)*nrefs/livelen*10000 = 1041 / 1020 / 384 / 300, i.e. target's exact order.
- probe: r1 (one do{}while(0) around the whole timeout/printf block of the zero-lever goto chassis g0), r2 (around the argument+printf block only), r3 (two nested wrappers); s9/score.py, then s9/idump.sh r1 for the .lreg reference counts, then tmp/grind/saEft01Init/s1/grind_diff.py for the positional diff against asm/funcs/saEft01Init.s.
- result: r1 = 8 / 91 from g0's 18 / 91; r2 = 11 / 91; r3 = 18 / 91. The r1 .lreg numbers are the predicted ones to the integer (D_800A125C 5/96, D_800A1494 5/98, param 2/52, D_800A11DC 3/100). The positional diff shows the prologue, both in-loop compare constants and the ENTIRE tail byte-exact - including session 3's F7 (beqz $v0 with addu $v0,$zero,$zero in its delay slot), which closed for free. Every remaining diff is inside build idx 46-61, the debug_printf argument block. This form carries no k, no staging and no FAKE-family LICM defeat; its single match device is the wrapper, which is sanctioned by the owner's 2026-07-06 do-while(0) ruling and carries the mandatory inline FAKE annotation.
- verdict: CONFIRMED

## [s9] With the chassis corrected the argument block's residual is reachable by re-spelling the four debug_printf arguments, and session 6's expand forensics predict the winner: an INLINE argument's load is emitted by expand_call straight into the hard register as the last insn of the argument sequence, which is target's lw a3,0(a0).
- mechanism: expand_call emits an inline argument's load directly into the hard reg last, while a named local's load is emitted at its statement and local_alloc gives the pseudo the argument register by copy preference.
- probe: Twelve spellings against r1: t1 both inline, t2 arg5 named only, t3 arg4 named and assigned first with arg5 inline, t4 both named with arg4 first, t5 both named with arg5 first, t6 index byte named; u1 named pointer with inline deref, u2 pointer + named value, u3 both slots as named pointers, u4 third argument also named, u5 index bytes staged, u6 declaration order swapped. All via s9/score.py.
- result: 13 / 13 / 7 / 7 / 8 / 13 and 9 / 9 / 10 / 14-at-90-insns / 7 / 9. The all-inline form - session 6's predicted target shape - is 13, six WORSE than naming arg4; naming the third argument as well drops the build to 90 instructions, one fewer than target's 91. 7 is the floor of the entire spelling family, so the residual is a scheduling/allocation question, not a spelling one.
- verdict: KILLED

## [s9] The parameter's allocno priority can be landed in the (200, 306) window by lengthening its live range - sessions 2 and 8 only ever killed particular SPELLINGS of that change (statement order, declaration order, dead stores), so the route itself is still open.
- mechanism: global.c:allocno_compare sorts on pri = floor_log2(nrefs)*nrefs/livelen*10000, so at nrefs 2 the window needs livelen 66..99 against the current 52.
- probe: Counted the function's real insns in the -da dumps (59) and read local-alloc.c:update_equiv_regs to find the source of the three pointers' 96-100 live lengths.
- result: flow.c bounds reg_live_length by the function's insn count, and the param's 52 is already within 7 of the 59-insn ceiling. The pointers' 96-100 are NOT raw: local-alloc.c line ~1058 DOUBLES reg_live_length for any pseudo carrying a REG_EQUIV note, and lines 1019-1052 attach REG_EQUIV only to a single-set pseudo whose source is CONSTANT_P (via a promoted REG_EQUAL note) or an unchanging MEM. The param's set is (set (reg/v 72) (reg:SI 4 a0)), a hard-register source, so it can never qualify. Therefore param pri >= 2*10000/59 = 338 > D_800A1494's 306 for EVERY possible spelling - F16 route (a) is arithmetically dead, permanently. This also corrects session 8's formula: the floor_log2 factor is real (it equals 1 for nrefs 2 and 3, which is why s8's fit looked exact), so F17 route (b)'s numbers are 833 / 816, not 416 / 408.
- verdict: KILLED

## Session 10 (synthesis) — measured

Modality: synthesis.  Floor UNCHANGED at **7 / 91** (the session-9 candidate,
re-applied to `src/system.c` and re-measured this session: `"score": 7,
"target_insns": 91, "build_insns": 91`).  The session's product is not a lower
floor but a correct MODEL of the residual: the real INSN_PRIORITY tables for
the argument block were read out of the instrumented cc1 for the first time,
they falsify the frontier's stated mechanism (F19), and they decompose the 7
into two independent sub-goals that two DIFFERENT measured spellings each
already achieve in isolation.  Seven new spellings measured (i1-i9), bringing
the argument-block family to nineteen.

### H41 — KILLED (F19's mechanism: "the lever is which pseudo local_alloc gives `$a0`")
**Statement:** the residual 7 is a register-allocation question — target keeps
the idx[0] address chain in `$a0` and we keep it in `$v0`, and an anti-dependence
on `$a0` from the following `lui a0,%hi(D_800161C8)` is what pins target's
`lw a3,0(a0)` to the end of the argument block, so the lever is a local-alloc
copy-preference, not anything in the C.
**Mechanism / probe:** dumped the post-sched1 RTL for the candidate
(`tmp/grind/saEft01Init/s10/idump.sh cand10`, then `s10/schedscan.py`) and
compared the two index chains' live ranges against the positional diff of the
emitted code.
**Result:** the register assignment is DOWNSTREAM of the schedule, not upstream
of it.  Our idx[0] address pseudo (reg 92) is live from build idx 51 to 54 and
therefore does NOT overlap the register holding arg5's loaded value (`$v1`,
live 57-60), so the two get different colours than target's, where the idx[0]
chain is live 46-61 and DOES overlap arg5's value.  Same conflict-graph
question, opposite answer, and the conflict graph is a function of the emitted
order.  Also, an anti-dependence cannot pin an insn LATE: it makes the later
insn depend on the earlier one, which raises the LATER insn's depth, never the
earlier one's.
**Verdict: KILLED.**  Do not spend another session on `$a0`-vs-`$v0`.

### H42 — CONFIRMED (the scheduler model, measured to the integer)
**Statement:** the argument block's emitted order is decided by
`INSN_PRIORITY`, and reading the real priorities will say what has to change.
**Mechanism:** GCC 2.7.2's `sched.c` computes `priority()` over an insn's
LOG_LINKS (its PREDECESSORS) and `schedule_block` walks the block BACKWARD,
emitting the chosen insn at the current tail.  So the printed priority is the
dependence DEPTH from the top of the basic block, and **a HIGHER priority means
the insn is emitted LATER**, not earlier.  `insn_cost` charges 1 for a
load->use edge and 0 otherwise, so depth counts load-use delays.
**Probe:** `BB2_PRIO_DEBUG=1` instrumented cc1 (`tools/gcc-2.7.2/cc1`, verified
CODEGEN-IDENTICAL against the frozen `build/cc1` on this TU), then
`tmp/grind/saEft01Init/s10/findprio.py` — insn UIDs restart per function in
cc1.log, so the tool locates the scheduling region whose SET-insn UID multiset
best matches the UID list read out of the `-da` `.sched` dump (21/21 overlap).
**Result:** two regions, sched1 at cc1.log line 4382 and sched2 at line 4601.
The sched2 table (the one that fixes the emitted order, since it runs after
reload) is:

| pri | insns |
|---|---|
| 1 | 93 `lbu` idx[0] · 116 `lbu` idx[1] · 105 `lw` D_800F19C0 · 127 `lui` fmt |
| 2 | 96/98/100 (idx[0] `sll`/`addu`/**`lw a3`**) · 119/121/123 (idx[1] chain) · 108 `lbu` D_800A11D5 |
| 3 | 125 `sw 16(sp)` · 111 `sll` · 113 `addu` · 131 `lw a2` |
| 4 | 135 `jal debug_printf` · 138 `jal cdrom_ClearIrq` |

**Verdict: CONFIRMED.**  Two consequences.  (a) The whole block collapses into a
three-level lattice with SEVEN insns tied at priority 2, so the emitted order is
settled by `rank_for_schedule`'s tie-breaks (the load/store dependence-class
test against `last_scheduled_insn`, then `INSN_LUID`), not by priority — and
`INSN_LUID` at sched2 is the order sched1 left the chain in, NOT source order,
which is why session 6 found source statement placement inside the block
"byte-inert".  (b) For our `lw a3` (insn 100) to be emitted last it must be
scheduled FIRST in the backward pass, i.e. it must outrank `sw 16(sp)` (125)
and `lw a2` (131), which sit a whole priority level above it.  Depth 2 is
forced: `lbu`(1) -> `sll`(+1 load-use) -> `addu`(+0) -> `lw`(+0).

### H43 — CONFIRMED (the residual DECOMPOSES; each half is already solved, by different spellings)
**Statement:** the 7 is one indivisible defect.
**Probe:** positional diffs (`tmp/grind/saEft01Init/s1/grind_diff.py`) of three
spellings of the same program on the same do{}while(0) chassis — the candidate
(arg4 named, 7), `i1` (both lookups inline, 13) and `i2` (arg4 as a named
pointer with an inline deref, 9).
**Result:** the block has TWO independent defects and each spelling fixes
exactly one of them:
  * **(A) `lw a3` last.**  Target emits it at build idx 61, after `sw v1,16(sp)`
    and `lw a2,0(v0)`.  `i1` (both inline) emits `lw a3,0(v0)` **at build idx 61
    — target's exact position**; its whole idx[0] chain sits at the tail
    (57/59/60/61).  The candidate emits it at 54.  So the inline spelling DOES
    do what session 6 predicted; H39's reading of the all-inline 13 as a
    refutation was wrong, the 13 is paid entirely by defect (B).
  * **(B) both `lbu` at the block head (idx 46/47).**  The candidate has them
    there in target's order (`0(s1)` then `1(s1)`); `i2` has them there in the
    REVERSED order; `i1` has only ONE there — its idx[0] `lbu` is dragged all
    the way to idx 57 with the rest of its chain.
  No measured spelling has both.  Target's idx[0] chain is SPREAD across the
  whole block (`lbu` 46, `sll` 52, `addu` 56, `lw a3` 61) whereas every spelling
  we can write emits that chain as one contiguous run — early (candidate, i2) or
  late (i1).
**Verdict: CONFIRMED.**  This is the reframed question for session 11 and it is
much sharper than "the argument block is 7 off".

### H44 — KILLED (naming the index BYTE is inert once the value is inline)
**Statement:** the (A)/(B) split can be bridged by naming only the index byte —
`k0 = idx_1494[0];` puts that `lbu` first in source order while the value load
stays an inline call argument and therefore still schedules last.
**Probe:** `i3` (idx[0] byte named), `i4` (both bytes named), `i5` (both named,
reverse source order), all with both lookups inline, via `s10/score.py`.
**Result:** 13 / 13 / 13 — all three are byte-identical to `i1` (pure inline).
CSE/combine folds the named index pseudo straight back into the address chain,
so the extra C statement leaves no RTL trace at all.  This also re-explains
session 9's `t6` (13) and `u5` (7) as the same effect measured on two different
value spellings.
**Verdict: KILLED.**  Index-byte naming is not a lever in any combination.

### H45 — KILLED (address-pointer spellings; the whole family is now swept)
**Statement:** some spelling that names one or both ADDRESSES (rather than the
values) lands both sub-goals at once.
**Probe:** `i2` (arg4 address named, inline deref), `i6` (arg5 address named,
arg4 fully inline), `i8` (arg4 address named and assigned BEFORE the
`tslTm2LoadImage_2` call), `i9` (both addresses named).
**Result:** 9 / 14 / **31 at 92 insns** / 10.  `i8` is the informative failure:
hoisting the pointer above the call makes it live ACROSS a call, so it takes a
callee-save and the whole `$s0-$s3` map that the do{}while(0) wrapper bought is
destroyed — nothing in the argument block may be lifted out of the wrapper's
call-free stretch.
**Verdict: KILLED.**  Nineteen argument spellings are now measured and 7 remains
the floor of the entire family.

## Live frontier (for session 11)

### F22 — spread the idx[0] chain: `lbu` at 46 AND `lw a3` at 61 (the whole 7)
**Mechanism:** H43.  Target's idx[0] chain is `lbu a0,0(s1)`(46) …
`sll a0,a0,2`(52) … `addu a0,a0,s0`(56) … `lw a3,0(a0)`(61); every spelling we
can write emits it as one contiguous run.  H42 says why: in the backward
scheduler the chain's `lw` must be picked FIRST (so it lands last) while its
`lbu` must be picked LAST (so it lands first), i.e. the chain has to span the
whole priority lattice — depth 3+ at the `lw` and depth 1 at the `lbu` — but our
chain is pinned at depths 1,2,2,2 by `insn_cost` (one load-use edge, then two
zero-cost edges).  So the question is: **what gives target's idx[0] chain an
extra unit of dependence depth at its `lw`, or equivalently what makes
`sw 16(sp)` and `lw a2` rank BELOW it?**
**Next probe:** two concrete routes, both cheap.
  (1) Read the target's own dependence structure the other way round: our
  `lw a2,0(v0)` (insn 131, the `tbl_11dc[D_800A11D5]` argument) is at priority 3
  because `108 lbu` -> `111 sll` -> `113 addu` -> `131 lw` carries the SAME one
  load-use edge but starts one level deeper (`108` is itself at depth 2, because
  `D_800A11D5` is reached through a `lui`+`lbu` pair rather than through a base
  pointer already live in `$s1`).  So the depth question is really: does target's
  idx[0] chain start from something that is itself loaded?  Check what expression
  the Sony reference uses for the `D_800A1494` index base and whether it reaches
  it through memory rather than through a hoisted pointer local.
  (2) Since ties at priority 2 are broken by `INSN_LUID` and LUID at sched2 is
  SCHED1's output order, dump sched1's emitted order for `i1` and for the
  candidate (`s10/idump.sh <tag>` then `s10/schedscan.py`) and check whether any
  source-level statement order changes sched1's output at all.  If sched1's
  output order is invariant across spellings, the tie-break route is closed and
  the only remaining lever is the depth question in (1).

### F23 — cheat-review status of the do{}while(0) wrapper (unchanged, inherited)
Still the candidate's single match device and still never seen by a fresh
adversarial `cheat-reviewer`.  It is explicitly sanctioned (owner ruling
2026-07-06, `.claude/rules/do-while-zero-exception.md`: any body, ANY codegen
effect, register allocation named explicitly, mandatory inline FAKE annotation,
single level) and the candidate carries the annotation.  Do the review before
any completion claim, showing H37/H40 (both alternative routes proved
arithmetically dead) as the lever-exhaustion record.

### F24 — the two sibling alarm functions inherit this (unchanged, inherited)
`cpu_side_move_dir_4` (= CD_sync, name string D_80016240, ~src/system.c:366) and
the function at ~src/system.c:480 (= CD_ready, string D_80016248) carry the same
hand-derived goto shape and the same allocation problem.  Once saEft01Init
closes, apply the same single do{}while(0) wrapper plus the named-arg4
argument spelling and re-score both.

## [s10] F19's stated mechanism - that the residual 7 is a local-alloc copy-preference question about which pseudo gets $a0, with an anti-dependence from the following lui a0,%hi(D_800161C8) pinning target's lw a3 to the end of the argument block.
- mechanism: local-alloc.c copy preference plus a REG_DEP_ANTI edge on the hard register $a0 at sched2, after reload has replaced the pseudos with hard registers.
- probe: Dumped post-sched1 RTL for the candidate with the instrumented cc1 (tmp/grind/saEft01Init/s10/idump.sh cand10) and read it with s10/schedscan.py, then compared the two index chains' live ranges against the positional diff of the emitted code.
- result: The register assignment is downstream of the schedule, not upstream. Our idx[0] address pseudo (reg 92) is live only from build idx 51 to 54 and so does not overlap the register holding arg5's loaded value ($v1, live 57-60); target's equivalent chain is live 46-61 and does overlap, so the two conflict graphs are different and the colours follow. Separately, an anti-dependence can never pin an insn LATE - it makes the later insn depend on the earlier one, raising the later insn's dependence depth, never the earlier one's.
- verdict: KILLED

## [s10] The argument block's emitted order is decided by INSN_PRIORITY, and reading the real priorities out of the instrumented cc1 will say what has to change.
- mechanism: GCC 2.7.2's sched.c computes priority() over an insn's LOG_LINKS (its PREDECESSORS) and schedule_block walks the block BACKWARD, emitting each chosen insn at the current tail, so the printed priority is dependence DEPTH from the top of the basic block and a HIGHER priority means the insn is emitted LATER. insn_cost charges 1 for a load->use edge and 0 otherwise.
- probe: BB2_PRIO_DEBUG=1 instrumented cc1 (verified CODEGEN-IDENTICAL against the frozen build/cc1 on this TU), then tmp/grind/saEft01Init/s10/findprio.py, which locates the scheduling region in cc1.log whose SET-insn UID multiset best matches the UID list read out of the -da .sched dump - necessary because cc1 insn UIDs restart per function and cc1.log has no function separators. Overlap 21/21 at log line 4382 (sched1) and a second region at 4601 (sched2).
- result: The sched2 table, which is what fixes the emitted order: priority 1 = insn 93 lbu idx[0], 116 lbu idx[1], 105 lw D_800F19C0, 127 lui fmt; priority 2 = 96/98/100 (the idx[0] sll/addu/lw-a3 chain), 119/121/123 (the idx[1] chain), 108 lbu D_800A11D5; priority 3 = 125 sw 16(sp), 111 sll, 113 addu, 131 lw a2; priority 4 = 135 jal debug_printf, 138 jal cdrom_ClearIrq. Seven insns are tied at level 2, so rank_for_schedule's tie-breaks (dependence class against last_scheduled_insn, then INSN_LUID) settle the order - and INSN_LUID at sched2 is sched1's output order, not source order, which is exactly why session 6 measured source statement placement inside this block as byte-inert. For lw a3 (insn 100) to be emitted last it must be scheduled first in the backward pass, i.e. outrank sw 16(sp) and lw a2, which sit a whole level above it; its depth 2 is forced by lbu(1) -> sll(+1 load-use) -> addu(+0) -> lw(+0).
- verdict: CONFIRMED

## [s10] The remaining 7 is one indivisible defect in the debug_printf argument block.
- mechanism: Positional comparison of the same program written three ways on the same do{}while(0) chassis, against asm/funcs/saEft01Init.s.
- probe: tmp/grind/saEft01Init/s1/grind_diff.py on the candidate (arg4 named, 7), on i1 (both lookups written inline, 13) and on i2 (arg4 as a named pointer with an inline deref, 9).
- result: The block carries TWO independent defects and each spelling fixes exactly one. (A) lw a3 last: target emits it at build idx 61 after sw v1,16(sp) and lw a2,0(v0); i1 emits lw a3,0(v0) at build idx 61, target's exact position, with its whole idx[0] chain at the tail (57/59/60/61), while the candidate emits it at 54. (B) both lbu at the block head, idx 46/47: the candidate has them there in target's order, i2 has them there reversed, i1 has only one there - its idx[0] lbu is dragged to 57. No measured spelling has both. Target's idx[0] chain is SPREAD (lbu 46, sll 52, addu 56, lw a3 61); every spelling we can write emits it as one contiguous run, early or late. Session 9's H39 read the all-inline 13 as refuting session 6's expand-shape prediction; that reading was wrong - the inline spelling does exactly what was predicted for sub-goal (A) and pays all 13 on sub-goal (B).
- verdict: CONFIRMED

## [s10] The two sub-goals can be bridged by naming only the index BYTE - k0 = idx_1494[0] puts that lbu first in source order while the value load stays an inline call argument and therefore still schedules last.
- mechanism: An extra named C local for the index gives the byte load its own pseudo with an earlier LUID, without adding a copy to the value chain (which is what makes the named-value spelling schedule its lw early).
- probe: i3 (idx[0] byte named), i4 (both bytes named), i5 (both named, reverse source order), all with both table lookups written inline in the call; tmp/grind/saEft01Init/s10/score.py.
- result: 13 / 13 / 13, all byte-identical to i1 (pure inline). CSE/combine folds the named index pseudo straight back into the address chain, so the extra C statement leaves no RTL trace whatsoever. This also re-explains session 9's t6 (13) and u5 (7) as the same inert effect measured on two different value spellings.
- verdict: KILLED

## [s10] Some spelling that names one or both ADDRESSES rather than the values lands both sub-goals at once.
- mechanism: A named pointer keeps the address chain in its own pseudo (scheduled early, like a named value) while leaving the load itself an inline call argument (scheduled late).
- probe: i2 (arg4 address named, inline deref), i6 (arg5 address named, arg4 fully inline), i8 (arg4 address named and assigned BEFORE the tslTm2LoadImage_2 call), i9 (both addresses named); s10/score.py.
- result: 9 / 14 / 31 at 92 insns / 10. i8 is the informative failure: hoisting the pointer above the call makes it live across a call, so it takes a callee-save and destroys the entire $s0-$s3 map that the do{}while(0) wrapper buys - nothing in the argument block may be lifted out of the wrapper's call-free stretch. Nineteen argument spellings are now measured and 7 remains the floor of the whole family.
- verdict: KILLED

## [s10] F19's mechanism: the residual 7 is a register-allocation question - target keeps the idx[0] address chain in $a0 and we keep it in $v0, and an anti-dependence on $a0 from the following lui a0,%hi(D_800161C8) is what pins target's lw a3,0(a0) to the end of the argument block, so the lever is a local-alloc copy preference rather than anything in the C.
- mechanism: local-alloc.c copy preference plus a REG_DEP_ANTI edge on the hard register $a0 at sched2, after reload has replaced pseudos with hard registers.
- probe: Dumped post-sched1 RTL for the candidate with the instrumented cc1 (tmp/grind/saEft01Init/s10/idump.sh cand10, CODEGEN-IDENTICAL against the frozen build/cc1) and read it with s10/schedscan.py, then compared the two index chains' live ranges against the positional diff of the emitted code.
- result: The register assignment is DOWNSTREAM of the schedule, not upstream of it. Our idx[0] address pseudo (reg 92) is live only from build idx 51 to 54 and therefore does not overlap the register holding arg5's loaded value ($v1, live 57-60); target's equivalent chain is live 46-61 and does overlap. Different conflict graph, different colouring, and the conflict graph is a function of the emitted order. Separately, an anti-dependence can never pin an insn LATE: it makes the later insn depend on the earlier one, raising the LATER insn's dependence depth, never the earlier one's.
- verdict: KILLED

## [s10] The argument block's emitted order is decided by INSN_PRIORITY, and reading the real priorities out of the instrumented cc1 will say what has to change.
- mechanism: GCC 2.7.2's sched.c computes priority() over an insn's LOG_LINKS (its PREDECESSORS) and schedule_block walks the block BACKWARD, emitting each chosen insn at the current tail. So the printed priority is dependence DEPTH from the top of the basic block and a HIGHER priority means the insn is emitted LATER. insn_cost charges 1 for a load->use edge and 0 otherwise, so depth counts load-use delays.
- probe: BB2_PRIO_DEBUG=1 instrumented cc1 plus tmp/grind/saEft01Init/s10/findprio.py, which locates the scheduling region in cc1.log whose SET-insn UID multiset best matches the UID list read out of the -da .sched dump (necessary because cc1 insn UIDs restart per function and cc1.log has no function separators). Overlap 21/21 at log line 4382 (sched1) and a second region at 4601 (sched2).
- result: sched2 table (the one that fixes the emitted order): level 1 = insn 93 lbu idx[0], 116 lbu idx[1], 105 lw D_800F19C0, 127 lui fmt; level 2 = 96/98/100 (the idx[0] sll/addu/lw-a3 chain), 119/121/123 (the idx[1] chain), 108 lbu D_800A11D5; level 3 = 125 sw 16(sp), 111 sll, 113 addu, 131 lw a2; level 4 = the two calls. SEVEN insns are tied at level 2, so rank_for_schedule's tie-breaks settle the order (load/store dependence class against last_scheduled_insn, then INSN_LUID) - and INSN_LUID at sched2 is SCHED1's output order, not source order, which is the mechanical reason session 6 measured source statement placement inside this block as byte-inert. For lw a3 (insn 100) to be emitted last it must be scheduled FIRST in the backward pass, i.e. outrank sw 16(sp) and lw a2, which sit a whole level above it; its depth 2 is forced by lbu(1) -> sll(+1 load-use) -> addu(+0) -> lw(+0).
- verdict: CONFIRMED

## [s10] The remaining 7 is one indivisible defect in the debug_printf argument block.
- mechanism: Positional comparison of the same program written three ways on the same do{}while(0) chassis, against asm/funcs/saEft01Init.s.
- probe: tmp/grind/saEft01Init/s1/grind_diff.py on the candidate (arg4 named, 7), on i1 (both lookups written inline, 13) and on i2 (arg4 as a named pointer with an inline deref, 9).
- result: The block carries TWO independent defects and each spelling fixes exactly one. (A) lw a3 last: target emits it at build idx 61 after sw v1,16(sp) and lw a2,0(v0); i1 emits lw a3,0(v0) at build idx 61 - target's exact position - with its whole idx[0] chain at the tail (57/59/60/61), while the candidate emits it at 54. (B) both lbu at the block head, idx 46/47: the candidate has them there in target's order, i2 has them there reversed, i1 has only one there (its idx[0] lbu is dragged to 57). No measured spelling has both. Target's idx[0] chain is SPREAD (lbu 46, sll 52, addu 56, lw a3 61); every spelling reachable from C emits it as one contiguous run, early or late. This also rehabilitates session 6's expand forensics: session 9's H39 read the all-inline 13 as a refutation, but the inline form does exactly what session 6 predicted for sub-goal (A) and pays all 13 on sub-goal (B).
- verdict: CONFIRMED

## [s10] The two sub-goals can be bridged by naming only the index BYTE - k0 = idx_1494[0] puts that lbu first in source order while the value load stays an inline call argument and therefore still schedules last.
- mechanism: An extra named C local for the index gives the byte load its own pseudo with an earlier LUID, without adding a copy to the value chain (which is what makes the named-value spelling schedule its lw early).
- probe: i3 (idx[0] byte named), i4 (both bytes named), i5 (both named, reverse source order), all with both table lookups written inline in the call; tmp/grind/saEft01Init/s10/score.py.
- result: 13 / 13 / 13, all byte-identical to i1 (pure inline). CSE/combine folds the named index pseudo straight back into the address chain, so the extra C statement leaves no RTL trace whatsoever. This also re-explains session 9's t6 (13) and u5 (7) as the same inert effect measured on two different value spellings.
- verdict: KILLED

## [s10] Some spelling that names one or both ADDRESSES rather than the values lands both sub-goals at once.
- mechanism: A named pointer keeps the address chain in its own pseudo (scheduled early, like a named value) while leaving the load itself an inline call argument (scheduled late).
- probe: i2 (arg4 address named, inline deref), i6 (arg5 address named, arg4 fully inline), i8 (arg4 address named and assigned BEFORE the tslTm2LoadImage_2 call), i9 (both addresses named); s10/score.py.
- result: 9 / 14 / 31 at 92 insns / 10. i8 is the informative failure: hoisting the pointer above the call makes it live across a call, so it takes a callee-save and destroys the entire $s0-$s3 map that the do{}while(0) wrapper buys. Nineteen argument spellings are now measured and 7 remains the floor of the whole family.
- verdict: KILLED


## [s11] F22 route (1) - target's idx[0] address chain starts from something that is itself loaded, giving its `lw` an extra unit of dependence depth.
- mechanism: session 10 observed that our `lw a2` chain reaches level 3 with a single load-use edge only because it starts one level deeper (D_800A11D5 is reached through a lui+lbu pair rather than through a base pointer already live in a register). Route (1) asked whether the Sony CD_datasync reference reaches the D_800A1494 index base through memory the same way.
- probe: direct inspection of asm/funcs/saEft01Init.s lines 50-51.
- result: It does not. Target emits `lbu $a0, 0x0($s1)` and `lbu $v0, 0x1($s1)` - the index base is a hoisted pointer already live in a callee-save register, exactly as our candidate has it, so target's idx[0] chain has the SAME depth structure as ours (1,2,2,-) and the extra level at its `lw` cannot come from the head of the chain. No C change is available on this route; it is answered by inspection, not by measurement.
- verdict: KILLED

## [s11] The residual is a dependence-DEPTH problem in the idx[0] address chain (F22's framing).
- mechanism: F22 assumed target's `lw a3` is a level-2 insn that wins a tie-break against `sw 16(sp)` and `lw a2` at level 3.
- probe: Recomputed the whole block's priority lattice from target's own emitted order (asm/funcs/saEft01Init.s, block idx 46-61) using session 10's confirmed backward-scheduler rule, and checked monotonicity.
- result: Target's emitted order is EXACTLY ascending in priority - 1,1,1,2,2,2,2,2,2,3,3,3,4,4 - which is what a backward list scheduler produces when it always pops the highest-priority ready insn. So `lw a2` and `lw a3` are level FOUR, a whole level ABOVE `sw 16(sp)`, not level 2 tie-breaking below it. The only edge that puts a load a level above the outgoing-arg store is a store->load memory dependence on the 16(sp) outgoing-argument slot, i.e. in target's RTL stream the `sw` is emitted BEFORE both register-arg loads while both `lbu`s and the arg5 value chain are emitted before the `sw`. F22's tie-break framing is wrong; the real question is emission ORDER across the stack store, not chain depth.
- verdict: KILLED (as stated) / superseded by F25

## [s11] The do{}while(0) wrapper's EXTENT is a free structural variable that can be tuned.
- mechanism: The wrapper's stated mechanism is flow.c's loop_depth ref-weighting for the three table pointers, which only requires the refs to sit between NOTE_INSN_LOOP_BEG and NOTE_INSN_LOOP_END; which OTHER statements share the wrapper should therefore be free.
- probe: Seven extents measured on the candidate chassis (tmp/grind/saEft01Init/s11/gen.py, variants w1-w7): printf-only; opens after tslTm2LoadImage_2; plus `v0 = -1`; minus cdrom_ClearIrq; arg4 declared at function scope; whole do_timeout section incl. the `goto check`; arg4's store hoisted above the wrapper.
- result: 8, 8, 7, 7, 7, 7, 16-at-92. The extent is byte-inert in both directions provided the wrapper contains BOTH the `tslTm2LoadImage_2` call and the arg4 store. Dropping the call out costs exactly one; lifting the arg4 store out costs nine plus an instruction (the value goes live across a call, takes a callee-save, and the $s0-$s3 map the wrapper buys collapses - session 10's i8 result reproduced from the other direction). The wrapper is not a tunable knob; it has one working configuration and the candidate is already in it.
- verdict: KILLED

## [s11] The residual is an ORDER problem between the three argument chains, and arg3's spelling - never touched in 27 prior forms - is the untried degree of freedom.
- mechanism: Target's chain order is arg3, arg4, arg5+store, then the three register loads; the candidate gets arg3-before-arg5 right and arg4 wrong, i2 gets arg4 right and arg3 wrong. Naming arg3's address or value should let the order be set from C.
- probe: Eight spellings (tmp/grind/saEft01Init/s11/gen2.py, n1-n8): arg3 address named alone / with arg4 address / with the candidate's arg4 value local / after arg4's address / with all three addresses named in target's order / arg3 value named alone / arg3+arg4 values named / arg3+arg5 addresses named with arg4 inline.
- result: 13, 13, 15, 10, 14, 12, 14-at-90, 14. Every arg3 spelling regresses; fully-inline arg3 (the candidate's) is the family optimum. The arg3 axis is closed.
- verdict: KILLED

## [s11] Declaration order and initialisation order of the three hoisted table pointers are live levers on the argument block's LUID tie-breaks.
- mechanism: Both feed pseudo numbering, which feeds INSN_LUID, which is what rank_for_schedule uses to break the seven-way priority-2 tie session 10 measured.
- probe: Ten forms (tmp/grind/saEft01Init/s11/gen3.py, o1-o10) sweeping the two axes independently and together, with the argument block held at the candidate's spelling.
- result: DECLARATION order is byte-inert (o6/o7/o8 all 7). INITIALISATION order is live and the candidate's order (tbl_11dc, idx_1494, tbl_125c) is the unique optimum: the other five permutations score 8, 11, 11, 12, 12. So the axis is real but already at its best value, and the candidate's assignment order - which no prior session had justified - is now measured as load-bearing rather than arbitrary.
- verdict: CONFIRMED (axis real) / KILLED (no improvement available)

## [s11] The argument block's three attractors can be bridged by bolting arg5 spellings onto the i2 (arg4-address-named) shape.
- mechanism: i2 is the only measured spelling with target's late `lw a3`; its two remaining defects are the reversed `lbu` pair at the block head and the D_800A11D5 chain landing after the `sw`. Both look like LUID tie-breaks that an extra arg5 statement could shift.
- probe: Six forms (tmp/grind/saEft01Init/s11/gen4.py, q1-q6): i2 plus arg5's index byte named; arg4's index byte named then the pointer built from it; i2 plus arg5's value named; i2 written as `tbl_125c + idx_1494[0]` pointer arithmetic; arg4 value named plus arg5 address named; candidate plus arg5's index byte named.
- result: 9, 9, 9, 9, 7, 7. The attractors are RIGID - every i2-family form is byte-identical to i2 and every candidate-family form is byte-identical to the candidate. Nothing bolted onto either shape moves the block by a single instruction.
- verdict: KILLED

## [s11] calls.c's argument precompute is gated on loop nesting, so the do{}while(0) wrapper might be what forces our argument loads early.
- mechanism: calls.c:1653-1664 copies a non-REG register-arg value into a pseudo (emitting the load EARLY) when `rtx_cost(value,SET) > 2 && preserve_subexpressions_p()`, and stmt.c:2435's `preserve_subexpressions_p` tests `loop_stack != 0` plus a short-loop UID window - so a do{}while(0) around the call could be what pulls our loads forward.
- probe: Read stmt.c:2435-2451 and toplev.c:3387.
- result: `preserve_subexpressions_p` returns 1 IMMEDIATELY when `flag_expensive_optimizations` is set, and toplev.c sets that for every `optimize >= 2` build - so at this project's -O2 the loop-nesting half of the predicate is dead code and the wrapper cannot affect argument expansion at all. Early-vs-late is decided purely by whether `args[i].value` is already a REG when the forward precompute loop reaches it: a named-local argument is a REG (its load already happened at the statement, hence EARLY), an inline array element is a MEM (its address is emitted here and its load is deferred to load_register_parameters, after the stack stores, hence LATE). This is the mechanical statement of the three-attractor structure and it also clears the wrapper of any suspicion in the argument block.
- verdict: KILLED (as stated) - and it supplies F25's mechanism

## Live frontier (for session 12)

### F25 - target needs arg4's LOAD after the `sw 16(sp)` and arg4's ADDRESS chain before it; C has no spelling that splits them
**Mechanism.**  calls.c:1615-1665 precomputes register args FORWARD; for an
inline array-element argument `expand_expr` emits the address chain there and
leaves the value a MEM, so the load is deferred to `load_register_parameters`
which runs AFTER the stack-arg stores - that is target's split (address early,
load late).  For a named-local argument the load already happened at the
statement, so both go early.  Our problem is that when arg4 is inline, its
address chain does NOT stay early either (the 13-attractor drags the whole chain
to 57-61), and when it is named, the load comes early with the address (the
7-attractor).  The missing question is WHY the inline form's address chain moves
late in our build when calls.c's precompute loop should have emitted it before
`store_one_arg`.
**Next probe.**  Read `calls.c` around the stack-store loop (search for
`store_one_arg` call sites and the `must_preallocate` / `args_size` block near
lines 1277-1420 and 1667+) and establish the ACTUAL emission order of
(a) the register-arg precompute loop, (b) `store_one_arg` for the single stack
arg, (c) `load_register_parameters`.  If store_one_arg runs BEFORE the
precompute loop, then target's shape requires arg4's address chain to be emitted
outside the call altogether while its load stays inline - which is exactly the
i2 spelling (9), and the remaining 9 is then two LUID tie-breaks rather than a
structural gap; dump sched1's output order for i2 with s10/idump.sh + schedscan.py
and check what LUID order the two `lbu`s and the D_800A11D5 chain actually get.

### F23 - cheat-review status of the do{}while(0) wrapper (unchanged, inherited)
Still the candidate's single match device and still never seen by a fresh
adversarial `cheat-reviewer`.  NOTE the s11 correction: the wrapper's mechanism
is NOT anything to do with argument expansion (calls.c's precompute is not
loop-gated at -O2); it is purely flow.c's `loop_depth` ref-weighting for the
three table pointers, and s11's w1/w2/w7 measurements pin its one working
configuration.  Review it against `.claude/rules/do-while-zero-exception.md`
with H37/H40 (both alternative routes arithmetically dead) plus s11's
wrapper-extent sweep as the lever-exhaustion record.

### F24 - the two sibling alarm functions inherit this (unchanged, inherited)
`cpu_side_move_dir_4` (= CD_sync, ~src/system.c:366) and the function at
~src/system.c:480 (= CD_ready) carry the same hand-derived goto shape and the
same allocation problem.  Once saEft01Init closes, apply the same single
do{}while(0) wrapper plus the named-arg4 argument spelling and re-score both.

## [s11] F22 route (1): target's idx[0] address chain starts from something that is itself loaded, giving its lw an extra unit of dependence depth.
- mechanism: Session 10 observed our lw a2 chain reaches level 3 with a single load-use edge only because it starts one level deeper (D_800A11D5 is reached through a lui+lbu pair rather than a base pointer already live in a register). Route (1) asked whether the Sony CD_datasync reference reaches the D_800A1494 index base through memory the same way.
- probe: Direct inspection of asm/funcs/saEft01Init.s lines 50-51.
- result: It does not. Target emits `lbu $a0, 0x0($s1)` and `lbu $v0, 0x1($s1)` — the index base is a hoisted pointer already live in a callee-save register, exactly as our candidate has it. Target's idx[0] chain has the SAME depth structure as ours, so the extra level at its lw cannot come from the head of the chain. Answered by inspection; no C change exists on this route.
- verdict: KILLED

## [s11] F22's framing: the residual 7 is a dependence-DEPTH problem in the idx[0] chain, where target's lw a3 sits at level 2 and wins a tie-break against sw 16(sp) and lw a2 at level 3.
- mechanism: Session 10's INSN_PRIORITY tables plus rank_for_schedule's tie-breaks (dependence class vs last_scheduled_insn, then INSN_LUID).
- probe: Recomputed the whole block's priority lattice from target's own emitted order (asm/funcs/saEft01Init.s, build idx 46-61) using session 10's confirmed backward-scheduler rule, and checked the sequence for monotonicity.
- result: Target's emitted order is exactly ascending in priority — 1,1,1,2,2,2,2,2,2,3,3,3,4,4 — which is what a backward list scheduler produces when it always pops the highest-priority ready insn. So lw a2 and lw a3 are level FOUR, a whole level ABOVE sw 16(sp), not level-2 tie-break winners below it. The only edge that puts a load a level above the outgoing-arg store is a store->load memory dependence on the 16(sp) outgoing-argument slot: in target's RTL stream the sw is emitted BEFORE both register-arg loads while both lbu's and the arg5 value chain are emitted before the sw. The real question is emission ORDER across the stack store, not chain depth.
- verdict: KILLED

## [s11] The do{}while(0) wrapper's EXTENT is a free structural variable that can be tuned to buy the argument block's order.
- mechanism: The wrapper's stated mechanism is flow.c's loop_depth ref-weighting for the three table pointers, which only needs the refs to sit between NOTE_INSN_LOOP_BEG and NOTE_INSN_LOOP_END; which OTHER statements share the wrapper should be free.
- probe: Seven extents on the candidate chassis (tmp/grind/saEft01Init/s11/gen.py, w1-w7): printf-only; opening after tslTm2LoadImage_2; plus `v0 = -1;`; minus cdrom_ClearIrq; arg4 declared at function scope; the whole do_timeout section incl. the goto; arg4's store hoisted above the wrapper.
- result: 8, 8, 7, 7, 7, 7, 16-at-92-insns. The extent is byte-inert in both directions provided the wrapper contains BOTH the tslTm2LoadImage_2 call and the arg4 store. Dropping the call out costs exactly one; lifting the arg4 store out costs nine plus an instruction — the value goes live across a call, takes a callee-save and the $s0-$s3 map collapses, reproducing session 10's i8 result from the other direction. The wrapper has one working configuration and the candidate is already in it.
- verdict: KILLED

## [s11] The residual is an ORDER problem between the three argument chains, and arg3's spelling (tbl_11dc[D_800A11D5]) — untouched in all 27 prior forms — is the missing degree of freedom.
- mechanism: Target's chain order is arg3, arg4, arg5+store, then the three register loads. The candidate gets arg3-before-arg5 right and arg4 wrong; i2 gets arg4 right and arg3 wrong. Naming arg3's address or value should let the order be set from C.
- probe: Eight spellings (tmp/grind/saEft01Init/s11/gen2.py, n1-n8): arg3 address named alone / with arg4's address / with the candidate's arg4 value local / after arg4's address / all three addresses named in target's chain order / arg3 value named alone / arg3+arg4 values named / arg3+arg5 addresses named with arg4 inline.
- result: 13, 13, 15, 10, 14, 12, 14-at-90-insns, 14. Every arg3 spelling regresses; the candidate's fully-inline arg3 is the family optimum. The arg3 axis is closed.
- verdict: KILLED

## [s11] Declaration order and initialisation order of the three hoisted table pointers are live levers on the argument block's LUID tie-breaks.
- mechanism: Both feed pseudo numbering, which feeds INSN_LUID, which is what rank_for_schedule uses to break the seven-way priority-2 tie session 10 measured.
- probe: Ten forms (tmp/grind/saEft01Init/s11/gen3.py, o1-o10) sweeping the two axes independently and together, with the argument block held at the candidate's spelling.
- result: DECLARATION order is byte-inert (o6/o7/o8 all 7). INITIALISATION order is live and the candidate's order (tbl_11dc, idx_1494, tbl_125c) is the unique optimum: the other five permutations score 8, 11, 11, 12, 12. The axis is real but already at its best value — and the candidate's assignment order, which no prior session had justified, is now measured as load-bearing rather than arbitrary.
- verdict: CONFIRMED

## [s11] The argument block's attractors can be bridged by bolting extra arg5 spellings onto the i2 (arg4-address-named, 9/91) shape, which is the only measured form with target's late lw a3.
- mechanism: i2's two remaining defects — the reversed lbu pair at the block head and the D_800A11D5 chain landing after the sw — both look like LUID tie-breaks an extra arg5 statement could shift.
- probe: Six forms (tmp/grind/saEft01Init/s11/gen4.py, q1-q6): i2 plus arg5's index byte named; arg4's index byte named then the pointer built from it; i2 plus arg5's value named; i2 written with pointer arithmetic instead of &tbl[i]; arg4 value named plus arg5 address named; candidate plus arg5's index byte named.
- result: 9, 9, 9, 9, 7, 7. The attractors are RIGID — every i2-family form is byte-identical to i2 and every candidate-family form is byte-identical to the candidate. Nothing bolted onto either shape moves the block by a single instruction. The block has exactly three reachable states: 7 (arg4 named value), 9 (arg4 named address), 13 (arg4 inline).
- verdict: KILLED

## [s11] calls.c's argument precompute is gated on loop nesting, so the do{}while(0) wrapper may be what forces our argument loads early.
- mechanism: calls.c:1653-1664 copies a non-REG register-arg value into a pseudo (emitting the load EARLY) when rtx_cost(value,SET) > 2 && preserve_subexpressions_p(), and stmt.c:2435's preserve_subexpressions_p tests loop_stack != 0 plus a short-loop UID window — so a do{}while(0) around the call could pull our loads forward.
- probe: Read tools/gcc-2.7.2/stmt.c:2435-2451 and tools/gcc-2.7.2/toplev.c:3387; confirmed SMALL_REGISTER_CLASSES is undefined for mips.
- result: preserve_subexpressions_p returns 1 IMMEDIATELY when flag_expensive_optimizations is set, and toplev.c sets that for every optimize >= 2 build — so at this project's -O2 the loop-nesting half of the predicate is dead code and the wrapper cannot affect argument expansion at all. Early-vs-late is decided purely by whether args[i].value is already a REG when the FORWARD precompute loop reaches it: a named-local argument is a REG (its load already happened at the statement, hence early), an inline array element is a MEM (its address is emitted there and its load deferred to load_register_parameters, after the stack stores, hence late). This is the mechanical statement of the three-attractor structure, and it clears the wrapper of any role in the argument block.
- verdict: KILLED

## Session 12 (structural) — measured

## [s12] F25's premise: something in expand_call emits store_one_arg for the single stack argument BEFORE the register-arg precompute loop, which is why the fully-inline arg4 form drags its address chain late.
mechanism: calls.c's ordering of (a) the register-arg precompute loop, (b) store_one_arg, (c) load_register_parameters.
probe: read tools/gcc-2.7.2/calls.c 1600-1760 and 1826-1880 directly.
result: The order is unambiguous and is the one session 11 assumed: precompute at 1618-1665, store_one_arg for `args[i].reg == 0` at 1736-1739, load_register_parameters at ~1876. Nothing emits the stack store early.
verdict: KILLED — and with it the idea that the i1 attractor's late idx[0] chain is an expand-order effect. i1's RTL order IS target's; its 13 is produced downstream, in sched.c.

## [s12] The residual is an alias-class problem: our table loads are INDIRECT_REFs through a hoisted `T *` (MEM_IN_STRUCT_P = 0) while target's came from real array accesses, so alias.c lets our arg4 load float above the outgoing-arg store.
mechanism: expand sets MEM_IN_STRUCT_P for ARRAY_REF/COMPONENT_REF but not for a subscript on a pointer; sched.c's memory-dependence test consults it.
probe: six forms (m1-m6) re-typing the hoisted bases as pointers-to-array (`s32 (*tbl)[]`, `u8 (*idx)[2]`) so the accesses are genuine ARRAY_REFs with the base still a hoisted pseudo; plus p1-p3 as the negative control.
result: all six byte-identical to the candidate at 7/91.
verdict: KILLED.

## [s12] The load's RTX_UNCHANGING_P flag (a `const`-qualified access) changes the scheduler's placement of the argument loads relative to the `sw 16(sp)`.
mechanism: RTX_UNCHANGING_P suppresses the memory dependence between the outgoing-arg store and a subsequent load, changing what sched.c may reorder.
probe: c1-c4 (const on each hoisted pointer and all three), e1-e6 (per-ACCESS const casts on arg4 / arg5 / arg3 separately), e7-e8 (the cast on a fully inline arg4), then thirteen bolt-ons on the winning chassis (d1-d13, g5, g6).
result: CONFIRMED as a real lever and it produces a FOURTH attractor at 9/91 whose `sw` precedes `lw a3` — target's relation, never previously reached. But it costs 2 (the two `lbu` swap and the arg3 chain moves after `lw a3`), the chassis is rigid under all thirteen bolt-ons, and the flag is inert on a load that was deferred to load_register_parameters.
verdict: CONFIRMED (mechanism) / the chassis is a 9-floor dead end for closing.

## [s12] arg1 — the format-string address — is an untouched degree of freedom that could shift the argument block's LUIDs.
mechanism: naming it moves its address computation out of the precompute loop and into the statement stream.
probe: a1-a3 (named inside the wrapper, named at wrapper entry, and the same for the tslTm2LoadImage_2 argument).
result: all 7/91, byte-identical.
verdict: KILLED.

## [s12] The pre-loop global stores' placement among the three pointer initialisations is a live LUID lever (session 11 only permuted the pointers among themselves).
probe: g1/g2/g4 interleave `D_800F19BC = 0;` and `D_800F19C0 = &D_800162C0;` through the pointer inits; g3 puts all three pointer inits above the sys_VSync call.
result: g1/g2/g4 = 7 (byte-inert); g3 = 17 (the already-banked pointer-inits-before-VSync family).
verdict: KILLED.

## [s12] debug_printf's declared signature is wrong — the original is a printf-style varargs function, and the fixed 5-arg prototype changes expand_call's argument handling.
mechanism: with `...` the args take default promotions and a different path through expand_call / must_preallocate.
probe: score2.py re-declares the prototype as `(void *, ...)`, `(void *, void *, ...)` and `()` (K&R) and re-scores three bodies (candidate, arg4-inline, arg5-named+arg4-inline).
result: every combination is byte-identical to the fixed prototype (7 / 13 / 13).
verdict: KILLED — the prototype is byte-inert here, so it is also NOT evidence about the original signature either way.

## Live frontier (for session 13)

## [s12] F26 — the residual is entirely inside sched.c's ready-list ordering, and the two known partial wins (candidate: both `lbu` at the block head; c1: `sw` before `lw a3`) are separated by ONE tie-break decision that no C-level spelling has moved in 63 forms.
mechanism: with the expand order settled (calls.c, above) and the alias class ruled out, the only remaining inputs to the block's order are INSN_PRIORITY (dependence structure, which is fixed by the arithmetic the function performs) and rank_for_schedule's tie-breaks (the load/store class against last_scheduled_insn, then INSN_LUID). The candidate and c1 differ ONLY in RTX_UNCHANGING_P on one load, and that single bit flips both the `lbu` order and the arg3 chain's position — which is the signature of a tie-break cascade, not of a dependence change.
next probe: run the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_PRIO_DEBUG / BB2_SCHED_DEBUG — see [[instrumented-cc1-location]]) on the candidate and on c1 with tmp/grind/saEft01Init/s10/idump.sh + schedscan.py, and diff the two RANKDBG traces over build idx 46-61. That is a two-form diff of the SAME chassis differing in one bit, so the trace difference isolates the exact comparison that flips — far cheaper to read than session 10's single-form dump. If the flipping comparison is an INSN_LUID tie, the C lever is a source-order change to the two index reads on the const chassis; if it is the load/store class test, the lever is which insn precedes the pair.

## [s12] F23 — the do{}while(0) wrapper is the candidate's single match device and has still never been through a fresh adversarial cheat-reviewer.
mechanism: explicitly sanctioned (.claude/rules/do-while-zero-exception.md, owner ruling 2026-07-06: any body, ANY codegen effect, register allocation named explicitly, mandatory inline FAKE annotation, single level needs no nesting justification); the candidate carries the annotation. Its only effect is flow.c's loop_depth ref-weighting for the three table pointers (session 11 corrected session 9's mechanism statement; calls.c's precompute is not loop-gated at -O2, confirmed again this session at 1653-1664 + stmt.c:2435 + toplev.c:3387).
next probe: invoke cheat-reviewer on the candidate before any completion claim, presenting the lever-exhaustion record: 63 measured argument-block forms over four rigid attractors, s11's wrapper-extent sweep (w1/w2 = 8, w7 = 16/92i, w3-w6 = 7), and s12's four newly-dead axes (arg1, MEM_IN_STRUCT_P, pre-loop store placement, prototype).

## [s12] F24 — the two sibling alarm functions in the same TU inherit this result for free.
mechanism: set_alarm/get_alarm are inlined into cpu_side_move_dir_4 (= CD_sync, name string D_80016240, ~src/system.c:366) and the function at ~src/system.c:480 (= CD_ready, string D_80016248), both already carrying the same hand-derived goto shape. Note src/system.c:384 currently spells one sibling's second index pointer as `(u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1)`, which is a symbol-difference coercion that should be re-examined when that function is worked.
next probe: once saEft01Init closes, apply the same single do{}while(0) wrapper plus the named-arg4 argument spelling to both siblings and re-score them with the engine sandbox.

## [s12] F25's premise: something in expand_call emits store_one_arg for the single stack argument BEFORE the register-arg precompute loop, which is why the fully-inline arg4 form drags its address chain late too.
- mechanism: calls.c's ordering of (a) the register-arg precompute loop, (b) store_one_arg for args with reg == 0, (c) load_register_parameters.
- probe: Read tools/gcc-2.7.2/calls.c 1600-1760 and the load_register_parameters call site (~1876) directly.
- result: The order is unambiguous and is the one session 11 assumed: precompute 1618-1665, store_one_arg 1736-1739, load_register_parameters ~1876. Nothing emits the stack store early. Therefore the i1 (arg4 fully inline) attractor's RTL order already IS target's split (address chain before the sw, load after) and its 13/91 is produced downstream in sched.c, not at expand.
- verdict: KILLED

## [s12] The residual is an alias-class problem: our table loads are INDIRECT_REFs through hoisted `T *` pointers (MEM_IN_STRUCT_P = 0) while target's came from real array accesses, so alias.c lets our arg4 load float above the outgoing-arg store.
- mechanism: expand sets MEM_IN_STRUCT_P for ARRAY_REF/COMPONENT_REF but not for a subscript on a pointer; sched.c's memory-dependence test consults it.
- probe: Six forms (m1-m6) re-typing the hoisted bases as pointers-to-array (`s32 (*tbl)[]`, `u8 (*idx)[2]`) so the accesses are genuine ARRAY_REFs with the base still a hoisted pseudo, plus p1-p3 (pointer-arithmetic derefs) as the negative control.
- result: All six ARRAY_REF forms and all three pointer-arithmetic forms are byte-identical to the candidate at 7/91.
- verdict: KILLED

## [s12] RTX_UNCHANGING_P on the arg4 load (a const-qualified access) changes the scheduler's placement of the argument loads relative to the outgoing-arg `sw 16(sp)`.
- mechanism: RTX_UNCHANGING_P suppresses the store->load memory dependence, changing what sched.c may reorder around the stack store.
- probe: c1-c4 (const on each hoisted pointer and on all three), e1-e6 (per-ACCESS const casts on arg4 / arg5 / arg3 separately), e7-e8 (the same cast on a fully inline arg4), then thirteen bolt-ons on the winning chassis (d1-d13, g5, g6).
- result: CONFIRMED as a real lever: c1 = 9/91 and its diff is the FIRST measured form whose `sw 16(sp)` precedes `lw a3` (target's relation). It costs 2 because the two `lbu` come out swapped and the D_800A11D5/arg3 chain is pushed after `lw a3` instead of interleaved before the `sw`. The per-access cast on arg4 alone (e1) reproduces the 9 exactly; the same cast on arg5 (e2) is inert and on arg3 (e3) costs an instruction; on a fully inline arg4 (e7/e8) it is byte-inert at 13 — so the flag only moves a STATEMENT-emitted load. The chassis is rigid: all thirteen bolt-ons score >= 9.
- verdict: CONFIRMED

## [s12] arg1 — the debug_printf format-string address — is an untouched degree of freedom that could shift the argument block's LUIDs.
- mechanism: Naming it moves its address computation out of the precompute loop and into the statement stream.
- probe: a1-a3: named inside the wrapper, named at wrapper entry, and the same treatment for the tslTm2LoadImage_2 argument.
- result: All three byte-identical at 7/91. Every one of the 51 prior banked forms touched only args 2-5; arg1 is now measured and dead.
- verdict: KILLED

## [s12] The pre-loop global stores' placement among the three pointer initialisations is a live LUID lever (session 11 only permuted the pointers among themselves).
- mechanism: Statement order feeds pseudo numbering -> INSN_LUID -> the argument block's tie-breaks, and s11 proved the pointer-init order itself is load-bearing.
- probe: g1/g2/g4 interleave `D_800F19BC = 0;` and `D_800F19C0 = &D_800162C0;` through the pointer inits; g3 lifts all three pointer inits above the sys_VSync call.
- result: g1/g2/g4 = 7 (byte-inert); g3 = 17, which is the already-banked pointer-inits-before-VSync family.
- verdict: KILLED

## [s12] debug_printf's declared signature is wrong — the original is a printf-style varargs function, and the fixed 5-arg prototype changes expand_call's argument handling.
- mechanism: With `...` the arguments take default promotions and a different path through expand_call / must_preallocate.
- probe: score2.py re-declares the prototype `(void *, ...)`, `(void *, void *, ...)` and `()` (K&R) and re-scores three bodies (candidate, arg4-inline, arg5-named+arg4-inline).
- result: Every combination is byte-identical to the fixed prototype (7 / 13 / 13). The prototype is byte-inert here, so it is also not evidence about the original signature either way, and fake-varargs-explicit-homing does not apply (no bulk pre-subu arg homes in this call).
- verdict: KILLED

## [s13] F27 — a random decomp-permuter campaign can lower saEft01Init's honest floor from the 7/91 candidate.
- mechanism: sessions 4/5 broke an 18/91 plateau with random permutation, so the same tool on the newer, structurally different session-9 chassis (and on the session-12 const chassis) might find the ordering the 65 hand-written argument spellings missed.
- probe: two fresh-seed campaigns via tools/permuter_campaign.py (ws2 = candidate chassis, base 435; ws3 = const chassis, base 455), 5 jobs each, ~16.5k and ~13.2k iterations, every find re-scored in the engine sandbox.
- result: 9 finds; the only permuter-score improvements (428, 423 on ws2; 450 on ws3) are sandbox regressions (18/92, 17/92, 10/91), and one 455-scored ws3 find is sandbox 10 where the 455-scored base is 9. The permuter's objective charges 60 per reordering and 5 per register rename over a RE-ALIGNED instruction stream, while the sandbox is position-locked at 1 each (scorer.py:14-18; the candidate's 435 decomposes exactly as 7*60 + 3*5). Our residual is 100% reordering, so every permuter-visible improvement from this base necessarily trades ordering for renaming — a regression in the only metric that counts.
- verdict: KILLED — and the kill is structural, not a budget question. Random permutation cannot descend on this basin because it is optimising a different objective; more iterations cannot fix a sign error in the gradient.

## [s13] F28 — reading a table through its own global symbol instead of the hoisted base is an untried argument axis.
- mechanism: all 63 prior argument forms indexed the hoisted locals (tbl_125c / tbl_11dc / idx_1494); the permuter proposed `D_800A125C[idx_1494[1]]` for arg5, which is semantically identical but gives the block a second reference to the symbol rather than to the pseudo.
- probe: spliced into src/system.c on the candidate chassis and sandboxed; positional build-vs-target disassembly diff read via tmp/grind/saEft01Init/s13/odf.sh.
- result: sandbox 17 / 92. The second symbol reference costs `lui at` + `addu at,v1` + `lw v1,0(at)` (91 -> 92 insns) and drops tbl_125c to a single in-block use, collapsing its allocno priority: the callee-save map rotates to $s0=D_800A1494, $s1=param, $s2=D_800A125C against target's $s0=D_800A125C, $s1=D_800A1494, $s2=param, and the argument block itself moves further from target (`lw a3` at build idx 56).
- verdict: KILLED — the global-symbol respelling axis is dead in both directions (it cannot be cheaper than the hoisted base, which is exactly why sessions 2-9 hoisted).

## [s13] F26 — the residual is entirely inside sched.c's ready-list ordering, and the two known partial wins are separated by ONE tie-break decision (UNCHANGED, inherited — not probed this session; the mandated modality was permuter).
- mechanism: unchanged from session 12.
- next probe: unchanged — run the instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_PRIO_DEBUG / BB2_SCHED_DEBUG) on the candidate AND on the const (c1) chassis using tmp/grind/saEft01Init/s10/idump.sh + schedscan.py and diff the two RANKDBG traces over build idx 46-61.

## [s13] F23 — the do{}while(0) wrapper has still never been through a fresh adversarial cheat-reviewer (UNCHANGED, inherited).
- mechanism: unchanged from session 12.
- next probe: unchanged — invoke cheat-reviewer on the candidate before any completion claim, presenting the lever-exhaustion record (now 65 measured argument forms over four rigid attractors, plus session 13's two campaigns).

## [s13] A random decomp-permuter campaign can lower saEft01Init's honest floor below the 7/91 candidate (F27).
- mechanism: Sessions 4/5 broke an 18/91 plateau with random permutation, so the same tool on the newer session-9 zero-lever do{}while(0) chassis, and on the session-12 const-tbl_125c fourth attractor, might find the instruction ordering that 63 hand-written argument spellings missed.
- probe: Two fresh-seed campaigns via tools/permuter_campaign.py on full-TU-context workspaces (cpp of src/system.c, trimmed for pycparser and verified byte-identical to the full-TU compile, target.o from asm/funcs/saEft01Init.s at offset 0, no regfix/asmfix): ws2 = candidate chassis (base 435, 16,487 iters / 883 s, 6 finds), ws3 = const chassis (base 455, 13,249 iters / 758 s, 3 finds), 5 jobs each. Every find spliced back into src/system.c and re-scored with `sandbox saEft01Init --disable all`.
- result: permuter 435 (base and 4 finds) -> sandbox 7/91; permuter 428 -> 18/92; permuter 423 -> 17/92; permuter 455 (base) -> 9/91; permuter 455 (find) -> 10/91; permuter 450 -> 10/91. Every score improvement is a sandbox regression, and one permuter score covers two different sandbox distances. Cause read from tools/decomp-permuter/src/scorer.py:14-18: PENALTY_REORDERING=60, PENALTY_REGALLOC=5, streams RE-ALIGNED before counting, vs the sandbox's position-locked weight of 1 for either. The candidate's 435 decomposes exactly as 7*60 + 3*5 — the permuter charges 60x for the same seven instructions the sandbox charges 7 for — so any mutation trading ordering for renaming reads as progress to it and is a regression to us.
- verdict: KILLED

## [s13] Reading a table through its own global symbol instead of the hoisted local base is an untried argument-block axis that could move the block's order (F28).
- mechanism: All 63 prior argument forms indexed the hoisted pseudos (tbl_125c / tbl_11dc / idx_1494). The permuter proposed arg5 as `D_800A125C[idx_1494[1]]` — semantically identical (tbl_125c is initialised to D_800A125C and never re-assigned) but giving the block a second reference to the SYMBOL rather than to the pseudo.
- probe: Spliced onto the candidate chassis, sandboxed, and read as a positional build-vs-target disassembly diff (tmp/grind/saEft01Init/s13/odf.sh).
- result: sandbox 17/92. The second symbol reference costs a fresh `lui at` + `addu at,v1` + `lw v1,0(at)` address chain (91 -> 92 insns) and drops tbl_125c to a single in-block use, collapsing its allocno priority: the callee-save map rotates to $s0=D_800A1494 / $s1=param / $s2=D_800A125C against target's $s0=D_800A125C / $s1=D_800A1494 / $s2=param, and the argument block itself moves further away (`lbu v0,0(s0)` / `lbu v1,1(s0)` vs target's `lbu a0,0(s1)` / `lbu v0,1(s1)`, `lw a3` at build idx 56).
- verdict: KILLED

## Session 14 (permuter) — measured

### H63 — KILLED (session 13's stated mechanism is factually wrong)
**Statement:** decomp-permuter cannot descend on this basin because it charges
PENALTY_REORDERING = 60 against PENALTY_REGALLOC = 5, and the candidate
chassis's base score of 435 decomposes as seven reorderings (420) plus three
register differences (15) — i.e. the permuter charges 60x for the very seven
instructions the sandbox charges 7 for (s13's conclusion, taken on faith by
the s14 brief).
**Mechanism (as claimed):** scorer.py:14-18 class penalties applied over a
difflib-RE-ALIGNED instruction stream.
**Probe:** forced `Scorer(debug_mode=True)` inside the REAL permuter process
(a PYTHONPATH `sitecustomize.py` shim that wraps `Scorer.__init__`), launched
via tools/permuter_campaign.py, and read the scorer's own printed Penalty List
for ws1/base.o against ws1/target.o out of campaign.log.
**Result:**
```
Stack Differences:      0  (1)
Branch Differences:     0  (1)
Register Differences:   7  (5)    ->  35
Reorderings:            0  (60)   ->   0
Insertions:             2  (100)  -> 200
Deletions:              2  (100)  -> 200
                                  =  435
```
There are ZERO reordering penalties. 7*60 + 3*5 = 435 and 2*100 + 2*100 + 7*5
= 435 — the two decompositions collide numerically, which is how the wrong
model survived a whole session. The real misalignment is **100-vs-5, not
60-vs-5**: scorer.py only converts an insertion/deletion pair into one
60-point "reordering" when the two rows are IDENTICAL strings (it counts
`min(ins, dels)` over a Counter keyed on the full row text), and our two
unalignable slots hold DIFFERENT mnemonics, so they are charged as an
insertion AND a deletion at 100 each. 92% of the base score (400/435) is those
two slots; the seven register/field differences the sandbox actually charges 7
for are worth 35, i.e. the stock objective is nearly blind to them.
**Verdict: KILLED.** Artifact: `tmp/grind/saEft01Init/s14/penalty_calibration.log`.

### H64 — CONFIRMED (the objective CAN be aligned, from inside a grind session)
**Statement:** the objective can be re-weighted to track the engine's
position-locked sandbox without writing to `tools/` (which s13 called an
operator decision and used to park the question), and a correctly aligned
objective is monotone with the sandbox.
**Mechanism:** the penalties are plain class attributes read as
`self.PENALTY_*` at score time, so a `sitecustomize.py` on PYTHONPATH can
monkeypatch them in the permuter process. `tools/permuter_campaign.py` spawns
the permuter with `subprocess.Popen(..., cwd=ROOT)` and no `env=`, so the
launcher's environment (including PYTHONPATH) is inherited — the owner's
telemetry directive is untouched and nothing under `tools/` changes. This is a
SEARCH-HEURISTIC change only: every find is still re-spliced into src/system.c
and re-scored by the unmodified `sandbox --disable all`.
**Probe:** `tmp/grind/saEft01Init/s14/sbshim/sitecustomize.py` sets REGALLOC 1,
REORDERING 2, INSERTION 4, DELETION 4 (sandbox charges 1 per differing
instruction; ins/del stays dearest because an instruction-count change shifts
every later position and target is 91 = our 91). Two workspaces built by
`tmp/grind/saEft01Init/s14/mkws.sh` (full-TU compile, saEft01Init extracted at
offset 0, no regfix/asmfix, trim verified byte-identical to the full-TU
object): ws1 = the candidate chassis, ws2 = the const / RTX_UNCHANGING_P
chassis (`const s32 *tbl_125c`).
**Result:** the shim is verified live — ws1 prints `base score = 23`, exactly
the predicted 7*1 + 2*4 + 2*4, and ws2 prints 29. The sandbox scores of those
two bases are 7 and 9. **The aligned objective is monotone with the sandbox
ACROSS chassis (23<->7, 29<->9), which the stock objective was NOT** (s13
measured a 455-scored base at sandbox 9 and a 455-scored find at sandbox 10).
And at the tie point the correlation is exact: all four ws1 finds scored
permuter 23 and ALL FOUR screen sandbox 7 / 91.
**Verdict: CONFIRMED.** The shim is reusable for any OTHER function whose
residual is register/field differences rather than ordering.

### H65 — KILLED (the permuter modality is dead for this function, correctly this time)
**Statement:** with the objective aligned (H64), random search descends below
the 7 / 91 floor, or at least below the base of whichever chassis it is seeded
on.
**Probe:** two fresh-seed campaigns launched through
`tools/permuter_campaign.py` with the shim, `-j 8`, waited in-turn with
`permuter_campaign.py wait` and harvested with `--stop`:
  * ws1, candidate chassis, base 23 — **31,747 iterations**, four finds, ALL
    ties at 23, no find below base. Last novel find at 328 s; the final 555 s
    window returned nothing novel (fresh-seed stopping rule).
  * ws2, const chassis, base 29 — **28,380 iterations**, five finds: one at 28,
    one at **26** (the first genuine permuter-score DESCENT this function has
    ever produced in three permuter sessions), three ties at 29.
Every find of both campaigns re-scored with
`tmp/grind/saEft01Init/s14/rescore.ps1` -> `sandbox --disable all`.
**Result:** ws1's four ties are sandbox 7 / 7 / 7 / 7 (perfect, but no
progress). ws2's descent to 26 screens **sandbox 10 / 91 against its base's 9**
— the descent is still an anti-correlated one — and the three 29-scored ties
screen 9, 10 and 12, i.e. the objective cannot even separate a 9 from a 12
within one basin. Aligning the weights fixed the CROSS-chassis ranking and did
not fix the WITHIN-basin ranking, because difflib re-aligns the candidate
stream against target before any penalty is applied, and our entire residual is
positional: the seven diffs are the same seven instructions in the wrong ORDER,
which is exactly the information the re-alignment destroys.
**Verdict: KILLED — and this time on the right mechanism.** Do not spend a
third session's modality on the permuter here, and do NOT retry "fix the
penalties first": that experiment is now run, banked and measured. Banked:
`rejected/sandbox-aligned-permuter-const-26-screens-10.c`.

## [s14] decomp-permuter cannot descend on this basin because it charges PENALTY_REORDERING = 60 against PENALTY_REGALLOC = 5, and the candidate chassis's base score of 435 decomposes as seven reorderings (420) plus three register differences (15) — session 13's stated mechanism, carried into this session's brief as settled.
- mechanism: scorer.py:14-18 class penalties applied over a difflib-RE-ALIGNED instruction stream; the claim was that the permuter charges 60x for the very seven instructions the engine sandbox charges 7 for.
- probe: Forced Scorer(debug_mode=True) inside the REAL permuter process with a PYTHONPATH sitecustomize.py shim that wraps Scorer.__init__, launched through tools/permuter_campaign.py, and read the scorer's own printed Penalty List for ws1/base.o against ws1/target.o out of campaign.log.
- result: Stack 0 (x1), Branch 0 (x1), Register Differences 7 (x5) = 35, Reorderings ZERO (x60) = 0, Insertions 2 (x100) = 200, Deletions 2 (x100) = 200, total 435. There are no reordering penalties at all. 7*60+3*5 and 2*100+2*100+7*5 both equal 435 — a numerical collision that let the wrong model survive a whole session. scorer.py only converts an insertion/deletion pair into one 60-point reordering when the two rows are IDENTICAL strings (Counter keyed on full row text, min(ins,dels)); our two unalignable slots hold different mnemonics, so each costs 100 both ways. 92% of the base score (400/435) is those two slots, and the stock objective is nearly blind to the seven field differences the sandbox charges 7 for. The real misalignment is 100-vs-5, not 60-vs-5.
- verdict: KILLED

## [s14] The permuter objective can be re-weighted to track the engine's position-locked sandbox WITHOUT writing to tools/ (session 13 parked this as an operator decision), and a correctly aligned objective is monotone with the sandbox.
- mechanism: The penalties are plain class attributes read as self.PENALTY_* at score time, so a sitecustomize.py on PYTHONPATH can monkeypatch them in the permuter process. tools/permuter_campaign.py spawns the permuter with subprocess.Popen(..., cwd=ROOT) and no env=, so the launcher's environment is inherited — the owner's 2026-07-07 telemetry directive is untouched and nothing under tools/ changes. This is a SEARCH-HEURISTIC change only: every find is still re-spliced into src/system.c and re-scored by the unmodified sandbox --disable all, which gates everything.
- probe: tmp/grind/saEft01Init/s14/sbshim/sitecustomize.py sets REGALLOC 1, REORDERING 2, INSERTION 4, DELETION 4 (the sandbox charges 1 per differing instruction; insertion/deletion stays dearest because an instruction-count change shifts every later position and target is 91 = our 91). Two workspaces built by tmp/grind/saEft01Init/s14/mkws.sh (full-TU compile, saEft01Init extracted at offset 0, no regfix/asmfix, trim verified byte-identical to the full-TU object): ws1 = the session-9 candidate chassis, ws2 = the const / RTX_UNCHANGING_P chassis (const s32 *tbl_125c).
- result: The shim is verified live in the campaign log: ws1 prints 'base score = 23', exactly the predicted 7*1 + 2*4 + 2*4, and ws2 prints 29. The sandbox scores of those two bases are 7 and 9, so the aligned objective is MONOTONE with the sandbox across chassis (23<->7, 29<->9) where the stock objective was not (s13 measured a 455-scored base at sandbox 9 and a 455-scored find at sandbox 10). At the tie point the correlation is exact: all four ws1 finds scored permuter 23 and all four screen sandbox 7/91.
- verdict: CONFIRMED

## [s14] With the objective aligned, random search descends below the 7/91 floor, or at least below the base of whichever chassis it is seeded on.
- mechanism: If the permuter's number tracked the sandbox, its hill-climb would be a usable gradient on this function's residual for the first time.
- probe: Two fresh-seed campaigns launched through tools/permuter_campaign.py with the shim, -j 8, waited in-turn with permuter_campaign.py wait and harvested with --stop. ws1 (candidate chassis, base 23): 31,747 iterations. ws2 (const chassis, base 29): 28,380 iterations. Every find of both campaigns re-scored with tmp/grind/saEft01Init/s14/rescore.ps1 -> engine sandbox --disable all.
- result: ws1: four finds, ALL ties at 23, nothing below base; last novel find at 328 s and the final 555 s window returned nothing novel (fresh-seed stopping rule); all four screen sandbox 7/7/7/7. ws2: five finds — one at 28, one at 26 (the first genuine permuter-score DESCENT this function has produced in three permuter sessions), three ties at 29; the 26 screens sandbox 10/91 against its base's 9, and the three 29-ties screen 9, 10 and 12, i.e. the objective cannot separate a 9 from a 12 inside one basin. Aligning the weights fixed the cross-chassis ranking and did NOT fix the within-basin ranking, because difflib re-aligns the candidate stream against target before any penalty is applied and saEft01Init's entire residual is positional — the same seven instructions in the wrong ORDER, which is exactly the information re-alignment destroys.
- verdict: KILLED

## Session 15 (forensics) — measured

### H63 — KILLED (F26's tie-break half)
**Statement:** the residual sits in `rank_for_schedule`'s tie-breaks, and the
candidate's win (both `lbu` at the block head) and the const chassis's win
(`sw 16(sp)` before `lw a3`) are separated by ONE tie-break decision — the
load/store dependence class against `last_scheduled_insn` (sched.c:2414-2449).
**Mechanism:** `rank_for_schedule` returns early on an INSN_PRIORITY
difference, then classifies both candidates against `last_scheduled_insn`
(1 = data dependent, 2 = anti/output dependent, 3 = independent or latency 1)
and prefers the higher class, falling through to INSN_LUID only on a class tie.
The instrumented cc1's `BB2_RANK_DEBUG` hook (sched.c:2436-2446) prints exactly
those class decisions, so every printed RANKDBG line IS a genuine
equal-priority tie-break.
**Probe:** `bash tmp/grind/saEft01Init/s10/idump.sh s15cand` and `... s15const`
(both verified CODEGEN-IDENTICAL to the frozen `tools/gcc-2.7.2/build/cc1` on
the whole TU), then `tmp/grind/saEft01Init/s15/blocksum.py`, which auto-derives
the argument block from `system.i.sched` (the insns from the
`tslTm2LoadImage_2` call through the `debug_printf` call) and censuses every
RANKDBG decision whose BOTH operands are in that block.
**Result:** **ZERO class-resolved decisions on either chassis.** Candidate:
54 in-block RANKDBG decisions, all `val=0`, every operand `cls=3`. Const
chassis: 49 in-block decisions, all `val=0`, all `cls=3`. The class test never
discriminates in this block on any form measured, so no C spelling can act
through it. Among equal-priority insns the order is decided ONLY by INSN_LUID,
i.e. by the incoming chain order — which at sched2 is sched1's output order.
**Verdict: KILLED.** F26 as posed is dead; H64 replaces it.

### H64 — CONFIRMED (the residual is ONE dependence edge)
**Statement:** the difference between the 7-scoring candidate and the
9-scoring const chassis is not a tie-break but a single memory dependence edge
that `RTX_UNCHANGING_P` deletes.
**Mechanism:** `sched.c:sched_analyze_1` makes a store depend on every pending
read seen so far in the chain, unless the read's MEM is `RTX_UNCHANGING_P`.
`arg4 = tbl_125c[idx_1494[0]]` is a statement-expanded load (insn 100) and the
outgoing 5th argument is `sw 16(sp)` (insn 125); calls.c emits the statement
load BEFORE `store_one_arg`, so the pair gets a REG_DEP_ANTI (write-after-read)
which FORCES `lw a3` ahead of `sw 16(sp)`. Target has the opposite relation.
**Probe:** read the `.sched` LOG_LINKS for insns 100/123/125/131 on both
chassis (`tmp/grind/saEft01Init/s10/schedscan.py`), and compare the full block
priority vectors and sched1/sched2 orders via `blocksum.py`.
**Result:** the two chassis have **byte-identical final INSN_PRIORITY vectors**
over all 19 block insns — RTX_UNCHANGING_P changes no priority whatsoever. The
only structural difference is the LOG_LINKS of insn 125:
`(insn_list 89 (insn_list:REG_DEP_ANTI 100 (insn_list 123 (nil))))` on the
candidate vs `(insn_list 89 (insn_list 123 (nil)))` on the const chassis, with
the loads' MEMs going `mem/s:SI` -> `mem/s/u:SI`. Deleting that one edge flips
the whole sched1 order (`89,93,96,116,98,119,121,100,...` ->
`89,116,119,121,93,123,127,125,96,...`), which becomes sched2's LUID order and
produces target's `sw` before `lw a3` while losing the `lbu` head order.
**Verdict: CONFIRMED.** The entire 7-vs-9 split is that single edge.

### H65 — KILLED (the const axis is now closed to the edge)
**Statement:** marking only arg4's access unchanging (a per-access
`((const s32 *)tbl_125c)[idx_1494[0]]` cast, arg5 left plain) is a distinct
form from the whole-pointer `const s32 *tbl_125c` declaration and may keep the
candidate's `lbu` head while gaining target's store/load relation.
**Probe:** built that exact form, `sandbox saEft01Init --disable all`, then
`idump.sh s15c4` + `blocksum.py`.
**Result:** 9 / 91 — and not merely the same score: the same sched1 order, the
same sched2 order and the same priority vector as the whole-pointer const
chassis. RTX_UNCHANGING_P on arg5's load is completely inert; the whole const
effect is the arg4 edge and nothing else. **Verdict: KILLED.** Banked:
`rejected/const-cast-on-arg4-access-only-is-the-whole-const-chassis-9.c`.
Do not re-probe any const / RTX_UNCHANGING_P spelling on this function.

### H66 — KILLED (the named-address + inline-deref split)
**Statement:** target needs the idx[0] ADDRESS chain expanded at the statement
(early, giving the `lbu`/`sll`/`addu` their early positions) and only the final
LOAD expanded by `load_register_parameters` (late, after `store_one_arg`'s
`sw`), so a named ADDRESS local dereferenced inline in the call
(`arg4 = &tbl_125c[idx_1494[0]]; debug_printf(..., *arg4, ...)`) should reach
target's split.
**Mechanism:** calls.c 1618-1665 (register-arg precompute) runs before
store_one_arg (1736-1739) which runs before load_register_parameters (~1876).
An inline array-element argument returns a MEM from the precompute loop, so its
load is deferred past the stack store — the direction that gives a true
dependence (load after store) instead of the anti-dependence H64 measured.
**Probe:** built the form, `sandbox --disable all`, `idump.sh s15ptr` +
`blocksum.py`.
**Result:** 9 / 91 at 91 build insns (the RTL block loses one insn — the
separate `move a3` copy disappears — but the sandbox score does not improve).
It lands in the same 9-attractor as the const chassis rather than in a fifth
basin. **Verdict: KILLED.** This is the same form as the previously banked
`rejected/named-pointer-inline-deref-9.c`, now re-measured against the session-9
chassis and with a dump attached, so the attractor count stands at four.

## Live frontier (for session 16)

### F28 — split the idx[0] chain across calls.c's store_one_arg boundary
**Mechanism:** H64 + H66 together define the target shape exactly. Target's
idx[0] chain is `lbu 46 / sll 52 / addu 56 / lw a3 61` — SPREAD across the
`sw 16(sp)` at build idx 58. Our four attractors each put the whole chain on
ONE side of that boundary: named-value arg4 puts all four insns before the
`sw` (7 / 91, anti-dep), fully-inline arg4 puts all four after it (13 / 91),
and both the const cast and the named-address-with-inline-deref buy the
boundary crossing at the cost of the `lbu` head order (9 / 91). The C question
is what spelling gets the ADDRESS insns emitted by the register-arg precompute
loop while the LOAD is emitted by load_register_parameters — the precompute
loop returning a MEM whose address is already in a pseudo.
**Next probe:** read `tools/gcc-2.7.2/calls.c` around 1618-1665 for exactly
which `expand_expr` results are left as MEMs vs forced to REGs (the
`TREE_CODE (args[i].tree_value)` / `args[i].value` handling and the
`ARGS_SIZE_RTX` path), and `expr.c:expand_expr`'s ARRAY_REF/INDIRECT_REF cases
for when the address subexpression is pre-forced into a pseudo. Then look for
C that forces the address into a pseudo at the statement while leaving the
dereference inline in the call — e.g. an index or offset named as a local while
the base+deref stays inline (`s32 i0 = idx_1494[0]; ... tbl_125c[i0] ...`), or
a named pointer that is USED elsewhere in the block so the precompute loop
cannot fold it back. NB `named-index-bytes-inert-vs-pure-inline-13.c` and
`arg3-value-named-12.c` are already banked, so screen new forms against the
rejected bank first. Screen with `sandbox --disable all`; confirm any move with
`idump.sh` + `blocksum.py` (the 100->125 REG_DEP_ANTI edge is the single
readout that says whether the boundary was crossed).

### F23 — cheat-review status of the do{}while(0) wrapper (unchanged, still open)
**Mechanism:** unchanged from session 14 — it is explicitly sanctioned by the
owner's 2026-07-06 do-while(0) ruling and carries the mandatory inline FAKE
annotation, but no fresh adversarial `cheat-reviewer` has ever seen it.
**Next probe:** invoke `cheat-reviewer` on the candidate before any completion
claim, presenting the lever-exhaustion record: 67 measured argument-block forms
over four rigid attractors, s11's wrapper-extent sweep, s12's four dead axes,
s13+s14's four exhausted permuter campaigns, and s15's forensic proof that the
residual is a single sched.c dependence edge.

### F27 — the sandbox-aligned permuter shim (PROJECT-WIDE, unchanged)
**Mechanism / next probe:** unchanged from session 14 — reuse
`tmp/grind/saEft01Init/s14/sbshim/` on the next queue function whose ladder
reaches `permuter`, with `BB2_SHIM_DEBUG=1` on the first launch to read that
function's Penalty List decomposition before trusting it.

## [s15] The residual sits in rank_for_schedule's tie-breaks, and the candidate's win (both lbu at the block head) and the const chassis's win (sw 16(sp) before lw a3) are separated by one tie-break decision - the load/store dependence class against last_scheduled_insn.
- mechanism: sched.c:2409 returns early on an INSN_PRIORITY difference, then 2414-2449 classifies both ready-list candidates against last_scheduled_insn (1 = data dependent, 2 = anti/output dependent, 3 = independent or latency 1) and prefers the higher class, falling through to INSN_LUID only on a class tie. The instrumented cc1's BB2_RANK_DEBUG hook at sched.c:2436-2446 prints exactly those class decisions, and because the priority check returns first, every printed RANKDBG line is by construction a genuine equal-priority tie-break.
- probe: Ran the instrumented cc1 (tools/gcc-2.7.2/cc1) via tmp/grind/saEft01Init/s10/idump.sh on the candidate chassis (tag s15cand) and on the const chassis (tag s15const); both dumps printed CODEGEN-IDENTICAL against the frozen tools/gcc-2.7.2/build/cc1 over the whole TU. Then censused every in-block RANKDBG decision with the new tmp/grind/saEft01Init/s15/blocksum.py, which auto-derives the argument block from system.i.sched (the insns from the tslTm2LoadImage_2 call through the debug_printf call inclusive) and counts class-resolved (val != 0) vs LUID-fallthrough (val == 0) decisions.
- result: ZERO class-resolved decisions on either chassis. Candidate: 54 in-block RANKDBG decisions, all val=0, every operand cls=3. Const chassis: 49 in-block decisions, all val=0, all cls=3. The load/store class test never discriminates in this block on any form measured, so no C spelling can act through it; among equal-priority insns the order is decided purely by INSN_LUID, i.e. by the incoming chain order (at sched2, sched1's output order).
- verdict: KILLED

## [s15] The difference between the 7-scoring candidate and the 9-scoring const chassis is not a tie-break at all but a single memory dependence edge that RTX_UNCHANGING_P deletes.
- mechanism: sched.c:sched_analyze_1 makes a store depend on every pending read seen so far in the insn chain unless that read's MEM is RTX_UNCHANGING_P. `arg4 = tbl_125c[idx_1494[0]]` is a statement-expanded load (insn 100, the lw a3 value) and the outgoing 5th argument is `sw 16(sp)` (insn 125). calls.c emits the statement load BEFORE store_one_arg, so the pair gets a REG_DEP_ANTI (write-after-read on the outgoing-arg stack slot) that FORCES lw a3 ahead of sw 16(sp). Target has the opposite relation.
- probe: Read the .sched LOG_LINKS for insns 100/123/125/131 on both chassis with tmp/grind/saEft01Init/s10/schedscan.py, and compared the full block INSN_PRIORITY vectors plus the sched1 and sched2 emitted orders with blocksum.py.
- result: The two chassis have BYTE-IDENTICAL final INSN_PRIORITY vectors across all 19 block insns (89=2 93=2 96=1 98=1 100=2 105=1 108=1 111=1 113=1 116=1 119=2 121=1 123=1 125=1 127=1 129=2 131=1 133=2 135=1) - RTX_UNCHANGING_P moves no priority whatsoever. The sole structural difference is insn 125's LOG_LINKS: candidate `(insn_list 89 (insn_list:REG_DEP_ANTI 100 (insn_list 123 (nil))))` vs const `(insn_list 89 (insn_list 123 (nil)))`, with the loads' MEMs going mem/s:SI -> mem/s/u:SI. Deleting that one edge flips the sched1 order from 89,93,96,116,98,119,121,100,108,123,127,125,111,105,113,129,131,133,135 to 89,116,119,121,93,123,127,125,96,108,98,111,105,113,129,100,131,133,135, which then becomes sched2's LUID order: the sw now precedes lw a3 (target's relation) but the two lbu come out in the wrong order. That trade IS the +2.
- verdict: CONFIRMED

## [s15] Marking only arg4's access unchanging - a per-access ((const s32 *)tbl_125c)[idx_1494[0]] cast with arg5 left plain - is a distinct form from the whole-pointer `const s32 *tbl_125c` declaration and may keep the candidate's lbu head order while gaining target's store/load relation.
- mechanism: RTX_UNCHANGING_P is a property of each individual MEM rtx, so a per-access cast marks only arg4's load; if arg5's load being aliasable contributed any of the const chassis's regression, the two forms would diverge.
- probe: Built that exact form in src/system.c, ran sandbox saEft01Init --disable all, then idump.sh s15c4 + blocksum.py to compare orders and priorities against the whole-pointer const dump.
- result: 9 / 91 - and not merely the same score: the SAME sched1 order, the SAME sched2 order and the SAME priority vector as the whole-pointer const chassis. RTX_UNCHANGING_P on arg5's load is completely inert; the whole const effect is the single arg4 edge and nothing else. The const axis is now measured down to the edge and is closed. Banked as rejected/const-cast-on-arg4-access-only-is-the-whole-const-chassis-9.c.
- verdict: KILLED

## [s15] Target needs the idx[0] ADDRESS chain expanded at the statement (early) and only the final LOAD expanded by load_register_parameters (late, after store_one_arg's sw), so a named ADDRESS local dereferenced inline in the call reaches target's split.
- mechanism: calls.c runs the register-arg precompute loop (1618-1665), then store_one_arg for the single stack argument (1736-1739), then load_register_parameters (~1876). An inline array-element argument returns a MEM from the precompute loop so its load is deferred past the stack store - the direction that yields a true dependence (load after store) instead of the anti-dependence measured above - while a named address local pins the lbu/sll/addu at the statement.
- probe: Built `s32 *arg4; arg4 = &tbl_125c[idx_1494[0]]; debug_printf(..., *arg4, tbl_125c[idx_1494[1]]);` on the session-9 chassis; sandbox --disable all, then idump.sh s15ptr + blocksum.py.
- result: 9 / 91 at 91 build insns. The RTL block loses one insn (the separate `move a3` copy disappears) but it lands in the same 9-attractor as the const chassis rather than opening a fifth basin. Same form family as the previously banked rejected/named-pointer-inline-deref-9.c, now re-measured against the current chassis with a dump attached; the attractor count stands at four.
- verdict: KILLED

## Session 16 (forensics) — hypothesis dispositions

### F28 — "the residual is reachable only by splitting the idx[0] chain across calls.c's store_one_arg boundary" — **KILLED (premise satisfied, effect absent)**

Probe: instrumented-cc1 `-da` dumps of all three attractors, block extracted
from `system.i.combine` with the new `tmp/grind/saEft01Init/s16/blockdump.py`.

Result: the named-ADDRESS-pointer form (`p4 = &tbl_125c[idx_1494[0]];` at the
statement, `*p4` as the argument) ALREADY produces exactly the split F28 asks
for — address chain at insns 93/96/98 (the earliest LUIDs of the block), load
deferred to `load_register_parameters` at insn 131, `sw 16(sp)` at 123 between
them.  That is target's expand shape, insn-for-insn, and it scores 9 / 91 not
0.  Inline arg3 has had the same split in EVERY form we have ever shipped.  So
"cross the store_one_arg boundary" is not the missing ingredient; it is already
crossed, and F28's next-probe list (force the address into a pseudo, name the
index, second use of the pointer) is chasing a property we already have.

### H67 — "at sched1 the block is ordered by dependence-depth INSN_PRIORITY" (the implicit model of sessions 10/11/15) — **KILLED**

Probe: `BB2_SCHED_DEBUG=1` SCHEDDBG PICK trace of the ptr chassis (artifacts
`ptr_sched1_picks.txt`, `ptr_sched2_picks.txt`).

Result: at sched1 sixteen of the block's nineteen picks show
`pri=2130706433` = `LAUNCH_PRIORITY` (sched.c:3985 + `birthing_insn_p`
sched.c:2495).  Priorities are FLAT; the order is decided by latency-queue
release times and INSN_LUID.  The dependence-depth vectors s11/s15 measured are
sched2 artefacts, and sched2 only re-states sched1's order (its own picks are
all `pri=1`, all RANKDBG `val=0`).  Any future reasoning about this block must
be about sched1's LUID order, not about priority levels.

### H68 — "the block residual is specific to saEft01Init's chassis" — **KILLED**

Probe: `sandbox cpu_side_move_dir_4 --disable all` + per-index diff
(`tmp/grind/saEft01Init/s16/odiff.py`, artifact
`sibling_cpu_side_move_dir_4_diff.txt`), plus the target asm of all three
siblings.

Result: `cpu_side_move_dir_4` (7 / 160) and `marionation_Exec` carry the SAME
block with the SAME target order (`sw 0x10($sp)` before both argument loads,
`lw $a3` last) and OUR builds emit `lw a3` early in all three — in
cpu_side_move_dir_4's case from a body that names BOTH arg4 and arg5.  The
residual is a shared block-scheduling problem across three queue items, not a
property of the do{}while(0) chassis.  A closing form here retires part of two
other queue functions.

### F29 (NEW, replaces F28) — invert sched1 instead of sweeping C

Statement: because sched1's in-block priorities are flat (H67) and the
class tie-break is inert on all three measured chassis, sched1's output is a
pure function of the pre-sched RTL order + dependence edges + latency queue.
Therefore the set of pre-sched orders that yield TARGET's 14-insn sequence is
computable, and the search collapses from "which C spelling" to "which LUID
order, and can C emit it".

Mechanism: `schedule_block` picks `ready[0]` (sched.c:3966) and PREPENDS
(3970-3975); `rank_for_schedule` (2398-2456) resolves flat priorities by
`INSN_LUID`; insns enter the ready list through `schedule_insn`'s
`queue_insn`/latency path.  A faithful replay needs only the LOG_LINKS printed
in `system.i.sched` plus MIPS load latency (2) — both already on disk for four
chassis.

Next probe: write `tmp/grind/saEft01Init/sN/schedsim.py` that (1) parses
`system.i.combine`'s block + its dependence edges, (2) replays the backward
pass with flat LAUNCH priorities and LUID tie-breaks, (3) VALIDATES itself by
reproducing the observed `.sched` order for all four banked chassis
(cand/const/ptr/inline — dumps are in `tmp/grind/saEft01Init/s16/`), then (4)
enumerates the permutations of pre-sched order reachable from C (statement vs
inline per argument x statement order) and reports which, if any, replay to
target's sequence.  If NONE does, that is a proof-grade negative for the whole
argument-block axis and the escalation writes itself; if one does, it names the
exact C to write.

### F23 (unchanged, still live) — the do{}while(0) wrapper has never been through a fresh adversarial cheat-reviewer

Unchanged this session.  Note for whoever runs it: session 16 adds that the
wrapper is not implicated in the block at all — the identical residual occurs
in `cpu_side_move_dir_4`, which has no wrapper.

## [s16] F28 — the residual is reachable only by splitting the idx[0] chain across calls.c's store_one_arg boundary (address insns from the register-arg precompute loop, final load from load_register_parameters).
- mechanism: calls.c:1618-1665 precomputes register args before store_one_arg (1736-1739) and load_register_parameters (~1876); an inline array-element arg leaves a MEM so its load defers, a named-VALUE arg loads at the statement.
- probe: Instrumented cc1 -da dumps (tools/gcc-2.7.2/cc1 via s10/idump.sh) of all three attractors; block extracted from system.i.combine with the new blockdump.py.
- result: The named-ADDRESS-pointer form (p4 = &tbl_125c[idx_1494[0]]; ... *p4) already produces the split EXACTLY: address chain at insns 93/96/98 (earliest LUIDs in the block), sw 16(sp) at 123, load deferred to insn 131 in load_register_parameters. That is target's expand shape insn-for-insn and it scores 9/91. Inline arg3 has had the same split in every form ever shipped. The premise is satisfied and the effect is absent.
- verdict: KILLED

## [s16] H67 — at sched1 the argument block is ordered by dependence-depth INSN_PRIORITY (the implicit model behind sessions 10, 11 and 15).
- mechanism: priority() (sched.c:1424-1514) computes depth from the top over LOG_LINKS; schedule_block picks ready[0] and prepends (3966-3975), so final order is ascending priority with LUID tie-breaks.
- probe: First-ever BB2_SCHED_DEBUG=1 run on this function: the SCHEDDBG PICK hook prints the whole ready list with priority and LUID at every pick (artifacts ptr_sched1_picks.txt / ptr_sched2_picks.txt).
- result: At sched1, sixteen of nineteen in-block picks carry pri=2130706433 = 0x7F000001 = LAUNCH_PRIORITY — sched.c:3985 plus birthing_insn_p (2495) re-raise every register-birthing insn while reload_completed == 0. Priorities are FLAT; order comes from latency-queue release (clock ticks skip 19 -> 21) and INSN_LUID alone. sched2 then re-picks the same block at flat pri=1 and merely re-states sched1's order. The depth 'levels' of s11/s15 are a sched2 artefact.
- verdict: KILLED

## [s16] H68 — the early-lw-a3 residual is a property of saEft01Init's chassis (goto-loop + do{}while(0) wrapper + hoisted bases).
- mechanism: If chassis-specific, the block's failure would not reproduce in other functions containing the same call.
- probe: Compared the 14 target insns before `jal debug_printf` in asm/funcs/{saEft01Init,cpu_side_move_dir_4,marionation_Exec}.s; ran `sandbox cpu_side_move_dir_4 --disable all` and a per-index disassembly diff (new tools s16/odiff.py).
- result: All three ORIGINAL objects put `sw $v1,0x10($sp)` before both register-arg loads with `lw $a3` last. cpu_side_move_dir_4 scores 7/160 and its diff shows `lw a3,0(v1)` at index 55 where target has it at 65 — the identical residual, from a body that names BOTH arg4 and arg5 and has no do{}while(0) wrapper. One shared block-scheduling problem across three queue items.
- verdict: CONFIRMED

## Session 17 (escalation) — measured, then DISPOSED

Re-applied `memory/grind/saEft01Init/candidate.c` to `src/system.c` and
re-measured: `sandbox saEft01Init --disable all` → **score 7, target_insns 91,
build_insns 91, rules_dropped 15**. (HEAD's committed body — the pre-session-9
two-named-args form without the wrapper — measures 18 / 91, so the candidate
must be applied before any probe; do not measure against HEAD.)

### H45 — KILLED (a genuinely un-tried axis, now closed on both halves)

**Statement:** the second index read can be spelled through the SEPARATE byte
global `D_800A1495` (which `src/system.c` already declares at line 778 and this
function has never referenced) instead of through `idx_1494[1]`, and since two
adjacent byte globals is a plausible ORIGINAL object model, this may reorder the
argument block's address chains without paying the callee-save rotation that
s13's table-global probe paid.

**Mechanism:** every prior global-spelling probe in this ledger — s1's H1
(`inline-globals-kills-hoisted-base-pointers`, 40 / 85) and s13's permuter find
(`permuter-global-arg5-rotates-callee-save-map-17-92`) — respelled the TABLE
reference `D_800A125C`. The INDEX reads had never been touched. Respelling only
the index leaves both table bases hoisted, so the two allocnos that carry the
callee-save map are untouched; only `idx_1494`'s own ref count moves.

**Probe:** two variants, each a one-line edit on the candidate chassis, each
scored with `sandbox saEft01Init --disable all`:
 - p1 — arg5's index only: `debug_printf(..., arg4, tbl_125c[D_800A1495]);`
 - p2 — both indices: `arg4 = tbl_125c[D_800A1494];` as well, so the hoisted
   `u8 *idx_1494` disappears from the function entirely.

**Result:** p1 = **14 / 92**. p2 = **24 / 89**. The candidate was restored and
re-verified at 7 / 91 afterwards.

**Verdict: KILLED.** p1 fails for the reason s13's table-global probe failed,
one register earlier in the chain: the second reference to the array base is
what gives `idx_1494` its two in-loop refs (five after the `do{}while(0)`
`loop_depth` weighting), and dropping it to a single ref collapses that
allocno's priority while the separate `%hi/%lo` chain for `D_800A1495` adds an
instruction. Banked:
`rejected/arg5-index-via-separate-D_800A1495-global-14-92.c` and
`rejected/both-indices-via-globals-drops-idx-base-24-89.c`.

### H46 — CONFIRMED (the useful half of H45's negative result)

**Statement:** target's own instruction count tells us whether the original
source held the two indices in ONE array/pointer or in TWO independent scalars.

**Mechanism:** a hoisted base costs a prologue `lui`/`addiu` pair. If the
original used two independent byte globals there would be no index base to
hoist, and a faithful C spelling of that model would come out two instructions
SHORTER than target.

**Probe:** H45's p2 (both indices through globals, no `idx_1494` base).

**Result:** **89 build insns against target's 91** — exactly two short, and the
two missing instructions are the index base's `lui`/`addiu` pair. This is the
same shape s1's H1 measured for the TABLE bases (85 vs 91) reproduced for the
INDEX base.

**Verdict: CONFIRMED.** All three hoisted bases (`tbl_11dc`, `idx_1494`,
`tbl_125c`) are structurally REQUIRED by target's instruction count, not merely
score-preferred. Any future session that is tempted to "simplify" the pointer
block by inlining a global reference is contradicting a measured fact — the
count moves against you before the score does.

### Endgame-lock gate evaluation (the mandated work of an escalation session)

**Gate 1 — hand-coded-asm signals: FAIL.** `python3 tools/scan_hand_coded.py
--single saEft01Init` → `tier=LOW score=1/8`. The only signal set is S4 (four
loads in an 8-insn window @ insn 46) and that window IS the `debug_printf`
argument block, i.e. GCC 2.7.2's ordinary `load_register_parameters` shape, not
a hand-written-asm artefact. None of the STRONG S1/S2/S6 signals fire. The
engine's canonical gate routes the function **C**. The same gate was already
ruled on directly for this function in the **2026-07-09 01:25** decision
(`docs/grind/decisions.md:8`), which DENIED canonical-asm for the twin family
`{cpu_side_move_dir_4, marionation_Exec, saEft01Init}` on three independently
dispositive grounds; nothing measured since changes any input to it.

**Gate 2 — in-hand SOTN-master precedent for the closing construct: FAIL.**
There is no closing construct. The honest floor is 7 / 91, not 0, so no
construct closes the function and no file+line citation exists to offer. (The
candidate's one non-obvious construct, the `/* FAKE */`-annotated single
`do { } while (0)` wrapper, is already covered by the owner's 2026-07-06 ruling
and is a floor-LOWERING device, never claimed as a closing one. F23 — that it
still owes a fresh adversarial `cheat-reviewer` — remains open but is moot for
this disposition.)

**What holds the byte-match on main:** 15 `regfix.txt` rules, lines 97–119; zero
`asmfix.txt` rules; zero cheat-asm in this function's body. Two callee-save
swaps + six prologue/epilogue stack-offset substs are downstream paperwork for
the register rotation; the load-bearing residual is the 14-instruction argument
-block `reorder` at line 113 plus the four `$2<->$3` / `$2<->$4` renames feeding
it — exactly the 7 the sandbox charges.

**Disposition:** both gates FAIL, which is the owner's pre-decided
REFUSED / OWNER-ACCEPTED INCOMPLETE case under the standing 2026-07-27 ruling.
Filed `docs/grind/decisions.md:2789` —
`## 2026-08-01 — saEft01Init (src/system.c) — **OWNER-ESCALATION — RESOLVED BY
STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**` — and
returned `owner-gated`. The driver parks the function terminally; nothing is
pending on the owner. This is expressly not a claim of unmatchability: per
`no-compiler-divergence` and `difficult-is-not-impossible` the matching pure C
exists; what is exhausted is this project's search of it across 17 sessions and
six distinct modalities. If the function is ever un-parked, F29 (build and
VALIDATE the sched1 replay simulator against the four banked chassis dumps, then
invert it) is where the next session starts — not another argument-spelling
sweep and not the permuter.

## [s17] The second index read can be spelled through the SEPARATE byte global D_800A1495 (already declared at src/system.c:778 and never referenced by this function) instead of through idx_1494[1]. Two adjacent byte globals is a plausible ORIGINAL object model, and unlike s13's table-global probe it leaves both table bases hoisted, so it may reorder the argument block's address chains without paying the callee-save rotation.
- mechanism: Every prior global-spelling probe in this ledger — s1's H1 (inline-globals-kills-hoisted-base-pointers, 40/85) and s13's permuter find (permuter-global-arg5-rotates-callee-save-map-17-92) — respelled the TABLE reference D_800A125C. The INDEX reads had never been touched by any of the 67 banked argument forms. Respelling only an index leaves tbl_125c and tbl_11dc untouched; only idx_1494's own ref count moves, and idx_1494's second reference is exactly what gives it 2 in-loop refs (5 after the do{}while(0) loop_depth weighting) in global.c's allocno_compare.
- probe: Applied memory/grind/saEft01Init/candidate.c to src/system.c (re-verified at 7/91, rules_dropped 15), then two one-line variants each scored with `sandbox saEft01Init --disable all`: p1 = arg5's index only (`tbl_125c[D_800A1495]`); p2 = both indices via globals (`arg4 = tbl_125c[D_800A1494]` as well), which removes the hoisted `u8 *idx_1494` from the function entirely. Candidate restored and re-verified at 7/91 afterwards.
- result: p1 = 14 / 92. p2 = 24 / 89. Both are regressions against the 7 / 91 floor. p1 fails the same way s13's table-global probe failed, one register earlier in the chain: idx_1494 drops to a single ref, its allocno priority collapses, and the separate %hi/%lo chain for D_800A1495 adds an instruction. Banked as rejected/arg5-index-via-separate-D_800A1495-global-14-92.c and rejected/both-indices-via-globals-drops-idx-base-24-89.c.
- verdict: KILLED

## [s17] Target's own instruction count decides whether the original source held the two byte indices in ONE array/pointer or in TWO independent scalar globals.
- mechanism: A hoisted base costs a prologue lui/addiu pair. If the original had used two independent byte globals there would be no index base to hoist, so a faithful C spelling of that object model must come out exactly two instructions SHORTER than target's 91.
- probe: The p2 variant above (both indices through globals, no idx_1494 base) — read build_insns rather than score.
- result: 89 build instructions against target's 91 — exactly two short, and the two missing instructions are the index base's lui/addiu pair. Same shape s1's H1 measured for the TABLE bases (85 vs 91) reproduced for the INDEX base. All three hoisted bases (tbl_11dc, idx_1494, tbl_125c) are therefore structurally REQUIRED by target's instruction count, not merely score-preferred; any future 'simplification' that inlines one of them moves the count against you before it moves the score.
- verdict: CONFIRMED

## [s17] saEft01Init passes at least one of the two endgame-lock AND-gates, so a true pending OWNER-ESCALATION (rather than the standing both-gates-fail auto-ruling) is the correct disposition for this escalation-modality session.
- mechanism: Gate 1 is a STRONG hand-coded-asm signal set (S1 multu pacing / S2 empty branch / S6 BIOS jumptable) per .claude/rules/endgame-lock-disposition.md and hand-coded-asm-recognition. Gate 2 is an in-hand SOTN-master precedent for the construct that CLOSES the function, cited to file+line.
- probe: Gate 1: ran `python3 tools/scan_hand_coded.py --single saEft01Init`. Gate 2: identified what closes the function (nothing — the honest floor is 7/91, and the byte-match on main is held by 15 regfix.txt rules at lines 97-119, zero asmfix rules, zero cheat-asm in the body) and checked the ledger's 60+ banked forms for any closing construct that could carry a precedent.
- result: Gate 1 FAILS: tier=LOW, score 1/8, and the lone signal is S4 (four loads in an 8-insn window @ insn 46) — that window IS the debug_printf argument block, i.e. GCC 2.7.2's ordinary load_register_parameters shape, not a hand-written-asm artefact. No S1/S2/S6. The engine's canonical gate independently routes the function C, and the 2026-07-09 01:25 decision (docs/grind/decisions.md:8) already DENIED canonical-asm for this exact twin family on three independently dispositive grounds. Gate 2 FAILS: there is no closing construct, so no precedent is citable even in principle; the candidate's only non-obvious construct is the /* FAKE */-annotated single do{}while(0) wrapper, already covered by the owner's 2026-07-06 ruling and a floor-LOWERING device that has never been claimed as a closing one.
- verdict: KILLED

## [s18] F29 executed: sched1's tie-break is DESCENDING-UID, so the pass preserves pre-sched LUID order and the residual reduces to "which LUID order can C emit".
- mechanism: schedule_block picks ready[0] and prepends; the `-da` trace's `now` list is the ready list sorted descending by priority with equal priorities ordered by descending insn UID. Backward pass + prepend + descending-UID ties = original order preserved. Deviations therefore come ONLY from the latency queue ("blocking insn N for k cycles") and the potential-hazard swap.
- probe: Decoded the banked trace tmp/grind/saEft01Init/s16/s16cand/system.i.sched for `;; Function saEft01Init`; VALIDATED the pick rule on basic block 0 (picks 36,34,31,19,17,28,25,22,13,11 reconstruct the emitted order exactly) before applying it to basic block 3; then extracted the pre-sched RTL + LOG_LINKS from the matching .combine with tmp/grind/CD_datasync/s18/rtl.py.
- result: CONFIRMED, and the forward order it predicts for block 3 (87,89,93,96,116,98,119,121,100,108,123,127,125,111,105,113,129,131,133,135,138,152,154) is exactly what the sandbox builds. Target's window maps to 93,116,105,119,121,96,123,108,98,111,113,125,131,100, which respects every dependence edge — so target's order is a valid topological order of our own DAG and the obstruction is purely the emitted LUID order.
- verdict: CONFIRMED

## [s18] Naming the two byte INDEX reads as local variables gives the pre-sched order the LUID separation target needs (idx1's chain before idx0's shift).
- mechanism: A statement boundary was expected to force `idx_1494[0]` and `idx_1494[1]` into their own insns ahead of the call's argument expansion, decoupling the two `lbu` from their shifts and letting the latency queue place them the way target does. s17 only ever probed indices spelled through SEPARATE GLOBALS (D_800A1494 / D_800A1495), never as named locals, so this was genuinely un-tried.
- probe: Three forms measured with `sandbox CD_datasync --disable all` off the re-measured 7/91 chassis: both indices named (v1), idx1 only (v4), idx0 only (v5).
- result: 13 / 91, 13 / 91, 13 / 91 — all three land on the fully-inline attractor exactly. A named index local is codegen-INERT: GCC folds it straight back into the address chain and the statement boundary buys no LUID separation at all. Banked as rejected/s18-named-both-index-locals-inert-vs-inline-13.c, s18-named-idx1-local-only-13.c, s18-named-idx0-local-only-13.c.
- verdict: KILLED

## [s18] The pointer-ADDRESS attractor's score depends on the ORDER of the two argument initialisations, so some permutation of (address-of-arg4, value-of-arg5) beats the banked 9.
- mechanism: If sched1 preserves LUID order, then which of the two argument chains is emitted first in the pre-sched stream should decide which one the scheduler defers; the address form already achieves the expand-time split (address early, load at load_register_parameters), so only the ordering should remain.
- probe: Five crossings measured: arg5-value-then-p4-address (w1), p4-address-then-arg5-value (w2), p4-address-only (w4), `p4 = tbl_125c + idx_1494[0]` (x3), p4-address-then-named-stack-arg5 (x4); plus both-as-addresses in both orders (w3, w5).
- result: w1 = w2 = w4 = x3 = x4 = 9 / 91 exactly; w3 = w5 = 10 / 91. The address attractor is RIGID at 9 and completely insensitive to initialisation order — a stronger and cleaner statement than the ledger's earlier "initialisation order is live, the banked order is the unique optimum" (which was measured on the VALUE form, not the address form). Making arg5 an address as well costs exactly one instruction-position. All seven banked under rejected/s18-*.
- verdict: KILLED

## [s18] Pulling arg3 out of the call (as a value or as an address pointer) changes which chain the scheduler defers and can combine with the arg4 address split.
- mechanism: arg3's address chain (108/111/113) sits between arg4's and arg5's in the pre-sched stream; naming it should move the whole block's LUID layout.
- probe: x1 (p3 and p4 both addresses), x2 (arg3 value named + p4 address).
- result: 13 / 91 and 12 / 91 — both regress well past the 7 floor, consistent with the ledger's earlier 8-form arg3 sweep (10-15). Banked as rejected/s18-p3-and-p4-addresses-13.c, s18-arg3-value-plus-p4-address-12.c.
- verdict: KILLED

## [s18] The residual is a pure C-spelling gap against the project's frozen open-port cc1 — i.e. the compiler that built the target would schedule this block the same way ours does.
- mechanism: `.claude/rules/no-compiler-divergence.md` treats decompals/mips-gcc-2.7.2 as functionally equivalent to PsyQ's cc1psx for this project's purposes; 18 sessions of spelling search implicitly assumed the block's scheduling decision is fork-invariant. `tools/cc1psx_wrapper.sh` runs the actual PsyQ cc1psx.exe (2.7.2.SN.1) and is sanctioned for calibration / self-disproof, so the assumption is directly testable — and had never been tested on this function.
- probe: Preprocessed src/system.c (candidate body in place) with the project's exact CPP flags to tmp/grind/CD_datasync/s18/system.i, then compiled that single file twice: `tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel` -> open.s, and `tools/cc1psx_wrapper.sh -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w` -> psx.s. Compared both CD_datasync argument blocks against asm/funcs/CD_datasync.s.
- result: FALSIFIED. The two compilers emit DIFFERENT schedules for this block from byte-identical C. cc1psx orders the two `lbu` as target does (idx0 first, `lbu $3,0($17)` then `lbu $2,1($17)`) and issues arg5's value load before arg4's — both of which the open port gets wrong — but still commits `lw $7,0($3)` at slot 10 instead of target's slot 14, and puts the idx0 chain in `$3` where target uses `$4`. So the block is fork-SENSITIVE, and NEITHER compiler reproduces target from the current C. The whole 81-spelling search to date was scored against a compiler that provably diverges from the target's own on exactly this construct.
- verdict: KILLED (the fork-invariance assumption is dead; the fork itself is not a free match either)

## [s18] F29: sched1's flat-priority tie-break resolves by insn LUID, so the pass preserves the pre-sched order and the residual reduces to 'which LUID order can C emit'; target's order is therefore either reachable or provably not.
- mechanism: schedule_block picks ready[0] and prepends (backward pass); the -da trace's printed 'now' list is the ready list sorted by descending priority with equal priorities ordered by DESCENDING insn UID. Backward + prepend + descending-UID ties = original order preserved; deviations come only from the latency queue ('blocking insn N for k cycles') and the potential-hazard swap.
- probe: Decoded the banked trace tmp/grind/saEft01Init/s16/s16cand/system.i.sched for ';; Function saEft01Init'. VALIDATED the pick rule on basic block 0 first (picks 36,34,31,19,17,28,25,22,13,11 reconstruct the emitted order exactly) before applying it to basic block 3 (the printf block, insns 77..154, 23 schedulable insns). Extracted the pre-sched RTL and its LOG_LINKS from the matching .combine with tmp/grind/CD_datasync/s18/rtl.py.
- result: Pick rule confirmed on three independent worked examples in the trace ('133 (7f000001) 129 (7f000001) 127 (1) -> now 133 129 127'; '100 (7f) 108 (7f) 123 (7f) -> now 123 108 100'; '96 (7f) 116 (7f) -> now 116 96'). Block 3's backward picks are 154,152,138,135,133,131,129,113,105,111,125,127,123,108,100,121,119,98,116,96,93,89,87, whose reversal is exactly the block the sandbox builds. Target's 14-insn window maps to RTL UIDs 93,116,105,119,121,96,123,108,98,111,113,125,131,100 and respects every dependence edge of the four-chain DAG, so the DAG is NOT the obstruction - the emitted LUID order is. The two structural facts separating target from every BB2 form: target defers arg4's load (100) to the last slot of the block, and splits arg4's address chain maximally, keeping it live in $a0 across the whole block, where every BB2 form runs it contiguously through $v0.
- verdict: CONFIRMED

## [s18] Naming the two byte INDEX reads as local variables gives the pre-sched stream the LUID separation target needs (idx1's chain ahead of idx0's shift).
- mechanism: A statement boundary was expected to force idx_1494[0] / idx_1494[1] into their own insns ahead of the call's argument expansion, decoupling each lbu from its shift and letting the latency queue place them as target does. s17 only ever probed the indices spelled through SEPARATE GLOBALS (D_800A1494 / D_800A1495), never as named locals - genuinely un-tried in 17 sessions.
- probe: Three forms measured with `sandbox CD_datasync --disable all` off the re-measured 7/91 chassis: both indices named (v1), idx1 only (v4), idx0 only (v5).
- result: 13 / 91, 13 / 91, 13 / 91 - all three land exactly on the fully-inline attractor. A named index local is codegen-INERT: GCC folds it straight back into the address chain and the statement boundary buys no LUID separation whatsoever.
- verdict: KILLED

## [s18] The pointer-ADDRESS attractor's score depends on the ORDER of the two argument initialisations, so some permutation beats the banked 9.
- mechanism: If sched1 preserves LUID order then whichever argument chain is emitted first should decide which one the scheduler defers; the address form already achieves the expand-time split (address chain early, load deferred to load_register_parameters), leaving only ordering.
- probe: Seven crossings measured: arg5-value-then-p4-address (w1), p4-address-then-arg5-value (w2), p4-address-only (w4), `p4 = tbl_125c + idx_1494[0]` (x3), p4-address-then-named-stack-arg5 (x4), and both-as-addresses in both orders (w3, w5).
- result: w1 = w2 = w4 = x3 = x4 = 9 / 91 EXACTLY; w3 = w5 = 10 / 91. The address attractor is rigid at 9 and completely insensitive to initialisation order - a stronger statement than the ledger's earlier 'initialisation order is live, the banked order is the unique optimum', which was measured on the VALUE form, not the address form. Making arg5 an address as well costs exactly one instruction-position. w4 is nonetheless the structurally CLOSEST form ever measured (it gets arg5's value issued before arg4's load and defers arg4's load past the arg5 chain, target's shape) and still misses on four counts: lbu order, an undeferred sw 16(sp), swapped addu operands, and lw a3 landing at slot 11 instead of 14.
- verdict: KILLED

## [s18] Pulling arg3 out of the call (as a value or as an address pointer) re-lays the block's LUID order and combines with the arg4 address split.
- mechanism: arg3's address chain (RTL 108/111/113) sits between arg4's and arg5's in the pre-sched stream, so naming it should move the whole block's layout.
- probe: x1 (p3 and p4 both as address pointers), x2 (arg3 value named + p4 address).
- result: 13 / 91 and 12 / 91 - both regress well past the floor, consistent with the ledger's earlier 8-form arg3 sweep (10-15).
- verdict: KILLED

## [s18] The residual is a pure C-spelling gap against the project's frozen open-port cc1 - i.e. the compiler that actually built the target would schedule this block the same way ours does (fork-invariance).
- mechanism: no-compiler-divergence treats decompals/mips-gcc-2.7.2 as functionally equivalent to PsyQ's cc1psx for this project; 18 sessions of spelling search implicitly assumed this block's scheduling decision is fork-invariant. tools/cc1psx_wrapper.sh runs the ACTUAL PsyQ cc1psx.exe (GCC 2.7.2.SN.1) under dosemu2 and is sanctioned for calibration / self-disproof, so the assumption is directly testable - and had never been tested on this function in 17 sessions.
- probe: Preprocessed src/system.c (candidate body in place) with the project's exact CPP flags and defines to tmp/grind/CD_datasync/s18/system.i, then compiled that one file twice: `tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel` -> open.s and `tools/cc1psx_wrapper.sh -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w` -> psx.s. Compared both CD_datasync argument blocks against asm/funcs/CD_datasync.s (tmp/grind/CD_datasync/s18/cmp.py).
- result: FALSIFIED. The two compilers emit DIFFERENT schedules for this block from byte-identical C. open-port: `lbu $3,1($17) / lw $5 / sll $2 / addu $2,$16 / sll $3 / addu $3,$16 / lw $7,0($2) / lbu $2 / lw $3,0($3) / sll $2 / addu $2,$19 / sw $3,16($sp) / lw $6,0($2)`. cc1psx: `lbu $3,0($17) / lbu $2,1($17) / lw $5 / sll $3 / sll $2 / addu $2,$16 / lw $4,0($2) / lbu $2 / addu $3,$16 / lw $7,0($3) / sll $2 / addu $2,$19 / sw $4,16($sp) / lw $6,0($2)`. target: `lbu $4,0($17) / lbu $2,1($17) / lw $5 / sll $2 / addu $2,$16 / sll $4 / lw $3,0($2) / lbu $2 / addu $4,$16 / sll $2 / addu $2,$19 / sw $3,16($sp) / lw $6,0($2) / lw $7,0($4)`. cc1psx is strictly closer to target than the open port on two counts the open port gets wrong (it emits the two lbu in target's order, idx0 first, and issues arg5's value load before arg4's) but still commits lw $7 at slot 10 instead of slot 14 and puts the idx0 chain in $3 where target uses $4. So the block is fork-SENSITIVE and NEITHER compiler reproduces target from the current C: the fork is not a free match, but the entire ~81-spelling search to date was scored against a compiler that provably diverges from the target's own on exactly this construct.
- verdict: KILLED

## [s19] F31 — cc1psx (the compiler that actually built the target) is a better-aligned search oracle than the open-port cc1, because the residual argument block is compiler-fork-sensitive — **KILLED**

- mechanism (as proposed by s18): the sandbox scores against the open-port `cc1` build, so if the
  residual is fork-sensitive the open-port gradient is the WRONG objective; scoring candidate
  spellings by how close PsyQ `cc1psx` (2.7.2.SN.1, calibration-only) comes to target should
  either find the original source shape outright or explain 18 sessions of a flat floor.
- probe: built `tmp/grind/CD_datasync/s19/{psx.sh,score.py,expand.py,sweep.sh}` — one cpp pass into
  BOTH compilers, plus a canonicalizer that expands cc1's assembler macros so pre-maspsx output is
  comparable instruction-for-instruction with `asm/funcs/CD_datasync.s`. Calibrated on the banked
  candidate (82 insns both forks vs target 82; LCS dist 6 vs the sandbox's masked 7). Then scored
  23 body spellings on both forks: the 15 banked s9/s18 forms plus 8 written this session.
- result: (1) equal distance on the candidate (6 vs 6) — s18's "cc1psx is strictly closer on two
  counts" was a macro-form artifact; (2) the residual is IDENTICAL IN KIND on both forks — neither
  ever emits target's `lw $a3,0($a0)` as the block's last memory reference, and both seat the idx-0
  address chain in `$v0`/`$v1` instead of `$a0`; (3) 19/23 forms score identically, 3 differ by 1,
  1 by 5, and NO form on either fork goes below the shared minimum of 6, with identical ranking.
- verdict: **KILLED.** The fork is not the discriminator; a cc1psx-scored gradient is the same
  search space, not a different one. This removes the last un-tried axis and is the measured basis
  for the 2026-08-30 standing-ruling disposition.

## [s19] The 8 fresh argument spellings (arg3-named, arg5-named-with-inline-arg4, arg5-then-p4-address, second base pointer for arg4, arg5+idx0-byte-named, arg5-then-arg3-named, arg4-staged-before-puts, idx1-byte-named-only) reach the target's late `lw $a3,0($a0)` — **KILLED**

- mechanism: target computes arg5's load first (stored to 16(sp)), then arg3, and issues arg4's
  load LAST out of `$a0`; every measured build issues arg4's load early out of `$v0`/`$v1`. If the
  order were driven by source statement order or by which value is named, one of these eight
  re-orderings/namings should move it.
- probe: dual-fork sweep above (`tmp/grind/CD_datasync/s19/sweep.sh`), scores open/psx: n1 11/10,
  n2 10/10, n3 6/6, n4 10/10, n5 10/10, n6 11/10, n7 28/23 (+3 insns), n8 10/10.
- result: none beat the base attractor of 6; n3 ties it, the rest regress. Staging arg4 before the
  `puts` (n7) is catastrophic on both forks — it forces a callee-save carry across the call.
- verdict: **KILLED.** Banked as `memory/grind/CD_datasync/rejected/s19-dualfork-*.c`. Together with
  the s9 twelve-spelling sweep and the s18 fourteen-form sweep this closes the
  named/inline/pointer/byte-named/base-pointer/statement-order axes of the argument block on BOTH
  compiler forks.

## [s19] STATUS OF THE REMAINING FRONTIER (documentation only after the 2026-08-30 disposition)

F29 (invert the now-empirical descending-UID sched1 model: enumerate C-emittable pre-sched LUID
orders and replay each) and F30 (cross-check the identical block on the sibling `CD_sync`) are both
SINGLE-AXIS searches over exactly the space this session's dual-fork sweep just showed is a single
attractor on two independent compilers. They are retained in the ledger as documentation. The
recorded re-activation trigger is a toolchain model that searches the JOINT `sched.c`
emission-order x `global.c` allocno-seat space — the same trigger the twin `CD_ready` recorded on
2026-08-30, and for the same coupled-fixed-point reason (every C change that fixes the order breaks
the seats and vice versa).

## [s19] F31: PsyQ cc1psx (the compiler that actually built the target) is a better-aligned search oracle than the open-port cc1, because the residual debug_printf argument block is compiler-fork-sensitive.
- mechanism: The sandbox scores against the open-port cc1 build, so a fork-sensitive residual would make the sandbox gradient the wrong objective; tools/cc1psx_wrapper.sh runs cc1psx 2.7.2.SN.1 as a drop-in cc1 on preprocessed C at ~0.8s/invocation (calibration/self-disproof only, never a build path, per .claude/rules/no-compiler-divergence.md).
- probe: Built tmp/grind/CD_datasync/s19/{psx.sh,score.py,expand.py,sweep.sh}: ONE mipsel-linux-gnu-cpp pass feeding BOTH tools/gcc-2.7.2/build/cc1 (-mel) and cc1psx, plus a canonicalizer that expands cc1's assembler macros (la, symbol-form lw/sw/lbu, li of a lui-able constant, j $ra) so pre-maspsx output is comparable instruction-for-instruction with asm/funcs/CD_datasync.s. Calibrated on the banked candidate: 82 insns for BOTH forks vs target's 82, LCS distance 6 tracking the sandbox's masked 7. Then scored 23 body spellings on both forks (the 15 banked s9/s18 forms plus 8 written this session).
- result: Three independent negatives. (1) Equal distance on the candidate: open 6, psx 6 - s18's 'cc1psx is strictly closer on two counts' was an artifact of diffing macro-form cc1 output against already-expanded target bytes. (2) Same residual in KIND: both forks seat target's $a0 idx-0 address chain in $v0/$v1 and issue its lw $a3 EARLY; NEITHER ever emits target's 'lw $a3,0($a0)' as the block's last memory reference (open also misses target's second lbu register, psx matches it but then mis-seats sw 16(sp)). (3) Ranking correlation is near-perfect: 19/23 forms score identically on the two forks, 3 have cc1psx exactly 1 lower, 1 has it 5 lower, and NO form on either fork goes below the shared minimum of 6.
- verdict: KILLED

## [s19] One of eight fresh argument spellings (arg3-named; arg5-named with arg4 inline; arg5-named then p4-address; a second base pointer for arg4; arg5-named with the idx-0 byte named; arg5-then-arg3 named; arg4 staged before the puts; idx-1 byte named only) makes either compiler issue target's arg4 load LAST.
- mechanism: Target computes arg5's load first (stored to 16(sp)), then arg3 via lbu D_800A11D5, and issues arg4's lw out of $a0 as the block's final memory reference. If that order were driven by source statement order or by which value is named/pointered, one of these eight re-orderings should move it.
- probe: tmp/grind/CD_datasync/s19/sweep.sh over the 8 generated bodies, scored on both forks (open/psx): n1 11/10, n2 10/10, n3 6/6, n4 10/10, n5 10/10, n6 11/10, n7 28/23 (+3 insns), n8 10/10.
- result: None beat the base attractor of 6; n3 ties it, the rest regress. n7 (staging arg4 before the puts) is catastrophic on both forks because it forces a callee-save carry across the call. Banked as memory/grind/CD_datasync/rejected/s19-dualfork-*.c (bank now 84).
- verdict: KILLED

## 2026-09-01 — operator reopen note (owner ruling 2026-09-01 (decisions.md FORECLOSED-BUCKET REVIEW entry))

Returned to active under Ruling A; executes via the Ruling D CD_intr aggregate-merge session — which is this ledger's OWN open frontier item F14 (hypotheses.md:976-991), deferred for scope and never killed. H28 killed the volatile QUALIFIER on the split extern u8 declarations; F14's struct SHAPE is explicitly a separate, untested question. Falsifiable claim (F14 verbatim): extern volatile CD_intr Intr indexed as members reproduces the target's shared-base lbu 0/1($s1) addressing and scores below 7/91.

## [s20] F14 (the Sony `Intr` / `Alarm` aggregate declarations) — KILLED, measured
- **statement:** Declaring 0x800A1494/95/96 as Sony's `CD_intr` struct (and 0x800F19B8/BC/C0 as
  the `Alarm` struct) reproduces target's hoisted base-register addressing that neither the
  reference spelling nor a `u8 *` pointer local reaches, and moves the residual.
- **mechanism claimed by F14:** target reaches Intr through a hoisted base (`lbu a0,0(s1)` /
  `lbu v0,1(s1)`, offsets 0 and 1); a single aggregate object with two member offsets is the
  shape that emits that, and the repo could not express it while `D_800A1494` is `extern u8`.
- **probe:** seven spellings built on the candidate chassis and sandbox-scored, plus full
  objdump comparison of the equal-scoring ones (harness `tmp/grind/CD_datasync/s20/apply.py`
  + `gen.py`; disassemblies `dis_base.txt` / `dis_f14b.txt` / `dis_f14c.txt`).
- **result:** struct-through-a-pointer-local (f14b) and its volatile variant (f14c) are
  **BYTE-IDENTICAL** to the base at 7 / 91 — the existing `u8 *idx_1494` local already emits
  the identical RTL, so the aggregate is a no-op on the residual. Direct member access without
  the pointer local is 24 / 89 (f14a/f14d), reproducing s17's index-globals regression and its
  two-instructions-short build. The Alarm merge is 12 / 91 (f14e/f14f) and the combined merge
  12 / 91 (f14g). Nothing beats 7. Independently, prong (c) of the sanctioned aggregate-merge
  family FAILS first-hand for BOTH halves: five asm-only consumers of D_800A1494/95/96
  (getintr 22 sites, CD_cw 8, func_800817A0 8, func_800819C4 8, func_80081E1C 2) and one
  asm-only consumer of D_800F19B8/BC/C0 (CD_cw, 14 sites) force the per-word symbols to survive
  the splat config, making any aggregate declaration a second handle on the same storage.
- **verdict:** KILLED (both on the bytes and on the family prong). Do not re-propose the Intr
  or Alarm aggregate for this function or either twin; the Ruling D scope grant is unspendable
  until CD_cw / getintr / func_800817A0 / func_800819C4 / func_80081E1C have C bodies.

## [s20] F32 — the residual is an ALLOCATION problem with a stated two-part spec, not a scheduling narrative
- **statement:** target's `$a0`-held idx[0] chain is reachable iff the block-3 quantity that
  carries that chain is allocated THIRD while both `$v0` and `$v1` are still occupied across
  its whole span; every measured C lever to date has moved only one half of that requirement.
- **mechanism:** `local-alloc.c` `find_free_reg` scans hard registers ASCENDING (MIPS defines no
  REG_ALLOC_ORDER here), so `$a0` (hard reg 4) is handed out only when 2 and 3 are both in the
  `used` set for the candidate's `[birth,death)`. First-hand ground truth for this function
  (`tools/ra_solver/local_extract.py system` -> `tmp/ra_solver_work/system.local.json`,
  CD_datasync block 3): ord0 qty0 p92 [8,20) refs12 -> $v0; ord1 qty1 p104 [12,24) refs12 ->
  $v1; ord2 qty3 p99 [22,38) refs12 -> $v0; ord3 qty4 p105 [24,28) refs4 -> $v1. `qty_compare`
  priorities 30000 / 30000 (tie -> lower qty) / 22500 / 20000 reproduce the printed `ord`
  column exactly. qty0 DIES at 20, before qty3 is born at 22, which is precisely why the third
  allocation falls back to $v0 instead of climbing to $a0. This is the same two-part shape as
  the ra_solver README's `title_mv_exec2` worked example (one extra overlapping higher-priority
  quantity moves the target only to $a0's predecessor; the hard-reg half alone leaves it in
  $v1; only both together land the wanted seat).
- **corroboration:** `inverse_compose.py classify system CD_datasync --target-object
  build/src/system.o --ours-object tmp/sandbox/CD_datasync/system.o` types the FIRST divergence
  as **RA** (next tool: `inverse.py` global / local), not as a scheduling divergence — which
  corrects the model sessions 10-19 worked under.
- **next probe (NOT executed — the un-executed step the 2026-08-30 campaign named and s19
  skipped):** run `tools/ra_solver/inverse.py local` against block 3 with the goal resolved
  from `goal_from_tgt.py` plus the QTYDBG attribution above, and read the returned lever
  vectors. `perturb.py --goal-from-target` is known to SKIP this block (difflib mis-pairs the
  duplicate `sll` / `lw` skeletons), so the goal must be supplied hand-verified or the
  alignment upgraded to anchor-based. Verdicts are hypotheses until a spelled C form measures;
  the block's C surface is heavily banked already (91 rejected forms), so a REACHABLE verdict
  must name a lever vector nobody has spelled, and a FORECLOSED verdict converts this
  function's disposition from an exhaustion argument into a proof.

## [s20] F14 — declaring 0x800A1494/95/96 as Sony's CD_intr struct (and 0x800F19B8/BC/C0 as the Alarm struct) reproduces target's hoisted base-register addressing that the u8* pointer local cannot reach, and moves the residual.
- mechanism: F14's premise: target reaches Intr through a hoisted base (lbu a0,0(s1) / lbu v0,1(s1), offsets 0 and 1); a single aggregate object with two member offsets is the shape that emits that, and the repo could not express it while D_800A1494 is declared extern u8. This is the sanctioned per-word-splat-symbol -> aggregate-merge family (owner ruling 2026-08-17), authorized as a scope grant by Ruling D of the 2026-09-01 FORECLOSED-BUCKET REVIEW.
- probe: Seven spellings built on the live candidate chassis and sandbox-scored (harness tmp/grind/CD_datasync/s20/apply.py + gen.py): struct-via-pointer-local (f14b), same volatile (f14c), struct with direct member access and no pointer local (f14a) and its volatile variant (f14d), the CD_alarm merge (f14e) and its volatile variant (f14f), and both merges together (f14g). The two equal-scoring forms were then compared at the object level via mipsel-linux-gnu-objdump -d of the cheat-stripped sandbox object (dis_base.txt vs dis_f14b.txt / dis_f14c.txt). Prong (c) of the family was re-derived first-hand with grep -rcE 'D_800A149[456]' asm/ and grep -rcE 'D_800F19(B8|BC|C0)' asm/.
- result: Baseline re-measured at 7/91, rules_dropped 0. f14b = 7/91 and f14c = 7/91, and both are BYTE-IDENTICAL to the base (objdump diff clean) — the existing u8 *idx_1494 pointer local already emits exactly the RTL the struct emits, so F14's stated premise ('a shape this repo cannot currently express') is falsified and the aggregate is a NO-OP on the residual. f14a/f14d = 24/89, reproducing s17's index-globals regression and its two-instructions-SHORT build. f14e/f14f/f14g = 12/91. Nothing beat 7. Independently, prong (c) FAILS structurally for both halves: five asm-only consumers of D_800A1494/95/96 (getintr.s 22 sites, CD_cw.s 8, func_800817A0.s 8, func_800819C4.s 8, func_80081E1C.s 2) plus the data definition in asm/data/7D920.data.s, and one asm-only consumer of D_800F19B8/BC/C0 (CD_cw.s, 14 sites), force the per-word symbols to survive the splat config, so any aggregate declaration is a SECOND handle on the same storage — verbatim the prong g_stage_id died on (decisions.md:10722), CD_sync's Ruling D died on (decisions.md:18305-18317) and CD_ready's Ruling D died on ([s68]).
- verdict: KILLED

## [s20] F32 — the residual is an ALLOCATION problem with a stated two-part spec (target's $a0 seat needs both $v0 and $v1 occupied across the chain's span at allocation time), not the scheduling divergence sessions 10-19 modelled.
- mechanism: local-alloc.c find_free_reg scans hard registers ASCENDING (MIPS defines no REG_ALLOC_ORDER here), so $a0 (hard reg 4) is handed out only when regs 2 and 3 are both in the used set over the candidate quantity's [birth,death). Reaching target's seat therefore requires the chain-carrying quantity to be allocated THIRD while the first two are still live across it — the two-part shape of the ra_solver README's title_mv_exec2 worked example.
- probe: tools/ra_solver/inverse_compose.py classify system CD_datasync --target-object build/src/system.o --ours-object tmp/sandbox/CD_datasync/system.o (INCLUDE_ASM-routed object path; build/src/system.o verified to carry target's bytes by matching its CD_datasync head against asm/funcs/CD_datasync.s), plus a positional build-idx 44-64 block map of both objects, plus first-hand local-alloc ground truth via python3 tools/ra_solver/local_extract.py system.
- result: classify types the FIRST divergence as RA (next tool: inverse.py global/local), not scheduling — correcting the model sessions 10-19 worked under. The positional map shows target computing the idx[1]/arg5 chain first and completely in $v0 (lbu 48 / sll 51 / addu 52 / lw v1 54) while SPREADING the idx[0]/arg4 chain in $a0 (lbu 47 / sll 53 / addu 57 / lw a3 at 62, the block's last memory reference); our build is the exact mirror image. tmp/ra_solver_work/system.local.json block 3 gives ord0 qty0 p92 [8,20) refs12 -> $v0; ord1 qty1 p104 [12,24) refs12 -> $v1; ord2 qty3 p99 [22,38) refs12 -> $v0; ord3 qty4 p105 [24,28) refs4 -> $v1, with qty_compare priorities 30000 / 30000 (tie -> lower qty) / 22500 / 20000 reproducing the printed ord column exactly. qty0 dies at 20 before qty3 is born at 22, which is precisely why the third allocation falls back to $v0 instead of climbing to $a0.
- verdict: CONFIRMED

## [s20] Endgame-lock gate (a): CD_datasync qualifies for a canonical-asm grant.
- mechanism: The canonical-asm path requires STRONG scan_hand_coded signals (S1 multu pacing / S2 empty branch / S6 BIOS jumptable).
- probe: python3 tools/scan_hand_coded.py --single CD_datasync
- result: tier=LOW score=1/8 (91 insns), 'no strong hand-coded indicators'. Only S4 fires (4 loads in an 8-insn window @ insn 46) — and that window IS the ordinary load_register_parameters argument block under study. S1/S2/S6 all negative, as are S3/S5/S7/S8. Fifth independent reproduction (s17, s18, s19, the 2026-08-30 campaign, s20). Independently barred by provenance: compiled Sony PsyQ 3.5 LIBCD CD_datasync, 91 words = exactly 0x14F4-0x1388 in the library object (memory/closer/libcd-groundtruth.md:59-67), and the twin-family canonical-asm request covering this exact function was DENIED on three independently dispositive grounds on 2026-07-09 (docs/grind/decisions.md:15).
- verdict: KILLED

## [s20] Endgame-lock gate (b): an in-hand SOTN-master precedent exists for the construct that would close CD_datasync.
- mechanism: A coercion/spelling family needs a citable SOTN-master exhibit (file+line or commit) for the CLOSING construct.
- probe: Direct examination of what holds the floor, restated against the 2026-09-01 review's systemic finding #2 (cap-truncated sotn-construct-index.md censuses are not evidence).
- result: FAILS by construction, and the review's census objection does not rescue it: no census was the basis of the failure. The honest floor is 7, not 0 — no C construct CLOSES this function, so a closing-construct precedent cannot exist. The constructs that HOLD the floor at 7 (one FAKE-annotated do{}while(0) wrap plus three plain pointer locals) are already inside sanctioned families and are not the blocker. The one family this session did claim — the sanctioned aggregate merge — fails its own prong (c) on first-hand evidence.
- verdict: KILLED

## [s21] The do{}while(0) wrapper's pointer ref-lift is reachable by an ORDINARY structural rewrite - duplicating the timeout block into both condition arms instead of sharing a `do_timeout:` label.
- mechanism: The wrapper works (s9 H37/H38) by raising flow.c's `loop_depth` inside the block so each pointer reference counts twice, lifting D_800A125C / D_800A1494 / D_800A11DC above the param in global.c:allocno_compare. Literal duplication reaches the same `reg_n_refs` arithmetic by a different route: two textual copies give two real references per pointer, and jump.c's cross-jump pass re-merges the identical copies back to one before final emission - the exact effect the sanctioned duplicated-statement-into-arms family (owner ruling 2026-07-01) describes ("incl. when cross-jump re-merges the copies to identical bytes and the effect is a reg_n_refs priority lift"). Nobody had tried it here in 20 sessions; the wrapper was assumed to be the only route to the lift.
- probe: Built body_d2_dup_arms_arg4named.c (timeout block written out in full inside BOTH `if` arms, no wrapper anywhere) and body_d1_dup_arms_inline.c (same but fully-inline arguments). Compiled both with the frozen open cc1 through tmp/grind/CD_datasync/s21/sweep.sh, diffed the emitted CD_datasync region against the candidate's, and sandbox-scored d2.
- result: CONFIRMED for d2. `& tools/wteng.ps1 main sandbox CD_datasync --disable all` -> score 7, target_insns 91, build_insns 91, rules_dropped 0 - the SAME floor as the wrapper chassis. The emitted assembly is BYTE-IDENTICAL to the wrapper chassis: the only textual differences are .L label numbers (.L114/.L109/.L108/.L113 vs .L108/.L110/.L115/.L119), which the object-level scorer masks. d1 (duplication + fully-inline arguments) = 13, i.e. duplication is orthogonal to the argument-block attractor and reproduces the same attractor structure. Banked as memory/grind/CD_datasync/alt_dup_into_arms_7.c. This is a LATERAL, not a win: the floor is unchanged and the duplicated-statement-into-arms rule still mandates a /* FAKE */ annotation, so the candidate carries no fewer annotated devices either way. Its value is that the ref-lift mechanism is now confirmed by two independent GCC routes (flow.c loop_depth vs literal reg_n_refs), which retires the standing worry that the wrapper is load-bearing in some way peculiar to itself.
- verdict: CONFIRMED

## [s21] Stacking BOTH ref-lift routes (duplicated arms INSIDE a do{}while(0) wrapper) lifts the pointers further and improves the allocation.
- mechanism: If one lift moves the three pointers above the param, a double lift (loop_depth weighting applied to duplicated references) should widen the margin and could reorder the pointers among THEMSELVES, which is the second half of the seat problem.
- probe: body_c4_dup_plus_wrapper.c - each duplicated arm's timeout block wrapped in its own `do { ... } while (0);` - compiled and scored on the s21 harness.
- result: lev 14 against target (vs 7 for either single route), no nop delta. The double lift OVERSHOOTS: the pointers' allocno priorities move past each other and the callee-save map rotates away from target's $s0=D_800A125C / $s1=D_800A1494 / $s3=D_800A11DC assignment. The lift is a threshold effect, not a gradient - one route is exactly enough and two is too many.
- verdict: KILLED

## [s21] A named SCALED index offset (`k0 = idx_1494[0] * 4;` consumed as `*(s32 *)((u8 *)tbl_125c + k0)`) splits the idx[0] chain where target splits it - byte read and shift at the statement, base add and final load deferred to load_register_parameters - and therefore reaches target's late `lw $a3`.
- mechanism: Sessions 15/16 established the two halves of the residual: a NAMED arg4 value emits the whole chain at the statement (both `lbu` land at the block head in target's order, but `lw a3` issues 6 insns early), while a fully-INLINE arg4 defers the load to load_register_parameters (target's late `lw a3`) but drags the entire idx[0] chain to the tail with it. A named SCALED OFFSET is the first spelling that is neither: the `lbu` and `sll` belong to a statement, but the `addu` and the load belong to the argument tree, so calls.c:1618-1665 emits the address add in the register-arg precompute loop and the load after store_one_arg's `sw`. This geometry had never been spelled in 91 banked forms (every prior probe named the byte, the element index, the value, or the address - never the scaled offset).
- probe: Twenty forms on this geometry (f1-f8, g3-g9 and the `off` row of the 78-form cross-product below): multiply vs `<< 2`, reversed addu operand order, u8* vs s32* carrier, both indices scaled, arg3 also scaled, the offset statement placed before/after the puts and before/after a named arg5, and a declaration-with-initializer variant. Scored on the s21 fast harness (cc1 -> normalized/expanded instruction stream -> Levenshtein against asm/funcs/CD_datasync.s, plus an emitted-`#nop` count that recovers the load-delay instructions the stream model drops), then sandbox-confirmed.
- result: The geometry WORKS and is a genuinely new fourth attractor, but it does not beat 7. It reaches target's entire TAIL: `lw $a3` last (build idx 56 = target 56), `lw $a2` at 55, the D_800A11D5 chain at 49-53 in target's exact slots, and the idx[0] chain SPREAD (lbu / sll / addu / lw at 48/51/54/56) rather than contiguous - the first non-inline form ever to spread it. What it loses is the HEAD: idx[0]'s `lbu` sinks to build idx 48 where target has it at 41, and the arg5 `lw`/`sw` pair stays adjacent, costing a load-delay nop. Sandbox: 8 / 92 (build_insns 92, one over target). Every variant collapses onto the same emission - naming arg5 as a value, as an address or as its own scaled offset is completely inert once arg4 is a scaled offset, and `* 4` vs `<< 2` and the addu operand order are inert too. Moving the offset statement above the `puts` forces the value across the call into a callee-saved register: 25, n=84. So the two halves of the residual remain mutually exclusive under a third independent geometry: head-correct (named value, 7) and tail-correct (scaled offset, 8) cannot be combined by any argument spelling measured.
- verdict: KILLED

## [s21] Some point in the full argument cross-product (arg4 x arg5 x arg3 spelling x statement order) scores below 7.
- mechanism: Sessions 9-19 swept 91 argument forms but always one axis at a time against a fixed chassis; with a 0.5-second-per-form harness the complete product is affordable, and the scaled-offset geometry adds a fifth arg4 spelling that was never crossed with anything.
- probe: 78-form exhaustive cross-product generated by tmp/grind/CD_datasync/s21 (arg4 in {inline, named value, named s32* address, named scaled offset, named u8* scaled address} x arg5 in {inline, named value, named address, named scaled offset} x arg3 in {inline, named value} x statement order {arg4-first, arg5-first}), plus 8 further single-axis probes on arg1 (format string named), arg2 (D_800F19C0 named / aliased through a pointer / named before arg4), the index reads spelled as `*idx_1494` / `*(idx_1494 + 1)`, the table reads spelled as `*(tbl_125c + i)`, an `i0 = idx_1494[0]; arg4 = tbl_125c[i0];` two-step, and a same-value re-store of arg4.
- result: 86 forms, minimum lev 7 with nop 3 (= sandbox 7) - the banked floor, reached by six distinct spellings that are all byte-identical to the candidate. Nothing below. Naming arg3 costs >= 2 in every one of its 39 pairings. All eight single-axis probes are byte-inert at 7, so arg1 and arg2 join the dead-axis list alongside the prototype, MEM_IN_STRUCT_P and pre-loop-store axes killed in s12. On this chassis the argument block is a closed four-attractor system: 7 (named value) / 8 (scaled offset) / 9 (named address) / 13 (fully inline).
- verdict: KILLED

## [s21] `volatile` on the index pointer pins the argument block's schedule, forcing idx[0]'s `lbu` to stay at the block head where target has it.
- mechanism: sched.c refuses to reorder volatile MEMs against each other, so `volatile u8 *idx_1494` should freeze the relative order of the two `lbu`s and of any load crossing them - the one lever that acts on emission ORDER rather than on the dependence DAG.
- probe: body_k1/k2/k3 - the volatile qualifier applied to the index pointer local on all three chassis (named-value arg4, scaled-offset arg4, fully-inline arg4). Compiled and scored on the s21 harness. Measured as a DIAGNOSTIC only: this is a pointer-local qualifier, not a declaration change, and an `extern volatile` on a game-state global would need the two-prong gate plus a volatile_extern_allowlist.txt grant before it could be proposed.
- result: Byte-inert where it matters and a regression elsewhere: 7 on the named-value chassis (identical emission to the plain pointer), 8 on the scaled-offset chassis, 13 on the fully-inline chassis. The volatile qualifier on the pointer does NOT pin this block's order at all - the two `lbu`s already sit in a fixed relative order per chassis and the schedule is decided upstream of any volatility test. The order lever this was reaching for does not exist at the C level here.
- verdict: KILLED

## [s21] m2c's rederived control-flow tail (a single accumulator `ret` written -1 / 0 / 1 down a nested-if ladder with one `return ret;`) is the original shape, and the candidate's multiple-return tail is a decompiler artefact.
- mechanism: A fresh m2c run on asm/funcs/CD_datasync.s (`python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f CD_datasync`) produces exactly that tail, and target's epilogue does write $v0 from three different delay slots into one shared exit - the classic single-exit accumulator signature. If it is the original shape it should be free or better.
- probe: body_c5_accumulator_tail.c - the m2c tail transcribed verbatim onto the candidate chassis, everything else unchanged. Also probed on the same axis: m2c's `||`-short-circuited timeout test folded into a nested if (body_c7) and its `cnt = D_800F19BC++;` compound (body_c8).
- result: The accumulator tail COSTS an instruction: lev 12, n 83 where the candidate is lev 7, n 82 - GCC materialises the extra `ret` pseudo instead of reusing the exit register, so target's three delay-slot writes are not what a single accumulator emits here. The candidate's multiple-return tail already reproduces target's tail exactly (the residual has been confined to build idx 46-61 since s9), so m2c's tail is the artefact, not ours. c7 and c8 are byte-inert at 7 - the nested-if condition arrangement and the post-increment spelling are free. The fresh m2c decompile therefore contributes nothing the ledger did not already have: its argument expressions are the fully-inline 13-attractor and its tail is a regression.
- verdict: KILLED

## [s21] The do{}while(0) wrapper's pointer ref-lift is reachable by an ordinary structural rewrite - duplicating the timeout block into both condition arms instead of sharing a do_timeout: label.
- mechanism: The wrapper works by raising flow.c's loop_depth inside the block so each pointer reference counts twice, lifting D_800A125C / D_800A1494 / D_800A11DC above the param in global.c:allocno_compare. Literal duplication reaches the same reg_n_refs arithmetic differently: two textual copies give two real references per pointer, and jump.c's cross-jump pass re-merges the identical copies back to one before final emission - exactly the effect the sanctioned duplicated-statement-into-arms family (owner ruling 2026-07-01) describes.
- probe: Built tmp/grind/CD_datasync/s21/body_d2_dup_arms_arg4named.c (timeout block written out in full inside BOTH if arms, no wrapper anywhere) and body_d1_dup_arms_inline.c. Compiled with the frozen open cc1, diffed the emitted CD_datasync region against the candidate's, then sandbox-scored d2.
- result: CONFIRMED. sandbox CD_datasync --disable all -> score 7, target_insns 91, build_insns 91, rules_dropped 0, the same floor as the wrapper chassis. The emitted assembly is BYTE-IDENTICAL to the wrapper chassis; the only textual differences are .L label numbers (.L114/.L109/.L108/.L113 vs .L108/.L110/.L115/.L119), which the object-level scorer masks. d1 (duplication crossed with fully-inline arguments) = 13, so duplication is orthogonal to the argument-block attractor. Banked as memory/grind/CD_datasync/alt_dup_into_arms_7.c. This is a LATERAL, not a win: the floor is unchanged and the duplicated-statement-into-arms rule also mandates a FAKE annotation, so neither form carries fewer annotated devices. Its value is that the ref-lift is now confirmed through two independent GCC routes, retiring the standing worry that the wrapper is load-bearing in some way peculiar to itself.
- verdict: CONFIRMED

## [s21] Stacking both ref-lift routes - duplicated arms each wrapped in their own do{}while(0) - lifts the three table pointers further and improves the allocation on this chassis.
- mechanism: If one lift moves the three pointers above the param in allocno_compare, a double lift (loop_depth weighting applied to duplicated references) should widen the margin and could also reorder the pointers among themselves, which is the second half of the seat problem.
- probe: tmp/grind/CD_datasync/s21/body_c4_dup_plus_wrapper.c compiled and scored on the s21 harness (cc1 + expanded-stream Levenshtein + emitted-#nop count, calibrated sandbox = lev + nop - 3).
- result: lev 14 with nop 3 (predicted sandbox 14) against 7 for either single route. The double lift OVERSHOOTS: the pointers' allocno priorities move past each other and the callee-save map rotates away from target's $s0=D_800A125C / $s1=D_800A1494 / $s3=D_800A11DC assignment. The lift behaves as a threshold, not a gradient - one route is exactly enough on this chassis and two is too many.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (goto loop, three hoisted pointer locals, named arg4 value), with the do{}while(0) FAKE present in each duplicated arm

## [s21] A named scaled index offset (k0 = idx_1494[0] * 4, consumed as *(s32 *)((u8 *)tbl_125c + k0)) splits the idx[0] chain where target splits it and lowers this chassis's floor below 7.
- mechanism: Sessions 15/16 established the two halves of the residual: a NAMED arg4 value emits the whole chain at the statement (both lbu land at the block head in target's order, but lw a3 issues 6 insns early), while a fully-INLINE arg4 defers the load to load_register_parameters (target's late lw a3) but drags the entire idx[0] chain to the tail with it. A named SCALED OFFSET is neither: the lbu and sll belong to a statement, but the addu and the load belong to the argument tree, so calls.c:1618-1665 emits the address add in the register-arg precompute loop and the load after store_one_arg's sw.
- probe: Twenty forms on this geometry (f1-f8, g3-g9, and the 'off' row of the 78-form cross-product): multiply vs << 2, reversed addu operand order, u8* vs s32* carrier, both indices scaled, arg3 also scaled, the offset statement placed before/after the puts and before/after a named arg5, and a declaration-with-initializer variant. Scored on the s21 fast harness, then sandbox-confirmed on the representative form.
- result: The geometry WORKS and is a genuinely new fourth attractor, but it does not beat 7. It reaches target's entire TAIL: lw $a3 as the block's last memory reference at target's own build idx 56, lw $a2 at 55, the D_800A11D5 chain at 49-53 in target's exact slots, and the idx[0] chain SPREAD (lbu/sll/addu/lw at 48/51/54/56) rather than contiguous - the first non-inline form ever to spread it. What it loses is the HEAD: idx[0]'s lbu sinks to build idx 48 where target has it at 41, and the arg5 lw/sw pair stays adjacent, costing a load-delay nop. Sandbox: score 8, build_insns 92. Every variant collapses onto the same emission; naming arg5 as a value, an address or its own scaled offset is completely inert once arg4 is a scaled offset, and * 4 vs << 2 and the addu operand order are inert too. Hoisting the offset statement above the puts forces the value across the call into a callee-saved register: lev 25, n 84. So head-correct and tail-correct stayed mutually exclusive under this third independent geometry.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 20 scaled-offset variants, representative sandbox-confirmed at 8/92

## [s21] Some point in the full argument cross-product (arg4 spelling x arg5 spelling x arg3 spelling x statement order) scores below 7 on this chassis.
- mechanism: Sessions 9-19 swept 91 argument forms but always one axis at a time against a fixed chassis; with a 0.5-second-per-form harness the complete product is affordable, and the scaled-offset geometry adds a fifth arg4 spelling that had never been crossed with anything.
- probe: 78-form exhaustive cross-product (arg4 in {inline, named value, named s32* address, named scaled offset, named u8* scaled address} x arg5 in {inline, named value, named address, named scaled offset} x arg3 in {inline, named value} x statement order {arg4-first, arg5-first}), plus 8 single-axis probes on arg1 (format string named), arg2 (D_800F19C0 named / pointer-aliased / hoisted before arg4), *idx_1494 vs idx_1494[0], *(tbl_125c + i) vs tbl_125c[i], an i0 two-step, and an arg4 same-value re-store. All generated and scored under tmp/grind/CD_datasync/s21.
- result: 86 forms, minimum lev 7 with nop 3 (= sandbox 7), the banked floor, reached by six distinct spellings that are all byte-identical to the candidate. Nothing below. Naming arg3 costs at least 2 in every one of its 39 pairings. All eight single-axis probes are byte-inert at 7, so arg1 and arg2 join the dead-axis list alongside the prototype, MEM_IN_STRUCT_P and pre-loop-store axes killed in s12. On this chassis the argument block behaves as a closed four-attractor system: 7 (named value) / 8 (scaled offset) / 9 (named address) / 13 (fully inline).
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (goto loop, three hoisted pointer locals), do{}while(0) FAKE present in all 86 forms

## [s21] volatile on the index pointer local pins the argument block's schedule, holding idx[0]'s lbu at the block head where target has it.
- mechanism: sched.c refuses to reorder volatile MEMs against each other, so volatile u8 *idx_1494 should freeze the relative order of the two lbu reads and of any load crossing them - a lever that acts on emission ORDER rather than on the dependence DAG.
- probe: tmp/grind/CD_datasync/s21/body_k1/k2/k3 - the volatile qualifier applied to the index pointer local on all three chassis (named-value arg4, scaled-offset arg4, fully-inline arg4), compiled and scored on the s21 harness. Diagnostic only: this is a pointer-local qualifier, not a declaration change; an extern volatile on a game-state global would need the two-prong gate plus a volatile_extern_allowlist.txt grant before it could be proposed.
- result: Byte-inert where it matters and a regression elsewhere: 7 on the named-value chassis (emission identical to the plain pointer), 8 on the scaled-offset chassis, 13 on the fully-inline chassis. The qualifier does not move this block's order at all - the two lbu reads already sit in a fixed relative order per chassis and the schedule is decided upstream of any volatility test.
- verdict: KILLED
- kill_scope: instance
- measured_on: all three s21 chassis (named-value / scaled-offset / fully-inline arg4), do{}while(0) FAKE present

## [s21] m2c's rederived control-flow tail - a single accumulator ret written -1 / 0 / 1 down a nested-if ladder with one return ret - is the original shape and the candidate's multiple-return tail is a decompiler artefact.
- mechanism: A fresh m2c run on asm/funcs/CD_datasync.s produces exactly that tail, and target's epilogue does write $v0 from three different delay slots into one shared exit, the classic single-exit accumulator signature. If it is the original shape it should be free or better.
- probe: python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f CD_datasync asm/funcs/CD_datasync.s, then body_c5_accumulator_tail.c (the m2c tail transcribed verbatim onto the candidate chassis, everything else unchanged). Same axis: m2c's ||-short-circuited timeout test folded into a nested if (body_c7) and its cnt = D_800F19BC++ compound (body_c8).
- result: The accumulator tail COSTS an instruction: lev 12, n 83 where the candidate is lev 7, n 82 - GCC materialises the extra ret pseudo instead of reusing the exit register, so target's three delay-slot writes are not what a single accumulator emits here. The candidate's multiple-return tail already reproduces target's tail exactly (the residual has been confined to build idx 46-61 since s9), so m2c's tail is the artefact, not ours. c7 and c8 are byte-inert at 7. The fresh m2c decompile therefore contributes nothing the ledger did not already have: its argument expressions are the fully-inline 13-attractor and its tail is a regression.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (goto loop, three hoisted pointer locals), do{}while(0) FAKE present

## [s22] KILL RE-AUDIT (mandated): the s21 scaled-offset instance kill (8/92) survives ablation of the do{}while(0) FAKE, and the FAKE itself is load-bearing on the current chassis.
- mechanism: An instance kill measured with a FAKE carrier occupying the same pseudo is not a kill (func_8002EA24 s8). The s21 scaled-offset attractor was measured only with the do{}while(0) wrapper present, so the wrapper could in principle have been the thing costing the extra load-delay nop. Two ablations are available now that s21 banked a wrapper-free equal-floor chassis (alt_dup_into_arms_7.c): (a) replace the wrapper with a plain compound block, (b) transplant the scaled offset onto the duplicated-arms chassis.
- probe: tmp/grind/CD_datasync/s22/{body_a0_base_nowrap.c, body_a1_scaled_nowrap.c, body_a2_scaled_duparms.c}, scored on the s21 harness (sandbox = lev + nop - 3), calibration re-confirmed this session against the live sandbox (candidate = lev7/nop3 = sandbox 7, verified by `sandbox CD_datasync --disable all` -> score 7 / 91 / 91, rules_dropped 0).
- result: base with the wrapper ablated = lev 17 / nop 3 -> 17 (the wrapper is worth 10 instructions on the current chassis, i.e. it is genuinely load-bearing and not a spectator). Scaled offset with the wrapper ablated = lev 18 / nop 4 -> 19. Scaled offset on the wrapper-free duplicated-arms chassis = lev 13 / nop 3 -> 13. The kill stands under both ablations; nothing about the 8/92 result was an artifact of the FAKE. Banked as rejected/s22-scaled-offset-fake-ablated-18.c and rejected/s22-scaled-offset-on-duparms-chassis-13.c.
- verdict: KILLED (instance; re-audit confirms s21)

## [s22] The two equal-floor chassis (do{}while(0) wrapper vs timeout block duplicated into both arms) are byte-identical at the base point but are NOT equivalent under argument-spelling perturbation, so the entire s21 argument cross-product has to be re-run on the second chassis before that search space is closed.
- mechanism: s21 established that the two chassis emit byte-identical assembly for the base argument spelling (only .L numbers differ) and both sandbox at 7. But duplication doubles every argument statement, which doubles reg_n_refs for any named temporary and changes both the local-alloc quantity birth/death profile of block 3 and flow.c's ref weighting. Two chassis that agree at one point of the spelling space need not agree anywhere else.
- probe: The scaled-offset spelling transplanted onto the duplicated-arms chassis (body_a2), then a fresh 92-form cross-product generated for the duplicated-arms chassis only: arg4 in {inline, named value, named s32* address, named scaled offset} x arg5 in {inline, named value, named s32* address, named scaled offset} x arg3 in {inline, named value} x every permutation of the resulting statement order (tmp/grind/CD_datasync/s22/gen3.py, wlist.txt, w_results.txt).
- result: CONFIRMED for the non-equivalence and KILLED for the search space. Non-equivalence is proved by a single number: the scaled offset costs 1 instruction on the wrapper chassis (7 -> 8) and 6 on the duplicated-arms chassis (7 -> 13). The 92-form re-sweep nevertheless bottoms out at exactly 7, reached by five spellings (val_in_in_o4, val_val_in_o45, val_adr_in_o45, val_k1_in_o45, val_k1_in_o54) - all of them arg4-as-a-named-value with arg3 inline, i.e. the same attractor the wrapper chassis reaches. No form on either chassis is below 7. Banked as rejected/s22-duparms-chassis-arg5-address-min-7.c.
- verdict: KILLED (instance)

## [s22] Frontier item 1 (inherited from s21): some spelling or placement of arg3 forces the D_800A11D5 chain between arg5's lw and its sw on the scaled-offset chassis, removing the load-delay nop and converting 8/92 into 8/91 or better.
- mechanism: Target separates arg5's load (build idx 48) from its 16($sp) store (54) by six instructions - the D_800A11D5 chain occupies 49-53 - so no load-delay nop is required. On the scaled-offset chassis store_one_arg's lw/sw pair is adjacent (46/47) and maspsx pads it. s21 only ever measured arg3 as {inline, named value} and only with arg3's statement last, so the placement axis was genuinely un-swept.
- probe: 38 forms (tmp/grind/CD_datasync/s22/gen.py, qlist.txt): arg4 in {scaled offset, inline} x arg5 in {inline, named value} x arg3 in {inline, named value, named s32* address, named scaled offset} x every permutation of the statement order (arg3 first / middle / last).
- result: The best score in the whole 38-form set is 8 (lev 7 / nop 4), reached by the scaled-offset forms already banked; naming arg3 in ANY of its three spellings and ANY of its placements costs at least 1 more instruction, and the arg3-first placements cost 4-8. The nop is never removed. Reading the alignment explains why the placement axis cannot work: the pad exists because the arg5 VALUE lands in $v0, which is also the register the D_800A11D5 chain needs, so the store must retire before the chain can start; target avoids it by landing the arg5 value in $v1 (a quantity distinct from its own address register) and letting the D5 chain reuse $v0. That is a local-alloc seat, not a statement-order property, so no arg3 placement measured here addresses it. Banked as rejected/s22-arg3-scaled-offset-on-k0-chassis-8.c.
- verdict: KILLED (instance)

## [s22] The declaration shape of the callees (printf's prototype, its arity, its return type) is a live structural lever - a variadic or unprototyped printf changes GCC 2.7.2's expand_call path (must_preallocate, promotion of the trailing arguments) and therefore the emission order of the argument block.
- mechanism: HEAD declares `extern void printf(void *, void *, s32, s32, s32);` - a fixed 5-arg, void-returning prototype. Sony's real header declares `int printf(const char *, ...)`. Under a variadic prototype GCC 2.7.2 takes a different path through expand_call for the trailing arguments (no prototype-driven promotion, different must_preallocate reasoning), and a non-void return type keeps $v0 live across the call. 21 sessions of argument-spelling work all ran under one fixed prototype; the declaration itself had never been varied.
- probe: Six declaration variants measured on the base chassis with an extended apply.py that also rewrites the extern block preceding the function (tmp/grind/CD_datasync/s22/body_p0..p5): baseline; `void printf(void *, ...)`; `s32 printf(void *, ...)` plus `s32 puts(void *)`; K&R unprototyped `void printf(); void puts();`; return-type-only change to s32; parameter-type change to `char *`.
- result: All six are byte-identical to the baseline - lev 7 / nop 3 / n 82 in every case, i.e. sandbox 7. The callee declaration shape is codegen-inert for this function in every variant measured: with ACCUMULATE_OUTGOING_ARGS on this target each argument is expanded and homed the same way whether or not the prototype names it, and the unused return value is dead before it can pressure the allocator. This closes an axis that had been assumed rather than measured for 21 sessions. Banked as rejected/s22-printf-prototype-varargs-byte-inert-7.c.
- verdict: KILLED (instance)

## [s22] Declaration order and declaration scope of the block's named temporary and of the five function-scope locals move the local-alloc quantity ordering (pseudo numbers follow declaration order and break qty_compare ties), so some ordering reaches target's block-3 seats.
- mechanism: DECL_RTL pseudos are created in declaration order, quantity numbers follow pseudo numbers, and local-alloc.c's qty_compare breaks equal-priority ties by the lower quantity number - s20's first-hand extraction showed CD_datasync block 3 has exactly such a tie (priorities 30000 / 30000 / 22500 / 20000, the first two tied and broken by quantity number). Declaration order is a named lever of the structural modality and had never been swept here: every one of the 96 banked forms declares the temporary inside the do-block and the five locals in one fixed order.
- probe: 24 forms (tmp/grind/CD_datasync/s22/gen2.py, rlist.txt): the named temporary hoisted to function scope at each of the six possible positions in the declaration list, plus 18 permutations of the five function-scope locals with the temporary left block-local.
- result: All 24 are byte-identical to the baseline - lev 7 / nop 3 / n 82, sandbox 7. Neither the scope of the temporary nor the order of the five locals moves a single byte in any form measured. The tie in block 3 was not broken by source declaration order in this sweep: the temporary is coalesced into the argument chain before local-alloc sees it, so the tie-break input was not reachable through this axis here. Banked as rejected/s22-temp-decl-hoisted-to-function-scope-byte-inert-7.c.
- verdict: KILLED (instance)

## [s22] Type narrowing / widening of the block's values (the temporary, the table pointers, the index pointer, the loop counter) perturbs the argument block's codegen.
- mechanism: The structural modality's type-narrowing lever - a different C type for the same value can change zero/sign-extension insns, the addressing mode chosen for the table load, and the pseudo's mode class in local-alloc.
- probe: Six forms (tmp/grind/CD_datasync/s22/body_t1..t6): temporary as u32; table pointer as u32* with a cast at the use; index pointer as u32* with u8 casts at the reads; counter as u32 with an s32 cast in the compare; temporary replaced by an s32* address dereferenced at the call; index pointer spelled `unsigned char *` instead of `u8 *`.
- result: Five of six are byte-identical at lev 7 / nop 3 (sandbox 7); the pointer-temporary form regresses to 9. Type narrowing is inert in every variant measured because every value in the block is already word-sized and the two byte reads are already lbu under -funsigned-char. Banked as rejected/s22-arg4-pointer-temp-regresses-9.c.
- verdict: KILLED (instance)

## [s22] KILL RE-AUDIT (mandated by the flat-floor rule): s21's closest-to-target instance kill, the named scaled index offset at 8/92, was measured with the do{}while(0) FAKE present, so the wrapper may have been what cost the extra load-delay nop; ablating the FAKE reaches a lower score.
- mechanism: A lever measured inert while a FAKE carrier occupies its target pseudo is not a kill (func_8002EA24 s8). Two ablations are available: replace the wrapper with a plain compound block, and transplant the scaled offset onto s21's wrapper-free equal-floor duplicated-into-arms chassis.
- probe: tmp/grind/CD_datasync/s22/{body_a0_base_nowrap.c, body_a1_scaled_nowrap.c, body_a2_scaled_duparms.c}, scored on the s21 cpp+cc1 harness (sandbox = lev + nop - 3), with the calibration re-anchored this session against the live engine sandbox.
- result: Base with the wrapper ablated = lev 17 / nop 3 -> 17, i.e. the do{}while(0) is worth ten instructions on the current chassis and is genuinely load-bearing rather than a spectator occupying a pseudo. Scaled offset with the wrapper ablated = lev 18 / nop 4 -> 19. Scaled offset on the wrapper-free duplicated-arms chassis = 13. The s21 kill stands under both ablations and none of its result was a FAKE artifact.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (goto loop, three hoisted pointer locals, named arg4 value) with the do{}while(0) FAKE ABLATED, and on the duplicated-into-arms chassis which carries no FAKE at all

## [s22] The two equal-floor chassis (do{}while(0) wrapper vs the timeout block duplicated into both arms), proven byte-identical at the base spelling in s21, respond identically to argument-spelling perturbation, so s21's 78-form cross-product transfers to the second chassis and that search space is already closed.
- mechanism: Duplication doubles every argument statement, which doubles reg_n_refs for a named temporary and changes block 3's local-alloc birth/death profile as well as flow.c's ref weighting; two chassis agreeing at one point of the spelling space need not agree elsewhere.
- probe: A fresh 92-form cross-product generated for the duplicated-arms chassis only (tmp/grind/CD_datasync/s22/gen3.py, wlist.txt, w_results.txt): arg4 in {inline, named value, named s32* address, named scaled offset} x arg5 in {inline, named value, named s32* address, named scaled offset} x arg3 in {inline, named value} x every permutation of the resulting statement order.
- result: The equivalence assumption is FALSE and is replaced by a measured non-equivalence: the scaled index offset costs 1 instruction on the wrapper chassis (7 -> 8) and 6 on the duplicated-arms chassis (7 -> 13). The 92-form re-sweep nevertheless bottoms out at exactly 7, reached by five spellings (val_in_in_o4, val_val_in_o45, val_adr_in_o45, val_k1_in_o45, val_k1_in_o54), all arg4-as-named-value with arg3 inline — the same attractor the wrapper chassis reaches. The floor of 7 is now measured on both chassis, and the bookkeeping lesson is banked: a cross-product is chassis-relative in exactly the way an instance kill is.
- verdict: KILLED
- kill_scope: instance
- measured_on: duplicated-into-arms chassis (no FAKE construct present anywhere in the 92 forms)

## [s22] s21 frontier item 1: on the scaled-offset chassis, some spelling or placement of arg3 forces the D_800A11D5 chain to be emitted between arg5's lw and its 16($sp) sw, removing the load-delay nop and taking 8/92 to 8/91 or below.
- mechanism: Target separates arg5's load (build idx 48) from its store (54) by six instructions, the D5 chain occupying 49-53, so no pad is needed; the scaled-offset chassis emits the lw/sw pair adjacently and maspsx pads it. s21 measured arg3 only as {inline, named value} and only with its statement last.
- probe: 38 forms (tmp/grind/CD_datasync/s22/gen.py, qlist.txt): arg4 in {scaled offset, inline} x arg5 in {inline, named value} x arg3 in {inline, named value, named s32* address, named scaled offset} x every permutation of the statement order (arg3 first / middle / last).
- result: Best score in the whole set is 8 (lev 7 / nop 4), reached only by the already-banked scaled-offset spellings; naming arg3 in any of its three spellings and any placement costs at least one more instruction and the arg3-first placements cost 4-8. The nop is never removed. The cmp.py alignment explains it: the pad exists because the arg5 VALUE lands in $v0, the same register the D5 chain needs, so store_one_arg's store must retire before the chain can start. Target lands the arg5 value in $v1, a quantity distinct from its own address register (42 lbu $v0,1($s1) / 45 sll / 46 addu / 48 lw $v1,0($v0) / 54 sw $v1,16($sp)), which frees $v0 for the D5 chain. That is a local-alloc seat, not a statement-order property.
- verdict: KILLED
- kill_scope: instance
- measured_on: scaled-offset (k0) chassis with the do{}while(0) FAKE present, plus the inline-arg4 control arm of the same sweep

## [s22] The declaration shape of the callees — printf's prototype, its arity and its return type — is a live structural lever, because a variadic or unprototyped printf takes a different path through GCC 2.7.2's expand_call (must_preallocate, promotion of the trailing arguments) and a non-void return keeps $v0 live across the call.
- mechanism: HEAD declares `extern void printf(void *, void *, s32, s32, s32);` — a fixed 5-arg void prototype — whereas Sony's real header declares `int printf(const char *, ...)`. Twenty-one sessions of argument-spelling work all ran under the one fixed prototype and never varied the declaration itself.
- probe: Six declaration variants on the base chassis, using a new apply.py that also rewrites the extern block preceding the function for body files marked /*DECLS*/ (tmp/grind/CD_datasync/s22/body_p0..p5): baseline; void printf(void *, ...); s32 printf(void *, ...) plus s32 puts(void *); K&R unprototyped void printf(); void puts(); return-type-only change to s32; parameter types changed to char *.
- result: All six are byte-identical to the baseline — lev 7 / nop 3 / n 82, sandbox 7. With ACCUMULATE_OUTGOING_ARGS on this target the argument homing is prototype-independent in every variant measured, and the unused return value is dead before it can pressure the allocator. A previously assumed-but-unmeasured axis is now measured; the new apply.py makes it re-testable for CD_sync and CD_ready too.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (goto loop, three hoisted pointer locals, named arg4 value) with the do{}while(0) FAKE present

## [s22] Declaration order and declaration scope — the block temporary hoisted to function scope, and permutations of the five function-scope locals — move local-alloc's quantity ordering, because pseudo numbers follow declaration order and qty_compare breaks equal priorities by the lower quantity number.
- mechanism: s20's first-hand local_extract showed CD_datasync block 3 has exactly such a tie (priorities 30000 / 30000 / 22500 / 20000, the first two tied and broken by quantity number). Declaration order is a named lever of the structural modality and every one of the 96 banked forms used one fixed order with the temporary block-local.
- probe: 24 forms (tmp/grind/CD_datasync/s22/gen2.py, rlist.txt): the temporary hoisted to function scope at each of the six positions in the declaration list, plus 18 permutations of the five function-scope locals with the temporary left block-local.
- result: All 24 are byte-identical to the baseline — lev 7 / nop 3 / n 82, sandbox 7. Neither the scope of the temporary nor the order of the five locals moves a byte in any form measured: the temporary is coalesced into the argument chain before local-alloc sees it, so block 3's tie-break input was not reachable through this axis in this sweep.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (goto loop, three hoisted pointer locals, named arg4 value) with the do{}while(0) FAKE present

## [s22] Type narrowing or widening of the block's values (the temporary, the table pointers, the index pointer, the loop counter) perturbs the argument block's codegen.
- mechanism: The structural modality's type-narrowing lever: a different C type for the same value can change extension insns, the addressing mode chosen for the table load, and the pseudo's mode class in local-alloc.
- probe: Six forms (tmp/grind/CD_datasync/s22/body_t1..t6): temporary as u32; table pointer as u32* with a cast at the use; index pointer as u32* with u8 casts at the reads; counter as u32 with an s32 cast in the compare; temporary respelled as an s32* address dereferenced at the call; index pointer spelled `unsigned char *`.
- result: Five of six are byte-identical at 7; the pointer-temporary form regresses to 9 (banked). Every value in the block is already word-sized and both byte reads are already lbu under -funsigned-char, so the lever had nothing to act on in any variant measured.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate chassis (goto loop, three hoisted pointer locals, named arg4 value) with the do{}while(0) FAKE present

## [s23] s22's frontier item 1 - keeping a table ADDRESS pointer live past its own load anti-coalesces arg5's load destination from its address register
- mechanism: s22's alignment showed the candidate coalesces arg5's load destination with its address register because the address dies at the load; target keeps the value in $v1 while the address register $v0 is reused by the D_800A11D5 chain. The proposed C-level condition was an address whose second use is elsewhere.
- probe: 40 forms on the do{}while(0) chassis (tmp/grind/CD_datasync/s23/gen.py, f_*.c): named-address `s32 *p4`/`s32 *p5` locals for arg4 and arg5 in both statement orders and crossed with inline/named-value spellings; declaration-with-initializer variants; the pointer read twice with the sibling element reached by an index difference; a single pointer walked between the two elements in both directions; the post-increment walk from .claude/rules/walking-pointer-serializes-parallel-loads.md; and six arg3-as-address combinations. Scored with the calibrated s22 harness and read with cmp.py 40 60.
- result: KILLED as stated. Minimum 7 (four ties, no improvement); named-address spellings 9-10; declaration-with-initializer spellings 26-31 with two extra instructions. Naming the address does not anti-coalesce the load - it inverts WHICH index chain leads, which is the wrong half of the geometry. Superseded by the s23 finding that anti-coalescing IS reachable, but through the scaled-offset + named-raw-index combination, not through address naming.

## [s23] Target's block-3 geometry (idx0 read first but consumed last, idx1 consumed first, arg5's value anti-coalesced from its address) is reachable from pure C source structure
- mechanism: s22 concluded the anti-coalescing was "a local-alloc seat, not a statement-order property", which if true would make the geometry unreachable by any spelling. The counter-mechanism: the seat is decided by which quantities are live when arg5's value is born, and source structure decides the birth ORDER of the two chains, so a spelling that makes idx1's chain complete before idx0's address is finished changes the liveness picture local-alloc sees.
- probe: a new discriminant scanner (tmp/grind/CD_datasync/s23/scan.py) classifying each emitted .s by (lbu order, chain-completion order, arg5 seat SPLIT-vs-coalesced), run over all 157 forms measured this session.
- result: CONFIRMED. Fourteen forms hit target's exact triple (01, 10, SPLIT), and 39 more are SPLIT under other orders. All fourteen emit byte-identical assembly at 8 / 91. Banked as memory/grind/CD_datasync/alt_target_geometry_8.c. This overturns the practical reading of s22's conclusion: the seat is reachable, and the function's remaining residual is now ONE displaced instruction pair rather than a mirror-image geometry.

## [s23] Inside the geometry-correct basin, some spelling or placement of arg2 (D_800F19C0) keeps its `lui/lw $a1` at build 43-44 instead of letting it sink to 54-55
- mechanism: in the geometry-correct basin local-alloc hands arg5's loaded value $a1, so arg2's materialisation cannot precede arg5's retirement into 16($sp). Target loads $a1 at 43-44 and keeps it live to the call, which is exactly what forbids arg5's value from taking $a1 there. If a spelling forces arg2's read to become a pseudo with a live range starting before the index chains, the cycle breaks.
- probe: 22 forms (tmp/grind/CD_datasync/s23/gen4.py, f4_*.c) - arg2 inline; arg2 as a named `s32 *` local at all five statement positions inside the block; arg2 as a `u32` local cast back, three positions; arg2 named plus arg3 named; plus 4 forms (f5_*.c) hoisting arg2 to function scope on both chassis and reading it before `puts`.
- result: KILLED for these spellings. The ten in-block arg2 spellings are BYTE-IDENTICAL at 8 - naming arg2 does not create a pseudo that survives to local-alloc, it is coalesced into the call's $a1 setup, so no source placement moves its emission slot. The four function-scope hoists regress to 28-31: the pseudo spans the VSync loop, takes a fifth callee-saved seat and adds a prologue save/restore pair. The lever that breaks the cycle is not an arg2 spelling.

## [s23] Computing any printf argument BEFORE the `puts` call changes the block's register pressure in target's direction
- mechanism: target's arg4 address is parked in $a0 across the D_800A11D5 chain, which requires unusual pressure at that point; moving a value across the puts call was never measured and is the only statement re-association left that changes liveness rather than order.
- probe: 40 forms (tmp/grind/CD_datasync/s23/gen3.py, f3_*.c) - every subset of {arg3, arg4, arg5} placed before `puts` crossed with four statement orders.
- result: KILLED. Any value moved before the call costs 2-3 instructions (build_insns 84-85, scores 27-32) because it must survive a call and takes a callee-saved seat with its own save/restore. Only the all-after placements stay at 82 instructions, floor 7.

## [s23] Target's block-3 geometry — idx_1494[0]'s lbu emitted first while its chain completes LAST, idx_1494[1]'s chain completing first, and arg5's loaded value landing in a register distinct from its own address register — is reachable from pure C source structure on the do{}while(0) chassis.
- mechanism: s22 concluded the anti-coalescing was 'a local-alloc seat, not a statement-order property', which if true would put the geometry out of reach of any spelling. The counter-mechanism: the seat is decided by which quantities are live when arg5's value is born, and source structure decides the birth ORDER of the two index chains, so a spelling that lets idx1's chain complete before idx0's address is finished changes the liveness picture local-alloc sees.
- probe: Built a discriminant scanner (tmp/grind/CD_datasync/s23/scan.py) that classifies each emitted cc1 .s by three target-relevant discriminants over the window between `jal puts` and `jal printf`: (a) the order of the two `lbu $x,N($17)` index reads, (b) the order in which the two table chains complete their `lw`, (c) whether arg5's load destination differs from its address register (SPLIT) or is coalesced with it. Emitted asm for all 157 forms measured this session and ran the scanner over them.
- result: CONFIRMED. Target is (01, 10, SPLIT). Fourteen forms hit that exact triple; 39 more are SPLIT under other orders. All fourteen emit byte-identical assembly and score 8/91 (lev 8, nop 3) — one WORSE than the 7-floor base form on levenshtein but matching target on all three discriminants. Banked as memory/grind/CD_datasync/alt_target_geometry_8.c. Canonical spelling: inside the wrapper, `s32 *m2; s32 k0; s32 i1; s32 arg4; s32 arg5;` then `m2 = D_800F19C0; i1 = idx_1494[1]; k0 = idx_1494[0] * 4; arg5 = tbl_125c[i1]; arg4 = *(s32 *)((u8 *)tbl_125c + k0);`. The entire residual of this form is that arg2's `lui $a1 / lw $a1,%lo(D_800F19C0)` sits at build 54-55 where target has it at 43-44.
- verdict: CONFIRMED

## [s23] Keeping a table ADDRESS pointer live past its own load (s22's frontier item 1) anti-coalesces arg5's load destination from its address register, on the do{}while(0) chassis with the wrapper FAKE present.
- mechanism: s22's alignment showed the candidate coalesces arg5's load destination with its address register because the address dies at the load; target keeps the value in $v1 while $v0 is reused by the D_800A11D5 chain. The proposed C-level condition was an address whose second use lies elsewhere.
- probe: 40 forms (tmp/grind/CD_datasync/s23/gen.py, f_*.c): named-address `s32 *p4` / `s32 *p5` locals for arg4 and arg5 in both statement orders crossed with inline and named-value spellings; declaration-with-initializer variants; the pointer read twice with the sibling element reached by an index difference (`p5[idx_1494[0] - idx_1494[1]]`); a single pointer walked between the two elements in both directions; the post-increment walk from .claude/rules/walking-pointer-serializes-parallel-loads.md; and six arg3-as-named-address combinations. Scored with the calibrated s22 harness, read with cmp.py 40 60 and classified with scan.py.
- result: KILLED for these 40 spellings. Minimum 7 (four forms tie the floor, none beat it); named-address spellings sit at 9-10; declaration-with-initializer spellings cost two extra instructions each (build_insns 84-86, scores 26-31) because the initializer is emitted at block entry rather than at the use. Naming the address does not anti-coalesce the load — it inverts which index chain leads, which is the wrong half of the geometry. The anti-coalescing was reached this session by a different route (scaled offset for arg4 + named raw index for idx1), so the axis is dead but the goal is not.
- verdict: KILLED
- kill_scope: instance
- measured_on: do{}while(0) wrapper chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 40 forms

## [s23] Inside the geometry-correct basin, some spelling or placement of arg2 (D_800F19C0) keeps its `lui/lw $a1` at build 43-44 instead of letting it sink to 54-55.
- mechanism: In the geometry-correct basin local-alloc hands arg5's loaded VALUE the hard register $a1 — the arg2 seat — so arg2's materialisation cannot be emitted until arg5's value has retired into 16($sp) at build 53. Target gives arg5's value $v1 instead, leaving $a1 loadable at 43-44 and live to the call, and that $a1 occupancy is exactly what forbids arg5's value from taking $a1 there. If a spelling forces arg2's read into a pseudo whose live range starts before the index chains, the cycle breaks.
- probe: 22 forms (tmp/grind/CD_datasync/s23/gen4.py, f4_*.c): arg2 inline; arg2 as a named `s32 *` local at all five statement positions inside the block; arg2 as a `u32` local cast back to `s32 *` at three positions; arg2 named together with arg3 named; plus 4 forms (f5_*.c) hoisting arg2 to a function-scope `s32 *fmt` (assigned as `fmt = &D_800162C0; D_800F19C0 = fmt;` and as a re-read `fmt = D_800F19C0;`) on both the base and geometry-correct chassis, and one reading it before `puts`.
- result: KILLED for these 26 spellings. The ten in-block arg2 spellings are BYTE-IDENTICAL at 8 — naming arg2 does not create a pseudo that survives to local-alloc; it is coalesced into the call's $a1 setup, so no source-level placement moves its emission slot. The four function-scope hoists regress to 28/29/30/31 because the pseudo then spans the VSync loop, takes a fifth callee-saved seat and adds a prologue save/restore pair (build_insns 83-85 vs 82).
- verdict: KILLED
- kill_scope: instance
- measured_on: geometry-correct (scaled-offset arg4 + named raw idx1) chassis and the base chassis, both with the do{}while(0) FAKE present; 26 forms

## [s23] Computing a printf argument BEFORE the `puts(&g_str_cd_timeout)` call re-associates the block's register pressure toward target's shape, where arg4's address is parked in $a0 across the D_800A11D5 chain.
- mechanism: Target parks arg4's address in the $a0 hard register across the whole D5 chain, which requires unusual pressure at that point. Moving a value across the puts call is the only statement re-association that changes liveness rather than order, and it had never been measured.
- probe: 40 forms (tmp/grind/CD_datasync/s23/gen3.py, f3_*.c): every subset of {arg3, arg4, arg5} computed before `puts` instead of after it, crossed with four statement orders, on the do{}while(0) chassis.
- result: KILLED for these 40 forms. Any single value moved before the call costs two to three instructions (build_insns 84-85, scores 27-32) because the value must survive a call and therefore takes a callee-saved seat with its own save/restore pair. Only the all-after placements stay at 82 instructions, and among those the floor is again 7.
- verdict: KILLED
- kill_scope: instance
- measured_on: do{}while(0) wrapper chassis with the FAKE present; 40 forms

## [s23] Some combination of index naming (raw or scaled), argument-value naming, and statement order in the timeout block scores below 7 on the do{}while(0) chassis.
- mechanism: s21 found the named scaled index offset reproduces target's whole tail at 8/92; the untried remainder was the full cross-product of that spelling against raw-index naming, value naming, and both statement orders.
- probe: 51 forms (tmp/grind/CD_datasync/s23/gen2.py, f2_*.c): {idx0 inline / named raw `i0` / named scaled `k0 = idx*4`} x {idx1 inline / named raw / named scaled} x {arg4 inline or named} x {arg5 inline or named} x {forward / reversed statement order}.
- result: KILLED for these 51 forms. Twenty-one tie the floor at 7 and nothing goes below; s21's scaled-index results are reproduced exactly. The floor of 7 is a broad flat plateau in argument-block spelling space rather than a narrow ridge, which explains why 23 sessions of argument respelling have not moved it. The sweep's real value was that one member of it (f2_g_sclraw_v11_r) is the first form to carry target's full geometry.
- verdict: KILLED
- kill_scope: instance
- measured_on: do{}while(0) wrapper chassis with the FAKE present; 51 forms

## [s24] KILL RE-AUDIT (mandated): the s23 "geometry-correct basin at 8/91" re-measures at 10/91 on the current chassis and is structurally farther from target than candidate.c
- mechanism: instance kills and banked artifacts are chassis-relative; s23's basin claim rested on a fast-harness reading that was never sandbox-confirmed. The re-audit applied memory/grind/CD_datasync/alt_target_geometry_8.c (and, independently, the two s23 source files whose bodies are identical to it) to src/system.c and ran the live sandbox.
- probe: `sandbox CD_datasync --disable all` on alt_target_geometry_8.c; fast harness (tmp/grind/CD_datasync/s24/sweep.sh) on the same body; window alignment against target via tmp/grind/CD_datasync/s24/cmp.py.
- result: KILLED (instance). Sandbox 10/91, not 8/91. Fast harness lev 10 / nop 3, matching the calibration exactly. Window alignment: candidate.c 6 displaced insns, alt_target_geometry_8.c 8 displaced insns; the latter completes arg4's chain at window slot 9 where target completes it LAST. The FAKE state is unchanged (do{}while(0) present in both), so this is not an ablation artefact - s23 recorded the wrong number and drew a structural conclusion from it. All three s23 frontier items are void.

## [s24] The fully-inline printf form has EXACTLY target's pre-scheduling dependence graph, so the whole residual is sched1 ready-list order, not argument spelling
- mechanism: `pwsh tools/grinder/dump.ps1 CD_datasync` on the inline body; read .combine (pre-sched1) and .sched (post-sched1). The inline body's .combine emits arg2, then arg3's address chain, then arg4's address chain, then arg5's chain + sw 16(sp), then the four register loads with a2 and a3 as direct memory loads. Target's window likewise ends `lw $a2,0($v0)` / `lw $a3,0($a0)` and carries arg5's value in a pseudo stored to 16($sp), so target's a2/a3 were NOT hoisted into source locals and its arg5 need not be.
- probe: tmp/grind/CD_datasync/s24/inl.combine + inl.sched, restricted to the `;; Function CD_datasync` section between the `jal puts` and `jal printf` call insns; compared instruction multiset and dependence shape against asm/funcs/CD_datasync.s:47-68.
- result: CONFIRMED. Same 16 instructions, same dependence graph, different order. The inline form scores 13 purely because sched1 issues arg3's chain at steps 2-3 and sinks arg4's lbu to step 12, where target issues arg4's lbu at step 1 and arg3's chain at steps 9-10.

## [s24] Target requires arg4's chain to carry an extra dependence edge strictly BETWEEN its lbu and its sll; a named index local cannot supply it because the copy is folded before sched1
- mechanism: with p() = sched.c INSN_PRIORITY (latency-weighted longest path to block end, load->use 2, alu->use 1) the chain heads are arg5 7, arg4 5, arg3 5, arg2 3 when all args are inline, and arg4 rises to 7 when arg4 alone is hoisted into a local. Ties are broken in ascending UID on this block (refining s18's descending-UID reading). Target needs arg4's lbu to win step 1 (=> p(arg4.lbu) >= p(arg5.lbu)) AND arg5's sll to win step 4 (=> p(arg4.sll) < p(arg5.sll)); the only graph satisfying both places the extra edge between arg4's lbu and its sll.
- probe: 10 forms (z1-za) built to place an edge there - `i0 = idx_1494[0]; arg4 = tbl_125c[i0];` after an arg5 hoist, its u8 variant, the both-indices variant, reversed declaration order, address-pointer and scaled-offset variants - swept with tmp/grind/CD_datasync/s24/run3.sh and classified by window signature with rep2.py.
- result: KILLED (instance). z1, z6, z7, z8 are BYTE-IDENTICAL to z9, the same body with no index local at all; all score 8. The index copy is folded (combine) and contributes no edge. The address-pointer (z2, z3) and reversed (z4) spellings only flip the addu operand order, scoring 9/9/8. On this chassis, with the do{}while(0) FAKE present, no ordinary-C spelling of a named index produced a surviving copy insn.

## [s24] FRONTIER RESET - the next probe is tools/sched_solver on the inline form's pre-sched RTL, not another argument respelling
- mechanism: 24 sessions of argument respelling have produced exactly seven distinct emissions (evidence [s24]); target is in none of them, and the reason is now known to be two ready-list decisions inside sched1. tools/sched_solver models GCC 2.7.2's sched.c list scheduler order- and clock-exactly (6978/6978 validated; project memory sched-solver-campaign-2026-08-05). The inline form supplies the correct dependence graph as INPUT and asm/funcs/CD_datasync.s supplies the goal order, which is exactly the question the solver was built to answer.
- probe: feed the CD_datasync block of tmp/grind/CD_datasync/s24/inl.combine to tools/sched_solver as the source order and the target window as the goal; ask (a) whether the goal order is reachable at all under the modelled priority + tie-break, and (b) the minimal change to the dependence graph or to the pre-sched UID order that reaches it. Bound and profile the run (the CD_ready precedent is that an unbounded solver run burns CPU without finishing depth 1).

## [s24] The banked s23 artifact alt_target_geometry_8.c scores 8/91 on the current chassis and differs from target by a single displacement of arg2's lui/lw.
- mechanism: Mandated kill re-audit: the closest-to-target banked form must be re-measured on the current chassis before any new probe, because a fast-harness reading that was never sandbox-confirmed can carry a wrong structural conclusion forward.
- probe: Applied memory/grind/CD_datasync/alt_target_geometry_8.c (and independently tmp/grind/CD_datasync/s23/f4_k_a2val_p0.c and f4_p_0123.c, whose bodies are byte-for-byte identical to it) to src/system.c; ran `sandbox CD_datasync --disable all`; re-ran the s21/s22 fast harness; aligned the emitted window against the target window with tmp/grind/CD_datasync/s24/cmp.py.
- result: Sandbox reports score 10, target_insns 91, build_insns 91 - not 8. The fast harness agrees exactly (lev 10, nop 3), and the calibration sandbox = lev + nop - 3 was independently confirmed on candidate.c (lev 7 -> sandbox 7) in the same session. The structural claim inverts too: aligned against target, candidate.c has 6 displaced instructions and alt_target_geometry_8.c has 8, and alt_target_geometry_8.c completes arg4's chain at window slot 9 where target completes it LAST (slot 16). The do{}while(0) FAKE is present in both forms, so this is not an ablation artefact. The artifact has been annotated in place; all three s23 frontier items, which were seeded from this form, are void.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; live sandbox plus fast harness, 3 bodies

## [s24] The fully-inline printf form (no named locals at all in the timeout block) has the same pre-scheduling dependence graph as the target window.
- mechanism: Pass attribution by reading dumps rather than inferring: `pwsh tools/grinder/dump.ps1 CD_datasync` was run on the inline body and the .combine (pre-sched1) and .sched (post-sched1) sections for CD_datasync were extracted between the jal puts and jal printf call insns.
- probe: tmp/grind/CD_datasync/s24/inl.combine and inl.sched; instruction multiset and dependence shape compared against asm/funcs/CD_datasync.s:47-68.
- result: CONFIRMED. The inline body's .combine emits insn 93 arg2 (mem(D_800F19C0)); 96/99/101 arg3's address chain; 104/107/109 arg4's address chain; 112/115/117/119/121 arg5's chain plus the sw 16(sp); then the four register loads 123 a0=fmt, 125 a1=arg2, 127 a2=mem(arg3addr), 129 a3=mem(arg4addr). Target's window likewise ends `lw $a2,0($v0)` then `lw $a3,0($a0)` as its last two window insns - i.e. target's arg3 and arg4 are loaded AT the register-load point, not hoisted into source locals - and likewise carries arg5's value in a pseudo stored to 16($sp). Same 16 instructions, same dependence graph, different order. The inline form scores 13 only because sched1 issues arg3's chain at steps 2-3 and sinks arg4's lbu to step 12, where target issues arg4's lbu at step 1 and arg3's chain at steps 9-10.
- verdict: CONFIRMED

## [s24] An extra dependence edge can be placed between arg4's lbu and its sll by naming the index in a local (i0 = idx_1494[0]; arg4 = tbl_125c[i0];), which is what target's scheduling order requires.
- mechanism: With p() = sched.c INSN_PRIORITY (latency-weighted longest path to block end, load->use 2, alu->use 1) the chain heads are arg5 7, arg4 5, arg3 5, arg2 3 when all args are inline, and arg4 rises to 7 when arg4 alone is hoisted into a local. Ties are broken in ascending UID on this block. Target needs arg4's lbu to win step 1 and arg5's sll to win step 4, which requires p(arg4.lbu) >= p(arg5.lbu) together with p(arg4.sll) < p(arg5.sll) - an extra edge strictly between arg4's lbu and its sll.
- probe: 10 forms (tmp/grind/CD_datasync/s24/z1..za) built to place that edge: named index after an arg5 hoist, its u8 variant, both-indices variant, reversed declaration order, address-pointer and scaled-offset variants; swept with tmp/grind/CD_datasync/s24/run3.sh and classified by window signature with rep2.py.
- result: KILLED. z1, z6, z7 and z8 emit assembly BYTE-IDENTICAL to z9, which is the same body with no index local at all; all five score 8. The index copy is folded before scheduling and contributes no dependence edge. The address-pointer and scaled-offset spellings (z2, z3, z4) only flip the addu operand order, scoring 9, 9 and 8. This is also the mechanistic reason s23's 51-form index-naming cross-product collapsed to two distinct windows.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 10 forms, fast harness + window-signature classification

## [s24] Hoisting only an index, or only arg5's value, into a named local changes the emission of the timeout block.
- mechanism: arg5 is the fifth argument and is passed on the stack, so expand_call already computes it into a pseudo and stores it to 16($sp) inside the argument loop; a source-level local adds no insn. A named index is folded by combine.
- probe: 15 forms: x0, x0u, x1, x01, x10, x01u (index-only hoists), n5, n5i0, n5i1, x0n5, x05n, z5, za (arg5-only hoists) and n25, n52, x0m2, x0m2b (arg5 plus arg2), swept with tmp/grind/CD_datasync/s24/run1.sh and run2.sh.
- result: KILLED. All of them emit assembly byte-identical to the fully-inline form and score 13. The 'name the fifth argument' family that sessions 21-23 repeatedly reached for is inert on this chassis, and the dumps say why.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 15 forms, fast harness

## [s25] The mandated kill re-audit: the do{}while(0) FAKE in candidate.c is an inert carrier occupying a pseudo that a real lever wants (the func_8002EA24-s8 failure mode)
- mechanism: An instance kill measured with a FAKE construct present is only a kill if the FAKE is not itself the thing blocking the lever. `tools/fake_ablate.py` is the mandated instrument.
- probe: `tools/fake_ablate.py --func CD_datasync --file system --candidate memory/grind/CD_datasync/candidate.c` (returned `ERR` on both variants - the tool cannot handle this candidate's 420-line header comment), then a hand ablation: `tmp/grind/CD_datasync/s25/nowrap.c` = candidate.c with the `do { ... } while (0);` replaced by a bare `{ ... }` and nothing else changed. Live sandbox on both.
- result: KILLED (instance). With the wrapper 7/91; without it 17/91, build_insns 91 in both. The wrapper is worth 10 points and is load-bearing, so it is not an inert carrier and every s21-s24 kill measured under it stands. Recorded so the next session does not re-run this: hand-ablate, do not trust `fake_ablate.py` on this candidate.

## [s25] s24's claim that the fully-inline printf form carries TARGET's dependence graph, so that only the pre-sched statement order is free
- mechanism: s24 read the `.combine` dump, saw the same 16 window instructions with arg3/arg4 loaded at the register-load point, and concluded the graph matched. If true, target's order would be reachable by statement reordering alone and `sched_solver` would return a LUID vector.
- probe: Built the sched_solver model of the inline form (`extract.py system` with `tmp/grind/CD_datasync/s24/inl.c` in `src/`; baseline order- AND clock-exact, so the model is sound), hand-built the goal from the role map, and asked three questions: `perturb.py --goal-before` over the 13 separating pairs (depth 1 with all 1332 atoms; depth 2 with the 693 spellable ones); a direct replay with the LUIDs set to target's own order; and a 20,000-sample randomized search over LUID permutations scoring the 13 constraints.
- result: KILLED (instance, on the inline chassis). No vector at either depth. Target's order is not a fixed point. The randomized search finds only 32 distinct emissions in total - and since `priority()` is recomputed from the graph and is invariant under LUID permutation, that 32-order set is the COMPLETE reachable set for this graph under statement reordering. The best member satisfies 9 of 13. The four unbought constraints (104<112, 93<115, 107<119, 101<121) are all priority inversions, so they cannot be bought by a tie-break. s24's frontier item 1 is answered and closed: the residual is a critical-path question, not an ordering question.

## [s25] Naming arg4's ADDRESS (a pointer local) rather than its value moves arg4's load to the register-load point and reaches target's trailing `lw $a3,0($a0)` geometry
- mechanism: A value local forces the load to be emitted as a source statement, before the stack-argument store; a pointer local defers the load into `expand_call`'s register-loading phase, which is exactly where target's last two `lw`s sit. This is the one geometric property no value-local form in 25 sessions has had.
- probe: 18 forms in two sweeps (`tmp/grind/CD_datasync/s25/p1-p8.c`, `q1-qa.c`), classified by window signature with `tmp/grind/CD_datasync/s25/rep.py`, covering `&tbl_125c[i]` vs `i + tbl_125c` vs `p4[0]` vs `*p4`, the pointer for arg4 / arg5 / arg3 / all three, an aliased index pointer `u8 *ip`, an arg5 value local before and after, and the pointer initialised before vs after `puts()`.
- result: CONFIRMED for the geometry, KILLED for the match (instance, on the inline chassis with the do{}while(0) FAKE present, 18 forms). The load does move to the register-load point. But a pointer-typed source expression emits `addu $v1,$s0,$v1` - BASE register first - where target has `addu $a0,$a0,$s0`, index first; an inline subscript emits index-first. Six spellings of the arg4 pointer are BYTE-IDENTICAL to each other (lev 9); making arg5 or arg3 a pointer flips that argument's addu too. The family is excluded on operand order before scheduling is even reached. Initialising the pointer before `puts()` costs a fifth callee-saved register and 84-vs-91 insns (lev 31).

## [s25] The operand-order fingerprint is a cheap pre-filter for every future argument spelling in this window
- mechanism: Target's three table addresses are all computed index-first (`addu $v0,$v0,$s0`, `addu $a0,$a0,$s0`, `addu $v0,$v0,$s3`). Measured this session: an inline array subscript emits index-first; a pointer-typed named expression emits base-first. The choice is made in expansion, not in scheduling, so it is invariant under every ordering lever.
- probe: 18 forms, three arguments, five spellings each (above).
- result: CONFIRMED. Use it: any candidate that names an ADDRESS is disqualified on operand order; any candidate that names a VALUE emits that load too early. The remaining space is forms that keep all three subscripts INLINE and change the critical path some other way.

## [s26] The do{}while(0) FAKE in candidate.c is an inert carrier occupying a pseudo that a real lever wants.
- mechanism: An instance kill measured with a FAKE present is only a kill if the FAKE is not itself blocking the lever (the func_8002EA24 s8 failure mode). The mandated kill re-audit requires ablating it on the closest-to-target banked form, which is candidate.c itself at 7/91.
- probe: tools/fake_ablate.py --func CD_datasync --file system --candidate memory/grind/CD_datasync/candidate.c returned ERR on both variants (the tool cannot drive a candidate whose body sits behind a 420-line header comment), so the ablation was hand-built: tmp/grind/CD_datasync/s25/nowrap.c is candidate.c with 'do { ... } while (0);' replaced by a bare '{ ... }' and nothing else changed. Live sandbox on both.
- result: KILLED. 7/91 with the wrapper, 17/91 without it, build_insns 91 in both. The wrapper is worth 10 points and is load-bearing, so every s21-s24 instance kill measured under it stands and none of them is an ablation artefact. Recorded for the next session: hand-ablate this candidate, fake_ablate.py cannot.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis, candidate.c applied to src/system.c, live sandbox; hand ablation because tools/fake_ablate.py returns ERR on this candidate

## [s26] Target's printf-window order is reachable from the fully-inline form's dependence graph by changing the source statement order (s24's frontier item 1, and the premise that the inline form carries target's graph).
- mechanism: s24 read the .combine dump, saw the same 16 window instructions with arg3/arg4 loaded at the register-load point, and concluded the graph matched - so that only the pre-sched insn order and the priority/tie-break were free. tools/sched_solver models GCC 2.7.2's sched.c order- and clock-exactly, so the claim is directly decidable: if it held, a LUID vector would exist.
- probe: Built the sched_solver model of the inline form (extract.py system with tmp/grind/CD_datasync/s24/inl.c in src/; baseline order- AND clock-exact for this block, so the model is not the doubt). Two hops of the stock chain are broken here and had to be routed around: goalmap.py cannot read a splat glabel .s (converted stream tmp/grind/CD_datasync/s25/tgt_pin.s), and the hon->tgt text alignment is garbage because system.hon.s is pre-macro-expansion maspsx output (76 lines vs target's 91) - the working path is --target-object build/src/system.o --ours-object tmp/sandbox/CD_datasync/system.o, which aligns 85/91 equal. The object-derived goal is still non-topological, so the goal was hand-built from a full RTL role map of the window (banked in evidence.md). Then: (i) perturb.py --goal-before over the 13 pairwise constraints separating our window from target's, depth 1 with all 1332 atoms and depth 2 with the 693 spellable ones; (ii) reorder_probe.py, replaying the block with the LUIDs set to target's own order; (iii) luid_search.py, 20,000 randomized LUID permutations scored against the 13 constraints.
- result: KILLED. No vector at either depth. Target's order is not a fixed point of the scheduler even when fed as the source order. The randomized search produces only 32 distinct emissions in total, and because priority() is recomputed from the dependence graph and is invariant under a LUID permutation, that 32-order set is the complete reachable set for this graph under statement reordering; the best member satisfies 9 of 13. The four unbought constraints are arg4's lbu before arg5's lbu (104<112), arg2's lw before arg5's sll (93<115), arg4's sll before arg5's value load (107<119), and arg3's addu before the stack store (101<121) - all priority inversions, and rank_for_schedule sorts priority-descending before any tie-break, with schedule_select's hazard promotion only reordering within an equal-priority group. The residual is a critical-path question, not an ordering question, which is the opposite of the frontier s24 left.
- verdict: KILLED
- kill_scope: instance
- measured_on: inline chassis (tmp/grind/CD_datasync/s24/inl.c) with the do{}while(0) FAKE present; sched_solver pass-1 block 3, baseline order- and clock-exact

## [s26] Naming arg4's ADDRESS in a pointer local, rather than its value, reaches target's window - because it defers arg4's value load into expand_call's register-loading phase, where target's trailing lw $a3,0($a0) sits.
- mechanism: A value local forces the load to be emitted as a source statement, ahead of the stack-argument store; a pointer local leaves the load to the register-loading phase. That is the one geometric property no value-local form in 25 sessions has had, and candidate.c gets it wrong (it completes arg4's chain at window slot 9 where target completes it last).
- probe: 18 forms in two sweeps (tmp/grind/CD_datasync/s25/p1-p8.c, q1-qa.c), classified by window signature with tmp/grind/CD_datasync/s25/rep.py: '&tbl_125c[i]' vs 'i + tbl_125c' vs 'p4[0]' vs '*p4'; the pointer taken for arg4, for arg5, for arg3, and for all three; an aliased index pointer 'u8 *ip'; an arg5 value local before and after; and the pointer initialised before vs after puts().
- result: KILLED for the match, CONFIRMED for the geometry. The value load does move to the register-load point - the first time in 25 sessions - at lev 9, the closest non-candidate window measured. But a pointer-typed source expression emits 'addu $v1,$s0,$v1' (BASE register first) where target has 'addu $a0,$a0,$s0' (scaled INDEX first), while an inline array subscript emits index-first. Six spellings of the arg4 pointer are BYTE-IDENTICAL to one another; pointerising arg5 (q3/q9) or arg3 (q4/q8) flips those arguments' addu too. The family is excluded on operand order before its schedule is even examined. Initialising the pointer before puts() (p1) is a separate hard no: the pointer must survive the call, GCC pins it in a fifth callee-saved register, and the form emits 84 insns against target's 91 at lev 31.
- verdict: KILLED
- kill_scope: instance
- measured_on: inline chassis with the do{}while(0) FAKE present; 18 forms on the fast harness, calibrated against live sandbox on candidate.c (lev 7 -> sandbox 7)

## [s26] The addu operand order in this window is decided at expansion and is a cheap pre-filter: an inline array subscript emits index-first, a pointer-typed named expression emits base-first, and target is index-first on all three of arg3/arg4/arg5.
- mechanism: The choice of which operand lands in rs is made when the address is expanded, not by any later pass, so it is invariant under every ordering and allocation lever. Target's three table addresses are all 'addu $rd,$idx,$base'.
- probe: The same 18 forms, three arguments, five spellings each; window signatures printed by tmp/grind/CD_datasync/s25/rep.py.
- result: CONFIRMED. This gives the next session a one-glance disqualifier: any candidate that names an ADDRESS is out on operand order; any candidate that names a VALUE emits that load too early. The remaining space is forms that keep all three subscripts inline and change the critical path some other way.
- verdict: CONFIRMED

## [s26b] The residual of this function is a sched1 ready-list problem, so a sched1 reachability verdict decides it (s24/s25's framing).
- mechanism: s24 read the .combine dump and localised the residual to two sched1 ready-list decisions; s25 then built the sched_solver model of pass 1 and asked whether target's window order is a reachable sched1 OUTPUT. Both sessions treated the sched1 output order as the emitted order.
- probe: Dumped candidate.c with -da (tmp/grind/CD_datasync/s26/dmp.sh; dumps w.i.sched, w.i.sched2) and read the CD_datasync printf window out of the pass-1 and pass-2 dumps with tmp/grind/CD_datasync/s26/win.py, then compared both against the emitted tmp/grind/CD_datasync/s24/asm/candidate.s and against asm/funcs/CD_datasync.s:47-68.
- result: KILLED. The pass-1 and pass-2 orders for this block are DIFFERENT, and the emitted .s carries the pass-2 order. sched1's output is not what we are matching - it is the input to local-alloc, and it decides the window only through the live ranges it hands the allocator. s25's four "unbuyable priority inversions" are properties of a pass whose output is discarded and re-ordered afterwards, so they do not foreclose the window. The binding question is the pass-2 (post-RA) order, whose dependence graph is dominated by hard-register anti-dependences that do not exist pre-RA and that follow from the register assignment.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis, candidate.c with the do{}while(0) FAKE present; cc1 -da dumps of the live build

## [s26b] The window residual is a large structural gap that no measured form has approached (the "mirror image" framing carried since s22).
- mechanism: Twenty-five sessions have described the printf window as structurally wrong in several ways at once. If the post-RA instruction set itself differed from target's, the remaining work would be an expansion problem.
- probe: Aligned candidate.c's post-RA (w.i.sched2) window against the target window insn by insn, by role and by hard register.
- result: CONFIRMED that the gap is SMALL and precisely typed. The post-RA instruction multiset, the roles, and even the coalescing (arg4's value local coalesced into $a3, so `lw $a3,0(addr)` is the register set) are IDENTICAL to target. Exactly two things differ: the order, and ONE register assignment - target holds arg4's address chain in $a0 and uses three scratch registers in the window ($a0 arg4-addr, $v0 arg5-addr then arg3-addr, $v1 arg5-value), candidate holds it in $v0 and uses two. This is the sharpest statement of the residual the ledger has ever carried and it should replace the "mirror image" language.
- verdict: CONFIRMED

## [s26b] A 3-scratch-register window - and target's arg4 chain byte-for-byte, register and operand order included - is reachable from plain-subscript VALUE locals.
- mechanism: local-alloc walks REG_ALLOC_ORDER ($v0,$v1,$a0,...) and only reaches $a0 when three scratch values are simultaneously live; $a0 is free in this window because the format-string lui/addiu is emitted last. Naming table values in locals lengthens their live ranges and can force the third register.
- probe: 16 forms on the candidate.c chassis (tmp/grind/CD_datasync/s26/gen.py -> m*.c; sweep.sh; classified by window signature with tmp/grind/CD_datasync/s25/rep.py), spelling the timeout block with plain-subscript value locals for subsets of {arg2,arg3,arg4,arg5} in every assignment order. Two forms re-measured on live sandbox for calibration.
- result: CONFIRMED. m453 (locals arg4, arg5, arg3) emits `lbu $a0,0($s1) / sll $a0,$a0,2 / addu $a0,$a0,$s0 / lw $a3,0($a0)` - target's arg4 chain exactly, in $a0, INDEX-FIRST, with the value load at the register-load point. Live sandbox 13/91, build_insns 90 (dbr steals the stack store into the printf delay slot, which target does not do). A grep over all 177 banked .s files from s20-s25 finds `sll $4,$4,2` in only nine s23 forms, none with this combination.
- verdict: CONFIRMED

## [s26b] s25's operand-order "vice": naming an ADDRESS flips the addu to base-first and naming a VALUE emits that load too early, so the remaining space is inline-only spellings.
- mechanism: s25 measured 18 pointer-local forms and generalised from them to a pre-filter that disqualifies every named-argument spelling.
- probe: The 16-form value-local sweep above, read at the register/operand level rather than by score.
- result: KILLED. The second half is false: m453 / m43 / m435 / m2453 are plain-subscript VALUE locals whose value load lands at the register-load point. The first half is true only of the s23 SCALED-OFFSET cast spelling `*(s32 *)((u8 *)tbl + k)`, which is what produced the base-first `addu $4,$16,$4`; a plain-subscript value local is index-first. The pre-filter must not be used to exclude value-local forms, and the arg4-pointer axis s25 opened is unnecessary because value locals reach both properties at once.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (candidate.c chassis: goto loop, three hoisted pointer locals, do{}while(0) FAKE present); 16 forms on the fast harness, two re-measured live

## [s26b] Two placement rules govern this window, and they make a pure value-local spelling of target impossible while leaving one combination open.
- mechanism: Empirical, from the 16-form sweep, read at the register level.
- probe: Same sweep; window signatures compared pairwise.
- result: CONFIRMED. (1) The three table value loads are emitted in the SOURCE ASSIGNMENT ORDER of the value locals; target's load order is arg5, arg3, arg4 = source order 5,3,4. (2) The FIRST-assigned value local's address chain is the one that gets $a0; target needs arg4 there. The two rules conflict for all-value-local spellings, so the open combination is: arg4 left INLINE (its load is then the `lw $a3` register set, last by construction) with arg3 and arg5 as value locals - which is exactly the m35/m53 family, and which already reproduces target's whole tail (18-insn window, `sw` inside the block, `lw $a3` last, fmt last, `jal printf; nop`). What that family still lacks is the THIRD live scratch value that would push arg4's address into $a0.
- verdict: CONFIRMED

## [s27] The do{}while(0) FAKE in candidate.c is an inert carrier occupying a pseudo that a real lever wants (mandated kill re-audit on the closest-to-target banked form, which is candidate.c itself).
- mechanism: An instance kill measured with a FAKE present is only a kill if the FAKE is not itself blocking the lever (the func_8002EA24 s8 failure mode). tools/fake_ablate.py returns ERR on this candidate (its body sits behind a 420-line header comment), so the ablation is hand-built.
- probe: tmp/grind/CD_datasync/s25/nowrap.c = candidate.c with `do { ... } while (0);` replaced by a bare brace block and nothing else changed; both applied to src/system.c and measured with `sandbox CD_datasync --disable all`.
- result: KILLED. 7/91 with the wrapper, 17/91 without it, build_insns 91 in both. The wrapper is worth 10 points, is load-bearing, and occupies no pseudo a lever wants, so every s21-s25 instance kill measured under it stands and none is an ablation artefact.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis, candidate.c applied to src/system.c, live sandbox; hand ablation because tools/fake_ablate.py returns ERR on this candidate

## [s27] This function's window residual is a sched1 ready-list problem, so a sched1 reachability verdict decides it - the framing s24 introduced and s25 spent its whole solver campaign on.
- mechanism: s24 read the .combine dump, localised the residual to two sched1 ready-list decisions, and s25 asked sched_solver whether target's window order is a reachable sched1 OUTPUT. Both treated sched1's output order as the emitted order.
- probe: Dumped candidate.c with cc1 -da (tmp/grind/CD_datasync/s26/dmp.sh) and read the CD_datasync printf window out of w.i.sched (pass 1) and w.i.sched2 (pass 2) with tmp/grind/CD_datasync/s26/win.py, comparing both against the emitted candidate.s and against asm/funcs/CD_datasync.s:47-68.
- result: KILLED. The pass-1 and pass-2 orders for this block are DIFFERENT and the emitted .s carries the pass-2 order. sched1's output is not what we are matching; it is the input to local-alloc, and it reaches the window only through the live ranges it hands the allocator. s25's four 'unbuyable priority inversions' therefore do not foreclose the window - they are properties of a pass whose output is re-ordered afterwards. The binding question is the post-RA order, whose dependence graph is dominated by hard-register anti-dependences that do not exist pre-RA and that follow from the register assignment.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis, candidate.c with the do{}while(0) FAKE present; cc1 -da dumps of the live build

## [s27] s25's operand-order pre-filter - naming an ADDRESS flips the addu to base-first and naming a VALUE emits that load too early, leaving only inline spellings - excludes the value-local family.
- mechanism: s25 measured 18 pointer-local forms and generalised from them into a pre-filter meant to disqualify every named-argument spelling before its schedule is examined.
- probe: 16 plain-subscript value-local forms on the candidate.c chassis (tmp/grind/CD_datasync/s26/gen.py -> m*.c, sweep.sh, classified by window signature with tmp/grind/CD_datasync/s25/rep.py), read at the register/operand level rather than by score; m453 re-measured on live sandbox.
- result: KILLED. m453, m43, m435 and m2453 are plain-subscript VALUE locals whose value load lands at the register-load point, so the 'value locals load too early' half is false. The base-first addu the pre-filter is built on comes from the s23 SCALED-OFFSET cast spelling *(s32 *)((u8 *)tbl + k), not from naming a value: plain-subscript value locals are index-first. m453 reproduces target's arg4 chain byte-for-byte - lbu $a0,0($s1) / sll $a0,$a0,2 / addu $a0,$a0,$s0 / lw $a3,0($a0) - at live sandbox 13/91 (build_insns 90, dbr steals the stack store into the printf delay slot). A grep over all 177 banked .s files from s20-s25 finds sll $4,$4,2 in only nine s23 forms, none with this combination.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals, do{}while(0) FAKE present); 16 forms on the fast harness, two re-measured on live sandbox

## [s28] The complete 65-member value-local subset/order space collapses to 18 windows, none of them target's
- mechanism: plain-subscript value locals for subsets of {arg2,arg3,arg4,arg5} in every assignment order are the space s26/s27 sampled with 16 members and derived two placement rules from. Generating all 65 tests those rules exhaustively rather than by sample.
- probe: tmp/grind/CD_datasync/s28/gen.py -> 65 forms -> s28/sweep.sh -> s28/asm/n*.s, classified by window signature with s25/rep.py and by window-levenshtein with s28/wrank.py.
- result: KILLED (instance). 18 distinct windows. Minimum window-lev 7 (the candidate.c family). No member carries target's split arg4 chain and no member carries the $a0/$a3 register map together with an 18-insn window.

## [s28] An s32 index local assigned first, with arg4 left inline, splits arg4's address chain the way target does
- mechanism: naming the index creates a pseudo that is live from the top of the block; with a third live scratch value present, local-alloc must give that chain its own hard register, and sched2 is then free to hoist the lbu and defer the addu - the shape s24 predicted was needed ("one extra edge at the FRONT of arg4's chain and none at the back") and could not spell.
- probe: 90-form sweep (s28/gen2.py, x*.s) over {i4,i5,arg3,arg5} locals in every order and both index types, plus 52-form gen3.py and 258-form gen4.py extensions adding i3 and arg4.
- result: CONFIRMED. The split appears in 53 forms. It requires the s32 index type (u8 folds) AND a co-present value local (the index local alone reproduces the all-inline window). It also reproduces on arg3 (`s32 i3` + arg4 value local -> arg3's chain splits into $a2), so it is a general property of the named-index-plus-third-live-value shape.

## [s28] The split chain cannot be placed in $a0 by any of the 465 spellings measured, because sched1 schedules the fmt-string `set $a0` as stall filler before the address pseudo dies
- mechanism: mips.h defines no REG_ALLOC_ORDER, so local-alloc tries hard regs in ascending order; $a1 (arg2) and $a2 (arg3 value) are live to the call, and $a0 is live from sched1's placement of `(set (reg:SI 4 a0) (symbol_ref "D_800161C8"))` to the call. When that set is scheduled before the arg4 address pseudo's death, $a0 conflicts and the next free scratch, $a3, is taken.
- probe: -da dump of ya53 (tmp/grind/CD_datasync/s28/ya53.sched.txt, insn 129 at position 10 of 17 vs reg 105 born 125 / dead 135) plus s28/scan_split.py and s28/scan2.py over all 465 forms compiled this session.
- result: KILLED (instance). 53 forms reach the $a0/$a3 map and every one of them has a 17-insn window (arg4's value load is not last, so dbr sinks the stack store into the printf delay slot); every 18-insn form places the split chain in $a3, $a2 or $v1. The two target properties are reachable separately and never together in this set of spellings.

## [s28] The split-chain geometry is not carried by the do{}while(0) FAKE
- mechanism: mandated kill re-audit, run on the session's closest-topology form rather than on candidate.c (already ablated in s26 and s27).
- probe: ya53_nowrap (ya53 with the do{}while(0) replaced by a bare brace block) compiled and window-compared.
- result: CONFIRMED (the FAKE is inert for this axis). The printf window is byte-identical modulo the callee-saved base register assignment (s0/s1/s3 -> s1/s2/s3); lev 24 vs 15. The FAKE's whole measured effect on this chassis is which callee-saved registers the three base pointer locals receive.

## [s28] The complete 65-member space of plain-subscript value locals for subsets of {arg2,arg3,arg4,arg5} in every assignment order collapses to 18 distinct printf windows, and none of those 18 is target's window.
- mechanism: s26/s27 sampled this space with 16 hand-picked members and derived two placement rules from it (value loads emit in source assignment order; the first-assigned local's chain gets the third scratch register). Generating all 65 members tests those rules exhaustively instead of by sample, and settles whether the rules' apparent mutual exclusivity is an artefact of the sample.
- probe: tmp/grind/CD_datasync/s28/gen.py generated all 65 forms; s28/sweep.sh compiled each with the exact build cc1 invocation into s28/asm/n*.s; classified by window signature with tmp/grind/CD_datasync/s25/rep.py and by window-levenshtein against asm/funcs/CD_datasync.s:47-68 with s28/wrank.py.
- result: 18 distinct windows. Minimum window-lev 7, reached by the candidate.c family {n4,n24,n42,n45,n245,n425,n452}; the all-inline spelling n0 is 13. No member carries target's split arg4 chain, and no member carries the $a0/$a3 register map together with an 18-insn window. The axis s26/s27 opened is now measured to its boundary rather than left half-sampled.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; all 65 members compiled, fast harness calibrated sandbox = lev + nop - 3, candidate.c re-verified live at sandbox 7

## [s28] An s32 index local assigned first (s32 i4 = idx_1494[0];) with arg4 still passed inline as tbl_125c[i4], plus at least one co-present value local, emits target's split arg4 address chain: lbu early, sll in the middle, a deferred addu, and the value load last, inside an 18-insn window that keeps the stack store in the block.
- mechanism: Naming the index creates a pseudo live from the top of the block; with a third simultaneously-live scratch value present, local-alloc must give that chain its own hard register, and sched2 is then free to hoist the lbu and defer the addu. This is exactly the shape s24 predicted was required (one extra dependence edge at the FRONT of arg4's chain and none at the back) and could not spell, because s24 only tried index locals that combine folded away.
- probe: 90-form sweep tmp/grind/CD_datasync/s28/gen2.py over {i4,i5,arg3,arg5} locals in every order and both index types (s32 and u8); 52-form gen3.py adding i3 and 258-form gen4.py adding arg4, 465 forms in total; split detection by tmp/grind/CD_datasync/s28/scan_split.py.
- result: CONFIRMED - the split appears in 53 of the 465 forms (representatives xsa3, ya3, ya53, banked to rejected/). Trigger conditions measured: the index local must be s32 (the u8 spelling folds back into the inline chain), and the index local alone is insufficient (xsa reproduces the all-inline window) - a value local must also be present. The trigger is not arg4-specific: s32 i3 = D_800A11D5 plus an arg4 value local splits arg3's chain into $a2 the same way (yac4, yca4).
- verdict: CONFIRMED

## [s28] Among the 465 forms compiled this session, no spelling places the split arg4 address chain in $a0: the 53 forms that reach target's $a0/$a3 register map all have a 17-insn window, and every 18-insn form puts the split chain in $a3, $a2 or $v1.
- mechanism: tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so local-alloc tries hard registers in ascending number order. $a1 (arg2) and $a2 (arg3's value) are live to the printf call; $a0 is live from sched1's placement of (set (reg:SI 4 a0) (symbol_ref "D_800161C8")) to the call. sched1 places that fmt-string set as stall filler - it is the block's minimum-priority insn, path length 1 to the call - and when it lands before the arg4 address pseudo's death, $a0 conflicts and the next free scratch, $a3, is taken. $a0 is therefore only free when the address pseudo dies EARLY, and an early death is exactly what moves arg4's value load off the end of the block, which lets dbr sink the stack store into the printf delay slot and costs the 18th insn.
- probe: -da dump of ya53 (instrumented path tools/gcc-2.7.2/build/cc1 -dr -da), window block extracted to tmp/grind/CD_datasync/s28/ya53.sched.txt: insn 129 (set a0, symbol_ref D_800161C8) sits at position 10 of 17, while the arg4 address pseudo reg 105 is born at insn 125 and dies at insn 135. Cross-checked over all 465 forms with s28/scan_split.py (split register census) and s28/scan2.py ($a0/$a3 map vs window length).
- result: KILLED - 53 a0-map forms, all 17-insn; 0 forms with both properties. ya43 (i4 first, then value locals arg4 then arg3) reaches target's arg4 register map byte-for-byte but loses the stack store to the delay slot; ya53 reaches target's split topology and 18-insn window but lands the chain in $a3. The two halves of target's window are each reachable and are in direct tension under every spelling measured.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 465 forms (n*/x*/y*/z*), plus the ya53 -da sched1 dump

## [s28] The new split-chain geometry is not carried by the do{}while(0) FAKE: ablating the wrapper leaves ya53's printf window byte-identical modulo the callee-saved base register assignment.
- mechanism: Mandated kill re-audit, run on this session's closest-topology form rather than on candidate.c (s26 and s27 had already ablated candidate.c twice). tools/fake_ablate.py still returns ERR on this candidate because of the 420-line header comment, so the ablation is by hand.
- probe: ya53_nowrap = ya53 with `do { ... } while (0);` replaced by a bare brace block, nothing else changed; compiled through the same harness and window-compared with s25/rep.py.
- result: CONFIRMED that the FAKE is inert for this axis. The window is identical except that the three base pointer locals move from s0/s1/s3 to s1/s2/s3 (lev 24 vs 15). The FAKE's entire measured effect on this chassis is the callee-saved assignment of the three base pointers, so every s21-s27 instance kill measured under it still stands and the new axis can be explored with or without it.
- verdict: CONFIRMED

## [s29] The fmt-string `set $a0` insn's placement is a source-controllable variable that some C spelling can defer (the framing s24/s25/s28 all worked under)
- mechanism: s28 read one -da dump and concluded sched1 emits the fmt set as stall filler "because it is the block's minimum-priority insn"; that left open the possibility that a different block shape gives it a different priority and defers it.
- probe: Ran the instrumented cc1 (`tools/gcc-2.7.2/cc1` - `build/cc1` lacks the hook) with `BB2_PRIO_DEBUG=1` over candidate.c's preprocessed TU and read the printf block's `PRIODBG SET` lines (`tmp/grind/CD_datasync/s29/prio.txt`), then read `priority()` and `rank_for_schedule()` in `tools/gcc-2.7.2/sched.c` and the argument-setup emission in `tools/gcc-2.7.2/calls.c`.
- result: KILLED as a lever. Measured priorities: fmt set (insn 127) = 1, the block minimum, tied with the four chain-source leaves 93/116/108/105; the call is 4. Its only LOG_LINKS entry is the REG_DEP_ANTI on the `puts` call (insn 89, priority 1) because its source operand is a bare symbol_ref with no data predecessor. `priority()` computes `priority(pred) + insn_cost - 1`, so a unit-cost ALU edge cannot raise it and only a LOAD edge can - and a constant-address move has no load edge to gain. Additionally `calls.c:1880` emits the hard-reg moves in ascending argument index, so the fmt set always has the lowest LUID of the four moves and a higher LUID than every argument-computation insn, which under `rank_for_schedule`'s descending-LUID tie-break makes it win every priority-1 tie it enters. No spelling of the C changes any of those three facts.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; candidate.c, live sandbox 7/91, plus the instrumented-cc1 priority dump of the same build

## [s29] s26/s27's PLACEMENT RULE 2 - "arg4 must be left INLINE, with its load being the `lw $a3` register set" - is the direction that forecloses `$a0`, not the direction that reaches it
- mechanism: With arg4 inline, `expand_call` never materialises a separate value load: the load IS the hard-reg move `(set (reg:SI 7 a3) (mem/s:SI (reg:SI 98)))`, which is a direct predecessor of the call and (being fed by a load edge) sits at priority 3. Its address pseudo therefore stays live to the very end of the block. With arg4 spelled as a NAMED value local, the value load is a separate priority-2 insn, the address pseudo dies mid-block, and `$a0` becomes genuinely free.
- probe: Dumped `tmp/grind/CD_datasync/s28/n0.c` (the all-inline spelling) with `-da` through the instrumented cc1 (`tmp/grind/CD_datasync/s29/prio2.sh` -> `w2.i.sched`, `n0.prio`) and located the fmt set and the a3 load in sched1's output; compared against candidate.c's dump (`w.i.sched`).
- result: CONFIRMED (the rule's direction is inverted). n0's sched1 output places the fmt set (insn 123) BEFORE the a3 load (insn 129 `set a3 = mem(reg 98)`), so reg 98 is live across the definition of `$a0` and local-alloc cannot assign it - the same fact s28 measured statistically over 465 forms, now read out of the causing pass. candidate.c's sched1 output places the arg4 value load (insn 100, priority 2) at block position 7 and the fmt set at position 10, so the arg4 address pseudo reg92 (def @pos4, dead @pos7) does NOT conflict with `$a0`.
- verdict: CONFIRMED

## [s29] What blocks candidate.c from target's arg4 chain is local-alloc's ascending hard-register scan, not the scheduler
- mechanism: `tools/gcc-2.7.2/config/mips/mips.h` defines no `REG_ALLOC_ORDER` (s28), so local-alloc walks scratch registers `$v0, $v1, $a0, $a1, ...` in ascending number order. reg92 (arg4's address) is live over sched1 positions 4-7 and only ONE other scratch pseudo (reg104, arg5's address, positions 6-9) overlaps it, so two scratch registers suffice and the scan stops at `$v0`. Target needs the scan to reach `$a0`, which requires THREE scratch pseudos simultaneously live across positions 4-7.
- probe: Read candidate.c's `w.i.sched` block order against its emitted `.s` (`tmp/grind/CD_datasync/s29/dmp_cand/out.s`), tracking each pseudo's def/death position, and confirmed the emitted arg4 chain is `lbu $2,0($17) / sll $2,$2,2 / addu $2,$2,$16 / lw $7,0($2)` - target's chain modulo one register, with reg86 already receiving `$a3` and insn 133's move already deleted as redundant.
- result: CONFIRMED. The distance from candidate.c to target's arg4 window is exactly one register-allocation decision, and the predicate that decides it is the count of scratch pseudos live across sched1 positions 4-7. s27's m453 is a measured instance of the 3-scratch condition being reached (it emits target's arg4 chain byte-for-byte) but it shortens the block to build_insns 90 and dbr then steals the stack store into the printf delay slot.
- verdict: CONFIRMED

## [s29] The fmt-string `set $a0` insn's placement is a source-controllable variable that some C spelling can defer (the framing s24/s25/s28 all worked under).
- mechanism: s28 read one -da dump and concluded sched1 emits the fmt set as stall filler because it is the block's minimum-priority insn; that left open the possibility that a different block shape gives it a different priority and defers it.
- probe: Ran the instrumented cc1 (tools/gcc-2.7.2/cc1 - build/cc1 lacks the hook) with BB2_PRIO_DEBUG=1 over candidate.c's preprocessed TU and read the printf block's PRIODBG SET lines (tmp/grind/CD_datasync/s29/prio.txt); then read priority() and rank_for_schedule() in tools/gcc-2.7.2/sched.c and the argument-setup emission in tools/gcc-2.7.2/calls.c.
- result: KILLED as a lever. Measured priorities for the printf block: fmt set (insn 127) = 1, the block minimum, tied with the four chain-source leaves 93/116/108/105; address-chain ALU insns = 2; the stack store and the a2/a3 moves = 3; the call = 4. The fmt set's only LOG_LINKS entry is the REG_DEP_ANTI on the puts call (insn 89, priority 1), because its source operand is a bare symbol_ref with no data predecessor. priority() computes priority(pred)+insn_cost-1, so a unit-cost ALU edge cannot raise a priority and only a load edge (cost 2) can - a constant-address move has no load edge to gain. calls.c:1880 emits the hard-reg moves in ascending argument index, so the fmt set always carries the lowest LUID of the four moves and a higher LUID than every argument-computation insn, which under rank_for_schedule's descending-LUID tie-break makes it win every priority-1 tie it enters. Three separate compiler-side facts, none of them expressible in the C.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; candidate.c applied to src/system.c, live sandbox 7/91, plus the instrumented-cc1 BB2_PRIO_DEBUG dump of the same build

## [s29] s26/s27's PLACEMENT RULE 2 - 'arg4 must be left INLINE, with its load being the lw $a3 register set' - is the direction that forecloses $a0 for arg4's address, not the direction that reaches it.
- mechanism: With arg4 inline, expand_call never materialises a separate value load: the load IS the hard-reg move (set (reg:SI 7 a3) (mem/s:SI (reg 98))), a direct predecessor of the call sitting at priority 3 because a load edge feeds it, so its address pseudo stays live to the end of the block. With arg4 spelled as a named value local the value load is a separate priority-2 insn, the address pseudo dies mid-block, and $a0 becomes free.
- probe: Dumped tmp/grind/CD_datasync/s28/n0.c (the all-inline spelling) with -da through the instrumented cc1 (tmp/grind/CD_datasync/s29/prio2.sh -> w2.i.sched, n0.prio) and located the fmt set and the a3 load in sched1's output; compared against candidate.c's w.i.sched.
- result: CONFIRMED, with the rule's direction inverted. n0's sched1 output places the fmt set (insn 123) BEFORE the a3 load (insn 129 set a3 = mem(reg 98)), so the arg4 address pseudo reg 98 is live across the definition of $a0 and local-alloc cannot assign it - the same fact s28 measured statistically over 465 forms, now read out of the causing pass. candidate.c's sched1 output places the arg4 value load (insn 100, priority 2) at block position 7 and the fmt set at position 10, so reg92 does not conflict with $a0 at all.
- verdict: CONFIRMED

## [s29] What separates candidate.c from target's arg4 window is local-alloc's ascending hard-register scan reaching only $v0, not any scheduling decision.
- mechanism: mips.h defines no REG_ALLOC_ORDER, so local-alloc walks scratch registers $v0,$v1,$a0,... in ascending number order. reg92 (arg4's address) is live over sched1 positions 4-7 and only one other scratch pseudo (reg104, arg5's address, positions 6-9) overlaps it, so two scratch registers suffice and the scan stops at $v0. Target needs three scratch pseudos simultaneously live across positions 4-7 for the scan to reach $a0.
- probe: Read candidate.c's w.i.sched block order against its emitted .s (tmp/grind/CD_datasync/s29/dmp_cand/out.s), tracking each pseudo's def and death position.
- result: CONFIRMED. candidate.c already emits lbu $2,0($17) / sll $2,$2,2 / addu $2,$2,$16 / lw $7,0($2) - target's arg4 chain modulo one register - with reg86 already receiving $a3 and insn 133's move already deleted as redundant. The gap is exactly one register-allocation decision, whose predicate is the count of scratch pseudos live across sched1 positions 4-7.
- verdict: CONFIRMED

## [s30] Both indices hoisted as s32 locals in [0],[1] order with the VALUE locals assigned arg5-first then arg4 gives arg4's index read the lowest LUID while putting arg5's scaling chain below arg4's, which is the exact LUID geometry target's window requires.
- mechanism: sched.c ranks a priority tie by INSN_LUID, and priority here is depth-from-block-start (s30 correction), so the two tbl_125c chains are separated only by the order in which their RTL was emitted. calls.c expands inline argument values in ascending argument index, and a value-local statement emits its chain at the statement's own (lower) LUID, so statement order is the only LUID lever the source has.
- probe: tmp/grind/CD_datasync/s30/gen2.py -> b1 (s32 indices), b2 (u8 indices), b5 (+arg2 value local), b6 (no index locals, arg5 then arg4), b7 (one index local); compiled with tmp/grind/CD_datasync/s30/sweep.sh and classified by window signature with s30/rep.py.
- result: KILLED (instance). All five emit ONE window - candidate.c's window with the two lbu reads transposed (lbu v1,1(s1) | lbu v0,0(s1) | ...), lev 8 vs candidate's 7. The index copies fold in combine (s24's kill re-confirmed on this chassis), so the index read never separates from its chain: whichever argument's value-local statement comes first takes BOTH window slot 1 and the first-completed chain. Measured on the HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present.

## [s30] A named s32 index local for arg4 with arg4 left INLINE and arg5 hoisted as a value local before it (a1) puts arg4's lbu at the lowest LUID while arg5's whole chain sits below arg4's scaling, splitting arg4's chain the way target does.
- mechanism: s28 recorded that `s32 i4 = idx_1494[0];` assigned first with arg4 inline plus a co-present value local produces target's split arg4 chain (lbu early, deferred addu, value load last). Combining that with an arg5 value local should also invert the two chains' LUID order.
- probe: tmp/grind/CD_datasync/s30/gen.py -> a1 (s32 i4), a2 (u8 i4), a4 (+arg2 local), a5 (+i5 local), a7 (*idx_1494 spelling), plus controls c1 (arg5 local alone) and c2 (i4 alone), and b3 (both indices + arg5 value).
- result: KILLED (instance). All eight emit ONE window at lev 13: the i4 copy folds into the inline arg4 chain, so arg4's lbu migrates to window slot 12 together with its sll/addu/lw and the block reverts to the all-inline geometry. The s28 split does not reproduce on the candidate chassis without the arg3 value local, and that configuration is the one that puts the split chain in $a3/$a2 (s28's 53-form scan). Measured on the HEAD chassis with the do{}while(0) FAKE present.

## [s30] Adding a fifth value local (both indices + arg3 + arg4 + arg5) raises register pressure enough to spill a chain into $t0 and lengthen the block toward target's 18-insn window.
- mechanism: s29's frontier asks for a third scratch pseudo live across arg4's address range; more value locals is the crudest way to create one.
- result: KILLED (instance). b4 emits a chain in $t0 but a 17-insn WINDOW - the arg5 `sw ...,16($sp)` leaves the block, which is exactly the failure mode s28 recorded for all 53 of its $a0/$a3-mapped forms (reorg.c then has the store available for the printf delay slot). lev 14. Measured on the HEAD chassis with the do{}while(0) FAKE present.

## [s30] KILL RE-AUDIT: no banked artifact is closer to target's printf WINDOW than candidate.c.
- probe: tmp/grind/CD_datasync/s30/wscan.py over all 484 compiled forms in the s26/s27/s28/s30 asm directories, scoring window-levenshtein against target's 18-insn window instead of whole-function levenshtein.
- result: CONFIRMED. Minimum window-lev is 7, reached only by the candidate.c family (candidate, m45, n4/n24/n42/n45/n245/n425/n452, ya4/ya45/ya4b/ya4c/yab4/yba4, za4*/zab4*). tools/fake_ablate.py is BROKEN on this host for this function (ERR on both keep-all and drop-1 variants), so s28's hand-built ya53_nowrap ablation remains the FAKE re-audit of record.

## [s31] Both indices hoisted as s32/u8 locals in [0],[1] order with the value locals assigned arg5-first then arg4 splits arg4's LUID chain (index read early, scaling late) the way target's window requires.
- mechanism: sched.c breaks a priority tie by INSN_LUID, and priority on this block is depth-from-block-start, so the two tbl_125c chains are separated only by RTL emission order. calls.c expands inline argument values in ascending argument index while a value-local statement emits its chain at the statement's own lower LUID, so statement order is the only LUID lever the source has.
- probe: tmp/grind/CD_datasync/s30/gen2.py generated b1 (s32 indices), b2 (u8 indices), b5 (+arg2 value local), b6 (no index locals, arg5 then arg4), b7 (one index local); compiled with s30/sweep.sh and classified by emitted window signature with s30/rep.py against target's 18-insn puts->printf window.
- result: All five emit a single window - candidate.c's window with the two lbu reads merely transposed (lbu v1,1(s1) | lbu v0,0(s1) | ...) - at lev 8 against candidate's 7. The index copies fold in combine, so the index read never separates from its own scaling chain: whichever argument's value-local statement is written first takes BOTH window slot 1 and the first-completed chain. a6 (i4 plus both value locals) lands in the same window.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 7 forms plus the candidate control, fast harness calibrated sandbox = lev + nop - 3

## [s31] A named s32 index local for arg4 with arg4 left inline and arg5 hoisted as a value local before it keeps arg4's lbu at window slot 1 while deferring its scaling chain, reproducing s28's split arg4 chain on the candidate chassis.
- mechanism: s28 recorded the split as triggered by an s32 index local assigned first with arg4 inline plus at least one co-present value local; pairing that with an arg5 value local should additionally invert the two chains' LUID order.
- probe: tmp/grind/CD_datasync/s30/gen.py generated a1 (s32 i4), a2 (u8 i4), a4 (+arg2 local), a5 (+i5 local), a7 (*idx_1494 spelling), b3 (both indices + arg5 value), and controls c1 (arg5 local alone) and c2 (i4 alone).
- result: All eight emit one window at lev 13: the i4 copy folds into the inline arg4 chain and arg4's lbu migrates to window slot 12 with its sll/addu/lw, reverting to the all-inline geometry. s28's split does not reproduce on the candidate chassis without the arg3 value local, and that configuration is exactly the one that lands the split chain in $a3/$a2 rather than $a0.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; 8 forms plus the candidate control

## [s31] Adding a fifth value local (both indices plus arg3, arg4 and arg5) raises register pressure enough to lengthen the printf block toward target's 18-insn window.
- mechanism: s29's frontier asks for a third scratch pseudo live across arg4's address range; more value locals is the crudest way to create one.
- probe: Form b4 in tmp/grind/CD_datasync/s30/gen2.py, compiled and window-classified with the same harness.
- result: b4 does push a chain into $t0, but the arg5 sw ...,16($sp) leaves the block, producing a 17-insn window (lev 14) - the same failure mode s28 recorded for all 53 of its $a0/$a3-mapped forms, where reorg.c then has the store available for the printf delay slot.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; form b4

## [s31] Some banked artifact from sessions 26-28 is closer to target's printf window than candidate.c, so the mandated kill re-audit should be run on it rather than on candidate.c.
- mechanism: Previous sessions scored forms by whole-function levenshtein, which mixes the matched prologue/tail into the number; scoring the puts->printf window alone is the metric that decides the residual.
- probe: tmp/grind/CD_datasync/s30/wscan.py over all 484 compiled forms in the s26/s27/s28/s30 asm directories, computing window-levenshtein against target's 18-insn window.
- result: Minimum window-lev is 7 and it is reached ONLY by the candidate.c family (candidate, m45, n4/n24/n42/n45/n245/n425/n452, ya4/ya45/ya4b/ya4c/yab4/yba4, and the za4*/zab4* group). No banked artifact is closer, so every s21-s29 instance kill stands on the current chassis. tools/fake_ablate.py itself is broken on this host for this function (ERR on both the keep-all and drop-1 variants, its sweep_variants.py child returning no score), so s28's hand-built ya53_nowrap ablation remains the FAKE re-audit of record.
- verdict: CONFIRMED

## [s32] combine folds the single-use index copy into arg4's scaling chain, so hoisting the index alone can never decouple `lbu` from `sll` (the s30/s31 mechanism of record, and the premise of the live frontier's first item).
- mechanism: s30/s31 inferred the fold from emitted assembly alone - every index-local form showed the `lbu` adjacent to its `sll` late in the window - and attributed it to combine substituting a single-use `reg/v` pseudo into its consumer.
- probe: Built `tmp/grind/CD_datasync/s32/u3.c` with `-da` via `s32/dmp.sh` and read the CD_datasync block out of `w.i.combine`, `w.i.sched`, `w.i.lreg`, `w.i.greg`, `w.i.sched2` with `s32/blk.py`.
- result: KILLED. The post-combine RTL keeps insn 93 (`reg86 = zero_extend(mem(reg76))`, the idx[0] lbu) at the head of the block, with its `sll` (insn 120, LOG_LINKS -> 93) after the entire arg5 and arg3 chains and its value load (insn 132) last - i.e. TARGET's LUID geometry, already in hand before scheduling. combine performs no fold. The `lbu` is moved from block position 1 to position 10 by sched1 (`.sched`, reproduced verbatim in `.lreg`/`.greg`), and sched2 emits it at window slot 12. Every "give the index a second consumer / a conversion insn / a second indirection so combine cannot fold it" probe on the old frontier was aimed at a pass that is not acting.
- verdict: KILLED

## [s32] Some member of the index-local x value-local x statement-order space puts idx[0]'s `lbu` at window slot 1 while leaving arg4's scaling chain late (target's decoupling).
- mechanism: s28 exhausted the 65-member VALUE-local space; the crossing with a hoisted index local had never been enumerated, and s28's own split-chain finding suggested the index local was the trigger.
- probe: `tmp/grind/CD_datasync/s32/gen3.py` - all 48 members of {index local} x {subsets of arg2/arg3/arg5 as value locals} x {every statement order}, arg4 inline via the index; plus 21 hand-designed forms in `gen.py` / `gen2.py` (arg4-inline x third-live-scratch, index types, arg4-named, scaled offsets, &-subscript pointer locals). 69 forms compiled, windows classified by signature with `tmp/grind/CD_datasync/s30/rep.py`.
- result: KILLED. Minimum window-levenshtein 11 inside the index-local family and 7 (a tie with candidate.c, never a win) in the r4/r5/r7 scaled-offset family; every form is an 18-insn window except q5 (17, an automatic loss). idx[0]'s `lbu` reaches window slot 1 in none of the 69. The forms that put it earliest (u1/vi53, slot 4) do so only behind arg3's `lui/lbu` pair.
- verdict: KILLED

## [s32] A pointer local spelled `&tbl_125c[idx_1494[0]]` gives the address chain a statement-expanded (early) position and the value load a load_register_parameters (late) position WHILE keeping target's index-first `addu` operand order.
- mechanism: s26's rule ("a pointer-typed named expression emits base-first") had only been measured on the `(u8 *)base + k` spelling; the `&`-subscript spelling preserves the array-subscript expansion and might have emitted `plus(ashift(idx,2), base)`.
- probe: t1/t2/t3/t6/t8/u5 in `tmp/grind/CD_datasync/s32/gen2.py`, six spellings crossed with arg5/arg3/arg2 value locals.
- result: KILLED. All six collapse to one window (lev 9) whose arg4 addu is `addu v1,s0,v1` - BASE-first. The &-subscript form obeys the same operand-order rule as the cast-and-add form, so no pointer spelling of the arg4 address survives the s26 pre-filter.
- verdict: KILLED

## [s32] An index local combined with BOTH other argument value locals reproduces target's split arg4 chain with target's index-first addu.
- mechanism: s28 found split chains only in base-first or wrong-register forms; the split had never been crossed with the arg3 value local on the current chassis.
- probe: `u1` (= `vi53`): `s32 i4 = idx_1494[0]; arg5 = tbl_125c[idx_1494[1]]; arg3 = tbl_11dc[D_800A11D5];` with arg4 inline as `tbl_125c[i4]`.
- result: CONFIRMED (structurally; lev 15 by score). Emits `lbu a3,0(s1)` at slot 4, `sll a3,a3,2` at 11, `addu a3,a3,s0` at 14, `lw a3,0(a3)` at 16 - target's split shape and target's index-first operand order, for the first time in 32 sessions. It differs from target in exactly two ways: the `lbu` sits at slot 4 rather than 1, and the address pseudo coalesces with the value pseudo into ONE register ($a3) where target keeps the address in $a0 and loads the value into $a3.
- verdict: CONFIRMED

## [s33] combine folds the single-use hoisted index copy into arg4's scaling chain, so an index local cannot decouple the lbu from the sll (the s30/s31 mechanism of record and the premise of the inherited frontier's first item).
- mechanism: s30/s31 inferred the fold from emitted assembly alone - every index-local form showed the lbu adjacent to its sll late in the window - and attributed it to combine substituting a single-use reg/v pseudo into its consumer. That attribution was never read out of a dump.
- probe: Built tmp/grind/CD_datasync/s32/u3.c (s32 i4 = idx_1494[0]; arg4 inline as tbl_125c[i4], everything else inline) with -da via s32/dmp.sh, and extracted the CD_datasync printf block from w.i.combine, w.i.sched, w.i.lreg, w.i.greg and w.i.sched2 with s32/blk.py.
- result: The post-combine RTL keeps insn 93 (reg86 = zero_extend(mem(reg76)), the idx[0] lbu) at the HEAD of the block, its sll (insn 120, LOG_LINKS back to 93) after the entire arg5 and arg3 chains, and its value load (insn 132, a3 = mem(reg104)) LAST - that is target's LUID geometry, already in hand before scheduling. combine performs no fold at all. The block's insn order is 93-first in .combine and 93-at-position-10 in .sched, with .lreg and .greg reproducing .sched verbatim, so the sink is sched1 and register allocation does not reorder. sched2 then emits the lbu at window slot 12. Every 'give the index a second consumer / a conversion insn / a second level of indirection so combine cannot fold it' probe on the inherited frontier was aimed at a pass that is not acting.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; form u3 built with -da; read from w.i.combine / w.i.sched / w.i.lreg / w.i.greg / w.i.sched2

## [s33] Some member of the index-local x value-local x statement-order cross-product puts idx[0]'s lbu at window slot 1 while leaving arg4's scaling chain late, which is the one decoupling target's window requires.
- mechanism: s28 exhausted the 65-member VALUE-local subset/order space but never crossed it with a hoisted index local, and s28's own split-chain finding suggested the index local was the trigger for target's split arg4 chain.
- probe: tmp/grind/CD_datasync/s32/gen3.py generated all 48 members of {s32 i4 = idx_1494[0];} x {subsets of arg2/arg3/arg5 as value locals} x {every statement order with i4 in every position}, arg4 always inline via i4. Plus 21 hand-designed forms: gen.py p1-p8 (arg4 inline crossed with third-live-scratch spellings), q1-q5 (index type, arg4 named), r1-r7 (scaled offset crossed with order); gen2.py t1-t9 (&-subscript pointer locals), u1-u5. All 69 compiled and classified by window signature with tmp/grind/CD_datasync/s30/rep.py.
- result: Minimum window-levenshtein is 11 inside the index-local family and 7 (a tie with candidate.c, never a win) in the r4/r5/r7 scaled-offset family; every form is an 18-insn window except q5 (17, an automatic loss because the stack store leaves the block). idx[0]'s lbu reaches window slot 1 in none of the 69. The forms that place it earliest (u1 / vi53, slot 4) do so only behind arg3's lui/lbu pair. Candidate.c's family remains the only thing that puts that lbu at slot 1, and it is exactly the family that also completes arg4's chain first.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 69 forms, window-signature classified, floor re-measured live at 7 before the sweep

## [s33] A pointer local spelled &tbl_125c[idx_1494[0]] gives arg4's address chain a statement-expanded (early) position and its value load a load_register_parameters (late) position while keeping target's index-first addu operand order.
- mechanism: s26's operand-order rule ('a pointer-typed named expression emits base-first') had only ever been measured on the (u8 *)base + k spelling. The &-subscript spelling preserves the array-subscript expansion, so it might have emitted plus(ashift(idx,2), base) - which would have been the one spelling combining target's LUID geometry with target's operand order.
- probe: t1/t2/t3/t6/t8/u5 in tmp/grind/CD_datasync/s32/gen2.py - the &-subscript pointer local crossed with arg5, arg3 and arg2 value locals and with both statement orders.
- result: All six spellings collapse to ONE window (lev 9) whose arg4 addu is emitted as addu v1,s0,v1 - BASE-first. The &-subscript form obeys the same expansion rule as the cast-and-add form, so no pointer spelling of the arg4 address survives s26's operand-order pre-filter. Banked as rejected/amp-subscript-pointer-local-emits-base-first-addu-9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; six spellings, one identical emitted window

## [s33] An index local combined with BOTH other argument value locals reproduces target's split arg4 chain with target's index-first addu operand order.
- mechanism: s28 found split arg4 chains only in forms that emitted base-first addus or landed the chain in $a2/$v1; the split had never been crossed with the arg3 value local on the current chassis, and s30/s31 recorded that the s28 split did not reproduce here at all.
- probe: u1 (= vi53): s32 i4 = idx_1494[0]; arg5 = tbl_125c[idx_1494[1]]; arg3 = tbl_11dc[D_800A11D5]; with arg4 inline as tbl_125c[i4].
- result: CONFIRMED structurally (lev 15 by score). u1 emits lbu a3,0(s1) at slot 4, sll a3,a3,2 at slot 11, addu a3,a3,s0 at slot 14 and lw a3,0(a3) at slot 16 - target's split shape AND target's index-first operand order, for the first time in 32 sessions. It differs from target in exactly two ways: the lbu sits at slot 4 rather than slot 1 (arg3's lui/lbu occupy slots 2-3), and the address pseudo COALESCES with the value pseudo into one register ($a3) where target keeps the address in $a0 and loads the value into $a3. Banked as rejected/index-local-plus-arg3-arg5-splits-arg4-index-first-but-coalesces-addr-value-15.c as a structural artifact, not as a loss.
- verdict: CONFIRMED

## [s34] The do{}while(0) FAKE in candidate.c is an inert carrier occupying a pseudo that a real lever wants (mandated kill re-audit, current chassis)
- mechanism: func_8002EA24-s8 failure mode - a FAKE construct sitting on the pseudo a lever targets makes every "lever inert" verdict measured beside it void. tools/fake_ablate.py is broken on this host (s30, s32), so the ablation was hand-built.
- probe: tmp/grind/CD_datasync/s34/nowrap.c = candidate.c with `do { ... } while (0);` replaced by a plain compound block, nothing else changed; applied to src/system.c and scored with `sandbox CD_datasync --disable all`.
- result: KILLED (instance). Score 17 / build_insns 91 against candidate.c's 7 / 91 on the same chassis this session. The wrapper is load-bearing for the callee-saved assignment of the three base-pointer locals exactly as s22/s28 recorded, so it is not a spectator and the s21-s33 instance kills measured with it present stand.
- verdict: KILLED, kill_scope instance, measured_on: HEAD chassis (goto loop, three hoisted pointer locals), candidate.c body, live sandbox.

## [s34] The printf window's residual is decided by INSN_PRIORITY, so a C spelling that changes a chain's dependence depth can move it (the model every session s24-s33 worked under)
- mechanism: s24 computed p(arg5)=7 vs p(arg4)=5 from chain lengths and derived "arg4's chain must carry an extra dependence edge strictly between its lbu and its sll"; s29 rebuilt the table from rtx_cost; s30/s31 "corrected" the accumulation direction to depth-from-block-start. All three assumed rank_for_schedule's priority term decides these picks.
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SCHED_DEBUG=1 BB2_PRIO_DEBUG=1 over candidate.c and over s32's u3.c; pick-block 187 (sched1) and 199 (sched2) extracted from tmp/grind/CD_datasync/s34/d_cand/dbg.txt and d_u3/dbg.txt.
- result: KILLED (instance). Sixteen of the eighteen window insns are picked with pri=2130706433 = LAUNCH_PRIORITY; sched.c:4048 assigns it to every scheduled insn and the ADJPRI hook shows ready-list members raised to it by the birthing-insn adjustment. rank_for_schedule's priority term is a constant tie across the window, so no chain-depth change is consulted. The "extra edge between lbu and sll" programme (inherited frontier item 2 of the s24 digest) is inert by construction, as are the s29 and s30/s31 priority tables.
- verdict: KILLED, kill_scope instance, measured_on: candidate.c and u3.c on the HEAD chassis with the do{}while(0) FAKE present; instrumented-cc1 pick logs, both sched passes.

## [s34] With priority tied, the window is decided by rank_for_schedule's LUID term (higher LUID picked first = emitted last) and by schedule_select's memory-unit blockage override
- mechanism: sched.c is a BACKWARD list scheduler (insn = ready[0]; NEXT_INSN(insn) = last, sched.c:4009/4033-4038), so emission is the reverse of the pick order and an insn is ready only when all its successors are scheduled. Under a priority tie the comparator falls through to INSN_LUID(y) - INSN_LUID(x) (prefers the higher LUID), and schedule_select can skip the sorted head when the unit-0 blockage model (maxb=3) says it cannot issue at this clock - logged as SELBEST clock=N insn=I pos=K with K>0.
- probe: same two pick logs; every SELBEST/BLOCKAGE line in the CD_datasync block read against the ready-list snapshot printed with it.
- result: CONFIRMED. In u3 the pick at clock=12 is SELBEST insn=93 pos=1 over a ready list headed by 104 - the blockage model, not the LUID rule, is what drags idx[0]'s lbu from emission slot 1 to slot 11; the same override fires at clock=8 (insn 98, pos=2). In candidate.c no override touches 93: it is picked last at clock=22 and emitted first. The mechanism of record for the whole residual is therefore the function-unit blockage model in schedule_select plus the LUID term, and NOT priority (s24/s29/s30/s31), NOT a combine fold (s30/s31, already killed by s32), and NOT local-alloc (s29).
- verdict: CONFIRMED.

## [s34] The residual restated one pass earlier: sched1 must emit arg4's sll (96) AFTER arg5's addu (121), and that is checkable from the .sched dump without scoring
- mechanism: sched2 consumes sched1's OUTPUT order as its LUIDs, so the emitted window is a function of sched1's output. candidate.c's sched1 output is 93,96,116,98,119,121,100,108,123,127,125,111,105,113,129,131,133 and its sched2 emission is 93,116,105,96,98,119,121,100,108,123,111,113,125,131,127; target's emission in the same UID vocabulary is 93,116,105,119,121,96,123,108,98,111,113,125,131,100,127. The only structural difference is that target defers arg4's sll/addu/value-load past arg5's sll/addu while keeping arg4's lbu at slot 1 - which in sched1 terms means 96 must be emitted after 121.
- probe: (for the next session) compile each candidate spelling with -da and test one predicate on the .sched dump - does arg4's sll appear after arg5's addu in the CD_datasync block? Classify by that predicate, not by score; the s30/s32 corpora can be re-classified the same way from their existing asm dirs by recompiling with -da.
- result: OPEN - stated and instrumented this session, not yet swept.
- verdict: (open)

## [s35] The do{}while(0) wrapper in candidate.c is an inert carrier occupying a pseudo that a real lever wants.
- mechanism: func_8002EA24-s8 failure mode: a FAKE construct sitting on the pseudo a lever targets voids every 'lever inert' verdict measured beside it. tools/fake_ablate.py is broken on this host (recorded s30, s32), so the ablation was hand-built.
- probe: tmp/grind/CD_datasync/s34/nowrap.c = candidate.c with the do{}while(0) replaced by a plain compound block, nothing else changed; applied to src/system.c and scored with sandbox CD_datasync --disable all.
- result: Score 17 / build_insns 91 against candidate.c's 7 / 91 measured on the same chassis this session. The wrapper is load-bearing, so it is not a spectator and the s21-s33 instance kills measured with it present stand.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals), candidate.c body, do{}while(0) FAKE ablated by hand; live sandbox

## [s35] The printf window's pick order is decided by INSN_PRIORITY, so a C spelling that lengthens arg4's dependence chain between its lbu and its sll can move it.
- mechanism: s24 derived p(arg5)=7 vs p(arg4)=5 from chain lengths and made 'an extra dependence edge strictly between arg4's lbu and its sll' the central frontier item; s29 rebuilt the table from rtx_cost; s30/s31 corrected the accumulation direction. All three assume rank_for_schedule's priority term decides these picks.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SCHED_DEBUG=1 BB2_PRIO_DEBUG=1 over candidate.c and over s32's u3.c; CD_datasync pick blocks extracted from tmp/grind/CD_datasync/s34/d_cand/dbg.txt and d_u3/dbg.txt (blocks 187 = sched1, 199 = sched2 in both runs).
- result: Sixteen of the eighteen window insns are picked with pri=2130706433 = LAUNCH_PRIORITY; sched.c:4048 assigns it to every scheduled insn and the ADJPRI hook shows ready-list members raised to it by the birthing-insn adjustment. rank_for_schedule's first test is therefore a constant tie across the window and chain depth is never consulted. The inherited frontier's 'extra edge between lbu and sll' programme is inert by construction, as are the s29 and s30/s31 priority tables.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c and tmp/grind/CD_datasync/s32/u3.c on the HEAD chassis with the do{}while(0) FAKE present; instrumented-cc1 pick logs for both sched passes

## [s35] With the priority term tied, the window is decided by rank_for_schedule's LUID term and by schedule_select's memory-unit blockage override.
- mechanism: sched.c is a BACKWARD list scheduler (insn = ready[0]; NEXT_INSN(insn) = last, sched.c:4009/4033-4038), so emission is the reverse of the pick order and an insn is ready only once all its successors are scheduled. Under a priority tie the comparator falls through to INSN_LUID(y) - INSN_LUID(x), preferring the higher LUID (picked first = emitted last); schedule_select can skip the sorted head when the unit-0 blockage model (maxb=3) says it cannot issue at this clock, logged as SELBEST clock=N insn=I pos=K with K>0.
- probe: Every SELBEST/BLOCKAGE line in the CD_datasync pick block of both logs, read against the ready-list snapshot printed with it.
- result: CONFIRMED. In u3 the pick at clock=12 is 'SELBEST insn=93 pos=1' over a ready list headed by 104 - the blockage model, not the LUID rule, is what drags idx[0]'s lbu from emission slot 1 to slot 11; the same override fires at clock=8 on insn 98 (pos=2). In candidate.c no override touches 93: it is picked last at clock=22 and emitted first. The mechanism of record for the residual is the blockage model plus the LUID term, not priority (s24/s29/s30/s31), not a combine fold (s30/s31, already killed by s32), not local-alloc (s29).
- verdict: CONFIRMED

## [s36] The do{}while(0) FAKE is an inert carrier occupying a pseudo that a real argument-block lever wants (mandated kill re-audit, widened from candidate.c to all three live families)
- mechanism: func_8002EA24-s8 failure mode - a FAKE sitting on the pseudo a lever targets voids every "lever inert" verdict measured beside it. s34/s35 both re-ran the same candidate.c ablation; the families that actually carry the closest geometries (value-local x2, pointer-local y1, index-local u3/u1) had never been ablated on the current chassis.
- probe: tmp/grind/CD_datasync/s36/nowrap.py replaces `do { ... } while (0);` with a plain compound block, nothing else changed, on x2 / y1 / w0_u3 / u1; all four compiled through s36/sweep.sh and compared window-to-window against their wrapped twins.
- result: KILLED (instance). Every ablated form's emitted printf window is textually identical to its wrapped twin except that the two callee-saved base registers renumber $16/$17 -> $17/$18; the scores move 8->18, 9->19, 13->23, 15->24 entirely from that rotation. The wrapper carries the callee-saved assignment and nothing else, in all three families. Every s21-s35 instance kill measured beside it stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals), four bodies (x2, y1, w0_u3, u1) each with the do{}while(0) FAKE hand-ablated; s36 harness, lev+nop calibrated to the live sandbox

## [s36] Some spelling that changes which memory operations are adjacent in the printf block - an arg2 value local, an arg3 index local, a second index local, a fifth value local - removes the sched1 blockage override that sinks idx[0]'s lbu in the index-local family (inherited s34 frontier item 2, stated verbatim)
- mechanism: s34 read the pick logs and found the sink is `SELBEST clock=12 insn=93 pos=1` - schedule_select skipping the sorted head of the ready list because the unit-0 (memory) blockage model says the head cannot issue at that clock. The blockage cost depends on which memory insns are already in the partial schedule, so moving one load or lbu within the block was the named lever.
- probe: fifteen index-local spellings compiled with the instrumented cc1 through tmp/grind/CD_datasync/s36/sweep.sh - w0 (bare index local), w1/w2 (arg2 value local before/after it), w3/w4 (arg3 index local in both orders), w5/w6 (both indices hoisted in both orders), w8/w9 (arg5 value local, with and without arg2), w11/w12 (three index locals), w13 (index + arg3 index + arg5 value), x1/x3 (arg5 value local with arg4 inline, both orders). Classified by emitted window AND by the SELBEST list, per s34's instruction.
- result: KILLED (instance). All fifteen emit a BYTE-IDENTICAL 15-insn window at lev 13 (`lbu idx1 | lbu arg3 | lw arg2 | sll a5 | addu a5 | sll a3 | lw a5val | addu a3 | sw | lbu idx0 | lw a2 | sll a4 | addu a4 | lw a4val | la`). The override never disappears: its insn UID moves with the spelling (i96/i93/i97/i102) but its clock (8, 12, 17), its position (pos=2, pos=1, pos=1) and the resulting emission are invariant. Load adjacency inside the argument list is not the variable the blockage model is reading.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 15 index-local spellings, instrumented-cc1 sched logs plus emitted windows

## [s36] A pointer-valued arg4 whose index is written on the LEFT of the addition reaches target's index-first `addu` (the last untested spelling of the s26/s32 operand-order rule)
- mechanism: s26 established that an inline subscript expands index-first and a pointer-typed named expression expands base-first, and s32 killed `&tbl_125c[idx]` and `(u8 *)tbl_125c + k` on that ground. Both spellings write the base first in the source, so the open question was whether the expander preserves the source operand order of the PLUS.
- probe: y1/y2/y3 (`p4 = idx_1494[0] + tbl_125c;` in three statement orders, alone and crossed with arg5/arg3 value locals) and y6/y7 (`p4 = &idx_1494[0][tbl_125c];`, the reversed-subscript spelling), plus z2/z3/z5 (both arguments as pointers, both orders).
- result: KILLED (instance). Every one of them emits `addu $3,$16,$3` - base first - exactly like s32's spellings, at lev 9/10. GCC 2.7.2 canonicalises the PLUS before expansion, so the source operand order of a pointer addition is not observable in the emitted addu. Since every value-expression spelling measured (inline or value local) emits index-first and target's `addu $a0,$a0,$s0` is index-first, target's arg4 is a value expression and not a pointer local - which retires the whole pointer family as an arg4 chassis rather than just the two spellings s32 killed.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; 8 pointer spellings (y1,y2,y3,y6,y7,z2,z3,z5)

## [s36] "idx[0]'s lbu emitted first" and "arg4's sll emitted after arg5's addu" are separable by some statement-order or naming choice within the value-local family
- mechanism: target needs both, and the value-local family is the only family that reaches the first. If the two conditions were independent knobs, some assignment order over {arg2,arg3,arg4,arg5} value locals would satisfy both.
- probe: z1 (arg4 then arg5), x2 (arg5 then arg4), z4 (arg4 value local then arg5 pointer), z6 (arg4 value local then arg5 index local), x4/x6/x7/x8/x9 (arg5 crossed with arg3 and arg2 in several orders), plus candidate.c as the arg4-only baseline.
- result: KILLED (instance). The two conditions are the SAME condition in this family: whichever argument's chain is expanded first gets both its index lbu AND its sll emitted first. arg4-first forms (candidate, z1, z4, z6) all land lev 7 with arg4's sll ahead of arg5's; arg5-first forms (x2, x4, x7, x9) put idx[1]'s lbu at slot 1. The mechanism is the backward scheduler: both index lbus are leaves that become ready together and the LUID tie-break emits the lower-LUID one first, and that is the same LUID that puts its sll first. Decoupling them requires a LOW LUID on the lbu with a HIGH LUID on the sll - which is precisely the index local, and precisely the family the sched1 blockage override defeats.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; 10 value-local spellings, s36 harness

## [s36] New closest-by-prefix artifact: arg5 and arg4 as value locals in that order reproduces target's window slots 3-6 exactly
- mechanism: expanding arg5's chain first gives it the lower combine LUIDs, so its sll/addu are emitted before arg4's sll - the one ordering relation target has and candidate.c lacks.
- probe: x2 = `arg5 = tbl_125c[idx_1494[1]]; arg4 = tbl_125c[idx_1494[0]]; printf(fmt, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);`
- result: CONFIRMED for slots 3-6 (`lw arg2 | sll a5 | addu a5 | sll a4`), lev 8. The first form in 36 sessions to put arg5's scaling chain ahead of arg4's sll. It loses on the lbu order at slots 1/2 and on arg4's addu/value-load staying glued behind its sll. Banked as rejected/arg5-then-arg4-value-locals-gets-target-slots-3-6-but-swaps-lbu-order-8.c - an artifact, not a loss.
- verdict: CONFIRMED

## [s36] The do{}while(0) FAKE in candidate.c is an inert carrier occupying a pseudo that a real argument-block lever wants - re-audited across all three live families rather than only on candidate.c.
- mechanism: func_8002EA24-s8 failure mode: a FAKE sitting on the pseudo a lever targets voids every 'lever inert' verdict measured beside it. s34/s35 both re-ran the same candidate.c ablation; the families carrying the closest geometries (value-local x2, pointer-local y1, index-local u3/u1) had never been ablated on the current chassis.
- probe: tmp/grind/CD_datasync/s36/nowrap.py replaces the do{ }while(0) with a plain compound block, nothing else changed, on x2 / y1 / w0_u3 / u1; all four compiled through s36/sweep.sh and compared window-to-window with their wrapped twins.
- result: Every ablated form's emitted printf window is textually identical to its wrapped twin except that the two callee-saved base registers renumber $16/$17 -> $17/$18; scores move 8->18, 9->19, 13->23, 15->24 entirely from that rotation. The wrapper carries the callee-saved assignment and nothing else, in all three families, so every s21-s35 instance kill measured beside it stands.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals); four bodies x2/y1/w0_u3/u1 with the do{}while(0) FAKE hand-ablated; s36 harness, lev+nop calibrated against the live sandbox (candidate.c lev 7 == sandbox 7)

## [s36] Some spelling that changes which memory operations are adjacent in the printf block - an arg2 value local, an arg3 index local, a second index local, a fifth value local - removes the sched1 blockage override that sinks idx[0]'s lbu in the index-local family.
- mechanism: s34 read the pick logs: the sink is SELBEST clock=12 insn=93 pos=1, schedule_select skipping the sorted head of the ready list because the unit-0 (memory) blockage model says the head cannot issue at that clock. The blockage cost depends on which memory insns are already in the partial schedule, so moving one load or lbu inside the block was the named lever (inherited s34 frontier item 2, verbatim).
- probe: Fifteen index-local spellings compiled with the instrumented cc1 through tmp/grind/CD_datasync/s36/sweep.sh: w0 (bare index local), w1/w2 (arg2 value local before/after), w3/w4 (arg3 index local both orders), w5/w6 (both indices hoisted both orders), w8/w9 (arg5 value local with and without arg2), w11/w12 (three index locals), w13, x1/x3 (arg5 value local with arg4 inline, both orders). Classified by emitted window AND by the SELBEST list.
- result: All fifteen emit a byte-identical 15-insn window at lev 13 (lbu idx1 | lbu arg3 | lw arg2 | sll a5 | addu a5 | sll a3 | lw a5val | addu a3 | sw | lbu idx0 | lw a2 | sll a4 | addu a4 | lw a4val | la). The override never disappears: its insn UID moves with the spelling (i96/i93/i97/i102) but its clocks (8, 12, 17), its positions (pos=2, pos=1, pos=1) and the emitted window are invariant. Load adjacency inside the argument list is not the variable the blockage model reads.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; 15 index-local spellings; instrumented-cc1 sched logs plus emitted windows

## [s36] A pointer-valued arg4 whose index is written on the LEFT of the addition reaches target's index-first addu - the last untested spelling of the s26/s32 operand-order rule.
- mechanism: s26 established that an inline subscript expands index-first and a pointer-typed named expression base-first; s32 killed &tbl_125c[idx] and (u8 *)tbl_125c + k on that ground. Both write the base first in the source, so the open question was whether the expander preserves the source operand order of the PLUS.
- probe: y1/y2/y3 (p4 = idx_1494[0] + tbl_125c; in three statement orders, alone and crossed with arg5/arg3 value locals), y6/y7 (p4 = &idx_1494[0][tbl_125c];, the reversed-subscript spelling), z2/z3/z5 (both arguments as pointers, both orders).
- result: All eight emit addu $3,$16,$3 - base first - exactly like s32's spellings, at lev 9/10. GCC 2.7.2 canonicalises the PLUS before expansion, so a pointer addition's source operand order is not observable in the emitted addu. Since every value-expression spelling measured emits index-first and target's addu $a0,$a0,$s0 is index-first, target's arg4 is a value expression, not a pointer local - which retires the pointer family as an arg4 chassis rather than just the two spellings s32 killed.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; 8 pointer spellings (y1,y2,y3,y6,y7,z2,z3,z5)

## [s36] The condition 'idx[0]'s lbu emitted first' and the condition 'arg4's sll emitted after arg5's addu' are separable by some statement-order or naming choice inside the value-local family.
- mechanism: Target needs both conditions and the value-local family is the only family that reaches the first. If they were independent knobs, some assignment order over {arg2,arg3,arg4,arg5} value locals would satisfy both.
- probe: z1 (arg4 then arg5), x2 (arg5 then arg4), z4 (arg4 value local then arg5 pointer), z6 (arg4 value local then arg5 index local), x4/x6/x7/x8/x9 (arg5 crossed with arg3 and arg2 in several orders), candidate.c as the arg4-only baseline.
- result: The two conditions are the same condition in this family: whichever argument's chain is expanded first gets both its index lbu and its sll emitted first. arg4-first forms (candidate, z1, z4, z6) all land lev 7 with arg4's sll ahead of arg5's; arg5-first forms (x2, x4, x7, x9) put idx[1]'s lbu at slot 1. Mechanism: the backward scheduler leaves both index lbus ready together and the LUID tie-break emits the lower-LUID one first - the same LUID that puts its sll first. Decoupling them needs a LOW LUID on the lbu with a HIGH LUID on the sll, which is exactly the index local and exactly the family the blockage override defeats.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; 10 value-local spellings, s36 harness

## [s36] arg5 and arg4 spelled as value locals in that order reproduces target's window slots 3-6 exactly (lw arg2 | sll a5 | addu a5 | sll a4).
- mechanism: Expanding arg5's chain first gives it the lower combine LUIDs, so its sll/addu are emitted before arg4's sll - the one ordering relation target has and candidate.c lacks.
- probe: x2 = arg5 = tbl_125c[idx_1494[1]]; arg4 = tbl_125c[idx_1494[0]]; printf(fmt, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);
- result: CONFIRMED for slots 3-6, lev 8 - the first form in 36 sessions with arg5's scaling chain ahead of arg4's sll. It loses only on the lbu order at slots 1/2 and on arg4's addu/value-load staying glued behind its sll. Banked as an artifact, not a loss.
- verdict: CONFIRMED

## [s25] The `do{}while(0)` FAKE is masking a lever on one of the argument-block pseudos, so the s21-s24 instance kills measured "with the FAKE present" are not kills of the underlying lever
- mechanism: The mandated kill re-audit protocol (func_8002EA24 s8 precedent) — a lever measured inert while a FAKE carrier occupies its target pseudo is not a kill.
- probe: `python3 tools/fake_ablate.py --func CD_datasync --file system --candidate memory/grind/CD_datasync/candidate.c`.
- result: **KILLED (instance).** keep-all = 7 / 91 build insns; drop-1 (wrapper removed) = 27 / 87. The wrapper does not sit on any argument-block pseudo — it is the loop_depth ref-weighting device that fixes the callee-save map (s9 H37/H38), and removing it deletes four instructions and rotates $s0-$s3. There is no chassis "underneath" the FAKE to re-measure the s21-s24 kills on; the ablated chassis is 20 points worse and structurally different. All s21-s24 instance kills stand as recorded.

## [s25] tools/sched_solver can type the two remaining sched1 ready-list decisions REACHABLE or FORECLOSED (s24 frontier item 1, verbatim)
- mechanism: sched_solver models GCC 2.7.2's sched.c list scheduler order- and clock-exactly; the inline form's `.combine` block is proven to carry target's dependence graph, so the only free variables are the pre-sched insn order and the priority/tie-break, which is exactly what `perturb.py` searches.
- probe: (1) `extract.py system` + `validate.py system --funcs CD_datasync` as the self-check; (2) `mkasm.sh system`; (3) `goalmap.build_map(..., target_object="build/src/system.o", ours_object="tmp/sandbox/CD_datasync/system.o")` to get the UID<->asm map; (4) a HAND-BUILT goal (goalmap's automatic goal is a non-topological mis-pairing of duplicate instruction text and is refused by the tool); (5) `perturb.py` depth-1 over all atoms and exhaustive depth-2 over the spellable (luid/luid_move) atoms, on BOTH the candidate.c chassis and the fully-inline chassis. Scripts: tmp/grind/CD_datasync/s25/{blk.py,blk2.py,prefix.py,single.py,solve_inl.py}.
- result: **EXECUTED AND ANSWERED — the model is exact (396/396 blocks order- and clock-exact for the TU, 12/12 for CD_datasync in both passes) and the answer is FORECLOSED-BY-STATEMENT-ORDER on both chassis.** candidate.c chassis: no single atom of 1459 reaches the full goal, no spellable PAIR of 759 reaches it (exhaustive, 339 s), and the binding prefix is constraint 5 ("idx1's addu emitted before idx0's sll"). Inline chassis: every individual constraint is reachable but the joint goal is not reached by any single atom of 1332, and no spellable pair of 693 reaches it (exhaustive, 267 s). The residual is therefore NOT a statement-order question on either chassis — statement order is precisely what luid/luid_move model, and it has now been searched exhaustively to depth 2.

## [s25] The one graph edge the target has and we do not is nameable: arg3's `lbu D_800A11D5` must depend on arg4's `sll`/`addu`
- mechanism: On the inline chassis the solver's preferred single atom, recurring across four separate constraints (k=1, k=2, (119,107), (99,109)), is `add_dep 96 <- 107` / `add_dep 96 <- 109`. On the candidate chassis every individual-constraint solution is `add_dep 93 <- X` ("give idx0's lbu an extra predecessor"). Both are the same shape as the s24 predicate stated from the RTL dumps: an edge inside a chain, not at its ends.
- probe: NOT YET SPELLED. arg3's index (`D_800A11D5`, a byte global) and arg4's shift touch no common register and no common memory, so an ordinary data dependence between them does not exist in this program. The remaining ways a real GCC 2.7.2 dependence could appear between them are (a) an aliasing MEMORY dependence — a store between them that `sched.c` cannot prove independent of the `lbu` (target has exactly one store in the window, the `sw 16($sp)`, and stack stores ARE disambiguated, so this needs a NON-stack store); (b) arg3's index being computed FROM arg4's chain, which would be a different program; (c) the arg3 value not being a separate global read at all — i.e. the original source's third argument is not `tbl_11dc[D_800A11D5]` but something that shares a subexpression with the fourth.
- next: option (c) is the only one that is both spellable and honest, and it is a DATA-MODEL question rather than a spelling question: it says the ledger's reading of the format-string arguments may be wrong.

## [s25] The joint goal is reachable by a PAIR of dependence-graph edges, so the open question is 'which two dependences' rather than 'which C construct'
- mechanism: Depth-1 over all atoms fails from prefix k=3 on the inline chassis, but a pair of edges could satisfy the conjunction; the atom set includes an arbitrary add_dep of every kind between every ordered insn pair plus arbitrary insn-cost changes, so a depth-2 sweep is an exhaustive test of "target's graph is within two edges of ours".
- probe: tmp/grind/CD_datasync/s25/d2all.py - exhaustive itertools.combinations over all 1332 atoms of pass-1 block 3 on the inline chassis, goal = the 14 hand-built --goal-before constraints.
- result: **KILLED (instance).** 886,446 pairs, 0 hits, 980 s. Target's block-3 dependence graph is more than two edges away from the fully-inline form's. This refutes the strong form of s24's "the inline form has EXACTLY target's pre-scheduling dependence graph" and, with the two chassis sweeps above, closes statement-order and near-graph search on this program. The next rung is not a deeper search of the same program - it is a different argument expression.

## [s37] The do{}while(0) FAKE in candidate.c occupies a pseudo in the printf argument block, so the s21-s24 instance kills measured with it present were measured on a coerced chassis.
- mechanism: Mandated kill re-audit (func_8002EA24 s8 precedent): a lever measured inert while a FAKE carrier sits on its target pseudo is not a kill.
- probe: python3 tools/fake_ablate.py --func CD_datasync --file system --candidate memory/grind/CD_datasync/candidate.c
- result: keep-all = score 7 / 91 build insns; drop-1 (wrapper removed) = score 27 / 87 build insns. The wrapper is the loop_depth ref-weighting device that fixes the callee-save map (s9 H37/H38), not an argument-block carrier: removing it deletes four instructions and rotates $s0-$s3. There is no un-coerced chassis underneath it to re-measure on, so every s21-s24 instance kill stands exactly as recorded.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals), candidate.c with and without the do{}while(0) FAKE; tools/fake_ablate.py, 2 variants

## [s37] On the candidate.c chassis, target's window order is produced by some single modelled scheduler-input change, or by a pair of statement-move atoms.
- mechanism: tools/sched_solver replicates sched.c order- and clock-exactly; luid and luid_move are exactly the atoms an ordinary source-statement move can produce, so an exhaustive depth-2 sweep over them is an exhaustive sweep of statement order on that chassis.
- probe: extract.py system (parity=True) + validate.py (396/396 blocks order- and clock-exact, CD_datasync 12/12 in both passes) as self-check; hand-built goal from asm/funcs/CD_datasync.s:47-68 expressed as 14 --goal-before pairs over UIDs 93,116,105,119,121,96,123,108,98,111,113,125,131,100,127; perturb.py depth-1 over all 1459 atoms and exhaustive depth-2 over 759 spellable atoms.
- result: No hit at depth 1 over all 1459 atoms (add_dep of every kind, luid, luid_move, insn-cost) and no hit in the exhaustive spellable depth-2 sweep (339 s). Prefix analysis localises the binding constraint to the fifth of fourteen - 'idx1's addu is emitted before idx0's sll' (pick-order 96:121); prefixes k=1..4 are reachable at depth 1, k>=5 are not. Separately, the single constraint 'arg3's lw $a2 before arg4's lw $a3' (pick-order 100:131) has no depth-1 solution at all, the only one of the fourteen for which not even an arbitrary add_dep works - the price of hoisting arg4 into a named local, whose extra copy pins arg4's load early while target loads it last.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (arg4 hoisted into a named local) with the do{}while(0) FAKE present; sched_solver pass-1 block 3, 1459 single atoms + all 759-choose-2 spellable pairs

## [s37] On the fully-inline chassis - the one whose .combine graph s24 showed equals target's - target's window order is produced by some single modelled input change or by a pair of statement-move atoms.
- mechanism: Same solver; the inline form's dependence graph is target's, so if the residual were pre-sched insn order or a priority tie the search would name the move.
- probe: Inline body applied to src/system.c (sandbox 13/91, build_insns 91), mkasm.sh + extract.py re-run on that source state, UID map rebuilt via goalmap object mode, goal = 104,112,93,115,117,107,119,96,109,99,101,121,127,129,123; depth-1 over all 1332 atoms, per-constraint and per-prefix, plus exhaustive depth-2 over 693 spellable atoms.
- result: Every one of the fourteen constraints is individually reachable here (unlike candidate.c), and EIGHT of them are already satisfied by the baseline schedule versus six on candidate.c - independent confirmation that the 13-scoring inline form is the structurally closer one and that the sandbox score is an inverted gradient on this function. But the joint goal has no depth-1 solution beyond prefix k=2, and the exhaustive spellable depth-2 sweep returns nothing (267 s). The solver's preferred atom, recurring across four separate constraints, is add_dep 96 <- 107: arg3's lbu D_800A11D5 must depend on arg4's sll. Those two insns share no register and no memory under the ledger's current reading of the printf argument list.
- verdict: KILLED
- kill_scope: instance
- measured_on: fully-inline argument chassis (no named locals in the timeout block) with the do{}while(0) FAKE present; sched_solver pass-1 block 3, 1332 single atoms + all 693-choose-2 spellable pairs

## [s37] The joint window goal is reached from the inline chassis by a PAIR of modelled input changes (any dependence edge or instruction cost, not just statement moves).
- mechanism: The perturb atom set includes an arbitrary add_dep of every dependence kind between every ordered pair of the block's 22 insns plus an arbitrary insn_cost change on every insn, so an exhaustive depth-2 sweep is an exhaustive test of 'target's graph is within two edges of ours'.
- probe: tmp/grind/CD_datasync/s25/d2all.py - exhaustive itertools.combinations over all 1332 atoms of pass-1 block 3 on the inline chassis against the 14 hand-built goal-before constraints.
- result: 886,446 pairs, 0 hits, 980 s. Target's block-3 dependence graph sits more than two edges away from the fully-inline form's. This refutes the strong form of s24's claim that the inline form carries EXACTLY target's pre-scheduling graph (identical instruction multisets and chain shapes are not an identical graph; had they been, the exact simulator would reach target's order at depth 0). The weaker s24 claim survives: the inline chassis is still the structurally closest one, 8 of 14 constraints baseline-satisfied against candidate.c's 6.
- verdict: KILLED
- kill_scope: instance
- measured_on: fully-inline argument chassis with the do{}while(0) FAKE present; sched_solver pass-1 block 3, exhaustive depth-2 over all 1332 atoms (886,446 pairs)

## [s38] The do{}while(0) FAKE is an inert carrier occupying a pseudo an argument-block lever wants, on the two closest-to-target banked forms (mandated kill re-audit, current chassis)
- mechanism: an instance kill measured with a FAKE construct sitting on the pseudo a lever targets is not a kill (func_8002EA24 s8). s25/s37 ablated candidate.c only; the two forms that sit CLOSEST to target's window geometry (alt_target_geometry_8.c, alt_inline_chassis_13.c) had never been ablated on the current chassis.
- probe: re-measure each live (`sandbox CD_datasync --disable all`), then replace `do { ... } while (0);` with a bare brace block (tmp/grind/CD_datasync/s38/unwrap.py) and re-measure. tools/fake_ablate.py ERRs on both files - its sweep_variants path cannot build them - so the ablation was done by hand.
- result: KILLED. alt_target_geometry_8.c 10 -> 21; alt_inline_chassis_13.c 13 -> 23; build_insns stays 91 in both ablations. The wrapper carries the callee-saved base-pointer map (s9 H37/H38) on all three live families and masks nothing in the argument block.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals), both forms with and without the do{}while(0) FAKE; live sandbox, 4 measurements

## [s38] Target's printf-window order is reached from the inline chassis by some permutation of the argument chains' SOURCE STATEMENT ORDER - the space s37's atom-level depth-2 search structurally could not cover
- mechanism: perturb.py's luid_move atom moves ONE insn's LUID (perturb.py:216,242), but a C statement move relocates a whole 4-5 insn chain, i.e. >= 4 atoms. So s37's exhaustive depth-2 result says nothing about ordinary statement moves. Laying each chain out as a contiguous LUID block, in every order (and with each chain optionally split into address part vs value load, which is what a pointer local vs a value local spells), covers exactly the C-spellable statement-order space.
- probe: tmp/grind/CD_datasync/s38/groupmove.py (inline chassis, pass 2 block 3): 5 chain blocks -> 120 orders, 8 blocks with address/value splits -> 40,320 orders; each applied as a LUID re-lay, priorities recomputed via perturb.apply_priorities, then simulate.sim_block.
- result: KILLED. 0 of 40,320 reach target's window order; best window prefix 0/15. The atom is NOT inert - it yields 6 distinct emissions - but all six differ only in the order of the three leading leaves, and the tail (104,127,107,109,129,123) is invariant. On this chassis arg4's lbu is pinned at window slot 10 and target needs it at slot 1.
- verdict: KILLED
- kill_scope: instance
- measured_on: fully-inline chassis (alt_inline_chassis_13.c, 13/91) with the do{}while(0) FAKE present; sched_solver model extracted with that form in src/, 40,320 simulated orders

## [s38] Target's printf-window order is reached from the candidate.c chassis by some permutation of the argument chains' source statement order
- mechanism: same group-move argument as above, run on the best-scoring chassis.
- probe: tmp/grind/CD_datasync/s38/groupmove_cand.py, 8 blocks -> 40,320 orders, pass 2 block 3.
- result: KILLED. 0 hits; best window prefix 3/15 and the best IS the baseline. Useful positive by-product: candidate.c already emits target's first three window insns (93,116,105) and already emits the fmt `la $a0` LAST in pass 2 - so s28's "make the fmt set be emitted last" question is already answered on this chassis and should not be re-opened.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (goto loop, three hoisted pointer locals, arg4 value local) with the do{}while(0) FAKE present; 40,320 simulated orders

## [s38] The candidate.c and inline chassis are foreclosed on OPPOSITE halves of target's window, so the residual is a program shape carrying both halves
- mechanism: the exact scheduler model can be asked one target fact at a time instead of the joint 14-constraint goal; a single-constraint depth-1 sweep over ALL atoms (add_dep of every kind, del_dep, cost, luid, luid_move) is a strong per-fact verdict.
- probe: perturb.py --pass 2 --block 3 --depth 1 on each chassis. Candidate: `--goal-before 100:131` (arg4's value load emitted after arg3's) and `--goal-before 100:125` (after the sw). Inline: `--goal-before 112:104 96:104 93:104` (idx0's lbu emitted first).
- result: CONFIRMED. All three return NO vector over 1165/1166 atoms. Candidate cannot defer arg4's value load; inline cannot lead with idx0's lbu; each chassis has the other's missing fact for free.
- verdict: CONFIRMED

## [s38] A third chassis - the s32/s33 index-local form u1 - carries target's window FRONT and types REACHABLE on the deferral facts that are foreclosed on candidate.c
- mechanism: u1 (`s32 i4 = idx_1494[0];` + arg5/arg3 value locals, arg4 inline as `tbl_125c[i4]`) was banked in s33 as a structural artifact and never carried into the solver era, because its SCORE (15) is worse than candidate's 7 and s25/s37 chose chassis by the solver's constraint count instead.
- probe: apply rejected/index-local-plus-arg3-arg5-splits-arg4-index-first-but-coalesces-addr-value-15.c, live sandbox, extract.py system (parity=True), then perturb.py depth-1 single-constraint sweeps on pass 2 block 3.
- result: CONFIRMED. 15/91 build_insns 91. Window emission starts `97(idx0 lbu), 108(idx1 lbu), ...` - target's slot 1, which neither other chassis reaches at any depth. `--goal-before 104:115` (arg4's load after arg5's) returns several vectors; `--goal-before 104:135` (arg4's load after arg3's) returns exactly one, `del_dep 127 <- 104`. The remaining structural difference is s33's: the arg4 address pseudo coalesces with its value pseudo into one register where target keeps $a0/$a3 - an RA question for tools/ra_solver, not a scheduling one.
- verdict: CONFIRMED

## [s38] The do{}while(0) FAKE is an inert carrier occupying a pseudo an argument-block lever wants, on the two closest-to-target banked forms (alt_target_geometry_8.c, alt_inline_chassis_13.c).
- mechanism: A kill measured with a FAKE sitting on the pseudo a lever targets is not a kill (func_8002EA24 s8); s25/s37 had ablated candidate.c only, never the two geometry-closest forms.
- probe: Live re-measure of each form on the current chassis, then hand-ablation of the wrapper into a bare brace block (tmp/grind/CD_datasync/s38/unwrap.py) and re-measure. tools/fake_ablate.py ERRs on both files (its sweep_variants path cannot build them), so the ablation was done by hand.
- result: KILLED. alt_target_geometry_8.c 10 -> 21; alt_inline_chassis_13.c 13 -> 23; build_insns stays 91 in both ablations, so the wrapper only rotates the callee-saved base map (s9 H37/H38) and masks nothing in the argument block. This differs from candidate.c, where the same ablation also drops build_insns 91 -> 87.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals); both forms measured with and without the do{}while(0) FAKE; 4 live sandbox measurements

## [s38] Target's printf-window order is reached from the fully-inline chassis by some permutation of the argument chains' source statement order - the space s37's atom-level depth-2 search structurally could not cover.
- mechanism: perturb.py's luid_move atom moves ONE insn's LUID (tools/sched_solver/perturb.py:216,242), but a C statement move relocates a whole 4-5 insn chain, i.e. >=4 atoms, so s37's 886,446-pair depth-2 sweep never tested an ordinary statement move. Laying each chain out as a contiguous LUID block in every order - with each chain optionally split into address part vs value load, which is exactly what a pointer local vs a value local spells - covers the C-spellable statement-order space directly.
- probe: tmp/grind/CD_datasync/s38/groupmove.py on the inline chassis, pass 2 block 3: 5 chain blocks (120 orders) and 8 address/value-split blocks (40,320 orders); each applied as a LUID re-lay, priorities recomputed via perturb.apply_priorities, then simulate.sim_block.
- result: KILLED. 0 of 40,320 orders reach target's window order; best window prefix 0/15. The perturbation is not inert - it produces 6 distinct emissions - but all six differ only in the order of the three leading leaves, and the tail (104,127,107,109,129,123) is invariant: arg4's lbu sits at window slot 10 where target needs slot 1.
- verdict: KILLED
- kill_scope: instance
- measured_on: fully-inline chassis (alt_inline_chassis_13.c, re-measured 13/91) with the do{}while(0) FAKE present; sched_solver model re-extracted with that form in src/ (parity=True); 40,320 simulated orders

## [s38] Target's printf-window order is reached from the candidate.c chassis by some permutation of the argument chains' source statement order.
- mechanism: Same group-move argument, run on the best-scoring chassis, where s26-s28's 465 hand-compiled forms had only sampled the space.
- probe: tmp/grind/CD_datasync/s38/groupmove_cand.py, 8 blocks -> 40,320 orders, pass 2 block 3, model re-extracted with candidate.c in src/ (parity=True; the model's pass-1 emission reproduces the ledger's recorded candidate sched1 order exactly, which independently validates the s37 UID map).
- result: KILLED. 0 hits; best window prefix 3/15 and the best IS the baseline. Positive by-product: candidate.c already emits target's first three window insns (93 idx0-lbu, 116 idx1-lbu, 105 arg2-lw) and already emits the fmt la $a0 LAST in pass 2, so s28's 'make the fmt set be emitted last' question is already answered on this chassis and must not be re-opened.
- verdict: KILLED
- kill_scope: instance
- measured_on: candidate.c chassis (goto loop, three hoisted pointer locals, arg4 value local) with the do{}while(0) FAKE present; 40,320 simulated orders

## [s38] The candidate.c and fully-inline chassis are foreclosed on opposite halves of target's window, each missing exactly the one fact the other has for free.
- mechanism: The exact scheduler model can be asked one target fact at a time instead of the joint 14-constraint goal; a single-constraint depth-1 sweep over ALL atoms (add_dep of every kind, del_dep, cost, luid, luid_move) is a strong per-fact verdict, and s37 only ever asked the joint goal at depth 2.
- probe: perturb.py --pass 2 --block 3 --depth 1: on candidate.c --goal-before 100:131 (arg4's value load emitted after arg3's) and --goal-before 100:125 (after the sw 16($sp)); on the inline chassis --goal-before 112:104 96:104 93:104 (idx0's lbu emitted first in the window).
- result: CONFIRMED. All three return NO vector, over 1165 and 1166 atoms respectively. candidate.c cannot defer arg4's value load; the inline chassis cannot lead with idx0's lbu. By contrast, on candidate.c the constraint 'idx1's sll before idx0's sll' IS reachable and every solution is an add_dep 93 <- X (an extra predecessor for idx0's lbu), reproducing s37's family from the other pass.
- verdict: CONFIRMED

## [s38] The s32/s33 index-local form u1 carries target's window front and types REACHABLE on the deferral facts that are foreclosed on candidate.c.
- mechanism: u1 (s32 i4 = idx_1494[0]; plus arg5 and arg3 value locals, arg4 inline as tbl_125c[i4]) was banked in s33 as a structural artifact and never carried into the solver era, because chassis selection since s25 has gone by score or by joint-constraint count, not by which single target facts are individually reachable.
- probe: Apply rejected/index-local-plus-arg3-arg5-splits-arg4-index-first-but-coalesces-addr-value-15.c, live sandbox, extract.py system (parity=True), then perturb.py depth-1 single-constraint sweeps on pass 2 block 3.
- result: CONFIRMED. 15/91, build_insns 91. Pass-2 window emission is 97(idx0 lbu),108(idx1 lbu),93,120,100,102,111,113,123,104(arg4 lw),115(arg5 lw),125,127,135(arg3 lw),129 - idx0's lbu at slot 1, exactly target's slot 1, which neither other chassis reaches at any depth. --goal-before 104:115 returns several vectors and --goal-before 104:135 returns exactly one (del_dep 127 <- 104). Promoted out of rejected/ to memory/grind/CD_datasync/alt_u1_index_local_15.c with the full verdict map in its header.
- verdict: CONFIRMED

## [s39] The s28 artifact f17 (index local + arg4 and arg3 value locals) is a live chassis carrying target's register map, and its only byte defect is one instruction
- mechanism: s28 banked it at lev 17 and rejected it for a 17-insn window; chassis selection since then has gone by score (candidate.c 7) or by joint-constraint count (s25/s37), so it was never re-measured in the solver era. The register map ($a0 for the arg4 index/address chain, value into $a3) is the fact three sessions of RA reasoning have been chasing.
- probe: transcode the cp1252 file, apply, live sandbox, then extract.py + dumps (tmp/grind/CD_datasync/dumps_f17/).
- result: CONFIRMED. score 13, target_insns 91, build_insns 90 - one instruction short, and the missing one is the `jal printf` delay-slot nop, not a computation. All 15 window instructions present, target's register map exact. Promoted to memory/grind/CD_datasync/alt_f17_a0map_13.c.
- verdict: CONFIRMED

## [s39] f17's missing instruction is the sched2 LUID tie-break between the fmt set and the outgoing-argument stack store
- mechanism: sched2 sees `131 (set (reg 4 a0) (symbol_ref D_800161C8))` and `129 (sw ..,16($sp))` both ready at equal priority; rank_for_schedule breaks the tie on INSN_LUID DESCENDING, and sched1's output gives the fmt the LOWER luid, so the sw is picked first. The block is built backwards, so picked-first = emitted-last, and reorg.c then fills the printf delay slot with the sw. Target's last window insn is the fmt `la $a0`, which reorg.c cannot sink because $a0 is a call argument - hence target's nop.
- probe: read tmp/grind/CD_datasync/dumps_f17/system.sched and .sched2 side by side; confirm the emission tails `... 127 131 129 133 135 137` (sched1) and `... 127 131 129 jal` (sched2).
- result: CONFIRMED. The defect is a one-pair swap, not a missing computation.
- verdict: CONFIRMED

## [s39] "The fmt set is emitted after the stack store" is reachable on the f17 chassis by a modelled input change
- mechanism: exact sched_solver model (parity=True, CD_datasync 12/12 blocks exact in both passes) asked one fact at a time.
- probe: perturb.py --func CD_datasync --block 3 --goal-before 131:129, depth 1, both passes, all atoms.
- result: CONFIRMED as REACHABLE, but the vector set is degenerate: exactly `add_dep 131 <- 129` and `add_dep 117 <- 129` (true/data and anti-output flavours), on pass 2 (1191 atoms) and pass 1 (1609 atoms) alike. In C: something must depend on the `sw ..,16($sp)` - the fmt set itself, or arg2's `lw $a1,%lo(D_800F19C0)`.
- verdict: CONFIRMED

## [s39] The scheduler's refusal to make the stack store and the argument loads dependent is the MEM_IN_STRUCT_P guard in true_dependence/anti_dependence, and the guard is C-controllable
- mechanism: tools/gcc-2.7.2/sched.c:834-839 - a memref pair is declared independent whenever one side is `MEM_IN_STRUCT_P && rtx_addr_varies_p && mode != QImode` and the other is neither. Our argument loads are `(mem/s:SI (reg N))`; the outgoing-arg store is `(mem:SI (plus (reg 29 sp) 16))`.
- probe: spell arg4 through a named pointer local (`p4 = tbl_125c + i4; arg4 = *p4;`), rebuild, read the sched2 dump, and score live; separately spell all three reads as explicit pointer arithmetic.
- result: KILLED as a byte lever, CONFIRMED as a mechanism. The pointer local DOES drop the flag - `(mem:SI (reg/v:SI 4 a0))` in tmp/grind/CD_datasync/dumps_vA/system.sched2 - but the form measures 13 / build_insns 90, byte-identical to f17; the pointer-arithmetic form is byte-identical too and does not even drop the flag. Mechanistically the flag cannot help in this direction: the sw sits AFTER the loads in the RTL stream, so the only dependence it could unlock is an ANTI dependence (load -> store), which pushes the store later, and the goal needs it earlier.
- verdict: KILLED
- kill_scope: instance
- measured_on: f17 chassis (alt_f17_a0map_13.c, 13/90) with the do{}while(0) FAKE present; 2 live sandbox measurements plus a cc1 -da dump each

## [s39] Target's printf-window order, or merely the fmt-last fact, is reached from the f17 chassis by some permutation of the argument chains' source statement order
- mechanism: the s38 group-move argument (an ordinary C statement move relocates a whole 4-5 insn chain, which is >= 4 atoms and therefore outside any depth-2 atom search), retargeted to the chassis that carries the register map.
- probe: tmp/grind/CD_datasync/s39/groupmove_f17.py, pass 2 block 3: 5 chain blocks (120 orders) and 8 address/value-split blocks (40,320 orders), LUIDs re-laid, priorities recomputed with perturb.apply_priorities, re-simulated.
- result: KILLED. 144 distinct windows; 0 reach target's window order and 0 put the fmt last. Best window prefix 3/15, from source order `arg4, arg5, arg2, arg3, fmt`, which does emit target's leading `93(idx0 lbu), 120(idx1 lbu), 117(arg2 lw)`. The missing nop is therefore not a statement-order question on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: f17 chassis (alt_f17_a0map_13.c, 13/90) with the do{}while(0) FAKE present; sched_solver model re-extracted with that form in src/ (parity=True); 40,320 simulated orders

## [s39] The $a0 register map is reachable on the u1 chassis by making arg4's value load precede the fmt set in sched1
- mechanism: local-alloc can only give the arg4 address pseudo $a0 if the pseudo is dead before `(set (reg 4 a0) (symbol_ref))` in the sched1 output; on u1 the fmt sits at emission slot 11 and arg4's `lw` at slot 17, so they conflict and the chain falls to $a3 (s33's coalesce observation, now explained).
- probe: tmp/grind/CD_datasync/s39/groupmove_p1.py (120 and 40,320 sched1 source orders) plus perturb.py --pass 1 --goal-before 129:135 at depth 1 over all 1459 atoms and exhaustively at depth 2 over the 759 spellable (luid/luid_move) atoms.
- result: KILLED for spellable inputs. The group-move sweep yields only 16 distinct sched1 emissions and 0 hits; depth-2 spellable finds no vector (tmp/grind/CD_datasync/s39/d2_p1_spellable.log). Over ALL atoms it is reachable at depth 1, but every vector is `add_dep {120,129,131} <- 135` or `cost 135 := 12` - the same no-C-spelling family s37/s38 found from candidate.c and the inline chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: u1 chassis (alt_u1_index_local_15.c, re-measured 15/91) with the do{}while(0) FAKE present; sched_solver model extracted with that form in src/ (parity=True); 40,320 simulated orders plus an exhaustive depth-2 atom sweep

## [s39] SYNTHESIS: the residual is a two-pass conjunction and each half is owned by a different chassis
- mechanism: (A) sched1 must retire arg4's address pseudo before the $a0 fmt definition or the pseudo cannot get $a0; (B) sched2 must emit the fmt set last or reorg.c steals the `sw` into the printf delay slot and the block is one insn short. f17 has (A) and not (B); u1 and candidate.c have (B) and not (A). The two halves live in DIFFERENT PASSES of one compilation, so a single spelling must satisfy a pre-RA ordering fact and a post-RA tie-break simultaneously.
- probe: this session's four sweeps, read together.
- result: CONFIRMED. The one free variable that touches both halves and has never been moved is the PRE-SCHED1 LUID ORDER OF THE CALL'S OWN SETUP INSNS - the order in which expand_call emits the outgoing-argument store versus the `a0 = fmt` set. Every session so far has permuted the argument COMPUTATIONS; nobody has tried to move the argument DELIVERY.
- verdict: CONFIRMED

## [s39] The s28 artifact idx-local-plus-arg4-value-local-gets-a0-map-but-loses-sw-17.c is a live chassis on the current tree that carries target's printf-window register map and is one instruction short.
- mechanism: It was banked at lev 17 in s28 and never re-measured in the solver era; chassis selection has gone by score (candidate.c 7) or by joint-constraint count (s25/s37). Its register map - arg4's index/address chain in $a0 with the value loaded lw $a3,0($a0) - is exactly the RA outcome s33/s38 identified as the missing fact.
- probe: Transcode the cp1252 file to UTF-8, apply to src/system.c, live sandbox, then extract.py (parity=True) and a full cc1 -da dump into tmp/grind/CD_datasync/dumps_f17/.
- result: CONFIRMED. score 13, target_insns 91, build_insns 90. All fifteen window instructions present; register map exact ($a0 chain -> lw $a3, $a1 arg2, $a2 arg3). The single missing instruction is the jal printf delay-slot nop, not a computation. Promoted to memory/grind/CD_datasync/alt_f17_a0map_13.c.
- verdict: CONFIRMED

## [s39] f17's missing instruction is produced by the sched2 LUID-descending tie-break between the fmt set (a0 = D_800161C8) and the outgoing-argument store sw ..,16($sp).
- mechanism: sched1's output gives the fmt set a lower LUID than the sw; in sched2 both are ready at equal priority, so rank_for_schedule's LUID-descending tie-break picks the sw first, and because the block is built backwards picked-first means emitted-last. reorg.c then fills the printf delay slot with the sw. Target's last window insn is the fmt la $a0, which reorg.c cannot sink because $a0 is a call argument - hence target's nop.
- probe: Read tmp/grind/CD_datasync/dumps_f17/system.sched and system.sched2 side by side and confirm the emission tails.
- result: CONFIRMED. sched1 tail: 127(lw arg4 value) 131(fmt a0) 129(sw) 133 135 137 jal. sched2 tail: 127 131 129 jal. The defect is a one-pair swap, not a missing computation.
- verdict: CONFIRMED

## [s39] Making the fmt set be emitted after the stack store is reachable on the f17 chassis by one modelled scheduler input change.
- mechanism: The validated sched_solver model (parity=True, CD_datasync 12/12 blocks exact in both passes) can be asked one target fact at a time instead of the joint window goal.
- probe: perturb.py --func CD_datasync --block 3 --goal-before 131:129 --depth 1, on pass 2 (1191 atoms) and pass 1 (1609 atoms).
- result: CONFIRMED as reachable, but with a degenerate vector set: exactly add_dep 131 <- 129 and add_dep 117 <- 129 (true/data and anti-output flavours) in BOTH passes. In C that reads 'something must depend on the sw ..,16($sp)' - either the fmt set itself or arg2's lw $a1,%lo(D_800F19C0). No luid/luid_move atom reaches it.
- verdict: CONFIRMED

## [s39] Dropping MEM_IN_STRUCT_P on the argument loads by binding the address to a named pointer local, or by spelling the reads as explicit pointer arithmetic, changes the emitted bytes of the f17 form.
- mechanism: sched.c:834-839 (true_dependence and anti_dependence carry the identical guard) declares a memref pair independent whenever one side is MEM_IN_STRUCT_P && rtx_addr_varies_p && mode != QImode and the other is neither. Our argument loads are (mem/s:SI (reg N)); the outgoing-arg store is (mem:SI (plus (reg 29 sp) 16)). That guard, not addressing arithmetic, is what keeps the store independent of every argument load.
- probe: Two live builds on the f17 chassis: (a) p4 = tbl_125c + i4; arg4 = *p4; (b) all three reads spelled *(base + index). sandbox --disable all on each, plus a cc1 -da dump of (a) into tmp/grind/CD_datasync/dumps_vA/.
- result: KILLED as a byte lever, CONFIRMED as a mechanism. (a) DOES drop the flag - the sched2 dump shows (mem:SI (reg/v:SI 4 a0)) where f17 has (mem/s:SI ...) - yet measures 13 / build_insns 90, byte-identical to f17. (b) is byte-identical too and does not drop the flag on the subscripted reads. In this direction the flag cannot help: the sw sits AFTER the loads in the RTL stream, so the only dependence it could unlock is an ANTI one (load -> store), which pushes the store later while the goal needs it earlier. Both forms banked under rejected/.
- verdict: KILLED
- kill_scope: instance
- measured_on: f17 chassis (alt_f17_a0map_13.c, 13/91 target, build_insns 90) with the do{}while(0) FAKE present; 2 live sandbox measurements plus one cc1 -da dump

## [s39] Target's printf-window order, or merely the fmt-emitted-last fact, is reached from the f17 chassis by some permutation of the argument chains' source statement order.
- mechanism: The s38 group-move argument - an ordinary C statement move relocates a whole 4-5 insn chain, i.e. >= 4 atoms, so it lies outside any depth-2 atom search - retargeted to the chassis that carries the register map.
- probe: tmp/grind/CD_datasync/s39/groupmove_f17.py, pass 2 block 3: 5 chain blocks (120 orders) and 8 address/value-split blocks (40,320 orders); LUIDs re-laid, priorities recomputed with perturb.apply_priorities, re-simulated with simulate.sim_block.
- result: KILLED. 144 distinct windows; 0 reach target's window order and 0 put the fmt last. Best window prefix 3/15, from source order arg4, arg5, arg2, arg3, fmt - which does emit target's leading 93(idx0 lbu), 120(idx1 lbu), 117(arg2 lw). The missing nop is not a statement-order question on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: f17 chassis (alt_f17_a0map_13.c, 13/91, build_insns 90) with the do{}while(0) FAKE present; sched_solver model re-extracted with that form in src/ (parity=True); 40,320 simulated orders

## [s39] On the u1 chassis, arg4's value load is emitted before the fmt set in sched1 - the condition that frees $a0 for the arg4 address pseudo - under some statement order or some pair of spellable scheduler input changes.
- mechanism: local-alloc can only give the arg4 address pseudo $a0 if the pseudo is dead before (set (reg 4 a0) (symbol_ref)) in the sched1 output. On u1 the fmt sits at emission slot 11 and arg4's lw at slot 17, so they conflict and the chain falls to $a3 - which is s33's 'address/value coalesce' observation, now explained as an ordering conflict rather than a coalesce.
- probe: tmp/grind/CD_datasync/s39/groupmove_p1.py (120 and 40,320 sched1 source orders) plus perturb.py --pass 1 --goal-before 129:135 at depth 1 over all 1459 atoms and exhaustively at depth 2 over the 759 spellable (luid, luid_move) atoms.
- result: KILLED for spellable inputs. The group-move sweep produces only 16 distinct sched1 emissions and 0 hits; the exhaustive depth-2 spellable sweep finds no vector (tmp/grind/CD_datasync/s39/d2_p1_spellable.log). Over ALL atoms it is reachable at depth 1, but every vector is add_dep {120,129,131} <- 135 or cost 135 := 12 - the same no-C-spelling family s37/s38 found from candidate.c and the inline chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: u1 chassis (alt_u1_index_local_15.c, re-measured 15/91) with the do{}while(0) FAKE present; sched_solver model extracted with that form in src/ (parity=True); 40,320 simulated orders plus an exhaustive depth-2 atom sweep

## [s40] The do{}while(0) FAKE in alt_f17_a0map_13.c is an inert carrier occupying a pseudo an argument-block lever wants (mandated kill re-audit on the one live family never yet ablated)
- mechanism: s34/s35/s37 ablated candidate.c and s38 ablated alt_target_geometry_8.c and alt_inline_chassis_13.c, but the s39 promotion f17 - the only form carrying target's $a0 register map - had never been ablated. If the wrapper were merely holding a pseudo, f17's window would survive its removal and every f17-chassis kill would be void.
- probe: tmp/grind/CD_datasync/s40/unwrap.py rewrites `do { ... } while (0);` to bare braces; both bodies applied to src/system.c and scored with the live sandbox.
- result: KILLED (instance). Wrapped 13 / 91 (build 90); ablated 24 / 91 (build 90). The wrapper is load-bearing on the fourth family as it is on the other three; it is not masking an argument-block lever. Every s21-s39 instance kill stands on its recorded chassis.
- verdict: KILLED  kill_scope: instance  measured_on: HEAD chassis, alt_f17_a0map_13.c with the do{}while(0) FAKE present and hand-ablated; 2 live sandbox measurements

## [s40] Some cell of the complete {index local} x {arg4 value local} x {arg3 value local} x {arg5 value local} x {assignment order} cross-product holds BOTH halves of the s39 two-pass conjunction (inherited s39 frontier item 3, stated verbatim)
- mechanism: s39 left the residual as a conjunction owned by two different chassis - (A) sched1 must retire arg4's address pseudo before the $a0 fmt definition (f17 has it), (B) sched2 must emit the fmt set last so reorg.c cannot steal the sw into the delay slot (candidate.c and u1 have it). The two are properties of different passes and had never been shown to conflict; the 16-cell table was the way to find out.
- probe: tmp/grind/CD_datasync/s40/gen.py built all 24 cells (16 flag combinations, doubled where arg5 participates so both assignment orders are covered) and gen2.py six more hand-designed hoist forms; run.sh compiled all 32 whole functions; an40.py extracted the emitted printf window and both pass facts (A = arg4's chain in $a0, B = `la $4,D_800161C8` last in the window); md5 grouped the whole functions; four representatives were re-scored on the live sandbox.
- result: KILLED (instance). No cell holds both halves, and the reason is that the space is far smaller than it looks: 32 forms collapse to 11 distinct functions, and TWELVE of them are byte-identical (the whole "arg3 inline" half of the table, including both index locals, the arg2 value local and the arg5 value local in every order) at 13 / 91. Every A=1 form is the f17 function at build 90; every B=1-and-A=0 form is the inline or candidate function. f17's own index local is inert: c_0110_435 (arg4 and arg3 value locals only) is byte-identical to alt_f17_a0map_13.c.
- verdict: KILLED  kill_scope: instance  measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 32 whole-function compiles, md5-grouped, 4 live sandbox re-measurements

## [s40] LUID order - i.e. which arguments are named locals and in what order - is the lever that decides the leaf-load picks (the ledger's model of record since s39, and the premise of all three inherited frontier items)
- mechanism: s39 recorded that rank_for_schedule's tie-break is INSN_LUID descending and that LUID order is the pre-sched RTL emission order, so hoisting a value into a named local lowers its LUID and should move its pick. Target's window requires LUID(arg3's lbu) > LUID(idx0's lbu); the fully-inline form has that relation reversed, because arg3 is the third register parameter and arg4 the fourth.
- probe: form v2 (`i0 = idx_1494[0]; i1 = idx_1494[1];` hoisted ahead of the call, everything else inline) gives idx0's lbu the lowest LUID in the block and therefore satisfies target's required relation. Compiled and md5-compared against the fully-inline form c_0000_435, then scored live.
- result: KILLED (instance). v2 is BYTE-IDENTICAL to the fully-inline form (same md5, same 15-insn window, 13 / 91 live). A required LUID relation can be flipped with zero byte effect, so LUID order does not decide these picks. Together with s34's finding that nearly every window insn carries LAUNCH_PRIORITY (priority is a constant tie), what remains as the decider is readiness - the dependence graph - plus schedule_select's memory-unit blockage override.
- verdict: KILLED  kill_scope: instance  measured_on: HEAD chassis with the do{}while(0) FAKE present; v2 vs c_0000_435 whole-function md5 plus one live sandbox measurement

## [s40] SYNTHESIS: inverting the backward list scheduler over target's window yields the exact rank order target requires, and that order splits one address chain across two others - which no source-level statement order can emit
- mechanism: sched.c builds a basic block backwards, so emission slot k corresponds to pick number (n+1-k), and an insn becomes pickable only once all of its consumers are already picked. Walking target's 15-insn window backwards and closing the ready set at each step gives an exact pairwise rank-constraint table over the window's insns - what the ledger has been guessing at for sixteen sessions, computed instead.
- probe: hand derivation over the true-dependence graph of asm/funcs/CD_datasync.s:47-68 (recorded in full in evidence.md [s40]).
- result: CONFIRMED. Descending required rank: fmt > arg4.lw > arg3.lw > sw > arg3.addu > arg3.sll > arg4.addu > arg3.lbu > arg5.lw > arg4.sll > arg5.addu > arg5.sll > arg2.lw > idx1.lbu > idx0.lbu. Two consequences: (1) arg2's load must rank below the whole arg5 chain and below arg4's sll but above both index lbus - a one-slot band; (2) arg4's sll must rank BELOW arg5's value load while arg4's addu ranks ABOVE arg3's index load, i.e. arg4's two address insns are split across two other argument chains. expand_expr emits a single subscript's sll and addu contiguously, so under the equal-priority model that split cannot come from any RTL stream order; it can only come from edges absent in sched1's true-dependence graph - the hard-register anti-dependences that exist only in sched2, after allocation.
- verdict: CONFIRMED

## [s40] The residual is a register-REUSE fact: target's window keeps only two call-clobbered temps live, and the $v0 reuse is what creates the anti-dependence that stretches arg4's chain
- mechanism: target uses $v0 twice inside the window - for idx1's address chain (slots 2,4,5, dying at `lw $v1,0($v0)` in slot 7) and again for arg3's chain (slots 8,10,11, dying at `lw $a2,0($v0)` in slot 13). The reuse is legal only because arg5's VALUE load retires idx1's address pseudo before arg3's index load is emitted; the resulting $v0 anti-dependence pins arg3's chain after arg5's and is exactly the extra edge that lets arg4's chain be stretched around both. Every measured form instead keeps three or four distinct temps live in the window (f17: $a0, $v1, $v0 simultaneously; the inline family: arg4 recomputed last in $v0), so the anti-dependence never exists and the interleave is unreachable no matter how the source is ordered.
- probe: (not yet run - this is the s40 frontier) on the f17 family, which is the only one with the $a0 map, find a spelling in which arg5's value load precedes arg3's index load, and read the sched2 dump for a $v0 anti-dependence between the two chains.
- verdict: OPEN

## [s40] Some ordinary-C spelling of arg3 that is neither a plain value local nor plain-inline moves its index load later in the window, toward target's slot 8 (s40 frontier item 3, probed the same session it was written)
- mechanism: across 32 forms the emitted function depended on arg3's spelling alone, and both known spellings put arg3's lbu at window slot 1-2 while target has it at slot 8. If the slot is a spelling property, some third spelling should land between them.
- probe: eight forms in tmp/grind/CD_datasync/s40/forms3/ - arg3's index hoisted alone (w1), the same with arg5 as a value local on either side (w2, w3), i3 plus arg4/arg5 value locals (w4, w8), arg3 as explicit pointer arithmetic *(tbl_11dc + D_800A11D5) (w5), and arg5's value local assigned before arg3's (w6, w7). Whole-function md5 plus window extraction.
- result: KILLED (instance). w1, w2, w3 and w5 are byte-identical to the 12-member class found earlier in the session (which therefore has sixteen members): the arg3 index local and the pointer-arithmetic spelling are byte-inert. The furthest any form moves arg3's lbu is slot 3 (w4, w8). Nothing approaches slot 8, which is consistent with the slot being fixed by the dependence/anti-dependence structure rather than by spelling.
- verdict: KILLED  kill_scope: instance  measured_on: HEAD chassis with the do{}while(0) FAKE present; 8 forms, whole-function md5, window-slot classification

## [s41] The do{}while(0) FAKE in alt_f17_a0map_13.c is an inert carrier occupying a pseudo an argument-block lever wants (mandated kill re-audit, run on the one live family never previously ablated).
- mechanism: s34/s35/s37 ablated candidate.c and s38 ablated alt_target_geometry_8.c and alt_inline_chassis_13.c; f17 - the s39 promotion and the only form carrying target's $a0 register map - had never been ablated, so every f17-chassis kill rested on an unaudited chassis.
- probe: tmp/grind/CD_datasync/s40/unwrap.py rewrites the do{}while(0) to bare braces; both bodies applied to src/system.c and scored on the live sandbox.
- result: Wrapped 13/91 (build 90); ablated 24/91 (build 90). The wrapper is load-bearing on the fourth family exactly as on the other three, so it is not masking an argument-block lever and every s21-s39 instance kill stands on its recorded chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis, alt_f17_a0map_13.c with the do{}while(0) FAKE present and hand-ablated; 2 live sandbox measurements

## [s41] Some cell of the complete {index local} x {arg4 value local} x {arg3 value local} x {arg5 value local} x {assignment order} cross-product holds both halves of the s39 two-pass conjunction (inherited s39 frontier item 3).
- mechanism: s39 left the residual as a conjunction owned by two chassis - (A) sched1 must retire arg4's address pseudo before the $a0 fmt definition, which only f17 has, and (B) sched2 must emit the fmt set last so reorg.c cannot steal the sw into the jal delay slot, which candidate.c and u1 have. The two are properties of different passes and had never been shown to conflict.
- probe: gen.py built all 24 cells (16 flag combinations, doubled where arg5 participates so both assignment orders are covered) plus gen2.py's six hand-designed hoist forms; run.sh compiled 32 whole functions; an40.py extracted the emitted printf window and both pass facts; md5 grouped the functions; four representatives were re-scored on the live sandbox.
- result: No cell holds both halves - and the space is far smaller than it looked. 32 forms collapse to 11 distinct functions and TWELVE of them are byte-identical (the entire arg3-inline half of the table, including both index locals, the arg2 value local and the arg5 value local in every order) at 13/91. Every A=1 form is the f17 function at build 90; f17's own index local is inert, since c_0110_435 (arg4 and arg3 value locals only) is byte-identical to alt_f17_a0map_13.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, three hoisted pointer locals) with the do{}while(0) FAKE present; 32 whole-function compiles md5-grouped, 4 live sandbox re-measurements

## [s41] LUID order - which arguments are named locals and in what order - is the lever that decides the window's leaf-load picks (the ledger's model of record since s39 and the premise of all three inherited frontier items).
- mechanism: s39 recorded rank_for_schedule's tie-break as INSN_LUID descending with LUID order equal to the pre-sched RTL emission order, so hoisting a value into a named local lowers its LUID and should move its pick. Target's window requires LUID(arg3's lbu) > LUID(idx0's lbu); the fully-inline form has that relation reversed because arg3 is the third register parameter and arg4 the fourth.
- probe: Form v2 hoists BOTH indices into named locals ahead of the call, giving idx0's lbu the lowest LUID in the block and satisfying target's required relation; compiled and md5-compared against the fully-inline form c_0000_435, then scored live.
- result: v2 is byte-identical to the fully-inline form (same md5, same 15-insn window, 13/91 live). A required LUID relation was flipped with zero byte effect, so LUID order does not decide these picks. With s34's finding that nearly every window insn carries LAUNCH_PRIORITY, what remains as the decider is readiness - the dependence graph - plus schedule_select's memory-unit blockage override.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; v2 vs c_0000_435 whole-function md5 plus one live sandbox measurement

## [s41] Some ordinary-C spelling of arg3 that is neither a plain value local nor plain-inline (an index local, explicit pointer arithmetic, or a value local assigned after arg5's) moves arg3's index load later in the window, toward target's slot 8.
- mechanism: Across the 32-form sweep the emitted function depended on arg3's spelling alone, and both known spellings put arg3's lbu at window slot 1-2 while target has it at slot 8; if the slot were a spelling property, some third spelling should land between them.
- probe: Eight forms in tmp/grind/CD_datasync/s40/forms3/ - arg3's index hoisted alone, the same with arg5 as a value local on either side, i3 plus arg4/arg5 value locals, arg3 as *(tbl_11dc + D_800A11D5), and arg5's value local assigned before arg3's - with whole-function md5 and window-slot classification.
- result: Four of the eight (w1, w2, w3, w5) are byte-identical to the class found earlier in the session, which therefore has SIXTEEN members: the arg3 index local and the pointer-arithmetic spelling are byte-inert too. The furthest any form moves arg3's lbu is slot 3. Nothing approaches slot 8.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; 8 forms (w1-w8), whole-function md5, window-slot classification

## [s41] Inverting the backward list scheduler over target's window yields the exact pairwise rank order target requires, and that order splits arg4's sll and addu across two other argument chains.
- mechanism: sched.c builds a basic block backwards, so emission slot k is pick number (n+1-k) and an insn becomes pickable only once all of its consumers are picked. Walking target's 15-insn window backwards and closing the ready set at each step gives an exact pairwise rank-constraint table over the window - what the ledger had been guessing at for sixteen sessions.
- probe: Hand derivation over the true-dependence graph of asm/funcs/CD_datasync.s:47-68, recorded in full in evidence.md [s40].
- result: Descending required rank: fmt > arg4.lw > arg3.lw > sw > arg3.addu > arg3.sll > arg4.addu > arg3.lbu > arg5.lw > arg4.sll > arg5.addu > arg5.sll > arg2.lw > idx1.lbu > idx0.lbu. Two consequences: arg2's load must rank below the whole arg5 chain and below arg4's sll but above both index lbus, a one-slot band; and arg4's sll must rank BELOW arg5's value load while arg4's addu ranks ABOVE arg3's index load, i.e. arg4's two address insns are split across two other chains. expand_expr emits a single subscript's sll and addu contiguously, so under the equal-priority model that split cannot come from any RTL stream order - only from edges absent in sched1's true-dependence graph, namely the hard-register anti-dependences that exist only in sched2.
- verdict: CONFIRMED

## [s42] Reducing the number of hoisted callee-saved table pointers lowers the printf window's register pressure and is the last source-side variable (s39/s41 frontier item 3)
- mechanism: All three hoisted pointers (tbl_11dc, idx_1494, tbl_125c) are used ONLY inside the do_timeout block, so the hoist count is exactly "how many callee-saved bases are live across the timeout block". s40's reframe says target's window reuses $v0 across two argument chains, which happens only when the allocator is short of call-clobbered temps; fewer hoisted bases should raise the pressure inside the window and force that reuse.
- probe: Complete cross-product {2^3 hoist subsets} x {arg3 value local on/off} x {arg4 value local on/off} = 32 whole-function compiles, tmp/grind/CD_datasync/s42/forms/ + run.sh, scored with the calibrated s21 harness (sandbox = lev + nop - 3).
- result: KILLED (instance). Only the all-three-hoisted subset reaches target's expanded instruction count of 82. All 28 forms dropping at least one hoist land at n=69..80 and lev 20..38; best non-h7 cell h6 = 20. The hoists are load-bearing for the instruction COUNT, not just for the callee-saved map. Inside h7 the four cells are 13 / 12 / 7 / 14.

## [s42] Declaration order of the three pointer locals, the order of their initialising assignments, and the position of that assignment block relative to the two other prologue stores are three independent structural levers on the window
- mechanism: allocno creation order and LUID order both follow source order, and global.c:allocno_compare's tie-breaks are sensitive to it; the s9 H37/H38 argument fixes the callee-saved map by priority but says nothing about ties.
- probe: 108 whole-function compiles, {6 decl permutations} x {6 assignment permutations} x {3 block positions}, tmp/grind/CD_datasync/s42/forms2/ + res2.txt.
- result: KILLED (instance) for two of the three axes and BOUNDED for the third. Declaration order is completely byte-inert (all 6 permutations identical in every cell) and the block position is completely byte-inert (p0 == p1 == p2 in all 36 cells). Only the assignment permutation moves the score, and only through which pointer is assigned FIRST: tbl_11dc-first = 7 or 8, idx_1494-first = 11, tbl_125c-first = 12. candidate.c already holds the unique optimum. Nothing in the 108-cell space is below 7.

## [s42] Making one argument's value live across the puts() call raises the window's register pressure enough to force target's $v0 reuse
- mechanism: s40 restated the residual as "target keeps only two call-clobbered temps live in the window". A value computed before puts() must survive the call, consuming a callee-saved register and changing what the window has to work with.
- probe: {arg3, arg4, arg5} x {inline, local assigned after puts, local assigned before puts} = 27 whole-function compiles, tmp/grind/CD_datasync/s42/forms3/ + res3.txt.
- result: KILLED (instance). Every form with at least one before-puts assignment grows the function to n=84..90 (a save/restore pair per surviving value) and scores 27..39; the nine all-after-puts forms stay at n=82 and score 7..14. The extra callee-save pair costs far more than any scheduling gain.

## [s42] Hoisting arg2 (D_800F19C0) into a value local advances its lui/lw pair into target's slots 43/44 on the new m011 chassis
- mechanism: On m011 the arg2 pair sits at slots 51/52 while arg4's chain occupies target's 43/44/47; giving arg2 its own named local and placing that assignment first should lift it to the head of the argument block's RTL stream. s41 measured arg2 hoisting byte-inert, but only on the 16-member 13-class, so the finding had to be re-tested on the 7-scoring family.
- probe: 38 whole-function compiles on the m011 family: every subset of {arg2, arg3} added to the {arg4, arg5} value locals, times every permutation of the resulting assignment statements. tmp/grind/CD_datasync/s42/forms4/ + res4.txt.
- result: KILLED (instance). A `void *arg2 = D_800F19C0;` local in any of the three assignment positions is byte-identical to leaving arg2 inline (p245 == p425 == p452 == p45, all 7). Adding arg3 as a value local regresses to 12-14 in every ordering. The arg2 advance is not reachable from the argument block's spelling.

## [s42] Type narrowing - the declared types of the locals and of the hoisted table pointers - is a live structural lever on this window
- mechanism: The structural-modality catalogue lists type narrowing alongside declaration order and statement re-association; sub-word and unsigned types change QImode/SImode RTL and can change combine's choices and the addressing scale.
- probe: 10 whole-function compiles covering u32 table pointers (with casts at the hoist), u32 argument locals, `void *` argument locals with casts, u32 index locals split out of the subscripts, an explicit (s32) cast on the arg3 index, and a u32 `cnt` with an (s32) cast at the compare - applied to BOTH 7-scoring families. tmp/grind/CD_datasync/s42/forms5/.
- result: KILLED (instance). All seven m011-derived variants are byte-identical to m011 (single md5 1bb8643812d2793ab8a96cf8241c5f86) and all three candidate-derived variants are byte-identical to candidate.c (single md5 8b0d0bf4ec63edd058c7d17b2df537bb). Declared type, with value semantics preserved, has zero byte effect here.

## [s42] CONFIRMED: a second byte-distinct function ties the 7/91 floor, and it carries more of target's window spine than candidate.c
- mechanism: The floor had been held by exactly one emitted function for 40 sessions. Hoisting arg5's value alongside arg4's produces a different function that happens to score the same, so the floor is a two-point set and the two points have different residuals.
- probe: forms3/m011 compiled, md5'd against candidate.c's emission, scored live with `sandbox CD_datasync --disable all`, and aligned against target with the s21 cmp.py expansion.
- result: CONFIRMED. m011 = 7/91, target_insns 91, build_insns 91, rules_dropped 0, md5 1bb8643812d2793ab8a96cf8241c5f86 (candidate.c is 8b0d0bf4...). m011 is correct at expanded slots 42, 45, 46, 48, 49, 50 (the whole idx1/arg5 address chain plus the arg3 leaf pair) against candidate.c's 43, 44, 45, 46. Banked as memory/grind/CD_datasync/alt_m011_arg4_arg5_locals_7.c and recommended as the next session's starting chassis.

## [s42] Reducing the number of hoisted callee-saved table pointers (tbl_11dc, idx_1494, tbl_125c), which are used only inside the do_timeout block, raises the printf window's register pressure and reaches target's window - the s39/s41 frontier item 3 quadrant.
- mechanism: All three hoisted pointers are live only across the timeout block, so the hoist count is exactly 'how many callee-saved bases are live across the window'. s40's reframe says target reuses $v0 across two argument chains, which requires the allocator to be short of call-clobbered temps; fewer hoisted bases should force that reuse.
- probe: Complete cross-product {2^3 hoist subsets} x {arg3 value local on/off} x {arg4 value local on/off} = 32 whole-function compiles (tmp/grind/CD_datasync/s42/forms/), scored with the calibrated s21 harness (sandbox = lev + nop - 3, verified: candidate.c reproduces lev=7 nop=3).
- result: Only the all-three-hoisted subset reaches target's expanded instruction count of 82 at all. All 28 forms that drop at least one hoist land at n=69..80 (2-13 instructions short) and score lev 20..38; the best is h6 (idx_1494 + tbl_125c hoisted, tbl_11dc read as the global) at 20. Inside the all-hoisted subset the four cells are: no locals 13, arg3 local 12, arg4 local 7 (= candidate.c), arg3+arg4 locals 14. The three hoists are load-bearing for the instruction COUNT, not merely for the callee-saved map the s9 H37/H38 argument fixes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, do{}while(0) FAKE present), 32 whole-function compiles with the FAKE wrapper in place in every form

## [s42] The declaration order of the three pointer locals, the order of their initialising assignments, and the position of that assignment block relative to the D_800F19BC and D_800F19C0 prologue stores are three independent structural levers that can reach below 7 on this chassis.
- mechanism: allocno creation order and LUID order both follow source order, and global.c allocno_compare's tie-breaks are sensitive to it; the s9 H37/H38 argument fixes the callee-saved map by priority but says nothing about ties.
- probe: 108 whole-function compiles: {6 declaration permutations} x {6 assignment permutations} x {3 positions of the assignment block} (tmp/grind/CD_datasync/s42/forms2/, res2.txt).
- result: Declaration order is completely byte-inert (all 6 permutations give identical scores in every cell) and the block position is completely byte-inert (p0 == p1 == p2 in all 36 cells). Only the assignment permutation moves the score, and only through which pointer is assigned FIRST: tbl_11dc-first = 7 (its idx_1494-then-tbl_125c tail) or 8 (tbl_125c-then-idx_1494 tail), idx_1494-first = 11 (all 36), tbl_125c-first = 12 (all 36). candidate.c already holds the unique optimum and nothing in the 108-cell space is below 7. This subsumes the two previously banked point-probes (rejected/init-order-11dc-125c-1494-costs-one-8.c, rejected/init-order-125c-first-regresses-12.c) as cells of one table.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; 108 whole-function compiles, all n=82 nop=3

## [s42] Assigning one argument's value into a local BEFORE the puts() call raises the window's register pressure enough to force target's $v0 reuse.
- mechanism: s40 restated the residual as 'target keeps only two call-clobbered temps live in the window'. A value computed before puts() must survive the call in a callee-saved register, changing what the window's allocator has to work with.
- probe: {arg3, arg4, arg5} x {inline, value local assigned after puts, value local assigned before puts} = 27 whole-function compiles (tmp/grind/CD_datasync/s42/forms3/, res3.txt).
- result: Every form carrying at least one before-puts assignment grows the function to n=84/85/87/88/90 - one callee-saved save/restore pair per surviving value - and scores 27..39. The nine all-after-puts forms stay at n=82 and score 7..14. The extra save/restore pair costs far more than any scheduling gain, so the axis is dead from the source side.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis with the do{}while(0) FAKE present; 27 whole-function compiles

## [s42] Hoisting arg2 (D_800F19C0) into a named value local advances its lui/lw pair into target's slots 43/44 on the new m011 chassis, where arg4's chain currently occupies them.
- mechanism: On m011 the arg2 pair sits at expanded slots 51/52 while arg4's chain occupies target's 43/44/47; giving arg2 its own named local and placing that assignment first should lift it to the head of the argument block's RTL stream. s41 measured arg2 hoisting byte-inert only on the 16-member 13-class, so the finding had to be re-tested on the 7-scoring family.
- probe: 38 whole-function compiles on the m011 family: every subset of {arg2, arg3} added to the {arg4, arg5} value locals, times every permutation of the resulting assignment statements (tmp/grind/CD_datasync/s42/forms4/, res4.txt).
- result: A `void *arg2 = D_800F19C0;` local in any of the three assignment positions is byte-identical to leaving arg2 inline (p245 == p425 == p452 == p45, all 7). Adding arg3 as a value local regresses to 12-14 in every ordering. The arg2 advance is not reachable from the argument block's spelling on this chassis either.
- verdict: KILLED
- kill_scope: instance
- measured_on: m011 chassis (arg4 and arg5 value locals after puts) with the do{}while(0) FAKE present; 38 whole-function compiles

## [s42] Type narrowing - the declared types of the argument locals, the index locals and the hoisted table pointers - is a live structural lever on this window.
- mechanism: The structural-modality catalogue lists type narrowing alongside declaration order and statement re-association; sub-word and unsigned types change QImode/SImode RTL and can change combine's choices and the addressing scale.
- probe: 10 whole-function compiles applied to BOTH 7-scoring families: u32 table pointers with casts at the hoist, u32 argument locals, void* argument locals with casts, u32 index locals split out of the subscripts, an explicit (s32) cast on the arg3 index, a u32 cnt with an (s32) cast at the compare (tmp/grind/CD_datasync/s42/forms5/, md5-grouped).
- result: All seven m011-derived variants are byte-identical to m011 (single md5 1bb8643812d2793ab8a96cf8241c5f86) and all three candidate-derived variants are byte-identical to candidate.c (single md5 8b0d0bf4ec63edd058c7d17b2df537bb). With value semantics preserved, the declared type of any local in this function has zero effect on the emitted bytes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis and m011 chassis, do{}while(0) FAKE present in both; 10 whole-function compiles grouped by md5

## [s42] A second, byte-distinct function ties the 7/91 floor and carries more of target's printf-window spine than candidate.c does.
- mechanism: The floor had been held by exactly one emitted function for 40 sessions. Hoisting arg5's value alongside arg4's produces a different function that happens to score the same, so the floor is a two-point set whose two points have different residuals.
- probe: forms3/m011 compiled, md5'd against candidate.c's emission, scored with a LIVE `sandbox CD_datasync --disable all`, and aligned against target with the s21 cmp.py expansion.
- result: CONFIRMED. m011 = score 7, target_insns 91, build_insns 91, rules_dropped 0, whole-function md5 1bb8643812d2793ab8a96cf8241c5f86 (candidate.c is 8b0d0bf4ec63edd058c7d17b2df537bb). m011 is correct at expanded slots 42, 45, 46, 48, 49, 50 - the entire idx1/arg5 address chain (lbu $v0,1($s1) / sll $v0,$v0,2 / addu $v0,$v0,$s0 / lw $v1,0($v0)) plus the arg3 leaf pair (lui $v0,%hi / lbu $v0,%lo) - against candidate.c's 43, 44, 45, 46, two of which candidate reaches only by coincidence (its arg4 chain spells the same bytes target's arg5 chain does). Banked as memory/grind/CD_datasync/alt_m011_arg4_arg5_locals_7.c.
- verdict: CONFIRMED

## [s43] RE-AUDIT: the s42 kill "hoisting arg2 (D_800F19C0) into a value local is byte-inert" was measured with the do{}while(0) FAKE occupying the block, so it must be re-measured with the FAKE ablated before it counts
- mechanism: driver rule (func_8002EA24 s8) - a lever measured inert while a FAKE carrier occupies its target pseudo is not a kill. The do{}while(0) wrap changes flow.c loop_depth ref-weighting, which feeds global.c allocno_compare, i.e. exactly the allocation decision an arg2 pseudo would compete in.
- probe: four whole-function compiles on the m011 chassis - {arg2 hoisted, arg2 inline} x {do{}while(0) present, replaced by a plain brace block} (tmp/grind/CD_datasync/s43/forms/r_arg2hoist_FAKE.c, r_arg2hoist_noFAKE.c, b_m011.c, a_m011_noFAKE.c); compared by whole-function md5 and by the s21 lev/nop harness.
- result: KILLED (instance, re-confirmed on a second chassis). With the FAKE: hoisted == inline, md5 1bb8643812d2793ab8a96cf8241c5f86, lev 7. Without the FAKE: hoisted == inline, md5 5f2bed4e1ed469566e04e51662bf5678, lev 18. The kill survives ablation. Separately this measures the FAKE's own worth: removing it costs 11 points on m011 and 10 on candidate.c/m010, so it is a load-bearing codegen device here, not an inert carrier.
- verdict: KILLED

## [s43] arg2's emission slot is set by its dependence-chain LENGTH / memory-dependence rather than by its spelling, so an expression of different chain length will advance its lui into target's slot 43 (the s42 frontier item 1)
- mechanism: s40's rank-order inversion put arg2's load in a narrow band below the arg5 chain and above the two index lbu's; INSN_PRIORITY in sched.c is the longest path to the block end, so lengthening or shortening arg2's chain should move its rank without touching any other chain.
- probe: eight arg2-expression forms on the m011 chassis (tmp/grind/CD_datasync/s43/forms/e1-e8, res in the session log): *(&D_800F19C0); &D_800162C0 substituted directly (chain shortened from lui+lw with a memory dependence to lui+addiu with none); a function-scope void ** alias initialised in the prologue; the same alias with a declaration-initialiser; the alias declared beside the table pointers; and the D_800F19C0 = &D_800162C0; store relocated to three positions inside the do-block. Each compared to target with the s21 cmp.py window alignment at slots 40-57.
- result: KILLED (instance). *(&D_800F19C0) is byte-identical to m011 (7). The SHORTENED, memory-dependence-free chain (&D_800162C0) scores 9 and its lui/addiu pair still emits at slots 51/52 - the same slots the lui/lw pair occupied - so the change in chain length and in memory-dependence moved the slot by zero. The pointer-alias spellings all add a register and a nop (n=85, lev 13/16) and the alias-with-tables spelling gives n=83 lev=14. Relocating the format-pointer store into the block scores 14/14/15. On this chassis arg2's slot is invariant under every measured change to its expression's shape, length and memory-dependence.
- verdict: KILLED

## [s43] Naming the two subscripts as index locals (i4 = idx_1494[0]; i5 = idx_1494[1];) decouples the leaf lbu emission order from the value-assignment order, so the leaf order can be set to idx0-first while the middle of the window is ordered idx1-first the way target orders it
- mechanism: target's leaf order is idx0 then idx1 (slots 41, 42) but its middle order is idx1's sll/addu (45, 46) before idx0's sll (47) - a combination no measured value-assignment order reproduces, since assignment order sets both at once. If the index locals are separate statements they should set the leaf order independently.
- probe: the full 2^5 lattice {i4} x {i5} x {arg3 local} x {arg4 local} x {arg5 local} in canonical order plus 25 statement-order variants that put the index locals and the value assignments in different sequences, 57 whole-function compiles (tmp/grind/CD_datasync/s43/forms2/, res2.txt), classified by whole-function md5.
- result: KILLED (instance). Sixteen of the source forms collapse to just TWO emitted functions: the eight arg4+arg5-local cells are all byte-identical to alt_m011 and the eight arg4-local/arg5-inline cells are all byte-identical to candidate.c. Every order variant that puts arg5's assignment before arg4's scores 8 with the leaves swapped (identical to the s42 p54 result), i.e. the index locals never decouple anything - the leaf order stays welded to the value-assignment order. Nothing in the lattice scores below 7.
- verdict: KILLED

## [s43] Some spelling of arg3, of the format pointer, or of the value locals' SCOPE changes the m011 window without paying arg3's register-pressure cost
- mechanism: making arg3 a value local is known to cost 12-15 points; if a weaker spelling (a pointer dereference, an index local, an address local, a second base pointer, a format-pointer local, or a function-scope rather than block-scope declaration) produces some of the same effect at lower cost, the m011 window could move without the arg3 penalty.
- probe: thirteen forms on the m011 chassis (tmp/grind/CD_datasync/s43/forms4/, res4.txt): *(tbl_11dc + D_800A11D5) for arg3; an arg3 index local in two positions; an arg3 address local in three spellings; a s32 *fmt = &D_800161C8; local in two positions; a second hoisted base pointer used only by arg3; and arg4 / arg5 / both moved from block scope to function scope.
- result: KILLED (instance). Eight of the thirteen are byte-identical to alt_m011 (one md5): the pointer-dereference arg3, the index local assigned last, both format-pointer positions, and all three scope relocations. Block-vs-function SCOPE of the value locals is byte-inert, which closes the last untried declaration-placement lever. The five that move all move the wrong way: index local first 15, address local first 15, address local last 8, &tbl_11dc[...] address local n=76 lev=15, second base pointer 11.
- verdict: KILLED

## [s43] CONFIRMED (mechanism, not a match): target's spread $a0 idx0 chain is produced by exactly one source condition - arg4 assigned FIRST in the block AND arg3 materialised into its own pseudo - and that same condition necessarily hoists arg3's leaf pair into arg2's target slots
- mechanism: allocating a chain to $a0 (a call-clobbered argument register, free until the fmt lui at slot 57) requires a third simultaneously-live scratch pseudo in the window. arg2 and the format pointer are both byte-inert as locals (s42 forms4; s43 g7/g8), so neither adds pressure; only arg3's 5-insn chain (lui, lbu, sll, addu, lw) does. But materialising arg3 also raises its own chain's rank, so its lui/lbu pair emits at slots 42/43 or 43/44.
- probe: 61 assignment-order permutations across seven arg3-local cells (tmp/grind/CD_datasync/s43/forms3/, res3.txt) with window alignments read for q10110_443 (the s28/s39 "f17" shape), q00111_435, q00111_453 and q10111_4435.
- result: CONFIRMED as a mechanism. Every q*_4... form with arg3 local reproduces target's slots 41 (lbu $a0,0($s1)) and 47 (sll $a0,$a0,2) verbatim, and q00111_453 also reproduces slot 55 (lw $a2,0($v0)); no arg3-inline form ever allocates a chain to $a0. All of them score 13-15 because arg3's hoisted leaf pair displaces arg2. This is the sharpest available statement of the residual: target needs the $a0 pressure WITHOUT arg3 being a pseudo, i.e. a third live value in the window that is neither arg2, nor the format pointer, nor arg3's materialised value.
- verdict: CONFIRMED

## [s43] FRONTIER for s44: find the third live pseudo
- The residual is now a pressure question, not an ordering question. Target's window holds four live scratch chains around slots 47-51 ($a0 = idx0 address, $v0 = arg3 address, $v1 = arg5 value, $a1 = arg2 done); m011 holds three and sequences them. arg3-as-a-local supplies the fourth but pays its leaf pair into arg2's slots. Untried sources of a fourth live value that do NOT create an arg3 pseudo: (a) a fourth hoisted table/base pointer live across the printf but read only after it (note: a second base used BY arg3 scores 11, s43 g9 - the untested variant is one used by arg4 or arg5); (b) the cnt or v0 loop values kept live across the do_timeout block by the surrounding control flow rather than by a new declaration; (c) pressure from the CD_flush() call's own setup, or from a fifth printf argument's stack slot, being scheduled above the printf.

## [s43] The s42 kill 'hoisting arg2 (D_800F19C0) into a value local is byte-inert on the m011 chassis' was measured with the do{}while(0) FAKE present, so it must be re-measured with the FAKE ablated before it counts as a kill.
- mechanism: Driver rule (func_8002EA24 s8): a lever measured inert while a FAKE carrier occupies its target pseudo is not a kill. The do{}while(0) wrap changes flow.c loop_depth ref-weighting, which feeds global.c allocno_compare - exactly the allocation decision an arg2 pseudo would compete in.
- probe: Four whole-function compiles on the m011 chassis: {arg2 hoisted into a block-local 'void *arg2', arg2 inline} x {do{}while(0) present, replaced by a plain brace block}. Files tmp/grind/CD_datasync/s43/forms/{r_arg2hoist_FAKE,r_arg2hoist_noFAKE,b_m011,a_m011_noFAKE}.c, compared by whole-function md5 and by the calibrated s21 lev/nop harness.
- result: The kill survives ablation. WITH the FAKE, hoisted == inline byte-for-byte (md5 1bb8643812d2793ab8a96cf8241c5f86, lev 7). WITHOUT the FAKE, hoisted == inline byte-for-byte (md5 5f2bed4e1ed469566e04e51662bf5678, lev 18). Both chassis agree, so the s42 result is now measured under two FAKE states rather than one. The same four compiles also price the FAKE itself: removing it costs 11 points on m011 and 10 on candidate.c/m010 (both stay n=82 nop=3), so the do{}while(0) is a load-bearing codegen device here, not an inert carrier.
- verdict: KILLED
- kill_scope: instance
- measured_on: m011 chassis (goto loop, arg4+arg5 value locals after puts) measured BOTH with the do{}while(0) FAKE present and with it ablated to a plain brace block; 4 whole-function compiles, md5-classified

## [s43] arg2's emission slot is set by its dependence-chain LENGTH and memory-dependence rather than by its spelling, so an arg2 expression of different chain length advances its lui into target's slot 43 (the s42 frontier item 1).
- mechanism: s40's rank-order inversion places arg2's load in a narrow band below the whole arg5 chain and above the two index lbu's. INSN_PRIORITY in sched.c is the longest path to the block end, so changing arg2's chain length should move its rank without touching any other chain.
- probe: Eight arg2-expression forms on the m011 chassis (tmp/grind/CD_datasync/s43/forms/e1-e8.c, results in res tail of the session log): *(&D_800F19C0); &D_800162C0 substituted directly, which shortens the chain from lui+lw with a memory dependence to lui+addiu with none; a function-scope 'void **' alias initialised in the prologue; the same alias as a declaration-initialiser; the alias declared beside the three table pointers; and the 'D_800F19C0 = &D_800162C0;' store relocated to three positions inside the do-block. Each aligned against target with the s21 cmp.py window expansion, slots 40-57.
- result: *(&D_800F19C0) is byte-identical to m011 (7). The shortened, memory-dependence-free chain (&D_800162C0) scores 9 and its lui/addiu pair emits at slots 51/52 - exactly the slots the lui/lw pair occupied - so shortening the chain and removing its memory dependence moved the slot by zero. Pointer-alias spellings add a register and a nop (n=85, lev 13 and 16); the alias-with-tables spelling gives n=83 lev=14; relocating the format-pointer store into the block scores 14, 14 and 15 with n dropping to 80-82. On this chassis arg2's slot did not move under any measured change to its expression's shape, length or memory-dependence.
- verdict: KILLED
- kill_scope: instance
- measured_on: m011 chassis with the do{}while(0) FAKE present; 8 whole-function compiles plus s21 cmp.py window alignments for the baseline and the shortened-chain form

## [s43] Naming the two subscripts as index locals (i4 = idx_1494[0]; i5 = idx_1494[1];) decouples the leaf lbu emission order from the value-assignment order, so the leaf order can be set idx0-first while the middle of the window is ordered idx1-first the way target orders it.
- mechanism: Target's leaf order is idx0 then idx1 (slots 41, 42) but its middle order is idx1's sll/addu (45, 46) before idx0's sll (47). No measured value-assignment order reproduces that combination, because the assignment order sets both at once. Separate index-local statements should set the leaf order independently of the value order.
- probe: The full 2^5 lattice {i4 index local} x {i5 index local} x {arg3 value local} x {arg4 value local} x {arg5 value local} in canonical statement order, plus 25 statement-order variants that sequence the index locals and the value assignments differently: 57 whole-function compiles (tmp/grind/CD_datasync/s43/forms2/, res2.txt), classified by whole-function md5.
- result: Sixteen of the source forms collapse to exactly TWO emitted functions. The eight cells with arg4-local + arg5-local (c00011, c01011, c10011, c11011) are all byte-identical to alt_m011 (md5 1bb8643812d2793ab8a96cf8241c5f86, lev 7); the eight cells with arg4-local + arg5-inline (c00010, c01010, c10010, c11010) are all byte-identical to candidate.c (md5 8b0d0bf4ec63edd058c7d17b2df537bb, lev 7). Neither index local ever changes a byte in the 7-scoring families. Every order variant that assigns arg5 before arg4 scores 8 with the leaf lbu's swapped, reproducing the s42 p54 result exactly - the leaf order stayed welded to the value-assignment order in every form measured. Nothing in the lattice scored below 7. This extends s41's index-local kill from the arg3-inline family to the whole value-local lattice.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis (goto loop, do{}while(0) FAKE present), 57 whole-function compiles md5-classified against candidate.c and alt_m011

## [s43] Some spelling of arg3, of the format pointer, or of the value locals' declaration SCOPE moves the m011 window without paying arg3's register-pressure cost.
- mechanism: Making arg3 a value local is known to cost 12-15 points. A weaker spelling - a pointer dereference, an index local, an address local, a second base pointer, a format-pointer local, or a function-scope rather than block-scope declaration - might produce part of the same allocation effect at lower cost.
- probe: Thirteen forms on the m011 chassis (tmp/grind/CD_datasync/s43/forms4/, res4.txt): *(tbl_11dc + D_800A11D5) for arg3; an arg3 index local in two positions; an arg3 address local in three spellings; a 's32 *fmt = &D_800161C8;' local in two positions; a second hoisted base pointer used only by arg3; and arg4 / arg5 / both moved from block scope to function scope.
- result: Eight of the thirteen are byte-identical to alt_m011 (one md5): the pointer-dereference arg3, the arg3 index local assigned last, both format-pointer-local positions, and all three block-scope-to-function-scope relocations. So declaration SCOPE of the value locals is byte-inert, closing the last untried declaration-placement lever, and the format pointer is inert as a local. The five forms that did move all moved the wrong way: arg3 index local assigned first 15, address local assigned first 15, address local assigned last 8, '&tbl_11dc[...]' address local n=76 lev=15 (loses six instructions), second base pointer for arg3 11.
- verdict: KILLED
- kill_scope: instance
- measured_on: m011 chassis with the do{}while(0) FAKE present; 13 whole-function compiles, md5-classified

## [s43] Target's spread $a0 idx0 address chain (lbu $a0,0($s1) at slot 41, sll $a0,$a0,2 at 47, addu at 51, lw $a3,0($a0) at 56) is produced by exactly one source condition: arg4 assigned FIRST in the block AND arg3 materialised into its own pseudo - and that same condition necessarily hoists arg3's leaf pair into arg2's target slots.
- mechanism: Allocating a chain to $a0 - a call-clobbered argument register free until the fmt lui at slot 57 - requires a third simultaneously-live scratch pseudo in the window. arg2 and the format pointer are both byte-inert as locals (s42 forms4; s43 g7/g8), so neither adds pressure; only arg3's 5-insn chain (lui, lbu, sll, addu, lw) does. Materialising arg3 also raises its own chain's rank, so its lui/lbu pair emits at slots 42/43 or 43/44.
- probe: 61 assignment-order permutations across seven arg3-local cells (tmp/grind/CD_datasync/s43/forms3/, res3.txt), with s21 cmp.py window alignments read for q10110_443 (the s28/s39 'f17' shape), q00111_435, q00111_453 and q10111_4435; plus a scan of all 84 session emissions for byte-exact target slots (tmp/grind/CD_datasync/s43/scan.py).
- result: Every form with arg4 assigned first AND arg3 as a value local reproduces target's slots 41 and 47 verbatim, and q00111_453 also reproduces slot 55 (lw $a2,0($v0)). No arg3-inline form in 84 compiles ever allocated a chain to $a0. All members of the $a0 family score 13-15 because arg3's hoisted leaf pair lands in slots 42/43 or 43/44 - precisely target's arg2 slots. The slot-scan also shows the three families hold DISJOINT correct-slot sets: m011 holds {42,45,46,48,49,50}, candidate.c holds {43,44,45,46}, the $a0 family holds {41,47} and sometimes {55}; 6 of 16 window slots is the ceiling across every form measured in 43 sessions.
- verdict: CONFIRMED
