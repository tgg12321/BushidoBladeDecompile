# Evidence bank — func_800858D0

- WIP rejected_form: {'form': 't for the 0x18 only', 'score': 27, 'reason': 'stays single-set, still hoisted'}

- WIP rejected_form: {'form': 't for the 1 only', 'score': 27, 'reason': 'stays single-set, still hoisted'}

- WIP rejected_form: {'form': 'reuse the existing `offset` variable for the 1', 'score': 20, 'reason': 'frame 96 but regs=4, insns=74'}

- WIP rejected_form: {'form': 'offset reuse for 1 + t for 0x18', 'score': 20, 'reason': 'regs=4, insns=74'}

- WIP rejected_form: {'form': 's16 t instead of s32 t', 'score': 16, 'reason': 'better metric but structurally worse: regs=4, insns=74. Not the resume base.'}

- WIP rejected_form: {'form': 'named sign-extend local (s32 idx / s16 idx) reused as the shift amount', 'score': 22, 'reason': 'inert'}

- WIP rejected_form: {'form': 't placement: loop top / immediately before each use / 0x18 store last / shift before stores / idx+shift-first', 'score': 22, 'reason': 'all inert'}

- == imported from memory/wip notes.md ==
# title_mv_exec2 — WIP (current state 2026-08-05, sched_solver goal-mapper)
`src/main.c:879`. Builds a 64-byte GPU/primitive buffer on the stack, then for
each of `D_80101BCC` entries writes six s16 fields at `entry * 54` byte stride
into the `D_800F4E1x` block, sets `buf[0] = 1 << i`, and calls
`saTan1MainJump(buf)` + `motutil_GetAngTableNum(1)`.
18 regfix rules; 0 asmfix; no prologue_config / frame_fix / maspsx-gate entries.

## Where it stands

| body | score | frame | saves | build_insns |
|---|---|---|---|---|
| committed HEAD | 27 | 104 | 5 | 76 |
| **`candidate.c`** | **22** | **96** | **3** | **72** |
| target | 0 | 96 | 3 | 72 |

Frame, saves and insn count all match target; the 22 is register naming.

## The lever — [[defeat-licm-hoist-var-reuse]]

**Never a phantom-frame problem.** Both have `vars=64`; we emitted **5
callee-saves against target's 3**. The extra saves were loop-invariant constants
`loop.c` hoisted (`li s3,24`, `li s2,1`); target rematerialises both in the loop.
Routing both through ONE scratch local (`s32 t; t = 0x18;` before the six stores,
`t = 1;` before `buf[0] = t << (s16)var_s0;`) makes the pseudo multi-set, so it
is not a `move_movables` candidate. GCC rotates the `t = 0x18` reload into the
loop-back branch's delay slot — loop rotation, not a stale value. Same shape
already shipping in `func_8003DBE4`.

## The swap, and its quantified spec (ra_solver Phase 5)

Ours puts `t` in `$a1` with the sign-extended `var_s0` in `$v1`; target has them
swapped, and emits `addiu a0,sp,16` early where we emit it late. Global model
3/3 (allocnos 75 = `t`, 72 = `var_s0`, 74 = `ff`); `ra_solver/perturb.py` spec
`{75: 3}` finds NO vector — correct, not a search failure: the `$v1` block on
`t` is the sign-extend, a **local-alloc** quantity. The local-alloc model is
exact here (2/2 order, 10/10 assign); block 2 allocates qty2 (pri 60000)→`$v0`,
qty1 (50000)→`$v0`, **qty0 = the sign-extend, span [6,38), pri 22500 →`$v1`**,
qty3 (20000)→`$v1` — `find_free_reg` scans ascending, so it takes `$v1` because
only `$v0` is occupied across its range.

Target holds it in `$a1`, needing `$v0`, `$v1` **and** `$a0` occupied there.
`tmp/ra/tmv_local.py` gives a **two-part** requirement, neither part sufficient:
(1) an extra qty overlapping [6,38) with pri > 22500 → moves it only to `$a0`;
(2) hard `$a0` live across [6,38) alone → unchanged `$v1`; **both → `$a1`.**
That is why the call-argument family was inert at 22 — it supplies only part 2.
Target supplies both: `addiu a0,sp,16` at line 23 keeps hard `$a0` live across
the sign-extend, and `li v1,24` at 31 consumed at 34 makes the `24` a three-insn
`$v1` quantity. Full write-up in `ra_solver/README`.

