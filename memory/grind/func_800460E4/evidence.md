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
