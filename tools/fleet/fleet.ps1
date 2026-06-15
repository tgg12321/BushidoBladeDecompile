#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Supervisor for the Autonomous Decomp Fleet. The ONLY process that mutates `main`.

.DESCRIPTION
  Launches the producer lanes (2 backlog workers, 1 blocked worker, 1 adjudicator)
  as isolated background processes, then runs a single-threaded monitor loop that:
    - restarts any dead/stalled producer lane,
    - drives the Auditor inline: claim IN_REVIEW (or idle REAUDIT) work, run the
      auditor agent, and on PASS perform the privileged merge-to-main + full oracle
      gate (rolling back on any failure),
    - periodically re-verifies the oracle as a backstop,
    - on any oracle break / toolchain failure, trips the circuit and runs the
      Overseer agent, then executes its recovery decision (resume / revert / halt).

  Three invariants keep `main` safe even if a lane misbehaves:
    1. Only this supervisor merges to main (producers live in throwaway worktrees).
    2. Every merge is oracle-gated (full SHA1 == oracle) and rolled back on failure.
    3. On break, the Overseer restores main to its last green commit, or halts.

  Design: docs/superpowers/specs/2026-06-15-autonomous-decomp-fleet-design.md

.EXAMPLE
  pwsh tools/fleet/fleet.ps1 -DryRun
  pwsh tools/fleet/fleet.ps1 -MaxMinutes 30        # bounded (drill)
  pwsh tools/fleet/fleet.ps1                        # unattended, runs until halted
