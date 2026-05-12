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
#   bash tools/dc.sh frame-shift <func> [--delta N] [--apply] — auto-cascade regfix when frame size differs
#   bash tools/dc.sh asmfix-slice <func> <start> <end> [--apply] — lift target asm slice into asmfix.txt
#   bash tools/dc.sh post-match-validate <func>  — sibling-regression check after a match
#   bash tools/dc.sh retire <func>                — start retirement of a bridged function (see feedback_bridge_is_not_decomp.md)
#   bash tools/dc.sh audit-bridges                — caller-signature audit on 209 bridged funcs
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
#   bash tools/dc.sh subagent-prompt <func>       — generate worker-subagent prompt (autonomous mode)
#   bash tools/dc.sh integrate <branch>           — merge worker branch into main with full verification
#   bash tools/dc.sh agent-status                  — drop-in view of running parallel-worker progress
#   bash tools/dc.sh port-prior <func> <branch>   — quick-port src+regfix from prior worker (deep-retry)
#   bash tools/dc.sh stuck-clusters                — scan logs for cluster-level toolchain gaps
#   bash tools/dc.sh classify-batch <funcs...>     — pre-spawn validation; flags already-pure-C entries
#   bash tools/dc.sh permute-adaptive <func>       — permuter with budget scaled to ins+del count
#   bash tools/dc.sh clean-worktrees [--apply]     — safely prune fully-matched worktrees (preserves stuck)
#   bash tools/dc.sh queue-easy [N]                — easiest N queue entries by quick-win score
#   bash tools/dc.sh next-structural [N]           — pull/preview structural split queue
#   bash tools/dc.sh next-asmfix [N]               — pull/preview asmfix retirement queue
#   bash tools/dc.sh fix-asmfix-drift [--apply]    — auto-fix .L<N> rename drift in asmfix.txt
#   bash tools/dc.sh run-log <event> [args...]    — log an autonomous-run event (run-start, func-*, run-end)
#   bash tools/dc.sh run-summary [--all|--json]   — summarize autonomous run(s)
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
            echo "  - 'dc.sh next*' queue pulls (refused while active)"
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

        # Unclosed-autonomous-run warning (silent if log is clean)
        python3 tools/autonomous_run_summary.py --check-open 2>/dev/null

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

1. Pull from WORK_QUEUE.md via `dc.sh next` (or an explicit alternate queue). No hunting.
2. Once selected, the function MUST be matched in 100% pure C before
   you can commit or pull another. The hook blocks otherwise.
3. Stuck = switch *technique*, not target. Escalate via the toolbox:
   classify → attempt → recipes → near-miss → C-techniques → regfix
   → compound regfix → new pipeline pass.
4. Don't ask the user for direction. Don't run `dc.sh release`
   yourself. Build new tools without asking.

--- Modes ---

  Solo (default):       work one function end-to-end yourself.
  Autonomous (user opt-in: "run through queue", "work for N hours",
              "do the next 10 without stopping"):
                        spawn ONE fresh subagent per function via the
                        Agent tool. Subagent context discards on
                        return; main context stays lean over hours.
                        See feedback_workflow_rules.md "Autonomous mode".

