"""Ruling 11 (C)(1)/(D)(4) spellings on the landing chassis c4 (tmp/func_800187F4/c4.c).

Seven reused locals, each value named; a value keeping the shared name stays shared. Fresh
locals are declared at the innermost scope enclosing their writes (the reuse variable's own
declaration is dropped when no value keeps it). Statements never change: only declarations and
identifiers. Tags written to tmp/func_800187F4/:
  r11pv_<var>          every value of <var> in its own local, the other variables unchanged
  r11abl_<var>_<name>  one value split out alone, the rest still shared (vars with >= 3 values)
  r11pv_all            all seven split at once
Run from the repo root; prints the tags."""
import re
import sys

D = "tmp/func_800187F4/"
import os
C4 = open(D + os.environ.get("R11BASE", "c4") + ".c").read()
NODE_DECL = "        s32 idx;\n        s32 nforce;\n\n"
COLL_DECL = "            s32 delta;\n\n"
SPH_DECL = "                s32 temp;\n                s32 work;\n"
LZ1_DECL = "                    s32 nbits;\n\n"
LZ2_DECL = "                    s32 nbits2;\n\n"


def once(s, old, new):
    assert s.count(old) == 1, (old[:70], s.count(old))
    return s.replace(old, new)


def redecl(s, anchor_old, var, fresh, indent):
    """Replace `s32 var;` inside anchor_old by the fresh names (keep var if still used)."""
    line = f"{' ' * indent}s32 {var};\n"
    assert line in anchor_old, (var, anchor_old)
    keep = re.search(rf"\b{var}\b", s.replace(anchor_old, "")) is not None
    new_lines = (line if keep else "") + "".join(f"{' ' * indent}s32 {n};\n" for n in fresh)
    return once(s, anchor_old, anchor_old.replace(line, new_lines))


def rename_in(seg, var, new):
    return seg if new == var else re.sub(rf"\b{var}\b", new, seg)


def part_idx(s, names):
    parts = s.split("for (idx = 0; idx < ")
    assert len(parts) == 4, len(parts)
    out = parts[0]
    for k in range(3):
        out += f"for ({names[k]} = 0; {names[k]} < " + rename_in(parts[k + 1], "idx", names[k])
    fresh_node = [n for n in names[:2] if n != "idx"]
    out = once(out, "        s32 idx;\n", ("        s32 idx;\n" if "idx" in names else "")
               + "".join(f"        s32 {n};\n" for n in fresh_node))
    if names[2] != "idx":
        out = once(out, "            s32 delta;\n" if "            s32 delta;\n" in out else "        if (*(s32 *)((u8 *)arg0 + 0xC) != 0) {\n",
                   ("            s32 delta;\n" if "            s32 delta;\n" in out else "        if (*(s32 *)((u8 *)arg0 + 0xC) != 0) {\n")
                   + f"            s32 {names[2]};\n")
    return out


def part_nforce(s, names):
    a, b = names
    s = once(s, "        nforce = node[7];\n", f"        {a} = node[7];\n")
    s = once(s, "        nforce = node[8];\n", f"        {b} = node[8];\n")
    hdr = re.compile(r"(for \((\w+) = 0; \2 < )nforce(; \2\+\+\) \{)")
    assert len(hdr.findall(s)) == 2
    s = hdr.sub(lambda m: m.group(1) + a + m.group(3), s, count=1)
    s = hdr.sub(lambda m: m.group(1) + b + m.group(3), s, count=1)
    fresh = [n for n in names if n != "nforce"]
    return once(s, "        s32 nforce;\n", "".join(f"        s32 {n};\n" for n in fresh)) if "nforce" not in names \
        else once(s, "        s32 nforce;\n", "        s32 nforce;\n" + "".join(f"        s32 {n};\n" for n in fresh))


def part_temp(s, names):
    c, l1, l2 = names
    s = once(s, "                temp = sq1;\n", f"                {c} = sq1;\n")
    s = once(s, "@gte_Lzc(temp, &lz[0]);", f"@gte_Lzc({c}, &lz[0]);")
    s = once(s, "                    temp = (&D_8008D118)[sq1 >> nbits];\n",
             f"                    {l1} = (&D_8008D118)[sq1 >> nbits];\n")
    s = once(s, "(temp << 16) >> (0x13 - (nbits >> 1))", f"({l1} << 16) >> (0x13 - (nbits >> 1))")
    s = once(s, "                    temp = (&D_8008D118)[sq2 >> nbits2];\n",
             f"                    {l2} = (&D_8008D118)[sq2 >> nbits2];\n")
    s = once(s, "(temp << 16) >> (0x13 - (nbits2 >> 1))", f"({l2} << 16) >> (0x13 - (nbits2 >> 1))")
    if "temp" not in names:
        s = once(s, "                s32 temp;\n", "")
    if c != "temp":
        s = once(s, "                s32 sq1, dist1;\n", f"                s32 sq1, dist1;\n                s32 {c};\n")
    if l1 != "temp":
        s = once(s, "                    s32 nbits;\n", f"                    s32 nbits;\n                    s32 {l1};\n")
    if l2 != "temp":
        s = once(s, "                    s32 nbits2;\n", f"                    s32 nbits2;\n                    s32 {l2};\n")
    return s


