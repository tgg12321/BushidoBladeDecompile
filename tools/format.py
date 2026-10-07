#!/usr/bin/env python3
"""format.py [--check] [PATH ...]: format the C sources with the repo's .clang-format.

Scope: tracked `src/**/*.c`, `src/**/*.h` and `include/**/*.h` (or the PATHs given).
Style: `.clang-format` at the repo root (SOTN's, with `s32 *p` pointers and the
token-changing options off). clang-format is pinned in requirements.txt; another
version formats differently, so a mismatch refuses.

Two things clang-format does not decide:
  - Inline asm statements (`__asm__` / `__asm`) keep their hand layout: each is
    copied back from the input after clang-format runs, re-indented with the
    statement. They mirror PsyQ's inline_c.h macro text.
  - A file is written only when its C token stream is unchanged
    (engine.layer2.tokens: whitespace and comments dropped, literals verbatim),
    so formatting can never change what the compiler sees, or a layer-2 key.

--check writes nothing and exits 1 if any file would change.
--check-staged checks the index's copies (tools/hooks/format_guard.py).
--stdin-batch formats a JSON {path: text} batch (format_blobs; tools/codex_worker.py).
Runs in WSL (the repo .venv); started on Windows it re-runs itself there.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CLANG_FORMAT_VERSION = "18.1.8"
SUFFIXES = (".c", ".h")


def _rerun_in_wsl(argv: list[str]) -> int:
    drive, rest = os.path.splitdrive(str(ROOT))
    wsl_root = f"/mnt/{drive[0].lower()}{rest.replace(os.sep, '/')}"
    rel = [os.path.relpath(a, ROOT) if os.path.isabs(a) else a for a in argv]
    args = " ".join(shlex.quote(a.replace("\\", "/")) for a in rel)
    cmd = f"cd {shlex.quote(wsl_root)} && .venv/bin/python3 tools/format.py {args}"
    return subprocess.run(["wsl", "-e", "bash", "-c", cmd]).returncode


def _import_paths() -> None:
    """engine/ and tools/ importable; only on the formatting side, so an importer
    (tools/codex_worker.py) keeps its own sys.path."""
    for p in (str(ROOT / "tools"), str(ROOT)):
        if p not in sys.path:
            sys.path.insert(0, p)


def _clang_format() -> str:
    exe = ROOT / ".venv/bin/clang-format"
    if not exe.exists():
        sys.exit(f"format.py: {exe} missing; install it: .venv/bin/pip install -r requirements.txt")
    out = subprocess.run([str(exe), "--version"], capture_output=True, text=True).stdout
    if CLANG_FORMAT_VERSION not in out:
        sys.exit(f"format.py: clang-format {CLANG_FORMAT_VERSION} required, found: {out.strip()}")
    return str(exe)


def _line_indent(text: str, pos: int) -> str | None:
    """Leading whitespace of pos's line, or None when pos is not the line's
    first non-blank character."""
    start = text.rfind("\n", 0, pos) + 1
    lead = text[start:pos]
    return lead if not lead.strip() else None


def restore_asm(orig: str, fmt: str) -> str:
    """`fmt` with every inline asm statement's text taken from `orig`."""
    _import_paths()
    from classify_inline_asm import STATEMENT_SPELLINGS, find_asm_keywords

    ko = find_asm_keywords(orig, STATEMENT_SPELLINGS)
    kf = find_asm_keywords(fmt, STATEMENT_SPELLINGS)
    if len(ko) != len(kf):
        raise ValueError(f"asm statement count changed ({len(ko)} -> {len(kf)})")
    out = fmt
    for a, b in reversed(list(zip(ko, kf))):
        if a.end < 0 or b.end < 0:
            continue  # no operand list (a macro argument): leave clang-format's text
        region = orig[a.start:a.end]
        old_ind, new_ind = _line_indent(orig, a.start), _line_indent(fmt, b.start)
        if old_ind is not None and new_ind is not None and old_ind != new_ind:
            lines = region.split("\n")
            for i in range(1, len(lines)):
                if lines[i].startswith(old_ind):
                    lines[i] = new_ind + lines[i][len(old_ind):]
            region = "\n".join(lines)
        out = out[:b.start] + region + out[b.end:]
    return out


