#!/usr/bin/env python3
"""inventory.py <tree> <pre-switch-tag> <out.md>: the dated small-data inventory for the Q65 adoption, checked
against .claude/rules/per-file-gp-model.md (A1)/(A2)/(K1)-(K3) and its gap-filler clause.

Inputs (from the series' switch step, run on that tree): /tmp/q56/s04_spans.json (the blocks and their items),
/tmp/q56/defs_plan.json (per object file: every small-data name it touches, gp or not), /tmp/q56/pre04obj
(the pre-switch oracle build's objects), the pre-switch symbol files (git show <tag>:...), asm/funcs and
asm/data at <tag>, and the original EXE.

For every block item:
  object  - E1 basis (gp from this file / (A2) between this file's gp objects), every OTHER file that
            references the name (any relocation), and the rule consequence ((A2)(1) / K2 cross-file);
  gap     - the run's references (a symbol-file name inside it that any object, asm function or asm data
            names; any 4-aligned word of the EXE whose value falls inside it), whether the build's own
            alignment of the next item already produces it (padding: no object), and otherwise whether ONE
            exact-size object can sit there (size <= 8; .lcomm aligns a static by its size: >=8 -> 8,
            >=4 -> 4, >=2 -> 2; an initialized object takes cc1's alignment: an array/struct is word-aligned
            (DATA_ALIGNMENT), a scalar its own size)."""
import json, os, re, subprocess, sys
T, TAG, OUT = sys.argv[1], sys.argv[2], sys.argv[3]
os.chdir(T)


def sh(c):
    return subprocess.run(c, shell=True, capture_output=True, text=True).stdout


EXE = open("disc/SLUS_006.63", "rb").read()
SP = json.load(open("/tmp/q56/s04_spans.json"))
P = json.load(open("/tmp/q56/defs_plan.json"))
symaddr = {}
for sf in ("undefined_syms_auto.txt", "named_syms.txt"):
    for l in sh(f"git show {TAG}:{sf}").splitlines():
        m = re.match(r"^\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)\s*;", l)
        if m:
            symaddr.setdefault(m.group(1), int(m.group(2), 16))
# every name any object references (relocations), per object
refs_by_obj = {}
for o in sorted(os.listdir("/tmp/q56/pre04obj")):
    if o.endswith(".o"):
        out = sh(f"mipsel-linux-gnu-objdump -r /tmp/q56/pre04obj/{o}")
        refs_by_obj[o[:-2]] = {re.sub(r"\+0x[0-9a-f]+$", "", l.split()[-1]) for l in out.splitlines()
                               if re.match(r"^[0-9a-f]+\s+R_MIPS_", l)}
asm_refs = set()
for d in ("asm/funcs", "asm/data"):
    for fn in sh(f"git ls-tree --name-only {TAG} {d}/").split():
        t = sh(f"git show {TAG}:{fn}")
        asm_refs |= set(re.findall(r"%(?:hi|lo|gp_rel)\((\w+)", t))
        asm_refs |= set(re.findall(r"^\s*(?:/\*.*?\*/)?\s*\.word\s+(\w+)", t, re.M))
all_refs = set().union(*refs_by_obj.values()) | asm_refs
words = {}
for off in range(0x800, len(EXE) - 3, 4):
    v = int.from_bytes(EXE[off:off + 4], "little")
    if 0x800A30CC <= v < 0x800AB0CC:
        words.setdefault(v, []).append(0x80010000 + off - 0x800)


def lcomm_align(n):
    # (A8, owner ruling Q79, rules: 8c57bc4ab): every `.lcomm` static is 4-aligned (lcomm_align_probe)
    return 4


def sdata_align(n):
    return n if n in (1, 2, 4) else 4   # 3, 5..8 bytes need an array or struct: word-aligned


rows, flags = [], []
for f, blocks in sorted(SP["items"].items()):
    for reg, items in blocks:
        for i, (kind, a, size, nm) in enumerate(items):
            a = int(a, 16)
            if kind == "obj":
                gp = P.get(f, {}).get(nm, {}).get("gp", False)
                others = sorted(o for o, rs in refs_by_obj.items() if o != f and nm in rs)
                basis = "gp" if gp else "(A2) between gp objects"
                note = ""
                if others:
                    note = ("K2 static referenced from another file -> borderline" if reg == "static"
                            else ("(A2)(1): referenced from another file -> merge test / borderline" if not gp
                                  else "K3, also referenced (non-gp) from other files: allowed"))
                    if not gp or reg == "static":
                        flags.append((f, reg, hex(a), nm, note + f" ({', '.join(others)})"))
                rows.append((f, reg, hex(a), size, "object " + nm, basis, ", ".join(others) or "-", note))
                continue
            end = a + size
            named = sorted(n for n, x in symaddr.items() if a <= x < end and n in all_refs)
            ptrs = sorted({w for v, ws in words.items() if a <= v < end for w in ws})
            nxt = items[i + 1] if i + 1 < len(items) else None
            if nxt is None:
                nal = 2   # the next input section in link order: bb2.ld SUBALIGN(2)
            else:
                ns = nxt[2]
                nal = lcomm_align(ns) if reg == "static" else sdata_align(ns)
            pad = ((a + nal - 1) // nal) * nal == end
            if named or ptrs:
                users = sorted({o for n in named for o, rs in refs_by_obj.items() if n in rs})
                cross = [u for u in users if u != f]
                verdict = ("objects named by the blob's labels, referenced from " + (", ".join(users) or "asm data only")
                           + ": " + ", ".join(named + [f"ptr-word@{w:#x}" for w in ptrs][:4]))
                if cross or ptrs:
                    flags.append((f, reg, hex(a), f"{size}B", verdict + (" - (A2)(1): another file references it" if cross
                                                                          else " - a pointer word in asm data names it")))
            elif pad:
                verdict = "alignment padding (no object)"
            elif size > 8:
                verdict = "run > 8 bytes -> borderline"
                flags.append((f, reg, hex(a), f"{size}B", verdict))
            else:
                al = lcomm_align(size) if reg == "static" else sdata_align(size)
                if a % al == 0:
                    verdict = f"one object D_{a:08X} ({size} B)"
                else:
                    verdict = f"NO single {size}-byte object fits at {a:#x} (needs {al}-alignment)"
                    flags.append((f, reg, hex(a), f"{size}B", verdict))
            rows.append((f, reg, hex(a), size, "gap", "-", "-", verdict))
with open(OUT, "w", newline="\n") as o:
    date = sh("date -u +%Y-%m-%d").strip()
    base = sh(f"git rev-parse --short {TAG}~0").strip()
    o.write(f"# Small-data inventory ({date}, pre-switch tree {TAG} = {base})\n\n")
    o.write("Blocks per file in link order (s04_spans.json); every object and every gap.\n\n")
    o.write("| file | region | address | size | item | E1 basis | other files referencing | rule note |\n")
    o.write("|---|---|---|---|---|---|---|---|\n")
    for r in rows:
        o.write("| " + " | ".join(str(x) for x in r) + " |\n")
    o.write(f"\nHeld blocks: {SP['held'] or 'none'}\n\n## Items the rule does not decide or refuses ({len(flags)})\n\n")
    for fl in flags:
        o.write("- " + " | ".join(fl) + "\n")
    o.write("\n## COMMON tentatives (K1) per file\n\n")
    for f, l in sorted(SP["tentative"].items()):
        o.write(f"- {f}: {', '.join(nm for _, nm in l)}\n")
print(f"{len(rows)} rows, {len(flags)} flags -> {OUT}")
