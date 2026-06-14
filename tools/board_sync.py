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


# ---------------------------------------------------------------------------
# reconciler (pure) — diff desired vs current; emit minimal actions; never delete
# ---------------------------------------------------------------------------

def _val_eq(field, want, got):
    """Field-aware equality. Numbers compare numerically (API returns floats)."""
    if field in ("Distance", "Rules"):
        if want is None:
            return got is None
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


# ---------------------------------------------------------------------------
# gh client — all network goes through gh_graphql() (the mockable chokepoint)
# ---------------------------------------------------------------------------

class GhError(Exception):
    pass


def gh_graphql(query, fvars=None, Fvars=None):
    """Run a GraphQL query/mutation via `gh api graphql` and return the 'data'
    object. String vars via -f, numeric/raw vars via -F. sys.exit on missing gh /
    auth failure (data is intact); raises GhError on GraphQL or other gh errors."""
    argv = ["gh", "api", "graphql", "-H", "X-Github-Next-Global-ID: 1",
            "-f", "query=" + query]
    for k, v in (fvars or {}).items():
        argv += ["-f", f"{k}={v}"]
    for k, v in (Fvars or {}).items():
        argv += ["-F", f"{k}={v}"]
    try:
        r = subprocess.run(argv, capture_output=True, text=True)
    except FileNotFoundError:
        sys.exit("FATAL: `gh` CLI not found on PATH. Install GitHub CLI and run `gh auth login`. "
                 "Engine state is untouched; rerun board_sync later.")
    if r.returncode != 0:
        err = r.stderr.strip()
        low = err.lower()
        if ("401" in err or "bad credentials" in low or "gh auth login" in low
                or "not logged in" in low or "authentication" in low):
            sys.exit("FATAL: gh is not authenticated (" + (err or "auth error") + ").\n"
                     "  Run `gh auth login` (needs the 'project' scope). "
                     "Engine state is untouched; rerun board_sync later.")
        raise GhError(err or f"gh exited {r.returncode}")
    out = json.loads(r.stdout)
    if out.get("errors"):
        raise GhError(json.dumps(out["errors"]))
    return out["data"]


def _viewer_id():
    return gh_graphql("query{ viewer { id } }")["viewer"]["id"]


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
            fvars={"id": project_id} | ({"cursor": cursor} if cursor else {}),
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
        opts_json = json.dumps([
            {"name": o, "color": OPTION_COLORS.get(o, "GRAY"), "description": ""} for o in options
        ])
        data = gh_graphql(
            "mutation($p:ID!,$name:String!,$opts:[ProjectV2SingleSelectFieldOptionInput!]!){ "
            "createProjectV2Field(input:{projectId:$p,dataType:SINGLE_SELECT,name:$name,singleSelectOptions:$opts}){ "
            "projectV2Field{ ... on ProjectV2SingleSelectField{ id name options{ id name } } } } }",
            fvars={"p": project_id, "name": name}, Fvars={"opts": opts_json},
        )
        field = data["createProjectV2Field"]["projectV2Field"]
        return {"id": field["id"], "options": {o["name"]: o["id"] for o in field["options"]}}
    data = gh_graphql(
        "mutation($p:ID!,$name:String!,$dt:ProjectV2CustomFieldType!){ "
        "createProjectV2Field(input:{projectId:$p,dataType:$dt,name:$name}){ "
        "projectV2Field{ ... on ProjectV2FieldCommon{ id name } } } }",
        fvars={"p": project_id, "name": name, "dt": dtype},
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


def ensure_project(title, login):
    """Return the node id of the user's project named `title`, creating it if absent."""
    cursor = None
    while True:
        data = gh_graphql(
            "query($login:String!,$cursor:String){ user(login:$login){ "
            "projectsV2(first:100,after:$cursor){ nodes{ id number title } "
            "pageInfo{ hasNextPage endCursor } } } }",
            fvars={"login": login} | ({"cursor": cursor} if cursor else {}),
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
        fvars={"ownerId": owner, "title": title},
    )
    return created["createProjectV2"]["projectV2"]["id"]
