# Score the typed alternatives behind batch 5's debt rows (generator options; the landed bodies keep HEAD's lines).
param([string]$Root = (Resolve-Path "$PSScriptRoot/../../../../..").Path)
Set-Location $Root
$wsl = (Get-Location).Path -replace '\\', '/' -replace '^C:', '/mnt/c'
$rows = @(
    @('fdb0', 'func_8002FDB0', 'src/main/17AFC.c'),
    @('fdb0,fdb0b', 'func_8002FDB0', 'src/main/17AFC.c'),
    @('sc_c', 'func_8002304C', 'src/main/9F9C.c'),
    @('ws', 'func_80029454', 'src/main/17AFC.c'),
    @('rnd_sub', 'func_800342A0', 'src/main/17AFC.c')
)
foreach ($r in $rows) {
    $opt, $fn, $file = $r
    $tag = $opt -replace ',', '_'
    $out = "tmp/w2/debt5/$tag"
    wsl bash -lc "cd '$wsl' && python3 memory/grind/phase2-2026-10-03/lt/fzz/w2b5.py opt=$opt out=$out >/dev/null && python3 tmp/w2/extract.py $out/$file $fn $out.$fn.c" | Out-Null
    $s = & tools/wteng.ps1 $Root sandbox $fn --disable all --candidate "$out.$fn.c" 2>&1 | Select-String '"score"'
    "$opt ($fn): $s"
}
