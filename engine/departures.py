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
NOMINATES edges — each ci -> N and the hops c0 -> c1 ... ck -> N — and so does
every rename event out of a gone name (a `git revert` of a wave restores the
old name with no chain). A nominated edge counts only when ALL of these hold,
where D is the commit in which `old` last left the queue ("the working tree"
if only there, its parent then being HEAD; for a never-listed intermediate
name, renamed twice between queue versions, D is its rename event R):
  (a) a RENAME EVENT R carrying the SAME MACHINE CODE — equal ordered splat
      columns /* offset vaddr bytes */ (a wave rewrites every .s with its
      whole name map, so operand text naming other renamed functions is not
      compared; hand-written asm with no columns must match token for token
      outside the glabel line) under `glabel old` -> `glabel new` (a file-stem
      move that keeps its glabel is no rename). R is one of:
        - git's own rename detection on the ancestry path (--raw -M):
          asm/funcs/<old>.s deleted, asm/funcs/<new>.s added;
        - an in-place edit of an asm/funcs file whose stem is not the
          function's name (tools/naming_wave.py renames only files whose stem
          is in its map; it rewrites the glabel inside the others);
        - for a wave not yet committed: HEAD has <old>.s, the working tree has
          <new>.s and no <old>.s;
      and in R the body moves — at R^ `old` has a body and `new` none, at R
      `new` has one and `old` none, over EVERY source file (src/**/*.c,
      src/**/*.h, include/**/*.h; a C definition or an asm body such as
      INCLUDE_ASM). A function with NO glabel file under either name has a
      BODY-MOVE event instead: in a commit touching the name registries
      (symbol_addrs.txt, named_syms.txt, undefined_funcs_auto.txt, the census
      docs/naming/function-names.csv) or D itself, the body moves and the
      registries give `old` (at R^) and `new` (at R) the same single address;
  (b) R is the function `old` had at D^: its file there has the same machine
      columns (so the same addresses; for hand-written asm, the same tokens)
      under `glabel old` — or, for a body-move event, the registries at D^
      give `old` that same address;
  (c) `new` is not listed in the queue version at D^ and has no body in any
      source file at D^;
  (d) `old` has no body in any source file now and no asm/funcs/<old>.s now
      (checked against the directory listing, case-exact: a case-only rename
      on a case-insensitive filesystem is not "still there");
  (e) injective: no other gone name has a valid edge to the same `new` (two
      claimants -> neither counts).
There is no address fallback from a `func_XXXXXXXX` spelling. Resolution
follows valid edges and stops at the first name that is queued now (nothing to
audit) or is itself listed in a post-gate version (it is audited under its own
departure); the departed body is looked up under every name on the way, in
FORWARD order (the listed name first: a later name may have belonged to a
different function at D).

BODY LOCATION: one pass over src/**/*.c, src/**/*.h and include/**/*.h gives a
function's current file (TU resplits and header moves); a queue item's `file`
is only the fallback.

UNPARSEABLE VERSIONS (e.g. a committed conflict): the function names in them
are recovered by pattern (`"func": "<name>"`) and audited like any other; only
a version yielding no name at all is a violation.

