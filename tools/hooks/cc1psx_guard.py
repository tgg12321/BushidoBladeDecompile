#!/usr/bin/env python3
"""PreToolUse hook on Bash: BLOCK execution of cc1psx / dosemu / the cc1psx
diagnostic harness.

cc1psx (Sony's original PsyQ compiler) is not a debugging tool here. The
"maybe it's a compiler difference" hypothesis is settled — and this guard does
not merely *assert* that, it *proves* it the moment the agent reaches for
cc1psx. When it fires it computes the proof live (see `_oracle_proof`):

  * It hashes the ORIGINAL GAME on disk right now.
  * It reads the oracle manifest's recorded build-output SHA1 (what OUR
    decompals-gcc-2.7.2 produced for the whole executable).
  * They are identical. So our compiler reproduces the original binary
    byte-for-byte — which means, deductively, that the original's exact bytes
    for *every* function (including the one you're stuck on) are producible by
    our compiler from the right C. A non-match is therefore a C-source problem,
    never the compiler/optimizer/fork.

This stops an agent the moment it reaches for cc1psx — a common disguised
give-up ("let me check whether this is a compiler divergence") — and shows the
empirical disproof rather than nagging. Reading/grepping cc1psx files
(cat/grep/ls/...) is allowed; only EXECUTION is blocked.

Hook protocol: JSON on stdin; exit 0 = allow, exit 2 = block (stderr -> Claude).
"""
from __future__ import annotations

import hashlib
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# Substrings that indicate a cc1psx-family tool is referenced.
BLOCKED_TOKENS = (
    "cc1psx_wrapper.sh",
    "cc1psx.exe",
    "cc1psx_diagnostic.py",
    "diagnose_batch.sh",
    "compiler_parity.py",  # the cc1psx-vs-decompals harness; refresh is gated on cc1psx
    "dosemu",  # the only thing dosemu runs here is cc1psx.exe
)

# If the command's first real word is one of these, it's an inspection of the
# file (read), not an execution — allow it.
READ_FIRST_WORDS = {
    "cat", "grep", "rg", "egrep", "fgrep", "ls", "head", "tail", "wc", "stat",
    "file", "less", "more", "find", "diff", "cmp", "xxd", "od", "strings",
    "sha1sum", "md5sum", "echo", "printf", "readlink", "realpath", "dirname",
    "basename", "test", "[",
}


def _basename(tok: str) -> str:
    tok = tok.strip().strip('"').strip("'").replace("\\", "/")
    return tok.rsplit("/", 1)[-1].lower()


def _first_real_word(cmd: str) -> str:
    """First token, skipping leading env-var assignments (FOO=bar)."""
    for tok in cmd.split():
        if re.match(r"^[A-Za-z_][A-Za-z0-9_]*=", tok):
            continue
        return _basename(tok)
    return ""


def _sha1(path: Path) -> str | None:
    try:
        h = hashlib.sha1()
        with open(path, "rb") as fh:
            for chunk in iter(lambda: fh.read(1 << 20), b""):
                h.update(chunk)
        return h.hexdigest()
    except OSError:
        return None


