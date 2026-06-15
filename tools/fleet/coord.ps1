#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Coordinator / message-bus for the Autonomous Decomp Fleet.

.DESCRIPTION
  The single source of truth for LANE STATE that engine/queue.json does not model:
  IN_REVIEW, NEEDS_DECISION, and actively-worked BLOCKED (with the 10-try
  rotate-to-bottom rule). It is ADDITIVE over engine/queue.json — it reads the
  queue for the incomplete-work set + easiest-first ordering, and owns the routing
  itself. engine/queue.py is pinned substrate and is never mutated here.

  All state mutations go through this one script under a named mutex, so the
  concurrent lanes (background processes) can never corrupt the state file.
  State lives at tmp/fleet/state.json (gitignored, persists on disk so the
  supervisor resumes after a crash/restart).

  Design: docs/superpowers/specs/2026-06-15-autonomous-decomp-fleet-design.md

.PARAMETER Command
  init | claim | submit | release | status | heartbeat | circuit | reaudit-seed | dump

  init        Seed/refresh function entries from engine/queue.json (preserves
              existing lane_state/attempts). Idempotent.
  claim       -Role <backlog-worker|blocked-worker|auditor|adjudicator>
              -Lane <id>
              Atomically pick + lock the next work item for that role. Emits the
              work packet as JSON on stdout (or {"func":null} if none).
  submit      -Lane <id> -Outcome <o> [-Func F] [-Note s] [-Reason s]
              [-Branch b] [-Sha s] [-Verdict v] [-Rationale s]
              Record a lane's result and transition state.
  release     -Lane <id>   Drop a lane's claim, return its item to its origin
                           state (crash recovery).
  status      Human-readable counts + lane heartbeats + circuit state.
  heartbeat   -Lane <id>   Touch a lane's heartbeat.
  circuit     [-Set running|paused|halted] [-Reason s]   Get/set the global
              circuit-breaker the supervisor + overseer use.
  reaudit-seed -File <path>  Seed the historical re-audit worklist from a file of
              completed function names (one per line). Used by seed_reaudit.ps1.
  dump        Print raw state.json.

.NOTES
  Outcomes by role (enforced loosely; the lane runner passes the right one):
    worker    : in-review | blocked | needs-decision | no-op
    auditor   : approved  | rejected | to-adjudicator | regressed
    adjudicator: to-review | canonical-authorized | go-back
#>
[CmdletBinding()]
param(
    [Parameter(Position = 0, Mandatory = $true)]
    [ValidateSet('init','claim','submit','release','status','heartbeat','circuit','reaudit-seed','sweep','dump')]
    [string]$Command,

    [string]$Role,
    [string]$Lane,
    [string]$Outcome,
    [string]$Func,
    [string]$Note,
    [string]$Reason,
    [string]$Branch,
    [string]$Sha,
    [string]$Verdict,
    [string]$Rationale,
    [string]$Set,
    [string]$File
)

$ErrorActionPreference = 'Stop'

# Repo root = two levels up from this script (tools/fleet/coord.ps1 -> root).
$root      = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$fleetDir  = Join-Path $root 'tmp\fleet'
$statePath = Join-Path $fleetDir 'state.json'
$queuePath = Join-Path $root 'engine\queue.json'
$BLOCKED_ROTATE_AT   = 10   # genuine blocked attempts before rotate-to-bottom
$REJECT_ADJUDICATE_AT = 3   # every Nth rejection of one func routes it to the Adjudicator
$LANE_STALE_MINUTES  = 120  # claim is stranded if its lane heartbeat is older than this

if (-not (Test-Path $fleetDir)) { New-Item -ItemType Directory -Path $fleetDir -Force | Out-Null }

# ── time helper (Date.now is fine here; this is a live tool, not a workflow) ──
function Now-Iso { (Get-Date).ToUniversalTime().ToString('o') }
function Now-Unix { [int64]([DateTimeOffset]::UtcNow.ToUnixTimeSeconds()) }

