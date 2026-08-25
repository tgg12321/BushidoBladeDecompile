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
import re
import sys

MODALITIES = ["recon", "structural", "permuter", "solver", "forensics", "rederive", "synthesis"]
# Sessions walk this ladder ONCE; a flat floor across the cycle forces the
# escalation modality (R1, owner ruling 2026-08-19) — never a second cycle.
# R2 (modality-effectiveness 2026-08-19, owner ruling asm-until-matched): the single
# synthesis pass runs at s6 — it is the only modality with deep-plateau yield (4/44
# after 3+ flat vs 2% baseline; all three 5+-flat drops in the dataset), its value is
# entirely in its FIRST pass, and at its old s10 slot it sat OUTSIDE the 8-session
# flat window, so exhaustion could fire before the one plateau-breaker ever ran.
# Revert trigger: ~15 early-synthesis sessions with zero drops.
# "solver" (added 2026-08-24, escalation-not-parked rollout): ra_solver /
# sched_solver inverse search — sits after synthesis, before forensics: it is
# strictly downstream of a dump but cheaper than a full forensics session and
# converts guess-and-measure into typed REACHABLE/FORECLOSED verdicts. Built
# for the RA/scheduler-tiebreak endgame class returning to the active lane.
LADDER = ["structural", "structural", "permuter", "permuter", "synthesis",
          "solver", "forensics", "forensics", "rederive", "rederive"]
RESULTS = ("progress", "candidate-ready", "ruling-request", "owner-gated")
MAX_FRONTIER = 3

# ── Self-vet artifact (2026-08-07 review-audit fix #2) ───────────────────────
# 46% of Judge FAILs were cheat-by-spelling where the worker prompt never
# carried the standards, and 10/18 cited a rule whose own text excluded the
# construct. The standards now front-load into roles/grind-session.md; this is
# the matching LAYER-1 ARTIFACT: a candidate-ready session must have answered
# the 6 tests IN WRITING, quoted the SCOPE SENTENCE of every rule it claims,
# and cited the SOTN precedent as file:line or a commit hash. A session that
# cannot write this down does not have a defensible candidate, and the driver
# discards it exactly like a scope violation.
SELF_VET_NAME = "self_vet.md"
# Each entry: (regex, human explanation of what is missing)
_SELF_VET_REQUIRED = (
    (r"(?im)^\s*CONSTRUCTS\s*:", "a `CONSTRUCTS:` line enumerating every construct in the diff"),
    (r"(?im)^\s*#*\s*T1\b", "the T1 (semantic purpose) answer"),
    (r"(?im)^\s*#*\s*T2\b", "the T2 (human-programmer) answer"),
    (r"(?im)^\s*#*\s*T3\b", "the T3 (GCC-internals justification) answer"),
    (r"(?im)^\s*#*\s*T4\b", "the T4 (permuter/search provenance) answer"),
    (r"(?im)^\s*#*\s*T5\b", "the T5 (family check) answer"),
    (r"(?im)^\s*#*\s*T6\b", "the T6 (naming-announces-intent) answer"),
    (r"(?im)^\s*SANCTIONED-FAMILY-CLAIMS\s*:",
     "a `SANCTIONED-FAMILY-CLAIMS:` section (write `none` if you claim no family)"),
    (r"(?im)^\s*ANNOTATION-CONFORMANCE\s*:",
     "an `ANNOTATION-CONFORMANCE:` line (write `n/a — no FAKE construct` if none)"),
)
# A claimed family must carry BOTH a verbatim scope quote and a hard citation.
_FAMILY_BLOCK = re.compile(r"(?im)^\s*FAMILY\s*:\s*(.+)$")
_SCOPE_LINE = re.compile("(?im)^\\s*SCOPE\\s*:\\s*[\"“](.+?)[\"”]\\s*$")
_PRECEDENT_LINE = re.compile(r"(?im)^\s*PRECEDENT\s*:\s*(.+)$")
# file:line, or a git hash (>=7 hex). "same spirit" is explicitly not a citation.
_CITATION = re.compile(r"([\w./\\-]+\.\w+:\d+)|(\b[0-9a-f]{7,40}\b)")


def self_vet_path(root, func):
    return os.path.join(ledger_dir(root, func), SELF_VET_NAME)


def validate_self_vet(root, func):
    """Return (ok, reason) for memory/grind/<func>/self_vet.md.

    Mechanical only — it cannot tell a good answer from a bad one. Its job is to
    make the ANSWERS EXIST before a Judge cycle is spent, and to refuse the two
    failure shapes the audit measured: a family claimed with no verbatim scope
    sentence, and a precedent asserted with no file:line/commit citation."""
    p = self_vet_path(root, func)
    if not os.path.isfile(p):
        return False, (f"candidate-ready requires a self-vet artifact at "
                       f"memory/grind/{func}/{SELF_VET_NAME} (6 tests answered in "
                       f"writing, scope sentences quoted, precedents cited) — none found")
    try:
        with open(p, encoding="utf-8", errors="replace") as f:
            txt = f.read()
    except OSError as e:
        return False, f"self_vet.md unreadable: {e}"
    if len(txt.strip()) < 80:
        return False, "self_vet.md is effectively empty"
    missing = [why for rx, why in _SELF_VET_REQUIRED if not re.search(rx, txt)]
    if missing:
        return False, "self_vet.md is missing " + "; ".join(missing)
    # Family claims: every FAMILY: block needs a quoted SCOPE: and a cited PRECEDENT:
    fams = _FAMILY_BLOCK.findall(txt)
    if fams:
        scopes = _SCOPE_LINE.findall(txt)
        precs = _PRECEDENT_LINE.findall(txt)
        if len(scopes) < len(fams):
            return False, (f"self_vet.md claims {len(fams)} sanctioned family/families but "
                           f"quotes only {len(scopes)} verbatim SCOPE sentence(s) — every "
                           "claimed family needs its rule's scope sentence in quotes")
        if len(precs) < len(fams):
            return False, (f"self_vet.md claims {len(fams)} sanctioned family/families but "
                           f"gives only {len(precs)} PRECEDENT line(s)")
        for pr in precs:
            if not _CITATION.search(pr):
                return False, (f"self_vet.md PRECEDENT {pr.strip()!r} is not a citation — "
                               "give file:line or a commit hash ('same spirit' does not count)")
        # Citation hygiene (2026-08-19 audit: 1/3 of layer-1 FAILs were
        # right-construct/wrong-paperwork; func_800453E0 FAILed purely on a
        # nonexistent rule file). Cheap, deterministic, pre-Judge:
        # (a) every repo-path citation must resolve to a real file;
        for pr in precs:
            for m in _CITATION.finditer(pr):
                if not m.group(1):
                    continue  # commit hash — skip
                path = m.group(1).rsplit(":", 1)[0].replace("\\", "/")
                while path.startswith("./"):
                    path = path[2:]
                if _is_repo_shaped_citation(path) and \
                        not os.path.isfile(os.path.join(root, path)):
                    return False, (f"self_vet.md PRECEDENT cites {path!r}, which does not "
                                   "exist in the repo — an unresolvable citation is an "
                                   "automatic FAIL at review; fix the citation (or cite a "
                                   "commit hash) before candidate-ready")
        # (b) a claimed family whose rule mandates a /* FAKE */ annotation
        #     cannot ship with ANNOTATION-CONFORMANCE "n/a" (func_80034F88
        #     FAILed exactly this way, asserting the family had no requirement).
        ann = _ANNOTATION_LINE.search(txt)
        ann_txt = ann.group(1).strip().lower() if ann else ""
        if ann_txt.startswith("n/a"):
            for fam in fams:
                fl = fam.lower()
                hit = next((slug for slug, keys in _FAKE_MANDATORY_FAMILIES.items()
                            if any(k in fl for k in keys)), None)
                if hit:
                    return False, (f"self_vet.md claims family {fam.strip()!r}, whose rule "
                                   f"({hit}) MANDATES a /* FAKE: ... */ annotation, but "
                                   "ANNOTATION-CONFORMANCE says 'n/a' — annotate the "
                                   "construct (what + mechanism + lever-exhaustion) or drop "
                                   "the family claim")
    return True, ""


