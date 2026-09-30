"""Layer-2 review record and the `queue done` gate (owner ruling Q39, 2026-09-29).

Q39, verbatim: "Add a hard gate so a function can't be marked done (`queue
done`) without a recorded second-review PASS for the exact code being landed?"
— owner chose "Yes, add the gate" (docs/grind/owner-rulings-2026-09-26.md
§ Q39). The 2026-09-29 retro-audit found 23 of 98 landings with no recorded
layer-2 PASS, and PASSes given on an earlier body than the one landed; the
verdicts lived only as free text in ledgers.

THE RECORD: `memory/grind/<func>/layer2.jsonl`, append-only, one JSON object
per line, committed with the landing:

  {"func", "verdict": PASS|FAIL|NEEDS_USER, "body_hash", "body_kind": c|asm,
   "file", "reviewer", "scope", "date", "head", "notes"}

Written by `python3 -m engine.cli layer2 record <func> --verdict ...
--expect-hash <h>`. `--expect-hash` is REQUIRED: <h> is the hash the reviewer
reported (`layer2 hash <func>`) for the body it ruled on, and the record is
refused unless the body now in src/ hashes to it — so a PASS on body A can
never be written against a body B edited in afterwards. The LAST line is the
function's standing verdict.

THE GATE (`gate`): `queue done` — and regen's mechanical drop of an item it
previously listed — requires that last line to be PASS with body_hash equal to
the current body's hash. A FAIL/NEEDS_USER after a PASS revokes it; any code
change after the review breaks the hash. No override flag exists (none is
authorized by any owner ruling).

THE HASH is sha1[:16] of the function's FULL definition (storage class and
return type through the closing brace, as located by
inlineasm._func_body_span) reduced to its C token sequence: comments dropped,
string/char literals kept verbatim, tokens joined by one space. A comment or
layout edit keeps the hash; any token change — `s16` vs `s32`, `static`, a
space inside a string, `a - --b` vs `a-- - b` — moves it. Deliberately NOT
grindlib.body_hash, which collapses whitespace inside strings, strips `//`
inside strings, merges `- --` into `---`, omits the return type, and keys
K&R definitions by the whole file (layer-2 review of 5d46a66e1). A function
supplied wholly from asm (canonical INCLUDE_ASM / `.include` / `glabel`
block) is keyed by its asm block text plus the included .s file.
"""
from __future__ import annotations

import datetime
import hashlib
import json
import re
import subprocess
from pathlib import Path

from . import inlineasm

RECORD_NAME = "layer2.jsonl"
VERDICTS = ("PASS", "FAIL", "NEEDS_USER")
# The completion-class commit kinds review-discipline-before-commit lists, plus
# the grinder's Judge FINAL CALL (judge-sole-gate: the Judge is the acceptance
# gate for autonomous work).
SCOPES = ("match", "cheat-cleanup", "auth", "grinder-final-call")


# One C token per match, maximal munch; `tokens` drops the `skip` group
# (whitespace, comments). A string/char literal is ONE token, so nothing inside
# it is normalized.
_TOKEN_RE = re.compile(r"""
    (?P<skip> \s+ | /\*.*?\*/ | //[^\n]* )
  | "(?:\\.|[^"\\\n])*"
  | '(?:\\.|[^'\\\n])*'
  | \.?[0-9](?:[eEpP][+-]|[A-Za-z0-9_.])*
  | [A-Za-z_]\w*
  | <<= | >>= | \.\.\. | -> | \+\+ | -- | << | >> | <= | >= | == | != | && | \|\|
  | [-+*/%&|^]= | \#\#
  | .
""", re.S | re.X)


def tokens(text: str) -> list[str]:
    """`text` as C tokens: comments and whitespace dropped, literals verbatim."""
    return [m.group(0) for m in _TOKEN_RE.finditer(text or "") if m.group("skip") is None]


