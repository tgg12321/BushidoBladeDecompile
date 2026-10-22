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

Files are TU ids (path under src/ without .c, engine/tus.py). The GPREL16 check reads OUR OWN build objects (build/src/<id>.o), not the shipped code: a gp-relative relocation
in a library file's object would contradict -G0, and the tool reports it. Prints the set; --table prints every
src/**/*.c file with its .text range and class; --check compares the set with the Makefile and
engine/buildconfig.py and exits 1 on any difference. Needs a linked build (build/bb2.map).

It also derives the data-only no-small-data set (owner ruling Q122, .claude/rules/data-only-no-sdata.md;
Makefile DATA_ONLY_NO_SDATA_FILES, mirrored in engine/buildconfig.py): a file with no .text input section
and no function symbol, whose .data input section is non-empty and lies entirely in the original .data
range below the .sdata range (0x800A30DC), and which defines an initialized object of .size <= 8. Such a
file is also built without maspsx -G8, so those objects stay in .data. The derivation reads its own build's
output (a file in the set keeps its small objects in .data), so `--check` first requires the linked
build/bb2.exe SHA1 == the oracle: only then is the map the original's layout."""
import hashlib, os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def library_files(root=ROOT):
    cen = open(os.path.join(root, "memory/closer/psyq-library-census.md"), encoding="utf-8").read()
    m = re.search(r"Contiguous span (0x[0-9A-Fa-f]+)\.\.(0x[0-9A-Fa-f]+)", cen)
    s0, s1 = int(m.group(1), 16), int(m.group(2), 16)
    out, gp = [], []
    mp = open(os.path.join(root, "build/bb2.map")).read()
    for mm in re.finditer(r"^ \.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) build/src/(\S+)\.o$", mp, re.M):
        a, n, f = int(mm.group(1), 16), int(mm.group(2), 16), mm.group(3)
        if n and s0 <= a and a + n <= s1:
            out.append(f)
            r = subprocess.run(["mipsel-linux-gnu-objdump", "-r", os.path.join(root, f"build/src/{f}.o")],
                               capture_output=True, text=True).stdout
            if "R_MIPS_GPREL16" in r:
                gp.append(f)
    return sorted(out), gp


SDATA_START = 0x800A30DC  # the original .sdata range starts here (.claude/rules/data-only-no-sdata.md)
ORACLE_SHA1 = "62efab4f73f992798c43e8c730aa43baa10bb4fa"


def build_is_oracle(root=ROOT):
    p = os.path.join(root, "build/bb2.exe")
    return os.path.exists(p) and hashlib.sha1(open(p, "rb").read()).hexdigest() == ORACLE_SHA1


def data_only_files(root=ROOT):
    """Q122: TU ids with no .text and no function symbol whose .data lies in the original .data below
    SDATA_START and holds an initialized object of .size <= 8."""
    mp = open(os.path.join(root, "build/bb2.map")).read()
    sec = {}
    for s, a, n, f in re.findall(r"^ (\.text|\.data)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) build/src/(\S+)\.o$", mp, re.M):
        sec.setdefault(f, {})[s] = (int(a, 16), int(n, 16))
    # the original .data starts at the lowest non-empty .data input section of any object (C or asm)
    data = [int(a, 16) for a, n in re.findall(r"^ \.data\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) build/\S+\.o$", mp, re.M)
            if int(n, 16) and int(a, 16)]
    lo = min(data) if data else SDATA_START
    out = []
    for f, ss in sorted(sec.items()):
        a, n = ss.get(".data", (0, 0))
        if ss.get(".text", (0, 0))[1] or not n or not (lo <= a and a + n <= SDATA_START):
            continue
        syms = subprocess.run(["mipsel-linux-gnu-objdump", "-t", os.path.join(root, f"build/src/{f}.o")],
                              capture_output=True, text=True).stdout
        rows = [ln.split() for ln in syms.splitlines() if re.match(r"^[0-9a-f]{8} ", ln)]
        if any(len(r) >= 3 and r[2] == "F" for r in rows):
            continue
        # "<value> <flags> O .data <size> <name>": an initialized .data object of 1..8 bytes
        if any(len(r) >= 6 and r[-4] == "O" and r[-3] == ".data" and 0 < int(r[-2], 16) <= 8 for r in rows):
            out.append(f)
    return out


def _declared(root, name):
    mk = open(os.path.join(root, "Makefile")).read()
    m = re.search(r"^" + name + r" :=(.*)$", mk, re.M)
    a = sorted(m.group(1).split()) if m else None
    bc = open(os.path.join(root, "engine/buildconfig.py")).read()
    m = re.search(r"^" + name + r" = (\{.*\}|set\(\))$", bc, re.M)
    b = sorted(re.findall(r'"([\w/.-]+)"', m.group(1))) if m else None
    return a, b


def declared(root=ROOT):
    return _declared(root, "PSYQ_LIBRARY_FILES")


def declared_data_only(root=ROOT):
    return _declared(root, "DATA_ONLY_NO_SDATA_FILES")


def table(root=ROOT):
    """every src/**/*.c file: (TU id, .text start, .text end or None, class)."""
    cen = open(os.path.join(root, "memory/closer/psyq-library-census.md"), encoding="utf-8").read()
    m = re.search(r"Contiguous span (0x[0-9A-Fa-f]+)\.\.(0x[0-9A-Fa-f]+)", cen)
    s0, s1 = int(m.group(1), 16), int(m.group(2), 16)
    mp = open(os.path.join(root, "build/bb2.map")).read()
    txt = {f: (int(a, 16), int(n, 16)) for a, n, f in
           re.findall(r"^ \.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) build/src/(\S+)\.o$", mp, re.M)}
    rows = []
    src = os.path.join(root, "src")
    ids = sorted(os.path.relpath(os.path.join(d, c), src)[:-2].replace(os.sep, "/")
                 for d, _, cs in os.walk(src) for c in cs if c.endswith(".c"))
    for f in ids:
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
    dat = data_only_files()
    print(" ".join(lib))
    print("DATA_ONLY_NO_SDATA_FILES:", " ".join(dat))
    if gp:
        print("CONTRADICTION: gp-relative accesses in library files:", " ".join(gp), file=sys.stderr)
        sys.exit(1)
    if "--check" in sys.argv:
        if not build_is_oracle():
            print(f"NO EVIDENCE: build/bb2.exe SHA1 != oracle {ORACLE_SHA1}; rebuild first", file=sys.stderr)
            sys.exit(1)
        mk, bc = declared()
        if mk != lib or bc != lib:
            print(f"MISMATCH: Makefile {mk} / buildconfig {bc} / evidence {lib}", file=sys.stderr)
            sys.exit(1)
        mk, bc = declared_data_only()
        if mk != dat or bc != dat:
            print(f"MISMATCH DATA_ONLY_NO_SDATA_FILES: Makefile {mk} / buildconfig {bc} / evidence {dat}",
                  file=sys.stderr)
            sys.exit(1)
