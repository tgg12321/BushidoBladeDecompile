"""wave69 rows: the unnamed functions of 2B344.c that Codex researched (tmp/codex/w69-research.md:
14 RENAME leads, 33 NO_NAME), re-derived from the code at main f00e4da2b. Codex names were leads only.
No kept address is another tmp/p3w* draft's. 4 RENAME. No new prefix (fight_ and menu_ are admitted).

Pre-apply review pending (VERDICT / VERIFIER_NOTE empty).

Not named (dropped Codex leads):
- 0x8003DDF8 (gpu_SetLastOrderingTableLink): wave50b round 1 already dropped gpu_SetOtTailLink for
  this function. It writes D_800A378C[0xFFF] (2B344.c:1497-1501) and nothing shows that slot is the
  last of the table or the chain tail (its caller at 2B344.c:1490-1495 otherwise uses addPrim at
  [0xFFB]); "Last" is unproved.
- grid_ (a new prefix) for 0x8003E2D8, 0x8003E6A0, 0x8003F388, 0x8003F52C, 0x8003F54C, 0x8003F274,
  0x8003EDC0, 0x8003F1C8, 0x8003F1D4: wave50b and wave67 already rejected the cellgrid_ prefix for
  this D_800A8FB0 family (the same 32 x 32 x 2000-unit grid is walked by the collision code, so the
  prefix does not single out one subsystem), and the meaning of the cell bits is still not
  established; no strong argument for a new prefix was found. Specific doubts: 0x8003E2D8 /
  0x8003E6A0 / 0x8003F388 would be named by what they set, 0x8003F52C overwrites with mask 3,
  0x8003F54C and 0x8003F1C8 have no caller, 0x8003F1D4 returns the node-record table (wave67
  node_ territory).
- 0x8003C958 / 0x8003C9A4 as menu_InitPostFightChoice / menu_UpdatePostFightChoice: the menu_ part
  is borne out (own pad code), "PostFight" is not: the state is entered from three places
  (2B344.c:733, :806, :1109) after other presentation handlers of unresolved identity, so "after a
  fight" is an inference. Renamed to the neutral Choice form below.
- The 33 NO_NAME rows stay neutral (not re-mined).
"""

VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply (round 1, 2/4): fight_InitCommandStream and fight_RunCommandStream CONFIRM; menu_InitChoice / menu_UpdateChoice FAIL (Choice is generic: 9F9C.c func_8001EA04/func_8001EA84 is a twin three-item choice on D_800A3817/D_800A3929, and menu_UpdateChoiceGrid exists) -- left neutral; re-mine both pairs together later"

FUNCS = [
    ('0x8003B20C', 'fight_InitCommandStream', 'RENAME',
     'BODY 2B344.c:219-234 (s32 arg0): clears the stream state (D_800A3894, D_800A385C, the save '
     'sentinels D_800A3836 / D_800A3915, D_800A37A0 / D_800A37A4), selects the byte stream '
     'D_800900EC[arg0] into the cursor D_800A3844, runs eff_Init, the initial image scan '
     '(func_8003AE5C(D_800A3844)), func_8003AF40(0) and derives D_800A376C from '
     'D_8008D538[fighter]. Only caller: 24BF0.c:49 (func_800344B4 case 0, the fight setup), '
     'immediately followed by func_8003B5A4() at :51, which consumes that cursor. WHY: prepares '
     'the command stream that func_8003B5A4 runs (its commands configure both fighters and '
     'change fight states, 2B344.c:320-418). fight_ is the two-fighter session: the caller sets '
     'it up together with fighter_ClearCpuTally (24BF0.c:52) and the stream commands load both '
     'fighters. COUNTER-EVIDENCE: it also runs eff_Init and sets up one fighter, so Init means '
     'the whole start state, not only the cursor; the stream also schedules non-bout states '
     '(playback, movie, 2B344.c:395-415), which is why no mode word (story, practice) is put in the '
     'name. FRESH: fight_InitCommandStream absent from src/, include/, named_syms.txt, '
     'symbol_addrs.txt and every tmp/p3w* draft. CENSUS NAME (draft time): func_8003B20C (AUTO, '
     'splat-auto).'),
    ('0x8003B5A4', 'fight_RunCommandStream', 'RENAME',
     'BODY 2B344.c:320-418: snd_ResetVoicesAndQueue, then loops reading one byte command from the '
     'cursor D_800A3844 and advancing it: fighter configuration commands (func_8003B3A4 / '
     'func_8003B484 / func_8003B4DC with the operands), the image-record command, descriptor '
     'installs (D_800A3878 / D_800A385C via func_8003B56C), state changes D_800A3834 = n; the loop '
     'ends when a command sets var_0 (a command that yields to a state). Callers: 24BF0.c:51 '
     '(right after func_8003B20C), 28708.c:1507 and 9F9C.c:1302 (a bout or replay ended, resume '
     'the stream), 2B344.c:443 (func_8003B8E4) and :591 (func_8003BCB4), the last two from the '
     'state handlers that continue the stream after a load or a playback. WHY: runs the stream '
     'until a command hands control to a state. Every caller resumes the same cursor. '
     'COUNTER-EVIDENCE: not the replay-ring interpreter (replay_ is a different thing, 120 frames); '
     'Run does not claim it consumes the whole stream in one call; the stream has non-fight '
     'commands (movie, playback). FRESH: fight_RunCommandStream absent from src/, include/, '
     'named_syms.txt, symbol_addrs.txt and every tmp/p3w* draft. CENSUS NAME (draft time): '
     'func_8003B5A4 (AUTO, splat-auto).'),
]

DATA = []
