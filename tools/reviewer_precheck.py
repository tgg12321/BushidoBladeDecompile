#!/usr/bin/env python3
"""Mechanical pre-reviewer checklist — run BEFORE invoking cheat-reviewer.

Why (token-usage audit 2026-06-12): a layer-1 reviewer burned ~117k tokens
re-deriving procedural facts (allowlist coverage, rule counts, commit-field
presence) that are mechanically checkable. This tool computes those facts in
one call; paste its output into the reviewer brief so reviewer tokens go to
the SEMANTIC judgment (the 6-test checklist) only.

This is a BRIEFING AID, not a gate, and emphatically NOT a replacement for
the reviewer: per review-discipline-before-commit.md the detectors/checklists
"are a backstop, not the standard" — the semantic layer stays mandatory.

Usage (WSL, venv active):
  python3 tools/reviewer_precheck.py --func sys_VSync [--file ings2]
      [--staged | --commit <ref>] [--msg-file tmp/msg.txt] [--json]
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RULE_FILES = ["regfix.txt", "regfix_stage2.txt", "asmfix.txt"]


def git(*args: str) -> str:
    return subprocess.run(["git", *args], capture_output=True, text=True,
                          cwd=ROOT).stdout


def read_text_at_commit(rel_path: str, commit: str) -> str | None:
    """Read a repo-relative path's contents at a specific commit (via `git show
    <commit>:<path>`). Returns None if the path doesn't exist at that commit
    (git exits non-zero with empty stdout). Used so the worker-side precheck
    gate (lane.ps1) and any other caller passing --commit sees the candidate's
    file contents instead of MAIN's working tree (2026-06-22 bugfix — the
    previous behavior false-positived every worker submission because main
    still had the un-retired rules / pinned src that the worker had cleaned
    up in its worktree branch)."""
    r = subprocess.run(["git", "show", f"{commit}:{rel_path}"],
                       capture_output=True, text=True, cwd=ROOT)
    if r.returncode != 0:
        return None
    return r.stdout


def list_src_files_at_commit(commit: str) -> list[str]:
    """src/*.c files present at <commit> (repo-relative paths)."""
    r = subprocess.run(["git", "ls-tree", "-r", "--name-only", commit, "src/"],
                       capture_output=True, text=True, cwd=ROOT)
    if r.returncode != 0:
        return []
    return [l for l in r.stdout.splitlines() if l.endswith(".c")]


def rule_lines(func: str, commit: str | None = None) -> dict[str, list[str]]:
    """Count regfix/asmfix entries for `func`. When `commit` is given, read the
    files from that commit's tree (the candidate's state); otherwise read the
    working tree (legacy / standalone-CLI default)."""
    out: dict[str, list[str]] = {}
    pat = re.compile(rf"^{re.escape(func)}\s*:")
    for rf in RULE_FILES:
        text: str | None
        if commit:
            text = read_text_at_commit(rf, commit)
        else:
            p = ROOT / rf
            text = p.read_text(errors="replace") if p.exists() else None
        if text is None:
            continue
        hits = [f"{i}: {l.rstrip()}"
                for i, l in enumerate(text.splitlines(), 1)
                if pat.match(l)]
        if hits:
            out[rf] = hits
    return out


def diff_text(staged: bool, commit: str | None) -> str:
    if commit:
        return git("show", commit, "--format=")
    return git("diff", "--cached") if staged else git("diff", "HEAD")


def diff_files(staged: bool, commit: str | None) -> list[str]:
    if commit:
        out = git("show", commit, "--name-only", "--format=")
    else:
        out = git("diff", "--cached", "--name-only") if staged \
            else git("diff", "HEAD", "--name-only")
    return [l.strip() for l in out.splitlines() if l.strip()]


def function_body(func: str, file_stem: str | None,
                  commit: str | None = None) -> tuple[str, str] | None:
    """(path, body) for the function. When `commit` is given, read src/*.c via
    `git show <commit>:<path>` (the candidate's state); otherwise read the
    working tree. Bugfix 2026-06-22: previously always read working tree, which
    meant the worker-side precheck gate (lane.ps1) saw MAIN's still-pinned src
    instead of the worker's clean candidate — false-positive on every submit."""
    sys.path.insert(0, str(ROOT / "tools"))
    try:
        from sweep_variants import find_function_span  # reuse the span finder
    except ImportError:
        return None
    if commit:
        if file_stem:
            paths = [f"src/{file_stem}.c"]
        else:
            paths = list_src_files_at_commit(commit)
        for rel in paths:
            text = read_text_at_commit(rel, commit)
            if text is None:
                continue
            if re.search(rf"\b{re.escape(func)}\s*\(", text):
                try:
                    s, e = find_function_span(text, func)
                    return rel, text[s:e]
                except ValueError:
                    continue
        return None
    # working-tree path (CLI default; no --commit)
    candidates = ([ROOT / "src" / f"{file_stem}.c"] if file_stem
                  else sorted((ROOT / "src").glob("*.c")))
    for src in candidates:
        if not src.exists():
            continue
        text = src.read_text(errors="replace")
        if re.search(rf"\b{re.escape(func)}\s*\(", text):
            try:
                s, e = find_function_span(text, func)
                return str(src.relative_to(ROOT)), text[s:e]
            except ValueError:
                continue
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--func", required=True)
    ap.add_argument("--file", help="src stem (speeds up body location)")
    ap.add_argument("--staged", action="store_true",
                    help="audit the staged diff (default: working tree vs HEAD)")
    ap.add_argument("--commit", help="audit an existing commit instead")
    ap.add_argument("--msg-file", help="pending commit message file to field-check")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()

    report: dict = {"func": a.func, "checks": {}}
    ck = report["checks"]

    # 1. Residual rules — at the candidate's commit when --commit is given (the
    #    worker may have retired rules in its worktree branch that MAIN still
    #    has). 2026-06-22 fix.
    rules = rule_lines(a.func, a.commit)
    ck["rules"] = {"count": sum(len(v) for v in rules.values()), "hits": rules}

    # 2. Cheat-asm constructs inside the function body. Read from the candidate
    #    commit's src/ tree when --commit is given (same bugfix as #1).
    body = function_body(a.func, a.file, a.commit)
    if body:
        path, text = body
        pins = re.findall(r"register\s+\w[\w\s\*]*asm\s*\(\s*\"[^\"]+\"\s*\)", text)
        asms = re.findall(r"__asm__[^;{]*\(|\basm\s+volatile\s*\(", text)
        hard_n = re.findall(r"__asm__[^;]*\"[^\"]*\$\d+[^\"]*\"", text)
        ck["body"] = {"file": path, "register_asm_pins": len(pins),
                      "asm_blocks": len(asms),
                      "hardcoded_dollarN_asm": len(hard_n)}
    else:
        ck["body"] = {"file": None,
                      "note": "function body not found in working tree "
                              "(whole-body glabel asm or wrong --file?)"}

    # 3. Diff-scoped checks
    files = diff_files(a.staged, a.commit)
    dtext = diff_text(a.staged, a.commit)
    added = [l[1:] for l in dtext.splitlines()
             if l.startswith("+") and not l.startswith("+++")]

    rules_docs = [f for f in files if f.startswith(".claude/rules/")]
    ck["claude_rules_in_diff"] = rules_docs  # self-sanctioning risk if non-empty

    canon_adds = [l.strip() for l in added
                  if "inline_asm_canonical.txt" in " ".join(files)
                  and re.match(r"\s*func_|^\s*[A-Za-z_]\w+\s+#", l)] \
        if "inline_asm_canonical.txt" in files else []
    ck["canonical_registry_additions"] = canon_adds

    # 4. New `extern volatile` decls vs the allowlist
    new_vols = []
    for l in added:
        m = re.match(r"\s*extern\s+volatile\s+[\w\s\*]+?(\w+)\s*(;|\[|\s*asm)", l)
        if m:
            new_vols.append(m.group(1))
    allow = (ROOT / "volatile_extern_allowlist.txt")
    allow_syms = set()
    if allow.exists():
        for l in allow.read_text(errors="replace").splitlines():
            l = l.strip()
            if l and not l.startswith("#"):
                allow_syms.add(l.split()[0])
    ck["new_extern_volatile"] = [
        {"symbol": s, "allowlisted": s in allow_syms} for s in new_vols
    ]

    # 5. Commit-message carve-out fields (when a new volatile is in play)
    if a.msg_file and new_vols:
        msg = Path(a.msg_file).read_text(errors="replace") \
            if Path(a.msg_file).exists() else ""
        ck["msg_fields"] = {
            "irq_writer_line": bool(re.search(r"^IRQ writer:\s*\S+", msg, re.M)),
            "use_site_line": bool(re.search(r"^Use-site construct:\s*\S+", msg, re.M)),
        }

    # Verdict-ish summary (mechanical only — the reviewer makes the real call)
    flags = []
    if ck["rules"]["count"]:
        flags.append(f"{ck['rules']['count']} residual rule line(s)")
    if ck.get("body", {}).get("register_asm_pins"):
        flags.append("register-asm pins in body")
    if ck.get("body", {}).get("hardcoded_dollarN_asm"):
        flags.append("hardcoded-$N asm in body")
    if rules_docs:
        flags.append(".claude/rules/ doc rides along (self-sanctioning check!)")
    for v in ck["new_extern_volatile"]:
        if not v["allowlisted"]:
            flags.append(f"extern volatile {v['symbol']} NOT allowlisted")
    mf = ck.get("msg_fields")
    if mf and not all(mf.values()):
        flags.append("carve-out commit-msg fields incomplete")
    report["mechanical_flags"] = flags

    if a.json:
        print(json.dumps(report, indent=2))
    else:
        print(f"=== reviewer precheck: {a.func} ===")
        print(json.dumps(ck, indent=2))
        if flags:
            print("MECHANICAL FLAGS (resolve or explain in the reviewer brief):")
            for f in flags:
                print(f"  - {f}")
        else:
            print("mechanical checks clean — brief the reviewer on SEMANTICS only "
                  "(6-test checklist); cite this output so it skips re-derivation.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
