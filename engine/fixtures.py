"""Golden fixtures — pure-C, zero-cheat functions used as a tool-health gate.

A fixture pins the exact bytes a known-good function compiles to. The (Phase 3)
contract test rebuilds the containing file with the clean driver and confirms
the function's bytes in the final binary still hash to `text_sha1`. If cc1 /
maspsx / the pipeline ever regress, a fixture goes red in seconds — that's how
we catch "a tool broke and nobody noticed".

Each fixture records: {name, file, vram, size, text_sha1}.

Selection rule (Phase 0): the function must have NO regfix/regfix_stage2/asmfix
rule (rules are keyed by source name — verified) and its file must contain no
inline __asm__. Phase 1's cheat-invisible sandbox will let us auto-prove
cheat-freeness; until then this is the cross-check.
"""
from __future__ import annotations

import json
import re
import subprocess
from pathlib import Path

from . import buildconfig as cfg
from . import pipeline as P
from . import oracle as O

# objdump -t line for a function:  VRAM <flags> F <section> SIZE NAME
_FUNC_RE = re.compile(r"^([0-9a-fA-F]+)\s+\S.*\sF\s+(\S+)\s+([0-9a-fA-F]+)\s+(\S+)\s*$")


def _func_table() -> dict[str, tuple[int, int]]:
    """name -> (vram, size) for every function in build/bb2.elf.

    Size is taken from the symbol table when present, but recomputed from the
    gap to the next function address when the symbol size is 0 (maspsx output
    does not always emit .size directives) so we never trust a bogus 0.
    """
    out = P.sh(f"{cfg.OBJDUMP} -t build/bb2.elf", capture_output=True, text=True).stdout
    funcs = {}
    for line in out.splitlines():
        m = _FUNC_RE.match(line)
        if m:
            vram = int(m.group(1), 16)
            size = int(m.group(3), 16)
            name = m.group(4)
            funcs[name] = (vram, size)
    # recompute zero sizes from address gaps
    by_addr = sorted({v for v, _ in funcs.values()})
    nxt = {a: by_addr[i + 1] for i, a in enumerate(by_addr[:-1])}
    fixed = {}
    for name, (vram, size) in funcs.items():
        if size == 0 and vram in nxt:
            size = nxt[vram] - vram
        fixed[name] = (vram, size)
    return fixed


def _file_index() -> dict[str, str]:
    """function name -> source file stem, derived from each object's symbols."""
    idx = {}
    for o in sorted(Path("build/src").glob("*.o")):
        out = P.sh(f"{cfg.NM} {o}", capture_output=True, text=True).stdout
        for line in out.splitlines():
            parts = line.split()
            if len(parts) == 3 and parts[1] in ("t", "T"):
                idx[parts[2]] = o.stem
    return idx


def _func_bytes(vram: int, size: int) -> bytes:
    off = cfg.HEADER_SIZE + (vram - cfg.LOAD_ADDR)
    data = Path("build/bb2.exe").read_bytes()
    return data[off:off + size]


def _sha1_bytes(b: bytes) -> str:
    import hashlib
    return hashlib.sha1(b).hexdigest()


def add(names: list[str]) -> list[dict]:
    """Record the named functions as golden fixtures in the oracle manifest.
    Requires a current matching build (uses build/bb2.elf + build/bb2.exe).
    """
    table = _func_table()
    files = _file_index()
    man = json.loads(O.MANIFEST.read_text())
    existing = {f["name"]: f for f in man.get("golden_fixtures", [])}
    for name in names:
        if name not in table:
            raise KeyError(f"{name}: not a function in build/bb2.elf")
        vram, size = table[name]
        existing[name] = {
            "name": name,
            "file": f"src/{files.get(name, '?')}.c",
            "vram": f"0x{vram:08X}",
            "size": size,
            "text_sha1": _sha1_bytes(_func_bytes(vram, size)),
        }
    man["golden_fixtures"] = [existing[k] for k in sorted(existing)]
    O.MANIFEST.write_text(json.dumps(man, indent=2) + "\n")
    return man["golden_fixtures"]


def verify() -> list[tuple]:
    """Re-check every fixture's bytes in the CURRENT build/bb2.exe against the
    recorded hash. (Phase 3 will rebuild the file first; Phase 0 checks the
    standing binary.) Returns (name, status) with status OK / MISMATCH / MISSING.
    """
    man = json.loads(O.MANIFEST.read_text())
    results = []
    for fx in man.get("golden_fixtures", []):
        vram = int(fx["vram"], 16)
        got = _sha1_bytes(_func_bytes(vram, fx["size"]))
        results.append((fx["name"], "OK" if got == fx["text_sha1"] else "MISMATCH"))
    return results
