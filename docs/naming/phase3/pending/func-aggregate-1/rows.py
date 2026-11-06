"""wave124 rows (func-aggregate-1, Q134 revival): func_80016D78 (src/main/6CF8.c:283-308), func_8001C51C (src/main/9F9C.c:1251-1272),
func_80040D48 (src/main/31548.c:10-189), func_80046B44 (src/main/368E4.c:429-448).
Research input tmp/codex/func-aggregate-1-research.md (untrusted; every claim re-derived at HEAD 9531ed19e, see notes.txt).
Pre-apply (naming-b, 2026-11-05): 0x8001C51C PASS; 0x80016D78 and 0x80040D48 FAIL, replaced by the reviewer's suggested names (marked suggested, judged fresh at R6); 0x80046B44 FAIL, dropped (stays NO_NAME). (DATA is empty.)
"""
VERDICT = "CONFIRM"
VERIFIER_NOTE = "naming-reviewer pre-apply PASS (round 1, 2026-11-05, naming-b)"
NOTES = {
    "0x80016D78": "naming-reviewer pre-apply FAIL of sys_InitGameAtBoot ('Game' unsupported: snd_InitAndLoadCommonVab -> snd_Init runs SsInit / SsStart, device init, so 'game state vs devices' does not separate it from sys_Init); SUGGESTED name sys_InitAtBoot from that reviewer, marked suggested: R6 judges it fresh at every use",
    "0x80040D48": "naming-reviewer pre-apply FAIL of slot_FinishObjectFrame ('Finish' false: callers keep queueing the same slot's nodes and draw moves after it, 28708.c:1447-1460, 4510C.c:175-188; check against slot_BlendTwoPosesIntoNodeMatrices); SUGGESTED name slot_UpdateObjectFrame from that reviewer, marked suggested: R6 judges it fresh at every use",
}
DROPPED = {
    "0x80046B44": "sys_InitSceneState FAIL at pre-apply (N4/R4 Q134): 'Scene' does not cover ndata_StartReadInf (NDATA.INF read), arena_Reset, math_InitRotMatrixFuncs or reloc_ClearCounts, and the name states no occasion; sys_ReinitAfterMovie is false for the boot caller (6CF8.c:301). Stays NO_NAME (w124).",
}

