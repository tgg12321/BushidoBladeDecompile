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
    # DELIBERATE DEVIATION from the plan's line-filter parser (which kept only
    # lines starting with '{'/'['): `queue next` PRETTY-PRINTS a multi-line JSON
    # object (indented inner lines don't start with '{') behind wteng's [wteng]
    # banner, with a nested "wip" key when a checkpoint exists — a line filter
    # would shred it. Robust parse instead: take the substring from the first '{'
    # to the last '}' (the object's own closing brace is always the final one)
    # and ConvertFrom-Json. Returns $null if there is no object or it won't parse.
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

function Circuit-Break([string]$Reason) {
    $inc = Join-Path $Root 'docs\grind\INCIDENT.md'
    @("# GRINDER CIRCUIT-BREAK — $(Get-Date -Format 'yyyy-MM-dd HH:mm')",
      "", "**Reason:** $Reason", "",
      "git HEAD: $(git -C $Root rev-parse --short HEAD)",
      "git status:", '```', (git -C $Root status --short | Out-String), '```',
      "Last 20 log lines:", '```',
      ((Get-Content (Join-Path $GrindTmp 'grind.log') -Tail 20) -join "`n"), '```'
     ) | Set-Content $inc -Encoding utf8
    Log "CIRCUIT-BREAK: $Reason — see docs/grind/INCIDENT.md"
    Journal "CIRCUIT-BREAK: $Reason"
    Remove-Item $PidFile -ErrorAction SilentlyContinue
    exit 3
}

function Add-Decision([string]$func, [string]$kind, [string]$verdict, [string]$justification) {
    if (-not (Test-Path $Decisions)) {
        "# Grinder judge decisions — the owner's audit trail`n" | Set-Content $Decisions -Encoding utf8
    }
    @("", "## $(Get-Date -Format 'yyyy-MM-dd HH:mm') — $func — $kind — **$verdict**",
      "", $justification) | Add-Content $Decisions
}

function Invoke-Judge([string]$func, [string]$TaskText) {
    # Retry transient failures with backoff (a proven candidate is never
    # discarded because of an API hiccup).
    $outPath = Join-Path $GrindTmp "judge_$func.json"
    $briefPath = Join-Path $GrindTmp "judge_brief_$func.md"
    Set-Content $briefPath -Value $TaskText -Encoding utf8
    for ($try = 1; $try -le 5; $try++) {
        $v = Invoke-GrindAgent $briefPath $outPath (Join-Path $RolesDir 'judge.md') $JudgeModel $MockJudgeScript
        if ($v -and $v.verdict -in @('PASS', 'FAIL')) { return $v }
        Log "judge attempt $try returned no valid verdict; backing off $([math]::Pow(2,$try) * 30)s."
        Start-Sleep -Seconds ([math]::Pow(2, $try) * 30)
    }
    Circuit-Break "judge unreachable/invalid after 5 attempts for $func"
}

function Invoke-JudgeRuling([string]$func, [string]$question) {
    $led = "memory/grind/$func"
    $task = @"
RULING REQUEST for $func.

Question from the grind session:
$question

Read the ledger first: $led/state.json (judge_constraints), $led/hypotheses.md,
$led/evidence.md, $led/candidate.c, $led/rejected/. Verify claims yourself.
Write your verdict JSON to the exact path given below.
"@
    $v = Invoke-Judge $func $task
    $qShort = $question.Substring(0, [Math]::Min(80, $question.Length))
    Add-Decision $func "ruling: $qShort" $v.verdict $v.justification
    if ($v.constraint) { python tools/grinder/grindlib.py constrain . $func ([string]$v.constraint) | Out-Null }
    git -C $Root add -- memory/grind docs/grind 2>$null
    git -C $Root commit -m "grind: $func judge ruling [skip-park-src-guard]" 2>$null | Out-Null
    Log "${func}: judge ruling $($v.verdict) recorded."
}

