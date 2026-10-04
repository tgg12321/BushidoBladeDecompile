"""cmodel.py: the C model the Phase 2 census tools share (casts.py, protos.py). Importing it parses every
linked TU (run in WSL, repo root): per TU its preprocessed text, typedef names, cc1 struct layouts
(layout.py), file-scope and header declarations, functions with parameters / locals, and a global call
index; resolve() names the struct a base expression points at (decl / member / assign / caller / ...);
leaf() maps (struct, offset, size, signedness) to the member there."""
import collections, json, os, re, subprocess, sys
sys.path.insert(0, ".")
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from engine import tus
from citems import items, strip_cs
from ctok import Toks, typedef_names, read_type, read_declarators, SCALAR, EXPR_KW
from layout import layouts

CPP = ["mipsel-linux-gnu-cpp", "-Iinclude", "-undef", "-Wall", "-lang-c", "-fno-builtin", "-Dmips",
       "-D__GNUC__=2", "-D__OPTIMIZE__", "-D__mips__", "-D__mips", "-Dpsx", "-D__psx__", "-D__psx", "-D_PSYQ",
       "-D__EXTENSIONS__", "-D_MIPSEL", "-D_LANGUAGE_C", "-DLANGUAGE_C"]
OPERAND_END = re.compile(r"[A-Za-z_0-9\])]")


def scal(base, stars):
    """(size, signed) of a scalar/pointer type, None for aggregates/void."""
    if stars:
        return (4, 0)
    b = base.replace("unsigned ", "u ").split()
    if "u" in b:
        b = [x for x in b if x != "u"]
        sz = SCALAR.get(b[-1] if b else "int", (4, 0))[0]
        return (sz, 0)
    if "signed" in b and "char" in b:
        return (1, 1)
    return SCALAR.get(b[-1]) if b else None


def const_val(toks):
    s = " ".join(toks)
    if not s or not re.fullmatch(r"[0-9xXa-fA-F uUlL+\-*()<>|&]+", s):
        return None
    s = re.sub(r"(0[xX][0-9a-fA-F]+|\d+)[uUlL]+", r"\1", s)
    try:
        return int(eval(s, {"__builtins__": {}}))
    except Exception:
        return None


def split_add(tt):
    """top-level additive terms of a token list: [(sign, [tokens])]."""
    terms, cur, d, sign = [], [], 0, 1
    for i, x in enumerate(tt):
        if x in "([":
            d += 1
        elif x in ")]":
            d -= 1
        if d == 0 and x in ("+", "-") and cur and OPERAND_END.match(cur[-1]):
            terms.append((sign, cur))
            cur, sign = [], (1 if x == "+" else -1)
            continue
        cur.append(x)
    terms.append((sign, cur))
    return terms


