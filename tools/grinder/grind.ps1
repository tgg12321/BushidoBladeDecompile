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
    [string]$Model = 'claude-opus-5[1m]',
    [string]$JudgeModel = 'claude-opus-5[1m]',
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
function Reap-PermuterOrphans([string]$When) {
    # Backstop for the fresh-seed campaign discipline (owner directive
    # 2026-07-07): permuter campaigns must not outlive the session that seeded
    # them. Workers harvest+stop their own campaigns (tools/permuter_campaign.py,
    # which records the telemetry); this reaps whatever survived a crashed,
    # timed-out, or discarded session, so orphans can never accumulate again
    # (2026-07-07 incident: ~100 workers, some 21h old, from dead sessions).
    # The backslash in the pattern keeps pgrep/pkill from matching their own
    # invoking bash cmdline.
    try {
        $n = (& wsl bash -c 'pgrep -fc "permuter\.py" || true' 2>$null | Out-String).Trim()
        if ($n -and [int]$n -gt 0) {
            & wsl bash -c 'pkill -f "permuter\.py" || true' 2>$null | Out-Null
            Log "reaped $n orphaned permuter process(es) ($When)."
        }
    } catch { }
    # decomp-permuter's import.py does os.makedirs("nonmatchings/") relative to
    # cwd; a session that runs it from the repo root (instead of
    # tools/decomp-permuter/) drops a root-level nonmatchings/ that the scope
    # check flags as out-of-surface dirt — three such discards circuit-broke the
    # grinder on func_800611A4 (2026-07-19). It is gitignored now, but sweep the
    # physical dir too so it can't accumulate across sessions.
    $nm = Join-Path $Root 'nonmatchings'
    if (Test-Path $nm) { Remove-Item $nm -Recurse -Force -ErrorAction SilentlyContinue }
    # Clear stale campaign-registry 'active' flags. Reaping above kills orphan
    # PROCESSES but leaves tmp/permuter_campaigns.json marking them active, which
    # would make the grind_check.sh Stop-gate false-positive on a NEXT session of
    # the same func. Zeroing active at every boundary makes the invariant exact:
    # an active entry at Stop time was orphaned by THE CURRENT session.
    try { python tools/permuter_campaign.py deactivate-all 2>$null | Out-Null } catch { }
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

function Get-JudgeLimitReset([string]$AgentLog) {
    # If the last spawn died on a plan usage-limit 429, return the [datetime]
    # the limit resets (parsed from the CLI result text, plan-local time, e.g.
    # "resets 4pm (America/Chicago)"); 429 with unparseable text -> now+30min;
    # anything else (no 429, no log, parse error) -> $null. Best-effort.
    try {
        if (-not (Test-Path $AgentLog)) { return $null }
        $raw = Get-Content $AgentLog -Raw
        $lo = $raw.IndexOf('{'); $hi = $raw.LastIndexOf('}')
        if ($lo -lt 0 -or $hi -le $lo) { return $null }
        $d = $raw.Substring($lo, $hi - $lo + 1) | ConvertFrom-Json
        if ([int]$d.api_error_status -ne 429) { return $null }
        $m = [regex]::Match([string]$d.result, 'resets\s+(\d{1,2})(?::(\d{2}))?\s*(am|pm)', 'IgnoreCase')
        if (-not $m.Success) { return (Get-Date).AddMinutes(30) }
        $h = [int]$m.Groups[1].Value % 12
        if ($m.Groups[3].Value -match '(?i)pm') { $h += 12 }
        $min = 0; if ($m.Groups[2].Success) { $min = [int]$m.Groups[2].Value }
        $t = (Get-Date).Date.AddHours($h).AddMinutes($min)
        if ($t -le (Get-Date)) { $t = $t.AddDays(1) }
        return $t
    } catch { return $null }
}

function Invoke-Judge([string]$func, [string]$TaskText) {
    # Retry transient failures with backoff (a proven candidate is never
    # discarded because of an API hiccup). Usage-limit 429s are NOT transient
    # hiccups: they last hours, so they get their own limit-aware wait and do
    # not consume retry attempts (2026-07-21 incident: five instant 429 deaths
    # in a 32-min backoff window circuit-broke an otherwise healthy grind with
    # a bytes-proven candidate waiting).
    $outPath = Join-Path $GrindTmp "judge_$func.json"
    $briefPath = Join-Path $GrindTmp "judge_brief_$func.md"
    Set-Content $briefPath -Value $TaskText -Encoding utf8
    $try = 0; $limitWaits = 0
    while ($true) {
        # NB: $Func deliberately NOT passed (judges never launch campaigns, so the
        # GRIND_FUNC Stop-gate stays unarmed); UsageFunc carries it for telemetry.
        $v = Invoke-GrindAgent $briefPath $outPath (Join-Path $RolesDir 'judge.md') $JudgeModel $MockJudgeScript -UsageFunc $func -UsageRole 'judge'
        if ($v -and $v.verdict -in @('PASS', 'FAIL')) { return $v }
        $reset = if ($MockJudgeScript) { $null } else { Get-JudgeLimitReset ($outPath + '.agent.log') }
        if ($reset) {
            $limitWaits++
            if ($limitWaits -gt 12) { Circuit-Break "judge still usage-limited after 12 wait cycles for $func" }
            # Cap each cycle at 4h so a mis-parsed far-future reset can't hang the
            # driver for a day; +120s slack past the stated reset.
            $waitS = [int][Math]::Min(4 * 3600, [Math]::Max(60, ($reset - (Get-Date)).TotalSeconds + 120))
            Log "judge hit usage-limit 429 (stated reset $($reset.ToString('HH:mm'))); waiting $([int]($waitS/60)) min, candidate preserved (limit-wait $limitWaits)."
            $end = (Get-Date).AddSeconds($waitS)
            while ((Get-Date) -lt $end) {
                if (Test-Path $StopFile) {
                    # Owner asked for a stop mid-wait: exit via circuit-break so the
                    # proven-candidate state is preserved + documented. Benign.
                    Circuit-Break "STOP requested during judge usage-limit wait for $func (benign: candidate preserved; relaunch after the limit resets)"
                }
                Start-Sleep -Seconds 60
            }
            continue   # a limit window never consumes a retry attempt
        }
        $try++
        if ($try -ge 5) { Circuit-Break "judge unreachable/invalid after 5 attempts for $func" }
        Log "judge attempt $try returned no valid verdict; backing off $([math]::Pow(2,$try) * 30)s."
        Start-Sleep -Seconds ([math]::Pow(2, $try) * 30)
    }
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
    git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
    git -C $Root commit -m "grind: $func judge ruling [skip-park-src-guard]" 2>$null | Out-Null
    Log "${func}: judge ruling $($v.verdict) recorded."
}

function Get-ExtraScope([string]$func) {
    # Per-function extra build inputs a candidate may touch (tools/grinder/scope_allow.txt).
    # Returned paths are allowed by the scope check AND staged into the Match commit —
    # the two MUST stay in lockstep or a Match commits a tree that doesn't rebuild.
    $f = Join-Path $Root 'tools\grinder\scope_allow.txt'
    if (-not (Test-Path $f)) { return @() }
    foreach ($line in (Get-Content $f)) {
        $t = $line.Trim()
        if (-not $t -or $t.StartsWith('#')) { continue }
        $parts = @($t -split '\s+' | Where-Object { $_ })
        if ($parts.Count -ge 2 -and $parts[0] -eq $func) { return @($parts[1..($parts.Count - 1)]) }
    }
    return @()
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
    $extra = @(Get-ExtraScope $func)
    $offStem = @($buildMods | Where-Object { $_ -ne "src/$stem.c" -and $extra -notcontains $_ })
    if ($offStem.Count) {
        # LIVELOCK BACKSTOP (2026-08-01). This rejection can NOT feed
        # $consecutiveInvalid: a structurally-valid outcome resets that counter at the
        # top of the routing switch, BEFORE this path runs — so it was pinned at 1 and
        # the circuit breaker was unreachable. replay_camera_Init re-proposed the same
        # include/ edit 161 times in 11 hours with no backstop.
        # Escalate on a per-function+paths key that nothing resets, and prefer
        # SELF-CORRECTION over halting: tell the session why (constraint), then park
        # and let the queue advance. A full stop is reserved for the systemic case
        # (livelock across several functions), so the owner returns to a working
        # pipeline and ONE ledger entry — not a wall of incidents.
        $key = "$func|" + ((@($offStem) | Sort-Object) -join ',')
        $script:scopeRejects[$key] = 1 + [int]$script:scopeRejects[$key]
        $n = [int]$script:scopeRejects[$key]
        Log "${func}: candidate touches build inputs beyond src/$stem.c ($($offStem -join ', ')) — rejected as invalid session (repeat $n)."
        Revert-SessionEdits
        if ($n -eq 2) {
            $c = "OUT OF SCOPE: candidates for $func may only edit src/$stem.c. Edits to " +
                 "$($offStem -join ', ') are rejected by the driver and can never be accepted, " +
                 "however good the bytes. Either find a form confined to src/$stem.c, or file an " +
                 "OWNER-ESCALATION requesting these paths be added to tools/grinder/scope_allow.txt " +
                 "and return owner-gated. Do NOT re-propose the same out-of-scope edit."
            python tools/grinder/grindlib.py constrain . $func $c | Out-Null
            git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
            git -C $Root commit -m "grind: $func out-of-scope constraint banked [skip-park-src-guard]" 2>$null | Out-Null
            Log "${func}: banked out-of-scope constraint after $n identical rejections."
        }
        if ($n -ge 5) {
            $reason = "scope livelock: $n candidates re-proposed edits to $($offStem -join ', '), " +
                      "which the driver cannot accept. Needs an owner decision: extend " +
                      "tools/grinder/scope_allow.txt for this function, or reprioritise it."
            Invoke-Eng @('queue', 'park', $func, '--reason', $reason) | Out-Null
            Add-Decision $func 'scope livelock' 'OWNER-ESCALATION' $reason
            Journal "$func SCOPE-LIVELOCK — parked after $n identical out-of-scope candidates ($($offStem -join ', '))."
            Log "${func}: SCOPE LIVELOCK — parked so the queue advances; owner decision needed."
            git -C $Root add -- memory/grind docs/grind metrics/events.jsonl engine/queue.json 2>$null
            git -C $Root commit -m "grind: $func parked on scope livelock [skip-park-src-guard]" 2>$null | Out-Null
            $script:livelockParks++
            if ($script:livelockParks -ge 3) {
                Circuit-Break "scope livelock on $script:livelockParks distinct functions — the scope gate looks systemically wrong, not function-specific"
            }
        }
        return
    }
    $null = Invoke-Eng @('retire', $func)          # drops rules if any; SHA1-gated internally
    $vo = Invoke-Eng @('verify-oracle', '--rebuild', '--allow-dirty')
    if ($LASTEXITCODE -ne 0) {
        Log "${func}: FULL-BUILD SHA1 FAILED after retire — reverting, banking constraint."
        # stage events BEFORE the broad checkout: `checkout -- .` restores the
        # worktree FROM the index, so staged telemetry survives the revert
        git -C $Root add -- metrics/events.jsonl 2>$null
        git -C $Root checkout -- . 2>$null
        python tools/grinder/grindlib.py constrain . $func "candidate form failed full-build SHA1 on main (masked-0 register diff class) — reg-alloc gap is real" | Out-Null
        git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
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
            # A Judge-PASSed, bytes-proven candidate that queue done still
            # refuses means an un-retired config-level cheat remains (a stale
            # prologue_fix entry retire couldn't drop before the 2026-07-19 fix,
            # a maspsx cheat-pathway gate, etc.). The oracle is still green, so
            # this is a per-function "needs more work" signal, NOT pipeline
            # corruption — bank a constraint and grind on instead of halting the
            # whole driver (the old Circuit-Break here stopped everything).
            $reason = ("queue done refused a judge-PASSed candidate — an un-retired config-level " +
                       "cheat remains (prologue_fix / maspsx cheat-pathway gate / other) that the " +
                       "honest COMPLETED-C form must eliminate, or the function needs owner " +
                       "canonical-asm authorization. gate: " + (($qd -replace '["\r\n\t]+', ' ') -replace '\s+', ' ').Trim())
            $reason = $reason.Substring(0, [Math]::Min(400, $reason.Length))
            Log "${func}: queue done REFUSED after judge PASS — banking constraint, grind continues."
            git -C $Root add -- metrics/events.jsonl 2>$null
            git -C $Root checkout -- . 2>$null
            Invoke-Eng @('verify-oracle', '--rebuild') | Out-Null   # restore canonical build/
            python tools/grinder/grindlib.py constrain . $func $reason | Out-Null
            git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
            git -C $Root commit -m "grind: $func queue-done-refusal constraint banked [skip-park-src-guard]" 2>$null | Out-Null
            Journal "${func}: queue done refused a bytes-proven candidate — un-retired config cheat; constraint banked."
            return
        }
        # Stage the per-function ledger INTO the Match commit so its exhaustion
        # evidence is durable in git history BEFORE line 329 deletes the working
        # copy. Without this, a function that closes in one session (candidate-ready
        # with no prior 'grind: ledger sN' progress commit) has its ledger created,
        # never committed, then deleted — never reaching git. That gap let
        # func_80041988's FAKE annotation cite a memory/grind/ ledger that never
        # existed (2026-07-22 backlog audit); a fabricated-evidence cheat then passed
        # the same-tier fable judge unverifiable. Committing the ledger makes every
        # COMPLETED-C function's evidence auditable after the fact.
        # $extraScope MUST match what the scope check allowed above — the committed
        # tree has to be exactly the tree that passed the full-build SHA1 gate, or a
        # Match lands missing a load-bearing edit and the next rebuild breaks.
        $extraScope = @(Get-ExtraScope $func)
        git -C $Root add -- "src/$stem.c" $extraScope engine/queue.json regfix.txt regfix_stage2.txt asmfix.txt tools/prologue_config.json tools/frame_fix_funcs.txt tools/delay_slot_ra_funcs.txt "memory/grind/$func" 2>$null
        git -C $Root commit -m "Match: $func — COMPLETED-C (grinder, $sessionsTaken sessions)" | Out-Null
        Add-Decision $func 'final call' 'PASS' $v.justification
        Journal "$func COMPLETED-C after $sessionsTaken sessions."
        Remove-Item -Recurse -Force (Join-Path $Root "memory\grind\$func")
        git -C $Root add -A -- memory/grind docs/grind 2>$null
        git -C $Root add -- metrics/events.jsonl 2>$null
        git -C $Root commit -m "grinder: close ledger for $func" | Out-Null
        Log "${func}: MERGED — COMPLETED-C."
    } else {
        # Ordering matters: the rejected-form Copy-Item must read the still-dirty
        # src BEFORE the broad checkout; every LEDGER write (Add-Decision,
        # constrain, the commit) must come AFTER it, or `git checkout -- .` would
        # revert the tracked decisions.md/state.json we just wrote.
        # keep the byte-matching form as evidence, but main goes back to HEAD
        Copy-Item (Join-Path $Root "src\$stem.c") (Join-Path $Root "memory\grind\$func\rejected\judge-fail-$(Get-Date -Format 'MMdd-HHmm').c") -ErrorAction SilentlyContinue
        git -C $Root add -- metrics/events.jsonl 2>$null   # staged telemetry survives the checkout
        git -C $Root checkout -- . 2>$null
        Invoke-Eng @('verify-oracle', '--rebuild') | Out-Null   # restore green build/
        Add-Decision $func 'final call' 'FAIL' $v.justification
        $c = if ($v.constraint) { [string]$v.constraint } else { [string]$v.justification }
        python tools/grinder/grindlib.py constrain . $func $c | Out-Null
        git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
        git -C $Root commit -m "grind: $func judge FAIL banked [skip-park-src-guard]" 2>$null | Out-Null
        Log "${func}: judge FAILED the candidate — constraint banked, grind continues."
        $jShort = $v.justification.Substring(0, [Math]::Min(120, $v.justification.Length))
        Journal "${func}: judge FAILED a bytes-proven candidate — $jShort"
    }
}

