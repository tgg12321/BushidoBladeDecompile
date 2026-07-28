# Evidence bank — func_800200DC

## s1 recon (2026-07-27)
- Baseline re-verified at HEAD: canonical verdict C (168 insns, distance 14);
  `sandbox --disable all` = 14, build 168 / target 168, 14 rules stripped.
- Exact residual map (artifact tmp/grind/func_800200DC/s1/diff_named.txt):
  14 = Rotation A (6: dy v0-vs-v1 through the disc chain) + Rotation B
  (8: sq/quotient $v1<->$a1 swap after the second sqrt call). No insertion/
  deletion/reorder diffs — pure renames, consistent with all-14-subst rules.
- Target burns $t1 for the arg3*dy2 mflo while our build reuses the freed
  v-reg — the one asymmetry that is NOT a pure rename; suggests a liveness or
  allocno-order difference rooted at dy (Rotation A root).
- m2c reference shape confirms our C structure exactly (a0=300 preset hoisted
  into the bltz delay slot; shared arg4[2] tail store via cross-jump). Artifact:
  tmp/grind/func_800200DC/s1/m2c.c.
- No duplicate/sibling analog: find_duplicates.py full scan, 0 entries for this
  function (tmp/grind/func_800200DC/s1/dups_all.txt, 206 pairs).

- WIP rejected_form: {'form': 'compute a2 = arg2<<5 BEFORE sq=func_8007E11C(...) (across the call)', 'score': 15, 'reason': 'Forces a2 callee-save across the call; +1 worse.'}

- WIP rejected_form: {'form': 'disc = arg3*dy2 + arg2*arg2 (operand order swap)', 'score': 38, 'reason': 'Reassociates the mult/add chain; insn count drops to 167, large divergence.'}

- WIP rejected_form: {'form': 'hoist dy = arg1[1]-arg0[1] before the dist==0 early-return', 'score': 72, 'reason': 'dy must survive the func_8007E11C(dx*dx+dz*dz) call -> callee-save cascade, -5 insns, severe divergence.'}

- == imported from memory/wip notes.md ==
# func_800200DC — coupled register-rotation wall (code6cac.c, projectile angle)

## TL;DR (2026-06-14)
Honest distance 14; both builds 168 insns (structure already correct). The 14
regfix rules are ALL `subst` register renames (NO insert/delete/reorder) — a
dense coupled $v0/$v1/$t1/$a1/$a2 rotation across the projectile-launch-angle
compute chain. A mid-function CALL (`func_8007E11C`, the sqrt) forces
callee-save decisions that couple the whole allocation, so every statement
reorder I tried cascaded WORSE (15 / 38 / 72), never better. No floor
improvement; blocked. Same plateau class as func_80072CD4 / D_80083418 this
session (but heavier — 14 coupled renames vs their handful).

## Resume steps
1. `sandbox func_800200DC --disable all` on HEAD = 14 (14 rules stripped).
2. The diff is pure register-rename: first/cleanest is the `dy = arg1[1] -
   arg0[1]` subu landing in $v1 (target $v0); then the disc/sq/a0 chain mflo
   pairs and the `move v1,v0` / `addu v0,a2,v1` cluster swap $v0/$v1/$t1/$a1.

## Live hypotheses
- Instrumented cc1 ALLOCDBG/PRIODBG ([[register-alloc-deep-dive]]) — hand
  levers cascade because of the sqrt call's callee-save coupling; need to read
  the allocno-priority tiebreak directly.
- Flip ONLY `dy`'s register without disturbing the call save/restore (root of
  the rotation) — not found by reorder.
- Likely register-alloc-pure-c "confirmed limits" -> canonical-asm review.

## Ruled out (do not re-derive)
- a2 = arg2<<5 before the sqrt call -> 15 (callee-save +1).
- disc operand-order swap (arg3*dy2 + arg2*arg2) -> 38 (reassociation, 167 insns).
- hoist dy before the dist==0 return -> 72 (dy survives the call, -5 insns).

## Pointers
- `.claude/rules/register-alloc-pure-c.md` (confirmed-limits class).
- Siblings on the same wall this session: memory/wip/func_80072CD4/,
  memory/wip/D_80083418/. Contrast mot_data_set (text1a_c) which WAS closable —
  a single clean swap fixed by reordering chroma before the copy; this one's
  rotation is coupled through a call and does not yield to the same trick.


- [s1] Baseline at HEAD: canonical=C, sandbox --disable all = 14, build 168 / target 168, 14 subst rules stripped (matches ledger import).

- [s1] Exact diff map artifact: Rotation A = subu dest + mflo/sll/addu operand renames in the disc chain; Rotation B = move-sq/mflo-quotient/bgez-sra-addiu-sra/subu cluster, a pure $v1<->$a1 swap.

