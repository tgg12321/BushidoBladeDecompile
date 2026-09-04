#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# The explicit coding declaration is LOAD-BEARING on Windows python 3.9: its
# CLI tokenizer has a long-line/multibyte buffer bug (spurious "Non-UTF-8 code
# starting with '\xe2'" SyntaxError when an em-dash straddles a tokenizer
# buffer boundary — surfaced 2026-08-31 after a brief-string edit shifted byte
# offsets). The declaration switches decoding to the codecs path, which is
# correct. Do not remove.
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
import hashlib
import json
import os
import re
import sys

MODALITIES = ["recon", "structural", "permuter", "solver", "forensics", "rederive", "synthesis",
              "object-model"]
# "object-model" (2026-09-03, func_80033550 post-mortem): a one-session audit of
# declared shape vs evidence for every global the target touches, forced ONCE
# before the driver may declare exhaustion. 13 of that function's 17 sessions
# modeled register allocation for a tail named_syms.txt had described as a
# 12-byte record table since 2026-05-17; no ladder rung ever asked whether the
# declarations were right. Not a ladder rung: it sits between exhaustion-ready
# and `escalation`, so a flat floor can no longer reach a foreclosure record
# without the object model having been checked on the record.
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


_OBJECT_MODEL_RE = re.compile(r"\bOBJECT MODEL\b", re.I)


def _has_object_model(o):
    """True when the outcome banks an `OBJECT MODEL:` evidence entry."""
    return any(_OBJECT_MODEL_RE.search(str(e)) for e in (o.get("evidence") or []))


def data_model_signals(root, func):
    """(rows, flags) from engine/datamodel.py evaluated with cwd=root.
    Degrades to ([], []) on any failure — never blocks a dispatch."""
    cwd = os.getcwd()
    try:
        os.chdir(root)
        if root not in sys.path:
            sys.path.insert(0, root)
        from engine import datamodel as _dm
        _dm.reset_cache()
        return _dm.data_model(func)
    except Exception:
        return [], []
    finally:
        os.chdir(cwd)


# Per-use puns on a splat symbol's ADDRESS: `(T *)&D_x`, `(&D_x + i)`,
# `(&D_x)[i]`. Each spells an object model at the use site that belongs at
# the declaration (aggregate-merge family prong (d)); both func_80033550
# layer-1 FAILs on 2026-09-03 were exactly these, one session apart.
_SYM = r"(D_[0-9A-Fa-f]{8}|g_\w+)"
_PUN_RES = (re.compile(r"\(\s*(?:struct\s+\w+|\w+)\s*\*+\s*\)\s*\(?\s*&\s*" + _SYM + r"\b"),
            re.compile(r"\(\s*&\s*" + _SYM + r"\s*[+-]"),
            re.compile(r"&\s*" + _SYM + r"\s*\)\s*\["))


def scan_declaration_puns(root, func, path=None):
    """['<line>: <text>  [<sym>]'] for every candidate.c line carrying a per-use
    address pun on a splat symbol. Empty when clean or unreadable."""
    p = path or os.path.join(ledger_dir(root, func), "candidate.c")
    try:
        with open(p, encoding="utf-8", errors="replace") as fh:
            lines = fh.read().split("\n")
    except OSError:
        return []
    hits = []
    for i, ln in enumerate(lines, 1):
        for rx in _PUN_RES:
            m = rx.search(ln)
            if m:
                hits.append(f"{i}: {ln.strip()[:120]}  [{m.group(1)}]")
                break
    return hits


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


def supersede_bans(root, func, needles, superseded_by):
    """Move every banned_constructs entry containing any needle (case-insensitive)
    into state['superseded_bans'] with the grant reference. Returns the list of
    moved entries (each {text, superseded_by, when}) so the caller can report
    exactly which bans it cleared — a bare count hid a needle that matched the
    wrong ban (review of a4d3ba3e).
    The ban text is preserved for the audit trail; only the mechanical tripwire
    (check_banned_constructs) stops seeing it."""
    st = load_state(root, func)
    if not st:
        return []
    keep, moved = [], []
    for b in st.get("banned_constructs", []):
        if any(n.lower() in str(b).lower() for n in needles):
            moved.append({"text": b, "superseded_by": superseded_by, "when": _now()})
        else:
            keep.append(b)
    if moved:
        st["banned_constructs"] = keep
        st.setdefault("superseded_bans", []).extend(moved)
        save_state(root, func, st)
    return moved


def preview_superseded_bans(root, func, needles):
    """Dry-run counterpart of supersede_bans: the ban texts that WOULD move,
    without touching the ledger. The grant-rescan dry run prints these so a
    too-broad term is visible before it clears an unrelated ban."""
    st = load_state(root, func) or {}
    return [str(b) for b in st.get("banned_constructs", [])
            if any(n.lower() in str(b).lower() for n in needles)]


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
            "judge_constraints": [], "banned_constructs": [], "kills": [],
            "ladder_skip": 0,
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
    extra = ""
    if h.get("verdict") == "KILLED":
        extra = (f"- kill_scope: {h.get('kill_scope', 'instance')}\n"
                 f"- measured_on: {h.get('measured_on', '?')}\n")
        if h.get("predicate_cite"):
            extra += f"- predicate_cite: {h['predicate_cite']}\n"
    with open(p, "a", encoding="utf-8", newline="\n") as f:
        f.write(f"\n## [{tag}] {h.get('statement', '?')}\n"
                f"- mechanism: {h.get('mechanism', '?')}\n"
                f"- probe: {h.get('probe', '?')}\n"
                f"- result: {h.get('result', '?')}\n"
                f"- verdict: {h.get('verdict', '?')}\n" + extra)


def _has_measurement(text):
    return any(ch.isdigit() for ch in str(text))


# ── Kill hygiene (2026-09-01 post-mortem) ────────────────────────────────────
# Four of six 20+-session runs were prolonged by a KILLED hypothesis measured
# on one confounded instance (3 of 12 arms with a staged read; a lever under a
# FAKE carrier on the same pseudo; a loop.c predicate missing one term) and
# then cited by later sessions as class-level law. A kill now declares its
# scope: `instance` kills are chassis-relative and re-testable; `class` kills
# must cite the gate predicate by file:line.
#
# The class-claim scan reads the STATEMENT field ONLY (review 2026-09-02):
# replaying it over statement+result across 484 kill-bearing sessions would
# have discarded 17.1% of them, 95 of 110 trips coming from the `result`
# narration, where sweeping words describe a measurement rather than assert a
# law. `foreclosed` and `by construction` are likewise dropped from the
# vocabulary: the solver playbook and the escalation gate MANDATE those words,
# so matching them punished sessions for obeying the driver.
KILL_SCOPES = ("instance", "class")
_CLASS_CLAIM_RE = re.compile(
    r"\b(any (natural )?(geometry|form|spelling|shape)|every (form|spelling|chassis)|"
    r"all (forms|spellings|chassis)|no natural|unreachable|impossible|permanently|"
    r"cannot (ever|be made to))\b", re.I)


# Bare-name cite search path, in order. House convention in the ledgers is a
# BARE filename with no directory (`loop.c:1631`, `mips.c:4680`): 273 of 279
# compiler cites and every project-source cite are spelled that way. The two
# most-cited compiler files live under config/mips (mips.c 66 cites, mips.h
# 47), not at the gcc-2.7.2 root, and project sources (main.c 49, display.c
# 30, code6cac.c 18, text1b.c 15, system.c 7) live under src/.
_CITE_SEARCH_DIRS = (
    ("tools", "gcc-2.7.2"),
    ("tools", "gcc-2.7.2", "config", "mips"),
    ("src",),
    ("include",),
)
_CITE_SEARCH_DIRS_TEXT = ", ".join("/".join(d) + "/" for d in _CITE_SEARCH_DIRS)


def _within_root(path, root):
    """True when path really sits inside root (symlinks and .. resolved)."""
    try:
        rp, rr = os.path.realpath(path), os.path.realpath(root)
        return os.path.commonpath([rp, rr]) == rr
    except (ValueError, OSError):
        return False


def _resolve_cite_path(rel, root):
    """Absolute path for the file part of a cite, or None. A cite with a
    directory component is resolved relative to the repo root. A BARE name
    (no separator) is looked up in each _CITE_SEARCH_DIRS entry in order.
    Cites containing a `..` segment, and anything that resolves outside the
    repo root, are refused outright."""
    parts = [p for p in rel.replace("\\", "/").split("/") if p]
    if not parts or any(p == ".." for p in parts):
        return None
    cands = [os.path.join(root, *parts)]
    if len(parts) == 1:
        cands += [os.path.join(root, *(d + (parts[0],))) for d in _CITE_SEARCH_DIRS]
    for path in cands:
        if os.path.isfile(path) and _within_root(path, root):
            return path
    return None


def _cite_resolves(cite, root):
    """True when a predicate_cite actually points at something: a `path:line`
    whose file exists in the repo (bare names are searched in
    _CITE_SEARCH_DIRS) with at least that many lines, or a hash git knows. An
    unresolvable cite is how a class kill gets asserted without a
    predicate — the exact failure the scope field exists to stop."""
    m = re.search(r"([\w./\\-]+\.\w+):(\d+)", cite)
    if m:
        path = _resolve_cite_path(m.group(1), root)
        if path is None:
            return False
        try:
            with open(path, encoding="utf-8", errors="replace") as f:
                nlines = sum(1 for _ in f)
        except OSError:
            return False
        return 0 < int(m.group(2)) <= nlines
    m = re.search(r"\b([0-9a-f]{7,40})\b", cite)
    if m:
        try:
            import subprocess
            return subprocess.call(["git", "-C", root, "cat-file", "-e",
                                    m.group(1) + "^{object}"],
                                   stdout=subprocess.DEVNULL,
                                   stderr=subprocess.DEVNULL) == 0
        except Exception:
            return False
    return False


