# SELF-VET — func_80075670

CONSTRUCTS: three pointer locals holding D_800A36A0 (`base` top-half, `work` bottom-half, `q` post-loop block); single-statement read+write through `q`; removal of pre-existing dead `p = p;` and empty `;` statements. No inline asm, no volatile, no dead stores, no unused locals, no FAKE constructs.

## T1 semantic purpose: PASS per construct. `base`, `work`, and `q` are each assigned and then dereferenced multiple times to compute addresses of real loads/stores that appear in the output; every store in the function is a live game-state write. The `p = p;` REMOVAL deletes a construct that had no semantic purpose (net cheat-debt reduction). No construct in the diff is byte-inert filler; deleting any of the three locals' uses changes the emitted code's structure (they carry the address computations).
## T2 human-programmer: PASS. Separate named pointers for separate regions of work over the same global block (`base` for the early-out/input check, `work` for the state-table writes, `q` for the "other player's counter" update) is ordinary C a programmer writes from the spec. No reader-puzzling constructs remain; the diff REMOVED the puzzling ones (`p = p;`, bare `;`).
## T3 GCC-internals justification: the constructs are justified by program logic (each pointer is used for its region's accesses); the ANALYSIS of why this split matches bytes references global.c allocation order, as all matching work does. No construct exists whose ONLY explanation is a GCC internal: every local is a live, multiply-read variable, not a coercion artifact (no dead store, no pin, no barrier, no wrapper).
## T4 permuter/search provenance: none. No permuter or auto-search was run; the form was derived by hand from the target-vs-build objdump diff and confirmed by two sandbox measurements (20 → 4 → 0).
## T5 family check: no forbidden family matches. No register pins, no asm, no volatile coercion, no alias renames, no dead stores/self-assigns (one was REMOVED), no unused locals/arrays, no scheduling barriers, no do-while(0), no goto tricks. Variable structure/naming of live locals is the ordinary pure-C lever, within the frozen-list "variable reuse for codegen control / named-intermediate declaration order" families and requiring no exception because nothing here is dead or semantic-free.
## T6 naming-announces-intent: PASS. Names are `base`, `work`, `q`, `p`, `i` — descriptive locals, no `pad`/`dummy`/`unused`/`spill`-class names; all are used (read) after assignment.

SANCTIONED-FAMILY-CLAIMS: none — no exception is claimed; the diff contains only live, semantically-used plain C (no FAKE annotation required anywhere).

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
