"""Tier-3 inline-asm stripping for the cheat-invisible sandbox + tier-3 campaign.

Tier-3 (workaround) __asm__ blocks and register-asm pins are injection cheats:
the sandbox strips them so the score reflects pure-C codegen — injecting asm
can't move the score. Tier-2 (GTE/cop2/.word-cop2/BIOS/HW) is canonical and is
KEPT. A mixed block (any tier-2 instruction) is kept whole (canonical GTE
sequences include their feeder loads).

Tier classification reuses tools/classify_inline_asm (single source of truth for
the tier-2/tier-3 line). Stripping a function's tier-3 asm doesn't change OTHER
functions' .text bytes (only file-scope label numbers shift), so a file-wide
strip + masked scoring triages every tier-3 function in one build.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import classify_inline_asm as cia  # noqa: E402

_PIN_QUALIFIER = re.compile(r'\basm\s*\(\s*"[^"]+"\s*\)')


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


def _block_tier(asm_body: str) -> str:
    """tier of an __asm__(...) body, mirroring classify_inline_asm.scan_file."""
    first_colon = asm_body.find(":")
    tmpl_body = asm_body if first_colon == -1 else asm_body[:first_colon]
    tmpl_strs = cia.extract_strings(tmpl_body)
    if any(cia.GLABEL_HINT_RE.search(s) for s in tmpl_strs):
        return "tier1"  # canonical-body glabel block — never strip
    instrs = []
    for t in tmpl_strs:
        instrs.extend(cia.split_template(t))
    if not instrs:
        return "tier3" if cia.EMPTY_ASM_IS_TIER3 else "tier2"
    return "tier2" if any(cia.classify_template(x) == "tier2" for x in instrs) else "tier3"


def _strip_spans(text: str) -> list[tuple[int, int]]:
    """Char spans to delete: tier-3 __asm__ statements + the asm("$N") qualifier
    of register pins (the variable declaration itself is kept)."""
    spans = []
    for m in cia.ASM_KEYWORD_RE.finditer(text):
        line_start = text.rfind("\n", 0, m.start()) + 1
        if text[line_start:m.start()].lstrip().startswith("#"):
            continue  # macro definition — leave (stripping would break uses)
        paren_open = m.end() - 1
        close = _match_paren(text, paren_open)
        if close < 0:
            continue
        if _block_tier(text[paren_open + 1:close - 1]) != "tier3":
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


def strip_tier3_file(text: str) -> tuple[str, int]:
    """Return (modified text, count) with all tier-3 asm blocks + pins removed."""
    spans = sorted(_strip_spans(text), reverse=True)
    for s, e in spans:
        text = text[:s] + text[e:]
    return text, len(spans)


def write_stripped(stem: str, out_path: str) -> int:
    """Write src/<stem>.c with tier-3 asm stripped to out_path. Returns count."""
    text = Path(f"src/{stem}.c").read_text()
    stripped, n = strip_tier3_file(text)
    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    Path(out_path).write_text(stripped)
    return n