class TU:
    def __init__(self, tu):
        self.tu = tu
        self.path = f"src/{tu}.c"
        self.raw = open(self.path, encoding="utf-8").read()
        self.rawlines = self.raw.split("\n")
        self.clean = strip_cs(self.raw)
        self.pre = subprocess.run(CPP + [self.path], capture_output=True).stdout.decode("utf-8", "replace")
        cpre = re.sub(r"(?m)^#.*$", lambda m: " " * len(m.group(0)), strip_cs(self.pre))
        self.tn = typedef_names(cpre)
        self.lay = layouts(tu, self.pre)
        self.glob, self.hdr = self.file_scope(cpre)
        self.T = Toks(self.clean)
        self.funcs = self.functions()

    def file_scope(self, cpre):
        """name -> (base, stars, arr) for file-scope declarations; header ones separately."""
        # map each preprocessed offset to its source file via line markers
        marks = [(m.start(), m.group(1)) for m in re.finditer(r'^# \d+ "([^"]*)"', self.pre, re.M)]
        T = Toks(cpre)
        g, h, self.fret = {}, {}, {}
        k, depth = 0, 0
        mi = 0
        while k < len(T.t):
            x = T.t[k]
            if x == "{":
                depth += 1
            elif x == "}":
                depth -= 1
            elif depth == 0 and (k == 0 or T.t[k - 1] in (";", "}")) and x != "typedef":
                r = read_type(T, k, self.tn)
                if r and r[1] < len(T.t):
                    base, j = r
                    decls, j2 = read_declarators(T, j)
                    while mi + 1 < len(marks) and marks[mi + 1][0] <= T.pos[k]:
                        mi += 1
                    src = marks[mi][1] if marks else self.path
                    for name, stars, arr, init, proto in decls:
                        if name and not proto:
                            (g if src == self.path else h)[name] = (base, stars, arr)
                        elif name and proto:
                            self.fret[name] = (base, stars)
                    if j2 > k:
                        k = j2
                        continue
            k += 1
        return g, h

    def functions(self):
        its, lines = items(self.raw)
        T = self.T
        out = []
        for it in its:
            if it["kind"] != "func":
                continue
            a = next((i for i in range(len(T.t)) if T.line(i) >= it["start"]), None)
            if a is None:
                continue
            b = a
            while b < len(T.t) and T.line(b) <= it["end"]:
                b += 1
            # find the body '{' : first '{' after the name's ')'
            k = a
            while k < b and not (T.t[k] == it["name"] and k + 1 < b and T.t[k + 1] == "("):
                k += 1
            if k >= b:
                continue
            pe = T.match(k + 1)
            q = pe + 1
            while q < b and T.t[q] != "{":
                q += 1
            if q >= b:
                continue
            f = dict(name=it["name"], start=it["start"], end=it["end"], head=(k, pe), body=(q, T.match(q)),
                     params=[], vars=collections.defaultdict(list))
            self.parse_params(f, k + 2, pe, pe + 1, q)
            self.parse_locals(f)
            fake_lines = [i + 1 for i in range(it["start"] - 1, it["end"]) if "FAKE" in self.rawlines[i]]
            f["fake_lines"] = fake_lines
            txt = "\n".join(self.rawlines[it["start"] - 1:it["end"]])
            f["cast_note"] = bool(re.search(r"/\*[^*]*?(cast|byte offset|raw offset|\(u8 \*\))[^*]*?"
                                            r"(score|codegen|load-bearing|lhu|lbu|bytes|match)", txt, re.S | re.I))
            out.append(f)
        return out

    def parse_params(self, f, a, b, ka, kb):
        T = self.T
        if a >= b or (b - a == 1 and T.t[a] == "void"):
            return
        # ANSI if the first param starts with a type
        segs, cur, d = [], [], 0
        for i in range(a, b):
            x = T.t[i]
            if x in "([":
                d += 1
            elif x in ")]":
                d -= 1
            if x == "," and d == 0:
                segs.append(cur); cur = []
                continue
            cur.append(i)
        segs.append(cur)
        ansi = segs and segs[0] and (T.t[segs[0][0]] in self.tn or T.t[segs[0][0]] in
                                    ("void", "char", "short", "int", "long", "unsigned", "signed", "struct",
                                     "union", "enum", "const", "volatile", "register"))
        if ansi:
            for s in segs:
                if not s:
                    continue
                r = read_type(T, s[0], self.tn)
                if not r:
                    continue
                base, j = r
                decls, _ = read_declarators(T, j)
                for name, stars, arr, init, proto in decls[:1]:
                    if name:
                        f["params"].append(name)
                        f["vars"][name].append((base, stars + (1 if arr else 0), False))
        else:
            names = [T.t[s[0]] for s in segs if s]
            f["params"] = names
            k = ka
            while k < kb:
                r = read_type(T, k, self.tn)
                if not r:
                    k += 1
                    continue
                base, j = r
                decls, j2 = read_declarators(T, j)
                for name, stars, arr, init, proto in decls:
                    if name:
                        f["vars"][name].append((base, stars + (1 if arr else 0), False))
                k = max(j2, j) + 1
            for n in names:
                f["vars"].setdefault(n, [("int", 0, False)])

    def parse_locals(self, f):
        T = self.T
        q, e = f["body"]
        k = q + 1
        while k < e:
            if T.t[k - 1] in ("{", ";", "}") and (T.t[k] in self.tn or T.t[k] in
                                                  ("char", "short", "int", "long", "unsigned", "signed",
                                                   "struct", "union", "const", "volatile", "register",
                                                   "static", "extern", "void")):
                if k + 1 < e and T.t[k + 1] in ("=", "(", "[", "->", ".", "++", "--", "+=", "-="):
                    k += 1
                    continue
                r = read_type(T, k, self.tn)
                if r:
                    base, j = r
                    decls, j2 = read_declarators(T, j)
                    for name, stars, arr, init, proto in decls:
                        if name and not proto:
                            f["vars"][name].append((base, stars, arr, init))
                    if j2 > k:
                        k = j2
                        continue
            k += 1


# ---------------------------------------------------------------- pass 1: every TU
units = {}
for t in tus.linked_tus():
    if os.path.exists(f"src/{t}.c"):
        units[t] = TU(t)
allstructs = {}
for u in units.values():
    for n, l in u.lay.items():
        allstructs.setdefault(n, l)
gdecl = collections.defaultdict(set)   # global name -> {(base, stars, arr)} struct-typed declarations anywhere
for u in units.values():
    for d in (u.glob, u.hdr):
        for n, (b, st, a) in d.items():
            if b in allstructs:
                gdecl[n].add((b, st, a))
