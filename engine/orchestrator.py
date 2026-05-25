"""Deterministic controller — Phase 2a: autonomous redundant-rule retirement.

scan_file(stem): ONE build with all rules stripped, then score every keyed
function in the file. distance 0 == that function's rules are redundant (the C
already compiles to the right bytes without them). Because regfix rules are
function-scoped, the all-stripped build gives each function the same bytes it
would have with only its own rules removed — so one build scores the whole file.

retire_redundant(funcs): for each distance-0 candidate, integrate.retire_function
deletes its rules, rebuilds, and confirms the full-build SHA1 still matches. The
integrated SHA1 is the real gate; a sandbox false-zero cannot land a broken
build (it rolls back).

This is the no-model first rung. Model-driven gap-closing (nonzero distances) is
a later layer.
"""
from __future__ import annotations

from pathlib import Path

from . import cheats
from . import pipeline as P
from . import score


def scan_file(stem: str, workdir: str = "tmp/scan", rebuild: bool = True) -> dict:
    """{func: {redundant, difficulty}} for the keyed functions DEFINED IN this
    file's object (using its own symbol table — no ambiguous name->file index).

    From one all-rules-stripped build: `redundant` is EXACT byte-identity (the
    sound dead-rule test); `difficulty` is the masked instruction distance for
    ranking non-redundant functions (forced >=1 so a masked false-zero never
    masquerades as redundant).
    """
    ref_o = f"build/src/{stem}.o"
    if not Path(ref_o).exists():
        return {}
    defined = set(score._o_func_table(ref_o).keys())
    funcs = sorted(defined & cheats.all_keyed_functions())
    if not funcs:
        return {}
    stripped_o = f"{workdir}/{stem}/{stem}.o"
    if rebuild or not Path(stripped_o).exists():
        ov = cheats.empty_overrides(f"{workdir}/{stem}/cfg")
        P.build_c_object(stem, stripped_o, cheat_overrides=ov)
    out = {}
    for func in funcs:
        try:
            redundant = score.is_redundant(stripped_o, ref_o, func)
            difficulty = 0 if redundant else max(
                1, score.score_func(stripped_o, ref_o, func)["score"])
        except KeyError:
            continue
        out[func] = {"redundant": redundant, "difficulty": difficulty}
    return out


def scan_all(rebuild: bool = True) -> dict:
    res = {}
    for stem in P.c_stems():
        per = scan_file(stem, rebuild=rebuild)
        if per:
            res[stem] = per
    return res


def redundant_in(scan_result: dict) -> list[str]:
    """Functions whose rules are exact-byte redundant, flattened + sorted."""
    out = []
    for per_func in scan_result.values():
        out += [f for f, v in per_func.items() if v["redundant"]]
    return sorted(out)


def retire_redundant(funcs: list[str]) -> list[dict]:
    return [integrate.retire_function(f) for f in funcs]
