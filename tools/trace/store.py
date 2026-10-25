"""Bulk capture store keyed by the capture path relative to this repo root.

Example: <store>/docs/naming/runtime/family/baseline/capture.jsonl.
External artifacts require an outputs SHA1 and byte count; local reads retain
their historical behavior.
"""
import hashlib
import json
import os
from pathlib import Path, PureWindowsPath
import subprocess
import sys

REPO_ROOT = Path(__file__).resolve().parents[2]
_warned_git_fallback = False


class StoreError(Exception):
    """A capture artifact cannot safely be supplied."""


class StoreMissing(StoreError, FileNotFoundError):
    """An artifact is missing from both locations."""


class StoreUnverified(StoreError, ValueError):
    """An external artifact lacks trustworthy manifest metadata."""


class StoreMismatch(StoreUnverified):
    """An external artifact disagrees with its manifest."""


def environment_path(override, windows_host):
    """Normalize Windows drive overrides on WSL; refuse ambiguous paths."""
    windows = PureWindowsPath(override)
    if not windows_host:
        if windows.drive:
            if not windows.is_absolute() or len(windows.drive) != 2:
                raise StoreUnverified("BB2_CAPTURE_STORE needs an absolute drive path: " + override)
            return Path("/mnt") / windows.drive[0].lower() / Path(*windows.parts[1:])
        if "\\" in override:
            raise StoreUnverified("BB2_CAPTURE_STORE needs an absolute drive path: " + override)
    return Path(override)


def store_root():
    global _warned_git_fallback
    override = os.environ.get("BB2_CAPTURE_STORE")
    if override:
        return environment_path(override, os.name == "nt")
    try:
        result = subprocess.run(
            ["git", "-C", str(REPO_ROOT), "rev-parse", "--git-common-dir"],
            check=True, capture_output=True, text=True, encoding="utf-8")
        if not result.stdout.strip():
            raise ValueError("empty git common directory")
        common = Path(result.stdout.strip())
        if not common.is_absolute():
            common = REPO_ROOT / common
        main = common.resolve().parent
    except (OSError, subprocess.SubprocessError, ValueError) as exc:
        if not _warned_git_fallback:
            print(f"capture store: warning: git rev-parse --git-common-dir failed "
                  f"({exc}); using this repository's sibling store", file=sys.stderr)
            _warned_git_fallback = True
        main = REPO_ROOT
    return main.parent / "bb2-captures"


def is_bulk(name):
    """Bulk means capture.jsonl, PNG screenshots, script logs, or card copies."""
    name = Path(name).name
    return name == "capture.jsonl" or name.endswith((".png", ".script.log", ".mcd"))


def verify(path, manifest_rec):
    """Require and check SHA1 plus runkit.file_rec's size (or bytes)."""
    size = manifest_rec.get("size", manifest_rec.get("bytes"))
    sha1 = manifest_rec.get("sha1")
    if not isinstance(sha1, str) or len(sha1) != 40 or type(size) is not int or size < 0:
        raise StoreUnverified(f"{path}: outputs record requires SHA1 and byte count")
    data = Path(path).read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    if digest != sha1 or len(data) != size:
        raise StoreMismatch(f"{path}: capture store SHA1/size mismatch "
                            f"(expected {sha1}/{size}, got {digest}/{len(data)})")


def _records(manifest, name):
    if not isinstance(manifest, dict) or not isinstance(manifest.get("outputs"), list):
        return []
    return [rec for rec in manifest["outputs"] if isinstance(rec, dict)
            and isinstance(rec.get("path"), str)
            and PureWindowsPath(rec["path"]).name == name]


def _manifest(capture_dir, name):
    stem = name[:-6] if name.endswith(".jsonl") else "capture"
    own = capture_dir / (stem + ".manifest.json")
    candidates = [own] + [p for p in sorted(capture_dir.glob("*.manifest.json")) if p != own]
    for path in candidates:
        try:
            manifest = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, ValueError):
            if path == own and path.exists() and name.endswith(".jsonl"):
                raise StoreUnverified(f"{path}: invalid capture manifest") from None
            continue
        if _records(manifest, name):
            return manifest
        if path == own and name.endswith(".jsonl"):
            return manifest
    raise StoreUnverified(f"{capture_dir}: no manifest outputs record for {name}")


def resolve(capture_dir, name, manifest=None, allow_local=True):
    """Prefer local artifacts; require verified outputs metadata for store hits.

    Index readers disable allow_local so an untracked working file cannot
    substitute for an indexed capture.
    """
    if not isinstance(name, str) or name in ("", ".", "..") or "/" in name or "\\" in name or PureWindowsPath(name).drive:
        raise StoreUnverified(f"invalid capture filename: {name!r}")
    capture_dir = Path(capture_dir).resolve()
    local = capture_dir / name
    if allow_local and local.is_file():
        return local
    try:
        key = capture_dir.relative_to(REPO_ROOT.resolve())
    except ValueError:
        raise StoreMissing(f"capture file missing: {local}; no store key outside "
                           "the repository (BB2_CAPTURE_STORE)") from None
    external = store_root() / key / name
    if not external.is_file():
        raise StoreMissing(f"capture file missing: {local}; store: {external}; "
                           "set BB2_CAPTURE_STORE to the capture store root")
    if manifest is None:
        manifest = _manifest(capture_dir, name)
    records = _records(manifest, name)
    if not records:
        raise StoreUnverified(f"{external}: no manifest outputs record for {name}")
    for rec in records:
        verify(external, rec)
    return external


def capture_path(path, manifest=None):
    """Resolve a reader's existing logical capture filename."""
    path = Path(path)
    return resolve(path.parent, path.name, manifest)


def problem(error, path):
    """A portable report diagnostic: error class and logical artifact name."""
    path = Path(path)
    try:
        shown = path.resolve().relative_to(REPO_ROOT.resolve()).as_posix()
    except ValueError:
        shown = (Path(path.parent.name) / path.name).as_posix()
    reason = ("SHA1/size mismatch" if isinstance(error, StoreMismatch) else
              "missing capture artifact" if isinstance(error, StoreMissing) else
              "unverified store artifact")
    return f"{type(error).__name__}: {shown}: {reason}"


def open_capture_file(capture_dir, name, manifest=None, mode="r"):
    path = resolve(capture_dir, name, manifest)
    return path.open(mode, **({} if "b" in mode else {"encoding": "utf-8"}))
