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
import hashlib
import json
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
# Local resume state for the Done-card grind. GitHub's items(first:N) does NOT
# reliably return ARCHIVED items, so once a Done card is created+archived it
# becomes invisible to the board read. The ledger is therefore the ONLY reliable
# "this Done card exists" signal — without it every resume would RE-CREATE all
# ~1024 Done cards as duplicates. tmp/ is gitignored (correct: local state).
DEFAULT_LEDGER = ROOT / "tmp" / "board_enrich_done_ledger.json"

# Pacing / safety knobs (seconds / counts).
CONTENT_WRITE_DELAY = 8.0     # between addDraft / updateDraftBody (≈450/hr)
FIELD_DELAY = 0.3            # Status set / archive are cheaper
RATE_FLOOR = 200            # GraphQL points remaining before we wait for reset
RATE_SLACK = 5              # extra seconds past reset before resuming
SECONDARY_BACKOFF = 60      # patient backoff on a secondary-limit error
MAX_BODY = 65000            # GitHub body hard cap is 65536; stay defensive
BUDGET_POLL_EVERY = 25      # re-poll `gh api rate_limit` every N content-writes (I1)

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


# ---------------------------------------------------------------------------
# Local ledger — two jobs:
#  1. The authoritative "this archived Done card exists" signal (archived items
#     are invisible to items(first:N), so the board read can't see them).
#  2. Body idempotency for ALL cards (active + done): GitHub NORMALIZES a draft
#     body on storage (the remote read-back is longer than what we sent), so
#     "remote_body != local_body" is ALWAYS true and would re-update every card
#     on every run. Instead we record body_sha = sha256(the body we SENT) and
#     skip when it still matches the local desired body. This is API-independent.
#
# Entry shape: {func: {"draft_id", "item_id", "finalized": <bool>, "body_sha"}}.
# Active funcs get an entry too (carrying body_sha + ids; finalized stays False
# and they are never archived).
# ---------------------------------------------------------------------------

def body_sha(text):
    """sha256 hex of the EXACT body we send (local card content), so idempotency
    never depends on GitHub's normalized read-back."""
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def load_ledger(path):
    """Load the ledger {func: {"draft_id","item_id","finalized","body_sha"}}.
    Missing/corrupt file -> {}."""
    p = Path(path)
    if not p.exists():
        return {}
    try:
        data = json.loads(p.read_text(encoding="utf-8"))
    except (ValueError, OSError):
        return {}
    return data if isinstance(data, dict) else {}


def save_ledger(path, ledger):
    """Write-through the ledger (utf-8) after each change, for crash-safety."""
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    Path(path).write_text(json.dumps(ledger, indent=2, sort_keys=True), encoding="utf-8")


def build_desired_bodies(queue_funcs, inventory, cards_dir,
                         only_active=False, only_done=False):
    """Compute the desired {func: {"body": <str>, "done": <bool>}} map.

    active funcs  = the queue funcs (board_sync.load_queue) -> done=False.
    completed     = inventory - queue funcs (the Done cards to seed) -> done=True.
    Bodies come from tmp/cards/<func>.md; funcs with no card file are dropped
    (the caller warns). `done` drives create-as-Done-archived + the finalize
    recovery path (a Done card whose body matched but whose Status/archive never
    completed on a prior, interrupted run).
    """
    desired = {}
    if not only_done:
        for func in queue_funcs:
            body = read_card_body(cards_dir, func)
            if body is not None:
                desired[func] = {"body": body, "done": False}
    if not only_active:
        for func in inventory:
            if func in queue_funcs:
                continue
            body = read_card_body(cards_dir, func)
            if body is not None:
                desired[func] = {"body": body, "done": True}
    return desired


# ---------------------------------------------------------------------------
# reconciler (pure) — never deletes; only update-differing / create-absent
# ---------------------------------------------------------------------------

