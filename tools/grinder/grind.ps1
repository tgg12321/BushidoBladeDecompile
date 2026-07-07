<#
.SYNOPSIS
  The Grinder — single-lane walk-away decomp driver.
  Spec: docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md
.DESCRIPTION
  Deterministic loop: queue top -> ensure ledger -> assemble brief (mandated
  modality) -> spawn ONE grind session -> validate outcome -> apply/revert ->
  candidate path (verify bytes THEN judge) -> repeat. No 'blocked' outcome
  exists; invalid sessions are discarded and respawned. Stop: create tmp/grind/STOP
  or run stop via -Stop.
.PARAMETER Once
  Run exactly one iteration (drills, supervised runs).
.PARAMETER MockSessionScript
  Path to a .ps1 executed INSTEAD of claude for the grind session (drills).
  It receives env GRIND_BRIEF_PATH and GRIND_OUTCOME_PATH.
.PARAMETER MockJudgeScript
  Same, for the judge.
#>
[CmdletBinding()]
param(
    [switch]$Once,
    [switch]$Stop,
    [string]$Model = 'opus',
    [string]$JudgeModel = 'opus',
    [int]$SessionTimeoutMin = 90,
    [string]$MockSessionScript = '',
    [string]$MockJudgeScript = ''
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)   # repo root
Set-Location $Root
$GrindTmp = Join-Path $Root 'tmp\grind'
$PidFile  = Join-Path $GrindTmp 'grind.lock'
$StopFile = Join-Path $GrindTmp 'STOP'
$Journal  = Join-Path $Root 'docs\grind\journal.md'
$Decisions = Join-Path $Root 'docs\grind\decisions.md'
$RolesDir = Join-Path $PSScriptRoot 'roles'
New-Item -ItemType Directory -Force $GrindTmp, (Join-Path $Root 'docs\grind') | Out-Null

function Log([string]$msg) {
    $line = "[grind $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')] $msg"
    Write-Host $line
    Add-Content -Path (Join-Path $GrindTmp 'grind.log') -Value $line
}
function Journal([string]$msg) {
    Add-Content -Path $Journal -Value "- $(Get-Date -Format 'yyyy-MM-dd HH:mm') $msg"
}

if ($Stop) {
    New-Item -ItemType File -Force $StopFile | Out-Null
    Log "STOP sentinel written; driver exits at next session boundary."
    exit 0
}

# ── single-instance lock with stale reclaim ──────────────────────────────────
if (Test-Path $PidFile) {
    $oldPid = Get-Content $PidFile -ErrorAction SilentlyContinue
    if ($oldPid -and (Get-Process -Id $oldPid -ErrorAction SilentlyContinue)) {
        Log "Another grinder is running (pid $oldPid). Exiting."
        exit 1
    }
    Log "Reclaiming stale lock (dead pid $oldPid)."
}
$PID | Set-Content $PidFile
Remove-Item $StopFile -ErrorAction SilentlyContinue

function Invoke-Eng([string[]]$CmdArgs) {
    # engine command on main; returns stdout text (throws on wteng failure)
    (& (Join-Path $Root 'tools\wteng.ps1') 'main' @CmdArgs 2>&1 | Out-String)
}
function Test-OracleGreen {
    try { Invoke-Eng @('verify-oracle') | Out-Null; return $LASTEXITCODE -eq 0 }
    catch { return $false }
}
function Get-QueueTop {
    # `queue next` prints a single pretty-printed JSON object (with a nested
    # "wip" key when a checkpoint exists), preceded by wteng's [wteng] banner.
    # Robust parse: take the substring from the first '{' to the last '}' and
    # ConvertFrom-Json. Returns $null if there is no object or it won't parse.
    $raw = Invoke-Eng @('queue', 'next')
    $lo = $raw.IndexOf('{'); $hi = $raw.LastIndexOf('}')
    if ($lo -lt 0 -or $hi -le $lo) { return $null }
    try { return ($raw.Substring($lo, $hi - $lo + 1) | ConvertFrom-Json) }
    catch { return $null }
}
function Assert-CleanTree {
    $dirty = @(git -C $Root status --porcelain | Where-Object { $_ })
    return $dirty
}

