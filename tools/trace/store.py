"""Bulk capture store keyed by the capture path relative to this repo root.

Example: <store>/docs/naming/runtime/family/baseline/capture.jsonl.
External artifacts require an outputs SHA1 and byte count; local reads retain
their historical behavior.

To re-record, run the capture writer again at the same logical path. offload()
automatically selects <filename>.sha1-<SHA1> when the old key has different
bytes, and records that sibling as outputs[].store_name before publication.
Existing manifests continue to resolve their original immutable generation.
"""
import hashlib
import json
import os
from pathlib import Path, PureWindowsPath
import subprocess
import sys
import argparse
import tempfile
import re
import errno
import shutil

REPO_ROOT = Path(__file__).resolve().parents[2]
_warned_git_fallback = False
LEGACY_BASE = "5b51335f3c627710f3109bd10cc78821fbf5466d^"


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
    if manifest is None:
        # Preserve the missing-file diagnostic even when no manifest exists.
        try:
            manifest = _manifest(capture_dir, name)
        except StoreUnverified:
            if (store_root() / key / name).is_file():
                raise
    records = _records(manifest, name)
    external = store_target(capture_dir / name, records)
    if not external.is_file():
        raise StoreMissing(f"capture file missing: {local}; store: {external}; "
                           "set BB2_CAPTURE_STORE to the capture store root")
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


def file_record(path):
    path = Path(path)
    data = path.read_bytes()
    return dict(path=path.name, sha1=hashlib.sha1(data).hexdigest(), size=len(data))


def store_key(path):
    return Path(path).resolve().relative_to(REPO_ROOT.resolve())


def store_target(path, records):
    """store_name is a sibling content generation, never a path supplied by a writer."""
    names = []
    for rec in records:
        name = rec.get("store_name", Path(path).name)
        if not isinstance(name, str):
            raise StoreUnverified(f"{path}: invalid store generation")
        names.append(name)
    names = set(names)
    if len(names) > 1:
        raise StoreUnverified(f"{path}: conflicting store generations")
    name = next(iter(names), Path(path).name)
    if name != Path(path).name and (not isinstance(name, str) or not re.fullmatch(
            re.escape(Path(path).name) + r"\.sha1-[0-9a-f]{40}", name)):
        raise StoreUnverified(f"{path}: invalid store generation")
    return store_root() / store_key(path).parent / name


def select_generation(path, records):
    """Re-record automatically into a content-suffixed immutable sibling key.

    Old manifests keep their original key. Persist this selection before moving
    any bytes, so interruption cannot claim the old generation as the new one.
    """
    for rec in records:
        verify(path, rec)
    target = store_target(path, records)
    if target.exists():
        try:
            for rec in records:
                verify(target, rec)
        except StoreMismatch:
            name = Path(path).name + ".sha1-" + file_record(path)["sha1"]
            for rec in records:
                rec["store_name"] = name
            target = store_target(path, records)
            if target.exists():
                for rec in records:
                    verify(target, rec)


def git_output(*args):
    try:
        return subprocess.run(["git", "-C", str(REPO_ROOT), *args], check=True,
                              capture_output=True, text=True, encoding="utf-8").stdout.strip()
    except (OSError, subprocess.SubprocessError) as exc:
        raise StoreUnverified(f"cannot establish committed provenance: {exc}") from exc


def head_blob(path):
    key = store_key(path).as_posix()
    entry = git_output("ls-tree", "HEAD", "--", key)
    return entry.split()[2] if entry else None


def check_head(path, required=False):
    """Raw working bytes must equal HEAD whenever HEAD owns this path."""
    blob = head_blob(path)
    if blob is None:
        if required or git_output("ls-files", "--", store_key(path).as_posix()):
            raise StoreUnverified(f"{path}: not tracked in HEAD")
        return
    if git_output("hash-object", "--no-filters", "--", str(Path(path).resolve())) != blob:
        raise StoreUnverified(f"{path}: working file differs from HEAD")


def ever_committed(path):
    return bool(git_output("log", "-1", "--format=%H", "--", store_key(path).as_posix()))


