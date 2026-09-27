param([Parameter(ValueFromRemainingArguments=$true)][string[]]$V)
foreach ($v in $V) {
  $out = "tmp/func_80027AD8/r_$v.txt"
  pwsh tmp/orch/sbx.ps1 func_80027AD8 "tmp/func_80027AD8/$v.c" *> $out
  $s = (Select-String -Path $out -Pattern '"score": (\d+)' | Select-Object -First 1)
  if ($s) { "$v " + $s.Matches[0].Groups[1].Value } else { "$v ERROR" }
}
