"""Q39 departures audit — completions that bypassed `queue done`.

Owner ruling Q39 gates `queue done` (engine/layer2.py). This audit is the
standing backstop for the one way around it: editing engine/queue.json by
hand. Run by tools/check_completion_integrity.py; every finding is a
violation.

SEMANTICS (union of versions, not adjacent diffs): a function is a DEPARTURE
when it is listed in ANY version of engine/queue.json since the gate existed —
every version committed on the ancestry path from the gate's anchor commit to
HEAD (side branches included: --full-history), the anchor's own — and is NOT
listed in the working tree's queue now under its current name. A function that
departed before the gate never appears in a post-gate version, so it is exempt
without any date cutoff.

WHICH BODY (Q39: "a PASS for the exact code being landed"): a departure
committed in commit C is judged against the function's body AT C — the body
that left the queue. A departure still in the working tree (HEAD lists it, the
working tree does not) is judged against the current body. Either way a PASS
on the current body also clears it. So a later naming wave, which moves every
referencing body's key, does not force completed functions back to review;
only an edit made while landing does. A PASS clears unless a later record on
that same body revokes it (FAIL / NEEDS_USER).

ANCHOR: the oldest commit (topological order, full history) in HEAD's history
that ADDS `GATE_FILE` (engine/layer2.py) — found by ancestry, never by date,
so a skewed commit date cannot move it. Any other commit adding the file must
descend from it (delete + re-add); independent adds (no single chain), or an
anchor with no parent in this history (a root commit or a shallow boundary),
fail closed. A move of the gate module would silently re-anchor it later;
test_departures pins GATE_FILE to engine.layer2's own path so such a move
fails the suite instead.

RENAMES need positive evidence AND an absent body. tools/naming_wave.py
appends the old name to a `renamed_from` LIST on the queue item it renames and
on every layer2.jsonl line it retargets; the audit reads those chains from the
current queue, from every post-gate queue version, and from the live and
archived records. An edge old -> new counts only when `old` has no C or asm
body anywhere in src/ now — a forged chain cannot launder a function that
still exists.

BODY LOCATION: a function's current file comes from one pass over src/ (a TU
resplit moves functions between files); a queue item's `file` is only the
fallback.

UNPARSEABLE VERSIONS (e.g. a committed conflict): the function names in them
are recovered by pattern (`"func": "<name>"`) and audited like any other; only
a version yielding no name at all is a violation.

GIT: three processes however long the history — the anchor lookup, one
`git log --raw` over the ancestry path, and ONE interactive
`git cat-file --batch` for every queue.json version and every departure-time
source file. Every git failure and a missing object is a violation: an audit
that cannot read history must not pass. Under WSL, a worktree whose `.git`
points at a Windows path (`gitdir: C:/...`) is read through
`--git-dir=/mnt/c/...`, which WSL git cannot derive itself.

BLIND SPOTS (by design): a function never listed in any post-gate queue
version is not audited (it never was INCOMPLETE work under the gate); and
queue versions on forks that branched BEFORE the anchor are not read — a
function such a fork lists and drops before merging is invisible, although
once merged, anything the post-gate line itself listed is still audited.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
from pathlib import Path

from . import inlineasm, layer2

GATE_FILE = "engine/layer2.py"
QUEUE_FILE = "engine/queue.json"
_FUNC_RE = re.compile(rb'"func"\s*:\s*"([^"\\]+)"')
_WIN_GITDIR = re.compile(r"^([A-Za-z]):[\\/](.*)$")


class _AuditError(Exception):
    pass


def wsl_gitdir(git_file_text: str) -> str | None:
    """`.git` FILE content `gitdir: C:/x/y` -> `/mnt/c/x/y`; None when it does
    not name a Windows drive path."""
    m = re.match(r"\s*gitdir:\s*(.+?)\s*$", git_file_text or "", re.S)
    w = _WIN_GITDIR.match(m.group(1)) if m else None
    if not w:
        return None
    return f"/mnt/{w.group(1).lower()}/" + w.group(2).replace("\\", "/")


def _git_cmd() -> list[str]:
    if os.name == "posix" and Path(".git").is_file():
        g = wsl_gitdir(layer2._read_text(Path(".git")) or "")
        if g:
            return ["git", f"--git-dir={g}"]
    return ["git"]


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
        if obj in self.cache:
            return self.cache[obj]
        try:
            self.p.stdin.write(obj.encode() + b"\n")
            self.p.stdin.flush()
            header = self.p.stdout.readline().split()
        except OSError as e:
            raise _AuditError(f"git cat-file --batch died: {e}") from None
        if len(header) == 2 and header[1] in (b"missing", b"ambiguous"):
            out = None
        elif len(header) == 3:
            out = self.p.stdout.read(int(header[2]))
            self.p.stdout.read(1)            # trailing LF
        else:
            err = self.p.stderr.read().decode("utf-8", "replace").strip()
            raise _AuditError(f"git cat-file --batch failed: {err or header}")
        self.cache[obj] = out
        return out

    def close(self):
        try:
            self.p.stdin.close()
        except OSError:
            pass
        rc = self.p.wait()
        if rc != 0:
            err = self.p.stderr.read().decode("utf-8", "replace").strip()
            raise _AuditError(f"git cat-file --batch exited {rc}: {err}")


def _anchor() -> tuple[str, list[str]] | None:
    """(anchor, the other commits adding GATE_FILE), or None: no gate here."""
    lines = _git("log", "--topo-order", "--full-history", "--diff-filter=A",
                 "--format=%H %P", "HEAD", "--", GATE_FILE).decode().split("\n")
    adds = [l.split() for l in lines if l.strip()]
    if not adds:
        return None
    anc = adds[-1]
    if len(anc) == 1:
        raise _AuditError(f"cannot establish the gate anchor (shallow clone?): {anc[0][:9]}, "
                          f"the commit adding {GATE_FILE}, has no parent in this history")
    return anc[0], [a[0] for a in adds[:-1]]


def _history(anc: str) -> tuple[list[tuple[str, str, str]], set[str]]:
    """Queue diffs (commit, src blob, dst blob) newest first, and the commits
    on the ancestry path that add GATE_FILE."""
    diffs, gate_adds, commit = [], set(), ""
    for line in _git("log", "--topo-order", "--full-history", "--ancestry-path", "-m", "--raw",
                     "--no-abbrev", "--format=commit %H", f"{anc}..HEAD", "--",
                     QUEUE_FILE, GATE_FILE).decode().splitlines():
        if line.startswith("commit "):
            commit = line.split()[1]
        elif line.startswith(":"):
            meta, path = line.split("\t", 1)
            f = meta.split()
            if path == GATE_FILE and f[4].startswith("A"):
                gate_adds.add(commit)
            elif path == QUEUE_FILE:
                diffs.append((commit, f[2], f[3]))
    return diffs, gate_adds


def _parse(blob: bytes) -> dict[str, dict] | None:
    """func -> item; names recovered by pattern when the JSON is broken (a
    committed conflict); None when not even a name can be found."""
    try:
        return {it["func"]: it for it in json.loads(blob)["items"]}
    except (ValueError, KeyError, TypeError):
        names = [m.decode("utf-8", "replace") for m in _FUNC_RE.findall(blob)]
        return {n: {"func": n} for n in names} or None


def _chain(v) -> list[str]:
    if isinstance(v, str):
        return [v]
    return [x for x in v if isinstance(x, str)] if isinstance(v, list) else []


def _src_scan(names: set[str]) -> tuple[dict[str, str], set[str]]:
    """One pass over src/*.c: name -> the file holding its single keyable
    body, and the names with ANY C or asm body (even an ambiguous one)."""
    where, present = {}, set()
    for p in sorted(Path("src").glob("*.c")):
        text = layer2._read_text(p)
        if not text:
            continue
        asm = None
        for n in names:
            if n not in text:
                continue
            if asm is None:
                asm = inlineasm.whole_body_asm_funcs(text)
            if inlineasm._func_body_span(text, n) is not None or n in asm:
                present.add(n)
                if n not in where and layer2.body_key(text, n) is not None:
                    where[n] = p.stem
    return where, present


def _records(func: str) -> tuple[list[dict], str | None]:
    """func's records (live ledger, else archived) and where; ([], reason) on
    a malformed file."""
    for p in (layer2.record_path(func), layer2.completed_record_path(func)):
        if p.exists():
            try:
                recs = layer2.read_records(func, p)
            except ValueError as e:
                return [], str(e)
            if recs:
                return recs, p.as_posix()
    return [], None


def _clears(func: str, dep_key: str | None, cur_key: str | None) -> str | None:
    """None when a PASS covers the departed body (not revoked by a later record
    on that body) or the latest record is a PASS on the current body."""
    recs, where = _records(func)
    if not recs:
        return where or (f"no layer-2 record in {layer2.record_path(func).as_posix()} or "
                         f"{layer2.completed_record_path(func).as_posix()}")
    last = recs[-1]
    if cur_key and last["verdict"] == "PASS" and last["body_hash"] == cur_key:
        return None
    if dep_key:
        on_body = [r for r in recs if r["body_hash"] == dep_key]
        if on_body and on_body[-1]["verdict"] == "PASS":
            return None
    bodies = " / ".join(f"{k} {v}" for k, v in (("departed body", dep_key),
                                                ("current body", cur_key)) if v)
    return (f"no unrevoked PASS in {where} on its {bodies or 'body (none found)'} "
            f"(latest record: {last['verdict']} on {last['body_hash']})")


def _fix(func: str, stem: str) -> str:
    return (f"Fix: put it back (`python3 -m engine.cli queue reopen {func} --file {stem} "
            f"--reason \"Q39: left the queue without a layer-2 PASS\"`), or have a fresh "
            f"cheat-reviewer rule on its current body and record the verdict "
            f"(`python3 -m engine.cli layer2 record {func} --reviewer <id> --scope "
            f"<match|cheat-cleanup|auth> --verdict-file <its JSON>`).")


def unreviewed_departures() -> list[str]:
    """Violations (strings) — see the module docstring. Run from the repo root."""
    cur_text = layer2._read_text(Path(QUEUE_FILE))
    if cur_text is None:
        return [f"Q39 departures audit: {QUEUE_FILE} is unreadable"]
    try:
        current = {it["func"]: it for it in json.loads(cur_text)["items"]}
    except (ValueError, KeyError, TypeError) as e:
        return [f"Q39 departures audit: working-tree {QUEUE_FILE} is not a queue ({e})"]
    out: list[str] = []
    batch = None
    try:
        a = _anchor()
        if a is None:
            return []                       # the gate is not in this history yet
        anc, other_adds = a
        diffs, gate_adds = _history(anc)
        stray = [c[:9] for c in other_adds if c not in gate_adds]
        if stray:
            raise _AuditError(f"{GATE_FILE} is added by commits not descended from the "
                              f"oldest add {anc[:9]} ({', '.join(stray)}) — no single ancestry "
                              f"chain, so no gate anchor can be chosen")
        batch = _Batch()
        # every post-gate version: the anchor's, each diff's result; plus HEAD's
        versions = [(f"anchor {anc[:9]}", f"{anc}:{QUEUE_FILE}")]
        versions += [(c[:9], dst) for c, _src, dst in diffs if set(dst) != {"0"}]
        parsed: dict[str, dict | None] = {}
        listed: dict[str, dict] = {}
        historical_chains: list[tuple[str, list[str]]] = []
        for where, obj in versions:
            if obj in parsed:
                continue
            blob = batch.get(obj)
            if blob is None:
                parsed[obj] = None
                out.append(f"Q39 departures audit: {QUEUE_FILE} version {obj} ({where}) is "
                           f"missing from the object store — its functions cannot be audited")
                continue
            items = parsed[obj] = _parse(blob)
            if items is None:
                out.append(f"Q39 departures audit: {QUEUE_FILE} version {obj} ({where}) names "
                           f"no function at all — read it with `git cat-file -p {obj}` and "
                           f"review each function it could have listed")
                continue
            for f, it in items.items():
                listed.setdefault(f, dict(it, _where=where))
                historical_chains.append((f, _chain(it.get("renamed_from"))))
        head_blob = batch.get(f"HEAD:{QUEUE_FILE}")
        head_items = (_parse(head_blob) or {}) if head_blob is not None else {}

        # renames: evidence from the current queue, every post-gate version and
        # the records — each edge only when the old name has no body in src/
        chains = [(f, _chain(it.get("renamed_from"))) for f, it in current.items()]
        chains += historical_chains
        for base in ("memory/grind", "memory/grind/_completed"):
            for p in sorted(Path(base).glob(f"*/{layer2.RECORD_NAME}")):
                for line in (layer2._read_text(p) or "").splitlines():
                    try:
                        rec = json.loads(line)
                    except ValueError:
                        continue
                    if isinstance(rec, dict) and isinstance(rec.get("func"), str):
                        chains.append((rec["func"], _chain(rec.get("renamed_from"))))
        gone = {f for f in listed if f not in current}
        relevant = [(n, ch) for n, ch in chains if set(ch) & gone]
        scan = set(gone) | {n for n, _ch in relevant} | {o for _n, ch in relevant for o in ch}
        where_now, present = _src_scan(scan)
        renamed = {old: new for new, ch in relevant for old in ch
                   if old != new and old not in present}

        # the commit each name last left the queue in (newest first)
        left_in: dict[str, tuple[str, dict]] = {}
        for f in gone:
            if f in head_items:
                left_in[f] = ("working tree", head_items[f])
        for commit, src, dst in diffs:
            before = parsed.get(src) if src in parsed else None
            if before is None and set(src) != {"0"}:
                sb = batch.get(src)
                before = parsed[src] = _parse(sb) if sb is not None else None
            after = parsed.get(dst) or {}
            for f in (before or {}):
                if f in gone and f not in after and f not in left_in:
                    left_in[f] = (commit, before[f])

        for f, it in sorted(listed.items()):
            if f not in gone:
                continue
            name, hops = f, 0
            while name in renamed and hops < 64:
                name, hops = renamed[name], hops + 1
            if name in current:
                continue
            stem = where_now.get(name) or it.get("file") or ""
            cur = layer2.current_key(name, stem) if stem else None
            dep_key, dep_where = None, "the working tree"
            commit, dep_item = left_in.get(f, (None, it))
            if commit and commit != "working tree":
                dep_where = f"commit {commit[:9]}"
                for s in dict.fromkeys(filter(None, (dep_item.get("file"), where_now.get(f),
                                                     where_now.get(name)))):
                    text = batch.get(f"{commit}:src/{s}.c")
                    k = layer2.body_key(text.decode("utf-8", "replace"), f) if text else None
                    if k:
                        dep_key = k[1]
                        break
            why = _clears(name, dep_key, cur[1] if cur else None)
            if why:
                shown = name if name == f else f"{name} (listed as {f})"
                out.append(f"{shown}: left engine/queue.json after the Q39 gate (in "
                           f"{dep_where}; listed in the {it['_where']} version) without a "
                           f"layer-2 PASS on the body that left — {why}. {_fix(name, stem)}")
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
