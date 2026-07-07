# Evidence bank — func_800392C8

- Audit diagnosis (regressions.md): Comma-expression cheat `j -= (0, 0x10)` in loop2: the left operand `0` is a constant with no side effects and is discarded by the C comma operator, making this semantically identical to `j -= 0x10`. Fails Test 1 (no semantic purpose — runtime behavior is byte-identical without the `0`), Test 2 (no human writes `(0, 0x10)` from a spec), and Test 3 (only justification is GCC-internal COMPOUND_EXPR tree coercion). Worker should: (1) confirm `j -= 0x10` produces identical asm — if yes, replace `(0, 0x10)` with `0x10` and re-verify oracle; (2) if `j -= 0x10` produces wrong asm, the comma expression is a genuine codegen coercion and a clean pure-C structural alternative must be found (e.g. restructuring the loop, adjusting variable types, or using a named intermediate) before this function can be COMPLETED-C.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct at src/code6cac_c_mid.c:1235: `j -= (0, 0x10);` in loop2 of func_800392C8 (lines 1233-1236).

- [s1] [fable-blitz 2026-07-07] COMPILER-SOURCE PROOF OF INERTNESS: tools/gcc-2.7.2/c-typeck.c:3599-3612 - internal_build_compound_expr checks `if (! TREE_SIDE_EFFECTS (TREE_VALUE (list)))` and `if (! pedantic) return rest;`. The literal `0` has no side effects, so the parser returns the tree for `0x10` directly; NO COMPOUND_EXPR node is ever built. The judge's test-3 concern (COMPOUND_EXPR tree coercion) cannot apply - the node does not exist downstream of the parser.

- [s1] [fable-blitz 2026-07-07] Build flags contain no -pedantic: Makefile:36-37 (CC_FLAGS/-O2 -G0 ... -w, CC_FLAGS_GP/-G8 variant) - the `! pedantic` early-return path is taken unconditionally in this build.

- [s1] [fable-blitz 2026-07-07] Therefore `j -= (0, 0x10)` and `j -= 0x10` produce byte-identical trees, RTL, and asm - the judge's option (1) branch is the true one; option (2) (genuine coercion) is ruled out by construction.

- [s1] [fable-blitz 2026-07-07] Target loop2 (asm/funcs/func_800392C8.s:17-23) is an ordinary sh/addiu -0x10/bgez loop with a NOP delay slot; the only cross-loop scheduling artifact is loop1's bgez slot holding loop2's `addiu $v0,$zero,0xB30` init (asm line 15), which comes from the j=0xB30 assignment position (src line 1232), not from the comma expression.

- [s1] [fable-blitz 2026-07-07] Companion construct note: loop1 uses `i -= 0x10` WITHOUT the comma (src line 1228) and produces the same loop shape (asm lines 9-14) - in-function control confirming the comma adds nothing.
