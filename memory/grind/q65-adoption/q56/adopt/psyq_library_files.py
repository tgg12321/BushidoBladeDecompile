#!/usr/bin/env python3
"""psyq_library_files.py [--check]: the C files that are Sony PsyQ library code (owner ruling Q69).

Sony's library objects were compiled -G0, so maspsx runs them without -G8; every other file gets -G8
(Makefile PSYQ_LIBRARY_FILES, mirrored in engine/buildconfig.py). The set is derived from evidence, never
hand-picked: a file is library code when its whole .text (from build/bb2.map) lies inside the library span
recorded by the provenance census, memory/closer/psyq-library-census.md ("Contiguous span A..B contains ALL
of it": 177 verbatim PsyQ 4.0 module placements plus the newer-build LIBSND/LIBSPU gaps between them; the
2026-08-18 provenance research, docs/grind/psyq-provenance-2026-08-18.md, confirms the linked versions).

The rule is per-file-gp-model.md (A4): a file is library code when its link-map .text input section is
non-empty and lies entirely within the span; any other file, including one with no .text, gets -G8.

The GPREL16 check reads OUR OWN build objects (build/src/*.o), not the shipped code: a gp-relative relocation
in a library file's object would contradict -G0, and the tool reports it. Prints the set; --table prints every
src/*.c file with its .text range and class; --check compares the set with the Makefile and
engine/buildconfig.py and exits 1 on any difference. Needs a linked build (build/bb2.map)."""
import os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def library_files(root=ROOT):
    cen = open(os.path.join(root, "memory/closer/psyq-library-census.md"), encoding="utf-8").read()
    m = re.search(r"Contiguous span (0x[0-9A-Fa-f]+)\.\.(0x[0-9A-Fa-f]+)", cen)
    s0, s1 = int(m.group(1), 16), int(m.group(2), 16)
    out, gp = [], []
    mp = open(os.path.join(root, "build/bb2.map")).read()
    for mm in re.finditer(r"^ \.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) build/src/(\w+)\.o", mp, re.M):
        a, n, f = int(mm.group(1), 16), int(mm.group(2), 16), mm.group(3)
        if n and s0 <= a and a + n <= s1:
            out.append(f)
            r = subprocess.run(["mipsel-linux-gnu-objdump", "-r", os.path.join(root, f"build/src/{f}.o")],
                               capture_output=True, text=True).stdout
            if "R_MIPS_GPREL16" in r:
                gp.append(f)
    return sorted(out), gp


def declared(root=ROOT):
    mk = open(os.path.join(root, "Makefile")).read()
    a = sorted(re.search(r"^PSYQ_LIBRARY_FILES :=(.*)$", mk, re.M).group(1).split())
    bc = open(os.path.join(root, "engine/buildconfig.py")).read()
    b = sorted(re.findall(r'"(\w+)"', re.search(r"^PSYQ_LIBRARY_FILES = \{(.*)\}$", bc, re.M).group(1)))
    return a, b


def table(root=ROOT):
    """every src/*.c file: (stem, .text start, .text end or None, class)."""
    cen = open(os.path.join(root, "memory/closer/psyq-library-census.md"), encoding="utf-8").read()
    m = re.search(r"Contiguous span (0x[0-9A-Fa-f]+)\.\.(0x[0-9A-Fa-f]+)", cen)
    s0, s1 = int(m.group(1), 16), int(m.group(2), 16)
    mp = open(os.path.join(root, "build/bb2.map")).read()
    txt = {f: (int(a, 16), int(n, 16)) for a, n, f in
           re.findall(r"^ \.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) build/src/(\w+)\.o", mp, re.M)}
    rows = []
    for c in sorted(os.listdir(os.path.join(root, "src"))):
        if not c.endswith(".c"):
            continue
        f = c[:-2]
        a, n = txt.get(f, (None, 0))
        lib = bool(n) and s0 <= a and a + n <= s1
        rows.append((f, a if n else None, a + n if n else None, "library (-G0)" if lib else "-G8"))
    return rows


if __name__ == "__main__":
    if "--table" in sys.argv:
        for f, a, e, c in sorted(table(), key=lambda r: (r[1] is None, r[1] or 0, r[0])):
            print(f"| {f} | " + (f"{a:#010x}..{e:#010x}" if a is not None else "no .text") + f" | {c} |")
        sys.exit(0)
    lib, gp = library_files()
    print(" ".join(lib))
    if gp:
        print("CONTRADICTION: gp-relative accesses in library files:", " ".join(gp), file=sys.stderr)
        sys.exit(1)
    if "--check" in sys.argv:
        mk, bc = declared()
        if mk != lib or bc != lib:
            print(f"MISMATCH: Makefile {mk} / buildconfig {bc} / evidence {lib}", file=sys.stderr)
            sys.exit(1)
