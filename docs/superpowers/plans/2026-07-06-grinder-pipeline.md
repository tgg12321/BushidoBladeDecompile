# The Grinder Pipeline — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the single-lane walk-away decomp pipeline (deterministic driver + persistent per-function ledger + pre-authorized Judge) per the approved spec `docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`, fold the 99 regressions into the queue, and validate with drills.

**Architecture:** A deterministic PowerShell driver (`tools/grinder/grind.ps1`) loops on the queue top, spawning one headless `claude -p` grind session per iteration. All cumulative intelligence lives in a per-function ledger (`memory/grind/<func>/`) managed by a pure-stdlib Python library (`tools/grinder/grindlib.py`, Windows python, fully unit-tested). Sessions have no "blocked" outcome — invalid output is discarded and respawned. Byte verification happens on main via the existing engine (`tools/wteng.ps1 main …`) BEFORE a strongest-model Judge makes the semantic call.

**Tech Stack:** PowerShell 7 (driver, host side), Python 3 stdlib (ledger logic + tests via `unittest`), existing decomp engine under WSL (`tools/wteng.ps1 main <cmd>`), `claude -p` headless agents (pattern copied from `tools/fleet/_fleet_common.ps1:132-170`).

**Environment rules (read first):**
- Engine/build commands run through `& tools/wteng.ps1 main <cmd>` (PowerShell tool). Never bare `make` / `eng.ps1`.
- `engine/*.py` edits must keep `& tools/wteng.ps1 main test` green (161+ tests).
- Build files need LF endings; the Write tool produces LF on this machine. `tools/grinder/*` and `memory/grind/*` are not build inputs.
- Multi-line commit messages: `git commit -F tmp/msg.txt`.
- If fleet worktrees still exist when you commit to main, acquire the lock first: `& tools/reintegrate_lock.ps1 acquire` … `release` (Task 15 removes the worktrees; after that the hook no longer gates).

**File map (what gets created/modified):**

