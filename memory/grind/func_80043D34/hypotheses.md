# Hypothesis ledger — func_80043D34

## s1 (2026-09-02, recon)
- H-OBJ (CONFIRMED): the object is a PsyQ `POLY_GT3` (0x28 bytes; caller stride 0x28 at func_80043454.s:182,198) and the function shifts tpage/clut fields + v0/v1/v2 exactly like func_80043BD0 does for POLY_FT3.
- H-TRANSFER (CONFIRMED): the func_80043BD0 s1 mask-spelling form, re-typed onto POLY_GT3, compiles to the target on the first measurement — sandbox --disable all = 0 (43/43).
