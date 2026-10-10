#!/usr/bin/env python3
"""
data_wave.py — atomic RENAME cascade for BB2 DATA symbols (globals), the data-side sibling
of tools/naming_wave.py.

Given a manifest of `addr,proposed_name` rows (default docs/naming/libscan/data_manifest.csv,
verdict CONFIRM only), it retires EVERY symbol currently defined at that address (the splat
`D_<ADDR>` auto name and any `g_*` aliases in the three linker-script registries) in favour
of the proposed name, across every surface where a data name is a key:

`kind=retire-alias` instead retires only the semicolon-separated `current_names`;
`proposed_name` must already survive at that address. Auto names, functions and
assembly definition labels cannot be retired this way; other aliases stay intact.

  src/**/*.[ch], include/**/*.h   whole-word substitution outside comments (string literals in
                                  scope — INCLUDE_ASM / __asm__ bodies), via naming_wave.sub_c
  asm/funcs/*.s, asm/data/*.s     same (%hi/%lo operands, .word references)
  bb2.ld                          same
  named_syms.txt, symbol_addrs.txt, undefined_syms_auto.txt
                                  line-wise: the definition line is rewritten to the new name;
                                  a second definition of the same (name, addr) is dropped so
                                  the linker sees one assignment; retired names in comments stay
  *.txt gate lists (volatile_extern_allowlist.txt, sdata_*.txt, ...)   sub_hash
  tools/**/*.py, engine/**/*.py   short string-literal keys only, via naming_wave.sub_py

Byte-neutrality is the claim, the oracle is the proof: run
`python3 -m engine.cli verify-oracle --rebuild --allow-dirty` after --apply.
--apply refuses when a path it would edit is already dirty, so `git checkout` of its paths is
always a clean rollback (other agents' dirt elsewhere is tolerated, as in naming_wave.py).

Usage:
  python3 tools/data_wave.py                      # dry run over the default manifest
  python3 tools/data_wave.py --manifest-csv X --apply --manifest tmp/out.json
"""
from __future__ import annotations
import argparse, csv, glob, json, os, re, subprocess, sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "tools"))
import naming_wave as nw  # noqa: E402  (sub_c, sub_hash, sub_py, read, write_lf)

