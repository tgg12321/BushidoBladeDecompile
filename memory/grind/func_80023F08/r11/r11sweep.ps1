$out = @()
foreach ($f in Get-ChildItem tmp/func_80023F08/r11b/*.c | Sort-Object Name) {
  $o = (pwsh tmp/orch/sbx.ps1 func_80023F08 ("tmp/func_80023F08/r11b/" + $f.Name) 2>&1 | Out-String)
  $m = [regex]::Match($o, '"score":\s*(\d+)'); $b = [regex]::Match($o, 'ours (\d+) insns')
  $out += "$($f.BaseName): $($m.Groups[1].Value) (ours $($b.Groups[1].Value) insns)"
}
$out | Set-Content -Encoding utf8 tmp/func_80023F08/r11b/scores.txt
$out