# ── state load/save (atomic: temp file + replace, all under the caller's mutex) ─
function Load-State {
    if (-not (Test-Path $statePath)) {
        return [ordered]@{
            version    = 1
            updated_at = (Now-Iso)
            functions  = @{}
            lanes      = @{}
            reaudit    = @{ pending = @(); cursor = 0 }
            gate_failed = @{}      # candidate shas that PASSed review but failed the privileged merge gate
            regressions = @()      # funcs found cheaty by re-audit of committed code (owner-facing)
            circuit    = @{ state = 'running'; reason = ''; since = (Now-Iso) }
        }
    }
    $st = Get-Content $statePath -Raw -Encoding utf8 | ConvertFrom-Json -AsHashtable
    if (-not $st.ContainsKey('gate_failed')) { $st.gate_failed = @{} }
    if (-not $st.ContainsKey('regressions')) { $st.regressions = @() }
    if (-not $st.reaudit) { $st.reaudit = @{ pending = @(); cursor = 0 } }
    return $st
}
function Save-State($st) {
    $st.updated_at = (Now-Iso)
    $tmp = "$statePath.tmp"
    ($st | ConvertTo-Json -Depth 30) | Set-Content -Path $tmp -Encoding utf8
    Move-Item -Path $tmp -Destination $statePath -Force
}

# ── mutex wrapper: every state op runs the scriptblock under exclusive lock ────
function With-Lock([scriptblock]$Body) {
    $mtx = New-Object System.Threading.Mutex($false, 'bb2_fleet_coord_mutex')
    $got = $false
    try {
        $got = $mtx.WaitOne([TimeSpan]::FromSeconds(60))
        if (-not $got) { throw 'coord: could not acquire state mutex within 60s' }
        & $Body
    } finally {
        if ($got) { $mtx.ReleaseMutex() | Out-Null }
        $mtx.Dispose()
    }
}

# ── routing helper: queue verdict/status -> initial fleet lane_state ──────────
function Initial-LaneState($item) {
    $status  = [string]$item.status
    $verdict = [string]$item.verdict
    if ($verdict -eq 'JTBL-INFRA') { return 'needs_decision' }  # known no-C-form -> expert
    switch ($status) {
        'active'    { return 'backlog' }
        'authorize' { return 'blocked' }   # gate's GUESS it's asm; no-quit grinds pure-C first
        'parked'    { return 'blocked' }
        default     { return 'backlog' }
    }
}

# A .c file is BUSY if any function in it is in flight ANYWHERE in the pipeline
# (in_progress, in_review, or needs_decision with a pending candidate). This keeps
# each translation unit single-writer from claim through merge, which is what makes
# the supervisor's merge conflict-free (no two candidates touch the same .c).
function Busy-Files($st) {
    $busy = @{}
    foreach ($k in $st.functions.Keys) {
        $e = $st.functions[$k]
        $inflight = ($e.lane_state -in @('in_progress','in_review')) -or
                    ($e.lane_state -eq 'needs_decision' -and $e.candidate_sha)
        if ($inflight -and $e.file) { $busy[$e.file] = $true }
    }
    return $busy
}

function Touch-Lane($st, $laneId, $role, $func) {
    if (-not $st.lanes[$laneId]) { $st.lanes[$laneId] = @{} }
    $st.lanes[$laneId].role        = $role
    $st.lanes[$laneId].current_func = $func
    $st.lanes[$laneId].heartbeat   = (Now-Iso)
}

function Min-Order($st) {
    $vals = @($st.functions.Values | ForEach-Object { [double]$_.order })
    if ($vals.Count -eq 0) { return 0.0 } else { return ($vals | Measure-Object -Minimum).Minimum }
}
function Max-Order($st) {
    $vals = @($st.functions.Values | ForEach-Object { [double]$_.order })
    if ($vals.Count -eq 0) { return 0.0 } else { return ($vals | Measure-Object -Maximum).Maximum }
}

# ───────────────────────────────── commands ──────────────────────────────────

