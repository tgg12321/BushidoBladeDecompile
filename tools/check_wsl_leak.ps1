# check_wsl_leak.ps1 — measure the WSL kernel-object leak and nonpaged-pool state.
#
# Every wsl.exe invocation leaks ~1 Job object (pool tags PsJb/Job) and pins
# ~11 File objects (tag File) — measured identically on WSL 2.6.3.0 and
# 2.7.11.0 on Windows 11 26200.x (2026-08-12). Grinder workloads make
# thousands of wsl.exe calls per day, so nonpaged pool climbs by GBs; the
# host's audio starts stuttering (kernel latency) around ~2.5 GB and the WSL
# service itself can wedge (Wsl/Service/E_UNEXPECTED). Nothing but a REBOOT
# frees the leaked objects — they are kernel-referenced, not handle-held, so
# killing processes and `wsl --shutdown` do not drain them.
#
# Usage:
#   pwsh tools/check_wsl_leak.ps1            # report pool state + top tags
#   pwsh tools/check_wsl_leak.ps1 -Probe     # also run the 100-call leak probe
#
# Related: memory/project/wsl-kernel-object-leak-audio.md, the grind.ps1
# pre-flight watchdog, and docs/grind/journal.md around 2026-08-12.
param([switch]$Probe)

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

if ($Probe) {
    "probe: running 100 no-op wsl.exe calls..."
    $b = Get-Tags @('PsJb','File')
    1..100 | ForEach-Object { wsl.exe -e true 2>$null | Out-Null }
    Start-Sleep -Seconds 10
    $a = Get-Tags @('PsJb','File')
    $dJob = $a['PsJb'][1] - $b['PsJb'][1]
    $dFile = $a['File'][1] - $b['File'][1]
    "probe result: +$dJob Job objects, +$dFile File objects per 100 calls"
    if ($dJob -ge 50) { "LEAK PRESENT (~$([math]::Round($dJob/100.0,2)) jobs/call) — this WSL build still leaks." }
    else { "leak NOT detected — this WSL build appears fixed; update the watchdog docs." }
}