fdefs = {}            # function name -> (tu, func)
calls = collections.defaultdict(list)   # callee -> [(tu, func, [arg token lists])]
for u in units.values():
    T = u.T
    for f in u.funcs:
        fdefs.setdefault(f["name"], (u, f))
        q, e = f["body"]
        for k in range(q, e):
            if re.match(r"[A-Za-z_]", T.t[k]) and T.t[k + 1] == "(" and T.t[k - 1] not in (".", "->") \
                    and T.t[k] not in EXPR_KW and T.t[k] not in u.tn:
                ce = T.match(k + 1)
                args, cur, d = [], [], 0
                for i in range(k + 2, ce):
                    x = T.t[i]
                    if x in "([":
                        d += 1
                    elif x in ")]":
                        d -= 1
                    if x == "," and d == 0:
                        args.append(cur); cur = []
                        continue
                    cur.append(T.t[i])
                if cur:
                    args.append(cur)
                calls[T.t[k]].append((u, f, args))


def var_type(u, f, name):
    """(base, stars, arr, scope) of name in function f of TU u, or None."""
    if f and name in f["vars"]:
        v = f["vars"][name][-1]
        return (v[0], v[1], v[2], "local" if name not in f["params"] else "param")
    if name in u.glob:
        b, s, a = u.glob[name]
        return (b, s, a, "tu-global")
    if name in u.hdr:
        b, s, a = u.hdr[name]
        return (b, s, a, "header-global")
    return None


def struct_of_type(base, stars, arr, addr_of, elem):
    """struct name a base expression of this declared type points at, or None."""
    nm = base.replace("struct ", "struct ") if base.startswith("struct ") else base
    if nm not in allstructs:
        return None
    ptr_level = stars + (1 if arr else 0) + (1 if addr_of else 0) - (1 if elem else 0)
    return nm if ptr_level == 1 else None


def strip_casts(tt):
    casts = []
    while len(tt) > 2 and tt[0] == "(":
        d, j = 0, 0
        for j, x in enumerate(tt):
            d += x == "("
            d -= x == ")"
            if d == 0:
                break
        inner = tt[1:j]
        if inner and all(re.match(r"[A-Za-z_]\w*$|\*$", x) for x in inner) and j < len(tt) - 1:
            casts.append(" ".join(inner))
            tt = tt[j + 1:]
            continue
        if j == len(tt) - 1:
            tt = tt[1:-1]   # redundant parens
            continue
        break
    return tt, casts


def resolve(u, f, tt, depth=0, seen=None):
    """(struct, how, root, root_decl) for a base expression token list."""
    seen = seen or set()
    tt, casts = strip_casts(list(tt))
    for c in casts:
        cb = c.replace(" *", "").replace("*", "").strip()
        if cb in allstructs and c.count("*") == 1:
            return cb, "cast", "", c
    addr_of = bool(tt) and tt[0] == "&"
    if addr_of:
        tt = tt[1:]
    tt, _ = strip_casts(tt)
    if tt and re.match(r"0[xX]1[fF]80", tt[0]):
        return None, "scratchpad", tt[0], "literal"
    if not tt or not re.match(r"[A-Za-z_]", tt[0]):
        return None, "unknown", "", ""
    root = tt[0]
    if len(tt) > 1 and tt[1] == "(" and root in u.fret and (not f or root not in f["vars"]):
        b, st = u.fret[root]
        s = struct_of_type(b, st, False, False, False)
        return (s, "return", root, f"{b} {'*' * st} {root}()") if s else (None, "untyped-return", root,
                                                                         f"{b} {'*' * st} {root}()")
    vt = var_type(u, f, root)
    decl = f"{vt[0]}{' ' + '*' * vt[1] if vt[1] else ''}{'[]' if vt[2] else ''} ({vt[3]})" if vt else "?"
    rest = tt[1:]
    elem = False
    if rest and rest[0] == "[":
        d = 0
        for j, x in enumerate(rest):
            d += x == "["
            d -= x == "]"
            if d == 0:
                break
        rest = rest[j + 1:]
        elem = True
    if not vt:
        return None, "unknown", root, decl
    if rest and rest[0] in ("->", ".") and len(rest) >= 2:
        s = struct_of_type(vt[0], vt[1], vt[2], rest[0] == ".", elem) if rest[0] == "->" else \
            (vt[0] if vt[0] in allstructs else None)
        if rest[0] == "->":
            s = struct_of_type(vt[0], vt[1], vt[2], False, elem)
        if s and len(rest) == 2:
            m = next((m for m in allstructs[s]["members"] if m["name"] == rest[1]), None)
            if m:
                ms = struct_of_type(m["base"], m["stars"], m["arr"], addr_of, False)
                if ms:
                    return ms, "member", root, decl
        return None, "unknown", root, decl
    if rest:
        return None, "unknown", root, decl
    s = struct_of_type(vt[0], vt[1], vt[2], addr_of, elem)
    if s:
        return s, "decl", root, decl
    if vt[3] in ("tu-global", "header-global"):
        el = {struct_of_type(b, st, a, addr_of, elem) for b, st, a in gdecl.get(root, ())} - {None}
        if len(el) == 1:
            return next(iter(el)), "decl-elsewhere", root, decl
    if depth >= 4 or (u.tu, f and f["name"], root) in seen:
        return None, "untyped", root, decl
    seen = seen | {(u.tu, f and f["name"], root)}
    if vt[3] == "local":
        ev, na = assignments(u, f, root, depth, seen)
        if ev == {"@scratchpad"}:
            return None, f"scratchpad-local({na[0]}/{na[1]})", root, decl
        ev.discard("@scratchpad")
        if len(ev) == 1:
            return next(iter(ev)), f"assign({na[0]}/{na[1]})", root, decl
        if len(ev) > 1:
            return None, "ambiguous(" + "|".join(sorted(ev)) + ")", root, decl
    if vt[3] == "param":
        i = f["params"].index(root)
        ev, n, k = set(), 0, 0
        for cu, cf, args in calls.get(f["name"], []):
            if i < len(args):
                n += 1
                s2, how2, _, _ = resolve(cu, cf, args[i], depth + 1, seen)
                s2 = s2 or ("@scratchpad" if how2.startswith("scratchpad") else None)
                if s2:
                    ev.add(s2)
                    k += 1
        if ev == {"@scratchpad"}:
            return None, f"scratchpad-param({k}/{n})", root, decl
        ev.discard("@scratchpad")
        if len(ev) == 1:
            return next(iter(ev)), f"caller({k}/{n})", root, decl
        if len(ev) > 1:
            return None, "ambiguous(" + "|".join(sorted(ev)) + ")", root, decl
    return None, "untyped", root, decl


