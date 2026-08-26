# SELF-VET — func_8001F938 (grind session s11, 2026-08-25)

Diff under review: `src/code6cac.c` — `INCLUDE_ASM("asm/funcs", func_8001F938);`
replaced by the pure-C body (identical to `memory/grind/func_8001F938/candidate.c`).
No other file in the build pipeline is touched. Sandbox `--disable all` = **0**
(build_insns 107 == target_insns 107); full-build `verify-oracle` exit 0 (SHA1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa).

CONSTRUCTS: (C1) `kind = kind_full & 0xFFFFU;` — a named 16-bit-masked copy of the
0x6A field, used for the `==` comparison set while the raw `kind_full` drives the
`(u32)((s32)kind_full - K) < 2U` range checks; (C2) `s16 dmg` — a short local
holding the halfword field at +0x270, clamped in place and scaled by `* 2`;
(C3) `goto`-based control flow with a shared `clamp:` exit and an inline `return`;
(C4) block-scoped named intermediates (`f`, `vv0`, `vv1`, `sum`, `sum_or_3`);
(C5) `(sum < 4) ? sum : 3` ternary clamp. No inline asm, no volatile, no dead
stores, no self-assigns, no unused locals, no register pins, no pad/dummy locals,
no `__asm__`, no alias renames.

## T1 semantic purpose
- C1: the mask is redundant *as a value operation* — `kind_full` is loaded through
  `*(u16 *)` so its top 16 bits are already zero, and removing the mask does not
  change what the function computes. Its effect is on WHICH C-level value each
  comparison group reads: the equality set reads the masked name, the subtraction
  set reads the raw name. Disclosed plainly rather than argued away: this construct
  is byte-load-bearing and value-neutral. See T5 for its disposition.
- C2: fully semantic. `s16` is the ACTUAL width and signedness of the field being
  read (`*(s16 *)(arg0 + 0x270)`); holding a halfword field's value in a `short`
  is a type choice, not a coercion device. The value is clamped in place
  (`if (dmg >= 4) dmg = 3;`) and consumed by `idx = dmg * 2;`.
  Every statement is executed and every value is read.
- C3/C4/C5: ordinary control flow and ordinary named locals; each intermediate is
  written once and read.

## T2 human-programmer
- C2: yes. A programmer told "read the halfword damage counter at +0x270, clamp it
  to 3, use it to index the `s16` table at +0x276" writes exactly this: a `short`
  local, an in-place clamp, `* 2` for the halfword stride. There is nothing in it a
  reader asks "why is this here?" about — it is the shortest correct spelling. It is
  also strictly SIMPLER than the s10 form it replaces (`s32 probe` + a separate
  `dmg` + `(dmg << 16) >> 15`).
- C1: a reader can ask "why mask a value that was just loaded as a u16?". The honest
  answer is that a 16-bit code masked out of a status word is idiomatic defensive C,
  but the mask is not *required* by the program logic here.
- C3/C4/C5: yes — ordinary decompiled-C shapes.

## T3 GCC-internals justification
- C2: NO. The construct is justified by the field's type; the GCC behaviour (a HImode
  pseudo materialised with `lhu`, plus a separate `lh` for the sign-extended compare
  operand) is an OBSERVED CONSEQUENCE recorded in the ledger, not the reason a
  programmer would write `short`. The C is what it is; the mechanism explains why the
  target's two loads are not evidence of a dual typed view (which is what ten prior
  sessions wrongly concluded).
- C1: partially. The reason the masked name is kept separate from the raw name is that
  it reproduces target's `andi $v1,$a1,0xFFFF`; a `(u16)` cast spelling produces `move`
  instead. That is a codegen-shaped justification and is disclosed as such.

## T4 permuter/search provenance
None of these constructs came from a permuter or automated search. C2 was derived by
reading the target region and the per-pass cc1 evidence banked in s6/s7, then measured
directly (floor 8 -> 4 -> 2 -> 0 over three hand-written spellings this session). The
s4/s5 permuter campaigns produced only cheats (`volatile short pad`, a dead self-assign)
and are recorded as KILLED; nothing from them survives in this diff.

## T5 family check
- C2 is NOT a member of any forbidden family, and specifically NOT the pre-banned
  signedness-split / dual-typed-read family: there is exactly ONE dereference of
  +0x270 in the C, of exactly ONE type (`s16`), with no `(s16)`/`(u16)` cast, no
  union, no second pointer, no volatile, and no guard. Prior FAILed forms in
  `memory/grind/func_8001F938/rejected/` all contained a second *typed view*
  (`*(u16*)` + `(s16)` cast, guarded ternary, union, two-pointer) or a
  purpose-free `u16` truncation of a signed read; this form contains neither.
  It is ordinary C typing.
- C1 matches no forbidden family literally (it is not a dead store, dead local,
  pad, alias, volatile coercion, pin, barrier, or asm injection) but it IS a
  value-neutral construct kept for its codegen effect, so it is disclosed for the
  reviewer's judgement rather than defended by analogy.
