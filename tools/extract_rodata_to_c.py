#!/usr/bin/env python3
"""Extract a splat-generated asm/data/*.rodata*.s file into C declarations
suitable for a sub-TU `.c` file (the rodata-cleanup-project recipe).

Used 2026-06-09 to retire the 6 sub-TU-pattern blocks in the rodata-cleanup
project. Promoted from `tmp/gen_101C_pre.py` to a tools/ artifact so future
similar projects (e.g. .data block re-attribution) have a starting point.

Usage:
  python3 tools/extract_rodata_to_c.py <asm/data/file.s> > /tmp/out.c

Handles:
  - `.word .L<HEX>` / `.word func_<HEX>` → 0xHEX literals
  - `.word <named_symbol>` → resolves via undefined_syms_auto.txt /
    named_syms.txt / symbol_addrs.txt, or falls back to the .s comment
    column's 4-byte little-endian byte content
  - `.asciz "..."` (single or multi per dlabel) — properly decodes asm
    escape sequences (\\n \\t \\r \\\\ \\" \\0 \\xHH) and re-emits as
    unambiguous C string literals

A leading `.word 0x00000000` (alignment padding before the first dlabel)
is emitted as `static const u32 _bb2_<id>_lead`. Place at the TOP of your
sub-TU .c file so cc1 emits it first in .rodata.

Recipe context: docs/rodata-cleanup-project.md +
memory/project/rodata-cleanup-progress.md. Use after
tools/re_attribute_rodata.py confirms the cascade math.
"""
import re
from pathlib import Path

import sys
target = sys.argv[1] if len(sys.argv) > 1 else "asm/data/101C.rodata_pre.s"
text = Path(target).read_text()

# Load named symbol addresses so we can resolve `.word saTan2LineDraw` etc.
NAMED_SYMS = {}
for syms_file in ("undefined_syms_auto.txt", "named_syms.txt", "symbol_addrs.txt"):
    p = Path(syms_file)
    if not p.exists():
        continue
    for line in p.read_text(errors="replace").splitlines():
        m = re.match(r"^([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)", line)
        if m:
            NAMED_SYMS.setdefault(m.group(1), int(m.group(2), 16))

