"""Detector + stripper for `volatile` codegen-coercion cheats.

These patterns produce the same kind of byte coercion as a tier-3 `__asm__`
barrier or a `register T x asm("$N")` pin — they force GCC's scheduler/CSE/
register-allocator to emit a specific shape that the natural source wouldn't —
yet escape the existing `engine/inlineasm.py` tier-3 detector (which only walks
`__asm__(...)` blocks and `register T x asm("Y")` pins).

THREE coercion patterns this module catches:

1. **Alias-rename pattern (ALWAYS a cheat).** A `_v`-suffixed extern declaration
   re-aliases an existing global as `volatile` via an asm-name annotation:
       extern volatile s32 D_800A14E4_v asm("D_800A14E4");
   The original `D_800A14E4` is declared non-volatile elsewhere. The alias serves
   ONLY to defeat CSE / force re-materialization at uses of `_v`. There is no
   legitimate reason to re-declare a global as volatile under a synonym; if the
   memory were truly volatile, the primary declaration would be.

2. **Inline `*(volatile T *)&D_globalsym` casts on game-RAM symbols.** Treats a
   plain game-state global as volatile at the access site, defeating CSE/
   scheduling. Distinguished from `*(volatile T *)(ptr + offset)` (pointer-based,
   typically legitimate hardware access via a pointer the function received).

3. **Plain `extern volatile T D_globalsym;` (scalar) on game-RAM symbols.** Same
   as #2 but spelled at declaration. The `D_` prefix marks anonymous game state
   (splat-named) — `g_`-prefixed names (e.g. `g_cd_index_reg`, `g_gpu_data_reg`,
   `g_spu_dma_ctrl`) follow the project's hardware/system-global naming convention
   and are kept (the volatile reflects genuine hardware-mapped memory).

Pointer-to-volatile declarations (`extern volatile T *name;` — the VARIABLE is a
pointer, the POINTEE is volatile) are legitimate even on `D_`-named globals,
because the pointee is whatever address the pointer holds (typically a hardware
register address loaded at startup). Only SCALAR volatile is the coercion.

STRIPPING. The sandbox needs to see the function without these coercions so the
distance reflects honest pure-C. For each cheat span, the strip just removes the
`volatile` qualifier — the declarations and casts remain syntactically valid
without it, and the resulting C compiles with normal (non-coerced) semantics.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

# Project hardware/system convention: `g_`-prefixed globals are the legitimate
# volatile family (CD-ROM registers, SPU/GPU registers, DMA control, scratchpad
# pointers, ISR-shared state). Anything else with `volatile` is a candidate
# coercion.
_LEGIT_NAME_RE = re.compile(r"^g_")

# The cheat patterns target ANONYMOUS game-RAM globals (splat-named `D_xxxxxxxx`).
# A volatile cast on a LOCAL variable (e.g. `*(volatile s32 *)&ptr` where `ptr`
# is a function-local pointer being aliased) is a different idiom and not part
# of this audit. The detector requires the global-cast target to be a `D_`-
# prefixed splat name.
_GLOBAL_NAME_RE = re.compile(r"^D_[0-9A-Fa-f]+")

# Pattern 1: alias rename. `extern volatile T name asm("OrigSym");`
#  - Group "decl" = the full match (for span info)
#  - Group "name" = the C identifier
#  - Group "sym"  = the asm-name target symbol
# Match form supports `T name`, `T *name`, `T **name`, `T name[]`, etc.
_ALIAS_RENAME_RE = re.compile(
    r"(?P<decl>\bextern\s+(?P<vol>volatile)\s+"
    r"(?:\w+\s+)+"           # type words
    r"\**\s*(?P<name>\w+)"   # optional pointer stars + identifier
    r"\s*(?:\[[^\]]*\])?"     # optional array suffix
    r"\s*asm\s*\(\s*\"(?P<sym>\w+)\"\s*\)\s*;)"
)

# Pattern 2: inline `*(volatile T *)&<GLOBAL>` cast (game-state global by name)
# We require the cast target to be `volatile T *` (not `volatile T **`) and the
# operand to be `&IDENT` where IDENT is a non-`g_` symbol (game state).
_GLOBAL_CAST_RE = re.compile(
    r"\*\s*\(\s*(?P<vol>volatile)\s+"
    r"(?:\w+\s+)+"            # type words
    r"\*\s*\)\s*&\s*(?P<name>\w+)"
)

# Pattern 3: plain scalar `extern volatile T <GLOBAL>;`. Distinguished from
# pointer-to-volatile (`extern volatile T *X;`) by absence of `*` between the
# type and the identifier. The identifier must be a game-RAM symbol (not `g_`).
_PLAIN_VOLATILE_RE = re.compile(
    r"(?P<decl>\bextern\s+(?P<vol>volatile)\s+"
    r"(?:\w+\s+)+"            # type words (must NOT contain pointer stars; enforced
                              # by the next char NOT being `*` — checked manually)
    r"(?P<name>\w+)"          # plain identifier (no `*` prefix)
    r"\s*(?:\[[^\]]*\])?"      # optional array suffix
    r"\s*;)"
)


def _is_legit_name(name: str) -> bool:
    return bool(_LEGIT_NAME_RE.match(name))


def _decl_has_pointer(text: str, vol_start: int) -> bool:
    """Return True if the type span between `volatile` and the identifier
    contains a `*` — i.e. it is a pointer-to-volatile declaration (legitimate).
    `vol_start` is the position of `volatile` in `text`."""
    # Scan forward from past `volatile ` to the first identifier; if a `*` appears,
    # it's a pointer type. Skip whitespace and word chars (type names).
    i = vol_start + len("volatile")
    n = len(text)
    while i < n and text[i] in " \t":
        i += 1
    # Walk type-name tokens followed by spaces; track whether we see `*`.
    while i < n:
        c = text[i]
        if c == "*":
            return True
        if c == "[" or c == ";" or c == "=" or c == "," or c == "(":
            return False
        if c.isalnum() or c == "_":
            # consume identifier
            while i < n and (text[i].isalnum() or text[i] == "_"):
                i += 1
            # space/star follows?
            while i < n and text[i] in " \t":
                i += 1
            # if next is `*` -> pointer
            if i < n and text[i] == "*":
                return True
            # if next is identifier (another type word like `s32`) keep scanning;
            # if next is identifier and followed by `;` or `[`, it's the var name
            # and we've reached the end of the type without seeing `*`.
            # We'll check by peeking ahead at this point.
            # Peek: is the next token the variable name? Heuristic: if there is
            # ONE remaining word followed by `;`/`[`, it's the var name.
            j = i
            while j < n and (text[j].isalnum() or text[j] == "_"):
                j += 1
            while j < n and text[j] in " \t":
                j += 1
            if j < n and text[j] in ";[=,()":
                return False  # no `*` before var name -> scalar volatile
            continue
        return False
    return False


def find_alias_renames(text: str) -> list[tuple[int, int, str]]:
    """All `extern volatile T NAME asm("SYM");` alias renames where NAME != SYM.
    Returns (start, end, vol_keyword_index)."""
    out = []
    for m in _ALIAS_RENAME_RE.finditer(text):
        name, sym = m.group("name"), m.group("sym")
        if name == sym:
            continue  # same name; e.g. `extern volatile T g_x asm("g_x");`
        # legit-name on the ALIASED side? If sym is g_xxx, this is still cheat
        # (the alias gives access via a non-`g_` name). Treat as cheat regardless.
        vol_idx = m.start("vol")
        out.append((m.start(), m.end(), vol_idx))
    return out


def find_global_volatile_casts(text: str) -> list[tuple[int, int, str]]:
    """All `*(volatile T *)&<NAME>` casts where NAME is a splat-named global
    (D_xxxxxxxx pattern — anonymous game-RAM symbol). Returns (cast_start,
    cast_end_at_name, volatile_keyword_start)."""
    out = []
    for m in _GLOBAL_CAST_RE.finditer(text):
        name = m.group("name")
        if _is_legit_name(name):
            continue
        if not _GLOBAL_NAME_RE.match(name):
            continue  # exclude locals + non-splat globals
        out.append((m.start(), m.end(), m.start("vol")))
    return out


def find_plain_volatile_externs(text: str) -> list[tuple[int, int, str]]:
    """All `extern volatile T NAME;` declarations where:
      - The declaration is SCALAR (no `*` between type and NAME),
      - NAME is `D_xxxxxxxx` (splat-named anonymous global, i.e. game RAM).
    Returns (start, end, vol_keyword_index)."""
    out = []
    for m in _PLAIN_VOLATILE_RE.finditer(text):
        name = m.group("name")
        if _is_legit_name(name):
            continue
        if not _GLOBAL_NAME_RE.match(name):
            continue
        # Verify scalar (no pointer star between type and NAME)
        if _decl_has_pointer(text, m.start("vol")):
            continue
        out.append((m.start(), m.end(), m.start("vol")))
    return out


def find_all_cheats(text: str) -> list[tuple[int, int, str]]:
    """All three categories combined, sorted by start position. Each tuple is
    (cheat_span_start, cheat_span_end_or_anchor, volatile_keyword_position).

    Note: spans may OVERLAP in pathological text; the strip path tolerates this
    by removing the volatile keyword by exact position (idempotent)."""
    out = []
    out.extend(find_alias_renames(text))
    out.extend(find_global_volatile_casts(text))
    out.extend(find_plain_volatile_externs(text))
    out.sort()
    return out


_VOLATILE_KW_RE = re.compile(r"\bvolatile\b")


def strip_volatile_cheats_file(text: str) -> tuple[str, int]:
    """Return (modified text, count) with the `volatile` qualifier removed from
    every detected cheat declaration/cast. The strip removes ONLY the qualifier
    word (and one trailing space) — the rest of the declaration/cast survives
    untouched so the file still parses.

    The strip is byte-position-driven: we collect the volatile-keyword positions
    from all detectors, deduplicate, and rewrite the file by deleting each
    keyword (+ the following whitespace run, up to 1 char) in reverse order.
    Order matters because earlier positions shift later positions when text is
    edited; reverse-order edits preserve every position."""
    cheats_ = find_all_cheats(text)
    # Deduplicate volatile-keyword positions (same keyword can be referenced
    # from multiple detectors if patterns overlap).
    vol_positions = sorted({c[2] for c in cheats_}, reverse=True)
    n_stripped = 0
    for pos in vol_positions:
        # Sanity: the position should start with the `volatile` token.
        m = _VOLATILE_KW_RE.match(text, pos)
        if not m:
            continue
        # Remove the keyword + 1 trailing space (if any) so the rest stays clean.
        end = m.end()
        if end < len(text) and text[end] in " \t":
            end += 1
        text = text[:pos] + text[end:]
        n_stripped += 1
    return text, n_stripped


def write_stripped(stem: str, out_path: str) -> int:
    """Strip volatile-coercion cheats from src/<stem>.c into out_path. Returns
    count. Mirrors inlineasm.write_stripped's signature so it composes."""
    text = Path(f"src/{stem}.c").read_text()
    stripped, n = strip_volatile_cheats_file(text)
    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    Path(out_path).write_text(stripped)
    return n


