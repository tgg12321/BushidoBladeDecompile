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

import functools
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import classify_inline_asm as cia  # noqa: E402

_PIN_QUALIFIER = re.compile(r'\basm\s*\(\s*"[^"]+"\s*\)')
_REGISTER_KEYWORD = re.compile(r"\bregister\b")

# --- whole-body asm constructs -------------------------------------------
# A function whose bytes come VERBATIM from asm/funcs/<name>.s instead of being
# compiled from C. Two spellings reach the build:
#
#   INCLUDE_ASM("asm/funcs", name);              the include_asm.h macro
#   __asm__(... ".include \"asm/funcs/name.s\"")  the same thing, hand-expanded
#   __asm__(... "glabel name" ...)                a hand-written whole body
#
# None is compiled C, so all three are cheat-asm ATTRIBUTED to `name` even
# though no C body exists to attribute them to positionally.
#
# Attribution is deliberately spelling-INDEPENDENT. An earlier draft covered
# only the two INCLUDE_ASM forms and argued the `glabel` spelling was safe
# because every glabel body in the tree today happens to be canonical-authorized
# (and so drops earlier via canon_funcs). Layer-2 review rejected that: it is a
# census of the current tree, not an invariant, and
# .claude/rules/canonical-asm-authorization-recipe.md actively instructs authors
# to write that form — so a future non-canonical glabel body would reproduce the
# ang_hosei defect one spelling over. Whether an attributed function may still be
# COMPLETED is decided by canonical-asm membership, never by which spelling was
# used.
#
# ATTRIBUTION IS NOT STRIPPING. Only the INCLUDE_ASM forms are stripped for
# scoring; `glabel` bodies keep _block_category's "canonical_body" never-strip
# treatment, unchanged.
#
# The MACRO spelling is the one that used to escape entirely: this module runs
# on UNEXPANDED source text, where `INCLUDE_ASM(` is simply not `__asm__(` and
# so never matches cia.ASM_KEYWORD_RE. It only becomes `__asm__` after cpp,
# which the stripper never runs. The construct was therefore never even a
# CANDIDATE for stripping — it was not classified and kept, it was unseen. That
# blind spot let a function with zero lines of decompiled C report an honest
# pure-C sandbox distance of 0 (measured on ang_hosei: recorded distance 50,
# post-conversion `sandbox --disable all` score 0), and made
# func_cheat_asm_count return -1, which queue.generate/mark_done read as clean.
_INCLUDE_ASM_MACRO_RE = re.compile(
    r'(?m)^[ \t]*INCLUDE_ASM\s*\(\s*"[^"]*"\s*,\s*([A-Za-z_]\w*)\s*\)\s*;?[ \t]*')
_INCLUDE_DIRECTIVE_RE = re.compile(r'\.include\s+\\?"[^"\\]*?/([A-Za-z_]\w*)\.s\\?"')
_GLABEL_NAME_RE = re.compile(r'\bglabel\s+([A-Za-z_]\w*)')


def include_asm_spans(text: str) -> list[tuple[str, int, int]]:
    """(func, start, end) for every `INCLUDE_ASM(FOLDER, func)` invocation.

    Anchored at line start, so a `#define INCLUDE_ASM(...)` in a header and a
    commented-out invocation do not match.
    """
    return [(m.group(1), m.start(), m.end())
            for m in _INCLUDE_ASM_MACRO_RE.finditer(text)]


# Directives known NOT to emit bytes. This is a DENY-list on purpose: anything
# unrecognised counts as emitting, so an unfamiliar directive fails safe toward
# "this block is a body" (retained/attributed) rather than "this is a bare
# marker" (droppable as not-a-function). `.align` is listed because emitting
# only alignment padding does not make a block a body.
#
# The inverse allowlist was the first draft and it misfiled real code: ings.c
# ships func_800164F8 as a raw word stream (0x2402270F = `addiu v0,0x270F`,
# 0x03E00008 = `jr ra`) and func_800164AC as a 19-entry `.word` pointer table.
# Treating every `.`-prefixed line as a non-instruction called both bare symbol
# markers. Census of every directive head across all `__asm__` blocks in src/:
# .aent .global .globl .include .section .set .text .type .word
_NON_EMITTING_DIRECTIVES = (".set", ".section", ".text", ".data", ".rodata",
                            ".bss", ".globl", ".global", ".type", ".size",
                            ".ent", ".end", ".aent", ".align", ".local",
                            ".weak", ".extern", ".file", ".ident", ".internal")
