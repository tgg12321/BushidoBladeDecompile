"""Q39 departures audit — completions that bypassed `queue done`.

Owner ruling Q39 gates `queue done` (engine/layer2.py). This audit is the
standing backstop for the one way around it: editing engine/queue.json by
hand. Run by tools/check_completion_integrity.py; every finding is a
violation.

SEMANTICS (union of versions, not adjacent diffs): a function is a DEPARTURE
when it is listed in ANY version of engine/queue.json since the gate existed —
every version committed on the ancestry path from the gate's anchor commit to
HEAD, the anchor's own, and the working tree's — is NOT listed in the working
tree's queue now (under its current name), and has no layer-2 PASS on its
current body. A function that departed before the gate never appears in a
post-gate version, so it is exempt without any date cutoff.

ANCHOR: the oldest commit in HEAD's history that ADDS `GATE_FILE`
(engine/layer2.py) — found by ancestry, never by date, so a skewed commit date
cannot move it. A move of the gate module would silently re-anchor it later;
test_departures_anchor pins GATE_FILE to engine.layer2's own path so such a
move fails the suite instead.

RENAMES need positive evidence: tools/naming_wave.py appends the old name to a
`renamed_from` LIST on the queue item it renames and on every layer2.jsonl line
it retargets. An old name found in any such chain resolves to the name that
carries it now; nothing else (no field-similarity guess) counts.

GIT: three processes however long the history — the anchor lookup, one
`git log --raw` over the ancestry path, one `git cat-file --batch` for every
queue.json blob. Every git failure, a missing blob, and an unparseable queue
version is itself a violation: an audit that cannot read history must not pass.
"""
from __future__ import annotations

import json
import subprocess
from pathlib import Path

from . import layer2

GATE_FILE = "engine/layer2.py"
QUEUE_FILE = "engine/queue.json"


class _AuditError(Exception):
    pass


def _git(*args: str, stdin: bytes | None = None) -> bytes:
    try:
        r = subprocess.run(["git", *args], input=stdin, capture_output=True)
    except OSError as e:
        raise _AuditError(f"git {args[0]} could not run: {e}") from None
    if r.returncode != 0:
        err = r.stderr.decode("utf-8", "replace").strip().splitlines()
        raise _AuditError(f"git {' '.join(args[:2])} failed (exit {r.returncode}): "
                          f"{err[-1] if err else 'no message'}")
    return r.stdout


def anchor() -> str | None:
    """The oldest commit in HEAD's history adding GATE_FILE (None: no gate)."""
    out = _git("log", "--diff-filter=A", "--format=%H", "HEAD", "--", GATE_FILE).split()
    return out[-1].decode() if out else None


def _queue_blobs(anc: str) -> list[tuple[str, str]]:
    """(where, object name) for every post-gate queue.json version."""
    blobs = [(f"anchor {anc[:9]}", f"{anc}:{QUEUE_FILE}")]
    commit = ""
    for line in _git("log", "--ancestry-path", "-m", "--raw", "--no-abbrev",
                     "--format=commit %H", f"{anc}..HEAD", "--", QUEUE_FILE).decode().splitlines():
        if line.startswith("commit "):
            commit = line[7:]
        elif line.startswith(":"):
            dst = line.split()[3]
            if set(dst) != {"0"}:            # all-zero = the file was deleted
                blobs.append((commit[:9], dst))
    seen, out = set(), []
    for where, obj in blobs:
        if obj not in seen:
            seen.add(obj)
            out.append((where, obj))
    return out


def _cat_blobs(objs: list[str]) -> list[bytes | None]:
    """Contents of every object in ONE `git cat-file --batch` (None = missing)."""
    raw = _git("cat-file", "--batch", stdin=("\n".join(objs) + "\n").encode())
    out, i = [], 0
    for _ in objs:
        j = raw.index(b"\n", i)
        header = raw[i:j].split()
        if len(header) == 2 and header[1] == b"missing":
            out.append(None)
            i = j + 1
            continue
        size = int(header[2])
        out.append(raw[j + 1:j + 1 + size])
        i = j + 1 + size + 1                 # content + trailing LF
    return out


def _items(text: bytes | str) -> dict[str, dict]:
    q = json.loads(text)
    return {it["func"]: it for it in q["items"]}


