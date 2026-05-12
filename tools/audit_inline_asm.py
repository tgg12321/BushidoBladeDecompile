#!/usr/bin/env python3
"""Audit C functions whose body is inline asm.

Per community standard (SOTN, MGS, Silent Hill): inline `__asm__` /
`asm volatile` is acceptable ONLY for:
  * GTE coprocessor-2 ops (mfc2/cfc2/mtc2/ctc2/lwc2/swc2)
  * BIOS calls (syscall, break)
  * Scratchpad reads/writes that need %0 register placeholders
  * Data embedding (.incbin)

It is NOT acceptable as a body wrapper for hand-tuned scheduling.
Functions that wrap their entire body in `asm volatile` are
"fake-matched" — they should be marked undecompiled (use
`replace_with_asmfile` in asmfix.txt) until properly decompiled in
pure C.

Output: list of offenders with line counts, organized by file.
"""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"

# Patterns that indicate ACCEPTABLE inline asm (whitelisted).
#
# GTE ops encoded as `.word 0xXX......` are the canonical form in BB2
# when the assembler can't parse the symbolic mnemonic (most BB2
# wrappers use this). Acceptable opcode-byte prefixes:
#   0x48-0x4B  → COP2 family (mfc2/cfc2/mtc2/ctc2/avsz/gpf/gpl/mvmva/...)
#   0xC8-0xCB  → lwc2
#   0xE8-0xEB  → swc2
ACCEPTABLE_OPS = re.compile(
    r"(\b("
    r"mfc2|cfc2|mtc2|ctc2|lwc2|swc2|"  # GTE cop2 (symbolic)
    r"syscall|break|"  # BIOS / trap
    r"move|"  # pseudo-mnemonic for `addu rd, rs, $zero` placement-control
    r"\.incbin|\.section\s+\.data|\.pushsection|\.popsection"  # data
    r")\b)"
    r"|"
    r"\.word\s+0x(4[89A-Ba-b]|[Cc][89A-Ba-b]|[Ee][89A-Ba-b])[0-9A-Fa-f]{6}\b"
)

# Patterns that indicate UNACCEPTABLE body-wrapper inline asm.
SUSPECT_OPS = re.compile(
    r"\b("
    r"multu|mult|mflo|mfhi|"  # arithmetic
    r"addu|subu|sll|sra|srl|"  # alu (when many in a row)
    r"lh|lhu|lb|lbu|lw|sh|sb|sw|"  # memory
    r"bgez|blez|bne|beq|bltz|bgtz|j\s|jal\s|jr"  # control
    r")\b"
)


def find_function_bounds(text: str, name_match) -> tuple[int, int] | None:
    """Given a regex match for a function definition (regex ends at the `{`),
    return (body_start_inclusive, body_end_exclusive) for matching brace.

    Functions whose body is a single `__asm__ volatile(...)` block contain
    no inner braces, so we must NOT search for a second `{` — start counting
    from the brace at name_match.end()-1."""
    n = len(text)
    if name_match.end() == 0 or text[name_match.end() - 1] != "{":
        return None
    body_start = name_match.end()  # just after `{`
    depth = 1
    i = body_start
    while i < n and depth > 0:
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
        i += 1
    if depth != 0:
        return None
    return body_start, i - 1  # i-1 is the `}`


def main() -> None:
    offenders: list[dict] = []
    inline_asm_re = re.compile(r"\b__asm__\s*(?:volatile)?\s*\(")
    # Match a function definition at start-of-line. Type-prefix is one or
    # more tokens (word/space/*), the function name is the LAST identifier
    # immediately before `(`. Capture the name via a tail anchor.
    func_def_re = re.compile(
        r"(?m)^[A-Za-z_][\w *]*?\s\**(\w+)\s*\([^)]*\)\s*\{",
    )

    for c in sorted(SRC.glob("*.c")):
        text = c.read_text(encoding="utf-8")
        for fm in func_def_re.finditer(text):
            name = fm.group(1)
            if name in ("if", "while", "for", "switch", "do"):
                continue  # paranoia
            bounds = find_function_bounds(text, fm)
            if not bounds:
                continue
            body_start, body_end = bounds
            body = text[body_start:body_end]
            asm_matches = list(inline_asm_re.finditer(body))
            if not asm_matches:
                continue
            # Examine each asm block's content
            classes = set()
            asm_chars = 0
            asm_count = 0
            for am in asm_matches:
                # Find matching close-paren for the asm( )
                depth = 0
                j = am.end() - 1  # at the open paren
                while j < len(body):
                    if body[j] == "(":
                        depth += 1
                    elif body[j] == ")":
                        depth -= 1
                        if depth == 0:
                            asm_body = body[am.end() : j]
                            asm_chars += len(asm_body)
                            asm_count += 1
                            if ACCEPTABLE_OPS.search(asm_body):
                                classes.add("acceptable")
                            if SUSPECT_OPS.search(asm_body):
                                classes.add("suspect")
                            break
                    j += 1
            body_chars = len(body)
            asm_ratio = asm_chars / body_chars if body_chars else 0

            verdict = "OK"
            if "suspect" in classes and "acceptable" not in classes:
                verdict = "OFFENDER"  # only suspect ops, no GTE/BIOS justification
            elif "suspect" in classes and asm_ratio > 0.5:
                verdict = "OFFENDER (mixed; asm-dominant)"
            elif "acceptable" in classes:
                verdict = "ok (GTE/BIOS)"

            offenders.append({
                "file": c.name,
                "func": name,
                "asm_blocks": asm_count,
                "asm_chars": asm_chars,
                "body_chars": body_chars,
                "asm_ratio": asm_ratio,
                "verdict": verdict,
                "line": text[: fm.start()].count("\n") + 1,
            })

    # Print offenders by file
    by_verdict: dict[str, list] = {}
    for o in offenders:
        by_verdict.setdefault(o["verdict"], []).append(o)

    for verdict in sorted(by_verdict.keys(), key=lambda v: 0 if "OFFENDER" in v else 1):
        items = by_verdict[verdict]
        print(f"=== {verdict}: {len(items)} ===")
        for o in sorted(items, key=lambda x: (x["file"], x["line"])):
            print(f"  {o['file']:24s} {o['func']:30s}  "
                  f"asm={o['asm_chars']:5d}/{o['body_chars']:5d} "
                  f"({o['asm_ratio']*100:.0f}%)  blocks={o['asm_blocks']}")
        print()


if __name__ == "__main__":
    main()
