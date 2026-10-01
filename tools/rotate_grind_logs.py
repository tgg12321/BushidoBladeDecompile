#!/usr/bin/env python3
"""Rotate the append-only Grinder logs down to what the live queue still needs.

    python3 tools/rotate_grind_logs.py [--anchor REF] [--incidents-days N] [--dry-run]

Run from the repo root. Rewrites, LF-only:

  docs/grind/decisions.md   machine-read header (verbatim) + rotation notes + the
                            "Standing owner rulings — index" (one line per OWNER
                            RULING/DECISION entry ever rotated out, pointing at its line
                            at the anchor) + every entry whose `## ` heading names a
                            queued function (engine/queue.json) or a borderline.md alias
                            of one, plus a DISCARDED-SESSION MARKER directly after it.
  docs/grind/journal.md     header comment + rotation notes + the lines naming a queued
                            function/alias.
  docs/tooling_incidents.md header + rotation notes + DEFERRED entries + entries dated
                            within the last N days (default 30).

History stays reachable: the anchor (default HEAD, recorded as its commit hash; a tag
is recorded by name) must hold the full text. Only entries/lines whose exact text exists
in the anchor's copy of the file are rotated out, so uncommitted appends are never lost;
commit them and re-run to rotate them too. Idempotent: a second run with nothing new to
remove leaves every file byte-identical.
"""
import argparse
import datetime
import json
import re
import subprocess
import sys

DECISIONS = "docs/grind/decisions.md"
JOURNAL = "docs/grind/journal.md"
INCIDENTS = "docs/tooling_incidents.md"
QUEUE = "engine/queue.json"
BORDERLINE = "docs/grind/borderline.md"

INDEX_HEAD = "## Standing owner rulings — index"
INDEX_INTRO = ("One line per OWNER RULING/DECISION entry rotated out of this file, pointing at "
               "its line in the anchor commit or tag named. The operative text is the cited "
               "`.claude/rules/` file; later verbatim owner answers are in "
               "docs/grind/owner-rulings-2026-09-26.md.")
OWNER_HEAD = re.compile(r"OWNER (RULING|DECISION|CAMPAIGN|ELECTION|GRANT|APPROVES|RATIFIES|CONFIRMS)"
                        r"|owner rul|owner grants|OWNER-DIRECTED|ELECTION REVISED|owner rules", re.I)
GATE_TOKENS = re.compile(r"OWNER-ESCALATION|CANONICAL-ASM GRANT PATH")
NOTE_MARK = "rotated out;"


def git(*args):
    return subprocess.run(["git", *args], capture_output=True, check=True).stdout.decode("utf-8")


def resolve_anchor(ref):
    """(label used in citations, human description) for the anchor ref."""
    if git("tag", "--list", ref).strip() == ref:
        return ref, "git tag " + ref
    sha = git("rev-parse", "--short=12", ref + "^{commit}").strip()
    return sha, "commit " + sha


def at_anchor(ref, path):
    try:
        return git("show", "%s:%s" % (ref, path)).replace("\r", "")
    except subprocess.CalledProcessError:
        return ""


def read(path):
    with open(path, encoding="utf-8") as f:
        return f.read().replace("\r", "")


def write(path, text, dry):
    if dry:
        return
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


def queued_pattern():
    items = json.loads(read(QUEUE)).get("items", [])
    funcs = {i["func"] for i in items}
    names = set(funcs)
    # the same alias-pair regex as engine/dossier.py aliases()
    for m in re.finditer(r"(\w[\w$]*)\s*(?:\([^)]*\))?\s*=\s*(\w[\w$]*)", read(BORDERLINE)):
        if m.group(1) in funcs or m.group(2) in funcs:
            names.update(m.groups())
    if not names:
        sys.exit("rotate_grind_logs: engine/queue.json lists no functions — refusing to empty the logs")
    return re.compile(r"\b(?:" + "|".join(re.escape(n) for n in sorted(names)) + r")\b"), len(funcs)


def split_sections(lines):
    """(preamble lines, [section line lists]) split at `## ` headings."""
    first = next((i for i, ln in enumerate(lines) if ln.startswith("## ")), len(lines))
    secs, cur = [], None
    for ln in lines[first:]:
        if ln.startswith("## "):
            cur = [ln]
            secs.append(cur)
        else:
            cur.append(ln)
    return lines[:first], secs


def strip_preamble(pre, extra_trailers=()):
    """Header lines without rotation notes, and the notes found."""
    notes = [ln for ln in pre if NOTE_MARK in ln]
    head = [ln for ln in pre if NOTE_MARK not in ln]
    while head and head[-1].strip() in ("",) + tuple(extra_trailers):
        head.pop()
    return head, notes


def add_note(notes, note, removed):
    if removed and note not in notes:
        notes = notes + [note]
    return notes


def anchor_entry_lines(anchor_text):
    """{exact entry text: 1-based heading line} for the anchor's copy."""
    out = {}
    lines = anchor_text.split("\n")
    pre, secs = split_sections(lines)
    n = len(pre) + 1
    for sec in secs:
        out.setdefault("\n".join(sec).rstrip("\n"), n)
        n += len(sec)
    return out


