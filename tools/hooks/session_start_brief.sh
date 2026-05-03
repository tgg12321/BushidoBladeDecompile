#!/bin/bash
# SessionStart hook. Runs `dc.sh start` and emits the briefing as
# additional context for the agent. Output goes via stdout in JSON
# format per the Claude Code hook protocol; we use additionalContext
# to prepend the briefing to the session.

set -e

PROJECT_ROOT="/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"

# Collect the briefing. Run dc.sh start via WSL because dc.sh expects
# the WSL path layout. Strip the systemd warning line that wsl emits.
BRIEFING=$(wsl bash -c "cd /mnt/c/Users/Trenton/Desktop/'Bushido Blade 2 Decompile' && bash tools/dc.sh start 2>&1" \
    | grep -v "Failed to start.*systemd" || true)

# Emit JSON. Claude Code reads `additionalContext` and injects it as
# system context for the session — visible to the agent on first turn.
python3 -c "
import json, sys
briefing = '''$BRIEFING'''
print(json.dumps({
    'hookSpecificOutput': {
        'hookEventName': 'SessionStart',
        'additionalContext': briefing
    }
}))
"
