/* s8 REJECTED (exactly inert, 11 at 135): declaring the LoadImage rect as a real
 * `struct { s16 x, y, w, h; }` and passing `&rect` (or `&rect.x`) instead of the
 * `s16 rect[4]` array. Both spellings are byte-identical to the array form, so the
 * PsyQ-idiomatic RECT rederive of this function is a no-op on the residual. */
    BB2Rect rect; ... LoadImage((s32)&rect, (s32)((u8 *)&D_800A9A24 + off));