_REPO_TOPDIRS = {".claude", "docs", "memory", "tools", "engine", "asm"}


def _is_repo_shaped_citation(path):
    """True only for paths that are unambiguously THIS repo's — never for
    external-project precedents (SOTN cites like src/main/psxsdk/... or
    src/dra/42398.c are legitimate and must not be existence-checked).
    BB2's src/ and include/ are FLAT, so a src|include path counts as
    repo-shaped only with exactly one path segment after the topdir."""
    parts = path.split("/")
    top = parts[0]
    if top in _REPO_TOPDIRS:
        return True
    if top in ("src", "include"):
        return len(parts) == 2
    return False
# Families whose rule files mandate a /* FAKE */ annotation, keyed by rule slug,
# matched against the vet's FAMILY: line by keyword (family names are free
# text). Sourced from .claude/rules/no-new-park-categories.md prerequisites —
# update BOTH places if a family's annotation requirement ever changes.
_FAKE_MANDATORY_FAMILIES = {
    "dead-store-fake-exception": ("dead-store", "dead store", "self-assign"),
    "named-local-fake-exception": ("named-local", "constant-holder",
                                   "constant holder", "dead scalar"),
    "pointer-alias-fake-exception": ("pointer-alias", "pointer alias"),
    "duplicated-statement-into-arms": ("duplicated-statement",
                                       "duplicated statement"),
    "dead-vars-local-array": ("local array", "dead-vars", "written-never-read",
                              "pad local", "frame pad"),
    "do-while-zero-exception": ("do-while", "do while", "while (0)", "while(0)"),
    "staged-value-reused-variable": ("staged-value", "staged value"),
    "defeat-licm-hoist-var-reuse": ("variable reuse", "variable-reuse",
                                    "var-reuse", "licm"),
    "narrow-byte-args-packed-call": ("named-intermediate",
                                     "named intermediate"),
}
_ANNOTATION_LINE = re.compile(r"(?im)^\s*ANNOTATION-CONFORMANCE\s*:\s*(.+)$")


def _declared_constructs(root, func):
    """Self-vet text, ORIGINAL CASE (section headings are ALL-CAPS)."""
    try:
        with open(self_vet_path(root, func), encoding="utf-8", errors="replace") as f:
            return f.read()
    except OSError:
        return ""


def _significant_terms(text):
    """Content words of a banned-construct phrase, for substring matching.

    Deliberately crude: the ban is a TRIPWIRE, not a parser. If enough of the
    phrase's content words appear in the DECLARED constructs, the session is
    re-proposing the banned thing under some spelling and the driver makes it
    explain itself rather than burning a Judge cycle.

    (Weighting these toward code-ish 'distinctive' tokens was tried and
    reverted: most bans are plain English — "shared return label with goto end
    accumulator" — and demanding identifiers made those bans inert, which
    silently disarms the tripwire instead of narrowing it.)"""
    stop = {"the", "a", "an", "of", "to", "in", "for", "and", "or", "is", "it",
            "that", "this", "with", "on", "as", "by", "be", "was", "not", "no",
            "construct", "family", "form", "use", "using", "used", "cheat"}
    return [w for w in re.findall(r"[a-z0-9_()*]{4,}", str(text).lower())
            if w not in stop]


# The declared-constructs block: the CONSTRUCTS: line through to the next
# structural boundary. Terminators are the format's OWN mandated markers
# (_SELF_VET_REQUIRED: T1..T6, SANCTIONED-FAMILY-CLAIMS:, ANNOTATION-CONFORMANCE:),
# plus any other heading-like line, plus a blank line.
#
# A first cut ended the section at the next ALL-CAPS heading and fell back to
# the whole file when it found none. That fallback silently restored the very
# false positive it was meant to fix the moment a vet used differently-cased
# headings (func_800645B0 tripped again at 19:55 on 2026-08-12 for exactly
# that reason). Boundaries are matched case-INSENSITIVELY now, and there is no
# whole-file fallback: a vet with no CONSTRUCTS: line is format-invalid and
# _SELF_VET_REQUIRED rejects it on its own.
_CONSTRUCTS_SEC = re.compile(
    r"^[ \t]*CONSTRUCTS[ \t]*:.*?"
    r"(?=(?:^[ \t]*(?:\#*[ \t]*T[0-9]\b|[A-Za-z][A-Za-z0-9 /_-]{2,}[ \t]*:|[ \t]*$))|\Z)",
    re.M | re.S | re.I)

# Declarations are short; this bounds the damage if a vet runs prose straight
# into the block with no blank line or heading to end it.
_DECL_MAX = 2000

_DISCLAIM = re.compile(
    r"\b(not present|is banned|was banned|banned and|no longer|not used|"
    r"does not|do not|did not|never|avoided|avoid|without|retired|removed|"
    r"rejected|forbidden|must not|cannot)\b")


def _strip_disclaimers(text):
    """Drop sentences that ASSERT ABSENCE. A vet is REQUIRED to address a
    standing ban, and "X is BANNED and is NOT present here" is the honest way to
    do it — reading that as a re-declaration punishes the wanted behaviour."""
    return " ".join(s for s in re.split(r"(?<=[.;:])\s+|\n", text)
                    if not _DISCLAIM.search(s))


def _ban_trips(ban_text, vet_text):
    """The tripwire match: enough of the ban's content words appear in what the
    vet DECLARES it used.

    SCOPE is the fix for the 2026-08-12 false positive (func_800645B0). The
    check used to read the WHOLE vet, so a long prose ban — mostly domain
    vocabulary like 'relocated / from / inner / loop / body' — matched any vet
    that merely DISCUSSED the banned axis. It discarded a session whose vet
    honestly said the construct "is BANNED and is NOT present here", throwing
    away a sandbox-0 candidate, and it would fire again on every honest vet:
    sessions are REQUIRED to reason about a standing ban, so reading that
    reasoning as a re-declaration punishes exactly the wanted behaviour.

    Now: scan ONLY the declared-constructs block, and drop sentences that
    assert ABSENCE. No CONSTRUCTS: line means no declaration to check — such a
    vet is format-invalid and _SELF_VET_REQUIRED rejects it separately, so
    falling back to a whole-file scan here would only resurrect the bug.
    A construct used but never declared is a vet-completeness failure the
    default-FAIL Judge catches; this tripwire only saves Judge cycles on
    respelling loops, so erring toward the Judge is the safe direction."""
    terms = _significant_terms(ban_text)
    if len(terms) < 2:
        return False, []
    m = _CONSTRUCTS_SEC.search(vet_text)
    if not m:
        return False, []
    decl = _strip_disclaimers(m.group(0)[:_DECL_MAX].lower())
    hits = [t for t in terms if t in decl]
    return len(hits) >= max(2, int(len(terms) * 0.5)), hits


# Minimal text every FORMAT-VALID self-vet must contain, by _SELF_VET_REQUIRED's
# own demands plus the checklist's standard heading vocabulary. A ban whose
# significant terms trip on THIS template would auto-discard every valid vet a
# session could possibly write — a deadlock, not a tripwire. That happens when
# a layer-1 reviewer's evidence[].construct field carried prose about the vet's
# PAPERWORK rather than a C construct (func_800401CC, 2026-08-11: the banked
# phrase "Annotation-conformance claim ('One FAKE construct')" collided with
# the mandatory `ANNOTATION-CONFORMANCE:` line and every vet auto-discarded
# until a Judge removed the entry). add_banned_construct refuses such entries.
_VET_TEMPLATE_MIN = (
    "constructs: "
    "t1 semantic purpose t2 human-programmer t3 gcc-internals justification "
    "t4 permuter/search provenance t5 family check t6 naming-announces-intent "
    "sanctioned-family-claims: none "
    "annotation-conformance: n/a - no fake construct"
)


