#!/bin/bash
# Stop-event hook. Fires when the agent finishes its turn.
#
# Purpose: catch voluntary-stop attempts on an active, unmatched function.
# Per the user's standing directive, the agent may NOT decide to end work on
# an active function. Stopping is allowed only when the function MATCHES, the
# user explicitly authorized stopping, or the machine is physically broken.
#
# ARCHITECTURE (rewritten 2026-05-19): this hook does the cheap, deterministic
# gating, then delegates the actual stop/continue judgment to a SEMANTIC LLM
# JUDGE (tools/hooks/resilience_judge.sh, default Sonnet). The previous version
# grepped the agent's text for a fixed list of "quit phrases" — but a capable
# agent paraphrases around any literal blocklist (and publishing the list, as
# the skill did, actively trained the evasion). The judge rules on MEANING:
# is this a disguised give-up ("toolchain limitation", "your call", "here are
# the options", naming-an-untried-avenue-then-stopping) or a legitimate stop?
#
# Gate order (cheap -> expensive; bail early to avoid LLM cost):
#   1. stop_hook_active   -> exit (don't recurse)
#   2. transcript present -> else exit
#   3. project root + active marker resolved -> else exit
#   4. operator override files -> exit
#   5. active marker non-empty -> else allow stop
#   6. dc.sh verify MATCH -> allow stop (function done)
#   7. recent decomp activity (last 30 tool calls) -> else allow stop
#      (agent isn't decomping; marker is incidental)
#   8. SEMANTIC JUDGE -> BLOCK or ALLOW
#
# Hook protocol: read JSON on stdin {session_id, transcript_path,
# stop_hook_active}. Emit {decision:"block", reason:"..."} on stdout to keep
# the agent going; emit nothing + exit 0 to allow the stop.
#
# We deliberately do NOT `set -e`: gating fails OPEN (if we can't determine
# there's active unmatched work, allowing the stop is the only sane default),
# but once active-unmatched-decomp is established, the JUDGE fails CLOSED
# (errors -> BLOCK), so an agent can't escape by breaking the judge.

INPUT=$(cat)

HOOK_ACTIVE=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print('true' if d.get('stop_hook_active') else 'false')" 2>/dev/null || echo "false")
[ "$HOOK_ACTIVE" = "true" ] && exit 0

TRANSCRIPT=$(echo "$INPUT" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('transcript_path',''))" 2>/dev/null || echo "")
{ [ -z "$TRANSCRIPT" ] || [ ! -f "$TRANSCRIPT" ]; } && exit 0

# Project root STRICTLY from cwd's git toplevel (per-worktree marker).
PROJECT_ROOT=$(git rev-parse --show-toplevel 2>/dev/null)
{ [ -z "$PROJECT_ROOT" ] || [ ! -d "$PROJECT_ROOT" ]; } && exit 0
case "$PROJECT_ROOT" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_ROOT#?:}"; REST="${REST//\\//}"
        PROJECT_ROOT="/$DRIVE$REST"
        ;;
esac

ACTIVE_FILE="$PROJECT_ROOT/.bb2_active_func"

# Operator overrides.
[ -f "$PROJECT_ROOT/.bb2_stop_hook_off" ] && exit 0
if [ -f "$PROJECT_ROOT/.bb2_stop_hook_suppress_once" ]; then
    rm -f "$PROJECT_ROOT/.bb2_stop_hook_suppress_once"
    exit 0
fi

# No active marker -> allow stop.
[ ! -s "$ACTIVE_FILE" ] && exit 0
ACTIVE=$(tr -d '[:space:]' < "$ACTIVE_FILE")
[ -z "$ACTIVE" ] && exit 0

