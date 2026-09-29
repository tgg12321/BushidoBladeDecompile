# Evidence bank — func_8005C8A8

- [s1] Queue entry: distance=752, verdict=ASM-STRUCTURAL, rules=1, status=parked (2026-06-09 audit REJECTED auto-auth: standard GCC-compiled C). Single rule: asmfix.txt:78 replace_with_asmfile; src/text1b.c body is a stub. The regfix.txt:1751 mention of func_8005C8A8 is only a COMMENT on func_80016E60's rule (a caller-side path note), not a rule on this function.

- [s1] Structure (tmp/blitz/func_8005C8A8_map.txt): 753 insns @0x8005C8A8-0x8005D468, frame 0xB8, saves s0-s7+fp+ra. Single jr $ra. Returns constant 0x4F0 (sp70 set once at entry — prim-buffer bytes consumed). ONE mult, no div insns (all /2 spelled as sign-fixed shifts), no GTE, no jump tables (mode dispatch is an if/else ladder on arg0: 0, 1, 2, >2).

- [s1] Signature: s32 func_8005C8A8(s32 mode, u16 sel, void *primBuf, s32 otDepth). primBuf walks +0x10 per tile prim (var_s3 cursor); prim area at +0x4D8 gets the final texpage (sp68); text prim cursor var_s5 starts at primBuf+0xF0 and is threaded through every func_8007352C return.

- [s1] THE CENTRAL PATTERN: a draw-request param block on the stack (sp+0x18..0x43) rewritten field-subset-wise before EACH of the 11 func_8007352C calls. (s2: settled — it is the 0x2C-byte EnvA descriptor passed by address, `func_8007352C((s32)&s)`, the same as every completed text1b.c caller, e.g. func_8005D814.)

- [s1] Mode ladder, loops, layout tables: see the s1 recon notes in git history (commit of this file before 2026-09-29); superseded by the s2 draft below.

## s2 (2026-09-29, manual lane laneB) — full-body draft, 752 -> 42

Candidate: memory/grind/func_8005C8A8/candidate.c. Measured with `pwsh tmp/orch/sbx.ps1` (sandbox --disable all).
Floor trail: first full draft 319 -> 167 (arg1 addressable + split cell arrays) -> 119 (tile field order x0,y0,w,h; hdr split
at 0x8009B14C) -> 90 (return value spelled `end_off = arg2 + 0x4F0; size = end_off - arg2;`) -> 77 (declaration order
sel, y_base, mode_off, end_off, size) -> 75 (`top + 0x73 + i`) -> 59 (tail loop: `s.table = D_8009B2AC[j]` right after
`s.header`) -> 42 (`top = ...` computed before the colour stores).

Settled facts (each measured):
- Dispatch is `switch (mode)` with cases in source order 2 (falls through), 0 (break), 1 (break): GCC's 3-case
  decision tree (beq 1; slti 2; beqz 0; bne 2) is exactly the target's ladder. The `li s6,2` in three delay slots is
  reorg stealing the post-switch `for (j = 2; ...)` init.
- arg1 is an `s32` whose address is taken: the prologue `sw a1,0xBC(sp); lhu t0,0xBC(sp)` and the in-loop
  `lw v1,0xBC(sp)` (arg1 >> (j+16)) are reads of the parameter's own stack home. `sel = *(s16 *)&arg1;` reproduces
  both (narrow-stack-param-subword-offset family; SOTN sub-word param read).
- Return: the target keeps 0x4F0 in a spill slot (li t0,1264; sw t0,0x70). A literal `size = 0x4F0` is
  REG_EQUIV-constant (cse adds REG_EQUAL) and local-alloc substitutes it at the return (li v0,1264 at the end).
  `end_off = arg2 + 0x4F0; size = end_off - arg2;` folds in COMBINE (no REG_EQUAL -> no REG_EQUIV) and gives the
  target's spilled pseudo. Dump: tmp/func_8005C8A8/dump (f.cse insn 35 carries REG_EQUAL for the literal form).
- Spill-slot order = pseudo creation order: mode 0x48, ot 0x50, sel 0x58, y_base 0x60, mode_off 0x68, size 0x70 ->
  locals declared `s16 sel; u16 y_base; s32 mode_off; s32 end_off; s32 size;`. y_base is u16 (lhu + addu, no sext).
- Data: sprite headers 0x8009B0E0..0x8009B14B are ONE 12-byte-record array (target forms &D_8009B110 once and reaches
  -0x30/-0x24/-0x18/-0xC; &D_8009B0F8 reaches +0x48). D_8009B14C and D_8009B158 are separate. Cell tables are separate
  per sprite (D_8009B184 is NOT related to D_8009B20C: one big array made cse relate them, +136, which the target does
  not); D_8009B164 is [2][2] (&D_8009B164[1] = +0x10 related in target). D_8009B2BC is {s16 w, h}[3] (D_8009B2C4 =
  [2].w). D_8009B2AC is a 4-entry pointer table.
- Tile loops: field stores in source order x0, y0, w, h (loop.c giv base = last field, +0xE, as target).
- Tail loop: loop.c threshold T=19 in this function (bounded by the f.loop decisions: life 2 * 19 >= 38 moved, < 40
  not). The target hoists &D_8009B2AC into s4; ours only does when `s.table = D_8009B2AC[j]` directly follows
  `s.header` (life 3). That also restores `li s6,2` as the first insn after the switch, so reorg fills
  `j tail` and does NOT invert the case-0 loop branch (reorg.c:3923 reversal only fires on an unfilled jump).

Open at 42 (frontier):
1. Icon loop (case 2): ours hoists &D_8009B14C (pseudo 144) into fp and spills (s16)sel (138) to slot 0x78; target has
   (s16)sel in fp and rematerializes D_8009B14C / D_8009B14E (REG_EQUIV-constant pseudo with no hard reg). Global
   priorities are 144=490 vs 138=476 (BB2_ALLOC_DEBUG). Spelling the count read before the header (`s.table` first)
   stops the hoist and gives 138 fp, but the frame shrinks to 0xB0 (target 0xB8 has an unreferenced slot at 0x78).
2. Case-2 first block: header/table address loads order.
3. Final block (hdr[0]/hdr[1] pair): header store placement and the first `D_8009B2BC[mode].h` load base (v0 vs s4).
