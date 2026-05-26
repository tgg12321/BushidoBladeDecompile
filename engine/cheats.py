"""Cheat-rule introspection + filtered-config generation for the sandbox.

Every regfix/asmfix rule is a single line keyed `funcname:` (verified — even
splice/subst_multi are one line). The cheat-invisible sandbox builds a file with
a function's cheat rules removed, so the score reflects honest cheat-free
codegen. Re-adding a cheat can't help the score because the ORCHESTRATOR owns
the (frozen, filtered) config the sandbox compiles against — the agent only
edits C.

`disable` modes:
  "all"          drop every rule keyed by the function — the Tier-4 zero-rules
                 target (honest pure-C distance).
  "lost-codegen" drop only the lost-codegen insert rules (insert/insert_after of
                 `addu $X,_,$zero|$0`) — the specific unauthorized asm-injection
                 the cheat-cleanup queue targets, leaving other rules in place.
"""
from __future__ import annotations

import re
from pathlib import Path

REGFIX = "regfix.txt"
REGFIX2 = "regfix_stage2.txt"
ASMFIX = "asmfix.txt"

# A lost-codegen insert: insert/insert_after whose body is an `addu` that writes
# a register sourced from $zero/$0 — the instruction GCC's optimizer dropped
# (const-prop / dead-store). These are asm injection: bytes not from C.
_LCG_RE = re.compile(
    r'^\s*\S+\s*:\s*insert(?:_after)?\s+["\']addu[^"\']*(?:\$zero|\$0)[^"\']*["\']'
)


def _key_re(func: str) -> re.Pattern:
    return re.compile(rf"^{re.escape(func)}\s*:")


def is_lost_codegen(line: str) -> bool:
    return bool(_LCG_RE.match(line))


# Jump-table rodata-split infrastructure: a `switch` whose jump table splat
# carved into a separate asm/data rodata block (so it can't live in the C file's
# own .rodata without a global rodata reorder). The asmfix rules wire the
# function's table reference to the external `jtbl_<addr>` symbol and delete the
# duplicate table GCC emits. These are canonical infrastructure, NOT register/
# codegen cheats — see .claude/rules/jtbl-rodata-split-infrastructure.md.
_JTBL_RULE_TYPES = {"rename", "replace_first", "delete_between"}
_RULETYPE_RE = re.compile(r"^\S+\s*:\s*(\w+)")


def is_jtbl_infra(func: str, regfix: str = REGFIX, regfix2: str = REGFIX2,
                  asmfix: str = ASMFIX) -> bool:
    """True iff `func`'s rules are EXCLUSIVELY jump-table-infrastructure asmfix
    rules — every rule is rename/replace_first/delete_between, at least one
    references a `jtbl_` symbol, and there are ZERO regfix/regfix_stage2 rules
    (any regfix rule means a real register/codegen cheat is present). Such a
    function cannot reach zero-rules Tier-4 without a project-wide rodata reorder,
    so the queue routes it to `authorize` instead of handing it to a worker."""
    if func_rule_lines(func, regfix) or func_rule_lines(func, regfix2):
        return False
    asm = func_rule_lines(func, asmfix)
    if not asm:
        return False
    saw_jtbl = False
    for _i, ln in asm:
        m = _RULETYPE_RE.match(ln)
        if not m or m.group(1) not in _JTBL_RULE_TYPES:
            return False
        if "jtbl_" in ln:
            saw_jtbl = True
    return saw_jtbl


def func_rule_lines(func: str, cfg: str = REGFIX) -> list[tuple[int, str]]:
    """(0-based line index, line) for every rule keyed by `func` in `cfg`."""
    kr = _key_re(func)
    if not Path(cfg).exists():
        return []
    return [(i, ln) for i, ln in enumerate(Path(cfg).read_text().splitlines())
            if kr.match(ln)]


def _filter_text(func: str, disable: str, cfg: str) -> tuple[str, int]:
    if not Path(cfg).exists():
        return "", 0
    kr = _key_re(func)
    out, dropped = [], 0
    for ln in Path(cfg).read_text().splitlines(keepends=True):
        if kr.match(ln) and (disable == "all" or
                             (disable == "lost-codegen" and is_lost_codegen(ln))):
            dropped += 1
            continue
        out.append(ln)
    return "".join(out), dropped


def all_keyed_functions() -> set[str]:
    """Every symbol that keys at least one rule across the three configs."""
    keyre = re.compile(r"^([A-Za-z_]\w*)\s*:")
    funcs: set[str] = set()
    for c in (REGFIX, REGFIX2, ASMFIX):
        if Path(c).exists():
            for ln in Path(c).read_text().splitlines():
                m = keyre.match(ln)
                if m:
                    funcs.add(m.group(1))
    return funcs


def empty_overrides(sandbox_dir: str) -> dict:
    """Override dict pointing every config at an EMPTY file. Building a file
    this way disables ALL rules at once; since rules are function-scoped, each
    function's bytes then equal what it would be with only its own rules removed
    — so one build scores every function in the file for redundancy."""
    sd = Path(sandbox_dir)
    sd.mkdir(parents=True, exist_ok=True)
    ov = {"func": "<all>", "disable": "file-all"}
    for label, src in (("regfix_path", REGFIX), ("regfix2_path", REGFIX2),
                       ("asmfix_path", ASMFIX)):
        p = sd / Path(src).name
        p.write_text("")
        ov[label] = str(p)
    return ov


def make_overrides(func: str, disable: str, sandbox_dir: str) -> dict:
    """Write filtered copies of all three configs into sandbox_dir and return
    the cheat_overrides dict the pipeline consumes. `dropped_*` reports how many
    rules each filter removed (a sanity signal: 0 dropped == nothing disabled).
    """
    sd = Path(sandbox_dir)
    sd.mkdir(parents=True, exist_ok=True)
    ov = {"func": func, "disable": disable}
    for label, src, key in (("regfix_path", REGFIX, "regfix"),
                            ("regfix2_path", REGFIX2, "regfix2"),
                            ("asmfix_path", ASMFIX, "asmfix")):
        txt, dropped = _filter_text(func, disable, src)
        p = sd / Path(src).name
        p.write_text(txt)
        ov[label] = str(p)
        ov[f"dropped_{key}"] = dropped
    ov["dropped"] = ov["dropped_regfix"] + ov["dropped_regfix2"] + ov["dropped_asmfix"]
    return ov
