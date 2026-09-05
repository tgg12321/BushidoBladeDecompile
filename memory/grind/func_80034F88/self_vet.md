# SELF-VET — func_80034F88

Session 67 (forensics dispatch; the frontier was a landing, not a probe — the
pipeline scope grant that s66 was blocked on is now present in
tools/grinder/scope_allow.txt). Bytes re-proven THIS session with the edits in
place: `sandbox func_80034F88 --disable all` = score 0, target_insns 49,
build_insns 49, rules_dropped 0; full clean-driver `verify-oracle` = ok true,
build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == the locked oracle.

CONSTRUCTS: (1) `u8 *q = &D_80106A73;` FAKE pointer alias carrying block 0's
read-modify-write; (2) `u8 *r = &D_80106A73;` FAKE pointer alias carrying flag
blocks 1 and 2, re-initialised once from the same direct `&D_80106A73`
initializer; (3) `u = 0;` FAKE dead store to a local between block 0's store and
its re-read; (4) FAKE variable reuse — the copy loop's counter is staged through
`q` after `q`'s pointer value is dead; (5) the honest aggregate declaration
`extern u8 D_80106A70[3];` at include/code6cac.h:472 replacing the three splat
per-byte scalars, with src/code6cac.c's two consumers converted to element form
and the surviving undefined_syms_auto.txt rows for D_80106A71/D_80106A72
suffixed per prong (c).

## T1 semantic purpose
(1) and (2) are the only handles through which the function reads and writes the
flag byte; every load and store in the target's bytes goes through them. They are
byte-materializing, not decorative. (3) `u = 0;` has no observable effect on the
function's output — it is a FAKE, declared as one, annotated as one, and it sits
inside the sanctioned dead-store family whose entire premise is that the
construct is semantically inert. (4) both values carried by `q` are real and
consumed: the pointer performs three memory operations, the counter drives the
three-iteration copy loop and indexes both the source read and the destination
store; the loop adds no instruction anywhere in the function. (5) the array
declaration is the honest object model — the target indexes D_80106A70 with a
computed register (`lui $at,%hi; addu $at,$at,$v1; sb $v0,%lo($at)`), which is
what an array is; the declaration is more truthful than the three scalars it
replaces, not less.

## T2 human-programmer
(1), (2), (5): yes. A programmer writing "set three flag bits in a byte, then
copy three bytes into a colour table" naturally writes a pointer to the byte and
an array subscript into the table. (3) and (4): no — a reader would ask why `u`
is zeroed and why the loop counter is spelled as a pointer. That is exactly why
both carry `/* FAKE: ... */` annotations naming the pass and the exhaustion
ledger, which is the disclosure the sanctioned families require. They are not
presented as ordinary program logic (banned_constructs entry 2 is about exactly
that framing, and this vet does not make that claim).

## T3 GCC-internals justification
Yes, and it is disclosed rather than hidden. (1)/(2): global.c:1275 assigns one
hard register per allocno and GCC 2.7.2 does no live-range splitting, so one
handle cannot occupy both $v1 and $a0; the Judge verified this claim itself and
granted EXACTLY TWO annotated alias objects (docs/grind/decisions.md:23437).
(3): cse2 (cse.c) forwards the `sb` into the following `lbu` while the stored
value's pseudo still holds it. (4): flow.c counts REG_N_REFS weighted by loop
depth, and global.c prices allocnos as floor_log2(nrefs)*nrefs*10000/live_length
(fitted exactly across three extracted models in s65); the merge lifts that
allocno from 5 refs / pri 3571 to 16 refs / pri 30476 with live length rising
only 14 -> 21, so it is seated in $v1 before block 0's value allocno (pri 17500)
is considered and find_reg's ascending scan sends the value to $a0 — the target
register. Every one of these is a named pass with a dump/model artifact behind
it, and each sits inside a family the owner has already sanctioned for
"whatever GCC pass it acts through" reasoning; naming the mechanism is the
annotation requirement, not the cheat signal.

## T4 permuter/search provenance
No permuter output is in this body. Every construct was derived from instrumented
cc1 ALLOCDBG/GREG dumps and tools/ra_solver models (s53-s66) and then spelled by
hand; the winning spelling z2 was one of four hand-written variants, and its two
near neighbours (z3 pointer-compared bound, z4 subscripted source read) measure 1
and 2, which is a mechanism the ledger explains rather than a detector artifact.

