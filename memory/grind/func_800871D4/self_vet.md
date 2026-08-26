# SELF-VET — func_800871D4  (session 7, forensics, 2026-08-26)

NOTE ON STATUS: this session returns `ruling-request`, NOT candidate-ready.
The vet below is written to state the classification honestly and to make the
open question precise; it is not a self-approval.

CONSTRUCTS: two fresh locals `u16 okof1`, `u16 okof2`, each written once
(`okof1 = D_801078D8 | bitsLower;`) and read once (`D_801078D8 = okof1;`),
staging the two key-off words' new values across the two voice-slot halfword
clears. Nothing else: no mask, no cast for effect, no dead store, no volatile,
no inline asm, no annotation, no reordering of any global read-modify-write
into phases beyond what those two locals express. Sony's key-on lines are
verbatim (`D_800F1B10 &= ~D_801078D8;`).

## T1 semantic purpose
Yes — and this is the fact that distinguishes this form from the s6 form the
2026-08-26 04:54 layer-1 review FAILed. The locals change the function's
observable memory-access ORDER: without them the two key-off loads cannot sit
above the two voice-slot halfword clears, because the load and the store of a
`|=` statement are emitted adjacent and sched.c refuses to move a bare-symbol
MEM across a `(plus (reg) (symbol_ref))` store (dependence lists in
tmp/grind/func_800871D4/s4/sched_direct.txt: insns 60/65/70 ref_count 8, insn
73 depends on all three). The simpler form — the one the layer-1 review asked
to be measured — is measured: score 8. So the construct is not byte-neutral
noise around a simpler equivalent; it is the only C spelling of the target's
memory order.

## T2 human-programmer
Yes. "Compute the new pending-key-off masks, release the voice slot, then
commit the masks and clear the matching key-on bits" is an ordinary thing to
write, and it is one statement-pair's distance from Sony's own published
shape. A reader asking "why is `okof1` here?" gets a program answer (the new
mask is computed before the slot is released), not a compiler answer.

## T3 GCC-internals justification
The GCC-internals material in this session is used as EVIDENCE OF
IMPOSSIBILITY for the simpler form (sched.c's alias-driven dependences), and
as an explanation of where the target's `nor` comes from (cse.c store-to-load
forwarding). It is NOT the mechanism by which the construct is claimed to
work: no allocator, priority, live-length, or n_refs reasoning appears in this
form's justification — which is precisely what the s6 form did and was FAILed
for. That distinction is the substance of the ruling question; a reviewer who
judges it insufficient is entitled to, and this session does not self-approve.

## T4 permuter/search provenance
No. The permuter campaign on this function is a banked negative (s4: ~2550
iterations, best 40, never approached 0). This form came from reading the
target's memory order, measuring the layer-1 review's requested remedy, and
reading the cc1 dumps.

## T5 family check
This is the open question, and it is why the outcome is `ruling-request`.
Candidate readings:
 (a) Ordinary C, no family — a local holding a value the target demonstrably
     holds in a register, once written, once read.
 (b) named-intermediate (`.claude/rules/narrow-byte-args-packed-call.md` +
     the 2026-08-17 clarification in `.claude/rules/no-new-park-categories.md`)
     — fresh, once-written / once-read, real consumed value present in the
     target's bytes — which would require a /* FAKE */ annotation and the
     exhaustion ledger.
This session does NOT assert (a). Per the role prompt ("if you cannot quote a
scope sentence and cite a precedent for a family you are claiming, you do not
have that family, and the correct outcome is ruling-request"), it asks.

## T6 naming-announces-intent
`okof1`/`okof2` name Sony's `_svm_okof1`/`_svm_okof2` (the pending key-off
masks) — the values they hold. No `pad`/`dummy`/`tmp`/`spill` naming, and both
are consumed.

SANCTIONED-FAMILY-CLAIMS: none asserted (see T5 — this is the ruling request).

ANNOTATION-CONFORMANCE: n/a — no /* FAKE */ construct is emitted, because no
family is being claimed. If the ruling is that reading (b) applies, the fix is
one annotation line plus the exhaustion citation, not a new form.