COLUMN_LIMIT = 80
_STARTS_STATEMENT = (";", "{", "}", "*/", ":")


def _asm_spans(text: str) -> list[tuple[int, int, int, int]]:
    """(start offset, end offset, first line, last line) of every asm statement."""
    _import_paths()
    from classify_inline_asm import STATEMENT_SPELLINGS, find_asm_keywords
    return [(k.start, k.end, text.count("\n", 0, k.start), text.count("\n", 0, k.end))
            for k in find_asm_keywords(text, STATEMENT_SPELLINGS) if k.end >= 0]


def _comment_starts(line_text: str, in_comment: bool):
    """(trailing comment column or -1, still-in-comment at end of line) for one
    line; string and char literals are skipped."""
    i, n, trailing = 0, len(line_text), -1
    while i < n:
        if in_comment:
            j = line_text.find("*/", i)
            if j < 0:
                return trailing, True
            in_comment, i = False, j + 2
            continue
        c = line_text[i]
        if c in "\"'":
            j = i + 1
            while j < n and line_text[j] != c:
                j += 2 if line_text[j] == "\\" else 1
            i = j + 1
        elif line_text.startswith("//", i):
            if trailing < 0 and line_text[:i].strip():
                trailing = i
            return trailing, False
        elif line_text.startswith("/*", i):
            if trailing < 0 and line_text[:i].strip():
                trailing = i
            in_comment, i = True, i + 2
        else:
            i += 1
    return trailing, in_comment


def hoist_trailing_comments(text: str) -> str:
    """Move a trailing comment that does not fit beside its statement (it wraps,
    or the line passes the column limit) onto its own lines above the statement.
    clang-format otherwise splits the declaration to make room, or breaks a
    macro. A comment after an asm statement goes above the statement's first
    line. Left in place: comments inside asm statements, macro continuation
    lines, lines that continue an earlier line or close a block (`} while ...;`,
    `} Name;`: above would be inside the block)."""
    lines = text.split("\n")
    starts = [0]
    for ln in lines[:-1]:
        starts.append(starts[-1] + len(ln) + 1)
    spans = _asm_spans(text)
    out: list[str] = []
    out_pos: list[int] = []  # out index of each input line's first output line
    in_comment = False
    i = 0
    while i < len(lines):
        out_pos.append(len(out))
        line = lines[i]
        col, still = _comment_starts(line, in_comment)
        start_in_comment = in_comment
        in_comment = still
        end = i
        if col >= 0:
            while end < len(lines) - 1 and in_comment:
                end += 1
                _, in_comment = _comment_starts(lines[end], True)
        for _ in range(i + 1, end + 1):
            out_pos.append(len(out))  # keeps out_pos indexed by input line
        off = starts[i] + max(col, 0)
        inside_asm = any(s <= off < e for s, e, _, _ in spans)
        asm_tail = next((sl for s, e, sl, el in spans if el == i and e <= off and sl < i), None)
        head = asm_tail if asm_tail is not None else i  # the statement's first line
        prev = next((l.rstrip() for l in reversed(out[:out_pos[head]]) if l.strip()), "")
        code = line[:max(col, 0)].rstrip()
        movable = (col >= 0 and not start_in_comment and not in_comment and not inside_asm
                   and (end > i or len(line) > COLUMN_LIMIT)
                   and not prev.endswith("\\") and not lines[end].rstrip().endswith("\\")
                   and not (code.lstrip().startswith("}") and not code.endswith("{"))
                   and (not prev or prev.endswith(_STARTS_STATEMENT) or prev.lstrip().startswith("#")))
        if not movable:
            out.extend(lines[i:end + 1])
            i = end + 1
            continue
        hl = lines[head]
        indent = hl[:len(hl) - len(hl.lstrip())]
        if line.startswith("//", col):
            tail_end = len(lines[end])
        elif end == i:
            tail_end = line.find("*/", col + 2) + 2
        else:
            tail_end = lines[end].find("*/") + 2
        after = lines[end][tail_end:].strip()
        if end == i:
            body = [line[col:tail_end]]
        else:
            body = [line[col:].rstrip()] + lines[i + 1:end] + [lines[end][:tail_end]]
        code += " " + after if after else ""
        # A comment on a `{` line labels the construct that opens the block (`do {`,
        # `for (...) {`), so it goes above that line, except after a `}` (`} else {`),
        # where above would be inside the previous block: then it opens the new one.
        opens = code.endswith("{") and code.lstrip().startswith("}")
        cind = indent + "    " if opens else indent
        comment = [cind + body[0]]
        for b in body[1:]:
            s = b.strip()
            comment.append(cind + (" " + s if s.startswith("*") else "   " + s))
        if opens:
            out.extend([code] + comment)
        elif asm_tail is not None:
            out[out_pos[head]:out_pos[head]] = comment
            out.append(code)
        else:
            out.extend(comment + [code])
        i = end + 1
    return "\n".join(out)


