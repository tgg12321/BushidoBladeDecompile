"""Layer-2 review record and the `queue done` gate (owner ruling Q39, 2026-09-29).

Q39, verbatim: "Add a hard gate so a function can't be marked done (`queue
done`) without a recorded second-review PASS for the exact code being landed?"
— owner chose "Yes, add the gate" (docs/grind/owner-rulings-2026-09-26.md
§ Q39). The 2026-09-29 retro-audit found 23 of 98 landings with no recorded
layer-2 PASS, and PASSes given on an earlier body than the one landed; the
verdicts lived only as free text in ledgers.

THE RECORD: `memory/grind/<func>/layer2.jsonl`, append-only, one JSON object
per line, committed with the landing:

  {"func", "addr", "verdict": PASS|FAIL|NEEDS_USER, "body_hash",
   "body_kind": c|asm|"", "file", "reviewer", "scope", "date", "head", "notes"
   [, "verdict_file", "verdict_sha1"]}

`addr` is the function's VRAM address at record time (addr_of: its glabel
file, else the link map) — the key engine/departures.py matches records by,
since a rename never changes it; a function without one cannot be recorded.

Written by `python3 -m engine.cli layer2 record <func> ...` with the hash the
reviewer reported (`layer2 hash <func>` on the body it ruled on) — given as
`--expect-hash <h>` or read, with the verdict, from the reviewer's own JSON via
`--verdict-file` (whose path + sha1 are then recorded). The hash is REQUIRED.
A PASS is refused unless the body now in src/ hashes to it, so a PASS on body
A can never be written against a body B edited in afterwards. A FAIL or
NEEDS_USER binds to the reviewer's hash even when src/ has moved on: those
verdicts can only close the gate. The LAST line is the function's standing
verdict.

THE GATE (`gate`): `queue done` — and regen's drop of an item it previously
listed — requires that last line to be PASS with body_hash equal to the current
body's hash. Any later FAIL/NEEDS_USER revokes it; any change to the definition
breaks the hash. No override flag exists (none is authorized by any owner
ruling).

THE HASH is sha1[:16] of the function's definition (storage class and return
type through the closing brace, as located by inlineasm._func_body_span)
reduced to its C token sequence: comments dropped, string/char literals kept
verbatim, preprocessor lines newline-terminated, tokens joined by one space.
A comment or layout edit inside the definition keeps the hash; a token change
inside it moves it. SCOPE: the definition only — file-scope macros, typedefs,
globals, prototypes and helper functions the body depends on can change
without moving the key; the reviewer and the oracle cover those, not this
hash. A function with more than one definition in the file (e.g. an `#if 0`
copy), or with both a C definition and an asm body, has NO key (fail closed).
Deliberately NOT grindlib.body_hash, which collapses whitespace inside
strings, strips `//` inside strings, merges `- --` into `---`, omits the
return type, and keys K&R definitions by the whole file (layer-2 review of
5d46a66e1). A function supplied wholly from asm (canonical INCLUDE_ASM /
`.include` / `glabel` block) is keyed by its asm block text plus the included
.s file.
"""
from __future__ import annotations

import csv
import datetime
import hashlib
import io
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
_HASH_RE = re.compile(r"[0-9a-f]{16}")


# One C token per match, maximal munch. `splice` (backslash-newline) and `skip`
# (whitespace, comments) are dropped. A string/char literal is ONE token, so
# nothing inside it is normalized.
_TOKEN_RE = re.compile(r"""
    (?P<splice> \\\r?\n )
  | (?P<skip> \s+ | /\*.*?\*/ | //[^\n]* )
  | "(?:\\.|[^"\\\n])*"
  | '(?:\\.|[^'\\\n])*'
  | \.?[0-9](?:[eEpP][+-]|[A-Za-z0-9_.])*
  | [A-Za-z_]\w*
  | <<= | >>= | \.\.\. | -> | \+\+ | -- | << | >> | <= | >= | == | != | && | \|\|
  | [-+*/%&|^]= | \#\#
  | .
""", re.S | re.X)


def tokens(text: str) -> list[str]:
    """`text` as C tokens: comments and whitespace dropped, literals verbatim.
    A preprocessor line (first token `#`) ends with a "\\n" token, because a
    directive is newline-terminated: `#define A 1` + `x` differs from
    `#define A 1 x`."""
    text = text or ""
    out, in_pp = [], False
    for m in _TOKEN_RE.finditer(text):
        tok = m.group(0)
        if m.group("splice") is not None:
            continue
        if m.group("skip") is not None:
            if in_pp and "\n" in tok:
                out.append("\n")
                in_pp = False
            continue
        if (tok == "#" and not in_pp
                and not text[text.rfind("\n", 0, m.start()) + 1:m.start()].strip()):
            in_pp = True
        out.append(tok)
    return out