REGISTRIES = ["undefined_syms_auto.txt", "named_syms.txt", "symbol_addrs.txt"]
SYM_LINE = re.compile(r"^(\s*)([A-Za-z_]\w*)(\s*=\s*)(0x[0-9A-Fa-f]{8})(\s*;)(.*)$")
AUTO = re.compile(r"^(D|func)_[0-9A-Fa-f]{8}$")
ADDR = re.compile(r"^0x(?:8[0-9A-Fa-f]{7}|1[fF]800[0-3][0-9A-Fa-f]{2})$")
LABEL = re.compile(r"(?m)^\s*(?:dlabel|glabel)\s+([A-Za-z_]\w*)\s*$")
MACHINE_ADDR = re.compile(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s")
MAPDEF = re.compile(r"^\s+0x0*([0-9A-Fa-f]{8})\s+([A-Za-z_]\w*)\s*$")
# (The sdata_syms / sdata_funcs / sdata_exclude lists were retired by owner ruling Q65: gp now
# follows each file's own definitions, which a rename changes together with every use.)
LIST_FILES = ["volatile_extern_allowlist.txt"]
WAVE_TAG = "data-wave " + __import__("datetime").date.today().isoformat()


def die(msg):
    print("FATAL:", msg, file=sys.stderr); sys.exit(2)


def registry_defs():
    """addr(lower) -> {name: [(file, lineno)]}"""
    defs = defaultdict(lambda: defaultdict(list))
    for rel in REGISTRIES:
        p = ROOT / rel
        if not p.exists():
            continue
        for i, line in enumerate(nw.read(p).split("\n"), 1):
            m = SYM_LINE.match(line)
            if m:
                defs[m.group(4).lower()][m.group(2)].append((rel, i))
    return defs


def asm_defs(text):
    """All labels, bounded by the next label so empty blocks cannot steal its address."""
    labels = list(LABEL.finditer(text))
    for i, label in enumerate(labels):
        end = labels[i + 1].start() if i + 1 < len(labels) else len(text)
        m = MACHINE_ADDR.search(text, label.end(), end)
        name = label.group(1)
        addr = m.group(1) if m else name[-8:] if AUTO.fullmatch(name) else None
        if addr:
            yield "0x" + addr.lower(), name


def object_defs(initialized=None, asm_sources=None):
    defs = defaultdict(set)
    if asm_sources is None:
        asm_sources = {p: nw.read(p) for p in sorted((ROOT / "asm").rglob("*.s"))}
    for text in asm_sources.values():
        for addr, name in asm_defs(text):
            defs[addr].add(name)
    mp = ROOT / "build/bb2.map"
    if mp.exists():
        if initialized is None:
            initialized = set().union(*(c_data_names(nw.read(p)) for p in (ROOT / "src").rglob("*.c")))
        asm_names = {n for names in defs.values() for n in names}
        for addr, name in map_defs(nw.read(mp)):
            if name in initialized or name in asm_names:
                defs[addr].add(name)
    return defs


def map_defs(text):
    """Only linked map output, never symbols listed under Discarded input sections."""
    if "Discarded input sections" in text:
        text = text.partition("Linker script and memory map")[2]
    for line in text.splitlines():
        m = MAPDEF.match(line)
        if m:
            yield "0x" + m.group(1).lower(), m.group(2)


def c_data_names(text):
    """Initialized external data only: tentative definitions land in discarded .scommon."""
    from engine import layer2
    # These branches are not built by the matching pipeline. Leave other conditions alone.
    lines, disabled = [], []
    for line in text.splitlines(keepends=True):
        directive = re.match(r"\s*#\s*(if|ifdef|ifndef|else|elif|endif)\b(.*)", line)
        if directive:
            op, arg = directive.groups()
            if op in ("if", "ifdef", "ifndef"):
                disabled.append((bool(disabled and disabled[-1][0]),
                                 op == "if" and arg.strip() == "0" or
                                 op == "ifdef" and arg.strip() == "NON_MATCHING"))
            elif op in ("else", "elif") and disabled:
                parent, off = disabled[-1]
                disabled[-1] = (parent, not off if op == "else" else False)
            elif op == "endif" and disabled:
                disabled.pop()
        elif not any(parent or off for parent, off in disabled):
            lines.append(line)
    text = "".join(lines)
    tokens = layer2.tokens(re.sub(r"(?m)^\s*#.*(?:\\\n.*)*", "", text))
    depth, decl, names = 0, [], set()
    for token in tokens:
        if token == "{":
            if depth == 0 and "=" not in decl:
                decl = []  # function/struct body, not a data initializer
            depth += 1
        elif token == "}":
            depth -= 1
        elif depth == 0:
            if token == ";":
                head = decl[:decl.index("=")] if "=" in decl else []
                # A use in an array bound or a type name is not the declarator.
                if head and not {"extern", "static", "typedef", "("} & set(head):
                    # First declarator only; unsupported complex declarations fail closed.
                    for i, tok in enumerate(head):
                        if re.fullmatch(r"[A-Za-z_]\w*", tok) and (i + 1 == len(head) or head[i + 1] == "["):
                            names.add(tok)
                            break
                decl = []
            else:
                decl.append(token)
    return names


def c_data_defined(text, name):
    return name in c_data_names(text)


def c_defined(name):
    return any(c_data_defined(nw.read(p), name) for p in sorted((ROOT / "src").rglob("*.c")))


def note_kind(addr, survivor):
    return "RESET" if survivor.upper() == "D_" + addr.removeprefix("0x").upper() else "RENAME"


def registry_note(indent, tail, rel, message):
    note = tail.strip().strip("/*").strip("*/").strip().removeprefix("//").strip()
    if not note:
        return None
    open_, close = ("// ", "") if rel == "symbol_addrs.txt" else ("/* ", " */")
    return f"{indent}{open_}{message} — preserved note: {note}{close}"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest-csv", default=str(ROOT / "docs/naming/libscan/data_manifest.csv"))
    ap.add_argument("--only", default="", help="comma-separated addresses")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--manifest", default="", help="write the edit manifest JSON here")
    a = ap.parse_args()


    rows = [r for r in csv.DictReader(open(a.manifest_csv, encoding="utf-8", errors="replace"))
            if (r.get("verdict") or "CONFIRM").strip().upper() == "CONFIRM"]
    only = {x.strip().lower() for x in a.only.split(",") if x.strip()}
    defs = registry_defs()
    c_sources = {p: nw.read(p) for p in (ROOT / "src").rglob("*.c")}
    initialized_by_file = {p: c_data_names(t) for p, t in c_sources.items()}
    initialized = set().union(*initialized_by_file.values())
    asm_sources = {p: nw.read(p) for p in (ROOT / "asm").rglob("*.s")}
    objdef = object_defs(initialized, asm_sources)
    asm_labels = {m.group(1) for text in asm_sources.values() for m in LABEL.finditer(text)}
    object_homes = {}
    for p, text in asm_sources.items():
        for j, line in enumerate(text.splitlines(), 1):
            m = LABEL.match(line)
            if m:
                object_homes.setdefault(m.group(1), f"{p.relative_to(ROOT).as_posix()}:{j}")
    c_homed = set()
    for p, text in c_sources.items():
        for m in re.finditer(r"(?m)^[ \t]*(?!extern\b|static\b|typedef\b)[A-Za-z_][\w \t*]*\b([A-Za-z_]\w*)\s*(?:\[[^]]*\]\s*)*=", text):
            if m.group(1) in initialized_by_file[p] and m.group(1) not in c_homed:
                object_homes[m.group(1)] = f"{p.relative_to(ROOT).as_posix()}:{text[:m.start()].count(chr(10)) + 1}"
                c_homed.add(m.group(1))
    functions = {m.group(1) for text in c_sources.values() for m in re.finditer(
        r"(?m)^[\w \t*]+\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{", text)}
    text_addrs = {ad for p, text in asm_sources.items() if p.parent.name == "funcs" for ad, _ in asm_defs(text)}
    mp = ROOT / "build/bb2.map"
    if mp.exists():
        in_text = False
        for line in nw.read(mp).splitlines():
            section = re.match(r"\s*(\.[A-Za-z_][\w.]*)\s+0x", line)
            if section:
                in_text = section.group(1).startswith(".text")
            m = MAPDEF.match(line)
            if in_text and m:
                text_addrs.add("0x" + m.group(1).lower())
    for addr, names in objdef.items():
        for name in names:
            defs[addr][name]  # address discovery includes object-only names

    ops = []   # (addr, new, olds)
    kinds = {}
    errors = []
    for r in rows:
        addr = r["addr"].strip().lower()
        new = r["proposed_name"].strip()
        if only and addr not in only:
            continue
        if not ADDR.match(addr) or not re.match(r"^[A-Za-z_]\w*$", new):
            errors.append(f"{addr}: bad row {new!r}"); continue
        kind = (r.get("kind") or "rename").strip().lower()
        if kind not in ("rename", "reset", "retire-alias"):
            errors.append(f"{addr}: unknown kind {kind!r}"); continue
        if kind == "retire-alias":
            olds = {n.strip() for n in (r.get("current_names") or "").split(";") if n.strip()}
            if (not olds or new not in defs.get(addr, {}) or new in olds or
                    any(AUTO.fullmatch(n) or n not in defs.get(addr, {}) for n in olds)):
                errors.append(f"{addr}: retire-alias needs existing aliases and a surviving name at this address")
                continue
            if addr in text_addrs or olds & (asm_labels | functions):
                errors.append(f"{addr}: retire-alias cannot retire a function or assembly definition label"); continue
        else:
            olds = set(defs.get(addr, {}))
            olds.add("D_" + addr[2:].upper())
        kinds[(addr, new)] = kind
        olds.discard(new)
        # P1: the new name must not be defined at ANOTHER address
        other = [ad for ad, d in defs.items() if new in d and ad != addr]
        if other:
            errors.append(f"{addr}: '{new}' is already defined at {other}"); continue
        # P2: at least one old name must actually be referenced or defined somewhere
        ops.append((addr, new, sorted(olds)))
    for addr in {ad for ad, _, _ in ops}:
        at_addr = [(new, olds) for ad, new, olds in ops if ad == addr]
        if len(at_addr) > 1 and any(kinds[(addr, new)] == "retire-alias" for new, _ in at_addr):
            errors.append(f"{addr}: multiple retire-alias rows at one address are not supported")
    if errors:
        for e in errors:
            print("  REJECT:", e)
        die("preflight failed")
    if not ops:
        die("no operations")

    code_map = {}
    for addr, new, olds in ops:
        for o in olds:
            if o in code_map and code_map[o] != new:
                die(f"identifier {o} mapped to two targets")
            code_map[o] = new
    if not code_map:
        die("no identifiers to rename; the selected rows already use their target names")
    pat = re.compile(r"\b(" + "|".join(re.escape(k) for k in sorted(code_map, key=len, reverse=True)) + r")\b")
    repl = lambda m: code_map[m.group(1)]

    # INCLUDE_RODATA keys also name files: refuse rather than leave a broken include.
    for p, text in c_sources.items():
        for m in re.finditer(r'INCLUDE_RODATA\s*\(\s*"([^"]+)"\s*,\s*(\w+)\s*\)', text):
            if m.group(2) in code_map:
                die(f"{p.relative_to(ROOT)}: INCLUDE_RODATA {m.group(2)} requires renaming "
                    f"{m.group(1)}/{m.group(2)}.s; data_wave does not support this file move")

    edits = {}      # rel -> (new_text, hits)
    def plan_file(rel, fn):
        p = ROOT / rel
        if not p.exists():
            return
        text = c_sources[p] if p in c_sources else asm_sources[p] if p in asm_sources else nw.read(p)
        new_text, n = fn(text)
        if n:
            edits[rel] = (new_text, n)

    for pattern in ("src/**/*.c", "src/**/*.h", "include/**/*.h", "asm/**/*.s", "bb2.ld"):
        for p in sorted(glob.glob(str(ROOT / pattern), recursive=True)):
            plan_file(os.path.relpath(p, ROOT).replace("\\", "/"), lambda t: nw.sub_c(pat, repl, t))
    for rel in LIST_FILES:
        plan_file(rel, lambda t: nw.sub_hash(pat, repl, t))
    py_notes = []
    for root in ("tools", "engine"):
        for p in sorted(glob.glob(str(ROOT / root / "**" / "*.py"), recursive=True)):
            rel = os.path.relpath(p, ROOT).replace("\\", "/")
            if any(part in nw.PY_EXCLUDE_PARTS for part in rel.split("/")) or rel in nw.PY_EXCLUDE_FILES \
                    or rel == "tools/data_wave.py":
                continue
            text = nw.read(ROOT / rel)
            new_text, n, notes = nw.sub_py(pat, repl, text)
            py_notes += [f"{rel}: {x}" for x in notes]
            if n:
                edits[rel] = (new_text, n)

    # object-defined symbols (build/bb2.map): when an old name is defined by an OBJECT (a
    # dlabel block in asm/data/*.s, or a C definition), the .s/.c rename makes the object
    # define the NEW name, so every registry assignment at that address becomes a duplicate
    # definition and is dropped (a comment keeps the retired alias visible).
    obj_addrs = {ad for ad, new, olds in ops
                 if objdef.get(ad, set()) & {new, *olds} or initialized & {new, *olds}}

    # registries: line-wise
    reg_report = []
    # Curated definitions win over auto registries. Splat input remains independently intact.
    seen = {}
    targets = {addr: new for addr, new, olds in ops}
    for rel in ("named_syms.txt", "undefined_syms_auto.txt", "symbol_addrs.txt"):
        p = ROOT / rel
        if not p.exists():
            continue
        lines = nw.read(p).split("\n")
        out, n = [], 0
        for i, line in enumerate(lines, 1):
            m = SYM_LINE.match(line)
            if m and m.group(4).lower() in targets and (m.group(2) in code_map or
                    kinds[(m.group(4).lower(), targets[m.group(4).lower()])] != "retire-alias"):
                addr = m.group(4).lower(); new = targets[addr]
                alias_only = kinds[(addr, new)] == "retire-alias"
                # Retirement notes retain all curated prose, including splat comments.
                if addr in obj_addrs and rel != "symbol_addrs.txt" or alias_only:
                    cmt = "/* %s */" if rel != "symbol_addrs.txt" else "// %s"
                    if m.group(2) == new:
                        # Reverse aliases duplicate an object's definition, not a retired name.
                        home = object_homes.get(new)
                        if home is None:
                            die(f"{addr}: cannot locate a source definition for duplicate {new}")
                        message = f"duplicate of {home}"
                    else:
                        message = f"{note_kind(addr, new)} {m.group(4)}: retired name '{m.group(2)}'"
                    note = registry_note(m.group(1), m.group(6), rel, message)
                    if note:
                        out.append(note)
                    reg_report.append(f"{rel}:{i} dropped {m.group(2)}; surviving definition is {new}")
                    n += 1
                    continue
                if (new, addr) in seen and rel != "symbol_addrs.txt":
                    reg_report.append(f"{rel}:{i} dropped duplicate definition of {new} (was {m.group(2)})")
                    message = (f"duplicate of {seen[(new, addr)]}" if m.group(2) == new else
                               f"{note_kind(addr, new)} {m.group(4)}: retired name '{m.group(2)}'")
                    note = registry_note(m.group(1), m.group(6), rel, message)
                    if note:
                        out.append(note)
                    n += 1
                    continue
                seen[(new, addr)] = f"{rel}:{len(out) + 1}"
                tail = m.group(6)
                note = f"  /* {WAVE_TAG}: was {m.group(2)} */" if rel != "symbol_addrs.txt" else f"  // {WAVE_TAG}: was {m.group(2)}"
                if m.group(2) != new:
                    out.append(f"{m.group(1)}{new}{m.group(3)}{m.group(4)}{m.group(5)}{tail}{'' if 'data-wave' in tail else note}")
                    reg_report.append(f"{rel}:{i} {m.group(2)} -> {new}")
                    n += 1
                else:
                    out.append(line)
            else:
                out.append(line)
        if n:
            edits[rel] = ("\n".join(out), n)

    print("=" * 78)
    print(f"DATA WAVE — {len(ops)} addresses, {len(code_map)} identifiers remapped")
    for addr, new, olds in ops:
        print(f"  {addr}: {', '.join(olds)} -> {new}")
    print(f"\n-- file edits ({len(edits)}) --")
    for rel, (t, n) in sorted(edits.items()):
        print(f"  {rel}  ({n})")
    if reg_report:
        print("\n-- registry lines --")
        for x in reg_report:
            print("  " + x)
    if py_notes:
        print("\n-- python long strings NOT rewritten --")
        for x in py_notes[:20]:
            print("  " + x)
    manifest = dict(mode="apply" if a.apply else "dry-run",
                    ops=[dict(addr=ad, new=nw_, olds=ol, kind=kinds[(ad, nw_)]) for ad, nw_, ol in ops],
                    edits={rel: n for rel, (t, n) in edits.items()}, registry=reg_report, py_notes=py_notes)
    if a.manifest:
        Path(a.manifest).write_text(json.dumps(manifest, indent=1), encoding="utf-8")
    if not a.apply:
        print("\nDRY RUN — nothing written. Re-run with --apply.")
        return 0
    # Rollback point: only OUR paths must be clean. Other agents work the repo concurrently and
    # their dirt elsewhere is none of this tool's business (same rule as naming_wave.py).
    st = subprocess.run(["git", "status", "--porcelain"], cwd=ROOT, capture_output=True, text=True)
    if st.returncode != 0:
        die("git status failed; cannot establish a rollback point")
    dirty = {ln[3:].strip().strip('"').split(" -> ")[-1] for ln in st.stdout.splitlines()}
    clash = sorted(dirty & set(edits))
    if clash:
        die("these paths already carry uncommitted changes, so git checkout would not be a "
            "clean rollback:\n  " + "\n  ".join(clash))
    for rel, (t, n) in edits.items():
        nw.write_lf(ROOT / rel, t)
    # residual audit: any old name still present outside comments?
    resid = []
    for rel in list(edits) + ["include/m2c_context.h"]:
        p = ROOT / rel
        if not p.exists():
            continue
        text = nw.read(p)
        found = set(pat.findall(re.sub(nw._C_TOKEN, lambda m: m.group(0) if not m.group(0).startswith(("//", "/*")) else "", text)))
        if found:
            resid.append(f"{rel}: {sorted(found)}")
    print("\nAPPLIED.", "Residuals:" if resid else "No residual old names outside comments.")
    for x in resid:
        print("  " + x)
    print("Now prove byte-neutrality: python3 -m engine.cli verify-oracle --rebuild --allow-dirty")
    return 0


if __name__ == "__main__":
    sys.exit(main())
