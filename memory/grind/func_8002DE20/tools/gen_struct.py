"""Structural respellings of the same-side tests, derived from r11_cand.c.

  s_inline.c  : no variables, `if (((EA) ^ (EB)) >= 0)` per test
  s_helper.c  : static inline cross product helper, per-test fresh pair
                (cross_a = cross2(ex, ey, px, py) ...), one write each
  s_helper_inline.c : helper, no variables (helper calls inside the test)
  s_group.c   : one pair per group, declared in a block around the group
"""
import re

D = "tmp/func_8002DE20/"
src = open(D + "r11_cand.c", newline="").read()
lines = src.split("\n")


def split_top(s, sep):
    out, depth, cur, i = [], 0, "", 0
    while i < len(s):
        if s[i] == "(":
            depth += 1
        elif s[i] == ")":
            depth -= 1
        if depth == 0 and s.startswith(sep, i):
            out.append(cur)
            cur = ""
            i += len(sep)
            continue
        cur += s[i]
        i += 1
    out.append(cur)
    return out


def strip_paren(s):
    s = s.strip()
    if s.startswith("(") and s.endswith(")"):
        depth = 0
        for i, c in enumerate(s):
            depth += c == "("
            depth -= c == ")"
            if depth == 0 and i < len(s) - 1:
                return s
        return s[1:-1]
    return s


def helper_call(expr):
    p, q = split_top(expr, " - ")
    ey, px = split_top(p, " * ")
    ex, py = split_top(q, " * ")
    return f"cross2({strip_paren(ex)}, {strip_paren(ey)}, {strip_paren(px)}, {strip_paren(py)})"


HELPER = """static inline s32 cross2(s32 ex, s32 ey, s32 px, s32 py)
{
    return ey * px - ex * py;
}

"""

wr = re.compile(r"^( *)cross_([ab]) = (.*);$")


def variant(mode):
    out, i = [], 0
    while i < len(lines):
        m1 = wr.match(lines[i])
        if m1 and m1.group(2) == "a":
            m2 = wr.match(lines[i + 1])
            t = lines[i + 2]
            assert m2 and "(cross_a ^ cross_b)" in t
            ind, ea, eb = m1.group(1), m1.group(3), m2.group(3)
            if mode == "inline":
                out.append(t.replace("(cross_a ^ cross_b)", f"(({ea}) ^ ({eb}))"))
            elif mode == "helper_inline":
                out.append(t.replace("(cross_a ^ cross_b)",
                                     f"({helper_call(ea)} ^ {helper_call(eb)})"))
            elif mode == "helper":
                out.append(f"{ind}cross_a = {helper_call(ea)};")
                out.append(f"{ind}cross_b = {helper_call(eb)};")
                out.append(t)
            i += 3
            continue
        out.append(lines[i])
        i += 1
    s = "\n".join(out)
    if mode in ("inline", "helper_inline"):
        s = re.sub(r"    /\* cross_a / cross_b each hold.*?\*/\n    s32 cross_a;\n    s32 cross_b;\n",
                   "", s, flags=re.S)
    if mode.startswith("helper"):
        k = s.index("s32 func_8002DE20(")
        s = s[:k] + HELPER + s[k:]
    return s


for mode in ("inline", "helper_inline", "helper"):
    s = variant(mode)
    if mode == "helper":
        # per-test fresh pair: reuse gen_pv's split on the helper body
        open(D + "s_helper_shared.c", "w", newline="\n").write(s)
    else:
        open(D + f"s_{mode}.c", "w", newline="\n").write(s)
print("ok")
