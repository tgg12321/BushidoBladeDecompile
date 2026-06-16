#!/usr/bin/env pwsh
<#
.SYNOPSIS
  Put the current process (and therefore every descendant it spawns) into a Win32
  Job Object with JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE.

.DESCRIPTION
  When the process that created the job exits — gracefully, via Stop-Process, OR on a
  hard crash — Windows atomically terminates EVERY process in the job. Child processes
  inherit job membership automatically (Start-Process does not set
  CREATE_BREAKAWAY_FROM_JOB), so the fleet supervisor calling Enter-KillOnCloseJob once
  at startup guarantees its lanes, their `claude -p` agents, and `wsl.exe` relays cannot
  outlive it as orphans.

  This is the source-side prevention that complements:
    * stop.ps1            — graceful, explicit shutdown
    * tools/reap_orphans.ps1 — catch-all sweep for harnesses we do NOT control (Codex/Omnara)

  It is purely additive and FAIL-OPEN: if the job cannot be created/assigned (already in a
  restrictive job, access denied, etc.) it logs a warning and returns $false; the caller
  continues normally and relies on stop.ps1 + the reaper. It NEVER kills anything itself —
  kill-on-close is the kernel's job, triggered only by THIS process's own exit.

  Nested jobs are supported on Windows 8+/Server 2012+, so this works even when the shell
  is already inside a Windows Terminal / conhost job.

.EXAMPLE
  . tools/fleet/_jobobject.ps1 ; Enter-KillOnCloseJob
.EXAMPLE
  pwsh tools/fleet/_jobobject.ps1 -SelfTest   # prove kill-on-close end-to-end
#>
param([switch]$SelfTest)

$script:JobMgrSource = @'
using System;
using System.Runtime.InteropServices;
public static class FleetJobMgr {
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
    public static extern IntPtr CreateJobObject(IntPtr a, string lpName);
    [DllImport("kernel32.dll")]
    public static extern bool SetInformationJobObject(IntPtr hJob, int infoClass, IntPtr lpInfo, uint cb);
    [DllImport("kernel32.dll", SetLastError = true)]
    public static extern bool AssignProcessToJobObject(IntPtr hJob, IntPtr hProcess);

    [StructLayout(LayoutKind.Sequential)]
    public struct JOBOBJECT_BASIC_LIMIT_INFORMATION {
        public long PerProcessUserTimeLimit;
        public long PerJobUserTimeLimit;
        public uint LimitFlags;
        public UIntPtr MinimumWorkingSetSize;
        public UIntPtr MaximumWorkingSetSize;
        public uint ActiveProcessLimit;
        public UIntPtr Affinity;
        public uint PriorityClass;
        public uint SchedulingClass;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct IO_COUNTERS {
        public ulong ReadOperationCount, WriteOperationCount, OtherOperationCount,
                     ReadTransferCount, WriteTransferCount, OtherTransferCount;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct JOBOBJECT_EXTENDED_LIMIT_INFORMATION {
        public JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimitInformation;
        public IO_COUNTERS IoInfo;
        public UIntPtr ProcessMemoryLimit;
        public UIntPtr JobMemoryLimit;
        public UIntPtr PeakProcessMemoryUsed;
        public UIntPtr PeakJobMemoryUsed;
    }
    public const int  JobObjectExtendedLimitInformation = 9;
    public const uint JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE = 0x2000;

    public static IntPtr CreateKillOnClose() {
        IntPtr h = CreateJobObject(IntPtr.Zero, null);
        if (h == IntPtr.Zero) return IntPtr.Zero;
        var ext = new JOBOBJECT_EXTENDED_LIMIT_INFORMATION();
        ext.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        int len = Marshal.SizeOf(ext);
        IntPtr p = Marshal.AllocHGlobal(len);
        try {
            Marshal.StructureToPtr(ext, p, false);
            if (!SetInformationJobObject(h, JobObjectExtendedLimitInformation, p, (uint)len)) return IntPtr.Zero;
        } finally { Marshal.FreeHGlobal(p); }
        return h;
    }
    public static bool AssignCurrent(IntPtr hJob) {
        return AssignProcessToJobObject(hJob, System.Diagnostics.Process.GetCurrentProcess().Handle);
    }
}
'@

function Enter-KillOnCloseJob {
    [CmdletBinding()] param()
    try {
        if (-not ('FleetJobMgr' -as [type])) { Add-Type -TypeDefinition $script:JobMgrSource -Language CSharp -ErrorAction Stop }
        $h = [FleetJobMgr]::CreateKillOnClose()
        if ($h -eq [IntPtr]::Zero) { Write-Warning "[jobobject] CreateKillOnClose failed; continuing without crash-reaping."; return $false }
        if (-not [FleetJobMgr]::AssignCurrent($h)) {
            $err = [System.Runtime.InteropServices.Marshal]::GetLastWin32Error()
            Write-Warning "[jobobject] AssignProcessToJobObject failed (win32=$err); continuing without crash-reaping."
            return $false
        }
        # PIN the handle for this process's lifetime. We deliberately NEVER CloseHandle:
        # the OS closes it on process exit, which is exactly what triggers kill-on-close.
        $global:__FleetJobHandle = $h
        return $true
    }
    catch { Write-Warning "[jobobject] $_"; return $false }
}

# ─────────────────────────────── self-test ───────────────────────────────
# Proves kill-on-close end to end: a CHILD pwsh creates a kill-on-close job, spawns a
# long-sleeping GRANDCHILD, prints its PID, then exits abruptly. The grandchild must die.
if ($SelfTest) {
    $thisFile = $PSCommandPath
    $childScript = @"
. '$thisFile'
if (-not (Enter-KillOnCloseJob)) { Write-Output 'JOBFAIL'; exit 2 }
`$g = Start-Process pwsh -PassThru -WindowStyle Hidden -ArgumentList @('-NoProfile','-Command','Start-Sleep -Seconds 120')
Write-Output ("GRANDCHILD=" + `$g.Id)
Start-Sleep -Seconds 1   # let the grandchild settle
# exit abruptly -> child's job handle closes -> kill-on-close must kill the grandchild
"@
    $tmp = Join-Path ([System.IO.Path]::GetTempPath()) "fleet_jobtest_$PID.ps1"
    Set-Content -Path $tmp -Value $childScript -Encoding utf8
    try {
        $out = & pwsh -NoProfile -File $tmp 2>&1
        $line = ($out | Where-Object { $_ -match 'GRANDCHILD=(\d+)' })
        if ($out -match 'JOBFAIL') { Write-Host "SELF-TEST: job creation FAILED on this host (fail-open path)." -ForegroundColor Yellow; exit 1 }
        if (-not $line) { Write-Host "SELF-TEST: could not read grandchild pid. out=$out" -ForegroundColor Red; exit 1 }
        $null = ($line -match 'GRANDCHILD=(\d+)'); $gpid = [int]$Matches[1]
        Write-Host "  child exited; grandchild was pid=$gpid. Waiting to confirm kill-on-close..."
        Start-Sleep -Seconds 2
        $still = Get-Process -Id $gpid -ErrorAction SilentlyContinue
        if ($still) {
            Write-Host "SELF-TEST FAILED: grandchild $gpid survived the job-owner's exit." -ForegroundColor Red
            Stop-Process -Id $gpid -Force -ErrorAction SilentlyContinue
            exit 1
        }
        Write-Host "SELF-TEST PASSED: grandchild died with its job owner (kill-on-close works)." -ForegroundColor Green
        exit 0
    }
    finally { Remove-Item $tmp -ErrorAction SilentlyContinue }
}