def _key(text: str) -> str:
    return hashlib.sha1(" ".join(tokens(text)).encode("utf-8")).hexdigest()[:16]


def record_path(func: str) -> Path:
    return Path("memory/grind") / func / RECORD_NAME


def completed_record_path(func: str) -> Path:
    """Where the record lives once the ledger is archived (the grinder's
    close-ledger step; manual ledgers archived under _completed/)."""
    return Path("memory/grind/_completed") / func / RECORD_NAME


def _read_text(p: Path) -> str | None:
    try:
        return p.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return None


def _disk(path: str) -> str | None:
    return _read_text(Path(path))


def _asm_pieces(text: str, func: str, read=_disk) -> list[str]:
    """Source text that supplies `func` wholly from asm: its INCLUDE_ASM line or
    BIOS_[ABC]_FUNCTION trampoline line,
    every `__asm__` block naming it (`glabel func` or `.include .../func.s`),
    and the contents of each included .s file, fetched by `read(path)` (the
    working tree by default; a git revision for the departures audit). A
    missing file hashes as a marker, so it can never alias a present one."""
    pieces = []
    for m in inlineasm._INCLUDE_ASM_MACRO_RE.finditer(text):
        if m.group(1) != func:
            continue
        pieces.append(m.group(0))
        folder = re.search(r'"([^"]*)"', m.group(0)).group(1)
        inc = Path(folder) / f"{func}.s"
        pieces.append(read(inc.as_posix()) or f"<missing {inc.as_posix()}>")
    for m in inlineasm._BIOS_MACRO_RE.finditer(text):
        if (m.group(1) or m.group(2)) == func:
            pieces.append(m.group(0))  # the macro line IS the whole body
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
            pieces.append(read(rel) or f"<missing {rel}>")
    return pieces


def body_source(text: str, func: str, read=_disk) -> tuple[str, str] | None:
    """(kind, source text) the key covers — the one C definition, or the asm
    that supplies the function — or None when there is no single body."""
    span = inlineasm._func_body_span(text, func)
    pieces = _asm_pieces(text, func, read)
    if span is not None:
        if pieces or inlineasm._func_body_span(text[span[1]:], func) is not None:
            return None  # a second definition, or C AND asm: ambiguous
        # A definition sharing a line with the previous function's `}` starts
        # its span at that brace; it is not part of this function.
        return "c", re.sub(r"^\}", "", text[span[0]:span[1]])
    if pieces:
        return "asm", "\n".join(pieces)
    return None


def body_key(text: str, func: str, read=_disk) -> tuple[str, str] | None:
    """(kind, 16-hex hash) of `func`'s body in the C file `text`, or None when
    there is no single body (fail closed: nothing to key a verdict to).
    `read(path)` supplies included .s files (default: the working tree)."""
    src = body_source(text, func, read)
    return None if src is None else (src[0], _key(src[1]))


_ASM_VADDR = re.compile(r"/\*\s*[0-9A-Fa-f]+\s+([0-9A-Fa-f]{8})\s+[0-9A-Fa-f]{8}\s*\*/")
_GLABEL_RE = re.compile(r"^\s*glabel\s+(\S+)", re.M)
_MAP_DEF = re.compile(r"^\s+0x0*([0-9A-Fa-f]{1,8})\s+([A-Za-z_]\w*)(?:\s*=.*)?\s*$", re.M)
ADDR_RE = re.compile(r"[0-9A-F]{8}")


def _asm_addr(text: str | None, func: str) -> str | None:
    g = _GLABEL_RE.search(text or "")
    m = _ASM_VADDR.search(text or "") if g and g.group(1) == func else None
    return m.group(1).upper() if m else None


# the tracked name -> address registries (so a fresh clone needs no build/):
# splat's / the linker's symbol files, then the naming census
SYMBOL_FILES = ("symbol_addrs.txt", "named_syms.txt", "undefined_funcs_auto.txt",
                "undefined_syms_auto.txt")
CENSUS = "docs/naming/function-names.csv"
_SYM_LINE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*0x([0-9A-Fa-f]{1,8})\s*;", re.M)
_AUTO_NAME = re.compile(r"^func_([0-9A-Fa-f]{8})$")