## sched_solver goal-mapper round (2026-08-05)

`perturb.py --goal-from-target main`: **blocks 1 and 2 are goal == identity —
the whole loop body's instruction ORDER already matches target;** only block 0
(the prologue) differs, so the banked "one placement" (`addiu a0,sp,16` early)
is not a scheduling question.

Block 0 differed in two clusters (slots 1–5 and 18–20). No single-atom vector;
depth 2 restricted to **spellable atoms** (`--atoms luid,luid_move`) returns a
**fully spellable pair** — two ordinary statement moves, no dependence surgery:

| | edit |
|---|---|
| **A** (`luid swap 9 <-> 14`) | put `buf[1] = 0x60093;` **before** `var_s0 = 0;` |
| **B** (`luid swap 34 <-> 46`) | move `*(s16 *)((u8 *)buf + 0x3C) = 0x4000;` to the **end** of the init block |

**Both tested by compiling the TU.** Honest-stream insns differing from target:
HEAD **19** → A **17** → B **16** → **A+B 14**; the halves compose.

**Re-goaled from the A+B form against a PINNED HEAD target: all three blocks are
now goal == identity — the scheduling class is CLOSED here.** `hon→tgt` goes
`equal 38 / moved 6` → `equal 43 / moved 1`. What is left is `replace 12`
(register naming) and `delete 5 / insert 1` (61 insns to target's 57) — the
extra callee-saves, i.e. the `move_movables` question below.

Note uid 172 is a *reload-generated callee-save store*, so a vector moving it is
evidence about reload's save placement (frame/RA), not statement order.

## The real open question

Target keeps the two constants **separate and short-lived**: `24` in `$v1`
(31→34), the `1` in `$v0` (`li v0,1` at 50, `sllv v0,v0,a1` at 51), neither
hoisted. Our only anti-hoist lever routes both through ONE multi-set `t`, which
necessarily makes a single long-lived pseudo — exactly what destroys part 1. So:

> **why does `loop.c`'s `move_movables` not hoist target's single-set loop
> constants, when it hoists ours?**

Answer that and the `24` becomes a short-lived local qty in `$v1` (part 1), the
shared `t` is unnecessary, and part 2 is a placement question the call-argument
family already half-explored.

## Measured negative / inert (do not re-run)

All score 22 unless noted: named sign-extend local (`s32`/`s16 idx`, register
map bit-identical); `t` at loop-body block scope; every `t` placement sweep
(loop top, at each use, store reordered, shift-first, idx+shift-first —
shift-first is worse, 42 differing lines vs 40); call-arg naming per
[[hoist-call-arg-local-flips-jal-delay]] (`s32 *p = buf;` block-wrapped, at
loop-body top, `u8 *p`, the rule's exact shape) — but **at FUNCTION scope it
regresses to 35**; statement order around the call.

Worse or structurally wrong: `t` for the `0x18` only or the `1` only (still
hoisted, 27); reusing `offset` for the `1` (frame 96, 74 insns, 20) and routing
the `1` through `offset` after the stores (74 insns); `s16 t` (74 insns, 16 —
better on the metric but an extra save and extra insns); `t = (s16)var_s0;
buf[0] = 1 << t;` (74 insns). Shift-first at the qty level adds a 5th qty but it
allocates last into `$v0` — does **not** supply part 1. `store-before-jal` does
not apply (the pre-call `sw …,16(sp)` is already in the jal delay slot both
sides). **cc1psx** ([[cc1psx-calibration-only]]) chooses as we do.

**Build gate:** applying the candidate breaks the oracle (SHA1 `31fe8c21`) — the
18 regfix rules are calibrated to HEAD's shape. `src/main.c` reverted, build
re-verified at `62efab4f73f992798c43e8c730aa43baa10bb4fa`.

## Resume here — BANKED at 22, do not force

Fold the A+B statement moves into `candidate.c` and re-measure, then work the
`move_movables` question above, not another placement sweep: both RA parts must
land together, part 1 is unreachable while `t` is shared, and the loop's
instruction order is now proven already correct.


## s1 (2026-08-20, recon — first post-migration session)

- **New measured floor: 15** (was 22). src/main.c:857 is INCLUDE_ASM at HEAD
  (asm-until-matched migration); the 18 regfix rules are RETIRED, so the old
  "applying the candidate breaks the oracle" warning is MOOT — sessions now just
  apply candidate.c over the INCLUDE_ASM line and sandbox. Candidate = floor-22
  form + A+B statement moves folded (A: buf[1]=0x60093 before var_s0=0;
  B: 0x3C store last in init block). build_insns 72 == target_insns 72.
- **Residual-15 map** (ours = tmp/sandbox/func_800858D0/main.o vs
  asm/funcs/func_800858D0.s; full alignment in tmp/grind/func_800858D0/s1/):
  (a) rotation identity — reorg steals the loop-body block-top insn into
  preheader (slot 22) + loop-back delay slot (slot 65): target steals
  `addiu a0,sp,16`, we steal the t-carrier's `li 24`;
  (b) mid-loop cluster — our `li a1,1` / `addiu a0,sp,16` / `sllv` sit right
  after store #1 (t=1 is freed the moment the 24's store retires, and sllv's
  dest kills idx); target keeps `li v0,1; sllv v0,v0,a1` just before the jal;
  (c) register naming — idx sign-extend a1 (target) vs v1 (ours); the 24 in v1
  vs a1; the 1/shift-result in v0 vs a1/v1.
- **loop.c decision profile now MATCHES target**: main.loop shows "Loop from 52
  to 146: 30 real insns" with ZERO movables (t = reg 75 only "possible biv,
  discarded"). Target's stream likewise shows nothing hoisted.
- **Single-set constants are PROVEN always-hoisted here** (kills a whole
  spelling class): threshold = (1+n_non_fixed_regs) = 61 with calls in loop
  (loop.c:532, mips.h FIXED_REGISTERS: 8 fixed of 68); move test
  61*savings*lifetime >= insn_count=30 holds even at savings=lifetime=1
  (loop.c:1631). The reg_single_usage substitution escape (loop.c:735) cannot
  fire: movhi's condition (mips.md:3352) rejects const-source HI stores (only
  const 0 via $zero), sllv rejects const shift operands. So target's 24/1 were
  NOT single-set const pseudos — and ours must not be either. The remaining
  puzzle is that target uses TWO registers (v1 for 24, v0 for 1) while the only
  known no-movable spelling (shared multi-set t) yields ONE.
