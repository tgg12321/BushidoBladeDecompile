# Sandbox every candidate in tmp/laneH/cand against the current tree; prints func.variant score/target.
Set-Location 'C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile'
$out = 'tmp/laneH/cand_scores.txt'
"" | Set-Content $out
Get-ChildItem tmp/laneH/cand/*.c | Sort-Object Name | ForEach-Object {
  $name = $_.BaseName; $func = $name.Split('.')[0]
  $r = & tools/wteng.ps1 main sandbox $func --disable all --candidate "tmp/laneH/cand/$($_.Name)" 2>&1 | Out-String
  $s = [regex]::Match($r, '"score": (\d+)').Groups[1].Value
  $ti = [regex]::Match($r, '"target_insns": (\d+)').Groups[1].Value
  $bi = [regex]::Match($r, '"build_insns": (\d+)').Groups[1].Value
  $line = "$name score=$s target=$ti build=$bi"
  if (-not $s) { $line += "  ERROR: " + ($r -split "`n" | Select-Object -Last 3) -join ' ' }
  $line | Add-Content $out
}
Get-Content $out
