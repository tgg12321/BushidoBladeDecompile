# Board Sync (Phase 1) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build `tools/board_sync.py` — a standalone, idempotent tool that renders the BB2 decomp engine state (`engine/queue.json` + `memory/wip/` + `inline_asm_canonical.txt` + optionally the completed-function inventory) into a GitHub Projects v2 board as a one-way projection, never writing back to engine state.

**Architecture:** One self-contained Python script (mirroring `tools/metrics/sync.py`) with three responsibility groups: (1) a **pure state collector** that turns engine files into a `desired` dict, (2) a **pure reconciler** that diffs `desired` against the current board and emits a minimal action list that never deletes, (3) a **GitHub client** that talks to Projects v2 via `gh api graphql` through a single mockable `gh_graphql()` chokepoint. The pure halves are fully unit-tested offline; the client is tested by stubbing the chokepoint.

**Tech Stack:** Python 3 (Windows-side: `python` 3.9.5 / `python3` 3.12.8), stdlib only (`argparse`, `json`, `subprocess`, `re`, `time`, `pathlib`), and the `gh` CLI (2.88.1, authenticated with the `project` scope). No third-party deps. Tests use the repo's hand-rolled `check()`/`eq()` runner (no pytest). All files saved with **LF line endings**.

---

## Background facts the implementer needs (verified)

- **`board_sync.py` is Windows-side** because `gh` auth (keyring, `project` scope) lives on Windows. It calls `gh` via `subprocess.run([...])` with **list args (no shell)** and passes the query + variables as a JSON body on stdin (see next bullet), so there is **zero quoting hazard** — do NOT use `tmp/*.graphql` files or heredocs.
- **`gh api graphql` variable passing:** send variables as a JSON request body — `json.dumps({"query": <str>, "variables": <dict>})` piped to `gh api graphql --input -` on stdin (real Python objects: strings, numbers, lists, dicts all serialize correctly). Do NOT use `-f`/`-F` flags: `-F` does NOT parse a JSON-array string into a GraphQL list (verified against gh 2.88.1 — `-F 'ids=["A","B"]'` is sent as one scalar string), so single-select option lists silently fail. Always add header `-H "X-Github-Next-Global-ID: 1"`.
- **Single-select fields:** each option needs `name`, `color`, `description` (all required; `description` may be `""`). `color` ∈ `GRAY BLUE GREEN YELLOW ORANGE RED PINK PURPLE`. Setting an item's single-select value requires the **option id**, not the label — so the client must build and cache an option-label→id map per field.
- **Engine status → board column (1:1, the spec's locked mapping):** `active`→`Backlog`, `authorize`→`Needs-Decision`, `parked`→`Blocked`; completed (not in queue)→`Done`+archived.
- **`engine/queue.json` item schema:** `func` (str), `file` (str stem, no `.c`), `distance` (int; `-1`=unscored), `verdict` (str), `rules` (int), `status` (str), `park_reason` (only when parked), `scorable` (only when false). Read it with plain `json.load` (do NOT `import engine.queue` — that pulls the whole WSL engine).
- **Status enum:** `active`, `authorize`, `parked`. **Verdict enum:** `C`, `ASM-PARTIAL`, `ASM-SUSPECT`, `ASM-STRUCTURAL`, `ASM-WHOLE`, `JTBL-INFRA`.
- **WIP check:** `memory/wip/<func>/` directory existence (skip `README.md`).
- **Completed inventory (for `--seed-done` only):** parse `build/bb2.map`. Object lines look like `` .text          0x00000000        0x0 build/src/<stem>.o`` and symbol lines look like `                0x800164ac                func_800164AC``. Track the current object stem; assign each following symbol to it. Exclude the 4 known non-functions: `g_module_func_tbl`, `g_module_type_tbl`, `func_80037F08_ret`, `cdrom_IrqHandler`. `completed = inventory_names − queue_funcs` (≈1024).
- **Test harness:** no pytest. New test file `tools/test_board_sync.py`, self-contained, importing the module via `sys.path.insert(0, str(_REPO / "tools")); import board_sync`. Run: `python tools/test_board_sync.py` (exit 0 = pass). Works cross-platform (no real `gh` needed — the client is stubbed).
- **Commits:** prefix `board:`. Append the repo's `Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>` trailer per convention. Multi-line messages via `git commit -F tmp/<file>.txt`.

## File structure

- **Create:** `tools/board_sync.py` — the whole tool (3 banner-separated sections: collector / reconciler / gh-client + `main()`).
- **Create:** `tools/test_board_sync.py` — the unit tests.
- **Modify (Task 11):** `docs/STATUS.md` or a new `docs/board.md` — a short usage doc; and `CLAUDE.md` is **not** touched in Phase 1 (the workflow change lands with Phase 5).

Single-file tool is the established convention here (`tools/metrics/sync.py` is one file). Keep the pure logic free of network/`gh` calls so it stays unit-testable.

---

## Task 1: Scaffold the module + constants + queue loader

**Files:**
- Create: `tools/board_sync.py`
- Test: `tools/test_board_sync.py`

- [ ] **Step 1: Write the failing test**

Create `tools/test_board_sync.py`:

```python
#!/usr/bin/env python3
"""Unit tests for tools/board_sync.py.

Run: python tools/test_board_sync.py   (exit 0 = pass)
"""
from __future__ import annotations

import contextlib
import io
import json
import sys
import tempfile
from pathlib import Path

_REPO = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(_REPO / "tools"))
import board_sync  # noqa: E402

_passed = _failed = 0

def check(desc, cond):
    global _passed, _failed
    if cond:
        _passed += 1
    else:
        _failed += 1
        print(f"  FAIL: {desc}", file=sys.stderr)

def eq(desc, got, want):
    check(f"{desc} (got {got!r}, want {want!r})", got == want)


def test_load_queue():
    with tempfile.TemporaryDirectory() as td:
        p = Path(td) / "queue.json"
        p.write_text(json.dumps({"items": [
            {"func": "func_1", "file": "a", "distance": 9, "verdict": "C", "rules": 3, "status": "active"},
        ]}), encoding="utf-8")
        items = board_sync.load_queue(p)
        eq("loads one item", len(items), 1)
        eq("item func", items[0]["func"], "func_1")

def test_load_queue_missing():
    items = board_sync.load_queue(Path("does/not/exist.json"))
    eq("missing queue -> empty list", items, [])


def main():
    test_load_queue()
    test_load_queue_missing()
    print(f"\n{_passed} passed, {_failed} failed")
    return 1 if _failed else 0

if __name__ == "__main__":
    sys.exit(main())
