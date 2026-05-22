#!/bin/bash
# End-to-end test of the tooling-incident loop in Git Bash (matches the real
# hook runner): detector -> marker -> stop-gate block -> resolver -> allow.
#
# Self-cleaning AND non-polluting: it snapshots the real incident marker +
# ledger before running and restores them after, so running it during real
# work never corrupts incident history.
#
#   bash tools/hooks/test_tooling_incident_e2e.sh
#
# Exit 0 = all pass. Companion to the unit suite (test_tooling_error_guard.py).
set -u
ROOT=$(git rev-parse --show-toplevel)
cd "$ROOT" || exit 1
MARKER="$ROOT/.bb2_tooling_incident.json"
LEDGER="$ROOT/docs/tooling_incidents.md"

# --- snapshot real state so the test never pollutes it ---
SNAP_DIR=$(mktemp -d)
MARKER_PRE=0; [ -f "$MARKER" ] && { cp "$MARKER" "$SNAP_DIR/marker"; MARKER_PRE=1; }
LEDGER_PRE=0; [ -f "$LEDGER" ] && { cp "$LEDGER" "$SNAP_DIR/ledger"; LEDGER_PRE=1; }
restore() {
    if [ "$MARKER_PRE" = 1 ]; then cp "$SNAP_DIR/marker" "$MARKER"; else rm -f "$MARKER"; fi
    if [ "$LEDGER_PRE" = 1 ]; then cp "$SNAP_DIR/ledger" "$LEDGER"; else rm -f "$LEDGER"; fi
    rm -rf "$SNAP_DIR" tmp/_crlf_payload.json
}
trap restore EXIT

P=0; F=0
chk(){ if [ "$2" = "1" ]; then P=$((P+1)); else F=$((F+1)); echo "  FAIL: $1"; fi; }

mkdir -p tmp
python3 - <<'PY' > tmp/_crlf_payload.json
import json
stderr = "bash: line 1: " + chr(36)+chr(39)+chr(92)+"r"+chr(39) + ": command not found"
print(json.dumps({"tool_name":"Bash","tool_input":{"command":"bash tools/foo.sh"},
                  "tool_response":{"stderr":stderr}}))
PY

rm -f "$MARKER"

# 1. detector raises an incident on CRLF
python3 tools/hooks/tooling_error_guard.py < tmp/_crlf_payload.json >/dev/null 2>&1; DRC=$?
[ $DRC -eq 2 ] && r=1 || r=0; chk "detector exits 2 on CRLF (got $DRC)" $r
[ -s "$MARKER" ] && r=1 || r=0; chk "marker created" $r

# 2. stop-gate BLOCKS while unresolved
SG=$(echo '{"stop_hook_active":false}' | bash tools/hooks/tooling_incident_stop_guard.sh 2>/dev/null)
echo "$SG" | grep -q '"decision": "block"' && r=1 || r=0; chk "stop-gate emits block" $r
echo "$SG" | grep -q 'fix-tooling-incident' && r=1 || r=0; chk "block reason names resolver" $r
echo "$SG" | grep -q $'\r' && r=1 || r=0; chk "block reason has NO raw CR" $((1-r))

# 3. resolver --false-positive clears it
python3 tools/resolve_tooling_incident.py --false-positive "e2e misfire" >/dev/null 2>&1
[ ! -f "$MARKER" ] && r=1 || r=0; chk "false-positive clears marker" $r

# 4. stop-gate now ALLOWS (empty output)
SG=$(echo '{"stop_hook_active":false}' | bash tools/hooks/tooling_incident_stop_guard.sh 2>/dev/null)
[ -z "$SG" ] && r=1 || r=0; chk "stop-gate allows after resolve" $r

# 5. --fixed accepts a guard with a recent change (untracked new file = dirty)
python3 tools/hooks/tooling_error_guard.py < tmp/_crlf_payload.json >/dev/null 2>&1
python3 tools/resolve_tooling_incident.py --fixed --guard tools/hooks/tooling_error_guard.py \
        --root-cause "e2e" --verify "e2e suite" >/dev/null 2>&1; RC=$?
[ $RC -eq 0 ] && r=1 || r=0; chk "--fixed accepts dirty guard (got $RC)" $r
[ ! -f "$MARKER" ] && r=1 || r=0; chk "--fixed clears marker" $r

# 6. --fixed REJECTS a nonexistent guard, marker retained
python3 tools/hooks/tooling_error_guard.py < tmp/_crlf_payload.json >/dev/null 2>&1
python3 tools/resolve_tooling_incident.py --fixed --guard does/not/exist.xyz --root-cause x >/dev/null 2>&1; RC=$?
[ $RC -eq 2 ] && r=1 || r=0; chk "--fixed rejects missing guard (got $RC)" $r
[ -s "$MARKER" ] && r=1 || r=0; chk "marker retained on reject" $r

# 7. --fixed REJECTS an unchanged tracked file (AGENTS.md is clean)
python3 tools/resolve_tooling_incident.py --fixed --guard AGENTS.md --root-cause x --verify y >/dev/null 2>&1; RC=$?
[ $RC -eq 2 ] && r=1 || r=0; chk "--fixed rejects unchanged tracked file (got $RC)" $r

# 8. --defer clears with reason
python3 tools/resolve_tooling_incident.py --defer "e2e one-off" >/dev/null 2>&1; RC=$?
[ $RC -eq 0 ] && r=1 || r=0; chk "--defer exits 0 (got $RC)" $r
[ ! -f "$MARKER" ] && r=1 || r=0; chk "--defer clears marker" $r

# 9. off-switch suppresses detection
touch "$ROOT/.bb2_tooling_guard_off"
python3 tools/hooks/tooling_error_guard.py < tmp/_crlf_payload.json >/dev/null 2>&1; RC=$?
[ $RC -eq 0 ] && r=1 || r=0; chk "off-switch -> detector exits 0 (got $RC)" $r
[ ! -f "$MARKER" ] && r=1 || r=0; chk "off-switch -> no marker" $r
rm -f "$ROOT/.bb2_tooling_guard_off"

echo ""
echo "$P passed, $F failed"
[ $F -eq 0 ]