function Invoke-CandidatePath([string]$func, [string]$stem, [string]$modality, $o) {
    # 1) bytes first — driver-verified, never trusted from the session
    $sb = Invoke-Eng @('sandbox', $func, '--disable', 'all')
    if ($sb -notmatch '"score"\s*:\s*0\b') {
        Log "${func}: candidate-ready claim FAILED driver sandbox check — treating as invalid session."
        Revert-SessionEdits
        return
    }
    # Single-stem gate: the ONLY build-input change allowed in a candidate is the
    # target file itself. A load-bearing edit to another src/include file would be
    # byte-verified now but DROPPED by the fixed Match commit below — producing a
    # committed Match that fails a fresh rebuild. Reject before retire (retire
    # legitimately mutates regfix/asmfix afterwards). Keys on the PATH under any
    # git status (M/A/MM/rename/?? untracked header) — not modifications alone.
    # NB: porcelain format is exactly "XY path"; extract by fixed offset. (The
    # previous `-replace '^[\sA-Z?]+'` was case-INSENSITIVE per PowerShell default
    # and ate the lowercase 'src' prefix too, mangling every path.)
    $buildMods = @(git -C $Root status --porcelain |
        Where-Object { $_ -match '^..\s+("?)(src/|include/)' } |
        ForEach-Object { $_.Substring(3).Trim().Trim('"') })
    $offStem = @($buildMods | Where-Object { $_ -ne "src/$stem.c" })
    if ($offStem.Count) {
        Log "${func}: candidate touches build inputs beyond src/$stem.c ($($offStem -join ', ')) — rejected as invalid session."
        Revert-SessionEdits
        $script:consecutiveInvalid++
        if ($script:consecutiveInvalid -ge 3) { Circuit-Break "3 consecutive invalid sessions on $func" }
        return
    }
    $null = Invoke-Eng @('retire', $func)          # drops rules if any; SHA1-gated internally
    $vo = Invoke-Eng @('verify-oracle', '--rebuild', '--allow-dirty')
    if ($LASTEXITCODE -ne 0) {
        Log "${func}: FULL-BUILD SHA1 FAILED after retire — reverting, banking constraint."
        git -C $Root checkout -- . 2>$null
        python tools/grinder/grindlib.py constrain . $func "candidate form failed full-build SHA1 on main (masked-0 register diff class) — reg-alloc gap is real" | Out-Null
        git -C $Root add -- memory/grind docs/grind 2>$null
        git -C $Root commit -m "grind: $func byte-fail constraint banked [skip-park-src-guard]" 2>$null | Out-Null
        return
    }
    # 2) bytes proven — now the Judge rules on the C
    $diff = (git -C $Root diff -- "src/$stem.c" | Out-String)
    $led = "memory/grind/$func"
    $task = @"
FINAL CALL for $func — bytes are already proven on main (sandbox 0 + retire +
full-build SHA1 == oracle). Rule ONLY on the legitimacy of the C.

The candidate diff against HEAD:
``````diff
$diff
``````

Ledger: $led/state.json (judge_constraints — includes the regression diagnosis
if this is a regression-origin item), $led/hypotheses.md, $led/evidence.md,
$led/rejected/. Write your verdict JSON to the exact path given below.
"@
    $v = Invoke-Judge $func $task
    $sessionsTaken = ((Get-Content (Join-Path $Root "memory\grind\$func\state.json") -Raw | ConvertFrom-Json).session_count + 1)
    if ($v.verdict -eq 'PASS') {
        $qd = Invoke-Eng @('queue', 'done', $func)
        if ($qd -notmatch '"ok"\s*:\s*true') {
            Log "${func}: queue done REFUSED after judge PASS: $qd"
            git -C $Root checkout -- . 2>$null
            Circuit-Break "queue done refused a judge-PASSed, bytes-proven candidate for $func — investigate"
        }
        git -C $Root add -- "src/$stem.c" engine/queue.json regfix.txt asmfix.txt 2>$null
        git -C $Root commit -m "Match: $func — COMPLETED-C (grinder, $sessionsTaken sessions)" | Out-Null
        Add-Decision $func 'final call' 'PASS' $v.justification
        Journal "$func COMPLETED-C after $sessionsTaken sessions."
        Remove-Item -Recurse -Force (Join-Path $Root "memory\grind\$func")
        git -C $Root add -A -- memory/grind docs/grind 2>$null
        git -C $Root commit -m "grinder: close ledger for $func" | Out-Null
        Log "${func}: MERGED — COMPLETED-C."
    } else {
        # Ordering matters: the rejected-form Copy-Item must read the still-dirty
        # src BEFORE the broad checkout; every LEDGER write (Add-Decision,
        # constrain, the commit) must come AFTER it, or `git checkout -- .` would
        # revert the tracked decisions.md/state.json we just wrote.
        # keep the byte-matching form as evidence, but main goes back to HEAD
        Copy-Item (Join-Path $Root "src\$stem.c") (Join-Path $Root "memory\grind\$func\rejected\judge-fail-$(Get-Date -Format 'MMdd-HHmm').c") -ErrorAction SilentlyContinue
        git -C $Root checkout -- . 2>$null
        Invoke-Eng @('verify-oracle', '--rebuild') | Out-Null   # restore green build/
        Add-Decision $func 'final call' 'FAIL' $v.justification
        $c = if ($v.constraint) { [string]$v.constraint } else { [string]$v.justification }
        python tools/grinder/grindlib.py constrain . $func $c | Out-Null
        git -C $Root add -- memory/grind docs/grind 2>$null
        git -C $Root commit -m "grind: $func judge FAIL banked [skip-park-src-guard]" 2>$null | Out-Null
        Log "${func}: judge FAILED the candidate — constraint banked, grind continues."
        $jShort = $v.justification.Substring(0, [Math]::Min(120, $v.justification.Length))
        Journal "${func}: judge FAILED a bytes-proven candidate — $jShort"
    }
}

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
# docs/grind/ is included because the DRIVER's own journal/INCIDENT/decisions
# writes can be left uncommitted by a circuit-break — they must never trip the
# scope gate of a later session (2026-07-07 incident: the gate's broad checkout
# also wiped concurrent uncommitted ledger work).
$AllowedDirtyPattern = '^(\?\?|.M|M.|A.|.A)\s+("?)(memory/grind/|docs/grind/|tmp/|metrics/events\.jsonl|src/|include/)'

