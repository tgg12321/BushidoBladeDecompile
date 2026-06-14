#!/usr/bin/env python3
"""tools/board_enrich.py — push rich card BODIES to the BB2 Decomp board, and
seed the completed-function Done cards WITH their bodies.

board_sync.py owns the *fields* projection (Status/Verdict/Distance/...). This
tool owns the *body* (the rich markdown each card shows) and the one-time
creation of the ~1024 completed Done cards. Input bodies come from
tmp/cards/<func>.md (written by board_cards.py — read at runtime; missing files
are warned and skipped). board_sync is imported and reused, never modified.

The full job (~1024 Done creates + ~1472 active/Done body updates) is a
MULTI-HOUR grind against GitHub's secondary content-write limits
(~80/min AND ~500/hr). This tool is built to survive that:

  * Gentle pacing — sleep ~8s between content-writes (addDraft / updateDraftBody)
    to stay under ~500/hr. Field sets (Status) / archive are cheaper (~0.3s).
  * Rate-limit-aware — before each content-write it checks the GraphQL budget;
    if `remaining` < a safety floor it SLEEPS until the reset timestamp (+slack),
    prints a clear message, then resumes. One run can grind through many windows.
  * Resumable + idempotent — each run reads the CURRENT board (every DraftIssue
    content id + body + title + isArchived + Status) and only writes when the
    current body != desired body. A crash mid-run loses nothing; re-running tops
    up / resumes.
  * Patient backoff — on a secondary-limit error it backs off >=60s (secondary
    limits need ~a minute to clear).

Usage:
    python tools/board_enrich.py               # update active bodies + create+seed Done cards
    python tools/board_enrich.py --dry-run     # compute what WOULD change; write nothing
    python tools/board_enrich.py --limit N      # stop after N content-writes
    python tools/board_enrich.py --only-active  # only update active-card bodies
    python tools/board_enrich.py --only-done    # only seed Done cards
"""
from __future__ import annotations

import argparse
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import board_sync  # noqa: E402

DEFAULT_QUEUE = ROOT / "engine" / "queue.json"
DEFAULT_MAP = ROOT / "build" / "bb2.map"
DEFAULT_ELF = ROOT / "build" / "bb2.elf"
DEFAULT_CARDS = ROOT / "tmp" / "cards"

# Pacing / safety knobs (seconds / counts).
CONTENT_WRITE_DELAY = 8.0     # between addDraft / updateDraftBody (≈450/hr)
FIELD_DELAY = 0.3            # Status set / archive are cheaper
RATE_FLOOR = 200            # GraphQL points remaining before we wait for reset
RATE_SLACK = 5              # extra seconds past reset before resuming
SECONDARY_BACKOFF = 60      # patient backoff on a secondary-limit error
MAX_BODY = 65000            # GitHub body hard cap is 65536; stay defensive

# ---------------------------------------------------------------------------
# GraphQL fragments
# ---------------------------------------------------------------------------

# Read every item's DraftIssue content id + title + body, plus isArchived and
# the Status single-select value.
_ITEMS_QUERY = (
    "query($id:ID!,$cursor:String){ node(id:$id){ ... on ProjectV2{ "
    "items(first:100,after:$cursor){ "
    "pageInfo{ hasNextPage endCursor } "
    "nodes{ id isArchived "
    "content{ __typename ... on DraftIssue{ id title body } } "
    "fieldValues(first:20){ nodes{ __typename "
    "... on ProjectV2ItemFieldSingleSelectValue{ name field{ ... on ProjectV2FieldCommon{ name } } } "
    "} } } } } } }"
)

_UPDATE_BODY = (
    "mutation($d:ID!,$b:String!){ updateProjectV2DraftIssue(input:{draftIssueId:$d, body:$b}){ "
    "draftIssue{ id } } }"
)

_ADD_DRAFT = (
    "mutation($p:ID!,$t:String!,$b:String!){ addProjectV2DraftIssue(input:{projectId:$p,title:$t,body:$b}){ "
    "projectItem{ id content{ ... on DraftIssue{ id } } } } }"
)


# ---------------------------------------------------------------------------
# board read (one paginated snapshot)
# ---------------------------------------------------------------------------

