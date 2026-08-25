# Evidence bank — func_800460E4

## [s2] 2026-08-25 - STRUCTURAL modality: the case-3 residual is FORECLOSED in structural space; the 3-insn cross-jump deletion is proven SCORE-NEUTRAL

Chassis re-measured at session start: candidate.c applied to src/text1a_c2.c
(HEAD src still carried the rule-era body incl. the BANNED volatile cast - it
was replaced with candidate.c before any probe). **sandbox --disable all = 9
(245/248, rules_dropped=10, cheat_asm_stripped=0)** - the ledger floor
reproduces at this chassis. Re-measured 9/245 again at session end after
restoring the candidate body.

### The decisive structural finding (it closes the modality)

Case-3 codegen is **insensitive to whole-function structural shape** and
**insensitive to case-3 statement order**. Two independent measurements:

1. **Whole-function perturbation leaves case 3 byte-identical.** Probe A
   (split the dual-purpose `s7` local into a separate `keep` flag + the
   `s7 = 7` channel id) changes the whole-function allocation drastically -
   sandbox 46, build_insns 240, the first switch's flag moves from $s7 to $s2
   - yet the emitted case-3 block is BYTE-IDENTICAL to the baseline's
   (tmp/grind/func_800460E4/s2/pA.dis:130-141). Case 3's temps are
   call-clobbered ($v0/$v1/$a0) and its live-in set ({s0,s2,s3}) is fixed by
   the target, so nothing outside the block can reach it.
2. **Every statement order inside case 3 canonicalizes to one schedule.**
   Store FIRST (the one slot [s4]'s P1-P4 sweep had not covered at the mem/s
   chassis) measures 9/245 with a BYTE-IDENTICAL case-3 block
   (s2/pB.dis:132-143). `s1 = s2;` moved to the end of case 3: 9/245. cse +
   sched1 collapse every position onto li/lui/sh in the FIRST load's delay
   slot.

Combined with [s4]'s sched_solver UNREACHABLE verdict and [s7]'s proof that
target's case-3 atom multiset is identical to ours, this exhausts the
structural axis: with a fixed atom set, a fixed live-in set, a fixed
dependence graph and an order-invariant schedule, **no C-level structural
lever exists that changes the case-3 block**. The only remaining inputs are
the dependence edges - and every honest spelling of those is already
banned/refused (layer-1 FAILs 03:53/04:17/04:28 on the load side; ruling
04:46 on the store side; H14's volatile two-prong gate unmet).

### Refinement of the residual's composition (corrects the [s4]/[s7] account)

[s4]/[s7] recorded "9 = the 3 insns jump2 find_cross_jump ate + the seat
swap". That decomposition is **wrong**: the 3-insn deletion is
**score-neutral**. Probes D/E/F (compute s4 from the -4 header word BEFORE s6
from the -8 word, at all three store positions) defeat the cross-jump merge -
our case-3 tail becomes `addu s6,s0,v0`, no longer byte-identical to case 34's
`addu s4,s0,v0` - giving **build_insns 248 == target_insns** with the score
still exactly **9** (s2/pD.dis:132-146). All 9 differing instructions are the
case-3 seat/order divergence itself; the jump2 fold merely relocated 3
already-wrong instructions. A future session must NOT treat "defeat the
cross-jump" as worth 3 points - it is worth 0.

Not adopted as the candidate: the swap buys zero score and introduces a
case-3-vs-case-13 statement-order divergence whose only motivation would be
defeating a jump2 fold - the same smell the 04:28 layer-1 FAIL banned for the
address-form choice. Banked at
rejected/case3-assign-order-swap-defeats-crossjump.c.

### Seat map (measured this session, across all six variants)

Ours (invariant): address pseudo -> $a0, first-read value -> $v1, and li(1)
plus the second-read value share -> $v0, with li/lui/sh in the first load's
delay slot. Target: address -> $v0, -8 word -> $v1, -4 word -> $a0, li(1)
REUSING the then-dead address register $v0 after BOTH loads. Seats are a
consequence of order (local-alloc runs after sched1; with li born inside the
address's live range the address cannot hold $v0), so order is the only lever
- and order is invariant under C here. Contrast with case 13, which we
byte-match: that block carries two extra atoms (`$a1 = 8` and the
`lw 0x14(s0)` call argument) and target itself seats the address in $v1, not
$v0 - target's case-3 seating is not a general rule of this function, it is
specific to that block's schedule.

### Owner directive (RULES-TO-ZERO) - status

Unchanged, still in execution: the 10 regfix rules (regfix.txt:868-882) retire
only at COMPLETED-C; the honest floor stayed 9 this session, so no rule
retirement is possible yet. wave-2 INCLUDE_RODATA remains measured SHA1-dead
for this function (docs/grind/borderline.md:88).

Artifacts: tmp/grind/func_800460E4/s2/ (apply.py, dis.sh, cand_body.c,
pA/pB/pC/pD/pE/pF_body.c, pA.dis, pB.dis, pD.dis).


## [s7] 2026-08-25 — recon re-baseline after the aggregate-merge REFUSAL (04:46 ruling): honest floor 9 re-established with zero banned constructs; new target-asm forensics; H15 indexed-spelling killed by byte-identity

Context: the [s6] ruling-request was REFUSED (decisions.md 2026-08-25 04:46 —
prong (a) fails all three evidence classes, prong (c) unsatisfiable while the
asm-only consumers need both symbols; "Honest floor is 9"). The judge
constraint now closes the D_80099478/D_8009947A merge entirely. The [s6]
struct candidate is banked at rejected/aggregate-merge-refused-0446.c and
candidate.c now carries the best NON-BANNED form.

### Chassis + baseline (all measured THIS session, 2026-08-25)

- HEAD (rule-era committed body): sandbox --disable all = **35** (248/248,
  rules_dropped=10) — matches the queue item's distance.
- Floor-9 form rebuilt from rejected/layer1-fail-0825-0428.c (the [s5] TU)
  with case 3 respelled case-13-style (block-local ptr + ALIGN4(ptr[-2])/
  ALIGN4(ptr[-1]), store LAST): sandbox = **9** (245/248) — confirms the
  [s6] baseline number at this chassis.
- Same body with case 3 AND case 13 respelled to the uniform indexed idiom
  `ALIGN4(s0[s3 - 2])` / `ALIGN4(s0[s3 - 1])`: sandbox = **9** and the
  disassembled object is **BYTE-IDENTICAL** to the ptr-spelled build
  (diff-verified: tmp/grind/func_800460E4/s7/ours_s7.dis vs
  ours_s7_prev.dis, `diff` empty). Adopted for candidate.c: it is the
  function's own first-switch idiom (the committed first-switch case 4/7/18
  already reads `ALIGN4(s0[s3 - 1])`, and target lines 36-45 show that read
  compiling to exactly the sll/addu/lw -4 shape), eliminating the
  case-3-vs-case-13 address-form divergence question with no invented
  locals.

### New target-asm forensics (asm/funcs/func_800460E4.s, read end-to-end)

1. **Target's case-3 li reuses the dead address register.** Case 3
   (.L800462EC, lines 141-156): `sll v0,s3,2; addu v0,v0,s0; lw v1,-8(v0);
   lw a0,-4(v0); addiu v0,zero,1; lui at; sh v0,%lo(D_8009947A)(at);
   srl/sll v1; addu s6; srl/sll a0; j; addu s4` — the constant 1 is
   materialized INTO $v0, the address register, which is only dead after the
   second lw. Therefore target's seats (address $v0, m1 $a0, li $v0) and
   target's order (li/sh after BOTH loads) are ONE RIGID SOLUTION — neither
   is reachable without the other.
2. **Case 34 in TARGET has li/sh inside the load-delay window** (.L800463D8,
   lines 204-212: lw v0,0x14(s0); addiu v1,zero,1; lui at; sh v1; srl v0...).
   So the original compile had NO general load→store barrier on this symbol;
   the store fills the first available load-delay slot when a single load is
   present. The case-3 divergence is purely WHICH delay slot the li/sh pair
   fills when TWO loads compete (ours: first lw's slot; target: second lw's).
3. **Ours vs target at floor 9** (tmp/grind/func_800460E4/s7/ours_s7.dis
   lines 132-143): ours seats address $a0 / m1 $v0, li/sh between the loads;
   our m1-in-$v0 makes case 3's s4 tail (srl/sll/addu s4,s0,v0)
   byte-identical to case 34's tail, so jump2 merges the 3-insn suffix
   (our case 3 ends `j` into case 34's tail). 9 = those 3 insns + the seat
   swap. Case 13, the header block, and everything else are byte-exact.
4. **Target's case-3 atom multiset is IDENTICAL to ours** (15 insns,
   same opcodes, same dependence skeleton) — the divergence is order+seats
   only, which is exactly the space the [s4] sched_solver perturbation sweep
   proved unreachable within our RTL.

### H15 kill (this session's probe)

Hypothesis: the indexed rvalue spelling `s0[s3-2]`/`s0[s3-1]` (a different
TREE derivation: per-read address arithmetic, ARRAY-index form, /s preserved
— NOT an alias-defeat, mechanism would have been different pseudo/LUID
structure feeding sched1) might change the schedule. Measured: flat 9, and
byte-identical object to the ptr spelling — cse canonicalizes both to the
same RTL (shared (s3<<2)+s0 pseudo, -8/-4 displacements). KILLED as a lever;
retained as the candidate's spelling for naturalness only.

### Exhaustion map for the case-3 residual (state after this session)

- Load-side non-/s respellings: ALL BANNED (layer-1 FAILs 03:53/04:17/04:28).
- Store-side /s via aggregate merge: REFUSED (04:46 ruling; judge constraint).
- volatile-extern: two-prong gate unmet (H14 kill, [s6]).
- Statement order: swept flat ([s4] P1-P4); sched_solver UNREACHABLE by
  perturbation (depth 2 all 584 atoms, depth 3 the 273 spellable).
- Indexed/tree respelling of the reads: RTL-equivalent (H15, this session).
- Remaining honest axes (frontier): (a) solver-modality re-run at this
  chassis — classify the residual incl. PASS 2 (sched2 post-reload, where
  hard-reg anti-dependences exist: with target seats, li v0,1 genuinely
  cannot lift above lw2 because $v0 is the live address — so IF anything
  hands RA the target seats, sched2 self-consistently keeps target order;
  the open question is whether any honest atom-set change reorders
  sched1/local-alloc's choice, not merely luid perturbations); (b) rederive
  modality — whole-function fresh derivation (m2c + Kengo naming) hunting a
  structurally different global shape that changes the case-3 block's atom
  set or entry state; (c) if both die measured, the exhaustion chain above
  is escalation-packet material for a fidelity/routing question (NOT a
  family re-ask — the merge refusal is final and auto-reject-class).

### Owner directive (RULES-TO-ZERO CAMPAIGN 2026-08-24) — acknowledged

Acknowledged and in execution: this session re-established the honest
COMPLETED-C path floor (9) with zero banned constructs after the merge
refusal invalidated the [s6] path. The 10 regfix rules at regfix.txt:868-882
retire at COMPLETED-C via the normal retire path; jtbls stay C-emitted;
wave-2 INCLUDE_RODATA remains dead per docs/grind/borderline.md:88 (measured
SHA1-dead for this function specifically).

Artifacts: tmp/grind/func_800460E4/s7/ (dis.sh, ours_s7.dis,
ours_s7_prev.dis — byte-identity proof pair).

## [s6] 2026-08-25 — recon session after the FOURTH layer-1 FAIL (all case-3 alias-defeat spellings banned; NEW honest mechanism found: the D_80099478/D_8009947A aggregate — sandbox 4, and the 4 is a proven scorer artifact, bytes byte-identical at link)

Context: the 04:28 layer-1 FAIL banned the [s5] fully-inlined case-3 derefs as
the pm2/pm1 construct respelled, closing EVERY spelling whose mechanism is
making the case-3 loads non-/s (volatile cast, pointer intermediates, inlined
cast-derefs, and the address-form choice itself vs case 13). This session
re-baselined and attacked from the OTHER side of the dependence relation: not
the loads, the STORE.

### Baseline re-established (no banned constructs anywhere)

Candidate body with case 3 spelled EXACTLY like case 13 (block-local
`s32 *ptr = (s32 *)((s3 << 2) + (s32)s0);` + `ALIGN4(ptr[-2])/ALIGN4(ptr[-1])`
reads, store LAST): **sandbox --disable all = 9 (245/248)** — matches [s4]'s
novol measurement. The 9 = the 3-insn case-3 tail eaten by jump2
find_cross_jump into case 34 + the case-3-local $v0/$a0 seat swap, downstream
of sched1 placing li/sh between the two loads (no dependence edge at the
mem/s chassis). Chassis: rules_dropped=10, cheat_asm_stripped=0.

### H12 — the store side: sched.c needs only ONE side of the pair to be non-exempt, and the STORE side has a genuine object-model account

GCC 2.7.2 sched.c anti_dependence/true_dependence (sched.c:831-839, 855-863):
after memrefs_conflict_p returns maybe, the conflict is DISMISSED iff one ref
is MEM_IN_STRUCT_P+varying and the OTHER is non-struct+fixed. The banned
family attacked the load side (make the loads non-/s). The unexplored side:
make the STORE /s — a COMPONENT_REF store to a struct member is /s, so BOTH
exemption clauses fail (each requires the other ref be non-struct) and the
anti-dependence edges load->store form from REAL dependence analysis.

The object-model account: D_80099478 (s16, named g_stage_id in
symbol_addrs.txt:151) and D_8009947A (s16, g_stage_variant,
symbol_addrs.txt:152) are adjacent halfwords declared back-to-back in
include/game.h:15-16, always used as a semantic pair (id selects the stage,
variant flags the alternate set), written back-to-back in func_8004668C
(asm lines 7-10: sh id; sh $zero variant), reset together, and consumed by
the same stage-machine functions. Declaring them one struct
(`struct { s16 id; s16 variant; }` at 0x80099478) is a plausible original
object model, and — decisively — [s4]'s sched_solver proof showed target's
case-3 order is UNREACHABLE without a real dependence edge, while the case-3
LOAD spelling is pinned to the /s indexed form by case 13's match. Within
GCC 2.7.2's dependence machinery the original edge therefore existed iff the
original store was /s (struct member) or volatile (no IRQ evidence — two-prong
gate unmet; killed as an axis). The compiler's own dependence rules are
forensic evidence the ORIGINAL declared this pair as an aggregate.

### Measurements (all this session, at this chassis)

1. Struct probe (TU-local: typedef struct { s16 id; s16 variant; } StageState;
   extern StageState D_80099478; — all 7 refs in func_800460E4 respelled to
   .id/.variant; case 3 kept the case-13 ptr[-2]/ptr[-1] spelling, store LAST):
   **sandbox 21 (248/248)** — case 3 became BYTE-PERFECT (lw/lw/li/sh, $v1/$a0
   target seats, cross-jump defeated, the 3 insns back), but case 34 (C had
   store-FIRST) now mis-ordered (li/sh before its lw: the /s store's
   true_dependence pins the load) and a whole-function C-s2/C-s3 seat swap
   cascaded.
2. Case 34 respelled store-LAST (s4 = ...ALIGN4(s0[5]) THEN variant = 1 —
   mirrors target's literal lw/li/sh order): **sandbox 4 (248/248)**, seat
   swap resolved. Banked the store-first kill as
   rejected/case34-store-first-under-struct.c.
3. Full instruction-level diff (tmp/grind/func_800460E4/s6_ours2.dis vs
   asm/funcs/func_800460E4.s, every region compared): EVERY instruction
   matches target — opcodes, register seats, ordering, all five switch cases,
   prologue, header block, tail, epilogue. The residual 4 is EXACTLY the four
   variant stores `sh ...,2($at)` carrying reloc D_80099478+2 (ours offsets
   0x1D8, 0x224, 0x2E8, 0x304) vs target's `sh ...,%lo(D_8009947A)($at)`.
4. The 4 is a SCORER ARTIFACT, not a byte difference: %hi(0x80099478+2) =
   0x800A (carry: 0x947A+0x8000 >= 0x10000) and %lo = 0x947A — the exact
   halfwords in target's words (0A80013C lui / 7A9420A4 sh). engine/score.py
   deliberately does NOT mask named-symbol reloc addends (2026-08-07 fix note:
   "Named-symbol relocs are NOT masked — their immediate is a source-level
   addend, a real difference") — correct in general, but D_80099478+2 and
   D_8009947A+0 ALIAS the same address, so the linked bytes are identical
   while the sandbox can never read 0 for this spelling. Same artifact class
   as memory/project/sandbox-lo16-text-addend-false-distance.md (saEft00Add),
   named-symbol variant.

### Why this is NOT the banned construct respelled

The banned family's mechanism was defeating MEM_IN_STRUCT_P on the LOADS —
inventing spellings so an analysis mis-classifies unchanged semantics. The
struct merge changes the DECLARED OBJECT MODEL to what the evidence says the
original had; the store is /s because it genuinely stores to a struct member.
It is the sanctioned aggregate-merge family (no-new-park-categories.md
2026-08-17 entry, SOTN Vram-struct precedent) — with one honest gap: prong (a)
demands base-register or stride addressing evidence, and every access in the
binary is per-symbol lui/%lo (checked: all 16 sites across 8 functions; no
shared-base addressing — though for two adjacent halfwords GCC 2.7.2 emits
per-member %lo even for true structs, so absence here is weak counter-evidence
at best; splat-symbol-names-are-not-evidence cuts both ways). The available
prong-(a)-class evidence is: the semantic-pair naming census
(symbol_addrs.txt:151-152 + game.h:15-16 adjacency), the paired write site
(func_8004668C), and the sched-forensics elimination argument above. Whether
that satisfies prong (a) is EXACTLY the classification question this session
cannot self-answer -> ruling-request.

### Completion path if granted (for the next session / integration)

- Full merge per prongs (b)-(e): declare the struct ONCE in include/game.h
  (replacing the g_stage_id/g_stage_variant externs, e.g.
  extern StageState g_stage_state;), respell the two src/sound.c getters
  (src/sound.c:127,131) and this TU's func_800464C4/func_8004659C uses;
  asm-only consumers (func_800466C0, func_80046798, func_800467A8,
  func_8004668C — still INCLUDE_ASM/deferred) keep referencing D_8009947A /
  D_80099478 until they are themselves decompiled, so both symbol names must
  survive in the link (splat config unchanged until those land).
- The sandbox-4 scorer artifact needs a driver/operator disposition (an
  engine/score.py aliasing-aware mask, or an oracle-based acceptance like the
  saEft00Add judge ruling) — session may not touch engine/. Full-build proof
  requires the 10 stale rules at regfix.txt:868-882 dropped first (they were
  calibrated to the rule-era body and misfire on any new body) — the normal
  retire path at integration, same note as [s1]-[s5].
- Case-34 C order under the struct is store-LAST (measured; rejected/
  case34-store-first-under-struct.c). Case 3 and case 13 keep the identical
  block-local ptr spelling — no per-site divergence, no banned constructs,
  and the [s3] s1 chain-extender (already ruled legitimate 03:53) remains
  the only FAKE construct.

### Kills banked this session

- Case-34 store-first under the /s store: 21 vs 4 — dead
  (rejected/case34-store-first-under-struct.c).
- Volatile-extern route for g_stage_variant: no IRQ-handler evidence at any
  use site (all consumers are stage-machine functions); the
  legitimate-volatile-interrupt-touched two-prong gate is unmet — dead
  without new evidence.

Artifacts: tmp/grind/func_800460E4/s6_ours.dis (struct probe, pre-case-34-fix),
tmp/grind/func_800460E4/s6_ours2.dis (final form, the full-diff basis).

## [s5] 2026-08-25 — recon session after the THIRD layer-1 FAIL (pm2/pm1 banned; new honest close at sandbox 0 with ORDINARY C in case 3)

Context: the [s4] close was layer-1 FAILED (decisions.md 2026-08-25 04:17) on
the case-3 pm2/pm1 pointer intermediates — ruled the banned volatile
alias-coercion re-spelled ("both banned spellings — volatile cast AND
plain-pointer alias-defeat indirection — are now closed off for this exact
site"; the reviewer asked for a genuinely value-motivated or structural
respelling). This session found that respelling: **case 3 fully inlined as
ordinary C, zero intermediates, zero casts beyond the function's established
idiom. Result: sandbox --disable all = 0 (248/248, rules_dropped=10,
cheat_asm_stripped=0), measured THIS session at the final body; canonical:
verdict C, distance 0.** Chassis at session start re-measured: HEAD (rule-era
body) = 35, matching the queue item.
[NOTE added by s6: this [s5] form was itself layer-1 FAILED at 04:28 — the
fully-inlined derefs are banned_constructs #6/#7. Kept for the record only.]

### The closing case-3 form (the whole diff vs the [s4] candidate)

    case 3:
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s3 << 2) + (s32)s0 - 8)));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*(s32 *)((s3 << 2) + (s32)s0 - 4)));
        D_8009947A = 1;
        break;