#>
[CmdletBinding()]
param(
    [string]$Model = 'opus',
    [int]$MaxMinutes = 0,                 # 0 = run forever
    [int]$OracleBackstopMinutes = 30,
    [int]$LaneTimeoutMinutes = 120,       # heartbeat-stale kill threshold
    [switch]$DryRun,
    [switch]$NoLanes,                     # supervisor-only (drill: drive auditor by hand)
    [switch]$AuditOnce,                   # drill: run exactly one Auditor-Cycle then exit
    [switch]$SelfTest                     # drill: deterministic merge-rollback + overseer wiring proofs
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_fleet_common.ps1')

$env:CLAUDE_CODE_SESSION_ID = 'fleet-supervisor'   # stable reintegration-lock identity
$LogDir = Join-Path $RepoRoot 'tmp\fleet\logs'
if (-not (Test-Path $LogDir)) { New-Item -ItemType Directory -Path $LogDir -Force | Out-Null }
$ReintLock = Join-Path $RepoRoot 'tools\reintegrate_lock.ps1'

$ProducerLanes = @(
    @{ id = 'fleet-bw1'; role = 'backlog-worker' },
    @{ id = 'fleet-bw2'; role = 'backlog-worker' },
    @{ id = 'fleet-blk'; role = 'blocked-worker' },
    @{ id = 'fleet-adj'; role = 'adjudicator'    }
)

$script:LastGreen = $null

# ───────────────────────────── privileged merge ──────────────────────────────
function Merge-Candidate([string]$Func, [string]$Sha) {
    # Promote a candidate to main. Apply the worker's WHOLE candidate diff (src +
    # ALL build-input edits it made: regfix/asmfix/prologue_config/etc.) with
    # `cherry-pick -n`, then REBUILD + run `queue done` as the comprehensive gate
    # (0 rules + 0 prologue_fix + 0 cheat-asm + SHA1 == oracle). Commit src+config+
    # queue.json in one scoped commit. Roll back to preHead on ANY failure so main is
    # never left dirty or non-green. Single-writer-per-.c (coord Busy-Files) makes the
    # src apply conflict-free; config deletions are distinct-line and 3-way-merge
    # cleanly, and a rare genuine conflict aborts -> reject (retry later).
    # Returns @{ ok; reason; head }.
    $main = Get-MainRoot
    if (-not $Sha) { return @{ ok = $false; reason = 'no candidate sha' } }
    git -C "$main" cat-file -e "$Sha^{commit}" 2>$null
    if ($LASTEXITCODE -ne 0) { return @{ ok = $false; reason = "candidate sha $Sha not found" } }

    if ((Get-MainDirtyLines).Count) { git -C "$main" checkout -- . 2>&1 | Out-Null }   # discard stray (agents never edit main; telemetry ignored)
    $script:_mcPreHead = (git -C "$main" rev-parse HEAD | Out-String).Trim()

    & $ReintLock acquire -Label "fleet-merge $Func" 2>&1 | Out-Null
    try {
        # 1) apply the candidate diff (no auto-commit so we control the final commit)
        git -C "$main" cherry-pick -n $Sha 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            git -C "$main" cherry-pick --abort 2>&1 | Out-Null
            git -C "$main" reset --hard $script:_mcPreHead 2>&1 | Out-Null
            git -C "$main" checkout -- . 2>&1 | Out-Null
            return @{ ok = $false; reason = 'merge-conflict' }
        }
        # drop any candidate change to the shared append-only telemetry log (both main
        # and worktrees write it; merging the two append-streams is meaningless)
        git -C "$main" checkout HEAD -- metrics/events.jsonl 2>$null

        # 1b) ANTI-TAMPER: a candidate may only touch source + its own build-input
        #     config. Reject anything that edits engine/, .claude/, tools/ (except the
        #     prologue config), the Makefile, or linker scripts — those would let a
        #     worker weaken a detector or self-sanction a rule to fake a match.
        $staged = @(git -C "$main" diff --cached --name-only | Where-Object { $_ })
        $allowedExact = @('regfix.txt','regfix_stage2.txt','asmfix.txt','inline_asm_canonical.txt',
                          'tools/prologue_config.json','tools/delay_slot_ra_funcs.txt','tools/frame_fix_funcs.txt',
                          'named_syms.txt','metrics/events.jsonl')
        $bad = @($staged | Where-Object { -not ($_ -like 'src/*' -or $_ -like 'include/*' -or $_ -like 'memory/wip/*' -or ($allowedExact -contains $_)) })
        if ($bad.Count) {
            Restore-MainTo $script:_mcPreHead
            return @{ ok = $false; reason = "scope violation: candidate touched disallowed path(s): $($bad -join ', ')" }
        }

        # 2) rebuild build/ from the new source + oracle SHA1 gate (cherry-pick only
        #    changed source — build/ MUST be rebuilt before queue done trusts it)
        if (-not (Test-OracleRebuild)) { Restore-MainTo $script:_mcPreHead; return @{ ok = $false; reason = 'oracle-gate-failed' } }

        # 3) comprehensive completion gate: 0 rules + 0 prologue_fix + 0 cheat-asm + SHA1
        $qd = Invoke-WtEng main queue done $Func
        if (-not $qd -or -not $qd.ok) {
            $why = if ($qd) { [string]$qd.reason } else { 'queue done produced no verdict' }
            Restore-MainTo $script:_mcPreHead
            return @{ ok = $false; reason = "queue-done-refused: $why" }
        }

        # 4) commit: cherry-pick -n already staged the candidate's files; add queue.json.
        git -C "$main" add -- engine/queue.json 2>&1 | Out-Null
        git -C "$main" commit -m "Match: $Func — COMPLETED-C (autonomous fleet)" 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) { Restore-MainTo $script:_mcPreHead; return @{ ok = $false; reason = 'commit failed' } }

        # 5) the tree must be clean (build/ + tmp/ gitignored; telemetry ignored)
        $postState = Get-MainDirtyLines
        if ($postState.Count) { Restore-MainTo $script:_mcPreHead; return @{ ok = $false; reason = "main dirty after commit: $($postState -join ',')" } }

        return @{ ok = $true; head = (git -C "$main" rev-parse HEAD | Out-String).Trim() }
    } finally {
        & $ReintLock release 2>&1 | Out-Null
    }
}

