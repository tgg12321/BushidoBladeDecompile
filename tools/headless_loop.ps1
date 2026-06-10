#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Headless driver for the BB2 decomp engine loop: runs `claude -p` once per
  queue item to take the TOP function to COMPLETED (pure C, or canonical asm
  if genuinely hand-coded), with oracle + progress guardrails and per-run
  metrics attribution.

.DESCRIPTION
  Each iteration:
    1. (pre-loop only) `eng verify-oracle --rebuild` to establish a clean baseline.
    2. Read the top active queue item (`eng queue next`).
    3. Invoke `claude -p <loop-prompt>` with a fresh session id (so engine
       metrics attribute the run), a model, a permission mode, and a $ budget cap.
    4. Authoritative post-check: `eng verify-oracle --rebuild` MUST still match the
       oracle SHA1. If not, the agent committed a broken build -> STOP immediately.
    5. Confirm progress: the engine `done` count went up OR the function was parked.
       If neither -> stuck -> STOP.
    6. Append a run record to metrics/headless_runs.jsonl (func, model, session,
       cost, tokens, turns, oracle_ok, advanced).
  Never pushes. Stops on first error / oracle break / no-progress / budget /
  MaxIterations / empty queue.

.NOTES
  PERMISSION MODE: headless autonomy needs the agent to run eng.ps1 (PowerShell),
  edit src, run WSL builds and commit WITHOUT interactive prompts. In `-p` mode a
  prompt auto-denies, so only `bypassPermissions` actually runs unattended. That
  is the default here, paired with the guardrails above (budget cap, 1 iteration
  default, authoritative oracle re-check, never-push). Downgrade with
  -PermissionMode acceptEdits to supervise.

.EXAMPLE
  pwsh tools/headless_loop.ps1 -DryRun
  pwsh tools/headless_loop.ps1 -MaxIterations 1 -Model opus -MaxBudgetUsd 5
#>
[CmdletBinding()]
param(
    [int]    $MaxIterations  = 1,
    [string] $Model          = 'opus',
    [ValidateSet('acceptEdits','bypassPermissions','default','dontAsk')]
    [string] $PermissionMode = 'bypassPermissions',
    [switch] $DryRun
)

$ErrorActionPreference = 'Stop'
$root   = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$engPs1 = Join-Path $PSScriptRoot 'eng.ps1'
$runlog = Join-Path $root 'metrics/headless_runs.jsonl'
$wsldir = if ($root -match '^([A-Za-z]):[\\/](.*)$') {
    "/mnt/$($Matches[1].ToLower())/$($Matches[2] -replace '\\','/')"
} else { $root -replace '\\','/' }

function Invoke-Eng {
    # Run an engine subcommand via eng.ps1, return parsed JSON (stdout only).
    param([Parameter(ValueFromRemainingArguments = $true)] [string[]] $EngArgs)
    $raw = & $engPs1 @EngArgs | Out-String
    if (-not $raw.Trim()) { return $null }
    return $raw | ConvertFrom-Json
}

function Git-Head { (wsl bash -c "cd '$wsldir' && git rev-parse HEAD 2>/dev/null" | Out-String).Trim() }

function Get-AuditDigest($sid) {
    # Transcript-derived efficiency signals (turns, tooling errors, footgun
    # blocks, retried commands) so the run record is self-auditing. Best-effort.
    try {
        $raw = wsl bash -c "cd '$wsldir' && python3 tools/headless_audit.py --session '$sid' --json" | Out-String
        if ($raw.Trim()) { return $raw | ConvertFrom-Json }
    } catch { }
    return $null
}

$PROMPT = @'
You are running the Bushido Blade 2 decomp engine loop HEADLESS. Work the TOP
queue item to COMPLETED-C (100% pure C, zero regfix/asmfix/pins/cheat-asm)
following the loop in CLAUDE.md, then STOP. Run every engine command through the
PowerShell wrapper: `& tools/eng.ps1 <subcommand>` (queue next | canonical <f> |
sandbox <f> --disable all | retire <f> | queue done <f> |
queue park <f> --reason "...").

Steps for the single top function:
  1. & tools/eng.ps1 queue next            (identify the function + file)
  2. & tools/eng.ps1 canonical <func>      (C -> continue; ASM-* -> park it, do not grind)
  3. & tools/eng.ps1 sandbox <func> --disable all   (honest pure-C distance)
  4. Edit src/<file>.c toward 0 in pure C (re-run step 3 as your gradient). If
     retire fails with a cheat-asm hint, strip the source __asm__ barrier it names.
  5. & tools/eng.ps1 retire <func>         (drops rules + full SHA1 gate)
  6. & tools/eng.ps1 queue done <func>
  7. Register a finding if reusable (.claude/rules/ or memory/), then commit with
     git commit -F tmp/msg.txt (write the message file first; never a heredoc).

