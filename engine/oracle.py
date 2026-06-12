"""The oracle: the immutable byte-match ground truth + drift/verification.

The oracle is the regression anchor for the entire greenfield rebuild. Under a
"full clean slate", the ONE thing we never gamble is the proof that the tree
builds the original binary. `oracle/manifest.json` freezes:

  * the original EXE's SHA1 + size (the target)
  * the expected build SHA1 (must equal the original)
  * the git commit it was locked at
  * toolchain identity (cc1 hash, maspsx rev, as/python versions)
  * per-file hashes of the matched C corpus
  * per-file hashes of the pipeline config (regfix/asmfix/sdata/... )
  * the golden fixtures (pure-C, zero-cheat functions a tool-health gate
    re-derives in seconds)

`engine verify-oracle` reports drift against the manifest and confirms the
current tree still builds byte-identical. Nothing in later phases is trusted
until this is green.
"""
from __future__ import annotations

import json
import subprocess
from pathlib import Path

from . import buildconfig as cfg
from . import pipeline as P

ORACLE_DIR = Path("oracle")
MANIFEST = ORACLE_DIR / "manifest.json"

# Pipeline-config files whose contents affect codegen. Drift here means the
# build inputs changed since the oracle was locked.
CONFIG_FILES = [
    "regfix.txt", "regfix_stage2.txt", "asmfix.txt",
    "sdata_syms.txt", "sdata_funcs.txt", "sdata_exclude.txt",
    "expand_lb_funcs.txt", "multu_funcs.txt", "multu_pad_funcs.txt",
    "expand_dest_funcs.txt", "named_syms.txt",
    "undefined_funcs_auto.txt", "undefined_syms_auto.txt",
    "bb2.ld", "tools/prologue_config.json",
]


def _git(args: list[str]) -> str:
    return subprocess.run(["git"] + args, capture_output=True, text=True).stdout.strip()


def _toolchain_identity() -> dict:
    cc1 = Path(cfg.CC1)
    return {
        "cc1_sha1": P.sha1(cc1) if cc1.exists() else None,
        "maspsx_rev": subprocess.run(
            ["git", "-C", "tools/maspsx", "rev-parse", "HEAD"],
            capture_output=True, text=True).stdout.strip() or None,
        "as_version": subprocess.run(
            ["bash", "-c", "mipsel-linux-gnu-as --version | head -1"],
            capture_output=True, text=True).stdout.strip(),
        "python": subprocess.run(
            ["python3", "--version"], capture_output=True, text=True
        ).stdout.strip(),
    }


def lock(fixtures: list | None = None, git_commit: str | None = None) -> dict:
    """Capture the current tree as the oracle manifest. Pass `fixtures` to
    preserve/replace the golden-fixture list (defaults to whatever is already
    locked, so re-locking after a corpus change doesn't drop them).

    `git_commit` overrides the recorded commit. Needed because WSL git cannot
    read this worktree's `.git` file (it points at a Windows-style gitdir path);
    the orchestrator captures the commit from Windows git and passes it here.
    """
    ORACLE_DIR.mkdir(exist_ok=True)
    if fixtures is None and MANIFEST.exists():
        fixtures = json.loads(MANIFEST.read_text()).get("golden_fixtures", [])
    if git_commit is None:
        git_commit = _git(["rev-parse", "HEAD"]) or None
    manifest = {
        "schema": 1,
        "original_exe": {
            "path": cfg.TARGET_EXE,
            "sha1": P.sha1(cfg.TARGET_EXE),
            "size": Path(cfg.TARGET_EXE).stat().st_size,
        },
        "expected_build_sha1": cfg.ORACLE_SHA1,
        "git_commit": git_commit,
        "toolchain": _toolchain_identity(),
        "corpus": {f"src/{s}.c": P.sha1(f"src/{s}.c") for s in P.c_stems()},
        "config_files": {p: P.sha1(p) for p in CONFIG_FILES if Path(p).exists()},
        "golden_fixtures": fixtures or [],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n")
    return manifest


def dirty_build_inputs() -> list[str]:
    """Uncommitted (tracked) modifications to files that feed the build.

    A `verify-oracle --rebuild` with these dirty is the documented
    reference-corruption footgun (CLAUDE.md "Reference gotcha"): it rebuilds
    build/ from half-finished source, and the sandbox then scores against
    garbage. The CLI refuses --rebuild while this list is non-empty unless
    --allow-dirty is passed (legitimate e.g. mid-revert, where the dirty
    state IS the intended new reference).
    """
    out = _git(["status", "--porcelain", "--untracked-files=no"])
    watch_prefixes = ("src/", "include/")
    watch_files = set(CONFIG_FILES) | {"Makefile", "maspsx_label_nop_funcs.txt"}
    dirty = []
    for line in out.splitlines():
        path = line[3:].strip().strip('"')
        if " -> " in path:  # rename: report the new side
            path = path.split(" -> ", 1)[1]
        p = path.replace("\\", "/")
        if p.startswith(watch_prefixes) or p in watch_files:
            dirty.append(p)
    return dirty


def verify(rebuild: bool = False) -> dict:
    """Compare the current tree to the locked manifest and confirm a
    byte-identical build. `rebuild` forces a fresh full build first.
    """
    if not MANIFEST.exists():
        return {"ok": False, "error": "no oracle manifest; run `engine oracle-lock`"}
    man = json.loads(MANIFEST.read_text())

    drift = {"corpus": [], "config": [], "toolchain": []}
    for f, h in man["corpus"].items():
        if (P.sha1(f) if Path(f).exists() else None) != h:
            drift["corpus"].append(f)
    for f, h in man["config_files"].items():
        if (P.sha1(f) if Path(f).exists() else None) != h:
            drift["config"].append(f)
    cur_tc = _toolchain_identity()
    for k, v in man["toolchain"].items():
        if cur_tc.get(k) != v:
            drift["toolchain"].append(k)

    exe = Path("build/bb2.exe")
    if rebuild or not exe.exists():
        P.build_all()
    build_sha1 = P.sha1("build/bb2.exe")
    build_matches = build_sha1 == man["expected_build_sha1"]

    return {
        "ok": build_matches,
        "build_sha1": build_sha1,
        "expected": man["expected_build_sha1"],
        "build_matches": build_matches,
        "original_sha1_now": P.sha1(cfg.TARGET_EXE),
        "original_sha1_locked": man["original_exe"]["sha1"],
        "drift": {k: v for k, v in drift.items() if v},
        "locked_at_commit": man["git_commit"],
        "golden_fixtures": man.get("golden_fixtures", []),
    }
