#!/usr/bin/env python3
"""Per-function asm text post-pass for label/symbol/linkage fixes.

Supports `{lbl#N}` function-local label slot references (added 2026-06-08)
in replacement text of `rename`, `replace_first`, `replace_all`,
`insert_before`, `insert_after` rules. At rule-apply time, `{lbl#N}` is
substituted with the Nth `.L<digit>+` label CC1 emitted within the target
function's body (1-indexed, document order). This mirrors the analogous
mechanism in tools/regfix.py and makes asmfix rules drift-robust against
TU-wide `.L<N>` renumbering. Substitution applies only to REPLACEMENT
text — source-side patterns (the first quoted argument of any rule, plus
both arguments of `delete_between`) are left alone. `replace_with_asmfile`
reads its body verbatim from a file and is therefore unaffected.
"""

from __future__ import annotations

import os
import re
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
# ASMFIX_CONFIG env override mirrors regfix.py's REGFIX_CONFIG, so the engine's
# cheat-invisible sandbox can point asmfix at a filtered config without mutating
# the canonical asmfix.txt. Unset -> the canonical file (identical behavior).
CONFIG_PATH = Path(os.environ.get("ASMFIX_CONFIG", REPO_ROOT / "asmfix.txt"))


# ---------------------------------------------------------------------------
# Function-local label slot references — {lbl#N} substitution
# ---------------------------------------------------------------------------
# Same shape as tools/regfix.py — see that module's header for rationale.
# Within a function block (funcname: ... .end\tfuncname), gather every
# `.L<digit>+` label DEFINITION in document order; substitute `{lbl#N}` in
# replacement strings with `labels[N-1]`. Out-of-range references emit a
# warning + an `.L_UNRESOLVED_lbl_<N>` marker that fails at assembly time
# (preferring build-fail to silent-wrong-codegen).
_LABEL_DEF_RE = re.compile(r'^\s*(\.L\d+):\s*$', re.MULTILINE)
_LBL_SLOT_REF_RE = re.compile(r'\{lbl#(\d+)\}')


def _extract_local_label_defs(text: str) -> list[str]:
    """Return list of `.L<N>` labels DEFINED in this function-block text, in
    document order (1-indexed when used as slot refs)."""
    return [m.group(1) for m in _LABEL_DEF_RE.finditer(text)]


def _substitute_label_slots(text: str, labels: list[str], fname: str | None = None,
                            context: str | None = None) -> str:
    """Replace `{lbl#N}` placeholders in `text` with `labels[N-1]`. Returns
    `text` unchanged if no placeholders present. Out-of-range slot references
    emit a warning + an unresolved-marker (`.L_UNRESOLVED_lbl_<N>`) that the
    assembler will fail on — silent-wrong-codegen is worse than build-fail."""
    if '{lbl#' not in text:
        return text

    def repl(m: re.Match[str]) -> str:
        n = int(m.group(1))
        if 1 <= n <= len(labels):
            return labels[n - 1]
        prefix = f"asmfix: WARNING: {fname}" if fname else "asmfix: WARNING"
        if context:
            prefix += f" ({context})"
        sample = labels[:5] + ['...'] if len(labels) > 5 else labels
        print(
            f"{prefix}: {{lbl#{n}}} references label slot #{n}, "
            f"but function defines only {len(labels)} labels (first few: "
            f"{sample}). Emitting unresolved-marker; assembler will fail.",
            file=sys.stderr,
        )
        return f".L_UNRESOLVED_lbl_{n}"
    return _LBL_SLOT_REF_RE.sub(repl, text)


def unescape_config_text(text: str) -> str:
    return text.replace("\\\\", "\\")


