param([string[]]$Files)
foreach ($f in $Files) {
  $out = & tools/wteng.ps1 main sandbox func_80058580 --disable all --candidate $f 2>&1 | Out-String
  $m = [regex]::Match($out, '"score":\s*(-?\d+)')
  $b = [regex]::Match($out, '"build_insns":\s*(\d+)')
  "{0} score={1} insns={2}" -f (Split-Path $f -Leaf), $m.Groups[1].Value, $b.Groups[1].Value
}
