"""Cheat-asm stripping for the cheat-invisible sandbox + cheat-asm campaign.

Cheat-asm (workaround) `__asm__` blocks, register-asm pins, and plain
`register` storage hints are injection/allocation cheats: the sandbox strips
them so the score reflects pure-C codegen — injecting asm or nudging the
allocator can't move the score. Canonical inline asm
(GTE/cop2/.word-cop2/BIOS/HW) is authentic and is KEPT. A mixed block (any
canonical instruction) is kept whole (canonical GTE sequences include their
feeder loads). For the sandbox only, the cop2-free statements of a
header-exact PsyQ GTE macro unit are kept too (engine.gtemacro, owner ruling
2026-09-25).

Category classification reuses tools/classify_inline_asm (single source of truth
for the canonical-vs-cheat line). Stripping a function's cheat asm doesn't change
OTHER functions' .text bytes (only file-scope label numbers shift), so a
file-wide strip + masked scoring triages every cheat-asm function in one build.
"""
from __future__ import annotations

import bisect
import functools
import re
import sys
from pathlib import Path
from typing import NamedTuple

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
# so is never found by cia.find_asm_keywords. It only becomes `__asm__` after cpp,
# which the stripper never runs. The construct was therefore never even a
# CANDIDATE for stripping — it was not classified and kept, it was unseen. That
# blind spot let a function with zero lines of decompiled C report an honest
# pure-C sandbox distance of 0 (measured on ang_hosei: recorded distance 50,
# post-conversion `sandbox --disable all` score 0), and made
# func_cheat_asm_count return -1, which queue.generate/mark_done read as clean.
_INCLUDE_ASM_MACRO_RE = re.compile(
    r'(?m)^[ \t]*INCLUDE_ASM\s*\(\s*"[^"]*"\s*,\s*([A-Za-z_]\w*)\s*\)\s*;?[ \t]*')
# BIOS_[ABC]_FUNCTION(name, id) (include/bios.h): the BIOS vector trampoline
# macro. Same fact as an INCLUDE_ASM line -- a whole-body canonical asm function
# supplied without C -- so include_asm_spans reports it alongside INCLUDE_ASM.
# The id (and the direct BIOS_FUNCTION form's vector) must be a plain numeric
# literal: the macro stringifies them into asm text, so anything else could
# smuggle extra instructions (layer-2 hardening note, 2026-10-01).
_BIOS_MACRO_RE = re.compile(
    r'(?m)^[ \t]*BIOS_(?:[ABC]_FUNCTION\s*\(\s*([A-Za-z_]\w*)\s*,'
    r'|FUNCTION\s*\(\s*([A-Za-z_]\w*)\s*,\s*0[xX][ABCabc]0\s*,)'
    r'\s*(?:0[xX][0-9A-Fa-f]+|[0-9]+)\s*\)\s*;?[ \t]*')
# DETECTION (fail closed): ANY invocation of the BIOS trampoline macros, any
# argument text, anywhere -- the macros expand to __asm__ the unexpanded-text
# scanners cannot see. Every match is a strip span (counted, removed from the
# sandbox source); only the strict _BIOS_MACRO_RE above ACCEPTS an invocation
# as a function's whole-body asm.
_BIOS_ANY_RE = re.compile(r'\bBIOS_(?:[ABC]_)?FUNCTION\s*\(')


def bios_macro_spans(text: str) -> list[tuple[int, int]]:
    """(start, end) of every BIOS_[ABC]_FUNCTION / BIOS_FUNCTION invocation in
    code (comments and strings ignored), through the balanced `)` and an
    optional `;`. An unbalanced invocation spans to end of text (fail closed)."""
    code = _code_without_comments_and_strings(text)
    out = []
    for m in _BIOS_ANY_RE.finditer(code):
        line_start = code.rfind('\n', 0, m.start()) + 1
        if code[line_start:m.start()].lstrip().startswith(('#', '%:')):
            continue  # a macro definition, not an invocation
        depth, i = 0, m.end() - 1
        while i < len(code):
            if code[i] == '(':
                depth += 1
            elif code[i] == ')':
                depth -= 1
                if depth == 0:
                    break
            i += 1
        end = min(i + 1, len(text))
        while end < len(text) and text[end] in ' \t':
            end += 1
        if end < len(text) and text[end] == ';':
            end += 1
        out.append((m.start(), end))
    return out


