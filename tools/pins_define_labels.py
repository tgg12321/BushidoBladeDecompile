#!/usr/bin/env python3
"""Define undefined pins as data labels where the data asm already holds them.

pins-define, first slice: a pin that tools/pins_audit.py classifies `b` (no
linked object defines the name) whose address is the first byte of a data line
in a linked splat data file (asm/data/<off>.data.s, linked by bb2.ld) or an
asm/rodata/<name>.s pulled in by an INCLUDE_RODATA in src/ gets a
`dlabel <name>` line right before that data line. The label is zero-size and
emits no bytes; the object then defines the name at the pinned address, and
tools/pins_drop.py (run next, after a build) removes the pin.

Deterministic and re-runnable (Lane-Replay):
    python3 tools/pins_define_labels.py && make build/bb2.elf >/dev/null \
        && python3 tools/pins_drop.py
It runs make itself first (refresh_build), so it never classifies against a
stale object. A second run finds nothing to label.
"""
import glob
import os
import re
import subprocess
import sys
from collections import defaultdict

sys.path.insert(0, "tools")
import pins_audit  # noqa: E402

VADDR = re.compile(r"^\s*/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\b")
DLABEL = re.compile(r"^\s*dlabel\s+(\w+)\s*$", re.M)
INCLUDE_RODATA = re.compile(r'^\s*INCLUDE_RODATA\(\s*"asm/rodata"\s*,\s*(\w+)\s*\)', re.M)


def data_files():
    # the linked objects as bb2.ld names them (tracked; no stale map needed)
    with open("bb2.ld", encoding="utf-8") as f:
        linked = set(re.findall(r"build/[\w./-]+\.o", f.read()))
    out = [s for s in sorted(glob.glob("asm/data/*.data.s"))
           if "build/" + s[:-2] + ".o" in linked]
    includer = {}  # asm/rodata file -> the linked C file that includes it
    for c in sorted(glob.glob("src/**/*.c", recursive=True)):
        if "build/" + c[:-2] + ".o" in linked:
            with open(c, encoding="utf-8") as f:
                for n in INCLUDE_RODATA.findall(f.read()):
                    includer.setdefault("asm/rodata/%s.s" % n, c)
    out += sorted(includer)
    return out, includer


def refresh_build(includer):
    """make does not track INCLUDE_RODATA: rebuild a TU whose included rodata
    asm is newer than its object, then relink, so the classification below
    reads the objects this tree builds."""
    for s, c in includer.items():
        o = "build/" + c[:-2] + ".o"
        if not os.path.exists(o) or os.path.getmtime(s) > os.path.getmtime(o):
            os.utime(c)
    subprocess.run(["make", "build/bb2.elf"], check=True, stdout=subprocess.DEVNULL)


def main():
    files, includer = data_files()
    refresh_build(includer)
    rows, _ = pins_audit.classify()
    want = defaultdict(set)  # address -> names
    for r in rows:
        if r[3] == "b":
            want[int(r[1], 16)].add(r[0])
    labelled = set()  # names a data file already labels (its rebuild may be pending)
    for fn in files:
        with open(fn, encoding="utf-8") as f:
            labelled.update(DLABEL.findall(f.read()))
    for a in list(want):
        want[a] -= labelled
        if not want[a]:
            del want[a]
    added = 0
    for fn in files:
        with open(fn, encoding="utf-8") as f:
            lines = f.read().split("\n")
        out = []
        seen = set()
        for line in lines:
            m = VADDR.match(line)
            a = int(m.group(1), 16) if m else None
            if a is not None and a in want and a not in seen:
                seen.add(a)
                for n in sorted(want[a]):
                    out.append("dlabel %s" % n)
                    added += 1
            out.append(line)
        if seen:
            with open(fn, "w", encoding="utf-8", newline="\n") as f:
                f.write("\n".join(out))
            if fn in includer:
                # make does not track INCLUDE_RODATA: rebuild the including TU
                os.utime(includer[fn])
            for a in seen:
                del want[a]
    print("pins_define_labels: %d labels added" % added)


if __name__ == "__main__":
    main()
