"""Bulk capture store keyed by the capture path relative to this repo root.

Example: <store>/docs/naming/runtime/family/baseline/capture.jsonl.
External artifacts require an outputs SHA1 and byte count; local reads retain
their historical behavior.

To re-record, run the capture writer again at the same logical path. offload()
automatically selects <filename>.sha1-<SHA1> when the old key has different
bytes, and records that sibling as outputs[].store_name before publication.
Existing manifests continue to resolve their original immutable generation.
Removing a published capture directory is refused while its store copy exists;
remove both indexed bulk and stored generations for a deliberate full removal.
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
# The bulk migration (migrate --apply). A commit whose parent descends from it
# may not add or change a bulk blob: bulk lives in the store from then on.
MIGRATION_COMMIT = "dab4c0fc1698816e9b997f173be4937f98094aa6"


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
    name = Path(name).name.casefold()
    return name == "capture.jsonl" or name.endswith((".png", ".script.log", ".mcd"))


def alias_name(name):
    """Windows drops a trailing dot or space, opens a stream at ':' and may
    read NAME~N.EXT as an 8.3 short name: such a name can reach another file
    than the one it spells (and "capture.jsonl." escapes is_bulk)."""
    return (name != name.rstrip(". ") or ":" in name or
            re.fullmatch(r"[^.]{1,6}~[0-9]+(\.[^.]{0,3})?", name) is not None)


def verify(path, manifest_rec):
    """Require and check SHA1 plus runkit.file_rec's size (or bytes)."""
    size = manifest_rec.get("size", manifest_rec.get("bytes"))
    sha1 = manifest_rec.get("sha1")
    if not isinstance(sha1, str) or len(sha1) != 40 or type(size) is not int or size < 0:
        raise StoreUnverified(f"{path}: outputs record requires SHA1 and byte count")
    digest, actual = file_digest(path)
    if digest != sha1 or actual != size:
        raise StoreMismatch(f"{path}: capture store SHA1/size mismatch "
                            f"(expected {sha1}/{size}, got {digest}/{actual})")


def file_digest(path):
    """SHA1 and byte count, read in 1 MiB chunks (capture.jsonl can be large)."""
    digest = hashlib.sha1()
    size = 0
    with Path(path).open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
            size += len(chunk)
    return digest.hexdigest(), size


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
    if (not isinstance(name, str) or name in ("", ".", "..") or "/" in name or "\\" in name
            or PureWindowsPath(name).drive or alias_name(name)):
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
    sha1, size = file_digest(path)
    return dict(path=path.name, sha1=sha1, size=size)


def store_key(path):
    return Path(path).resolve().relative_to(REPO_ROOT.resolve())


def store_target(path, records, key=None, external=None):
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
    return (external if external is not None else store_root()) / (
        key if key is not None else store_key(path)).parent / name


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
        except Exception as original:
            # A concurrent publisher may have taken the preflighted key.
            # Never leave publication intent claiming its different bytes.
            try:
                rollback_stored(manifest_path, manifest,
                                [n for n in names if (capture_dir / n).is_file()])
            except Exception as rollback:
                raise StoreUnverified(f"publication failed: {original}; rollback failed: {rollback}") from original
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


def unstored_sha1(manifest):
    """SHA1 of a manifest as written before offload marked records stored.

    migrate --apply added only `stored` flags, through write_manifest, so
    pre-migration citations (knockout inventories, report and compare headers)
    name this digest of the current manifest.
    """
    plain = json.loads(json.dumps(manifest))
    if isinstance(plain, dict) and isinstance(plain.get("outputs"), list):
        for rec in plain["outputs"]:
            if isinstance(rec, dict):
                rec.pop("stored", None)
    return hashlib.sha1((json.dumps(plain, indent=2) + "\n").encode("utf-8")).hexdigest()


def manifest_cited(path, sha1):
    """True when a cited SHA1 names this manifest, now or before offload."""
    data = Path(path).read_bytes()
    if hashlib.sha1(data).hexdigest() == sha1:
        return True
    try:
        manifest = json.loads(data)
    except ValueError:
        return False
    stored = isinstance(manifest, dict) and any(
        isinstance(rec, dict) and "stored" in rec for rec in manifest.get("outputs") or [])
    return stored and unstored_sha1(manifest) == sha1


def inventoried_before_migration(inventory, rel, sha1):
    """The inventory held rel -> sha1 at MIGRATION_COMMIT (legacy entry)."""
    key = repo_key(inventory)
    try:
        old = json.loads(git_output("show", f"{MIGRATION_COMMIT}:{key}"))
    except (StoreError, ValueError):
        return False
    return isinstance(old, dict) and old.get(rel) == sha1


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
                except Exception as original:
                    try:
                        for owner, owning_manifest in owners:
                            rollback_stored(owner, owning_manifest, [path.name])
                    except Exception as rollback:
                        raise StoreUnverified(f"publication failed: {original}; rollback failed: {rollback}") from original
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


