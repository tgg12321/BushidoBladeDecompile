<#
.SYNOPSIS
  The manual-lane handshake: hand the repo from the Grinder to a hand-driven
  session and back again.

.DESCRIPTION
  Exists because the dangerous parts of a manual session are mechanical, and a
  checklist followed by hand is exactly the failure mode the project has
  recorded twice ([[grinder-clobbers-uncommitted-edits]],
  [[engine-queue-ops-revert-uncommitted-tree]]).

  `begin` stops the Grinder, WAITS for it to actually let go, refuses a dirty
  tree or a red oracle, pops the queue top, and prints the full context bundle.
  `end` asserts the tree is clean, banks the ledger, and relaunches.

  One writer on main, ever. See docs/superpowers/specs/2026-09-21-decomp-manual-design.md
  and the decomp-manual skill.

.EXAMPLE
  pwsh tools/manual_session.ps1 begin
  pwsh tools/manual_session.ps1 begin -Func func_8002C22C
  pwsh tools/manual_session.ps1 end -NoRelaunch
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet('begin', 'end', 'status')]
    [string]$Verb,

    # begin: work this function instead of the queue top.
    [string]$Func = '',

    # begin: print what would happen; touch nothing, stop nothing.
    [switch]$DryRun,

    # begin: proceed even if a prior session.json exists (clobbers it).
    [switch]$Force,

    # end: leave the Grinder down.
    [switch]$NoRelaunch,

    # begin: how long to wait for the Grinder to reach a session boundary.
    [int]$StopTimeoutMinutes = 45
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$Root = Split-Path -Parent $PSScriptRoot
$LockFile = Join-Path $Root 'tmp/grind/grind.lock'
$GrindLog = Join-Path $Root 'tmp/grind/grind.log'
$StateDir = Join-Path $Root 'tmp/manual'
$StateFile = Join-Path $StateDir 'session.json'
$Oracle = '62efab4f73f992798c43e8c730aa43baa10bb4fa'

function Say([string]$m, [string]$c = 'Gray') { Write-Host $m -ForegroundColor $c }
function Head([string]$m) { Write-Host "`n=== $m ===" -ForegroundColor Cyan }
function Die([string]$m) { Write-Host "[manual] $m" -ForegroundColor Red; exit 1 }

function Get-GrinderPid {
    if (-not (Test-Path $LockFile)) { return $null }
    $p = (Get-Content $LockFile -ErrorAction SilentlyContinue | Select-Object -First 1)
    if (-not $p) { return $null }
    if (-not (Get-Process -Id $p -ErrorAction SilentlyContinue)) { return $null }  # stale
    return [int]$p
}

# Tracked-file dirt only. memory/grind, docs/grind, tmp/ and metrics/events.jsonl
# are the Grinder's own working surfaces and are not "dirt" for our purposes --
# this mirrors grind.ps1's $AllowedDirtyPattern.
function Get-BlockingDirt {
    $lines = @(git -C $Root status --porcelain | Where-Object { $_ })
    return @($lines | Where-Object {
        $_ -notmatch '^(\?\?|.M|M.|A.|.A|D.|.D)\s+"?(memory/grind/|docs/grind/|tmp/|metrics/events\.jsonl)'
    })
}

function Invoke-Eng {
    param([string[]]$EngArgs)
    $wteng = Join-Path $Root 'tools/wteng.ps1'
    & $wteng main @EngArgs 2>&1 | Out-String
}

function Assert-OracleGreen {
    Head 'oracle'
    $out = Invoke-Eng @('verify-oracle')
    if ($out -match [regex]::Escape($Oracle)) { Say "oracle GREEN ($Oracle)" 'Green'; return }
    Write-Host $out
    Die "oracle is NOT green. That is an incident, not a manual-session start. Fix or revert to green first."
}