- **sched1 mechanism for cluster (a)**: main.sched block 2 — backward scheduler;
  our t=24 set (insn 71, priority 1) is ready from T-14 but loses to the
  0x7f000001 address-chain insns every cycle and lands FIRST in the block →
  reorg steals it. Whatever insn sits at block top becomes the
  preheader+delay-slot pair. Target had the a0-set there.
- **Re-measured kill**: block-scope `s32 *p = buf;` at loop-body top is inert at
  15 on the new chassis (canon_reg folds it; rejected/p-loop-top-block-scope-
  inert-15.c). Function-scope p regressed to 35 on the old chassis — do not
  retry either.
- Artifacts: tmp/grind/func_800858D0/dumps/ (main.loop / main.sched / main.greg
  etc., generated FROM the floor-15 form), tmp/grind/func_800858D0/s1/.
- NOTE for next session: the old-chassis sched_solver "goal == identity /
  scheduling CLOSED" claim predates the migration and describes the A+B form vs
  the OLD sandbox — the observed cluster (b) means scheduling questions are OPEN
  again on the new chassis. The proven next tools: sched_solver
  `perturb.py --goal-from-target --atoms luid,luid_move` (it FOUND A+B) and
  ra_solver Phase 5 re-run, both against the floor-15 form.

- [s1] HEAD floor is unmeasurable (INCLUDE_ASM since asm-until-matched migration); migration pin 27; new banked candidate floor 15, measured this session

- [s1] Residual-15 = 3 clusters: (a) reorg preheader/delay-slot steal takes OUR li 24 where target's takes addiu a0,sp,16; (b) our li 1 / addiu a0 / sllv cluster sits after store #1 vs target's just before the jal; (c) register naming idx a1-vs-v1, 24 v1-vs-a1, 1/shift v0-vs-a1/v1 (alignment: tmp/grind/func_800858D0/s1/recon-alignment.md)

