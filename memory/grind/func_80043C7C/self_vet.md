# SELF-VET — func_80043C7C
CONSTRUCTS: none (ordinary C only: a POLY_FT4 struct typedef, packed-field
mask/shift arithmetic, four `+=` byte updates, `(s16)` narrowing casts on
three params — identical shape to the already-matched sibling func_80043BD0)
## T1 semantic purpose: every statement stores a real value into the primitive
(tpage x/y fields, four vertex v bytes, clut x/y fields). No construct exists
whose removal leaves bytes unchanged.
## T2 human-programmer: this is the quad (POLY_FT4) counterpart of the
triangle (POLY_FT3) routine directly above it; a programmer would write
exactly the same body with one more vertex. The `(s16)` casts express the
packed-field unit conversion (64-px / 256-px page, 16-px clut units).
## T3 GCC-internals justification: none. No pass is named as a mechanism; the
form was derived from the sibling's semantics and the struct layout.
## T4 permuter/search provenance: none — no permuter run; hand-derived from
func_80043BD0 and the asm field offsets (0xD/0x15/0x1D/0x25 = v0..v3 of a
0x28-byte POLY_FT4).
## T5 family check: no forbidden family matched. No pins, asm, volatile,
dead locals, dead stores, barriers, aliases, or wrapping constructs.
## T6 naming-announces-intent: names are tpage/clut/tx/ty/cx/cy/du/dv/dcx/dcy
and the LIBGPU POLY_FT4 field names; none announce coercion.
SANCTIONED-FAMILY-CLAIMS: none
ANNOTATION-CONFORMANCE: n/a — no FAKE construct
