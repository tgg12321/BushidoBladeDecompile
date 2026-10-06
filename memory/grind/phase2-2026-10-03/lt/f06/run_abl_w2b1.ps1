# Score each ablation candidate in tmp/w2/abl/list.txt (abl_w2b1.py writes it) with the engine sandbox.
param([string]$Root = (Resolve-Path "$PSScriptRoot/../../../../..").Path)
Set-Location $Root
foreach ($line in Get-Content tmp/w2/abl/list.txt) {
    $name, $fn, $file = $line -split ' '
    $out = & tools/wteng.ps1 $Root sandbox $fn --disable all --candidate $file 2>&1
    $score = ($out | Select-String -Pattern '"score"|_insns') -join ' '
    "$name ($fn): $score"
}
