#!/usr/bin/env python3
"""Grinder ledger + session logic. Pure stdlib; runs on Windows python.

The ledger (memory/grind/<func>/) is the pipeline's persistent brain:
  state.json     machine state the driver reads (see spec)
  evidence.md    append-only facts bank
  hypotheses.md  append-only hypothesis ledger
  candidate.c    best form so far
  rejected/      judge/reviewer-rejected forms, named for the violated rule

Spec: docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md
"""
import datetime
import json
import os

MODALITIES = ["recon", "structural", "permuter", "forensics", "rederive", "synthesis"]
# Sessions 2..10 cycle through this ladder, then repeat (spec: "ladder repeats from 2").
LADDER = ["structural", "structural", "permuter", "permuter",
          "forensics", "forensics", "rederive", "rederive", "synthesis"]
RESULTS = ("progress", "candidate-ready", "ruling-request")
MAX_FRONTIER = 3


def _now():
    return datetime.datetime.now(datetime.timezone.utc).isoformat()


def ledger_dir(root, func):
    return os.path.join(root, "memory", "grind", func)


def load_state(root, func):
    p = os.path.join(ledger_dir(root, func), "state.json")
    if not os.path.isfile(p):
        return None
    with open(p, encoding="utf-8") as f:
        return json.load(f)


def save_state(root, func, state):
    p = os.path.join(ledger_dir(root, func), "state.json")
    with open(p, "w", encoding="utf-8", newline="\n") as f:
        json.dump(state, f, indent=1)


def init_ledger(root, func, file_stem, origin="queue"):
    """Create the ledger skeleton. Idempotent: never overwrites existing files."""
    d = ledger_dir(root, func)
    os.makedirs(os.path.join(d, "rejected"), exist_ok=True)
    sp = os.path.join(d, "state.json")
    if not os.path.isfile(sp):
        save_state(root, func, {
            "func": func, "file": file_stem, "session_count": 0,
            "current_modality": None, "floor_history": [], "frontier": [],
            "judge_constraints": [], "origin": origin, "created": _now(),
        })
    for name, header in (("evidence.md", f"# Evidence bank — {func}\n"),
                         ("hypotheses.md", f"# Hypothesis ledger — {func}\n")):
        p = os.path.join(d, name)
        if not os.path.isfile(p):
            with open(p, "w", encoding="utf-8", newline="\n") as f:
                f.write(header)
    return d


def append_evidence(root, func, text, session=None):
    p = os.path.join(ledger_dir(root, func), "evidence.md")
    tag = f"[s{session}] " if session else ""
    with open(p, "a", encoding="utf-8", newline="\n") as f:
        f.write(f"\n- {tag}{text}\n")


def append_hypothesis(root, func, h, session=None):
    p = os.path.join(ledger_dir(root, func), "hypotheses.md")
    tag = f"s{session}" if session else "?"
    with open(p, "a", encoding="utf-8", newline="\n") as f:
        f.write(f"\n## [{tag}] {h.get('statement', '?')}\n"
                f"- mechanism: {h.get('mechanism', '?')}\n"
                f"- probe: {h.get('probe', '?')}\n"
                f"- result: {h.get('result', '?')}\n"
                f"- verdict: {h.get('verdict', '?')}\n")