def _key(text: str) -> str:
    return hashlib.sha1(" ".join(tokens(text)).encode("utf-8")).hexdigest()[:16]


def record_path(func: str) -> Path:
    return Path("memory/grind") / func / RECORD_NAME


def _read_text(p: Path) -> str | None:
    try:
        return p.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None


def _asm_pieces(text: str, func: str) -> list[str]:
    """Source text that supplies `func` wholly from asm: its INCLUDE_ASM line,
    every `__asm__` block naming it (`glabel func` or `.include .../func.s`),
    and the contents of each included .s file (a missing file hashes as a
    marker, so it can never alias a present one)."""
    pieces = []
    for m in inlineasm._INCLUDE_ASM_MACRO_RE.finditer(text):
        if m.group(1) != func:
            continue
        pieces.append(m.group(0))
        folder = re.search(r'"([^"]*)"', m.group(0)).group(1)
        inc = Path(folder) / f"{func}.s"
        pieces.append(_read_text(inc) or f"<missing {inc.as_posix()}>")
    glabel = re.compile(r"\bglabel\s+" + re.escape(func) + r"\b")
    include = re.compile(r'\.include\s+\\?"([^"\\]*?/' + re.escape(func) + r'\.s)\\?"')
    for kw in inlineasm.cia.find_asm_keywords(text):
        if kw.directive or kw.end < 0:
            continue
        incs = include.findall(kw.body)
        if not incs and not glabel.search(kw.body):
            continue
        pieces.append(text[kw.start:kw.end + 1])
        for rel in incs:
            pieces.append(_read_text(Path(rel)) or f"<missing {rel}>")
    return pieces


def body_key(text: str, func: str) -> tuple[str, str] | None:
    """(kind, 16-hex hash) of `func`'s body in the C file `text`, or None when
    no body can be located (fail closed: nothing to key a verdict to)."""
    span = inlineasm._func_body_span(text, func)
    if span is not None:
        # A definition sharing a line with the previous function's `}` starts
        # its span at that brace; it is not part of this function.
        return "c", _key(re.sub(r"^\}", "", text[span[0]:span[1]]))
    pieces = _asm_pieces(text, func)
    if pieces:
        return "asm", _key("\n".join(pieces))
    return None


def current_key(func: str, stem: str) -> tuple[str, str] | None:
    text = _read_text(Path(f"src/{stem}.c"))
    return None if text is None else body_key(text, func)


def locate_stem(func: str) -> str | None:
    """src/<stem>.c holding `func`'s body: the queue item's file, else a scan."""
    try:
        q = json.loads(Path("engine/queue.json").read_text())
        for it in q.get("items", []):
            if it.get("func") == func and it.get("file"):
                return it["file"]
    except (OSError, ValueError):
        pass
    for p in sorted(Path("src").glob("*.c")):
        text = _read_text(p)
        if text and func in text and body_key(text, func) is not None:
            return p.stem
    return None


def read_records(func: str) -> list[dict]:
    """Every record for `func`, oldest first. Raises ValueError on a malformed
    line — a record the gate cannot read must never read as absent."""
    p = record_path(func)
    text = _read_text(p) if p.exists() else ""
    if text is None:
        raise ValueError(f"{p} is unreadable")
    out = []
    for n, line in enumerate(text.splitlines(), 1):
        if not line.strip():
            continue
        try:
            rec = json.loads(line)
        except ValueError as e:
            raise ValueError(f"{p}:{n} is not JSON ({e})") from None
        if (not isinstance(rec, dict) or rec.get("verdict") not in VERDICTS
                or not rec.get("body_hash") or rec.get("func") != func):
            raise ValueError(f"{p}:{n} is not a {func} layer-2 record")
        out.append(rec)
    return out


