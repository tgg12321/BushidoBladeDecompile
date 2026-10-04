#!/usr/bin/env python3
"""proto_trial.py [TARGETS.tsv] [OUT.tsv] : compile every caller of every function row under its target
prototype and compare cc1's assembly with the unmodified TU (run in WSL, repo root).
Input: tmp/p2/targets.tsv (refs.py; columns class, symbol, ..., target). Per (symbol, TU that names it):
  - every declaration of the symbol in the preprocessed TU (headers included) is renamed away, and
    `extern <target>;` is inserted at file scope just before the first use (same line: line numbers
    stay valid); an implicit call becomes a prototyped one the same way;
  - cc1 runs with the TU's real flags (GP_FILES -> -G8) and warnings on; the asm is compared with the
    unmodified TU's.
The definition's own TU is not recompiled: `defcompat` says whether the target, declared right
before the definition, is type-compatible with it (cc1 reports no conflicting-types error).
Writes OUT (default tmp/p2/proto_trial.tsv):
  class symbol tu calls result changed_functions call_site_warnings
result: same | asm-diff(N lines) | error: <first error> ; for the def TU: defcompat yes/no."""
import bisect, concurrent.futures as cf, difflib, glob, os, re, subprocess, sys
sys.path.insert(0, ".")
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from citems import strip_cs

