#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Safely reap ORPHANED agent-harness processes (Claude / Codex / Omnara / BB2-fleet
  leaks) that linger after an agent run ends and eat RAM.

.DESCRIPTION
  DEFAULT-DENY. A process is a kill target ONLY if it matches an explicit positive
  orphan signature below. "old", "idle", or "not interactive" are NEVER sufficient on
  their own. Anything unmatched is left strictly alone.

  Classification is by COMMAND LINE + ANCESTRY, never by image name. (On this host every
  claude.exe — including the LIVE session — runs off a renamed `claude.exe.old.<ts>`
  binary after an auto-update, so the image name is useless as a signal and dangerous to
  key on.)

  Orphan classes:
    omnara-headless-claude  claude `--output-format stream-json` under omnara.exe
    codex-headless-claude   headless claude under codex.exe
    codex-headless-shell    pwsh `-NonInteractive -EncodedCommand` under codex.exe
    fleet-lane / -supervisor / -agent
                            fleet pwsh/claude NOT under a live supervisor (orphanhood is
                            enforced structurally: a live supervisor's whole subtree is in
                            the protect-set, so only genuinely-detached fleet procs qualify)
    orphan-wsl-host         (-IncludeWslHosts) wsl.exe relay whose launcher is gone

  SAFETY (each rule below was demanded by an adversarial safety review; see git history):
    * The protect-set is computed DYNAMICALLY from $PID every run — never a hardcoded PID.
      It covers: $PID, the full ancestor chain, the live-claude session root's ENTIRE
      subtree (tool-call shells churn as siblings), every interactive `claude
      --dangerously-skip-permissions` session's subtree, the omnara/codex launchers
      themselves, and a live fleet supervisor.pid subtree.
    * Interactive Claude sessions are NEVER swept by a generic flag. The only way to kill
      one is to name its PID explicitly: -KillInteractivePids <pid>.
    * DRY-RUN is the default. Actual termination requires -Execute.
    * Kills are leaves-first, snapshot-bounded, with a per-PID CreationDate (PID-recycle)
      TOCTOU guard re-checked against a FRESH snapshot immediately before each kill. No
      `taskkill /T` (it walks live links and can kill processes the reaper never classified).
    * The omnara.exe / codex.exe LAUNCHERS are never killed — only their orphaned children.

.EXAMPLE
  pwsh tools/reap_orphans.ps1                       # dry-run: show what WOULD be reaped
  pwsh tools/reap_orphans.ps1 -Execute             # reap omnara+codex+fleet orphans
  pwsh tools/reap_orphans.ps1 -Harness codex -Execute
  pwsh tools/reap_orphans.ps1 -MinAgeHours 6 -Execute
  pwsh tools/reap_orphans.ps1 -KillInteractivePids 44276 -Execute   # only way to kill an interactive session
#>
[CmdletBinding()]
param(
    [switch]$Execute,
    [ValidateSet('omnara', 'codex', 'fleet')][string[]]$Harness = @('omnara', 'codex', 'fleet'),
    [double]$MinAgeHours = 2,
    [double]$MaxCpuPerHour = 60,
    [int[]]$KillInteractivePids = @(),
    [switch]$IncludeWslHosts,
    [switch]$Json
)
$ErrorActionPreference = 'Continue'
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$Now = Get-Date

# ----------------------------------------------------------------------------- snapshot
function Get-Snapshot {
    $procs = Get-CimInstance Win32_Process
    $byId = @{}; $kids = @{}
    foreach ($p in $procs) {
        $byId[[int]$p.ProcessId] = $p
        $pp = [int]$p.ParentProcessId
        if (-not $kids.ContainsKey($pp)) { $kids[$pp] = New-Object System.Collections.Generic.List[int] }
        $kids[$pp].Add([int]$p.ProcessId)
    }
    return @{ byId = $byId; kids = $kids }
}
function Get-Descendants($snap, [int]$rootPid) {
    # BFS, cycle-guarded. Returns descendants nearest-first (reverse for leaves-first kill).
    $out = New-Object System.Collections.Generic.List[int]
    $seen = @{}; $q = New-Object System.Collections.Generic.Queue[int]
    $q.Enqueue($rootPid)
    while ($q.Count) {
        $n = $q.Dequeue()
        if ($snap.kids.ContainsKey($n)) {
            foreach ($c in $snap.kids[$n]) {
                if (-not $seen.ContainsKey($c)) { $seen[$c] = $true; $out.Add($c); $q.Enqueue($c) }
            }
        }
    }
    return $out
}
function Get-Ancestors($snap, [int]$startPid) {
    $out = New-Object System.Collections.Generic.List[int]
    $seen = @{}; $cur = $startPid
    while ($cur -and $snap.byId.ContainsKey($cur) -and -not $seen.ContainsKey($cur)) {
        $seen[$cur] = $true
        $par = [int]$snap.byId[$cur].ParentProcessId
        if ($par -and $snap.byId.ContainsKey($par) -and -not $seen.ContainsKey($par)) { $out.Add($par); $cur = $par }
        else { break }
    }
    return $out
}