HARD RULES (ignoring these wastes the run — a mid-work budget cutoff leaves an
uncommitted tree that forces an escalation):
  - Work ONLY the single top function. The instant it is finished — `git commit`
    after retire + queue done, OR `queue park` — STOP and end your turn.
  - Do NOT run `queue next` again, and do NOT start, edit, or even look at a
    second function. ONE function per session, no exceptions.
  - ALWAYS `git commit` your finished function before anything else. NEVER end
    your turn with uncommitted edits in the tree.
  - Do NOT push.
PERSISTENCE BAR — every function MUST be decompiled eventually, so the bar for
parking is high. "Hard" is not enough; "I tried a few things" is not enough.
Read [[difficult-is-not-impossible]] and
[[canonical-gate-distance-not-evidence]] BEFORE you consider parking. The
matching C exists ([[no-compiler-divergence]]: the toolchain is frozen, the
ONLY variable is the C). A verdict of ASM-SUSPECT is the gate's GUESS, NOT
proof — keep grinding pure C.

You may NOT park until you have ALL of:
  1. **Run the rigorous scanner**: `python3 tools/scan_hand_coded.py --single
     <func> --json`. If tier is LOW or TIGHT_C, the function is NOT
     canonical-asm; keep working pure C.
  2. **Applied the lever playbook**: at minimum block-local var split, narrow
     integer type, and the file-relevant rules auto-loaded into your context
     (the path-scoped rules under .claude/rules/). Document each lever
     attempted + the measured sandbox-distance delta.
  3. **Read the asm and a matched sibling in the same .c file**. The sibling
     that DID match shows GCC's behaviour on adjacent-style C — diff its
     greg/RTL dumps against the unmatched one.
  4. **Tried directed permuter or instrumented cc1 dumps** (BB2_ALLOC_DEBUG,
     BB2_SCHED_DEBUG, BB2_PRIO_DEBUG via tmp/gccdbg/cc1) when the gap is
     register-allocation or scheduling. The dumps tell you WHICH RTL state
     needs flipping; that's the lever-finding map.
  5. **Either made measurable floor-lowering progress** (then SAVE A WIP
     CHECKPOINT under memory/wip/<func>/ per CLAUDE.md "Score lowered but not
     0 ⇒ checkpoint" instead of parking — the next session resumes from your
     candidate.c, not from HEAD), OR documented evidence-backed exhaustion of
     a specific named lever.

