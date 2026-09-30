"""Q39 departures audit — completions that bypassed `queue done`.

Owner ruling Q39 gates `queue done` (engine/layer2.py). This audit is the
standing backstop for the one way around it: editing engine/queue.json by
hand. Run by tools/check_completion_integrity.py; every finding is a
violation.

WHAT IT PROVES: for every function listed in a post-gate engine/queue.json
version and not queued now (under its current name), either the body present
at the commit where its item last left the queue holds an unrevoked layer-2
PASS on that exact hash, or the latest record is a PASS on the current body —
and in both cases the function's LATEST record is not a FAIL / NEEDS_USER on
any body (fail closed).

SEMANTICS (union of versions, not adjacent diffs): the post-gate versions are
every version committed on the ancestry path from the gate's anchor commit to
HEAD (side branches included) and the anchor's own. A function that departed
before the gate never appears in one, so it is exempt without any date
cutoff. A committed departure is judged against the body AT the commit where
the item last left the queue (Q39: "a PASS for the exact code being landed");
a departure only in the working tree (HEAD lists it, the working tree does
not) against the current body. So a later naming wave, which moves every
referencing body's key, does not send completed functions back to review.

ANCHOR: the oldest commit (topological order, full history) in HEAD's history
that ADDS `GATE_FILE` (engine/layer2.py) — found by ancestry, never by date.
Any other commit adding the file must descend from it (delete + re-add);
independent adds (no single chain), an anchor with no parent in this history
(a root commit or a shallow boundary), and a grafts file fail closed; replace
refs are ignored (`--no-replace-objects`). test_departures pins GATE_FILE to
engine.layer2's own path, so moving the module fails the suite instead of
silently re-anchoring.

RENAMES. tools/naming_wave.py appends the old name to a `renamed_from` LIST on
the queue item it renames and on every layer2.jsonl line it retargets; the
audit reads those chains from the current queue, every post-gate queue
version, and the live and archived records. A chain [c0..ck] on a carrier N
only NOMINATES edges — each ci -> N and the hops c0 -> c1 ... ck -> N — and an
edge counts only when ALL of these hold, where D is the commit in which `old`
last left the queue ("the working tree" if only there, its parent then being
HEAD; for a never-listed intermediate name, renamed twice between queue
versions, D is its rename event R):
  (a) a git-visible RENAME EVENT R on the ancestry path (git's own rename
      detection, --raw -M): asm/funcs/<old>.s deleted and asm/funcs/<new>.s
      added, their contents identical once <old> is spelled <new>; and in R
      the body moves — at R^ `old` has a body and `new` none, at R `new` has
      one and `old` none, over EVERY source file (src/**/*.c, src/**/*.h,
      include/**/*.h; a C definition or an asm body such as INCLUDE_ASM);
  (b) the address in asm/funcs/<old>.s at D^ (first address comment, as the
      census reads it) equals the address in R's <old>.s;
  (c) `new` is not listed in the queue version at D^ and has no body in any
      source file at D^;
  (d) `old` has no body in any source file now and no asm/funcs/<old>.s now;
  (e) injective: no other gone name has a valid edge to the same `new` (two
      claimants -> neither counts).
There is no address fallback from a `func_XXXXXXXX` spelling. Resolution
follows valid edges and stops at the first name that is queued now (nothing to
audit) or is itself listed in a post-gate version (it is audited under its own
departure); the departed body is looked up under every name on the way.

BODY LOCATION: one pass over src/**/*.c, src/**/*.h and include/**/*.h gives a
function's current file (TU resplits and header moves); a queue item's `file`
is only the fallback.

UNPARSEABLE VERSIONS (e.g. a committed conflict): the function names in them
are recovered by pattern (`"func": "<name>"`) and audited like any other; only
a version yielding no name at all is a violation.

GIT: four processes however long the history — the grafts check, the anchor
lookup, one `git log --raw -M` over the ancestry path (queue.json, the gate
module and asm/funcs), and ONE interactive `git cat-file --batch` for every
queue version, source tree, source file and asm/funcs blob it needs. Every git failure and a missing object is a
violation: an audit that cannot read history must not pass. Under WSL, a
worktree `.git` naming a Windows gitdir (`gitdir: C:/...`) is read through
`--git-dir=/mnt/c/...`; under Windows, `gitdir: /mnt/c/...` through
`--git-dir=C:/...`.

KNOWN LIMITS: (1) a function never listed in any post-gate version is not
audited; (2) queue versions on forks that branched BEFORE the anchor are not
read, and a pre-anchor fork merged in can make a function it re-lists look
like a departure (latent false positive); (3) two static functions of the same
name in different TUs are one name here (the first body found is used);
(4) a hand-forged layer2.jsonl PASS line is trusted, exactly as the core gate
trusts it — the records are committed and reviewed, not signed.
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
_MNT_GITDIR = re.compile(r"^/mnt/([A-Za-z])/(.*)$")
_ASM_ADDR = re.compile(r"^\s*/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s", re.M)
ASM_DIR = "asm/funcs"
_SRC_PATH = re.compile(r"^(src/.+\.[ch]|include/.+\.h)$")
_ZERO = "0" * 40


class _AuditError(Exception):
    pass


def _gitdir_line(git_file_text: str) -> str | None:
    m = re.match(r"\s*gitdir:\s*(.+?)\s*$", git_file_text or "", re.S)
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

    def text(self, obj: str) -> str | None:
        b = self.get(obj)
        return None if b is None else b.decode("utf-8", "replace")

    def close(self):
        try:
            self.p.stdin.close()
        except OSError:
            pass
        rc = self.p.wait()
        if rc != 0:
            err = self.p.stderr.read().decode("utf-8", "replace").strip()
            raise _AuditError(f"git cat-file --batch exited {rc}: {err}")


def _no_grafts() -> None:
    p = _git("rev-parse", "--git-path", "info/grafts").decode().strip()
    if p and Path(p).exists():
        raise _AuditError(f"a grafts file ({p}) rewrites this history's parents — the "
                          f"audit cannot trust the ancestry it walks")


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


def _history(anc: str):
    """Queue diffs (commit, src blob, dst blob), newest first in topological
    order; the commits on the ancestry path that add GATE_FILE; and git's
    asm/funcs rename events {(old, new): [(commit, old blob, new blob)]}."""
    diffs, gate_adds, renames, commit = [], set(), {}, ""
    for line in _git("log", "--topo-order", "--full-history", "--ancestry-path", "-m", "-M",
                     "--raw", "--no-abbrev", "--format=commit %H", f"{anc}..HEAD", "--",
                     QUEUE_FILE, GATE_FILE, ASM_DIR).decode().splitlines():
        if line.startswith("commit "):
            commit = line.split()[1]
        elif line.startswith(":"):
            meta, *paths = line.split("\t")
            f = meta.split()
            if f[4].startswith("R") and len(paths) == 2:
                o, n = (Path(p) for p in paths)
                if o.parent.as_posix() == ASM_DIR == n.parent.as_posix() \
                        and o.suffix == n.suffix == ".s":
                    renames.setdefault((o.stem, n.stem), []).append((commit, f[2], f[3]))
            elif paths[0] == GATE_FILE and f[4].startswith("A"):
                gate_adds.add(commit)
            elif paths[0] == QUEUE_FILE:
                diffs.append((commit, f[2], f[3]))
    return diffs, gate_adds, renames


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


def _source_files() -> list[Path]:
    files = set()
    for pat in ("src/**/*.c", "src/**/*.h", "include/**/*.h"):
        files.update(Path(".").glob(pat))
    return sorted(files)


def _src_scan(names: set[str]) -> dict[str, str]:
    """One pass over src/ and every header: name -> the path (repo-relative)
    of the file holding its single keyable body."""
    where = {}
    for p in _source_files():
        text = layer2._read_text(p)
        if not text:
            continue
        for n in names:
            if n not in where and n in text and layer2.body_key(text, n) is not None:
                where[n] = p.as_posix()
    return where


def _key(text: str | None, name: str) -> str | None:
    k = layer2.body_key(text, name) if text else None
    return k[1] if k else None


def _addr_of(asm_text: str | None) -> str | None:
    """First address comment of a splat .s (as docs/naming/build_census.py
    reads it); no fallback from the spelling of a name."""
    m = _ASM_ADDR.search(asm_text) if asm_text else None
    return m.group(1).upper() if m else None


_WB_CACHE: dict[int, set[str]] = {}


def _has_body(text: str | None, name: str) -> bool:
    """A C definition of `name`, or an asm body (INCLUDE_ASM / .include /
    glabel block) supplying it, in this file's text."""
    if not text or name not in text:
        return False
    if inlineasm._func_body_span(text, name) is not None:
        return True
    k = hash(text)
    if k not in _WB_CACHE:
        _WB_CACHE[k] = inlineasm.whole_body_asm_funcs(text)
    return name in _WB_CACHE[k]