def _func_body_span(text: str, func: str) -> tuple[int, int] | None:
    """Locate `func`'s DEFINITION body. Splat-generated src/*.c put each
    definition at column 0; calls and nested references are indented. We must
    skip extern declarations (`extern T func(args);`) which match the same
    column-0 pattern — these are recognised by the FIRST non-whitespace after
    the closing `)` being `;` (declaration) rather than `{` (definition)."""
    import re as _re

    pattern = _re.compile(r"(?m)^[A-Za-z_][\w \t\*]*\b" + _re.escape(func) + r"\s*\(")
    for m in pattern.finditer(text):
        # Walk forward to the matching `)` of the opening `(`.
        i, n = m.end(), len(text)
        depth = 1  # we matched the opening `(`
        in_str = False
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
        # Now i is past the closing `)`. Skip whitespace and find next non-ws.
        j = i
        while j < n and text[j] in " \t\n\r":
            j += 1
        if j >= n:
            continue
        if text[j] == ";":
            continue  # declaration, not definition
        if text[j] != "{":
            continue  # weird; skip
        # Match the closing `}` of the body.
        brace = j
        depth, k = 0, brace
        while k < n:
            c = text[k]
            if c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return (m.start(), k + 1)
            k += 1
    return None


def _names_used_in_body(text: str, lo: int, hi: int) -> set[str]:
    """Return the set of C identifiers referenced inside text[lo:hi]."""
    return set(re.findall(r"\b([A-Za-z_]\w*)\b", text[lo:hi]))


