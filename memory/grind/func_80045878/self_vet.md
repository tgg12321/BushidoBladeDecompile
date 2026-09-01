# SELF-VET — func_80045878  (s14, 2026-08-31, solver modality)

The diff against HEAD is: the `INCLUDE_ASM("asm/funcs", func_80045878);` line in
src/text1a_c.c replaced by the body of memory/grind/func_80045878/candidate.c.
Nothing else in the tree is touched.

MEASURED WITH THAT DIFF IN PLACE:
  `sandbox func_80045878 --disable all` -> score 0, build_insns 108 ==
     target_insns 108, scorable true, rules_dropped 0 (measured twice, the
     second time after a full oracle rebuild).
  `verify-oracle` -> "ok": true, "build_matches": true (full clean-driver
     build+link SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa).

CONSTRUCTS: none

There is no carrier local, no fresh local of any kind, no dead store, no dead
mention, no alias, no duplicated statement, no volatile, no inline asm, no
annotation.  The body contains only live statements whose values are all read.
Two things about it are nevertheless DECLARED here explicitly, because they are
what changed relative to every previous submission on this function, and the
reviewer should judge them rather than have them buried:

  (D1) The function is declared `s16 *func_80045878(s32 a0, s32 a1, s32 a2)`
       and ends with `return s1;`.  Previous sessions used `void`.
  (D2) The six tail stores are written in a particular source order: the three
       `a0` half-word stores, then `s1[11] = a0 + 3;`, then the 0x8000 store.

## T1 semantic purpose
D1: YES, a real observable effect. The function's whole job is "look up the
object for slot a0, create+register it if absent, initialise it" -- and the
returned pointer is the object. It is consumed by a caller that is ALREADY
MATCHED in this project: src/text1a_pre.c:173 declares
`extern s32 *func_80045878(s32);` and src/text1a_pre.c:179-186 does
`ptr = func_80045878(a0); g_player_ptrs[a0] = (s32)ptr; func_80040594(ptr);
 ... return ptr;`. The value is also observable in the TARGET's own bytes: the
object pointer sits in $v0 at `jr $ra` (`addu $v0,$s1,$zero` at .L800459DC,
never clobbered afterwards). Declaring the function `void` was the defect; the
return value is the function's output.
D2: statement order has no "presence" to justify -- every one of the six stores
is a required initialisation store present in target's bytes. No statement was
added, removed, duplicated or made dead to obtain the order.

## T2 human-programmer test
D1: a human given the specification ("get or create the per-slot object,
initialise it") writes exactly this: it returns the object, and its caller uses
it. Nobody reading the body asks "why is this here?" about `return s1;` -- the
opposite is true, a `void` version makes the already-matched caller
`func_80040510` nonsensical.
D2: a human writes the initialisation stores in some order; this is one of the
orders a human writes. There is no marker, no comment, no extra token that
exists to steer the compiler.

## T3 GCC-internals justification
The construct inventory is empty, so nothing in the diff is justified by a GCC
pass. Ledger prose DOES explain WHY the byte-exact form was previously missed
(the target insn is the return-value move, not a manufactured base copy, and it
carries the local-alloc copy suggestion for $v0). That is an explanation of a
past error, not a mechanism a construct depends on: delete the explanation and
the C is unchanged and still ordinary. Conversely, EVERY previously submitted
form on this function DID depend on a named pass (cse.c:836-864 canonicality +
flow.c life_analysis deletion) -- that dependency is exactly what is gone.

## T4 permuter/search provenance
Not permuter output. No permuter ran this session. The form came from reading
the inverse solver's single minimal vector for the block-13 seat
(`inverse.py local ... --goal {"0":3,"1":3}` -> one atom: "hard $v0 live across
[6,20) -- a real argument/return value genuinely live across that span"), then
asking what ordinary C produces a live $v0 there -- a return value -- and then
finding that an already-matched caller independently documents that return
value. The construct is not "necessary only because a search found it"; it is
independently attested by src/text1a_pre.c and by target's own register state
at `jr $ra`.

## T5 family check
No forbidden family is touched: no register pin, no `__asm__`, no scheduling
barrier, no volatile, no alias rename, no dead store, no dead local, no dead
param assign, no unused array, no `if (1)`, no `do{}while(0)`, no goto pad, no
DImode chain, no build-time asm rewriting, no linker-script reorder. Nor is any
SANCTIONED family used -- there is nothing to route. The nearest thing to a
"technique" in the diff is a function returning a value, which is C.
The two standing per-function judge_constraints (no fresh local as an
allocation carrier / duplicate handle, by any spelling, write-count or name;
no dead store reading an unassigned local) are satisfied VACUOUSLY: the diff
contains no fresh local and no dead store. The banned `p = s1;` and
`s0 = (s32) p;` are both absent -- not respelled, absent.

## T6 naming-announces-intent
No new identifier is introduced by the diff at all. The locals are the
long-standing `s3 / v0 / s1 / s0` (m2c-derived names carried by this function
since s1), every one of which holds a real value that is read. There is no
`pad`, `dummy`, `tmp`, `spill`, `_buf`, or similarly-announcing name.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct

## Disclosures the reviewer should weigh (stated, not argued away)
1. The signature change is a PROTOTYPE CONTRADICTION with two stale
   declarations elsewhere in the tree, and this session deliberately did NOT
   edit either of them (both are out of this session's scope and touching them
   could perturb an already-matched function):
     - src/text1a_pre.c:173 `extern s32 *func_80045878(s32);` -- right return
       type, wrong arity (one arg). Separate TU, so no diagnostic; the oracle
       still matches (verified).
     - include/m2c_context.h:624 `void func_80045878(s32,s32,s32);` -- stale
       void declaration in the m2c scaffolding header, which no src/*.c
       includes (grepped).
   Recommended follow-up for whoever integrates: reconcile both to
   `s16 *func_80045878(s32, s32, s32);` in a separate, separately-verified
   change.
2. The tail SOURCE ORDER is load-bearing for the bytes (a0+3 store written
   first instead of last costs one insn: 109 / score 6, banked as
   rejected/s14-return-sig-a0plus3-store-first-109-score6.c). Source order of
   independent stores is an ordinary authoring choice, not a construct, but it
   is disclosed so that nobody "cleans up" the order later and silently
   un-matches the function.
