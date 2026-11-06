#!/usr/bin/env python3
"""Constants-wave applier and K3 checker (.claude/rules/constants-bar.md, owner ruling Q135).

A wave manifest is a TSV with the header `kind<TAB>where<TAB>literal<TAB>name<TAB>role<TAB>evidence`:

  site    src/main/foo.c:123   0x10    PADLup        pad bit tested ...   libetc.h (PsyQ 4.0)
  define  include/psxsdk/libetc.h  0x10  PADLup      pad bit              libetc.h (PsyQ 4.0)
  enum    include/bb2_const.h  3       GameMode.GAME_MODE_VS  mode code   trace ...

- `site`: the literal token at file:line (the line in the tree the wave starts from) becomes NAME.
- `define`: `#define NAME <literal>` in the file (reused if the file already defines NAME the
  same way; a different body is refused).
- `enum`: member NAME = <literal> of `enum Tag` (name column `Tag.NAME`) in the file.

`apply` substitutes the sites (token-exact; refusing a literal that is not one integer / char
token, not unique on its line, on a preprocessor line, inside a string literal or an __asm__
statement, or in a .s / .inc file) and then writes the definitions.

`check` proves the applied diff (--base rev .. --new rev, or the working tree):
  1. every changed token in src/ and include/ is explained by the manifest: one site row per
     literal -> NAME replacement, inserted lines only #define / enum / #include (a tracked
     header holding one of its rows) for its rows; no comment added, changed or moved; no other
     path outside docs/ changes. Sources are read as cpp reads them (CR and CRLF are newlines;
     backslash, blanks, newline is a splice joining with nothing; a directive is compared as
     its token sequence with literals verbatim and whitespace as present / absent), and a
     file this reader could misread is refused: a wave may not add or remove a splice, ??/ trigraph, CR,
     NUL, form feed, vertical tab or __has_include, and a splice is accepted only inside a
     directive's code (never in a comment, a string or a code line);
  2. K3 per TU, over every TU of the tree (cpp's line markers decide what reaches what): the
     cpp output equals the base's apart from line markers and layout, or -- only for a TU
     reached by an enum row, a define whose body is not the site's own token, or an added
     #include -- the cc1 .s output equals the base's. No TU may read a changed docs/ file.
Runs in WSL (the Makefile's cpp and cc1). Exit 0 = OK, 1 = FAIL, 2 = no verdict (bad input or
toolchain). `lanes.py land` runs `check` on every `naming: const-*` commit (strict; with
--loose-lines when it was replayed onto a moved main); its manifest is
docs/naming/phase3/pending/<wave>/const.tsv.
"""
from __future__ import annotations

import argparse
import bisect
import difflib
import os
import re
import shutil
import subprocess
import sys
import uuid
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(os.environ.get("CONST_WAVE_ROOT") or Path(__file__).resolve().parent.parent).resolve()
MANIFEST_FILE = "const.tsv"
MANIFEST_DIR = "docs/naming/phase3/pending/{}/"
HEADER = ["kind", "where", "literal", "name", "role", "evidence"]
KINDS = ("site", "define", "enum")
IDENT = re.compile(r"[A-Za-z_]\w*\Z")
INT_LITERAL = re.compile(r"(?:0[xX][0-9A-Fa-f]+|\d+)[uUlL]*\Z")
CHAR_LITERAL = re.compile(r"'(?:\\.|[^'\\\n])+'\Z")
TOKEN = re.compile(r"""
    (?P<str>"(?:\\.|[^"\\\n])*")
  | (?P<chr>'(?:\\.|[^'\\\n])*')
  | (?P<num>\.?\d(?:[eEpP][+-]|[\w.])*)
  | (?P<id>[A-Za-z_]\w*)
  | (?P<punct>\.\.\.|<<=|>>=|->|\+\+|--|<<|>>|<=|>=|==|!=|&&|\|\||[-+*/%&|^!=<>]=|\#\#|[^\s])
""", re.X)
ASM_WORDS = {"asm", "__asm", "__asm__"}


class Refuse(Exception):
    """Bad input or a refused site: no verdict (exit 2)."""


# ---------------------------------------------------------------- manifest

@dataclass
class Row:
    kind: str
    where: str
    literal: str
    name: str
    role: str
    evidence: str
    lineno: int  # manifest line, for messages

    @property
    def path(self) -> str:
        return self.where.rsplit(":", 1)[0] if self.kind == "site" else self.where

    @property
    def line(self) -> int:
        return int(self.where.rsplit(":", 1)[1])

    @property
    def tag(self) -> str | None:
        return self.name.split(".", 1)[0] if self.kind == "enum" else None

    @property
    def ident(self) -> str:
        return self.name.split(".", 1)[1] if self.kind == "enum" else self.name


