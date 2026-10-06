# Score the typed alternatives behind batch 3's debt rows (the bodies whose raw lines stay as at HEAD).
param([string]$Root = (Resolve-Path "$PSScriptRoot/../../../../..").Path)
Set-Location $Root
$wsl = (Get-Location).Path -replace '\\', '/' -replace '^C:', '/mnt/c'
$rows = @(
    @('cb_a', 'func_80040CB8', 'src/main/309CC.c'),
    @('q_node', 'func_80041688', 'src/main/31D3C.c'),
    @('q_two', 'func_80041688', 'src/main/31D3C.c'),
    @('q_while', 'func_80041688', 'src/main/31D3C.c')
)
foreach ($r in $rows) {
    $opt, $fn, $file = $r
    $out = "tmp/w2/debt3/$opt"
    wsl bash -lc "cd '$wsl' && python3 memory/grind/phase2-2026-10-03/lt/f08/w2b3.py opt=sc_00f8b,$opt out=$out && python3 tmp/w2/extract.py $out/$file $fn $out.$fn.c" | Out-Null
    $s = & tools/wteng.ps1 $Root sandbox $fn --disable all --candidate "$out.$fn.c" 2>&1 | Select-String '"score"'
    "$opt ($fn): $s"
}
