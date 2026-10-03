#!/usr/bin/env python3
"""Restructure step 4f: src/main/psxsdk/libspu/spu.c (ex main.c, as committed at RENAME_COMMIT) -> one file
per LIBSND / LIBSPU / LIBAPI module (LIBSCAN verbatim spans; SSSTOP bit-verbatim in the Jun-06-1997 4.0
build) plus one file per unidentified gap (modules4f.py). spu.c keeps exactly LIBSPU SPU.

Moves only. Every top-level item of the old file (citems.py) goes, verbatim and in order, to the module
holding its function (declarations / comments / macros between two functions go with the function that
follows them). Then each part gets copies of the old file's declarations it needs, found mechanically and
checked by the compiler (run in WSL):
  - a header the old file included (system.h, psx.h, sound.h) when the part names something it declares;
  - typedefs and #defines located before the part in the old file whose name the part uses (to closure);
  - for every identifier cc1 reports undeclared, the latest declaration of it before the part (an extern,
    a typedef, or for a function defined above, its definition's header as a prototype);
  - for every call cc1 reports implicit where the old file had an explicit declaration before that
    function, the same.
The implicit-declaration state of every call is then the old file's. Dropped, used by no part: the old
include list, the stale "--- Functions ... (text4 segment) ---" banner, head declarations no part needs,
and the file-tail comment about main.c's .rodata (restated per part where it still applies).
Usage (WSL, repo root): split4f.py [--apply] [--ids]"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import citems  # noqa: E402
import modules4f as M  # noqa: E402

ROOT = HERE
while not os.path.exists(os.path.join(ROOT, "bb2.ld")):
    ROOT = os.path.dirname(ROOT)
os.chdir(ROOT)
RENAME_COMMIT = os.environ.get("RENAME_COMMIT", "caf277120")
SRC = "src/main/psxsdk/libspu/spu.c"
SPU_ID = "main/psxsdk/libspu/spu"
text = subprocess.run(["git", "show", f"{RENAME_COMMIT}:{SRC}"], capture_output=True, check=True).stdout.decode()
assert "\r" not in text
ITEMS, LINES = citems.items(text)
NSYM = {}
for l in open(os.path.join(HERE, "mainsyms.txt")):
    p = l.split()
    NSYM[p[2]] = int(p[0], 16) + 0x80083BE4

HEADERS = ["system.h", "psx.h", "sound.h"]
HNAMES = {}
for h in HEADERS:
    its, _ = citems.items(open(f"include/{h}", encoding="utf-8").read())
    HNAMES[h] = set().union(*[i["names"] for i in its if i["kind"] in ("decl", "pp")])
    HNAMES[h] -= {"SOUND_H", "SYSTEM_H", "PSX_H"}
assert [i["start"] for i in ITEMS[:7]] == [1, 2, 3, 4, 5, 6, 7] and LINES[6] == '#include "sound.h"'


def mod_of(a):
    return max(k for k, m in enumerate(M.MODS) if m[0] <= a)


FIRST = next(k for k, i in enumerate(ITEMS) if i["kind"] in ("func", "stub"))
assign = [None] * len(ITEMS)
nxt = None
for k in range(len(ITEMS) - 1, FIRST - 1, -1):
    it = ITEMS[k]
    if it["kind"] in ("func", "stub") and it["name"] in NSYM:
        nxt = mod_of(NSYM[it["name"]])
    assign[k] = nxt
PARTS = {}
for k in range(FIRST, len(ITEMS)):
    PARTS.setdefault(assign[k], []).append(k)
assert sorted(PARTS) == list(range(len(M.MODS)))
for mi, ks in PARTS.items():
    s, e = M.span(mi)
    for k in ks:
        it = ITEMS[k]
        if it["kind"] in ("func", "stub") and it["name"] in NSYM:
            assert s <= NSYM[it["name"]] < e, (it["name"], hex(s))
TAIL = ITEMS[-1]["end"]  # lines after it: the file-tail comment (dropped)
assert "main.c's rodata" in "\n".join(LINES[TAIL:]) and all(
    l.startswith(("/*", " *", "")) for l in LINES[TAIL:])
BANNER = "/* --- Functions 0x80083BE4 - 0x8008D060 (text4 segment) --- */"
assert LINES[ITEMS[FIRST]["lead"] - 2] == BANNER or BANNER in LINES[:ITEMS[FIRST]["lead"]]

TOK = re.compile(r"[A-Za-z_]\w*")


def tokens(s):
    return set(TOK.findall(citems.strip_cs(s)))


def item_text(k, lead=True):
    it = ITEMS[k]
    a = it["lead"] if lead else it["start"]
    return "\n".join(LINES[a - 1:it["end"]])


def copy_text(k):
    """a copied declaration: its own lines, plus the comment directly above it (no blank line between)
    only when that comment is about it: it names one of the item's identifiers, or it is a `volatile`
    justification above a volatile declaration. Other leads (module banners, "Externs for globals",
    notes on the next function) stay with the code they describe."""
    it = ITEMS[k]
    lead = LINES[it["lead"] - 1:it["start"] - 1]
    while lead and lead[0].strip() == "":
        lead = lead[1:]
    own = LINES[it["start"] - 1:it["end"]]
    keep = []
    starts = [i for i, l in enumerate(lead) if l.lstrip().startswith("/*")]
    lead = lead[starts[-1]:] if starts else []  # only the last comment block, the one directly above
    if lead and all(l.strip() for l in lead) and lead[0].lstrip().startswith("/*"):
        lt = " ".join(lead)
        names = it["names"] | ({it["name"]} if it["name"] else set())
        if any(re.search(r"\b" + re.escape(n) + r"\b", lt) for n in names) or (
                "volatile" in "".join(own) and "volatile" in lt):
            keep = lead
    return "\n".join(keep + own)


def rendered(k):
    """the text a copied item becomes: a function's header as a prototype, else copy_text."""
    return citems.proto(LINES, ITEMS[k]) if ITEMS[k]["kind"] == "func" else copy_text(k)