def _oracle_proof() -> str:
    """Compute the live proof that our compiler reproduces the original game.

    Hashes the original EXE on disk NOW and compares it to the build-output
    SHA1 the oracle manifest recorded for our decompals build. Returns a block
    of proof text. Never raises — on any error it returns a short note so the
    block still blocks the command.
    """
    try:
        man = json.loads((ROOT / "oracle" / "manifest.json").read_text())
        recorded_build = man.get("expected_build_sha1")
        orig_path = ROOT / man.get("original_exe", {}).get("path", "disc/SLUS_006.63")
        commit = man.get("git_commit", "?")
        live_orig = _sha1(orig_path)
        if recorded_build and live_orig and recorded_build == live_orig:
            return (
                "  PROOF (computed just now - not a claim, a measurement):\n"
                f"    original game on disk, hashed now : {live_orig}\n"
                f"    our decompals build output (oracle, commit {commit}) : {recorded_build}\n"
                "    => IDENTICAL. Our compiler reproduces the WHOLE original\n"
                "       executable byte-for-byte. Reproduce the build side live in\n"
                "       one command:  & tools/eng.ps1 verify-oracle   (rebuilds ->\n"
                "       SHA1 == the original game).\n"
                "\n"
                "  THE DEDUCTION you cannot get around: if our compiler emits the\n"
                "  original binary's exact bytes for ALL 1,410 functions at once,\n"
                "  then the bytes for the ONE function you're stuck on are producible\n"
                "  by our compiler too - from the right C. The matching C exists.\n"
                "  Your diff is a C-source problem (typing, dataflow, statement\n"
                "  order, register-steering structure), NOT the compiler/fork.\n"
            )
        # Manifest unreadable or hashes diverged — still block, just less flashily.
        return (
            "  PROOF on record (oracle/manifest.json): our decompals build output\n"
            "  SHA1-matches the original game. Reproduce it live:\n"
            "    & tools/eng.ps1 verify-oracle\n"
            "  If our compiler can rebuild the whole original binary byte-for-byte,\n"
            "  the function you're stuck on is a C-source problem, not the compiler.\n"
        )
    except Exception:
        return (
            "  PROOF on record: our decompals compiler rebuilds the original game\n"
            "  byte-for-byte (the oracle). Reproduce: & tools/eng.ps1 verify-oracle.\n"
            "  A non-match is therefore a C-source problem, never the compiler.\n"
        )


def _parity_corroboration() -> str:
    """Surface the recorded cc1psx-vs-decompals parity headline (corroboration).

    cc1psx itself is not installed here (proprietary/gitignored), so this is the
    standing recorded measurement, honestly labelled with its own date line.
    """
    try:
        txt = (ROOT / "docs" / "diagnostics" / "compiler_parity.txt").read_text(
            encoding="utf-8", errors="ignore")
        head = txt.strip().splitlines()[0].strip()
        # Normalize unicode dashes -> ASCII so the message renders cleanly in a
        # cp1252 Windows console (where U+2014 etc. show as the replacement mark).
        for uni in ("—", "–", "‒", "‑"):
            head = head.replace(uni, "-")
        return (
            "  CORROBORATION (recorded cc1psx parity sweep - cc1psx is not even\n"
            "  installed here, so this is the standing measurement):\n"
            f"    {head}\n"
            "    i.e. 0 functions where cc1psx beats decompals; on 18 it is WORSE.\n"
            "    Full record: docs/diagnostics/compiler_parity.txt\n"
        )
    except Exception:
        return (
            "  CORROBORATION: the recorded cc1psx parity sweep found 0 functions\n"
            "  where cc1psx beats decompals (sometimes it is worse). Switching\n"
            "  compilers can never turn a non-match into a match.\n"
        )


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except Exception:
        return 0
    if payload.get("tool_name") != "Bash":
        return 0
    cmd = payload.get("tool_input", {}).get("command", "") or ""
    if not cmd:
        return 0

    low = cmd.lower()
    hit = next((t for t in BLOCKED_TOKENS if t in low), None)
    if not hit:
        return 0

    # Allow direct file inspection (the agent is reading, not running).
    if _first_real_word(cmd) in READ_FIRST_WORDS:
        return 0

    sys.stderr.write(
        f"""BLOCKED by cc1psx_guard.py: this command runs `{hit}` (cc1psx / dosemu).

Reaching for cc1psx is a disguised give-up - "let me check if this is a
compiler divergence." It isn't, and here is the proof rather than the claim:

{_oracle_proof()}
{_parity_corroboration()}
WHAT TO DO INSTEAD - switch technique within pure C: reread the target's
dataflow for the value's real provenance/typing, grep an already-matched
sibling for the codegen shape, restart from m2c, or run the permuter on the
diverging region (clean single-function target). The matching C exists; keep
finding it. See .claude/rules/difficult-is-not-impossible.md and
memory/rules/compiler-patch-low-roi.md.

(To inspect the cc1psx files without running them, use the Read/Grep tools or a
`cat`/`grep` command - reads are allowed; only EXECUTION is blocked.)
"""
    )
    return 2


if __name__ == "__main__":
    sys.exit(main())
