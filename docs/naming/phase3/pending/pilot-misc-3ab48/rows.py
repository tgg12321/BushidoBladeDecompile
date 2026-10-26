"""wave91 rows (pilot-misc-3ab48): Q132 mechanical names for 11 of the 18 neutral functions in main/3AB48 (the
textured, outline and record-embedded-packet GPU primitive emitters between gpu_EmitOrientationAdjustedDepthBiasedGouraud-
TexturedTriangles and gte_ReadFarColor).
Research input: tmp/codex/pilot-misc-3ab48-research.md (11 RENAME, 7 NO_NAME over 18 targets; untrusted proposals).
Every row was re-derived from the asm body (asm/funcs/<glabel>.s; all 11 are INCLUDE_ASM) and every use found by a
search of src/, include/, asm/, named_syms.txt, symbol_addrs.txt, bb2.ld and tools/ (the dispatch tables D_80015480 /
D_80095280 / D_80095328 / D_80095508 in src/main/d_5BF0.c and src/main/32D04.c are the only references; none of the 11 is a
jal target, and 80051944 / 80051B04 have no reference at all besides their INCLUDE_ASM line).
Research NO_NAMEs (7; stay neutral, not rows): 8004D634 8004D838 (instruction-identical twins of the w82-excluded
80050334 / 80050538), 80050AB8 80050E60 and 80050C68 80051010 (two instruction-identical outline pairs), 80052C10 (stores
zero to 0x1F800400 and returns; the destination's role is not established, so no verb covers the store).
Research RENAMEs dropped by this draft: none. Names changed against the research (CHANGED FROM RESEARCH in the evidence
and the row note):
- 80051208 800513B0: AndLoadRecordColor -> AndLoadRecordRgbc (names the GTE register the lwc2 writes; the word is never
  used as the emitted color, which comes from the patched immediates).
- 800515AC 80051754: FromExtendedRecords -> FromWideRecords. 'Extended' says the record is the compact one plus more;
  the quad's index fetch moves (halfwords at +0xE/+0x10 of the 0x18 record vs one word at +0x10 of the 0x2C record), so
  only the stride (0x24 / 0x2C vs 0x18) is shown. (Open question 1 of the research.)
- 80051944 80051B04 80051D08 80051ED4: 'Prebuilt' -> 'Embedded'. Only 80051B04 leaves the packet's color/command word
  untouched; the other three store a lit color (the command byte arrives in GTE RGBC from the record's word at +0xC), so
  a pre-existing command is not shown. What all four show is that the packet address is the record address plus a small
  offset with the same stride, i.e. packets embedded in the record array. (Open question 2.) 'NormalColorLit' -> 'NccsLit'
  (the opcode, no reading of the lit color); Update -> Write (the body stores coordinates, color and link).
- 80051D08 80051ED4: ToCoarseOt -> ToOt256 (the target is g_gpu_ot256_ptr, the existing 368E4.c global; no 'coarse' reading).
Open question 3 (the 0x1F800400 store of 80052C10) resolved conservatively: stays neutral.
The research's data suggestions (D_80015480 / D_80095280 / D_80015820 / D_80095508 names) need separate all-access data
manifests and are not part of this wave (DATA is empty).
"""
import csv
import os

VERDICT = "CONFIRM"
VERIFIER_NOTE = ("naming-reviewer pre-apply (round 1): 9/11 PASS as proposed; 2 FAIL (8004FF40, 80050120: 'Scaled' read as geometric scale, the body scales only the lit colour) replaced by the reviewer-suggested names (NOTES); none dropped.")
NOTES = {
    '0x8004FF40': 'suggested: naming-reviewer pre-apply round 1 failed gpu_EmitScaledFlatLitTexturedTriangles (GPF by IR0 scales only the lit colour; project "Scaled" means geometric size, gpu_AddScaledSpriteQuads) and suggested gpu_EmitColorScaledFlatLitTexturedTriangles; R6 must judge it fresh at every use.',
    '0x80050120': 'suggested: naming-reviewer pre-apply round 1 failed gpu_EmitScaledFlatLitTexturedQuads (same objection as 8004FF40) and suggested gpu_EmitColorScaledFlatLitTexturedQuads; R6 must judge it fresh at every use.',
}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-c")
_COV = {}
_rd = list(csv.reader(open(os.path.join(WT, "docs", "naming", "runtime", "matrix", "coverage.tsv"), encoding="utf-8"),
                      delimiter="\t"))