def _registry_addrs(read) -> dict[str, str]:
    """name -> address from the tracked registries, read through `read`."""
    out: dict[str, str] = {}
    for rel in SYMBOL_FILES:
        for m in _SYM_LINE.finditer(read(rel) or ""):
            out.setdefault(m.group(1), m.group(2).upper().zfill(8))
    text = read(CENSUS)
    if text:
        for row in csv.DictReader(io.StringIO(text)):
            a = (row.get("address") or "").strip().upper().replace("0X", "").zfill(8)
            if not ADDR_RE.fullmatch(a):
                continue
            for n in (row.get("current_name"), row.get("glabel"),
                      *(row.get("aliases") or "").split(";")):
                if n and n.strip():
                    out.setdefault(n.strip(), a)
    return out


def auto_addr(func: str) -> str | None:
    """A splat auto-name's own address (func_XXXXXXXX)."""
    m = _AUTO_NAME.match(func)
    return m.group(1).upper() if m else None


def addr_at(func: str, read) -> str | None:
    """`func`'s address from TRACKED files only, read through `read(path)` (the
    working tree, or a git revision) — the history-time subset of addr_of's
    precedence: its own glabel file, the symbol files, the census, a splat
    auto-name."""
    return (_asm_addr(read(f"asm/funcs/{func}.s"), func) or _registry_addrs(read).get(func)
            or auto_addr(func))


def addr_index() -> dict[str, str]:
    """name -> VRAM address (8 upper-case hex digits), with ONE precedence
    (addr_of uses the same): splat's asm/funcs glabel files (glabel -> the
    first machine column's vaddr, as docs/naming/build_census.py reads it),
    the tracked symbol files, the census, a splat auto-name's own address,
    then the link map build/bb2.map (definition AND `name = 0x...` lines).
    Names only an auto-name addresses are not listed: use lookup()."""
    out: dict[str, str] = {}
    for p in sorted(Path("asm/funcs").glob("*.s")):
        text = _read_text(p) or ""
        g = _GLABEL_RE.search(text)
        a = _asm_addr(text, g.group(1)) if g else None
        if a:
            out.setdefault(g.group(1), a)
    for n, a in _registry_addrs(_disk).items():
        out.setdefault(n, a)
    for m in _MAP_DEF.finditer(_read_text(Path("build/bb2.map")) or ""):
        out.setdefault(m.group(2), auto_addr(m.group(2)) or m.group(1).upper().zfill(8))
    return out


def lookup(index: dict[str, str], func: str) -> str | None:
    """`func`'s address from an addr_index() — or its auto-name's."""
    return index.get(func) or auto_addr(func)


def addr_of(func: str) -> str | None:
    """`func`'s VRAM address now (addr_index precedence)."""
    return lookup(addr_index(), func)


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
    from . import tus
    for tid in tus.src_tus():
        text = _read_text(Path(tus.src_path(tid)))
        if text and func in text and body_key(text, func) is not None:
            return tid
    return None


def read_records(func: str, path: Path | None = None) -> list[dict]:
    """Every record for `func`, oldest first. Raises ValueError on a malformed
    line — a record the gate cannot read must never read as absent."""
    p = path or record_path(func)
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


def read_verdict_file(path: str) -> dict:
    """The reviewer's JSON verdict (cheat-reviewer output schema): decision,
    function, body_hash, summary — plus the file's path and sha1 so the record
    is auditable back to it. Tolerates text around the JSON object (first `{`
    to last `}`). Raises ValueError when it is not a usable verdict."""
    try:
        raw = Path(path).read_bytes()
    except OSError as e:
        raise ValueError(f"cannot read verdict file {path}: {e}") from None
    text = raw.decode("utf-8", errors="replace")
    try:
        v = json.loads(text)
    except ValueError:
        i, j = text.find("{"), text.rfind("}")
        try:
            v = json.loads(text[i:j + 1]) if 0 <= i < j else None
        except ValueError:
            v = None
    if not isinstance(v, dict):
        raise ValueError(f"verdict file {path} holds no JSON object")
    if v.get("decision") not in VERDICTS:
        raise ValueError(f"verdict file {path}: decision must be one of {VERDICTS}")
    if not v.get("function"):
        raise ValueError(f"verdict file {path}: no `function`")
    if not v.get("body_hash"):
        raise ValueError(f"verdict file {path}: no `body_hash` (the reviewer must report "
                         f"`layer2 hash <func>` for the body it reviewed)")
    return {"verdict": v["decision"], "function": v["function"],
            "body_hash": str(v["body_hash"]), "summary": str(v.get("summary") or ""),
            "verdict_file": Path(path).as_posix(),
            "verdict_sha1": hashlib.sha1(raw).hexdigest(), "raw": raw}


