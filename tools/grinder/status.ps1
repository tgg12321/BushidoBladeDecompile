<#
Grinder status — the walk-away trust surface.
#>
$Root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$pidf = Join-Path $Root 'tmp\grind\grind.lock'
$alive = $false
if (Test-Path $pidf) {
    $gpid = Get-Content $pidf
    $alive = [bool](Get-Process -Id $gpid -ErrorAction SilentlyContinue)
}
Write-Host "=== Grinder status ($(Get-Date -Format 'yyyy-MM-dd HH:mm')) ===" -ForegroundColor Cyan
Write-Host ("driver: " + $(if ($alive) { "RUNNING (pid $gpid)" } else { "stopped" }))
# WSL kernel-object leak watchdog (see tools/check_wsl_leak.ps1): each wsl.exe
# call leaks nonpaged pool that only a reboot frees; audio degrades ~2.5 GB.
try {
    $npGB = [math]::Round((Get-Counter '\Memory\Pool Nonpaged Bytes').CounterSamples[0].CookedValue / 1GB, 2)
  # nonpaged-pool level measured but NOT printed (owner directive 2026-08-19:
  # no pool levels / reboot reminders in reports; surface breakage, not numbers)
} catch { }
# Bridge state — the lever on the leak above (default ON since 2026-08-14).
try {
    . (Join-Path $Root 'tools\wsl_bridge.ps1')
    $up = Test-WslBridge
    if ($env:BB2_WSL_BRIDGE -eq '0') { Write-Host "wsl bridge: DISABLED (BB2_WSL_BRIDGE=0) — engine calls leak 1 job each" }
    else { Write-Host ("wsl bridge: enabled, daemon " + $(if ($up) { 'up' } else { 'down (starts on next engine call)' })) }
} catch { }

$topRaw = & (Join-Path $Root 'tools\wteng.ps1') main queue next 2>$null | Out-String
$top = $null
$i = $topRaw.IndexOf('{'); $j = $topRaw.LastIndexOf('}')
if ($i -ge 0 -and $j -gt $i) {
    try { $top = $topRaw.Substring($i, $j - $i + 1) | ConvertFrom-Json } catch { $top = $null }
}
if ($top -and $top.func) {
    $origin = if ($top.origin) { $top.origin } else { 'queue' }
    Write-Host "`ntarget: $($top.func) (src/$($top.file).c)  origin=$origin"
    $sp = Join-Path $Root "memory\grind\$($top.func)\state.json"
    if (Test-Path $sp) {
        $st = Get-Content $sp -Raw | ConvertFrom-Json
        $next = (python (Join-Path $Root 'tools\grinder\grindlib.py') modality $Root $top.func).Trim()
        Write-Host "sessions: $($st.session_count)   next modality: $next"
        Write-Host "floor trajectory: $(($st.floor_history | ForEach-Object { $_.floor }) -join ' -> ')"
        Write-Host "frontier ($($st.frontier.Count)):"
        $st.frontier | ForEach-Object { Write-Host "  - $($_.hypothesis)" }
        Write-Host "judge constraints: $($st.judge_constraints.Count)"
        # Sibling ledgers (2026-09-04): which other ledgers this one is coupled
        # to, and whether any of them moved below this floor unread.
        try {
            $sib = (python (Join-Path $Root 'tools\grinder\grindlib.py') siblings $Root $top.func | Out-String)
            $names = @([regex]::Matches($sib, '(?m)^  - (\S+)') | ForEach-Object { $_.Groups[1].Value })
            $unspent = @([regex]::Matches($sib, '-> UNSPENT')).Count
            $forced = $sib -match 'FORCED REDERIVE'
            if ($names.Count) {
                Write-Host ("siblings: " + ($names -join ', ') + "  unspent: $unspent" + $(if ($forced) { "  (next session: forced rederive on sibling progress)" } else { '' }))
            }
        } catch { }
        $hyp = Join-Path $Root "memory\grind\$($top.func)\hypotheses.md"
        if (Test-Path $hyp) {
            $killed = @(Select-String -Path $hyp -Pattern 'verdict: KILLED').Count
            $conf   = @(Select-String -Path $hyp -Pattern 'verdict: CONFIRMED').Count
            Write-Host "hypotheses: $conf confirmed / $killed killed"
        }
    } else { Write-Host "(no ledger yet — grind not started on this target)" }
}

# Owner actions (owner ruling 2026-09-15): blockers the no-progress tripwire
# found to be outside every session's reach. The pipeline is NOT waiting on
# these — the item was rotated and returns automatically — but each one is a
# one-line operator edit that unblocks a function, so surface them FIRST.
$oa = @()
try { $oa = @(python (Join-Path $Root 'tools\grinder\grindlib.py') owner-actions $Root 2>$null | Where-Object { $_ }) } catch { }
if ($oa.Count) {
    Write-Host "`n=== OPEN owner actions ($($oa.Count)) — one-line operator edits, nothing is blocked on them ===" -ForegroundColor Yellow
    $oa | ForEach-Object { Write-Host "  $_" }
}

Write-Host "`n=== completions (grinder Match: commits) ===" -ForegroundColor Cyan
git -C $Root log --oneline --grep='(grinder' -n 10
Write-Host "`n=== recent judge decisions ===" -ForegroundColor Cyan
$dec = Join-Path $Root 'docs\grind\decisions.md'
if (Test-Path $dec) { Get-Content $dec -Tail 20 } else { Write-Host "(none yet)" }
Write-Host "`n=== journal tail ===" -ForegroundColor Cyan
$jr = Join-Path $Root 'docs\grind\journal.md'
if (Test-Path $jr) { Get-Content $jr -Tail 10 } else { Write-Host "(none yet)" }