```

- [ ] **Step 2: Run test to verify it fails**

Run: `python tools/test_board_sync.py`
Expected: FAIL — `ModuleNotFoundError: No module named 'board_sync'` (module not created yet).

- [ ] **Step 3: Write minimal implementation**

Create `tools/board_sync.py`:

```python
#!/usr/bin/env python3
"""tools/board_sync.py — one-way projection of the BB2 decomp engine state
into a GitHub Projects v2 board.

Reads engine/queue.json (+ memory/wip/ + inline_asm_canonical.txt, and the
completed-function inventory for --seed-done) and reconciles a GitHub Project
("BB2 Decomp") to match. The board is a MIRROR: this tool never writes back to
engine state, and never deletes board items (only add / update / archive).

Usage:
    python tools/board_sync.py --dry-run        # print the change set, mutate nothing
    python tools/board_sync.py                   # apply: create/update active cards
    python tools/board_sync.py --seed-done       # also backfill completed funcs as archived

Idempotent: re-running with no engine changes makes zero mutations.
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_QUEUE = ROOT / "engine" / "queue.json"
DEFAULT_WIP_DIR = ROOT / "memory" / "wip"
DEFAULT_MAP = ROOT / "build" / "bb2.map"
PROJECT_TITLE = "BB2 Decomp"

# Engine status -> board column (the spec's locked 1:1 mapping).
ENGINE_STATUS_TO_COLUMN = {"active": "Backlog", "authorize": "Needs-Decision", "parked": "Blocked"}

STATUS_OPTIONS = ["Backlog", "Needs-Decision", "Blocked", "In-Progress", "In-Review", "Done"]
VERDICT_OPTIONS = ["C", "ASM-PARTIAL", "ASM-SUSPECT", "ASM-STRUCTURAL", "ASM-WHOLE", "JTBL-INFRA"]
WIP_OPTIONS = ["yes", "no"]
ACTIVE_COLUMNS = {"Backlog", "Needs-Decision", "Blocked"}

# Option colors (single-select options require a color).
OPTION_COLORS = {
    "Backlog": "GRAY", "Needs-Decision": "YELLOW", "Blocked": "RED",
    "In-Progress": "BLUE", "In-Review": "PURPLE", "Done": "GREEN",
    "yes": "BLUE", "no": "GRAY",
}

# (name, dataType, options)  — options is None for non-single-select fields.
FIELD_SPECS = [
    ("Status", "SINGLE_SELECT", STATUS_OPTIONS),
    ("Verdict", "SINGLE_SELECT", VERDICT_OPTIONS),
    ("WIP", "SINGLE_SELECT", WIP_OPTIONS),
    ("File", "TEXT", None),
    ("Distance", "NUMBER", None),
    ("Rules", "NUMBER", None),
]

# Non-functions that appear in build/bb2.map's .text symbol list (excluded from inventory).
_MAP_EXCLUDE = {"g_module_func_tbl", "g_module_type_tbl", "func_80037F08_ret", "cdrom_IrqHandler"}


# ---------------------------------------------------------------------------
# state collector (pure)
# ---------------------------------------------------------------------------

def load_queue(path):
    """Parse queue.json -> list of item dicts. Missing file -> [] (non-fatal)."""
    path = Path(path)
    if not path.exists():
        return []
    return json.loads(path.read_text(encoding="utf-8")).get("items", [])
```

- [ ] **Step 4: Run test to verify it passes**

Run: `python tools/test_board_sync.py`
Expected: PASS — `2 passed, 0 failed`.

- [ ] **Step 5: Commit**

```bash
git add tools/board_sync.py tools/test_board_sync.py
git commit -m "board: scaffold board_sync.py + queue loader (Phase 1)"
```

---

## Task 2: Desired-state collector (status mapping + WIP)

**Files:**
- Modify: `tools/board_sync.py` (add to the "state collector" section)
- Test: `tools/test_board_sync.py`

- [ ] **Step 1: Write the failing test**

Add to `tools/test_board_sync.py` (new test functions + register in `main()`):

```python
def test_wip_exists():
    with tempfile.TemporaryDirectory() as td:
        wip = Path(td)
        (wip / "func_has").mkdir()
        eq("wip present", board_sync.wip_exists("func_has", wip), True)
        eq("wip absent", board_sync.wip_exists("func_none", wip), False)

def test_build_desired_status_mapping():
    items = [
        {"func": "func_a", "file": "x", "distance": 9, "verdict": "C", "rules": 3, "status": "active"},
        {"func": "func_b", "file": "y", "distance": 4, "verdict": "ASM-STRUCTURAL", "rules": 1, "status": "authorize"},
        {"func": "func_c", "file": "z", "distance": -1, "verdict": "ASM-SUSPECT", "rules": 2, "status": "parked"},
    ]
    with tempfile.TemporaryDirectory() as td:
        desired = board_sync.build_desired_from_queue(items, Path(td))
    eq("active -> Backlog", desired["func_a"]["fields"]["Status"], "Backlog")
    eq("authorize -> Needs-Decision", desired["func_b"]["fields"]["Status"], "Needs-Decision")
    eq("parked -> Blocked", desired["func_c"]["fields"]["Status"], "Blocked")
    eq("verdict carried", desired["func_a"]["fields"]["Verdict"], "C")
    eq("distance carried", desired["func_a"]["fields"]["Distance"], 9)
    eq("rules carried", desired["func_a"]["fields"]["Rules"], 3)
    eq("file carried", desired["func_a"]["fields"]["File"], "x")
    eq("wip default no", desired["func_a"]["fields"]["WIP"], "no")
    eq("active not archived", desired["func_a"]["archived"], False)
```

Register both in `main()` (before the summary print):

```python
    test_wip_exists()
    test_build_desired_status_mapping()
```

- [ ] **Step 2: Run test to verify it fails**

Run: `python tools/test_board_sync.py`
Expected: FAIL — `AttributeError: module 'board_sync' has no attribute 'wip_exists'`.

- [ ] **Step 3: Write minimal implementation**

Append to the "state collector" section of `tools/board_sync.py`:

```python
def wip_exists(func, wip_dir):
    """True iff memory/wip/<func>/ exists (a checkpoint is present)."""
    return (Path(wip_dir) / func).is_dir()


def build_desired_from_queue(items, wip_dir):
    """Turn queue items into desired board cards.

    desired[func] = {"fields": {...}, "archived": bool}
    """
    desired = {}
    for it in items:
        col = ENGINE_STATUS_TO_COLUMN.get(it["status"], "Blocked")
        desired[it["func"]] = {
            "fields": {
                "Status": col,
                "Verdict": it["verdict"],
                "WIP": "yes" if wip_exists(it["func"], wip_dir) else "no",
                "File": it["file"],
                "Distance": it["distance"],
                "Rules": it["rules"],
            },
            "archived": False,
        }
    return desired
```

- [ ] **Step 4: Run test to verify it passes**

Run: `python tools/test_board_sync.py`
Expected: PASS — `4 passed, 0 failed`.

- [ ] **Step 5: Commit**

```bash
git add tools/board_sync.py tools/test_board_sync.py
git commit -m "board: desired-state collector (status mapping + WIP flag)"
```

---

## Task 3: The reconciler (pure) — add / update / archive, never delete

**Files:**
- Modify: `tools/board_sync.py` (new "reconciler" section)
- Test: `tools/test_board_sync.py`

This is the heart. A `current` board snapshot is a list of items:
`{"item_id": str, "title": func, "is_archived": bool, "fields": {name: value}}`.
Number field values come back as floats, so comparison must be numeric for Distance/Rules.

- [ ] **Step 1: Write the failing test**

Add to `tools/test_board_sync.py`:

```python
def _board_from_desired(desired):
    """Helper: synthesize a 'current' board snapshot that already matches desired
    (used to prove idempotency: reconcile against it must yield no actions)."""
    current = []
    for i, (func, want) in enumerate(desired.items()):
        fields = dict(want["fields"])
        # numbers come back from the API as floats:
        for k in ("Distance", "Rules"):
            if k in fields:
                fields[k] = float(fields[k])
        current.append({"item_id": f"IID_{i}", "title": func,
                        "is_archived": want["archived"], "fields": fields})
    return current

def test_reconcile_add_new():
    desired = {"func_a": {"fields": {"Status": "Backlog", "Distance": 9}, "archived": False}}
    actions = board_sync.reconcile(desired, [])
    eq("one add action", len(actions), 1)
    eq("op is add", actions[0]["op"], "add")
    eq("add carries func", actions[0]["func"], "func_a")

def test_reconcile_idempotent():
    desired = {
        "func_a": {"fields": {"Status": "Backlog", "Verdict": "C", "WIP": "no",
                              "File": "x", "Distance": 9, "Rules": 3}, "archived": False},
    }
    current = _board_from_desired(desired)
    actions = board_sync.reconcile(desired, current)
    eq("no actions when in sync", actions, [])

def test_reconcile_update_changed_field():
    desired = {"func_a": {"fields": {"Status": "Blocked", "Distance": 9}, "archived": False}}
    current = [{"item_id": "IID_0", "title": "func_a", "is_archived": False,
               "fields": {"Status": "Backlog", "Distance": 9.0}}]
    actions = board_sync.reconcile(desired, current)
    eq("one set action", len(actions), 1)
    eq("op is set", actions[0]["op"], "set")
    eq("sets Status", actions[0]["field"], "Status")
    eq("to Blocked", actions[0]["value"], "Blocked")

def test_reconcile_completed_off_queue_archives():
    # func_old is on the board (Backlog) but no longer in the queue -> completed.
    desired = {}
    current = [{"item_id": "IID_0", "title": "func_old", "is_archived": False,
               "fields": {"Status": "Backlog"}}]
    actions = board_sync.reconcile(desired, current)
    ops = {(a["op"], a.get("field"), a.get("value")) for a in actions}
    check("sets Status=Done", ("set", "Status", "Done") in ops, True)
    check("archives it", any(a["op"] == "archive" for a in actions), True)

def test_reconcile_never_deletes():
    desired = {"func_a": {"fields": {"Status": "Backlog"}, "archived": False}}
    current = [{"item_id": "IID_0", "title": "func_stale", "is_archived": False,
               "fields": {"Status": "In-Progress"}}]  # In-Progress = not auto-touched in P1
    actions = board_sync.reconcile(desired, current)
    check("never emits delete", all(a["op"] != "delete" for a in actions), True)
```

Register in `main()`:

```python
    test_reconcile_add_new()
    test_reconcile_idempotent()
    test_reconcile_update_changed_field()
    test_reconcile_completed_off_queue_archives()
    test_reconcile_never_deletes()
```

- [ ] **Step 2: Run test to verify it fails**

Run: `python tools/test_board_sync.py`
Expected: FAIL — `AttributeError: module 'board_sync' has no attribute 'reconcile'`.

- [ ] **Step 3: Write minimal implementation**

Add a new section to `tools/board_sync.py`:

```python
# ---------------------------------------------------------------------------
# reconciler (pure) — diff desired vs current; emit minimal actions; never delete
# ---------------------------------------------------------------------------

def _val_eq(field, want, got):
    """Field-aware equality. Numbers compare numerically (API returns floats)."""
    if field in ("Distance", "Rules"):
        if got is None:
            return False
        return float(want) == float(got)
    return want == got


def reconcile(desired, current):
    """Diff desired (dict func->{fields, archived}) against the current board
    snapshot (list of {item_id, title, is_archived, fields}).

    Returns a list of action dicts. Action ops: 'add', 'set', 'archive',
    'unarchive'. NEVER 'delete' — a board item with no desired entry is treated
    as a completed function (Done + archive), only when it is currently in an
    active column. Items already in In-Progress / In-Review (Phase 2 columns)
    are left untouched.
    """
    actions = []
    cur_by_title = {c["title"]: c for c in current}

    for func, want in desired.items():
        cur = cur_by_title.get(func)
        if cur is None:
            actions.append({"op": "add", "func": func,
                            "fields": want["fields"], "archived": want["archived"]})
            continue
        for fname, dval in want["fields"].items():
            if not _val_eq(fname, dval, cur["fields"].get(fname)):
                actions.append({"op": "set", "item_id": cur["item_id"],
                                "field": fname, "value": dval, "func": func})
        if want["archived"] and not cur["is_archived"]:
            actions.append({"op": "archive", "item_id": cur["item_id"], "func": func})
        elif not want["archived"] and cur["is_archived"]:
            actions.append({"op": "unarchive", "item_id": cur["item_id"], "func": func})

    # On the board but no longer in desired -> completed. Done + archive, but only
    # if it's currently in an active column (don't disturb Phase 2 work-in-flight).
    for func, cur in cur_by_title.items():
        if func in desired:
            continue
        if cur["fields"].get("Status") in ACTIVE_COLUMNS:
            if cur["fields"].get("Status") != "Done":
                actions.append({"op": "set", "item_id": cur["item_id"],
                                "field": "Status", "value": "Done", "func": func})
            if not cur["is_archived"]:
                actions.append({"op": "archive", "item_id": cur["item_id"], "func": func})
    return actions
```

- [ ] **Step 4: Run test to verify it passes**

Run: `python tools/test_board_sync.py`
Expected: PASS — `9 passed, 0 failed`.

- [ ] **Step 5: Commit**

```bash
git add tools/board_sync.py tools/test_board_sync.py
git commit -m "board: pure reconciler (add/update/archive, never delete, idempotent)"
```

---

## Task 4: GitHub client chokepoint (`gh_graphql`) + `ensure_project`

**Files:**
- Modify: `tools/board_sync.py` (new "gh client" section)
- Test: `tools/test_board_sync.py`

All network goes through one function so tests can stub it.

- [ ] **Step 1: Write the failing test**

Add to `tools/test_board_sync.py` a fake-`gh` recorder and tests:

```python
class FakeGh:
    """Records (query, variables) calls and returns scripted responses."""
    def __init__(self, responses):
        self.responses = list(responses)
        self.calls = []
    def __call__(self, query, variables=None):
        self.calls.append({"query": query, "variables": variables or {}})
        return self.responses.pop(0)

def _with_stub(fake, fn):
    saved = board_sync.gh_graphql
    try:
        board_sync.gh_graphql = fake
        return fn()
    finally:
        board_sync.gh_graphql = saved

def test_ensure_project_reuses_existing():
    fake = FakeGh([
        {"user": {"projectsV2": {"nodes": [{"id": "PVT_existing", "number": 3, "title": "BB2 Decomp"}],
                                 "pageInfo": {"hasNextPage": False, "endCursor": None}}}},
    ])
    pid = _with_stub(fake, lambda: board_sync.ensure_project("BB2 Decomp", "tgg12321"))
    eq("reuses existing project id", pid, "PVT_existing")
    eq("only one call (no create)", len(fake.calls), 1)

def test_ensure_project_creates_when_absent():
    fake = FakeGh([
        {"user": {"projectsV2": {"nodes": [], "pageInfo": {"hasNextPage": False, "endCursor": None}}}},
        {"viewer": {"id": "U_me"}},
        {"createProjectV2": {"projectV2": {"id": "PVT_new", "number": 9, "title": "BB2 Decomp"}}},
    ])
    pid = _with_stub(fake, lambda: board_sync.ensure_project("BB2 Decomp", "tgg12321"))
    eq("creates and returns new id", pid, "PVT_new")
    eq("three calls (list, viewer, create)", len(fake.calls), 3)
```

Register in `main()`:

```python
    test_ensure_project_reuses_existing()
    test_ensure_project_creates_when_absent()
```

- [ ] **Step 2: Run test to verify it fails**

Run: `python tools/test_board_sync.py`
Expected: FAIL — `AttributeError: module 'board_sync' has no attribute 'gh_graphql'` (or `ensure_project`).

- [ ] **Step 3: Write minimal implementation**

Add a new section to `tools/board_sync.py`:

```python
# ---------------------------------------------------------------------------
# gh client — all network goes through gh_graphql() (the mockable chokepoint)
# ---------------------------------------------------------------------------

class GhError(Exception):
    pass


def gh_graphql(query, variables=None):
    """Run a GraphQL query/mutation via `gh api graphql` and return the 'data'
    object. Variables (strings, numbers, lists, objects) are sent as a JSON
    request body on stdin via `--input -` (gh's -f/-F flags do NOT parse JSON
    arrays into GraphQL lists). sys.exit on missing gh / auth failure (data is
    intact); raises GhError on GraphQL or other gh errors."""
    body = json.dumps({"query": query, "variables": variables or {}})
    argv = ["gh", "api", "graphql", "-H", "X-Github-Next-Global-ID: 1", "--input", "-"]
    try:
        r = subprocess.run(argv, input=body, capture_output=True, text=True)
    except FileNotFoundError:
        sys.exit("FATAL: `gh` CLI not found on PATH. Install GitHub CLI and run `gh auth login`. "
                 "Engine state is untouched; rerun board_sync later.")
    if r.returncode != 0:
        raise GhError(r.stderr.strip() or f"gh exited {r.returncode}")
    out = json.loads(r.stdout)
    if out.get("errors"):
        raise GhError(json.dumps(out["errors"]))
    return out["data"]


def _viewer_id():
    return gh_graphql("query{ viewer { id } }")["viewer"]["id"]


def ensure_project(title, login):
    """Return the node id of the user's project named `title`, creating it if absent."""
    cursor = None
    while True:
        data = gh_graphql(
            "query($login:String!,$cursor:String){ user(login:$login){ "
            "projectsV2(first:100,after:$cursor){ nodes{ id number title } "
            "pageInfo{ hasNextPage endCursor } } } }",
            variables={"login": login, "cursor": cursor},
        )
        pv = data["user"]["projectsV2"]
        for n in pv["nodes"]:
            if n["title"] == title:
                return n["id"]
        if not pv["pageInfo"]["hasNextPage"]:
            break
        cursor = pv["pageInfo"]["endCursor"]
    owner = _viewer_id()
    created = gh_graphql(
        "mutation($ownerId:ID!,$title:String!){ createProjectV2(input:{ownerId:$ownerId,title:$title}){ "
        "projectV2{ id number title } } }",
        variables={"ownerId": owner, "title": title},
    )
    return created["createProjectV2"]["projectV2"]["id"]
