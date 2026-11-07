"""sweep-368e4.2 rows (unit sweep-368e4.2, kind function): func_8004B6E4 (src/main/3AB48.c:21) and func_8004A940 (3AB48.c:20).
Research input tmp/codex/sweep-368e4.2-research.md (untrusted; every claim re-derived at HEAD 06770719a, see notes.txt).
Verdicts CONFIRM (naming-reviewer pre-apply PASS 2/2). The 13 NO_NAME rows of the research file are not rows here (notes.txt). (DATA is empty.)
"""
VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS (round 1, 2/2, 2026-11-06, naming-e)"
NOTES = {}
DROPPED = {}

FUNCS = [
    ("0x8004B6E4", "gpu_EmitOtCutoffFlatLitTexturedTrianglesWithOptionalSubdivision", "RENAME",
     "BODY: asm/funcs/func_8004B6E4.s (INCLUDE_ASM src/main/3AB48.c:21; decl src/main/d_5BF0.c:45), hand-written (the addi lines are marked handwritten); "
     "entered with a0 = record count, a2 = stream, a3 = level (stored to 0x1F8001E0 at :14), returns the advanced stream in v0. Per 0x18-byte record (s0 += 0x18 at :276, :283, bnez back "
     "to .L8004B744 at :198, :277, :284): three vertex indices into the scratch XY table (s5 = 0x1F800020 :20) and Z table (s6 = 0x1F800110 :21), GTE nclip at :53 (backface/off-screen "
     "rejects), OT index from the Zs shifted by 0x1F8001C6 and range-tested against 0x1F800018 / 0x1F800010 (:94-96, the OT cutoff pair), ncs at :115 (one lit colour from the "
     "record normal), then either a POLY_FT3 written at g_prim_buf_cursor (s4) with len word 0x07000000 (:192) and s4 += 0x20 (:197) and linked into the OT, or, when "
     "the 0x1F8001E0 level is positive, jal gpu_SubdivideAndEmitTexturedTriangle (:270, a0 = level). At exit s4 is stored back to g_prim_buf_cursor (:287-288) and the saved "
     "registers are restored. "
     "CALLERS / USES: one table word, D_80015470[0] (src/main/d_5BF0.c:71), reached by the indexed jalr in gpu_ProjectQueuedNodeRunVerticesAndDispatchPackets "
     "(asm/funcs/gpu_ProjectQueuedNodeRunVerticesAndDispatchPackets.s:317-327: type >> 3, a2 = stream, the returned v0 becomes the stream at :329). No jal, no other address word "
     "(git grep -w over src include asm: the INCLUDE_ASM, the d_5BF0.c decl and table entry, the glabel; symbol_addrs.txt:753 comment and :994 note). "
     "SIBLINGS: D_80015470[1] gpu_EmitOtCutoffFlatLitTexturedQuadsWithOptionalSubdivision (landed) has the same scratch layout (0x1F800020 / 0x1F800110, :20-21), the same "
     "0x1F8001C6 / 0x1F800018 / 0x1F800010 cutoff (:121-123), the same level byte at 0x1F8001E0, and its only jal is gpu_SubdivideAndEmitTexturedQuad (:316); the triangle subdivider is "
     "gpu_SubdivideAndEmitTexturedTriangle (landed). gpu_EmitOtCutoffFlatLitTexturedTriangles (32D04.c:1237, handler slot :1452) is a different body with no jal at all "
     "(asm/funcs/gpu_EmitOtCutoffFlatLitTexturedTriangles.s has 0 jal), i.e. no subdivision path, which the suffix separates. "
     "WHY: Emit = writes primitives into the packet buffer; OtCutoff = the 0x1F800018/0x1F800010 range reject, same word as the landed Quads sibling; FlatLit = one ncs colour per triangle; "
     "Textured = POLY_FT3 with uv/clut/tpage from the record; Triangles = three vertices per 0x18-byte record (the Quads sibling takes four); WithOptionalSubdivision = the guarded "
     "jal to the triangle subdivider. The vocabulary is the landed sibling's, so the pair reads as a pair. "
     "COUNTER-EVIDENCE: (1) the colour-level shift from 0x1F8001C0 and the 0x1F8001E1 bit-0 OR into the colour are not in the name; the Quads sibling omits them too, so the pair stays "
     "consistent; (2) the name is long; (3) the 'Optional' condition (record flag bits plus the 0x1F8001E0 level) is read from the asm, not from a named field; "
     "(4) hand-written convention (a0 count, a2 stream, a3 level, scratch base in s7), so no C prototype is claimed and the name states the effect only. "
     "Fresh: gpu_EmitOtCutoffFlatLitTexturedTrianglesWithOptionalSubdivision occurs nowhere in src, include, symbol_addrs.txt, named_syms.txt, function-names.csv "
     "(whole-word, checked at HEAD). Aliases: none beyond the symbol_addrs.txt:994 note for the old name."),
    ("0x8004A940", "gpu_DispatchPrimitiveGroupsThroughHandlerTable", "RENAME",
     "BODY: asm/funcs/func_8004A940.s:1-23 (20 insns, INCLUDE_ASM src/main/3AB48.c:20; df15e20c9 split the jtbl_8001541C case bodies at 0x8004A990-0x8004B6E3 out of the symbol, "
     "see the file's trailing comment). Hand-written: s1 = scratch base, s0 = stream, s4 = handler table. sw ra,4(s1) :2; lhu a0,0(s0) (group count) :3, s0 += 2; while a0 != 0: lhu t0,0(s0) (type), "
     "s0 += 2, t0 = table[type] (sll 2, addu s4, lw :7-10), jalr t0 with a0 = count :13, after which the handler has advanced s0 past its records; lhu the next count; loop; lw ra,4(s1); jr ra. "
     "CALLERS: five jal, all in gpu_DrawQueuedEntries (asm/funcs/gpu_DrawQueuedEntries.s:93, :114, :138, :155, :171), each with s4 holding a primitive handler table: the "
     "0x1F80001C word (lw s4 at :22, :116, :140; set to g_gpu_outline_primitive_handlers or g_gpu_default_primitive_handlers at src/main/32D04.c:506, :515, :1151), "
     "g_gpu_default_primitive_handlers (:107-108, +0xB0 at :112 for entry byte 1 bit 0; :152-153), g_gpu_embedded_lit_packet_handlers (:136-137), and "
     "g_gpu_packed_vertex_textured_primitive_handlers (:169-170). Every slot of those tables (32D04.c:1313-1480 and following) is a primitive emitter taking the record count in a0. "
     "Compare gpu_ProjectQueuedNodeRunVerticesAndDispatchPackets, which dispatches through the fixed D_80015470 by type >> 3 and projects vertices itself; this code only loops, so "
     "ThroughHandlerTable (the table is a register argument) tells them apart. "
     "REVIVE: the ledger row (docs/naming/phase3/no_name.tsv:32, e58177be6) declined only because the symbol then also covered the jtbl_8001541C case bodies ('stays neutral until a split', "
     "pending/func-second-pass-24/notes.txt:11); df15e20c9 split them out, so the objection no longer applies (revive line in notes.txt). "
     "WHY: Dispatch = indirect call through a table, nothing else is done; PrimitiveGroups = the (count, type, records...) unit the stream is made of, terminated by a zero count; "
     "ThroughHandlerTable = the callee comes from the table in s4 by type, the table differing per call site (all landed gpu_ primitive-handler tables). "
     "COUNTER-EVIDENCE: (1) the loop never steps over records, each handler advances s0, so 'Groups' names the stream unit, not what this code consumes; (2) the g_gpu_embedded_lit_packet_handlers table "
     "says 'packet' while the others say 'primitive' (its entries are primitive emitters too); (3) the register convention (s0, s1, s4) is hand-written, so no C prototype is claimed; "
     "(4) alternatives gpu_DispatchPrimitiveRunsThroughHandlerTable / gpu_DrawPrimitiveGroupsThroughHandlerTable; Draw is rejected since this code draws nothing itself. "
     "Fresh: gpu_DispatchPrimitiveGroupsThroughHandlerTable occurs nowhere in src, include, symbol_addrs.txt, named_syms.txt, function-names.csv (whole-word, checked at HEAD). "
     "Old-name surfaces for the comment pass: include/bb2.h:69 and include/game.h:2162 cite 'asm/funcs/func_8004A940.s, after its endlabel' (a file name, renamed with the .s)."),
]

DATA = []
