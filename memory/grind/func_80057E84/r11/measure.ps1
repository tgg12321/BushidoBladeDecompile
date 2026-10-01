# measure.ps1 <dir> <outfile>: sandbox-score every .c in <dir> as a func_80057E84 body (engine
# sandbox --disable all --diff --candidate, via tmp/orch/sbx.ps1, with the 0x360/0x361 header
# fields staged by prep_hdr.py) and record score / insns / hunk summary.
param([string]$Dir, [string]$Out)
$Root = 'C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile'
Set-Location $Root
bash tools/wsl.sh 'python3 memory/grind/func_80057E84/r11/prep_hdr.py' | Out-Null
"" | Set-Content $Out
foreach ($f in Get-ChildItem "$Dir/*.c" | Sort-Object Name) {
  $rel = "$Dir/$($f.Name)"
  $o = pwsh tmp/orch/sbx.ps1 func_80057E84 $rel 2>&1 | Out-String
  $sc = if ($o -match '"score": (\w+)') { $Matches[1] } else { 'ERR' }
  $bi = if ($o -match '"build_insns": (\w+)') { $Matches[1] } else { '?' }
  $hk = if ($o -match '(\d+) source-level · (\d+) operand-only[^·]*· (\d+) not-scored') { "src=$($Matches[1]) op=$($Matches[2]) masked=$($Matches[3])" } else { '' }
  "$($f.Name) | score $sc | insns $bi/447 | $hk" | Add-Content $Out
}
"DONE" | Add-Content $Out