- C3 is the ordinary mixed-exit shape (`.claude/rules/cross-jump-store-tail-merge.md`
  — "ordinary C; no FAKE needed" per the family-selection table).

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `tail`, `slack`, `_buf`,
`sp_*` or similar. `kind_full`/`kind`/`dmg`/`val`/`factor`/`idx`/`sum` all name
the value they hold, and every one of them is read.

SANCTIONED-FAMILY-CLAIMS: none.
  No frozen-list family is claimed for any construct in this diff. C2 and C3-C5 are
  claimed as ORDINARY C (no exception needed). C1 is claimed as ordinary defensive
  masking and is disclosed above as value-neutral; it is NOT presented as a member of
  variable-reuse, split-init, named-intermediate, or any other sanctioned family, and
  no scope sentence is quoted for it because none applies. Supporting (not
  authorising) evidence: an independent fresh adversarial cheat-reviewer examined this
  exact construct on this exact function in session s2 and returned PASS —
  `tmp/grind/func_8001F938/s2/cheat_reviewer_verdict.txt:4`. If the layer-1 reviewer
  disagrees, C1 is the ONLY construct at issue: the rest of the function is byte-exact
  and the ledger records four measured alternative spellings of the 0x6A read
  (floor 1 / 16 / 16 / 16), so a bounce is a one-construct problem, not a re-grind.

ANNOTATION-CONFORMANCE: n/a — no /* FAKE */ construct in the diff (no sanctioned-family
exception is claimed, so no annotation is mandated by any cited rule).

---

## s11b ADDENDUM (2026-08-25, escalation modality) — status and the family-scope argument

This vet is NOT being submitted as a candidate-ready. The layer-1 cheat-reviewer FAILed
C2 on 2026-08-25 23:08 as "a new spelling of the pre-banned signedness-split/dual-typed-view
fold-defeat family", and the driver banned the exact statement text, so re-submitting it —
under any identifier — would be a re-declared banned construct and an invalid session.
Session s11b instead re-measured the form, re-verified the oracle, and filed a RULING
REQUEST on the scope of that family finding. The renaming of the local from `raw_or_3` to
`dmg` is a T6 hygiene fix, not an attempt to respell around the ban; codegen is identical
(sandbox 0 both ways) and the ban is treated as covering both names.

RE-MEASURED THIS SESSION (s11b, independent of s11's claims):
- `sandbox func_8001F938 --disable all` = score 0, build_insns 107 == target_insns 107,
  rules_dropped 0.
- `verify-oracle` = ok true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
  original_sha1_locked. Zero regfix/asmfix rules, zero cheat-asm, zero inline asm.
- `scan_hand_coded --single func_8001F938` = tier LOW 0/8 (endgame-lock gate (a) FAILS —
  as it must, since a pure-C form exists; the region is not hand-written asm).

WHY THE FAMILY FINDING IS ARGUED TO BE A SCOPE ERROR (the ruling question):
1. The banned family is defined by TWO typed views of ONE address. The pre-ban enumerates
   its spellings verbatim: "guarded ternary, unconditional split, union, two-pointer, or
   single-u16-read + (s16) cast". Every one contains two C-level views. This construct
   contains exactly ONE dereference, of exactly ONE type, with no cast, no union, no
   second pointer, no guard, no volatile.
2. The 2026-07-23 ruling states the family's harm verbatim: "the second dereference
   changes nothing about what the program computes ... its ONLY function is to create two
   distinct internal expression nodes so CSE will not collapse them". There is no second
   dereference here to be purpose-free. The target's `lhu` is emitted by cc1's own HImode
   pseudo materialisation, not by a C-level memory reference.
3. Checklist T1 inverts against the finding: this construct is the SIMPLER form. The
   currently-blessed floor-8 spelling (`s32 probe` + `(probe << 16) >> 15`) is strictly
   more contrived than `s16 dmg = ...; if (dmg >= 4) dmg = 3; idx = dmg * 2;`. A cheat
   rule that forces the more convoluted C is being applied outside its purpose.
4. Checklist T2 passes outright: `short` is the ACTUAL width of the field, and clamp-then-
   index is the shortest correct spelling of the program logic.
5. In-repo shape precedent among zero-rule byte-matched COMPLETED-C functions:
   `src/code6cac_b.c:377` (func_8002798C: 0 regfix, 0 asmfix, absent from engine/queue.json
   and inline_asm_canonical.txt) and `src/code6cac.c:777` / `src/code6cac.c:792`
   (func_8001B478). 135 narrow-typed locals ship across src/. The shape is project-native
   ordinary C, not a new family.
6. Fidelity evidence: the target's 8-byte frame reserves the HImode local's own stack slot.
   The shipped binary allocates storage that exists only if the original source held this
   value in a `short` local.

If the ruling upholds the family finding, the honest consequence is that no pure-C form of
this function is permissible, the function stays INCOMPLETE at floor 8, and the endgame-lock
disposition applies — NOT that another spelling should be hunted.