FUNCS = [
    ("0x80016D78", "sys_InitAtBoot", "RENAME",
     "BODY: src/main/6CF8.c:283-308: void func_80016D78(void): printf(g_str_limit, 0x8010DB00) (context only, N3); "
     "InitSavedRecordAndResetCameraDistanceOffset; motion_InitBuffers; D_800A3770[0]/[1] = 0x801D8800/0x801EBC00 (the frame-parity "
     "primitive buffers) and D_800A3798 = 0x13400; g_eff_initialized = g_snd_initialized = 0 then snd_InitAndLoadCommonVab; "
     "pad_ResetStateMarkValid; font_LoadImage; fight_InitSettings; D_800A36F9, g_fight_view_option, D_800A3744-46 = 0; "
     "func_80046B44; g_main_frame_period = 2; g_motion_fighter_loaded_data_ids[1] = 0; D_800A36B0 = 0; g_comb_abort_request = 0. "
     "CALLERS: exactly one, main (6CF8.c:441, jal asm/funcs/main.s:17), at power-on: main runs sys_Init (:440), this (:441), "
     "SetDispMask(1) (:442), the boot image fade (:443-444), then sets g_main_loop_handler_index = 0xF (:446) before the frame loop. "
     "main is not re-entered, so this runs once per power-on. include/bb2.h:389 and symbol_addrs.txt / named_syms.txt mention it in "
     "comments only. The movie return path does not call it: sys_Exec (cdrom.c:217-238) redoes only sys_Init (:235) and "
     "snd_InitAndLoadCommonVab (:236), and ExecMovieOverlayAndReinitialize (2B344.c:682-693) redoes only func_80046B44. "
     "WHY: the one occasion is the game's first start. Every part sets a first-time default that the frame loop and later modes "
     "rely on and that must not be redone after a movie (saved record, fight settings, option bytes, g_main_frame_period, "
     "comb-abort flag) or prepares boot-time resources the loop reads at once (prim buffers D_800A3770, read by main at "
     "6CF8.c:455; g_comb_abort_request, read at :460; font image; common VAB). 'Game' is the noun for those defaults (saved "
     "record, fight settings, option bytes), 'AtBoot' is the occasion main's single call shows; the prefix sys_ is main's boot "
     "sibling sys_Init. "
     "COUNTER-EVIDENCE: (1) sibling sys_Init (6CF8.c:182-191) = ResetCallback, InitPAD/StartPAD, disp_Init, cdrom_Init, "
     "memcard_Init, rcnt: Sony device init, runs at boot AND after every movie (cdrom.c:235), so 'GameAtBoot' separates the two "
     "(sys_Init has two occasions and touches devices; this has one and touches game state); (2) the callees "
     "snd_InitAndLoadCommonVab and func_80046B44 also run after movies, but only as parts here: the body as a whole is "
     "boot-only; (3) 'Game' is a summary noun: the mildest claim is sys_InitAtBoot (occasion only), flagged for the reviewer as "
     "the fallback if 'Game' reads as a guess; (4) not a list of jobs: no chained verbs in the name; (5) not a Q132 pure "
     "forwarder: it stores and calls many things. Census docs/naming/function-names.csv:24 (splat-auto, KEEP); no alias. "
     "PRE-APPLY: naming-reviewer pre-apply FAIL of sys_InitGameAtBoot ('Game' unsupported: snd_InitAndLoadCommonVab -> snd_Init runs SsInit / SsStart, device init, so 'game state vs devices' does not separate it from sys_Init); SUGGESTED name sys_InitAtBoot from that reviewer, marked suggested: R6 judges it fresh at every use. The name now claims only the occasion (main's single power-on call; sys_Init also runs after every movie, cdrom.c:235, so AtBoot still separates them); the 'Game' wording above is superseded."),
    ("0x8001C51C", "fight_StartNextSecondFighter", "RENAME",
     "BODY: src/main/9F9C.c:1251-1272: void func_8001C51C(void): fight_StartFighter0DefaultVariantScriptIfMoveKind0x32Or0x11; "
     "fighter_Init(1, unk_4[5], unk_4[1], unk_4[3], last arg 0 only when g_fight_mode_selector == 3 && "
     "g_fight_start_position_pair_changed, else 1); fighter_ConfigureCpuControl; motion_ResetVariantIndex(1); fighter 1 "
     "default variant script started (fighter_StartMoveScript(1, ..)); g_fight_defeat_delay_frames = 0; "
     "g_fight_followup_fighter_index = -1; object_FreeAttachedObjects; object_ClearFixedKindRange; "
     "camera_InitTwoFighterView(&D_80101EC8[0], &D_80101EC8[1]); replay_Reset; "
     "fighter_AssignMotionSlotAndScanDataIdNibbles(1). "
     "CALLERS: two, both in fight_UpdateDefeatDelayAndModeProgress (9F9C.c:1355; jal asm/funcs/fight_UpdateDefeatDelayAndModeProgress.s:115 "
     "and :209), both past the defeat-delay gate (g_fight_defeat_delay_frames >= 0x3D, :1364; the delay counts only while a "
     "fighter is_defeated, :1359-1360) and both inside a branch that first requires fighter 0 NOT defeated "
     "(is_defeated != 0 -> break, :1368 and :1404), i.e. fighter 1 was beaten and fighter 0 stands: (a) :1390 in "
     "g_fight_mode_selector 0 when g_fight_second_fighter_defeats_left is decremented and still nonzero (zero goes to the "
     "command stream / substitution playback instead, :1371-1389); (b) :1430 in mode 3 after "
     "fight_AdvanceStepTableRowOrInsertBestTimeAndSetFlags advanced the step table and slot 1 was reloaded with "
     "g_fight_step_row_object_id (:1405-1426). No other reference (git grep -w over src include asm = def, two calls, glabel). "
     "WHY: one occasion, the bout goes on against the next second fighter after the previous one was beaten. Fighter 1 is "
     "re-created and started; fighter 0 is brought out of move kind 0x32/0x11 to its default script; the defeat delay and the "
     "follow-up fighter index that drove the KO are cleared; leftover objects of the old bout are freed; the camera is "
     "re-framed on the pair; the replay is restarted; the motion slot of fighter 1 is reassigned. 'Second fighter' is the "
     "existing wording of the landed global g_fight_second_fighter_defeats_left (bb2.h:316), the counter whose nonzero "
     "decrement leads to caller (a). fight_ is both callers' prefix. "
     "COUNTER-EVIDENCE: (1) sibling fight_Init (9F9C.c:1900) runs the same callees (fighter_Init :1922, camera_InitTwoFighterView "
     ":2067, replay_Reset :2071) for BOTH fighters at round start, so 'SecondFighter' separates this one; (2) fighter 0's "
     "conditional script reset is the other side of the same transition, not a separate job; (3) in mode 0 the next second "
     "fighter may be the same character (config unk_4 is not changed by this body), so 'Next' claims the next bout's "
     "opponent, not a new character; (4) the fight_QueuePositionalSoundWithRandomAlternate call after it (:1391) is the "
     "caller's, not this body's; (5) the name chains no verbs; the earlier NO_NAME objection 'no covering verb' is what "
     "Q134 answers. Census docs/naming/function-names.csv:80 (splat-auto, KEEP); no alias."),
    ("0x80040D48", "slot_UpdateObjectFrame", "RENAME",
     "BODY: src/main/31548.c:10-189: void func_80040D48(slot_index, a1, position, rotation, arg4, arg5): returns if "
     "g_slot_objects[slot_index] == 0; writes transform_nodes[0] rotation (rotation[0..2]) and translation (position[0..2]); "
     "a1 == 0 copies decoded pose rotations (arg4) through g_node_pose_rotation_indices into the node chain, a1 == 1 zeroes "
     "node 1 rotation (cases 2-6 are empty, no caller passes them); scales node 1 y by position_scale; "
     "node_UpdateWorldMatrix over the nodes; pushes the nodes, duplicate_nodes (after copying their linked record matrix) and "
     "projected_nodes onto g_draw_queue_cursor; node_SetWordOnAllListedNodes(projected_nodes, arg5); object_ground_y = arg5; "
     "softbody_SetObjectGround(obj, arg5); softbody_UpdateObject(obj); light_SetPointLightTarget(slot_index, node 1 matrix t). "
     "CALLERS: three, each once per frame per slot, directly after that frame's pose exists: fighter_Update (9F9C.c:5476, jal "
     "asm/funcs/fighter_Update.s:2048) right after slot_BlendTwoPosesIntoNodeMatrices (:5471-5474), a1 = 1, the fighter's "
     "ground_y; replay_PlayFrame (28708.c:1446, jal asm/funcs/replay_PlayFrame.s:156) right after "
     "slot_BlendTwoPosesIntoNodeMatrices (:1437-1440), a1 = 1, replay_ground_y; motion_UpdatePlayback (4510C.c:174, jal "
     "asm/funcs/motion_UpdatePlayback.s:349) right after motion_DecodeFrame (:150-153), a1 = 0, origin_y. The callers then "
     "queue weapon nodes (28708.c:1447-1458, 4510C.c:175-183), which need the matrices this body built. Other mentions: "
     "include/bb2.h:873 decl, include/m2c_context.h decl, comments at game.h:2281, 31548.c:332 and 4510C.c:41. "
     "WHY: one occasion, the slot object's per-frame finish once its pose is known: place it (root rotation / translation), "
     "build its world matrices, queue its nodes for drawing, give it and its softbodies this frame's ground height, step "
     "the softbodies, aim the point light at it. Prefix slot_: it indexes g_slot_objects (slot_GetObject, "
     "slot_BlendTwoPosesIntoNodeMatrices, slot_LoadObject... family). "
     "COUNTER-EVIDENCE: (1) sibling slot_BlendTwoPosesIntoNodeMatrices (31548.c:193) fills the node matrices from two poses "
     "and runs first in two of the three callers (the third decodes one frame), so 'Finish' separates this one as the step "
     "after it; (2) 'Finish' means the end of this body's frame work for the slot, not that nothing follows: callers queue "
     "weapon nodes after it, and slot_UpdateObjectFrame is flagged as the weaker-claim alternative for the reviewer; "
     "(3) the light part only acts for slots 0-1 (light_SetPointLightTarget, 31D3C.c:404) and callers only pass 0-1; "
     "(4) the name chains no verbs; the earlier objection (no covering verb for the seven parts) is what Q134 answers. "
     "Census docs/naming/function-names.csv:437 (splat-auto, KEEP); no alias. "
     "PRE-APPLY: naming-reviewer pre-apply FAIL of slot_FinishObjectFrame ('Finish' false: callers keep queueing the same slot's nodes and draw moves after it, 28708.c:1447-1460, 4510C.c:175-188; check against slot_BlendTwoPosesIntoNodeMatrices); SUGGESTED name slot_UpdateObjectFrame from that reviewer, marked suggested: R6 judges it fresh at every use. Update claims the slot object's per-frame step (all three callers, once per frame per slot) and not that the frame work ends here; the 'Finish' wording above is superseded."),
]

DATA = []
