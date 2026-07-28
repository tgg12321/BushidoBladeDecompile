/* REJECTED (s1, measured): two HImode spellings hoping to trigger phantom
 * frame slots (phantom-frame-slots-gcc272). Both keep vars=0 — no frame effect:
 *   Probe A: pass s16 loop locals to efc_buki_draw_zanzou WITHOUT (s32) casts
 *            (a1v, a2v, a3v, (s16)v0v)          -> vars= 0
 *   Probe B: v0v declared s16 (not u16), load via (s16)(*(u16*)p), pass (s32)v0v
 *            (HImode increment + compare)        -> vars= 0
 * Conclusion: naked-s16 call args and s16-vs-u16 loop locals do NOT allocate
 * stack temps in cc1 2.7.2 here. The tslLineG5Init trigger is specifically an
 * HImode BITWISE expression; plain HImode add/compare/convert paths don't
 * hit assign_stack_temp. Probe sources: tmp/grind/func_800481E8/s1/probe_A.c, probe_B.c. */
