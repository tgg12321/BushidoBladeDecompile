# Hypothesis ledger — func_80043BD0

## s1 (2026-09-02, recon)
- H-OBJ (CONFIRMED): the object is a PsyQ `POLY_FT3` and the function shifts tpage/clut fields + v0/v1/v2. Evidence: offsets, 0x20 caller stride, field bit widths (4/1 tpage, 6/9 clut) = the tpage/clut packed formats.
- H-BITFIELD (KILLED, instance): a HImode bitfield-struct copy with two field inserts, in the h1/h2/h6 spellings on this chassis, does not reproduce the single-SImode-lhu extract path (shared zero_extend -> `andi 0xffff`); measured 39, 42, 34 (rtl dump insns 21/31).
- H-MASK (CONFIRMED): `u16` local copy + mask arithmetic with distinct once-written per-block locals for the new field values, `x | ((w & M) | (y << s))` pack, `(s16)` casts on s32 deltas, byte adds in v0,v1,v2 order -> sandbox 0 (43/43).
- H-S16PARAMS (KILLED, instance): `s16` params without casts -> 46 insns / 41.
- H-LEFTASSOC (KILLED, instance): left-assoc or-tree -> 13.
- H-SHAREDLOCALS (KILLED, instance): x/y reused across both blocks -> 38.
- H-BYTEORDER (KILLED, instance): v0,v2,v1 add order -> 4; v0,v1,v2 -> 0.
Frontier after s1: none for this function (matched). Transfer lead: siblings func_80043C7C / func_80043D34 / func_80043DE0.
