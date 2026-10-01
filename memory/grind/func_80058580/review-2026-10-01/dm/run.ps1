$root = "C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile"
Set-Location $root
$out = "$root\tmp\rev58580dm\results.txt"
"" | Set-Content $out
Get-ChildItem "$root\tmp\rev58580dm\v*.c" | Sort-Object Name | ForEach-Object {
  $rel = "tmp/rev58580dm/" + $_.Name
  $j = & tools/wteng.ps1 main sandbox func_80058580 --disable all --candidate $rel 2>&1 | Out-String
  $s = ($j | Select-String '"score":\s*(-?\d+)').Matches | ForEach-Object { $_.Groups[1].Value } | Select-Object -First 1
  $b = ($j | Select-String '"build_insns":\s*(\d+)').Matches | ForEach-Object { $_.Groups[1].Value } | Select-Object -First 1
  "$($_.Name) score=$s build_insns=$b" | Add-Content $out
}
"DONE" | Add-Content $out