def _validate_kill(h, root):
    """(ok, reason) for one KILLED hypothesis dict."""
    scope = str(h.get("kill_scope", "")).strip().lower()
    if scope not in KILL_SCOPES:
        return False, ("KILLED hypothesis lacks kill_scope ('instance' or 'class'): "
                       f"{str(h.get('statement', ''))[:80]!r}")
    if not str(h.get("measured_on", "")).strip():
        return False, ("KILLED hypothesis lacks measured_on (the chassis + FAKE-construct "
                       "state it was measured under, e.g. 'HEAD chassis, L3 carrier present'): "
                       f"{str(h.get('statement', ''))[:80]!r}")
    statement = str(h.get("statement", ""))
    mt = _CLASS_CLAIM_RE.search(statement) if scope == "instance" else None
    if mt:
        return False, (f"KILLED hypothesis statement makes a class-level claim "
                       f"({mt.group(0)!r}) with kill_scope='instance': "
                       f"{statement[:80]!r}. Either narrow the STATEMENT wording to the "
                       "instance you measured (which arms, which chassis, which FAKE "
                       "state) or set kill_scope='class' and cite the gate predicate "
                       "(file:line that exists) in predicate_cite.")
    if scope == "class":
        cite = str(h.get("predicate_cite", "")).strip()
        if not cite or not _CITATION.search(cite):
            return False, ("class-scope KILLED hypothesis requires predicate_cite as file:line "
                           "(e.g. tools/gcc-2.7.2/loop.c:705) naming the gate predicate "
                           "the whole class fails; a search that came back empty is an "
                           "instance kill, not a class kill.")
        if not _cite_resolves(cite, root):
            return False, (f"predicate_cite does not resolve: {cite} (no such file "
                           f"(bare names are searched in {_CITE_SEARCH_DIRS_TEXT}) / "
                           "line beyond EOF / unknown hash / .. or out-of-root path)")
    return True, ""


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
    # Kill hygiene runs BEFORE the per-result branches: owner-gated and
    # ruling-request outcomes reach apply_outcome too, and owner-gated is the
    # sweeping-prose path where an unscoped kill does the most damage.
    for h in o.get("hypotheses", []):
        if h.get("verdict") == "KILLED":
            ok, why = _validate_kill(h, root)
            if not ok:
                return False, why
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
        # Owner ruling 2026-09-02 (ruling 3): both foreclosure titles — the
        # standing-ruling one and the non-endgame LADDER EXHAUSTED one — are
        # driver-declared dispositions; a session that just spent an owner's
        # named probe returns `progress`, never a foreclosure.
        if (("RESOLVED BY STANDING RULING" in ref or "LADDER EXHAUSTED" in ref)
                and modality != "escalation"):
            return False, ("owner-gated: a foreclosure disposition (standing ruling / "
                           f"ladder exhausted) requires `escalation` modality (driver-"
                           f"declared exhaustion), not `{modality}`. A dead axis in this "
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
    if modality == "object-model":
        if not _has_object_model(o):
            return False, ("object-model session must bank an evidence entry beginning "
                           "`OBJECT MODEL:` with per-symbol verdicts and the premise list")
        return True, ""
    if modality == "recon":
        if not o.get("frontier"):
            return False, "recon must produce an initial frontier"
        if not o.get("evidence"):
            return False, "recon must bank evidence"
        if func and not _has_object_model(o):
            _rows, _flags = data_model_signals(root, func)
            if _flags:
                return False, ("recon on a function whose DATA MODEL carries signals "
                               f"({len(_flags)}) must bank an evidence entry beginning "
                               "`OBJECT MODEL:` answering each flagged symbol")
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
# Owner ruling 2026-09-02 (decisions.md "foreclosure mechanics"): the 2026-07-27
# standing ruling is scoped to its actual subject — an RA/scheduler-tiebreak
# residual "a few instructions short". A flat floor ABOVE this is NOT an endgame
# lock: the ladder runs a SECOND full cycle before exhaustion may fire, and the
# record is titled LADDER EXHAUSTED (non-endgame residual), never claiming the
# standing ruling. Six items (floors 8..20) had been foreclosed under the
# standing ruling the moment the single cycle ran out.
ENDGAME_LOCK_MAX_FLOOR = 5
ESCALATION_FLAT_SESSIONS_WIDE = 2 * len(LADDER)
ESCALATION_MIN_MODALITIES_WIDE = 6


def _exhaustion_ready(state):
    """True when the honest floor is stuck — flat across a long, modality-diverse
    run — the signal that the ladder is exhausted and the function should be
    dispositioned, not ground further. Flat at 0 counts: a function still being
    dispatched many sessions after reaching floor 0 is blocked from completion
    by something the scorer can't see (byte residual, freeze), not matching.

    Two scopes (owner ruling 2026-09-02): a flat floor <= ENDGAME_LOCK_MAX_FLOOR
    (the standing ruling's subject) needs ESCALATION_FLAT_SESSIONS across
    ESCALATION_MIN_MODALITIES; anything wider needs a second full ladder cycle
    (ESCALATION_FLAT_SESSIONS_WIDE across ESCALATION_MIN_MODALITIES_WIDE).
    Only history AFTER `exhaustion_base` counts — an unpark stamps it
    (sync_unpark) so an owner ruling buys a fresh window, never one session
    (11/12 of the 2026-09-01 unparks were re-foreclosed after one session)."""
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
    base = state.get("exhaustion_base")
    if isinstance(base, int) and base > 0:
        hist = [e for e in hist if isinstance(e.get("session"), int) and e["session"] > base]
    if len(hist) < ESCALATION_FLAT_SESSIONS:
        return False
    # Floors are normally ints, but WIP-imported ledgers can carry a prose string
    # there — treat any non-int floor as "unknown" and never escalate on it (avoids
    # a str-vs-int comparison crash and a bogus escalation on malformed history).
    top = hist[-1].get("floor")
    if not isinstance(top, int):
        return False
    if top <= ENDGAME_LOCK_MAX_FLOOR:
        need, need_mods = ESCALATION_FLAT_SESSIONS, ESCALATION_MIN_MODALITIES
    else:
        need, need_mods = ESCALATION_FLAT_SESSIONS_WIDE, ESCALATION_MIN_MODALITIES_WIDE
    if len(hist) < need:
        return False
    window = hist[-need:]
    floors = [e.get("floor") for e in window]
    if any(not isinstance(f, int) for f in floors):
        return False
    if any(f != top for f in floors):      # floor still moving → keep grinding
        return False
    mods = {e.get("modality") for e in window}
    return len(mods) >= need_mods


def sync_unpark(root, func):
    """Owner ruling 2026-09-02 (ruling 1): when a queue item has returned to
    active with an `unpark_reason` the ledger has not yet seen, stamp
    `exhaustion_base` = the current session count so the flat-floor window
    restarts. Idempotent per reason (the base must not creep forward on every
    session). Returns True when a stamp was written. Called by the driver at
    dispatch, so every unpark path (owner `queue unpark`, grant_rescan --apply)
    is covered without touching engine code."""
    try:
        with open(os.path.join(root, "engine", "queue.json"), encoding="utf-8") as fh:
            qi = next((i for i in json.load(fh).get("items", [])
                       if i.get("func") == func), None)
    except (OSError, ValueError):
        return False
    reason = (qi or {}).get("unpark_reason") or ""
    if not reason:
        return False
    st = load_state(root, func)
    if not st or st.get("last_unpark_reason") == reason:
        return False
    st["exhaustion_base"] = int(st.get("session_count", 0))
    st["last_unpark_reason"] = reason
    st["last_unpark_at"] = _now()
    # A fresh window also gets its full deferral allowance (Ruling B.1).
    st["escalation_deferrals"] = 0
    st.pop("forced_next_modality", None)
    save_state(root, func, st)
    return True


# ── Escalation deferral (owner ruling 2026-09-04, Ruling B.1) ────────────────
# The exhaustion backstop's only test was "escalation modality AND floor did
# not drop". CD_sync s116 CONFIRMED a lever for the first time in 116 sessions,
# named the new wall, wrote that it declined to self-file because the axis was
# not exhausted — and the driver auto-filed the foreclosure anyway. Under the
# 2026-09-02 ruling "a spent probe is progress, not a disposition"; so is a
# confirmed one. An escalation session that banks >= 1 CONFIRMED hypothesis
# with a numeric measurement is honored as ordinary progress, at most
# ESCALATION_DEFERRALS_MAX times per exhaustion window (reset on unpark and on
# any floor drop), and the next session is FORCED to the first rung in
# _DEFERRAL_MODALITIES that has not run since the window base, so the deferral
# attacks the named wall instead of re-entering escalation. The third such
# session is backstopped exactly as before: the 2026-07-22 loop this backstop
# exists for (func_8007DC9C, 40 flat sessions) is bounded at two deferrals per
# window by construction.
ESCALATION_DEFERRALS_MAX = 2
_DEFERRAL_MODALITIES = ("solver", "permuter", "structural", "forensics")
_HYP_HEADER_RE = re.compile(r"(?m)^## \[s(\d+|\?)\] (.+)$")
_HYP_VERDICT_RE = re.compile(r"(?m)^- verdict: (\w+)")


def _norm_statement(s):
    return re.sub(r"\s+", " ", str(s or "")).strip().lower()[:200]


def _prior_confirmed_statements(root, func, before_session):
    """Normalized statements of every CONFIRMED hypothesis banked in a session
    strictly before `before_session` (hypotheses.md is append-only; each entry
    is `## [sN] <statement>` followed by `- verdict: X`)."""
    p = os.path.join(ledger_dir(root, func), "hypotheses.md")
    try:
        with open(p, encoding="utf-8", errors="replace") as f:
            txt = f.read()
    except OSError:
        return set()
    out = set()
    heads = list(_HYP_HEADER_RE.finditer(txt))
    for i, h in enumerate(heads):
        end = heads[i + 1].start() if i + 1 < len(heads) else len(txt)
        body = txt[h.end():end]
        v = _HYP_VERDICT_RE.search(body)
        if not v or v.group(1) != "CONFIRMED":
            continue
        sess = h.group(1)
        if sess.isdigit() and int(sess) >= before_session:
            continue
        out.add(_norm_statement(h.group(2)))
    return out


def qualifying_confirmed(root, func, o, session):
    """CONFIRMED hypotheses that can buy a deferral (layer-2 review
    2026-09-04: a bare CONFIRMED verdict has no hygiene gate, so a session
    could bank trivial re-confirmations to dodge the backstop). Mirrors the
    kill hygiene mechanically: a numeric measurement in `result`, a
    `measured_on` chassis, and a statement NOT already CONFIRMED in an earlier
    session of this ledger (a re-confirmation is not a new lever)."""
    prior = _prior_confirmed_statements(root, func, session)
    out = []
    for h in (o or {}).get("hypotheses", []):
        if h.get("verdict") != "CONFIRMED":
            continue
        if not _has_measurement(h.get("result", "")):
            continue
        if not str(h.get("measured_on", "")).strip():
            continue
        if _norm_statement(h.get("statement")) in prior:
            continue
        out.append(h)
    return out


def escalation_deferral(root, func, o):
    """Decide whether an escalation-modality session that did NOT drop the
    floor is honored as progress. Returns the forced next modality when it is
    (and stamps `escalation_deferrals` + `forced_next_modality` into the
    ledger), else ''. Call AFTER apply_outcome so the session is in
    floor_history. Requires: >= 1 qualifying CONFIRMED hypothesis (see
    qualifying_confirmed) AND a non-empty frontier — a deferral without a
    named next probe is a dodge, not an opened axis."""
    st = load_state(root, func)
    if not st:
        return ""
    session = int(st.get("session_count", 0))
    if not (o or {}).get("frontier"):
        return ""
    if not qualifying_confirmed(root, func, o, session):
        return ""
    used = int(st.get("escalation_deferrals") or 0)
    if used >= ESCALATION_DEFERRALS_MAX:
        return ""
    hist = st.get("floor_history", [])
    base = st.get("exhaustion_base")
    if isinstance(base, int) and base > 0:
        hist = [e for e in hist if isinstance(e.get("session"), int) and e["session"] > base]
    ran = {e.get("modality") for e in hist}
    pick = next((m for m in _DEFERRAL_MODALITIES if m not in ran), _DEFERRAL_MODALITIES[0])
    st["escalation_deferrals"] = used + 1
    st["forced_next_modality"] = pick
    save_state(root, func, st)
    return pick


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
    # Sibling progress (2026-09-04): a sibling ledger dropped BELOW this floor
    # since the last session -> exactly one forced `rederive` (the transplant
    # session). Sits above exhaustion on purpose: new sibling evidence is the
    # one thing that should pre-empt an escalation. One-shot by construction —
    # apply_outcome consumes the notice, so the override cannot fire twice on
    # the same news.
    if sibling_progress_pending(st):
        return "rederive"
    # Escalation deferral (2026-09-04, Ruling B.1): one-shot forced rung after
    # an honored escalation session. Sits above exhaustion so the deferral
    # attacks the wall; consumed by apply_outcome, so it cannot repeat.
    forced = st.get("forced_next_modality") if isinstance(st, dict) else None
    if forced in LADDER:
        return forced
    if _exhaustion_ready(state):
        # Object-model gate (2026-09-03): exhaustion may not be declared until
        # ONE session has audited declared shape vs evidence on the record.
        # apply_outcome stamps the flag when an `OBJECT MODEL:` evidence entry
        # is banked (any modality), so a recon that did the audit skips this.
        if isinstance(st, dict) and not st.get("object_model_audited"):
            return "object-model"
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
    # Sibling-progress notices are ONE-SHOT: the brief for session n carried
    # every unconsumed entry, so applying session n consumes them all (the
    # forced rederive cannot repeat). A discarded session never reaches here,
    # so it respawns with the same notice — bounded by the circuit-breaker.
    for e in st.get("sibling_progress") or []:
        if e.get("consumed") is None:
            e["consumed"] = n
    prev_floors = [e.get("floor") for e in st["floor_history"] if isinstance(e.get("floor"), int)]
    prev_min = min(prev_floors) if prev_floors else None
    if _has_object_model(o):
        st["object_model_audited"] = n
    for h in o.get("hypotheses", []):
        append_hypothesis(root, func, h, session=n)
    for e in o.get("evidence", []):
        append_evidence(root, func, e, session=n)
    st["session_count"] = n
    kills = st.setdefault("kills", [])
    for h in o.get("hypotheses", []):
        if h.get("verdict") == "KILLED":
            kills.append({"session": n,
                          "statement": str(h.get("statement", ""))[:200],
                          "kill_scope": str(h.get("kill_scope", "instance")),
                          "measured_on": str(h.get("measured_on", ""))[:200],
                          "predicate_cite": str(h.get("predicate_cite", ""))[:120],
                          "result": str(h.get("result", ""))[:120]})
    st["kills"] = kills[-60:]
    st["current_modality"] = modality
    # A forced deferral rung is one-shot: the session that ran it consumes it.
    st.pop("forced_next_modality", None)
    last_floor = next((e.get("floor") for e in reversed(st["floor_history"])
                       if isinstance(e.get("floor"), int)), None)
    if (isinstance(o.get("floor"), int) and isinstance(last_floor, int)
            and o["floor"] < last_floor):
        st["escalation_deferrals"] = 0     # a real drop earns a fresh allowance
    st["floor_history"].append({"session": n, "floor": o.get("floor"),
                                "modality": modality,
                                "headline": (o.get("headline") or "")[:200]})
    if o.get("frontier"):
        st["frontier"] = o["frontier"][:MAX_FRONTIER]
    save_state(root, func, st)
    # A genuine floor DROP (below every earlier int floor) is news to every
    # sibling ledger. A ledger's FIRST floor is not a drop — the SIBLING
    # LEDGERS block already shows new ledgers. Runs AFTER save_state and never
    # raises (see the sibling section's header for why an `apply` that raises
    # is a driver livelock).
    new_floor = o.get("floor")
    if isinstance(new_floor, int) and prev_min is not None and new_floor < prev_min:
        try:
            notify_siblings(root, func, n, new_floor, o.get("headline"))
        except Exception:
            pass
    return st


def add_judge_constraint(root, func, text):
    st = load_state(root, func)
    # Exact-duplicate lines are noise the next session pays to re-read.
    if text and text not in st["judge_constraints"]:
        st["judge_constraints"].append(text)
    save_state(root, func, st)


# ---------------------------------------------------------------------------
# REVIEW-LOOP BREAKER (2026-09-04). func_80062020 spent 5 layer-1 FAILs and 3
# Judge PASS rulings on ONE byte-proven body in a single evening; func_80072CD4
# started the same loop the next hour. Root cause: layer-1 ran on every
# submission, was not bound by per-function Judge PASS rulings, and read its
# own prior FAILs (stored in judge_constraints) as precedent. Fix, mechanically:
#   * every review verdict is keyed by a BODY HASH (comment- and
#     whitespace-insensitive, so a comments-only re-file is the same body);
#   * a Judge PASS ruling records a CLEARANCE of candidate.c's body — the driver
#     skips layer-1 for that exact body (the Judge outranks layer-1 on policy,
#     judge-sole-gate) and goes straight to bytes + FINAL CALL;
#   * a body layer-1 already FAILed is not re-reviewed by layer-1 — it goes to
#     the Judge (the authoritative default-FAIL gate) for one decision;
#   * a body the Judge already FAILed at FINAL CALL (and no later clearance)
#     is rejected by the driver with NO review spent;
#   * layer-1 FAIL summaries live in `reviewer_history`, not judge_constraints,
#     and the briefs say they are not precedent.
# ---------------------------------------------------------------------------
LAYER1_NOTE_PREFIX = "LAYER-1 CHEAT-REVIEWER FAIL"
_C_COMMENT_RE = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)


