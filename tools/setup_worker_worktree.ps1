# tools/setup_worker_worktree.ps1 — bootstrap a fresh git worktree for a
# CONCURRENT decomp worker agent, giving it a PRIVATE writable build/.
#
# This is a sibling to tools/setup_worktree.ps1. The ONLY difference:
#   setup_worktree.ps1 JUNCTIONS build/ from main (shared, one copy).
#   setup_worker_worktree.ps1 COPIES build/ into the worktree (private copy).
#
# Why a private build/ for concurrent workers:
#   The completion step of the engine loop (`verify-oracle --rebuild`,
#   `retire`, `queue done`) WRITES build/ — it rebuilds objects into build/
#   and SHA1-gates the result. If multiple worker agents share ONE build/
#   via junction, two agents rebuilding at once corrupt each other's
#   reference mid-build (one's partial .o overwrites the other's pristine
#   oracle). The engine's `sandbox` only READS build/ (it builds into tmp/),
#   so for a single worker a shared junction is fine — but for CONCURRENT
#   workers each must own a private writable build/ it can rebuild/queue-done
#   against without touching any other agent's build.
#
#   build/ is small (~4.7 MB), so the per-worktree copy is fast — unlike the
#   heavy READ-ONLY deps (.venv / gcc-2.7.2 / decomp-permuter / maspsx / disc,
#   multi-GB combined) which stay junctioned (no worker mutates them, so
#   sharing is safe).
#
# ⚠ CLEANUP — STILL USE safe_remove_worktree.ps1 ⚠
#   build/ HERE is a PRIVATE COPY, not a junction — it is a REAL directory and
#   does NOT carry the junction-follow hazard (deleting it deletes only this
#   worktree's bytes, never main's). BUT the OTHER deps (.venv / gcc-2.7.2 /
#   decomp-permuter / maspsx / disc, and the cc1psx.exe hardlink) ARE junctions
#   / links into main. `git worktree remove --force` would FOLLOW those
#   junctions and destroy main's targets (this wiped main's .venv on
#   2026-06-03). So this worktree MUST still be removed with:
#
#       pwsh tools/safe_remove_worktree.ps1 <worktree-path> -Force
#
#   That script un-junctions the remaining junctions (via `cmd /c rmdir`,
#   which removes a junction without following it), then runs
#   `git worktree remove` — which deletes the worktree including this private
#   build copy. DO NOT use `git worktree remove --force` directly.
#
# This script uses Windows DIRECTORY JUNCTIONS (no admin / dev-mode needed,
# unlike symbolic links) for the read-only deps; for the single tracked file
# tools/cc1psx.exe we use a hard link.
#
# Run from INSIDE the worktree (typically the first action of any concurrent
# worker that uses an isolated worktree):
#
#     pwsh tools/setup_worker_worktree.ps1
#
# Idempotent: re-running is safe (existing junctions/copies/files are left
# alone).
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
    Write-Output "setup_worker_worktree.ps1: refusing to run in main repo ($worktreeRoot\.git is a directory)"
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
    Write-Output "setup_worker_worktree.ps1: could not identify main repo (or already there). Bailing."
    exit 0
}

if (-not (Test-Path "$mainRepo\.git")) {
    Write-Output "setup_worker_worktree.ps1: main repo $mainRepo has no .git. Bailing."
    exit 1
}

# HEAD parity enforcement. The workflow framework's isolated worktrees
# sometimes start at a snapshot HEAD that lags main — stale state has caused
# workers to make decisions based on absolute .L labels that no longer exist
# in main. The fix is strict: if the worktree HEAD differs from main, sync it
# (or bail).
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
    Write-Output "setup_worker_worktree.ps1: HEAD parity check FAILED"
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
        Write-Output "setup_worker_worktree.ps1: BAILING — stale-policy=fail. Workflow harness must recreate worktree at $mainHead"
        exit 1
    } elseif ($policy -eq "warn") {
        Write-Output "setup_worker_worktree.ps1: WARN — proceeding with stale state. DO NOT trust file-line claims without verifying against main."
    } else {
        # Default: sync the worktree to main HEAD. Hard reset wipes any
        # uncommitted worker changes (intentional — worktrees are scratch).
        Write-Output "setup_worker_worktree.ps1: syncing worktree to main HEAD $mainHead (hard reset)"
        git -C "$worktreeRoot" reset --hard "$mainHead" 2>&1 | Select-Object -Last 2 | ForEach-Object { Write-Output "    $_" }
        # Verify the sync took.
        $wtHead2 = (git -C "$worktreeRoot" rev-parse HEAD 2>$null)
        if ($wtHead2 -ne $mainHead) {
            Write-Output "setup_worker_worktree.ps1: WARNING reset did not take — worktree HEAD still $wtHead2. Worker should bail."
            exit 1
        }
        Write-Output "setup_worker_worktree.ps1: HEAD parity restored ($mainHead)"
    }
} elseif ($mainHead -and $wtHead) {
    Write-Output "setup_worker_worktree.ps1: HEAD parity OK ($mainHead)"
}

Write-Output "setup_worker_worktree.ps1: bootstrapping $worktreeRoot from main $mainRepo"

$linked = 0
$skippedExists = 0
$skippedMissing = 0

# Heavy READ-ONLY directory paths to junction. NOTE: build/ is deliberately
# ABSENT here — it gets a private COPY below (the whole point of this script).
$dirs = @(
    ".venv"
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

# build/ — PRIVATE COPY (NOT a junction). This is the key difference from
# setup_worktree.ps1. Each concurrent worker gets its own pristine, writable
# build/ so its completion step (verify-oracle --rebuild / retire / queue
# done) can rebuild + SHA1-gate against a private oracle without corrupting
# any other agent's build. ~4.7 MB, so the copy is fast.
$buildRel = "build"
$buildSrc = Join-Path $mainRepo $buildRel
$buildDst = Join-Path $worktreeRoot $buildRel

if (-not (Test-Path $buildSrc -PathType Container)) {
    Write-Output "  - skip $buildRel (not present in main)"
    $skippedMissing++
} elseif (Test-Path $buildDst) {
    Write-Output "  - skip $buildRel (already present in worktree)"
    $skippedExists++
} else {
    try {
        Copy-Item -Recurse -Force $buildSrc $buildDst -ErrorAction Stop
        Write-Output "  + copy $buildRel (private writable build/, not a junction)"
        $linked++
    } catch {
        Write-Output "  ! FAILED to copy $buildRel ($_)"
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

Write-Output "setup_worker_worktree.ps1: done. linked/copied=$linked already-present=$skippedExists missing-in-main=$skippedMissing"

# Sanity check: can the worker invoke engine through tools/eng.ps1? This
# verifies the PRIVATE build copy matches the oracle (build == oracle).
$verifyPath = Join-Path $worktreeRoot "tools\eng.ps1"
if (Test-Path $verifyPath) {
    try {
        $result = & $verifyPath verify-oracle 2>&1 | Out-String
        if ($result -match '"build_matches"\s*:\s*true') {
            Write-Output "setup_worker_worktree.ps1: engine smoke test OK (verify-oracle matches against private build copy)"
        } else {
            Write-Output "setup_worker_worktree.ps1: WARNING engine smoke test returned unexpected output"
            Write-Output ($result -split "`n" | Select-Object -Last 5)
        }
    } catch {
        Write-Output "setup_worker_worktree.ps1: WARNING engine smoke test errored — worker should investigate ($_)"
    }
}
