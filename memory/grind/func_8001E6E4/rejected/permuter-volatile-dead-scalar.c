/* REJECTED — permuter score-0 find (s4, campaign s4-honest-random, iter 461),
 * rejected per policy WITHOUT submission. Cheat family: unused-local frame
 * coercion via volatile-typed scalar (explicitly enumerated in the forbidden
 * catalog: "unused-local-array frame coercion ... with volatile-typed scalar";
 * naming 'pad' announces intent, fails tests 1/2/3/5/6).
 *
 * Mechanism (measured): an UNWRITTEN `volatile unsigned short pad;` is the
 * ONLY local-declaration spelling that reaches vars=80 / frame 112 with ZERO
 * byte cost — GCC 2.7.2 gives a volatile local an unconditional stack home
 * even when nothing reads or writes it, while emitting no instruction.
 * Non-volatile dead scalars/pointers (CamWork *new_var; s32 *new_var;) do NOT
 * move the frame (vars=72 measured) — GCC 2.7.2 allocates no home for unused
 * non-volatile scalars (contrast: unused ARRAYS do get frame bytes, s1).
 * A WRITTEN volatile short (`s2[new_var3 = 0]`) reaches vars=80 but
 * materializes the volatile store (permuter score 100, not 0).
 *
 * Provenance note: the permuter ships a dedicated mutation for this —
 * perm_pad_var_decl ("Inserts an unused variable to adjust stack offsets.
 * Probably only useful with --stack-diffs enabled"), i.e. a cheat generator
 * under this project's policy. It was weight-zeroed for all subsequent
 * campaigns this session; dead decls still appeared via temp_for_expr temps
 * orphaned by later mutations, always volatile-typed when frame-moving.
 */
void func_8001E6E4(s32 arg0)
{
  volatile unsigned short pad;   /* <-- the cheat: unwritten, unread */
  CamWork local;
  s32 *s2;
  s2 = (s32 *) (&D_800F5328);
  if (((u32) (arg0 - 0x555)) >= 0x556U)
  {
    s2 = (s32 *) (&D_800F6608);
  }
  local.vx = s2[0] + D_800FF5C8;
  local.vy = s2[1] + D_800FF5CC;
  local.vz = s2[2] + D_800FF5D0;
  local.rx = (*((u16 *) (((u8 *) s2) + 0x10))) + ((u16) D_800FF5D8);
  local.ry = (*((u16 *) (((u8 *) s2) + 0x12))) + ((u16) D_800FF5DA);
  local.rz = (*((u16 *) (((u8 *) s2) + 0x14))) + ((u16) D_800FF5DC);
  local.dist = (*((s32 *) (((u8 *) s2) + 0x18))) + D_800FF5E0;
  func_80046BF4((s32 *) (&local), &local.rx, local.dist);
  {
    s32 *p20 = (s32 *) (((u8 *) s2) + 0x20);
    func_8001A538((s32 *) (&local), p20);
    func_80061064((s32 *) (&local.rx), p20);
  }
  D_800A36B4 = (s32) s2;
}
