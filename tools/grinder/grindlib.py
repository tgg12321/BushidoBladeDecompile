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
RESULTS = ("progress", "candidate-ready", "ruling-request", "owner-gated")
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
    """Single-writer by design: exactly one driver process mutates a ledger at a
    time (sessions run exclusively). Atomic write so a crash cannot tear state.json."""
    p = os.path.join(ledger_dir(root, func), "state.json")
    tmp = p + ".tmp"
    with open(tmp, "w", encoding="utf-8", newline="\n") as f:
        json.dump(state, f, indent=1)
    os.replace(tmp, p)


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
    for fitem in o.get("frontier", []):
        if not isinstance(fitem, dict) or not all(
                k in fitem for k in ("hypothesis", "mechanism", "next_probe")):
            return False, "frontier items require hypothesis/mechanism/next_probe keys"
    if res == "ruling-request":
        if not str(o.get("ruling_question", "")).strip():
            return False, "ruling-request requires ruling_question"
        return True, ""
    if res == "owner-gated":
        # Only legal when a filed OWNER-ESCALATION already exists for this
        # function in docs/grind/decisions.md — the session must cite it, and
        # the driver parks the item so the queue advances while the owner rules.
        ref = str(o.get("escalation_ref", "")).strip()
        if not ref:
            return False, ("owner-gated requires escalation_ref citing the "
                           "OWNER-ESCALATION entry in docs/grind/decisions.md")
        dec = os.path.join(root, "docs", "grind", "decisions.md")
        try:
            with open(dec, encoding="utf-8") as f:
                txt = f.read()
        except OSError:
            return False, "owner-gated: docs/grind/decisions.md not readable"
        if "OWNER-ESCALATION" not in txt:
            return False, ("owner-gated: no OWNER-ESCALATION entry found in "
                           "docs/grind/decisions.md")
        # EXHAUSTION IS THE DRIVER'S CALL (guard added 2026-07-28). The
        # standing-ruling TERMINAL disposition is only available once the driver
        # has declared exhaustion by assigning `escalation` modality (flat floor
        # across >=4 distinct modalities). A session in any other modality that
        # self-applies it is ending a function the ladder has not finished —
        # func_8006B92C was disposed at 4 sessions / 3 modalities this way.
        # Integration handoffs and gate-PASSING escalations stay legal anywhere.
        if ("RESOLVED BY STANDING RULING" in ref and modality != "escalation"):
            return False, ("owner-gated: the standing-ruling terminal disposition "
                           f"requires `escalation` modality (driver-declared "
                           f"exhaustion), not `{modality}`. A dead axis in this "
                           "modality is a `progress` outcome with the kills banked — "
                           "the ladder still has untried modalities.")
        # (the driver additionally verifies the entry names THIS function
        # before parking — validate_outcome does not know the func name)
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


# Escalation trigger (2026-07-22): the ladder used to repeat forever with no way
# to transition a genuinely-exhausted function to a disposition. Sessions kept
# returning `progress` with "escalation-ready, no owner entry filed" — obeying the
# keep-going directive while the owner-gated brief's chicken-and-egg wording made
# them think they couldn't self-file. func_8007DC9C burned 40 sessions at a flat
# floor this way. When the floor has been FLAT for this many sessions across at
# least this many DISTINCT modalities (real multi-modality exhaustion, not a
# premature give-up while the floor is still moving), assign_modality forces the
# `escalation` modality: file the OWNER-ESCALATION + return owner-gated.
ESCALATION_FLAT_SESSIONS = 8
ESCALATION_MIN_MODALITIES = 4


def _exhaustion_ready(state):
    """True when the honest floor is stuck > 0 across a long, modality-diverse
    run — the signal that pure-C levers are genuinely exhausted and the function
    should be dispositioned, not ground further."""
    if not state:
        return False
    hist = state.get("floor_history", [])
    if len(hist) < ESCALATION_FLAT_SESSIONS:
        return False
    window = hist[-ESCALATION_FLAT_SESSIONS:]
    # Floors are normally ints, but WIP-imported ledgers can carry a prose string
    # there — treat any non-int floor as "unknown" and never escalate on it (avoids
    # a str-vs-int comparison crash and a bogus escalation on malformed history).
    floors = [e.get("floor") for e in window]
    if any(not isinstance(f, int) for f in floors):
        return False
    top = floors[0]
    if top <= 0:                           # already matched → never escalate
        return False
    if any(f != top for f in floors):      # floor still moving → keep grinding
        return False
    mods = {e.get("modality") for e in window}
    return len(mods) >= ESCALATION_MIN_MODALITIES