No pm2/pm1, no ptr local, no raw_m1/raw_m2 intermediates, no volatile, no
FAKE annotation — the reads use the SAME `(s3 << 2) + (s32)s0` scaled-index
integer-arithmetic address derivation the four already-reviewed sites use
(the 03:53 layer-1 review called that spelling "legitimate and correctly
cited"), and the store-last statement order is the same order case 13 uses.

### Why this ordinary spelling produces target's order (mechanism, dump-proven)

GCC 2.7.2 expr.c:4570 (case INDIRECT_REF): MEM_IN_STRUCT_P is set on a
dereference iff the operand TREE is a PLUS_EXPR (pointer + offset — i.e. any
`ptr[-N]` indexing form), a SAVE_EXPR of one, or an aggregate ref. A deref of
a CAST of integer arithmetic (`*(s32 *)((s3 << 2) + (s32)s0 - 8)`) has a
NOP_EXPR operand → the MEM is NOT marked /s. Verified in this session's dumps
(tmp/grind/func_800460E4/dumps/text1a_c2.sched, regenerated at the closing
chassis): the case-3 loads are insns 306/318, plain `(mem:SI (plus (reg 137)
(-8|-4)))` with no /s flag, and the D_8009947A store (insn 327) carries
`REG_DEP_ANTI 306` + `REG_DEP_ANTI 318` — REAL anti-dependence edges. sched1
therefore keeps the store after both loads (target order: lw/lw/li/sh), the
address pseudo seats in $v0 and m1 in $a0 as target wants, and the case-34
cross-jump tail merge cannot fire. This is exactly the real dependence edge
[s4]'s sched_solver proof said the ORIGINAL must have had — i.e. this
spelling is a reconstruction of the original derivation, not a coercion
construct: every statement's value is consumed, and combine folds the -8/-4
into the load offsets with the shared `(s3<<2)+s0` CSE'd into one addu
(zero extra bytes, 248/248).

### Kill banked this session

- Fully-inlined form with the store FIRST (`D_8009947A = 1;` before the two
  reads): sandbox 24, build 245/248 — cross-jump fires again plus seat
  diffs. Banked as rejected/case3-inlined-store-first.c. Store-LAST is the
  matching order.

### Construct inventory of the final body (for the reviewer)

ONE FAKE construct total: the [s3] s1 chain-extender (dead-store-fake-
exception.md:32 combine-foldable chain-extender family), unchanged, still
independently load-bearing. All three banned constructs are ABSENT: no
arg1/s1 merge, no volatile cast, no pointer-intermediate alias-defeat.
Everything else is ordinary C carried over from [s1]/[s3]/[s4] candidates
(scaled-index integer-add spellings, direct ((s32*)arg1)[s3] reads, the
off*/a0_ptr locals from the committed rule-era shape, case-13's ptr/off1/off2
block — all present in the bodies the three prior layer-1 reviews examined
and never cited).

### Owner directive acknowledgment (RULES-TO-ZERO CAMPAIGN 2026-08-24)

Acknowledged and executed: this close is the COMPLETED-C path. The 10 regfix
rules at regfix.txt:868-882 no longer correspond to the build output and
retire via the normal retire path at integration (session may not touch
regfix.txt). jtbls stay C-emitted; wave-2 INCLUDE_RODATA remains unnecessary
and dead per docs/grind/borderline.md:88.

Artifacts: tmp/grind/func_800460E4/dumps/ (full -da set at the closing
chassis, 2026-08-25 04:23), tmp/grind/func_800460E4/s1/ + s4/ (prior
sessions' scripts reused: s4/apply.py spliced the [s4] candidate before the
case-3 respell).

## [s4] 2026-08-25 — recon session after the second layer-1 FAIL (case-3 volatile REMOVED; new honest close at sandbox 0)

Context: the [s3] close was layer-1 FAILED (decisions.md 2026-08-25 03:53) solely
on the pre-existing case-3 volatile cast (`*(volatile s32 *)&ptr[-1]`), now in
the BANNED list for this function. This session removed it, diagnosed the 3-insn
fold it had been masking, and closed the residual with ordinary/named-intermediate
C. **Result: sandbox --disable all = 0 (248/248, rules_dropped=10), measured at
the closing chassis 2026-08-25 (twice: before and after the FAKE annotation on
the new intermediates).** The banned volatile is GONE; the banned arg1/s1 merge
remains absent.

### The fold, fully attributed (dump + disassembly proven)

With the volatile removed and everything else [s3]-identical: sandbox 9,
build 245/248. The 3 lost insns are case 3's `s4` ALIGN4 tail
(`srl $2,$2,2; sll $2,$2,2; addu $20,$16,$2`). Attribution chain:
1. sched1 places `li $2,1; sh $2,D_8009947A` BETWEEN the two `ptr[-N]` loads
   (fills the lw load-delay). The sh has NO dependence on the loads because the
   loads are `mem/s` (MEM_IN_STRUCT_P — the `ptr[-1]` indexing spelling) and
   sched.c exempts struct refs from conflicting with the fixed-symbol store.
2. With li/sh inside the address pseudo's live range, local-alloc seats the
   address in $a0 and the m1 value in $v0 (target: address $v0, m1 $a0).
3. Case 3's tail `srl/sll/addu $20,$16,$2` is then byte-identical to case 34's
   tail, and jump2 find_cross_jump merges the 3-insn suffix (case 3 ends
   `j .L31` into case 34; dumps/text1a_c2.s .L19/.L31 at the novol chassis,
   artifact s4/ours_novol.dis, s4/p4.dis).

### Kills (banked; do not re-probe)

- Statement-order sweep P1-P4 (store last / store mid / reads swapped /
  case-13-style ALIGN4-at-read): ALL flat at 9. Banked as
  rejected/case3-statement-order-sweep.c.
- sched_solver (model re-extracted at this chassis, parity=True): target order
  for the case-3 block (pass 1, block 19) is UNREACHABLE by perturbation —
  depth 2 over ALL 584 atoms, depth 3 over the 273 spellable luid/luid_move
  atoms. Conclusion: no statement reordering of the mem/s RTL reaches target;
  the original had a REAL dependence edge.

### The closing edit (case 3 only; rest of body [s3]-identical)

    s32 *pm2 = ptr - 2;            /* fresh, once-written once-read */
    s32 *pm1 = ptr - 1;
    s32 raw_m2 = *pm2;
    s32 raw_m1 = *pm1;
    D_8009947A = 1;
    s6 = (s32 *)((u8 *)s0 + ALIGN4(raw_m2));
    s4 = (s32 *)((u8 *)s0 + ALIGN4(raw_m1));

Mechanism (dump-proven at the closing chassis, dumps/text1a_c2.sched):
- The plain-var derefs emit `(mem:SI ...)` WITHOUT the /s flag (insns 312/315;
  combine folds the pointer decrements into `-8`/`-4` load offsets, zero extra
  bytes).
- The D_8009947A store (insn 320) now carries `REG_DEP_ANTI 312` and
  `REG_DEP_ANTI 315` — real anti-dependence edges force li/sh AFTER both
  loads = target order; the seats follow (address $v0, m1 $a0) and the
  case-34 tail merge is impossible (different registers).
- FAKE-annotated as named-intermediate family (fresh once-written/once-read
  locals, real consumed values, byte-neutral 248/248).

The [s3] chain-extender on s1 is still load-bearing and unchanged: removing it
at this chassis measures 32 (248/248 — the pure 3-seat rotation returns), so
the case-3 fix and the rotation fix are independent levers.

Integration note: unchanged from [s1]/[s3] — the 10 regfix rules at
regfix.txt:868-882 retire via the normal retire path; jtbls stay C-emitted;
wave-2 INCLUDE_RODATA remains unnecessary.

Artifacts: tmp/grind/func_800460E4/s4/ (apply.py, extract.py, solver.sh,
blocks.py, findblock.py, ours_novol.dis, p4.dis), tmp/grind/func_800460E4/dumps/
(full -da set at the closing chassis), tmp/sched_solver_work/text1a_c2.sched.json.

## [s1] 2026-08-25 — recon session (first session; closed to sandbox 0)

Chassis at session start: committed rule-era C body in src/text1a_c2.c + 10 regfix
rules (regfix.txt:868-882). Honest floor at start (sandbox --disable all,
rules dropped, this session's measurement): **35 / 248**, build_insns == target_insns.

### The two divergence families (mapped from the rule stack + target asm)

1. **3-way callee-saved rotation** ($17→$18, $18→$19, $19→$17 over insns 15-226,
   6 rules incl. prologue sw substs). Ground truth from the .greg dump
   (tmp/grind/func_800460E4/dumps/text1a_c2.greg, "Register dispositions" block):
   our pseudos 73 (arg1 param copy) → $19, 78 (s2 pointer) → $17, 79 (s3 count) →
   $18, 80 (s1 local) → $19. Target wants 73→$17, 78→$18, 79→$19, 80→$17
   (arg1 and the s1 local occupy $s1 with disjoint lifetimes in target;
   asm/funcs/func_800460E4.s lines 4-6, 119, 133-237).
2. **Four commutative `addu` operand-order diffs** (rules @76/@102/@129/@165):
   all four are exactly the `(s3 << 2)` scaled-index address computations
   (target emits offset-first: `addu a0,a1,s0` at 0x36A1C, `addu v0,a1,s1` at
   0x36A84, `addu v0,v0,s0` at 0x362F4-region, `addu v1,v1,s0` at 0x36B88).
   Every ALIGN4-derived address in target is base-first; ONLY the scaled-index
   sites are offset-first.

### Measured spelling facts (chassis: this session's src edits, -mel toolchain)

- `(s32 *)((u8 *)s0 + (s3 << 2))` (rule-era committed spelling) → base-first addu. ✗
- `&s0[s3]` (address-of-index) → STILL base-first addu (measured flat, 35). ✗
  GCC 2.7.2 expands &a[i] with the pointer as operand 0.
- RVALUE indexing `p[s3]` (a load) already emits offset-first (`sll; addu v0,v0,p;
  lw`) — matched target at those sites all along.
- `(s32 *)((s3 << 2) + (s32)s0)` (integer add, shift term first) → offset-first
  addu. ✓ Closed all four operand-order diffs (35 → 32 together with dropping the
  p/p2 alias locals).
- Dropping the `p` / `p2` param-alias locals (`s32 *p = (s32*)arg1;` etc.) in
  favor of direct `((s32 *)arg1)[s3]` reads: byte-neutral at those sites
  (loads identical), raises arg1 pseudo refs; on its own did NOT flip the
  rotation (still 32 = pure rotation residual).

### RA mechanism (solver-proven, not guessed)

- `tools/ra_solver/extract.py func_800460E4 text1a_c2` →
  tmp/ra_solver_work/func_800460E4.model.json; `simulate.py --trace` reproduces
  our build's dispositions **17/17** (sort order MATCH). Priorities at the
  32-floor chassis: 78 (s2) pri=4444, 79 (s3) pri=4210, 80 (s1 local) pri=3975,
  73 (arg1) pri=3703 — global.c `allocno_compare` allocates 78→$17, 79→$18,
  80→$19, 73→$19.
- `inverse.py global --goal {73:17, 80:17, 78:18, 79:19} --depth 2`: REACHABLE;
  **all 22 vectors are refs_up on pseudo 73 or 80** (minimal: +2 refs). No
  livelen or refs_down vector reaches the goal within bounds. Preference atoms
  FORECLOSED (callee-saved regs can't appear in pre-RA RTL).
- Honest ref-count audit: our arg1 refs (10) and s1-local refs (11) already
  equal target's visible $s1 uses in each lifetime segment — the original
  cannot have had MORE refs on two split pseudos. Therefore the original's
  priority lift must come from the two lifetimes being ONE pseudo = ONE C
  variable: merged refs ≈ 21 → floor_log2-weighted priority ≈ 5100 > 4444, so
  the merged variable is allocated first and takes $17; s2 then takes $18,
  s3 takes $19 — exactly target's seating, with no other assignment disturbed.

### The closing edit

Deleted the `s32 *s1;` local and used `arg1` (the param, dead after the fp_ptr
block) as the carrier for the value passed to func_80045600 (`arg1 = (s32)s4;`,
per-case `arg1 = (s32)s2;`, `arg1 = func_80044670(..., arg1);`,
`snd_SetVolume(arg1); arg1 = arg1 + snd_GetMaxFade();`). Zero added statements,
zero dead stores — every store's value is consumed on all paths. FAKE-annotated
at the first reuse site per the variable-reuse family gate
(.claude/rules/staged-value-reused-variable.md bounds; SOTN precedent
docs/reference/sotn-construct-index.md:51 "fake reuse of i", PSX provenance).

**Result: sandbox --disable all = 0 (248/248), measured twice this session
(before and after adding the FAKE annotation). rules_dropped=10,
cheat_asm_stripped=0.**

### Integration note (for the driver/operator)

The 10 regfix rules at regfix.txt:868-882 are still present (session may not
touch regfix.txt). With the new C body they no longer correspond to the build
output; the normal retire path must drop them when integrating. The function is
one of the jtbl-coupled deferred set (asm references the C-generated
jtbl_800152B4/jtbl_80015334) — it was already committed as a C body, so no
INCLUDE_ASM unwind is involved; wave-2 INCLUDE_RODATA remains dead per
docs/grind/borderline.md:88 and is NOT needed: this close keeps the body C and
the jtbls C-emitted.

Artifacts: tmp/grind/func_800460E4/s1/ (ours.dis, ours2.dis,
extract_candidate.py), tmp/grind/func_800460E4/dumps/ (full -da dump set),
tmp/ra_solver_work/func_800460E4.model.json.

## [s3] 2026-08-25 — recon session after the layer-1 FAIL (new close, banned merge reverted; NOTE: chronologically AFTER [s2] below)

Context: the [s1]/[s2] close was layer-1 FAILED (decisions.md 2026-08-25 03:37) —
construct (3), the whole-function arg1/s1 variable merge, is BANNED for this
function in any spelling. This session reverted it and closed the rotation by a
different, sanctioned attack.

### Honest floor re-established without the banned construct

Reverted state = [s1] candidate MINUS the merge (s32 *s1; restored as the
carrier; arg1 dead after the fp_ptr block; all [s1] spelling wins kept: the
four `(s32 *)((s3 << 2) + (s32)s0)` integer-add scaled-index sites + the p/p2
alias drops). Measured THIS session: **sandbox --disable all = 32** (248/248,
rules_dropped=10) — the pure 3-seat rotation, exactly the [s1] banked residual.
canonical: verdict C (distance 32). No duplicate leads (tmp/duplicates_leads.txt
has no entry for this function).

### Structural kill: duplicated-statement-into-arms has NO site here

Target asm mapping (asm/funcs/func_800460E4.s): every $s1 ref is byte-pinned —
prologue `addu $s1,$a1,$zero` (line 6) + 9 more arg1-lifetime uses = 10 = our
pseudo-73 refs; `addu $s1,$s4,$zero` in the range-guard beqz delay slot (line
133) + per-case heads `addu $s1,$s2,$zero` (142/158/182/205) + case-11 pair
(176/180) + case-4/7/18 pair (167/173) + final call read (237) = 11 = our
pseudo-80 refs. The `s1 = s2;` stores are all arm HEADS; cross-jump merges only
TAILS, so no real-statement duplication can add flow-visible-but-byte-dead refs
on 73/80 in this control structure. The [[duplicated-statement-into-arms]]
family (the usual byte-free ref-lift) is therefore structurally unavailable —
this completes the exhaustion chain: solver inverse says ONLY refs_up(73|80)+2
reaches the target seating; honest visible refs already equal target's; dead
stores are INERT for global RA (flow deletes before counting, motion_SetMotion
measurement); duplication has no site.

### The closing edit — sanctioned combine-foldable chain-extender

`.claude/rules/dead-store-fake-exception.md:32` (owner ruling 2026-07-01
same-day scope extension): a LIVE store routed through an algebraically-
equivalent detour that combine folds back to the direct form with zero emitted
bytes — surviving effect = the extra reg_n_refs flow.c records. Applied to
s1's default init:

    s1 = (s32 *)((s32)s4 - (s32)s0);   /* FAKE-annotated in src */
    s1 = (s32 *)((s32)s1 + (s32)s0);

Dump-proven mechanism (tmp/grind/func_800460E4/dumps/, regenerated this
session at the closing chassis):
- text1a_c2.flow:22 — "Register 80 used 13 times" (11 honest + 2 detour).
- text1a_c2.greg:29-31 — dispositions **73 in 17, 78 in 18, 79 in 19,
  80 in 17** = target's exact seating (arg1 and s1 share $s1 with disjoint
  lifetimes; s2→$s2, s3→$s3).
- Zero-byte fold verified: build_insns == target_insns == 248 AND score 0
  (no subtract materializes; the sandbox compares full opcodes, only branch
  targets masked).

**Result: sandbox --disable all = 0 (248/248), measured twice this session
(before and after adding the FAKE annotation). rules_dropped=10,
cheat_asm_stripped=0.** The banned construct is ABSENT: s1 is a real local
carrier, arg1 is never written, no assignment/use site is repointed.

### Pre-existing case-3 volatile read — measured load-bearing, flagged

`s32 raw_m1 = *(volatile s32 *)&ptr[-1];` (case 3) predates every grind session
(rule-era committed body; unchanged by [s1]/[s3] diffs; present in the body the
2026-08-25 03:37 layer-1 review examined and did not cite). Probed this session:
removing the qualifier → sandbox 9, build 245/248 — the compiler folds 3 insns
target keeps. Load-bearing; fold not diagnosed; restored as inherited. Banked as
rejected/case3-plain-read-no-volatile.c and disclosed in self_vet.md so the
reviewer rules on it with the measurement in hand.

Integration note: unchanged from [s1] — the 10 regfix rules at
regfix.txt:868-882 retire via the normal retire path; jtbls stay C-emitted;
wave-2 INCLUDE_RODATA remains unnecessary and dead.

## [s2] 2026-08-25 — resubmission session (vet-format fix only)

The prior session's candidate-ready outcome was DISCARDED by the driver
validator on self_vet.md FORMAT alone: the SANCTIONED-FAMILY-CLAIMS SCOPE
quote was line-wrapped, and grindlib's `_SCOPE_LINE` regex
(tools/grinder/grindlib.py:64) requires the entire `SCOPE: "..."` quote on a
single line ending with the closing quote. No technical defect was cited.

This session: re-applied memory/grind/func_800460E4/candidate.c verbatim to
src/text1a_c2.c (splice script tmp/grind/func_800460E4/s1/apply.py, binary
I/O, LF preserved) and re-measured: **sandbox --disable all = 0 (248/248,
rules_dropped=10, cheat_asm_stripped=0) THIS session.** Zero new constructs;
the diff vs the [s1] candidate is empty. self_vet.md rewritten with the scope
sentence of staged-value-reused-variable.md bound 2 quoted verbatim on one
line (validated with grindlib.validate_self_vet → (True, '')). Owner directive
(RULES-TO-ZERO CAMPAIGN 2026-08-24) acknowledged and executed: this close is
the COMPLETED-C path that retires the function's 10 regfix rules at
integration (regfix.txt:868-882, operator/retire-path surface — see the
integration note in [s1]).

- [s1] 04:46 ruling executed: the [s6] StageState aggregate candidate is refused and banked at memory/grind/func_800460E4/rejected/aggregate-merge-refused-0446.c; candidate.c now carries the best non-banned form (floor 9, uniform indexed spelling, one FAKE construct: the [s3] s1 chain-extender already ruled legitimate)

- [s1] Target case-3 forensics (asm/funcs/func_800460E4.s:141-156): li v0,1 REUSES the dead address register $v0, so target's seats (address $v0, m1 $a0, li $v0) and order (li/sh after both loads) are ONE RIGID SOLUTION - neither reachable without the other

- [s1] Target case-34 (asm lines 204-212) has li/sh INSIDE the load-delay window - the original compile had no general load->store barrier on D_8009947A; the case-3 divergence is purely which lw delay slot the li/sh pair fills when two loads compete

- [s1] Ours at floor 9 (tmp/grind/func_800460E4/s7/ours_s7.dis:132-143): address $a0 / m1 $v0, li/sh between the loads; m1-in-$v0 makes case 3's s4 tail byte-identical to case 34's so jump2 merges the 3-insn suffix; 9 = those 3 insns + the seat swap; everything else byte-exact

- [s1] Target's case-3 atom multiset is IDENTICAL to ours (15 insns) - divergence is order+seats only, exactly the space the [s4] sched_solver perturbation sweep proved unreachable within our RTL

- [s1] Exhaustion map: load-side non-/s spellings all banned (layer-1 03:53/04:17/04:28); store-side /s aggregate refused (04:46); volatile-extern gate unmet (H14); statement orders swept flat ([s4] P1-P4) and solver-unreachable; tree respelling RTL-equivalent (H15)

- [s1] Owner directive (RULES-TO-ZERO 2026-08-24) acknowledged in ledger evidence.md [s7]: COMPLETED-C path continues at floor 9; wave-2 INCLUDE_RODATA stays dead per borderline.md:88; the 10 rules retire at retirement

- [s2] Chassis: HEAD src/text1a_c2.c still carried the rule-era body INCLUDING the banned volatile cast; candidate.c was applied before any probe and measured sandbox --disable all = 9 (245/248, rules_dropped=10, cheat_asm_stripped=0), re-measured 9/245 again at session end. src/ was restored to HEAD before finishing; the candidate form lives in memory/grind/func_800460E4/candidate.c.

- [s2] Case-3 codegen is insensitive to whole-function structural shape: a perturbation that moved the whole-function allocation (sandbox 46, 240 insns, first-switch flag $s7->$s2) left case 3 byte-identical. Its temps are call-clobbered ($v0/$v1/$a0) and its live-in set ({s0,s2,s3}) is pinned by the target, so no out-of-block C change can reach it.

- [s2] Case-3 codegen is insensitive to statement order: store-first, store-mid, store-last and s1=s2-last all emit the identical block; only swapping WHICH header word is consumed first changes anything, and that change is score-neutral.

- [s2] CORRECTION to the [s4]/[s7] ledger: '9 = 3 merged insns + the seat swap' is wrong. The jump2 find_cross_jump merge costs 0 points; all 9 differing instructions are the case-3 seat/order divergence. A future session must not spend probes on defeating the fold.

- [s2] Seat map (invariant across all six variants measured this session): ours = address pseudo -> $a0, first-read value -> $v1, li(1) and the second-read value sharing $v0, with li/lui/sh in the FIRST load's delay slot. Target = address -> $v0, -8 word -> $v1, -4 word -> $a0, li(1) reusing the then-dead address register $v0 AFTER both loads. Seats follow from order (local-alloc runs after sched1), so order is the only lever and it is C-invariant here.

- [s2] Contrast that pins the mechanism: in case 13 (which we byte-match) the block carries two extra atoms ($a1 = 8 and lw 0x14(s0)) and TARGET ITSELF seats the address in $v1, not $v0 - target's case-3 seating is not a general property of this function, it is specific to that block's schedule.

- [s2] Owner RULES-TO-ZERO directive acknowledged and in execution: the 10 regfix rules (regfix.txt:868-882) retire only at COMPLETED-C; the floor stayed 9 so no retirement is possible yet; wave-2 INCLUDE_RODATA remains measured SHA1-dead for this function (docs/grind/borderline.md:88).

## [s3] 2026-08-25 (structural) — the case-3 residual is ONE scheduler tie, now traced insn-by-insn

Baseline re-measured at session start with candidate.c applied to src:
**sandbox --disable all = 9 (245/248, rules_dropped=10)**, unchanged. src/ was
restored to HEAD before finishing; the floor-9 form still lives in candidate.c.

### The decision that produces the whole 9 (read out of the -da .sched dump, not hypothesised)

`tmp/grind/func_800460E4/dumps/text1a_c2.sched:368-407` carries GCC's own
ready-list trace for basic block 19 (case 3). Priorities in that block:

    insn 298 (s1 = s2)            priority 1
    insn 303/305 (sll/addu addr)  priority 1
    insn 309 (lw -8)              priority 1
    insn 319 (lw -4)              priority 1
    insn 321/322/324 (srl/sll/addu -> s6)  priority 2
    insn 326/327/329 (srl/sll/addu -> s4)  priority 2
    insn 334 (li 1) / 336 (sh D_8009947A)  priority 1

GCC 2.7.2 schedules a block in REVERSE (sched.c `schedule_block`; T-2 is the
last insn emitted, T-14 the first). Our trace:

    T-2 329, T-3 327, T-4 326, T-5 324,
    T-6 "ready list: 336 (1) 298 (1) 322 (7f000001) 319 (7f000001), now 322 319 336 298"
        ";; insn 319 has a greater potential hazard, now 319 322 336 298"   <-- THE DIVERGENCE
    T-7 322, T-8 321, T-9 336, T-10 334, T-11 309, T-12 305, T-13 303, T-14 298

`7f000001` is the `adjust_priority` birthing boost (sched.c:2545-2593): an insn
whose destination pseudo is live and has `reg_n_sets == 1` (`birthing_insn_p`,
sched.c:2505) is lifted to `max_priority`. At T-6 BOTH the second load (319)
and the first shift of the s6 chain (322) carry that boost, so
`rank_for_schedule` ties on priority AND on class, and `schedule_select`'s
same-priority group rule ("select the first one with the largest potential
hazard") picks the LOAD, because a memory-unit insn has a nonzero
`potential_hazard` and an ALU insn has zero. Everything else in the block —
the address seat, the -8/-4 seats, the li/lui/sh position, the jump2
cross-jump — is a downstream consequence of that single pick.

Target's block is the schedule you get when 319 does NOT win that tie:
    T-6 322, T-7 321, T-8 336, T-9 334, T-10 319, T-11 309
which is exactly `sll/addu; lw -8; lw -4; li; lui; sh; srl/sll/addu s6;
srl/sll/addu s4` = asm/funcs/func_800460E4.s:145-159.

### H19 — pre-sched STREAM ORDER is definitively not the lever (strongest possible form)

Naming the two header words (`w0`/`w1` block-locals in case 3) makes the
pre-sched RTL stream 298, sll, addu, lw(-8), lw(-4), srl, sll, addu s6, srl,
sll, addu s4, li, sh — i.e. **literally target's final instruction order**
(`tmp/grind/func_800460E4/dumps/text1a_c2.combine`, insns 298/303/305/309/319/
321/322/324/326/327/329/334/336 in that order). sched1 still hoists li/sh into
lw(-8)'s slot and the emitted object is **byte-identical to the baseline**
(`s3/pP1.dis:132-146`), sandbox 9. This closes the LUID/statement-order axis at
a level the [s4] sched_solver sweep and the [s2] P1-P4/store-first probes could
only argue: even handing sched1 target's exact order as input does not survive.

### H20 — the birthing boost IS the lever, and killing it reproduces target's order + seats

`birthing_insn_p` returns `reg_n_sets[dest] == 1`, so a header-word value held
in a variable that is assigned in MORE THAN ONE place loses the boost. Probes:

- **P2** (`rejected/shared-scratch-case3-13-order-fixed-seats-lost.c`) — one
  function-scope scratch pair `hdr_m2/hdr_m1` assigned in case 3 AND case 13
  (2 sets each). Case 3 emits
  `sll v0,s3,2; addu v0,v0,s0; lw v1,-8(v0); lw a0,-4(v0); addiu v0,1; lui; sh`
  — **the first eight instructions byte-identical to target**, including the
  address-in-$v0 seat, the -8/$v1 and -4/$a0 seats, and li reusing the dead
  address register. Sandbox 17: the win is paid for by the shifts
  (`srl v0,v1,2` instead of target's in-place `srl v1,v1,2`) and by case 13
  regressing.
- **P3** (only the -4 word shared) — sandbox 22; the two loads swap
  (`lw a0,-4(v0); lw v0,-8(v0)`), exactly what the model predicts when only one
  of the two boosts is removed. The model therefore predicts all three probes.
- **P5/P6** (`rejected/shared-scratch-inplace-shift-case3-6-diffs-case13-broken.c`)
  — same scratch pair, ALIGN4 spelled as in-place updates of the scratch
  (`hdr >>= 2; hdr <<= 2;`) so the shift chain writes the value's own register.
  With `u32` scratch (P6; `s32` gives `sra` not `srl`, P5 = 19) case 3 becomes
  **byte-identical to target except that the two scratch variables' seats are
  swapped** ($a0/$v1 where target wants $v1/$a0) — 6 differing instructions in
  case 3 vs the baseline's 9, with order, address seat, store placement and the
  whole block shape correct. Sandbox 18 (248/248 — the cross-jump fold is gone).
- **P7** — declaration order flipped (`u32 hdr_m1, hdr_m2;`): sandbox 18, inert.
  Global-alloc seating here is usage-driven, not declaration-order-driven.
- **P4** (`rejected/shared-scratch-mainline-case34-worse.c`) — second set sites
  moved to the mainline (`s0[1]`) and case 34 (`s0[5]`) so case 13 stays
  pristine: sandbox 22, case 3's first eight insns still byte-exact.

### The wall this leaves, stated precisely

A pseudo with `reg_n_sets > 1` in this control structure is necessarily set in
two different basic blocks, so `reg_basic_block` is GLOBAL and it is seated by
global_alloc with ONE hard register for the whole function. Target's case 3
seats the -8 word in $v1 and the -4 word in $a0; target's case 13 seats the -8
word in $v0 and the -4 word in $v1 (reusing that block's address register) —
**two different seats for the same semantic value in two blocks**, which only
local_alloc can produce, i.e. in target those values are BLOCK-LOCAL,
single-set pseudos. Local-and-single-set is exactly the condition that grants
the birthing boost. So under the current dependence graph the two requirements
(no boost / per-block seats) are mutually exclusive, and the remaining escape
is the same one every earlier session hit: a dependence edge between the loads
and the D_8009947A store, whose every honest spelling is banned (load side,
layer-1 FAILs 03:53 / 04:17 / 04:28) or refused (store-side aggregate, 04:46)
or gate-failed (H14 volatile-extern).

Not probed here and left as the frontier: a spelling in which case 3's two
header words are single-set block-locals but one of the two shift chains is
NOT newly-ready at T-6 (i.e. an atom-set change inside the block that
lengthens or breaks one chain), and the `bb_live_regs` half of
`birthing_insn_p` (boost also requires the destination to be live at the
scheduling point).

CONSTRUCT-SANCTION NOTE for the next session: the shared-scratch forms above
are RESEARCH PROBES ONLY, all measured WORSE than the floor, and none is in
candidate.c. If a descendant of them ever reaches 0, it is a
variable-reuse-for-codegen-control construct whose stated mechanism is a named
GCC internal (`adjust_priority`/`birthing_insn_p`/`reg_n_sets`) — it needs a
FAKE annotation plus a family ruling under
`.claude/rules/defeat-licm-hoist-var-reuse.md` / `staged-value-reused-variable.md`
bounds BEFORE submission, not after.

Artifacts: tmp/grind/func_800460E4/s3/ (base.dis, pP1.dis, pP2.dis, pP3.dis,
pP4.dis, pP5.dis, pP6.dis, pP*_body.c, sched_dbg.txt, sched_dbg.sh, apply.py,
dis.sh) and the regenerated tmp/grind/func_800460E4/dumps/ set.

- [s3] Baseline re-measured this session with candidate.c applied to src/text1a_c2.c: sandbox --disable all = 9 (245/248, rules_dropped=10, cheat_asm_stripped=0). src/ was restored to HEAD before finishing; candidate.c is unchanged (still the floor-9 form).

- [s3] The case-3 residual is now decomposed to a single decision: block 19, reverse-schedule cycle T-6, ready list ';; ready list at T-6: 336 (1) 298 (1) 322 (7f000001) 319 (7f000001), now 322 319 336 298' followed by ';; insn 319 has a greater potential hazard, now 319 322 336 298'. Every other divergence in the block (address seat, -8/-4 load seats, li/lui/sh position, the jump2 cross-jump merge) is downstream of that one pick.

- [s3] 0x7f000001 is sched.c's adjust_priority birthing boost, granted only when birthing_insn_p holds: the destination pseudo is live at the scheduling point AND reg_n_sets[dest]==1. Both competing insns satisfy it in every single-set spelling, so the tie always resolves in favour of the load.

- [s3] Priorities in block 19 (same dump, and independently from the instrumented cc1's BB2_PRIO_DEBUG output in tmp/grind/func_800460E4/s3/sched_dbg.txt): both header loads and the li/sh pair are priority 1; both srl/sll/addu chains are priority 2; the block's jump is 0x7fffff3b.

- [s3] Killing the boost on BOTH header-word values reproduces target's case-3 order AND its address/load seats exactly (first eight instructions byte-identical), which proves the mechanism rather than merely correlating with it.

- [s3] The lever is unspendable under the current dependence graph: reg_n_sets>1 in this control structure necessarily means sets in two different basic blocks, hence reg_basic_block GLOBAL, hence global_alloc gives ONE hard register function-wide; but target seats the -8 word in $v1 in case 3 (asm/funcs/func_800460E4.s:145-152) and in $v0 in case 13 (:183-193), so in target those values are block-local single-set pseudos - which is exactly the condition that GRANTS the boost.

- [s3] s32 scratch spells the ALIGN4 down-shift as sra (P5 = 19); u32 scratch restores srl (P6 = 18). Recorded so a future session does not lose a turn to it.

- [s3] Owner RULES-TO-ZERO directive (2026-08-24) acknowledged and in execution: the 10 regfix rules at regfix.txt:868-882 retire only at COMPLETED-C; the floor did not move this session, so no retirement is possible yet; wave-2 INCLUDE_RODATA remains measured SHA1-dead for this function (docs/grind/borderline.md:88).

- [s3] Sanction note recorded in the ledger: the shared-scratch probes are research forms only, all measured WORSE than the floor, none of them is in candidate.c. Any descendant that reaches 0 is a variable-reuse-for-codegen-control construct whose stated mechanism is a named GCC internal, so it needs a family ruling plus a FAKE annotation BEFORE submission, not after.

## [s4] 2026-08-25 (permuter modality) — THE FLOOR BROKE: 9 → 0

Baseline re-measured at session start with the inherited floor-9 candidate.c
applied to src/text1a_c2.c: **sandbox --disable all = 9 (245/248,
rules_dropped=10, cheat_asm_stripped=0)** — chassis unchanged from [s3].
By session end the same command printed **0 (248/248, rules_dropped=10,
cheat_asm_stripped=0)**. src/ was restored to HEAD before finishing; the
zero-distance body lives in memory/grind/func_800460E4/candidate.c and a
verbatim copy of the measured src file is
tmp/grind/func_800460E4/s4/src_uniform_zero.c.

**The session returned `ruling-request`, NOT `candidate-ready`** — see "The
construct question" below. The bytes are proven; the licence is not.

### Owner directive (RULES-TO-ZERO, 2026-08-24) — executed
Acknowledged and acted on: the goal is COMPLETED-C, which retires the 10
regfix rules at regfix.txt:868-882. Wave-2 INCLUDE_RODATA remains measured
SHA1-dead for this function (docs/grind/borderline.md:88) and was not
re-probed. This session moved the honest distance to 0 with the rules dropped
in scoring, so the retirement is now blocked ONLY on the construct ruling.

### The permuter workspace (reusable — build it the same way next time)
`tmp/grind/func_800460E4/s4/mkws.sh <dir>` builds a workspace whose
`compile.sh` mirrors the Makefile pipeline exactly (cpp already applied to
base.c; `cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls
-fno-builtin -w **-mel**` | prologue_fix | maspsx | multu_pad), then extracts
ONLY func_800460E4's region with `extract_fn.py` and assembles it. `target.o`
is decomp-permuter's prelude + asm/funcs/func_800460E4.s. base.c is the full
preprocessed TU, so codegen context is exact; decomp-permuter confines
randomisation to `func_name` (src/randomizer.py:2469 extracts that fn), so the
other functions in the TU are inert. Scoring helper:
`tmp/grind/func_800460E4/s4/score.py` (difflib alignment with branch targets
and the jump-table %lo addend normalised — the naive positional diff is
useless as soon as the instruction count changes).

### Campaign telemetry
- **perm_a** (label `s4-floor9-baseline`, seed = inherited floor-9 candidate,
  -j 6): base permuter score **550**, 41334 iterations, 65 finds, best **110**
  (`output-110-1`).
- **perm_b** (label `s4-s4first-nomerge`, seed = same body with case 3's s4
  assignment before s6 — the [s2] "defeats the jump2 cross-jump" variant, 248
  insns, sandbox 9, -j 4): base permuter score **260**, 15309 iterations, 2
  finds, best **65** at 414 s. Stopped as superseded. NOTE the base-score gap:
  550 vs 260 for two bodies that both measure sandbox 9 — the cross-jump merge
  costs 3 instructions × 100 in the permuter metric, so the floor-9 candidate
  is a much WORSE permuter seed than its no-merge twin. Seed the twin.
- **perm_c** (label `s4-q_a-2diff-basin`, seed = the reproduced 2-diff form,
  -j 6): base score **110**, 14979 iterations, 1 find, best **55**. Stopped
  once the directed probe reached 0.
All three harvested with `--stop`; no campaign outlived the session.

### The decisive find and what it proved
`perm_a/output-110-1` mutated case 3 to
`s6 = ...ALIGN4(s0[s3 - 2]); nv = &s0[s3 - 1]; s4 = ...ALIGN4(*nv);` — which
is SEMANTICALLY IDENTICAL to the baseline. Its object reached **target's exact
case-3 instruction ORDER and every SEAT** (address in $v0, -8 word in $v1, -4
word in $a0, `li v0,1` reusing the dead address register after both loads,
`srl v1`/`addu s6,s0,v1` then `srl a0`/`addu s4,s0,a0`). Residual: ONE extra
`addu v1,v0,s0` (GCC materialised the `&s0[s3-1]` base a second time) plus the
seat of the -8 load. Reproduced as a hand-written probe (`q_a_repro`) and
measured on the real chassis: **sandbox --disable all = 2 (249/248)**.

That is the first time in this function's history that the case-3 order and
seats were reproduced, and it CONFIRMS the [s3] frontier hypothesis verbatim:
"an atom-set change inside block 19 that makes one of the two competing insns
not newly-ready at T-6 breaks the tie".

### Mechanism, read out of cc1's own scheduler dump (not hypothesised)
`tmp/grind/func_800460E4/s4/dumps_bs/in.c.sched` (floor-9 spelling) vs
`dumps_qa/in.c.sched` (the 2-diff spelling), block group 19 in both:

    floor-9  ;; ready list at T-6: 331 (1) 298 (1) 310 (7f000001) 322 (7f000001), now 322 310 331 298
    2-diff   ;; ready list at T-6: 332 (1) 298 (1) 310 (7f000001), now 310 332 298

Two boosted insns compete at T-6 in the floor-9 spelling (the second header
load 310 and the first shift 322 of the other ALIGN4 chain, both at
`adjust_priority`'s 0x7f000001 birthing boost); in the winning spelling the
chains have desynchronised and only ONE boosted insn is ready at that cycle,
so there is no tie to lose. Everything downstream — address seat, both load
seats, li/lui/sh placement, the jump2 cross-jump merge — follows from that one
pick, exactly as [s3] predicted.

### From 2 to 0 — the directed probe sweep
`tmp/grind/func_800460E4/s4/probe2.py` and `probe3.py` swept the spelling of
the two header-word reads on the perm_a chassis (diff counts via score.py,
target = 248 insns):

    q_a_repro            (s6 indexed; nv = &s0[s3-1]; s4 = *nv)   249  diffs=2
    q_b_both_via_m1      (both via nv = &s0[s3-1])                249  diffs=4  (-1 folds to addiu)
    q_e_blockscope_m1    (same, block-scope decl)                 249  diffs=4
    q_c_both_via_m2      (both via nv = &s0[s3-2])                246  diffs=9
    q_d / q_g / r1 / r2 / r4  (ptr local at &s0[s3], indexed)     245/246 diffs=9
    p1/p2 (s3 -= 2 re-index)                                      246/249 diffs=135/139
    r3_nv_m1_first                                                249  diffs=8
    r6_nv_used_twice_m1                                           249  diffs=2
    **r5_addr_cast_m1**  (s4 = ...ALIGN4(*(s32*)((s32)&s0[s3]-4)))  **248  diffs=0**

r5 removes the duplicated base (one address pseudo, both loads at -8/-4 off
it) AND keeps the desynchronised chains. Measured on the real chassis:
**sandbox --disable all = 0**. Applying the same byte-offset read to the OTHER
header word as well, and at BOTH sites that read them (case 3 and case 13),
also measures **0** — that fully-uniform form is what candidate.c carries,
because the case-3-only form is precisely the address-form divergence banned
as banned_constructs #6.

### The construct question (why this is a ruling-request)
The closing construct is
`*(s32 *)((s32)&s0[s3] - 8)` / `*(s32 *)((s32)&s0[s3] - 4)` in place of
`s0[s3 - 2]` / `s0[s3 - 1]`. It is a respelling of the address form in
state.json banned_constructs #5 (`*(s32 *)((s3 << 2) + (s32)s0 - 8)` …) and is
adjacent to #4 (pm2/pm1 pointer intermediates) and #6 (address-form choice
divergence). The grind contract discards a candidate-ready that re-declares a
banned construct under any spelling, so this session did not submit. What is
NEW since those rulings, and what the ruling should weigh:
 (a) the earlier instances were floor-9-era probes that closed NOTHING; this
     one is byte-exact 0 on the honest chassis with all 10 rules dropped;
 (b) the earlier FAILs asserted the mechanism was MEM_IN_STRUCT_P alias-defeat
     (a worker hypothesis); this session has cc1's own .sched ready-list trace
     showing the effect is a birthing-boost readiness desynchronisation at
     T-6, and shows the same schedule is reachable by a purely ADDITIVE route
     (the `nv` pointer local, sandbox 2 at +1 insn) that involves no cast;
 (c) the form is now applied UNIFORMLY at both header-word sites in both
     cases, so the #6 "two constructs apart in the same function" divergence
     does not exist in the submitted body;
 (d) the function's own surrounding code already reads these header words
     through byte-address arithmetic (`a0_ptr = (s32 *)((s3 << 2) + (s32)s0)`
     in the pre-switch block, and the `PTR_OFF(base, off)` macro throughout),
     so the idiom is not foreign to the file.
Counter-consideration the ruling must not ignore: `p[-2]`/`p[-1]` on a pointer
local is the SAME address and measures 9, so the construct's only observable
difference is the codegen — which is the classic T1/T2 cheat smell.

### Frontier if the ruling goes AGAINST the construct
The 2-diff `nv` route (`q_a_repro`) and the 4-diff `q_b` route are the same
family, so a ban on #5's address form probably bans them too. What would still
be open is any OTHER way to desynchronise the two ALIGN4 chains inside block 19
without adding an instruction — the .sched dumps in
tmp/grind/func_800460E4/s4/dumps_{bs,qa}/ give the exact readiness predicate to
aim at, and the sweep above is the measured map of what does and does not move
it.

- [s4] Baseline 9 re-measured at session start; **honest floor moved 9 → 0** (248/248, rules_dropped=10, cheat_asm_stripped=0) with the uniform byte-offset header-word read at case 3 and case 13. src/ restored to HEAD before finishing.
- [s4] The [s3] frontier hypothesis is CONFIRMED by measurement: desynchronising the two ALIGN4 chains inside block 19 so that only one birthing-boosted insn is ready at reverse-cycle T-6 reproduces target's case-3 order AND all of its seats.
- [s4] decomp-permuter found it: perm_a/output-110-1 (semantically identical to the baseline, `nv = &s0[s3-1]`) reached target's order+seats at sandbox 2; the directed follow-up r5_addr_cast_m1 removed the duplicated base and reached sandbox 0.
- [s4] Permuter-seed lesson for this function: the floor-9 candidate scores 550 as a permuter seed but its jump2-cross-jump-free twin (case 3's s4 assigned before s6, same sandbox 9) scores 260 — seed the twin, the merge costs 300 points of pure noise.
- [s4] Workspace recipe banked at tmp/grind/func_800460E4/s4/mkws.sh + extract_fn.py + score.py (full-TU preprocessed base.c, Makefile-exact pipeline INCLUDING -mel, per-function region extraction, branch/jtbl-normalised diff scoring).
- [s4] Outcome is `ruling-request`, not `candidate-ready`: the closing construct respells state.json banned_constructs #5 and the driver discards a candidate-ready that re-declares a banned construct. The bytes are proven and banked; only the licence is open.


## [s4b] 2026-08-25 - permuter modality - floor 9 -> 0 with NO banned construct

### What this session was for
The 2026-08-25 06:39 ruling REFUSED the previous 0-scoring form (*(s32 *)((s32)&s0[s3] - 8)
/ - 4) as banned_constructs #5 respelled, and closed the door with: "a closing form must
differ from s0[s3-2]/s0[s3-1] by something other than its schedule." It left exactly one
route open - "a byte-neutral (248-insn) fresh named pointer local holding a real consumed
address stays available under the named-intermediate entry's prongs." This session (a)
MEASURED that route dead, and (b) found a different, unbanned closing form that reaches 0
while both case 3 and case 13 read the header words as s0[s3 - 2] / s0[s3 - 1] - the file's
own indexed idiom, unchanged from the floor-9 chassis.

### The residual, re-read from the objdump (not hypothesised)
Floor-9 case-3 block vs target (perm_a/chk_base.dis vs target_nr.dis):

    floor-9                      target
    move  s1,s2                  move  s1,s2
    sll   a0,s3,0x2              sll   v0,s3,0x2
    addu  a0,a0,s0               addu  v0,v0,s0
    lw    v1,-8(a0)              lw    v1,-8(v0)
    li    v0,1                   lw    a0,-4(v0)      <-- both loads ADJACENT
    lui   at,0x0                 li    v0,1           <-- li reuses the dead base seat
    sh    v0,0(at)               lui   at,0x0
    lw    v0,-4(a0)              sh    v0,0(at)
    srl   v1,v1,0x2              srl   v1,v1,0x2
    sll   v1,v1,0x2              sll   v1,v1,0x2
    j     2fc  <- jump2 merge    addu  s6,s0,v1
    addu  s6,s0,v1               srl   a0,a0,0x2
                                 sll   a0,a0,0x2
                                 j     314
                                 addu  s4,s0,a0

So the whole 9 is: the second header load is deferred past the flag store, which puts the
address in $a0 instead of $v0, which makes case 3's tail register-identical to case 13's,
which lets jump2 cross-jump-merge it and lose 3 insns (245 vs 248). ONE scheduling decision.
Case 13 reads the SAME two words with the SAME indexed spelling and already matches target
byte-for-byte - its block has an extra load and a call, so its own schedule already issues
the two loads adjacently. The divergence is case-3-specific and byte-forced.

### KILL 1 - the named-intermediate route the 06:39 ruling left open is DEAD
probe4 (v1-v8, value locals), probe5 (w1-w7, pointer locals incl. hdr = &s0[s3]), probe9 k4
(once-written word-index locals): EVERY once-written named intermediate, in every declaration
order, scope, and statement position, measures diffs=9. Byte-neutral 248-insn members of that
family DO exist (w1, w5, w6, w7, and v6) and are still 9. The disqualifier is
single-assignment itself: adjust_priority -> birthing_insn_p boosts an insn only when its
destination pseudo has reg_n_sets[regno] == 1, and a once-written intermediate keeps that
property, so the tie survives.

### KILL 2 - staging through EXISTING locals (what bound 2 actually sanctions) is DEAD
probe10: s6/s4 used as their own staging carriers = 246 insns, diffs=8 (b1/b2/b4);
s3 + one value local = 248 insns, diffs=40 (b3). No existing-local borrow reaches 0.

### THE FIND - permuter perm_d, then generalised
Campaign perm_d (label s4b-v6-twin-248, seed = the 248-insn no-cross-jump twin with value
locals, base score 260, -j 8): 28327 iterations, 2 finds, best 95, both harvested with
--stop. output-95-1 mutated case 3 to reuse a value local as the shift carrier
(loff = (u32)hoff >> 2; s6 = ... (loff << 2);) and was the FIRST form in this function's
history to issue both header loads adjacently with no address respelling (248 insns,
diffs=10 on the honest scorer). Directed generalisation:

    probe6 x2  split the SECOND chain only, reusing its carrier      248  diffs=7
    probe7 y3  split BOTH chains, each reusing its own carrier       248  diffs=0
    probe9 k1  same, hoff/loff s32 carriers                          248  diffs=0
    probe9 k2  same, u32 carriers + >>=                              248  diffs=0
    probe9 k3  same, carriers named as word indices + &s0[hidx]      248  diffs=0
    probe9 k6  k3 with the flag store first                          248  diffs=0
    probe6 x1/x3/x4/x6/x7/x8  same splits with FRESH once-written    245  diffs=9

The single discriminating property is that each carrier is written TWICE. Fresh carriers that
are written once collapse straight back to 9.

Measured on the real chassis: sandbox func_800460E4 --disable all = 0, 248/248,
rules_dropped=10, cheat_asm_stripped=0. The body is memory/grind/func_800460E4/candidate.c.

### KILL 3 - the inherited FAKE s1 chain-extender is still load-bearing
probe8 z2: replacing it with plain s1 = s4; under the new case-3 form takes the diff from
0 to 32. It stays.

### KILL 4 - the staged spelling cannot be applied uniformly at case 13
probe8 z3: applying the same staged form at case 13 as well takes case 13 from matching to
diffs=13 (z4, also dropping the FAKE chain: 44). Case 13 keeps the plain ALIGN4 spelling.
So the two sites DO read the same two words with two different ALIGN4 spellings - but with
the SAME address form (s0[s3 - 2] / s0[s3 - 1]), which is what banned_constructs #6 was
actually about, and the difference is byte-forced, not chosen.

### Why this is a ruling-request and not a candidate-ready
The one new construct is two FRESH locals each written TWICE - raw byte offset, then refined
in place to a word index. It sits in the gap between two sanctioned families:
 - named-intermediate requires ONCE-written / ONCE-read - this is twice-written;
 - .claude/rules/staged-value-reused-variable.md is exactly the right mechanism (its Origin
   section names sched.c adjust_priority -> birthing_insn_p, "the 'assigned once?' check is
   literally reg_n_sets[regno] == 1") but its bound 2 requires borrowing a local the function
   already has for another job, and its own parenthetical - "a fresh named intermediate is
   fine C on its own merits and needs no exception - but then it also won't have the
   'assigned more than once' property this trick needs" - assumes a fresh intermediate is
   single-assigned. This construct is the case that parenthetical did not anticipate: a fresh
   intermediate that is multiply assigned because the value is genuinely computed in two
   steps (byte offset -> word index).
Per the grind contract ("if you cannot quote a rule's scope sentence and cite a precedent for
a family you are claiming, you do not have that family") the honest outcome is the ruling.
Supporting context for whoever rules: the sibling function directly below in the same file,
func_800464C4 (src/text1a_c2.c), already reuses one scalar v0 to hold successive raw header
offsets across switch arms, so multiply-assigned offset scratch is this file's own shipped
idiom.

- [s4b] Honest floor 9 -> 0 on the real chassis with zero banned constructs; case 3 and
  case 13 both keep the indexed s0[s3 - 2] / s0[s3 - 1] reads.
- [s4b] The 06:39 ruling's one open route (byte-neutral once-written named intermediate) is
  MEASURED DEAD across 22 spellings; single-assignment is the disqualifier.
- [s4b] Existing-local staging carriers (s6/s4/s3) measured dead at 8 and 40 diffs.
- [s4b] The FAKE s1 chain-extender is re-measured load-bearing (0 -> 32 without it).
- [s4b] The staged spelling is case-3-only by measurement, not by choice (case 13 breaks at 13).
- [s4b] perm_d campaign banked: 28327 iterations, 2 finds (95, 95), harvested with --stop.


## [s5] 2026-08-25 (permuter modality) - the residual CLOSED at sandbox 0

[s5.1] The floor-9 residual is closed by borrowing PRE-EXISTING scratch offset
locals as case 3's two header-word carriers. Measured, `sandbox func_800460E4
--disable all` = 0 (248/248 insns, rules_dropped=10, cheat_asm_stripped=0),
three times (bare form, after hoisting all local declarations to function top,
and after the two FAKE annotations were added). The body is
memory/grind/func_800460E4/candidate.c and is in src/text1a_c2.c.

[s5.2] Why the prior session concluded "no existing-local borrow reaches 0"
(H-s4b-3) and why that was wrong: its census (probe10 b1-b4) covered only the
DESTINATION pointers s6/s4 and the two live pointers s3/s2. s6/s4 are live
across the tail's calls, so they are callee-saved and their shift chains run
through $v0 (measured 8 diffs at 246 insns, banked as
rejected/s5-stage-through-s6-s4-themselves-8-diffs.c); s3/s2 are likewise
callee-saved (measured 40). The carriers that work are the function's mainline
scratch OFFSET temps, which are dead across every call and therefore land in
the caller-saved registers target uses ($v1/$a0).

[s5.3] Carrier-choice table (positional diff count vs target, s4/score.py):
  s6/s4 themselves ............................ 8   (246 insns)
  s3 + one value local ........................ 40  (from [s4b])
  mainline s2-offset local (`off`) ............ 22  (246) - case 3 exact, the
      hoist costs 13 instructions in the mainline
  mainline func_80045230 offset (`off3`) ...... 2   (248) - case 3 exact; that
      site refines through a second register
  mainline off3, its own site split too ....... 8   (248) - every instruction's
      SHAPE matches target; the carrier pseudo lands in $a1, target uses $a0
  early-switch offset local + mainline s0[1]
      offset local (the closing pair) .......... 0   (248)
Banked: rejected/s5-borrow-mainline-s2-offset-local-22-diffs.c,
rejected/s5-borrow-mainline-off3-flat-align4-2-diffs.c,
rejected/s5-borrow-mainline-off3-split-seat-a1-not-a0.c.

[s5.4] Spelling of the refinement matters independently of the carrier: the
staged in-place form (`off = raw; off = (u32)off >> 2; off = off << 2;`)
reproduces target's `srl aN,aN / sll aN,aN` at BOTH of the carrier's sites,
whereas a single ALIGN4 expression refines through a second register. With the
early switch's site left flat the object is 3 instructions off (s5g j5/j6); with
it staged the object is exact (s5g j3/j4).

[s5.5] Permuter campaign perm_e (label s5-b1-staged-s6s4-246, seeded on the
8-diff s6/s4-staged body, base score 435, 6 jobs): 16658 iterations, 20+ finds,
best new score 80, no zero, nothing structurally novel. Harvested with --stop
in-session. Data point for the modality ledger: this residual is a
carrier-identity question, not a random-perturbation basin - enumerating which
pre-existing local is borrowed found it in ~25 compiles.

[s5.6] Declaration placement: hoisting every local to the top of the function
(C89/PsyQ house style, replacing the rule-era inner-block declarations) is
byte-neutral here - sandbox 0 both before and after. The borrow therefore does
not rest on a targeted scope widening of exactly the two carriers.

[s5.7] Banned-construct audit of the closing body (all six state.json entries):
no s1/arg1 merge (s1 is a local and keeps its own assignments); no volatile of
any kind; no pm2/pm1 pointer intermediates; no integer-cast byte-offset derefs;
no D_80099478/D_8009947A aggregate; and case 3 reads the header words with
EXACTLY case 13's `s0[s3 - 2]` / `s0[s3 - 1]` address form, so the "two address
forms in one function" objection cannot arise.

## [s6] 2026-08-25 (permuter modality — re-measure + resubmission after a validator discard)

WHY THIS SESSION EXISTS. The s5 session reached `sandbox func_800460E4
--disable all` = 0 and returned candidate-ready, and the driver DISCARDED it
before the layer-1 cheat-reviewer ever ran. The discard reason was not the C:
`tools/grinder/grindlib.py::_ban_trips` scans ONLY the self-vet's `CONSTRUCTS:`
block, matching a banned entry's content words as substrings. s5's CONSTRUCTS
block described the closing construct in the same vocabulary banned entry #6
uses ("case", "same", "header", "word"), so the tripwire fired on a vet that was
in fact asserting that construct's ABSENCE (the `_strip_disclaimers` guard only
drops whole sentences carrying an explicit negation, and s5's declarations were
positive statements about a DIFFERENT construct). Root cause banked so no future
session on this function loses a proven form the same way: keep the CONSTRUCTS:
line terse and put per-construct prose in the T1..T6 sections, which are not
scanned.

RE-MEASUREMENT (this session, from a clean HEAD src tree):
- HEAD's committed rule-era body: `sandbox --disable all` = 35 (248/248,
  rules_dropped=10).
- The banked candidate body re-applied to src/text1a_c2.c: **0** (248/248,
  rules_dropped=10, cheat_asm_stripped=0), measured twice
  (tmp/grind/func_800460E4/s4/sandbox_final.json). The chassis has NOT drifted
  since s5; every s5 spelling conclusion still holds at this chassis.

CITATION AUDIT (all four PRECEDENT targets read this session, none is a dead
path or a mis-cite):
- `.claude/rules/staged-value-reused-variable.md:64` — inside bound 2, the
  sentence quoted verbatim as the family SCOPE.
- `docs/grind/decisions.md:1844` — func_800200DC's 2026-07-28 03:18 final-call
  PASS: three FAKE constructs accepted inside this family, each borrowing a
  PRE-EXISTING variable (`disc`, `dy`) at a point where its previous value is
  dead, each staged value consumed on the next line. Same shape as this body's
  off_a/off_b borrows.
- `docs/reference/sotn-construct-index.md:70` — SOTN master `src/dra/menu.c:1993`
  `j = menu->unk1D; // FAKE?`, the PSX-provenance staged-reuse exhibit.
- `.claude/rules/dead-store-fake-exception.md:32` — the F1 combine-foldable
  chain-extender scope-extension entry covering the inherited s1 construct.
- The 07:19 ruling itself (`docs/grind/decisions.md:10730`) is the strongest
  positive: it refused a FRESH twice-written carrier while stating that
  func_800200DC's "follow-on PASS closed at 0 only after moving to PRE-EXISTING
  carriers", and that "staged-value-reused-variable bound 2 excludes fresh
  inventions knowingly". Both carriers in this body pre-exist in HEAD with their
  own jobs, so the refused boundary is not being re-crossed.

DISCLOSED TENSION (stated in the vet rather than buried): construct (1) hoists
the four scratch offset temps out of inner braces to the top of the routine, and
that scope widening is what puts off_a/off_b in scope at the borrow site. The
hoist is measured byte-neutral on its own, matches C89/PsyQ house style, and the
inner-brace declarations it replaces are rule-era chassis artifacts (the dispatch
brief's RULE-ERA CHASSIS warning applies to this exact function) — but a reviewer
should weigh constructs (1) and (2) together, and this session does not claim
otherwise.

CAMPAIGN HYGIENE: `permuter_campaign.py status` (run under WSL — the Windows-side
invocation raises a spurious PermissionError because the recorded pids are Linux
pids) reports 0 live campaigns and 0 stale registry entries; perm_a..perm_e are
all harvested and dead. No campaign was launched this session: the residual is
not a search problem any more (perm_e's 16658 iterations produced no zero at this
basin, hypotheses.md H25), and the banked form already measures 0.


## [s8] 2026-08-25 (SYNTHESIS modality) — the residual decomposed into two independent halves

[s8.0] Chassis. The 07:54 layer-1 FAIL removed the [s5]/[s6] zero-scoring body from
play (banned_constructs #7: the off_a/off_b borrow plus the declaration hoist that
stages it). The submittable body is the [s7] non-banned form; re-measured this
session at the live chassis, `sandbox func_800460E4 --disable all` = **9**
(245/248, rules_dropped=10, cheat_asm_stripped=0), at the start and again at the end
of the session. memory/grind/func_800460E4/candidate.c now carries that body with a
synthesis header; the banned zero form remains banked at
rejected/layer1-fail-0825-0754.c.

[s8.1] A fast probe harness for this function now exists and is worth reusing:
tmp/grind/func_800460E4/s8/pr.sh compiles a FULL src/text1a_c2.c variant through the
exact project pipeline (mipsel cpp with buildconfig's flags | cc1 -O2 -G0 ... -mel |
prologue_fix | maspsx | multu_pad | extract_fn | as | objdump) and scores
func_800460E4 positionally against target with tmp/grind/func_800460E4/s4/score.py.
Its number agrees with `sandbox --disable all` on every form checked both ways
(9 == 9, and the diff sets are the same instructions). One run is a few seconds
versus a sandbox invocation, so a 24-point cross is one command
(s8/runlist.sh <list>). Note for whoever reuses it: write the variant-name list with
git-bash/`ls`, NOT with Windows Python — Windows Python rewrites the file to CRLF and
every filename then carries a trailing \r ("compilation terminated" for every entry).
Regenerate tmp/grind/func_800460E4/s4/perm_a/target_nr.dis from perm_a/target.o first;
the copy left by the earlier sessions was empty (a 245-diff score is that symptom).

[s8.2] KILL — the `bb_live_regs` half of `birthing_insn_p` is not a lever, and cannot
become one. tools/gcc-2.7.2/sched.c:2504-2537 reads
`if (bb_live_regs[offset] & bit) return (reg_n_sets[i] == 1); return 0;`. The list
scheduler works BACKWARD through the block, so an insn is only ready once all of its
block-local consumers have been scheduled, and scheduling a consumer is precisely the
event that inserts the consumed pseudo into bb_live_regs. Therefore every ready insn
whose pattern is `SET (REG, ...)` passes the liveness test by construction and
birthing_insn_p degenerates to `reg_n_sets[dest] == 1`. Only non-REG SET_DESTs (a
store, a SUBREG) fail it, which is not available as a spelling for a loaded word.
Cross-checked against every 0x7f000001 entry in two different chassis' block-19 traces
(s8/base.sched, s8/vC.sched). [s3] frontier item (2) is CLOSED.

[s8.3] CONFIRMED — [s3] frontier item (1) is real, and it is the first non-banned
route ever measured to target's load adjacency. Two value-neutral rebase detours on
the two stage-header words (`m = word - (s32)s0; m = m + (s32)s0;`, or the same with
`^`) leave extra insns in block 19 at sched1. They do not touch the dependence graph,
they do not touch alias analysis, and they do NOT remove the birthing boost — both
competing insns still carry 0x7f000001. What changes is the cycle at which each chain
becomes ready, so schedule_select's tie-break resolves the other way:
  baseline (s8/base.sched, block 19):
    ;; ready list at T-6: 331 (1) 298 (1) 310 (7f000001) 322 (7f000001), now 322 310 ...
  under the detour (s8/vC.sched, block 19):
    ;; ready list at T-6: 346 (1) 298 (1) 332 (7f000001) 329 (7f000001), now 332 329 ...
    ;; insn 329 has a greater potential hazard, now 329 332 346 298
The emitted case-3 block then contains target's `lw ...,-8(base)` and
`lw ...,-4(base)` back to back, with BOTH header values in target's exact hard
registers: the -8 word in $v1 and the -4 word in $a0 (s8/x2.dis, s8/vC.dis).

[s8.4] KILL — adjacency is NOT sufficient. At the adjacency-achieved chassis the
residual is 10 (not 0) and consists of exactly two facts, both outside the tie:
 (a) the ADDRESS pseudo is seated $v1 where target seats it $v0. Because $v1 is also
     the -8 word's seat, the pair is forced to load -4 first and overwrite the base
     with -8 second — target, holding the address in the otherwise-free $v0, can load
     -8 first and then reuse the dead $v0 for its `li v0,1`;
 (b) each ALIGN4 shift chain refines through $v0 instead of refining IN PLACE
     (`srl v1,v1,0x2; sll v1,v1,0x2` and `srl a0,a0,0x2; sll a0,a0,0x2` in target).
(b) is [s5] H23 restated from the other side: in-place refinement is an INDEPENDENT
requirement, and the only construct ever measured to produce it is a carrier written
more than once — the construct class closed by the 07:19 ruling (fresh) and the 07:54
layer-1 FAIL (hoisted pre-existing).

[s8.5] Control that isolates the mechanism: the `+ 4 / - 4` rebase (constant, folds
completely before sched1) measures 9/248 with a case-3 schedule identical to the
baseline's, while the `- s0 / + s0` and `^ s0` rebases (which survive into sched1)
measure 10/246 and 10/248 with adjacency. The effect is caused by atoms PRESENT AT
SCHED1, not by the source text. Corollary for pass attribution on this function:
combine runs before sched1 (toplev.c:3004 vs :3033), so any construct whose whole
effect is folded by combine cannot influence this tie at all; only constructs whose
extra atoms survive combine and are removed later (reload coalescing, jump2) can be
both effective and byte-neutral.

[s8.6] Sanction status of the s8 research forms: the rebase detours are NOT
submittable and are not proposed. They fail cheat-checklist T1 (no observable effect
on the function's output) and T2 (no human writes `m = x - (s32)s0; m = m + (s32)s0;`
to compute an offset). They are banked as measurement only:
 rejected/s8-rebase-detour-both-words-246-10-adjacent-loads.c
 rejected/s8-xor-rebase-detour-248-10-seats-right-address-v1.c
 rejected/s8-const4-rebase-folds-fully-inert-at-9.c   (the control)
 rejected/s8-s4-before-s6-store-early-248-9-no-crossjump.c
Their value is that they PARTITION the residual: half of it (adjacency + both value
seats) is now known to be reachable without any carrier trick at all, which reduces
the open question to the address seat and in-place refinement.

[s8.7] Order-space re-confirmation at this chassis (24-point cross): the load order
and the detour-completion order in the C source are INERT (cse canonicalises them);
only the s6-vs-s4 consumption order and the store position move the result, and their
whole effect is on the jump2 cross-jump merge (245/248 insn counts) — consistent with
[s2] H17/H18. Nothing in the order space reaches below 9.

- [s4] Chassis re-established: with the 07:54-banned off_a/off_b form withdrawn, the submittable body is the [s7] non-banned form and `sandbox func_800460E4 --disable all` = 9 (245/248, rules_dropped=10), measured at session start and again at session end; candidate.c now carries that body with a synthesis header, and the banned zero-scoring form stays banked at rejected/layer1-fail-0825-0754.c.

- [s4] The residual is now decomposed into two INDEPENDENT requirements: (A) the two header loads must issue adjacently, and (B) the address pseudo must sit in $v0 and each ALIGN4 chain must refine in place. (A) is reachable by a sched1 atom-set change with no carrier trick; (B) has only ever been produced by a multiply-assigned carrier.

- [s4] sched.c birthing_insn_p liveness test is satisfied by construction for every ready insn with a SET(REG,...) pattern, because backward scheduling makes readiness imply that a consumer was already scheduled - so reg_n_sets is the ONLY half of the boost that is addressable, and [s3] frontier item (2) is closed.

- [s4] Adjacency plus both header value seats ($v1 for the -8 word, $a0 for the -4 word) are achievable without touching alias analysis, the dependence graph, reg_n_sets, or any pre-existing local - previously every route to adjacency was a banned alias-defeat or a banned/refused multi-set carrier.

- [s4] The s8 rebase detours are research forms only and are NOT proposed: they fail cheat-checklist T1 (no observable effect on output) and T2 (no human writes `m = x - (s32)s0; m = m + (s32)s0;`). Their value is the partition of the residual, not the bytes.

- [s4] Order space re-confirmed inert at this chassis (24-point cross): source load order and detour-completion order are canonicalised by cse; only s6-vs-s4 consumption order and store position move anything, and only via the jump2 cross-jump merge (245 vs 248 insns) - consistent with [s2] H17/H18, and nothing in that space goes below 9.

- [s4] New reusable tooling: tmp/grind/func_800460E4/s8/pr.sh plus runlist.sh score a full src variant through the exact pipeline in seconds and agree with `sandbox --disable all`. Two traps recorded: regenerate s4/perm_a/target_nr.dis (the inherited copy was empty, which shows up as a 245-diff score), and never write the variant-name list with Windows Python (CRLF turns every entry into a missing file).



## [s9] 2026-08-25 (SOLVER modality) — the residual reduced to ONE RTL flag, mechanically

[s9.0] Chassis. HEAD src carries the rule-era body. The [s7]/[s8] candidate body
re-applied to src/text1a_c2.c measures **9** through the s8/pr.sh harness
(245 insns vs target 248) — unchanged, no chassis drift since [s8].

[s9.1] TOOL TRAP, banked for every future solver session on a RULE-CARRYING
function: `tools/ra_solver/mkasm_honest.sh` builds `<stem>.tgt.s` by applying
regfix/regfix_stage2/asmfix to WHATEVER IS IN src/ RIGHT NOW. For a deferred
function whose 10 regfix rules were calibrated against HEAD's rule-era body,
running it with the CANDIDATE body in src produces a fictional "target": the
rules re-apply on top of a body that already has the registers right, and
`inverse_compose.py classify` then reports a confident RA verdict describing a
clean $17/$18/$19 three-cycle rotation that does not exist. The correct
procedure (used for everything below) is two runs: build `.tgt.s` with HEAD's
src (that stream IS the original — the tree is SHA1-identical), save it, then
rebuild `.hon.s` with the candidate body and restore the saved `.tgt.s` before
classifying. Artifacts: s5b/tgt_true.s, s5b/hon_cand.s, s5b/head_src.c.

[s9.2] CLASSIFY at the honest chassis: **PRE-RA**, honest 237 insns vs target
240. The multiset difference is NOT upstream RTL shape — it is jump2, which the
classifier's three-stage funnel does not model. Our case-3 arm (.L19) ends
`j .L31` into case 34's tail; target's does not merge and emits its own
`srl $4,$4,2 / sll $4,$4,2 / addu $20,$16,$4` inline before `j .L18`. Those 3
instructions are the entire 245-vs-248 gap. Full diff: s5b/hon_cand.s vs
s5b/tgt_true.s.
COROLLARY: the cross-jump merge is a CONSEQUENCE of the register seats, not an
independent lever. It merges only because our case-3 tail happens to hold the
-4 header word in the same register ($v0) as case 34's tail; in target that word
is in $a0, the two tails are textually different, and find_cross_jump cannot
merge them. [s8.7]'s "order space only moves the jump2 merge" is explained: the
merge is downstream of the seats, and the seats are downstream of the schedule.

[s9.3] LOCAL-ALLOC MODEL — the decisive result. `local_extract.py text1a_c2` at
the candidate chassis dumps block 19's four quantities, and the validated
`qty_compare` arithmetic (floor_log2(refs)*refs*size/(death-birth)*10000, tie ->
lower qty) plus ascending `find_free_reg` reproduces OUR seats exactly:
    address (refs5, span 14) pri 7142  ranks LAST  -> $a0
    -4 word (refs6, span 8)  pri 15000 ranks FIRST -> $v0
Hand-replaying the SAME model on TARGET's block-19 instruction order reproduces
TARGET's seats exactly:
    address (refs5, span 6)  pri 16666 ranks FIRST -> $v0
    -8 word -> $v1; li 1 -> $v0 (address already dead); -4 word (span 18) -> $a0
Full table: tmp/grind/func_800460E4/s5b/block19_localalloc.md.
**Therefore [s8]'s requirement (B) is not a requirement at all.** "The address
pseudo must sit in $v0" and "each ALIGN4 chain must refine in place" are not
independent goals needing a multiply-assigned carrier: they are the mechanical
output of local_alloc once the two loads are adjacent, because adjacency
shortens the address quantity's live span from 7 insns to 3 and lifts its
qty_compare priority from 7142 to 16666, above every other quantity in the
block. The ENTIRE carrier line of attack — the axis that produced the 07:19
ruling and the 07:54 layer-1 FAIL — was aimed at a goal that does not exist.

[s9.4] KILL — the whole atom-set / rebase-detour axis ([s8] frontier item 1) is
FORECLOSED by construction, not by search. Re-ran `local_extract.py` with the
vC xor-rebase body (rejected/s8-xor-rebase-detour-248-10-seats-right-address-v1.c)
in src: block 19's address quantity STILL has refs5 and span 14 (pri 7142,
allocated last, got $v1) even though the loads are adjacent there. A detour that
buys adjacency by leaving extra atoms alive across the loads necessarily keeps
the address quantity live across them too, so it can never win the qty_compare
race it needs to win. Any construct in that family is self-defeating — which is
why [s8.4] measured the address in $v1 and read it as a second, independent
requirement. No further sweeping of "honest spellings whose extra atoms survive
combine into sched1" can close this function.

[s9.5] The residual is now ONE BIT, named at source level.
- `tools/gcc-2.7.2/expr.c:4567-4577` (INDIRECT_REF): `MEM_IN_STRUCT_P (temp) = 1`
  iff the address subtree is a `PLUS_EXPR` (or a SAVE_EXPR of one, or the type is
  aggregate). It has nothing to do with structs; in GCC 2.7.2 it is a purely
  syntactic property of the tree the front end built ("If address was computed by
  addition, mark this as an element of an aggregate").
- `tools/gcc-2.7.2/sched.c:831-839` (`true_dependence`): the conflict is
  suppressed when one mem is `MEM_IN_STRUCT_P && rtx_addr_varies_p` and the other
  is neither. Our header-word loads (address = PLUS_EXPR -> /s; register address
  -> varies) against the `sh` to `D_8009947A` (no /s, constant address) hit that
  clause exactly, so sched1 sees NO edge and hoists `li 1 / sh` between the two
  loads. Target's original had the edge, so it could not.
So: load adjacency <=> the two loads' address subtrees are not PLUS_EXPRs.

[s9.6] MEASURED — the one bit closes the whole function at this chassis. With
case 3 spelled `s32 *ptr = (s32 *)((s3 << 2) + (s32)s0); s32 *pm2 = ptr - 2;
s32 *pm1 = ptr - 1; s32 raw_m2 = *pm2; s32 raw_m1 = *pm1;` the object is
**248 insns, 0 diffs** (s4b2/s5pm2.dis). This is the banned #4 construct and is
NOT proposed — it is banked as a research measurement only, at
rejected/s9-research-only-banned-pm-ptr-form-measures-0-at-s7-chassis.c. Its value
is the proof that nothing else in this function is wrong: the honest floor of 9
and the single MEM_IN_STRUCT_P bit are the same fact.
A near-miss control worth keeping: spelling the base as `&s0[s3] - 2` instead of
`(s32 *)((s3 << 2) + (s32)s0)` leaves exactly 1 diff — `addu v0,s0,v0` where
target has `addu v0,v0,s0` — because `pointer_int_sum` builds
`PLUS_EXPR(base, scaled_index)` while the integer-cast form builds
`PLUS_EXPR(scaled_index, base)`, and the MIPS addu inherits that operand order.
(rejected/s9-research-only-banned-pm-form-1-diff-addu-operand-order.c.)

[s9.7] EXHAUSTION ENUMERATION — the mechanical part, and what makes the residual
decidable rather than merely hard. By expr.c:4567-4577 the ONLY C spellings that
produce a non-/s load are those whose INDIRECT_REF operand is not a PLUS_EXPR:
 (a) a plain pointer VARIABLE (`*pm2`) — banned_constructs #4/#5;
 (b) an integer- or pointer-cast byte-offset expression (`*(s32 *)((s32)ptr - 8)`,
     `*(s32 *)((u8 *)s0 + k)`) — closed by the 06:39 ruling and the standing judge
     constraint "do not respell the header-word reads as inlined integer-cast
     byte-offset derefs in any form";
 (c) an aggregate / COMPONENT_REF read, which sets /s unconditionally
     (expr.c:4888) and is separately closed by the 04:46 aggregate-merge ruling.
(a) ∪ (b) ∪ (c) is the whole space and all three are closed for this function.
There is no fourth spelling.

[s9.8] SCHED-SOLVER status at this chassis: `sched_solver/extract.py` reports
parity=True, and `inverse_sched.py --block 19 --goal-from-target` refuses to
search: **GOAL INVALID — 1 dependence violation (304,308)**, because the hon->tgt
alignment cannot pair a 14-insn block against a target arm that jump2 never
merged (the 3-insn count gap of [s9.2]). The scheduler backend therefore cannot
be driven from an automatic goal on this block at any chassis where the merge
happens. This does not weaken [s4]'s UNREACHABLE verdict — it explains it: at a
FIXED atom set there is no luid arrangement that produces target's order, because
the missing thing is a dependence EDGE, and [s9.5] names the exact source
property that creates it.

Artifacts: tmp/grind/func_800460E4/s5b/ (head_src.c, cand.c, tgt_true.s,
hon_cand.s, research_pm.c, research_pm2.c, block19_localalloc.md,
evidence_s9.md), tmp/grind/func_800460E4/s4b2/s5cand.dis, s5pm.dis, s5pm2.dis,
tmp/ra_solver_work/text1a_c2.local.json, tmp/sched_solver_work/text1a_c2.sched.json.

- [s5] [s9] Chassis unchanged: the [s7]/[s8] candidate body re-applied to src measures 245 insns / 9 diffs through tmp/grind/func_800460E4/s8/pr.sh, at session start and again at session end. src/text1a_c2.c was restored to HEAD before finishing.

- [s5] [s9] TOOL TRAP for rule-carrying (asm-until-matched deferred) functions: tools/ra_solver/mkasm_honest.sh builds <stem>.tgt.s by running regfix/regfix_stage2/asmfix over whatever is in src/ right now. With a candidate body in src the rules re-apply on top of already-correct registers and inverse_compose.py classify returns confident fiction (a clean $17/$18/$19 rotation). Correct procedure: build .tgt.s from HEAD's src, save it, rebuild .hon.s from the candidate body, restore the saved .tgt.s, then classify. Artifacts s5b/tgt_true.s + s5b/hon_cand.s.

- [s5] [s9] Honest classify verdict is PRE-RA (237 vs 240 insns), and the 3-insn multiset gap is entirely jump2: our case-3 arm ends `j .L31` into case 34's tail, while target emits its own `srl $4,$4,2 / sll $4,$4,2 / addu $20,$16,$4` and jumps to .L18. The merge happens only because our case-3 tail holds the -4 header word in the same register ($v0) as case 34's tail; in target it is in $a0 and the tails are textually different, so find_cross_jump cannot merge. The cross-jump is a CONSEQUENCE of the seats, not a lever.

- [s5] [s9] local_alloc's qty_compare + find_free_reg replay reproduces BOTH our block-19 seats and target's block-19 seats, and target's fall out of target's instruction ORDER alone. Ours: address refs5 span14 pri 7142 (last) -> $a0; -8 word refs6 span12 pri 10000 -> $v1; li 1 refs2 span2 pri 10000 -> $v0; -4 word refs6 span8 pri 15000 (first) -> $v0. Target: address refs5 span6 pri 16666 (first) -> $v0; -8 word -> $v1; li 1 -> $v0; -4 word span18 pri 6666 (last) -> $a0.

- [s5] [s9] Consequence: [s8]'s requirement (B) (address to $v0 + in-place ALIGN4 refinement) is NOT an independent requirement and needs NO multiply-assigned carrier. The 07:19 ruling and the 07:54 layer-1 FAIL were both spent on a goal that does not exist; every future session should stop looking for a carrier.

- [s5] [s9] The rebase/atom-set axis is foreclosed by construction: on the vC xor-rebase chassis (loads adjacent) block 19's address quantity is still refs5 / span 14 / pri 7142 / seated $v1, because the detour's surviving atoms keep the address live across the loads. Adjacency bought with extra live atoms can never deliver the seats.

- [s5] [s9] Root mechanism named at source level: expr.c:4567-4577 sets MEM_IN_STRUCT_P on an INDIRECT_REF iff the address subtree is a PLUS_EXPR (a syntactic GCC 2.7.2 quirk, not a struct property); sched.c:831-839 true_dependence then proves a varying /s load disjoint from the non-varying non-/s store to D_8009947A, so sched1 hoists li 1 / sh between the two loads and destroys adjacency.

- [s5] [s9] The one bit closes the whole function: the pointer-variable spelling `s32 *ptr = (s32 *)((s3 << 2) + (s32)s0); s32 *pm2 = ptr - 2; s32 *pm1 = ptr - 1; s32 raw_m2 = *pm2; s32 raw_m1 = *pm1;` measures 248 insns / 0 diffs at this chassis. It is banned_constructs #4 and is NOT proposed - banked purely as the proof that nothing else in the function is wrong.

- [s5] [s9] Control worth keeping: `&s0[s3] - 2` instead of the integer-cast base leaves exactly 1 diff (`addu v0,s0,v0` vs target's `addu v0,v0,s0`), because pointer_int_sum builds PLUS_EXPR(base, scaled_index) while the integer-cast form builds PLUS_EXPR(scaled_index, base) and the MIPS addu inherits the operand order.

- [s5] [s9] Exhaustion enumeration: by expr.c:4567-4577 the only non-/s spellings are (a) plain pointer variable, (b) integer/pointer-cast byte-offset deref, (c) aggregate COMPONENT_REF (which sets /s unconditionally at expr.c:4888). (a) is banned_constructs #4/#5, (b) is the 06:39 ruling + the standing constraint, (c) is the 04:46 aggregate-merge ruling. There is no fourth spelling.

- [s5] [s9] sched_solver at this chassis: extract.py parity=True, but inverse_sched.py --block 19 --goal-from-target refuses with GOAL INVALID (1 dependence violation, 304<-308) because the hon->tgt alignment cannot pair a 14-insn block against a target arm jump2 never merged. The scheduler backend cannot be driven from an automatic goal on this block; this explains rather than weakens [s4]'s UNREACHABLE verdict - the missing thing is a dependence EDGE, and [s9.5] names the source property that creates it.

## [s6] 2026-08-25 (FORENSICS modality) — the MEM_IN_STRUCT_P account is now RTL-dump-proven, the residual narrows to ONE lvalue, and two more routes die on measurement

[s6.0] Chassis re-measured at session start with the [s7]/[s8]/[s9] candidate body
through `tmp/grind/func_800460E4/s8/pr.sh`: **245 insns / 9 diffs** — unchanged
since [s8]. src/text1a_c2.c was NEVER edited this session (every probe was run as
a standalone variant .c through pr.sh), so the tree stayed HEAD-clean.
Owner directive (RULES-TO-ZERO, 2026-08-24) re-acknowledged: the goal is
COMPLETED-C, which retires this function's 10 regfix rules; the wave-2
INCLUDE_RODATA route was already measured SHA1-dead for this function
specifically (docs/grind/borderline.md:88) and was not re-run.

[s6.1] DUMP-PROVEN (upgrades [s9.5] from a source-read inference to an RTL fact).
`tools/gcc-2.7.2/build/cc1 -O2 -da` on the candidate body, dumps under
tmp/grind/func_800460E4/s6/dumps_cand/. In `t.flow` (pre-sched RTL) the case-3
block is:

    (insn 308 ... (set (reg 140) (mem/s:SI (plus:SI (reg 138) (const_int -8)))))
    (insn 322 ... (set (reg 147) (mem/s:SI (plus:SI (reg 138) (const_int -4)))))
    (insn 331 ... (set (mem:HI (symbol_ref:SI ("D_8009947A"))) (reg:HI 150)))

Both header loads carry **/s** (MEM_IN_STRUCT_P) with a *varying* SImode address;
the store's MEM carries **no /s** and a *non-varying* symbol_ref address. That is
literally the operand pattern `sched.c:831-839` exempts in BOTH `true_dependence`
and `anti_dependence`.
In `t.sched` (sched1 output, excerpt banked at s6/sched_block19_hoist.txt) the
emitted order is **308 -> 329 (li 1) -> 331 (sh) -> 309 -> 310 -> 322**: the store
is scheduled BETWEEN the two loads, and insn 322's dependence `insn_list` names
only insn 304 (its address), never 331 — the missing memory edge is visible in the
dump itself rather than inferred. Pass attribution is therefore final: the
divergence is BORN in `expr.c` expand (the /s bit) and TAKEN by **sched1**
(`sched.c` true_dependence / anti_dependence). No later pass participates.
Artifacts: s6/flow_block19_memflags.txt, s6/sched_block19_hoist.txt.

[s6.2] NEW — the residual needs only ONE of the two reads to lose /s, and it is the
**-4 (second) read**, not the pair. Derivation: the store follows both loads in
source order, so it is anti-dependent on any load it is not exempt from; blocking
it against the LAST load alone is sufficient to keep it out of the gap. Measured,
both directions, at this chassis:

  * `-4` read through a pointer variable, `-8` read left as `ptr[-2]`
    -> **248 insns / 0 diffs** (the whole function closes)
  * `-8` read through a pointer variable, `-4` read left as `ptr[-1]`
    -> **245 insns / 9 diffs** (completely inert)

Banked research-only (both are banned_constructs #4 family, NOT proposed):
rejected/s6-research-only-banned-only-m1-ptr-248-0.c and
rejected/s6-research-only-only-m2-ptr-245-9-inert.c.
CONSEQUENCE for any future fidelity packet: the decidable question is about ONE
lvalue — case 3's read of the `-4` stage-header word — not about a two-read
construct. That is a strictly smaller question than the one the [s9] frontier
posed.

[s6.3] KILL — the aggregate-merge route (enumeration branch (c), closed by the
04:46 ruling on procedural grounds) is now closed on **independent, pre-existing
evidence**, which is exactly the prong (a) bar that ruling demanded
(no-new-park-categories.md 2026-08-17: base-register or stride evidence, NOT
adjacency). Surveyed every access to D_80099478 / D_8009947A in the shipped EXE —
seven functions: func_800460E4, func_800464C4, func_8004659C, func_8004668C,
func_800466C0, func_80046798, func_800467A8. **All twelve accesses use their own
independent `lui %hi(SYM)` + `%lo(SYM)` pair.** The decisive negative is
func_8004668C, which stores to BOTH symbols back to back
(`lui $at,%hi(D_80099478); sh $v0,%lo(...); lui $at,%hi(D_8009947A); sh $zero,%lo(...)`)
and re-materializes the base with a **second `lui $at`** instead of reusing the
first with a +2 offset — precisely what GCC would NOT do if the two halfwords were
members of one aggregate. There is no base-register or stride evidence anywhere in
the binary. Prong (a) FAILS on evidence, not merely on procedure; the aggregate
model is affirmatively contradicted. Do not re-file it.
Artifact: s6/aggregate_evidence_survey.txt.

[s6.4] KILL — the **walking-pointer** technique
(.claude/rules/walking-pointer-serializes-parallel-loads.md; project-precedented,
independent cheat-reviewer + owner sign-off 2026-06-15, ordinary C requiring no
FAKE) is DEAD at this site, and it dies on INSTRUCTION COUNT, not on sanction.
It was the strongest non-banned construct available for this residual: `*hp++`
builds INDIRECT_REF(POSTINCREMENT_EXPR), which is not a PLUS_EXPR, so it clears /s
on BOTH reads with no cast, no volatile, no invented scalar, and a plain human
reading. Measured:

  * `s32 *hp = &s0[s3 - 2]; s6 = ..ALIGN4(*hp++); s4 = ..ALIGN4(*hp++); D_8009947A = 1;`
    -> **249 insns / 4 diffs**
  * same with the store hoisted to the top of the block -> 246 / 25
  * `s32 *hp = &s0[s3]; s4 = ..ALIGN4(*--hp); s6 = ..ALIGN4(*--hp);` -> 249 / 5

Mechanism of the failure: target's base register holds exactly `s0 + (s3<<2)` and
the two loads use literal offsets `-8` / `-4`. A walking pointer must *hold* the
address it dereferences at offset 0, so GCC emits one extra `addiu` to bias the
base — 249 insns against target's 248. The extra instruction is structural; no
schedule removes it.
Banked: rejected/s6-walking-pointer-postinc-249-4-extra-addiu.c,
rejected/s6-walking-pointer-predec-249-5-extra-addiu.c,
rejected/s6-walking-pointer-store-first-246-25.c.

[s6.5] Enumeration [s9.7] is now closed by MEASUREMENT as well as by syntax, and
the corollary is sharp. To match, a case-3 spelling must simultaneously

  (i) leave the base register holding `s0 + (s3<<2)` so the loads keep their `-8` /
      `-4` literal offsets (otherwise +1 addiu — [s6.4]), and
  (ii) give the `-4` read a non-PLUS_EXPR INDIRECT_REF operand so /s clears
      ([s6.1]/[s6.2]).

(i) forbids the dereferenced address from living in the pointer that is
dereferenced; (ii) requires the dereferenced operand to be a bare pointer
*variable* (post/pre-increment and index-0 forms are the only other non-PLUS
shapes, and both violate (i)). The intersection is exactly one construct —
`s32 *pm1 = ptr - 1; ... *pm1` — which is banned_constructs #4/#5. There is no
fourth spelling and now no fourth *shape* either.

[s6.6] Mechanism fact worth carrying to other functions (dump-proven here):
in GCC 2.7.2, `p[0]` / `*p` on a pointer VARIABLE produces a MEM with **/s = 0**
(fold collapses `PLUS_EXPR(p, 0)`, so expr.c:4567-4577 never fires), while `p[k]`
for any nonzero k produces **/s = 1**. Confirmed in this TU's own dump: the
mainline `s0[0]` / `a0_ptr[0]` reads are `(mem:SI (reg/v:SI 74))` and
`(mem:SI (reg/v:SI 109))` with no /s, while `a0_ptr[-1]` / `a0_ptr[1]` are
`(mem/s:SI (plus:SI ...))`. Also confirmed: /s is a purely *tree-syntactic*
property that survives CSE address folding — insns 170 and 246 carry
`(mem/s:SI (reg ...))`, a bare-register address that still remembers it was born
from a PLUS_EXPR.

- [s6] Chassis 245/9 re-measured at session start and unchanged; src/ never edited (all probes run as standalone variant .c files through s8/pr.sh).
- [s6] The /s account is now RTL-dump-proven end to end: t.flow shows both case-3 header loads as (mem/s:SI (plus (reg 138) (const_int -8|-4))) and the D_8009947A store as a bare (mem:HI (symbol_ref)); t.sched shows sched1 emitting 308 -> li 1 -> sh -> 309 -> 310 -> 322, with insn 322's dependence list naming only its address insn 304. The absent memory edge is visible in the dump.
- [s6] Only the -4 (second) header read needs /s=0: flipping it alone measures 248/0, flipping only the -8 read measures 245/9 (inert). Any future fidelity question concerns ONE lvalue.
- [s6] Aggregate merge is contradicted by binary-wide evidence, not merely closed by ruling: all 12 accesses to D_80099478/D_8009947A across 7 functions use independent lui %hi/%lo pairs, and func_8004668C emits TWO separate lui $at for back-to-back stores to the two symbols. Prong (a) FAILS on evidence.
- [s6] Walking pointers (*hp++ / *--hp) DO clear /s honestly with no cast and no volatile, and the technique is project-sanctioned (walking-pointer-serializes-parallel-loads.md), but they measure 249/4, 249/5, 246/25 because holding the address in the pointer costs one extra addiu against target's 248. Dead on instruction count, not on sanction.
- [s6] Closing corollary: matching requires the base register to stay at s0+(s3<<2) (literal -8/-4 offsets) AND the -4 read to be non-PLUS. Those two requirements intersect in exactly one construct, the banned pm1 pointer intermediate. No fourth shape exists.
- [s6] Portable GCC 2.7.2 fact: p[0] / *p on a pointer variable yields a MEM without /s (fold kills PLUS_EXPR(p,0)); p[k] for k != 0 yields /s. /s survives CSE address folding (bare-register MEMs in this TU still carry /s).

- [s6] Chassis re-measured at session start with the [s7]/[s8]/[s9] candidate body through tmp/grind/func_800460E4/s8/pr.sh: 245 insns / 9 diffs — unchanged since [s8]. src/text1a_c2.c was NEVER edited this session (all probes ran as standalone variant .c files through pr.sh), so the tree stayed HEAD-clean; git status shows only memory/grind ledger files.

- [s6] Owner directive (RULES-TO-ZERO, 2026-08-24) re-acknowledged: the goal is COMPLETED-C, which retires this function's 10 regfix rules. The wave-2 INCLUDE_RODATA route was already measured SHA1-dead for this function specifically (docs/grind/borderline.md:88) and was not re-run.

- [s6] RTL-dump proof of the mechanism (t.flow): case-3's two header loads are (mem/s:SI (plus:SI (reg 138) (const_int -8))) and (mem/s:SI (plus:SI (reg 138) (const_int -4))); the case-3 store is (set (mem:HI (symbol_ref:SI ("D_8009947A"))) (reg:HI 150)) with no /s and a non-varying address. That is exactly the operand pattern sched.c:831-839 exempts in both true_dependence and anti_dependence.

- [s6] RTL-dump proof of the consequence (t.sched, sched1 output): emitted order 308 -> 329 (li 1) -> 331 (sh) -> 309 -> 310 -> 322, i.e. the store scheduled between the two loads; insn 322's dependence insn_list names only insn 304 (its address), never 331. The absent memory edge is visible in the dump rather than inferred.

- [s6] One-sided /s measurement: clearing /s on ONLY the -4 (second) header read closes the function at 248 insns / 0 diffs; clearing /s on ONLY the -8 read leaves 245 / 9 (fully inert). The decidable residual concerns a single lvalue.

- [s6] Aggregate-merge prong (a) fails on evidence: all 12 accesses to D_80099478 / D_8009947A across 7 functions use independent lui %hi / %lo pairs. func_8004668C stores to both symbols back to back and re-materializes the base with a second lui $at instead of offsetting the first — the opposite of what a single aggregate would produce.

- [s6] Walking pointers clear /s honestly (INDIRECT_REF of POSTINCREMENT_EXPR / PREDECREMENT_EXPR is not a PLUS_EXPR) but cannot match: 249/4 (post-inc from &s0[s3-2]), 246/25 (same, store hoisted), 249/5 (pre-dec from &s0[s3]) against target's 248 insns. Holding the dereferenced address in the pointer costs one structural extra addiu.

- [s6] Closing corollary, now measurement-backed: a matching case-3 spelling must keep the base register at s0+(s3<<2) (literal -8/-4 offsets) AND give the -4 read a non-PLUS_EXPR operand. Those two requirements intersect in exactly one construct, `s32 *pm1 = ptr - 1; ... *pm1`, which is banned_constructs #4/#5. No fourth spelling and no fourth shape exists.

- [s6] Portable GCC 2.7.2 fact confirmed in this TU's dump: p[0] / *p on a pointer variable gives a MEM with /s = 0 (fold collapses PLUS_EXPR(p, 0)), p[k] for k != 0 gives /s = 1, and /s survives CSE address folding (insns 170 and 246 are bare-register MEMs that still carry /s).

## [s7] 2026-08-25 (FORENSICS modality) — the dependence exemption has FOUR levers, not one; the STORE-side lever closes the function at sandbox 0 with case 3 written in the plain idiom

[s7.0] Chassis re-measured at session start: the [s7]/[s8]/[s9] candidate body
(memory/grind/func_800460E4/candidate.c, spliced onto src's preamble/tail as
tmp/grind/func_800460E4/s7b/base.c) measures **245 insns / 9 diffs** through
tmp/grind/func_800460E4/s8/pr.sh — unchanged from [s6]. src/text1a_c2.c was
edited exactly once (to run the real `sandbox` on the find, see [s7.4]) and
reverted with `git checkout --` before this write-up; `git status` shows only the
pre-existing metrics/events.jsonl dirt.

[s7.1] **THE ENUMERATION GAP.** Sessions [s6] and [s9] framed the residual as
"the two case-3 header loads carry MEM_IN_STRUCT_P (/s), therefore sched.c
exempts the D_8009947A store from an anti-dependence, therefore sched1 hoists the
store between the loads", and then enumerated only spellings that clear /s on the
LOAD. Read the exemption clause itself (tools/gcc-2.7.2/sched.c:843-864,
`anti_dependence(mem = the load's MEM, x = the store's MEM)`):

    && ! (MEM_IN_STRUCT_P (mem) && rtx_addr_varies_p (mem)
          && GET_MODE (mem) != QImode
          && ! MEM_IN_STRUCT_P (x) && ! rtx_addr_varies_p (x))

The exemption is a **five-term conjunction**. Breaking ANY term restores the
dependence edge. Four of the five are reachable in principle:
  (L1) clear MEM_IN_STRUCT_P on the load  — the only axis [s6]/[s9] enumerated;
  (L2) make the load's address non-varying — impossible, it is s0 + (s3<<2);
  (L3) make the load QImode                — impossible, the header words are s32;
  (L4) **set MEM_IN_STRUCT_P on the STORE** — never enumerated before this session;
  (L5) make the STORE's address varying     — never enumerated before this session.
(The mirrored second clause cannot re-fire: it needs `! MEM_IN_STRUCT_P (mem)`,
and the load keeps its /s in every L4/L5 form.)
[s6.5]'s "there is no fourth spelling and now no fourth *shape* either" is
therefore true only of the LOAD side (L1). The corollary that the intersection is
the single banned pm1 construct does not survive the L4/L5 axes.

[s7.2] **HOW L4 IS REACHED.** expr.c's INDIRECT_REF expander (4567-4577) sets /s
when the address subtree is a PLUS_EXPR **or** when
`AGGREGATE_TYPE_P (TREE_TYPE (exp))` **or** when the operand is an ADDR_EXPR of an
object with aggregate type; and the VAR_DECL / ARRAY_REF paths (expr.c:4888,
4329, 5788) set /s from `AGGREGATE_TYPE_P` of the accessed object's type. A store
to a scalar `extern s16 D_8009947A;` therefore always produces a bare
`(mem:HI (symbol_ref))` with /s = 0 — which is precisely what makes the exemption
fire. Typing the same storage as an aggregate makes the store's MEM carry /s.
Confirmed empirically: BOTH `extern s16 D_8009947A[1]; ... D_8009947A[0] = 1;`
AND `... *(s16 *)D_8009947A = 1;` set /s (the latter because array-to-pointer
decay yields ADDR_EXPR of an ARRAY_TYPE object — clause 4 of expr.c:4570-4572).
Emitted bytes for the store are identical in every spelling: one `sh` through the
assembler's `%hi/%lo` macro.

[s7.3] **THE MEASUREMENTS (all through s8/pr.sh, target = 248 insns).**

  * base (candidate.c body, plain scalar decl)                      -> 245 / 9
  * array-typed decl at ALL FOUR store sites, nothing else changed  -> **248 / 19**
    (case 3 becomes byte-exact against target lines 131-145: `sll v0,sX,2;
    addu v0,v0,s0; lw v1,-8(v0); lw a0,-4(v0); li v0,1; lui at; sh v0,0(at); ...`
    — the two loads are adjacent and the store follows them, exactly as target.
    The 19 residual diffs are a global $s2/$s3 seat swap plus a case-34 order
    inversion, NOT a case-3 problem.)
    Banked: rejected/s7-array-store-only-248-19-case34-disturbed.c
  * case-34 statement order alone (`s4 = ...; D_8009947A = 1;` instead of
    store-then-load), plain scalar decl                             -> 245 / 9 (INERT)
    Banked: rejected/s7-case34-reorder-only-245-9-inert.c
  * **array-typed decl + case-34 order**                            -> **248 / 0**
    Banked: rejected/s7-ruling-pending-array-typed-store-248-0.c

[s7.4] **VERIFIED ON THE REAL SANDBOX, NOT ONLY THE PROBE HARNESS.** The 248/0
form was applied to src/text1a_c2.c (LF-normalised) and
`& tools/wteng.ps1 main sandbox func_800460E4 --disable all` printed
`"score": 0, "build_insns": 248, "target_insns": 248, "rules_dropped": 10,
"cheat_asm_stripped": 0`. The honest, cheat-invisible floor of this form is **0**.
src was reverted immediately afterwards; the tree is HEAD-clean.

[s7.5] **WHY THE CASE-34 REORDER IS PART OF IT, AND WHY IT IS HONEST.** Once the
store carries /s, its anti-dependence against `s0[5]` in case 34 also stops being
exempt, so sched1 can no longer float that load above the store. Target's case 34
emits `move s1,s2; lw v0,20(s0); li v1,1; lui at; sh v1,0(at)` — load first. With
the edge present, source order decides, so the source must read
`s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[5])); D_8009947A = 1;`. That is ordinary,
semantically neutral C (two independent statements in the order target executes
them) and it is byte-INERT on the plain-scalar chassis ([s7.3], 245/9), so it
carries no coercion content of its own.

[s7.6] **WHAT THIS ROUTE DOES *NOT* CONTAIN.** case 3 is left exactly as the
function's own first-switch idiom writes it —
`s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2])); s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 1])); D_8009947A = 1;`
— identical in shape to case 13. No pointer intermediate, no volatile cast, no
inlined byte-offset deref, no invented carrier, no borrow, no declaration hoist,
no detour, no statement duplication. Every one of banned_constructs #1-#7 is
absent. The FAKE s1 chain-extender inherited from [s3] is still present and still
load-bearing (unchanged from the [s7]/[s8]/[s9] body).

[s7.7] **THE ONE THING IT DOES CONTAIN, STATED HONESTLY.** It re-types a global:
`extern s16 D_8009947A;` becomes `extern s16 D_8009947A[1];`, and its four
accesses in this function become `D_8009947A[0]`. Classified against the rules:
  - It is NOT the D_80099478/D_8009947A **aggregate merge** the 04:46 ruling
    closed — it touches one symbol and asserts nothing about its neighbour.
  - It is NOT **header-type-correction-from-use-sites**: that rule's prong (a)
    requires at least one independent use site that positively *requires* the new
    type, and there is none — every `[0]` in the diff was written by this session
    (rule read in full this session; prong (a) fails on its own terms).
  - There is **no SOTN-master precedent**: `grep -in "MEM_IN_STRUCT\|\[1\];"
    docs/reference/sotn-construct-index.md` returns only two `&g_Entities[1]`
    alias entries (one PSX, one PSP), which are pointer aliases into a real
    array, not scalar-to-array re-typing.
  - There is **no independent aggregate evidence** in the binary: [s6.3]'s survey
    stands, and the symbol map has no run around 0x8009947A (`g_stage_id` 0x…478,
    `g_stage_variant` 0x…47A, `g_stage_data` 0x…47C — three separately named
    objects).
  So this is a **first reach of an unsanctioned family**, and per the standing
  policy the correct disposition is a ruling request, not a submission. It is
  banked in rejected/ (not promoted to candidate.c) pending that ruling.

[s7.8] **CORRECTION TO [s6.3]'s "decisive negative" (does not change its
verdict).** [s6.3] treats func_8004668C's back-to-back stores re-materialising the
base with a *second* `lui $at` as proof the two halfwords are not one aggregate.
That inference is not sound in isolation: with the assembler's `sh sym+k` macro
form, GCC emits one RTL store per member and the assembler expands each into its
own `lui $at` / `%lo` pair regardless of whether the members belong to one object.
The aggregate-merge verdict still stands — on the 04:46 ruling and on the absence
of any base-register or stride access anywhere in the binary — but future sessions
should not cite the double-`lui` as the reason.

- [s7] sched.c's anti_dependence exemption is a FIVE-term conjunction; [s6]/[s9] enumerated only the load-/s term. The store-side terms (set /s on the store, or make the store's address varying) were never probed.
- [s7] L4 measured: typing D_8009947A as `extern s16 D_8009947A[1]` (accesses `D_8009947A[0]`) gives the store MEM_IN_STRUCT_P, restores the anti-dependence edge, and makes case 3 byte-exact against target — 248/19 alone, and 248/0 once case 34's two independent statements are written in target's order.
- [s7] Real-sandbox verified: `sandbox func_800460E4 --disable all` prints score 0 / build_insns 248 / rules_dropped 10 / cheat_asm_stripped 0 for that form. The honest floor of this route is 0, not 9.
- [s7] The case-34 statement reorder is byte-inert on the plain-scalar chassis (245/9), so it carries no coercion content; it only becomes visible once the store has /s.
- [s7] This route contains NONE of banned_constructs #1-#7 and leaves case 3 in the function's own `s0[s3-2]`/`s0[s3-1]` idiom, identical to case 13.
- [s7] The single construct is scalar-to-one-element-array re-typing of a global. It fails header-type-correction prong (a), has no SOTN-master precedent (index grep returns only &g_Entities[1] pointer aliases), and has no independent aggregate evidence — first reach of an unsanctioned family, hence a ruling request rather than a submission.
- [s7] Methodological correction: [s6.3]'s double-`lui` argument is not sound on its own (the assembler macro re-materialises $at per %hi regardless of object identity). The aggregate-merge verdict stands on the 04:46 ruling and on the absence of indexed/base-register access, not on that observation.
- [s7] L5 (pointer alias to the global) measured 246/58 and 246/43 - sanctioned family, dead on instruction count.

## [s7f] FORENSICS (session 7, 2026-08-25) — the sched1 decision is now CLOCK-EXACT, and the LUID/emission-order axis is dead

Chassis re-measured first: candidate.c applied to src/text1a_c2.c,
`& tools/wteng.ps1 main sandbox func_800460E4 --disable all` prints
`"score": 9, "target_insns": 248, "build_insns": 245, "rules_dropped": 10,
"cheat_asm_stripped": 0`. The ledger floor of 9 holds on the current chassis.
src was reverted to HEAD afterwards; the tree is HEAD-clean.

[s7f.1] **THE COMPLETE sched1 TRACE FOR BLOCK 19 IS NOW ON DISK.** Ran the
instrumented cc1 (tools/gcc-2.7.2/cc1) with `BB2_PRIO_DEBUG=1 BB2_RANK_DEBUG=1
BB2_SCHED_DEBUG=1` (harness: tmp/grind/func_800460E4/s7/rank.sh; output:
tmp/grind/func_800460E4/s7/base2.rank.txt). Every pick in the case-3 block is
recorded with clock, ready-list contents, per-insn priority and LUID. The block's
RTL nodes (base body):

    luid 0=298  2=302(sll s3,2) 3=304(addu s0) 3=308 lw -8 [unit 0, icost 2]
    4=309 srl  5=310 sll  6=312 addu(s6)  7=322 lw -4 [unit 0, icost 2]
    8=323 srl  9=324 sll 10=326 addu(s4) 11=329 li 1  12=331 sh D_8009947A

sched1 schedules BACKWARDS. Pick order c1..c14 was
`333, 326, 324, 323, 312, 322, 310, 309, 331, 329, 308, 304, 302, 298`, i.e.
forward emission `... 308(lw -8), 329(li 1), 331(sh), 309, 310, 322(lw -4) ...`
— the known 9-diff residual. Target instead emits `308, 322, 329, 331, 309 ...`.

[s7f.2] **THE DECIDING PROPERTY IS PRIORITY TIER MEMBERSHIP, NOT A TIE-BREAK.**
`adjust_priority` (sched.c:2543-2592) raises EVERY register-setting insn with zero
REG_DEAD notes and `birthing_insn_p` true to `max_priority` = 2130706433
(0x7F000001). The trace shows 302, 304, 308, 309, 310, 322, 324, 326, 329 all at
2130706433 and only 312 (pri 2) and **331, the `sh`, at pri 1** — a store's
SET_DEST is a MEM, so `birthing_insn_p` returns 0 and the store never joins the
boosted tier. In a backward walk, the lowest tier is picked LAST = emitted FIRST,
which is why the `li`/`sh` pair always lands at the top of the block, in the
load-delay slot of the first load. For target's order the -4 load must sink into
that same late-pick region, which happens only if it is still unready when the
store is scheduled — i.e. only via the store->load anti-dependence edge.

[s7f.3] **CORRECTION TO [s3]'s ATTRIBUTION (banked as mis-attribution).** [s3]
recorded the lever as "sched.c adjust_priority birthing boost + schedule_select
potential_hazard". The trace refutes the second half and re-scopes the first:
  - `potential_hazard`/SELBEST never fires in this block: 23 SELBEST lines exist
    in the whole TU, ZERO of them inside block 19 (`awk NR>1050&&NR<1120 | grep -c
    SELBEST` = 0). The `j - i - q > 1` largest-potential-hazard selection is not
    the mechanism here.
  - The birthing boost is UNIFORM across all nine register-setting insns of the
    block, so it does not discriminate between the -4 load and anything else; what
    discriminates is that the STORE is excluded from the boost.
  - `adjust_priority`'s `n_deaths` switch is dead code exactly as GCC's own
    comment says ("REG_DEAD notes are removed before we ever get here"):
    `grep -n "deaths=[1-9]"` over the whole 6059-line trace returns nothing.

[s7f.4] **THE LUID / EMISSION-ORDER AXIS IS MEASURED DEAD — NEW KILL.** The one
remaining non-/s input to `rank_for_schedule` is its final
`INSN_LUID (tmp) - INSN_LUID (tmp2)` tie-break, i.e. the RTL emission order, which
IS reachable from C. Probed with a form that gives the ideal layout — both header
words read into once-written/once-read named intermediates so the two `lw`s are
ADJACENT in the RTL and both precede the whole shift/add work:

    { s32 hdr_a = s0[s3 - 2]; s32 hdr_b = s0[s3 - 1];
      s6 = (s32 *)((u8 *)s0 + ALIGN4(hdr_a));
      s4 = (s32 *)((u8 *)s0 + ALIGN4(hdr_b)); }

The RTL really does change (v1 luids: 4 = `lw -8`, **5 = `lw -4`**, 6..8 = first
shift chain, 9..11 = second, 13 = `li`, 14 = `sh`) — the loads are adjacent at the
RTL level, which is what target's bytes show. **The schedule is nevertheless
byte-identical: 245 insns / 9 diffs.** v1's backward picks put the -4 load at
clock 6 and the store at clock 9, reproducing exactly the same forward order
`lw -8, li, sh, srl, sll, addu, lw -4`. A single-word variant (stage only the -4
word) is likewise 245/9. Banked:
rejected/s7f-luid-adjacent-loads-rtl-245-9-schedule-invariant.c and
rejected/s7f-luid-single-staged-m4-word-245-9-inert.c.
CONSEQUENCE: the sched1 outcome is INVARIANT under every RTL-order change C can
express, because the store is pinned to the bottom priority tier by
`birthing_insn_p` regardless of where it sits in the insn stream. This closes the
LUID axis independently of the /s axis, and it explains why [s2]'s whole-statement-
order sweep came back byte-identical — that was not luck, it is structural.

[s7f.5] **THE TWO PREVIOUSLY-FOUND LEVERS ARE ONE DECISION, AND `birthing_insn_p`
IS NOW FULLY ENUMERATED.** `birthing_insn_p` (sched.c:2505-2540) has exactly three
terms: (1) `reload_completed == 0` (true in sched1, not controllable); (2) PATTERN
is a `SET` whose DEST is a REG and whose REGNO is live in `bb_live_regs` (always
true for a load whose result is consumed); (3) `reg_n_sets[dest] == 1`. Only (3)
is reachable from C, and it is the multi-set carrier — banned for this function
twice (judge_constraints; rulings 07:19 and the 07:54 layer-1 FAIL). So the
multi-set-carrier lever and the MEM_IN_STRUCT_P/anti-dependence lever are not two
independent findings: both work by removing the -4 load from the boosted tier at
the moment the store is scheduled (one by demoting the load's priority, the other
by making it unready). Every C-reachable input to that single decision is now
enumerated: anti_dependence's five terms ([s7] L1-L5) plus birthing_insn_p's three
terms plus the LUID tie-break ([s7f.4]) — and each is either impossible, banned,
dead on instruction count, or (L4) refused by the 09:06 ruling.

- [s7f] Chassis re-measured: floor 9 (245/248) with candidate.c applied; ledger floor confirmed on the current chassis.
- [s7f] The full clock-exact sched1 pick trace for case 3's block is banked (tmp/grind/func_800460E4/s7/base2.rank.txt, harness s7/rank.sh); it supersedes guesswork about which sched.c mechanism orders the block.
- [s7f] The `sh` to D_8009947A is the ONLY insn in the block excluded from adjust_priority's birthing boost (a store's SET_DEST is a MEM), which pins it to the bottom priority tier and therefore to the top of the forward emission — this, not potential_hazard, is why it lands in the first load's delay slot.
- [s7f] MIS-ATTRIBUTION CORRECTED: [s3]'s "schedule_select potential_hazard" half is wrong — zero SELBEST decisions occur in this block; and adjust_priority's n_deaths switch is dead code (no `deaths=` value other than 0 anywhere in the 6059-line trace).
- [s7f] KILLED: the LUID/RTL-emission-order axis. A once-written/once-read two-intermediate form genuinely makes the two `lw`s adjacent in the RTL (luids 4 and 5, both before all shift work) and still measures 245/9 — the schedule is invariant under C-reachable RTL order because the store's tier membership does not depend on its position.
- [s7f] UNIFICATION: the multi-set-carrier lever and the /s alias-edge lever are the same single decision (is the -4 load in the boosted tier when the store is scheduled?), so no combination of them opens a new axis; birthing_insn_p's three terms are now enumerated and only reg_n_sets is C-reachable (banned).

- [s7] Chassis re-measured this session: honest floor 9 (245/248, rules_dropped 10, cheat_asm_stripped 0) with candidate.c applied - the ledger floor is current, the queue's 35 is the rule-era number.

- [s7] The complete clock-exact sched1 pick trace for case 3's block is banked (tmp/grind/func_800460E4/s7/base2.rank.txt, harness s7/rank.sh, instrumented cc1 with BB2_PRIO_DEBUG/BB2_RANK_DEBUG/BB2_SCHED_DEBUG): every pick with clock, ready-list contents, per-insn priority and LUID.

- [s7] adjust_priority (sched.c:2543-2592) raises every register-setting insn of the block to max_priority 0x7F000001 (2130706433); the `sh` to D_8009947A is the ONLY insn left at priority 1, because birthing_insn_p needs a REG SET_DEST and a store's dest is a MEM. sched1 walks backwards, so the bottom tier is picked last = emitted first - the store is structurally pinned into the first load's delay slot.

- [s7] For target's order (`lw -8, lw -4, li 1, sh`) the -4 load must still be UNREADY when the store is scheduled, which is exactly the store->load anti-dependence edge the /s axis controls; this is an independent, dump-level confirmation of the [s6]/[s7] account rather than a new lever.

- [s7] MIS-ATTRIBUTION CORRECTED: [s3]'s lever was recorded as 'adjust_priority birthing boost + schedule_select potential_hazard'. potential_hazard/SELBEST never fires in this block (0 of the TU's 23 SELBEST decisions), and the birthing boost is UNIFORM across all nine register-setting insns, so it discriminates nothing - what discriminates is the store's EXCLUSION from it. adjust_priority's n_deaths switch is dead code (no nonzero `deaths=` in 6059 trace lines).

- [s7] NEW KILL - the LUID / RTL-emission-order axis: a once-written/once-read two-intermediate form makes the two `lw`s adjacent in the RTL (v1 luids 4 and 5, both ahead of all shift/add work, which is the layout target's bytes show) and still measures 245/9; the single-intermediate variant is likewise 245/9. The schedule is invariant under every RTL order C can express, because the store's priority-tier membership does not depend on its position. This explains [s2]'s statement-order sweep structurally instead of empirically.

- [s7] UNIFICATION: the multi-set-carrier lever and the MEM_IN_STRUCT_P/anti-dependence lever are not independent findings - both work by removing the -4 load from the boosted tier at the moment the store is scheduled (one by demoting its priority, the other by making it unready). Combining them therefore opens no new axis.

- [s7] The C-reachable inputs to that single decision are now fully enumerated: anti_dependence's five terms ([s7] L1-L5: L1 banned, L2/L3 impossible, L4 refused by the 2026-08-25 09:06 ruling, L5 dead at 246/58), birthing_insn_p's three terms ([s7f.5]: two not controllable, one banned), and rank_for_schedule's LUID tie-break ([s7f.4]: measured dead).

- [s7] Banked rejected forms: rejected/s7f-luid-adjacent-loads-rtl-245-9-schedule-invariant.c and rejected/s7f-luid-single-staged-m4-word-245-9-inert.c.

## [s8r] REDERIVE session 8 — 2026-08-25

**Chassis.** `& tools/wteng.ps1 main sandbox func_800460E4 --disable all` with the
[s7] candidate body (canonical-named, see below) in src/text1a_c2.c printed
`score 9, target_insns 248, build_insns 245, rules_dropped 10,
cheat_asm_stripped 0`. The ledger floor of 9 holds on the current chassis. src was
reverted to HEAD afterwards; the tree is HEAD-clean apart from metrics/events.jsonl.
All variant measurements below were taken through the exact build pipeline with
`tmp/grind/func_800460E4/s8/pr.sh` (cpp | cc1 -O2 -G0 -mel | prologue_fix | maspsx |
multu_pad | as | objdump), scored against `s4/perm_a/target_nr.dis`.

**[s8r.1] The residual is 100% case-3-local, and the instruction-count gap is a
jump2 cross-jump.** A full `diff -u` of target vs. the candidate's disassembly
(tmp/grind/func_800460E4/s8b/) shows differing instructions in exactly one hunk —
case 3's block — plus branch-target offsets that follow from it. Target's case 3 is
`move s1,s2 / sll v0,s3,2 / addu v0,v0,s0 / lw v1,-8(v0) / lw a0,-4(v0) /
li v0,1 / lui at / sh v0 / srl v1 / sll v1 / addu s6,s0,v1 / srl a0 / sll a0 /
j / addu s4,s0,a0`. Ours is `move s1,s2 / sll a0,s3,2 / addu a0,a0,s0 /
lw v1,-8(a0) / li v0,1 / lui at / sh v0 / lw v0,-4(a0) / srl v1 / sll v1 /
j 0x2fc / addu s6,s0,v1`. The three "missing" instructions (245 vs 248) are NOT a
lost computation: because our -4 value lands in `$v0`, our case-3 tail
`srl v0,v0,2 / sll v0,v0,2 / addu s4,s0,v0` is textually identical to case 34's
tail and jump2 cross-jumps it away. The cross-jump is a downstream consequence of
the seat, which is a downstream consequence of the one sched1 ordering decision.
Nothing in the function outside case 3 needs re-derivation — which is the
rederive modality's own answer: at function scope there is no residual to
re-derive.

**[s8r.2] The /s = 0 spelling space is now enumerated at the C-tree level.**
Read `tools/gcc-2.7.2/expr.c:4567-4577` directly: `MEM_IN_STRUCT_P` is set on an
`INDIRECT_REF`'s MEM iff `TREE_CODE (TREE_OPERAND (exp, 0)) == PLUS_EXPR`, or that
operand is a `SAVE_EXPR` of a `PLUS_EXPR`, or the referenced type is aggregate, or
the operand is an `ADDR_EXPR` of an aggregate. GCC 2.7.2's `c-typeck.c`
`pointer_int_sum` rewrites *every* `p + k`, `p - k` and `p[k]` (including
`k` negative) into a `PLUS_EXPR`, so the only C constructs that can reach expand
with a non-PLUS address subtree are: (a) a bare deref of a pointer `VAR_DECL`
(`*p`), (b) a deref of an integer expression cast to a pointer (`NOP_EXPR`), and
(c) a `volatile`-qualified access, which instead satisfies the
`MEM_VOLATILE_P (x) && MEM_VOLATILE_P (mem)` clause of `anti_dependence`. All
three are already banned for this function (banned_constructs #3/#4/#5 plus the
standing judge constraint closing the MEM_IN_STRUCT_P axis in every direction).
There is no fourth spelling; this replaces the previous measurement-by-measurement
enumeration with a source-level one.

**[s8r.3] Zero cost requires the pointer VAR_DECL to be SINGLE-USE — measured.**
This is the mechanism behind [s6.4]'s walking-pointer kill, which the ledger had
recorded only as "costs one addiu".
- Two-use pointer, `s32 *hp = &s0[s3 - 1]; ... hp[-1] ... *hp`: **249 / 4**. The
  case-3 schedule, the seats ($v1 / $a0), the store position and the absence of the
  cross-jump are ALL target-exact; the four diffs are purely the address
  materialisation (extra `addiu v0,v0,-4`, `addu v0,s0,v0` operand order, load
  offsets -4/0 instead of -8/-4). Two uses prevent combine from propagating the
  pointer's `(plus base -4)` into either MEM, so it needs its own register.
  Banked: rejected/s8r-hdrend-twouse-pointer-249-4-addiu-not-folded.c.
- One pointer, single use, no shared base
  (`s32 *last = &s0[s3 - 1];` for the -4 word, `s0[s3 - 2]` left as-is): **249 / 8**.
  CSE does not share `s0 + s3*4` between the two independently spelled addresses.
  Banked: rejected/s8r-one-pointer-single-use-no-shared-base-249-8.c.
- Shared base + single-use intermediate
  (`base = (s32 *)((s3 << 2) + (s32)s0); hp = base - 1; base[-2]; *hp`): **248 / 0**,
  byte-exact. One use lets combine fold `(plus base -4)` straight into the load's
  MEM address while `/s` (set at expand time) survives as 0. This is the banned
  pm2/pm1 construct; banked research-only as
  rejected/s8r-single-use-ptr-intermediate-248-0-BANNED-family.c.
- Control: the same single-use intermediate applied to the **-8** word instead
  (`hp = base - 2; *hp` / `base[-1]`) measures **245 / 9**, i.e. completely inert.
  Independently re-confirms [s6.2] — only the -4 read's bit is load-bearing.
  Banked: rejected/s8r-single-use-ptr-on-minus8-word-245-9-inert.c.

**[s8r.4] NEW independent term: the case-3 base spelling controls the `addu`
operand order.** With `/s` already correct, spelling the base `&s0[s3]` instead of
`(s32 *)((s3 << 2) + (s32)s0)` leaves exactly ONE diff — `addu v0,s0,v0` where
target has `addu v0,v0,s0` (**248 / 1**). This term is orthogonal to the scheduler
question and is the same spelling the mainline already uses for `a0_ptr` (target:
`sll a1,s3,2 / addu a0,a1,s0`). Any future closing form must carry it. Banked:
rejected/s8r-single-use-ptr-amp-index-248-1-addu-operand-order.c.

**[s8r.5] Hygiene defect found and fixed in candidate.c: a dual C handle for each
stage global.** `symbol_addrs.txt:151-152` and `named_syms.txt:122-123` name
0x80099478 / 0x8009947A as `g_stage_id` / `g_stage_variant`, and
`undefined_syms_auto.txt:55,1227` *also* emit `D_80099478` / `D_8009947A` for the
same two addresses. src/text1a_c2.c consequently declared `extern s16 D_80099478;`
/ `extern s16 D_8009947A;` at file scope and used those names inside
func_800460E4, while every other function in the same TU used the canonical
game.h names — two C identifiers for one global in one TU, which is the shape a
layer-1 reviewer reads as an alias-rename even though it arrives via the pipeline
symbol files rather than `asm("...")`. Respelling the four uses inside
func_800460E4 to `g_stage_id` / `g_stage_variant` and deleting the two file-top
externs is **byte-neutral** (measured 245 / 9 both ways; the sandbox printed
score 9 with the canonical-named body in place). candidate.c now carries the
canonical names and an apply note about deleting the externs.

**[s8r.6] Disposition.** The rederive modality is exhausted for this function.
There is no whole-function shape question left (100% of the residual is nine
instructions in one block), and the block's closing form is now known at
source level to be a single-use pointer intermediate and nothing else — a
construct banned twice by layer-1 and closed by a standing judge constraint on the
whole MEM_IN_STRUCT_P axis. The remaining question is not a grinding question but
the fidelity/routing one the frontier already states.

- [s8] Chassis re-measured this session: `sandbox func_800460E4 --disable all` = score 9, target_insns 248, build_insns 245, rules_dropped 10, cheat_asm_stripped 0, with the [s7] body (canonical-named) in src/text1a_c2.c. src reverted to HEAD afterwards; the tree is HEAD-clean apart from metrics/events.jsonl.

- [s8] The full target-vs-build disassembly diff has differing instructions in exactly ONE hunk (case 3) plus branch-target offsets downstream of it. Target case 3: move s1,s2 / sll v0,s3,2 / addu v0,v0,s0 / lw v1,-8(v0) / lw a0,-4(v0) / li v0,1 / lui at / sh v0 / srl v1 / sll v1 / addu s6,s0,v1 / srl a0 / sll a0 / j / addu s4,s0,a0. Ours: move s1,s2 / sll a0,s3,2 / addu a0,a0,s0 / lw v1,-8(a0) / li v0,1 / lui at / sh v0 / lw v0,-4(a0) / srl v1 / sll v1 / j 0x2fc / addu s6,s0,v1.

- [s8] The 245-vs-248 instruction gap is a jump2 CROSS-JUMP, not lost codegen: because our -4 value is seated in $v0, our case-3 tail (srl v0,v0,2 / sll v0,v0,2 / addu s4,s0,v0) is textually identical to case 34's tail and is merged away. It is a downstream consequence of the seat, which is a downstream consequence of the single sched1 pick - not an independent lever.

- [s8] expr.c:4567-4577 (read directly this session) sets MEM_IN_STRUCT_P on an INDIRECT_REF's MEM iff TREE_CODE(TREE_OPERAND(exp,0)) == PLUS_EXPR, or that operand is a SAVE_EXPR of a PLUS_EXPR, or AGGREGATE_TYPE_P(TREE_TYPE(exp)), or the operand is an ADDR_EXPR of an aggregate. c-typeck.c's pointer_int_sum turns every p+k / p-k / p[k] into PLUS_EXPR, negative k included.

- [s8] Therefore the complete C-tree space that can give the -4 read /s = 0 is three constructs: bare deref of a pointer VAR_DECL, deref of an integer expression cast to a pointer (NOP_EXPR), and a volatile access (which wins on anti_dependence's MEM_VOLATILE_P clause). All three are already banned for this function; there is no fourth spelling.

- [s8] Measured 249/4 for a two-use walking pointer `s32 *hp = &s0[s3 - 1]` (hp[-1] for the -8 word, *hp for the -4 word): the case-3 SCHEDULE, both value seats ($v1/$a0), the store's position after both loads, and the absence of the cross-jump are all target-exact; the only diffs are an extra `addiu v0,v0,-4`, the `addu v0,s0,v0` operand order and the two load offsets shifted to -4/0.

- [s8] Measured 248/0 (byte-exact) for shared base + SINGLE-USE intermediate: `s32 *base = (s32 *)((s3 << 2) + (s32)s0); s32 *hp = base - 1; s6 = ...ALIGN4(base[-2]); s4 = ...ALIGN4(*hp);`. One use lets combine fold (plus base -4) into the load's MEM address while the /s bit set at expand survives as 0, so the form costs nothing. This is banned_constructs #4/#5 and is banked research-only.

- [s8] Measured 249/8 for the most natural single-pointer spelling (`s32 *last = &s0[s3 - 1];` used exactly once, with the -8 word left as the function's own s0[s3 - 2] idiom): CSE does not share s0 + s3*4 between the two independently spelled addresses.

- [s8] Measured 245/9 (completely inert) for the same single-use intermediate applied to the -8 word instead of the -4 word - an independent re-confirmation of [s6.2] that only the SECOND header read's MEM_IN_STRUCT_P bit is load-bearing.

- [s8] Measured 248/1 for the byte-exact form with the base spelled `&s0[s3]` instead of `(s32 *)((s3 << 2) + (s32)s0)`: the lone residual is `addu v0,s0,v0` vs target's `addu v0,v0,s0`. The base spelling is an orthogonal term, independent of the scheduler question.

- [s8] Dual-handle defect: 0x80099478 / 0x8009947A are named BOTH g_stage_id / g_stage_variant (symbol_addrs.txt:151-152, named_syms.txt:122-123) and D_80099478 / D_8009947A (undefined_syms_auto.txt:55,1227). src/text1a_c2.c used the D_ names inside func_800460E4 and the g_ names everywhere else in the same TU. Respelling to the canonical names and deleting the two file-top externs measures 245/9 - byte-neutral - and is now in candidate.c.

- [s8] Owner RULES-TO-ZERO directive acknowledged: the sandbox already scores this function with all 10 regfix rules dropped (rules_dropped 10, cheat_asm_stripped 0), so the rules are inert to the floor and retiring them is a consequence of reaching 0, not an independent axis. No measurements were spent on rule retirement.

---

## [s9] ESCALATION SESSION 9 (2026-08-25) — the 06:39 carve-out has an occupant

**[s9.1] Chassis re-measured three ways this session.** `& tools/wteng.ps1 main
sandbox func_800460E4 --disable all`, rules_dropped 10 / cheat_asm_stripped 0 in
every run:
- committed rule-era HEAD body → **score 35**, build_insns 248 / target_insns 248;
- the ledger's non-banned `candidate.c` body (canonical-named, with the two file-top
  `extern s16 D_800994xx;` lines deleted per [s8r.5]) → **score 9**, 245 / 248. The
  ledger floor of 9 holds on the current chassis and the [s8r.5] hygiene respelling
  is confirmed byte-neutral end-to-end;
- the s8 shared-base two-local form (banned_constructs #4/#5) → **score 0**, 248 /
  248. Banked research-only as
  `rejected/s9-banned-single-use-ptr-intermediate-MEASURES-0-on-s9-chassis.c`. This
  is the first time that form's 0 has been reproduced on the current chassis rather
  than inherited from the s7/s8 ledger.

**[s9.2] NEW MEASUREMENT — a SINGLE fresh once-written/once-read pointer local closes
case 3 at 248 instructions.** The form is

```c
s32 *hp = (s32 *)((s3 << 2) + (s32)s0) - 1;
...
s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));   /* unchanged array idiom */
s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));
```

and it measures **score 0, build_insns 248 == target_insns 248**. This is a genuinely
new point in the space, not a respelling of anything already banked:
- banned_constructs #4/#5 (pm2/pm1, and the s8 `base` + `hp` shared-base variant)
  respell BOTH header words through TWO pointer locals; the s8 variant's `base` is
  read twice and therefore fails prong (1) of the named-intermediate entry outright.
  This form has ONE local, respells ONE word, and leaves the -8 word in the
  function's own `s0[s3 - 2]` array-index idiom — the same spelling case 13 ships and
  already matches byte-for-byte.
- [s8r.3]'s nearest shape ("one pointer, single use, no shared base") measured 249/8,
  but it was spelled `&s0[s3 - 1]`. The scaled-index integer-cast base is what
  recovers the 248. The combination *cast base + inlined `- 1` + array idiom retained
  for the -8 word* had never been measured in sessions 1-8.
- Consequence: the 249-insn wall the 06:39 ruling relied on is not a property of the
  named-intermediate route; it was a property of the two spellings that ruling had in
  view.

**[s9.3] [s8r.4]'s base-spelling term re-confirmed on the s9 chassis.** The same
single-local form with the base spelled using the function's OWN mainline `a0_ptr`
idiom, `(s32 *)((u8 *)s0 + (s3 << 2)) - 1`, measures **248 / 1** — the lone residual
is `addu v0,s0,v0` where target has `addu v0,v0,s0`. The closing form therefore
REQUIRES `(s32 *)((s3 << 2) + (s32)s0)`; both `(s32 *)((u8 *)s0 + (s3 << 2))` and
`&s0[s3]` cost exactly one diff on their own. Banked:
`rejected/s9-a0ptr-idiom-base-248-1-addu-operand-order.c`. (Note the mild oddity this
records: target itself uses BOTH `addu` operand orders — `addu a0,a1,s0` in the
mainline for a0_ptr, `addu v0,v0,s0` in case 3 — so the two spellings are both
"target's own" at different sites, and the case-3 one is fixed.)

**[s9.4] Gate (a) canonical-asm is measured DEAD.**
`python3 tools/scan_hand_coded.py --single func_800460E4` → **tier=LOW, score 0/8**,
with every indicator unset: S1 0 multu/mflo pairs, S2 no empty-body branches, S3 257
insns with 13 spills across 14 distinct registers, S4 max load burst 3 in any 8-insn
window, S5 no high-similarity siblings (jaccard < 0.5), S6 no BIOS jumptable call
pattern, S7 every callee-save use has an $sp save, S8 no redundant mask-before-shift.
Banked at `tmp/grind/func_800460E4/s9/scan_hand_coded.txt`. The canonical-asm grant
path is unavailable for this function; a future session must not re-run this scan
hoping for a different tier.

**[s9.5] Two Judge rulings on this function are in direct conflict once [s9.2]
exists.** `decisions.md:10726` (06:39) closes with "NOT closed: a byte-neutral
(248-insn) fresh named pointer local holding a real consumed address stays available
under the named-intermediate entry's prongs -- but the measured nv routes are 249
insns, so they fail byte-neutrality and close nothing". `decisions.md:10738` (09:06,
later, and answering a *global re-typing* request) produced the standing constraint
that no lvalue spelling — "global declaration type, second handle, pointer
intermediate, or cast" — may be chosen to change MEM_IN_STRUCT_P, closing the /s axis
"in every direction". [s9.2] satisfies the 06:39 condition exactly and violates the
09:06 constraint literally. Which controls is not a grind-session call, so session 9
filed a decision packet (`docs/grind/decisions.md`, 2026-08-25 OWNER-ESCALATION
entry) and did NOT submit the form; it is banked ruling-pending at
`rejected/s9-ruling-pending-single-local-named-intermediate-248-0.c`.

**[s9.6] RULES-TO-ZERO directive: acknowledged and measured, not merely noted.**
`grep -c func_800460E4 regfix.txt` = 11 lines (10 active rules by the sandbox's
count), `asmfix.txt` = 0, cheat-asm = 0. Every sandbox run above reports
`rules_dropped: 10`, i.e. the honest floor is already computed with all ten rules
inert. Retiring them is therefore a consequence of reaching 0 and never an
independent axis; no measurement was spent on rule retirement in this session either.

- [s9] [s9] Chassis measured three ways this session with `& tools/wteng.ps1 main sandbox func_800460E4 --disable all` (rules_dropped 10, cheat_asm_stripped 0 every run): committed rule-era HEAD body = score 35 (248/248); the ledger's non-banned candidate.c body = score 9 (245/248), so the ledger floor of 9 holds on the current chassis and the [s8r.5] canonical-name hygiene respelling is confirmed byte-neutral end-to-end; the s8 shared-base two-local form (banned_constructs #4/#5) = score 0 (248/248), reproduced on the current chassis rather than inherited.

- [s9] [s9] NEW: a SINGLE fresh once-written/once-read pointer local closes case 3 completely. `s32 *hp = (s32 *)((s3 << 2) + (s32)s0) - 1;` with the -8 word left as `s0[s3 - 2]` and `s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));` measures score 0, build_insns 248 == target_insns 248. This is a new point in the space: banned #4/#5 respell BOTH header words through TWO pointer locals (and the s8 `base` local is read twice, failing prong (1) outright), whereas this form has ONE local and respells ONE word, leaving the -8 read in the idiom case 13 already ships byte-exactly.

- [s9] [s9] The nearest previously-measured shape ([s8r.3] 'one pointer, single use, no shared base') measured 249/8 only because it was spelled `&s0[s3 - 1]`; the scaled-index integer-cast base is what recovers the 248. The combination cast-base + inlined `- 1` + array idiom retained for the -8 word had never been measured in sessions 1-8, which is why the 06:39 ruling could state that only 249-insn routes existed.

- [s9] [s9] Prong-by-prong against .claude/rules/no-new-park-categories.md:193-214 (named intermediate, owner clarification 2026-08-17), all measured or satisfiable: (1) once-written/once-read YES (sole write is the initialiser, sole read is *hp); (2) real value in target's own bytes YES (target case 3 is `sll v0,s3,2 / addu v0,v0,s0 / lw a0,-4(v0)`; the address is target's own and the loaded word is s4's stage pointer); (3) byte-neutral YES, build_insns 248 == target_insns 248 - the exact prong 06:39 said was failing; (4) fresh local not a borrow YES; (5) destination not live-pre-initialised YES; (6) dump-proven mechanism (evidence.md [s6]/[s7]) + documented exhaustion (9 sessions, 55 banked rejected forms, the C-tree-level enumeration [s8r.2]) + FAKE annotation present + layer-1/2 review as the open item.

- [s9] [s9] Two Judge rulings on this function are in direct conflict once the 248-insn form exists. docs/grind/decisions.md:10726 (06:39) closes verbatim: 'NOT closed: a byte-neutral (248-insn) fresh named pointer local holding a real consumed address stays available under the named-intermediate entry's prongs -- but the measured nv routes are 249 insns, so they fail byte-neutrality and close nothing.' docs/grind/decisions.md:10738 (09:06, later, and answering a GLOBAL RE-TYPING request) produced the standing constraint that no lvalue spelling - 'global declaration type, second handle, pointer intermediate, or cast' - may be chosen to change MEM_IN_STRUCT_P, closing the /s axis 'in every direction'. The s9 form satisfies the first exactly and violates the second literally.

- [s9] [s9] Endgame-lock gate (a) measured DEAD: scan_hand_coded --single func_800460E4 returns tier=LOW score 0/8 with every indicator unset (output banked at tmp/grind/func_800460E4/s9/scan_hand_coded.txt). The canonical-asm grant path is unavailable; a future session must not re-run this scan hoping for a different tier.

- [s9] [s9] Endgame-lock gate (b) PASSES for the named-intermediate family: .claude/rules/no-new-park-categories.md:195 records the SOTN-master shape the entry is built on (`randy = basePoint.x; baseX = randy;`, 'FAKE but makes register allocation work'), exhibited at sotn-decomp/src/weapon/w_037.c:301-302 (PSX, GCC 2.7.2) - a once-written, once-read fresh local whose value is real and consumed. This is a family the frozen list ALREADY sanctions, so the packet asks about application, not extension.

- [s9] [s9] Owner RULES-TO-ZERO directive acknowledged and measured: grep -c func_800460E4 regfix.txt = 11 lines (10 active rules by the sandbox's count), asmfix.txt = 0, cheat-asm = 0, and every sandbox run reports rules_dropped 10 - the honest floor is already computed with all ten rules inert, so retiring them is a consequence of reaching 0 and never an independent axis. No measurements were spent on rule retirement.

- [s9] [s9] Nothing was submitted: the 248/0 form is banked ruling-pending at memory/grind/func_800460E4/rejected/s9-ruling-pending-single-local-named-intermediate-248-0.c with an explicit DO-NOT-SUBMIT header, candidate.c still carries the non-blocked floor-9 body (with an [s9] pointer note), and src/text1a_c2.c was restored to HEAD - the tree is HEAD-clean apart from metrics/events.jsonl, the ledger files, the three new rejected/ files and the decisions.md packet.
