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
#   bash tools/dc.sh add-regfix <op> <func> <args>  — append/modify validated regfix rule (op comes FIRST)
#   bash tools/dc.sh frame-shift <func> [--delta N] [--apply] — auto-cascade regfix when frame size differs
#   bash tools/dc.sh asmfix-slice <func> <start> <end> [--apply] — lift target asm slice into asmfix.txt
#   bash tools/dc.sh post-match-validate <func>  — sibling-regression check after a match
#   bash tools/dc.sh retire <func>                — start retirement of a bridged function (see feedback_bridge_is_not_decomp.md)
#   bash tools/dc.sh audit-bridges                — caller-signature audit on 209 bridged funcs
#   bash tools/dc.sh scan-canonical [--single F] [--diff]  — unified Tier 1/2 canonical-asm classifier + registry drift report
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
#   bash tools/dc.sh classify-batch <funcs...>     — classify a list of funcs; flags stale (already-pure-C) entries
#   bash tools/dc.sh permute-adaptive <func>       — permuter with budget scaled to ins+del count
#   bash tools/dc.sh next-structural [N]           — pull/preview structural split queue
#   bash tools/dc.sh next-asmfix [N]               — pull/preview asmfix retirement queue
#   bash tools/dc.sh next-cheat [N]                — pull/preview inline-asm cheat-fix work (active queue filtered to inline_asm_debt)
#   bash tools/dc.sh next-cheat-cleanup [N]        — pull/preview cheat-cleanup queue (every audit-flagged cheat func; entries also queued elsewhere get an `also:<queue>` tag)
#   bash tools/dc.sh next-for-file <files> [N]     — pull/preview active queue filtered to comma-separated src files (parallel sub-agent partition)
#   bash tools/dc.sh fix-asmfix-drift [--apply]    — auto-fix .L<N> rename drift in asmfix.txt
#   bash tools/dc.sh auto-repair                    — build + detect cascade-drift + auto-run repair tools (used by build-active)
#   bash tools/dc.sh preflight-cascade <func>       — read-only impact report: how many sibling rules at risk of label drift
#   bash tools/dc.sh suggest <func>                  — lean rollup: state + top techniques + don't-tries + next step (auto pre/post)
#   bash tools/dc.sh bootstrap                       — link gitignored deps from main repo (run after EnterWorktree mid-session)
#   bash tools/dc.sh verify-toolchain                — fast existence-check on cc1, maspsx, .venv, disc (~10ms; diagnoses link-failure mysteries)
#   bash tools/dc.sh new-worktree <name>             — create .claude/worktrees/<name> from local main HEAD, bootstrapped, ready to use
#   bash tools/dc.sh log-attempt <func> <cat> <variant> <min> '<outcome>'  — log a decomp attempt
#   bash tools/dc.sh check-attempts <func>           — validate attempts log vs escape-valve gate
#
# Cross-environment behavior:
#   When invoked from Git Bash / MSYS / Cygwin (Windows native shell), this
#   script re-execs itself under `wsl bash` at the same working directory.
#   That way the canonical one-call (`bash tools/dc.sh start`) works from
#   anywhere — main checkout, any worktree, Git Bash or WSL — without the
#   agent needing to know which shell or where on disk. WSL Linux is detected
#   by uname -s == Linux and the bounce is skipped.
case "$(uname -s 2>/dev/null)" in
    MINGW*|MSYS*|CYGWIN*)
        # Convert Git Bash POSIX path (/c/foo) → WSL mount path (/mnt/c/foo).
        WT_PWD_RAW="$(pwd)"
        case "$WT_PWD_RAW" in
            /[a-zA-Z]/*) WSL_PWD="/mnt$WT_PWD_RAW" ;;
            *)           WSL_PWD="$WT_PWD_RAW" ;;
        esac
        # Re-quote all args via printf %q so spaces / $vars in arguments survive.
        ARGS_Q=""
        for a in "$@"; do
            ARGS_Q="$ARGS_Q $(printf %q "$a")"
        done
        exec wsl bash -c "cd \"$WSL_PWD\" && bash tools/dc.sh$ARGS_Q"
        ;;
esac

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
    lessons)
        # Query the git commit corpus for project knowledge.
        # See tools/commit_lessons.py --help for full options.
        # Examples:
        #   dc.sh lessons func_8004A348              # all commits mentioning the function
        #   dc.sh lessons "delay slot fill"          # keyword search across subjects + bodies
        #   dc.sh lessons --type cheat-cleanup -n 30 # last 30 cheat-cleanup commits
        # ($@ is already post-CMD per the shift at line 62)
        python3 tools/commit_lessons.py "$@"
        ;;
    start)
        # Session startup briefing. Surfaces current state (build, active
        # marker, queue freshness, top-of-queue) and reminds the agent of
        # the rule-of-engagement one-liner. Auto-run by the SessionStart
        # hook so future agents get this without being told.
        echo "=== BB2 Decomp Session Startup ==="
        echo

        # Worktree auto-link: bootstrap symlinks gitignored binary deps
        # (cc1, .venv, etc.) from the main repo into this worktree.
        # Extracted to tools/worktree_bootstrap.sh so it can also be invoked
        # explicitly (`dc.sh bootstrap`) or by wsl.sh when an agent creates
        # a worktree mid-session (no SessionStart re-fire).
        if [ -f tools/worktree_bootstrap.sh ]; then
            _bootstrap_msg=$(bash tools/worktree_bootstrap.sh 2>&1) || true
            if [ -n "$_bootstrap_msg" ]; then
                echo "$_bootstrap_msg"
                echo
            fi
        fi

        # Regenerate MEMORY.md from current memory/*/*.md frontmatter so
        # the auto-loaded index stays current. Silent on success; loud on
        # broken-link / size-limit failures (those would truncate session
        # context if left unaddressed).
        if [ -f tools/regen_memory_index.py ]; then
            if ! _msg=$(python3 tools/regen_memory_index.py 2>&1); then
                echo "Memory:   INDEX REGEN FAILED"
                echo "$_msg" | sed 's/^/          /'
                echo
            fi
        fi

        # Root cleanliness check — surface suspicious / unknown root files.
        # Silent if clean; one-line summary + offenders if not.
        if [ -f tools/check_root_cleanliness.py ]; then
            _root_msg=$(python3 tools/check_root_cleanliness.py 2>&1)
            if [ $? -ne 0 ]; then
                # Compact: show only the summary line + offenders, not the full advice
                echo "$_root_msg" | grep -E "^Root scan:|^SUSPICIOUS|^UNKNOWN|^  [a-zA-Z0-9_.\(]" | sed 's/^/Root:     /' | head -10
                echo
            fi
        fi

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
            # Detect retirement state: an active `# RETIRE: <func>:` line in
            # asmfix.txt means the bridge is commented out and we're working
            # toward a pure-C body. The bridge-aware verify-c / diff-align
            # are the correct diagnostics; plain verify is bridge-blind.
            IS_RETIREMENT=0
            if grep -q "^# RETIRE: ${ACTIVE}: replace_with_asmfile" asmfix.txt 2>/dev/null; then
                IS_RETIREMENT=1
            fi
            if [ "$IS_RETIREMENT" = "1" ]; then
                echo "Active:   $ACTIVE  (RETIREMENT in progress — bridge commented out)"
            else
                echo "Active:   $ACTIVE  (in progress — finish before pulling another)"
            fi
            echo
            echo "  Resume work on $ACTIVE. The hook is enforcing — you cannot:"
            echo "  - 'dc.sh next*' queue pulls (refused while active)"
            echo "  - 'git commit' (refused unless verify shows MATCH)"
            echo "  - 'git checkout/restore' on src/ files"
            echo
            echo "  Diagnostic:"
            if [ "$IS_RETIREMENT" = "1" ]; then
                echo "    bash tools/dc.sh build-active $ACTIVE   # rebuild + bridge-aware verify-c"
                echo "    bash tools/dc.sh diff-align $ACTIVE     # real structural diffs"
                echo "    bash tools/dc.sh diff-summary $ACTIVE   # categorized verdict + next action"
            else
                echo "    bash tools/dc.sh verify $ACTIVE"
                echo "    bash tools/dc.sh diff $ACTIVE"
            fi
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

        # Asm-cheat debt. A "cheat" is a function whose binary bytes
        # come from regfix splice or file-scope `__asm__("glabel ...")`
        # instead of from real C compilation — functionally equivalent
        # to a `replace_with_asmfile` bridge but historically invisible
        # to the bridge guard. The active_func_guard hook blocks NEW
        # cheats; this line surfaces the EXISTING ones that should be
        # revisited.
        if [ -f "tools/audit_asm_cheats.py" ]; then
            python3 tools/audit_asm_cheats.py --summary 2>/dev/null || true
        fi

        # SOTN-bar gap: tier-3 inline asm (toolchain workarounds — register
        # pins, INLINE_MOVE_ALIASING blocks, scheduling barriers). These
        # are the deviation-from-SOTN-standard count. Tier-2 (authentic
        # GTE/BIOS/hardware) is fine and not part of the gap.
        if [ -f "tools/classify_inline_asm.py" ]; then
            python3 tools/classify_inline_asm.py --summary 2>/dev/null || true
        fi

        # Cascade-drift surface (literal-`.LN` rules across asmfix+regfix).
        # Auto-repair handles drift on rebuild, but tracking the surface
        # count over time is a useful trend metric — if it keeps growing,
        # the architectural Phase 3 fix (`rename @ <idx>` syntax) becomes
        # worth building. See memory/feedback_auto_drift_repair.md.
        if [ -f "tools/drift_surface_summary.py" ]; then
            python3 tools/drift_surface_summary.py 2>/dev/null || true
        fi

        echo
        echo "--- Top of queue ---"
        if [ -f "WORK_QUEUE.md" ] && [ ! -s ".bb2_active_func" ]; then
            QUEUE_OUT=$(awk -v n=3 '
                /^## Queue/ { in_queue=1; next }
                in_queue && /^## / { in_queue=0 }
                in_queue && /^```$/ { in_block = !in_block; next }
                in_queue && in_block && /^[[:space:]]*[0-9]+[[:space:]]/ {
                    print "  " $0
                    count++
                    if (count >= n) exit
                }
            ' WORK_QUEUE.md)
            if [ -n "$QUEUE_OUT" ]; then
                echo "$QUEUE_OUT"
                echo "  (pull with: bash tools/dc.sh next --with-context)"
            else
                # Active decomp queue empty — surface what IS pullable so
                # the agent/user doesn't read the blank section as "done."
                echo "  Active decomp queue: EMPTY"
                STRUCT_COUNT=$(awk '/^\| Structural Split Queue \|/ { gsub(/[^0-9]/, "", $4); print $4; exit }' WORK_QUEUE.md 2>/dev/null)
                ASMFIX_COUNT=$(awk '/^\| Asmfix Retirement Queue \|/ { gsub(/[^0-9]/, "", $4); print $4; exit }' WORK_QUEUE.md 2>/dev/null)
                [ -n "$STRUCT_COUNT" ] && [ "$STRUCT_COUNT" -gt 0 ] 2>/dev/null && echo "  Structural Split Queue: $STRUCT_COUNT items — pull with: bash tools/dc.sh next-structural --with-context"
                [ -n "$ASMFIX_COUNT" ] && [ "$ASMFIX_COUNT" -gt 0 ] 2>/dev/null && echo "  Asmfix Retirement Queue: $ASMFIX_COUNT items — pull with: bash tools/dc.sh next-asmfix --with-context"
                if [ -z "$STRUCT_COUNT$ASMFIX_COUNT" ] || { [ "${STRUCT_COUNT:-0}" = "0" ] && [ "${ASMFIX_COUNT:-0}" = "0" ]; }; then
                    echo "  All queues empty — decomp work complete."
                fi
            fi
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

--- Quick command reference ---

  dc.sh next [--with-context]    Pull next function (sets active marker)
  dc.sh next-structural [N]      Pull/preview structural split queue
  dc.sh next-asmfix [N]          Pull/preview asmfix retirement queue
  dc.sh next-cheat [N]           Pull next inline-asm cheat-fix work item
  dc.sh next-cheat-cleanup [N]   Pull next cheat-cleanup work item (matching-via-cheat funcs)
  dc.sh next-for-file <files> [N]  Pull next active-queue item from given .c file(s) (parallel sub-agent use)
  dc.sh classify <func>           Pre-dive: blockers, aliasing_heavy tag
  dc.sh agent-brief <func>        Full context dump
  dc.sh attempt <func>            Auto pipeline (smart→permute→gen_regfix)
  dc.sh diff <func>               Side-by-side: target vs build pipeline
  dc.sh verify <func>             Binary diff one function
  dc.sh fix-label-drift           Auto-fix .L<N> drift after match
  dc.sh refresh-queue             Regen WORK_QUEUE.md (post-match)
  dc.sh log-attempt <func> <cat> <variant> <min> '<outcome>'  Log a decomp attempt to .bb2_attempts/
  dc.sh check-attempts <func>     Validate attempts log against escape-valve gate
  dc.sh classify-inline-asm       Tier-2 (authentic) vs tier-3 (workaround) inline-asm metrics
  dc.sh new-worktree <name>       Create + bootstrap a worktree off local main HEAD (one command)
  dc.sh bootstrap                 Link gitignored deps from main repo (worktree first-run)
  dc.sh verify-toolchain          Fast existence-check on cc1, maspsx, .venv, disc
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

    bootstrap)
        # Explicit worktree bootstrap — symlinks gitignored binary deps
        # (gcc-2.7.2, decomp-permuter, .venv, disc) from the
        # main repo into this worktree. Idempotent; safe to run multiple
        # times. No-op in the main repo.
        #
        # Use this after creating a worktree mid-session (the SessionStart
        # hook only fires once per session, so a worktree created via
        # EnterWorktree won't get auto-bootstrapped until something runs
        # `dc.sh start` again).
        if [ ! -f tools/worktree_bootstrap.sh ]; then
            echo "ERROR: tools/worktree_bootstrap.sh missing — checkout is incomplete" >&2
            exit 1
        fi
        bash tools/worktree_bootstrap.sh
        ;;

    setup-venv)
        # Create/repair the local .venv with the packages listed in
        # requirements.txt. Idempotent — safe to re-run when requirements
        # change or when something looks broken. Run from the worktree
        # whose .venv you want to (re)build (typically main).
        #
        # worktree_bootstrap.sh auto-invokes this on main when it detects
        # main's .venv is missing or doesn't have the required packages,
        # so fresh worktrees inherit a working venv without manual setup.
        if [ ! -f requirements.txt ]; then
            echo "ERROR: requirements.txt missing in $(pwd)" >&2
            echo "  setup-venv reads its package list from requirements.txt at the repo root." >&2
            exit 1
        fi
        if [ ! -x .venv/bin/python3 ]; then
            echo "setup-venv: creating .venv ..."
            if ! python3 -m venv .venv; then
                echo "ERROR: python3 -m venv failed — install python3-venv?" >&2
                exit 1
            fi
        else
            echo "setup-venv: .venv exists, updating ..."
        fi
        # ensurepip in case the venv was created without pip (the Debian/Ubuntu
        # default; `python3 -m venv` skips pip unless --upgrade-deps).
        .venv/bin/python3 -m ensurepip --upgrade >/dev/null 2>&1 || true
        echo "setup-venv: installing requirements ..."
        if ! .venv/bin/python3 -m pip install --upgrade --quiet --disable-pip-version-check -r requirements.txt 2>&1; then
            echo "ERROR: pip install failed" >&2
            exit 1
        fi
        # Validate: only check the CRITICAL imports we actually use in
        # routine workflows. Don't iterate requirements.txt — splat's
        # top-level import chain drags in N64-specific deps that take
        # extra effort to satisfy and aren't critical for PSX builds.
        # If a new module becomes critical, add it to VALIDATE_IMPORTS.
        VALIDATE_IMPORTS="toml"
        _missing=""
        for mod in $VALIDATE_IMPORTS; do
            if ! .venv/bin/python3 -c "import $mod" 2>/dev/null; then
                _missing="$_missing $mod"
            fi
        done
        if [ -n "$_missing" ]; then
            echo "ERROR: installed but failed to import:$_missing" >&2
            echo "  Run: .venv/bin/python3 -c 'import <module>'  to see the actual error" >&2
            exit 1
        fi
        echo "setup-venv: OK"
        ;;

    classify-inline-asm)
        # Classify every __asm__ block + register-asm pin in src/*.c as
        # tier-2 (authentic GTE/BIOS/hardware) or tier-3 (toolchain
        # workaround for our gcc-vs-cc1psx allocator/scheduler divergence).
        # Tier-3 count is the gap-to-SOTN-bar metric.
        python3 tools/classify_inline_asm.py "$@"
        ;;

    log-attempt)
        # Append an attempt entry to .bb2_attempts/<func>.jsonl. The
        # escape-valve commit hook (tools/hooks/escape_valve_guard.sh)
        # gates INLINE_MOVE_ALIASING / asmfix bridge / canonical-asm
        # commits on the log showing ≥4 distinct categories, ≥6
        # attempts, ≥30 minutes of effort.
        #
        # Usage: dc.sh log-attempt <func> <category> <variant> <min> '<outcome>'
        # See tools/check_attempts.py for the category list.
        python3 tools/log_attempt.py "$@"
        ;;

    check-attempts)
        # Validate <func>'s attempt log against the escape-valve gate.
        # Exits 0 (pass) or 1 (fail). Prints which categories are still
        # needed when failing.
        python3 tools/check_attempts.py "$@"
        ;;

    new-worktree)
        # One-command worktree setup: creates .claude/worktrees/<name> on
        # a fresh branch off local main HEAD (not origin/main, which is
        # often behind), runs the bootstrap, prints next-step hints.
        #
        # Replaces the multi-step EnterWorktree → reset --hard to local
        # main → bootstrap dance. Use this whenever you're about to start
        # work that should not happen on main (i.e., almost always —
        # see tools/hooks/main_branch_guard.sh).
        NAME="${1:-}"
        if [ -z "$NAME" ]; then
            echo "Usage: bash tools/dc.sh new-worktree <name>" >&2
            echo "" >&2
            echo "  Creates .claude/worktrees/<name> on branch worktree-<name>" >&2
            echo "  from local main HEAD (not origin), bootstrapped with deps." >&2
            echo "" >&2
            echo "  Example: bash tools/dc.sh new-worktree cheat-cleanup-side" >&2
            exit 1
        fi
        # Validate: no slashes, no shell metacharacters.
        case "$NAME" in
            *[!a-zA-Z0-9._-]*|''|.|..)
                echo "ERROR: name must be alphanumeric (._- allowed), got '$NAME'" >&2
                exit 1
                ;;
        esac
        # Find the main worktree. Can't use `git worktree list` from a
        # sub-worktree under WSL — the .git pointer file is written with
        # Windows paths by Git Bash's `git worktree add`, which WSL git
        # interprets relative to cwd (same trap that worktree_bootstrap.sh
        # works around). Parse .git directly instead.
        if [ -d .git ]; then
            MAIN_WT="$(pwd)"
        elif [ -f .git ]; then
            GITDIR_LINE=$(head -1 .git | sed 's|^gitdir: *||')
            if [ -d /mnt/c ] && [ "${GITDIR_LINE#?:/}" != "$GITDIR_LINE" ]; then
                DRIVE=$(echo "$GITDIR_LINE" | cut -c1 | tr 'A-Z' 'a-z')
                GITDIR_ABS="/mnt/$DRIVE${GITDIR_LINE#?:}"
            else
                GITDIR_ABS="$GITDIR_LINE"
            fi
            MAIN_WT=$(dirname "$(dirname "$(dirname "$GITDIR_ABS")")")
        else
            echo "ERROR: not a git repository (no .git here)" >&2
            exit 1
        fi
        if [ ! -d "$MAIN_WT" ]; then
            echo "ERROR: resolved main worktree doesn't exist: $MAIN_WT" >&2
            exit 1
        fi
        WT_REL=".claude/worktrees/$NAME"
        WT_PATH="$MAIN_WT/$WT_REL"
        BRANCH="worktree-$NAME"
        if [ -e "$WT_PATH" ]; then
            echo "ERROR: $WT_REL already exists at $WT_PATH" >&2
            exit 1
        fi
        if git -C "$MAIN_WT" show-ref --quiet "refs/heads/$BRANCH"; then
            echo "ERROR: branch $BRANCH already exists" >&2
            echo "  Delete with: git -C \"$MAIN_WT\" branch -D $BRANCH" >&2
            exit 1
        fi
        # Resolve local main HEAD (not origin/main — they often diverge).
        MAIN_HEAD=$(git -C "$MAIN_WT" rev-parse main 2>/dev/null)
        if [ -z "$MAIN_HEAD" ]; then
            echo "ERROR: couldn't resolve local main HEAD" >&2
            exit 1
        fi
        echo "Creating worktree:"
        echo "  Path:   $WT_PATH"
        echo "  Branch: $BRANCH"
        echo "  Base:   main @ ${MAIN_HEAD:0:8}"
        echo
        if ! git -C "$MAIN_WT" worktree add -b "$BRANCH" "$WT_REL" "$MAIN_HEAD"; then
            echo "ERROR: git worktree add failed" >&2
            exit 1
        fi
        # Re-register with a Windows-native path. WSL git records the worktree
        # path as /mnt/X/... which Windows-side tools (EnterWorktree, native
        # `git worktree list` from Git Bash) can't navigate — EnterWorktree
        # tries to lstat C:\mnt\c\... and fails with ENOENT. git.exe (Git for
        # Windows) is reachable from WSL via the inherited Windows PATH and
        # accepts Windows-native paths, so we use it for a one-shot repair.
        # No-op on real Linux hosts (git.exe absent) and on Git Bash (this
        # code already ran post-WSL-bounce, but the guards keep it safe).
        WT_PATH_WIN="$WT_PATH"
        if [ -d /mnt/c ] && command -v git.exe >/dev/null 2>&1; then
            _to_win() {
                case "$1" in
                    /mnt/[a-zA-Z]/*)
                        _drv=$(printf '%s' "$1" | cut -c6 | tr 'a-z' 'A-Z')
                        printf '%s:%s\n' "$_drv" "${1#/mnt/?}"
                        ;;
                    *)
                        printf '%s\n' "$1"
                        ;;
                esac
            }
            WT_PATH_WIN=$(_to_win "$WT_PATH")
            _mw_win=$(_to_win "$MAIN_WT")
            git.exe -C "$_mw_win" worktree repair "$WT_PATH_WIN" >/dev/null 2>&1 || true
        fi
        # Bootstrap deps in the new worktree.
        echo
        echo "Bootstrapping deps..."
        if [ -f "$WT_PATH/tools/worktree_bootstrap.sh" ]; then
            (cd "$WT_PATH" && bash tools/worktree_bootstrap.sh) || {
                echo "WARNING: bootstrap reported errors — worktree created but deps may be incomplete" >&2
            }
        else
            echo "WARNING: worktree_bootstrap.sh missing in checkout — run setup manually" >&2
        fi
        echo
        echo "Ready. To switch into it from Claude Code:"
        echo "  EnterWorktree path=\"$WT_PATH_WIN\""
        echo
        echo "From a shell:"
        if [ "$WT_PATH_WIN" != "$WT_PATH" ]; then
            echo "  WSL:      cd \"$WT_PATH\""
            echo "  Git Bash: cd \"/${WT_PATH#/mnt/}\""
        else
            echo "  cd \"$WT_PATH\""
        fi
        ;;

    verify-toolchain)
        # Fast existence-check on the critical build deps. Used to diagnose
        # "why does my build fail with weird link errors" in ~10ms instead
        # of waiting 90s for a full build to produce a confusing error.
        # Hard requirements: cc1 binary, maspsx, python3 in PATH, original EXE.
        # Soft requirements (warning only): .venv (system python3 also works).
        _missing=()
        _warnings=()
        [ -x tools/gcc-2.7.2/build/cc1 ]      || _missing+=("tools/gcc-2.7.2/build/cc1 (cc1 compiler — fix: dc.sh bootstrap, or rebuild via tools/setup_wsl.sh)")
        [ -f tools/maspsx/maspsx.py ]         || _missing+=("tools/maspsx/maspsx.py (maspsx — fix: git submodule update --init)")
        command -v python3 >/dev/null 2>&1    || _missing+=("python3 in PATH (fix: install python3 or activate .venv)")
        [ -f disc/SLUS_006.63 ]               || _missing+=("disc/SLUS_006.63 (original EXE — fix: dc.sh bootstrap, or python3 tools/extract_iso.py)")
        # Advisory: .venv (build falls back to system python3, but the
        # permuter assumes `toml` is importable from the venv).
        if [ ! -x .venv/bin/python3 ]; then
            _warnings+=(".venv/bin/python3 (using system python3 — fix: bash tools/dc.sh setup-venv)")
        else
            # Check only critical modules (toml). splat's N64 import chain
            # isn't relevant for routine PSX work.
            _broken_mods=""
            for _mod in toml; do
                if ! .venv/bin/python3 -c "import $_mod" 2>/dev/null; then
                    _broken_mods="$_broken_mods $_mod"
                fi
            done
            if [ -n "$_broken_mods" ]; then
                _warnings+=(".venv missing critical module(s):$_broken_mods (fix: bash tools/dc.sh setup-venv)")
            fi
        fi
        # Advisory: decomp-permuter (required for `dc.sh permute/attempt`,
        # not for plain build/verify). Common silent-fail mode: directory
        # exists but is empty (clone got wiped). Check the actual marker.
        [ -f tools/decomp-permuter/permuter.py ] || _warnings+=("tools/decomp-permuter/permuter.py (permuter not installed — fix: rmdir tools/decomp-permuter && git clone https://github.com/simonlindholm/decomp-permuter.git tools/decomp-permuter)")
        # cc1psx.exe is deprecated for everyday work (2026-05-18). See
        # memory/rules/cc1psx-calibration-only.md — empirical findings
        # showed compiler choice doesn't move tier-3 retirement. Not
        # checked here.
        if [ "${#_missing[@]}" -eq 0 ]; then
            if [ "${#_warnings[@]}" -eq 0 ]; then
                echo "Toolchain: OK (cc1, maspsx, python3, disc present; .venv present)"
            else
                echo "Toolchain: OK with ${#_warnings[@]} advisory:"
                for w in "${_warnings[@]}"; do
                    echo "  $w"
                done
            fi
            exit 0
        fi
        echo "Toolchain: MISSING ${#_missing[@]} required dep(s):" >&2
        for m in "${_missing[@]}"; do
            echo "  $m" >&2
        done
        if [ "${#_warnings[@]}" -gt 0 ]; then
            echo "" >&2
            echo "Plus ${#_warnings[@]} advisory:" >&2
            for w in "${_warnings[@]}"; do
                echo "  $w" >&2
            done
        fi
        echo "" >&2
        echo "Fix: bash tools/dc.sh bootstrap   (links from main repo if it has them)" >&2
        echo "Or:  bash tools/setup_wsl.sh      (full reinstall — slow)" >&2
        exit 1
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
        # Incremental build + SHA1 check. make_check.py surfaces any regfix
        # rules that silently no-op'd during the build (drift-broken literal
        # labels) — otherwise they scroll past in the build log.
        python3 tools/make_check.py --tail 5
        ;;

    replace)
        # Replace INCLUDE_ASM stub with C code from a file (LF-safe)
        SRC_FILE="$1"
        FUNC_NAME="$2"
        C_FILE="$3"
        [ -z "$C_FILE" ] && { echo "Usage: dc.sh replace <src_file> <func_name> <c_file>"; exit 1; }
        [ ! -f "$SRC_FILE" ] && { echo "ERROR: $SRC_FILE not found"; exit 1; }
        [ ! -f "$C_FILE" ] && { echo "ERROR: $C_FILE not found"; exit 1; }
        # Identify which file we're modifying so the caller can spot a wrong
        # target before damage is done.
        ABS_SRC="$(cd "$(dirname "$SRC_FILE")" && pwd)/$(basename "$SRC_FILE")"
        echo "REPLACE: $ABS_SRC"
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

    check-permuter-parity)
        # Verify tools/permuter_compile.sh's cc1 + maspsx flags match the
        # Makefile's. If they drift, the permuter scores base.c against
        # different codegen than the real build (the func_8007CE0C trap:
        # permuter_compile.sh was missing --expand-lb/--expand-dest-funcs).
        # exit 1 = drift.
        python3 tools/check_permuter_parity.py "$@"
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
            echo "       dc.sh verify --clean        — clean rebuild then verify"
            echo
            echo "  --clean: removes build/ and rebuilds before checking. Use when"
            echo "  you suspect cached .o files are hiding regressions (e.g., as a"
            echo "  pre-commit safety check after register-allocation-sensitive work)."
            exit 1
        fi
        # --clean: clean rebuild then verify --all. Catches regressions
        # hidden by stale .o cache (the trap that broke func_8003D39C
        # between commits 54a5e54 and c71ff0a — a function's "matched"
        # state on the build cache silently diverged from a fresh
        # build's output).
        if [ "$ARG1" = "--clean" ]; then
            echo "[verify --clean] Removing build/ and rebuilding from source..."
            rm -rf build
            if [ -d ".venv" ]; then
                # shellcheck disable=SC1091
                source .venv/bin/activate 2>/dev/null || true
            fi
            # make_check.py surfaces silently-no-op'd regfix rules. This is
            # the canonical full rebuild, so refresh the regfix-warning
            # baseline here (and only here) on a match.
            if ! python3 tools/make_check.py --tail 3 --update-baseline; then
                echo "ERROR: make failed during --clean rebuild" >&2
                exit 1
            fi
            shift || true
            echo "[verify --clean] Running verify --all on fresh build..."
            python3 tools/regfix_verify.py --all "$@" 2>&1
            VERIFY_RC=$?
            if [ "$VERIFY_RC" -ne 0 ]; then
                # Whole-binary SHA1 failed. verify --all's per-function loop
                # only checks functions its return-type regex recognizes, so
                # it can miss the actual regression (this is exactly how the
                # func_8007CE0C sibling regression hid behind "all match").
                # diagnose_mismatch.py works off the linker map — it maps
                # EVERY differing byte to its function and flags drift-prone
                # regfix rules.
                echo
                echo "[verify --clean] Whole-binary mismatch — running diagnosis..."
                python3 tools/diagnose_mismatch.py 2>&1 || true
            fi
            exit "$VERIFY_RC"
        fi
        # Pre-check: if the function has an active asmfix bridge entry,
        # warn loudly. The verify report below will say MATCH no matter
        # what's in src/ because asmfix.py overwrites the .o bytes
        # with asm/funcs/<func>.s content. The match is meaningless
        # for retirement work — your C body is dead code.
        if grep -qE "^${ARG1}: replace_with_asmfile" asmfix.txt 2>/dev/null; then
            echo "WARNING: ${ARG1} has an ACTIVE asmfix bridge in asmfix.txt." >&2
            echo "         The bytes you're about to verify are from asm/funcs/${ARG1}.s," >&2
            echo "         NOT from your C body. To verify the C body produces matching" >&2
            echo "         bytes on its own, use \`dc.sh verify-c ${ARG1}\` instead." >&2
            echo >&2
        fi
        # Forward ALL args (not just $ARG1) so flags like `--force` reach
        # regfix_verify.py. Previously `dc.sh verify --all --force` silently
        # dropped `--force` and ran the SHA1 short-circuit path, hiding
        # cascading per-function regressions until they accumulated enough
        # to break a downstream commit. See feedback_cheat_cleanup_techniques.md.
        python3 tools/regfix_verify.py "$@" 2>&1
        ;;

    verify-c)
        # Bridge-aware verify: refuses if the function is bridged
        # (because then the "match" is meaningless — asmfix.py is
        # producing the bytes, not the C body). Use this during
        # retirement work to confirm the C body actually compiles to
        # matching bytes on its own.
        FUNC_NAME="${1:-}"
        if [ -z "$FUNC_NAME" ] || [ "$FUNC_NAME" = "--help" ]; then
            echo "Usage: dc.sh verify-c <func_name>"
            echo
            echo "  Verifies that the C body for <func> compiles to byte-matching"
            echo "  bytes WITHOUT an active asmfix bridge masking it. If the bridge"
            echo "  is still active, this command refuses — comment it out via"
            echo "  \`dc.sh retire $func\` first."
            echo
            echo "  Use this during retirement work. \`dc.sh verify <func>\` is"
            echo "  bridge-blind and will falsely report MATCH for bridged functions."
            exit 1
        fi
        if grep -qE "^${FUNC_NAME}: replace_with_asmfile" asmfix.txt 2>/dev/null; then
            echo "REFUSED: ${FUNC_NAME} has an ACTIVE asmfix bridge in asmfix.txt:" >&2
            grep -nE "^${FUNC_NAME}: replace_with_asmfile" asmfix.txt >&2
            echo >&2
            echo "Any verify against the linked binary at this address reads the" >&2
            echo "asm/funcs/${FUNC_NAME}.s content (i.e., the original game bytes)," >&2
            echo "NOT your C body's compiled output. The C body is DEAD CODE." >&2
            echo >&2
            echo "To verify the C body honestly:" >&2
            echo "  bash tools/dc.sh retire ${FUNC_NAME}   # comment out the bridge" >&2
            echo "  bash tools/dc.sh verify-c ${FUNC_NAME}" >&2
            exit 1
        fi
        # Stale-build guard: if asmfix.txt is newer than build/bb2.exe, the
        # binary on disk still reflects the *previous* asmfix state (e.g.,
        # the bridge bytes), and any verify will be a lie. This catches the
        # `retire` → `verify-c` → false-MATCH trap.
        if [ -f build/bb2.exe ]; then
            ASMFIX_MTIME=$(stat -c %Y asmfix.txt 2>/dev/null || stat -f %m asmfix.txt 2>/dev/null || echo 0)
            BIN_MTIME=$(stat -c %Y build/bb2.exe 2>/dev/null || stat -f %m build/bb2.exe 2>/dev/null || echo 0)
            if [ "$ASMFIX_MTIME" -gt "$BIN_MTIME" ] 2>/dev/null; then
                echo "REFUSED: asmfix.txt was modified after the last build (build/bb2.exe is stale)." >&2
                echo "Any verify against the on-disk binary now would reflect the PREVIOUS asmfix" >&2
                echo "state (e.g., the bridge bytes) — guaranteeing a false MATCH." >&2
                echo >&2
                echo "Rebuild first, then re-run verify-c:" >&2
                echo "  bash tools/dc.sh build-active ${FUNC_NAME}    # incremental rebuild + verify-c" >&2
                echo "  bash tools/dc.sh verify-c ${FUNC_NAME}        # standalone re-verify" >&2
                exit 1
            fi
        fi
        echo "[verify-c] No active bridge for ${FUNC_NAME}; running normal verify..."
        python3 tools/regfix_verify.py "$FUNC_NAME" 2>&1
        ;;

    purge-retirements)
        # Manually purge `# RETIRE: <func>: replace_with_asmfile ...`
        # lines from asmfix.txt. The auto-purge that used to live in
        # refresh-queue was removed because it couldn't tell whether a
        # SHA1 match was "C body produces correct bytes" or "bridge is
        # still producing the bytes from a non-comment source." This
        # tool is the safe, deliberate replacement.
        #
        # For each `# RETIRE: <func>: ...` line, the safe purge proves
        # the C body matches WITHOUT the bridge by:
        #   1. Verifying the line is commented (no active bridge for <func>)
        #   2. Running `dc.sh verify-c <func>` (which refuses if any
        #      active bridge exists) — must report MATCH
        # If both pass, the line is removed. Otherwise, it stays.
        if [ "$1" = "--help" ]; then
            echo "Usage: dc.sh purge-retirements [--dry-run]"
            echo
            echo "  Manually purge \`# RETIRE: <func>: replace_with_asmfile\`"
            echo "  lines from asmfix.txt, ONLY for functions that pass"
            echo "  bridge-aware verification (\`dc.sh verify-c\`)."
            echo
            echo "  --dry-run: report what would be purged, don't write."
            exit 1
        fi
        DRY_RUN=0
        for a in "$@"; do
            case "$a" in --dry-run) DRY_RUN=1 ;; esac
        done
        python3 - "$DRY_RUN" <<'PYEOF'
