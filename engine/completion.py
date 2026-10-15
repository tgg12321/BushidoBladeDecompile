"""Source checks shared by completion, queue regeneration and integrity audits.

A whole original-assembly body may use the canonical function grant. A C body
must instead match the reviewed assembly islands, and all remaining C is still
checked. Region hashes include operands/constraints, not just opcode strings.
An island is an asm statement or a call of a vendored PsyQ GTE macro
(engine.gtemacro.call_sites); a call must also pass gtemacro.call_issues.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path

from . import cheats, gtemacro, inlineasm, layer2

REGIONS = Path('tools/canonical_asm_regions.json')
# Schema 2 (owner 2026-10-06): a region hash covers the island's C tokens, so a
# layout-only change (tools/format.py) keeps the grant. Schema 1 hashed raw text.
REGIONS_SCHEMA = 2


# Owner decisions 2026-10-15 (design section 2, flag until the written-out
# islands are gone): refuse a written-out PsyQ macro (gtemacro.unit_spans) in a
# canonical body — "write the macro call" — so the old form cannot come back.
REFUSE_WRITTEN_OUT_UNITS = False


def _islands(text: str, func: str) -> list[tuple[int, int, str | None]]:
    """(start, end, header): every assembly island of func's body — asm
    keyword statements (header None) and calls of vendored PsyQ GTE macros
    (header = the TU's inline header; engine.gtemacro.call_sites) — sorted."""
    span = inlineasm._func_body_span(text, func)
    if span is None:
        return []
    result = []
    # Every asm keyword spelling, found the way cc1 tokenizes (comments and
    # backslash-newlines before the `(` are whitespace).
    for kw in inlineasm.cia.find_asm_keywords(text, inlineasm.cia.ASM_SPELLINGS):
        if not span[0] <= kw.start < span[1]:
            continue
        if kw.paren < 0:
            raise ValueError(f'{func}: assembly keyword without an operand list')
        if kw.end < 0:
            raise ValueError(f'{func}: unclosed assembly block')
        result.append((kw.start, kw.end + 1, None))
    calls = gtemacro.call_sites(text, func)
    if calls:
        header = gtemacro.tu_header(text) or '-'
        result += [(s, e, header) for s, e, _name in calls]
    return sorted(result)


def blocks(text: str, func: str) -> list[tuple[int, int]]:
    """Spans of func's assembly islands: asm statements and vendored macro calls."""
    return [(s, e) for s, e, _h in _islands(text, func)]


def region_hashes(text: str, func: str) -> list[str]:
    """One sha256 per island over its C tokens (layer2.tokens: string literals
    verbatim, whitespace and comments dropped). Any template, operand,
    constraint or clobber change moves the hash; re-indenting does not. A
    macro call hashes as `<inline header>:` + its tokens, so a call's
    arguments, its macro, or the TU's switch between inline_o.h and
    inline_c.h (same names, different bodies) voids the grant."""
    return [hashlib.sha256(((f'{h}:' if h else '') + ' '.join(layer2.tokens(text[s:e])))
                           .encode('utf-8')).hexdigest()
            for s, e, h in _islands(text, func)]


def grant_issues(stems: set[str]) -> list[str]:
    """Every region grant, whatever its function's queue state, still names a
    C TU and a canonical function and matches that body's islands now. A
    queued function never reaches source_issues, so a rename inside one of its
    islands would otherwise leave the grant stale until `queue done`."""
    try:
        grants = json.loads(REGIONS.read_text(encoding='utf-8'))
    except (OSError, ValueError) as exc:
        return [f'{REGIONS}: unreadable ({exc})']
    if not isinstance(grants, dict) or grants.get('schema') != REGIONS_SCHEMA \
            or not isinstance(grants.get('functions'), dict):
        return [f'{REGIONS}: not a schema {REGIONS_SCHEMA} grant registry']
    canon = cheats.canonical_asm_funcs()
    issues = []
    for func, entry in sorted(grants['functions'].items()):
        where = f'{REGIONS}: {func}'
        if not isinstance(entry, dict) or set(entry) != {'file', 'sha256'} \
                or not isinstance(entry['file'], str) \
                or not isinstance(entry['sha256'], list) or not entry['sha256']:
            issues.append(f'{where}: malformed grant')
            continue
        if func not in canon:
            issues.append(f'{where}: granted but not listed in inline_asm_canonical.txt')
        stem = entry['file']
        if stem not in stems:
            issues.append(f'{where}: grant file {stem!r} is not a C TU')
            continue
        text = inlineasm._read_src_cached(stem)
        try:
            got = region_hashes(text, func) if text is not None else None
        except ValueError as exc:
            got = str(exc)
        if got != entry['sha256']:
            issues.append(f'{where} ({stem}.c): assembly islands or operand '
                          f'constraints changed since review (stale grant)')
    return issues


def source_issues(stem: str, func: str, canonical: bool | None = None) -> list[str]:
    text = inlineasm._read_src_cached(stem)
    if text is None:
        return ['source unavailable']
    if canonical is None:
        canonical = func in cheats.canonical_asm_funcs()
    span = inlineasm._func_body_span(text, func)
    if span is None:
        if canonical and func in inlineasm.whole_body_asm_funcs(text):
            return []
        return ['no C body or authorized whole assembly body']
    try:
        islands = blocks(text, func)
        call_problems = gtemacro.call_issues(text, func)
    except ValueError as exc:
        return [str(exc)]
    if islands and not canonical:
        return call_problems + ['inline assembly requires canonical/mixed classification']
    if call_problems:
        return call_problems
    if canonical and REFUSE_WRITTEN_OUT_UNITS:
        units = sorted({m for s, _e, m in gtemacro.unit_spans(text) if span[0] <= s < span[1]})
        if units:
            return [f'written-out PsyQ macro {m}: write the macro call' for m in units]
    if canonical:
        if not islands:
            return ['canonical C body has no assembly island to authorize']
        try:
            grants = json.loads(REGIONS.read_text(encoding='utf-8'))
            if grants.get('schema') != REGIONS_SCHEMA:
                raise KeyError('schema')
            expected = grants['functions'][func]
        except (OSError, ValueError, KeyError, TypeError):
            return ['C/assembly function has no reviewed region grant']
        if expected != {'file': stem, 'sha256': region_hashes(text, func)}:
            return ['assembly islands or operand constraints changed since review']
        for start, end in reversed(islands):
            text = text[:start] + ''.join('\n' if c == '\n' else ' ' for c in text[start:end]) + text[end:]
    count = inlineasm.func_cheat_asm_count(text, func)
    if count < 0:
        return ['source cheat count unknown']
    return [f'{count} cheat construct(s) outside approved assembly'] if count else []
