# In-tree sandbox receipts for func_80055138 (run while holding the landing lock, model applied).
$Root = 'C:\Users\Trenton\desktop\Bushido Blade 2 Decompile'
$V = 'memory/grind/func_80055138/r11/v'
$names = @('cand','pv','pv_fs','s_u8','s_decl_init','s_tests','abl_lvl5','abl_lvl3','abl_row_idx','abl_move_mask','abl_stat1','abl_stat2','part_case_loop','part_mask_rest','part_casemask_stats','ctr_split','nostage_fs','nostage_blk','nostage_inline')
foreach ($n in $names) {
  $out = & (Join-Path $Root 'tools\wteng.ps1') main sandbox func_80055138 --disable all --candidate "$V/$n.c" 2>&1 | Out-String
  $m = [regex]::Match($out, '"score":\s*(\d+)[\s\S]*?"target_insns":\s*(\d+)[\s\S]*?"build_insns":\s*(\d+)')
  if ($m.Success) { "{0} score={1} target={2} build={3}" -f $n, $m.Groups[1].Value, $m.Groups[2].Value, $m.Groups[3].Value } else { "$n PARSE-FAIL: $($out.Substring([Math]::Max(0,$out.Length-400)))" }
}
