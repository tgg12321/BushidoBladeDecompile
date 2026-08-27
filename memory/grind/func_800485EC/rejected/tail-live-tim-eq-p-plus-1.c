/* REJECTED (s1, measured 22): blocking the fold with a LIVE tim set routed
 * into pixdata:
 *
 *     p = tim + 2;
 *     tim = p + 1;
 *     spr->x = x; spr->y = y;
 *     spr->h = ((u16 *)p)[1];
 *     spr->w = *p;
 *     spr->pixdata = tim;
 *
 * The set does invalidate tim's cse quantity (fold blocked) but pixdata is
 * then stored from s1 (sw s1) instead of target's sw v1, the addiu lands as
 * addiu s1,v1,4, and the shifted liveness cascades — score 22. Same lesson
 * as tail-tim-walker-no-p.c: the value stored to pixdata must be p's own
 * post-increment.
 */