function Revert-SessionEdits {
    git -C $Root checkout -- src include 2>$null
    git -C $Root checkout -- metrics/events.jsonl 2>$null
}

$script:consecutiveInvalid = 0
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
        $script:consecutiveInvalid++
        if ($script:consecutiveInvalid -ge 3) { Circuit-Break "3 consecutive invalid sessions on $func" }
        if ($Once) { break } else { continue }
    }

    # 6) validate the outcome (structure + modality proof)
    $valid = $false
    $invalidReason = if ($o) { '' } else { 'no outcome file / unparseable JSON' }
    if ($o) {
        $o | ConvertTo-Json -Depth 8 | Set-Content $outPath -Encoding utf8
        $invalidReason = (python tools/grinder/grindlib.py validate . $outPath $modality 2>&1 | Out-String).Trim()
        $valid = ($LASTEXITCODE -eq 0)
    }
    if (-not $valid) {
        # Preserve the discarded outcome for diagnosis — repeated invalids are
        # otherwise unexplainable after the respawn overwrites the file.
        if (Test-Path $outPath) {
            Copy-Item $outPath (Join-Path $GrindTmp "invalid_${func}_s${sessionN}_$(Get-Date -Format 'HHmmss').json") -ErrorAction SilentlyContinue
        }
        Log "${func}: INVALID session output ($invalidReason) — discarded, src reverted, respawning."
        Revert-SessionEdits
        $script:consecutiveInvalid++
        if ($script:consecutiveInvalid -ge 3) { Circuit-Break "3 consecutive invalid sessions on $func" }
        if ($Once) { break } else { continue }
    }
    $script:consecutiveInvalid = 0

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
