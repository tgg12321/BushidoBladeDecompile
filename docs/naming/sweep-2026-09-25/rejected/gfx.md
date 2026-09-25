# gfx vein: candidate ideas examined and dropped

- 0x80042478 disp_set_fade_color: considered RESET/RENAME to a DRAWENV-bg + SetFarColor restatement. Dropped. SetFarColor [VERIFIED] sets the GTE depth-cue far colour, which is the "fade" target, so the name is not contradicted. A restatement would have to encode the optional-gray branch, and that branch is gated on a flag from the INFERRED file_GetFlag0.
- 0x8006920C efc_ptr_array_offset_apply: considered UPGRADE (the mechanics are exact: relocate every non-(-1) entry of a 0-terminated u32 array by a0). Dropped because the "efc_" subsystem prefix is unproven. A RENAME to a mechanical name (e.g. mem_RelocPtrArray) would be admissible computation-restatement but low value. Not proposed.
- 0x800484A0 gpu_decode_load_image: considered RENAME to a TIM-CLUT upload name. Dropped because the grayscale branch depends on func_800486FC (wraps INFERRED file_GetFlag0) and "load_image" is literally true.
- 0x800455AC efc_particle_queue_entry: considered RESET of "particle". Dropped. The body is a keyed list append of {id, buffer cursor} (the D_800A33A0 cursor is bounded by 0x44FFF in func_80045230), and nothing in it contradicts "particle" outright, even though the sibling NDATA loaders suggest a resource cache.
- 0x80078628 disp_helper: a getter (return a0->+4). A computation name would be trivial. Kept.
- 0x8004954C efc_helper: closed form sum_{i<n}(a0-i)+a2-n. A computation name would be meaningless. Kept.
- 0x80016888 gpu_InitDisplay: considered UPGRADE (every call is VERIFIED). Dropped because "InitDisplay" summarises rather than restates (the display is actually turned OFF: SetDispMask(0)). A restating name would be long. 2026-09-24 also left it consistent.
- 0x80069AE4/… draw_anim_* / draw_anim_sprite*: checked the primitive family. The shared batch renderer func_8007352C calls SetSprt (SPRT) and all of them AddPrim. "sprite"/"draw" are consistent, and "anim_obj" is game-semantic. No contradiction.
- 0x800646E8 gpu_render: flagged by the mechanical pass (SetPolyFT4 but no AddPrim call). Checked: it links into the OT inline (g_gpu_ot_ptr + 0x00FFFFFF/0xFF000000 masks at 0x80064DE0). Not contradicted.
- 0x80072BC4 / 0x80072CD4 gpu_build_quad / gpu_build_gradient_quad: both are SetPolyG4. Checked that neither claims a different primitive family. KEEP.
- 0x80066EC0..0x800671CC efc_spawn (16 wrappers): the callee's "draw" is contradicted (see the 0x80067200 RESET), but "spawn" at the wrapper level is game-semantic, and the wrappers do start the record fill. Not reset.
- 0x80078824 disp_init_video_overlay: kept as MEDIUM RESET only. The contradiction of "video" depends on reading it as FMV.
- 0x8004881C gnd_helper → math_RgbToGray: kept as MEDIUM. The formula is proven, but callers disagree on which argument is R vs B.
