# SELF-VET — _exeque

CONSTRUCTS: `s32 *p = &D_8009BE7C;` (pointer alias to a global, carried
unchanged from s2 H5b); no new constructs introduced this session (the s3
change replaced a variable-reuse construct with a plain direct assignment,
i.e. REMOVED a construct rather than adding one).

## T1 semantic purpose
`s32 *p = &D_8009BE7C;` has an observable effect: it is dereferenced for
BOTH the guard read (`*p != 0`) and the clear store (`*p = 0;`) — removing
it changes the function back to two separate named-global accesses, which
is a real (not cosmetic) difference in how the address is computed. The s3
change (dropping the `mask` intermediate for the triple-store block) has
NO semantic-purpose question to answer because it removes a construct,
leaving two plain `GLOBAL = struct_field_access;` assignments — ordinary C
with no indirection to justify.

## T2 human-programmer test
Yes. A programmer implementing "save the queue slot's arg pointer and byte
count into the two globals used elsewhere for the last-run info" would
naturally write `D_8009BF6C = (s32)_que[idx].arg; D_8009BF70 = _que[idx].count;`
directly — there is no reason a human would introduce an intermediate
`mask` variable here, so the s3 simplification is, if anything, MORE
naturally human-written than the s2 form it replaces. The `s32 *p =
&D_8009BE7C;` pointer is also a natural spelling for "the guard flag we're
about to clear."

## T3 GCC-internals justification test
The pointer alias's justification (`defeat-combine-symbol-fold`) DOES
reference a GCC pass by name — but per [[ordinary-c-judge-decidable]]
Ruling 1(3), a construct with a real semantic reading is never a cheat
merely because its selection was informed by observing codegen; the
pointer is genuinely read and written through (not a discard/pin), so this
does not fail the test. The s3 direct-assignment simplification has NO
GCC-internals justification in its own right — it was chosen because it is
BYTE-IDENTICAL to two other measured spellings and is the simplest of the
three, not because of any internals argument for why it should differ.

## T4 permuter/search provenance
No permuter or auto-search tool was used this session. All three s3
variants (mask-reuse, fresh two-locals, direct-assignment) were manually
written and measured one at a time via `sandbox _exeque --disable all`;
the choice to keep the direct-assignment form was made by the worker based
on the "fewest constructs" comparison, not because a search tool flagged
it.

## T5 family check
`s32 *p = &D_8009BE7C;` — pointer-alias family
([[pointer-alias-fake-exception]] lineage), but per
[[ordinary-c-judge-decidable]] Ruling 1(3) this specific construct has a
real semantic reading (read AND written through) and is NOT treated as a
FAKE-requiring alias under that ruling's rename test — carried unchanged
from s2, where it closed the entire final-callback block byte-exact. No
other construct in the diff matches any forbidden family (no register
pins, no `__asm__`, no dead stores/discards, no unused locals, no volatile
coercion — the `mask` local from s2 was REMOVED, not retained).

## T6 naming-announces-intent test
No construct is named `pad`, `dummy`, `unused`, `spill`, `_buf`, `tail`,
`slack`, or similar coercion-announcing names. `p` is a plain, minimal
pointer name (matches the s2-banked form, previously reviewed under this
same test in spirit); the s3 change removes the only locally-scoped
named-for-role variable (`mask`) that existed in the s2 form.

SANCTIONED-FAMILY-CLAIMS: none required as a FAKE-annotated family. The
`s32 *p = &D_8009BE7C;` pointer is carried from s2 unchanged and was
already justified there as ordinary C with a real semantic reading (read
+ write through the same pointer) rather than a FAKE-gated alias — see
[[ordinary-c-judge-decidable]] Ruling 1(3) ("a construct with a real
semantic reading ... is NEVER a cheat merely because the agent chose it
after observing register allocation"). If the Judge disagrees and treats
it as requiring the `pointer-alias-fake-exception` family instead, that
family's scope sentence is:
  SCOPE: "SANCTIONED 2026-07-01 (last-resort): C-level local pointer alias
  / typed re-view of a global, `/* FAKE */`-annotated; `asm(\"Sym\")`
  alias-RENAMES still forbidden." (.claude/rules/no-new-park-categories.md,
  § "2026-07-01 additions", pointer-alias-fake-exception bullet)
  PRECEDENT: .claude/rules/no-new-park-categories.md:213 (pointer-alias-fake-exception bullet, SOTN `tilemap = &g_Tilemap;` / `fakeEntity = self;` citations)
  — but no `/* FAKE */` annotation has been added, because this session's
  position (consistent with how s2 banked H5b) is that Ruling 1(3) governs
  this specific construct instead, since the pointer is genuinely read AND
  written (not a discard, not a bare hold). This is flagged explicitly so
  the Judge can rule on which lens applies; if the Judge requires the FAKE
  annotation under the family lens, that is a one-line fix, not evidence
  of a hidden cheat.

ANNOTATION-CONFORMANCE: n/a — no `/* FAKE */` construct in this diff. The
pointer alias is treated as ordinary C under [[ordinary-c-judge-decidable]]
Ruling 1(3) as explained above.
