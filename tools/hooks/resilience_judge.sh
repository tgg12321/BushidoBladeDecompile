#!/bin/bash
# tools/hooks/resilience_judge.sh
#
# Semantic stop-gate. Invoked by grind_check.sh (the Stop hook) AFTER its cheap
# gates pass (active marker set, function unmatched, recent decomp activity, no
# operator override). Runs a fresh `claude -p` session on a cheaper model
# (default Sonnet) primed with the resilience-judge prompt, and decides whether
# the worker's attempt to end its turn is a legitimate stop or a disguised
# give-up.
#
# Replaces grind_check's old keyword filter, which capable agents paraphrased
# around. Judges MEANING, not literal phrases.
#
# Inputs (via env, set by grind_check.sh):
#   BB2_JUDGE_ACTIVE        active function name
#   BB2_JUDGE_CONTEXT_FILE  file containing the recent conversation text
#   BB2_JUDGE_TRAJECTORY    one-line iter-log trajectory summary (optional)
#   BB2_JUDGE_CC1PSX_LOG    "yes" | "no" — cc1psx calibration log present?
#   BB2_JUDGE_ROOT          project root (bash-form path)
#   BB2_JUDGE_MODEL         model id (default claude-sonnet-4-6)
#
# Output (stdout): first line is "DECISION=BLOCK" or "DECISION=ALLOW".
#   On BLOCK, the remaining lines are the reason + push to relay to the worker.
# Exit code is always 0 — grind_check reads the DECISION line. FAIL-CLOSED:
# any internal failure prints DECISION=BLOCK (we do not let an agent escape a
# stop-gate by breaking the judge).
#
# Standalone test:
#   BB2_JUDGE_ACTIVE=foo BB2_JUDGE_CONTEXT_FILE=/tmp/ctx.txt \
#   BB2_JUDGE_CC1PSX_LOG=no BB2_JUDGE_ROOT=$(pwd) \
#   bash tools/hooks/resilience_judge.sh
#   BB2_JUDGE_DRY=1 ... bash tools/hooks/resilience_judge.sh   # print prompts only

set -u
export PYTHONIOENCODING=utf-8

ROOT="${BB2_JUDGE_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null)}"
ACTIVE="${BB2_JUDGE_ACTIVE:-(unknown)}"
CONTEXT_FILE="${BB2_JUDGE_CONTEXT_FILE:-}"
TRAJECTORY="${BB2_JUDGE_TRAJECTORY:-(no iter-log trajectory available)}"
CC1PSX_LOG="${BB2_JUDGE_CC1PSX_LOG:-no}"
MODEL="${BB2_JUDGE_MODEL:-claude-sonnet-4-6}"
JUDGE_TIMEOUT_SEC="${BB2_JUDGE_TIMEOUT:-300}"

AGENT_DEF="$ROOT/.claude/agents/resilience-judge.md"
LOG_FILE="$ROOT/.bb2_resilience_log.json"

block_fail_closed() {
    # $1 = reason
    echo "DECISION=BLOCK"
    echo "$1"
    echo
    echo "(resilience judge fail-closed: it could not reach a clean ALLOW, so the stop is blocked. If the user has genuinely authorized stopping, drop a one-shot override: touch .bb2_stop_hook_suppress_once)"
    # best-effort log
    python3 - "$LOG_FILE" "$ACTIVE" "$1" <<'PY' 2>/dev/null || true
import json, sys, datetime
log, active, reason = sys.argv[1], sys.argv[2], sys.argv[3]
try:
    with open(log, "a", encoding="utf-8") as f:
        f.write(json.dumps({
            "ts": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds"),
            "active": active, "decision": "BLOCK", "fail_closed": True, "reason": reason,
        }) + "\n")
except Exception:
    pass
PY
    exit 0
}

[ -z "$ROOT" ] && block_fail_closed "judge: could not resolve project root"
[ -f "$AGENT_DEF" ] || block_fail_closed "judge: agent definition missing at $AGENT_DEF"
if [ -z "$CONTEXT_FILE" ] || [ ! -f "$CONTEXT_FILE" ]; then
    block_fail_closed "judge: no conversation context provided"
fi

TMPDIR_J=$(mktemp -d -t bb2judge.XXXXXX) || block_fail_closed "judge: mktemp failed"
trap 'rm -rf "$TMPDIR_J"' EXIT

# System prompt = agent-def body (strip the YAML frontmatter between the first
# two '---' lines). One source of truth: editing the agent def updates the hook.
SYS_FILE="$TMPDIR_J/sys.txt"
python3 - "$AGENT_DEF" "$SYS_FILE" <<'PY' 2>/dev/null || block_fail_closed "judge: failed to extract system prompt"
import sys, pathlib
src, out = sys.argv[1], sys.argv[2]
text = pathlib.Path(src).read_text(encoding="utf-8")
lines = text.splitlines()
body = text
if lines and lines[0].strip() == "---":
    # find the second '---'
    for i in range(1, len(lines)):
        if lines[i].strip() == "---":
            body = "\n".join(lines[i+1:]).lstrip("\n")
            break
pathlib.Path(out).write_text(body, encoding="utf-8")
PY