def assign_modality(session_count, state=None):
    """Modality for the NEXT session. Session 1 = recon; then walk/repeat LADDER —
    UNLESS the function is exhaustion-ready (flat floor across many modalities), in
    which case force `escalation` so the run reaches a disposition instead of
    looping. `state` is optional for back-compat; without it the trigger never fires."""
    if session_count == 0:
        return "recon"
    if _exhaustion_ready(state):
        return "escalation"
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
                                "headline": (o.get("headline") or "")[:200]})
    if o.get("frontier"):
        st["frontier"] = o["frontier"][:MAX_FRONTIER]
    save_state(root, func, st)
    return st


def add_judge_constraint(root, func, text):
    st = load_state(root, func)
    st["judge_constraints"].append(text)
    save_state(root, func, st)


def autoescalate(root, func, file_stem, scan_tier, rule_count, date):
    """Deterministic backstop: append a disposition entry for `func` to
    docs/grind/decisions.md from the ledger's exhaustion record, and return the
    escalation_ref line. The driver calls this when an `escalation`-modality session
    fails to self-file (dodges with a flat-floor progress), so the function can never
    loop unresolved. Per the owner's 2026-07-27 standing auto-ruling, a non-STRONG
    scan tier means both AND-gates fail and the entry is RESOLVED (terminal
    OWNER-ACCEPTED INCOMPLETE) — only a STRONG tier files a true pending
    escalation for owner sign-off."""
    st = load_state(root, func) or {}
    hist = st.get("floor_history", [])
    floor = hist[-1].get("floor") if hist else "?"
    sessions = st.get("session_count", len(hist))
    mods = sorted({e.get("modality") for e in hist if e.get("modality")})
    strong = "STRONG" in str(scan_tier).upper()
    if strong:
        ref = f"{date} — {func} — OWNER-ESCALATION (auto-filed by driver, exhaustion backstop; STRONG scan tier — awaiting owner ruling)"
        tail = f"""Gate 1 may PASS: `scan_hand_coded --single {func}` = **{scan_tier}** — canonical-asm
authorization requires owner sign-off. Awaiting owner ruling; the driver parks {func} until
the owner rules."""
    else:
        ref = f"{date} — {func} — OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE (auto-filed by driver, exhaustion backstop)"
        tail = f"""Both AND-gates fail on the ledger evidence: canonical-asm — `scan_hand_coded --single
{func}` = **{scan_tier}** (ordinary GCC RA/scheduler artifact, no hand-coded signature);
coercion family — no SOTN-master precedent recorded for the residual axes. Per the owner's
STANDING AUTO-RULING (2026-07-27, .claude/rules/endgame-lock-disposition.md): option (b)
applies immediately — REFUSED / OWNER-ACCEPTED INCOMPLETE, any retained cheat holds the
byte-match only and is not sanctioned, parked terminally out of active grind, eligible for
re-attempt if a genuine pure-C lever emerges. No owner action is pending."""
    entry = f"""
## {ref}

**Auto-filed by the grinder driver ({date})** after {sessions} sessions held the honest
floor flat at {floor} across {len(mods)} distinct modalities ({', '.join(mods)}) without a
session self-filing — the escalation-modality backstop (grind.ps1). This is the endgame-lock
species per the standing 2026-07-20 endgame-lock-disposition policy: byte-matches on main only
via a cheat ({rule_count} regfix/asmfix rule(s) or cheat-asm), honest pure-C floor {floor},
sanctioned levers exhausted across the full modality ladder (see memory/grind/{func}/
evidence.md + hypotheses.md for the per-session kill record). {tail}
"""
    dec = os.path.join(root, "docs", "grind", "decisions.md")
    with open(dec, "a", encoding="utf-8", newline="\n") as f:
        f.write(entry)
    return ref


