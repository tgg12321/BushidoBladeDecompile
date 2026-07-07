# Evidence bank — func_800397A0

- Audit diagnosis (regressions.md): Redundant `(u8)val` cast where val is already declared u8 fails tests 1+2 (no semantic purpose, human programmer would write `if (val == 0x77)`); may also be a codegen coercion forcing `andi $v1,$a0,0xFF` + register split in target asm. Owner should verify: drop cast to `if (val == 0x77)` — if asm is identical the source has a benign artifact to clean up; if asm differs this is a confirmed codegen coercion requiring a proper re-match.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct at src/code6cac_c_mid.c:1468: `if ((u8)val == 0x77)` where val is declared `u8 val = D_800A36F8;` (line 1467).

- [s1] [fable-blitz 2026-07-07] COMPILER-SOURCE PROOF OF INERTNESS: tools/gcc-2.7.2/c-typeck.c:3656-3664 - build_c_cast's `if (type == TREE_TYPE (value))` branch does nothing for a same-type cast (only a pedantic RECORD/UNION warning), and the function returns `value` unchanged at :3785; the non_lvalue wrap at :3782-3783 requires -pedantic, absent from CC_FLAGS (Makefile:36-37). The typedef u8 = unsigned char reuses the same type node, so the pointer-equality holds. No NOP_EXPR/CONVERT_EXPR node exists downstream.

- [s1] [fable-blitz 2026-07-07] Therefore `if (val == 0x77)` is tree-identical to the committed form; the judge's 'benign artifact' branch is the true one; 'confirmed codegen coercion' is ruled out by construction.

- [s1] [fable-blitz 2026-07-07] The target `andi $v1, $a0, 0xFF` (asm/funcs/func_800397A0.s:4) is explained by the u8 LOCAL itself, not the cast: the QImode pseudo needs SImode zero-extension for the compare (andi into a SPLIT register $v1) while the raw loaded reg $a0 feeds `val + 1` in the bne delay slot (asm line 6: `addiu $v0, $a0, 0x1` = line 1472's store value). Keeping `u8 val` is what preserves the andi + register split.

- [s1] [fable-blitz 2026-07-07] Both if-arms map cleanly: then-arm sb zero/sb 1 (asm lines 7-9, src 1469-1470), else-arm sb val+1 (asm line 13, src 1472) - no other suspect constructs in the function.
