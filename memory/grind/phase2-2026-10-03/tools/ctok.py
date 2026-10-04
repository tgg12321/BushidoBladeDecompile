"""ctok.py: a small C tokenizer and declaration reader for the Phase 2 census tools (casts.py, protos.py).
Works on comment/string-blanked text (citems.strip_cs), so offsets and line numbers match the source."""
import bisect, re

TOK = re.compile(r"""
  (?P<id>[A-Za-z_]\w*)
 |(?P<num>0[xX][0-9A-Fa-f]+[uUlL]*|\d+[uUlL]*)
 |(?P<op>->|\+\+|--|<<=|>>=|<<|>>|<=|>=|==|!=|&&|\|\||[-+*/%&|^]=|\.\.\.|[][(){};,.?:~!<>=+\-*/%&|^#'"])
""", re.X)
BASE_KW = {"void", "char", "short", "int", "long", "signed", "unsigned", "const", "volatile", "struct",
           "union", "enum", "register", "static", "extern", "auto"}
EXPR_KW = {"return", "else", "case", "do", "sizeof", "goto", "if", "while", "for", "switch"}
SCALAR = {"u8": (1, 0), "s8": (1, 1), "u16": (2, 0), "s16": (2, 1), "u32": (4, 0), "s32": (4, 1),
          "u64": (8, 0), "s64": (8, 1), "vu8": (1, 0), "vs8": (1, 1), "vu16": (2, 0), "vs16": (2, 1),
          "vu32": (4, 0), "vs32": (4, 1), "char": (1, 0), "short": (2, 1), "int": (4, 1), "long": (4, 1),
          "u_char": (1, 0), "u_short": (2, 0), "u_long": (4, 0), "u_int": (4, 0)}


class Toks:
    def __init__(self, clean):
        self.t, self.pos = [], []
        for m in TOK.finditer(clean):
            self.t.append(m.group(0))
            self.pos.append(m.start())
        self.nl = [i for i, c in enumerate(clean) if c == "\n"]

    def line(self, k):
        return bisect.bisect_right(self.nl, self.pos[k]) + 1

    def match(self, k):
        """index of the bracket closing the one at k."""
        o = self.t[k]
        c = {"(": ")", "[": "]", "{": "}"}[o]
        d = 0
        for j in range(k, len(self.t)):
            if self.t[j] == o:
                d += 1
            elif self.t[j] == c:
                d -= 1
                if d == 0:
                    return j
        return len(self.t) - 1


def typedef_names(clean):
    names = set()
    for m in re.finditer(r"\btypedef\b([^;{]*(\{)?)", clean):
        if m.group(2):
            # typedef struct ... { ... } NAME, *PNAME;
            d, j = 0, m.end() - 1
            while j < len(clean):
                if clean[j] == "{":
                    d += 1
                elif clean[j] == "}":
                    d -= 1
                    if d == 0:
                        break
                j += 1
            k = clean.find(";", j)
            for n in re.findall(r"[A-Za-z_]\w*", clean[j + 1:k]):
                names.add(n)
        else:
            k = clean.find(";", m.start())
            s = clean[m.start():k]
            mm = re.search(r"\(\s*\*\s*(\w+)\s*\)", s)
            if mm:
                names.add(mm.group(1))
            else:
                ids = re.findall(r"[A-Za-z_]\w*", re.sub(r"\[[^\]]*\]", "", s))
                if ids:
                    names.add(ids[-1])
    return names


def is_type_start(tok, tnames):
    return tok in BASE_KW or tok in tnames


def read_type(T, k, tnames):
    """at token k a type-specifier run starts; return (base_type_text, k_after) or None."""
    j, parts = k, []
    while j < len(T.t):
        x = T.t[j]
        if x in ("struct", "union", "enum"):
            parts.append(x)
            if j + 1 < len(T.t) and re.match(r"[A-Za-z_]", T.t[j + 1]):
                parts.append(T.t[j + 1])
                j += 2
            else:
                j += 1
            if j < len(T.t) and T.t[j] == "{":
                j = T.match(j) + 1
                parts.append("{...}")
            continue
        if x in BASE_KW or (x in tnames and not any(p in tnames for p in parts)):
            if x not in ("register", "static", "extern", "auto", "const", "volatile"):
                parts.append(x)
            j += 1
            continue
        break
    if not parts:
        return None
    return " ".join(parts), j


def read_declarators(T, k, end_tok=";"):
    """declarators from token k to the ';' (or ')' / ',' for parameters): [(name, stars, is_array, init_span)]."""
    out = []
    j = k
    while j < len(T.t):
        stars = 0
        while j < len(T.t) and T.t[j] in ("*", "const", "volatile"):
            stars += T.t[j] == "*"
            j += 1
        name = None
        if j < len(T.t) and T.t[j] == "(" and j + 1 < len(T.t) and T.t[j + 1] == "*":
            # function pointer / pointer-to-array declarator
            e = T.match(j)
            ids = [x for x in T.t[j:e] if re.match(r"[A-Za-z_]", x)]
            name = ids[0] if ids else None
            stars += 1
            j = e + 1
            if j < len(T.t) and T.t[j] == "(":
                j = T.match(j) + 1
        elif j < len(T.t) and re.match(r"[A-Za-z_]", T.t[j]):
            name = T.t[j]
            j += 1
        arr = False
        while j < len(T.t) and T.t[j] == "[":
            arr = True
            j = T.match(j) + 1
        if j < len(T.t) and T.t[j] == "(":   # prototype
            j = T.match(j) + 1
            out.append((name, stars, arr, None, True))
        init = None
        if j < len(T.t) and T.t[j] == "=":
            s = j + 1
            d = 0
            while j < len(T.t):
                x = T.t[j]
                if x in "([{":
                    d += 1
                elif x in ")]}":
                    if d == 0:
                        break
                    d -= 1
                elif x in (",", ";") and d == 0:
                    break
                j += 1
            init = (s, j)
        if name and not (out and out[-1][0] == name):
            out.append((name, stars, arr, init, False))
        if j < len(T.t) and T.t[j] == ",":
            j += 1
            continue
        break
    return out, j
