#!/usr/bin/env python3
"""sync_undefined_syms.py — keep undefined_syms_auto.txt populated with every
D_HHHHHHHH (and named) symbol that C source files reference.

Why:
    splat emits `<sym>.NON_MATCHING` placeholders (size 1) for any data symbol
    a C file references but that has no asm-side dlabel/.global. When .text
    grows or shrinks, those placeholders drift relative to neighbouring data,
    so the C-side reference resolves to the wrong address.

    Pinning the symbol with `<sym> = 0x<addr>;` in a `-T` linker script forces
    GNU ld to emit it as an *ABS* symbol at exactly that address, regardless of
    where the .NON_MATCHING placeholder ends up. The placeholder stops being
    load-bearing — only the pin matters.

What this does:
    1. Scan src/**/*.c for `extern ... <name> ...;` declarations.
    2. For each extern whose name encodes an address (D_HHHHHHHH), make sure
       `<name> = 0x<addr>;` exists in undefined_syms_auto.txt.
    3. For named externs (e.g. StatusUpBuf), look the address up in
       symbol_addrs.txt or named_syms.txt; pin if found.

The file is appended to so existing manual entries are preserved. Run after
adding new extern declarations or after splat regenerates auto files.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "src"
PIN_FILE = ROOT / "undefined_syms_auto.txt"
SYMBOL_ADDRS = ROOT / "symbol_addrs.txt"
NAMED_SYMS = ROOT / "named_syms.txt"

EXTERN_RE = re.compile(
    r"\bextern\b[^;{]*?\b([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^]]*\])?\s*(?:\([^)]*\))?\s*;",
    re.MULTILINE,
)
ADDR_NAME_RE = re.compile(r"^D_([0-9A-F]{8})$")
PIN_LINE_RE = re.compile(r"^([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;\s*$")


def load_existing_pins(path: Path) -> dict[str, int]:
    pins: dict[str, int] = {}
    if not path.exists():
        return pins
    for line in path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("/*") or line.startswith("//"):
            continue
        m = PIN_LINE_RE.match(line)
        if m:
            pins[m.group(1)] = int(m.group(2), 16)
    return pins


def load_symbol_addrs() -> dict[str, int]:
    addrs: dict[str, int] = {}
    for source in (SYMBOL_ADDRS, NAMED_SYMS):
        if not source.exists():
            continue
        for line in source.read_text().splitlines():
            line = line.strip()
            if not line or line.startswith("//") or line.startswith("#"):
                continue
            m = re.match(r"^([A-Za-z_][A-Za-z0-9_]*)\s*=\s*0x([0-9A-Fa-f]+)\s*;", line)
            if m:
                addrs[m.group(1)] = int(m.group(2), 16)
    return addrs


def collect_extern_names(files: Iterable[Path]) -> set[str]:
    names: set[str] = set()
    for path in files:
        text = path.read_text(errors="replace")
        # strip line comments and block comments before regex
        text = re.sub(r"//[^\n]*", "", text)
        text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
        for m in EXTERN_RE.finditer(text):
            names.add(m.group(1))
    return names


def resolve(name: str, named: dict[str, int]) -> int | None:
    m = ADDR_NAME_RE.match(name)
    if m:
        return int(m.group(1), 16)
    return named.get(name)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--dry-run", action="store_true",
                    help="report what would change but do not modify files")
    ap.add_argument("--show-unresolved", action="store_true",
                    help="list extern names that have no known address")
    args = ap.parse_args()

    src_files = sorted(SRC.rglob("*.c")) + sorted(SRC.rglob("*.h"))
    extern_names = collect_extern_names(src_files)
    pins = load_existing_pins(PIN_FILE)
    named = load_symbol_addrs()

    additions: list[tuple[str, int]] = []
    unresolved: list[str] = []
    for name in sorted(extern_names):
        if name in pins:
            continue
        addr = resolve(name, named)
        if addr is None:
            unresolved.append(name)
            continue
        additions.append((name, addr))

    if args.show_unresolved and unresolved:
        print(f"Unresolved externs ({len(unresolved)}):")
        for name in unresolved:
            print(f"    {name}")

    if not additions:
        print(f"No new pins needed. {len(pins)} already in {PIN_FILE.name}.")
        return 0

    print(f"Adding {len(additions)} pins to {PIN_FILE.name}:")
    for name, addr in additions[:20]:
        print(f"    {name} = 0x{addr:08X};")
    if len(additions) > 20:
        print(f"    ... and {len(additions) - 20} more")

    if args.dry_run:
        print("(dry-run; no file changes)")
        return 0

    with PIN_FILE.open("a", encoding="utf-8", newline="\n") as fh:
        for name, addr in additions:
            fh.write(f"{name} = 0x{addr:08X};\n")
    print(f"Wrote {len(additions)} pins.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
