#!/usr/bin/env python3
# Q115 batch a (87A0): the scene quad and the object graph typed through 87A0's step functions.
# SceneQuad moves to game.h (87A0 reads it); its unkC points at Unk8003F6D8Coll, the collision-volume
# block 2B344's Func8003F6D8Inner carries at +0x10 (VECTOR pairs: gte_SetMatrixRotTransIR's three
# swc2 words); 87A0 takes SceneQuad / Func80017A44Output / Func80017A44Record / Func80017848Edge.
# usage: q115a.py   writes tmp/p2/q115/out/ (scratch); BASE_REV env (default 643796e7f, ":" = index);
#        OPT env: comma list; `name` adds a variant, `-name` drops a default (DEFAULT below).
import os, re, subprocess
NL = chr(10)
OUT = "tmp/p2/q115/out/"
BASE_REV = os.environ.get("BASE_REV", "643796e7f")
DEFAULT = {"fa_temp", "fa_p68", "fa_dp_byte", "fa_scr"}
OPT = set(DEFAULT)
for x in os.environ.get("OPT", "").split(","):
    if x.startswith("-"):
        OPT.discard(x[1:])
    elif x:
        OPT.add(x)

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

def rep(t, pairs):
    for a, b in pairs:
        assert t.count(a) == 1, (a[:70], t.count(a))
        t = t.replace(a, b)
    return t

def body(t, sig_start):
    i = t.index(NL + sig_start) + 1
    j = t.index(NL + "}" + NL, i) + 3
    return i, j

def swap_func(t, sig_start, new):
    i, j = body(t, sig_start)
    return t[:i] + new + t[j:]

# ---------------------------------------------------------------- headers
SCENEQUAD = """/* The 16-byte scene quad (2B344 func_8003FA24 fills it in each SceneRec, func_8003F824 copies it into
 * Scene.quads; 87A0 func_8001924C steps one object per quad): the object's g_file_data_buf index (unk0),
 * its flags (unk2; bit 0 picks func_80019310 over func_800187F4), the object's matrix, its point table end
 * and its collision-volume block. */
typedef struct {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ MATRIX *unk4;
    /* 0x08 */ u8 *unk8;
    /* 0x0C */ struct Unk8003F6D8Coll *unkC;
} SceneQuad;

/* The collision volumes of one scene record (2B344 Func8003F6D8Inner +0x10; SceneQuad.unkC points here):
 * unk00 (func_800400B0 sets it; 87A0 func_80017FA0 scales it into the scratchpad's ground word), the
 * volume count, each volume's two foci (gte_SetMatrixRotTransIR writes each as three words) and its
 * bound (unk68). */
typedef struct Unk8003F6D8Coll {
    /* 0x00 */ s32 unk00;
    /* 0x04 */ s32 count;
    /* 0x08 */ VECTOR foci[3][2];
    /* 0x68 */ s32 unk68[3];
} Unk8003F6D8Coll;

"""

def game_h(t):
    anchor = "/* func_80017D84's argument (2B344 func_8003FA24 builds it on its stack)"
    assert t.count(anchor) == 1
    return t.replace(anchor, SCENEQUAD + anchor)

def bb2_h(t):
    return rep(t, [("extern void func_8001924C(s16 *, s32);", "extern void func_8001924C(SceneQuad *, s32);")])

# ---------------------------------------------------------------- 2B344
def s2B344(t):
    return rep(t, [
        ("""    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s16 pairs[3][16];
    /* 0x78 */ s32 unk78[3];
    /* 0x84 */ SVECTOR quads[3][2];""",
         """    /* 0x10 */ Unk8003F6D8Coll coll;
    /* 0x84 */ SVECTOR quads[3][2];"""),
        ("extern void gte_SetMatrixRotTransIR(MATRIX *, SVECTOR *, s16 *);",
         "extern void gte_SetMatrixRotTransIR(MATRIX *, SVECTOR *, VECTOR *);"),
        ("gte_SetMatrixRotTransIR(mat, &in->quads[j][0], in->pairs[j]);",
         "gte_SetMatrixRotTransIR(mat, &in->quads[j][0], &in->coll.foci[j][0]);"),
        ("gte_SetMatrixRotTransIR(mat, &in->quads[j][1], in->pairs[j] + 8);",
         "gte_SetMatrixRotTransIR(mat, &in->quads[j][1], &in->coll.foci[j][1]);"),
        ("""/* The 16 bytes func_8003FA24 fills at SceneRec +0x04 and func_8003F824 copies into Scene.quads:
   the record's id (unk0), two zero bytes, the object's matrix (its +0x18), its point table end and
   the address of the record's inner.unk10. */
typedef struct {
    /* 0x00 */ u16 unk0;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ MATRIX *unk4;
    /* 0x08 */ u8 *unk8;
    /* 0x0C */ void *unkC;
} SceneQuad;

""", ""),
        ("    rec->inner.unk14 = 0;\n", "    rec->inner.coll.count = 0;\n"),
        ("    rec->quad.unkC = &rec->inner.unk10;\n", "    rec->quad.unkC = &rec->inner.coll;\n"),
        ("            in->unk78[n] = *a2++;\n", "            in->coll.unk68[n] = *a2++;\n"),
        ("    in->unk14 = n;\n", "    in->coll.count = n;\n"),
        ("            v1->recs[i].inner.unk10 = a1;\n", "            v1->recs[i].inner.coll.unk00 = a1;\n"),
        ("    func_8001924C((s16 *)s0->quads, s0->count);\n", "    func_8001924C(s0->quads, s0->count);\n"),
    ])

