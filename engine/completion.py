"""Source checks shared by completion, queue regeneration and integrity audits.

A whole original-assembly body may use the canonical function grant. A C body
must instead match the reviewed assembly islands, and all remaining C is still
checked. Region hashes include operands/constraints, not just opcode strings.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re

from . import cheats, inlineasm

REGIONS = Path('tools/canonical_asm_regions.json')
ASM = re.compile(r'\b(?:__asm__|__asm|asm)\s*(?:(?:volatile|__volatile__)\s*)?\(')


def blocks(text: str, func: str) -> list[tuple[int, int]]:
    span = inlineasm._func_body_span(text, func)
    if span is None:
        return []
    clean = inlineasm._code_without_comments_and_strings(text)
    result = []
    for match in ASM.finditer(clean, *span):
        end = inlineasm._match_paren(text, match.end() - 1)
        if end < 0:
            raise ValueError(f'{func}: unclosed assembly block')
        result.append((match.start(), end + 1))
    return result


def region_hashes(text: str, func: str) -> list[str]:
    return [hashlib.sha256(text[s:e].encode('utf-8')).hexdigest()
            for s, e in blocks(text, func)]


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
    except ValueError as exc:
        return [str(exc)]
    if islands and not canonical:
        return ['inline assembly requires canonical/mixed classification']
    if canonical:
        if not islands:
            return ['canonical C body has no assembly island to authorize']
        try:
            grants = json.loads(REGIONS.read_text(encoding='utf-8'))
            if grants.get('schema') != 1:
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
