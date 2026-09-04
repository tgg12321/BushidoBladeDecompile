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
    # Owner directive 2026-08-17 (supersedes the same-day all-Fable switch):
    # split the model by modality — recon (session 1: floor measurement, lever
    # mapping, frontier hypotheses) runs on Fable 5, where model strength pays
    # off most; execution sessions grind pre-built frontiers on Opus. This
    # also keeps Fable's separate per-model allowance from stalling the
    # pipeline (the 2026-08-12 incident: five 429 wait cycles over 2.5 h).
    [string]$Model = 'claude-opus-5[1m]',
    # 2026-09-01: Fable 5.1 released; the `claude-fable-5[1m]` alias still
    # resolves to Fable 5 (probed via `claude -p --output-format json`), so the
    # recon lane is pinned to the new id explicitly. Judge/execution lanes are
    # unchanged (the 2026-08-12 / 2026-08-17 directives are about lane
    # assignment, not model version).
    [string]$ReconModel = 'claude-fable-5-1[1m]',
    # Judge stays on Opus per the 2026-08-12 directive: Fable 5 has its own
    # per-model allowance, and exhausting it on judging stalled the whole
    # pipeline — five 429 wait cycles over 2.5 h with a proven candidate parked
    # in front of the Judge, while every other model still had usage. (The
    # 2026-07-22 backlog audit note about a weaker same-tier fable judge
    # passing a fabricated-evidence cheat is in the ledger-staging note
    # further down.)
    [string]$JudgeModel = 'claude-opus-5[1m]',
    # Layer-1 (the pre-Judge cheat-reviewer gate) runs on the model the agent
    # definition declares — it is a high-volume, cheap gate whose job is to bounce
    # obvious cheat-by-spelling before a Judge cycle is spent.
    [string]$Layer1Model = 'claude-sonnet-5',
    [int]$SessionTimeoutMin = 90,
    [string]$MockSessionScript = '',
    [string]$MockJudgeScript = '',
    [string]$MockLayer1Script = ''
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
function Attribute-RedBuild([string]$When) {
    # Red SHA1 -> function list. Spotcheck attribute mode compares
    # build/bb2.exe against disc/SLUS_006.63 per function via the ELF symbol
    # table (no compile), so the driver logs WHICH functions moved instead of
    # a bare hash mismatch. Best-effort diagnostic: never blocks or alters the
    # failure path. Sleeps past the spotcheck fresh-artifact window (45s) so
    # its preflight does not refuse the tree the failed build just wrote;
    # --ignore-locks because OUR OWN driver lock is legitimately present.
    try {
        Start-Sleep -Seconds 50
        $wslRoot = (& wsl wslpath -a "$Root" 2>$null | Out-String).Trim()
        if (-not $wslRoot) { Log "red-build attribution skipped ($When): wslpath failed"; return }
        $cmd = "cd '$wslRoot' && source .venv/bin/activate && python3 tools/spotcheck/spot_check_completed.py --mode attribute --all --ignore-locks 2>&1 | tail -40"
        $out = (& wsl bash -c $cmd | Out-String)
        Log "red-build attribution (${When}):`n$out"
    } catch { Log "red-build attribution failed (${When}): $_" }
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
    Attribute-RedBuild 'pre-flight'
    Remove-Item $PidFile; exit 2
}
Log "pre-flight: oracle green."
# Nonpaged-pool watchdog (2026-08-12): every wsl.exe invocation leaks ~1 Job +
# ~11 File kernel objects (measured on WSL 2.6.3 AND 2.7.11; see
# tools/check_wsl_leak.ps1 and memory/project/wsl-kernel-object-leak-audio.md).
# Grind volume accumulates GBs of nonpaged pool, which degrades host audio/
# latency and eventually wedges the WSL service; only a reboot frees it.
# Warn loudly so the owner knows a reboot is due — never block the grind.
try {
    $npGB = [math]::Round((Get-Counter '\Memory\Pool Nonpaged Bytes').CounterSamples[0].CookedValue / 1GB, 2)
    if ($npGB -ge 2.5) {
    } elseif ($npGB -ge 1.8) {
    }
} catch { }
# WSL bridge state (2026-08-14): wteng now routes engine calls through one
# long-lived bash session BY DEFAULT, which takes the Job leak above from ~1 per
# call to 0 (measured: direct 1.00/call, bridge 0.00/call). It was opt-in from
# 2026-08-12 and nothing ever set the flag, so it never actually ran — log the
# state every grind so a disabled or silently-failing bridge is visible instead
# of quietly costing pool for another few thousand calls.
if ($env:BB2_WSL_BRIDGE -eq '0') {
    Log "pre-flight WARNING: WSL bridge DISABLED (BB2_WSL_BRIDGE=0) — every engine call leaks a Job object."
} else {
    Log "pre-flight: WSL bridge enabled (default; BB2_WSL_BRIDGE=0 disables). `make` stays on the direct path by design."
}

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

function Record-Review([string]$func, [string]$layer, [string]$verdict, [string]$cause = '') {
    # Review telemetry (2026-08-07 review-audit fix #5). Best-effort by the
    # metrics contract: it can never raise, never block, never gate.
    try { python tools/grinder/record_review.py $func $layer $verdict $cause 2>$null | Out-Null } catch { }
}

function Get-Layer1RoleFile {
    # The cheat-reviewer agent definition is a Claude-Code agent file with YAML
    # frontmatter; --append-system-prompt-file wants the prompt body alone. Strip
    # the frontmatter into tmp/ once per driver run. Returns '' if the definition
    # is missing, which makes layer-1 fail OPEN (the Judge still gates).
    $src = Join-Path $Root '.claude\agents\cheat-reviewer.md'
    if (-not (Test-Path $src)) { return '' }
    $dst = Join-Path $GrindTmp 'layer1_role.md'
    try {
        $lines = @(Get-Content $src)
        if ($lines.Count -and $lines[0].Trim() -eq '---') {
            $end = -1
            for ($i = 1; $i -lt $lines.Count; $i++) { if ($lines[$i].Trim() -eq '---') { $end = $i; break } }
            if ($end -ge 0) { $lines = $lines[($end + 1)..($lines.Count - 1)] }
        }
        Set-Content $dst -Value ($lines -join "`n") -Encoding utf8
        return $dst
    } catch { return '' }
}

function Invoke-Layer1([string]$func, [string]$stem, [string]$diff) {
    # LAYER-1 GATE (2026-08-07 review-audit fix #2b). A fresh adversarial
    # cheat-reviewer rules on the candidate diff + the session's own self-vet
    # BEFORE the Judge is spawned. Rationale from the audit: 46% of Judge FAILs
    # were cheat-by-spelling that a cheap adversarial pass catches, and every one
    # of them burned a full Judge cycle to say so.
    #
    # FAILS OPEN by design. If the reviewer is unreachable or returns nothing
    # parseable, the candidate proceeds to the Judge — which is default-FAIL and
    # is the authoritative gate. Layer-1 exists to SAVE Judge cycles, never to
    # become a second way to lose a bytes-proven candidate to an API hiccup.
    $role = Get-Layer1RoleFile
    if (-not $role) { Log "${func}: layer-1 role file unavailable — skipping to Judge."; return $null }
    $outPath = Join-Path $GrindTmp "layer1_$func.json"
    $briefPath = Join-Path $GrindTmp "layer1_brief_$func.md"
    $led = "memory/grind/$func"
    $vetPath = Join-Path $Root "memory\grind\$func\self_vet.md"
    $vet = if (Test-Path $vetPath) { Get-Content $vetPath -Raw } else { '(no self-vet on disk)' }
    $scopeBlock = ''
    try { $scopeBlock = (python tools/grinder/grindlib.py rule-scopes . $func 2>$null | Out-String).Trim() } catch { }
    # Mechanical pun scan (2026-09-03): per-use address puns on splat symbols in
    # candidate.c — the object model belongs at the declaration (prong (d)).
    $punBlock = ''
    try {
        $puns = (python tools/grinder/grindlib.py pun-scan . $func 2>$null | Out-String).Trim()
        if ($puns) { $punBlock = "DECLARATION PUNS (mechanical scan of candidate.c — a per-use cast or pointer arithmetic on a splat symbol's ADDRESS spells an object model at the use site; the aggregate-merge family requires it at the DECLARATION, prong (d). FAIL unless the self-vet justifies each line on its own evidence):`n$puns`n" }
    } catch { }
    $task = @"
LAYER-1 REVIEW for $func (src/$stem.c) — you are the pre-Judge gate in the
Grinder pipeline. A grind session has produced a candidate whose honest
cheat-invisible sandbox distance is 0. Rule ONLY on whether the C is legitimate
under the cheats-by-any-spelling policy. Default to FAIL.

The candidate diff against HEAD:
``````diff
$diff
``````

The session's OWN self-vet (its written answers to the 6-test checklist, its
claimed sanctioned families with scope sentences + precedents, and its
annotation-conformance line). Treat it as a CLAIM to verify, never as evidence:
``````
$vet
``````