DLABEL_RE = re.compile(
    r"^dlabel\s+(\S+)\s*\n(.*?)^enddlabel\s+\1",
    re.MULTILINE | re.DOTALL,
)
WORD_RE = re.compile(r"\.word\s+([\w\.]+|0x[0-9A-Fa-f]+)")
ASCIZ_RE = re.compile(r'\.asciz\s+"((?:[^"\\]|\\.)*)"')
ADDR_RE = re.compile(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s")


def fmt_word(w, byte_hint=None):
    """Convert .word arg to a u32 literal.

    Forms handled:
    - .L<HEX>       → 0xHEX (canonical splat-emitted local-label name)
    - 0xN / 0XN     → 0xN (literal)
    - func_<HEX>    → 0xHEX (the address is in the symbol name)
    - <named>       → 0xADDR (lookup in named_syms.txt / similar);
                      if not found, fall back to byte_hint (the 4-byte
                      content from the .s comment column).
    """
    if w.startswith(".L"):
        return f"0x{w[2:]}"
    if w.startswith(("0x", "0X")):
        return w
    if w.startswith("func_") and re.fullmatch(r"func_[0-9A-Fa-f]{8}", w):
        return f"0x{w[5:]}  /* {w} */"
    if w in NAMED_SYMS:
        return f"0x{NAMED_SYMS[w]:08X}  /* {w} */"
    if byte_hint is not None:
        return f"0x{byte_hint:08X}  /* {w} (resolved via bytes) */"
    return f"/* UNRESOLVED: {w} */ 0"


def decode_asciz(content_str):
    """Decode a `.asciz "..."` raw string content (with asm escape sequences)
    into a bytes object PLUS the trailing null byte.

    Handles: \\n \\t \\r \\\\ \\" \\xHH \\0 \\<digit> octal etc.
    """
    out = bytearray()
    i = 0
    while i < len(content_str):
        c = content_str[i]
        if c == "\\" and i + 1 < len(content_str):
            nxt = content_str[i + 1]
            if nxt == "n":
                out.append(0x0A)
                i += 2
            elif nxt == "t":
                out.append(0x09)
                i += 2
            elif nxt == "r":
                out.append(0x0D)
                i += 2
            elif nxt == "\\":
                out.append(0x5C)
                i += 2
            elif nxt == '"':
                out.append(0x22)
                i += 2
            elif nxt == "0":
                out.append(0x00)
                i += 2
            elif nxt == "x" and i + 3 < len(content_str):
                out.append(int(content_str[i + 2 : i + 4], 16))
                i += 4
            else:
                # Unknown escape — preserve as literal backslash + char
                out.append(0x5C)
                i += 1
        else:
            out.append(ord(c) if ord(c) < 256 else ord("?"))
            i += 1
    out.append(0)  # .asciz null terminator
    return bytes(out)


def emit_c_byte_literal(data_bytes, prefix="    "):
    """Emit `data_bytes` as a series of `"..."` literals, one per line (≤32B/line)."""
    out = []
    for i in range(0, len(data_bytes), 32):
        chunk = data_bytes[i : i + 32]
        s = ""
        for b in chunk:
            if b == 0:
                s += r"\0"
            elif b == 0x5C:
                s += r"\\"
            elif b == 0x22:
                s += r"\""
            elif b == 0x0A:
                s += r"\n"
            elif b == 0x09:
                s += r"\t"
            elif b == 0x0D:
                s += r"\r"
            elif 0x20 <= b < 0x7F:
                s += chr(b)
            else:
                s += f"\\x{b:02x}"
        out.append(f'{prefix}"{s}"')
    return "\n".join(out)


print("/* 101C.rodata_pre.s rodata extraction */")
print()

# Find any leading content (before first dlabel)
first_dlabel = DLABEL_RE.search(text)
if first_dlabel:
    leading = text[: first_dlabel.start()]
    leading_words = WORD_RE.findall(leading)
    if leading_words:
        print(f"/* Leading padding ({len(leading_words)} words before first dlabel): */")
        print(f"static const u32 _bb2_101C_pre_lead[{len(leading_words)}] = {{")
        for w in leading_words:
            print(f"    {fmt_word(w)},")
        print("};")
        print()

for m in DLABEL_RE.finditer(text):
    sym = m.group(1)
    body = m.group(2)
    # Address of symbol
    addr_m = ADDR_RE.search(body)
    addr = addr_m.group(1).upper() if addr_m else "?"

    words = WORD_RE.findall(body)
    asciz = ASCIZ_RE.findall(body)

    if asciz and not words:
        # String symbol — extract each .asciz line's address + content separately.
        # Use ASCIZ_RE + ADDR_RE in parallel rather than a fused regex (the fused
        # form had a Python-regex char-class escaping bug).
        items = []
        for line in body.splitlines():
            if ".asciz" not in line:
                continue
            am = ASCIZ_RE.search(line)
            adm = ADDR_RE.search(line)
            if am and adm:
                items.append((int(adm.group(1), 16), am.group(1)))
        if not items:
            print(f"/* {sym}: empty? */")
            continue
        # Compute symbol size by finding the next-thing address
        sym_addr = items[0][0]
        # Last string: pad to next 4-aligned
        # Decode all strings to bytes, concatenate with per-string padding
        out_bytes = bytearray()
        for i, (a, s) in enumerate(items):
            decoded = decode_asciz(s)  # includes trailing null
            # Per-string padding to next 4-aligned offset (asm's .align 2)
            if i + 1 < len(items):
                target_sz = items[i + 1][0] - a
            else:
                target_sz = ((len(decoded) + 3) // 4) * 4
            out_bytes.extend(decoded)
            pad = target_sz - len(decoded)
            if pad > 0:
                out_bytes.extend(b"\x00" * pad)
        total = len(out_bytes)
        print(f"/* {sym}: {len(items)} string(s), {total}B @ 0x{sym_addr:08X} */")
        print(f"const char {sym}[{total}] =")
        print(emit_c_byte_literal(bytes(out_bytes)))
        print("    ;")
        print()
    elif words:
        # Word array (jtbl or data) — also extract per-word byte hints from
        # the `/* OFFSET VRAM HEXBYTES */ .word ...` comments. The HEXBYTES
        # column is the 4-byte little-endian content; when the .word arg is
        # a named symbol that NAMED_SYMS doesn't resolve, fall back to it.
        word_lines = re.findall(
            r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s+\*/\s*\.word\s+([\w\.]+|0x[0-9A-Fa-f]+)",
            body,
        )
        sym_addr = int(word_lines[0][0], 16) if word_lines else 0
        n = len(word_lines) if word_lines else len(words)
        print(f"/* {sym}: {n} words ({n*4}B) @ 0x{sym_addr:08X} */")
        print(f"const u32 {sym}[{n}] = {{")
        for _addr_hex, bytes_hex, w in word_lines:
            # The hex bytes column is the 4-byte LE content. Reverse for u32 value.
            be = bytes.fromhex(bytes_hex)
            le_value = int.from_bytes(be, "little")
            print(f"    {fmt_word(w, byte_hint=le_value)},")
        print("};")
        print()