def parse_manifest(text: str) -> list[Row]:
    rows, seen_header = [], False
    for n, raw in enumerate(text.splitlines(), 1):
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        cols = raw.split("\t")
        if not seen_header:
            if [c.strip() for c in cols] != HEADER:
                raise Refuse(f"manifest line {n}: header must be {' '.join(HEADER)}")
            seen_header = True
            continue
        if len(cols) != len(HEADER):
            raise Refuse(f"manifest line {n}: {len(cols)} columns, want {len(HEADER)}")
        row = Row(*[c.strip() for c in cols], lineno=n)
        if row.kind not in KINDS:
            raise Refuse(f"manifest line {n}: kind {row.kind!r} is not one of {KINDS}")
        if not row.role or not row.evidence:
            raise Refuse(f"manifest line {n}: role and evidence are required (K5)")
        path = row.path
        if path.startswith("/") or ".." in Path(path).parts or "\\" in path:
            raise Refuse(f"manifest line {n}: unsafe path {path!r}")
        if not path.endswith((".c", ".h")):
            raise Refuse(f"manifest line {n}: {path}: only .c / .h sources (never .s / .inc)")
        if row.kind == "site":
            if not re.fullmatch(r"[^:]+:\d+", row.where):
                raise Refuse(f"manifest line {n}: site needs file:line, got {row.where!r}")
            if not literal_token(row.literal):
                raise Refuse(f"manifest line {n}: site literal {row.literal!r} is not one "
                             "integer or character token (a macro or expression is refused)")
            if not IDENT.match(row.name):
                raise Refuse(f"manifest line {n}: name {row.name!r} is not an identifier")
        elif row.kind == "define":
            if not IDENT.match(row.name) or not row.literal:
                raise Refuse(f"manifest line {n}: define needs NAME and a body")
        else:
            parts = row.name.split(".")
            if len(parts) != 2 or not all(IDENT.match(p) for p in parts):
                raise Refuse(f"manifest line {n}: enum name must be Tag.NAME, got {row.name!r}")
            if not literal_token(row.literal):
                raise Refuse(f"manifest line {n}: enum value {row.literal!r} is not one literal")
        rows.append(row)
    if not seen_header:
        raise Refuse("manifest has no header row")
    defs = {}
    for r in rows:
        if r.kind != "site":
            if r.ident in defs and (defs[r.ident].literal, defs[r.ident].path) != (r.literal, r.path):
                raise Refuse(f"manifest line {r.lineno}: {r.ident} defined twice differently (K4)")
            defs[r.ident] = r
    for r in rows:
        if r.kind == "site" and r.name not in defs:
            raise Refuse(f"manifest line {r.lineno}: site name {r.name} has no define / enum row")
    for r in rows:
        d = defs.get(r.name) if r.kind == "site" else None
        if d is not None and literal_token(d.literal) and literal_value(d.literal) != literal_value(r.literal):
            raise Refuse(f"manifest line {r.lineno}: site {r.literal} is not {d.ident}'s value "
                         f"{d.literal}")
    sites = [(r.path, r.line, r.literal) for r in rows if r.kind == "site"]
    if len(sites) != len(set(sites)):
        raise Refuse("manifest lists the same site twice")
    return rows


def literal_token(text: str) -> bool:
    return bool(INT_LITERAL.match(text) or CHAR_LITERAL.match(text))


def literal_value(text: str) -> int | str:
    if CHAR_LITERAL.match(text):
        return text
    digits = text.rstrip("uUlL")
    return int(digits, 16) if digits[:2] in ("0x", "0X") else int(digits, 8 if digits.startswith("0") and len(digits) > 1 else 10)


def definitions(rows: list[Row]) -> dict[str, Row]:
    return {r.ident: r for r in rows if r.kind != "site"}


def needs_cc1(defn: Row) -> bool:
    """K3: an enum, or a #define whose body is not one token equal to the site's own literal
    (sites are checked against the body), needs cc1 identity rather than cpp identity."""
    return defn.kind == "enum" or [t for _, t, _ in tokens(defn.literal)] != [defn.literal]


# ---------------------------------------------------------------- C tokens

SPLICE = re.compile(r"\\[ \t\f\v]*\n")
# spellings a wave may not add or remove: this reader and cpp could disagree on them
UNREADABLE = (("\r", "carriage return"), ("??/", "??/ trigraph (a splice)"), ("\0", "NUL"),
              ("\f", "form feed"), ("\v", "vertical tab"), ("__has_include", "__has_include"))


def cpp_text(raw: str) -> str:
    """Line ends as cpp reads them: CRLF and a lone CR are newlines."""
    return raw.replace("\r\n", "\n").replace("\r", "\n")


def misread_splices(text: str) -> list[int]:
    """1-based lines of `text` (cpp_text'ed) holding a splice this reader would not see as cpp
    does: one in a comment, in a string / char literal, or on a non-directive line. A splice
    in a directive's code continues the directive, which directive_lines() follows."""
    clean = blank_comments(text)
    dirs = directive_lines(clean, comment_joined_lines(text))
    in_lit, i, n = set(), 0, len(clean)
    while i < n:
        c = clean[i]
        if c in "\"'":
            j = i + 1
            while j < n and clean[j] != c and clean[j] != "\n":
                j += 2 if clean[j] == "\\" else 1
            in_lit.update(range(i, min(j + 1, n)))
            i = j + 1
        else:
            i += 1
    bad = []
    for m in SPLICE.finditer(text):
        p = m.start()
        line = text.count("\n", 0, p) + 1
        if clean[p] != "\\" or p in in_lit or line not in dirs:
            bad.append(line)
    return bad


