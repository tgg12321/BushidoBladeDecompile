"""Q39 departures audit — completions that bypassed `queue done`.

Owner ruling Q39 gates `queue done` (engine/layer2.py). This audit is the
standing backstop for the one way around it: editing engine/queue.json by
hand. Run by tools/check_completion_integrity.py; every finding is a
violation.

KEYED BY ADDRESS. Every queue item carries `addr`, its function's VRAM
address (engine/queue.py fills it and refuses an item without one;
layer2.addr_index: glabel files, the tracked symbol files and census, the link
map), and every layer-2 record carries the same field. A rename — a naming wave, a revert, a reset —
changes names, never addresses, so the audit needs no rename tracking at all.

ANCHOR: the oldest commit (topological order) in HEAD's queue.json history
whose version gives EVERY item an `addr`. Versions from the anchor on — every
commit descending from it on HEAD's history (side branches included), and the
working tree — are audited; earlier ones are exempt.

DEPARTED: the addresses listed in any audited version, minus the addresses
queued in the working tree now. For each departed address A:
  D      = the last commit in which A left the queue (topological order; "the
           working tree" when HEAD lists it and the working tree does not);
  name@D = the func of A's item in the version just before D;
  body@D = body_key of name@D in the source at D — its queue item's file,
           else any source file (src/**/*.c|h, include/**/*.h) — with an asm
           body's included .s read AT D too; a working-tree departure uses the
           current body.
A departure is CLEAR only when, among the layer-2 records carrying addr A
(memory/grind/**/layer2.jsonl, _completed/ included) whose body_hash equals
body@D, the latest is a PASS. Records on other bodies (e.g. a later reviewed
cheat-cleanup) neither clear nor revoke it; a later FAIL / NEEDS_USER on body@D
revokes it. "Latest" is by effective date: the running maximum of the dates
within the record's own file (line order decides there, whatever the clock
did; layer2.record also never stamps a date earlier than its file's last),
files merged by it.

FAIL CLOSED: a git failure, a shallow or anchor-less history (an anchor with no
parent), a grafts file (replace refs are ignored: --no-replace-objects), a
missing object, an unparseable audited version (its addresses are then
recovered by the pattern "addr": "<8 hex>" and audited; only a version giving
none is a violation), an audited item without `addr` that no tracked file of
its own commit addresses (glabel file, symbol files, census), or an
unreadable record file — each is a violation. Git: three processes however long the history (the
grafts check, one `git log` of queue.json, one interactive
`git cat-file --batch`). Under WSL a worktree `.git` naming a Windows gitdir
is read through `--git-dir=/mnt/c/...`, and the reverse under Windows.

KNOWN LIMITS: (1) a function never listed from the anchor on is not audited;
(2) a fork that branched before the anchor and is merged later contributes no
versions; (3) forging an `addr` in queue.json or in a layer2.jsonl line is
record forgery (like a forged PASS line in the core gate) — caught by review,
not by this tool.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
from pathlib import Path

from . import layer2

QUEUE_FILE = "engine/queue.json"
_ADDR_PAT = re.compile(rb'"addr"\s*:\s*"([0-9A-Fa-f]{8})"')
_WIN_GITDIR = re.compile(r"^([A-Za-z]):[\\/](.*)$")
_MNT_GITDIR = re.compile(r"^/mnt/([A-Za-z])/(.*)$")
_SRC_PATH = re.compile(r"^(src/.+\.[ch]|include/.+\.h)$")
_ZERO = "0" * 40


class _AuditError(Exception):
    pass


def _gitdir_line(text: str) -> str | None:
    m = re.match(r"\s*gitdir:\s*(.+?)\s*$", text or "", re.S)
    return m.group(1) if m else None


def wsl_gitdir(git_file_text: str) -> str | None:
    """`gitdir: C:/x/y` -> `/mnt/c/x/y` (for WSL git); None otherwise."""
    w = _WIN_GITDIR.match(_gitdir_line(git_file_text) or "")
    return f"/mnt/{w.group(1).lower()}/" + w.group(2).replace("\\", "/") if w else None


def win_gitdir(git_file_text: str) -> str | None:
    """`gitdir: /mnt/c/x/y` -> `C:/x/y` (for Windows git); None otherwise."""
    m = _MNT_GITDIR.match(_gitdir_line(git_file_text) or "")
    return f"{m.group(1).upper()}:/{m.group(2)}" if m else None


def _git_cmd(osname: str | None = None) -> list[str]:
    cmd = ["git", "--no-replace-objects"]
    if Path(".git").is_file():
        text = layer2._read_text(Path(".git")) or ""
        g = (wsl_gitdir if (osname or os.name) == "posix" else win_gitdir)(text)
        if g:
            cmd.append(f"--git-dir={g}")
    return cmd


def _git(*args: str) -> bytes:
    try:
        r = subprocess.run([*_git_cmd(), *args], capture_output=True)
    except OSError as e:
        raise _AuditError(f"git {args[0]} could not run: {e}") from None
    if r.returncode != 0:
        err = r.stderr.decode("utf-8", "replace").strip().splitlines()
        raise _AuditError(f"git {' '.join(args[:2])} failed (exit {r.returncode}): "
                          f"{err[-1] if err else 'no message'}")
    return r.stdout


class _Batch:
    """ONE `git cat-file --batch` process, asked object by object."""

    def __init__(self):
        try:
            self.p = subprocess.Popen([*_git_cmd(), "cat-file", "--batch"],
                                      stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
        except OSError as e:
            raise _AuditError(f"git cat-file could not run: {e}") from None
        self.cache: dict[str, bytes | None] = {}

    def get(self, obj: str) -> bytes | None:
        """Object content, or None when git reports it missing."""
        if obj not in self.cache:
            try:
                self.p.stdin.write(obj.encode() + b"\n")
                self.p.stdin.flush()
                header = self.p.stdout.readline().split()
            except OSError as e:
                raise _AuditError(f"git cat-file --batch died: {e}") from None
            if len(header) == 2 and header[1] in (b"missing", b"ambiguous"):
                self.cache[obj] = None
            elif len(header) == 3:
                self.cache[obj] = self.p.stdout.read(int(header[2]))
                self.p.stdout.read(1)            # trailing LF
            else:
                err = self.p.stderr.read().decode("utf-8", "replace").strip()
                raise _AuditError(f"git cat-file --batch failed: {err or header}")
        return self.cache[obj]

    def text(self, obj: str) -> str | None:
        b = self.get(obj)
        return None if b is None else b.decode("utf-8", "replace")

    def must(self, obj: str, what: str) -> bytes:
        b = self.get(obj)
        if b is None:
            raise _AuditError(f"{what} ({obj}) is missing from the object store")
        return b

    def close(self):
        try:
            self.p.stdin.close()
        except OSError:
            pass
        if self.p.wait() != 0:
            err = self.p.stderr.read().decode("utf-8", "replace").strip()
            raise _AuditError(f"git cat-file --batch exited {self.p.returncode}: {err}")


def _history():
    """queue.json's history from HEAD, newest first (topological, full
    history): {commit: rewritten parents} and diffs [(commit, src, dst)]."""
    parents, diffs, commit = {}, [], ""
    for line in _git("log", "--topo-order", "--full-history", "--parents", "-m", "--raw",
                     "--no-abbrev", "--format=commit %H %P", "HEAD", "--",
                     QUEUE_FILE).decode().splitlines():
        if line.startswith("commit "):
            commit, *ps = line.split()[1:]
            parents.setdefault(commit, ps)
        elif line.startswith(":"):
            f = line.split("\t")[0].split()
            diffs.append((commit, f[2], f[3]))
    return parents, diffs


def _items(blob: bytes | None) -> dict[str, dict] | None:
    """addr -> item of one queue version; an item without a valid addr is
    kept under "?<func>" (the caller reports it). A version that is not JSON
    (a committed conflict) yields its addresses by pattern; None: none."""
    if blob is None:
        return None
    try:
        items = json.loads(blob)["items"]
        out = {}
        for it in items:
            a = it.get("addr")
            out[a if isinstance(a, str) and layer2.ADDR_RE.fullmatch(a) else
                f"?{it.get('func')}"] = it
        return out
    except (ValueError, KeyError, TypeError, AttributeError):
        found = {m.decode().upper(): {"addr": m.decode().upper()}
                 for m in _ADDR_PAT.findall(blob)}
        return found or None


def _full_addr(items: dict | None) -> bool:
    return bool(items) and not any(k.startswith("?") for k in items)


def _tree_sources(batch: _Batch, rev: str) -> list[tuple[str, str]]:
    """(path, blob) of every source file at `rev`; a tree git cannot produce
    fails the audit."""
    out = []

    def entries(data):
        i = 0
        while i < len(data):
            sp = data.index(b" ", i)
            nul = data.index(b"\0", sp)
            yield data[i:sp], data[sp + 1:nul].decode("utf-8", "replace"), \
                data[nul + 1:nul + 21].hex()
            i = nul + 21

    def walk(sha, prefix):
        for mode, name, sub in entries(batch.must(sha, f"source tree {prefix} at {rev}")):
            path = f"{prefix}/{name}"
            if mode == b"40000":
                walk(sub, path)
            elif _SRC_PATH.match(path):
                out.append((path, sub))
    for mode, name, sha in entries(batch.must(f"{rev}^{{tree}}", f"the root tree of {rev}")):
        if mode == b"40000" and name in ("src", "include"):
            walk(sha, name)
    return out


def _body_at(batch: _Batch, rev: str, name: str, file: str | None) -> str | None:
    """body_key of `name` in the source at `rev` — its item's file first, then
    any source file — with included .s read at `rev` as well."""
    def read(path):
        return batch.text(f"{rev}:{path}")

    files = _tree_sources(batch, rev)
    first = f"src/{file}.c" if file else None
    for path, sha in sorted(files, key=lambda f: f[0] != first):
        text = batch.text(sha)
        if text is None:
            raise _AuditError(f"source blob {sha} ({path} at {rev}) is missing from the "
                              f"object store")
        if name in text:
            k = layer2.body_key(text, name, read)
            if k:
                return k[1]
    return None


def _body_now(name: str, file: str | None) -> str | None:
    paths = [Path(f"src/{file}.c")] if file else []
    for pat in ("src/**/*.c", "src/**/*.h", "include/**/*.h"):
        paths += sorted(Path(".").glob(pat))
    for p in paths:
        text = layer2._read_text(p)
        k = layer2.body_key(text, name) if text and name in text else None
        if k:
            return k[1]
    return None


def _records() -> dict[str, list[dict]]:
    """addr -> its records, oldest first. A line's effective date is the
    running maximum of the dates in its file up to it, so a clock that went
    back never reorders one file's history (line order decides there); files
    are merged by effective date. An unreadable record file fails the audit."""
    by_addr: dict[str, list[tuple]] = {}
    files = sorted(Path("memory/grind").glob("**/" + layer2.RECORD_NAME))
    for fi, p in enumerate(files):
        text = layer2._read_text(p)
        if text is None:
            raise _AuditError(f"{p.as_posix()} is unreadable")
        eff = ""
        for li, line in enumerate(text.splitlines()):
            if not line.strip():
                continue
            try:
                rec = json.loads(line)
            except ValueError:
                raise _AuditError(f"{p.as_posix()}:{li + 1} is not JSON") from None
            eff = max(eff, str(rec.get("date", "")) if isinstance(rec, dict) else "")
            a = rec.get("addr") if isinstance(rec, dict) else None
            if isinstance(a, str):
                by_addr.setdefault(a.upper(), []).append((eff, fi, li, rec))
    return {a: [r for *_k, r in sorted(v, key=lambda t: t[:3])] for a, v in by_addr.items()}


def _fix(name: str, file: str) -> str:
    return (f"Fix: put it back (`python3 -m engine.cli queue reopen {name} --file {file} "
            f"--reason \"Q39: left the queue without a layer-2 PASS\"`) and re-land it; or, "
            f"while the body that left is still the body in src/, have a fresh "
            f"cheat-reviewer rule on it and record the verdict (`python3 -m engine.cli layer2 "
            f"record {name} --reviewer <id> --scope <match|cheat-cleanup|auth> --verdict-file "
            f"<its JSON>`). If that body has changed since, only reopen then re-land works.")


def _no_grafts() -> None:
    p = _git("rev-parse", "--git-path", "info/grafts").decode().strip()
    if p and Path(p).exists():
        raise _AuditError(f"a grafts file ({p}) rewrites this history's parents")


def unreviewed_departures() -> list[str]:
    """Violations (strings) — see the module docstring. Run from the repo root."""
    cur_text = layer2._read_text(Path(QUEUE_FILE))
    try:
        current = _items(cur_text.encode()) if cur_text is not None else None
    except AttributeError:
        current = None
    if current is None:
        return [f"Q39 departures audit: working-tree {QUEUE_FILE} is unreadable or has no "
                f"addresses"]
    for k in [k for k in current if k.startswith("?")]:
        a = layer2.addr_of(k[1:])           # addressed from the working tree's files
        if a:
            current[a] = current.pop(k)
    out = [f"Q39 departures audit: working-tree {QUEUE_FILE} item {k[1:]} has no valid "
           f"addr and none of the tracked files gives one" for k in current if k.startswith("?")]
    batch = None
    try:
        _no_grafts()
        parents, diffs = _history()
        batch = _Batch()
        parsed: dict[str, dict | None] = {}

        def version(obj):
            if obj not in parsed:
                parsed[obj] = (_items(batch.must(obj, f"{QUEUE_FILE} version"))
                               if obj != _ZERO else {})
            return parsed[obj]

        # anchor: the oldest full-address version (topological order)
        order = list(dict.fromkeys(c for c, _s, _d in diffs))
        dst_of = {}
        for c, _s, d in diffs:
            dst_of.setdefault(c, d)
        full = [c for c in order if _full_addr(version(dst_of[c]))]
        if not full:
            return out                      # no addressed queue yet: nothing audited
        anchor = full[-1]
        if not parents.get(anchor):
            raise _AuditError(f"cannot establish the anchor (shallow clone?): {anchor[:9]}, "
                              f"the first addressed queue version, has no parent here")
        # the audited commits: the anchor and everything descending from it
        children: dict[str, list[str]] = {}
        for c, ps in parents.items():
            for p in ps:
                children.setdefault(p, []).append(c)
        audited, stack = set(), [anchor]
        while stack:
            c = stack.pop()
            if c not in audited:
                audited.add(c)
                stack += children.get(c, [])

        def resolved(items, *revs):
            """Items without addr, addressed from the tracked files at `revs`
            (a revision's own glabel file, symbol files, census)."""
            out_items = {}
            for k, it in (items or {}).items():
                if k.startswith("?"):
                    for rev in revs:
                        a = layer2.addr_at(k[1:], lambda p, rev=rev: batch.text(f"{rev}:{p}"))
                        if a:
                            k = a
                            break
                out_items[k] = it
            return out_items

        listed: dict[str, tuple[str, dict]] = {}
        for c, _s, d in diffs:
            if c not in audited or d == _ZERO:
                continue
            items = resolved(version(d), c) if version(d) is not None else None
            if items is None:
                out.append(f"Q39 departures audit: {QUEUE_FILE} at {c[:9]} ({d}) gives no "
                           f"address at all — read it with `git cat-file -p {d}`")
                continue
            for a, it in items.items():
                if a.startswith("?"):
                    out.append(f"Q39 departures audit: {QUEUE_FILE} at {c[:9]}: item "
                               f"{a[1:]} has no valid addr")
                else:
                    listed.setdefault(a, (c, it))
        head = resolved(_items(batch.get(f"HEAD:{QUEUE_FILE}")), "HEAD")
        departed = sorted(a for a in listed if a not in current)

        # D: where each departed address last left the queue
        left: dict[str, tuple[str, dict]] = {}
        for a in departed:
            if a in head:
                left[a] = ("WORKTREE", head[a])
        for c, s, d in diffs:
            if c not in audited or s == _ZERO:
                continue
            before, after = resolved(version(s), f"{c}^", c), resolved(version(d), c)
            for a in departed:
                if a in before and a not in after and a not in left:
                    left[a] = (c, before[a])

        records = _records()
        for a in departed:
            c, item = left.get(a, (None, listed[a][1]))
            name, file = item.get("func"), item.get("file")
            if not name:
                body, where = None, "an unparseable version"
            elif c == "WORKTREE":
                body, where = _body_now(name, file), "the working tree"
            elif c:
                body, where = _body_at(batch, c, name, file), f"commit {c[:9]}"
            else:
                body, where = None, "an unknown commit"
            recs = records.get(a, [])
            # only records on the body that left count: a later reviewed body
            # (a cheat-cleanup) neither clears nor revokes this departure
            on_body = [r for r in recs if body and r.get("body_hash") == body]
            last = on_body[-1] if on_body else None
            if last and last.get("verdict") == "PASS":
                continue
            why = ("no layer-2 record carries this address" if not recs else
                   f"no record on that body (latest: {recs[-1].get('verdict')} on "
                   f"{recs[-1].get('body_hash')})" if last is None else
                   f"the latest record on that body is {last.get('verdict')} "
                   f"({last.get('func')}, {last.get('date', '?')})")
            out.append(f"{name or '?'} (addr {a}): left {QUEUE_FILE} in {where} — body "
                       f"{body or '(not found)'} — without a layer-2 PASS on that body: {why}. "
                       f"{_fix(name or '<name>', file or '<file>')}")
        batch.close()
        batch = None
    except _AuditError as e:
        return [f"Q39 departures audit could not read git history — {e}. The audit fails "
                f"closed; run it where git works on this tree."]
    finally:
        if batch is not None:
            try:
                batch.p.kill()
            except OSError:
                pass
    return out