# ── session spawn (pattern from tools/fleet/_fleet_common.ps1:132-170) ────────
function Invoke-GrindAgent([string]$BriefPath, [string]$OutcomePath,
                           [string]$RoleFile, [string]$AgentModel,
                           [string]$MockScript, [string]$Func,
                           [string]$UsageFunc, [string]$UsageRole = 'session') {
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
        $t0 = Get-Date
        $job = Start-Job -ScriptBlock {
            param($Task, $RoleFile, $Model, $Sid, $Cwd, $AgentLog, $Func)
            Set-Location $Cwd
            $env:CLAUDE_SESSION_ID = $Sid
            # Arms the grind_check.sh Stop-gate for THIS session only: the hook
            # no-ops unless GRIND_FUNC is set, so interactive/operator sessions
            # and this session's own subagents (Stop-only wiring) are unaffected.
            $env:GRIND_FUNC = $Func
            # --strict-mcp-config with no --mcp-config = ZERO MCP servers for grind
            # spawns. The operator's user-scope servers (github/playwright/unity) are
            # useless to a decomp session but their tool surface rides in the baseline
            # of every turn, and a turn's context is re-read on all later turns. Also
            # removes the chance a session wanders into a browser/editor tool.
            $claudeArgs = @('-p', $Task, '--append-system-prompt-file', $RoleFile,
                            '--permission-mode', 'bypassPermissions', '--model', $Model,
                            '--strict-mcp-config',
                            '--session-id', $Sid, '--output-format', 'json')
            # Keep the CLI's result line — it is the only diagnostic when a spawn
            # dies instantly (usage limit, auth, API error). Overwritten per spawn.
            ($null | & claude @claudeArgs 2>&1 | Out-String) | Set-Content $AgentLog -Encoding utf8
        } -ArgumentList $task, $RoleFile, $AgentModel, $sid, $Root, ($OutcomePath + '.agent.log'), $Func
        if (-not (Wait-Job $job -Timeout ($SessionTimeoutMin * 60))) {
            Log "session TIMEOUT after $SessionTimeoutMin min; stopping job."
            Stop-Job $job -ErrorAction SilentlyContinue
        }
        Remove-Job $job -Force -ErrorAction SilentlyContinue
        $script:LastAgentSeconds = ((Get-Date) - $t0).TotalSeconds
        # Durable per-spawn usage telemetry: the agent.log result JSON (tokens,
        # turns, duration, cost, error state) is OVERWRITTEN by the next spawn of
        # the same func, so bank it into metrics/events.jsonl now as a
        # "grind-agent-usage" event. Best-effort by the metrics contract — never
        # raises, never blocks the driver (tools/grinder/record_usage.py).
        $uFunc = if ($UsageFunc) { $UsageFunc } else { $Func }
        try { python tools/grinder/record_usage.py ($OutcomePath + '.agent.log') $uFunc $UsageRole 2>$null | Out-Null } catch { }
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
    # Deliberately does NOT touch metrics/events.jsonl: events are append-only
    # facts about engine/permuter commands that really ran — valid telemetry
    # even from discarded sessions (owner directive 2026-07-07; reverting it
    # here wiped every session's permuter-harvest events). The ledger commits
    # sweep it up each boundary.
    git -C $Root checkout -- src include 2>$null
}