VERDICTS_DIR = "layer2_verdicts"


def record_from_verdict_file(func: str, path: str, reviewer: str, scope: str,
                             notes: str = "", stem: str | None = None,
                             verdict: str | None = None,
                             expect_hash: str | None = None) -> dict:
    """`record` with the verdict and hash read from the reviewer's JSON. Any
    explicit --verdict / --expect-hash must agree with it. The JSON is copied
    byte-for-byte to memory/grind/<func>/layer2_verdicts/<sha1>.json, and the
    record points at that copy, so verdict_sha1 stays checkable after tmp/ is
    cleaned."""
    try:
        vf = read_verdict_file(path)
    except ValueError as e:
        return {"ok": False, "func": func, "reason": str(e)}
    clash = [f"{n} {mine!r} vs the file's {theirs!r}" for n, mine, theirs in (
        ("function", func, vf["function"]),
        ("--verdict", verdict or vf["verdict"], vf["verdict"]),
        ("--expect-hash", expect_hash or vf["body_hash"], vf["body_hash"])) if mine != theirs]
    if clash:
        return {"ok": False, "func": func,
                "reason": "verdict file disagrees: " + "; ".join(clash)}
    copy = record_path(func).parent / VERDICTS_DIR / f"{vf['verdict_sha1']}.json"
    existed = copy.exists()
    if not existed:
        copy.parent.mkdir(parents=True, exist_ok=True)
        copy.write_bytes(vf["raw"])
    r = record(func, vf["verdict"], reviewer, scope, notes or vf["summary"], stem=stem,
               expect_hash=vf["body_hash"],
               extra={"verdict_file": copy.as_posix(), "verdict_source": vf["verdict_file"],
                      "verdict_sha1": vf["verdict_sha1"]})
    if not r.get("ok") and not existed:
        copy.unlink()
        try:
            copy.parent.rmdir()
        except OSError:
            pass
    return r


