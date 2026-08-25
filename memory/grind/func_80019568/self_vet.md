# SELF-VET — func_80019568   (session 3, 2026-08-25, permuter modality)

Measured THIS session, on today's chassis, with the diff in place in src/code6cac.c:
`& tools/wteng.ps1 main sandbox func_80019568 --disable all` ->
`{"score": 0, "target_insns": 141, "build_insns": 141, "rules_dropped": 5}`
(log: tmp/grind/func_80019568/s3/sandbox_final.json; disassembly of the produced
object: tmp/grind/func_80019568/s3/build_func.txt).

This vet SUPERSEDES the 2026-08-25 12:52 vet that drew a layer-1 FAIL. The FAIL was
a CITATION defect, not a construct defect: C2 was filed under
named-local-fake-exception, whose scope does not cover it. The owner/Judge ruling of
2026-08-25 13:45 (docs/grind/decisions.md:11114) resolved the family question — the
default-initialised flag is ordinary C and the per-arm write-out is
duplicated-statement-into-arms — and that is how it is filed here. The uninitialised
dead-pass-through shape that was banned (`s32 enable;` with no initialiser and a
value-less `enable = 0;` in the else arm) is NOT present in this diff.

CONSTRUCTS: (C1) per-iteration record pointers `u8 *rec` / `s16 *o` at the top of the
loop body; (C2) `s32 enable = 0;` default-off flag local, raised to 1 in the valid arm,
with the `o[2] = enable;` store written into BOTH arms; (C3) `s32 *p = &D_80102790;`
read-modify-write in the tail; (C4) the `sp` frame struct + `u8 *packets` byte view over
`sp.packets[]`; (C5) walking pointers `dst0`/`dst1`/`src` in the copy-out loop.

## T1 semantic purpose
C1: yes — `rec` and `o` are the record being parsed and the slot being written this
iteration; every subsequent statement in the body reads one of them. Removing them
means re-writing `packets[i*8+k]` / `sp.output[i+k]` at nine use sites.
C2: yes — `enable` carries live state across the join. It is DEFAULT-OFF at the top of
the body and RAISED to 1 only when the record is valid (`rec[0] == 0`); the initialiser
is READ on the else path. Delete the initialiser and the else arm stores garbage; delete
the variable and the slot's enable word is never written. It is not a dead pass-through.
C3: yes — `*p` is loaded (`old_mask`), the global is then overwritten with the new mask,
and `old_mask` is consumed by three subsequent expressions. Real read-modify-write.
C4: yes — the outgoing SPU packet block and its 4 s16 outputs live in this frame; both
are passed by address to func_8001B138 / func_8003A728. `packets` is the byte view the
per-field record reads need.
C5: yes — the copy-out loop moves four halfwords into two destination runs.
Only the DUPLICATION of C2's single store into two arms (rather than one copy after the
join) is match-motivated rather than semantically forced; it is FAKE-annotated below.

## T2 human-programmer
Yes for all five. A programmer writing "for each of the two records: if the record is
valid, publish its voice id and mark the slot enabled, else publish the fallback and
leave the slot disabled" writes a default-off flag and per-iteration record pointers
without prompting. `s32 *p = &G; old = *p; *p = new;` for a global read-modify-write is
idiomatic 1990s C. Nothing here reads as "why is this here?" from a spec standpoint:
there is no unused variable, no self-assignment, no empty statement, no discard, no
address-of-only local, no width-only cast, and no name announcing coercion intent.