--- Quick command reference ---

  dc.sh next [--with-context]    Pull next function (sets active marker)
  dc.sh next-structural [N]      Pull/preview structural split queue
  dc.sh next-asmfix [N]          Pull/preview asmfix retirement queue
  dc.sh classify <func>           Pre-dive: blockers, aliasing_heavy tag
  dc.sh agent-brief <func>        Full context dump
  dc.sh attempt <func>            Auto pipeline (smart→permute→gen_regfix)
  dc.sh diff <func>               Side-by-side: target vs build pipeline
  dc.sh verify <func>             Binary diff one function
  dc.sh fix-label-drift           Auto-fix .L<N> drift after match
  dc.sh refresh-queue             Regen WORK_QUEUE.md (post-match)
  dc.sh subagent-prompt <func>    Generate worker-subagent prompt
                                  (autonomous mode only)
  dc.sh run-log <event> ...       Log autonomous-run event (autonomous mode)
  dc.sh run-summary [--all|--json] Stats from last autonomous run
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
        # Permuter debug output. Default: penalty list + score only (the
        # full --debug emits ~200KB of side-by-side asm). Pass --full for
        # the entire dump. The penalty list alone tells you whether the
        # remaining diff is structural (Insertions/Deletions/Reorders) or
        # register-only — and that's the routing decision you want.
        FUNC_DIR="$1"
        shift || true
        [ -z "$FUNC_DIR" ] && { echo "Usage: dc.sh debug <func_dir> [--full]"; exit 1; }
        if [ "$1" = "--full" ]; then
            python3 tools/decomp-permuter/permuter.py "$FUNC_DIR" --debug 2>&1
        else
            # Capture full output, emit only the penalty-list section + final score.
            python3 tools/decomp-permuter/permuter.py "$FUNC_DIR" --debug 2>&1 \
                | awk '
                    /^--------------- Penalty/ { in_penalty=1 }
                    /^\[.*\] base score/ { in_score=1 }
                    in_penalty || in_score { print }
                    /^End of Debug Mode/ { in_penalty=0; in_score=0 }
                '
            echo "  (default: penalty-only; pass --full to dc.sh debug for the side-by-side asm)"
        fi
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
        # Auto-audit callers — catches signature-arity bugs that cause
        # cascade-regressions in sibling functions (see CLAUDE.md "caller-audit").
        python3 tools/caller_audit.py "$FUNC_NAME" --quiet 2>&1 || {
            echo
            echo "STOP: caller-audit reported an arity mismatch above."
            echo "  Fix the C signature before building, or callers will"
            echo "  silently dead-code the extra args and break sibling functions."
            exit 1
        }
        ;;

    caller-audit)
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh caller-audit <func> [--json] [--quiet]"; exit 1; }
        python3 tools/caller_audit.py "$FUNC_NAME" "$@" 2>&1
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

    regfix-suggest)
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh regfix-suggest <func> [--apply] [--max-rules N]"; exit 1; }
        python3 tools/regfix_suggest.py "$FUNC_NAME" "$@" 2>&1
        ;;

    frame-shift)
        # Auto-generate the frame-cascade regfix rules when GCC's frame size
        # differs from target by N bytes. Emits prologue/epilogue substs +
        # all sw/lw stack-offset shifts as one batch.
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh frame-shift <func> [--delta N] [--apply]"; exit 1; }
        python3 tools/frame_shift.py "$FUNC_NAME" "$@" 2>&1
        ;;

    asmfix-slice)
        # Extract a target asm slice from asm/funcs/<func>.s and emit
        # asmfix delete_between + insert_before rules to lift it into
        # the build pipeline. Last-resort tool when C+permuter+regfix
        # cannot reproduce target's scheduler/allocator decisions.
        FUNC_NAME="$1"
        START_LBL="$2"
        END_LBL="$3"
        shift 3 2>/dev/null || true
        if [ -z "$FUNC_NAME" ] || [ -z "$START_LBL" ] || [ -z "$END_LBL" ]; then
            echo "Usage: dc.sh asmfix-slice <func> <start_label> <end_label> [--apply]"
            exit 1
        fi
        python3 tools/asmfix_slice.py "$FUNC_NAME" "$START_LBL" "$END_LBL" "$@" 2>&1
        ;;

    find-label-at)
        # Resolve a target byte address to the right .L<N> label (or suggest
        # insert_label @ idx) in mine's compile. Eliminates empirical
        # `.L905 -> .L926 -> .L922 -> ...` search when a regfix subst's
        # hardcoded label drifted due to file-wide GCC label renumbering.
        FUNC_NAME="$1"
        ADDR="$2"
        if [ -z "$FUNC_NAME" ] || [ -z "$ADDR" ]; then
            echo "Usage: dc.sh find-label-at <func> <hex_address>"
            echo "Example: dc.sh find-label-at func_8007352C 0x800736DC"
            exit 1
        fi
        python3 tools/find_label_at.py "$FUNC_NAME" "$ADDR" 2>&1
        ;;

    diagnose-hoist)
        # Diff GCC LICM constant hoisting between mine and target. Surfaces
        # "mine hoists X to \$sN, target hoists Y" mismatches and recommends
        # inline-asm-defeat / register-asm-force recipes. First tool when
        # permuter score is high and verify shows callee-save reg-name diffs.
        FUNC_NAME="$1"
        if [ -z "$FUNC_NAME" ]; then
            echo "Usage: dc.sh diagnose-hoist <func>"
            exit 1
        fi
        python3 tools/diagnose_hoist.py "$FUNC_NAME" 2>&1
        ;;

    post-match-validate)
        # After per-function MATCH, detect sibling regressions caused by
        # `.L<N>` numbering shifts in the same .c file. Confirms
        # whole-binary SHA1 + lists regressed siblings with hints.
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh post-match-validate <func>"; exit 1; }
        python3 tools/post_match_validate.py "$FUNC_NAME" 2>&1
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

    subagent-prompt)
        # Generate the canonical worker-subagent prompt for a function.
        # Used by autonomous-mode loops where the main agent spawns a
        # fresh subagent per function (keeps main context lean).
        # Pass --worktree for the parallel-orchestration variant.
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh subagent-prompt <func> [--worktree]"; exit 1; }
        python3 tools/gen_subagent_prompt.py "$FUNC_NAME" "$@" 2>&1
        ;;

    run-log)
        # Append an event to the autonomous run log. Subcommands:
        #   run-start --budget N [--note ...]
        #   func-start <func>
        #   func-end <func> MATCHED|STUCK [--duration N --commit SHA
        #     --retro SHA --recipe "..." --retro-summary "..."
        #     --stuck-reason "..." --note "..."]
        #   run-end [--note ...]
        python3 tools/autonomous_run_log.py "$@" 2>&1
        ;;

    run-summary)
        # Print stats for the last autonomous run (or --all for the
        # full log, or --json for machine-readable).
        python3 tools/autonomous_run_summary.py "$@" 2>&1
        ;;

    next|next-structural|next-asmfix)
        # Print the next function from one WORK_QUEUE.md queue and SET it as the
        # active function in .bb2_active_func. The PreToolUse hook will
        # then block subsequent `git commit` (until verify passes) and queue
        # pulls until the active function is finished.
        #
        # Forms:
        #   dc.sh next                # top 1, set active, no auto-brief
        #   dc.sh next 5              # preview top 5 (no active change)
        #   dc.sh next --with-context # top 1 + auto-run agent-brief
        #   dc.sh next-structural     # top structural split task
        #   dc.sh next-asmfix         # top asmfix retirement task
        case "$CMD" in
            next)
                SECTION="## Queue (top = next)"
                QUEUE_NAME="active decomp queue"
                ;;
            next-structural)
                SECTION="## Structural Split Queue (top = next-structural)"
                QUEUE_NAME="structural split queue"
                ;;
            next-asmfix)
                SECTION="## Asmfix Retirement Queue (top = next-asmfix)"
                QUEUE_NAME="asmfix retirement queue"
                ;;
        esac
        WITH_CONTEXT=0
        NO_PULL=0
        N=1
        for a in "$@"; do
            case "$a" in
                --with-context) WITH_CONTEXT=1 ;;
                --no-pull) NO_PULL=1 ;;
                ''|*[!0-9]*) ;;
                *) N="$a" ;;
            esac
        done

        [ -f "WORK_QUEUE.md" ] || {
            echo "ERROR: WORK_QUEUE.md missing -- run 'dc.sh refresh-queue'" >&2
            exit 1
        }

        # Pull from origin first so we don't hand out a function that
        # someone else (a parallel session, an agent worktree, or a
        # remote teammate) already matched. Skipped on no-network or no-
        # remote setups; failure is non-fatal (warn and continue).
        # Pass --no-pull to skip explicitly.
        if [ "$NO_PULL" != "1" ] && git rev-parse --git-dir >/dev/null 2>&1; then
            if git remote get-url origin >/dev/null 2>&1; then
                if ! git diff-index --quiet HEAD -- 2>/dev/null; then
                    echo "WARNING: working tree has uncommitted changes; skipping git pull." >&2
                else
                    if ! git pull --rebase --quiet origin main 2>/tmp/dc_next_pull_err; then
                        echo "WARNING: 'git pull --rebase origin main' failed (continuing):" >&2
                        head -3 /tmp/dc_next_pull_err >&2
                    fi
                fi
            fi
        fi

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

        # Extract the top N entries from the selected queue.
        TOP=$(awk -v n="$N" -v section="$SECTION" '
            $0 == section { in_queue=1; next }
            in_queue && /^## / { in_queue=0 }
            in_queue && /^```$/ { in_block = !in_block; next }
            in_queue && in_block && /^[[:space:]]*[0-9]+[[:space:]]/ {
                print
                count++
                if (count >= n) exit
            }
        ' WORK_QUEUE.md)
        if [ -z "$TOP" ]; then
            echo "No entries found in $QUEUE_NAME. Run 'dc.sh refresh-queue' if this looks stale." >&2
            exit 1
        fi
        echo "$TOP"

        # Only set the active marker for the top-1 case. `dc.sh next 5`
        # is preview-only and doesn't claim a function.
        if [ "$N" = "1" ]; then
            FUNC=$(echo "$TOP" | head -n 1 | awk '{print $2}')
            if [ -n "$FUNC" ]; then
                # Resolve to the canonical name from asm/funcs/. The CSV
                # (and thus the queue) can carry stale `func_<addr>` raw
                # names from before the function was renamed in
                # asm-by-address. The hook runs `dc.sh verify <ACTIVE>`
                # to decide whether to clear the marker on commit, and
                # that lookup is by exact symbol name -- so the marker
                # MUST be the renamed form (`motion_SavePreCalcData_<addr>`),
                # not the queue's raw form.
                CANON=$(python3 tools/canonical_funcname.py "$FUNC" 2>/dev/null)
                if [ -z "$CANON" ]; then CANON="$FUNC"; fi
                if [ "$CANON" != "$FUNC" ]; then
                    echo "(queue name '$FUNC' resolved to '$CANON' from asm/funcs/)" >&2
                fi
                echo "$CANON" > .bb2_active_func
                echo "(active function set from $QUEUE_NAME: $CANON)" >&2
                if [ "$WITH_CONTEXT" = "1" ]; then
                    echo
                    echo "=== agent-brief ==="
                    python3 tools/agent_brief.py "$CANON" 2>&1
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
            # If this was a retirement-in-progress, restore the bridge by
            # un-commenting the `# RETIRE: ...` line in asmfix.txt.
            if grep -q "^# RETIRE: ${ACTIVE}: replace_with_asmfile" asmfix.txt 2>/dev/null; then
                python3 -c "
import re, sys
from pathlib import Path
p = Path('asmfix.txt')
lines = p.read_text(encoding='utf-8').splitlines()
fn = '$ACTIVE'
out = []
restored = 0
for line in lines:
    if line.startswith('# RETIRE: ') and re.match(rf'^# RETIRE: {re.escape(fn)}\s*:\s*replace_with_asmfile\b', line):
        out.append(line[len('# RETIRE: '):])
        restored += 1
    else:
        out.append(line)
p.write_text('\n'.join(out) + '\n', encoding='utf-8')
print(f'  restored {restored} bridge rule(s) (un-commented `# RETIRE: ...`)')
"
            fi
        else
            echo "Mismatch (expected '$ACTIVE', got '$CONFIRM'). Not releasing." >&2
            exit 1
        fi
        ;;

    retire)
        # Start retirement of a bridged (replace_with_asmfile) function:
        # comment out the asmfix rule, set active marker, point at C body.
        # See feedback_bridge_is_not_decomp.md.
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh retire <func>"; exit 1; }
        python3 tools/dc_retire.py "$FUNC_NAME" 2>&1
        ;;

    fix-label-drift)
        # Auto-fix .L<N> drift in regfix.txt rules. Default is dry-run;
        # pass --apply to actually edit regfix.txt. Drives off linker
        # errors (only fixes rules where the build genuinely broke).
        # Pass --also-verify to additionally run `dc.sh verify --all` to
        # catch existing-but-wrong-address labels (slow ~3-5 min).
        python3 tools/fix_label_drift.py "$@" 2>&1
        ;;

    fix-asmfix-drift)
        # Auto-fix .L<N> drift in asmfix.txt rename rules (the asmfix-slice
        # version of fix-label-drift). Useful after adding pure-C stubs that
        # shift file-wide GCC labels: each stub adds 1 to the .L<N> counter
        # for all subsequent funcs in the same .c file, breaking
        # rename ".L<N>" ".L<HEX_ADDR>" rules. This tool re-resolves the
        # right .L<N> from the absolute address in the rename target.
        python3 tools/fix_asmfix_drift.py "$@" 2>&1
        ;;

    diff)
        # Side-by-side diff: target asm vs build pipeline output for
        # one function. Useful for spotting reorderings, register
        # choices, and ins/del when iterating on a NEAR_MISS.
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh diff <func>"; exit 1; }
        python3 tools/diff_build.py "$FUNC_NAME" 2>&1
        ;;

    diff-align)
        # Sequence-aligned binary diff with relocation masking and recipe
        # detection. Use FIRST when a build fails — this collapses the
        # cascade caused by single-instruction length mismatches and shows
        # only the real STRUCTURAL diffs, plus suggested fixes (delay-slot
        # fill, label shift, hoist removal). Index-aligned `dc.sh diff` is
        # noisy when mine and target differ in length; this is not.
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh diff-align <func>"; exit 1; }
        python3 tools/diff_align.py "$FUNC_NAME" "$@" 2>&1
        ;;

    refresh-queue)
        # Refresh classifier CSV + regenerate WORK_QUEUE.md. Run after a
        # batch of matches so the queue drops them. ~2 minutes full, ~10s
        # in --fast mode.
        # ALSO clears .bb2_active_func -- if you got here, you matched.
        FAST_MODE=0
        for a in "$@"; do
            case "$a" in
                --fast) FAST_MODE=1 ;;
            esac
        done

        # --fast: skip the slow batch_attempt --classify-only pass
        # (~3 min on the current backlog). Use between consecutive
        # autonomous-mode matches when only one function changed and the
        # CSV doesn't need a full re-scan. Phases [1] capture-recipe,
        # [2] strip orphans, [3] purge `# RETIRE:` lines on clean build,
        # and [4] regen WORK_QUEUE.md from the existing CSV still run.
        # Full mode adds phases for batch_attempt + classify_asmfix.
        if [ "$FAST_MODE" = "1" ]; then
            echo "[1/4] (FAST) Capturing recipe from latest commit..."
            python3 tools/capture_recipe.py HEAD 2>&1 | tail -5 || true
            echo
            echo "[2/4] (FAST) Stripping orphan named_syms.txt assignments..."
            python3 tools/audit_named_syms_orphans.py --apply 2>&1 | tail -3 || true
            echo
            echo "[3/4] (FAST) Cleaning up completed # RETIRE: lines..."
            if grep -q "^# RETIRE: " asmfix.txt 2>/dev/null; then
                python3 -c "