def reconcile_bodies(desired, by_title, ledger=None):
    """Diff desired {func:{"body","done"}} against the board snapshot `by_title`
    (from list_draft_items) AND the local ledger
    `ledger` ({func: {"draft_id","item_id","finalized","body_sha"}}). Return a
    list of action dicts:

        {"op":"create","func","body","done","bytes","body_sha"}        — absent
        {"op":"update","func","draft_id","item_id","body","bytes","body_sha"}
                                                                 — body changed
        {"op":"finalize","func","item_id"}   — set Status=Done + archive an
            existing (created-but-not-finalized) Done card. No content-write.

    IDEMPOTENCY is hash-based, never via GitHub's read-back (it normalizes the
    stored body, so remote != local is always true). For every desired func we
    compute want_sha = body_sha(local body) and SKIP the body write when the
    ledger already records that exact want_sha.

    Done cards (done=True) — existence is ledger-authoritative (archived =
    invisible to items()):
      - in ledger, finalized==True : skip if body_sha matches; else UPDATE body
        (via stored draft_id), no re-finalize.
      - in ledger, finalized==False: UPDATE body if hash differs, then FINALIZE
        via stored item_id.
      - NOT in ledger, PRESENT in by_title (pre-ledger visible stray): UPDATE
        body if hash differs, then FINALIZE via snapshot item_id.
      - NOT in ledger, NOT in by_title: CREATE.

    Active cards (done=False) — existence via the board snapshot (they're
    visible); "needs update?" via the hash:
      - in ledger with matching body_sha: skip.
      - on board (stale/missing hash): UPDATE (records hash on send).
      - absent from board: CREATE.
      Active cards are NEVER finalized/archived (board_sync owns their Status).

    NEVER deletes; a board card with no desired entry is simply left alone.
    """
    ledger = ledger or {}
    actions = []
    for func, want in desired.items():
        body = want["body"]
        done = want["done"]
        want_sha = body_sha(body)
        led = ledger.get(func)
        sha_ok = led is not None and led.get("body_sha") == want_sha

        if done:
            if led is not None:
                if led.get("finalized"):
                    # Exists + fully done. Skip unless the local body changed.
                    if not sha_ok and led.get("draft_id"):
                        actions.append({"op": "update", "func": func,
                                        "draft_id": led["draft_id"],
                                        "item_id": led.get("item_id"),
                                        "body": body, "bytes": len(body),
                                        "body_sha": want_sha})
                    continue
                # created but not finalized -> update body if changed, finalize.
                if not sha_ok and led.get("draft_id"):
                    actions.append({"op": "update", "func": func,
                                    "draft_id": led["draft_id"],
                                    "item_id": led.get("item_id"),
                                    "body": body, "bytes": len(body),
                                    "body_sha": want_sha})
                actions.append({"op": "finalize", "func": func,
                                "item_id": led.get("item_id")})
                continue
            cur = by_title.get(func)
            if cur is None:
                actions.append({"op": "create", "func": func, "body": body,
                                "done": True, "bytes": len(body),
                                "body_sha": want_sha})
                continue
            # visible stray not in ledger: fix body (no trustworthy hash yet),
            # then finalize.
            actions.append({"op": "update", "func": func, "draft_id": cur["draft_id"],
                            "item_id": cur["item_id"], "body": body,
                            "bytes": len(body), "body_sha": want_sha})
            actions.append({"op": "finalize", "func": func, "item_id": cur["item_id"]})
            continue

        # Active func (done=False): hash-based idempotency; never finalized.
        if sha_ok:
            continue  # we already sent this exact body
        cur = by_title.get(func)
        if cur is None:
            actions.append({"op": "create", "func": func, "body": body,
                            "done": False, "bytes": len(body),
                            "body_sha": want_sha})
            continue
        actions.append({"op": "update", "func": func, "draft_id": cur["draft_id"],
                        "item_id": cur["item_id"], "body": body,
                        "bytes": len(body), "body_sha": want_sha})
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


