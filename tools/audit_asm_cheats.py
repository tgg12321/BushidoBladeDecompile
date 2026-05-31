#!/usr/bin/env python3
"""Audit asm-cheat patterns that bypass the pure-C decomp requirement.

Two patterns make the binary match without the C source actually
corresponding to the emitted bytes:

  1. LARGE SPLICE in regfix.txt
     `func: splice K..N "line1" "line2" ...` with N replacement lines.
     `splice` was designed for small structural transforms (3-line
     example in regfix.py docs). Replacing 100+ lines is essentially
     `replace_with_asmfile` reimplemented in regfix — the C source
     emits nothing useful, the bytes come from the splice content
     verbatim from asm/funcs/<func>.s.

  2. INLINE __asm__ FUNCTION BODY in src/*.c
     `__asm__(".section .text\\n... glabel funcname\\n ... endlabel ...")`
     A multi-instruction `__asm__` block containing `glabel funcname`
     is a file-scope asm function definition. GCC never sees the
     function body as C; the asm text goes straight to gas.

Both patterns are LEGITIMATE for functions whose ORIGINAL implementation
was hand-coded asm (see memory/feedback_hand_coded_asm_recognition.md).
Such functions must be listed in inline_asm_canonical.txt.

Anything not on that list is an UNAUTHORIZED cheat.

Modes:
  --all          : full report, exit 0
  --summary      : one-line counts ("S splices, I inline-asm, B BIOS trampolines"),
                   exit 0. Used by `dc.sh start` for session briefing.
  --func NAME    : check one function; exit 1 if unauthorized cheat found
  --check-new    : compare current state vs HEAD; exit 1 if new cheats added.
                   (Used by the commit_audit_guard hook before commit — Rule 1a.)
  --list-funcs   : print one unauthorized function name per line, exit 0.
                   Used by queue generators to surface cheat-fix work.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

# Force UTF-8 on Windows so the report's arrows / box characters reach the
# terminal intact (default cp1252 dies on U+2192 etc., the same bug that
# bit llm_audit.sh — see commit 5f609c1).
try:
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
except AttributeError:
    pass

# Thresholds: tuned to distinguish "small structural transform"
# (legitimate splice usage) from "full-body asm replacement".
LARGE_SPLICE_LINES = 30           # single splice rule with >= this many replacement lines
LARGE_SUBST_MULTI_LINES = 30      # single subst_multi rule with >= this many replacement lines
CUMULATIVE_RULE_LINES = 60        # sum across all multi-line ops per function — catches the
                                  # split-evasion attack (e.g., 4x20-line splices to dodge
                                  # the per-rule 30 threshold). 60 leaves headroom for
                                  # legit complex recipes (func_8007CE0C currently 46).
WILDCARD_SUBST_LIMIT = 10         # max `subst ".*"` / `subst ".+"` / `subst ""` rules per
                                  # function. Wildcard substs force-overwrite whatever the
                                  # C source compiled to with the rule's literal text —
                                  # functionally equivalent to replace_with_asmfile (the
                                  # bytes come from the rules, not C codegen). >=10 in
                                  # regfix.txt + regfix_stage2.txt combined = cheat.
GLABEL_BODY_LINES = 5

# Single-instruction `insert`/`insert_after`/`insert_before` rules inject one
# MIPS instruction into the maspsx stream — bytes that didn't come from C
# compilation. The common pattern is restoring something GCC's constant-prop
# or dead-store-eliminator removed (e.g., `addu $sN, $0, $zero` for s2=0).
# These are a smaller-scale cousin of splice: each rule only restores 1 insn
# but they're still asm-injection. Any NEW one in a commit must be flagged
# via --check-new; the EXISTING ones are tracked for cleanup. There is no
# authorized-list equivalent for these — if you need them, the function
# should be in inline_asm_canonical.txt or have a documented recipe note.
INSTRUCTION_INSERT_RE = re.compile(
    r'^(\w+):\s+(insert|insert_after|insert_before)\s+"([^"]*)"\s+@\s+\d+\s*$',
    re.MULTILINE,
)
# Patterns we KNOW are "lost-codegen restoration" (most severe).
# `_ZERO` matches both `$0` and `$zero` aliases (same physical register,
# different spelling); same for `_ZWO` (covers `$0`/`$zero` as second/third
# operand). Earlier patterns required the literal `$zero` and missed cases
# like `addu $12, $2, $0` which is semantically identical.
_ZERO = r"\$(?:0|zero)"
LOST_CODEGEN_PATTERNS = (
    re.compile(rf"^addu\s+\$\w+,\s*{_ZERO},\s*{_ZERO}"),       # zero-move (s2=0)
    re.compile(rf"^addu\s+\$\w+,\s*\$\w+,\s*{_ZERO}"),         # reg-move (move rd, rs)
    re.compile(rf"^addu\s+\$\w+,\s*{_ZERO},\s*\$\w+"),         # reg-move (move rd, rs, alt)
)

# 3-instruction BIOS trampoline: addiu $tN, $zero, 0xXX; jr $tN; addiu $tM,...
# Common in text1b_b.c func_80078XXX series. Legitimate but should be
# listed in inline_asm_canonical.txt — recommend adding rather than
# silently allowing.
BIOS_TRAMPOLINE = re.compile(
    r'addiu\s+\$t\d+,\s*\$zero,\s*0x[0-9A-Fa-f]+'
    r'.{0,80}?'
    r'jr\s+\$t\d+'
    r'.{0,80}?'
    r'addiu\s+\$t\d+,\s*\$zero,\s*0x[0-9A-Fa-f]+',
    re.DOTALL,
)


def load_authorized(path="inline_asm_canonical.txt"):
    auth = set()
    p = Path(path)
    if not p.exists():
        return auth
    for line in p.read_text(encoding="utf-8").split("\n"):
        name = line.split("#")[0].strip()
        if name:
            auth.add(name)
    return auth


def load_authorized_at_ref(ref="HEAD", path="inline_asm_canonical.txt"):
    """Load the canonical-asm authorization set from a specific git ref.
    Returns empty set if the file doesn't exist at that ref.

    Critical for closing the same-commit self-authorization loophole:
    `--check-new` must judge both cur and head scans against the HEAD
    auth state, otherwise an agent can add `func_X` to the canonical
    list and add inline asm for func_X in the SAME commit — the cur
    scan would silently skip the function (it's in auth) and the diff
    would show no new cheats."""
    try:
        out = subprocess.run(
            ["git", "show", f"{ref}:{path}"],
            capture_output=True, text=True, check=True,
            encoding="utf-8", errors="replace",
        ).stdout
    except subprocess.CalledProcessError:
        return set()
    auth = set()
    for line in out.split("\n"):
        name = line.split("#")[0].strip()
        if name:
            auth.add(name)
    return auth


def parse_canonical_entries_with_comments(content):
    """Return list of (name, comment) tuples for non-comment, non-blank lines
    in the canonical-asm file. `comment` is the trailing `# ...` text (may
    be empty string if absent)."""
    out = []
    for line in content.split("\n"):
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if "#" in line:
            name = line.split("#", 1)[0].strip()
            comment = line.split("#", 1)[1].strip()
        else:
            name = stripped
            comment = ""
        if name:
            out.append((name, comment))
    return out


# === Evidence-tag grammar for canonical-asm authorization ===
#
# Per user policy: SOTN-grade standard by default — pure-C is the target;
# inline asm and canonical-asm authorization are exceptions requiring
# COMPELLING EVIDENCE that the original code was hand-coded asm, not
# just that *we* found pure-C hard.
#
# Each new entry in inline_asm_canonical.txt must carry one of these
# tags in its `# justification` comment. The auditor verifies the tag is
# present and (where possible) re-runs the relevant scanner to confirm
# the agent's claimed signal matches reality.
#
# Tag forms:
#   gcc-cannot-emit:<reason>       e.g., gcc-cannot-emit:overflow_op,
#                                  gcc-cannot-emit:cop0, gcc-cannot-emit:break,
#                                  gcc-cannot-emit:sp_manipulation
#   custom-abi:<descriptor>        e.g., custom-abi:s0-in-no-save,
#                                  custom-abi:caller-builds-frame
#   hand-coded-signal:<tier>/<sigs>  e.g.,
#                                  hand-coded-signal:STRONG/S1+S2+S3+S4+S5,
#                                  hand-coded-signal:POSSIBLE/S6
#                                  Tier must match scan_hand_coded.py output;
#                                  the auditor re-runs the scanner to verify.
#   cluster-sibling:<func>,jaccard=N.NN
#                                  e.g.,
#                                  cluster-sibling:func_8007F87C,jaccard=1.00
#                                  Named sibling must itself be canonical;
#                                  jaccard must be ≥ 0.7.
#   bios-trampoline                3-insn PSX BIOS A/B/C-vector pattern
#                                  (addiu vector; jr; addiu func-id in slot).
#                                  Verified via existing BIOS_TRAMPOLINE regex.
#
# Multiple tags may co-occur on one entry (e.g., STRONG signal + cluster
# corroboration). At least one must be valid.
TAG_PATTERNS = (
    ("gcc-cannot-emit",
     re.compile(r"\bgcc-cannot-emit:([a-z][a-z0-9_]*)\b")),
    ("custom-abi",
     re.compile(r"\bcustom-abi:([a-z][a-z0-9_\-]*)\b")),
    ("hand-coded-signal",
     re.compile(r"\bhand-coded-signal:(STRONG|POSSIBLE)/((?:S\d+\+?)+)\b")),
    ("cluster-sibling",
     re.compile(r"\bcluster-sibling:(\w+),jaccard=(\d+\.\d+)\b")),
    ("bios-trampoline", re.compile(r"\bbios-trampoline\b")),
)


def parse_evidence_tags(comment):
    """Return list of (tag_kind, raw_match, groups_tuple) for each evidence
    tag found in `comment`. Empty list = no valid tags."""
    out = []
    for kind, pat in TAG_PATTERNS:
        for m in pat.finditer(comment):
            out.append((kind, m.group(0), m.groups()))
    return out


def validate_canonical_evidence_tag(name, comment, current_auth, run_scanner=True):
    """Return (ok, reason) for a single canonical-asm entry's justification.

    Validates the comment carries at least one valid evidence tag from the
    documented grammar. Where the tag is auto-verifiable (hand-coded-signal,
    cluster-sibling), re-runs the relevant scanner / cross-checks against
    current_auth.

    `run_scanner=False` skips the (slow) scan_hand_coded.py invocation,
    useful for `--all` reports where we only want format validation."""
    tags = parse_evidence_tags(comment)
    if not tags:
        return (False,
                "no evidence tag — need one of: gcc-cannot-emit:<reason>, "
                "custom-abi:<desc>, hand-coded-signal:<tier>/<sigs>, "
                "cluster-sibling:<func>,jaccard=N.NN, or bios-trampoline. "
                "See feedback_evidence_driven_authorization.md for format.")

    # Each tag form has its own verifiability constraint.
    for kind, raw, groups in tags:
        if kind == "hand-coded-signal" and run_scanner:
            claimed_tier, claimed_sigs = groups
            ok, msg = _verify_hand_coded_signal(name, claimed_tier, claimed_sigs)
            if not ok:
                return (False, f"hand-coded-signal tag failed verification: {msg}")
        elif kind == "cluster-sibling":
            sib, jaccard = groups
            if sib not in current_auth:
                return (False,
                        f"cluster-sibling:{sib} — named sibling is NOT in "
                        f"inline_asm_canonical.txt; cluster authorization "
                        f"requires the parent be already canonical.")
            try:
                if float(jaccard) < 0.7:
                    return (False,
                            f"cluster-sibling jaccard={jaccard} below 0.7 "
                            f"threshold — too dissimilar to inherit authorization.")
            except ValueError:
                return (False, f"cluster-sibling jaccard must be a number, got: {jaccard}")
        elif kind == "bios-trampoline":
            ok, msg = _verify_bios_trampoline(name)
            if not ok:
                return (False, f"bios-trampoline tag failed verification: {msg}")
    return (True, None)


def _verify_hand_coded_signal(name, claimed_tier, claimed_sigs):
    """Re-run scan_hand_coded.py --single <name> and compare the claimed
    tier+signals against what the scanner reports. Returns (ok, msg)."""
    try:
        proc = subprocess.run(
            ["python3", "tools/scan_hand_coded.py", "--single", name],
            capture_output=True, text=True, timeout=30,
            encoding="utf-8", errors="replace",
            env={**__import__("os").environ, "PYTHONIOENCODING": "utf-8"},
        )
    except (subprocess.TimeoutExpired, FileNotFoundError) as e:
        return (False, f"scanner unavailable: {e}")
    text = proc.stdout
    m = re.search(r"tier=(\w+)\s+score=(\d+)/", text)
    if not m:
        return (False, f"scanner output unparseable for {name}: {text[:200]}")
    scanner_tier = m.group(1)
    if scanner_tier == "LOW":
        return (False,
                f"scanner says tier=LOW for {name} but you claimed "
                f"hand-coded-signal:{claimed_tier} — evidence does not match.")
    # STRONG claim must match STRONG scanner output. POSSIBLE may have STRONG
    # too (downgrade-tolerant), but STRONG cannot be claimed when scanner
    # says only POSSIBLE — that's evidence inflation.
    tier_rank = {"STRONG": 3, "POSSIBLE": 2, "TIGHT_C": 1, "LOW": 0}
    if tier_rank.get(claimed_tier, -1) > tier_rank.get(scanner_tier, -1):
        return (False,
                f"claimed tier {claimed_tier} > scanner tier {scanner_tier} "
                f"for {name} — evidence inflation, REJECT.")
    return (True, None)


def _verify_bios_trampoline(name):
    """Verify the function's asm/funcs/<name>.s starts with the 3-insn
    PSX BIOS trampoline pattern."""
    asm_path = Path(f"asm/funcs/{name}.s")
    if not asm_path.exists():
        return (False, f"asm/funcs/{name}.s not found")
    content = asm_path.read_text(encoding="utf-8", errors="replace")
    if not _trampoline_at_start(content):
        return (False, "asm does not start with the 3-insn BIOS A/B/C-vector trampoline pattern")
    return (True, None)


# === Pure-C attempt log (in commit message) ===
#
# Any commit that adds inline asm, a canonical entry, an aliasing barrier,
# or a new regfix rule for a previously-clean function must document the
# pure-C alternatives the agent tried before reaching for the workaround.
# Format:
#
#   Pure-C attempts:
#     [1] technique=<name> score=<N> outcome=<observable evidence>
#     [2] technique=<name> score=<N> outcome=<observable evidence>
#     [3] technique=<name> score=<N> outcome=<observable evidence>
#
# At least 3 attempts. Each line: technique= (a named technique), score=
# (an integer from compile/permuter), outcome= (text describing observable
# state — dump-text output, permuter plateau, register-pin ignored, etc.).
#
# Vague outcomes ("didn't help", "no improvement", "tried everything") are
# rejected by the LLM auditor; the programmatic check only enforces format.

ATTEMPT_LOG_HEADER = re.compile(r"^Pure-C attempts:\s*$", re.MULTILINE)
ATTEMPT_LINE = re.compile(
    r"^\s*\[(\d+)\]\s+technique=(\S+)\s+score=(-?\d+|N/A)\s+outcome=(.+?)\s*$"
)


def parse_attempt_log(commit_msg):
    """Return list of (idx, technique, score, outcome) parsed from the
    `Pure-C attempts:` block in `commit_msg`. Empty list = no block, OR
    block found but no parseable lines."""
    if not commit_msg:
        return []
    lines = commit_msg.split("\n")
    in_block = False
    out = []
    for line in lines:
        if ATTEMPT_LOG_HEADER.match(line):
            in_block = True
            continue
        if in_block:
            m = ATTEMPT_LINE.match(line)
            if m:
                out.append((int(m.group(1)), m.group(2), m.group(3), m.group(4).strip()))
                continue
            # A blank line or a new block heading ends the attempt-log block.
            if line.strip() == "" or line.endswith(":"):
                # Only end on heading-like lines or repeated blanks
                if not out:
                    continue
                if line.strip() == "" and not out:
                    continue
                break
    return out


# === Aliasing-barrier governance ===
#
# `__asm__ volatile("move %0, %1" : "=r"(x) : "r"(y))` is a documented
# pattern-breaker (feedback_inline_move_aliasing.md). Each instance must
# carry an attached comment block in the source justifying its use:
#
#   /* INLINE_MOVE_ALIASING: pure-C alternatives failed.
#    *   - technique=<name>: <observable failure>
#    *   - technique=<name>: <observable failure>
#    * Per feedback_inline_move_aliasing.md, single-insn escape valve.
#    */
#   __asm__ volatile("move %0, %1" : "=r"(x) : "r"(y));
#
# Comment must be within 8 lines above the asm, contain `INLINE_MOVE_ALIASING:`,
# and list ≥2 bulleted technique=<name>:<outcome> lines.

ALIASING_BARRIER_RE = re.compile(
    r'__asm__\s*(?:volatile\s*)?\(\s*"move\s+%0\s*,\s*%1"',
)
INLINE_MOVE_HEADER = re.compile(r"INLINE_MOVE_ALIASING:")
INLINE_MOVE_BULLET = re.compile(
    r"^\s*\*?\s*[-*]\s+technique=\S+\s*:\s*\S+",
)


def _line_to_pos(content, line_1based):
    """Convert a 1-based line number into a character offset in `content`
    pointing at the start of that line (or 0 if line_1based < 1)."""
    if line_1based < 1:
        return 0
    pos = 0
    for _ in range(line_1based - 1):
        nl = content.find("\n", pos)
        if nl < 0:
            return pos
        pos = nl + 1
    return pos


def scan_aliasing_barriers(content, source_name):
    """Return list of (file, line_no, has_doc, missing_reason) for every
    `move %0, %1` aliasing barrier in `content`."""
    out = []
    lines = content.split("\n")
    for m in ALIASING_BARRIER_RE.finditer(content):
        line_no = content[:m.start()].count("\n") + 1
        has_doc, reason = _has_inline_move_doc(lines, line_no)
        out.append((source_name, line_no, has_doc, reason))
    return out


def _has_inline_move_doc(lines, asm_line_1based):
    """Return (ok, reason). Look back up to 8 lines from `asm_line_1based`
    for an `INLINE_MOVE_ALIASING:` block with ≥2 technique= bullets."""
    asm_idx = asm_line_1based - 1
    start = max(0, asm_idx - 8)
    window = lines[start:asm_idx]
    header_idx = None
    for i, line in enumerate(window):
        if INLINE_MOVE_HEADER.search(line):
            header_idx = i
            break
    if header_idx is None:
        return (False, "no INLINE_MOVE_ALIASING: comment within 8 lines above")
    # Count bullets from header to end of window
    bullets = 0
    for line in window[header_idx:]:
        if INLINE_MOVE_BULLET.match(line):
            bullets += 1
    if bullets < 2:
        return (False,
                f"INLINE_MOVE_ALIASING: comment has only {bullets} `- technique=NAME:OUTCOME` "
                f"bullets; need ≥2 documented failed alternatives")
    return (True, None)


# === Regfix-rule churn detection ===
#
# Per user policy: regfix is a deviation from SOTN bar. Existing rules are
# grandfathered; new rules for functions that have NO rules at HEAD require
# an attempt-log justification. New rules for already-rule-bearing functions
# also require attempt-log but are scrutinized less aggressively.

REGFIX_RULE_RE = re.compile(r"^(\w+):\s+(swap|subst|subst_multi|splice|delete|insert|insert_after|insert_before|insert_label|reorder|fill_delay|drain_delay|\$\d+\s*<->\s*\$\d+)\b")


def parse_regfix_rule_owners(content):
    """Return {func_name: count} of regfix rules per function."""
    from collections import defaultdict
    counts = defaultdict(int)
    for line in content.split("\n"):
        m = REGFIX_RULE_RE.match(line)
        if m:
            counts[m.group(1)] += 1
    return dict(counts)


def detect_new_regfix_rules(head_regfix, cur_regfix):
    """Return (newly_cheating_funcs, churning_funcs):
    - newly_cheating_funcs: functions with rules at cur but NONE at HEAD
    - churning_funcs: functions whose rule count increased
    """
    head_counts = parse_regfix_rule_owners(head_regfix)
    cur_counts = parse_regfix_rule_owners(cur_regfix)
    newly_cheating = {f for f in cur_counts if f not in head_counts}
    churning = {f for f in cur_counts
                if f in head_counts and cur_counts[f] > head_counts[f]}
    return newly_cheating, churning


def scan_regfix_splices(content):
    """Return list of (func, K_orig, N_repl, line_no) for splice rules."""
    cheats = []
    for i, line in enumerate(content.split("\n"), 1):
        m = re.match(r"^(\w+):\s+splice\s+(\d+)\.\.(\d+)\s+(.+)$", line)
        if m:
            func = m.group(1)
            k_orig = int(m.group(3)) - int(m.group(2)) + 1
            n_repl = len(re.findall(r'"[^"]*"', m.group(4)))
            cheats.append((func, k_orig, n_repl, i))
    return cheats


def scan_regfix_subst_multi(content):
    """Return list of (func, N_repl, line_no) for subst_multi rules.

    Syntax: `func: subst_multi "pattern" "line1" "line2" ... [@ idx]`.
    Replacement-line count = (quoted strings) - 1 (pattern is the first arg)."""
    cheats = []
    for i, line in enumerate(content.split("\n"), 1):
        m = re.match(r"^(\w+):\s+subst_multi\s+(.+)$", line)
        if m:
            quoted = re.findall(r'"[^"]*"', m.group(2))
            n_repl = max(0, len(quoted) - 1)
            cheats.append((m.group(1), n_repl, i))
    return cheats


def scan_regfix_wildcard_substs(content):
    """Return {func: wildcard_count} for `subst` rules whose pattern matches
    ANY input (`.*`, `.+`, empty string). These force-overwrite the C-source
    output at index N with the rule's literal text — functionally identical
    to replace_with_asmfile (binary content from the rule, not from C codegen)
    but applied via N small rules per function instead of one big bridge.

    A function with WILDCARD_SUBST_LIMIT+ of these is having its entire
    instruction stream rewritten by regfix — the C source is decorative."""
    from collections import defaultdict
    counts = defaultdict(int)
    for line in content.split("\n"):
        m = re.match(r'^(\w+):\s+subst\s+"([^"]*)"\s+"', line)
        if not m:
            continue
        func, pat = m.group(1), m.group(2)
        if pat in (".*", ".+", ""):
            counts[func] += 1
    return dict(counts)


def scan_regfix_instruction_inserts(content):
    """Return {func: [(op, body, line_no), ...]} for single-instruction
    insert/insert_after/insert_before rules. These inject MIPS instructions
    that didn't come from C compilation. The most common variant (and the
    one that motivated this scanner) is restoring `addu $sN, $0, $zero` —
    a register-zero move that GCC's constant-prop ate.

    Skips:
      - Multi-instruction inserts (contain `\\n` in the quoted body) — those
        are scheduling recipes (delay_slot_fill, etc.) that are checked by
        scan_regfix_cumulative against the 60-line threshold.
      - Label inserts (body ends with `:`) — those are anchor labels, not
        instruction injection.
    """
    from collections import defaultdict
    out = defaultdict(list)
    for m in INSTRUCTION_INSERT_RE.finditer(content):
        func, op, body = m.group(1), m.group(2), m.group(3)
        # Skip multi-insn (encoded via \n inside the quoted body)
        if "\\n" in body:
            continue
        # Skip label-only inserts
        if body.endswith(":"):
            continue
        line_no = content[:m.start()].count("\n") + 1
        out[func].append((op, body, line_no))
    return dict(out)


def _classify_insert_body(body):
    """Categorize a single-instruction insert body. Returns one of:
    'lost_codegen' (zero-move / reg-move — almost always restored GCC output),
    'nop' (scheduling barrier — usually legitimate),
    'other' (everything else).

    The body is the literal quoted string from regfix.txt — `\\t` is two
    characters (escaped tab marker), not an actual tab. Normalize before
    matching so the patterns see real whitespace."""
    norm = body.replace("\\t", " ").replace("\\n", "\n")
    for pat in LOST_CODEGEN_PATTERNS:
        if pat.match(norm):
            return "lost_codegen"
    if norm == "nop":
        return "nop"
    return "other"


def scan_regfix_cumulative(content):
    """Return {func: total_injected_lines} summed across all multi-line ops
    (splice + subst_multi + insert_after/before/label). Used to catch the
    split-evasion attack — splitting one giant splice into N medium-sized
    ones under the per-rule threshold."""
    from collections import defaultdict
    totals = defaultdict(int)
    for func, _k, n_repl, _line in scan_regfix_splices(content):
        totals[func] += n_repl
    for func, n_repl, _line in scan_regfix_subst_multi(content):
        totals[func] += n_repl
    for line in content.split("\n"):
        m = re.match(r"^(\w+):\s+(insert_after|insert_before|insert_label)\s+", line)
        if m:
            totals[m.group(1)] += 1
    return dict(totals)


def _extract_balanced_asm_blocks(text):
    """Yield (start_pos, end_pos, body) for each __asm__(...) block in `text`.
    Properly handles nested parens (offsets like `0($sp)` contain `)`).

    The original `__asm__\\s*\\(...\\([^)]+\\)` regex stops at the first `)`,
    truncating long asm bodies. This generator tracks paren depth."""
    i = 0
    while i < len(text):
        m = re.search(r"__asm__\s*(?:volatile\s*)?\(", text[i:])
        if not m:
            return
        start = i + m.start()
        body_start = i + m.end()
        depth = 1
        j = body_start
        while j < len(text) and depth > 0:
            if text[j] == "(":
                depth += 1
            elif text[j] == ")":
                depth -= 1
            j += 1
        yield start, j, text[body_start:j - 1]
        i = j


def _count_real_insns(block):
    """Count actual asm instructions in a __asm__ block body.

    Strips: directives (.set/.section), labels (glabel/endlabel/alabel/.L1:),
    quote scaffolding. Handles smuggled multi-insn-per-line (\\n separator
    inside a single quoted string)."""
    n = 0
    for src_line in block.split("\n"):
        s = src_line.strip()
        qm = re.match(r'^"(.+)\\n"$', s) or re.match(r'^"(.+)"$', s)
        if not qm:
            continue
        asm = qm.group(1).rstrip("\\n").strip()
        if not asm:
            continue
        if asm.startswith(".") or asm.startswith("glabel") or asm.startswith("endlabel") \
                or asm.startswith("alabel") or asm.endswith(":"):
            continue
        # Each \n inside the quoted string is another insn (smuggling pattern)
        n += asm.count("\\n") + 1
    return n


def _is_whitelisted_insn(insn):
    """Per §6.1: instructions allowed in any inline-asm block (single or multi).

    GTE coprocessor primitives, scheduling barriers (nop), and GTE ops encoded
    as raw words. Anything else is "function logic" and must be split into
    separate single-insn blocks (or expressed in pure C)."""
    insn = insn.strip()
    if insn == "nop":
        return True
    if re.match(r"(cop2|ctc2|mtc2|mfc2|cfc2|lwc2|swc2)\b", insn):
        return True
    # GTE/COP2 .word encodings: 0x4[89A-F]xxxxxx (cop2/ctc2/mtc2/...),
    # 0xC[89A-B]xxxxxx (lwc2), 0xE[89A-B]xxxxxx (swc2)
    if re.match(r"\.word\s+0x[4Cc][89A-Fa-f][0-9A-Fa-f]{6}\b", insn):
        return True
    if re.match(r"\.word\s+0x[Ee][89A-Bb][0-9A-Fa-f]{6}\b", insn):
        return True
    return False


def _enclosing_func_name(text, asm_start):
    """Find the C function definition that contains the asm block at asm_start.
    Returns the function name, or None if at file scope / not in a function."""
    depth = 0
    last_func = None
    pos = 0
    # Walk through the source character by character, tracking brace depth.
    # When depth transitions 0→1, the immediately preceding `name(args)` is
    # the function we're entering.
    func_def_re = re.compile(r"([A-Za-z_]\w*)\s*\([^)]*\)\s*$")
    while pos < asm_start:
        c = text[pos]
        if c == "{":
            if depth == 0:
                # Look back for the function name
                # Scan back through whitespace/newlines to find `name(args)`
                back = pos
                while back > 0 and text[back - 1] in " \t\n":
                    back -= 1
                # Find the matching `(`/`)` pair ending at `back`
                if back > 0 and text[back - 1] == ")":
                    # Find matching `(`
                    d = 1
                    k = back - 2
                    while k > 0 and d > 0:
                        if text[k] == ")":
                            d += 1
                        elif text[k] == "(":
                            d -= 1
                        k -= 1
                    # k+1 is at the `(`, scan back for the identifier
                    name_end = k + 1
                    name_start = name_end
                    while name_start > 0 and (text[name_start - 1].isalnum() or text[name_start - 1] == "_"):
                        name_start -= 1
                    if name_end > name_start:
                        last_func = text[name_start:name_end]
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                last_func = None
        pos += 1
    return last_func if depth > 0 else None


def _has_hardcoded_gpr(insn):
    """True if the instruction contains explicit MIPS GPR references.

    Matches: $0-$31, $zero, $at, $v0/$v1, $a0-$a3, $t0-$t9, $s0-$s7/$fp,
    $k0/$k1, $gp, $sp, $ra. Caller is responsible for first filtering out
    coprocessor instructions (ctc2/cfc2/mtc2/mfc2/lwc2/swc2/.word-encoded
    GTE) — those `$N` operands refer to COP2 registers, not GPRs."""
    return bool(re.search(
        r"\$(?:[0-9]+|zero|at|v[01]|a[0-3]|t[0-9]|s[0-7]|fp|k[01]|gp|sp|ra)\b",
        insn,
    ))


def scan_inline_asm_lost_codegen_injection(content, source_name):
    """Return list of (file, line_no, func_name, insn) for single-instruction
    `__asm__` blocks INSIDE C function bodies whose template:
      1. Contains zero `%N` / `%[name]` operand placeholders, AND
      2. Contains hardcoded GPR register references (`$8`, `$v1`, etc.), AND
      3. Matches a `_classify_insert_body` lost_codegen pattern
         (`addu $rD, $rS, $zero`, `addu $rD, $0, $zero`, `addu $rD, $zero, $rS`).

    These are the lost-codegen cheat migrated from regfix.txt into the C
    source. The bytes still don't come from compilation — they come from the
    asm template verbatim with no GCC tracking. Functionally identical to
    `insert_after "addu $t0,$v1,$zero" @ N` in regfix; we must catch both
    forms.

    Excluded (legitimate single-insn asm — these all pass):
      - Templates with `%N` placeholders (proper codegen hints; GCC picks regs)
      - Coprocessor ops (ctc2/cfc2/mtc2/mfc2/lwc2/swc2/COP2 .word) — `$N` there
        is a COP2 register, not GPR
      - Non-lost-codegen single insns (sll/lui/nop/etc.) — those are scheduling
        / barrier / constant-materialization hints, not the cheat pattern."""
    cheats = []
    for asm_start, _end, body in _extract_balanced_asm_blocks(content):
        if _count_real_insns(body) != 1:
            continue
        func_name = _enclosing_func_name(content, asm_start)
        if func_name is None:
            continue
        if re.search(r"glabel\s+\w+", body):
            continue
        # Split body on `:` to isolate the asm-template portion from the
        # constraint/clobber portions. A barrier like `__asm__("" ::: "$5")`
        # has empty template + clobber list; without this split the raw
        # regex below captures `" ::: "$5` as a fake "instruction" and
        # mis-flags the clobber's `$5` as a hardcoded GPR.
        template_part = body.split(":", 1)[0]
        # Extract the single instruction text from the template part only
        insns = []
        for src_line in template_part.split("\n"):
            s = src_line.strip()
            qm = re.match(r'^"(.*)\\n"$', s) or re.match(r'^"(.*)"$', s)
            if not qm:
                continue
            asm = qm.group(1).rstrip("\\n").strip()
            if not asm or (asm.startswith(".") and not asm.startswith(".word")):
                continue
            if asm.endswith(":") or asm.startswith("glabel") or asm.startswith("endlabel") \
                    or asm.startswith("alabel"):
                continue
            for sub in asm.split("\\n"):
                sub = sub.strip()
                if sub:
                    insns.append(sub)
        if len(insns) != 1:
            continue
        insn = insns[0]
        if re.search(r"%[0-9]+|%\[", insn):
            continue
        if _is_whitelisted_insn(insn):
            continue
        if not _has_hardcoded_gpr(insn):
            continue
        if _classify_insert_body(insn) != "lost_codegen":
            continue
        line_no = content[:asm_start].count("\n") + 1
        cheats.append((source_name, line_no, func_name, insn))
    return cheats


def scan_c_body_smuggled_work(content, source_name):
    """Return list of (file, line_no, insn_count, enclosing_func, insns) for
    multi-instruction __asm__ blocks INSIDE C function bodies that contain at
    least one instruction not on the §6.1 whitelist.

    §6.1's "single instruction per asm block" rule is violated by ANY
    multi-insn block, but the high-severity case is multi-insn blocks
    smuggling non-GTE/non-scheduling work via \\n-concatenation. Those are
    what this scanner flags as cheats."""
    cheats = []
    for asm_start, _, body in _extract_balanced_asm_blocks(content):
        insn_count = _count_real_insns(body)
        if insn_count <= 1:
            continue
        # Skip file-scope blocks (those go through scan_inline_asm_bodies)
        if _enclosing_func_name(content, asm_start) is None:
            continue
        # Skip blocks with glabel (file-scope cheats, in case is_file_scope missed)
        if re.search(r"glabel\s+\w+", body):
            continue
        # Extract instruction lines and check whitelist
        insns = []
        for src_line in body.split("\n"):
            s = src_line.strip()
            qm = re.match(r'^"(.+)\\n"$', s) or re.match(r'^"(.+)"$', s)
            if not qm:
                continue
            asm = qm.group(1).rstrip("\\n").strip()
            if not asm or asm.startswith(".") and not asm.startswith(".word"):
                continue
            if asm.endswith(":") or asm.startswith("glabel") or asm.startswith("endlabel") or asm.startswith("alabel"):
                continue
            # Handle smuggled multi-insn-per-line via \n
            for sub in asm.split("\\n"):
                sub = sub.strip()
                if sub:
                    insns.append(sub)
        has_smuggled_work = any(not _is_whitelisted_insn(i) for i in insns)
        if not has_smuggled_work:
            continue
        line_no = content[:asm_start].count("\n") + 1
        func = _enclosing_func_name(content, asm_start)
        cheats.append((source_name, line_no, insn_count, func, insns))
    return cheats


def _trampoline_at_start(block):
    """Return True if the trampoline pattern (addiu $tN, $zero, 0xVEC;
    jr $tN; addiu $tM, $zero, FUNC#) appears at the START of the function
    body — within the first ~5 instruction lines after glabel.

    This catches BIOS-trampoline-style functions regardless of trailing
    content (padding nops, embedded data tables, jumptable installation
    code, etc.). The original `insn_count <= 12 and BIOS_TRAMPOLINE.search()`
    check missed func_800831D8 (trampoline + 3 padding nops, 13 lines) and
    func_8008D060 (trampoline + 50-line module table)."""
    # Find glabel position; only look in the first ~400 chars after it
    gm = re.search(r"glabel\s+\w+", block)
    if not gm:
        return False
    # The first 400 chars after glabel covers: optional .set directives,
    # the 3 trampoline instructions, and small amounts of whitespace. The
    # original 80-char-gap regex requires the 3 pattern parts within tight
    # proximity; we just additionally constrain that they're NEAR the start.
    head = block[gm.end():gm.end() + 400]
    return bool(BIOS_TRAMPOLINE.search(head))


def scan_inline_asm_bodies(content, source_name):
    """Return list of (file, line_no, asm_lines, func_name, is_bios) for
    `__asm__` blocks containing `glabel funcname`.

    `is_bios=True` covers any block whose body STARTS with the 3-instruction
    BIOS trampoline pattern, regardless of trailing content. Padding nops,
    embedded module tables, jumptable installation code — all valid trailing
    content for a BIOS-related function. The trampoline-at-start is the
    discriminator."""
    cheats = []
    for m in re.finditer(r"__asm__\s*(?:volatile\s*)?\(([^)]+)\)", content):
        block = m.group(1)
        glabel_match = re.search(r"glabel\s+(\w+)", block)
        if not glabel_match:
            continue
        fname = glabel_match.group(1)
        insn_count = block.count("\\n")
        line_no = content[:m.start()].count("\n") + 1
        is_bios = _trampoline_at_start(block)
        cheats.append((source_name, line_no, insn_count, fname, is_bios))
    return cheats


def get_current_cheats(auth):
    """Scan current working tree. Return (splice_funcs, inline_funcs,
    bios_funcs, c_body_funcs, instruction_insert_funcs, asm_injection_funcs).

    `splice_funcs` covers all regfix-injection cheats (large splice / large
    subst_multi / cumulative-volume / wildcard subst). `inline_funcs` covers
    file-scope __asm__(glabel) cheats. `c_body_funcs` covers multi-instruction
    __asm__ blocks INSIDE C function bodies that smuggle non-GTE/non-scheduling
    work via \\n-concatenation. `instruction_insert_funcs` covers per-function
    counts of single-instruction lost-codegen insert rules in regfix.txt (zero-
    move / reg-move — these inject what GCC's optimizer ate, sub-threshold for
    the cumulative line check but still asm injection). `asm_injection_funcs`
    covers single-instruction `__asm__` blocks in C bodies whose template is
    a hardcoded lost-codegen-pattern instruction with no `%N` placeholders
    (the same cheat moved from regfix to C source)."""
    splice_funcs = set()
    inline_funcs = set()
    bios_funcs = set()
    c_body_funcs = set()
    instruction_insert_funcs = {}
    asm_injection_funcs = {}

    # Combine stage1 + stage2 regfix content for unified per-function checks.
    regfix_content = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = Path(fname)
        if p.exists():
            regfix_content += p.read_text(encoding="utf-8", errors="ignore") + "\n"
    if regfix_content:
        for func, k, n, line in scan_regfix_splices(regfix_content):
            if n >= LARGE_SPLICE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, n, line in scan_regfix_subst_multi(regfix_content):
            if n >= LARGE_SUBST_MULTI_LINES and func not in auth:
                splice_funcs.add(func)
        for func, total in scan_regfix_cumulative(regfix_content).items():
            if total >= CUMULATIVE_RULE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, wcount in scan_regfix_wildcard_substs(regfix_content).items():
            if wcount >= WILDCARD_SUBST_LIMIT and func not in auth:
                splice_funcs.add(func)
        # Track lost-codegen single-instruction inserts per function.
        # Authorized functions (inline_asm_canonical.txt) skip the count.
        for func, entries in scan_regfix_instruction_inserts(regfix_content).items():
            if func in auth:
                continue
            lost = sum(1 for _op, body, _ln in entries
                       if _classify_insert_body(body) == "lost_codegen")
            if lost > 0:
                instruction_insert_funcs[func] = lost

    for src in sorted(Path("src").glob("*.c")):
        text = src.read_text(encoding="utf-8")
        for f, l, n, fname, is_bios in scan_inline_asm_bodies(text, src.name):
            if n < GLABEL_BODY_LINES:
                continue
            if fname in auth:
                continue
            if is_bios:
                bios_funcs.add(fname)
            else:
                inline_funcs.add(fname)
        for f, l, n, fname, _insns in scan_c_body_smuggled_work(text, src.name):
            if fname and fname not in auth:
                c_body_funcs.add(fname)
        for f, l, fname, _insn in scan_inline_asm_lost_codegen_injection(text, src.name):
            if fname and fname not in auth:
                asm_injection_funcs[fname] = asm_injection_funcs.get(fname, 0) + 1

    return (splice_funcs, inline_funcs, bios_funcs, c_body_funcs,
            instruction_insert_funcs, asm_injection_funcs)


def get_head_cheats(auth):
    """Same scan but against HEAD revision (for --check-new diff)."""
    splice_funcs = set()
    inline_funcs = set()
    c_body_funcs = set()
    instruction_insert_funcs = {}
    asm_injection_funcs = {}

    head_regfix = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        try:
            head_regfix += subprocess.run(
                ["git", "show", f"HEAD:{fname}"],
                capture_output=True, text=True, check=True, encoding="utf-8", errors="replace",
            ).stdout + "\n"
        except subprocess.CalledProcessError:
            pass
    if head_regfix:
        for func, k, n, line in scan_regfix_splices(head_regfix):
            if n >= LARGE_SPLICE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, n, line in scan_regfix_subst_multi(head_regfix):
            if n >= LARGE_SUBST_MULTI_LINES and func not in auth:
                splice_funcs.add(func)
        for func, total in scan_regfix_cumulative(head_regfix).items():
            if total >= CUMULATIVE_RULE_LINES and func not in auth:
                splice_funcs.add(func)
        for func, wcount in scan_regfix_wildcard_substs(head_regfix).items():
            if wcount >= WILDCARD_SUBST_LIMIT and func not in auth:
                splice_funcs.add(func)
        for func, entries in scan_regfix_instruction_inserts(head_regfix).items():
            if func in auth:
                continue
            lost = sum(1 for _op, body, _ln in entries
                       if _classify_insert_body(body) == "lost_codegen")
            if lost > 0:
                instruction_insert_funcs[func] = lost

    # All .c files tracked at HEAD
    src_list = subprocess.run(
        ["git", "ls-tree", "-r", "--name-only", "HEAD", "src/"],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
    ).stdout.split("\n")
    for src_path in src_list:
        if not src_path.endswith(".c"):
            continue
        try:
            head_content = subprocess.run(
                ["git", "show", f"HEAD:{src_path}"],
                capture_output=True, text=True, check=True, encoding="utf-8", errors="replace",
            ).stdout
        except subprocess.CalledProcessError:
            continue
        for f, l, n, fname, is_bios in scan_inline_asm_bodies(head_content, src_path):
            if n < GLABEL_BODY_LINES:
                continue
            if fname in auth or is_bios:
                continue
            inline_funcs.add(fname)
        for f, l, n, fname, _insns in scan_c_body_smuggled_work(head_content, src_path):
            if fname and fname not in auth:
                c_body_funcs.add(fname)
        for f, l, fname, _insn in scan_inline_asm_lost_codegen_injection(head_content, src_path):
            if fname and fname not in auth:
                asm_injection_funcs[fname] = asm_injection_funcs.get(fname, 0) + 1

    return (splice_funcs, inline_funcs, c_body_funcs, instruction_insert_funcs,
            asm_injection_funcs)


def cmd_all():
    auth = load_authorized()
    # Combine stage1 + stage2 regfix files for unified per-function checks.
    regfix_content = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = Path(fname)
        if p.exists():
            regfix_content += p.read_text(encoding="utf-8", errors="ignore") + "\n"
    splices = scan_regfix_splices(regfix_content)
    full_splices = [s for s in splices if s[2] >= LARGE_SPLICE_LINES]
    unauth_splices = [s for s in full_splices if s[0] not in auth]

    subst_multis = scan_regfix_subst_multi(regfix_content)
    large_subst_multis = [s for s in subst_multis if s[1] >= LARGE_SUBST_MULTI_LINES]
    unauth_subst_multis = [s for s in large_subst_multis if s[0] not in auth]

    cumulative = scan_regfix_cumulative(regfix_content)
    over_cumulative = sorted(
        ((fn, total) for fn, total in cumulative.items()
         if total >= CUMULATIVE_RULE_LINES and fn not in auth),
        key=lambda x: -x[1],
    )

    wildcard = scan_regfix_wildcard_substs(regfix_content)
    over_wildcard = sorted(
        ((fn, n) for fn, n in wildcard.items()
         if n >= WILDCARD_SUBST_LIMIT and fn not in auth),
        key=lambda x: -x[1],
    )

    inserts = scan_regfix_instruction_inserts(regfix_content)
    insert_lost_per_func = {}
    for fn, entries in inserts.items():
        if fn in auth:
            continue
        lost = [(op, body, ln) for op, body, ln in entries
                if _classify_insert_body(body) == "lost_codegen"]
        if lost:
            insert_lost_per_func[fn] = lost
    insert_lost_sorted = sorted(insert_lost_per_func.items(), key=lambda x: -len(x[1]))

    print(f"=== regfix.txt splice rules ===")
    print(f"  {len(splices)} total, {len(full_splices)} full-body (>= {LARGE_SPLICE_LINES} replacement lines)")
    print(f"  {len(unauth_splices)} UNAUTHORIZED:")
    for func, k, n, line in sorted(unauth_splices, key=lambda x: -x[2]):
        print(f"    line {line:5d}  {func:32s}  orig_K={k:4d}  replace_N={n:4d}")

    print()
    print(f"=== regfix.txt subst_multi rules ===")
    print(f"  {len(subst_multis)} total, {len(large_subst_multis)} large (>= {LARGE_SUBST_MULTI_LINES} replacement lines)")
    print(f"  {len(unauth_subst_multis)} UNAUTHORIZED:")
    for func, n, line in sorted(unauth_subst_multis, key=lambda x: -x[1]):
        print(f"    line {line:5d}  {func:32s}  replace_N={n:4d}")

    print()
    print(f"=== regfix.txt cumulative rule volume per function ===")
    print(f"  {sum(1 for v in cumulative.values() if v > 0)} functions have multi-line rules")
    print(f"  threshold: {CUMULATIVE_RULE_LINES} cumulative lines (splice + subst_multi + insert_*)")
    print(f"  {len(over_cumulative)} UNAUTHORIZED over threshold:")
    for fn, total in over_cumulative:
        print(f"    {fn:32s}  total_lines={total}")

    print()
    print(f"=== regfix wildcard `subst \".*\"` rules per function ===")
    print(f"  {sum(1 for v in wildcard.values() if v > 0)} functions have wildcard substs")
    print(f"  threshold: {WILDCARD_SUBST_LIMIT}+ wildcard substs = force-overwrite cheat")
    print(f"  {len(over_wildcard)} UNAUTHORIZED over threshold:")
    for fn, n in over_wildcard:
        print(f"    {fn:32s}  wildcard_count={n}")

    print()
    print(f"=== regfix single-instruction lost-codegen inserts per function ===")
    print(f"  `insert_after \"addu $sN,$0,$zero\" @ idx` and similar — restore an")
    print(f"  instruction GCC's optimizer ate (constant-prop / dead-store).")
    print(f"  These are asm injection (1 insn each), below the cumulative-{CUMULATIVE_RULE_LINES}")
    print(f"  threshold but still bytes not from C compilation.")
    print(f"  {len(insert_lost_per_func)} functions have lost-codegen inserts:")
    for fn, entries in insert_lost_sorted:
        print(f"    {fn:32s}  count={len(entries)}")
        for op, body, ln in entries[:3]:
            print(f"      line {ln:5d}  {op:14s}  {body!r}")
        if len(entries) > 3:
            print(f"      ... and {len(entries) - 3} more")

    print()
    print(f"=== Inline __asm__ glabel function bodies in src/*.c ===")
    inline_all = []
    for src in sorted(Path("src").glob("*.c")):
        inline_all.extend(
            scan_inline_asm_bodies(src.read_text(encoding="utf-8"), src.name)
        )
    significant = [b for b in inline_all if b[2] >= GLABEL_BODY_LINES]
    auth_blocks = [b for b in significant if b[3] in auth]
    bios = [b for b in significant if b[4] and b[3] not in auth]
    unauth = [b for b in significant if not b[4] and b[3] not in auth]
    print(f"  {len(significant)} with >= {GLABEL_BODY_LINES} lines:")
    print(f"    {len(auth_blocks):4d} authorized (in inline_asm_canonical.txt)")
    print(f"    {len(bios):4d} BIOS trampolines (3-insn pattern; should be added to authorized list)")
    print(f"    {len(unauth):4d} UNAUTHORIZED")
    for f, l, n, fname, _ in sorted(unauth, key=lambda x: -x[2])[:25]:
        print(f"    {f:25s}:{l:6d}  {fname:25s}  ~{n} lines")
    if len(unauth) > 25:
        print(f"    ... and {len(unauth) - 25} more")

    print()
    print(f"=== Multi-instruction __asm__ inside C function bodies ===")
    c_body_all = []
    for src in sorted(Path("src").glob("*.c")):
        c_body_all.extend(
            scan_c_body_smuggled_work(src.read_text(encoding="utf-8"), src.name)
        )
    c_body_unauth = [c for c in c_body_all if c[3] and c[3] not in auth]
    print(f"  {len(c_body_all)} multi-insn blocks containing non-§6.1-whitelisted instructions")
    print(f"  {len(c_body_unauth)} UNAUTHORIZED (host function not in inline_asm_canonical.txt):")
    for f, l, n, fname, insns in sorted(c_body_unauth, key=lambda x: -x[2])[:15]:
        print(f"    {f:25s}:{l:6d}  in {fname:30s}  ({n} insns)")
    if len(c_body_unauth) > 15:
        print(f"    ... and {len(c_body_unauth) - 15} more")

    print()
    print(f"=== Inline-asm lost-codegen INJECTIONS inside C function bodies ===")
    print(f"  Single-instruction __asm__ blocks with hardcoded `$N` registers (no `%N`")
    print(f"  placeholders) emitting `addu RD,RS,$zero`-style moves — the same lost-")
    print(f"  codegen cheat migrated from regfix.txt into the C source. Bytes still")
    print(f"  don't come from compilation; GCC has no view into the operation.")
    asm_inj_all = []
    for src in sorted(Path("src").glob("*.c")):
        asm_inj_all.extend(
            scan_inline_asm_lost_codegen_injection(src.read_text(encoding="utf-8"), src.name)
        )
    asm_inj_unauth = [c for c in asm_inj_all if c[2] and c[2] not in auth]
    asm_inj_by_func = {}
    for f, l, fname, insn in asm_inj_unauth:
        asm_inj_by_func.setdefault(fname, []).append((f, l, insn))
    print(f"  {len(asm_inj_all)} injection blocks across all src/*.c")
    print(f"  {len(asm_inj_by_func)} UNAUTHORIZED functions (not in inline_asm_canonical.txt):")
    for fname, entries in sorted(asm_inj_by_func.items(), key=lambda x: -len(x[1])):
        print(f"    {fname:32s}  count={len(entries)}")
        for f, l, insn in entries[:3]:
            print(f"      {f}:{l}  __asm__(\"{insn}\")")
        if len(entries) > 3:
            print(f"      ... and {len(entries) - 3} more")

    print()
    # De-dup across regfix categories: a single func can hit splice + subst_multi
    # + cumulative + wildcard + lost-codegen inserts + inline-asm-injection at once.
    regfix_unauth_funcs = (
        {s[0] for s in unauth_splices}
        | {s[0] for s in unauth_subst_multis}
        | {fn for fn, _ in over_cumulative}
        | {fn for fn, _ in over_wildcard}
        | set(insert_lost_per_func.keys())
    )
    c_body_unauth_funcs = {c[3] for c in c_body_unauth}
    asm_inj_unauth_funcs = set(asm_inj_by_func.keys())
    total_unauth = (len(regfix_unauth_funcs) + len(unauth)
                    + len(c_body_unauth_funcs) + len(asm_inj_unauth_funcs))
    if total_unauth > 0:
        print(f"TOTAL UNAUTHORIZED ASM CHEATS: {total_unauth}")
        print(f"  ({len(bios)} BIOS trampolines should also be added to inline_asm_canonical.txt)")
    else:
        print("No unauthorized asm cheats. All inline-asm bodies and regfix injections are accounted for.")
    return 0


def cmd_func(name):
    auth = load_authorized()
    if name in auth:
        return 0
    regfix_content = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = Path(fname)
        if p.exists():
            regfix_content += p.read_text(encoding="utf-8", errors="ignore") + "\n"
    for func, k, n, line in scan_regfix_splices(regfix_content):
        if func == name and n >= LARGE_SPLICE_LINES:
            print(f"UNAUTHORIZED: {name} has full-body splice in regfix.txt:{line} "
                  f"({n}-line replacement). Add to inline_asm_canonical.txt or do real C decomp.",
                  file=sys.stderr)
            return 1
    for func, n, line in scan_regfix_subst_multi(regfix_content):
        if func == name and n >= LARGE_SUBST_MULTI_LINES:
            print(f"UNAUTHORIZED: {name} has large subst_multi in regfix.txt:{line} "
                  f"({n}-line replacement). Add to inline_asm_canonical.txt or do real C decomp.",
                  file=sys.stderr)
            return 1
    total = scan_regfix_cumulative(regfix_content).get(name, 0)
    if total >= CUMULATIVE_RULE_LINES:
        print(f"UNAUTHORIZED: {name} has {total} cumulative regfix-injected lines "
              f"(threshold {CUMULATIVE_RULE_LINES}). Looks like split-evasion of the "
              f"per-rule splice threshold. Add to inline_asm_canonical.txt or do real C decomp.",
              file=sys.stderr)
        return 1
    wcount = scan_regfix_wildcard_substs(regfix_content).get(name, 0)
    if wcount >= WILDCARD_SUBST_LIMIT:
        print(f"UNAUTHORIZED: {name} has {wcount} wildcard `subst \".*\"` rules in regfix.txt/"
              f"regfix_stage2.txt (threshold {WILDCARD_SUBST_LIMIT}). These force-overwrite "
              f"the C source's compiled output line-by-line — functionally equivalent to "
              f"replace_with_asmfile. Remove the substs and write real C, or add to "
              f"inline_asm_canonical.txt with authorization.",
              file=sys.stderr)
        return 1
    inserts = scan_regfix_instruction_inserts(regfix_content).get(name, [])
    lost = [(op, body, ln) for op, body, ln in inserts
            if _classify_insert_body(body) == "lost_codegen"]
    if lost:
        print(f"UNAUTHORIZED: {name} has {len(lost)} lost-codegen single-instruction "
              f"insert rule(s) in regfix.txt:", file=sys.stderr)
        for op, body, ln in lost:
            print(f"  line {ln:5d}  {op:14s}  {body!r}", file=sys.stderr)
        print(f"These inject MIPS instructions that didn't come from C compilation "
              f"(usually restoring what GCC's optimizer ate). Add to "
              f"inline_asm_canonical.txt or rewrite the C to defeat the optimizer "
              f"(register pinning, structure change, etc.).", file=sys.stderr)
        return 1
    for src in sorted(Path("src").glob("*.c")):
        text = src.read_text(encoding="utf-8")
        for f, l, n, fname, is_bios in scan_inline_asm_bodies(text, src.name):
            if fname == name and n >= GLABEL_BODY_LINES and not is_bios:
                print(f"UNAUTHORIZED: {name} has inline __asm__ body in {f}:{l} "
                      f"(~{n} lines). Add to inline_asm_canonical.txt or do real C decomp.",
                      file=sys.stderr)
                return 1
        for f, l, n, fname, _insns in scan_c_body_smuggled_work(text, src.name):
            if fname == name:
                print(f"UNAUTHORIZED: {name} has multi-insn __asm__ block in {f}:{l} "
                      f"({n} insns) containing non-§6.1-whitelisted instructions. "
                      f"Split into per-instruction __asm__ blocks or do real C decomp.",
                      file=sys.stderr)
                return 1
        injections = [(f, l, insn) for f, l, fn, insn
                      in scan_inline_asm_lost_codegen_injection(text, src.name)
                      if fn == name]
        if injections:
            print(f"UNAUTHORIZED: {name} has {len(injections)} inline-asm "
                  f"lost-codegen INJECTION(s) (hardcoded `$N` registers, no `%N` "
                  f"placeholders, body matches lost_codegen pattern):", file=sys.stderr)
            for f, l, insn in injections:
                print(f"  {f}:{l}  __asm__(\"{insn}\")", file=sys.stderr)
            print(f"These emit the same MIPS bytes as `insert_after \"addu RD,RS,$zero\" @ N` "
                  f"would — just sourced from inline asm with no GCC tracking. The cheat "
                  f"under a different name (per skill §7 anti-pattern). Use proper operand "
                  f"constraints (`%0`/`%1` with `=r`/`r` binding to `register T x asm(\"$N\")` "
                  f"variables) so GCC tracks the operation, or restructure the C to defeat "
                  f"the optimizer.", file=sys.stderr)
            return 1
    return 0


def cmd_check_new(commit_msg=None):
    """Return 1 if the working tree introduces new cheats since HEAD.

    Completion policy (2026-05-21, [[completion-standard]]): the only acceptable
    end states are COMPLETED-C (pure C) and COMPLETED-INLINE-ASM-CANONICAL
    (whole-function hand-asm / GTE / BIOS, evidence-gated). Any NET-NEW regfix
    rule, register-asm pin, or cheat-asm (move-aliasing / barrier) is BLOCKED
    outright -- these are diagnostic-only, never committed. The old "document
    it / log 3 attempts and it's allowed" escape valves for G4/G5 are removed.

    Gates, in order:
      G1.  Same-commit self-authorization detection (HEAD auth in both scans)
      G2.  New canonical-asm entry without valid evidence tag
      G3.  Standard cheat-pattern delta (splice/wildcard/lost-codegen/injection)
      G4.  New inline-move aliasing barrier (BLOCKED -- no doc escape valve)
      G5.  New regfix on a clean function OR churn on a rule-bearing one (BLOCKED)
      G5b. New register-asm pin (BLOCKED) -- except whole-function canonical bodies
      G6.  Canonical authorization (G2) still requires a Pure-C attempts: log
    """
    head_auth = load_authorized_at_ref("HEAD")
    cur_auth = load_authorized()  # working tree

    violations = []  # list of (gate, message)

    # === G2: validate evidence tags on NEW canonical entries ===
    new_auth_names = cur_auth - head_auth
    new_auth_with_comments = []
    if new_auth_names:
        canonical_content = Path("inline_asm_canonical.txt").read_text(
            encoding="utf-8", errors="replace")
        for name, comment in parse_canonical_entries_with_comments(canonical_content):
            if name in new_auth_names:
                new_auth_with_comments.append((name, comment))
                ok, reason = validate_canonical_evidence_tag(
                    name, comment, cur_auth)
                if not ok:
                    violations.append(("canonical_evidence_tag",
                        f"new canonical-asm entry `{name}` -- {reason}"))

    # === G1+G3: cheat scans using HEAD auth so newly-authorized funcs
    # do NOT immunize themselves. A function added to canonical THIS
    # commit + new inline asm THIS commit → cur scan flags it as
    # unauthorized → set diff shows it as new → BLOCKED. This is the
    # mechanical replacement for the user-authorization-phrase requirement.
    (cur_splice, cur_inline, cur_bios, cur_c_body, cur_inserts,
     cur_asm_inj) = get_current_cheats(head_auth)
    (head_splice, head_inline, head_c_body, head_inserts,
     head_asm_inj) = get_head_cheats(head_auth)

    new_splices = cur_splice - head_splice
    new_inline = cur_inline - head_inline
    new_c_body = cur_c_body - head_c_body
    new_inserts = {}
    for fn, n in cur_inserts.items():
        head_n = head_inserts.get(fn, 0)
        if n > head_n:
            new_inserts[fn] = (head_n, n)
    new_asm_inj = {}
    for fn, n in cur_asm_inj.items():
        head_n = head_asm_inj.get(fn, 0)
        if n > head_n:
            new_asm_inj[fn] = (head_n, n)

    # Surface same-commit self-auth explicitly (it's the most common
    # exploit pattern and the error message should name it).
    self_auths = (new_inline | new_c_body | set(new_asm_inj.keys())
                  | set(new_inserts.keys())) & new_auth_names
    for fn in sorted(self_auths):
        violations.append(("same_commit_self_auth",
            f"function `{fn}` newly added to inline_asm_canonical.txt AND "
            f"newly has inline asm / regfix injection in src/ -- this is "
            f"same-commit self-authorization. Split into two commits: "
            f"(1) add the canonical entry with valid evidence tag, "
            f"(2) AFTER the first commit lands, add the inline asm."))

    # Other new cheats (not self-auth — those are reported above)
    for fn in sorted(new_splices):
        violations.append(("regfix_injection",
            f"new regfix injection for `{fn}` (large splice / subst_multi / "
            f"cumulative ≥ {CUMULATIVE_RULE_LINES} lines / wildcard substs)"))
    for fn in sorted(new_inline - new_auth_names):
        violations.append(("inline_asm_glabel",
            f"new inline `__asm__(\".section .text\\n... glabel {fn} ...\")` "
            f"body in src/*.c; if hand-coded original, add to "
            f"inline_asm_canonical.txt with evidence tag (see G2)"))
    for fn in sorted(new_c_body - new_auth_names):
        violations.append(("c_body_multi_insn",
            f"new multi-insn `__asm__` block in C body of `{fn}` containing "
            f"non-§6.1-whitelisted instructions"))
    for fn in sorted(set(new_inserts.keys()) - new_auth_names):
        head_n, cur_n = new_inserts[fn]
        violations.append(("lost_codegen_insert",
            f"new lost-codegen single-insn regfix insert for `{fn}` "
            f"(HEAD={head_n}, now={cur_n}, +{cur_n - head_n})"))
    for fn in sorted(set(new_asm_inj.keys()) - new_auth_names):
        head_n, cur_n = new_asm_inj[fn]
        violations.append(("inline_asm_injection",
            f"new inline-asm lost-codegen INJECTION in C body of `{fn}` "
            f"(HEAD={head_n}, now={cur_n}, +{cur_n - head_n}); hardcoded "
            f"`$N` register, no `%N` placeholders"))

    # === G4: new aliasing barriers must have INLINE_MOVE_ALIASING comments ===
    # Count barriers per (file, host-function) pair to be resilient against
    # line-number shifts from edits elsewhere in the file. A pair is "new"
    # only when cur count > head count.
    def barriers_by_func(content, source_name):
        from collections import defaultdict
        out = defaultdict(int)
        for src_name, line_no, _has_doc, _reason in scan_aliasing_barriers(
                content, source_name):
            fname = _enclosing_func_name(content, _line_to_pos(content, line_no))
            out[(source_name, fname)] += 1
        return out

    head_barriers_by_func = {}
    try:
        head_src_paths = subprocess.run(
            ["git", "ls-tree", "-r", "--name-only", "HEAD", "src/"],
            capture_output=True, text=True, check=True,
            encoding="utf-8", errors="replace").stdout.split("\n")
        for path in head_src_paths:
            if not path.endswith(".c"):
                continue
            try:
                head_content = subprocess.run(
                    ["git", "show", f"HEAD:{path}"],
                    capture_output=True, text=True, check=True,
                    encoding="utf-8", errors="replace").stdout
            except subprocess.CalledProcessError:
                continue
            basename = path.split("/")[-1]
            for key, n in barriers_by_func(head_content, basename).items():
                head_barriers_by_func[key] = head_barriers_by_func.get(key, 0) + n
    except subprocess.CalledProcessError:
        pass

    # Completion policy (2026-05-21): inline-move aliasing is cheat-asm register
    # coercion — diagnostic-only, NEVER an allowed end state. Block EVERY new
    # barrier regardless of documentation (the old INLINE_MOVE_ALIASING: doc
    # escape valve is retired). Existing barriers are grandfathered via head_n.
    new_barriers = []
    for src_path in sorted(Path("src").glob("*.c")):
        content = src_path.read_text(encoding="utf-8", errors="replace")
        # Iterate scan results in order so reports identify the actual file:line.
        cur_barriers = list(scan_aliasing_barriers(content, src_path.name))
        for src_name, line_no, has_doc, reason in cur_barriers:
            fname = _enclosing_func_name(content, _line_to_pos(content, line_no))
            key = (src_name, fname)
            head_n = head_barriers_by_func.get(key, 0)
            # Charge this barrier against head_n first (grandfathered),
            # then any excess is a NEW barrier.
            if head_n > 0:
                head_barriers_by_func[key] = head_n - 1
                continue
            new_barriers.append((src_name, line_no, fname))

    for src_name, line_no, fname in new_barriers:
        violations.append(("new_inline_move_aliasing",
            f"new `__asm__ volatile(\"move %0, %1\" ...)` inline-move aliasing in "
            f"`{fname}` at {src_name}:{line_no}. Per completion policy this cheat-asm "
            f"register coercion is NOT an allowed end state (no INLINE_MOVE_ALIASING: "
            f"doc escape valve any more) -- it is diagnostic-only and must be stripped "
            f"before commit. Find the C structure that makes GCC emit the move, or use "
            f"canonical-asm authorization only for a physically un-compilable construct."))

    # === G5: new regfix rules ===
    head_regfix = ""
    cur_regfix = ""
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = Path(fname)
        if p.exists():
            cur_regfix += p.read_text(encoding="utf-8", errors="ignore") + "\n"
        try:
            head_regfix += subprocess.run(
                ["git", "show", f"HEAD:{fname}"],
                capture_output=True, text=True, check=True,
                encoding="utf-8", errors="replace").stdout + "\n"
        except subprocess.CalledProcessError:
            pass

    newly_cheating_via_regfix, churning_regfix = detect_new_regfix_rules(
        head_regfix, cur_regfix)
    # Don't double-report: if function is already flagged in self-auth or
    # standard cheat sets, skip.
    already_flagged = (self_auths | new_splices | set(new_inserts.keys())
                       | set(new_asm_inj.keys()))
    for fn in sorted(newly_cheating_via_regfix - already_flagged):
        violations.append(("regfix_new_clean_func",
            f"new regfix rule for `{fn}` which had ZERO rules at HEAD. "
            f"Per completion policy (2026-05-21) regfix is NEVER an end state -- this "
            f"is BLOCKED, not satisfiable by an attempt log. regfix is diagnostic-only; "
            f"strip it and find the C structure, or use canonical-asm authorization "
            f"only for a physically un-compilable construct."))
    for fn in sorted(churning_regfix - already_flagged - newly_cheating_via_regfix):
        violations.append(("regfix_churn",
            f"net-new regfix rule(s) added to `{fn}` (already had rules at HEAD). "
            f"Per completion policy, adding regfix is BLOCKED -- every rule is debt to "
            f"remove, not extend. Drive `{fn}` toward ZERO rules."))

    # === Completion gate: new register-asm pins are BLOCKED (2026-05-21) ===
    # `register T x asm("$N")` pins are cheat-asm debt, never an end state. They
    # are diagnostic-only; the committed match must reach the register via C
    # structure. Whole-function canonical-asm bodies (in inline_asm_canonical.txt)
    # are exempt.
    PIN_RE = re.compile(r'\bregister\b[^=;]*\basm\s*\(\s*"\$')

    def pins_by_func(text, source_name):
        from collections import defaultdict
        out = defaultdict(int)
        for ln_no, line in enumerate(text.split("\n"), 1):
            if PIN_RE.search(line):
                fn = _enclosing_func_name(text, _line_to_pos(text, ln_no))
                out[(source_name, fn)] += 1
        return dict(out)

    new_pins = []
    try:
        _pin_head_paths = head_src_paths  # reuse the fetch from G4 if available
    except NameError:
        _pin_head_paths = None
    if _pin_head_paths is not None:  # fail-open: skip pin gate if HEAD unavailable
        head_pins_by_func = {}
        for path in _pin_head_paths:
            if not path.endswith(".c"):
                continue
            try:
                head_content = subprocess.run(
                    ["git", "show", f"HEAD:{path}"],
                    capture_output=True, text=True, check=True,
                    encoding="utf-8", errors="replace").stdout
            except subprocess.CalledProcessError:
                continue
            for key, n in pins_by_func(head_content, path.split("/")[-1]).items():
                head_pins_by_func[key] = head_pins_by_func.get(key, 0) + n
        for src_path in sorted(Path("src").glob("*.c")):
            content = src_path.read_text(encoding="utf-8", errors="replace")
            for key, n in pins_by_func(content, src_path.name).items():
                sname, fname = key
                if fname in cur_auth:   # canonical-asm bodies are exempt
                    continue
                head_n = head_pins_by_func.get(key, 0)
                if n > head_n:
                    new_pins.append((sname, fname, head_n, n))
    for sname, fname, head_n, n in sorted(new_pins):
        violations.append(("new_register_pin",
            f"new `register T x asm(\"$N\")` pin in `{fname}` ({sname}: HEAD={head_n}, "
            f"now={n}). Per completion policy pins are diagnostic-only, never a committed "
            f"end state -- strip the pin and find the C structure that makes GCC choose "
            f"the register naturally."))

    # === G6: any of the above triggers require attempt-log in commit msg ===
    needs_attempt_log = (
        bool(new_auth_with_comments)
        or bool(new_inline) or bool(new_c_body)
        or bool(new_inserts) or bool(new_asm_inj)
        or bool(new_barriers)
        or bool(newly_cheating_via_regfix) or bool(churning_regfix)
        or bool(new_pins)
    )
    if needs_attempt_log:
        attempts = parse_attempt_log(commit_msg or "")
        if len(attempts) < 3:
            violations.append(("attempt_log_missing",
                f"commit introduces inline asm / canonical entry / new regfix / "
                f"aliasing barrier; commit message must contain a `Pure-C attempts:` "
                f"block with ≥3 entries (found {len(attempts)}). Format: \n"
                f"    Pure-C attempts:\n"
                f"      [1] technique=<name> score=<N> outcome=<observable evidence>\n"
                f"      [2] technique=<name> score=<N> outcome=<observable evidence>\n"
                f"      [3] technique=<name> score=<N> outcome=<observable evidence>\n"
                f"Outcomes must reference observable state (dump-text output, "
                f"permuter score plateau, register-pin ignored per RA dump, etc.) "
                f"-- vague outcomes will fail the adversarial LLM auditor."))
        else:
            # Quick programmatic check: outcomes must not be vague.
            VAGUE_PATS = (
                r"^(didn'?t (work|help)|no(t)? improv(e|ement|ed)|"
                r"tried (everything|all)|failed|same|nothing)\.?\s*$"
            )
            vague_re = re.compile(VAGUE_PATS, re.IGNORECASE)
            for idx, technique, score, outcome in attempts:
                if vague_re.match(outcome):
                    violations.append(("attempt_log_vague_outcome",
                        f"Pure-C attempts: entry [{idx}] technique={technique} "
                        f"has vague outcome '{outcome}'. Replace with specific "
                        f"observable state (e.g., 'dump-text shows pin ignored, "
                        f"v0 used instead' or 'permuter plateau at score=14 "
                        f"after 600s, no structural variant')."))

    # === Emit verdict ===
    if not violations:
        return 0

    print("ASM-CHEAT GUARD: new unauthorized patterns since HEAD:", file=sys.stderr)
    print(file=sys.stderr)
    for kind, msg in violations:
        print(f"  [{kind}]", file=sys.stderr)
        for line in msg.split("\n"):
            print(f"    {line}", file=sys.stderr)
        print(file=sys.stderr)
    print("Per completion policy (completion-standard, 2026-05-21): the ONLY end", file=sys.stderr)
    print("states are COMPLETED-C (0 regfix, 0 asmfix, 0 pins, 0 cheat-asm)", file=sys.stderr)
    print("and COMPLETED-INLINE-ASM-CANONICAL (GTE/BIOS/whole-function hand-asm,", file=sys.stderr)
    print("evidence-gated). New regfix / pins / inline-move are BLOCKED outright --", file=sys.stderr)
    print("they are diagnostic-only, never a committed match. Strip them and find the", file=sys.stderr)
    print("C structure, or use canonical-asm authorization for an un-compilable op.", file=sys.stderr)
    return 1


def cmd_summary():
    """One-line counts of unauthorized cheats. For session briefing."""
    auth = load_authorized()
    (cur_splice, cur_inline, cur_bios, cur_c_body, cur_inserts,
     cur_asm_inj) = get_current_cheats(auth)
    n_splice = len(cur_splice)
    n_inline = len(cur_inline)
    n_bios = len(cur_bios)
    n_c_body = len(cur_c_body)
    n_inserts = len(cur_inserts)
    n_asm_inj = len(cur_asm_inj)
    total = n_splice + n_inline + n_c_body + n_inserts + n_asm_inj
    if total == 0 and n_bios == 0:
        print("Cheats:   none — all inline asm and regfix injections are authorized")
    else:
        parts = []
        if n_splice:
            parts.append(f"{n_splice} splice")
        if n_inline:
            parts.append(f"{n_inline} inline-asm")
        if n_c_body:
            parts.append(f"{n_c_body} c-body-multi-insn")
        if n_inserts:
            total_lost = sum(cur_inserts.values())
            parts.append(f"{n_inserts} func(s) w/ lost-codegen inserts ({total_lost} insn(s))")
        if n_asm_inj:
            total_inj = sum(cur_asm_inj.values())
            parts.append(f"{n_asm_inj} func(s) w/ inline-asm injection ({total_inj} insn(s))")
        if n_bios:
            parts.append(f"{n_bios} BIOS-trampoline (recommend adding to authorized list)")
        msg = ", ".join(parts)
        print(f"Cheats:   {total} unauthorized ({msg})")
        print(f"          run 'python3 tools/audit_asm_cheats.py --all' to list, or")
        print(f"          'dc.sh next-cheat' to pull a cheat-fix work item")
    return 0


def cmd_list_funcs():
    """Print one unauthorized function name per line."""
    auth = load_authorized()
    (cur_splice, cur_inline, cur_bios, cur_c_body, cur_inserts,
     cur_asm_inj) = get_current_cheats(auth)
    for f in sorted(cur_splice | cur_inline | cur_c_body
                    | set(cur_inserts.keys()) | set(cur_asm_inj.keys())):
        print(f)
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--all", action="store_true", help="full report of all cheats (default)")
    g.add_argument("--summary", action="store_true", help="one-line cheat counts")
    g.add_argument("--func", help="check one function; non-zero exit if unauthorized")
    g.add_argument("--check-new", action="store_true",
                   help="compare current state vs HEAD; non-zero exit if new cheats added")
    g.add_argument("--list-funcs", action="store_true",
                   help="print unauthorized function names, one per line")
    ap.add_argument("--commit-msg-file",
                    help="path to a file containing the commit message (used by "
                         "--check-new to validate the Pure-C attempts: block)")
    args = ap.parse_args()
    if args.summary:
        return cmd_summary()
    if args.list_funcs:
        return cmd_list_funcs()
    if args.check_new:
        commit_msg = None
        if args.commit_msg_file:
            try:
                commit_msg = Path(args.commit_msg_file).read_text(
                    encoding="utf-8", errors="replace")
            except Exception as e:
                print(f"[audit] WARNING: could not read commit-msg-file "
                      f"{args.commit_msg_file}: {e}", file=sys.stderr)
        return cmd_check_new(commit_msg=commit_msg)
    if args.func:
        return cmd_func(args.func)
    return cmd_all()


if __name__ == "__main__":
    sys.exit(main())
