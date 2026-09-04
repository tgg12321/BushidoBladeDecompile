"""Per-function DATA MODEL: declared shape vs evidence for every global the
target asm touches. Surfaced by the dossier and the grind brief.

Why (2026-09-03 post-mortem, func_80033550): 13 of 17 sessions modeled
register allocation for a tail that named_syms.txt had described since
2026-05-17 as "12-byte stride per leaf, 6 entries" -- three splat per-word
scalars that were one record copy. The census row, the sibling function that
walks the table with `addiu 0xC`, and the scalar header declaration were all
on disk; nothing routed them to the session. This module routes them.

Signals (mechanical, evidence-cited):
  SPLIT-AGGREGATE  the function references a `+N from BASE` per-word symbol
                   (named_syms.txt census) -- the aggregate-merge family is
                   the sanctioned spelling
  INDEXED-ACCESS   the target asm indexes a symbol with a computed register
                   (`lui $at,%hi(S)` / `addu $at,$at,$rN` / `%lo(S)($at)`)
                   while the header declares it scalar -- it is an array
                   (or a record base)
  CENSUS-VS-DECL   the census comment names a stride/table/array/record shape
                   but the header declares a scalar

Read-only; every helper degrades to empty on failure rather than raising.
Paths are repo-relative (call with cwd = repo root, like engine/dossier.py).
ASCII only: tools/grinder/grindlib.py imports this under Windows python 3.9,
whose tokenizer mis-handles multibyte characters on long lines.
"""
from __future__ import annotations

import re
from pathlib import Path

SYMBOL_FILES = ("undefined_syms_auto.txt", "symbol_addrs.txt", "named_syms.txt")
_SHAPE_WORDS = re.compile(r"\b(stride|entries|entry|table|array|struct|record|per-\w+|\d+-byte)\b", re.I)
_PLUS_RE = re.compile(r"/\*\s*\+(\d+) from (\w+) \((0x[0-9A-Fa-f]+)\)")
_ROW_RE = re.compile(r"^\s*(\w[\w$]*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;(.*)$")
_REF_RE = re.compile(r"%(?:hi|lo)\((\w+)")
_LO_AT_RE = re.compile(r"%lo\((\w+)\)\(\$at\)")
_ADDU_AT_RE = re.compile(r"\baddu\s+\$at,\s*\$at,\s*\$\w+")
_SCALAR_TYPES = {"u8", "s8", "u16", "s16", "u32", "s32", "int", "char", "short",
                 "long", "unsigned", "signed"}
MAX_ROWS = 24

_cache: dict = {}


def reset_cache() -> None:
    _cache.clear()


def _read(p) -> str:
    try:
        return Path(p).read_text(encoding="utf-8", errors="replace")
    except OSError:
        return ""


def _comment(rest: str) -> str:
    m = re.search(r"/\*\s*(.*?)\s*\*/", rest) or re.search(r"//\s*(.*)$", rest)
    return (m.group(1).strip() if m else "")[:110]


def symbol_tables() -> tuple[dict, dict, dict]:
    """(addr_of{name:addr}, census{addr:(name,comment)}, sub{addr:(base,off,base_addr)}).
    named_syms.txt is read last so its schema comments win the census slot."""
    if "tables" in _cache:
        return _cache["tables"]
    addr_of, census, sub = {}, {}, {}
    for fn in SYMBOL_FILES:
        for ln in _read(fn).split("\n"):
            m = _ROW_RE.match(ln)
            if not m:
                continue
            name, addr, rest = m.group(1), int(m.group(2), 16), m.group(3)
            addr_of[name] = addr
            pm = _PLUS_RE.search(rest)
            if pm:
                sub[addr] = (pm.group(2), int(pm.group(1)), int(pm.group(3), 16))
                continue
            c = _comment(rest)
            if not name.startswith("D_") and (addr not in census or c):
                census[addr] = (name, c)
    _cache["tables"] = (addr_of, census, sub)
    return _cache["tables"]


def asm_refs(func: str) -> tuple[list[str], set[str]]:
    """(data symbols the target asm references in first-seen order, the subset
    accessed through a computed index). Function symbols are excluded."""
    lines = _read(Path("asm/funcs") / f"{func}.s").split("\n")
    syms, indexed = [], set()
    for i, ln in enumerate(lines):
        for s in _REF_RE.findall(ln):
            if s not in syms and not (Path("asm/funcs") / f"{s}.s").is_file():
                syms.append(s)
        m = _LO_AT_RE.search(ln)
        if m:
            # the addu must sit BETWEEN this symbol's lui and its %lo use, so a
            # neighbouring symbol's index computation cannot leak in
            window = lines[max(0, i - 3):i]
            hi = [k for k, p in enumerate(window) if f"%hi({m.group(1)})" in p]
            if hi and any(_ADDU_AT_RE.search(p) for p in window[hi[-1] + 1:]):
                indexed.add(m.group(1))
    return syms, indexed


def _src_texts() -> dict[str, str]:
    if "csrc" not in _cache:
        _cache["csrc"] = {p.stem: _read(p) for p in sorted(Path("src").glob("*.c"))}
    return _cache["csrc"]


def is_include_asm(func: str) -> bool:
    needle = f'INCLUDE_ASM("asm/funcs", {func});'
    return any(needle in t for t in _src_texts().values())


def xref_index() -> dict[str, list[str]]:
    """{symbol: [funcs whose asm references it]} over asm/funcs/*.s (one scan)."""
    if "xref" in _cache:
        return _cache["xref"]
    idx: dict[str, set] = {}
    for p in Path("asm/funcs").glob("*.s"):
        for s in set(_REF_RE.findall(_read(p))):
            idx.setdefault(s, set()).add(p.stem)
    _cache["xref"] = {k: sorted(v) for k, v in idx.items()}
    return _cache["xref"]