Verify against the ledger yourself: $led/state.json (judge_constraints and
banned_constructs — a re-declared banned construct is an automatic FAIL unless a
later dated family grant supersedes it (see DATED RULINGS below)),
$led/hypotheses.md and $led/evidence.md (the lever-exhaustion the FAKE
prerequisites demand — check it, do not take the claim), $led/rejected/.
Every quoted SCOPE sentence must actually appear in the rule file it cites, and
every PRECEDENT must resolve to the file:line or commit it names. A citation
that does not check out is a FAIL, not a rounding error.

$scopeBlock
$punBlock
DATED RULINGS: rulings in docs/grind/decisions.md carry dates; bans in state.json
do NOT — date a ban from the dated FAIL entry that created it in
docs/grind/decisions.md, or ``git log -S "<ban text>" -- memory/grind/<func>/state.json``.
A family grant in .claude/rules/no-new-park-categories.md dated AFTER a
per-function refusal or ban SUPERSEDES that refusal for the construct it covers.
Before citing a ban or an older ruling as a FAIL ground, date it; if a later
dated grant covers the construct, the ban is stale and the correct verdict is
decided on the grant's own prerequisites.

Write your verdict JSON (the schema in your role prompt: decision / function /
summary / evidence / next_action) to the exact path below. Write NOTHING else to
disk — you are read-only on the repo.

One additional field: if your decision is FAIL and the ONLY defect is a
resolvable citation error — the construct itself genuinely sits inside a
sanctioned family YOU verified, but the vet filed it under the wrong family,
cited the wrong precedent line, or cited a dead path — add "citation_only":
true and name the CORRECT citation in next_action. That routes the worker to a
one-comment re-cite instead of a full re-grind, with no construct ban. Use it
ONLY when you verified the correct family's own prerequisites hold for the
construct as written; a construct you actually object to is a normal FAIL.
"@
    Set-Content $briefPath -Value $task -Encoding utf8
    $v = $null
    for ($try = 1; $try -le 2; $try++) {
        $v = Invoke-GrindAgent $briefPath $outPath $role $Layer1Model $MockLayer1Script -UsageFunc $func -UsageRole 'layer1'
        if ($v -and $v.decision) { break }
        Log "${func}: layer-1 attempt $try returned no parseable decision."
        if ($try -lt 2) { Start-Sleep -Seconds 30 }
    }
    if (-not $v -or -not $v.decision) {
        Log "${func}: layer-1 UNAVAILABLE — failing open to the Judge."
        Record-Review $func 'layer1' 'UNAVAILABLE' 'unreachable'
        return $null
    }
    return $v
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
        if ($v -and $v.verdict -in @('PASS', 'FAIL', 'ESCALATE')) { return $v }
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

function Invoke-JudgeEscalation([string]$func, [string]$kind, $v) {
    # ESCALATE (2026-08-07 review-audit fix #4; rewired by owner ruling 2026-08-18,
    # .claude/rules/judge-sole-gate.md, commit b9d91163). The Judge still says
    # "the grant is above my standing authority" — but NOTHING waits on the owner
    # anymore. The driver routes on the verdict's escalate_kind:
    #   canonical-asm-grant — the driver independently re-runs scan_hand_coded;
    #     on a STRONG-class tier it EXECUTES the grant (inline_asm_canonical.txt
    #     entry via grindlib, LF-safe) and logs it to docs/grind/borderline.md.
    #     The function stays ACTIVE so the next session integrates to
    #     COMPLETED-INLINE-ASM-CANONICAL. Non-STRONG tier = evidence failure:
    #     falls through to the log-and-refuse path.
    #   any other kind — RETIRED (owner ruling 2026-08-31, ordinary-c-judge-
    #     decidable): the Judge no longer emits family-extension /
    #     policy-question. Defensively, a legacy/unknown kind is logged to
    #     docs/grind/borderline.md and the item is FORECLOSED silently
    #     (recorded disposition, skipped by `queue next`, nothing surfaced
    #     to the owner).
    $date = Get-Date -Format 'yyyy-MM-dd'
    $ekind = [string]$v.escalate_kind
    if (-not $ekind) { $ekind = 'policy-question' }
    $ref = "$date — $func — JUDGE ESCALATE on $kind ($ekind) — RESOLVED BY PIPELINE (owner ruling 2026-08-18, no owner wait)"
    $body = @"
**Filed by the grinder Judge ($date)** — verdict ESCALATE ($ekind): the work is
sound but the grant is above the Judge's standing authority. Per the owner's
2026-08-18 ruling (judge-sole-gate, b9d91163) the driver disposes it immediately;
nothing waits on the owner.

**The Judge's packet:**

$($v.justification)

$(if ($v.constraint) { "**Constraint recorded for any future session:** $($v.constraint)" })
"@
    @("", "## $ref", "", $body) | Add-Content $Decisions
    if ($ekind -eq 'canonical-asm-grant') {
        $tier = 'LOW'
        try { $sc = (python tools/scan_hand_coded.py --single $func 2>$null | Out-String)
              if ($sc -match 'tier=(\w+)') { $tier = $Matches[1] } } catch { }
        $granted = python tools/grinder/grindlib.py grant-canonical-asm . $func $tier $date
        if ($LASTEXITCODE -eq 0) {
            python tools/grinder/grindlib.py constrain . $func ("canonical-asm GRANTED (pipeline, $date, tier $tier): integrate the whole-body form per canonical-asm-authorization-recipe to COMPLETED-INLINE-ASM-CANONICAL; the allowlist entry is already written.") | Out-Null
            Journal "$func JUDGE ESCALATE (canonical-asm-grant) — grant EXECUTED (tier $tier), function stays active."
            Log "${func}: judge ESCALATE — canonical-asm grant executed (tier $tier); staying active for integration."
            git -C $Root add -- memory/grind docs/grind metrics/events.jsonl inline_asm_canonical.txt 2>$null
            git -C $Root commit -m "grind: $func canonical-asm grant executed (judge ESCALATE, ruling b9d91163) [skip-park-src-guard]" 2>$null | Out-Null
            return
        }
        # Tier not STRONG — the grant claim fails on evidence; log-and-refuse below.
        $ekind = 'canonical-asm-grant (REFUSED: tier ' + $tier + ', not STRONG-class and not owner-cluster-enumerated)'
    }
    if ($ekind -eq 'integration-handoff') {
        # integration-handoff-self-serve (owner ruling 2026-08-19): a bytes-proven
        # handoff's remedy — a scope_allow.txt widening and/or a Judge-superseded
        # ban clearance — is executed by the DRIVER on the Judge's verdict. The
        # function STAYS ACTIVE; the fix still passes every normal gate (scope
        # check, layer-1, Judge, full-build SHA1). Path classes are enforced by
        # grindlib add-scope-allow (denylist = the severe-blocker list).
        $did = @()
        if ($v.scope_paths) {
            $paths = @($v.scope_paths | ForEach-Object { [string]$_ })
            $line = (python tools/grinder/grindlib.py add-scope-allow . $func $date @paths | Out-String).Trim()
            if ($LASTEXITCODE -eq 0) { $did += "scope grant: $line" }
            else { Log "${func}: integration-handoff scope grant REFUSED ($line)." }
        }
        if ($v.unban_construct) {
            $n = (python tools/grinder/grindlib.py unban . $func ([string]$v.unban_construct) | Out-String).Trim()
            if ([int]$n -gt 0) { $did += "cleared $n superseded ban(s)" }
        }
        if ($did.Count) {
            python tools/grinder/grindlib.py constrain . $func ("integration-handoff EXECUTED (pipeline, $date): $($did -join '; '). Land the banked form through the normal gates; the widened paths are scope-checked AND staged.") | Out-Null
            python tools/grinder/grindlib.py log-borderline . $func 'integration-handoff' "judge ESCALATE packet in docs/grind/decisions.md ($ref)" ("driver-executed per integration-handoff-self-serve (owner ruling 2026-08-19): $($did -join '; '); function stays ACTIVE.") $date | Out-Null
            Journal "$func JUDGE ESCALATE (integration-handoff) — EXECUTED by driver ($($did -join '; ')), function stays active."
            Log "${func}: judge ESCALATE — integration-handoff executed ($($did -join '; ')); staying active."
            git -C $Root add -- memory/grind docs/grind metrics/events.jsonl tools/grinder/scope_allow.txt 2>$null
            git -C $Root commit -m "grind: $func integration-handoff executed (judge ESCALATE, ruling 2026-08-19) [skip-park-src-guard]" 2>$null | Out-Null
            return
        }
        $ekind = 'integration-handoff (REFUSED: no executable remedy in verdict)'
    }
    $evid = "judge ESCALATE packet in docs/grind/decisions.md ($ref)"
    $disp = "REFUSED under the current frozen policy; FORECLOSED silently (owner ruling 2026-08-31, ordinary-c-judge-decidable); candidate preserved at memory/grind/$func/candidate.c; re-attemptable if a later owner ruling spends this entry."
    python tools/grinder/grindlib.py log-borderline . $func $ekind $evid $disp $date | Out-Null
    Invoke-Eng @('queue', 'foreclose', $func, '--reason', "FORECLOSED (judge ESCALATE $ekind refused; owner ruling 2026-08-31): $ref") | Out-Null
    Journal "$func JUDGE ESCALATE ($kind, $ekind) — refused + FORECLOSED silently (owner ruling 2026-08-31)."
    Log "${func}: judge ESCALATE — borderline-logged + foreclosed (silent disposition)."
    git -C $Root add -- memory/grind docs/grind metrics/events.jsonl engine/queue.json 2>$null
    git -C $Root commit -m "grind: $func foreclosed (judge ESCALATE refused) [skip-park-src-guard]" 2>$null | Out-Null
}