def _check_digest(path, records, digest, size):
    for rec in records:
        expected_size = rec.get("size", rec.get("bytes"))
        if type(expected_size) is not int or expected_size < 0 or not isinstance(rec.get("sha1"), str):
            raise StoreUnverified(f"{path}: outputs record requires SHA1 and byte count")
        if rec["sha1"] != digest or expected_size != size:
            raise StoreMismatch(f"{path}: capture store SHA1/size mismatch")


def copy_recorded(path, temporary, records):
    """Hash the source once while copying; verify the disk copy separately."""
    digest = hashlib.sha1()
    size = 0
    with Path(path).open("rb") as source, Path(temporary).open("wb") as target:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            target.write(chunk)
            digest.update(chunk)
            size += len(chunk)
    _check_digest(path, records, digest.hexdigest(), size)


def mark_stored(manifest_path, manifest, names):
    """Persist publication intent in the metadata that travels with a capture."""
    changed = False
    for name in names:
        for rec in _records(manifest, name):
            if rec.get("stored") is not True:
                rec["stored"] = True
                changed = True
    if changed:
        write_manifest(manifest_path, manifest)


def rollback_stored(path, manifest, names):
    """Remove failed publication intent, preserving identical HEAD evidence."""
    committed = {}
    if head_blob(path) is not None:
        committed = json.loads(git_output("show", "HEAD:" + store_key(path).as_posix()))
    for name in names:
        for rec in _records(manifest, name):
            if not any(old.get("stored") is True
                       and record_identity(old) == record_identity(rec)
                       and old.get("store_name") == rec.get("store_name")
                       for old in _records(committed, name)):
                rec.pop("stored", None)
    write_manifest(path, manifest)


def check_migration_manifest(path, manifest):
    """Allow only our exact stored-flag additions when resuming a migration."""
    try:
        check_head(path)
        return
    except StoreUnverified:
        if head_blob(path) is None:
            raise
    committed = json.loads(git_output("show", "HEAD:" + store_key(path).as_posix()))
    if not isinstance(committed, dict):
        raise StoreUnverified(f"{path}: HEAD manifest must be an object")
    added = False
    for old, new in zip(committed.get("outputs", []), manifest.get("outputs", [])):
        if isinstance(old, dict) and isinstance(new, dict) and "stored" not in old and new.get("stored") is True:
            old["stored"] = True
            added = True
    if (not added or committed != manifest or
            Path(path).read_bytes() != (json.dumps(committed, indent=2) + "\n").encode("utf-8")):
        raise StoreUnverified(f"{path}: working manifest differs from HEAD beyond stored flags")


def move_verified(path, records):
    """Publish a verified copy before removing the source; failures keep it."""
    path = Path(path)
    if not records:
        raise StoreUnverified(f"{path}: no outputs record")
    before = path.stat()
    target = store_target(path, records)
    if target.resolve() == path.resolve():
        raise StoreUnverified(f"{path}: store target is the local source")
    target.parent.mkdir(parents=True, exist_ok=True)
    if target.exists():
        data = file_record(path)
        _check_digest(path, records, data["sha1"], data["size"])
        for rec in records:
            try:
                verify(target, rec)
            except StoreMismatch as exc:
                raise StoreMismatch(f"store key {store_key(path).as_posix()} already has different bytes; local kept") from exc
    else:
        fd, temporary = tempfile.mkstemp(prefix=".capture-", dir=str(target.parent))
        os.close(fd)
        try:
            copy_recorded(path, temporary, records)
            for rec in records:
                verify(temporary, rec)
            try:
                # Exclusive atomic publication: another lane can win the race,
                # but can never have its bytes overwritten.
                os.link(temporary, target)
            except FileExistsError:
                for rec in records:
                    verify(target, rec)
            except OSError as exc:
                if exc.errno not in {errno.EPERM, errno.EXDEV, errno.ENOSYS,
                                     errno.EOPNOTSUPP, errno.EINVAL}:
                    raise
                # Filesystems without hard links: exclusive create still
                # refuses another writer's bytes. A partial copy is never
                # accepted by readers (digest verification is mandatory).
                created = False
                try:
                    try:
                        with open(temporary, "rb") as source, target.open("xb") as output:
                            created = True
                            shutil.copyfileobj(source, output)
                    except FileExistsError:
                        if created:
                            raise
                    for rec in records:
                        verify(target, rec)
                except BaseException:
                    if created:
                        target.unlink()
                    raise
        finally:
            if os.path.exists(temporary):
                os.unlink(temporary)
    after = path.stat()
    if (after.st_size, after.st_mtime_ns, after.st_ino) != (before.st_size, before.st_mtime_ns, before.st_ino):
        raise StoreMismatch(f"{path}: source changed during publication; local kept")
    path.unlink()


