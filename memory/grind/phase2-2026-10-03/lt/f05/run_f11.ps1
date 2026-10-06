# Score the F11 typed trials in tmp/w2/f11/list.txt (f11_trials.py writes it) with the engine sandbox.
param([string]$Root = (Resolve-Path "$PSScriptRoot/../../../../..").Path)
Set-Location $Root
foreach ($line in Get-Content tmp/w2/f11/list.txt) {
    $fn, $file = $line -split ' '
    $s = & tools/wteng.ps1 $Root sandbox $fn --disable all --candidate $file 2>&1 | Select-String '"score"|_insns|error'
    "$fn : $($s -join ' ')"
}
