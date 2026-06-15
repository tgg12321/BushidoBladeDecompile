#!/usr/bin/env pwsh
# tools/fleet/seed_reaudit.ps1 — populate the fleet's re-audit worklist with the
# already-COMPLETED-C functions, so the Auditor's idle patrol covers the whole
# committed log (not just functions the fleet itself completes this run).
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if ($root -match '^([A-Za-z]):[\\/](.*)$') {
    $wsldir = "/mnt/$($Matches[1].ToLower())/$($Matches[2] -replace '\\','/')"
} else { $wsldir = $root -replace '\\','/' }

$outFile = Join-Path $root 'tmp\fleet\completed.txt'
Write-Host "[seed-reaudit] enumerating completed-C functions ..."
$list = wsl bash -c "cd '$wsldir' && source .venv/bin/activate && python3 tools/fleet/list_completed.py 2>/dev/null"
if (-not $list) {
    Write-Host "[seed-reaudit] no completed list produced (engine internals unavailable); the Auditor will still re-audit fresh fleet completions." -ForegroundColor Yellow
    exit 0
}
$list | Set-Content -Path $outFile -Encoding utf8
$n = (@($list)).Count
Write-Host "[seed-reaudit] $n completed-C functions written to tmp/fleet/completed.txt"
& (Join-Path $PSScriptRoot 'coord.ps1') reaudit-seed -File $outFile
