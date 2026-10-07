#!/usr/bin/env python3
"""naming_keycheck.py — prove a naming wave changed identifiers only (.claude/rules/naming-bar.md).

Compares the working tree (or --new <rev>) with --base <rev> (default HEAD). Fails closed: every
changed, added, deleted or renamed file must be explained by the wave's old->new pairs, using the
same substitutions the wave tools apply (tools/naming_wave.py, tools/data_wave.py):

  src/**, include/** *.c *.h  ctokens(new) == ctokens(old with the pairs substituted in code,
                              string literals (escape-aware) and comments), as is or after
                              tools/format.py (a longer name can re-wrap or hoist a comment):
                              C tokens plus each comment at its place and `#define NAME(`
                              adjacency. Files whose
                              only change is in comments are listed for the reviewer.
  memory/**/layer2.jsonl      byte-equal to naming_wave.retarget_layer2_record(old) for its ledger's
                              rename (func retargeted, renamed_from extended, verdict_file moved).
  other files in a moved ledger directory          byte-identical (a pure move).
  docs/naming/function-names.csv  must equal a fresh build_census.py run (working-tree mode;
                              it reads build/bb2.map, so run after verify-oracle --rebuild).
                              In --new mode a changed census FAILS (re-derive it in the tree).
  docs/naming/phase3/**       wave metadata: allowed, listed.
  .claude/**, movovl/**, this tool   any change FAILS (movovl/ is a separate binary the main
                              oracle does not cover).
  registries (named_syms.txt, symbol_addrs.txt, undefined_syms_auto.txt,
  undefined_funcs_auto.txt)   each old line becomes exactly one output the wave tools produce
                              for it (registry_ok): in-place rewrite; data_wave's rewrite with
                              `was <that line's old name>`; or, when the new name stays defined,
                              deletion, naming_wave's preserved-note line, or data_wave's
                              `... is now object-defined` line. A link-breaking deletion is left
                              to the build (SHA1).
  any other text file         equal to one of: whole-word substitution, sub_c, sub_hash, sub_py.
A path rename must be the old path with the pairs substituted; an added or deleted file must be
one side of such a rename. Two tree-wide checks: a non-auto new name must not already occur in
the base tree's build, tool and rule files (src, include, asm, tools, engine, movovl, .claude,
*.ld, *.txt, Makefile;
docs/ and memory/ cite proposals), an auto name must carry the old name's address, several old
names may share a new name only when they name one address, and no old name may survive in
a build file (src/, include/, asm/, *.ld, root *.txt, Makefile; C comments included).

--sub-comments is the wave's comment pass: it substitutes the pairs in the comments of every
src/ and include/ C file, then exits.

It lists every function whose layer-2 key (engine.layer2.body_key) moved, for the
naming-reviewer's R6: those keys moved by the pairs alone.

Usage:
  python3 tools/naming_keycheck.py --pairs old1=new1,old2=new2 [--base HEAD] [--new REV]
  python3 tools/naming_keycheck.py --pairs-file tmp/pairs.txt      # one old=new per line
Exit 0 = OK, 1 = a change the pairs do not explain, 2 = usage error.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

CODE = Path(__file__).resolve().parent.parent
# The repository checked; NAMING_KEYCHECK_ROOT points it at another clone (the probe suite).
ROOT = Path(os.environ.get("NAMING_KEYCHECK_ROOT") or CODE).resolve()
sys.path.insert(0, str(CODE))
sys.path.insert(0, str(CODE / "tools"))
from engine import layer2  # noqa: E402
import naming_wave as nw  # noqa: E402

IDENT = re.compile(r"[A-Za-z_]\w*")
AUTO = re.compile(r"^(?:func|D)_[0-9A-Fa-f]{8}$")
SELF = "tools/naming_keycheck.py"
METADATA = ("docs/naming/function-names.csv",)
METADATA_DIRS = ("docs/naming/phase3/",)
DENY_DIRS = (".claude/", "movovl/")  # rules/agents; the overlay (its own binary: no oracle here)
LEDGER_DIRS = tuple(d + "/" for d in nw.LEDGER_DIRS)
C_COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)


def git(*args: str, check: bool = True) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, check=check, capture_output=True,
                          text=True, encoding="utf-8", errors="surrogateescape").stdout


def show(rev: str | None, path: str) -> str | None:
    if rev is None:
        p = ROOT / path
        return nw.read(p) if p.is_file() else None
    r = subprocess.run(["git", "show", f"{rev}:{path}"], cwd=ROOT, capture_output=True,
                       text=True, encoding="utf-8", errors="surrogateescape")
    return r.stdout if r.returncode == 0 else None


def parse_pairs(spec: str) -> dict[str, str]:
    pairs: dict[str, str] = {}
    for item in re.split(r"[,\n]", spec):
        item = item.strip()
        if not item or item.startswith("#"):
            continue
        old, sep, new = item.partition("=")
        old, new = old.strip(), new.strip()
        if not sep or not IDENT.fullmatch(old) or not IDENT.fullmatch(new) or old == new:
            sys.exit(f"bad pair {item!r} (want old=new, two different identifiers)")
        if old in pairs and pairs[old] != new:
            sys.exit(f"{old} mapped twice")
        pairs[old] = new
    if not pairs:
        sys.exit("no pairs given")
    return pairs


def ctokens(text: str) -> list[str]:
    """engine.layer2.tokens plus what it drops that a wave must not change: each comment, as one
    token at its place (internal whitespace normalised), and `#define NAME(` adjacency (a
    function-like macro), which is a `NAME(` token."""
    out, in_pp, prev = [], False, []
    for m in layer2._TOKEN_RE.finditer(text):
        tok = m.group(0)
        if m.group("splice") is not None:
            continue
        if m.group("skip") is not None:
            if tok.startswith(("/*", "//")):
                out.append(" ".join(tok.split()))
            elif in_pp and "\n" in tok:
                out.append("\n")
                in_pp = False
            continue
        if tok == "#" and not in_pp and not text[text.rfind("\n", 0, m.start()) + 1:m.start()].strip():
            in_pp = True
        if prev[-2:] == ["#", "define"] and re.match(r"(?:\\\r?\n)*\(", text[m.end():]):
            tok += "("
        out.append(tok)
        prev = (prev + [tok])[-2:]
    return out


_FMT = None


def formatted(path: str, text: str) -> str | None:
    """`text` as tools/format.py would land it (format guard: staged C is always formatted), or
    None when the pinned clang-format is not installed. A rename can lengthen a line, and the
    formatter then re-wraps or hoists a comment; the wave is checked against that output."""
    global _FMT
    if _FMT is None:
        import format as cfmt  # tools/format.py
        exe = CODE / ".venv/bin/clang-format"
        _FMT = (cfmt, str(exe)) if exe.exists() else False
    if not _FMT:
        return None
    cfmt, exe = _FMT
    try:
        return cfmt.format_text(exe, path, text)
    except Exception:
        return None


def sub_comments(pat: re.Pattern, repl, text: str) -> tuple[str, int]:
    """Whole-word substitution inside C comments only (the wave's comment pass)."""
    out, n, pos = [], 0, 0
    for m in nw._C_TOKEN.finditer(text):
        tok = m.group(0)
        out.append(text[pos:m.start()])
        if tok.startswith(("//", "/*")):
            tok, k = pat.subn(repl, tok)
            n += k
        out.append(tok)
        pos = m.end()
    out.append(text[pos:])
    return "".join(out), n


def ledger_of(path: str) -> tuple[str, str] | None:
    """(ledger base, function dir) when `path` lies in a per-function ledger directory."""
    for base in sorted(LEDGER_DIRS, key=len, reverse=True):
        if path.startswith(base):
            rest = path[len(base):].split("/")
            if len(rest) >= 2:
                return base, rest[0]
    return None


REGISTRIES = ("named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt",
              "undefined_funcs_auto.txt")


def base_addr(base: str, name: str) -> str | None:
    """`name`'s address in the base tree: its own auto-name address, else the one address that
    ALL of these agree on: its asm/funcs/<name>.s glabel, `name = 0x...;` lines in the
    registries, and every census row naming it. None (unknown) when they disagree, when there is
    none, or when `name` is a #define in src/ or include/ (a macro names no address)."""
    if AUTO.match(name):
        return name[-8:].upper()
    if _is_macro(base, name):
        return None
    addrs = set()
    a = layer2._asm_addr(show(base, f"asm/funcs/{name}.s"), name)
    if a:
        addrs.add(a.upper())
    for reg in REGISTRIES:
        for m in re.finditer(r"(?m)^\s*" + re.escape(name) + r"\s*=\s*0x([0-9A-Fa-f]{8})\s*;",
                             show(base, reg) or ""):
            addrs.add(m.group(1).upper())
    # The census places C-defined names whose .s carries no address (hand-written listings);
    # every row naming `name` counts, so a name it lists at two addresses stays unknown.
    import csv
    import io
    for row in csv.DictReader(io.StringIO(show(base, METADATA[0]) or "")):
        names = {row.get("current_name"), row.get("glabel"),
                 *(x.strip() for x in (row.get("aliases") or "").split(";"))}
        if name in names:
            addrs.add((row.get("address") or "").upper().replace("0X", "").zfill(8))
    return addrs.pop() if len(addrs) == 1 else None


def _is_macro(base: str, name: str) -> bool:
    """True when src/ or include/ #defines `name` in the base tree."""
    r = subprocess.run(["git", "grep", "-q", "-E", r"^\s*#\s*define\s+" + re.escape(name) + r"\b",
                        base, "--", "src", "include"], cwd=ROOT, capture_output=True)
    return r.returncode == 0


SYM_LINE = re.compile(r"^(\s*)([A-Za-z_]\w*)(\s*=\s*)(0x[0-9A-Fa-f]{8})(\s*;)(.*)$")
DATE = r"\d{4}-\d{2}-\d{2}"


def registry_outputs(line: str, rel: str, pairs: dict[str, str], deletable, dup_before
                     ) -> list[list] | None:
    """What the wave tools may turn one old registry line into: a list of alternatives, each a
    list of 0 or 1 output lines (a str, or a compiled regex for a dated line). None = the line
    must stay byte-identical."""
    m = SYM_LINE.match(line)
    if not m:
        return None
    indent, name, eq, addr, semi, tail = m.groups()
    a8 = addr[2:].upper()
    open_, close = ("// ", "") if rel == "symbol_addrs.txt" else ("/* ", " */")
    if name not in pairs:
        # naming_wave drops a destination's own reverse alias (`func_X = 0xX;`) once X is defined
        if name in pairs.values() and AUTO.match(name) and name[-8:].upper() == a8:
            return [[line], []]
        return None
    new = pairs[name]
    rewritten = f"{indent}{new}{eq}{addr}{semi}{tail}"
    outs: list[list] = [[rewritten]]                       # naming_wave in place / data_wave
    if "data-wave" not in tail:                             # data_wave rewrite with provenance
        outs.append([re.compile(re.escape(rewritten + "  " + open_) + f"data-wave {DATE}: was "
                                + re.escape(name + close) + "$")])
    if dup_before(new, a8):
        outs.append([])                                     # data_wave's duplicate drop
    if deletable(new, a8):
        note = tail.strip()
        if note:                                            # naming_wave keeps the note
            note = note.strip("/*").strip("*/").strip()
            kind = "RESET" if AUTO.match(new) and new[-8:].upper() == a8 else "RENAME"
            outs.append([f"{indent}{open_}{kind} {addr}: retired name '{name}' \u2014 "
                         f"preserved note: {note}{close}"])
        else:
            outs.append([])                                 # naming_wave deletes a bare line
        outs.append([re.compile(re.escape(open_) + f"data-wave {DATE}: "
                                + re.escape(f"{name} = {addr} retired; {new} is now object-defined"
                                            + close) + "$")])
    return outs


def registry_ok(old: str, new: str, rel: str, pairs: dict[str, str], read_new=None) -> str | None:
    """A registry (`name = 0xADDR;  /* ... */` lines) changed by the wave tools. Every old line
    must become exactly one output the tools produce for it (registry_outputs):
      - the in-place rewrite (pairs substituted in the name);
      - data_wave's rewrite plus `  /* data-wave <date>: was <that line's old name> */`, only
        when the old tail had no data-wave note;
      - data_wave's duplicate drop: nothing, when an earlier output line already defines
        (new, addr);
      - when the new name stays defined (its own auto name, a (name, addr) still in the file, or
        a glabel file at that address): naming_wave's deletion of a line without a comment, its
        preserved-note line for a line with one (exactly `<RESET if the new name is the
        address's auto name, else RENAME> <addr>: retired name '<old>' -- preserved note:
        <the old comment, stripped as naming_wave strips it>`), or data_wave's
        `<old> = <addr> retired; <new> is now object-defined` line;
      - naming_wave's deletion of a destination's own reverse alias `func_X = 0xX;`.
    A link-breaking deletion the matcher allows is caught by the build (SHA1). Returns None when
    OK, else the first line no tool produces."""
    got = new.split("\n")
    remaining = {(mm.group(2), mm.group(4)[2:].upper()) for mm in map(SYM_LINE.match, got) if mm}
    emitted: set[tuple[str, str]] = set()

    def deletable(name: str, addr: str) -> bool:
        if AUTO.match(name) and name[-8:].upper() == addr:
            return True
        if (name, addr) in remaining:
            return True
        return bool(read_new and layer2._asm_addr(read_new(f"asm/funcs/{name}.s"), name) == addr)

    def fits(want, g: str) -> bool:
        return want.match(g) is not None if isinstance(want, re.Pattern) else want == g

    k = 0
    for line in old.split("\n"):
        outs = registry_outputs(line, rel, pairs, deletable, lambda n, a8: (n, a8) in emitted)
        here = got[k] if k < len(got) else None
        if outs is None:
            if here != line:
                return here if here is not None else f"missing: {line}"
        elif here is not None and any(alt and fits(alt[0], here) for alt in outs):
            pass
        elif [] in outs:
            continue
        else:
            return here if here is not None else f"missing: {line}"
        # data_wave's `seen`: only its own rewrites of pair lines make a later one a duplicate
        mo, mm = SYM_LINE.match(line), SYM_LINE.match(here)
        if mo and mo.group(2) in pairs and mm:
            emitted.add((mm.group(2), mm.group(4)[2:].upper()))
        k += 1
    if k != len(got):
        return got[k]
    return None


def changed_files(base: str, new: str | None) -> list[tuple[str, str, str]]:
    args = ["diff", "--no-renames", "--name-status", base] + ([new] if new else [])
    rows = []
    for line in git(*args).splitlines():
        st, path = line.split("\t", 1)
        rows.append((st[0], path))
    if new is None:
        rows += [("A", p) for p in git("ls-files", "--others", "--exclude-standard").splitlines()]
    return rows


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--pairs", default="")
    ap.add_argument("--pairs-file", default="")
    ap.add_argument("--from-manifests", default="",
                    help="comma-separated naming_wave / data_wave --manifest JSON files: the pairs "
                         "are their ops' old names -> new name (the rule's source of the pairs)")
    ap.add_argument("--base", default="HEAD")
    ap.add_argument("--new", default=None, help="revision to check (default: working tree)")
    ap.add_argument("--sub-comments", action="store_true",
                    help="the wave's comment pass: substitute the pairs in the comments of every "
                         "src/ and include/ C file in the working tree, then exit")
    a = ap.parse_args()
    spec = a.pairs + "\n" + (Path(a.pairs_file).read_text(encoding="utf-8") if a.pairs_file else "")
    for mf in filter(None, a.from_manifests.split(",")):
        for op in json.loads(Path(mf).read_text(encoding="utf-8")).get("ops", []):
            new_name = op.get("new_name") or op.get("new")
            for o in op.get("old_names") or op.get("olds") or []:
                if o != new_name:
                    spec += f"\n{o}={new_name}"
    pairs = parse_pairs(spec)
    pat = re.compile(r"\b(" + "|".join(map(re.escape, sorted(pairs, key=len, reverse=True))) + r")\b")
    repl = lambda m: pairs[m.group(1)]  # noqa: E731
    map_path = lambda p: pat.sub(repl, p)  # noqa: E731

    if a.sub_comments:
        hits = git("grep", "-l", "-w", "-E", "|".join(map(re.escape, pairs)), "--", "src", "include",
                   check=False).splitlines()
        for path in hits:
            if path.endswith((".c", ".h")):
                text, n = sub_comments(pat, repl, nw.read(ROOT / path))
                if n:
                    nw.write_lf(ROOT / path, text)
                    print(f"{path}: {n} comment substitution(s)")
        return 0

    fails: list[str] = []
    notes: list[str] = []
    groups: dict[str, list[str]] = {}
    for o, n in pairs.items():
        groups.setdefault(n, []).append(o)
    for n, olds in groups.items():
        if len(olds) > 1:
            addrs = {o: base_addr(a.base, o) for o in olds}
            if None in addrs.values() or len({x.upper() for x in addrs.values()}) != 1:
                fails.append(f"{', '.join(olds)} -> {n}: names of different (or unknown) "
                             f"addresses may not share a new name ({addrs})")
    rows = changed_files(a.base, a.new)
    deleted = {p for st, p in rows if st == "D"}
    added = {p for st, p in rows if st == "A"}
    renames = {}  # new path -> old path
    sources: dict[str, list[str]] = {}
    for d in sorted(deleted):
        if map_path(d) != d and map_path(d) in added:
            sources.setdefault(map_path(d), []).append(d)
    for target, olds in sources.items():
        if len(olds) > 1:  # several aliases' files would land on one path: nothing to compare
            fails.append(f"{target}: paths of a merged name collide ({', '.join(olds)})")
        renames[target] = olds[0]
    work: list[tuple[str, str]] = []  # (old path, new path) to compare
    for st, p in rows:
        if st == "D":
            if map_path(p) not in renames:
                fails.append(f"{p}: deleted, not a rename by the pairs")
        elif st == "A":
            if p in renames:
                work.append((renames[p], p))
            elif p.startswith(METADATA_DIRS):
                notes.append(f"{p}: wave metadata (added)")
            else:
                fails.append(f"{p}: added, not a rename by the pairs")
        else:
            work.append((p, p))

    src_scan: set[str] = set()
    moved_stems: set[str] = set()
    census_changed = False
    for old_path, new_path in work:
        if new_path == SELF or new_path.startswith(DENY_DIRS) or old_path.startswith(DENY_DIRS):
            fails.append(f"{new_path}: rules, agents, the MOVOVL overlay and this tool are never part "
                         f"of a wave")
            continue
        old = show(a.base, old_path)
        new = show(a.new, new_path)
        if old is None or new is None:
            fails.append(f"{new_path}: unreadable")
            continue
        if old_path != new_path:
            notes.append(f"{old_path} -> {new_path}: renamed")
            if new_path.endswith(".s"):
                moved_stems.add(Path(new_path).stem)
        if new_path in METADATA:
            census_changed = True
            continue
        if new_path.startswith(METADATA_DIRS):
            notes.append(f"{new_path}: wave metadata")
            continue
        led = ledger_of(new_path)
        if new_path.startswith("memory/") and (not led or old_path == new_path):
            fails.append(f"{new_path}: memory/ changes only as a ledger the pairs move")
            continue
        if led and Path(new_path).name == nw.LAYER2_RECORD:
            old_func = ledger_of(old_path)[1]
            new_func = led[1]
            expect = old if old_func == new_func else \
                nw.retarget_layer2_record(old, old_func, new_func, old_path)[0]
            if new != expect:
                fails.append(f"{new_path}: layer-2 record differs from the retarget of {old_path}")
            continue
        if led and old_path != new_path:
            if new != old:
                fails.append(f"{new_path}: a moved ledger file must move unchanged")
            continue
        if new_path.endswith((".c", ".h")) and new_path.startswith(("src/", "include/")):
            want = sub_comments(pat, repl, nw.sub_c(pat, repl, old)[0])[0]
            t_want, t_new = ctokens(want), ctokens(new)
            if t_want != t_new:
                fw = formatted(new_path, want)
                if fw is not None and ctokens(fw) == t_new:
                    t_want = t_new  # the wave's text, as the formatter lands it
            if t_want == t_new and layer2.tokens(old) == layer2.tokens(new):
                notes.append(f"{new_path}: comment substitutions only (read them)")
            if t_want != t_new:
                i = next((k for k, (x, y) in enumerate(zip(t_want, t_new)) if x != y),
                         min(len(t_want), len(t_new)))
                fails.append(f"{new_path}: token change at token {i}: "
                             f"{' '.join(t_want[max(0, i - 4):i + 4])!r} -> "
                             f"{' '.join(t_new[max(0, i - 4):i + 4])!r}")
            if new_path.startswith("src/"):
                src_scan.add(new_path)
            continue
        if new_path in REGISTRIES:
            bad = registry_ok(old, new, new_path, pairs, lambda q: show(a.new, q))
            if bad is not None:
                fails.append(f"{new_path}: a line the wave tools do not produce: {bad[:120]}")
            continue
        candidates = {pat.sub(repl, old), nw.sub_c(pat, repl, old)[0], nw.sub_hash(pat, repl, old)[0]}
        if new_path.endswith(".py"):
            candidates.add(nw.sub_py(pat, repl, old)[0])
        if new not in candidates:
            fails.append(f"{new_path}: differs beyond a substitution of the pairs")
        if new_path.endswith(".s"):
            moved_stems.add(Path(new_path).stem)

    if census_changed:
        if a.new is not None:
            fails.append(f"{METADATA[0]}: changed; --new mode cannot re-derive it (check in the tree)")
        else:
            with tempfile.TemporaryDirectory() as td:
                env = dict(os.environ, BUILD_CENSUS_OUTDIR=td)
                subprocess.run([sys.executable, str(ROOT / "docs/naming/build_census.py")], cwd=ROOT,
                               env=env, capture_output=True, check=False)
                fresh = Path(td) / "function-names.csv"
                if not fresh.is_file() or nw.read(fresh) != show(None, METADATA[0]):
                    fails.append(f"{METADATA[0]}: differs from a fresh build_census.py run")
                else:
                    notes.append(f"{METADATA[0]}: equals a fresh build_census.py run")

    # tree-wide: new names must be new; old names must be gone from build files
    news = []
    for o, n in pairs.items():
        if AUTO.match(n):
            addr = base_addr(a.base, o)
            if addr is None or addr.lower() != n[-8:].lower():
                fails.append(f"{o} -> {n}: an auto name must carry {o}'s own address "
                             f"(found {addr or 'none'})")
        else:
            news.append(n)
    if news:
        hits = git("grep", "-l", "-w", "-E", "|".join(news), a.base, "--", "src", "include", "asm",
                   "tools", "engine", "movovl", ".claude", "*.ld", "*.txt", "Makefile", ":!docs", ":!memory",
                   check=False).splitlines()
        for h in hits:
            fails.append(f"new name already in the base tree: {h.split(':', 1)[1]}")
    grep_rev = [a.new] if a.new else ["--untracked"]
    for h in git("grep", "-l", "-w", "-E", "|".join(map(re.escape, pairs)), *grep_rev,
                 check=False).splitlines():
        path = h.split(":", 1)[1] if a.new else h
        build = path.startswith(("src/", "include/", "asm/")) or path.endswith(".ld") \
            or path == "Makefile" or ("/" not in path and path.endswith(".txt"))
        if not build:
            continue
        text = show(a.new, path) or ""
        n = nw.sub_c(pat, repl, text)[1]
        if path.endswith((".c", ".h")) and path.startswith(("src/", "include/")):
            n += sub_comments(pat, repl, text)[1]  # C comments take the pairs too
        if path.endswith(".txt"):  # registries comment with /* */, gate lists with #
            n = min(n, nw.sub_hash(pat, repl, text)[1])
        if n:
            fails.append(f"{path}: an old name survives ({n})")

    # moved layer-2 keys
    if moved_stems:
        for h in git("grep", "-l", "-w", "-E", "|".join(moved_stems), *grep_rev, "--", "src",
                     check=False).splitlines():
            src_scan.add(h.split(":", 1)[1] if a.new else h)
    rev_pairs: dict[str, list[str]] = {}
    for o, n in pairs.items():
        rev_pairs.setdefault(n, []).append(o)
    moved = []
    for path in sorted(src_scan):
        new = show(a.new, path) or ""
        old = show(a.base, path) or ""
        for name in sorted({m.group(1) for m in layer2.inlineasm._INCLUDE_ASM_MACRO_RE.finditer(new)} |
                           {m.group(1) for m in re.finditer(
                               r"(?m)^(?:\}[ \t]*)?[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\(", new)}):
            k_new = layer2.body_key(new, name, lambda p: show(a.new, p))
            k_old, old_name = None, name
            for cand in rev_pairs.get(name, []) + [name]:
                k_old = layer2.body_key(old, cand, lambda p: show(a.base, p))
                if k_old:
                    old_name = cand
                    break
            if k_new and k_old and k_new != k_old:
                moved.append(f"{path}: {name}" + (f" (was {old_name})" if old_name != name else "")
                             + f"  {k_old[1]} -> {k_new[1]}")

    print(f"pairs: {len(pairs)}  changed paths: {len(rows)}")
    for o, n in sorted(pairs.items()):
        print(f"pair {o} -> {n}")
    for f in fails:
        print("FAIL", f)
    for n in notes:
        print("note", n)
    print(f"moved layer-2 keys: {len(moved)}")
    for m in moved:
        print("  ", m)
    print("OK: every change is explained by the pairs" if not fails
          else f"FAIL: {len(fails)} change(s) the pairs do not explain")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