```

NOTE: passing `cursor=None` for the first page serializes to JSON `null` → `after:null` (= from the start), so no dict-merge is needed.

- [ ] **Step 4: Run test to verify it passes**

Run: `python tools/test_board_sync.py`
Expected: PASS — `11 passed, 0 failed`.

- [ ] **Step 5: Commit**

```bash
git add tools/board_sync.py tools/test_board_sync.py
git commit -m "board: gh_graphql chokepoint + idempotent ensure_project"
```

---

## Task 5: `ensure_fields` — create/read custom fields + option maps

**Files:**
- Modify: `tools/board_sync.py`
- Test: `tools/test_board_sync.py`

Returns a `field_map`: `{name: {"id": str, "type": str, "options": {label: option_id}}}`.

- [ ] **Step 1: Write the failing test**

```python
def test_ensure_fields_creates_missing():
    # First call: list existing fields -> only built-in Title (no custom fields).
    # Then one createProjectV2Field response per FIELD_SPEC, in order.
    list_resp = {"node": {"fields": {"nodes": [
        {"id": "F_title", "name": "Title"},
    ], "pageInfo": {"hasNextPage": False, "endCursor": None}}}}
    create_resps = []
    for name, dtype, opts in board_sync.FIELD_SPECS:
        if dtype == "SINGLE_SELECT":
            create_resps.append({"createProjectV2Field": {"projectV2Field": {
                "id": f"F_{name}", "name": name,
                "options": [{"id": f"opt_{name}_{o}", "name": o} for o in opts]}}})
        else:
            create_resps.append({"createProjectV2Field": {"projectV2Field": {
                "id": f"F_{name}", "name": name}}})
    fake = FakeGh([list_resp] + create_resps)
    fmap = _with_stub(fake, lambda: board_sync.ensure_fields("PVT_x"))
    eq("all specced fields present", sorted(fmap), sorted(n for n, _, _ in board_sync.FIELD_SPECS))
    eq("status is single-select with options", fmap["Status"]["options"]["Backlog"], "opt_Status_Backlog")
    eq("number field has no options", fmap["Distance"]["options"], {})

