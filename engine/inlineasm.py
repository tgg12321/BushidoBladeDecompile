"""Cheat-asm stripping for the cheat-invisible sandbox + cheat-asm campaign.

Cheat-asm (workaround) `__asm__` blocks, register-asm pins, and plain
`register` storage hints are injection/allocation cheats: the sandbox strips
them so the score reflects pure-C codegen — injecting asm or nudging the
allocator can't move the score. Canonical inline asm
(GTE/cop2/.word-cop2/BIOS/HW) is authentic and is KEPT. A mixed block (any
canonical instruction) is kept whole (canonical GTE sequences include their
feeder loads).

Category classification reuses tools/classify_inline_asm (single source of truth
for the canonical-vs-cheat line). Stripping a function's cheat asm doesn't change
OTHER functions' .text bytes (only file-scope label numbers shift), so a
file-wide strip + masked scoring triages every cheat-asm function in one build.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import classify_inline_asm as cia  # noqa: E402

_PIN_QUALIFIER = re.compile(r'\basm\s*\(\s*"[^"]+"\s*\)')
_REGISTER_KEYWORD = re.compile(r"\bregister\b")


def _code_without_comments_and_strings(text: str) -> str:
    """Return text with comments and string/char literals blanked, preserving
    indexes and newlines. This lets source-token regexes avoid prose matches."""
    chars = list(text)
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if c in "\"'":
            quote = c
            chars[i] = " "
            i += 1
            while i < n:
                if text[i] == "\\":
                    chars[i] = " "
                    if i + 1 < n:
                        chars[i + 1] = " "
                    i += 2
                    continue
                chars[i] = "\n" if text[i] == "\n" else " "
                if text[i] == quote:
                    i += 1
                    break
                i += 1
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            chars[i] = chars[i + 1] = " "
            i += 2
            while i < n and text[i] != "\n":
                chars[i] = " "
                i += 1
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            chars[i] = chars[i + 1] = " "
            i += 2
            while i < n:
                if text[i] == "*" and i + 1 < n and text[i + 1] == "/":
                    chars[i] = chars[i + 1] = " "
                    i += 2
                    break
                chars[i] = "\n" if text[i] == "\n" else " "
                i += 1
            continue
        i += 1
    return "".join(chars)


def _preprocessor_line(text: str, idx: int) -> bool:
    line_start = text.rfind("\n", 0, idx) + 1
    return text[line_start:idx].lstrip().startswith("#")


def register_hint_spans(text: str) -> list[tuple[int, int]]:
    """Char spans for C `register` storage-class hints in real code tokens.
    Macro definitions are skipped so strip mode does not rewrite definitions."""
    masked = _code_without_comments_and_strings(text)
    return [m.span() for m in _REGISTER_KEYWORD.finditer(masked)
            if not _preprocessor_line(masked, m.start())]


def _match_paren(text: str, open_idx: int) -> int:
    """Index just past the ')' matching the '(' at open_idx (string-aware)."""
    depth, i, in_str = 0, open_idx, False
    while i < len(text):
        c = text[i]
        if in_str:
            if c == "\\":
                i += 2
                continue
            if c == '"':
                in_str = False
        elif c == '"':
            in_str = True
        elif c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return -1


def _match_brace(text: str, open_idx: int) -> int:
    """Index just past the '}' matching the '{' at open_idx. Skips braces inside
    string/char literals and // or /* */ comments. -1 if unbalanced."""
    depth, i, n = 0, open_idx, len(text)
    while i < n:
        c = text[i]
        if c in '"\'':
            q, i = c, i + 1
            while i < n:
                if text[i] == "\\":
                    i += 2
                    continue
                if text[i] == q:
                    break
                i += 1
            i += 1
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            nl = text.find("\n", i)
            i = n if nl == -1 else nl
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            ce = text.find("*/", i + 2)
            i = n if ce == -1 else ce + 2
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return -1


def _block_category(asm_body: str) -> str:
    """Category of an __asm__(...) body, mirroring classify_inline_asm.scan_file.
    Returns "canonical_body" (whole-function asm, never strip), "canonical"
    (authentic GTE/BIOS/HW, keep in source), or "cheat" (forbidden, strip for
    sandbox / refuse at COMPLETED gate)."""
    first_colon = asm_body.find(":")
    tmpl_body = asm_body if first_colon == -1 else asm_body[:first_colon]
    tmpl_strs = cia.extract_strings(tmpl_body)
    if any(cia.GLABEL_HINT_RE.search(s) for s in tmpl_strs):
        return "canonical_body"  # whole-function canonical body — never strip
    instrs = []
    for t in tmpl_strs:
        instrs.extend(cia.split_template(t))
    if not instrs:
        return "cheat" if cia.EMPTY_ASM_IS_CHEAT else "canonical"
    return "canonical" if any(cia.classify_template(x) == "canonical" for x in instrs) else "cheat"


def _strip_spans(text: str) -> list[tuple[int, int]]:
    """Char spans to delete: cheat-asm __asm__ statements + the asm("$N")
    qualifier of register pins (the variable declaration itself is kept)."""
    spans = []
    for m in cia.ASM_KEYWORD_RE.finditer(text):
        line_start = text.rfind("\n", 0, m.start()) + 1
        if text[line_start:m.start()].lstrip().startswith("#"):
            continue  # macro definition — leave (stripping would break uses)
        paren_open = m.end() - 1
        close = _match_paren(text, paren_open)
        if close < 0:
            continue
        if _block_category(text[paren_open + 1:close - 1]) != "cheat":
            continue
        end = close
        while end < len(text) and text[end] in " \t":
            end += 1
        if end < len(text) and text[end] == ";":
            end += 1
        spans.append((m.start(), end))
    for pm in cia.REGISTER_PIN_RE.finditer(text):
        am = _PIN_QUALIFIER.search(pm.group(0))
        if am:
            spans.append((pm.start() + am.start(), pm.start() + am.end()))
    return spans


