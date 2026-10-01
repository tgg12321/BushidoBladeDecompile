"""adoptlib.py: shared helpers for the Q65 adoption step scripts (tmp/q56/adopt/sNN_apply.py)."""
import os, re, subprocess, sys
H = os.path.dirname(os.path.abspath(__file__))
NL = "\n"


def run(*a):
    r = subprocess.run([sys.executable, *a], capture_output=True, text=True)
    print(r.stdout.strip())
    assert r.returncode == 0, r.stderr


_DECL = re.compile(r"^(extern\b.*;|typedef\b.*|#include\b.*|#define\b.*|[A-Za-z_][\w \t\*]*\([^)]*\)\s*;.*)$")


def decl_lines(text):
    """column-0 declarations (extern / typedef / #include / #define / prototype), whitespace-normalized; a
    multi-line typedef / struct / union / enum definition is one item, `<first line> ... <closing line>`"""
    out, L, i = [], text.split(NL), 0
    while i < len(L):
        l = L[i]
        if re.match(r"^(typedef\b|struct\b|union\b|enum\b)", l) and not l.rstrip().endswith(";"):
            j = i + 1
            while j < len(L) and not (L[j].startswith("}") and L[j].rstrip().endswith(";")):
                j += 1
            out.append(" ".join(l.split()) + " ... " + " ".join(L[min(j, len(L) - 1)].split()))
            i = j + 1
            continue
        if _DECL.match(l):
            out.append(" ".join(l.split()))
        i += 1
    return out


def carried_header(part, parent, cut):
    """splitc.py output `part` = the declarations it carried + the parent's lines cut.. verbatim; `parent` is the
    parent's text before the split. Returns the carried header's text."""
    P, Q = rd(part).split(NL), parent.split(NL)
    off = len(P) - (len(Q) - (cut - 1))
    assert off >= 0 and P[off:] == Q[cut - 1:], (part, off)
    return NL.join(P[:off])


def added_decls(before, after):
    """declarations `after` has that `before` does not (multiset difference, first-seen order)"""
    have = {}
    for l in decl_lines(before):
        have[l] = have.get(l, 0) + 1
    out = []
    for l in decl_lines(after):
        if have.get(l, 0):
            have[l] -= 1
        else:
            out.append(l)
    return out


def merge_msg(nn, intro, out, *parts):
    """run mergec.py and write the step's commit body (tmp/q56/adopt/sNN_msg.txt): the intro, then every
    verbatim-identical re-declaration the merge dropped, grouped by text with its count (layer-2 round 2)"""
    r = subprocess.run([sys.executable, f"{H}/mergec.py", out, *parts], capture_output=True, text=True)
    print(r.stdout.strip())
    assert r.returncode == 0, r.stderr
    drops = {}
    for l in r.stdout.splitlines():
        if "dropped verbatim repeat (" in l:
            txt = l.split("): ", 1)[1]
            drops.setdefault(txt, []).append(l.split("(", 1)[1].split(")")[0])
    body = [intro, "",
            f"Besides concatenating the parts in link order (union of their include blocks), the merge drops "
            f"{sum(len(v) for v in drops.values())} file-scope declarations that repeat, verbatim, one already emitted "
            f"earlier in the merged file (each object keeps its first declaration):"]
    for txt, where in sorted(drops.items(), key=lambda kv: -len(kv[1])):
        body.append(f"- {txt}  x{len(where)} ({', '.join(where[:4])}{', ...' if len(where) > 4 else ''})")
    open(f"{H}/s{nn}_msg.txt", "w", newline=NL).write(NL.join(body) + NL)


def rd(p):
    return open(p).read()


def wr(p, t):
    open(p, "w", newline=NL).write(t)


def sub1(p, a, b):
    t = rd(p)
    assert t.count(a) == 1, (p, a[:120], t.count(a))
    wr(p, t.replace(a, b))


def defline(path, func):
    """1-based line where func's definition (with its directly preceding comment block) starts."""
    L = rd(path).split(NL)
    i = next(k for k, l in enumerate(L)
             if re.match(r"^[\w\s\*]*\b%s\s*\(" % re.escape(func), l) and not l.rstrip().endswith(";"))
    while i > 0 and (L[i - 1].strip().startswith(("/*", "*", "//")) or L[i - 1].strip().endswith("*/")):
        i -= 1
    return i + 1


CPP = ("mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ "
       "-D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C")
CC1 = "tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"


def drop_header_duplicates(path):
    """A move brings the moved part's copied declarations along; a one-line typedef / struct definition that
    repeats, verbatim, one the file already gets from an included header is a redefinition error. Drop such
    lines (compile-driven, only verbatim header repeats; anything else is reported and left)."""
    hdr = set()
    for h in os.listdir("include"):
        if h.endswith(".h"):
            hdr |= {" ".join(l.split()) for l in rd(f"include/{h}").split(NL)}
    for _ in range(10):
        r = subprocess.run(f"{CPP} {path} 2>/dev/null | {CC1} -o /dev/null", shell=True, capture_output=True, text=True)
        bad = sorted({int(m.group(1)) for m in re.finditer(r"%s:(\d+): redefinition of" % re.escape(path), r.stderr)})
        if not bad:
            return
        L = rd(path).split(NL)
        drop = [n for n in bad if " ".join(L[n - 1].split()) in hdr]
        if not drop:
            print("redefinitions not verbatim header repeats:", bad)
            return
        for n in sorted(drop, reverse=True):
            print(f"{path}:{n}: dropped verbatim repeat of a header definition: {L[n - 1].strip()}")
            del L[n - 1]
        wr(path, NL.join(L))


def ld_follow(new, after):
    """bb2.ld: build/src/<new>.o follows build/src/<after>.o in every section list."""
    out = []
    for l in rd("bb2.ld").split(NL):
        out.append(l)
        m = re.match(r"^(\s*)build/src/%s\.o\(\.(\w+)\);" % re.escape(after), l)
        if m:
            out.append(f"{m.group(1)}build/src/{new}.o(.{m.group(2)});")
    wr("bb2.ld", NL.join(out))


def ld_merge(merged, members):
    """bb2.ld: the merged object takes its first member's place in each section list; the rest go."""
    ld = rd("bb2.ld").split(NL)
    for sec in ("rodata", "text", "data", "bss", "sdata", "sbss"):
        pat = re.compile(r"^\s*build/src/(%s)\.o\(\.%s\);" % ("|".join(map(re.escape, members)), sec))
        idx = [i for i, l in enumerate(ld) if l is not None and pat.match(l)]
        if not idx:
            continue
        ind = re.match(r"^(\s*)", ld[idx[0]]).group(1)
        ld[idx[0]] = f"{ind}build/src/{merged}.o(.{sec});"
        for i in idx[1:]:
            ld[i] = None
        ld = [l for l in ld if l is not None]
    wr("bb2.ld", NL.join(ld))
