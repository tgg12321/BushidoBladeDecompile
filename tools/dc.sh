#!/bin/bash
# dc.sh — Decomp helper for Claude Code sessions
# Eliminates fragile WSL one-liners. All commands run from project root.
#
# Usage (from WSL):
#   bash tools/dc.sh compile <func_dir>          — compile + objdump
#   bash tools/dc.sh score <func_dir>             — permuter score only
#   bash tools/dc.sh debug <func_dir>             — permuter --debug (full diff)
#   bash tools/dc.sh build                        — incremental make, tail output
#   bash tools/dc.sh replace <src_file> <func_name> <c_file>  — replace INCLUDE_ASM with C code (LF-safe)
#   bash tools/dc.sh setup <func_name> <src_file> — permuter setup (INCLUDE_ASM stub form)
#   bash tools/dc.sh inline-locate <func>         — locate inline __asm__ block in src/
#   bash tools/dc.sh inline-verify <func>         — verify inline asm matches asm/funcs/.s
#   bash tools/dc.sh inline-setup <func>          — set up permuter/<func>/ for inline-asm function
#   bash tools/dc.sh inline-replace <func> <c>    — replace inline __asm__ block with C from <c>
#   bash tools/dc.sh smart <func>                  — run smart_match.py 16-strategy sweep
#   bash tools/dc.sh permute <func> [opts]        — run permuter with early-termination cap
#   bash tools/dc.sh add-regfix <func> <op> <args> @ <idx>  — append validated regfix rule
#   bash tools/dc.sh classify <func>              — pre-dive classification report
#   bash tools/dc.sh gte <func>                    — gte_*() macro suggestion report
#   bash tools/dc.sh attempt <func>               — full mechanical attempt pipeline
#   bash tools/dc.sh recipes [<func>]             — list recipes / suggest recipes for <func>
#   bash tools/dc.sh apply-recipe <recipe> <func> — apply a named recipe to <func>
#   bash tools/dc.sh analysis <func_name>         — run asm_analysis.py
#   bash tools/dc.sh dump-text <func_name> [src]  — dump numbered TEXT indices from pipeline
#   bash tools/dc.sh validate-regfix [--func F]   — validate regfix.txt rules (static)
#   bash tools/dc.sh validate-regfix --live [--func F] — validate rules against pipeline
#   bash tools/dc.sh gen-regfix <func_name> [src] — auto-generate regfix rules from diff
#   bash tools/dc.sh verify <func_name>           — binary-level verify function against original
#   bash tools/dc.sh verify --all                  — verify all C functions
#
set -eo pipefail

# Find project root
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$SCRIPT_DIR/.."
cd "$ROOT"

# Activate venv if present
if [ -f ".venv/bin/activate" ]; then
    source .venv/bin/activate
fi

CMD="$1"
shift || { echo "Usage: bash tools/dc.sh <command> [args...]"; exit 1; }