_LABEL_MACROS = ("glabel", "endlabel", "alabel", "jlabel", "dlabel",
                 "enddlabel", "ehlabel", "nonmatching")


def _asm_block_has_instructions(body: str) -> bool:
    """True when an `__asm__` block EMITS BYTES — a real instruction or a
    byte-emitting directive — as opposed to only layout/symbol directives, label
    definitions and glabel/endlabel macros.

    A whole-BODY construct needs an actual body. Some blocks exist purely to
    place a SYMBOL — e.g. system.c emits `glabel D_80081F1C` with nothing in it,
    so that `&D_80081F1C` resolves to the address just before cdrom_IrqHandler.
    Attributing those would put a bare label marker in the queue as if it were an
    undecompiled function.
    """
    for s in cia.extract_strings(body):
        for line in s.replace("\\n", "\n").splitlines():
            t = line.strip()
            if not t or t.endswith(":"):
                continue
            head = t.split()[0]
            if head in _LABEL_MACROS:
                continue
            if head.startswith(".") and head in _NON_EMITTING_DIRECTIVES:
                continue
            return True
    return False


def symbol_marker_funcs(text: str) -> set[str]:
    """Names declared by an `__asm__` block that places a SYMBOL and nothing
    else — a `glabel <name>` with no instructions in the block.

    These are address markers, not functions (system.c's `glabel D_80081F1C`
    exists so `&D_80081F1C` resolves to the address before cdrom_IrqHandler).
    They land in the object's function symbol table, so the queue needs to know
    they are not decomp work.
    """
    names: set[str] = set()
    for m in cia.ASM_KEYWORD_RE.finditer(text):
        line_start = text.rfind("\n", 0, m.start()) + 1
        if text[line_start:m.start()].lstrip().startswith("#"):
            continue
        paren_open = m.end() - 1
        close = _match_paren(text, paren_open)
        if close < 0:
            continue
        body = text[paren_open + 1:close - 1]
        if _INCLUDE_DIRECTIVE_RE.search(body):
            continue  # pulls in an asm/funcs body — a body, never a marker
        if not _asm_block_has_instructions(body):
            names.update(_GLABEL_NAME_RE.findall(body))
    return names


