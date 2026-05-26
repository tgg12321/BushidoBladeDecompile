#!/usr/bin/env python3
"""Unit coverage for headless_audit.digest_transcript on a synthetic transcript.

Run: python3 tools/test_headless_audit.py   (exit 0 = pass)
"""
import json
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import headless_audit as ha


def asst(content, usage=None):
    m = {"role": "assistant", "content": content}
    if usage:
        m["usage"] = usage
    return {"type": "assistant", "message": m, "timestamp": "2026-05-26T00:00:00Z"}


def user(content):
    return {"type": "user", "message": {"role": "user", "content": content},
            "timestamp": "2026-05-26T00:00:01Z"}


def tool_use(name, command=None):
    inp = {"command": command} if command is not None else {}
    return {"type": "tool_use", "name": name, "id": "x", "input": inp}


def tool_result(text):
    return {"type": "tool_result", "tool_use_id": "x", "content": text}


TRANSCRIPT = [
    asst([{"type": "text", "text": "hi"},
          tool_use("PowerShell", "& tools/eng.ps1 sandbox func_X --disable all")],
         usage={"input_tokens": 100, "output_tokens": 200,
                "cache_read_input_tokens": 5000, "cache_creation_input_tokens": 50}),
    user([tool_result("score 0")]),
    # footgun block -> counts as footgun_block AND tooling error ("blocked by")
    asst([tool_use("Bash", "wsl bash -c 'awk ...'")]),
    user([tool_result("BLOCKED by shell_footgun_guard.py - shell-nesting footgun")]),
    # a genuine traceback -> tooling error
    asst([tool_use("Bash", "python3 tmp/x.py")]),
    user([tool_result("Traceback (most recent call last): ZeroDivisionError")]),
    # engine NEGATIVE (rolled back) -> informational, NOT a tooling error
    asst([tool_use("PowerShell", "& tools/eng.ps1 retire func_X")]),
    user([tool_result("ok false; rolled back; sha1 mismatch")]),
    # duplicate command (same Bash twice) -> thrash signal
    asst([tool_use("Bash", "git status")]),
    user([tool_result("clean")]),
    asst([tool_use("Bash", "git status")]),
    user([tool_result("clean")]),
    # benign WSL noise must NOT count as an error
    asst([tool_use("Bash", "wsl bash -c 'make'")]),
    user([tool_result("Failed to start the systemd user session for 'trenton'. ok")]),
]


def main() -> int:
    fails = 0

    def check(name, cond):
        nonlocal fails
        if not cond:
            fails += 1
            print(f"FAIL: {name}")

    with tempfile.NamedTemporaryFile("w", suffix=".jsonl", delete=False) as f:
        for line in TRANSCRIPT:
            f.write(json.dumps(line) + "\n")
        path = Path(f.name)

    d = ha.digest_transcript(path)
    path.unlink(missing_ok=True)

    check("turns counts assistant msgs (7)", d["turns"] == 7)
    check("tool mix PowerShell=2", d["tool_calls"].get("PowerShell") == 2)
    check("tool mix Bash=5", d["tool_calls"].get("Bash") == 5)
    check("footgun_blocks=1", d["footgun_blocks"] == 1)
    # tooling errors: footgun block + traceback = 2 (NOT the rolled-back/mismatch line)
    check("tooling errors=2 (block + traceback)", d["error_results"] == 2)
    check("engine_rollbacks=1 (rolled back/mismatch, informational)", d["engine_rollbacks"] == 1)
    check("benign systemd noise not counted", "failed to start the systemd" not in str(d["error_breakdown"]).lower())
    check("duplicate 'git status' x2", d["duplicate_cmds"].get("git status") == 2)
    check("engine subcmds parsed", d["eng_subcmds"].get("sandbox") == 1 and d["eng_subcmds"].get("retire") == 1)
    check("tokens summed", d["input_tokens"] == 100 and d["output_tokens"] == 200 and d["cache_read_tokens"] == 5000)
    check("error_samples bounded + present", 0 < len(d["error_samples"]) <= 8)

    print(f"headless_audit: {'PASS' if not fails else str(fails)+' FAILURE(S)'}")
    return 1 if fails else 0


if __name__ == "__main__":
    raise SystemExit(main())
