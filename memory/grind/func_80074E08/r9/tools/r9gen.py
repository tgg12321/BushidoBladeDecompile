"""func_80074E08 Ruling 9: the one-local-per-write spelling and structural respellings of the landing body
memory/grind/func_80074E08/candidate.c, written to r9/variants/ as sandbox candidates. Each variant is
r9/prefix.h (gpu.h + the retyped descriptor under the name EnvB, since the sandbox keeps src's EnvA
typedef) followed by the body with EnvA -> EnvB. usage: python r9gen.py"""
import os, re

H = os.path.dirname(os.path.abspath(__file__))
G = os.path.join(H, "..", "..")
OUT = os.path.join(H, "..", "variants")
prefix = open(os.path.join(H, "..", "prefix.h")).read()
body = open(os.path.join(G, "candidate.c")).read()
body = re.sub(r"    /\* the sprite sheet's cell array.*?\*/\n", "", body, flags=re.S)
body = body.replace("    EnvA s;", "    EnvB s;")
SITE = re.compile(r"( +)s\.header = records\[(\d)\];\n +cells = s\.header \+ 0xC;\n +s\.table = cells;\n")
assert len(SITE.findall(body)) == 4
V = {"reuse": body}


def per_site(fn, drop_decl=True, extra_decl=""):
    b = SITE.sub(lambda m: fn(m[1], m[2]), body)
    if drop_decl:
        b = b.replace("    s32 cells;\n", extra_decl)
    return b


# one local per write (function scope), same statement list
V["pv_fn"] = per_site(lambda i, n: f"{i}s.header = records[{n}];\n{i}cells{n} = s.header + 0xC;\n{i}s.table = cells{n};\n",
                      extra_decl="    s32 cells3;\n    s32 cells2;\n    s32 cells0;\n    s32 cells1;\n")
V["pv_fn_rev"] = per_site(lambda i, n: f"{i}s.header = records[{n}];\n{i}cells{n} = s.header + 0xC;\n{i}s.table = cells{n};\n",
                          extra_decl="    s32 cells1;\n    s32 cells0;\n    s32 cells2;\n    s32 cells3;\n")
# one local per write, each in its own block
V["pv_block"] = per_site(lambda i, n: f"{i}s.header = records[{n}];\n{i}{{\n{i}    s32 cells = s.header + 0xC;\n{i}    s.table = cells;\n{i}}}\n")
# no local
V["nolocal"] = per_site(lambda i, n: f"{i}s.header = records[{n}];\n{i}s.table = s.header + 0xC;\n")
V["nolocal_rec"] = per_site(lambda i, n: f"{i}s.header = records[{n}];\n{i}s.table = records[{n}] + 0xC;\n")
V["nolocal_tablefirst"] = per_site(lambda i, n: f"{i}s.table = records[{n}] + 0xC;\n{i}s.header = records[{n}];\n")
# per-block sheet local
V["hdr_block"] = per_site(lambda i, n: f"{i}{{\n{i}    s32 hdr = records[{n}];\n{i}    s.header = hdr;\n{i}    s.table = hdr + 0xC;\n{i}}}\n")
V["hdr_block_tablefirst"] = per_site(lambda i, n: f"{i}{{\n{i}    s32 hdr = records[{n}];\n{i}    s.table = hdr + 0xC;\n{i}    s.header = hdr;\n{i}}}\n")
V["hdr_cells_block"] = per_site(lambda i, n: f"{i}{{\n{i}    s32 hdr = records[{n}];\n{i}    s32 cells = hdr + 0xC;\n{i}    s.header = hdr;\n{i}    s.table = cells;\n{i}}}\n")
# cells computed from the record before the header store
V["pv_fn_from_rec"] = per_site(lambda i, n: f"{i}cells{n} = records[{n}] + 0xC;\n{i}s.header = records[{n}];\n{i}s.table = cells{n};\n",
                               extra_decl="    s32 cells3;\n    s32 cells2;\n    s32 cells0;\n    s32 cells1;\n")
for k, b in V.items():
    if k != "reuse":
        assert b != body, k
    open(os.path.join(OUT, k + ".c"), "w", newline="\n").write(prefix + b)
print(",".join("memory/grind/func_80074E08/r9/variants/%s.c" % k for k in V))