def c_refs(sym: str) -> list[str]:
    """src/*.c stems whose text names the symbol."""
    pat = re.compile(r"\b" + re.escape(sym) + r"\b")
    return sorted(k for k, t in _src_texts().items() if pat.search(t))


def header_decl(sym: str) -> tuple[str, str]:
    """(extern declaration line, header file) or ('', '')."""
    if "hdrs" not in _cache:
        _cache["hdrs"] = {p.name: _read(p).split("\n") for p in sorted(Path("include").glob("*.h"))}
    pat = re.compile(r"^\s*extern\b.*\b" + re.escape(sym) + r"\b")
    for name, lines in _cache["hdrs"].items():
        for ln in lines:
            if pat.match(ln):
                return re.sub(r"\s*/\*.*?\*/\s*", " ", ln).strip()[:90], name
    return "", ""


def is_scalar_decl(decl: str) -> bool:
    """True when the extern declares a plain scalar (no array, no aggregate type)."""
    if not decl or "[" in decl:
        return False
    m = re.match(r"extern\s+(?:const\s+|volatile\s+)*([A-Za-z_]\w*)", decl)
    return bool(m) and m.group(1) in _SCALAR_TYPES


def data_model(func: str) -> tuple[list[str], list[str]]:
    """(rows, flags). rows: one line per referenced global, address order.
    flags: the mechanical signals listed in the module docstring."""
    try:
        syms, indexed = asm_refs(func)
        if not syms:
            return [], []
        addr_of, census, sub = symbol_tables()
        xref = xref_index()
        rows, flags = [], []
        split: dict[str, list[str]] = {}
        asm_sibs: dict[str, list[str]] = {}
        for s in sorted(syms, key=lambda n: addr_of.get(n, 1 << 40)):
            a = addr_of.get(s)
            decl, hdr = header_decl(s)
            live = [f for f in xref.get(s, []) if f != func and is_include_asm(f)]
            csrc = c_refs(s)
            xr = ("asm:" + (",".join(live[:3]) + (f"+{len(live) - 3}" if len(live) > 3 else "")
                            if live else "-")
                  + " C:" + (",".join(csrc[:3]) if csrc else "-"))
            row = f"  {s}" + (f" @{a:08X}" if a is not None else " @(no address row)")
            if a in sub:
                base, off, ba = sub[a]
                row += f"  SUB-SYMBOL +{off} of {base}({ba:08X})"
                split.setdefault(base, []).append(s)
                asm_sibs.setdefault(base, []).extend(f for f in live if f not in asm_sibs.get(base, []))
            elif a in census:
                nm, c = census[a]
                row += f"  census {nm}" + (f' "{c}"' if c else "")
            row += (f"  decl `{decl}` ({hdr})" if decl else "  decl: NONE in include/*.h") + f"  xref {xr}"
            rows.append(row)
            if s in indexed and is_scalar_decl(decl):
                flags.append(f"!! INDEXED-ACCESS: the target indexes {s} with a computed register "
                             f"(lui/addu $at) but the header declares `{decl}` -- it is an ARRAY "
                             f"(or a record base); declare it as one (size from the loop bound / "
                             f"census). A per-use `*(&{s} + i)` pun is a banned construct.")
            if a in census and is_scalar_decl(decl) and _SHAPE_WORDS.search(census[a][1] or ""):
                flags.append(f"!! CENSUS-VS-DECL: {s} census says \"{census[a][1]}\" but the header "
                             f"declares `{decl}` -- settle the object model BEFORE any codegen lever.")
        for base, pieces in split.items():
            ba = addr_of.get(base)
            c = census.get(ba, ("", ""))[1] if ba is not None else ""
            bdecl = header_decl(base)[0] or (header_decl(f"D_{ba:08X}")[0] if ba is not None else "")
            sibs = asm_sibs.get(base) or []
            flags.append(f"!! SPLIT-AGGREGATE: {', '.join(pieces)} are per-word splat pieces of {base}"
                         + (f" @{ba:08X}" if ba is not None else "")
                         + (f' (census: "{c}")' if c else "")
                         + (f"; base declared `{bdecl}`" if bdecl else "")
                         + ". The aggregate-merge family (.claude/rules/no-new-park-categories.md, "
                         "'Per-word splat symbol -> aggregate merge') is the sanctioned spelling: merge "
                         "at the DECLARATION in include/*.h via integration handoff, prongs (a)-(e); "
                         "a per-use cast / pointer pun is an auto-FAIL."
                         + (f" Sub-symbol rows also serve still-INCLUDE_ASM {', '.join(sibs[:3])}: "
                            "keep those rows with the `alias of <base>+N; retire with <sibling>` "
                            "suffix (prong (c) amendment 2026-09-03)." if sibs else ""))
        if len(rows) > MAX_ROWS:
            keep = [r for r in rows if "SUB-SYMBOL" in r or "census" in r][:MAX_ROWS]
            rest = len(rows) - len(keep)
            rows = keep + [f"  (+{rest} more globals with no census note -- see asm/funcs/{func}.s)"]
        return rows, flags
    except Exception:
        return [], []


def render(func: str) -> str:
    rows, flags = data_model(func)
    if not rows:
        return ""
    out = [f"data model ({len(rows)} globals; declared shape vs evidence):"] + rows
    if flags:
        out.append("data-model signals:")
        out += [f"  {f}" for f in flags]
    return "\n".join(out)


if __name__ == "__main__":
    import sys
    print(render(sys.argv[1]))
