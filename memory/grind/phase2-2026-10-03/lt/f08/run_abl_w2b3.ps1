# Score each candidate in tmp/w2/abl3/list.txt (abl_w2b3.py writes it) with the engine sandbox.
param([string]$Root = (Resolve-Path "$PSScriptRoot/../../../../..").Path, [string]$List = "tmp/w2/abl3/list.txt")
Set-Location $Root
foreach ($line in Get-Content $List) {
    $name, $fn, $file = $line -split ' '
    $s = & tools/wteng.ps1 $Root sandbox $fn --disable all --candidate $file 2>&1 | Select-String '"score"|_insns|rror'
    "$name ($fn): $($s -join ' ')"
}
