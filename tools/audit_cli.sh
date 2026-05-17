#!/bin/bash
# tools/audit_cli.sh
#
# Ad-hoc invocation of the LLM cheat auditor (tools/hooks/llm_audit.sh)
# on arbitrary inputs, separate from the git-commit hook path. Used by
# the `cheat-audit` skill and available for direct use.
#
# Each subcommand assembles a synthetic-or-real diff into a temp file,
# then invokes llm_audit.sh in advisory mode (BB2_AUDIT_ADVISORY=1):
# verdict prints to stdout, exit is always 0 (never crashes the caller).
# The same allowlist + pre-filter logic applies — pointing this at a
# docs-only commit costs $0.
#
# Usage:
#   bash tools/audit_cli.sh commit <SHA-or-ref>
#       Audit one commit's diff (e.g., HEAD, HEAD~3, abc1234).
#
#   bash tools/audit_cli.sh range <REF1>..<REF2>
#       Audit the diff between two refs (e.g., main..HEAD, HEAD~5..HEAD).
#
#   bash tools/audit_cli.sh func <funcname>
#       Audit the CURRENT STATE of a function: locates its body in
#       src/*.c, bundles with related asmfix.txt / regfix.txt rules,
#       audits as a snapshot.
#
#   bash tools/audit_cli.sh file <path>
#       Audit the current state of a single file (relative to repo root).
#
#   bash tools/audit_cli.sh staged
#       Audit the currently-staged diff (same input the commit hook sees).
#
#   bash tools/audit_cli.sh working
#       Audit the working tree vs HEAD (unstaged + staged together).
#
#   bash tools/audit_cli.sh diff <path>
#       Audit a diff from a file on disk.

set -u

MODE="${1:-}"
ARG="${2:-}"

usage() {
    cat >&2 <<EOF
usage:
  bash tools/audit_cli.sh commit <SHA-or-ref>
  bash tools/audit_cli.sh range <REF1>..<REF2>
  bash tools/audit_cli.sh func <funcname>
  bash tools/audit_cli.sh file <path>
  bash tools/audit_cli.sh staged
  bash tools/audit_cli.sh working
  bash tools/audit_cli.sh diff <path>

The auditor runs in advisory mode: verdict + reasoning + violations
print to stdout, exit is always 0. Cost: ~\$0.20-0.30 per audit (Opus
4.7), \$0 when the allowlist/pre-filter skips.
EOF
    exit 2
}

[ -z "$MODE" ] && usage

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
PROJECT_ROOT=$(cd "$SCRIPT_DIR/.." && pwd)

# Normalize Windows-style path so git operations work regardless of how
# the user invoked this from Git Bash.
case "$PROJECT_ROOT" in
    [A-Za-z]:[/\\]*)
        DRIVE=$(echo "$PROJECT_ROOT" | cut -c1 | tr '[:upper:]' '[:lower:]')
        REST="${PROJECT_ROOT#?:}"
        REST="${REST//\\//}"
        PROJECT_ROOT="/$DRIVE$REST"
        ;;
esac

TMPDIR_AUDIT=$(mktemp -d -t bb2auditcli.XXXXXX)
trap "rm -rf $TMPDIR_AUDIT" EXIT
DIFF_FILE="$TMPDIR_AUDIT/input.diff"
LABEL=""
COMMIT_MSG=""

# Helper: emit a synthetic "all-added" diff for a single file at its
# current state. The auditor will see the entire file as new content
# and flag any inline_asm / register pins / etc. it contains. This is
# how we audit a snapshot rather than a real change.
synth_diff_for_file() {
    local rel="$1" abs="$PROJECT_ROOT/$1"
    if [ ! -f "$abs" ]; then
        echo "error: file not found: $abs" >&2
        exit 2
    fi
    {
        printf 'diff --git a/%s b/%s\n' "$rel" "$rel"
        printf -- '--- /dev/null\n'
        printf -- '+++ b/%s\n' "$rel"
        printf -- '@@ -0,0 +1,%d @@\n' "$(wc -l < "$abs")"
        sed 's/^/+/' "$abs"
    } > "$DIFF_FILE"
}

