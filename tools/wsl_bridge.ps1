#!/usr/bin/env pwsh
# tools/wsl_bridge.ps1 — persistent WSL command bridge.
#
# WHY: every wsl.exe INVOCATION leaks ~1 kernel Job object (~14 KB nonpaged,
# reboot-only drain). Measured 2026-08-12 (100 calls/arm):
#     cmd.exe control ........... 0.01 jobs/call   (not generic proc creation)
#     separate wsl.exe calls .... 0.99 jobs/call
#     + keep-alive holder ....... 1.01 jobs/call   (not instance cold-start)
#     ONE wsl.exe, stdin-fed .... 0.02 jobs/cmd    <= what this file gives you
# Thousands of grind calls/day therefore cost GBs of nonpaged pool, which
# degrades host audio past ~2.5 GB and can wedge the WSL service itself. The
# only lever we control is INVOCATION COUNT: keep one bash session alive and
# feed it commands. See memory/project/wsl-kernel-object-leak-audio.md.
#
# TRANSPORT: request/response FILES under $env:TEMP\bb2_wsl_bridge\.
# Named pipes were tried first and abandoned — a minimal two-process pipe round
# trip would not deliver a line in this environment (client connects and sends,
# server's ReadLine never returns, pipe then breaks). Files are slower per call
# (~50 ms poll) but that is nothing against a ~300 ms wsl.exe spawn, and they
# are inspectable when something goes wrong.
#
# MODES
#   pwsh tools/wsl_bridge.ps1 -Daemon    # run the daemon (clients auto-start it)
#   pwsh tools/wsl_bridge.ps1 -Status    # up? which pid? busy?
#   pwsh tools/wsl_bridge.ps1 -Stop      # shut it down
#   . tools/wsl_bridge.ps1               # dot-source -> Invoke-WslBridge
#
# FAIL-OPEN BY DESIGN: Invoke-WslBridge returns $null on ANY bridge problem
# (daemon down/stale, busy, malformed response, dead bash session) and the
# caller MUST fall back to a direct `wsl bash -c`. The bridge is an
# optimization, never a dependency — it must not be able to wedge the build.
param([switch]$Daemon, [switch]$Status, [switch]$Stop)

$BridgeDir = Join-Path ([IO.Path]::GetTempPath()) 'bb2_wsl_bridge'
$Beat      = Join-Path $BridgeDir 'daemon.beat'      # pid + heartbeat timestamp
$StopFile  = Join-Path $BridgeDir 'daemon.stop'
$LogFile   = Join-Path $BridgeDir 'daemon.log'
$BeatMaxAge = 30        # seconds; older heartbeat => daemon presumed dead

function Write-BridgeLog([string]$m) {
    try { "[$(Get-Date -f 'HH:mm:ss')] $m" | Add-Content $LogFile -EA SilentlyContinue } catch {}
}

# ---------------------------------------------------------------- client ----
# Live = the beat file names a pid that is still alive.
#
# Liveness is deliberately NOT based on the timestamp's freshness. The daemon is
# single-threaded, so while it runs a long command (verify-oracle, a sweep) it
# cannot refresh the beat — a freshness test therefore declared a perfectly
# healthy daemon dead after 30 s, and made in-flight clients abandon a command
# that was still running. The timestamp is kept for humans (-Status) only.
# A hard-killed daemon leaves a stale beat, but its pid is gone, so this catches it.
function Test-WslBridge {
    try {
        if (-not (Test-Path $Beat)) { return $false }
        $bpid = ((Get-Content $Beat -Raw -EA Stop).Trim() -split '\s+', 2)[0]
        return [bool](Get-Process -Id ([int]$bpid) -EA SilentlyContinue)
    } catch { return $false }
}

function Start-WslBridgeDaemon {
    $self = $PSCommandPath
    if (-not $self) { $self = Join-Path $PSScriptRoot 'wsl_bridge.ps1' }
    Start-Process pwsh -WindowStyle Hidden -ArgumentList `
        '-NoProfile','-File',"`"$self`"",'-Daemon' | Out-Null
    for ($i = 0; $i -lt 60; $i++) {          # up to ~15 s for first bash spin-up
        Start-Sleep -Milliseconds 250
        if (Test-WslBridge) { return $true }
    }
    return $false
}