def is_typedef(k):
    return ITEMS[k]["kind"] == "decl" and re.match(r"\s*typedef\b", citems.strip_cs(item_text(k, False)))


def latest_decl(name, before_line, exclude):
    """the latest item before BEFORE_LINE that declares NAME (decl / pp / func / stub), not in EXCLUDE."""
    best = None
    for k, it in enumerate(ITEMS):
        if it["end"] >= before_line or k in exclude:
            continue
        if name in it["names"] or (it["kind"] in ("func", "stub") and it["name"] == name):
            best = k
    return best


def explicit_before(name, line):
    """did the old file declare NAME explicitly (header, item) before LINE?"""
    if any(name in HNAMES[h] for h in HEADERS):
        return True
    return any(it["end"] < line and (name in it["names"] or it["name"] == name) and it["kind"] != "stub"
               for it in ITEMS)


CPP = ["mipsel-linux-gnu-cpp", "-Iinclude", "-undef", "-Wall", "-lang-c", "-fno-builtin", "-Dmips",
       "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ",
       "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
CC1 = ["tools/gcc-2.7.2/build/cc1", "-O2", "-G0", "-funsigned-char", "-quiet", "-mcpu=3000", "-mips1",
       "-mno-abicalls", "-fno-builtin", "-Wimplicit", "-mel", "-msoft-float", "-o", "/dev/null"]


def compile_msgs(path):
    pre = subprocess.run(CPP + [path], capture_output=True).stdout
    err = subprocess.run(CC1, input=pre, capture_output=True).stderr.decode("utf-8", "replace")
    return err


WORK = "tmp/r4f/work"
os.makedirs(WORK, exist_ok=True)


def wrap(t):
    out, cur = [], ""
    for w in t.split(" "):
        if len(cur) + len(w) + 1 > 100:
            out.append(cur)
            cur = " *"
        cur = (cur + " " + w) if cur else w
    return "\n".join(out + [cur])


def header_comment(mi, fns):
    a, kind, lib, mod, fid = M.MODS[mi]
    s, e = M.span(mi)
    fl = ", ".join(fns[:-1]) + " and " + fns[-1] if len(fns) > 1 else fns[0]
    if kind == "v":
        return wrap(f"/* PsyQ 4.0 {lib} {mod}: {fl}. .text 0x{s:08X}..0x{e:08X}, a verbatim LIBSCAN module span "
                    f"(docs/naming/libscan/matches.json), Q106 D3. */")
    if kind == "v40u":
        return wrap(f"/* PsyQ 4.0 {lib} {mod} (the Jun-06-1997 4.0 build): {fl}. .text 0x{s:08X}..0x{e:08X}, a "
                    f"bit-verbatim module span (memory/closer/libsnd-hunt-report.md \"New verbatim result\"; "
                    f"docs/naming/libscan/ambiguous_resolutions.md), Q106 D3. */")
    return wrap(f"/* {lib} code between verbatim modules: {fl}. .text 0x{s:08X}..0x{e:08X}: an unidentified "
                f"region between verbatim LIBSCAN modules (docs/naming/libscan/matches.json; "
                f"memory/closer/libsnd-hunt-report.md lists the probable newer-build modules), one file per "
                f"gap (Q106 D3), named by its ROM offset. */")


ORIG_IMPLICIT = None
DROPPED = {}


def build_part(mi):
    ks = PARTS[mi]
    a, kind, lib, mod, fid = M.MODS[mi]
    first_line = ITEMS[ks[0]]["lead"]
    # A plain declaration between two functions that this part never names (it sat in front of this
    # module's function but serves the next module, e.g. the SSCALL externs before SsStart) is left out:
    # the part that uses it receives it as a copy. Its comment goes with it only when it is about it.
    seg = {k: (LINES[ITEMS[k]["lead"] - 1:ITEMS[k]["start"] - 1], LINES[ITEMS[k]["start"] - 1:ITEMS[k]["end"]])
           for k in ks}
    dropped = []
    for k in ks:
        it = ITEMS[k]
        if it["kind"] != "decl" or is_typedef(k) or not it["names"]:
            continue
        rest = tokens("\n".join("\n".join(seg[j][1]) for j in ks if j != k))
        if it["names"] & rest:
            continue
        lead, body = seg[k]
        starts = [i for i, l in enumerate(lead) if l.lstrip().startswith("/*")]
        last = lead[starts[-1]:] if starts else []
        if last and all(l.strip() for l in last) and copy_text(k).startswith(last[0]):
            lead = lead[:starts[-1]]  # its own comment goes with it
        seg[k] = (lead, [])
        dropped.append(k)
    own = "\n".join("\n".join(seg[k][0] + seg[k][1]) for k in ks if seg[k][0] + seg[k][1])
    while "\n\n\n\n" in own:
        own = own.replace("\n\n\n\n", "\n\n\n")
    DROPPED[mi] = dropped
    # a stale banner as the part's first lead line goes
    own = own.replace(BANNER + "\n", "")
    fns = [ITEMS[k]["name"] for k in ks if ITEMS[k]["kind"] in ("func", "stub") and ITEMS[k]["name"] in NSYM]
    stubs = [k for k in ks if ITEMS[k]["kind"] == "stub"]
    copies = set()
    headers = []
    while True:
        # tokens of what the file will hold: a copied function is only its prototype
        body_toks = tokens(own + "\n" + "\n".join(rendered(k) for k in copies))
        changed = False
        for h in HEADERS:
            if h not in headers and HNAMES[h] & body_toks:
                headers.append(h)
                changed = True
        # typedefs / macros before the part, by name, to closure
        for k, it in enumerate(ITEMS):
            if it["end"] >= first_line or k in copies:
                continue
            if (it["kind"] == "pp" and it["name"] in body_toks) or (is_typedef(k) and it["names"] & body_toks):
                copies.add(k)
                changed = True
        if changed:
            continue
        txt = render(mi, fns, headers, copies, own, stubs)
        path = f"{WORK}/{fid.replace('/', '_')}.c"
        open(path, "w", encoding="utf-8", newline="\n").write(txt)
        err = compile_msgs(path)
        need = set()
        fn = None
        for line in err.splitlines():
            m = re.search(r"In function `([^']*)'", line)
            if m:
                fn = m.group(1)
                continue
            m = re.search(r"`(\w+)' undeclared", line)
            if m:
                need.add(m.group(1))
                continue
            m = re.search(r"implicit declaration of function `(\w+)'", line)
            if m and fn:
                fline = next(i["start"] for i in ITEMS if i["kind"] == "func" and i["name"] == fn)
                if explicit_before(m.group(1), fline):
                    need.add(m.group(1))
                continue
            m = re.search(r":(\d+): (parse error|storage size|invalid use|dereferencing pointer|.*incomplete type)", line)
            if m:
                src_line = txt.split("\n")[int(m.group(1)) - 1]
                need |= {t for t in tokens(src_line) if latest_decl(t, first_line, copies) is not None}
        added = False
        for name in sorted(need):
            k = latest_decl(name, first_line, copies)
            if k is not None:
                copies.add(k)
                added = True
        if not added:
            return txt, err, copies, headers
    return None


def render(mi, fns, headers, copies, own, stubs):
    a, kind, lib, mod, fid = M.MODS[mi]
    out = [header_comment(mi, fns)]
    if stubs:
        out.append("#define INCLUDE_ASM_USE_MACRO_INC 1\n#include \"include_asm.h\"\n#include \"bios.h\"")
    else:
        out.append('#include "common.h"' + "".join(f'\n#include "{h}"' for h in headers))
    out.append("")
    if copies:
        intro = ("/* Declarations from the old main.c's head that this module uses. */" if fid == SPU_ID else
                 "/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex "
                 "main.c). */")
        decl = []
        for k in sorted(copies):
            it = ITEMS[k]
            decl.append(rendered(k))
        out.append(intro + "\n" + "\n".join(decl) + "\n")
    out.append(own.lstrip("\n"))
    txt = "\n".join(out).rstrip("\n") + "\n"
    for f, old, new in POST_EDITS:  # comment-only: moved comments that the split made false
        if f == fid:
            assert txt.count(old) == 1, (fid, old)
            txt = txt.replace(old, new)
    return txt


# The only text changes besides includes and declarations: three moved comments that pointed at code in
# the old single file and would be false in their new one (review a43866054bc7d17eb).
POST_EDITS = [
    ("main/psxsdk/libspu/s_m_int",
     "   .L-label nop handling, as for siblings SpuFree and _spu_init in this\n   same translation unit. */",
     "   .L-label nop handling, as for siblings SpuFree (s_m_f.c) and _spu_init\n   (spu.c). */"),
    ("main/psxsdk/libspu/spu",
     "/* SPU-module debug strings (rodata 0x800163D8..0x80016420). Defined HERE —\n"
     " * before func_80088740 (_spu_init), their first user — so they emit into\n"
     " * .rodata AHEAD of the compiler-emitted jump tables of func_8008AF9C\n"
     " * (SpuSetCommonAttr), matching the original Sony spu.c literal order. */",
     "/* SPU-module debug strings (rodata 0x800163D8..0x80016420), defined before\n"
     " * func_80088740 (_spu_init), their first user. bb2.ld links this object's\n"
     " * .rodata after sstick.o's jump table and ahead of s_sca.o's (the tables of\n"
     " * func_8008AF9C, SpuSetCommonAttr), in link order. */"),
    ("main/psxsdk/libspu/7BC88",
     "SpuVoiceAttr (defined above _SsVmFlush) per PsyQ libspu.h",
     "SpuVoiceAttr (defined above) per PsyQ libspu.h"),
]


def main():
    results = []
    bad = False
    for mi in range(len(M.MODS)):
        txt, err, copies, headers = build_part(mi)
        fid = M.MODS[mi][4]
        errs = [l for l in err.splitlines() if ":" in l and "warning" not in l and "In function" not in l
                and "At top level" not in l]
        if errs:
            bad = True
            print("ERRORS in", fid, errs[:5])
        results.append((fid, txt))
    if "--ids" in sys.argv:
        print(" ".join(f for f, _ in results))
        return 0
    if bad:
        return 1
    for fid, txt in results:
        if "--apply" in sys.argv:
            p = f"src/{fid}.c"
            os.makedirs(os.path.dirname(p), exist_ok=True)
            assert fid == SPU_ID or not os.path.exists(p), p
            open(p, "w", encoding="utf-8", newline="\n").write(txt)
        else:
            print("=====", fid)
            print(txt if len(txt) < 3000 else txt[:2200] + "\n...\n" + txt[-500:])
    return 0


if __name__ == "__main__":
    sys.exit(main())


def report_dropped():
    for mi, ks in sorted(DROPPED.items()):
        if ks:
            print(M.MODS[mi][4], "leaves out:", ", ".join(sorted(set().union(*[ITEMS[k]["names"] for k in ks]))))