function Restore-MainTo([string]$Head) {
    $main = Get-MainRoot
    git -C "$main" cherry-pick --abort 2>&1 | Out-Null   # no-op if not mid-pick
    git -C "$main" reset --hard $Head 2>&1 | Out-Null
    git -C "$main" checkout -- . 2>&1 | Out-Null
    Test-OracleRebuild | Out-Null   # rebuild build/ back to the green baseline
}

# ───────────────────────────── auditor (inline) ──────────────────────────────
function Get-ReviewerPrecheck([string]$Func, [string]$Sha) {
    # Maj-6: mechanical pre-check so the auditor spends its judgment on semantics.
    $main = Get-MainRoot
    if ($main -match '^([A-Za-z]):[\\/](.*)$') { $wsl = "/mnt/$($Matches[1].ToLower())/$($Matches[2] -replace '\\','/')" } else { $wsl = $main -replace '\\','/' }
    $arg = if ($Sha) { "--func $Func --commit $Sha" } else { "--func $Func" }
    try { return (wsl bash -c "cd '$wsl' && source .venv/bin/activate && python3 tools/reviewer_precheck.py $arg 2>/dev/null" | Out-String) } catch { return '' }
}

function Build-AuditorTask($pkt, [string]$Layer = 'first', [string]$Precheck = '') {
    if ([string]$pkt.mode -eq 'reaudit') {
@"
You are the fleet AUDITOR. MODE: REAUDIT (no candidate is in review).
Re-audit the already-COMPLETED function $($pkt.func) against the current cheat lens.
FIRST read .claude/agents/cheat-reviewer.md in full — its checklist + carve-out
prerequisites are authoritative; do not reason about them from memory.
Locate its current source in main (e.g. grep -rl '$($pkt.func)' src/) and read the
function body as it stands in main. Apply the 6-test checklist. This is the standing
patrol that keeps everything already committed at the SOTN standard.
Emit reaudit-clean (still meets the bar) or reaudit-regressed (you found a cheat —
include evidence + reason so a worker can redo it cleanly).
"@
    } else {
        $ruling = ''
        if ($pkt.ruling) { $ruling = "An Adjudicator ruling is attached (verify it INDEPENDENTLY — it advises, you still gate; if verdict is sotn-family:<name> and that family is NOT in the frozen list in .claude/rules/no-new-park-categories.md, do NOT pass it — emit needs-adjudication): $($pkt.ruling | ConvertTo-Json -Compress)" }
        $layerNote = if ($Layer -eq 'second') { "You are the INDEPENDENT SECOND-LAYER reviewer. A first adversarial reviewer already returned PASS on this candidate; that verdict is NOT credited to you and you must re-derive the judgment yourself from the source. The two-layer rule exists because one adversarial pass measurably leaks cheats." } else { "" }
        $pc = if ($Precheck) { "MECHANICAL PRE-CHECK (read; necessary but NOT sufficient — your semantic review is authoritative):`n$Precheck" } else { "" }
@"
You are the fleet AUDITOR. MODE: NORMAL. A candidate for $($pkt.func) is IN REVIEW.
$layerNote
FIRST read .claude/agents/cheat-reviewer.md in full — its 6-test checklist and the
exact carve-out prerequisites (do-while-zero; extern-volatile-IRQ) are authoritative;
do not reason about them from memory.
The candidate is commit $($pkt.candidate_sha) (branch $($pkt.candidate_branch)).
Review the full change: git show $($pkt.candidate_sha)  and  git -C "$(Get-MainRoot)" diff $($pkt.candidate_sha)~1 $($pkt.candidate_sha)
Read the function's asm and a matched sibling in the same .c; read memory/wip/$($pkt.func)/.
$ruling
$pc
Apply the 6-test checklist to EVERY construct. Default to FAIL; when torn, FAIL. Emit
pass / fail / needs-adjudication. On a PASS the supervisor merges this to main and
re-runs the full oracle gate — so PASS means COMPLETED. Mean it.
"@
    }
}

