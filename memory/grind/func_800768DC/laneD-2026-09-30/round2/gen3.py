"""Round-2 generator: SelWork + D_8009BD20 into include/game.h, every C consumer through members.
Inputs: tmp/laneD/landing_A.c (round-1 staged tu2), HEAD text1b_tu1e.c, HEAD game.h (copies in tmp/laneD/).
usage: python3 tmp/laneD/gen3.py <outdir> [flags]"""
import sys, re, os

outdir = sys.argv[1]
flags = set(sys.argv[2:])
os.makedirs(outdir, exist_ok=True)
tu2 = open("tmp/laneD/landing_A.c", encoding="utf-8").read()
tu1e = open("tmp/laneD/head_tu1e.c", encoding="utf-8").read()
hdr = open("tmp/laneD/head_game.h", encoding="utf-8").read()


def rep(src, old, new, count=1):
    n = src.count(old)
    if n != count:
        raise SystemExit(f"expected {count} of {old!r}, found {n}")
    return src.replace(old, new)


def body(fn, variant=None):
    v = [f.split("=", 1)[1] for f in flags if f.startswith(fn + "=")]
    p = f"tmp/laneD/c_{fn}{'_' + v[0] if v else ''}.c"
    return open(p, encoding="utf-8").read()


def swap(src, fn):
    m = re.search(r"\n[a-z0-9 ]+%s\([^)]*\) \{.*?\n\}\n" % fn, src, re.S)
    return src[:m.start() + 1] + body(fn) + src[m.end():]


# ---- game.h: D_8009BD20, D_800A36A0, SelWork after the D_8009BCF8 declaration ----
hdr = rep(hdr, "extern Unk8009BCF8Record D_8009BCF8[20];\n",
          "extern Unk8009BCF8Record D_8009BCF8[20];\n" + open("tmp/laneD/struct_G.snip", encoding="utf-8").read())

# ---- text1b_tu2.c ----
m = re.search(r"/\* The select-screen work area D_800A36A0 points at.*?#define SELWORK \(\(SelWork \*\)D_800A36A0\)\n\n", tu2, re.S)
tu2 = tu2[:m.start()] + tu2[m.end():]
tu2 = rep(tu2, "extern u8 D_8009BD20[][2];\n", "")
tu2 = rep(tu2, "extern u8 D_8009BD21;\n", "")
tu2 = re.sub(r"extern u8 \*D_800A36A0;\n", "", tu2)
# func_800747D8: dead goto before its label; goto tail -> break
tu2 = rep(tu2, "        goto confirm;\nconfirm:\n", "confirm:\n")
tu2 = rep(tu2, "        goto tail;\n", "        break;\n", 2)
tu2 = rep(tu2, "        }\ntail:\n        if (input & 0x100010) {", "        }\n        if (input & 0x100010) {")
for fn in ("func_80074B18", "func_800753D8", "func_80076D74", "func_80077724"):
    if os.path.exists(f"tmp/laneD/c_{fn}.c"):
        tu2 = swap(tu2, fn)

# ---- text1b_tu1e.c ----
tu1e = rep(tu1e, "extern u8 D_8009BD20[][2];\nextern u8 *D_800A36A0;\n", "")
if os.path.exists("tmp/laneD/c_func_80074488.c"):
    tu1e = swap(tu1e, "func_80074488")

assert "D_800A36A0 +" not in tu2 and "D_800A36A0 +" not in tu1e
for name, txt in (("text1b_tu2.c", tu2), ("text1b_tu1e.c", tu1e), ("game.h", hdr)):
    open(os.path.join(outdir, name), "w", encoding="utf-8", newline="\n").write(txt)
