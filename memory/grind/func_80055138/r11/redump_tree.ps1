# Re-dump cand / pv / ctr from the landed tree (lock held): sandbox each body, then dump its TU.
$Root = 'C:\Users\Trenton\desktop\Bushido Blade 2 Decompile'
foreach ($p in @(@('tcand','candidate'), @('tpv','r11/v/pv'), @('tctr','r11/v/ctr_split'))) {
  $tag = $p[0]; $body = "memory/grind/func_80055138/$($p[1]).c"
  $out = & (Join-Path $Root 'tools\wteng.ps1') main sandbox func_80055138 --disable all --candidate $body 2>&1 | Out-String
  $m = [regex]::Match($out, '"score":\s*(\d+)[\s\S]*?"build_insns":\s*(\d+)')
  "$tag score=$($m.Groups[1].Value) build=$($m.Groups[2].Value)"
  & wsl.exe -e bash '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_80055138/r11/dump_tree.sh' $tag
}
