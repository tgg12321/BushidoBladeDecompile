# tools/setup_worktree.ps1 — bootstrap a fresh git worktree for engine work.
#
# `git worktree add` brings tracked files only. The heavy artifacts the engine
# depends on (the GCC 2.7.2 build, the .venv, the pristine build/ cache, the
# permuter clone, the maspsx clone, the disc directory) are all gitignored, so
# a fresh worktree has none of them. Without those, `engine sandbox` /
# `engine verify-oracle` / cheat-asm strip / permuter runs all error out at
# step one.
#
# ⚠ CRITICAL CLEANUP HAZARD ⚠
# This script creates DIRECTORY JUNCTIONS from the worktree to main's
# gitignored directories. `git worktree remove --force` on a worktree with
# junctions WILL FOLLOW THE JUNCTIONS and recursively delete the target
# (main's .venv / build / toolchain). This happened on 2026-06-03 and wiped
# main's .venv entirely.
#
# TO REMOVE A WORKTREE SAFELY:
#   pwsh tools/safe_remove_worktree.ps1 <worktree-path> -Force
#
# That script enumerates junctions, detaches them with `cmd /c rmdir` (which
# removes a junction without following it), then calls `git worktree remove`.
# DO NOT use `git worktree remove --force` directly on worktrees bootstrapped
# by this script.
#
# This script uses Windows DIRECTORY JUNCTIONS (no admin / dev-mode needed,
# unlike symbolic links) to link the heavy paths from the main repository into
# the worktree. Junctions work for directories; for the single tracked file
# tools/cc1psx.exe we use a hard link.
#
# Run from INSIDE the worktree (typically as the first action of any Workflow
# worker that uses `isolation: 'worktree'`):
#
#     pwsh tools/setup_worktree.ps1
#
# Why junctions (not copy):
#   - .venv / tools/gcc-2.7.2 / tools/decomp-permuter / tools/maspsx / disc are
#     LARGE (multi-GB combined). Copy would take minutes per worktree.
#   - All are READ-ONLY for engine use (no worker mutates them) — junctioning
#     is safe.
#   - build/ is the canonical oracle reference; sharing via junction ensures
#     the worker sees the same byte-identical reference the main repo verified.
#
# Idempotent: re-running is safe (existing junctions/files are left alone).
#
# Safety: refuses to run in the main repository (sentinel: `.git` is a
# directory, not a file). A worktree's `.git` is a FILE pointing back to
# main's gitdir.

$ErrorActionPreference = "Stop"

# Discover the worktree root.
$worktreeRoot = (git rev-parse --show-toplevel) -replace '/', '\'
Set-Location $worktreeRoot

# Safety: refuse to run in the main repo.
if (Test-Path -Path "$worktreeRoot\.git" -PathType Container) {
    Write-Output "setup_worktree.ps1: refusing to run in main repo ($worktreeRoot\.git is a directory)"
    Write-Output "  This script is for fresh worktrees only."
    exit 0
}

