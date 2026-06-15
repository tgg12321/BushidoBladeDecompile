#!/usr/bin/env pwsh
# tools/fleet/status.ps1 — human-readable dashboard of the running fleet.
# Safe, read-only. Run any time to see what the fleet has been doing.
$ErrorActionPreference = 'SilentlyContinue'
. (Join-Path $PSScriptRoot '_fleet_common.ps1')
$main = Get-MainRoot

Coord status

$log = Join-Path $RepoRoot 'docs\fleet\log.jsonl'
if (Test-Path $log) {
    $events = Get-Content $log | ForEach-Object { try { $_ | ConvertFrom-Json } catch {} } | Where-Object { $_ }
    $completed = @($events | Where-Object { $_.kind -eq 'completed' })
    $regressed = @($events | Where-Object { $_.kind -eq 'reaudit-regressed' })
    $halts     = @($events | Where-Object { $_.kind -eq 'halted' })
    $misses    = @($events | Where-Object { $_.kind -eq 'auditor-miss' })
    Write-Host "`n=== Fleet activity (docs/fleet/log.jsonl) ==="
    Write-Host "  completed (merged):     $($completed.Count)"
    Write-Host "  reaudit regressions:    $($regressed.Count)"
    Write-Host "  auditor post-PASS miss: $($misses.Count)"
    Write-Host "  halts:                  $($halts.Count)"
    Write-Host "`n  last 10 completions:"
    $completed | Select-Object -Last 10 | ForEach-Object { Write-Host "    $($_.ts)  $($_.func)" }
    if ($regressed.Count) {
        Write-Host "`n  ⚠ regressions flagged in committed code (need clean redo):" -ForegroundColor Yellow
        $regressed | Select-Object -Last 10 | ForEach-Object { Write-Host "    $($_.func): $($_.reason)" }
    }
    if ($halts.Count) {
        Write-Host "`n  ⚠ HALT events (fleet stopped for you):" -ForegroundColor Red
        $halts | ForEach-Object { Write-Host "    $($_.ts): $($_.reason)" }
    }
}

Write-Host "`n=== Recent fleet commits on main ==="
git -C "$main" log --oneline -12 | ForEach-Object { Write-Host "  $_" }

$adj = Join-Path $RepoRoot 'docs\fleet\adjudications.md'
$inc = Join-Path $RepoRoot 'docs\fleet\incidents.md'
if (Test-Path $adj) { Write-Host "`n(Adjudicator rulings: docs/fleet/adjudications.md)" }
if (Test-Path $inc) { Write-Host "(Overseer incidents:  docs/fleet/incidents.md)" }
Write-Host "`n(Lane logs: tmp/fleet/logs/<lane>.out.log)"