def _clang(exe: str, path: str, text: str) -> str:
    r = subprocess.run([exe, f"--style=file:{ROOT / '.clang-format'}", f"--assume-filename={path}"],
                       input=text.encode("utf-8"), capture_output=True, check=True)
    return r.stdout.decode("utf-8")


def format_text(exe: str, path: str, text: str) -> str:
    """clang-format to a fixed point (a hoisted comment can let a statement
    re-join, which can expose the next one), keeping asm statements' layout."""
    cur = text
    for _ in range(6):
        nxt = restore_asm(cur, _clang(exe, path, hoist_trailing_comments(cur)))
        if nxt == cur:
            return cur
        cur = nxt
    raise ValueError("formatting did not reach a fixed point")


def tracked_sources() -> list[str]:
    out = subprocess.run(["git", "ls-files", "--", "src", "include"], cwd=ROOT,
                         capture_output=True, text=True, check=True).stdout.split()
    return [p for p in out if p.endswith(SUFFIXES) and (p.startswith("src/") or p.endswith(".h"))]


_SPLICE = re.compile(r"\\\r?\n")


def same_tokens(a: str, b: str) -> bool:
    """Same C tokens, with and without backslash-newline splices applied first:
    layer2.tokens drops a splice between tokens, and cc1 joins `-\\<nl>-` into `--`."""
    _import_paths()
    from engine.layer2 import tokens
    return tokens(a) == tokens(b) and tokens(_SPLICE.sub("", a)) == tokens(_SPLICE.sub("", b))


def _format_checked(exe: str, path: str, text: str) -> tuple[str, str]:
    """(status, text): ("ok", formatted) / ("tokens", "") / ("error", message)."""
    try:
        new = format_text(exe, path, text)
    except (ValueError, subprocess.CalledProcessError) as exc:
        return "error", str(exc)
    return ("ok", new) if same_tokens(new, text) else ("tokens", "")


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--check", action="store_true", help="write nothing; exit 1 if a file would change")
    ap.add_argument("--stdin-batch", action="store_true",
                    help='format a JSON {path: text} object from stdin; write {path: [status, text]}')
    ap.add_argument("paths", nargs="*")
    a = ap.parse_args(argv)
    exe = _clang_format()
    if a.stdin_batch:
        batch = json.loads(sys.stdin.buffer.read().decode("utf-8"))
        with ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
            res = dict(zip(batch, pool.map(lambda p: _format_checked(exe, p, batch[p]), batch)))
        sys.stdout.buffer.write(json.dumps(res).encode("utf-8"))
        return 0
    paths = [p for p in a.paths if p.endswith(SUFFIXES)] if a.paths else tracked_sources()

    def one(p: str):
        try:
            text = (ROOT / p).read_bytes().decode("utf-8")
        except (OSError, UnicodeDecodeError) as exc:
            return p, "error", str(exc)
        status, new = _format_checked(exe, p, text)
        if status == "error":
            return p, "error", new
        if status == "tokens":
            return p, "error", "C tokens would change; not written"
        if new == text:
            return p, "clean", ""
        if not a.check:
            (ROOT / p).write_bytes(new.encode("utf-8"))
        return p, "changed", ""

    with ThreadPoolExecutor(max_workers=os.cpu_count() or 4) as pool:
        results = list(pool.map(one, paths))
    changed = [p for p, s, _ in results if s == "changed"]
    errors = [(p, m) for p, s, m in results if s == "error"]
    for p, m in errors:
        print(f"format.py: ERROR {p}: {m}", file=sys.stderr)
    verb = "would reformat" if a.check else "reformatted"
    for p in changed:
        print(f"{verb} {p}")
    print(f"format.py: {len(paths)} file(s), {len(changed)} {verb}, {len(errors)} error(s)")
    return 1 if errors or (a.check and changed) else 0


