#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Reorder the fleet's ACTIVE backlog toward the most-tractable items first.

.DESCRIPTION
  The queue is otherwise easiest-FIRST by raw pure-C distance, but low distance does
  not mean closeable (a distance-2 function can be an uncloseable compiler-fold/PRE
  plateau). This re-tiers the backlog `order` in the live coordinator state so the
  worker tackles the genuinely-winnable subset first:

    tier 0  verdict C  AND  distance <= MaxTractableDistance   (the tractable subset)
    tier 1  verdict C  AND  distance <= 50
    tier 2  verdict C  AND  distance  > 50  (large pure-C gaps)
    tier 3  ASM-SUSPECT / ASM-PARTIAL       (asm-leaning — pushed to the back)

  Within a tier: distance + 2*rules ascending (fewer rules / smaller gap first).

  Mutex-safe (shares coord.ps1's named mutex) so it is safe to run WHILE the fleet is
  live — it only rewrites `order` on lane_state='backlog' entries; the change takes
  effect on the worker's next claim. Operational-state only; persists across relaunch
  (coord init preserves `order`).

.EXAMPLE
  pwsh tools/fleet/reshuffle.ps1
  pwsh tools/fleet/reshuffle.ps1 -MaxTractableDistance 20
#>
[CmdletBinding()]
param([int]$MaxTractableDistance = 15)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$sp = Join-Path $root 'tmp\fleet\state.json'
if (-not (Test-Path $sp)) { throw "no coord state at $sp (run the fleet / coord init first)" }

$mtx = New-Object System.Threading.Mutex($false, 'bb2_fleet_coord_mutex')
if (-not $mtx.WaitOne([TimeSpan]::FromSeconds(60))) { throw 'could not acquire coord mutex' }
try {
    $st = Get-Content $sp -Raw -Encoding utf8 | ConvertFrom-Json -AsHashtable
    $n = 0
    foreach ($k in @($st.functions.Keys)) {
        $e = $st.functions[$k]
        if ($e.lane_state -ne 'backlog') { continue }   # only reorder claimable backlog
        $d = [double]$e.distance
        $tier = if ([string]$e.verdict -ne 'C') { 3 }
                elseif ($d -le $MaxTractableDistance) { 0 }
                elseif ($d -le 50) { 1 }
                else { 2 }
        $e.order = $tier * 100000.0 + $d + [int]$e.rules * 2
        $n++
    }
    $tmp = "$sp.tmp"
    ($st | ConvertTo-Json -Depth 30) | Set-Content -Path $tmp -Encoding utf8
    Move-Item -Path $tmp -Destination $sp -Force
    Write-Host "reshuffled $n backlog functions by tractability (tier-0 = verdict C + distance <= $MaxTractableDistance)"
} finally {
    $mtx.ReleaseMutex() | Out-Null
    $mtx.Dispose()
}
