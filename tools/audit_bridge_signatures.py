#!/usr/bin/env python3
"""Audit bridged-function stub signatures against caller / extern evidence.

WHY THIS MATTERS:
The round-6 trap on func_800693CC was: stub declared `void func_X(...)`
but caller in same file did `result = func_X();`. GCC dead-coded the
return-value load, shifting 76 bytes of binary and breaking sibling
matches.

The build currently matches SHA1, so no ACTIVE bugs. But the stubs
often disagree with the canonical signature inferred from extern
declarations and caller-site evidence. Future agents doing real decomp
on these functions need the correct signature — this audit produces it.

For each bridged function, this reports:
  - stub_signature: what the bridge currently says
  - canonical_signature: inferred from extern decls + caller usage
  - mismatch_kind: how they differ
  - severity: "active" (would break current build if no override),
              "latent" (overrode by local extern, but stub is misleading)
"""
from __future__ import annotations
import re
import json
from pathlib import Path
from collections import defaultdict
import sys

ROOT = Path(__file__).resolve().parent.parent
ASMFIX = ROOT / "asmfix.txt"
SRC_FILES = sorted((ROOT / "src").glob("*.c"))


def get_bridged_funcs():
    funcs = []
    for line in ASMFIX.read_text(encoding="utf-8").splitlines():
        s = line.strip()
        if s.startswith("#") or not s:
            continue
        m = re.match(r"^(\w+):\s+replace_with_asmfile\b", s)
        if m:
            funcs.append(m.group(1))
    return funcs


SOURCES = {src.name: src.read_text(encoding="utf-8", errors="ignore") for src in SRC_FILES}


def find_definition(func):
    """Return (filename, return_type, args_str) for the stub def."""
    pat = re.compile(
        rf"^\s*([a-zA-Z_][a-zA-Z0-9_ ]*?)\s*(\*+)?\s*{re.escape(func)}\s*\(([^)]*)\)\s*\{{",
        re.MULTILINE,
    )
    for name, content in SOURCES.items():
        for m in pat.finditer(content):
            return name, (m.group(1).strip() + (m.group(2) or "")), m.group(3).strip()
    return None


def find_externs(func):
    """Return list of (filename, line_no, return_type, args_str) for extern decls
    AND non-extern forward decls (e.g. `s32 func(s32);` mid-file)."""
    out = []
    pat = re.compile(
        rf"^\s*(?:extern\s+)?([a-zA-Z_][a-zA-Z0-9_ ]*?)\s*(\*+)?\s*{re.escape(func)}\s*\(([^)]*)\)\s*;"
    )
    # Filter: 'return' is a C statement keyword, not a return type
    BAD_RET = {"return", "if", "while", "for", "do", "switch", "case"}
    for name, content in SOURCES.items():
        for line_no, line in enumerate(content.splitlines(), 1):
            m = pat.match(line)
            if m:
                ret = m.group(1).strip()
                if ret.split()[-1] in BAD_RET if ret else False:
                    continue
                ret_full = ret + (m.group(2) or "")
                args = m.group(3).strip()
                out.append((name, line_no, ret_full, args))
    return out


RET_USED_RE = re.compile(
    r"(?:=|return|\?|\(|,|!|[<>=!]=|&&|\|\||[+\-*/&|^]|\[|\(\s*\w+\s*\))\s*$"
)


def find_callsites(func):
    out = []
    pat_call = re.compile(rf"\b{re.escape(func)}\s*\(")
    pat_def = re.compile(
        rf"^\s*[a-zA-Z_][a-zA-Z0-9_ ]*?\s*\**\s*{re.escape(func)}\s*\([^)]*\)\s*\{{"
    )
    pat_decl = re.compile(
        rf"^\s*(?:extern\s+)?[a-zA-Z_][a-zA-Z0-9_ ]*?\s*\**\s*{re.escape(func)}\s*\([^)]*\)\s*;"
    )
    for name, content in SOURCES.items():
        for line_no, line in enumerate(content.splitlines(), 1):
            if pat_def.match(line):
                continue
            if pat_decl.match(line):
                continue
            for m in pat_call.finditer(line):
                before = line[: m.start()]
                uses_return = bool(RET_USED_RE.search(before))
                rest = line[m.end():]
                depth = 1
                end = 0
                for i, c in enumerate(rest):
                    if c == "(":
                        depth += 1
                    elif c == ")":
                        depth -= 1
                        if depth == 0:
                            end = i
                            break
                args_str = rest[:end].strip()
                if not args_str:
                    n_args = 0
                else:
                    depth = 0
                    commas = 0
                    for c in args_str:
                        if c == "(":
                            depth += 1
                        elif c == ")":
                            depth -= 1
                        elif c == "," and depth == 0:
                            commas += 1
                    n_args = commas + 1
                out.append({
                    "file": name,
                    "line": line_no,
                    "text": line.strip()[:120],
                    "uses_return": uses_return,
                    "n_args": n_args,
                })
    return out


def count_params(args_str):
    s = args_str.strip()
    if not s or s == "void":
        return 0
    depth = 0
    commas = 0
    for c in s:
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
        elif c == "," and depth == 0:
            commas += 1
    return commas + 1