# ---------------------------------------------------------------- begin ------
function Invoke-Begin {
    if ((Test-Path $StateFile) -and -not $Force) {
        $prior = Get-Content $StateFile -Raw | ConvertFrom-Json
        Say "[manual] a prior manual session is still open:" 'Yellow'
        Say "         func=$($prior.func)  started=$($prior.started)  head=$($prior.start_head)"
        Say "         Resume it, or re-run with -Force to start over." 'Yellow'
        exit 2
    }

    # 1. Hand the repo over. Stop BEFORE reading any state: dossier/queue
    #    measure the tree, and a live driver is writing to it concurrently.
    $gpid = Get-GrinderPid
    $wasRunning = [bool]$gpid
    if ($wasRunning) {
        Head "grinder handoff (pid $gpid)"
        if ($DryRun) {
            Say '[dry-run] would stop the grinder and wait for the lock to clear.' 'Yellow'
        } else {
            & (Join-Path $Root 'tools/grinder/grind.ps1') -Stop | Out-Null
            Say 'STOP sentinel written; the driver exits at its next session boundary.'
            Say 'Not force-killing: that would lose the in-flight session ledger write.'
            $t0 = Get-Date
            while (Get-GrinderPid) {
                $mins = ((Get-Date) - $t0).TotalMinutes
                if ($mins -gt $StopTimeoutMinutes) {
                    Say "last grind.log line:" 'Yellow'
                    Get-Content $GrindLog -Tail 1 | ForEach-Object { Say "  $_" }
                    Die "grinder still holding the lock after $StopTimeoutMinutes min. Not proceeding (and not killing it)."
                }
                Say ("  waiting for session boundary... {0:n1} min" -f $mins)
                Start-Sleep -Seconds 20
            }
            Say ("grinder released the lock after {0:n1} min." -f ((Get-Date) - $t0).TotalMinutes) 'Green'
        }
    } else {
        Say '[manual] grinder not running.'
    }

    # 2. Never work over dirt.
    # @() re-wrap is load-bearing: PowerShell unrolls an empty array on
    # `return`, so Get-BlockingDirt yields $null on a clean tree and
    # $null.Count throws under StrictMode.
    $dirt = @(Get-BlockingDirt)
    if ($dirt.Count) {
        $dirt | ForEach-Object { Say "  $_" 'Yellow' }
        Die 'working tree has uncommitted tracked changes. Commit or revert before a manual session.'
    }
    Say '[manual] tree clean.' 'Green'

    if (-not $DryRun) { Assert-OracleGreen }

    # 3. Target.
    $target = $Func
    $stem = ''
    Head 'queue top'
    $qn = Invoke-Eng @('queue', 'next')
    Write-Host $qn
    $lo = $qn.IndexOf('{'); $hi = $qn.LastIndexOf('}')
    if ($lo -ge 0 -and $hi -gt $lo) {
        $item = $qn.Substring($lo, $hi - $lo + 1) | ConvertFrom-Json
        # StrictMode throws on a missing property, so probe before reading.
        $have = $item.PSObject.Properties.Name
        if (-not $target -and $have -contains 'func') { $target = [string]$item.func }
        if ($have -contains 'file') { $stem = [string]$item.file }
    }
    if (-not $target) { Die 'could not determine a target function (queue next gave nothing, and no -Func).' }
    Say "[manual] target: $target" 'Green'

    if ($DryRun) { Say '[dry-run] would print the context bundle and write tmp/manual/session.json.' 'Yellow'; return }

    # 4. The context bundle -- the whole point of the lane. Assembled
    #    mechanically because hand-gathering misses different pieces each time
    #    (owner directive 2026-08-24).
    Head "dossier — $target"
    Write-Host (Invoke-Eng @('dossier', $target))

    Head "sibling ledgers — $target"
    # Same invocation shape as tools/grinder/status.ps1:49 — Windows-side python,
    # absolute root. A sibling floor DROP is the single highest-value thing to
    # read before choosing a lever (2026-09-04 post-mortem: CD_datasync sat 41
    # sessions while its coupled sibling already held the fix).
    $sib = (python (Join-Path $Root 'tools\grinder\grindlib.py') siblings $Root $target 2>&1 | Out-String)
    Write-Host $sib

    # Sony-library provenance (published reference C, banked closer material).
    # Empty for game code. CD_cw 2026-09-22: without it the session re-derived
    # the SOTN structure by hand across ~15 variants before fetching the reference.
    $psyq = (python (Join-Path $Root 'tools\grinder\grindlib.py') psyq $Root $target 2>&1 | Out-String)
    if ($psyq.Trim()) {
        Head "Sony library provenance — $target"
        Write-Host $psyq
    }

    # Ledger files that define some OTHER function are pre-rename leftovers and
    # mislead a cold reader (CD_cw's pre-include-asm-body.c was a placeholder
    # for its old name tslTm2LoadImage). Flag, never delete.
    $ledgerDir = Join-Path $Root "memory/grind/$target"
    if (Test-Path $ledgerDir) {
        $stale = @(Get-ChildItem $ledgerDir -Filter *.c -File | Where-Object {
            $t = Get-Content $_.FullName -Raw
            $t -and ($t -notmatch "\b$([regex]::Escape($target))\b")
        })
        foreach ($f in $stale) {
            Say "[manual] NOTE: memory/grind/$target/$($f.Name) never mentions $target -- likely a pre-rename leftover; don't trust it." 'Yellow'
        }
    }

    Head "canonical route — $target"
    Write-Host (Invoke-Eng @('canonical', $target))

    # main carries INCLUDE_ASM for an incomplete function, so a bare sandbox
    # only measures the function's size. Score the banked candidate instead.
    $candRel = "memory/grind/$target/candidate.c"
    $sbArgs = @('sandbox', $target, '--disable', 'all', '--diff')
    if (Test-Path (Join-Path $Root $candRel)) { $sbArgs += @('--candidate', $candRel) }
    Head "honest floor + where it differs — $target"
    Write-Host (Invoke-Eng $sbArgs)

    # 5. Session state, so an interrupted session is resumable.
    New-Item -ItemType Directory -Force -Path $StateDir | Out-Null
    @{
        func                 = $target
        file                 = $stem
        grinder_was_running  = $wasRunning
        start_head           = (git -C $Root rev-parse HEAD)
        started              = (Get-Date).ToString('o')
    } | ConvertTo-Json | Set-Content $StateFile -Encoding utf8

    Head 'ready'
    Say "Working $target by hand. Iterate on $candRel and score with" 'Green'
    Say "  & tools/wteng.ps1 main sandbox $target --disable all --diff --candidate $candRel" 'Green'
    Say "  (--candidate is REQUIRED: without it sandbox scores the INCLUDE_ASM stub.)" 'Green'
    Say "Several variants at once, scored hunks only:" 'Green'
    Say "  pwsh tools/sandbox_sweep.ps1 -Func $target -Variants a.c,b.c [-Hunks]" 'Green'
    Say 'The tree stays clean while you do -- that is what keeps engine commands from reverting your work.'
    Say 'When done (matched or banked): pwsh tools/manual_session.ps1 end'
}