def blank_comments(text: str) -> str:
    """Comments become spaces (newlines kept); strings and chars stay."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", text[i:j]))
            i = j
        elif text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def tokens(text: str) -> list[tuple[int, str, str]]:
    """(line, token, group) for comment-free C text; line continuations join lines."""
    out = []
    line_starts = [0] + [m.end() for m in re.finditer(r"\n", text)]
    for m in TOKEN.finditer(text):
        if m.group(0) == "\\":
            continue
        out.append((bisect.bisect_right(line_starts, m.start()), m.group(0), m.lastgroup))
    return out


def comment_joined_lines(text: str) -> set[int]:
    """1-based lines whose newline lies inside a /* */ comment. cpp turns the comment into one
    space, so the line continues past that newline (a directive keeps going too)."""
    joined, i, n = set(), 0, len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            i = j + 1
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            line = text.count("\n", 0, i) + 1
            for k in range(text.count("\n", i, j)):
                joined.add(line + k)
            i = j
        elif text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        else:
            i += 1
    return joined


def directive_lines(text: str, joined=frozenset()) -> set[int]:
    """1-based lines that belong to a preprocessor directive: continued by a backslash, or by
    a comment that opens on the directive and closes on a later line (`joined`), as in cpp."""
    lines, cont = set(), False
    for n, line in enumerate(text.split("\n"), 1):
        if cont or line.lstrip().startswith("#"):
            lines.add(n)
            cont = line.rstrip().endswith("\\") or n in joined
        else:
            cont = False
    return lines


SPLICE_TAIL = re.compile(r"\\[ \t\f\v]*\Z")


def directive_key(logical: str) -> str:
    """A directive as cpp sees it: its tokens in order (string and char literals verbatim),
    one space where whitespace or a comment separated two tokens, nothing where none did."""
    parts, end = [], 0
    for m in TOKEN.finditer(logical):
        parts.append((" " if parts and m.start() > end else "") + m.group(0))
        end = m.end()
    return re.sub(r"^# ", "#", "".join(parts))


def comments(raw: str) -> list[tuple[int, int, str]]:
    """(offset, 1-based line, whitespace-normalised text) of every comment, as blank_comments
    finds them; adjacent // comments merge into one."""
    out, i, n = [], 0, len(raw)
    while i < n:
        c = raw[i]
        if c in "\"'":
            j = i + 1
            while j < n and raw[j] != c and raw[j] != "\n":
                j += 2 if raw[j] == "\\" else 1
            i = j + 1
        elif raw.startswith("/*", i) or raw.startswith("//", i):
            if raw[i + 1] == "*":
                j = raw.find("*/", i + 2)
                j = n if j < 0 else j + 2
            else:
                j = raw.find("\n", i)
                j = n if j < 0 else j
            line = raw.count("\n", 0, i) + 1
            if raw[i + 1] == "/":
                words = raw[i + 2:j].split()
                prev = out[-1] if out else None
                # a run of // comments with only blanks between is one unit: clang-format
                # (ReflowComments) splits a long trailing // comment onto the next line
                if (prev and prev[3] and not raw[prev[4]:i].strip()
                        and raw.count("\n", prev[4], i) <= 1):  # next line, no blank between
                    out[-1] = (prev[0], prev[1], prev[2] + " " + " ".join(words), True, j)
                else:
                    out.append((i, line, "// " + " ".join(words), True, j))
            else:
                out.append((i, line, " ".join(raw[i:j].split()), False, j))
            i = j
        else:
            i += 1
    return [(off, ln, text.rstrip()) for off, ln, text, _, _ in out]


def units(raw: str) -> list[tuple[int, str, str]]:
    """The file as a diff sees it, in source order: each code token (cpp_text'ed input); each
    preprocessor directive (a logical line, as cpp extends it) as one unit, group 'dir', keyed
    by directive_key(), so a diff never splits or merges directives; and each comment as one
    unit, group 'cmt', so a comment cannot change or move unseen."""
    clean, joined = blank_comments(raw), comment_joined_lines(raw)
    dirs = directive_lines(clean, joined)
    out, seen = [], set()
    lines = clean.split("\n")
    line_starts = [0] + [m.end() for m in re.finditer(r"\n", clean)]

    def continues(k):  # 1-based line k carries on to line k + 1
        return lines[k - 1].rstrip().endswith("\\") or k in joined

    for m in TOKEN.finditer(clean):
        if m.group(0) == "\\":
            continue
        ln = bisect.bisect_right(line_starts, m.start())
        if ln not in dirs:
            out.append((m.start(), ln, m.group(0), m.lastgroup))
            continue
        start = ln
        while start - 1 in dirs and continues(start - 1):
            start -= 1
        if start in seen:
            continue
        seen.add(start)
        end = start
        while end < len(lines) and continues(end):
            end += 1
        logical = ""
        for k in range(start, end + 1):  # a splice joins with nothing, a comment with a space
            line = lines[k - 1]
            if k < end and SPLICE_TAIL.search(line):
                logical += SPLICE_TAIL.sub("", line)
            else:
                logical += line + (" " if k < end else "")
        out.append((line_starts[start - 1], start, directive_key(logical), "dir"))
    out += [(off, ln, text, "cmt") for off, ln, text in comments(raw)]
    return [(ln, text, grp) for _, ln, text, grp in sorted(out, key=lambda u: u[0])]


def asm_token_spans(toks) -> set[int]:
    """Indices of tokens inside an asm / __asm__ statement's parentheses."""
    inside = set()
    i = 0
    while i < len(toks):
        if toks[i][1] in ASM_WORDS:
            j = i + 1
            while j < len(toks) and toks[j][1] in ("volatile", "__volatile__", "__volatile", "goto"):
                j += 1
            if j < len(toks) and toks[j][1] == "(":
                depth, k = 0, j
                while k < len(toks):
                    depth += (toks[k][1] == "(") - (toks[k][1] == ")")
                    if depth == 0:
                        break
                    k += 1
                inside.update(range(i, k + 1))
                i = k
        i += 1
    return inside


# ---------------------------------------------------------------- apply

def site_index(text: str, line: int, literal: str, path: str) -> int:
    """Character offset of the one `literal` token on `line`, or Refuse."""
    clean = blank_comments(text)
    toks = tokens(clean)
    hits = [k for k, (ln, t, _) in enumerate(toks) if ln == line and t == literal]
    on_line = [t for ln, t, _ in toks if ln == line]
    where = f"{path}:{line}"
    if not on_line:
        raise Refuse(f"{where}: no code on this line")
    if any(g == "str" and literal in t for ln, t, g in toks if ln == line) and not hits:
        raise Refuse(f"{where}: {literal} is inside a string literal")
    if len(hits) != 1:
        raise Refuse(f"{where}: literal {literal} occurs {len(hits)} times as a token on the line "
                     "(must be exactly once)")
    if line in directive_lines(clean, comment_joined_lines(text)):
        raise Refuse(f"{where}: site is on a preprocessor line (already a macro)")
    if hits[0] in asm_token_spans(toks):
        raise Refuse(f"{where}: site is inside an __asm__ statement")
    # map back to an offset: the k-th token match on the comment-blanked text
    offsets = [m.start() for m in TOKEN.finditer(clean) if m.group(0) != "\\"]
    return offsets[hits[0]]


def defined_body(text: str, name: str) -> str | None:
    m = re.search(r"(?m)^[ \t]*#[ \t]*define[ \t]+" + re.escape(name) + r"\b(?!\()[ \t]*(.*)$",
                  blank_comments(text))
    return " ".join(m.group(1).split()) if m else None