def is_unspecified_arity(args_str):
    """K&R-style empty parens `()` mean 'unspecified args' — any number legal.
    Varargs `...` (after at least one fixed param) accept any trailing args.
    Both should suppress arg-count mismatch flags."""
    s = args_str.strip()
    if s == "":
        return True
    if "..." in s:
        return True
    return False


def has_local_override(func, caller_file, caller_line, stub_ret, stub_params, externs):
    """Does the caller's file have a forward decl before the caller line
    that disagrees with the stub on return type OR arg count? GCC uses
    the most-recent visible decl at the call site, not the stub."""
    for ext_file, ext_line, ext_ret, ext_args in externs:
        if ext_file == caller_file and ext_line < caller_line:
            if ext_ret != stub_ret or count_params(ext_args) != stub_params:
                return f"{ext_ret} {func}({ext_args})"
    return None


def main():
    funcs = get_bridged_funcs()
    print(f"# Caller-signature audit: {len(funcs)} bridged functions")
    print()

    findings = []  # full per-func record
    summary = defaultdict(int)

    for func in funcs:
        defn = find_definition(func)
        if defn is None:
            summary["no_definition"] += 1
            continue
        stub_file, stub_ret, stub_args = defn
        stub_params = count_params(stub_args)

        externs = find_externs(func)
        callsites = find_callsites(func)

        # Infer canonical return type:
        # 1. Externs that disagree with stub_ret form a "candidate pool"
        # 2. If any callsite uses_return, prefer non-void
        ret_votes = defaultdict(int)
        for _, _, ext_ret, _ in externs:
            ret_votes[ext_ret] += 1
        non_void_caller = any(cs["uses_return"] for cs in callsites)
        inferred_ret = stub_ret  # default
        if non_void_caller and stub_ret == "void":
            # Pick most-common non-void extern ret, else "s32"
            non_void = {r: c for r, c in ret_votes.items() if r != "void"}
            if non_void:
                inferred_ret = max(non_void.items(), key=lambda x: x[1])[0]
            else:
                inferred_ret = "s32"

        # Infer canonical arg count: max of (stub_params, max_caller_argc, max_extern_argc)
        max_caller_args = max((cs["n_args"] for cs in callsites), default=0)
        max_extern_args = max((count_params(args) for _, _, _, args in externs), default=0)
        inferred_params = max(stub_params, max_caller_args, max_extern_args)

        # Classify severity
        issues = []
        if stub_ret == "void" and non_void_caller:
            first_use = next(cs for cs in callsites if cs["uses_return"])
            override = has_local_override(func, first_use["file"], first_use["line"], stub_ret, stub_params, externs)
            severity = "latent (local extern override present)" if override else "ACTIVE (no override found)"
            issues.append({
                "kind": "stub-void-but-caller-uses-return",
                "severity": severity,
                "override": override,
                "evidence": f"src/{first_use['file']}:{first_use['line']}: {first_use['text']}",
            })

        if max_caller_args > stub_params and not is_unspecified_arity(stub_args):
            offender = next(cs for cs in callsites if cs["n_args"] == max_caller_args)
            override = has_local_override(func, offender["file"], offender["line"], stub_ret, stub_params, externs)
            severity = "latent (local extern override present)" if override else "ACTIVE (no override found)"
            issues.append({
                "kind": f"caller-passes-{max_caller_args}-args-stub-declares-{stub_params}",
                "severity": severity,
                "evidence": f"src/{offender['file']}:{offender['line']}: {offender['text']}",
            })

        extern_ret_mismatch = [e for e in externs if e[2] != stub_ret]
        if extern_ret_mismatch:
            e = extern_ret_mismatch[0]
            issues.append({
                "kind": "stub-vs-extern-return-mismatch",
                "severity": "informational (stub signature contradicts callers' TU declaration)",
                "evidence": f"src/{e[0]}:{e[1]}: extern {e[2]} {func}({e[3]})",
            })

        if issues:
            findings.append({
                "func": func,
                "stub": f"src/{stub_file}: {stub_ret} {func}({stub_args})",
                "inferred_signature": f"{inferred_ret} {func}(... {inferred_params} params ...)",
                "issues": issues,
            })
            for it in issues:
                if "ACTIVE" in it["severity"]:
                    summary["ACTIVE issues"] += 1
                elif "latent" in it["severity"]:
                    summary["latent (suppressed by local extern)"] += 1
                else:
                    summary["informational"] += 1

    # Print summary
    print("## Summary")
    print(f"  bridged functions:                    {len(funcs)}")
    for k, v in summary.items():
        print(f"  {k:38s}  {v}")
    print()
    if findings:
        print("## Findings")
        for r in findings:
            print(f"\n### {r['func']}")
            print(f"    current stub:    {r['stub']}")
            print(f"    canonical hint:  {r['inferred_signature']}")
            for it in r["issues"]:
                print(f"    [{it['severity']}] {it['kind']}")
                print(f"        {it['evidence']}")

    # JSON output for future-agent tooling
    json_path = ROOT / "tmp" / "bridge_signature_audit.json"
    json_path.write_text(json.dumps(findings, indent=2))
    print(f"\n# JSON report written to {json_path.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