def test_ensure_fields_reuses_existing():
    nodes = [{"id": "F_title", "name": "Title"}]
    for name, dtype, opts in board_sync.FIELD_SPECS:
        if dtype == "SINGLE_SELECT":
            nodes.append({"id": f"F_{name}", "name": name,
                          "options": [{"id": f"opt_{name}_{o}", "name": o} for o in opts]})
        else:
            nodes.append({"id": f"F_{name}", "name": name})
    fake = FakeGh([{"node": {"fields": {"nodes": nodes,
                   "pageInfo": {"hasNextPage": False, "endCursor": None}}}}])
    fmap = _with_stub(fake, lambda: board_sync.ensure_fields("PVT_x"))
    eq("no create calls when all present", len(fake.calls), 1)
    eq("reused status option id", fmap["Status"]["options"]["Done"], "opt_Status_Done")
```

Register in `main()`:

```python
    test_ensure_fields_creates_missing()
    test_ensure_fields_reuses_existing()
```

- [ ] **Step 2: Run test to verify it fails**

Run: `python tools/test_board_sync.py`
Expected: FAIL — `AttributeError: ... 'ensure_fields'`.

- [ ] **Step 3: Write minimal implementation**

Append to the "gh client" section:

```python
def _list_fields(project_id):
    """Return {name: {"id", "type", "options"}} for existing project fields."""
    out = {}
    cursor = None
    while True:
        data = gh_graphql(
            "query($id:ID!,$cursor:String){ node(id:$id){ ... on ProjectV2{ "
            "fields(first:50,after:$cursor){ nodes{ "
            "__typename "
            "... on ProjectV2FieldCommon{ id name } "
            "... on ProjectV2SingleSelectField{ id name options{ id name } } } "
            "pageInfo{ hasNextPage endCursor } } } } }",
            variables={"id": project_id, "cursor": cursor},
        )
        f = data["node"]["fields"]
        for n in f["nodes"]:
            opts = {o["name"]: o["id"] for o in n.get("options", [])}
            out[n["name"]] = {"id": n["id"], "options": opts}
        if not f["pageInfo"]["hasNextPage"]:
            break
        cursor = f["pageInfo"]["endCursor"]
    return out


