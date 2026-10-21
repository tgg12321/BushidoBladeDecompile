#!/usr/bin/env python3
"""Drop the linker-script pins the build already defines (pins-drop).

A pin is droppable when tools/pins_audit.py classifies it `a` with same=Y: a
global, non-COMMON definition in a linked object sits at exactly the pinned
address, so the -T assignment only shadows it and removing it moves nothing.

Each droppable assignment line leaves named_syms.txt / undefined_syms_auto.txt
/ undefined_funcs_auto.txt. The name's address is kept in symbol_addrs.txt
(the registry ld never reads; the engine scorer, layer-2, datamodel and the
naming tools all read it), appended under one marker comment: every
commented pin line, in the order engine/datamodel.py reads the files, so its
census keeps the same notes (a name may then appear more than once, always at
one address); an uncommented line only when the registry lacks the name. docs/foundation/pins_audit.tsv is rewritten for the remaining pins.

Deterministic and re-runnable (Lane-Replay): it reads only the tracked pin
files, build/bb2.map and the linked objects, so build first:
    make build/bb2.elf && python3 tools/pins_drop.py
A second run finds nothing to drop and changes nothing.

With --unreferenced it also drops the `b` pins unreferenced_rule() accepts
(appended under their own marker):
    make build/bb2.elf && python3 tools/pins_drop.py --unreferenced
With --static-shadows it also drops the `al` pins static_shadow_rule() accepts
(appended under their own marker):
    make build/bb2.elf && python3 tools/pins_drop.py --static-shadows
"""
import re
import sys
from collections import defaultdict

sys.path.insert(0, "tools")
import pins_audit  # noqa: E402

REGISTRY = "symbol_addrs.txt"
MARKER = ("// pins-drop: names a linked object defines at this address "
          "(moved out of the linker pin files by tools/pins_drop.py)")
MARKER_UNREF = ("// pins-drop --unreferenced: names no linked object references, at "
                "addresses the link places (moved out of the linker pin files)")
MARKER_STATIC = ("// pins-drop --static-shadows: names only a static at this address "
                 "defines, no linked object references (moved out of the linker pin files)")
MARKERS = (MARKER, MARKER_UNREF, MARKER_STATIC)
SYM_LINE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\s*;")


def unreferenced_rule(rows, map_path="build/bb2.map"):
    """--unreferenced: a `b` pin no linked object references (no UND entry, no
    mention in bb2.ld) whose address lies inside an input section the link
    places (program memory the build lays out, under another name or none).
    The assignment feeds no relocation, so it leaves the linker files like an
    `a` pin does. BSS-range names (no placed section, bss-link) and hardware
    or kernel absolutes keep their pins: new C reaches them only through one.
    A name pinned at more than one address is left alone."""
    objs, spans = pins_audit.map_sections(map_path)
    spans = [s for s in spans if s[3] > 0 and s[0] != "build/asm/header.o"]
    refs = pins_audit.undefined_refs(objs)
    with open("bb2.ld", encoding="utf-8") as f:
        refs.update(re.findall(r"[A-Za-z_]\w*", f.read()))
    values = defaultdict(set)
    for r in rows:
        values[r[0]].add(r[1])

    def placed(a):
        return any(s[2] <= a < s[2] + s[3] for s in spans)

    def rule(row):
        return (row[3] == "b" and row[0] not in refs and len(values[row[0]]) == 1
                and placed(int(row[1], 16)))
    return rule


def static_shadow_rule():
    """--static-shadows: an `al` pin (only a static local of that name exists)
    at the static's own address, that no linked object references (no UND
    entry, no mention in bb2.ld). The static's TU resolves the name to its
    local; nothing resolves to the absolute global the -T assignment adds, so
    removing it moves nothing and the static keeps its address."""
    objs, _ = pins_audit.map_sections("build/bb2.map")
    refs = pins_audit.undefined_refs(objs)
    with open("bb2.ld", encoding="utf-8") as f:
        refs.update(re.findall(r"[A-Za-z_]\w*", f.read()))

    def rule(row):
        return row[3] == "al" and row[9] == "Y" and row[0] not in refs
    return rule