_INCLUDE_DIRECTIVE_RE = re.compile(r'\.include\s+\\?"[^"\\]*?/([A-Za-z_]\w*)\.s\\?"')
_GLABEL_NAME_RE = re.compile(r'\bglabel\s+([A-Za-z_]\w*)')


def include_asm_spans(text: str) -> list[tuple[str, int, int]]:
    """(func, start, end) for every `INCLUDE_ASM(FOLDER, func)` invocation and
    every `BIOS_[ABC]_FUNCTION(func, id)` trampoline invocation (include/bios.h,
    inline-asm audit A3 2026-10-01) -- both supply a whole function from asm.

    Anchored at line start, so a `#define INCLUDE_ASM(...)` in a header and a
    commented-out invocation do not match.
    """
    spans = [(m.group(1), m.start(), m.end())
             for m in _INCLUDE_ASM_MACRO_RE.finditer(text)]
    spans += [(m.group(1) or m.group(2), m.start(), m.end())
              for m in _BIOS_MACRO_RE.finditer(text)]
    return sorted(spans, key=lambda t: t[1])


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
    for kw in cia.find_asm_keywords(text):
        if kw.directive or kw.end < 0:
            continue
        body = kw.body
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
    for kw in cia.find_asm_keywords(text):
        if kw.directive or kw.end < 0:
            continue
        body = kw.body
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
    return cia.is_directive_lead(text[line_start:idx])


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
    invocations, asm reached through a macro name (gtemacro.macro_strip_spans),
    + the asm("$N") qualifier of register pins (the variable declaration itself
    is kept).

    INCLUDE_ASM is stripped because it is not compiled C: leaving it in would let
    the sandbox assemble the target bytes straight from asm/funcs/<name>.s and
    report an honest pure-C distance of 0 for a function with no C at all.
    Whole-body `glabel` __asm__ blocks are deliberately NOT stripped here —
    _block_category calls them "canonical_body" and that behaviour is unchanged
    at file scope; a `glabel` block inside a C function body is a cheat.

    Memoized by text: this is a whole-file scan and `func_cheat_asm_count`
    calls it once per FUNCTION, so a 400-function TU used to re-scan itself 400
    times. Returns a fresh list each call so callers can mutate the result
    without corrupting the cache.
    """
    return list(_strip_spans_cached(text))


@functools.lru_cache(maxsize=8)
def _strip_spans_cached(text: str) -> tuple[tuple[int, int], ...]:
    spans = [(s, e) for _f, s, e in include_asm_spans(text)]
    # A strict invocation is already a span (include_asm_spans); the detection
    # span of the same invocation can differ only in its edges (indent,
    # trailing blanks). Overlapping spans would be deleted one after the other
    # with stale offsets, so a BIOS span overlapping any existing span is
    # dropped -- the invocation is already stripped and counted once.
    for bs, be in bios_macro_spans(text):
        if not any(bs < e and s < be for s, e in spans):
            spans.append((bs, be))
    # Asm reached through a macro name (engine.gtemacro): an invocation of a
    # foreign header's asm macro, a #define/#undef shadowing a vendored PsyQ
    # macro, a call of a vendored macro without a cop2 or trap instruction
    # (gte_nop()). Admitted vendored calls are kept: cpp expands them in the
    # sandbox as in the build, and the completion gate treats them as islands.
    from . import gtemacro  # local import: gtemacro imports this module
    for ms, me in gtemacro.macro_strip_spans(text):
        if not any(ms < e and s < me for s, e in spans):
            spans.append((ms, me))
    # find_asm_keywords sees the keyword the way cc1 does: comments and
    # backslash-newlines between `__asm__`, its qualifiers and `(` are
    # whitespace, and a comment/string that merely mentions `__asm__` is not
    # the keyword. (The raw-text regex it replaces missed
    # `__asm__ /**/ ("move $5,%0" : : "r"(n));` in both gate and sandbox.)
    for kw in cia.find_asm_keywords(text):
        if kw.directive:
            continue  # macro definition — leave (stripping would break uses)
        if kw.end < 0:
            # No `( ... )` follows the keyword — a shape nothing here parses
            # (e.g. a macro argument `Q(__asm__)`). Fail safe: strip the bare
            # keyword, so the gate counts it and the sandbox cannot compile it.
            spans.append((kw.start, kw.kw_end))
            continue
        category = _block_category(kw.body)
        if category == "canonical_body" and kw.depth > 0:
            # A `glabel` inside a C function body is not a whole-function asm
            # body (those sit at file scope); it is a decoy label on inline asm.
            category = "cheat"
        if category != "cheat":
            continue
        end = kw.end
        while end < len(text) and text[end] in " \t":
            end += 1
        if end < len(text) and text[end] == ";":
            end += 1
        spans.append((kw.start, end))
    for pm in cia.REGISTER_PIN_RE.finditer(text):
        am = _PIN_QUALIFIER.search(pm.group(0))
        if am:
            spans.append((pm.start() + am.start(), pm.start() + am.end()))
    return tuple(spans)


def strip_cheat_asm_file(text: str, keep_gte_macro_units: bool = False) -> tuple[str, int]:
    """Return (modified text, count) with all cheat-asm blocks, register pins,
    plain `register` hints, AND every volatile-coercion cheat (alias renames,
    `*(volatile T *)&D_x` casts, scalar `extern volatile T D_x;` decls — see
    engine.volatile_cheats) stripped. Count is the sum of these cheat kinds.

    Both removals serve the same purpose: the sandbox sees the function the
    way `queue done` requires for COMPLETED-C (pure C, no codegen-coercion
    knobs), so the score reflects the honest pure-C distance.

    keep_gte_macro_units (the sandbox only — write_stripped): the statements of
    a header-exact PsyQ GTE macro unit (engine.gtemacro; owner ruling
    2026-09-25) are scored as written instead of stripped, and are not counted.
    The default stays False for every other caller: a whole-file count of 0 is
    read as "no cheat constructs here" (spot_check_completed), and the
    completion gate (func_cheat_asm_count) still counts those statements.
    """
    spans = _strip_spans(text)
    if keep_gte_macro_units:
        from . import gtemacro  # local import: gtemacro imports this module
        kept = {(s, e) for s, e, _m in gtemacro.unit_spans(text)}
        spans = [sp for sp in spans if sp not in kept]
    spans = sorted(spans + register_hint_spans(text), reverse=True)
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
    none. Header-exact GTE macro units are kept (engine.gtemacro)."""
    text = (source_text if source_text is not None
            else Path(f"src/{stem}.c").read_text(encoding="utf-8"))
    stripped, n = strip_cheat_asm_file(text, keep_gte_macro_units=True)
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
      * `void calc_LightMatrix(arg0) Unk800F62E0Rec *arg0;` (368E4.c) — old-style K&R
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