def list_draft_items(project_id):
    """Return the full board snapshot keyed by DraftIssue title (= func name):

        by_title[title] = {item_id, draft_id, body, is_archived, status}

    Only DraftIssue-backed items are included (Issues / PRs have no draftIssue id
    to update and are not our cards). Items with no title are skipped."""
    by_title = {}
    cursor = None
    while True:
        data = board_sync.gh_graphql(_ITEMS_QUERY, variables={"id": project_id, "cursor": cursor})
        page = data["node"]["items"]
        for n in page["nodes"]:
            content = n.get("content") or {}
            if content.get("__typename") != "DraftIssue":
                continue
            title = content.get("title")
            if not title:
                continue
            by_title[title] = {
                "item_id": n["id"],
                "draft_id": content.get("id"),
                "body": content.get("body") or "",
                "is_archived": n.get("isArchived", False),
                "status": _status_of(n.get("fieldValues", {}).get("nodes", [])),
            }
        if not page["pageInfo"]["hasNextPage"]:
            break
        cursor = page["pageInfo"]["endCursor"]
    return by_title


def _status_of(fv_nodes):
    for fn in fv_nodes:
        fld = fn.get("field") or {}
        if fld.get("name") == "Status" and fn.get("__typename") == "ProjectV2ItemFieldSingleSelectValue":
            return fn.get("name")
    return None


# ---------------------------------------------------------------------------
# desired bodies (pure)
# ---------------------------------------------------------------------------

def truncate_body(body, limit=MAX_BODY):
    """Defensively cap a body at `limit` chars with a visible note. board_cards
    already budgets <=65000, but a runaway body must never exceed GitHub's hard
    cap (65536)."""
    if len(body) <= limit:
        return body
    note = "\n\n... [truncated by board_enrich]\n"
    keep = max(0, limit - len(note))
    return body[:keep] + note


def read_card_body(cards_dir, func):
    """Read tmp/cards/<func>.md (utf-8), truncated defensively. None if absent."""
    p = Path(cards_dir) / f"{func}.md"
    if not p.exists():
        return None
    return truncate_body(p.read_text(encoding="utf-8"))


def build_desired_bodies(queue_funcs, inventory, cards_dir,
                         only_active=False, only_done=False):
    """Compute the desired {func: (body, is_done)} map.

    active funcs  = the queue funcs (board_sync.load_queue).
    completed     = inventory - queue funcs (the Done cards to seed).
    Bodies come from tmp/cards/<func>.md; funcs with no card file are dropped
    (the caller warns). `is_done` drives create-as-Done-archived vs body-update.
    """
    desired = {}
    if not only_done:
        for func in queue_funcs:
            body = read_card_body(cards_dir, func)
            if body is not None:
                desired[func] = (body, False)
    if not only_active:
        for func in inventory:
            if func in queue_funcs:
                continue
            body = read_card_body(cards_dir, func)
            if body is not None:
                desired[func] = (body, True)
    return desired


# ---------------------------------------------------------------------------
# reconciler (pure) — never deletes; only update-differing / create-absent
# ---------------------------------------------------------------------------

def reconcile_bodies(desired, by_title):
    """Diff desired {func:(body,is_done)} against the current board snapshot
    `by_title` (from list_draft_items). Return a list of action dicts:

        {"op":"update","func","draft_id","body","bytes"}        — body differs
        {"op":"create","func","body","is_done","bytes"}          — card absent
        (skip when body identical — idempotent)

    NEVER deletes; a board card with no desired entry is simply left alone.
    """
    actions = []
    for func, (body, is_done) in desired.items():
        cur = by_title.get(func)
        if cur is None:
            actions.append({"op": "create", "func": func, "body": body,
                            "is_done": is_done, "bytes": len(body)})
            continue
        if cur["body"] != body:
            actions.append({"op": "update", "func": func, "draft_id": cur["draft_id"],
                            "body": body, "bytes": len(body)})
        # identical body -> skip (idempotent)
    return actions


# ---------------------------------------------------------------------------
# rate-limit budget reader + wait
# ---------------------------------------------------------------------------