- [s1] main.sched block 2 (backward scheduler): t=24 set insn 71 priority 1, ready from T-14, loses to 0x7f000001 address-chain insns, lands FIRST in block -> reorg steals it; whatever insn is at block top becomes the preheader+delay pair

- [s1] Target uses TWO registers for the constants (24 in v1, 1 in v0) while the only known no-movable spelling (shared multi-set t) yields ONE (a1) - this two-carrier split is the core unsolved puzzle

- [s1] Old-chassis 'scheduling CLOSED / goal == identity' claim is stale: cluster (b) shows in-loop order divergence on the new chassis; sched_solver and ra_solver must be re-run against the floor-15 form

- [s1] The old 'candidate breaks the oracle' warning is moot: the 18 regfix rules are retired, src/main.c:857 is INCLUDE_ASM, sessions apply candidate.c directly

## s2 (2026-08-20, structural — MATCH FOUND, sandbox 0)

- **MATCHED at sandbox --disable all = 0** (72/72 insns, frame 96, saves 3);
  per-word disassembly of tmp/sandbox/func_800858D0/main.o identical to
  asm/funcs/func_800858D0.s modulo relocations (checked instruction by
  instruction, including both delay slots, the increment cluster
  `addiu v0,s0,1; addu s0,v0,zero`, and the preheader/loop-back
  `addiu a0,sp,16` pair). Edits in place in src/main.c over the former
  INCLUDE_ASM line.
- **The closing lever is the loop SPELLING: label + trailing
  `if (var_s0 < D_80101BCC) goto loop;` instead of do/while.** A goto-spelled
  loop emits no NOTE_INSN_LOOP_BEG/END, so loop.c never treats the region as
  a loop and move_movables never runs. This dissolves the s1 two-carrier
  puzzle: target's 24 and 1 ARE plain single-set constant pseudos — they were
  never candidates for hoisting because there was no loop for loop.c to see.
  H3's kill ("single-set const spellings are dead a priori") was correct ONLY
  under the hidden assumption of an intact loop-note structure; the goto
  spelling escapes its premise, not its math.