def offload(capture_dir, manifest, keep_cards=False, manifest_path=None):
    """Called after publishing a writer's manifest, while outputs are local."""
    if os.environ.get("BB2_CAPTURE_STORE_WRITE") == "0":
        return
    capture_dir = Path(capture_dir).resolve()
    try:
        capture_dir.relative_to((REPO_ROOT / "docs/naming/runtime").resolve())
    except ValueError:
        return
    names = sorted({PureWindowsPath(r["path"]).name
                    for r in manifest.get("outputs", []) if "path" in r})
    names = [name for name in names if is_bulk(name) and (capture_dir / name).is_file()
             and not (keep_cards and name.endswith(".mcd"))]
    if not names:
        return
    if manifest_path is None:
        manifest_path = next((p for p in sorted(capture_dir.glob("*.manifest.json"))
                              if json.loads(p.read_text(encoding="utf-8")) == manifest), None)
    if manifest_path is None:
        raise StoreUnverified(f"{capture_dir}: offload requires an owning manifest")
    for name in names:
        select_generation(capture_dir / name, _records(manifest, name))
    # Generation metadata must be persisted even if stored was already true.
    for name in names:
        for rec in _records(manifest, name):
            rec["stored"] = True
    write_manifest(manifest_path, manifest)
    for name in names:
        try:
            move_verified(capture_dir / name, _records(manifest, name))
        except Exception:
            # A concurrent publisher may have taken the preflighted key.
            # Never leave publication intent claiming its different bytes.
            rollback_stored(manifest_path, manifest,
                            [n for n in names if (capture_dir / n).is_file()])
            raise


def offload_report(capture_dir, manifest, keep_cards=False, manifest_path=None):
    try:
        offload(capture_dir, manifest, keep_cards, manifest_path)
    except (OSError, ValueError, StoreError) as exc:
        remaining = [PureWindowsPath(r["path"]).name for r in manifest["outputs"]
                     if is_bulk(r["path"]) and (Path(capture_dir) / PureWindowsPath(r["path"]).name).is_file()]
        print(f"capture store: manifest written; offload failed: {exc}; "
              f"stayed local: {', '.join(remaining)}", file=sys.stderr)
        return False
    return True


def write_manifest(path, manifest):
    """Publish metadata first; a crash during offload leaves readable outputs."""
    path = Path(path)
    fd, temporary = tempfile.mkstemp(prefix=".manifest-", dir=str(path.parent))
    try:
        with os.fdopen(fd, "w", encoding="utf-8", newline="\n") as stream:
            json.dump(manifest, stream, indent=2)
            stream.write("\n")
        os.replace(temporary, path)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)


def manifests(root):
    for path in sorted(Path(root).rglob("*.manifest.json")):
        yield path, json.loads(path.read_text(encoding="utf-8"))


def bulk_files(root):
    return [p for p in sorted(Path(root).rglob("*")) if p.is_file() and is_bulk(p.name)]


def compare_manifest_path(report):
    path = Path(report).with_suffix(".manifest.json")
    if path.name == "capture.manifest.json":
        raise StoreUnverified("capture.md would overwrite capture.manifest.json; choose another report name")
    return path


def compare_manifest(report, base, other, outputs):
    """Versioned compare schema; input paths are relative to this summary."""
    directory = Path(report).resolve().parent
    return dict(kind="compare", version=1,
                base=Path(os.path.relpath(Path(base).resolve(), directory)).as_posix(),
                other=Path(os.path.relpath(Path(other).resolve(), directory)).as_posix(),
                outputs=outputs)