function Auditor-Cycle {
    $pkt = $null
    try { $pkt = Coord claim -Role auditor -Lane fleet-aud | ConvertFrom-Json } catch { }
    if (-not $pkt -or -not $pkt.func) { return $false }
    $func = [string]$pkt.func
    $mode = [string]$pkt.mode
    try {
        Auditor-Cycle-Body $pkt $func $mode
    } catch {
        # any throw after the claim would otherwise strand the in_review item (its lane
        # heartbeat keeps refreshing). Requeue it immediately so it's re-reviewable.
        Write-Host "[auditor] cycle threw on ${func}: $_ — requeuing." -ForegroundColor Yellow
        if ($mode -ne 'reaudit') { try { Coord submit -Lane fleet-aud -Outcome requeue-review -Func $func | Out-Null } catch { } }
        Append-FleetLog 'auditor-throw' @{ func = $func; err = "$_" }
    }
    return $true
}

function Auditor-Cycle-Body($pkt, $func, $mode) {
    Write-Host "[auditor] $func (mode=$mode)"
    $precheck = if ($mode -ne 'reaudit') { Get-ReviewerPrecheck $func ([string]$pkt.candidate_sha) } else { '' }
    $task = Build-AuditorTask $pkt 'first' $precheck
    $outcome = Invoke-RoleAgent -Role auditor -Lane fleet-aud -TaskText $task -Model $Model
    Assert-MainClean | Out-Null   # Maj-8: the review agent must not have edited main

    if (-not $outcome -or -not $outcome.outcome) {
        Write-Host "[auditor] no verdict for $func (mode=$mode)." -ForegroundColor Yellow
        if ($mode -ne 'reaudit') { Coord submit -Lane fleet-aud -Outcome requeue-review -Func $func | Out-Null }  # B6: don't strand
        return $true
    }
    $o = [string]$outcome.outcome
    Write-Host "[auditor] $func -> $o"
    switch ($o) {
        'pass' {
            # B1: mandatory INDEPENDENT second-layer review before any merge.
            Write-Host "[auditor] $func passed layer 1; running independent second-layer verifier ..."
            $task2 = Build-AuditorTask $pkt 'second' $precheck
            $v = Invoke-RoleAgent -Role verifier -Lane fleet-aud2 -TaskText $task2 -Model $Model
            Assert-MainClean | Out-Null
            $v2 = if ($v) { [string]$v.outcome } else { '' }
            if ($v2 -ne 'pass') {
                Write-Host "[auditor] second layer did NOT pass ($v2) — not merging." -ForegroundColor Yellow
                if ($v2 -eq 'needs-adjudication') { Coord submit -Lane fleet-aud -Outcome to-adjudicator -Func $func | Out-Null }
                elseif ($v2 -eq 'fail')           { Coord submit -Lane fleet-aud -Outcome rejected -Func $func -Reason ("second-layer FAIL: " + [string]$v.reason) | Out-Null }
                else                              { Coord submit -Lane fleet-aud -Outcome requeue-review -Func $func | Out-Null }
                Append-FleetLog 'second-layer-block' @{ func = $func; verdict = $v2 }
                return $true
            }
            # both layers passed -> privileged merge
            $m = Merge-Candidate $func ([string]$pkt.candidate_sha)
            if ($m.ok) {
                Coord submit -Lane fleet-aud -Outcome approved -Func $func | Out-Null
                $script:LastGreen = $m.head
                Append-FleetLog 'completed' @{ func = $func; head = $m.head; via = ([string]$pkt.ruling.verdict) }
                Write-Host "[auditor] MERGED + COMPLETED $func (main @ $($m.head.Substring(0,8)))" -ForegroundColor Green
            }
            else {
                # B8a: PASSed both layers but failed the privileged gate (didn't reproduce
                # the oracle, or queue-done refused = a cheat both layers missed). Main was
                # rolled back to green by Merge-Candidate, so this is NOT a circuit event —
                # mark the sha gate-failed (never re-PASS it) and bounce to the worker.
                Coord submit -Lane fleet-aud -Outcome gate-failed -Func $func -Sha ([string]$pkt.candidate_sha) -Reason ([string]$m.reason) | Out-Null
                Append-FleetLog 'merge-gate-failed' @{ func = $func; reason = $m.reason }
                if ([string]$m.reason -like 'queue-done-refused*') {
                    Write-Host "[auditor] WARNING: both review layers PASSed but queue-done refused ($($m.reason)) — possible missed cheat. Logged." -ForegroundColor Red
                }
            }
        }
        'fail'               { Coord submit -Lane fleet-aud -Outcome rejected -Func $func -Reason ([string]$outcome.reason) | Out-Null }
        'needs-adjudication' { Coord submit -Lane fleet-aud -Outcome to-adjudicator -Func $func | Out-Null }
        'reaudit-clean'      { Append-FleetLog 'reaudit-clean' @{ func = $func } }
        'reaudit-regressed'  {
            Coord submit -Lane fleet-aud -Outcome regressed -Func $func -Sha ([string]$script:LastGreen) -Reason ([string]$outcome.reason) | Out-Null
            Append-FleetLog 'reaudit-regressed' @{ func = $func; reason = ([string]$outcome.reason) }
            # B5: owner-facing committed ledger of a cheat found on main.
            $reg = Join-Path $RepoRoot 'docs\fleet\regressions.md'
            $line = "- $((Get-Date).ToUniversalTime().ToString('o'))  **$func** — $([string]$outcome.reason) (re-opened for clean pure-C redo; the byte-correct cheat remains on main until the clean version lands)"
            Add-Content -Path $reg -Value $line -Encoding utf8
            Write-Host "[auditor] REGRESSION on committed $func — logged to docs/fleet/regressions.md, re-opened for clean redo." -ForegroundColor Red
        }
        default { Write-Host "[auditor] unknown outcome '$o'." -ForegroundColor Yellow }
    }
    return $true
}