function Set-FailRouting([string]$func, $v) {
    # BINDING NO-RESPELLING (2026-08-07 review-audit fix #3). A construct-class
    # FAIL does three things instead of one: the construct is BANNED for this
    # function (the driver rejects a later candidate that re-declares it), the
    # modality ladder is FORCE-ADVANCED so the next session attacks differently
    # rather than respelling, and an annotation-format-only FAIL is routed to a
    # one-comment fix-up brief instead of a whole re-grind.
    $ground = [string]$v.fail_ground
    if ($ground -eq 'ANNOTATION-FORMAT') {
        $detail = if ($v.constraint) { [string]$v.constraint } else { [string]$v.justification }
        $detail = $detail.Substring(0, [Math]::Min(600, $detail.Length))
        python tools/grinder/grindlib.py fixup . $func 'annotation' $detail | Out-Null
        Log "${func}: FAIL ground = ANNOTATION-FORMAT — next session routed to the fix-up brief (comment only)."
        return 'annotation'
    }
    if ($ground -eq 'CITATION') {
        # 2026-08-19 audit: 1/3 of layer-1 FAILs were right-construct/wrong-
        # citation. A construct the reviewer VERIFIED sits in a sanctioned
        # family, mis-filed under a neighbor, costs a one-brief re-cite, not a
        # re-grind. No construct ban (the construct is fine), no modality
        # advance (the attack was right).
        $detail = if ($v.constraint) { [string]$v.constraint } else { [string]$v.justification }
        $detail = $detail.Substring(0, [Math]::Min(600, $detail.Length))
        python tools/grinder/grindlib.py fixup . $func 'citation' $detail | Out-Null
        Log "${func}: FAIL ground = CITATION — next session routed to the fix-up brief (re-cite only)."
        return 'citation'
    }
    $banned = ''
    if ($v.banned_construct) { $banned = [string]$v.banned_construct }
    elseif ($ground -eq 'CONSTRUCT' -and $v.constraint) { $banned = [string]$v.constraint }
    if ($banned) {
        $banned = $banned.Substring(0, [Math]::Min(400, $banned.Length))
        python tools/grinder/grindlib.py ban . $func $banned | Out-Null
        Log "${func}: construct BANNED for this function — $banned"
    }
    $newMod = (python tools/grinder/grindlib.py advance-modality . $func 2>&1 | Out-String).Trim()
    Log "${func}: modality force-advanced after FAIL — next session is '$newMod'."
    if ($ground) { return $ground.ToLower() } else { return 'construct' }
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
    Record-Review $func 'judge' ([string]$v.verdict) 'ruling'
    if ($v.verdict -eq 'ESCALATE') {
        Add-Decision $func "ruling: $qShort" 'ESCALATE' $v.justification
        if ($v.constraint) { python tools/grinder/grindlib.py constrain . $func ([string]$v.constraint) | Out-Null }
        Invoke-JudgeEscalation $func 'ruling request' $v
        return
    }
    Add-Decision $func "ruling: $qShort" $v.verdict $v.justification
    if ($v.constraint) { python tools/grinder/grindlib.py constrain . $func ([string]$v.constraint) | Out-Null }
    if ($v.unban_construct) {
        # integration-handoff-self-serve (owner ruling 2026-08-19): a ruling that
        # explicitly narrows/supersedes an earlier ban clears the mechanical
        # tripwire, so the authorized resubmission is not auto-discarded
        # (func_8002D518 burned two sessions exactly this way).
        $n = (python tools/grinder/grindlib.py unban . $func ([string]$v.unban_construct) | Out-String).Trim()
        if ([int]$n -gt 0) { Log "${func}: $n superseded ban(s) cleared per judge narrowing." }
    }
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
        Revert-SessionEdits $func
        return
    }
    # Single-stem gate: the ONLY build-input change allowed in a candidate is the
    # target file itself. A load-bearing edit to another src/include file would be
    # byte-verified now but DROPPED by the fixed Match commit below — producing a
    # committed Match that fails a fresh rebuild. Reject before retire. Keys on
    # the PATH under any
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
        Revert-SessionEdits $func
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
                      "which the driver cannot accept (the legitimate route is a Judge " +
                      "ESCALATE integration-handoff scope grant, which no session took). " +
                      "FORECLOSED (owner ruling 2026-08-31); re-activated by an owner unpark " +
                      "or a scope_allow.txt grant."
            Invoke-Eng @('queue', 'foreclose', $func, '--reason', $reason) | Out-Null
            Add-Decision $func 'scope livelock' 'FORECLOSED' $reason
            Journal "$func SCOPE-LIVELOCK — foreclosed after $n identical out-of-scope candidates ($($offStem -join ', '))."
            Log "${func}: SCOPE LIVELOCK — foreclosed so the queue advances (silent disposition)."
            git -C $Root add -- memory/grind docs/grind metrics/events.jsonl engine/queue.json 2>$null
            git -C $Root commit -m "grind: $func foreclosed on scope livelock [skip-park-src-guard]" 2>$null | Out-Null
            $script:livelockParks++
            if ($script:livelockParks -ge 3) {
                Circuit-Break "scope livelock on $script:livelockParks distinct functions — the scope gate looks systemically wrong, not function-specific"
            }
        }
        return
    }
    # 1b) LAYER-1 GATE — a fresh adversarial cheat-reviewer rules on the diff +
    # self-vet BEFORE any Judge cycle (or the expensive retire+rebuild) is spent.
    # A FAIL here short-circuits straight back to the worker with the construct
    # banned and the modality advanced. Sandbox has already proven the honest
    # distance is 0, so nothing about the bytes is lost by rejecting here.
    $l1 = Invoke-Layer1 $func $stem ((git -C $Root diff -- "src/$stem.c" | Out-String))
    if ($l1 -and $l1.decision -eq 'FAIL' -and $l1.citation_only) {
        # Right construct, wrong paperwork (2026-08-19 audit). One-brief re-cite:
        # no construct ban, no modality advance, no Judge cycle. The candidate
        # source is reverted like any FAIL — the fix-up session re-emits it with
        # the corrected citation and the full gate chain re-runs.
        $l1Summary = if ($l1.summary) { [string]$l1.summary } else { 'citation defect' }
        Record-Review $func 'layer1' 'FAIL' 'citation'
        Copy-Item (Join-Path $Root "src\$stem.c") (Join-Path $Root "memory\grind\$func\rejected\layer1-citation-$(Get-Date -Format 'MMdd-HHmm').c") -ErrorAction SilentlyContinue
        Revert-SessionEdits $func
        $detail = "$l1Summary $(if ($l1.next_action) { [string]$l1.next_action })"
        python tools/grinder/grindlib.py fixup . $func 'citation' $detail.Substring(0, [Math]::Min(600, $detail.Length)) | Out-Null
        Add-Decision $func 'layer-1 review' 'FAIL (citation-only)' $l1Summary
        Journal "${func}: LAYER-1 citation-only FAIL — routed to re-cite fix-up (no ban, no Judge cycle): $l1Summary"
        Log "${func}: LAYER-1 citation-only FAIL — fix-up brief queued."
        git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
        git -C $Root commit -m "grind: $func layer-1 citation fix-up queued [skip-park-src-guard]" 2>$null | Out-Null
        return
    }
    if ($l1 -and $l1.decision -eq 'FAIL') {
        $l1Summary = if ($l1.summary) { [string]$l1.summary } else { 'layer-1 cheat-reviewer FAIL' }
        $l1Constructs = @($l1.evidence | ForEach-Object { [string]$_.construct } | Where-Object { $_ })
        Record-Review $func 'layer1' 'FAIL' 'construct'
        Log "${func}: LAYER-1 FAIL — $l1Summary (no Judge cycle spent)."
        Copy-Item (Join-Path $Root "src\$stem.c") (Join-Path $Root "memory\grind\$func\rejected\layer1-fail-$(Get-Date -Format 'MMdd-HHmm').c") -ErrorAction SilentlyContinue
        Revert-SessionEdits $func
        $c = "LAYER-1 CHEAT-REVIEWER FAIL: $l1Summary" +
             $(if ($l1.next_action) { " Next action: $($l1.next_action)" })
        python tools/grinder/grindlib.py constrain . $func $c.Substring(0, [Math]::Min(600, $c.Length)) | Out-Null
        foreach ($bc in $l1Constructs) {
            python tools/grinder/grindlib.py ban . $func $bc.Substring(0, [Math]::Min(400, $bc.Length)) | Out-Null
        }
        $newMod = (python tools/grinder/grindlib.py advance-modality . $func 2>&1 | Out-String).Trim()
        Log "${func}: modality force-advanced after layer-1 FAIL — next session is '$newMod'."
        Add-Decision $func 'layer-1 review' 'FAIL' $l1Summary
        Journal "${func}: LAYER-1 FAILED a sandbox-0 candidate — $l1Summary"
        git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
        git -C $Root commit -m "grind: $func layer-1 FAIL banked [skip-park-src-guard]" 2>$null | Out-Null
        return
    }
    if ($l1 -and $l1.decision) {
        # NEEDS_USER is NOT a stop here: it is precisely the authority question the
        # Judge's new ESCALATE verdict exists to route, and the Judge outranks
        # layer-1 on policy. Record it and let the Judge rule.
        Record-Review $func 'layer1' ([string]$l1.decision) $(if ($l1.decision -eq 'NEEDS_USER') { 'authority' } else { '' })
        Log "${func}: layer-1 $($l1.decision) — proceeding to bytes + Judge."
    }
    $null = Invoke-Eng @('retire', $func)          # drops rules if any; SHA1-gated internally
    $vo = Invoke-Eng @('verify-oracle', '--rebuild', '--allow-dirty')
    if ($LASTEXITCODE -ne 0) {
        Log "${func}: FULL-BUILD SHA1 FAILED after retire — reverting, banking constraint."
        # stage events BEFORE the broad checkout: `checkout -- .` restores the
        # worktree FROM the index, so staged telemetry survives the revert
        git -C $Root add -- metrics/events.jsonl 2>$null
        git -C $Root checkout -- . 2>$null
        Invoke-Eng @('verify-oracle', '--rebuild') | Out-Null   # restore green build/ (2026-08-11: stale red build/ here false-tripped the post-session circuit-break)
        python tools/grinder/grindlib.py constrain . $func "candidate form failed full-build SHA1 on main (masked-0 register diff class) — reg-alloc gap is real" | Out-Null
        git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
        git -C $Root commit -m "grind: $func byte-fail constraint banked [skip-park-src-guard]" 2>$null | Out-Null
        return
    }
    # 2) bytes proven — now the Judge rules on the C
    $diff = (git -C $Root diff -- "src/$stem.c" | Out-String)
    $led = "memory/grind/$func"
    $scopeBlock = ''
    try { $scopeBlock = (python tools/grinder/grindlib.py rule-scopes . $func 2>$null | Out-String).Trim() } catch { }
    $task = @"
