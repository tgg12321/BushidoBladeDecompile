#!/usr/bin/env python3
"""Campaign 4 conversion helper — retire an `asmfix.txt` `replace_with_asmfile`
rule by representing the function's asm body in source with `INCLUDE_ASM`.

The build-time substitution and the source-level `INCLUDE_ASM` emit the SAME
file (`asm/funcs/<func>.s`) verbatim, so this changes no byte's provenance. What
changes is that the source stops compiling a fake C body and silently discarding
it. See docs/superpowers/specs/2026-08-06-tu-resplit-campaign.md.

Per function it does three things, in place so the emitted text address is
unchanged:
  1. replace the stub DEFINITION with `extern <signature>;` + `INCLUDE_ASM(...)`
  2. delete the `<func>: replace_with_asmfile "..."` line from asmfix.txt
  3. (with --ledger) save the removed body to memory/grind/<func>/ first

Usage:
    python3 tools/resplit_convert.py --wave 2                  # show the diff
    python3 tools/resplit_convert.py --wave 2 --apply
    python3 tools/resplit_convert.py --func func_80017A44 --apply
    python3 tools/resplit_convert.py --tu display --apply --ledger

Writes LF unconditionally: src/*.c and asmfix.txt are build inputs and CRLF
silently breaks the GNU toolchain (AGENTS.md).
"""
from __future__ import annotations

import argparse
import difflib
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))

from engine import inlineasm  # noqa: E402  (reuses the ONE body-span impl)

MAP_PATH = ROOT / "tmp/resplit_map.json"
ASMFIX = ROOT / "asmfix.txt"


