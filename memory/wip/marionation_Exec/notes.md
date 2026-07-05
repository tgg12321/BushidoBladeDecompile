# marionation_Exec — iq3 form (masked 6, full s-web) LAYER-1 FAILED; BLOCKED ON USER RULINGS (2026-07-05)

## TL;DR
THE FRONTIER MEASUREMENT = rejected/cross-symbol-fake-plus-double-split.c
(masked 6, raw 28, full s-web + printf regs TARGET-CORRECT) — layer-1
FAILed it on 2 constructs (see meta.json.reviewer): (1) the cross-symbol
pointer FAKEs (idx_1495 tbl-derived + F19C0 rebase) = an UNSANCTIONED
family (also present in the restored mh5 candidate.c — retroactive);
(2) iq3's `+=1; +=1;` double-split exceeds the provisional split-init
sanction. BOTH load-bearing (no-#1 = masked 30; no-#2 = masked 16).
SURFACED TO USER — do not carry them forward without rulings.
candidate.c = mh5 (masked 4, wrong-basin mirage) restored per protocol.
Recipe of the failed frontier: m2c-tail (fixes region-1 exchange) +
F19C0-FAKE rebased to idx_1494 (+2 refs) + y1 saved-split + iq3
(`idx_1496 = idx_1494; +=1; +=1;` — 9 refs survive combine, ONE addiu
emitted). Ledger
(BB2_ALLOC_DEBUG): saved 3636→s1, i1494 9r/1800→s2, i1496 9r/1800→s3
(tie broken allocno-ascending, 76<78), arg1 952→s4, tbl 657→s5, i1495
405→s6 — ALL CORRECT. Levers measured: FAKE anchor = +2 refs to its
base pseudo; iq = +2; iq3 = +4; cross-symbol diffs never fold BUT
anchor must be OPAQUE (tbl = mem-loaded ✓; idx_1494-anchored idx_1495
FAKE FOLDED — cse traces la constants; F19C0-on-idx_1494 survives
because &D_80016248-&D_800A1494 spans sections). tbl NEEDS its +2
(idx_1495 FAKE) to hold s5.

## REMAINING 3 RESIDUALS (masked 6 = 2+2+2)
1. **56/57 pair**: ours `sll a0; addu v0` vs tgt `addu v0; sll a0`
   (printf block, the t0-shl C-luid order). Old archive: launch law
   (birthing_insn_p sched.c:2496) — `<<=`/`+=` in-place, `*=` launches.
2. **86/88 y1 cost**: ours `lbu s1; andi s1,s1` vs tgt `lbu v0; andi
   s1,v0`. Need saved≥3refs (seat s1) with load in a SEPARATE pseudo:
   plain=2refs loses seat; any user temp (tmp0/v0-var) rotates web
   (23/24). UNSOLVED — candidate keeps y1's 2-line cost.
3. **149 NOP**: check2-beqz slot: tgt NOP, ours fills `move a1,s4` ⇒
   178 vs 179; lines 150-177 = SHIFT artifact (beqz-s7 was NEVER
   missing). MECHANISM PINNED (s6n): DBRDBG trace insn=393: fill_slots_
   from_thread, fall-through side, own=1 likely=0 tif=0, oppregs=
   20fc0000 (={s2-s7,sp}, a1 NOT live at taken target) ⇒ sb loses
   (trap=1), move a1,s4 WINS. Forcing all-live at the taken label
   (BB2_ALLLIVE_LABEL fixpoint {462,101,557}, cc1_alllive) reproduces
   target EXACTLY — nop + arm-2 [sb;move;la] order together. reorg.c
   reads (3321-3820, 2186-2255): win needs !sets(opposite_needed)+
   !trap; own_thread_p checks HEAD labels only (deep done: label
   irrelevant); scan stops at sb-LOSE only if own=0; find_basic_block
   = back-to-BARRIER + consecutive-labels vs basic_block_head[] (-1 ⇒
   all-live). a1 semantically dead on taken path (calls write a1 pre-
   use) ⇒ original refusal MUST be all-live ⇒ its .L812C4-analog label
   was NOT resolvable (post-flow-minted/replaced). MEASURED FILLED
   (shapes that canonicalize back): goto-after indirection, 3rd return
   stub, explicit else, return-const, tail relocations above-check2 /
   pre-check (backward targets), r5d far-referencer inapplicable (no
   label at arm-2 head in TARGET bytes; .L812BC's 2 refs = the done:
   stub, harmless). Bytes prove NO cross-jump fired (both [j;move
   v0,a2] stubs present). NEXT: standalone mini of the EXACT new-world
   tail (uid-stable, fast) bisecting what mints/replaces the label —
   candidates: jump1 follow_jumps depth/loop-crossing refusals leaving
   a threadable chain for relax pass, label recreation via
   redirect+get_label_before in relax_delay_slots.

## Ledger arithmetic (BB2_ALLOC_DEBUG drill, tmp/mar_reb_drill.py)
p78 needs pri ∈ (952, 1800]; L=150 ⇒ pri=133.3×refs ⇒ refs 8-9. Base 5,
iq→7 (933 < arg1's 952 — why iq alone "did nothing"), iq3→9 ✓. p76
base 7 + rebase-FAKE 2 = 9. arg1 fixed 4r/84/952 (dst-test respell is
CSE'd back — can't drop; livelen byte-pinned). MEASURED DEAD: bothnines
via idx_1495-rebase (FOLDS, head shattered, 25/124); tmp/v0 saved
splits (23/24); dst-test (16, no-op); tail-end branch-sense respells
rev/chk/chk2 (16 — jump.c canonicalizes); natural-head all subsets
(30); i5-split (30); whole-m2c timeout respell (17-24, rotates web).

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5, i1495
  s6, arg0 s7; check a2, src a0, i v1, b v0, dst/dst2 a1.
- arm-2: sb -1(s3); move a1,s4; la F19A8; beqz a1→.L812BC slot=li v1,7;
  li a3,-1; loop; .L812BC: j .L812CC; move v0,a2. check2 beqz slot=NOP.
- .L812C4: beqz s7→.L810A4 slot=move v0,0; epilogue .L812CC. Tail has
  TWO identical [j .L812CC; move v0,a2] (jump-vs-jump cross-jump needs
  min-2; only 1). Arm-1 check-beqz slot=NOP naturally (verified trace).
- printf window: t0 chain IN-PLACE in a0; fmt-la LAST; 11DC via $at;
  11D5 lui+lbu on v0; head idx_1495/6 = addiu s6,s2,1 / addiu s3,s2,2;
  F19C0 stored from direct la v0,D_80016248.

## Known gotchas
- 42 rules index-anchored; end gate = retire-all-42 + full SHA1. Twin
  csmd4 shares system.c — uid/pseudo spaces COLLIDE in TU logs (segment
  per function: p82/pri=3636 fingerprint = marionation's block).
  Declare new_var/new_var3. Permuter finds = PROPOSALS (210-1/220-1
  REJECTED: int+ptr arith scales ×4 — check `t0 += tbl` patterns).
- Resume protocol: apply candidate.c to src/system.c, `sandbox
  marionation_Exec --disable all` must say 6; iterate from there.

## Tools (local/gitignored; regenerate from here if lost)
- tmp/gccdbg/cc1 (BB2_DBR/ALLLIVE/ALLOC/QTY/SCHED knobs); drills:
  tmp/mar_reb_drill.py (ALLOCDBG ledger), tmp/mar_refsweep.py (ref
  sweep, compile-only), tmp/mar_iq3_score.py (frontier verify),
  tmp/mar_mk_candidate.py (regen candidate from recipe).
- Permuters: tmp/perm_mar_iq3 (-j6, base 230) + tmp/perm_csmd4 (-j4);
  verify pattern: tmp/mar_verify_220.py. Traces: tmp/mar_trigger_gen*.
- Harness pattern: splice candidate body into src/system.c → engine
  sandbox → objdump tmp/sandbox/.../system.o vs build/src/system.o →
  restore src. See tmp/mar_tailend.py for the template.