MODALITY_PLAYBOOK = {
    "recon": ("Baseline + map. Run canonical + sandbox for the honest floor; scan for "
              "sibling/duplicate analogs (tmp/duplicates_leads.txt, tools/find_duplicates.py); "
              "read the m2c reference shape; read asm/funcs/<func>.s. Output an initial "
              "frontier of 1-3 mechanism-grounded hypotheses."),
    "structural": ("Structural levers: block-local var splits, declaration order, type "
                   "narrowing, statement re-association — the codegen-technique-index "
                   "catalog. Measure every form with sandbox; record deltas."),
    "permuter": ("Directed permuter on the diverging region: tools/permuter_annotate.py "
                 "--func <f> --hint <rule-slug>. Campaigns run ONLY via "
                 "tools/permuter_campaign.py launch/harvest (telemetry; owner directive "
                 "2026-07-07). Fresh-seed discipline per permuter-directives §Campaign "
                 "discipline: a basin yields early or not at all — if ~20-30 min after a "
                 "fresh seed there is no NOVEL find, harvest --stop and reseed a "
                 "structurally different chassis or switch lever; harvest+stop EVERY "
                 "campaign before your session ends (a 0-find harvest is the data point); "
                 "campaigns must not outlive the session — and you are a ONE-SHOT "
                 "process: ending your turn to 'wait on' a running campaign kills the "
                 "session with no outcome (it is discarded); wait IN-turn via "
                 "`tools/permuter_campaign.py wait --dir <ws>` (ONE blocking call per "
                 "~9-min window — never hand-poll across turns), then "
                 "harvest --stop, then write your outcome. Findings are PROPOSALS — vet "
                 "against the cheat catalog. Save logs under tmp/grind/<func>/s<N>/ and "
                 "list them in artifacts."),
    "forensics": ("Instrumented cc1: RTL/ALLOCDBG/GREG dumps. Name the exact GCC pass and "
                  "decision producing the divergence. Save dumps under tmp/grind/<func>/s<N>/ "
                  "and list them in artifacts."),
    "rederive": ("Re-derivation: fresh m2c decompile, decomp.me corpus "
                 "(tools/decomp_me_scrape.py), sibling/Kengo transplant. Produce a "
                 "structurally DIFFERENT C shape, not a tweak of the current one."),
    "synthesis": ("Re-read the ENTIRE ledger (evidence.md + hypotheses.md + rejected/). "
                  "Write the best merged attack. Reset the frontier to the strongest 1-3 "
                  "hypotheses for the next ladder pass."),
    "escalation": ("DISPOSITION SESSION — the honest floor has been FLAT across many "
                   "sessions and >=4 distinct modalities, so the driver has determined the "
                   "pure-C levers are exhausted. Your job THIS session is to REACH A "
                   "DISPOSITION, not to grind another variant. Valid outcomes: (1) if you "
                   "find a genuinely un-tried lever that DROPS the floor, use it — return "
                   "candidate-ready (if it hits 0) or progress WITH THE LOWER FLOOR (this "
                   "resets the exhaustion counter). (2) Otherwise evaluate the two "
                   "endgame-lock AND-gates: (a) run `python3 tools/scan_hand_coded.py "
                   "--single <func>` and note the tier; (b) confirm what holds the "
                   "byte-match (regfix/asmfix rule count, or cheat-asm); (c) state whether "
                   "an in-hand SOTN-master precedent EXISTS for the closing construct "
                   "(file+line citation — 'same spirit' does not count). If BOTH gates FAIL "
                   "(scan LOW + no precedent — the common case), APPLY THE OWNER'S STANDING "
                   "RULING (2026-07-27, .claude/rules/endgame-lock-disposition.md): APPEND "
                   "an `## <date> — <func> — **OWNER-ESCALATION — RESOLVED BY STANDING "
                   "RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**` entry to "
                   "docs/grind/decisions.md stating both gates' evidence and the exhaustion "
                   "(sessions/modalities/permuter iters from the ledger), then return "
                   "result=owner-gated with escalation_ref citing that entry — the driver "
                   "parks terminally, no owner wait. ONLY if a gate PASSES (STRONG scan "
                   "tier, or an actually-exhibited SOTN precedent) file a true pending "
                   "`**OWNER-ESCALATION**` entry presenting that evidence for owner "
                   "sign-off. A flat-floor `progress` is NOT an acceptable outcome this "
                   "session — the driver will auto-file the standing ruling if you dodge."),
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
  Schema: {{"result": "progress"|"candidate-ready"|"ruling-request"|"owner-gated", "floor": <int>,
  "headline": "<one line>", "hypotheses": [{{"statement","mechanism","probe","result","verdict":"CONFIRMED"|"KILLED"}}],
  "evidence": ["fact ..."], "frontier": [<=3 of {{"hypothesis","mechanism","next_probe"}}],
  "artifacts": ["tmp/grind/..."], "ruling_question": "", "escalation_ref": ""}}
- "candidate-ready" means: sandbox distance 0 THIS session, edits in place in src/. The driver re-verifies bytes itself — never claim it speculatively.
- "ruling-request" is for a construct you cannot classify (sanctioned SOTN family vs cheat; genuine hand-written-asm evidence). Ask a precise question.
- "owner-gated" is for when every remaining sanctioned axis is measured dead. FILE the entry in docs/grind/decisions.md YOURSELF THIS session (docs/grind/ is in your allowed surface), THEN return owner-gated with escalation_ref citing it — you do not wait for an entry to pre-exist, you create it. The driver verifies the entry names {func} and parks the function so the queue advances. Never use it to defer work that is still grindable (a floor still dropping is grindable). This is the mandated outcome in `escalation` modality.
- OWNER'S STANDING AUTO-RULING (2026-07-27) — this governs HOW you word an escalation, and it NEVER authorizes ending a function early. Two separate questions, do not conflate them:
  (A) IS THE FUNCTION EXHAUSTED? This is the DRIVER's call, not yours. The driver assigns `escalation` modality only after the honest floor has been FLAT across many sessions AND >=4 DISTINCT modalities. If your mandated modality is NOT `escalation`, the answer is NO — you may not dispose of the function, however dead your own axis looks. A killed axis is a `progress` outcome with the kills banked; the ladder still has untried modalities (forensics / rederive / synthesis) and the owner's standing directive is to work the top item to completion however many sessions it takes ([[no-deferral-work-to-completion]], [[difficult-is-not-impossible]]). Judge FAILs do NOT make a function exhausted — a FAILed construct is one dead lever, and a FAIL on annotation FORMAT is a one-comment fix, not a wall.
  (B) ONCE THE DRIVER HAS DECLARED EXHAUSTION (you are in `escalation` modality), evaluate the two endgame-lock AND-gates: (1) canonical-asm needs STRONG `scan_hand_coded` signals (S1/S2/S6); (2) a coercion/spelling family needs an in-hand SOTN-master precedent you can CITE (file+line or commit). "Same spirit", "genre-adjacent", "only lever left", "measured to work", and a partition/elimination argument do NOT qualify — and a census you ran that came back NEGATIVE is a FAILED gate, not an open question. If BOTH gates fail (the overwhelmingly common case), the owner has PRE-DECIDED it: title your entry `## <date> — <func> — **OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): REFUSED / OWNER-ACCEPTED INCOMPLETE**`, state both gates' evidence, and return owner-gated. That is terminal — the driver parks it and NOTHING is pending on the owner. Do NOT write "awaiting owner ruling" / "pending owner action" / a multi-option packet for a both-gates-fail case; that is a process error. ONLY when a gate genuinely PASSES (STRONG scan tier, or a real cited precedent) do you file a pending `**OWNER-ESCALATION**` for owner sign-off.
- Bytes proven but blocked ONLY by a surface you may not touch (regfix.txt/asmfix.txt/prologue_config.json/inline_asm_canonical.txt) is an INTEGRATION HANDOFF, not an endgame lock: say so plainly in the entry, list the exact operator steps, and return owner-gated. Do not dress it up as exhaustion — and note the operator still runs a fresh layer-2 cheat-reviewer on your C before it is accepted, so a Judge PASS on a construct is not a guarantee of acceptance.
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
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
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
        print(assign_modality(st["session_count"] if st else 0, st))
    elif cmd == "constrain":
        add_judge_constraint(sys.argv[2], sys.argv[3], sys.argv[4])
    elif cmd == "autoescalate":
        # autoescalate <root> <func> <file_stem> <scan_tier> <rule_count> <date>
        print(autoescalate(sys.argv[2], sys.argv[3], sys.argv[4],
                           sys.argv[5], sys.argv[6], sys.argv[7]))
    else:
        print(f"unknown cmd {cmd}")
        sys.exit(2)
