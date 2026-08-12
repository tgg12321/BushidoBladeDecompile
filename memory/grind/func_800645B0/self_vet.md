# SELF-VET — func_800645B0

STATUS: NOT a candidate-ready session.  The session-6 (forensics) grind returned
`progress` at honest distance 1, so this file carries no submission.

The vet that previously stood here covered a body that the layer-1 reviewer
FAILed.  That body has been reverted out of src/ and out of candidate.c and is
banked under rejected/; the reasoning that supported it is superseded and must
not be reused.

CONSTRUCTS: none submitted this session.

The body now in src/text1b.c and in candidate.c is the session-3/4 form whose
four constructs the session-4 layer-1 reviewer ruled legitimate:
  1. one scratch variable carries the constant 1 and later the occupancy read
     and the OR result;
  2. that scratch is the OR destination, matching the in-place `or` in target;
  3. the first and fourth library calls are named into a temp;
  4. the slot index variable later carries the derived word index.
Every one of those assignments is read afterwards; none is dead.

SANCTIONED-FAMILY-CLAIMS: none (no new family is claimed by a `progress` outcome).
ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
