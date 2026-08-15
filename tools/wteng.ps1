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

# --- persistent-bridge path (DEFAULT ON; set BB2_WSL_BRIDGE=0 to disable) ---
# Every wsl.exe invocation leaks a kernel Job object (~14 KB nonpaged, drained
# only by reboot), and the grind makes thousands of them — see
# tools/wsl_bridge.ps1 and memory/project/wsl-kernel-object-leak-audio.md.
# Routing through one long-lived bash session takes that to ~0.
#
# Opt-in until 2026-08-14, which meant it was never actually on: nothing in the
# repo or the environment ever set BB2_WSL_BRIDGE, so every grind since the
# bridge landed ran the direct path and leaked ~1 job per engine call. Re-measured
# that day (10 real `queue next` calls per arm): direct 1.00 jobs/call, bridge
# 0.00. Now opt-OUT — the fallback below already makes a broken bridge a
# non-event, so defaulting to the leaky path bought nothing.
#
# `make` deliberately stays on the DIRECT path: it is one invocation for a
# multi-minute build (so the leak saving is a single job object) and it is the
# case where watching output stream live actually matters. The bridge returns a
# command's output only on completion.
#
# ANY bridge problem returns $null and we fall through to the direct call, so
# this can slow the build down but never break it.
if ($env:BB2_WSL_BRIDGE -ne '0' -and $rest[0] -ine 'make') {
    . (Join-Path $PSScriptRoot 'wsl_bridge.ps1')
    $r = Invoke-WslBridge -WslCwd $wsldir -Command $bashCmd -AutoStart
    if ($null -ne $r) {
        foreach ($l in $r.out) { Write-Output $l }
        # [Console]::Error, NOT Write-Error: Write-Error wraps each line in a
        # PowerShell ErrorRecord ("Write-Error: usage: engine ..."), which does
        # not match what the direct `wsl bash -c` path passes through and would
        # break anything parsing engine stderr.
        foreach ($l in $r.err) { [Console]::Error.WriteLine($l) }
        exit $r.code
    }
    Write-Host "[wteng] bridge unavailable — direct wsl.exe" -ForegroundColor DarkGray
}

wsl bash -c $bashCmd
exit $LASTEXITCODE