def read_rate_budget():
    """Return (remaining, reset_epoch) for the GraphQL budget via `gh api
    rate_limit`. On any failure return (None, None) — the caller treats unknown
    budget as 'proceed' (pacing + secondary backoff still protect us)."""
    try:
        r = subprocess.run(["gh", "api", "rate_limit"], capture_output=True, text=True)
    except FileNotFoundError:
        return (None, None)
    if r.returncode != 0:
        return (None, None)
    try:
        import json
        data = json.loads(r.stdout)
        gql = data.get("resources", {}).get("graphql", {})
        return (gql.get("remaining"), gql.get("reset"))
    except (ValueError, KeyError):
        return (None, None)


def maybe_wait_for_budget(floor=RATE_FLOOR, slack=RATE_SLACK,
                          read=read_rate_budget, sleep=time.sleep, now=time.time):
    """If the GraphQL budget is below `floor`, sleep until reset (+slack) and
    return True. Otherwise return False. `read`/`sleep`/`now` are injectable for
    tests. Unknown budget (None) -> no wait."""
    remaining, reset = read()
    if remaining is None or remaining >= floor:
        return False
    wait = max(0, (reset or now()) - now()) + slack
    mins = wait / 60.0
    print(f"  rate budget low ({remaining}); sleeping until reset (~{mins:.1f}m)")
    sleep(wait)
    return True


# ---------------------------------------------------------------------------
# content-write executors (with patient secondary-limit backoff)
# ---------------------------------------------------------------------------

def _content_write(query, variables, delay, max_tries=6):
    """Run a content-write mutation with PATIENT backoff on secondary-limit
    errors (>=SECONDARY_BACKOFF). Paces `delay` after a success. Other GhErrors
    propagate."""
    for attempt in range(max_tries):
        try:
            data = board_sync.gh_graphql(query, variables=variables)
            if delay:
                time.sleep(delay)
            return data
        except board_sync.GhError as e:
            msg = str(e).lower()
            if ("rate limit" in msg or "secondary" in msg
                    or "was submitted too quickly" in msg or "abuse" in msg):
                back = SECONDARY_BACKOFF * (attempt + 1)
                print(f"  secondary rate limit hit; backing off {back}s")
                time.sleep(back)
                continue
            raise
    raise board_sync.GhError("content-write secondary-limit backoff exhausted")


def update_body(draft_id, body, delay=CONTENT_WRITE_DELAY):
    return _content_write(_UPDATE_BODY, {"d": draft_id, "b": body}, delay)


def create_card(project_id, func, body, delay=CONTENT_WRITE_DELAY):
    """Create a draft card WITH its body in one content-write. Returns
    (item_id, draft_id)."""
    res = _content_write(_ADD_DRAFT, {"p": project_id, "t": func, "b": body}, delay)
    pi = res["addProjectV2DraftIssue"]["projectItem"]
    item_id = pi["id"]
    draft_id = (pi.get("content") or {}).get("id")
    return item_id, draft_id


# ---------------------------------------------------------------------------
# driver
# ---------------------------------------------------------------------------

