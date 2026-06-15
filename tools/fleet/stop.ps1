#!/usr/bin/env pwsh
# tools/fleet/stop.ps1 — stop the fleet. Sets the circuit to 'halted' so the
# supervisor gracefully kills its lanes and exits on its next monitor tick.
# -Force also hard-kills the supervisor process immediately.
[CmdletBinding()]
param([switch]$Force)
$ErrorActionPreference = 'SilentlyContinue'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$pidf = Join-Path $root 'tmp\fleet\supervisor.pid'

& (Join-Path $PSScriptRoot 'coord.ps1') circuit -Set halted -Reason 'manual stop (stop.ps1)' | Out-Null
Write-Host "Circuit set to HALTED — supervisor will wind down its lanes and exit within ~15s."

if ($Force -and (Test-Path $pidf)) {
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