def leading_slot(body: list[str], header: bool) -> int:
    """Index of the line after the file's leading directive block, before its first code line:
    after the last #include (a .c), or after the include guard's #define and the includes (a
    header), never inside an #if of that block. A merged TU's later include block (25C38.c,
    libgpu/sys.c) would put a definition below its uses."""
    clean = blank_comments("\n".join(body)).split("\n")
    depth = base = idx = 0
    cont, kw, first = False, "", True
    for k, ln in enumerate(clean):
        s = ln.strip()
        if cont:
            cont = s.endswith("\\")
        elif not s:
            continue
        elif not s.startswith("#"):
            break
        else:
            m = re.match(r"#\s*(\w+)", s)
            kw = m.group(1) if m else ""
            if kw in ("if", "ifdef", "ifndef"):
                if header and first and kw == "ifndef":
                    base = 1  # the include guard
                depth += 1
            elif kw == "endif":
                depth -= 1
            first = False
            cont = s.endswith("\\")
        if not cont and depth == base and (kw == "include" or header and kw == "define"):
            idx = k + 1
    return idx


def insert_block(text: str, block: list[str], header: bool, early: bool = False) -> str:
    """A header's block goes before its final #endif, or, when the header's own code uses a
    name of the block (`early`), at leading_slot(); a .c file's always at leading_slot()."""
    lines = text.split("\n")
    trailing = lines and lines[-1] == ""
    body = lines[:-1] if trailing else lines
    if header and not early:
        idx = len(body)
        for k in range(len(body) - 1, -1, -1):
            s = body[k].strip()
            if not s:
                continue
            if re.match(r"#\s*endif\b", s):
                idx = k
            break
    else:
        idx = leading_slot(body, header)
    new = [""] + block + ([""] if idx < len(body) and body[idx].strip() else [])
    out = body[:idx] + new + body[idx:]
    return "\n".join(out) + ("\n" if trailing else "")


def _tools_on_path():
    here = str(Path(__file__).resolve().parent)
    if here not in sys.path:
        sys.path.insert(0, here)


def _format_module():
    _tools_on_path()
    import format as fmt  # tools/format.py: pure-Python hoist step, no clang-format needed
    return fmt


def apply(rows: list[Row], read, write) -> list[str]:
    """Apply the manifest with read(path)->text / write(path, text). Returns the changed paths.
    Every refusal happens before any write."""
    defs = definitions(rows)
    texts: dict[str, str] = {}

    def get(path):
        if path not in texts:
            t = read(path)
            if t is None:
                raise Refuse(f"{path}: no such file")
            if "\r" in t:
                raise Refuse(f"{path}: CRLF line endings")
            texts[path] = t
        return texts[path]

    by_file: dict[str, list[Row]] = {}
    for r in rows:
        if r.kind == "site":
            by_file.setdefault(r.path, []).append(r)
    for path, sites in by_file.items():
        text = get(path)
        edits = []
        for r in sites:
            off = site_index(text, r.line, r.literal, path)
            edits.append((off, r))
        if len({o for o, _ in edits}) != len(edits):
            raise Refuse(f"{path}: two rows name the same token")
        orig = text
        for off, r in sorted(edits, key=lambda e: e[0], reverse=True):
            text = text[:off] + r.name + text[off + len(r.literal):]
        # tools/format.py hoists a trailing comment that a longer line pushes past the column
        # limit onto its own line above the statement: check would read that as a moved
        # comment. Refuse such a site here; move its comment by hand in its own commit first.
        hoist = _format_module().hoist_trailing_comments
        if hoist(orig) == orig and hoist(text) != text:
            moved = next(k + 1 for k, (a, b) in enumerate(zip(text.split("\n"),
                                                               hoist(text).split("\n"))) if a != b)
            raise Refuse(f"{path}:{moved}: after the substitution tools/format.py would hoist "
                         "this line's trailing comment (it passes the column limit); move the "
                         "comment above the statement in its own commit first")
        texts[path] = text
    # definitions, grouped per file in manifest order
    blocks: dict[str, list[str]] = {}
    enums: dict[tuple[str, str], list[Row]] = {}
    for r in defs.values():
        text = get(r.path)
        if r.kind == "define":
            have = defined_body(text, r.name)
            if have is not None:
                if have != " ".join(r.literal.split()):
                    raise Refuse(f"{r.path}: {r.name} is already #defined as {have!r}, "
                                 f"not {r.literal!r}")
                continue
            if re.search(r"\b" + re.escape(r.name) + r"\b", blank_comments(read(r.path) or "")):
                raise Refuse(f"{r.path}: {r.name} already names something in this file")
            blocks.setdefault(r.path, []).append(f"#define {r.name} {r.literal}")
        else:
            if re.search(r"\b" + re.escape(r.ident) + r"\b", blank_comments(read(r.path) or "")):
                raise Refuse(f"{r.path}: {r.ident} already names something in this file")
            if re.search(r"\benum\s+" + re.escape(r.tag) + r"\b", blank_comments(text)):
                raise Refuse(f"{r.path}: enum {r.tag} exists; extend it by hand (one enum per set)")
            enums.setdefault((r.path, r.tag), []).append(r)
    for (path, tag), members in enums.items():
        lines = [f"enum {tag} {{"] + [f"    {m.ident} = {m.literal}," for m in members] + ["};"]
        blocks.setdefault(path, []).extend(lines)
    inserted = {r.ident: r for r in defs.values() if r.kind == "enum"
                or f"#define {r.name} {r.literal}" in blocks.get(r.path, [])}

    def code_uses(text: str, names) -> list[int]:  # lines of the names outside directives
        clean = blank_comments(text)
        dirs = directive_lines(clean, comment_joined_lines(text))
        return [ln for ln, t, _ in tokens(clean) if t in names and ln not in dirs]

    for path, block in blocks.items():
        names = {r.ident for r in inserted.values() if r.path == path}
        texts[path] = insert_block(get(path), block, path.endswith(".h"),
                                   early=bool(code_uses(get(path), names)))
    # define before use: in its own file, an inserted name is used only below its definition
    for r in inserted.values():
        text = texts[r.path]
        lines = blank_comments(text).split("\n")
        pat = (r"\s*#\s*define\s+" + re.escape(r.ident) + r"\b" if r.kind == "define"
               else r"\s*" + re.escape(r.ident) + r"\s*=")
        at = next((k for k, l in enumerate(lines, 1) if re.match(pat, l)), None)
        uses = code_uses(text, {r.ident})
        if at is None or any(u < at for u in uses if u != at):
            raise Refuse(f"{r.path}:{min(uses) if uses else '?'}: {r.ident} would be used above its inserted "
                         "definition; move the use or the definition in its own commit first")
    changed = []
    for path, text in texts.items():
        if text != read(path):
            write(path, text)
            changed.append(path)
    return changed