# ───────────────────────────── overseer / circuit ────────────────────────────
function Trip-Circuit([string]$Reason) {
    Write-Host "[supervisor] CIRCUIT TRIP: $Reason" -ForegroundColor Red
    Coord circuit -Set paused -Reason $Reason | Out-Null
    Append-FleetLog 'circuit-trip' @{ reason = $Reason }
    Handle-Incident $Reason
}

function Handle-Incident([string]$Reason) {
    $main = Get-MainRoot
    $task = @"
You are the fleet OVERSEER. An incident tripped the circuit breaker.
INCIDENT: $Reason
Last known green commit: $script:LastGreen
Investigate (read-only): & tools/wteng.ps1 main verify-oracle --rebuild ; git -C "$main" status ; git -C "$main" log --oneline -6 ; inspect tmp/fleet/state.json.
Diagnose, then emit your decision (resume | revert-and-resume | clean-and-resume | rebuild-worktree | halt) with revert_to / worktrees / report fields per your role.
"@
    $d = Invoke-RoleAgent -Role overseer -Lane fleet-ovr -TaskText $task -Model $Model
    $decision = if ($d) { [string]$d.outcome } else { 'halt' }
    Append-FleetLog 'overseer-decision' @{ incident = $Reason; decision = $decision; report = ($d.report) }
    Write-Host "[overseer] decision: $decision" -ForegroundColor Cyan

    & $ReintLock acquire -Label "overseer recovery" 2>&1 | Out-Null
    try {
        switch ($decision) {
            'resume' { }
            'revert-and-resume' {
                $to = if ($d.revert_to) { [string]$d.revert_to } else { $script:LastGreen }
                if ($to) { git -C "$main" reset --hard $to 2>&1 | Out-Null }
            }
            'clean-and-resume' {
                git -C "$main" reset --hard HEAD 2>&1 | Out-Null
            }
            'rebuild-worktree' {
                foreach ($wid in @($d.worktrees)) {
                    if ($wid) {
                        $wt = Join-Path (Split-Path $main) "bb2-work-$wid"
                        $safe = Join-Path $RepoRoot 'tools\safe_remove_worktree.ps1'
                        if (Test-Path $safe) { & $safe $wt -Force 2>&1 | Out-Null }
                        Ensure-Worktree $wid | Out-Null
                    }
                }
            }
            'halt' { }
            default { $decision = 'halt' }
        }
    } finally { & $ReintLock release 2>&1 | Out-Null }

    if ($decision -eq 'halt') {
        Coord circuit -Set halted -Reason $Reason | Out-Null
        Append-FleetLog 'halted' @{ reason = $Reason; report = ($d.report) }
        Write-Host "[supervisor] HALTED. Report left in docs/fleet/incidents.md. Awaiting owner." -ForegroundColor Red
        return
    }

    # only resume if main is genuinely green again
    if (Test-OracleRebuild) {
        $script:LastGreen = (Get-MainHead)
        Coord circuit -Set running -Reason "recovered: $decision" | Out-Null
        Write-Host "[supervisor] recovered ($decision); circuit running. main @ $($script:LastGreen.Substring(0,8))" -ForegroundColor Green
    } else {
        Coord circuit -Set halted -Reason "recovery '$decision' did not restore green oracle" | Out-Null
        Append-FleetLog 'halted' @{ reason = "recovery '$decision' failed to restore oracle" }
        Write-Host "[supervisor] HALTED: recovery did not restore the oracle." -ForegroundColor Red
    }
}

