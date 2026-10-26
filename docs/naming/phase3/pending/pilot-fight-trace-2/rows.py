"""wave82 rows (pilot-fight-trace-2): Q132 mechanical names for the GPU packet emitters and scratchpad
projection helpers of main/3AB48, the resource-selector / draw-queue / node-queue / view-matrix code of
main/368E4, and the match-tally / fog-move / cell-attribute-grid code of main/2B344 (52 functions). Research
input: tmp/codex/pilot-fight-trace-2-research.md (52 RENAME, 14 NO_NAME over 66 targets; untrusted proposals).
Every row was re-derived from the asm bodies (asm/funcs/*.s) or C bodies and every caller in this tree; names
changed against the research say CHANGED FROM RESEARCH in the evidence. The 14 research NO_NAMEs (A4E0, A940,
C404, DDB4, 50334, 50538, 47510, 4779C, 477C4, 477D4, 3F1E4, 3F218, 3F268, 3F5CC) stay neutral and are not rows;
the research's data suggestions (D_800F2B70, D_80099478, D_8009947A, D_800A33B4, D_8009947C, D_800F6654,
D_800EEDD0, D_800A382D, D_800A8FB0 ...) need separate all-access data manifests and are not part of this wave
(DATA is empty). TRACE clauses give the reach counts from docs/naming/runtime/matrix/coverage.tsv (reach only:
no knockout was run, so no name leans on a runtime role).
"""
import csv
import glob
import os
import re

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply 48/52 PASS; 4 failed rows (8004BCC0, 80046DA8, 80046EA0, 80049718) replaced by reviewer-suggested names (marked SUGGESTED, judged fresh at R6)"

NOTES = {}

WT = os.environ.get("P3WT", r"C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a")

# ---------------------------------------------------------------------------------------------------------
# helpers that read the tree so every line number below is the live one
_SRC = {}
for _f in sorted(glob.glob(os.path.join(WT, "src", "main", "*.c"))):
    _SRC[os.path.basename(_f)] = open(_f, encoding="utf-8", errors="replace").read().split("\n")

_COV = {}
_covp = os.path.join(WT, "docs", "naming", "runtime", "matrix", "coverage.tsv")
_rd = list(csv.reader(open(_covp, encoding="utf-8"), delimiter="\t"))
_CAPS = _rd[0][5:]
for _r in _rd[1:]:
    _COV[_r[0].lower()] = [_CAPS[i] for i in range(len(_CAPS)) if _r[5 + i] not in (".", "")]


def B(fn):
    """src/main/<file>:<first>-<last> of the C definition of fn."""
    for f, lines in _SRC.items():
        for k, l in enumerate(lines):
            if not re.match(r"^[A-Za-z_][\w \*]*\b%s\(" % fn, l) or l.startswith("extern"):
                continue
            j = k
            while j < len(lines) and "{" not in lines[j] and ";" not in lines[j]:
                j += 1
            if j >= len(lines) or ";" in lines[j] and "{" not in lines[j]:
                continue
            if lines[j].rstrip().endswith("}"):
                return f"src/main/{f}:{k + 1}-{j + 1}"
            while j < len(lines) and lines[j] != "}":
                j += 1
            return f"src/main/{f}:{k + 1}-{j + 1}"
    raise SystemExit(f"no C definition of {fn}")


def _asm(fn):
    return open(os.path.join(WT, "asm", "funcs", fn + ".s"), encoding="utf-8").read().split("\n")


def A(fn, *pats):
    """asm/funcs/<fn>.s:<first>-<last> body range plus the line of the first match of each pattern."""
    ls = _asm(fn)
    last = max(i for i, l in enumerate(ls) if l.strip() and not l.startswith("endlabel")) + 1
    out = [f"asm/funcs/{fn}.s:2-{last}"]
    for p in pats:
        for i, l in enumerate(ls):
            if re.search(p, l):
                m = re.search(r"\*/\s+(.*)", l)
                txt = re.sub(r"\s+", " ", m.group(1) if m else l).strip()
                out.append(f"'{txt}' at :{i + 1}")
                break
        else:
            raise SystemExit(f"{fn}: no line matches {p}")
    return "; ".join(out)


def T(addr):
    """Reach clause from coverage.tsv (verified at build time)."""
    h = _COV[addr.lower()]
    where = f" ({', '.join(h)})" if len(h) <= 2 else ""
    return (f"TRACE: reached in {len(h)} of {len(_CAPS)} registered captures in docs/naming/runtime/matrix/"
            f"coverage.tsv{where}; reach only, no knockout, no runtime role is claimed (Q132 mechanical name).")


# the shared contract of the indexed packet emitters (3AB48.c, entered through the handler tables
# D_800951D8 / D_80095328 / D_80095488 in 32D04.c:1255-1296 and D_80015480 in d_5BF0.c:76-81)
EMIT = ("COMMON CONTRACT (read from every emitter body): a0 = record count, a1 = records; each record's packed word holds "
        "three (or four) 8-bit vertex indices that index the projected screen-XY table at scratchpad 0x1F800020 "
        "(4 bytes per vertex) and a per-vertex depth table (0x1F8002B4 or 0x1F800244); vertex-outcode AND-mask reject, NCLIP, "
        "average depth mapped (srlv by the 0x1F800008 shift, then an exponent/mantissa remap) to an OT index, the primitive "
        "written at g_prim_buf_cursor and linked into the OT whose base is the scratch word 0x1F80000C; the cursor is stored "
        "back at exit and advances 0x20 (FT3) / 0x28 (FT4, GT3) / 0x34 (GT4) bytes per accepted record. gpu_ follows the "
        "primitive-buffer and OT writes (not a GTE opcode), as the already-named gpu_SubdivideAndEmitTexturedQuad does.")
ORIENT = ("ORIENTATION: a3 = (MAC0 < 0) - 1 (0 when NCLIP is negative, -1 otherwise); the packed lighting-normal words are "
          "xor-ed with a3 before the lighting op, so the normal is flipped for the winding the one-sided emitters would cull, "
          "and neither winding is rejected.")
BIAS = ("DEPTH BIAS: addiu -0x50 / xor with a3 / add into the depth before the shift gives -0x50 for a3 = 0 and +0x4F for "
        "a3 = -1; the Orientation-Adjusted sibling is instruction-identical except that it lacks exactly these three "
        "instructions (normalized diff).")
CUT = ("OT CUTOFF: after the OT index is formed it is compared (slt / bnez) with the scratch word 0x1F800018 and the record "
       "is skipped when below it; func_80044504 writes 0x1F800018 = 0xBE or 0x182 (32D04.c:1145-1150); the one-sided "
       "no-cutoff sibling lacks exactly the load and the compare.")


