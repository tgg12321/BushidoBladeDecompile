/* s3 P3 — KILLED, score 17 (full register rotation).
 * On top of the s2 candidate: arg1 also carries the sp20[1] repack
 * (arg1 = sp20[1]; sp10[1] = (s16)arg1;) to unboost v1lw symmetrically.
 * Result: arg1->$a3, val->$v1, post-call gp reload->$a0, sh order flips
 * (0x10,0x14,0x12). The extra refs/segments on arg1 shift the global
 * conflict-walk order (H11 family). The v1lw/a0lw "symmetric unboost"
 * route is dead: any second set on the sp20[1] temp is either arg1
 * (rotation, this) or 91/D+2 (F/H9 const-fold kill).
 */
void func_80061C00_p3_shape(void);
