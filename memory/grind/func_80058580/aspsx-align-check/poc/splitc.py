#!/usr/bin/env python3
"""splitc.py <src/X.c> <cut_line> <src/Y.c> [--dry]: split a C translation unit at a line.

Lines [cut_line..] move VERBATIM to Y.c. X.c keeps lines [..cut_line). Y.c's header is:
  - X.c's leading include block (the #define/#include lines before the first other line), then
  - every file-scope DECLARATION statement of X.c's first part that Y.c needs (transitively:
    extern declarations, prototypes, typedefs, struct/union/enum definitions, active #defines),
    verbatim and in original order, then
  - for a function or object DEFINED in the first part and used by Y.c, a declaration derived
    from its definition (ANSI header + ';', `ret name();` for a K&R header, `extern <declarator>;`
    for an object). A `static` one is reported and aborts (that cut is not a TU boundary).
Nothing else is added, removed or changed."""
import re, sys

KEYWORDS = set("""auto break case char const continue default do double else enum extern float for
goto if int long register return short signed sizeof static struct switch typedef union unsigned
void volatile while asm __asm__ inline __inline__""".split())


def blank(src):
    """Replace comments and string/char literal contents with spaces (newlines kept)."""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if src.startswith("/*", i):
            j = src.find("*/", i + 2); j = n if j < 0 else j + 2
            out.append("".join(ch if ch == "\n" else " " for ch in src[i:j])); i = j
        elif src.startswith("//", i):
            j = src.find("\n", i); j = n if j < 0 else j
            out.append(" " * (j - i)); i = j
        elif c in "\"'":
            j = i + 1
            while j < n and src[j] != c:
                j += 2 if src[j] == "\\" else 1
            out.append(c + "".join(ch if ch == "\n" else " " for ch in src[i + 1:j]) + c); i = j + 1
        else:
            out.append(c); i += 1
    return "".join(out)


def statements(raw, lo, hi):
    """File-scope statements of raw lines [lo, hi) (0-based) as dicts: kind, a, b (line range),
    text (blanked, joined)."""
    L = blank(raw).split("\n")
    res, depth, cur, start, knr = [], 0, "", None, False
    i = lo
    while i < hi:
        line = L[i]
        if depth == 0 and not cur.strip() and line.strip().startswith("#"):
            j = i
            while L[j].rstrip().endswith("\\") and j + 1 < hi:
                j += 1
            res.append({"kind": "pp", "a": i, "b": j, "text": " ".join(" ".join(L[i:j + 1]).replace("\\", " ").split())})
            i = j + 1
            continue
        for ch in line:
            if start is None and not ch.isspace():
                start = i
            cur += ch
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
            if depth == 0 and ch in ";}":
                t = " ".join(cur.split())
                if ch == "}":
                    head = t.split("{")[0]
                    if "(" in head and "=" not in head and not re.match(r"(typedef|struct|union|enum)\b", t):
                        res.append({"kind": "func", "a": start, "b": i, "text": t}); cur, start, knr = "", None, False
                    continue  # struct/initializer body: wait for ';'
                # K&R header: `type name(a, b) T a; T b; {` -- keep accumulating until the body closes
                if re.match(r"^[\w\s\*]*?\b\w+\s*\(\s*\w+(\s*,\s*\w+)*\s*\)\s*[^;{\s][^;{]*;$", t) and "=" not in t \
                        and not re.match(r"(extern|typedef)\b", t) or knr:
                    knr = True
                    continue
                res.append({"kind": "stmt", "a": start, "b": i, "text": t}); cur, start = "", None
            if ch == "{" and knr:
                knr = False  # body started; the '}' branch will close it as a function
        cur += "\n"
        i += 1
    assert not cur.strip(), "unterminated statement at end: %r" % cur.strip()[:200]
    return res


def idents(t):
    return set(re.findall(r"[A-Za-z_]\w*", t)) - KEYWORDS


def declared(st, typenames):
    t = st["text"]
    if st["kind"] == "pp":
        m = re.match(r"#\s*(define|undef)\s+(\w+)", t)
        return {m.group(2)} if m else set()
    if st["kind"] == "func":
        head = t.split("{")[0]
        return {re.findall(r"(\w+)\s*\(", head)[0]}
    if re.match(r"INCLUDE_(ASM|RODATA)\b", t):
        m = re.search(r",\s*(\w+)\s*\)", t)
        return {m.group(1)} if m else set()
    names = set()
    body = re.sub(r"\{[^{}]*\}", " ", t)
    while "{" in body:
        body = re.sub(r"\{[^{}]*\}", " ", body)
    for m in re.finditer(r"\b(struct|union|enum)\s+(\w+)", t.split("{")[0] if "{" in t else ""):
        names.add(m.group(2))
    if re.match(r"enum\b", t) and "{" in t:
        inner = t[t.index("{") + 1:t.rindex("}")]
        names |= {x.split("=")[0].strip() for x in inner.split(",") if x.strip()}
    if t.startswith("typedef"):
        m = re.search(r"\(\s*\*+\s*(\w+)\s*\)", body)
        if m:
            return names | {m.group(1)}
        ids = [x for x in re.findall(r"[A-Za-z_]\w*", re.sub(r"\[[^\]]*\]", " ", body)) if x not in KEYWORDS]
        return names | ({ids[-1]} if ids else set())
    decl = body.split("=")[0].rstrip(";")
    # Declared name of each declarator, independent of type names (header typedefs such as s32
    # are unknown here): `(*name)` -> name; `name(...)` -> the identifier before the first '(';
    # otherwise the last identifier once array bounds are removed.
    parts = re.split(r",(?![^()]*\))", decl)
    for p in parts:
        p = re.sub(r"\[[^\]]*\]", " ", p)
        m = re.search(r"\(\s*\*+\s*(\w+)\s*\)", p)
        if m:
            names.add(m.group(1)); continue
        if "(" in p:
            ids = re.findall(r"[A-Za-z_]\w*", p[:p.index("(")])
        else:
            ids = re.findall(r"[A-Za-z_]\w*", p)
        ids = [x for x in ids if x not in KEYWORDS]
        if ids:
            names.add(ids[-1])
    return names


