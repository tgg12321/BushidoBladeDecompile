#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Launch the Autonomous Decomp Fleet supervisor as a DETACHED background process
  that keeps running after this shell (and the launching agent session) exits.

.DESCRIPTION
  Starts tools/fleet/fleet.ps1 hidden + detached, records its PID, and tails the
  first lines of the supervisor log so you can confirm it came up. Stop it any
  time with tools/fleet/stop.ps1.

  For maximum durability across a full Windows logoff, register a Scheduled Task
  instead (see tools/fleet/README.md); a detached process survives a closed shell
  and an ended agent session but not a logoff.
#>
[CmdletBinding()]
param(
    [string]$Model = 'opus',
    [string]$CheapModel = 'sonnet',   # active backlog decomp, re-audit patrol, overseer
    [string]$StrongModel = 'opus',    # blocked no-quit grind, forward merge-gate, adjudicator
    [int]$Workers = 2,            # number of backlog-worker lanes
    [switch]$NoBlocked,           # drop the blocked-worker lane
    [switch]$NoAdjudicator,       # drop the adjudicator lane
    [switch]$NoReaudit            # auditor only gates IN_REVIEW (no idle reaudit patrol)
)
$ErrorActionPreference = 'Stop'
$root   = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$logDir = Join-Path $root 'tmp\fleet\logs'
if (-not (Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir -Force | Out-Null }
$fleet  = Join-Path $PSScriptRoot 'fleet.ps1'
$out    = Join-Path $logDir 'supervisor.out.log'
$err    = Join-Path $logDir 'supervisor.err.log'
$pidf   = Join-Path $root 'tmp\fleet\supervisor.pid'

# refuse to double-launch
if (Test-Path $pidf) {
    $oldpid = Get-Content $pidf
    if ($oldpid -and (Get-Process -Id $oldpid -ErrorAction SilentlyContinue)) {
        Write-Host "Fleet supervisor already running (pid $oldpid). Use tools/fleet/stop.ps1 first." -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "Launching fleet supervisor (detached) ..."
$fleetArgs = @('-NoProfile','-File',"`"$fleet`"",'-Model',$Model,'-CheapModel',$CheapModel,'-StrongModel',$StrongModel,'-Workers',$Workers)
if ($NoBlocked)     { $fleetArgs += '-NoBlocked' }
if ($NoAdjudicator) { $fleetArgs += '-NoAdjudicator' }
if ($NoReaudit)     { $fleetArgs += '-NoReaudit' }
Write-Host "  config: $($fleetArgs -join ' ')"
$p = Start-Process pwsh -PassThru -WindowStyle Hidden `
    -RedirectStandardOutput $out -RedirectStandardError $err `
    -ArgumentList $fleetArgs
$p.Id | Set-Content -Path $pidf -Encoding ascii
Write-Host "  supervisor pid: $($p.Id)  (written to tmp/fleet/supervisor.pid)"
Write-Host "  log: tmp/fleet/logs/supervisor.out.log"
Write-Host "  monitor : pwsh tools/fleet/status.ps1"
Write-Host "  stop    : pwsh tools/fleet/stop.ps1"

Start-Sleep -Seconds 8
if (Get-Process -Id $p.Id -ErrorAction SilentlyContinue) {
    Write-Host "`nSupervisor is up. First log lines:" -ForegroundColor Green
    if (Test-Path $out) { Get-Content $out -TotalCount 20 | ForEach-Object { Write-Host "  $_" } }
} else {
    Write-Host "`nSupervisor exited immediately — check tmp/fleet/logs/supervisor.err.log" -ForegroundColor Red
    if (Test-Path $err) { Get-Content $err -Tail 30 | ForEach-Object { Write-Host "  $_" } }
    exit 1
}
