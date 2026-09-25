# medium_revisit — examined, no change (2026-09-25, sweep 3)

Rows from `targets.csv` that end as KEEP after the revisit (the first-sweep MEDIUM idea is dropped; reasons in rejected.md):

- 0x80078824 disp_init_video_overlay — ClearOTagR + STAFF.BIN load + 2x LoadImage to VRAM x=640 + DRAWENV bg; "video" (VRAM) and "overlay" (an overlaid image) both survive a plain reading; not contradicted.
- 0x800274BC calc_NormVector — output is -4096*v/|v| (my emulation: 5000 vectors, +-38, dot(out,v) < 0 always); the input is an object's velocity (+0x44..0x4C, code6cac_b.c:3880) and a collision normal pointing against the motion is a normal reading of "NormVector"; not contradicted. A computation name would need the owner's ruling on approximate normalisation (same hold as gte_NormalizeIR).
- 0x80077A80 replay_camera_apply_transform_and_flip — full-depth reach DOES flip: 80077A80 -> 800770B8 -> 8006E950 -> game_FrameLoop -> func_800174F4 -> PutDrawEnv / DrawOTag [VERIFIED]; "flip" is not contradicted, so the name is not concretely false as a whole.
- 0x80016C74 file_ResetDmaFlag — the flag guards eff_Init [VERIFIED in-binary-string], which reads a disc file (cdrom_StartRead) and LoadImage's it to VRAM [VERIFIED] (a GPU DMA transfer); "file" and "Dma" are both supported in reach.
