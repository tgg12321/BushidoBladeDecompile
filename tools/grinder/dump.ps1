# dump.ps1 — pass-attribution capture for grind sessions (process improvement #4, 2026-08-18).
# Runs the project's EXACT cpp|cc1 front half for the TU containing <func>, with -da, into
# tmp/grind/<func>/dumps/, so sessions READ which GCC pass produced a divergence instead of
# hypothesizing it across sessions (s6-s8 of func_80017848 mis-attributed one copy for three
# sessions; the .combine dump names the pass in one read).
#
# Flags come from engine/buildconfig.py (the Makefile mirror — buildconfig-mirror-drift-false-
# mismatch is why we never hand-copy them). Canonical cc1 = tools/gcc-2.7.2/build/cc1 (what the
# oracle pipeline runs). For the BB2_*_DEBUG hook output use the INSTRUMENTED cc1 at
# tools/gcc-2.7.2/cc1 (instrumented-cc1-location) by passing -Instrumented.
param(
    [Parameter(Mandatory = $true)][string]$Func,
    [string]$Stem = '',
    [switch]$Instrumented
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
Set-Location $Root

if (-not $Stem) {
    $q = Get-Content 'engine/queue.json' -Raw | ConvertFrom-Json
    $item = $q.items | Where-Object { $_.func -eq $Func } | Select-Object -First 1
    if ($item) { $Stem = [string]$item.file }
}
if (-not $Stem) {
    # fall back to grepping src for the definition
    $hit = Select-String -Path 'src/*.c' -Pattern "^[A-Za-z].*\b$Func\s*\(" | Select-Object -First 1
    if ($hit) { $Stem = [IO.Path]::GetFileNameWithoutExtension($hit.Path) }
}
if (-not $Stem) { Write-Error "cannot resolve TU stem for $Func (not in queue.json, no definition found)"; exit 1 }

$outDir = "tmp/grind/$Func/dumps"
New-Item -ItemType Directory -Force $outDir | Out-Null

# The WSL-side one-liner is generated as a script file (PowerShell-first policy:
# no inline multi-command -c strings).
$cc1flag = if ($Instrumented) { 'tools/gcc-2.7.2/cc1' } else { '' }
$sh = @"
set -e
cd "`$(wslpath -a '$((Get-Location).Path)')" 2>/dev/null || cd '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile'
source .venv/bin/activate
PYTHONPATH=. python3 - <<'PYEOF'
import shlex, subprocess, sys
from engine import buildconfig as B
stem = '$Stem'; out = '$outDir'; func = '$Func'
cc1 = '$cc1flag' or B.CC1
src = f'src/{stem}.c'
cpp = f"{B.CPP} {B.CPP_FLAGS} {B.CPP_DEFS} {src}"
flags = B.CC_FLAGS_GP if hasattr(B, 'GP_FILES') and stem in getattr(B, 'GP_FILES', ()) else B.CC_FLAGS
cc = f"{cc1} {flags} -da -dumpbase {out}/{stem} -o {out}/{stem}.s"
p1 = subprocess.run(shlex.split(cpp), capture_output=True, text=True)
if p1.returncode:
    sys.stderr.write(p1.stderr); sys.exit(1)
p2 = subprocess.run(shlex.split(cc), input=p1.stdout, capture_output=True, text=True)
sys.stderr.write(p2.stderr[-2000:] if p2.stderr else '')
if p2.returncode:
    sys.exit(p2.returncode)
print(f"dumps for {func} (TU {stem}) written under {out}/")
PYEOF
ls -la '$outDir' | tail -n +2
"@
$tmpSh = "tmp/grind/$Func/run_dump.sh"
Set-Content $tmpSh -Value $sh -Encoding utf8 -NoNewline
# Direct wsl.exe with a script FILE argument — no inline -c string, no nested
# quoting (PowerShell-first policy); pwsh-spawned bash lacks wsl on PATH, so
# tools/wsl.sh is not usable from here.
$drive = ($Root.Substring(0, 1)).ToLower()
$wslSh = "/mnt/$drive/" + ($Root.Substring(3) -replace '\\', '/') + "/$tmpSh"
wsl.exe bash -c "sed -i 's/\r$//' '$wslSh' && bash '$wslSh'"
Write-Host "Pass files: .combine (fold/copy survival) .lreg/.greg (allocation) .sched/.sched2 (ordering) .loop (LICM) .cse/.cse2 (folds) .flow (liveness) .jump/.jump2 (cross-jump)"
