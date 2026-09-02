# SELF-VET — func_80043DE0
CONSTRUCTS: none (ordinary C: a POLY_GT4 struct typedef mirroring the PsyQ LIBGPU.H layout, and a function body that is the quad counterpart of the already-matched func_80043D34 / func_80043C7C, differing only by the added `p->v3 += dv;` and the struct type)
## T1 semantic purpose: every statement stores a real value into the primitive (tpage, v0..v3, clut); no statement is byte-neutral-removable. PASS.
## T2 human-programmer: identical shape to the matched siblings func_80043BD0 / func_80043C7C / func_80043D34; a reader sees a texture-source shift on a POLY_GT4. PASS.
## T3 GCC-internals justification: none required; the body was written from the specification (sibling analog), not from any pass mechanism. PASS.
## T4 permuter/search provenance: hand-written from the sibling; no permuter run. PASS.
## T5 family check: no forbidden family matched; no sanctioned-family carve-out used. `(s16)du >> 6` etc. are ordinary sign-extension of 16-bit args exactly as in the siblings. PASS.
## T6 naming-announces-intent: names are tpage/clut/tx/ty/cx/cy/du/dv/dcx/dcy, POLY_GT4 fields follow LIBGPU.H. PASS.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
