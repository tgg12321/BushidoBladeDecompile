# SELF-VET — func_800238C4

CONSTRUCTS: one — the common-tail statement `*(s32 *)(arg0 + 0x74) = *(s32 *)(arg0 + 0xBC);`
together with its control transfer (`goto skip_74;`) written into the first arm of the
kind-dispatch chain instead of that arm falling through to the shared copy that follows the
chain. No other construct is added, removed or respelled: the rest of the body is the s1
candidate verbatim.

## T1 semantic purpose
The duplicated statement is REAL on the path it sits on. Before the edit, the first arm fell
out of the if/else chain into `*(s32 *)(arg0 + 0x74) = *(s32 *)(arg0 + 0xBC);` and then into
`skip_74:`; after the edit it performs that same store itself and jumps to `skip_74:`. The
store is executed exactly once on that path in both spellings, with the same operands, and no
other path's behaviour changes. This is a control-flow restructure of a live statement, not a
no-effect construct: delete it from the arm and the arm no longer writes 0x74, which is a
behavioural change. (What is byte-NEUTRAL is the choice between the duplicated and the
label-shared spelling — jump2 cross-jump re-merges the two copies, so both spellings emit the
same 219 instructions. That is prerequisite 2 of the family, not an absence of semantics.)

## T2 human-programmer
Yes. Writing a tail statement out in an arm that also needs it, rather than falling through a
long else-if chain to a shared copy, is ordinary defensive C and is exactly how the
neighbouring 0x11 arm in this same function is already written (it does its own work and
`goto skip_74;`). A reader asks nothing about why the store is there: the arm needs the store.

## T3 GCC-internals justification
The MECHANISM by which the spelling reaches the target bytes is a GCC internal and is stated
plainly in the FAKE annotation (local-alloc `block_alloc`'s hand-rolled quantity sort,
tools/gcc-2.7.2/local-alloc.c:1539-1563, plus `qty_compare` at :1640). That is precisely what
the family's prerequisite 4 requires to be disclosed. It does not make the construct a cheat,
because the construct is a real statement on a real path (T1) that a programmer would write
(T2); the GCC reasoning explains WHICH of two semantically identical spellings the original
author used, which is the reconstruction argument the family rests on. Per the family rule,
the register-allocation effect must be documented, not hidden.

## T4 permuter/search provenance
No permuter, no automated search. The construct was derived from a measured model: the
instrumented cc1 (BB2_SUGG_DEBUG / BB2_QTY_DEBUG, tools/gcc-2.7.2/cc1) printed the block's
quantity table, the priority formula in local-alloc.c:1640 was read against it, and a
standalone 4-line reproduction (tmp/grind/func_800238C4/s2/probe/) showed that adding a third
quantity AFTER the store flips the seat. The C form was then written to satisfy that condition
with a statement the function already had.

## T5 family check
The construct sits in the duplicated-statement-into-arms family, in its 2026-08-06
clarified scope for control-transfer tails. All five prerequisites are met: (1) the statement is real on its path, not a dead
store; (2) byte-neutrality verified — `sandbox func_800238C4 --disable all` = 0 with
219 target / 219 build instructions, and a full build gives SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle; (3) lever-exhaustion is in the ledger
(hypotheses.md K0-K3 from s1 plus the s2 quantity-arithmetic derivation that shows the
two-quantity block can never seat the pointer first); (4) the `/* FAKE: ... */` annotation is
present on the duplicated copy; (5) this vet is layer 0, layer 1/2 follow. It is not a dead
store, not a duplicated CALL, and it does not survive to the final bytes (cross-jump merges
it), so the family's non-extension clause is not engaged.

## T6 naming-announces-intent
No new names of any kind are introduced — no locals, no aliases, no pads. The diff adds one
existing statement and one `goto` to an already-existing label.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: duplicated-statement-into-arms
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): duplicating a REAL statement into 2+ arms (instead of label-sharing) is legitimate — incl. when cross-jump re-merges the copies to identical bytes and the effect is a reg_n_refs priority lift. SOTN duplicates assignments across arms routinely (7-arm, 11-arm instances). Prerequisites: byte-neutrality verified, lever-exhaustion, FAKE annotation when match-motivated."
  PRECEDENT: .claude/rules/duplicated-statement-into-arms.md:80

ANNOTATION-CONFORMANCE: the duplicated copy carries, verbatim in src/code6cac.c:
  /* FAKE: the common tail `0x74 = 0xBC` + its control transfer duplicated into
   * this arm instead of falling through to the shared copy below,
   * mechanism: local-alloc block_alloc's hand-rolled quantity sort
   * (tools/gcc-2.7.2/local-alloc.c:1539-1563). ...
   * lever-exhaustion: memory/grind/func_800238C4/hypotheses.md K0-K3 + the s2
   * quantity-arithmetic derivation in evidence.md. */
which carries all three required parts: WHAT (the duplicated tail + transfer), MECHANISM (the
named GCC pass and the exact source location of the sort that the duplication perturbs), and
LEVER-EXHAUSTION (the ledger pointer).