def whole_body_asm_funcs(text: str) -> set[str]:
    """Names of functions this file supplies wholly from asm rather than C:
    INCLUDE_ASM macro invocations, their hand-expanded `.include` equivalent, and
    `glabel <name>` whole-body `__asm__` blocks.

    Used to attribute cheat-asm to a function that has NO C body, so the
    completion gate and the queue can tell "undecompiled, asm-supplied" apart
    from "unknown". Whether such a function may still be COMPLETED is decided by
    canonical-asm membership (inline_asm_canonical.txt), not here.
    """
    names = {f for f, _s, _e in include_asm_spans(text)}
    for m in cia.ASM_KEYWORD_RE.finditer(text):
        line_start = text.rfind("\n", 0, m.start()) + 1
        if text[line_start:m.start()].lstrip().startswith("#"):
            continue
        paren_open = m.end() - 1
        close = _match_paren(text, paren_open)
        if close < 0:
            continue
        body = text[paren_open + 1:close - 1]
        names.update(_INCLUDE_DIRECTIVE_RE.findall(body))
        if _asm_block_has_instructions(body):
            names.update(_GLABEL_NAME_RE.findall(body))
    return names


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
    Macro definitions are skipped so strip mode does not rewrite definitions.

    Memoized by text (see _strip_spans); returns a fresh list each call."""
    return list(_register_hint_spans_cached(text))


@functools.lru_cache(maxsize=8)
def _register_hint_spans_cached(text: str) -> tuple[tuple[int, int], ...]:
    masked = _code_without_comments_and_strings(text)
    return tuple(m.span() for m in _REGISTER_KEYWORD.finditer(masked)
                 if not _preprocessor_line(masked, m.start()))


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
    """Char spans to delete: cheat-asm __asm__ statements, INCLUDE_ASM whole-body
    invocations, + the asm("$N") qualifier of register pins (the variable
    declaration itself is kept).

    INCLUDE_ASM is stripped because it is not compiled C: leaving it in would let
    the sandbox assemble the target bytes straight from asm/funcs/<name>.s and
    report an honest pure-C distance of 0 for a function with no C at all.
    Whole-body `glabel` __asm__ blocks are deliberately NOT stripped here —
    _block_category calls them "canonical_body" and that behaviour is unchanged.

    Memoized by text: this is a whole-file scan and `func_cheat_asm_count`
    calls it once per FUNCTION, so a 400-function TU used to re-scan itself 400
    times. Returns a fresh list each call so callers can mutate the result
    without corrupting the cache.
    """
    return list(_strip_spans_cached(text))


@functools.lru_cache(maxsize=8)
def _strip_spans_cached(text: str) -> tuple[tuple[int, int], ...]:
    spans = [(s, e) for _f, s, e in include_asm_spans(text)]
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
    return tuple(spans)


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


def write_stripped(stem: str, out_path: str, source_text: str | None = None) -> int:
    """Write src/<stem>.c with cheat-asm, register hints, and volatile coercion
    cheats stripped to out_path. Returns the count of stripped constructs.

    `source_text` overrides what is read from disk, so a caller can substitute a
    candidate body first and still get the cheat-strip applied to the RESULT —
    the order matters: a candidate carrying cheats must not score as if it had
    none."""
    text = (source_text if source_text is not None
            else Path(f"src/{stem}.c").read_text(encoding="utf-8"))
    stripped, n = strip_cheat_asm_file(text)
    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    Path(out_path).write_text(stripped)
    return n


def substitute_body(text: str, func: str, new_body: str) -> str:
    """Return `text` with `func`'s definition replaced by `new_body`.

    Handles both representations a function can have on main: the
    `INCLUDE_ASM("asm/funcs", func);` line that every INCOMPLETE function
    carries since asm-until-matched (owner ruling 2026-08-19), and an existing
    C definition. Raises KeyError when neither is present, because silently
    appending or no-op'ing would produce a score for something other than what
    the caller asked about — the exact class of lie this exists to detect."""
    for name, start, end in include_asm_spans(text):
        if name == func:
            # The macro invocation carries a trailing `;` the span excludes.
            tail = end
            while tail < len(text) and text[tail] in " \t":
                tail += 1
            if tail < len(text) and text[tail] == ";":
                end = tail + 1
            return text[:start] + new_body.rstrip("\n") + text[end:]
    span = _func_body_span(text, func)
    if span is None:
        raise KeyError(
            f"{func} has neither an INCLUDE_ASM line nor a C definition in this "
            f"source — cannot substitute a candidate body for it")
    # _match_brace returns the index JUST PAST '}', so span[1] is exclusive
    # (pinned by func_cheat_asm_count's `lo <= s < hi`). Slicing at span[1]+1
    # would swallow the character after the body.
    return text[:span[0]] + new_body.rstrip("\n") + text[span[1]:]


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

    Two real shapes in this tree that a strict column-0 + `)`-then-`{` rule
    missed, leaving the function's cheat count UNKNOWN and its body's cheats
    invisible to the completion gate:
      * `}s32 func_8007DE08(s32 arg0) {`  (display.c) — the definition shares a
        line with the previous function's closing brace. A single optional `}`
        is allowed before the return type; arbitrary indentation still is NOT,
        because that is what distinguishes a definition from a call.
      * `void func_8004A1FC(arg0) s16 *arg0; {` (text1b.c) — old-style K&R
        parameter declarations sit between `)` and `{`.
    """
    # Leading whitespace is permitted ONLY after a closing `}`; a bare indented
    # line stays non-matching, which is what separates a definition from a call.
    pattern = re.compile(r"(?m)^(?:\}[ \t]*)?[A-Za-z_][\w \t\*]*\b"
                         + re.escape(func) + r"\s*\(")
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
        if j < n and text[j] != "{":
            # Possible K&R parameter declarations between `)` and `{`: a run of
            # `type name[, name];` statements. Accept only that shape — anything
            # else (`;` = a plain declaration, `=`, `(`) rejects the candidate.
            k = j
            while True:
                dm = re.compile(r"[A-Za-z_][\w \t\*,\[\]]*;").match(text, k)
                if not dm:
                    break
                k = dm.end()
                while k < n and text[k] in " \t\n\r":
                    k += 1
            j = k if (k > j and k < n and text[k] == "{") else j
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
        # No C body. If the file supplies this function wholly from asm
        # (INCLUDE_ASM invocation or a `glabel <func>` block), that construct IS
        # the cheat construct and is attributed to `func` — otherwise a function
        # with zero lines of C would count as clean and be recorded COMPLETED-C.
        # Only when nothing explains the symbol do we report -1 (UNKNOWN), which
        # callers must treat as not-clean rather than as zero.
        return 1 if func in whole_body_asm_funcs(text) else -1
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
