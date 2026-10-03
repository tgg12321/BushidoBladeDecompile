"""types.py: every typedef / struct / union / enum / #define defined in src/**/*.c, grouped by name, with
the files that define it and whether the definitions are textually identical (comments stripped)."""
import re, glob, collections
def strip(s):
    s = re.sub(r"/\*.*?\*/", "", s, flags=re.S)
    s = re.sub(r"//[^\n]*", "", s)
    return s
defs = collections.defaultdict(list)
for f in sorted(glob.glob("src/**/*.c", recursive=True)):
    t = strip(open(f, encoding="utf-8").read())
    # top-level typedef ... ; possibly multi-line with braces
    i, n, depth = 0, len(t), 0
    stmt_start = 0
    while i < n:
        c = t[i]
        if c == "{": depth += 1
        elif c == "}": depth -= 1
        elif c == ";" and depth == 0:
            st = t[stmt_start:i+1].strip()
            stmt_start = i + 1
            m = re.match(r"(typedef\b.*)", st, re.S)
            if m:
                body = " ".join(m.group(1).split())
                nm = re.findall(r"(\w+)\s*(?:\[[^\]]*\])*\s*;$", body)
                nm2 = re.search(r"\(\s*\*\s*(\w+)\s*\)", body)
                name = nm2.group(1) if nm2 and body.rstrip(";").endswith(")") else (nm[0] if nm else "?")
                defs[name].append((f, body))
            elif re.match(r"(struct|union|enum)\s+\w+\s*\{", st):
                body = " ".join(st.split())
                name = re.match(r"(?:struct|union|enum)\s+(\w+)", st).group(1)
                defs["struct " + name].append((f, body))
        elif c == "\n" and depth == 0 and t[stmt_start:i].strip().startswith("#"):
            stmt_start = i + 1
        elif c == "}" and depth == 0:
            pass
        if c == "}" and depth == 0:
            # end of a function body or a struct body; function bodies end a statement
            st = t[stmt_start:i+1]
            if re.search(r"\)\s*\{", st) and not st.lstrip().startswith(("typedef", "struct", "union", "enum")):
                stmt_start = i + 1
        i += 1
for name in sorted(defs, key=lambda k: (-len(defs[k]), k)):
    v = defs[name]
    bodies = collections.Counter(b for _, b in v)
    print(f"{name}: {len(v)} file(s), {len(bodies)} spelling(s)")
    for f, b in v:
        print(f"    {f[4:]}  [{list(bodies).index(b)}]")
    if len(bodies) > 1:
        for k, b in enumerate(bodies):
            print(f"      [{k}] {b[:300]}")
