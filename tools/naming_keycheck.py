#!/usr/bin/env python3
"""naming_keycheck.py — prove a naming wave changed identifiers only (.claude/rules/naming-bar.md).

Compares the working tree (or --new <rev>) with --base <rev> (default HEAD). Fails closed: every
changed, added, deleted or renamed file must be explained by the wave's old->new pairs, using the
same substitutions the wave tools apply (tools/naming_wave.py, tools/data_wave.py):

  src/**, include/** *.c *.h  ctokens(new) == ctokens(old with the pairs substituted in code,
                              string literals (escape-aware) and comments): C tokens plus each
                              comment at its place and `#define NAME(` adjacency. Files whose
                              only change is in comments are listed for the reviewer.
  memory/**/layer2.jsonl      byte-equal to naming_wave.retarget_layer2_record(old) for its ledger's
                              rename (func retargeted, renamed_from extended, verdict_file moved).
  other files in a moved ledger directory          byte-identical (a pure move).
  docs/naming/function-names.csv  must equal a fresh build_census.py run (working-tree mode;
                              it reads build/bb2.map, so run after verify-oracle --rebuild).
                              In --new mode a changed census FAILS (re-derive it in the tree).
  docs/naming/phase3/**       wave metadata: allowed, listed.
  .claude/**, this tool       any change FAILS.
  any other text file         equal to one of: whole-word substitution, sub_c, sub_hash, sub_py.
A path rename must be the old path with the pairs substituted; an added or deleted file must be
one side of such a rename. Two tree-wide checks: a non-auto new name must not already occur in
the base tree (an auto name must carry the old name's address), and no old name may survive in
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
DENY_DIRS = (".claude/",)
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
    if len(set(pairs.values())) != len(pairs):
        sys.exit("two old names map to one new name")
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


def base_addr(base: str, name: str) -> str | None:
    """`name`'s address in the base tree (engine.layer2.addr_at: glabel file, registries, census)."""
    return layer2.addr_at(name, lambda p: show(base, p))


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
    ap.add_argument("--base", default="HEAD")
    ap.add_argument("--new", default=None, help="revision to check (default: working tree)")
    ap.add_argument("--sub-comments", action="store_true",
                    help="the wave's comment pass: substitute the pairs in the comments of every "
                         "src/ and include/ C file in the working tree, then exit")
    a = ap.parse_args()
    spec = a.pairs + "\n" + (Path(a.pairs_file).read_text(encoding="utf-8") if a.pairs_file else "")
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
    rows = changed_files(a.base, a.new)
    deleted = {p for st, p in rows if st == "D"}
    added = {p for st, p in rows if st == "A"}
    renames = {}  # new path -> old path
    for d in deleted:
        if map_path(d) != d and map_path(d) in added:
            renames[map_path(d)] = d
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
            fails.append(f"{new_path}: changes to rules, agents or this tool are never part of a wave")
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
        hits = git("grep", "-l", "-w", "-E", "|".join(news), a.base, check=False).splitlines()
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
    rev_pairs = {n: o for o, n in pairs.items()}
    moved = []
    for path in sorted(src_scan):
        new = show(a.new, path) or ""
        old = show(a.base, path) or ""
        for name in sorted({m.group(1) for m in layer2.inlineasm._INCLUDE_ASM_MACRO_RE.finditer(new)} |
                           {m.group(1) for m in re.finditer(
                               r"(?m)^(?:\}[ \t]*)?[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\(", new)}):
            old_name = rev_pairs.get(name, name)
            k_new = layer2.body_key(new, name, lambda p: show(a.new, p))
            k_old = layer2.body_key(old, old_name, lambda p: show(a.base, p))
            if k_new and k_old and k_new != k_old:
                moved.append(f"{path}: {name}" + (f" (was {old_name})" if old_name != name else "")
                             + f"  {k_old[1]} -> {k_new[1]}")

    print(f"pairs: {len(pairs)}  changed paths: {len(rows)}")
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
