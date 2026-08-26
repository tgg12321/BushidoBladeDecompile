# SELF-VET — func_800871D4 (session 5, synthesis, 2026-08-26)

Diff under vet: `src/main.c` — the `INCLUDE_ASM("asm/funcs", func_800871D4);`
line is replaced by the body in `memory/grind/func_800871D4/candidate.c`.
Nothing else in the tree is touched. sandbox --disable all = 0 (52/52 insns,
rules_dropped 0); verify-oracle ok=true, build_sha1 ==
62efab4f73f992798c43e8c730aa43baa10bb4fa, build_matches true.

CONSTRUCTS: (1) `u16 voice` local typed to match the u16 global it is loaded
from; (2) `s32 idx = voice * 54` plus three byte-offset stores through the
splat-split `_svm_voice[]` symbols; (3) four `u16` locals (`keyoff_lo`,
`keyoff_hi`, `keyon_lo`, `keyon_hi`) holding the four global words that the
function read-modify-writes, with the two halves of the 24-voice bitmask
handled in parallel (both reads, both masks applied, both stores, both key-on
updates) rather than one whole group at a time.

Explicitly NOT present, and deliberately removed relative to the session-4
form that layer 1 FAILed: any `& 0xFFFF` on a u16-typed value; any dual-use of
a raw load kept alive to defeat combine.c; any `/* FAKE */` construct; any
register pin, `__asm__`, volatile, dead store, dead local, constant holder,
pointer alias, `do { } while (0)`, or barrier.

## T1 semantic purpose
(1) `u16 voice` — yes. `D_8010280A` is declared `u16` and holds an SPU voice
number in 0..23; the local's type states that fact. The function's behaviour
depends on it: `voice < 16`, `1 << voice`, `1 << (voice - 16)` and the array
index are all unsigned 16-bit reads. Deleting the type (making it `s32`)
changes nothing semantically but is also not "simpler" — it is less accurate.
(2) `idx` — yes; it is the byte offset of `_svm_voice[voice]`, consumed by
three stores. `54` is the measured BB2 SpuVoice stride.
(3) the four `u16` locals — yes; each holds a value that is loaded from a
global, combined, and stored back. Every one is written and read; removing any
of them removes a real load or a real store. `keyoff_lo`/`keyoff_hi` are the
new key-off masks and are *also* the operands of the key-on clear, exactly as
Sony's `_svm_okon1 &= ~_svm_okof1` does.
No construct in the diff is byte-identical-with-or-without: dropping any of
them changes the emitted instructions, not just their order.

## T2 human-programmer
Yes for all three. The strongest available evidence is that Sony's own
programmer wrote (1) and the surrounding shape: psyz's PsyQ-4.0 decomp of
LIBSND/VM_NOWOF declares `int bitsUpper; int bitsLower; u16 voice;` and the
same if/else mask split, the same three field clears in the same order, and
the same four global updates
(`tmp/psyq_prov/psyz/decomp/src/libsnd/vm_nowof.c`). This body is that
function with BB2's symbol names, BB2's 54-byte voice stride, and the two
bitmask halves advanced in lockstep. (3) is the ordinary way to write "apply
this key-off to both halves of a 24-voice mask": read both, update both, store
both. A reader asking "why is this here?" of `keyon_lo` gets the answer "it is
the old value of the key-on mask for voices 0-15" — a semantic answer, not a
codegen one.

## T3 GCC-internals justification
The C is justified by the routine's specification, not by a pass. GCC
internals appear in my ledger only as the *explanation of a measurement*, and
in two places they explain why a spelling is FORCED rather than chosen:
- the two `andi`s are PROMOTE_MODE zero-extensions of a `u16` local. I did not
  add anything to obtain them; I removed the s4 mask and they remained. The
  mechanism explains an ordinary declaration; it is not the reason the
  declaration exists (Sony's source has it).
- the parallel handling of the two halves is forced by a measured alias
  property (GCC 2.7.2 will not move a memory reference across a store whose
  address is `(plus reg symbol_ref)`) and confirmed by an allocno-priority
  computation. This is the "why did the other four spellings measure 6"
  narrative in the ledger, not a construct invented to poke a pass. There is
  no lever-named construct here: no reference is added, no store is
  duplicated, no variable is borrowed, no live range is padded.

## T4 permuter/search provenance
None. No permuter was run this session (s4's campaign is banked as a negative
datum). Every form measured was hand-written from the psyz source, and the
final form is the psyz body with one statement-ordering change whose necessity
was derived from two dump-verified mechanisms and confirmed by four negative
measurements. It is not "search output that happens to pass detectors".

## T5 family check
No forbidden family matches, and no sanctioned family is being claimed:
- redundant width casts (F2) — the exact family layer 1 FAILed s4 on — is
  GONE. There is no mask anywhere in the diff.
- dead-store / dead-local / constant-holder / dead array / pad: nothing is
  written-and-not-read.
- variable-reuse-for-codegen-control: no local carries a second unrelated
  value. `keyoff_lo |= bitsLower;` is a read-modify-write of the variable's
  own value, which is what the name says it is.
- named-intermediate (the 6-prong family): does not apply. Those locals are
  not once-written-once-read fresh temporaries introduced to place a value;
  each is written twice (load, then update) and read twice, and each is a
  named copy of a named global with a real update applied to it.
- volatile / MMIO / alias-rename / asm / barrier / do-while(0) / bitfield pun:
  none present.
- aggregate-merge: NOT claimed. The three `_svm_voice[]` field stores are
  spelled with the tree's existing per-word splat symbols and explicit byte
  offsets, exactly as the surrounding BB2 C does; no struct type is declared
  and no header is changed.

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `tmp`, `_buf`,
`slack` or similar. `voice`, `bitsLower`, `bitsUpper` are Sony's own names
from vm_nowof.c; `idx`, `keyoff_lo/hi`, `keyon_lo/hi` name the quantities they
hold (the `_svm_okof*` / `_svm_okon*` words). Every one has at least one real
read AND one real write; none is address-taken, discarded, or `(void)`-cast.

SANCTIONED-FAMILY-CLAIMS: none — the diff is ordinary C (typed locals, an
index computation, statement ordering) and claims no coercion family.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.
