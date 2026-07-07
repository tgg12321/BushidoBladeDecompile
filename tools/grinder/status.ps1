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
        $hyp = Join-Path $Root "memory\grind\$($top.func)\hypotheses.md"
        if (Test-Path $hyp) {
            $killed = @(Select-String -Path $hyp -Pattern 'verdict: KILLED').Count
            $conf   = @(Select-String -Path $hyp -Pattern 'verdict: CONFIRMED').Count
            Write-Host "hypotheses: $conf confirmed / $killed killed"
        }
    } else { Write-Host "(no ledger yet — grind not started on this target)" }
}

Write-Host "`n=== completions (grinder Match: commits) ===" -ForegroundColor Cyan
git -C $Root log --oneline --grep='(grinder' -n 10
Write-Host "`n=== recent judge decisions ===" -ForegroundColor Cyan
$dec = Join-Path $Root 'docs\grind\decisions.md'
if (Test-Path $dec) { Get-Content $dec -Tail 20 } else { Write-Host "(none yet)" }
Write-Host "`n=== journal tail ===" -ForegroundColor Cyan
$jr = Join-Path $Root 'docs\grind\journal.md'
if (Test-Path $jr) { Get-Content $jr -Tail 10 } else { Write-Host "(none yet)" }