def _head() -> str:
    try:
        return subprocess.run(["git", "rev-parse", "--short", "HEAD"], capture_output=True,
                              text=True, check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return ""


def record(func: str, verdict: str, reviewer: str, scope: str, notes: str = "",
           stem: str | None = None, expect_hash: str | None = None) -> dict:
    """Append one verdict for the body the reviewer reported (`expect_hash`),
    refused unless that is the body now in src/."""
    if verdict not in VERDICTS:
        return {"ok": False, "func": func, "reason": f"verdict must be one of {VERDICTS}"}
    if scope not in SCOPES:
        return {"ok": False, "func": func, "reason": f"scope must be one of {SCOPES}"}
    if not reviewer.strip():
        return {"ok": False, "func": func, "reason": "reviewer is required"}
    if not expect_hash:
        return {"ok": False, "func": func,
                "reason": ("--expect-hash is required: the `layer2 hash` the reviewer reported "
                           "for the body it ruled on (a verdict bound to whatever is in src/ at "
                           "record time is not a verdict on the landed body — owner ruling Q39)")}
    stem = stem or locate_stem(func)
    key = current_key(func, stem) if stem else None
    if key is None:
        return {"ok": False, "func": func,
                "reason": f"no body for {func} found in src/{stem or '*'}.c — nothing to key the verdict to"}
    kind, h = key
    if expect_hash != h:
        return {"ok": False, "func": func,
                "reason": (f"the body in src/{stem}.c hashes {h}, not the reviewed {expect_hash} — "
                           f"the reviewer ruled on a different body; nothing recorded")}
    try:
        read_records(func)
    except ValueError as e:
        return {"ok": False, "func": func, "reason": f"existing record is malformed: {e}"}
    rec = {"func": func, "verdict": verdict, "body_hash": h, "body_kind": kind,
           "file": stem, "reviewer": reviewer.strip(), "scope": scope,
           "date": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
           "head": _head(), "notes": notes}
    p = record_path(func)
    p.parent.mkdir(parents=True, exist_ok=True)
    with open(p, "a", encoding="utf-8", newline="\n") as fh:
        fh.write(json.dumps(rec, ensure_ascii=False) + "\n")
    return {"ok": True, "path": p.as_posix(), **rec}


def gate(func: str, stem: str) -> str | None:
    """None when the latest layer-2 record for `func` is a PASS on the current
    body; otherwise the refusal reason, naming the fix."""
    fix = (f"Fix: spawn a fresh cheat-reviewer (layer 2) on the exact body being landed; on PASS "
           f"run `python3 -m engine.cli layer2 record {func} --verdict PASS --reviewer <id> "
           f"--scope <{'|'.join(SCOPES)}> --expect-hash <the reviewer's `layer2 hash`> "
           f"--notes \"...\"` and commit memory/grind/{func}/"
           f"{RECORD_NAME} with the landing (owner ruling Q39, "
           f".claude/rules/review-discipline-before-commit.md).")
    key = current_key(func, stem)
    if key is None:
        return (f"layer-2 gate: no body for {func} found in src/{stem}.c, so no reviewed body "
                f"can match it. {fix}")
    _kind, h = key
    try:
        recs = read_records(func)
    except ValueError as e:
        return f"layer-2 gate: {e}. Repair the record (it is append-only history). {fix}"
    if not recs:
        return f"layer-2 gate: no layer-2 verdict is recorded for {func} (current body {h}). {fix}"
    last = recs[-1]
    if last["verdict"] != "PASS":
        return (f"layer-2 gate: the latest layer-2 verdict for {func} is {last['verdict']} "
                f"({last.get('date', '?')}, body {last['body_hash']}); a later PASS on the "
                f"current body {h} is required. {fix}")
    if last["body_hash"] != h:
        return (f"layer-2 gate: the latest layer-2 PASS for {func} ({last.get('date', '?')}) "
                f"covered body {last['body_hash']}, but the body now in src/{stem}.c hashes {h} "
                f"— the code changed after review (comment/whitespace edits keep the hash). {fix}")
    return None
