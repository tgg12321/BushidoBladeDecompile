#!/usr/bin/env pwsh
# tools/eng.ps1
#
# PowerShell front-door for the decomp engine. Run engine CLI commands with
# ZERO hand-written shell quoting:
#
#   pwsh tools/eng.ps1 queue next
#   pwsh tools/eng.ps1 canonical func_800233AC
#   pwsh tools/eng.ps1 sandbox func_800233AC --disable all
#   pwsh tools/eng.ps1 verify-oracle --rebuild
#   pwsh tools/eng.ps1 retire func_800233AC
#
# Why this exists: the engine toolchain only runs under WSL (Linux cc1/maspsx/
# ld), so every engine call is really `wsl bash -c 'cd <root> && source .venv
# && python3 -m engine.cli ...'`. Hand-writing that through the Bash tool nests
# THREE shells (Git Bash -> wsl -> bash); every $, quote and backslash is parsed
# three times, which is what eats awk/sed/heredocs. This wrapper builds the WSL
# command string itself, so callers pass plain tokens and never touch quoting.
#
# It also exports CLAUDE_SESSION_ID into the WSL env so engine/metrics.py can
# attribute the run (otherwise session_id is null for interactive sessions).
#
# Exit code is propagated from the engine command.

$ErrorActionPreference = 'Stop'

if ($args.Count -eq 0) {
    Write-Error @'
usage: pwsh tools/eng.ps1 <engine-subcommand> [args...]

Runs `python3 -m engine.cli <args>` inside WSL with cwd = repo root and the
.venv activated. No shell quoting needed -- pass plain tokens.

Examples:
  pwsh tools/eng.ps1 queue next
  pwsh tools/eng.ps1 sandbox func_800233AC --disable all
  pwsh tools/eng.ps1 verify-oracle --rebuild
'@
    exit 2
}

# Repo root = parent of this script's dir. cwd-independent, worktree-safe.
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# Windows path -> WSL /mnt/<drive>/... form.
if ($root -match '^([A-Za-z]):[\\/](.*)$') {
    $drive = $Matches[1].ToLower()
    $rest  = $Matches[2] -replace '\\', '/'
    $wsldir = "/mnt/$drive/$rest"
} else {
    # Already a unix-ish path; pass through.
    $wsldir = $root -replace '\\', '/'
}

# Join args into the engine command. Quote any token containing whitespace
# (engine tokens normally don't, but be safe).
$parts = foreach ($a in $args) {
    if ($a -match '\s') { "'" + ($a -replace "'", "'\''") + "'" } else { $a }
}
$cliArgs = $parts -join ' '

# Optional metrics attribution: only set the env var inside WSL if we have one,
# so we never stamp an empty string.
$sidPrefix = ''
if ($env:CLAUDE_SESSION_ID) {
    $sid = $env:CLAUDE_SESSION_ID -replace "'", "'\''"
    $sidPrefix = "CLAUDE_SESSION_ID='$sid' "
}

# Single-quote the (space-containing) cd path for bash; no double quotes anywhere
# in this string, so PowerShell passes it as one clean argv element to wsl.
$bashCmd = "cd '$wsldir' && source .venv/bin/activate && ${sidPrefix}python3 -m engine.cli $cliArgs"

wsl bash -c $bashCmd
exit $LASTEXITCODE
