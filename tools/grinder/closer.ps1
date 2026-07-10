<#
.SYNOPSIS
  The Closer — supervised single-session campaign driver for the final-stretch
  work on marionation_Exec + cpu_side_move_dir_4.
.DESCRIPTION
  Unlike grind.ps1 (an unattended loop of stateless 90-min sessions), this runs
  ONE long, high-reasoning session against a mission brief, then validates and
  reports. Owner-supervised: scope violations are logged loudly, not silently
  discarded; candidate trees are left in place for review.

  Flow: oracle pre-flight -> compose prompt (mission + rulings pointer) ->
  spawn ONE claude session (role: roles/closer.md) -> scope check -> outcome
  report -> byte-verify if candidate-ready -> reap permuter orphans.
.PARAMETER Mission
  Path to the mission brief markdown (e.g. docs/closer/mission-phase1-evidence.md).
.PARAMETER KeepSrc
  Leave src/include edits in the tree even for non-candidate outcomes.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Mission,
    [string]$Model = 'fable',
    [int]$TimeoutMin = 240,
    [switch]$KeepSrc
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)   # repo root
Set-Location $Root
$CloserTmp = Join-Path $Root 'tmp\closer'
New-Item -ItemType Directory -Force $CloserTmp | Out-Null

function Log([string]$msg) {
    $line = "[closer $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')] $msg"
    Write-Host $line
    Add-Content -Path (Join-Path $CloserTmp 'closer.log') -Value $line
}
function Invoke-Eng([string[]]$CmdArgs) {
    (& (Join-Path $Root 'tools\wteng.ps1') 'main' @CmdArgs 2>&1 | Out-String)
}
function Reap-Permuters {
    try {
        $n = (& wsl bash -c 'pgrep -fc "permuter\.py" || true' 2>$null | Out-String).Trim()
        if ($n -and [int]$n -gt 0) {
            & wsl bash -c 'pkill -f "permuter\.py" || true' 2>$null | Out-Null
            Log "reaped $n permuter process(es)."
        }
    } catch { }
}

# single-instance lock (a second closer editing src/ concurrently = disaster)
$LockFile = Join-Path $CloserTmp 'closer.lock'
if (Test-Path $LockFile) {
    $oldPid = Get-Content $LockFile -ErrorAction SilentlyContinue
    if ($oldPid -and (Get-Process -Id $oldPid -ErrorAction SilentlyContinue)) {
        Log "another closer is running (pid $oldPid). Exiting."
        exit 1
    }
    Log "reclaiming stale lock (dead pid $oldPid)."
}
$PID | Set-Content $LockFile

$MissionPath = if ([System.IO.Path]::IsPathRooted($Mission)) { $Mission } else { Join-Path $Root $Mission }
if (-not (Test-Path $MissionPath)) { Remove-Item $LockFile; throw "mission brief not found: $MissionPath" }
$missionName = [System.IO.Path]::GetFileNameWithoutExtension($MissionPath)
$OutcomePath = Join-Path $CloserTmp "outcome_$missionName.json"
Remove-Item $OutcomePath -ErrorAction SilentlyContinue

Log "closer starting: mission=$missionName model=$Model timeout=${TimeoutMin}m"

# pre-flight
Invoke-Eng @('verify-oracle') | Out-Null
if ($LASTEXITCODE -ne 0) { Log "PRE-FLIGHT FAIL: oracle not green on main."; Remove-Item $LockFile -ErrorAction SilentlyContinue; exit 2 }
Log "pre-flight: oracle green."

$Task = @"
$(Get-Content $MissionPath -Raw)

---
When finished, write your outcome JSON (single object, schema in your role
instructions) to EXACTLY this path (overwrite it):
  $OutcomePath
"@

$RoleFile = Join-Path $PSScriptRoot 'roles\closer.md'
$SessionId = [guid]::NewGuid().ToString()
Log "spawning session $SessionId"

$job = Start-Job -ScriptBlock {
    param($Task, $RoleFile, $Model, $SessionId)
    $claudeArgs = @('-p', $Task, '--append-system-prompt-file', $RoleFile,
                    '--permission-mode', 'bypassPermissions', '--model', $Model,
                    '--session-id', $SessionId, '--output-format', 'json')
    $null = ($null | & claude @claudeArgs | Out-String)
} -ArgumentList $Task, $RoleFile, $Model, $SessionId
if (-not (Wait-Job $job -Timeout ($TimeoutMin * 60))) {
    Log "session TIMED OUT at ${TimeoutMin}m — stopping."
    Stop-Job $job -ErrorAction SilentlyContinue
}
Remove-Job $job -Force -ErrorAction SilentlyContinue

