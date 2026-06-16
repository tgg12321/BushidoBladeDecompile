# tools/fleet/_fleet_common.ps1 — shared helpers for the Autonomous Decomp Fleet.
# Dot-sourced by lane.ps1 and fleet.ps1. No top-level side effects.

$script:FleetDir   = $PSScriptRoot
$script:RepoRoot   = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$script:CoordPs1   = Join-Path $PSScriptRoot 'coord.ps1'
$script:RolesDir   = Join-Path $PSScriptRoot 'roles'
$script:WtEng      = Join-Path $RepoRoot 'tools\wteng.ps1'
$script:OutcomeDir = Join-Path $RepoRoot 'tmp\fleet\outcomes'
if (-not (Test-Path $OutcomeDir)) { New-Item -ItemType Directory -Path $OutcomeDir -Force | Out-Null }

function Get-MainRoot {
    # The main repo = first entry of `git worktree list`. cwd-independent.
    foreach ($line in (git -C "$script:RepoRoot" worktree list --porcelain 2>$null)) {
        if ($line -match '^worktree (.+)$') { return ($Matches[1] -replace '/', '\') }
    }
    return $script:RepoRoot
}

function Get-WorktreeDir([string]$Id) {
    # All fleet worktrees live in a single container dir beside main
    # (`..\bb2-worktrees\bb2-work-<id>`) so the desktop stays uncluttered
    # (2026-06-15 — previously every worktree was a bare desktop sibling).
    # Ensures the container exists. Resolution by `wteng`/guards is by the
    # `bb2-work-<id>` LEAF, so the deeper nesting is transparent to them.
    $container = Join-Path (Split-Path (Get-MainRoot)) 'bb2-worktrees'
    if (-not (Test-Path $container)) { New-Item -ItemType Directory -Path $container -Force | Out-Null }
    return (Join-Path $container "bb2-work-$Id")
}

function Coord {
    # paramless: $args captures every token verbatim (incl. -Set/-Reason/...) so they
    # pass through to coord.ps1's own parameters instead of binding to this wrapper.
    & $script:CoordPs1 @args
}

function Read-Outcome([string]$Path) {
    if (-not (Test-Path $Path)) { return $null }
    try { return (Get-Content $Path -Raw -Encoding utf8 | ConvertFrom-Json -AsHashtable) }
    catch { return $null }
}

function Test-OracleRebuild {
    # Authoritative: full rebuild of main's build/ + SHA1 == oracle. Returns [bool].
    # B8c: distinguish a real SHA mismatch (build_matches:false) from a transient
    # build-infra hiccup (no parseable JSON — WSL/.venv flake). Retry the infra-flake
    # case a few times so a flaky toolchain can't falsely trip the circuit; a genuine
    # SHA mismatch returns false immediately.
    for ($attempt = 1; $attempt -le 3; $attempt++) {
        $raw = $null
        try { $raw = & $script:WtEng main verify-oracle --rebuild 2>&1 | Out-String } catch { $raw = '' }
        if ($raw -match '"build_matches"\s*:\s*true')  { return $true }
        if ($raw -match '"build_matches"\s*:\s*false') { return $false }   # real mismatch
        Start-Sleep -Seconds 5   # no verdict parsed -> infra flake; retry
    }
    return $false   # repeated infra failure -> treat as not-green (safe)
}

# Paths that are fleet-operational scratch / records, NOT source contamination, and
# must therefore be ignored by every dirty/clean check (otherwise no merge could pass
# the post-commit clean assertion, and Assert-MainClean would thrash on them):
#   - metrics/events.jsonl : append-only engine telemetry (best-effort, every command)
#   - docs/fleet/*         : the fleet's own logs/ledgers (committed periodically)
#   - memory/wip/*         : the shared WIP-checkpoint scratch (cwd=main on purpose, so
#                            checkpoints persist across worktree resets — by design)
$script:TelemetryPaths = @('metrics/events.jsonl')
$script:IgnorePrefixes = @('docs/fleet/', 'memory/wip/')

function Get-MainDirtyLines {
    # porcelain lines for main, EXCLUDING fleet-operational scratch/records. What
    # remains is genuine source/config contamination worth acting on.
    $main = Get-MainRoot
    $lines = @(git -C "$main" status --porcelain | Where-Object { $_ })
    return @($lines | Where-Object {
        $p = $_.Substring(3).Trim().Trim('"')
        $ignore = ($script:TelemetryPaths -contains $p)
        foreach ($pre in $script:IgnorePrefixes) { if ($p.StartsWith($pre)) { $ignore = $true } }
        -not $ignore
    })
}

function Clean-MainContamination([object[]]$DirtyLines) {
    # Surgically revert ONLY the contaminated paths (porcelain lines from
    # Get-MainDirtyLines, which already excludes fleet-operational scratch). Do NOT
    # `git checkout -- .` — that would also revert uncommitted docs/fleet findings.
    $main = Get-MainRoot
    foreach ($line in $DirtyLines) {
        if ($line.Length -lt 4) { continue }
        $code = $line.Substring(0, 2)
        $path = $line.Substring(3).Trim().Trim('"')
        if ($path -match ' -> ') { $path = ($path -split ' -> ')[-1].Trim().Trim('"') }  # rename
        if ($code -match '\?') { git -C "$main" clean -fdq -- "$path" 2>&1 | Out-Null }   # untracked
        else                   { git -C "$main" checkout -- "$path" 2>&1 | Out-Null }     # tracked
    }
}

function Assert-MainClean {
    # Maj-8: agents run with bypassPermissions and cwd=main; they must only edit their
    # worktrees. If an agent dirtied main's tracked tree (beyond fleet-operational
    # scratch), surgically discard just those paths (agents never legitimately edit
    # main) and report it. Returns $true if clean.
    $dirty = Get-MainDirtyLines
    if ($dirty.Count -eq 0) { return $true }
    Clean-MainContamination $dirty
    Append-FleetLog 'main-contamination-cleaned' @{ detail = ($dirty | Select-Object -First 8) }
    return $false
}

function Test-OracleQuick {
    # Cheap backstop: relink existing build/ objects + SHA1 check (no recompile).
    try {
        $raw = & $script:WtEng main verify-oracle 2>&1 | Out-String
        return ($raw -match '"build_matches"\s*:\s*true')
    } catch { return $false }
}

function Get-MainHead { (git -C (Get-MainRoot) rev-parse HEAD 2>$null | Out-String).Trim() }

function Invoke-WtEng {
    # Run a worktree-pinned engine command and parse the first JSON object out of
    # its output (wteng's own [wteng] banner is Write-Host, not captured). Returns
    # the parsed object or $null. Paramless so --flags pass through verbatim.
    $a = @($args)
    $target = $a[0]
    $rest = if ($a.Count -gt 1) { $a[1..($a.Count - 1)] } else { @() }
    $raw = & $script:WtEng $target @rest 2>&1 | Out-String
    $m = [regex]::Match($raw, '(?s)\{.*\}')
    if ($m.Success) { try { return ($m.Value | ConvertFrom-Json -AsHashtable) } catch { } }
    return $null
}

function Invoke-RoleAgent {
    <#
      Spawn one role agent (claude -p) with the role's system prompt appended,
      wait for it, and return its parsed outcome hashtable (or $null if it wrote
      no/invalid outcome file). cwd is the MAIN repo (matches the decomp-work
      contract: agents edit ..\bb2-worktrees\bb2-work-<id> via explicit paths + wteng <id>).
    #>
    param(
        [string]$Role,
        [string]$Lane,
        [string]$TaskText,
        [string]$Model = 'opus',
        [int]$TimeoutMinutes = 90
    )
    $rolePath    = Join-Path $script:RolesDir "$Role.md"
    if (-not (Test-Path $rolePath)) { throw "Invoke-RoleAgent: no role file $rolePath" }
    $outcomePath = Join-Path $script:OutcomeDir "$Lane.outcome.json"
    Remove-Item $outcomePath -ErrorAction SilentlyContinue

    # The task tells the agent exactly where to write its outcome.
    $fullTask = $TaskText + "`n`nWhen finished, write your outcome JSON to this exact absolute path (overwrite it):`n  $outcomePath`n"

    $sid = [guid]::NewGuid().ToString()
    $claudeArgs = @(
        '-p', $fullTask,
        '--append-system-prompt-file', $rolePath,
        '--permission-mode', 'bypassPermissions',
        '--model', $Model,
        '--session-id', $sid,
        '--output-format', 'json'
    )
    $env:CLAUDE_SESSION_ID = $sid
    try {
        # $null stdin -> skip the 3s "no stdin" wait. Out-String drains stdout.
        $null = ($null | & claude @claudeArgs | Out-String)
    } finally {
        Remove-Item Env:\CLAUDE_SESSION_ID -ErrorAction SilentlyContinue
    }
    return (Read-Outcome $outcomePath)
}

function Ensure-Worktree([string]$Id) {
    # Create the worktree if absent and bootstrap it (private build/, junctioned
    # deps, HEAD-sync to main). Idempotent. Returns the worktree path.
    $main = Get-MainRoot
    $wt = Get-WorktreeDir $Id
    if (-not (Test-Path $wt)) {
        $branch = "work/$Id"
        $head = Get-MainHead
        # reuse the branch if it already exists, else create it
        $exists = (git -C "$main" rev-parse --verify --quiet "refs/heads/$branch" 2>$null)
        if ($exists) { git -C "$main" worktree add "$wt" "$branch" 2>&1 | Out-Null }
        else         { git -C "$main" worktree add -b "$branch" "$wt" "$head" 2>&1 | Out-Null }
    }
    # bootstrap / HEAD-sync (run from inside the worktree)
    Push-Location $wt
    try {
        $env:WORKTREE_STALE_POLICY = 'sync'
        & (Join-Path $wt 'tools\setup_worker_worktree.ps1') 2>&1 | Out-Null
    } finally {
        Remove-Item Env:\WORKTREE_STALE_POLICY -ErrorAction SilentlyContinue
        Pop-Location
    }
    return $wt
}

function Append-FleetLog([string]$Kind, [hashtable]$Data) {
    # Durable, human-reviewable event log (committed periodically by the supervisor).
    $logDir = Join-Path $script:RepoRoot 'docs\fleet'
    if (-not (Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir -Force | Out-Null }
    $rec = [ordered]@{ ts = (Get-Date).ToUniversalTime().ToString('o'); kind = $Kind }
    foreach ($k in $Data.Keys) { $rec[$k] = $Data[$k] }
    ($rec | ConvertTo-Json -Compress -Depth 8) | Add-Content -Path (Join-Path $logDir 'log.jsonl') -Encoding utf8
}