# ───────────────────────────── lane process mgmt ─────────────────────────────
function Start-Lane($lane) {
    $logOut = Join-Path $LogDir "$($lane.id).out.log"
    $logErr = Join-Path $LogDir "$($lane.id).err.log"
    $laneScript = Join-Path $PSScriptRoot 'lane.ps1'
    $p = Start-Process pwsh -PassThru -WindowStyle Hidden -RedirectStandardOutput $logOut -RedirectStandardError $logErr `
        -ArgumentList @('-NoProfile','-File',$laneScript,'-Role',$lane.role,'-Lane',$lane.id,'-Model',$Model)
    $lane.proc = $p
    $lane.started = (Get-Date)
    Write-Host "[supervisor] launched lane $($lane.id) ($($lane.role)) pid=$($p.Id)"
}

function Heartbeat-Age($laneId) {
    try {
        $st = Get-Content (Join-Path $RepoRoot 'tmp\fleet\state.json') -Raw | ConvertFrom-Json -AsHashtable
        $hb = $st.lanes[$laneId].heartbeat
        if (-not $hb) { return 99999 }
        return ([DateTimeOffset]::UtcNow - [DateTimeOffset]::Parse($hb)).TotalMinutes
    } catch { return 0 }
}

# ───────────────────────────── self-test (drill) ─────────────────────────────
function Run-SelfTest {
    $main = Get-MainRoot
    $allPass = $true

    Write-Host "=== SELF-TEST 1: merge gate rollback (a non-matching candidate must NOT survive; main stays green) ===" -ForegroundColor Cyan
    $pre = (git -C "$main" rev-parse HEAD | Out-String).Trim()
    $bad = Join-Path (Split-Path $main) 'bb2-drill-bad'
    git -C "$main" worktree remove --force "$bad" 2>&1 | Out-Null
    git -C "$main" branch -D fleet-drill-bad 2>&1 | Out-Null
    git -C "$main" worktree add -b fleet-drill-bad "$bad" $pre 2>&1 | Out-Null
    $badSha = $null
    try {
        $somec = Join-Path $bad 'src\text1a_c.c'
        Add-Content -Path $somec -Value "`nint __fleet_drill_break__(void){return 12345;}`n"
        git -C "$bad" add -A 2>&1 | Out-Null
        git -C "$bad" commit -m "drill: deliberately non-matching candidate" 2>&1 | Out-Null
        $badSha = (git -C "$bad" rev-parse HEAD | Out-String).Trim()
    } catch { Write-Host "  could not build bad candidate: $_" -ForegroundColor Red; $allPass = $false }

    if ($badSha) {
        $r = Merge-Candidate 'fleet-drill' $badSha
        $headOk  = ((git -C "$main" rev-parse HEAD | Out-String).Trim() -eq $pre)
        $cleanOk = ((Get-MainDirtyLines).Count -eq 0)
        $green   = Test-OracleRebuild
        Write-Host "  Merge-Candidate -> ok=$($r.ok) reason=$($r.reason)"
        Write-Host "  main@preHead=$headOk  clean=$cleanOk  oracle-green=$green"
        if ($r.ok -or -not $headOk -or -not $cleanOk -or -not $green) {
            Write-Host "  SELF-TEST 1 FAILED" -ForegroundColor Red; $allPass = $false
        } else {
            Write-Host "  SELF-TEST 1 PASSED — non-matching candidate rejected, main untouched + green." -ForegroundColor Green
        }
    }
    $safe = Join-Path $RepoRoot 'tools\safe_remove_worktree.ps1'
    if (Test-Path $safe) { & $safe "$bad" -Force 2>&1 | Out-Null } else { git -C "$main" worktree remove --force "$bad" 2>&1 | Out-Null }
    git -C "$main" branch -D fleet-drill-bad 2>&1 | Out-Null

    Write-Host "=== SELF-TEST 2: overseer wiring (trip breaker with main green -> diagnose -> re-arm) ===" -ForegroundColor Cyan
    Coord circuit -Set running -Reason 'self-test reset' | Out-Null
    Handle-Incident "drill: simulated incident (main is actually green)"
    $circuit = $null
    try { $circuit = (Coord circuit | ConvertFrom-Json).state } catch { }
    Write-Host "  circuit after overseer recovery: $circuit"
    if ($circuit -eq 'running') { Write-Host "  SELF-TEST 2 PASSED — overseer diagnosed green + re-armed the fleet." -ForegroundColor Green }
    else { Write-Host "  SELF-TEST 2 INCONCLUSIVE — circuit=$circuit (overseer chose not to resume)." -ForegroundColor Yellow }

    if ($allPass) { Write-Host "`nSELF-TEST: core safety invariants hold." -ForegroundColor Green }
    else { Write-Host "`nSELF-TEST: a safety invariant FAILED — do not launch unattended." -ForegroundColor Red }
    return $allPass
}

