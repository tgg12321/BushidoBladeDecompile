"""Function-level asm scorer for the cheat-invisible sandbox.

Compares a cheat-disabled build of a function against the canonical (cheat-on,
byte-correct) build AT THE OBJECT LEVEL — both are .o files, so relocations
render identically and there are no false diffs from unresolved symbols. Control-
flow targets (branch/jump destinations) are masked so the score is
cascade-immune: removing an instruction shifts later addresses but not the
normalized instruction form. SECTION-RELATIVE HI16/LO16 relocation addends are
masked for the same reason: they are link-time-resolved offsets into this
object's own .text/.rodata/.data, so cheat-stripping earlier code in the TU
moves the referenced static and changes the addend without changing anything
the compiler emitted for THIS function (the saEft00Add false-distance case,
memory/sandbox-lo16-text-addend-false-distance.md).

What that mask COSTS (measured 2026-08-07: 74 instructions in 27 functions,
68 of them .rodata literal-pool loads, 6 .text `la`s of same-TU statics): at
those sites the score no longer sees WHICH same-section referent is used, nor
any source-level constant offset — a different literal-pool float, a different
same-TU static's address, `&D_x[2]` vs `&D_x[3]`, all compare equal. That is
inert for the regen / COMPLETED-C decision (both objects are built from the
same C text there, so only layout can differ), but it is LIVE while iterating
an edited src/<stem>.c against a stale build/src/<stem>.o. func_byte_signature
/ is_redundant stay unmasked and the full-build SHA1 oracle is the actual match
gate, so a masked false-zero cannot be committed — it can only waste a cycle.

score = instruction edit-distance (Levenshtein DP); 0 == instruction-identical.
This GUIDES the loop. verify-integrated (full-build SHA1) is the real match gate
— a masked-target false-zero is impossible to commit because integration checks
actual bytes.
"""
from __future__ import annotations

import difflib
import re
import subprocess

from . import buildconfig as cfg


def _objdump(*args: str) -> str:
    """objdump stdout, invoked as an argv list (no shell).

    Object paths reach objdump verbatim, so an absolute path containing spaces
    (this repo's own directory does) can't be word-split into a silently empty
    symbol table — which surfaced as a misleading "<func> not found in <obj>".
    """
    return subprocess.run([cfg.OBJDUMP, *args],
                          capture_output=True, text=True).stdout

# objdump -t function line:  OFFSET <flags> F <section> SIZE NAME
_SYMOFF_RE = re.compile(r"^([0-9a-fA-F]+)\s+\S.*\sF\s+\S+\s+([0-9a-fA-F]+)\s+(\S+)\s*$")
# objdump -d instruction line:  "   <addr>:\t<bytes>\t<mnemonic>\t<operands>"
_INSN_RE = re.compile(r"^\s*([0-9a-f]+):\t[0-9a-f ]+\t(\S+)\s*(.*)$")
_BRANCH = re.compile(r"^(b|bal|beq|bne|blez|bgtz|bltz|bgez|beqz|bnez|j|jal|jr|jalr)\b")

# objdump -dr relocation line:  "\t\t\t<addr>: <TYPE>\t<symbol>"
_RELOC_RE = re.compile(r"^\s+([0-9a-f]+):\s+(R_MIPS_\S+)\s+(\S+)\s*$")
# Only these two carry a link-resolved addend in the instruction's immediate
# field. R_MIPS_26 / R_MIPS_PC16 also occur against section symbols, but their
# field is a control-flow target already masked by _BRANCH; R_MIPS_GPREL16
# never occurs against a section symbol in this tree (measured 2026-08-07:
# 2295/2295 named). Named-symbol HI16/LO16 are NOT masked — their immediate is
# a source-level addend (`&sym + 2`), not a layout artifact — but they ARE
# RESOLVED (owner ruling 2026-09-04, Ruling B.4): the bytes the linker writes
# are %hi/%lo(symbol + addend), so `D_800F19B8+4` and `D_800F19BC` link to the
# same word and must score equal, while `&sym+2` vs `&sym+4` still differ.
# Before this, every aggregate-merge probe (struct spelling vs per-word splat
# symbol) paid one false point per member access — five on CD_datasync s58,
# with nothing about the emitted code differing. An unresolvable symbol keeps
# its literal immediate (the previous behaviour), never a false zero.
_SECTION_ADDEND_RELOCS = {"R_MIPS_HI16", "R_MIPS_LO16"}
# leading signed immediate of an operand: "8132", "0x1fc4", "32(at)" -> "32"
_IMM_RE = re.compile(r"^-?(?:0x[0-9a-fA-F]+|\d+)")
# linker symbol-file line:  NAME = 0xADDR;
_SYM_DEF_RE = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9a-fA-F]+)\s*;")
# name -> address, loaded once per process from cfg.LD_SYM_FILES (the same
# files bb2.ld links against). Tests pre-seed this to stub the table.
_SYMTAB_CACHE: dict[str, int] | None = None


