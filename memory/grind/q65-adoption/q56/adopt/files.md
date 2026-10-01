# Files touched per step (base 0107288ac, 2026-10-01)

- **01** substrate: maspsx _uses_gp - an indexed sym($reg) operand is never gp (owner ruling Q65, s
  engine/test_engine.py tools/maspsx/maspsx/__init__.py tools/maspsx/tests/test_indexed_gp_nop.py 
- **02** src: code6cac_b_tu2 / code6cac_b_tu3 boundary moves to func_800343F0 (owner ruling Q65, st
  docs/grind/rodata-align-2026-09-30.md src/code6cac_b_tu2.c src/code6cac_b_tu3.c 
- **03** src: text1b / text1b_tu1c boundary moves to func_80060A68 (owner ruling Q65, step 3, bound
  docs/grind/rodata-align-2026-09-30.md src/text1b.c src/text1b_tu1c.c 
- **04** src: text1a_c split before func_80044800 into text1a_c_tu2 (owner ruling Q65, step 4)
  bb2.ld src/text1a_c.c src/text1a_c_tu2.c 
- **05** src: code6cac_b2_pre + replay_camera_rob_back_loose2 + code6cac_b2_post are one file (owne
  bb2.ld src/code6cac_b2_post.c src/code6cac_b2_pre.c src/replay_camera_rob_back_loose2.c 
- **06** src: code6cac_c2 / config declarations of func_8004153C and D_800A3708 reconciled (owner r
  src/code6cac_c2.c src/config.c 
- **07** src: code6cac_c2 + config are one file (owner ruling Q65, step 7)
  bb2.ld src/code6cac_c2.c src/config.c 
- **08** src: text1a_c2 / text1a_b / text1a_b_pre_rodata / sound / text1b declarations reconciled b
  src/sound.c src/text1a_b.c src/text1a_b_pre_rodata.c src/text1a_c2.c src/text1b.c 
- **09** src: text1a_c2 + text1a_b + text1a_b_pre_rodata + sound + text1b are one file (owner rulin
  bb2.ld docs/grind/rodata-align-2026-09-30.md src/sound.c src/text1a_b.c src/text1a_b_pre_rodata.c src/text1a_c2.c src/text1b.c 
- **10** src: text1b_tu2 / text1b_b (and the text1b_tu1d definition) declarations reconciled by evi
  src/text1b_b.c src/text1b_tu1d.c 
- **11** src: text1b_tu2 + text1b_b are one file (owner rulings Q65/Q67, step 11)
  bb2.ld docs/grind/rodata-align-2026-09-30.md src/text1b_b.c src/text1b_tu2.c 
- **12** substrate: maspsx models .local+.comm (an uninitialized static) as .lcomm (owner ruling Q6
  engine/test_engine.py tools/maspsx/maspsx/__init__.py tools/maspsx/tests/test_comm_global.py tools/maspsx/tests/test_static_lcomm.py 
- **13** substrate: maspsx models cc1psx -G8's .sdata choice for small initialized objects (owner r
  docs/grind/gp-model-2026-09-30.md engine/test_engine.py tools/maspsx/maspsx/__init__.py tools/maspsx/tests/test_small_data_sdata.py 
- **14** src: one declaration each for D_800A3468 (text1b_tu1c) and g_anim_hit_flags (text1a_post) 
  asm/funcs/camera_CalcAngles.s engine/queue.json named_syms.txt src/text1a_post.c src/text1b.c src/text1b_tu1c.c src/text1b_tu1d.c undefined_syms_auto.txt 
- **15** src/build: per-file gp model - definitions follow the evidence, data blob cut, maspsx -G8 
  Makefile asm/data/91C98.data.s asm/data/938EA.data.s asm/data/938FE.data.s asm/data/93950.data.s asm/data/93978.data.s asm/data/939DC.data.s asm/data/939E6.data.s asm/data/93A0E.data.s asm/data/93A3E.data.s asm/data/93AC0.data.s asm/data/93AEA.data.s asm/data/93B08.data.s asm/data/93B12.data.s asm/data/93B56.data.s asm/data/93B7C.data.s asm/data/93B8C.data.s asm/data/93D30.data.s asm/data/93DCC.data.s asm/data/93E18.data.s bb2.ld docs/grind/gp-model-2026-09-30.md engine/buildconfig.py engine/buildstamp.py engine/oracle.py engine/pipeline.py engine/queue.py engine/test_engine.py include/game.h include/sound.h named_syms.txt sdata_exclude.txt sdata_funcs.txt sdata_syms.txt src/code6cac.c src/code6cac_b2_post.c src/code6cac_b3.c src/code6cac_b4.c src/code6cac_b4_post.c src/code6cac_b5.c src/code6cac_b_tu2.c src/code6cac_c.c src/code6cac_c0.c src/code6cac_c2.c src/code6cac_c_ab.c src/code6cac_c_mid.c src/code6cac_tu2.c src/ings.c src/text1a_c.c src/text1a_c_tu2.c src/text1a_post.c src/text1a_pre.c src/text1b.c src/text1b_b.c src/text1b_tu1c.c src/text1b_tu1d.c src/text1b_tu1e.c tools/psyq_library_files.py undefined_syms_auto.txt 
- **16** tools/records: follow the retired sdata lists and the moved functions (owner ruling Q65, s
  tools/check_root_cleanliness.py tools/data_wave.py tools/desync_audit.py tools/grinder/grindlib.py tools/mar_perm_compile.sh tools/mar_perm_workspace.sh tools/naming_wave.py tools/ra_solver/mkasm_honest.sh tools/sched_solver/mkasm.sh 