# ---------------------------------------------------------------- 87A0
F924C = """void func_8001924C(SceneQuad *arg0, s32 arg1) {
    s32 i = 0;
    SceneQuad *s0;
    /* FAKE: pointer alias -- g_file_data_buf's address held in an integer local. Referenced directly, its
     * lui/addiu is scheduled after `move s0,a0` (score 2); held as a u8 * the addu operands swap (score 2). */
    s32 buf;

    if (i < arg1) {
        buf = (s32)g_file_data_buf;
        s0 = arg0;
        do {
            if (s0->unk2 & 1) {
                s16 val = s0->unk0;
                func_80019310(s0, (Func80017A44Output *)(val * 52 + buf));
            } else {
                s16 val = s0->unk0;
                func_800187F4(s0, (Func80017A44Output *)(val * 52 + buf));
            }
            i++;
            s0++;
        } while (i < arg1);
    }
}
"""

def in_func(t, sig_start, pairs, counts=None):
    i, j = body(t, sig_start)
    b = t[i:j]
    for k, (a, c) in enumerate(pairs):
        n = (counts or {}).get(k, 1)
        assert b.count(a) == n, (sig_start, a[:60], b.count(a), n)
        b = b.replace(a, c)
    return t[:i] + b + t[j:]

def node_members(b, v):
    # v[k] -> Func80017A44Record member, for k in 0..8
    names = ["pos[0]", "pos[1]", "pos[2]", "field_C", "field_10", "field_14", "index", "field_1C", "field_20"]
    for k, nm in enumerate(names):
        b = re.sub(r"\b%s\[%d\]" % (v, k), "%s->%s" % (v, nm), b)
    return b

def f87F4(t):
    i, j = body(t, "void func_800187F4(s16 *arg0, s32 *arg1) {")
    b = t[i:j]
    b = rep(b, [
        ("void func_800187F4(s16 *arg0, s32 *arg1) {", "void func_800187F4(SceneQuad *arg0, Func80017A44Output *arg1) {"),
        ("    s32 *node;\n", "    Func80017A44Record *node;\n"),
        ("    func_80018094((s32 *)arg0, arg1);\n", "    func_80018094(arg0, arg1);\n"),
        ("    count = *(s16 *)((u8 *)arg1 + 4);\n    node = (s32 *)arg1[3];\n",
         "    count = arg1->count;\n    node = arg1->records;\n"),
        ("for (i = 0; i < count; i++, node += 16) {", "for (i = 0; i < count; i++, node++) {"),
        ("        if (*(s32 *)((u8 *)arg0 + 0xC) != 0) {", "        if (arg0->unkC != 0) {"),
        ("        bits = node[9];\n", "        bits = *(s32 *)&node->field_24[0];\n"),
        ("                bits = node[10];\n", "                bits = *(s32 *)&node->field_24[4];\n"),
        ("        bits2 = node[11];\n", "        bits2 = *(s32 *)&node->field_2C[0];\n"),
        ("                bits2 = node[12];\n", "                bits2 = *(s32 *)&node->field_2C[4];\n"),
    ])
    assert b.count('"r"(arg1[0] + i * 8)') == 2
    b = b.replace('"r"(arg1[0] + i * 8)', '"r"(&arg1->points[i])')
    b = node_members(b, "node")
    assert not re.search(r"\bnode\[", b), "node index left"
    return t[:i] + b + t[j:]