def _create_field(project_id, name, dtype, options):
    if dtype == "SINGLE_SELECT":
        data = gh_graphql(
            "mutation($p:ID!,$name:String!,$opts:[ProjectV2SingleSelectFieldOptionInput!]!){ "
            "createProjectV2Field(input:{projectId:$p,dataType:SINGLE_SELECT,name:$name,singleSelectOptions:$opts}){ "
            "projectV2Field{ ... on ProjectV2SingleSelectField{ id name options{ id name } } } } }",
            variables={"p": project_id, "name": name,
                       "opts": [{"name": o, "color": OPTION_COLORS.get(o, "GRAY"), "description": ""} for o in options]},
        )
        field = data["createProjectV2Field"]["projectV2Field"]
        return {"id": field["id"], "options": {o["name"]: o["id"] for o in field["options"]}}
    data = gh_graphql(
        "mutation($p:ID!,$name:String!,$dt:ProjectV2CustomFieldType!){ "
        "createProjectV2Field(input:{projectId:$p,dataType:$dt,name:$name}){ "
        "projectV2Field{ ... on ProjectV2FieldCommon{ id name } } } }",
        variables={"p": project_id, "name": name, "dt": dtype},
    )
    field = data["createProjectV2Field"]["projectV2Field"]
    return {"id": field["id"], "options": {}}


def ensure_fields(project_id):
    """Ensure every FIELD_SPEC field exists; return {name: {id, options}}."""
    existing = _list_fields(project_id)
    field_map = {}
    for name, dtype, options in FIELD_SPECS:
        if name in existing:
            field_map[name] = existing[name]
        else:
            field_map[name] = _create_field(project_id, name, dtype, options)
    return field_map
```

- [ ] **Step 4: Run test to verify it passes**

Run: `python tools/test_board_sync.py`
Expected: PASS — `13 passed, 0 failed`.

- [ ] **Step 5: Commit**

```bash
git add tools/board_sync.py tools/test_board_sync.py
git commit -m "board: ensure_fields (single-select/number/text + option maps)"
```

---

## Task 6: `list_items` — paginated board read + parse

**Files:**
- Modify: `tools/board_sync.py`
- Test: `tools/test_board_sync.py`

- [ ] **Step 1: Write the failing test**

```python
def _page(nodes, has_next, cursor):
    return {"node": {"items": {"nodes": nodes,
            "pageInfo": {"hasNextPage": has_next, "endCursor": cursor}}}}

def _item_node(iid, func, archived, status):
    return {"id": iid, "isArchived": archived,
            "content": {"title": func},
            "fieldValues": {"nodes": [
                {"__typename": "ProjectV2ItemFieldSingleSelectValue", "name": status,
                 "optionId": "x", "field": {"name": "Status"}},
            ]}}

def test_list_items_paginates_and_parses():
    fake = FakeGh([
        _page([_item_node("IID_0", "func_a", False, "Backlog")], True, "CUR1"),
        _page([_item_node("IID_1", "func_b", True, "Done")], False, None),
    ])
    items = _with_stub(fake, lambda: board_sync.list_items("PVT_x"))
    eq("two items across pages", len(items), 2)
    eq("second page cursor passed", fake.calls[1]["variables"].get("cursor"), "CUR1")
    eq("title parsed", items[0]["title"], "func_a")
    eq("archived parsed", items[1]["is_archived"], True)
    eq("single-select value parsed", items[0]["fields"]["Status"], "Backlog")
```

Register in `main()`:

```python
    test_list_items_paginates_and_parses()
```

- [ ] **Step 2: Run test to verify it fails**

Run: `python tools/test_board_sync.py`
Expected: FAIL — `AttributeError: ... 'list_items'`.

- [ ] **Step 3: Write minimal implementation**

Append to the "gh client" section:

```python
_ITEMS_QUERY = (
    "query($id:ID!,$cursor:String){ node(id:$id){ ... on ProjectV2{ "
    "items(first:100,after:$cursor){ "
    "pageInfo{ hasNextPage endCursor } "
    "nodes{ id isArchived "
    "content{ ... on DraftIssue{ title } ... on Issue{ title } ... on PullRequest{ title } } "
    "fieldValues(first:20){ nodes{ "
    "__typename "
    "... on ProjectV2ItemFieldTextValue{ text field{ ... on ProjectV2FieldCommon{ name } } } "
    "... on ProjectV2ItemFieldNumberValue{ number field{ ... on ProjectV2FieldCommon{ name } } } "
    "... on ProjectV2ItemFieldSingleSelectValue{ name optionId field{ ... on ProjectV2FieldCommon{ name } } } "
    "} } } } } } }"
)


def _parse_field_values(fv_nodes):
    fields = {}
    for n in fv_nodes:
        fld = n.get("field") or {}
        name = fld.get("name")
        if not name:
            continue
        t = n.get("__typename")
        if t == "ProjectV2ItemFieldTextValue":
            fields[name] = n.get("text")
        elif t == "ProjectV2ItemFieldNumberValue":
            fields[name] = n.get("number")
        elif t == "ProjectV2ItemFieldSingleSelectValue":
            fields[name] = n.get("name")
    return fields


def list_items(project_id):
    """Return the full board snapshot (including archived items):
    [{item_id, title, is_archived, fields}]."""
    items = []
    cursor = None
    while True:
        data = gh_graphql(_ITEMS_QUERY, variables={"id": project_id, "cursor": cursor})
        page = data["node"]["items"]
        for n in page["nodes"]:
            content = n.get("content") or {}
            items.append({
                "item_id": n["id"],
                "title": content.get("title"),
                "is_archived": n.get("isArchived", False),
                "fields": _parse_field_values(n.get("fieldValues", {}).get("nodes", [])),
            })
        if not page["pageInfo"]["hasNextPage"]:
            break
        cursor = page["pageInfo"]["endCursor"]
    return items
```

- [ ] **Step 4: Run test to verify it passes**

Run: `python tools/test_board_sync.py`
Expected: PASS — `14 passed, 0 failed`.

- [ ] **Step 5: Commit**

```bash
git add tools/board_sync.py tools/test_board_sync.py
git commit -m "board: list_items (paginated read + field-value parse)"
```

---

## Task 7: `apply` — execute the action list with pacing + backoff

**Files:**
- Modify: `tools/board_sync.py`
- Test: `tools/test_board_sync.py`

`apply` turns reconciler actions into `gh_graphql` mutations using `field_map` for field/option ids. It paces mutations and backs off on rate-limit errors. **It must never issue a delete.**

- [ ] **Step 1: Write the failing test**

```python
def _full_field_map():
    fmap = {}
    for name, dtype, opts in board_sync.FIELD_SPECS:
        fmap[name] = {"id": f"F_{name}",
                      "options": {o: f"opt_{name}_{o}" for o in (opts or [])}}
    return fmap