GIT: four processes however long the history — the grafts check, the anchor
lookup, one `git log --raw -M` over the ancestry path (queue.json, the gate
module, asm/funcs and the name registries), and ONE interactive
`git cat-file --batch` for every queue version, source tree, source file,
glabel file and registry it needs. Every git failure and every missing object
(a queue version, a source tree or blob — "unreadable" never reads as "no
body") is a violation: an audit that cannot read history must not pass. Under
WSL, a worktree `.git` naming a Windows gitdir (`gitdir: C:/...`) is read
through `--git-dir=/mnt/c/...`; under Windows, `gitdir: /mnt/c/...` through
`--git-dir=C:/...`.

BY DESIGN: a naming wave on one branch while another completes the function
under its OLD name, then a merge, flags the function — the wave changed the
key of the body being landed, and review-discipline-before-commit.md already
rules that a wave voids pending PASSes (re-review, record, done).

KNOWN LIMITS: (1) a function never listed in any post-gate version is not
audited; (2) queue versions on forks that branched BEFORE the anchor are not
read, and a pre-anchor fork merged in can make a function it re-lists look
like a departure (latent false positive); (3) two static functions of the same
name in different TUs are one name here (the first body found is used).
Deliberate RECORD FORGERY is caught by review, not by this tool (the core gate
shares this): (K1) a hand-crafted commit that fakes a rename — renames the
glabel file and moves the body of a function that was not renamed; (K2) forged
layer2.jsonl lines, PASS included — records are committed and reviewed, not
signed; (K3) a forged `renamed_from` chain riding a GENUINE rename (the edge is
real, so it resolves); (K4) edited machine columns in a glabel file (the build
never assembles a dead .s, and a live one is caught by the oracle).
"""
from __future__ import annotations

import csv
import io
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
ASM_DIR = "asm/funcs"
# name -> address registries a naming wave rewrites (tools/naming_wave.py
# SYMBOL_FILES + the census): the address source for a function with no .s
NAME_REGISTRIES = ("symbol_addrs.txt", "named_syms.txt", "undefined_funcs_auto.txt",
                   "docs/naming/function-names.csv")
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
    """One walk of the ancestry path: queue diffs (commit, src blob, dst blob),
    newest first in topological order; the commits adding GATE_FILE; git's
    asm/funcs rename events {(old stem, new stem): [(commit, old blob, new
    blob, old path)]}; in-place asm/funcs edits [(commit, path, old blob, new
    blob)] (a wave rewrites the glabel of a file whose stem is not the
    function's name without moving it); and the commits touching the name
    registries (candidate rename commits for a function with no .s at all)."""
    diffs, gate_adds, renames, edits, naming, commit = [], set(), {}, [], [], ""
    for line in _git("log", "--topo-order", "--full-history", "--ancestry-path", "-m", "-M",
                     "--raw", "--no-abbrev", "--format=commit %H", f"{anc}..HEAD", "--",
                     QUEUE_FILE, GATE_FILE, ASM_DIR, *NAME_REGISTRIES).decode().splitlines():
        if line.startswith("commit "):
            commit = line.split()[1]
        elif line.startswith(":"):
            meta, *paths = line.split("\t")
            f = meta.split()
            if f[4].startswith("R") and len(paths) == 2:
                o, n = (Path(p) for p in paths)
                if o.parent.as_posix() == ASM_DIR == n.parent.as_posix() \
                        and o.suffix == n.suffix == ".s":
                    renames.setdefault((o.stem, n.stem), []).append(
                        (commit, f[2], f[3], paths[0]))
            elif paths[0] == GATE_FILE and f[4].startswith("A"):
                gate_adds.add(commit)
            elif paths[0] == QUEUE_FILE:
                diffs.append((commit, f[2], f[3]))
            elif paths[0] in NAME_REGISTRIES:
                if commit not in naming:
                    naming.append(commit)
            elif f[4].startswith("M") and paths[0].startswith(ASM_DIR + "/") \
                    and paths[0].endswith(".s"):
                edits.append((commit, paths[0], f[2], f[3]))
    return diffs, gate_adds, renames, edits, naming


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


# splat's machine columns: /* <rom offset> <vaddr> <instruction bytes> */
_MACHINE = re.compile(r"/\*\s*([0-9A-Fa-f]+)\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s*\*/")
_GLABEL = re.compile(r"^\s*glabel\s+(\S+)", re.M)


def _machine(asm_text: str | None) -> list[tuple[str, str, str]]:
    return [tuple(x.upper() for x in m.groups()) for m in _MACHINE.finditer(asm_text or "")]


def same_asm(old_text: str | None, new_text: str | None, old: str, new: str) -> bool:
    """Is new_text the glabel file old_text renamed old -> new? Identity is
    the ordered splat machine columns (offset, vaddr, bytes) — which a naming
    wave never touches — plus `glabel old` -> `glabel new`. Text is NOT
    compared: a wave rewrites every .s with its whole map (jal / .word /
    %hi/%lo operands naming other renamed functions change too)."""
    g_old, g_new = _GLABEL.search(old_text or ""), _GLABEL.search(new_text or "")
    if not (g_old and g_new and g_old.group(1) == old and g_new.group(1) == new):
        return False
    ident = asm_identity(old_text)
    return bool(ident) and ident == asm_identity(new_text)


def asm_identity(asm_text: str | None) -> list:
    """What makes a glabel file THIS function: its ordered splat machine
    columns; for hand-written asm with none (e.g. the GTE register helpers),
    every token but the glabel line's — nothing normalised."""
    cols = _machine(asm_text)
    if cols:
        return cols
    return _GLABEL.sub("", asm_text or "", count=1).split()


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


_SYM_LINE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]+)\s*;", re.M)