def check_banned_constructs(root, func):
    """Return (ok, reason). A candidate whose self-vet re-declares a construct
    the Judge already banned for this function is rejected before the Judge sees
    it — the audit found 59% of FAILs sat in respelling loops."""
    st = load_state(root, func) or {}
    banned = st.get("banned_constructs") or []
    if not banned:
        return True, ""
    vet = _declared_constructs(root, func)
    if not vet:
        return True, ""
    for b in banned:
        tripped, hits = _ban_trips(b, vet)
        if tripped:
            return False, (f"self-vet re-declares a BANNED construct for {func}: {b!r} "
                           f"(matched on {', '.join(hits[:5])}). A banned construct "
                           "respelled is the same construct — change the attack, not the "
                           "spelling, or emit ruling-request.")
    return True, ""


def add_banned_construct(root, func, text):
    """Bank a Judge/layer-1 banned construct. Returns True if banked.

    REFUSES (returns False) an entry that would trip on the minimal valid-vet
    template: such an entry describes vet paperwork or review process, not a C
    construct, and banking it deadlocks the function (every format-valid vet
    auto-discards). The reviewer's finding still lives in judge_constraints —
    only the mechanical tripwire is refused."""
    tripped, hits = _ban_trips(text, _VET_TEMPLATE_MIN)
    if tripped:
        return False
    st = load_state(root, func)
    st.setdefault("banned_constructs", [])
    if text and text not in st["banned_constructs"]:
        st["banned_constructs"].append(text)
    save_state(root, func, st)
    return True


def unban_construct(root, func, needle):
    """Clear banned_constructs entries containing `needle` — driver-invoked
    ONLY, on a Judge verdict that explicitly narrows/supersedes the ban
    (integration-handoff-self-serve, owner ruling 2026-08-19). The narrowing
    itself stays in judge_constraints; this clears just the mechanical
    tripwire so a Judge-authorized resubmission is not auto-discarded
    (func_8002D518 deadlocked twice on exactly that). Returns entries removed."""
    st = load_state(root, func)
    if not st:
        return 0
    before = st.get("banned_constructs") or []
    kept = [b for b in before if needle not in b]
    removed = len(before) - len(kept)
    if removed:
        st["banned_constructs"] = kept
        save_state(root, func, st)
    return removed


# integration-handoff-self-serve (owner ruling 2026-08-19): path classes the
# driver may grant into scope_allow.txt. Everything else — and the explicit
# denylist — stays owner-only (the "most severe blockers" list).
_SCOPE_GRANT_ALLOWED_RE = re.compile(r"^(include/[\w.\-/]+\.h|src/[\w.\-/]+\.c|[\w\-]+\.txt)$")
_SCOPE_GRANT_DENY = {
    "inline_asm_canonical.txt",       # has its own evidence-gated grant path
    "maspsx_label_nop_funcs.txt",     # assembler fidelity gates: substrate-adjacent,
    "expand_lb_funcs.txt",            # owner-only
    "expand_dest_funcs.txt",
    "multu_funcs.txt",
    "multu_pad_funcs.txt",
}


def add_scope_allow(root, func, paths, date):
    """Driver-written per-function scope_allow.txt grant on a Judge
    ESCALATE(integration-handoff) verdict (integration-handoff-self-serve,
    owner ruling 2026-08-19). Widens scope, never standards: every granted
    path is BOTH scope-checked and staged into the Match commit, and the fix
    still passes layer-1 + Judge + full-build SHA1. Refuses (returns None) any
    path outside the allowed classes or on the denylist. Merges with an
    existing line for the function. LF-enforced."""
    clean = []
    for p in paths:
        p = str(p).strip().replace("\\", "/").lstrip("./")
        if not p:
            continue
        if p in _SCOPE_GRANT_DENY or not _SCOPE_GRANT_ALLOWED_RE.match(p):
            return None
        clean.append(p)
    if not clean:
        return None
    fpath = os.path.join(root, "tools", "grinder", "scope_allow.txt")
    with open(fpath, encoding="utf-8") as f:
        lines = f.read().split("\n")
    merged = list(dict.fromkeys(clean))
    out, replaced = [], False
    for ln in lines:
        t = ln.strip()
        if t and not t.startswith("#") and t.split()[0] == func:
            prior = t.split()[1:]
            merged = list(dict.fromkeys(prior + merged))
            replaced = True
            continue
        out.append(ln)
    while out and out[-1] == "":
        out.pop()
    out.append(f"# {func}: pipeline scope grant {date} (judge ESCALATE integration-handoff,")
    out.append("# integration-handoff-self-serve owner ruling 2026-08-19; packet in docs/grind/decisions.md).")
    entry = f"{func} " + " ".join(merged)
    out.append(entry)
    out.append("")
    with open(fpath, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(out))
    return entry if not replaced else entry + "  (merged with prior line)"


def set_pending_fixup(root, func, kind, detail):
    """Queue a tiny-scope fix-up brief for the NEXT session (one-shot)."""
    st = load_state(root, func)
    st["pending_fixup"] = {"kind": kind, "detail": detail, "set": _now()}
    save_state(root, func, st)


def clear_pending_fixup(root, func):
    st = load_state(root, func)
    if st.get("pending_fixup"):
        st["pending_fixup"] = None
        save_state(root, func, st)


def advance_modality(root, func):
    """Force the NEXT session onto a DIFFERENT rung of the ladder.

    Called after a construct-class FAIL: repeating the same modality is how a
    respelling loop starts, so the driver bumps a persistent ladder offset until
    the next assignment differs from the modality that produced the FAIL."""
    st = load_state(root, func)
    if st.get("pending_fixup"):
        return assign_modality(st.get("session_count", 0), st)   # fix-up wins; nothing to rotate
    cur = assign_modality(st.get("session_count", 0), st)
    for _ in range(len(LADDER)):
        st["ladder_skip"] = 1 + int(st.get("ladder_skip") or 0)
        if assign_modality(st.get("session_count", 0), st) != cur:
            break
    save_state(root, func, st)
    return assign_modality(st.get("session_count", 0), st)


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
            "judge_constraints": [], "banned_constructs": [], "ladder_skip": 0,
            "pending_fixup": None, "origin": origin, "created": _now(),
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


def validate_outcome(o, modality, root, func=None):
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
        if "OWNER-ESCALATION" not in txt and "CANONICAL-ASM GRANT PATH" not in txt:
            return False, ("owner-gated: no OWNER-ESCALATION / CANONICAL-ASM GRANT "
                           "PATH entry found in docs/grind/decisions.md")
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
        # LAYER-1 GATE (2026-08-07). `func` is optional for back-compat with
        # callers that don't know it; when the driver passes it, a candidate
        # without a conforming self-vet — or one that re-declares a banned
        # construct — is an INVALID SESSION, not a Judge problem.
        if func:
            ok, why = validate_self_vet(root, func)
            if not ok:
                return False, why
            ok, why = check_banned_constructs(root, func)
            if not ok:
                return False, why
        return True, ""
    # res == progress
    if modality == "annotation-fix":
        # A fix-up session's job is to re-submit the SAME form with a corrected
        # annotation, so it has no new measurement to make. It must still say in
        # writing what it did (or why the fix-up was not possible).
        if not o.get("evidence"):
            return False, ("annotation-fix must bank evidence saying what the "
                           "annotation now reads, or why the fix-up failed")
        return True, ""
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
    if modality in ("permuter", "forensics", "solver"):
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
#
# Flat at ZERO also escalates (2026-08-11): `main` sat at sandbox floor 0 with a
# scorer-invisible 2-byte reorg branch-target residual (full-build SHA1 off, so
# candidate-ready was unreachable) and the old `floor > 0` precondition kept the
# trigger dark — 22 stale worker dispatches looped with no path to a disposition.
# A genuinely-matching function completes and leaves the queue within a session
# or two of hitting 0; anything still being dispatched after a flat-0 window this
# long is stuck (byte residual, judge freeze, respell deadlock) and must reach a
# disposition the same way a flat-positive floor does.
ESCALATION_FLAT_SESSIONS = 8
ESCALATION_MIN_MODALITIES = 4