import re, subprocess, sys
from pathlib import Path

dry_run = sys.argv[1] == "1"
p = Path("asmfix.txt")
lines = p.read_text(encoding="utf-8").splitlines()
candidates = []
for i, line in enumerate(lines):
    m = re.match(r"^# RETIRE:\s*(\w+)\s*:\s*replace_with_asmfile\b", line)
    if m:
        candidates.append((i, m.group(1)))

if not candidates:
    print("No `# RETIRE: ` lines in asmfix.txt.")
    sys.exit(0)

print(f"Found {len(candidates)} `# RETIRE: ` line(s). Verifying each...")
purgable = set()
for idx, fname in candidates:
    # Safety: ensure NO active bridge for this function exists
    active_bridge = any(
        re.match(rf"^{re.escape(fname)}:\s*replace_with_asmfile", L)
        for L in lines
    )
    if active_bridge:
        print(f"  KEEP  {fname}: has an active bridge line too — refusing to purge")
        continue
    # Run verify-c
    res = subprocess.run(
        ["bash", "tools/dc.sh", "verify-c", fname],
        capture_output=True, text=True, timeout=120,
    )
    if ": MATCH " in res.stdout:
        print(f"  PURGE {fname}: verify-c says MATCH (C body produces correct bytes)")
        purgable.add(idx)
        # Post-match cleanup: clear iter_log so the next retirement of
        # this function (rare, but possible) starts with a fresh log.
        try:
            log = Path(".bb2_iter_log") / f"{fname}.jsonl"
            if log.is_file():
                log.unlink()
        except Exception:
            pass
    else:
        print(f"  KEEP  {fname}: verify-c did not report MATCH")
        if res.stdout.strip():
            for ln in res.stdout.splitlines()[:3]:
                print(f"           {ln}")