# Discover the main repo path from `git worktree list --porcelain`. The first
# `worktree <path>` line is always main. Path may contain spaces — read it
# verbatim from the line.
$mainRepo = $null
foreach ($line in (git worktree list --porcelain)) {
    if ($line -match '^worktree (.+)$') {
        $mainRepo = ($Matches[1] -replace '/', '\')
        break
    }
}

if (-not $mainRepo -or $mainRepo -eq $worktreeRoot) {
    Write-Output "setup_worktree.ps1: could not identify main repo (or already there). Bailing."
    exit 0
}

if (-not (Test-Path "$mainRepo\.git")) {
    Write-Output "setup_worktree.ps1: main repo $mainRepo has no .git. Bailing."
    exit 1
}

# HEAD parity enforcement. The workflow framework's `isolation: 'worktree'`
# sometimes creates worktrees at a snapshot HEAD that lags main — round-3
# workers reported regfix.txt was "~24 hours stale" and made decisions
# based on absolute .L labels that no longer exist in main. The fix is
# strict: if the worktree HEAD differs from main, sync it (or bail).
#
# Mode is controlled by $env:WORKTREE_STALE_POLICY:
#   "sync" (default): hard-reset worktree to main HEAD before running.
#         Worker's uncommitted changes are LOST — the worktree is
#         contractually a scratch space, not a long-lived branch.
#   "fail": refuse to bootstrap and exit non-zero. Forces workflow harness
#         to re-create the worktree at current HEAD.
#   "warn": log the divergence and proceed (for debugging only).
$mainHead = (git -C "$mainRepo" rev-parse HEAD 2>$null)
$wtHead = (git -C "$worktreeRoot" rev-parse HEAD 2>$null)

if ($mainHead -and $wtHead -and $mainHead -ne $wtHead) {
    $policy = if ($env:WORKTREE_STALE_POLICY) { $env:WORKTREE_STALE_POLICY } else { "sync" }
    Write-Output "setup_worktree.ps1: HEAD parity check FAILED"
    Write-Output "  worktree HEAD: $wtHead"
    Write-Output "  main HEAD:     $mainHead"
    Write-Output "  policy:        $policy"

    # Show a summary of what changed between worktree HEAD and main HEAD —
    # critical context. Workers may otherwise base decisions on file state
    # that no longer represents project truth.
    Write-Output "  diff summary (worktree..main):"
    $diffStat = (git -C "$worktreeRoot" log --oneline "$wtHead..$mainHead" 2>$null | Select-Object -First 8)
    foreach ($line in $diffStat) { Write-Output "    $line" }

    if ($policy -eq "fail") {
        Write-Output "setup_worktree.ps1: BAILING — stale-policy=fail. Workflow harness must recreate worktree at $mainHead"
        exit 1
    } elseif ($policy -eq "warn") {
        Write-Output "setup_worktree.ps1: WARN — proceeding with stale state. DO NOT trust file-line claims without verifying against main."
    } else {
        # Default: sync the worktree to main HEAD. Hard reset wipes any
        # uncommitted worker changes (intentional — worktrees are scratch).
        Write-Output "setup_worktree.ps1: syncing worktree to main HEAD $mainHead (hard reset)"
        git -C "$worktreeRoot" reset --hard "$mainHead" 2>&1 | Select-Object -Last 2 | ForEach-Object { Write-Output "    $_" }
        # Verify the sync took.
        $wtHead2 = (git -C "$worktreeRoot" rev-parse HEAD 2>$null)
        if ($wtHead2 -ne $mainHead) {
            Write-Output "setup_worktree.ps1: WARNING reset did not take — worktree HEAD still $wtHead2. Worker should bail."
            exit 1
        }
        Write-Output "setup_worktree.ps1: HEAD parity restored ($mainHead)"
    }
} elseif ($mainHead -and $wtHead) {
    Write-Output "setup_worktree.ps1: HEAD parity OK ($mainHead)"
}

Write-Output "setup_worktree.ps1: bootstrapping $worktreeRoot from main $mainRepo"

$linked = 0
$skippedExists = 0
$skippedMissing = 0

# Heavy directory paths to junction.
$dirs = @(
    ".venv"
    "build"
    "tools\gcc-2.7.2"
    "tools\decomp-permuter"
    "tools\maspsx"
    "disc"
)

foreach ($rel in $dirs) {
    $src = Join-Path $mainRepo $rel
    $dst = Join-Path $worktreeRoot $rel

    if (-not (Test-Path $src -PathType Container)) {
        Write-Output "  - skip $rel (not present in main)"
        $skippedMissing++
        continue
    }

    if (Test-Path $dst) {
        Write-Output "  - skip $rel (already present in worktree)"
        $skippedExists++
        continue
    }

    # Make sure parent dir exists.
    $parent = Split-Path $dst
    if (-not (Test-Path $parent)) {
        New-Item -ItemType Directory -Path $parent -Force | Out-Null
    }

    # Create directory junction (no admin needed on Windows).
    try {
        New-Item -ItemType Junction -Path $dst -Target $src -ErrorAction Stop | Out-Null
        Write-Output "  + junction $rel -> $src"
        $linked++
    } catch {
        Write-Output "  ! FAILED to create junction $rel ($_)"
    }
}

# tools/cc1psx.exe — single tracked-ignored file. Use a hard link (no admin).
$cc1psxRel = "tools\cc1psx.exe"
$cc1psxSrc = Join-Path $mainRepo $cc1psxRel
$cc1psxDst = Join-Path $worktreeRoot $cc1psxRel

if (Test-Path $cc1psxSrc -PathType Leaf) {
    if (Test-Path $cc1psxDst) {
        Write-Output "  - skip $cc1psxRel (already present)"
        $skippedExists++
    } else {
        try {
            New-Item -ItemType HardLink -Path $cc1psxDst -Target $cc1psxSrc -ErrorAction Stop | Out-Null
            Write-Output "  + hardlink $cc1psxRel"
            $linked++
        } catch {
            # Fall back to copy if hardlink fails (cross-volume, etc.)
            Copy-Item -Path $cc1psxSrc -Destination $cc1psxDst
            Write-Output "  + copy $cc1psxRel (hardlink fallback)"
            $linked++
        }
    }
} else {
    Write-Output "  - skip $cc1psxRel (not present in main)"
    $skippedMissing++
}

# tmp/ — never link. Each worktree gets its own scratch space.
if (-not (Test-Path "$worktreeRoot\tmp")) {
    New-Item -ItemType Directory -Path "$worktreeRoot\tmp" -Force | Out-Null
}

Write-Output "setup_worktree.ps1: done. linked=$linked already-present=$skippedExists missing-in-main=$skippedMissing"

# Sanity check: can the worker invoke engine through tools/eng.ps1?
$verifyPath = Join-Path $worktreeRoot "tools\eng.ps1"
if (Test-Path $verifyPath) {
    try {
        $result = & $verifyPath verify-oracle 2>&1 | Out-String
        if ($result -match '"build_matches"\s*:\s*true') {
            Write-Output "setup_worktree.ps1: engine smoke test OK (verify-oracle matches)"
        } else {
            Write-Output "setup_worktree.ps1: WARNING engine smoke test returned unexpected output"
            Write-Output ($result -split "`n" | Select-Object -Last 5)
        }
    } catch {
        Write-Output "setup_worktree.ps1: WARNING engine smoke test errored — worker should investigate ($_)"
    }
}