# ---------------------------------------------------------------- trees

class Tree:
    """A source tree: the working tree (rev None) or a git revision extracted under tmp/
    (every tracked file but docs/). read() returns what cpp reads, the same way in both modes:
    the file under root(), its bytes decoded with no newline translation."""

    def __init__(self, rev: str | None):
        self.rev, self.dir = rev, None

    def read(self, path: str) -> str | None:
        p = self.root() / path
        return p.read_bytes().decode("utf-8", "surrogateescape") if p.is_file() else None

    def root(self) -> Path:
        if self.rev is None:
            return ROOT
        if self.dir is None:
            d = ROOT / "tmp" / f"const-wave-{uuid.uuid4().hex[:12]}"
            d.mkdir(parents=True)
            self.dir = d
            extract_blobs(self.rev, d)
        return self.dir

    def close(self):
        if self.dir is not None:
            d = self.dir.resolve()
            if d.parent != (ROOT / "tmp").resolve() or not d.name.startswith("const-wave-"):
                raise Refuse(f"refusing to remove {d}")
            shutil.rmtree(d)
            self.dir = None

    def files(self) -> list[str]:
        if self.rev is None:
            r = subprocess.run(["git", "ls-files", "-z", "-co", "--exclude-standard"],
                               cwd=ROOT, capture_output=True, text=True)
        else:
            r = subprocess.run(["git", "ls-tree", "-z", "-r", "--name-only", self.rev],
                               cwd=ROOT, capture_output=True, text=True)
        if r.returncode:
            raise Refuse(f"cannot list files of {self.rev or 'working tree'}")
        return nul_split(r.stdout)


def extract_blobs(rev: str, dest: Path):
    """Every tracked regular file of `rev` but docs/, as its raw blob bytes (git archive would
    apply autocrlf / eol attributes, so a snapshot could differ from the commit)."""
    r = subprocess.run(["git", "ls-tree", "-z", "-r", rev], cwd=ROOT, capture_output=True)
    if r.returncode:
        raise Refuse(f"git ls-tree {rev}: {r.stderr.decode(errors='replace')[:300]}")
    entries = []
    for rec in r.stdout.split(b"\0"):
        if not rec:
            continue
        meta, path = rec.split(b"\t", 1)
        mode, kind, sha = meta.split()
        name = path.decode("utf-8", "surrogateescape")
        if kind == b"blob" and mode in (b"100644", b"100755") and not name.startswith("docs/"):
            entries.append((sha, name))
    cat = subprocess.run(["git", "cat-file", "--batch"], cwd=ROOT, capture_output=True,
                         input=b"".join(sha + b"\n" for sha, _ in entries))
    if cat.returncode:
        raise Refuse(f"git cat-file {rev}: {cat.stderr.decode(errors='replace')[:300]}")
    out, pos = cat.stdout, 0
    for sha, name in entries:
        nl = out.index(b"\n", pos)
        head = out[pos:nl].split()
        if head[0] != sha or head[1] != b"blob":
            raise Refuse(f"git cat-file {rev}: unexpected {out[pos:nl][:80]!r}")
        size = int(head[2])
        target = dest / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(out[nl + 1:nl + 1 + size])
        pos = nl + 1 + size + 1


def nul_split(out: str) -> list[str]:
    """git -z output: paths with spaces or newlines stay whole."""
    return [p for p in out.split("\0") if p]


def changed_paths(base: Tree, new: Tree) -> list[str]:
    """Every path the wave changes, whole tree (docs/ included; explain() decides)."""
    if new.rev is None:
        r = subprocess.run(["git", "diff", "-z", "--name-only", "--no-renames", base.rev],
                           cwd=ROOT, capture_output=True, text=True)
        # untracked: new files in the source trees and at the top level (a new TU, header,
        # .s, linker script or flag list). Not the worktree's .venv / disc / toolchain
        # junctions, which WSL git lists as untracked; tracked edits anywhere are in `r`.
        extra = subprocess.run(["git", "ls-files", "-z", "-o", "--exclude-standard", "--",
                                "src", "include", "asm", "."], cwd=ROOT, capture_output=True,
                               text=True)
        if r.returncode or extra.returncode:
            raise Refuse(f"git diff {base.rev} failed")
        new = {p for p in nul_split(extra.stdout)
               if p.startswith(("src/", "include/", "asm/"))
               or ("/" not in p and not (ROOT / p).is_symlink() and not (ROOT / p).is_dir())}
        return sorted(set(nul_split(r.stdout)) | new)
    r = subprocess.run(["git", "diff", "-z", "--name-only", "--no-renames", base.rev, new.rev],
                       cwd=ROOT, capture_output=True, text=True)
    if r.returncode:
        raise Refuse(f"git diff {base.rev} {new.rev} failed")
    return sorted(set(nul_split(r.stdout)))


# ---------------------------------------------------------------- diff explanation

DEFINE_LINE = re.compile(r"#\s*define\s+([A-Za-z_]\w*)(?!\()\s+(.*\S)\s*$")
INCLUDE_LINE = re.compile(r"#\s*include\s*[<\"]([^>\"]+)[>\"]\s*$")


