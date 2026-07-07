# Evidence bank — cpu_get_move_pattern_table_number

- Audit diagnosis (regressions.md): Line 1898: `if ((double)(D_800A3817 == 2))` — the `(double)` cast is a semantically-inert codegen trick (optimizer folds it away: target asm shows plain `bne $v1,$v0` with no float instructions). Fails tests 1/2/3/5. Worker must redo the last `if` arm without the cast.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct at src/code6cac.c:1897: if ((double)(D_800A3817 == 2)) { ... } - the last arm of the D_800A3817 if-chain (cases 0/1/2 at src/code6cac.c:1880,1890,1897)

- [s1] [fable-blitz 2026-07-07] Target asm (build/bb2.elf, func @ 0x8001ea84): the ==2 compare is at 0x8001ee7c: bne v1,v0,0x8001eea0 (epilogue) with nop delay; v0=2 was materialized in the PREVIOUS branch's delay slot (0x8001ee54: li v0,2 under bne v1,v0(=1)). NO float/double/cop1 instructions anywhere in the function - the cast folds away completely pre-RTL

- [s1] [fable-blitz 2026-07-07] Introducing commit 8d342343 ('Match cpu_get_move_pattern_table_number via C + buf[4] + ret-temp idx', 2026-04-20) documents the ret-temp branchless-index trick and s16 buf[4], but NEVER mentions the (double) cast - no recorded motivation; consistent with a leftover permuter/manual mutation that was never byte-load-bearing

- [s1] [fable-blitz 2026-07-07] Branch-sense check: plain if (D_800A3817 == 2) gives do_jump on EQ_EXPR -> jump-to-else-when-false = bne v1,v0,<skip> - exactly the target shape at 0x8001ee7c; the folded (double) form ((x==2)!=0) reaches the same compare, so removal should be RTL-identical

- [s1] [fable-blitz 2026-07-07] Arm-merge context (unaffected by the cast): case-1 arm ends j 0x8001ee98 with li v0,8 in the delay slot, landing on case-2's sh v0,D_800A3834 - a find_cross_jump store-tail merge present in target and produced by the ordinary if-chain; no construct needed to protect it

- [s1] [fable-blitz 2026-07-07] Rest of the function (src/code6cac.c:1814-1901) has no other flagged constructs; the committed byte-correct body differs from the clean form ONLY by the cast
