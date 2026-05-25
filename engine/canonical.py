"""Canonical-asm INTAKE gate — deterministic C-vs-asm classification BEFORE any
pure-C effort, so agents never (a) grind an impossible pure-C match or (b) reach
for asm injection to brute-force bytes. The decision is the orchestrator's, not
the agent's.

Definitive signals in the splat target disasm (asm/funcs/<func>.s):
  * cop2/GTE ops (mtc2/mfc2/ctc2/cfc2/cop2/lwc2/swc2 + GTE commands) — no C analog
  * splat's own `/* handwritten instruction */` annotation
  * syscall (BIOS trampoline)
  * .word (raw undisassembled instruction)

A function/region with these is AUTO-ROUTED to canonical-asm (no pure-C attempt).
Region-granular, so a partially-asm function gets pure C for the C parts and
inline-asm only for the genuinely-asm span. A function with NO definitive signal
is 'C' (the pure-C ladder); if that ladder plateaus, the controller PARKs with
evidence and escalates to a user canonical-asm call — that's the gate for the
heuristic/uncertain cases (auth model: auto-route definitive, gate the rest).
"""
from __future__ import annotations

import re
import subprocess
from pathlib import Path

from . import buildconfig as cfg

ASM_DIR = "asm/funcs"

# Unambiguous cop2/GTE mnemonics. 'break' is EXCLUDED (GCC emits it for div-by-zero
# / overflow traps — not an asm signal).
_GTE = re.compile(
    r"^(mtc2|mfc2|ctc2|cfc2|cop2|lwc2|swc2|"
    r"rtps|rtpt|nclip|avsz3|avsz4|mvmva|gpf|gpl|sqr|dcpl|dpcs|dpct|intpl|"
    r"ncs|nct|ncds|ncdt|cdp|nccs|ncct|ncc|op|cc)$"
)
_HANDWRITTEN = "/* handwritten instruction */"
# splat instruction line:  /* off vram bytes */  <mnem> <operands> [/* note */]
_LINE = re.compile(r"^\s*/\*[^*]*\*/\s+(\S+)(.*)$")


_VRAM_RE = re.compile(r"^([0-9a-fA-F]+)\s+\S.*\sF\s+\S+\s+[0-9a-fA-F]+\s+(\S+)\s*$")
_vram_cache: dict | None = None


def _vram_of(func: str) -> int | None:
    """src symbol -> vram, from build/bb2.elf (cached). Lets us resolve the
    address-named asm file (func_<VRAM>.s) for functions whose src name was
    renamed away from splat's original symbol."""
    global _vram_cache
    if _vram_cache is None:
        _vram_cache = {}
        out = subprocess.run(["bash", "-c", f"{cfg.OBJDUMP} -t build/bb2.elf"],
                             capture_output=True, text=True).stdout
        for line in out.splitlines():
            m = _VRAM_RE.match(line)
            if m:
                _vram_cache[m.group(2)] = int(m.group(1), 16)
    return _vram_cache.get(func)


def _func_path(func: str) -> str:
    p = f"{ASM_DIR}/{func}.s"
    if Path(p).exists():
        return p
    vram = _vram_of(func)
    if vram is not None:
        alt = f"{ASM_DIR}/func_{vram:08X}.s"
        if Path(alt).exists():
            return alt
    return p  # original; classify() reports NO-TARGET if absent


def definitive_insns(func: str):
    """Returns (hits, total_insns) where hits is a list of
    (idx, mnemonic, reason) for definitive-asm instructions. None if no target."""
    p = Path(_func_path(func))
    if not p.exists():
        return None
    hits, idx = [], 0
    for line in p.read_text().splitlines():
        m = _LINE.match(line)
        if not m:
            continue  # label / directive / blank
        mn = m.group(1)
        reason = None
        if mn == ".word":
            reason = "raw .word"
        elif mn == "syscall":
            reason = "syscall (BIOS trampoline)"
        elif _GTE.match(mn):
            reason = f"GTE/cop2 op ({mn})"
        elif _HANDWRITTEN in line:
            reason = "splat handwritten annotation"
        if reason:
            hits.append((idx, mn, reason))
        idx += 1
    return hits, idx


def _regions(indices: list[int]) -> list[tuple[int, int]]:
    """Collapse instruction indices into contiguous [start,end] spans."""
    if not indices:
        return []
    spans, s, p = [], indices[0], indices[0]
    for i in indices[1:]:
        if i == p + 1:
            p = i
        else:
            spans.append((s, p))
            s = p = i
    spans.append((s, p))
    return spans


def classify(func: str) -> dict:
    res = definitive_insns(func)
    if res is None:
        return {"func": func, "verdict": "NO-TARGET", "reason": f"no {_func_path(func)}"}
    hits, total = res
    if not hits:
        return {"func": func, "verdict": "C", "asm_insns": 0, "total": total,
                "reason": "no definitive asm signal — pure-C target"}
    spans = _regions([i for i, _, _ in hits])
    frac = len(hits) / total if total else 0
    verdict = "ASM-WHOLE" if frac >= 0.8 else "ASM-PARTIAL"
    reasons = sorted({r for _, _, r in hits})
    return {"func": func, "verdict": verdict, "asm_insns": len(hits), "total": total,
            "regions": spans, "reasons": reasons,
            "reason": f"{len(hits)}/{total} insns canonical-asm ({'; '.join(reasons)})"}


def scan_all() -> list[dict]:
    funcs = sorted(p.stem for p in Path(ASM_DIR).glob("*.s"))
    return [classify(f) for f in funcs]
