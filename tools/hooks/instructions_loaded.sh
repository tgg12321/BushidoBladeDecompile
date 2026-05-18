#!/bin/bash
# tools/hooks/instructions_loaded.sh
#
# Hook for the InstructionsLoaded event. Logs every rule/memory file
# that Claude Code loads — including path-scoped .claude/rules/*.md
# files triggered by `paths:` glob matches — to a rolling log in tmp/.
#
# Observability only — Claude Code's InstructionsLoaded event has no
# decision control, so this never blocks anything. Always exits 0.
#
# Log format: JSONL, one line per load event:
#   {"ts":"2026-05-18T05:43:17Z","event":{<full payload>}}
#
# Inspect with:
#   tail -50 tmp/instructions_loaded.log | python3 -m json.tool --no-ensure-ascii
#   grep '"load_reason":"path_glob_match"' tmp/instructions_loaded.log
#   grep -o '"file_path":"[^"]*"' tmp/instructions_loaded.log | sort | uniq -c | sort -rn

set -u

REPO_ROOT="${CLAUDE_PROJECT_DIR:-$(cd "$(dirname "$0")/../.." && pwd)}"
LOG_FILE="$REPO_ROOT/tmp/instructions_loaded.log"
mkdir -p "$(dirname "$LOG_FILE")"

# Hook payload arrives on stdin as a JSON object.
PAYLOAD=$(cat)

# Append timestamp-wrapped JSONL line.
TS=$(date -u +%Y-%m-%dT%H:%M:%SZ)
printf '{"ts":"%s","event":%s}\n' "$TS" "$PAYLOAD" >> "$LOG_FILE"

# Rotate if log exceeds ~1MB (keep last 2000 lines).
if [ -f "$LOG_FILE" ] && [ "$(wc -c < "$LOG_FILE")" -gt 1048576 ]; then
    tail -n 2000 "$LOG_FILE" > "$LOG_FILE.tmp" && mv "$LOG_FILE.tmp" "$LOG_FILE"
fi

exit 0
