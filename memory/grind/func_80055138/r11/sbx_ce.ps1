# Sandbox + dump each sanctioned-family escape variant (v/ce_*.c); lock held, model applied.
$Root = 'C:\Users\Trenton\desktop\Bushido Blade 2 Decompile'
$V = 'tmp/func_80055138/r11/v'
$names = Get-ChildItem (Join-Path $Root "tmp\func_80055138\r11\v") -Filter 'ce_*.c' | ForEach-Object { $_.BaseName } | Sort-Object
foreach ($n in $names) {
  $out = & (Join-Path $Root 'tools\wteng.ps1') main sandbox func_80055138 --disable all --candidate "$V/$n.c" 2>&1 | Out-String
  $m = [regex]::Match($out, '"score":\s*(\d+)[\s\S]*?"target_insns":\s*(\d+)[\s\S]*?"build_insns":\s*(\d+)')
  if ($m.Success) { "{0} score={1} target={2} build={3}" -f $n, $m.Groups[1].Value, $m.Groups[2].Value, $m.Groups[3].Value } else { "$n PARSE-FAIL: $($out.Substring([Math]::Max(0,$out.Length-300)))" }
  & wsl.exe -e bash '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_80055138/r11/dump_sbx.sh' $n 2>&1 | Out-Null
  if (Test-Path (Join-Path $Root "tmp/func_80055138/r11/rtl/$n.lreg")) { "  dumped $n" } else { "  DUMP FAILED $n" }
11
tl\$n.lreg")) { "  dumped $n" } else { "  DUMP FAILED $n" }
}
