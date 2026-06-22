#!/usr/bin/env pwsh
# tools/fleet/stop.ps1 — stop the fleet. Sets the circuit to 'halted' so the
# supervisor gracefully kills its lanes and exits on its next monitor tick.
# -Force also hard-kills the supervisor process immediately.
#
# -PreserveWip (composes with -Force): before the hard-kill, commit any in-flight
# WIP checkpoints and fleet bookkeeping that the supervisor would otherwise be
# in the middle of writing. Prevents Assert-MainClean on the next launch from
# wiping a worker's half-saved checkpoint or stale event logs. Added 2026-06-22
# after two stop -Force sessions in one day each required a manual rescue commit.
[CmdletBinding()]
param([switch]$Force, [switch]$PreserveWip)
$ErrorActionPreference = 'SilentlyContinue'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$pidf = Join-Path $root 'tmp\fleet\supervisor.pid'
$reint = Join-Path $root 'tools\reintegrate_lock.ps1'

& (Join-Path $PSScriptRoot 'coord.ps1') circuit -Set halted -Reason 'manual stop (stop.ps1)' | Out-Null
Write-Host "Circuit set to HALTED — supervisor will wind down its lanes and exit within ~15s."

# ── PreserveWip: rescue mid-write state before force-killing ────────────────
function Save-DirtyWipState {
    # The paths the supervisor / workers might be mid-write on. Anything outside
    # this set we leave alone (build-pipeline edits indicate worker contamination,
    # NOT something to preserve — Assert-MainClean should still wipe those).
    $pushd = Push-Location $root
    try {
        $statusLines = git -C $root status --porcelain 2>$null
        if (-not $statusLines) { return $false }
        $wipPaths = @()
        foreach ($l in $statusLines) {
            if ($l.Length -lt 4) { continue }
            $p = $l.Substring(3).Trim()
            # Allowed preserve targets
            if ($p -like 'memory/wip/*' -or
                $p -like 'docs/fleet/*' -or
                $p -eq 'metrics/events.jsonl') {
                $wipPaths += $p
            }
        }
        if (-not $wipPaths) { return $false }
        Write-Host "  preserving: $($wipPaths -join ', ')"
        & $reint acquire 2>&1 | Out-Null
        try {
            foreach ($p in $wipPaths) { git -C $root add -- $p 2>$null }
            $msgFile = Join-Path $root 'tmp\stop_preserve_wip_msg.txt'
            if (-not (Test-Path (Split-Path $msgFile))) { New-Item -ItemType Directory -Path (Split-Path $msgFile) -Force | Out-Null }
            $ts = (Get-Date).ToUniversalTime().ToString('o')
            $body = "fleet: preserve in-flight WIP / log state on stop -Force -PreserveWip ($ts)`n`nPaths preserved:`n" + (($wipPaths | ForEach-Object { "  - $_" }) -join "`n") + "`n`nAuto-committed by tools/fleet/stop.ps1 -PreserveWip before force-kill so Assert-MainClean on the next launch doesn't wipe worker checkpoints / fleet bookkeeping mid-write.`n"
            Set-Content -Path $msgFile -Value $body -Encoding utf8 -NoNewline
            git -C $root commit -F $msgFile 2>&1 | Select-Object -Last 3 | ForEach-Object { Write-Host "  $_" }
        } finally { & $reint release 2>&1 | Out-Null }
        return $true
    } finally { Pop-Location }
}

if ($Force -and (Test-Path $pidf)) {
    if ($PreserveWip) {
        Write-Host "PreserveWip: checking for in-flight WIP / log state to commit before force-kill..."
        if (-not (Save-DirtyWipState)) { Write-Host "  (nothing dirty to preserve)" }
    }
    $spid = Get-Content $pidf
    if ($spid -and (Get-Process -Id $spid -ErrorAction SilentlyContinue)) {
        Stop-Process -Id $spid -Force
        Write-Host "Force-killed supervisor pid $spid."
    }
    # also sweep any lingering lane processes running lane.ps1
    Get-CimInstance Win32_Process -Filter "Name='pwsh.exe'" |
        Where-Object { $_.CommandLine -match 'fleet\\lane\.ps1' } |
        ForEach-Object { Stop-Process -Id $_.ProcessId -Force; Write-Host "killed lane pid $($_.ProcessId)" }
    Remove-Item $pidf -ErrorAction SilentlyContinue
}
Write-Host "Done. (Re-launch with tools/fleet/launch.ps1.)"