# scope check — log loudly, don't destroy (supervised mode)
$AllowedDirtyPattern = '^(\?\?|.M|M.|A.|.A)\s+("?)(memory/|docs/closer/|docs/grind/|tmp/|metrics/|src/|include/|evidence/)'
$violations = @(git -C $Root status --porcelain | Where-Object { $_ -and $_ -notmatch $AllowedDirtyPattern })
if ($violations.Count) {
    Log "SCOPE WARNING — session touched: $($violations -join ' | ')"
    Log "review and revert manually; NOT auto-discarding (supervised mode)."
}

$o = $null
if (Test-Path $OutcomePath) {
    try { $o = Get-Content $OutcomePath -Raw | ConvertFrom-Json } catch { }
}
if (-not $o) {
    Log "NO OUTCOME (missing/unparseable $OutcomePath) — session unproven; src reverted."
    git -C $Root checkout -- src include 2>$null
    Reap-Permuters
    Remove-Item $LockFile -ErrorAction SilentlyContinue
    exit 1
}

Log "result=$($o.result) headline='$($o.headline)'"

# ── batch completion path (library-adoption missions) ────────────────────────
# outcome.completed_funcs = funcs the session drove to sandbox 0 (--disable all).
# The DRIVER (not the agent) strips their regfix/asmfix rules — deletion-only by
# construction (lines keyed "^func:") — then re-proves the WHOLE oracle and runs
# each through the real `queue done` gate (mark_done self-guards: 0 rules +
# oracle green + no cheat constructs). Anything refused stays in the queue.
if ($o.completed_funcs -and @($o.completed_funcs).Count -gt 0) {
    $funcs = @($o.completed_funcs | ForEach-Object { [string]$_ })
    Log "completion path: $($funcs.Count) claimed: $($funcs -join ', ')"
    foreach ($f in $funcs) {
        foreach ($rf in 'regfix.txt', 'asmfix.txt') {
            $p = Join-Path $Root $rf
            $lines = @(Get-Content $p)
            $kept = @($lines | Where-Object { $_ -notmatch "^\s*$([regex]::Escape($f))\s*:" })
            if ($kept.Count -ne $lines.Count) {
                Set-Content -Path $p -Value $kept -Encoding utf8
                Log "stripped $($lines.Count - $kept.Count) $rf rule(s) for $f"
            }
        }
    }
    $vo = Invoke-Eng @('verify-oracle', '--rebuild', '--allow-dirty')
    if ($LASTEXITCODE -ne 0) {
        Log "BYTE-VERIFY FAILED after rule strip — restoring rules, completing NOTHING. Output tail:`n$(($vo -split "`n") | Select-Object -Last 8 | Out-String)"
        git -C $Root checkout -- regfix.txt asmfix.txt 2>$null
    } else {
        Log "BYTES GREEN with rules stripped."
        $done = @()
        foreach ($f in $funcs) {
            $qd = Invoke-Eng @('queue', 'done', $f)
            if ($LASTEXITCODE -eq 0) { $done += $f; Log "queue done: $f" }
            else { Log "queue done REFUSED for ${f}: $(($qd -split "`n") | Select-Object -Last 2)" }
        }
        git -C $Root add -- src include regfix.txt asmfix.txt engine/queue.json memory docs/closer metrics/events.jsonl 2>$null
        git -C $Root commit -m "closer: adopt PsyQ library sources — completed $($done -join ', ')" 2>$null | Out-Null
        Log "COMPLETED $($done.Count)/$($funcs.Count) — committed."
    }
}
switch ([string]$o.result) {
    'candidate-ready' {
        Log "candidate claimed — byte-verifying (tree left in place for review)..."
        $vo = Invoke-Eng @('verify-oracle', '--rebuild', '--allow-dirty')
        if ($LASTEXITCODE -eq 0) { Log "BYTES VERIFIED GREEN. Review + commit via normal candidate path." }
        else { Log "BYTE-VERIFY FAILED — candidate claim not proven. Output:`n$vo" }
    }
    'ruling-request' {
        Log "RULING REQUESTED: $($o.ruling_question)"
        if (-not $KeepSrc) { git -C $Root checkout -- src include 2>$null }
    }
    default {
        if (-not $KeepSrc) { git -C $Root checkout -- src include 2>$null; Log "progress outcome — src reverted, findings live in memory/closer/." }
    }
}
Reap-Permuters
Remove-Item $LockFile -ErrorAction SilentlyContinue
Log "closer done: $missionName"