- [s1] Asymmetry: target uses $t1 for the arg3*dy2 mflo (`mflo $t1; addu $v1,$v1,$t1`) where ours reuses the freed v-reg (`mflo $v1; addu $v1,$v0,$v1`) — with low-first alloc order this implies $v0/$a0/$a1/$t0 were occupied in target's RTL there, i.e. dy possibly live past the sll; Rotation A is the likely rotation root.

- [s1] m2c confirms structural completeness (a0=300 delay-slot preset, cross-jumped shared arg4[2] store).

- [s1] No duplicate/sibling analog exists (full find_duplicates.py scan).

## s2 structural (2026-07-28) — FLOOR 14 → 6 (Rotation B closed)

- [s2] **disc-reuse closed Rotation B**: `disc = func_8007E11C(disc << 10);`
  (reusing disc for the sqrt result instead of a fresh `sq` local) dropped the
  floor 14 → 6 in one probe. Evidence trail: target has disc in $v1
  (addu/bltz/sll<<10) AND the call-result copy in $v1 (`move $v1,$v0`) AND the
  a2-disc subu reading $v1 — one variable, one register. The merged pseudo
  (105) has ~6 refs and wins $v1; the arm-1 quotient then takes $a1 exactly as
  target. Semantically real reuse (value used, nothing dead) — not a FAKE.
- [s2] Residual 6 (artifact tmp/grind/func_800200DC/s2/diff_named.txt,
  generated at the score-6 form) = Rotation A ONLY: subu/bnez dest ($v0 tgt vs
  $v1 ours) + mflo/sll pair (arg2^2 temp $v1 tgt/$v0 ours; sll src follows dy)
  + mflo $t1/addu (tempX $t1 tgt vs $v1 ours). All other diffs are
  li/addiu + lui-spelling + maspsx-nop normalization noise.
- [s2] cc1 -da dump (tmp/grind/func_800200DC/s2/code6cac.i.{lreg,greg}) at the
  score-6 form: dy = pseudo 87, prefers hard reg 3 ($v1) via global.c
  set_preference (for `set dy (minus A B)` the preference source is XEXP(src,0)
  = the MINUEND's reg — GCC always prefers the first operand of the
  subtraction); 87's conflict list does NOT include $v0. arg2^2 temp = 106
  (→$v0), arg3*dy2 temp = 107 (→$v1), disc/sq merged = 105 (→$v1 ✓ target).
  Global alloc order: … 107 105 87 102 83 73 72 106 … (dy before 106).
  Target needs 106→$v1, 107→$t1, dy evicted to $v0; the flip is in
  allocno order/conflicts around dy vs the two mult temps, NOT in dy's
  preference (same minuend/$v1 preference must exist in target's RTL).
- [s2] "Spilling reg 8" in our greg dump — reload reserves $t0 as the spill
  reg for the sqrt-arg insn; explains why target's tempX lands at $t1 (first
  free t-reg) rather than $t0 when the low regs are occupied.
- [s2] m2c's `var_a0 = 0x12C` before the disc test is post-reorg APPEARANCE
  only — testing that source shape scores 29. The else-arm form + cross-jump
  is correct (target's delay-slot preset comes from reorg, not source order).
- [s2] cc1 parse errors ("parse error before `GameObj'") exist in the
  production build too (GameObj typedef nowhere in code6cac's include chain;
  errors swallowed by the pipe, cc1 recovers, oracle stays green). Replicating
  the pipeline for dumps: don't `set -e` on the cc1 step.

- [s2] sandbox floor 6 verified twice this session at the candidate form (168/168 insns, 14 rules stripped); edits in place in src/code6cac.c

- [s2] residual 6 = Rotation A only: subu/bnez dy dest ($v0 tgt vs $v1 ours), arg2^2 mflo/sll pair ($v1 tgt vs $v0 ours), arg3*dy2 mflo $t1 tgt vs $v1 ours (artifact s2/diff_named.txt)

- [s2] cc1 -da dump at score-6 form: dy=87 prefers $v1 via set_preference(minuend), no $v0 conflict; temps 106->$v0, 107->$v1, disc/sq merged 105->$v1 (target-matching); global alloc order ... 107 105 87 ... 106 ...

- [s2] reload reserves $t0 as spill reg (Spilling reg 8), explaining target's $t1 temp landing spot

- [s2] cc1 'parse error before GameObj' occurs in the production build too (pipe swallows it; cc1 recovers; oracle green) — dump replication must not set -e on cc1

## s3 structural (2026-07-28) — FLOOR 6 -> 5; a SANDBOX-0 form exists (pending ruling)

- [s3] Mechanism of Rotation A fully proven with the instrumented in-tree cc1
  (tools/gcc-2.7.2/cc1 has BB2_ALLOC_DEBUG + BB2_FINDREG_DEBUG; build/cc1 does
  NOT — rebuilt-in-tree binary emits byte-identical asm, parity checked).
  Artifacts: s3/allocdbg*.txt, s3/findreg*.txt, s3/fr*.txt.
- [s3] Corrected s2's pseudo map: 107 is the mult's LO-class result (hardreg
  65); the "mflo $t1 vs $v1" register is a RELOAD reg for the GR need at the
  disc addu, not a global allocno. 98 is a TAIL quotient temp. The real actors:
  dy(87), arg2^2(106, LO-preferred-class), disc(105), and the reload.
- [s3] Full causal chain of the old 6-insn residual: (1) find_reg pass 0 lands
  dy on $v0 (natural scan; $v0 in used_so_far), then the preference-override
  steals it to $v1 — dy's pref {3} comes from set_preference taking the
  MINUEND's local-alloc-renumbered reg at `(set dy (minus L_v1 L_v0))`.
  (2) With dy=$v1, arg2^2 (conflicting with dy) is pushed to $v0 at its late
  turn. (3) With the addu output $v1 not among inputs, reload SHARES the output
  reg for the LO->GR input reload (mflo $v1). Target's state (dy=$v0,
  arg2^2=$v1, mflo $t1) follows from ONE bit: dy having no usable $v1 pref.