def _head() -> str:
    try:
        return subprocess.run(["git", "rev-parse", "--short", "HEAD"], capture_output=True,
                              text=True, check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return ""


def record(func: str, verdict: str, reviewer: str, scope: str, notes: str = "",
           stem: str | None = None, expect_hash: str | None = None,
           extra: dict | None = None) -> dict:
    """Append one verdict for the body the reviewer reported (`expect_hash`).
    A PASS is refused unless src/ holds that body; a FAIL/NEEDS_USER is not."""
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
    if not _HASH_RE.fullmatch(expect_hash):
        return {"ok": False, "func": func,
                "reason": f"--expect-hash {expect_hash!r} is not a 16-hex `layer2 hash`"}
    stem = stem or locate_stem(func)
    key = current_key(func, stem) if stem else None
    if verdict == "PASS":
        if key is None:
            return {"ok": False, "func": func,
                    "reason": (f"no single body for {func} found in src/{stem or '*'}.c — "
                               f"nothing to key the PASS to")}
        if expect_hash != key[1]:
            return {"ok": False, "func": func,
                    "reason": (f"the body in src/{stem}.c hashes {key[1]}, not the reviewed "
                               f"{expect_hash} — the reviewer ruled on a different body; "
                               f"nothing recorded")}
    try:
        prior = read_records(func)
    except ValueError as e:
        return {"ok": False, "func": func, "reason": f"existing record is malformed: {e}"}
    addr = addr_of(func)
    if addr is None:
        return {"ok": False, "func": func,
                "reason": (f"no address for {func} (no glabel file, symbol-file or census "
                           f"entry, no build/bb2.map line) — a record must carry the "
                           f"function's address, which renames never change; add "
                           f"`{func} = 0x<ADDR>;` to symbol_addrs.txt")}
    # never earlier than ANY record already carrying this address — in any
    # ledger, archived ones included: the departures audit merges files by
    # date, so a clock that went back must not reorder this function's history
    fmt = "%Y-%m-%dT%H:%M:%SZ"
    now = datetime.datetime.now(datetime.timezone.utc).strftime(fmt)
    date = now
    others = [d for d in addr_dates(addr) if d]
    for d in others:
        try:
            datetime.datetime.strptime(d, fmt)
        except ValueError:
            return {"ok": False, "func": func,
                    "reason": (f"a record carrying {addr} has the effective date {d!r} "
                               f"(its ledger's running max), not a canonical {fmt} date — "
                               f"the audit orders records by date, so repair that ledger "
                               f"line before recording (nothing recorded)")}
    if others and max(others) >= now:
        # a record (in any ledger) is dated at or past now: go one second past
        # it, so a TIE is never broken by ledger path order in the audit
        try:
            date = (datetime.datetime.strptime(max(others), fmt)
                    + datetime.timedelta(seconds=1)).strftime(fmt)
        except OverflowError:
            return {"ok": False, "func": func,
                    "reason": (f"a record carrying {addr} is dated {max(others)!r}; no later "
                               f"date exists to stamp this one after it — repair that ledger "
                               f"line before recording (nothing recorded)")}
    date = max([date, *(str(r.get("date", "")) for r in prior)])
    rec = {"func": func, "addr": addr, "verdict": verdict, "body_hash": expect_hash,
           "body_kind": key[0] if key and key[1] == expect_hash else "",
           "file": stem, "reviewer": reviewer.strip(), "scope": scope,
           "date": date, "head": _head(), "notes": notes, **(extra or {})}
    p = record_path(func)
    p.parent.mkdir(parents=True, exist_ok=True)
    with open(p, "a", encoding="utf-8", newline="\n") as fh:
        fh.write(json.dumps(rec, ensure_ascii=False) + "\n")
    return {"ok": True, "path": p.as_posix(), **rec}


def addr_dates(addr: str) -> list[str]:
    """The EFFECTIVE dates of every record carrying `addr`, in every ledger
    (memory/grind/**/layer2.jsonl, _completed/ included) — each line's date
    raised to the running maximum of the dates before it in its own file,
    exactly the order engine/departures.py merges records by."""
    out = []
    for p in sorted(Path("memory/grind").glob("**/" + RECORD_NAME)):
        eff = ""
        for line in (_read_text(p) or "").splitlines():
            try:
                rec = json.loads(line)
            except ValueError:
                continue
            if not isinstance(rec, dict):
                continue
            eff = max(eff, str(rec.get("date", "")))
            if str(rec.get("addr", "")).upper() == addr:
                out.append(eff)
    return out


def gate(func: str, stem: str) -> str | None:
    """None when the latest layer-2 record for `func` is a PASS on the current
    body; otherwise the refusal reason, naming the fix."""
    fix = (f"Fix: spawn a fresh cheat-reviewer (layer 2) on the exact body being landed; on PASS "
           f"run `python3 -m engine.cli layer2 record {func} --reviewer <id> "
           f"--scope <{'|'.join(SCOPES)}> --verdict-file <its JSON verdict>` (or --verdict PASS "
           f"--expect-hash <its `layer2 hash`>) and commit memory/grind/{func}/"
           f"{RECORD_NAME} with the landing (owner ruling Q39, "
           f".claude/rules/review-discipline-before-commit.md).")
    key = current_key(func, stem)
    if key is None:
        return (f"layer-2 gate: no single body for {func} in src/{stem}.c (none, more than one "
                f"definition, or C and asm both), so no reviewed body can match it. {fix}")
    _kind, h = key
    try:
        recs = read_records(func)
    except ValueError as e:
        return f"layer-2 gate: {e}. Repair the record (it is append-only history). {fix}"
    if not recs:
        return f"layer-2 gate: no layer-2 verdict is recorded for {func} (current body {h}). {fix}"
    last = recs[-1]
    if not (isinstance(last.get("addr"), str) and ADDR_RE.fullmatch(last["addr"])):
        return (f"layer-2 gate: the latest layer-2 record for {func} carries no `addr` (it "
                f"predates address-keyed records) — the departures audit cannot match it to "
                f"the function. Record the verdict again with `layer2 record`. {fix}")
    now_addr = addr_of(func)
    if now_addr is None:
        return (f"layer-2 gate: {func} has no address now — regenerate the census or "
                f"rebuild (build/bb2.map), then retry. {fix}")
    if last["addr"] != now_addr:
        return (f"layer-2 gate: the latest layer-2 record for {func} carries addr "
                f"{last['addr']}, but {func} is at {now_addr} now — the record is not this "
                f"function's. Record the verdict again with `layer2 record`. {fix}")
    if last["verdict"] != "PASS":
        return (f"layer-2 gate: the latest layer-2 verdict for {func} is {last['verdict']} "
                f"({last.get('date', '?')}, body {last['body_hash']}); a later PASS on the "
                f"current body {h} is required. {fix}")
    if last["body_hash"] != h:
        return (f"layer-2 gate: the latest layer-2 PASS for {func} ({last.get('date', '?')}) "
                f"covered body {last['body_hash']}, but the body now in src/{stem}.c hashes {h} "
                f"— the code changed after review (comment/layout edits keep the hash). {fix}")
    return None
