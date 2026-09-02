# SELF-VET — func_80017D84
CONSTRUCTS: none (ordinary C only: a TU-local `typedef struct { s32 v[8]; } ObjBlock;` used for one 32-byte struct assignment; a plain local `c` holding the loaded word that is stored once and reused in the next expression, exactly as the committed sibling obj_UpdatePosition already does; statement order chosen among semantically-equivalent orders)
## T1 semantic purpose: Every statement writes a distinct slot field or the block; the struct assignment copies 32 real bytes that the target copies (8 lw / 8 sw). `c` is read twice (store to +0xC and the +0x10 sum), so it carries a real consumed value. Nothing is byte-identical with-vs-without.
## T2 human-programmer: A programmer would write a struct copy for a 32-byte record and would keep the loaded value in a local before using it twice; the sibling obj_UpdatePosition in the same file is written the same way. Statement order is an ordinary choice among independent field initialisations.
## T3 GCC-internals justification: None needed. The struct copy is required by the program (32 bytes must be copied); the block-move expansion is simply how cc1 compiles struct assignment. The tail order was selected by measurement, but each order is equally natural C and the chosen one has no GCC-internal mechanism as its justification.
## T4 permuter/search provenance: The raw-cc1 sweep enumerated the 24 legal orders of 5 independent statements; the winner is plain field-initialisation order, not a detector-evading spelling.
## T5 family check: No forbidden family. No pins, no asm, no volatile, no dead stores, no dead locals, no pad arrays, no do-while wrap, no aliases. The typedef is not a splat-scalar aggregate merge (no D_x symbols merged; it types an anonymous 32-byte record reached through a pointer).
## T6 naming-announces-intent: Names are `p`, `i`, `c`, `ObjBlock` — no pad/dummy/unused/spill/tail/slack names.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