## T5 family check
(1)/(2) pointer-alias-fake-exception, with an explicit Judge PASS for this exact
two-object shape on this function. (3) dead-store-fake-exception (a local, not a
global). (4) variable reuse under defeat-licm-hoist-var-reuse with
staged-value-reused-variable's bounds: bound 1 both values real and consumed;
bound 2 `q` already exists for a real job (three byte-materializing memory
operations that predate the borrow by ten sessions) — it was not invented in
order to be borrowed, which is the quadrant bound 2 excludes; bound 3 the borrow
is provably safe (the pointer value is dead from `*q = c;` and is never read
again, and the counter is dead at function exit); bounds 4-6 annotation,
receipts, review. (5) the per-word-splat-to-aggregate merge entry at
.claude/rules/no-new-park-categories.md:238, all five prongs: (a) the object
model predates this grind — the naming census names the three bytes as a colour
-component record and the target itself indexes them with a computed register;
(b) a flat array is what the evidence shows (unit stride, one byte per element);
(c) the merge is complete — no C code names D_80106A71 or D_80106A72 anywhere,
and the two surviving undefined_syms_auto.txt rows carry the mandated
`/* alias of D_80106A70+N; retire with func_8001BE20 */` suffix for the
still-INCLUDE_ASM sibling func_8001BE20; (d) header-canonical, spelled once at
include/code6cac.h:472, never TU-local (the TU-local block-scope spelling also
measures 0 and was banked as INADMISSIBLE in
rejected/s66-blockscope-array-decl-score0-but-prong-d-tu-local.c precisely
because of this prong); (e) byte-neutral project-wide — proven by a full
clean-driver build SHA1 equal to the oracle, not by inspection.
No construct in this body is outside those four families.

## T6 naming-announces-intent
Names are `p`, `q`, `r`, `u`, `v`, `c` — neutral, as the Judge's own constraint
required ("with neutral names"). Nothing is named pad/dummy/unused/spill/slack.
No local is declared and left unused; no address-of-a-dead-object appears; no
`(void)` discard appears; there is no local array and no frame coercion.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: pointer-alias-fake-exception
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a C-level local pointer alias (second handle) to a global — `Type* t = &g_Thing;`, `s16 (*p)[] = &D_xxx;`, FakePrim-style reinterpret views — is allowed as a LAST-RESORT lever with `/* FAKE: ... */` annotation + lever-exhaustion."
  PRECEDENT: docs/grind/decisions.md:23437

  FAMILY: dead-store-fake-exception
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a dead store / self-assignment to a LOCAL or PARAMETER, annotated `/* FAKE: ... */`, is allowed as a LAST-RESORT codegen lever after documented lever-exhaustion."
  PRECEDENT: docs/reference/sotn-construct-index.md:65

  FAMILY: variable reuse (defeat-licm-hoist-var-reuse, bounded by staged-value-reused-variable)
  SCOPE: "When GCC hoists a loop-invariant (e.g. limit-1) that the target recomputes INLINE, reuse one C variable for a used loop-variant AND the invariant — multi-set pseudo isn't a loop.c movable, so it's not hoisted. Pure C, no asm."
  PRECEDENT: docs/reference/sotn-construct-index.md:92

  FAMILY: per-word splat symbol -> aggregate merge
  SCOPE: "two or more splat-invented `D_<addr>` scalars may be replaced by a single aggregate declaration."
  PRECEDENT: .claude/rules/no-new-park-categories.md:238

ANNOTATION-CONFORMANCE:
  /* FAKE: block 0's own address object (a second C handle on D_80106A73), mechanism: global.c allocation priority floor_log2(nrefs)*nrefs*10000/live_length -- blocks 1 and 2 cannot be reached from this handle because global.c:1275 assigns exactly one hard register per allocno and GCC 2.7.2 does no live-range splitting. lever-exhaustion: memory/grind/func_80034F88/hypotheses.md s53-s65. */
  /* FAKE: the address object for flag blocks 1 and 2, mechanism: global.c:1275 assigns exactly one hard register per allocno and GCC 2.7.2 does no live-range splitting, so blocks 1/2 cannot be reached from the block-0 object. lever-exhaustion: as above. */
  /* FAKE: cse2 value invalidator, mechanism: cse2 (cse.c) forwards the sb into the following lbu only while the stored value's pseudo still holds it. lever-exhaustion: hypotheses.md s57-s62. */
  /* FAKE: the copy loop's counter is staged through q, whose pointer value is dead from block 0's store above and is never read again, mechanism: flow.c counts REG_N_REFS per RTL insn weighted by loop depth, so the loop's eleven counter references lift this allocno from 5 refs / pri 3571 to 16 refs / pri 30476 and global.c seats it in $v1 before block 0's value allocno (pri 17500) is considered, which sends that value to $a0 as the target has it. Both values are real and used; the loop adds no instruction anywhere in the function. lever-exhaustion: hypotheses.md s53-s65 -- s65's branch (A), whose other byte-neutral spellings (duplicated store into arms, split reads, merged mask) are all banked dead. */
  Each carries what + mechanism (named GCC pass) + lever-exhaustion. The
  aggregate array declaration carries no FAKE and needs none: it is the honest
  object model, not a coercion.