def _rename_map(current: dict[str, dict]) -> dict[str, str]:
    """old name -> the name that carries it now, from naming-wave evidence
    only: `renamed_from` chains on current queue items and on layer2.jsonl
    lines (live and archived ledgers)."""
    m: dict[str, str] = {}

    def add(name, chain):
        # naming_wave refuses any other shape; one here is not evidence
        for old in ([chain] if isinstance(chain, str) else
                    chain if isinstance(chain, list) else []):
            if isinstance(old, str) and old != name:
                m[old] = name
    for f, it in current.items():
        add(f, it.get("renamed_from"))
    for base in ("memory/grind", "memory/grind/_completed"):
        for p in sorted(Path(base).glob(f"*/{layer2.RECORD_NAME}")):
            for line in (layer2._read_text(p) or "").splitlines():
                try:
                    rec = json.loads(line)
                except ValueError:
                    continue
                if isinstance(rec, dict) and isinstance(rec.get("func"), str):
                    add(rec["func"], rec.get("renamed_from"))
    return m


def _fix(func: str, stem: str) -> str:
    return (f"Fix: put it back (`python3 -m engine.cli queue reopen {func} --file {stem} "
            f"--reason \"Q39: left the queue without a layer-2 PASS\"`), or have a fresh "
            f"cheat-reviewer rule on its current body and record the verdict "
            f"(`python3 -m engine.cli layer2 record {func} --reviewer <id> --scope "
            f"<match|cheat-cleanup|auth> --verdict-file <its JSON>`).")


def pass_on_current_body(func: str, stem: str) -> str | None:
    """None when func's latest record (live ledger, else the archived one) is a
    PASS on its current body; otherwise why not."""
    key = layer2.current_key(func, stem)
    if key is None:
        return f"no single body for {func} in src/{stem}.c"
    for p in (layer2.record_path(func), layer2.completed_record_path(func)):
        if not p.exists():
            continue
        try:
            recs = layer2.read_records(func, p)
        except ValueError as e:
            return str(e)
        if not recs:
            continue
        last = recs[-1]
        if last["verdict"] != "PASS":
            return f"its latest record ({p.as_posix()}) is {last['verdict']}"
        if last["body_hash"] != key[1]:
            return (f"its latest PASS ({p.as_posix()}) covers body {last['body_hash']}, "
                    f"the current body is {key[1]}")
        return None
    return (f"no layer-2 record in {layer2.record_path(func).as_posix()} or "
            f"{layer2.completed_record_path(func).as_posix()}")


def unreviewed_departures() -> list[str]:
    """Violations (strings) — see the module docstring. Run from the repo root."""
    try:
        cur_text = layer2._read_text(Path(QUEUE_FILE))
        if cur_text is None:
            return [f"Q39 departures audit: {QUEUE_FILE} is unreadable"]
        try:
            current = _items(cur_text)
        except (ValueError, KeyError, TypeError) as e:
            return [f"Q39 departures audit: working-tree {QUEUE_FILE} is not a queue ({e})"]
        anc = anchor()
        if anc is None:
            return []                       # the gate is not in this history yet
        versions = _queue_blobs(anc)
        contents = _cat_blobs([obj for _w, obj in versions])
    except _AuditError as e:
        return [f"Q39 departures audit could not read git history — {e}. The audit "
                f"fails closed; run it where git works on this tree."]
    out = []
    listed: dict[str, dict] = {}
    for (where, obj), blob in zip(versions, contents):
        if blob is None:
            out.append(f"Q39 departures audit: {QUEUE_FILE} version {obj} ({where}) is missing "
                       f"from the object store — its functions cannot be audited")
            continue
        try:
            items = _items(blob)
        except (ValueError, KeyError, TypeError) as e:
            out.append(f"Q39 departures audit: {QUEUE_FILE} version {obj} ({where}) is "
                       f"unparseable ({e}) — the functions it lists cannot be audited; read it "
                       f"with `git cat-file -p {obj}` and review each one that is no longer queued")
            continue
        for f, it in items.items():
            listed.setdefault(f, dict(it, _where=where))
    renamed = _rename_map(current)
    for f, it in sorted(listed.items()):
        name, hops = f, 0
        while name in renamed and hops < 64:
            name, hops = renamed[name], hops + 1
        if name in current:
            continue
        stem = it.get("file") or layer2.locate_stem(name) or ""
        why = pass_on_current_body(name, stem)
        if why:
            shown = name if name == f else f"{name} (listed as {f})"
            out.append(f"{shown}: left engine/queue.json after the Q39 gate (listed in the "
                       f"{it['_where']} version) without a layer-2 PASS on its current body — "
                       f"{why}. {_fix(name, stem)}")
    return out