def _name_addrs(batch: "_Batch", rev: str, name: str) -> set[str]:
    """Every address the name registries at `rev` give `name`: the symbol
    files' `name = 0xADDR;` lines and the census rows naming it (current name,
    glabel or alias) — how tools/naming_wave.py maps names to addresses."""
    out = set()
    for reg in NAME_REGISTRIES:
        text = batch.text(f"{rev}:{reg}")
        if not text or name not in text:
            continue
        if reg.endswith(".csv"):
            for row in csv.DictReader(io.StringIO(text)):
                aliases = {a.strip() for a in (row.get("aliases") or "").split(";")}
                if name in ((row.get("current_name") or "").strip(),
                            (row.get("glabel") or "").strip()) or name in aliases:
                    out.add((row.get("address") or "").strip().upper().replace("0X", ""))
        else:
            out.update(m.group(2).upper().zfill(8) for m in _SYM_LINE.finditer(text)
                       if m.group(1) == name)
    out.discard("")
    return out


def _tree_entries(data: bytes) -> list[tuple[bytes, str, str]]:
    out, i = [], 0
    while i < len(data):
        sp = data.index(b" ", i)
        nul = data.index(b"\0", sp)
        out.append((data[i:sp], data[sp + 1:nul].decode("utf-8", "replace"),
                    data[nul + 1:nul + 21].hex()))
        i = nul + 21
    return out


def _tree_sources(batch: "_Batch", rev: str) -> list[str]:
    """Blob ids of every source file (src/**/*.c|h, include/**/*.h) at `rev`,
    walked through the batch process (no extra git process). A tree the object
    store cannot produce fails the audit: "unreadable" never reads as "no
    body". (A commit with no include/ at all simply has none.)"""
    out = []

    def tree(obj, what):
        data = batch.get(obj)
        if data is None:
            raise _AuditError(f"the {what} tree at {rev} is missing from the object store")
        return data

    def walk(sha, prefix):
        for mode, name, sub in _tree_entries(tree(sha, prefix)):
            path = f"{prefix}/{name}"
            if mode == b"40000":
                walk(sub, path)
            elif _SRC_PATH.match(path):
                out.append(sub)
    for mode, name, sha in _tree_entries(tree(f"{rev}^{{tree}}", "root")):
        if mode == b"40000" and name in ("src", "include"):
            walk(sha, name)
    return out