def test_apply_add_then_set_fields():
    # add returns a new item id; then one set per field; field order = insertion order.
    add_resp = {"addProjectV2DraftIssue": {"projectItem": {"id": "PVTI_new"}}}
    set_resp = {"updateProjectV2ItemFieldValue": {"projectV2Item": {"id": "PVTI_new"}}}
    fields = {"Status": "Backlog", "Distance": 9}
    actions = [{"op": "add", "func": "func_a", "fields": fields, "archived": False}]
    fake = FakeGh([add_resp, set_resp, set_resp])
    _with_stub(fake, lambda: board_sync.apply("PVT_x", _full_field_map(), actions, delay=0))
    eq("add + 2 field sets = 3 calls", len(fake.calls), 3)
    check("first call is the draft add", "addProjectV2DraftIssue" in fake.calls[0]["query"], True)

def test_apply_single_select_uses_option_id():
    set_resp = {"updateProjectV2ItemFieldValue": {"projectV2Item": {"id": "IID"}}}
    actions = [{"op": "set", "item_id": "IID", "field": "Status", "value": "Done", "func": "f"}]
    fake = FakeGh([set_resp])
    _with_stub(fake, lambda: board_sync.apply("PVT_x", _full_field_map(), actions, delay=0))
    eq("single-select set sends option id", fake.calls[0]["variables"].get("opt"), "opt_Status_Done")

def test_apply_number_passes_number():
    set_resp = {"updateProjectV2ItemFieldValue": {"projectV2Item": {"id": "IID"}}}
    actions = [{"op": "set", "item_id": "IID", "field": "Distance", "value": 9, "func": "f"}]
    fake = FakeGh([set_resp])
    _with_stub(fake, lambda: board_sync.apply("PVT_x", _full_field_map(), actions, delay=0))
    eq("number passed directly in variables", fake.calls[0]["variables"].get("v"), 9)

def test_apply_archive():
    arch_resp = {"archiveProjectV2Item": {"item": {"id": "IID", "isArchived": True}}}
    actions = [{"op": "archive", "item_id": "IID", "func": "f"}]
    fake = FakeGh([arch_resp])
    _with_stub(fake, lambda: board_sync.apply("PVT_x", _full_field_map(), actions, delay=0))
    check("archive mutation issued", "archiveProjectV2Item" in fake.calls[0]["query"], True)
```

Register in `main()`:

```python
    test_apply_add_then_set_fields()
    test_apply_single_select_uses_option_id()
    test_apply_number_passes_number()
    test_apply_archive()
```

- [ ] **Step 2: Run test to verify it fails**

Run: `python tools/test_board_sync.py`
Expected: FAIL — `AttributeError: ... 'apply'`.

- [ ] **Step 3: Write minimal implementation**

Append to the "gh client" section:

```python
_ADD_DRAFT = ("mutation($p:ID!,$t:String!,$b:String!){ "
              "addProjectV2DraftIssue(input:{projectId:$p,title:$t,body:$b}){ projectItem{ id } } }")
_SET_TEXT = ("mutation($p:ID!,$i:ID!,$f:ID!,$v:String!){ "
             "updateProjectV2ItemFieldValue(input:{projectId:$p,itemId:$i,fieldId:$f,value:{text:$v}}){ projectV2Item{ id } } }")
_SET_NUM = ("mutation($p:ID!,$i:ID!,$f:ID!,$v:Float!){ "
            "updateProjectV2ItemFieldValue(input:{projectId:$p,itemId:$i,fieldId:$f,value:{number:$v}}){ projectV2Item{ id } } }")
_SET_SEL = ("mutation($p:ID!,$i:ID!,$f:ID!,$opt:String!){ "
            "updateProjectV2ItemFieldValue(input:{projectId:$p,itemId:$i,fieldId:$f,value:{singleSelectOptionId:$opt}}){ projectV2Item{ id } } }")
_ARCHIVE = "mutation($p:ID!,$i:ID!){ archiveProjectV2Item(input:{projectId:$p,itemId:$i}){ item{ id isArchived } } }"
_UNARCHIVE = "mutation($p:ID!,$i:ID!){ unarchiveProjectV2Item(input:{projectId:$p,itemId:$i}){ item{ id isArchived } } }"

_NUMBER_FIELDS = {"Distance", "Rules"}


def _mutate(query, variables=None, delay=0.2, max_tries=5):
    """gh_graphql with rate-limit backoff + inter-mutation pacing."""
    for attempt in range(max_tries):
        try:
            data = gh_graphql(query, variables=variables)
            if delay:
                time.sleep(delay)
            return data
        except GhError as e:
            msg = str(e).lower()
            if "rate limit" in msg or "secondary" in msg or "was submitted too quickly" in msg:
                back = 2 ** attempt
                print(f"  rate-limited; backing off {back}s")
                time.sleep(back)
                continue
            raise
    raise GhError("rate-limit backoff exhausted")


def _set_field(project_id, field_map, item_id, field, value, delay):
    spec = field_map[field]
    if field in _NUMBER_FIELDS:
        _mutate(_SET_NUM, variables={"p": project_id, "i": item_id, "f": spec["id"], "v": value},
                delay=delay)
    elif spec["options"]:  # single-select
        opt_id = spec["options"][value]
        _mutate(_SET_SEL, variables={"p": project_id, "i": item_id, "f": spec["id"], "opt": opt_id},
                delay=delay)
    else:  # text
        _mutate(_SET_TEXT, variables={"p": project_id, "i": item_id, "f": spec["id"], "v": str(value)},
                delay=delay)


def apply(project_id, field_map, actions, delay=0.2):
    """Execute reconciler actions as gh mutations. Never deletes."""
    for a in actions:
        op = a["op"]
        if op == "add":
            body = f"file: {a['fields'].get('File', '')}"
            res = _mutate(_ADD_DRAFT, variables={"p": project_id, "t": a["func"], "b": body}, delay=delay)
            item_id = res["addProjectV2DraftIssue"]["projectItem"]["id"]
            for field, value in a["fields"].items():
                _set_field(project_id, field_map, item_id, field, value, delay)
            if a.get("archived"):
                _mutate(_ARCHIVE, variables={"p": project_id, "i": item_id}, delay=delay)
        elif op == "set":
            _set_field(project_id, field_map, a["item_id"], a["field"], a["value"], delay)
        elif op == "archive":
            _mutate(_ARCHIVE, variables={"p": project_id, "i": a["item_id"]}, delay=delay)
        elif op == "unarchive":
            _mutate(_UNARCHIVE, variables={"p": project_id, "i": a["item_id"]}, delay=delay)
        else:
            raise ValueError(f"unknown action op: {op!r}")
