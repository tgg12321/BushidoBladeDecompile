# FAKE ablations for w2b2: each variant is the generator's output with one option (tmp/w2/abl2/<opt>/),
# the function body scored with `engine sandbox <func> --disable all --candidate` against the applied tree.
param([string]$Root = (Resolve-Path "$PSScriptRoot/../../../../..").Path)
Set-Location $Root
$rows = @(
    @('fill', 'func_800392C8', 'src/main/28708.c'),      # HEAD's constant holder, kept (removed in the landed form)
    @('a_fill_lit', 'func_80039320', 'src/main/28708.c'),    # the fill holder's literal
    @('a_fill_newval', 'func_80039320', 'src/main/28708.c'), # HEAD's newval doubling as the holder
    @('a_sent_lit', 'func_800395B4', 'src/main/28708.c'),    # the sentinel holder's literal
    @('mask_u32', 'func_800395B4', 'src/main/28708.c'),      # the (u32) on the bound compares
    @('a_sp120', 'func_8003993C', 'src/main/28708.c'),   # existing frame-layout FAKE
    @('a_next', 'func_8003993C', 'src/main/28708.c'),    # existing named-intermediate FAKE
    @('rec_local', 'func_8003993C', 'src/main/28708.c')  # the comment's claim: a frame-record local
)
foreach ($r in $rows) {
    $opt, $fn, $file = $r
    $out = "tmp/w2/abl2/$opt"
    wsl bash -lc "cd '$((Get-Location).Path -replace '\\','/' -replace '^C:','/mnt/c')' && python3 memory/grind/phase2-2026-10-03/lt/f09/w2b2.py opt=$opt out=$out" | Out-Null
    $src = Get-Content -Raw "$out/$file"
    $m = [regex]::Match($src, "\n[A-Za-z][^\n;]*\b$fn\([^;{]*\)\s*\{")
    $i = $m.Index + 1
    $j = $src.IndexOf("`n}`n", $i) + 3
    Set-Content -NoNewline -Path "$out.$fn.c" -Value $src.Substring($i, $j - $i)
    $s = & tools/wteng.ps1 $Root sandbox $fn --disable all --candidate "$out.$fn.c" 2>&1 | Select-String '"score"|_insns'
    "$opt ($fn): $($s -join ' ')"
}