# Runs <Command> in <WslCwd> through the persistent bash session.
# RETURNS: [pscustomobject] @{ code; out; err }, or $null if the bridge could
#          not be used — the caller must then fall back to a direct wsl.exe.
# It deliberately does NOT write to the host streams and does NOT return a bare
# exit code: emitting output and returning the code on the same pipeline makes
# them indistinguishable to the caller (`@('hello', 0)`). The caller renders.
function Invoke-WslBridge {
    param([Parameter(Mandatory)][string]$WslCwd,
          [Parameter(Mandatory)][string]$Command,
          [switch]$AutoStart,
          [int]$BusyWaitMs = 2000)
    try {
        if (-not (Test-WslBridge)) {
            if (-not $AutoStart) { return $null }
            if (-not (Start-WslBridgeDaemon)) { return $null }
        }
        $id  = [guid]::NewGuid().ToString('N')
        $req = Join-Path $BridgeDir "$id.req"
        $res = Join-Path $BridgeDir "$id.res"
        $tmp = "$req.tmp"
        @{ cwd = $WslCwd; cmd = $Command } | ConvertTo-Json -Compress | Set-Content $tmp -Encoding utf8
        Move-Item $tmp $req -Force            # atomic publish: no half-written .req

        # Wait for pickup. If the daemon is busy with a long command it will not
        # take ours promptly — rather than queue behind it, give up and let the
        # caller use a direct wsl.exe (correct, just leaky).
        $sw = [Diagnostics.Stopwatch]::StartNew()
        while ((Test-Path $req) -and $sw.ElapsedMilliseconds -lt $BusyWaitMs) { Start-Sleep -Milliseconds 25 }
        if (Test-Path $req) { Remove-Item $req -Force -EA SilentlyContinue; return $null }

        # Picked up: now wait as long as the command needs, but bail if the
        # daemon dies mid-command (stale heartbeat) so we never hang forever.
        while (-not (Test-Path $res)) {
            Start-Sleep -Milliseconds 40
            if (-not (Test-WslBridge)) { return $null }
        }
        $r = Get-Content $res -Raw -Encoding utf8 | ConvertFrom-Json
        Remove-Item $res -Force -EA SilentlyContinue
        if ($null -eq $r -or $null -eq $r.code) { return $null }
        return [pscustomobject]@{ code = [int]$r.code; out = @($r.out); err = @($r.err) }
    } catch { return $null }
}

# ---------------------------------------------------------------- daemon ----
function Start-BashSession {
    $psi = [Diagnostics.ProcessStartInfo]::new('wsl.exe', 'bash -s')
    $psi.RedirectStandardInput = $true; $psi.RedirectStandardOutput = $true
    $psi.UseShellExecute = $false
    # BOTH directions must be UTF-8. StandardInputEncoding defaults to the
    # console's ANSI codepage, which mangles non-ASCII on the way IN (a command
    # containing "café" reached bash as "caf?").
    $psi.StandardOutputEncoding = [Text.Encoding]::UTF8
    $psi.StandardInputEncoding  = [Text.UTF8Encoding]::new($false)
    $p = [Diagnostics.Process]::Start($psi)
    $p.StandardInput.AutoFlush = $true      # else the command sits in the buffer
    [pscustomobject]@{ proc = $p }
}

function Invoke-InSession($sess, [string]$cwd, [string]$cmd) {
    $sentinel = '__BB2_END__'
    $errTag   = '__BB2_ERR__'
    # Subshell so cd/env/set changes cannot leak into the next command on this
    # reused session. Stderr goes to a temp file and replays tagged on stdout,
    # so this layer only ever reads ONE stream (an async stderr reader was tried
    # and hung). The sentinel carries the subshell's exit status.
    $sess.proc.StandardInput.WriteLine(
        "__bb2e=`$(mktemp); ( cd '$cwd' && $cmd ) 2>`"`$__bb2e`"; __bb2c=`$?; " +
        "sed -e 's/^/$errTag/' `"`$__bb2e`" 2>/dev/null; rm -f `"`$__bb2e`"; " +
        "printf '$sentinel%d\n' `"`$__bb2c`"")
    $out = [Collections.Generic.List[string]]::new()
    $err = [Collections.Generic.List[string]]::new()
    while ($true) {
        $line = $sess.proc.StandardOutput.ReadLine()
        if ($null -eq $line) { return $null }                  # session died
        if ($line.StartsWith($sentinel)) {
            return [pscustomobject]@{ code = [int]$line.Substring($sentinel.Length); out = $out; err = $err }
        }
        if ($line.StartsWith($errTag)) { $err.Add($line.Substring($errTag.Length)) }
        else { $out.Add($line) }
    }
}