```

- [ ] **Step 4: Run test to verify it passes**

Run: `python tools/test_board_sync.py`
Expected: PASS — `18 passed, 0 failed`.

- [ ] **Step 5: Commit**

```bash
git add tools/board_sync.py tools/test_board_sync.py
git commit -m "board: apply() — execute actions with pacing + rate-limit backoff"
```

---

## Task 8: `main()` + CLI wiring (`--dry-run`, `--project`, `--login`)

**Files:**
- Modify: `tools/board_sync.py`
- Test: `tools/test_board_sync.py`

- [ ] **Step 1: Write the failing test**

Test the dry-run path end-to-end by stubbing the client functions so no network is touched and asserting `apply` is NOT called.

```python
def test_run_sync_dry_run_makes_no_mutations():
    # Stub the client layer; only reconcile runs for real.
    saved = (board_sync.ensure_project, board_sync.ensure_fields,
             board_sync.list_items, board_sync.apply)
    applied = {"called": False}
    try:
        board_sync.ensure_project = lambda title, login: "PVT_x"
        board_sync.ensure_fields = lambda pid: _full_field_map()
        board_sync.list_items = lambda pid: []   # empty board -> adds desired
        board_sync.apply = lambda *a, **k: applied.__setitem__("called", True)
        with tempfile.TemporaryDirectory() as td:
            q = Path(td) / "queue.json"
            q.write_text(json.dumps({"items": [
                {"func": "func_a", "file": "x", "distance": 9, "verdict": "C", "rules": 3, "status": "active"}]}),
                encoding="utf-8")
            buf = io.StringIO()
            with contextlib.redirect_stdout(buf):
                n = board_sync.run_sync(queue_path=q, wip_dir=Path(td), project_title="BB2 Decomp",
                                        login="tgg12321", dry_run=True, seed_done=False, map_path=None)
            out = buf.getvalue()
        eq("reports 1 planned action", n, 1)
        eq("dry-run never applies", applied["called"], False)
        check("prints the add", "func_a" in out, True)
    finally:
        (board_sync.ensure_project, board_sync.ensure_fields,
         board_sync.list_items, board_sync.apply) = saved
```

Register in `main()`:

```python
    test_run_sync_dry_run_makes_no_mutations()
```

- [ ] **Step 2: Run test to verify it fails**

Run: `python tools/test_board_sync.py`
Expected: FAIL — `AttributeError: ... 'run_sync'`.

- [ ] **Step 3: Write minimal implementation**

Append a "driver / main" section to `tools/board_sync.py`:

```python
# ---------------------------------------------------------------------------
# driver / main
# ---------------------------------------------------------------------------

def run_sync(queue_path, wip_dir, project_title, login,
             dry_run=False, seed_done=False, map_path=None):
    """Build desired state, reconcile against the live board, and apply (unless
    dry_run). Returns the number of planned actions. Engine state is never written."""
    items = load_queue(queue_path)
    desired = build_desired_from_queue(items, wip_dir)
    if seed_done:
        desired.update(build_desired_done(load_inventory(map_path), {it["func"] for it in items}))
    print(f"board sync -> {project_title} ({'DRY RUN' if dry_run else 'apply'})")
    print(f"  desired cards: {len(desired)} (seed_done={seed_done})")

    project_id = ensure_project(project_title, login)
    field_map = ensure_fields(project_id)
    current = list_items(project_id)
    actions = reconcile(desired, current)
    print(f"  current board items: {len(current)}; planned actions: {len(actions)}")
    for a in actions:
        if a["op"] == "add":
            print(f"    + add {a['func']} ({a['fields'].get('Status')})")
        elif a["op"] == "set":
            print(f"    ~ set {a['func']}.{a['field']} = {a['value']}")
        elif a["op"] in ("archive", "unarchive"):
            print(f"    {'#' if a['op']=='archive' else '^'} {a['op']} {a['func']}")
    if not dry_run and actions:
        apply(project_id, field_map, actions)
    print("board sync complete." if not dry_run else "dry run complete (no mutations).")
    return len(actions)


def main():
    ap = argparse.ArgumentParser(description="project the BB2 decomp engine state onto a GitHub board")
    ap.add_argument("--queue", default=str(DEFAULT_QUEUE))
    ap.add_argument("--wip-dir", default=str(DEFAULT_WIP_DIR))
    ap.add_argument("--map", default=str(DEFAULT_MAP), help="build/bb2.map for --seed-done")
    ap.add_argument("--project", default=PROJECT_TITLE)
    ap.add_argument("--login", default="tgg12321")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--seed-done", action="store_true",
                    help="backfill completed funcs as archived Done items (one-time)")
    a = ap.parse_args()
    run_sync(Path(a.queue), Path(a.wip_dir), a.project, a.login,
             dry_run=a.dry_run, seed_done=a.seed_done, map_path=Path(a.map))


if __name__ == "__main__":
    main()
```

NOTE: `build_desired_done` and `load_inventory` are added in Task 9. Since `run_sync` only calls them under `seed_done=True`, the Task 8 test (`seed_done=False`) passes before Task 9 exists. **Do not run with `--seed-done` until Task 9 is complete.**

- [ ] **Step 4: Run test to verify it passes**

Run: `python tools/test_board_sync.py`
Expected: PASS — `19 passed, 0 failed`.

- [ ] **Step 5: Commit**

```bash
git add tools/board_sync.py tools/test_board_sync.py
git commit -m "board: run_sync + CLI (--dry-run/--project/--login)"
```

---

## Task 9: `--seed-done` — completed-function inventory from `build/bb2.map`

**Files:**
- Modify: `tools/board_sync.py` (add to "state collector" section)
- Test: `tools/test_board_sync.py`

- [ ] **Step 1: Write the failing test**

```python
def test_load_inventory_parses_map():
    sample = (
        " .text          0x00000000        0x0 build/src/empty.o\n"
        " .text          0x80016400      0x100 build/src/text1b.o\n"
        "                0x800164ac                func_800164AC\n"
        "                0x800164f8                func_800164F8\n"
        " .text          0x80020000      0x080 build/src/display.o\n"
        "                0x80020010                func_80020010\n"
        "                0x80020040                func_80037F08_ret\n"  # excluded
    )
    with tempfile.TemporaryDirectory() as td:
        m = Path(td) / "bb2.map"
        m.write_text(sample, encoding="utf-8")
        inv = board_sync.load_inventory(m)
    eq("three real funcs", len(inv), 3)
    eq("func->stem text1b", inv["func_800164AC"], "text1b")
    eq("func->stem display", inv["func_80020010"], "display")
    check("excluded non-function dropped", "func_80037F08_ret" not in inv, True)

def test_build_desired_done():
    inv = {"func_done": "text1b", "func_active": "display"}
    queue_funcs = {"func_active"}
    done = board_sync.build_desired_done(inv, queue_funcs)
    eq("only the completed func", list(done), ["func_done"])
    eq("status Done", done["func_done"]["fields"]["Status"], "Done")
    eq("file carried", done["func_done"]["fields"]["File"], "text1b")
    eq("archived true", done["func_done"]["archived"], True)
```

Register in `main()`:

```python
    test_load_inventory_parses_map()
    test_build_desired_done()
```

- [ ] **Step 2: Run test to verify it fails**

Run: `python tools/test_board_sync.py`
Expected: FAIL — `AttributeError: ... 'load_inventory'`.

- [ ] **Step 3: Write minimal implementation**

Append to the "state collector" section:

```python
_MAP_OBJ_RE = re.compile(r"^ \.text\s+0x[0-9a-fA-F]+\s+0x[0-9a-fA-F]+\s+build/src/(\S+)\.o")
_MAP_SYM_RE = re.compile(r"^\s+0x[0-9a-fA-F]{8,}\s+(\S+)\s*$")


def load_inventory(map_path):
    """Parse build/bb2.map -> {func_name: src_stem} for every .text symbol under
    a build/src/<stem>.o object (excluding the known non-functions). Used only
    for --seed-done."""
    map_path = Path(map_path)
    inv = {}
    cur_stem = None
    for line in map_path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = _MAP_OBJ_RE.match(line)
        if m:
            cur_stem = m.group(1)
            continue
        s = _MAP_SYM_RE.match(line)
        if s and cur_stem is not None:
            name = s.group(1)
            if name not in _MAP_EXCLUDE:
                inv[name] = cur_stem
    return inv


