# tools/safe_remove_worktree.ps1 — junction-safe wrapper around `git worktree remove`.
#
# THE HAZARD this script prevents:
#   `git worktree remove --force <path>` recursively deletes the worktree's
#   working tree. On Windows, if the worktree contains DIRECTORY JUNCTIONS
#   (created by tools/setup_worktree.ps1 for .venv, build, tools/gcc-2.7.2,
#   etc.), the recursive delete FOLLOWS the junctions and destroys the
#   TARGET contents — wiping out main's .venv / build / toolchain.
#
#   This actually happened 2026-06-03: running `git worktree remove --force`
#   on a test worktree wiped main's .venv, breaking every subsequent engine
#   command until rebuild.
#
# THE FIX:
#   Before calling `git worktree remove`, this script enumerates junctions
#   under the worktree path and removes them with `cmd /c rmdir <path>` —
#   the Windows command for deleting a junction WITHOUT following it. After
#   junctions are detached, the regular `git worktree remove` is safe.
#
# Usage:
#   pwsh tools/safe_remove_worktree.ps1 <worktree-path>
#   pwsh tools/safe_remove_worktree.ps1 <worktree-path> -Force
#
# Refuses to operate on the main repo (sentinel: .git is a directory not file).

param(
    [Parameter(Mandatory=$true)]
    [string]$WorktreePath,
    [switch]$Force
)

$ErrorActionPreference = "Stop"

# Normalize the path.
$WorktreePath = (Resolve-Path -Path $WorktreePath -ErrorAction Stop).Path

# Safety: refuse to operate on main repo.
if (Test-Path -Path "$WorktreePath\.git" -PathType Container) {
    Write-Output "safe_remove_worktree.ps1: REFUSING to operate on $WorktreePath"
    Write-Output "  .git is a directory (main repo), not a file (linked worktree)."
    exit 1
}

if (-not (Test-Path -Path "$WorktreePath\.git" -PathType Leaf)) {
    Write-Output "safe_remove_worktree.ps1: $WorktreePath does not appear to be a git worktree (no .git file)"
    exit 1
}

Write-Output "safe_remove_worktree.ps1: preparing to remove worktree $WorktreePath"

# Enumerate junctions under the worktree using `cmd /c dir /AL /S /B` (list
# reparse points). PowerShell's Get-Item with ReparsePoint attribute also
# works but is slower for deep trees.
$junctions = @()
$out = (cmd /c dir /AL /S /B "$WorktreePath" 2>$null) | Where-Object { $_ -ne $null }
foreach ($line in $out) {
    if ($line -and (Test-Path $line)) {
        $item = Get-Item -Force $line -ErrorAction SilentlyContinue
        if ($item -and ($item.Attributes -band [System.IO.FileAttributes]::ReparsePoint)) {
            $junctions += $line
        }
    }
}

if ($junctions.Count -eq 0) {
    Write-Output "  no junctions found — safe to use git worktree remove directly"
} else {
    Write-Output "  found $($junctions.Count) junction(s) to detach BEFORE removal:"
    foreach ($j in $junctions) { Write-Output "    $j" }

    if (-not $Force) {
        Write-Output ""
        Write-Output "Use -Force to actually detach + remove. (Dry-run only without -Force.)"
        exit 0
    }

    # Detach each reparse point. For DIRECTORY junctions use `cmd /c rmdir`
    # (removes without following). For FILE symlinks use Remove-Item -Force
    # (deletes the link without following). Sort longest-first so nested
    # reparse points detach inside-out.
    $junctions = $junctions | Sort-Object -Property Length -Descending
    foreach ($j in $junctions) {
        $item = Get-Item -Force $j -ErrorAction SilentlyContinue
        if (-not $item) {
            Write-Output "  skip (already gone): $j"
            continue
        }
        $isDir = $item.PSIsContainer
        Write-Output "  detaching $(if ($isDir) {'junction'} else {'symlink'}): $j"
        if ($isDir) {
            cmd /c rmdir "$j" 2>&1 | ForEach-Object { Write-Output "    $_" }
        } else {
            # File symlink — Remove-Item -Force removes the link without
            # following it.
            Remove-Item -Force $j -ErrorAction SilentlyContinue
        }
        if (Test-Path $j) {
            Write-Output "    WARNING: reparse point still present after detach; aborting to avoid destructive recursion"
            exit 1
        }
    }
    Write-Output "  all reparse points detached"
}

# Now safe to call git worktree remove.
if ($Force) {
    Write-Output "  running: git worktree remove --force $WorktreePath"
    git worktree remove --force "$WorktreePath" 2>&1 | ForEach-Object { Write-Output "    $_" }
} else {
    Write-Output "  (no junctions to detach; -Force needed to actually call git worktree remove)"
    exit 0
}

Write-Output "safe_remove_worktree.ps1: done"