def _tree_sources(batch: "_Batch", rev: str) -> list[str]:
    """Blob ids of every source file (src/**/*.c|h, include/**/*.h) at `rev`,
    walked through the batch process (no extra git process)."""
    out = []

    def walk(obj, prefix):
        data = batch.get(obj)
        if data is None:
            return
        i = 0
        while i < len(data):
            sp = data.index(b" ", i)
            nul = data.index(b"\0", sp)
            mode, name = data[i:sp], data[sp + 1:nul].decode("utf-8", "replace")
            sha = data[nul + 1:nul + 21].hex()
            i = nul + 21
            path = f"{prefix}/{name}"
            if mode == b"40000":
                walk(sha, path)
            elif _SRC_PATH.match(path):
                out.append(sha)
    for top in ("src", "include"):
        walk(f"{rev}:{top}", top)
    return out


def _body_at(batch: "_Batch", rev: str | None, name: str) -> bool:
    """Does `name` have a body in any source file at `rev` (None = now)?"""
    if rev is None:
        return any(_has_body(layer2._read_text(p), name) for p in _source_files())
    return any(_has_body(batch.text(sha), name) for sha in _tree_sources(batch, rev))


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
    """None when the latest record is not a FAIL/NEEDS_USER and either a PASS
    covers the departed body (not revoked by a later record on that body) or
    the latest record is a PASS on the current body."""
    recs, where = _records(func)
    if not recs:
        return where or (f"no layer-2 record in {layer2.record_path(func).as_posix()} or "
                         f"{layer2.completed_record_path(func).as_posix()}")
    last = recs[-1]
    if last["verdict"] != "PASS":
        return (f"its latest record ({where}) is {last['verdict']} on body "
                f"{last['body_hash']}")
    if cur_key and last["body_hash"] == cur_key:
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