FUNCS = [
    ("0x8004BCC0", "gpu_EmitOtCutoffFlatLitTexturedQuadsWithOptionalSubdivision", "RENAME",
     f"SUGGESTED by pre-apply reviewer (judge fresh at every use in R6); failed name gpu_EmitFlatLitTexturedQuadsWithOptionalSubdivision: the body also tests the remapped OT index against the scratch cutoff word 0x1F800018 (func_8004BCC0.s:122-133), the discriminator that names its OtCutoff siblings. "
     f"CHANGED FROM RESEARCH: gpu_EmitAdaptiveSubdividedFlatLitTexturedQuads; 'Adaptive' is not shown (the level is the a3 argument "
     f"minus a per-record bit field). BODY: {A('func_8004BCC0', r'\bnclip\b', r'jal\s+gpu_SubdivideAndEmitTexturedQuad', r'\bncs\b', r'lui\s+\$t2, \(0x9000000')}. "
     f"Indexed FT4 emitter, four vertex indices: one-sided (bgez on MAC0), rejected when no corner is within y < 0xF0, depth = maximum of the four corner depths "
     f"(+0x3E8 when record byte 2 bit 0), OT index range-checked against 0x1F800010 and 0x1F800018, NCS flat lighting, UV halfwords shifted right by a "
     f"screen-area level (area < 0x400 / 0x700); when the remaining level a3 is greater than 0 it copies the four vertices and calls "
     f"gpu_SubdivideAndEmitTexturedQuad, otherwise writes the 0x9000000 FT4 packet itself and advances 0x28. Returns the advanced record pointer (move v0, s0). "
     f"CALLERS: 368E4.c:966 (s7val = ...(0x10, base, s7val, 0)), asm wave_Draw.s:97, pointer in D_80015470 (d_5BF0.c:46,72). WHY: flat lit (NCS) textured quads that may be "
     f"subdivided separates it from FD40 (never subdivides) and from the table-colour and Gouraud emitters. COUNTER-EVIDENCE: wave_Draw is one of two routes, so no wave_ "
     f"prefix. {T('0x8004BCC0')}"),
    ("0x8004C994", "gpu_EmitOrientationAdjustedDepthBiasedFlatLitTexturedTriangles", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitOrientationBiasedFlatLitTexturedTriangles; the sibling EF10 also adjusts by orientation, so the bias is what differs and both parts are named. "
     f"BODY: {A('func_8004C994', r'\bnclip\b', r'\bncs\b', r'-0x50', r'xor\s+\$t0, \$t0, \$a3')}. {EMIT} Primitive: FT3 (0x7000000 header, 0x20 per record, record stride 0x18, NCS single colour). "
     f"{ORIENT} {BIAS} CALLERS: only the D_80015480 table (d_5BF0.c:48,79, slot 0), indexed by func_8004DA74 from the run header. "
     f"WHY: Adjusted = winding-dependent normal flip, DepthBiased = the three-instruction bias, FlatLit = NCS, Triangles = FT3; EF10 (no bias), FB74 (one-sided), CB8C (FT4) and CDB0 (NCT) each differ. "
     f"COUNTER-EVIDENCE: no material or fighter identity is claimed; the wave03 gte_nclip name was rejected as a GTE-opcode name for a packet emitter. {T('0x8004C994')}"),
    ("0x8004CB8C", "gpu_EmitOrientationAdjustedDepthBiasedFlatLitTexturedQuads", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitOrientationBiasedFlatLitTexturedQuads (see 8004C994). BODY: {A('func_8004CB8C', r'\bnclip\b', r'\bncs\b', r'-0x50', r'lui\s+\$s5, \(0x9000000')}. {EMIT} "
     f"Primitive: FT4 (0x9000000, 0x28 per record, stride 0x18, four indices, average of four depths, NCS). {ORIENT} {BIAS} CALLERS: D_80015480 slot 1 (d_5BF0.c:49,79). "
     f"WHY: the quad twin of C994 (packet size, four indices); F0FC lacks the bias. {T('0x8004CB8C')}"),
    ("0x8004CDB0", "gpu_EmitOrientationAdjustedDepthBiasedGouraudTexturedTriangles", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitOrientationBiasedGouraudTexturedTriangles (see 8004C994). BODY: {A('func_8004CDB0', r'\bnclip\b', r'\bnct\b', r'-0x50', r'lui\s+\$s5, \(0x9000000')}. {EMIT} "
     f"Primitive: GT3 (0x9000000, 0x28 per record, stride 0x24): five normal words are loaded and xor-ed with a3, lit with NCT, the three colours stored with swc2 20/21/22. "
     f"{ORIENT} {BIAS} CALLERS: D_80015480 slot 2 (d_5BF0.c:50,79). WHY: NCT with three stored colours = Gouraud, which separates it from C994/CB8C; F314 lacks the bias; "
     f"CFE0 (not in this wave) is the GT4 (0xC000000). COUNTER-EVIDENCE: the inherited scratch OT base is an input contract, not a gameplay role. {T('0x8004CDB0')}"),
    ("0x8004D244", "gpu_EmitOtCutoffFlatLitTexturedTriangles", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitDepthFilteredFlatLitTexturedTriangles; what is compared is the remapped OT index against a scratch cutoff, so OtCutoff is the exact wording. "
     f"BODY: {A('func_8004D244', r'\bnclip\b', r'bgez', r'\bncs\b', r'0x1F800018')}. {EMIT} Primitive: FT3 (stride 0x18, NCS). One-sided: records with MAC0 >= 0 are skipped. {CUT} "
     f"CALLERS: D_80095488 slot 4 (32D04.c:1215,1290; func_8004A4E0 installs that table at asm :169-170); D_80095328 carries FB74 in the same slot. WHY: FB74 plus the cutoff (normalized diff: "
     f"two loads, slt, bnez); EF10 (two-sided) and D634 (Gouraud) differ. COUNTER-EVIDENCE: no material identity. {T('0x8004D244')}"),
    ("0x8004D424", "gpu_EmitOtCutoffFlatLitTexturedQuads", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitDepthFilteredFlatLitTexturedQuads (see 8004D244). BODY: {A('func_8004D424', r'\bnclip\b', r'bgez', r'\bncs\b', r'0x1F800018')}. {EMIT} Primitive: FT4 (0x28, four indices, "
     f"average of four depths, NCS), one-sided. {CUT} CALLERS: D_80095488 slot 12 (32D04.c:1216,1292). WHY: the quad twin of D244; FD40 lacks the cutoff. {T('0x8004D424')}"),
    ("0x8004DA74", "gpu_ProjectStridedNodeVerticesAndDispatchPackets", "RENAME",
     f"BODY: {A('func_8004DA74', r'\brtpt\b', r'jalr', r'D_80103608', r'D_80015480')}. a0 = node: the resource stream is fetched through D_80103608[node.s16@4][node.s16@2]; its first word gives the vertex-triple "
     f"count (low half) and the shift a1 (high half); the embedded triples are skipped; the vertex source is the pointer at node+0x60 read at a 0x40 stride (words at +0, +0x40, +0x80 per triple, advancing 0xC0), "
     f"each xyz word >> 7, minus the translation of D_800A3708 (+0x2C..0x34), shifted right by a1; RTPT per triple, SXY stored to 0x1F800020 (0xC per triple) and depth to 0x1F8002B4 (6 per triple); "
     f"then 0x1F800008 = 3 - a1 and the run list is walked: each u16 header >> 3 indexes D_80015480 (+0x10 when node byte 1 bit 0 is set), the handler is called through jalr with a1 = the run, "
     f"until a zero header; returns the node (move v0, s1). CALLER: asm func_8004A940.s:597. WHY: it produces the scratch tables the emitters above read and drives them, "
     f"which separates it from the packed-triple projectors (C404) and the matrix-stream decoder (DDB4), both left neutral. COUNTER-EVIDENCE: no fighter or screen identity; "
     f"not a plain RTPT wrapper (the wave03 gte_rtpt name was rejected). {T('0x8004DA74')}"),
    ("0x8004E574", "gpu_EmitAveragedTableColorTexturedTriangles", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitAverageTableColorTexturedTriangles ('Averaged' reads as the operation). BODY: {A('func_8004E574', r'D_800F2B70', r'\bgpf\b', r'\bgpl\b', r'0x555')}. {EMIT} "
     f"Primitive: FT3 (0x7000000, stride 0x14), one-sided (bgez on MAC0). Colour: three 10-bit table indices from the record word at +0x10 each index the word table D_800F2B70, the RGB bytes (<< 4) "
     f"are accumulated as GPF (IR0 = 0x555) + GPL + GPL, about one third each, and the result is stored with swc2 22. WHY: the averaged table colour separates it from EAC8 (per-vertex colours kept) "
     f"and from the NCS/NCT lit siblings; E7E4 is the four-colour quad (IR0 = 0x400). COUNTER-EVIDENCE: nothing is claimed about D_800F2B70 beyond 'a word table whose RGB bytes are read as colours'; "
     f"no material identity. {T('0x8004E574')}"),
    ("0x8004E7E4", "gpu_EmitAveragedTableColorTexturedQuads", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitAverageTableColorTexturedQuads (see 8004E574). BODY: {A('func_8004E7E4', r'D_800F2B70', r'\bgpf\b', r'0x400', r'lui\s+\$s5, \(0x9000000')}. {EMIT} "
     f"Primitive: FT4 (0x28, stride 0x18), one-sided, four 10-bit table indices (record +0x14 and +0xE), colours accumulated GPF + GPL x3 with IR0 = 0x400 (a quarter each), stored with swc2 22. "
     f"WHY: the quad twin of E574. {T('0x8004E7E4')}"),
    ("0x8004EAC8", "gpu_EmitTableColorGouraudTexturedTriangles", "RENAME",
     f"BODY: {A('func_8004EAC8', r'D_800F2B70', r'0x34000000', r'lui\s+\$s5, \(0x9000000')}. {EMIT} Primitive: GT3 (0x9000000, 0x28 per record, stride 0x14), one-sided, three 10-bit table indices from the record word "
     f"at +0x10; the three table words are copied unchanged into the three vertex colour slots (the first or-ed with the 0x34000000 command), UVs copied. WHY: per-vertex table colours (no GTE colour op) separate it from the "
     f"averaged E574 and from the NCT lit CDB0/F314; ECC8 is the GT4. COUNTER-EVIDENCE: no material or table-provenance claim; Gouraud follows the three distinct colours in one GT3 packet. {T('0x8004EAC8')}"),
    ("0x8004ECC8", "gpu_EmitTableColorGouraudTexturedQuads", "RENAME",
     f"BODY: {A('func_8004ECC8', r'D_800F2B70', r'0x3C000000', r'lui\s+\$s5, \(0xC000000')}. {EMIT} Primitive: GT4 (0xC000000, 0x34 per record, stride 0x18), one-sided, four 10-bit table indices "
     f"(record +0x14 and +0xE), four table words copied unchanged into the four vertex colour slots (the first or-ed with the 0x3C000000 command). WHY: the quad twin of EAC8. {T('0x8004ECC8')}"),
    ("0x8004EF10", "gpu_EmitOrientationAdjustedFlatLitTexturedTriangles", "RENAME",
     f"BODY: {A('func_8004EF10', r'\bnclip\b', r'\bncs\b', r'xor\s+\$t0, \$t0, \$a3')}. {EMIT} Primitive: FT3 (stride 0x18, NCS). {ORIENT} No depth bias: the normalized diff against C994 is exactly the "
     f"three bias instructions. CALLERS: D_80095328 and D_80095488 slot 6 (32D04.c:1223,1271,1291). WHY: Adjusted = the winding-dependent normal flip with no cull; C994 adds the bias; FB74 is the "
     f"one-sided version without the flip. COUNTER-EVIDENCE: table membership gives no screen identity. {T('0x8004EF10')}"),
    ("0x8004F0FC", "gpu_EmitOrientationAdjustedFlatLitTexturedQuads", "RENAME",
     f"BODY: {A('func_8004F0FC', r'\bnclip\b', r'\bncs\b', r'xor\s+\$t0, \$t0, \$a3')}. {EMIT} Primitive: FT4 (0x28, four indices, NCS). {ORIENT} No bias (the diff against CB8C is the same three instructions). "
     f"CALLERS: D_80095328 and D_80095488 slot 14 (32D04.c:1224,1272,1292). WHY: the quad twin of EF10. {T('0x8004F0FC')}"),
    ("0x8004F314", "gpu_EmitOrientationAdjustedGouraudTexturedTriangles", "RENAME",
     f"BODY: {A('func_8004F314', r'\bnclip\b', r'\bnct\b', r'xor\s+\$t0, \$t0, \$a3')}. {EMIT} Primitive: GT3 (stride 0x24, NCT, three colours stored with swc2 20/21/22). {ORIENT} No bias (the diff against CDB0 "
     f"is the three bias instructions plus one nop). CALLERS: D_80095328 and D_80095488 slot 22 (32D04.c:1225,1274,1294). WHY: Gouraud (NCT) separates it from EF10/F0FC; CDB0 adds the bias. {T('0x8004F314')}"),
    ("0x8004F53C", "gpu_EmitOrientationAdjustedGouraudTexturedQuads", "RENAME",
     f"BODY: {A('func_8004F53C', r'\bnclip\b', r'\bncs\b', r'\bnct\b', r'xor\s+\$t2, \$t2, \$a3')}. {EMIT} Primitive: GT4 (0xC000000, 0x34, stride 0x2C): vertex 0 is lit with NCS from the normal words at +0x14/+0x18 and "
     f"vertices 1..3 with NCT from +0x1C..+0x28. {ORIENT} Only four of the six normal words are xor-ed (+0x14, +0x18, +0x20, +0x24; +0x1C and +0x28 are loaded as they are), so 'Adjusted' is "
     f"claimed and not 'flipped' or 'two-sided'. No bias. CALLERS: D_80095328 and D_80095488 slot 30 (32D04.c:1226,1276,1296). WHY: the GT4 member of the Adjusted family; "
     f"CFE0 (not in this wave) adds the bias. COUNTER-EVIDENCE: the partial xor is stated here rather than hidden by the name. {T('0x8004F53C')}"),
    ("0x8004F798", "gpu_EmitOtCutoffRawTexturedTriangles", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitDepthFilteredRawTexturedTriangles (see 8004D244). BODY: {A('func_8004F798', r'\bnclip\b', r'bgez', r'0x25808080', r'0x1F800018')}. {EMIT} Primitive: FT3 with the fixed first "
     f"word 0x25808080 (command 0x25 = polygon + textured + raw-texture bit, colour 0x808080; no lighting op, no colour loaded from the record), record stride 0x10, one-sided. {CUT} CALLERS: D_80095328 and D_80095488 slot 5 "
     f"(32D04.c:1227,1271,1291). WHY: D244 with NCS and the normal loads replaced by the constant raw word; 'Raw' follows the command bit and invents no material identity. {T('0x8004F798')}"),
    ("0x8004F970", "gpu_EmitOtCutoffRawTexturedQuads", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitDepthFilteredRawTexturedQuads (see 8004D244). BODY: {A('func_8004F970', r'\bnclip\b', r'bgez', r'0x2D808080', r'0x1F800018')}. {EMIT} Primitive: FT4 with the fixed first word "
     f"0x2D808080 (polygon + quad + textured + raw bit, colour 0x808080), record stride 0x14, one-sided. {CUT} CALLERS: D_80095328 and D_80095488 slot 13 (32D04.c:1228,1272,1292). WHY: the quad twin of F798. {T('0x8004F970')}"),
    ("0x8004FB74", "gpu_EmitFlatLitTexturedTriangles", "RENAME",
     f"BODY: {A('func_8004FB74', r'\bnclip\b', r'bgez', r'\bncs\b')}. {EMIT} Primitive: FT3 (stride 0x18, NCS), one-sided (bgez on MAC0), no normal flip, no bias, no cutoff: the plain member. "
     f"CALLERS: D_80095328 slot 4 (32D04.c:1229,1270). WHY: D244 = this + the OT cutoff (normalized diff: the load, slt and bnez); EF10 = this with the winding-dependent normal flip and no cull; FF40 "
     f"(not in this wave) adds a GPF colour scale. COUNTER-EVIDENCE: nothing contradicts 'FlatLit' (NCS); the name is not claimed unique among unnamed siblings. {T('0x8004FB74')}"),
    ("0x8004FD40", "gpu_EmitFlatLitTexturedQuads", "RENAME",
     f"BODY: {A('func_8004FD40', r'\bnclip\b', r'bgez', r'\bncs\b')}. {EMIT} Primitive: FT4 (0x28, four indices, NCS), one-sided, no flip, no bias, no cutoff. CALLERS: D_80095328 slot 12 (32D04.c:1230,1272). "
     f"WHY: the quad twin of FB74; D424 adds the cutoff, F0FC the flip. {T('0x8004FD40')}"),
    ("0x80050774", "gpu_EmitBlackFlatTriangles", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitBlackTrianglesWithReservedSlots; 'Reserved' is an interpretation. The shown fact is that the cursor advances for every input record and it is stated here. "
     f"BODY: {A('func_80050774', r'\bnclip\b', r'bgez', r'0x20000000', r'addiu\s+\$s6, \$s6, 0x14')}. {EMIT} Primitive: F3 (0x4000000, command word 0x20000000 = untextured polygon with colour 0, i.e. black), "
     f"three indices from a 4-byte record (stride 4), no lighting, no UV, one-sided. The cursor advance (0x14) sits at the loop tail, so it happens for every input record, accepted or not. "
     f"CALLERS: D_80095328 slot 32 (32D04.c:1235,1276). WHY: an untextured black triangle separates it from every textured emitter. COUNTER-EVIDENCE: black output supplies no shadow identity and none is claimed. {T('0x80050774')}"),
    ("0x80050908", "gpu_EmitBlackFlatQuads", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_EmitBlackQuadsWithReservedSlots (see 80050774). BODY: {A('func_80050908', r'\bnclip\b', r'bgez', r'0x28000000', r'addiu\s+\$s6, \$s6, 0x18')}. {EMIT} Primitive: F4 (0x5000000, command word "
     f"0x28000000, colour 0 = black), four indices from a 4-byte record, average of four depths, one-sided; cursor advance 0x18 per input record, accepted or not. CALLERS: D_80095328 slot 33 (32D04.c:1236,1276). "
     f"WHY: the quad twin of 50774. {T('0x80050908')}"),
    ("0x800520B8", "mem_MoveBlockRoundedUpToWords", "RENAME",
     f"BODY: {A('func_800520B8', r'slt\s+\$at, \$a0, \$a1', r'srl\s+\$a2, \$a2, 2', r'jr\s+\$t9')}. a0 = source, a1 = destination, a2 = byte count: a count of 0 returns; otherwise the count is rounded up to whole words "
     f"((n >> 2) + (n & 3 != 0)); when source >= destination the words are copied forward (16-word unrolled loop, then an indexed tail jump through D_800521AC / D_800521FC); when source < destination both pointers are moved to the end "
     f"and the words are copied backward, so an overlapping block is moved correctly. No game state is read or written. CALLERS: asm arena_Copy, arena_ShiftBlocks, reloc_CopySelectedEntries, reloc_KeepListedEntries, "
     f"reloc_KeepSelectedEntries, ndata_ReadFromPack, func_80040594, func_80049C24 and C sites 309CC.c:211, 32D04.c:989,1029,1094, 35000.c:311,366,509, 368E4.c:2138-2154: each passes (source, destination, byte size). "
     f"WHY: mem_ is explained by the body (a pure memory move with no game-state access, used by the arena / relocation / data-copy code); 'RoundedUpToWords' is necessary because it is not a byte-exact memmove; the "
     f"direction choice makes it a move rather than a copy. COUNTER-EVIDENCE: mem_ is a new prefix in this tree, justified by the operation. {T('0x800520B8')}"),
    ("0x800523E0", "gte_LerpMatrixRowsAndRebuildOrthonormalBasis", "RENAME",
     f"BODY: {A('func_800523E0', r'\bgpf\b', r'jal\s+gte_NormalizeIRVector', r'\bop\b', r'cfc2')}. a0, a1 = two matrices (s16 3x3 at +0, translation untouched), a2 = weight w, a3 = output matrix. Row 0 of both matrices is "
     f"blended with GPF/GPL ((0x1000 - w) * A + w * B) and normalized with gte_NormalizeIRVector; row 1 is blended the same way, its component along the new row 0 is removed (mult x3 for the dot product, GPL with IR0 = "
     f"-dot) and the remainder normalized; row 2 is the cross product (OP) of the two; rotation control registers 0/2/4 are saved and restored around the OP. The rows are stored at a3+0, +6 and +0xC. "
     f"CALLERS: 31548.c:219,249 (interpolating two pose matrices into ents[i].node.work) and asm func_80041188.s:63,116. WHY: gte_ follows the GPF/GPL/OP/control-register work (the other gte_ "
     f"functions of this TU); the name states that rows are blended and the basis rebuilt, which separates it from a plain matrix lerp (math_LerpMatrix3x3). "
     f"COUNTER-EVIDENCE: translation is not touched, so no full-transform interpolation is claimed. {T('0x800523E0')}"),
    ("0x80052C4C", "gte_TransformPointDifferenceWithInheritedLastRow", "RENAME",
     f"BODY: {A('func_80052C4C', r'ctc2\s+\$t0, \$0', r'ctc2\s+\$zero, \$5', r'mvmva')}. a0 = a record of three s16 (origin) followed at +6 by the first two rows of a 3x3 s16 matrix; the six matrix halfwords are loaded "
     f"into rotation control registers 0..2 and translation registers 5..7 are zeroed; control registers 3 and 4 (the last row) are NOT loaded; the vector (a1 - x, a2 - y, a3 - z) is formed and MVMVA 1,0,0,0,0 "
     f"runs; the result stays in IR/MAC, nothing is returned. CALLER: 43500.c:493 (and asm func_80053E9C.s:239); it consumes IR1/IR2 via gte_ReadIR1IR2Sra2 (43500.c:507). WHY: the name states the three facts "
     f"the code shows (subtracted origin, two loaded rows, inherited last row); a full-matrix name would be false for IR3. COUNTER-EVIDENCE: the caller's use of rows 0 and 1 only is why the limitation matters, "
     f"not a claim about collision. {T('0x80052C4C')}"),
    ("0x8004678C", "fight_GetPositionRecordList", "RENAME",
     f"BODY: {B('func_8004678C')} returns D_800A33B4. D_800A33B4 is stored in the resource loader from the fifth header word of the loaded block (368E4.c:176 = sp20, :248, rebased at :267). CALLERS: 17AFC.c:949 "
     f"(func_8002906C clears every entry's used field, the list ends at type == 0) and :1006 (fight_ProcessPositionRecordsInQuad tests the unused entries against the quad), asm fight_ProcessPositionRecordsInQuad.s:106 and "
     f"func_8002906C.s:4. Every caller treats the pointer as the 16-byte PosRec list (17AFC.c:938-947). WHY: fight_ follows fight_ProcessPositionRecordsInQuad, the only consumer. "
     f"COUNTER-EVIDENCE: the stale symbol_addrs comment 'current SE id' (:153) is contradicted by the loader store and every use. {T('0x8004678C')}"),
    ("0x80046798", "resource_GetLoadedSelector", "RENAME",
     f"BODY: {B('func_80046798')} returns D_80099478. D_80099478 is written with the selector func_800460E4 is called with (368E4.c:76) and with -1 when the block is freed (:254); the loader's "
     f"switch cases 3, 4, 7, 11, 13, 18, 34 are keyed on it (:200, :268). CALLERS: 2B344.c:1976,1977,2174,2175 (indexing D_800948BC), 32D04.c:1165 (switch key), 368E4.c:535 (indexing the fog table), 43500.c:592 "
     f"(indexing the start-position rows), asm func_8003E6D8, func_8003F168, func_80044670, func_80046EA0, motion_StartPlayback: each uses the value as a table index or switch key. WHY: it returns the selector of the "
     f"currently loaded arena-block-7 resource; 'resource' is explained by the loader it reports on (arena_OpenBlock, reloc_RelocateBlock, collision_SetGrid on the block) and 'Selector' because every consumer keys on it. "
     f"COUNTER-EVIDENCE: the retired stage_GetId name claimed a stage identity; none is claimed here. {T('0x80046798')}"),
    ("0x800467A8", "resource_HasRetainedSubblocks", "RENAME",
     f"BODY: {B('func_800467A8')} returns D_8009947A. D_8009947A is set to 1 only in the loader cases 3, 13 and 34 (368E4.c:142,162,166), cleared at the start of each load (:128), by func_800464C4 after it relocates the "
     f"remaining sub-blocks and shrinks the arena block (:188-215), and on unload (:255). func_8004659C(n) returns early unless it is set (:229) and re-points the loader at sub-block set n (:217-247). CALLERS: 368E4.c:678 "
     f"(func_800475A4 returns early while it is set) and asm func_800475A4.s:7. WHY: the flag reports that the loaded block still holds its alternate sub-block sets, which func_8004659C selects and "
     f"func_800464C4 finalizes; 'Retained' is the state between load and finalize. COUNTER-EVIDENCE: it is not a stage variant; the name claims none of selectors 3, 13, 34. {T('0x800467A8')}"),
    ("0x80046BF4", "gpu_ResetDrawQueueAndPrepareViewAndLighting", "RENAME",
     f"BODY: {B('func_80046BF4')}. g_draw_queue_cursor = g_draw_queue; D_800A3808 = g_gpu_ot_ptr; D_800A38D6++; D_800A378C = g_gpu_ot_ptr + 4 (the per-frame queue / OT reset); when a0 != 0 the view node D_80101DF0 "
     f"gets the negated rotation, a position pushed back by a2 along the rotated z axis and its matrix (node_SetMatrixFromNegatedRot), then func_80047210 and func_8003F274 refresh the derived matrix and the "
     f"cell attributes; always: three calc_LightMatrix calls, light_ResetPointLight, func_8003F568 and func_8003F5CC. CALLERS: 2B344.c:891,973, 9F9C.c:1996,2028 and asm camera_ApplyFightView.s:178, "
     f"camera_ApplyReplayView.s:56, func_8003C9A4.s:27, func_8003CD10.s:27 (4510C.c:180-186 repeats the first four stores in func_80054F68). WHY: queue/OT reset first, then the optional "
     f"view, then the lights: the name lists the three parts. COUNTER-EVIDENCE: a 'camera-only' wording (wave70) would omit the queue and lights. {T('0x80046BF4')}"),
    ("0x80046DA8", "gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue", "RENAME",
     f"SUGGESTED by pre-apply reviewer (judge fresh at every use in R6); failed name gpu_CollectGridDrawablesApplyPointLightTargetsAndFlushDrawQueue: its a0&1 branch runs func_80046EA0, i.e. the whole func_8003E6D8 pass (collection, selector callback, billboard draw; 2B344.c:1976-1982) and func_8003DA8C's fog moves (2B344.c:1430-1432); 'CollectGridDrawables' named only part of it. "
     f"CHANGED FROM RESEARCH: gpu_CollectDrawablesAndApplyPointLights; the research read func_800444BC as an empty hook, but it calls func_80044504, which builds the view matrices, sets the OT scratch words and runs "
     f"func_8004A4E0 over the queue, then resets g_draw_queue_cursor ({A('func_80044504', r'jal\s+func_8004A4E0', r'g_draw_queue_cursor')}; 32D04.c:1119,1121-1153). BODY: {B('func_80046DA8')}: if (a0 & 1) func_80046EA0(D_800A33C0) (collect the grid drawables and queue fog "
     f"moves); light_ApplyPointLightTargets(); func_800444BC(). CALLERS: 28708.c:1488, 2B344.c:892,974, 9F9C.c:2080,2118,2229 and asm fight_Update.s:91, func_8001EA84.s:48, func_8001EFA0.s:59, func_8003C9A4.s:29, "
     f"func_8003CD10.s:29, replay_PlayFrame.s:395 (each passes 1 or a boolean). WHY: every effect of the body is named: optional collection, light targets, flush. COUNTER-EVIDENCE: 'flush' is the draw-queue interpreter run "
     f"(func_8004A4E0) plus the cursor reset, not a GPU transfer. {T('0x80046DA8')}"),
    ("0x80046DEC", "node_GetSlotMatrixPointerArray", "RENAME",
     f"BODY: {B('func_80046DEC')}: v0 = func_8004153C(a0) (the slot object); returns v0->node_matrix_ptrs or NULL. CALLERS: 17AFC.c:318,3626,3675; 368E4.c:1570; 9F9C.c:2823,2853,2919,5139 and asm fighter_TryDropQueuedHeadAtBone, "
     f"fighter_Update, fighter_UpdateBoneWorldData, func_800203B4, func_800204C0, func_80048BA4, object_AttachToOpponentBone, object_SubmitAttachedTransform: all index the result as a MATRIX * table. "
     f"WHY: node_ follows the node layout (Unk80045878Obj.node_matrix_ptrs) and node_UpdateWorldMatrix; the sibling func_80046E18 (not in this wave) returns the transform_nodes array. COUNTER-EVIDENCE: no fighter-only "
     f"claim, the slot argument is generic. {T('0x80046DEC')}"),
    ("0x80046E54", "gpu_SetRaisedOtCutoffRequested", "RENAME",
     f"BODY: {B('func_80046E54')}: D_800F6654 = (a0 != 0). CALLER: only func_80044504 (asm jal after func_8004A4E0; 32D04.c:1153) with 1; init also stores 1 (368E4.c:434) and func_80046E44 stores 0. The only reader of the "
     f"flag is func_80046E7C, whose only caller func_80044504 writes the scratch word 0x1F800018 = 0x182 when it is set and the override func_8003F268() is 0 (else 0xBE) (32D04.c:1145-1150); the emitters D244, D424, F798, F970 "
     f"and BCC0 compare the remapped OT index with that word. WHY: raised = 0x182 over 0xBE; Requested because D_800A322C overrides it. COUNTER-EVIDENCE: the old g_game_pause_flag symbol comment (symbol_addrs.txt:148) is "
     f"contradicted by every use; no pause or input behaviour is claimed. {T('0x80046E54')}"),
    ("0x80046E7C", "gpu_GetRaisedOtCutoffRequested", "RENAME",
     f"BODY: {B('func_80046E7C')} returns D_800F6654 (see 80046E54). CALLER: only func_80044504 (asm jal; 32D04.c:1147). WHY: the getter of the flag above. {T('0x80046E7C')}"),
    ("0x80046EA0", "gpu_RunGridDrawPassAndQueueFogMoves", "RENAME",
     f"SUGGESTED by pre-apply reviewer (judge fresh at every use in R6); failed name gpu_CollectGridDrawablesAndQueueFogMoves: its first callee func_8003E6D8 also invokes the loaded-selector callback and draws the billboard list (2B344.c:1976-1982), so 'CollectGridDrawables' dropped effects. "
     f"CHANGED FROM RESEARCH: gpu_CollectDrawablesAndQueueFogMoves ('Grid' states which collection: func_8003E6D8's cell-grid pass). BODY: {B('func_80046EA0')}: func_8003E6D8(a0); func_8003DA8C(func_80046798(), a0). "
     f"CALLERS: 4510C.c:169 (10000), func_80046DA8 (368E4.c:494, D_800A33C0 = the a2 distance func_80046BF4 stored) and asm motion_UpdatePlayback.s:399. WHY: the two callee operations named in order; "
     f"fog is established by the already-named g_gpu_last_fog_ot_index / gpu_InitFogDrawMoves that func_8003DA8C writes. COUNTER-EVIDENCE: the retired game_InitStageSound name read it as sound, which no callee does. {T('0x80046EA0')}"),
    ("0x80046F14", "fight_GetStartPositionPairTable", "RENAME",
     f"BODY: {B('func_80046F14')} returns &D_8009947C, an s16[38][4][2][3] block (d_89C78.c:34): per row four pairs of (x, y, z) points. CALLERS: 17AFC.c:199-225 (fighter reset: reads the x and z of one point, halfword offset "
     f"D_800A36A4 * 12 + idx * 3, into the fighter's world/base/collision position), 9F9C.c:3419 (fight_GetStartPosition: row * 0x18 + pair * 6 + point * 3), :3451-3585 (nearest-pair search and the "
     f"playback code), 43500.c:591 (motion start: row func_80046798() * 24 + pair * 6, three halfwords into s->unkC..0x14), asm fight_FindNearestStartPair.s:6, fight_GetStartPosition.s:9, func_80021DB0.s:14, "
     f"func_80022224.s:8, func_80027640.s:9, motion_StartPlayback.s:61. WHY: every access reads (x, y, z) points of the block and uses them as positions a fighter or motion is placed at (fight_ follows the two "
     f"already-named fight_ readers). COUNTER-EVIDENCE: the fallback endpoint use at 9F9C.c:3532 reads the same block as position pairs; 17AFC.c's halfword stride (12 per row) differs from the typed row (24), which "
     f"is stated here and does not change what the block holds. {T('0x80046F14')}"),
    ("0x80047210", "camera_CopyViewMatrixWithHalvedMiddleRow", "RENAME",
     f"BODY: {B('func_80047210')}: D_800EEDD0 = D_80101DF0.xf.mat; D_800EEDD0.m[1][0..2] >>= 1 (rotation row 1, the middle row; translation copied unchanged). D_80101DF0 is the view node func_80046BF4 builds "
     f"from the camera rotation and position (368E4.c:456-476; 4510C.c:118 does the same for playback). CALLERS: 368E4.c:478 (func_80046BF4), 4510C.c:125 and asm "
     f"motion_UpdatePlayback.s:245, func_80046BF4.s:84. WHY: camera_ follows camera_ApplyFightView / camera_ApplyReplayView, whose output (D_80101DF0) is the source; the name states source, copy and the halved "
     f"row exactly, answering the wave70 HalfY failure. COUNTER-EVIDENCE: D_800A3778 receives the matrix pointer but has no reader (symbol_addrs.txt:904), so no consumer role is claimed. {T('0x80047210')}"),
    ("0x800472B0", "camera_GetViewMatrixWithHalvedMiddleRow", "RENAME",
     f"BODY: {B('func_800472B0')} returns &D_800EEDD0, whose only writer is func_80047210 (368E4.c:602-605). CALLERS: 28708.c:1372, 9F9C.c:2056,2106,2216 (each stores it in D_800A3778) and asm fight_Update.s:4, "
     f"func_8001EA84.s:10, func_8001EFA0.s:9, replay_PlayFrame.s:57. WHY: the getter of the matrix 80047210 writes (same wording). {T('0x800472B0')}"),
    ("0x800475A4", "node_OrientAndQueueViewOffsetNode", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_OrientAndQueueViewOffsetNode; the body builds and queues a node record (node_InitIdentity is its init partner), so node_ is the prefix the code shows. BODY: {B('func_800475A4')}: returns at once when "
     f"func_800467A8() != 0; rotates (0, 0, 0x6590) by D_80101DF0's matrix (ApplyMatrix), takes the yaw with ratan2 and the pitch from the horizontalized vector, stores "
     f"them in D_800EEDF0.xf.rot (pitch negated), sets its translation to the view translation with +0x6590 added to z, builds its matrix from D_800F66A0[4] and D_800F66A0[0] with MulMatrix0, and appends the node to "
     f"g_draw_queue. D_800EEDF0 is initialized by node_InitIdentity in func_80047570 (rotation_order 4). CALLER: the per-selector callback table D_800948BC {{func_80047570, func_800475A4}} "
     f"(d_80DF8.c:880), invoked by func_8003E6D8 (2B344.c:1976-1977). WHY: the node is positioned 0x6590 along the view axis and "
     f"oriented from the view, so ViewOffset; no sky or background identity is claimed. COUNTER-EVIDENCE: the early return on the retained-subblock flag is a guard, not an effect, so it is not in the name. {T('0x800475A4')}"),
    ("0x800486FC", "gpu_RefreshGrayscaleMode", "RENAME",
     f"BODY: {B('func_800486FC')}: g_color_mode = func_800167AC() ? 1 : 0; returns g_color_mode. func_800167AC returns D_80106A50.flags & 1 (6CF8.c:114). CALLERS: 31D3C.c:110,260,453,493 (each branches on it), "
     f"32D04.c:18 (math_HsvToRgb forces saturation 0 when set), 368E4.c:331 (func_800467B8 adds 0x1B to the index), :1230,1296 (image loads that run math_GrayscaleRgb555 when set), asm "
     f"func_80041688, func_80041BF4, func_800422BC, func_80042478, func_800467B8, gpu_LoadTim, gpu_LoadTimClut, math_HsvToRgb. Every consumer takes the result as a grayscale predicate. WHY: g_color_mode is already the grayscale flag (symbol_addrs.txt:165) and the function "
     f"re-reads its source flag each time; gpu_ because the image/CLUT loads (gpu_LoadTim*) are consumers. COUNTER-EVIDENCE: the origin of the source flag is not claimed. {T('0x800486FC')}"),
    ("0x8004939C", "ResetEntrySelectionState", "RENAME",
     f"CHANGED FROM RESEARCH: resource_ResetDrawableAndSoundSelections; the drawable / sound readings of D_800EF980 and D_800A33E8 are not shown, and resource_ would be one more unexplained prefix. "
     f"BODY: {B('func_8004939C')}: the 58 halfwords D_800EF980[0..57] = -1, D_800EF9F4 = -2, D_800A33E8[0..1] = -1, D_800A33EC = -1. These are exactly the objects the neighbours use as a selection state: func_800493E4 marks "
     f"entry arg0 and its companions D_80099CC8[arg0][0..1] as 1 and fixes the mode word D_800A33EC to 0 or 1 by arg0 < 0x33 ({B('func_800493E4')}); func_800494D4 stores a bounded value into D_800A33E8[idx] ({B('func_800494D4')}); "
     f"func_80049584 compacts the marked entries into consecutive indices and derives a pair index from D_800A33E8 and the mode ({B('func_80049584')}); the node builders read D_800EF980[...] * 2 ({B('func_80049718')}, {B('func_80049A2C')}). "
     f"CALLERS: 2B344.c:181,203, 9F9C.c:1808 and asm fight_Init.s:90, func_8003AFFC.s:14, func_8003B10C.s:12 (set-up before entries are marked). WHY: it resets the map, its sentinel, the selected pair and "
     f"the mode to the unselected values the marking functions test against; no prefix is claimed. COUNTER-EVIDENCE: nothing is said about what an entry is. {T('0x8004939C')}"),
    ("0x80049718", "node_QueueEntryNodesFromTransformOrUpdatedSlotNode", "RENAME",
     f"SUGGESTED by pre-apply reviewer (judge fresh at every use in R6); failed name node_QueueEntryNodesFromTransformOrSlotNode: the slot branch scales the slot node's transform in place and writes the composed matrix back into it (368E4.c:1924-1929, 1945), so the slot node is not a read-only source. "
     f"CHANGED FROM RESEARCH: node_QueueResourceNodesFromPoseOrSlot; 'Resource' and 'Pose' are not shown. BODY: {B('func_80049718')}: arg0 = entry index (D_800EF980[arg0] < 0 traps), flags != 0 builds a 0x68-byte "
     f"node at g_prim_buf_cursor with unk2 = D_800EF980[arg0] * 2; flags == 1 takes rotation and translation from rot_in / pos; otherwise node 19 + (flags & 1) of the slot object func_8004153C(flags >> 1) is "
     f"scaled by position_scale, composed with the slot root matrix, written back, and a second node (type 3, parent = the first, unk2 = entry * 2 + 1) follows; each node is appended to g_draw_queue; "
     f"g_prim_buf_cursor advances past them. CALLERS: 17AFC.c:3692,3720; 28708.c:1410,1413,1484; 4510C.c:156; 9F9C.c:5020,5023 and asm fighter_Update.s:2399,2414, motion_UpdatePlayback.s:357, "
     f"object_SubmitAttachedTransform.s:72, object_SubmitPoolTransforms.s:58, replay_PlayFrame.s:166,180,387. WHY: node_ follows the node records it builds (node_UpdateWorldMatrix, node_InitIdentity); "
     f"the two sources of the transform are the two branches of the body. COUNTER-EVIDENCE: no weapon, character or effect identity is claimed. {T('0x80049718')}"),
    ("0x80049A2C", "node_QueueTableEntryNodePairUnderSlotNode", "RENAME",
     f"CHANGED FROM RESEARCH: node_QueueResourceNodePairAttachedToSlot ('Resource' is not shown). BODY: {B('func_80049A2C')}: entry = D_80099CC8[arg0][arg2] (0xFF returns); D_800EF980[entry] < 0 traps; a 0x68-byte "
     f"node with unk2 = D_800EF980[entry] * 2, translation (D_80099D3C[(arg1 & 1) * 6 ..] scaled by the slot's position_scale) and rotation from the same row, parent = transform_nodes[12] of the slot object "
     f"func_8004153C(arg1 >> 1), world matrix updated by node_UpdateWorldMatrix, queued; a second node (type 3, parent = the first, unk2 + 1) is queued; the cursor advances past both. CALLERS: 28708.c:1416,1419; "
     f"4510C.c:157,160; 9F9C.c:5016,5026,5029 and asm fighter_Update.s:2387,2424,2438, motion_UpdatePlayback.s:361,368, replay_PlayFrame.s:190,203. WHY: always a pair, the entry chosen through a table pair, "
     f"always under slot node 12; func_80049718 builds its pair from a given transform or from another slot node. {T('0x80049A2C')}"),
    ("0x8003C42C", "fight_TallyMatchResultsAndInitPresentation", "RENAME",
     f"BODY: {B('func_8003C42C')}: counts[g_fight_match_results[i]]++ over g_fight_match_count; D_800A382D = counts[0] < counts[1], and on equal counts the sums of g_fight_match_scores[i][0..1] decide the same way, else 2; "
     f"then gpu_SetDrawEnvBg(1, 0, 0, 0), D_800A37B8 = 0 (timer) and D_800A3834 = 0x15 (next state; func_8003C560 is the following entry of the state table d_7D870.c:50-51). CALLER: that state table only. "
     f"WHY: the tally of the already-named match result / score arrays and the set-up of the timer, background and next state that the following handler uses; fight_ follows the g_fight_match_* names. "
     f"COUNTER-EVIDENCE: no winner rule beyond the code's comparison is claimed ('Tally' states the counting). {T('0x8003C42C')}"),
    ("0x8003C560", "fight_UpdateMatchTallyPresentation", "RENAME",
     f"BODY: {B('func_8003C560')}: D_800A37B8++ (timer); queues sound 0xA4 at 0x1E when D_800A382D == 2, else 0xA8 / 0xA7 (by D_8008D9EC[D_80101EC8[D_800A382D].unk_0A]) at 0x1E and 0xA9 at 0x43; advances "
     f"g_prim_buf_cursor by the byte count func_8005E54C(D_800A3784, cursor, 1) returns (D_800A3784 = the packed match scores, include/bb2.h:484); on a pressed button or timer >= 0xF1 it sets the next state "
     f"(0x18 for a draw, else cdrom_PauseAndWait, D_800A38A4, state 0x12). CALLER: the state table (d_7D870.c:51). WHY: the handler that follows the tally state, using D_800A382D and the packed scores to draw and play "
     f"sounds. COUNTER-EVIDENCE: 'Update' states the timer, sound, draw and state change together. {T('0x8003C560')}"),
    ("0x8003DA8C", "gpu_LatchFogDepthOnTransitionAndQueueMoves", "RENAME",
     f"BODY: {B('func_8003DA8C')}: g_gpu_last_fog_ot_index = 0xFFFF; when D_800906A4[arg0][0] != 0: dist = (0x6590 or 0x55F0 by func_8003F268()) - arg1, clamped to >= 0x1770 with the shortfall as offset; "
     f"when D_800F6656 bit 0 rises relative to the stored D_80090608 it latches dist and offset in D_80090600 / D_80090604; arg1 is reduced by the latched or current offset; D_80090608 = D_800F6656; "
     f"two func_8003DBE4 calls queue the move chains for both banks (0x1F with D_800A3D70, 0x13 with D_800A4340). CALLER: func_80046EA0 (368E4.c:536). WHY: the latch on the flag's rising edge and the queued moves are "
     f"the two effects; fog is established by g_gpu_last_fog_ot_index and gpu_InitFogDrawMoves. COUNTER-EVIDENCE: 'transition' is the flag edge only. {T('0x8003DA8C')}"),
    ("0x8003DBE4", "gpu_QueueDepthSpacedMovesAndLinkTerminalPacket", "RENAME",
     f"BODY: {B('func_8003DBE4')}: step = D_80090600 when D_800F6656 bit 0 else (0x6590 or 0x55F0) - arg0; returns for step < 0; step = min(step, arg3) / arg1; for i < limit it maps arg0 >> 2 to an OT index "
     f"(gpu_DepthToOtIndex), addPrim's the packet at the cursor and advances it by 2 when the index < 0x1000, records the last index in g_gpu_last_fog_ot_index, arg0 += step; after the loop arg4 != 0 stores the next packet into OT slot 0xFFF "
     f"(func_8003DDF8), else addPrim at slot 0xFFB. CALLER: func_8003DA8C (2B344.c:1430,1432). WHY: the packets are spaced by a depth step, and the packet after the loop is linked differently by arg4; "
     f"'Terminal' names the packet after the loop and no more. COUNTER-EVIDENCE: it is not claimed to be the last chain element. {T('0x8003DBE4')}"),
    ("0x8003E2D8", "gpu_SetSegmentCellsGateBit", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_MarkSegmentCellsForGatedDrawableVariant; 'DrawableVariant' is not shown (the collector sets bit 0 of the 16-byte record's unk7, whose role is not established). "
     f"BODY: {B('func_8003E2D8')}: (x0, z0) and (x1, z1) are mapped to the 32x32 grid of 2000-unit cells (+0x7D00, / 2000); a degenerate segment marks one cell, otherwise a DDA-style walk along the dominant axis "
     f"calls func_8003F388 on each cell it crosses (range-checked 0..0x1F) and on the last cell. func_8003F388 ORs 4 into D_800A8FB0[(z + 0x10) * 32 + x + 0x10]. CALLER: func_8003E6A0 (2B344.c:1841). "
     f"WHY: Segment = the marched cells, GateBit = the bit set (4), spelled as func_8003F388. COUNTER-EVIDENCE: no occlusion, shadow or collision claim. {T('0x8003E2D8')}"),
    ("0x8003E6A0", "gpu_SetViewOriginSegmentCellsGateBit", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_MarkViewOriginSegmentForGatedDrawableVariant. BODY: {B('func_8003E6A0')}: func_8003E2D8(D_80101DF0.work.t[0], D_80101DF0.work.t[2], arg0, arg1): the segment starts at the view node's "
     f"x/z. CALLERS: 9F9C.c:2077,2078 (the two fighters' world x/z). WHY: ViewOrigin = the first endpoint. {T('0x8003E6A0')}"),
    ("0x8003E6D8", "gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_CollectGridDrawablesInvokeResourceCallbackAndDrawBillboards ('Resource' replaced by the selector function that picks the callback). BODY: {B('func_8003E6D8')}: the view angle "
     f"(ratan2 of the view matrix applied to (0, 0, 0x1000)) picks one of 64 row-mask tables (D_800927C0 / D_80090740, optionally ANDed with D_80094840); every set cell of the 31x31 window around the view node's "
     f"cell queues its 16-byte records on g_draw_queue (setting unk6 = attr & 3 and unk7 bit 0 from the cell attribute bits 4 / 8) or collects its pool records once; func_8003EB84 repeats the pass over the full grid "
     f"when D_800F6656 bit 0 is set and D_800A322C is 0; then D_800948BC[func_80046798()].unk4 is called when non-null and eff_DrawBillboardList draws the billboard list. CALLER: func_80046EA0 (368E4.c:533). "
     f"WHY: each part of the body is in the name, in execution order. {T('0x8003E6D8')}"),
    ("0x8003F274", "gpu_ResetAndStampViewCenteredCellAttributes", "RENAME",
     f"BODY: {B('func_8003F274')}: clears D_800A8FB0 (256 words = the 32x32 attribute byte grid), then for the 16x16 window around the view node's cell ((work.t + 0x7D00) / 2000) ORs 2-bit values from the row words of "
     f"D_80094A6C into D_800A8FB0[z * 32 + x] (bounds-checked). The collector reads the low two bits as unk6 of the cell's records (2B344.c:1943). CALLERS: 368E4.c:479 (func_80046BF4), 4510C.c:126 and asm func_80046BF4.s. "
     f"WHY: gpu_ because the grid drives which records are queued for drawing; 'Cell' is the 2000-unit grid cell. COUNTER-EVIDENCE: it is not called a collision grid: same-size spatial cells "
     f"do not make it one. {T('0x8003F274')}"),
    ("0x8003F388", "gpu_SetCellGateBit", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_MarkCellForGatedDrawableVariant. BODY: {B('func_8003F388')}: x = a0[0] + 0x10, y = a0[2] + 0x10 (a0 = s16 x/z), when both < 0x20 D_800A8FB0[y * 32 + x] |= 4. "
     f"The collector (2B344.c:1943-1948, 2042-2045) sets the record bit when (attr & 8) or (attr & 4 and the record's unk7 & 8) and clears it otherwise, so 4 gates and 8 forces. CALLERS: func_8003E2D8 "
     f"(2B344.c:1744,1792,1816,1835) only. WHY: the bit set and its role in the collector's test, spelled as the code spells it. COUNTER-EVIDENCE: no meaning for the record bit is claimed. {T('0x8003F388')}"),
    ("0x8003F3D4", "gpu_SetCellForceBit", "RENAME",
     f"CHANGED FROM RESEARCH: gpu_MarkCellForForcedDrawableVariant. BODY: {B('func_8003F3D4')}: the same address computation as func_8003F388, D_800A8FB0[y * 32 + x] |= 8, which the collector tests as the unconditional "
     f"case (attr & 8). CALLERS: 9F9C.c:2001,2002 (h30[0], h30[1] of the fight record). WHY: sibling of SetCellGateBit with the other bit. {T('0x8003F3D4')}"),
    ("0x8003F568", "ClearSavedWordPairsAndTwoWriteOnlyWords", "RENAME",
     f"BODY: {B('func_8003F568')}: D_800A3374 = D_800A3370 = 0 and D_800A93B0[0..2] = D_800A93BC[0..2] = 0. D_800A3370 / D_800A3374 are written only here and in func_8003F7F4 (2B344.c:2388-2389) and never read "
     f"(symbol_addrs.txt:146-147); D_800A93B0 / D_800A93BC are set by the uncalled func_8003F5A8 and read only by func_8003F5CC, which passes the three pairs to the empty func_80017F98 (6CF8.c:798). "
     f"CALLER: func_80046BF4 (368E4.c:488). WHY: no subsystem is established, so no prefix; 'WriteOnly' states the readerless words. COUNTER-EVIDENCE: the two words are not called counters; no access proves counting. {T('0x8003F568')}"),
]

DATA = []