## T3 GCC-internals justification
The PROGRAM LOGIC is the explanation for every construct; the GCC mechanism is recorded
as evidence for why the alternative spellings measured worse, not as the reason a
construct exists.
C1 is how you parse a record per iteration. (The banked mechanism — a lone DEST_REG giv
leader has benefit 2, loop.c:3804 subtracts add_cost*biv_count to 0, loop.c:3824 marks it
not-worth-while, all_reduced=0, so loop.c:4034 skips biv elimination and the counter
survives — explains why the flat-index spelling diverged, and it also says the ORIGINAL
was the pointer spelling, since target keeps the counter and the `slti` exit test.)
C2's flag is program state; the movable analysis (loop.c:695-716: no movable is created
for a pseudo with n_times_set==2 and non-consecutive sets, so the `addiu $v0,$zero,1`
stays in the loop) is the evidence that target's own bytes came from a multiply-set
variable — target hoists the literal 4 into $t2 at :49/:60 but leaves the 1 in-loop.
C3 is an RMW; the note that one address register serves both accesses is a consequence.
No construct here is justified by an allocator/scheduler internal with no program-level
reading, and there are no lever-named constructs, no register pins, no barriers.

## T4 permuter/search provenance
None of these came out of a permuter run as an opaque winner. C1/C2/C3 were derived by
reading tools/gcc-2.7.2/loop.c and the target asm and were each measured by hand
(hypotheses.md H5, H6/H10/H14-H24, H8/H9/H11/H12). ~40k permuter iterations across three
distinct seeded chassis produced NO novel form and are recorded as a negative result in
evidence.md [s3]/[s3b], not as the provenance of anything in the diff. Every construct
survives explanation without reference to a search: I can state what each one does from
the spec, and the rejected/ bank shows the alternatives that were tried and what they
measured.

## T5 family check
C1, C4, C5 are ordinary C and claim no family; layer-1 already PASSED C1 on 2026-08-25.
C2 needs one family for the DUPLICATION of the store into both arms:
duplicated-statement-into-arms. The flag itself needs no family (owner ruling
2026-08-25 13:45). C3 is pointer-rmw-global-sanctioned and layer-1 already PASSED it on
2026-08-25. I checked the adjacent families and they do NOT apply, deliberately:
 - named-local-fake-exception is the WRONG file for C2 (that is the constant-holder /
   dead-scalar shape; `enable` is neither dead nor a constant holder). This is the exact
   mis-citation that produced the 12:52 FAIL and it is corrected here.
 - dead-store-fake-exception does not apply: neither `o[2] = enable;` copy is dead — the
   target emits both (`addiu $v0,$zero,1; sh $v0,0x4($a2)` at func_80019568.s:35-36 and
   `sh $zero,0x4($a2)` at :61).
 - defeat-licm-hoist-var-reuse does not apply: `enable` is a FRESH local, not an
   existing local borrowed for a second unrelated value.
 - pointer-alias-fake-exception does not apply to C3: this is a load AND a store through
   `p`, which is the RMW shape the narrower rule sanctions, not a single-use alias.
No construct in the diff matches any entry in the forbidden-family catalog, by analogy
or by respelling: there is no register-asm pin, no hardcoded-$N asm, no scheduling
barrier, no volatile coercion of any spelling, no unused-local-array frame coercion, no
dead-param-assign, no dead-conditional-store, no empty-body if, no `if (1)`, no
dead-goto pad, no DImode chain, no `asm("sym")` rename, no opaque `s32 one = 1;`, no
redundant width cast, and no linker-script reorder. The diff also DELETES the rule-era
body's cheat-asm content and retires 5 regfix rules — the direction is strictly
cheat-reducing.