_CAPS = _rd[0][5:]
for _r in _rd[1:]:
    _COV[_r[0].lower()] = [_CAPS[i] for i in range(len(_CAPS)) if _r[5 + i] not in (".", "")]


def T(addr):
    h = _COV[addr.lower()]
    return (f"TRACE: not used; coverage.tsv lists the function in {len(h)} of {len(_CAPS)} registered captures "
            f"(docs/naming/runtime/matrix/coverage.tsv), and no part of the name rests on a runtime observation (Q132 mechanical name).")


def R(addr, new, body, uses, why, counter, prior, changed=""):
    g = "func_" + addr[2:].upper()
    ev = (f"{changed}BODY: asm/funcs/{g}.s: {body}. USES: {uses}. WHY: {why}. COUNTER-EVIDENCE: {counter}. "
          f"Prior manifests: no_name.tsv ({prior}). {T(addr)}")
    if changed:
        NOTES[addr] = ("adjusted from research: " + changed.replace("CHANGED FROM RESEARCH: ", "").strip() +
                       " Pending naming-reviewer pre-apply (round 1).")
    return (addr, new, "RENAME", ev)


_OUT = ("the packet is written at g_prim_buf_cursor, its color/command word is the lui/ori immediate pair that "
        "func_80041398 (src/main/31548.c:267-295) rewrites at the .L site listed in D_80015820 (src/main/d_5BF0.c:138-139), "
        "the list ends with the 0x55555555 terminator word, the OT link uses the nonlinear depth mapping")