# ─────────────────────────────────── main ────────────────────────────────────
Write-Host "=== Autonomous Decomp Fleet supervisor ===" -ForegroundColor Cyan
Write-Host "[supervisor] pre-flight: verify-oracle --rebuild on main ..."
if (-not (Test-OracleRebuild)) {
    Write-Host "[supervisor] ABORT: main does not build to the oracle. Fix the tree before launching." -ForegroundColor Red
    exit 1
}
$script:LastGreen = (Get-MainHead)
Write-Host "[supervisor] baseline green. main @ $($script:LastGreen.Substring(0,8))"

# Drill modes reuse the real privileged code paths but do NOT init/clobber state
# or launch lanes.
if ($SelfTest)  { $ok = Run-SelfTest; exit ([int](-not $ok)) }
if ($AuditOnce) { Coord circuit -Set running -Reason 'audit-once' | Out-Null; Auditor-Cycle | Out-Null; exit 0 }

Coord init | Out-Host
Coord circuit -Set running -Reason 'fleet start' | Out-Null

if ($DryRun) {
    Write-Host "[supervisor] DRY RUN — would launch lanes:" -ForegroundColor Yellow
    $ProducerLanes | ForEach-Object { Write-Host "  $($_.id)  $($_.role)" }
    Write-Host "  + inline auditor (fleet-aud) + overseer (fleet-ovr) driven by supervisor"
    Coord status
    exit 0
}