def assignments(u, f, name, depth, seen):
    T = u.T
    q, e = f["body"]
    ev, n, k = set(), 0, 0
    for v in f["vars"].get(name, []):
        if len(v) > 3 and v[3]:
            n += 1
            s, how, _, _ = resolve(u, f, T.t[v[3][0]:v[3][1]], depth + 1, seen)
            s = s or ("@scratchpad" if how.startswith("scratchpad") else None)
            if s:
                ev.add(s)
                k += 1
    for k in range(q, e):
        if T.t[k] == name and T.t[k + 1] == "=" and T.t[k - 1] not in (".", "->"):
            j, d = k + 2, 0
            while j < e:
                x = T.t[j]
                if x in "([":
                    d += 1
                elif x in ")]":
                    if d == 0:
                        break
                    d -= 1
                elif x in (";", ",") and d == 0:
                    break
                j += 1
            rhs = T.t[k + 2:j]
            n += 1
            for sign, term in split_add(rhs)[:1]:
                s, how, _, _ = resolve(u, f, term, depth + 1, seen)
                s = s or ("@scratchpad" if how.startswith("scratchpad") else None)
                if s:
                    ev.add(s)
                    k += 1
    return ev, (k, n)


def leaf(s, off, size, signed, path=""):
    """member at off in struct s: (match, path)."""
    L = allstructs.get(s)
    if not L:
        return "no-layout", ""
    if off >= L["size"] or off < 0:
        return "beyond", f"{path}+0x{off:X}>=0x{L['size']:X}"
    best = None
    for m in L["members"]:
        if m["off"] <= off < m["off"] + m["size"]:
            best = m
            if not L.get("union"):
                break
    if not best:
        return "hole", path
    m = best
    p = f"{path}.{m['name']}" if path else m["name"]
    rel = off - m["off"]
    if m["arr"]:
        es = m["esize"] or 1
        idx, rel = divmod(rel, es)
        p += f"[{idx}]"
        if m["base"] in allstructs and not m["stars"]:
            return leaf(m["base"], rel, size, signed, p)
        if re.match(r"(unk|pad|field|filler)_?", m["name"]) and es == 1 and m["base"] in ("u8", "s8", "char"):
            return "pad", p.split("[")[0] + f"[0x{off - m['off']:X}]"
        msz, msg = es, (scal(m["base"], m["stars"]) or (es, 0))[1]
    elif m["base"] in allstructs and not m["stars"]:
        return leaf(m["base"], rel, size, signed, p)
    else:
        msz = m["size"]
        msg = (scal(m["base"], m["stars"]) or (msz, 0))[1]
    if rel != 0:
        return "inside", p
    if size is None:
        return "aggregate", p
    if msz != size:
        return "size", p
    if signed is not None and msg != signed and not m["stars"]:
        return "sign", p
    return "exact", p


