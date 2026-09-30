"""Emit PsyQ inline_o.h macros as verbatim inline statements (text from the local PsyQ 4.0
INLINE_O.H, identical to the pinned 4.3 text except $PSLibId per engine/gtemacro.py).

Library use: from verbatim import emit ; emit("gte_ldv0", ["ptr"], indent)
DMPSX: gte_rtv0 etc. are emitted with the post-DMPSX word from engine.gtemacro.DMPSX_WORDS.
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, ".")
from engine.gtemacro import DMPSX_WORDS  # noqa: E402

_HDR = Path("tmp/libscan/psyq40/INCLUDE/INLINE_O.H").read_bytes().decode("latin-1").replace("\r\n", "\n")
_LINES = _HDR.split("\n")
MACROS = {}
i = 0
while i < len(_LINES):
    m = re.match(r"#define (gte_\w+)\(([^)]*)\)\s*\{\s*\\$", _LINES[i])
    if m:
        name, params = m.group(1), [p.strip() for p in m.group(2).split(",") if p.strip()]
        stmts = []
        start = i + 1
        j = i + 1
        while not _LINES[j].startswith("}"):
            s = _LINES[j].rstrip("\\").strip()
            if s:
                stmts.append(s)
            j += 1
        MACROS[name] = (params, stmts, (start, j + 1))
        i = j
    i += 1


def emit(name, args, indent="    ", dmpsx=True):
    params, stmts, (a, b) = MACROS[name]
    assert len(params) == len(args), (name, params, args)
    out = []
    for s in stmts:
        for p, v in zip(params, args):
            s = re.sub(r'"r"\(' + re.escape(p) + r'\)', '"r"(' + v + ')', s)
        if dmpsx:
            mm = re.search(r'\.word (0x[0-9a-fA-F]+)', s)
            if mm and int(mm.group(1), 16) in DMPSX_WORDS:
                s = s.replace(mm.group(0), ".word 0x%08X" % DMPSX_WORDS[int(mm.group(1), 16)])
        out.append(indent + s)
    return "\n".join(out) + "\n"


def header_lines(name):
    return MACROS[name][2]


if __name__ == "__main__":
    for n in sys.argv[1:]:
        print(header_lines(n))
        print(emit(n, [f"ARG{k}" for k in range(len(MACROS[n][0]))]))
