"""citems.py: split a C file into top-level items (whole-line spans) for the restructure splitters.

Item = (lead, start, end, kind, name, names): lines are 1-based; `lead` is the first line of the
comments / blank lines in front of the item (they travel with it); [start, end] the item's own lines.
kind: 'func' (definition with a body), 'stub' (a top-level BIOS_x_FUNCTION / INCLUDE_ASM invocation),
'pp' (preprocessor line), 'decl' (anything ending in ';' at file scope: externs, prototypes, typedefs,
definitions of data). `name` is the function / stub / macro name; `names` the identifiers a decl
declares (heuristic: declarator names, typedef names, struct/union tags)."""
import re

KEYW = set("""auto break case char const continue default do double else enum extern float for goto if inline
int long register return short signed sizeof static struct switch typedef union unsigned void volatile while
__inline__ __volatile__ __asm__ asm""".split())


def strip_cs(s):
    """comments and string/char literals blanked (same length, newlines kept)."""
    out, i, n = [], 0, len(s)
    while i < n:
        c = s[i]
        if s.startswith("/*", i):
            j = s.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", s[i:j]))
            i = j
        elif s.startswith("//", i):
            j = s.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and s[j] != c:
                j += 2 if s[j] == "\\" else 1
            out.append(c + re.sub(r"[^\n]", " ", s[i + 1:j]) + c)
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def items(text):
    lines = text.split("\n")
    clean = strip_cs(text)
    cl = clean.split("\n")
    res = []
    i = 0
    nl = len(lines)
    lead = None
    while i < nl:
        if cl[i].strip() == "":
            if lead is None:
                lead = i
            i += 1
            continue
        start = i
        if cl[i].lstrip().startswith("#"):
            end = i
            while lines[end].rstrip().endswith("\\"):
                end += 1
            m = re.match(r"\s*#\s*(\w+)\s*(\w*)", lines[i])
            res.append(dict(lead=(lead if lead is not None else start) + 1, start=start + 1, end=end + 1,
                            kind="pp", name=m.group(2) if m.group(1) == "define" else "#" + m.group(1),
                            names={m.group(2)} if m.group(1) == "define" else set()))
            lead = None
            i = end + 1
            continue
        depth = paren = 0
        seen_body = False
        j = i
        k = 0
        buf = []
        done = None
        while j < nl and done is None:
            line = cl[j]
            for ch in line:
                if ch == "(":
                    paren += 1
                elif ch == ")":
                    paren -= 1
                elif ch == "{":
                    depth += 1
                    if depth == 1:
                        head = "".join(buf)
                        if (re.search(r"\)\s*$", head) and not re.match(r"\s*(typedef|struct|union|enum)\b", head)
                                and "=" not in head):
                            seen_body = True
                elif ch == "}":
                    depth -= 1
                    if depth == 0 and seen_body:
                        done = "func"
                        break
                elif ch == ";" and depth == 0 and paren == 0:
                    done = "decl"
                    break
                buf.append(ch)
            buf.append("\n")
            j += 1
        end = j - 1
        src = "\n".join(cl[start:end + 1])
        if done == "func":
            head = src[:src.index("{")]
            m = re.search(r"(\w+)\s*\([^()]*(\([^()]*\)[^()]*)*\)\s*$", head)
            name = m.group(1)
            kind = "func"
            names = {name}
        else:
            m = re.match(r"\s*(BIOS_[ABC]_FUNCTION|INCLUDE_ASM)\s*\(\s*(\"[^\"]*\"\s*,\s*)?(\w+)", src)
            if m:
                kind, name, names = "stub", m.group(3), {m.group(3)}
            else:
                kind, name, names = "decl", None, decl_names(src)
        res.append(dict(lead=(lead if lead is not None else start) + 1, start=start + 1, end=end + 1, kind=kind,
                        name=name, names=names))
        lead = None
        i = end + 1
    return res, lines


def decl_names(src):
    s = src
    names = set()
    for m in re.finditer(r"\b(struct|union|enum)\s+(\w+)", s):
        names.add(m.group(2))
    # drop brace bodies
    while True:
        t = re.sub(r"\{[^{}]*\}", " ", s)
        if t == s:
            break
        s = t
    # drop initializers
    s = re.sub(r"=[^,;]*", " ", s)
    # drop parameter lists: '(' after an identifier or ')' whose content does not start with '*'
    while True:
        t = re.sub(r"(\w|\))\s*\((?!\s*\*)[^()]*\)", r"\1", s)
        if t == s:
            break
        s = t
    s = re.sub(r"\[[^\]]*\]", "[]", s)
    for m in re.finditer(r"([A-Za-z_]\w*)\s*(?=[;,\[\)])", s):
        if m.group(1) not in KEYW:
            names.add(m.group(1))
    return names


def proto(lines, it):
    """a prototype for a function item: its header up to '{', plus ';'."""
    text = "\n".join(lines[it["start"] - 1:it["end"]])
    head = text[:strip_cs(text).index("{")].rstrip()
    return head + ";"