switch ($Command) {

'init' {
    With-Lock {
        $st = Load-State
        if (-not (Test-Path $queuePath)) { throw "coord init: $queuePath not found" }
        $q = Get-Content $queuePath -Raw -Encoding utf8 | ConvertFrom-Json -AsHashtable
        $idx = 0
        foreach ($item in $q.items) {
            $f = [string]$item.func
            $idx++
            if ($st.functions[$f]) {
                # preserve lane_state/attempts; refresh file + verdict + rule metadata
                $st.functions[$f].file    = [string]$item.file
                $st.functions[$f].verdict = [string]$item.verdict
                $st.functions[$f].rules   = [int]$item.rules
                continue
            }
            $st.functions[$f] = @{
                func             = $f
                file             = [string]$item.file
                verdict          = [string]$item.verdict
                distance         = $item.distance
                rules            = [int]$item.rules
                lane_state       = (Initial-LaneState $item)
                home             = $(if ((Initial-LaneState $item) -eq 'backlog') { 'backlog' } else { 'blocked' })
                order            = [double]$idx
                claimed_by       = $null
                attempts         = 0
                lifetime_attempts= 0
                blocked_attempts = 0
                reject_count     = 0
                feedback_path    = "memory/wip/$f/"
                candidate_branch = $null
                candidate_sha    = $null
                ruling           = $null
                regression       = $false
                history          = @()
                created_at       = (Now-Iso)
            }
        }
        Save-State $st
        $counts = @{}
        foreach ($e in $st.functions.Values) {
            $ls = $e.lane_state; if (-not $counts[$ls]) { $counts[$ls] = 0 }; $counts[$ls]++
        }
        [pscustomobject]@{ ok = $true; total = $st.functions.Count; by_state = $counts } | ConvertTo-Json -Compress
    }
}

'claim' {
    if (-not $Role -or -not $Lane) { throw 'claim requires -Role and -Lane' }
    With-Lock {
        $st = Load-State
        if ($st.circuit.state -ne 'running') {
            [pscustomobject]@{ func = $null; paused = $true; circuit = $st.circuit.state } | ConvertTo-Json -Compress
            return
        }
        Touch-Lane $st $Lane $Role $null
        $pick = $null
        $mode = 'normal'

        if ($Role -eq 'backlog-worker') {
            $busy = Busy-Files $st
            $cands = @($st.functions.Values |
                Where-Object { $_.lane_state -eq 'backlog' -and -not $busy[$_.file] } |
                Sort-Object { [double]$_.order })
            if ($cands.Count -gt 0) { $pick = $cands[0] }
            else {
                # backlog drained -> help the blocked queue instead of idling
                $cands = @($st.functions.Values |
                    Where-Object { $_.lane_state -eq 'blocked' -and -not $busy[$_.file] } |
                    Sort-Object { [double]$_.order })
                if ($cands.Count -gt 0) { $pick = $cands[0] }
            }
        }
        elseif ($Role -eq 'blocked-worker') {
            $busy = Busy-Files $st
            $cands = @($st.functions.Values |
                Where-Object { $_.lane_state -eq 'blocked' -and -not $busy[$_.file] } |
                Sort-Object { [double]$_.order })
            if ($cands.Count -gt 0) { $pick = $cands[0] }
        }
        elseif ($Role -eq 'adjudicator') {
            $cands = @($st.functions.Values |
                Where-Object { $_.lane_state -eq 'needs_decision' -and -not $_.claimed_by } |
                Sort-Object { [double]$_.order })
            if ($cands.Count -gt 0) { $pick = $cands[0] }
        }
        elseif ($Role -eq 'auditor') {
            # 1) anything IN_REVIEW (FIFO by review_ts), skipping shas that already
            #    PASSed review but failed the privileged merge gate (no re-PASS loop)
            $cands = @($st.functions.Values |
                Where-Object { $_.lane_state -eq 'in_review' -and -not $_.claimed_by `
                               -and -not ($_.candidate_sha -and $st.gate_failed[$_.candidate_sha]) } |
                Sort-Object { $_.review_ts })
            if ($cands.Count -gt 0) { $pick = $cands[0] }
            else {
                # 2) idle -> re-audit a completed function (historical seed worklist)
                if ($st.reaudit.pending.Count -gt 0) {
                    $c = [int]$st.reaudit.cursor
                    if ($c -ge $st.reaudit.pending.Count) { $c = 0 }
                    $rf = [string]$st.reaudit.pending[$c]
                    $st.reaudit.cursor = $c + 1
                    Touch-Lane $st $Lane $Role $rf
                    Save-State $st
                    [pscustomobject]@{ func = $rf; file = $null; mode = 'reaudit'; origin = 'completed' } | ConvertTo-Json -Compress
                    return
                }
            }
        }
        else { throw "claim: unknown role '$Role'" }

        if (-not $pick) {
            Save-State $st
            [pscustomobject]@{ func = $null } | ConvertTo-Json -Compress
            return
        }

        $pick.lane_state = 'in_progress'
        $pick.claimed_by = $Lane
        $pick.claimed_role = $Role   # who is working it now (auditor claims do NOT change `home`)
        $pick.claimed_at = (Now-Iso)
        $pick.history += @{ ts = (Now-Iso); lane = $Lane; event = "claim:$Role" }
        Touch-Lane $st $Lane $Role $pick.func
        Save-State $st

        [pscustomobject]@{
            func            = $pick.func
            file            = $pick.file
            verdict         = $pick.verdict
            distance        = $pick.distance
            rules           = $pick.rules
            mode            = $mode
            origin_role     = $Role
            attempts        = $pick.attempts
            lifetime_attempts = $pick.lifetime_attempts
            blocked_attempts= $pick.blocked_attempts
            reject_count    = $pick.reject_count
            feedback_path   = $pick.feedback_path
            last_feedback   = $pick.last_feedback
            candidate_branch= $pick.candidate_branch
            candidate_sha   = $pick.candidate_sha
            ruling          = $pick.ruling
        } | ConvertTo-Json -Compress -Depth 6
    }
}

'submit' {
    if (-not $Lane -or -not $Outcome) { throw 'submit requires -Lane and -Outcome' }
    With-Lock {
        $st = Load-State
        # resolve the function: explicit -Func, else the lane's current claim
        $f = $Func
        if (-not $f -and $st.lanes[$Lane]) { $f = [string]$st.lanes[$Lane].current_func }
        if (-not $f) { throw "submit: no function (pass -Func, or lane '$Lane' has no current claim)" }
        $e = $st.functions[$f]
        if (-not $e) { throw "submit: function '$f' not tracked" }
        if (-not $e.home) { $e.home = 'backlog' }
        $origin = $e.home   # the function's stable tier (backlog vs blocked), not who worked it
        $hist = @{ ts = (Now-Iso); lane = $Lane; event = "submit:$Outcome" }
        if ($Note)   { $hist.note   = $Note }
        if ($Reason) { $hist.reason = $Reason }

        switch ($Outcome) {
            'in-review' {
                $e.lane_state = 'in_review'
                $e.claimed_by = $null
                $e.candidate_branch = $Branch
                $e.candidate_sha = $Sha
                $e.review_ts = (Now-Iso)
                $e.attempts = [int]$e.attempts + 1
                $e.lifetime_attempts = [int]$e.lifetime_attempts + 1
            }
            'blocked' {
                $e.claimed_by = $null
                $e.attempts = [int]$e.attempts + 1
                $e.lifetime_attempts = [int]$e.lifetime_attempts + 1
                $e.lane_state = 'blocked'
                $e.home = 'blocked'         # now blocked-tier; the no-quit worker owns it
                $e.candidate_sha = $null    # stuck: no candidate; frees the .c file
                # Rotate for EVERY blocked attempt regardless of which lane made it, so a
                # backlog-origin function can't head-of-line-block the blocked queue.
                $e.blocked_attempts = [int]$e.blocked_attempts + 1
                if ([int]$e.blocked_attempts -ge $BLOCKED_ROTATE_AT) {
                    $e.order = (Max-Order $st) + 1.0   # rotate to bottom of blocked
                    $e.blocked_attempts = 0
                    $hist.rotated = $true
                }
            }
            'needs-decision' {
                $e.lane_state = 'needs_decision'
                $e.claimed_by = $null
                $e.attempts = [int]$e.attempts + 1
                $e.lifetime_attempts = [int]$e.lifetime_attempts + 1
            }
            'no-op' {
                # lane produced nothing / crashed mid-run: return to origin, drop any
                # half-made candidate so a reused worktree can't carry it forward.
                $e.lane_state = $origin
                $e.claimed_by = $null
                $e.candidate_sha = $null
                $e.attempts = [int]$e.attempts + 1
            }
            # ---- auditor outcomes ----
            'approved' {
                $e.lane_state = 'done'
                $e.claimed_by = $null
                $e.done_ts = (Now-Iso)
                $e.reject_count = 0
                $e.regression = $false    # if this was a regression redo, it's clean now
                # feed the historical re-audit worklist with what we just completed
                if ($st.reaudit.pending -notcontains $f) { $st.reaudit.pending += $f }
            }
            'requeue-review' {
                # B6: auditor produced no verdict — return the item to in_review so it
                # is re-claimable (do NOT strand it claimed-but-unreviewed).
                $e.lane_state = 'in_review'
                $e.claimed_by = $null
            }
            'rejected' {
                # Anti-ratchet (Maj-7): every Nth rejection routes to the Adjudicator
                # for a fresh pure-C path instead of grinding the same near-miss. The
                # feedback is framed as "this was a cheat — find a DIFFERENT structure",
                # not "tweak this to pass".
                $e.claimed_by = $null
                $e.reject_count = [int]$e.reject_count + 1
                $e.lifetime_attempts = [int]$e.lifetime_attempts + 1
                $e.candidate_sha = $null
                if ($Reason) { $e.last_feedback = "REJECTED (treat the rejected form as a CHEAT; do not reproduce it or a near-spelling — find a genuinely different pure-C structure): $Reason" }
                if (([int]$e.reject_count % $REJECT_ADJUDICATE_AT) -eq 0) {
                    $e.lane_state = 'needs_decision'   # escalate to the deep expert
                } else {
                    $e.lane_state = $origin
                    $e.order = (Min-Order $st) - 1.0
                }
            }
            'to-adjudicator' {
                $e.lane_state = 'needs_decision'
                $e.claimed_by = $null
            }
            'gate-failed' {
                # B8a: the candidate PASSed review but failed the privileged merge gate
                # (didn't reproduce the oracle, or queue-done refused). Record the sha so
                # it is never re-PASSed, and bounce the function back to its worker queue.
                if ($Sha) { $st.gate_failed[$Sha] = $true }
                $e.claimed_by = $null
                $e.candidate_sha = $null
                $e.lane_state = $origin
                $e.order = (Min-Order $st) - 1.0
                if ($Reason) { $e.last_feedback = "PRIVILEGED MERGE GATE FAILED ($Reason) — your candidate did not reproduce the oracle on main. Re-derive a genuine match." }
            }
            'regressed' {
                # B5: re-audit found a cheat in an already-committed COMPLETED function.
                # Re-open for a clean redo; main keeps the (byte-correct) cheat until a
                # pure-C replacement lands (reverting now would break the oracle). Record
                # it in the owner-facing ledger and stop re-flagging it via reaudit.
                $e.lane_state = 'backlog'
                $e.claimed_by = $null
                $e.regression = $true
                $e.order = (Min-Order $st) - 1.0
                if ($Reason) { $e.last_feedback = "REGRESSION: this committed function carries a cheat ($Reason). Produce a clean pure-C replacement." }
                $st.regressions = @($st.regressions | Where-Object { $_.func -ne $f }) + @(@{ func = $f; reason = $Reason; sha = $Sha; ts = (Now-Iso) })
                $st.reaudit.pending = @($st.reaudit.pending | Where-Object { $_ -ne $f })  # don't re-flag while open
            }
            # ---- adjudicator outcomes ----
            'to-review' {
                $e.lane_state = 'in_review'
                $e.claimed_by = $null
                $e.review_ts = (Now-Iso)
                $e.ruling = @{ verdict = $Verdict; rationale = $Rationale; ts = (Now-Iso) }
                if ($Branch) { $e.candidate_branch = $Branch }
                if ($Sha)    { $e.candidate_sha = $Sha }
            }
            'canonical-authorized' {
                # B3/Maj-5: canonical-asm is the highest cheat-on-main risk and is
                # irreversible-ish. It does NOT auto-merge — it parks for the owner's
                # review. The supervisor logs it to docs/fleet/pending_owner.md.
                $e.lane_state = 'pending_owner'
                $e.claimed_by = $null
                $e.ruling = @{ verdict = 'canonical-asm'; rationale = $Rationale; ts = (Now-Iso) }
                if ($Branch) { $e.candidate_branch = $Branch }
                if ($Sha)    { $e.candidate_sha = $Sha }
            }
            'needs-owner' {
                # B4: a genuinely NOVEL technique family the Adjudicator cannot sanction
                # autonomously — parks for the owner with the SOTN-evidence question.
                $e.lane_state = 'pending_owner'
                $e.claimed_by = $null
                $e.ruling = @{ verdict = 'novel-needs-owner'; rationale = $Rationale; ts = (Now-Iso) }
            }
            'go-back' {
                $e.lane_state = $origin
                $e.claimed_by = $null
                $e.reject_count = [int]$e.reject_count + 1
                $e.order = (Min-Order $st) - 1.0
                $e.ruling = @{ verdict = 'go-back'; rationale = $Rationale; ts = (Now-Iso) }
                if ($Reason) { $e.last_feedback = "ADJUDICATOR go-back (find the pure-C path described, or avoid the named cheat): $Reason" }
            }
            default { throw "submit: unknown outcome '$Outcome'" }
        }
        $e.history += $hist
        if ($st.lanes[$Lane]) { $st.lanes[$Lane].current_func = $null; $st.lanes[$Lane].heartbeat = (Now-Iso) }
        Save-State $st
        [pscustomobject]@{ ok = $true; func = $f; new_state = $e.lane_state; attempts = $e.attempts; blocked_attempts = $e.blocked_attempts } | ConvertTo-Json -Compress
    }
}

'release' {
    if (-not $Lane) { throw 'release requires -Lane' }
    With-Lock {
        $st = Load-State
        $f = if ($st.lanes[$Lane]) { [string]$st.lanes[$Lane].current_func } else { $null }
        if ($f -and $st.functions[$f] -and $st.functions[$f].claimed_by -eq $Lane) {
            $e = $st.functions[$f]
            # preserve a committed candidate (return to review); else to its home tier
            if ($e.candidate_sha) { $e.lane_state = 'in_review' }
            else { $e.lane_state = if ($e.home -eq 'blocked') { 'blocked' } else { 'backlog' } }
            $e.claimed_by = $null
            $e.history += @{ ts = (Now-Iso); lane = $Lane; event = 'release' }
        }
        if ($st.lanes[$Lane]) { $st.lanes[$Lane].current_func = $null }
        Save-State $st
        [pscustomobject]@{ ok = $true; released = $f } | ConvertTo-Json -Compress
    }
}

'heartbeat' {
    if (-not $Lane) { throw 'heartbeat requires -Lane' }
    With-Lock {
        $st = Load-State
        if (-not $st.lanes[$Lane]) { $st.lanes[$Lane] = @{} }
        $st.lanes[$Lane].heartbeat = (Now-Iso)
        Save-State $st
        '{"ok":true}'
    }
}

'circuit' {
    With-Lock {
        $st = Load-State
        if ($Set) {
            if ($Set -notin @('running','paused','halted')) { throw "circuit -Set must be running|paused|halted" }
            $st.circuit.state = $Set
            $st.circuit.reason = $Reason
            $st.circuit.since = (Now-Iso)
            Save-State $st
        }
        $st.circuit | ConvertTo-Json -Compress
    }
}

'reaudit-seed' {
    if (-not $File) { throw 'reaudit-seed requires -File' }
    if (-not (Test-Path $File)) { throw "reaudit-seed: file not found: $File" }
    With-Lock {
        $st = Load-State
        $names = Get-Content $File | ForEach-Object { $_.Trim() } | Where-Object { $_ -and -not $_.StartsWith('#') }
        $added = 0
        foreach ($n in $names) { if ($st.reaudit.pending -notcontains $n) { $st.reaudit.pending += $n; $added++ } }
        Save-State $st
        [pscustomobject]@{ ok = $true; added = $added; total_pending = $st.reaudit.pending.Count } | ConvertTo-Json -Compress
    }
}

'sweep' {
    # Maj-10: release stranded claims, aged off the per-item `claimed_at` (NOT the
    # lane heartbeat — the inline auditor refreshes fleet-aud's heartbeat every tick,
    # so a thrown auditor agent would otherwise strand its item forever). A swept item
    # with a committed candidate returns to in_review (keep the sha); else to its home.
    With-Lock {
        $st = Load-State
        $now = [DateTimeOffset]::UtcNow
        $freed = @()
        foreach ($k in @($st.functions.Keys)) {
            $e = $st.functions[$k]
            if ($e.lane_state -eq 'in_progress' -and $e.claimed_by) {
                $ageMin = 99999.0
                if ($e.claimed_at) { try { $ageMin = ($now - [DateTimeOffset]::Parse($e.claimed_at)).TotalMinutes } catch { } }
                if ($ageMin -gt $LANE_STALE_MINUTES) {
                    if ($e.candidate_sha) {
                        $e.lane_state = 'in_review'      # preserve the worker's committed candidate
                    } else {
                        $e.lane_state = if ($e.home -eq 'blocked') { 'blocked' } else { 'backlog' }
                    }
                    $e.claimed_by = $null
                    $e.history += @{ ts = (Now-Iso); event = 'swept-stale-claim'; age_min = [int]$ageMin }
                    $freed += $k
                }
            }
        }
        if ($freed.Count) { Save-State $st }
        [pscustomobject]@{ ok = $true; freed = $freed } | ConvertTo-Json -Compress
    }
}

'status' {
    # read-only: no lock needed for a status print
    $st = Load-State
    $counts = @{}
    foreach ($e in $st.functions.Values) {
        $ls = [string]$e.lane_state; if (-not $counts[$ls]) { $counts[$ls] = 0 }; $counts[$ls]++
    }
    Write-Host "=== Fleet status ($($st.updated_at)) ==="
    Write-Host "circuit: $($st.circuit.state)  $(if($st.circuit.reason){"($($st.circuit.reason))"})"
    Write-Host "functions: $($st.functions.Count) tracked"
    foreach ($k in ($counts.Keys | Sort-Object)) { Write-Host ("  {0,-16} {1}" -f $k, $counts[$k]) }
    Write-Host "reaudit pending: $($st.reaudit.pending.Count) (cursor $($st.reaudit.cursor))"
    Write-Host "gate-failed shas: $($st.gate_failed.Count)"
    if ($st.regressions.Count) {
        Write-Host "REGRESSIONS (committed code found cheaty, awaiting clean redo): $($st.regressions.Count)" -ForegroundColor Yellow
        foreach ($r in $st.regressions) { Write-Host "    $($r.func): $($r.reason)" }
    }
    $owner = @($st.functions.Values | Where-Object { $_.lane_state -eq 'pending_owner' })
    if ($owner.Count) {
        Write-Host "PENDING OWNER REVIEW (canonical-asm / novel technique — not auto-merged): $($owner.Count)" -ForegroundColor Yellow
        foreach ($o in $owner) { Write-Host "    $($o.func): $($o.ruling.verdict)" }
    }
    Write-Host "lanes:"
    foreach ($lid in ($st.lanes.Keys | Sort-Object)) {
        $l = $st.lanes[$lid]
        Write-Host ("  {0,-14} role={1,-16} func={2,-22} hb={3}" -f $lid, $l.role, ($l.current_func), $l.heartbeat)
    }
}

'dump' {
    if (Test-Path $statePath) { Get-Content $statePath -Raw } else { '{}' }
}

}