New-Item -ItemType Directory -Path $BridgeDir -Force | Out-Null

if ($Status) {
    if (Test-WslBridge) { "wsl bridge: UP — $((Get-Content $Beat -Raw).Trim())" }
    else { "wsl bridge: down" }
    return
}
if ($Stop) {
    if (-not (Test-WslBridge)) { "wsl bridge: already down"; return }
    New-Item $StopFile -ItemType File -Force | Out-Null
    for ($i = 0; $i -lt 40; $i++) { Start-Sleep -Milliseconds 250; if (-not (Test-WslBridge)) { break } }
    if (Test-WslBridge) { "wsl bridge: still up — daemon may be mid-command" }
    else { "wsl bridge: stopped" }
    return
}
if (-not $Daemon) { return }   # dot-sourced: expose functions only

# --- daemon main: one bash session, serialized command service ---------------
if (Test-WslBridge) { Write-BridgeLog "daemon already running; exiting"; return }
Remove-Item $StopFile -Force -EA SilentlyContinue
Write-BridgeLog "daemon starting (pid $PID)"
"$PID $(Get-Date -Format o)" | Set-Content $Beat
$sess = $null
$idle = [Diagnostics.Stopwatch]::StartNew()
try {
    $beatSw = [Diagnostics.Stopwatch]::StartNew()
    while (-not (Test-Path $StopFile)) {
        # Refresh the beat occasionally — NOT every iteration. Rewriting it in a
        # 25 ms loop while clients are reading it produced sharing violations,
        # and the throw killed the daemon (it kept "mysteriously" restarting).
        # Wrapped as well, because a losing race here must never be fatal.
        if ($beatSw.Elapsed.TotalSeconds -ge 5) {
            try { "$PID $(Get-Date -Format o)" | Set-Content $Beat -EA Stop; $beatSw.Restart() } catch {}
        }
        # A daemon nobody uses should not hold a bash session forever.
        if ($idle.Elapsed.TotalHours -ge 2) { Write-BridgeLog 'idle 2h; exiting'; break }

        $reqs = @(Get-ChildItem $BridgeDir -Filter '*.req' -EA SilentlyContinue | Sort-Object CreationTimeUtc)
        if ($reqs.Count -eq 0) { Start-Sleep -Milliseconds 25; continue }
        $idle.Restart()
        $req = $reqs[0]
        try {
            $body = Get-Content $req.FullName -Raw -Encoding utf8 | ConvertFrom-Json
            Remove-Item $req.FullName -Force -EA SilentlyContinue   # mark picked up
            if ($null -eq $sess -or $sess.proc.HasExited) {
                $sess = Start-BashSession
                Write-BridgeLog "new bash session pid $($sess.proc.Id)"
            }
            $r = Invoke-InSession $sess $body.cwd $body.cmd
            if ($null -eq $r) {
                $sess = $null
                Write-BridgeLog 'bash session died mid-command; no response (client falls back)'
            } else {
                $out = Join-Path $BridgeDir "$($req.BaseName).res"
                @{ code = $r.code; out = @($r.out); err = @($r.err) } |
                    ConvertTo-Json -Compress -Depth 4 | Set-Content "$out.tmp" -Encoding utf8
                Move-Item "$out.tmp" $out -Force        # atomic: client never sees a partial .res
            }
        } catch { Write-BridgeLog "request error: $_" }
    }
} finally {
    if ($sess -and -not $sess.proc.HasExited) { $sess.proc.Kill() }
    # Only clear the beat if it is still OURS. A departing daemon that blindly
    # deleted the file would erase a newer daemon's heartbeat, and every client
    # would then see "down" and spawn yet another one.
    try {
        if ((Test-Path $Beat) -and ((Get-Content $Beat -Raw).Trim() -split '\s+', 2)[0] -eq "$PID") {
            Remove-Item $Beat -Force -EA SilentlyContinue
        }
    } catch {}
    Remove-Item $StopFile -Force -EA SilentlyContinue
    Write-BridgeLog "daemon exit (pid $PID)"
}