## T6 naming-announces-intent
Names in the diff: `rec`, `o`, `enable`, `bits`, `voice2`, `voice_mask`, `old_mask`, `p`,
`packets`, `output`, `base_addr`, `dst0`, `dst1`, `src`, `i`, `sp`. None is `pad`,
`_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`, `tail`, `slack`, or `_frame_pad`.
Every one of them is read at least once on a live path; none exists only as a discard,
an address-of, or a bare declaration. `enable` names the slot word it writes and `p`
names the pointer it is.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: duplicated-statement-into-arms  (construct C2's per-arm `o[2] = enable;`)
  SCOPE: "Writing the SAME real statement in two or more control-flow arms — instead of sharing one copy via a label/goto — is a legitimate matching technique, **including** when:"
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:63
    Prerequisites, each verified against this diff:
    (1) REAL on its path — target emits both stores (asm/funcs/func_80019568.s:35-36 and :61).
    (2) Byte-neutral vs the canonical reference — build_insns 141 == target_insns 141,
        sandbox score 0 this session. (The joined spelling measures 21/136, i.e. FOUR
        target instructions SHORT, so the duplication reproduces target's own duplication
        rather than materialising instructions; this was the misreading the Judge
        corrected on 2026-08-25 13:45.)
    (3) Lever-exhaustion documented — hypotheses.md H14/H15/H17-H20/H22-H24 plus the
        rejected/ bank: armscope-single-set-named-local-8.c, blockscope-enable-pointer-8.c,
        bare-literal-o2-li-hoisted-8.c, computed-flag-sltiu-10.c,
        default-store-then-override-13.c, flag-store-after-join-21.c,
        inverted-arms-li-still-hoisted-17.c, voice-reuse-instead-of-flag-8.c, plus ~40k
        permuter iterations over three seeds with no novel find.
    (4) FAKE annotation present on the duplicated copy — see ANNOTATION-CONFORMANCE.
    (5) Dual review — layer-1 runs on this diff; layer-2 at integration.

  FAMILY: pointer-rmw-global-sanctioned  (construct C3, `s32 *p = &D_80102790;`)
  SCOPE: "Sanctioned: a pointer local to a global used for an actual READ-MODIFY-WRITE sequence (load through it, compute, store through it) — the pointer has at least one load AND one store use."
  PRECEDENT: .claude/rules/pointer-rmw-global-sanctioned.md:36
    Shape check: `p` has exactly one load (`old_mask = *p;`) and one store
    (`*p = sp.voice_mask;`), zero displacement, neutral name, and real program logic
    between them (the three derived masks consume `old_mask`). Layer-1 PASSED this exact
    construct on 2026-08-25 12:52. Lever-exhaustion: hypotheses.md H8/H9/H11/H12
    (aggregate and statement-order explanations both KILLED by measurement).

ANNOTATION-CONFORMANCE:
  /* FAKE: the `o[2] = enable;` store is written into BOTH arms rather
   * than once after the join (family: duplicated-statement-into-arms,
   * .claude/rules/duplicated-statement-into-arms.md; owner ruling
   * 2026-08-25 13:45, docs/grind/decisions.md).  mechanism: loop.c scan_loop
   * (loop.c:695-716) only creates a movable for the `1`-holding
   * pseudo when it has a single set or consecutive sets; the
   * loop-top default plus this in-arm set are non-consecutive, so no
   * movable exists and the `addiu $v0,$zero,1` stays in the loop
   * filling target's lhu load-delay slot.
   * lever-exhaustion: memory/grind/func_80019568/hypotheses.md
   * H6/H10/H12 + s3 H14-H17 (bare literal 8/142, `bits` carrier
   * reuse 6/141, computed `enable = (rec[0] == 0)` 10/142,
   * single store after the join 21/136). */
  It carries all three required parts: WHAT (the store is duplicated into both arms
  instead of sharing one copy after the join), MECHANISM (a NAMED GCC pass —
  loop.c scan_loop's movable creation test at loop.c:695-716), and LEVER-EXHAUSTION (a
  pointer to the hypotheses ledger plus the four measured alternatives with their
  scores). C3 carries no FAKE annotation and needs none:
  pointer-rmw-global-sanctioned.md is a user-decision precedent for the exact shape and
  states no annotation prerequisite.

INTEGRATION NOTE (not a construct question): func_80019568 still carries 5 regfix rules
calibrated to the superseded rule-era body. They must be retired by the driver's normal
`retire` step before the full build; the sandbox already scores with them dropped. I did
not touch regfix.txt.
