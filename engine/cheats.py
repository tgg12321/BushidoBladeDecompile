"""Cheat-rule introspection + filtered-config generation for the sandbox.

Every regfix/asmfix rule is a single line keyed `funcname:` (verified — even
splice/subst_multi are one line). The cheat-invisible sandbox builds a file with
a function's cheat rules removed, so the score reflects honest cheat-free
codegen. Re-adding a cheat can't help the score because the ORCHESTRATOR owns
the (frozen, filtered) config the sandbox compiles against — the agent only
edits C.

`disable` modes:
  "all"          drop every rule keyed by the function — the COMPLETED-C
                 zero-rules target (honest pure-C distance).
  "lost-codegen" drop only the lost-codegen insert rules (insert/insert_after of
                 `addu $X,_,$zero|$0`) — the specific unauthorized asm-injection
                 the cheat-cleanup queue targets, leaving other rules in place.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

REGFIX = "regfix.txt"
REGFIX2 = "regfix_stage2.txt"
ASMFIX = "asmfix.txt"
INLINE_ASM_CANONICAL = "inline_asm_canonical.txt"

# prologue_fix per-function configs. A function with an entry here relies on
# prologue_fix reordering cc1's OWN emitted prologue insns (or altering its
# frame size / delay slot) into the target order — a per-function cheat the
# cheat-invisible sandbox must STRIP, exactly like a regfix rule, so the honest
# pure-C distance is real and a prologue-reordered function cannot pass as
# COMPLETED-C (audit 2026-06-15: prologue_fix is a cheat by any spelling).
PROLOGUE_CONFIG = "tools/prologue_config.json"
DELAY_SLOT_RA = "tools/delay_slot_ra_funcs.txt"
FRAME_FIX = "tools/frame_fix_funcs.txt"

# Per-function maspsx gate lists — assembler-behavior toggles keyed by function
# name, adjudicated 2026-07-13 (.claude/rules/maspsx-gate-lists.md). They are
# part of the CANONICAL build (engine/buildconfig.py), NOT sandbox-stripped:
# the fidelity-class behaviors model the ORIGINAL toolchain (stripping them
# would score against a wrong toolchain model). Completion must still be
# transparent about them, and cheat-pathway lists must never gate a C function
# (a pure-C spelling exists for those effects — the honest fix is the C).
#   fidelity      — no C spelling can produce the effect; the gate restores
#                   original-assembler behavior at oracle-verified sites.
#   cheat-pathway — a C spelling EXISTS (e.g. unsigned types for multu);
#                   gating a C function through it is a cheat by config.
MASPSX_GATE_LISTS: dict[str, str] = {
    "maspsx_label_nop_funcs.txt": "fidelity",      # ASPSX load-delay nop; fork .L-prefix blind spot
    "expand_lb_funcs.txt": "fidelity",             # lb->lbu+sll+sra; all C spellings fold to lb (probe 2026-07-13)
    "expand_dest_funcs.txt": "fidelity",           # $rdest-vs-$at macro-expansion temp; assembler-internal choice
    "multu_funcs.txt": "cheat-pathway",            # mult->multu; C spelling exists (unsigned operands)
    "multu_pad_funcs.txt": "cheat-pathway",        # injected nops between mult/mflo; bytes from config
}


def maspsx_gate_entries(func: str) -> list[tuple[str, str]]:
    """(list_file, classification) for every maspsx gate list that names
    `func`. Empty list == the function's bytes have no per-function assembler
    toggle behind them. Names are the first whitespace token per non-comment
    line (same format as the other one-name-per-line sidecars)."""
    out: list[tuple[str, str]] = []
    for path, cls in MASPSX_GATE_LISTS.items():
        if func in _prologue_txt_funcs(path):
            out.append((path, cls))
    return out


def canonical_asm_funcs(path: str = INLINE_ASM_CANONICAL) -> set[str]:
    """Functions authorized as COMPLETED-INLINE-ASM-CANONICAL
    (inline_asm_canonical.txt). Their inline asm IS the accepted finished form,
    so the COMPLETED gate exempts THEM (and only them) from the 'zero cheat-asm'
    bar. Name is the first whitespace-delimited token; blank/`#` lines ignored."""
    p = Path(path)
    if not p.exists():
        return set()
    out: set[str] = set()
    for ln in p.read_text(encoding="utf-8").splitlines():
        ln = ln.strip()
        if ln and not ln.startswith("#"):
            out.add(ln.split()[0])
    return out

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
    function cannot reach zero-rules COMPLETED-C without a project-wide rodata
    reorder, so the queue routes it to `authorize` instead of handing it to a
    worker."""
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
    return [(i, ln) for i, ln in enumerate(Path(cfg).read_text(encoding="utf-8").splitlines())
            if kr.match(ln)]