def include_ok(path: str, target: str, close: str, tracked: set[str], def_paths: set[str]) -> bool:
    """An added #include must name a tracked header of the new tree that holds one of the
    manifest's definitions, relative, no `..`: reached through -Iinclude, or (quoted) next to
    the including file. Anything else (an absolute or out-of-tree path, an untracked file, a
    header no row needs) would add an unrecorded build input or an unexplained change."""
    if (target.startswith("/") or "\\" in target or ":" in target or ".." in target.split("/")
            or not target.endswith(".h")):
        return False
    hits = ["include/" + target]
    if close == '"':
        hits.append(str(Path(path).parent.as_posix()) + "/" + target)
    return any(h in tracked and h in def_paths for h in hits)
ENUM_OPEN = re.compile(r"enum\s+([A-Za-z_]\w*)\s*\{\s*$")
ENUM_MEMBER = re.compile(r"([A-Za-z_]\w*)\s*=\s*(\S+?)\s*,?\s*$")
ENUM_CLOSE = re.compile(r"\}\s*;\s*$")


@dataclass
class Explained:
    used: dict = field(default_factory=dict)       # (path, line, literal) site key -> count
    cc1_files: set = field(default_factory=set)    # changed files whose edits need cc1 proof
    problems: list = field(default_factory=list)


def explain_file(path: str, old: str, new: str, rows: list[Row], loose: bool, ex: Explained,
                 tracked: frozenset = frozenset()):
    defs = definitions(rows)
    sites = [r for r in rows if r.kind == "site" and r.path == path]
    # cpp reads CR as a newline and splices backslash(-blanks)-newline before it sees comments;
    # a wave never needs to add or remove a splice or anything this reader could misread
    for spelling, what in UNREADABLE:
        if new.count(spelling) != old.count(spelling):
            ex.problems.append(f"{path}: the wave adds or removes a {what}; a constants wave "
                               "never needs one")
            return
    old, new = cpp_text(old), cpp_text(new)
    if len(SPLICE.findall(new)) != len(SPLICE.findall(old)):
        ex.problems.append(f"{path}: the wave adds or removes a line splice (backslash-newline);"
                           " a constants wave never needs one")
        return
    for which, text in (("base", old), ("new", new)):
        bad = misread_splices(text)
        if bad:
            ex.problems.append(f"{path}:{bad[0]}: ({which}) a backslash-newline in a comment, "
                               "a string or a code line; this checker cannot read the file as "
                               "cpp does")
            return
    ot, nt = units(old), units(new)
    o_asm = asm_token_spans(ot)
    # strict: a site row names the one token of its literal on its line (as apply refuses a
    # line holding it twice), so a line holding it twice explains no replacement
    seen_lit: dict = {}
    for ln, t, g in ot:
        if g in ("num", "chr"):
            seen_lit[(ln, t)] = seen_lit.get((ln, t), 0) + 1
    dups = {k for k, n in seen_lit.items() if n > 1}
    sm = difflib.SequenceMatcher(None, [t for _, t, _ in ot], [t for _, t, _ in nt], autojunk=False)
    inserted: list[int] = []
    for op, i1, i2, j1, j2 in sm.get_opcodes():
        if op == "equal":
            continue
        if op == "replace" and i2 - i1 == j2 - j1 and all(
                _site_replacement(path, ot[i], nt[j], i, o_asm, sites, defs, loose, ex, dups)
                for i, j in zip(range(i1, i2), range(j1, j2))):
            continue
        if op == "insert":
            inserted.extend(range(j1, j2))
            continue
        ex.problems.append(f"{path}:{ot[i1][0] if i1 < len(ot) else '?'}: change not explained "
                           f"by the manifest: {' '.join(t for _, t, _ in ot[i1:i2])!r} -> "
                           f"{' '.join(t for _, t, _ in nt[j1:j2])!r}")
    # inserted units: whole directives (#define / #include), or whole lines of an enum block
    shared = {nt[j][0] for op, _, _, j1, j2 in sm.get_opcodes() if op == "equal"
              for j in range(j1, j2)}
    nlines = blank_comments(new).split("\n")
    in_enum, last, opened, done = None, 0, set(), set()

    def adjacent(ln):  # nothing but blank lines since the enum block's previous line
        return all(not nlines[k - 1].strip() for k in range(last + 1, ln))

    for j in inserted:
        ln, text, grp = nt[j]
        if grp == "cmt":
            ex.problems.append(f"{path}:{ln}: inserted or moved comment: {text!r} (a constants "
                               "wave never touches comments)")
            continue
        if grp == "dir" and in_enum:
            ex.problems.append(f"{path}:{ln}: directive inside inserted enum {in_enum}: {text!r}")
            continue
        if grp == "dir":
            m = DEFINE_LINE.match(text)
            d = defs.get(m.group(1)) if m else None
            if (d and d.kind == "define" and d.path == path
                    and text == directive_key(f"#define {d.name} {d.literal}")):
                continue
            inc = INCLUDE_LINE.match(text)
            if inc and include_ok(path, inc.group(1), inc.group(0).rstrip()[-1], tracked,
                                  {d.path for d in defs.values()}):
                ex.cc1_files.add(path)
                continue
            ex.problems.append(f"{path}:{ln}: inserted directive is not a manifest definition: "
                               f"{text!r}")
            continue
        if ln in done:
            continue
        done.add(ln)
        s = " ".join(nlines[ln - 1].split())
        bad = ln in shared
        if bad:
            pass
        elif (not in_enum and ENUM_OPEN.match(s) and ENUM_OPEN.match(s).group(1) not in opened
              # at a declaration boundary: nothing (`extern`, a type) may bind to the block
              and (j == 0 or nt[j - 1][2] in ("dir", "cmt") or nt[j - 1][1] in (";", "}"))
              and any(d.tag == ENUM_OPEN.match(s).group(1) and d.path == path
                      for d in defs.values())):
            in_enum = ENUM_OPEN.match(s).group(1)  # one contiguous block per tag
            opened.add(in_enum)
            last = ln
            ex.cc1_files.add(path)
        elif in_enum and ENUM_MEMBER.match(s) and adjacent(ln):
            m = ENUM_MEMBER.match(s)
            d = defs.get(m.group(1))
            bad = not (d and d.kind == "enum" and d.tag == in_enum and d.literal == m.group(2)
                       and d.path == path)
            last = ln
        elif in_enum and ENUM_CLOSE.match(s) and adjacent(ln):
            in_enum = None
        else:
            bad = True
        if bad:
            ex.problems.append(f"{path}:{ln}: inserted line is not a manifest definition: {s!r}")
    if in_enum:
        ex.problems.append(f"{path}: inserted enum {in_enum} is not closed")