# --- same-TU `static inline` helpers (owner ruling Q126, 2026-10-23) ---------
# "A GTE macro call may sit in a `static inline` helper defined in the same TU;
# the engine attributes a called helper's calls and tokens to each caller
# (region hashes, layer-2 key, integrity)." A helper GCC inlines has no symbol
# of its own, so no per-function gate would ever read it: every gate that reads
# a function's body (completion islands and region hashes, this cheat count,
# engine.volatile_cheats, the layer-2 key) reads its helpers' bodies too, as
# part of the caller. Scope: `static` + `inline` (`__inline`, `__inline__`)
# definitions in the same .c text only — not plain statics (a function of
# their own, audited as such), not headers.
_INLINE_WORDS = frozenset({"inline", "__inline", "__inline__"})
_IDENT_TOKEN_RE = re.compile(r"[A-Za-z_]\w*")
_ATTRIBUTE_RE = re.compile(r"\b__attribute(?:__)?\s*\(")
_PP_DEFINE_RE = re.compile(r"[ \t]*#[ \t]*define[ \t]+([A-Za-z_]\w*)(.*)")


class TUFunction(NamedTuple):
    """One file-scope function body found in the compiler view."""
    name: str
    words: frozenset    # identifiers of the head before the name (storage, type)
    start: int          # original-text index of the head's first token
    end: int            # original-text index just past the closing `}`

    @property
    def static_inline(self) -> bool:
        return "static" in self.words and bool(self.words & _INLINE_WORDS)


class _TUIndex(NamedTuple):
    masked: str
    pos: tuple
    functions: tuple    # TUFunction, file order
    macros: dict        # #define name -> identifiers of its body (and params)
    unbalanced: bool    # unsafe brace walk, including file-scope brace macro use


