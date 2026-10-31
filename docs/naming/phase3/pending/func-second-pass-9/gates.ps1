param([string]$Json = 'fn.json', [string]$Tag = 'fn')
$E = 'C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile\tools\wteng.ps1'
$W = 'C:\Users\Trenton\Desktop\bb2-worktrees\ln-naming-a'
$T = 'C:\Users\Trenton\Desktop\Bushido Blade 2 Decompile\tmp\p3w100'
& $E $W verify-oracle --rebuild --allow-dirty *> "$T\oracle_$Tag.out"
& $E $W tus-check *> "$T\tus_$Tag.out"
wsl bash '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/p3w100/gates.sh' p3w100 --from-manifests "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/p3w100/$Json" *> "$T\gates_$Tag.out"
& $E $W test *> "$T\test_$Tag.out"
wsl bash -c "cd /mnt/c/Users/Trenton/Desktop/bb2-worktrees/ln-naming-a && source .venv/bin/activate && python3 tools/test_label_census.py" *> "$T\tlc_$Tag.out"
"done" | Out-File "$T\done_$Tag.txt"