- **Launch mechanism (names the pass, read from sched.c):** when a consumer is
  scheduled, `schedule_insn` (sched.c:4049) temporarily holds it at
  LAUNCH_PRIORITY (0x7f000001) and `adjust_priority` (sched.c:2584-2590)
  raises a newly-readied parent to that priority — but ONLY if
  `birthing_insn_p` (sched.c:2504-2526) approves, which requires the parent's
  dest pseudo to have **reg_n_sets == 1**. So single-set constant pseudos are
  launched ADJACENT to their consumers (li v1,24 next to its sh; li v0,1 next
  to sllv/jal — exactly target's clusters), while ANY multi-set carrier
  forfeits launch, keeps priority 1, and drifts to sched1 block top. This is
  why every multi-set-t chassis (floors 27/22/15/13) had a li stolen by reorg
  into the preheader/delay pair: the anti-hoist trick itself created the
  block-top identity divergence. The two goals ("don't hoist" and "launch
  adjacent") are CONTRADICTORY for any spelling loop.c can see — the only
  resolution is no loop notes.
- **Rotation identity + RA parts land for free:** with all constants launched,
  the a0=sp+16 hard-reg set (multi-set hard reg, never launchable, ready from
  T-12) is the lone priority-1/class-3 straggler; the rank_for_schedule LUID
  tiebreak (sched.c:2461-2464, higher LUID issues earlier backward) leaves it
  at block top; reorg steals block-top into preheader + loop-back delay slot
  (target's identity); hard $a0 is then live across the sign-extend span, and
  with $v0/$v1 occupied by the launched constants the sign-extend lands in
  $a1 — BOTH parts of the ra_solver Phase 5 two-part requirement satisfied at
  once, exactly as the s1 ledger predicted they must land together.
- **Kill (rejected/two-carrier-incr-temp-sext-save-25.c):** spelling the
  increment through an int temp (`t = var_s0 + 1; var_s0 = t;`) sign-extends
  var_s0 BEFORE the add; CSE reuses the loop-top sign-extend, which then
  lives across both calls -> 4th callee-save, 73 insns, score 25. Target
  increments the raw register and extends the temp afterwards; only direct
  `var_s0 = var_s0 + 1;` (s16 arithmetic) reproduces it.
- Intermediate do-while measurements this session (all superseded): baseline
  candidate re-confirmed 15; two-carrier t{1,incr}/u{24,lim-reload} with
  direct increment = 17 (cluster b CLOSED: li v0,1/sllv adjacent to jal;
  proves the multi-set-u limit-reload pairing puts 24 in $v1 and lbu in the
  same pseudo, matching target's v1 pairing); + `offset = 1;` reuse as the
  shift source (anti-dep on the six stores pins the li late) = 13 (new
  do-while floor). These forms are structurally instructive but the goto
  spelling makes all carrier machinery unnecessary.
- The `(s16)` casts on var_s0 reads are byte-inert in the goto form (var_s0
  is already s16): removed, still 0. Final body has zero redundant casts,
  zero scratch carriers, zero dead code — every statement is live, consumed
  program logic.
- SOTN-master precedent for goto-spelled loops in matched PSX code:
  docs/reference/sotn-construct-index.md:1033 (src/main/main.c:40
  `main_search_loop_1:` goto from line 57), :1015 (src/dra/5F60C.c:579
  `loop_check_equip_id_1:` goto from 582), :1035 (sprintf.c:96 `loop_30:`).
- Artifacts: tmp/grind/func_800858D0/s2/ (sched_func.txt = block-2 sched1
  trace of the floor-13 form showing insn 71 ready T-21 losing every cycle
  and insn 113 losing the LUID tiebreak; ours.dis; dis.sh; sched_slice.py),
  tmp/grind/func_800858D0/dumps/ (regenerated from the floor-13 form).

## s3 (2026-08-20, permuter — BYTES PROVEN via F6 empty-if; sandbox blocked by detector)

- **Floor-13 do-while form RECONSTRUCTED and re-confirmed at 13** (72/72,
  frame 96, saves 3). The s2 ledger never banked it as a file; the working
  reconstruction is: u={0x18, limit-reload} carrier, offset={stride, 1} reuse,
  direct s16 increment, A/B init-block moves. It is now the body of
  candidate.c (minus the closer). Residual-13 fully mapped
  (tmp/grind/func_800858D0/s3/ours13.dis): ONE coupled flip — ours steals
  `li a1,24` into preheader+loop-back delay where target steals
  `addiu a0,sp,16`, plus the cascading a1<->v1 renames (sign-extend, 24/limit
  carrier, slt) — 13 diffs, one root cause (block-top identity).
- **Noted-loop pin trilemma PROVEN analytically (kills the whole honest
  in-loop-notes spelling space for the mid-loop li 24):**
  (1) loop.c:692-700 — a movable needs one of three cases;
      for a SINGLE-SET user-var const all three hold or fail as follows:
      case 1 (reg_in_basic_block_p, loop.c:1062) requires regno_first_uid ==
      the set insn — any same-block consumer keeps it true; case 2 requires
      !REG_USERVAR_P (a named C local always fails it, i.e. case 2 never
      protects a user var — but never blocks one either); case 3 fails only
      via maybe_never (impossible: target loop body is branch-free) or
      used-before-set (semantically impossible: iteration 1 would read
      garbage; the only consumer is the FIRST sh). So a single-set const in
      a noted loop is ALWAYS movable, and H3's threshold math (61 >= 30)
      always moves it. No spelling escapes within loop notes.
  (2) sched.c:2504-2537 birthing_insn_p requires GLOBAL reg_n_sets == 1 for
      launch — any multi-set carrier (every anti-hoist spelling) forfeits
      the adjacent-to-consumer launch that puts li 24 mid-loop.
  (3) rank_for_schedule (sched.c:2407-2464): the two pri-1 stragglers
      (li 24, a0-set) are both class-3 (independent) at the deciding cycle,
      so the LUID tiebreak places the LOWER-LUID insn at block top; li 24's
      LUID is necessarily below the a0-set's (its consumer sh precedes the
      call that generates a0=sp+16), so no statement order flips it.
  Together: "don't hoist" and "launch adjacent" are contradictory for any
  spelling loop.c can see — CONFIRMING s2's conclusion, now at file:line.
- **THE HONEST RESOLUTION EXISTS ANYWAY — pass-order window.** Permuter
  campaign (permuter/func_800858D0_s3, 18,667 iters, ~7 min to zero,
  --stop-on-zero, --stack-diffs) found score 0: floor-13 form +
  `if (D_80101BCC) { }` inserted between the increment and the limit
  re-read. Dump-proven mechanism (dumps regenerated from this form):
  jump1 deletes the empty branch; the manufactured lbu survives as
  reg101/reg102; cse1 rewrites the while-compare to read reg102 directly,
  leaving `u = reg102` (insn 139, main.cse2) DEAD; loop.c runs BEFORE flow
  and counts reg75(u) as 2-set -> not a movable -> li 24 stays in-loop
  (main.loop: "Loop from 52 to 154: 32 real insns", zero moved lines);
  flow deletes insn 139 (main.flow: NOTE_INSN_DELETED 139); sched1 runs
  AFTER flow, sees reg_n_sets[75]==1, birthing_insn_p approves, the li 24
  is LAUNCHED adjacent to its sh; the a0-set becomes the lone straggler at
  block top; reorg steals it (target's rotation identity); hard $a0 across
  the sign-extend span lands the extend in $a1, the 24/limit class in $v1 —
  all 13 diffs flip together. loop.c counts sets pre-DCE, sched counts
  post-DCE: that window is the only honest escape from the trilemma.
- **Byte proof:** permuter score 0 vs target.o built from
  asm/funcs/func_800858D0.s + prelude (offset-0, --stack-diffs, honest
  pipeline: cc1 -mel | prologue_fix | maspsx | as). TU-context emission
  word-identical 72/72 (tmp/grind/func_800858D0/s3/ours_tu.dis — full
  Makefile pipeline incl. regfix stages on src/main.c — vs zero.dis; only
  section-offset branch TEXT differs, instruction words equal).
  verify-oracle --rebuild correctly REFUSED on dirty src (guard working as
  designed); not overridden.
- **BLOCKER: engine/volatile_cheats.py find_empty_if_dead_reads (line 482,
  written for the 2026-06-02 audit) strips the empty-if before scoring —
  sandbox reads 13 with cheat_asm_stripped 67->68. The detector PREDATES the
  2026-08-18 F6 sanction (no-new-park-categories.md:371-383, "Sanctions ONLY
  the exact cancellation pair and empty-condition shapes") and has NO
  allowlist hook (unlike _SANCTIONED_UNWRITTEN_PADS for the F7/pad family).
  candidate-ready ("sandbox printed 0 THIS session") is therefore
  MECHANICALLY UNREACHABLE for this construct until the engine gains an F6
  allowlist row — engine/ is outside grind-session scope.**
- **Classification conflict needing a ruling:** the cheat-reviewer catalog
  (and the 2026-06-02 detector docstring) list "empty-body if (cond) { }
  dead-read" as a forbidden family; no-new-park-categories.md:371-383 (owner
  ruling 2026-08-18, "Go ahead with all your recommendations on these")
  sanctions the empty-condition shape with SOTN exhibits
  (docs/reference/sotn-construct-index.md:34 — empty_if family, 17 PSX
  instances; :63 — dra/5D5BC.c:769 "!FAKE, permuter found it";
  no-new-park-categories cites dra/5D5BC.c:770 `if (!i) { }` and
  st/st0/cutscene.c:203 `if (prim && prim)`). Our exhibit is the exact
  SOTN shape: permuter-found empty-if over a REAL global read, annotated,
  with a named multi-pass mechanism and a proven-exhausted lever space.
- Permuter kills banked: rejected/permuter-sh-before-set-semantics-broken-160.c
  (reorder mutations are NOT semantics-preserving — the sh-above-set family
  stores a stale/uninitialized u; every such find rejected on correctness),
  rejected/permuter-newvar-alias-if1-cheats-30.c (invented alias/staging
  locals + if(1){} wrap — forbidden shapes).
- Hand probes measured this session (all at 13, inert): u = var_s0*54 staged
  through u then copied to offset (cse folds the copy — emission identical);
  `u = 0x18` hoisted above the stride multiply (LUID of the set does not
  matter, consistent with trilemma lock 3).
- Artifacts: tmp/grind/func_800858D0/s3/ (ours13.dis, base13.dis, ours_tu.dis,
  zero.dis, zero-source.c, zero-diff.txt, campaign-tail.txt,
  campaign_meta.json, tu_build.sh), permuter/func_800858D0_s3/ (workspace,
  campaign.log, 35 output dirs incl. output-0-1), tmp/grind/func_800858D0/
  dumps/ (regenerated from the zero form).