def _head_initializer(head: str) -> bool:
    """A top-level assignment `=`, excluding comparisons and ()/[] contents."""
    parens = brackets = 0
    for i, c in enumerate(head):
        if c == "(":
            parens += 1
        elif c == ")":
            parens -= 1
        elif c == "[":
            brackets += 1
        elif c == "]":
            brackets -= 1
        elif c == "=" and parens == 0 and brackets == 0:
            before = head[i - 1] if i else ""
            after = head[i + 1] if i + 1 < len(head) else ""
            if before not in ("=", "<", ">", "!") and after != "=":
                return True
    return False


def _head_function(head: str) -> tuple[str, frozenset] | None:
    """(name, head words) when a file-scope `head {` is a function definition
    head: it has a parameter list and no top-level initializer `=`. Attribute
    groups are skipped; the name is the identifier before the first
    remaining top-level `(`."""
    if _head_initializer(head):
        return None
    m = _ATTRIBUTE_RE.search(head)
    while m:
        close = _match_paren(head, m.end() - 1)
        if close < 0:
            return None
        head = head[:m.start()] + " " * (close - m.start()) + head[close:]
        m = _ATTRIBUTE_RE.search(head)
    p = head.find("(")
    if p < 0:
        return None
    nm = re.search(r"([A-Za-z_]\w*)\s*$", head[:p])
    if not nm:
        return None
    return nm.group(1), frozenset(_IDENT_TOKEN_RE.findall(head[:nm.start()]))


def _tu_index(text: str) -> _TUIndex:
    """Index functions/#defines in one masked view of TU text only.

    Headers and #include'd code are unseen; directives are skipped, #if 0
    bodies kept. Comments/literals are blanked and line splices removed.
    `unbalanced` refuses:
    - literal-view disagreement, $, form feed/vertical tab, alternate tokens,
      or unparsed define/conditional-looking directives;
    - unmatched braces or nonzero/negative branch deltas (nesting independent);
    - unbalanced macros, file-scope brace/paste macros (aliases included),
      macro names/words in heads, unknown heads, or duplicate ordinary names.
    Clear initializers/specifiers and body-only balanced macros are allowed.
    """
    return _tu_index_cached(text)


@functools.lru_cache(maxsize=4)
def _tu_index_cached(text: str) -> _TUIndex:
    view = cia.compiler_view(text)
    masked, pos = view.masked, view.pos
    n = len(masked)
    functions, macros = [], {}
    depth, item, i, bol = 0, 0, 0, True
    unbalanced = (masked != view.masked_eol or any(token in masked for token in
                  ("$", "\f", "\v", "%:", "<%", "%>", "??=", "??<", "??>")))
    branch_deltas = []  # innermost branch owns code braces, not directive braces
    brace_macros, file_scope_names = set(), set()
    segs = [0]          # file-scope item starts since the last `}` (K&R heads)
    head_fn = None
    while i < n:
        c = masked[i]
        if bol and c == "#":
            eol = masked.find("\n", i)
            eol = n if eol < 0 else eol
            dm = _PP_DEFINE_RE.match(masked, masked.rfind("\n", 0, i) + 1, eol)
            if dm:
                macros.setdefault(dm.group(1), set()).update(
                    _IDENT_TOKEN_RE.findall(dm.group(2)))
                if any(token in dm.group(2) for token in ("{", "}", "##")):
                    brace_macros.add(dm.group(1))
                macro_depth = 0
                for brace in dm.group(2):
                    if brace == "{":
                        macro_depth += 1
                    elif brace == "}":
                        macro_depth -= 1
                        if macro_depth < 0:
                            unbalanced = True
                if macro_depth != 0:
                    unbalanced = True
            directive = re.match(r"#[ \t]*(\w+)", masked[i:eol])
            looking = re.match(r"#\s*(define|if\w*|elif|else|endif)\b", masked[i:eol])
            if looking:
                kind = looking.group(1)
                if (not directive or directive.group(1) != kind
                        or (kind == "define" and not dm)
                        or kind not in ("define", "if", "ifdef", "ifndef",
                                        "elif", "else", "endif")):
                    unbalanced = True
            if directive:
                kind = directive.group(1)
                if kind in ("if", "ifdef", "ifndef"):
                    branch_deltas.append(0)
                elif kind in ("elif", "else", "endif") and branch_deltas:
                    if branch_deltas[-1] != 0:
                        unbalanced = True
                    if kind == "endif":
                        branch_deltas.pop()
                    else:
                        branch_deltas[-1] = 0
            if depth == 0:
                item = eol
                segs.append(eol)
            i = eol
            continue
        if c == "\n":
            bol = True
        elif c not in " \t\r\f\v":
            bol = False
        if depth == 0 and (i == 0 or not (
                masked[i - 1].isalnum() or masked[i - 1] == "_")):
            ident = _IDENT_TOKEN_RE.match(masked, i)
            if ident:
                file_scope_names.add(ident.group())
        if c == "{":
            if branch_deltas:
                branch_deltas[-1] += 1
            if depth == 0:
                head = masked[item:i]
                if not head.strip():
                    # K&R: `T f(a) T a; {` -- the head is the nearest item
                    # with a parameter list; the rest declare parameters
                    for st in reversed(segs[:-1]):
                        seg = masked[st:item].split(";")[0]
                        if "(" in seg and not _head_initializer(seg):
                            item = st
                            break
                    head = masked[item:i]
                hf = _head_function(head)
                if (hf is None and not _head_initializer(head)
                        and not re.search(r"\b(?:struct|union|enum)"
                                          r"(?:\s+[A-Za-z_]\w*)?\s*$", head)):
                    unbalanced = True
                lead = len(head) - len(head.lstrip())
                head_fn = (hf, item + lead) if hf else None
            depth += 1
        elif c == "}":
            if branch_deltas:
                branch_deltas[-1] -= 1
                if branch_deltas[-1] < 0:
                    unbalanced = True
            if depth == 0:
                unbalanced = True
            depth = max(0, depth - 1)
            if depth == 0:
                if head_fn:
                    (name, words), hs = head_fn
                    functions.append(TUFunction(name, words, pos[hs], pos[i] + 1))
                head_fn = None
                item = i + 1
                segs = [item]
        elif c == ";" and depth == 0:
            item = i + 1
            segs.append(item)
        i += 1
    while True:
        reached = {name for name, body in macros.items() if body & brace_macros}
        if reached <= brace_macros:
            break
        brace_macros.update(reached)
    ordinary_names = set()
    for fn in functions:
        if fn.name in macros or fn.words & macros.keys():
            unbalanced = True
        if not fn.static_inline:
            if fn.name in ordinary_names:
                unbalanced = True
            ordinary_names.add(fn.name)
    return _TUIndex(masked, pos, tuple(functions),
                    {k: frozenset(v) for k, v in macros.items()},
                    unbalanced or depth != 0 or any(branch_deltas)
                    or bool(brace_macros & file_scope_names))


