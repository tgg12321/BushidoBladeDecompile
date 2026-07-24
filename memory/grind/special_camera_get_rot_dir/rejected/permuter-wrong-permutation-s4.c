/* REJECTED — s4 permuter best (chassis-2, block-local seed), perm-score 113, NOT a match.
 * 72 insns, but the callee-saved values land in a DIFFERENT wrong permutation than target:
 *   this form: index->s3, cam->s4, const->s5, copy_end->s1, buf2->s2  (+ scrambled)
 *   target:    index->s2, cam->s3, const->s4, copy_end->s5, buf2->s1
 * Across ~46k iterations from two structurally-distinct seeds (score-10 hoisted +
 * score-9 block-local), the permuter's statement/decl-reorder + var-permutation search
 * NEVER produced the target permutation — it can only shuffle which value takes which
 * callee-saved slot, and the target ordering is unreachable because (s2 instrumented-cc1
 * proof) no C arrangement yields the allocno-priority order that assigns it.
 * The `index = 3; index = ... << index;` split is permuter cosmetic noise, not a lever.
 * This is the allocno-priority wall (marionation_Exec / cpu_side_move_dir_4 class).
 */
void special_camera_get_rot_dir(s32 *dest)
{
  u8 sp_buf[0x800];
  u8 sp_buf2[8];
  s32 index;
  s32 cam_base;
  s32 v0;
  s32 constant_80;
  Quad *copy_end;
  s32 *buf2_ptr;
  constant_80 = 0x80;
  index = func_80036EA8(6, 0) << 3;
  cam_base = (s32) (&SpecialCam);
  buf2_ptr = (s32 *) sp_buf2;
  copy_end = (Quad *) (&sp_buf[0x40]);
retry:
  func_80080258(2, index + cam_base, 0);
  v0 = ((s32 (*)()) func_800372F4)(0x800, (s32) sp_buf, constant_80);
  if (v0 != 0) goto retry;
  {
    Quad *dst_q = (Quad *) dest;
    Quad *src = (Quad *) (&sp_buf[0x10]);
    do {
      *dst_q = *src;
      src++;
      dst_q++;
    } while (src != copy_end);
    *((Triple *) dst_q) = *((Triple *) src);
  }
  v0 = cdrom_BcdToFrames(index + cam_base);
  cdrom_FramesToBcd(v0 + 1, (s32) buf2_ptr);
  func_80080258(2, (s32) buf2_ptr, 0);
  v0 = ((s32 (*)()) func_800372F4)(dest[3], dest[2], constant_80);
  if (v0 != 0) goto retry;
}
