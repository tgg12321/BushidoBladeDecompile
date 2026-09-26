# In-tree sandbox of the do-while(0) escape rows (lock held, model applied).
$Root = 'C:\Users\Trenton\desktop\Bushido Blade 2 Decompile'
$list = @('v/ce_l_dw_between','v/ce_l_dw_wrap','v/ce_l_dw_wrapuse','v/ce_m_dw_between','v/ce_m_dw_wrap','x/abl_stat2_dw','x/abl_stat1_dw','x/pv_dw_stats')
foreach ($n in $list) {
  $out = & (Join-Path $Root 'tools\wteng.ps1') main sandbox func_80055138 --disable all --candidate "memory/grind/func_80055138/r11/$n.c" 2>&1 | Out-String
  $m = [regex]::Match($out, '"score":\s*(\d+)[\s\S]*?"target_insns":\s*(\d+)[\s\S]*?"build_insns":\s*(\d+)')
  if ($m.Success) { "{0} score={1} target={2} build={3}" -f $n, $m.Groups[1].Value, $m.Groups[2].Value, $m.Groups[3].Value } else { "$n PARSE-FAIL" }
}