def f9310(t):
    i, j = body(t, "void func_80019310(s16 *arg0, s32 *arg1) {")
    b = t[i:j]
    b = rep(b, [
        ("void func_80019310(s16 *arg0, s32 *arg1) {", "void func_80019310(SceneQuad *arg0, Func80017A44Output *arg1) {"),
        ("    s32 *dst;\n", "    Func80017A44Record *dst;\n"),
        ("    dst = (s32 *)arg1[3];\n    for (i = 0; i < ((s16 *)arg1)[2]; i++) {",
         "    dst = arg1->records;\n    for (i = 0; i < arg1->count; i++) {"),
        ('        :: "r"((s32 *)(arg1[0] + i * 8)) : "$12", "memory");', '        :: "r"(&arg1->points[i]) : "$12", "memory");'),
        ("        dst = (s32 *)((u8 *)dst + 0x40);\n", "        dst++;\n"),
        ("    *(MATRIX *)(arg1 + 5) = **(MATRIX **)(arg0 + 2);\n", "    arg1->matrix = *arg0->unk4;\n"),
    ])
    assert b.count('"r"(*(s32 *)(arg0 + 2))') == 2
    b = b.replace('"r"(*(s32 *)(arg0 + 2))', '"r"(arg0->unk4)')
    b = node_members(b, "dst")
    assert not re.search(r"\bdst\[", b)
    return t[:i] + b + t[j:]

def f8094(t):
    i, j = body(t, "void func_80018094(s32 *arg0, s32 *arg1) {")
    b = t[i:j]
    b = rep(b, [
        ("void func_80018094(s32 *arg0, s32 *arg1) {", "void func_80018094(SceneQuad *arg0, Func80017A44Output *arg1) {"),
        ("    s32 *dst;\n", ""),
        ("    dx = ((s32 *)arg0[1])[5] - arg1[10];\n", "    dx = arg0->unk4->t[0] - arg1->matrix.t[0];\n"),
        ("    dy = ((s32 *)arg0[1])[6] - arg1[11];\n", "    dy = arg0->unk4->t[1] - arg1->matrix.t[1];\n"),
        ("    dz = ((s32 *)arg0[1])[7] - arg1[12];\n", "    dz = arg0->unk4->t[2] - arg1->matrix.t[2];\n"),
        # rev-q115a: the LZC count's register-named holders become one labelled mask local
        ("""                    s32 lw_v1 = sp_tmp[0];
                    s32 li_v0 = -2;
                    li_v0 = lw_v1 & li_v0;
                    shift_a = 0x16 - li_v0;
""", """                    /* FAKE: the -2 mask held in a local that then takes the masked count (li -2 /
                     * and v0,v1,v0); as `0x16 - (sp_tmp[0] & -2)` the load and the AND move to $a0:
                     * score 4 */
                    s32 tmp = -2;
                    tmp = sp_tmp[0] & tmp;
                    shift_a = 0x16 - tmp;
"""),
        # dst was a copy of arg1 (ablated: score 0) and goes
        ("    dst = arg1;\n    *(MATRIX *)(dst + 5) = *(MATRIX *)arg0[1];\n    func_80018300(dst);\n",
         "    arg1->matrix = *arg0->unk4;\n    func_80018300(arg1);\n"),
    ])
    assert b.count('"r"(arg0[1])') == 2
    b = b.replace('"r"(arg0[1])', '"r"(arg0->unk4)')
    return t[:i] + b + t[j:]

def f8300(t):
    i, j = body(t, "void func_80018300(s32 *arg0) {")
    b = t[i:j]
    b = rep(b, [
        ("void func_80018300(s32 *arg0) {", "void func_80018300(Func80017A44Output *arg0) {"),
        ("    s32 *data;\n    u8 *base;\n", "    Func80017848Edge *data;\n    Func80017A44Record *base;\n"),
        ("    s32 *p1, *p2;\n", "    Func80017A44Record *p1, *p2;\n"),
        ("    count = *(s16 *)((u8 *)arg0 + 6) - 1;\n    data = *(s32 **)((u8 *)arg0 + 0x10);\n    base = *(u8 **)((u8 *)arg0 + 0xC);\n",
         "    count = arg0->edge_count - 1;\n    data = arg0->edges;\n    base = arg0->records;\n"),
        ("        data += 4;\n", "        data++;\n"),
    ])
    b = b.replace("thresh = data[1];", "thresh = data->ends.pair;")
    b = b.replace("radius = data[0];", "radius = data->dist;")
    b = b.replace("(s32 *)(base + ((thresh >> 16) << 6))", "&base[thresh >> 16]")
    b = b.replace("(s32 *)(base + ((thresh & 0xFFFF) << 6))", "&base[thresh & 0xFFFF]")
    b = node_members(b, "p1")
    b = node_members(b, "p2")
    assert not re.search(r"\b(p1|p2|data)\[", b), re.findall(r".*\b(?:p1|p2|data)\[.*", b)[:3]
    return t[:i] + b + t[j:]