def format_blobs(texts: dict[str, str], env: dict | None = None) -> dict[str, tuple[str, str]]:
    """{path: text} -> {path: (status, text)} as _format_checked, from either side
    of WSL (one WSL process for the whole batch). `env`: the child environment
    (tools/codex_worker.py passes its clean_env())."""
    if not texts:
        return {}
    if os.name != "nt":
        exe = _clang_format()
        return {p: _format_checked(exe, p, t) for p, t in texts.items()}
    drive, rest = os.path.splitdrive(str(ROOT))
    wsl_root = f"/mnt/{drive[0].lower()}{rest.replace(os.sep, '/')}"
    cmd = f"cd {shlex.quote(wsl_root)} && .venv/bin/python3 tools/format.py --stdin-batch"
    r = subprocess.run(["wsl", "-e", "bash", "-c", cmd], input=json.dumps(texts).encode("utf-8"),
                       capture_output=True, env=env)
    if r.returncode != 0:
        msg = r.stderr.decode("utf-8", "replace").strip()
        return {p: ("error", f"format.py --stdin-batch: {msg}") for p in texts}
    return {p: tuple(v) for p, v in json.loads(r.stdout.decode("utf-8")).items()}


def check_staged() -> int:
    """The index's copy of every staged src/include C file is formatted. Reads the
    index on this side: git hands a hook its own GIT_INDEX_FILE (`commit -a`,
    `commit <paths>`), which does not cross into WSL."""
    staged = subprocess.run(["git", "diff", "--cached", "--name-only", "--diff-filter=ACMR", "-z"],
                            cwd=ROOT, capture_output=True, check=True).stdout.decode("utf-8").split("\0")
    texts, bad = {}, []
    for p in (p for p in staged if p.endswith(SUFFIXES) and p.startswith(("src/", "include/"))):
        try:
            texts[p] = subprocess.run(["git", "show", f":{p}"], cwd=ROOT, capture_output=True,
                                      check=True).stdout.decode("utf-8")
        except (UnicodeDecodeError, subprocess.CalledProcessError) as exc:
            bad.append(f"{p} (cannot read: {exc})")
    results = format_blobs(texts)
    for p in texts:
        status, new = results.get(p, ("error", "no result returned"))
        if status == "error":
            bad.append(f"{p} (cannot format: {new}; fix the source or use [skip-format])")
        elif status == "tokens":
            bad.append(f"{p} (formatting would change C tokens; use [skip-format])")
        elif new != texts[p]:
            bad.append(p)
    for p in bad:
        print(f"not formatted: {p}")
    return 1 if bad else 0


if __name__ == "__main__":
    if sys.argv[1:] == ["--check-staged"]:
        sys.exit(check_staged())
    if os.name == "nt":
        sys.exit(_rerun_in_wsl(sys.argv[1:]))
    sys.exit(main(sys.argv[1:]))
