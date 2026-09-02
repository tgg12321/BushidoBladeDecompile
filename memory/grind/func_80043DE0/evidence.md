# Evidence bank — func_80043DE0

## s1 (2026-09-02, recon)
- Target: 46 insns, leaf, no stack frame. Reads u16 at +0x1A (tpage), u8 at +0xD/+0x19/+0x25/+0x31 (v0..v3), u16 at +0xE (clut); 5th arg from 0x10($sp). Layout == PsyQ LIBGPU.H POLY_GT4 (0x34 bytes).
- HEAD body on main was a legacy register-pinned cheat body (pins on $8/$9/$2/$3 + an `lw $5,16($29)` asm block); the sandbox stripped 20 cheat-asm lines from the file, so the pre-session honest floor was undefined (queue quoted 37).
- Pure-C form = exact quad counterpart of matched func_80043D34 (POLY_GT3) with `p->v3 += dv;` added and a POLY_GT4 typedef: `canonical` = C (distance 0), `sandbox --disable all` = score 0, 46/46 insns. No FAKE constructs.
- Sibling matched precedents in the same TU: func_80043BD0 (POLY_FT3, commit 84b91c08), func_80043C7C (POLY_FT4, ad8c4f47), func_80043D34 (POLY_GT3, 178e8f28).
