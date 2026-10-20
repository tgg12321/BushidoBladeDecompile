"""wave74 rows: the unnamed functions of 3AB48.c, 4D0A8.c and 5ED34.c that Codex researched
(tmp/codex/w74-research.md: 8 RENAME leads, 49 NO_NAME), re-derived from the code at main 69f00a556.
Codex names were leads only. 4 RENAME. No new prefix (gpu_ is admitted).

Pre-apply review pending (VERDICT / VERIFIER_NOTE empty).

Not named (dropped Codex leads):
- 0x80052B7C (math_TransformSVectorWithTranslation): already judged in tmp/p3w65/rows.py.
- 0x8006EC0C (gpu_UpdateScreenSlide) and 0x8006F528 (gpu_DrawSlidingScreenPanels): already judged in
  tmp/p3w56/rows.py.
- 0x800600C8 (gpu_DrawDecimalSpriteValue): the noun is generic (8005D814 / 8005E098 / 8005F1C8 also
  draw decimal fields) and the name leaves out the fixed frame header drawn first (4D0A8.c:1417-1430,
  whose x depends on the value); the value is D_800A391F, the entry count of the opponent-order list
  (17AFC.c:5419, bb2.h:505), but a role name for the readout (opponents left?) is a guess.
- The 49 NO_NAME rows stay neutral (not re-mined).
"""

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply (round 1, 3/4): the two gpu_SubdivideAndEmitTextured* rows and gpu_DrawImageAndAdvanceSplitWipe CONFIRM; 0x8005D554 FAIL (Clusters is plural, one base point per call; the D_800A326C phase step is unnamed) -- left neutral. Evidence correction for 0x8005FC9C: the picture stays put; the two DR_AREA clip windows' inner edges move out from the centre (4D0A8.c:1333-1347). Emit (not Add): these bodies chain packet tags; the callers store into the OT."

FUNCS = [
    ('0x8004BB68', 'gpu_SubdivideAndEmitTexturedTriangle', 'RENAME',
     'BODY asm/funcs/func_8004BB68.s: a0 is a depth. Depth 0 (:2-33): three vertex records at 0x8/0xC, '
     '0x10/0x14, 0x18/0x1C of the work frame s3 go through RTPT, the u/v byte pairs (0xE, 0x16, 0x1E) '
     'and the clut/tpage halves (0x1CC/0x1CE of the scratchpad base s7) are packed, the three '
     'projected XYs and the code word 0x1D0(s7) are stored into the packet at the cursor s4, its tag '
     'is linked (tag length 7 words, i.e. a textured triangle), and s4 advances 0x20. Depth > 0 '
     '(:34-88): pushes a 0x28-byte frame, builds three edge midpoints with math_MidpointS16x3U8x2 '
     '(position and u/v both averaged: asm/funcs/math_MidpointS16x3U8x2.s) and recurses four times '
     'with depth-1 (three corner triangles and the middle one). Only caller outside the recursion: '
     'func_8004A940.s:1153 (the packet-building case that filled the vertex frame, followed by the '
     'ordering-table store at :1154-1158); no C caller, no table reference (grep src/, asm/data). WHY: '
     'gpu_ is the packet layout and linkage in its own body (tag/code/xy/uv/clut words, RTPT, cursor '
     'advance), not the caller. Subdivide covers the positive-depth branch, Emit the packet write of '
     'every leaf, Textured the u/v+clut/tpage words. COUNTER-EVIDENCE: inherited-register contract '
     "(s0-s7, fp) and a scratch frame, so it has no C prototype (include/m2c_context.h's void(s32) is "
     "m2c's guess); the name makes no claim about what the caller's models are. The quad twin "
     '0x8004C1F4 is distinguished by Triangle/Quad. FRESH: absent from src/, include/, named_syms.txt, '
     'symbol_addrs.txt and every tmp/p3w* draft. CENSUS NAME (draft time): func_8004BB68 (retired note '
     "named_syms.txt:2963 'gte_rtpt_8004BB68')."),
    ('0x8004C1F4', 'gpu_SubdivideAndEmitTexturedQuad', 'RENAME',
     'BODY asm/funcs/func_8004C1F4.s: same shape as func_8004BB68 with four vertices. Depth 0 (:2-37): '
     'RTPT on three vertices then RTPS on the fourth (0x20/0x24 of the frame), u/v and clut/tpage '
     'packed, packet tag length 9 words (a textured quad), s4 advances 0x28. Depth > 0 (:38-125): edge '
     'midpoints plus the centre via math_MidpointS16x3U8x2 (several calls), recursion four times with '
     'depth-1. Only external caller: func_8004BCC0.s:316, which fills the four-vertex frame from the '
     'index list and links the result (:317-321); no C caller or table reference. WHY/COUNTER-EVIDENCE '
     'as for 0x8004BB68: gpu_ from the packet words and linkage in the body; inherited-register '
     'contract, no C prototype. FRESH: absent from src/, include/, named_syms.txt, symbol_addrs.txt '
     'and every tmp/p3w* draft. CENSUS NAME (draft time): func_8004C1F4.'),
    ('0x8005FC9C', 'gpu_DrawImageAndAdvanceSplitWipe', 'RENAME',
     "BODY 4D0A8.c:1287-1394: sets the draw area to the current display buffer's clip rect, then for j "
     '= 0, 1 (left and right half): once D_800A3278 >= 0xB5 it builds a semi-transparent Gouraud quad '
     'edge strip (red fading to black, colour from the displacement) at x = 0x140 -/+ the '
     'displacement, and a narrower draw area (left part ending at the displacement before centre, '
     'right part starting after it); draws the two 64-row sprite strips of D_8009B698 (y = i<<6) under '
     'that clip; ends with a draw mode and D_800A3278++ while the displacement is <= 0x140; returns '
     'the chunk size. Counter D_800A3278 is zeroed by func_8005FBC8 (4D0A8.c:1281), which loads the '
     'picture, and incremented only here (grep src/). Sole caller: 2B344.c:726 (func_8003C2C0, once '
     'per frame in state 0x13 until func_80054F68() or a button ends it). WHY: for the first 0xB4 '
     'frames it draws only the picture; afterwards the two clipped halves move apart with a growing '
     'gap and edge strips, i.e. a picture shown and then split open. Image = the sprite strips, '
     'SplitWipe = the clipped halves and strips, Advance = the frame counter step, so the verb covers '
     'both effects. gpu_ from the draw-area and primitive writes. COUNTER-EVIDENCE: nothing claims '
     'what the picture is; Advance names the one state write the body makes; the counter stops at '
     '0x140 displacement. FRESH: absent from src/, include/, named_syms.txt, symbol_addrs.txt and '
     'every tmp/p3w* draft. CENSUS NAME (draft time): func_8005FC9C.'),
]

DATA = []