def legacy_diff(path, report, shot):
    """Require committed evidence and reproduce the exact committed image."""
    check_head(path, required=True)
    check_head(report, required=True)
    original = path.read_bytes()
    text = report.read_text(encoding="utf-8")
    if f"| {shot} (frame " not in text:
        raise StoreUnverified(f"{report}: no screenshot row for {shot}")
    references = re.findall(r"^- (?:baseline|stubbed|poked|modified): `([^`]+\.manifest\.json)`", text, re.M)
    if len(references) != 2:
        raise StoreUnverified(f"{report}: cannot attribute both input manifests")
    captures = []
    screenshots = []
    for reference in references:
        source = (report.parent / reference.replace("\\", "/")).resolve()
        check_head(source, required=True)
        manifest = json.loads(source.read_text(encoding="utf-8"))
        stem = source.name[:-len(".manifest.json")]
        name = stem + "." + shot + ".png"
        records = _records(manifest, name)
        if not records:
            raise StoreUnverified(f"{source}: no source screenshot record for {shot}")
        screenshot = resolve(source.parent, name, manifest)
        for rec in records:
            verify(screenshot, rec)
        captures.append(source.parent / (stem + ".jsonl"))
        screenshots.append(screenshot)
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import gdbtrace
    fd, temporary = tempfile.mkstemp(suffix=".png")
    os.close(fd)
    try:
        result, error = gdbtrace.diff_shot(str(screenshots[0]), str(screenshots[1]), temporary)
        if error or not result[0] or Path(temporary).read_bytes() != original:
            raise StoreUnverified(f"{path}: regenerated diff does not match committed image")
    finally:
        os.unlink(temporary)
    record = dict(path=path.name, sha1=hashlib.sha1(original).hexdigest(), size=len(original))
    return compare_manifest(report, captures[0], captures[1], []), record


def backfill_records(root, check=False):
    """Attribute legacy compare images using their report and source records."""
    changed = {}
    count = size = unrecorded = 0
    for path in bulk_files(root):
        try:
            manifest = _manifest(path.parent, path.name)
            if _records(manifest, path.name):
                continue
        except StoreUnverified:
            pass
        owner = None
        summary = None
        if path.name.endswith(".diff.png"):
            for report in sorted(path.parent.glob("*.md")):
                prefix = report.stem + "."
                if not path.name.startswith(prefix):
                    continue
                shot = path.name[len(prefix):-len(".diff.png")]
                try:
                    owner = compare_manifest_path(report)
                    summary, record = legacy_diff(path, report, shot)
                    break
                except (OSError, ValueError, StoreError) as exc:
                    print(f"REFUSE {path}: {exc}")
                    owner = None
        if owner is None:
            print(f"UNATTRIBUTED {path}")
            unrecorded += 1
            continue
        if owner not in changed:
            changed[owner] = summary
            if owner.exists():
                changed[owner]["outputs"] = json.loads(owner.read_text(encoding="utf-8")).get("outputs", [])
        changed[owner].setdefault("outputs", []).append(record)
        print(f"RECORD {path} -> {owner}")
        count += 1
        size += record["size"]
    if not check:
        for path, manifest in sorted(changed.items()):
            manifest["outputs"].sort(key=lambda r: r["path"])
            write_manifest(path, manifest)
    print(f"backfill-records: files={count} bytes={size} manifests={len(changed)} "
          f"unattributed={unrecorded} check={check}")
    return int(bool(unrecorded or (check and changed)))