def repo_key(path):
    """Lexical key for trusted tree paths: no filesystem access on /mnt/c."""
    return Path(os.path.abspath(path)).relative_to(Path(os.path.abspath(REPO_ROOT))).as_posix()


def _git_blob_records(objects, hashed=False):
    """One request at a time bounds pipe and capture memory to a 1 MiB chunk."""
    objects = list(dict.fromkeys(objects))
    if not objects:
        return
    try:
        process = subprocess.Popen(
            ["git", "-C", str(REPO_ROOT), "cat-file", "--batch"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except OSError as exc:
        raise StoreUnverified(f"cannot read Git blobs: {exc}") from exc
    try:
        for obj in objects:
            process.stdin.write((obj + "\n").encode("utf-8"))
            process.stdin.flush()
            header = process.stdout.readline().split()
            if len(header) != 3 or header[1] != b"blob":
                raise StoreUnverified(f"{obj}: expected Git blob")
            size = remaining = int(header[2])
            digest = hashlib.sha1()
            data = bytearray()
            while remaining:
                chunk = process.stdout.read(min(remaining, 1024 * 1024))
                if not chunk:
                    raise StoreUnverified(f"{obj}: truncated Git blob")
                remaining -= len(chunk)
                if hashed:
                    digest.update(chunk)
                else:
                    data.extend(chunk)
            if process.stdout.read(1) != b"\n":
                raise StoreUnverified(f"{obj}: invalid Git blob delimiter")
            yield obj, (digest.hexdigest(), size) if hashed else bytes(data)
        process.stdin.close()
        if process.wait() != 0:
            raise StoreUnverified("cannot read Git blobs: " + process.stderr.read().decode("utf-8", "replace"))
    finally:
        if process.poll() is None:
            process.terminate()
        for stream in (process.stdin, process.stdout, process.stderr):
            stream.close()
        process.wait()


def git_blobs(objects):
    """Batch metadata blobs; bulk callers use incremental digests instead."""
    return dict(_git_blob_records(objects))


def git_blob_digests(objects):
    return dict(_git_blob_records(objects, hashed=True))


def tree_entries(root, revision, problems=None):
    key = repo_key(root)
    if revision == ":":
        raw = git_output("ls-files", "-s", "-z", "--", ":(icase)" + key)
    else:
        raw = git_output("ls-tree", "-r", "-z", revision)
    entries = {}
    for item in raw.split("\0"):
        if not item:
            continue
        info, name = item.split("\t", 1)
        if key != "." and not (name.casefold() == key.casefold() or
                               name.casefold().startswith(key.casefold() + "/")):
            continue
        fields = info.split()
        error = None
        if key != "." and not (name == key or name.startswith(key + "/")):
            error = f"{name}: case-variant runtime root (expected {key})"
        if revision == ":" and fields[2] != "0":
            error = f"{name}: unmerged index entry"
        mode = fields[0]
        blob = fields[1] if revision == ":" else fields[2]
        if mode not in ("100644", "100755"):
            error = f"{name}: expected regular file"
        if error:
            if problems is None:
                raise StoreUnverified(error)
            problems.append(error)
            continue
        entries[name] = blob
    return entries


class LegacyRecords:
    """Two root-wide history walks and one blob batch, cached per command.

    The boundary walk is ancestry based, never date based. Keep manifest paths
    separate even when blobs are shared; renamed manifests cannot borrow age.
    """

    def __init__(self, root=None, revision="HEAD"):
        self.root = root or REPO_ROOT / "docs/naming/runtime"
        self.revision = revision
        self.manifests = None
        self.histories = None
        self.manifest_errors = {}

    def _load(self):
        if self.manifests is not None:
            return
        if git_output("rev-parse", "--is-shallow-repository") == "true":
            raise StoreUnverified("legacy provenance requires complete Git history")
        key = repo_key(self.root)
        histories = set()
        versions = {}
        for revision, legacy in ((self.revision, False), (LEGACY_BASE, True)):
            raw = git_output("log", "--full-history", "-m", "--format=", "--raw",
                             "--no-abbrev", "-z", "--no-renames", revision, "--", key)
            items = iter(raw.split("\0"))
            for item in items:
                header = item.strip()
                if not header:
                    continue
                if not header.startswith(":"):
                    raise StoreUnverified("unexpected Git raw history entry")
                name = next(items)
                fields = header[1:].split()
                if legacy:
                    if name.endswith(".manifest.json") and fields[4] in ("A", "M"):
                        versions.setdefault(name, set()).add(fields[3])
                else:
                    histories.add(name)
        blobs = git_blobs(blob for values in versions.values() for blob in values)
        manifests = {}
        for name, values in versions.items():
            records = set()
            for blob in values:
                try:
                    old = json.loads(blobs[blob])
                except (ValueError, UnicodeError) as exc:
                    self.manifest_errors[name] = str(exc)
                    continue
                if isinstance(old, dict) and isinstance(old.get("outputs"), list):
                    for output in old["outputs"]:
                        if (isinstance(output, dict) and "stored" not in output
                                and isinstance(output.get("path"), str)
                                and isinstance(output.get("sha1"), str)
                                and type(output.get("size", output.get("bytes"))) is int):
                            records.add(record_identity(output))
            manifests[name] = records
        self.histories, self.manifests = histories, manifests

    def was_committed(self, logical):
        self._load()
        return repo_key(logical) in self.histories

    def qualifies(self, manifest_path, rec):
        # A legacy record predates both flags: stored marks an offload and a
        # store_name names a stored generation: a record with either is not legacy.
        if "stored" in rec or "store_name" in rec:
            return False
        name = PureWindowsPath(rec["path"]).name
        if self.was_committed(manifest_path.parent / name):
            return False
        key = repo_key(manifest_path)
        if key in self.manifest_errors:
            raise StoreUnverified(f"{manifest_path}: malformed legacy manifest: {self.manifest_errors[key]}")
        return record_identity(rec) in self.manifests.get(key, set())


def after_migration(parent):
    """True when parent is MIGRATION_COMMIT or descends from it (ancestry, not date)."""
    if parent is None:
        return False
    if git_output("rev-parse", "--is-shallow-repository") == "true":
        raise StoreUnverified("migration ancestry requires complete Git history")
    try:
        result = subprocess.run(["git", "-C", str(REPO_ROOT), "merge-base", "--is-ancestor",
                                 MIGRATION_COMMIT, parent], capture_output=True)
    except (OSError, subprocess.SubprocessError) as exc:
        raise StoreUnverified(f"cannot establish migration ancestry: {exc}") from exc
    if result.returncode not in (0, 1):
        # An unknown migration commit (a clone without it) is not an ancestor.
        probe = subprocess.run(["git", "-C", str(REPO_ROOT), "cat-file", "-e",
                                MIGRATION_COMMIT + "^{commit}"], capture_output=True)
        if probe.returncode == 0:
            raise StoreUnverified("cannot establish migration ancestry: "
                                  + result.stderr.decode("utf-8", "replace").strip())
    return result.returncode == 0


def check_commit(root, revision=None):
    """Judge index vs HEAD, or a commit's immutable tree vs its first parent."""
    count = failed = legacy = 0
    if revision is None:
        proposed, parent, history_rev = ":", "HEAD", "HEAD"
    else:
        proposed = git_output("rev-parse", "--verify", revision + "^{commit}")
        parents = git_output("rev-list", "--parents", "-n1", proposed).split()
        if len(parents) > 2:
            raise StoreUnverified("check-commit --rev requires a non-merge commit")
        parent = parents[1] if len(parents) == 2 else None
        history_rev = proposed
    history = LegacyRecords(root, history_rev)
    external = store_root()
    indexed = tree_entries(root, proposed)
    previous = tree_entries(root, parent) if parent else {}
    manifest_keys = {key for key in indexed.keys() | previous.keys()
                     if key.endswith(".manifest.json")}
    blobs = git_blobs([entries[key] for entries in (indexed, previous)
                       for key in manifest_keys if key in entries])
    digests = git_blob_digests(blob for key, blob in indexed.items() if is_bulk(key))
    if after_migration(parent):
        # Recorded or not, a new bulk blob would re-bloat git; carried-over
        # entries and deletions stay allowed.
        for key in sorted(key for key, blob in indexed.items()
                          if is_bulk(key) and previous.get(key) != blob):
            print(f"REFUSE {REPO_ROOT / key}: bulk blob added to git after the store "
                  "migration; offload it to the capture store instead")
            failed += 1
    for key in sorted(manifest_keys, key=lambda key: REPO_ROOT / key):
        path = REPO_ROOT / key
        try:
            manifest = json.loads(blobs[indexed[key]]) if key in indexed else {"outputs": []}
            old = json.loads(blobs[previous[key]]) if key in previous else {"outputs": []}
            for value in (manifest, old):
                if not isinstance(value, dict) or not isinstance(value.get("outputs", []), list):
                    raise StoreUnverified("manifest must be an object with outputs list")
                if any(not isinstance(r, dict) or not isinstance(r.get("path"), str)
                       for r in value.get("outputs", [])):
                    raise StoreUnverified("invalid output record")
            new_records = manifest.get("outputs", [])
            # A re-record updates the same output path; its replacement bytes
            # are verified below. Disappearing paths require full removal.
            output_paths = {PureWindowsPath(rec["path"]).name for rec in new_records}
            for rec in old.get("outputs", []):
                name = PureWindowsPath(rec["path"]).name
                if not is_bulk(name) or name in output_paths:
                    continue
                logical = path.parent / name
                bulk_key = (Path(key).parent / name).as_posix()
                present = (bulk_key in indexed or
                           store_target(logical, [rec], Path(bulk_key), external).exists())
                print(f"{'REFUSE' if present else 'REMOVED'} {path}: removed output record {name}"
                      + (" still indexed or stored" if present else " (full removal)"))
                if present:
                    failed += 1
            for rec in new_records:
                name = PureWindowsPath(rec["path"]).name
                if alias_name(name):
                    raise StoreUnverified(f"{name!r}: Windows alias output name")
                if not is_bulk(name):
                    continue
                logical = path.parent / name
                bulk_key = (Path(key).parent / name).as_posix()
                if rec.get("missing") is True:
                    if "store_name" in rec:
                        raise StoreUnverified(f"{name}: missing record cannot have store_name")
                    plain = external / bulk_key
                    old_targets = [store_target(logical, [r], Path(bulk_key), external)
                                   for r in old.get("outputs", [])
                                   if PureWindowsPath(r["path"]).name == name]
                    if (bulk_key in indexed or (revision is None and logical.exists()) or
                            plain.exists() or any(candidate.name.casefold().startswith(
                                (name + ".sha1-").casefold())
                                for candidate in plain.parent.glob("*")) or
                            any(target.exists() for target in old_targets)):
                        raise StoreUnverified(f"{name}: missing record has existing bulk file")
                    if recorded_absence(rec):
                        continue
                count += 1
                if bulk_key in indexed:
                    digest, size = digests[indexed[bulk_key]]
                    _check_digest(logical, [rec], digest, size)
                    continue
                try:
                    target = store_target(logical, _records(manifest, name), Path(bulk_key), external)
                    if not target.is_file():
                        raise StoreMissing(f"{name}: store file absent")
                    verify(target, rec)
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
    history = LegacyRecords(root)
    problems = []
    entries = tree_entries(root, "HEAD", problems)
    for error in problems:
        print(f"FAIL {error}")
        failed += 1
    tracked = {REPO_ROOT / key for key in entries if key.endswith(".manifest.json")}
    blobs = git_blobs(entries[repo_key(path)] for path in tracked)
    for path in sorted(set(Path(root).rglob("*.manifest.json")) | tracked):
        try:
            if path in tracked:
                data = blobs[entries[repo_key(path)]]
                if not path.is_file() or path.read_bytes() != data:
                    print(f"MANIFEST DIFFERS {path}: working file differs from HEAD")
                    failed += 1
                manifest = json.loads(data)
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
            if alias_name(name):
                print(f"MALFORMED {path}: Windows alias output name {name!r}")
                failed += 1
                continue
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
    sub = commands.add_parser("manifest-sha1", help="current and pre-store SHA1 of manifests")
    sub.add_argument("manifests", nargs="+", type=Path)
    for command in ("backfill-records", "migrate", "verify", "check-commit"):
        sub = commands.add_parser(command)
        sub.add_argument("--root", type=Path, default=REPO_ROOT / "docs/naming/runtime")
        if command == "backfill-records":
            sub.add_argument("--check", action="store_true")
        if command == "check-commit":
            sub.add_argument("--rev", help="judge a commit tree against its parent")
        if command == "migrate":
            sub.add_argument("--apply", action="store_true")
    args = parser.parse_args(argv)
    if args.command == "manifest-sha1":
        for path in args.manifests:
            data = path.read_bytes()
            print(hashlib.sha1(data).hexdigest(), unstored_sha1(json.loads(data)), path)
        return 0
    if args.command != "check-commit" and not args.root.is_dir():
        parser.error(f"root is not a directory: {args.root}")
    if args.command == "backfill-records":
        return backfill_records(args.root, args.check)
    if args.command == "migrate":
        return migrate(args.root, args.apply)
    if args.command == "check-commit":
        try:
            return check_commit(args.root, args.rev)
        except (OSError, ValueError, StoreError) as exc:
            print(f"REFUSE check-commit: {exc}")
            return 1
    return verify_tree(args.root)


if __name__ == "__main__":
    sys.exit(main())