def f7FA0(t):
    i, j = body(t, "void func_80017FA0(s32 *a0) {")
    b = t[i:j]
    b = rep(b, [
        ("void func_80017FA0(s32 *a0) {", "void func_80017FA0(SceneQuad *a0) {"),
    ])
    if "fa_temp" in OPT:   # keep the integer holder for the null test
        b = rep(b, [("    s32 temp;\n    s32 *ptr;\n",
                     "    /* FAKE: temp takes the block pointer for the null test and ptr a copy after it (lw v0 / move t1,v0);\n"
                     "     * tested as ptr itself the load goes straight to t1 and the move drops: score 3 */\n"
                     "    Unk8003F6D8Coll *temp;\n    Unk8003F6D8Coll *ptr;\n"),
                    ("    temp = a0[3];\n", "    temp = a0->unkC;\n"), ("    ptr = (s32 *)temp;\n", "    ptr = temp;\n")])
    else:
        b = rep(b, [("    s32 temp;\n    s32 *ptr;\n", "    Unk8003F6D8Coll *ptr;\n"),
                    ("    temp = a0[3];\n    if (temp == 0) {\n        goto end;\n    }\n    ptr = (s32 *)temp;\n",
                     "    ptr = a0->unkC;\n    if (ptr == 0) {\n        goto end;\n    }\n")])
    b = rep(b, [
        ("    scr[0x2E] = ptr[0] << 7;\n", "    scr[0x2E] = ptr->unk00 << 7;\n"),
        ("        if (i < ptr[1]) {\n", "        if (i < ptr->count) {\n"),
        ("            } while (i < ptr[1]);\n", "            } while (i < ptr->count);\n"),
        ("    scr[0x18] = ((s32 *)a0[3])[1];\n", "    scr[0x18] = a0->unkC->count;\n"),
    ])
    if "fa_scr" in OPT:
        b = rep(b, [("    s32 *scr = (s32 *)0x1F800000;\n", ""),
                    ("    scr[0x2E] = ptr->unk00 << 7;\n", "    SCR->ground = ptr->unk00 << 7;\n"),
                    ("    scr[0x18] = a0->unkC->count;\n", "    SCR->nsph = a0->unkC->count;\n")])
    if "fa_ac" in OPT:
        b = rep(b, [("            s32 *ac_base = (s32 *)0x1F800000;\n", "            s32 *ac_base = SCR->rad;\n"),
                    ("ac_base[0x2B] = ", "*ac_base = ")])
    if "fa_p68w" in OPT:
        b = rep(b, [("            s32 *p68 = ptr;\n", "            s32 *p68 = ptr->unk68;\n"),
                    ("                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;\n                p68 = (s32 *)((u8 *)p68 + 4);\n",
                     "                ac_base[0x2B] = *p68 << 2;\n                p68++;\n")])
    elif "fa_p68c" in OPT:
        b = rep(b, [("            s32 *p68 = ptr;\n", "            Unk8003F6D8Coll *p68 = ptr;\n"),
                    ("                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;\n                p68 = (s32 *)((u8 *)p68 + 4);\n",
                     "                ac_base[0x2B] = p68->unk68[0] << 2;\n                p68 = (Unk8003F6D8Coll *)((s32 *)p68 + 1);\n")])
    elif "fa_p68" in OPT:
        b = rep(b, [("            s32 *p68 = ptr;\n", "            s32 *p68 = (s32 *)ptr;\n")])
    else:
        b = rep(b, [("            s32 *p68 = ptr;\n", ""),
                    ("                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;\n                p68 = (s32 *)((u8 *)p68 + 4);\n",
                     "                ac_base[0x2B] = ptr->unk68[i] << 2;\n")])
    if "fa_dp_byte" not in OPT:
        b = rep(b, [("                    s32 *dp = (s32 *)((u8 *)ptr + data_off);\n",
                     "                    VECTOR *dp = (VECTOR *)((u8 *)ptr->foci + data_off);\n"),
                    ("dp[2] << 2", "dp->vx << 2"), ("dp[3] << 2", "dp->vy << 2"), ("dp[4] << 2", "dp->vz << 2")])
    return t[:i] + b + t[j:]