class _BudgetPoller:
    """Periodic rate-budget gate (I1): instead of spawning `gh api rate_limit`
    before EVERY content-write (~2,500 subprocesses for the full grind), poll
    once and then only every `poll_every` content-writes. On each fresh poll
    (and on the first call) it reads the budget ONCE and hands that same
    (remaining, reset) snapshot to `wait`; if remaining < floor, `wait` sleeps
    until reset. Between poll boundaries `maybe_wait()` is a cheap no-op (no
    subprocess). `poll_every`/`floor`/`wait`/`read` are injectable for tests.
    Graceful degradation on an unparseable rate_limit is inherited from
    maybe_wait_for_budget (unknown budget -> proceed)."""

    def __init__(self, poll_every=BUDGET_POLL_EVERY, floor=RATE_FLOOR,
                 wait=maybe_wait_for_budget, read=read_rate_budget):
        self.poll_every = poll_every
        self.floor = floor
        self._wait = wait
        self._read = read
        self._since_poll = None   # None => never polled (force a poll on first call)
        self.polls = 0            # test-visible: how many times we actually polled
        self.remaining = None     # last-seen budget (for progress prints)

    def maybe_wait(self):
        """Call before a content-write. On a poll boundary: re-poll the budget
        once and possibly sleep; otherwise a cheap no-op."""
        if self._since_poll is not None and self._since_poll < self.poll_every:
            return
        self.polls += 1
        snapshot = self._read()           # (remaining, reset) — ONE subprocess
        self.remaining = snapshot[0]
        self._wait(floor=self.floor, read=lambda: snapshot)
        self._since_poll = 0

    def tick(self):
        """Count a completed content-write toward the next poll boundary."""
        if self._since_poll is None:
            self._since_poll = 0
        self._since_poll += 1


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


def _finalize_done(project_id, field_map, item_id, delay=FIELD_DELAY):
    """Set Status=Done + archive a Done card. Cheap field ops (NOT content-writes)
    and idempotent: re-setting Done / re-archiving an archived item is harmless,
    so this is safe to re-run on the partial-create recovery path (C1)."""
    board_sync._set_field(project_id, field_map, item_id, "Status", "Done", delay)
    board_sync._mutate(board_sync._ARCHIVE,
                       variables={"p": project_id, "i": item_id}, delay=delay)


# ---------------------------------------------------------------------------
# driver
# ---------------------------------------------------------------------------