def rotate_decisions(pat, ref, label, where, today, dry):
    lines = read(DECISIONS).rstrip("\n").split("\n")
    pre, secs = split_sections(lines)
    head, notes = strip_preamble(pre)
    index, entries = [], []
    for sec in secs:
        if sec[0].startswith(INDEX_HEAD):
            index = [ln for ln in sec if ln.startswith("- ")]
        else:
            entries.append(sec)
    in_anchor = anchor_entry_lines(at_anchor(ref, DECISIONS))
    keep = set()
    for k, sec in enumerate(entries):
        if pat.search(sec[0]):
            keep.add(k)
            if k + 1 < len(entries) and "DISCARDED-SESSION MARKER" in entries[k + 1][0]:
                keep.add(k + 1)
    titles = {ln.rsplit(" — `", 1)[0] for ln in index}
    out_entries, removed = [], 0
    for k, sec in enumerate(entries):
        key = "\n".join(sec).rstrip("\n")
        if k in keep or key not in in_anchor:
            out_entries.append(sec)
            continue
        removed += 1
        if OWNER_HEAD.search(sec[0]) and not GATE_TOKENS.search(sec[0]):
            title = re.sub(r"\*\*", "", sec[0][3:]).strip()
            if len(title) > 150:
                title = title[:147].rstrip() + "..."
            if "- " + title not in titles:
                titles.add("- " + title)
                index.append("- %s — `%s:%s:%d`" % (title, label, DECISIONS, in_anchor[key]))
    notes = add_note(notes, "> Entries before %s rotated out; full history + any `decisions.md:N` "
                     "citation resolves at %s." % (today, where), removed)
    out = head + [""] + notes + [""]
    if index:
        out += [INDEX_HEAD + " (tools/rotate_grind_logs.py)", "", INDEX_INTRO, ""] + index + [""]
    for sec in out_entries:
        while sec and not sec[-1].strip():
            sec = sec[:-1]
        out += sec + [""]
    write(DECISIONS, "\n".join(out).rstrip("\n") + "\n", dry)
    return removed, len(out_entries)


def rotate_journal(pat, ref, where, today, dry):
    lines = [ln for ln in read(JOURNAL).split("\n") if ln.strip()]
    in_anchor = set(at_anchor(ref, JOURNAL).split("\n"))
    head = [ln for ln in lines if not ln.startswith("- ") and NOTE_MARK not in ln]
    notes = [ln for ln in lines if not ln.startswith("- ") and NOTE_MARK in ln]
    body, removed = [], 0
    for ln in lines:
        if not ln.startswith("- "):
            continue
        if pat.search(ln) or ln not in in_anchor:
            body.append(ln)
        else:
            removed += 1
    notes = add_note(notes, "<!-- Entries before %s rotated out; full history + any `journal.md:N` "
                     "citation resolves at %s. -->" % (today, where), removed)
    write(JOURNAL, "\n".join(head + notes + body) + "\n", dry)
    return removed, len(body)


def rotate_incidents(ref, where, today, days, dry):
    lines = read(INCIDENTS).rstrip("\n").split("\n")
    pre, secs = split_sections(lines)
    head, notes = strip_preamble(pre, extra_trailers=("---",))
    cutoff = (datetime.date.fromisoformat(today) - datetime.timedelta(days=days)).isoformat()
    in_anchor = anchor_entry_lines(at_anchor(ref, INCIDENTS))
    kept, removed = [], 0
    for sec in secs:
        while sec and sec[-1].strip() in ("", "---"):
            sec = sec[:-1]
        key = "\n".join(sec)
        if "DEFERRED" in sec[0] or sec[0][3:13] >= cutoff or key not in in_anchor:
            kept.append(sec)
        else:
            removed += 1
    notes = add_note(notes, "> Entries before %s (other than DEFERRED) rotated out; full history "
                     "resolves at %s." % (cutoff, where), removed)
    out = head + [""] + notes + ["", "---", ""]
    for sec in kept:
        out += sec + [""]
    write(INCIDENTS, "\n".join(out).rstrip("\n") + "\n", dry)
    return removed, len(kept)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--anchor", default="HEAD",
                    help="git ref holding the full history (default HEAD, recorded as its hash)")
    ap.add_argument("--incidents-days", type=int, default=30)
    ap.add_argument("--date", default=datetime.date.today().isoformat(),
                    help="rotation date written into the notes (default today)")
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    label, where = resolve_anchor(a.anchor)
    pat, nfuncs = queued_pattern()
    print("anchor %s; %d queued functions" % (where, nfuncs))
    print("decisions.md: rotated %d entries, kept %d" % rotate_decisions(pat, a.anchor, label, where, a.date, a.dry_run))
    print("journal.md: rotated %d lines, kept %d" % rotate_journal(pat, a.anchor, where, a.date, a.dry_run))
    print("tooling_incidents.md: rotated %d entries, kept %d" % rotate_incidents(a.anchor, where, a.date, a.incidents_days, a.dry_run))


if __name__ == "__main__":
    main()