def func_volatile_cheat_count(text: str, func: str) -> int:
    """Number of volatile-coercion cheats AFFECTING `func`. Counts:
      - cheats whose source span is INSIDE the function body (inline casts,
        in-body extern decls), AND
      - file-scope alias-renames whose alias NAME is referenced in the body,
        AND
      - file-scope plain volatile externs whose SYMBOL name is referenced in
        the body.

    -1 if the body can't be located (caller falls back to no-signal). Counts
    each cheat once (by unique volatile-keyword position).

    Rationale: an alias-rename declared at file scope (between functions) is
    still load-bearing for any function that uses the alias's name — stripping
    the qualifier changes that function's codegen. The same is true for plain
    `extern volatile T D_x;` decls at file scope. The strip path already
    removes ALL such qualifiers file-wide; this count makes sure `mark_done`
    refuses every function the strip would touch."""
    span = _func_body_span(text, func)
    if span is None:
        return -1
    lo, hi = span
    used = _names_used_in_body(text, lo, hi)
    counted_vol_positions: set[int] = set()
    # In-body cheats (inline casts, in-body extern decls).
    for s, _e, vp in find_all_cheats(text):
        if lo <= s < hi:
            counted_vol_positions.add(vp)
    # File-scope alias renames whose alias name is referenced inside body.
    for s, e, vp in find_alias_renames(text):
        if lo <= s < hi:
            continue  # already counted above
        m = _ALIAS_RENAME_RE.search(text, s, e)
        if m and m.group("name") in used:
            counted_vol_positions.add(vp)
    # File-scope plain volatile externs whose symbol is referenced inside body.
    for s, e, vp in find_plain_volatile_externs(text):
        if lo <= s < hi:
            continue
        m = _PLAIN_VOLATILE_RE.search(text, s, e)
        if m and m.group("name") in used:
            counted_vol_positions.add(vp)
    return len(counted_vol_positions)


