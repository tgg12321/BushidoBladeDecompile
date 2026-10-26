#!/usr/bin/env python3
"""move_tu.py OLD NEW [--apply]: move or rename ONE C translation unit.

A TU id is the path under src/ without .c (engine/tus.py). One move per call,
dry-run by default (prints the plan); --apply performs it. Refuses on a dirty
tree, so the move is the whole of the next commit's diff.

Every surface keyed by the id follows the file:
  - git mv src/OLD.c src/NEW.c
  - bb2.ld: every `build/src/OLD.o(<section>);` line, in place (link order kept)
  - Makefile per-file lists and engine/buildconfig.py sets (GP / PSYQ_LIBRARY /
    EXPAND_LH / NO_SR), in place
  - tools/canonical_asm_regions.json `file` (the islands are re-hashed from the
    moved file first; a mismatch refuses)
  - oracle/manifest.json corpus key and golden-fixture `file` (hash kept: the
    bytes did not change)
  - tools/cc1_tu_expectation.txt TU key, engine/queue.json item `file`
  - tools/lanes/lanes.json regenerate paths, commands, subjects and inputs
    (existing generated evidence files move byte-for-byte with their paths)
  - exact `"src/OLD.c"` entries in .claude/rules/*.md `paths:`
  - tools/tu_renames.tsv: one appended row (old id, new id, `after:<HEAD>`)
Any other tracked reference to the old id outside docs/ and memory/ (historical
citations stay as written, Q106 D9) is listed for review, never rewritten.

Moving a file is byte-neutral by construction (no __FILE__ / .incbin; includes
resolve through -Iinclude); prove it anyway: clean build + verify-oracle --rebuild.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

FLAG_LISTS = ("GP_FILES", "PSYQ_LIBRARY_FILES", "DATA_ONLY_NO_SDATA_FILES", "EXPAND_LH_FILES", "NO_SR_FILES", "NOWARN_FILES")
_ID_RE = re.compile(r"^[A-Za-z0-9_][A-Za-z0-9_.\-]*(/[A-Za-z0-9_][A-Za-z0-9_.\-]*)*$")
RENAMES_HEADER = ("# TU renames (restructure, owner decision Q106 D9): old id, new id, and the\n"
                  "# HEAD the move was made on (`after:<sha>`; the move is the next commit that\n"
                  "# touches this row). Ids are paths under src/ without .c. Resolve a\n"
                  "# historical src/<id>.c citation by following old -> new (engine/tus.py).\n")


class MoveError(Exception):
    pass


def norm_id(v: str) -> str:
    v = v.replace("\\", "/")
    if v.startswith("src/"):
        v = v[4:]
    if v.endswith(".c"):
        v = v[:-2]
    if not _ID_RE.match(v) or ".." in v.split("/"):
        raise MoveError(f"not a TU id: {v!r}")
    return v


def git(root: Path, *args: str, check: bool = True) -> str:
    r = subprocess.run(["git", *args], cwd=root, capture_output=True, text=True,
                       encoding="utf-8", errors="replace")
    if check and r.returncode != 0:
        raise MoveError(f"git {' '.join(args)} failed: {r.stderr.strip()}")
    return r.stdout


def dirty(root: Path) -> list[str]:
    return [line for line in git(root, "status", "--porcelain").splitlines() if line.strip()]


def _sub_exact(text: str, old: str, new: str, want: int | None, what: str) -> tuple[str, int]:
    n = text.count(old)
    if want is not None and n != want:
        raise MoveError(f"{what}: expected {want} occurrence(s) of {old!r}, found {n}")
    return text.replace(old, new), n


def plan(root: Path, old: str, new: str) -> tuple[dict[str, bytes], dict[str, str], list[str], list[str]]:
    """(edits, generated file moves, notes, leftover references); raises MoveError."""
    from engine import tus
    src_old, src_new = root / "src" / f"{old}.c", root / "src" / f"{new}.c"
    if old == new:
        raise MoveError("OLD and NEW are the same id")
    if not src_old.is_file():
        raise MoveError(f"src/{old}.c does not exist")
    if src_new.exists():
        raise MoveError(f"src/{new}.c already exists")
    ld = (root / "bb2.ld").read_text(encoding="utf-8")
    pre = tus.check(root / "bb2.ld", root / "src", text=ld, lists=_buildconfig_lists(root))
    if pre:
        raise MoveError("bb2.ld/flag lists inconsistent BEFORE the move:\n  " + "\n  ".join(pre))
    sections = tus.ld_sections(text=ld)
    nsec = sum(ids.count(old) for ids in sections.values())
    if not nsec:
        raise MoveError(f"bb2.ld does not link build/src/{old}.o")

    edits: dict[str, bytes] = {}
    notes: list[str] = []
    moves: dict[str, str] = {}
    old_note = f"docs/naming/runtime/evidence/{old.replace('/', '__')}.tsv"

    def put(rel: str, text: str) -> None:
        edits[rel] = text.encode("utf-8")

    # bb2.ld: every section line, in place
    t, n = _sub_exact(ld, f"build/src/{old}.o(", f"build/src/{new}.o(", nsec, "bb2.ld")
    put("bb2.ld", t)
    notes.append(f"bb2.ld: {n} section line(s)")

    # Makefile per-file lists
    mk = (root / "Makefile").read_text(encoding="utf-8")
    hit = []

    def mk_line(m: re.Match) -> str:
        toks = m.group(2).split()
        if old not in toks:
            return m.group(0)
        hit.append(m.group(1))
        return m.group(1) + " :=" + "".join(" " + (new if x == old else x) for x in toks)
    mk2 = re.sub(r"^(" + "|".join(FLAG_LISTS) + r") :=(.*)$", mk_line, mk, flags=re.M)
    if hit:
        put("Makefile", mk2)
    # engine/buildconfig.py sets
    bc = (root / "engine/buildconfig.py").read_text(encoding="utf-8")
    bhit = []

    def bc_line(m: re.Match) -> str:
        if f'"{old}"' not in m.group(0):
            return m.group(0)
        bhit.append(m.group(1))
        return m.group(0).replace(f'"{old}"', f'"{new}"')
    bc2 = re.sub(r"^(" + "|".join(FLAG_LISTS) + r") = \{.*\}$", bc_line, bc, flags=re.M)
    if sorted(hit) != sorted(bhit):
        raise MoveError(f"Makefile lists {sorted(hit)} and engine/buildconfig.py sets {sorted(bhit)} "
                        f"disagree on {old!r}: fix the mirror first")
    if bhit:
        put("engine/buildconfig.py", bc2)
    notes.append(f"flag lists: {', '.join(hit) or 'none'}")

    # canonical-asm region grants: re-hash the islands from the file before relocating
    regions = root / "tools/canonical_asm_regions.json"
    if regions.exists():
        data = json.loads(regions.read_text(encoding="utf-8"))
        moved = [f for f, g in data.get("functions", {}).items() if g.get("file") == old]
        if moved:
            from engine import completion
            text = src_old.read_text(encoding="utf-8")
            for f in moved:
                try:
                    hashes = completion.region_hashes(text, f)
                except ValueError as exc:
                    raise MoveError(f"canonical_asm_regions: {f} in src/{old}.c: {exc}; "
                                    "refusing to relocate its grant") from exc
                if hashes != data["functions"][f]["sha256"]:
                    raise MoveError(f"canonical_asm_regions: {f}'s islands in src/{old}.c no longer "
                                    "hash to the reviewed grant; refusing to relocate it")
                data["functions"][f]["file"] = new
            put("tools/canonical_asm_regions.json", json.dumps(data, indent=2) + "\n")
        notes.append(f"canonical_asm_regions: {len(moved)} grant(s)")

    # oracle manifest: corpus key (sorted as `oracle-lock` writes it) + golden fixtures
    man_p = root / "oracle/manifest.json"
    if man_p.exists():
        man = json.loads(man_p.read_text(encoding="utf-8"))
        corpus = man.get("corpus", {})
        k_old, k_new = f"src/{old}.c", f"src/{new}.c"
        changed = 0
        if k_old in corpus:
            corpus[k_new] = corpus.pop(k_old)
            man["corpus"] = {k: corpus[k] for k in sorted(corpus, key=lambda k: k[4:-2])}
            changed += 1
        for fx in man.get("golden_fixtures", []):
            if fx.get("file") == k_old:
                fx["file"] = k_new
                changed += 1
        if changed:
            put("oracle/manifest.json", json.dumps(man, indent=2) + "\n")
        notes.append(f"oracle/manifest.json: {changed} key(s)")

    # cc1 expectation manifest: `<sha1>  <id>` lines
    exp = root / "tools/cc1_tu_expectation.txt"
    if exp.exists():
        t = exp.read_text(encoding="utf-8")
        t2, n = re.subn(r"^([0-9a-f]{40}  )" + re.escape(old) + r"$", r"\g<1>" + new, t, flags=re.M)
        if n:
            put("tools/cc1_tu_expectation.txt", t2)
        notes.append(f"cc1_tu_expectation: {n} line(s)")

    # queue items
    qp = root / "engine/queue.json"
    if qp.exists():
        q = json.loads(qp.read_text(encoding="utf-8"))
        n = 0
        for it in q.get("items", []):
            if it.get("file") == old:
                it["file"] = new
                n += 1
        if n:
            put("engine/queue.json", json.dumps(q, indent=2) + "\n")
        notes.append(f"queue.json: {n} item(s)")

    # Lane regenerate entries are hand-formatted: edit only the keyed strings,
    # retaining whitespace and all unrelated configuration verbatim.
    lp = root / "tools/lanes/lanes.json"
    if lp.exists():
        t = lp.read_bytes().decode("utf-8")
        m = re.search(r'"regenerate"\s*:\s*(?=\[)', t)
        if m:
            _, end = json.JSONDecoder().raw_decode(t, m.end())
            block = t[m.end():end]
            new_note = f"docs/naming/runtime/evidence/{new.replace('/', '__')}.tsv"
            block, paths = re.subn(r'("path"\s*:\s*)"' + re.escape(old_note) + r'"',
                                   lambda m: m[1] + f'"{new_note}"', block)
            cmds = 0

            def lane_command(m: re.Match) -> str:
                nonlocal cmds
                command = json.loads(m[2])
                raw = m[2][1:-1]
                # Map decoded character boundaries to the original JSON spelling.
                offsets = [0]
                for char in command:
                    pos = offsets[-1]
                    size = 1
                    if raw[pos] == "\\":
                        size = 6 if raw[pos + 1] == "u" else 2
                        if size == 6 and ord(char) > 0xFFFF:
                            size = 12  # a JSON surrogate pair
                    offsets.append(pos + size)
                args = list(re.finditer(
                    r"(?<!\S)(--tu(?:\s+|=))(?:(['\"])" + re.escape(old)
                    + r"\2|" + re.escape(old) + r")(?=\s|$)",
                    command))
                for arg in reversed(args):
                    start = arg.end(1) + bool(arg[2])
                    end = start + len(old)
                    if (raw[offsets[start]:offsets[end]] != old
                            or raw[offsets[arg.start(1)]:offsets[arg.end(1)]] != arg[1]):
                        raise MoveError("lanes.json cmd: escaped --tu argument; refusing to rewrite")
                    raw = raw[:offsets[start]] + new + raw[offsets[end]:]
                cmds += len(args)
                return m[1] + '"' + raw + '"' if args else m[0]

            block = re.sub(r'("cmd"\s*:\s*)("(?:\\.|[^"\\])*")',
                           lane_command, block)
            block, subjects = re.subn(r'("subject"\s*:\s*"[^"\n]*?)\(' + re.escape(old)
                                      + r'\)', lambda m: m[1] + f'({new})', block)
            inputs = 0

            def lane_inputs(m: re.Match) -> str:
                nonlocal inputs
                text, n = _sub_exact(m[0], f'"src/{old}.c"', f'"src/{new}.c"',
                                     None, "lanes.json inputs")
                inputs += n
                return text
            block = re.sub(r'"inputs"\s*:\s*\[[^\]]*\]', lane_inputs, block)
            if paths + cmds + subjects + inputs:
                put("tools/lanes/lanes.json", t[:m.end()] + block + t[end:])
            notes.append(f"lanes.json regenerate: {paths} path(s), {cmds} command(s), "
                         f"{subjects} subject(s), {inputs} input(s)")
            if paths and (root / old_note).is_file():
                if (root / new_note).exists():
                    raise MoveError(f"{new_note} already exists")
                moves[old_note] = new_note
                notes.append(f"generated evidence: {old_note} -> {new_note}")

    # rule frontmatter `paths:` naming the file exactly
    rules = 0
    for p in sorted((root / ".claude/rules").glob("*.md")):
        t = p.read_text(encoding="utf-8")
        m = re.match(r"^---\n(.*?)\n---", t, re.S)
        if m and f'"src/{old}.c"' in m.group(1):
            fm = m.group(1).replace(f'"src/{old}.c"', f'"src/{new}.c"')
            put(p.relative_to(root).as_posix(), t[:m.start(1)] + fm + t[m.end(1):])
            rules += 1
    notes.append(f"rule paths: {rules} file(s)")

    # the rename map
    rn = root / "tools/tu_renames.tsv"
    head = git(root, "rev-parse", "--short=12", "HEAD").strip()
    base = rn.read_text(encoding="utf-8") if rn.exists() else RENAMES_HEADER
    put("tools/tu_renames.tsv", base + f"{old}\t{new}\tafter:{head}\n")

    # after the move the layout must still be consistent
    new_ld = edits["bb2.ld"].decode("utf-8")
    new_lists = _buildconfig_lists(root, edits.get("engine/buildconfig.py"))
    post = [p for p in tus.check(root / "bb2.ld", root / "src", text=new_ld, lists=new_lists)
            if f"src/{old}.c" not in p and f"src/{new}.c" not in p]
    if post:
        raise MoveError("the move would leave bb2.ld/flag lists inconsistent:\n  " + "\n  ".join(post))

    # leftovers: tracked references outside docs/memory that this tool does not own
    pat = (r"(^|[^A-Za-z0-9_/]|src/)" + re.escape(old)
           + r"(\.[co]([^A-Za-z0-9_]|$)|[^A-Za-z0-9_./]|$)")
    out = git(root, "grep", "-n", "-E", pat, "--", ".", ":!docs", ":!memory",
              ":!src", ":!bb2.ld", ":!tools/tu_renames.tsv", check=False)
    leftovers = [l for l in out.splitlines() if l.split(":", 1)[0] not in edits]
    # A rewritten lanes file can still contain unrelated references, including
    # ones on the same line as a rewritten field. Scan the resulting text.
    if "tools/lanes/lanes.json" in edits:
        leftovers.extend(f"tools/lanes/lanes.json:{i}:{line}"
                         for i, line in enumerate(
                             edits["tools/lanes/lanes.json"].decode("utf-8").splitlines(), 1)
                         if re.search(pat, line))
    if (root / old_note).is_file() and old_note not in moves:
        leftovers.append(f"generated evidence with no regenerate entry: {old_note}")
    return edits, moves, notes, leftovers


def _buildconfig_lists(root: Path, text: bytes | None = None) -> dict[str, set]:
    t = text.decode("utf-8") if text is not None else \
        (root / "engine/buildconfig.py").read_text(encoding="utf-8")
    out = {}
    for name in FLAG_LISTS:
        m = re.search(r"^" + name + r" = (\{.*\}|set\(\))$", t, re.M)
        if not m:
            raise MoveError(f"engine/buildconfig.py: no `{name} = {{...}}` line")
        out[name] = set(re.findall(r'"([^"]+)"', m.group(1)))
    return out


def apply(root: Path, old: str, new: str, edits: dict[str, bytes], moves: dict[str, str]) -> None:
    (root / "src" / f"{new}.c").parent.mkdir(parents=True, exist_ok=True)
    git(root, "mv", f"src/{old}.c", f"src/{new}.c")
    for old_path, new_path in moves.items():
        (root / new_path).parent.mkdir(parents=True, exist_ok=True)
        git(root, "mv", old_path, new_path)
    for rel, data in edits.items():
        (root / rel).write_bytes(data)
    git(root, "add", "--", *edits)
    # drop directories the move emptied
    d = (root / "src" / f"{old}.c").parent
    while d != root / "src" and d.is_dir() and not any(d.iterdir()):
        d.rmdir()
        d = d.parent


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("old")
    ap.add_argument("new")
    ap.add_argument("--apply", action="store_true", help="perform the move (default: dry run)")
    ap.add_argument("--root", default=str(ROOT), help=argparse.SUPPRESS)
    a = ap.parse_args(argv)
    root = Path(a.root).resolve()
    try:
        old, new = norm_id(a.old), norm_id(a.new)
        d = dirty(root)
        if d:
            raise MoveError("dirty tree; commit or stash first:\n  " + "\n  ".join(d[:20]))
        edits, moves, notes, leftovers = plan(root, old, new)
    except MoveError as e:
        print(f"move_tu: REFUSED: {e}", file=sys.stderr)
        return 1
    print(f"move_tu: src/{old}.c -> src/{new}.c ({'APPLY' if a.apply else 'dry run'})")
    for n in notes:
        print(f"  {n}")
    print(f"  files rewritten: {', '.join(sorted(edits))}")
    if leftovers:
        print("  other tracked references to the old id (review by hand; not rewritten):")
        leftovers = sorted(leftovers, key=lambda l: not l.startswith(
            "generated evidence with no regenerate entry:"))
        for l in leftovers[:40]:
            print(f"    {l[:160]}")
        if len(leftovers) > 40:
            print(f"    ... and {len(leftovers) - 40} more")
    if not a.apply:
        print("  (dry run: pass --apply to perform)")
        return 0
    apply(root, old, new, edits, moves)
    print("  applied and staged. Next: clean build + `verify-oracle --rebuild`, then commit.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