def _filter_text(func: str, disable: str, cfg: str) -> tuple[str, int]:
    if not Path(cfg).exists():
        return "", 0
    kr = _key_re(func)
    out, dropped = [], 0
    for ln in Path(cfg).read_text(encoding="utf-8").splitlines(keepends=True):
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
            for ln in Path(c).read_text(encoding="utf-8").splitlines():
                m = keyre.match(ln)
                if m:
                    funcs.add(m.group(1))
    return funcs


def _prologue_txt_funcs(path: str) -> set[str]:
    """Function names in a one-name-per-line prologue list (delay_slot_ra /
    frame_fix); first whitespace token, '#'/blank lines ignored."""
    p = Path(path)
    if not p.exists():
        return set()
    out: set[str] = set()
    for ln in p.read_text(encoding="utf-8").splitlines():
        tok = ln.strip().split()
        if tok and not tok[0].startswith("#"):
            out.add(tok[0])
    return out


def prologue_fix_funcs() -> set[str]:
    """Every function that relies on prologue_fix (a prologue_config.json entry,
    a delay_slot_ra entry, or a frame_fix entry) — each is a tracked cheat."""
    pc = Path(PROLOGUE_CONFIG)
    cfg = json.loads(pc.read_text()) if pc.exists() else {}
    return set(cfg.keys()) | _prologue_txt_funcs(DELAY_SLOT_RA) | _prologue_txt_funcs(FRAME_FIX)


def func_prologue_count(func: str) -> int:
    """Number of prologue_fix entries keyed to `func` across the 3 configs
    (0..3). Nonzero == the function carries a prologue_fix cheat."""
    pc = Path(PROLOGUE_CONFIG)
    cfg = json.loads(pc.read_text()) if pc.exists() else {}
    n = 1 if func in cfg else 0
    n += 1 if func in _prologue_txt_funcs(DELAY_SLOT_RA) else 0
    n += 1 if func in _prologue_txt_funcs(FRAME_FIX) else 0
    return n


def _write_prologue_overrides(sd: Path, drop_func: str | None) -> dict:
    """Write FILTERED prologue configs into sd so the sandbox build strips
    prologue_fix. drop_func=None -> remove ALL entries (empty: prologue_fix is a
    no-op for the whole file); a name -> remove only that function's entries.
    Returns {prologue_config_path, delay_slot_ra_path, frame_fix_path,
    dropped_prologue}."""
    sd = Path(sd)
    sd.mkdir(parents=True, exist_ok=True)
    out: dict = {}
    dropped = 0
    pc = Path(PROLOGUE_CONFIG)
    cfg = json.loads(pc.read_text()) if pc.exists() else {}
    if drop_func is None:
        dropped += len(cfg)
        cfg = {}
    elif drop_func in cfg:
        del cfg[drop_func]
        dropped += 1
    (sd / "prologue_config.json").write_text(json.dumps(cfg))
    out["prologue_config_path"] = str(sd / "prologue_config.json")
    for label, src in (("delay_slot_ra_path", DELAY_SLOT_RA), ("frame_fix_path", FRAME_FIX)):
        sp = Path(src)
        lines = sp.read_text(encoding="utf-8").splitlines(keepends=True) if sp.exists() else []
        kept = []
        for ln in lines:
            tok = ln.strip().split()
            name = tok[0] if (tok and not tok[0].startswith("#")) else None
            if name and (drop_func is None or name == drop_func):
                dropped += 1
                continue
            kept.append(ln)
        (sd / Path(src).name).write_text("".join(kept))
        out[label] = str(sd / Path(src).name)
    out["dropped_prologue"] = dropped
    return out


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
    # Strip prologue_fix for the whole file too (empty configs) so the honest
    # pure-C distance excludes the prologue reorder/frame/delay cheat.
    ov.update(_write_prologue_overrides(sd, None))
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
    # The honest pure-C ("all") mode also strips this function's prologue_fix
    # entry; "lost-codegen" is regfix-scoped and leaves prologue_fix untouched.
    if disable == "all":
        ov.update(_write_prologue_overrides(sd, func))
    return ov
