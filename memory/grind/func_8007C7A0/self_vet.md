# SELF-VET — func_8007C7A0

CONSTRUCTS: (1) named clamp intermediates `s16 x = arg0; s16 tx;` with three-arm
assignment into tx and write-back `x = tx;`, (2) param clamping written into
`arg1` (live reassignment), (3) per-arm returns in the dispatch (each arm ends
in its own `return`), (4) per-arm block-scoped locals `s32 hi; s32 lo;`,
(5) def-then-modify statement style (`s32 hi = arg1 & 0xFFF; hi = hi << 12;`
and `lo = x & 0xFFF; lo = lo | 0xE3000000;`), (6) inline dispatch expression
`if ((u32)(D_8009BE74 - 1) < 2U)`.

## T1 semantic purpose
Every construct computes a value that is subsequently read and materializes in
the emitted output. (1) tx is the clamp join value, read by `x = tx`; x is read
by both arms' `x & MASK`. (2) arg1's clamped value is read by both arms'
`arg1 & MASK`. (3) each return arm computes and returns the packed GP0 word for
its bit layout. (4)-(5) hi/lo hold the y-field and x-field of the packet; every
statement maps 1:1 to an emitted instruction (andi / sll / andi / or / or —
verified against the emitted stream, 51/51). (6) the range check dispatches on
the video-mode global. ZERO dead stores, zero unused declarations, zero UB
(both params are fully initialized before every read; block-locals are
init-before-use in their own arms). PASS for all constructs.

## T2 human-programmer
The body reads as a textbook GP0 drawing-area packet builder: clamp x to
[0, xlim-1], clamp y to [0, ylim-1], then pack at 12/12 or 10/10 bits depending
on mode, or-ing in the 0xE3000000 command. Per-arm returns are exactly how the
published Sony reference for this routine family is written (SOTN's get_cs is
per-arm returns of full packed expressions). Names are semantic (x, tx =
temporary x, hi/lo = high/low packet fields). No construct would prompt "why is
this here?". PASS.

## T3 GCC-internals justification
The constructs are justified by program logic (T1/T2 above); none exists solely
to steer a GCC pass with no output effect. The SESSION'S DERIVATION used
instrumented-cc1 dumps to understand why prior shared-tail chassis could not
match (documented in evidence.md s6), but the resulting source contains no
coercion construct: no dead code, no pads, no reuse-for-allocation levers (the
prior candidate's permuter-found L1 tx-dispatch-reuse and L2 hi-staging levers
were REMOVED, not respelled — the final form is strictly more natural than the
banked 12-form). The def-then-modify statement style (5) corresponds to real
separate emitted instructions (the or against a lui-materialized constant
cannot fold into the andi), so it is not a fold-back-into-one-insn refs trick.
PASS.

## T4 permuter/search provenance
The closing form was NOT produced by auto-search. It was derived this session
from ground-truth dumps (local-alloc QTYDBG + global.c conflict-walk source
reading) plus the Sony reference's per-arm-return shape, and verified by
sandbox measurement. The two levers that WERE permuter-found in s4 (L1/L2) are
no longer present in the body. PASS.

## T5 family check
Checked against the full forbidden-family catalog: no register pins, no
__asm__, no volatile, no alias renames, no dead stores/params/conditionals, no
unused arrays/scalars, no do-while(0)/always-true wrappers, no goto/label
constructs, no width casts beyond the semantically required `(u32)` in the
unsigned range-check idiom (present in every prior banked form and in the
target's sltiu). Constructs (1)-(6) fall inside ordinary C plus the two
sanctioned families claimed below. PASS.

## T6 naming-announces-intent
Names: x, tx, hi, lo, arg0, arg1 — all semantic, none from the pad/dummy/
spill/slack catalog, no address-of or discard-only uses. PASS.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: split-init accumulation (same-variable def-then-modify)
  SCOPE: "This sanctions the specific shape above (split a real `a+b` into init + `+=` on the same var, combine folds it back)."
  PRECEDENT: ad11a8c8
  (Applied conservatively to constructs (5): `hi = arg1 & 0xFFF; hi = hi << 12;`
  and `lo = x & 0xFFF; lo = lo | 0xE3000000;`. Note these are WEAKER than the
  sanctioned shape's edge case: combine does not even need to fold them — each
  statement maps to its own emitted target instruction, so they are ordinary
  statement-per-instruction C; the family is claimed out of caution.)

  FAMILY: named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:189
  (Applied to construct (1): the tx/x named-intermediate pair in the X clamp,
  carried over unchanged from every banked floor form since round 16.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. No construct in the diff
requires or carries a /* FAKE */ annotation; all code is live and semantically
motivated.
