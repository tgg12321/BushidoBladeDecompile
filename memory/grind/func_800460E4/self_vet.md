# SELF-VET — func_800460E4

CONSTRUCTS: (1) `hp`, a fresh block-scoped pointer local in the second switch's stage-3 arm, assigned exactly once and dereferenced exactly once, naming the location of the stage table's final entry; (2) a FAKE-annotated algebraically-neutral detour on the live default initialization of the stage-pointer local, which folds back to the direct assignment with zero emitted bytes; (3) the mainline scaled base expression bound to `a0_ptr`, with the earlier parameter-alias locals dropped; (4) canonical header names `g_stage_id` / `g_stage_variant` replacing this file's duplicate splat-named externs for those two objects; (5) ordinary block-scoped offset locals `off1_raw`, `off`, `off3`.

## T1 semantic purpose
- (1) `hp` names a real address that the target itself computes (`sll v0,s3,2 / addu v0,v0,s0 / lw a0,-4(v0)`) and the word it loads is the stage pointer that initializes `s4`. The value is consumed; the local is not a discard, not an address-of pad, not a constant holder. It is byte-neutral: 248 built insns vs 248 target insns.
- (2) The detour carries a LIVE value: the stage-pointer local's default initialization. Removing it entirely (measured this session as the direct assignment) still builds 248 insns but scores 32, so the construct is load-bearing; combine folds it back to the direct form and it materializes no bytes. Declared as a FAKE construct under the sanctioned scope extension below — its surviving effect is the `reg_n_refs` count, and I state that plainly rather than dressing it as semantics.
- (3) An ordinary base-plus-scaled-index address expression, consumed three times in the mainline. Dropping the parameter-alias locals removes indirection rather than adding it.
- (4) Pure hygiene with a semantic purpose: it removes a genuine defect (two C handles for each of 0x80099478 and 0x8009947A inside one TU). Byte-neutral, measured both ways in session 8.
- (5) Ordinary named sub-expressions holding real, consumed offsets.

## T2 human-programmer
- (1) Yes. A programmer reading a stage header whose word count lives in `s0[0]` naturally names the pointer to the header's last word before dereferencing it. The same file already spells the mainline read this way (`a0_ptr[-1]`).
- (2) No — this one would draw a "why is this here?" from a reader, which is exactly why it is FAKE-annotated and claimed under an explicitly sanctioned scope extension rather than presented as natural code.
- (3), (4), (5) Yes. These are the forms a programmer writes by default; (4) is what the rest of the TU already does.

## T3 GCC-internals justification
- (1) The mechanism is dump-proven (evidence.md [s6]/[s7]) and I name it in the FAKE annotation as the rules require, but the construct does not need the mechanism to be defensible: it is a once-written/once-read fresh local holding a real consumed value, which the frozen entry admits "whatever GCC pass it acts through". I am not claiming that GCC internals make an otherwise-pointless construct legitimate.
- (2) The mechanism (`flow.c` `reg_n_refs` feeding `global.c` allocno priority) IS the justification, and I say so — that is what the sanctioned scope extension for this family explicitly contemplates, together with its extra prerequisite that the fold emits zero bytes, which I verified.
- (3), (4), (5) No GCC-internals justification; these are program-logic forms.

## T4 permuter/search provenance
No permuter or automated search produced any construct in this diff. (1) was derived by hand in session 9 from the frozen entry's six prongs and measured; (2) dates to session 1 and is unchanged; (3)/(4)/(5) were hand-authored in sessions 7-8. Nothing here passes detectors merely because a detector misses the spelling: every construct is declared and claimed to a named family below.