def part_work(s, names):
    sq, rt = names
    s = once(s, "                work = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n",
             f"                {sq} = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n")
    s = re.sub(r"(\n\s+\w+ = )work;\n", rf"\g<1>{sq};\n", s, count=1)   # the LZC-input copy
    s = once(s, "                if (work < 0x400) {\n                    work = (&D_8008D118)[work] >> 3;\n",
             f"                if ({sq} < 0x400) {{\n                    {rt} = (&D_8008D118)[{sq}] >> 3;\n")
    s = once(s, "(&D_8008D118)[sq1 >> nbits]", f"(&D_8008D118)[{sq} >> nbits]")
    head, tail = s.split(f"(&D_8008D118)[{sq} >> nbits];\n", 1)
    s = head + f"(&D_8008D118)[{sq} >> nbits];\n" + rename_in(tail, "work", rt)
    fresh = [n for n in names if n != "work"]
    return once(s, "                s32 work;\n", ("" if "work" not in names else "                s32 work;\n")
                + "".join(f"                s32 {n};\n" for n in fresh))


def part_delta(s, names):
    g, y0 = names
    chunks = re.split(r"(\bdelta = )", s)
    assert len(chunks) == 5, len(chunks)
    out = chunks[0]
    for k, n in enumerate(names):
        out += n + " = " + rename_in(chunks[2 + 2 * k], "delta", n)
    if "delta" not in names:
        out = once(out, "            s32 delta;\n", "")
    if g != "delta":
        out = once(out, "\n\n            " + g + " = SCR->pos[1]", f"\n            s32 {g};\n\n            {g} = SCR->pos[1]") \
            if f"\n\n            {g} = SCR->pos[1]" in out else out
    if y0 != "delta":
        out = once(out, "                s32 work;\n", f"                s32 work;\n                s32 {y0};\n") \
            if "                s32 work;\n" in out else once(out, "                s32 sq2, dist2;\n",
                                                             f"                s32 sq2, dist2;\n                s32 {y0};\n")
    return out


def part_nbits(s, names, var="nbits", lz="lz[0]"):
    cnt, sh = names
    s = once(s, f"                    {var} = {lz};\n", f"                    {cnt} = {lz};\n")
    s = once(s, f"                    {var} = 0x16 - ({var} & ~1);\n", f"                    {sh} = 0x16 - ({cnt} & ~1);\n")
    head, tail = s.split(f"                    {sh} = 0x16 - ({cnt} & ~1);\n", 1)
    tail_end = tail.index("                }\n")
    tail = rename_in(tail[:tail_end], var, sh) + tail[tail_end:]
    s = head + f"                    {sh} = 0x16 - ({cnt} & ~1);\n" + tail
    fresh = [n for n in names if n != var]
    decl = f"                    s32 {var};\n"
    return once(s, decl, ("" if var not in names else decl) + "".join(f"                    s32 {n};\n" for n in fresh))


def part_nbits2(s, names):
    return part_nbits(s, names, "nbits2", "lz[1]")


VARS = {"idx": (part_idx, ["idx_add", "idx_sub", "idx_sph"]),
        "nforce": (part_nforce, ["nforce_add", "nforce_sub"]),
        "temp": (part_temp, ["lzc_in", "lut1", "lut2"]),
        "nbits": (part_nbits, ["lzcount", "shift"]),
        "nbits2": (part_nbits2, ["lzcount2", "shift2"])}

if __name__ == "__main__":
    tags = []
    for var, (fn, fresh) in VARS.items():
        open(D + f"r11pv_{var}.c", "w", newline="\n").write(fn(C4, fresh))
        tags.append(f"r11pv_{var}")
        if len(fresh) >= 3:
            for i in range(len(fresh)):
                names = [fresh[j] if j == i else var for j in range(len(fresh))]
                tag = f"r11abl_{var}_{fresh[i]}"
                open(D + tag + ".c", "w", newline="\n").write(fn(C4, names))
                tags.append(tag)
    s = C4
    for var, (fn, fresh) in VARS.items():
        s = fn(s, fresh)
    open(D + "r11pv_all.c", "w", newline="\n").write(s)
    tags.append("r11pv_all")
    print(" ".join(tags))
