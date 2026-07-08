/* s3 REJECTED: V6 explicit arg4 named local for the *(s32*)t0 deref.
 * masked 11 (baseline 2) — heavy regression.
 * Mechanism: naming the t0 dereference gives it its own allocno,
 * disrupting the h5 base's t0 multi-set launch-suppression AND
 * introducing a new load-before-call scheduling seat.
 * Corroborates twin's rejected v16/v17 arg4-inline finding at a
 * different position within the block (s-reg coupling now local).
 */
    s32 arg4;
    /* ... */
    arg4 = *(s32 *)t0;
    debug_printf(..., arg4, arg5);