def _site_replacement(path, otok, ntok, i, o_asm, sites, defs, loose, ex, dups=frozenset()) -> bool:
    line, lit, grp = otok
    name = ntok[1]
    if lit == name:
        return True
    d = defs.get(name)
    if d is None or i in o_asm or grp != "num" and grp != "chr" or ntok[2] != "id":
        return False
    for r in sites:
        if r.literal == lit and r.name == name and (loose or (r.line == line
                                                              and (line, lit) not in dups)):
            key = (r.path, r.line, r.literal)
            ex.used[key] = ex.used.get(key, 0) + 1
            if needs_cc1(d) or (d.kind == "define" and d.literal != lit):
                ex.cc1_files.add(path)
            return True
    return False


def explain(rows, base: Tree, new: Tree, loose: bool) -> tuple[Explained, list[str]]:
    ex = Explained()
    tracked = frozenset(new.files())
    changed = []
    for path in changed_paths(base, new):
        changed.append(path)
        if path.startswith("docs/"):
            continue  # the manifest, the backlog row: not build inputs (prove: no TU reads one)
        if not path.endswith((".c", ".h")) or not path.startswith(("src/", "include/")):
            ex.problems.append(f"{path}: a constants wave changes only .c / .h under src/ and "
                               "include/ (and docs/)")
            continue
        old, cur = base.read(path), new.read(path)
        if old is None or cur is None:
            ex.problems.append(f"{path}: added or deleted (a constants wave edits files in place)")
            continue
        explain_file(path, old, cur, rows, loose, ex, tracked)
    # K4: each definition row is where the manifest says (not merely defined elsewhere)
    for d in definitions(rows).values():
        text = blank_comments(cpp_text(new.read(d.path) or ""))
        if d.kind == "define":
            ok = defined_body(text, d.name) == " ".join(d.literal.split())
        else:
            ok = re.search(r"\benum\s+" + re.escape(d.tag) + r"\s*\{[^}]*\b" + re.escape(d.ident)
                           + r"\s*=\s*" + re.escape(d.literal) + r"\s*[,}]", text) is not None
        if not ok:
            ex.problems.append(f"{d.path}: does not define {d.name} = {d.literal} as its "
                               f"manifest row (line {d.lineno}) says")
    if not loose:
        for r in rows:
            if r.kind == "site":
                n = ex.used.get((r.path, r.line, r.literal), 0)
                if n != 1:
                    ex.problems.append(f"{r.where}: site row {r.literal} -> {r.name} matched {n} "
                                       "replacement(s), want 1")
        return ex, changed
    # loose (a replayed wave: lines may have moved): replacements per (file, literal, name)
    # must number exactly the manifest's rows for that triple
    want, got = {}, {}
    for r in rows:
        if r.kind == "site":
            want[(r.path, r.literal, r.name)] = want.get((r.path, r.literal, r.name), 0) + 1
    for path in changed:
        if path.endswith((".c", ".h")) and path.startswith(("src/", "include/")):
            for a, b in _replacements(base.read(path) or "", new.read(path) or ""):
                got[(path, a, b)] = got.get((path, a, b), 0) + 1
    for k in sorted(set(want) | set(got)):
        if want.get(k, 0) != got.get(k, 0):
            ex.problems.append(f"{k[0]}: {got.get(k, 0)} replacement(s) {k[1]} -> {k[2]}, the "
                               f"manifest lists {want.get(k, 0)}")
    return ex, changed


def _replacements(old: str, new: str):
    ot, nt = units(cpp_text(old)), units(cpp_text(new))
    sm = difflib.SequenceMatcher(None, [t for _, t, _ in ot], [t for _, t, _ in nt], autojunk=False)
    for op, i1, i2, j1, j2 in sm.get_opcodes():
        if op == "replace" and i2 - i1 == j2 - j1:
            for i, j in zip(range(i1, i2), range(j1, j2)):
                if ot[i][1] != nt[j][1]:
                    yield ot[i][1], nt[j][1]


# ---------------------------------------------------------------- K3 proof (toolchain)

def toolchain(makefile: str, tu: str):
    _tools_on_path()
    import naming_keycheck as nk  # the project's fail-closed Makefile reader
    cpp, cc1 = nk.cpp_command(makefile), nk.cc1_command(makefile, tu)
    for cmd in (cpp, cc1):
        exe = str(ROOT / cmd[0]) if "/" in cmd[0] else shutil.which(cmd[0])
        if not exe or not Path(exe).is_file():
            raise Refuse(f"toolchain missing: {cmd[0]} (run in WSL with the venv active)")
        cmd[0] = exe
    return cpp, cc1


def run_cpp(tree: Tree, tu: str) -> str:
    cpp, _ = toolchain(tree.read("Makefile") or "", tu)
    r = subprocess.run([*cpp, tu], cwd=tree.root(), capture_output=True, text=True,
                       encoding="utf-8", errors="surrogateescape")
    if r.returncode:
        raise Refuse(f"cpp failed for {tu} in {tree.rev or 'working tree'}: {r.stderr[:400]}")
    return r.stdout