def _body_at(batch: "_Batch", rev: str | None, name: str) -> bool:
    """Does `name` have a body in any source file at `rev` (None = now)? A
    source blob the object store cannot produce fails the audit."""
    if rev is None:
        return any(_has_body(layer2._read_text(p), name) for p in _source_files())
    for sha in _tree_sources(batch, rev):
        text = batch.text(sha)
        if text is None:
            raise _AuditError(f"source blob {sha} at {rev} is missing from the object store")
        if _has_body(text, name):
            return True
    return False


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
        diffs, gate_adds, rename_events, asm_edits, naming_commits = _history(anc)
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

        # the glabel files present NOW, from the directory listing: a
        # case-only rename must not read as "still there" on a
        # case-insensitive filesystem
        asm_now = set(os.listdir(ASM_DIR)) if os.path.isdir(ASM_DIR) else set()

        # a wave rewrites the glabel of a .s whose stem is not the function's
        # name IN PLACE (the file is not renamed): that is a rename event too
        if gone:
            for commit, path, o_blob, n_blob in asm_edits:
                g_o = _GLABEL.search(batch.text(o_blob) or "")
                g_n = _GLABEL.search(batch.text(n_blob) or "")
                if g_o and g_n and g_o.group(1) != g_n.group(1):
                    rename_events.setdefault((g_o.group(1), g_n.group(1)), []).append(
                        (commit, o_blob, n_blob, path))

        # a rename still in the working tree (an uncommitted wave): HEAD has
        # <old>.s, the working tree has <new>.s and no <old>.s, same machine code
        head_asm = {}
        tree = batch.get(f"HEAD:{ASM_DIR}")
        for _mode, name, sha in (_tree_entries(tree) if tree else []):
            head_asm[name] = sha
        wt_new = [p for p in sorted(Path(ASM_DIR).glob("*.s")) if p.name not in head_asm]
        for old in sorted(gone):
            sha = head_asm.get(f"{old}.s")
            if sha is None or f"{old}.s" in asm_now:
                continue
            o_text = batch.text(sha)
            for p in wt_new:
                if same_asm(o_text, layer2._read_text(p), old, p.stem):
                    rename_events.setdefault((old, p.stem), []).append(
                        ("WORKTREE", sha, None, f"{ASM_DIR}/{old}.s"))

        where_now = _src_scan(set(gone) | {n for n, _ch in relevant}
                              | {n for (o, n) in rename_events if o in gone})

        def parent_of(commit):
            return "HEAD" if commit in ("HEAD", "WORKTREE") else f"{commit}^"

        def queue_at(rev):
            b = batch.get(f"{rev}:{QUEUE_FILE}")
            return (_parse(b) or {}) if b is not None else {}

        def valid_edge(old, new):
            """The docstring's (a)-(d) for old -> new; (e) is applied after.
            A never-listed intermediate name (renamed twice between queue
            versions) has no departure of its own: its rename event stands in."""
            # (d) old is gone from the tree: no body, no glabel file
            if f"{old}.s" in asm_now or _body_at(batch, None, old):
                return False
            events = rename_events.get((old, new), [])
            dep = left_in.get(old)
            if dep is None:
                if old in listed or not events:
                    return False
                commit = events[0][0]
                queue_before = queue_at(parent_of(commit))
            else:
                commit, _item, queue_before = dep
            par = parent_of(commit)
            # (c) new was not queued, and had no body, when old left
            if new in queue_before or _body_at(batch, par, new):
                return False

            def body_moved(r_commit):
                r_par = parent_of(r_commit)
                r_rev = None if r_commit == "WORKTREE" else r_commit
                return (_body_at(batch, r_par, old) and not _body_at(batch, r_par, new)
                        and _body_at(batch, r_rev, new) and not _body_at(batch, r_rev, old))

            for r_commit, o_blob, n_blob, o_path in events:
                wt = r_commit == "WORKTREE"
                o_text = batch.text(o_blob)
                n_text = layer2._read_text(Path(f"{ASM_DIR}/{new}.s")) if wt else batch.text(n_blob)
                # (a) the same machine code under the new glabel ...
                if not same_asm(o_text, n_text, old, new):
                    continue
                # (b) ... and it is the file old had when it left the queue
                # (same machine columns — so the same addresses — as at D^)
                at_d = batch.text(f"{par}:{o_path}")
                g = _GLABEL.search(at_d or "")
                ident_old = asm_identity(at_d) if g and g.group(1) == old else []
                if not ident_old or asm_identity(o_text) != ident_old:
                    continue
                # (a) ... and the body moved with it
                if body_moved(r_commit):
                    return True
            if events:
                return False
            # no glabel file for either name: a BODY-MOVE event — the body moves
            # old -> new in one commit (a naming commit, or D) and the name
            # registries give both names the same single address, which is
            # also old's address at D^
            addr_d = _name_addrs(batch, par, old)
            if len(addr_d) != 1:
                return False
            for r_commit in dict.fromkeys([*naming_commits, *([commit] if commit != "HEAD" else [])]):
                if batch.get(f"{r_commit}^:{ASM_DIR}/{old}.s") is not None \
                        or batch.get(f"{r_commit}:{ASM_DIR}/{new}.s") is not None:
                    continue
                if (_name_addrs(batch, f"{r_commit}^", old) == addr_d
                        == _name_addrs(batch, r_commit, new) and body_moved(r_commit)):
                    return True
            return False

        # edges are nominated by renamed_from chains — a chain [c0, ..., ck]
        # on carrier N gives each ci -> N and the hops c0->c1, ..., ck->N — and
        # straight by git's rename events (so a `git revert` of a wave, which
        # restores the old name with no chain, nominates the reverse edge);
        # only edges out of a gone name or a never-listed intermediate matter
        nominated = {(old, new) for carrier, ch in relevant
                     for old, new in [*zip(ch, ch[1:] + [carrier]), *((c, carrier) for c in ch)]
                     if old != new and (old in gone or old not in listed)}
        nominated |= {(old, new) for (old, new) in rename_events if old != new and old in gone}
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
                    dep_key = next(filter(None, (_key(text, n) for n in names)), None)
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