# -------------------------------------------------------------- per-process classifiers
function Test-IsClaude($p) {
    return (([string]$p.Name) -match '^claude') -or (([string]$p.CommandLine) -match '[\\/ ]claude(\.exe)?[\s"]') -or (([string]$p.CommandLine) -match '^claude\s')
}
function Test-IsHeadlessClaude([string]$cmd) {
    return ($cmd -match '--output-format\s+stream-json') -or (($cmd -match '(^|\s)(-p|--print)(\s|$)') -and ($cmd -match '--session-id'))
}
function Test-IsInteractiveClaude($p) {
    $cmd = [string]$p.CommandLine
    return (Test-IsClaude $p) -and ($cmd -match '--dangerously-skip-permissions') -and -not (Test-IsHeadlessClaude $cmd)
}

# --------------------------------------------------------------- dynamic protect-set
function Build-ProtectSet($snap, [int]$selfPid, [int[]]$killInteractive) {
    $protect = @{}
    function _tree($p) { $protect[[int]$p] = $true; foreach ($d in (Get-Descendants $snap ([int]$p))) { $protect[[int]$d] = $true } }

    # 1. self + full ancestor chain
    $protect[$selfPid] = $true
    $anc = Get-Ancestors $snap $selfPid
    foreach ($a in $anc) { $protect[[int]$a] = $true }
    # 2. live-claude session ROOT (highest claude on the chain) — protect its whole subtree
    $sessionRoot = $selfPid
    foreach ($a in (@($selfPid) + $anc)) { if ($snap.byId.ContainsKey($a) -and ($snap.byId[$a].Name -match '^claude')) { $sessionRoot = $a } }
    _tree $sessionRoot
    _tree $selfPid
    # 3. belt: EVERY interactive claude session subtree (covers a 2nd live session), unless named for kill
    foreach ($p in $snap.byId.Values) {
        if ((Test-IsInteractiveClaude $p) -and ($killInteractive -notcontains [int]$p.ProcessId)) { _tree ([int]$p.ProcessId) }
    }
    # 4. never kill the harness LAUNCHERS themselves
    foreach ($p in $snap.byId.Values) { if ((($p.Name) -replace '\.exe$', '') -in 'omnara', 'codex') { $protect[[int]$p.ProcessId] = $true } }
    # 5. a live fleet supervisor + its subtree
    $supPid = Join-Path $RepoRoot 'tmp/fleet/supervisor.pid'
    if (Test-Path $supPid) {
        $sp = 0; [void][int]::TryParse(((Get-Content $supPid -Raw -ErrorAction SilentlyContinue)).Trim(), [ref]$sp)
        if ($sp -and $snap.byId.ContainsKey($sp)) { _tree $sp }
    }
    return $protect
}

# --------------------------------------------------------------------- classify (snap1)
$snap = Get-Snapshot
$protect = Build-ProtectSet $snap $PID $KillInteractivePids
$cands = @()
foreach ($p in $snap.byId.Values) {
    $procId = [int]$p.ProcessId
    if ($protect.ContainsKey($procId)) { continue }
    $cmd = [string]$p.CommandLine
    if (-not $cmd) { continue }
    $isClaude = Test-IsClaude $p
    $isPwsh = ($p.Name) -match '^(pwsh|powershell)'
    $isWsl = ($p.Name) -match '^wsl'
    $isHeadless = Test-IsHeadlessClaude $cmd
    $ancNames = ((Get-Ancestors $snap $procId) | ForEach-Object { $snap.byId[$_].Name }) -join ';'
    $hasOmnara = $ancNames -match 'omnara'
    $hasCodex = $ancNames -match 'codex'

    $kind = $null; $harnessOf = $null; $named = $false
    if ($isClaude -and $isHeadless -and $hasOmnara) { $kind = 'omnara-headless-claude'; $harnessOf = 'omnara' }
    elseif ($isClaude -and $isHeadless -and $hasCodex) { $kind = 'codex-headless-claude'; $harnessOf = 'codex' }
    elseif ($isPwsh -and ($cmd -match '-NonInteractive') -and ($cmd -match '-EncodedCommand') -and $hasCodex) { $kind = 'codex-headless-shell'; $harnessOf = 'codex' }
    elseif ($isPwsh -and ($cmd -match '-File\s+["'']?\S*fleet[\\/]lane\.ps1')) { $kind = 'fleet-lane'; $harnessOf = 'fleet' }
    elseif ($isPwsh -and ($cmd -match '-File\s+["'']?\S*fleet[\\/]fleet\.ps1')) { $kind = 'fleet-supervisor'; $harnessOf = 'fleet' }
    elseif ($isClaude -and $isHeadless -and ($cmd -match 'roles[\\/]\w+\.md')) { $kind = 'fleet-agent'; $harnessOf = 'fleet' }
    elseif ((Test-IsInteractiveClaude $p) -and ($KillInteractivePids -contains $procId)) { $kind = 'interactive-claude(named)'; $harnessOf = 'interactive'; $named = $true }
    elseif ($isWsl -and $IncludeWslHosts -and -not $snap.byId.ContainsKey([int]$p.ParentProcessId)) { $kind = 'orphan-wsl-host'; $harnessOf = 'fleet' }
    else { continue }

    if (($harnessOf -in 'omnara', 'codex', 'fleet') -and ($Harness -notcontains $harnessOf)) { continue }

    $ageH = ($Now - $p.CreationDate).TotalHours
    $cpuSec = ([double]$p.KernelModeTime + [double]$p.UserModeTime) / 1e7
    $cpuH = if ($ageH -gt 0) { $cpuSec / $ageH } else { 0 }
    if (-not $named) {
        if ($ageH -lt $MinAgeHours) { continue }       # age gate (defense in depth)
        if ($cpuH -gt $MaxCpuPerHour) { continue }      # idle gate (spare recently-active)
    }
    $cands += [pscustomobject]@{
        PID = $procId; Kind = $kind; Harness = $harnessOf; Name = $p.Name
        RAM_MB = [math]::Round($p.WorkingSetSize / 1MB, 1); AgeH = [math]::Round($ageH, 1)
        CpuPerH = [math]::Round($cpuH, 1); CreationDate = $p.CreationDate
    }
}

