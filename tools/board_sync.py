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
