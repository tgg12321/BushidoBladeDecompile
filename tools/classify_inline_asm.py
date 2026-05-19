#!/usr/bin/env python3
"""Classify each `__asm__` block in src/*.c into tier 2 vs tier 3.

Tier system (canonical statement of where BB2 stands relative to the
SOTN community bar — see .claude/rules/inline-asm-tiers.md):

  1 = full canonical-asm function body. The original was hand-written
      asm OR was emitted via file-scope `__asm__("glabel ...")`. Listed
      in inline_asm_canonical.txt. NOT classified here.
  2 = inline `__asm__` in a C function body using opcodes that ONLY exist
      in asm form. Authentic — original developers wrote these.
        - GTE coprocessor ops: ctc2/cfc2/mtc2/mfc2/lwc2/swc2
        - GTE math ops: .word 0x4XXXXXXX (cop2 instruction encoding)
        - BIOS vectors: jumps to 0xA0/0xB0/0xC0
        - Cache/DMA hardware register pokes (specific addresses)
  3 = inline `__asm__` in a C function body using general-purpose GPR
      opcodes (move/addu/nop/lui/etc.) to steer GCC's allocator or
      scheduler. Workarounds we added; NOT in original source.
  4 = pure C, no inline asm needed. Not counted here.

SOTN-bar functions are tier 1, 2, or 4. Tier 3 is the BB2-specific gap.
The "gap to SOTN bar" metric is the count of tier-3 functions.

Heuristic limitations (documented):
  - Multi-line `__asm__()` blocks (with one string per line) need the
    parser to track parentheses across lines; we handle this by joining
    physical lines inside an open `__asm__(` until the matching `)`.
  - File-scope `__asm__("...glabel...")` blocks are excluded (tier 1).
  - `sw`/`lw` to scratchpad addresses (0x1F8003xx) are tier 2 (hardware
    pokes); to general addresses they default to tier 3.

Output formats:
  default — per-function table + summary
  --summary — just the summary counts
  --json — structured JSON for tooling consumption
  --func F — classify ONE function (for `dc.sh classify-inline-asm <func>`)
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = ROOT / "src"

# --- Tier 2: authentic asm (no C equivalent exists) ---
TIER2_OPS = frozenset({
    # GTE register transfers
    "ctc2", "cfc2", "mtc2", "mfc2",
    # GTE memory load/store
    "lwc2", "swc2",
    # cop2 misc (rare)
    "ctc0", "mfc0", "mtc0",  # MIPS coprocessor 0 — kernel-mode
})

# Tier 2: GTE op encodings via .word — top byte 0x48-0x4F is cop2 op range.
TIER2_DOTWORD_RE = re.compile(r'\.word\s*0x4[89A-Fa-f]', re.IGNORECASE)

# Tier 2: BIOS vector jumps. A0/B0/C0 are the PSX BIOS dispatch vectors.
TIER2_BIOS_RE = re.compile(
    r'\b(?:j|jal)\s+0x[abc]0\b|\bjr\s+\$t1\b',
    re.IGNORECASE,
)

# Tier 2: scratchpad / hardware register addresses.
# Scratchpad: 0x1F800000–0x1F8003FF (1 KB).
# I/O ports: 0x1F801000–0x1F803FFF (GPU/SPU/DMA/etc.).
TIER2_HW_ADDR_RE = re.compile(r'0x1f80[0123]\b|0x1f8003', re.IGNORECASE)

# --- Tier 3: general-purpose GPR ops used as workarounds ---
TIER3_OPS = frozenset({
    # Arithmetic / logical
    "move", "addu", "addiu", "subu", "negu", "neg",
    "add", "sub", "addi",
    "lui", "li", "la",
    "andi", "ori", "xori", "and", "or", "xor", "nor",
    # Shifts
    "sll", "sra", "srl", "sllv", "srlv", "srav",
    # Compare
    "slt", "slti", "sltu", "sltiu",
    # Memory (general — overridden to tier 2 when address is hardware)
    "sw", "lw", "sh", "lh", "sb", "lb", "lhu", "lbu", "lwl", "lwr", "swl", "swr",
    # Multiply/divide
    "mult", "multu", "div", "divu", "mflo", "mfhi", "mtlo", "mthi",
    # Filler / no-op
    "nop",
    # Branches / jumps (general)
    "beq", "bne", "bgez", "bltz", "blez", "bgtz", "b",
    "bgezl", "bltzl", "blezl", "bgtzl", "beql", "bnel",
    "bgezal", "bltzal",
})

# Empty asm template (typically `__asm__ volatile("" ::: "memory")` —
# scheduling barrier). Count as tier 3 since it's purely codegen control.
EMPTY_ASM_IS_TIER3 = True

# --- AST-light parser ---
# We don't fully parse C. Instead we use line-by-line state tracking.
# This is accurate enough for the BB2 codebase's actual style.

# Matches a C function DEFINITION signature. The body's `{` may be on
# this line OR the next non-blank line (K&R style). Two regexes:
#   FUNC_OPEN_RE — signature + `{` on same line
#   FUNC_SIG_RE  — signature only (must look ahead for `{` on next line)
FUNC_OPEN_RE = re.compile(
    r'^\s*(?:static\s+)?(?:inline\s+)?'
    r'[a-zA-Z_][\w*]*\s+\**\s*([a-zA-Z_]\w*)\s*\([^;{]*\)\s*\{'
)
FUNC_SIG_RE = re.compile(
    r'^\s*(?:static\s+)?(?:inline\s+)?'
    r'[a-zA-Z_][\w*]*\s+\**\s*([a-zA-Z_]\w*)\s*\([^;{]*\)\s*$'
)
BRACE_ALONE_RE = re.compile(r'^\s*\{\s*$')

# File-scope `__asm__("...glabel...")` block (canonical-asm function body).
# We exclude these from inline-asm classification.
GLABEL_HINT_RE = re.compile(r'\bglabel\b|\bendlabel\b')

# `__asm__` keyword (with double underscores) for inline-asm STATEMENTS.
# We deliberately do NOT match bare `asm(` here — that form is used as a
# register-binding qualifier in declarations like `register T x asm("$N")`
# and would double-count. Register pins are classified separately below.
ASM_KEYWORD_RE = re.compile(r'\b(__asm__|__asm)\s*(?:volatile|__volatile__)?\s*\(')

# Register-asm pin declarations: `register T x asm("$N");` or
# `register T x asm("$N") = expr;`. These are also tier-3 workarounds
# (allocation hints we added) but a different KIND than `__asm__` blocks.
REGISTER_PIN_RE = re.compile(r'\bregister\s+[^=;]*\basm\s*\(\s*"[^"]+"\s*\)')


def classify_template(template: str) -> str:
    """Classify a single asm template string. Returns 'tier2' or 'tier3'."""
    text = template.strip()
    if not text:
        return "tier3" if EMPTY_ASM_IS_TIER3 else "tier2"
    # Check tier-2 regex signals first (.word cop2, BIOS vectors, HW addr).
    if TIER2_DOTWORD_RE.search(text):
        return "tier2"
    if TIER2_BIOS_RE.search(text):
        return "tier2"
    if TIER2_HW_ADDR_RE.search(text):
        return "tier2"
    # Opcode-based check: get the first whitespace-separated token.
    # Skip `.set noreorder` etc. directives.
    first = text.lstrip().split(None, 1)[0].lower()
    if first.startswith("."):
        # Directive like .set, .word (handled above). Default tier 3.
        return "tier3"
    if first in TIER2_OPS:
        return "tier2"
    if first in TIER3_OPS:
        return "tier3"
    # Unknown opcode — default to tier 3 (workaround / unclassified is
    # safer than letting unknowns pass as authentic).
    return "tier3"


def extract_strings(parens_body: str) -> list[str]:
    """Extract C string literals from inside the parentheses of an asm
    block. Each string is the content between consecutive `"`s, with
    \\n / \\t literally preserved (we don't unescape; that would lose
    the multi-instruction grouping)."""
    return re.findall(r'"((?:[^"\\]|\\.)*)"', parens_body)


def split_template(template: str) -> list[str]:
    """Split a multi-line template (with literal `\\n` separators) into
    one entry per logical instruction. The asm template can be either:
       "ctc2 %0, $0"                  — one instruction
       "ctc2 %0, $0\\nlwc2 $0, 0(%1)" — two instructions joined by \\n
    """
    parts = re.split(r'\\n', template)
    return [p.strip() for p in parts if p.strip()]


def scan_file(path: Path) -> list[dict]:
    """Scan one .c file. Returns list of records, one per __asm__ block.
    Each record: {file, line, func, tier, template, opcode_first, is_glabel}.
    """
    try:
        text = path.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return []
    lines = text.splitlines()

    # Track enclosing function via brace depth. We assume a function body
    # opens with `{` and closes with `}` at column 0 (BB2 style).
    func_stack: list[tuple[str, int]] = []  # (name, depth_at_entry)
    depth = 0
    current_func: str | None = None

    records: list[dict] = []

    # We need to handle __asm__ blocks that span multiple lines (e.g., a
    # multi-string template). Do a single pass: when we see `__asm__(`
    # without the matching `)`, accumulate lines until we find it.
    i = 0
    while i < len(lines):
        line = lines[i]

        # Update brace depth from THIS line (count opens/closes outside
        # of string literals — simplified: just count chars). Not perfect
        # but good enough for BB2 style.
        # First detect function-definition opening (which gives us depth+1).
        # Handle both `void foo(...) {` (same line) and K&R `void foo(...)\n{`.
        # `just_pushed` prevents the pop check below from immediately popping
        # K&R signatures (where the `{` hasn't yet raised depth).
        just_pushed = False
        if depth == 0:
            m = FUNC_OPEN_RE.match(line)
            if m:
                current_func = m.group(1)
                # Save depth-BEFORE the `{` (= 0); pop when depth returns to it.
                func_stack.append((current_func, depth))
                just_pushed = True
            else:
                sig = FUNC_SIG_RE.match(line)
                if sig:
                    # Look ahead for `{` on the next non-blank line.
                    j = i + 1
                    while j < len(lines) and not lines[j].strip():
                        j += 1
                    if j < len(lines) and BRACE_ALONE_RE.match(lines[j]):
                        current_func = sig.group(1)
                        func_stack.append((current_func, depth))
                        just_pushed = True

        # Detect register-asm pins on this line (tier-3 workaround,
        # different KIND from __asm__ blocks).
        for pm in REGISTER_PIN_RE.finditer(line):
            records.append({
                "file": str(path.relative_to(ROOT)),
                "line": i + 1,
                "func": current_func,
                "tier": "tier3",
                "kind": "register_pin",
                "templates": [pm.group(0)],
                "first_op": "register-asm pin",
            })

        # Find any __asm__ keywords on this line. If found, capture the
        # full asm( ... ) span across lines.
        for m in ASM_KEYWORD_RE.finditer(line):
            # Position of the opening `(`.
            paren_start = m.end() - 1  # ASM_KEYWORD_RE ends at the `(`
            # Find the matching `)`, possibly spanning lines.
            buf = line[paren_start + 1:]
            j = i
            depth_p = 1
            k = 0
            while True:
                while k < len(buf):
                    c = buf[k]
                    if c == '(':
                        depth_p += 1
                    elif c == ')':
                        depth_p -= 1
                        if depth_p == 0:
                            break
                    k += 1
                if depth_p == 0:
                    # Found matching close. The asm block body is buf[:k].
                    asm_body = buf[:k]
                    break
                # Need more lines.
                j += 1
                if j >= len(lines):
                    asm_body = buf  # truncated, best effort
                    break
                buf = buf + "\n" + lines[j]
                k = 0
                # Re-scan from start of newly-appended portion.
                # (Simpler: re-scan entire buf each iteration.)
                k = 0
                depth_p = 1
                while k < len(buf):
                    c = buf[k]
                    if c == '(':
                        depth_p += 1
                    elif c == ')':
                        depth_p -= 1
                        if depth_p == 0:
                            break
                    k += 1
                if depth_p == 0:
                    asm_body = buf[:k]
                    break

            # Extract template strings from the asm body.
            strings = extract_strings(asm_body)
            # Strip the operand/clobber portion: a string after `:` doesn't
            # contribute opcodes. Operand strings look like "r"(var), but
            # we only extract from BEFORE the first `:` at depth 1.
            # Simpler: classify each template-position string, then strip
            # operand-position strings by detecting `:` in the body.
            first_colon = asm_body.find(":")
            template_body = asm_body if first_colon == -1 else asm_body[:first_colon]
            template_strs = extract_strings(template_body)
            # Detect glabel/endlabel — tier 1 (canonical body), skip.
            is_glabel = any(GLABEL_HINT_RE.search(s) for s in template_strs)

            if not is_glabel:
                # Classify each instruction in the templates. If any tier-2
                # opcode is present, the block is tier 2. Otherwise tier 3.
                block_tier = "tier3"  # default for empty / unknown
                if not template_strs:
                    block_tier = "tier3" if EMPTY_ASM_IS_TIER3 else "tier2"
                else:
                    instrs = []
                    for t in template_strs:
                        instrs.extend(split_template(t))
                    if not instrs:
                        block_tier = "tier3" if EMPTY_ASM_IS_TIER3 else "tier2"
                    else:
                        per_instr_tiers = [classify_template(ins) for ins in instrs]
                        if "tier2" in per_instr_tiers:
                            block_tier = "tier2"
                        else:
                            block_tier = "tier3"
                    first_op = instrs[0].split(None, 1)[0].lower() if instrs else ""

                records.append({
                    "file": str(path.relative_to(ROOT)),
                    "line": i + 1,
                    "func": current_func,
                    "tier": block_tier,
                    "kind": "asm_block",
                    "templates": template_strs,
                    "first_op": instrs[0] if instrs else "",
                })
            # Skip past this asm block (we'll continue scanning subsequent
            # asm blocks on the joined buffer; for simplicity, break to
            # advance to the line after the asm block's close).
            i = j  # last line consumed by the asm
            break  # restart line loop from i+1

        # Update brace depth for the WHOLE line (after asm parsing).
        # Simplification: count `{` and `}` ignoring strings/comments.
        # This is approximate but adequate.
        # Remove string literals to avoid braces inside strings.
        stripped = re.sub(r'"(?:[^"\\]|\\.)*"', '', line)
        stripped = re.sub(r"'(?:[^'\\]|\\.)*'", '', stripped)
        # Remove line comments.
        stripped = re.sub(r'//.*$', '', stripped)
        opens = stripped.count('{')
        closes = stripped.count('}')
        depth += opens - closes
        if depth < 0:
            depth = 0  # defensive: don't go negative on mis-parse
        # Pop funcs that have closed. Saved value is depth BEFORE the
        # function's `{`; pop when current depth has returned to it.
        # Skip pop check on lines that just pushed (otherwise K&R
        # signatures would pop immediately, before their `{` arrives).
        if not just_pushed:
            while func_stack and func_stack[-1][1] >= depth:
                func_stack.pop()
        current_func = func_stack[-1][0] if func_stack else None
        i += 1

    return records


def summarize(records: list[dict]) -> dict:
    """Compute per-function and per-project rollup."""
    per_func: dict[str | None, dict[str, int]] = {}
    tier2_total = 0
    tier3_total = 0
    tier3_pins_total = 0
    tier3_blocks_total = 0
    for r in records:
        # Key file-scope records per-file so distinct files don't merge.
        func = r["func"] or f"<file-scope:{r['file']}>"
        bucket = per_func.setdefault(func, {
            "tier2": 0, "tier3": 0, "tier3_pins": 0, "tier3_blocks": 0,
            "file": r["file"],
        })
        bucket[r["tier"]] += 1
        if r["tier"] == "tier2":
            tier2_total += 1
        elif r["tier"] == "tier3":
            tier3_total += 1
            if r.get("kind") == "register_pin":
                tier3_pins_total += 1
                bucket["tier3_pins"] += 1
            else:
                tier3_blocks_total += 1
                bucket["tier3_blocks"] += 1
    # Per-function tier label.
    func_tiers: dict[str, list[dict]] = {"tier2": [], "tier3": [], "mixed": []}
    for func, b in per_func.items():
        if b["tier3"] > 0 and b["tier2"] > 0:
            func_tiers["mixed"].append({"func": func, **b})
        elif b["tier3"] > 0:
            func_tiers["tier3"].append({"func": func, **b})
        else:
            func_tiers["tier2"].append({"func": func, **b})
    return {
        "totals": {
            "tier2_instances": tier2_total,
            "tier3_instances": tier3_total,
            "tier3_register_pins": tier3_pins_total,
            "tier3_asm_blocks": tier3_blocks_total,
            "tier2_funcs_only": len(func_tiers["tier2"]),
            "tier3_funcs_only": len(func_tiers["tier3"]),
            "mixed_funcs": len(func_tiers["mixed"]),
            "gap_to_sotn_funcs": len(func_tiers["tier3"]) + len(func_tiers["mixed"]),
            "gap_to_sotn_instances": tier3_total,
        },
        "per_func": per_func,
        "func_tiers": func_tiers,
    }


def render_human(summary: dict, *, color: bool = False, show_funcs: bool = True) -> str:
    GREEN = "\033[32m" if color else ""
    YELLOW = "\033[33m" if color else ""
    RED = "\033[31m" if color else ""
    DIM = "\033[2m" if color else ""
    RESET = "\033[0m" if color else ""
    t = summary["totals"]
    out = []
    out.append("=== inline-asm tier classification ===")
    out.append(f"  {GREEN}tier-2 (authentic GTE/BIOS/HW){RESET}: "
               f"{t['tier2_instances']} instances across "
               f"{t['tier2_funcs_only']} pure-tier-2 funcs")
    out.append(f"  {RED}tier-3 (toolchain workaround){RESET}:    "
               f"{t['tier3_instances']} instances "
               f"({t['tier3_asm_blocks']} __asm__ blocks + "
               f"{t['tier3_register_pins']} register-asm pins) "
               f"across {t['tier3_funcs_only']} pure-tier-3 funcs")
    out.append(f"  {YELLOW}mixed (both tiers in one func){RESET}:    "
               f"{t['mixed_funcs']} funcs")
    out.append("")
    out.append(f"  {RED}GAP TO SOTN BAR{RESET}:")
    out.append(f"    {t['gap_to_sotn_funcs']} functions use tier-3 inline asm")
    out.append(f"    {t['gap_to_sotn_instances']} total tier-3 instances to retire")
    out.append("")
    out.append(f"  {DIM}(SOTN-bar funcs use only tier-1 [canonical body], tier-2 "
               f"[authentic asm], or tier-4 [pure C].){RESET}")
    if show_funcs:
        out.append("")
        out.append("--- tier-3 functions (sorted by instance count) ---")
        t3 = sorted(summary["func_tiers"]["tier3"], key=lambda b: -b["tier3"])
        for b in t3[:30]:
            out.append(f"  {b['tier3']:3d}  {b['func']:<40s}  ({b['file']})")
        if len(t3) > 30:
            out.append(f"  ... and {len(t3) - 30} more")
        if summary["func_tiers"]["mixed"]:
            out.append("")
            out.append("--- mixed-tier functions (have both tier-2 + tier-3) ---")
            mx = sorted(summary["func_tiers"]["mixed"], key=lambda b: -b["tier3"])
            for b in mx[:20]:
                out.append(f"  t2={b['tier2']:3d} t3={b['tier3']:3d}  "
                           f"{b['func']:<40s}  ({b['file']})")
            if len(mx) > 20:
                out.append(f"  ... and {len(mx) - 20} more")
    return "\n".join(out)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--json", action="store_true",
                    help="emit JSON to stdout instead of human-readable")
    ap.add_argument("--summary", action="store_true",
                    help="just the summary line (for briefing integration)")
    ap.add_argument("--func", type=str, default=None,
                    help="classify a single function (show its asm blocks)")
    ap.add_argument("--src", type=str, default=None,
                    help="scan a single .c file instead of all of src/")
    args = ap.parse_args()

    files: list[Path]
    if args.src:
        files = [Path(args.src)]
        if not files[0].is_absolute():
            files[0] = ROOT / args.src
    else:
        files = sorted(SRC_DIR.glob("*.c"))

    records: list[dict] = []
    for f in files:
        records.extend(scan_file(f))

    if args.func:
        # Filter to just the requested function.
        records = [r for r in records if r.get("func") == args.func]
        if not records:
            print(f"No __asm__ blocks found in function '{args.func}' across {len(files)} file(s).")
            return 0
        for r in records:
            print(f"{r['file']}:{r['line']}  [{r['tier']}]  {r['first_op'][:60]}")
        return 0

    summary = summarize(records)
    if args.json:
        print(json.dumps(summary, indent=2))
        return 0
    if args.summary:
        t = summary["totals"]
        print(f"Inline asm: {t['tier2_instances']} tier-2 (authentic), "
              f"{t['tier3_instances']} tier-3 (workaround) "
              f"across {t['gap_to_sotn_funcs']} funcs. "
              f"SOTN gap: {t['gap_to_sotn_funcs']} funcs / "
              f"{t['gap_to_sotn_instances']} instances.")
        return 0
    print(render_human(summary, color=sys.stdout.isatty()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
