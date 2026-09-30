"""FAKE ablations on tmp/D780/v2b.c -> tmp/D780/ab_*.c"""
import re
from pathlib import Path

base = Path("tmp/D780/v2b.c").read_bytes().decode()


def strip_fake_comment_before(t, stmt):
    """remove a /* FAKE ... */ comment that immediately precedes `stmt` (same indentation)."""
    i = t.index(stmt)
    j = t.rindex("/* FAKE", 0, i)
    k = t.index("*/", j) + 2
    assert t[k:i].strip() == "", (stmt, t[k:i][:80])
    line_start = t.rindex("\n", 0, j) + 1
    return t[:line_start] + t[k:].lstrip("\n").join([""]) if False else t[:line_start] + t[i - (i - t.rindex("\n", 0, i) - 1):]


def no_flag(t):
    t = strip_fake_comment_before(t, "flag = z2 - z0;")
    t = t.replace("                flag = z2 - z0;\n", "                dz = z2 - z0;\n")
    t = t.replace("                s32 dx;\n", "                s32 dz;\n                s32 dx;\n")
    t = t.replace("kc = (flag * ax)", "kc = (dz * ax)").replace("kp = (flag * (px - x0))", "kp = (dz * (px - x0))")
    assert "flag *" not in t and "flag = z2" not in t
    return t


def no_ax(t):
    t = strip_fake_comment_before(t, "ax = pz - z0;")
    t = t.replace("                ax = pz - z0;\n", "                bz = pz - z0;\n")
    t = t.replace("                s32 az;\n", "                s32 az;\n                s32 bz;\n")
    t = t.replace("- (dx * ax);\n                if ((kc ^ kp)", "- (dx * bz);\n                if ((kc ^ kp)")
    assert "dx * bz" in t
    return t


def no_m(t):
    t = strip_fake_comment_before(t, "m = dist;\n                /* gte_Lzc")
    t = t.replace("                m = dist;\n                /* gte_Lzc", "                /* gte_Lzc")
    return t


vs = {"noflag": no_flag(base), "noax": no_ax(base), "nom": no_m(base)}
vs["none"] = no_m(no_ax(no_flag(base)))
vs["noflag_noax"] = no_ax(no_flag(base))
for k, v in vs.items():
    Path(f"tmp/D780/ab_{k}.c").write_bytes(v.encode())
print("ok", list(vs))