# ------------------------------------------------------------------------------- report
$mode = if ($Execute) { 'EXECUTE' } else { 'DRY-RUN (no -Execute; nothing killed)' }
if (-not $Json) {
    Write-Host "[reap_orphans] mode=$mode  harness=$($Harness -join ',')  MinAgeHours=$MinAgeHours  MaxCpuPerHour=$MaxCpuPerHour"
    if (-not $cands) { Write-Host "  no orphan candidates." }
    else {
        $cands | Sort-Object RAM_MB -Descending | Format-Table PID, Kind, Harness, Name, RAM_MB, AgeH, CpuPerH -Auto | Out-String | Write-Host
        $direct = ($cands | Measure-Object RAM_MB -Sum).Sum
        Write-Host ("  {0} candidate root(s); ~{1} MB direct (subtrees reclaim more)." -f $cands.Count, [math]::Round($direct, 0))
        if (-not $Execute) { Write-Host "  re-run with -Execute to reap. Interactive sessions require -KillInteractivePids <pid>." }
    }
}

# -------------------------------------------------------------------------------- reap
$killedList = @(); $reclaim = 0.0
if ($Execute -and $cands) {
    $snap2 = Get-Snapshot
    $protect2 = Build-ProtectSet $snap2 $PID $KillInteractivePids   # FRESH protect-set
    foreach ($c in $cands) {
        # subtree leaves-first, from the FRESH snapshot
        $subtree = New-Object System.Collections.Generic.List[int]
        foreach ($d in (Get-Descendants $snap2 $c.PID)) { $subtree.Add($d) }
        $subtree.Reverse()                 # BFS nearest-first reversed -> leaves-first-ish
        $subtree.Add($c.PID)               # root last
        foreach ($k in $subtree) {
            if ($protect2.ContainsKey($k)) { continue }
            $live = $snap2.byId[$k]
            if (-not $live) { continue }
            $gp = Get-Process -Id $k -ErrorAction SilentlyContinue
            if (-not $gp) { continue }
            $st = $null; try { $st = $gp.StartTime } catch {}
            # PID-recycle TOCTOU guard: live StartTime must match the snapshot CreationDate
            if ($st -and $live.CreationDate -and ([math]::Abs(($st - $live.CreationDate).TotalSeconds) -gt 2)) { continue }
            $ram = [math]::Round($live.WorkingSetSize / 1MB, 1)
            try {
                Stop-Process -Id $k -Force -ErrorAction Stop
                $killedList += [pscustomobject]@{ PID = $k; Name = $live.Name; RAM_MB = $ram }
                $reclaim += $ram
            }
            catch {}
        }
    }
    if (-not $Json) {
        Write-Host ""
        if ($killedList) {
            $killedList | Format-Table PID, Name, RAM_MB -Auto | Out-String | Write-Host
            Write-Host ("  reaped {0} process(es); reclaimed ~{1} MB." -f $killedList.Count, [math]::Round($reclaim, 0))
        }
        else { Write-Host "  nothing reaped (all candidates vanished or were protected on re-check)." }
    }
}

if ($Json) {
    [pscustomobject]@{
        mode = $mode; harness = $Harness; minAgeHours = $MinAgeHours; maxCpuPerHour = $MaxCpuPerHour
        candidates = $cands; killed = $killedList; reclaimedMB = [math]::Round($reclaim, 1)
    } | ConvertTo-Json -Depth 6
}
