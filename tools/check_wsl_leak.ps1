# check_wsl_leak.ps1 — measure the WSL kernel-object leak and nonpaged-pool state.
#
# Every wsl.exe INVOCATION leaks ~1 Job object (pool tag PsJb, ~14 KB nonpaged)
# and pins ~4-7 File objects. Grinder workloads make thousands of wsl.exe calls,
# so nonpaged pool climbs by GBs; host audio starts stuttering (kernel latency)
# around ~2.5 GB and the WSL service itself can wedge (Wsl/Service/E_UNEXPECTED).
# Nothing but a REBOOT frees them — they are kernel-referenced, not handle-held,
# so killing processes and `wsl --shutdown` do not drain them.
#
# Re-verified 2026-08-12 post-reboot on WSL 2.7.11.0 / kernel 6.18.33.2-2: still
# present. Controls that pin the mechanism (100 calls each):
#     cmd.exe /c exit ............ 0.01 jobs/call  (NOT generic process creation)
#     separate wsl.exe calls ..... 0.99 jobs/call
#     + a keep-alive holder ...... 1.01 jobs/call  (NOT instance cold-start)
#     ONE wsl.exe, stdin-fed ..... 0.02 jobs/cmd   (=> persistent bridge is the fix)
#
# Usage:
#   pwsh tools/check_wsl_leak.ps1            # report pool state + leak tags
#   pwsh tools/check_wsl_leak.ps1 -Top       # + top-15 nonpaged consumers, uptime
#   pwsh tools/check_wsl_leak.ps1 -Probe     # + controlled probe (idle baseline
#                                            #   -> N calls -> 60 s drain check)
#
# Related: memory/project/wsl-kernel-object-leak-audio.md, the grind.ps1
# pre-flight watchdog, and docs/grind/journal.md around 2026-08-12.
param([switch]$Probe, [switch]$Top, [int]$N = 100, [int]$IdleSec = 30)

Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Collections.Generic;
public static class BB2PoolTags {
    [DllImport("ntdll.dll")]
    public static extern int NtQuerySystemInformation(int cls, IntPtr buf, int len, out int outLen);
    public static Dictionary<string,long[]> Snapshot() {
        int len = 8 * 1024 * 1024; IntPtr buf = Marshal.AllocHGlobal(len); int outLen;
        var rows = new Dictionary<string,long[]>();
        try {
            if (NtQuerySystemInformation(22, buf, len, out outLen) != 0) return rows;
            int count = Marshal.ReadInt32(buf, 0);
            // Layout empirically: array at +4, stride 28, 32-bit fields:
            // Tag[4] PagedAllocs PagedFrees PagedUsed NpAllocs NpFrees NpUsed
            for (int i = 0; i < count; i++) {
                int off = 4 + i * 28;
                byte[] t = new byte[4];
                Marshal.Copy(new IntPtr(buf.ToInt64() + off), t, 0, 4);
                long live = (long)Marshal.ReadInt32(buf, off + 16) - Marshal.ReadInt32(buf, off + 20);
                long np = (uint)Marshal.ReadInt32(buf, off + 24);
                rows[Encoding.ASCII.GetString(t)] = new long[]{ np, live };
            }
        } finally { Marshal.FreeHGlobal(buf); }
        return rows;
    }
}
'@

function Get-Tags([string[]]$names) {
    $snap = [BB2PoolTags]::Snapshot()
    $out = @{}
    foreach ($n in $names) { $out[$n] = if ($snap.ContainsKey($n)) { $snap[$n] } else { @(0L, 0L) } }
    return $out
}

$np = (Get-Counter '\Memory\Pool Nonpaged Bytes').CounterSamples[0].CookedValue
$npGB = [math]::Round($np / 1GB, 2)
$verdict = if ($npGB -ge 2.5) { 'DEGRADED — audio stutter likely, REBOOT RECOMMENDED' }
           elseif ($npGB -ge 1.8) { 'elevated — reboot at next opportunity' }
           else { 'healthy' }
