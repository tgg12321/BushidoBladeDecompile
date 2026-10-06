#!/usr/bin/env python3
# Lane closing batch: the raw sites the closing census left workable in 51268 / 2B344 / 3AB48 / 6CF8,
# with FZZ batch 2 (../fzz/fzz2.py: g_file_data_buf typed, 6CF8's object record types in game.h,
# func_80017D84 takes its Func80017A44Input, 2B344 func_8003FA24 builds one).
# usage: fclose1.py   writes tmp/p2/fclose1/ (scratch only); BASE_REV env (default f2bcb4406)
import os, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "fdesc"))
sys.path.insert(0, os.path.join(HERE, "..", "fzz"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import fzz2 as Z
sys.argv = _argv
sub1 = Z.sub1
OUT = "tmp/p2/fclose1/"
BASE_REV = os.environ.get("BASE_REV", "f2bcb4406")
FILES = {"51268.c": "src/main/51268.c", "2B344.c": "src/main/2B344.c", "3AB48.c": "src/main/3AB48.c",
         "6CF8.c": "src/main/6CF8.c", "game.h": "include/game.h", "bb2.h": "include/bb2.h",
         "libgpu.h": "include/psxsdk/libgpu.h", "5ED34.c": "src/main/5ED34.c"}

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

def between(t, a, b, new):
    """replace t[a .. b) (a included, b excluded; each marker unique) with new"""
    assert t.count(a) == 1 and t.count(b) == 1, (a[:40], b[:40])
    i = t.index(a)
    j = t.index(b)
    return t[:i] + new + t[j:]

S51268 = [
    # func_80061064: D_800F1150 is this file's u8 array
    ("        if (*((u8 *)&D_800F1150 + i) != 0) {\n", "        if (D_800F1150[i] != 0) {\n"),
    # func_800611A4: arg1 is the caller's three u16 values
    ("void func_800611A4(s32 *arg0, s32 *arg1) {\n", "void func_800611A4(s32 *arg0, u16 *arg1) {\n"),
    ("    svec[0] = *((u16 *) (((s32) arg1) + 0));\n    svec[1] = *((u16 *) (((s32) arg1) + 2));\n",
     "    svec[0] = arg1[0];\n"
     "    /* FAKE: integer-address loads keep the copy behind the D_800A3468 store; arg1[1] / arg1[2] score 15 */\n"
     "    svec[1] = *(u16 *)((s32)arg1 + 2);\n"),
    ("    svec[2] = *((u16 *) (((s32) arg1) + 4));\n", "    svec[2] = *(u16 *)((s32)arg1 + 4);\n"),
    # func_80062020: arg0 is a list of D_800F1198's records
    ("void func_80062020(s32 *arg0) {\n    s32 i;\n    s32 ofs;\n    s32 t;\n    t = *(s32 *)((u8 *)arg0 + 0);\n",
     "void func_80062020(Unk800F1198Record *arg0) {\n    s32 i;\n    s32 t;\n    t = arg0[0].unk0;\n"),
    ("    if ((t & 1) == 0) goto end;\n    ofs = 0;\n    do {\n"
     "        D_800F1198[i].unk0 = *(s32 *)((u8 *)arg0 + ofs + 0);\n"
     "        D_800F1198[i].unk4 = *(s32 *)((u8 *)arg0 + ofs + 4);\n"
     "        D_800F1198[i].unk8 = *(s32 *)((u8 *)arg0 + ofs + 8);\n"
     "        i = i + 1;\n        ofs = ofs + 12;\n        t = *(s32 *)((u8 *)arg0 + ofs + 0);\n",
     "    if ((t & 1) == 0) goto end;\n    do {\n"
     "        D_800F1198[i].unk0 = arg0[i].unk0;\n"
     "        D_800F1198[i].unk4 = arg0[i].unk4;\n"
     "        D_800F1198[i].unk8 = arg0[i].unk8;\n"
     "        i = i + 1;\n        t = arg0[i].unk0;\n"),
    # func_800692C0: D_800A32C8 / D_800A32D0 are the two u32[2] pad-bit mask tables
    ("extern u32 D_800A32D0;\n", "extern u32 D_800A32D0[2];\n"),
    ("        p = &D_800A32D0;\n", "        p = D_800A32D0;\n"),
    ("            s32 idx4;\n            arg3 = (s16 *)((s32)arg3 + a3_off);\n            v = i * 4;\n",
     "            /* FAKE: byte-offset step (arg3 += a3_off with an element count scores 3) */\n"
     "            arg3 = (s16 *)((u8 *)arg3 + a3_off);\n"
     "            v = i * 4; /* FAKE: dead store (v is reloaded below); removed: score 13 */\n"),
    ("            idx4 = v;\n            maskA = *(u32 *)((s32)&D_800A32C8 + idx4) << arg1;\n",
     "            maskA = D_800A32C8[i] << arg1;\n"),
    # func_8006B578 / func_8006B898: arg1 is the pad word (func_8006B92C's u32 *arg1)
    ("s32 func_8006B578(s32 *arg0, s32 *arg1) {\n", "s32 func_8006B578(s32 *arg0, u32 *arg1) {\n"),
    ("    v = *(u32 *)arg1;\n    sp10 = (v & 0xFFFF) | (v >> 16);\n    ret = func_800692C0((u32 *)&sp10",
     "    v = *arg1;\n    sp10 = (v & 0xFFFF) | (v >> 16);\n    ret = func_800692C0((u32 *)&sp10"),
    ("s32 func_8006B898(s32 arg0, s32 arg1) {\n", "s32 func_8006B898(s32 arg0, u32 arg1) {\n"),
]

def s51268(t):
    t = Z.subs(t, S51268)
    i = t.index("s32 func_8006B578(s32 *arg0, u32 *arg1) {\n")
    j = t.index("\n}\n", i)
    body = t[i:j]
    assert body.count("*(u32 *)arg1 & ") == 5, body.count("*(u32 *)arg1 & ")
    t = t[:i] + body.replace("*(u32 *)arg1 & ", "*arg1 & ") + t[j:]
    # B4 sweep (func_8006B898): the frame's GpuDb read ahead of func_8006E390
    i = t.index("s32 func_8006B898(s32 arg0, u32 arg1) {\n")
    j = t.index("\n}\n", i)
    body = sub1(t[i:j], "    t = &g_gpu_db[D_800A36AC & 1];\n",
                "    /* FAKE: the frame's GpuDb taken ahead of func_8006E390; at its use: score 23 */\n"
                "    t = &g_gpu_db[D_800A36AC & 1];\n")
    return t[:i] + body + t[j:]

AFFC = """void func_8003AFFC(void) {
    s32 addr = (s32)0x80190800; /* FAKE: the load address in a local; at the call: score 12 */
    s32 i;

    gpu_ResetGraphMode1();
    func_80020D38();
    func_8004939C();

    for (i = 0; i < 2; i++) {
        func_800493E4(D_80101EC8[i].unk_12);
        func_800494D4(i, D_8008E5CC[D_80101EC8[i].unk_0A][D_80101EC8[i].unk_0E]);
        if (D_80101EC8[i].unk_14 != -1) {
            func_800493E4(D_8008EB80[D_80101EC8[i].unk_14]);
            if (D_80101EC8[i].unk_14 == 14) {
                func_800493E4(D_8008EB80[14] + 3);
            }
        }
    }

    func_80049584(addr);
}
"""

S2B344 = [
    # func_8003AE5C: D_800A36A4 is declared s16; B4 sweep: the address holder labelled
    ("        D_800A37A8[D_800A37A0] = *(u16 *)&D_800A36A4;\n", "        D_800A37A8[D_800A37A0] = D_800A36A4;\n"),
    ("    s32 *addr = (s32 *)0x80190800;\n    s32 result = -1;\n",
     "    s32 *addr = (s32 *)0x80190800; /* FAKE: the load address in a local; at the call: score 11 */\n    s32 result = -1;\n"),
    # func_8003C040: B4 sweep: a0 held the func_8003AF40 argument and then the stage passed to func_8005FBC8;
    # its own local for the second value is byte-identical
    ("    s32 a0;\n    s8 *p;\n    gpu_InitDisplay();\n", "    s32 a0;\n    s32 sel;\n    s8 *p;\n    gpu_InitDisplay();\n"),
    ("    if (D_800A38A4 != 9) {\n        a0 = D_800A38A4;\n    } else {\n        a0 = 8;\n    }\n    func_8005FBC8(a0, ",
     "    if (D_800A38A4 != 9) {\n        sel = D_800A38A4;\n    } else {\n        sel = 8;\n    }\n    func_8005FBC8(sel, "),
    # func_8003C040: D_8008EA70 is a table of (track, channel) s8 pairs
    ("extern s8 D_8008EA70;\n", "extern s8 D_8008EA70[][2];\n"),
    ("    p = (s8 *)(((s8 *)(&D_8008EA70)) + (D_800A38A4 << 1));\n", "    p = D_8008EA70[D_800A38A4];\n"),
    # func_8003D330: D_800A3D30 is a DR_TPAGE pair, one per frame parity
    ("extern u32 D_800A3D30;\n", "extern DR_TPAGE D_800A3D30[2]; /* one per frame parity (D_800A3218) */\n"),
    ("    OTag *p = (OTag *)((u8 *)&D_800A3D30 + (D_800A3218 << 3));\n    OTag *ot;\n    p->len = 1;\n"
     "    *((u32 *)p + 1) = 0xE100001F;\n    ot = (OTag *)g_gpu_ot_ptr;\n    p->addr = ot->addr;\n",
     "    DR_TPAGE *p = &D_800A3D30[D_800A3218];\n    OTag *ot;\n    setlen(p, 1);\n"
     "    p->code[0] = 0xE100001F;\n    ot = (OTag *)g_gpu_ot_ptr;\n    setaddr(p, ot->addr);\n"),
    # func_8003D39C: the SDK macro for the primitive's tag link
    ("    ot = (OTag *)g_gpu_ot_ptr;\n    ((OTag *)p)->addr = ot->addr;\n", "    ot = (OTag *)g_gpu_ot_ptr;\n    setaddr(p, ot->addr);\n"),
    # BitStream: func_8003D774 / func_8003D7B4 / bitstream_ReadBits
    ("    ptr->unk0[0] = arg0;\n    ptr->unk0[1] = 0;\n    ptr->unk0[2] = 0;\n",
     "    ptr->bits.next = (u32 *)arg0;\n    ptr->bits.word = 0;\n    ptr->bits.avail = 0;\n"),
    ("extern s32 bitstream_ReadBits(u32 *, s32);\n", "extern s32 bitstream_ReadBits(BitStream *, s32);\n"),
    ("        nbits = bitstream_ReadBits(base->unk0, 4);\n", "        nbits = bitstream_ReadBits(&base->bits, 4);\n"),
    ("        val = (s16)bitstream_ReadBits(base->unk0, nbits);\n", "        val = (s16)bitstream_ReadBits(&base->bits, nbits);\n"),
    ("/* Bitstream reader.  State through `u32 *s`: s[0]=word pointer, s[1]=current word,\n"
     "   s[2]=bits still available in s[1].  Returns the next `n` bits.\n",
     "/* Bitstream reader: returns the next `n` bits of the BitStream `s`, loading the next word\n"
     "   when the current one has fewer than `n` unread bits.\n"),
    ("s32 bitstream_ReadBits(u32 *s, s32 n)\n", "s32 bitstream_ReadBits(BitStream *s, s32 n)\n"),
    ("    s32 avail = s[2];\n", "    s32 avail = s->avail;\n"),
    ("        u32 p;\n", "        u32 *wp;\n        u32 w;\n"),
    ("        r = s[1] & m1;\n        p = s[0];\n        s[0] = p + 4;\n",
     "        r = s->word & m1;\n        wp = s->next;\n        s->next = wp + 1;\n"),
    ("        p = *(u32 *)p;\n        s[2] = shift;\n        s[1] = p;\n        hi = ((u32)p >> shift) & m2;\n",
     "        w = *wp;\n        s->avail = shift;\n        s->word = w;\n        hi = (w >> shift) & m2;\n"),
    ("        s[2] = avail - n;\n        r = (s[1] >> (avail - n)) & ((1 << n) - 1);\n",
     "        s->avail = avail - n;\n        r = (s->word >> (avail - n)) & ((1 << n) - 1);\n"),
    # func_8003E2AC: D_800F6656 is declared s16
    ("    u16 *p = (u16 *)&D_800F6656;\n    *p = *p & 0xFFFD;\n",
     "    /* FAKE: the flag word's address held in p (one lui / addiu, lhu / sh off it); `D_800F6656 &= ~2` scores 6 */\n"
     "    s16 *p = &D_800F6656;\n    *p &= ~2;\n"),
]

S3AB48 = [
    ("    /* The next vertex's address is spelled as the integer sum, index first: every pointer\n",
     "    /* FAKE (score 4 as arg0->vtx[(s16)next_idx]): the next vertex's address is spelled as the\n"
     "     * integer sum, index first: every pointer\n"),
]

S5ED34 = [
    ("    s32 state = *(u8 *)&D_800A3578;  /* entry dispatch reads low byte only -> lbu */\n",
     "    s32 state = D_800A3578 & 0xFF;\n"),
    # B4 sweep: the staged D_800A3584 read
    ("        s16 v3584 = D_800A3584;\n",
     "        s16 v3584 = D_800A3584; /* FAKE: read ahead of the D_800A3570 store; at its use: score 2 */\n"),
]

S6CF8 = [
    ("extern u8 D_800A30D4;\n", "extern Rect D_800A30D4;\n"),
    ("    rect = *(Rect *)&D_800A30D4;\n", "    rect = D_800A30D4;\n"),
]

BITSTREAM_OLD = """/* 0x800A3D40: 24-byte records func_8003D774 starts and func_8003D7B4 advances: the bit stream
   bitstream_ReadBits reads (unk0, its u32 words) and six s16 values func_8003D7B4 adds the decoded
   deltas to and returns (3AB48 func_8005490C reads them as an offset and a rotation). */
typedef struct {
    u32 unk0[3];
    s16 unkC[6];
} Unk800A3D40Rec;
"""
BITSTREAM_NEW = """/* The bit stream bitstream_ReadBits reads, most significant bit first: the next u32 word to load,
   the word being read and how many of its low bits are still unread. */
typedef struct {
    u32 *next;
    u32 word;
    s32 avail;
} BitStream;

/* 0x800A3D40: 24-byte records func_8003D774 starts and func_8003D7B4 advances: the bit stream
   bitstream_ReadBits reads and six s16 values func_8003D7B4 adds the decoded deltas to and
   returns (3AB48 func_8005490C reads them as an offset and a rotation). */
typedef struct {
    BitStream bits;
    s16 unkC[6];
} Unk800A3D40Rec;
"""

SBB2 = [
    ("extern s32 D_800A32C8;\n", "extern u32 D_800A32C8[2];\n"),
    ("extern s32 func_8006B898(s32, s32);\n", "extern s32 func_8006B898(s32, u32);\n"),
]

SGPU = [
    ("typedef struct { u32 tag; u32 code[2]; } DR_MODE;   /* Drawing Mode */\n",
     "typedef struct { u32 tag; u32 code[2]; } DR_MODE;   /* Drawing Mode */\n"
     "typedef struct { u32 tag; u32 code[1]; } DR_TPAGE;\n"),   # worker 2's batch 5 spelling, byte for byte
]

# ---------------------------------------------------------------- rev-fclose1's fixes (measured)
OLD_HI = """        /* FAKE: `hi` names the masked high-bit slice of the freshly loaded word,
           mechanism: GCC 2.7.2 RTL expansion (expr.c expand_binop) fixes the iorsi3
           source-operand order from the C expression tree and combine preserves it --
           naming the slice moves it to operand 1 (`or v1,v1,v0`, target) without
           touching statement order, so sched1's order and the greg allocation are
           byte-identical to the un-named form, whereas swapping the operands in the
           source expression itself also moves the LUID and regresses the
           allocation. */
"""
NEW_HI = """        /* FAKE: `hi` names the masked high-bit slice of the freshly loaded word; written into
           the or (`r = (r << n) | ((w >> shift) & m2)`): score 17 -- s and n swap argument
           registers (move a3,a0 / move a2,a1 for the target's move a2,a0 / move a3,a1). */
"""

def rev1(out):
    out["2B344.c"] = Z.subs(out["2B344.c"], [
        ("    s32 a0;\n    s32 sel;\n",
         "    s32 a0; /* FAKE: func_8003AF40's 0 held in a local set on each dispatch path (move a0,zero in each branch's delay slot); the literal at the call scores 5 */\n    s32 sel;\n"),
        (OLD_HI, NEW_HI),
        ("        sval = val;\n",
         "        sval = val; /* FAKE: the sign test reads an s32 copy of val; tested as val: score 2 (addiu a1,s0,-1 moves two slots) */\n"),
        ("an unwritten\n       three-word tail; the argument alone gives a frame 8 short (0x48 for 0x50): score 16. */\n",
         "an unwritten\n       tail (two or three words are byte-identical: three chosen); the argument alone gives a frame\n"
         "       8 short (0x48 for 0x50): score 16. */\n"),
    ])
    out["6CF8.c"] = Z.subs(out["6CF8.c"], [
        ("                s32 value = (s32)(pixel & 0x1F) >> 1;\n",
         "                /* FAKE: (s32) makes the shift signed; unsigned: score 1 (srl for the target's sra) */\n"
         "                s32 value = (s32)(pixel & 0x1F) >> 1;\n"),
        ("                pixel &= 0xFFFF;\n",
         "                pixel &= 0xFFFF; /* FAKE: no-op mask of the u16 read; removed: score 1 (the andi goes) */\n"),
        ("    u8 *c;\n", "    Func80017A44Record *c;\n"),
        ("    c = a0->buf;\n", "    c = (Func80017A44Record *)a0->buf;\n"),
        ("    p->records = (Func80017A44Record *)c;\n    p->edges = (Func80017848Edge *)((Func80017A44Record *)c + p->count);\n",
         "    p->records = c;\n    p->edges = (Func80017848Edge *)(c + p->count);\n"),
    ])
    t = out["51268.c"]
    i = t.index("s32 func_800692C0(u32 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {")
    j = t.index("\n}\n", i)
    b = Z.subs(t[i:j], [
        ("    s32 c;\n", ""),
        ("                    c = -1;\n                    *arg3 = c;\n", "                    *arg3 = -1;\n"),
        ("                    c = 6;\n                    *arg2 = c;\n", "                    *arg2 = 6;\n"),
        ("                    c = -6;\n                    *arg3 = c;\n", "                    *arg3 = -6;\n"),
        ("                    c = 2;\n                    sum += c << bitpos;\n", "                    sum += 2 << bitpos;\n"),
        ("            v = i * 4; /* FAKE: dead store (v is reloaded below); removed: score 13 */\n",
         "            v = i * 4; /* FAKE: dead store -- v is reassigned from *arg0 before any read; removed, loop.c strength-reduces D_800A32C8[i] to a walking pointer (lui/addiu t3, addiu t3,t3,4) instead of the target's sll / addu indexed load: score 13 */\n"),
    ])
    t = t[:i] + b + t[j:]
    out["51268.c"] = Z.subs(t, [
        ("    /* FAKE: integer-address loads keep the copy behind the D_800A3468 store; arg1[1] / arg1[2] score 15 */\n",
         "    /* FAKE: integer-address loads keep the arg1[1] load ahead of the D_800A3468 / D_800F1178 stores; arg1[1] / arg1[2] score 15 */\n"),
        ("    ((void (*)())func_8006B120)(sp10);\n", "    func_8006B120(sp10);\n"),
    ])
    out["libgpu.h"] = Z.subs(out["libgpu.h"], [
        ("/* PsyQ LIBGPU.H drawing-environment primitives: the OT tag word, then two GPU command words. */",
         "/* PsyQ LIBGPU.H drawing-environment primitives: the OT tag word, then its GPU command words. */"),
        ("typedef struct { u32 tag; u32 code[1]; } DR_TPAGE;\n", "typedef struct { u32 tag; u32 code[1]; } DR_TPAGE;  /* Drawing TPage */\n"),
    ])
    return out

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {k: show(v) for k, v in FILES.items()}
    out["6CF8.c"], blks = Z.s6CF8(out["6CF8.c"])
    out["game.h"] = Z.game(out["game.h"], blks)
    out["bb2.h"] = Z.bb2(out["bb2.h"])
    out["51268.c"] = s51268(out["51268.c"])
    t = between(Z.s2B344(out["2B344.c"]),"void func_8003AFFC(void) {\n", "void func_8003B10C(s32 arg0) {\n", AFFC)
    out["2B344.c"] = Z.subs(t, S2B344)
    out["3AB48.c"] = Z.subs(out["3AB48.c"], S3AB48)
    out["6CF8.c"] = Z.subs(out["6CF8.c"], S6CF8)
    out["5ED34.c"] = Z.subs(out["5ED34.c"], S5ED34)
    out["game.h"] = sub1(out["game.h"], BITSTREAM_OLD, BITSTREAM_NEW)
    out["bb2.h"] = Z.subs(out["bb2.h"], SBB2)
    out["libgpu.h"] = Z.subs(out["libgpu.h"], SGPU)
    out = rev1(out)
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote fclose1")
