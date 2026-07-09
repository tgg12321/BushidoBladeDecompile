' Windowless launcher for the permuter reaper.
'
' A scheduled task that runs pwsh.exe (a console app) flashes a console window
' every interval, because -WindowStyle Hidden only hides AFTER creation. wscript
' is a GUI-subsystem host with no console, and WScript.Shell.Run with window
' style 0 creates the child already hidden (SW_HIDE) — so pwsh, and the wsl.exe
' it invokes, never show a window. Point the PermuterReaper task at THIS file.
Option Explicit
Dim shell, repo, cmd
Set shell = CreateObject("WScript.Shell")
repo = "C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile\tools\permuter_reaper.ps1"
cmd = "pwsh.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File " _
      & Chr(34) & repo & Chr(34) & " -TtlSeconds 3600"
' 0 = hidden window, False = do not wait (fire and forget)
shell.Run cmd, 0, False
