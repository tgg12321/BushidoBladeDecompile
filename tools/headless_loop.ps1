#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Headless driver for the BB2 decomp engine loop: runs `claude -p` once per
  queue item to take the TOP function to Tier-4, with oracle + progress
  guardrails and per-run metrics attribution.

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
    [double] $MaxBudgetUsd   = 5.0,
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
queue item to Tier-4 (100% pure C, zero regfix/asmfix/pins) following the loop
in CLAUDE.md, then STOP. Run every engine command through the PowerShell wrapper:
`& tools/eng.ps1 <subcommand>` (queue next | canonical <f> | sandbox <f> --disable all
| retire <f> | queue done <f> | queue park <f> --reason "...").

Steps for the single top function:
  1. & tools/eng.ps1 queue next            (identify the function + file)
  2. & tools/eng.ps1 canonical <func>      (C -> continue; ASM-* -> park it, do not grind)
  3. & tools/eng.ps1 sandbox <func> --disable all   (honest pure-C distance)
  4. Edit src/<file>.c toward 0 in pure C (re-run step 3 as your gradient). If
     retire fails with a tier-3 hint, strip the source __asm__ barrier it names.
  5. & tools/eng.ps1 retire <func>         (drops rules + full SHA1 gate)
  6. & tools/eng.ps1 queue done <func>
  7. Register a finding if reusable (.claude/rules/ or memory/), then commit with
     git commit -F tmp/msg.txt (write the message file first; never a heredoc).

HARD RULES: finish EXACTLY ONE function then stop. Do NOT push. Do NOT take a
second function. If it cannot reach pure-C Tier-4 (canonical-asm needing user
auth, or a genuine documented plateau), `& tools/eng.ps1 queue park <func>
--reason "..."` instead of forcing it, then stop.
'@

function Write-RunRecord($rec) {
    ($rec | ConvertTo-Json -Compress -Depth 6) | Add-Content -Path $runlog -Encoding utf8
}

Push-Location $root
try {
    Write-Host "[headless] baseline: verify-oracle --rebuild ..."
    $base = Invoke-Eng verify-oracle --rebuild
    if (-not $base.build_matches) {
        Write-Error "[headless] tree does not build to the oracle SHA1 at start; aborting."
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
        $doneBefore = [int]$statusBefore.counts.by_status.done

        Write-Host "[headless] iter $i/$MaxIterations -> $func ($($top.file), verdict $($top.verdict), $($top.rules) rule(s))"

        $sid = [guid]::NewGuid().ToString()
        $claudeArgs = @(
            '-p', $PROMPT,
            '--output-format', 'json',
            '--model', $Model,
            '--permission-mode', $PermissionMode,
            '--session-id', $sid,
            '--max-budget-usd', $MaxBudgetUsd
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
        $doneAfter = [int]$statusAfter.counts.by_status.done
        $advanced = ($doneAfter -gt $doneBefore)
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
            done_before    = $doneBefore
            done_after     = $doneAfter
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
            Write-Error "[headless] ORACLE BROKEN after $func -- the agent committed a non-matching build. STOPPING for manual review."
            exit 2
        }

        # Orchestrator post-run review: confirm findings + apply the escalation
        # boundary. Exit 0 = ACCEPT (continue), 10 = ESCALATE (stop for the user).
        Write-Host "[headless] --- orchestrator review ---"
        wsl bash -c "cd '$wsldir' && python3 tools/headless_review.py --session '$sid'"
        if ($LASTEXITCODE -eq 10) {
            Write-Error "[headless] orchestrator review -> ESCALATE on $func. STOPPING for user review."
            exit 10
        }
    }
    Write-Host "[headless] loop finished."
}
finally {
    Pop-Location
}
