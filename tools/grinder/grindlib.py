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


def _has_measurement(text):
    return any(ch.isdigit() for ch in str(text))


def validate_outcome(o, modality, root):
    """Return (ok, reason). A session is VALID only if it proves work:
    - candidate-ready / ruling-request have their own requirements;
    - progress requires >=1 hypothesis with a CONFIRMED/KILLED verdict and a
      numeric measurement in its result (recon instead requires frontier+evidence);
    - permuter/forensics sessions must attach >=1 existing artifact file.
    'blocked' does not exist. Anything unproven is invalid and gets discarded."""
    if not isinstance(o, dict):
        return False, "outcome is not a JSON object"
    res = o.get("result")
    if res not in RESULTS:
        return False, f"result must be one of {RESULTS}, got {res!r}"
    if len(o.get("frontier", [])) > MAX_FRONTIER:
        return False, f"frontier exceeds cap of {MAX_FRONTIER}"
    if res == "ruling-request":
        if not str(o.get("ruling_question", "")).strip():
            return False, "ruling-request requires ruling_question"
        return True, ""
    if not isinstance(o.get("floor"), int):
        return False, "floor (int) is required"
    if res == "candidate-ready":
        return True, ""
    # res == progress
    if modality == "recon":
        if not o.get("frontier"):
            return False, "recon must produce an initial frontier"
        if not o.get("evidence"):
            return False, "recon must bank evidence"
        return True, ""
    proven = [h for h in o.get("hypotheses", [])
              if h.get("verdict") in ("CONFIRMED", "KILLED")
              and _has_measurement(h.get("result", ""))]
    if not proven:
        return False, ("progress requires >=1 hypothesis with verdict "
                       "CONFIRMED/KILLED and a numeric measurement in result")
    if modality in ("permuter", "forensics"):
        arts = [a for a in o.get("artifacts", [])
                if os.path.isfile(os.path.join(root, a)) and
                os.path.getsize(os.path.join(root, a)) > 0]
        if not arts:
            return False, f"{modality} session must attach >=1 existing non-empty artifact"
    return True, ""


def assign_modality(session_count):
    """Modality for the NEXT session given completed count. Session 1 = recon;
    sessions 2..10 walk LADDER; then the ladder repeats from 'structural'."""
    if session_count == 0:
        return "recon"
    return LADDER[(session_count - 1) % len(LADDER)]


def apply_outcome(root, func, o, modality):
    """Fold a VALIDATED outcome into the ledger. Appends are never compacted."""
    st = load_state(root, func)
    n = st["session_count"] + 1
    for h in o.get("hypotheses", []):
        append_hypothesis(root, func, h, session=n)
    for e in o.get("evidence", []):
        append_evidence(root, func, e, session=n)
    st["session_count"] = n
    st["current_modality"] = modality
    st["floor_history"].append({"session": n, "floor": o.get("floor"),
                                "modality": modality,
                                "headline": o.get("headline", "")[:200]})
    if o.get("frontier"):
        st["frontier"] = o["frontier"][:MAX_FRONTIER]
    save_state(root, func, st)
    return st


def add_judge_constraint(root, func, text):
    st = load_state(root, func)
    st["judge_constraints"].append(text)
    save_state(root, func, st)