# ------------------------------------------------------------------ end ------
function Invoke-End {
    if (-not (Test-Path $StateFile)) { Die 'no open manual session (tmp/manual/session.json missing).' }
    $s = Get-Content $StateFile -Raw | ConvertFrom-Json
    Head "closing manual session — $($s.func)"

    $dirt = @(Get-BlockingDirt)
    if ($dirt.Count) {
        $dirt | ForEach-Object { Say "  $_" 'Yellow' }
        Die 'tree still has uncommitted tracked changes. Commit the match, or revert to INCLUDE_ASM, then re-run end. (Relaunching over dirt makes the Grinder discard its next session.)'
    }
    Say '[manual] tree clean.' 'Green'

    # Bank any ledger movement so the Grinder resumes informed. Explicit
    # pathspecs on the commit: `git commit` takes the WHOLE index otherwise.
    $ledgerPaths = @("memory/grind/$($s.func)", 'docs/grind')
    $ledgerDirt = @(git -C $Root status --porcelain -- @ledgerPaths | Where-Object { $_ })
    if ($ledgerDirt.Count) {
        git -C $Root add -- @ledgerPaths 2>$null
        git -C $Root commit -q -m "grind: $($s.func) — manual session ledger update

Banked by tools/manual_session.ps1 end so the Grinder resumes from what the
manual lane learned. See the decomp-manual skill." -- @ledgerPaths 2>$null | Out-Null
        Say '[manual] ledger banked.' 'Green'
    }

    # Engine events accumulate in metrics/events.jsonl during the session; the
    # convention is a separate metrics: capture commit (see git log).
    if (@(git -C $Root status --porcelain -- metrics/events.jsonl | Where-Object { $_ }).Count) {
        git -C $Root commit -q -m "metrics: capture from $($s.func) manual session" -- metrics/events.jsonl 2>$null | Out-Null
        Say '[manual] metrics captured.' 'Green'
    }

    # Listed AFTER the banking commits so the report includes them.
    $range = "$($s.start_head)..HEAD"
    Head "commits this session ($range)"
    $commits = @(git -C $Root log --oneline $range)
    if ($commits.Count) { $commits | ForEach-Object { Say "  $_" } } else { Say '  (none)' }

    Remove-Item $StateFile -Force

    if ($NoRelaunch) { Say '[manual] -NoRelaunch: leaving the Grinder down.' 'Yellow'; return }
    if (-not $s.grinder_was_running) { Say '[manual] Grinder was not running at begin; not starting it.' 'Yellow'; return }

    # Re-drill only if the session touched the machinery the drills cover.
    $touched = @(git -C $Root diff --name-only $range)
    $needDrill = @($touched | Where-Object { $_ -match '^(tools/grinder/|engine/|tools/hooks/)' }).Count -gt 0
    if ($needDrill) {
        Head 'grinder machinery changed — re-drilling before relaunch'
        & (Join-Path $Root 'tools/grinder/drill.ps1')
        if ($LASTEXITCODE -ne 0) { Die 'drill NO-GO. Not relaunching; investigate first.' }
    }

    Head 'relaunching grinder'
    Start-Process pwsh -WindowStyle Hidden -ArgumentList '-NoProfile', '-File', "`"$Root\tools\grinder\grind.ps1`""
    Start-Sleep -Seconds 12
    & (Join-Path $Root 'tools/grinder/status.ps1')
}

# --------------------------------------------------------------- status ------
function Invoke-Status {
    $gpid = Get-GrinderPid
    Say ("grinder: " + $(if ($gpid) { "RUNNING (pid $gpid)" } else { 'stopped' }))
    if (Test-Path $StateFile) {
        $s = Get-Content $StateFile -Raw | ConvertFrom-Json
        Say "manual session OPEN: func=$($s.func) started=$($s.started) head=$($s.start_head)"
    } else {
        Say 'manual session: none open'
    }
    $dirt = @(Get-BlockingDirt)
    Say ("blocking dirt: " + $(if ($dirt.Count) { "$($dirt.Count) path(s)" } else { 'none' }))
}

switch ($Verb) {
    'begin'  { Invoke-Begin }
    'end'    { Invoke-End }
    'status' { Invoke-Status }
}
