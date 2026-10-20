#!/usr/bin/env pwsh
# tools/lanes/launch.ps1 <lane>... [-ClaudeArgs "<args>"] [-DryRun]
#
# Start one interactive Claude session per lane, each in its own Windows Terminal tab (window
# "bb2-lanes"), in the MAIN checkout so CLAUDE.md, hooks and skills load. Each session runs the
# lane's skill, which opens the lane worktree itself:
#   naming-*   -> /naming-session <lane>
#   anything   -> /foundation-session <lane>
#
# Examples:
#   pwsh tools/lanes/launch.ps1 naming-a naming-b pins
#   pwsh tools/lanes/launch.ps1 naming-a fake -ClaudeArgs "--permission-mode bypassPermissions"
#
# Status of every lane: python tools/lanes/lanes.py status

param(
    [Parameter(Position = 0, ValueFromRemainingArguments = $true)][string[]]$Lanes,
    [string]$ClaudeArgs = "",
    [switch]$DryRun
)
$ErrorActionPreference = 'Stop'

if (-not $Lanes -or $Lanes.Count -eq 0) {
    Write-Error "usage: pwsh tools/lanes/launch.ps1 <lane>... (e.g. naming-a naming-b pins)"
    exit 2
}
$main = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$first = git -C "$main" worktree list --porcelain 2>$null | Select-String '^worktree ' | Select-Object -First 1
if ($first) { $main = (($first.ToString()) -replace '^worktree ', '').Trim() -replace '/', '\' }

$seen = @{}
foreach ($lane in $Lanes) {
    if ($lane -notmatch '^[a-z0-9][a-z0-9-]{0,30}$') { Write-Error "bad lane name '$lane'"; exit 2 }
    if ($seen.ContainsKey($lane)) { Write-Error "lane '$lane' given twice (one session per lane)"; exit 2 }
    $seen[$lane] = $true
}

foreach ($lane in $Lanes) {
    $skill = if ($lane -like 'naming*') { 'naming-session' } else { 'foundation-session' }
    $prompt = "/$skill $lane"
    $extra = $ClaudeArgs  # passed through verbatim, e.g. "--permission-mode bypassPermissions"
    $inner = "Set-Location -LiteralPath '$($main -replace "'", "''")'; claude $extra '$prompt'"
    if ($DryRun) { Write-Host "[launch] $lane : $inner"; continue }
    # wt treats ';' as its own command separator, so pass the command encoded.
    $enc = [Convert]::ToBase64String([Text.Encoding]::Unicode.GetBytes($inner))
    wt -w bb2-lanes new-tab --title "lane $lane" -d "$main" pwsh -NoExit -EncodedCommand $enc
    Write-Host "[launch] started $lane ($prompt)"
    Start-Sleep -Milliseconds 700  # let wt attach each tab to the same window in order
}
