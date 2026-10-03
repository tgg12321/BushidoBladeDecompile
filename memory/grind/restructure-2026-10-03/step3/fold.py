# Restructure step-3 folds as applied 2026-10-03 (run from the repo root, one per commit):
# python fold.py <which>   (ings | memcard | tu1b | mid)
# Edits the owner .c, removes the data file (git rm), rewrites bb2.ld, oracle manifest corpus
# key, cc1 expectation line and appends tools/tu_renames.tsv. LF everywhere.
import json, re, subprocess, sys

which = sys.argv[1]


def rd(p):
    return open(p, encoding="utf-8", newline="").read()


def wr(p, s):
    assert "\r" not in s
    open(p, "w", encoding="utf-8", newline="\n").write(s)


def once(s, a, b, what):
    assert s.count(a) == 1, (what, s.count(a), a[:80])
    return s.replace(a, b)


def drop_tu(old, ld_repl=None):
    """Remove src/<old>.c from bb2.ld (or replace its single .rodata line), manifest, cc1 exp."""
    ld = rd("bb2.ld")
    line = f"        build/src/{old}.o(.rodata);\n"
    assert ld.count(f"build/src/{old}.o(") == 1, old
    ld = once(ld, line, ld_repl or "", "bb2.ld " + old)
    wr("bb2.ld", ld)
    man = json.loads(rd("oracle/manifest.json"))
    del man["corpus"][f"src/{old}.c"]
    wr("oracle/manifest.json", json.dumps(man, indent=2) + "\n")
    exp = rd("tools/cc1_tu_expectation.txt")
    exp2, n = re.subn(r"^[0-9a-f]{40}  " + re.escape(old) + r"\n", "", exp, flags=re.M)
    assert n == 1, old
    wr("tools/cc1_tu_expectation.txt", exp2)
    subprocess.run(["git", "rm", "-q", f"src/{old}.c"], check=True)


def rename_row(old, new):
    head = subprocess.run(["git", "rev-parse", "--short=12", "HEAD"], capture_output=True,
                          text=True, check=True).stdout.strip()
    wr("tools/tu_renames.tsv", rd("tools/tu_renames.tsv") + f"{old}\t{new}\tafter:{head}\n")


if which == "ings":
    p = "src/main/6CF8.c"
    s = rd(p)
    s = once(s, "extern const char g_str_overflow[];\nextern const char g_str_eff_init[];\n",
             "/* This file's .rodata: debug format strings, and the build date that D_800A30E0\n"
             " * (below) points at. */\n"
             "const char g_str_overflow[12] = \"OVER FLOW\\n\";\n"
             "const char g_str_eff_init[28] = \"eff_init:%08x size:%08x\\n\";\n"
             "const char g_str_limit[12] = \"LIMIT:%08x\\n\";\n"
             "const char g_str_prim_overflow[24] = \"common prim over flow\\n\";\n"
             "const char g_str_build_date[28] = \"Fri Aug  7 22:26:32 1998\\n\";\n", p)
    s = once(s, "extern const char g_str_prim_overflow[];\n", "", p)
    s = once(s, "extern const char g_str_limit[];\n", "", p)
    s = once(s, "extern const char g_str_build_date[];\n", "", p)
    wr(p, s)
    drop_tu("ings_strings", "        build/src/main/6CF8.o(.rodata);\n")
    rename_row("ings_strings", "main/6CF8")
elif which == "memcard":
    p = "src/main/memcard.c"
    s = rd(p)
    s = once(s, "extern s32 D_800109BC;\n",
             "/* This file's .rodata: the memory-card path formats. */\n"
             "const char g_str_memcard_fmt[12] = \"bu%1d%1d:*\";\n"
             "const char D_800109BC[12] = \"bu%1d%1d:%s\";\n", p)
    s = once(s, "extern const char g_str_memcard_fmt[];\n", "", p)
    wr(p, s)
    p = "src/main/28514.c"
    s = rd(p)
    s = once(s, "#include \"common.h\"\n\n",
             "#include \"common.h\"\n\n"
             "/* memcard_Format's (28708.c) path format. It sits in this file's .rodata, before\n"
             " * func_80037D14's jump table, whose .align 3 supplies the zero bytes after it. */\n"
             "const char D_800109C8[] = \"bu%1d%1d:\";\n\n", p)
    wr(p, s)
    drop_tu("code6cac_b_rodata_post")
    rename_row("code6cac_b_rodata_post", "main/memcard")
elif which == "tu1b":
    p = "src/main/3AB48.c"
    s = rd(p)
    s = once(s, "extern s32 D_800158B4;\n",
             "const char D_800158B4[24] = \"common_vab start:%08x\\n\";\n", p)
    s = once(s, "extern const char D_800158CC[];\n",
             "const char D_800158CC[20] = \"vab id:%d mistake\\n\";\n", p)
    wr(p, s)
    drop_tu("text1a_b_pre_rodata_b")
    rename_row("text1a_b_pre_rodata_b", "main/3AB48")
elif which == "mid":
    drop_tu("text1a_b_mid_rodata")
    rename_row("text1a_b_mid_rodata", "-")
else:
    sys.exit("unknown fold")
print("folded", which)
