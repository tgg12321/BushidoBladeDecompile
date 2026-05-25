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