def tu_unbalanced(text: str) -> bool:
    """Whether one masked TU view is unsafe to index (headers/includes unseen):
    - lexical view/identifier/directive escapes;
    - brace/branch imbalance;
    - unsafe macros/heads or duplicate ordinary definitions.
    See `_tu_index` for exceptions and the complete refusal list.
    """
    return _tu_index(text).unbalanced


def tu_functions(text: str) -> tuple[TUFunction, ...]:
    """Every file-scope function body of `text` (compiler view), file order."""
    return _tu_index(text).functions


def static_inline_defs(text: str) -> dict[str, list[tuple[int, int]]]:
    """name -> definition spans (start, end) of every `static inline`
    function defined in `text`, in file order. More than one span: the name
    is ambiguous (e.g. an `#if 0` copy) and every gate fails closed on it."""
    if "inline" not in text:
        return {}
    out: dict[str, list[tuple[int, int]]] = {}
    for f in tu_functions(text):
        if f.static_inline:
            out.setdefault(f.name, []).append((f.start, f.end))
    return out


def macro_named_helpers(text: str) -> set[str]:
    """`static inline` helpers whose name appears in a #define body."""
    defs = static_inline_defs(text)
    if not defs:
        return set()
    named: set[str] = set()
    for body in _tu_index(text).macros.values():
        named |= body
    return named & defs.keys()


# `T name;` / `T *name = ...` / `T name[...]`: a local declaration of {0}
_LOCAL_DECL = (r"(?:^|[;{{}}(,])\s*(?!(?:return|case|goto|else|do|sizeof)\b)"
               r"(?:[A-Za-z_]\w*[\s\*]+)+\**\s*{0}\s*[;=\[,)]")
_OBJECT_SPECIFIERS = frozenset(
    "void char short int long float double signed unsigned const volatile "
    "register auto static struct union enum".split())


