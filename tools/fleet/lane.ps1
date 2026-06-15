#!/usr/bin/env pwsh
<#
.SYNOPSIS
  One PRODUCER lane of the Autonomous Decomp Fleet (backlog-worker, blocked-worker,
  or adjudicator). Runs as a background job under the supervisor (fleet.ps1).

.DESCRIPTION
  Loop: claim a function from the coordinator for this role -> sync this lane's
  isolated worktree to main HEAD -> spawn the role agent (claude -p) to work it ->
  read the agent's outcome file -> record the transition back to the coordinator.

  Producers NEVER mutate main. They commit candidates to their own worktree branch;
  the supervisor (the sole main-writer) merges on the Auditor's PASS. The auditor and
  overseer roles are driven inline by the supervisor, NOT here, so that main's build/
  has exactly one writer.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][ValidateSet('backlog-worker','blocked-worker','adjudicator')]
    [string]$Role,
    [Parameter(Mandatory = $true)][string]$Lane,
    [string]$Model = 'opus',
    [int]$PollSeconds = 20,
    [switch]$Once,            # run a single claim/work cycle then exit (drill/test)
    [int]$TimeoutMinutes = 90
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot '_fleet_common.ps1')

function Build-WorkerTask($pkt, $wtId) {
    $fb = $pkt.feedback_path
    $rule = ''
    if ($pkt.ruling) { $rule = "An Adjudicator ruling is attached — treat it as the primary hint: $($pkt.ruling | ConvertTo-Json -Compress)" }
    $fbHint = ''
    if ($pkt.last_feedback) { $fbHint = "Most recent reviewer feedback (address it directly): $($pkt.last_feedback)" }
@"
You are fleet lane '$Lane' (role: $Role) in the autonomous decomp fleet.
Worktree id: '$wtId'  (its directory is ..\bb2-work-$wtId ; the lane runner has
already created + HEAD-synced it to current main for you).

ASSIGNED FUNCTION (work ONLY this one): $($pkt.func)
  file:    $($pkt.file)
  verdict: $($pkt.verdict)   honest pure-C distance: $($pkt.distance)
  feedback path: $fb   (READ it FIRST if it exists)
$fbHint
$rule

Follow your role definition exactly. Use & tools/wteng.ps1 $wtId <cmd> for every
engine/build command. Edit only ..\bb2-work-$wtId\src and the worktree's build
inputs. Do NOT touch main, do NOT run 'queue done', do NOT merge, do NOT push.
"@
}

function Run-Cycle {
    $pktRaw = Coord claim -Role $Role -Lane $Lane
    $pkt = $null
    try { $pkt = $pktRaw | ConvertFrom-Json } catch { }
    if (-not $pkt -or -not $pkt.func) {
        if ($pkt.paused) { Write-Host "[$Lane] circuit $($pkt.circuit); idling." }
        return $false   # nothing claimed
    }
    $func = $pkt.func
    Write-Host "[$Lane/$Role] claimed $func ($($pkt.file)); syncing worktree..."

    try { Ensure-Worktree $Lane | Out-Null }
    catch {
        Write-Host "[$Lane] worktree bootstrap FAILED: $_" -ForegroundColor Yellow
        Coord release -Lane $Lane | Out-Null
        return $true
    }

    $task = Build-WorkerTask $pkt $Lane
    Write-Host "[$Lane/$Role] working $func ..."
    $outcome = Invoke-RoleAgent -Role $Role -Lane $Lane -TaskText $task -Model $Model -TimeoutMinutes $TimeoutMinutes
    Assert-MainClean | Out-Null   # Maj-8: producers edit only their worktree; never main

    if (-not $outcome -or -not $outcome.outcome) {
        Write-Host "[$Lane] agent produced no valid outcome for $func -> no-op." -ForegroundColor Yellow
        Coord submit -Lane $Lane -Outcome no-op -Func $func | Out-Null
        return $true
    }

    $o = [string]$outcome.outcome
    Write-Host "[$Lane/$Role] $func -> $o"
    switch ($o) {
        'in-review'            { Coord submit -Lane $Lane -Outcome in-review -Func $func -Branch "work/$Lane" -Sha ([string]$outcome.sha) -Reason ([string]$outcome.reason) | Out-Null }
        'needs-decision'       { Coord submit -Lane $Lane -Outcome needs-decision -Func $func -Reason ([string]$outcome.reason) | Out-Null }
        'blocked'              { Coord submit -Lane $Lane -Outcome blocked -Func $func -Reason ([string]$outcome.reason) | Out-Null }
        'to-review'            { Coord submit -Lane $Lane -Outcome to-review -Func $func -Verdict ([string]$outcome.verdict) -Rationale ([string]$outcome.rationale) -Branch ([string]$outcome.branch) -Sha ([string]$outcome.sha) | Out-Null }
        'canonical-authorized' {
            Coord submit -Lane $Lane -Outcome canonical-authorized -Func $func -Rationale ([string]$outcome.rationale) -Branch ([string]$outcome.branch) -Sha ([string]$outcome.sha) | Out-Null
            $po = Join-Path (Resolve-Path (Join-Path $PSScriptRoot '..\..')) 'docs\fleet\pending_owner.md'
            Add-Content -Path $po -Value "- $func — CANONICAL-ASM authorized by Adjudicator (NOT auto-merged; awaiting your sign-off). Rationale: $([string]$outcome.rationale)" -Encoding utf8
        }
        'needs-owner'          {
            Coord submit -Lane $Lane -Outcome needs-owner -Func $func -Rationale ([string]$outcome.rationale) | Out-Null
            $po = Join-Path (Resolve-Path (Join-Path $PSScriptRoot '..\..')) 'docs\fleet\pending_owner.md'
            Add-Content -Path $po -Value "- $func — NOVEL technique, needs your SOTN sign-off (NOT auto-merged). Rationale: $([string]$outcome.rationale)" -Encoding utf8
        }
        'go-back'              { Coord submit -Lane $Lane -Outcome go-back -Func $func -Rationale ([string]$outcome.rationale) -Reason ([string]$outcome.reason) | Out-Null }
        default {
            Write-Host "[$Lane] unrecognized outcome '$o' -> no-op." -ForegroundColor Yellow
            Coord submit -Lane $Lane -Outcome no-op -Func $func | Out-Null
        }
    }
    Append-FleetLog 'lane-cycle' @{ lane = $Lane; role = $Role; func = $func; outcome = $o }
    return $true
}

Write-Host "[$Lane/$Role] lane runner starting (poll ${PollSeconds}s, once=$Once)."
do {
    # honor the circuit breaker
    $circuit = $null
    try { $circuit = (Coord circuit | ConvertFrom-Json).state } catch { $circuit = 'running' }
    if ($circuit -ne 'running') {
        if ($Once) { break }
        Start-Sleep -Seconds $PollSeconds
        continue
    }
    Coord heartbeat -Lane $Lane | Out-Null
    $did = $false
    try { $did = Run-Cycle }
    catch { Write-Host "[$Lane] cycle error: $_" -ForegroundColor Red; try { Coord release -Lane $Lane | Out-Null } catch {} }
    if ($Once) { break }
    if (-not $did) { Start-Sleep -Seconds $PollSeconds }
} while ($true)
Write-Host "[$Lane/$Role] lane runner exiting."
