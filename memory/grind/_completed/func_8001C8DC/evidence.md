# func_8001C8DC — evidence (manual session 2026-09-23)

- First draft from asm/funcs/func_8001C8DC.s: sandbox 32/291. Two source-level gaps:
  (1) case 0's `--D_800A3680` test must read `if (--x == 0) {...} else { func_8001C51C(); func_8001C820(); }`
  (the else arm is laid out last); (2) the `D_800A37D2[D_80101F5E != 0]++` index formation.
- Index spelling sweep (tmp/c8dc/{a,b,c,d,e}.c, on the swapped-branch chassis):
  direct `(&D_800A37D2)[D_80101F5E != 0]++` 19; `t = D_80101F5E;` before the if 19;
  `p = &D_800A37D2 + (t != 0); (*p)++` 6; `(t = D_80101F5E) == 0 ... (&D_800A37D2)[t != 0]++` 17;
  **`(t = D_80101F5E) == 0 ... p = &D_800A37D2; p[t != 0]++` 2** (candidate.c).
- Residual sandbox 2 = the single operand-only hunk `lw v0,0(at)` vs `lw v0,124(at)`: the jtbl `%lo`
  addend, a link-geometry value the sandbox cannot produce (same as func_800747D8 s10). Floor is not a
  C-level gap.
- Full build: with just the body spliced (INCLUDE_RODATA jtbl_800100C4 + INCLUDE_ASM removed) the EXE was
  4 bytes short. The rodata file had 8 words but the dispatch is `sltiu 0x7`, so the 8th word
  (0x00000000 @ 0x800100E0) is not part of the table. Supplied as
  `const u32 D_800100E0[1] = { 0x00000000 };` right after the function, mirroring the func_800747D8
  precedent (src/text1b.c `D_80015A20`). With it, verify-oracle --rebuild = 62efab4f… (oracle).