def run_enrich(queue_path=DEFAULT_QUEUE, map_path=DEFAULT_MAP, elf_path=DEFAULT_ELF,
               cards_dir=DEFAULT_CARDS, ledger_path=DEFAULT_LEDGER,
               project_title=board_sync.PROJECT_TITLE,
               login="tgg12321", dry_run=False, limit=None,
               only_active=False, only_done=False):
    """Read the board + Done-card ledger, compute the desired bodies, and
    reconcile (unless dry_run). Returns a result dict
    {updated, created, finalized, skipped, remaining}."""
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

    ledger = load_ledger(ledger_path)
    by_title = list_draft_items(project_id)
    actions = reconcile_bodies(desired, by_title, ledger)
    n_update = sum(1 for a in actions if a["op"] == "update")
    n_create = sum(1 for a in actions if a["op"] == "create")
    n_finalize = sum(1 for a in actions if a["op"] == "finalize")
    # content-writes are the rate-limited ops; finalize is cheap field ops only.
    n_writes = n_update + n_create
    skipped = len(desired) - len({a["func"] for a in actions})
    print(f"  desired cards: {len(desired)} (active+done, with card files); "
          f"on board: {len(by_title)}; ledger Done cards: {len(ledger)}")
    print(f"  planned content-writes: {n_writes} ({n_update} update, {n_create} create); "
          f"finalize-only: {n_finalize}; already-correct (skipped): {skipped}")

    if dry_run:
        for a in actions[:20]:
            if a["op"] == "finalize":
                print(f"    finalize {a['func']} (Status=Done+archive)")
            else:
                print(f"    {a['op']} {a['func']} ({a['bytes']}b)")
        if len(actions) > 20:
            print(f"    ... and {len(actions) - 20} more")
        print("dry run complete (no writes).")
        return {"updated": 0, "created": 0, "finalized": 0,
                "skipped": skipped, "remaining": len(actions)}

    updated = created = finalized = 0
    total = len(actions)
    # Resolve wait/read from the module at call time so tests that patch
    # board_enrich.maybe_wait_for_budget / read_rate_budget take effect.
    budget = _BudgetPoller(wait=maybe_wait_for_budget, read=read_rate_budget)
    for i, a in enumerate(actions, start=1):
        # --limit caps content-writes (create/update); finalize is cheap.
        if limit is not None and a["op"] in ("create", "update") and (updated + created) >= limit:
            print(f"  --limit {limit} reached; stopping (resumable on re-run).")
            break
        if a["op"] == "finalize":
            print(f"[{i}/{total}] finalize {a['func']} (Status=Done+archive)")
            _finalize_done(project_id, field_map, a["item_id"])
            # ledger: record (visible-stray case adds a new entry; pending case
            # flips finalized) — archived cards are invisible to the next read.
            entry = ledger.setdefault(a["func"], {"draft_id": None})
            entry["item_id"] = a["item_id"]
            entry["finalized"] = True
            save_ledger(ledger_path, ledger)
            finalized += 1
            continue
        # content-write: gate on the (periodically-polled) rate budget first.
        budget.maybe_wait()
        if a["op"] == "update":
            print(f"[{i}/{total}] update {a['func']} ({a['bytes']}b)")
            update_body(a["draft_id"], a["body"])
            # Record body_sha + ids so the next run skips this exact body
            # (GitHub normalizes the stored body; only our hash is trustworthy).
            entry = ledger.setdefault(a["func"], {"finalized": False})
            entry["draft_id"] = a["draft_id"]
            entry["item_id"] = a.get("item_id")
            entry["body_sha"] = a["body_sha"]
            save_ledger(ledger_path, ledger)
            updated += 1
        else:  # create
            print(f"[{i}/{total}] create {a['func']} ({a['bytes']}b)")
            item_id, draft_id = create_card(project_id, a["func"], a["body"])
            if a.get("done"):  # seed Done cards as Status=Done + archived
                # Record the card IMMEDIATELY (before finalize) so a crash in the
                # finalize window still leaves a ledger entry -> resume finalizes
                # it instead of re-creating a duplicate.
                ledger[a["func"]] = {"draft_id": draft_id, "item_id": item_id,
                                     "finalized": False, "body_sha": a["body_sha"]}
                save_ledger(ledger_path, ledger)
                _finalize_done(project_id, field_map, item_id)
                ledger[a["func"]]["finalized"] = True
                save_ledger(ledger_path, ledger)
            else:  # active card created -> record hash + ids (never finalized)
                ledger[a["func"]] = {"draft_id": draft_id, "item_id": item_id,
                                     "finalized": False, "body_sha": a["body_sha"]}
                save_ledger(ledger_path, ledger)
            created += 1
        budget.tick()
        if (updated + created) % 25 == 0:
            print(f"  progress: {updated} updated, {created} created, "
                  f"{finalized} finalized; budget remaining={budget.remaining}")

    remaining = total - (updated + created + finalized)
    print(f"board enrich complete: {updated} updated, {created} created, "
          f"{finalized} finalized, {skipped} skipped, {remaining} remaining-to-do.")
    return {"updated": updated, "created": created, "finalized": finalized,
            "skipped": skipped, "remaining": remaining}


def main():
    ap = argparse.ArgumentParser(description="push rich card bodies + seed Done cards on the BB2 board")
    ap.add_argument("--queue", default=str(DEFAULT_QUEUE))
    ap.add_argument("--map", default=str(DEFAULT_MAP))
    ap.add_argument("--elf", default=str(DEFAULT_ELF))
    ap.add_argument("--cards", default=str(DEFAULT_CARDS), help="dir of <func>.md bodies (board_cards output)")
    ap.add_argument("--ledger", default=str(DEFAULT_LEDGER),
                    help="local Done-card resume ledger (gitignored tmp/ state)")
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
                   cards_dir=Path(a.cards), ledger_path=Path(a.ledger),
                   project_title=a.project, login=a.login,
                   dry_run=a.dry_run, limit=a.limit,
                   only_active=a.only_active, only_done=a.only_done)
    except board_sync.GhError as e:
        sys.exit(f"board enrich failed (GitHub API error): {e}")


if __name__ == "__main__":
    main()
