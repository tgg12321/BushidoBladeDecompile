#!/usr/bin/env python3
"""Define every BSS-range pin as a label in a linked .bss object (bss-link).

crt0 (asm/funcs/__SN_ENTRY_POINT.s) zeroes D_800A3308..BSS_END_NAME; the part
past .main (BSS_START up) is .bss, linked as the NOLOAD .main_bss section of
bb2.ld. No game or library object has a non-empty .bss yet, so each name the
code uses there was only a linker-script pin. This script writes OUT: one
`dlabel` per name, in address order, with `.space` gaps between them up to
BSS_END_NAME, and links it last in .main_bss. A COMMON of the same name in a
C object merges into the label. tools/pins_drop.py then finds each pin
shadowing a definition at its own address (class a, same=Y) and moves it to
symbol_addrs.txt, so the labels are the only definitions left.

Names come from the BSS-range pins plus the labels OUT already holds, so a
second run (no pins left) writes the same file. A pinned name a linked object
already defines outside COMMON is left to that object; a label already in OUT
stays until it is removed by hand (do that when its variable moves into a C
object's .bss, which also shifts every label after it). A name pinned at two
addresses, or labelled at one and pinned at another, stops the script.

Each address group starts with a `/* ADDR */` line, then bare `dlabel NAME`
lines, the form other label scanners (data_wave.py, engine/score.py) read.

Deterministic and re-runnable (Lane-Replay), in WSL at the repo root:
    make build/bb2.elf && python3 tools/bss_link.py && make build/bb2.elf \\
        && python3 tools/pins_drop.py
"""
import re
import sys
from collections import defaultdict

sys.path.insert(0, "tools")
import pins_audit  # noqa: E402

BSS_START = 0x800A3800  # .main ends here (text+data 0x93800 bytes from 0x80010000)
BSS_END_NAME = "D_801078E0"  # the end address crt0 zeroes up to
OUT = "asm/data/94000.bss.s"
OBJ_LINE = "        build/asm/data/94000.bss.o(.bss);\n"
LD_END = "        main_BSS_END = .;\n"
ADDR_LINE = re.compile(r"^/\* ([0-9A-F]{8}) \*/$")
LABEL = re.compile(r"^dlabel (\S+)(?: /\* ([0-9A-F]{8}) \*/)?$")


def read_labels():
    try:
        with open(OUT, encoding="utf-8") as f:
            text = f.read()
    except FileNotFoundError:
        return {}
    out, addr = {}, None
    for line in text.split("\n"):
        m = ADDR_LINE.match(line)
        if m:
            addr = int(m.group(1), 16)
            continue
        m = LABEL.match(line)
        if m:
            at = int(m.group(2), 16) if m.group(2) else addr
            if at is None:
                sys.exit("bss_link: %s in %s has no address" % (m.group(1), OUT))
            out[m.group(1)] = at
    return out


def object_defined():
    """Names a linked object defines as a global/weak symbol outside COMMON and OUT."""
    rows, _ = pins_audit.classify()
    out = set()
    for r in rows:
        objs = r[4]
        if r[3] == "a" and r[5] != "*COM*" and not objs.startswith("build/" + OUT[:-2]):
            out.add(r[0])
    return out


def main():
    labels = read_labels()
    pins = defaultdict(set)
    for name, addr, _, _ in pins_audit.read_pins():
        pins[name].add(addr)
    end = labels.get(BSS_END_NAME)
    if end is None:
        if len(pins.get(BSS_END_NAME, ())) != 1:
            sys.exit("bss_link: %s is neither labelled in %s nor pinned once" % (BSS_END_NAME, OUT))
        end = next(iter(pins[BSS_END_NAME]))
    owned = object_defined()
    want = dict(labels)
    for name, addrs in sorted(pins.items()):
        inr = [a for a in addrs if BSS_START <= a <= end]
        if not inr or name in owned:
            continue
        if len(addrs) > 1:
            sys.exit("bss_link: %s is pinned at %s" % (name, ", ".join(map(hex, sorted(addrs)))))
        if name in want and want[name] != inr[0]:
            sys.exit("bss_link: %s is labelled at 0x%08X but pinned at 0x%08X"
                     % (name, want[name], inr[0]))
        want[name] = inr[0]

    groups = defaultdict(list)
    for name, addr in want.items():
        groups[addr].append(name)
    addrs = sorted(groups)
    lines = [".include \"macro.inc\"", "",
             "/* .bss 0x%08X..0x%08X: one label per name the code uses, written by" % (BSS_START, end),
             " * tools/bss_link.py (crt0 zeroes this range; nothing here is in the EXE). */", "",
             ".bss", "    .align 0", ""]
    if addrs and addrs[0] > BSS_START:
        lines += ["    .space 0x%X" % (addrs[0] - BSS_START), ""]
    for i, addr in enumerate(addrs):
        names = sorted(groups[addr])
        lines.append("/* %08X */" % addr)
        lines += ["dlabel %s" % n for n in names]
        if i + 1 < len(addrs):
            lines.append("    .space 0x%X" % (addrs[i + 1] - addr))
        elif addr < end:
            lines.append("    .space 0x%X" % (end - addr))
        lines += ["enddlabel %s" % n for n in names]
        lines.append("")
    with open(OUT, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))

    with open("bb2.ld", encoding="utf-8") as f:
        ld = f.read()
    if OBJ_LINE not in ld:
        if ld.count(LD_END) != 1:
            sys.exit("bss_link: bb2.ld has no single `main_BSS_END = .;` line")
        ld = ld.replace(LD_END, OBJ_LINE + LD_END)
        with open("bb2.ld", "w", encoding="utf-8", newline="\n") as f:
            f.write(ld)
    print("bss_link: %d labels at %d addresses in %s (0x%08X..0x%08X)"
          % (len(want), len(addrs), OUT, BSS_START, end))


if __name__ == "__main__":
    main()