Log "grinder starting (pid $PID, model $Model, judge $JudgeModel)"
if (-not (Test-OracleGreen)) {
    Log "PRE-FLIGHT FAIL: oracle not green on main. Fix before grinding."
    Remove-Item $PidFile; exit 2
}
Log "pre-flight: oracle green."

function Circuit-Break([string]$Reason) { Log "CIRCUIT-BREAK: $Reason"; Remove-Item $PidFile -EA SilentlyContinue; exit 3 }
function Invoke-JudgeRuling([string]$f, [string]$q) { Log "judge ruling stub: $f — $q" }
function Invoke-CandidatePath([string]$f, [string]$s, [string]$m, $o) { Log "candidate path stub: $f" }

# ── session spawn (pattern from tools/fleet/_fleet_common.ps1:132-170) ────────
function Invoke-GrindAgent([string]$BriefPath, [string]$OutcomePath,
                           [string]$RoleFile, [string]$AgentModel,
                           [string]$MockScript) {
    Remove-Item $OutcomePath -ErrorAction SilentlyContinue
    if ($MockScript) {
        $env:GRIND_BRIEF_PATH = $BriefPath; $env:GRIND_OUTCOME_PATH = $OutcomePath
        try { & pwsh -NoProfile -File $MockScript } finally {
            Remove-Item Env:\GRIND_BRIEF_PATH, Env:\GRIND_OUTCOME_PATH -ErrorAction SilentlyContinue
        }
    } else {
        # Fleet-proven invocation: task text directly to -p via array splat, inside a
        # Job so we get a wall-clock timeout.
        $task = (Get-Content $BriefPath -Raw -Encoding utf8) +
            "`n`nWhen finished, write your outcome JSON to this exact absolute path (overwrite it):`n  $OutcomePath`n"
        $sid = [guid]::NewGuid().ToString()
        $job = Start-Job -ScriptBlock {
            param($Task, $RoleFile, $Model, $Sid, $Cwd)
            Set-Location $Cwd
            $env:CLAUDE_SESSION_ID = $Sid
            $claudeArgs = @('-p', $Task, '--append-system-prompt-file', $RoleFile,
                            '--permission-mode', 'bypassPermissions', '--model', $Model,
                            '--session-id', $Sid, '--output-format', 'json')
            $null = ($null | & claude @claudeArgs | Out-String)
        } -ArgumentList $task, $RoleFile, $AgentModel, $sid, $Root
        if (-not (Wait-Job $job -Timeout ($SessionTimeoutMin * 60))) {
            Log "session TIMEOUT after $SessionTimeoutMin min; stopping job."
            Stop-Job $job -ErrorAction SilentlyContinue
        }
        Remove-Job $job -Force -ErrorAction SilentlyContinue
    }
    if (-not (Test-Path $OutcomePath)) { return $null }
    try { return (Get-Content $OutcomePath -Raw | ConvertFrom-Json) } catch { return $null }
}

# Paths a session may legitimately touch. ANYTHING else dirty = invalid session.
$AllowedDirtyPattern = '^(\?\?|.M|M.|A.|.A)\s+("?)(memory/grind/|tmp/|metrics/events\.jsonl|src/|include/)'

function Revert-SessionEdits {
    git -C $Root checkout -- src include 2>$null
    git -C $Root checkout -- metrics/events.jsonl 2>$null
}