def migrate(root, apply=False):
    count = size = refused = 0
    checked = set()
    for path in bulk_files(root):
        try:
            check_head(path)
            manifest = _manifest(path.parent, path.name)
            owners = []
            for owner in sorted(path.parent.glob("*.manifest.json")):
                owning_manifest = json.loads(owner.read_text(encoding="utf-8"))
                if _records(owning_manifest, path.name):
                    if owner not in checked:
                        check_migration_manifest(owner, owning_manifest)
                        checked.add(owner)
                    owners.append((owner, owning_manifest))
            records = [rec for _, owning_manifest in owners
                       for rec in _records(owning_manifest, path.name)]
            if not records:
                raise StoreUnverified(f"{path}: no outputs record")
            for rec in records:
                verify(path, rec)
            key = store_key(path)
            print(f"{'MOVE' if apply else 'PLAN'} {path}: {records} key={key.as_posix()}")
            length = path.stat().st_size
            if apply:
                for owner, owning_manifest in owners:
                    mark_stored(owner, owning_manifest, [path.name])
                try:
                    move_verified(path, records)
                except Exception:
                    for owner, owning_manifest in owners:
                        rollback_stored(owner, owning_manifest, [path.name])
                    raise
            count += 1
            size += length
        except (OSError, ValueError, StoreError) as exc:
            print(f"REFUSE {path}: {exc}")
            refused += 1
    print(f"migrate: files={count} bytes={size} refused={refused} apply={apply}")
    return int(bool(refused))


def tracked_manifests(root, revision="HEAD"):
    key = store_key(root).as_posix()
    if revision == ":":
        names = git_output("ls-files", "--", key)
    else:
        names = git_output("ls-tree", "-r", "--name-only", revision, "--", key)
    return {REPO_ROOT / name for name in names.splitlines()
            if name.endswith(".manifest.json")}


def record_identity(rec):
    return (rec.get("path"), rec.get("sha1"), rec.get("size", rec.get("bytes")))


def recorded_absence(rec):
    return rec.get("missing") is True and "sha1" not in rec


class LegacyRecords:
    """Per-command provenance cache, using ancestry rather than commit dates.

    Only identical path/SHA1/size records in a same-path manifest blob at an
    ancestor of module 2's parent qualify. Renamed manifests may stay refused
    (we deliberately do not follow renames). New/changed records cannot borrow
    a manifest's age. Bulk paths with any HEAD-reachable history never qualify.
    """

    def __init__(self):
        self.manifests = {}
        self.histories = {}

    def was_committed(self, logical):
        directory = store_key(logical).parent.as_posix()
        if directory not in self.histories:
            self.histories[directory] = set(git_output(
                "log", "--full-history", "--format=", "--name-only", "--no-renames", "--",
                directory).splitlines())
        return store_key(logical).as_posix() in self.histories[directory]

    def qualifies(self, manifest_path, rec):
        if "stored" in rec:
            return False
        name = PureWindowsPath(rec["path"]).name
        if self.was_committed(manifest_path.parent / name):
            return False
        key = store_key(manifest_path).as_posix()
        if key not in self.manifests:
            records = set()
            # Restrict traversal to the fixed boundary's ancestors. Backdated
            # descendants and unrelated commits cannot enter this walk.
            commits = git_output("log", "--full-history", "--format=%H", "--diff-filter=AM",
                                 "--no-renames", LEGACY_BASE, "--", key)
            for commit in commits.splitlines():
                old = json.loads(git_output("show", commit + ":" + key))
                if isinstance(old, dict) and isinstance(old.get("outputs"), list):
                    for output in old["outputs"]:
                        if (isinstance(output, dict) and "stored" not in output
                                and isinstance(output.get("path"), str)
                                and isinstance(output.get("sha1"), str)
                                and type(output.get("size", output.get("bytes"))) is int):
                            records.add(record_identity(output))
            self.manifests[key] = records
        return record_identity(rec) in self.manifests[key]


def check_commit(root):
    """Check the proposed index, including ignored bulk absent from that index.

    Local ignored files are deliberately insufficient: a manifest-only commit
    must have verified store bytes or be a historical never-published record.
    """
    count = failed = legacy = 0
    history = LegacyRecords()
    indexed = set(git_output("ls-files", "--", store_key(root).as_posix()).splitlines())
    for path in sorted(tracked_manifests(root, ":")):
        try:
            manifest = json.loads(git_output("show", ":" + store_key(path).as_posix()))
            if not isinstance(manifest, dict) or not isinstance(manifest.get("outputs", []), list):
                raise StoreUnverified("manifest must be an object with outputs list")
            for rec in manifest.get("outputs", []):
                if not isinstance(rec, dict) or not isinstance(rec.get("path"), str):
                    raise StoreUnverified("invalid output record")
                name = PureWindowsPath(rec["path"]).name
                if not is_bulk(name) or recorded_absence(rec):
                    continue
                logical = path.parent / name
                count += 1
                if store_key(logical).as_posix() in indexed:
                    continue
                try:
                    resolve(path.parent, name, manifest, allow_local=False)
                except StoreMissing:
                    if history.qualifies(path, rec):
                        legacy += 1
                        continue
                    raise StoreUnverified(f"{name}: bulk output neither indexed nor stored")
        except (OSError, ValueError, StoreError) as exc:
            print(f"REFUSE {path}: {exc}")
            failed += 1
    print(f"check-commit: outputs={count} refused={failed} legacy_never_published={legacy}")
    return int(bool(failed))


