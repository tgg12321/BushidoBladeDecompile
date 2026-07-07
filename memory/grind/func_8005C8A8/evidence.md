# Evidence bank — func_8005C8A8

- [s1] Queue entry: distance=752, verdict=ASM-STRUCTURAL, rules=1, status=parked (2026-06-09 audit REJECTED auto-auth: standard GCC-compiled C). Single rule: asmfix.txt:78 replace_with_asmfile; src/text1b.c body is a stub. The regfix.txt:1751 mention of func_8005C8A8 is only a COMMENT on func_80016E60's rule (a caller-side path note), not a rule on this function.

- [s1] Structure (tmp/blitz/func_8005C8A8_map.txt): 753 insns @0x8005C8A8-0x8005D468, frame 0xB8, saves s0-s7+fp+ra. Single jr $ra. Returns constant 0x4F0 (sp70 set once at entry — prim-buffer bytes consumed). ONE mult, no div insns (all /2 spelled as sign-fixed shifts), no GTE, no jump tables (mode dispatch is an if/else ladder on arg0: 0, 1, 2, >2).

- [s1] Signature: s32 func_8005C8A8(s32 mode, u16 sel, void *primBuf, s32 otDepth). primBuf walks +0x10 per tile prim (var_s3 cursor); prim area at +0x4D8 gets the final texpage (sp68); text prim cursor var_s5 starts at primBuf+0xF0 and is threaded through every func_8007352C return.

- [s1] THE CENTRAL PATTERN: a draw-request param block on the stack (sp+0x18..0x43: +0x18 image ptr, +0x1C position-pair ptr, +0x20 prim cursor, +0x28 highlight flag, +0x2C ot depth, +0x30 x-extra, +0x34 y offset, +0x40..0x43 rgb bytes 0x40/0x40/0x40 + flag 0) rewritten field-subset-wise before EACH of the 11 func_8007352C calls (m2c renders as locals + call `func_8007352C(&sp18)`). Whether these are true stack ARGS (args 5+ homed at sp+0x18.. of a many-arg call) or a by-reference struct must be settled FIRST in the structural session by reading func_8007352C's prologue (it also appears in motion_ShiftControl with a heap struct — same request layout).

- [s1] Only fields that CHANGE are rewritten between consecutive calls (e.g. blocks at m2c 215-245: three consecutive calls rewrite sp18/sp1C/sp34/sp28/sp20/sp2C but sp30 once) — the original relied on the block persisting across calls; drafting must reproduce exactly which stores repeat (redundant stores like sp2C = sp50 EVERY call are in the asm).

- [s1] 8 loops, all tiny counted loops with s16 counters (the (var << 0x10) churn): 3 border-tile 2-iter do-whiles (identical bodies: initTile; rgb = i?0x3C:0x52 into 3 s8 fields at -0xA/-9/-8; y = base+i; x = (0x280-w)/2; h=1; w field; gpu_SetSemiTransp(prim,0); ot_Link(D_800A374C + depth*4, prim); prim += 0x10) — at m2c 186-208, 246-268; a 3x2 nested icon loop (loop_10: outer var_s6_2<3 over icon columns with bit test ((arg1 >> (16+col)) & 1) == row for highlight, inner var_s2<2); the closing double loop var_s6_3<2 each containing TWO 2-iter tile loops (horizontal then vertical border strips, loop_43 + loop_51 with u32-cast /2 midpoint math).

- [s1] Mode ladder specifics: mode==1 -> single title draw (D_8009B11C/D_8009B1BC) then joins the shared column loop; mode==2 -> full setup (0x33 y-base, D_8009B184/18C centered from D_8009B2C4, two draws from D_8009B0F8 block, the 3x2 icon loop, 2 border tiles) then FALLS INTO block_20 (mode==0 entry point!) — i.e. case 0 is a label inside case 2's flow (goto/fallthrough in original source); mode>2 (non-2) -> just var_s6=2 join. Shared tail: column loop var_s6 2..3 (headers from D_8009B110+var_s6*0xC, positions from ptr table D_8009B2AC[var_s6]), then the centered pair D_8009B164/16C + D_8009B164+0x10/17C draws, closing borders, full-screen dim tile (rgb 0/0/0, size w x h from tables, SemiTransp mode 1), texpage via saMotionSet(&D_8009B0E0, 0).

- [s1] Layout tables: D_8009B2BC[mode*2]/D_8009B2BE[mode*2] (s16 w/h per mode, screen 0x280x0xF0 centered), D_8009B2C4 (mode-2 width), position pairs D_8009B164/16C/17C/184/18C/194/1AC/1BC/20C written before use (centered computes `(0x280 - w)/2` and `(w + 0x280)/2 - 0xC`), image blocks D_8009B0E0/0F8/110/11C/14C/158, icon position stride D_8009B14E * 8 * col + D_8009B23C, highlight pos table D_8009B29C[row*8].

- [s1] Highlight flag logic per call: sp28 = (sel == K) ? 0 : 1 with K varying (0, col+3, (arg1>>(16+col))&1 == row, var_s6-1) — note the asm computes `sel == ...` into the flag with INVERTED sense per site; and mode==1 path has the quirk `sp28 = sp58 ? sp48 : 0` (reuses mode value 1 as the flag — m2c line 276: sp28 = sp48).

- [s1] Color constant pattern: selected rows get 0x52 gray, unselected 0x3C (or swapped by row XOR outer index in loop_51's diamond: `if (outer != 0) ? (i==0 ? 0x52 : 0x3C) : (i!=0 ? 0x52 : 0x3C)` — an XOR-shaped 2x2 condition, m2c blocks 56/57 with shared tails).

- [s1] m2c decompiles CLEAN standalone (no jtbls): tmp/blitz/func_8005C8A8_m2c.c (429 lines, --valid-syntax).

- [s1] No Kengo annotation on this function's stub. Related INCOMPLETE siblings: func_8007352C (the draw-request consumer, called 11x here and in motion_ShiftControl — reading its asm prologue settles the stack-args-vs-struct question for BOTH ledgers), motion_ShiftControl (same request-block family, recon'd this same blitz).

- [s1] Ledger did not exist (this init); no WIP checkpoint; zero regfix rules.
