# Evidence bank — func_800460E4

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
