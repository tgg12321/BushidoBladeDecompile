"""Ablation variants on w2 (= w1 + byte-neutral dist2 split) for the Ruling 11 (D)(4) record.
For a variable whose writes are split into values, a PARTITION maps each value to a name;
values mapped to the original name stay shared. Fresh names are declared at the innermost
scope enclosing their writes. Writes tmp/func_800187F4/<tag>.c; prints the tags."""
import itertools
import re
import sys

D = "tmp/func_800187F4/"
W1 = open(D + "w1.c").read()


def once(s, old, new):
    assert s.count(old) == 1, (old, s.count(old))
    return s.replace(old, new)


# w2 = w1 with dist2 split (sq2 = squared length, dist2 = root/scale), both block-scoped
s = W1.replace("    s32 dist1, dist2, tot, pen;\n", "    s32 dist1, tot, pen;\n")
s = once(s, "                dist2 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n",
         "                sq2 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n")
s = once(s, "                if (dist2 < 0x400) {\n                    dist2 = (&D_8008D118)[dist2] >> 3;\n",
         "                if (sq2 < 0x400) {\n                    dist2 = (&D_8008D118)[sq2] >> 3;\n")
s = once(s, "@gte_Lzc(dist2, &lz[1]);", "@gte_Lzc(sq2, &lz[1]);")
s = once(s, "(&D_8008D118)[dist2 >> shift2]", "(&D_8008D118)[sq2 >> shift2]")
s = once(s, "            for (j = 0; j < SCR->nsph; j++) {\n",
         "            for (j = 0; j < SCR->nsph; j++) {\n                s32 sq2, dist2;\n\n")
W2 = s
SPH = "            for (j = 0; j < SCR->nsph; j++) {\n                s32 sq2, dist2;\n\n"
NODE = "    for (i = 0; i < count; i++, node += 16) {\n"
COLL = "        if (*(s32 *)(arg0 + 0xC) != 0) {\n"


def add_decl(s, anchor, names):
    if names:
        s = once(s, anchor, anchor + "".join(f"{' ' * (len(anchor) - len(anchor.lstrip()) + 4)}s32 {n};\n" for n in names) + "\n")
    return s


def part_d(names):
    """names: 7 entries (ground, dy0, dx0, dz0, dy1, dx1, dz1); 'd' = shared."""
    s = W2
    chunks = re.split(r"(\bd = )", s)
    assert len(chunks) == 15
    out = chunks[0]
    for k in range(7):
        out += names[k] + " = " + re.sub(r"\bd\b", names[k], chunks[2 + 2 * k])
    if "d" not in names:
        out = once(out, "    s32 r, d;\n", "    s32 r;\n")
    out = add_decl(out, COLL, [n for n in names[:1] if n != "d"])
    out = add_decl(out, SPH, [n for n in names[1:] if n != "d"])
    return out


def part_j(names):
    s = W2
    parts = s.split("for (j = 0; j < ")
    assert len(parts) == 4
    out = parts[0]
    for k in range(3):
        seg = parts[k + 1] if names[k] == "j" else re.sub(r"\bj\b", names[k], parts[k + 1])
        out += f"for ({names[k]} = 0; {names[k]} < " + seg
    if "j" not in names:
        out = once(out, "    s32 i, j, n;\n", "    s32 i, n;\n")
    out = add_decl(out, NODE, [n for n in names[:2] if n != "j"])
    out = add_decl(out, COLL, [n for n in names[2:] if n != "j"])
    return out


def part_byte(names):
    """names: (copy, lut1, lut2)."""
    s = W2
    c, l1, l2 = names
    s = once(s, "                byte = dist1;\n", f"                {c} = dist1;\n")
    s = once(s, "@gte_Lzc(byte, &lz[0]);", f"@gte_Lzc({c}, &lz[0]);")
    s = once(s, "                    byte = (&D_8008D118)[dist1 >> shift];\n",
             f"                    {l1} = (&D_8008D118)[dist1 >> shift];\n")
    s = once(s, "(byte << 16) >> (0x13 - (shift >> 1))", f"({l1} << 16) >> (0x13 - (shift >> 1))")
    s = once(s, "                    byte = (&D_8008D118)[sq2 >> shift2];\n",
             f"                    {l2} = (&D_8008D118)[sq2 >> shift2];\n")
    s = once(s, "(byte << 16) >> (0x13 - (shift2 >> 1))", f"({l2} << 16) >> (0x13 - (shift2 >> 1))")
    if "byte" not in names:
        s = once(s, "    s32 byte;\n", "")
    # all byte values sit inside the sphere loop body; declare fresh ones there
    s = add_decl(s, SPH, [n for n in names if n != "byte"])
    return s


def part_n(names):
    s = W2
    a, b = names
    s = once(s, "        n = node[7];\n", f"        {a} = node[7];\n")
    s = s.replace("        for (j = 0; j < n; j++) {\n", "@@LOOP_A@@", 1)
    s = once(s, "        n = node[8];\n", f"        {b} = node[8];\n")
    s = once(s, "        for (j = 0; j < n; j++) {\n", f"        for (j = 0; j < {b}; j++) {{\n")
    s = once(s, "@@LOOP_A@@", f"        for (j = 0; j < {a}; j++) {{\n")
    if "n" not in names:
        s = once(s, "    s32 i, j, n;\n", "    s32 i, j;\n")
    return add_decl(s, NODE, [n for n in names if n != "n"])


def part_dist1(names):
    sq, rt = names
    s = W2
    s = once(s, "                dist1 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n",
             f"                {sq} = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n")
    s = once(s, "                byte = dist1;\n", f"                byte = {sq};\n")
    s = once(s, "                if (dist1 < 0x400) {\n                    dist1 = (&D_8008D118)[dist1] >> 3;\n",
             f"                if ({sq} < 0x400) {{\n                    {rt} = (&D_8008D118)[{sq}] >> 3;\n")
    s = once(s, "(&D_8008D118)[dist1 >> shift]", f"(&D_8008D118)[{sq} >> shift]")
    # root/scale value: every later dist1
    head, tail = s.split("(&D_8008D118)[" + sq + " >> shift];\n", 1)
    tail = re.sub(r"\bdist1\b", rt, tail)
    s = head + "(&D_8008D118)[" + sq + " >> shift];\n" + tail
    if "dist1" not in names:
        s = once(s, "    s32 dist1, tot, pen;\n", "    s32 tot, pen;\n")
    return add_decl(s, SPH, [n for n in names if n != "dist1"])


GEN = {"d": (part_d, ["dg", "dy0", "dx0", "dz0", "dy1", "dx1", "dz1"], "d"),
       "j": (part_j, ["j_add", "j_sub", "j_sph"], "j"),
       "byte": (part_byte, ["lzc_in", "byte1", "byte2"], "byte"),
       "n": (part_n, ["n_add", "n_sub"], "n"),
       "dist1": (part_dist1, ["sq1", "dist1r"], "dist1")}

if __name__ == "__main__":
    open(D + "w2.c", "w", newline="\n").write(W2)
    tags = ["w2"]
    for var in sys.argv[1:]:
        fn, fresh, shared = GEN[var]
        k = len(fresh)
        # every partition where each value is either shared or split alone (2^k masks)
        for mask in range(1, 2 ** k):
            names = [fresh[i] if mask >> i & 1 else shared for i in range(k)]
            tag = f"abl_{var}_{mask:0{k}b}"
            open(D + tag + ".c", "w", newline="\n").write(fn(names))
            tags.append(tag)
    print(" ".join(tags))