def move_scr(t):
    a = t.index("typedef struct {\n    s32 d0[3];      /* 0x00 delta to focus 0 (GTE input) */")
    e = t.index("#define SCR ((Scr1F800000 *)0x1F800000)\n") + len("#define SCR ((Scr1F800000 *)0x1F800000)\n")
    blk = t[a:e]
    t = t[:a] + t[e:]
    anchor = "/* func_80017FA0 - copies "
    assert t.count(anchor) == 1
    return t.replace(anchor, blk + anchor)

COMMENTS = [
    ("""/* func_80017FA0 - copies scaled fields out of the block at a0[3] into scratchpad
 * RAM (0x1F800000). ptr[0] is written scaled by 128; ptr[1] is the group count, and
 * each group writes three words scaled by 4 at a 0x18 stride plus one word
 * taken from the 0x68 array. Nothing happens when a0[3] is null.""",
     """/* func_80017FA0 - copies the scene quad's collision volumes (a0->unkC) into scratchpad
 * RAM (0x1F800000). unk00 is written scaled by 128 (the ground word); count is the
 * volume count, and each volume writes its two foci (three words each, scaled by 4) at
 * a 0x18 stride plus its bound from unk68. Nothing happens when a0->unkC is null."""),
    (" * `if (i < ptr[1])`, not `if (ptr[1] > 0)`: `i` then survives to frame layout,",
     " * `if (i < ptr->count)`, not `if (ptr->count > 0)`: `i` then survives to frame layout,"),
    ("""/* func_80018094 -- loads the GTE rotation/translation from the MATRIX at arg0[1],
 * runs func_80017FA0, writes that MATRIX's translation minus arg1[10..12] to
 * scratchpad, scales it by a factor derived from its length (byte-LUT integer
 * sqrt, GTE LZC above 0x400; 0x100 beyond 250000), copies the MATRIX to arg1+5 and
 * runs func_80018300.""",
     """/* func_80018094 -- loads the GTE rotation/translation from the quad's MATRIX (arg0->unk4),
 * runs func_80017FA0, writes that MATRIX's translation minus the object's (arg1->matrix.t)
 * to scratchpad, scales it by a factor derived from its length (byte-LUT integer
 * sqrt, GTE LZC above 0x400; 0x100 beyond 250000), copies the MATRIX to arg1->matrix and
 * runs func_80018300."""),
    (""" * Distance-constraint pass over a chain of 64-byte nodes. arg0+6 is the link
 * count, arg0+0xC the node array, arg0+0x10 a table of 16-byte links (word 0 =
 * rest length, word 1 = two packed node indices). For each link the node pair is
 * bisected (the midpoint replaces the node named by the low index half if
 * its word 6 is negative, else the node named by the high half, and that
 * node's words 3..5 are quartered) until every axis delta is within 3x the""",
     """ * Distance-constraint pass over a chain of 64-byte nodes. arg0->edge_count is the
 * link count, arg0->records the node array, arg0->edges a table of 16-byte links (dist =
 * rest length, ends = two packed node indices). For each link the node pair is
 * bisected (the midpoint replaces the node named by the low index half if
 * its index (state) is negative, else the node named by the high half, and that
 * node's field_C..field_14 are quartered) until every axis delta is within 3x the"""),
    (""" * Node-chain integrator. func_8001924C calls it for each 16-byte record (arg0;
 * +0xC enables collision) whose flag bit 0 is clear, with the record's descriptor
 * (arg1: +0 table of 8-byte anchor vectors, +4 s16 node count, +0xC the 64-byte
 * nodes). func_80018094 first sets up the GTE rotation/translation. Per node
 * (words 0-2 position, 3-5 velocity, 6 state, 7/8 force counts, 9-12 packed
 * force-table indices): state >= 0 springs the node toward its GTE-transformed""",
     """ * Node-chain integrator. func_8001924C calls it for each scene quad (arg0;
 * unkC enables collision) whose flag bit 0 is clear, with the quad's object
 * (arg1: points, the 8-byte anchor vectors; count, the node count; records, the 64-byte
 * nodes). func_80018094 first sets up the GTE rotation/translation. Per node
 * (pos, velocity field_C..field_14, state index, force counts field_1C / field_20,
 * packed force-table indices field_24 / field_2C): state >= 0 springs the node toward its GTE-transformed"""),
    ("""        /* Ruling 11 (reused local): two values, both force counts -- node
         * word 7 (forces added) and node word 8 (forces subtracted). */""",
     """        /* Ruling 11 (reused local): two values, both force counts -- node
         * field_1C (forces added) and node field_20 (forces subtracted). */"""),
]