def parse_config(path: Path) -> dict[str, list[tuple[str, ...]]]:
    config: dict[str, list[tuple[str, ...]]] = {}
    if not path.exists():
        return config

    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue

        m = re.match(r'(\w+)\s*:\s*rename\s+"([^"]+)"\s+"([^"]+)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("rename", m.group(2), m.group(3)))
            continue

        m = re.match(r'(\w+)\s*:\s*replace_first\s+"([^"]+)"\s+"([^"]*)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("replace_first", unescape_config_text(m.group(2)), unescape_config_text(m.group(3))))
            continue

        m = re.match(r'(\w+)\s*:\s*replace_all\s+"([^"]+)"\s+"([^"]*)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("replace_all", unescape_config_text(m.group(2)), unescape_config_text(m.group(3))))
            continue

        m = re.match(r'(\w+)\s*:\s*insert_before\s+"([^"]+)"\s+"([^"]*)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("insert_before", unescape_config_text(m.group(2)), unescape_config_text(m.group(3)).replace("\\n", "\n")))
            continue

        m = re.match(r'(\w+)\s*:\s*insert_after\s+"([^"]+)"\s+"([^"]*)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("insert_after", unescape_config_text(m.group(2)), unescape_config_text(m.group(3)).replace("\\n", "\n")))
            continue

        m = re.match(r'(\w+)\s*:\s*delete_between\s+"([^"]+)"\s+"([^"]+)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("delete_between", unescape_config_text(m.group(2)), unescape_config_text(m.group(3))))
            continue

        m = re.match(r'(\w+)\s*:\s*replace_with_asmfile\s+"([^"]+)"$', line)
        if m:
            config.setdefault(m.group(1), []).append(("replace_with_asmfile", m.group(2)))
            continue

        print(f"asmfix: WARNING: ignoring malformed line: {raw_line}", file=sys.stderr)

    return config


def apply_ops(func_name: str, text: str, ops: list[tuple[str, ...]]) -> str:
    # Function-local label sequence — gathered ONCE from the cc1+regfix output
    # block passed in (lines from `funcname:` through `.end\tfuncname`). Used
    # for `{lbl#N}` substitution in replacement text of all rule kinds that
    # carry replacement strings. The labels visible here are exactly the same
    # `.L<digit>+` definitions cc1 emitted within this function's body, which
    # is the property that makes the slot reference drift-robust. See module
    # header + tools/regfix.py for full rationale.
    local_labels = _extract_local_label_defs(text)

    def _sub(s: str, ctx: str) -> str:
        return _substitute_label_slots(s, local_labels, fname=func_name, context=ctx)

    for op in ops:
        kind = op[0]
        if kind == "replace_with_asmfile":
            asm_path = (REPO_ROOT / op[1]).resolve()
            try:
                return asm_path.read_text(encoding="utf-8")
            except OSError as exc:
                print(f"asmfix: WARNING: could not read asm file for {func_name}: {asm_path} ({exc})", file=sys.stderr)
                continue

        if kind == "rename":
            # Use word-boundary regex so `.L800` doesn't match inside
            # `.L8006BE00`. Without \b on both sides, source label being
            # a prefix substring of a target rename's output would corrupt
            # the cascade (this hit when stubs shifted file-wide GCC labels
            # into the 8xx range that overlaps target's `.L8006xxxx`
            # absolute-label prefixes).
            pattern = re.escape(op[1]) + r"(?!\w)"
            text = re.sub(pattern, _sub(op[2], "rename dst"), text)
            continue

        if kind == "replace_first":
            text, count = re.subn(op[1], _sub(op[2], "replace_first repl"), text, count=1, flags=re.MULTILINE)
            if count == 0:
                print(f'asmfix: WARNING: replace_first did not match in {func_name}: {op[1]}', file=sys.stderr)
            continue

        if kind == "replace_all":
            text, count = re.subn(op[1], _sub(op[2], "replace_all repl"), text, flags=re.MULTILINE)
            if count == 0:
                print(f'asmfix: WARNING: replace_all did not match in {func_name}: {op[1]}', file=sys.stderr)
            continue

        if kind == "insert_before":
            pattern, insert_text = op[1], _sub(op[2], "insert_before text")
            m = re.search(pattern, text, flags=re.MULTILINE)
            if not m:
                print(f'asmfix: WARNING: insert_before did not match in {func_name}: {pattern}', file=sys.stderr)
                continue
            text = text[:m.start()] + insert_text + ("\n" if insert_text and not insert_text.endswith("\n") else "") + text[m.start():]
            continue

        if kind == "insert_after":
            pattern, insert_text = op[1], _sub(op[2], "insert_after text")
            m = re.search(pattern, text, flags=re.MULTILINE)
            if not m:
                print(f'asmfix: WARNING: insert_after did not match in {func_name}: {pattern}', file=sys.stderr)
                continue
            end = m.end()
            text = text[:end] + ("\n" if not text[:end].endswith("\n") else "") + insert_text + ("\n" if insert_text and not insert_text.endswith("\n") else "") + text[end:]
            continue

        if kind == "delete_between":
            # Both patterns are source-side; do NOT substitute {lbl#N} here.
            start_pat, end_pat = op[1], op[2]
            start = re.search(start_pat, text, flags=re.MULTILINE)
            if not start:
                print(f'asmfix: WARNING: delete_between start did not match in {func_name}: {start_pat}', file=sys.stderr)
                continue
            end = re.search(end_pat, text[start.end():], flags=re.MULTILINE)
            if not end:
                print(f'asmfix: WARNING: delete_between end did not match in {func_name}: {end_pat}', file=sys.stderr)
                continue
            end_start = start.end() + end.start()
            text = text[:start.start()] + text[end_start:]
            continue

    return text


def process_asm_text(asm_text: str, config: dict[str, list[tuple[str, ...]]]) -> str:
    if not config:
        return asm_text

    lines = asm_text.splitlines(keepends=True)
    out: list[str] = []
    idx = 0
    while idx < len(lines):
        line = lines[idx]
        stripped = line.strip()
        if stripped.endswith(":"):
            func_name = stripped[:-1]
            if func_name in config:
                block = [line]
                idx += 1
                while idx < len(lines):
                    block.append(lines[idx])
                    if lines[idx].strip() == f".end\t{func_name}":
                        idx += 1
                        break
                    idx += 1
                out.append(apply_ops(func_name, "".join(block), config[func_name]))
                continue
        out.append(line)
        idx += 1
    return "".join(out)


def main() -> int:
    config = parse_config(CONFIG_PATH)
    asm_text = sys.stdin.read()
    sys.stdout.write(process_asm_text(asm_text, config))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