# Allow stop if the function actually matches (marker is stale/about-to-clear).
case "$PROJECT_ROOT" in
    /[a-zA-Z]/*) WSL_ROOT="/mnt$PROJECT_ROOT" ;;
    *)           WSL_ROOT="$PROJECT_ROOT" ;;
esac
if command -v wsl >/dev/null 2>&1; then
    VERIFY=$(wsl bash -c "cd '$WSL_ROOT' && bash tools/dc.sh verify $ACTIVE 2>&1" 2>/dev/null | head -1)
    case "$VERIFY" in
        *": MATCH "*) exit 0 ;;
    esac
else
    VERIFY="(verify unavailable — wsl not on PATH)"
fi

# Extract: (a) recent decomp-activity count (last 30 tool calls), and
# (b) the recent conversation (last messages, both roles) to a context file
# for the judge. One python pass.
CTX_FILE=$(mktemp -t bb2grind.XXXXXX 2>/dev/null || echo "$PROJECT_ROOT/.bb2_grind_ctx.tmp")
DECOMP_COUNT=$(python3 - "$TRANSCRIPT" "$CTX_FILE" <<'PY' 2>/dev/null || echo 0
import json, re, sys
path, ctx_out = sys.argv[1], sys.argv[2]
events = []
try:
    with open(path, encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                ev = json.loads(line)
            except Exception:
                continue
            if ev.get('type') in ('user', 'assistant'):
                events.append(ev)
except Exception:
    pass

DECOMP_BASH = ('dc.sh attempt','dc.sh verify','dc.sh diff','dc.sh permute','dc.sh smart',
    'dc.sh recipes','dc.sh apply-recipe','dc.sh gen-regfix','dc.sh add-regfix','dc.sh build-active',
    'dc.sh inline-','dc.sh score','dc.sh fix-','dc.sh asmfix-slice','dc.sh dump-text','dc.sh classify',
    'dc.sh suggest','dc.sh next','bb2_permuter','cc1psx','m2c','kengo_ref')
MAKE_RE = re.compile(r'(^|[;&|`(\s])make(\s|$)')
SUFFIX = ('regfix.txt','asmfix.txt','regfix_stage2.txt')

tool_uses, convo = [], []
for ev in events:
    role = ev.get('type')
    msg = ev.get('message') or {}
    content = msg.get('content')
    texts = []
    if isinstance(content, str):
        texts.append(content)
    elif isinstance(content, list):
        for b in content:
            if not isinstance(b, dict):
                continue
            if b.get('type') == 'text':
                texts.append(b.get('text', ''))
            elif b.get('type') == 'tool_use':
                tool_uses.append(b)
    txt = '\n'.join(t for t in texts if t).strip()
    if txt:
        convo.append((role, txt))

decomp = 0
for tu in tool_uses[-30:]:
    name = tu.get('name', ''); inp = tu.get('input') or {}
    if name in ('Edit','Write','MultiEdit','NotebookEdit'):
        fp = (inp.get('file_path','') or '').replace('\\','/').lower()
        if ('/src/' in fp and fp.endswith('.c')) or fp.endswith(SUFFIX):
            decomp += 1
    elif name == 'Bash':
        cmd = inp.get('command','')
        if any(s in cmd for s in DECOMP_BASH) or MAKE_RE.search(cmd):
            decomp += 1

try:
    with open(ctx_out, 'w', encoding='utf-8') as f:
        for role, txt in convo[-8:]:
            if len(txt) > 4000:
                txt = txt[:2000] + "\n...[truncated]...\n" + txt[-2000:]
            f.write(f"\n----- {role.upper()} -----\n{txt}\n")
except Exception:
    pass

print(decomp)
PY
)

# Agent isn't actively decomping -> marker incidental -> allow stop.
if [ -z "$DECOMP_COUNT" ] || [ "$DECOMP_COUNT" = "0" ]; then
    rm -f "$CTX_FILE" 2>/dev/null
    exit 0
fi

# Gather judge context: iter-log trajectory + cc1psx calibration log presence.
TRAJ="(no iter-log)"
ITER_LOG="$PROJECT_ROOT/.bb2_iter_log/$ACTIVE.jsonl"
if [ -f "$ITER_LOG" ]; then
    TRAJ=$(python3 - "$ITER_LOG" <<'PY' 2>/dev/null || echo "(iter-log parse failed)"
import json, sys
rows = []
for line in open(sys.argv[1], encoding='utf-8'):
    line = line.strip()
    if not line: continue
    try: rows.append(json.loads(line))
    except Exception: pass
n = len(rows)
diffs = [r.get('diffs', r.get('structural', '?')) for r in rows[-12:]]
print(f"{n} build round(s) recorded; recent diff counts: {diffs}")
PY
)
fi

CC1PSX_LOG="no"
[ -f "$PROJECT_ROOT/tmp/cc1psx_calibration_$ACTIVE.md" ] && CC1PSX_LOG="yes"
[ -f "$PROJECT_ROOT/.bb2_calibration/$ACTIVE.md" ] && CC1PSX_LOG="yes"

# Consult the semantic judge.
JUDGE_OUT=$(BB2_JUDGE_ACTIVE="$ACTIVE" \
    BB2_JUDGE_CONTEXT_FILE="$CTX_FILE" \
    BB2_JUDGE_TRAJECTORY="$TRAJ" \
    BB2_JUDGE_CC1PSX_LOG="$CC1PSX_LOG" \
    BB2_JUDGE_ROOT="$PROJECT_ROOT" \
    bash "$PROJECT_ROOT/tools/hooks/resilience_judge.sh" 2>/dev/null)
rm -f "$CTX_FILE" 2>/dev/null

DECISION=$(echo "$JUDGE_OUT" | head -1)
JUDGE_BODY=$(echo "$JUDGE_OUT" | tail -n +2)

case "$DECISION" in
    DECISION=ALLOW)
        exit 0
        ;;
    DECISION=ESCALATE)
        # Genuine terminus per the judge (rare). Allow the stop so the worker
        # can surface its structured report to the owner. The judge already
        # logged the ESCALATE verdict to .bb2_resilience_log.json.
        exit 0
        ;;
    *)
        # DECISION=BLOCK or anything unexpected -> fail closed to BLOCK.
        REASON="STOP BLOCKED by the resilience judge: $ACTIVE is active and UNMATCHED.

Build: dc.sh verify $ACTIVE -> $VERIFY

$JUDGE_BODY

The only legitimate ways to end work on an active function are: it MATCHES,
the user explicitly told you to stop, or the machine is physically broken.
'Difficulty', 'plateau', 'toolchain limit', and offering the user options are
NOT among them. Switch technique, not target. Tool calls only — take the next
action above; do not summarize and wait.

(If the user genuinely authorized this stop and the judge misread it, the
operator can drop a one-shot override: touch .bb2_stop_hook_suppress_once)"
        REASON="$REASON" python3 -c "
import json, os
print(json.dumps({'decision':'block','reason':os.environ['REASON']}))
" 2>/dev/null
        exit 0
        ;;
esac
