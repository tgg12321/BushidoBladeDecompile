#!/usr/bin/env pwsh
# tools/wteng.ps1  <target>  <engine-args...>
#
# WORKTREE-PINNED engine/build wrapper. cwd-INDEPENDENT.
#
# Why this exists (2026-06-14 cross-contamination incident): the plain
# tools/eng.ps1 resolves its repo root from $PSScriptRoot, so a RELATIVE
# `& tools/eng.ps1 ...` runs the engine against whatever repo the *shell cwd*
# points at. A worker subagent's PowerShell cwd is ALWAYS the MAIN repo (the
# `cd ../bb2-work-<id>` it ran earlier does not persist across tool calls), so a
# relative eng.ps1 silently builds / scores / MUTATES main's src — corrupting
# both the worker's own results (it reads main, not its edits) and other agents'
# work (stray edits / staged files land on shared main). wteng takes the target
# repo EXPLICITLY, so the result never depends on cwd.
#
#   <target> :
#     main          -> the MAIN repo
#     <id>          -> the sibling worker worktree  (branch work/<id>, dir
#                      ..\bb2-work-<id>); resolved from `git worktree list`
#     <path>        -> an explicit repo/worktree path
#
#   <engine-args> :
#     make [args]   -> raw `make` in <target> (full clean build + SHA1 gate)
#     <subcmd ...>  -> `python3 -m engine.cli <subcmd ...>` in <target>
#
# Examples:
#   pwsh tools/wteng.ps1 main queue status
#   pwsh tools/wteng.ps1 orch0614b-3 sandbox func_X --disable all
#   pwsh tools/wteng.ps1 orch0614b-3 make
#
# Exit code is propagated from the engine/make command.

$ErrorActionPreference = 'Stop'

if ($args.Count -lt 2) {
    Write-Error @'
usage: pwsh tools/wteng.ps1 <target> <engine-args...>
  <target> = main | <worktree-id> | <path>
  examples:
    pwsh tools/wteng.ps1 main queue status
    pwsh tools/wteng.ps1 <id> sandbox func_X --disable all
    pwsh tools/wteng.ps1 <id> make
'@
    exit 2
}

$target = [string]$args[0]
$rest   = @($args[1..($args.Count - 1)])

# Anchor = the repo containing THIS script. Works whether wteng is invoked
# relative (cwd=main -> $PSScriptRoot = main\tools) or by absolute path. We use
# git from the anchor to enumerate worktrees, so target resolution is
# cwd-independent.
$anchor = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

$wt  = @()
$cur = $null
foreach ($line in (git -C "$anchor" worktree list --porcelain 2>$null)) {
    if ($line -match '^worktree (.+)$') {
        $cur = [ordered]@{ path = ($Matches[1] -replace '/', '\'); branch = $null }
        $wt += [pscustomobject]$cur
    } elseif ($line -match '^branch (.+)$' -and $cur) {
        $wt[-1].branch = $Matches[1]
    }
}
if ($wt.Count -eq 0) { Write-Error "wteng.ps1: 'git worktree list' returned nothing from $anchor"; exit 3 }
$mainPath = $wt[0].path

# Resolve <target> -> repo root.
$root = $null
if ($target -ieq 'main') {
    $root = $mainPath
} elseif ($target -match '[\\/:]') {
    if (Test-Path $target) { $root = (Resolve-Path $target).Path }
    else { Write-Error "wteng.ps1: path not found: $target"; exit 3 }
} else {
    foreach ($w in $wt) {
        if ($w.branch -eq "refs/heads/work/$target" -or (Split-Path $w.path -Leaf) -ieq "bb2-work-$target") {
            $root = $w.path; break
        }
    }
    if (-not $root) {
        $cand = Join-Path (Join-Path (Split-Path $mainPath) 'bb2-worktrees') "bb2-work-$target"
        if (Test-Path $cand) { $root = (Resolve-Path $cand).Path }
    }
    if (-not $root) {
        $known = ($wt | ForEach-Object { "    $($_.path)  [$($_.branch)]" }) -join "`n"
        Write-Error "wteng.ps1: no worktree for id '$target' (looked for branch work/$target or dir bb2-work-$target).`nKnown worktrees:`n$known"
        exit 3
    }
}

# Windows path -> WSL /mnt/<drive>/...
if ($root -match '^([A-Za-z]):[\\/](.*)$') {
    $drive  = $Matches[1].ToLower()
    $relp   = $Matches[2] -replace '\\', '/'
    $wsldir = "/mnt/$drive/$relp"
} else {
    $wsldir = $root -replace '\\', '/'
}

# Optional metrics attribution.
$sid = ''
if ($env:CLAUDE_SESSION_ID) {
    $s = $env:CLAUDE_SESSION_ID -replace "'", "'\''"
    $sid = "CLAUDE_SESSION_ID='$s' "
}

if ($rest[0] -ieq 'make') {
    $makeArgs = (@($rest | Select-Object -Skip 1) -join ' ')
    $bashCmd  = "cd '$wsldir' && source .venv/bin/activate && ${sid}make $makeArgs"
} else {
    # Quote any token containing whitespace for bash.
    $parts = foreach ($a in $rest) {
        if ($a -match '\s') { "'" + ($a -replace "'", "'\''") + "'" } else { $a }
    }
    $cli = $parts -join ' '
    $bashCmd = "cd '$wsldir' && source .venv/bin/activate && ${sid}python3 -m engine.cli $cli"
}

Write-Host "[wteng] target=$target  root=$root" -ForegroundColor DarkGray
wsl bash -c $bashCmd
exit $LASTEXITCODE
