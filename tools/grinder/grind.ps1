<#
.SYNOPSIS
  The Grinder — single-lane walk-away decomp driver.
  Spec: docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md
.DESCRIPTION
  Deterministic loop: queue top -> ensure ledger -> assemble brief (mandated
  modality) -> spawn ONE grind session -> validate outcome -> apply/revert ->
  candidate path (verify bytes THEN judge) -> repeat. No 'blocked' outcome
  exists; invalid sessions are discarded and respawned. Stop: create tmp/grind/STOP
  or run stop via -Stop.
.PARAMETER Once
  Run exactly one iteration (drills, supervised runs).
.PARAMETER MockSessionScript
  Path to a .ps1 executed INSTEAD of claude for the grind session (drills).
  It receives env GRIND_BRIEF_PATH and GRIND_OUTCOME_PATH.
.PARAMETER MockJudgeScript
  Same, for the judge.
#>
[CmdletBinding()]
param(
    [switch]$Once,
    [switch]$Stop,
    [string]$Model = 'opus',
    [string]$JudgeModel = 'opus',
    [int]$SessionTimeoutMin = 90,
    [string]$MockSessionScript = '',
    [string]$MockJudgeScript = ''
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)   # repo root
Set-Location $Root
$GrindTmp = Join-Path $Root 'tmp\grind'
$PidFile  = Join-Path $GrindTmp 'grind.lock'
$StopFile = Join-Path $GrindTmp 'STOP'
$Journal  = Join-Path $Root 'docs\grind\journal.md'
$Decisions = Join-Path $Root 'docs\grind\decisions.md'
$RolesDir = Join-Path $PSScriptRoot 'roles'
New-Item -ItemType Directory -Force $GrindTmp, (Join-Path $Root 'docs\grind') | Out-Null

function Log([string]$msg) {
    $line = "[grind $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')] $msg"
    Write-Host $line
    Add-Content -Path (Join-Path $GrindTmp 'grind.log') -Value $line
}
function Journal([string]$msg) {
    Add-Content -Path $Journal -Value "- $(Get-Date -Format 'yyyy-MM-dd HH:mm') $msg"
}

if ($Stop) {
    New-Item -ItemType File -Force $StopFile | Out-Null
    Log "STOP sentinel written; driver exits at next session boundary."
    exit 0
}

# ── single-instance lock with stale reclaim ──────────────────────────────────
if (Test-Path $PidFile) {
    $oldPid = Get-Content $PidFile -ErrorAction SilentlyContinue
    if ($oldPid -and (Get-Process -Id $oldPid -ErrorAction SilentlyContinue)) {
        Log "Another grinder is running (pid $oldPid). Exiting."
        exit 1
    }
    Log "Reclaiming stale lock (dead pid $oldPid)."
}
$PID | Set-Content $PidFile
Remove-Item $StopFile -ErrorAction SilentlyContinue

function Invoke-Eng([string[]]$CmdArgs) {
    # engine command on main; returns stdout text (throws on wteng failure)
    (& (Join-Path $Root 'tools\wteng.ps1') 'main' @CmdArgs 2>&1 | Out-String)
}
function Test-OracleGreen {
    try { Invoke-Eng @('verify-oracle') | Out-Null; return $LASTEXITCODE -eq 0 }
    catch { return $false }
}
function Get-QueueTop {
    # `queue next` prints a single pretty-printed JSON object (with a nested
    # "wip" key when a checkpoint exists), preceded by wteng's [wteng] banner.
    # Robust parse: take the substring from the first '{' to the last '}' and
    # ConvertFrom-Json. Returns $null if there is no object or it won't parse.
    $raw = Invoke-Eng @('queue', 'next')
    $lo = $raw.IndexOf('{'); $hi = $raw.LastIndexOf('}')
    if ($lo -lt 0 -or $hi -le $lo) { return $null }
    try { return ($raw.Substring($lo, $hi - $lo + 1) | ConvertFrom-Json) }
    catch { return $null }
}
function Assert-CleanTree {
    $dirty = @(git -C $Root status --porcelain | Where-Object { $_ })
    return $dirty
}

Log "grinder starting (pid $PID, model $Model, judge $JudgeModel)"
if (-not (Test-OracleGreen)) {
    Log "PRE-FLIGHT FAIL: oracle not green on main. Fix before grinding."
    Remove-Item $PidFile; exit 2
}
Log "pre-flight: oracle green."