"nonpaged pool: $npGB GB — $verdict"

$tags = Get-Tags @('PsJb','Job','File')
"leak tags: PsJb live=$($tags['PsJb'][1]) ($([math]::Round($tags['PsJb'][0]/1MB,1)) MB)  Job live=$($tags['Job'][1])  File live=$($tags['File'][1]) ($([math]::Round($tags['File'][0]/1MB,1)) MB)"

if ($Top) {
    ""
    "uptime: {0:N1} h" -f ((Get-Date) - (Get-CimInstance Win32_OperatingSystem).LastBootUpTime).TotalHours
    "tag        nonpaged MB    live objects"
    [BB2PoolTags]::Snapshot().GetEnumerator() |
        Sort-Object { -$_.Value[0] } | Select-Object -First 15 |
        ForEach-Object { "{0,-6}  {1,12:N1}  {2,14:N0}" -f $_.Key, ($_.Value[0]/1MB), $_.Value[1] }
}

if ($Probe) {
    # An IDLE baseline of the same shape comes first: without it, background
    # traffic (a running grinder) can mask or fabricate the verdict — that is
    # exactly how the old no-baseline probe produced a false "fixed" reading.
    # A cmd.exe control arm follows, so a positive result cannot be generic
    # process-creation cost masquerading as a WSL leak.
    ""
    "probe: $IdleSec s idle baseline -> $N cmd.exe control -> $N wsl.exe -> 60 s drain..."
    $b0 = Get-Tags @('PsJb','File')
    Start-Sleep -Seconds $IdleSec
    $b1 = Get-Tags @('PsJb','File')
    $idleRate = ($b1['PsJb'][1] - $b0['PsJb'][1]) / [double]$IdleSec    # PsJb/sec of noise

    $swC = [Diagnostics.Stopwatch]::StartNew()
    1..$N | ForEach-Object { cmd.exe /c exit }
    $swC.Stop()
    $c = Get-Tags @('PsJb','File')
    $dCtl = ($c['PsJb'][1] - $b1['PsJb'][1]) - ($idleRate * $swC.Elapsed.TotalSeconds)

    $sw = [Diagnostics.Stopwatch]::StartNew()
    1..$N | ForEach-Object { wsl.exe bash -c 'true' 2>$null | Out-Null }
    $sw.Stop()
    $a = Get-Tags @('PsJb','File')
    Start-Sleep -Seconds 60
    $d = Get-Tags @('PsJb','File')

    $dJob  = ($a['PsJb'][1] - $c['PsJb'][1]) - ($idleRate * $sw.Elapsed.TotalSeconds)
    $dFile = $a['File'][1] - $c['File'][1]
    $rJob  = $d['PsJb'][1] - $c['PsJb'][1]      # retained after drain
    "  control (cmd.exe)     : {0,6:N2} jobs/call" -f ($dCtl/$N)
    "  wsl.exe               : {0,6:N2} jobs/call, {1,5:N2} File/call ({2:N2} s/call)" -f `
        ($dJob/$N), ($dFile/$N), ($sw.Elapsed.TotalSeconds/$N)
    "  wsl.exe after 60s drain: {0,5:N2} jobs/call retained" -f ($rJob/$N)
    if (($rJob/$N) -ge 0.5) {
        "LEAK PRESENT (~{0:N2} jobs/call, ~{1:N0} KB nonpaged/call) — reboot is the only drain." -f `
            ($rJob/$N), (($rJob/$N) * 14)
        "  Mitigation: cut wsl.exe INVOCATION count (persistent stdin-fed bash bridge = ~0.02/cmd)."
    } else {
        "Job leak NOT detected this run. Repeat before trusting it — a single 0.00 reading"
        "  has been an outlier here (2026-08-12); confirm with 2+ runs and the control arm."
    }
}