case "$MODE" in
    commit)
        [ -z "$ARG" ] && usage
        if ! git -C "$PROJECT_ROOT" rev-parse --verify "$ARG^{commit}" >/dev/null 2>&1; then
            echo "error: not a valid commit ref: $ARG" >&2
            exit 2
        fi
        git -C "$PROJECT_ROOT" diff "$ARG^!" > "$DIFF_FILE"
        COMMIT_MSG=$(git -C "$PROJECT_ROOT" log -1 --pretty=%B "$ARG")
        LABEL="commit $(git -C "$PROJECT_ROOT" rev-parse --short "$ARG") — $(git -C "$PROJECT_ROOT" log -1 --pretty=%s "$ARG")"
        ;;

    range)
        [ -z "$ARG" ] && usage
        case "$ARG" in
            *..*) : ;;  # ok
            *) echo "error: range must be REF1..REF2 (got: $ARG)" >&2; exit 2 ;;
        esac
        git -C "$PROJECT_ROOT" diff "$ARG" > "$DIFF_FILE" 2>/dev/null || {
            echo "error: could not diff range: $ARG" >&2
            exit 2
        }
        LABEL="range $ARG"
        ;;

    func)
        [ -z "$ARG" ] && usage
        # Find the .c file containing the function (either a C definition
        # or an INCLUDE_ASM stub). Search src/ recursively.
        cfile=""
        for f in $(find "$PROJECT_ROOT/src" -name '*.c' -type f 2>/dev/null); do
            if grep -qE "(\b${ARG}\s*\(|INCLUDE_ASM\([^)]*\b${ARG}\b)" "$f" 2>/dev/null; then
                cfile="$f"
                break
            fi
        done
        if [ -z "$cfile" ]; then
            echo "error: could not find function $ARG in src/*.c" >&2
            exit 2
        fi
        rel="${cfile#$PROJECT_ROOT/}"
        # Bundle the C file as a synthetic all-added diff, plus any
        # regfix/asmfix rules referencing the function as a synthetic
        # added section. This lets the auditor see the FULL picture for
        # the function: its C body, its pipeline rules, its declared
        # canonical-asm status. Skips the standard staged-diff framing
        # because we're auditing a snapshot, not a change.
        {
            printf 'diff --git a/%s b/%s\n' "$rel" "$rel"
            printf -- '--- /dev/null\n'
            printf -- '+++ b/%s\n' "$rel"
            printf -- '@@ -0,0 +1,%d @@\n' "$(wc -l < "$cfile")"
            sed 's/^/+/' "$cfile"
        } > "$DIFF_FILE"

        # Append regfix/asmfix rules for this function as additional
        # diff sections so the auditor sees the rules in the same view.
        for ruleset in regfix.txt regfix_stage2.txt asmfix.txt; do
            rf="$PROJECT_ROOT/$ruleset"
            [ -f "$rf" ] || continue
            rules=$(grep -E "^${ARG}:" "$rf" 2>/dev/null)
            [ -z "$rules" ] && continue
            n=$(echo "$rules" | wc -l)
            {
                printf '\ndiff --git a/%s b/%s\n' "$ruleset" "$ruleset"
                printf -- '--- /dev/null\n'
                printf -- '+++ b/%s\n' "$ruleset"
                printf -- '@@ -0,0 +1,%d @@\n' "$n"
                echo "$rules" | sed 's/^/+/'
            } >> "$DIFF_FILE"
        done

        LABEL="func $ARG snapshot (C body in $rel + related regfix/asmfix rules)"
        # Set active marker context so the auditor knows what function
        # to focus its reasoning on.
        export BB2_AUDIT_ACTIVE_OVERRIDE="$ARG"
        ;;

    file)
        [ -z "$ARG" ] && usage
        synth_diff_for_file "$ARG"
        LABEL="file $ARG snapshot"
        ;;

    staged)
        git -C "$PROJECT_ROOT" diff --cached > "$DIFF_FILE"
        LABEL="staged diff"
        ;;

    working)
        git -C "$PROJECT_ROOT" diff HEAD > "$DIFF_FILE"
        LABEL="working tree diff (HEAD vs working)"
        ;;

    diff)
        [ -z "$ARG" ] && usage
        if [ ! -f "$ARG" ]; then
            echo "error: diff file not found: $ARG" >&2
            exit 2
        fi
        cp "$ARG" "$DIFF_FILE"
        LABEL="diff from $ARG"
        ;;

    -h|--help|help)
        usage
        ;;

    *)
        echo "error: unknown mode: $MODE" >&2
        usage
        ;;
esac

bytes=$(wc -c < "$DIFF_FILE" 2>/dev/null || echo 0)
echo "[audit-cli] Input: $LABEL"
echo "[audit-cli] Diff size: $bytes bytes"
if [ "$bytes" = "0" ]; then
    echo "[audit-cli] Empty diff — nothing to audit."
    exit 0
fi
echo "[audit-cli] Invoking auditor (advisory mode; verdict goes to stdout, exit always 0)..."
echo

# In func mode we pass the function name explicitly via the active-
# override env var (the auditor's normal source — .bb2_active_func —
# is unrelated to this ad-hoc invocation).
ENV_VARS=(
    "BB2_AUDIT_DIFF=$DIFF_FILE"
    "BB2_AUDIT_ROOT=$PROJECT_ROOT"
    "BB2_AUDIT_ADVISORY=1"
)
if [ -n "$COMMIT_MSG" ]; then
    ENV_VARS+=("BB2_AUDIT_COMMIT_MSG=$COMMIT_MSG")
fi
env "${ENV_VARS[@]}" bash "$SCRIPT_DIR/hooks/llm_audit.sh"