## T5 family check
- (1) Frozen named-intermediate entry, and the owner's 2026-08-25 ruling on the session-9 decision packet sanctions this exact form for this exact function.
- (2) The dead-store family's own 2026-07-01 scope extension (see claims below), FAKE-annotated, byte-neutral, non-materializing.
- (3), (4), (5) No family needed — ordinary C.
- Explicit non-matches, checked against this function's standing bans: no volatile cast anywhere in the body; no second C handle for any object (construct (4) removes the only such pair); no two-pointer respelling of the header words — the -8 word keeps the plain array-index idiom identical to the stage-13 arm; no inlined integer-cast byte-offset deref; no function-invented multi-write carrier, since every local introduced here is written once; the parameter is not reused as a carrier and the stage-pointer local is not deleted.

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `tail`, `slack`, or any sibling. `hp` = header pointer, `a0_ptr` / `off1_raw` / `off` / `off3` describe the values they hold, and every one of them is read. `g_stage_id` / `g_stage_variant` are the committed canonical names from `include/game.h`.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: named intermediate (fresh once-written/once-read local), for construct (1)
  SCOPE: "A fresh named intermediate therefore qualifies under this entry **whatever GCC pass it acts through** (LUID bias, cse.c re-materialization, allocno priority), provided ALL of: (1) once-written, once-read — multi-write carriers are NOT this entry (the `y1` FAIL, decisions.md:1833, stands); (2) real value — the intermediate holds a computation that appears in the target's own bytes and only relocates where the value is named; pure no-op copies stay with the dead-store family and its prerequisites; (3) byte-neutral — `build_insns == target_insns`, the compiler folds the copy; (4) fresh local, not a borrow — [[staged-value-reused-variable]] keeps its own bounds; (5) destination not live-pre-initialized (the `x/tx` FAIL, decisions.md:4251, stands); (6) standard prerequisites: dump-proven named mechanism, documented lever exhaustion, `/* FAKE: ... */` annotation, layer-1 + layer-2 review."
  PRECEDENT: docs/reference/sotn-construct-index.md:113
  PRECEDENT: docs/grind/decisions.md:11006
  PRECEDENT: .claude/rules/no-new-park-categories.md:193

  FAMILY: dead-store FAKE exception, 2026-07-01 scope extension, for construct (2)
  SCOPE: "a LIVE store/computation routed through an algebraically-equivalent detour that combine folds back to the direct form with ZERO emitted bytes — its only surviving effect is the extra `reg_n_refs` count flow.c records before the fold."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:32

ANNOTATION-CONFORMANCE:
  /* FAKE: fresh once-written/once-read pointer intermediate naming the address of the stage header's last word, mechanism: expand-time MEM_IN_STRUCT_P (expr.c:4567-4577) -> sched.c anti_dependence exemption -> sched1 load/store order, lever-exhaustion: memory/grind/func_800460E4/hypotheses.md + evidence.md [s1]-[s8r] */
  /* FAKE: live default init of s1 routed through a delta-rebase detour that combine folds back to s1 = s4 with zero emitted bytes, mechanism: flow.c reg_n_refs (+2 on s1's pseudo) lifts its global.c allocno_compare priority above the s2 pointer so allocation order matches target, lever-exhaustion: this function's grind ledger evidence.md [s1]+[s3] */
  Both lines are present verbatim at their construct sites in src/text1a_c2.c, and both carry what + a named GCC-pass mechanism + a lever-exhaustion pointer. Both constructs sit inside the sanctioned families claimed above.

## Prong evidence for construct (1) — the six named-intermediate prongs
(1) once-written, once-read: sole write is the initializer, sole read is the single dereference. (2) real value: the address is target's own (`sll` / `addu` / `lw -4`), and the loaded word is the stage pointer stored into `s4`. (3) byte-neutral: `sandbox func_800460E4 --disable all` printed score 0, target_insns 248, build_insns 248, rules_dropped 10, cheat_asm_stripped 0 THIS session. (4) fresh local, not a borrow: declared in the arm's own block. (5) destination not live-pre-initialized. (6) mechanism dump-proven in evidence.md [s6]/[s7]; exhaustion is 10 sessions, 6 modalities, 56+ banked rejected forms plus the source-level enumeration [s8r.2]; the FAKE line is present; layer-1 and layer-2 review are the open item.

## Verification record
- `& tools/wteng.ps1 main sandbox func_800460E4 --disable all` → score 0, 248/248, rules_dropped 10, cheat_asm_stripped 0.
- Control measured this session: replacing construct (2) with the direct assignment → score 32 (248/248). Construct (2) is load-bearing and non-materializing.
- The 10 regfix/asmfix rules this function still carries on main are now dead weight: the honest body needs none of them. Retiring them is an operator/driver step (`retire`), outside this session's allowed surface.