if not purgable:
    print(f"\nNothing to purge.")
    sys.exit(0)

if dry_run:
    print(f"\n(dry-run) {len(purgable)} line(s) would be purged.")
    sys.exit(0)

out = [L for i, L in enumerate(lines) if i not in purgable]
p.write_text("\n".join(out) + "\n", encoding="utf-8")
print(f"\nPurged {len(purgable)} line(s) from asmfix.txt.")
PYEOF
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
        [ -z "$1" ] && { echo "Usage: dc.sh add-regfix <op> <func> <args...>   (op FIRST; run 'python3 tools/add_regfix.py --help' for ops)"; exit 1; }
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

    scan-canonical)
        # Unified canonical-asm classifier. With no args, scans every
        # function and reports Tier 1/2 hits plus registry drift. Pass
        # --single <func> for a one-function report, --diff to also
        # write proposed registry appends to tmp/canonical_asm_diff.txt.
        python3 tools/scan_canonical_asm.py "$@" 2>&1
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

    cheats)
        # Wrapper for tools/audit_asm_cheats.py. Surfaces existing
        # asm-cheat patterns (large splice rules in regfix.txt, inline
        # __asm__ glabel function bodies in src/*.c). The hook blocks
        # NEW cheats; this command shows existing ones for cleanup.
        #
        #   dc.sh cheats              # full report
        #   dc.sh cheats <funcname>   # check one function
        #   dc.sh cheats --summary    # one-line count (also runs in dc.sh start)
        #   dc.sh cheats --list       # one function name per line
        if [ "$#" -eq 0 ]; then
            python3 tools/audit_asm_cheats.py --all
        else
            case "$1" in
                --summary) python3 tools/audit_asm_cheats.py --summary ;;
                --list)    python3 tools/audit_asm_cheats.py --list-funcs ;;
                --*)       python3 tools/audit_asm_cheats.py "$1" ;;
                *)         python3 tools/audit_asm_cheats.py --func "$1" ;;
            esac
        fi
        ;;

    next|next-structural|next-asmfix|next-cheat|next-cheat-cleanup|next-for-file)
        # Print the next function from one WORK_QUEUE.md queue and SET it as the
        # active function in .bb2_active_func. The PreToolUse hook will
        # then block subsequent `git commit` (until verify passes) and queue
        # pulls until the active function is finished.
        #
        # Forms (all five queues support the same flags):
        #   dc.sh next                          # top 1, set active, no auto-brief
        #   dc.sh next 5                        # preview top 5 (no active change)
        #   dc.sh next --with-context           # top 1 + auto-run agent-brief
        #   dc.sh next-structural [--with-context] [N]
        #   dc.sh next-asmfix     [--with-context] [N]
        #   dc.sh next-cheat      [--with-context] [N]  # like `next` but filtered
        #                                                # to inline_asm_debt entries
        #   dc.sh next-for-file <files> [--with-context] [N]
        #       # Active-decomp queue filtered to entries whose src column
        #       # contains any of the comma-separated <files>. Used by
        #       # parallel sub-agent workers so two workers in separate
        #       # worktrees can pull from disjoint file partitions without
        #       # colliding on the same .c file's label numbering.
        TAG_FILTER=""
        FILE_FILTER=""
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
            next-cheat)
                # Cheat-fix work shares the active queue, but filters to any
                # *_debt tag: inline_asm_debt (file-scope cheats),
                # c_body_asm_debt (C-body multi-insn smuggling cheats),
                # regfix_overwrite_debt (wildcard-subst force-rewrite cheats).
                SECTION="## Queue (top = next)"
                QUEUE_NAME="active decomp queue (filtered: *_debt tags)"
                TAG_FILTER="_debt"
                ;;
            next-cheat-cleanup)
                # Cheat-cleanup queue: every audit-flagged cheat function,
                # sorted by retirement difficulty. SHA1 matches today only
                # because of a regfix/asmfix/inline-__asm__ cheat; work is
                # to remove the cheat and coerce the C source to emit
                # equivalent codegen naturally. Funcs that ALSO appear in
                # the active/asmfix/structural queue are still listed here
                # (single source of truth for the cheat set) and carry an
                # `also:<queue>` tag indicating the overlap.
                SECTION="## Cheat Cleanup Queue (top = next-cheat-cleanup)"
                QUEUE_NAME="cheat cleanup queue"
                ;;
            next-for-file)
                if [ -z "${1:-}" ] || case "$1" in --*|''|*[!a-zA-Z0-9_.,-]*) true ;; *) false ;; esac; then
                    echo "ERROR: next-for-file requires a file pattern as the first argument." >&2
                    echo "Usage: bash tools/dc.sh next-for-file <files> [--with-context] [N]" >&2
                    echo "  <files> is a comma-separated list of substrings matched against the src column." >&2
                    echo "  Examples:" >&2
                    echo "    bash tools/dc.sh next-for-file text1b.c" >&2
                    echo "    bash tools/dc.sh next-for-file display.c,main.c,sound.c" >&2
                    echo "    bash tools/dc.sh next-for-file text1b.c 5    # preview top 5" >&2
                    exit 1
                fi
                FILE_FILTER="$1"
                shift
                SECTION="## Queue (top = next)"
                QUEUE_NAME="active decomp queue (filtered: src matches '$FILE_FILTER')"
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
        # a remote teammate already matched. Skipped on no-network or
        # no-remote setups; failure is non-fatal (warn and continue).
        # Pass --no-pull to skip explicitly.
        if [ "$NO_PULL" != "1" ] && git rev-parse --git-dir >/dev/null 2>&1; then
            if git remote get-url origin >/dev/null 2>&1; then
                # Refresh the index first so stat-only differences (mtime/
                # ctime) don't fire a false "uncommitted changes" warning
                # on a content-clean tree. Then use the porcelain status
                # which only reports real content/staging differences.
                git update-index --refresh >/dev/null 2>&1 || true
                if [ -n "$(git status --porcelain 2>/dev/null)" ]; then
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
        # When TAG_FILTER is set (next-cheat), only count rows whose
        # bracketed tag column contains the filter string.
        # When FILE_FILTER is set (next-for-file), only count rows whose
        # src column matches any of the comma-separated substrings.
        TOP=$(awk -v n="$N" -v section="$SECTION" -v tag="$TAG_FILTER" -v files="$FILE_FILTER" '
            BEGIN {
                if (files != "") {
                    nf = split(files, file_list, ",")
                } else { nf = 0 }
            }
            $0 == section { in_queue=1; next }
            in_queue && /^## / { in_queue=0 }
            in_queue && /^```$/ { in_block = !in_block; next }
            in_queue && in_block && /^[[:space:]]*[0-9]+[[:space:]]/ {
                if (tag != "" && index($0, tag) == 0) next
                if (nf > 0) {
                    matched = 0
                    for (i = 1; i <= nf; i++) {
                        if (file_list[i] != "" && index($0, file_list[i]) > 0) {
                            matched = 1; break
                        }
                    }
                    if (!matched) next
                }
                print
                count++
                if (count >= n) exit
            }
        ' WORK_QUEUE.md)
        if [ -z "$TOP" ]; then
            if [ -n "$FILE_FILTER" ]; then
                echo "No entries in $QUEUE_NAME (no rows match files '$FILE_FILTER')." >&2
                echo "Check that those .c files have entries in WORK_QUEUE.md." >&2
            elif [ -n "$TAG_FILTER" ]; then
                echo "No entries in $QUEUE_NAME (no rows tagged '$TAG_FILTER')." >&2
                echo "Run 'python3 tools/audit_asm_cheats.py --summary' to verify there are unauthorized cheats." >&2
            else
                echo "No entries found in $QUEUE_NAME. Run 'dc.sh refresh-queue' if this looks stale." >&2
            fi
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

    preflight)
        # Single focused brief before starting work on a function:
        # bridge state, source location, classification, size, blockers,
        # kengo equivalent, existing regfix/asmfix rule counts, sibling
        # commits to clone, one-line recommendation. Replaces 3-4
        # separate tool calls.
        python3 tools/preflight.py "$@" 2>&1
        ;;

    diff-summary)
        # One-line-per-category diff verdict. Reads regfix_verify
        # output and classifies each differing instruction by failure
        # mode (opcode-only, register-rename, immediate, branch-offset,
        # structural), then prints a compact summary with a suggested
        # next action. Faster decision-making during iteration than
        # reading the full side-by-side diff.
        python3 tools/diff_summary.py "$@" 2>&1
        ;;

    suggest)
        # Lean per-function rollup: state + top 3 techniques + don't-tries
        # + next single command, plus (post-build) diff routing and recipe
        # candidates. Cheap enough to run on every function, short enough
        # to actually be read. Complements agent-brief (the full dump) by
        # serving as the orientation pass before drilling in.
        python3 tools/suggest_func.py "$@" 2>&1
        ;;

    build-active)
        # Incremental rebuild for one function: nuke the .o for the
        # containing .c file + link products, run make (~30s vs ~2 min
        # for full clean), then run bridge-aware verify-c. Use during
        # the edit-iterate loop. The full clean-rebuild gate at commit
        # time (active_func_guard hook) still protects against cache
        # lag — this is just for tight iteration.
        # Also appends to .bb2_iter_log/<func>.jsonl and surfaces the
        # trajectory + plateau advice (see iter_log.py for the rule).
        python3 tools/build_active.py "$@" 2>&1
        ;;

    iter-log)
        # Per-function build trajectory: shows recent rounds + plateau
        # advice. Auto-fed by `build-active`. Use to check if you're
        # stuck on the same diff count and should switch technique.
        python3 tools/iter_log.py "$@"
        ;;

    regfix-drift-immune)
        # Audit regfix.txt for `subst` rules with hardcoded `.L<N>` label
        # numbers in their PATTERN and rewrite to `\.L\d+`. Only rewrites
        # cases where the REPLACEMENT label is project-custom (e.g.,
        # `.LbodyX`); skips paired-shift cases (replacement is itself a
        # GCC-emitted `.L<N>`). See tools/regfix_drift_immune.py.
        python3 tools/regfix_drift_immune.py "$@" 2>&1
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

    auto-repair)
        # Build wrapper that detects cascade-drift symptoms (asmfix
        # `did not match` warnings, doubly-defined `.LN` labels, regfix
        # subst no-match) and auto-runs fix-asmfix-drift + fix-label-drift
        # then rebuilds. Used by `dc.sh build-active` by default; can also
        # be called directly to retry a stuck build. See
        # memory/feedback_auto_drift_repair.md for full behaviour.
        python3 tools/auto_drift_repair.py "$@" 2>&1
        ;;

    preflight-cascade)
        # Pre-integration impact report (read-only). Lists how many sibling
        # rules in the function's .c file use literal `.LN` labels — those
        # are the rules that can silently break when the integration shifts
        # GCC's file-wide label counter. Auto-repair (in build-active)
        # handles drift after the fact; this just sets expectations.
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh preflight-cascade <func> [--verbose]"; exit 1; }
        python3 tools/preflight_cascade.py "$FUNC_NAME" "$@" 2>&1
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

    side-by-side)
        # Shape-aligned binary diff with per-row category (MATCH / REG-RENAME
        # / BRANCH-OFFSET / STRUCTURAL). Cascade-immune by construction —
        # difflib aligns on opcode+register-blind shape, so a 1-instruction
        # length difference does NOT cause all subsequent rows to show as
        # different.
        #
        # Use this when verify-c reports a high diff count and you want to
        # know which are real structural changes vs pure register renames vs
        # branch-offset drift (which routes to fix-branch-drift).
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh side-by-side <func> [--all] [--no-color]"; exit 1; }
        python3 tools/binary_diff.py side-by-side "$FUNC_NAME" "$@" 2>&1
        ;;

    gen-substs)
        # Auto-generate regfix `subst` rules for REG-RENAME diff rows from
        # the shape-aligned binary diff. Output goes to stdout by default;
        # pass --apply to append directly to regfix.txt.
        #
        # This is the high-leverage tool from the exec_game retrospective:
        # what was 4 hours of manual subst-writing collapses to one command.
        # Always re-run after every build that closes structural gaps —
        # the only register renames it can emit are the ones at MATCHED
        # shape positions, which means structural fixes must come first.
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh gen-substs <func> [--apply]"; exit 1; }
        python3 tools/binary_diff.py gen-substs "$FUNC_NAME" "$@" 2>&1
        ;;

    binary-diff-count)
        # JSON: {structural, rename, branch_offset, total} for one function.
        # Used by build_active.py to drive cascade-immune REGRESSION
        # detection in iter_log.
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh binary-diff-count <func>"; exit 1; }
        python3 tools/binary_diff.py count "$FUNC_NAME" 2>&1
        ;;

    branch-offsets)
        # JSON dump of BRANCH-OFFSET rows — branch instructions whose
        # opcode+registers match but whose immediate differs. Consumed by
        # fix-branch-drift to emit .word-encoded subst rules.
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh branch-offsets <func>"; exit 1; }
        python3 tools/binary_diff.py branch-offsets "$FUNC_NAME" 2>&1
        ;;

    fix-branch-drift)
        # Auto-emit .word-encoded subst rules for BRANCH-OFFSET diffs —
        # the residual end-game when all STRUCTURAL/REG-RENAME diffs are
        # gone but a few branch instructions still differ in immediate only
        # (label drift that survives gen-substs). The .word escape hatch
        # forces the assembler to emit the exact target bytes, bypassing
        # the label resolution. From the exec_game match (commit 10ee50c).
        FUNC_NAME="$1"
        shift || true
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh fix-branch-drift <func> [--apply]"; exit 1; }
        python3 tools/word_branches.py "$FUNC_NAME" "$@" 2>&1
        ;;

    whereami)
        # Worker self-verification: print the current working tree, branch,
        # whether it's a sub-agent worktree, and the active marker. Designed
        # to be the first command a parallel sub-agent runs to confirm it
        # is in its own worktree and not the main repo.
        GIT_ROOT=$(git rev-parse --show-toplevel 2>/dev/null)
        if [ -z "$GIT_ROOT" ]; then
            echo "ERROR: not inside a git working tree." >&2
            exit 1
        fi
        BRANCH=$(git branch --show-current 2>/dev/null)
        [ -z "$BRANCH" ] && BRANCH="(detached HEAD)"
        case "$GIT_ROOT" in
            *.claude/worktrees/agent-*) TREE_KIND="sub-agent worktree (parallel-safe)" ;;
            *) TREE_KIND="main repo (NOT a worktree -- sub-agents should NEVER be here)" ;;
        esac
        ACTIVE="(none)"
        if [ -s "$GIT_ROOT/.bb2_active_func" ]; then
            ACTIVE=$(tr -d '[:space:]' < "$GIT_ROOT/.bb2_active_func")
        fi
        echo "Working tree: $GIT_ROOT"
        echo "Branch:       $BRANCH"
        echo "Tree kind:    $TREE_KIND"
        echo "Active func:  $ACTIVE"
        echo
        echo "(If you are a parallel sub-agent and 'Tree kind' shows 'main repo',"
        echo " STOP -- you have escaped your worktree. Report to user immediately.)"
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
        # matches when only one function changed and the CSV doesn't
        # need a full re-scan. Phases [1] capture-recipe, [2] strip
        # orphans, [3] regen WORK_QUEUE.md from the existing CSV.
        # Full mode adds phases for batch_attempt + classify_asmfix.
        #
        # CHANGED 2026-05-12: refresh-queue no longer auto-purges
        # `# RETIRE: ` lines. The old auto-purge tried to detect "the
        # retirement succeeded" by checking SHA1 match, but a SHA1
        # match doesn't prove the C body produces the correct bytes —
        # the asmfix bridge could still be silently producing them
        # (this was the func_8007B3A8 trap, commit 836d9a1). Now
        # `# RETIRE: <func>: ` lines are kept as permanent historical
        # markers. Run `dc.sh purge-retirements` explicitly to clean
        # them up after manually confirming each function retires correctly.
        if [ "$FAST_MODE" = "1" ]; then
            echo "[1/4] (FAST) Capturing recipe from latest commit..."
            python3 tools/capture_recipe.py HEAD 2>&1 | tail -5 || true
            echo
            echo "[2/4] (FAST) Auto-purging verified \`# RETIRE: \` lines..."
            # Safe to auto-purge: purge-retirements runs verify-c per function,
            # which refuses if any bridge is still active. Only lines whose C
            # body genuinely produces matching bytes (without bridge masking)
            # are removed. See feedback_retirement_recipes.md gotcha #7.
            bash "$0" purge-retirements 2>&1 | tail -5 || true
            echo
            echo "[3/4] (FAST) Stripping orphan named_syms.txt assignments..."
            python3 tools/audit_named_syms_orphans.py --apply 2>&1 | tail -3 || true
            echo
            echo "[4/4] (FAST) Regenerating WORK_QUEUE.md from existing CSV..."
            python3 tools/gen_work_queue.py 2>&1 | tail -5
            echo
            echo "[lint] Auditing regfix.txt for drift-fragile literal-.L<N> rules..."
            python3 tools/regfix_drift_immune.py 2>&1 | tail -10 || true
            : > .bb2_active_func 2>/dev/null || true
            exit 0
        fi

        echo "[1/7] Capturing recipe from latest commit (if it's a match)..."
        python3 tools/capture_recipe.py HEAD 2>&1 | tail -5 || true
        echo
        echo "[2/7] Auto-purging verified \`# RETIRE: \` lines from asmfix.txt..."
        # Safe to auto-purge: purge-retirements runs verify-c per function,
        # which refuses if any bridge is still active. Only lines whose C
        # body genuinely produces matching bytes (without bridge masking)
        # are removed — this avoids the historical SHA1-only-trap (commit
        # 836d9a1, func_8007B3A8) where the bridge was silently masking
        # a broken C body. See feedback_retirement_recipes.md gotcha #7.
        bash "$0" purge-retirements 2>&1 | tail -10 || true
        echo
        echo "[3/7] Stripping orphan named_syms.txt assignments..."
        # Orphans are `<name> = 0x<addr>;` entries that shadow a real
        # text-section symbol now defined by a C .o (typically because
        # the function got decompiled but its old asm-link assignment
        # was left behind). The shadow form breaks `dc.sh verify <name>`
        # symbol lookups in the linker map.
        python3 tools/audit_named_syms_orphans.py --apply 2>&1 | tail -3 || true
        echo
        echo "[4/7] Classifying inline_asm + INCLUDE_ASM (batch_attempt --classify-only)..."
        python3 tools/batch_attempt.py --classify-only 2>&1 | tail -3
        echo
        echo "[5/7] Classifying replace_with_asmfile entries..."
        python3 tools/classify_asmfix.py 2>&1 | tail -3
        echo
        echo "[6/7] Reporting any \`# RETIRE: \` lines that didn't pass auto-purge..."
        RETIRE_COUNT=$(grep -c "^# RETIRE: " asmfix.txt 2>/dev/null || echo 0)
        if [ "$RETIRE_COUNT" -gt 0 ] 2>/dev/null; then
            echo "  $RETIRE_COUNT line(s) remain (auto-purge couldn't verify these — usually means the C body still doesn't compile-match):"
            grep -nE "^# RETIRE: " asmfix.txt | head -10 | sed 's/^/    /'
            echo "  Investigate manually or finish the retirement before next refresh."
        else
            echo "  None — asmfix.txt is clean."
        fi
        echo
        echo "[7/7] Generating WORK_QUEUE.md..."
        python3 tools/gen_work_queue.py 2>&1
        echo
        echo "[lint] Auditing regfix.txt for drift-fragile literal-.L<N> rules..."
        # A literal `.L<N>` in a subst pattern silently no-ops if a sibling
        # function's C body shifts GCC's file-wide label numbering. This
        # audit (dry-run) flags them; `dc.sh regfix-drift-immune --apply`
        # rewrites the safe ones. See the func_8007CE0C retrospective.
        python3 tools/regfix_drift_immune.py 2>&1 | tail -12 || true
        # Belt-and-suspenders: the commit-via-hook path already cleared
        # the marker, but if for some reason a refresh runs while it's
        # set, we know the queue is being regenerated and the active
        # function is no longer relevant.
        : > .bb2_active_func 2>/dev/null || true
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

    queue-structural)
        bash tools/dc.sh next-structural "${1:-10}" --no-pull
        ;;

    queue-asmfix)
        bash tools/dc.sh next-asmfix "${1:-10}" --no-pull
        ;;

    classify-batch)
        # Classify N functions and flag stale (already pure C) entries
        # so you can skip no-op queue items.
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

    audit-bridges)
        # Audit caller signatures vs stub signatures for all bridged
        # (replace_with_asmfile) functions. Reports ACTIVE / latent /
        # informational issues. JSON written to tmp/bridge_signature_audit.json.
        # See feedback_bridge_signature_audit.md.
        mkdir -p tmp
        python3 tools/audit_bridge_signatures.py "$@"
        ;;

    memory-check)
        # §2.5 of the decomp-next skill: surface project memory + prior-work
        # context for one function BEFORE launching the §3 pipeline.
        # Reports CANONICAL_ASM status (from inline_asm_canonical.txt),
        # direct memory mentions of the function, sibling memory hits
        # (functions adjacent in the same .c file), and the data-driven
        # hand-coded-asm signal tier (STRONG / POSSIBLE / TIGHT_C / LOW).
        # Closes the two failure modes the user flagged: agents
        # re-deriving documented gotchas, and agents pure-C-attacking
        # functions that were canonically asm.
        FUNC_NAME="$1"
        [ -z "$FUNC_NAME" ] && { echo "Usage: dc.sh memory-check <func> [--quiet]"; exit 1; }
        shift
        python3 tools/memory_check.py "$FUNC_NAME" "$@"
        ;;

    scan-hand-coded)
        # Bulk hand-coded-asm scanner over asm/funcs/*.s. Reports STRONG /
        # POSSIBLE / TIGHT_C tiers based on the 5 signals from
        # feedback_hand_coded_asm_recognition.md. The S1 (uniform multu
        # pacing) and S2 (empty-body branch) signals are the
        # GCC-impossible discriminators; functions with neither are not
        # hand-coded regardless of how many tightness signals fire.
        # Flags: --single <func>, --all, --json, --min-score N
        python3 tools/scan_hand_coded.py "$@"
        ;;

    *)
        echo "Unknown command: $CMD"
        echo "Commands: compile, score, debug, build, replace, setup,"
        echo "          inline-locate, inline-verify, inline-setup, inline-replace,"
        echo "          smart, permute, add-regfix, classify, gte, attempt,"
        echo "          recipes, apply-recipe, memory-check, scan-hand-coded,"
        echo "          analysis, dump-text, validate-regfix, gen-regfix, verify,"
        echo "          frame-shift, asmfix-slice, find-label-at, diagnose-hoist,"
        echo "          next, next-structural, next-asmfix, next-cheat, next-for-file, refresh-queue, release,"
        echo "          retire, audit-bridges,"
        echo "          side-by-side, gen-substs, binary-diff-count, branch-offsets,"
        echo "          fix-branch-drift"
        exit 1
        ;;
esac