import re, subprocess
from pathlib import Path
p = Path('asmfix.txt')
lines = p.read_text(encoding='utf-8').splitlines()
am = Path('.bb2_active_func')
active = am.read_text(encoding='utf-8').strip() if am.exists() else ''
res = subprocess.run(['bash', 'tools/dc.sh', 'verify', '--all'], capture_output=True, text=True, timeout=60)
clean = 'SHA1 match:' in res.stdout
if not clean:
    print('  build not clean; leaving # RETIRE lines in place')
else:
    out, purged = [], 0
    for line in lines:
        m = re.match(r'^# RETIRE:\s*(\w+)\s*:\s*replace_with_asmfile\b', line)
        if m and m.group(1) != active:
            purged += 1
            continue
        out.append(line)
    if purged:
        p.write_text('\n'.join(out) + '\n', encoding='utf-8')
    print(f'  purged {purged} completed retirement(s)')
"
            fi
            echo
            echo "[4/4] (FAST) Regenerating WORK_QUEUE.md from existing CSV..."
            python3 tools/gen_work_queue.py 2>&1 | tail -5
            : > .bb2_active_func 2>/dev/null || true
            exit 0
        fi

        echo "[1/6] Capturing recipe from latest commit (if it's a match)..."
        python3 tools/capture_recipe.py HEAD 2>&1 | tail -5 || true
        echo
        echo "[2/6] Stripping orphan named_syms.txt assignments..."
        # Orphans are `<name> = 0x<addr>;` entries that shadow a real
        # text-section symbol now defined by a C .o (typically because
        # the function got decompiled but its old asm-link assignment
        # was left behind). The shadow form breaks `dc.sh verify <name>`
        # symbol lookups in the linker map.
        python3 tools/audit_named_syms_orphans.py --apply 2>&1 | tail -3 || true
        echo
        echo "[3/6] Cleaning up completed retirements (# RETIRE: lines now matching)..."
        # If a `# RETIRE: <func>: replace_with_asmfile ...` line exists AND
        # the build matches AND the function is no longer the active marker,
        # the retirement succeeded — remove the commented bridge rule.
        if grep -q "^# RETIRE: " asmfix.txt 2>/dev/null; then
            python3 -c "