def run_enrich(queue_path=DEFAULT_QUEUE, map_path=DEFAULT_MAP, elf_path=DEFAULT_ELF,
               cards_dir=DEFAULT_CARDS, project_title=board_sync.PROJECT_TITLE,
               login="tgg12321", dry_run=False, limit=None,
               only_active=False, only_done=False):
    """Read the board, compute the desired bodies, and reconcile (unless
    dry_run). Returns a result dict {updated, created, skipped, remaining}."""
    print(f"board enrich -> {project_title} ({'DRY RUN' if dry_run else 'apply'})")

    project_id = board_sync.find_project(project_title, login)
    if project_id is None:
        sys.exit(f"FATAL: project '{project_title}' does not exist. "
                 "Run board_sync.py first to create it + its fields.")

    field_map = None
    if not dry_run:
        field_map = board_sync.ensure_fields(project_id)

    items = board_sync.load_queue(queue_path)
    queue_funcs = {it["func"] for it in items}
    inventory = {}
    if not only_active:
        inventory = board_sync.load_inventory(map_path, elf_path)

    desired = build_desired_bodies(queue_funcs, inventory, cards_dir,
                                   only_active=only_active, only_done=only_done)

    # Warn about funcs we wanted to enrich but have no card file.
    wanted = set()
    if not only_done:
        wanted |= queue_funcs
    if not only_active:
        wanted |= {f for f in inventory if f not in queue_funcs}
    missing = sorted(f for f in wanted if f not in desired)
    if missing:
        print(f"  WARN: {len(missing)} func(s) have no tmp/cards/<func>.md (skipped); "
              f"e.g. {', '.join(missing[:5])}")

    by_title = list_draft_items(project_id)
    actions = reconcile_bodies(desired, by_title)
    n_update = sum(1 for a in actions if a["op"] == "update")
    n_create = sum(1 for a in actions if a["op"] == "create")
    skipped = len(desired) - len(actions)
    print(f"  desired cards: {len(desired)} (active+done, with card files); "
          f"on board: {len(by_title)}")
    print(f"  planned content-writes: {len(actions)} ({n_update} update, {n_create} create); "
          f"already-correct (skipped): {skipped}")

    if dry_run:
        for a in actions[:20]:
            print(f"    {a['op']} {a['func']} ({a['bytes']}b)")
        if len(actions) > 20:
            print(f"    ... and {len(actions) - 20} more")
        print("dry run complete (no writes).")
        return {"updated": 0, "created": 0, "skipped": skipped, "remaining": len(actions)}

    updated = created = 0
    total = len(actions)
    for i, a in enumerate(actions, start=1):
        if limit is not None and (updated + created) >= limit:
            print(f"  --limit {limit} reached; stopping (resumable on re-run).")
            break
        maybe_wait_for_budget()
        if a["op"] == "update":
            print(f"[{i}/{total}] update {a['func']} ({a['bytes']}b)")
            update_body(a["draft_id"], a["body"])
            updated += 1
        else:  # create (+ Status=Done + archive)
            print(f"[{i}/{total}] create {a['func']} ({a['bytes']}b)")
            item_id, _draft_id = create_card(project_id, a["func"], a["body"])
            board_sync._set_field(project_id, field_map, item_id, "Status", "Done", FIELD_DELAY)
            board_sync._mutate(board_sync._ARCHIVE,
                               variables={"p": project_id, "i": item_id}, delay=FIELD_DELAY)
            created += 1
        if (updated + created) % 25 == 0:
            rem, _ = read_rate_budget()
            print(f"  progress: {updated} updated, {created} created; budget remaining={rem}")

    remaining = total - (updated + created)
    print(f"board enrich complete: {updated} updated, {created} created, "
          f"{skipped} skipped, {remaining} remaining-to-do.")
    return {"updated": updated, "created": created, "skipped": skipped, "remaining": remaining}


def main():
    ap = argparse.ArgumentParser(description="push rich card bodies + seed Done cards on the BB2 board")
    ap.add_argument("--queue", default=str(DEFAULT_QUEUE))
    ap.add_argument("--map", default=str(DEFAULT_MAP))
    ap.add_argument("--elf", default=str(DEFAULT_ELF))
    ap.add_argument("--cards", default=str(DEFAULT_CARDS), help="dir of <func>.md bodies (board_cards output)")
    ap.add_argument("--project", default=board_sync.PROJECT_TITLE)
    ap.add_argument("--login", default="tgg12321")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--limit", type=int, default=None, help="stop after N content-writes")
    ap.add_argument("--only-active", action="store_true", help="only update active-card bodies")
    ap.add_argument("--only-done", action="store_true", help="only seed Done cards")
    a = ap.parse_args()
    if a.only_active and a.only_done:
        sys.exit("--only-active and --only-done are mutually exclusive")
    try:
        run_enrich(queue_path=Path(a.queue), map_path=Path(a.map), elf_path=Path(a.elf),
                   cards_dir=Path(a.cards), project_title=a.project, login=a.login,
                   dry_run=a.dry_run, limit=a.limit,
                   only_active=a.only_active, only_done=a.only_done)
    except board_sync.GhError as e:
        sys.exit(f"board enrich failed (GitHub API error): {e}")


if __name__ == "__main__":
    main()