def file_func_volatile_cheat_count(stem: str, func: str) -> int:
    try:
        text = Path(f"src/{stem}.c").read_text()
    except OSError:
        return -1
    return func_volatile_cheat_count(text, func)


# ---------------------------------------------------------------------------
# Project-wide audit (for the BEFORE/AFTER report)

def audit_project(src_dir: str = "src") -> dict:
    """Walk src/*.c and report every detected volatile cheat. Returns a
    structured dict keyed by file -> [{kind, name, line, decl}] and a
    function-level rollup (func -> count)."""
    import re as _re

    by_file: dict[str, list[dict]] = {}
    by_func: dict[str, int] = {}
    # All .c files
    for p in sorted(Path(src_dir).glob("*.c")):
        text = p.read_text()
        cheats_ = find_all_cheats(text)
        if not cheats_:
            continue
        # Map char-position -> line number
        line_starts = [0]
        for i, c in enumerate(text):
            if c == "\n":
                line_starts.append(i + 1)
        def line_of(pos: int) -> int:
            # Binary search
            import bisect as _bs
            return _bs.bisect_right(line_starts, pos)

        for s, e, vp in cheats_:
            line = line_of(s)
            # Categorize by which detector caught it
            # (re-test the spans)
            slice_ = text[s:min(e + 4, len(text))]
            kind = "unknown"
            for m in _ALIAS_RENAME_RE.finditer(text, s, e + 1):
                if m.start() == s:
                    kind = "alias_rename"
                    break
            if kind == "unknown":
                for m in _GLOBAL_CAST_RE.finditer(text, s, e + 1):
                    if m.start() == s:
                        kind = "global_cast"
                        break
            if kind == "unknown":
                for m in _PLAIN_VOLATILE_RE.finditer(text, s, e + 1):
                    if m.start() == s:
                        kind = "plain_extern"
                        break
            # Extract name + decl snippet
            name = ""
            for m in _re.finditer(r"\b(\w+)\b", text[s:e]):
                tok = m.group(1)
                if tok not in ("extern", "volatile", "asm",
                               "s8", "u8", "s16", "u16", "s32", "u32",
                               "char", "short", "int", "long",
                               "void", "static", "const"):
                    name = tok
                    break
            decl_snip = text[s:min(e, len(text))].replace("\n", " ").strip()
            by_file.setdefault(str(p), []).append({
                "kind": kind, "line": line, "name": name, "decl": decl_snip,
            })
    return {"by_file": by_file,
            "files": sorted(by_file.keys()),
            "total_cheats": sum(len(v) for v in by_file.values())}


if __name__ == "__main__":
    import json as _j
    print(_j.dumps(audit_project(), indent=2))