FUNCS = [
    R("0x8004CFE0", "gpu_EmitOrientationAdjustedDepthBiasedGouraudTexturedQuads",
      "NCLIP (:52) gives the orientation mask $a3; four vertex words and the 4-vertex average depth (:74); the mask flips the sign "
      "of the packed normals (xor, :86-87, :107-108) and is xored with -0x50 and added to the depth as the orientation-dependent "
      "bias (:90-92); NCS for the first vertex (:96), NCT for the other three (:119); a 0x34-byte POLY_GT4 is stored at "
      "g_prim_buf_cursor (:22-23, :146-147) and linked into the OT at the nonlinear depth index; the source record stride is 0x2C",
      "src/main/d_5BF0.c:51 (declaration) and :82 (D_80015480 entry, indexed by "
      "gpu_ProjectStridedNodeVerticesAndDispatchPackets); src/main/3AB48.c:29 INCLUDE_ASM; named_syms.txt:1606 retired-alias comment; "
      "no direct jal",
      "it is gpu_EmitOrientationAdjustedGouraudTexturedQuads plus exactly one operation, the orientation-dependent depth bias "
      "(instruction diff: the three lines :90-92), as gpu_EmitOrientationAdjustedDepthBiasedGouraudTexturedTriangles relates to "
      "the triangle form",
      "the registers set before the loop (RGBC, scratchpad pointers) are an ABI detail; no model or mode identity is claimed",
      "w59,w74"),

    R("0x8004FF40", "gpu_EmitColorScaledFlatLitTexturedTriangles",
      "reads a halfword at 0x60($s2) of the incoming $s2 (:13, in the beqz delay slot), loads it into IR0 (mtc2 $s7, $8, :96) after "
      "NCS (:80); GPF 1 (:97) multiplies the lit IR1-3 by IR0 and the result (RGB2) is stored as the packet color (:105); otherwise "
      "instruction-identical to gpu_EmitFlatLitTexturedTriangles (stride 0x18 :107, 0x20-byte packet at g_prim_buf_cursor)",
      "src/main/32D04.c:1232 (declaration) and :1346 (D_80095328 entry); src/main/3AB48.c:53 INCLUDE_ASM; named_syms.txt:1625 "
      "retired-alias comment; no direct jal",
      "the flat-lit textured triangle emitter plus one multiplication of the lit color by a halfword scale (instruction diff against "
      "gpu_EmitFlatLitTexturedTriangles)",
      "the scale's source (0x60 past a caller-set $s2) is not interpreted: no fade, alpha or material meaning is claimed",
      "w59,w74"),

    R("0x80050120", "gpu_EmitColorScaledFlatLitTexturedQuads",
      "same as 8004FF40 for quads: lh $s7, 0x60($s2) (:13), IR0 from it (:109), NCS (:91), GPF 1 (:110), RGB2 stored as the "
      "color (:118); otherwise gpu_EmitFlatLitTexturedQuads (packed indices from +0xE/+0x10 :29, stride 0x18 :120)",
      "src/main/32D04.c:1233 (declaration) and :1347 (D_80095328 entry); src/main/3AB48.c:54 INCLUDE_ASM; named_syms.txt:1626 "
      "retired-alias comment; no direct jal",
      "quad counterpart of gpu_EmitColorScaledFlatLitTexturedTriangles; differs from gpu_EmitFlatLitTexturedQuads by the scale only "
      "(instruction diff)",
      "the scale's meaning is not claimed",
      "w59,w74"),

    R("0x80051208", "gpu_EmitPatchedColorTriangleOutlinesAndLoadRecordRgbc",
      "per record (stride 0x18, :96) lwc2 $6 loads the word at +0 into GTE RGBC (:28), packed indices from +0xC, NCLIP (:46) and AVSZ3 "
      "(:67) cull and depth (table at 0x1F8002B4), then a closed LINE_F4 outline: vertices first/second/third/first, color/command "
      "0x4C808080 and terminator 0x55555555 built from lui/ori (:88-91, label .L80051358); " + _OUT,
      "src/main/32D04.c:1242 (declaration), :1293 and :1294 (D_80095280 entries), :1352 and :1354 (D_80095328 entries); "
      "src/main/d_5BF0.c:52 (declaration), :83 (D_80015480 entry), :41 and :139 (patch site .L80051358); src/main/3AB48.c:63 "
      "INCLUDE_ASM; named_syms.txt:1635 retired-alias comment; no direct jal",
      "emits a closed three-edge outline (LINE_F4) in the code-patched color and, unlike 800515AC (same body, stride 0x24), also "
      "loads the record's first word into GTE RGBC, which the body does not use afterwards; the name states the load as a separate effect",
      "the emitted color is not the loaded word (it is the patched immediate), so a fixed-color or record-color name would fail N1; "
      "the loaded word's role is not claimed",
      "w59,w74",
      "CHANGED FROM RESEARCH: AndLoadRecordColor -> AndLoadRecordRgbc, the lwc2 targets the GTE RGBC register and the word is not the emitted color. "),

    R("0x800513B0", "gpu_EmitPatchedColorQuadOutlinesAndLoadRecordRgbc",
      "indices from halfwords at +0xE and +0x10 (:27-28), lwc2 $6 from +0 (:29), NCLIP (:54) and the 4-vertex average depth, then "
      "two LINE_F3 packets (color/command 0x48808080, label .L800514FC, :86-87) with vertex orders second/first/third and "
      "third/fourth/second linked into the same OT bucket one after the other (:107, :114); stride 0x18 (:117); " + _OUT,
      "src/main/32D04.c:1243 (declaration), :1295 (D_80095280, two entries), :1360 and :1362 (D_80095328 entries); "
      "src/main/d_5BF0.c:53 (declaration), :84 (D_80015480 entry), :42 and :139 (patch site .L800514FC); src/main/3AB48.c:64 "
      "INCLUDE_ASM; named_syms.txt:1636 retired-alias comment; no direct jal",
      "quad counterpart of 80051208: a closed four-edge outline as two three-point polylines, plus the same RGBC load",
      "no lighting claim (the loaded word is unused); the name does not claim a polyline count",
      "w59,w74",
      "CHANGED FROM RESEARCH: AndLoadRecordColor -> AndLoadRecordRgbc, same reason as 80051208. "),

    R("0x800515AC", "gpu_EmitPatchedColorTriangleOutlinesFromWideRecords",
      "identical to 80051208 except the lwc2 $6 at the top is a nop (:28) and the record stride is 0x24 (:96); closed LINE_F4 with "
      "the patched 0x4C808080 (label .L800516FC, :87-88)",
      "src/main/32D04.c:1244 (declaration), :1297 (D_80095280, two entries), :1368 and :1370 (D_80095328 entries); "
      "src/main/d_5BF0.c:54 (declaration), :85 (D_80015480 entry), :43 and :139 (patch site .L800516FC); src/main/3AB48.c:65 "
      "INCLUDE_ASM; named_syms.txt:1637 retired-alias comment; no direct jal",
      "triangle outline emitter that walks 0x24-byte records (80051208 walks 0x18) and loads no record color; 'Wide' is the observed "
      "stride only",
      "the trailing record data is not read here and no normal or lighting meaning is claimed; the research's 'Extended' (a superset "
      "of the compact record) is not shown",
      "w59,w74",
      "CHANGED FROM RESEARCH: FromExtendedRecords -> FromWideRecords, 'Extended' implies a superset layout that the code does not show; only the stride differs for the triangle. "),

    R("0x80051754", "gpu_EmitPatchedColorQuadOutlinesFromWideRecords",
      "indices from the single word at +0x10 (:27, no RGBC load), NCLIP (:51) and the 4-vertex average depth, then the two LINE_F3 "
      "packets of 800513B0 (0x48808080, label .L80051894, :83-84, links :104 and :111); stride 0x2C (:114)",
      "src/main/32D04.c:1245 (declaration), :1298 and :1299 (D_80095280 entries), :1376 and :1378 (D_80095328 entries); "
      "src/main/d_5BF0.c:55 (declaration), :86 (D_80015480 entry), :44 and :139 (patch site .L80051894); src/main/3AB48.c:66 "
      "INCLUDE_ASM; named_syms.txt:1638 retired-alias comment; symbol_addrs.txt:754 (D_80015480 comment); no direct jal",
      "quad outline emitter over 0x2C-byte records (800513B0: 0x18) without the RGBC load; its index fetch (one word at +0x10) is "
      "the fetch gpu_EmitOrientationAdjustedGouraudTexturedQuads uses",
      "'Extended' not used: the index offset moved relative to the compact record, so a superset layout is not shown, and nothing "
      "about the unread payload is claimed",
      "w59,w74",
      "CHANGED FROM RESEARCH: FromExtendedRecords -> FromWideRecords; the index fetch moves from the +0xE/+0x10 halfwords to the +0x10 word, so the record is not shown to extend the compact one. "),

    R("0x80051944", "gpu_WriteAndLinkEmbeddedNccsLitTrianglePackets",
      "$s6 = $s0 + (word at 0x1F800014 & 0x14) selects one of two 0x14-byte packets inside the record (:22-26), $s0 += 0x28 to the "
      "source part (:27); stride 0x38 for both $s0 and $s6 (:101-102); NCLIP (:51) and AVSZ3 (:72) cull and depth; the three vertex "
      "words are stored to +8/+0xC/+0x10 of the packet (:70, :73, :75); lwc2 $6 from +0xC, lwc2 $0/$1 from +4/+8, NCCS (:84-91); the "
      "OT link with tag length 4 (:98) and RGB2 (swc2 $22, 4($s6), :99) are stored; no use of g_prim_buf_cursor; $s0 ends advanced by "
      "the whole array (:106)",
      "src/main/3AB48.c:67 INCLUDE_ASM; named_syms.txt:1639 retired-alias comment; no call, table or function-pointer reference "
      "anywhere (src/, include/, asm/, tables)",
      "the packets live inside the source records (packet address = record address + 0 or 0x14, same stride) rather than being "
      "allocated, so the function writes their coordinates and lit color and links them; NCCS is the opcode that multiplies the "
      "normal's light by the record color",
      "never referenced, so the name rests on the body alone; the command byte arrives via RGBC from the record word, so a "
      "pre-existing command is not claimed; no model or scene identity",
      "w59,w74",
      "CHANGED FROM RESEARCH: UpdateAndLinkNormalColorLitPrebuiltTrianglePackets -> WriteAndLinkEmbeddedNccsLitTrianglePackets, 'Prebuilt' (pre-existing command) is not shown because the command byte comes through RGBC, and 'NormalColorLit' read the opcode. "),

    R("0x80051B04", "gpu_WriteAndLinkDepthBiasedEmbeddedQuadsAndAuxiliaryPackets",
      "$s6 = $s0 + (word at 0x1F800014 & 0x18) quad packet (:23-27); $s0 += 0x30; $s7 = $s0 + ((word & 0xC) + 8) auxiliary packet "
      "(:28-32); stride 0x50 for all three (:116-118); NCLIP (:60) and the 4-vertex average depth (:82); stores only the four vertex "
      "words at +8/+0xC/+0x10/+0x14 (:88-91), adds the record's word at +4 to the depth before the nonlinear mapping (:92-94), and "
      "links $s6 with tag length 5 (:107), then $s7 with tag length 2 (:114) into the same bucket, so $s7 becomes the bucket "
      "head; no color or command word of either packet is written",
      "src/main/3AB48.c:68 INCLUDE_ASM; named_syms.txt:1640 retired-alias comment; no call, table or function-pointer reference anywhere",
      "the quad's color/command is never stored, so the packet content other than coordinates and tag is already in the record "
      "('Embedded' is shown by the address arithmetic); the auxiliary packet is a second, 2-word packet linked last; the "
      "record-supplied depth bias is added, not computed",
      "the auxiliary packet's 2-word format is not interpreted (no draw-mode or texture-page claim); never referenced",
      "w59,w74",
      "CHANGED FROM RESEARCH: LinkDepthBiasedPrebuiltQuadsAndAuxiliaryPackets -> WriteAndLinkDepthBiasedEmbeddedQuadsAndAuxiliaryPackets, the body writes the four coordinates and 'Embedded' is what the address arithmetic shows. "),

    R("0x80051D08", "gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedTrianglePacketsToOt256",
      "80051944's body with: the OT base g_gpu_ot256_ptr (:17-18) instead of the scratchpad OT pointer; the depth gets -0x3E8 (:82) "
      "before the nonlinear mapping; the mapped index is reduced by srl 4 and andi 0xFF (:93-94) before the OT insertion; stride 0x38",
      "src/main/32D04.c:1246 (declaration) and :1428 (D_80095508 entry); src/main/3AB48.c:69 INCLUDE_ASM; named_syms.txt:1641 "
      "retired-alias comment; no direct jal",
      "the embedded-packet write of 80051944, biased by a fixed -0x3E8 and linked into the 256-entry OT g_gpu_ot256_ptr "
      "(src/main/368E4.c:1570, :1641-1642: ClearOTagR(g_gpu_ot256_ptr, 0x100))",
      "no scene or mode role; the OT is named by its existing global",
      "w59,w74",
      "CHANGED FROM RESEARCH: UpdateAndLinkDepthBiasedNormalColorLitPrebuiltTrianglesToCoarseOt -> WriteAndLinkEmbeddedNccsLitDepthBiasedTrianglePacketsToOt256, 'Prebuilt' and 'Coarse' were readings; the target is g_gpu_ot256_ptr. "),

    R("0x80051ED4", "gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedQuadPacketsToOt256",
      "quad counterpart of 80051D08: stride 0x40 (:110), packet = $s0 + (word & 0x18) (:22-27), four indices and the 4-vertex average "
      "depth (:78), vertex stores :84-87, depth -0x3E8 (:88), NCCS (:95), index >> 4 & 0xFF (:99) into g_gpu_ot256_ptr (:17-18), "
      "RGB2 stored (:108); tag length 5; one packet linked",
      "src/main/32D04.c:1247 (declaration) and :1429 (D_80095508 entry); include/game.h:1276 (comment listing it as a reader of the "
      "s16 table at 0x1F8002B4); src/main/3AB48.c:70 INCLUDE_ASM; named_syms.txt:1642 retired-alias comment; no direct jal",
      "quad counterpart of gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedTrianglePacketsToOt256; differs from 80051B04 by the NCCS "
      "lighting, the fixed bias, the OT256 target and the absence of the auxiliary packet",
      "game.h's comment lists the function only as a reader of the vertex table, no identity claim",
      "w59,w74",
      "CHANGED FROM RESEARCH: UpdateAndLinkDepthBiasedNormalColorLitPrebuiltQuadsToCoarseOt -> WriteAndLinkEmbeddedNccsLitDepthBiasedQuadPacketsToOt256, same reasons as 80051D08. "),
]

DATA = []
DROPPED = {}