$script:consecutiveInvalid = 0
$script:spawnFails = 0
# Scope-livelock tracking. Keyed per function+offending-paths and deliberately
# NEVER reset — a repeat is a repeat even if healthy sessions happen in between.
$script:scopeRejects = @{}
$script:livelockParks = 0
$script:LastAgentSeconds = 9999   # only the real-spawn path sets this; mock/drill paths must never look like spawn failures
while ($true) {
    # Loop-TOP placement is deliberate: it fires after EVERY session disposition
    # (progress, candidate, scope-violation `continue`, invalid `continue`), at
    # driver start (leftovers from a crashed prior run), and before a STOP exit.
    Reap-PermuterOrphans 'session boundary'
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
    $stObj = Get-Content $state -Raw | ConvertFrom-Json
    $sessionN = ($stObj.session_count + 1)
    # Floor entering this session — the escalation backstop uses it to tell a real
    # floor-drop (progress) from a flat-floor dodge in `escalation` modality. Floor
    # is normally an int, but WIP-imported ledgers can carry a prose string there
    # (func_80062020: "loop body solved …"); parse defensively — a non-numeric floor
    # leaves $priorFloor $null, which just disables the dodge check for that session.
    $priorFloor = $null
    if ($stObj.floor_history -and $stObj.floor_history.Count) {
        $pf = $stObj.floor_history[-1].floor
        if ($null -ne $pf -and "$pf" -match '^-?\d+$') { $priorFloor = [int]$pf }
    }
    Log "${func}: session $sessionN starting, modality=$modality"

    # 4) spawn
    $o = Invoke-GrindAgent $briefPath $outPath (Join-Path $RolesDir 'grind-session.md') $Model $MockSessionScript $func

    # 5) scope check — any edit outside the allowed surface invalidates the session
    $dirty = Assert-CleanTree
    $violations = @($dirty | Where-Object { $_ -notmatch $AllowedDirtyPattern })
    if ($violations.Count) {
        Log "${func}: SCOPE VIOLATION — $($violations -join ' | ') — session discarded."
        git -C $Root add -- metrics/events.jsonl 2>$null   # staged telemetry survives the checkout
        git -C $Root checkout -- . 2>$null; git -C $Root clean -fd -- tmp 2>$null
        # ALSO purge untracked out-of-surface dirt: checkout only restores TRACKED
        # files, and `clean -fd -- tmp` skips gitignored tmp — so an untracked junk
        # file elsewhere (2026-07-21: a zero-byte root file named `]<U+F022>`)
        # survives the discard and poisons EVERY respawn's scope check -> guaranteed
        # circuit-break, with innocent sessions discarded (one had closed
        # saSeMain_80045600 to sandbox 0). Keep the allowed untracked surfaces;
        # remove everything else the check would flag again.
        git -C $Root clean -fdq -e memory -e docs -e src -e include 2>$null
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
        # owner-gated: the validator can't see $func, so the driver verifies the
        # cited OWNER-ESCALATION entry actually names THIS function.
        if ($valid -and [string]$o.result -eq 'owner-gated') {
            $decTxt = Get-Content (Join-Path $Root 'docs\grind\decisions.md') -Raw -ErrorAction SilentlyContinue
            $named = @(($decTxt -split "`n") | Where-Object { $_ -match 'OWNER-ESCALATION' -and $_ -match [regex]::Escape($func) })
            if (-not $named.Count) {
                $valid = $false
                $invalidReason = "owner-gated claim rejected: no OWNER-ESCALATION entry in docs/grind/decisions.md names $func"
            }
        }
    }
    if (-not $valid) {
        # SPAWN FAILURE, not a bad session: no outcome at all AND the agent died
        # near-instantly (usage-limit window, auth expiry, API outage). These are
        # environmental — back off and retry instead of feeding the circuit
        # breaker (2026-07-17 incident: three 4-second spawn deaths during a
        # usage-limit window circuit-broke an otherwise healthy grind).
        if (-not $o -and $script:LastAgentSeconds -lt 120) {
            $script:spawnFails++
            $delay = [int][Math]::Min(1800, 60 * [Math]::Pow(2, $script:spawnFails - 1))
            Log "${func}: agent SPAWN FAILURE ($([int]$script:LastAgentSeconds)s, no outcome — likely usage-limit/API; see $outPath.agent.log) — attempt $($script:spawnFails), retrying in ${delay}s."
            Revert-SessionEdits
            Start-Sleep -Seconds $delay
            if ($Once) { break } else { continue }
        }
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
    $script:spawnFails = 0

    # 7) route by result
    switch ([string]$o.result) {
        'ruling-request' { Invoke-JudgeRuling $func ([string]$o.ruling_question); Revert-SessionEdits }
        'candidate-ready' { Invoke-CandidatePath $func $stem $modality $o }
        'owner-gated' {
            # A filed OWNER-ESCALATION (verified above to name $func) blocks
            # this function on an owner-only ruling and every sanctioned axis is
            # measured dead. Park it so the queue advances (parked items are
            # skipped by `queue next`; the escalation stays open in
            # docs/grind/decisions.md — parking is a wait-state, not a
            # disposition, per no-park-permanently).
            python tools/grinder/grindlib.py apply . $func $outPath $modality | Out-Null
            Revert-SessionEdits
            # Standing auto-ruling (owner, 2026-07-27): a RESOLVED BY STANDING RULING
            # entry is terminal (OWNER-ACCEPTED INCOMPLETE, nothing pending); only a
            # true pending escalation (gate-passing case) waits on the owner.
            $escRef = [string]$o.escalation_ref
            if ($escRef -match 'RESOLVED BY STANDING RULING') {
                $reason = "OWNER-ACCEPTED INCOMPLETE (standing ruling 2026-07-27): $escRef"
                Invoke-Eng @('queue', 'park', $func, '--reason', $reason) | Out-Null
                Log "${func}: STANDING RULING APPLIED — REFUSED / OWNER-ACCEPTED INCOMPLETE, parked terminally ($escRef)."
                Journal "$func s$sessionN [$modality] STANDING RULING (2026-07-27) applied — OWNER-ACCEPTED INCOMPLETE: $($o.headline)"
            } else {
                $reason = "owner escalation pending: $escRef"
                Invoke-Eng @('queue', 'park', $func, '--reason', $reason) | Out-Null
                Log "${func}: OWNER-GATED — parked pending owner ruling ($escRef)."
                Journal "$func s$sessionN [$modality] OWNER-GATED — parked pending owner ruling: $($o.headline)"
            }
            # engine/queue.json is where `queue park` wrote the parked status — it
            # MUST be staged, or the park stays as working-tree dirt and the next
            # session's scope check (engine/ is not in AllowedDirtyPattern) reverts
            # it via `git checkout -- .`, un-parking the function and bouncing it
            # straight back to the queue top forever (2026-07-18: ~40 sessions
            # burned re-parking motion_SetMotion; see grinder-park-queue-dirt-deadlock).
            git -C $Root add -- memory/grind docs/grind metrics/events.jsonl engine/queue.json 2>$null
            git -C $Root commit -m "grind: $func parked owner-gated pending ruling [skip-park-src-guard]" 2>$null | Out-Null
        }
        default {
            # ESCALATION BACKSTOP: in `escalation` modality the session was mandated
            # to file the OWNER-ESCALATION + return owner-gated (or DROP the floor with
            # a real lever). If it dodged — returned progress without lowering the floor
            # — the driver files the escalation itself so an exhausted function can never
            # loop (2026-07-22: func_8007DC9C ground 40 flat sessions this way). A real
            # floor drop is honored as ordinary progress (the exhaustion counter resets).
            $dodged = ($modality -eq 'escalation' -and $null -ne $priorFloor -and
                       $null -ne $o.floor -and [int]$o.floor -ge $priorFloor)
            if ($dodged) {
                python tools/grinder/grindlib.py apply . $func $outPath $modality | Out-Null
                Revert-SessionEdits
                $tier = 'LOW'
                try { $sc = (python tools/scan_hand_coded.py --single $func 2>$null | Out-String)
                      if ($sc -match 'tier=(\w+)') { $tier = $Matches[1] } } catch { }
                $rc = 0
                try { $rc = @(Select-String -Path (Join-Path $Root 'regfix.txt'),(Join-Path $Root 'asmfix.txt') -Pattern "^$([regex]::Escape($func)):" -ErrorAction SilentlyContinue).Count } catch { }
                $ref = (python tools/grinder/grindlib.py autoescalate . $func $stem $tier $rc (Get-Date -Format 'yyyy-MM-dd')).Trim()
                if ($ref -match 'RESOLVED BY STANDING RULING') {
                    $bsReason = "OWNER-ACCEPTED INCOMPLETE (standing ruling 2026-07-27, auto-filed backstop): $ref"
                } else {
                    $bsReason = "owner escalation pending (auto-filed backstop): $ref"
                }
                Invoke-Eng @('queue', 'park', $func, '--reason', $bsReason) | Out-Null
                Log "${func}: ESCALATION BACKSTOP — session dodged in escalation modality (floor $($o.floor) >= prior $priorFloor); driver auto-filed + parked."
                Journal "$func s$sessionN [escalation] AUTO-FILED by driver backstop (session did not self-file): $ref"
                git -C $Root add -- memory/grind docs/grind metrics/events.jsonl engine/queue.json 2>$null
                git -C $Root commit -m "grind: $func auto-escalated owner-gated (backstop) [skip-park-src-guard]" 2>$null | Out-Null
            } else {
                python tools/grinder/grindlib.py apply . $func $outPath $modality | Out-Null
                Revert-SessionEdits
                Log "${func}: progress applied — floor=$($o.floor), '$($o.headline)'"
                Journal "$func s$sessionN [$modality] floor=$($o.floor): $($o.headline)"
                git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
                git -C $Root commit -m "grind: $func ledger s$sessionN update [skip-park-src-guard]" 2>$null | Out-Null
            }
        }
    }
    # spec: oracle checked around every session — src is reverted (or merged) by
    # this point, so any red here means real corruption -> stop, don't limp.
    if (-not (Test-OracleGreen)) { Circuit-Break "oracle not green after session on $func" }
    if ($Once) { break }
}
Remove-Item $PidFile -ErrorAction SilentlyContinue
Log "grinder stopped."
