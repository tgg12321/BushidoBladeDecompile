# func_8001C8DC -- evidence

## 2026-09-29 -- REOPENED (retro-audit FAIL, Q37 class C, 803d0fea1)

The 025f88d91 landing FAILed the 2026-09-29 retro-audit: cross-symbol arithmetic: `p = &D_800A37D2; p[t != 0]++` reaches the separately declared D_800A37D3 (refused 2026-07-20) and is an unannotated pointer alias to a global. Per owner Q37 class C the body went back to `INCLUDE_ASM("asm/funcs", func_8001C8DC);` and the function is back in the queue. Landed text banked verbatim in `rejected/retro-audit-2026-09-29.c`. Also restored `INCLUDE_RODATA("asm/rodata", jtbl_800100C4);` (the landing had replaced it with the compiler-emitted table plus the `const u32 D_800100E0[1]` tail word, removed here). The landing's file-scope externs stay: func_80040510/func_80041BF4 are used by later code; `extern u8 *D_800A3894;` is now unused in code6cac.c.

## s1 (2026-09-30, laneC, ledger-only: src/code6cac_tu2.c is peer-reserved) -- no admissible 0

Frontier from the reopen: remove the refused cross-symbol idiom `p = &D_800A37D2; p[t != 0]++`
(F4, refused 2026-07-20) and its unannotated pointer alias, via a single-object model or branches.

Harness (s1/): mktree.sh exports HEAD (git archive, read-only) to tmp/c8dc/tree with tools/, disc/,
engine/ symlinked; apply.py applies the model; buildtree.sh runs engine.pipeline.build_all() there
(full clean-driver build + link) and prints the EXE SHA1; bindiff.py / dis.sh localise differences.
The unmodified export builds 62efab4f73f992798c43e8c730aa43baa10bb4fa (harness check).

1. Single-object model `extern u8 D_800A37D2[2];` (include/code6cac.h, evidence comment
   s1/hdr_block.txt: base+index addressing in func_8003CF84 0x8003D1DC-0x8003D200 and in this
   function 0x8001CC44-0x8001CC60 / 0x8001CCE4-0x8001CD00), every consumer converted (s1/apply.py:
   func_8001C8DC s1/cand_a.c with `D_800A37D2[t != 0]++` and [0]/[1] clamps; func_800343F0,
   func_8003CF84, func_8001CE60, func_80055138; D_800A37D3 row dropped from undefined_syms_auto.txt;
   INCLUDE_RODATA jtbl_800100C4 removed, `const u32 D_800100E0[1]` tail kept):
   full build SHA1 687de142b791d59599e4d5340878736005c9ef6e (not the oracle). 42 differing words, only in
   func_8001C8DC (8, the two clamps at 0x8001CD10..) and func_80055138 (34, its four D_800A37D2 uses).
   The flag-indexed increments and func_8003CF84's player-indexed access match with the array.
   Why (s1/merged_*_disdiff.txt): the target reaches each byte by its own symbol at every multi-use
   site (`lbu $v0,%lo(D_800A37D2)($v0)` ... `sb $v0,%lo(D_800A37D2)($at)`, then the same for
   D_800A37D3), which is what a scalar's DECL_RTL gives. An array or struct element at a constant
   offset is expanded through change_address -> memory_address (emit-rtl.c:1293-1315), and
   memory_address forces every constant address into a register "to cse them"
   (explow.c:396-399); cse then shares that register between the load and the store (clamp) or
   across the reads (func_80055138), so combine cannot fold it back: `lui/addiu $v1,&D_800A37D2;
   lbu 0($v1); sb 0($v1)`. Respelling the clamps as `*D_800A37D2` / `*(D_800A37D2 + 1)`
   (s1/cand_c1.c): identical result (same SHA1, same 42 words).
2. Plain branches on the two scalars (no index): s1/br1.c (if/else inside the guard) 29, 307/291;
   s1/br2.c (split guard) 36. Sandbox --disable all.
3. The cross-symbol index without the pointer local, `(&D_800A37D2)[t != 0]++` (s1/xsym.c): 19,
   290/291 -- the pointer local is load-bearing too.
4. Reference: the retro-audit body (both scalars + `p`) is sandbox 2 (the jtbl %lo addend,
   operand-only) and matched the oracle when landed (025f88d91).

Reading: the target needs D_800A37D2 and D_800A37D3 as two separately declared scalars (direct
symbol access at multi-use sites, in this function and in func_80055138) AND an index from the
first byte's address (sltu/addu at both increments). With GCC 2.7.2 no single C object gives the
first property, and two scalars give the second only through arithmetic on &D_800A37D2 reaching
D_800A37D3 -- the refused F4 idiom. A per-file split (Q21) cannot help: func_8001C8DC needs both
properties in one function. SOTN matched PS1 code has no F4 precedent (2026-08-18 survey; the
!FAKE `&entity_ranges[0]` at src/dra/7879C.c:1949-1952 @aa53500 stays inside one array).
Side note: func_8003CF84 (src/code6cac_c2.c:954) on main carries the same idiom,
`(&D_800A37D2)[D_800A3748]`; with the array declaration it matches, so it is fixable by the
merge if the merge ever becomes possible, and is existing debt until then.
Status: needs an owner-level decision on F4 for this byte pair (reported to the orchestrator);
the function stays INCLUDE_ASM/active.