def normalize_c(text):
    """Strip C comments and collapse whitespace so two spellings of the SAME
    code hash equal. Comments carry citations/annotations the reviewers read
    in the diff itself; for loop detection only the code matters."""
    text = _C_COMMENT_RE.sub(" ", text or "")
    text = re.sub(r"\s+", " ", text).strip()
    # drop every space that does not separate two identifier characters, so
    # `a+1` and `a + 1` (and `){` vs `) {`) are the same body
    return re.sub(r"(?<!\w) | (?!\w)", "", text)


def extract_function_body(norm, func):
    """`func`'s DEFINITION (identifier through its closing brace) from
    normalized C text, or None. Skips prototypes/calls (no `{` after the
    parameter list)."""
    for m in re.finditer(r"\b" + re.escape(func) + r"\s*\(", norm):
        i, depth = m.end() - 1, 0
        while i < len(norm):
            if norm[i] == "(":
                depth += 1
            elif norm[i] == ")":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        j = i + 1
        while j < len(norm) and norm[j] == " ":
            j += 1
        if j >= len(norm) or norm[j] != "{":
            continue
        depth = 0
        for k in range(j, len(norm)):
            if norm[k] == "{":
                depth += 1
            elif norm[k] == "}":
                depth -= 1
                if depth == 0:
                    return norm[m.start():k + 1]
        return None
    return None


def body_hash(text, func):
    """16-hex-char key for the candidate BODY of `func` in `text` (a whole
    src/*.c file or a bare candidate.c). Falls back to the whole normalized
    text when the definition is not found, so a hash always exists."""
    norm = normalize_c(text)
    body = extract_function_body(norm, func) or norm
    return hashlib.sha1(body.encode("utf-8")).hexdigest()[:16]


def body_hash_from_file(path, func):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return body_hash(f.read(), func)
    except OSError:
        return ""


def record_review_verdict(root, func, layer, verdict, h, summary=""):
    """Append {layer, verdict, hash, when, summary} to state['review_ledger']."""
    st = load_state(root, func)
    st.setdefault("review_ledger", []).append({
        "layer": layer, "verdict": verdict, "hash": h, "when": _now(),
        "summary": (summary or "")[:300]})
    save_state(root, func, st)


def record_judge_clearance(root, func, h, ref, justification=""):
    """A Judge PASS ruling on the body hashed `h` (candidate.c at ruling time).
    Also logged to review_ledger as layer=judge verdict=PASS so the ordering
    logic in review_disposition sees one timeline."""
    st = load_state(root, func)
    st.setdefault("judge_clearances", []).append({
        "hash": h, "when": _now(), "ref": ref,
        "justification": (justification or "")[:600]})
    st.setdefault("review_ledger", []).append({
        "layer": "judge", "verdict": "PASS", "hash": h, "when": _now(),
        "summary": f"ruling PASS ({ref})"})
    save_state(root, func, st)


def review_disposition(root, func, h):
    """What the record already says about body `h`:
      judge-cleared  — the Judge's LAST word on this body is PASS (ruling
                       clearance); layer-1 is skipped, bytes + FINAL CALL run.
      judge-failed   — the Judge's LAST word on this body is FAIL; the driver
                       rejects the resubmission without spending any review.
      layer1-repeat  — no Judge word yet, but layer-1 already FAILed this body;
                       layer-1 is skipped and the Judge decides once.
      fresh          — never reviewed."""
    st = load_state(root, func) or {}
    events = [e for e in st.get("review_ledger", []) if e.get("hash") == h]
    judge = [e for e in events if e.get("layer") == "judge"]
    if judge:
        last = sorted(judge, key=lambda e: e.get("when", ""))[-1]
        return "judge-cleared" if last.get("verdict") == "PASS" else "judge-failed"
    if any(e.get("layer") == "layer1" and e.get("verdict") == "FAIL" for e in events):
        return "layer1-repeat"
    return "fresh"


def add_reviewer_note(root, func, text):
    """Layer-1 FAIL findings: kept for the session to read, NEVER precedent."""
    st = load_state(root, func)
    st.setdefault("reviewer_history", [])
    if text and text not in st["reviewer_history"]:
        st["reviewer_history"].append(text)
    save_state(root, func, st)


