param([string]$Cand = "memory/grind/func_800198D0/candidate.c", [switch]$Regs, [int]$Head = 120)
$out = & tools/wteng.ps1 main sandbox func_800198D0 --disable all --candidate $Cand 2>&1 | Out-String
$m = [regex]::Match($out, '"score":\s*(\d+)[\s\S]*?"build_insns":\s*(\d+)')
if ($m.Success) { "SCORE $($m.Groups[1].Value) build_insns $($m.Groups[2].Value)" } else { $out }
$flag = if ($Regs) { "--regs" } else { "" }
bash tools/wsl.sh "python3 tmp/func_800198D0/sdiff.py $flag" | Select-Object -First $Head