def _refs(idx: _TUIndex, span: tuple[int, int], names) -> set[str]:
    """Names (of `names`) a body refers to: identifiers not after `.` / `->`,
    each expanded through the TU's #define bodies, transitively. Discard a
    non-macro-reached helper only if declared as a parameter in the head's
    top-level parameter list or a local at brace depth 1 whose declarator is
    the first non-member occurrence of the name after the head and is at
    parenthesis depth 0 since the last `;`, `{` or `}`, and neither
    `name(...)` nor `(name)(...)` (with optional whitespace) occurs anywhere.
    Nested-block declarations never discard helpers: fail closed for other
    call forms and address-taking outside the nested block. The declaration
    test is heuristic and errs toward keeping the helper; a name immediately
    preceded by `struct`, `union` or `enum` is a tag, never a shadowing name.
    Brace depth starts at the head's opening `{`, ignoring any preceding
    stray `}`. A preprocessor directive line or any same-TU macro identifier
    in the span disables all shadow-based discards, retaining every found
    helper. Macros defined in headers are not seen. A depth-1 local must have
    no `extern` / `typedef` specifier and only C keywords in _OBJECT_SPECIFIERS
    (after removing struct/union/enum tag pairs) before its name; pointer/array/
    initializer syntax alone is insufficient. This object test does not
    apply to head parameters. All typedef prefixes, including project integer
    typedefs, keep the helper because they may denote function types. Locate
    the first top-level parameter list after blanking attributes, then its
    following body brace. Missing lists/braces, any `{` before the matching
    `)`, or non-blank text between `)` and the body brace disable all shadow
    discards (including K&R declarations and trailing attributes)."""
    masked, pos = idx.masked, idx.pos
    lo = bisect.bisect_left(pos, span[0])
    hi = bisect.bisect_left(pos, span[1])
    body = masked[lo:hi]
    occurrences = []
    direct, todo = set(), []
    for m in _IDENT_TOKEN_RE.finditer(body):
        k = m.start() - 1
        while k >= 0 and body[k] in " \t\n":
            k -= 1
        if k >= 0 and (body[k] == "." or (body[k] == ">" and k > 0 and body[k - 1] == "-")):
            continue
        direct.add(m.group(0))
        occurrences.append((m.group(0), m.start()))
    found, seen, via_macro = set(), set(), set()
    todo = [(i, False) for i in direct]
    while todo:
        ident, expanded = todo.pop()
        if (ident, expanded) in seen:
            continue
        seen.add((ident, expanded))
        if ident in names:
            found.add(ident)
            if expanded:
                via_macro.add(ident)
        todo += [(i, True) for i in idx.macros.get(ident, ())]
    if (re.search(r"(?m)^[^\S\n]*#", body)
            or set(_IDENT_TOKEN_RE.findall(body)) & idx.macros.keys()):
        return found
    # Attributes are not the head's parameter list. Preserve offsets.
    head = body
    for attr in reversed(list(_ATTRIBUTE_RE.finditer(head))):
        close = _match_paren(head, attr.end() - 1)
        if close < 0:
            return found
        head = head[:attr.start()] + " " * (close - attr.start()) + head[close:]
    param_open = head.find("(")
    param_close = _match_paren(head, param_open) if param_open >= 0 else -1
    if param_close < 0 or "{" in body[:param_close]:
        return found
    body_open = body.find("{", param_close)
    if body_open < 0 or body[param_close:body_open].strip():
        return found
    head = head[:body_open]
    first_use = {}
    for ident, offset in occurrences:
        if offset > body_open:
            first_use.setdefault(ident, offset)
    # Spans include the function head; depth 1 is its outermost block.
    depths, parens, depth, statement_parens = [], [], 0, 0
    for i, char in enumerate(body):
        depths.append(depth)
        parens.append(statement_parens)
        if body_open < 0 or i < body_open:
            continue
        if char in ";{}":
            statement_parens = 0
        elif char == "(":
            statement_parens += 1
        elif char == ")":
            statement_parens -= 1
        if char == "{":
            depth += 1
        elif char == "}":
            depth = max(0, depth - 1)

    def object_decl(m) -> bool:
        prefix = body[m.start():m.start("shadow")]
        words = set(_IDENT_TOKEN_RE.findall(prefix))
        if words & {"extern", "typedef"}:
            return False
        specifiers = re.sub(r"\b(?:struct|union|enum)\s+[A-Za-z_]\w*", "", prefix)
        return set(_IDENT_TOKEN_RE.findall(specifiers)) <= _OBJECT_SPECIFIERS

    for name in found - via_macro:
        decls = re.finditer(_LOCAL_DECL.format("(?P<shadow>" + re.escape(name) + ")"), body)
        whole_body_local = any(
            (depths[m.start("shadow")] == 1
             and parens[m.start("shadow")] == 0
             and m.start("shadow") == first_use.get(name)
             and object_decl(m))
            or (param_open < m.start("shadow") < param_close
                and head[param_open:m.start("shadow")].count("(")
                - head[param_open:m.start("shadow")].count(")") == 1)
            for m in decls
            if not re.search(r"\b(?:struct|union|enum)\s*$",
                             body[:m.start("shadow")]))
        if (whole_body_local
                and not re.search(r"(?:\b" + re.escape(name) + r"\s*\(|\(\s*"
                                  + re.escape(name) + r"\s*\)\s*\()", body)):
            found.discard(name)
    return found


