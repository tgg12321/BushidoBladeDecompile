# Run engine sandbox for each ablation candidate listed in tmp/p2/lt/f01/abl/list.txt (python memory/grind/phase2-2026-10-03/lt/f01/abl_a.py writes it); print the score lines.
$root = "C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile"
Set-Location $root
foreach ($line in Get-Content tmp/p2/lt/f01/abl/list.txt) {
    $name, $fn = $line -split ' '
    $out = & tools/wteng.ps1 main sandbox $fn --disable all --candidate "tmp/p2/lt/f01/abl/$name.c" 2>&1
    $score = ($out | Select-String -Pattern '"score"|_insns' ) -join ' '
    "$name ($fn): $score"
}