SC = " Ablated (2026-10-06): score %d. */"
SCORES = [
    ("             * (tools/maspsx/maspsx/__init__.py:1183). */", "             * (tools/maspsx/maspsx/__init__.py:1183)." + SC % 12),
    ("     * dead pad): sp_tmp[0] is the live LZC output. */", "     * dead pad): sp_tmp[0] is the live LZC output. (as s32 sp_tmp[1]: score 8) */"),
    ("             * (flow.c:2081), the numerator of global.c's allocno_compare priority.\n             * Family: do-while-zero-exception. */",
     "             * (flow.c:2081), the numerator of global.c's allocno_compare priority.\n             * Family: do-while-zero-exception." + SC % 13),
    ("             * borrow is safe in both directions.\n             * Family: staged-value-reused-variable. */",
     "             * borrow is safe in both directions.\n             * Family: staged-value-reused-variable. (island input sum_sq, no copy: score 10) */"),
    ("                 * the LZC arm and the `lut = sum_sq;` island-input copy survives.\n                 * Family: do-while-zero-exception. */",
     "                 * the LZC arm and the `lut = sum_sq;` island-input copy survives.\n                 * Family: do-while-zero-exception." + SC % 10),
    ("                 * each of the three wraps is load-bearing and none subsumes another.\n                 * Family: do-while-zero-exception. */",
     "                 * each of the three wraps is load-bearing and none subsumes another.\n                 * Family: do-while-zero-exception." + SC % 13),
    ("     * written 4-byte object gives ALIGN8(4)+16 = 0x18 != 0x28. */",
     "     * written 4-byte object gives ALIGN8(4)+16 = 0x18 != 0x28. (as s32 lz[1]: score 8) */"),
    ("     * seats it in $t1 as the target does.\n     * Family: staged-value-reused-variable. */",
     "     * seats it in $t1 as the target does.\n     * Family: staged-value-reused-variable. (its own local: score 5) */"),
    ("             * Family: staged-value-reused-variable; same sum-for-byte reuse\n             * as func_8002F2D0 (src/code6cac_b.c). */",
     "             * Family: staged-value-reused-variable; same sum-for-byte reuse\n             * as func_8002F2D0 (src/code6cac_b.c). (fresh shift / byte locals: score 22) */"),
    ("        /* FAKE: same staged len/sum reuse as the loop's LZC arm above. */",
     "        /* FAKE: same staged len/sum reuse as the loop's LZC arm above (fresh shift / byte locals: score 3). */"),
    ("     * lz[2] gives frame 0x68, lz[3]/lz[4] 0x70, lz[5]/lz[6] 0x78,\n     * lz[7]/lz[8] 0x80. */",
     "     * lz[2] gives frame 0x68, lz[3]/lz[4] 0x70, lz[5]/lz[6] 0x78,\n     * lz[7]/lz[8] 0x80. (as s32 lz[2]: score 24) */"),
]

def s87A0(t):
    t = rep(t, COMMENTS)
    if "fa_scr" in OPT or "fa_ac" in OPT:
        t = move_scr(t)
    t = f7FA0(t)
    t = f87F4(t)
    t = f9310(t)
    t = f8094(t)
    t = f8300(t)
    t = rep(t, SCORES)
    t = rep(t, [
        ("extern void func_80018300(s32 *);", "extern void func_80018300(Func80017A44Output *);"),
        ("void func_800187F4(s16 *arg0, s32 *arg1);\nvoid func_80019310(s16 *arg0, s32 *arg1);",
         "void func_800187F4(SceneQuad *arg0, Func80017A44Output *arg1);\n"
         "void func_80019310(SceneQuad *arg0, Func80017A44Output *arg1);"),
    ])
    t = swap_func(t, "void func_8001924C(", F924C)
    return t

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {
        "87A0.c": s87A0(show("src/main/87A0.c")),
        "2B344.c": s2B344(show("src/main/2B344.c")),
        "game.h": game_h(show("include/game.h")),
        "bb2.h": bb2_h(show("include/bb2.h")),
    }
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote q115a", sorted(OPT))