def write_lf(path: Path, text: str) -> None:
    with open(path, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(text)


def load_map() -> dict:
    if not MAP_PATH.exists():
        sys.exit(f"{MAP_PATH} missing — run tmp/resplit_scope.py && "
                 f"tmp/build_resplit_map.py first")
    return json.loads(MAP_PATH.read_text(encoding="utf-8"))


def targets(m: dict, wave: int | None, tu: str | None,
            func: str | None) -> list[tuple[str, str]]:
    """-> [(stem, func), ...] in source order per TU."""
    out = []
    for stem, t in m["tus"].items():
        if tu and stem != tu:
            continue
        if wave is not None and t.get("wave") != wave:
            continue
        for f in t["funcs"]:
            if func and f["func"] != func:
                continue
            out.append((stem, f["func"]))
    if func and not out:
        sys.exit(f"{func}: not a conversion target in tmp/resplit_map.json")
    return out


# Mnemonics whose FIRST operand is not a destination.
_NO_DEST = re.compile(r'^(s[bhwd]|sw[lr]|s[wd]c[0-9]|b\w*|j|jal|jr|jalr|'
                      r'mult|multu|div|divu|mt(hi|lo|c[0-9]|)|teq|break|nop|'
                      r'ctc[0-9]|cop[0-9])$')
_CALLER_SAVED = {f"$a{i}" for i in range(4)} | {f"$v{i}" for i in range(2)} | \
                {f"$t{i}" for i in range(10)} | {"$ra", "$at"}
_INSN = re.compile(r'^\s*(?:/\*.*?\*/)?\s*(\.?\w+)\s*(.*?)\s*$')


def asm_incoming_args(func: str) -> set[str]:
    """Argument registers `asm/funcs/<func>.s` READS BEFORE WRITING — i.e. the
    function's actual incoming arguments, read off the code rather than trusted
    from a stub that nothing ever checked.

    Conservative in the direction that matters: after a `jal` every caller-saved
    register is treated as written, so a post-call read is never miscounted as an
    incoming argument.
    """
    p = ROOT / f"asm/funcs/{func}.s"
    if not p.exists():
        return set()
    written: set[str] = set()
    incoming: set[str] = set()
    for raw in p.read_text(encoding="utf-8").splitlines():
        line = re.sub(r'/\*.*?\*/', ' ', raw).strip()
        if not line or line.startswith((".", "glabel", "endlabel")) or line.endswith(":"):
            continue
        m = _INSN.match(line)
        if not m:
            continue
        mn, rest = m.group(1), m.group(2)
        ops = [o.strip() for o in rest.split(",") if o.strip()]
        regs = [re.findall(r'\$\w+', o) for o in ops]
        dest = None if _NO_DEST.match(mn) else (regs[0][0] if regs and regs[0] else None)
        srcs = [r for i, rl in enumerate(regs) for r in rl if not (i == 0 and dest)]
        for r in srcs:
            if r.startswith("$a") and r[2:].isdigit() and r not in written:
                incoming.add(r)
        if dest:
            written.add(dest)
        if mn in ("jal", "jalr"):
            written |= _CALLER_SAVED
    return incoming


def has_declaration(text: str, func: str) -> bool:
    """True when `text` already declares `func` (a `... func(...) ;` at line
    start) — src/ings.c declares func_80017A44 on the line after its
    definition."""
    for m in re.finditer(rf'(?m)^[A-Za-z_][\w \t\*]*\b{re.escape(func)}\s*\(', text):
        i = inlineasm._match_paren(text, m.end() - 1)
        if i < 0:
            continue
        j = i
        while j < len(text) and text[j] in " \t\n\r":
            j += 1
        if j < len(text) and text[j] == ";":
            return True
    return False


def convert_source(text: str, func: str) -> tuple[str, str]:
    """-> (new text, removed body). Raises rather than guess.

    NEVER emits a declaration derived from the stub. The stub's signature is
    FICTION — it was written to make a placeholder compile, and the build threw
    it away, so nothing ever checked it. Laundering it into an `extern` would
    convert an obviously-fake body into the canonical C form for ASSERTING a
    signature, which is the campaign's own defect one layer up. Measured case
    (layer-2, Wave 2): src/config.c's stub claimed `s32 func_8003FA24(void *a0)`
    while asm/funcs/func_8003FA24.s reads $a0/$a1/$a2 into $s2/$s5/$s1 before
    defining them (three incoming args) and include/m2c_context.h:524 declares
    `void func_8003FA24(GameObj *, s32, s32)` — wrong arity AND wrong return.

    So: if the name is referenced after the conversion point and the file does
    not already carry a declaration, ABORT and let a human supply a verified
    one. Guessing is what this campaign exists to stop.
    """
    span = inlineasm._func_body_span(text, func)
    if span is None:
        raise SystemExit(f"{func}: no C definition found — already converted?")
    lo, hi = span
    # _func_body_span's `^(?:\}[ \t]*)?` prefix means the span can START at the
    # PREVIOUS function's closing brace (the `}s32 func_8007DE08(...) {` shape in
    # display.c, a campaign TU). Deleting that would delete the previous
    # function's body terminator.
    while lo < hi and text[lo] in "} \t":
        lo += 1
    body = text[lo:hi]
    if "INCLUDE_ASM" in body:
        raise SystemExit(f"{func}: already an INCLUDE_ASM")

    rest = text[:lo] + text[hi:]
    decl = ""
    if not has_declaration(rest, func):
        # Only a reference in REAL CODE needs a declaration — the name also
        # appears in `/* kengo:... */` provenance comments next to several
        # stubs, which must not trigger this.
        masked = inlineasm._code_without_comments_and_strings(text)
        m = re.search(rf'\b{re.escape(func)}\b', masked[hi:])
        if m:
            line = text[:hi + m.start()].count("\n") + 1
            # A caller FOLLOWS, so it was already compiled against this
            # definition's signature. Re-stating that signature PRESERVES the
            # typing the TU already had — it is not a new assertion. (Emitting
            # one where no caller follows WOULD be: that is the func_8003FA24
            # case layer-2 caught, where the only caller preceded the
            # definition and the stub's `s32 f(void *)` was simply wrong.)
            head = re.sub(r"\s+", " ", body.split("{", 1)[0]).strip()
            if "//" in head or ";" in head:
                raise SystemExit(
                    f"{func}: definition header is not a simple prototype "
                    f"(K&R parameters or a trailing comment): {head!r}. "
                    f"Refusing to transform it — hand-convert this one.")
            params = head[head.find("("):] if "(" in head else ""
            nparams = (0 if re.fullmatch(r'\(\s*(void)?\s*\)', params)
                       else params.count(",") + 1)
            incoming = len(asm_incoming_args(func))
            # Cross-check the preserved signature against the code. A mismatch
            # means the in-file caller is already calling with the wrong arity;
            # do not bake that into a declaration silently.
            if nparams != incoming:
                raise SystemExit(
                    f"{func}: called at line {line} AFTER the definition, so its "
                    f"signature must be preserved — but it CONTRADICTS the asm.\n"
                    f"  definition header : {head}   ({nparams} parameter(s))\n"
                    f"  asm reads         : {sorted(asm_incoming_args(func)) or 'no argument registers'}\n"
                    f"Hand-verify against asm/funcs/{func}.s and "
                    f"include/m2c_context.h, fix the declaration, then re-run.")
            decl = f"{head};\n"
    return text[:lo] + decl + f'INCLUDE_ASM("asm/funcs", {func});' + text[hi:], body


def drop_asmfix_rule(text: str, func: str) -> str:
    pat = re.compile(rf'(?m)^{re.escape(func)}:\s*replace_with_asmfile\s+"[^"]+"\s*\n')
    new, n = pat.subn("", text)
    if n != 1:
        raise SystemExit(f"{func}: expected exactly 1 replace_with_asmfile rule, found {n}")
    return new


def ledger(func: str, body: str) -> Path:
    """Preserve a substantial stub before it is replaced (campaign R2: the body
    is a real prior decomp attempt and is the resume point)."""
    d = ROOT / "memory/grind" / func
    d.mkdir(parents=True, exist_ok=True)
    p = d / "pre-include-asm-body.c"
    # Describe what this actually IS. The campaign plan (R2) calls these bodies
    # "a real prior decomp attempt ... the resume point", which is true of some
    # of them and false of a `{ (void)a0; return 0; }` placeholder — and
    # asserting it blindly is the same genre of unchecked claim the campaign
    # exists to remove. Classify instead of assert.
    stripped = [l.strip() for l in body.split("{", 1)[-1].splitlines() if l.strip()]
    meat = [l for l in stripped
            if not re.fullmatch(r'\(void\)\s*\w+\s*;|return\s+[-\w]+\s*;|\}', l)]
    kind = ("a real prior decomp attempt — the resume point when this function "
            "is decompiled" if meat else
            "a PLACEHOLDER (no decompiled logic), kept only for provenance")
    write_lf(p, (
        f"/* {func} — C body removed by Campaign 4 (INCLUDE_ASM conversion).\n"
        f" * The build DISCARDED this body (asmfix replace_with_asmfile substituted\n"
        f" * asm/funcs/{func}.s verbatim), so it never reached the binary.\n"
        f" * This body is {kind}.\n"
        f" * NOTE: its signature was never checked by anything — do not trust it.\n"
        f" * Campaign plan R2.\n"
        f" */\n{body}\n"))
    return p


def main() -> int:
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--wave", type=int)
    g.add_argument("--tu")
    g.add_argument("--func")
    ap.add_argument("--apply", action="store_true", help="write; default is diff only")
    ap.add_argument("--ledger", action="store_true",
                    help="save each removed body to memory/grind/<func>/ first")
    a = ap.parse_args()

    m = load_map()
    todo = targets(m, a.wave, a.tu, a.func)
    if not todo:
        sys.exit("no targets matched")

    by_stem: dict[str, list[str]] = {}
    for stem, func in todo:
        by_stem.setdefault(stem, []).append(func)

    # Compute EVERYTHING before writing anything: a failure on a later function
    # must not leave earlier sources converted while their replace_with_asmfile
    # rules are still live in asmfix.txt (which would silently double-emit).
    asmfix_text = ASMFIX.read_text(encoding="utf-8")
    pending: list[tuple[Path, str, str, list[str]]] = []
    ledgered = []
    for stem, funcs in sorted(by_stem.items()):
        src = ROOT / f"src/{stem}.c"
        text = orig = src.read_text(encoding="utf-8")
        for func in funcs:
            text, body = convert_source(text, func)
            asmfix_text = drop_asmfix_rule(asmfix_text, func)
            if a.ledger and len(body.splitlines()) > 3:
                ledgered.append((func, body))
        pending.append((src, orig, text, funcs))

    if not a.apply:
        for src, orig, text, _funcs in pending:
            sys.stdout.writelines(difflib.unified_diff(
                orig.splitlines(True), text.splitlines(True),
                f"a/{src.relative_to(ROOT)}", f"b/{src.relative_to(ROOT)}"))
        print(f"\n# asmfix.txt: would drop {len(todo)} replace_with_asmfile rule(s)")
        for _stem, func in todo:
            print(f"#   {func}")
        return 0

    for func, body in ledgered:
        print(f"ledgered: {ledger(func, body).relative_to(ROOT)}")
    for src, _orig, text, funcs in pending:
        write_lf(src, text)
        print(f"applied: {src.relative_to(ROOT)} ({len(funcs)} function(s))")
    write_lf(ASMFIX, asmfix_text)
    rules = len(re.findall(r'(?m)^[A-Za-z_]\w*:', asmfix_text))
    print(f"applied: asmfix.txt — {len(todo)} rule(s) dropped, {rules} rule lines left")
    return 0


if __name__ == "__main__":
    sys.exit(main())