TGT = sys.argv[1] if len(sys.argv) > 1 else "tmp/p2/targets.tsv"
OUT = sys.argv[2] if len(sys.argv) > 2 else "tmp/p2/proto_trial.tsv"
CPP = ["mipsel-linux-gnu-cpp", "-Iinclude", "-undef", "-Wall", "-lang-c", "-fno-builtin", "-Dmips",
       "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ",
       "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
mk = open("Makefile").read()
GP = set(re.search(r"^GP_FILES\s*:=(.*)$", mk, re.M).group(1).split())


def cc1(tu, warn=False):
    g = "-G8" if tu in GP else "-G0"
    return ["tools/gcc-2.7.2/build/cc1", "-O2", g, "-funsigned-char", "-quiet", "-mcpu=3000", "-mips1",
            "-mno-abicalls", "-fno-builtin", "-mel", "-msoft-float"] + ([] if warn else ["-w"])


EXPR_KW = {"return", "else", "case", "do", "sizeof", "goto"}
_pre, _base, _clean = {}, {}, {}


def cleaned(tu):
    if tu not in _clean:
        _clean[tu] = strip_cs(pre(tu))
    return _clean[tu]


def pre(tu):
    if tu not in _pre:
        _pre[tu] = subprocess.run(CPP + [f"src/{tu}.c"], capture_output=True).stdout.decode("utf-8", "replace")
    return _pre[tu]


def base(tu):
    if tu not in _base:
        _base[tu] = subprocess.run(cc1(tu, warn=True), input=pre(tu).encode(), capture_output=True).stdout.decode()
    return _base[tu]


def decl_text(sym, t):
    ret, params = re.match(r"(.*?)\s*@\((.*)\)$", t).groups()
    if "(*)" in ret:   # function-pointer return: `R (*)(A)` -> `R (*sym(P))(A)`
        head, tail = ret.split("(*)", 1)
        return f"extern {head}(*{sym}({params})){tail};"
    return f"extern {ret} {sym}({params});"


def classify(text, sym):
    """[(pos, 'decl'|'use')] for every whole-word occurrence of sym (text: comments/strings blanked)."""
    out = []
    for m in re.finditer(r"\b" + re.escape(sym) + r"\b", text):
        b = text[:m.start()].rstrip()
        if re.search(r"\(\s*\*$", b):
            out.append((m.start(), "fptr"))
            continue
        k = b
        while k.endswith("*"):
            k = k[:-1].rstrip()
        w = re.search(r"([A-Za-z_]\w*)$", k)
        if w and w.group(1) not in EXPR_KW:
            out.append((m.start(), "decl"))
        else:
            out.append((m.start(), "use"))
    return out


_cands = {}


def candidates(tu, clean):
    """sorted offsets just after every file-scope ';' and function-closing '}' of the TU."""
    if tu not in _cands:
        depth, stack, out, knr, prev = 0, [], [], False, 0
        for m in re.finditer(r"[{};]", clean):
            c, i = m.group(0), m.start()
            if c == "{":
                head = clean[max(0, i - 200):i].rstrip()
                stack.append(head.endswith(")") or (depth == 0 and knr))
                if depth == 0:
                    knr = False
                depth += 1
            elif c == "}":
                depth -= 1
                fn = stack.pop() if stack else False
                if depth == 0 and fn:
                    out.append(i + 1)
            elif depth == 0:
                seg = clean[prev:i]
                # K&R parameter declarations (`f(a) s32 a; {`): no insertion until the body opens
                if not knr and "=" not in seg and re.search(r"\)\s*[A-Za-z_][^;{}()]*$", seg):
                    knr = True
                if not knr:
                    out.append(i + 1)
            if depth == 0 and c in ";}":
                prev = i + 1
        _cands[tu] = out
    return _cands[tu]


def insertion_point(tu, clean, pos):
    """offset just after the last file-scope ';' or function-closing '}' before pos."""
    c = candidates(tu, clean)
    k = bisect.bisect_right(c, pos) - 1
    return c[k] if k >= 0 else 0


CKW = {"void", "int", "char", "short", "long", "unsigned", "signed", "const", "volatile", "struct", "union",
       "enum", "u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64"}


def missing_types(tgt, clean):
    """type names the target uses that the TU never typedefs."""
    out = []
    for t in sorted(set(re.findall(r"[A-Za-z_]\w*", tgt)) - CKW):
        if not re.search(r"(typedef[^;]*\b|\}\s*)" + re.escape(t) + r"\s*;", clean):
            out.append(t)
    return out


def trial(cls, sym, tgt, tu):
    p = pre(tu)
    clean = cleaned(tu)
    occ = classify(clean, sym)
    uses = [q for q, k in occ if k == "use"]
    calls = sum(1 for q in uses if re.match(r"\s*\(", clean[q + len(sym):]))
    if not uses:
        return None
    if find_def(clean, sym):
        return (cls, sym, tu, "def", defcompat(sym, tgt, tu) + " (TU defines it; not in census defs)", "", "")
    first = uses[0]
    ip = insertion_point(tu, clean, first)
    s = p[:ip] + " " + decl_text(sym, tgt) + p[ip:]
    shift = ip + 1 + len(decl_text(sym, tgt))
    s = list(s)
    for q, k in reversed(occ):
        if k == "decl":
            q2 = q if q < ip else q + shift - ip
            s[q2:q2 + len(sym)] = list(sym + "__p2old")
    s = "".join(s)
    r = subprocess.run(cc1(tu, warn=True), input=s.encode(), capture_output=True)
    err = r.stderr.decode("utf-8", "replace").splitlines()
    msgs = [l for l in err if re.search(r":\d+: ", l)]
    errors = [l for l in msgs if "warning:" not in l]
    warns = [l for l in msgs if "warning:" in l and f"`{sym}'" in l]
    if r.returncode != 0 or errors:
        miss = missing_types(tgt, clean)
        if miss:
            return (cls, sym, tu, str(calls), "type-missing: " + ",".join(miss), "", "")
        return (cls, sym, tu, str(calls), "error: " + (errors[0] if errors else f"cc1 exit {r.returncode}"), "", "")
    new = r.stdout.decode()
    old = base(tu)
    if new == old:
        res, fns = "same", ""
    else:
        d = [l for l in difflib.unified_diff(old.splitlines(), new.splitlines(), lineterm="", n=0)
             if l[:1] in "+-" and not l.startswith(("+++", "---"))]
        fns = set()
        cur, ents = None, {}
        for i, l in enumerate(old.splitlines()):
            m = re.match(r"\s*\.ent\s+(\w+)", l)
            if m:
                cur = m.group(1)
            ents[i] = cur
        sm = difflib.SequenceMatcher(None, old.splitlines(), new.splitlines(), autojunk=False)
        for op, a1, a2, b1, b2 in sm.get_opcodes():
            if op != "equal":
                fns.add(ents.get(a1, ents.get(max(a1 - 1, 0))) or "?")
        res, fns = f"asm-diff({len(d)} lines)", ",".join(sorted(f for f in fns if f))
    ws = "; ".join(re.sub(r"^src/main/", "", re.sub(r"warning: ", "", w)) for w in warns[:8])
    return (cls, sym, tu, str(calls), res, fns, ws + (f"; +{len(warns) - 8} more" if len(warns) > 8 else ""))


def find_def(clean, sym):
    """the match of sym's C definition header in the TU (ANSI or K&R), or None."""
    for mm in re.finditer(r"\b" + re.escape(sym) + r"\s*\(", clean):
        rest = clean[mm.end():]
        depth = 1; j = 0
        while j < len(rest) and depth:
            depth += rest[j] == "("; depth -= rest[j] == ")"; j += 1
        if re.match(r"\s*\{", rest[j:]) or re.match(r"\s*[A-Za-z_][^;{}()]*;([^;{}()]*;)*\s*\{", rest[j:]):
            return mm
    return None


def defcompat(sym, tgt, tu):
    p = pre(tu)
    clean = cleaned(tu)
    m = find_def(clean, sym)
    if not m:
        return "no C definition"
    ip = insertion_point(tu, clean, m.start())
    s = p[:ip] + " " + decl_text(sym, tgt) + p[ip:]
    r = subprocess.run(cc1(tu, warn=True), input=s.encode(), capture_output=True)
    e = [l for l in r.stderr.decode("utf-8", "replace").splitlines() if re.search(r":\d+: ", l) and "warning:" not in l]
    if r.returncode == 0 and not e:
        return "defcompat yes"
    miss = missing_types(tgt, clean)
    return "defcompat type-missing: " + ",".join(miss) if miss else "defcompat no: " + (e[0] if e else "cc1 failed")


rows = [l.rstrip("\n").split("\t") for l in open(TGT, encoding="utf-8") if not l.startswith("#")]
srcs = {f[4:-2].replace("\\", "/"): strip_cs(open(f, encoding="utf-8").read())
        for f in glob.glob("src/**/*.c", recursive=True)}
jobs, defjobs = [], []
for r in rows:
    cls, sym, d, tgt = r[0], r[1], r[2], r[6]
    if not tgt:
        continue
    deftu = re.search(r"\[src/(\S+)\.c:\d+\]", d)
    deftu = deftu.group(1) if deftu else None
    for tu, t in srcs.items():
        if tu == deftu or not re.search(r"\b" + re.escape(sym) + r"\b", t):
            continue
        jobs.append((cls, sym, tgt, tu))
    if deftu:
        defjobs.append((cls, sym, tgt, deftu))
for tu in {j[3] for j in jobs} | {j[3] for j in defjobs}:
    cleaned(tu)
with cf.ThreadPoolExecutor(os.cpu_count() or 4) as ex:
    list(ex.map(base, sorted({j[3] for j in jobs})))
    res = list(ex.map(lambda j: trial(*j), jobs))
    dres = list(ex.map(lambda j: defcompat(j[1], j[2], j[3]), defjobs))
out = [x for x in res if x]
for (cls, sym, tgt, tu), d in zip(defjobs, dres):
    out.append((cls, sym, tu, "def", d, "", ""))
out.sort(key=lambda x: (x[0], x[1], x[3] == "def", x[2]))
with open(OUT, "w", encoding="utf-8", newline="\n") as fh:
    fh.write("# class\tsymbol\ttu\tcalls\tresult\tchanged_functions\tcall_site_warnings\n")
    for x in out:
        fh.write("\t".join(x) + "\n")
import collections
print(len(out), "rows;", collections.Counter(x[4].split("(")[0].split(":")[0] for x in out))
