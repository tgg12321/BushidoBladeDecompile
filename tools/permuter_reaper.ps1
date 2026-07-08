<#
.SYNOPSIS
  Independent orphaned-permuter reaper. Kills permuter.py process groups older
  than a TTL, so a crashed/abandoned grinder never leaves campaigns running.
.DESCRIPTION
  Registered as a Windows Scheduled Task (see tools/install-permuter-reaper.ps1)
  that fires every 15 min, decoupled from the grinder — cleanup no longer
  depends on the pipeline being restarted.

  Zero day-to-day cost: permuters only ever live inside WSL, so this first asks
  the LxssManager (cheap, does NOT boot a distro) whether any distro is running.
  If WSL is down — the normal state when you're not grinding — it exits in well
  under a second without waking WSL. Only when WSL is already up does it run the
  actual /proc scan + age-gated group kill.

  TTL default 3600s (60 min): ~2x the longest legitimate fresh-seed campaign, so
  a healthy run is never touched; a crash-orphan dies within TTL + poll interval.
#>
[CmdletBinding()]
param(
    [int]$TtlSeconds = 3600,
    [string]$Repo = 'C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile'
)
$ErrorActionPreference = 'SilentlyContinue'

# Is any WSL distro running? `--list --running` queries service state; it does
# NOT start a distro. Output is UTF-16 with NULs — strip to real content.
$running = (wsl.exe --list --running --quiet 2>$null) -join "`n"
$running = ($running -replace "`0", '').Trim()
if (-not $running) { exit 0 }   # WSL down => no permuters can exist => no-op

$wslRepo = '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile'
# Interpreter picked inside bash (backtick-escaped `$PY so PowerShell leaves it
# for the shell): prefer the repo venv, fall back to system python3.
wsl.exe bash -lc "cd '$wslRepo' && PY=./.venv/bin/python3; [ -x `$PY ] || PY=python3; `$PY tools/permuter_campaign.py reap --ttl $TtlSeconds" 2>$null | Out-Null
exit 0