def _symtab() -> dict[str, int]:
    global _SYMTAB_CACHE
    if _SYMTAB_CACHE is None:
        tab: dict[str, int] = {}
        for fn in cfg.LD_SYM_FILES:
            try:
                with open(fn, encoding="utf-8", errors="replace") as fh:
                    for line in fh:
                        m = _SYM_DEF_RE.match(line)
                        if m:
                            tab.setdefault(m.group(1), int(m.group(2), 16))
            except OSError:
                continue
        _SYMTAB_CACHE = tab
    return _SYMTAB_CACHE


def _leading_imm(ops: str) -> int | None:
    """The signed immediate at the head of the LAST operand, or None."""
    parts = ops.split(",")
    m = _IMM_RE.match(parts[-1]) if parts else None
    if not m:
        return None
    try:
        return int(m.group(0), 0)
    except ValueError:
        return None


def _resolve_named_pair(insns: list[list[str]], hi_i: int, hi: int | None,
                        lo_i: int, sym: str) -> None:
    """Rewrite a named-symbol HI16/LO16 pair's immediates to their LINKED
    values. ld's MIPS REL rule: value = S + (hi_field << 16) + sext16(lo_field);
    lui gets ((value + 0x8000) >> 16) & 0xffff, the LO16 insn gets
    value & 0xffff. Tokens `@hi(0x....)` / `@lo(0x....)` encode exactly the
    words the linker writes, so equal bytes score equal and different bytes
    still differ. `hi` is the lui's RAW immediate captured when its HI16 reloc
    was seen (the operand is a token after the first rewrite). Leaves both
    immediates untouched when the symbol is unknown or either immediate is not
    parseable."""
    addr = _symtab().get(sym)
    if addr is None:
        return
    lo = _leading_imm(insns[lo_i][1])
    if hi is None or lo is None:
        return
    lo_s = ((lo & 0xFFFF) ^ 0x8000) - 0x8000
    value = (addr + ((hi & 0xFFFF) << 16) + lo_s) & 0xFFFFFFFF
    hi_out = ((value + 0x8000) >> 16) & 0xFFFF
    lo_out = value & 0xFFFF
    for idx, tok in ((hi_i, f"@hi(0x{hi_out:04x})"), (lo_i, f"@lo(0x{lo_out:04x})")):
        parts = insns[idx][1].split(",")
        parts[-1] = _IMM_RE.sub(tok, parts[-1], count=1)
        insns[idx][1] = ",".join(parts)


def _mask_section_addend(ops: str, sym: str) -> str:
    """Replace the immediate field of a section-relative reloc with `@<section>`.

    `addiu a0,a0,8132` (R_MIPS_LO16 .text) -> `addiu a0,a0,@.text`
    `lw v0,32(at)`     (R_MIPS_LO16 .text) -> `lw v0,@.text(at)`

    The section name is KEPT in the token, so a reference that moves between
    .text and .rodata still scores as a difference. Everything else about the
    referent is erased: the offset identifies WHICH object in the section, so
    two different .rodata literals or two different same-TU statics compare
    equal here. See the module docstring for the bound on that cost.
    """
    parts = ops.split(",")
    if parts and _IMM_RE.match(parts[-1]):
        parts[-1] = _IMM_RE.sub("@" + sym, parts[-1], count=1)
        return ",".join(parts)
    return ops  # unexpected operand shape: leave it counting, don't hide it