def build_desired_done(inventory, queue_funcs):
    """completed = inventory - queue_funcs -> archived Done cards."""
    desired = {}
    for func, stem in inventory.items():
        if func in queue_funcs:
            continue
        desired[func] = {"fields": {"Status": "Done", "File": stem}, "archived": True}
    return desired
```

- [ ] **Step 4: Run test to verify it passes**

Run: `python tools/test_board_sync.py`
Expected: PASS — `21 passed, 0 failed`.

- [ ] **Step 5: Commit**

```bash
git add tools/board_sync.py tools/test_board_sync.py
git commit -m "board: --seed-done inventory (build/bb2.map parse) + Done cards"
```

---

## Task 10: Live smoke test (manual, gated)

**Files:** none (manual verification against the real GitHub project).

> This task requires the live `gh` auth and creates a real "BB2 Decomp" project. Do it once, carefully. If anything looks wrong, the board is non-destructive (we never delete), and you can delete the test project from the GitHub UI and re-run.

- [ ] **Step 1: Dry-run against the live account (read-only)**

Run: `python tools/board_sync.py --dry-run`
Expected: prints `desired cards: 448 ...`, `current board items: 0`, and `planned actions: 448` (one add per remaining func). No project is created (dry-run skips `apply`, but note `ensure_project`/`ensure_fields`/`list_items` DO run — on first dry-run they will create the project + fields because those are not behind the dry-run guard).

> **Implementer note:** if you want a *truly* read-only dry-run, that is a refinement — in this plan `--dry-run` guards only `apply()` (mutations to items). Project + field creation are idempotent setup. Confirm that's acceptable; if not, file a follow-up to gate setup behind `--dry-run` too.

- [ ] **Step 2: Real run — populate the active board**

Run: `python tools/board_sync.py`
Expected: `~448` adds, then `board sync complete.` Open `https://github.com/users/tgg12321/projects/<number>` and confirm Backlog/Needs-Decision/Blocked are populated, sorted by Distance (set a board view grouped by Status, sorted by Distance).

- [ ] **Step 3: Idempotency check (the key acceptance criterion)**

Run: `python tools/board_sync.py`
Expected: `planned actions: 0` and `board sync complete.` (A second run with no engine changes makes ZERO mutations.)

- [ ] **Step 4: Done-seed (one-time backfill)**

Run: `python tools/board_sync.py --seed-done`
Expected: `desired cards: ~1472`, `~1024` adds for completed funcs (each created then archived). Then run once more: `python tools/board_sync.py --seed-done` → `planned actions: 0` (archived items are returned by `list_items`, so they are not re-added). **If the second seed run re-adds the completed funcs, archived items are not being returned by the items query — STOP and fix `list_items` to include archived items before continuing (this is the one live-API assumption to verify).**

- [ ] **Step 5: Record the result**

No commit (no file changes). Note the project number and the verified item counts in the PR/commit message for Task 11.

---

## Task 11: Usage doc + final wrap

**Files:**
- Create: `docs/board.md`

- [ ] **Step 1: Write the doc**

Create `docs/board.md`:

```markdown
# BB2 Decomp board (GitHub Projects v2)

`tools/board_sync.py` projects the engine's worklist onto a GitHub Project named
**BB2 Decomp** (`https://github.com/users/tgg12321/projects/<number>`).

- The board is a **one-way mirror** of `engine/queue.json` (+ `memory/wip/`).
  It never writes back to engine state and never deletes items.
- Columns map 1:1 to engine status: `active`→Backlog, `authorize`→Needs-Decision,
  `parked`→Blocked, completed→archived Done.

## Usage (Windows-side; needs `gh` authed with the `project` scope)

    python tools/board_sync.py --dry-run     # preview the change set
    python tools/board_sync.py               # sync the ~448 active cards
    python tools/board_sync.py --seed-done   # one-time: backfill completed funcs as archived Done

Re-running with no engine changes makes zero mutations (idempotent). Run it after
`queue regen` / `queue done` to refresh the board.

## Tests

    python tools/test_board_sync.py          # exit 0 = pass (no network; gh is stubbed)

## Phases 2-5

This is Phase 1 (visibility). Claim/lease (git branches), worktree isolation,
PR reintegration, and the multi-agent entry protocol land in later phases — see
`docs/superpowers/specs/2026-06-13-decomp-kanban-board-design.md`.
```

- [ ] **Step 2: Run the full test suite one last time**

Run: `python tools/test_board_sync.py`
Expected: PASS — `21 passed, 0 failed`.

- [ ] **Step 3: Commit**

```bash
git add docs/board.md
git commit -m "board: usage doc for board_sync.py (Phase 1)"
```

---

## Self-review (completed during planning)

**Spec coverage:**
- §5.1 project — Task 4 (`ensure_project`).
- §5.2 status field + mapping — Tasks 5 (field), 2 (mapping).
- §5.3 card fields + sources — Tasks 2 (active fields), 9 (Done file).
- §5.4 collector / client / reconciler — Tasks 2 / 4-7 / 3. Inventory source resolved: pure-Python `build/bb2.map` parse (Task 9), no WSL/readelf dependency.
- §5.5 data flow — Task 8 (`run_sync`).
- §5.6 CLI (`--dry-run`, `--project`, `--no-archive-seed`) — Task 8. **Deviation:** the spec named `--no-archive-seed` (default-on seed); this plan inverts it to `--seed-done` (default-OFF seed) so the common run stays fast and the ~1024-item backfill is opt-in. Same capability, safer default. Flag for user confirmation.
- §5.7 error handling — Task 4 (`gh_graphql` fatal-on-missing/auth; `load_queue` non-fatal on missing) + Task 7 (backoff). Never-delete enforced in Task 3 + tested.
- §5.8 testing — every task is TDD; idempotency + never-delete have dedicated tests (Task 3); live idempotency is Task 10 step 3.
- §5.9 out of scope — no claims/worktrees/PRs/write-back appear. Confirmed.
- §8 acceptance criteria 1-6 — criteria 1,2,4,6 = Tasks 8/10; criterion 3 (archived Done) = Tasks 9/10 step 4; criterion 5 (fixtures test green) = the whole suite.

**Placeholder scan:** none — every step has real code/commands/expected output.

**Type/name consistency:** `desired[func] = {"fields", "archived"}`, `current` item = `{item_id, title, is_archived, fields}`, `field_map[name] = {id, options}`, action ops `add|set|archive|unarchive` — used consistently across Tasks 2-9. `gh_graphql(query, variables)` signature is stable across client funcs and the `FakeGh` stub (variables sent as a JSON `{"query","variables"}` body via `gh api graphql --input -`).

**Open items for the user (surfaced, not silently decided):**
1. `--seed-done` default-OFF (vs spec's default-on `--no-archive-seed`).
2. `--dry-run` gates only item mutations, not project/field setup (Task 10 step 1 note).
3. The one live-API assumption to verify in Task 10 step 4: `list_items` returns archived items (required for Done-seed idempotency).