def main():
    path, cut, out = sys.argv[1], int(sys.argv[2]) - 1, sys.argv[3]
    dry = "--dry" in sys.argv
    raw = open(path).read()
    lines = raw.split("\n")
    rawl = raw.split("\n")
    n_inc = 0
    while n_inc < len(lines) and (lines[n_inc].startswith("#include") or lines[n_inc].startswith("#define INCLUDE_ASM_USE_MACRO_INC")):
        n_inc += 1
    sts = statements(raw, n_inc, cut)
    typenames = set()
    for st in sts:
        if st["text"].startswith("typedef"):
            typenames |= declared(st, typenames)
    tail = "\n".join(lines[cut:])
    need = idents(blank(tail))
    # macro state at the cut
    active = {}
    for st in sts:
        if st["kind"] == "pp":
            m = re.match(r"#\s*(define|undef)\s+(\w+)", st["text"])
            if m:
                active[m.group(2)] = st if m.group(1) == "define" else None
            elif re.match(r"#\s*(if|ifdef|ifndef|else|elif|endif)", st["text"]):
                raise SystemExit("conditional compilation in the first part (line %d): handle by hand" % (st["a"] + 1))
    chosen = {}
    changed = True
    while changed:
        changed = False
        for idx, st in enumerate(sts):
            if idx in chosen:
                continue
            names = declared(st, typenames)
            if not names & need:
                continue
            if st["kind"] == "pp":
                m = re.match(r"#\s*define\s+(\w+)", st["text"])
                if not m or active.get(m.group(1)) is not st:
                    continue
                emit = "\n".join(rawl[st["a"]:st["b"] + 1])
                deps = idents(st["text"])
            elif st["kind"] == "func":
                head = "\n".join(rawl[st["a"]:st["b"] + 1])
                bh = st["text"].split("{")[0].strip()
                if re.match(r"(static)\b", bh):
                    raise SystemExit("static function %s used across the cut" % names)
                m = re.match(r"^(.*?\b(\w+)\s*\()(.*?)\)(.*)$", bh)
                params, trailer = m.group(3), m.group(4).strip()
                if trailer:  # K&R
                    emit = m.group(1).strip() + ");"
                else:
                    orig_head = head[:head.index("{")].rstrip()
                    emit = orig_head + ";"
                deps = idents(bh if not trailer else m.group(1))
            elif re.match(r"INCLUDE_(ASM|RODATA)\b", st["text"]):
                continue  # an INCLUDE_ASM function used across the cut is declared by the headers/undefined syms
            else:
                t = st["text"]
                if re.match(r"(extern|typedef)\b", t) or re.match(r"(struct|union|enum)\s+\w+\s*\{", t) \
                        or (re.search(r"\)\s*;$", t) and "=" not in t and "{" not in t):
                    emit = "\n".join(rawl[st["a"]:st["b"] + 1])
                    deps = idents(t)
                else:  # object definition
                    if re.match(r"static\b", t):
                        raise SystemExit("static object %s used across the cut" % names)
                    d = re.split(r"\s=\s|=(?!=)", t, maxsplit=1)[0].rstrip(";").strip()
                    emit = "extern " + d + ";"
                    deps = idents(d)
            chosen[idx] = emit
            need |= deps
            changed = True
    hdr = lines[:n_inc]
    body, seen = [], set()
    for k in sorted(chosen):
        if chosen[k] not in seen:
            seen.add(chosen[k]); body.append(chosen[k])
    new = "\n".join(hdr) + "\n\n/* Declarations from the file this TU was split from (%s). */\n" % path.split("/")[-1] \
        + "\n".join(body) + "\n\n" + tail
    if dry:
        print("\n".join(body)); return
    open(out, "w", newline="\n").write(new)
    open(path, "w", newline="\n").write("\n".join(lines[:cut]).rstrip("\n") + "\n")
    print("%s: kept %d lines; %s: %d moved lines + %d declarations" % (path, cut, out, len(lines) - cut, len(body)))


main()