def _o_func_table(o_path: str) -> dict[str, tuple[int, int]]:
    """name -> (offset, size) for functions in a .o; size recomputed from the
    next function's offset when the symbol size is 0 (maspsx omits .size)."""
    out = _objdump("-t", o_path)
    funcs = {}
    for line in out.splitlines():
        m = _SYMOFF_RE.match(line)
        if m:
            funcs[m.group(3)] = (int(m.group(1), 16), int(m.group(2), 16))
    offs = sorted({o for o, _ in funcs.values()})
    nxt = {o: offs[i + 1] for i, o in enumerate(offs[:-1])}
    # section end as a backstop for the last function
    end = _section_size(o_path, ".text")
    return {
        n: (o, (s if s else (nxt.get(o, end) - o if (nxt.get(o, end) or 0) > o else 0)))
        for n, (o, s) in funcs.items()
    }


def _section_size(o_path: str, section: str) -> int:
    out = _objdump("-h", o_path)
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 3 and parts[1] == section:
            return int(parts[2], 16)
    return 0


def normalized_insns(o_path: str, func: str, mask: bool = True) -> list[str]:
    """Normalized instruction strings for a function. mask=True masks
    control-flow targets AND section-relative HI16/LO16 relocation addends
    (cascade-immune, for scoring); mask=False keeps full operands (for diff
    diagnosis)."""
    tbl = _o_func_table(o_path)
    if func not in tbl:
        raise KeyError(f"{func} not found in {o_path}")
    off, size = tbl[func]
    out = _objdump("-dr", f"--start-address={off}",
                   f"--stop-address={off + size}", o_path)
    insns: list[list[str]] = []
    at_addr: dict[str, int] = {}
    # Most recent named-symbol HI16 per symbol: the lui a following LO16 on the
    # same symbol pairs with. Keyed (not consumed) so a lui shared by several
    # loads of one symbol resolves every load, whether as emitted one HI16
    # line per LO16 or a single one (layer-2 review 2026-09-04).
    pending_hi: dict[str, tuple[int, int | None]] = {}   # sym -> (lui index, raw imm)
    for line in out.splitlines():
        m = _INSN_RE.match(line)
        if not m:
            # A relocation line annotates the instruction ABOVE it, so it can
            # only be applied once that instruction is already in `insns`.
            r = _RELOC_RE.match(line) if mask else None
            if r and r.group(2) in _SECTION_ADDEND_RELOCS:
                i = at_addr.get(r.group(1))
                sym = r.group(3)
                if i is None:
                    continue
                if sym.startswith("."):
                    insns[i][1] = _mask_section_addend(insns[i][1], sym)
                elif r.group(2) == "R_MIPS_HI16":
                    if sym not in pending_hi or pending_hi[sym][0] != i:
                        pending_hi[sym] = (i, _leading_imm(insns[i][1]))
                elif sym in pending_hi:
                    hi_i, hi_imm = pending_hi[sym]
                    _resolve_named_pair(insns, hi_i, hi_imm, i, sym)
            continue  # labels, other reloc lines, section headers
        addr, mn, ops = m.group(1), m.group(2), m.group(3).strip()
        ops = re.split(r"\s+<", ops)[0]      # drop "<sym+0x..>" annotation
        ops = ops.split("#")[0].strip()       # drop trailing comment
        if mask and _BRANCH.match(mn):
            parts = ops.split(",")
            if parts:
                parts[-1] = "@"               # mask control-flow target
            ops = ",".join(parts)
        at_addr[addr] = len(insns)
        insns.append([mn, ops])
    return [f"{mn} {ops}".strip() for mn, ops in insns]


def _levenshtein(a: list[str], b: list[str]) -> int:
    """Exact minimal edit distance (insert/delete/substitute) between two
    instruction lists. SequenceMatcher's greedy block-matching over-counts when
    deletions shift positions (an identical line gets scored as delete+insert);
    the DP gives the true minimum. Lists are short (<~few hundred insns)."""
    m, n = len(a), len(b)
    prev = list(range(n + 1))
    for i in range(1, m + 1):
        cur = [i] + [0] * n
        ai = a[i - 1]
        for j in range(1, n + 1):
            cost = 0 if ai == b[j - 1] else 1
            cur[j] = min(prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost)
        prev = cur
    return prev[n]