FINAL CALL for $func — bytes are already proven on main (sandbox 0 + retire +
full-build SHA1 == oracle). Rule ONLY on the legitimacy of the C.

The candidate diff against HEAD:
``````diff
$diff
``````

$scopeBlock

Ledger: $led/state.json (judge_constraints — includes the regression diagnosis
if this is a regression-origin item), $led/hypotheses.md, $led/evidence.md,
$led/rejected/. Write your verdict JSON to the exact path given below.
"@
    $v = Invoke-Judge $func $task
    $sessionsTaken = ((Get-Content (Join-Path $Root "memory\grind\$func\state.json") -Raw | ConvertFrom-Json).session_count + 1)
    Record-Review $func 'judge' ([string]$v.verdict) ([string]$v.fail_ground).ToLower()
    if ($v.verdict -eq 'ESCALATE') {
        # Sound work, authority limit. Preserve the candidate as the record's
        # evidence, put main back to HEAD, then route per owner ruling 2026-08-18
        # (Invoke-JudgeEscalation: grant executed, or borderline-logged + terminal
        # park — never a wait on the owner).
        Copy-Item (Join-Path $Root "src\$stem.c") (Join-Path $Root "memory\grind\$func\candidate.c") -ErrorAction SilentlyContinue
        git -C $Root add -- metrics/events.jsonl 2>$null
        git -C $Root checkout -- . 2>$null
        Invoke-Eng @('verify-oracle', '--rebuild') | Out-Null   # restore green build/
        Add-Decision $func 'final call' 'ESCALATE' $v.justification
        Invoke-JudgeEscalation $func 'final call' $v
        return
    }
    if ($v.verdict -eq 'PASS') {
        # Owner Ruling C 2026-09-02 (decisions.md "2026-09-02 — OWNER RULING"): a
        # Judge PASS on an island-carrying body must go through the canonical-asm
        # grant door BEFORE `queue done`. The engine gate scores cop2 islands as
        # non-cheat, so without this check the function lands titled COMPLETED-C
        # with no allowlist line (func_8002E838 49d6927e, func_80031890 1a2e49e4).
        $bucket = 'COMPLETED-C'
        $isl = (python tools/grinder/grindlib.py island-count . $func $stem 2>$null | Out-String).Trim()
        $nIsl = 0; $listed = $false
        if ($isl -match '^(\d+)\s+(yes|no)') { $nIsl = [int]$Matches[1]; $listed = ($Matches[2] -eq 'yes') }
        if ($nIsl -gt 0 -and -not $listed) {
            $gdate = Get-Date -Format 'yyyy-MM-dd'
            $tier = 'LOW'
            try { $sc = (python tools/scan_hand_coded.py --single $func 2>$null | Out-String)
                  if ($sc -match 'tier=(\w+)') { $tier = $Matches[1] } } catch { }
            $granted = (python tools/grinder/grindlib.py grant-canonical-asm . $func $tier $gdate 2>&1 | Out-String).Trim()
            if ($LASTEXITCODE -ne 0) {
                # No evidence door admits the islands: refuse the merge, keep the
                # candidate as evidence, bank the constraint. Never lands as C.
                $reason = ("MERGE REFUSED (owner Ruling C 2026-09-02): the Judge PASSed a body carrying $nIsl " +
                           "non-cop2-whitelist inline-asm island(s) but func is not allowlisted and no grant door " +
                           "admits it (scan tier $tier, not in tools/grinder/owner_cluster_grants.txt). Either the " +
                           "islands are C-expressible (respell them in C) or the function needs an owner cluster " +
                           "row. gate: $granted")
                $reason = $reason.Substring(0, [Math]::Min(500, $reason.Length))
                Log "${func}: MERGE REFUSED after judge PASS — island-carrying body with no grant door; banking constraint."
                Copy-Item (Join-Path $Root "src\$stem.c") (Join-Path $Root "memory\grind\$func\candidate.c") -ErrorAction SilentlyContinue
                git -C $Root add -- metrics/events.jsonl 2>$null
                git -C $Root checkout -- . 2>$null
                Invoke-Eng @('verify-oracle', '--rebuild') | Out-Null   # restore canonical build/
                python tools/grinder/grindlib.py constrain . $func $reason | Out-Null
                git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
                git -C $Root commit -m "grind: $func merge refused — unallowlisted islands, constraint banked [skip-park-src-guard]" 2>$null | Out-Null
                Journal "${func}: MERGE REFUSED after judge PASS — $nIsl unallowlisted inline-asm island(s), no grant door; constraint banked."
                return
            }
            Log "${func}: judge PASS on an island-carrying body — canonical-asm grant executed via the PASS path (tier $tier)."
            $listed = $true
        }
        if ($nIsl -gt 0 -and $listed) { $bucket = 'COMPLETED-INLINE-ASM-CANONICAL' }
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
                       "honest COMPLETED-C form must eliminate, or the function needs the pipeline " +
                       "canonical-asm grant (judge ESCALATE canonical-asm-grant per ruling 2026-08-18). gate: " + (($qd -replace '["\r\n\t]+', ' ') -replace '\s+', ' ').Trim())
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
        # inline_asm_canonical.txt rides along when the PASS-path grant wrote it
        # (owner Ruling C 2026-09-02) so the allowlist line lands in the same
        # byte-verified commit as the body it authorizes.
        git -C $Root add -- "src/$stem.c" $extraScope engine/queue.json tools/prologue_config.json tools/frame_fix_funcs.txt tools/delay_slot_ra_funcs.txt "memory/grind/$func" inline_asm_canonical.txt 2>$null
        git -C $Root commit -m "Match: $func — $bucket (grinder, $sessionsTaken sessions)" | Out-Null
        Add-Decision $func 'final call' 'PASS' $v.justification
        # R5 (modality-effectiveness 2026-08-19): record the CLOSING modality —
        # without it the closer is only inferable from the ladder, and the one
        # discriminator available showed ~1/3 of inferred credit was wrong.
        Journal "$func $bucket after $sessionsTaken sessions (closer: s$sessionN [$modality])."
        Remove-Item -Recurse -Force (Join-Path $Root "memory\grind\$func")
        git -C $Root add -A -- memory/grind docs/grind 2>$null
        git -C $Root add -- metrics/events.jsonl 2>$null
        git -C $Root commit -m "grinder: close ledger for $func" | Out-Null
        Log "${func}: MERGED — $bucket."
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
        # Make the constraint BIND: ban the construct + force a modality change,
        # or route an annotation-only FAIL to the one-comment fix-up brief.
        $null = Set-FailRouting $func $v
        git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
        git -C $Root commit -m "grind: $func judge FAIL banked [skip-park-src-guard]" 2>$null | Out-Null
        Log "${func}: judge FAILED the candidate — constraint banked, grind continues."
        $jShort = $v.justification.Substring(0, [Math]::Min(120, $v.justification.Length))
        Journal "${func}: judge FAILED a bytes-proven candidate — $jShort"
    }
}