def _stem(path: str | None) -> str | None:
    if path and path.startswith("src/") and path.endswith(".c"):
        return path[4:-2]
    return None


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
        _no_grafts()
        a = _anchor()
        if a is None:
            return []                       # the gate is not in this history yet
        anc, other_adds = a
        diffs, gate_adds, rename_events = _history(anc)
        stray = [c[:9] for c in other_adds if c not in gate_adds]
        if stray:
            raise _AuditError(f"{GATE_FILE} is added by commits not descended from the "
                              f"oldest add {anc[:9]} ({', '.join(stray)}) — no single ancestry "
                              f"chain, so no gate anchor can be chosen")
        batch = _Batch()
        # every post-gate version: the anchor's and each diff's result
        versions = [(f"anchor {anc[:9]}", f"{anc}:{QUEUE_FILE}")]
        versions += [(c[:9], dst) for c, _src, dst in diffs if dst != _ZERO]
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
        gone = {f for f in listed if f not in current}

        # the commit each gone name last left the queue in (newest first);
        # "HEAD" = it left only in the working tree
        # (commit, the item, the queue before) — "HEAD": only in the working tree
        left_in: dict[str, tuple[str, dict, dict]] = {}
        for f in gone:
            if f in head_items:
                left_in[f] = ("HEAD", head_items[f], head_items)
        for commit, src, dst in diffs:
            if src == _ZERO:
                continue
            if src not in parsed:
                sb = batch.get(src)
                parsed[src] = _parse(sb) if sb is not None else None
            before, after = parsed[src] or {}, parsed.get(dst) or {}
            for f in before:
                if f in gone and f not in after and f not in left_in:
                    left_in[f] = (commit, before[f], before)

        # renames: every chain naming a gone name, each edge bound to identity
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
        relevant = [(n, ch) for n, ch in chains if set(ch) & gone]
        where_now = _src_scan(set(gone) | {n for n, _ch in relevant})

        def parent_of(commit):
            return "HEAD" if commit == "HEAD" else f"{commit}^"

        def queue_at(rev):
            b = batch.get(f"{rev}:{QUEUE_FILE}")
            return (_parse(b) or {}) if b is not None else {}

        def valid_edge(old, new):
            """The docstring's (a)-(d) for old -> new; (e) is applied after.
            A never-listed intermediate name (renamed twice between queue
            versions) has no departure of its own: its rename event stands in."""
            # (d) old is gone from the tree: no body, no glabel file
            if Path(f"{ASM_DIR}/{old}.s").exists() or _body_at(batch, None, old):
                return False
            events = rename_events.get((old, new), [])
            dep = left_in.get(old)
            if dep is None:
                if old in listed or not events:
                    return False
                commit = events[0][0]
                queue_before = queue_at(f"{commit}^")
            else:
                commit, _item, queue_before = dep
            par = parent_of(commit)
            # (c) new was not queued, and had no body, when old left
            if new in queue_before or _body_at(batch, par, new):
                return False
            addr_old = _addr_of(batch.text(f"{par}:{ASM_DIR}/{old}.s"))
            for r_commit, o_blob, n_blob in events:
                o_text, n_text = batch.text(o_blob), batch.text(n_blob)
                # (a) the glabel file moved unchanged but for the name ...
                if o_text is None or n_text is None or o_text.replace(old, new) != n_text:
                    continue
                # (b) ... and it is the file old had when it left the queue
                if not addr_old or _addr_of(o_text) != addr_old:
                    continue
                # (a) ... and the body moved with it
                r_par = f"{r_commit}^"
                if (_body_at(batch, r_par, old) and not _body_at(batch, r_par, new)
                        and _body_at(batch, r_commit, new) and not _body_at(batch, r_commit, old)):
                    return True
            return False

        # a chain [c0, ..., ck] on carrier N nominates each ci -> N and the
        # hops c0->c1, ..., ck->N (naming_wave appends one name per wave); only
        # edges out of a gone name or a never-listed intermediate matter
        nominated = {(old, new) for carrier, ch in relevant
                     for old, new in [*zip(ch, ch[1:] + [carrier]), *((c, carrier) for c in ch)]
                     if old != new and (old in gone or old not in listed)}
        valid = {(old, new) for old, new in sorted(nominated) if valid_edge(old, new)}
        # (e) injective, both ways: one old -> one new, one new <- one old
        olds_of = {n: {o for o, n2 in valid if n2 == n} for _o, n in valid}
        news_of = {o: {n for o2, n in valid if o2 == o} for o, _n in valid}
        renamed = {o: n for o, n in valid if len(news_of[o]) == 1 and len(olds_of[n]) == 1}

        for f, it in sorted(listed.items()):
            if f not in gone:
                continue
            names, hops = [f], 0
            while names[-1] in renamed and hops < 64:
                names.append(renamed[names[-1]])
                hops += 1
                if names[-1] in current or names[-1] in listed:
                    break
            name = names[-1]
            if name in current:
                continue
            if name != f and name in listed:
                continue                    # audited under its own departure
            path_now = where_now.get(name)
            stem = _stem(path_now) or it.get("file") or ""
            cur = _key(layer2._read_text(Path(path_now)), name) if path_now else (
                _key(layer2._read_text(Path(f"src/{stem}.c")), name) if stem else None)
            dep_key, dep_where = None, "the working tree"
            commit, dep_item, _before = left_in.get(f, (None, it, None))
            if commit and commit != "HEAD":
                dep_where = f"commit {commit[:9]}"
                paths = dict.fromkeys(filter(None, (
                    f"src/{dep_item.get('file')}.c" if dep_item.get("file") else None,
                    *(where_now.get(n) for n in names))))
                for path in paths:
                    text = batch.text(f"{commit}:{path}")
                    dep_key = next(filter(None, (_key(text, n) for n in reversed(names))), None)
                    if dep_key:
                        break
            why = _clears(name, dep_key, cur)
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