def inline_helpers(text: str, func: str) -> list[str]:
    """The same-TU `static inline` functions `func` reaches, transitively
    (cycle-safe; `func` itself excluded), in file order. A helper counts when
    its name appears as an identifier in a reached body (a call, or its
    address taken) or in a #define body such an identifier expands to — read
    in the compiler view, so comments and literals never count and a
    backslash-newline inside a name does not hide it. An ambiguous helper
    (more than one definition) is listed; attributed_spans refuses it."""
    defs = static_inline_defs(text)
    if not defs:
        return []
    span = _func_body_span(text, func)
    if span is None:
        return []
    return list(_inline_helpers_cached(text, func, span))


@functools.lru_cache(maxsize=256)
def _inline_helpers_cached(text: str, func: str, span: tuple[int, int]) -> tuple[str, ...]:
    defs = static_inline_defs(text)
    idx = _tu_index(text)
    seen: set[str] = set()
    todo = [span]
    while todo:
        for name in _refs(idx, todo.pop(), defs.keys()):
            if name != func and name not in seen:
                seen.add(name)
                todo += defs[name]
    return tuple(sorted(seen, key=lambda n: defs[n][0]))


def attributed_spans(text: str, func: str) -> list[tuple[str, tuple[int, int]]]:
    """(name, definition span) of `func`'s body and then of each helper it
    reaches (inline_helpers): every span a per-function gate reads for `func`.
    [] when `func` has no C body. Raises ValueError when a reached helper has
    more than one definition (ambiguous: fail closed)."""
    span = _func_body_span(text, func)
    if span is None:
        return []
    defs = static_inline_defs(text)
    out = [(func, span)]
    for h in inline_helpers(text, func):
        if len(defs[h]) != 1:
            raise ValueError(f"{func}: static inline helper {h} has {len(defs[h])} "
                             f"definitions (ambiguous)")
        out.append((h, defs[h][0]))
    return out


def func_cheat_asm_count(text: str, func: str) -> int:
    """Number of cheat constructs inside func's body — and inside every
    same-TU `static inline` helper it reaches (inline_helpers, owner ruling
    Q126): cheat-asm strip-spans (asm blocks + register pins), plain
    `register` hints, and volatile-coercion cheats (alias renames +
    `*(volatile T *)&D_x` casts + scalar `extern volatile T D_x;` decls — see
    engine.volatile_cheats, which reads the helpers as part of `func` too).
    -1 if the body can't be located.

    `queue done` (engine/queue.py) refuses to mark COMPLETED-C if this count > 0
    for a non-canonical-asm function, so adding volatile cheats here is what
    makes them a hard-stop in the same way `register T x asm("$N")` pins
    already are.
    """
    try:
        spans = attributed_spans(text, func)
    except ValueError:
        return -1  # an ambiguous helper: unknown, never clean
    if not spans:
        # No C body. If the file supplies this function wholly from asm
        # (INCLUDE_ASM invocation or a `glabel <func>` block), that construct IS
        # the cheat construct and is attributed to `func` — otherwise a function
        # with zero lines of C would count as clean and be recorded COMPLETED-C.
        # Only when nothing explains the symbol do we report -1 (UNKNOWN), which
        # callers must treat as not-clean rather than as zero.
        return 1 if func in whole_body_asm_funcs(text) else -1
    strip_spans = _strip_spans(text) + register_hint_spans(text)
    cheat_asm = sum(1 for s, _e in strip_spans
                    if any(lo <= s < hi for _n, (lo, hi) in spans))
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