# provision + launch producer lanes
if (-not $NoLanes) {
    foreach ($lane in $ProducerLanes) {
        Write-Host "[supervisor] provisioning worktree for $($lane.id) ..."
        try { Ensure-Worktree $lane.id | Out-Null } catch { Write-Host "  worktree provisioning issue for $($lane.id): $_" -ForegroundColor Yellow }
        Start-Lane $lane
    }
}

$start = Get-Date
$lastBackstop = Get-Date
Write-Host "[supervisor] entering monitor loop (MaxMinutes=$MaxMinutes)."
try {
    while ($true) {
        if ($MaxMinutes -gt 0 -and ((Get-Date) - $start).TotalMinutes -ge $MaxMinutes) {
            Write-Host "[supervisor] MaxMinutes reached; stopping."; break
        }
        $circuit = $null
        try { $circuit = (Coord circuit | ConvertFrom-Json).state } catch { $circuit = 'running' }
        if ($circuit -eq 'halted') { Write-Host "[supervisor] circuit halted; stopping monitor."; break }

        # 0) sweep stranded claims (Maj-10) — keyed off state.json, not live pointers
        try { Coord sweep | Out-Null } catch { }

        # 1) restart dead / stalled producer lanes
        if (-not $NoLanes -and $circuit -eq 'running') {
            foreach ($lane in $ProducerLanes) {
                $dead = (-not $lane.proc) -or ($lane.proc.HasExited)
                $stale = (Heartbeat-Age $lane.id) -gt $LaneTimeoutMinutes
                if ($dead) {
                    Write-Host "[supervisor] lane $($lane.id) is down; restarting." -ForegroundColor Yellow
                    try { Coord release -Lane $lane.id | Out-Null } catch { Append-FleetLog 'release-failed' @{ lane = $lane.id } }
                    Start-Lane $lane
                } elseif ($stale) {
                    Write-Host "[supervisor] lane $($lane.id) heartbeat stale (>$LaneTimeoutMinutes min); recycling." -ForegroundColor Yellow
                    try { $lane.proc.Kill() } catch { }
                    try { Coord release -Lane $lane.id | Out-Null } catch { Append-FleetLog 'release-failed' @{ lane = $lane.id } }
                    Start-Lane $lane
                }
            }
        }

        # 2) auditor cycle (inline; sole main-writer path)
        $didAudit = $false
        if ($circuit -eq 'running') {
            try { $didAudit = Auditor-Cycle } catch { Write-Host "[supervisor] auditor cycle error: $_" -ForegroundColor Red }
        }

        # 3) oracle backstop
        if (((Get-Date) - $lastBackstop).TotalMinutes -ge $OracleBackstopMinutes) {
            Write-Host "[supervisor] oracle backstop check ..."
            if (-not (Test-OracleRebuild)) { Trip-Circuit "oracle backstop: main not green" }
            else { $script:LastGreen = (Get-MainHead) }
            $lastBackstop = Get-Date
            # periodically persist the durable fleet logs to git (non-build files)
            $main = Get-MainRoot
            if ((git -C "$main" status --porcelain docs/fleet | Out-String).Trim()) {
                & $ReintLock acquire -Label 'fleet-log-commit' 2>&1 | Out-Null
                try {
                    git -C "$main" add docs/fleet 2>&1 | Out-Null
                    git -C "$main" commit -m "fleet: log + adjudication/incident/regression records [skip-park-src-guard]" 2>&1 | Out-Null
                } finally { & $ReintLock release 2>&1 | Out-Null }
            }
        }

        if (-not $didAudit) { Start-Sleep -Seconds 15 }
    }
}
finally {
    Write-Host "[supervisor] shutting down lanes ..."
    if (-not $NoLanes) {
        foreach ($lane in $ProducerLanes) {
            if ($lane.proc -and -not $lane.proc.HasExited) { try { $lane.proc.Kill() } catch { } }
        }
    }
    try { & $ReintLock release 2>&1 | Out-Null } catch { }
    Remove-Item Env:\CLAUDE_CODE_SESSION_ID -ErrorAction SilentlyContinue
    Write-Host "[supervisor] final status:"
    try { Coord status } catch { }
}