def strip_cheat_asm_file(text: str) -> tuple[str, int]:
    """Return (modified text, count) with all cheat-asm blocks, register pins,
    plain `register` hints, AND every volatile-coercion cheat (alias renames,
    `*(volatile T *)&D_x` casts, scalar `extern volatile T D_x;` decls — see
    engine.volatile_cheats) stripped. Count is the sum of these cheat kinds.

    Both removals serve the same purpose: the sandbox sees the function the
    way `queue done` requires for COMPLETED-C (pure C, no codegen-coercion
    knobs), so the score reflects the honest pure-C distance.
    """
    spans = sorted(_strip_spans(text) + register_hint_spans(text), reverse=True)
    n_cheat_asm = len(spans)
    for s, e in spans:
        text = text[:s] + text[e:]
    # Second pass: strip volatile-coercion cheats (qualifier removal only).
    from . import volatile_cheats  # local import to avoid cycle
    text, n_vol = volatile_cheats.strip_volatile_cheats_file(text)
    return text, n_cheat_asm + n_vol


def write_stripped(stem: str, out_path: str) -> int:
    """Write src/<stem>.c with cheat-asm, register hints, and volatile coercion
    cheats stripped to out_path. Returns the count of stripped constructs."""
    text = Path(f"src/{stem}.c").read_text(encoding="utf-8")
    stripped, n = strip_cheat_asm_file(text)
    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    Path(out_path).write_text(stripped)
    return n


def _func_body_span(text: str, func: str) -> tuple[int, int] | None:
    """Locate `func`'s DEFINITION body (not its declaration). Splat-generated
    src/*.c put each definition at column 0; calls and nested references are
    indented. We must SKIP extern declarations (`extern T func(args);`) which
    match the same column-0 pattern — these are recognised by the FIRST non-
    whitespace after the closing `)` being `;` rather than `{`. Returns None
    rather than guess, so callers degrade to no-signal.

    Earlier versions used `text.find('{', m.end())` which silently picked up
    the NEXT function's opening brace when `func` had an extern declaration
    at the top of the file, attributing far-away unrelated cheats to it.
    """
    pattern = re.compile(r"(?m)^[A-Za-z_][\w \t\*]*\b" + re.escape(func) + r"\s*\(")
    n = len(text)
    for m in pattern.finditer(text):
        # Walk to the matching `)` of the opening `(`.
        i, depth, in_str = m.end(), 1, False
        while i < n and depth > 0:
            c = text[i]
            if in_str:
                if c == "\\":
                    i += 2
                    continue
                if c == "\"":
                    in_str = False
            elif c == "\"":
                in_str = True
            elif c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
            i += 1
        if depth != 0:
            continue
        # Skip whitespace after `)`; expect `{` (definition) or `;` (decl).
        j = i
        while j < n and text[j] in " \t\n\r":
            j += 1
        if j >= n or text[j] != "{":
            continue  # declaration or other; skip this candidate
        end = _match_brace(text, j)
        if end != -1:
            return (m.start(), end)
    return None


def func_cheat_asm_count(text: str, func: str) -> int:
    """Number of cheat constructs inside func's body: cheat-asm strip-spans
    (asm blocks + register pins), plain `register` hints, and volatile-coercion
    cheats (alias renames + `*(volatile T *)&D_x` casts + scalar
    `extern volatile T D_x;` decls — see engine.volatile_cheats). -1 if the
    body can't be located.

    `queue done` (engine/queue.py) refuses to mark COMPLETED-C if this count > 0
    for a non-canonical-asm function, so adding volatile cheats here is what
    makes them a hard-stop in the same way `register T x asm("$N")` pins
    already are.
    """
    span = _func_body_span(text, func)
    if span is None:
        return -1
    lo, hi = span
    strip_spans = _strip_spans(text) + register_hint_spans(text)
    cheat_asm = sum(1 for s, _e in strip_spans if lo <= s < hi)
    from . import volatile_cheats  # local import to avoid cycle
    vol = volatile_cheats.func_volatile_cheat_count(text, func)
    return cheat_asm + (vol if vol > 0 else 0)


_FILE_TEXT_CACHE: dict[str, tuple[float, str]] = {}


def _read_src_cached(stem: str) -> str | None:
    """Read src/<stem>.c and cache by (path, mtime). Invalidates on edit."""
    p = Path(f"src/{stem}.c")
    try:
        mt = p.stat().st_mtime
    except OSError:
        return None
    cached = _FILE_TEXT_CACHE.get(stem)
    if cached and cached[0] == mt:
        return cached[1]
    try:
        text = p.read_text(encoding="utf-8")
    except OSError:
        return None
    _FILE_TEXT_CACHE[stem] = (mt, text)
    return text


def file_func_cheat_asm_count(stem: str, func: str) -> int:
    """func_cheat_asm_count against src/<stem>.c on disk. -1 on any read/locate
    miss. Caches the file text by mtime — queue regen calls this once per
    function, and re-reading + re-parsing the whole file every time is the
    dominant cost for files with many functions (text1b.c, code6cac.c, etc.)."""
    text = _read_src_cached(stem)
    if text is None:
        return -1
    return func_cheat_asm_count(text, func)
