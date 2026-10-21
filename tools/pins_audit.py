#!/usr/bin/env python3
"""Classify every linker-script address pin against the linked objects.

A pin is a `name = 0xADDR;` assignment in named_syms.txt, undefined_syms_auto.txt
or undefined_funcs_auto.txt (all passed to ld with -T, so an assignment wins
over an object definition of the same global name).

Each pin gets a class:
  a  shadows a definition in a linked build/**/*.o (global, weak or common)
  al the name exists only as a local (static) symbol in some linked object;
     the pin is what other objects resolve to, the local is a separate symbol
  b  no definition anywhere in the linked objects

For class a/al the object's own address (section placement from the map +
symbol offset) is computed, and `same` says whether it equals the pin value.
a+same=Y is the set pins-drop can remove without moving any symbol.

Run in WSL at the repo (or worktree) root after a build:
    python3 tools/pins_audit.py [--out docs/foundation/pins_audit.tsv]
Reads build/bb2.map (LOAD lines = linked objects) and the objects' symtabs.
"""
import argparse
import re
import subprocess
import sys
from collections import defaultdict

PIN_FILES = ["named_syms.txt", "undefined_syms_auto.txt", "undefined_funcs_auto.txt"]
ASSIGN = re.compile(r"([A-Za-z_.$][A-Za-z0-9_.$]*)\s*=\s*(0x[0-9A-Fa-f]+|\d+)\s*;")
COMMENT = re.compile(r"/\*.*?\*/", re.S)
READELF = "mipsel-linux-gnu-readelf"


def blank_comments(text):
    # keep newlines so line numbers survive
    return COMMENT.sub(lambda m: re.sub(r"[^\n]", " ", m.group(0)), text)


def read_pins():
    pins = []
    for fn in PIN_FILES:
        with open(fn, encoding="utf-8") as f:
            text = blank_comments(f.read())
        for ln, line in enumerate(text.split("\n"), 1):
            for m in ASSIGN.finditer(line):
                pins.append((m.group(1), int(m.group(2), 0), fn, ln))
    return pins


def read_map(path):
    """Linked objects and input-section addresses: {(obj, section): addr}."""
    objs, place = [], {}
    pending = None
    with open(path, encoding="utf-8", errors="replace") as f:
        lines = f.read().split("\n")
    in_memmap = False
    for line in lines:
        if line.startswith("LOAD "):
            o = line[5:].strip()
            if o.endswith(".o"):
                objs.append(o)
            continue
        if line.startswith("Linker script and memory map"):
            in_memmap = True
            continue
        if not in_memmap:
            continue
        m = re.match(r"^ (\.\S+)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+\.o)\s*$", line)
        if m:
            place.setdefault((m.group(4), m.group(1)), int(m.group(2), 16))
            pending = None
            continue
        m = re.match(r"^ (\.\S+)\s*$", line)
        if m:
            pending = m.group(1)
            continue
        if pending:
            m = re.match(r"^\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(\S+\.o)\s*$", line)
            if m:
                place.setdefault((m.group(3), pending), int(m.group(1), 16))
            pending = None
    return objs, place


def object_symbols(obj):
    """[(name, value, size, type, bind, section_name)] for defined symbols."""
    out = subprocess.run([READELF, "-SsW", obj], capture_output=True, text=True, check=True).stdout
    secs = {}
    syms = []
    for line in out.split("\n"):
        m = re.match(r"^\s*\[\s*(\d+)\]\s+(\S+)\s+\S+\s+[0-9a-f]+", line)
        if m:
            secs[m.group(1)] = m.group(2)
            continue
        m = re.match(r"^\s*\d+:\s+([0-9a-f]+)\s+(\d+)\s+(\S+)\s+(\S+)\s+\S+\s+(\S+)\s+(\S+)\s*$", line)
        if not m:
            continue
        val, size, typ, bind, ndx, name = m.groups()
        if ndx == "UND" or typ in ("SECTION", "FILE"):
            continue
        sec = "*COM*" if ndx == "COM" else "*ABS*" if ndx == "ABS" else secs.get(ndx, ndx)
        syms.append((name, int(val, 16), int(size), typ, bind, sec))
    return syms


def obj_addr(d, place):
    o, val, typ, bind, sec = d
    if sec == "*ABS*":
        return val
    base = place.get((o, sec))
    return None if base is None else base + val


def classify(map_path="build/bb2.map"):
    """One row per pin assignment (columns as in the table), and the number of
    linked objects."""
    pins = read_pins()
    objs, place = read_map(map_path)
    if not objs:
        sys.exit("pins_audit: no LOAD lines in " + map_path)

    defs = defaultdict(list)  # name -> [(obj, value, type, bind, sec)]
    for o in objs:
        for name, val, size, typ, bind, sec in object_symbols(o):
            defs[name].append((o, val, typ, bind, sec))

    names = defaultdict(int)
    for p in pins:
        names[p[0]] += 1
    rows = []
    for name, addr, fn, ln in pins:
        ds = defs.get(name, [])
        glob = [d for d in ds if d[3] in ("GLOBAL", "WEAK")]
        # a real definition outranks a tentative (COMMON) one, as it does in ld
        pick = sorted(glob or ds, key=lambda d: d[4] == "*COM*")
        cls = "a" if glob else "al" if ds else "b"
        obj = typ = bind = sec = natural = same = ""
        if pick:
            o, val, typ, bind, sec = pick[0]
            obj = o
            nats = [obj_addr(d, place) for d in pick if d[4] != "*COM*"]
            nat = nats[0] if nats else None
            natural = "" if nat is None else "0x%08X" % nat
            if not nats or None in nats:
                same = "?"
            else:
                same = "Y" if all(n == addr for n in nats) else "N"
            if len(pick) > 1:
                obj += " (+%d)" % (len(pick) - 1)
        rows.append([name, "0x%08X" % addr, "%s:%d" % (fn, ln), cls, obj, sec, typ, bind,
                     natural, same, "dup" if names[name] > 1 else ""])
    return rows, len(objs)


def droppable(row):
    """A pin the build defines at the same address: removing it moves nothing."""
    return row[3] == "a" and row[9] == "Y"


def write_table(rows, out):
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("# generated by tools/pins_audit.py -- do not hand-edit\n")
        f.write("# class: a = shadows a global/weak/common definition in a linked object; "
                "al = only a local of that name exists; b = no definition\n")
        f.write("# same: object's own address == pin value (Y/N, ? = common or unplaced)\n")
        f.write("\t".join(["name", "pin", "where", "class", "def_obj", "def_sec", "type",
                           "bind", "obj_addr", "same", "dup"]) + "\n")
        for r in rows:
            f.write("\t".join(r) + "\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--map", default="build/bb2.map")
    ap.add_argument("--out", default="docs/foundation/pins_audit.tsv")
    args = ap.parse_args()
    rows, nobjs = classify(args.map)
    write_table(rows, args.out)
    count = defaultdict(int)
    for r in rows:
        count[(r[3], r[9])] += 1
    print("pins_audit: %d pins, %d linked objects -> %s" % (len(rows), nobjs, args.out))
    for k in sorted(count):
        print("  class=%-2s same=%-1s  %d" % (k[0], k[1] or "-", count[k]))
    uniq = defaultdict(set)
    for r in rows:
        uniq[r[3]].add(r[0])
    print("  unique names: " + ", ".join("%s=%d" % (c, len(uniq[c])) for c in sorted(uniq)))


if __name__ == "__main__":
    main()