def _exhaustion_ready(state):
    """True when the honest floor is stuck — flat across a long, modality-diverse
    run — the signal that the ladder is exhausted and the function should be
    dispositioned, not ground further. Flat at 0 counts: a function still being
    dispatched many sessions after reaching floor 0 is blocked from completion
    by something the scorer can't see (byte residual, freeze), not matching."""
    if not state:
        return False
    # Owner continue-directive (2026-08-18): an explicit
    # `owner_continue_until_session: N` in state.json suppresses the exhaustion
    # trigger until session N — used when the owner reverses a park with a
    # continue directive (e.g. func_80017848: mechanism fully named, one
    # spelling short; re-parking on the old flat history would waste the arc).
    cont = state.get("owner_continue_until_session")
    if isinstance(cont, int) and int(state.get("session_count", 0)) < cont:
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
    if any(f != top for f in floors):      # floor still moving → keep grinding
        return False
    mods = {e.get("modality") for e in window}
    return len(mods) >= ESCALATION_MIN_MODALITIES


def assign_modality(session_count, state=None):
    """Modality for the NEXT session. Session 1 = recon; then walk LADDER (one
    cycle — R1 escalation fires on a flat cycle, not a repeat) —
    UNLESS the function is exhaustion-ready (flat floor across many modalities), in
    which case force `escalation` so the run reaches a disposition instead of
    looping. `state` is optional for back-compat; without it the trigger never fires.

    Two overrides sit ABOVE the ladder (2026-08-07 review-audit fix #3):
      * a pending ANNOTATION fix-up short-circuits to the `annotation-fix`
        modality — a FAIL on comment format is a one-comment job, not a re-grind;
      * `ladder_skip` (bumped by advance_modality after a construct-class FAIL)
        rotates the ladder so the next session cannot repeat the modality that
        produced the FAIL. That is what makes a no-respelling constraint bite:
        the session is moved to a different ATTACK, not just told to stop."""
    st = state or {}
    if isinstance(st, dict) and (st.get("pending_fixup") or {}):
        return "annotation-fix"
    if session_count == 0:
        return "recon"
    if _exhaustion_ready(state):
        return "escalation"
    skip = int(st.get("ladder_skip") or 0) if isinstance(st, dict) else 0
    mod = LADDER[(session_count - 1 + skip) % len(LADDER)]
    # Permuter gating (process improvement #5, 2026-08-18): never mandate a
    # SECOND permuter session after a zero-yield one — the remaining population
    # is measured not-permuter-closable (permuter-closability-evaluated;
    # 58k-iteration zero-yield exhibit s5 func_80017848). The FIRST permuter
    # session per function still runs.
    if mod == "permuter" and isinstance(st, dict):
        fh = st.get("floor_history") or []
        # R3 hard cap (modality-effectiveness 2026-08-19): at most 2 permuter
        # sessions per function, EVER — 3rd-or-later permuter measured 0 drops
        # in 64 sessions. Applies regardless of yield (the zero-yield gate
        # below already handles the repeat-after-failure case).
        n_perm = sum(1 for e in fh if e.get("modality") == "permuter")
        blocked = n_perm >= 2
        if not blocked:
            for i, e in enumerate(fh):
                if e.get("modality") == "permuter":
                    prev = fh[i - 1].get("floor") if i else None
                    cur = e.get("floor")
                    if isinstance(prev, int) and isinstance(cur, int) and cur >= prev:
                        blocked = True
                        break
        if blocked:
            # walk past ALL consecutive permuter slots (the ladder holds two)
            step = session_count + skip
            while LADDER[step % len(LADDER)] == "permuter":
                step += 1
            mod = LADDER[step % len(LADDER)]
    return mod


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
    OWNER-ACCEPTED INCOMPLETE). Per the 2026-08-18 ruling (judge-sole-gate,
    b9d91163) a STRONG tier no longer waits on the owner either: it routes to the
    pipeline canonical-asm grant path (function stays ACTIVE; the Judge makes the
    final call on the authored candidate and the driver writes the grant)."""
    st = load_state(root, func) or {}
    hist = st.get("floor_history", [])
    floor = hist[-1].get("floor") if hist else "?"
    sessions = st.get("session_count", len(hist))
    mods = sorted({e.get("modality") for e in hist if e.get("modality")})
    strong = "STRONG" in str(scan_tier).upper()
    if strong:
        ref = f"{date} — {func} — CANONICAL-ASM GRANT PATH (auto-filed by driver, exhaustion backstop; STRONG scan tier — owner ruling 2026-08-18, no owner wait)"
        tail = f"""Gate 1 PASSES: `scan_hand_coded --single {func}` = **{scan_tier}**. Per the owner's
2026-08-18 ruling (.claude/rules/judge-sole-gate.md, commit b9d91163) canonical-asm
authorization is pipeline-executed — nothing waits on the owner. The function STAYS ACTIVE:
the next session authors the whole-body canonical form per
.claude/rules/canonical-asm-authorization-recipe.md and proves bytes on main; the Judge makes
the final call; on its verdict the driver writes the inline_asm_canonical.txt grant and logs
it to docs/grind/borderline.md for later owner audit."""
    else:
        ref = f"{date} — {func} — OWNER-ESCALATION — ESCALATED WITH DECISION PACKET (endgame lock, both gates fail; auto-filed by driver, exhaustion backstop)"
        tail = f"""Both AND-gates fail on the ledger evidence: canonical-asm — `scan_hand_coded --single
{func}` = **{scan_tier}** (ordinary GCC RA/scheduler artifact, no hand-coded signature);
coercion family — no SOTN-master precedent recorded for the residual axes. Per the owner's
2026-08-24 ruling (.claude/rules/escalation-not-parked.md): the item is ESCALATED with a
decision packet, not parked — the packet must state the DECIDABLE question this function's
residual poses (the specific grant/family/fidelity/routing choice), the evidence pointers,
and the consequence of each answer. The two AND-gates remain the unchanged STANDARD; the
owner rules on packets in batches, and the ruling returns the item to active either way.
If no decidable question exists, the item stays ACTIVE with a modality change instead
(difficult-is-not-impossible) — "this is hard" is not a packet."""
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


def log_borderline(root, func, category, evidence, disposition, date):
    """Append an entry to docs/grind/borderline.md (owner ruling 2026-08-18,
    judge-sole-gate). Entries are informational — nothing pending, nothing
    authorized by the entry itself. LF-enforced (the driver must never append
    to pipeline-adjacent files from PowerShell — CRLF)."""
    entry = f"""