def verify_tree(root):
    root = Path(root).resolve()
    count = size = failed = never_published = 0
    history = LegacyRecords()
    tracked = tracked_manifests(root)
    for path in sorted(set(Path(root).rglob("*.manifest.json")) | tracked):
        try:
            if path in tracked:
                try:
                    check_head(path, required=True)
                except (OSError, StoreError) as exc:
                    print(f"MANIFEST DIFFERS {path}: {exc}")
                    failed += 1
                manifest = json.loads(git_output("show", "HEAD:" + store_key(path).as_posix()))
            else:
                manifest = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, ValueError, StoreError) as exc:
            print(f"MALFORMED {path}: {exc}")
            failed += 1
            continue
        if not isinstance(manifest, dict) or not isinstance(manifest.get("outputs", []), list):
            print(f"MALFORMED {path}: outputs must be a list")
            failed += 1
            continue
        for rec in manifest.get("outputs", []):
            if not isinstance(rec, dict) or not isinstance(rec.get("path"), str):
                print(f"MALFORMED {path}: invalid output record")
                failed += 1
                continue
            name = PureWindowsPath(rec["path"]).name
            if not is_bulk(name) or recorded_absence(rec):
                continue
            try:
                if not rec.get("missing"):
                    # Validate metadata before classifying an absent output.
                    size_rec = rec.get("size", rec.get("bytes"))
                    sha1_rec = rec.get("sha1")
                    if type(size_rec) is not int or size_rec < 0 or not isinstance(sha1_rec, str) or not re.fullmatch(r"[0-9a-f]{40}", sha1_rec):
                        raise StoreUnverified(f"{path}: malformed SHA1/byte count for {name}")
                resolved = resolve(path.parent, name, manifest)
                verify(resolved, rec)
                count += 1
                size += resolved.stat().st_size
            except StoreMissing as exc:
                logical = path.parent / name
                try:
                    lost = (not history.qualifies(path, rec) if path in tracked
                            else rec.get("stored") is True or history.was_committed(logical))
                except (OSError, ValueError, StoreError):
                    lost = True
                if lost:
                    print(f"LOST {logical}: {exc}")
                    failed += 1
                else:
                    print(f"NEVER PUBLISHED {logical}")
                    never_published += 1
            except (OSError, StoreError) as exc:
                logical = path.parent / name
                label = "LOST" if rec.get("stored") is True and not logical.is_file() else "FAIL"
                print(f"{label} {logical}: {exc}")
                failed += 1
    print(f"verify: files={count} bytes={size} lost_or_mismatched={failed} never_published={never_published}")
    return int(bool(failed))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    for command in ("backfill-records", "migrate", "verify", "check-commit"):
        sub = commands.add_parser(command)
        sub.add_argument("--root", type=Path, default=REPO_ROOT / "docs/naming/runtime")
        if command == "backfill-records":
            sub.add_argument("--check", action="store_true")
        if command == "migrate":
            sub.add_argument("--apply", action="store_true")
    args = parser.parse_args(argv)
    if not args.root.is_dir():
        parser.error(f"root is not a directory: {args.root}")
    if args.command == "backfill-records":
        return backfill_records(args.root, args.check)
    if args.command == "migrate":
        return migrate(args.root, args.apply)
    if args.command == "check-commit":
        return check_commit(args.root)
    return verify_tree(args.root)


if __name__ == "__main__":
    sys.exit(main())
