#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Pre-flight drill for the Autonomous Decomp Fleet. Run this (and watch it) before
  launching unattended.

.DESCRIPTION
  1. Init the coordinator + seed the historical re-audit worklist.
  2. Run ONE real backlog-worker cycle (a live agent on the top function).
  3. Run ONE real Auditor cycle (reviews an in-review candidate and, on PASS,
     performs the privileged merge + oracle gate — or re-audits a completed
     function if nothing is in review).
  4. Run the deterministic SELF-TEST: prove a non-matching candidate cannot survive
     the merge gate (main stays green) and the overseer wiring re-arms the fleet.
  5. Report a go / no-go verdict.

  Safe to run: the only main mutation is a legitimate completion (if the worker
  closes the function) — the self-test's bad candidate is always rolled back.
#>
[CmdletBinding()]
param([string]$Model = 'opus')
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_fleet_common.ps1')
$fleet = Join-Path $PSScriptRoot 'fleet.ps1'
$lane  = Join-Path $PSScriptRoot 'lane.ps1'

Write-Host "############ FLEET PRE-FLIGHT DRILL ############" -ForegroundColor Cyan

Write-Host "`n[1/5] coord init + seed re-audit worklist ..."
Coord init | Out-Host
& (Join-Path $PSScriptRoot 'seed_reaudit.ps1')
Coord circuit -Set running -Reason 'drill' | Out-Null

Write-Host "`n[2/5] one REAL backlog-worker cycle (live agent on the top function) ..." -ForegroundColor Cyan
& pwsh -NoProfile -File $lane -Role backlog-worker -Lane fleet-bw1 -Model $Model -Once
Write-Host "--- coord status after worker ---"
Coord status

Write-Host "`n[3/5] one REAL auditor cycle (review + privileged merge on PASS) ..." -ForegroundColor Cyan
& pwsh -NoProfile -File $fleet -AuditOnce -Model $Model
Write-Host "--- coord status after auditor ---"
Coord status

Write-Host "`n[4/5] post-pipeline oracle check (main must be byte-identical to the oracle) ..." -ForegroundColor Cyan
$green = Test-OracleRebuild
Write-Host "  oracle green on main: $green"

Write-Host "`n[5/5] deterministic SELF-TEST (merge rollback safety + overseer wiring) ..." -ForegroundColor Cyan
& pwsh -NoProfile -File $fleet -SelfTest -Model $Model
$selftest = ($LASTEXITCODE -eq 0)

Write-Host "`n############ DRILL VERDICT ############" -ForegroundColor Cyan
Write-Host "  oracle green after live pipeline : $green"
Write-Host "  self-test safety invariants      : $selftest"
if ($green -and $selftest) {
    Write-Host "  GO — safe to launch unattended (pwsh tools/fleet/launch.ps1)." -ForegroundColor Green
    exit 0
} else {
    Write-Host "  NO-GO — investigate before launching." -ForegroundColor Red
    exit 1
}