case "$CMD" in
    start)
        # Session startup briefing. Surfaces current state (build, active
        # marker, queue freshness, top-of-queue) and reminds the agent of
        # the rule-of-engagement one-liner. Auto-run by the SessionStart
        # hook so future agents get this without being told.
        echo "=== BB2 Decomp Session Startup ==="
        echo

        # Build status (silent unless mismatch)
        if [ -f "build/bb2.exe" ] && [ -f "bb2.sha1" ]; then
            if sha1sum -c bb2.sha1 >/dev/null 2>&1; then
                echo "Build:    OK (SHA1 matches)"
            else
                echo "Build:    MISMATCH — last build did not match. Investigate before any work."
            fi
        else
            echo "Build:    not built yet — run 'bash tools/dc.sh build' to verify clean state"
        fi

        # Active marker state
        if [ -s ".bb2_active_func" ]; then
            ACTIVE=$(tr -d '[:space:]' < .bb2_active_func)
            echo "Active:   $ACTIVE  (in progress — finish before pulling another)"
            echo
            echo "  Resume work on $ACTIVE. The hook is enforcing — you cannot:"
            echo "  - 'dc.sh next' (refused while active)"
            echo "  - 'git commit' (refused unless dc.sh verify $ACTIVE shows MATCH)"
            echo "  - 'git checkout/restore' on src/ files"
            echo
            echo "  Diagnostic:"
            echo "    bash tools/dc.sh verify $ACTIVE"
            echo "    bash tools/dc.sh diff $ACTIVE"
        else
            echo "Active:   NONE (clear to pull from queue)"
        fi

        # Queue freshness
        if [ -f "WORK_QUEUE.md" ]; then
            QUEUE_AGE=$(( ( $(date +%s) - $(stat -c %Y WORK_QUEUE.md 2>/dev/null || stat -f %m WORK_QUEUE.md 2>/dev/null || echo 0) ) / 86400 ))
            COUNT=$(grep -c "^[[:space:]]*[0-9]\+[[:space:]]\+func\|^[[:space:]]*[0-9]\+[[:space:]]\+sa\|^[[:space:]]*[0-9]\+[[:space:]]\+[A-Za-z]" WORK_QUEUE.md 2>/dev/null || echo "?")
            if [ "$QUEUE_AGE" -gt 7 ] 2>/dev/null; then
                echo "Queue:    WORK_QUEUE.md is $QUEUE_AGE days old — run 'dc.sh refresh-queue' before pulling"
            else
                echo "Queue:    WORK_QUEUE.md current (${QUEUE_AGE}d old)"
            fi
        else
            echo "Queue:    WORK_QUEUE.md missing — run 'dc.sh refresh-queue' first"
        fi

        echo
        echo "--- Top of queue ---"
        if [ -f "WORK_QUEUE.md" ] && [ ! -s ".bb2_active_func" ]; then
            awk -v n=3 '
                /^## Queue/ { in_queue=1; next }
                in_queue && /^## / { in_queue=0 }
                in_queue && /^```$/ { in_block = !in_block; next }
                in_queue && in_block && /^[[:space:]]*[0-9]+[[:space:]]/ {
                    print "  " $0
                    count++
                    if (count >= n) exit
                }
            ' WORK_QUEUE.md
        elif [ -s ".bb2_active_func" ]; then
            echo "  (suppressed — finish active function first)"
        fi

        cat <<'EOF'

--- THE HARD RULE (enforced by hook) ---

1. Pull from WORK_QUEUE.md via `dc.sh next`. No hunting.
2. Once selected, the function MUST be matched in 100% pure C before
   you can commit or pull another. The hook blocks otherwise.
3. Stuck = switch *technique*, not target. Escalate via the toolbox:
   classify → attempt → recipes → near-miss → C-techniques → regfix
   → compound regfix → new pipeline pass.
4. Don't ask the user for direction. Don't run `dc.sh release`
   yourself. Build new tools without asking.

--- Quick command reference ---

  dc.sh next [--with-context]    Pull next function (sets active marker)
  dc.sh classify <func>           Pre-dive: blockers, aliasing_heavy tag
  dc.sh agent-brief <func>        Full context dump
  dc.sh attempt <func>            Auto pipeline (smart→permute→gen_regfix)
  dc.sh diff <func>               Side-by-side: target vs build pipeline
  dc.sh verify <func>             Binary diff one function
  dc.sh fix-label-drift           Auto-fix .L<N> drift after match
  dc.sh refresh-queue             Regen WORK_QUEUE.md (post-match)
  dc.sh release                   ESCAPE HATCH (user-only, typed confirm)

--- Where to read ---

  CLAUDE.md                       project rules (already loaded)
  WORK_QUEUE.md                   ordered work list
  memory MEMORY.md links to:
    feedback_workflow_rules.md      THE HARD RULE, escalation, integration
    feedback_matching_playbook.md   techniques, recipes, penalty→technique
                                    routing, things that don't work
    feedback_regfix_reference.md    regfix.txt syntax + every gotcha

EOF
        ;;

    compile)
        # Compile a permuter base.c and dump disassembly
        FUNC_DIR="$1"
        [ -z "$FUNC_DIR" ] && { echo "Usage: dc.sh compile <func_dir>"; exit 1; }
        OUT="/tmp/dc_test_$$.o"
        trap 'rm -f "$OUT"' EXIT
        bash "$FUNC_DIR/compile.sh" "$FUNC_DIR/base.c" -o "$OUT" 2>&1
        mipsel-linux-gnu-objdump -d "$OUT"
        ;;

    score)
        # Get permuter score (number only)
        FUNC_DIR="$1"
        [ -z "$FUNC_DIR" ] && { echo "Usage: dc.sh score <func_dir>"; exit 1; }
        python3 tools/decomp-permuter/permuter.py "$FUNC_DIR" --debug 2>&1 | grep "score"
        ;;

    debug)
        # Full permuter debug output (diff + score)
        FUNC_DIR="$1"
        [ -z "$FUNC_DIR" ] && { echo "Usage: dc.sh debug <func_dir>"; exit 1; }
        python3 tools/decomp-permuter/permuter.py "$FUNC_DIR" --debug 2>&1
        ;;

    build)
        # Incremental build + SHA1 check
        make 2>&1 | tail -5
        ;;

    replace)
        # Replace INCLUDE_ASM stub with C code from a file (LF-safe)
        SRC_FILE="$1"
        FUNC_NAME="$2"
        C_FILE="$3"
        [ -z "$C_FILE" ] && { echo "Usage: dc.sh replace <src_file> <func_name> <c_file>"; exit 1; }
        [ ! -f "$SRC_FILE" ] && { echo "ERROR: $SRC_FILE not found"; exit 1; }
        [ ! -f "$C_FILE" ] && { echo "ERROR: $C_FILE not found"; exit 1; }
        # Loudly identify which repo we are operating on. Past agents have hit
        # the main repo instead of their worktree because dc.sh cd's to its own
        # SCRIPT_DIR/.. — print the absolute resolved path so the caller can
        # spot the wrong target before damage is done.
        ABS_SRC="$(cd "$(dirname "$SRC_FILE")" && pwd)/$(basename "$SRC_FILE")"
        BRANCH="$(git symbolic-ref --short HEAD 2>/dev/null || echo '<detached>')"
        echo "REPLACE: $ABS_SRC (branch: $BRANCH)"
        if [ "$BRANCH" = "main" ]; then
            echo "WARNING: operating on main branch — if you intended a worktree, abort now."
        fi
        # Read replacement C code (strip any CR)
        REPLACEMENT="$(tr -d '\r' < "$C_FILE")"
        # Do the replacement using python (safe with any content)
        python3 -c "
import sys
src = '$SRC_FILE'
func = '$FUNC_NAME'
with open(src, 'r') as f:
    content = f.read()
stub = 'INCLUDE_ASM(\"asm/funcs\", ' + func + ');'
if stub not in content:
    print(f'ERROR: stub {stub} not found in {src}', file=sys.stderr)
    sys.exit(1)
with open('$C_FILE', 'r') as f:
    repl = f.read().rstrip()
content = content.replace(stub, repl)
with open(src, 'w', newline='\n') as f:
    f.write(content)
print(f'Replaced {func} in {src}')
"
        ;;

    setup)
        # Set up permuter directory for a function
        FUNC_NAME="$1"
        SRC_FILE="$2"
        [ -z "$SRC_FILE" ] && { echo "Usage: dc.sh setup <func_name> <src_file>"; exit 1; }
        bash tools/permuter_setup.sh "$FUNC_NAME" "$SRC_FILE"
        ;;

    analysis)
        # Run asm_analysis on a function
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh analysis <func_name>"; exit 1; }
        python3 tools/asm_analysis.py "asm/funcs/${FUNC_NAME}.s" 2>&1
        ;;

    dump-text)
        # Dump numbered TEXT indices from build pipeline.
        # Use --post-regfix to dump indices AFTER regfix passes (useful for
        # debugging multi-rule regfix sequences and verifying inserted
        # instructions land where intended).
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh dump-text <func_name> [src_file] [--post-regfix]"; exit 1; }
        shift
        python3 tools/dump_text_indices.py "$FUNC_NAME" "$@"
        ;;

    siblings)
        # Find structural siblings of a function. Use this when a sibling
        # of a matched function is likely already on the work queue —
        # porting the matched body usually takes 10-15 min vs 30-60 for
        # novel structures.
        if [ "$1" = "--all" ]; then
            python3 tools/siblings.py --all
        else
            FUNC_NAME="$1"
            [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh siblings <func_name> | --all"; exit 1; }
            python3 tools/siblings.py "$FUNC_NAME"
        fi
        ;;

    check-gp-layout)
        # Verify build's symbol layout matches canonical addresses.
        # Catches silent gp-rel miscompiles (e.g. D_800A3514 ending up at
        # gp+1092 instead of gp+1096 because sdata ordering differs).
        # Run after layout-affecting changes; exit code 1 = mismatch.
        python3 tools/check_gp_layout.py "$@"
        ;;

    validate-regfix)
        # Validate regfix.txt rules
        python3 tools/validate_regfix.py "$@" 2>&1
        ;;

    gen-regfix)
        # Auto-generate regfix rules from pipeline vs target diff
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh gen-regfix <func_name> [src_file]"; exit 1; }
        SRC_FILE="${2:-}"
        if [ -n "$SRC_FILE" ]; then
            python3 tools/gen_regfix.py "$FUNC_NAME" "$SRC_FILE"
        else
            python3 tools/gen_regfix.py "$FUNC_NAME"
        fi
        ;;

    verify)
        # Binary-level verification of function against original
        ARG1="${1:---help}"
        if [ "$ARG1" = "--help" ]; then
            echo "Usage: dc.sh verify <func_name>   — verify one function"
            echo "       dc.sh verify --all          — verify all C functions"
            exit 1
        fi
        python3 tools/regfix_verify.py "$ARG1" 2>&1
        ;;

    inline-locate)
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh inline-locate <func>"; exit 1; }
        python3 tools/extract_inline_asm.py locate "$FUNC_NAME" 2>&1
        ;;

    inline-verify)
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh inline-verify <func>"; exit 1; }
        python3 tools/extract_inline_asm.py verify "$FUNC_NAME" 2>&1
        ;;

    inline-setup)
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh inline-setup <func>"; exit 1; }
        python3 tools/extract_inline_asm.py setup "$FUNC_NAME" 2>&1
        ;;

    inline-replace)
        FUNC_NAME="$1"
        C_FILE="$2"
        [ -z "$C_FILE" ] && { echo "Usage: dc.sh inline-replace <func> <c_file>"; exit 1; }
        python3 tools/extract_inline_asm.py replace "$FUNC_NAME" "$C_FILE" 2>&1
        ;;

    smart)
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh smart <func> [smart_match flags]"; exit 1; }
        python3 tools/smart_match.py --func "$FUNC_NAME" "$@" 2>&1
        ;;

    permute)
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh permute <func> [--max-flat N] [--max-time S]"; exit 1; }
        python3 tools/permute_capped.py "$FUNC_NAME" "$@" 2>&1
        ;;

    add-regfix)
        [ -z "$1" ] && { echo "Usage: dc.sh add-regfix <func> <op> <args...> @ <idx>"; exit 1; }
        python3 tools/add_regfix.py "$@" 2>&1
        ;;

    classify)
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh classify <func>"; exit 1; }
        python3 tools/classify_func.py "$FUNC_NAME" 2>&1
        ;;

    gte)
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh gte <func>"; exit 1; }
        python3 tools/gte_classifier.py "$FUNC_NAME" 2>&1
        ;;

    gte-migrate)
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh gte-migrate <func> [--stdout|--setup]"; exit 1; }
        python3 tools/gte_migrate.py "$FUNC_NAME" "$@" 2>&1
        ;;

    attempt)
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh attempt <func> [--time N] [--max-flat N]"; exit 1; }
        python3 tools/attempt_func.py "$FUNC_NAME" "$@" 2>&1
        ;;

    recipes)
        FUNC_NAME="${1:-}"
        if [ -z "$FUNC_NAME" ]; then
            python3 tools/recipes.py list 2>&1
        else
            python3 tools/recipes.py suggest "$FUNC_NAME" 2>&1
        fi
        ;;

    near-miss)
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh near-miss <func> [--apply] [--no-c-edit]"; exit 1; }
        python3 tools/near_miss_attempt.py "$FUNC_NAME" "$@" 2>&1
        ;;

    agent-brief)
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh agent-brief <func> [--no-asm] [--json]"; exit 1; }
        python3 tools/agent_brief.py "$FUNC_NAME" "$@" 2>&1
        ;;

    capture-recipe)
        COMMIT="${1:-HEAD}"
        shift || true
        python3 tools/capture_recipe.py "$COMMIT" "$@" 2>&1
        ;;

    apply-recipe)
        RECIPE="$1"
        FUNC_NAME="$2"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh apply-recipe <recipe> <func>"; exit 1; }
        python3 tools/recipes.py apply "$RECIPE" "$FUNC_NAME" 2>&1
        ;;

    next)
        # Print the next function from WORK_QUEUE.md and SET it as the
        # active function in .bb2_active_func. The PreToolUse hook will
        # then block subsequent `git commit` (until verify passes) and
        # `dc.sh next` calls until the active function is finished.
        #
        # Forms:
        #   dc.sh next                # top 1, set active, no auto-brief
        #   dc.sh next 5              # preview top 5 (no active change)
        #   dc.sh next --with-context # top 1 + auto-run agent-brief
        WITH_CONTEXT=0
        N=1
        for a in "$@"; do
            case "$a" in
                --with-context) WITH_CONTEXT=1 ;;
                ''|*[!0-9]*) ;;
                *) N="$a" ;;
            esac
        done

        [ -f "WORK_QUEUE.md" ] || {
            echo "ERROR: WORK_QUEUE.md missing -- run 'dc.sh refresh-queue'" >&2
            exit 1
        }

        # Refuse if a function is already in progress. (The hook also
        # enforces this, but exiting cleanly here gives a better error.)
        if [ -s ".bb2_active_func" ]; then
            ACTIVE=$(tr -d '[:space:]' < .bb2_active_func)
            echo "ERROR: $ACTIVE is still in progress (set by an earlier 'dc.sh next')." >&2
            echo "Finish it (build matches + commit) before pulling another function." >&2
            echo "If the user has explicitly authorized abandoning this function:" >&2
            echo "  bash tools/dc.sh release   # requires typing the function name to confirm" >&2
            exit 1
        fi

        # Stale-queue check: warn if WORK_QUEUE.md is older than 7 days.
        if [ -f WORK_QUEUE.md ]; then
            QUEUE_AGE_DAYS=$(( ( $(date +%s) - $(stat -c %Y WORK_QUEUE.md 2>/dev/null || stat -f %m WORK_QUEUE.md 2>/dev/null || echo 0) ) / 86400 ))
            if [ "$QUEUE_AGE_DAYS" -gt 7 ] 2>/dev/null; then
                echo "WARNING: WORK_QUEUE.md is $QUEUE_AGE_DAYS days old. Run 'dc.sh refresh-queue' to drop matched functions." >&2
            fi
        fi

        # Extract the top N entries from the queue.
        TOP=$(awk -v n="$N" '
            /^## Queue/ { in_queue=1; next }
            in_queue && /^## / { in_queue=0 }
            in_queue && /^```$/ { in_block = !in_block; next }
            in_queue && in_block && /^[[:space:]]*[0-9]+[[:space:]]/ {
                print
                count++
                if (count >= n) exit
            }
        ' WORK_QUEUE.md)
        echo "$TOP"

        # Only set the active marker for the top-1 case. `dc.sh next 5`
        # is preview-only and doesn't claim a function.
        if [ "$N" = "1" ]; then
            FUNC=$(echo "$TOP" | head -n 1 | awk '{print $2}')
            if [ -n "$FUNC" ]; then
                echo "$FUNC" > .bb2_active_func
                echo "(active function set: $FUNC)" >&2
                if [ "$WITH_CONTEXT" = "1" ]; then
                    echo
                    echo "=== agent-brief ==="
                    python3 tools/agent_brief.py "$FUNC" 2>&1
                fi
            fi
        fi
        ;;

    release)
        # Explicit user-authorized abandonment of the current active
        # function. Requires typing the function name to confirm — this
        # is the ONLY escape hatch from the hard rule, and it's user-
        # driven, never agent-driven.
        if [ ! -s ".bb2_active_func" ]; then
            echo "No active function to release."
            exit 0
        fi
        ACTIVE=$(tr -d '[:space:]' < .bb2_active_func)
        echo "About to release: $ACTIVE"
        echo "This abandons WIP per explicit user authorization."
        echo "Type the function name exactly to confirm:"
        read -r CONFIRM
        if [ "$CONFIRM" = "$ACTIVE" ]; then
            : > .bb2_active_func
            echo "Released $ACTIVE. The hook will no longer block on this function."
        else
            echo "Mismatch (expected '$ACTIVE', got '$CONFIRM'). Not releasing." >&2
            exit 1
        fi
        ;;

    fix-label-drift)
        # Auto-fix .L<N> drift in regfix.txt rules. Default is dry-run;
        # pass --apply to actually edit regfix.txt. Drives off linker
        # errors (only fixes rules where the build genuinely broke).
        shift || true
        python3 tools/fix_label_drift.py "$@" 2>&1
        ;;

    diff)
        # Side-by-side diff: target asm vs build pipeline output for
        # one function. Useful for spotting reorderings, register
        # choices, and ins/del when iterating on a NEAR_MISS.
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh diff <func>"; exit 1; }
        python3 tools/diff_build.py "$FUNC_NAME" 2>&1
        ;;

    refresh-queue)
        # Refresh classifier CSV + regenerate WORK_QUEUE.md. Run after a
        # batch of matches so the queue drops them. ~2 minutes.
        # ALSO clears .bb2_active_func -- if you got here, you matched.
        echo "[1/4] Capturing recipe from latest commit (if it's a match)..."
        python3 tools/capture_recipe.py HEAD 2>&1 | tail -5 || true
        echo
        echo "[2/4] Classifying inline_asm + INCLUDE_ASM (batch_attempt --classify-only)..."
        python3 tools/batch_attempt.py --classify-only 2>&1 | tail -3
        echo
        echo "[3/4] Classifying replace_with_asmfile entries..."
        python3 tools/classify_asmfix.py 2>&1 | tail -3
        echo
        echo "[4/4] Generating WORK_QUEUE.md..."
        python3 tools/gen_work_queue.py 2>&1
        # Belt-and-suspenders: the commit-via-hook path already cleared
        # the marker, but if for some reason a refresh runs while it's
        # set, we know the queue is being regenerated and the active
        # function is no longer relevant.
        : > .bb2_active_func 2>/dev/null || true
        ;;

    *)
        echo "Unknown command: $CMD"
        echo "Commands: compile, score, debug, build, replace, setup,"
        echo "          inline-locate, inline-verify, inline-setup, inline-replace,"
        echo "          smart, permute, add-regfix, classify, gte, attempt,"
        echo "          recipes, apply-recipe,"
        echo "          analysis, dump-text, validate-regfix, gen-regfix, verify,"
        echo "          next, refresh-queue, release"
        exit 1
        ;;
esac
