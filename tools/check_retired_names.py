#!/usr/bin/env python3
"""Check tracked C comments for names retired by naming waves.

Historical manifests and layer-2 records supply the retired names; identifiers
still live in source or symbol registries are excluded. Stale comment names can
repeat rejected claims, violating .claude/rules/naming-bar.md and completion-bar
item 3 (comments assert nothing false).
"""
from __future__ import annotations

import argparse
import contextlib
import csv
import io
import json
import re
import subprocess
import sys
import shutil
import uuid
from pathlib import Path
from unittest.mock import patch

# Import the naming tools' tokenizer without creating files in their directory.
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
from naming_wave import _C_TOKEN

ROOT = Path(__file__).resolve().parent.parent
IDENT = re.compile(r"\b[A-Za-z_]\w*\b")
AUTO = re.compile(r"(?:func_|D_)[0-9a-fA-F]{8}\Z")
REGISTRIES = (
    "named_syms.txt", "symbol_addrs.txt", "undefined_syms_auto.txt",
    "undefined_funcs_auto.txt",
)


def retired_names(root: Path) -> set[str]:
    names = set()
    for path in (root / "docs/naming").rglob("*manifest*.csv"):
        with path.open(encoding="utf-8-sig", newline="") as stream:
            for row in csv.DictReader(stream):
                for column in ("current_name", "current_names"):
                    names.update(name for name in re.split(
                        r"[;,\s]+", row.get(column) or "")
                        if name != row.get("proposed_name"))
    reset = root / "docs/naming/reset-wave-dryrun.json"
    if reset.exists():
        for op in json.loads(reset.read_text(encoding="utf-8")).get("ops", []):
            names.update(name for name in op.get("old_names", [])
                         if name != op.get("new_name"))
    for path in (root / "memory").rglob("layer2.jsonl"):
        for line in path.read_text(encoding="utf-8").splitlines():
            try:
                record = json.loads(line)
            except ValueError:
                continue
            if not isinstance(record, dict):
                continue
            old = record.get("renamed_from", [])
            if isinstance(old, str):
                old = [old]
            if isinstance(old, list):
                names.update(name for name in old if isinstance(name, str))
    return {name for name in names if IDENT.fullmatch(name)
            and ("_" in name or re.search(r"[a-z][A-Z]", name))
            and not AUTO.fullmatch(name)}


def tracked_sources(root: Path) -> list[str]:
    result = subprocess.run(["git", "ls-files", "-z", "--", "src", "include"],
                            cwd=root, check=True, stdout=subprocess.PIPE)
    return sorted(path for path in result.stdout.decode("utf-8").split("\0")
                  if path.startswith(("src/", "include/"))
                  and Path(path).suffix in (".c", ".h"))


def scan(root: Path) -> tuple[int, int, list[tuple[str, int, str]]]:
    known = retired_names(root)
    live = set()
    comments = []
    for path in tracked_sources(root):
        text = (root / path).read_text(encoding="utf-8")
        pos = 0
        for token in _C_TOKEN.finditer(text):
            live.update(IDENT.findall(text[pos:token.start()]))
            if token.group().startswith(("//", "/*")):
                comments.append((path, text, token))
            pos = token.end()
        live.update(IDENT.findall(text[pos:]))
    for filename in REGISTRIES:
        path = root / filename
        if path.exists():
            live.update(re.findall(r"^\s*([A-Za-z_]\w*)\s*=\s*0x[0-9a-fA-F]+\s*;",
                                  path.read_text(encoding="utf-8"), re.M))
    retired = known - live
    hits = set()
    for path, text, token in comments:
        for name in IDENT.finditer(token.group()):
            if name.group() in retired:
                line = text.count("\n", 0, token.start() + name.start()) + 1
                hits.add((path, line, name.group()))
    return len(known), len(known & live), sorted(hits)


@contextlib.contextmanager
def fixture_directory():
    # Default directory permissions avoid Windows sandbox ACL issues with
    # tempfile's restrictive mode=0o700 directories.
    scratch = ROOT / "tmp"
    scratch.mkdir(exist_ok=True)
    root = scratch / ("retired-names-" + uuid.uuid4().hex)
    root.mkdir()
    try:
        yield root
    finally:
        shutil.rmtree(root)


def self_test() -> int:
    with fixture_directory() as root:
        (root / "docs/naming").mkdir(parents=True)
        (root / "src").mkdir()
        (root / "docs/naming/test-manifest.csv").write_text(
            "current_name,proposed_name\nold_name,new_name\nlive_name,new_live\n"
            "registry_name,new_registry\ncopy,new_copy\n", encoding="utf-8")
        (root / "src/test.c").write_text(
            '/* old_name */\n// old_name\nchar *s = "old_name";\n'
            'int live_name; /* live_name */\n/* copy */\n/* registry_name */\n',
            encoding="utf-8")
        (root / "named_syms.txt").write_text(
            "registry_name = 0x80010000;\n", encoding="utf-8")
        # Supply the fixture's tracked inventory without creating a Git repository.
        with patch(__name__ + ".tracked_sources", return_value=["src/test.c"]):
            known, dropped, hits = scan(root)
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                status = main(["--root", str(root)])
        cases = [
            ("block comment", ("src/test.c", 1, "old_name") in hits),
            ("line comment", ("src/test.c", 2, "old_name") in hits),
            ("string literal skipped", not any(line == 3 for _, line, _ in hits)),
            ("live identifier excluded", not any(n == "live_name" for _, _, n in hits)),
            ("English word excluded", "copy" not in retired_names(root)),
            ("registry definition excluded", dropped == 2 and known == 3),
            ("--root and hit exit status", status == 1 and len(hits) == 2
             and "hits: 2" in output.getvalue()),
        ]
        for label, passed in cases:
            print(f"{'PASS' if passed else 'FAIL'}: {label}")
        return int(not all(passed for _, passed in cases))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args(argv)
    if args.self_test:
        return self_test()
    known, dropped, hits = scan(args.root.resolve())
    for path, line, name in hits:
        print(f"{path}:{line}: {name}")
    print(f"Retired names known: {known}; live names dropped: {dropped}; hits: {len(hits)}")
    return int(bool(hits))


if __name__ == "__main__":
    sys.exit(main())