# ── session spawn (pattern from tools/fleet/_fleet_common.ps1:132-170 — fleet RETIRED 2026-07-06, historical attribution only) ────────
function Test-AgentApiError([string]$AgentLog) {
    # Process improvement #1 (2026-08-18): the last line of the agent log is the
    # harness result JSON; terminal_reason 'api_error' with a 5xx/429 status is
    # environmental (server overload / rate limit), not a bad session. The
    # 2026-08-18 circuit-break was three 192-second 529 deaths that outran the
    # 120s spawn-failure window and were miscounted as invalid sessions.
    if (-not (Test-Path $AgentLog)) { return $false }
    $last = Get-Content $AgentLog -Tail 1 -ErrorAction SilentlyContinue
    try { $j = $last | ConvertFrom-Json } catch { return $false }
    if ([string]$j.terminal_reason -ne 'api_error') { return $false }
    $code = 0; try { $code = [int]$j.api_error_status } catch { }
    return ($code -ge 500 -or $code -eq 429)
}

function Get-AgentSpawnException([string]$AgentLog) {
    # Returns the launch-exception text if the last spawn never started the CLI
    # (terminal_reason=spawn_exception, written by Invoke-GrindAgent), else ''.
    # A launch exception is a DRIVER/HOST defect (argv limit, missing binary,
    # broken PATH), not weather — retrying it forever can never succeed.
    if (-not (Test-Path $AgentLog)) { return '' }
    try {
        $j = (Get-Content $AgentLog -Tail 1 -ErrorAction Stop) | ConvertFrom-Json
        if ([string]$j.terminal_reason -eq 'spawn_exception') { return [string]$j.result }
    } catch { }
    return ''
}

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
        # Task text goes to the CLI on STDIN, never as an argv element (2026-09-02
        # incident: func_800300B4's brief grew past 32,767 chars after ten sessions,
        # and Windows refused to launch `claude -p <brief>` — "The filename or
        # extension is too long". The exception died inside the Job, the agent.log
        # was never rewritten, and the driver read the 0-second death as a
        # usage-limit blip, retrying at the 30-min cap indefinitely.) STDIN has no
        # length limit; briefs only grow. Inside a Job so we get a wall-clock timeout.
        $task = (Get-Content $BriefPath -Raw -Encoding utf8) +
            "`n`nWhen finished, write your outcome JSON to this exact absolute path (overwrite it):`n  $OutcomePath`n"
        $sid = [guid]::NewGuid().ToString()
        $t0 = Get-Date
        # A stale agent.log from the previous spawn must never masquerade as this
        # spawn's diagnostics (it hid the launch failure above for 8 attempts).
        Remove-Item ($OutcomePath + '.agent.log') -ErrorAction SilentlyContinue
        $job = Start-Job -ScriptBlock {
            param($Task, $RoleFile, $Model, $Sid, $Cwd, $AgentLog, $Func)
            Set-Location $Cwd
            $env:CLAUDE_SESSION_ID = $Sid
            # 1-HOUR prompt-cache TTL (2026-08-11 token audit): grind sessions
            # pause >5min inside builds/permuter waits, and the 5m TTL expiry
            # forced 4-5 full-context recaches per session (~500-620k
            # cache-write tokens, ~35-40% of session cost). The 2x write
            # premium pays for itself by the third avoided rewrite. No-op on
            # auth modes that already get 1h.
            $env:ENABLE_PROMPT_CACHING_1H = '1'
            # Arms the grind_check.sh Stop-gate for THIS session only: the hook
            # no-ops unless GRIND_FUNC is set, so interactive/operator sessions
            # and this session's own subagents (Stop-only wiring) are unaffected.
            $env:GRIND_FUNC = $Func
            # --strict-mcp-config with no --mcp-config = ZERO MCP servers for grind
            # spawns. The operator's user-scope servers (github/playwright/unity) are
            # useless to a decomp session but their tool surface rides in the baseline
            # of every turn, and a turn's context is re-read on all later turns. Also
            # removes the chance a session wanders into a browser/editor tool.
            # `-p` with no inline prompt reads the task from stdin (see the
            # argv-length incident note above).
            $claudeArgs = @('-p', '--append-system-prompt-file', $RoleFile,
                            '--permission-mode', 'bypassPermissions', '--model', $Model,
                            '--strict-mcp-config',
                            '--session-id', $Sid, '--output-format', 'json')
            # Keep the CLI's result line — it is the only diagnostic when a spawn
            # dies instantly (usage limit, auth, API error). Overwritten per spawn.
            # A LAUNCH exception (process never started) is written in the same
            # place as a JSON line with terminal_reason=spawn_exception so the
            # driver can tell "never launched" from "launched and hit the API".
            try {
                $prev = [Console]::OutputEncoding
                [Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)
                $OutputEncoding = [System.Text.UTF8Encoding]::new($false)
                ($Task | & claude @claudeArgs 2>&1 | Out-String) | Set-Content $AgentLog -Encoding utf8
                [Console]::OutputEncoding = $prev
            } catch {
                @{ terminal_reason = 'spawn_exception'; is_error = $true
                   result = ("agent launch exception: " + $_.Exception.Message) } |
                    ConvertTo-Json -Compress | Set-Content $AgentLog -Encoding utf8
            }
        } -ArgumentList $task, $RoleFile, $AgentModel, $sid, $Root, ($OutcomePath + '.agent.log'), $Func
        if (-not (Wait-Job $job -Timeout ($SessionTimeoutMin * 60))) {
            Log "session TIMEOUT after $SessionTimeoutMin min; stopping job."
            Stop-Job $job -ErrorAction SilentlyContinue
        }
        # Belt and braces: if the Job itself died (script-block error outside the
        # try above) and left no agent.log, bank its error text so the failure is
        # never silent.
        $agentLogPath = $OutcomePath + '.agent.log'
        if (-not (Test-Path $agentLogPath)) {
            $jobErr = ''
            try { $jobErr = (Receive-Job $job -ErrorAction SilentlyContinue 2>&1 | Out-String).Trim() } catch { }
            if (-not $jobErr) { $jobErr = "job state $($job.State), no output" }
            @{ terminal_reason = 'spawn_exception'; is_error = $true
               result = ("agent job failure: " + $jobErr) } |
                ConvertTo-Json -Compress | Set-Content $agentLogPath -Encoding utf8
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

function Revert-SessionEdits([string]$func = '') {
    # Deliberately does NOT touch metrics/events.jsonl: events are append-only
    # facts about engine/permuter commands that really ran — valid telemetry
    # even from discarded sessions (owner directive 2026-07-07; reverting it
    # here wiped every session's permuter-harvest events). The ledger commits
    # sweep it up each boundary.
    git -C $Root checkout -- src include 2>$null
    # Granted scope_allow.txt paths must revert too. Once the session scope check
    # honours a grant (2026-08-22), a rejected candidate can leave a granted
    # root-level file (e.g. undefined_syms_auto.txt) dirty; src/include-only
    # reverting would carry that dirt into the NEXT session, whose scope check
    # flags it and discards an innocent session — the park-queue dirt-deadlock
    # shape (memory/project/grinder-park-queue-dirt-deadlock.md). Tracked paths
    # only: checkout cannot restore an untracked file, and the scope-violation
    # branch already `git clean`s those.
    if ($func) {
        foreach ($p in @(Get-ExtraScope $func)) {
            git -C $Root checkout -- $p 2>$null
        }
    }
}

$script:consecutiveInvalid = 0
$script:spawnFails = 0
$script:spawnExceptions = 0   # consecutive "CLI never launched" spawns (see Get-AgentSpawnException)
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

    # Snapshot the owner audit file so a discarded session's decisions.md append
    # can be stamped (2026-08-19 phantom-ruling audit: an invalidated session
    # left a terminal-sounding ruling in decisions.md — Revert-SessionEdits
    # covers src/include only and docs/ is allowed-dirty by design, so the
    # append survived with nothing marking it as thrown away).
    $script:decisionsLines = 0
    try { $script:decisionsLines = @(Get-Content $Decisions -ErrorAction SilentlyContinue).Count } catch { }

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

    # 2b) unpark sync (owner ruling 2026-09-02, ruling 1): a queue item that
    # returned to active with a fresh unpark_reason gets its exhaustion window
    # reset BEFORE the modality is assigned — otherwise the old flat history
    # forces `escalation` on the very first post-unpark session and the owner's
    # ruling buys exactly one session (11/12 of the 2026-09-01 unparks).
    try {
        $sync = (python tools/grinder/grindlib.py sync-unpark . $func).Trim()
        if ($sync -eq 'stamped') {
            Log "${func}: unpark detected — exhaustion window reset (fresh flat window from here)."
            git -C $Root add -- "memory/grind/$func/state.json" 2>$null
            git -C $Root commit -m "grind: $func exhaustion window reset on unpark [skip-park-src-guard]" 2>$null | Out-Null
        }
    } catch { }

    # 3) brief with mandated modality
    $modality = (python tools/grinder/grindlib.py modality . $func).Trim()
    $outPath  = Join-Path $GrindTmp "outcome_$func.json"
    $briefPath = Join-Path $GrindTmp "brief_$func.md"
    # Process improvement #2 (2026-08-18): measure the HEAD honest floor at
    # dispatch and inject it as the brief's CHASSIS CHECK — stale ledger floors
    # and contaminated templates cost whole sessions (s5 func_80017848; the
    # 27-vs-22 / 57-vs-26 stale-queue exhibits). Best-effort: a failed
    # measurement degrades to 'unavailable', never blocks dispatch.
    $headFloor = ''
    try {
        $sb = (& tools/wteng.ps1 main sandbox $func --disable all 2>&1 | Out-String)
        if ($sb -match '"distance"\s*:\s*(\d+)') { $headFloor = $Matches[1] }
        elseif ($sb -match 'distance[^0-9]*([0-9]+)') { $headFloor = $Matches[1] }
    } catch { }
    python tools/grinder/grindlib.py brief . $func $modality $outPath $headFloor | Set-Content $briefPath -Encoding utf8
    # Respawn feedback (2026-08-07 circuit-break class fix): a discarded session's
    # validator reason is appended to the next brief. Without it, fresh sessions
    # re-read the same ledger and deterministically repeat the same rejected
    # output — two identical citation-format circuit-breaks on 2026-08-07
    # (func_80048AD0 16:05, func_80021A98 22:31). Cleared on the first valid session.
    if ($script:lastDiscardReason) {
        Add-Content $briefPath -Encoding utf8 -Value @(
            '',
            '## PREVIOUS SESSION DISCARDED — FIX THIS FIRST',
            'The previous session on this function was DISCARDED by the driver validator for the reason below. Your output will be discarded the same way unless you correct it:',
            '',
            ('> ' + $script:lastDiscardReason),
            '',
            'If this is a self_vet.md citation-format rejection: every PRECEDENT line must contain a literal file:line (e.g. `.claude/rules/no-new-park-categories.md:172`) or a 7-40 char commit hash. Prose descriptions, dates, or file + section-heading references are mechanically rejected regardless of merit.'
        )
    }
    # The annotation fix-up is ONE-SHOT: consume it once the brief has carried the
    # notice, so a discarded or failed fix-up session drops back onto the normal
    # ladder instead of pinning the function on comments forever.
    if ($modality -eq 'annotation-fix') { python tools/grinder/grindlib.py clear-fixup . $func | Out-Null }
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
    # Per-modality model: recon sessions get the strong model (frontier quality
    # determines how many execution sessions follow); everything else grinds on
    # the cheaper worker model.
    # object-model (2026-09-03) is a one-shot audit whose quality decides whether
    # the function forecloses — it gets the recon-tier model for the same reason.
    $sessionModel = if ($modality -in @('recon', 'object-model')) { $ReconModel } else { $Model }
    Log "${func}: session $sessionN starting, modality=$modality, model=$sessionModel"

    # 4) spawn
    $o = Invoke-GrindAgent $briefPath $outPath (Join-Path $RolesDir 'grind-session.md') $sessionModel $MockSessionScript $func

    # 5) scope check — any edit outside the allowed surface invalidates the session
    $dirty = Assert-CleanTree
    # A scope_allow.txt grant is honoured HERE too, not only in Invoke-CandidatePath.
    # Until 2026-08-22 this check consulted $AllowedDirtyPattern alone, so a grant for
    # ANY path outside src/ or include/ was silently inert: Get-ExtraScope was reached
    # only from Invoke-CandidatePath, whose own filter is '^..\s+("?)(src/|include/)',
    # and this check runs FIRST — so a session that made the mandated edit was discarded
    # before its outcome was ever read, WITH the grant already in place. func_80038170
    # burned two grants and a session exactly this way (owner ruling 2026-08-22;
    # memory/grinder-scope-grant-inert-outside-src-include.md documented the shape and
    # it still cost a session). A grant that silently no-ops is worse than no grant.
    $granted = @(Get-ExtraScope $func)
    $violations = @($dirty | Where-Object {
        $_ -notmatch $AllowedDirtyPattern -and
        $granted -notcontains ($_.Substring(3).Trim().Trim('"'))
    })
    if ($violations.Count) {
        Log "${func}: SCOPE VIOLATION — $($violations -join ' | ') — session discarded."
        $script:lastDiscardReason = "SCOPE VIOLATION: you edited files outside the allowed surface ($($violations -join ' | ')). Touch ONLY your function's src file, memory/grind/<func>/, and tmp/."
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
        # $func is passed so the validator can run the SELF-VET gate and the
        # banned-construct check on a candidate-ready (2026-08-07 review-audit
        # fixes #2a/#3b): a candidate with no written 6-test vet, or one whose vet
        # re-declares a construct the Judge already banned for this function, is an
        # INVALID SESSION — discarded and respawned like a scope violation, with no
        # Judge cycle spent.
        $invalidReason = (python tools/grinder/grindlib.py validate . $outPath $modality $func 2>&1 | Out-String).Trim()
        $valid = ($LASTEXITCODE -eq 0)
        if (-not $valid -and [string]$o.result -eq 'candidate-ready') {
            $cause = if ($invalidReason -match 'BANNED') { 'banned' } else { 'selfvet' }
            Record-Review $func 'layer1' 'REJECTED' $cause
        }
        # owner-gated: the validator can't see $func, so the driver verifies the
        # cited escalation entry (OWNER-ESCALATION or CANONICAL-ASM GRANT PATH,
        # ruling 2026-08-18) actually names THIS function.
        if ($valid -and [string]$o.result -eq 'owner-gated') {
            $decTxt = Get-Content (Join-Path $Root 'docs\grind\decisions.md') -Raw -ErrorAction SilentlyContinue
            $named = @(($decTxt -split "`n") | Where-Object { $_ -match 'OWNER-ESCALATION|CANONICAL-ASM GRANT PATH' -and $_ -match [regex]::Escape($func) })
            if (-not $named.Count) {
                $valid = $false
                $invalidReason = "owner-gated claim rejected: no OWNER-ESCALATION / CANONICAL-ASM GRANT PATH entry in docs/grind/decisions.md names $func"
            }
        }
    }
    if (-not $valid) {
        # SPAWN FAILURE, not a bad session: no outcome at all AND the agent died
        # near-instantly (usage-limit window, auth expiry, API outage). These are
        # environmental — back off and retry instead of feeding the circuit
        # breaker (2026-07-17 incident: three 4-second spawn deaths during a
        # usage-limit window circuit-broke an otherwise healthy grind).
        if (-not $o -and ($script:LastAgentSeconds -lt 120 -or (Test-AgentApiError "$outPath.agent.log"))) {
            $script:spawnFails++
            Revert-SessionEdits $func
            # (a) LAUNCH exception = the CLI process never started. That is a
            # driver/host defect, not weather: two in a row circuit-break with
            # the exception text in INCIDENT.md (2026-09-02: eight silent
            # retries on an argv-too-long launch failure).
            $spawnEx = Get-AgentSpawnException "$outPath.agent.log"
            if ($spawnEx) {
                $script:spawnExceptions++
                Log "${func}: agent LAUNCH FAILURE ($([int]$script:LastAgentSeconds)s) — $spawnEx (consecutive $($script:spawnExceptions))."
                if ($script:spawnExceptions -ge 2) {
                    Circuit-Break "agent process failed to LAUNCH twice in a row on $func — not an API/usage-limit condition, retrying cannot help: $spawnEx"
                }
            } else {
                $script:spawnExceptions = 0
            }
            # (b) Environmental failures (usage limit, API outage) back off, but
            # NOT forever: 16 consecutive is ~7 hours at the 30-min cap — longer
            # than any plan-limit window — so beyond that the cause is not weather.
            if ($script:spawnFails -ge 16) {
                Circuit-Break "agent spawn failed $($script:spawnFails) consecutive times on $func (~7h of backoff) — see $outPath.agent.log; not retrying indefinitely"
            }
            $delay = [int][Math]::Min(1800, 60 * [Math]::Pow(2, $script:spawnFails - 1))
            Log "${func}: agent SPAWN/API FAILURE ($([int]$script:LastAgentSeconds)s, no outcome — usage-limit/API-error per agent.log; see $outPath.agent.log) — attempt $($script:spawnFails), retrying in ${delay}s."
            # Sleep in slices so a STOP request is honoured within a minute
            # instead of after a full 30-min backoff.
            $until = (Get-Date).AddSeconds($delay)
            while ((Get-Date) -lt $until) {
                if (Test-Path $StopFile) { break }
                Start-Sleep -Seconds ([int][Math]::Min(30, [Math]::Max(1, ($until - (Get-Date)).TotalSeconds)))
            }
            if ($Once) { break } else { continue }
        }
        # Preserve the discarded outcome for diagnosis — repeated invalids are
        # otherwise unexplainable after the respawn overwrites the file.
        if (Test-Path $outPath) {
            Copy-Item $outPath (Join-Path $GrindTmp "invalid_${func}_s${sessionN}_$(Get-Date -Format 'HHmmss').json") -ErrorAction SilentlyContinue
        }
        Log "${func}: INVALID session output ($invalidReason) — discarded, src reverted, respawning."
        $script:lastDiscardReason = $invalidReason
        Revert-SessionEdits $func
        # Stamp any decisions.md append the discarded session left behind — a
        # thrown-away session must not leave ruling-shaped text in the owner
        # audit surface with nothing marking it void (2026-08-19 audit). Only
        # the session itself can have appended between the loop-top snapshot
        # and here: driver appends (rulings, escalations) happen strictly after
        # validation, which this session failed.
        if ($script:decisionsLines) {
            $nowLines = 0
            try { $nowLines = @(Get-Content $Decisions -ErrorAction SilentlyContinue).Count } catch { }
            if ($nowLines -gt $script:decisionsLines) {
                @("", "## $(Get-Date -Format 'yyyy-MM-dd HH:mm') — $func — DISCARDED-SESSION MARKER (driver-stamped)", "",
                  "Text appended above by session s$sessionN of $func, which the driver DISCARDED as invalid ($invalidReason). It is not a ruling and carries no standing; terminal-sounding language in that span is void.") | Add-Content $Decisions
                git -C $Root add -- docs/grind metrics/events.jsonl 2>$null
                git -C $Root commit -m "grind: $func discarded-session marker stamped in decisions.md [skip-park-src-guard]" 2>$null | Out-Null
                Log "${func}: discarded session's decisions.md append stamped void."
            }
        }
        $script:consecutiveInvalid++
        if ($script:consecutiveInvalid -ge 3) { Circuit-Break "3 consecutive invalid sessions on $func" }
        if ($Once) { break } else { continue }
    }
    $script:consecutiveInvalid = 0
    $script:spawnFails = 0
    $script:spawnExceptions = 0
    $script:lastDiscardReason = $null

    # 7) route by result
    switch ([string]$o.result) {
        'ruling-request' { Invoke-JudgeRuling $func ([string]$o.ruling_question); Revert-SessionEdits $func }
        'candidate-ready' { Invoke-CandidatePath $func $stem $modality $o }
        'owner-gated' {
            # A filed disposition entry (verified above to name $func) means every
            # sanctioned axis is measured dead. Per owner rulings 2026-08-18
            # (judge-sole-gate) and 2026-08-31 (ordinary-c-judge-decidable)
            # NOTHING waits on the owner and no packet is filed: a
            # RESOLVED BY STANDING RULING entry FORECLOSES silently; a
            # CANONICAL-ASM GRANT PATH entry keeps the function ACTIVE for
            # authoring/integration; any legacy pending-shaped ref is
            # borderline-logged and foreclosed.
            python tools/grinder/grindlib.py apply . $func $outPath $modality | Out-Null
            Revert-SessionEdits $func
            $escRef = [string]$o.escalation_ref
            if ($escRef -match 'RESOLVED BY STANDING RULING') {
                $reason = "FORECLOSED (standing ruling 2026-07-27; silent disposition per owner ruling 2026-08-31): $escRef"
                Invoke-Eng @('queue', 'foreclose', $func, '--reason', $reason) | Out-Null
                Log "${func}: ENDGAME LOCK — FORECLOSED silently ($escRef; owner ruling 2026-08-31)."
                Journal "$func s$sessionN [$modality] STANDING RULING (2026-07-27) applied — FORECLOSED: $($o.headline)"
            } elseif ($escRef -match 'LADDER EXHAUSTED') {
                # Owner ruling 2026-09-02 (ruling 2): a non-endgame residual (floor >
                # ENDGAME_LOCK_MAX_FLOOR) foreclosed after two full ladder cycles —
                # the record never claims the 2026-07-27 standing ruling.
                $reason = "FORECLOSED (ladder exhausted, non-endgame residual — owner ruling 2026-09-02; silent per owner ruling 2026-08-31): $escRef"
                Invoke-Eng @('queue', 'foreclose', $func, '--reason', $reason) | Out-Null
                Log "${func}: LADDER EXHAUSTED (non-endgame) — FORECLOSED silently ($escRef)."
                Journal "$func s$sessionN [$modality] LADDER EXHAUSTED (non-endgame residual) — FORECLOSED: $($o.headline)"
            } elseif ($escRef -match 'CANONICAL-ASM GRANT PATH') {
                # Owner ruling 2026-08-18 (judge-sole-gate, b9d91163): STRONG-tier
                # canonical-asm no longer waits on the owner. Stay ACTIVE — the next
                # session authors the whole-body form; the Judge makes the final
                # call and the driver writes the grant on its verdict.
                python tools/grinder/grindlib.py constrain . $func ("canonical-asm GRANT PATH ($escRef): author the whole-body form per canonical-asm-authorization-recipe and integrate; no owner wait.") | Out-Null
                Log "${func}: CANONICAL-ASM GRANT PATH — stays active for authoring/integration (no owner wait)."
                Journal "$func s$sessionN [$modality] canonical-asm grant path — stays active: $($o.headline)"
                git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
                git -C $Root commit -m "grind: $func canonical-asm grant path filed (stays active) [skip-park-src-guard]" 2>$null | Out-Null
            } elseif ($escRef -match 'INTEGRATION HANDOFF') {
                # integration-handoff-self-serve (owner ruling 2026-08-19): a
                # bytes-proven handoff is pipeline-executable, never a terminal
                # park. Route the filed packet to the Judge; on its
                # ESCALATE(integration-handoff) verdict the driver widens scope /
                # clears the superseded ban and the function STAYS ACTIVE.
                Invoke-JudgeRuling $func ("INTEGRATION HANDOFF filed for $func : $escRef`n" +
                    "Read the escalation entry in docs/grind/decisions.md and the ledger, and verify its bytes claim yourself (the banked form + measurements). If the claim is sound and the remedy is a scope widening and/or a superseded-ban clearance per .claude/rules/integration-handoff-self-serve.md, return ESCALATE with escalate_kind=integration-handoff plus scope_paths=[...] (allowed classes: include/*.h, src/*.c, root-level rule/allowlist *.txt; the denylist is refused mechanically) and/or unban_construct=<substring>. If the claim does not hold, FAIL with the defect.")
            } else {
                # Legacy pending-shaped ref (pre-2026-08-18). No pending states exist
                # anymore: log to the borderline ledger and foreclose silently.
                $date = Get-Date -Format 'yyyy-MM-dd'
                python tools/grinder/grindlib.py log-borderline . $func 'policy-question' "session-filed disposition: $escRef" "FORECLOSED silently (owner ruling 2026-08-31 — no pending states, no packets); re-attemptable if a later owner ruling spends this entry." $date | Out-Null
                $reason = "FORECLOSED (logged to borderline.md; owner ruling 2026-08-31): $escRef"
                Invoke-Eng @('queue', 'foreclose', $func, '--reason', $reason) | Out-Null
                Log "${func}: OWNER-GATED — borderline-logged + FORECLOSED silently: $escRef"
                Journal "$func s$sessionN [$modality] OWNER-GATED — borderline-logged, foreclosed: $($o.headline)"
            }
            # engine/queue.json is where `queue park` wrote the parked status — it
            # MUST be staged, or the park stays as working-tree dirt and the next
            # session's scope check (engine/ is not in AllowedDirtyPattern) reverts
            # it via `git checkout -- .`, un-parking the function and bouncing it
            # straight back to the queue top forever (2026-07-18: ~40 sessions
            # burned re-parking motion_SetMotion; see grinder-park-queue-dirt-deadlock).
            git -C $Root add -- memory/grind docs/grind metrics/events.jsonl engine/queue.json 2>$null
            git -C $Root commit -m "grind: $func owner-gated disposition (ruling 2026-08-18, nothing pending) [skip-park-src-guard]" 2>$null | Out-Null
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
                Revert-SessionEdits $func
                $tier = 'LOW'
                try { $sc = (python tools/scan_hand_coded.py --single $func 2>$null | Out-String)
                      if ($sc -match 'tier=(\w+)') { $tier = $Matches[1] } } catch { }
                $rc = 0
                $ref = (python tools/grinder/grindlib.py autoescalate . $func $stem $tier $rc (Get-Date -Format 'yyyy-MM-dd')).Trim()
                if ($ref -match 'CANONICAL-ASM GRANT PATH') {
                    # Owner ruling 2026-08-18: STRONG tier routes to the pipeline
                    # grant path — the function STAYS ACTIVE for authoring/integration.
                    python tools/grinder/grindlib.py constrain . $func ("canonical-asm GRANT PATH ($ref): author the whole-body form per canonical-asm-authorization-recipe and integrate; no owner wait.") | Out-Null
                    Log "${func}: ESCALATION BACKSTOP — STRONG tier, canonical-asm grant path; stays active."
                    Journal "$func s$sessionN [escalation] AUTO-FILED by driver backstop — canonical-asm grant path (stays active): $ref"
                    git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
                    git -C $Root commit -m "grind: $func auto-filed canonical-asm grant path (backstop) [skip-park-src-guard]" 2>$null | Out-Null
                } else {
                    # The ref carries its own title: RESOLVED BY STANDING RULING (floor <=
                    # ENDGAME_LOCK_MAX_FLOOR) or LADDER EXHAUSTED (wider residual, owner
                    # ruling 2026-09-02) — never claim the standing ruling for the latter.
                    $bsKind = if ($ref -match 'LADDER EXHAUSTED') { 'ladder exhausted, non-endgame residual — owner ruling 2026-09-02' } else { 'standing ruling 2026-07-27' }
                    $bsReason = "FORECLOSED ($bsKind, auto-filed backstop; silent per owner ruling 2026-08-31): $ref"
                    Invoke-Eng @('queue', 'foreclose', $func, '--reason', $bsReason) | Out-Null
                    Log "${func}: EXHAUSTION BACKSTOP — session dodged in escalation modality (floor $($o.floor) >= prior $priorFloor); driver auto-filed + foreclosed."
                    Journal "$func s$sessionN [escalation] AUTO-FILED by driver backstop (session did not self-file) — foreclosed: $ref"
                    git -C $Root add -- memory/grind docs/grind metrics/events.jsonl engine/queue.json 2>$null
                    git -C $Root commit -m "grind: $func foreclosed (exhaustion backstop) [skip-park-src-guard]" 2>$null | Out-Null
                }
            } else {
                python tools/grinder/grindlib.py apply . $func $outPath $modality | Out-Null
                Revert-SessionEdits $func
                Log "${func}: progress applied — floor=$($o.floor), '$($o.headline)'"
                Journal "$func s$sessionN [$modality] floor=$($o.floor): $($o.headline)"
                git -C $Root add -- memory/grind docs/grind metrics/events.jsonl 2>$null
                git -C $Root commit -m "grind: $func ledger s$sessionN update [skip-park-src-guard]" 2>$null | Out-Null
            }
        }
    }
    # spec: oracle checked around every session — src is reverted (or merged) by
    # this point, so any red here means real corruption -> stop, don't limp.
    # A red from STALE build/ artifacts (e.g. a failed retire's leftovers) is
    # not corruption: retry once with a clean rebuild before circuit-breaking
    # (2026-08-11 false-trip). A --rebuild red is real -> break as before.
    if (-not (Test-OracleGreen)) {
        Log "post-session oracle red — retrying with clean rebuild before circuit-break."
        Invoke-Eng @('verify-oracle', '--rebuild') | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Attribute-RedBuild 'post-session'
            Circuit-Break "oracle not green after session on $func"
        } else {
            Log "post-session oracle green after clean rebuild (stale artifacts) — continuing."
        }
    }
    if ($Once) { break }
}
Remove-Item $PidFile -ErrorAction SilentlyContinue
Log "grinder stopped."
