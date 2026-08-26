# SELF-VET — func_8001F938 (grind session s11, 2026-08-25)

Diff under review: `src/code6cac.c` — `INCLUDE_ASM("asm/funcs", func_8001F938);`
replaced by the pure-C body (identical to `memory/grind/func_8001F938/candidate.c`).
No other file in the build pipeline is touched. Sandbox `--disable all` = **0**
(build_insns 107 == target_insns 107); full-build `verify-oracle` exit 0 (SHA1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa).

CONSTRUCTS: (C1) `kind = kind_full & 0xFFFFU;` — a named 16-bit-masked copy of the
0x6A field, used for the `==` comparison set while the raw `kind_full` drives the
`(u32)((s32)kind_full - K) < 2U` range checks; (C2) `s16 raw_or_3` — a short local
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
  (`if (raw_or_3 >= 4) raw_or_3 = 3;`) and consumed by `idx = raw_or_3 * 2;`.
  Every statement is executed and every value is read.
- C3/C4/C5: ordinary control flow and ordinary named locals; each intermediate is
  written once and read.

## T2 human-programmer
- C2: yes. A programmer told "read the halfword damage counter at +0x270, clamp it
  to 3, use it to index the `s16` table at +0x276" writes exactly this: a `short`
  local, an in-place clamp, `* 2` for the halfword stride. There is nothing in it a
  reader asks "why is this here?" about — it is the shortest correct spelling. It is
  also strictly SIMPLER than the s10 form it replaces (`s32 probe` + a separate
  `raw_or_3` + `(raw_or_3 << 16) >> 15`).
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
`sp_*` or similar. `kind_full`/`kind`/`raw_or_3`/`val`/`factor`/`idx`/`sum` all name
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