def main():
    unreferenced = "--unreferenced" in sys.argv[1:]
    rows, _ = pins_audit.classify()
    droppable = pins_audit.droppable
    if unreferenced:
        alias = unreferenced_rule(rows)
        droppable = lambda r: pins_audit.droppable(r) or alias(r)  # noqa: E731
    static = "--static-shadows" in sys.argv[1:]
    if static:
        shadow, base = static_shadow_rule(), droppable
        droppable = lambda r: base(r) or shadow(r)  # noqa: E731
    drop = defaultdict(set)  # file -> line numbers
    addr = {}
    for r in rows:
        if droppable(r):
            fn, ln = r[2].rsplit(":", 1)
            drop[fn].add(int(ln))
            addr[r[0]] = r[1]
    # a name is either dropped everywhere or nowhere
    for r in rows:
        if r[0] in addr and not droppable(r):
            sys.exit("pins_drop: %s is droppable in one place but not at %s" % (r[0], r[2]))

    moved = defaultdict(list)  # file -> [(name, trailing text)] in line order
    for fn in pins_audit.PIN_FILES:
        with open(fn, encoding="utf-8") as f:
            raw = f.read()
        blank = pins_audit.blank_comments(raw).split("\n")
        lines = raw.split("\n")
        # lines a comment runs into or out of (a dropped line must hold whole comments)
        spanned = set()
        for m in pins_audit.COMMENT.finditer(raw):
            first = raw.count("\n", 0, m.start()) + 1
            last = first + m.group(0).count("\n")
            if last > first:
                spanned.update(range(first, last + 1))
        keep = []
        for i, line in enumerate(lines, 1):
            if i not in drop[fn]:
                keep.append(line)
                continue
            found = list(pins_audit.ASSIGN.finditer(blank[i - 1]))
            if len(found) != 1 or blank[i - 1][:found[0].start()].strip():
                sys.exit("pins_drop: %s:%d is not one assignment per line" % (fn, i))
            if i in spanned:
                sys.exit("pins_drop: %s:%d holds part of a multi-line comment" % (fn, i))
            if blank[i - 1][found[0].end():].strip():
                sys.exit("pins_drop: %s:%d has text after the assignment" % (fn, i))
            moved[fn].append((found[0].group(1), line[found[0].end():].strip()))
        with open(fn, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join(keep))

    with open(REGISTRY, encoding="utf-8") as f:
        reg = f.read()
    have = {}
    seen = set()
    for ln in reg.split("\n"):
        m = SYM_LINE.match(ln)
        if m:
            have.setdefault(m.group(1), int(m.group(2), 16))
            seen.add(ln.strip())
    # Every commented pin line moves, in the order engine/datamodel.py reads the
    # files (undefined_syms_auto.txt, then named_syms.txt last so its schema
    # comments win), so its address census keeps the same notes. An uncommented
    # line moves only when the registry has no line for the name yet.
    add = []
    for fn in ["undefined_funcs_auto.txt", "undefined_syms_auto.txt", "named_syms.txt"]:
        for name, tail in moved[fn]:
            if name in have and have[name] != int(addr[name], 16):
                sys.exit("pins_drop: %s is %s in %s but pinned at %s"
                         % (name, hex(have[name]), REGISTRY, addr[name]))
            line = "%s = %s;%s" % (name, addr[name], " " + tail if tail else "")
            if line in seen or (not tail and name in have):
                continue
            add.append(line)
            seen.add(line)
            have.setdefault(name, int(addr[name], 16))
    if add:
        marker = MARKER_STATIC if static else MARKER_UNREF if unreferenced else MARKER
        # lines append at the end, so the last marker there must be this mode's
        if reg.rfind(marker) < max(reg.rfind(m) for m in MARKERS) or marker not in reg:
            reg = reg.rstrip("\n") + "\n\n" + marker + "\n"
        reg = reg.rstrip("\n") + "\n" + "\n".join(add) + "\n"
        with open(REGISTRY, "w", encoding="utf-8", newline="\n") as f:
            f.write(reg)

    rows, _ = pins_audit.classify()
    pins_audit.write_table(rows, "docs/foundation/pins_audit.tsv")
    print("pins_drop: dropped %d assignment lines (%d names); %d added to %s; %d pins remain"
          % (sum(len(v) for v in drop.values()), len(addr), len(add), REGISTRY, len(rows)))


if __name__ == "__main__":
    main()