def func_byte_signature(o_path: str, func: str) -> str:
    """Exact concatenated instruction bytes (the objdump hex column) for a
    function in a .o. Two pre-link .o's share identical relocations, so this is
    an EXACT redundancy oracle: a rule that only retargets a local branch (which
    the masked instruction score ignores) still changes these bytes."""
    tbl = _o_func_table(o_path)
    if func not in tbl:
        raise KeyError(f"{func} not found in {o_path}")
    off, size = tbl[func]
    out = _objdump("-d", f"--start-address={off}",
                   f"--stop-address={off + size}", o_path)
    sig = []
    for line in out.splitlines():
        parts = line.split("\t")
        if len(parts) >= 3 and parts[0].strip().endswith(":"):
            sig.append(parts[1].replace(" ", ""))
    return "".join(sig)


def is_redundant(stripped_o: str, reference_o: str, func: str) -> bool:
    """True iff `func`'s exact bytes are unchanged when its rules are removed —
    the only sound test for 'this function's rules are dead'. Immune to the
    masked-target false-zero that the instruction score has."""
    return func_byte_signature(stripped_o, func) == func_byte_signature(reference_o, func)


def score_func(cheat_disabled_o: str, reference_o: str, func: str) -> dict:
    """Edit-distance between the cheat-disabled function and the canonical
    (cheat-on) function. reference_o is build/src/<file>.o (byte-correct).
    0 == instruction-identical (modulo masked control-flow targets and
    masked section-relative relocation addends)."""
    target = normalized_insns(reference_o, func)
    built = normalized_insns(cheat_disabled_o, func)
    return {"score": _levenshtein(target, built),
            "target_insns": len(target), "build_insns": len(built)}


def _opcode(insn: str) -> str:
    return insn.split()[0] if insn else ""


def _hunk_class(target: list[str], built: list[str],
                target_masked: list[str], built_masked: list[str]) -> str:
    """Which KIND of divergence a hunk is — the question that decides whether
    the next lever is a source-level restructure or a register-seat nudge.

    not-scored    the two runs are EQUAL once masked, so the score does not
                  count this hunk at all: a branch/jump displacement or a
                  section-relative addend that moved because earlier code
                  changed size. Pure cascade artifact — chasing it is wasted
                  work, which is exactly why the score masks it.
    operand-only  same opcodes in the same order, different operands, and the
                  difference SURVIVES masking: a real register-allocation /
                  scheduling-seat difference.
    source-level  the opcode sequence itself differs (or a whole run is
                  present on one side only): the C is saying something
                  different, and no amount of reg-seat work closes it.

    Measured motivation: _SsSndCrescendo plateaued at floor 130 for four
    sessions of reg-alloc work; when the full diff was finally read, 12 of the
    13 surplus instructions were source-level."""
    if target_masked == built_masked:
        return "not-scored"
    if len(target) != len(built) or not target:
        return "source-level"
    return ("operand-only"
            if all(_opcode(t) == _opcode(b) for t, b in zip(target, built))
            else "source-level")


def insn_diff(built_o: str, reference_o: str, func: str) -> dict:
    """WHERE the built function differs from the target, not just how much.

    The hunks are aligned and DISPLAYED unmasked (mask=False): the score masks
    control-flow targets and section-relative addends for cascade-immunity, but
    a reader diagnosing a gap needs the real registers and offsets. Each hunk is
    then CLASSIFIED against the masked forms as well, so a hunk the score does
    not count is labelled `not-scored` instead of masquerading as a
    register-allocation difference. Masking never changes the instruction count,
    so both views share one index space.

    Returns the hunk list plus a per-class tally; `_hunk_class` defines the
    three classes."""
    target = normalized_insns(reference_o, func, mask=False)
    built = normalized_insns(built_o, func, mask=False)
    target_m = normalized_insns(reference_o, func, mask=True)
    built_m = normalized_insns(built_o, func, mask=True)
    sm = difflib.SequenceMatcher(a=target, b=built, autojunk=False)
    hunks = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == "equal":
            continue
        t, b = target[i1:i2], built[j1:j2]
        hunks.append({"tag": tag, "target_at": i1, "build_at": j1,
                      "target": t, "built": b,
                      "class": _hunk_class(t, b, target_m[i1:i2], built_m[j1:j2])})
    return {"func": func, "target_insns": len(target), "build_insns": len(built),
            "hunks": hunks,
            "operand_only_hunks": sum(1 for h in hunks if h["class"] == "operand-only"),
            "source_level_hunks": sum(1 for h in hunks if h["class"] == "source-level"),
            "not_scored_hunks": sum(1 for h in hunks if h["class"] == "not-scored")}