def split_constraints(st):
    """(judge_constraints without legacy layer-1 lines, reviewer notes incl.
    legacy layer-1 lines). Ledgers written before 2026-09-04 stored layer-1
    FAIL summaries in judge_constraints."""
    jc = [str(c) for c in (st.get("judge_constraints") or [])]
    legacy = [c for c in jc if c.startswith(LAYER1_NOTE_PREFIX)]
    kept = [c for c in jc if not c.startswith(LAYER1_NOTE_PREFIX)]
    notes = [str(n) for n in (st.get("reviewer_history") or [])]
    return kept, legacy + [n for n in notes if n not in legacy]


def render_review_context(root, func, h=""):
    """Block for the layer-1 / Judge briefs: dated Judge clearances, prior
    verdicts on THIS body, and the precedence rule."""
    st = load_state(root, func) or {}
    out = ["REVIEW RECORD (mechanical, from state.json review_ledger):"]
    cl = st.get("judge_clearances") or []
    if cl:
        out.append("Judge PASS rulings on record (each is a DATED per-function ruling; it "
                   "SUPERSEDES every ban, layer-1 FAIL, and older Judge FAIL for the "
                   "construct it names — decide on the ruling's own terms, do not re-cite "
                   "what it superseded):")
        for c in cl[-4:]:
            out.append(f"  - {c.get('when', '')[:16]} body={c.get('hash', '')} ref={c.get('ref', '')}: "
                       f"{str(c.get('justification', ''))[:400]}")
    if h:
        ev = [e for e in st.get("review_ledger", []) if e.get("hash") == h]
        if ev:
            out.append(f"Prior verdicts on THIS EXACT body (hash {h}):")
            for e in ev[-6:]:
                out.append(f"  - {e.get('when', '')[:16]} {e.get('layer')} {e.get('verdict')}: "
                           f"{str(e.get('summary', ''))[:200]}")
        else:
            out.append(f"This body (hash {h}) has no prior review verdict.")
    out.append("PRECEDENCE: a layer-1 FAIL is a reviewer opinion, not precedent — never cite "
               "'already FAILed by layer-1' as a ground. The Judge outranks layer-1 on policy "
               "(judge-sole-gate). Bans and FAILs older than a Judge PASS ruling covering the "
               "same construct are stale.")
    return "\n".join(out)


def autoescalate(root, func, file_stem, scan_tier, rule_count, date):
    """Deterministic backstop: append a disposition entry for `func` to
    docs/grind/decisions.md from the ledger's exhaustion record, and return the
    escalation_ref line. The driver calls this when an `escalation`-modality session
    fails to self-file (dodges with a flat-floor progress), so the function can never
    loop unresolved. Per the owner's 2026-07-27 standing auto-ruling, a non-STRONG
    scan tier means both AND-gates fail and the entry is RESOLVED — since the
    2026-08-31 ruling (ordinary-c-judge-decidable) that disposition is a SILENT
    FORECLOSURE (no decision packet, nothing surfaced to the owner). Per the
    2026-08-18 ruling (judge-sole-gate, b9d91163) a STRONG tier routes to the
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
    elif isinstance(floor, int) and floor > ENDGAME_LOCK_MAX_FLOOR:
        # Owner ruling 2026-09-02 (ruling 2): a residual wider than the endgame-lock
        # floor is NOT the standing ruling's subject — the record says what it is.
        ref = f"{date} — {func} — LADDER EXHAUSTED (non-endgame residual, floor {floor}): FORECLOSED (auto-filed by driver, exhaustion backstop after two full ladder cycles)"
        tail = f"""This is NOT an endgame lock (honest floor {floor} > ENDGAME_LOCK_MAX_FLOOR =
{ENDGAME_LOCK_MAX_FLOOR}; owner ruling 2026-09-02): the 2026-07-27 standing ruling is not
claimed. The floor held flat across two full ladder cycles ({ESCALATION_FLAT_SESSIONS_WIDE}
sessions, >= {ESCALATION_MIN_MODALITIES_WIDE} modalities) with `scan_hand_coded --single
{func}` = **{scan_tier}**. Per the owner's 2026-08-31 ruling
(.claude/rules/ordinary-c-judge-decidable.md) the item is FORECLOSED silently — this
entry is the proof-of-foreclosure record, not a question. The function stays INCLUDE_ASM
on main; re-activation triggers are a new owner class grant covering the residual, a
toolchain-fidelity finding, a new diagnostic/modality, or an explicit owner `queue
unpark` (which resets the exhaustion window)."""
    else:
        ref = f"{date} — {func} — RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED (endgame lock, both gates fail; auto-filed by driver, exhaustion backstop)"
        tail = f"""Both AND-gates fail on the ledger evidence: canonical-asm — `scan_hand_coded --single
{func}` = **{scan_tier}** (ordinary GCC RA/scheduler artifact, no hand-coded signature);
coercion family — no SOTN-master precedent recorded for the residual axes. Per the owner's
2026-08-31 ruling (.claude/rules/ordinary-c-judge-decidable.md): the item is FORECLOSED
silently — this entry is the proof-of-foreclosure record, not a question; nothing is
surfaced to the owner and no packet exists. The two AND-gates remain the unchanged
STANDARD. The function stays INCLUDE_ASM on main; re-activation triggers are a new owner
class grant covering the residual, a toolchain-fidelity finding, or an explicit owner
`queue unpark`."""
    entry = f"""
## {ref}

