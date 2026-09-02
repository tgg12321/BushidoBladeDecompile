# SELF-VET — func_80043D34
CONSTRUCTS: none (ordinary C: TU-local POLY_GT3 struct typedef, u16 local copies of two packed words, four once-written once-read s32 locals holding the new field values, mask/shift arithmetic, (s16) casts on s32 deltas, three byte += statements)
## T1 semantic purpose: every statement changes the primitive's bytes (tpage x/y fields, three v bytes, clut x/y fields); every local holds a value that is stored. No construct is byte-neutral.
## T2 human-programmer: a programmer updating packed tpage/clut fields by hand writes exactly a load, masked field arithmetic, and a repack; the (s16) casts express that du/dv/dcx are 16-bit texture-coordinate deltas whose high bits carry the page/clut step.
## T3 GCC-internals justification: none needed — the form is explained by the program logic; no pass is cited as the mechanism of any construct.
## T4 permuter/search provenance: no permuter or search was run; the form is the same-file sibling's matched spelling transferred by hand.
## T5 family check: no forbidden family matched; no pins, asm, volatile, dead stores, dead locals, arrays, or wrappers. The prior pin/__asm__ body was removed, not respelled.
## T6 naming-announces-intent: names are p, du, dv, dcx, dcy, tpage, clut, tx, ty, cx, cy — all real values used in stores.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