- [s3] Why no same-statement-set respelling can flip it (kills that whole
  space): every allocno conflicting with dy is call-crossing, so
  prune_preferences (IOR call_used_reg_set) empties their prefs ->
  regs_someone_prefers[dy] is structurally empty; and arg2^2's preferred class
  is LO_REG, so prune wipes any GP pref expand_preferences merges into it
  (resolves the "expand should have merged 105->106" contradiction).
- [s3] THE LEVER: stage the subtraction minuend through a cross-block pseudo.
  set_preference only records hard<->pseudo pairs; a global (unrenumbered)
  minuend gives dy NO pref -> dy keeps $v0 -> cascade matches target.
- [s3] Staged through disc (existing var, both minuends): floor 6 -> 5.
  Residual 5 = arm-2 quotient lands $a0 vs target $v1: disc has pref {4}
  (combine folds `sll $a0,disc,10` into the call-arg set -> set_preference),
  and when disc dies at the staged dy-subu, expand_preferences merges {4} into
  dy, whence it propagates dy->dy2->divres->quotient (each link a die-into-set
  merge); the quotient's override then takes free $a0. The dx-side stage is
  load-bearing: disc's first segment overlaps arg1-ptr, blocking the merge
  that leaks arg1-ptr's {5} ($a1 entry-copy pref) — without it the arm-1
  quotient lands $t0 (dy-only disc stage measured 7).
- [s3] Fresh 2-write relay y1 (prefs empty, no {4}/{5} leaks): **sandbox 0**
  (168/168, 14 rules stripped), independent rule-free pipeline diff 0
  (s3/final_y1.diff.txt). Layer-1 cheat-reviewer FAILED it (fresh variable is
  outside the staged-value-reused-variable sanction) -> OWNER RULING REQUESTED;
  form preserved at memory/grind/func_800200DC/pending-ruling-y1.c.
- [s3] neg as carrier (existing zero-arm var): 26 — carrier lands $a1
  (rejected/neg-carrier-26.c). Register-asm pins on dy (both "$2" and "v0"
  spellings) are silently ignored by this cc1 — diagnostic pin probes are
  useless here.
- [s3] Reload detail: with the addu output reg also an input (arg2^2=$v1),
  the LO reload cannot share the output and falls to the spill/free pool;
  measured $t1 (target-matching) in the y1/disc forms, i.e. the pool ordering
  is right once the allocnos are.

## s3b structural (2026-07-28, session 3 of the s3-numbered ledger) — carrier exhaustion; floor stays 5

- [s3b] Baseline floor 5 re-verified twice (start + close) with candidate.c in
  src (168/168, 14 rules stripped).
- [s3b] a2-carrier (both minuends staged through existing a2): **8**. Downstream
  PERFECT (dy=$v0, /32 temps $a1/$v1, disc $v1) but carrier lands $a2 + arg2
  evicted to $s3 in prologue. FINDREG(77): someone_prefers={3,4,7} = disc's
  full prefs (a2 CONFLICTS with disc — its real segment spans arg2<<5 through
  the arm-2 (a2-disc) use). Structurally dead: carrier can never share $v1
  with disc. Artifact: s3/a2_carrier_8.diff.txt, a2c.*.