## {date} — {func} — {category}
category: {category}
evidence: {evidence}
disposition taken: {disposition}
"""
    with open(os.path.join(root, "docs", "grind", "borderline.md"), "a",
              encoding="utf-8", newline="\n") as f:
        f.write(entry)


def grant_canonical_asm(root, func, tier, date):
    """Execute a pipeline canonical-asm grant (owner ruling 2026-08-18,
    judge-sole-gate, b9d91163): append the inline_asm_canonical.txt entry and
    the borderline-ledger record. The driver calls this on a Judge ESCALATE
    with escalate_kind=canonical-asm-grant AFTER independently re-running
    scan_hand_coded and confirming the STRONG tier — the grant is mechanical
    and evidence-bound, never judgment-bound. Refuses (returns None) if the
    tier is not STRONG-class or the function is already listed."""
    if "STRONG" not in str(tier).upper():
        return None
    allow = os.path.join(root, "inline_asm_canonical.txt")
    with open(allow, encoding="utf-8") as f:
        existing = {ln.strip().split()[0] for ln in f
                    if ln.strip() and not ln.strip().startswith("#")}
    line = (f"{func}  # pipeline grant {date}: scan_hand_coded tier={tier} "
            f"(STRONG class, driver-verified), judge ESCALATE canonical-asm-grant — "
            f"owner ruling 2026-08-18 (.claude/rules/judge-sole-gate.md, b9d91163). "
            f"Packet in docs/grind/decisions.md {date} entry.")
    if func not in existing:
        with open(allow, "a", encoding="utf-8", newline="\n") as f:
            f.write(line + "\n")
    log_borderline(
        root, func, "canonical-asm-grant",
        f"scan_hand_coded --single {func} tier={tier} (driver-verified); "
        f"judge ESCALATE packet in docs/grind/decisions.md ({date})",
        "inline_asm_canonical.txt entry written by the driver per owner ruling "
        "2026-08-18; function stays ACTIVE for canonical-asm integration.",
        date)
    return line


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
    "solver": ("SOLVER modality — tools/ra_solver + tools/sched_solver inverse search "
               "(validated: global 10/10, reload 194/194, sched 6978/6978 blocks). "
               "Operational rules, in order: (1) run `python3 tools/ra_solver/inverse_compose.py "
               "classify <stem> <func>` FIRST — it triages the residual PRE-RA / RA / SCHED / "
               "IDENTICAL and stops you searching the wrong layer. (2) If the ledger banks a "
               "candidate that beats main, RE-DERIVE the model from the CANDIDATE body applied "
               "to src, never from stripped main (func_80072CD4 baseline-routing defect). "
               "(3) State the FULL target register disposition as the goal — a subset goal "
               "voids the vectors (func_80041188 s2 lesson). (4) Scheduler searches: "
               "`perturb.py --atoms luid,luid_move` first, and pin the target with "
               "`--target <stem>.tgt.head.s` (regfix indexes against HEAD). (5) A typed "
               "UNREACHABLE/FORECLOSED verdict is PROGRESS — bank the kill with the vector "
               "space and depth; it closes an axis mechanically. (6) inverse.py lacks "
               "atom-masking; a working reference fork is tmp/grind/func_80041188/s3/"
               "masked_inverse.py if dead atoms pollute the search. Solver verdicts are "
               "HYPOTHESES until a spelled C form measures — a foreclosure was once "
               "retracted by hand-restructuring (func_8001B748 s2); spell and measure the "
               "top vectors before crediting a negative. Save reports under "
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
    "annotation-fix": ("ANNOTATION FIX-UP — TINY SCOPE. The Judge FAILed the previous "
                       "candidate on ANNOTATION FORMAT ONLY: the work itself was accepted, "
                       "and the sole defect is the /* FAKE: ... */ comment's presence or "
                       "wording. Your ENTIRE job this session: restore "
                       "memory/grind/<func>/candidate.c into src/, fix the annotation so it "
                       "reads /* FAKE: <what>, mechanism: <named GCC pass>, lever-exhaustion: "
                       "<where> */ per the Judge's stated defect (quoted in the fix-up notice "
                       "below), re-run `sandbox <func> --disable all` to confirm the floor is "
                       "unchanged, refresh self_vet.md's ANNOTATION-CONFORMANCE line, and "
                       "return candidate-ready. NO NEW CONSTRUCTS ARE PERMITTED THIS SESSION — "
                       "not a rename, not a reordering, not a 'while I'm here' improvement. A "
                       "diff that changes anything but comments (and the self-vet) is a scope "
                       "violation and the session is discarded. If the annotation genuinely "
                       "cannot be written truthfully — because the exhaustion or the mechanism "
                       "the template demands does not exist — that is not a comment problem: "
                       "say so in `evidence` and return progress, and the ladder resumes."),
    "escalation": ("DISPOSITION SESSION — the honest floor has been FLAT across many "
                   "sessions and >=4 distinct modalities, so the driver has determined the "
                   "pure-C levers are exhausted. (A floor FLAT AT 0 lands here too: it means "
                   "sandbox 0 but completion is blocked by something the scorer can't see — "
                   "a full-build byte residual, a judge freeze, a respell deadlock; the "
                   "ledger's judge_constraints say which.) Your job THIS session is to REACH "
                   "A DISPOSITION, not to grind another variant. Valid outcomes: (1) if you "
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
                   "parks terminally, no owner wait. If gate (a) PASSES (STRONG scan tier), "
                   "append a `## <date> — <func> — CANONICAL-ASM GRANT PATH` entry with the "
                   "scanner evidence and return owner-gated citing it — per the owner's "
                   "2026-08-18 ruling (judge-sole-gate) the function STAYS ACTIVE and the "
                   "next session authors the whole-body canonical form; no owner wait. If "
                   "only gate (b) passes (an actually-exhibited SOTN precedent), file the "
                   "standing-ruling entry AND include the precedent citation — the driver "
                   "borderline-logs it for owner batch review; the frozen list is owner-only "
                   "to extend and the disposition is still the terminal refusal. NEVER file "
                   "an 'awaiting owner ruling' entry — that shape is retired. A flat-floor "
                   "`progress` is NOT an acceptable outcome this session — the driver will "
                   "auto-file the disposition if you dodge."),
}


def psyq_identity(root, func):
    """Sony-library provenance for `func`, as a brief section (or '' if none).

    The 2026-07-09 closer census proved 11.3% of the EXE is bit-verbatim Sony
    PsyQ library .text, and named 92 queue items. Phase 3 was retired
    2026-07-13 on the premise that "the Grinder inherits every remaining item
    by construction" — true of the WORK ITEMS (they are in the queue) but false
    of the KNOWLEDGE: nothing carried the identity across, so sessions
    re-derived library routines blind (marionation_Exec = CD_ready ground 57
    sessions; saEft01Init = CD_datasync ground 6). This function is that
    missing link. Never raises — a missing/!malformed closer artifact must
    degrade to a normal brief, never break the pipeline.
    """
    proven, probable, refs = None, None, []
    try:
        p = os.path.join(root, "memory", "closer", "psyq-queue-hits.json")
        if os.path.isfile(p):
            with open(p, encoding="utf-8") as fh:
                for h in json.load(fh).get("queue_hits", []):
                    if h.get("func") == func:
                        proven = h
                        break
    except Exception:
        pass
    try:
        p = os.path.join(root, "memory", "closer", "libsnd-hunt-report.md")
        if os.path.isfile(p):
            with open(p, encoding="utf-8", errors="replace") as fh:
                m = re.search(r"\*\*\s*" + re.escape(func) + r"\s*=\s*([A-Za-z_]\w*)",
                              fh.read())
            if m:
                probable = m.group(1)
    except Exception:
        pass
    if not proven and not probable:
        return ""
    try:
        cdir = os.path.join(root, "memory", "closer", "candidates")
        for fn in sorted(os.listdir(cdir)) if os.path.isdir(cdir) else []:
            hit = func.lower() in fn.lower()
            if not hit:
                try:
                    with open(os.path.join(cdir, fn), encoding="utf-8",
                              errors="replace") as fh:
                        hit = func in fh.read()
                except Exception:
                    hit = False
            if hit:
                refs.append("memory/closer/candidates/" + fn)
    except Exception:
        pass

    out = ["## SONY LIBRARY PROVENANCE — READ THIS FIRST",
           "",
           "This function's queue name is an auto-generated MISNOMER. It is not game code."]
    if proven:
        out += ["",
                f"**{func} = `{proven.get('sony')}` — verbatim-linked Sony PsyQ 4.0 "
                f"library code** ({proven.get('lib')}/{proven.get('mod')} module, "
                f"@{proven.get('addr')}).",
                "",
                "Bit-exact provenance: the 2026-07-09 census verified 100% of "
                "non-reloc-masked bits across the whole module .text.",
                "",
                "PUBLISHED REFERENCE C IS A LEAD, NOT THE ANSWER. Measure it, never assume "
                "it. BB2 links a DIFFERENT BUILD of this library family than the projects "
                "that published matched C, so the same routine can compile to different "
                "bytes here. Measured 2026-08-01: for saEft01Init (CD_datasync) the verbatim "
                "SOTN reference scored 35 against a ground-up floor of 7, and the Tomba and "
                "Xeno spellings also lost. Adoption is still the fast path when it works "
                "(65 functions closed that way) — it is just not guaranteed. Score the "
                "reference, bank it to rejected/ WITH ITS NUMBER if it loses, and keep the "
                "lower floor. A losing reference means 'different build', NOT 'wrong source'.",
                "",
                "Reference sources to try, in order:",
                "  1. sotn-decomp psxsdk tree (matched C, same library family + GCC 2.7.2 era)",
                "  2. sozud/psy-q-decomp",
                "  3. the ground-truth object itself — you have the original bytes AND the "
                "Sony symbol names, which is far easier than blind decomp",
                "",
                "Adapt symbol names to this repo's externs; keep Sony's own struct and "
                "volatile declarations (original semantics, NOT coercions — a volatile that "
                "the reference source has is legitimate, not a cheat).",
                "",
                "Provenance comment required on the adopted body:",
                f"  /* PsyQ 4.0 {proven.get('lib')} {proven.get('mod')}: {proven.get('sony')} "
                "— verbatim-linked Sony object (census 2026-07-09); C ref: <source> */"]
    if probable:
        out += ["",
                f"**{func} is PROBABLE Sony LIBSND/LIBSPU code: `{probable}`** — partial "
                "verbatim match only. BB2 links an interim 4.0-lineage build (compiled "
                "between 1997-06-06 and ship) that is in no public SDK dump, so there is NO "
                "exact reference C. Do NOT adopt on faith. Use the identity to guide "
                "informed decomp: you know what the routine DOES and what Sony called it. "
                "See memory/closer/libsnd-hunt-report.md."]
    if refs:
        out += ["", "Banked reference material from the retired closer campaign "
                    "(read before probing — this work is already done):"]
        out += [f"  - {r}" for r in refs]
    out += ["",
            "Do NOT rename the symbol (queue keys, regfix anchors and this ledger all "
            "reference the current name). The completion bar is unchanged: pure C, zero "
            "rules, byte-identical.", ""]
    return "\n".join(out)


def knowledge_sweep(root, func, limit=40, names=None):
    """file:line hits for `func` across durable knowledge surfaces OUTSIDE the
    function's own ledger (process improvement #3, 2026-08-18 — the libcd-twins
    failure class: memory/closer/ solved CD_ready while its grind ledger never
    heard of it). Read-only; any failure degrades to ''."""
    import glob as _glob
    surfaces = (_glob.glob(os.path.join(root, "memory", "**", "*.md"), recursive=True)
                + [os.path.join(root, "docs", "grind", "decisions.md"),
                   os.path.join(root, "docs", "grind", "borderline.md")]
                + _glob.glob(os.path.join(root, ".claude", "rules", "*.md")))
    own = os.path.join("memory", "grind", func) + os.sep
    hits = []
    pat = re.compile(r"\b(?:" + "|".join(re.escape(n) for n in (names or [func])) + r")\b")
    try:
        for p in surfaces:
            if own in p or not os.path.isfile(p):
                continue
            try:
                with open(p, encoding="utf-8", errors="replace") as fh:
                    for i, ln in enumerate(fh, 1):
                        if pat.search(ln):
                            rel = os.path.relpath(p, root)
                            hits.append(f"  {rel}:{i}: {ln.strip()[:160]}")
                            if len(hits) >= limit:
                                raise StopIteration
            except OSError:
                continue
    except StopIteration:
        pass
    except Exception:
        return ""
    if not hits:
        return ""
    return ("\n## CROSS-KNOWLEDGE HITS (auto-swept — READ the cited files before probing)\n"
            "Knowledge about this function exists OUTSIDE your ledger. Prior campaigns\n"
            "have solved functions whose grind ledgers never heard about it.\n"
            + "\n".join(hits) + "\n")


def build_brief(root, func, modality, outcome_path, head_floor=""):
    st = load_state(root, func)
    d = ledger_dir(root, func)
    rejected = sorted(os.listdir(os.path.join(d, "rejected"))) if os.path.isdir(
        os.path.join(d, "rejected")) else []
    floors = "\n".join(f"  s{e['session']:>2} [{e['modality']}] floor={e['floor']}  {e['headline']}"
                       for e in st["floor_history"][-8:]) or "  (none yet)"
    if len(st["floor_history"]) > 8:
        floors = (f"  (earlier sessions s1..s{st['floor_history'][-8]['session'] - 1} "
                  "are in the ledger files below)\n") + floors
    frontier = "\n".join(f"  - {f['hypothesis']}\n    mechanism: {f['mechanism']}\n"
                         f"    next probe: {f['next_probe']}"
                         for f in st["frontier"]) or "  (empty — build one)"
    constraints = "\n".join(f"  - {c}" for c in st["judge_constraints"]) or "  (none)"
    # Banned constructs get their own loud block ABOVE the ledger state: the audit
    # found 59% of FAILs sitting in respelling loops, where the constraint existed
    # but read as advice buried in a list. This one is mechanically enforced.
    banned_list = st.get("banned_constructs") or []
    if banned_list:
        banned = ("\n## BANNED CONSTRUCTS — MECHANICALLY ENFORCED, NOT ADVICE\n"
                  "The Judge has already ruled each of these a cheat FOR THIS FUNCTION. The\n"
                  "driver REJECTS a candidate-ready whose self_vet.md re-declares one, under any\n"
                  "spelling, before the Judge ever sees it — the session is discarded as invalid\n"
                  "and your work is lost. Respelling a banned construct is the same construct.\n"
                  "Change the ATTACK. If you believe a ban is wrong, emit `ruling-request`.\n"
                  + "\n".join(f"  - BANNED: {b}" for b in banned_list) + "\n")
    else:
        banned = ""
    fx = st.get("pending_fixup") or {}
    if fx:
        fixup = ("\n## FIX-UP NOTICE — the Judge's stated defect (this is your whole task)\n"
                 f"  kind: {fx.get('kind')}\n  defect: {fx.get('detail')}\n")
    else:
        fixup = ""
    # Per-item owner directive (escalation-not-parked rollout 2026-08-24): a
    # queue item returned to active by an owner ruling may carry a DIRECTIVE
    # in unpark_reason (e.g. "or-tree carve-out re-test", "chain-extender
    # first", "provenance before pure-C"). Sessions previously never saw it —
    # 30 directives were invisible. Surface it ABOVE the ledger state.
    directive = ""
    try:
        with open(os.path.join(root, "engine", "queue.json"), encoding="utf-8") as fh:
            _qi = next((i for i in json.load(fh).get("items", [])
                        if i.get("func") == func), None)
        if _qi and _qi.get("unpark_reason"):
            directive = ("\n## OWNER DIRECTIVE FOR THIS FUNCTION (from the queue item)\n"
                         f"  {_qi['unpark_reason']}\n"
                         "  Follow it BEFORE default modality work unless the ledger shows it\n"
                         "  already executed and measured.\n")
    except (OSError, ValueError, StopIteration):
        pass
    # Sony-library provenance goes ABOVE the modality playbook: if the target is
    # library code with published reference C, that changes what the session
    # should DO, so it must be read before the playbook frames the work.
    # Full-picture hook (owner directive 2026-08-24, full-picture-first): the
    # dossier auditor supplies name aliases (rulings often live under OLD
    # names) and cross-surface consistency warnings. Degrades silently.
    _names, _warns = [func], []
    try:
        _cwd = os.getcwd()
        os.chdir(root)
        try:
            sys.path.insert(0, root)
            from engine import dossier as _dos
            _names = _dos.aliases(func)
            _warns = _dos.audit(func)
        finally:
            os.chdir(_cwd)
    except Exception:
        pass
    consistency = ""
    if _warns:
        consistency = ("\n## CONSISTENCY WARNINGS (auto-audit — account for these BEFORE probing)\n"
                       + "\n".join("  - " + w for w in _warns) + "\n")
    if len(_names) > 1:
        consistency = ("\n## NAME ALIASES: " + ", ".join(_names) + " — grep ALL of these when "
                       "searching decisions.md / ledgers / rules.\n") + consistency
    psyq = psyq_identity(root, func)
    ksweep = knowledge_sweep(root, func, names=_names)
    last_floor = next((e.get("floor") for e in reversed(st["floor_history"])
                       if isinstance(e.get("floor"), int)), None)
    chassis = (f"\n## CHASSIS CHECK (driver-measured at dispatch — trust THIS number)\n"
               f"HEAD honest floor right now: {head_floor or 'measurement unavailable'}\n"
               f"Ledger's last recorded floor: {last_floor if last_floor is not None else '(none)'}\n"
               f"If these differ, the chassis has changed since the ledger entry: every banked\n"
               f"spelling conclusion is chassis-relative and MUST be re-measured before it is\n"
               f"spent. Do not quote the ledger floor to the Judge; quote this one.\n")
    # asm-until-matched (owner ruling 2026-08-19): a target still carrying
    # regfix/asmfix rules is one of the 68 byte-coupling deferred functions —
    # its committed C body predates the migration and was CALIBRATED TO ITS
    # RULES, not to honesty. Warn the session before it inherits that shape
    # (CD_datasync burned s1-s6 on rule-era constructs the reference later
    # indicted).
    nrules = 0
    for _rf in ("regfix.txt", "regfix_stage2.txt", "asmfix.txt"):
        _p = os.path.join(root, _rf)
        if os.path.isfile(_p):
            with open(_p, encoding="utf-8", errors="replace") as _f:
                nrules += sum(1 for _ln in _f
                              if re.match(r"^" + re.escape(func) + r"\s*:", _ln.strip()))
    if nrules:
        chassis += (
            f"\n## RULE-ERA CHASSIS WARNING (asm-until-matched deferred function)\n"
            f"This function still carries {nrules} regfix/asmfix rule(s) — it is one of the\n"
            f"68 byte-coupling deferred functions (its body emits jtbl/rodata or is\n"
            f"position-coupled, so it could not be converted to INCLUDE_ASM). The committed\n"
            f"C body's SHAPE was calibrated to those rules, not to honesty: named\n"
            f"intermediates, split statements, and declaration order in it may be rule-era\n"
            f"inventions the original never had. Weigh the ledger candidate and a fresh\n"
            f"derivation (m2c + target asm + sibling idioms) over the committed shape; do\n"
            f"not treat the committed body as evidence of original structure.\n")
    return f"""# GRIND SESSION — {func} (src/{st['file']}.c)

