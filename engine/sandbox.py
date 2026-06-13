"""Cheat-invisible scoring sandbox — the orchestrator's BASELINE/scoring step.

sandbox_score(func, disable) builds the function's containing file with that
function's cheat rules removed (via cheats.make_overrides) into a per-function
workspace, then scores it against the canonical cheat-on .o. The result is the
honest, cheat-free codegen distance: re-adding a cheat cannot improve it because
the orchestrator owns the filtered config the sandbox compiles against — the
agent only edits C.
"""
from __future__ import annotations

import functools
from pathlib import Path

from . import cheats
from . import fixtures as fx
from . import pipeline
from . import score


@functools.lru_cache(maxsize=1)
def _file_index_cached() -> dict[str, str]:
    return fx._file_index()


def func_file(func: str) -> str:
    stem = _file_index_cached().get(func)
    if stem is None:
        raise KeyError(f"{func}: not found in any build/src/*.o (run `engine build` first)")
    return stem


def sandbox_score(func: str, disable: str = "lost-codegen",
                  strip_cheat_asm: bool = False, workdir: str = "tmp/sandbox") -> dict:
    stem = func_file(func)
    wd = Path(workdir) / func
    if disable == "all" and strip_cheat_asm:
        # Honest pure-C distance: cheat-asm is stripped file-wide, which shifts
        # maspsx indices for EVERY function — so any sibling's index-based regfix
        # (reorder/subst/insert) misapplies on the shifted asm and can emit
        # malformed text that crashes assembly (confirmed: text1a_c.c's
        # efc_rob_set_type_flash positional prologue-swap produced `lh
        # $sp,$sp,88`, failing the whole-file build before the scored function
        # could be extracted). Per-function codegen is independent, so dropping
        # ALL rules file-wide leaves the SCORED function's bytes identical while
        # letting the file assemble. Mirrors build_stripped_object's empty-config
        # recipe — the honest pure-C distance has no rules anywhere by definition.
        ov = cheats.empty_overrides(str(wd / "cfg"))
        ov["dropped"] = sum(len(cheats.func_rule_lines(func, c))
                            for c in (cheats.REGFIX, cheats.REGFIX2, cheats.ASMFIX))
    else:
        ov = cheats.make_overrides(func, disable, str(wd / "cfg"))
    cheat_asm_stripped = 0
    if strip_cheat_asm:
        from . import inlineasm
        src_ovr = str(wd / "src" / f"{stem}.c")
        cheat_asm_stripped = inlineasm.write_stripped(stem, src_ovr)
        ov["src_override"] = src_ovr
    disabled_o = str(wd / f"{stem}.o")
    pipeline.build_c_object(stem, disabled_o, cheat_overrides=ov)
    reference_o = f"build/src/{stem}.o"
    if not Path(reference_o).exists():
        raise FileNotFoundError(f"{reference_o} missing — run `engine build` first")
    try:
        res = score.score_func(disabled_o, reference_o, func)
        res["scorable"] = True
    except KeyError as e:
        # The function is ABSENT from the cheat-disabled object: the whole-file
        # build got truncated — typically a SIBLING function's index-based regfix
        # `reorder` rule crashed the pipeline after cheat-asm stripping shifted
        # maspsx indices (see .claude/rules/jtbl-rodata-split-infrastructure.md).
        # Return a clean unscorable result instead of a traceback so callers
        # (CLI, canonical, the headless worker) can fall back to structural
        # analysis.
        res = {"score": None, "scorable": False, "target_insns": None,
               "build_insns": None,
               "error": (f"score unavailable: {e}. Cheat-disabled build of {stem}.c "
                         f"is missing {func} (pipeline likely truncated by a sibling "
                         f"index-based reorder rule after cheat-asm strip).")}
    res.update(func=func, file=stem, disable=disable, strip_cheat_asm=strip_cheat_asm,
               rules_dropped=ov["dropped"], cheat_asm_stripped=cheat_asm_stripped,
               disabled_o=disabled_o)
    return res


def build_stripped_object(stem: str, out_o: str, cfgdir: str,
                          strip_cheat_asm: bool = True) -> dict:
    """Build <stem>.o once with EVERY regfix/asmfix rule removed (empty configs)
    and, when strip_cheat_asm, all cheat-asm pins/inline-asm stripped too.
    Because both regfix rules and cheat asm are function-local, one whole-file
    build gives each function the bytes it would have with only its own crutches
    removed — so a single object scores every function in the file. This is the
    shared honest-distance recipe used by scan-redundant (difficulty) and
    diagnose; with strip_cheat_asm it is the true pure-C COMPLETED-C distance,
    without it the regfix-only distance `retire` verifies (source pins stay).
    Returns the overrides dict."""
    ov = cheats.empty_overrides(cfgdir)
    if strip_cheat_asm:
        from . import inlineasm
        src_ovr = str(Path(cfgdir).parent / "src" / f"{stem}.c")
        inlineasm.write_stripped(stem, src_ovr)
        ov["src_override"] = src_ovr
    pipeline.build_c_object(stem, out_o, cheat_overrides=ov)
    return ov
