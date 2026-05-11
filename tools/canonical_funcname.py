#!/usr/bin/env python3
"""Resolve a raw function name (e.g. `func_80080014`) to its canonical
name based on `asm/funcs/`.

The work queue, classifier CSV, and active-marker file may carry stale
raw `func_<addr>` names from before a function was renamed-in-asm (e.g.
to `motion_SavePreCalcData_80080014`). When the hook then runs
`dc.sh verify <raw>` on that stale name it can't find the symbol, and
the commit gate refuses to clear even though the build matches.

The address suffix is the durable identity: every BB2 function name
ends in `_<HEX-ADDR>` with the original load address. We use that to
look up the renamed file in `asm/funcs/`.

CLI prints the canonical name (or the raw name unchanged when no
canonical form can be inferred). Exit code 0 always.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
_ADDR_SUFFIX = re.compile(r"_([0-9A-Fa-f]{6,8})$")


def canonicalize(raw: str) -> str:
    """Return the canonical name for `raw` (or `raw` itself).

    Resolution rules, in order:
      1. asm/funcs/<raw>.s exists -> already canonical.
      2. raw ends in _<HEX-ADDR> AND exactly one asm/funcs/*_<ADDR>.s
         exists -> return that file's stem.
      3. Otherwise -> return raw unchanged.
    """
    if not raw:
        return raw
    if (ASM_FUNCS / f"{raw}.s").exists():
        return raw
    m = _ADDR_SUFFIX.search(raw)
    if not m:
        return raw
    addr = m.group(1).lower()
    if not ASM_FUNCS.is_dir():
        return raw
    matches: list[str] = []
    for f in ASM_FUNCS.iterdir():
        if f.suffix != ".s":
            continue
        m2 = _ADDR_SUFFIX.search(f.stem)
        if m2 and m2.group(1).lower() == addr:
            matches.append(f.stem)
    if len(matches) == 1:
        return matches[0]
    return raw


def main(argv: list[str]) -> int:
    if len(argv) != 2:
        print(f"usage: {argv[0]} <func-name>", file=sys.stderr)
        return 2
    print(canonicalize(argv[1]))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