You are session {st['session_count'] + 1} of a cumulative grind. Your mandated
modality for THIS session is: **{modality}**

{psyq}
{ksweep}{chassis}
{MODALITY_PLAYBOOK[modality]}
{fixup}{directive}{consistency}{banned}
## Ledger state (your inheritance — do not re-derive any of it)
Floor history:
{floors}

Live frontier:
{frontier}

Judge constraints (BINDING — forms/techniques already ruled out):
{constraints}

Rejected forms bank (do NOT re-propose; full list in memory/grind/{func}/rejected/): {(f"{len(rejected)} total, newest: " + ', '.join(rejected[-12:])) if len(rejected) > 12 else (', '.join(rejected) or '(empty)')}

READ before working: memory/grind/{func}/evidence.md, memory/grind/{func}/hypotheses.md,
memory/grind/{func}/candidate.c (apply it to src/{st['file']}.c as your starting point).

## Your contract
- Work ONLY {func} in src/{st['file']}.c. Engine commands: `& tools/wteng.ps1 main sandbox {func} --disable all` (your gradient), canonical, diagnose. NEVER edit regfix.txt/asmfix.txt/.claude/rules/engine/tools/Makefile/*.ld; NEVER run queue done/retire; NEVER commit.
- Save your best form to memory/grind/{func}/candidate.c before finishing (even if it did not improve the floor). Save disproven forms to memory/grind/{func}/rejected/<slug>.c.
- Scratch space: tmp/grind/{func}/s{st['session_count'] + 1}/ — put permuter logs / cc1 dumps there and list them in artifacts.
- PASS ATTRIBUTION: before hypothesizing WHICH GCC pass produced a divergence, run
  `pwsh tools/grinder/dump.ps1 {func}` and READ the relevant dump in tmp/grind/{func}/dumps/
  (.combine for fold/copy survival, .lreg/.greg for allocation, .sched for ordering, .loop
  for LICM). Guessing pass attribution across sessions is the failure mode this exists to
  kill (s6-s8 of func_80017848 mis-attributed one copy for three sessions; the dump names
  the pass in one read). The instrumented cc1 is tools/gcc-2.7.2/cc1.
- TURN BUDGET ~35, self-paced. By turn 30 stop probing and write your outcome. This is not a cap you are punished for approaching — it is how the pipeline is designed to work: the LEDGER is the continuation mechanism, so a hypothesis KILLED with a measurement and banked at turn 30 is a FULL success that the next session inherits for free. A 100-turn session is fighting that design, and it is the expensive failure mode: cost grows with the SQUARE of turns (every turn re-reads everything before it), and if the 90-minute timeout kills you mid-probe you write no outcome, the driver discards the session as INVALID, and every token you spent is lost. Bank early, return, let the next session pick up the frontier.
- CONTEXT DISCIPLINE — a fat tool result is not paid once, it is re-read on EVERY later turn in this session. So: NEVER Read a whole src/*.c file (src/text1b.c alone is ~125k tokens — half a context window in one call); Grep to locate, then Read with offset/limit around the hit. Never `cat` a cc1 -da dump, permuter log, or asm/funcs/*.s — grep/tail the part you need. Never re-Read a file you already read this session; scroll back. Prefer one targeted Grep over three exploratory Reads.
- NARRATION vs ARTIFACTS. Keep your own prose terse: fragments over sentences, no preamble, no recap of what a tool result already shows, no restating the plan each turn. This does NOT apply to what you WRITE: evidence.md, hypotheses.md, candidate.c header comments, any docs/grind/decisions.md entry, and the outcome JSON stay full, precise, self-contained prose — they are the owner's audit trail and the next session's entire inheritance, and a terse ledger costs far more than it saves by forcing re-derivation.
- When finished, write your outcome JSON (single object) to EXACTLY this path: {outcome_path}
  Schema: {{"result": "progress"|"candidate-ready"|"ruling-request"|"owner-gated", "floor": <int>,
  "headline": "<one line>", "hypotheses": [{{"statement","mechanism","probe","result","verdict":"CONFIRMED"|"KILLED"}}],
  "evidence": ["fact ..."], "frontier": [<=3 of {{"hypothesis","mechanism","next_probe"}}],
  "artifacts": ["tmp/grind/..."], "ruling_question": "", "escalation_ref": ""}}
- "candidate-ready" means: sandbox distance 0 THIS session, edits in place in src/. The driver re-verifies bytes itself — never claim it speculatively.
- SELF-VET IS MANDATORY FOR candidate-ready. Before you write the outcome JSON, write memory/grind/{func}/self_vet.md using the template in your role prompt: a CONSTRUCTS: line, the six cheat-checklist tests answered IN WRITING for every construct in your diff, a SANCTIONED-FAMILY-CLAIMS: section (each claimed family carrying its rule's SCOPE sentence quoted VERBATIM plus a PRECEDENT as file:line or a commit hash), and an ANNOTATION-CONFORMANCE: line. The driver checks all of that mechanically and DISCARDS a candidate-ready session that lacks it — the same disposition as a scope violation. Then a fresh adversarial cheat-reviewer (layer 1) rules on your diff BEFORE the Judge is spawned; a layer-1 FAIL bounces straight back without a Judge cycle. Writing the vet honestly is how you pass both: if you cannot quote a scope sentence and cite a precedent for a family you are claiming, you do not have that family, and the correct outcome is `ruling-request`, not a submission.
- "ruling-request" is for a construct you cannot classify (sanctioned SOTN family vs cheat; genuine hand-written-asm evidence). Ask a precise question.
- "owner-gated" is for when every remaining sanctioned axis is measured dead. FILE the entry in docs/grind/decisions.md YOURSELF THIS session (docs/grind/ is in your allowed surface), THEN return owner-gated with escalation_ref citing it — you do not wait for an entry to pre-exist, you create it. The driver verifies the entry names {func} and ESCALATES the function (decision packet awaiting an owner ruling — .claude/rules/escalation-not-parked.md, 2026-08-24) so the queue advances. Never use it to defer work that is still grindable (a floor still dropping is grindable). This is the mandated outcome in `escalation` modality.
- OWNER'S STANDING AUTO-RULING (2026-07-27) — this governs HOW you word an escalation, and it NEVER authorizes ending a function early. Two separate questions, do not conflate them:
  (A) IS THE FUNCTION EXHAUSTED? This is the DRIVER's call, not yours. The driver assigns `escalation` modality only after the honest floor has been FLAT across many sessions AND >=4 DISTINCT modalities. If your mandated modality is NOT `escalation`, the answer is NO — you may not dispose of the function, however dead your own axis looks. A killed axis is a `progress` outcome with the kills banked; the ladder still has untried modalities (forensics / rederive / synthesis) and the owner's standing directive is to work the top item to completion however many sessions it takes ([[no-deferral-work-to-completion]], [[difficult-is-not-impossible]]). Judge FAILs do NOT make a function exhausted — a FAILed construct is one dead lever, and a FAIL on annotation FORMAT is a one-comment fix, not a wall.
  (B) ONCE THE DRIVER HAS DECLARED EXHAUSTION (you are in `escalation` modality), evaluate the two endgame-lock AND-gates: (1) canonical-asm needs STRONG `scan_hand_coded` signals (S1/S2/S6); (2) a coercion/spelling family needs an in-hand SOTN-master precedent you can CITE (file+line or commit). "Same spirit", "genre-adjacent", "only lever left", "measured to work", and a partition/elimination argument do NOT qualify — and a census you ran that came back NEGATIVE is a FAILED gate, not an open question. Whatever the gate outcome, your entry is a DECISION PACKET (owner ruling 2026-08-24, .claude/rules/escalation-not-parked.md): title it `## <date> — {func} — **OWNER-ESCALATION — ESCALATED WITH DECISION PACKET**` and state (i) the single DECIDABLE question this residual poses (the specific grant / family / fidelity / routing choice — with the gate evidence), (ii) evidence POINTERS (ledger lines, measurements, scan output), (iii) the concrete consequence of each answer (what closes at what floor / what resumes). The driver escalates the item; the owner rules on packets in batches and the item returns to active either way — nothing is terminal. "This is hard" is NOT a packet: if no decidable question exists, the honest outcome is `progress` with the kills banked, and the item stays active for the next modality. AUTO-REJECT CLASS (owner ruling 2026-08-24, second): a packet whose YES would LOWER a standard — permanent-rule sanction, a no-precedent family grant, a canonical evidence-bar override, any "accept the debt" wording — is PRE-DECIDED NO and must NOT be filed; that residual stays ACTIVE under standing policy. File packets only for fidelity/routing/provenance questions or genuinely gate-PASSING evidence.
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
    #   grindlib.py validate <root> <outcome_json_path> <modality> [func] -> exit 0 ok / 1 invalid (prints reason)
    #   grindlib.py selfvet <root> <func>                           -> exit 0 ok / 1 invalid (prints reason)
    #   grindlib.py ban <root> <func> <construct>
    #   grindlib.py advance-modality <root> <func>                  -> prints the new modality
    #   grindlib.py fixup <root> <func> <kind> <detail>
    #   grindlib.py clear-fixup <root> <func>
    #   grindlib.py apply <root> <func> <outcome_json_path> <modality>
    #   grindlib.py init <root> <func> <file_stem> [origin]
    #   grindlib.py convert-wip <root> <func> <file_stem>
    #   grindlib.py modality <root> <func>                          -> prints next modality
    #   grindlib.py constrain <root> <func> <text>
    #   grindlib.py grant-canonical-asm <root> <func> <tier> <date>   -> prints allowlist line / exit 1 refused
    #   grindlib.py log-borderline <root> <func> <category> <evidence> <disposition> <date>
    import sys
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    cmd = sys.argv[1]
    if cmd == "brief":
        hf = sys.argv[6] if len(sys.argv) > 6 else ""
        print(build_brief(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], hf))
    elif cmd == "validate":
        o = json.load(open(sys.argv[3], encoding="utf-8"))
        ok, why = validate_outcome(o, sys.argv[4], sys.argv[2],
                                   sys.argv[5] if len(sys.argv) > 5 else None)
        if not ok:
            print(why)
            sys.exit(1)
    elif cmd == "selfvet":
        ok, why = validate_self_vet(sys.argv[2], sys.argv[3])
        if ok:
            ok, why = check_banned_constructs(sys.argv[2], sys.argv[3])
        if not ok:
            print(why)
            sys.exit(1)
    elif cmd == "ban":
        if not add_banned_construct(sys.argv[2], sys.argv[3], sys.argv[4]):
            print("ban REFUSED (template collision — reviewer prose, not a C "
                  "construct; finding stays in judge_constraints only)")
    elif cmd == "advance-modality":
        print(advance_modality(sys.argv[2], sys.argv[3]))
    elif cmd == "fixup":
        set_pending_fixup(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
    elif cmd == "clear-fixup":
        clear_pending_fixup(sys.argv[2], sys.argv[3])
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
    elif cmd == "grant-canonical-asm":
        # grant-canonical-asm <root> <func> <tier> <date>
        line = grant_canonical_asm(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
        if line is None:
            print("grant REFUSED (tier not STRONG-class — evidence gate failed)")
            sys.exit(1)
        print(line)
    elif cmd == "log-borderline":
        # log-borderline <root> <func> <category> <evidence> <disposition> <date>
        log_borderline(sys.argv[2], sys.argv[3], sys.argv[4],
                       sys.argv[5], sys.argv[6], sys.argv[7])
    elif cmd == "unban":
        # unban <root> <func> <needle>  -> prints number of entries removed
        print(unban_construct(sys.argv[2], sys.argv[3], sys.argv[4]))
    elif cmd == "add-scope-allow":
        # add-scope-allow <root> <func> <date> <path> [more paths...]
        line = add_scope_allow(sys.argv[2], sys.argv[3], sys.argv[5:], sys.argv[4])
        if line is None:
            print("scope grant REFUSED (path class or denylist — "
                  "integration-handoff-self-serve)")
            sys.exit(1)
        print(line)
    else:
        print(f"unknown cmd {cmd}")
        sys.exit(2)