import re, subprocess
from pathlib import Path
p = Path('asmfix.txt')
lines = p.read_text(encoding='utf-8').splitlines()
# Active marker (must NOT be the function whose retirement we'd remove)
am = Path('.bb2_active_func')
active = am.read_text(encoding='utf-8').strip() if am.exists() else ''
# Only purge retirements once build is SHA1-clean
res = subprocess.run(['bash', 'tools/dc.sh', 'verify', '--all'], capture_output=True, text=True, timeout=60)
clean = 'SHA1 match:' in res.stdout
if not clean:
    print('  build not clean; leaving # RETIRE lines in place')
else:
    out, purged = [], 0
    for line in lines:
        m = re.match(r'^# RETIRE:\s*(\w+)\s*:\s*replace_with_asmfile\b', line)
        if m and m.group(1) != active:
            purged += 1
            continue
        out.append(line)
    if purged:
        p.write_text('\n'.join(out) + '\n', encoding='utf-8')
    print(f'  purged {purged} completed retirement(s)')
"
        fi
        echo
        echo "[4/6] Classifying inline_asm + INCLUDE_ASM (batch_attempt --classify-only)..."
        python3 tools/batch_attempt.py --classify-only 2>&1 | tail -3
        echo
        echo "[5/6] Classifying replace_with_asmfile entries..."
        python3 tools/classify_asmfix.py 2>&1 | tail -3
        echo
        echo "[6/6] Generating WORK_QUEUE.md..."
        python3 tools/gen_work_queue.py 2>&1
        # Belt-and-suspenders: the commit-via-hook path already cleared
        # the marker, but if for some reason a refresh runs while it's
        # set, we know the queue is being regenerated and the active
        # function is no longer relevant.
        : > .bb2_active_func 2>/dev/null || true
        ;;

    agent-status)
        # Snapshot of all running parallel-orchestration workers.
        # Reads tmp/parallel_logs/<func>.log from each worker (written
        # via tools/agent_log.sh) and prints a per-worker summary.
        # Safe to run any time; doesn't touch worker state.
        python3 tools/agent_status.py "$@" 2>&1
        ;;

    port-prior)
        # Deep-retry quick-port: copy src body + regfix rules for <func>
        # from a prior worker branch into the current worktree, then run
        # fix-label-drift, build, and verify. Does NOT commit -- the
        # worker reviews and commits explicitly.
        #
        # Usage: dc.sh port-prior <func> <prior_branch>
        #
        # Audit-driven: deep retries that ported prior work matched in
        # 9-21min (~150K tokens). Ones that re-ran the search took
        # 90-150min (~400-600K tokens).
        FUNC_NAME="$1"
        PRIOR_BRANCH="$2"
        [ -z "$PRIOR_BRANCH" ] && { echo "Usage: dc.sh port-prior <func> <prior_branch>"; exit 1; }

        if ! git rev-parse --verify "$PRIOR_BRANCH" >/dev/null 2>&1; then
            echo "ERROR: branch '$PRIOR_BRANCH' not found" >&2
            exit 1
        fi

        # Find which src/<file>.c contains the function on the prior branch
        SRC_FILE=$(git show "$PRIOR_BRANCH" --name-only --format= | grep '^src/.*\.c$' | head -1)
        if [ -z "$SRC_FILE" ]; then
            echo "ERROR: prior branch didn't change any src/*.c" >&2
            exit 1
        fi
        echo "[port-prior] src file: $SRC_FILE"

        # Use python to splice in just the function body (between the
        # first occurrence of '<type> <FUNC_NAME>(' and the closing brace)
        python3 tools/port_prior.py "$FUNC_NAME" "$PRIOR_BRANCH" "$SRC_FILE" 2>&1 || {
            echo "ERROR: port_prior.py failed" >&2
            exit 1
        }

        # Apply prior regfix rules (append, dedup at file level)
        echo "[port-prior] applying regfix rules..."
        git show "$PRIOR_BRANCH" -- regfix.txt > /tmp/port_prior_regfix_diff.patch 2>/dev/null || true

        # Drift fix
        echo "[port-prior] running fix-label-drift..."
        python3 tools/fix_label_drift.py --apply 2>&1 | tail -3

        # Build
        echo "[port-prior] building..."
        BUILD_OUT=$(make 2>&1 | tail -5)
        echo "$BUILD_OUT" | tail -3

        if echo "$BUILD_OUT" | grep -q "OK: bb2 matches"; then
            echo "[port-prior] BUILD OK -- function should match. Run 'dc.sh verify $FUNC_NAME' to confirm, then commit."
        else
            echo "[port-prior] BUILD MISMATCH. Run 'dc.sh diff $FUNC_NAME' to see what's off (likely 1-2 instructions)."
        fi
        ;;

    stuck-clusters)
        # Scan tmp/parallel_logs/ for STUCK events that share toolchain
        # gap keywords (multu, schedule-insns, register-cycle, etc.).
        # Emits a CLUSTER ALERT when 2+ functions match the same
        # pattern -- coordinator should pause spawning and have ONE
        # worker build the shared fix before re-spawning cluster
        # members.
        python3 tools/agent_status.py 2>&1 | sed -n '/CLUSTER ALERT/,/Action:/p'
        ;;

    permute-adaptive)
        # Run permuter with budget scaled to ins+del penalty count.
        # 0 ins/del -> skip; 1-2 -> 90s; 3-5 -> 5min; 6-10 -> 15min; >10 -> 30min.
        # Backed by tools/permute_adaptive.py.
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh permute-adaptive <func> [--dry-run]"; exit 1; }
        python3 tools/permute_adaptive.py "$FUNC_NAME" "$@" 2>&1
        ;;

    clean-worktrees)
        # Safely prune fully-matched worker worktrees. SAFETY:
        # only worktrees with 0 commits ahead OR fully-merged branches
        # are removed. Preserved-stuck branches (unique commits not
        # in main) are NEVER deleted.
        # Backed by tools/worktree_janitor.py.
        # Default is dry-run; pass --apply to actually remove.
        python3 tools/worktree_janitor.py "$@" 2>&1
        ;;

    queue-easy)
        # Show easiest N queue entries by 'expected ease' score
        # (lower = quicker win). Recommends quick-win prioritization.
        # Backed by tools/queue_easy.py.
        python3 tools/queue_easy.py "$@" 2>&1
        ;;

    queue-structural)
        bash tools/dc.sh next-structural "${1:-10}" --no-pull
        ;;

    queue-asmfix)
        bash tools/dc.sh next-asmfix "${1:-10}" --no-pull
        ;;

    classify-batch)
        # Pre-spawn validation: classify N functions in parallel and
        # emit which ones are stale (already pure C). Use this BEFORE
        # spawning workers so you skip no-op queue entries.
        #
        # Usage: dc.sh classify-batch <func1> [<func2> ...]
        [ -z "$1" ] && { echo "Usage: dc.sh classify-batch <func1> [<func2> ...]"; exit 1; }
        for FUNC_NAME in "$@"; do
            CAT=$(python3 tools/classify_func.py "$FUNC_NAME" 2>&1 | grep "src" | head -1)
            if echo "$CAT" | grep -q "c_function"; then
                printf '  STALE  %-32s (already pure C in main -- skip)\n' "$FUNC_NAME"
            elif echo "$CAT" | grep -qE "inline_asm|INCLUDE_ASM|replace_with_asmfile"; then
                printf '  WORK   %-32s (needs match)\n' "$FUNC_NAME"
            else
                printf '  ?      %-32s (%s)\n' "$FUNC_NAME" "$CAT"
            fi
        done
        ;;

    integrate)
        # Sequential integration of a worker subagent's branch into main.
        # Used by parallel orchestration: workers match on their own
        # branches, then the coordinator (parent agent) calls this once
        # per successful worker to merge into main with full verification.
        #
        # On failure the merge is reverted and the branch is left intact
        # for inspection. Exit codes:
        #   0  integrated cleanly
        #   1  preflight failure (wrong branch, dirty tree, branch missing)
        #   2  merge/build/verify failure (auto-reverted)
        BRANCH="$1"
        [ -z "$BRANCH" ] && { echo "Usage: dc.sh integrate <branch>"; exit 1; }

        # Preflight: must be on main, clean tree, no active marker
        CUR_BRANCH=$(git symbolic-ref --short HEAD 2>/dev/null || echo "")
        if [ "$CUR_BRANCH" != "main" ]; then
            echo "ERROR: must be on main (currently: '$CUR_BRANCH')" >&2
            exit 1
        fi
        if [ -n "$(git status --porcelain)" ]; then
            echo "ERROR: working tree not clean — commit or stash first" >&2
            git status --short >&2
            exit 1
        fi
        if [ -s ".bb2_active_func" ]; then
            ACTIVE=$(tr -d '[:space:]' < .bb2_active_func)
            echo "ERROR: main has an active function set ($ACTIVE); refusing to integrate" >&2
            echo "  Coordinator should never have an active marker on main." >&2
            exit 1
        fi
        if ! git rev-parse --verify "$BRANCH" >/dev/null 2>&1; then
            echo "ERROR: branch '$BRANCH' not found" >&2
            exit 1
        fi

        AHEAD=$(git rev-list --count "main..$BRANCH")
        if [ "$AHEAD" = "0" ]; then
            echo "[integrate] $BRANCH has no commits ahead of main — nothing to do"
            exit 0
        fi
        echo "[integrate] $BRANCH has $AHEAD commit(s) ahead of main"

        PRE_SHA=$(git rev-parse HEAD)
        echo "[integrate] pre-state: $PRE_SHA"

        # Merge with --no-ff so the integration is visible in history.
        # --no-edit keeps the default merge message and avoids spawning
        # an editor (which would block in non-interactive contexts).
        if ! git merge --no-ff --no-edit "$BRANCH" 2>&1; then
            echo "[integrate] FAIL: merge conflict — manual resolution required"
            git merge --abort 2>/dev/null || true
            git reset --hard "$PRE_SHA" 2>/dev/null || true
            exit 2
        fi
        POST_MERGE_SHA=$(git rev-parse HEAD)
        echo "[integrate] merged at $POST_MERGE_SHA"

        # Build and check SHA1
        echo "[integrate] building..."
        BUILD_OUT=$(make 2>&1 | tail -10)
        echo "$BUILD_OUT" | tail -3

        BUILD_OK=0
        if echo "$BUILD_OUT" | grep -q "OK: bb2 matches"; then
            BUILD_OK=1
            echo "[integrate] build OK on first try"
        else
            echo "[integrate] build mismatch — trying fix-label-drift"
            DRIFT_OUT=$(python3 tools/fix_label_drift.py --apply 2>&1 | tail -5)
            echo "$DRIFT_OUT"
            BUILD_OUT=$(make 2>&1 | tail -10)
            echo "$BUILD_OUT" | tail -3
            if echo "$BUILD_OUT" | grep -q "OK: bb2 matches"; then
                BUILD_OK=1
                echo "[integrate] build OK after fix-label-drift"
                if [ -n "$(git status --porcelain)" ]; then
                    git add -A
                    git commit -m "Auto: fix-label-drift after integrating $BRANCH" >/dev/null
                fi
            fi
        fi

        if [ "$BUILD_OK" != "1" ]; then
            echo "[integrate] FAIL: build still broken — reverting to $PRE_SHA"
            git reset --hard "$PRE_SHA"
            exit 2
        fi

        # verify --all: any per-function regression triggers revert
        echo "[integrate] verify --all..."
        VERIFY_OUT=$(python3 tools/regfix_verify.py --all 2>&1)
        echo "$VERIFY_OUT" | tail -3
        if echo "$VERIFY_OUT" | grep -qiE "FAIL|MISMATCH|differ"; then
            echo "[integrate] FAIL: verify --all reported regressions — reverting to $PRE_SHA"
            echo "$VERIFY_OUT" | grep -iE "FAIL|MISMATCH|differ" | head -10 >&2
            git reset --hard "$PRE_SHA"
            exit 2
        fi

        # Refresh queue (drops the matched function) and commit any updates
        echo "[integrate] refresh queue..."
        bash tools/dc.sh refresh-queue 2>&1 | tail -3
        if [ -n "$(git status --porcelain)" ]; then
            git add -A
            git commit -m "Auto: refresh queue after integrating $BRANCH" >/dev/null
        fi

        FINAL_SHA=$(git rev-parse HEAD)
        echo "[integrate] OK: $BRANCH integrated as $FINAL_SHA"
        ;;

    audit-bridges)
        # Audit caller signatures vs stub signatures for all bridged
        # (replace_with_asmfile) functions. Reports ACTIVE / latent /
        # informational issues. JSON written to tmp/bridge_signature_audit.json.
        # See feedback_bridge_signature_audit.md.
        mkdir -p tmp
        python3 tools/audit_bridge_signatures.py "$@"
        ;;

    *)
        echo "Unknown command: $CMD"
        echo "Commands: compile, score, debug, build, replace, setup,"
        echo "          inline-locate, inline-verify, inline-setup, inline-replace,"
        echo "          smart, permute, add-regfix, classify, gte, attempt,"
        echo "          recipes, apply-recipe,"
        echo "          analysis, dump-text, validate-regfix, gen-regfix, verify,"
        echo "          frame-shift, asmfix-slice, find-label-at, diagnose-hoist,"
        echo "          next, next-structural, next-asmfix, refresh-queue, release,"
        echo "          retire, audit-bridges"
        exit 1
        ;;
esac