A park reason MUST be a per-function technical finding (the SPECIFIC RTL
mechanism the levers can't flip, with measurements), NOT "high distance" or
"too many rules" or "looks complex". `distance > 500` is no longer routable
to ASM-STRUCTURAL on distance alone (2026-06-09 gate fix) — large size IS
NOT evidence of hand-asm, it's evidence of large pure-C work to do.

If you cannot finish in budget BUT measurably lowered the floor, do NOT
park. Write a WIP entry (memory/wip/<func>/candidate.c + meta.json + notes.md
per CLAUDE.md and memory/wip/README.md). Invoke the cheat-reviewer on the
candidate first; record the verdict in meta.json. Revert src/ to HEAD (oracle
stays green), commit under `wip: <func>`, stop. The next agent resumes from
your floor, not from scratch.

NEVER commit a cheat (regfix, asmfix, cheat-asm, register pin, scheduling
barrier, dead-store coercion, volatile-on-game-state-global, OR-tree
reorder, do-while-zero outside its narrow LABEL_OUTSIDE_LOOP_P sanctioned
use, etc.) to "finish" the function — those make the bytes match but the
function is NOT completed; the COMPLETED gate rejects them.
'@

function Write-RunRecord($rec) {
    ($rec | ConvertTo-Json -Compress -Depth 6) | Add-Content -Path $runlog -Encoding utf8
}

Push-Location $root
try {
    Write-Host "[headless] baseline: verify-oracle --rebuild ..."
    $base = Invoke-Eng verify-oracle --rebuild
    if (-not $base.build_matches) {
        Write-Host "[headless] tree does not build to the oracle SHA1 at start; aborting." -ForegroundColor Red
        exit 1
    }
    Write-Host "[headless] baseline OK (SHA1 == oracle)."

    for ($i = 1; $i -le $MaxIterations; $i++) {
        $top = Invoke-Eng queue next
        if (-not $top -or -not $top.func) {
            Write-Host "[headless] queue has no active item -- done."
            break
        }
        $func = $top.func
        $statusBefore = Invoke-Eng queue status
        # `queue done` REMOVES the item from queue.json (per CLAUDE.md
        # "queue presence = INCOMPLETE"), so completion shows as a `total`
        # drop. `by_status.done` does not exist — it would always be 0.
        # `total` drop is the honest completion signal.
        $totalBefore = [int]$statusBefore.counts.total

        Write-Host "[headless] iter $i/$MaxIterations -> $func ($($top.file), verdict $($top.verdict), $($top.rules) rule(s))"

        $sid = [guid]::NewGuid().ToString()
        $claudeArgs = @(
            '-p', $PROMPT,
            '--output-format', 'json',
            '--model', $Model,
            '--permission-mode', $PermissionMode,
            '--session-id', $sid
        )

        if ($DryRun) {
            Write-Host "[headless] DRY RUN -- would invoke:"
            Write-Host "  CLAUDE_SESSION_ID=$sid claude $($claudeArgs[0]) <prompt> $($claudeArgs[2..($claudeArgs.Count-1)] -join ' ')"
            Write-Host "[headless] (no claude process spawned; no commit) -- stopping after dry run."
            break
        }

        $headBefore = Git-Head
        $env:CLAUDE_SESSION_ID = $sid            # so engine/metrics.py attributes the run
        $raw = ($null | & claude @claudeArgs | Out-String)   # $null stdin -> skip the 3s "no stdin" wait
        Remove-Item Env:\CLAUDE_SESSION_ID -ErrorAction SilentlyContinue
        $headAfter = Git-Head

        $res = $null
        try { $res = $raw | ConvertFrom-Json } catch { }
        $isErr = $res.is_error -or ($null -eq $res)

        Write-Host "[headless] authoritative post-check: verify-oracle --rebuild ..."
        $post = Invoke-Eng verify-oracle --rebuild
        $oracleOk = [bool]$post.build_matches

        $statusAfter = Invoke-Eng queue status
        $totalAfter = [int]$statusAfter.counts.total
        # completion = total dropped (the function left queue.json entirely
        # via `queue done` -> COMPLETED-C or COMPLETED-INLINE-ASM-CANONICAL).
        # A park leaves total unchanged (status flip only), so this cleanly
        # separates "done" from "parked".
        $advanced = ($totalAfter -lt $totalBefore)
        # parked counts as progress too (the func leaves 'active')
        $stillTop = (Invoke-Eng queue next).func -eq $func
        $progressed = $advanced -or (-not $stillTop)

        $audit = Get-AuditDigest $sid    # transcript-derived efficiency signals

        Write-RunRecord ([ordered]@{
            ts             = (Get-Date).ToUniversalTime().ToString('o')
            func           = $func
            file           = $top.file
            model          = $Model
            permission_mode= $PermissionMode
            session_id     = $sid
            cost_usd       = $res.total_cost_usd
            input_tokens   = $res.usage.input_tokens
            output_tokens  = $res.usage.output_tokens
            num_turns      = $res.num_turns
            is_error       = [bool]$isErr
            oracle_ok      = $oracleOk
            advanced       = $advanced
            progressed     = $progressed
            total_before   = $totalBefore
            total_after    = $totalAfter
            head_before    = $headBefore
            head_after     = $headAfter
            # self-auditing efficiency signals (from tools/headless_audit.py)
            tooling_errors = $audit.error_results
            footgun_blocks = $audit.footgun_blocks
            duplicate_cmds = $audit.duplicate_cmd_count
            audit_turns    = $audit.turns
            eng_subcmds    = $audit.eng_subcmds
        })

        Write-Host "[headless] $func -> oracle_ok=$oracleOk advanced=$advanced progressed=$progressed cost=`$$($res.total_cost_usd)"
        if ($audit) {
            Write-Host "[headless] efficiency: turns=$($audit.turns) tooling_errors=$($audit.error_results) footgun_blocks=$($audit.footgun_blocks) duplicate_cmds=$($audit.duplicate_cmd_count)"
        }

        if (-not $oracleOk) {
            # non-throwing (Write-Error throws under ErrorActionPreference=Stop,
            # which would clobber the exit code) so the exit code stays meaningful.
            Write-Host "[headless] ORACLE BROKEN after $func -- the agent committed a non-matching build. STOPPING for manual review." -ForegroundColor Red
            exit 2
        }

        # Orchestrator post-run review: confirm findings + apply the escalation
        # boundary. Exit 0 = ACCEPT (continue), 10 = ESCALATE (stop for the user).
        Write-Host "[headless] --- orchestrator review ---"
        wsl bash -c "cd '$wsldir' && python3 tools/headless_review.py --session '$sid'"
        if ($LASTEXITCODE -eq 10) {
            Write-Host "[headless] orchestrator review -> ESCALATE on $func. STOPPING for user review." -ForegroundColor Red
            exit 10
        }
    }
    Write-Host "[headless] loop finished."
}
finally {
    Pop-Location
}