def run_cc1(tree: Tree, tu: str, source: str) -> str:
    _, cc1 = toolchain(tree.read("Makefile") or "", tu)
    r = subprocess.run(cc1, input=source, cwd=tree.root(), capture_output=True, text=True,
                       encoding="utf-8", errors="surrogateescape")
    if r.returncode:
        raise Refuse(f"cc1 failed for {tu} in {tree.rev or 'working tree'}: {r.stderr[:400]}")
    return r.stdout


MARKER = re.compile(r'^#\s*\d+\s+"([^"]*)"')


def cpp_files(out: str) -> set[str]:
    return {m.group(1).removeprefix("./") for m in map(MARKER.match, out.splitlines()) if m}


def cpp_tokens(out: str) -> list[str]:
    """The token stream of cpp output, line markers and layout dropped."""
    body = "\n".join(l for l in out.splitlines() if not MARKER.match(l))
    return [t for _, t, _ in tokens(body)]


def prove(base: Tree, new: Tree, changed: list[str], cc1_files: set[str], log=print,
          cpp=run_cpp, cc1=run_cc1) -> list[str]:
    """K3 per TU in scope. Returns the problems (empty = proved)."""
    problems = []
    changed_set = set(changed)
    # every TU, whatever changed: any file (a .c too) can be #included by another TU, and only
    # cpp's own line markers say what reaches what
    tus = sorted(p for p in set(new.files()) & set(base.files())
                 if p.startswith("src/") and p.endswith(".c"))
    base.root(), new.root()  # materialise once, before the workers share them
    with ThreadPoolExecutor(max_workers=8) as pool:
        outs = list(pool.map(lambda tu: (cpp(base, tu), cpp(new, tu)), tus))
    scoped = 0
    tracked = {p for p in new.files() if p.startswith(("src/", "include/"))}
    for tu, (b_out, n_out) in zip(tus, outs):
        # fail closed on inputs, by any spelling: every file the new cpp reads is one the base
        # already read, or a tracked source of the new tree (never an out-of-tree header)
        stray = cpp_files(n_out) - cpp_files(b_out) - tracked
        if stray:
            problems.append(f"{tu}: cpp now reads files outside the tracked tree: "
                            + ", ".join(sorted(stray)))
            continue
        reach = (cpp_files(b_out) | cpp_files(n_out) | {tu}) & changed_set
        docs = sorted(p for p in reach if p.startswith("docs/"))
        if docs:
            problems.append(f"{tu}: cpp reads a changed docs/ file: {', '.join(docs)}")
            continue
        same = cpp_tokens(b_out) == cpp_tokens(n_out)
        if not reach and same:
            continue
        scoped += 1
        if same:
            log(f"  {tu}: cpp identical")
            continue
        if not reach & cc1_files:
            problems.append(f"{tu}: cpp output changed, and no enum / non-literal body / added "
                            "#include reaches it (a #define row must leave cpp output identical)")
            continue
        if cc1(base, tu, b_out) == cc1(new, tu, n_out):
            log(f"  {tu}: cc1 .s identical")
        else:
            problems.append(f"{tu}: cc1 .s output differs from base")
    log(f"K3: {scoped} TU(s) in scope")
    return problems


def check(rows, base_rev: str, new_rev: str | None, loose: bool, log=print,
          cpp=run_cpp, cc1=run_cc1) -> list[str]:
    base, new = Tree(base_rev), Tree(new_rev)
    try:
        ex, changed = explain(rows, base, new, loose)
        problems = list(ex.problems)
        if not problems:
            problems += prove(base, new, changed, ex.cc1_files, log, cpp, cc1)
        return problems
    finally:
        try:
            base.close()
        finally:
            new.close()


# ---------------------------------------------------------------- CLI

def load_manifest(spec: str, rev: str | None) -> list[Row]:
    if rev and not Path(spec).is_file():
        r = subprocess.run(["git", "show", f"{rev}:{spec}"], cwd=ROOT, capture_output=True)
        if r.returncode:
            raise Refuse(f"no manifest {spec} at {rev}")
        return parse_manifest(r.stdout.decode("utf-8"))
    p = Path(spec)
    if not p.is_absolute():
        p = ROOT / p
    if not p.is_file():
        raise Refuse(f"no manifest {spec}")
    return parse_manifest(p.read_text(encoding="utf-8"))


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    a = sub.add_parser("apply", help="apply a manifest to the working tree")
    a.add_argument("manifest")
    c = sub.add_parser("check", help="prove an applied wave (explained diff + K3)")
    c.add_argument("manifest", help="manifest path (or a path in --new's tree)")
    c.add_argument("--base", default="HEAD")
    c.add_argument("--new", default=None, help="revision to check (default: the working tree)")
    c.add_argument("--loose-lines", action="store_true",
                   help="match sites by file, literal and name, not line (a replayed wave)")
    args = ap.parse_args(argv)
    try:
        if args.cmd == "apply":
            rows = load_manifest(args.manifest, None)
            changed = apply(rows, Tree(None).read,
                            lambda p, t: (ROOT / p).write_text(t, encoding="utf-8", newline="\n",
                                                               errors="surrogateescape"))
            print("\n".join(f"changed {p}" for p in changed) or "nothing to change")
            print(f"applied {sum(r.kind == 'site' for r in rows)} site(s); next: "
                  "python tools/format.py <files>, then const_wave.py check")
            return 0
        rows = load_manifest(args.manifest, args.new)
        problems = check(rows, args.base, args.new, args.loose_lines)
    except (Refuse, ValueError, OSError) as e:  # no verdict, never read as FAIL
        print(f"ERROR: {e}", file=sys.stderr)
        return 2
    except Exception:  # a crash is no verdict either (exit 1 means FAIL)
        import traceback
        traceback.print_exc()
        return 2
    if problems:
        for p in problems:
            print(f"  {p}")
        print(f"FAIL: {len(problems)} problem(s) in the constants wave")
        return 1
    print("OK: every change is a manifest constant and K3 holds")
    return 0


if __name__ == "__main__":
    sys.exit(main())