$consecutiveInvalid = 0
while ($true) {
    if (Test-Path $StopFile) { Log "STOP sentinel found; exiting cleanly."; break }

    # 1) target = queue top
    $item = Get-QueueTop
    if (-not $item -or -not $item.func) { Log "queue empty — nothing to grind."; break }
    $func = [string]$item.func; $stem = [string]$item.file

    # 2) ensure ledger (convert WIP on first contact)
    $state = Join-Path $Root "memory\grind\$func\state.json"
    if (-not (Test-Path $state)) {
        if (Test-Path (Join-Path $Root "memory\wip\$func")) {
            python tools/grinder/grindlib.py convert-wip . $func $stem | Out-Null
            Log "${func}: seeded ledger from memory/wip checkpoint."
        } else {
            $origin = if ($item.origin) { [string]$item.origin } else { 'queue' }
            python tools/grinder/grindlib.py init . $func $stem $origin | Out-Null
        }
    }

    # 3) brief with mandated modality
    $modality = (python tools/grinder/grindlib.py modality . $func).Trim()
    $outPath  = Join-Path $GrindTmp "outcome_$func.json"
    $briefPath = Join-Path $GrindTmp "brief_$func.md"
    python tools/grinder/grindlib.py brief . $func $modality $outPath | Set-Content $briefPath -Encoding utf8
    $sessionN = ((Get-Content $state -Raw | ConvertFrom-Json).session_count + 1)
    Log "${func}: session $sessionN starting, modality=$modality"

    # 4) spawn
    $o = Invoke-GrindAgent $briefPath $outPath (Join-Path $RolesDir 'grind-session.md') $Model $MockSessionScript

    # 5) scope check — any edit outside the allowed surface invalidates the session
    $dirty = Assert-CleanTree
    $violations = @($dirty | Where-Object { $_ -notmatch $AllowedDirtyPattern })
    if ($violations.Count) {
        Log "${func}: SCOPE VIOLATION — $($violations -join ' | ') — session discarded."
        git -C $Root checkout -- . 2>$null; git -C $Root clean -fd -- tmp 2>$null
        $consecutiveInvalid++
        if ($consecutiveInvalid -ge 3) { Circuit-Break "3 consecutive invalid sessions on $func" }
        if ($Once) { break } else { continue }
    }

    # 6) validate the outcome (structure + modality proof)
    $valid = $false
    if ($o) {
        $o | ConvertTo-Json -Depth 8 | Set-Content $outPath -Encoding utf8
        python tools/grinder/grindlib.py validate . $outPath $modality
        $valid = ($LASTEXITCODE -eq 0)
    }
    if (-not $valid) {
        Log "${func}: INVALID session output — discarded, src reverted, respawning."
        Revert-SessionEdits
        $consecutiveInvalid++
        if ($consecutiveInvalid -ge 3) { Circuit-Break "3 consecutive invalid sessions on $func" }
        if ($Once) { break } else { continue }
    }
    $consecutiveInvalid = 0

    # 7) route by result
    switch ([string]$o.result) {
        'ruling-request' { Invoke-JudgeRuling $func ([string]$o.ruling_question); Revert-SessionEdits }
        'candidate-ready' { Invoke-CandidatePath $func $stem $modality $o }
        default {
            python tools/grinder/grindlib.py apply . $func $outPath $modality | Out-Null
            Revert-SessionEdits
            Log "${func}: progress applied — floor=$($o.floor), '$($o.headline)'"
            Journal "$func s$sessionN [$modality] floor=$($o.floor): $($o.headline)"
            git -C $Root add -- memory/grind docs/grind 2>$null
            git -C $Root commit -m "grind: $func ledger s$sessionN update [skip-park-src-guard]" 2>$null | Out-Null
        }
    }
    # spec: oracle checked around every session — src is reverted (or merged) by
    # this point, so any red here means real corruption -> stop, don't limp.
    if (-not (Test-OracleGreen)) { Circuit-Break "oracle not green after session on $func" }
    if ($Once) { break }
}
Remove-Item $PidFile -ErrorAction SilentlyContinue
Log "grinder stopped."
