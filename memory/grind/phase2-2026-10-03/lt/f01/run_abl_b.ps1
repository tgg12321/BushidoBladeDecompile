# Run engine sandbox for each ablation candidate in <dir>/list.txt (abl_a.py writes tmp/p2/lt/f01/abl,
# abl_b.py tmp/p2/lt/f01/ablb); print the score lines. Usage: pwsh run_abl_b.ps1 [dir]
param([string]$Dir = "tmp/p2/lt/f01/ablb")
$root = "C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile"
Set-Location $root
foreach ($line in Get-Content "$Dir/list.txt") {
    $name, $fn = $line -split ' '
    $out = & tools/wteng.ps1 main sandbox $fn --disable all --candidate "$Dir/$name.c" 2>&1
    $score = ($out | Select-String -Pattern '"score"|_insns' ) -join ' '
    "$name ($fn): $score"
}
