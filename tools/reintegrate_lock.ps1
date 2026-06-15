#!/usr/bin/env pwsh
# tools/reintegrate_lock.ps1  <acquire|release|status|steal>  [-Reason "..."] [-Label "..."]
#
# MAIN-REINTEGRATION MUTEX manager. Pairs with the PreToolUse hook
# tools/hooks/main_reintegration_lock.py, which BLOCKS main-mutating git/queue
# ops (and build-input edits) unless THIS session holds the lock — preventing
# the 2026-06-14 orch3/orch0614b collision (one orchestrator's `git merge`/
# `reset` on main clobbering another's uncommitted reintegration).
#
# Hold the lock for the WHOLE reintegration window (apply -> build -> commit ->
# queue done), then release. Only one orchestrator may hold it at a time.
#
#   acquire   take the lock for this session (fails if another session holds a
#             FRESH lock; reclaims your own / an absent / a stale lock). Also
#             drops tmp/.allow_main_edits so worktree_contamination_guard.py
#             permits this orchestrator to edit main's build inputs.
#   release   drop the lock + tmp/.allow_main_edits (only if the lock is yours).
#   status    show the current holder + age.
#   steal     force-take a STALE lock from a presumed-dead session (-Reason logged).
#
# Identity = $env:CLAUDE_CODE_SESSION_ID (the same session_id the hook reads from
# its payload). cwd-independent: the lock lives in the MAIN repo's tmp/.

param(
    [Parameter(Position = 0)][string]$Action = "status",
    [string]$Reason = "",
    [string]$Label = ""
)
$ErrorActionPreference = 'Stop'
$STALE_SECS = 5400  # keep in sync with main_reintegration_lock.py

# Resolve the MAIN repo root (first entry of `git worktree list`), cwd-independent.
$anchor = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$first = git -C "$anchor" worktree list --porcelain 2>$null | Select-String '^worktree ' | Select-Object -First 1
if (-not $first) { Write-Error "reintegrate_lock: cannot resolve main repo from $anchor"; exit 3 }
$mainRoot = (($first.ToString()) -replace '^worktree ', '').Trim() -replace '/', '\'
$tmpDir   = Join-Path $mainRoot 'tmp'
$lockPath = Join-Path $tmpDir '.main_reintegration.lock'
$allowPath = Join-Path $tmpDir '.allow_main_edits'
if (-not (Test-Path $tmpDir)) { New-Item -ItemType Directory -Path $tmpDir -Force | Out-Null }

$sid = if ($env:CLAUDE_CODE_SESSION_ID) { $env:CLAUDE_CODE_SESSION_ID } else { "unknown-$PID" }
$now = [int64]([DateTimeOffset]::UtcNow.ToUnixTimeSeconds())

function Read-Lock {
    if (-not (Test-Path $lockPath)) { return $null }
    try { return (Get-Content $lockPath -Raw | ConvertFrom-Json) } catch { return $null }
}
function Lock-Age([object]$lk) { try { return ($now - [int64]$lk.ts) } catch { return 999999 } }
function Write-Lock {
    $head = (git -C "$mainRoot" rev-parse --short HEAD 2>$null)
    $obj = [ordered]@{ session = $sid; ts = $now; head = "$head"; label = $Label }
    ($obj | ConvertTo-Json -Compress) | Set-Content -Path $lockPath -Encoding UTF8 -NoNewline
    if (-not (Test-Path $allowPath)) { Set-Content -Path $allowPath -Value "reintegration in progress; see tools/reintegrate_lock.ps1" -Encoding UTF8 }
}

switch ($Action.ToLower()) {
    'acquire' {
        $lk = Read-Lock
        if ($lk -and $lk.session -ne $sid -and (Lock-Age $lk) -lt $STALE_SECS) {
            Write-Error "reintegrate_lock: HELD by another session '$($lk.session)' for $(Lock-Age $lk)s (head $($lk.head)). Wait for release, or 'steal' if it is dead."
            exit 1
        }
        Write-Lock
        Write-Host "[reintegrate_lock] ACQUIRED  session=$sid  head=$(git -C "$mainRoot" rev-parse --short HEAD)  (main=$mainRoot)" -ForegroundColor Green
        exit 0
    }
    'release' {
        $lk = Read-Lock
        if (-not $lk) { Write-Host "[reintegrate_lock] no lock present"; exit 0 }
        if ($lk.session -ne $sid) {
            Write-Error "reintegrate_lock: lock is held by '$($lk.session)', not you ('$sid'). Not releasing. Use 'steal' if it is dead."
            exit 1
        }
        Remove-Item $lockPath -Force -ErrorAction SilentlyContinue
        Remove-Item $allowPath -Force -ErrorAction SilentlyContinue
        Write-Host "[reintegrate_lock] RELEASED  session=$sid" -ForegroundColor Green
        exit 0
    }
    'status' {
        $lk = Read-Lock
        if (-not $lk) { Write-Host "[reintegrate_lock] FREE (no lock)  main=$mainRoot"; exit 0 }
        $age = Lock-Age $lk
        $stale = if ($age -gt $STALE_SECS) { " STALE" } else { "" }
        $mine = if ($lk.session -eq $sid) { " (yours)" } else { "" }
        Write-Host "[reintegrate_lock] HELD$stale  session=$($lk.session)$mine  age=${age}s  head=$($lk.head)  label=$($lk.label)"
        exit 0
    }
    'steal' {
        $lk = Read-Lock
        if ($lk -and $lk.session -ne $sid -and (Lock-Age $lk) -lt $STALE_SECS) {
            Write-Warning "reintegrate_lock: stealing a NON-stale lock held by '$($lk.session)' (age $(Lock-Age $lk)s < ${STALE_SECS}s). Only do this if you are certain that session is dead."
        }
        Write-Lock
        $logLine = "$(Get-Date -Format o) STEAL by $sid from '$($lk.session)' reason='$Reason'"
        Add-Content -Path (Join-Path $tmpDir 'reintegrate_lock_steals.log') -Value $logLine
        Write-Host "[reintegrate_lock] STOLE lock  session=$sid  reason='$Reason'" -ForegroundColor Yellow
        exit 0
    }
    default {
        Write-Error "usage: reintegrate_lock.ps1 <acquire|release|status|steal> [-Reason ...] [-Label ...]"
        exit 2
    }
}