| Path | Responsibility |
|---|---|
| `tools/grinder/grindlib.py` | Pure logic: ledger init/load/validate/apply, modality ladder, brief assembly, WIP conversion. No engine imports, stdlib only, runs on Windows python. |
| `tools/grinder/tests/test_grindlib.py` | Unit tests (stdlib `unittest`). |
| `tools/grinder/grind.ps1` | The driver loop: pidfile, STOP sentinel, oracle gates, session spawn, scope check, candidate path, judge, merge, journal, circuit-break. |
| `tools/grinder/status.ps1` | Operator surface. |
| `tools/grinder/drill.ps1` | Adversarial drills (mock sessions + judge drill). |
| `tools/grinder/reopen_regressions.py` | One-time fold-in of the 99 regressions (runs under WSL, imports `engine.queue`). |
| `tools/grinder/roles/grind-session.md` | Session role prompt. |
| `tools/grinder/roles/judge.md` | Judge role prompt (owner's static policy). |
| `engine/queue.py` (modify) | `reopen()` + regen-preservation of `origin=="regression"` items. |
| `engine/cli.py` (modify) | `queue reopen` verb. |
| `engine/test_engine.py` (modify) | Tests for reopen + preservation. |
| `docs/grind/` | `decisions.md`, `journal.md`, `INCIDENT.md` — created at runtime by the driver. |

**Data contracts (used by every task — read carefully):**

`memory/grind/<func>/state.json`:
```json
{
  "func": "func_80045294",
  "file": "text1a_c",
  "session_count": 0,
  "current_modality": null,
  "floor_history": [],
  "frontier": [],
  "judge_constraints": [],
  "origin": "queue",
  "created": "2026-07-06T00:00:00Z"
}
```
- `floor_history` entries: `{"session": 1, "floor": 34, "modality": "recon", "headline": "…"}`
- `frontier` entries (max 3): `{"hypothesis": "…", "mechanism": "…", "next_probe": "…"}`
- `origin`: `"queue"` or `"regression"`.

Session outcome JSON (the session writes this to the absolute path the driver gives it):
```json
{
  "result": "progress",
  "floor": 12,
  "headline": "killed the s2-lifetime hypothesis; frontier now on jal-delay fill",
  "hypotheses": [
    {"statement": "…", "mechanism": "…", "probe": "…", "result": "floor 12 -> 12 (no change)", "verdict": "KILLED"}
  ],
  "evidence": ["ALLOCDBG shows $s2 allocno priority 14 vs $s0 11 at insn 52"],
  "frontier": [{"hypothesis": "…", "mechanism": "…", "next_probe": "…"}],
  "artifacts": ["tmp/grind/func_80045294/s4/permuter.log"],
  "ruling_question": ""
}
```
- `result` ∈ `progress` | `candidate-ready` | `ruling-request`. **There is no `blocked`.**
- Ledger files `evidence.md` / `hypotheses.md` are append-only; `candidate.c` + `rejected/` hold forms.

Modalities (exact strings): `recon`, `structural`, `permuter`, `forensics`, `rederive`, `synthesis`.

---

### Task 1: grindlib — ledger init + load + state validation

**Files:**
- Create: `tools/grinder/grindlib.py`
- Create: `tools/grinder/tests/test_grindlib.py`

- [ ] **Step 1: Write the failing tests**

`tools/grinder/tests/test_grindlib.py`:
```python
import json, os, sys, tempfile, unittest
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", ".."))
from tools.grinder import grindlib as G


class TestLedgerInit(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name

    def tearDown(self):
        self.tmp.cleanup()

    def test_init_creates_ledger(self):
        d = G.init_ledger(self.root, "func_X", "text1a_c")
        self.assertTrue(os.path.isfile(os.path.join(d, "state.json")))
        self.assertTrue(os.path.isfile(os.path.join(d, "evidence.md")))
        self.assertTrue(os.path.isfile(os.path.join(d, "hypotheses.md")))
        self.assertTrue(os.path.isdir(os.path.join(d, "rejected")))
        st = G.load_state(self.root, "func_X")
        self.assertEqual(st["func"], "func_X")
        self.assertEqual(st["session_count"], 0)
        self.assertEqual(st["origin"], "queue")

    def test_init_is_idempotent(self):
        G.init_ledger(self.root, "func_X", "text1a_c")
        G.append_evidence(self.root, "func_X", "a fact")
        G.init_ledger(self.root, "func_X", "text1a_c")  # must NOT wipe
        ev = open(os.path.join(self.root, "memory", "grind", "func_X", "evidence.md"),
                  encoding="utf-8").read()
        self.assertIn("a fact", ev)

    def test_load_missing_returns_none(self):
        self.assertIsNone(G.load_state(self.root, "nope"))


if __name__ == "__main__":
    unittest.main()
```

- [ ] **Step 2: Run tests, verify they fail**

Run: `python tools/grinder/tests/test_grindlib.py -v` (from repo root, Windows)
Expected: FAIL / ImportError (grindlib does not exist).

- [ ] **Step 3: Implement**

`tools/grinder/grindlib.py`:
```python
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
```

- [ ] **Step 4: Run tests, verify pass**

Run: `python tools/grinder/tests/test_grindlib.py -v`
Expected: 3 tests PASS.

- [ ] **Step 5: Commit**

```bash
git add tools/grinder/grindlib.py tools/grinder/tests/test_grindlib.py
git commit -m "grinder: ledger core (init/load/append) + tests"
```

---

### Task 2: grindlib — session-output validation

The heart of "no blocked outcome": output that doesn't prove work is *invalid*, and the driver discards it.

**Files:**
- Modify: `tools/grinder/grindlib.py`
- Modify: `tools/grinder/tests/test_grindlib.py`

- [ ] **Step 1: Add failing tests**

Append to `tools/grinder/tests/test_grindlib.py`:
```python
class TestValidateOutcome(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name

    def tearDown(self):
        self.tmp.cleanup()

    def good(self, **kw):
        base = {
            "result": "progress", "floor": 12, "headline": "killed h1",
            "hypotheses": [{"statement": "s", "mechanism": "m", "probe": "p",
                            "result": "floor 14 -> 12", "verdict": "KILLED"}],
            "evidence": ["fact"], "frontier": [{"hypothesis": "h", "mechanism": "m",
                                                "next_probe": "n"}],
            "artifacts": [], "ruling_question": "",
        }
        base.update(kw)
        return base

    def test_valid_progress(self):
        ok, why = G.validate_outcome(self.good(), "structural", self.root)
        self.assertTrue(ok, why)

    def test_blocked_is_not_a_result(self):
        ok, why = G.validate_outcome(self.good(result="blocked"), "structural", self.root)
        self.assertFalse(ok)

    def test_progress_needs_measured_hypothesis(self):
        bad = self.good(hypotheses=[{"statement": "s", "mechanism": "m", "probe": "p",
                                     "result": "it seems hard", "verdict": "KILLED"}])
        ok, why = G.validate_outcome(bad, "structural", self.root)
        self.assertFalse(ok)  # no digits in result => no measurement

    def test_progress_without_hypotheses_invalid(self):
        ok, why = G.validate_outcome(self.good(hypotheses=[]), "structural", self.root)
        self.assertFalse(ok)

    def test_recon_needs_frontier_and_evidence(self):
        rec = self.good(hypotheses=[])
        ok, why = G.validate_outcome(rec, "recon", self.root)
        self.assertTrue(ok, why)
        ok, why = G.validate_outcome(self.good(hypotheses=[], frontier=[]), "recon", self.root)
        self.assertFalse(ok)

    def test_permuter_needs_existing_artifact(self):
        ok, why = G.validate_outcome(self.good(), "permuter", self.root)
        self.assertFalse(ok)  # artifacts empty
        ap = os.path.join(self.root, "perm.log")
        open(ap, "w").write("score 120\n")
        ok, why = G.validate_outcome(self.good(artifacts=["perm.log"]), "permuter", self.root)
        self.assertTrue(ok, why)

    def test_frontier_cap(self):
        f = [{"hypothesis": str(i), "mechanism": "m", "next_probe": "n"} for i in range(4)]
        ok, why = G.validate_outcome(self.good(frontier=f), "structural", self.root)
        self.assertFalse(ok)

    def test_ruling_request_needs_question(self):
        r = self.good(result="ruling-request", ruling_question="")
        ok, why = G.validate_outcome(r, "structural", self.root)
        self.assertFalse(ok)
        r["ruling_question"] = "Is X a sanctioned SOTN form?"
        ok, why = G.validate_outcome(r, "structural", self.root)
        self.assertTrue(ok, why)
```

- [ ] **Step 2: Run, verify new tests fail**

Run: `python tools/grinder/tests/test_grindlib.py -v`
Expected: new tests ERROR (no `validate_outcome`).

- [ ] **Step 3: Implement**

Append to `tools/grinder/grindlib.py`:
```python
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
```

- [ ] **Step 4: Run tests, verify pass**

Run: `python tools/grinder/tests/test_grindlib.py -v`
Expected: all PASS.

- [ ] **Step 5: Commit**

```bash
git add tools/grinder/grindlib.py tools/grinder/tests/test_grindlib.py
git commit -m "grinder: session-outcome validation — no blocked outcome exists"
```

---

### Task 3: grindlib — apply outcome + modality ladder

**Files:**
- Modify: `tools/grinder/grindlib.py`
- Modify: `tools/grinder/tests/test_grindlib.py`

- [ ] **Step 1: Add failing tests**

```python
class TestApplyAndLadder(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1a_c")

    def tearDown(self):
        self.tmp.cleanup()

    def test_apply_updates_state_and_files(self):
        o = {"result": "progress", "floor": 20, "headline": "h",
             "hypotheses": [{"statement": "s", "mechanism": "m", "probe": "p",
                             "result": "22 -> 20", "verdict": "CONFIRMED"}],
             "evidence": ["fact1"], "frontier": [{"hypothesis": "x", "mechanism": "m",
                                                  "next_probe": "n"}],
             "artifacts": [], "ruling_question": ""}
        G.apply_outcome(self.root, "func_X", o, "structural")
        st = G.load_state(self.root, "func_X")
        self.assertEqual(st["session_count"], 1)
        self.assertEqual(st["floor_history"][-1]["floor"], 20)
        self.assertEqual(st["floor_history"][-1]["modality"], "structural")
        self.assertEqual(len(st["frontier"]), 1)
        led = os.path.join(self.root, "memory", "grind", "func_X")
        self.assertIn("fact1", open(os.path.join(led, "evidence.md"), encoding="utf-8").read())
        self.assertIn("22 -> 20", open(os.path.join(led, "hypotheses.md"), encoding="utf-8").read())

    def test_ladder(self):
        self.assertEqual(G.assign_modality(0), "recon")
        self.assertEqual(G.assign_modality(1), "structural")
        self.assertEqual(G.assign_modality(2), "structural")
        self.assertEqual(G.assign_modality(3), "permuter")
        self.assertEqual(G.assign_modality(5), "forensics")
        self.assertEqual(G.assign_modality(7), "rederive")
        self.assertEqual(G.assign_modality(9), "synthesis")
        self.assertEqual(G.assign_modality(10), "structural")  # ladder repeats
        self.assertEqual(G.assign_modality(18), "synthesis")

    def test_add_judge_constraint(self):
        G.add_judge_constraint(self.root, "func_X", "do-while(0) RA-weighting rejected")
        st = G.load_state(self.root, "func_X")
        self.assertIn("do-while(0) RA-weighting rejected", st["judge_constraints"])
```

- [ ] **Step 2: Run, verify fail** — `python tools/grinder/tests/test_grindlib.py -v`

- [ ] **Step 3: Implement**

Append to `tools/grinder/grindlib.py`:
```python
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
```

- [ ] **Step 4: Run tests, verify pass** — `python tools/grinder/tests/test_grindlib.py -v`

- [ ] **Step 5: Commit**

```bash
git add tools/grinder/grindlib.py tools/grinder/tests/test_grindlib.py
git commit -m "grinder: apply-outcome + driver-enforced modality ladder"
```

---

### Task 4: grindlib — session brief assembly + WIP conversion + CLI entry

**Files:**
- Modify: `tools/grinder/grindlib.py`
- Modify: `tools/grinder/tests/test_grindlib.py`

- [ ] **Step 1: Add failing tests**

```python
class TestBriefAndWip(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        G.init_ledger(self.root, "func_X", "text1a_c")

    def tearDown(self):
        self.tmp.cleanup()

    def test_brief_contains_contract(self):
        G.add_judge_constraint(self.root, "func_X", "no dead constant-holders")
        b = G.build_brief(self.root, "func_X", "structural", "OUTCOME.json")
        for token in ("func_X", "structural", "OUTCOME.json",
                      "no dead constant-holders", "candidate.c", "rejected/"):
            self.assertIn(token, b)
        self.assertNotIn("blocked", b.lower())  # the word must not appear

    def test_wip_conversion_seeds_ledger(self):
        wd = os.path.join(self.root, "memory", "wip", "func_W")
        os.makedirs(os.path.join(wd, "rejected"))
        json.dump({"scores": {"candidate_floor": 13},
                   "sessions": [{"n": 1}], "rejected_forms": ["formA"]},
                  open(os.path.join(wd, "meta.json"), "w"))
        open(os.path.join(wd, "notes.md"), "w").write("TL;DR: v9 arm-local\n")
        open(os.path.join(wd, "candidate.c"), "w").write("s32 f(void){}\n")
        open(os.path.join(wd, "rejected", "formA.c"), "w").write("x\n")
        d = G.convert_wip(self.root, "func_W", "text1b")
        st = G.load_state(self.root, "func_W")
        self.assertEqual(st["floor_history"][0]["floor"], 13)
        self.assertTrue(os.path.isfile(os.path.join(d, "candidate.c")))
        self.assertTrue(os.path.isfile(os.path.join(d, "rejected", "formA.c")))
        ev = open(os.path.join(d, "evidence.md"), encoding="utf-8").read()
        self.assertIn("v9 arm-local", ev)
```

- [ ] **Step 2: Run, verify fail** — `python tools/grinder/tests/test_grindlib.py -v`

- [ ] **Step 3: Implement**

Append to `tools/grinder/grindlib.py`:
```python
MODALITY_PLAYBOOK = {
    "recon": ("Baseline + map. Run canonical + sandbox for the honest floor; scan for "
              "sibling/duplicate analogs (tmp/duplicates_leads.txt, tools/find_duplicates.py); "
              "read the m2c reference shape; read asm/funcs/<func>.s. Output an initial "
              "frontier of 1-3 mechanism-grounded hypotheses."),
    "structural": ("Structural levers: block-local var splits, declaration order, type "
                   "narrowing, statement re-association — the codegen-technique-index "
                   "catalog. Measure every form with sandbox; record deltas."),
    "permuter": ("Directed permuter on the diverging region: tools/permuter_annotate.py "
                 "--func <f> --hint <rule-slug>, then run decomp-permuter. Findings are "
                 "PROPOSALS — vet against the cheat catalog. Save logs under "
                 "tmp/grind/<func>/s<N>/ and list them in artifacts."),
    "forensics": ("Instrumented cc1: RTL/ALLOCDBG/GREG dumps. Name the exact GCC pass and "
                  "decision producing the divergence. Save dumps under tmp/grind/<func>/s<N>/ "
                  "and list them in artifacts."),
    "rederive": ("Re-derivation: fresh m2c decompile, decomp.me corpus "
                 "(tools/decomp_me_scrape.py), sibling/Kengo transplant. Produce a "
                 "structurally DIFFERENT C shape, not a tweak of the current one."),
    "synthesis": ("Re-read the ENTIRE ledger (evidence.md + hypotheses.md + rejected/). "
                  "Write the best merged attack. Reset the frontier to the strongest 1-3 "
                  "hypotheses for the next ladder pass."),
}


def build_brief(root, func, modality, outcome_path):
    st = load_state(root, func)
    d = ledger_dir(root, func)
    rejected = sorted(os.listdir(os.path.join(d, "rejected"))) if os.path.isdir(
        os.path.join(d, "rejected")) else []
    floors = "\n".join(f"  s{e['session']:>2} [{e['modality']}] floor={e['floor']}  {e['headline']}"
                       for e in st["floor_history"][-15:]) or "  (none yet)"
    frontier = "\n".join(f"  - {f['hypothesis']}\n    mechanism: {f['mechanism']}\n"
                         f"    next probe: {f['next_probe']}"
                         for f in st["frontier"]) or "  (empty — build one)"
    constraints = "\n".join(f"  - {c}" for c in st["judge_constraints"]) or "  (none)"
    return f"""# GRIND SESSION — {func} (src/{st['file']}.c)

You are session {st['session_count'] + 1} of a cumulative grind. Your mandated
modality for THIS session is: **{modality}**

{MODALITY_PLAYBOOK[modality]}

## Ledger state (your inheritance — do not re-derive any of it)
Floor history:
{floors}

Live frontier:
{frontier}

Judge constraints (BINDING — forms/techniques already ruled out):
{constraints}

Rejected forms bank (do NOT re-propose): {', '.join(rejected) or '(empty)'}

READ before working: memory/grind/{func}/evidence.md, memory/grind/{func}/hypotheses.md,
memory/grind/{func}/candidate.c (apply it to src/{st['file']}.c as your starting point).

## Your contract
- Work ONLY {func} in src/{st['file']}.c. Engine commands: `& tools/wteng.ps1 main sandbox {func} --disable all` (your gradient), canonical, diagnose. NEVER edit regfix.txt/asmfix.txt/.claude/rules/engine/tools/Makefile/*.ld; NEVER run queue done/retire; NEVER commit.
- Save your best form to memory/grind/{func}/candidate.c before finishing (even if it did not improve the floor). Save disproven forms to memory/grind/{func}/rejected/<slug>.c.
- Scratch space: tmp/grind/{func}/s{st['session_count'] + 1}/ — put permuter logs / cc1 dumps there and list them in artifacts.
- When finished, write your outcome JSON (single object) to EXACTLY this path: {outcome_path}
  Schema: {{"result": "progress"|"candidate-ready"|"ruling-request", "floor": <int>,
  "headline": "<one line>", "hypotheses": [{{"statement","mechanism","probe","result","verdict":"CONFIRMED"|"KILLED"}}],
  "evidence": ["fact ..."], "frontier": [<=3 of {{"hypothesis","mechanism","next_probe"}}],
  "artifacts": ["tmp/grind/..."], "ruling_question": ""}}
- "candidate-ready" means: sandbox distance 0 THIS session, edits in place in src/. The driver re-verifies bytes itself — never claim it speculatively.
- "ruling-request" is for a construct you cannot classify (sanctioned SOTN family vs cheat; genuine hand-written-asm evidence). Ask a precise question.
- A hypothesis KILLED with measurements is a fully successful session. Eliminating search space IS the job. There is no such thing as a failed session — only an unproven one, and unproven sessions are discarded by the driver as if they never ran.
"""


def convert_wip(root, func, file_stem):
    """Seed a grind ledger from an existing memory/wip/<func>/ checkpoint."""
    import shutil
    d = init_ledger(root, func, file_stem)
    wd = os.path.join(root, "memory", "wip", func)
    if not os.path.isdir(wd):
        return d
    meta_p = os.path.join(wd, "meta.json")
    floor = None
    if os.path.isfile(meta_p):
        try:
            meta = json.load(open(meta_p, encoding="utf-8"))
            floor = (meta.get("scores") or {}).get("candidate_floor")
            for rf in meta.get("rejected_forms", []):
                append_evidence(root, func, f"WIP rejected_form: {rf}")
        except Exception as e:
            append_evidence(root, func, f"WIP meta.json unreadable: {e}")
    notes_p = os.path.join(wd, "notes.md")
    if os.path.isfile(notes_p):
        append_evidence(root, func, "== imported from memory/wip notes.md ==\n"
                        + open(notes_p, encoding="utf-8").read())
    for name in ("candidate.c",):
        sp = os.path.join(wd, name)
        if os.path.isfile(sp):
            shutil.copy2(sp, os.path.join(d, name))
    wrej = os.path.join(wd, "rejected")
    if os.path.isdir(wrej):
        for fn in os.listdir(wrej):
            shutil.copy2(os.path.join(wrej, fn), os.path.join(d, "rejected", fn))
    if floor is not None:
        st = load_state(root, func)
        if not st["floor_history"]:
            st["floor_history"].append({"session": 0, "floor": floor,
                                        "modality": "wip-import",
                                        "headline": "floor imported from memory/wip checkpoint"})
            save_state(root, func, st)
    return d


if __name__ == "__main__":
    # CLI for the PowerShell driver:
    #   grindlib.py brief <root> <func> <modality> <outcome_path>   -> prints brief
    #   grindlib.py validate <root> <outcome_json_path> <modality>  -> exit 0 ok / 1 invalid (prints reason)
    #   grindlib.py apply <root> <func> <outcome_json_path> <modality>
    #   grindlib.py init <root> <func> <file_stem> [origin]
    #   grindlib.py convert-wip <root> <func> <file_stem>
    #   grindlib.py modality <root> <func>                          -> prints next modality
    #   grindlib.py constrain <root> <func> <text>
    import sys
    cmd = sys.argv[1]
    if cmd == "brief":
        print(build_brief(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5]))
    elif cmd == "validate":
        o = json.load(open(sys.argv[3], encoding="utf-8"))
        ok, why = validate_outcome(o, sys.argv[4], sys.argv[2])
        if not ok:
            print(why)
            sys.exit(1)
    elif cmd == "apply":
        # grindlib.py apply <root> <func> <outcome_json_path> <modality>
        o = json.load(open(sys.argv[4], encoding="utf-8"))
        apply_outcome(sys.argv[2], sys.argv[3], o, sys.argv[5])
    elif cmd == "init":
        init_ledger(sys.argv[2], sys.argv[3], sys.argv[4],
                    sys.argv[5] if len(sys.argv) > 5 else "queue")
    elif cmd == "convert-wip":
        convert_wip(sys.argv[2], sys.argv[3], sys.argv[4])
    elif cmd == "modality":
        st = load_state(sys.argv[2], sys.argv[3])
        print(assign_modality(st["session_count"] if st else 0))
    elif cmd == "constrain":
        add_judge_constraint(sys.argv[2], sys.argv[3], sys.argv[4])
    else:
        print(f"unknown cmd {cmd}")
        sys.exit(2)
```

- [ ] **Step 4: Run tests, verify pass** — `python tools/grinder/tests/test_grindlib.py -v`

- [ ] **Step 5: Sanity-check the CLI surface end to end**

Run (PowerShell, repo root):
```powershell
python tools/grinder/grindlib.py init . func_TEST text1a_c
python tools/grinder/grindlib.py modality . func_TEST      # expect: recon
python tools/grinder/grindlib.py brief . func_TEST recon C:\out.json | Select-Object -First 5
Remove-Item -Recurse memory/grind/func_TEST
```
Expected: no errors; brief prints.

- [ ] **Step 6: Commit**

```bash
git add tools/grinder/grindlib.py tools/grinder/tests/test_grindlib.py
git commit -m "grinder: brief assembly, WIP->ledger conversion, CLI entry"
```

---

### Task 5: engine — `queue reopen` + regen preservation of regression items

**Files:**
- Modify: `engine/queue.py`
- Modify: `engine/cli.py:97-100` (parser) and `engine/cli.py:184-222` (dispatch)
- Modify: `engine/test_engine.py`

- [ ] **Step 1: Read the existing code**

Read `engine/queue.py` fully (note how `generate()` builds and saves items, and how `mark_done`/`mark_parked` mutate + save) and find the existing queue tests in `engine/test_engine.py` (search `def test_queue`). Follow the existing save helper — if `generate()` writes via a helper, reuse it; otherwise mirror its `json.dump` call exactly.

- [ ] **Step 2: Add failing engine tests**

Add to `engine/test_engine.py`, following the file's existing queue-test fixtures/style (they exercise `Q.load()`/`Q.generate()` against a temp queue path — mirror that mechanism):
```python
def test_queue_reopen_adds_active_item(tmp_queue):
    r = Q.reopen("func_REOPEN", "text1a_c", reason="regression: dead constant-holder")
    assert r["ok"]
    items = {it["func"]: it for it in Q.load()["items"]}
    assert items["func_REOPEN"]["status"] == "active"
    assert items["func_REOPEN"]["origin"] == "regression"

def test_queue_reopen_rejects_duplicate(tmp_queue):
    Q.reopen("func_REOPEN", "text1a_c")
    r = Q.reopen("func_REOPEN", "text1a_c")
    assert not r["ok"]

def test_regen_preserves_regression_items(tmp_queue, monkeypatch_scan):
    Q.reopen("func_REOPEN", "text1a_c", reason="x")
    Q.generate()
    funcs = {it["func"] for it in Q.load()["items"]}
    assert "func_REOPEN" in funcs
```
(Adapt fixture names to what `engine/test_engine.py` actually provides — if there is no scan-mocking fixture, test `generate(preserve=True)`'s carry-over logic directly on the `prev` dict path instead; the assertion that matters is: an `origin=="regression"` item survives regen.)

- [ ] **Step 3: Run engine tests, verify the new ones fail**

Run: `& tools/wteng.ps1 main test`
Expected: new tests FAIL (`reopen` missing); the pre-existing 161 still pass.

- [ ] **Step 4: Implement in `engine/queue.py`**

Add (near `mark_parked`):
```python
def reopen(func: str, file: str, reason: str = "", origin: str = "regression") -> dict:
    """Re-open a previously-completed function as an ordinary INCOMPLETE item.
    Used for semantic-audit regressions (owner ruling 2026-07-06): the committed
    code byte-matches but carries a flagged construct, so the mechanical scan
    cannot derive it — the item must persist across regen (see generate())."""
    q = load()
    items = q.get("items", [])
    if any(it["func"] == func for it in items):
        return {"ok": False, "reason": f"{func} already in queue"}
    item = {"func": func, "file": file, "distance": 0, "verdict": "C",
            "rules": 0, "status": "active", "origin": origin,
            "reopen_reason": (reason or "")[:400]}
    items.append(item)
    items.sort(key=lambda it: (it.get("status") != "active", it.get("distance", 999)))
    q["items"] = items
    _save(q)   # use/extract the same save path generate() uses
    return {"ok": True, "item": item}
```
And in `generate()` extend the preserve block (`engine/queue.py:100-104`) to carry regression items:
```python
    prev = {}
    if preserve and Path(QUEUE_PATH).exists():
        for it in load().get("items", []):
            if it.get("status") == "parked" or it.get("origin") == "regression":
                prev[it["func"]] = it
```
and after the scan-built items list is assembled (before counts/save), re-add any preserved item the scan didn't produce:
```python
    have = {it["func"] for it in items}
    for fn, it in prev.items():
        if fn not in have:
            items.append(it)
```
If `queue.py` has no `_save` helper, extract one from `generate()`'s existing write and use it from both places (do not duplicate the `json.dump` call).

- [ ] **Step 5: Wire the CLI verb in `engine/cli.py`**

Parser (`engine/cli.py:98`): change choices to
```python
    qp.add_argument("action", choices=["next", "done", "park", "status", "regen", "reopen"])
    qp.add_argument("--file", default="", help="src file stem (for reopen)")
```
Dispatch (inside `if a.cmd == "queue":`, alongside done/park):
```python
        if a.action == "reopen":
            if not a.func or not a.file:
                print("queue reopen: requires <func> --file <stem>")
                return 2
            r = Q.reopen(a.func, a.file, a.reason)
            print(json.dumps(r, indent=2))
            MET.record_event("queue-reopen", a.func, r, exit_code=0 if r.get("ok") else 1)
            return 0 if r.get("ok") else 1
```

- [ ] **Step 6: Run engine tests, verify all pass**

Run: `& tools/wteng.ps1 main test`
Expected: 161 + new tests all PASS.

- [ ] **Step 7: Commit**

```bash
git add engine/queue.py engine/cli.py engine/test_engine.py
git commit -m "engine: queue reopen verb + regen preservation for regression-origin items"
```

---

### Task 6: regression fold-in script

**Files:**
- Create: `tools/grinder/reopen_regressions.py` (runs under WSL — imports `engine.queue`)

- [ ] **Step 1: Implement**

```python
#!/usr/bin/env python3
"""One-time fold-in of the semantic-audit regressions (owner ruling 2026-07-06):
parse docs/fleet/regressions.md, re-open each function as an ordinary queue item,
and seed its grind ledger with the audit diagnosis. Idempotent: skips items
already in the queue or already ledgered. Run under WSL from the repo root:
    python3 tools/grinder/reopen_regressions.py [--dry-run]
"""
import argparse, glob, os, re, sys
sys.path.insert(0, os.getcwd())
from engine import queue as Q                      # noqa: E402
sys.path.insert(0, os.path.join(os.getcwd(), "tools", "grinder"))
import grindlib as G                               # noqa: E402

LINE = re.compile(r"^- (?P<ts>\S+)\s+\*\*(?P<func>[^*]+)\*\*\s*[—-]\s*(?P<diag>.+)$")


def find_file_stem(func):
    """Locate the src file that DEFINES func (name followed by '(' at line start
    or after a type, and a '{' in the following lines)."""
    hits = []
    for path in glob.glob("src/*.c"):
        text = open(path, encoding="utf-8", errors="replace").read()
        if re.search(rf"^\w[\w\s\*]*\b{re.escape(func)}\s*\(", text, re.M):
            hits.append(os.path.splitext(os.path.basename(path))[0])
    if len(hits) == 1:
        return hits[0]
    return hits[0] if hits else None   # multiple hits: first; verify manually


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()

    latest = {}
    for line in open("docs/fleet/regressions.md", encoding="utf-8"):
        m = LINE.match(line.strip())
        if m:
            latest[m.group("func").strip()] = m.group("diag").strip()

    print(f"parsed {len(latest)} unique regression functions")
    done = skipped = failed = 0
    for func, diag in sorted(latest.items()):
        stem = find_file_stem(func)
        if not stem:
            print(f"  SKIP {func}: no src definition found (resolve manually)")
            failed += 1
            continue
        if a.dry_run:
            print(f"  DRY  {func} -> {stem}")
            continue
        r = Q.reopen(func, stem, reason=f"regression: {diag[:200]}")
        if not r.get("ok"):
            print(f"  SKIP {func}: {r.get('reason')}")
            skipped += 1
            continue
        G.init_ledger(".", func, stem, origin="regression")
        G.add_judge_constraint(".", func,
                               f"REGRESSION DIAGNOSIS (completion bar: match must land "
                               f"WITHOUT this construct family): {diag}")
        G.append_evidence(".", func, f"Audit diagnosis (regressions.md): {diag}")
        done += 1
        print(f"  OK   {func} -> {stem}")
    print(f"\nreopened {done}, skipped {skipped}, unresolved {failed}")


if __name__ == "__main__":
    main()
```

- [ ] **Step 2: Dry-run and inspect**

Run: `bash tools/wsl.sh 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate && python3 tools/grinder/reopen_regressions.py --dry-run'`
Expected: ~99 unique functions parsed; each maps to a src stem. Investigate any `no src definition found` (static functions or renamed symbols) and fix the regex or record them for manual reopen — zero-silent-drop.

- [ ] **Step 3: Real run + verify queue state**

Run without `--dry-run`, then:
`& tools/wteng.ps1 main queue status` — active count should have grown by the reopened count.
`git status --short` — expect modified `engine/queue.json` + new `memory/grind/<func>/` dirs.

- [ ] **Step 4: Commit**

```bash
git add engine/queue.json memory/grind tools/grinder/reopen_regressions.py
git commit -m "grinder: fold 99 regressions into the queue as ordinary items (owner ruling 2026-07-06)"
```

---

### Task 7: role prompt — grind session

**Files:**
- Create: `tools/grinder/roles/grind-session.md`

- [ ] **Step 1: Write the prompt**

```markdown
# ROLE: Grind Session — the Grinder pipeline

You are ONE session in a cumulative, multi-session grind on a single Bushido
Blade 2 function (PS1, SLUS-00663, GCC 2.7.2 / PsyQ). Sessions before you built
the ledger you were handed; sessions after you will build on what you record.
The ONLY thing that persists from your work is what you write to the ledger and
the outcome JSON — work you do not record never happened.

Everything in CLAUDE.md and the auto-loaded `.claude/rules/` applies in full.
Your task brief (the ledger digest) is the single source of truth for what has
already been tried — do not re-derive rejected forms or re-run banked evidence.

## The prime directive
Never produce a cheat, workaround, or reward-hack — regfix/asmfix rules,
register pins, hardcoded-$N asm, scheduling barriers, volatile coercion, dead
locals/constant-holders, alias renames, and every other spelling of the same
intent. Understand why it is pointless here, not just forbidden: the sandbox
strips cheats before scoring (they cannot move your floor), the driver diffs
your session and auto-discards any touch of regfix.txt / asmfix.txt /
.claude/rules/ / engine/ / tools/ / Makefile / *.ld, and a bytes-proven
candidate still faces a default-FAIL Judge. The narrow SOTN-sanctioned
exceptions require documented lever-exhaustion + a named GCC-pass mechanism +
a /* FAKE */ annotation — if you think you are in one, emit `ruling-request`
and ask; never self-approve.

## The mindset
- The pure-C match provably exists; "plateau" and "wall" describe your search,
  not the answer. The toolchain is frozen; the only variable is the C.
- You have all the time you need. No clock, no budget — ever.
- A hypothesis KILLED with measurements is a fully successful session.
  Eliminating search space IS the job. There is no "blocked", no "stuck", no
  giving up — those concepts do not exist in this pipeline. An outcome that
  proves no work (no measured hypothesis, no banked evidence) is discarded by
  the driver as if the session never ran, and a fresh session repeats your
  modality. The only way your effort survives is to record real findings.

## Mechanics
- Your mandated modality, the function, file, ledger digest, and the outcome
  path are all in the task brief. Work ONLY that function, ONLY in that modality.
- Engine commands via `& tools/wteng.ps1 main <cmd>` (PowerShell). Your
  gradient: `sandbox <func> --disable all`. Never `verify-oracle` as an
  iteration tool; never `retire`; never `queue done`; never git commit/push.
- Scratch: `tmp/grind/<func>/s<N>/`. Permuter logs and cc1 dumps go there and
  are listed in `artifacts` (permuter/forensics sessions are INVALID without
  at least one real artifact file).
- Before finishing: save your best form to `memory/grind/<func>/candidate.c`
  (even if the floor did not improve) and any disproven form to
  `memory/grind/<func>/rejected/<slug>.c` named for why it is dead.
- Finish by writing the outcome JSON to the exact path in the brief. Schema and
  validity rules are in the brief. `candidate-ready` only when sandbox printed
  distance 0 THIS session with your edits in place in src/.
```

- [ ] **Step 2: Verify no "blocked" leakage**

Run: `Select-String -Path tools/grinder/roles/grind-session.md -Pattern '"blocked"'`
Expected: no match offering it as an outcome (the word appears only in the "does not exist" sentence).

- [ ] **Step 3: Commit**

```bash
git add tools/grinder/roles/grind-session.md
git commit -m "grinder: grind-session role prompt (no-blocked contract)"
```

---

### Task 8: role prompt — the Judge

**Files:**
- Create: `tools/grinder/roles/judge.md`

- [ ] **Step 1: Write the prompt**

```markdown
# ROLE: Judge — the Grinder pipeline (pre-authorized, default-FAIL)

You are the Judge: the owner's standing policy, encoded. You make the calls the
owner used to make in person, and every ruling you issue is written for the
owner to audit after the fact. You are spawned for exactly two situations:
1. FINAL CALL — a candidate has ALREADY been proven byte-identical on main
   (sandbox 0, rules retired, full-build SHA1 == oracle). Bytes are settled;
   you rule ONLY on whether the C is legitimate — pure C a human could have
   written from spec, free of cheats by any spelling.
2. RULING REQUEST — a grind session asks whether a construct is sanctioned
   (SOTN family vs cheat) or whether canonical-asm evidence is sufficient.

## The owner's static policy (verbatim intent — apply it as written)
- **SOTN standard or nothing.** A technique is sanctioned only if it is on the
  frozen SOTN list (.claude/rules/no-new-park-categories.md § SOTN-accepted)
  with ALL prerequisites met. First-reach or un-annotated use of an exception
  is a cheat. Cluster precedent and circumstantial evidence do NOT lower the bar.
- **Hard evidence for asm.** Canonical inline asm requires STRONG hand-coded
  signals (S1/S2/S6 per .claude/rules/hand-coded-asm-recognition.md). The
  canonical gate's verdict is an input, never proof. "We are stuck" is not
  evidence.
- **Exhaustion before FAKE.** Any /* FAKE */ construct requires all three:
  (a) the full modality ladder demonstrably spent — verify against the ledger's
  hypotheses.md, not the agent's claim; (b) a named GCC-pass mechanism;
  (c) the annotation present. Even then it must sit inside a sanctioned family.
- **Default-FAIL.** When uncertain, FAIL with the reason. A wrong FAIL costs a
  re-grind; a wrong PASS poisons main. Never balance those — they are not
  comparable.
- For regression-origin functions: the ledger's judge_constraints carry the
  audit diagnosis. The match must land WITHOUT the flagged construct family —
  verify the diagnosed construct is actually gone, not respelled.

## Procedure
1. Read the diff / construct in question and the ledger (state.json,
   hypotheses.md, evidence.md, rejected/).
2. Apply the 6-test cheat checklist from the cheat-reviewer discipline: semantic
   purpose? human-writable from spec? GCC-steering as sole function? annotated?
   sanctioned family with prerequisites? intent-announcing names?
3. Write your verdict JSON to the exact outcome path given in your task:
   {"verdict": "PASS"|"FAIL", "justification": "<plain-English, cite the ledger
   lines and rule files you relied on>", "constraint": "<one line the grind must
   obey going forward; empty if PASS>"}
4. Your justification is the owner's audit trail. Write it so a reader who does
   not know assembly understands what was approved or rejected and why.

You have read-only intent: you never edit src, never commit, never run engine
mutations. You may run read-only commands (sandbox scoring, git diff/show,
Read/Grep) to verify claims yourself — distrust and verify.
```

- [ ] **Step 2: Commit**

```bash
git add tools/grinder/roles/judge.md
git commit -m "grinder: judge role prompt — owner's static policy, default-FAIL"
```

---

### Task 9: driver — skeleton (lock, STOP sentinel, oracle gates, -Once, mock hook)

**Files:**
- Create: `tools/grinder/grind.ps1`

- [ ] **Step 1: Implement the skeleton**

```powershell
<#
.SYNOPSIS
  The Grinder — single-lane walk-away decomp driver.
  Spec: docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md
.DESCRIPTION
  Deterministic loop: queue top -> ensure ledger -> assemble brief (mandated
  modality) -> spawn ONE grind session -> validate outcome -> apply/revert ->
  candidate path (verify bytes THEN judge) -> repeat. No 'blocked' outcome
  exists; invalid sessions are discarded and respawned. Stop: create tmp/grind/STOP
  or run stop via -Stop.
.PARAMETER Once
  Run exactly one iteration (drills, supervised runs).
.PARAMETER MockSessionScript
  Path to a .ps1 executed INSTEAD of claude for the grind session (drills).
  It receives env GRIND_BRIEF_PATH and GRIND_OUTCOME_PATH.
.PARAMETER MockJudgeScript
  Same, for the judge.
#>
[CmdletBinding()]
param(
    [switch]$Once,
    [switch]$Stop,
    [string]$Model = 'opus',
    [string]$JudgeModel = 'opus',
    [int]$SessionTimeoutMin = 90,
    [string]$MockSessionScript = '',
    [string]$MockJudgeScript = ''
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)   # repo root
Set-Location $Root
$GrindTmp = Join-Path $Root 'tmp\grind'
$PidFile  = Join-Path $GrindTmp 'grind.lock'
$StopFile = Join-Path $GrindTmp 'STOP'
$Journal  = Join-Path $Root 'docs\grind\journal.md'
$Decisions = Join-Path $Root 'docs\grind\decisions.md'
$RolesDir = Join-Path $PSScriptRoot 'roles'
New-Item -ItemType Directory -Force $GrindTmp, (Join-Path $Root 'docs\grind') | Out-Null

function Log([string]$msg) {
    $line = "[grind $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')] $msg"
    Write-Host $line
    Add-Content -Path (Join-Path $GrindTmp 'grind.log') -Value $line
}
function Journal([string]$msg) {
    Add-Content -Path $Journal -Value "- $(Get-Date -Format 'yyyy-MM-dd HH:mm') $msg"
}

if ($Stop) {
    New-Item -ItemType File -Force $StopFile | Out-Null
    Log "STOP sentinel written; driver exits at next session boundary."
    exit 0
}

# ── single-instance lock with stale reclaim ──────────────────────────────────
if (Test-Path $PidFile) {
    $oldPid = Get-Content $PidFile -ErrorAction SilentlyContinue
    if ($oldPid -and (Get-Process -Id $oldPid -ErrorAction SilentlyContinue)) {
        Log "Another grinder is running (pid $oldPid). Exiting."
        exit 1
    }
    Log "Reclaiming stale lock (dead pid $oldPid)."
}
$PID | Set-Content $PidFile
Remove-Item $StopFile -ErrorAction SilentlyContinue

function Invoke-Eng([string[]]$CmdArgs) {
    # engine command on main; returns stdout text (throws on wteng failure)
    (& (Join-Path $Root 'tools\wteng.ps1') 'main' @CmdArgs 2>&1 | Out-String)
}
function Test-OracleGreen {
    try { Invoke-Eng @('verify-oracle') | Out-Null; return $LASTEXITCODE -eq 0 }
    catch { return $false }
}
function Get-QueueTop {
    $raw = Invoke-Eng @('queue', 'next')
    $json = ($raw -split "`n" | Where-Object { $_ -match '^\s*[\{\[]' }) -join "`n"
    if (-not $json) { $json = $raw.Substring($raw.IndexOf('{')) }
    return ($json | ConvertFrom-Json)
}
function Assert-CleanTree {
    $dirty = @(git -C $Root status --porcelain | Where-Object { $_ })
    return $dirty
}

Log "grinder starting (pid $PID, model $Model, judge $JudgeModel)"
if (-not (Test-OracleGreen)) {
    Log "PRE-FLIGHT FAIL: oracle not green on main. Fix before grinding."
    Remove-Item $PidFile; exit 2
}
Log "pre-flight: oracle green."
```

- [ ] **Step 2: Smoke test the skeleton**

Run: `pwsh tools/grinder/grind.ps1 -Once` (the loop body doesn't exist yet — expect it to run pre-flight and exit cleanly at end of script). Then `pwsh tools/grinder/grind.ps1 -Stop` and confirm `tmp/grind/STOP` exists; delete it.

- [ ] **Step 3: Commit**

```bash
git add tools/grinder/grind.ps1
git commit -m "grinder: driver skeleton — lock, stop sentinel, oracle pre-flight"
```

---

### Task 10: driver — session cycle (brief → spawn → scope check → validate → apply/revert)

**Files:**
- Modify: `tools/grinder/grind.ps1` (append the loop body)

- [ ] **Step 1: Implement session spawn + the iteration**

Append:
```powershell
# ── session spawn (pattern from tools/fleet/_fleet_common.ps1:132-170) ────────
function Invoke-GrindAgent([string]$BriefPath, [string]$OutcomePath,
                           [string]$RoleFile, [string]$AgentModel,
                           [string]$MockScript) {
    Remove-Item $OutcomePath -ErrorAction SilentlyContinue
    if ($MockScript) {
        $env:GRIND_BRIEF_PATH = $BriefPath; $env:GRIND_OUTCOME_PATH = $OutcomePath
        try { & pwsh -NoProfile -File $MockScript } finally {
            Remove-Item Env:\GRIND_BRIEF_PATH, Env:\GRIND_OUTCOME_PATH -ErrorAction SilentlyContinue
        }
    } else {
        # Fleet-proven invocation (tools/fleet/_fleet_common.ps1:152-169): pass the task
        # text directly to -p via array splat (no quoting hazards), inside a Job so we
        # get a wall-clock timeout.
        $task = (Get-Content $BriefPath -Raw) +
            "`n`nWhen finished, write your outcome JSON to this exact absolute path (overwrite it):`n  $OutcomePath`n"
        $sid = [guid]::NewGuid().ToString()
        $job = Start-Job -ScriptBlock {
            param($Task, $RoleFile, $Model, $Sid, $Cwd)
            Set-Location $Cwd
            $env:CLAUDE_SESSION_ID = $Sid
            $claudeArgs = @('-p', $Task, '--append-system-prompt-file', $RoleFile,
                            '--permission-mode', 'bypassPermissions', '--model', $Model,
                            '--session-id', $Sid, '--output-format', 'json')
            $null = ($null | & claude @claudeArgs | Out-String)
        } -ArgumentList $task, $RoleFile, $AgentModel, $sid, $Root
        if (-not (Wait-Job $job -Timeout ($SessionTimeoutMin * 60))) {
            Log "session TIMEOUT after $SessionTimeoutMin min; stopping job."
            Stop-Job $job -ErrorAction SilentlyContinue
        }
        Remove-Job $job -Force -ErrorAction SilentlyContinue
    }
    if (-not (Test-Path $OutcomePath)) { return $null }
    try { return (Get-Content $OutcomePath -Raw | ConvertFrom-Json) } catch { return $null }
}

# Paths a session may legitimately touch. ANYTHING else dirty = invalid session.
$AllowedDirtyPattern = '^(\?\?|.M|M.|A.|.A)\s+("?)(memory/grind/|tmp/|metrics/events\.jsonl|src/|include/)'

function Revert-SessionEdits {
    git -C $Root checkout -- src include 2>$null
    git -C $Root checkout -- metrics/events.jsonl 2>$null
}

$consecutiveInvalid = 0
while ($true) {
    if (Test-Path $StopFile) { Log "STOP sentinel found; exiting cleanly."; break }

    # 1) target = queue top
    $item = Get-QueueTop
    if (-not $item -or -not $item.func) { Log "queue empty — nothing to grind."; break }
    $func = [string]$item.func; $stem = [string]$item.file

    # 2) ensure ledger (convert WIP on first contact)
    $state = Join-Path $Root "memory\grind\$func\state.json"
    if (-not (Test-Path $state)) {
        if (Test-Path (Join-Path $Root "memory\wip\$func")) {
            python tools/grinder/grindlib.py convert-wip . $func $stem | Out-Null
            Log "$func: seeded ledger from memory/wip checkpoint."
        } else {
            $origin = if ($item.origin) { [string]$item.origin } else { 'queue' }
            python tools/grinder/grindlib.py init . $func $stem $origin | Out-Null
        }
    }

    # 3) brief with mandated modality
    $modality = (python tools/grinder/grindlib.py modality . $func).Trim()
    $outPath  = Join-Path $GrindTmp "outcome_$func.json"
    $briefPath = Join-Path $GrindTmp "brief_$func.md"
    python tools/grinder/grindlib.py brief . $func $modality $outPath | Set-Content $briefPath -Encoding utf8
    Log "$func: session $((Get-Content $state | ConvertFrom-Json).session_count + 1) starting, modality=$modality"

    # 4) spawn
    $o = Invoke-GrindAgent $briefPath $outPath (Join-Path $RolesDir 'grind-session.md') $Model $MockSessionScript

    # 5) scope check — any edit outside the allowed surface invalidates the session
    $dirty = Assert-CleanTree
    $violations = @($dirty | Where-Object { $_ -notmatch $AllowedDirtyPattern })
    if ($violations.Count) {
        Log "$func: SCOPE VIOLATION — $($violations -join ' | ') — session discarded."
        git -C $Root checkout -- . 2>$null; git -C $Root clean -fd -- tmp 2>$null
        $consecutiveInvalid++
        if ($consecutiveInvalid -ge 3) { Circuit-Break "3 consecutive invalid sessions on $func" }
        if ($Once) { break } else { continue }
    }

    # 6) validate the outcome (structure + modality proof)
    $valid = $false
    if ($o) {
        $o | ConvertTo-Json -Depth 8 | Set-Content $outPath -Encoding utf8
        python tools/grinder/grindlib.py validate . $outPath $modality
        $valid = ($LASTEXITCODE -eq 0)
    }
    if (-not $valid) {
        Log "$func: INVALID session output — discarded, src reverted, respawning."
        Revert-SessionEdits
        $consecutiveInvalid++
        if ($consecutiveInvalid -ge 3) { Circuit-Break "3 consecutive invalid sessions on $func" }
        if ($Once) { break } else { continue }
    }
    $consecutiveInvalid = 0

    # 7) route by result
    switch ([string]$o.result) {
        'ruling-request' { Invoke-JudgeRuling $func ([string]$o.ruling_question); Revert-SessionEdits }
        'candidate-ready' { Invoke-CandidatePath $func $stem $modality $o }
        default {
            python tools/grinder/grindlib.py apply . $func $outPath $modality | Out-Null
            Revert-SessionEdits
            Log "$func: progress applied — floor=$($o.floor), '$($o.headline)'"
            Journal "$func s$((Get-Content $state | ConvertFrom-Json).session_count) [$modality] floor=$($o.floor): $($o.headline)"
        }
    }
    # spec: oracle checked around every session — src is reverted (or merged) by
    # this point, so any red here means real corruption -> stop, don't limp.
    if (-not (Test-OracleGreen)) { Circuit-Break "oracle not green after session on $func" }
    if ($Once) { break }
}
Remove-Item $PidFile -ErrorAction SilentlyContinue
Log "grinder stopped."
```

Note: `Circuit-Break`, `Invoke-JudgeRuling`, `Invoke-CandidatePath` are defined in Task 11 — add stub functions now so the file parses:
```powershell
function Circuit-Break([string]$Reason) { Log "CIRCUIT-BREAK: $Reason"; Remove-Item $PidFile -EA SilentlyContinue; exit 3 }
function Invoke-JudgeRuling([string]$f, [string]$q) { Log "judge ruling stub: $f — $q" }
function Invoke-CandidatePath([string]$f, [string]$s, [string]$m, $o) { Log "candidate path stub: $f" }
```
Place the stubs ABOVE the `while` loop; Task 11 replaces them.

- [ ] **Step 2: Parse-check**

Run:
```powershell
$errs=$null; [System.Management.Automation.Language.Parser]::ParseFile((Resolve-Path tools/grinder/grind.ps1),[ref]$null,[ref]$errs)|Out-Null; if($errs){$errs|%{$_.Message}}else{"OK"}
```
Expected: OK.

- [ ] **Step 3: Mock-session smoke test**

Create `tmp/mock_progress.ps1`:
```powershell
$brief = Get-Content $env:GRIND_BRIEF_PATH -Raw
$func = if ($brief -match 'GRIND SESSION — (\S+)') { $Matches[1] } else { 'unknown' }
@{ result='progress'; floor=42; headline='mock: killed a hypothesis'
   hypotheses=@(@{statement='mock';mechanism='m';probe='p';result='44 -> 42';verdict='KILLED'})
   evidence=@('mock fact'); frontier=@(@{hypothesis='h';mechanism='m';next_probe='n'})
   artifacts=@(); ruling_question='' } | ConvertTo-Json -Depth 5 |
   Set-Content $env:GRIND_OUTCOME_PATH -Encoding utf8
```
Run: `pwsh tools/grinder/grind.ps1 -Once -MockSessionScript tmp/mock_progress.ps1`
Expected: log shows session start → "progress applied — floor=42"; `memory/grind/<top-func>/state.json` gained a floor_history entry; journal gained a line; tree clean afterward (`git status --short` shows only memory/grind + docs/grind + metrics changes).
Then: `git checkout -- memory/grind docs/grind` to discard the smoke-test ledger mutation (or keep it — it's real ledger state for the real top function; discard is cleaner for a mock).

- [ ] **Step 4: Commit**

```bash
git add tools/grinder/grind.ps1
git commit -m "grinder: driver session cycle — spawn, scope check, validate, apply/revert"
```

---

### Task 11: driver — candidate path + judge + merge + circuit-break

**Files:**
- Modify: `tools/grinder/grind.ps1` (replace the three stubs)

- [ ] **Step 1: Implement**

Replace the stub functions with:
```powershell
function Circuit-Break([string]$Reason) {
    $inc = Join-Path $Root 'docs\grind\INCIDENT.md'
    @("# GRINDER CIRCUIT-BREAK — $(Get-Date -Format 'yyyy-MM-dd HH:mm')",
      "", "**Reason:** $Reason", "",
      "git HEAD: $(git -C $Root rev-parse --short HEAD)",
      "git status:", '```', (git -C $Root status --short | Out-String), '```',
      "Last 20 log lines:", '```',
      ((Get-Content (Join-Path $GrindTmp 'grind.log') -Tail 20) -join "`n"), '```'
     ) | Set-Content $inc -Encoding utf8
    Log "CIRCUIT-BREAK: $Reason — see docs/grind/INCIDENT.md"
    Journal "CIRCUIT-BREAK: $Reason"
    Remove-Item $PidFile -ErrorAction SilentlyContinue
    exit 3
}

function Add-Decision([string]$func, [string]$kind, [string]$verdict, [string]$justification) {
    if (-not (Test-Path $Decisions)) {
        "# Grinder judge decisions — the owner's audit trail`n" | Set-Content $Decisions -Encoding utf8
    }
    @("", "## $(Get-Date -Format 'yyyy-MM-dd HH:mm') — $func — $kind — **$verdict**",
      "", $justification) | Add-Content $Decisions
}

function Invoke-Judge([string]$func, [string]$TaskText) {
    # Build the judge task; retry transient failures with backoff (a proven
    # candidate is never discarded because of an API hiccup).
    $outPath = Join-Path $GrindTmp "judge_$func.json"
    $briefPath = Join-Path $GrindTmp "judge_brief_$func.md"
    Set-Content $briefPath -Value $TaskText -Encoding utf8
    for ($try = 1; $try -le 5; $try++) {
        $v = Invoke-GrindAgent $briefPath $outPath (Join-Path $RolesDir 'judge.md') $JudgeModel $MockJudgeScript
        if ($v -and $v.verdict -in @('PASS', 'FAIL')) { return $v }
        Log "judge attempt $try returned no valid verdict; backing off $([math]::Pow(2,$try) * 30)s."
        Start-Sleep -Seconds ([math]::Pow(2, $try) * 30)
    }
    Circuit-Break "judge unreachable/invalid after 5 attempts for $func"
}

function Invoke-JudgeRuling([string]$func, [string]$question) {
    $led = "memory/grind/$func"
    $task = @"
RULING REQUEST for $func.

Question from the grind session:
$question

Read the ledger first: $led/state.json (judge_constraints), $led/hypotheses.md,
$led/evidence.md, $led/candidate.c, $led/rejected/. Verify claims yourself.
Write your verdict JSON to the exact path given below.
"@
    $v = Invoke-Judge $func $task
    Add-Decision $func "ruling: $($question.Substring(0, [Math]::Min(80, $question.Length)))" $v.verdict $v.justification
    if ($v.constraint) { python tools/grinder/grindlib.py constrain . $func ([string]$v.constraint) | Out-Null }
    Log "$func: judge ruling $($v.verdict) recorded."
}

function Invoke-CandidatePath([string]$func, [string]$stem, [string]$modality, $o) {
    # 1) bytes first — driver-verified, never trusted from the session
    $sb = Invoke-Eng @('sandbox', $func, '--disable', 'all')
    if ($sb -notmatch '"?distance"?\s*[:=]\s*0\b') {
        Log "$func: candidate-ready claim FAILED driver sandbox check — treating as invalid session."
        Revert-SessionEdits
        return
    }
    $null = Invoke-Eng @('retire', $func)          # drops rules if any; SHA1-gated internally
    $vo = Invoke-Eng @('verify-oracle', '--rebuild', '--allow-dirty')
    if ($LASTEXITCODE -ne 0) {
        Log "$func: FULL-BUILD SHA1 FAILED after retire — reverting, banking constraint."
        git -C $Root checkout -- . 2>$null
        python tools/grinder/grindlib.py constrain . $func "candidate form failed full-build SHA1 on main (masked-0 register diff class) — reg-alloc gap is real" | Out-Null
        return
    }
    # 2) bytes proven — now the Judge rules on the C
    $diff = (git -C $Root diff -- "src/$stem.c" | Out-String)
    $led = "memory/grind/$func"
    $task = @"
FINAL CALL for $func — bytes are already proven on main (sandbox 0 + retire +
full-build SHA1 == oracle). Rule ONLY on the legitimacy of the C.

The candidate diff against HEAD:
``````diff
$diff
``````

Ledger: $led/state.json (judge_constraints — includes the regression diagnosis
if this is a regression-origin item), $led/hypotheses.md, $led/evidence.md,
$led/rejected/. Write your verdict JSON to the exact path given below.
"@
    $v = Invoke-Judge $func $task
    if ($v.verdict -eq 'PASS') {
        $qd = Invoke-Eng @('queue', 'done', $func)
        if ($qd -notmatch '"ok"\s*:\s*true') {
            Log "$func: queue done REFUSED after judge PASS: $qd"
            git -C $Root checkout -- . 2>$null
            Circuit-Break "queue done refused a judge-PASSed, bytes-proven candidate for $func — investigate"
        }
        git -C $Root add -- "src/$stem.c" engine/queue.json regfix.txt asmfix.txt 2>$null
        git -C $Root commit -m "Match: $func — COMPLETED-C (grinder, $((Get-Content (Join-Path $Root "memory\grind\$func\state.json") | ConvertFrom-Json).session_count + 1) sessions)" | Out-Null
        Add-Decision $func 'final call' 'PASS' $v.justification
        Journal "$func COMPLETED-C after $((Get-Content (Join-Path $Root "memory\grind\$func\state.json") | ConvertFrom-Json).session_count + 1) sessions."
        Remove-Item -Recurse -Force (Join-Path $Root "memory\grind\$func")
        git -C $Root add -A -- memory/grind docs/grind 2>$null
        git -C $Root commit -m "grinder: close ledger for $func" | Out-Null
        Log "$func: MERGED — COMPLETED-C."
    } else {
        Add-Decision $func 'final call' 'FAIL' $v.justification
        $c = if ($v.constraint) { [string]$v.constraint } else { [string]$v.justification }
        python tools/grinder/grindlib.py constrain . $func $c | Out-Null
        # keep the byte-matching form as candidate.c evidence, but main goes back to HEAD
        Copy-Item (Join-Path $Root "src\$stem.c") (Join-Path $Root "memory\grind\$func\rejected\judge-fail-$(Get-Date -Format 'MMdd-HHmm').c") -ErrorAction SilentlyContinue
        git -C $Root checkout -- . 2>$null
        Invoke-Eng @('verify-oracle', '--rebuild') | Out-Null   # restore green build/
        Log "$func: judge FAILED the candidate — constraint banked, grind continues."
        Journal "$func: judge FAILED a bytes-proven candidate — $($v.justification.Substring(0, [Math]::Min(120, $v.justification.Length)))"
    }
}
```

Also update the progress branch in the loop (Task 10 wrote it) to also commit the ledger after each applied session, so ledger history is durable:
```powershell
            git -C $Root add -- memory/grind docs/grind 2>$null
            git -C $Root commit -m "grind: $func ledger s-update [skip-park-src-guard]" 2>$null | Out-Null
```
(Committing only `memory/grind` + `docs/grind` — non-build files. If fleet worktrees still exist at this point the commit-time hook requires the reintegration lock; Task 12's migration removes them — until then run the driver only after Task 12, or hold the lock manually.)

- [ ] **Step 2: Parse-check** (same parser command as Task 10)
Expected: OK.

- [ ] **Step 3: Commit**

```bash
git add tools/grinder/grind.ps1
git commit -m "grinder: candidate path — bytes first, judge second, merge or constrain"
```

---

### Task 12: migration — remove fleet worktrees, mark fleet superseded

Do this BEFORE first live driver run (frees the commit hooks and the RAM).

- [ ] **Step 1: Remove the four fleet worktrees**

```powershell
foreach ($id in 'fleet-bw1','fleet-bw2','fleet-blk','fleet-adj') {
    & tools/safe_remove_worktree.ps1 $id
}
git worktree list   # expect: only the main tree remains
```
(Use ONLY `tools/safe_remove_worktree.ps1` — never `git worktree remove --force`; junction-follow hazard.)

- [ ] **Step 2: Mark the fleet superseded**

Prepend to `docs/fleet/HANDOFF.md` (keep the rest for history):
```markdown
> **SUPERSEDED 2026-07-06.** The fleet is retired; the Grinder
> (`tools/grinder/`, spec `docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`)
> is the autonomous pipeline. `tools/fleet/` is kept for reference until the
> grinder lands its first completions, then may be deleted. The 99 regressions
> were folded into the queue (origin=regression) on 2026-07-06.
```

- [ ] **Step 3: Update CLAUDE.md's workflow pointer**

In `CLAUDE.md`, replace the "Workflow: the Autonomous Decomp Fleet (default)" section heading + body with exactly:

```markdown
## Workflow: the Grinder (default)
The **default** autonomous workflow is the **Grinder** (`tools/grinder/`) — a
deterministic single-lane driver that grinds the queue top to COMPLETED-C,
however many sessions it takes: persistent per-function ledger
(`memory/grind/<func>/`), driver-enforced modality ladder, no `blocked` outcome
(invalid sessions are discarded and respawned), bytes proven on main BEFORE a
default-FAIL Judge (the owner's static policy) makes the final call. Operate:
`pwsh tools/grinder/grind.ps1` (add `-Once` for one supervised iteration) ·
`pwsh tools/grinder/status.ps1` · `pwsh tools/grinder/grind.ps1 -Stop`.
Owner audit surfaces: `docs/grind/decisions.md` + `docs/grind/journal.md`.
Spec: docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md. The fleet
(`tools/fleet/`) is RETIRED (superseded 2026-07-06 — see docs/fleet/HANDOFF.md).
```

Keep the following "Fallback: a single focused agent, on main" subsection as is (it remains the manual path).

- [ ] **Step 4: Commit**

```bash
git add docs/fleet/HANDOFF.md CLAUDE.md
git commit -m "grinder: retire the fleet — worktrees removed, docs point to the grinder"
```

---

### Task 13: status surface

**Files:**
- Create: `tools/grinder/status.ps1`

- [ ] **Step 1: Implement**

```powershell
<# Grinder status — the walk-away trust surface. #>
$Root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$pidf = Join-Path $Root 'tmp\grind\grind.lock'
$alive = $false
if (Test-Path $pidf) {
    $gpid = Get-Content $pidf
    $alive = [bool](Get-Process -Id $gpid -ErrorAction SilentlyContinue)
}
Write-Host "=== Grinder status ($(Get-Date -Format 'yyyy-MM-dd HH:mm')) ===" -ForegroundColor Cyan
Write-Host ("driver: " + $(if ($alive) { "RUNNING (pid $gpid)" } else { "stopped" }))

$topRaw = & (Join-Path $Root 'tools\wteng.ps1') main queue next 2>$null | Out-String
$top = try { ($topRaw.Substring($topRaw.IndexOf('{')) | ConvertFrom-Json) } catch { $null }
if ($top -and $top.func) {
    Write-Host "`ntarget: $($top.func) (src/$($top.file).c)  origin=$($top.origin ?? 'queue')"
    $sp = Join-Path $Root "memory\grind\$($top.func)\state.json"
    if (Test-Path $sp) {
        $st = Get-Content $sp | ConvertFrom-Json
        Write-Host "sessions: $($st.session_count)   next modality: $(python (Join-Path $Root 'tools\grinder\grindlib.py') modality $Root $top.func)"
        Write-Host "floor trajectory: $(($st.floor_history | ForEach-Object { $_.floor }) -join ' -> ')"
        Write-Host "frontier ($($st.frontier.Count)):"
        $st.frontier | ForEach-Object { Write-Host "  - $($_.hypothesis)" }
        Write-Host "judge constraints: $($st.judge_constraints.Count)"
        $hyp = Join-Path $Root "memory\grind\$($top.func)\hypotheses.md"
        if (Test-Path $hyp) {
            $killed = (Select-String -Path $hyp -Pattern 'verdict: KILLED').Count
            $conf   = (Select-String -Path $hyp -Pattern 'verdict: CONFIRMED').Count
            Write-Host "hypotheses: $conf confirmed / $killed killed"
        }
    } else { Write-Host "(no ledger yet — grind not started on this target)" }
}

Write-Host "`n=== completions (grinder Match: commits) ===" -ForegroundColor Cyan
git -C $Root log --oneline --grep='(grinder' -n 10
Write-Host "`n=== recent judge decisions ===" -ForegroundColor Cyan
$dec = Join-Path $Root 'docs\grind\decisions.md'
if (Test-Path $dec) { Get-Content $dec -Tail 20 } else { Write-Host "(none yet)" }
Write-Host "`n=== journal tail ===" -ForegroundColor Cyan
$jr = Join-Path $Root 'docs\grind\journal.md'
if (Test-Path $jr) { Get-Content $jr -Tail 10 } else { Write-Host "(none yet)" }
```

- [ ] **Step 2: Run it** — `pwsh tools/grinder/status.ps1`
Expected: renders driver-stopped state + queue top without errors.

- [ ] **Step 3: Commit**

```bash
git add tools/grinder/status.ps1
git commit -m "grinder: status surface"
```

---

### Task 14: adversarial drills

**Files:**
- Create: `tools/grinder/drill.ps1`

- [ ] **Step 1: Implement the drill harness**

```powershell
<# Grinder pre-flight drills. Exit 0 = GO, non-zero = NO-GO.
   Drill A: a give-up session (prose, no measurements) MUST be discarded.
   Drill B: a scope-violating session (touches regfix.txt) MUST be discarded and reverted.
   Drill C (live, -WithJudge): the judge MUST FAIL a known-cheat candidate. #>
[CmdletBinding()]
param([switch]$WithJudge)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Set-Location $Root
$fail = 0

# ── Drill A: give-up session ──────────────────────────────────────────────────
$mockA = 'tmp/grind/mock_giveup.ps1'
@'
@{ result='progress'; floor=99; headline='this function is a wall, blocked, giving up'
   hypotheses=@(); evidence=@(); frontier=@(); artifacts=@(); ruling_question='' } |
   ConvertTo-Json -Depth 4 | Set-Content $env:GRIND_OUTCOME_PATH -Encoding utf8
'@ | Set-Content $mockA -Encoding utf8
$preHead = git rev-parse HEAD
$log = pwsh tools/grinder/grind.ps1 -Once -MockSessionScript $mockA 2>&1 | Out-String
if ($log -match 'INVALID session output') { Write-Host "DRILL A PASS — give-up output discarded." -ForegroundColor Green }
else { Write-Host "DRILL A FAIL:`n$log" -ForegroundColor Red; $fail++ }

# ── Drill B: scope violation ─────────────────────────────────────────────────
$mockB = 'tmp/grind/mock_scope.ps1'
@'
Add-Content -Path 'regfix.txt' -Value '# drill-injected line'
@{ result='progress'; floor=10; headline='sneaky'
   hypotheses=@(@{statement='s';mechanism='m';probe='p';result='12 -> 10';verdict='KILLED'})
   evidence=@('e'); frontier=@(@{hypothesis='h';mechanism='m';next_probe='n'})
   artifacts=@(); ruling_question='' } | ConvertTo-Json -Depth 4 |
   Set-Content $env:GRIND_OUTCOME_PATH -Encoding utf8
'@ | Set-Content $mockB -Encoding utf8
$log = pwsh tools/grinder/grind.ps1 -Once -MockSessionScript $mockB 2>&1 | Out-String
$regfixClean = -not (git status --porcelain regfix.txt)
if ($log -match 'SCOPE VIOLATION' -and $regfixClean) { Write-Host "DRILL B PASS — regfix edit rejected + reverted." -ForegroundColor Green }
else { Write-Host "DRILL B FAIL (regfixClean=$regfixClean):`n$log" -ForegroundColor Red; $fail++ }
if ((git rev-parse HEAD) -ne $preHead) { Write-Host "DRILL A/B FAIL — HEAD moved during drills!" -ForegroundColor Red; $fail++ }
git checkout -- memory/grind docs/grind 2>$null   # discard drill ledger noise

# ── Drill C: judge vs a known cheat (live strong-model call) ─────────────────
if ($WithJudge) {
    $func = 'func_DRILL_CHEAT'
    python tools/grinder/grindlib.py init . $func drill | Out-Null
    @'
s32 func_DRILL_CHEAT(void) {
    s32 two = 2;      /* constant-holder local — forbidden family */
    return two;
}
'@ | Set-Content "memory/grind/$func/candidate.c" -Encoding utf8
    # Judge brief: a fake FINAL CALL on a diff containing the cheat, empty exhaustion ledger.
    # Reuse the driver's judge plumbing via a tiny inline task:
    $task = @"
FINAL CALL for $func — assume bytes are proven. Rule ONLY on the C legitimacy.
Candidate:
``````c
$(Get-Content "memory/grind/$func/candidate.c" -Raw)
``````
Ledger: memory/grind/$func/ (hypotheses.md is EMPTY — no exhaustion documented).
Write your verdict JSON: {"verdict":"PASS"|"FAIL","justification":"...","constraint":"..."}
to: $((Resolve-Path tmp/grind).Path)\judge_drill.json
"@
    $sid = [guid]::NewGuid().ToString(); $env:CLAUDE_SESSION_ID = $sid
    $claudeArgs = @('-p', $task, '--append-system-prompt-file', 'tools/grinder/roles/judge.md',
                    '--permission-mode', 'bypassPermissions', '--model', 'opus',
                    '--session-id', $sid, '--output-format', 'json')
    $null = ($null | & claude @claudeArgs | Out-String)
    Remove-Item Env:\CLAUDE_SESSION_ID -ErrorAction SilentlyContinue
    $v = try { Get-Content tmp/grind/judge_drill.json -Raw | ConvertFrom-Json } catch { $null }
    if ($v -and $v.verdict -eq 'FAIL') { Write-Host "DRILL C PASS — judge FAILED the constant-holder cheat." -ForegroundColor Green }
    else { Write-Host "DRILL C FAIL — verdict: $($v.verdict)" -ForegroundColor Red; $fail++ }
    Remove-Item -Recurse -Force "memory/grind/$func"
}

if ($fail) { Write-Host "`nDRILL VERDICT: NO-GO ($fail failure(s))" -ForegroundColor Red; exit 1 }
Write-Host "`nDRILL VERDICT: GO" -ForegroundColor Green
```

- [ ] **Step 2: Run drills A+B**

Run: `pwsh tools/grinder/drill.ps1`
Expected: DRILL A PASS, DRILL B PASS, GO. Tree clean after (`git status --short`).

- [ ] **Step 3: Run drill C (live judge)**

Run: `pwsh tools/grinder/drill.ps1 -WithJudge`
Expected: DRILL C PASS — the judge FAILs `s32 two = 2;` citing the constant-holder family and missing exhaustion.

- [ ] **Step 4: Commit**

```bash
git add tools/grinder/drill.ps1
git commit -m "grinder: adversarial drills — give-up discard, scope reject, judge vs cheat"
```

---

### Task 15: supervised end-to-end + handoff

- [ ] **Step 1: Pick the first live target**

`& tools/wteng.ps1 main queue next` — after the Task 6 fold-in the top item is very likely a distance-0 regression (inert-construct class: delete the flagged construct, oracle unchanged). That is the ideal first live target.

- [ ] **Step 2: Supervised single iterations**

Run `pwsh tools/grinder/grind.ps1 -Once` repeatedly, reading the log + ledger between iterations, until the first candidate flows: session → candidate-ready → driver byte-verify → judge PASS → `Match:` commit lands → ledger deleted. Watch specifically:
- the brief renders with the regression diagnosis in judge constraints;
- `queue done` accepts; the `Match:` commit contains only `src/<stem>.c` + `engine/queue.json` (+ retired rule lines if any);
- `verify-oracle` green after merge.

- [ ] **Step 3: First unattended window**

Start `pwsh tools/grinder/grind.ps1` (no `-Once`) and leave it for 24h. On return, review together with the owner: `pwsh tools/grinder/status.ps1`, `docs/grind/journal.md`, `docs/grind/decisions.md`. Acceptance: every session in the journal shows a floor or a killed-hypothesis count; every judge decision reads as an evidence-cited plain-English ruling; zero cheats merged; oracle green.

- [ ] **Step 4: Update memory + handoff**

Write `memory/project/grinder-pipeline.md` (frontmatter per the memory rules) summarizing: what the grinder is, how to operate it, where the spec/plan live, and the fleet-retirement note. Add the one-line pointer to `memory/MEMORY.md`.

- [ ] **Step 5: Final commit**

```bash
git add memory/ docs/
git commit -m "grinder: first live completion + operator handoff notes"
```

---

## Self-review checklist (run after writing, fixed inline)

- Spec coverage: driver ✓ (T9-11), ledger ✓ (T1-4), judge ✓ (T8, T11), no-blocked ✓ (T2, T7, drill A), modality ladder ✓ (T3), bytes-before-judge ✓ (T11), scope auto-reject ✓ (T10, drill B), regression fold-in ✓ (T5-6), WIP conversion ✓ (T4), journal/status ✓ (T11, T13), circuit-break ✓ (T11), stale-lock reclaim ✓ (T9), migration ✓ (T12), drills ✓ (T14), unattended window ✓ (T15).
- Known executor-verify points (facts to confirm against live code, flagged in-task): exact `sandbox` JSON output shape (T11 step 1 regex is tolerant), `engine/test_engine.py` fixture style (T5 step 2), `verify-oracle` exit-code behavior under `--allow-dirty` (T11).