# User prompt = the context the judge needs to rule.
USER_FILE="$TMPDIR_J/user.txt"
python3 - "$USER_FILE" "$ACTIVE" "$CC1PSX_LOG" "$TRAJECTORY" "$CONTEXT_FILE" <<'PY' 2>/dev/null || block_fail_closed "judge: failed to build user prompt"
import sys, pathlib
out, active, cc1psx, traj, ctx_path = sys.argv[1:6]
try:
    ctx = pathlib.Path(ctx_path).read_text(encoding="utf-8", errors="replace")
except Exception:
    ctx = "(context unavailable)"
prompt = f"""Rule on whether this worker may end its turn.

=== ACTIVE FUNCTION (currently UNMATCHED) ===
{active}

=== CC1PSX CALIBRATION LOG PRESENT ===
{cc1psx}

=== ATTEMPT TRAJECTORY (iter-log) ===
{traj}

=== RECENT CONVERSATION (most recent last; the worker's would-be-final message is at the end) ===
{ctx}

=== END CONTEXT ===

Emit your JSON verdict now. Default to BLOCK; ALLOW only on explicit user
authorization or genuine catastrophic external state. Scrutinize sophisticated
rationalizations — plausibility is not proof.
"""
pathlib.Path(out).write_text(prompt, encoding="utf-8")
PY

SCHEMA='{"type":"object","properties":{"decision":{"type":"string","enum":["BLOCK","ESCALATE","ALLOW"]},"reasoning":{"type":"string"},"quit_pattern":{"type":"string"},"push":{"type":"string"}},"required":["decision","reasoning"]}'

if [ "${BB2_JUDGE_DRY:-0}" = "1" ]; then
    echo "=== MODEL ==="; echo "$MODEL"
    echo "=== SYSTEM PROMPT ==="; cat "$SYS_FILE"
    echo "=== USER PROMPT ==="; cat "$USER_FILE"
    exit 0
fi

command -v claude >/dev/null 2>&1 || block_fail_closed "judge: \`claude\` CLI not on PATH"

RESP="$TMPDIR_J/resp.json"
( cd "$TMPDIR_J" && timeout "$JUDGE_TIMEOUT_SEC" claude \
        -p \
        --model "$MODEL" \
        --tools "" \
        --setting-sources "" \
        --system-prompt-file "$SYS_FILE" \
        --output-format json \
        --json-schema "$SCHEMA" \
        --max-budget-usd 0.50 \
        --no-session-persistence \
        --dangerously-skip-permissions \
        < "$USER_FILE" \
        > "$RESP" 2>"$TMPDIR_J/claude.stderr" )
rc=$?
[ "$rc" -ne 0 ] && block_fail_closed "judge: \`claude -p\` failed (exit $rc, timeout=${JUDGE_TIMEOUT_SEC}s): $(tail -3 "$TMPDIR_J/claude.stderr" 2>/dev/null)"

# Parse the verdict and emit the DECISION line + body.
OUT=$(python3 - "$RESP" "$LOG_FILE" "$ACTIVE" "$MODEL" <<'PY' 2>/dev/null
import json, sys, datetime
resp, log, active, model = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
def emit_block(reason):
    print("DECISION=BLOCK"); print(reason)
try:
    env = json.load(open(resp, encoding="utf-8"))
except Exception as e:
    emit_block(f"judge: could not parse response envelope ({e})"); sys.exit(0)
if env.get("is_error"):
    emit_block(f"judge: API error: {env.get('result') or env.get('api_error_status')}"); sys.exit(0)
obj = env.get("structured_output")
if not obj:
    raw = env.get("result")
    if isinstance(raw, dict):
        obj = raw
    elif isinstance(raw, str) and raw.strip():
        s = raw.strip()
        if s.startswith("```"):
            s = s.split("\n", 1)[-1]
            if s.endswith("```"): s = s[:-3]
        try: obj = json.loads(s)
        except Exception: obj = None
if not isinstance(obj, dict) or "decision" not in obj:
    emit_block("judge: response had no decision object"); sys.exit(0)
decision = (obj.get("decision") or "").upper()
reasoning = obj.get("reasoning", "")
quit_pattern = obj.get("quit_pattern", "")
push = obj.get("push", "")
cost = env.get("total_cost_usd") or 0.0
try:
    with open(log, "a", encoding="utf-8") as f:
        f.write(json.dumps({
            "ts": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds"),
            "active": active, "model": model, "decision": decision,
            "quit_pattern": quit_pattern, "reasoning": reasoning, "push": push,
            "cost_usd": cost,
        }) + "\n")
except Exception:
    pass
if decision == "ALLOW":
    print("DECISION=ALLOW"); print(reasoning)
elif decision == "ESCALATE":
    # Genuine terminus: let the worker surface to the owner. The hook allows
    # the stop but flags it as an escalation (logged distinctly above).
    print("DECISION=ESCALATE")
    body = []
    if reasoning: body.append(reasoning)
    if push: body.append(f"\nEscalation note: {push}")
    print("\n".join(body) if body else "Genuine terminus reached; surfacing to owner.")
else:
    # BLOCK (and anything unexpected → fail closed to BLOCK)
    print("DECISION=BLOCK")
    body = []
    if quit_pattern: body.append(f"Detected quit pattern: {quit_pattern}")
    if reasoning: body.append(reasoning)
    if push: body.append(f"\nDo this next: {push}")
    print("\n".join(body) if body else "Stop blocked: the active function is unmatched and avenues remain.")
PY
)
if [ -z "$OUT" ]; then
    block_fail_closed "judge: empty parse output"
fi
echo "$OUT"
exit 0