- [s3b] neg's $a1 landing EXPLAINED with FINDREG (judge precondition met):
  someone_prefers EMPTY; hard conflict {2,3,4}. The $v1 conflict is the
  zero-arm quotient-1 local (negc local table: 94 in 3) — once neg leaves the
  local pool (carrier=global), local-alloc's scan gives quot1 $v1 (in intact
  forms local-neg@$v1 blocks it to $a0 = target). neg-carrier is
  SELF-DEFEATING. Artifacts: negc.*, negc_fr77.txt.
- [s3b] neg-carrier + zero-arm a0-split (a0=neg*dx; a0=a0/denom; store): **18**.
  Zero arm fully matches (a0 global -> $a0 home = target bytes) but the
  (a2+/-disc)*dist products become LOCALS@$v1 (nega0: 94,98 in 3) -> both /32
  temps get LITERAL {3} set_preferences (greg ";; 112/118 preferences: 3") ->
  steal $v1; disc drops to {4,7}/$a3. Artifacts: nega0.*, neg_a0split_18.diff.txt.
- [s3b] **divmodsi4 insns ARE preference/merge edges**: GCC 2.7.2 single_set()
  ignores sets whose dest is REG_UNUSED (rtlanal.c:601), and the dead mod-half
  always is — so set_preference fires with XEXP(div,0)=the DIVIDEND (local
  renumbering visible!) and expand_preferences merges quotient<->dividend/
  divisor. Corrects s2/s3's "dy->dy2->divres" chain model.
- [s3b] Corrected floor-5 {4}-route to the arm-2 quotient (fr120={4}):
  disc{4} (call-arg ashift fold) --[insn-148 (a2-disc) subu, disc dies,
  merge]--> 115 --[mult]--> 116 --[divmod]--> 120. dy/dy2 never carry {4}
  (expand is ONE forward pass; the dy-relay merge at insn 67 happens BEFORE
  disc gains {4} at insn 121). If 120 ALSO had {3}, find_reg's low-first
  pref-override would pick $v1 (target) over $a0 — a {3} injection into the
  arm-2 chain is a theoretical closer for the last 5.
- [s3b] Working-equilibrium invariant (measured across staged2/a2c/y1f vs
  negc/nega0 greg pref tables): disc MUST hold pref {3} (";; 105 preferences:
  3 4 7"); it both wins disc $v1 and — via regs_someone_prefers (prune:
  merges prefs of lower-priority CONFLICTING allocnos) — shields $v1 at the
  arm-1 /32 temp's ord-0 turn (it conflicts with disc because disc stays live
  through arm-1 into arm-2). In every neg-carrier form disc loses {3}.
  PROVENANCE of disc's {3} (and {7}) is NOT yet traced: not from its set
  insns (plus operands are unrenumbered globals; call-copy gives {2};
  call-arg gives {4}), not from any modeled expand merge (prefweb.py), and no
  other allocno holds pref 3 in y1f (so it is not a symmetric-merge residue).
  regs_someone_prefers construction read from source (global.c:846-900);
  FINDREG conflicts field = hard_reg_conflicts incl. post-allocation updates.
- [s3b] Existing-variable carrier axis EXHAUSTED (judge axis-1): disc=5(base),
  a2=8, neg=26, neg+a0split=18 measured; dx/dz/dist=callee-save homes, dy=$v0,
  dy2=$s0, a0=$a0 — the stage lw DEST byte IS the carrier's reg, so any
  wrong-home carrier mismatches bytes structurally; denom shares the zero-arm
  local-pool problem. Only a variable that is global, dead outside the two
  stage segments, and absent from every local pool can color $v1 — i.e. the
  banned fresh 2-write relay.
- [s3b] Tooling: s3/prefweb.py (pref-web simulator, single_set-aware),
  s3/mkdiff.sh (named-reg normalized diff), s3/dump_carrier.sh + frdump.sh
  (parameterized FINDREG/ALLOCDBG). greg dump ITSELF prints post-prune
  ";; N preferences:" + allocno conflict lists (dump_conflicts) — cheaper
  than FINDREG for pref surveys.

- [s3] floor 5 re-verified twice this session with candidate.c applied in src (168/168, 14 rules stripped)

- [s3] judge precondition met: neg's $a1 landing explained with FINDREG (hard {3} conflict from the local-pool rotation, not politeness)

- [s3] existing-variable carrier axis (judge axis-1) measured dead: disc 5 / a2 8 / neg 26 / neg+a0split 18 + structural exclusions for the remaining 6 variables

- [s3] fr120={4} only: if the arm-2 /32 temp ALSO carried {3}, find_reg's low-first override would pick $v1 (target) over $a0 — concrete closer candidate for the last 5

- [s3] greg -da dump prints post-prune ';; N preferences:' and allocno conflict lists (dump_conflicts) — cheap pref-survey surface for future sessions

- [s3] prefweb.py simulator + measured tables agree everywhere except disc's {3}/{7} provenance, which remains the one untraced channel