**Auto-filed by the grinder driver ({date})** after {sessions} sessions held the honest
floor flat at {floor} across {len(mods)} distinct modalities ({', '.join(mods)}) without a
session self-filing — the escalation-modality backstop (grind.ps1). This is the endgame-lock
species per the standing 2026-07-20 endgame-lock-disposition policy: byte-matches on main only
via a cheat (cheat-asm), honest pure-C floor {floor},
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
    tier is not STRONG-class or the function is already listed.

    Second evidence door (owner ruling 2026-08-30, decisions.md
    escalation-batch entry, ruling 4): a function enumerated BY NAME in
    tools/grinder/owner_cluster_grants.txt — the operator-maintained registry
    of landed owner cluster rulings — qualifies with tier
    "OWNER-CLUSTER". Sessions can never add registry rows (tools/ is outside
    session scope); every other gate applies unchanged."""
    tier_s = str(tier).upper()
    if "STRONG" not in tier_s:
        reg = os.path.join(root, "tools", "grinder", "owner_cluster_grants.txt")
        cluster_cite = None
        if os.path.isfile(reg):
            with open(reg, encoding="utf-8") as f:
                for ln in f:
                    ln = ln.strip()
                    if ln and not ln.startswith("#") and ln.split()[0] == func:
                        cluster_cite = ln.split(None, 1)[1] if " " in ln else ""
                        break
        if cluster_cite is None:
            return None
        tier = f"OWNER-CLUSTER ({cluster_cite})"
    allow = os.path.join(root, "inline_asm_canonical.txt")
    with open(allow, encoding="utf-8") as f:
        existing = {ln.strip().split()[0] for ln in f
                    if ln.strip() and not ln.strip().startswith("#")}
    if str(tier).startswith("OWNER-CLUSTER"):
        line = (f"{func}  # pipeline grant {date}: {tier} — owner-enumerated "
                f"cluster member (registry tools/grinder/owner_cluster_grants.txt; "
                f"door per owner ruling 2026-08-30), judge-verified. "
                f"Packet in docs/grind/decisions.md {date} entry.")
    else:
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


def island_count(root, func, stem):
    """(n_islands, allowlisted) for the APPLIED body of `func` in src/<stem>.c.

    Owner Ruling C 2026-09-02 (decisions.md "2026-09-02 — OWNER RULING"): two
    cluster members (func_8002E838, func_80031890) merged as COMPLETED-C because
    the Judge said PASS (with a prose "write the allowlist line" note the driver
    never parses) and the engine gate scores cop2 islands as non-cheat
    ([[cop2-island-completed-c-gate-gap]]). The PASS path now asks this
    question BEFORE `queue done`: n>0 and not allowlisted => the grant door
    (grant_canonical_asm) must admit the function or the merge is refused.
    Uses the SAME island scanner as tools/audit_asm_cheats.py /
    tools/check_completion_integrity.py so the three can never disagree."""
    tools_dir = os.path.join(root, "tools")
    if tools_dir not in sys.path:
        sys.path.insert(0, tools_dir)
    import audit_asm_cheats as AAC  # noqa: E402
    src = os.path.join(root, "src", f"{stem}.c")
    n = 0
    if os.path.isfile(src):
        with open(src, encoding="utf-8", errors="replace") as f:
            text = f.read()
        for _f, _line, _n, fname, _insns in AAC.scan_c_body_smuggled_work(text, f"{stem}.c"):
            if fname == func:
                n += 1
    allow = os.path.join(root, "inline_asm_canonical.txt")
    listed = False
    if os.path.isfile(allow):
        with open(allow, encoding="utf-8") as f:
            listed = any(ln.strip() and not ln.strip().startswith("#")
                         and ln.strip().split()[0] == func for ln in f)
    return n, listed


MODALITY_PLAYBOOK = {
    "recon": ("Baseline + map. Run canonical + sandbox for the honest floor; scan for "
              "sibling/duplicate analogs (tmp/duplicates_leads.txt, tools/find_duplicates.py); "
              "read the m2c reference shape; read asm/funcs/<func>.s. Output an initial "
              "frontier of 1-3 mechanism-grounded hypotheses."
              " DATA MODEL FIRST: the brief's DATA MODEL section lists every global the "
              "target touches with its census row, header declaration, cross-references "
              "and mechanical SIGNALS. If ANY signal is present, hypothesis #1 is the "
              "declaration fix (measure it with sandbox before any codegen lever), and you "
              "MUST bank one evidence entry beginning `OBJECT MODEL:` giving, per flagged "
              "symbol, MATCHES / MISMATCH (measured score N) / MISMATCH-unmeasured (why). "
              "The driver discards a recon session on a flagged function without it."),
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
                 "list them in artifacts."
                 " CHASSIS RULE (2026-09-01): if the ledger already banks a permuter campaign "
                 "on the SAME candidate chassis with 0 novel finds after >=20k iterations, "
                 "re-seeding it is not a valid probe (it validates mechanically but is a "
                 "wasted session on the record) — permute a structurally "
                 "different chassis (a banked instance-kill form is a good seed) or a "
                 "different lever hint. The permuter cannot express N-way statement "
                 "duplication into arms, goto-into-existing-body, chassis swaps, or "
                 "FAKE-construct removal — if the frontier names one of those, spend the "
                 "session on `tools/sweep_variants.py` over hand-written variants and "
                 "`tools/fake_ablate.py`, and bank those measurements as the artifact."),
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
               "`--target <stem>.tgt.head.s`. (5) A typed "
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
                  "and list them in artifacts."
                  " PASS-INPUT ENUMERATION: naming the pass that produced the divergence "
                  "is half the job; the other half is enumerating the source-side INPUT "
                  "shapes that change what the pass sees (func_80038C70: 46 sessions on "
                  "how to stop find_cross_jump, when the closer was to give the block no "
                  "set-insn at all). Use tools/loop_movables.py (loop.c decisions, every "
                  "predicate term incl. n_times_set and the running threshold), "
                  "tools/nrefs_census.py (reg_n_refs / allocno order + what-if lifts), "
                  "tools/label_census.py (branch targets, predecessors, callee-saved ref "
                  "counts, target vs build) and bank their output as artifacts."),
    "rederive": ("Re-derivation: fresh m2c decompile, decomp.me corpus "
                 "(tools/decomp_me_scrape.py), sibling/Kengo transplant. Produce a "
                 "structurally DIFFERENT C shape, not a tweak of the current one."),
    "synthesis": ("Re-read the ENTIRE ledger (evidence.md + hypotheses.md + rejected/). "
                  "Write the best merged attack. Reset the frontier to the strongest 1-3 "
                  "hypotheses for the next ladder pass."
                  " KILL RE-AUDIT (2026-09-01): list every instance kill in state.json "
                  "kills[] whose measured_on differs from the current chassis or names a "
                  "FAKE construct no longer present; re-measure the two closest-to-target "
                  "ones with tools/fake_ablate.py BEFORE proposing anything new. "
                  "CONTRADICTION RULE: if the ledger now marks EVERY chassis as foreclosed, "
                  "impossible, or dead, at least one of those verdicts is wrong — the "
                  "matching C exists. Re-audit the WEAKEST foreclosure (the one with no "
                  "predicate_cite, or the oldest) first; func_80041188 spent 14 sessions "
                  "proving one chassis impossible while the other sat foreclosed on an "
                  "incomplete loop.c predicate. (This rule governs ledger-internal chassis "
                  "verdicts; it does not override a driver-declared exhaustion "
                  "disposition in `escalation` modality.)"),
    "object-model": ("OBJECT-MODEL AUDIT — the driver has found the honest floor flat across "
                     "the ladder and will declare exhaustion NEXT session unless the object "
                     "model is checked first. func_80033550 (2026-09-03) spent 13 of 17 sessions "
                     "modeling register allocation for a tail that named_syms.txt had described "
                     "as a 12-byte record table since 2026-05-17; the residual closed the moment "
                     "the three per-word scalars were declared as one record. Your ENTIRE job: "
                     "for EVERY global in the brief's DATA MODEL section, state declared shape vs "
                     "evidence (census row, sibling-function addressing in asm/funcs/*.s, index "
                     "arithmetic in this function's own asm, the m2c shape). For every mismatch, "
                     "spell the corrected DECLARATION (header-level; aggregate-merge family prongs "
                     "(a)-(e); never a per-use cast) and MEASURE it with sandbox. Then list the "
                     "PREMISES the current floor argument rests on (e.g. 'the tail is three "
                     "independent scalar stores') as explicit, attackable statements. Bank ONE "
                     "evidence entry beginning `OBJECT MODEL:` with the per-symbol verdicts "
                     "(MATCHES / MISMATCH measured score N / MISMATCH-unmeasured why) and the "
                     "premise list; a session without it is invalid and discarded. A corrected "
                     "declaration that needs include/*.h or a symbol-config edit is an integration "
                     "handoff (candidate-ready with the surface named), not a scope violation."),
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
                   "byte-match (cheat-asm); (c) state whether "
                   "an in-hand SOTN-master precedent EXISTS for the closing construct "
                   "(file+line citation — 'same spirit' does not count). If BOTH gates FAIL "
                   "(scan LOW + no precedent — the common case), APPLY THE OWNER'S STANDING "
                   "RULING (2026-07-27, .claude/rules/endgame-lock-disposition.md; silent "
                   "foreclosure per the 2026-08-31 ruling ordinary-c-judge-decidable): APPEND "
                   "an `## <date> — <func> — **RESOLVED BY STANDING RULING (2026-07-27): "
                   "FORECLOSED**` entry to docs/grind/decisions.md stating both gates' "
                   "evidence and the exhaustion (sessions/modalities/permuter iters from the "
                   "ledger) — this is a proof-of-foreclosure RECORD, never a question or a "
                   "decision packet addressed to the owner — then return result=owner-gated "
                   "with escalation_ref citing that entry; the driver forecloses silently, "
                   "no owner wait, nothing surfaced. If gate (a) PASSES (STRONG scan tier), "
                   "append a `## <date> — <func> — CANONICAL-ASM GRANT PATH` entry with the "
                   "scanner evidence and return owner-gated citing it — per the owner's "
                   "2026-08-18 ruling (judge-sole-gate) the function STAYS ACTIVE and the "
                   "next session authors the whole-body canonical form; no owner wait. If "
                   "only gate (b) passes (an actually-exhibited SOTN precedent), file the "
                   "foreclosure entry AND include the precedent citation — the driver "
                   "borderline-logs it for owner batch review; the frozen list is owner-only "
                   "to extend and the disposition is still the silent foreclosure. NEVER "
                   "file an 'awaiting owner ruling' or 'DECISION PACKET' entry — those "
                   "shapes are retired. A flat-floor `progress` is NOT an acceptable "
                   "outcome this session — the driver will auto-file the disposition if "
                   "you dodge."),
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
            "Do NOT rename the symbol (queue keys and this ledger all "
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


# ── Sibling ledgers (2026-09-04 post-mortem: the CD_datasync plateau) ────────
# CD_datasync sat 41 sessions (s9-s49) at floor 7 on a printf window it shares
# byte-for-byte with CD_sync and CD_ready. CD_sync solved that window to floor
# 2 on 2026-09-02 and was then FORECLOSED — off the queue, so nothing ever
# dispatched it again and nothing carried its candidate back. CD_datasync's
# ledger had named CD_sync as a twin at s16 and never re-read it; s50 finally
# did and went 7 -> 2 in one session. Two mechanisms close the gap:
#   1. the brief carries a SIBLING LEDGERS block — every other ledger this one
#      names (or that names this one), with its CURRENT floor, the date that
#      floor was reached, the date THIS ledger last mentioned it, and an
#      UNSPENT flag when the sibling improved after that mention;
#   2. a genuine floor DROP on any function stamps `sibling_progress` into the
#      ledger of every sibling; assign_modality then forces ONE `rederive`
#      session on that sibling (consumed by its next applied session, so it
#      can never loop), and the brief says why.
# Everything here is read-only over other ledgers except the stamp, which is
# wrapped so a broken sibling ledger can never make `apply` fail (an `apply`
# that raises leaves session_count unbumped and the driver re-dispatches the
# same session number forever).
_SIBLING_MAX_BYTES = 4 * 1024 * 1024
_JOURNAL_LINE_RE = re.compile(r"^- (\d{4}-\d{2}-\d{2} \d{2}:\d{2}) (\S+) s(\d+) \[")
_SESSION_TAG_RE = re.compile(r"\[s(\d+)\]")


def _ident_like(name):
    """A function name safe to word-search in prose. All-lowercase alphabetic
    names (`main`, `prnt`, `sprintf`) are ordinary English/prose tokens and
    would make every ledger a sibling of `main`; anything carrying a digit,
    an underscore, or an upper-case letter is treated as an identifier."""
    return bool(name) and not (name.isalpha() and name.islower())


def _read_capped(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return f.read(_SIBLING_MAX_BYTES)
    except OSError:
        return ""


def _ledger_text(root, func):
    """hypotheses.md + evidence.md + the state's frontier/headlines — the
    surfaces a ledger uses to name other functions."""
    d = ledger_dir(root, func)
    parts = [_read_capped(os.path.join(d, "hypotheses.md")),
             _read_capped(os.path.join(d, "evidence.md"))]
    st = load_state(root, func) or {}
    parts.append(json.dumps(st.get("frontier", [])))
    parts.append(json.dumps([e.get("headline", "") for e in st.get("floor_history", [])]))
    return "\n".join(parts)


def _ledger_names(root, func, extra=()):
    st = load_state(root, func) or {}
    names = [func, str(st.get("func") or "")] + list(extra)
    out = []
    for n in names:
        if _ident_like(n) and n not in out:
            out.append(n)
    return out


def _name_re(names):
    return re.compile(r"(?<![\w$])(?:" + "|".join(re.escape(n) for n in names) + r")(?![\w$])")


def _last_mention_session(root, func, pat):
    """Highest session number under which this ledger's hypotheses.md /
    evidence.md mention a name matching `pat` (None if never)."""
    d = ledger_dir(root, func)
    best = None
    cur = None
    for line in _read_capped(os.path.join(d, "hypotheses.md")).splitlines():
        m = re.match(r"^## \[s(\d+)\]", line)
        if m:
            cur = int(m.group(1))
        if pat.search(line) and cur is not None:
            best = cur if best is None else max(best, cur)
    # evidence.md entries start `- [sN] ...` and may continue over several
    # lines; an INLINE `[sN]` inside the text is a citation of some other
    # ledger's session (CD_datasync cited "CD_ready evidence [s68]" at its own
    # s20), so only the entry-leading tag sets the current session.
    cur = None
    for line in _read_capped(os.path.join(d, "evidence.md")).splitlines():
        m = re.match(r"^- \[s(\d+)\]", line)
        if m:
            cur = int(m.group(1))
        if pat.search(line) and cur is not None:
            best = cur if best is None else max(best, cur)
    return best


def journal_session_dates(root):
    """{(func_name, session): 'YYYY-MM-DD HH:MM'} from docs/grind/journal.md —
    the only dated per-session record; floor_history carries no timestamps."""
    out = {}
    for line in _read_capped(os.path.join(root, "docs", "grind", "journal.md")).splitlines():
        m = _JOURNAL_LINE_RE.match(line)
        if m:
            out[(m.group(2), int(m.group(3)))] = m.group(1)
    return out


def _floor_since(hist):
    """(floor, session) — the current int floor and the session that opened
    its trailing run (the last session whose floor differed). WIP-imported
    ledgers carry a session-0 seed and non-monotone early history, so a
    running-minimum would date CD_sync's floor 2 to s0; the trailing run
    dates it to the session that actually reached it."""
    ints = [(e.get("session"), e.get("floor")) for e in hist if isinstance(e.get("floor"), int)]
    if not ints:
        return None, None
    floor = ints[-1][1]
    since = ints[-1][0]
    for s, f in reversed(ints):
        if f != floor:
            break
        since = s
    return floor, since


def _queue_status(root, func):
    try:
        with open(os.path.join(root, "engine", "queue.json"), encoding="utf-8") as fh:
            qi = next((i for i in json.load(fh).get("items", []) if i.get("func") == func), None)
    except (OSError, ValueError):
        return "unknown"
    return str(qi.get("status", "unknown")) if qi else "not in queue"


def sibling_ledgers(root, func, names=None):
    """Every other ledger under memory/grind/ that this ledger names, or that
    names this ledger (identifier-like names only, word-bounded). Each entry:
    func, names, file, queue_status, floor, floor_since_session,
    floor_since_date, sessions, candidate (repo-relative path or None),
    candidate_mtime, mentioned_at_session, mentioned_at_date, unspent."""
    base = os.path.join(root, "memory", "grind")
    if not os.path.isdir(base):
        return []
    own_names = _ledger_names(root, func, names or ())
    own_text = _ledger_text(root, func)
    own_pat = _name_re(own_names) if own_names else None
    dates = journal_session_dates(root)
    own_st = load_state(root, func) or {}
    own_floor, _ = _floor_since(own_st.get("floor_history", []))
    out = []
    for d in sorted(os.listdir(base)):
        if d == func or not os.path.isfile(os.path.join(base, d, "state.json")):
            continue
        sib_names = _ledger_names(root, d)
        if not sib_names:
            continue
        sib_pat = _name_re(sib_names)
        outbound = bool(sib_pat.search(own_text))
        inbound = bool(own_pat and own_pat.search(_ledger_text(root, d)))
        if not (outbound or inbound):
            continue
        st = load_state(root, d) or {}
        floor, since = _floor_since(st.get("floor_history", []))
        since_date = next((dates.get((n, since)) for n in sib_names
                           if since is not None and (n, since) in dates), None)
        ment = _last_mention_session(root, func, sib_pat) if outbound else None
        if ment is not None and ment > int(own_st.get("session_count", 0)):
            ment = None               # a cited foreign session number, not ours
        ment_date = next((dates.get((n, ment)) for n in own_names
                          if ment is not None and (n, ment) in dates), None)
        cand = os.path.join(base, d, "candidate.c")
        cand_rel = f"memory/grind/{d}/candidate.c" if os.path.isfile(cand) else None
        cand_mtime = ""
        if cand_rel:
            try:
                cand_mtime = datetime.datetime.fromtimestamp(
                    os.path.getmtime(cand)).strftime("%Y-%m-%d %H:%M")
            except OSError:
                pass
        # UNSPENT = the sibling moved after this ledger last read it (dated), or
        # — when that cannot be dated, or this ledger never read it (a caller
        # that merely names us) — the sibling sits strictly BELOW our floor. A
        # caller at a higher floor is listed for context, never mandated.
        below = isinstance(floor, int) and isinstance(own_floor, int) and floor < own_floor
        if since_date and ment_date:
            unspent = since_date > ment_date
        else:
            unspent = below
        out.append({"func": d, "names": sib_names, "file": st.get("file", "?"),
                    "queue_status": _queue_status(root, d), "floor": floor,
                    "floor_since_session": since, "floor_since_date": since_date,
                    "sessions": int(st.get("session_count", 0)),
                    "candidate": cand_rel, "candidate_mtime": cand_mtime,
                    "mentioned_at_session": ment, "mentioned_at_date": ment_date,
                    "unspent": bool(unspent)})
    return out


def render_siblings(sibs, func):
    if not sibs:
        return ""
    lines = []
    for s in sibs:
        aka = f" (aka {', '.join(n for n in s['names'] if n != s['func'])})" if len(s["names"]) > 1 else ""
        since = (f" since its s{s['floor_since_session']}"
                 + (f" ({s['floor_since_date']})" if s["floor_since_date"] else "")) \
            if s["floor_since_session"] is not None else ""
        cand = (f"{s['candidate']} (written {s['candidate_mtime']})" if s["candidate"]
                else "no candidate.c")
        if s["mentioned_at_session"] is None:
            ment = "your ledger has NEVER mentioned it (it names you)"
        else:
            ment = (f"your ledger last mentions it at your s{s['mentioned_at_session']}"
                    + (f" ({s['mentioned_at_date']})" if s["mentioned_at_date"] else ""))
        flag = "  -> UNSPENT: it moved after that, or sits below your floor — READ ITS candidate.c FIRST" \
            if s["unspent"] else ""
        lines.append(f"  - {s['func']}{aka} src/{s['file']}.c — queue: {s['queue_status']} — "
                     f"floor {s['floor'] if s['floor'] is not None else '?'}{since} — "
                     f"{s['sessions']} sessions — candidate: {cand}\n"
                     f"      {ment}{flag}")
    return ("\n## SIBLING LEDGERS (auto — functions this ledger names, or that name this one)\n"
            "A sibling's ledger is inheritance you did not write, and it keeps moving after\n"
            "you last read it — INCLUDING when the sibling is foreclosed and off the queue.\n"
            "CD_datasync sat 41 sessions at floor 7 while foreclosed CD_sync held the shared\n"
            "window's fix at floor 2 (2026-09-04 post-mortem). For every UNSPENT sibling,\n"
            "BEFORE any probe of your own: read its candidate.c, transplant its spelling of\n"
            "every block you share onto your chassis, measure it, and bank the result as a\n"
            "CONFIRMED/KILLED hypothesis that names the sibling and its session number.\n"
            + "\n".join(lines) + "\n")


def sibling_progress_pending(st):
    """Unconsumed sibling_progress entries whose floor is at or below this
    ledger's own last int floor (or any, if this ledger has no int floor).
    AT-OR-BELOW, not strictly below (owner ruling 2026-09-04, Ruling B.2): a
    sibling reaching your floor from a different chassis is transplantable
    news — CD_datasync's s58 link-identical struct spelling reached neither
    CD_sync nor CD_ready, both sitting at the same floor 2, and both were
    re-foreclosed without it."""
    if not isinstance(st, dict):
        return []
    own, _ = _floor_since(st.get("floor_history", []))
    out = []
    for e in st.get("sibling_progress") or []:
        if e.get("consumed") is not None:
            continue
        f = e.get("floor")
        if isinstance(f, int) and (not isinstance(own, int) or f <= own):
            out.append(e)
    return out


def render_sibling_progress(st):
    pend = sibling_progress_pending(st)
    if not pend:
        return ""
    lines = "\n".join(f"  - {e.get('from')} reached floor {e.get('floor')} at its s{e.get('session')} "
                      f"({str(e.get('at', ''))[:16]}): {str(e.get('headline', ''))[:200]}\n"
                      f"      read memory/grind/{e.get('from')}/candidate.c and "
                      f"memory/grind/{e.get('from')}/hypotheses.md (its s{e.get('session')} entries)"
                      for e in pend)
    return ("\n## SIBLING PROGRESS SINCE YOUR LAST SESSION — THIS SESSION IS A FORCED REDERIVE\n"
            "A function whose ledger cites yours (or that yours cites) dropped its floor TO OR\n"
            "BELOW yours since you last ran. The driver forced `rederive` for exactly this session.\n"
            "Your first probe is the transplant: apply the sibling's new spelling of every\n"
            "shared block to your chassis and measure it. Bank the result either way; the\n"
            "notice is consumed when this session's outcome is applied, and does not repeat.\n"
            + lines + "\n")


def notify_siblings(root, func, session, floor, headline):
    """Stamp a floor DROP on `func` into every sibling ledger's
    `sibling_progress`. One live entry per source function (a newer drop
    replaces an unconsumed older one). Never raises."""
    stamped = []
    try:
        sibs = sibling_ledgers(root, func)
    except Exception:
        return stamped
    for s in sibs:
        try:
            st = load_state(root, s["func"])
            if not st:
                continue
            lst = [e for e in (st.get("sibling_progress") or [])
                   if not (e.get("from") == func and e.get("consumed") is None)]
            lst.append({"from": func, "floor": floor, "session": session, "at": _now(),
                        "headline": str(headline or "")[:200], "consumed": None})
            st["sibling_progress"] = lst[-12:]
            save_state(root, s["func"], st)
            stamped.append(s["func"])
        except Exception:
            continue
    return stamped


# ── Current-scope injection (2026-09-01 post-mortem) ─────────────────────────
# A ledger paraphrases a rule's scope at the time it was written; rule text
# moves by owner ruling. func_800283D0 banked its decisive find as "out of
# scope" by quoting a scoping abolished seven weeks earlier and lost 18
# sessions; func_8003C714 s1 carried the same caveat. The brief now prints the
# CURRENT `description:` line of every rule slug the ledger mentions, so a
# stale kill cannot survive a dispatch unchallenged.
_RULE_DESC_RE = re.compile(r'(?m)^description:\s*"?(.+?)"?\s*$')


def rule_descriptions(root):
    """{slug: current description line} for every .claude/rules/*.md."""
    d = os.path.join(root, ".claude", "rules")
    out = {}
    if not os.path.isdir(d):
        return out
    for fn in sorted(os.listdir(d)):
        if not fn.endswith(".md"):
            continue
        try:
            with open(os.path.join(d, fn), encoding="utf-8", errors="replace") as f:
                head = f.read(4000)
        except OSError:
            continue
        m = _RULE_DESC_RE.search(head)
        if m:
            out[fn[:-3]] = m.group(1).strip()
    return out


def cited_rule_scopes(root, func):
    """[(slug, current_description)] for every rule slug that appears anywhere
    in this function's ledger: state.json constraints/bans/frontier/floor
    history, hypotheses.md, evidence.md, candidate.c, and the first 60 lines
    of each rejected/*."""
    descs = rule_descriptions(root)
    if not descs:
        return []
    d = ledger_dir(root, func)
    texts = []
    st = load_state(root, func) or {}
    texts += [str(x) for x in st.get("judge_constraints", [])]
    texts += [str(x) for x in st.get("reviewer_history", [])]
    texts += [str(x) for x in st.get("banned_constructs", [])]
    texts += [json.dumps(f) for f in st.get("frontier", [])]
    texts.append(json.dumps(st.get("floor_history", [])))
    for name in ("hypotheses.md", "evidence.md", "candidate.c"):
        p = os.path.join(d, name)
        if os.path.isfile(p):
            try:
                with open(p, encoding="utf-8", errors="replace") as f:
                    texts.append(f.read())
            except OSError:
                pass
    rj = os.path.join(d, "rejected")
    if os.path.isdir(rj):
        for fn in sorted(os.listdir(rj)):
            try:
                with open(os.path.join(rj, fn), encoding="utf-8", errors="replace") as f:
                    texts.append("".join(f.readlines()[:60]))
            except OSError:
                pass
    blob = "\n".join(texts)
    return [(s, descs[s]) for s in sorted(descs) if s in blob]


def render_rule_scopes(pairs):
    """Render [(slug, description)] as the brief's current-scope block ('' if empty)."""
    if not pairs:
        return ""
    lines = "\n".join(f'  - {s}: "{d if len(d) <= 300 else d[:300] + "…"}" '
                      f'(.claude/rules/{s}.md)' for s, d in pairs)
    return ("\n## CURRENT SCOPE OF EVERY RULE THIS LEDGER CITES (authoritative NOW)\n"
            "Rule text changes by owner ruling. A scope quoted in hypotheses.md, a\n"
            "rejected/ header, or an older Judge ruling may be SUPERSEDED. The lines\n"
            "below are the rules' CURRENT `description:` lines, extracted at dispatch.\n"
            "If a banked kill, rejection, or ban rests on a NARROWER scoping than what is\n"
            "printed here, that kill is VOID: re-measure the form under the current scope\n"
            "before spending a session elsewhere.\n" + lines + "\n")


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
    _jc, _rn = split_constraints(st)
    constraints = "\n".join(f"  - {c}" for c in _jc) or "  (none)"
    reviewer = ""
    if _rn:
        reviewer = ("\nLayer-1 reviewer findings (READ these — they say what the reviewer will look\n"
                    "for — but they are NOT precedent and NOT constraints: the Judge outranks\n"
                    "layer-1, and a later dated Judge PASS ruling supersedes them):\n"
                    + "\n".join(f"  - {n[:400]}" for n in _rn[-6:]) + "\n")
    clearances = ""
    _cl = st.get("judge_clearances") or []
    if _cl:
        clearances = ("\n## JUDGE CLEARANCES ON RECORD (a Judge PASS ruling on a specific body)\n"
                      "The driver SKIPS layer-1 for a candidate whose body hash matches one of these\n"
                      "(comments/whitespace ignored) and runs bytes + FINAL CALL directly. If the\n"
                      "cleared body is the one in candidate.c, submit it EXACTLY — do not respell it.\n"
                      + "\n".join(f"  - {c.get('when', '')[:16]} body={c.get('hash', '')} ref={c.get('ref', '')}: "
                                  f"{str(c.get('justification', ''))[:240]}" for c in _cl[-3:]) + "\n")
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
    sup = st.get("superseded_bans") or []
    if sup:
        banned += ("\n## SUPERSEDED BANS (cleared by a later family grant — no longer enforced)\n"
                   + "\n".join(f"  - {str(b.get('text', ''))[:160]}\n      superseded by: {b.get('superseded_by', '')}" for b in sup) + "\n")
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
    # Data model (2026-09-03): declared shape vs evidence for every global the
    # target touches, plus a pun scan of the inherited candidate. Rows are
    # capped in engine/datamodel.py; a clean function costs a few lines.
    dmodel = ""
    _rows, _flags = data_model_signals(root, func)
    if _rows:
        dmodel = ("\n## DATA MODEL (auto: declared shape vs evidence, every global the target touches)\n"
                  + "\n".join(_rows) + "\n")
        if _flags:
            dmodel += ("SIGNALS — declaration-level facts, not codegen levers. A flagged symbol's\n"
                       "declaration is hypothesis #1 (func_80033550: 13 sessions of RA modeling for a\n"
                       "record copy the census had named since 2026-05-17):\n"
                       + "\n".join("  " + f for f in _flags) + "\n")
    _puns = scan_declaration_puns(root, func)
    if _puns:
        dmodel += ("\n## DECLARATION PUNS IN candidate.c (auto-scan)\n"
                   "Each spells an object model at the USE site. The sanctioned fix is at the\n"
                   "DECLARATION (aggregate-merge family prong (d); header edits go through an\n"
                   "integration handoff). A candidate carrying these FAILs layer-1:\n"
                   + "\n".join("  " + h for h in _puns) + "\n")
    psyq = psyq_identity(root, func)
    ksweep = knowledge_sweep(root, func, names=_names)
    # Sibling ledgers (2026-09-04): the surfaces knowledge_sweep cannot see —
    # the sibling's CURRENT floor and candidate, and whether this ledger has
    # read them since they moved. Degrades to '' on any failure.
    try:
        siblings = render_siblings(sibling_ledgers(root, func, names=_names), func)
    except Exception:
        siblings = ""
    try:
        sib_progress = render_sibling_progress(st)
    except Exception:
        sib_progress = ""
    last_floor = next((e.get("floor") for e in reversed(st["floor_history"])
                       if isinstance(e.get("floor"), int)), None)
    chassis = (f"\n## CHASSIS CHECK (driver-measured at dispatch — trust THIS number)\n"
               f"HEAD honest floor right now: {head_floor or 'measurement unavailable'}\n"
               f"Ledger's last recorded floor: {last_floor if last_floor is not None else '(none)'}\n"
               f"If these differ, the chassis has changed since the ledger entry: every banked\n"
               f"spelling conclusion is chassis-relative and MUST be re-measured before it is\n"
               f"spent. Do not quote the ledger floor to the Judge; quote this one.\n")
    try:
        scopes = render_rule_scopes(cited_rule_scopes(root, func))
    except Exception:
        scopes = ""
    kills = st.get("kills") or []
    inst = [k for k in kills if k.get("kill_scope") != "class"]
    cls = [k for k in kills if k.get("kill_scope") == "class"]
    # A WIP-imported ledger can carry a PROSE floor (func_80062020): the
    # isinstance guard then leaves the trigger dark by design — no comparable
    # number means no evidence the floor is flat.
    floors_only = [e.get("floor") for e in st["floor_history"][-3:]]
    flat3 = (len(floors_only) == 3 and all(isinstance(f, int) for f in floors_only)
             and len(set(floors_only)) == 1)
    kill_block = ""
    if kills:
        kill_block = ("\n## KILL LEDGER\n"
                      f"  instance kills (chassis-relative, RE-TESTABLE): {len(inst)}\n"
                      f"  class kills (predicate-cited, standing): {len(cls)}\n")
    if inst:
        kill_block += "  newest instance kills:\n" + "\n".join(
            f"    s{k.get('session', '')}: {str(k.get('statement', ''))[:110]}\n"
            f"        measured on: {str(k.get('measured_on', ''))[:110]}"
            for k in inst[-6:]) + "\n"
    if flat3 and inst:
        kill_block += (
            "\n## KILL RE-AUDIT REQUIRED (floor flat 3 sessions; instance kills exist)\n"
            "An instance kill is only as good as the chassis and FAKE state it was measured\n"
            "under. Before ANY new probe this session: pick the instance kill whose form sat\n"
            "closest to the target, and re-measure it (a) on the CURRENT chassis and (b) with\n"
            "every FAKE construct ablated (`python3 tools/fake_ablate.py --func " + func +
            " --file " + st['file'] + " --candidate <form.c>`). A lever measured 'inert' while\n"
            "a FAKE carrier occupied its target pseudo is not a kill (func_8002EA24 s8);\n"
            "a lever killed on 3 of 12 arms is not a class kill (func_800324D0 s6). Record\n"
            "the re-measurement as a hypothesis either way.\n")
    return f"""# GRIND SESSION — {func} (src/{st['file']}.c)

You are session {st['session_count'] + 1} of a cumulative grind. Your mandated
modality for THIS session is: **{modality}**

{psyq}
{sib_progress}{ksweep}{siblings}{chassis}
{MODALITY_PLAYBOOK[modality]}
{scopes}{fixup}{directive}{consistency}{dmodel}{banned}{clearances}
## Ledger state (your inheritance — do not re-derive any of it)
Floor history:
{floors}
{kill_block}
Live frontier:
{frontier}

Judge constraints (BINDING — forms/techniques already ruled out):
{constraints}
{reviewer}
Rejected forms bank (do NOT re-propose; full list in memory/grind/{func}/rejected/): {(f"{len(rejected)} total, newest: " + ', '.join(rejected[-12:])) if len(rejected) > 12 else (', '.join(rejected) or '(empty)')}

READ before working: memory/grind/{func}/evidence.md, memory/grind/{func}/hypotheses.md,
memory/grind/{func}/candidate.c (apply it to src/{st['file']}.c as your starting point).

## Your contract
- Work ONLY {func} in src/{st['file']}.c. Engine commands: `& tools/wteng.ps1 main sandbox {func} --disable all` (your gradient), canonical, diagnose. NEVER edit .claude/rules/engine/tools/Makefile/*.ld; NEVER run queue done/retire; NEVER commit.
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
  "headline": "<one line>", "hypotheses": [{{"statement","mechanism","probe","result","verdict":"CONFIRMED"|"KILLED",
     "kill_scope":"instance"|"class" (KILLED only, REQUIRED), "measured_on":"<chassis + FAKE state>" (KILLED only, REQUIRED),
     "predicate_cite":"<file:line of the gate predicate — must EXIST>" (class kills only, REQUIRED)}}],
  "evidence": ["fact ..."], "frontier": [<=3 of {{"hypothesis","mechanism","next_probe"}}],
  "artifacts": ["tmp/grind/..."], "ruling_question": "", "escalation_ref": ""}}
- "candidate-ready" means: sandbox distance 0 THIS session, edits in place in src/. The driver re-verifies bytes itself — never claim it speculatively.
- KILL SCOPE IS MANDATORY. `kill_scope` is REQUIRED on every KILLED hypothesis; `instance` is the normal choice = "this form, on this chassis, with these FAKE constructs present, measured N" — re-testable. `class` = "every form fails predicate P" and needs `predicate_cite` as file:line. The check reads your STATEMENT field only (your `result` narration is free prose): wording like "unreachable", "any natural geometry", "all forms", "impossible" in the STATEMENT of an instance kill makes the session INVALID. Say what you measured, not what you inferred.
- SELF-VET IS MANDATORY FOR candidate-ready. Before you write the outcome JSON, write memory/grind/{func}/self_vet.md using the template in your role prompt: a CONSTRUCTS: line, the six cheat-checklist tests answered IN WRITING for every construct in your diff, a SANCTIONED-FAMILY-CLAIMS: section (each claimed family carrying its rule's SCOPE sentence quoted VERBATIM plus a PRECEDENT as file:line or a commit hash), and an ANNOTATION-CONFORMANCE: line. The driver checks all of that mechanically and DISCARDS a candidate-ready session that lacks it — the same disposition as a scope violation. Then a fresh adversarial cheat-reviewer (layer 1) rules on your diff BEFORE the Judge is spawned; a layer-1 FAIL bounces straight back without a Judge cycle. Writing the vet honestly is how you pass both: if you cannot quote a scope sentence and cite a precedent for a family you are claiming, you do not have that family, and the correct outcome is `ruling-request`, not a submission.
- REVIEW LOOPS ARE CLOSED MECHANICALLY: the driver keys every review verdict by the BODY (comments/whitespace ignored). A body the Judge FAILed at FINAL CALL is rejected on resubmission with no review — respelling comments does not make a new body. A body layer-1 FAILed goes to the Judge directly the second time (the Judge decides once; layer-1 does not re-run). A body a Judge PASS ruling cleared skips layer-1. So: if layer-1 FAILed a body you believe is ordinary C, `ruling-request` with the precise question is the ONE correct move — never resubmit with new comments, never respell to dodge a ban.
- "ruling-request" is for a construct you cannot classify (sanctioned SOTN family vs cheat; genuine hand-written-asm evidence). Ask a precise question.
- "owner-gated" is for when every remaining sanctioned axis is measured dead. FILE the entry in docs/grind/decisions.md YOURSELF THIS session (docs/grind/ is in your allowed surface), THEN return owner-gated with escalation_ref citing it — you do not wait for an entry to pre-exist, you create it. The driver verifies the entry names {func} and FORECLOSES the function silently (owner ruling 2026-08-31, .claude/rules/ordinary-c-judge-decidable.md — a recorded disposition, never a question to the owner) so the queue advances. Never use it to defer work that is still grindable (a floor still dropping is grindable). This is the mandated outcome in `escalation` modality.
- OWNER'S STANDING AUTO-RULING (2026-07-27) — this governs HOW you word an escalation, and it NEVER authorizes ending a function early. Two separate questions, do not conflate them:
  (A) IS THE FUNCTION EXHAUSTED? This is the DRIVER's call, not yours. The driver assigns `escalation` modality only after the honest floor has been FLAT across many sessions AND >=4 DISTINCT modalities. If your mandated modality is NOT `escalation`, the answer is NO — you may not dispose of the function, however dead your own axis looks. A killed axis is a `progress` outcome with the kills banked; the ladder still has untried modalities (forensics / rederive / synthesis) and the owner's standing directive is to work the top item to completion however many sessions it takes ([[no-deferral-work-to-completion]], [[difficult-is-not-impossible]]). Judge FAILs do NOT make a function exhausted — a FAILed construct is one dead lever, and a FAIL on annotation FORMAT is a one-comment fix, not a wall.
  (B) ONCE THE DRIVER HAS DECLARED EXHAUSTION (you are in `escalation` modality), evaluate the two endgame-lock AND-gates: (1) canonical-asm needs STRONG `scan_hand_coded` signals (S1/S2/S6); (2) a coercion/spelling family needs an in-hand SOTN-master precedent you can CITE (file+line or commit). "Same spirit", "genre-adjacent", "only lever left", "measured to work", and a partition/elimination argument do NOT qualify — and a census you ran that came back NEGATIVE is a FAILED gate, not an open question. Whatever the gate outcome, your entry is a PROOF-OF-FORECLOSURE RECORD (owner ruling 2026-08-31, .claude/rules/ordinary-c-judge-decidable.md — the DECISION PACKET shape is retired; you never address a question to the owner): if the flat honest floor is <= {ENDGAME_LOCK_MAX_FLOOR} title it `## <date> — {func} — **RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED**`; if the floor is > {ENDGAME_LOCK_MAX_FLOOR} the standing ruling is NOT its subject (owner ruling 2026-09-02) — title it `## <date> — {func} — **LADDER EXHAUSTED (non-endgame residual, floor N): FORECLOSED**` and never claim endgame-lock status. Either way state (i) the gate evidence (scan tier, precedent census result), (ii) evidence POINTERS (ledger lines, measurements, scan output), (iii) the re-activation triggers that would make the residual attackable again (a class grant covering it, a toolchain finding). The driver forecloses the item silently; nothing is surfaced to the owner; an owner unpark or a later ruling re-activates it. "This is hard" is NOT a disposition: if the ladder is not exhausted, the honest outcome is `progress` with the kills banked, and the item stays active for the next modality. AUTO-REJECT CLASS (owner ruling 2026-08-24, reaffirmed 2026-08-31): a construct outside the frozen family list is a clean FAIL/refusal — do not argue for it in the record beyond citing the negative census; that residual's disposition is the same silent foreclosure.
- Bytes proven but blocked ONLY by a surface you may not touch (prologue_config.json/inline_asm_canonical.txt) is an INTEGRATION HANDOFF, not an endgame lock: say so plainly in the entry, list the exact operator steps, and return owner-gated. Do not dress it up as exhaustion — and note the operator still runs a fresh layer-2 cheat-reviewer on your C before it is accepted, so a Judge PASS on a construct is not a guarantee of acceptance.
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
    #   grindlib.py defer-escalation <root> <func> <outcome_json_path> -> forced next modality | none
    #   grindlib.py constrain <root> <func> <text>
    #   grindlib.py reviewer-note <root> <func> <text>               (layer-1 findings; not precedent)
    #   grindlib.py body-hash <root> <func> <path>                   -> body key (comment/ws-insensitive)
    #   grindlib.py review-verdict <root> <func> <layer> <verdict> <hash> [summary]
    #   grindlib.py clearance <root> <func> <hash> <ref> [justification]   (Judge PASS ruling)
    #   grindlib.py review-disposition <root> <func> <hash>          -> judge-cleared|judge-failed|layer1-repeat|fresh
    #   grindlib.py review-context <root> <func> [hash]              -> brief block
    #   grindlib.py grant-canonical-asm <root> <func> <tier> <date>   -> prints allowlist line / exit 1 refused
    #   grindlib.py log-borderline <root> <func> <category> <evidence> <disposition> <date>
    #   grindlib.py rule-scopes <root> <func>                        -> prints the current-scope block
    #   grindlib.py supersede-bans <root> <func> <superseded_by> <needle> [needles...]  -> prints count moved
    #   grindlib.py siblings <root> <func>                          -> sibling-ledger block + pending notices
    import sys
    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8")
    cmd = sys.argv[1]
    if cmd == "brief":
        hf = sys.argv[6] if len(sys.argv) > 6 else ""
        print(build_brief(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], hf))
    elif cmd == "pun-scan":
        # pun-scan <root> <func>  -> one line per per-use address pun in candidate.c
        print("\n".join(scan_declaration_puns(sys.argv[2], sys.argv[3])))
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
    elif cmd == "sync-unpark":
        # sync-unpark <root> <func>  -> prints "stamped" when the exhaustion
        # window was reset for a fresh unpark_reason (owner ruling 2026-09-02)
        print("stamped" if sync_unpark(sys.argv[2], sys.argv[3]) else "unchanged")
    elif cmd == "defer-escalation":
        # defer-escalation <root> <func> <outcome_json_path> -> prints the forced
        # next modality when the escalation session is honored as progress
        # (owner ruling 2026-09-04, Ruling B.1), else "none". Call after apply.
        o = json.load(open(sys.argv[4], encoding="utf-8"))
        print(escalation_deferral(sys.argv[2], sys.argv[3], o) or "none")
    elif cmd == "constrain":
        add_judge_constraint(sys.argv[2], sys.argv[3], sys.argv[4])
    elif cmd == "reviewer-note":
        # reviewer-note <root> <func> <text>  (layer-1 findings: not precedent)
        add_reviewer_note(sys.argv[2], sys.argv[3], sys.argv[4])
    elif cmd == "body-hash":
        # body-hash <root> <func> <path>  -> prints the 16-hex body key ("" if unreadable)
        p = sys.argv[4]
        if not os.path.isabs(p):
            p = os.path.join(sys.argv[2], p)
        print(body_hash_from_file(p, sys.argv[3]))
    elif cmd == "review-verdict":
        # review-verdict <root> <func> <layer> <verdict> <hash> [summary]
        record_review_verdict(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], sys.argv[6],
                              sys.argv[7] if len(sys.argv) > 7 else "")
    elif cmd == "clearance":
        # clearance <root> <func> <hash> <ref> [justification]
        record_judge_clearance(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5],
                               sys.argv[6] if len(sys.argv) > 6 else "")
    elif cmd == "review-disposition":
        # review-disposition <root> <func> <hash> -> judge-cleared|judge-failed|layer1-repeat|fresh
        print(review_disposition(sys.argv[2], sys.argv[3], sys.argv[4]))
    elif cmd == "review-context":
        # review-context <root> <func> [hash] -> brief block
        print(render_review_context(sys.argv[2], sys.argv[3],
                                    sys.argv[4] if len(sys.argv) > 4 else ""))
    elif cmd == "autoescalate":
        # autoescalate <root> <func> <file_stem> <scan_tier> <rule_count> <date>
        print(autoescalate(sys.argv[2], sys.argv[3], sys.argv[4],
                           sys.argv[5], sys.argv[6], sys.argv[7]))
    elif cmd == "grant-canonical-asm":
        # grant-canonical-asm <root> <func> <tier> <date>
        line = grant_canonical_asm(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
        if line is None:
            print("grant REFUSED (tier not STRONG-class and not owner-cluster-enumerated — evidence gate failed)")
            sys.exit(1)
        print(line)
    elif cmd == "island-count":
        # island-count <root> <func> <stem>  -> "<n> yes|no"  (n = non-cop2-whitelist
        # multi-insn __asm__ islands in the applied body; yes = already allowlisted)
        n, listed = island_count(sys.argv[2], sys.argv[3], sys.argv[4])
        print(f"{n} {'yes' if listed else 'no'}")
    elif cmd == "log-borderline":
        # log-borderline <root> <func> <category> <evidence> <disposition> <date>
        log_borderline(sys.argv[2], sys.argv[3], sys.argv[4],
                       sys.argv[5], sys.argv[6], sys.argv[7])
    elif cmd == "unban":
        # unban <root> <func> <needle>  -> prints number of entries removed
        print(unban_construct(sys.argv[2], sys.argv[3], sys.argv[4]))
    elif cmd == "supersede-bans":
        # supersede-bans <root> <func> <superseded_by> <needle> [needles...]
        print(len(supersede_bans(sys.argv[2], sys.argv[3], sys.argv[5:], sys.argv[4])))
    elif cmd == "add-scope-allow":
        # add-scope-allow <root> <func> <date> <path> [more paths...]
        line = add_scope_allow(sys.argv[2], sys.argv[3], sys.argv[5:], sys.argv[4])
        if line is None:
            print("scope grant REFUSED (path class or denylist — "
                  "integration-handoff-self-serve)")
            sys.exit(1)
        print(line)
    elif cmd == "rule-scopes":
        # rule-scopes <root> <func> -> prints the CURRENT SCOPE block (empty if none cited)
        print(render_rule_scopes(cited_rule_scopes(sys.argv[2], sys.argv[3])))
    elif cmd == "siblings":
        # siblings <root> <func> -> the SIBLING LEDGERS block + pending progress
        # notices, exactly as the next brief would carry them (read-only)
        print(render_siblings(sibling_ledgers(sys.argv[2], sys.argv[3]), sys.argv[3]))
        print(render_sibling_progress(load_state(sys.argv[2], sys.argv[3]) or {}))
    else:
        print(f"unknown cmd {cmd}")
        sys.exit(2)
